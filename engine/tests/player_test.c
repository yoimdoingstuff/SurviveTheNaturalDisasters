#include "engine/game/player.h"
#include "engine/scene/instance.h"
#include <assert.h>
#include <math.h>

static nds_instance* make_scene(void)
{
    nds_instance* root = nds_instance_create(NDS_CLASS_DATAMODEL, "TestWorld");
    nds_instance* spawn = nds_instance_create(NDS_CLASS_SPAWN_POINT, "Spawn");
    nds_part_properties p;
    assert(root && spawn);
    p.position = (nds_vec3){2, 3, 4};
    p.size = (nds_vec3){2, 1, 2};
    p.rotation = (nds_vec3){0, 0, 0};
    p.transparency = 0;
    p.reflectance = 0;
    p.color_rgba = 0xffffffffu;
    p.anchored = 1;
    p.can_collide = 0;
    p.visible = 1;
    p.mesh = NULL;
    p.texture = NULL;
    assert(nds_part_set_properties(spawn, &p) == NDS_OK);
    assert(nds_instance_set_parent(spawn, root) == NDS_OK);
    return root;
}

int main(void)
{
    nds_instance* scene = make_scene();
    nds_player_controller player;
    nds_camera camera;
    nds_instance* avatar;
    nds_instance* torso;
    nds_instance* left_arm;
    nds_instance* right_arm;
    nds_part_properties torso_props, left_props, right_props;

    nds_player_init(&player, scene);
    assert(player.alive);
    assert(player.third_person);
    assert(fabsf(player.position.x - 2.0f) < 0.001f);
    assert(fabsf(player.position.y - 5.0f) < 0.001f);
    assert(fabsf(player.position.z - 4.0f) < 0.001f);

    assert(nds_player_attach_visual(&player, scene) == NDS_OK);
    avatar = nds_instance_find_child(scene, "PlayerAvatar");
    assert(avatar);
    torso = nds_instance_find_child(avatar, "PlayerTorso");
    left_arm = nds_instance_find_child(avatar, "PlayerLeftArm");
    right_arm = nds_instance_find_child(avatar, "PlayerRightArm");
    assert(torso && left_arm && right_arm);
    assert(nds_part_get_properties(torso, &torso_props) == NDS_OK);
    assert(nds_part_get_properties(left_arm, &left_props) == NDS_OK);
    assert(nds_part_get_properties(right_arm, &right_props) == NDS_OK);
    assert(fabsf(torso_props.position.x - player.position.x) < 0.001f);
    assert(fabsf(torso_props.position.y - player.position.y) < 0.001f);
    assert(left_props.position.x < player.position.x);
    assert(right_props.position.x > player.position.x);
    assert(fabsf(left_props.position.z - player.position.z) < 0.001f);
    assert(fabsf(right_props.position.z - player.position.z) < 0.001f);
    assert(left_props.mesh != NULL);
    assert(right_props.mesh != NULL);

    nds_player_update_visual(&player, scene);
    assert(nds_part_get_properties(left_arm, &left_props) == NDS_OK);
    assert(nds_part_get_properties(right_arm, &right_props) == NDS_OK);
    assert(left_props.position.x < player.position.x);
    assert(right_props.position.x > player.position.x);

    nds_camera_init(&camera);
    nds_player_apply_camera(&player, &camera);
    assert(fabsf(camera.position[0] - player.position.x) > 1.0f || fabsf(camera.position[2] - player.position.z) > 1.0f);

    nds_player_set_third_person(&player, 0);
    nds_player_apply_camera(&player, &camera);
    assert(fabsf(camera.position[0] - player.position.x) < 0.001f);
    assert(fabsf(camera.position[1] - (player.position.y + 0.35f)) < 0.001f);

    nds_player_set_third_person(&player, 1);
    nds_player_rotate_camera(&player, 90.0f, 20.0f);
    nds_player_apply_camera(&player, &camera);
    assert(fabsf(camera.position[0] - player.position.x) > 1.0f || fabsf(camera.position[2] - player.position.z) > 1.0f);

    player.alive = 0;
    nds_player_update_visual(&player, scene);
    assert(nds_part_get_properties(torso, &torso_props) == NDS_OK);
    assert(!torso_props.visible);

    nds_instance_destroy(scene);
    return 0;
}
