#include "engine/game/physics.h"
#include <assert.h>
#include <math.h>

static nds_instance* make_part(nds_instance* root,const char* name,nds_vec3 pos,nds_vec3 size,int anchored){
    nds_instance* p=nds_instance_create(NDS_CLASS_PART,name);nds_part_properties props={0};assert(p);props.position=pos;props.size=size;props.anchored=(uint8_t)anchored;props.can_collide=1;props.visible=1;props.color_rgba=0xffffffffu;assert(nds_part_set_properties(p,&props)==NDS_OK);assert(nds_instance_set_parent(p,root)==NDS_OK);return p;
}

int main(void){
    nds_instance* root=nds_instance_create(NDS_CLASS_DATAMODEL,"PhysicsTest");nds_physics_world world;nds_instance *floor,*box,*far_box;nds_vec3 pos;nds_physics_body* body;
    assert(root);
    floor=make_part(root,"Floor",(nds_vec3){0,-1,0},(nds_vec3){10,2,10},1);
    box=make_part(root,"Box",(nds_vec3){0,4,0},(nds_vec3){1,1,1},0);
    far_box=make_part(root,"FarBox",(nds_vec3){100,4,100},(nds_vec3){1,1,1},0);
    assert(nds_physics_init(&world,4)==NDS_OK);assert(nds_physics_add_scene(&world,root)==NDS_OK);assert(world.count==3);
    body=nds_physics_find_body(&world,box);assert(body&&body->dynamic);

    /* Falling body lands on the static floor and the broad phase ignores the
     * unrelated body hundreds of units away. */
    assert(nds_physics_update(&world,1.0f)==NDS_OK);assert(nds_part_get_position(box,&pos)==NDS_OK);assert(pos.y>0.0f&&pos.y<2.0f);assert(body->grounded);
    assert(nds_part_get_position(far_box,&pos)==NDS_OK);assert(pos.y>0.0f);

    /* A vertical impulse must leave the contact cleanly. */
    assert(nds_physics_apply_impulse(&world,box,(nds_vec3){0,10,0})==NDS_OK);assert(body->velocity.y>0.0f);
    assert(nds_physics_update(&world,.05f)==NDS_OK);assert(body->velocity.y<10.0f);

    /* Friction should damp horizontal motion while the body is supported. */
    body->velocity.x=8.0f;body->velocity.y=0.0f;body->velocity.z=0.0f;
    assert(nds_part_get_position(box,&pos)==NDS_OK);pos.y=.51f;assert(nds_part_set_position(box,pos)==NDS_OK);
    assert(nds_physics_update(&world,.016f)==NDS_OK);assert(fabsf(body->velocity.x)<8.0f);

    nds_physics_destroy(&world);nds_instance_destroy(root);return 0;
}
