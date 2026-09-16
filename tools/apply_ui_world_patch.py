from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
MARK = "NDS_UI_WORLD_PATCH_V2"


def replace_once(path: str, pattern: str, replacement: str, flags=0):
    p = ROOT / path
    s = p.read_text(encoding="utf-8")
    if MARK in s:
        return False
    n, out = re.subn(pattern, replacement, s, count=1, flags=flags)
    if n != 1:
        raise RuntimeError(f"patch target not found: {path}: {pattern[:120]}")
    p.write_text(out, encoding="utf-8")
    return True


def patch_header():
    replace_once(
        "engine/include/engine/render/gles2.h",
        r"typedef struct nds_hud_state \{\s*const nds_player_controller\* player;\s*const nds_round\* round;\s*const nds_disaster_system\* disasters;\s*\} nds_hud_state;",
        "typedef struct nds_hud_state {\n    const nds_player_controller* player;\n    const nds_round* round;\n    const nds_disaster_system* disasters;\n    int menu_active;\n} nds_hud_state; /* " + MARK + " */",
        re.S,
    )


def patch_app():
    p = ROOT / "engine/src/core/app.c"
    s = p.read_text(encoding="utf-8")
    if MARK in s:
        return
    old = "int menu_active=smoke_mode?0:1;int menu_previous=0,import_previous=0,enter_previous=0;"
    new = "int menu_active=smoke_mode?0:1;int menu_previous=0,import_previous=0,enter_previous=0;int menu_left_previous=0,menu_right_previous=0;size_t menu_map_index=nds_map_catalog_selected();"
    if old not in s:
        raise RuntimeError("app menu state declaration not found")
    s = s.replace(old, new, 1)
    old2 = "if(menu_key&&!menu_previous)menu_active=!menu_active;menu_previous=menu_key;if(enter_key&&!enter_previous&&menu_active)menu_active=0;enter_previous=enter_key;"
    new2 = "if(menu_key&&!menu_previous)menu_active=!menu_active;menu_previous=menu_key;{int menu_left=platform_is_key_down(PLATFORM_KEY_LEFT);int menu_right=platform_is_key_down(PLATFORM_KEY_RIGHT);if(menu_active&&menu_left&&!menu_left_previous){size_t count=nds_map_catalog_count();if(count){menu_map_index=(menu_map_index+count-1u)%count;nds_map_catalog_set_selected(menu_map_index);}}if(menu_active&&menu_right&&!menu_right_previous){size_t count=nds_map_catalog_count();if(count){menu_map_index=(menu_map_index+1u)%count;nds_map_catalog_set_selected(menu_map_index);}}menu_left_previous=menu_left;menu_right_previous=menu_right;}if(enter_key&&!enter_previous&&menu_active)menu_active=0;enter_previous=enter_key;/* " + MARK + " */"
    if old2 not in s:
        raise RuntimeError("app menu input block not found")
    s = s.replace(old2, new2, 1)
    old3 = "nds_hud_state hud={&player,&round,&disasters};"
    new3 = "nds_hud_state hud={&player,&round,&disasters,menu_active};"
    if old3 not in s:
        raise RuntimeError("HUD state construction not found")
    s = s.replace(old3, new3, 1)
    p.write_text(s, encoding="utf-8")


