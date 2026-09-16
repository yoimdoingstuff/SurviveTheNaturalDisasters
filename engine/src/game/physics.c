#include "engine/game/physics.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct nds_physics_bounds {
    float minx, maxx;
    float miny, maxy;
    float minz, maxz;
} nds_physics_bounds;

static nds_vec3 addv(nds_vec3 a, nds_vec3 b) { return (nds_vec3){a.x+b.x,a.y+b.y,a.z+b.z}; }
static nds_vec3 subv(nds_vec3 a, nds_vec3 b) { return (nds_vec3){a.x-b.x,a.y-b.y,a.z-b.z}; }
static nds_vec3 mulv(nds_vec3 a, float s) { return (nds_vec3){a.x*s,a.y*s,a.z*s}; }
static float dotv(nds_vec3 a, nds_vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static float maxf(float a,float b) { return a>b?a:b; }
static float minf(float a,float b) { return a<b?a:b; }
static float clampf(float v,float lo,float hi) { return v<lo?lo:(v>hi?hi:v); }

static int is_part(const nds_instance* i) {
    nds_instance_class c=i?nds_instance_get_class(i):NDS_CLASS_INSTANCE;
    return c==NDS_CLASS_PART||c==NDS_CLASS_SPAWN_POINT;
}

static void bounds(const nds_part_properties* p, nds_physics_bounds* out)
{
    float x=p->size.x*.5f,y=p->size.y*.5f,z=p->size.z*.5f;
    float rx=p->rotation.x*.0174532925199433f,ry=p->rotation.y*.0174532925199433f,rz=p->rotation.z*.0174532925199433f;
    float cx=cosf(rx),sx=sinf(rx),cy=cosf(ry),sy=sinf(ry),cz=cosf(rz),sz=sinf(rz);
    float ex=fabsf(cy*cz)*x+fabsf(sx*sy*cz-cx*sz)*y+fabsf(cx*sy*cz+sx*sz)*z;
    float ey=fabsf(cy*sz)*x+fabsf(sx*sy*sz+cx*cz)*y+fabsf(cx*sy*sz-sx*cz)*z;
    float ez=fabsf(sy)*x+fabsf(sx*cy)*y+fabsf(cx*cy)*z;
    out->minx=p->position.x-ex; out->maxx=p->position.x+ex;
    out->miny=p->position.y-ey; out->maxy=p->position.y+ey;
    out->minz=p->position.z-ez; out->maxz=p->position.z+ez;
}

static int aabb_overlap(const nds_part_properties* a,const nds_part_properties* b,nds_vec3* n,float* depth)
{
    nds_physics_bounds ab,bb;
    float ox,oy,oz;
    float dx,dy,dz;
    bounds(a,&ab); bounds(b,&bb);
    ox=minf(ab.maxx,bb.maxx)-maxf(ab.minx,bb.minx);
    oy=minf(ab.maxy,bb.maxy)-maxf(ab.miny,bb.miny);
    oz=minf(ab.maxz,bb.maxz)-maxf(ab.minz,bb.minz);
    if(ox<=0.0f||oy<=0.0f||oz<=0.0f)return 0;

    dx=b->position.x-a->position.x;
    dy=b->position.y-a->position.y;
    dz=b->position.z-a->position.z;
    *n=(nds_vec3){0,0,0};
    if(oy<=ox&&oy<=oz){*n=(nds_vec3){0,dy>=0.0f?1.0f:-1.0f,0};*depth=oy;}
    else if(oz<=ox&&oz<=oy){*n=(nds_vec3){0,0,dz>=0.0f?1.0f:-1.0f};*depth=oz;}
    else {*n=(nds_vec3){dx>=0.0f?1.0f:-1.0f,0,0};*depth=ox;}
    return 1;
}

static nds_result ensure_capacity(nds_physics_world* w,size_t needed)
{
    nds_physics_body* grown;size_t cap;
    if(needed<=w->capacity)return NDS_OK;
    cap=w->capacity?w->capacity:64;
    while(cap<needed){if(cap>65536u)return NDS_ERR_UNKNOWN;cap*=2;}
    grown=(nds_physics_body*)realloc(w->bodies,cap*sizeof(*grown));if(!grown)return NDS_ERR_UNKNOWN;
    w->bodies=grown;w->capacity=cap;return NDS_OK;
}

static nds_result ensure_broadphase_capacity(nds_physics_world* w,size_t needed)
{
    size_t* grown;size_t cap;
    if(needed<=w->broadphase_capacity)return NDS_OK;
    cap=w->broadphase_capacity?w->broadphase_capacity:64;
    while(cap<needed){if(cap>65536u)return NDS_ERR_UNKNOWN;cap*=2;}
    grown=(size_t*)realloc(w->broadphase_order,cap*sizeof(*grown));if(!grown)return NDS_ERR_UNKNOWN;
    w->broadphase_order=grown;w->broadphase_capacity=cap;return NDS_OK;
}

static nds_result collect(nds_physics_world* w,nds_instance* node)
{
    size_t i;nds_result rc;if(!node)return NDS_OK;
    if(is_part(node)){
        nds_part_properties p;
        if(nds_part_get_properties(node,&p)!=NDS_OK)return NDS_ERR_UNKNOWN;
        if(nds_physics_find_body(w,node))return NDS_OK;
        rc=ensure_capacity(w,w->count+1);if(rc!=NDS_OK)return rc;
        memset(&w->bodies[w->count],0,sizeof(*w->bodies));w->bodies[w->count].instance=node;
        w->bodies[w->count].dynamic=(uint8_t)!p.anchored;
        w->bodies[w->count].mass=maxf(.1f,p.size.x*p.size.y*p.size.z);
        w->bodies[w->count].restitution=.05f;w->bodies[w->count].friction=.7f;++w->count;
    }
    for(i=0;i<nds_instance_child_count(node);++i){rc=collect(w,nds_instance_child_at(node,i));if(rc!=NDS_OK)return rc;}
    return NDS_OK;
}

nds_result nds_physics_init(nds_physics_world* w,size_t capacity)
{
    if(!w)return NDS_ERR_INVALID_ARG;memset(w,0,sizeof(*w));w->gravity=(nds_vec3){0,-32.0f,0};w->max_dt=.05f;
    if(capacity){w->bodies=(nds_physics_body*)calloc(capacity,sizeof(*w->bodies));if(!w->bodies)return NDS_ERR_UNKNOWN;w->capacity=capacity;}
    return NDS_OK;
}

void nds_physics_destroy(nds_physics_world* w){if(!w)return;free(w->bodies);free(w->broadphase_order);memset(w,0,sizeof(*w));}
void nds_physics_clear(nds_physics_world* w){if(w)w->count=0;}

nds_physics_body* nds_physics_find_body(nds_physics_world* w,const nds_instance* i){size_t n;if(!w||!i)return NULL;for(n=0;n<w->count;++n)if(w->bodies[n].instance==i)return &w->bodies[n];return NULL;}
nds_result nds_physics_add_scene(nds_physics_world* w,nds_instance* root){if(!w||!root)return NDS_ERR_INVALID_ARG;return collect(w,root);}
nds_result nds_physics_apply_impulse(nds_physics_world* w,nds_instance* i,nds_vec3 impulse){nds_physics_body* b;if(!w||!i)return NDS_ERR_INVALID_ARG;b=nds_physics_find_body(w,i);if(!b||!b->dynamic)return NDS_ERR_INVALID_ARG;b->velocity=addv(b->velocity,mulv(impulse,1.0f/b->mass));b->grounded=0;return NDS_OK;}

static void integrate_body(nds_physics_world* w,nds_physics_body* b,float dt)
{
    nds_part_properties p;if(!b->dynamic||nds_part_get_properties(b->instance,&p)!=NDS_OK)return;
    b->velocity=addv(b->velocity,mulv(w->gravity,dt));
    b->velocity=addv(b->velocity,mulv(b->accumulated_force,dt/b->mass));
    b->accumulated_force=(nds_vec3){0,0,0};
    p.position=addv(p.position,mulv(b->velocity,dt));
    b->grounded=0;
    nds_part_set_position(b->instance,p.position);
}

static void apply_support_friction(nds_physics_body* dynamic_body,const nds_physics_body* other,float dt)
{
    float speed,normal_force,impulse,mu;
    if(!dynamic_body->dynamic||other->dynamic)return;
    speed=sqrtf(dynamic_body->velocity.x*dynamic_body->velocity.x+dynamic_body->velocity.z*dynamic_body->velocity.z);
    if(speed<=0.0001f)return;
    mu=sqrtf(maxf(0.0f,dynamic_body->friction)*maxf(0.0f,other->friction));
    normal_force=32.0f*dynamic_body->mass;
    impulse=minf(speed*dynamic_body->mass,normal_force*mu*dt);
    dynamic_body->velocity.x-=dynamic_body->velocity.x/speed*(impulse/dynamic_body->mass);
    dynamic_body->velocity.z-=dynamic_body->velocity.z/speed*(impulse/dynamic_body->mass);
}

static void resolve_pair(nds_physics_body* a,nds_physics_body* b,float dt)
{
    nds_part_properties pa,pb;nds_vec3 n,rv,tangent,impulse;float depth,inva=0,invb=0,total,rel,j,jt,maxjt,mu;
    if(nds_part_get_properties(a->instance,&pa)!=NDS_OK||nds_part_get_properties(b->instance,&pb)!=NDS_OK||!pa.can_collide||!pb.can_collide||(!a->dynamic&&!b->dynamic)||!aabb_overlap(&pa,&pb,&n,&depth))return;
    if(a->dynamic)inva=1.0f/a->mass;if(b->dynamic)invb=1.0f/b->mass;total=inva+invb;if(total<=0)return;

    if(a->dynamic){pa.position=subv(pa.position,mulv(n,depth*(inva/total)));nds_part_set_position(a->instance,pa.position);}
    if(b->dynamic){pb.position=addv(pb.position,mulv(n,depth*(invb/total)));nds_part_set_position(b->instance,pb.position);}

    rv=subv(b->velocity,a->velocity);rel=dotv(rv,n);
    if(rel<0.0f){
        j=-(1.0f+minf(a->restitution,b->restitution))*rel/total;
        impulse=mulv(n,j);
        if(a->dynamic)a->velocity=subv(a->velocity,mulv(impulse,inva));
        if(b->dynamic)b->velocity=addv(b->velocity,mulv(impulse,invb));
        tangent=subv(rv,mulv(n,rel));
        {float tl=sqrtf(dotv(tangent,tangent));if(tl>0.0001f){tangent=mulv(tangent,1.0f/tl);jt=-dotv(rv,tangent)/total;mu=sqrtf(maxf(0,a->friction)*maxf(0,b->friction));maxjt=j*mu;jt=clampf(jt,-maxjt,maxjt);impulse=mulv(tangent,jt);if(a->dynamic)a->velocity=subv(a->velocity,mulv(impulse,inva));if(b->dynamic)b->velocity=addv(b->velocity,mulv(impulse,invb));}}
    }
    if(a->dynamic&&n.y<-.5f){a->grounded=1;if(a->velocity.y>0)a->velocity.y=0;apply_support_friction(a,b,dt);}
    if(b->dynamic&&n.y>.5f){b->grounded=1;if(b->velocity.y<0)b->velocity.y=0;apply_support_friction(b,a,dt);}
}

static float body_min_x(const nds_physics_body* b)
{
    nds_part_properties p;nds_physics_bounds bnd;if(nds_part_get_properties(b->instance,&p)!=NDS_OK)return 0.0f;bounds(&p,&bnd);return bnd.minx;
}

static void broadphase_sort(nds_physics_world* w)
{
    size_t i,j,key;float keyx;
    for(i=0;i<w->count;++i)w->broadphase_order[i]=i;
    for(i=1;i<w->count;++i){key=w->broadphase_order[i];keyx=body_min_x(&w->bodies[key]);j=i;while(j>0&&body_min_x(&w->bodies[w->broadphase_order[j-1]])>keyx){w->broadphase_order[j]=w->broadphase_order[j-1];--j;}w->broadphase_order[j]=key;}
}

static void broadphase_collide(nds_physics_world* w,float dt)
{
    size_t i,j,ia,ib;nds_part_properties pa,pb;nds_physics_bounds ab,bb;
    broadphase_sort(w);
    for(i=0;i<w->count;++i){ia=w->broadphase_order[i];if(nds_part_get_properties(w->bodies[ia].instance,&pa)!=NDS_OK)continue;bounds(&pa,&ab);
        for(j=i+1;j<w->count;++j){ib=w->broadphase_order[j];if(nds_part_get_properties(w->bodies[ib].instance,&pb)!=NDS_OK)continue;bounds(&pb,&bb);
            if(bb.minx>ab.maxx)break;
            if(bb.maxy<=ab.miny||bb.miny>=ab.maxy||bb.maxz<=ab.minz||bb.minz>=ab.maxz)continue;
            resolve_pair(&w->bodies[ia],&w->bodies[ib],dt);
        }
    }
}

nds_result nds_physics_update(nds_physics_world* w,float dt)
{
    size_t i;int steps;float step;if(!w)return NDS_ERR_INVALID_ARG;
    if(dt<0.0f)dt=0.0f;if(dt>w->max_dt)dt=w->max_dt;steps=(int)ceilf(dt/.0166667f);if(steps<1)steps=1;if(steps>4)steps=4;
    if(ensure_broadphase_capacity(w,w->count)!=NDS_OK)return NDS_ERR_UNKNOWN;
    step=dt/steps;
    for(i=0;i<(size_t)steps;++i){size_t j;for(j=0;j<w->count;++j)integrate_body(w,&w->bodies[j],step);broadphase_collide(w,step);}
    return NDS_OK;
}
