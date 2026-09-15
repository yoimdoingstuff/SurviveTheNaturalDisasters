#ifndef NDS_ENGINE_DRAW_H
#define NDS_ENGINE_DRAW_H

#include "engine/core/types.h"
#include "engine/scene/part.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_draw_part {
    nds_vec3 position;
    nds_vec3 size;
    nds_vec3 rotation;
    float transparency;
    uint32_t color_rgba;
    uint8_t visible;
    const nds_mesh* mesh;
    const nds_texture* texture;
} nds_draw_part;

typedef struct nds_draw_list {
    nds_draw_part* parts;
    size_t count;
    size_t capacity;
} nds_draw_list;

void nds_draw_list_init(nds_draw_list* list);
void nds_draw_list_reset(nds_draw_list* list);
void nds_draw_list_destroy(nds_draw_list* list);
nds_result nds_draw_list_add_part(nds_draw_list* list, const nds_instance* instance);
nds_result nds_draw_list_build_from_tree(nds_draw_list* list, const nds_instance* root);

#ifdef __cplusplus
}
#endif

#endif
