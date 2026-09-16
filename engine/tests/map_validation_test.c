#include "engine/content/map_validation.h"
#include "engine/scene/instance.h"
#include "engine/scene/part.h"
#include <assert.h>
#include <math.h>

static void set_props(nds_instance* part, nds_vec3 position, nds_vec3 size, int collide)
{
    nds_part_properties p = {0};
    p.position = position;
    p.size = size;
    p.anchored = 1;
    p.can_collide = (uint8_t)collide;
    p.visible = 1;
    p.color_rgba = 0xffffffffu;
    assert(nds_part_set_properties(part, &p) == NDS_OK);
}

static nds_instance* make_valid_scene(void)
{
    nds_instance* root = nds_instance_create(NDS_CLASS_DATAMODEL, "ValidationWorld");
    nds_instance* spawn = nds_instance_create(NDS_CLASS_SPAWN_POINT, "Spawn");
    size_t i;
    assert(root && spawn);
    set_props(spawn, (nds_vec3){0, 2, 0}, (nds_vec3){2, 1, 2}, 0);
    assert(nds_instance_set_parent(spawn, root) == NDS_OK);
    for (i = 0; i < 8; ++i) {
        char name[32];
        nds_instance* part;
        (void)snprintf(name, sizeof(name), "Block%u", (unsigned)i);
        part = nds_instance_create(NDS_CLASS_PART, name);
        assert(part);
        set_props(part, (nds_vec3){(float)i * 3.0f, 0, 0}, (nds_vec3){2, 2, 2}, 1);
        assert(nds_instance_set_parent(part, root) == NDS_OK);
    }
    return root;
}

int main(void)
{
    nds_map_validation_report report;
    nds_instance* scene = make_valid_scene();
    nds_instance* button = nds_instance_create(NDS_CLASS_PART, "LaunchButton");
    nds_part_properties p;

    assert(scene && button);
    set_props(button, (nds_vec3){0, 2.2f, 0}, (nds_vec3){1, .25f, 1}, 0);
    assert(nds_instance_set_parent(button, scene) == NDS_OK);

    assert(nds_map_validate(scene, &report) == NDS_OK);
    assert(report.part_count == 9);
    assert(report.spawn_count == 1);
    assert(report.invalid_part_count == 0);
    assert(report.unsafe_spawn_count == 0);
    assert(report.non_granular_part_count == 0);
    assert(report.interaction_count == 1);
    assert(!nds_map_validation_has_errors(&report));

    p.position = (nds_vec3){0, 100, 0};
    assert(nds_part_get_properties(button, &p) == NDS_OK);
    p.size = (nds_vec3){100, 2, 2};
    assert(nds_part_set_properties(button, &p) == NDS_OK);
    assert(nds_map_validate(scene, &report) == NDS_OK);
    assert(report.non_granular_part_count == 1);
    assert(nds_map_validation_has_errors(&report));

    p.position = (nds_vec3){0, 2.2f, 0};
    p.size = (nds_vec3){0, 2, 2};
    assert(nds_part_set_properties(button, &p) == NDS_OK);
    assert(nds_map_validate(scene, &report) == NDS_OK);
    assert(report.invalid_part_count == 1);

    nds_instance_destroy(scene);
    {
        nds_instance* bad = nds_instance_create(NDS_CLASS_DATAMODEL, "BadWorld");
        nds_instance* bad_spawn = nds_instance_create(NDS_CLASS_SPAWN_POINT, "Spawn");
        assert(bad && bad_spawn);
        set_props(bad_spawn, (nds_vec3){0, 20, 0}, (nds_vec3){2, 1, 2}, 0);
        assert(nds_instance_set_parent(bad_spawn, bad) == NDS_OK);
        assert(nds_map_validate(bad, &report) == NDS_OK);
        assert(report.spawn_count == 1);
        assert(report.unsafe_spawn_count == 1);
        assert(nds_map_validation_has_errors(&report));
        nds_instance_destroy(bad);
    }
    return 0;
}
