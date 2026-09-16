#ifndef NDS_ENGINE_GAME_DISASTER_SYSTEM_H
#define NDS_ENGINE_GAME_DISASTER_SYSTEM_H

#include "engine/game/disaster.h"
#include "engine/game/round.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_windstorm {
    float elapsed;
    float gust_timer;
    float gust_interval;
    float strength;
    uint32_t gust_count;
    uint8_t active;
} nds_windstorm;

typedef struct nds_disaster_settings {
    float warning_duration;
    float earthquake_pulse_interval;
    float windstorm_gust_interval;
    float windstorm_strength;
} nds_disaster_settings;

typedef struct nds_disaster_system {
    nds_earthquake earthquake;
    nds_windstorm windstorm;
    nds_disaster_type active_type;
    float warning_duration;
    float warning_remaining;
    uint8_t active;
} nds_disaster_system;

void nds_disaster_system_init(nds_disaster_system* system);
void nds_disaster_system_set_settings(nds_disaster_system* system,
                                       const nds_disaster_settings* settings);
void nds_disaster_system_start(nds_disaster_system* system, nds_disaster_type type);
void nds_disaster_system_update(nds_disaster_system* system,
                                nds_player_controller* player,
                                nds_instance* scene, float dt);
void nds_disaster_system_stop(nds_disaster_system* system, nds_instance* scene);
float nds_disaster_warning_remaining(const nds_disaster_system* system);
int nds_disaster_is_warning(const nds_disaster_system* system);

#ifdef __cplusplus
}
#endif

#endif
