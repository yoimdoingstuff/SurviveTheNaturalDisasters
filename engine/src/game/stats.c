#include "engine/game/stats.h"

void nds_game_stats_init(nds_game_stats* stats)
{
    if (!stats) return;
    *stats = (nds_game_stats){0};
}

void nds_game_stats_record_round(nds_game_stats* stats, int survived, float survival_seconds)
{
    if (!stats) return;
    if (survival_seconds < 0.0f) survival_seconds = 0.0f;
    stats->rounds_played++;
    stats->total_survival_seconds += survival_seconds;
    if (survived) {
        stats->rounds_survived++;
        stats->current_survival_streak++;
        if (stats->current_survival_streak > stats->best_survival_streak)
            stats->best_survival_streak = stats->current_survival_streak;
    } else {
        stats->rounds_eliminated++;
        stats->current_survival_streak = 0;
    }
}

float nds_game_stats_survival_rate(const nds_game_stats* stats)
{
    if (!stats || stats->rounds_played == 0) return 0.0f;
    return (float)stats->rounds_survived / (float)stats->rounds_played;
}

float nds_game_stats_average_survival_seconds(const nds_game_stats* stats)
{
    if (!stats || stats->rounds_played == 0) return 0.0f;
    return stats->total_survival_seconds / (float)stats->rounds_played;
}
