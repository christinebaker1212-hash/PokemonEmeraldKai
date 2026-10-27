/*
 * Debug commands shared by the console and the Lua API (original module
 * 0x421A40-0x421E7F, strings "Life power and time reset" to "Setting player
 * %d to state %d"). The original also has "Max power mode set to %i",
 * "Requesting round reset" and "Requesting match reload" here; their
 * functions were inlined into the console and the Lua API.
 */
#include "mugen.h"

/*
 * 0x00421A40: every character gets full life and power (a lying-down player
 * in state 5150 gets up, to 5120), and the round timer starts again.
 */
void Debug_ResetLifePowerTime(void)
{
    GameVars *g = g_game;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];

        if (p == NULL || !p->active || p->helper != 0) {
            continue;
        }
        if (p->state_no == 5150) {
            StateDefs *states = p->custom_states != NULL ? p->custom_states : p->states;
            KeyedArray *list = states->list;
            int index = KeyedArray_Find(list, 5120, 0);
            int exists = index != -1 && list->data + list->elem_size * index != NULL;
            int prev = p->state_no;

            p->prev_state_no = prev;
            p->state_no = 5120;
            p->state_time = 0;
            if (!exists) {
                Player_Warnf(p, "changed to invalid state %i (from state %i)\n", 5120, prev);
                g = g_game;
            }
        }
        p = g->players[i];
        p->life = p->life_max;
        g->players[i]->alive = 1;
        g->players[i]->visible = 1;
        g->players[i]->gethit_yveladd = 0;
        g->players[i]->gethit_xveladd = 0;
        p = g->players[i];
        *p->power_ref = *p->power_max_ref;
    }
    g->round_time = Round_GetStartTime();
    Log_Printf("Life power and time reset\n");
}

/*
 * 0x00421BD0: the round time in ticks: ([Options] Time + 1) * [Time]
 * framespercount - 1, or -1 (no time limit) in training, the survival modes
 * and demo mode.
 */
int Round_GetStartTime(void)
{
    GameVars *g = g_game;
    int t = (g->time + 1) * g->fight.time.frames_per_count - 1;

    if (g->team_variant == 3 || g->team_variant == 1 || g->demo_mode != 0) {
        t = -1;
    }
    return t;
}

/* 0x00421C10: empties the debug clipboard and the warnings of the players in the match. */
void Debug_ClearClipboards(void)
{
    GameVars *g = g_game;
    TextRing *r = g->clipboard;
    int i;

    r->cur = 0;
    r->head = 0;
    r->lines[0][0] = '\0';
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && p->warnings != NULL) {
            r = p->warnings;
            r->cur = 0;
            r->head = 0;
            r->lines[0][0] = '\0';
        }
    }
}

/*
 * 0x00421C80: quick VS only: training mode on or off (team variant 3),
 * player 2 under human control, no round time; the training menu shows in
 * a fight.
 */
void Debug_ToggleTrainingMode(void)
{
    GameVars *g = g_game;
    Player *p;

    if (g->quick_vs == 0) {
        return;
    }
    g->game_mode_side = 0;
    p = g->players[2];
    if (p != NULL && p->active) {
        p->ai = 0;
    }
    g->team_variant = g->team_variant != 3 ? 3 : 0;
    g->round_time = -1;
    g->training_value = 0;
    g->training_cursor = 0;
    g->training_menu = g->team_variant == 3 && g->screen_kind == 4;
}

/* 0x00421D00: puts player n in or out of the match. */
void Debug_TogglePlayer(int n)
{
    GameVars *g = g_game;
    Player *p;

    if (n < 0 || n > g->player_slots || (p = g->players[n]) == NULL) {
        Log_Printf("Player %d requested\n", n);
        return;
    }
    Players_SetActive(p->index, !p->active);
    Log_Printf("Player %d %s\n", n, p->active ? "enabled" : "disabled");
}

/* 0x00421D80: turns the AI of player n (in the match) on or off. */
void Debug_ToggleAi(int n)
{
    GameVars *g = g_game;
    Player *p;

    if (n < g->players_first || n > g->players_last) {
        return;
    }
    p = g->players[n];
    if (p == NULL || !p->active) {
        return;
    }
    p->ai ^= 1;
    Log_Printf("Player %d AI turned %s\n", n, p->ai ? "on" : "off");
}

/*
 * 0x00421DD0: puts player n (a character in the match) in state `state`, with
 * control and no velocity, using its own states.
 */
void Debug_SetPlayerState(int n, int state)
{
    GameVars *g = g_game;
    Player *p;

    if (n < g->players_first || n > g->players_last) {
        return;
    }
    p = g->players[n];
    if (p == NULL || !p->active || p->helper != 0) {
        return;
    }
    Log_Printf("Setting player %d to state %d", n, state);
    Player_ClearCustomState(p);
    Player_ChangeState(state, p);
    p->ctrl = 1;
    /* the original copies vel.pad_24 from its uninitialised stack */
    p->vel.x = 0.0;
    p->vel.y = 0.0;
    p->vel.z = 0.0;
    p->vel.unit = 1.0;
    p->vel.type = COORD_TYPE_2D;
    p->vel.pad_24 = 0;
}