def patch_backend():
    p = ROOT / "engine/src/render/gles2_backend.c"
    s = p.read_text(encoding="utf-8")
    if MARK in s:
        return
    if '#include "engine/content/map_catalog.h"' not in s:
        s = s.replace('#include "engine/core/log.h"', '#include "engine/core/log.h"\n#include "engine/content/map_catalog.h"', 1)

    helper = r'''
typedef struct nds_hud_vertex {float x,y,u,v;} nds_hud_vertex;
static void hud_quad(nds_hud_vertex* vertices,GLushort* indices,size_t* quad_count,float x,float y,float w,float h){size_t q=*quad_count;vertices[q*4+0]=(nds_hud_vertex){x,y,0,0};vertices[q*4+1]=(nds_hud_vertex){x+w,y,1,0};vertices[q*4+2]=(nds_hud_vertex){x+w,y+h,1,1};vertices[q*4+3]=(nds_hud_vertex){x,y+h,0,1};indices[q*6+0]=(GLushort)(q*4+0);indices[q*6+1]=(GLushort)(q*4+1);indices[q*6+2]=(GLushort)(q*4+2);indices[q*6+3]=(GLushort)(q*4+0);indices[q*6+4]=(GLushort)(q*4+2);indices[q*6+5]=(GLushort)(q*4+3);*quad_count=q+1;}
static unsigned char hud_font_row(char c,int row){static const unsigned char f[36][7]={
{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,14},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},{7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},{17,17,10,4,4,4,4},{31,1,2,4,8,16,31},{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{30,1,1,14,1,1,30},{14,17,1,6,8,16,31},{14,17,1,6,1,17,14},{17,17,14,1,1,17,14},{14,17,3,13,17,17,14},{14,17,1,7,1,17,14}};int i=-1;if(c>='A'&&c<='Z')i=c-'A';else if(c>='0'&&c<='9')i=26+(c-'0');return i<0?0:f[i][row<7?row:0];}
static void hud_text(nds_hud_vertex* vertices,GLushort* indices,size_t* quads,float x,float y,float scale,const char* text){const char* p=text;float start=x;while(*p){char c=*p++;if(c=='\n'){y+=8.0f*scale;x=start;continue;}if(c==' '){x+=4.0f*scale;continue;}for(int row=0;row<7;++row){unsigned char bits=hud_font_row((char)((c>='a'&&c<='z')?c-'a'+'A':c),row);for(int col=0;col<5;++col)if(bits&(1u<<(4-col)))hud_quad(vertices,indices,quads,x+col*scale,y+row*scale,scale,scale);}x+=6.0f*scale;}}
static void hud_draw_batch(nds_gles2_backend* b,nds_hud_vertex* vertices,GLushort* indices,size_t quads,float r,float g,float bl,float a){if(!quads)return;b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer);b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(quads*4*sizeof(vertices[0])),vertices,GL_STREAM_DRAW);b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)(quads*6*sizeof(indices[0])),indices,GL_STREAM_DRAW);b->gl.glVertexAttribPointer((GLuint)b->position_attrib,2,GL_FLOAT,GL_FALSE,(GLsizei)sizeof(nds_hud_vertex),(const void*)0);b->gl.glVertexAttribPointer((GLuint)b->uv_attrib,2,GL_FLOAT,GL_FALSE,(GLsizei)sizeof(nds_hud_vertex),(const void*)(2*sizeof(float)));b->gl.glEnableVertexAttribArray((GLuint)b->position_attrib);b->gl.glEnableVertexAttribArray((GLuint)b->uv_attrib);b->gl.glUniform4f(b->color_uniform,r,g,bl,a);glDrawElements(GL_TRIANGLES,(GLsizei)(quads*6),GL_UNSIGNED_SHORT,(const void*)0);}
'''
    s = re.sub(r'typedef struct nds_hud_vertex \{.*?nds_result nds_gles2_backend_draw_hud\(', helper + 'nds_result nds_gles2_backend_draw_hud(', s, count=1, flags=re.S)
    new_func = r'''nds_result nds_gles2_backend_draw_hud(nds_gles2_backend* b,const nds_hud_state* state){nds_hud_vertex vertices[4096];GLushort indices[6144];nds_hud_vertex text_vertices[4096];GLushort text_indices[6144];size_t quads=0,text_quads=0;float health=0.0f,timer=0.0f,warning=0.0f;uint32_t round_number=0;nds_mat4 ortho;if(!b||!state)return NDS_ERR_INVALID_ARG;if(state->player){health=state->player->health/100.0f;if(health<0)health=0;if(health>1)health=1;}if(state->round){timer=1.0f-nds_round_phase_progress(state->round);round_number=state->round->round_number;}if(state->disasters&&nds_disaster_is_warning(state->disasters)){warning=nds_disaster_warning_remaining(state->disasters)/3.0f;if(warning<0)warning=0;if(warning>1)warning=1;}ortho.m[0]=2.0f/(float)b->width;ortho.m[1]=0;ortho.m[2]=0;ortho.m[3]=0;ortho.m[4]=0;ortho.m[5]=-2.0f/(float)b->height;ortho.m[6]=0;ortho.m[7]=0;ortho.m[8]=0;ortho.m[9]=0;ortho.m[10]=-1;ortho.m[11]=0;ortho.m[12]=-1;ortho.m[13]=1;ortho.m[14]=0;ortho.m[15]=1;b->gl.glUseProgram(b->program);glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);b->gl.glUniformMatrix4fv(b->mvp_uniform,1,GL_FALSE,ortho.m);b->gl.glUniform1i(b->use_texture_uniform,0);if(state->menu_active){float w=(float)b->width,h=(float)b->height;float panel_w=w<720?w-48:720.0f;float panel_h=h<620?h-48:500.0f;float px=(w-panel_w)*0.5f,py=(h-panel_h)*0.5f;float s=w<900?2.0f:2.5f;const char* map_name=nds_map_catalog_name(nds_map_catalog_selected());hud_quad(vertices,indices,&quads,0,0,w,h);hud_quad(vertices,indices,&quads,px,py,panel_w,panel_h);hud_quad(vertices,indices,&quads,px+10,py+10,panel_w-20,6);hud_quad(vertices,indices,&quads,px+28,py+86,panel_w-56,104);hud_quad(vertices,indices,&quads,px+28,py+208,panel_w-56,78);hud_quad(vertices,indices,&quads,px+28,py+306,panel_w-56,62);hud_quad(vertices,indices,&quads,px+28,py+390,panel_w-56,70);hud_draw_batch(b,vertices,indices,quads,0.015f,0.025f,0.045f,0.90f);text_quads=0;hud_text(text_vertices,text_indices,&text_quads,px+42,py+34,s,"NATURAL DISASTER SURVIVAL");hud_text(text_vertices,text_indices,&text_quads,px+48,py+104,1.5f,"PLAY");hud_text(text_vertices,text_indices,&text_quads,px+48,py+126,1.25f,"ENTER TO START");hud_text(text_vertices,text_indices,&text_quads,px+48,py+226,1.5f,"SELECT MAP");hud_text(text_vertices,text_indices,&text_quads,px+48,py+254,1.8f,"<");hud_text(text_vertices,text_indices,&text_quads,px+78,py+254,1.7f,map_name?map_name:"UNKNOWN MAP");hud_text(text_vertices,text_indices,&text_quads,px+panel_w-70,py+254,1.8f,">");hud_text(text_vertices,text_indices,&text_quads,px+48,py+322,1.35f,"I IMPORT ROBLOX MAP");hud_text(text_vertices,text_indices,&text_quads,px+48,py+406,1.1f,"WASD MOVE  SPACE JUMP");hud_text(text_vertices,text_indices,&text_quads,px+48,py+426,1.1f,"RMB CAMERA  C THIRD PERSON");hud_text(text_vertices,text_indices,&text_quads,px+48,py+448,1.1f,"M MENU");hud_draw_batch(b,text_vertices,text_indices,text_quads,0.88f,0.96f,1.0f,0.96f);}else{hud_quad(vertices,indices,&quads,24,24,260,30);hud_quad(vertices,indices,&quads,28,28,252,22*health);hud_quad(vertices,indices,&quads,24,h-54,260,30);hud_quad(vertices,indices,&quads,28,h-50,252*timer,22);if(warning>0){hud_quad(vertices,indices,&quads,w*0.5f-180,48,360,42);hud_quad(vertices,indices,&quads,w*0.5f-174,54,348*warning,30);}for(int i=0;i<10;++i)if(i<(int)(round_number%10))hud_quad(vertices,indices,&quads,w-210+i*18,24,14,14);if(!state->player||!state->player->alive)hud_quad(vertices,indices,&quads,w*0.5f-120,h-90,240,48);hud_draw_batch(b,vertices,indices,quads,0.02f,0.03f,0.04f,0.78f);hud_draw_batch(b,vertices,indices,1,0,0,0,0);text_quads=0;hud_text(text_vertices,text_indices,&text_quads,30,31,1.0f,"HEALTH");hud_text(text_vertices,text_indices,&text_quads,30,h-48,1.0f,"SURVIVAL");if(warning>0)hud_text(text_vertices,text_indices,&text_quads,w*0.5f-105,62,1.0f,"WARNING");hud_text(text_vertices,text_indices,&text_quads,w-188,26,0.9f,"ROUND");if(!state->player||!state->player->alive)hud_text(text_vertices,text_indices,&text_quads,w*0.5f-77,h-77,1.2f,"ELIMINATED");hud_draw_batch(b,text_vertices,text_indices,text_quads,0.92f,0.96f,1.0f,0.95f);}glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);b->gl.glDisableVertexAttribArray((GLuint)b->position_attrib);b->gl.glDisableVertexAttribArray((GLuint)b->uv_attrib);b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer);b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_vertices),cube_vertices,GL_STATIC_DRAW);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer);b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_indices),cube_indices,GL_STATIC_DRAW);b->gl.glUniform1i(b->use_texture_uniform,0);return NDS_OK;}
'''
    s, n = re.subn(r'nds_result nds_gles2_backend_draw_hud\(.*?\nnds_result nds_gles2_backend_end\(', new_func + 'nds_result nds_gles2_backend_end(', s, count=1, flags=re.S)
    if n != 1:
        raise RuntimeError("HUD function replacement failed")
    p.write_text(s, encoding="utf-8")


