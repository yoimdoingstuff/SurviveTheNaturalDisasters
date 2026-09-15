#include "engine/render/texture_cache.h"

#include <stdlib.h>
#include <string.h>

typedef struct nds_texture_cache_entry {
    char* path;
    nds_texture texture;
    uint8_t used;
} nds_texture_cache_entry;

struct nds_texture_cache {
    nds_texture_cache_entry* entries;
    size_t capacity;
    size_t count;
};

static char* copy_string(const char* value)
{
    size_t length;
    char* copy;
    if (!value) return NULL;
    length = strlen(value);
    if (length == SIZE_MAX) return NULL;
    copy = (char*)malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, value, length + 1);
    return copy;
}

nds_result nds_texture_cache_create(nds_texture_cache** out_cache, size_t capacity)
{
    nds_texture_cache* cache;
    if (!out_cache || capacity == 0) return NDS_ERR_INVALID_ARG;
    *out_cache = NULL;
    cache = (nds_texture_cache*)calloc(1, sizeof(*cache));
    if (!cache) return NDS_ERR_UNKNOWN;
    cache->entries = (nds_texture_cache_entry*)calloc(capacity, sizeof(*cache->entries));
    if (!cache->entries) { free(cache); return NDS_ERR_UNKNOWN; }
    cache->capacity = capacity;
    *out_cache = cache;
    return NDS_OK;
}

void nds_texture_cache_clear(nds_texture_cache* cache)
{
    size_t i;
    if (!cache) return;
    for (i = 0; i < cache->capacity; ++i) {
        if (!cache->entries[i].used) continue;
        free(cache->entries[i].path);
        nds_texture_destroy(&cache->entries[i].texture);
        cache->entries[i] = (nds_texture_cache_entry){0};
    }
    cache->count = 0;
}

void nds_texture_cache_destroy(nds_texture_cache* cache)
{
    if (!cache) return;
    nds_texture_cache_clear(cache);
    free(cache->entries);
    free(cache);
}

nds_result nds_texture_cache_get(nds_texture_cache* cache, const char* path,
                                 const nds_texture** out_texture)
{
    size_t i;
    char* owned_path;
    nds_texture decoded = {0};
    if (!cache || !path || !out_texture || !path[0]) return NDS_ERR_INVALID_ARG;
    *out_texture = NULL;
    for (i = 0; i < cache->capacity; ++i) {
        if (cache->entries[i].used && strcmp(cache->entries[i].path, path) == 0) {
            *out_texture = &cache->entries[i].texture;
            return NDS_OK;
        }
    }
    if (cache->count >= cache->capacity) return NDS_ERR_UNKNOWN;
    if (nds_texture_load(path, &decoded) != NDS_OK) return NDS_ERR_NOT_FOUND;
    owned_path = copy_string(path);
    if (!owned_path) {
        nds_texture_destroy(&decoded);
        return NDS_ERR_UNKNOWN;
    }
    for (i = 0; i < cache->capacity; ++i) {
        if (!cache->entries[i].used) {
            cache->entries[i].path = owned_path;
            cache->entries[i].texture = decoded;
            cache->entries[i].used = 1;
            ++cache->count;
            *out_texture = &cache->entries[i].texture;
            return NDS_OK;
        }
    }
    free(owned_path);
    nds_texture_destroy(&decoded);
    return NDS_ERR_UNKNOWN;
}

size_t nds_texture_cache_count(const nds_texture_cache* cache)
{
    return cache ? cache->count : 0;
}
