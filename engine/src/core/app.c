#include "engine/core/app.h"
#include "engine/platform/platform.h"
#include "engine/core/log.h"
#include "engine/core/config.h"
#include "engine/core/clock.h"
#include "engine/core/memstat.h"
#include "engine/scene/instance.h"
#include "engine/scene/part.h"
#include "engine/content/map_loader.h"
#include "engine/render/draw.h"
#include "engine/render/gles2.h"
#include "engine/render/camera.h"
#include "engine/render/mesh_cache.h"
#include "engine/render/texture_cache.h"
#include "engine/game/player.h"
#include "engine/game/round.h"

#include <string.h>

static const char* TAG = "app";
static const char* DEFAULT_MAP_PATH = "game/content/maps/render_test.ndsmap.json";

static nds_result create_demo_scene(nds_instance** out_root)
{
    if (!out_root) return NDS_ERR_INVALID_ARG;
    *out_root = nds_instance_create(NDS_CLASS_DATAMODEL, "DemoWorld");
    if (!*out_root) return NDS_ERR_UNKNOWN;
    nds_instance* floor = nds_instance_create(NDS_CLASS_PART, "Floor");
    nds_instance* tower = nds_instance_create(NDS_CLASS_PART, "Tower");
    nds_instance* block = nds_instance_create(NDS_CLASS_PART, "Block");
    if (!floor || !tower || !block) { nds_instance_destroy(*out_root); *out_root = NULL; return NDS_ERR_UNKNOWN; }
    nds_part_properties p; memset(&p, 0, sizeof(p)); p.anchored = 1; p.can_collide = 1; p.visible = 1;
    p.position = (nds_vec3){0.0f, -1.0f, 0.0f}; p.size = (nds_vec3){12.0f, 1.0f, 8.0f}; p.color_rgba = 0x4a6fa1ff; nds_part_set_properties(floor, &p);
    p.position = (nds_vec3){-3.0f, 1.5f, 0.0f}; p.size = (nds_vec3){2.0f, 4.0f, 2.0f}; p.color_rgba = 0xd06060ff; nds_part_set_properties(tower, &p);
    p.position = (nds_vec3){2.5f, 0.5f, -1.5f}; p.size = (nds_vec3){3.0f, 2.0f, 3.0f}; p.color_rgba = 0x66b06aff; nds_part_set_properties(block, &p);
    if (nds_instance_set_parent(floor, *out_root) != NDS_OK || nds_instance_set_parent(tower, *out_root) != NDS_OK || nds_instance_set_parent(block, *out_root) != NDS_OK) { nds_instance_destroy(*out_root); *out_root = NULL; return NDS_ERR_UNKNOWN; }
    return NDS_OK;
}

static nds_result load_startup_scene(const char* map_path, nds_mesh_cache* mesh_cache, nds_texture_cache* texture_cache, nds_instance** out_root)
{
    nds_result rc; const char* path = map_path && map_path[0] ? map_path : DEFAULT_MAP_PATH;
    if (!out_root) return NDS_ERR_INVALID_ARG; *out_root = NULL;
    rc = nds_map_load_json(path, out_root);
    if (rc == NDS_OK) {
        NDS_LOGI(TAG, "loaded project map: %s", path);
        if (mesh_cache) { rc = nds_map_resolve_meshes(*out_root, mesh_cache); if (rc != NDS_OK) NDS_LOGW(TAG, "one or more project mesh assets could not be resolved (%d)", (int)rc); }
        if (texture_cache) { rc = nds_map_resolve_textures(*out_root, texture_cache); if (rc != NDS_OK) NDS_LOGW(TAG, "one or more project texture assets could not be resolved (%d)", (int)rc); }
        return NDS_OK;
    }
    NDS_LOGW(TAG, "project map '%s' unavailable (%d), using built-in demo scene", path, (int)rc);
    return create_demo_scene(out_root);
}

