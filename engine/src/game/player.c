#include "engine/game/player.h"
#include "engine/render/mesh.h"
#include "engine/platform/platform.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int overlap(float amin,float amax,float bmin,float bmax){return amax>bmin&&amin<bmax;}
static float rad(float degrees){return degrees*0.0174532925199433f;}
static float deg(float radians){return radians*57.29577951308232f;}

static void bounds(const nds_part_properties* p,float* ax,float* bx,float* ay,float* by,float* az,float* bz){
    float x=p->size.x*.5f,y=p->size.y*.5f,z=p->size.z*.5f,rx=rad(p->rotation.x),ry=rad(p->rotation.y),rz=rad(p->rotation.z);
    float cx=cosf(rx),sx=sinf(rx),cy=cosf(ry),sy=sinf(ry),cz=cosf(rz),sz=sinf(rz);
    float ex=fabsf(cy*cz)*x+fabsf(sx*sy*cz-cx*sz)*y+fabsf(cx*sy*cz+sx*sz)*z;
    float ey=fabsf(cy*sz)*x+fabsf(sx*sy*sz+cx*cz)*y+fabsf(cx*sy*sz-sx*cz)*z;
    float ez=fabsf(sy)*x+fabsf(sx*cy)*y+fabsf(cx*cy)*z;
    *ax=p->position.x-ex;*bx=p->position.x+ex;*ay=p->position.y-ey;*by=p->position.y+ey;*az=p->position.z-ez;*bz=p->position.z+ez;
}

static void collide(nds_player_controller* pl,const nds_instance* root,int axis,float old,float half){
    size_t i,n=nds_instance_child_count(root);
    for(i=0;i<n;++i){
        const nds_instance* c=nds_instance_child_at(root,i);nds_part_properties p;
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
                else if(pl->velocity.y<0&&old-half>=by-.01f&&py0<by){pl->position.y=by+half;pl->velocity.y=0;}
                if(pl->velocity.y==0&&old-half>=by-.02f)pl->grounded=1;
            }else if(axis==2&&overlap(px0,px1,ax,bx)&&overlap(py0,py1,ay,by)){
                if(pl->velocity.z>0&&old+half<=az+.01f&&pz1>az){pl->position.z=az-half;pl->velocity.z=0;}
                else if(pl->velocity.z<0&&old-half>=bz-.01f&&pz0<bx){pl->position.z=bz+half;pl->velocity.z=0;}
            }
        }
        if(nds_instance_child_count(c))collide(pl,c,axis,old,half);
    }
}

static float move_toward(float current,float target,float max_delta){float delta=target-current;if(delta>max_delta)return current+max_delta;if(delta<-max_delta)return current-max_delta;return target;}
static float approach_angle(float current,float target,float max_delta){float delta=fmodf(target-current+180.0f,360.0f)-180.0f;if(delta>max_delta)delta=max_delta;else if(delta<-max_delta)delta=-max_delta;return current+delta;}

void nds_player_init(nds_player_controller* p,const nds_instance* scene){
    size_t i;if(!p)return;
    p->position=(nds_vec3){0,5,0};p->velocity=(nds_vec3){0,0,0};p->half_width=.45f;p->half_height=1.5f;
    p->move_speed=8;p->jump_speed=8;p->gravity=24;p->health=100.0f;p->grounded=0;p->alive=1;
    p->camera_yaw=0.0f;p->camera_pitch=12.0f;p->camera_distance=8.0f;p->facing_yaw=0.0f;p->animation_time=0.0f;
    p->launch_land_timer=0.0f;p->launch_land_services=0;p->launch_land_bridge_open=0;p->launch_land_rocket_active=0;p->third_person=1;
    if(!scene)return;
    for(i=0;i<nds_instance_child_count(scene);++i){const nds_instance* c=nds_instance_child_at(scene,i);nds_part_properties q;if(c&&nds_instance_get_class(c)==NDS_CLASS_SPAWN_POINT&&nds_part_get_properties(c,&q)==NDS_OK){p->position=q.position;p->position.y+=p->half_height+.5f;return;}}
}

void nds_player_damage(nds_player_controller* p,float amount){if(!p||!p->alive||amount<=0.0f)return;p->health-=amount;if(p->health<=0.0f){p->health=0.0f;p->alive=0;p->velocity=(nds_vec3){0,0,0};}}

