#include "engine/content/map_loader.h"
#include "engine/scene/part.h"
#include "engine/render/mesh_cache.h"
#include "engine/render/texture_cache.h"

#include <stdio.h>

static int test_valid_map(void)
{
    const char* json = "{\"format\":\"nds-map\",\"version\":1,\"instances\":[{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},{\"id\":1,\"class\":\"Part\",\"name\":\"Island\",\"parent\":0,\"transform\":{\"position\":[1,2,3],\"size\":[10,2,8],\"rotation\":[0,45,0]},\"part\":{\"transparency\":0.25,\"reflectance\":0.1,\"anchored\":true,\"can_collide\":true,\"color\":[0.5,0.25,1.0]}}]}";
    nds_instance* root = NULL; nds_instance* part; nds_part_properties props;
    if (nds_map_load_json_text(json, &root) != NDS_OK || !root) return 1;
    part = nds_instance_find_child(root, "Island");
    if (!part || nds_part_get_properties(part, &props) != NDS_OK) { nds_instance_destroy(root); return 2; }
    if (props.position.x != 1.0f || props.position.y != 2.0f || props.position.z != 3.0f) { nds_instance_destroy(root); return 3; }
    if (props.size.x != 10.0f || props.size.y != 2.0f || props.size.z != 8.0f) { nds_instance_destroy(root); return 4; }
    if (props.transparency != 0.25f || props.reflectance != 0.1f || !props.anchored || !props.can_collide) { nds_instance_destroy(root); return 5; }
    if (props.color_rgba != 0x7f3fffFFu && props.color_rgba != 0x7f40ffffu) { nds_instance_destroy(root); return 6; }
    nds_instance_destroy(root); return 0;
}

static int test_mesh_resolution(void)
{
    const char* json = "{\"format\":\"nds-map\",\"version\":1,\"instances\":[{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},{\"id\":1,\"class\":\"MeshPart\",\"name\":\"Triangle\",\"parent\":0,\"geometry\":{\"type\":\"mesh\",\"mesh\":\"game/content/meshes/triangle.ndsmesh\"}}]}";
    nds_instance* root = NULL; nds_instance* part; nds_mesh_cache* cache = NULL; const nds_mesh* mesh = NULL;
    if (nds_map_load_json_text(json, &root) != NDS_OK || !root) return 1;
    part = nds_instance_find_child(root, "Triangle"); if (!part) { nds_instance_destroy(root); return 2; }
    if (nds_mesh_cache_create(&cache, 4) != NDS_OK) { nds_instance_destroy(root); return 3; }
    if (nds_map_resolve_meshes(root, cache) != NDS_OK) { nds_mesh_cache_destroy(cache); nds_instance_destroy(root); return 4; }
    if (nds_part_get_mesh(part, &mesh) != NDS_OK || !mesh || mesh->vertex_count != 3 || mesh->index_count != 3) { nds_mesh_cache_destroy(cache); nds_instance_destroy(root); return 5; }
    if (nds_mesh_cache_count(cache) != 1) { nds_mesh_cache_destroy(cache); nds_instance_destroy(root); return 6; }
    nds_mesh_cache_destroy(cache); nds_instance_destroy(root); return 0;
}

static int test_texture_resolution(void)
{
    const char* json = "{\"format\":\"nds-map\",\"version\":1,\"instances\":[{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},{\"id\":1,\"class\":\"Part\",\"name\":\"Textured\",\"parent\":0,\"geometry\":{\"texture\":\"game/content/textures/fixture.rgba8\"}}]}";
    nds_instance* root = NULL; nds_instance* part; nds_texture_cache* cache = NULL; const nds_texture* texture = NULL;
    if (nds_map_load_json_text(json, &root) != NDS_OK || !root) return 1;
    part = nds_instance_find_child(root, "Textured"); if (!part) { nds_instance_destroy(root); return 2; }
    if (nds_texture_cache_create(&cache, 4) != NDS_OK) { nds_instance_destroy(root); return 3; }
    if (nds_map_resolve_textures(root, cache) != NDS_OK) { nds_texture_cache_destroy(cache); nds_instance_destroy(root); return 4; }
    if (nds_part_get_texture(part, &texture) != NDS_OK || !texture || texture->width != 2 || texture->height != 2 || !texture->rgba8) { nds_texture_cache_destroy(cache); nds_instance_destroy(root); return 5; }
    if (nds_texture_cache_count(cache) != 1) { nds_texture_cache_destroy(cache); nds_instance_destroy(root); return 6; }
    nds_texture_cache_destroy(cache); nds_instance_destroy(root); return 0;
}

static int test_rejects_invalid_parent(void)
{
    const char* json = "{\"format\":\"nds-map\",\"version\":1,\"instances\":[{\"id\":10,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},{\"id\":20,\"class\":\"Part\",\"name\":\"Broken\",\"parent\":999}]}";
    nds_instance* root = NULL; return nds_map_load_json_text(json, &root) == NDS_OK || root != NULL;
}
static int test_rejects_duplicate_id(void)
{
    const char* json = "{\"format\":\"nds-map\",\"version\":1,\"instances\":[{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},{\"id\":0,\"class\":\"Part\",\"name\":\"Duplicate\",\"parent\":0}]}";
    nds_instance* root = NULL; return nds_map_load_json_text(json, &root) == NDS_OK || root != NULL;
}
static int test_rejects_trailing_data(void)
{
    const char* json = "{\"format\":\"nds-map\",\"version\":1,\"instances\":[{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null}]} garbage";
    nds_instance* root = NULL; return nds_map_load_json_text(json, &root) == NDS_OK || root != NULL;
}
int main(void)
{
    int rc;
    rc = test_valid_map(); if (rc) return rc;
    rc = test_mesh_resolution(); if (rc) return 20 + rc;
    rc = test_texture_resolution(); if (rc) return 30 + rc;
    if (test_rejects_invalid_parent()) return 10;
    if (test_rejects_duplicate_id()) return 11;
    if (test_rejects_trailing_data()) return 12;
    puts("map loader test passed"); return 0;
}
