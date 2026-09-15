#include "engine/content/map_catalog.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    size_t count = nds_map_catalog_count();
    assert(count > 0);
    assert(nds_map_catalog_path(0) != NULL);
    assert(nds_map_catalog_name(0) != NULL);
    assert(strcmp(nds_map_catalog_name(0), "Render Test") == 0);
    assert(nds_map_catalog_path(count) == NULL);
    assert(nds_map_catalog_name(count) == NULL);
    assert(nds_map_catalog_select(1) == 0);
    assert(nds_map_catalog_select(100) == 0);
    return 0;
}
