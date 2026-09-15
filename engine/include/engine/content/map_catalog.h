#ifndef NDS_ENGINE_MAP_CATALOG_H
#define NDS_ENGINE_MAP_CATALOG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Built-in catalog of maps shipped with the current project build. */
size_t nds_map_catalog_count(void);
const char* nds_map_catalog_path(size_t index);
const char* nds_map_catalog_name(size_t index);

/* Deterministic round-based selection. This is deliberately simple so the
 * same map order can later be shared by offline and host-authoritative modes. */
size_t nds_map_catalog_select(size_t round_number);

#ifdef __cplusplus
}
#endif

#endif
