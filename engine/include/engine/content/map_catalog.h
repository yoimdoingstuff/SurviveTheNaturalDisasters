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

/* The menu can override the normal round rotation with a selected map. */
void nds_map_catalog_set_selected(size_t index);
size_t nds_map_catalog_selected(void);

/* Deterministic round-based selection unless the menu has selected a map. */
size_t nds_map_catalog_select(size_t round_number);

#ifdef __cplusplus
}
#endif

#endif
