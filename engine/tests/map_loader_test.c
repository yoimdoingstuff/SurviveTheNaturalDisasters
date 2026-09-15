#include "engine/content/map_loader.h"
#include "engine/scene/part.h"

#include <stdio.h>

static int test_valid_map(void)
{
    const char* json =
        "{\"format\":\"nds-map\",\"version\":1,\"instances\":["
        "{\"id\":0,\"referent\":null,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null,\"properties\":{},\"children\":[1]},"
        "{\"id\":1,\"referent\":null,\"class\":\"Part\",\"name\":\"Island\",\"parent\":0,\"properties\":{},\"children\":[],"
        "\"transform\":{\"position\":[1,2,3],\"size\":[10,2,8],\"rotation\":[0,45,0]},"
        "\"part\":{\"transparency\":0.25,\"reflectance\":0.1,\"anchored\":true,\"can_collide\":true,\"color\":[0.5,0.25,1.0]}}]}";
    nds_instance* root = NULL;
    nds_instance* part;
    nds_part_properties props;
    if (nds_map_load_json_text(json, &root) != NDS_OK || !root) return 1;
    part = nds_instance_find_child(root, "Island");
    if (!part || nds_part_get_properties(part, &props) != NDS_OK) { nds_instance_destroy(root); return 2; }
    if (props.position.x != 1.0f || props.position.y != 2.0f || props.position.z != 3.0f) { nds_instance_destroy(root); return 3; }
    if (props.size.x != 10.0f || props.size.y != 2.0f || props.size.z != 8.0f) { nds_instance_destroy(root); return 4; }
    if (props.transparency != 0.25f || props.reflectance != 0.1f || !props.anchored || !props.can_collide) { nds_instance_destroy(root); return 5; }
    if (props.color_rgba != 0x7f3fffFFu && props.color_rgba != 0x7f40ffffu) { nds_instance_destroy(root); return 6; }
    nds_instance_destroy(root);
    return 0;
}

static int test_rejects_invalid_parent(void)
{
    const char* json =
        "{\"format\":\"nds-map\",\"version\":1,\"instances\":["
        "{\"id\":10,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},"
        "{\"id\":20,\"class\":\"Part\",\"name\":\"Broken\",\"parent\":999}]}";
    nds_instance* root = NULL;
    return nds_map_load_json_text(json, &root) == NDS_OK || root != NULL;
}

static int test_rejects_duplicate_id(void)
{
    const char* json =
        "{\"format\":\"nds-map\",\"version\":1,\"instances\":["
        "{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null},"
        "{\"id\":0,\"class\":\"Part\",\"name\":\"Duplicate\",\"parent\":0}]}";
    nds_instance* root = NULL;
    return nds_map_load_json_text(json, &root) == NDS_OK || root != NULL;
}

static int test_rejects_trailing_data(void)
{
    const char* json =
        "{\"format\":\"nds-map\",\"version\":1,\"instances\":["
        "{\"id\":0,\"class\":\"DataModel\",\"name\":\"Game\",\"parent\":null}]} garbage";
    nds_instance* root = NULL;
    return nds_map_load_json_text(json, &root) == NDS_OK || root != NULL;
}

int main(void)
{
    int rc;
    rc = test_valid_map(); if (rc) return rc;
    if (test_rejects_invalid_parent()) return 10;
    if (test_rejects_duplicate_id()) return 11;
    if (test_rejects_trailing_data()) return 12;
    puts("map loader test passed");
    return 0;
}
