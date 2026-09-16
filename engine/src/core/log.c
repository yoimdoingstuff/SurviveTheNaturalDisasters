/* Needed to expose POSIX localtime_r on glibc under strict -std=c11.
 * Has no effect on the Windows build (it uses localtime_s instead). */
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "engine/core/log.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static nds_log_level g_min_level = NDS_LOG_DEBUG;
static FILE* g_runtime_log_file = NULL;
static int g_runtime_log_initialized = 0;

static const char* level_name(nds_log_level level)
{
    switch (level) {
        case NDS_LOG_DEBUG: return "DEBUG";
        case NDS_LOG_INFO:  return "INFO";
        case NDS_LOG_WARN:  return "WARN";
        case NDS_LOG_ERROR: return "ERROR";
        default: return "?";
    }
}

static void ensure_runtime_log_file(void)
{
    if (g_runtime_log_initialized) {
        return;
    }
    g_runtime_log_initialized = 1;
    g_runtime_log_file = fopen("runtime.log", "ab");
    if (g_runtime_log_file) {
        fprintf(g_runtime_log_file, "\n=== runtime log started ===\n");
        fflush(g_runtime_log_file);
    }
}

void nds_log_set_level(nds_log_level min_level)
{
    g_min_level = min_level;
}

void nds_log(nds_log_level level, const char* tag, const char* fmt, ...)
{
    if (level < g_min_level) {
        return;
    }

    time_t now = time(NULL);
    struct tm tm_now;
    /* localtime_s vs localtime_r is a CRT portability shim, not a platform
     * backend - it does not belong behind engine/platform/platform.h. */
#if defined(_WIN32)
    localtime_s(&tm_now, &now);
#else
    localtime_r(&now, &tm_now);
#endif

    char time_buf[16];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &tm_now);
    const char* level_text = level_name(level);
    const char* tag_text = tag ? tag : "-";

    fprintf(stdout, "[%s][%s][%s] ", time_buf, level_text, tag_text);
    ensure_runtime_log_file();
    if (g_runtime_log_file) {
        fprintf(g_runtime_log_file, "[%s][%s][%s] ", time_buf, level_text, tag_text);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);

    va_start(args, fmt);
    if (g_runtime_log_file) {
        vfprintf(g_runtime_log_file, fmt, args);
        fprintf(g_runtime_log_file, "\n");
        fflush(g_runtime_log_file);
    }
    va_end(args);
}
