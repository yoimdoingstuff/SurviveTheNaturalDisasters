#ifndef NDS_ENGINE_MAP_LOADER_H
#define NDS_ENGINE_MAP_LOADER_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"
#include "engine/render/mesh_cache.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Loads the project-owned nds-map JSON produced by tools/map_importer.
 * The reader intentionally supports only the stable fields needed by the
 * runtime, keeping it small enough for legacy iOS/Android targets. */
nds_result nds_map_load_json(const char* path, nds_instance** out_root);

/* Same loader for an already-buffered UTF-8 JSON document. */
nds_result nds_map_load_json_text(const char* text, nds_instance** out_root);

/* Resolves project-owned mesh paths retained by the map loader. Mesh pointers
 * are borrowed from cache and therefore require the cache to outlive root. */
nds_result nds_map_resolve_meshes(nds_instance* root, nds_mesh_cache* cache);

#ifdef __cplusplus
}
#endif

#endif
