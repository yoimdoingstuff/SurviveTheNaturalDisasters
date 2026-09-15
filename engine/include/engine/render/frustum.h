#ifndef NDS_ENGINE_RENDER_FRUSTUM_H
#define NDS_ENGINE_RENDER_FRUSTUM_H

#include "engine/core/types.h"
#include "engine/render/math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_aabb {
    nds_vec3 min;
    nds_vec3 max;
} nds_aabb;

/* Conservative clip-space test. Returns non-zero when the AABB may be visible. */
int nds_frustum_aabb_visible(const nds_mat4* view_projection, const nds_aabb* bounds);

#ifdef __cplusplus
}
#endif

#endif
