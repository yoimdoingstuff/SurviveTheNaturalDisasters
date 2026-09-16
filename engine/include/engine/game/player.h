#ifndef NDS_ENGINE_GAME_PLAYER_H
#define NDS_ENGINE_GAME_PLAYER_H

#include "engine/core/types.h"
#include "engine/render/camera.h"
#include "engine/scene/part.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_player_controller {
    nds_vec3 position;
    nds_vec3 velocity;
    float half_width;
    float half_height;
    float move_speed;
    float jump_speed;
    float gravity;
    float health;
    float camera_yaw;
    float camera_pitch;
    float camera_distance;
    float facing_yaw;
    float animation_time;
    float launch_land_timer;
    uint8_t grounded;
    uint8_t alive;
    uint8_t third_person;
    uint8_t launch_land_services;
    uint8_t launch_land_bridge_open;
    uint8_t launch_land_rocket_active;
} nds_player_controller;

void nds_player_init(nds_player_controller* player, const nds_instance* scene);
void nds_player_update(nds_player_controller* player, const nds_instance* scene,
                       float dt, int move_forward, int move_back,
                       int move_left, int move_right, int jump);
void nds_player_apply_camera(const nds_player_controller* player, nds_camera* camera);
void nds_player_damage(nds_player_controller* player, float amount);
nds_result nds_player_attach_visual(nds_player_controller* player, nds_instance* scene);
void nds_player_update_visual(const nds_player_controller* player, nds_instance* scene);
void nds_player_rotate_camera(nds_player_controller* player, float yaw_delta, float pitch_delta);
void nds_player_zoom_camera(nds_player_controller* player, float zoom_delta);
void nds_player_set_third_person(nds_player_controller* player, int enabled);

#ifdef __cplusplus
}
#endif

#endif
