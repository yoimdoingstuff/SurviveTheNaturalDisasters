#include "engine/render/math.h"

#include <math.h>

void nds_mat4_identity(nds_mat4* out)
{
    int i;
    if (!out) return;
    for (i = 0; i < 16; ++i) out->m[i] = 0.0f;
    out->m[0] = 1.0f;
    out->m[5] = 1.0f;
    out->m[10] = 1.0f;
    out->m[15] = 1.0f;
}

void nds_mat4_perspective(nds_mat4* out, float fov_y_degrees,
                          float aspect, float near_plane, float far_plane)
{
    float f;
    if (!out || aspect <= 0.0f || near_plane <= 0.0f || far_plane <= near_plane) return;
    f = 1.0f / tanf((fov_y_degrees * 0.01745329251994329577f) * 0.5f);
    nds_mat4_identity(out);
    out->m[0] = f / aspect;
    out->m[5] = f;
    out->m[10] = (far_plane + near_plane) / (near_plane - far_plane);
    out->m[11] = -1.0f;
    out->m[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
    out->m[15] = 0.0f;
}

void nds_mat4_translate(nds_mat4* out, float x, float y, float z)
{
    nds_mat4_identity(out);
    if (!out) return;
    out->m[12] = x;
    out->m[13] = y;
    out->m[14] = z;
}

void nds_mat4_scale(nds_mat4* out, float x, float y, float z)
{
    nds_mat4_identity(out);
    if (!out) return;
    out->m[0] = x;
    out->m[5] = y;
    out->m[10] = z;
}

void nds_mat4_mul(nds_mat4* out, const nds_mat4* a, const nds_mat4* b)
{
    nds_mat4 r;
    int row, col, k;
    if (!out || !a || !b) return;
    for (col = 0; col < 4; ++col) {
        for (row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (k = 0; k < 4; ++k) {
                sum += a->m[k * 4 + row] * b->m[col * 4 + k];
            }
            r.m[col * 4 + row] = sum;
        }
    }
    *out = r;
}
