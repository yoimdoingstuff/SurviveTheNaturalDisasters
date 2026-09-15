#ifndef NDS_ENGINE_GAME_DISASTER_H
#define NDS_ENGINE_GAME_DISASTER_H

#include "engine/game/player.h"
#include "engine/scene/instance.h"
#include <stdint.h>

typedef struct nds_earthquake {
    float elapsed;
    float pulse_timer;
    float pulse_interval;
    float previous_shake_x;
    float previous_shake_z;
    uint32_t pulse_count;
    uint8_t active;
} nds_earthquake;

void nds_earthquake_init(nds_earthquake* earthquake);
void nds_earthquake_start(nds_earthquake* earthquake);
void nds_earthquake_stop(nds_earthquake* earthquake);
void nds_earthquake_update(nds_earthquake* earthquake, nds_player_controller* player,
                           nds_instance* scene, float dt);

#endif
