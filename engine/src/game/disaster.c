#include "engine/game/disaster.h"

#include <math.h>

void nds_earthquake_init(nds_earthquake* earthquake)
{
    if (!earthquake) return;
    earthquake->elapsed = 0.0f;
    earthquake->pulse_timer = 0.0f;
    earthquake->pulse_interval = 4.0f;
    earthquake->pulse_count = 0;
    earthquake->active = 0;
}

void nds_earthquake_start(nds_earthquake* earthquake)
{
    if (!earthquake) return;
    earthquake->elapsed = 0.0f;
    earthquake->pulse_timer = earthquake->pulse_interval;
    earthquake->pulse_count = 0;
    earthquake->active = 1;
}

void nds_earthquake_stop(nds_earthquake* earthquake)
{
    if (!earthquake) return;
    earthquake->active = 0;
}

void nds_earthquake_update(nds_earthquake* earthquake, nds_player_controller* player, float dt)
{
    float direction;
    if (!earthquake || !player || !earthquake->active || !player->alive) return;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.1f) dt = 0.1f;

    earthquake->elapsed += dt;
    earthquake->pulse_timer -= dt;

    /* Continuous low-amplitude shaking. Keep this cheap enough for GLES2-era hardware. */
    direction = sinf(earthquake->elapsed * 18.0f);
    player->velocity.x += direction * 1.2f * dt;
    player->velocity.z += cosf(earthquake->elapsed * 15.0f) * 1.0f * dt;

    if (earthquake->pulse_timer <= 0.0f) {
        earthquake->pulse_timer += earthquake->pulse_interval;
        earthquake->pulse_count++;

        /* A pulse can throw an airborne player around. Staying grounded is the safe strategy. */
        direction = (earthquake->pulse_count & 1u) ? 1.0f : -1.0f;
        player->velocity.x += direction * 5.0f;
        player->velocity.z += (earthquake->pulse_count % 3u == 0u) ? 4.0f : -3.0f;
        if (!player->grounded) {
            nds_player_damage(player, 25.0f);
        }
    }
}