static nds_instance* named_part(nds_instance* scene,const char* name){return scene&&name?nds_instance_find_child(scene,name):NULL;}
static void set_button_state(nds_instance* scene,const char* name,uint32_t color,float y_delta){nds_instance* button=named_part(scene,name);nds_part_properties props;if(!button||nds_part_get_properties(button,&props)!=NDS_OK)return;props.color_rgba=color;props.position.y+=y_delta;nds_part_set_properties(button,&props);}
static void set_part_position(nds_instance* scene,const char* name,nds_vec3 position){nds_instance* part=named_part(scene,name);nds_part_properties props;if(!part||nds_part_get_properties(part,&props)!=NDS_OK)return;props.position=position;nds_part_set_properties(part,&props);}

static int button_aimed_at(const nds_player_controller* p,const nds_part_properties* props){
    float dx=props->position.x-p->position.x,dy=props->position.y-(p->position.y+1.0f),dz=props->position.z-p->position.z;
    float distance=sqrtf(dx*dx+dy*dy+dz*dz),yaw,pitch,fx,fy,fz,dot;
    if(distance<0.7f||distance>9.0f)return 0;
    yaw=rad(p->camera_yaw);pitch=rad(p->camera_pitch);
    fx=-sinf(yaw)*cosf(pitch);fy=sinf(pitch);fz=-cosf(yaw)*cosf(pitch);
    dot=(dx/distance)*fx+(dy/distance)*fy+(dz/distance)*fz;
    return dot>=0.82f;
}

static void launch_land_update_world(nds_player_controller* p,nds_instance* scene,float dt){
    float offset;
    if(!p||!scene||strcmp(nds_instance_get_name(scene),"Launch Land")!=0)return;
    if(p->launch_land_bridge_open)set_part_position(scene,"Bridge",(nds_vec3){7,15.5f,0});
    if(!p->launch_land_rocket_active)return;
    p->launch_land_timer+=dt;
    offset=p->launch_land_timer<4.0f?p->launch_land_timer*7.0f:28.0f;
    set_part_position(scene,"RocketBody",(nds_vec3){25,8.0f+offset,0});
    set_part_position(scene,"RocketNose",(nds_vec3){36,8.0f+offset,0});
    set_part_position(scene,"RocketTank",(nds_vec3){25,8.0f+offset,0});
    set_part_position(scene,"RocketBoosterA",(nds_vec3){25,3.0f+offset,4});
    set_part_position(scene,"RocketBoosterB",(nds_vec3){25,3.0f+offset,-4});
    set_part_position(scene,"RocketCabin",(nds_vec3){31,7.0f+offset,0});
    set_part_position(scene,"RocketLaunchButton",(nds_vec3){33.5f,10.0f+offset,0});
}

static void launch_land_interact(nds_player_controller* p,nds_instance* scene){
    static int last_mouse_down=0;
    int mouse_down=platform_is_mouse_button_down(0);size_t i,n;
    if(!p||!scene||strcmp(nds_instance_get_name(scene),"Launch Land")!=0){last_mouse_down=mouse_down;return;}
    if(!mouse_down||last_mouse_down){last_mouse_down=mouse_down;return;}
    n=nds_instance_child_count(scene);
    for(i=0;i<n;++i){
        nds_instance* c=nds_instance_child_at(scene,i);nds_part_properties q;const char* name;
        if(!c||nds_instance_get_class(c)!=NDS_CLASS_PART)continue;
        name=nds_instance_get_name(c);if(!name)continue;
        if(strncmp(name,"ServiceTowerButton",18)!=0&&strcmp(name,"BridgeButton")!=0&&strcmp(name,"RocketLaunchButton")!=0)continue;
        if(nds_part_get_properties(c,&q)!=NDS_OK||!button_aimed_at(p,&q))continue;
        if(strncmp(name,"ServiceTowerButton",18)==0){uint8_t bit=(uint8_t)(strcmp(name,"ServiceTowerButtonLower")==0?1:2);if(!(p->launch_land_services&bit)){p->launch_land_services|=bit;set_button_state(scene,name,0x48e06eff,-.18f);}}
        else if(strcmp(name,"BridgeButton")==0&&p->launch_land_services==3&&!p->launch_land_bridge_open){p->launch_land_bridge_open=1;set_button_state(scene,name,0x48e06eff,-.18f);}
        else if(strcmp(name,"RocketLaunchButton")==0&&p->launch_land_bridge_open&&!p->launch_land_rocket_active){p->launch_land_rocket_active=1;p->launch_land_timer=0.0f;set_button_state(scene,name,0x48e06eff,-.18f);}
    }
    last_mouse_down=mouse_down;
}

