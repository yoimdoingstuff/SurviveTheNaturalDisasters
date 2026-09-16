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
    nds_instance *floor, *box, *far_box, *wall, *switch_box;
    nds_vec3 pos, normal;
    nds_instance* hit = NULL;
    float distance = 0.0f;
    nds_physics_body* body;
    nds_physics_body* switch_body;
    nds_part_properties switch_props;
    int frame;

    CHECK(root != NULL);
    floor = make_part(root, "Floor", (nds_vec3){0,-1,0}, (nds_vec3){10,2,10}, 1);
    box = make_part(root, "Box", (nds_vec3){0,4,0}, (nds_vec3){1,1,1}, 0);
    far_box = make_part(root, "FarBox", (nds_vec3){100,4,100}, (nds_vec3){1,1,1}, 0);
    wall = make_part(root, "Wall", (nds_vec3){0,2,-4}, (nds_vec3){6,4,1}, 1);
    switch_box = make_part(root, "SwitchBox", (nds_vec3){2,8,2}, (nds_vec3){1,1,1}, 1);
    CHECK(floor != NULL && box != NULL && far_box != NULL && wall != NULL && switch_box != NULL);
    CHECK(nds_physics_init(&world, 5) == NDS_OK);
    CHECK(nds_physics_add_scene(&world, root) == NDS_OK);
    CHECK(world.count == 5);
    body = nds_physics_find_body(&world, box);
    switch_body = nds_physics_find_body(&world, switch_box);
    CHECK(body != NULL && body->dynamic);
    CHECK(switch_body != NULL && !switch_body->dynamic);

    CHECK(nds_physics_raycast(&world, (nds_vec3){0,2,0}, (nds_vec3){0,0,-1}, 20.0f,
                              &hit, &distance, &normal) == NDS_OK);
    CHECK(hit == wall);
    CHECK(fabsf(distance - 3.5f) < 0.001f);
    CHECK(fabsf(normal.z - 1.0f) < 0.001f);

    hit = NULL;
    CHECK(nds_physics_raycast(&world, (nds_vec3){0,2,0}, (nds_vec3){0,0,-2}, 20.0f,
                              &hit, &distance, &normal) == NDS_OK);
    CHECK(hit == wall);
    CHECK(fabsf(distance - 3.5f) < 0.001f);

    hit = wall;
    CHECK(nds_physics_raycast(&world, (nds_vec3){0,2,0}, (nds_vec3){0,0,-1}, 3.0f,
                              &hit, &distance, &normal) == NDS_OK);
    CHECK(hit == NULL);
    CHECK(fabsf(distance - 3.0f) < 0.001f);

    for (frame = 0; frame < 60; ++frame)
        CHECK(nds_physics_update(&world, 1.0f / 60.0f) == NDS_OK);
    CHECK(nds_part_get_position(box, &pos) == NDS_OK);
    CHECK(pos.y > 0.0f && pos.y < 2.0f);
    CHECK(body->grounded);
    CHECK(nds_part_get_position(far_box, &pos) == NDS_OK);
    CHECK(pos.y < 0.0f);
    CHECK(nds_part_get_position(switch_box, &pos) == NDS_OK);
    CHECK(fabsf(pos.y - 8.0f) < 0.001f);

    CHECK(nds_part_get_properties(switch_box, &switch_props) == NDS_OK);
    switch_props.anchored = 0;
    CHECK(nds_part_set_properties(switch_box, &switch_props) == NDS_OK);
    CHECK(!switch_body->dynamic);
    CHECK(nds_physics_update(&world, .05f) == NDS_OK);
    CHECK(switch_body->dynamic);
    CHECK(nds_part_get_position(switch_box, &pos) == NDS_OK);
    CHECK(pos.y < 8.0f);

    CHECK(nds_physics_apply_impulse(&world, box, (nds_vec3){0,10,0}) == NDS_OK);
    CHECK(body->velocity.y > 0.0f);
    CHECK(nds_physics_update(&world, .05f) == NDS_OK);
    CHECK(body->velocity.y < 10.0f);

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
