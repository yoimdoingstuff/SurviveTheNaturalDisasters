#include "engine/render/frustum.h"

#include <assert.h>

static nds_aabb box(float min_x, float min_y, float min_z,
                    float max_x, float max_y, float max_z)
{
    nds_aabb b;
    b.min = (nds_vec3){min_x, min_y, min_z};
    b.max = (nds_vec3){max_x, max_y, max_z};
    return b;
}

int main(void)
{
    nds_mat4 identity;
    nds_mat4 perspective;

    nds_mat4_identity(&identity);
    assert(nds_frustum_aabb_visible(&identity, &box(-0.5f, -0.5f, -0.5f,
                                                      0.5f, 0.5f, 0.5f)));
    assert(!nds_frustum_aabb_visible(&identity, &box(2.0f, -0.5f, -0.5f,
                                                       3.0f, 0.5f, 0.5f)));
    assert(!nds_frustum_aabb_visible(&identity, &box(-0.5f, -0.5f, 2.0f,
                                                       0.5f, 0.5f, 3.0f)));

    nds_mat4_perspective(&perspective, 70.0f, 1.0f, 0.1f, 100.0f);
    assert(nds_frustum_aabb_visible(&perspective, &box(-0.5f, -0.5f, -5.0f,
                                                         0.5f, 0.5f, -4.0f)));
    assert(!nds_frustum_aabb_visible(&perspective, &box(100.0f, -0.5f, -5.0f,
                                                          101.0f, 0.5f, -4.0f)));
    assert(nds_frustum_aabb_visible(NULL, &box(0, 0, 0, 1, 1, 1)) == 0);
    assert(nds_frustum_aabb_visible(&identity, NULL) == 0);
    return 0;
}
