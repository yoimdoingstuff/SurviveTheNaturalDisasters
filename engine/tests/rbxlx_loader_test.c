#include "engine/content/rbxlx_loader.h"
#include "engine/scene/part.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    nds_instance* root = NULL;
    nds_instance* part;
    const char* path = "engine/tests/fixtures/minimal.rbxlx";
    assert(nds_rbxlx_load(path, &root) == NDS_OK);
    assert(root != NULL);
    assert(nds_instance_child_count(root) == 2);
    part = nds_instance_child_at(root, 0);
    assert(part != NULL);
    assert(nds_instance_get_class(part) == NDS_CLASS_PART);
    {
        nds_part_properties props;
        assert(nds_part_get_properties(part, &props) == NDS_OK);
        assert(props.position.x == 2.0f);
        assert(props.position.y == 3.0f);
        assert(props.position.z == 4.0f);
        assert(props.size.x == 6.0f);
        assert(props.size.y == 2.0f);
        assert(props.size.z == 4.0f);
        assert(props.anchored == 1);
        assert(props.can_collide == 1);
    }
    nds_instance_destroy(root);
    puts("rbxlx loader tests passed");
    return 0;
}
