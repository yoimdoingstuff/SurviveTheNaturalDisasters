#ifndef NDS_ENGINE_RENDER_MATH_H
#define NDS_ENGINE_RENDER_MATH_H

#include "engine/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_mat4 {
    float m[16];
} nds_mat4;

void nds_mat4_identity(nds_mat4* out);
void nds_mat4_perspective(nds_mat4* out, float fov_y_degrees,
                          float aspect, float near_plane, float far_plane);
void nds_mat4_translate(nds_mat4* out, float x, float y, float z);
void nds_mat4_scale(nds_mat4* out, float x, float y, float z);
void nds_mat4_mul(nds_mat4* out, const nds_mat4* a, const nds_mat4* b);

#ifdef __cplusplus
}
#endif

#endif
