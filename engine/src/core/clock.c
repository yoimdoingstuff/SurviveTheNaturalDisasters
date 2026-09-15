#include "engine/core/clock.h"
#include "engine/platform/platform.h"

void nds_clock_init(nds_clock* clock)
{
    if (!clock) {
        return;
    }
    uint64_t now = platform_time_now_ns();
    clock->start_ns = now;
    clock->last_ns = now;
}

double nds_clock_tick(nds_clock* clock)
{
    if (!clock) {
        return 0.0;
    }
    uint64_t now = platform_time_now_ns();
    double delta_seconds = (double)(now - clock->last_ns) / 1000000000.0;
    clock->last_ns = now;
    return delta_seconds;
}

double nds_clock_elapsed_seconds(const nds_clock* clock)
{
    if (!clock) {
        return 0.0;
    }
    uint64_t now = platform_time_now_ns();
    return (double)(now - clock->start_ns) / 1000000000.0;
}
