#include "engine/game/disaster.h"
#include <assert.h>

int main(void)
{
    nds_earthquake earthquake;
    nds_player_controller player = {0};

    nds_earthquake_init(&earthquake);
    assert(!earthquake.active);
    assert(earthquake.pulse_interval == 4.0f);

    player.health = 100.0f;
    player.alive = 1;
    player.grounded = 0;
    nds_earthquake_start(&earthquake);
    assert(earthquake.active);

    nds_earthquake_update(&earthquake, &player, 4.0f);
    assert(earthquake.pulse_count == 1);
    assert(player.health == 75.0f);
    assert(player.alive);

    nds_earthquake_stop(&earthquake);
    assert(!earthquake.active);
    return 0;
}
