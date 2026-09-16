#ifndef NDS_ENGINE_RENDER_GLES2_H
#define NDS_ENGINE_RENDER_GLES2_H

#include "engine/core/types.h"
#include "engine/render/draw.h"
#include "engine/render/camera.h"
#include "engine/game/player.h"
#include "engine/game/round.h"
#include "engine/game/disaster_system.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_gles2_renderer nds_gles2_renderer;

typedef struct nds_gles2_desc {
    int width;
    int height;
    float fov_y_degrees;
    float near_plane;
    float far_plane;
} nds_gles2_desc;

typedef struct nds_hud_state {
    const nds_player_controller* player;
    const nds_round* round;
    const nds_disaster_system* disasters;
    int menu_active;
} nds_hud_state; /* NDS_UI_WORLD_PATCH_V2 */

nds_result nds_gles2_renderer_create(nds_gles2_renderer** out_renderer,
                                      const nds_gles2_desc* desc);
void nds_gles2_renderer_destroy(nds_gles2_renderer* renderer);
nds_result nds_gles2_renderer_resize(nds_gles2_renderer* renderer, int width, int height);
nds_result nds_gles2_renderer_begin(nds_gles2_renderer* renderer);
nds_result nds_gles2_renderer_draw_parts(nds_gles2_renderer* renderer,
                                          const nds_draw_list* list,
                                          const nds_camera* camera);
nds_result nds_gles2_renderer_draw_hud(nds_gles2_renderer* renderer,
                                       const nds_hud_state* state);
nds_result nds_gles2_renderer_end(nds_gles2_renderer* renderer);

#ifdef __cplusplus
}
#endif

#endif
