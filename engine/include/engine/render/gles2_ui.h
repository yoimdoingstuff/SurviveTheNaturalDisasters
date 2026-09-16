#ifndef NDS_ENGINE_RENDER_GLES2_UI_H
#define NDS_ENGINE_RENDER_GLES2_UI_H

#include "engine/render/gles2.h"

#ifdef __cplusplus
extern "C" {
#endif

void nds_gles2_ui_draw(int width, int height, const nds_hud_state* state);

#ifdef __cplusplus
}
#endif

#endif
