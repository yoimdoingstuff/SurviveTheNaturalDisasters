#ifndef NDS_ENGINE_PLATFORM_H
#define NDS_ENGINE_PLATFORM_H

#include "engine/core/types.h"
#include <stddef.h>
#include <stdint.h>

/* Shared platform boundary. OS headers belong in platform implementations,
 * never in shared engine/game code. */

nds_result platform_init(void);
void platform_shutdown(void);

typedef struct platform_window_desc { const char* title; int width; int height; int resizable; } platform_window_desc;
nds_result platform_create_window(const platform_window_desc* desc);
void platform_destroy_window(void);
void platform_get_window_size(int* out_width, int* out_height);
void platform_present(void);
int platform_poll_events(void);
int platform_quit_requested(void);

typedef enum platform_key {
    PLATFORM_KEY_UNKNOWN = 0,
    PLATFORM_KEY_ESCAPE,
    PLATFORM_KEY_W,
    PLATFORM_KEY_A,
    PLATFORM_KEY_S,
    PLATFORM_KEY_D,
    PLATFORM_KEY_SPACE,
    PLATFORM_KEY_Q,
    PLATFORM_KEY_E,
    PLATFORM_KEY_C,
    PLATFORM_KEY_COUNT
} platform_key;

int platform_is_key_down(platform_key key);
void platform_get_mouse_position(int* out_x, int* out_y);
int platform_is_mouse_button_down(int button_index);

typedef struct platform_file platform_file;
platform_file* platform_file_open(const char* path, const char* mode);
size_t platform_file_read(platform_file* file, void* buffer, size_t size);
size_t platform_file_write(platform_file* file, const void* buffer, size_t size);
long platform_file_size(platform_file* file);
void platform_file_close(platform_file* file);
int platform_file_exists(const char* path);
uint64_t platform_time_now_ns(void);
void platform_sleep_ms(uint32_t milliseconds);
nds_result platform_audio_init(void);
void platform_audio_shutdown(void);
nds_result platform_socket_init(void);
void platform_socket_shutdown(void);

#endif
