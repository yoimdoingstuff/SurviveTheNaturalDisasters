#ifndef NDS_GLES2_BACKEND_H
#define NDS_GLES2_BACKEND_H

#include "engine/core/types.h"
#include "engine/render/draw.h"
#include "engine/render/camera.h"
#include "engine/render/gles2.h"

typedef struct nds_gles2_backend nds_gles2_backend;

nds_result nds_gles2_backend_create(nds_gles2_backend** out_backend, int width, int height,
                                    float fov_y_degrees, float near_plane, float far_plane);
void nds_gles2_backend_destroy(nds_gles2_backend* backend);
nds_result nds_gles2_backend_resize(nds_gles2_backend* backend, int width, int height);
nds_result nds_gles2_backend_begin(nds_gles2_backend* backend);
nds_result nds_gles2_backend_draw_parts(nds_gles2_backend* backend,
                                        const nds_draw_list* list,
                                        const nds_camera* camera);
nds_result nds_gles2_backend_draw_hud(nds_gles2_backend* backend,
                                      const nds_hud_state* state);
nds_result nds_gles2_backend_end(nds_gles2_backend* backend);

#endif
