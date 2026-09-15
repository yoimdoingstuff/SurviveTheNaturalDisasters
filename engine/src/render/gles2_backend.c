#include "gles2_backend.h"
#include "engine/platform/gl_context.h"
#include "engine/render/math.h"
#include "engine/core/log.h"

#include <GL/gl.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82

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
typedef GLint (*PFNGLGETATTRIBLOCATIONPROC)(GLuint, const GLchar*);
typedef void (*PFNGLGENBUFFERSPROC)(GLsizei, GLuint*);
typedef void (*PFNGLBINDBUFFERPROC)(GLenum, GLuint);
typedef void (*PFNGLBUFFERDATAPROC)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void (*PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint);
typedef void (*PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint);

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
    PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
} nds_gl_api;

struct nds_gles2_backend {
    int width, height;
    float fov_y_degrees, near_plane, far_plane;
    GLuint program, vertex_buffer, index_buffer;
    GLint position_attrib, mvp_uniform, color_uniform;
    nds_gl_api gl;
};

static const GLfloat cube_vertices[] = {
    -0.5f,-0.5f,0.5f, 0.5f,-0.5f,0.5f, 0.5f,0.5f,0.5f, -0.5f,0.5f,0.5f,
    0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,0.5f,-0.5f, 0.5f,0.5f,-0.5f,
    -0.5f,0.5f,-0.5f, -0.5f,0.5f,0.5f, 0.5f,0.5f,0.5f, 0.5f,0.5f,-0.5f,
    -0.5f,-0.5f,0.5f, 0.5f,-0.5f,0.5f, 0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
    0.5f,-0.5f,0.5f, 0.5f,0.5f,0.5f, 0.5f,0.5f,-0.5f, 0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f, -0.5f,0.5f,-0.5f, -0.5f,0.5f,0.5f, -0.5f,-0.5f,0.5f
};
static const GLushort cube_indices[] = {
    0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11,
    12,13,14,12,14,15, 16,17,18,16,18,19, 20,21,22,20,22,23
};

static const char* vertex_shader_source =
    "attribute vec3 a_position;\n"
    "uniform mat4 u_mvp;\n"
    "void main(){gl_Position=u_mvp*vec4(a_position,1.0);}\n";
static const char* fragment_shader_source =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "uniform vec4 u_color;\n"
    "void main(){gl_FragColor=u_color;}\n";

static nds_result compile_shader(nds_gles2_backend* b, GLenum type, const char* source, GLuint* out)
{
    GLint ok = 0;
    GLuint shader = b->gl.glCreateShader(type);
    if (!shader) return NDS_ERR_INIT_FAILED;
    b->gl.glShaderSource(shader, 1, &source, NULL);
    b->gl.glCompileShader(shader);
    b->gl.glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; GLsizei written = 0;
        b->gl.glGetShaderInfoLog(shader, (GLsizei)sizeof(log)-1, &written, log);
        log[written < (GLsizei)sizeof(log) ? written : (GLsizei)sizeof(log)-1] = '\0';
        NDS_LOGE("gles2", "shader compile failed: %s", log);
        b->gl.glDeleteShader(shader);
        return NDS_ERR_INIT_FAILED;
    }
    *out = shader;
    return NDS_OK;
}

static nds_result create_program(nds_gles2_backend* b)
{
    GLuint vs = 0, fs = 0, program = 0; GLint ok = 0;
    if (compile_shader(b, GL_VERTEX_SHADER, vertex_shader_source, &vs) != NDS_OK) return NDS_ERR_INIT_FAILED;
    if (compile_shader(b, GL_FRAGMENT_SHADER, fragment_shader_source, &fs) != NDS_OK) { b->gl.glDeleteShader(vs); return NDS_ERR_INIT_FAILED; }
    program = b->gl.glCreateProgram();
    if (!program) goto fail;
    b->gl.glAttachShader(program, vs); b->gl.glAttachShader(program, fs); b->gl.glLinkProgram(program);
    b->gl.glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) goto fail;
    b->program = program;
    b->position_attrib = b->gl.glGetAttribLocation(program, "a_position");
    b->mvp_uniform = b->gl.glGetUniformLocation(program, "u_mvp");
    b->color_uniform = b->gl.glGetUniformLocation(program, "u_color");
    b->gl.glDeleteShader(vs); b->gl.glDeleteShader(fs);
    return (b->position_attrib >= 0 && b->mvp_uniform >= 0 && b->color_uniform >= 0) ? NDS_OK : NDS_ERR_INIT_FAILED;
