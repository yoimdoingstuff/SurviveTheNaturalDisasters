#include "gles2_backend.h"
#include "engine/platform/gl_context.h"

#include <GL/gl.h>
#include <math.h>
#include <stdlib.h>

struct nds_gles2_backend {
    int width;
    int height;
    float fov_y_degrees;
    float near_plane;
    float far_plane;
};

static void set_projection(const nds_gles2_backend* backend)
{
    float aspect = backend->height > 0 ? (float)backend->width / (float)backend->height : 1.0f;
    float f = 1.0f / tanf(backend->fov_y_degrees * 3.14159265358979323846f / 360.0f);
    float n = backend->near_plane;
    float fa = backend->far_plane;
    float top = n / f;
    float right = top * aspect;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-right, right, -top, top, n, fa);
    glMatrixMode(GL_MODELVIEW);
}

static void draw_cube(float sx, float sy, float sz)
{
    const float x = sx * 0.5f, y = sy * 0.5f, z = sz * 0.5f;
    glBegin(GL_QUADS);
    glNormal3f(0,0,1); glVertex3f(-x,-y,z); glVertex3f(x,-y,z); glVertex3f(x,y,z); glVertex3f(-x,y,z);
    glNormal3f(0,0,-1); glVertex3f(x,-y,-z); glVertex3f(-x,-y,-z); glVertex3f(-x,y,-z); glVertex3f(x,y,-z);
    glNormal3f(1,0,0); glVertex3f(x,-y,z); glVertex3f(x,-y,-z); glVertex3f(x,y,-z); glVertex3f(x,y,z);
    glNormal3f(-1,0,0); glVertex3f(-x,-y,-z); glVertex3f(-x,-y,z); glVertex3f(-x,y,z); glVertex3f(-x,y,-z);
    glNormal3f(0,1,0); glVertex3f(-x,y,z); glVertex3f(x,y,z); glVertex3f(x,y,-z); glVertex3f(-x,y,-z);
    glNormal3f(0,-1,0); glVertex3f(-x,-y,-z); glVertex3f(x,-y,-z); glVertex3f(x,-y,z); glVertex3f(-x,-y,z);
    glEnd();
}

nds_result nds_gles2_backend_create(nds_gles2_backend** out_backend, int width, int height,
                                    float fov_y_degrees, float near_plane, float far_plane)
{
    if (!out_backend || width <= 0 || height <= 0) return NDS_ERR_INVALID_ARG;
    *out_backend = NULL;
    if (platform_gl_context_create() != NDS_OK) return NDS_ERR_INIT_FAILED;
    nds_gles2_backend* backend = (nds_gles2_backend*)calloc(1, sizeof(*backend));
    if (!backend) {
        platform_gl_context_destroy();
        return NDS_ERR_UNKNOWN;
    }
    backend->width = width;
    backend->height = height;
    backend->fov_y_degrees = fov_y_degrees > 1.0f ? fov_y_degrees : 70.0f;
    backend->near_plane = near_plane > 0.001f ? near_plane : 0.1f;
    backend->far_plane = far_plane > backend->near_plane ? far_plane : 2000.0f;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    nds_gles2_backend_resize(backend, width, height);
    *out_backend = backend;
    return NDS_OK;
}

void nds_gles2_backend_destroy(nds_gles2_backend* backend)
{
    free(backend);
    platform_gl_context_destroy();
}

nds_result nds_gles2_backend_resize(nds_gles2_backend* backend, int width, int height)
{
    if (!backend || width <= 0 || height <= 0) return NDS_ERR_INVALID_ARG;
    backend->width = width;
    backend->height = height;
    glViewport(0, 0, width, height);
    set_projection(backend);
    return NDS_OK;
}

nds_result nds_gles2_backend_begin(nds_gles2_backend* backend)
{
    if (!backend) return NDS_ERR_INVALID_ARG;
    if (platform_gl_context_make_current() != NDS_OK) return NDS_ERR_INIT_FAILED;
    glClearColor(0.055f, 0.075f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    set_projection(backend);
    glLoadIdentity();
    glTranslatef(0.0f, -5.0f, -14.0f);
    return NDS_OK;
}

nds_result nds_gles2_backend_draw_parts(nds_gles2_backend* backend, const nds_draw_list* list)
{
    if (!backend || !list) return NDS_ERR_INVALID_ARG;
    for (size_t i = 0; i < list->count; ++i) {
        const nds_draw_part* part = &list->parts[i];
        uint32_t c = part->color_rgba;
        unsigned int r = (c >> 24) & 0xffu, g = (c >> 16) & 0xffu;
        unsigned int b = (c >> 8) & 0xffu, a = c & 0xffu;
        float alpha = (1.0f - part->transparency) * ((float)a / 255.0f);
        if (alpha <= 0.0f) continue;
        glColor4ub((GLubyte)r, (GLubyte)g, (GLubyte)b, (GLubyte)(alpha * 255.0f));
        glPushMatrix();
        glTranslatef(part->position.x, part->position.y, part->position.z);
        glRotatef(part->rotation.x, 1, 0, 0);
        glRotatef(part->rotation.y, 0, 1, 0);
        glRotatef(part->rotation.z, 0, 0, 1);
        draw_cube(part->size.x, part->size.y, part->size.z);
        glPopMatrix();
    }
    return NDS_OK;
}

nds_result nds_gles2_backend_end(nds_gles2_backend* backend)
{
    if (!backend) return NDS_ERR_INVALID_ARG;
    glFlush();
    platform_gl_swap_buffers();
    return NDS_OK;
}
