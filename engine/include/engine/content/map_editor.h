#ifndef NDS_ENGINE_MAP_EDITOR_H
#define NDS_ENGINE_MAP_EDITOR_H

#include "engine/core/types.h"
#include "engine/scene/instance.h"
#include "engine/scene/part.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_map_editor {
    nds_instance* root;
    nds_instance* selected;
    float grid_size;
    uint32_t next_id_hint;
} nds_map_editor;

nds_result nds_map_editor_init(nds_map_editor* editor, nds_instance* root);
void nds_map_editor_reset(nds_map_editor* editor);
void nds_map_editor_set_grid(nds_map_editor* editor, float grid_size);
float nds_map_editor_get_grid(const nds_map_editor* editor);

nds_result nds_map_editor_select(nds_map_editor* editor, nds_instance* instance);
void nds_map_editor_clear_selection(nds_map_editor* editor);
nds_instance* nds_map_editor_get_selected(const nds_map_editor* editor);

nds_result nds_map_editor_create_part(nds_map_editor* editor, const char* name, nds_instance** out_part);
nds_result nds_map_editor_delete_selected(nds_map_editor* editor);
nds_result nds_map_editor_duplicate_selected(nds_map_editor* editor, nds_instance** out_copy);

nds_result nds_map_editor_set_position(nds_map_editor* editor, nds_vec3 position);
nds_result nds_map_editor_set_size(nds_map_editor* editor, nds_vec3 size);
nds_result nds_map_editor_set_rotation(nds_map_editor* editor, nds_vec3 rotation);
nds_result nds_map_editor_snap_position(const nds_map_editor* editor, nds_vec3 position, nds_vec3* out_position);
nds_result nds_map_editor_set_part_properties(nds_map_editor* editor, const nds_part_properties* properties);

#ifdef __cplusplus
}
#endif

#endif