fail:
    if (program) b->gl.glDeleteProgram(program);
    b->gl.glDeleteShader(vs); b->gl.glDeleteShader(fs);
    return NDS_ERR_INIT_FAILED;
}

static void make_model(nds_mat4* out, const nds_draw_part* p)
{
    nds_mat4 t, s, rx, ry, rz, rxy, rxyz, rs;
    const float d = 0.01745329251994329577f;
    float cx = cosf(p->rotation.x*d), sx = sinf(p->rotation.x*d);
    float cy = cosf(p->rotation.y*d), sy = sinf(p->rotation.y*d);
    float cz = cosf(p->rotation.z*d), sz = sinf(p->rotation.z*d);
    nds_mat4_translate(&t, p->position.x, p->position.y, p->position.z);
    nds_mat4_scale(&s, p->size.x, p->size.y, p->size.z);
    nds_mat4_identity(&rx); nds_mat4_identity(&ry); nds_mat4_identity(&rz);
    rx.m[5]=cx; rx.m[6]=sx; rx.m[9]=-sx; rx.m[10]=cx;
    ry.m[0]=cy; ry.m[2]=-sy; ry.m[8]=sy; ry.m[10]=cy;
    rz.m[0]=cz; rz.m[1]=sz; rz.m[4]=-sz; rz.m[5]=cz;
    nds_mat4_mul(&rxy,&ry,&rx); nds_mat4_mul(&rxyz,&rz,&rxy); nds_mat4_mul(&rs,&rxyz,&s); nds_mat4_mul(out,&t,&rs);
}

#define LOAD_GL(name, type) do { b->gl.name=(type)platform_gl_get_proc_address(#name); if(!b->gl.name) goto fail; } while(0)

nds_result nds_gles2_backend_create(nds_gles2_backend** out_backend, int width, int height,
                                    float fov_y_degrees, float near_plane, float far_plane)
{
    nds_gles2_backend* b;
    if (!out_backend || width <= 0 || height <= 0) return NDS_ERR_INVALID_ARG;
    *out_backend = NULL;
    if (platform_gl_context_create() != NDS_OK) return NDS_ERR_INIT_FAILED;
    b=(nds_gles2_backend*)calloc(1,sizeof(*b)); if(!b){platform_gl_context_destroy();return NDS_ERR_UNKNOWN;}
    b->width=width; b->height=height; b->fov_y_degrees=fov_y_degrees>1?fov_y_degrees:70; b->near_plane=near_plane>0.001f?near_plane:0.1f; b->far_plane=far_plane>b->near_plane?far_plane:2000;
    LOAD_GL(glCreateShader,PFNGLCREATESHADERPROC); LOAD_GL(glShaderSource,PFNGLSHADERSOURCEPROC); LOAD_GL(glCompileShader,PFNGLCOMPILESHADERPROC);
    LOAD_GL(glGetShaderiv,PFNGLGETSHADERIVPROC); LOAD_GL(glGetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC); LOAD_GL(glDeleteShader,PFNGLDELETESHADERPROC);
    LOAD_GL(glCreateProgram,PFNGLCREATEPROGRAMPROC); LOAD_GL(glAttachShader,PFNGLATTACHSHADERPROC); LOAD_GL(glLinkProgram,PFNGLLINKPROGRAMPROC);
    LOAD_GL(glGetProgramiv,PFNGLGETPROGRAMIVPROC); LOAD_GL(glGetProgramInfoLog,PFNGLGETPROGRAMINFOLOGPROC); LOAD_GL(glDeleteProgram,PFNGLDELETEPROGRAMPROC);
    LOAD_GL(glUseProgram,PFNGLUSEPROGRAMPROC); LOAD_GL(glGetUniformLocation,PFNGLGETUNIFORMLOCATIONPROC); LOAD_GL(glUniformMatrix4fv,PFNGLUNIFORMMATRIX4FVPROC);
    LOAD_GL(glUniform4f,PFNGLUNIFORM4FPROC); LOAD_GL(glGetAttribLocation,PFNGLGETATTRIBLOCATIONPROC); LOAD_GL(glGenBuffers,PFNGLGENBUFFERSPROC);
    LOAD_GL(glBindBuffer,PFNGLBINDBUFFERPROC); LOAD_GL(glBufferData,PFNGLBUFFERDATAPROC); LOAD_GL(glDeleteBuffers,PFNGLDELETEBUFFERSPROC);
    LOAD_GL(glVertexAttribPointer,PFNGLVERTEXATTRIBPOINTERPROC); LOAD_GL(glEnableVertexAttribArray,PFNGLENABLEVERTEXATTRIBARRAYPROC); LOAD_GL(glDisableVertexAttribArray,PFNGLDISABLEVERTEXATTRIBARRAYPROC);
    if(create_program(b)!=NDS_OK) goto fail;
    b->gl.glGenBuffers(1,&b->vertex_buffer); b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer); b->gl.glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_vertices),cube_vertices,GL_STATIC_DRAW);
    b->gl.glGenBuffers(1,&b->index_buffer); b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer); b->gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)sizeof(cube_indices),cube_indices,GL_STATIC_DRAW);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); glDisable(GL_CULL_FACE);
    nds_gles2_backend_resize(b,width,height); *out_backend=b; return NDS_OK;