nds_result nds_app_run(const nds_app_options* options)
{
    if (!options) return NDS_ERR_INVALID_ARG;
    nds_result rc = platform_init();
    if (rc != NDS_OK) { NDS_LOGE(TAG, "platform_init failed (%d)", (int)rc); return rc; }
    nds_config* cfg = nds_config_create();
    if (options->config_path) { nds_result cfg_rc = nds_config_load_file(cfg, options->config_path); if (cfg_rc != NDS_OK) NDS_LOGW(TAG, "no config at '%s' yet, using defaults", options->config_path); }
    platform_window_desc win_desc;
    win_desc.title = options->window_title ? options->window_title : "Natural Disaster Survival - Local Runtime";
    win_desc.width = options->window_width > 0 ? options->window_width : nds_config_get_int(cfg, "window.width", 1024);
    win_desc.height = options->window_height > 0 ? options->window_height : nds_config_get_int(cfg, "window.height", 768);
    win_desc.resizable = 1;
    rc = platform_create_window(&win_desc);
    if (rc != NDS_OK) { NDS_LOGE(TAG, "platform_create_window failed (%d)", (int)rc); nds_config_destroy(cfg); platform_shutdown(); return rc; }
    NDS_LOGI(TAG, "boot ok: window %dx%d", win_desc.width, win_desc.height);
    nds_instance* scene = NULL; nds_draw_list draw_list; nds_gles2_renderer* renderer = NULL; nds_mesh_cache* mesh_cache = NULL; nds_texture_cache* texture_cache = NULL; nds_camera camera; nds_player_controller player; nds_round round;
    nds_draw_list_init(&draw_list); nds_camera_init(&camera); nds_round_init(&round);
    rc = nds_mesh_cache_create(&mesh_cache, 256);
    if (rc != NDS_OK) { NDS_LOGE(TAG, "mesh cache creation failed (%d)", (int)rc); nds_draw_list_destroy(&draw_list); platform_destroy_window(); nds_config_destroy(cfg); platform_shutdown(); return rc; }
    rc = nds_texture_cache_create(&texture_cache, 256);
    if (rc != NDS_OK) { NDS_LOGE(TAG, "texture cache creation failed (%d)", (int)rc); nds_mesh_cache_destroy(mesh_cache); nds_draw_list_destroy(&draw_list); platform_destroy_window(); nds_config_destroy(cfg); platform_shutdown(); return rc; }
    rc = load_startup_scene(options->map_path, mesh_cache, texture_cache, &scene);
    if (rc != NDS_OK) { NDS_LOGE(TAG, "scene creation failed (%d)", (int)rc); nds_texture_cache_destroy(texture_cache); nds_mesh_cache_destroy(mesh_cache); nds_draw_list_destroy(&draw_list); platform_destroy_window(); nds_config_destroy(cfg); platform_shutdown(); return rc; }
    nds_player_init(&player, scene); nds_player_apply_camera(&player, &camera);
    nds_gles2_desc render_desc; render_desc.width = win_desc.width; render_desc.height = win_desc.height; render_desc.fov_y_degrees = camera.fov_y_degrees; render_desc.near_plane = camera.near_plane; render_desc.far_plane = camera.far_plane;
    rc = nds_gles2_renderer_create(&renderer, &render_desc);
    if (rc != NDS_OK) { NDS_LOGE(TAG, "renderer creation failed (%d)", (int)rc); nds_instance_destroy(scene); nds_texture_cache_destroy(texture_cache); nds_mesh_cache_destroy(mesh_cache); nds_draw_list_destroy(&draw_list); platform_destroy_window(); nds_config_destroy(cfg); platform_shutdown(); return rc; }
    NDS_LOGI(TAG, "OpenGL rendering backend active; WASD moves, SPACE jumps");
    int render_width = win_desc.width, render_height = win_desc.height; nds_clock clock; nds_clock_init(&clock); nds_perf_reset(); unsigned long frames_run = 0; const int smoke_frames = options->smoke_test_frames;
    while (!platform_quit_requested()) {
        if (platform_poll_events() != 0) platform_request_quit();
        if (platform_is_key_down(PLATFORM_KEY_ESCAPE)) platform_request_quit();
        nds_perf_begin_frame(); double dt = nds_clock_tick(&clock);
        nds_round_state previous_round_state = round.state;
        nds_round_update(&round, (float)dt);
        if (round.state != previous_round_state) {
            NDS_LOGI(TAG, "round %u: %s", round.round_number, nds_round_state_name(round.state));
        }
        if (round.state == NDS_ROUND_PLAYING) {
            nds_player_update(&player, scene, (float)dt,
                              platform_is_key_down(PLATFORM_KEY_W), platform_is_key_down(PLATFORM_KEY_S),
                              platform_is_key_down(PLATFORM_KEY_A), platform_is_key_down(PLATFORM_KEY_D),
                              platform_is_key_down(PLATFORM_KEY_SPACE));
        }
        nds_player_apply_camera(&player, &camera);
        int window_width = 0, window_height = 0; platform_get_window_size(&window_width, &window_height);
        if (window_width > 0 && window_height > 0 && (window_width != render_width || window_height != render_height)) { if (nds_gles2_renderer_resize(renderer, window_width, window_height) == NDS_OK) { render_width = window_width; render_height = window_height; } else { NDS_LOGE(TAG, "renderer resize failed (%dx%d)", window_width, window_height); platform_request_quit(); } }
        nds_draw_list_reset(&draw_list); rc = nds_draw_list_build_from_tree(&draw_list, scene); if (rc == NDS_OK) rc = nds_gles2_renderer_begin(renderer); if (rc == NDS_OK) rc = nds_gles2_renderer_draw_parts(renderer, &draw_list, &camera); if (rc == NDS_OK) rc = nds_gles2_renderer_end(renderer);
        if (rc != NDS_OK) { NDS_LOGE(TAG, "render failed (%d)", (int)rc); platform_request_quit(); }
        nds_perf_end_frame(); frames_run++;
        if (smoke_frames > 0 && (int)frames_run >= smoke_frames) { NDS_LOGI(TAG, "smoke test target reached (%lu frames), requesting quit", frames_run); platform_request_quit(); }
        if (frames_run % 300 == 0) { nds_perf_stats perf; nds_perf_get_stats(&perf); nds_mem_stats mem; nds_mem_get_stats(&mem); NDS_LOGI(TAG, "frame %lu: avg %.2fms (min %.2f / max %.2f) mem %zu bytes (peak %zu, %zu allocs / %zu frees)", frames_run, perf.avg_frame_ms, perf.min_frame_ms, perf.max_frame_ms, mem.current_bytes, mem.peak_bytes, mem.total_allocations, mem.peak_bytes, mem.total_allocations, mem.total_frees); }
    }
    NDS_LOGI(TAG, "shutting down after %lu frames", frames_run); nds_gles2_renderer_destroy(renderer); nds_instance_destroy(scene); nds_texture_cache_destroy(texture_cache); nds_mesh_cache_destroy(mesh_cache); nds_draw_list_destroy(&draw_list); platform_destroy_window(); nds_config_destroy(cfg); platform_shutdown(); if (smoke_frames > 0) NDS_LOGI(TAG, "SMOKE TEST PASSED"); return NDS_OK;
}
