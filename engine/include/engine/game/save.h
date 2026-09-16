#ifndef NDS_ENGINE_GAME_SAVE_H
#define NDS_ENGINE_GAME_SAVE_H

#include "engine/game/stats.h"
#include "engine/game/disaster_system.h"
#include "engine/game/round.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_save_state {
    nds_game_stats stats;
    float intermission_duration;
    float round_duration;
    float results_duration;
    nds_disaster_settings disaster_settings;
} nds_save_state;

/* Fill a state with the runtime defaults used by a fresh installation. */
void nds_save_state_init(nds_save_state* state);

/* Load a save/config state. Missing keys retain their supplied defaults. */
nds_result nds_save_load(const char* path, nds_save_state* state);

/* Persist gameplay statistics and user-adjustable gameplay settings. */
nds_result nds_save_write(const char* path, const nds_save_state* state);

#ifdef __cplusplus
}
#endif

#endif
