#ifndef NDS_ENGINE_RENDERER_H
#define NDS_ENGINE_RENDERER_H

#include "engine/core/types.h"
#include "engine/render/draw.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_renderer nds_renderer;

typedef struct nds_renderer_desc {
    int width;
    int height;
    const char* title;
} nds_renderer_desc;

nds_result nds_renderer_create(nds_renderer** out_renderer, const nds_renderer_desc* desc);
void nds_renderer_destroy(nds_renderer* renderer);
nds_result nds_renderer_begin(nds_renderer* renderer);
nds_result nds_renderer_draw(nds_renderer* renderer, const nds_draw_list* list);
nds_result nds_renderer_end(nds_renderer* renderer);

#ifdef __cplusplus
}
#endif

#endif
