/*
 * The continue screen of arcade mode (original module 0x40EBC0-0x40F53F,
 * strings "Continue screen init" to "Entering continue screen.").
 *
 * The human team's character stays on the stage in state 5500 (with its
 * partner, 5500 too, moved aside) while the screen asks "Continue?"; the
 * answer goes to GameVars.continued.
 */
#include "mugen.h"

/* The state of the continue screen (ScreenLoop.state, calloc'ed by Continue_Setup). */
typedef struct ContinueState {
    Player *p;                      /* 0x00 the human team's character */
    Player *partner;                /* 0x04 its partner (simul), or NULL */
    int choice;                     /* 0x08 1 yes, 0 no */
} ContinueState;
MUGEN_ASSERT_SIZE(ContinueState, 0x0C);

/*
 * 0x0040EBC0: the human team's players go to state 5500 at the camera
 * position (the partner 50 game units in front, facing back), the others
 * leave the match. Returns 1.
 */
int Continue_Init(ScreenLoop *screen, void *state_)
{
    ContinueState *state = state_;
    GameVars *g;
    Player *o;

    state->p = NULL;
    state->partner = NULL;
    state->choice = 0;
    g = g_game;
    if (g->arcade_side == 2) {
        state->p = g->players[2];
        state->partner = g->players[4];
        if (g->players[1] != NULL && g->players[1]->active) {
            Players_SetActive(g->players[1]->index, 0);
            g = g_game;
        }
        o = g->players[3];
    } else {
        state->p = g->players[1];
        state->partner = g->players[3];
        if (g->players[2] != NULL && g->players[2]->active) {
            Players_SetActive(g->players[2]->index, 0);
            g = g_game;
        }
        o = g->players[4];
    }
    if (o != NULL && o->active) {
        Players_SetActive(o->index, 0);
    }
    Player_Reset(state->p);
    Player_ChangeState(5500, state->p);
    state->p->pos.x = g_game->camera_offset_x;
    if (state->partner != NULL) {
        float w;
        Player_Reset(state->partner);
        Player_ChangeState(5500, state->partner);
        g = g_game;
        w = (float)((double)Ftol(g->game_coord[0]) * 50.0 / 320.0);
        state->partner->pos.x = (double)w * state->p->facing + g->camera_offset_x;
        if (state->partner->facing == state->p->facing) {
            state->partner->facing = -state->partner->facing;
            Player_UpdateInputFacing(state->partner);
        }
        w = (float)((double)Ftol(g->game_coord[0]) * 50.0 / 320.0);
        state->p->pos.x = state->p->pos.x - (double)w * state->p->facing;
    }
    state->choice = 1;
    Log_Printf("Continue screen init\n");
    return 1;
}

/* 0x0040EDF0: the answer goes to GameVars.continued. Returns 1. */
int Continue_End(ScreenLoop *screen, void *state_)
{
    ContinueState *state = state_;

    Log_Printf("End of continue screen loop\n");
    g_game->continued = state->choice;
    state->p = NULL;
    state->partner = NULL;
    state->choice = 0;
    return 1;
}

/*
 * 0x0040EE20: the human side (or anyone when player_sides[0] == player_sides[1]) moves the
 * choice with left / right and confirms with a button; escape answers no.
 * The answer starts the fade out. While fading out, escape or a quit request
 * ends it at once.
 */
int Continue_Update(ScreenLoop *screen, void *state_)
{
    ContinueState *state = state_;
    GameVars *g;
    int pressed = 0;

    if (screen->fade_out != 0) {
        if (screen->fade_ticks <= 0) {
            return 0;
        }
        if (screen->key == 27 || g_quitRequested) {
            screen->fade_ticks = 0;
            screen->fade_out = 1;
        }
        return 1;
    }
    g = g_game;
    if (screen->input_side + 1 == g->arcade_side || g->player_sides[0] == g->player_sides[1]) {
        if (screen->input & SCREEN_IN_LEFT) {
            state->choice = 1;
        }
        if (screen->input & SCREEN_IN_RIGHT) {
            state->choice = 0;
        }
        if (screen->input & 0xFF0) {
            pressed = 1;
        }
    }
    if (screen->done != 0) {
        state->choice = 0;
    } else if (!pressed) {
        return 0;
    }
    screen->fade_out = 1;
    screen->fade_ticks = screen->fadeout_time;
    g->fade.type = FADE_PALETTE;
    g->fade.ticks = 0;
    g->fade.time = screen->fadeout_time;
    g->fade.out = 1;
    return 0;
}

/*
 * 0x0040EEE0: halves the colour factors of the palette groups of the first
 * mask word (the players) every tick. Returns 1.
 */
