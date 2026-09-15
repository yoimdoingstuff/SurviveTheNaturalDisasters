#include "engine/render/camera.h"

#include <math.h>

static void normalize3(float* x, float* y, float* z)
{
    float length = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
    if (length > 0.000001f) {
        *x /= length;
        *y /= length;
        *z /= length;
    }
}

void nds_camera_init(nds_camera* camera)
{
    if (!camera) return;
    camera->position[0] = 0.0f;
    camera->position[1] = 5.0f;
    camera->position[2] = 10.0f;
    camera->target[0] = 0.0f;
    camera->target[1] = 0.0f;
    camera->target[2] = 0.0f;
    camera->up[0] = 0.0f;
    camera->up[1] = 1.0f;
    camera->up[2] = 0.0f;
    camera->fov_y_degrees = 70.0f;
    camera->near_plane = 0.1f;
    camera->far_plane = 2000.0f;
}

void nds_camera_view_matrix(const nds_camera* camera, nds_mat4* out)
{
    float fx, fy, fz;
    float sx, sy, sz;
    float ux, uy, uz;
    if (!camera || !out) return;

    fx = camera->target[0] - camera->position[0];
    fy = camera->target[1] - camera->position[1];
    fz = camera->target[2] - camera->position[2];
    normalize3(&fx, &fy, &fz);

    sx = fy * camera->up[2] - fz * camera->up[1];
    sy = fz * camera->up[0] - fx * camera->up[2];
    sz = fx * camera->up[1] - fy * camera->up[0];
    normalize3(&sx, &sy, &sz);

    ux = sy * fz - sz * fy;
    uy = sz * fx - sx * fz;
    uz = sx * fy - sy * fx;

    nds_mat4_identity(out);
    out->m[0] = sx; out->m[4] = sy; out->m[8] = sz;
    out->m[1] = ux; out->m[5] = uy; out->m[9] = uz;
    out->m[2] = -fx; out->m[6] = -fy; out->m[10] = -fz;
    out->m[12] = -(sx * camera->position[0] + sy * camera->position[1] + sz * camera->position[2]);
    out->m[13] = -(ux * camera->position[0] + uy * camera->position[1] + uz * camera->position[2]);
    out->m[14] = fx * camera->position[0] + fy * camera->position[1] + fz * camera->position[2];
}

void nds_camera_projection_matrix(const nds_camera* camera, float aspect, nds_mat4* out)
{
    if (!camera || !out) return;
    nds_mat4_perspective(out, camera->fov_y_degrees, aspect,
                         camera->near_plane, camera->far_plane);
}
