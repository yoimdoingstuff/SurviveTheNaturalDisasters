#include "engine/content/map_validation.h"
#include "engine/scene/part.h"
#include <math.h>
#include <string.h>

static int finite_positive(float value)
{
    return isfinite(value) && value > 0.001f;
}

static int is_interactive_name(const char* name)
{
    if (!name) return 0;
    return strstr(name, "Button") != NULL ||
           strstr(name, "Switch") != NULL ||
           strstr(name, "Trigger") != NULL ||
           strstr(name, "Door") != NULL;
}

static int spawn_has_ground(const nds_instance* root, nds_vec3 spawn)
{
    size_t i, count;
    if (!root) return 0;
    count = nds_instance_child_count(root);
    for (i = 0; i < count; ++i) {
        const nds_instance* child = nds_instance_child_at(root, i);
        nds_part_properties props;
        float dx, dz, top;
        if (!child || nds_instance_get_class(child) != NDS_CLASS_PART) continue;
        if (nds_part_get_properties(child, &props) != NDS_OK) continue;
        if (!props.visible || !props.can_collide) continue;
        if (!finite_positive(props.size.x) || !finite_positive(props.size.y) || !finite_positive(props.size.z)) continue;
        dx = fabsf(props.position.x - spawn.x);
        dz = fabsf(props.position.z - spawn.z);
        top = props.position.y + props.size.y * 0.5f;
        if (dx <= 2.0f && dz <= 2.0f && top <= spawn.y + 0.25f && top >= spawn.y - 6.0f)
            return 1;
    }
    return 0;
}

void nds_map_validation_report_init(nds_map_validation_report* report)
{
    if (!report) return;
    memset(report, 0, sizeof(*report));
}

nds_result nds_map_validate(const nds_instance* scene, nds_map_validation_report* report)
{
    size_t i, count;
    if (!scene || !report) return NDS_ERR_INVALID_ARG;
    nds_map_validation_report_init(report);
    count = nds_instance_child_count(scene);

    for (i = 0; i < count; ++i) {
        const nds_instance* child = nds_instance_child_at(scene, i);
        nds_instance_class class_id;
        const char* name;
        nds_part_properties props;
        if (!child) continue;
        class_id = nds_instance_get_class(child);
        name = nds_instance_get_name(child);
        if (class_id == NDS_CLASS_SPAWN_POINT) {
            report->spawn_count++;
            if (nds_part_get_properties(child, &props) != NDS_OK || !spawn_has_ground(scene, props.position))
                report->unsafe_spawn_count++;
            continue;
        }
        if (class_id != NDS_CLASS_PART || nds_part_get_properties(child, &props) != NDS_OK)
            continue;

        report->part_count++;
        if (!finite_positive(props.size.x) || !finite_positive(props.size.y) || !finite_positive(props.size.z) ||
            !isfinite(props.position.x) || !isfinite(props.position.y) || !isfinite(props.position.z))
            report->invalid_part_count++;
        if (props.visible && props.can_collide &&
            (props.size.x > 64.0f || props.size.y > 64.0f || props.size.z > 64.0f))
            report->non_granular_part_count++;
        if (is_interactive_name(name))
            report->interaction_count++;
    }

    return NDS_OK;
}

int nds_map_validation_has_errors(const nds_map_validation_report* report)
{
    if (!report) return 1;
    return report->part_count < 8 ||
           report->spawn_count == 0 ||
           report->invalid_part_count != 0 ||
           report->unsafe_spawn_count != 0 ||
           report->non_granular_part_count != 0;
}
