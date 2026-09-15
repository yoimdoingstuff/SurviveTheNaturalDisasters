#ifndef NDS_ENGINE_TYPES_H
#define NDS_ENGINE_TYPES_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#define NDS_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
#define NDS_PLATFORM_ANDROID 1
#elif defined(__APPLE__)
#define NDS_PLATFORM_APPLE 1
#else
#define NDS_PLATFORM_UNKNOWN 1
#endif

/* Shared result type for engine/platform calls. Kept as a plain enum
 * (not exceptions, not errno) per AGENTS.md: simple, deterministic,
 * predictable in hot paths. */
typedef enum nds_result {
    NDS_OK = 0,
    NDS_ERR_UNKNOWN = -1,
    NDS_ERR_NOT_IMPLEMENTED = -2,
    NDS_ERR_INIT_FAILED = -3,
    NDS_ERR_IO = -4,
    NDS_ERR_INVALID_ARG = -5
} nds_result;

#endif /* NDS_ENGINE_TYPES_H */
