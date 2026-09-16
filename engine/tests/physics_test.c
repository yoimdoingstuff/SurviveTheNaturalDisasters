#include "engine/game/physics.h"

#include <math.h>
#include <stdio.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "physics_test: check failed: %s\n", #expr); \
        return 1; \
    } \
} while (0)

static nds_instance* make_part(nds_instance* root, const char* name, nds_vec3 pos, nds_vec3 size, int anchored)
{
    nds_instance* p = nds_instance_create(NDS_CLASS_PART, name);
    nds_part_properties props = {0};
    if (!p) return NULL;
    props.position = pos;
    props.size = size;
    props.anchored = (uint8_t)anchored;
    props.can_collide = 1;
    props.visible = 1;
    props.color_rgba = 0xffffffffu;
    if (nds_part_set_properties(p, &props) != NDS_OK || nds_instance_set_parent(p, root) != NDS_OK) {
        nds_instance_destroy(p);
        return NULL;
    }
    return p;
}

int main(void)
{
    nds_instance* root = nds_instance_create(NDS_CLASS_DATAMODEL, "PhysicsTest");
    nds_physics_world world;
    nds_instance *floor, *box, *far_box;
    nds_vec3 pos;
    nds_physics_body* body;
    int frame;

    CHECK(root != NULL);
    floor = make_part(root, "Floor", (nds_vec3){0,-1,0}, (nds_vec3){10,2,10}, 1);
    box = make_part(root, "Box", (nds_vec3){0,4,0}, (nds_vec3){1,1,1}, 0);
    far_box = make_part(root, "FarBox", (nds_vec3){100,4,100}, (nds_vec3){1,1,1}, 0);
    CHECK(floor != NULL && box != NULL && far_box != NULL);
    CHECK(nds_physics_init(&world, 4) == NDS_OK);
    CHECK(nds_physics_add_scene(&world, root) == NDS_OK);
    CHECK(world.count == 3);
    body = nds_physics_find_body(&world, box);
    CHECK(body != NULL && body->dynamic);

    /* Simulate roughly one second at a time step that the engine actually
     * accepts. The body's bottom should settle on the floor near y=0.5. */
    for (frame = 0; frame < 60; ++frame)
        CHECK(nds_physics_update(&world, 1.0f / 60.0f) == NDS_OK);
    CHECK(nds_part_get_position(box, &pos) == NDS_OK);
    CHECK(pos.y > 0.0f && pos.y < 2.0f);
    CHECK(body->grounded);
    CHECK(nds_part_get_position(far_box, &pos) == NDS_OK);
    /* FarBox is intentionally outside the floor's X/Z bounds, so it should
     * fall rather than being kept above the floor by a false collision. */
    CHECK(pos.y < 0.0f);

    /* A vertical impulse must leave the contact cleanly. */
    CHECK(nds_physics_apply_impulse(&world, box, (nds_vec3){0,10,0}) == NDS_OK);
    CHECK(body->velocity.y > 0.0f);
    CHECK(nds_physics_update(&world, .05f) == NDS_OK);
    CHECK(body->velocity.y < 10.0f);

    /* Friction should damp horizontal motion while the body is supported.
     * Place the box slightly into the floor so the discrete collision solver
     * has an overlap to resolve during this frame. */
    body->velocity.x = 8.0f;
    body->velocity.y = 0.0f;
    body->velocity.z = 0.0f;
    CHECK(nds_part_get_position(box, &pos) == NDS_OK);
    pos.y = .49f;
    CHECK(nds_part_set_position(box, pos) == NDS_OK);
    CHECK(nds_physics_update(&world, .016f) == NDS_OK);
    CHECK(fabsf(body->velocity.x) < 8.0f);

    nds_physics_destroy(&world);
    nds_instance_destroy(root);
    return 0;
}
