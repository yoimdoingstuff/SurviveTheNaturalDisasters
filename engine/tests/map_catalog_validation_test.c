#include "engine/content/map_catalog.h"
#include "engine/content/map_loader.h"
#include "engine/content/map_validation.h"
#include "engine/scene/instance.h"
#include "engine/scene/part.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static nds_instance* find_named(nds_instance* node, const char* name)
{
    size_t i;
    if (!node || !name) return NULL;
    for (i = 0; i < nds_instance_child_count(node); ++i) {
        nds_instance* child = (nds_instance*)nds_instance_child_at(node, i);
        const char* child_name;
        nds_instance* nested;
        if (!child) continue;
        child_name = nds_instance_get_name(child);
        if (child_name && strcmp(child_name, name) == 0) return child;
        nested = find_named(child, name);
        if (nested) return nested;
    }
    return NULL;
}

static void assert_persistent_island(nds_instance* root)
{
    const char* required[] = {"Ocean", "IslandBase", "IslandShore", "IslandViewTowerDeck"};
    size_t i;
    for (i = 0; i < sizeof(required) / sizeof(required[0]); ++i) {
        nds_instance* part = find_named(root, required[i]);
        nds_part_properties props;
        assert(part != NULL);
        assert(nds_instance_get_class(part) == NDS_CLASS_PART);
        assert(nds_part_get_properties(part, &props) == NDS_OK);
        assert(props.anchored == 1);
        assert(props.visible == 1);
    }

    {
        nds_instance* ocean = find_named(root, "Ocean");
        nds_instance* island_base = find_named(root, "IslandBase");
        nds_instance* island_shore = find_named(root, "IslandShore");
        nds_instance* tower_base = find_named(root, "IslandViewTowerBase");
        nds_instance* tower_deck = find_named(root, "IslandViewTowerDeck");
        nds_instance* tower_roof = find_named(root, "IslandViewTowerRoof");
        nds_part_properties ocean_props, base_props, shore_props, tower_base_props, tower_deck_props, tower_roof_props;
        assert(nds_part_get_properties(ocean, &ocean_props) == NDS_OK);
        assert(nds_part_get_properties(island_base, &base_props) == NDS_OK);
        assert(nds_part_get_properties(island_shore, &shore_props) == NDS_OK);
        assert(nds_part_get_properties(tower_base, &tower_base_props) == NDS_OK);
        assert(nds_part_get_properties(tower_deck, &tower_deck_props) == NDS_OK);
        assert(nds_part_get_properties(tower_roof, &tower_roof_props) == NDS_OK);
        assert(ocean_props.can_collide == 0);
        assert(base_props.can_collide == 1);
        assert(shore_props.can_collide == 1);
        assert(tower_base_props.can_collide == 1);
        assert(tower_deck_props.can_collide == 1);
        assert(tower_roof_props.can_collide == 1);
        assert(base_props.size.x > 0.0f && base_props.size.z > 0.0f);
        assert(shore_props.size.x > base_props.size.x);
        assert(shore_props.size.z > base_props.size.z);
        assert(tower_deck_props.position.y > 5.0f);
        assert(tower_deck_props.size.x >= 5.0f && tower_deck_props.size.z >= 5.0f);
        assert(tower_roof_props.position.y > tower_deck_props.position.y);
    }
}

int main(void)
{
    size_t i;

    for (i = 0; i < nds_map_catalog_count(); ++i) {
        const char* path = nds_map_catalog_path(i);
        nds_instance* root = NULL;
        nds_map_validation_report report;

        assert(path != NULL);
        assert(nds_map_load_json(path, &root) == NDS_OK);
        assert(root != NULL);
        assert(nds_map_validate(root, &report) == NDS_OK);
        assert(report.part_count >= 20);
        assert(report.spawn_count >= 1);
        assert(report.invalid_part_count == 0);
        assert(report.unsafe_spawn_count == 0);
        assert(report.non_granular_part_count == 0);
        assert(!nds_map_validation_has_errors(&report));
        assert_persistent_island(root);

        printf("validated %s: parts=%zu spawns=%zu interactions=%zu\n",
               path,
               report.part_count,
               report.spawn_count,
               report.interaction_count);
        nds_instance_destroy(root);
    }

    return 0;
}
