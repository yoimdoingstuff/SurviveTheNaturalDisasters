#include "engine/game/round.h"

#include <assert.h>
#include <string.h>
#include <math.h>

int main(void)
{
    nds_round round;
    nds_round_init(&round);

    assert(round.state == NDS_ROUND_INTERMISSION);
    assert(round.round_number == 0);
    assert(round.disaster == NDS_DISASTER_EARTHQUAKE);
    assert(strcmp(nds_round_state_name(round.state), "Intermission") == 0);
    assert(strcmp(nds_disaster_type_name(round.disaster), "Earthquake") == 0);
    assert(fabsf(nds_round_time_remaining(&round) - 5.0f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round)) < 0.001f);
    assert(nds_round_is_countdown(&round));

    /* Settings should affect phase durations without resetting the current phase. */
    nds_round_update(&round, 1.0f);
    nds_round_set_durations(&round, 3.0f, 12.0f, 2.0f);
    assert(round.state == NDS_ROUND_INTERMISSION);
    assert(fabsf(nds_round_time_remaining(&round) - 2.0f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round) - (1.0f / 3.0f)) < 0.001f);

    /* Non-positive values are ignored so a bad settings file cannot disable a phase. */
    nds_round_set_durations(&round, 0.0f, -1.0f, 0.0f);
    assert(fabsf(round.intermission_duration - 3.0f) < 0.001f);
    assert(fabsf(round.round_duration - 12.0f) < 0.001f);
    assert(fabsf(round.results_duration - 2.0f) < 0.001f);

    nds_round_update(&round, 2.0f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 1);
    assert(round.disaster == NDS_DISASTER_EARTHQUAKE);
    assert(fabsf(nds_round_time_remaining(&round) - 12.0f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round)) < 0.001f);
    assert(!nds_round_is_countdown(&round));

    nds_round_update(&round, 6.0f);
    assert(fabsf(nds_round_phase_progress(&round) - 0.5f) < 0.001f);

    /* Elimination should immediately enter the results phase and preserve the result. */
    nds_round_finish(&round, 0);
    assert(round.state == NDS_ROUND_RESULTS);
    assert(round.player_survived == 0);
    assert(fabsf(nds_round_time_remaining(&round) - 2.0f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round)) < 0.001f);
    assert(nds_round_is_countdown(&round));

    /* Finishing a non-playing phase is a no-op. */
    nds_round_finish(&round, 1);
    assert(round.state == NDS_ROUND_RESULTS);
    assert(round.player_survived == 0);

    nds_round_update(&round, 2.0f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    /* A long frame/time jump must carry its unused time into the next state. */
    nds_round_update(&round, 5.5f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 2);
    assert(round.disaster == NDS_DISASTER_WINDSTORM);
    assert(fabsf(nds_round_time_remaining(&round) - 9.5f) < 0.001f);
    assert(strcmp(nds_disaster_type_name(round.disaster), "Windstorm") == 0);
    return 0;
}
