#include "engine/game/disaster_system.h"
#include "engine/scene/part.h"
#include <math.h>

static void windstorm_visit(nds_windstorm* storm, nds_player_controller* player,
                            nds_instance* node, float dt, float gust)
{
    size_t i;
    if (!node) return;
    if (nds_instance_get_class(node) == NDS_CLASS_PART) {
        nds_part_properties p;
        if (nds_part_get_properties(node, &p) == NDS_OK && !p.anchored && p.visible) {
            float phase = p.position.x * 0.11f + p.position.z * 0.07f + storm->elapsed * 1.7f;
            float lateral = cosf(phase) * gust;
            float crosswind = sinf(phase * 0.83f) * gust;
            p.position.x += lateral * dt;
            p.position.z += crosswind * dt;
            /* Loose debris tumbles as the wind catches it. Keep the rotation bounded
             * so repeated frames do not eventually overflow the transform. */
            p.rotation.x += crosswind * 0.018f * dt;
            p.rotation.y += lateral * 0.012f * dt;
            p.rotation.z += gust * 0.010f * sinf(phase * 1.31f) * dt;
            if (p.rotation.x > 360.0f || p.rotation.x < -360.0f) p.rotation.x = fmodf(p.rotation.x, 360.0f);
            if (p.rotation.y > 360.0f || p.rotation.y < -360.0f) p.rotation.y = fmodf(p.rotation.y, 360.0f);
            if (p.rotation.z > 360.0f || p.rotation.z < -360.0f) p.rotation.z = fmodf(p.rotation.z, 360.0f);
            nds_part_set_properties(node, &p);
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
    gust = storm->strength * (0.55f + 0.45f * sinf(storm->elapsed * 2.1f));
    windstorm_visit(storm, player, scene, dt, gust);
    if (storm->gust_timer <= 0.0f) {
        storm->gust_count++;
        storm->gust_timer += storm->gust_interval;
        if (storm->gust_timer <= 0.0f) storm->gust_timer = storm->gust_interval;
        if (player && player->alive) {
            player->velocity.x += gust * 0.35f;
            player->velocity.z += gust * 0.20f;
            if (!player->grounded && gust > storm->strength * 0.9f)
                nds_player_damage(player, 8.0f);
        }
        /* A strong gust gives loose debris a brief lift. The actual scene physics
         * still owns player movement, while this keeps environmental debris lively. */
        windstorm_visit(storm, player, scene, 0.045f, gust * 0.35f);
    }
}

static void start_active_disaster(nds_disaster_system* system)
{
    if (!system) return;
    switch (system->active_type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_start(&system->earthquake);
        break;
    case NDS_DISASTER_WINDSTORM:
        windstorm_start(&system->windstorm);
        break;
    default:
        system->active = 0;
        break;
    }
}

void nds_disaster_system_init(nds_disaster_system* system)
{
    if (!system) return;
    nds_earthquake_init(&system->earthquake);
    windstorm_init(&system->windstorm);
    system->active_type = NDS_DISASTER_EARTHQUAKE;
    system->warning_duration = 3.0f;
    system->warning_remaining = 0.0f;
    system->active = 0;
}

void nds_disaster_system_start(nds_disaster_system* system, nds_disaster_type type)
{
    if (!system) return;
    system->active_type = type;
    system->warning_remaining = system->warning_duration;
    system->active = 1;
    switch (type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_stop(&system->earthquake, NULL);
        break;
    case NDS_DISASTER_WINDSTORM:
        windstorm_stop(&system->windstorm);
        break;
    default:
        system->active = 0;
        break;
    }
}

void nds_disaster_system_update(nds_disaster_system* system,
                                nds_player_controller* player,
                                nds_instance* scene, float dt)
{
    float active_dt;
    if (!system || !system->active || dt <= 0.0f) return;

    active_dt = dt;
    if (system->warning_remaining > 0.0f) {
        float warning_step = dt;
        if (warning_step > system->warning_remaining)
            warning_step = system->warning_remaining;
        system->warning_remaining -= warning_step;
        active_dt -= warning_step;
        if (system->warning_remaining > 0.0f || active_dt <= 0.0f)
            return;
        system->warning_remaining = 0.0f;
        start_active_disaster(system);
        if (!system->active) return;
    }

    switch (system->active_type) {
    case NDS_DISASTER_EARTHQUAKE:
        nds_earthquake_update(&system->earthquake, player, scene, active_dt);
        break;
    case NDS_DISASTER_WINDSTORM:
        windstorm_update(&system->windstorm, player, scene, active_dt);
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
    system->warning_remaining = 0.0f;
    system->active = 0;
}

float nds_disaster_warning_remaining(const nds_disaster_system* system)
{
    if (!system || !system->active || system->warning_remaining <= 0.0f) return 0.0f;
    return system->warning_remaining;
}

int nds_disaster_is_warning(const nds_disaster_system* system)
{
    return system && system->active && system->warning_remaining > 0.0f;
}
