#include "engine/render/mesh_cache.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    static const char* path = "game/content/meshes/triangle.ndsmesh";
    static const char* missing_path = "game/content/meshes/does-not-exist.ndsmesh";
    nds_mesh_cache* cache = NULL;
    const nds_mesh* first = NULL;
    const nds_mesh* second = NULL;
    const nds_mesh* third = NULL;

    assert(nds_mesh_cache_create(&cache, 2) == NDS_OK);
    assert(nds_mesh_cache_get(cache, path, &first) == NDS_OK);
    assert(first != NULL);
    assert(first->vertex_count == 3);
    assert(first->index_count == 3);
    assert(nds_mesh_cache_count(cache) == 1);

    assert(nds_mesh_cache_get(cache, path, &second) == NDS_OK);
    assert(second == first);
    assert(nds_mesh_cache_count(cache) == 1);

    assert(nds_mesh_cache_get(cache, missing_path, &third) != NDS_OK);
    assert(third == NULL);
    assert(nds_mesh_cache_count(cache) == 1);

    nds_mesh_cache_clear(cache);
    assert(nds_mesh_cache_count(cache) == 0);
    assert(nds_mesh_cache_get(cache, path, &third) == NDS_OK);
    assert(third != NULL);
    assert(nds_mesh_cache_count(cache) == 1);

    nds_mesh_cache_destroy(cache);
    puts("mesh cache test passed");
    return 0;
}
