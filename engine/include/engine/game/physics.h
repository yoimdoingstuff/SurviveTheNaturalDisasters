#ifndef NDS_ENGINE_GAME_PHYSICS_H
#define NDS_ENGINE_GAME_PHYSICS_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"
#include "engine/scene/part.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_physics_body {
    nds_instance* instance;
    nds_vec3 velocity;
    nds_vec3 accumulated_force;
    float mass;
    float restitution;
    float friction;
    uint8_t dynamic;
    uint8_t grounded;
} nds_physics_body;

typedef struct nds_physics_world {
    nds_physics_body* bodies;
    size_t count;
    size_t capacity;
    /* Reused broad-phase index storage. Keeping this beside the world avoids
     * per-frame heap churn on legacy/mobile targets. */
    size_t* broadphase_order;
    size_t broadphase_capacity;
    nds_vec3 gravity;
    float max_dt;
} nds_physics_world;

nds_result nds_physics_init(nds_physics_world* world, size_t capacity);
void nds_physics_destroy(nds_physics_world* world);
nds_result nds_physics_add_scene(nds_physics_world* world, nds_instance* root);
void nds_physics_clear(nds_physics_world* world);
nds_physics_body* nds_physics_find_body(nds_physics_world* world, const nds_instance* instance);
nds_result nds_physics_apply_impulse(nds_physics_world* world, nds_instance* instance, nds_vec3 impulse);
/* Cast a ray against registered collidable parts. Direction is normalized
 * internally. Returns NDS_OK with hit_instance == NULL when nothing is hit. */
nds_result nds_physics_raycast(const nds_physics_world* world, nds_vec3 origin,
                              nds_vec3 direction, float max_distance,
                              nds_instance** hit_instance, float* hit_distance,
                              nds_vec3* hit_normal);
nds_result nds_physics_update(nds_physics_world* world, float dt);

#ifdef __cplusplus
}
#endif

#endif
