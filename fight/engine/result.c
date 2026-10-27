/*
 * Survival result screen (original module 0x444450-0x444A9F; string literals
 * "Result screen init" to "Entering result screen.").
 *
 * After a survival run the human team's players are put back on the stage in
 * a win (180) or lose (170) state, and the motif's [Survival Results Screen]
 * winstext is drawn with the number of matches won, until show.time or a
 * button press starts the fade out. The other team's players are
 * deactivated and every palette group is darkened (mul halved) each tick.
 *
 * Quirk kept from the original: the second player is placed relative to the
 * first one's facing even when there is no first player.
 */
#include "mugen.h"


/* x offset of a player from the centre: 50 in 320-wide units. */
static float result_offset(void)
{
    return (float)((int)g_game->game_coord[0] * 50.0 / 320.0);
}

/* 0x00444450 */
int Result_Init(ScreenLoop *screen, ResultState *st)
{
    GameVars *g = g_game;
    Player *other;
    int state;

    st->p1 = NULL;
    st->p2 = NULL;
    st->wins = 0;
    if (g->survival_side != 0) {
        /* the match wins of a side, 12 bytes apart from GameVars 0x8710 */
        st->wins = (&g->continued)[g->survival_side * 3];
    }
    if (g->survival_side == 2) {
        st->p1 = g->players[2];
        st->p2 = g->players[4];
        if (g->players[1] != NULL && g->players[1]->active) {
            Players_SetActive(g->players[1]->index, 0);
        }
        other = g->players[3];
    } else {
        st->p1 = g->players[1];
        st->p2 = g->players[3];
        if (g->players[2] != NULL && g->players[2]->active) {
            Players_SetActive(g->players[2]->index, 0);
        }
        other = g->players[4];
    }
    if (other != NULL && other->active) {
        Players_SetActive(other->index, 0);
    }
    state = st->wins < g->survival_results.roundstowin ? RESULT_STATE_LOSE : RESULT_STATE_WIN;
    if (st->p1 != NULL) {
        Player *p = st->p1;
        Players_SetActive(p->index, 1);
        Player_Reset(p);
        Player_ChangeState(state, p);
        p->pos.x = g_game->camera_offset_x;
        if (st->p2 != NULL) {
            float off = result_offset();
            p->pos.x = p->pos.x - off * p->facing;
        }
        p->no_keyctrl = 1;
    }
    if (st->p2 != NULL) {
        Player *p = st->p2;
        float off;
        Players_SetActive(p->index, 1);
        Player_Reset(p);
        Player_ChangeState(state, p);
        off = result_offset();
        p->pos.x = off * st->p1->facing + g_game->camera_offset_x;
        if (p->facing == st->p1->facing) {
            p->facing = -p->facing;
            Player_UpdateInputFacing(p);
        }
        p->no_keyctrl = 1;
    }
    Log_Printf("Result screen init\n");
    return 1;
}

/* 0x00444710 */
int Result_End(ScreenLoop *screen, ResultState *st)
{
    Log_Printf("End of result screen loop\n");
    st->p1 = NULL;
    st->p2 = NULL;
    st->wins = 0;
    return 1;
}

/* Starts the fade out of the screen. */
static void result_fade_out(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int time = screen->fadeout_time;

    screen->fade_out = 1;
    screen->fade_ticks = time;
    g->fade.type = 1;
    g->fade.ticks = 0;
    g->fade.time = time;
    g->fade.out = 1;
}

/* 0x00444740: a button (0xFF0: the buttons and one more bit) of the human side, or of either side when both are human, ends the screen. */
int Result_Update(ScreenLoop *screen, ResultState *st)
{
    GameVars *g;
    int pressed = 0;

    if (screen->fade_out) {
        if (screen->fade_ticks > 0) {
            if (screen->key == 27 || g_quitRequested) {
                screen->fade_ticks = 0;
                screen->fade_out = 1;
            }
            return 1;
        }
        return 0;
    }
    g = g_game;
    if ((screen->input_side + 1 == g->survival_side || g->player_sides[0] == g->player_sides[1]) &&
        (screen->input & 0xFF0)) {
        pressed = 1;
    }
    if (screen->done || pressed) {
        result_fade_out(screen);
    }
    return 0;
}

/* 0x004447E0: fades out after show.time; darkens every palette group. */
int Result_Tick(ScreenLoop *screen, ResultState *st)
{
    if (!screen->fade_out && screen->ticks == g_game->survival_results.show_time) {
        result_fade_out(screen);
    }
    PalGroups_MulColor(0xFFFFFFFFu, 0, 0, 0, 0, 0x80, 0x80, 0x80);
    return 1;
}

/* 0x004448F0: winstext with the number of wins put in its text (as a printf format) for one frame. */
int Result_Draw(ScreenLoop *screen, ResultState *st)
{
    GameVars *g = g_game;
    SysElem *text = &g->survival_results.winstext;
    char format[0x100];

    if (!g->skip_draw) {
        format[0] = '\0';
        strncat(format, text->text, sizeof(format) - 1);
        _snprintf(text->text, sizeof(text->text), format, st->wins);
        SysElem_AddMotif(text, 0.0f, 0.0f);
        text->text[0] = '\0';
        strncat(text->text, format, sizeof(text->text) - 1);
    }
    return 1;
}

/* 0x00444980: the result screen loop; returns 0 when the state cannot be allocated. */
int Result_Setup(ScreenLoop *screen)
{
    GameVars *g = g_game;

    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->kind = 7;
    screen->fps = 60;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->reload_motif = 1;
    screen->start_frame = g->frames;
    screen->state = calloc(1, sizeof(ResultState));
    if (screen->state == NULL) {
        return 0;
    }
    screen->init = (ScreenFn)Result_Init;
    screen->end = (ScreenFn)Result_End;
    screen->update = (ScreenFn)Result_Update;
    screen->tick = (ScreenFn)Result_Tick;
    screen->draw = (ScreenFn)Result_Draw;
    screen->overlay = Screen_TickNop;
    screen->loads_match = 1;
    screen->reload_motif = 1;
    screen->fadein_time = g->survival_results.fadein_time;
    screen->fadeout_time = g->survival_results.fadeout_time;
    return 1;
}

/* 0x00444A40: runs the result screen; the next gameflow state (6, or 4 when the loop fails). */
int Result_Run(void)
{
    ScreenLoop screen;
    int next = 6;

    Log_Printf("Entering result screen.\n");
    Result_Setup(&screen);      /* the result is not checked */
    if (!Screen_Run(&screen)) {
        next = 4;
    }
    screen.result = screen.done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    return next;
}
