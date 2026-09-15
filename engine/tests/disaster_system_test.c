#include "engine/game/disaster_system.h"

#include <assert.h>

int main(void)
{
    nds_disaster_system system;
    nds_player_controller player;
    nds_instance* scene = nds_instance_create("Scene", NDS_CLASS_DATAMODEL);

    assert(scene != NULL);
    nds_player_init(&player, scene);
    nds_disaster_system_init(&system);
    assert(system.active == 0);

    nds_disaster_system_start(&system, NDS_DISASTER_EARTHQUAKE);
    assert(system.active == 1);
    assert(system.active_type == NDS_DISASTER_EARTHQUAKE);
    assert(system.earthquake.active == 1);

    nds_disaster_system_update(&system, &player, scene, 1.0f);
    assert(system.earthquake.elapsed > 0.0f);

    nds_disaster_system_stop(&system, scene);
    assert(system.active == 0);
    assert(system.earthquake.active == 0);

    nds_instance_destroy(scene);
    return 0;
}
