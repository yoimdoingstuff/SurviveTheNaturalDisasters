#include "engine/game/save.h"
#include "engine/core/config.h"

#include <stdio.h>
#include <stdlib.h>

static float get_float(const nds_config* cfg, const char* key, float fallback)
{
    const char* value = nds_config_get_string(cfg, key, NULL);
    char* end = NULL;
    float result;
    if (!value || !value[0]) return fallback;
    result = strtof(value, &end);
    if (end == value || (end && *end != '\0') || result <= 0.0f) return fallback;
    return result;
}

static void set_float(nds_config* cfg, const char* key, float value)
{
    char text[32];
    snprintf(text, sizeof(text), "%.3f", value);
    nds_config_set_string(cfg, key, text);
}

void nds_save_state_init(nds_save_state* state)
{
    if (!state) return;
    nds_game_stats_init(&state->stats);
    state->intermission_duration = 5.0f;
    state->round_duration = 60.0f;
    state->results_duration = 5.0f;
    state->disaster_settings.warning_duration = 3.0f;
    state->disaster_settings.earthquake_pulse_interval = 4.0f;
    state->disaster_settings.windstorm_gust_interval = 2.5f;
    state->disaster_settings.windstorm_strength = 18.0f;
}

nds_result nds_save_load(const char* path, nds_save_state* state)
{
    nds_config* cfg;
    if (!path || !path[0] || !state) return NDS_ERR_INVALID_ARG;
    nds_save_state_init(state);
    cfg = nds_config_create();
    if (!cfg) return NDS_ERR_UNKNOWN;
    if (nds_config_load_file(cfg, path) != NDS_OK) {
        nds_config_destroy(cfg);
        return NDS_ERR_UNKNOWN;
    }

    state->stats.rounds_played = (uint32_t)nds_config_get_int(cfg, "stats.rounds_played", 0);
    state->stats.rounds_survived = (uint32_t)nds_config_get_int(cfg, "stats.rounds_survived", 0);
    state->stats.rounds_eliminated = (uint32_t)nds_config_get_int(cfg, "stats.rounds_eliminated", 0);
    state->stats.current_survival_streak = (uint32_t)nds_config_get_int(cfg, "stats.current_survival_streak", 0);
    state->stats.best_survival_streak = (uint32_t)nds_config_get_int(cfg, "stats.best_survival_streak", 0);
    state->stats.total_survival_seconds = get_float(cfg, "stats.total_survival_seconds", 0.0f);
    state->intermission_duration = get_float(cfg, "round.intermission_duration", state->intermission_duration);
    state->round_duration = get_float(cfg, "round.duration", state->round_duration);
    state->results_duration = get_float(cfg, "round.results_duration", state->results_duration);
    state->disaster_settings.warning_duration = get_float(cfg, "disaster.warning_duration", state->disaster_settings.warning_duration);
    state->disaster_settings.earthquake_pulse_interval = get_float(cfg, "disaster.earthquake_pulse_interval", state->disaster_settings.earthquake_pulse_interval);
    state->disaster_settings.windstorm_gust_interval = get_float(cfg, "disaster.windstorm_gust_interval", state->disaster_settings.windstorm_gust_interval);
    state->disaster_settings.windstorm_strength = get_float(cfg, "disaster.windstorm_strength", state->disaster_settings.windstorm_strength);
    nds_config_destroy(cfg);
    return NDS_OK;
}

nds_result nds_save_write(const char* path, const nds_save_state* state)
{
    nds_config* cfg;
    if (!path || !path[0] || !state) return NDS_ERR_INVALID_ARG;
    cfg = nds_config_create();
    if (!cfg) return NDS_ERR_UNKNOWN;

    nds_config_set_int(cfg, "stats.rounds_played", (int)state->stats.rounds_played);
    nds_config_set_int(cfg, "stats.rounds_survived", (int)state->stats.rounds_survived);
    nds_config_set_int(cfg, "stats.rounds_eliminated", (int)state->stats.rounds_eliminated);
    nds_config_set_int(cfg, "stats.current_survival_streak", (int)state->stats.current_survival_streak);
    nds_config_set_int(cfg, "stats.best_survival_streak", (int)state->stats.best_survival_streak);
    set_float(cfg, "stats.total_survival_seconds", state->stats.total_survival_seconds < 0.0f ? 0.0f : state->stats.total_survival_seconds);
    set_float(cfg, "round.intermission_duration", state->intermission_duration);
    set_float(cfg, "round.duration", state->round_duration);
    set_float(cfg, "round.results_duration", state->results_duration);
    set_float(cfg, "disaster.warning_duration", state->disaster_settings.warning_duration);
    set_float(cfg, "disaster.earthquake_pulse_interval", state->disaster_settings.earthquake_pulse_interval);
    set_float(cfg, "disaster.windstorm_gust_interval", state->disaster_settings.windstorm_gust_interval);
    set_float(cfg, "disaster.windstorm_strength", state->disaster_settings.windstorm_strength);

    {
        nds_result result = nds_config_save_file(cfg, path);
        nds_config_destroy(cfg);
        return result;
    }
}
