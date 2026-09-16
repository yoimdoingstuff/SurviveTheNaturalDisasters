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

    nds_round_update(&round, 2.5f);
    assert(fabsf(nds_round_time_remaining(&round) - 2.5f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round) - 0.5f) < 0.001f);

    nds_round_update(&round, 2.5f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 1);
    assert(round.disaster == NDS_DISASTER_EARTHQUAKE);
    assert(fabsf(nds_round_time_remaining(&round) - 60.0f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round)) < 0.001f);
    assert(!nds_round_is_countdown(&round));

    nds_round_update(&round, 30.0f);
    assert(fabsf(nds_round_phase_progress(&round) - 0.5f) < 0.001f);

    /* Elimination should immediately enter the results phase and preserve the result. */
    nds_round_finish(&round, 0);
    assert(round.state == NDS_ROUND_RESULTS);
    assert(round.player_survived == 0);
    assert(fabsf(nds_round_time_remaining(&round) - 5.0f) < 0.001f);
    assert(fabsf(nds_round_phase_progress(&round)) < 0.001f);
    assert(nds_round_is_countdown(&round));

    /* Finishing a non-playing phase is a no-op. */
    nds_round_finish(&round, 1);
    assert(round.state == NDS_ROUND_RESULTS);
    assert(round.player_survived == 0);

    nds_round_update(&round, 5.0f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    /* A long frame/time jump must carry its unused time into the next state. */
    nds_round_update(&round, 10.5f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 2);
    assert(round.disaster == NDS_DISASTER_WINDSTORM);
    assert(fabsf(nds_round_time_remaining(&round) - 54.5f) < 0.001f);
    assert(strcmp(nds_disaster_type_name(round.disaster), "Windstorm") == 0);
    return 0;
}
