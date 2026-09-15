#include "engine/render/draw.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    nds_instance* root = nds_instance_create(NDS_CLASS_DATAMODEL, "Workspace");
    nds_instance* part = nds_instance_create(NDS_CLASS_PART, "Island");
    nds_draw_list list;
    nds_part_properties props;

    assert(root != NULL);
    assert(part != NULL);
    assert(nds_instance_set_parent(part, root) == NDS_OK);
    assert(nds_part_get_properties(part, &props) == NDS_OK);
    props.position.x = 10.0f;
    props.position.y = 20.0f;
    props.position.z = -5.0f;
    props.size.x = 30.0f;
    props.size.y = 2.0f;
    props.size.z = 12.0f;
    props.transparency = 0.25f;
    props.color_rgba = 0x3366ccff;
    assert(nds_part_set_properties(part, &props) == NDS_OK);

    nds_draw_list_init(&list);
    assert(nds_draw_list_build_from_tree(&list, root) == NDS_OK);
    assert(list.count == 1);
    assert(list.parts[0].position.x == 10.0f);
    assert(list.parts[0].size.z == 12.0f);
    assert(list.parts[0].transparency == 0.25f);
    assert(list.parts[0].color_rgba == 0x3366ccff);

    props.visible = 0;
    assert(nds_part_set_properties(part, &props) == NDS_OK);
    assert(nds_draw_list_build_from_tree(&list, root) == NDS_OK);
    assert(list.count == 0);

    nds_draw_list_destroy(&list);
    nds_instance_destroy(root);
    puts("draw list test passed");
    return 0;
}
