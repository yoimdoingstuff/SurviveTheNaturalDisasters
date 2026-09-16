#include "engine/content/map_catalog.h"
#include "engine/content/map_loader.h"
#include "engine/content/map_validation.h"
#include "engine/scene/instance.h"

#include <assert.h>
#include <stdio.h>

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
        assert(report.part_count >= 8);
        assert(report.spawn_count >= 1);
        assert(report.invalid_part_count == 0);
        assert(report.unsafe_spawn_count == 0);
        assert(report.non_granular_part_count == 0);
        assert(!nds_map_validation_has_errors(&report));

        printf("validated %s: parts=%zu spawns=%zu interactions=%zu\n",
               path,
               report.part_count,
               report.spawn_count,
               report.interaction_count);
        nds_instance_destroy(root);
    }

    return 0;
}