void nds_player_update(nds_player_controller* p,const nds_instance* scene,float dt,int f,int b,int l,int r,int jump){
    float local_x,local_forward,len,ox,oy,oz,yaw,s,c,target_x,target_z,target_facing;const float acceleration=52.0f,deceleration=68.0f,facing_turn_speed=1080.0f;
    if(!p||!scene||!p->alive)return;if(dt<0)dt=0;if(dt>.05f)dt=.05f;
    if(p->third_person){if(platform_is_key_down(PLATFORM_KEY_Q))nds_player_zoom_camera(p,12.0f*dt);if(platform_is_key_down(PLATFORM_KEY_E))nds_player_zoom_camera(p,-12.0f*dt);}
    local_x=(float)(r-l);local_forward=(float)(f-b);len=sqrtf(local_x*local_x+local_forward*local_forward);if(len>0.001f){local_x/=len;local_forward/=len;}else{local_x=0;local_forward=0;}
    yaw=rad(p->camera_yaw);s=sinf(yaw);c=cosf(yaw);target_x=local_forward*(-s)+local_x*c;target_z=local_forward*(-c)+local_x*(-s);
    if(len>0.001f){p->velocity.x=move_toward(p->velocity.x,target_x*p->move_speed,acceleration*dt);p->velocity.z=move_toward(p->velocity.z,target_z*p->move_speed,acceleration*dt);target_facing=deg(atan2f(target_x,-target_z));p->facing_yaw=approach_angle(p->facing_yaw,target_facing,facing_turn_speed*dt);p->animation_time+=dt*(p->grounded?9.0f:4.0f);}else{p->velocity.x=move_toward(p->velocity.x,0.0f,deceleration*dt);p->velocity.z=move_toward(p->velocity.z,0.0f,deceleration*dt);p->animation_time+=dt*2.0f;}
    if(jump&&p->grounded){p->velocity.y=p->jump_speed;p->grounded=0;}p->velocity.y-=p->gravity*dt;ox=p->position.x;oy=p->position.y;oz=p->position.z;p->grounded=0;
    p->position.x+=p->velocity.x*dt;collide(p,scene,0,ox,p->half_width);p->position.y+=p->velocity.y*dt;collide(p,scene,1,oy,p->half_height);p->position.z+=p->velocity.z*dt;collide(p,scene,2,oz,p->half_width);if(p->position.y<-20)nds_player_damage(p,100.0f);
    launch_land_interact(p,(nds_instance*)scene);launch_land_update_world(p,(nds_instance*)scene,dt);
}

static nds_instance* child(nds_instance* model,const char* name){return nds_instance_find_child(model,name);}
static float signed_power(float value,float exponent){float magnitude=powf(fabsf(value),exponent);return value<0?-magnitude:magnitude;}

static nds_mesh* rounded_character_mesh(void){
    static nds_mesh mesh;static int initialized=0;const size_t segments=16,rings=8;const float exponent=.55f;size_t i,j,index;if(initialized)return mesh.vertices?&mesh:NULL;initialized=1;
    mesh.vertex_count=(segments+1)*(rings+1);mesh.index_count=segments*rings*6;mesh.vertices=(nds_mesh_vertex*)calloc(mesh.vertex_count,sizeof(*mesh.vertices));mesh.indices=(uint16_t*)calloc(mesh.index_count,sizeof(*mesh.indices));
    if(!mesh.vertices||!mesh.indices){free(mesh.vertices);free(mesh.indices);mesh.vertices=NULL;mesh.indices=NULL;return NULL;}
    for(j=0;j<=rings;++j){float v=-1.57079632679f+3.14159265359f*(float)j/(float)rings,cv=cosf(v),sv=sinf(v),cvp=signed_power(cv,exponent),svp=signed_power(sv,exponent);for(i=0;i<=segments;++i){float u=-3.14159265359f+6.28318530718f*(float)i/(float)segments,cu=cosf(u),su=sinf(u),cup=signed_power(cu,exponent),sup=signed_power(su,exponent);index=j*(segments+1)+i;mesh.vertices[index].x=.5f*cvp*cup;mesh.vertices[index].y=.5f*svp;mesh.vertices[index].z=.5f*cvp*sup;mesh.vertices[index].u=(float)i/(float)segments;mesh.vertices[index].v=(float)j/(float)rings;}}
    index=0;for(j=0;j<rings;++j)for(i=0;i<segments;++i){uint16_t a=(uint16_t)(j*(segments+1)+i),b=(uint16_t)(a+1),cc=(uint16_t)((j+1)*(segments+1)+i+1),d=(uint16_t)((j+1)*(segments+1)+i);mesh.indices[index++]=a;mesh.indices[index++]=b;mesh.indices[index++]=cc;mesh.indices[index++]=a;mesh.indices[index++]=cc;mesh.indices[index++]=d;}return &mesh;
}

