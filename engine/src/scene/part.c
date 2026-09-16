#include "engine/scene/part.h"
#include <stdlib.h>
#include <string.h>

typedef struct nds_part_state {
    nds_part_properties properties;
    char* mesh_asset;
    char* texture_asset;
} nds_part_state;

static int is_part_class(nds_instance_class c) { return c == NDS_CLASS_PART || c == NDS_CLASS_SPAWN_POINT; }
static void destroy_state(void* p)
{
    nds_part_state* s = (nds_part_state*)p;
    if (!s) return;
    free(s->mesh_asset);
    free(s->texture_asset);
    free(s);
}
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
    s->properties.texture = NULL;
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

static void create_view_tower_part(nds_instance* root, const char* name,
                                   nds_vec3 position, nds_vec3 size, uint32_t color)
{
    nds_instance* part;
    nds_part_properties props = {0};
    if (!root || !name) return;
    part = nds_instance_create(NDS_CLASS_PART, name);
    if (!part) return;
    props.position = position;
    props.size = size;
    props.rotation = (nds_vec3){0.0f, 0.0f, 0.0f};
    props.color_rgba = color;
    props.anchored = 1;
    props.can_collide = 1;
    props.visible = 1;
    if (nds_instance_set_parent(part, root) != NDS_OK ||
        nds_part_set_properties(part, &props) != NDS_OK)
        nds_instance_destroy(part);
}

static void ensure_island_view_tower(nds_instance* instance)
{
    nds_instance* root;
    const float x = -12.0f;
    const float z = -8.5f;
    const uint32_t steel = 0xd7daddu;
    const uint32_t trim = 0x69727affu;
    const uint32_t roof = 0x8f3030ffu;

    if (!instance) return;
    if (strcmp(nds_instance_get_name(instance), "IslandBase") != 0) return;
    root = nds_instance_get_parent(instance);
    if (!root || nds_instance_find_child(root, "IslandViewTowerDeck")) return;

    create_view_tower_part(root, "IslandViewTowerBase", (nds_vec3){x, -0.05f, z}, (nds_vec3){4.5f, 0.6f, 4.5f}, trim);
    create_view_tower_part(root, "IslandViewTowerLegNW", (nds_vec3){x - 1.65f, 3.0f, z - 1.65f}, (nds_vec3){0.45f, 6.0f, 0.45f}, steel);
    create_view_tower_part(root, "IslandViewTowerLegNE", (nds_vec3){x + 1.65f, 3.0f, z - 1.65f}, (nds_vec3){0.45f, 6.0f, 0.45f}, steel);
    create_view_tower_part(root, "IslandViewTowerLegSW", (nds_vec3){x - 1.65f, 3.0f, z + 1.65f}, (nds_vec3){0.45f, 6.0f, 0.45f}, steel);
    create_view_tower_part(root, "IslandViewTowerLegSE", (nds_vec3){x + 1.65f, 3.0f, z + 1.65f}, (nds_vec3){0.45f, 6.0f, 0.45f}, steel);
    create_view_tower_part(root, "IslandViewTowerDeck", (nds_vec3){x, 6.15f, z}, (nds_vec3){6.0f, 0.5f, 6.0f}, trim);
    create_view_tower_part(root, "IslandViewTowerRailNorth", (nds_vec3){x, 7.35f, z - 2.75f}, (nds_vec3){5.8f, 1.8f, 0.35f}, steel);
    create_view_tower_part(root, "IslandViewTowerRailSouth", (nds_vec3){x, 7.35f, z + 2.75f}, (nds_vec3){5.8f, 1.8f, 0.35f}, steel);
    create_view_tower_part(root, "IslandViewTowerRailEast", (nds_vec3){x + 2.75f, 7.35f, z}, (nds_vec3){0.35f, 1.8f, 5.8f}, steel);
    create_view_tower_part(root, "IslandViewTowerRailWest", (nds_vec3){x - 2.75f, 7.35f, z}, (nds_vec3){0.35f, 1.8f, 5.8f}, steel);
    create_view_tower_part(root, "IslandViewTowerRoof", (nds_vec3){x, 8.65f, z}, (nds_vec3){6.4f, 0.35f, 6.4f}, roof);
    create_view_tower_part(root, "IslandViewTowerLadder", (nds_vec3){x + 2.2f, 3.0f, z}, (nds_vec3){0.35f, 5.9f, 1.0f}, trim);
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
    s->properties = *p;
    ensure_island_view_tower(i);
    return NDS_OK;
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
nds_result nds_part_set_texture(nds_instance* i, const nds_texture* texture)
{
    nds_part_state* s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    s->properties.texture = texture;
    return NDS_OK;
}
nds_result nds_part_get_texture(const nds_instance* i, const nds_texture** out_texture)
{
    nds_part_properties p;
    if (!out_texture || nds_part_get_properties(i, &p) != NDS_OK) return NDS_ERR_INVALID_ARG;
    *out_texture = p.texture; return NDS_OK;
}

static nds_result set_asset(char** slot, const char* path)
{
    char* copy = NULL;
    if (path && path[0]) {
        size_t len = strlen(path);
        copy = (char*)malloc(len + 1);
        if (!copy) return NDS_ERR_UNKNOWN;
        memcpy(copy, path, len + 1);
    }
    free(*slot);
    *slot = copy;
    return NDS_OK;
}
nds_result nds_part_set_mesh_asset(nds_instance* i, const char* path)
{
    nds_part_state* s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    return set_asset(&s->mesh_asset, path);
}
nds_result nds_part_get_mesh_asset(const nds_instance* i, const char** out_path)
{
    nds_part_state* s;
    if (!i || !out_path || !is_part_class(nds_instance_get_class(i))) return NDS_ERR_INVALID_ARG;
    s = (nds_part_state*)nds_instance_get_user_data(i);
    *out_path = s ? s->mesh_asset : NULL;
    return NDS_OK;
}
nds_result nds_part_set_texture_asset(nds_instance* i, const char* path)
{
    nds_part_state* s = ensure_state(i); if (!s) return NDS_ERR_INVALID_ARG;
    return set_asset(&s->texture_asset, path);
}
nds_result nds_part_get_texture_asset(const nds_instance* i, const char** out_path)
{
    nds_part_state* s;
    if (!i || !out_path || !is_part_class(nds_instance_get_class(i))) return NDS_ERR_INVALID_ARG;
    s = (nds_part_state*)nds_instance_get_user_data(i);
    *out_path = s ? s->texture_asset : NULL;
    return NDS_OK;
}
