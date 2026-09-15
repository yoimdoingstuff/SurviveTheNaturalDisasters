#include "engine/game/disaster_system.h"
#include "engine/scene/part.h"
#include <math.h>

static void windstorm_visit(nds_windstorm* storm, nds_player_controller* player,
                            nds_instance* node, float dt, float gust)
{
    size_t i;
    if (!node) return;

    if (nds_instance_get_class(node) == NDS_CLASS_PART ||
        nds_instance_get_class(node) == NDS_CLASS_SPAWN_POINT) {
        nds_part_properties p;
        if (nds_part_get_properties(node, &p) == NDS_OK && !p.anchored && p.visible) {
            /* Apply a horizontal wind displacement. The physics system will
             * subsequently integrate gravity and resolve contacts, while the
             * direct gust gives legacy/mobile scenes a cheap environmental
             * force without allocating temporary force objects. */
            float phase = p.position.x * 0.11f + p.position.z * 0.07f + storm->elapsed * 1.7f;
            float wx = cosf(phase) * gust * dt;
            float wz = sinf(phase * 0.83f) * gust * dt;
            p.position.x += wx;
            p.position.z += wz;
            nds_part_set_position(node, p.position);
        }
    }

    for (i = 0; i < nds_instance_child_count(node); ++i)
        windstorm_visit(storm, player, nds_instance_child_at(node, i), dt, gust);

    (void)player;
}

static void windstorm_init(nds_windstorm* storm)
{
    storm->elapsed = 0.0f;
    storm->gust_timer = 0.0f;
    storm->gust_interval = 2.5f;
    storm->strength = 18.0f;
    storm->gust_count = 0;
    storm->active = 0;
}

static void windstorm_start(nds_windstorm* storm)
{
    if (!storm) return;
    storm->elapsed = 0.0f;
    storm->gust_timer = storm->gust_interval;
    storm->gust_count = 0;
    storm->active = 1;
}

static void windstorm_stop(nds_windstorm* storm)
{
    if (!storm) return;
    storm->active = 0;
}

static void windstorm_update(nds_windstorm* storm, nds_player_controller* player,
                             nds_instance* scene, float dt)
{
    float gust;
    if (!storm || !storm->active || !scene || dt <= 0.0f) return;
    if (dt > 0.1f) dt = 0.1f;
    storm->elapsed += dt;
    storm->gust_timer -= dt;

    /* Wind strength rises and falls continuously rather than teleporting
     * between two speeds. Gusts are still counted for deterministic tests and
     * future network replication. */
    gust = storm->strength * (0.55f + 0.45f * sinf(storm->elapsed * 2.1f));
    windstorm_visit(storm, player, scene, dt, gust);

    if (storm->gust_timer <= 0.0f) {
        storm->gust_count++;
        storm->gust_timer += storm->gust_interval;
        if (storm->gust_timer <= 0.0f) storm->gust_timer = storm->gust_interval;

        /* A strong gust can knock a player out of a precarious position. */
        if (player && player->alive) {
            player->velocity.x += gust * 0.35f;
            player->velocity.z += gust * 0.20f;
            if (!player->grounded && gust > storm->strength * 0.9f)
                nds_player_damage(player, 8.0f);
        }
    }
}

void nds_disaster_system_init(nds_disaster_system* system)
{
    if (!system) return;
    nds_earthquake_init(&system->earthquake);
    windstorm_init(&system->windstorm);
    system->active_type = NDS_DISASTER_EARTHQUAKE;
    system->active = 0;
}

void nds_disaster_system_start(nds_disaster_system* system, nds_disaster_type type)
{
    if (!system) return;
    system->active_type = type;
    system->active = 0;

    switch (type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_start(&system->earthquake);
        system->active = 1;
        break;
    case NDS_DISASTER_WINDSTORM:
        windstorm_start(&system->windstorm);
        system->active = 1;
        break;
    default:
        break;
    }
}

void nds_disaster_system_update(nds_disaster_system* system,
                                nds_player_controller* player,
                                nds_instance* scene, float dt)
{
    if (!system || !system->active) return;

    switch (system->active_type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_update(&system->earthquake, player, scene, dt);
        break;
    case NDS_DISASTER_WINDSTORM:
        windstorm_update(&system->windstorm, player, scene, dt);
        break;
    default:
        system->active = 0;
        break;
    }
}

void nds_disaster_system_stop(nds_disaster_system* system, nds_instance* scene)
{
    if (!system || !system->active) return;

    switch (system->active_type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_stop(&system->earthquake, scene);
        break;
    case NDS_DISASTER_WINDSTORM:
        windstorm_stop(&system->windstorm);
        break;
    default:
        break;
    }

    system->active = 0;
}
