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

    fprintf(stdout, "[%s][%s][%s] ", time_buf, level_name(level), tag ? tag : "-");

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    fprintf(stdout, "\n");
    fflush(stdout);
}
