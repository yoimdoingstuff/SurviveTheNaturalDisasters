#include "engine/core/app.h"
#include "engine/platform/platform.h"
#include "engine/core/log.h"
#include "engine/core/config.h"
#include "engine/core/clock.h"
#include "engine/core/memstat.h"

static const char* TAG = "app";

nds_result nds_app_run(const nds_app_options* options)
{
    if (!options) {
        return NDS_ERR_INVALID_ARG;
    }

    nds_result rc = platform_init();
    if (rc != NDS_OK) {
        NDS_LOGE(TAG, "platform_init failed (%d)", (int)rc);
        return rc;
    }

    nds_config* cfg = nds_config_create();
    if (options->config_path) {
        nds_result cfg_rc = nds_config_load_file(cfg, options->config_path);
        if (cfg_rc != NDS_OK) {
            NDS_LOGW(TAG, "no config at '%s' yet, using defaults", options->config_path);
        }
    }

    platform_window_desc win_desc;
    win_desc.title = options->window_title
        ? options->window_title
        : "Natural Disaster Survival - Local Runtime";
    win_desc.width = options->window_width > 0
        ? options->window_width
        : nds_config_get_int(cfg, "window.width", 1024);
    win_desc.height = options->window_height > 0
        ? options->window_height
        : nds_config_get_int(cfg, "window.height", 768);
    win_desc.resizable = 1;

    rc = platform_create_window(&win_desc);
    if (rc != NDS_OK) {
        NDS_LOGE(TAG, "platform_create_window failed (%d)", (int)rc);
        nds_config_destroy(cfg);
        platform_shutdown();
        return rc;
    }

    NDS_LOGI(TAG, "boot ok: window %dx%d", win_desc.width, win_desc.height);

    nds_clock clock;
    nds_clock_init(&clock);
    nds_perf_reset();

    unsigned long frames_run = 0;
    const int smoke_frames = options->smoke_test_frames;

    while (!platform_quit_requested()) {
        if (platform_poll_events() != 0) {
            platform_request_quit();
        }

        if (platform_is_key_down(PLATFORM_KEY_ESCAPE)) {
            platform_request_quit();
        }

        nds_perf_begin_frame();
        double dt = nds_clock_tick(&clock);
        (void)dt; /* Phase 1: no scene/game update to drive yet - Phase 3+
                     will consume this delta time once there's something
                     to simulate. */

        platform_present();
        nds_perf_end_frame();

        frames_run++;

        if (smoke_frames > 0 && (int)frames_run >= smoke_frames) {
            NDS_LOGI(TAG, "smoke test target reached (%lu frames), requesting quit", frames_run);
            platform_request_quit();
        }

        if (frames_run % 300 == 0) {
            nds_perf_stats perf;
            nds_perf_get_stats(&perf);
            nds_mem_stats mem;
            nds_mem_get_stats(&mem);
            NDS_LOGI(TAG, "frame %lu: avg %.2fms (min %.2f / max %.2f) mem %zu bytes (peak %zu, %zu allocs / %zu frees)",
                      frames_run, perf.avg_frame_ms, perf.min_frame_ms, perf.max_frame_ms,
                      mem.current_bytes, mem.peak_bytes, mem.total_allocations, mem.total_frees);
        }
    }

    NDS_LOGI(TAG, "shutting down after %lu frames", frames_run);

    platform_destroy_window();
    nds_config_destroy(cfg);
    platform_shutdown();

    if (smoke_frames > 0) {
        NDS_LOGI(TAG, "SMOKE TEST PASSED");
    }

    return NDS_OK;
}
