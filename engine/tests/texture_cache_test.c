#include "engine/render/texture_cache.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    nds_texture_cache* cache = NULL;
    const nds_texture* first = NULL;
    const nds_texture* second = NULL;

    assert(nds_texture_cache_create(&cache, 2) == NDS_OK);
    assert(nds_texture_cache_count(cache) == 0);
    assert(nds_texture_cache_get(cache, "game/content/textures/checker.ndstex", &first) == NDS_OK);
    assert(first != NULL);
    assert(first->width == 2 && first->height == 2);
    assert(nds_texture_cache_count(cache) == 1);
    assert(nds_texture_cache_get(cache, "game/content/textures/checker.ndstex", &second) == NDS_OK);
    assert(second == first);
    assert(nds_texture_cache_count(cache) == 1);
    assert(nds_texture_cache_get(cache, "game/content/textures/missing.ndstex", &second) != NDS_OK);
    assert(second == NULL);
    assert(nds_texture_cache_count(cache) == 1);
    nds_texture_cache_clear(cache);
    assert(nds_texture_cache_count(cache) == 0);
    assert(nds_texture_cache_get(cache, "game/content/textures/checker.ndstex", &second) == NDS_OK);
    assert(second != NULL);
    nds_texture_cache_destroy(cache);
    puts("texture cache tests passed");
    return 0;
}