static void set_part(nds_instance* part,nds_vec3 position,nds_vec3 size,uint32_t color,nds_vec3 rotation,int visible){nds_part_properties p;const char* name;if(!part||nds_part_get_properties(part,&p)!=NDS_OK)return;p.anchored=1;p.can_collide=0;p.visible=(uint8_t)(visible!=0);p.position=position;p.size=size;p.color_rgba=color;p.rotation=rotation;name=nds_instance_get_name(part);if(name&&strncmp(name,"Player",6)==0&&strcmp(name,"PlayerShadow")!=0)p.mesh=rounded_character_mesh();nds_part_set_properties(part,&p);}

nds_result nds_player_attach_visual(nds_player_controller* p,nds_instance* scene){
    nds_instance *model,*body,*head,*left_arm,*right_arm,*left_leg,*right_leg,*shadow;nds_part_properties prop;
    if(!p||!scene)return NDS_ERR_INVALID_ARG;if(nds_instance_find_child(scene,"PlayerAvatar"))return NDS_OK;model=nds_instance_create(NDS_CLASS_MODEL,"PlayerAvatar");body=nds_instance_create(NDS_CLASS_PART,"PlayerTorso");head=nds_instance_create(NDS_CLASS_PART,"PlayerHead");left_arm=nds_instance_create(NDS_CLASS_PART,"PlayerLeftArm");right_arm=nds_instance_create(NDS_CLASS_PART,"PlayerRightArm");left_leg=nds_instance_create(NDS_CLASS_PART,"PlayerLeftLeg");right_leg=nds_instance_create(NDS_CLASS_PART,"PlayerRightLeg");shadow=nds_instance_create(NDS_CLASS_PART,"PlayerShadow");
    if(!model||!body||!head||!left_arm||!right_arm||!left_leg||!right_leg||!shadow){if(model)nds_instance_destroy(model);return NDS_ERR_UNKNOWN;}
    memset(&prop,0,sizeof(prop));prop.anchored=1;prop.can_collide=0;prop.visible=1;prop.size=(nds_vec3){.95f,1.05f,.55f};prop.color_rgba=0x2f6fedff;prop.position=p->position;nds_part_set_properties(body,&prop);prop.size=(nds_vec3){.82f,.82f,.82f};prop.color_rgba=0xf0c9a4ff;prop.position=(nds_vec3){p->position.x,p->position.y+1,p->position.z};nds_part_set_properties(head,&prop);prop.size=(nds_vec3){.38f,1,.42f};prop.color_rgba=0xf0c9a4ff;prop.position=(nds_vec3){p->position.x-.68f,p->position.y+.02f,p->position.z};nds_part_set_properties(left_arm,&prop);prop.position=(nds_vec3){p->position.x+.68f,p->position.y+.02f,p->position.z};nds_part_set_properties(right_arm,&prop);prop.size=(nds_vec3){.42f,1,.48f};prop.color_rgba=0x27364dff;prop.position=(nds_vec3){p->position.x-.25f,p->position.y-.98f,p->position.z};nds_part_set_properties(left_leg,&prop);prop.position=(nds_vec3){p->position.x+.25f,p->position.y-.98f,p->position.z};nds_part_set_properties(right_leg,&prop);prop.size=(nds_vec3){1.65f,.025f,1.1f};prop.color_rgba=0x17202b70u;prop.transparency=.55f;prop.position=(nds_vec3){p->position.x,p->position.y-p->half_height+.025f,p->position.z};nds_part_set_properties(shadow,&prop);
    if(nds_instance_set_parent(model,scene)!=NDS_OK||nds_instance_set_parent(body,model)!=NDS_OK||nds_instance_set_parent(head,model)!=NDS_OK||nds_instance_set_parent(left_arm,model)!=NDS_OK||nds_instance_set_parent(right_arm,model)!=NDS_OK||nds_instance_set_parent(left_leg,model)!=NDS_OK||nds_instance_set_parent(right_leg,model)!=NDS_OK||nds_instance_set_parent(shadow,model)!=NDS_OK){nds_instance_destroy(model);return NDS_ERR_UNKNOWN;}return NDS_OK;
}

