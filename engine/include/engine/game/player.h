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
    uint8_t grounded;
} nds_player_controller;

void nds_player_init(nds_player_controller* player, const nds_instance* scene);
void nds_player_update(nds_player_controller* player, const nds_instance* scene,
                       float dt, int move_forward, int move_back,
                       int move_left, int move_right, int jump);
void nds_player_apply_camera(const nds_player_controller* player, nds_camera* camera);

#ifdef __cplusplus
}
#endif

#endif
