#include "engine/render/renderer.h"

#include <stdlib.h>

struct nds_renderer {
    nds_renderer_desc desc;
    unsigned int frame_count;
};

nds_result nds_renderer_create(nds_renderer** out_renderer, const nds_renderer_desc* desc)
{
    nds_renderer* renderer;
    if (!out_renderer || !desc || desc->width <= 0 || desc->height <= 0) return NDS_ERR_INVALID_ARG;
    renderer = (nds_renderer*)calloc(1, sizeof(*renderer));
    if (!renderer) return NDS_ERR_UNKNOWN;
    renderer->desc = *desc;
    *out_renderer = renderer;
    return NDS_OK;
}

void nds_renderer_destroy(nds_renderer* renderer)
{
    free(renderer);
}

nds_result nds_renderer_begin(nds_renderer* renderer)
{
    if (!renderer) return NDS_ERR_INVALID_ARG;
    return NDS_OK;
}

nds_result nds_renderer_draw(nds_renderer* renderer, const nds_draw_list* list)
{
    if (!renderer || !list) return NDS_ERR_INVALID_ARG;
    (void)list;
    return NDS_OK;
}

nds_result nds_renderer_end(nds_renderer* renderer)
{
    if (!renderer) return NDS_ERR_INVALID_ARG;
    ++renderer->frame_count;
    return NDS_OK;
}
