#include "engine/game/disaster_system.h"
#include "engine/scene/part.h"

#include <assert.h>
#include <math.h>
#include <string.h>

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

static nds_instance* find_named_part(nds_instance* scene, const char* name)
{
    size_t i;
    if (!scene || !name) return NULL;
    for (i = 0; i < nds_instance_child_count(scene); ++i) {
        nds_instance* child = (nds_instance*)nds_instance_child_at(scene, i);
        const char* child_name;
        nds_instance* nested;
        if (!child) continue;
        child_name = nds_instance_get_name(child);
        if (child_name && strcmp(child_name, name) == 0) return child;
        nested = find_named_part(child, name);
        if (nested) return nested;
    }
    return NULL;
}

int main(void)
{
    nds_disaster_system system;
    nds_disaster_settings settings;
    nds_player_controller player;
    nds_instance* scene = nds_instance_create(NDS_CLASS_DATAMODEL, "Scene");
    nds_instance* part;
    nds_instance* island_base;
    nds_instance* island_shore;
    nds_vec3 before, after, island_base_before, island_base_after, island_shore_before, island_shore_after;

    assert(scene != NULL);

    /* Every gameplay scene gets a persistent world shell: water, island base, and shore. */
    island_base = nds_instance_create(NDS_CLASS_PART, "IslandBase");
    island_shore = nds_instance_create(NDS_CLASS_PART, "IslandShore");
    assert(island_base != NULL);
    assert(island_shore != NULL);
    assert(nds_instance_set_parent(island_base, scene) == NDS_OK);
    assert(nds_instance_set_parent(island_shore, scene) == NDS_OK);
    {
        nds_part_properties props = {0};
        props.position = (nds_vec3){0.0f, -1.0f, 0.0f};
        props.size = (nds_vec3){34.0f, 1.0f, 28.0f};
        props.anchored = 1;
        props.can_collide = 1;
        props.visible = 1;
        assert(nds_part_set_properties(island_base, &props) == NDS_OK);
        props.position = (nds_vec3){0.0f, -0.4f, 0.0f};
        props.size = (nds_vec3){37.0f, 0.45f, 31.0f};
        assert(nds_part_set_properties(island_shore, &props) == NDS_OK);
    }

    part = make_unanchored_part(scene);
    nds_player_init(&player, scene);
    nds_disaster_system_init(&system);
    assert(system.active == 0);
    assert(system.warning_duration - 3.0f < 0.001f && 3.0f - system.warning_duration < 0.001f);
    assert(system.environment.wind_intensity == 0.0f);
    assert(system.environment.shake_intensity == 0.0f);
    assert(system.environment.debris_intensity == 0.0f);
    assert(system.environment.water_intensity == 0.0f);
    assert(system.environment.fire_intensity == 0.0f);
    assert(system.environment.sky_darkness == 0.0f);

    settings.warning_duration = 1.5f;
    settings.earthquake_pulse_interval = 2.0f;
    settings.windstorm_gust_interval = 1.0f;
    settings.windstorm_strength = 24.0f;
    nds_disaster_system_set_settings(&system, &settings);
    assert(fabsf(system.warning_duration - 1.5f) < 0.001f);
    assert(fabsf(system.earthquake.pulse_interval - 2.0f) < 0.001f);
    assert(fabsf(system.windstorm.gust_interval - 1.0f) < 0.001f);
    assert(fabsf(system.windstorm.strength - 24.0f) < 0.001f);

    /* Invalid settings must not disable a disaster or create zero-length timers. */
    settings.warning_duration = 0.0f;
    settings.earthquake_pulse_interval = -1.0f;
    settings.windstorm_gust_interval = 0.0f;
    settings.windstorm_strength = -4.0f;
    nds_disaster_system_set_settings(&system, &settings);
    assert(fabsf(system.warning_duration - 1.5f) < 0.001f);
    assert(fabsf(system.earthquake.pulse_interval - 2.0f) < 0.001f);
    assert(fabsf(system.windstorm.gust_interval - 1.0f) < 0.001f);
    assert(fabsf(system.windstorm.strength - 24.0f) < 0.001f);

    nds_part_get_position(island_base, &island_base_before);
    nds_part_get_position(island_shore, &island_shore_before);
    nds_disaster_system_start(&system, NDS_DISASTER_EARTHQUAKE);
    assert(system.active == 1);
    assert(system.active_type == NDS_DISASTER_EARTHQUAKE);
    assert(system.earthquake.active == 0);
    assert(nds_disaster_is_warning(&system));
    assert(fabsf(nds_disaster_warning_remaining(&system) - 1.5f) < 0.001f);
    assert(system.environment.sky_darkness > 0.0f);

    nds_disaster_system_update(&system, &player, scene, 1.0f);
    assert(nds_disaster_is_warning(&system));
    assert(fabsf(nds_disaster_warning_remaining(&system) - 0.5f) < 0.001f);
    assert(system.earthquake.elapsed == 0.0f);
    assert(system.environment.sky_darkness > 0.0f);

    nds_disaster_system_update(&system, &player, scene, 1.0f);
    assert(!nds_disaster_is_warning(&system));
    assert(system.earthquake.active == 1);
    assert(fabsf(system.earthquake.elapsed - 0.5f) < 0.001f);
    assert(system.environment.shake_intensity > 0.0f);
    assert(system.environment.sky_darkness > 0.0f);

    /* Earthquakes may shake map structures, but the persistent island never moves. */
    nds_part_get_position(island_base, &island_base_after);
    nds_part_get_position(island_shore, &island_shore_after);
    assert(fabsf(island_base_after.x - island_base_before.x) < 0.0001f);
    assert(fabsf(island_base_after.z - island_base_before.z) < 0.0001f);
    assert(fabsf(island_shore_after.x - island_shore_before.x) < 0.0001f);
    assert(fabsf(island_shore_after.z - island_shore_before.z) < 0.0001f);

    nds_disaster_system_stop(&system, scene);
    assert(system.active == 0);
    assert(system.earthquake.active == 0);
    assert(system.environment.shake_intensity == 0.0f);
    assert(system.environment.debris_intensity == 0.0f);
    assert(system.environment.sky_darkness == 0.0f);

    assert(nds_part_get_position(part, &before) == NDS_OK);
    nds_disaster_system_start(&system, NDS_DISASTER_WINDSTORM);
    assert(system.active == 1);
    assert(system.active_type == NDS_DISASTER_WINDSTORM);
    assert(system.windstorm.active == 0);

    nds_disaster_system_update(&system, &player, scene, 1.5f);
    assert(!nds_disaster_is_warning(&system));
    assert(system.windstorm.active == 1);
    assert(fabsf(system.windstorm.elapsed - 0.0f) < 0.001f);
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
    assert(system.environment.wind_intensity > 0.0f);
    assert(system.environment.debris_intensity > 0.0f);
    assert(system.environment.sky_darkness > 0.0f);

    nds_disaster_system_stop(&system, scene);
    assert(system.active == 0);
    assert(system.windstorm.active == 0);
    assert(system.environment.wind_intensity == 0.0f);
    assert(system.environment.debris_intensity == 0.0f);
    assert(system.environment.sky_darkness == 0.0f);

    nds_instance_destroy(scene);
    (void)find_named_part;
    return 0;
}
