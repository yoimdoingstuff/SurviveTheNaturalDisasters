#include "engine/content/map_catalog.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    size_t count = nds_map_catalog_count();
    assert(count == 9);
    assert(strcmp(nds_map_catalog_name(0), "Happy Home") == 0);
    assert(strcmp(nds_map_catalog_path(0), "game/content/maps/happy_home.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(1), "Island Ruins") == 0);
    assert(strcmp(nds_map_catalog_path(1), "game/content/maps/island_ruins.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(2), "Downtown") == 0);
    assert(strcmp(nds_map_catalog_path(2), "game/content/maps/downtown.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(3), "Coastal Village") == 0);
    assert(strcmp(nds_map_catalog_path(3), "game/content/maps/coastal_village.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(4), "Arch Park") == 0);
    assert(strcmp(nds_map_catalog_path(4), "game/content/maps/arch_park.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(5), "Surf Central") == 0);
    assert(strcmp(nds_map_catalog_path(5), "game/content/maps/surf_central.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(6), "Fort Indestructible") == 0);
    assert(strcmp(nds_map_catalog_path(6), "game/content/maps/fort_indestructible.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(7), "Trailer Park") == 0);
    assert(strcmp(nds_map_catalog_path(7), "game/content/maps/trailer_park.ndsmap.json") == 0);
    assert(strcmp(nds_map_catalog_name(8), "Glass Office") == 0);
    assert(strcmp(nds_map_catalog_path(8), "game/content/maps/glass_office.ndsmap.json") == 0);
    assert(nds_map_catalog_path(count) == NULL);
    assert(nds_map_catalog_name(count) == NULL);
    assert(nds_map_catalog_select(1) == 0);
    assert(nds_map_catalog_select(2) == 1);
    assert(nds_map_catalog_select(9) == 8);
    assert(nds_map_catalog_select(10) == 0);
    assert(nds_map_catalog_select(100) == 0);
    return 0;
}
