#include "engine/scene/part.h"
#include <stdlib.h>
#include <string.h>

typedef struct nds_part_state { nds_part_properties properties; } nds_part_state;

static int is_part_class(nds_instance_class c) { return c == NDS_CLASS_PART || c == NDS_CLASS_SPAWN_POINT; }
static void destroy_state(void* p) { free(p); }
static nds_part_state* make_default(void)
{
    nds_part_state* s = (nds_part_state*)calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->properties.size.x = s->properties.size.y = s->properties.size.z = 1.0f;
    s->properties.color_rgba = 0xffffffffu;
    s->properties.anchored = 1;
    s->properties.can_collide = 1;
    s->properties.visible = 1;
    s->properties.mesh = NULL;
    return s;
}
static nds_part_state* ensure_state(nds_instance* i)
{
    nds_part_state* s;
    if (!i || !is_part_class(nds_instance_get_class(i))) return NULL;
    s = (nds_part_state*)nds_instance_get_user_data(i);
    if (s) return s;
    s = make_default();
    if (!s || nds_instance_set_user_data(i, s, destroy_state) != NDS_OK) { free(s); return NULL; }
    return s;
}

nds_result nds_part_get_properties(const nds_instance* i, nds_part_properties* out)
{
    nds_part_state* s;
    if (!i || !out || !is_part_class(nds_instance_get_class(i))) return NDS_ERR_INVALID_ARG;
    s = (nds_part_state*)nds_instance_get_user_data(i);
    if (!s) { s = make_default(); if (!s) return NDS_ERR_UNKNOWN; *out = s->properties; free(s); return NDS_OK; }
    *out = s->properties;
    return NDS_OK;
}
nds_result nds_part_set_properties(nds_instance* i, const nds_part_properties* p)
{
    nds_part_state* s;
    if (!p) return NDS_ERR_INVALID_ARG;
    s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    s->properties = *p; return NDS_OK;
}
nds_result nds_part_set_position(nds_instance* i, nds_vec3 p)
{
    nds_part_state* s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    s->properties.position = p; return NDS_OK;
}
nds_result nds_part_get_position(const nds_instance* i, nds_vec3* out)
{
    nds_part_properties p;
    if (!out || nds_part_get_properties(i, &p) != NDS_OK) return NDS_ERR_INVALID_ARG;
    *out = p.position; return NDS_OK;
}
nds_result nds_part_set_size(nds_instance* i, nds_vec3 size)
{
    nds_part_state* s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    s->properties.size = size; return NDS_OK;
}
nds_result nds_part_get_size(const nds_instance* i, nds_vec3* out)
{
    nds_part_properties p;
    if (!out || nds_part_get_properties(i, &p) != NDS_OK) return NDS_ERR_INVALID_ARG;
    *out = p.size; return NDS_OK;
}
nds_result nds_part_set_mesh(nds_instance* i, const nds_mesh* mesh)
{
    nds_part_state* s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    s->properties.mesh = mesh;
    return NDS_OK;
}
nds_result nds_part_get_mesh(const nds_instance* i, const nds_mesh** out_mesh)
{
    nds_part_properties p;
    if (!out_mesh || nds_part_get_properties(i, &p) != NDS_OK) return NDS_ERR_INVALID_ARG;
    *out_mesh = p.mesh;
    return NDS_OK;
}
