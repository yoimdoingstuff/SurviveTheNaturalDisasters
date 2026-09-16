#include "engine/game/round.h"

static float nds_round_duration(const nds_round* round)
{
    if (!round) return 0.0f;
    switch (round->state) {
    case NDS_ROUND_INTERMISSION: return round->intermission_duration;
    case NDS_ROUND_PLAYING: return round->round_duration;
    case NDS_ROUND_RESULTS: return round->results_duration;
    default: return 0.0f;
    }
}

static void nds_round_set_state(nds_round* round, nds_round_state state)
{
    round->state = state;
    round->state_time = 0.0f;
}

static nds_disaster_type nds_round_select_disaster(uint32_t round_number)
{
    return (nds_disaster_type)((round_number - 1u) % 2u);
}

void nds_round_init(nds_round* round)
{
    if (!round) return;
    round->state = NDS_ROUND_INTERMISSION;
    round->state_time = 0.0f;
    round->intermission_duration = 5.0f;
    round->round_duration = 60.0f;
    round->results_duration = 5.0f;
    round->round_number = 0;
    round->player_survived = 0;
    round->disaster = NDS_DISASTER_EARTHQUAKE;
}

void nds_round_set_durations(nds_round* round, float intermission_seconds,
                             float playing_seconds, float results_seconds)
{
    if (!round) return;
    if (intermission_seconds > 0.0f) round->intermission_duration = intermission_seconds;
    if (playing_seconds > 0.0f) round->round_duration = playing_seconds;
    if (results_seconds > 0.0f) round->results_duration = results_seconds;
}

void nds_round_update(nds_round* round, float delta_seconds)
{
    float remaining;
    if (!round || delta_seconds <= 0.0f) return;
    remaining = delta_seconds;
    while (remaining > 0.0f) {
        float duration = nds_round_duration(round);
        float available;
        if (duration <= 0.0f) available = remaining;
        else {
            available = duration - round->state_time;
            if (available < 0.0f) available = 0.0f;
        }
        if (duration > 0.0f && remaining < available) {
            round->state_time += remaining;
            remaining = 0.0f;
            break;
        }
        if (duration > 0.0f) {
            remaining -= available;
            round->state_time = duration;
        } else remaining = 0.0f;
        switch (round->state) {
        case NDS_ROUND_INTERMISSION:
            round->round_number++;
            round->player_survived = 0;
            round->disaster = nds_round_select_disaster(round->round_number);
            nds_round_set_state(round, NDS_ROUND_PLAYING);
            break;
        case NDS_ROUND_PLAYING:
            nds_round_set_state(round, NDS_ROUND_RESULTS);
            break;
        case NDS_ROUND_RESULTS:
            nds_round_set_state(round, NDS_ROUND_INTERMISSION);
            break;
        default:
            return;
        }
    }
}

void nds_round_finish(nds_round* round, uint8_t player_survived)
{
    if (!round || round->state != NDS_ROUND_PLAYING) return;
    round->player_survived = player_survived ? 1u : 0u;
    nds_round_set_state(round, NDS_ROUND_RESULTS);
}

float nds_round_time_remaining(const nds_round* round)
{
    float duration = nds_round_duration(round);
    if (!round || duration <= 0.0f || round->state_time >= duration) return 0.0f;
    return duration - round->state_time;
}

float nds_round_phase_progress(const nds_round* round)
{
    float duration = nds_round_duration(round);
    if (!round || duration <= 0.0f) return 1.0f;
    if (round->state_time <= 0.0f) return 0.0f;
    if (round->state_time >= duration) return 1.0f;
    return round->state_time / duration;
}

int nds_round_is_countdown(const nds_round* round)
{
    if (!round) return 0;
    return round->state == NDS_ROUND_INTERMISSION || round->state == NDS_ROUND_RESULTS;
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
