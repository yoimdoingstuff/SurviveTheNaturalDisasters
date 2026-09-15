#include "engine/game/disaster_system.h"

void nds_disaster_system_init(nds_disaster_system* system)
{
    if (!system) return;
    nds_earthquake_init(&system->earthquake);
    system->active_type = NDS_DISASTER_EARTHQUAKE;
    system->active = 0;
}

void nds_disaster_system_start(nds_disaster_system* system, nds_disaster_type type)
{
    if (!system) return;
    system->active_type = type;
    system->active = 0;

    switch (type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_start(&system->earthquake);
        system->active = 1;
        break;
    default:
        break;
    }
}

void nds_disaster_system_update(nds_disaster_system* system,
                                nds_player_controller* player,
                                nds_instance* scene, float dt)
{
    if (!system || !system->active) return;

    switch (system->active_type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_update(&system->earthquake, player, scene, dt);
        break;
    default:
        system->active = 0;
        break;
    }
}

void nds_disaster_system_stop(nds_disaster_system* system, nds_instance* scene)
{
    if (!system || !system->active) return;

    switch (system->active_type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_stop(&system->earthquake, scene);
        break;
    default:
        break;
    }

    system->active = 0;
}