void nds_player_update_visual(const nds_player_controller* p,nds_instance* scene){
    nds_instance* model;float speed,phase,swing,bob,yaw,rx,rz,fx,fz;nds_vec3 body_pos,head_pos;if(!p||!scene)return;model=nds_instance_find_child(scene,"PlayerAvatar");if(!model)return;speed=sqrtf(p->velocity.x*p->velocity.x+p->velocity.z*p->velocity.z);phase=p->animation_time;swing=speed>.1f?28.0f*sinf(phase):0;bob=speed>.1f?.045f*fabsf(sinf(phase)):0;yaw=p->facing_yaw;rx=cosf(rad(yaw));rz=-sinf(rad(yaw));fx=-sinf(rad(yaw));fz=-cosf(rad(yaw));body_pos=(nds_vec3){p->position.x,p->position.y+bob,p->position.z};head_pos=(nds_vec3){p->position.x,p->position.y+1+bob,p->position.z};
    {float arm_swing=.5f*sinf(rad(swing)),leg_swing=.5f*sinf(rad(-swing));nds_vec3 la=(nds_vec3){body_pos.x-rx*.68f+fx*arm_swing,body_pos.y+.02f,body_pos.z-rz*.68f+fz*arm_swing};nds_vec3 ra=(nds_vec3){body_pos.x+rx*.68f+fx*(-arm_swing),body_pos.y+.02f,body_pos.z+rz*.68f+fz*(-arm_swing)};nds_vec3 ll=(nds_vec3){body_pos.x-rx*.25f+fx*leg_swing,body_pos.y-.98f,body_pos.z-rz*.25f+fz*leg_swing};nds_vec3 rl=(nds_vec3){body_pos.x+rx*.25f+fx*(-leg_swing),body_pos.y-.98f,body_pos.z+rz*.25f+fz*(-leg_swing)};int show=p->alive&&p->third_person;set_part(child(model,"PlayerLeftArm"),la,(nds_vec3){.38f,1,.42f},0xf0c9a4ff,(nds_vec3){swing,yaw,0},show);set_part(child(model,"PlayerRightArm"),ra,(nds_vec3){.38f,1,.42f},0xf0c9a4ff,(nds_vec3){-swing,yaw,0},show);set_part(child(model,"PlayerLeftLeg"),ll,(nds_vec3){.42f,1,.48f},0x27364dff,(nds_vec3){-swing,yaw,0},show);set_part(child(model,"PlayerRightLeg"),rl,(nds_vec3){.42f,1,.48f},0x27364dff,(nds_vec3){swing,yaw,0},show);}
    {int show=p->alive&&p->third_person;set_part(child(model,"PlayerTorso"),body_pos,(nds_vec3){.95f,1.05f,.55f},0x2f6fedff,(nds_vec3){0,yaw,0},show);set_part(child(model,"PlayerHead"),head_pos,(nds_vec3){.82f,.82f,.82f},0xf0c9a4ff,(nds_vec3){0,yaw,0},show);set_part(child(model,"PlayerShadow"),(nds_vec3){p->position.x,p->position.y-p->half_height+.025f,p->position.z},(nds_vec3){1.65f,.025f,1.1f},0x17202b70u,(nds_vec3){0,0,0},show);}
}

void nds_player_rotate_camera(nds_player_controller* p,float yaw_delta,float pitch_delta){if(!p)return;p->camera_yaw+=yaw_delta;p->camera_pitch+=pitch_delta;if(p->camera_pitch<5)p->camera_pitch=5;if(p->camera_pitch>55)p->camera_pitch=55;}
void nds_player_zoom_camera(nds_player_controller* p,float zoom_delta){if(!p||!p->third_person)return;p->camera_distance-=zoom_delta;if(p->camera_distance<2)p->camera_distance=2;if(p->camera_distance>24)p->camera_distance=24;}
void nds_player_set_third_person(nds_player_controller* p,int enabled){if(p)p->third_person=enabled?1:0;}

void nds_player_apply_camera(const nds_player_controller* p,nds_camera* camera){
    float yaw,pitch,cy,sy,cp,sp,dist;if(!p||!camera)return;yaw=rad(p->camera_yaw);pitch=rad(p->camera_pitch);cy=cosf(yaw);sy=sinf(yaw);cp=cosf(pitch);sp=sinf(pitch);dist=p->camera_distance;
    camera->target[0]=p->position.x;camera->target[1]=p->position.y+1.0f;camera->target[2]=p->position.z;
    if(!p->third_person){camera->position[0]=p->position.x;camera->position[1]=p->position.y+1.0f;camera->position[2]=p->position.z;camera->target[0]=p->position.x-sy*cp;camera->target[1]=p->position.y+1.0f+sp;camera->target[2]=p->position.z-cy*cp;return;}
    camera->position[0]=p->position.x+sy*cp*dist;camera->position[1]=p->position.y+sp*dist+1.0f;camera->position[2]=p->position.z+cy*cp*dist;
}
