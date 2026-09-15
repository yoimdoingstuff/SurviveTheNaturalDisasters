#ifndef NDS_ENGINE_APP_H
#define NDS_ENGINE_APP_H

#include "engine/core/types.h"

typedef struct nds_app_options {
    const char* window_title;
    int window_width;   /* 0 = use config/default */
    int window_height;  /* 0 = use config/default */
    const char* config_path; /* NULL = skip loading a config file */

    /* CI / automated verification support: if > 0, the app runs exactly
     * this many frames and then exits on its own instead of waiting for
     * window-close/ESC. This is what lets GitHub Actions prove the
     * runtime actually boots end-to-end, not just that it compiles. */
    int smoke_test_frames;
} nds_app_options;

/* Runs platform_init -> window creation -> main loop -> shutdown.
 * Phase 1 scope: proves the runtime boots and stays alive; there is no
 * renderer/scene/gameplay update yet (that's Phase 2 onward). */
nds_result nds_app_run(const nds_app_options* options);

#endif /* NDS_ENGINE_APP_H */
