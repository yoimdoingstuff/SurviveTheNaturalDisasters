#include "engine/game/disaster.h"
#include "engine/scene/part.h"
#include <assert.h>

int main(void)
{
    nds_earthquake earthquake;
    nds_player_controller player = {0};
    nds_instance* scene = nds_instance_create(NDS_CLASS_DATAMODEL, "TestWorld");
    nds_instance* loose = nds_instance_create(NDS_CLASS_PART, "LoosePart");
    nds_instance* shelter = nds_instance_create(NDS_CLASS_PART, "Shelter");
    nds_part_properties props = {0};
    nds_vec3 original;

    assert(scene && loose && shelter);
    props.position = (nds_vec3){2.0f, 1.0f, 3.0f};
    props.size = (nds_vec3){2.0f, 2.0f, 2.0f};
    props.anchored = 0;
    props.can_collide = 1;
    props.visible = 1;
    assert(nds_part_set_properties(loose, &props) == NDS_OK);
    assert(nds_instance_set_parent(loose, scene) == NDS_OK);
    original = props.position;

    props.position = (nds_vec3){0.0f, 1.5f, 0.0f};
    props.size = (nds_vec3){4.0f, 3.0f, 4.0f};
    props.anchored = 1;
    props.can_collide = 1;
    props.visible = 1;
    assert(nds_part_set_properties(shelter, &props) == NDS_OK);
    assert(nds_instance_set_parent(shelter, scene) == NDS_OK);

    nds_earthquake_init(&earthquake);
    assert(!earthquake.active);
    assert(earthquake.pulse_interval == 4.0f);

    player.health = 100.0f;
    player.alive = 1;
    player.grounded = 0;
    player.position = (nds_vec3){8.0f, 1.5f, 8.0f};
    nds_earthquake_start(&earthquake);
    assert(earthquake.active);

    nds_earthquake_update(&earthquake, &player, scene, 4.0f);
    assert(earthquake.pulse_count == 1);
    assert(player.health == 75.0f);
    assert(player.alive);

    assert(nds_part_get_position(loose, &props.position) == NDS_OK);
    assert(props.position.x != original.x || props.position.z != original.z);
    nds_earthquake_stop(&earthquake, scene);
    assert(!earthquake.active);
    assert(nds_part_get_position(loose, &props.position) == NDS_OK);
    assert(props.position.x == original.x);
    assert(props.position.z == original.z);

    player.health = 100.0f;
    player.alive = 1;
    player.grounded = 1;
    player.position = (nds_vec3){0.0f, 1.5f, 0.0f};
    nds_earthquake_start(&earthquake);
    nds_earthquake_update(&earthquake, &player, scene, 4.0f);
    assert(earthquake.pulse_count == 1);
    assert(player.health == 100.0f);
    assert(player.alive);
    nds_earthquake_stop(&earthquake, scene);

    nds_instance_destroy(scene);
    return 0;
}
