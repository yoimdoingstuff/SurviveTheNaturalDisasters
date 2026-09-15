#include "engine/game/round.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    nds_round round;
    nds_round_init(&round);

    assert(round.state == NDS_ROUND_INTERMISSION);
    assert(round.round_number == 0);
    assert(round.disaster == NDS_DISASTER_EARTHQUAKE);
    assert(strcmp(nds_round_state_name(round.state), "Intermission") == 0);
    assert(strcmp(nds_disaster_type_name(round.disaster), "Earthquake") == 0);

    nds_round_update(&round, 9.9f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    nds_round_update(&round, 0.1f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 1);
    assert(round.disaster == NDS_DISASTER_EARTHQUAKE);

    nds_round_update(&round, 60.0f);
    assert(round.state == NDS_ROUND_RESULTS);

    nds_round_update(&round, 5.0f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    nds_round_update(&round, 10.0f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 2);
    assert(round.disaster == NDS_DISASTER_WINDSTORM);
    assert(strcmp(nds_disaster_type_name(round.disaster), "Windstorm") == 0);
    return 0;
}
