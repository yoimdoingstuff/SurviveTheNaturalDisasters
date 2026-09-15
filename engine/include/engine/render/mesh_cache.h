#ifndef NDS_ENGINE_MESH_CACHE_H
#define NDS_ENGINE_MESH_CACHE_H

#include "engine/core/types.h"
#include "engine/render/mesh.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_mesh_cache nds_mesh_cache;

nds_result nds_mesh_cache_create(nds_mesh_cache** out_cache, size_t capacity);
void nds_mesh_cache_destroy(nds_mesh_cache* cache);

/* Returns a stable, cache-owned mesh pointer. Repeated requests for the same
 * path reuse the decoded mesh rather than reparsing it every frame. */
nds_result nds_mesh_cache_get(nds_mesh_cache* cache, const char* path,
                              const nds_mesh** out_mesh);
void nds_mesh_cache_clear(nds_mesh_cache* cache);
size_t nds_mesh_cache_count(const nds_mesh_cache* cache);

#ifdef __cplusplus
}
#endif

#endif
