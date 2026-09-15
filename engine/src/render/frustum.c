#include "engine/render/frustum.h"

static void transform_point(const nds_mat4* m, float x, float y, float z,
                            float* out_x, float* out_y, float* out_z, float* out_w)
{
    *out_x = m->m[0] * x + m->m[4] * y + m->m[8] * z + m->m[12];
    *out_y = m->m[1] * x + m->m[5] * y + m->m[9] * z + m->m[13];
    *out_z = m->m[2] * x + m->m[6] * y + m->m[10] * z + m->m[14];
    *out_w = m->m[3] * x + m->m[7] * y + m->m[11] * z + m->m[15];
}

int nds_frustum_aabb_visible(const nds_mat4* view_projection, const nds_aabb* bounds)
{
    int outside_left = 1;
    int outside_right = 1;
    int outside_bottom = 1;
    int outside_top = 1;
    int outside_near = 1;
    int outside_far = 1;
    int xi, yi, zi;

    if (!view_projection || !bounds) return 0;

    for (xi = 0; xi < 2; ++xi) {
        const float x = xi ? bounds->max.x : bounds->min.x;
        for (yi = 0; yi < 2; ++yi) {
            const float y = yi ? bounds->max.y : bounds->min.y;
            for (zi = 0; zi < 2; ++zi) {
                const float z = zi ? bounds->max.z : bounds->min.z;
                float cx, cy, cz, cw;
                transform_point(view_projection, x, y, z, &cx, &cy, &cz, &cw);

                /* A point with w <= 0 is behind the eye. Keep the box visible
                   here because it can straddle the camera plane. */
                if (cw <= 0.0f) return 1;
                if (cx >= -cw) outside_left = 0;
                if (cx <=  cw) outside_right = 0;
                if (cy >= -cw) outside_bottom = 0;
                if (cy <=  cw) outside_top = 0;
                if (cz >= -cw) outside_near = 0;
                if (cz <=  cw) outside_far = 0;
            }
        }
    }

    return !(outside_left || outside_right || outside_bottom ||
             outside_top || outside_near || outside_far);
}
