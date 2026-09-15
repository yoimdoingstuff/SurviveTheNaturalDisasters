#ifndef NDS_ENGINE_PART_H
#define NDS_ENGINE_PART_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"
#include "engine/render/mesh.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_vec3 {
    float x;
    float y;
    float z;
} nds_vec3;

typedef struct nds_part_properties {
    nds_vec3 position;
    nds_vec3 size;
    nds_vec3 rotation;
    float transparency;
    float reflectance;
    uint32_t color_rgba;
    uint8_t anchored;
    uint8_t can_collide;
    uint8_t visible;
    const nds_mesh* mesh;
} nds_part_properties;

nds_result nds_part_get_properties(const nds_instance* instance,
                                   nds_part_properties* out_properties);
nds_result nds_part_set_properties(nds_instance* instance,
                                   const nds_part_properties* properties);
nds_result nds_part_set_position(nds_instance* instance, nds_vec3 position);
nds_result nds_part_get_position(const nds_instance* instance, nds_vec3* out_position);
nds_result nds_part_set_size(nds_instance* instance, nds_vec3 size);
nds_result nds_part_get_size(const nds_instance* instance, nds_vec3* out_size);
nds_result nds_part_set_mesh(nds_instance* instance, const nds_mesh* mesh);
nds_result nds_part_get_mesh(const nds_instance* instance, const nds_mesh** out_mesh);

#ifdef __cplusplus
}
#endif

#endif
