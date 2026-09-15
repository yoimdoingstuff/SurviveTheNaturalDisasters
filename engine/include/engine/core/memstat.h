#ifndef NDS_ENGINE_MEMSTAT_H
#define NDS_ENGINE_MEMSTAT_H

#include <stddef.h>

/* Phase 1 "basic memory/performance instrumentation" item.
 *
 * nds_malloc/nds_realloc/nds_free are thin wrappers around the CRT
 * allocator that track current/peak bytes and allocation counts. Engine
 * code should prefer these over raw malloc/free so the numbers stay
 * meaningful; this does not replace a real allocator (arenas/pools for
 * hot loops are later work called out in AGENTS.md). */

void* nds_malloc(size_t size);
void* nds_realloc(void* ptr, size_t size);
void nds_free(void* ptr);

typedef struct nds_mem_stats {
    size_t current_bytes;
    size_t peak_bytes;
    size_t total_allocations;
    size_t total_frees;
} nds_mem_stats;

void nds_mem_get_stats(nds_mem_stats* out_stats);

typedef struct nds_perf_stats {
    double last_frame_ms;
    double avg_frame_ms;
    double min_frame_ms;
    double max_frame_ms;
    unsigned long frame_count;
} nds_perf_stats;

void nds_perf_begin_frame(void);
void nds_perf_end_frame(void);
void nds_perf_get_stats(nds_perf_stats* out_stats);
void nds_perf_reset(void);

#endif /* NDS_ENGINE_MEMSTAT_H */
