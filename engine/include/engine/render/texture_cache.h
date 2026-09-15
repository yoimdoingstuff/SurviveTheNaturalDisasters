#ifndef NDS_ENGINE_TEXTURE_CACHE_H
#define NDS_ENGINE_TEXTURE_CACHE_H

#include "engine/core/types.h"
#include "engine/render/texture.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_texture_cache nds_texture_cache;

nds_result nds_texture_cache_create(nds_texture_cache** out_cache, size_t capacity);
void nds_texture_cache_destroy(nds_texture_cache* cache);
nds_result nds_texture_cache_get(nds_texture_cache* cache, const char* path,
                                 const nds_texture** out_texture);
void nds_texture_cache_clear(nds_texture_cache* cache);
size_t nds_texture_cache_count(const nds_texture_cache* cache);

#ifdef __cplusplus
}
#endif

#endif
