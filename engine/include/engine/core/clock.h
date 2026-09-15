#ifndef NDS_ENGINE_CLOCK_H
#define NDS_ENGINE_CLOCK_H

#include <stdint.h>

typedef struct nds_clock {
    uint64_t start_ns;
    uint64_t last_ns;
} nds_clock;

void nds_clock_init(nds_clock* clock);

/* Returns seconds elapsed since the previous call (or since init on the
 * first call) and advances the clock's internal "last tick" marker. */
double nds_clock_tick(nds_clock* clock);

double nds_clock_elapsed_seconds(const nds_clock* clock);

#endif /* NDS_ENGINE_CLOCK_H */