int Continue_Tick(ScreenLoop *screen, void *state)
{
    PalGroups *pg = g_game->palgroups;
    static const unsigned int select[5] = { 0xFFFFFFFFu, 0, 0, 0, 0 };
    int i, k;

    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        unsigned int mask[5];
        for (k = 0; k < 5; k++) {
            mask[k] = e->mask[k];
        }
        for (k = 0; k < 5; k++) {
            if (select[k] & mask[k]) {
                PalFx *fx = e->fx;
                fx->mul[0] = (int)((unsigned int)fx->mul[0] << 7) >> 8;
                fx->mul[1] = (int)((unsigned int)fx->mul[1] << 7) >> 8;
                fx->mul[2] = (int)((unsigned int)fx->mul[2] << 7) >> 8;
                fx->serial = g_palFxSerial++;
                break;
            }
        }
    }
    return 1;
}

/* A [Continue Screen] element of layer 0, drawn with the fight.def sprites, animations and fonts. */
static void Continue_DrawElem(const SysElem *elem, float x, float y)
{
    GameVars *g;
    int idx;

    if (elem->layerno != 0) {
        return;
    }
    g = g_game;
    idx = SysElem_Add(elem, g->sysexplods, g->fight.sff, g->fight.anim, (void **)g->fight.fonts, x, y);
    if (idx >= 0) {
        float s = (float)g_game->motif_scale;
        SysElem_SetScale(idx, s, 1.0f);
        SysDraw_DrawItem(g_game->sysexplods, idx);
    }
}

/*
 * 0x0040EFB0: draws "Continue?" and the yes / no choice with the debug font
 * when the motif has no elements for them, and the motif's elements.
 * Returns 1.
 */
int Continue_Draw(ScreenLoop *screen, void *state_)
{
    ContinueState *state = state_;
    ContinueInfo *ci = &g_game->continue_screen;
    int x, y;
    int yes_r = 0x100, yes_g = 0x100, yes_b = 0x100;
    int no_r = 0x100, no_g = 0x100, no_b = 0x100;

    if (g_game->skip_draw != 0) {
        return 1;
    }
    x = ci->pos[0];
    y = ci->pos[1];
    if (state->choice != 0) {
        no_r = no_g = no_b = 0x80;
    } else {
        yes_r = yes_g = yes_b = 0x80;
    }
    if (ci->continue_text.type == SYSELEM_NONE) {
        Font_DrawText("Continue?", g_game->font_medium, x + 1, y + 1, 0, 0, 0, 0, 0);
        Font_DrawText("Continue?", g_game->font_medium, x, y, 0, 0, 0x100, 0x100, 0x100);
    }
    if (ci->yes.type == SYSELEM_NONE) {
        Font_DrawText("Yes", g_game->font_medium, x - 9, y + 21, -1, 0, 0, 0, 0);
        Font_DrawText("Yes", g_game->font_medium, x - 10, y + 20, -1, 0, yes_r, yes_g, yes_b);
        Font_DrawText("No", g_game->font_medium, x + 11, y + 21, 1, 0, 0, 0, 0);
        Font_DrawText("No", g_game->font_medium, x + 10, y + 20, 1, 0, no_r, no_g, no_b);
    }
    Continue_DrawElem(&ci->continue_text, (float)x, (float)y);
    if (state->choice != 0) {
        Continue_DrawElem(&ci->yes_active, (float)x, (float)y);
        Continue_DrawElem(&ci->no, (float)x, (float)y);
    } else {
        Continue_DrawElem(&ci->yes, (float)x, (float)y);
        Continue_DrawElem(&ci->no_active, (float)x, (float)y);
    }
    return 1;
}

/* 0x0040F400: the screen loop of the continue screen; 0 when its state cannot be allocated. */
int Continue_Setup(ScreenLoop *screen)
{
    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->kind = 8;
    screen->fps = 60;
    screen->reload_motif = 1;
    screen->start_frame = g_game->frames;
    screen->state = calloc(1, sizeof(ContinueState));
    if (screen->state == NULL) {
        return 0;
    }
    screen->init = Continue_Init;
    screen->end = Continue_End;
    screen->update = Continue_Update;
    screen->tick = Continue_Tick;
    screen->draw = Continue_Draw;
    screen->overlay = Screen_TickNop;
    screen->loads_match = 1;
    screen->reload_motif = 1;
    return 1;
}

/*
 * 0x0040F4B0: runs the continue screen; returns the next game flow step:
 * 14 when the player gave up, else 7 or 10 (GameVars.continue_reselect).
 */
int Continue_Run(void)
{
    ScreenLoop screen;
    GameVars *g;

    Log_Printf("Entering continue screen.\n");
    Continue_Setup(&screen);
    Screen_Run(&screen);
    screen.result = screen.done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    memset(&screen, 0, sizeof(screen));
    g = g_game;
    if (g->continued != 0) {
        return g->continue_reselect != 0 ? 7 : 10;
    }
    return 14;
}
