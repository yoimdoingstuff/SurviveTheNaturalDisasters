#include "gles2_backend.h"
#include "engine/platform/gl_context.h"
#include "engine/render/math.h"
#include "engine/render/frustum.h"
#include "engine/core/log.h"
#include "engine/content/map_catalog.h"

#include <GL/gl.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_STREAM_DRAW 0x88E0
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F

typedef GLuint (*PFNGLCREATESHADERPROC)(GLenum);
typedef void (*PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar* const*, const GLint*);
typedef void (*PFNGLCOMPILESHADERPROC)(GLuint);
typedef void (*PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
typedef void (*PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (*PFNGLDELETESHADERPROC)(GLuint);
typedef GLuint (*PFNGLCREATEPROGRAMPROC)(void);
typedef void (*PFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef void (*PFNGLLINKPROGRAMPROC)(GLuint);
typedef void (*PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void (*PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (*PFNGLDELETEPROGRAMPROC)(GLuint);
typedef void (*PFNGLUSEPROGRAMPROC)(GLuint);
typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (*PFNGLUNIFORM4FPROC)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (*PFNGLUNIFORM1IPROC)(GLint, GLint);
typedef GLint (*PFNGLGETATTRIBLOCATIONPROC)(GLuint, const GLchar*);
typedef void (*PFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
typedef void (*PFNGLBINDBUFFERPROC)(GLenum, GLuint);
typedef void (*PFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void (*PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void (*PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void (*PFNGLGENTEXTURESPROC)(GLsizei, GLuint*);
typedef void (*PFNGLBINDTEXTUREPROC)(GLenum, GLuint);
typedef void (*PFNGLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
typedef void (*PFNGLTEXIMAGE2DPROC)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
typedef void (*PFNGLDELETETEXTURESPROC)(GLsizei, const GLuint*);
typedef void (*PFNGLACTIVETEXTUREPROC)(GLenum);

typedef struct nds_gl_api {
    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLUSEPROGRAMPROC glUseProgram;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
    PFNGLUNIFORM4FPROC glUniform4f;
    PFNGLUNIFORM1IPROC glUniform1i;
    PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    PFNGLGENTEXTURESPROC glGenTextures;
    PFNGLBINDTEXTUREPROC glBindTexture;
    PFNGLTEXPARAMETERIPROC glTexParameteri;
    PFNGLTEXIMAGE2DPROC glTexImage2D;
    PFNGLDELETETEXTURESPROC glDeleteTextures;
    PFNGLACTIVETEXTUREPROC glActiveTexture;
} nds_gl_api;

typedef struct nds_gpu_mesh { const nds_mesh* source; GLuint vertex_buffer; GLuint index_buffer; size_t index_count; } nds_gpu_mesh;
typedef struct nds_gpu_texture { const nds_texture* source; GLuint texture; } nds_gpu_texture;
#define NDS_GPU_MESH_CACHE_CAPACITY 128u
#define NDS_GPU_TEXTURE_CACHE_CAPACITY 128u

struct nds_gles2_backend {
    int width, height; float fov_y_degrees, near_plane, far_plane;
    GLuint program, vertex_buffer, index_buffer; GLint position_attrib, uv_attrib;
    GLint mvp_uniform, color_uniform, texture_uniform, use_texture_uniform; nds_gl_api gl;
    nds_gpu_mesh mesh_cache[NDS_GPU_MESH_CACHE_CAPACITY]; size_t mesh_cache_count;
    nds_gpu_texture texture_cache[NDS_GPU_TEXTURE_CACHE_CAPACITY]; size_t texture_cache_count;
};

static const GLfloat cube_vertices[] = {
    -0.5f,-0.5f,0.5f,0,0, 0.5f,-0.5f,0.5f,1,0, 0.5f,0.5f,0.5f,1,1, -0.5f,0.5f,0.5f,0,1,
    0.5f,-0.5f,-0.5f,0,0, -0.5f,-0.5f,-0.5f,1,0, -0.5f,0.5f,-0.5f,1,1, 0.5f,0.5f,-0.5f,0,1,
    -0.5f,0.5f,-0.5f,0,0, -0.5f,0.5f,0.5f,1,0, 0.5f,0.5f,0.5f,1,1, 0.5f,0.5f,-0.5f,0,1,
    -0.5f,-0.5f,0.5f,0,0, 0.5f,-0.5f,0.5f,1,0, 0.5f,-0.5f,-0.5f,1,1, -0.5f,-0.5f,-0.5f,0,1,
    0.5f,-0.5f,0.5f,0,0, 0.5f,0.5f,0.5f,1,0, 0.5f,0.5f,-0.5f,1,1, 0.5f,-0.5f,-0.5f,0,1,
    -0.5f,-0.5f,-0.5f,0,0, -0.5f,0.5f,-0.5f,1,0, -0.5f,0.5f,0.5f,1,1, -0.5f,-0.5f,0.5f,0,1
};
static const GLushort cube_indices[] = {0,1,2,0,2,3,4,5,6,4,6,7,8,9,10,8,10,11,12,13,14,12,14,15,16,17,18,16,18,19,20,21,22,20,22,23};

static const char* vertex_shader_source = "attribute vec3 a_position;\nattribute vec2 a_uv;\nuniform mat4 u_mvp;\nvarying vec2 v_uv;\nvoid main(){gl_Position=u_mvp*vec4(a_position,1.0);v_uv=a_uv;}\n";
static const char* fragment_shader_source = "#ifdef GL_ES\nprecision mediump float;\n#endif\nuniform vec4 u_color;\nuniform sampler2D u_texture;\nuniform int u_use_texture;\nvarying vec2 v_uv;\nvoid main(){vec4 base=u_color;if(u_use_texture!=0)base*=texture2D(u_texture,v_uv);gl_FragColor=base;}\n";

static nds_result compile_shader(nds_gles2_backend* b, GLenum type, const char* source, GLuint* out)
{ GLint ok=0; GLuint shader=b->gl.glCreateShader(type); if(!shader)return NDS_ERR_INIT_FAILED; b->gl.glShaderSource(shader,1,&source,NULL); b->gl.glCompileShader(shader); b->gl.glGetShaderiv(shader,GL_COMPILE_STATUS,&ok); if(!ok){char log[1024];GLsizei written=0;b->gl.glGetShaderInfoLog(shader,(GLsizei)sizeof(log)-1,&written,log);log[written<(GLsizei)sizeof(log)?written:(GLsizei)sizeof(log)-1]='\0';NDS_LOGE("gles2","shader compile failed: %s",log);b->gl.glDeleteShader(shader);return NDS_ERR_INIT_FAILED;}*out=shader;return NDS_OK; }
static nds_result create_program(nds_gles2_backend* b)
{ GLuint vs=0,fs=0,program=0;GLint ok=0;if(compile_shader(b,GL_VERTEX_SHADER,vertex_shader_source,&vs)!=NDS_OK)return NDS_ERR_INIT_FAILED;if(compile_shader(b,GL_FRAGMENT_SHADER,fragment_shader_source,&fs)!=NDS_OK){b->gl.glDeleteShader(vs);return NDS_ERR_INIT_FAILED;}program=b->gl.glCreateProgram();if(!program)goto fail;b->gl.glAttachShader(program,vs);b->gl.glAttachShader(program,fs);b->gl.glLinkProgram(program);b->gl.glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok){char log[1024];GLsizei written=0;b->gl.glGetProgramInfoLog(program,(GLsizei)sizeof(log)-1,&written,log);log[written<(GLsizei)sizeof(log)?written:(GLsizei)sizeof(log)-1]='\0';NDS_LOGE("gles2","program link failed: %s",log);goto fail;}b->program=program;b->position_attrib=b->gl.glGetAttribLocation(program,"a_position");b->uv_attrib=b->gl.glGetAttribLocation(program,"a_uv");b->mvp_uniform=b->gl.glGetUniformLocation(program,"u_mvp");b->color_uniform=b->gl.glGetUniformLocation(program,"u_color");b->texture_uniform=b->gl.glGetUniformLocation(program,"u_texture");b->use_texture_uniform=b->gl.glGetUniformLocation(program,"u_use_texture");b->gl.glDeleteShader(vs);b->gl.glDeleteShader(fs);return b->position_attrib>=0&&b->uv_attrib>=0&&b->mvp_uniform>=0&&b->color_uniform>=0&&b->texture_uniform>=0&&b->use_texture_uniform>=0?NDS_OK:NDS_ERR_INIT_FAILED;fail:if(program)b->gl.glDeleteProgram(program);b->gl.glDeleteShader(vs);b->gl.glDeleteShader(fs);return NDS_ERR_INIT_FAILED; }
static void make_model(nds_mat4* out,const nds_draw_part* p)
{ nds_mat4 t,s,rx,ry,rz,rxy,rxyz,rs;const float d=0.01745329251994329577f;float cx=cosf(p->rotation.x*d),sx=sinf(p->rotation.x*d),cy=cosf(p->rotation.y*d),sy=sinf(p->rotation.y*d),cz=cosf(p->rotation.z*d),sz=sinf(p->rotation.z*d);nds_mat4_translate(&t,p->position.x,p->position.y,p->position.z);nds_mat4_scale(&s,p->size.x,p->size.y,p->size.z);nds_mat4_identity(&rx);nds_mat4_identity(&ry);nds_mat4_identity(&rz);rx.m[5]=cx;rx.m[6]=sx;rx.m[9]=-sx;rx.m[10]=cx;ry.m[0]=cy;ry.m[2]=-sy;ry.m[8]=sy;ry.m[10]=cy;rz.m[0]=cz;rz.m[1]=sz;rz.m[4]=-sz;rz.m[5]=cz;nds_mat4_mul(&rxy,&ry,&rx);nds_mat4_mul(&rxyz,&rz,&rxy);nds_mat4_mul(&rs,&rxyz,&s);nds_mat4_mul(out,&t,&rs); }
static nds_aabb part_world_bounds(const nds_draw_part* p)
{ nds_mat4 model;nds_aabb bounds;int xi,yi,zi,first=1;make_model(&model,p);for(xi=0;xi<2;++xi){const float x=xi?0.5f:-0.5f;for(yi=0;yi<2;++yi){const float y=yi?0.5f:-0.5f;for(zi=0;zi<2;++zi){const float z=zi?0.5f:-0.5f;const float wx=model.m[0]*x+model.m[4]*y+model.m[8]*z+model.m[12],wy=model.m[1]*x+model.m[5]*y+model.m[9]*z+model.m[13],wz=model.m[2]*x+model.m[6]*y+model.m[10]*z+model.m[14];if(first){bounds.min.x=bounds.max.x=wx;bounds.min.y=bounds.max.y=wy;bounds.min.z=bounds.max.z=wz;first=0;}else{if(wx<bounds.min.x)bounds.min.x=wx;if(wx>bounds.max.x)bounds.max.x=wx;if(wy<bounds.min.y)bounds.min.y=wy;if(wy>bounds.max.y)bounds.max.y=wy;if(wz<bounds.min.z)bounds.min.z=wz;if(wz>bounds.max.z)bounds.max.z=wz;}}}}return bounds; }
static nds_result upload_gpu_mesh(nds_gles2_backend* b,const nds_mesh* mesh,nds_gpu_mesh* out)
{if(!b||!mesh||!mesh->vertices||!mesh->indices||!mesh->vertex_count||!mesh->index_count||!out)return NDS_ERR_INVALID_ARG;b->gl.glGenBuffers(1,&out->vertex_buffer);b->gl.glGenBuffers(1,&out->index_buffer);if(!out->vertex_buffer||!out->index_buffer)return NDS_ERR_UNKNOWN;b->gl.glBindBuffer(GL_ARRAY_BUFFER,out->vertex_buffer);b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(mesh->vertex_count*sizeof(*mesh->vertices)),mesh->vertices,GL_STATIC_DRAW);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,out->index_buffer);b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)(mesh->index_count*sizeof(*mesh->indices)),mesh->indices,GL_STATIC_DRAW);out->source=mesh;out->index_count=mesh->index_count;return NDS_OK;}
static nds_gpu_mesh* get_gpu_mesh(nds_gles2_backend* b,const nds_mesh* mesh)
{size_t i;nds_gpu_mesh* entry;if(!b||!mesh)return NULL;for(i=0;i<b->mesh_cache_count;++i)if(b->mesh_cache[i].source==mesh)return &b->mesh_cache[i];if(b->mesh_cache_count>=NDS_GPU_MESH_CACHE_CAPACITY)return NULL;entry=&b->mesh_cache[b->mesh_cache_count];*entry=(nds_gpu_mesh){0};if(upload_gpu_mesh(b,mesh,entry)!=NDS_OK){if(entry->vertex_buffer)b->gl.glDeleteBuffers(1,&entry->vertex_buffer);if(entry->index_buffer)b->gl.glDeleteBuffers(1,&entry->index_buffer);*entry=(nds_gpu_mesh){0};return NULL;}++b->mesh_cache_count;return entry;}
static nds_result upload_gpu_texture(nds_gles2_backend* b,const nds_texture* texture,nds_gpu_texture* out)
{if(!b||!texture||!texture->rgba8||!texture->width||!texture->height||!out)return NDS_ERR_INVALID_ARG;b->gl.glGenTextures(1,&out->texture);if(!out->texture)return NDS_ERR_UNKNOWN;b->gl.glActiveTexture(GL_TEXTURE0);b->gl.glBindTexture(GL_TEXTURE_2D,out->texture);b->gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);b->gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);b->gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);b->gl.glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);glPixelStorei(GL_UNPACK_ALIGNMENT,1);b->gl.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,(GLsizei)texture->width,(GLsizei)texture->height,0,GL_RGBA,GL_UNSIGNED_BYTE,texture->rgba8);glPixelStorei(GL_UNPACK_ALIGNMENT,4);out->source=texture;return NDS_OK;}
static nds_gpu_texture* get_gpu_texture(nds_gles2_backend* b,const nds_texture* texture)
{size_t i;nds_gpu_texture* entry;if(!b||!texture)return NULL;for(i=0;i<b->texture_cache_count;++i)if(b->texture_cache[i].source==texture)return &b->texture_cache[i];if(b->texture_cache_count>=NDS_GPU_TEXTURE_CACHE_CAPACITY)return NULL;entry=&b->texture_cache[b->texture_cache_count];*entry=(nds_gpu_texture){0};if(upload_gpu_texture(b,texture,entry)!=NDS_OK){if(entry->texture)b->gl.glDeleteTextures(1,&entry->texture);*entry=(nds_gpu_texture){0};return NULL;}++b->texture_cache_count;return entry;}
static float camera_depth(const nds_camera* camera,const nds_draw_part* p){nds_mat4 view;nds_camera_view_matrix(camera,&view);return view.m[2]*p->position.x+view.m[6]*p->position.y+view.m[10]*p->position.z+view.m[14];}
static void sort_transparent_parts(const nds_draw_list* list,const nds_camera* camera,size_t* order,size_t count){size_t i;for(i=0;i<count;++i)order[i]=i;for(i=1;i<count;++i){size_t key=order[i],j=i;float key_depth=camera_depth(camera,&list->parts[key]);while(j>0){float prev_depth=camera_depth(camera,&list->parts[order[j-1]]);if(prev_depth<=key_depth)break;order[j]=order[j-1];--j;}order[j]=key;}}
#define LOAD_GL(name,type) do{b->gl.name=(type)platform_gl_get_proc_address(#name);if(!b->gl.name)goto fail;}while(0)
nds_result nds_gles2_backend_create(nds_gles2_backend** out_backend,int width,int height,float fov_y_degrees,float near_plane,float far_plane)
{nds_gles2_backend* b;if(!out_backend||width<=0||height<=0)return NDS_ERR_INVALID_ARG;*out_backend=NULL;if(platform_gl_context_create()!=NDS_OK)return NDS_ERR_INIT_FAILED;b=(nds_gles2_backend*)calloc(1,sizeof(*b));if(!b){platform_gl_context_destroy();return NDS_ERR_UNKNOWN;}b->width=width;b->height=height;b->fov_y_degrees=fov_y_degrees>1?fov_y_degrees:70;b->near_plane=near_plane>0.001f?near_plane:0.1f;b->far_plane=far_plane>b->near_plane?far_plane:2000;LOAD_GL(glCreateShader,PFNGLCREATESHADERPROC);LOAD_GL(glShaderSource,PFNGLSHADERSOURCEPROC);LOAD_GL(glCompileShader,PFNGLCOMPILESHADERPROC);LOAD_GL(glGetShaderiv,PFNGLGETSHADERIVPROC);LOAD_GL(glGetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC);LOAD_GL(glDeleteShader,PFNGLDELETESHADERPROC);LOAD_GL(glCreateProgram,PFNGLCREATEPROGRAMPROC);LOAD_GL(glAttachShader,PFNGLATTACHSHADERPROC);LOAD_GL(glLinkProgram,PFNGLLINKPROGRAMPROC);LOAD_GL(glGetProgramiv,PFNGLGETPROGRAMIVPROC);LOAD_GL(glGetProgramInfoLog,PFNGLGETPROGRAMINFOLOGPROC);LOAD_GL(glDeleteProgram,PFNGLDELETEPROGRAMPROC);LOAD_GL(glUseProgram,PFNGLUSEPROGRAMPROC);LOAD_GL(glGetUniformLocation,PFNGLGETUNIFORMLOCATIONPROC);LOAD_GL(glUniformMatrix4fv,PFNGLUNIFORMMATRIX4FVPROC);LOAD_GL(glUniform4f,PFNGLUNIFORM4FPROC);LOAD_GL(glUniform1i,PFNGLUNIFORM1IPROC);LOAD_GL(glGetAttribLocation,PFNGLGETATTRIBLOCATIONPROC);LOAD_GL(glGenBuffers,PFNGLGENBUFFERSPROC);LOAD_GL(glBindBuffer,PFNGLBINDBUFFERPROC);LOAD_GL(glBufferData,PFNGLBUFFERDATAPROC);LOAD_GL(glDeleteBuffers,PFNGLDELETEBUFFERSPROC);LOAD_GL(glVertexAttribPointer,PFNGLVERTEXATTRIBPOINTERPROC);LOAD_GL(glEnableVertexAttribArray,PFNGLENABLEVERTEXATTRIBARRAYPROC);LOAD_GL(glDisableVertexAttribArray,PFNGLDISABLEVERTEXATTRIBARRAYPROC);LOAD_GL(glGenTextures,PFNGLGENTEXTURESPROC);LOAD_GL(glBindTexture,PFNGLBINDTEXTUREPROC);LOAD_GL(glTexParameteri,PFNGLTEXPARAMETERIPROC);LOAD_GL(glTexImage2D,PFNGLTEXIMAGE2DPROC);LOAD_GL(glDeleteTextures,PFNGLDELETETEXTURESPROC);LOAD_GL(glActiveTexture,PFNGLACTIVETEXTUREPROC);if(create_program(b)!=NDS_OK)goto fail;b->gl.glGenBuffers(1,&b->vertex_buffer);b->gl.glGenBuffers(1,&b->index_buffer);if(!b->vertex_buffer||!b->index_buffer)goto fail;b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer);b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_vertices),cube_vertices,GL_STATIC_DRAW);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer);b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_indices),cube_indices,GL_STATIC_DRAW);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDisable(GL_BLEND);glDepthMask(GL_TRUE);glViewport(0,0,width,height);b->gl.glUseProgram(b->program);b->gl.glUniform1i(b->texture_uniform,0);b->gl.glUniform1i(b->use_texture_uniform,0);*out_backend=b;return NDS_OK;fail:if(b){if(b->gl.glDeleteBuffers){if(b->vertex_buffer)b->gl.glDeleteBuffers(1,&b->vertex_buffer);if(b->index_buffer)b->gl.glDeleteBuffers(1,&b->index_buffer);}if(b->gl.glDeleteProgram&&b->program)b->gl.glDeleteProgram(b->program);free(b);}platform_gl_context_destroy();return NDS_ERR_INIT_FAILED;}
void nds_gles2_backend_destroy(nds_gles2_backend* b){size_t i;if(!b)return;if(b->gl.glDeleteTextures)for(i=0;i<b->texture_cache_count;++i)if(b->texture_cache[i].texture)b->gl.glDeleteTextures(1,&b->texture_cache[i].texture);if(b->gl.glDeleteBuffers){for(i=0;i<b->mesh_cache_count;++i){if(b->mesh_cache[i].vertex_buffer)b->gl.glDeleteBuffers(1,&b->mesh_cache[i].vertex_buffer);if(b->mesh_cache[i].index_buffer)b->gl.glDeleteBuffers(1,&b->mesh_cache[i].index_buffer);}if(b->vertex_buffer)b->gl.glDeleteBuffers(1,&b->vertex_buffer);if(b->index_buffer)b->gl.glDeleteBuffers(1,&b->index_buffer);}if(b->gl.glDeleteProgram&&b->program)b->gl.glDeleteProgram(b->program);free(b);platform_gl_context_destroy();}
nds_result nds_gles2_backend_resize(nds_gles2_backend* b,int width,int height){if(!b||width<=0||height<=0)return NDS_ERR_INVALID_ARG;b->width=width;b->height=height;glViewport(0,0,width,height);return NDS_OK;}
nds_result nds_gles2_backend_begin(nds_gles2_backend* b){if(!b)return NDS_ERR_INVALID_ARG;if(platform_gl_context_make_current()!=NDS_OK)return NDS_ERR_INIT_FAILED;glClearColor(0.055f,0.075f,0.10f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_BLEND);glDepthMask(GL_TRUE);b->gl.glUseProgram(b->program);return NDS_OK;}
static nds_result draw_one_part(nds_gles2_backend* b,const nds_draw_part* p,const nds_mat4* pv){nds_mat4 model,mvp;size_t index_count;nds_gpu_texture* texture=NULL;float r=(float)((p->color_rgba>>24)&0xff)/255.0f,g=(float)((p->color_rgba>>16)&0xff)/255.0f,bl=(float)((p->color_rgba>>8)&0xff)/255.0f,a=(float)(p->color_rgba&0xff)/255.0f;if(p->mesh&&p->mesh->vertices&&p->mesh->indices&&p->mesh->vertex_count&&p->mesh->index_count){nds_gpu_mesh* gpu=get_gpu_mesh(b,p->mesh);if(!gpu)return NDS_ERR_UNKNOWN;b->gl.glBindBuffer(GL_ARRAY_BUFFER,gpu->vertex_buffer);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gpu->index_buffer);index_count=gpu->index_count;}else{b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer);index_count=sizeof(cube_indices)/sizeof(cube_indices[0]);}b->gl.glVertexAttribPointer((GLuint)b->position_attrib,3,GL_FLOAT,GL_FALSE,(GLsizei)(5*sizeof(float)),(const void*)0);b->gl.glVertexAttribPointer((GLuint)b->uv_attrib,2,GL_FLOAT,GL_FALSE,(GLsizei)(5*sizeof(float)),(const void*)(3*sizeof(float)));b->gl.glEnableVertexAttribArray((GLuint)b->uv_attrib);if(p->texture){texture=get_gpu_texture(b,p->texture);if(!texture)return NDS_ERR_UNKNOWN;b->gl.glActiveTexture(GL_TEXTURE0);b->gl.glBindTexture(GL_TEXTURE_2D,texture->texture);}b->gl.glUniform1i(b->use_texture_uniform,texture?1:0);a*=1.0f-p->transparency;make_model(&model,p);nds_mat4_mul(&mvp,pv,&model);b->gl.glUniformMatrix4fv(b->mvp_uniform,1,GL_FALSE,mvp.m);b->gl.glUniform4f(b->color_uniform,r,g,bl,a);glDrawElements(GL_TRIANGLES,(GLsizei)index_count,GL_UNSIGNED_SHORT,(const void*)0);return NDS_OK;}
nds_result nds_gles2_backend_draw_parts(nds_gles2_backend* b,const nds_draw_list* list,const nds_camera* camera){nds_mat4 projection,view,pv;float aspect;size_t* transparent_order=NULL,transparent_count=0,i,oi;nds_result result=NDS_OK;if(!b||!list||!camera)return NDS_ERR_INVALID_ARG;aspect=b->height>0?(float)b->width/(float)b->height:1.0f;nds_camera_projection_matrix(camera,aspect,&projection);nds_camera_view_matrix(camera,&view);nds_mat4_mul(&pv,&projection,&view);transparent_order=list->count?(size_t*)malloc(list->count*sizeof(*transparent_order)):NULL;if(list->count&&!transparent_order)return NDS_ERR_UNKNOWN;for(i=0;i<list->count;++i)if(list->parts[i].visible&&list->parts[i].transparency>0.0f&&list->parts[i].transparency<1.0f)transparent_order[transparent_count++]=i;sort_transparent_parts(list,camera,transparent_order,transparent_count);b->gl.glEnableVertexAttribArray((GLuint)b->position_attrib);for(int pass=0;pass<2;++pass){int transparent_pass=pass!=0;if(transparent_pass){glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);}else{glDisable(GL_BLEND);glDepthMask(GL_TRUE);}if(transparent_pass){for(oi=0;oi<transparent_count;++oi){const nds_draw_part* p=&list->parts[transparent_order[oi]];nds_aabb bounds=part_world_bounds(p);if(!nds_frustum_aabb_visible(&pv,&bounds))continue;result=draw_one_part(b,p,&pv);if(result!=NDS_OK)goto done;}}else{for(i=0;i<list->count;++i){const nds_draw_part* p=&list->parts[i];if(!p->visible||p->transparency>=1.0f||p->transparency>0.0f)continue;nds_aabb bounds=part_world_bounds(p);if(!nds_frustum_aabb_visible(&pv,&bounds))continue;result=draw_one_part(b,p,&pv);if(result!=NDS_OK)goto done;}}}done:free(transparent_order);glDisable(GL_BLEND);glDepthMask(GL_TRUE);b->gl.glDisableVertexAttribArray((GLuint)b->position_attrib);b->gl.glDisableVertexAttribArray((GLuint)b->uv_attrib);b->gl.glUniform1i(b->use_texture_uniform,0);return result;}


