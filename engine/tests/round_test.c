#include "engine/game/round.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    nds_round round;
    nds_round_init(&round);

    assert(round.state == NDS_ROUND_INTERMISSION);
    assert(round.round_number == 0);
    assert(strcmp(nds_round_state_name(round.state), "Intermission") == 0);

    nds_round_update(&round, 9.9f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    nds_round_update(&round, 0.1f);
    assert(round.state == NDS_ROUND_PLAYING);
    assert(round.round_number == 1);

    nds_round_update(&round, 60.0f);
    assert(round.state == NDS_ROUND_RESULTS);
    assert(round.player_survived == 1);

    nds_round_update(&round, 5.0f);
    assert(round.state == NDS_ROUND_INTERMISSION);

    return 0;
}
