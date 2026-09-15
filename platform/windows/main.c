/*
 * Windows dev-build entry point.
 *
 * Deliberately a plain console-subsystem `main()` rather than WinMain for
 * now: it still creates a normal Win32 window (that works fine from a
 * console subsystem app), while keeping stdout log output visible in the
 * same terminal/CI job. A WINDOWS-subsystem entry point can be swapped in
 * later for the shipping build (see TODO.md Phase 13 "base release lock")
 * without touching engine/game code.
 */

#include "engine/core/app.h"
#include "engine/core/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* argv0)
{
    printf("usage: %s [--smoke-test [frames]]\n", argv0);
    printf("  --smoke-test [frames]  run N frames (default 60) then exit automatically.\n");
    printf("                         Used by CI to prove the runtime boots.\n");
}

int main(int argc, char** argv)
{
    nds_app_options options;
    memset(&options, 0, sizeof(options));
    options.window_title = "Natural Disaster Survival - Local Runtime (Windows dev build)";
    options.window_width = 1024;
    options.window_height = 768;
    options.config_path = "runtime_config.ini";
    options.smoke_test_frames = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--smoke-test") == 0) {
            options.smoke_test_frames = 60;
            if (i + 1 < argc) {
                int maybe_frames = atoi(argv[i + 1]);
                if (maybe_frames > 0) {
                    options.smoke_test_frames = maybe_frames;
                    ++i;
                }
            }
        }
    }

    nds_result rc = nds_app_run(&options);
    if (rc != NDS_OK) {
        NDS_LOGE("main", "nds_app_run failed (%d)", (int)rc);
        return 1;
    }
    return 0;
}
