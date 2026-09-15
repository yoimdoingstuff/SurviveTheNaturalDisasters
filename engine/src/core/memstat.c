#include "engine/core/memstat.h"
#include "engine/platform/platform.h"

#include <stdlib.h>
#include <stdint.h>

typedef struct alloc_header {
    size_t size;
} alloc_header;

static size_t g_current_bytes = 0;
static size_t g_peak_bytes = 0;
static size_t g_total_allocations = 0;
static size_t g_total_frees = 0;

void* nds_malloc(size_t size)
{
    alloc_header* header = (alloc_header*)malloc(sizeof(alloc_header) + size);
    if (!header) {
        return NULL;
    }

    header->size = size;
    g_current_bytes += size;
    g_total_allocations++;
    if (g_current_bytes > g_peak_bytes) {
        g_peak_bytes = g_current_bytes;
    }

    return (void*)(header + 1);
}

void* nds_realloc(void* ptr, size_t size)
{
    if (!ptr) {
        return nds_malloc(size);
    }

    alloc_header* old_header = ((alloc_header*)ptr) - 1;
    size_t old_size = old_header->size;

    alloc_header* new_header = (alloc_header*)realloc(old_header, sizeof(alloc_header) + size);
    if (!new_header) {
        return NULL;
    }

    new_header->size = size;
    g_current_bytes = g_current_bytes - old_size + size;
    if (g_current_bytes > g_peak_bytes) {
        g_peak_bytes = g_current_bytes;
    }

    return (void*)(new_header + 1);
}

void nds_free(void* ptr)
{
    if (!ptr) {
        return;
    }
    alloc_header* header = ((alloc_header*)ptr) - 1;
    g_current_bytes -= header->size;
    g_total_frees++;
    free(header);
}

void nds_mem_get_stats(nds_mem_stats* out_stats)
{
    if (!out_stats) {
        return;
    }
    out_stats->current_bytes = g_current_bytes;
    out_stats->peak_bytes = g_peak_bytes;
    out_stats->total_allocations = g_total_allocations;
    out_stats->total_frees = g_total_frees;
}

/* ---- Frame performance stats ---- */

static uint64_t g_frame_start_ns = 0;
static double g_sum_frame_ms = 0.0;
static double g_min_frame_ms = 0.0;
static double g_max_frame_ms = 0.0;
static double g_last_frame_ms = 0.0;
static unsigned long g_frame_count = 0;

void nds_perf_begin_frame(void)
{
    g_frame_start_ns = platform_time_now_ns();
}

void nds_perf_end_frame(void)
{
    uint64_t now = platform_time_now_ns();
    double ms = (double)(now - g_frame_start_ns) / 1000000.0;

    g_last_frame_ms = ms;
    g_sum_frame_ms += ms;
    g_frame_count++;

    if (g_frame_count == 1 || ms < g_min_frame_ms) {
        g_min_frame_ms = ms;
    }
    if (ms > g_max_frame_ms) {
        g_max_frame_ms = ms;
    }
}

void nds_perf_get_stats(nds_perf_stats* out_stats)
{
    if (!out_stats) {
        return;
    }
    out_stats->last_frame_ms = g_last_frame_ms;
    out_stats->avg_frame_ms = g_frame_count > 0 ? (g_sum_frame_ms / (double)g_frame_count) : 0.0;
    out_stats->min_frame_ms = g_min_frame_ms;
    out_stats->max_frame_ms = g_max_frame_ms;
    out_stats->frame_count = g_frame_count;
}

void nds_perf_reset(void)
{
    g_sum_frame_ms = 0.0;
    g_min_frame_ms = 0.0;
    g_max_frame_ms = 0.0;
    g_last_frame_ms = 0.0;
    g_frame_count = 0;
}
