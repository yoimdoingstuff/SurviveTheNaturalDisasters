#include "engine/render/mesh_cache.h"

#include <stdlib.h>
#include <string.h>

typedef struct nds_mesh_cache_entry {
    char* path;
    nds_mesh mesh;
} nds_mesh_cache_entry;

struct nds_mesh_cache {
    nds_mesh_cache_entry* entries;
    size_t count;
    size_t capacity;
};

static char* copy_string(const char* text)
{
    size_t length;
    char* copy;
    if (!text) return NULL;
    length = strlen(text);
    copy = (char*)malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, text, length + 1);
    return copy;
}

static void destroy_entry(nds_mesh_cache_entry* entry)
{
    if (!entry) return;
    free(entry->path);
    entry->path = NULL;
    nds_mesh_destroy(&entry->mesh);
}

nds_result nds_mesh_cache_create(nds_mesh_cache** out_cache, size_t capacity)
{
    nds_mesh_cache* cache;
    if (!out_cache || capacity == 0) return NDS_ERR_INVALID_ARG;
    *out_cache = NULL;
    cache = (nds_mesh_cache*)calloc(1, sizeof(*cache));
    if (!cache) return NDS_ERR_UNKNOWN;
    cache->entries = (nds_mesh_cache_entry*)calloc(capacity, sizeof(*cache->entries));
    if (!cache->entries) {
        free(cache);
        return NDS_ERR_UNKNOWN;
    }
    cache->capacity = capacity;
    *out_cache = cache;
    return NDS_OK;
}

void nds_mesh_cache_clear(nds_mesh_cache* cache)
{
    size_t i;
    if (!cache) return;
    for (i = 0; i < cache->count; ++i) destroy_entry(&cache->entries[i]);
    cache->count = 0;
}

void nds_mesh_cache_destroy(nds_mesh_cache* cache)
{
    if (!cache) return;
    nds_mesh_cache_clear(cache);
    free(cache->entries);
    free(cache);
}

nds_result nds_mesh_cache_get(nds_mesh_cache* cache, const char* path,
                              const nds_mesh** out_mesh)
{
    size_t i;
    nds_mesh_cache_entry* entry;
    if (!cache || !path || !path[0] || !out_mesh) return NDS_ERR_INVALID_ARG;
    *out_mesh = NULL;

    for (i = 0; i < cache->count; ++i) {
        if (strcmp(cache->entries[i].path, path) == 0) {
            *out_mesh = &cache->entries[i].mesh;
            return NDS_OK;
        }
    }

    if (cache->count >= cache->capacity) return NDS_ERR_UNKNOWN;
    entry = &cache->entries[cache->count];
    entry->path = copy_string(path);
    if (!entry->path) return NDS_ERR_UNKNOWN;
    if (nds_mesh_load(path, &entry->mesh) != NDS_OK) {
        free(entry->path);
        entry->path = NULL;
        return NDS_ERR_IO;
    }
    ++cache->count;
    *out_mesh = &entry->mesh;
    return NDS_OK;
}

size_t nds_mesh_cache_count(const nds_mesh_cache* cache)
{
    return cache ? cache->count : 0;
}
