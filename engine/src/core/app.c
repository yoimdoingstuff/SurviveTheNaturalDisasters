#include "engine/core/app.h"
#include "engine/platform/platform.h"
#include "engine/core/log.h"
#include "engine/core/config.h"
#include "engine/core/clock.h"
#include "engine/core/memstat.h"
#include "engine/scene/instance.h"
#include "engine/content/map_loader.h"
#include "engine/content/map_catalog.h"
#include "engine/render/draw.h"
#include "engine/render/gles2.h"
#include "engine/render/camera.h"
#include "engine/render/mesh_cache.h"
#include "engine/render/texture_cache.h"
#include "engine/game/player.h"
#include "engine/game/round.h"
#include "engine/game/disaster_system.h"
#include "engine/game/physics.h"
#include "engine/game/save.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char* TAG="app";
static const char* DEFAULT_MAP_PATH="game/content/maps/island_ruins.ndsmap.json";
static const char* DEFAULT_SAVE_PATH="savegame.ini";

static const char* resolve_map_path(const char* requested, char* resolved, size_t resolved_size)
{
    const char* candidates[5];
    char parent1[512], parent2[512], parent3[512];
    size_t i;
    FILE* file;
    if (!requested || !requested[0] || !resolved || resolved_size == 0) return NULL;
    candidates[0] = requested;
    snprintf(parent1, sizeof(parent1), "../%s", requested);
    snprintf(parent2, sizeof(parent2), "../../%s", requested);
    snprintf(parent3, sizeof(parent3), "../../../%s", requested);
    candidates[1] = parent1;
    candidates[2] = parent2;
    candidates[3] = parent3;
    candidates[4] = NULL;
    for (i = 0; candidates[i]; ++i) {
        file = fopen(candidates[i], "rb");
        if (!file) continue;
        fclose(file);
        snprintf(resolved, resolved_size, "%s", candidates[i]);
        return resolved;
    }
    return NULL;
}

static nds_result create_demo_scene(nds_instance** out_root)
{
    nds_instance* floor; nds_instance* tower; nds_instance* block; nds_part_properties p;
    if(!out_root)return NDS_ERR_INVALID_ARG;
    *out_root=nds_instance_create(NDS_CLASS_DATAMODEL,"DemoWorld");
    if(!*out_root)return NDS_ERR_UNKNOWN;
    floor=nds_instance_create(NDS_CLASS_PART,"Floor"); tower=nds_instance_create(NDS_CLASS_PART,"Tower"); block=nds_instance_create(NDS_CLASS_PART,"Block");
    if(!floor||!tower||!block){nds_instance_destroy(*out_root);*out_root=NULL;return NDS_ERR_UNKNOWN;}
    memset(&p,0,sizeof(p));p.anchored=1;p.can_collide=1;p.visible=1;p.position=(nds_vec3){0,-1,0};p.size=(nds_vec3){12,1,8};p.color_rgba=0x4a6fa1ff;nds_part_set_properties(floor,&p);
    p.position=(nds_vec3){-3,1.5f,0};p.size=(nds_vec3){2,4,2};p.color_rgba=0xd06060ff;nds_part_set_properties(tower,&p);
    p.position=(nds_vec3){2.5f,.5f,-1.5f};p.size=(nds_vec3){3,2,3};p.color_rgba=0x66b06aff;nds_part_set_properties(block,&p);
    if(nds_instance_set_parent(floor,*out_root)!=NDS_OK||nds_instance_set_parent(tower,*out_root)!=NDS_OK||nds_instance_set_parent(block,*out_root)!=NDS_OK){nds_instance_destroy(*out_root);*out_root=NULL;return NDS_ERR_UNKNOWN;}
    return NDS_OK;
}

static nds_result load_startup_scene(const char* map_path,nds_mesh_cache* mesh_cache,nds_texture_cache* texture_cache,nds_instance** out_root)
{
    nds_result rc; char resolved[512]; const char* path=map_path&&map_path[0]?map_path:DEFAULT_MAP_PATH;
    if(!out_root)return NDS_ERR_INVALID_ARG; *out_root=NULL;
    if(!resolve_map_path(path,resolved,sizeof(resolved))) {
        NDS_LOGW(TAG,"map file '%s' was not found from the current runtime directory",path);
        return create_demo_scene(out_root);
    }
    rc=nds_map_load_json(resolved,out_root);
    if(rc==NDS_OK){
        NDS_LOGI(TAG,"loaded project map: %s",resolved);
        if(mesh_cache){rc=nds_map_resolve_meshes(*out_root,mesh_cache);if(rc!=NDS_OK)NDS_LOGW(TAG,"one or more project mesh assets could not be resolved (%d)",(int)rc);}
        if(texture_cache){rc=nds_map_resolve_textures(*out_root,texture_cache);if(rc!=NDS_OK)NDS_LOGW(TAG,"one or more project texture assets could not be resolved (%d)",(int)rc);}
        return NDS_OK;
    }
    NDS_LOGW(TAG,"project map '%s' unavailable (%d), using built-in demo scene",resolved,(int)rc);
    return create_demo_scene(out_root);
}

