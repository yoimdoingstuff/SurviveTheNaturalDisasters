#include "engine/content/map_editor.h"

#include <stdio.h>

static int test_editor_workflow(void)
{
    nds_instance* root = nds_instance_create(NDS_CLASS_DATAMODEL, "EditorTest");
    nds_map_editor editor;
    nds_instance* part = NULL;
    nds_instance* copy = NULL;
    nds_vec3 snapped;
    nds_part_properties props;
    int rc;

    if (!root) return 1;
    rc = nds_map_editor_init(&editor, root);
    if (rc != NDS_OK) { nds_instance_destroy(root); return 2; }
    nds_map_editor_set_grid(&editor, 0.5f);
    if (nds_map_editor_create_part(&editor, "TestPart", &part) != NDS_OK || !part) {
        nds_instance_destroy(root); return 3;
    }
    if (nds_map_editor_get_selected(&editor) != part) {
        nds_instance_destroy(root); return 4;
    }
    if (nds_map_editor_set_position(&editor, (nds_vec3){1.0f, 2.0f, 3.0f}) != NDS_OK) {
        nds_instance_destroy(root); return 5;
    }
    if (nds_map_editor_snap_position(&editor, (nds_vec3){1.24f, 2.26f, 3.74f}, &snapped) != NDS_OK ||
        snapped.x != 1.0f || snapped.y != 2.5f || snapped.z != 3.5f) {
        nds_instance_destroy(root); return 6;
    }
    if (nds_map_editor_set_size(&editor, (nds_vec3){2.0f, 3.0f, 4.0f}) != NDS_OK ||
        nds_part_get_properties(part, &props) != NDS_OK ||
        props.size.x != 2.0f || props.size.y != 3.0f || props.size.z != 4.0f) {
        nds_instance_destroy(root); return 7;
    }
    if (nds_map_editor_duplicate_selected(&editor, &copy) != NDS_OK || !copy || copy == part) {
        nds_instance_destroy(root); return 8;
    }
    if (nds_instance_child_count(root) != 2) {
        nds_instance_destroy(root); return 9;
    }
    if (nds_map_editor_delete_selected(&editor) != NDS_OK || nds_instance_child_count(root) != 1) {
        nds_instance_destroy(root); return 10;
    }
    nds_map_editor_clear_selection(&editor);
    if (nds_map_editor_get_selected(&editor) != NULL) {
        nds_instance_destroy(root); return 11;
    }
    nds_instance_destroy(root);
    puts("map editor test passed");
    return 0;
}

int main(void)
{
    return test_editor_workflow();
}
