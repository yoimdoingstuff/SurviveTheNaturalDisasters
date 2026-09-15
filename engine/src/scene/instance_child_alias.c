#include "engine/scene/instance.h"

nds_instance* nds_instance_get_child(const nds_instance* instance, size_t index)
{
    return nds_instance_child_at(instance, index);
}
