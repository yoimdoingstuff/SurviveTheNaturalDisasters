/*
 * Windows dev-build entry point.
 *
 * Deliberately a plain console-subsystem `main()` rather than WinMain for
 * now: it keeps stdout log output visible in the same terminal/CI job.
 */

#include "engine/core/app.h"
#include "engine/core/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* argv0)
{
    printf("usage: %s [--map path] [--smoke-test [frames]]\n", argv0);
    printf("  --map path             render a specific .ndsmap.json package.\n");
    printf("  --smoke-test [frames]  run N frames (default 60) then exit automatically.\n");
}

int main(int argc, char** argv)
{
    nds_app_options options;
    memset(&options, 0, sizeof(options));
    options.window_title = "Natural Disaster Survival - Local Runtime (Windows dev build)";
    options.window_width = 1024;
    options.window_height = 768;
    options.config_path = "runtime_config.ini";
    options.map_path = NULL;
    options.smoke_test_frames = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--map") == 0) {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                fprintf(stderr, "--map requires a path\n");
                print_usage(argv[0]);
                return 2;
            }
            options.map_path = argv[++i];
            continue;
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
            continue;
        }
        fprintf(stderr, "unknown argument: %s\n", argv[i]);
        print_usage(argv[0]);
        return 2;
    }

    nds_result rc = nds_app_run(&options);
    if (rc != NDS_OK) {
        NDS_LOGE("main", "nds_app_run failed (%d)", (int)rc);
        return 1;
    }
    return 0;
}
