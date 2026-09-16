#ifndef NDS_ENGINE_GAME_STATS_H
#define NDS_ENGINE_GAME_STATS_H

#include <stdint.h>

typedef struct nds_game_stats {
    uint32_t rounds_played;
    uint32_t rounds_survived;
    uint32_t rounds_eliminated;
    uint32_t current_survival_streak;
    uint32_t best_survival_streak;
    float total_survival_seconds;
} nds_game_stats;

void nds_game_stats_init(nds_game_stats* stats);
void nds_game_stats_record_round(nds_game_stats* stats, int survived, float survival_seconds);
float nds_game_stats_survival_rate(const nds_game_stats* stats);
float nds_game_stats_average_survival_seconds(const nds_game_stats* stats);

#endif
