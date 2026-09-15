#ifndef NDS_ENGINE_GL_CONTEXT_H
#define NDS_ENGINE_GL_CONTEXT_H

#include "engine/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Platform-owned OpenGL context lifecycle. The renderer never sees an OS
 * window handle and can request only the operations it actually needs. */
nds_result platform_gl_context_create(void);
void platform_gl_context_destroy(void);
nds_result platform_gl_context_make_current(void);
void platform_gl_swap_buffers(void);
void* platform_gl_get_proc_address(const char* name);

#ifdef __cplusplus
}
#endif

#endif
