#include "engine/render/texture.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    nds_texture texture = {0};
    nds_texture missing = {0};
    assert(nds_texture_load("game/content/textures/test.ndstex", &texture) == NDS_OK);
    assert(texture.width == 2 && texture.height == 2);
    assert(texture.rgba8 != NULL);
    assert(texture.rgba8[0] == 255 && texture.rgba8[1] == 0 && texture.rgba8[2] == 0 && texture.rgba8[3] == 255);
    assert(texture.rgba8[12] == 255 && texture.rgba8[13] == 255 && texture.rgba8[14] == 255 && texture.rgba8[15] == 128);
    nds_texture_destroy(&texture);
    assert(texture.rgba8 == NULL && texture.width == 0 && texture.height == 0);
    assert(nds_texture_load("game/content/textures/does-not-exist.ndstex", &missing) == NDS_ERR_NOT_FOUND);
    assert(missing.rgba8 == NULL);
    puts("texture tests passed");
    return 0;
}