typedef struct nds_hud_vertex {float x,y,u,v;} nds_hud_vertex;
static void hud_quad(nds_hud_vertex* vertices,GLushort* indices,size_t* quad_count,float x,float y,float w,float h){size_t q;if(!vertices||!indices||!quad_count||*quad_count>=8192u)return;q=*quad_count;vertices[q*4+0]=(nds_hud_vertex){x,y,0,0};vertices[q*4+1]=(nds_hud_vertex){x+w,y,1,0};vertices[q*4+2]=(nds_hud_vertex){x+w,y+h,1,1};vertices[q*4+3]=(nds_hud_vertex){x,y+h,0,1};indices[q*6+0]=(GLushort)(q*4+0);indices[q*6+1]=(GLushort)(q*4+1);indices[q*6+2]=(GLushort)(q*4+2);indices[q*6+3]=(GLushort)(q*4+0);indices[q*6+4]=(GLushort)(q*4+2);indices[q*6+5]=(GLushort)(q*4+3);*quad_count=q+1;}
static unsigned char hud_font_row(char c,int row){static const unsigned char f[36][7]={
{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,14},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},{7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},{17,17,10,4,4,4,4},{31,1,2,4,8,16,31},{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{30,1,1,14,1,1,30},{14,17,1,6,8,16,31},{14,17,1,6,1,17,14},{17,17,14,1,1,17,14},{14,17,3,13,17,17,14},{14,17,1,7,1,17,14}};int i=-1;if(c>='A'&&c<='Z')i=c-'A';else if(c>='0'&&c<='9')i=26+(c-'0');return i<0?0:f[i][row<7?row:0];}
static void hud_text(nds_hud_vertex* vertices,GLushort* indices,size_t* quads,float x,float y,float scale,const char* text){const char* p=text;float start=x;while(*p){char c=*p++;if(c=='\n'){y+=8.0f*scale;x=start;continue;}if(c==' '){x+=4.0f*scale;continue;}for(int row=0;row<7;++row){unsigned char bits=hud_font_row((char)((c>='a'&&c<='z')?c-'a'+'A':c),row);for(int col=0;col<5;++col)if(bits&(1u<<(4-col)))hud_quad(vertices,indices,quads,x+col*scale,y+row*scale,scale,scale);}x+=6.0f*scale;}}
static void hud_draw_batch(nds_gles2_backend* b,nds_hud_vertex* vertices,GLushort* indices,size_t quads,float r,float g,float bl,float a){if(!quads)return;b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer);b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(quads*4*sizeof(vertices[0])),vertices,GL_STREAM_DRAW);b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)(quads*6*sizeof(indices[0])),indices,GL_STREAM_DRAW);b->gl.glVertexAttribPointer((GLuint)b->position_attrib,2,GL_FLOAT,GL_FALSE,(GLsizei)sizeof(nds_hud_vertex),(const void*)0);b->gl.glVertexAttribPointer((GLuint)b->uv_attrib,2,GL_FLOAT,GL_FALSE,(GLsizei)sizeof(nds_hud_vertex),(const void*)(2*sizeof(float)));b->gl.glEnableVertexAttribArray((GLuint)b->position_attrib);b->gl.glEnableVertexAttribArray((GLuint)b->uv_attrib);b->gl.glUniform4f(b->color_uniform,r,g,bl,a);glDrawElements(GL_TRIANGLES,(GLsizei)(quads*6),GL_UNSIGNED_SHORT,(const void*)0);}
nds_result nds_gles2_backend_draw_hud(nds_gles2_backend* b,const nds_hud_state* state){const size_t max_quads=8192u;nds_hud_vertex* vertices=NULL;GLushort* indices=NULL;nds_hud_vertex* text_vertices=NULL;GLushort* text_indices=NULL;size_t quads=0,text_quads=0;vertices=(nds_hud_vertex*)malloc(max_quads*4*sizeof(*vertices));indices=(GLushort*)malloc(max_quads*6*sizeof(*indices));text_vertices=(nds_hud_vertex*)malloc(max_quads*4*sizeof(*text_vertices));text_indices=(GLushort*)malloc(max_quads*6*sizeof(*text_indices));if(!vertices||!indices||!text_vertices||!text_indices){free(vertices);free(indices);free(text_vertices);free(text_indices);return NDS_ERR_UNKNOWN;}float health=0.0f,timer=0.0f,warning=0.0f;uint32_t round_number=0;nds_mat4 ortho;if(!b||!state)return NDS_ERR_INVALID_ARG;if(state->player){health=state->player->health/100.0f;if(health<0)health=0;if(health>1)health=1;}if(state->round){timer=1.0f-nds_round_phase_progress(state->round);round_number=state->round->round_number;}if(state->disasters&&nds_disaster_is_warning(state->disasters)){warning=nds_disaster_warning_remaining(state->disasters)/3.0f;if(warning<0)warning=0;if(warning>1)warning=1;}ortho.m[0]=2.0f/(float)b->width;ortho.m[1]=0;ortho.m[2]=0;ortho.m[3]=0;ortho.m[4]=0;ortho.m[5]=-2.0f/(float)b->height;ortho.m[6]=0;ortho.m[7]=0;ortho.m[8]=0;ortho.m[9]=0;ortho.m[10]=-1;ortho.m[11]=0;ortho.m[12]=-1;ortho.m[13]=1;ortho.m[14]=0;ortho.m[15]=1;b->gl.glUseProgram(b->program);glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);b->gl.glUniformMatrix4fv(b->mvp_uniform,1,GL_FALSE,ortho.m);b->gl.glUniform1i(b->use_texture_uniform,0);if(state->menu_active){float w=(float)b->width,h=(float)b->height;float panel_w=w<720?w-48:720.0f;float panel_h=h<620?h-48:500.0f;float px=(w-panel_w)*0.5f,py=(h-panel_h)*0.5f;float s=w<900?2.0f:2.5f;const char* map_name=nds_map_catalog_name(nds_map_catalog_selected());hud_quad(vertices,indices,&quads,0,0,w,h);hud_quad(vertices,indices,&quads,px,py,panel_w,panel_h);hud_quad(vertices,indices,&quads,px+10,py+10,panel_w-20,6);hud_quad(vertices,indices,&quads,px+28,py+86,panel_w-56,104);hud_quad(vertices,indices,&quads,px+28,py+208,panel_w-56,78);hud_quad(vertices,indices,&quads,px+28,py+306,panel_w-56,62);hud_quad(vertices,indices,&quads,px+28,py+390,panel_w-56,70);hud_draw_batch(b,vertices,indices,quads,0.015f,0.025f,0.045f,0.90f);text_quads=0;hud_text(text_vertices,text_indices,&text_quads,px+42,py+34,s,"NATURAL DISASTER SURVIVAL");hud_text(text_vertices,text_indices,&text_quads,px+48,py+104,1.5f,"PLAY");hud_text(text_vertices,text_indices,&text_quads,px+48,py+126,1.25f,"ENTER TO START");hud_text(text_vertices,text_indices,&text_quads,px+48,py+226,1.5f,"SELECT MAP");hud_text(text_vertices,text_indices,&text_quads,px+48,py+254,1.8f,"<");hud_text(text_vertices,text_indices,&text_quads,px+78,py+254,1.7f,map_name?map_name:"UNKNOWN MAP");hud_text(text_vertices,text_indices,&text_quads,px+panel_w-70,py+254,1.8f,">");hud_text(text_vertices,text_indices,&text_quads,px+48,py+322,1.35f,"I IMPORT ROBLOX MAP");hud_text(text_vertices,text_indices,&text_quads,px+48,py+406,1.1f,"WASD MOVE  SPACE JUMP");hud_text(text_vertices,text_indices,&text_quads,px+48,py+426,1.1f,"RMB CAMERA  C THIRD PERSON");hud_text(text_vertices,text_indices,&text_quads,px+48,py+448,1.1f,"M MENU");hud_draw_batch(b,text_vertices,text_indices,text_quads,0.88f,0.96f,1.0f,0.96f);}else{hud_quad(vertices,indices,&quads,24,24,260,30);hud_quad(vertices,indices,&quads,28,28,252,22*health);hud_quad(vertices,indices,&quads,24,h-54,260,30);hud_quad(vertices,indices,&quads,28,h-50,252*timer,22);if(warning>0){hud_quad(vertices,indices,&quads,w*0.5f-180,48,360,42);hud_quad(vertices,indices,&quads,w*0.5f-174,54,348*warning,30);}for(int i=0;i<10;++i)if(i<(int)(round_number%10))hud_quad(vertices,indices,&quads,w-210+i*18,24,14,14);if(!state->player||!state->player->alive)hud_quad(vertices,indices,&quads,w*0.5f-120,h-90,240,48);hud_draw_batch(b,vertices,indices,quads,0.02f,0.03f,0.04f,0.78f);hud_draw_batch(b,vertices,indices,1,0,0,0,0);text_quads=0;hud_text(text_vertices,text_indices,&text_quads,30,31,1.0f,"HEALTH");hud_text(text_vertices,text_indices,&text_quads,30,h-48,1.0f,"SURVIVAL");if(warning>0)hud_text(text_vertices,text_indices,&text_quads,w*0.5f-105,62,1.0f,"WARNING");hud_text(text_vertices,text_indices,&text_quads,w-188,26,0.9f,"ROUND");if(!state->player||!state->player->alive)hud_text(text_vertices,text_indices,&text_quads,w*0.5f-77,h-77,1.2f,"ELIMINATED");hud_draw_batch(b,text_vertices,text_indices,text_quads,0.92f,0.96f,1.0f,0.95f);}glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);b->gl.glDisableVertexAttribArray((GLuint)b->position_attrib);b->gl.glDisableVertexAttribArray((GLuint)b->uv_attrib);b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer);b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_vertices),cube_vertices,GL_STATIC_DRAW);b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer);b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_indices),cube_indices,GL_STATIC_DRAW);b->gl.glUniform1i(b->use_texture_uniform,0);free(vertices);free(indices);free(text_vertices);free(text_indices);return NDS_OK;}
nds_result nds_gles2_backend_end(nds_gles2_backend* b){if(!b)return NDS_ERR_INVALID_ARG;platform_gl_swap_buffers();return NDS_OK;}
