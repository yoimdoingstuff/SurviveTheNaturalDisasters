#include "engine/render/draw.h"

#include <stdlib.h>

static nds_result reserve(nds_draw_list* list, size_t needed)
{
    size_t capacity;
    nds_draw_part* grown;
    if (needed <= list->capacity) return NDS_OK;
    capacity = list->capacity ? list->capacity : 64;
    while (capacity < needed) {
        if (capacity > ((size_t)-1) / 2) return NDS_ERR_UNKNOWN;
        capacity *= 2;
    }
    grown = (nds_draw_part*)realloc(list->parts, capacity * sizeof(*grown));
    if (!grown) return NDS_ERR_UNKNOWN;
    list->parts = grown;
    list->capacity = capacity;
    return NDS_OK;
}

void nds_draw_list_init(nds_draw_list* list)
{
    if (!list) return;
    list->parts = NULL; list->count = 0; list->capacity = 0;
}
void nds_draw_list_reset(nds_draw_list* list) { if (list) list->count = 0; }
void nds_draw_list_destroy(nds_draw_list* list)
{
    if (!list) return;
    free(list->parts); list->parts = NULL; list->count = 0; list->capacity = 0;
}
nds_result nds_draw_list_add_part(nds_draw_list* list, const nds_instance* instance)
{
    nds_part_properties props;
    nds_draw_part* draw;
    if (!list || !instance) return NDS_ERR_INVALID_ARG;
    if (nds_part_get_properties(instance, &props) != NDS_OK) return NDS_ERR_INVALID_ARG;
    if (!props.visible || props.transparency >= 1.0f) return NDS_OK;
    if (reserve(list, list->count + 1) != NDS_OK) return NDS_ERR_UNKNOWN;
    draw = &list->parts[list->count++];
    draw->position = props.position;
    draw->size = props.size;
    draw->rotation = props.rotation;
    draw->transparency = props.transparency;
    draw->color_rgba = props.color_rgba;
    draw->visible = props.visible;
    draw->mesh = props.mesh;
    draw->texture = props.texture;
    return NDS_OK;
}
static nds_result collect(nds_draw_list* list, const nds_instance* instance)
{
    size_t i; nds_instance_class class_id; nds_result result;
    if (!instance) return NDS_OK;
    class_id = nds_instance_get_class(instance);
    if (class_id == NDS_CLASS_PART || class_id == NDS_CLASS_SPAWN_POINT) {
        result = nds_draw_list_add_part(list, instance);
        if (result != NDS_OK) return result;
    }
    for (i = 0; i < nds_instance_child_count(instance); ++i) {
        result = collect(list, nds_instance_child_at(instance, i));
        if (result != NDS_OK) return result;
    }
    return NDS_OK;
}
nds_result nds_draw_list_build_from_tree(nds_draw_list* list, const nds_instance* root)
{
    if (!list || !root) return NDS_ERR_INVALID_ARG;
    nds_draw_list_reset(list);
    return collect(list, root);
}
