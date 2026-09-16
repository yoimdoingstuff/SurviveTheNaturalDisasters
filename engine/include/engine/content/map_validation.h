#ifndef NDS_ENGINE_CONTENT_MAP_VALIDATION_H
#define NDS_ENGINE_CONTENT_MAP_VALIDATION_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_map_validation_report {
    size_t part_count;
    size_t spawn_count;
    size_t invalid_part_count;
    size_t unsafe_spawn_count;
    size_t non_granular_part_count;
    size_t interaction_count;
} nds_map_validation_report;

void nds_map_validation_report_init(nds_map_validation_report* report);
nds_result nds_map_validate(const nds_instance* scene, nds_map_validation_report* report);
int nds_map_validation_has_errors(const nds_map_validation_report* report);

#ifdef __cplusplus
}
#endif

#endif
