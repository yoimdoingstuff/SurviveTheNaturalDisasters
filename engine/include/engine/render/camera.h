#ifndef NDS_ENGINE_RENDER_CAMERA_H
#define NDS_ENGINE_RENDER_CAMERA_H

#include "engine/core/types.h"
#include "engine/render/math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_camera {
    float position[3];
    float target[3];
    float up[3];
    float fov_y_degrees;
    float near_plane;
    float far_plane;
} nds_camera;

void nds_camera_init(nds_camera* camera);
void nds_camera_view_matrix(const nds_camera* camera, nds_mat4* out);
void nds_camera_projection_matrix(const nds_camera* camera, float aspect, nds_mat4* out);

#ifdef __cplusplus
}
#endif

#endif