/* Keep the third-person camera outside collidable geometry. The player avatar
 * itself is non-collidable, so the ray can start at the player's camera target
 * without immediately hitting the character. */
static void constrain_third_person_camera(nds_camera* camera, const nds_player_controller* player,
                                          const nds_physics_world* physics)
{
    nds_vec3 origin, desired, direction, normal;
    nds_instance* hit = NULL;
    float dx, dy, dz, distance, hit_distance;
    const float camera_padding = 0.35f;
    if (!camera || !player || !physics || !player->third_person) return;
    origin = (nds_vec3){camera->target[0], camera->target[1], camera->target[2]};
    desired = (nds_vec3){camera->position[0], camera->position[1], camera->position[2]};
    dx = desired.x - origin.x; dy = desired.y - origin.y; dz = desired.z - origin.z;
    distance = sqrtf(dx*dx + dy*dy + dz*dz);
    if (distance <= 0.001f) return;
    direction = (nds_vec3){dx / distance, dy / distance, dz / distance};
    if (nds_physics_raycast(physics, origin, direction, distance, &hit, &hit_distance, &normal) != NDS_OK || !hit) return;
    hit_distance -= camera_padding;
    if (hit_distance < 0.5f) hit_distance = 0.5f;
    if (hit_distance > distance) hit_distance = distance;
    camera->position[0] = origin.x + direction.x * hit_distance;
    camera->position[1] = origin.y + direction.y * hit_distance;
    camera->position[2] = origin.z + direction.z * hit_distance;
}

