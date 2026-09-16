#include "engine/content/map_catalog.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    size_t count = nds_map_catalog_count();
    assert(count == 3);
    assert(strcmp(nds_map_catalog_name(0), "Island Ruins") == 0);
    assert(strcmp(nds_map_catalog_path(0), "game/content/maps/island_ruins.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(1), "Downtown") == 0);
    assert(strcmp(nds_map_catalog_path(1), "game/content/maps/downtown.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(2), "Coastal Village") == 0);
    assert(strcmp(nds_map_catalog_path(2), "game/content/maps/coastal_village.ndsmap.json") == 0);
    assert(nds_map_catalog_path(count) == NULL);
    assert(nds_map_catalog_name(count) == NULL);
    assert(nds_map_catalog_select(1) == 0);
    assert(nds_map_catalog_select(2) == 1);
    assert(nds_map_catalog_select(3) == 2);
    assert(nds_map_catalog_select(4) == 0);
    assert(nds_map_catalog_select(100) == 0);
    return 0;
}
