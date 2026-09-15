#include "engine/render/gles2.h"
#include "gles2_backend.h"

#include <stdlib.h>

struct nds_gles2_renderer {
    nds_gles2_desc desc;
    nds_gles2_backend* backend;
    unsigned int frame_count;
};

nds_result nds_gles2_renderer_create(nds_gles2_renderer** out_renderer,
                                      const nds_gles2_desc* desc)
{
    if (!out_renderer || !desc || desc->width <= 0 || desc->height <= 0) return NDS_ERR_INVALID_ARG;
    *out_renderer = NULL;
    nds_gles2_renderer* renderer = (nds_gles2_renderer*)calloc(1, sizeof(*renderer));
    if (!renderer) return NDS_ERR_UNKNOWN;
    renderer->desc = *desc;
    if (renderer->desc.fov_y_degrees <= 0.0f) renderer->desc.fov_y_degrees = 70.0f;
    if (renderer->desc.near_plane <= 0.0f) renderer->desc.near_plane = 0.1f;
    if (renderer->desc.far_plane <= renderer->desc.near_plane) renderer->desc.far_plane = 1000.0f;
    nds_result rc = nds_gles2_backend_create(&renderer->backend,
        renderer->desc.width, renderer->desc.height,
        renderer->desc.fov_y_degrees, renderer->desc.near_plane, renderer->desc.far_plane);
    if (rc != NDS_OK) {
        free(renderer);
        return rc;
    }
    *out_renderer = renderer;
    return NDS_OK;
}

void nds_gles2_renderer_destroy(nds_gles2_renderer* renderer)
{
    if (!renderer) return;
    nds_gles2_backend_destroy(renderer->backend);
    free(renderer);
}

nds_result nds_gles2_renderer_resize(nds_gles2_renderer* renderer, int width, int height)
{
    if (!renderer || width <= 0 || height <= 0) return NDS_ERR_INVALID_ARG;
    renderer->desc.width = width;
    renderer->desc.height = height;
    return nds_gles2_backend_resize(renderer->backend, width, height);
}

nds_result nds_gles2_renderer_begin(nds_gles2_renderer* renderer)
{
    if (!renderer) return NDS_ERR_INVALID_ARG;
    return nds_gles2_backend_begin(renderer->backend);
}

nds_result nds_gles2_renderer_draw_parts(nds_gles2_renderer* renderer,
                                          const nds_draw_list* list)
{
    if (!renderer || !list) return NDS_ERR_INVALID_ARG;
    return nds_gles2_backend_draw_parts(renderer->backend, list);
}

nds_result nds_gles2_renderer_end(nds_gles2_renderer* renderer)
{
    if (!renderer) return NDS_ERR_INVALID_ARG;
    nds_result rc = nds_gles2_backend_end(renderer->backend);
    if (rc == NDS_OK) ++renderer->frame_count;
    return rc;
}
