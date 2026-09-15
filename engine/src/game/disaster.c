#include "engine/game/disaster.h"

#include <math.h>

static void move_parts(nds_instance* root, float dx, float dz)
{
    size_t i;
    if (!root) return;
    for (i = 0; i < nds_instance_child_count(root); ++i) {
        nds_instance* child = (nds_instance*)nds_instance_child_at(root, i);
        nds_part_properties props;
        if (!child) continue;
        if (nds_instance_get_class(child) == NDS_CLASS_PART &&
            nds_part_get_properties(child, &props) == NDS_OK && !props.anchored) {
            props.position.x += dx;
            props.position.z += dz;
            nds_part_set_properties(child, &props);
        }
        if (nds_instance_child_count(child)) move_parts(child, dx, dz);
    }
}

void nds_earthquake_init(nds_earthquake* earthquake)
{
    if (!earthquake) return;
    earthquake->elapsed = 0.0f;
    earthquake->pulse_timer = 0.0f;
    earthquake->pulse_interval = 4.0f;
    earthquake->previous_shake_x = 0.0f;
    earthquake->previous_shake_z = 0.0f;
    earthquake->pulse_count = 0;
    earthquake->active = 0;
}

void nds_earthquake_start(nds_earthquake* earthquake)
{
    if (!earthquake) return;
    earthquake->elapsed = 0.0f;
    earthquake->pulse_timer = earthquake->pulse_interval;
    earthquake->previous_shake_x = 0.0f;
    earthquake->previous_shake_z = 0.0f;
    earthquake->pulse_count = 0;
    earthquake->active = 1;
}

void nds_earthquake_stop(nds_earthquake* earthquake)
{
    if (!earthquake) return;
    earthquake->active = 0;
}

void nds_earthquake_update(nds_earthquake* earthquake, nds_player_controller* player,
                           nds_instance* scene, float dt)
{
    float shake_x, shake_z, direction;
    if (!earthquake || !player || !scene || !earthquake->active || !player->alive) return;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.1f) dt = 0.1f;

    earthquake->elapsed += dt;
    earthquake->pulse_timer -= dt;

    /* Move unanchored parts around their original positions without accumulating drift. */
    shake_x = sinf(earthquake->elapsed * 18.0f) * 0.10f;
    shake_z = cosf(earthquake->elapsed * 15.0f) * 0.08f;
    move_parts(scene, shake_x - earthquake->previous_shake_x,
               shake_z - earthquake->previous_shake_z);
    earthquake->previous_shake_x = shake_x;
    earthquake->previous_shake_z = shake_z;

    /* Keep the player under a small continuous tremor. */
    direction = sinf(earthquake->elapsed * 18.0f);
    player->velocity.x += direction * 1.2f * dt;
    player->velocity.z += cosf(earthquake->elapsed * 15.0f) * 1.0f * dt;

    if (earthquake->pulse_timer <= 0.0f) {
        earthquake->pulse_timer += earthquake->pulse_interval;
        earthquake->pulse_count++;
        direction = (earthquake->pulse_count & 1u) ? 1.0f : -1.0f;
        player->velocity.x += direction * 5.0f;
        player->velocity.z += (earthquake->pulse_count % 3u == 0u) ? 4.0f : -3.0f;
        if (!player->grounded) nds_player_damage(player, 25.0f);
    }
}
