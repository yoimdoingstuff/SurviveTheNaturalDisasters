#include "engine/content/map_editor.h"

#include <math.h>
#include <string.h>

static nds_instance* selected_or_null(const nds_map_editor* editor)
{
    if (!editor || !editor->selected) return NULL;
    if (editor->selected == editor->root) return NULL;
    return editor->selected;
}

static float snap_value(float value, float grid)
{
    return floorf((value / grid) + 0.5f) * grid;
}

nds_result nds_map_editor_init(nds_map_editor* editor, nds_instance* root)
{
    if (!editor || !root) return NDS_ERR_INVALID_ARG;
    memset(editor, 0, sizeof(*editor));
    editor->root = root;
    editor->grid_size = 1.0f;
    editor->next_id_hint = 1u;
    return NDS_OK;
}

void nds_map_editor_reset(nds_map_editor* editor)
{
    if (!editor) return;
    editor->root = NULL;
    editor->selected = NULL;
    editor->grid_size = 1.0f;
    editor->next_id_hint = 1u;
}

void nds_map_editor_set_grid(nds_map_editor* editor, float grid_size)
{
    if (!editor || grid_size <= 0.0f) return;
    editor->grid_size = grid_size;
}

float nds_map_editor_get_grid(const nds_map_editor* editor)
{
    return editor ? editor->grid_size : 1.0f;
}

nds_result nds_map_editor_select(nds_map_editor* editor, nds_instance* instance)
{
    if (!editor || !editor->root || !instance) return NDS_ERR_INVALID_ARG;
    if (instance != editor->root && nds_instance_get_parent(instance) == NULL) return NDS_ERR_INVALID_ARG;
    editor->selected = instance;
    return NDS_OK;
}

void nds_map_editor_clear_selection(nds_map_editor* editor)
{
    if (editor) editor->selected = NULL;
}

nds_instance* nds_map_editor_get_selected(const nds_map_editor* editor)
{
    return editor ? editor->selected : NULL;
}

nds_result nds_map_editor_create_part(nds_map_editor* editor, const char* name, nds_instance** out_part)
{
    nds_instance* part;
    nds_part_properties props;
    if (!editor || !editor->root || !out_part) return NDS_ERR_INVALID_ARG;
    part = nds_instance_create(NDS_CLASS_PART, name && name[0] ? name : "Part");
    if (!part) return NDS_ERR_UNKNOWN;
    if (nds_instance_set_parent(part, editor->root) != NDS_OK) {
        nds_instance_destroy(part);
        return NDS_ERR_UNKNOWN;
    }
    memset(&props, 0, sizeof(props));
    props.position = (nds_vec3){0.0f, 1.0f, 0.0f};
    props.size = (nds_vec3){4.0f, 1.0f, 4.0f};
    props.rotation = (nds_vec3){0.0f, 0.0f, 0.0f};
    props.color_rgba = 0xffffffffu;
    props.visible = 1u;
    props.anchored = 1u;
    props.can_collide = 1u;
    if (nds_part_set_properties(part, &props) != NDS_OK) {
        nds_instance_destroy(part);
        return NDS_ERR_UNKNOWN;
    }
    editor->selected = part;
    editor->next_id_hint++;
    *out_part = part;
    return NDS_OK;
}

nds_result nds_map_editor_delete_selected(nds_map_editor* editor)
{
    nds_instance* selected = selected_or_null(editor);
    if (!selected) return NDS_ERR_INVALID_ARG;
    nds_instance_destroy(selected);
    editor->selected = NULL;
    return NDS_OK;
}

nds_result nds_map_editor_duplicate_selected(nds_map_editor* editor, nds_instance** out_copy)
{
    nds_instance* selected = selected_or_null(editor);
    nds_instance* copy;
    nds_part_properties props;
    nds_instance* parent;
    const char* name;
    char duplicate_name[128];
    if (!selected || !out_copy) return NDS_ERR_INVALID_ARG;
    if (nds_instance_get_class(selected) != NDS_CLASS_PART &&
        nds_instance_get_class(selected) != NDS_CLASS_SPAWN_POINT) return NDS_ERR_INVALID_ARG;
    parent = nds_instance_get_parent(selected);
    if (!parent) return NDS_ERR_INVALID_ARG;
    name = nds_instance_get_name(selected);
    if (!name) name = "Part";
    (void)snprintf(duplicate_name, sizeof(duplicate_name), "%s_Copy", name);
    copy = nds_instance_create(nds_instance_get_class(selected), duplicate_name);
    if (!copy) return NDS_ERR_UNKNOWN;
    if (nds_instance_set_parent(copy, parent) != NDS_OK ||
        nds_part_get_properties(selected, &props) != NDS_OK ||
        nds_part_set_properties(copy, &props) != NDS_OK) {
        nds_instance_destroy(copy);
        return NDS_ERR_UNKNOWN;
    }
    props.position.x += editor->grid_size;
    if (nds_part_set_position(copy, props.position) != NDS_OK) {
        nds_instance_destroy(copy);
        return NDS_ERR_UNKNOWN;
    }
    editor->selected = copy;
    editor->next_id_hint++;
    *out_copy = copy;
    return NDS_OK;
}

nds_result nds_map_editor_set_position(nds_map_editor* editor, nds_vec3 position)
{
    nds_instance* selected = selected_or_null(editor);
    if (!selected) return NDS_ERR_INVALID_ARG;
    return nds_part_set_position(selected, position);
}

nds_result nds_map_editor_set_size(nds_map_editor* editor, nds_vec3 size)
{
    nds_instance* selected = selected_or_null(editor);
    if (!selected || size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f) return NDS_ERR_INVALID_ARG;
    return nds_part_set_size(selected, size);
}

nds_result nds_map_editor_set_rotation(nds_map_editor* editor, nds_vec3 rotation)
{
    nds_instance* selected = selected_or_null(editor);
    nds_part_properties props;
    if (!selected || nds_part_get_properties(selected, &props) != NDS_OK) return NDS_ERR_INVALID_ARG;
    props.rotation = rotation;
    return nds_part_set_properties(selected, &props);
}

nds_result nds_map_editor_snap_position(const nds_map_editor* editor, nds_vec3 position, nds_vec3* out_position)
{
    float grid;
    if (!editor || !out_position || editor->grid_size <= 0.0f) return NDS_ERR_INVALID_ARG;
    grid = editor->grid_size;
    out_position->x = snap_value(position.x, grid);
    out_position->y = snap_value(position.y, grid);
    out_position->z = snap_value(position.z, grid);
    return NDS_OK;
}

nds_result nds_map_editor_set_part_properties(nds_map_editor* editor, const nds_part_properties* properties)
{
    nds_instance* selected = selected_or_null(editor);
    if (!selected || !properties) return NDS_ERR_INVALID_ARG;
    return nds_part_set_properties(selected, properties);
}
