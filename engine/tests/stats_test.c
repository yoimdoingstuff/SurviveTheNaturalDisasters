#include "engine/game/stats.h"

#include <assert.h>
#include <math.h>

int main(void)
{
    nds_game_stats stats;
    nds_game_stats_init(&stats);
    assert(stats.rounds_played == 0);
    assert(stats.rounds_survived == 0);
    assert(stats.rounds_eliminated == 0);
    assert(stats.best_survival_streak == 0);
    assert(fabsf(nds_game_stats_survival_rate(&stats)) < 0.001f);

    nds_game_stats_record_round(&stats, 1, 12.0f);
    nds_game_stats_record_round(&stats, 1, 8.0f);
    assert(stats.rounds_played == 2);
    assert(stats.rounds_survived == 2);
    assert(stats.rounds_eliminated == 0);
    assert(stats.current_survival_streak == 2);
    assert(stats.best_survival_streak == 2);
    assert(fabsf(nds_game_stats_survival_rate(&stats) - 1.0f) < 0.001f);
    assert(fabsf(nds_game_stats_average_survival_seconds(&stats) - 10.0f) < 0.001f);

    nds_game_stats_record_round(&stats, 0, 4.0f);
    assert(stats.rounds_played == 3);
    assert(stats.rounds_survived == 2);
    assert(stats.rounds_eliminated == 1);
    assert(stats.current_survival_streak == 0);
    assert(stats.best_survival_streak == 2);
    assert(fabsf(nds_game_stats_survival_rate(&stats) - (2.0f / 3.0f)) < 0.001f);
    assert(fabsf(nds_game_stats_average_survival_seconds(&stats) - 8.0f) < 0.001f);

    /* Invalid negative survival time is clamped rather than corrupting totals. */
    nds_game_stats_record_round(&stats, 1, -10.0f);
    assert(stats.rounds_played == 4);
    assert(stats.total_survival_seconds > 23.9f && stats.total_survival_seconds < 24.1f);
    assert(stats.current_survival_streak == 1);
    assert(stats.best_survival_streak == 2);
    return 0;
}
