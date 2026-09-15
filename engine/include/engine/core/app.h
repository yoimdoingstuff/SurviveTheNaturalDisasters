#ifndef NDS_ENGINE_APP_H
#define NDS_ENGINE_APP_H

#include "engine/core/types.h"

typedef struct nds_app_options {
    const char* window_title;
    int window_width;   /* 0 = use config/default */
    int window_height;  /* 0 = use config/default */
    const char* config_path; /* NULL = skip loading a config file */
    const char* map_path;    /* NULL = use the default project render map */

    /* CI / automated verification support: if > 0, the app runs exactly
     * this many frames and then exits on its own instead of waiting for
     * window-close/ESC. */
    int smoke_test_frames;
} nds_app_options;

/* Runs platform_init -> window creation -> main loop -> shutdown. */
nds_result nds_app_run(const nds_app_options* options);

#endif /* NDS_ENGINE_APP_H */
