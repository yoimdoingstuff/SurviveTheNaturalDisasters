#include "engine/game/player.h"
#include <math.h>
#include <stddef.h>

static int overlap(float amin, float amax, float bmin, float bmax) { return amax > bmin && amin < bmax; }

static void bounds(const nds_part_properties* p, float* ax, float* bx, float* ay, float* by, float* az, float* bz)
{
    float x=p->size.x*.5f,y=p->size.y*.5f,z=p->size.z*.5f;
    float rx=p->rotation.x*.0174532925199433f,ry=p->rotation.y*.0174532925199433f,rz=p->rotation.z*.0174532925199433f;
    float cx=cosf(rx),sx=sinf(rx),cy=cosf(ry),sy=sinf(ry),cz=cosf(rz),sz=sinf(rz);
    float ex=fabsf(cy*cz)*x+fabsf(sx*sy*cz-cx*sz)*y+fabsf(cx*sy*cz+sx*sz)*z;
    float ey=fabsf(cy*sz)*x+fabsf(sx*sy*sz+cx*cz)*y+fabsf(cx*sy*sz-sx*cz)*z;
    float ez=fabsf(sy)*x+fabsf(sx*cy)*y+fabsf(cx*cy)*z;
    *ax=p->position.x-ex;*bx=p->position.x+ex;*ay=p->position.y-ey;*by=p->position.y+ey;*az=p->position.z-ez;*bz=p->position.z+ez;
}

static void collide(nds_player_controller* pl,const nds_instance* root,int axis,float old,float half)
{
    size_t i,n=nds_instance_child_count(root);
    for(i=0;i<n;++i){
        const nds_instance* c=nds_instance_child_at(root,i); nds_part_properties p;
        float ax,bx,ay,by,az,bz,px0,px1,py0,py1,pz0,pz1;
        if(!c)continue;
        if(nds_instance_get_class(c)==NDS_CLASS_PART&&nds_part_get_properties(c,&p)==NDS_OK&&p.can_collide&&p.visible){
            bounds(&p,&ax,&bx,&ay,&by,&az,&bz);
            px0=pl->position.x-pl->half_width;px1=pl->position.x+pl->half_width;
            py0=pl->position.y-pl->half_height;py1=pl->position.y+pl->half_height;
            pz0=pl->position.z-pl->half_width;pz1=pl->position.z+pl->half_width;
            if(axis==0&&overlap(py0,py1,ay,by)&&overlap(pz0,pz1,az,bz)){
                if(pl->velocity.x>0&&old+half<=ax+.01f&&px1>ax){pl->position.x=ax-half;pl->velocity.x=0;}
                else if(pl->velocity.x<0&&old-half>=bx-.01f&&px0<bx){pl->position.x=bx+half;pl->velocity.x=0;}
            }else if(axis==1&&overlap(px0,px1,ax,bx)&&overlap(pz0,pz1,az,bz)){
                if(pl->velocity.y>0&&old+half<=ay+.01f&&py1>ay){pl->position.y=ay-half;pl->velocity.y=0;}
                else if(pl->velocity.y<0&&old-half>=by-.01f&&py0<by){pl->position.y=by+half;pl->velocity.y=0;pl->grounded=1;}
            }else if(axis==2&&overlap(px0,px1,ax,bx)&&overlap(py0,py1,ay,by)){
                if(pl->velocity.z>0&&old+half<=az+.01f&&pz1>az){pl->position.z=az-half;pl->velocity.z=0;}
                else if(pl->velocity.z<0&&old-half>=bz-.01f&&pz0<bz){pl->position.z=bz+half;pl->velocity.z=0;}
            }
        }
        if(nds_instance_child_count(c))collide(pl,c,axis,old,half);
    }
}

void nds_player_init(nds_player_controller* p,const nds_instance* scene)
{
    size_t i;
    if(!p)return;
    p->position=(nds_vec3){0,5,0};p->velocity=(nds_vec3){0,0,0};p->half_width=.45f;p->half_height=1.5f;p->move_speed=8;p->jump_speed=8;p->gravity=24;p->grounded=0;
    if(!scene)return;
    for(i=0;i<nds_instance_child_count(scene);++i){const nds_instance* c=nds_instance_child_at(scene,i);nds_part_properties q;if(c&&nds_instance_get_class(c)==NDS_CLASS_SPAWN_POINT&&nds_part_get_properties(c,&q)==NDS_OK){p->position=q.position;p->position.y+=p->half_height+.5f;return;}}
}

void nds_player_update(nds_player_controller* p,const nds_instance* scene,float dt,int f,int b,int l,int r,int jump)
{
    float x,z,len,ox,oy,oz;if(!p||!scene)return;if(dt<0)dt=0;if(dt>.05f)dt=.05f;
    x=(float)(r-l);z=(float)(b-f);len=sqrtf(x*x+z*z);if(len>0){x/=len;z/=len;}p->velocity.x=x*p->move_speed;p->velocity.z=z*p->move_speed;
    if(jump&&p->grounded){p->velocity.y=p->jump_speed;p->grounded=0;}p->velocity.y-=p->gravity*dt;
    ox=p->position.x;oy=p->position.y;oz=p->position.z;p->grounded=0;
    p->position.x+=p->velocity.x*dt;collide(p,scene,0,ox,p->half_width);
    p->position.y+=p->velocity.y*dt;collide(p,scene,1,oy,p->half_height);
    p->position.z+=p->velocity.z*dt;collide(p,scene,2,oz,p->half_width);
    if(p->position.y<-100){p->position=(nds_vec3){0,10,0};p->velocity=(nds_vec3){0,0,0};}
}

void nds_player_apply_camera(const nds_player_controller* p,nds_camera* camera)
{
    if(!p||!camera)return;
    camera->position[0]=p->position.x;camera->position[1]=p->position.y+.35f;camera->position[2]=p->position.z;
    camera->target[0]=p->position.x;camera->target[1]=p->position.y+.35f;camera->target[2]=p->position.z-1;
}
