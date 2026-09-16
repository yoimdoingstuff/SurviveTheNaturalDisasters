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
    assert(fabsf(nds_round_time_remaining(&round) - 10.0f) < 0.001f);

    nds_round_update(&round, 9.9f);
    assert(round.state == NDS_ROUND_INTERMISSION);
    assert(fabsf(nds_round_time_remaining(&round) - 0.1f) < 0.001f);

    nds_round_update(&round, 0.1f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 1);
    assert(round.disaster == NDS_DISASTER_EARTHQUAKE);
    assert(fabsf(nds_round_time_remaining(&round) - 60.0f) < 0.001f);

    nds_round_update(&round, 60.0f);
    assert(round.state == NDS_ROUND_RESULTS);
    assert(fabsf(nds_round_time_remaining(&round) - 5.0f) < 0.001f);

    nds_round_update(&round, 5.0f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    /* A long frame/time jump must carry its unused time into the next state. */
    nds_round_update(&round, 10.5f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 2);
    assert(round.disaster == NDS_DISASTER_WINDSTORM);
    assert(fabsf(nds_round_time_remaining(&round) - 59.5f) < 0.001f);
    assert(strcmp(nds_disaster_type_name(round.disaster), "Windstorm") == 0);
    return 0;
}
