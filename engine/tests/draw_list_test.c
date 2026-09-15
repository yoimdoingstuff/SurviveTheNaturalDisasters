#include "engine/render/draw.h"
#include "engine/render/mesh.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "draw_list_test: check failed: %s\n", #expr); \
        return 1; \
    } \
} while (0)

int main(void)
{
    static const char* mesh_text =
        "NDSMESH 1\n"
        "vertex_count 3\n"
        "index_count 3\n"
        "vertices\n"
        "0 0 0\n"
        "1 0 0\n"
        "0 1 0\n"
        "indices\n"
        "0 1 2\n";
    nds_mesh mesh = {0};
    nds_instance* root = NULL;
    nds_instance* part = NULL;
    nds_draw_list list = {0};
    nds_part_properties props;

    CHECK(nds_mesh_load_text(mesh_text, &mesh) == NDS_OK);
    root = nds_instance_create(NDS_CLASS_DATAMODEL, "Workspace");
    part = nds_instance_create(NDS_CLASS_PART, "Island");
    CHECK(root != NULL);
    CHECK(part != NULL);
    CHECK(nds_instance_set_parent(part, root) == NDS_OK);
    CHECK(nds_part_get_properties(part, &props) == NDS_OK);
    props.position.x = 10.0f;
    props.position.y = 20.0f;
    props.position.z = -5.0f;
    props.size.x = 30.0f;
    props.size.y = 2.0f;
    props.size.z = 12.0f;
    props.transparency = 0.25f;
    props.color_rgba = 0x3366ccff;
    props.mesh = &mesh;
    CHECK(nds_part_set_properties(part, &props) == NDS_OK);

    nds_draw_list_init(&list);
    CHECK(nds_draw_list_build_from_tree(&list, root) == NDS_OK);
    CHECK(list.count == 1);
    CHECK(list.parts[0].position.x == 10.0f);
    CHECK(list.parts[0].size.z == 12.0f);
    CHECK(list.parts[0].transparency == 0.25f);
    CHECK(list.parts[0].color_rgba == 0x3366ccff);
    CHECK(list.parts[0].mesh == &mesh);

    props.visible = 0;
    CHECK(nds_part_set_properties(part, &props) == NDS_OK);
    CHECK(nds_draw_list_build_from_tree(&list, root) == NDS_OK);
    CHECK(list.count == 0);

    nds_draw_list_destroy(&list);
    nds_instance_destroy(root);
    nds_mesh_destroy(&mesh);
    puts("draw list test passed");
    return 0;
}
