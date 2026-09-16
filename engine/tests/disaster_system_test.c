#include "engine/game/disaster_system.h"
#include "engine/scene/part.h"

#include <assert.h>
#include <math.h>

static nds_instance* make_unanchored_part(nds_instance* scene)
{
    nds_instance* part = nds_instance_create(NDS_CLASS_PART, "WindTarget");
    nds_part_properties props = {0};
    assert(part != NULL);
    props.position = (nds_vec3){0.0f, 3.0f, 0.0f};
    props.size = (nds_vec3){1.0f, 1.0f, 1.0f};
    props.rotation = (nds_vec3){0.0f, 0.0f, 0.0f};
    props.anchored = 0;
    props.can_collide = 1;
    props.visible = 1;
    assert(nds_part_set_properties(part, &props) == NDS_OK);
    assert(nds_instance_set_parent(part, scene) == NDS_OK);
    return part;
}

int main(void)
{
    nds_disaster_system system;
    nds_player_controller player;
    nds_instance* scene = nds_instance_create(NDS_CLASS_DATAMODEL, "Scene");
    nds_instance* part;
    nds_vec3 before, after;

    assert(scene != NULL);
    part = make_unanchored_part(scene);
    nds_player_init(&player, scene);
    nds_disaster_system_init(&system);
    assert(system.active == 0);

    nds_disaster_system_start(&system, NDS_DISASTER_EARTHQUAKE);
    assert(system.active == 1);
    assert(system.active_type == NDS_DISASTER_EARTHQUAKE);
    assert(system.earthquake.active == 0);
    assert(nds_disaster_is_warning(&system));
    assert(fabsf(nds_disaster_warning_remaining(&system) - 3.0f) < 0.001f);

    nds_disaster_system_update(&system, &player, scene, 1.0f);
    assert(nds_disaster_is_warning(&system));
    assert(fabsf(nds_disaster_warning_remaining(&system) - 2.0f) < 0.001f);
    assert(system.earthquake.elapsed == 0.0f);

    nds_disaster_system_update(&system, &player, scene, 2.0f);
    assert(!nds_disaster_is_warning(&system));
    assert(system.earthquake.active == 1);
    assert(system.earthquake.elapsed < 0.001f);

    nds_disaster_system_stop(&system, scene);
    assert(system.active == 0);
    assert(system.earthquake.active == 0);

    assert(nds_part_get_position(part, &before) == NDS_OK);
    nds_disaster_system_start(&system, NDS_DISASTER_WINDSTORM);
    assert(system.active == 1);
    assert(system.active_type == NDS_DISASTER_WINDSTORM);
    assert(system.windstorm.active == 0);

    nds_disaster_system_update(&system, &player, scene, 3.0f);
    assert(!nds_disaster_is_warning(&system));
    assert(system.windstorm.active == 1);
    assert(system.windstorm.elapsed < 0.001f);
    assert(nds_part_get_position(part, &after) == NDS_OK);
    assert(fabsf(after.x - before.x) + fabsf(after.z - before.z) < 0.001f);

    nds_disaster_system_update(&system, &player, scene, 0.5f);
    assert(system.windstorm.elapsed > 0.49f);
    assert(nds_part_get_position(part, &after) == NDS_OK);
    assert(fabsf(after.x - before.x) + fabsf(after.z - before.z) > 0.0f);
    {
        nds_part_properties props;
        assert(nds_part_get_properties(part, &props) == NDS_OK);
        assert(fabsf(props.rotation.x) + fabsf(props.rotation.y) + fabsf(props.rotation.z) > 0.0f);
    }

    nds_disaster_system_stop(&system, scene);
    assert(system.active == 0);
    assert(system.windstorm.active == 0);

    nds_instance_destroy(scene);
    return 0;
}
