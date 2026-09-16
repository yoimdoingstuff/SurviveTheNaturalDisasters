#include "engine/content/map_catalog.h"

/* Render Test remains available as a development asset, but is not part of the
 * normal survival rotation. */
static const char* const MAP_PATHS[] = {
    "game/content/maps/happy_home.ndsmap.json",
    "game/content/maps/island_ruins.ndsmap.json",
    "game/content/maps/downtown.ndsmap.json",
    "game/content/maps/coastal_village.ndsmap.json",
    "game/content/maps/arch_park.ndsmap.json",
    "game/content/maps/surf_central.ndsmap.json",
    "game/content/maps/fort_indestructible.ndsmap.json",
    "game/content/maps/trailer_park.ndsmap.json",
    "game/content/maps/glass_office.ndsmap.json",
    "game/content/maps/launch_land.ndsmap.json"
};

static const char* const MAP_NAMES[] = {
    "Happy Home",
    "Island Ruins",
    "Downtown",
    "Coastal Village",
    "Arch Park",
    "Surf Central",
    "Fort Indestructible",
    "Trailer Park",
    "Glass Office",
    "Launch Land"
};

static size_t selected_map = 0;
static int has_manual_selection = 0;

size_t nds_map_catalog_count(void)
{
    return sizeof(MAP_PATHS) / sizeof(MAP_PATHS[0]);
}

const char* nds_map_catalog_path(size_t index)
{
    return index < nds_map_catalog_count() ? MAP_PATHS[index] : NULL;
}

const char* nds_map_catalog_name(size_t index)
{
    return index < nds_map_catalog_count() ? MAP_NAMES[index] : NULL;
}

void nds_map_catalog_set_selected(size_t index)
{
    if (index < nds_map_catalog_count()) {
        selected_map = index;
        has_manual_selection = 1;
    }
}

size_t nds_map_catalog_selected(void)
{
    return selected_map;
}

size_t nds_map_catalog_select(size_t round_number)
{
    size_t count = nds_map_catalog_count();
    if (!count) return 0u;
    if (has_manual_selection) return selected_map % count;
    return (round_number - 1u) % count;
}