nds_result nds_app_run(const nds_app_options* options)
{
    nds_result rc; nds_config* cfg; platform_window_desc win_desc; nds_instance* scene=NULL; nds_draw_list draw_list; nds_gles2_renderer* renderer=NULL; nds_mesh_cache* mesh_cache=NULL; nds_texture_cache* texture_cache=NULL; nds_camera camera; nds_player_controller player; nds_round round; nds_disaster_system disasters; nds_physics_world physics; nds_save_state save_state; int render_width=0,render_height=0; nds_clock clock; unsigned long frames_run=0; const int smoke_frames=options?options->smoke_test_frames:0; const int smoke_mode=smoke_frames>0; int elimination_logged=0; int camera_toggle_previous=0; int camera_drag_previous=0; int previous_mouse_x=0; int previous_mouse_y=0; float round_survival_time=0.0f; int result_recorded=0;
    if(!options)return NDS_ERR_INVALID_ARG; rc=platform_init();if(rc!=NDS_OK){NDS_LOGE(TAG,"platform_init failed (%d)",(int)rc);return rc;}
    cfg=nds_config_create();if(!cfg){platform_shutdown();return NDS_ERR_UNKNOWN;}
    if(options->config_path){nds_result cfg_rc=nds_config_load_file(cfg,options->config_path);if(cfg_rc!=NDS_OK)NDS_LOGW(TAG,"no config at '%s' yet, using defaults",options->config_path);}
    nds_save_state_init(&save_state);
    if(nds_save_load(DEFAULT_SAVE_PATH,&save_state)!=NDS_OK)
        NDS_LOGI(TAG,"no existing save at '%s', using fresh defaults",DEFAULT_SAVE_PATH);
    memset(&win_desc,0,sizeof(win_desc));win_desc.title=options->window_title?options->window_title:"Natural Disaster Survival - Local Runtime";win_desc.width=options->window_width>0?options->window_width:nds_config_get_int(cfg,"window.width",1024);win_desc.height=options->window_height>0?options->window_height:nds_config_get_int(cfg,"window.height",768);win_desc.resizable=1;
    rc=platform_create_window(&win_desc);if(rc!=NDS_OK){nds_config_destroy(cfg);platform_shutdown();return rc;}
    nds_draw_list_init(&draw_list);nds_camera_init(&camera);nds_round_init(&round);nds_round_set_durations(&round,save_state.intermission_duration,save_state.round_duration,save_state.results_duration);nds_disaster_system_init(&disasters);nds_disaster_system_set_settings(&disasters,&save_state.disaster_settings);
    rc=nds_physics_init(&physics,256);if(rc!=NDS_OK)goto cleanup_window;rc=nds_mesh_cache_create(&mesh_cache,256);if(rc!=NDS_OK)goto cleanup_physics;rc=nds_texture_cache_create(&texture_cache,256);if(rc!=NDS_OK)goto cleanup_mesh;
    {
        size_t initial_map=nds_map_catalog_select(1);const char* initial_path=options->map_path&&options->map_path[0]?options->map_path:nds_map_catalog_path(initial_map);rc=load_startup_scene(initial_path,mesh_cache,texture_cache,&scene);
    }
    if(rc!=NDS_OK)goto cleanup_texture;
    rc=nds_physics_add_scene(&physics,scene);if(rc!=NDS_OK)goto cleanup_scene;
    nds_player_init(&player,scene);nds_player_attach_visual(&player,scene);nds_player_update_visual(&player,scene);nds_player_apply_camera(&player,&camera);constrain_third_person_camera(&camera,&player,&physics);
    if(!smoke_mode){nds_gles2_desc render_desc={win_desc.width,win_desc.height,camera.fov_y_degrees,camera.near_plane,camera.far_plane};rc=nds_gles2_renderer_create(&renderer,&render_desc);if(rc!=NDS_OK)goto cleanup_scene;render_width=win_desc.width;render_height=win_desc.height;NDS_LOGI(TAG,"OpenGL rendering backend active; WASD moves, SPACE jumps, RMB drag or LEFT/RIGHT arrows rotate camera, C toggles first/third person");}else NDS_LOGI(TAG,"smoke test mode: rendering disabled; exercising engine/game systems only");
    nds_clock_init(&clock);nds_perf_reset();
    while(!platform_quit_requested()){
        double dt;nds_round_state previous_round_state;int mouse_x=0,mouse_y=0;int camera_drag=platform_is_mouse_button_down(1);nds_perf_begin_frame();dt=nds_clock_tick(&clock);if(platform_poll_events()!=0)platform_request_quit();if(platform_is_key_down(PLATFORM_KEY_ESCAPE))platform_request_quit();
        {int toggle=platform_is_key_down(PLATFORM_KEY_C);if(toggle&&!camera_toggle_previous)nds_player_set_third_person(&player,!player.third_person);camera_toggle_previous=toggle;}
        platform_get_mouse_position(&mouse_x,&mouse_y);if(camera_drag&&!camera_drag_previous){previous_mouse_x=mouse_x;previous_mouse_y=mouse_y;}if(camera_drag){int dx=mouse_x-previous_mouse_x;int dy=mouse_y-previous_mouse_y;if(dx||dy)nds_player_rotate_camera(&player,-0.25f*(float)dx,0.25f*(float)dy);}camera_drag_previous=camera_drag;previous_mouse_x=mouse_x;previous_mouse_y=mouse_y;if(platform_is_key_down(PLATFORM_KEY_LEFT))nds_player_rotate_camera(&player,-120.0f*(float)dt,0.0f);if(platform_is_key_down(PLATFORM_KEY_RIGHT))nds_player_rotate_camera(&player,120.0f*(float)dt,0.0f);
        previous_round_state=round.state;nds_round_update(&round,(float)dt);
        if(round.state!=previous_round_state){
            NDS_LOGI(TAG,"round %u: %s (%s)",round.round_number,nds_round_state_name(round.state),nds_disaster_type_name(round.disaster));
            if(round.state==NDS_ROUND_PLAYING){
                size_t map_index=nds_map_catalog_select(round.round_number);const char* map_path=options->map_path&&options->map_path[0]?options->map_path:nds_map_catalog_path(map_index);const char* map_name=options->map_path&&options->map_path[0]?"Custom Map":nds_map_catalog_name(map_index);
                if(map_path){nds_instance* next_scene=NULL;nds_result map_rc=load_startup_scene(map_path,mesh_cache,texture_cache,&next_scene);if(map_rc==NDS_OK&&next_scene){nds_instance_destroy(scene);scene=next_scene;nds_physics_clear(&physics);if(nds_physics_add_scene(&physics,scene)!=NDS_OK)NDS_LOGW(TAG,"round %u physics scene registration failed",round.round_number);nds_player_init(&player,scene);nds_player_attach_visual(&player,scene);nds_player_update_visual(&player,scene);nds_player_apply_camera(&player,&camera);constrain_third_person_camera(&camera,&player,&physics);NDS_LOGI(TAG,"round %u map: %s (%s)",round.round_number,map_name?map_name:"Unnamed",map_path);}else NDS_LOGW(TAG,"round %u map selection failed: %s",round.round_number,map_path);}elimination_logged=0;round_survival_time=0.0f;result_recorded=0;nds_disaster_system_start(&disasters,round.disaster);
            }else if(round.state==NDS_ROUND_RESULTS){nds_disaster_system_stop(&disasters,scene);if(!round.player_survived)round.player_survived=player.alive;if(!result_recorded){nds_game_stats_record_round(&save_state.stats,round.player_survived,round_survival_time);result_recorded=1;if(nds_save_write(DEFAULT_SAVE_PATH,&save_state)!=NDS_OK)NDS_LOGW(TAG,"failed to save gameplay state to '%s'",DEFAULT_SAVE_PATH);else NDS_LOGI(TAG,"saved stats: %u played, %u survived, %u eliminated",save_state.stats.rounds_played,save_state.stats.rounds_survived,save_state.stats.rounds_eliminated);}NDS_LOGI(TAG,"round %u result: %s",round.round_number,round.player_survived?"SURVIVED":"ELIMINATED");}
            else if(round.state==NDS_ROUND_INTERMISSION&&previous_round_state==NDS_ROUND_RESULTS){nds_player_init(&player,scene);nds_player_attach_visual(&player,scene);nds_player_update_visual(&player,scene);nds_player_apply_camera(&player,&camera);constrain_third_person_camera(&camera,&player,&physics);}
        }
        if(round.state==NDS_ROUND_PLAYING&&player.alive){round_survival_time+=(float)dt;nds_disaster_system_update(&disasters,&player,scene,(float)dt);if(nds_physics_update(&physics,(float)dt)!=NDS_OK)NDS_LOGW(TAG,"physics update failed");nds_player_update(&player,scene,(float)dt,platform_is_key_down(PLATFORM_KEY_W),platform_is_key_down(PLATFORM_KEY_S),platform_is_key_down(PLATFORM_KEY_A),platform_is_key_down(PLATFORM_KEY_D),platform_is_key_down(PLATFORM_KEY_SPACE));nds_player_update_visual(&player,scene);if(!player.alive&&!elimination_logged){NDS_LOGW(TAG,"round %u: player eliminated",round.round_number);elimination_logged=1;nds_round_finish(&round,0);nds_disaster_system_stop(&disasters,scene);NDS_LOGI(TAG,"round %u: entering results early after elimination",round.round_number);}}
        nds_player_apply_camera(&player,&camera);constrain_third_person_camera(&camera,&player,&physics);
        if(!smoke_mode){int window_width=0,window_height=0;platform_get_window_size(&window_width,&window_height);if(window_width>0&&window_height>0&&(window_width!=render_width||window_height!=render_height)){if(nds_gles2_renderer_resize(renderer,window_width,window_height)==NDS_OK){render_width=window_width;render_height=window_height;}else{NDS_LOGE(TAG,"renderer resize failed (%dx%d)",window_width,window_height);platform_request_quit();}}nds_draw_list_reset(&draw_list);rc=nds_draw_list_build_from_tree(&draw_list,scene);if(rc==NDS_OK)rc=nds_gles2_renderer_begin(renderer);if(rc==NDS_OK)rc=nds_gles2_renderer_draw_parts(renderer,&draw_list,&camera);if(rc==NDS_OK){nds_hud_state hud={&player,&round,&disasters};rc=nds_gles2_renderer_draw_hud(renderer,&hud);}if(rc==NDS_OK)rc=nds_gles2_renderer_end(renderer);if(rc!=NDS_OK){NDS_LOGE(TAG,"render failed (%d)",(int)rc);platform_request_quit();}}
        nds_perf_end_frame();frames_run++;if(smoke_frames>0&&(int)frames_run>=smoke_frames){NDS_LOGI(TAG,"smoke test target reached (%lu frames), requesting quit",frames_run);platform_request_quit();}if(frames_run%300==0){nds_perf_stats perf;nds_perf_get_stats(&perf);nds_mem_stats mem;nds_mem_get_stats(&mem);NDS_LOGI(TAG,"frame %lu: avg %.2fms (min %.2f / max %.2f) mem %zu bytes (peak %zu, %zu allocs / %zu frees)",frames_run,perf.avg_frame_ms,perf.min_frame_ms,perf.max_frame_ms,mem.current_bytes,mem.peak_bytes,mem.total_allocations,mem.total_frees);}
    }
    rc=NDS_OK;
cleanup_renderer:nds_gles2_renderer_destroy(renderer);
cleanup_scene:nds_instance_destroy(scene);
cleanup_texture:nds_texture_cache_destroy(texture_cache);
cleanup_mesh:nds_mesh_cache_destroy(mesh_cache);
cleanup_physics:nds_physics_destroy(&physics);
nds_draw_list_destroy(&draw_list);
cleanup_window:platform_destroy_window();nds_config_destroy(cfg);platform_shutdown();return rc;
}
