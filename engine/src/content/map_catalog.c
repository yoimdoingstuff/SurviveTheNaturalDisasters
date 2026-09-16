#include "engine/content/map_catalog.h"

static const char* const MAP_PATHS[] = {
    "game/content/maps/render_test.ndsmap.json",
    "game/content/maps/island_ruins.ndsmap.json",
    "game/content/maps/downtown.ndsmap.json"
};

static const char* const MAP_NAMES[] = {
    "Render Test",
    "Island Ruins",
    "Downtown"
};

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

size_t nds_map_catalog_select(size_t round_number)
{
    size_t count = nds_map_catalog_count();
    return count ? (round_number - 1u) % count : 0u;
}
