#ifndef NDS_ENGINE_GAME_ROUND_H
#define NDS_ENGINE_GAME_ROUND_H

#include <stdint.h>

typedef enum nds_round_state {
    NDS_ROUND_INTERMISSION = 0,
    NDS_ROUND_PLAYING = 1,
    NDS_ROUND_RESULTS = 2
} nds_round_state;

typedef enum nds_disaster_type {
    NDS_DISASTER_EARTHQUAKE = 0,
    NDS_DISASTER_WINDSTORM = 1
} nds_disaster_type;

typedef struct nds_round {
    nds_round_state state;
    float state_time;
    float intermission_duration;
    float round_duration;
    float results_duration;
    uint32_t round_number;
    uint8_t player_survived;
    nds_disaster_type disaster;
} nds_round;

void nds_round_init(nds_round* round);
void nds_round_update(nds_round* round, float delta_seconds);
float nds_round_time_remaining(const nds_round* round);
float nds_round_phase_progress(const nds_round* round);
int nds_round_is_countdown(const nds_round* round);
const char* nds_round_state_name(nds_round_state state);
const char* nds_disaster_type_name(nds_disaster_type disaster);

#endif
