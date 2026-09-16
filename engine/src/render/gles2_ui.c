#include "engine/render/gles2_ui.h"

#ifdef _WIN32
#include <GL/gl.h>
#endif
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
void nds_gles2_ui_draw(int width, int height, const nds_hud_state* state)
{
    (void)width; (void)height; (void)state;
}
#else
static const unsigned char font5x7[36][7] = {
    {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
    {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,14},{17,17,17,31,17,17,17},
    {14,4,4,4,4,4,14},{7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
    {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
    {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
    {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
    {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},
    {14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{30,1,1,14,1,1,30},{14,17,1,6,8,16,31},
    {14,17,1,6,1,17,14},{17,17,14,1,1,17,14},{14,17,3,13,17,17,14},{14,17,1,7,1,17,14}
};

static int glyph_index(char c)
{
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + c - '0';
    return -1;
}

static void rect(float x, float y, float w, float h, float r, float g, float b, float a)
{
    glColor4f(r,g,b,a);
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

static void text(float x, float y, float scale, const char* s, float r, float g, float b, float a)
{
    float start = x;
    glColor4f(r,g,b,a);
    while (s && *s) {
        char c = *s++;
        if (c == '\n') { y += 8.0f*scale; x = start; continue; }
        if (c == ' ') { x += 4.0f*scale; continue; }
        {
            int gi = glyph_index(c);
            int row;
            if (gi < 0) { x += 6.0f*scale; continue; }
            for (row=0; row<7; ++row) {
                int col;
                for (col=0; col<5; ++col) {
                    if (font5x7[gi][row] & (1u << (4-col)))
                        rect(x+col*scale, y+row*scale, scale, scale, r,g,b,a);
                }
            }
            x += 6.0f*scale;
        }
    }
}

static float clamp01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

void nds_gles2_ui_draw(int width, int height, const nds_hud_state* state)
{
    float w=(float)width, h=(float)height;
    float health=0.0f, phase=0.0f, warning=0.0f, remaining=0.0f;
    char line[96];
    const char* phase_name="INTERMISSION";
    const char* disaster_name="";
    int warning_active=0;

    if (width <= 0 || height <= 0 || !state || state->menu_active) return;
    if (state->player) health=clamp01(state->player->health/100.0f);
    if (state->round) {
        phase=clamp01(nds_round_phase_progress(state->round));
        remaining=nds_round_time_remaining(state->round);
        phase_name=nds_round_state_name(state->round->state);
        disaster_name=nds_disaster_type_name(state->round->disaster);
    }
    if (state->disasters) {
        warning_active=nds_disaster_is_warning(state->disasters);
        warning=clamp01(nds_disaster_warning_remaining(state->disasters)/3.0f);
        if (state->disasters->active) disaster_name=nds_disaster_type_name(state->disasters->active_type);
    }

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0,w,h,0,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    rect(0,0,w,5.0f,0.08f,0.55f,0.95f,0.9f);
    rect(18,18,230,64,0.015f,0.03f,0.06f,0.88f);
    rect(24,24,218,4,0.08f,0.55f,0.95f,0.95f);
    text(30,38,1.8f,"SURVIVAL",0.92f,0.97f,1.0f,1.0f);
    text(30,54,1.0f,"HEALTH",0.72f,0.80f,0.88f,1.0f);
    rect(96,54,132,10,0.08f,0.1f,0.13f,1.0f);
    rect(96,54,132*health,10,0.15f,0.82f,0.55f,1.0f);

    rect(w-250,18,232,64,0.015f,0.03f,0.06f,0.88f);
    text(w-236,32,1.0f,"ROUND",0.72f,0.80f,0.88f,1.0f);
    snprintf(line,sizeof(line),"%u",state->round ? state->round->round_number : 0u);
    text(w-236,46,1.8f,line,0.96f,0.98f,1.0f,1.0f);
    snprintf(line,sizeof(line),"%s  %02d",phase_name,(int)(remaining < 0 ? 0 : remaining + 0.5f));
    text(w-184,53,0.85f,line,0.74f,0.84f,0.92f,1.0f);

    if (warning_active || (state->disasters && state->disasters->active)) {
        rect(w*0.5f-210,20,420,76,0.17f,0.03f,0.03f,0.9f);
        rect(w*0.5f-204,26,408,5,1.0f,0.26f,0.18f,0.9f);
        text(w*0.5f-80,39,1.55f,warning_active ? "WARNING" : "DISASTER",1.0f,0.9f,0.72f,1.0f);
        text(w*0.5f-92,58,1.15f,disaster_name ? disaster_name : "UNKNOWN",1.0f,0.74f,0.62f,1.0f);
        if (warning_active) rect(w*0.5f-190,82,380*warning,4,1.0f,0.65f,0.2f,0.9f);
    }

    rect(18,h-78,410,56,0.015f,0.03f,0.06f,0.78f);
    text(30,h-62,0.85f,"WASD MOVE   RMB LOOK   SPACE JUMP",0.82f,0.89f,0.96f,1.0f);
    text(30,h-46,0.85f,"C CAMERA   Q/E ZOOM   M MENU",0.65f,0.75f,0.84f,1.0f);
    text(w-260,h-62,0.8f,"MAP STATUS",0.65f,0.75f,0.84f,1.0f);
    rect(w-260,h-48,214,6,0.08f,0.1f,0.13f,1.0f);
    rect(w-260,h-48,214*phase,0.15f,0.08f,0.55f,0.95f,1.0f);
    text(w-260,h-36,0.78f,phase_name,0.9f,0.95f,1.0f,1.0f);

    glColor4f(0.9f,0.96f,1.0f,0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(w*0.5f-8,h*0.5f); glVertex2f(w*0.5f+8,h*0.5f);
    glVertex2f(w*0.5f,h*0.5f-8); glVertex2f(w*0.5f,h*0.5f+8);
    glEnd();

    if (state->disasters) {
        float intensity=clamp01(state->disasters->environment.debris_intensity);
        rect(w-250,100,232,42,0.015f,0.03f,0.06f,0.70f);
        text(w-236,111,0.75f,"DISASTER INTENSITY",0.68f,0.78f,0.88f,1.0f);
        rect(w-236,127,204,7,0.08f,0.1f,0.13f,1.0f);
        rect(w-236,127,204*intensity,7,0.95f,0.45f,0.18f,1.0f);
    }

    if (!state->player || !state->player->alive ||
        (state->round && state->round->state == NDS_ROUND_RESULTS)) {
        rect(w*0.5f-190,h*0.5f-58,380,116,0.015f,0.025f,0.05f,0.94f);
        rect(w*0.5f-184,h*0.5f-52,368,4,0.95f,0.35f,0.18f,0.95f);
        text(w*0.5f-105,h*0.5f-28,1.6f,
             (state->round && state->round->player_survived) ? "SURVIVED" : "ELIMINATED",
             1.0f,0.92f,0.74f,1.0f);
        snprintf(line,sizeof(line),"%s",state->round ? nds_round_result_name(state->round) : "ROUND OVER");
        text(w*0.5f-110,h*0.5f+2,1.0f,line,0.78f,0.86f,0.94f,1.0f);
        text(w*0.5f-124,h*0.5f+26,0.8f,"NEXT ROUND STARTS SOON",0.58f,0.68f,0.78f,1.0f);
    }

    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glPopAttrib();
}
#endif
