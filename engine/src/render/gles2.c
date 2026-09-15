#include "engine/render/gles2.h"

#include <stdlib.h>

/*
 * This file intentionally contains no GLES headers yet. The actual GL calls
 * belong behind the platform context adapter so the same renderer logic can
 * target iOS 6/8, Android API 14+, and desktop OpenGL ES-compatible contexts.
 */
struct nds_gles2_renderer {
    nds_gles2_desc desc;
    float aspect;
    unsigned int frame_count;
};

static void update_aspect(nds_gles2_renderer* renderer)
{
    renderer->aspect = renderer->desc.height > 0
        ? (float)renderer->desc.width / (float)renderer->desc.height
        : 1.0f;
}

nds_result nds_gles2_renderer_create(nds_gles2_renderer** out_renderer,
                                      const nds_gles2_desc* desc)
{
    nds_gles2_renderer* renderer;
    if (!out_renderer || !desc || desc->width <= 0 || desc->height <= 0) {
        return NDS_ERR_INVALID_ARG;
    }
    renderer = (nds_gles2_renderer*)calloc(1, sizeof(*renderer));
    if (!renderer) return NDS_ERR_UNKNOWN;
    renderer->desc = *desc;
    if (renderer->desc.fov_y_degrees <= 0.0f) renderer->desc.fov_y_degrees = 70.0f;
    if (renderer->desc.near_plane <= 0.0f) renderer->desc.near_plane = 0.1f;
    if (renderer->desc.far_plane <= renderer->desc.near_plane) renderer->desc.far_plane = 1000.0f;
    update_aspect(renderer);
    *out_renderer = renderer;
    return NDS_OK;
}

void nds_gles2_renderer_destroy(nds_gles2_renderer* renderer)
{
    free(renderer);
}

nds_result nds_gles2_renderer_resize(nds_gles2_renderer* renderer, int width, int height)
{
    if (!renderer || width <= 0 || height <= 0) return NDS_ERR_INVALID_ARG;
    renderer->desc.width = width;
    renderer->desc.height = height;
    update_aspect(renderer);
    return NDS_OK;
}

nds_result nds_gles2_renderer_begin(nds_gles2_renderer* renderer)
{
    if (!renderer) return NDS_ERR_INVALID_ARG;
    return NDS_OK;
}

nds_result nds_gles2_renderer_draw_parts(nds_gles2_renderer* renderer,
                                          const nds_draw_list* list)
{
    if (!renderer || !list) return NDS_ERR_INVALID_ARG;
    /* GL buffer/shader submission is the next backend-specific step. */
    (void)list;
    return NDS_OK;
}

nds_result nds_gles2_renderer_end(nds_gles2_renderer* renderer)
{
    if (!renderer) return NDS_ERR_INVALID_ARG;
    ++renderer->frame_count;
    return NDS_OK;
}
