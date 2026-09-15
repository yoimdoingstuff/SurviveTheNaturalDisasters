#ifndef NDS_ENGINE_PLATFORM_H
#define NDS_ENGINE_PLATFORM_H

/*
 * Shared platform API (TODO.md Phase 0: "Define shared platform API for
 * graphics, input, files, audio, sockets, timing, lifecycle").
 *
 * Every function declared here must be implemented once per platform
 * backend (engine/src/platform/<name>/...). Engine/core and, later, game
 * code must call ONLY through this header and must never include OS
 * headers directly - that is what "keep platform-specific code isolated"
 * means in AGENTS.md.
 *
 * Status as of this pass:
 *   - lifecycle / window / input / files / timing: implemented for Windows.
 *   - audio: interface defined, backend intentionally deferred to Phase 6.
 *   - sockets: interface defined, backend intentionally deferred to Phase 9.
 *   - graphics: this only covers window/surface + present lifecycle. The
 *     actual OpenGL ES 2-class renderer is Phase 2 scope and will extend
 *     this header rather than replace it.
 */

#include "engine/core/types.h"
#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------------
 * Lifecycle
 * ------------------------------------------------------------------- */

nds_result platform_init(void);
void platform_shutdown(void);

/* ---------------------------------------------------------------------
 * Window / graphics surface
 * ------------------------------------------------------------------- */

typedef struct platform_window_desc {
    const char* title;
    int width;
    int height;
    int resizable; /* 0 or 1 */
} platform_window_desc;

nds_result platform_create_window(const platform_window_desc* desc);
void platform_destroy_window(void);
void platform_get_window_size(int* out_width, int* out_height);

/* Presents/flushes the current frame. Phase 1 backends may implement this
 * with whatever OS-native mechanism proves a frame was produced (e.g. a
 * GDI repaint on Windows); Phase 2 replaces the body with a real
 * swap-buffers call once the GLES2-class renderer exists, without
 * changing this signature. */
void platform_present(void);

/* Pumps the platform event queue and updates input state for this frame.
 * Returns non-zero if the platform itself is requesting shutdown. */
int platform_poll_events(void);
int platform_quit_requested(void);
void platform_request_quit(void);

/* ---------------------------------------------------------------------
 * Input
 * ------------------------------------------------------------------- */

typedef enum platform_key {
    PLATFORM_KEY_UNKNOWN = 0,
    PLATFORM_KEY_ESCAPE,
    PLATFORM_KEY_W,
    PLATFORM_KEY_A,
    PLATFORM_KEY_S,
    PLATFORM_KEY_D,
    PLATFORM_KEY_SPACE,
    PLATFORM_KEY_COUNT
} platform_key;

int platform_is_key_down(platform_key key);
void platform_get_mouse_position(int* out_x, int* out_y);
int platform_is_mouse_button_down(int button_index); /* 0=left, 1=right, 2=middle */

/* ---------------------------------------------------------------------
 * Files
 *
 * Deliberately mirrors simple stdio semantics today. iOS/Android backends
 * will route this through sandboxed bundle/document paths in later
 * phases without changing this interface - callers must never assume
 * platform_file is a FILE*.
 * ------------------------------------------------------------------- */

typedef struct platform_file platform_file;

platform_file* platform_file_open(const char* path, const char* mode);
size_t platform_file_read(platform_file* file, void* buffer, size_t size);
size_t platform_file_write(platform_file* file, const void* buffer, size_t size);
long platform_file_size(platform_file* file);
void platform_file_close(platform_file* file);
int platform_file_exists(const char* path);

/* ---------------------------------------------------------------------
 * Timing
 * ------------------------------------------------------------------- */

uint64_t platform_time_now_ns(void);
void platform_sleep_ms(uint32_t milliseconds);

/* ---------------------------------------------------------------------
 * Audio (API surface only - see docs/GAMEPLAY.md / TODO.md Phase 6)
 * ------------------------------------------------------------------- */

nds_result platform_audio_init(void);
void platform_audio_shutdown(void);

/* ---------------------------------------------------------------------
 * Sockets (API surface only - see docs/LOCAL_MULTIPLAYER.md / TODO.md Phase 9)
 * ------------------------------------------------------------------- */

nds_result platform_socket_init(void);
void platform_socket_shutdown(void);

#endif /* NDS_ENGINE_PLATFORM_H */
