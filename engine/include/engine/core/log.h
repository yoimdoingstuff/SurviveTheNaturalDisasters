#ifndef NDS_ENGINE_LOG_H
#define NDS_ENGINE_LOG_H

/* Phase 1 "logging/debug console" item: this covers the logging half
 * (leveled, tagged, timestamped console output). An interactive in-game
 * debug console is separate follow-up work layered on top of this. */

typedef enum nds_log_level {
    NDS_LOG_DEBUG = 0,
    NDS_LOG_INFO,
    NDS_LOG_WARN,
    NDS_LOG_ERROR
} nds_log_level;

void nds_log_set_level(nds_log_level min_level);

/* printf-style; do not pass user-controlled strings as fmt. */
void nds_log(nds_log_level level, const char* tag, const char* fmt, ...);

#define NDS_LOGD(tag, ...) nds_log(NDS_LOG_DEBUG, tag, __VA_ARGS__)
#define NDS_LOGI(tag, ...) nds_log(NDS_LOG_INFO, tag, __VA_ARGS__)
#define NDS_LOGW(tag, ...) nds_log(NDS_LOG_WARN, tag, __VA_ARGS__)
#define NDS_LOGE(tag, ...) nds_log(NDS_LOG_ERROR, tag, __VA_ARGS__)

#endif /* NDS_ENGINE_LOG_H */