fail:
    if(b->program && b->gl.glDeleteProgram)b->gl.glDeleteProgram(b->program); if(b->vertex_buffer&&b->gl.glDeleteBuffers)b->gl.glDeleteBuffers(1,&b->vertex_buffer); if(b->index_buffer&&b->gl.glDeleteBuffers)b->gl.glDeleteBuffers(1,&b->index_buffer);
    free(b); platform_gl_context_destroy(); return NDS_ERR_INIT_FAILED;
}

void nds_gles2_backend_destroy(nds_gles2_backend* b)
{
    if(!b)return; if(b->gl.glDeleteProgram&&b->program)b->gl.glDeleteProgram(b->program); if(b->gl.glDeleteBuffers){if(b->vertex_buffer)b->gl.glDeleteBuffers(1,&b->vertex_buffer);if(b->index_buffer)b->gl.glDeleteBuffers(1,&b->index_buffer);} free(b); platform_gl_context_destroy();
}

nds_result nds_gles2_backend_resize(nds_gles2_backend* b,int width,int height)
{ if(!b||width<=0||height<=0)return NDS_ERR_INVALID_ARG; b->width=width;b->height=height;glViewport(0,0,width,height);return NDS_OK; }

nds_result nds_gles2_backend_begin(nds_gles2_backend* b)
{ if(!b)return NDS_ERR_INVALID_ARG; if(platform_gl_context_make_current()!=NDS_OK)return NDS_ERR_INIT_FAILED; glClearColor(0.055f,0.075f,0.10f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);b->gl.glUseProgram(b->program);return NDS_OK; }

nds_result nds_gles2_backend_draw_parts(nds_gles2_backend* b,const nds_draw_list* list,const nds_camera* camera)
{
    nds_mat4 projection,view,pv,model,mvp; float aspect;
    if(!b||!list||!camera)return NDS_ERR_INVALID_ARG;
    aspect=b->height>0?(float)b->width/(float)b->height:1.0f;
    nds_camera_projection_matrix(camera,aspect,&projection);
    nds_camera_view_matrix(camera,&view);
    nds_mat4_mul(&pv,&projection,&view);
    b->gl.glBindBuffer(GL_ARRAY_BUFFER,b->vertex_buffer); b->gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,b->index_buffer); b->gl.glEnableVertexAttribArray((GLuint)b->position_attrib); b->gl.glVertexAttribPointer((GLuint)b->position_attrib,3,GL_FLOAT,GL_FALSE,0,(const void*)0);
    for(size_t i=0;i<list->count;++i){
        const nds_draw_part* p=&list->parts[i];
        float r,g,bl,a;
        if(!p->visible||p->transparency>=1.0f)continue;
        r=(float)((p->color_rgba>>24)&0xff)/255.0f; g=(float)((p->color_rgba>>16)&0xff)/255.0f; bl=(float)((p->color_rgba>>8)&0xff)/255.0f; a=(float)(p->color_rgba&0xff)/255.0f; a*=1.0f-p->transparency;
        make_model(&model,p); nds_mat4_mul(&mvp,&pv,&model); b->gl.glUniformMatrix4fv(b->mvp_uniform,1,GL_FALSE,mvp.m); b->gl.glUniform4f(b->color_uniform,r,g,bl,a);
        glDrawElements(GL_TRIANGLES,(GLsizei)(sizeof(cube_indices)/sizeof(cube_indices[0])),GL_UNSIGNED_SHORT,(const void*)0);
    }
    b->gl.glDisableVertexAttribArray((GLuint)b->position_attrib); return NDS_OK;
}

nds_result nds_gles2_backend_end(nds_gles2_backend* b)
{ if(!b)return NDS_ERR_INVALID_ARG; platform_gl_swap_buffers(); return NDS_OK; }
