#ifndef NDS_ENGINE_GAME_DISASTER_SYSTEM_H
#define NDS_ENGINE_GAME_DISASTER_SYSTEM_H

#include "engine/game/disaster.h"
#include "engine/game/round.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_disaster_system {
    nds_earthquake earthquake;
    nds_disaster_type active_type;
    uint8_t active;
} nds_disaster_system;

void nds_disaster_system_init(nds_disaster_system* system);
void nds_disaster_system_start(nds_disaster_system* system, nds_disaster_type type);
void nds_disaster_system_update(nds_disaster_system* system,
                                nds_player_controller* player,
                                nds_instance* scene, float dt);
void nds_disaster_system_stop(nds_disaster_system* system, nds_instance* scene);

#ifdef __cplusplus
}
#endif

#endif
