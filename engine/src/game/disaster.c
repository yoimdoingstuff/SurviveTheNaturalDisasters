#include "engine/game/disaster.h"
#include "engine/scene/part.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static void move_parts(nds_instance* root, float dx, float dz)
{
    size_t i;
    if (!root) return;
    for (i = 0; i < nds_instance_child_count(root); ++i) {
        nds_instance* child = (nds_instance*)nds_instance_child_at(root, i);
        nds_part_properties props;
        const char* name;
        if (!child) continue;
        name = nds_instance_get_name(child);
        if ((nds_instance_get_class(child) == NDS_CLASS_PART) &&
            nds_part_get_properties(child, &props) == NDS_OK && props.visible &&
            (!name || strncmp(name, "Player", 6) != 0)) {
            props.position.x += dx;
            props.position.z += dz;
            nds_part_set_position(child, props.position);
        }
        if (nds_instance_child_count(child)) move_parts(child, dx, dz);
    }
}

static int protected_part_name(const char* name)
{
    if (!name) return 0;
    if (strncmp(name, "Player", 6) == 0) return 1;
    if (strcmp(name, "Shelter") == 0 || strcmp(name, "ShelterRoof") == 0) return 1;
    if (strcmp(name, "Spawn") == 0 || strcmp(name, "Ocean") == 0) return 1;
    if (strcmp(name, "IslandBase") == 0 || strcmp(name, "IslandShore") == 0) return 1;
    if (strncmp(name, "Island", 6) == 0) return 1;
    return 0;
}

static void break_map_blocks(nds_instance* root, uint32_t pulse_count)
{
    static uint32_t serial = 0;
    size_t i;
    if (!root) return;
    for (i = 0; i < nds_instance_child_count(root); ++i) {
        nds_instance* child = (nds_instance*)nds_instance_child_at(root, i);
        nds_part_properties props;
        const char* name;
        if (!child) continue;
        name = nds_instance_get_name(child);
        if (nds_instance_get_class(child) == NDS_CLASS_PART &&
            nds_part_get_properties(child, &props) == NDS_OK &&
            props.visible && props.can_collide && !protected_part_name(name)) {
            unsigned int pick = (unsigned int)((serial++ + pulse_count * 3u) % 7u);
            if (pick <= 2u) {
                float wobble = (float)((int)((serial * 13u) % 11u) - 5) * 0.08f;
                props.anchored = 0;
                props.rotation.x += wobble * 3.0f;
                props.rotation.z -= wobble * 2.0f;
                props.position.x += wobble;
                props.position.z -= wobble * 0.7f;
                nds_part_set_properties(child, &props);
            } else if (pulse_count >= 2u && pick == 3u) {
                props.anchored = 0;
                props.can_collide = 0;
                props.visible = 0;
                nds_part_set_properties(child, &props);
            }
        }
        if (nds_instance_child_count(child)) break_map_blocks(child, pulse_count);
    }
}

static int player_in_shelter(nds_instance* node, nds_vec3 player_position)
{
    size_t i;
    if (!node) return 0;
    for (i = 0; i < nds_instance_child_count(node); ++i) {
        nds_instance* child = (nds_instance*)nds_instance_child_at(node, i);
        nds_part_properties p;
        const char* name;
        if (!child) continue;
        name = nds_instance_get_name(child);
        if (name && strcmp(name, "Shelter") == 0 &&
            nds_part_get_properties(child, &p) == NDS_OK && p.visible) {
            float radians = -p.rotation.y * 0.0174532925199433f;
            float dx = player_position.x - p.position.x;
            float dz = player_position.z - p.position.z;
            float local_x = dx * cosf(radians) - dz * sinf(radians);
            float local_z = dx * sinf(radians) + dz * cosf(radians);
            float half_x = p.size.x * 0.5f;
            float half_y = p.size.y * 0.5f;
            float half_z = p.size.z * 0.5f;
            if (fabsf(local_x) <= half_x && fabsf(player_position.y - p.position.y) <= half_y + 0.75f &&
                fabsf(local_z) <= half_z)
                return 1;
        }
        if (player_in_shelter(child, player_position)) return 1;
    }
    return 0;
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

void nds_earthquake_stop(nds_earthquake* earthquake, nds_instance* scene)
{
    if (!earthquake) return;
    if (earthquake->active && scene)
        move_parts(scene, -earthquake->previous_shake_x, -earthquake->previous_shake_z);
    earthquake->previous_shake_x = 0.0f;
    earthquake->previous_shake_z = 0.0f;
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

    shake_x = sinf(earthquake->elapsed * 18.0f) * 0.16f;
    shake_z = cosf(earthquake->elapsed * 15.0f) * 0.12f;
    move_parts(scene, shake_x - earthquake->previous_shake_x,
               shake_z - earthquake->previous_shake_z);
    earthquake->previous_shake_x = shake_x;
    earthquake->previous_shake_z = shake_z;

    direction = sinf(earthquake->elapsed * 18.0f);
    player->velocity.x += direction * 1.8f * dt;
    player->velocity.z += cosf(earthquake->elapsed * 15.0f) * 1.5f * dt;

    if (earthquake->pulse_timer <= 0.0f) {
        earthquake->pulse_timer += earthquake->pulse_interval;
        earthquake->pulse_count++;
        break_map_blocks(scene, earthquake->pulse_count);
        direction = (earthquake->pulse_count & 1u) ? 1.0f : -1.0f;
        player->velocity.x += direction * 5.0f;
        player->velocity.z += (earthquake->pulse_count % 3u == 0u) ? 4.0f : -3.0f;
        if (!player_in_shelter(scene, player->position))
            nds_player_damage(player, player->grounded ? 10.0f : 25.0f);
    }
}