def patch_world_loader():
    p = ROOT / "engine/src/content/map_loader.c"
    s = p.read_text(encoding="utf-8")
    if MARK in s:
        return
    helper = r'''
static void add_world_shell(nds_instance* root){nds_instance* ocean,*base,*beach;if(!root)return;ocean=nds_instance_create(NDS_CLASS_PART,"Ocean");base=nds_instance_create(NDS_CLASS_PART,"IslandBase");beach=nds_instance_create(NDS_CLASS_PART,"IslandShore");if(!ocean||!base||!beach){if(ocean)nds_instance_destroy(ocean);if(base)nds_instance_destroy(base);if(beach)nds_instance_destroy(beach);return;}nds_instance_set_parent(ocean,root);nds_instance_set_parent(base,root);nds_instance_set_parent(beach,root);{nds_part_properties p;memset(&p,0,sizeof(p));p.position=(nds_vec3){0,-2.15f,0};p.size=(nds_vec3){80,0.5f,80};p.rotation=(nds_vec3){0,0,0};p.color_rgba=0x1976CFFF;p.transparency=0.08f;p.reflectance=0.05f;p.anchored=1;p.can_collide=0;nds_part_set_properties(ocean,&p);p.position=(nds_vec3){0,-1.05f,0};p.size=(nds_vec3){34,1.0f,28};p.color_rgba=0x28583FFF;p.transparency=0.0f;p.reflectance=0.0f;p.can_collide=1;nds_part_set_properties(base,&p);p.position=(nds_vec3){0,-0.42f,0};p.size=(nds_vec3){37,0.45f,31};p.color_rgba=0xC2A15BFF;p.can_collide=1;nds_part_set_properties(beach,&p);}}
'''
    anchor = "static nds_result resolve_meshes_recursive"
    if anchor not in s:
        raise RuntimeError("map loader insertion point not found")
    s = s.replace(anchor, helper + "\n" + anchor, 1)
    target = "free(objects);free_entries(entries,count);if(!root)return NDS_ERR_IO;*out_root=root;return NDS_OK;}"
    if target not in s:
        raise RuntimeError("map loader return tail not found")
    s = s.replace(target, "free(objects);free_entries(entries,count);if(!root)return NDS_ERR_IO;add_world_shell(root);/* " + MARK + " */*out_root=root;return NDS_OK;}", 1)
    p.write_text(s, encoding="utf-8")


patch_header()
patch_app()
patch_backend()
patch_world_loader()
print("UI/world patch applied")
