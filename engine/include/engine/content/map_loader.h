#ifndef NDS_ENGINE_MAP_LOADER_H
#define NDS_ENGINE_MAP_LOADER_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"
#include "engine/render/mesh_cache.h"
#include "engine/render/texture_cache.h"

#ifdef __cplusplus
extern "C" {
#endif

nds_result nds_map_load_json(const char* path, nds_instance** out_root);
nds_result nds_map_load_json_text(const char* text, nds_instance** out_root);

/* Mesh/texture pointers are borrowed from their caches and therefore require
 * the caches to outlive the loaded instance tree. */
nds_result nds_map_resolve_meshes(nds_instance* root, nds_mesh_cache* cache);
nds_result nds_map_resolve_textures(nds_instance* root, nds_texture_cache* cache);

#ifdef __cplusplus
}
#endif

#endif
