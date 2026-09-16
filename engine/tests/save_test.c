#include "engine/game/save.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void test_defaults(void)
{
    nds_save_state state;
    nds_save_state_init(&state);
    assert(state.stats.rounds_played == 0);
    assert(fabsf(state.round_duration - 60.0f) < 0.001f);
    assert(fabsf(state.disaster_settings.warning_duration - 3.0f) < 0.001f);
    assert(fabsf(state.disaster_settings.windstorm_strength - 18.0f) < 0.001f);
}

static void test_round_trip(void)
{
    const char* path = "engine/tests/.nds_save_test.ini";
    nds_save_state source, loaded;
    nds_save_state_init(&source);
    source.stats.rounds_played = 17;
    source.stats.rounds_survived = 11;
    source.stats.rounds_eliminated = 6;
    source.stats.current_survival_streak = 3;
    source.stats.best_survival_streak = 7;
    source.stats.total_survival_seconds = 481.25f;
    source.intermission_duration = 4.5f;
    source.round_duration = 45.0f;
    source.results_duration = 6.0f;
    source.disaster_settings.warning_duration = 2.25f;
    source.disaster_settings.earthquake_pulse_interval = 3.0f;
    source.disaster_settings.windstorm_gust_interval = 1.75f;
    source.disaster_settings.windstorm_strength = 22.5f;

    assert(nds_save_write(path, &source) == NDS_OK);
    memset(&loaded, 0, sizeof(loaded));
    assert(nds_save_load(path, &loaded) == NDS_OK);
    assert(loaded.stats.rounds_played == source.stats.rounds_played);
    assert(loaded.stats.rounds_survived == source.stats.rounds_survived);
    assert(loaded.stats.rounds_eliminated == source.stats.rounds_eliminated);
    assert(loaded.stats.current_survival_streak == source.stats.current_survival_streak);
    assert(loaded.stats.best_survival_streak == source.stats.best_survival_streak);
    assert(fabsf(loaded.stats.total_survival_seconds - source.stats.total_survival_seconds) < 0.01f);
    assert(fabsf(loaded.round_duration - source.round_duration) < 0.01f);
    assert(fabsf(loaded.disaster_settings.warning_duration - source.disaster_settings.warning_duration) < 0.01f);
    assert(fabsf(loaded.disaster_settings.windstorm_strength - source.disaster_settings.windstorm_strength) < 0.01f);
    remove(path);
}

static void test_missing_file_keeps_defaults(void)
{
    nds_save_state state;
    assert(nds_save_load("engine/tests/.does_not_exist.ini", &state) != NDS_OK);
    assert(state.round_duration == 60.0f);
    assert(state.stats.rounds_played == 0);
}

int main(void)
{
    test_defaults();
    test_round_trip();
    test_missing_file_keeps_defaults();
    puts("save tests passed");
    return 0;
}
