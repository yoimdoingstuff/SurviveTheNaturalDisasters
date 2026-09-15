#include "engine/game/round.h"

static void nds_round_set_state(nds_round* round, nds_round_state state)
{
    round->state = state;
    round->state_time = 0.0f;
}

static nds_disaster_type nds_round_select_disaster(uint32_t round_number)
{
    /* Keep selection deterministic so offline play and future authoritative
     * networking can reproduce the same round sequence. */
    return (nds_disaster_type)((round_number - 1u) % 2u);
}

void nds_round_init(nds_round* round)
{
    if (!round) return;
    round->state = NDS_ROUND_INTERMISSION;
    round->state_time = 0.0f;
    round->intermission_duration = 10.0f;
    round->round_duration = 60.0f;
    round->results_duration = 5.0f;
    round->round_number = 0;
    round->player_survived = 0;
    round->disaster = NDS_DISASTER_EARTHQUAKE;
}

void nds_round_update(nds_round* round, float delta_seconds)
{
    if (!round || delta_seconds <= 0.0f) return;
    round->state_time += delta_seconds;
    switch (round->state) {
    case NDS_ROUND_INTERMISSION:
        if (round->state_time >= round->intermission_duration) {
            round->round_number++;
            round->player_survived = 0;
            round->disaster = nds_round_select_disaster(round->round_number);
            nds_round_set_state(round, NDS_ROUND_PLAYING);
        }
        break;
    case NDS_ROUND_PLAYING:
        if (round->state_time >= round->round_duration) nds_round_set_state(round, NDS_ROUND_RESULTS);
        break;
    case NDS_ROUND_RESULTS:
        if (round->state_time >= round->results_duration) nds_round_set_state(round, NDS_ROUND_INTERMISSION);
        break;
    }
}

const char* nds_round_state_name(nds_round_state state)
{
    switch (state) {
    case NDS_ROUND_INTERMISSION: return "Intermission";
    case NDS_ROUND_PLAYING: return "Playing";
    case NDS_ROUND_RESULTS: return "Results";
    default: return "Unknown";
    }
}

const char* nds_disaster_type_name(nds_disaster_type disaster)
{
    switch (disaster) {
    case NDS_DISASTER_EARTHQUAKE: return "Earthquake";
    case NDS_DISASTER_WINDSTORM: return "Windstorm";
    default: return "Unknown";
    }
}
