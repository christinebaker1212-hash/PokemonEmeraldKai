/*
 * Rounds of a match (original module 0x4124A0-0x419A4F, part 3): the tick
 * of a fight (Round_Tick), the round state machine (Round_RunState: 0 wait,
 * 1 intro, 2 fight, 3 over (KO / time over), 4 win poses), the round
 * announcements, combo counters, the life recovery of turns teams and the
 * win types of the rounds (the win icons).
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double and are rounded to float where the original stores a float.
 */
#include "mugen.h"

#define Rd_Sin(x) sin(x)

/* A state change to `state` as inlined by the original: warns when the player has no such state. */
static void Rd_ChangeState(Player *p, int state)
{
    StateDefs *states = p->custom_states != NULL ? p->custom_states : p->states;
    KeyedArray *list = states->list;
    int index = KeyedArray_Find(list, state, 0);
    int exists = index != -1 && list->data + list->elem_size * index != NULL;
    int prev = p->state_no;

    p->prev_state_no = prev;
    p->state_no = state;
    p->state_time = 0;
    if (!exists) {
        Player_Warnf(p, "changed to invalid state %i (from state %i)\n", state, prev);
    }
}

/* The player back to its own states and commands, in state 0 (inlined twice by Round_RunState). */
static void Rd_ResetToOwnStates(Player *p)
{
    if (p->cmds == p->custom_cmds) {
        memset(p->cmd_active, 0, sizeof(p->cmd_active));
        p->cmds = &p->data->cmds;
    }
    p->custom_states = NULL;
    p->custom_owner = -1;
    p->custom_anim = NULL;
    p->custom_cmds = NULL;
    p->prev_state_no = p->state_no;
    p->state_no = 0;
    p->state_time = 0;
}

/*
 * A fight.def sound at the [Sound] volume (Sound_PlaySystem with channel -1
 * and volume 100, inlined by Round_RunState).
 */
static void Rd_PlayFightSound(int group, int number)
{
    GameVars *g = g_game;
    double v = g->master_volume / 100.0 * g->wav_volume / 100.0 * 100.0;
    Snd *snd = g->fight.snd;
    int slot;

    if (v < 0.0) {
        v = 0.0;
    } else if (v > 100.0) {
        v = 100.0;
    }
    if (g->sound_enabled == 0 || snd == NULL || g->no_sounds != 0 || group == -1 || g_sndActive != 1) {
        return;
    }
    slot = Sound_FindFreeSlot();
    if (slot == -1) {
        return;
    }
    if (Sound_PlayOnSlot(slot, snd, group, number, v) != -1) {
        g_sndSlots[slot].owner = -1;
    }
}

/* The KO sound of the round result (win_type: 1 KO, 2 double KO, 3 time over). */
static void Rd_PlayKoSound(const FightRound *r)
{
    switch (g_game->win_type) {
    case 3:
        Rd_PlayFightSound(r->to_snd[0], r->to_snd[1]);
        break;
    case 2:
        Rd_PlayFightSound(r->dko_snd[0], r->dko_snd[1]);
        break;
    case 1:
        Rd_PlayFightSound(r->ko_snd[0], r->ko_snd[1]);
        break;
    default:
        break;
    }
}

/*
 * 0x00413F00: the round state machine, once a tick of a fight: the result
 * of the round (KO of a team, double KO, time over by life), a skipped
 * intro, then the state of the round (GameVars.round_state):
 *   0 waiting [Round] start.waittime (fade in);
 *   1 intro: the round sound, the round starts after intro.time;
 *   2 fight: the round timer, the "Fight" sound, control at ctrl.time, the
 *     round is over on a result (with slow motion after a KO);
 *   3 over: KO / time over sounds, slow motion steps, precaching of the
 *     next members, the win poses once everyone stands (or after 900
 *     ticks) and over.waittime has passed;
 *   4 win: the win poses (states 180 / 170 / 175) and turns life recovery
 *     at over.wintime, the fade out and the next round or the end of the
 *     match at over.time.
 * A state change runs the new state in the same tick. Returns 1 when the
 * round has ended (next round set up, or the match is over), else 0.
 *
 * Quirk kept: a team without members divides by zero.
 */
int Round_RunState(void)
{
    GameVars *g = g_game;
    const FightRound *r = &g->fight.round;
    int ko[3] = { -1, -1, -1 };     /* by team side: 1 all KO, 0 someone standing, -1 nobody */
    int life[3] = { 0, 0, 0 };      /* by team side: mean life in 1/1000, plus 1 per living member */
    int t, i, k;

    for (t = 0; t < 2; t++) {
        const TeamMember *m = &g->match_members[t];
        if (m->count > 0) {
            int sum = life[t + 1];
            for (k = 0; k < m->count; k++) {
                Player *p = g->players[m->slots[k].player];
                sum += p->life * 1000 / p->life_max + (p->alive != 0);
            }
            life[t + 1] = sum;
        }
        life[t + 1] /= m->count;
    }
    for (t = 0; t < 2; t++) {
        const TeamMember *m = &g->match_members[t];
        for (k = 0; k < m->count; k++) {
            Player *p = g->players[m->slots[k].player];
            int *side;
            if (p == NULL || !p->active || p->helper != 0) {
                continue;
            }
            side = &ko[p->team_side];
            if (*side == -1) {
                *side = 1;
            }
            /* the computer partner of a human simul player does not count, unless Team.LoseOnKO */
            if (m->mode == TEAM_SIMUL && k >= 1 && g->team_lose_on_ko == 0 && m->slots[k].ai != 0 &&
                m->slots[k - 1].ai == 0) {
                continue;
            }
            if (p->alive != 0) {
                *side = 0;
            }
        }
    }
    if (ko[1] != -1 && ko[2] != -1) {
        if (ko[1] != 0) {
            if (ko[2] != 0) {
                g->win_type = 2;
                g->round_winner = 0;
            } else {
                g->win_type = 1;
                g->round_winner = 2;
            }
        } else if (ko[2] != 0) {
            g->win_type = 1;
            g->round_winner = 1;
        } else if (g->round_time == 0) {
            g->win_type = 3;
            if (life[1] > life[2]) {
                g->round_winner = 1;
                if (g->rounds_won[0] < 10 && g->round_state != 3) {
                    int n = g->rounds_won[0];
                    g->win_icons[0][n] = WIN_ICON_T;
                    g->win_perfect[0][n] = Team_HasFullLife(1) ? WIN_ICON_PERFECT : -1;
                }
            } else if (life[1] < life[2]) {
                g->round_winner = 2;
                if (g->round_state != 3 && g->rounds_won[1] < 10) {
                    g->win_icons[1][g->rounds_won[1]] = WIN_ICON_T;
                }
                Round_SetWinType(2, WIN_ICON_T, 0);
            } else {
                Round_SetWinType(1, WIN_ICON_T, 0);
                Round_SetWinType(2, WIN_ICON_T, 0);
                g->round_winner = 0;
            }
        }
    }
    if (g->debug_no_ko != 0) {
        g->win_type = 0;
    }
    if (g->round_state != 1 && g->round_state != 0) {
        g->intro_skipped = 0;
    }
    if (g->intro_skipped != 0 && g->fade.type == FADE_NONE) {
        /* the intro skipped by a button: straight to the fight, closing bars */
        g->intro_skipped = 0;
        g->round_state = 2;
        Round_PlacePlayers();
        g->round_state_time = -1;
        g->fade.bar_size = g_targetHeight / 8;
        g->fade.type = FADE_BARS;
        g->fade.ticks = 0;
        g->fade.time = 12;
        g->fade.out = 0;
        Fade_Update(&g->fade, 1);
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            if (p != NULL && p->active && p->helper == 0) {
                Rd_ResetToOwnStates(p);
            }
        }
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            if (p != NULL) {
                p->suspended = 0;
            }
            p = g->players[i];
            if (p == NULL) {
                continue;
            }
            if (p->active && p->helper == 0) {
                /* the projectiles go */
                KeyedArray *list = ((ProjectileList *)p->projectiles)->list;
                Projectile *e;
                for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
                    KeyedArray *cur = ((ProjectileList *)g->players[i]->projectiles)->list;
                    Projectile *pe = (Projectile *)(cur->data + cur->elem_size * cur->iter);
                    if (pe != NULL) {
                        AfterImage *ai = pe->afterimage;
                        if (ai != NULL) {
                            ai->running = 0;
                            (pe->afterimage)->anim = NULL;
                            pe->afterimage = NULL;
                        }
                        KeyedArray_Remove(cur->iter, cur);
                    }
                }
            } else if (p->active && p->helper == 1) {
                Helper_Remove(p, 1, 1);
                g = g_game;
            }
        }
        SysDraw_Clear(g->explods);
        SysDraw_Clear(g_game->sysexplods);
        g = g_game;
    }
    g->hits_enabled = 1;
    for (;;) {
        int all;

        switch (g->round_state) {
        case 0:
            g->hits_enabled = 0;
            if (g->round_state_time == 0) {
                g->fade.type = FADE_PALETTE;
                g->fade.ticks = 0;
                g->fade.time = 16;
                g->fade.out = 0;
            }
            if (g->round_state_time < r->start_waittime) {
                goto done;
            }
            g->round_state = 1;
            g->round_state_time = 0;
            break;

        case 1:
            for (i = g->players_first; i <= g->players_last; i++) {
                Player *p = g->players[i];
                if (p != NULL && p->active && p->helper == 0 && g->round_no == 1) {
                    if (g->round_state_time >= 0) {
                        p->suspended = 0;
                    } else {
                        g->assert_intro = 1;
                    }
                }
            }
            if (g->assert_intro != 0) {
                g->intro_time = 0;
            }
            if (g->intro_time == r->round_sndtime + 1 && g->round_no < 10 && g->demo_mode == 0) {
                Rd_PlayFightSound(r->round_snd_group[g->round_no], r->round_snd_number[g->round_no]);
            }
            if (g->intro_time < r->intro_time) {
                g->intro_time++;
                goto done;
            }
            g->round_state = 2;
            g->round_state_time = 0;
            Round_PlacePlayers();
            break;

        case 2:
            if (g->pause_kind == -1 && g->slow_counter == 0 && g->round_state_time >= r->ctrl_time && g->round_time > 0 &&
                g->assert_timerfreeze == 0) {
                g->round_time--;
                if (g->round_time == 0 && g->round_state_time > r->ctrl_time) {
                    g->round_state = 3;
                    g->round_state_time = 0;
                    break;
                }
            }
            if (g->round_state_time == r->fight_sndtime && g->demo_mode == 0) {
                Rd_PlayFightSound(r->fight_snd[0], r->fight_snd[1]);
                g = g_game;
            }
            if (g->round_state_time == r->ctrl_time) {
                for (i = g->players_first; i <= g->players_last; i++) {
                    Player *p = g->players[i];
                    if (p == NULL || !p->active || p->helper != 0 || p->alive == 0) {
                        continue;
                    }
                    p->ctrl = 1;
                    if (p->state_no != 0) {
                        Rd_ResetToOwnStates(p);
                    }
                }
            }
            if (g->win_type == 0 || g->assert_noko != 0 || g->round_state_time <= r->ctrl_time) {
                g->round_ticks++;
                goto done;
            }
            g->round_state = 3;
            g->round_state_time = 0;
            if (g->win_type != 3 && g->assert_nokoslow == 0) {
                g->slow_time = r->slow_time;
                g->slow_step = 500;
                g->slow_counter = 500;
            }
            break;

        case 3:
            g->ko_state_time = g->round_state_time;
            if (g->round_state_time == r->ko_sndtime) {
                Rd_PlayKoSound(r);
                g = g_game;
            }
            if (g->win_type != 3) {
                /* the KO slow motion slows down step by step */
                int slow = g->fight.round.slow_time;
                int now = g->round_state_time;
                if (now < slow * 5 / 10 && now == 5) {
                    g->slow_step = 200;
                    g->slow_counter = 200;
                }
                if (now == slow * 5 / 10) {
                    g->slow_step = 100;
                    g->slow_counter = 100;
                }
                if (now == slow * 7 / 10) {
                    g->slow_step = 50;
                    g->slow_counter = 50;
                }
                if (now == slow * 9 / 10) {
                    g->slow_step = 25;
                    g->slow_counter = 25;
                }
            }
            if (g->round_state_time == r->over_waittime || (g->round_state_time == 0 && g->round_time == 0)) {
                for (i = g->players_first; i <= g->players_last; i++) {
                    Player *p = g->players[i];
                    if (p != NULL && p->active) {
                        p->no_input_states = 1;
                    }
                }
            }
            if (g->precache_enabled != 0 && g->round_state_time == g->fight.round.over_hittime) {
                /* precache the next members of the teams that lost the round */
                for (t = 0; t < 2; t++) {
                    TeamSel *ts;
                    if (g->round_winner == t + 1) {
                        continue;
                    }
                    ts = &g->teams[t];
                    if (ts->cur_member + 1 < ts->member_count) {
                        TextRing_Printf(g->clipboard, "start pre-caching team %i next %i\n", t + 1, ts->cur_member + 1);
                        Precache_QueueTeam(&ts->members[ts->cur_member + 1]);
                        g = g_game;
                        continue;
                    }
                    if (2 - g->arcade_side == t && Match_CheckOver() == 2 - t) {
                        Precache_QueueArcade(1);
                        continue;
                    }
                    if (g->arcade_side == t && Match_CheckOver() == t + 1) {
                        Precache_QueueTeam(&ts->members[0]);
                        g = g_game;
                    }
                }
                Precache_Start();
                g = g_game;
            }
            if (g->round_state_time >= r->over_waittime + 180) {
                g->hits_enabled = 0;
            }
            all = 1;
            for (i = g->players_first; i <= g->players_last; i++) {
                Player *p = g->players[i];
                int ready;
                if (p == NULL || !p->active || p->helper != 0) {
                    continue;
                }
                ready = (p->ctrl != 0 && p->statetype == STATETYPE_STAND && p->movetype == MOVETYPE_IDLE) ||
                        (p->state_no == 5150 && p->state_time > 0);
                all &= ready;
            }
            if ((g->round_state_time < 900 && !all) || g->round_state_time < r->over_waittime) {
                goto done;
            }
            for (i = g->players_first; i <= g->players_last; i++) {
                Player *p = g->players[i];
                if (p != NULL && p->active && p->helper == 0) {
                    p->ctrl = 0;
                }
            }
            g->round_state = 4;
            g->round_state_time = 0;
            g->slow_time = 0;
            break;

        case 4:
            g->hits_enabled = 0;
            if (g->round_state_time + r->over_waittime == r->ko_sndtime) {
                Rd_PlayKoSound(r);
                g = g_game;
            }
            if (g->round_state_time == r->over_wintime) {
                int w;
                for (i = g->players_first; i <= g->players_last; i++) {
                    Player *p = g->players[i];
                    if (p == NULL || !p->active || p->helper != 0) {
                        continue;
                    }
                    if (g->round_winner == p->team_side) {
                        g->players[i]->no_input_states = 0;
                        if (g->players[i]->alive != 0) {
                            Rd_ChangeState(g->players[i], 180);
                            g = g_game;
                        }
                    } else if (g->round_winner != 0) {
                        if (p->alive != 0) {
                            Rd_ChangeState(p, 170);
                            g = g_game;
                        }
                    } else if (p->alive != 0) {
                        Rd_ChangeState(p, 175);
                        g = g_game;
                    }
                }
                w = g->round_winner;
                if (w != 0) {
                    g->round_not_counted = 1;
                    g->rounds_won[w - 1]++;
                    if (Match_CheckOver() == 0) {
                        /* turns: the members of the winning team recover life */
                        int team = g->round_winner - 1;
                        TeamMember *m = &g->teams[team].members[g->teams[team].cur_member];
                        for (k = 0; k < m->count; k++) {
                            Player *p = g->players[m->slots[k].player];
                            Player_SetLife(p, Team_LifeRecovery(team) * p->life_max / 1000 + p->life);
                        }
                    }
                } else {
                    g->draws++;
                    if (g->draws > g->teams[0].draws_max) {
                        g->rounds_won[0]++;
                    }
                    if (g->draws > g->teams[1].draws_max) {
                        g->rounds_won[1]++;
                    }
                    g->round_not_counted = 1;
                }
                if (g->round_not_counted != 0) {
                    g->unused_8718[1] = g->rounds_won[0];
                    g->unused_8718[4] = g->rounds_won[1];
                }
            }
            if (g->assert_roundnotover != 0 && g->over_skipped == 0) {
                /* AssertSpecial roundnotover */
                g->over_end_time = g->round_state_time + 19;
            }
            if (g->over_end_time - g->round_state_time < 16 && g->fade.type == FADE_NONE) {
                g->fade.type = FADE_PALETTE;
                g->fade.ticks = 0;
                g->fade.time = 16;
                g->fade.out = 1;
            }
            if (g->round_state_time < g->over_end_time) {
                goto done;
            }
            {
                int over = Match_CheckOver();
                if (over != 0) {
                    g->match_winner = over;
                    g->match_has_result = 1;
                    g->winner_member = g->teams[over - 1].cur_member;
                }
            }
            if (g->match_has_result == 0) {
                for (i = g->players_first; i <= g->players_last; i++) {
                    Player *p = g->players[i];
                    if (p != NULL && p->active && p->helper == 0) {
                        p->rounds_existed++;
                    }
                }
                Precache_Stop();
                g_gfxClear(0);
                g_game->skip_draw = 0;
                Screen_Present();
                Screen_Present();
                Match_NextRound();
            }
            return 1;

        default:
            goto done;
        }
        g->hits_enabled = 1;
        if ((unsigned int)g->round_state > 4) {
            break;
        }
    }
done:
    if (g->slow_time > 0) {
        g->slow_time--;
    }
    if (g->slow_time == 0 && g->slow_step != 0) {
        g->slow_step = 0;
        g->slow_counter = 0;
    }
    g->round_state_time++;
    g->camera_mode++;
    return 0;
}

/* An announcement element of fight.def at the [Round] position. */
static void Rd_Announce(const SysElem *elem, const FightRound *r)
{
    SysElem_AddFight(elem, (float)r->pos[0], (float)r->pos[1]);
}

/*
 * An announcement whose text is a format: printed into the element with the
 * arguments for the announcement, then put back.
 */
static void Rd_AnnounceText(SysElem *elem, const FightRound *r, const char *a, const char *b, int n, int kind)
{
    char fmt[0x40];

    fmt[0] = '\0';
    strncat(fmt, elem->text, 0x3F);
    if (kind == 0) {
        _snprintf(elem->text, 0x40, fmt, n);
    } else if (kind == 1) {
        _snprintf(elem->text, 0x40, fmt, a);
    } else {
        _snprintf(elem->text, 0x40, fmt, a, b);
    }
    Rd_Announce(elem, r);
    elem->text[0] = '\0';
    strncat(elem->text, fmt, 0x3F);
}

/*
 * 0x00415430: the announcements of the round as system elements: "Round n"
 * (roundN and round.default with the number) at round.time, "Fight" at
 * fight.time, the winner (win / win2 with the display names of the
 * characters) or a draw at win.time, and KO / DKO / TO at ko.time.
 */
void Round_AddAnnouncements(void)
{
    GameVars *g = g_game;
    FightRound *r = &g->fight.round;
    Player *p1 = g->players[1], *p2 = g->players[2], *p3 = g->players[3], *p4 = g->players[4];
    int st, t;

    switch (g->round_state) {
    case 1:
        if (g->intro_time == r->round_time + 2 && g->demo_mode == 0) {
            int n = g->round_no;
            if (n < 10) {
                /* round1..round9 (round "0" is the element before them) */
                Rd_Announce(&r->round[n - 1], r);
            }
            Rd_AnnounceText(&r->round_default, r, NULL, NULL, g_game->round_no, 0);
            g = g_game;
        }
        break;
    case 2:
        if (g->round_state_time == r->fight_time + 1 && g->demo_mode == 0) {
            Rd_Announce(&r->fight, r);
            g = g_game;
        }
        break;
    case 4:
        if (g->round_state_time != r->win_time + 1) {
            break;
        }
        if (g->round_winner == 1) {
            if (p3 == NULL) {
                Rd_AnnounceText(&r->win, r, p1->data->displayname, NULL, 0, 1);
            } else {
                Rd_AnnounceText(&r->win2, r, p1->data->displayname, p3->data->displayname, 0, 2);
            }
        } else if (g->round_winner == 2) {
            if (p4 == NULL) {
                Rd_AnnounceText(&r->win, r, p2->data->displayname, NULL, 0, 1);
            } else {
                Rd_AnnounceText(&r->win2, r, p2->data->displayname, p4->data->displayname, 0, 2);
            }
        } else if (g->win_type == 3) {
            /* a draw: the format gets player 2's name */
            Rd_AnnounceText(&r->draw, r, p2->data->displayname, NULL, 0, 1);
        } else {
            break;
        }
        g = g_game;
        break;
    default:
        break;
    }
    st = g->round_state;
    if (st != 3 && st != 4) {
        return;
    }
    t = g->round_state_time;
    if (st == 4) {
        t += g->ko_state_time;
    }
    if (t != r->ko_time + 1) {
        return;
    }
    switch (g->win_type) {
    case 3:
        Rd_Announce(&r->to, r);
        break;
    case 2:
        Rd_Announce(&r->dko, r);
        break;
    case 1:
        Rd_Announce(&r->ko, r);
        break;
    default:
        break;
    }
}

/*
 * 0x00417400: the combo of p's team goes on while one of p's targets is
 * still being hit (not lying down in state 5150 past its first tick) or p
 * has an active ReversalDef; otherwise the targets are dropped and p's
 * combo hit counter (combo_hits) is cleared. Nothing for team side 0.
 */
void Player_UpdateCombo(Player *p)
{
    KeyedArray *list;
    TargetEntry *e;

    if (p->team_side == 0) {
        return;
    }
    list = p->targets;
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        Player *t = e->player;
        const HitDef *hd = &p->hitdef;
        if ((t->movetype == MOVETYPE_HIT && (t->state_no != 5150 || t->state_time == 0)) ||
            (hd->active != 0 && hd->reversal_attr != 0)) {
            g_game->combo[p->team_side - 1].ongoing = 1;
            return;
        }
        KeyedArray_Remove(list->iter, list);
    }
    p->combo_hits = 0;
}

/*
 * 0x00419350: the life the members of team `team` (0, 1) of a turns match
 * recover after winning a round, in 1/1000 of their life maximum: the
 * team's base (recovery_base) plus its rate (recovery_rate) scaled by the time left
 * in the round (none past 100 counts) and by the opponents' remaining life
 * (weight recovery_weight).
 */
int Team_LifeRecovery(int team)
{
    GameVars *g = g_game;
    const TeamSel *ts = &g->teams[team];
    const TeamSel *other = (const TeamSel *)((const char *)&g->teams[1] - team * (int)sizeof(TeamSel));
    float left, rate;
    double m;

    left = (float)((100.0 - (double)g->round_ticks / g->fight.time.frames_per_count) / 100.0);
    if (0.0f > left) {
        left = 0.0f;
    }
    m = (double)other->life_ratio * ts->recovery_weight + (1.0 - ts->recovery_weight);
    rate = (float)((double)ts->recovery_rate * left);
    return Ftol(m * rate + ts->recovery_base);
}

/* ------------------------------------------------------------------ the tick of a fight */

/*
 * 0x00413170: one tick of a fight (Game_Tick): the end of the screen when
 * the match has a result or after the frames of -frames (quits), the
 * debug full power, which players run this tick (not paused by a Pause /
 * SuperPause they are not exempt from), the enemies, the input codes
 * queued for the human players; unless paused: the AI, the round state
 * machine (Round_RunState: 1 ends the tick), the demo clock, the auto
 * turning (stand, crouch, walk, jump start, land), the players' input and
 * updates (attackers first; helpers created meanwhile get theirs too), the
 * binds, the EnvShake, animations, projectiles, afterimages, hit pauses,
 * hits, dust, the camera and the stage backgrounds, the draw parameters,
 * the one-shot animations, combos, explods and the announcements.
 * Returns 1 when the round ended this tick.
 */
int Round_Tick(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int *run = g->player_tick;
    int px[65];
    int i;

    if (screen->kind == 4 && g->match_has_result != 0 && screen->fade_out == 0) {
        screen->fade_out = 1;
        screen->fade_ticks = 0;
    }
    if (g->exit_frame > 0 && (int)g->frames >= g->exit_frame) {
        screen->fade_out = 1;
        screen->fade_ticks = 0;
        g_quitRequested = 1;
    }
    g->debug_no_ko = 0;
    if (g->team_variant == 3 || g->demo_mode != 0 || g->screen_kind != 4) {
        g->debug_no_ko = 1;
    }
    if (g->debug_full_power != 0) {
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            if (p != NULL && p->active) {
                Player_SetPower(p, *p->power_max_ref);
            }
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        run[i] = p != NULL && p->active;
    }
    for (i = 1; i <= g->player_slots; i++) {
        Player *p = g->players[i];
        Coord pos;
        Player *e;
        if (p == NULL) {
            continue;
        }
        pos = p->pos;
        px[i] = Ftol(pos.x);
        if (!p->active) {
            continue;
        }
        e = p->enemy;
        if (e != NULL && e->active && e->helper == 0) {
            p->last_enemy = e;
        }
        p->enemy = Player_FindEnemy(p);
    }
    if (g->pause_kind != -1) {
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            if (p != NULL && p->active && p->pause_movetime[g->pause_kind] == 0) {
                run[i] = 0;
            }
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        int flip;
        if (p == NULL || !p->active) {
            continue;
        }
        if (g->input_dir_mode == 1 && p->enemy != NULL) {
            flip = !(p->pos.x <= p->enemy->pos.x);
        } else {
            flip = p->facing == -1;
        }
        Input_SetFlipped(&p->input, flip);
    }
    /* the input codes of the frame go to the human players */
    for (;;) {
        IntArray *q = &g->key_codes;
        int code;
        if (q->read == q->write) {
            break;
        }
        code = q->data[q->read];
        q->read++;
        if (q->read >= q->capacity) {
            q->read = 0;
        }
        if (code == 0) {
            break;
        }
        if (g->defocus_paused != 0 && !(code & 0x1000)) {
            continue;
        }
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            InputBuf *in;
            int key, k;
            if (p == NULL || !p->active || p->ai != 0) {
                continue;
            }
            k = Keys_FindCode(code, p->input_keys);
            if (k == 0) {
                continue;
            }
            in = &p->input;
            if (in->flipped != 0) {
                if ((k & 0xF) == KEY_LEFT) {
                    k = (k & ~0xD) | KEY_RIGHT;
                } else if ((k & 0xF) == KEY_RIGHT) {
                    k = (k & ~0xB) | KEY_LEFT;
                }
            }
            key = k & 0xF;
            if (key == 0 || in->locked != 0) {
                continue;
            }
            in->ring[in->write] = (unsigned int)k;
            in->write++;
            if (in->write >= PLAYER_INPUT_RING_SIZE) {
                in->write = 0;
            }
            if (in->write == in->read) {
                in->read++;
                if (in->read >= PLAYER_INPUT_RING_SIZE) {
                    in->read = 0;
                }
            }
            in->keys[key] = ~((unsigned int)k >> 4) & 1;
        }
    }
    if (g->defocus_paused != 0 || g->slow_counter > 0) {
        return 0;
    }
    /* the AI */
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p == NULL || !p->active || p->ai_state.state_msg_sent != 1) {
            continue;
        }
        if ((p->movetype == MOVETYPE_IDLE && p->ctrl != 0) || p->custom_states != NULL ||
            p->movetype == MOVETYPE_HIT) {
            p->ai_state.state_msg_sent = 0;
            Ai_Broadcast(p, NULL, 1, -1, NULL);
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            Ai_Tick(p);
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            p->ai_state.msg_count = 0;
        }
    }
    if (screen->kind == 4 && g->defocus_paused == 0) {
        if (Round_RunState()) {
            return 1;
        }
        g = g_game;
        if (g->defocus_paused == 0 && g->demo_mode != 0 && Demo_TickFight(screen)) {
            return 1;
        }
    }
    run = g->player_tick;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && p->suspended != 0) {
            run[i] = 0;
        }
    }
    /* autoturn */
    if (g->stage.autoturn != 0 && g->screen_kind != 6 && g->screen_kind != 7 && g->screen_kind != 8) {
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            Player *e;
            int s;
            if (p == NULL || !p->active || p->helper != 0 || run[i] == 0) {
                continue;
            }
            e = p->enemy;
            if (e == NULL || !e->active) {
                if (g->round_state <= 2) {
                    continue;
                }
                e = p->last_enemy;
                if (e == NULL || !e->active) {
                    continue;
                }
            }
            s = p->state_no;
            if (s == 52) {
                /* landing: on the last tick of the animation */
                const AnimState *a = g->players[i]->anim;
                const KeyedArray *actions = a->anim->actions;
                const AnimAction *act = (const AnimAction *)(actions->data + actions->elem_size * a->action);
                if (a->time < act->total_time - 1) {
                    continue;
                }
            } else if (s != 0 && s != 11 && s != 20 && s != 40) {
                continue;
            }
            Player_FaceTowards(p, e);
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            Player_UpdateInput(p);
            g = g_game;
        }
    }
    memset(&g->assert_intro, 0, 11);
    g->zoom_active = 0.0f;
    g->zoom_y = 0.0f;
    g->zoom_x = 0.0f;
    g->zoom_scale = 1.0f;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && run[i] != 0) {
            memset(&p->nostandguard, 0, 9);
        }
    }
    /* the players' updates: attackers first, then the rest; again while helpers get created */
    g->player_tick[64] = 0;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && run[i] != 0 && p->movetype == MOVETYPE_ATTACK) {
            Player_Update(p);
            g = g_game;
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && run[i] != 0 && (unsigned int)p->update_frame != g->frames) {
            Player_Update(p);
            g = g_game;
        }
    }
    while (g->player_tick[64] != 0) {
        g->player_tick[64] = 0;
        for (i = g->players_first; i <= g->players_last; i++) {
            Player *p = g->players[i];
            if (p != NULL && p->active && run[i] != 0 && (unsigned int)p->update_frame != g->frames) {
                Player_Update(p);
                g = g_game;
            }
        }
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && g->player_tick[i] != 0) {
            Player_UpdateBind(p);
        }
    }
    /* EnvShake */
    if (g->envshake[1] > 0 && g->defocus_paused == 0 && g->slow_counter == 0) {
        const float *env = (const float *)g->envshake;
        float s, f;
        double v = Rd_Sin(((double)env[3] * g->envshake[0] + env[5]) * 3.141592653589793 / 180.0);
        s = (float)(v * (env[4] / (g_gameCoord[0] / g->stage.unit)));
        f = (float)floor(s + 0.5);
        g->camera_y_base = Ftol(f);
        Camera_UpdateCrop();
        g->envshake[0]++;
        if (g->envshake[0] == g->envshake[1]) {
            g->envshake[1] = 0;
            f = (float)floor(0.0 + 0.5);
            g->camera_y_base = Ftol(f);
            Camera_UpdateCrop();
            Camera_UpdateCrop();
        }
    }
    run = g->player_tick;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && run[i] != 0 && p->hitpause_time == 0) {
            Anim_Step(p->anim);
            p->anim_owner = p->custom_states != NULL ? p->custom_owner : -1;
        }
    }
    for (i = g->players_last; i >= g->players_first; i--) {
        Player *p = g->players[i];
        if (p == NULL || !p->active) {
            continue;
        }
        if (run[i] != 0) {
            Projectiles_Update((ProjectileList *)p->projectiles);
        }
        Projectiles_UpdateDraw((ProjectileList *)g->players[i]->projectiles);
    }
    for (i = g->players_last; i >= g->players_first; i--) {
        Player *p = g->players[i];
        AfterImage *ai;
        if (p == NULL || !p->active || run[i] == 0) {
            continue;
        }
        ai = p->afterimage;
        if (p->suspended == 0 && ai != NULL) {
            if (!AfterImage_Update(&p->draw, ai, p->remap, p->hitpause_time == 0)) {
                ai->anim = NULL;
                p->afterimage = NULL;
            }
        }
        Projectiles_UpdateAfterImages((ProjectileList *)g->players[i]->projectiles);
    }
    for (i = g->players_last; i >= g->players_first; i--) {
        Player *p = g->players[i];
        if (p != NULL && p->active && run[i] != 0 && p->hitpause_time > 0) {
            p->hitpause_time--;
        }
    }
    if (g->hits_enabled != 0) {
        Players_CheckHits();
        g = g_game;
    }
    if (g->pause_kind == -1) {
        for (i = g->players_last; i >= g->players_first; i--) {
            Player *p = g->players[i];
            Coord pos;
            if (p == NULL || !p->active) {
                continue;
            }
            pos = p->pos;
            if (Ftol(pos.x) != px[i]) {
                Player_MakeDust(g->players[i]);
            }
        }
    }
    for (i = g->players_last; i >= g->players_first; i--) {
        Player *p = g->players[i];
        if (p != NULL && p->active && p->suspended == 0) {
            Ai_UpdateLearn(p, g->player_tick[i]);
        }
    }
    Camera_Update();
    Camera_UpdateZoom();
    Stage_UpdateBgCamera(screen);
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            Player_UpdateDrawParams(p, 1);
        }
    }
    AnimSet_Update(g->anims);
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            Player_UpdateCombo(p);
        }
    }
    {
        SysDrawList *list = g->explods;
        for (i = 0; i < list->capacity; i++) {
            SysDraw_Start(list, i);
        }
    }
    Round_AddAnnouncements();
    return 0;
}

/*
 * 0x00416EA0: team `team` (1, 2) hit `hits` more times in its combo; the
 * counter shows from 2 hits on (at most 999) for [Combo] displaytime ticks.
 */
void Combo_AddHits(int team, int hits)
{
    GameVars *g;
    ComboState *c;
    const FightCombo *fc;
    int n;

    if (team != 1 && team != 2) {
        return;
    }
    g = g_game;
    c = &g->combo[team - 1];
    fc = &g->fight.combo[team - 1];
    c->hits += hits;
    n = c->hits;
    if (c->timer == 0) {
        if (n < 2) {
            return;
        }
        c->x = fc->start_x;
    }
    if (n < 2) {
        return;
    }
    c->timer = fc->displaytime;
    c->shown = n;
    if (n > 999) {
        c->shown = 999;
    }
}

/*
 * 0x004172F0: the combo counter of team `team` (1, 2): the hits go back to
 * 0 when the combo was not continued this tick (ongoing); while shown, the
 * counter slides from [Combo] start.x towards pos.x (an eighth of the way
 * and a unit a tick) and its time runs out; once hidden it slides back off
 * at 3 units (in 320-wide units) a tick and is cleared there.
 */
void Combo_Tick(int team)
{
    GameVars *g = g_game;
    const FightCombo *fc = &g->fight.combo[team - 1];
    ComboState *c;

    if (team != 1 && team != 2) {
        return;
    }
    c = &g->combo[team - 1];
    if (c->ongoing == 0) {
        c->hits = 0;
    }
    c->ongoing = 0;
    if (c->timer > 0) {
        int target = fc->pos[0];
        if (c->x != target) {
            int dir = c->x < target ? 1 : -1;
            c->x = (target - c->x) / 8 + c->x + dir;
            if ((target - c->x) * dir < 0) {
                c->x = target;
            }
        }
        c->timer--;
    }
    if (c->hits < 2 && c->timer == 0) {
        int target = fc->start_x;
        if (c->x != target) {
            int dir = c->x < target ? 1 : -1;
            float step = (float)((double)Ftol(g->game_coord[0]) * 3.0 / 320.0);
            c->x = Ftol((double)step * dir + c->x);
            if ((target - c->x) * dir <= 0) {
                c->x = target;
                c->shown = 0;
            }
        }
    }
}

/* The shake of a combo counter in its first ticks (counter.shake): 1, 2, 1, -, 1, -, 1 units in 320. */
static int Rd_ComboShake(const FightCombo *fc, const ComboState *c, int base, int team1)
{
    int t = fc->displaytime;
    int tm = c->timer;
    int v = base;

    if (fc->shake == 0) {
        return v;
    }
    if (tm == t - 1) {
        /* team 1 adds a constant 0 here, not the base */
        v = team1 ? Ftol(Units_From320(1.0f) + 0.0f) : Ftol(Units_From320(1.0f) + (double)v);
    }
    if (tm == t - 2) {
        v = Ftol(Units_From320(2.0f) + (double)v);
    }
    if (tm == t - 3) {
        v = Ftol(Units_From320(1.0f) + (double)v);
    }
    if (tm == t - 5) {
        v = Ftol(Units_From320(1.0f) + (double)v);
    }
    if (tm == t - 7) {
        v = Ftol(Units_From320(1.0f) + (double)v);
    }
    return v;
}

/*
 * 0x00416F10: draws the combo counter of team `team` (from 2 hits): the
 * counter and text formats get the hits; team 1 puts the text after the
 * counter, team 2 the counter before the text (by the font width of the
 * other element; team 2 draws no counter without a text font). The counter
 * shakes in its first ticks. Drawn with the PalFX of palette group 0x200.
 */
void Combo_Draw(int team)
{
    GameVars *g = g_game;
    FightCombo *fc = &g->fight.combo[team - 1];
    const ComboState *c;
    char text[0x40], counter[0x40];
    float y;
    int x, n, w;

    if (team != 1 && team != 2) {
        return;
    }
    c = &g->combo[team - 1];
    n = c->shown;
    if (n < 2) {
        return;
    }
    g_palFxCur = &g->palgroup_200_palfx;
    x = c->x;
    _snprintf(text, 0x40, fc->text_fmt, n);
    fc->text.text[0] = '\0';
    strncat(fc->text.text, text, 0x3F);
    _snprintf(counter, 0x40, fc->counter_fmt, n);
    fc->counter.text[0] = '\0';
    strncat(fc->counter.text, counter, 0x3F);
    if (team == 1) {
        int idx = fc->counter.group;
        w = 0;
        if (idx != -1 && g_game->fight.fonts[idx] != NULL) {
            w = Ftol((double)Font_TextWidth(counter, g_game->fight.fonts[idx]) * fc->counter.scale_x);
        }
        y = (float)fc->pos[1];
        SysElem_DrawFight(&fc->text, -1, (float)(x + w), y);
        SysElem_DrawFight(&fc->counter, -1, (float)(Rd_ComboShake(fc, c, 0, 1) + x), y);
    } else {
        int idx;
        y = (float)fc->pos[1];
        SysElem_DrawFight(&fc->text, -1, (float)x, y);
        idx = fc->text.group;
        if (idx != -1) {
            w = 0;
            if (g_game->fight.fonts[idx] != NULL) {
                w = Ftol((double)Font_TextWidth(text, g_game->fight.fonts[idx]) * fc->text.scale_x);
            }
            SysElem_DrawFight(&fc->counter, -1, (float)(x - Rd_ComboShake(fc, c, w, 0)), y);
        }
    }
    g_palFxCur = NULL;
}

/* 0x00419570: every character (not helper) of team `side` in the match has full life. */
int Team_HasFullLife(int side)
{
    GameVars *g = g_game;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && p->helper == 0 && p->team_side == side && p->life != p->life_max) {
            return 0;
        }
    }
    return 1;
}

/*
 * 0x004194B0: records how team `team` won the current round: `type`
 * (WIN_ICON_*), or with type -1 from the attributes of the finishing hit
 * (normal, throw, special, hyper), plus the perfect icon when the team still
 * has full life. At most 10 rounds are recorded, and none once the round is
 * over (state 3).
 */
void Round_SetWinType(int team, int type, unsigned int attr)
{
    GameVars *g = g_game;
    int n;

    if (g->rounds_won[team - 1] >= 10 || g->round_state == 3) {
        return;
    }
    n = g->rounds_won[team - 1];
    if (type == -1) {
        g->win_icons[team - 1][n] = WIN_ICON_N;
        if (attr & 0x1C0) {
            g->win_icons[team - 1][n] = WIN_ICON_THROW;
        }
        if (attr & 0x490) {
            g->win_icons[team - 1][n] = WIN_ICON_S;
        } else if (attr & 0x920) {
            g->win_icons[team - 1][n] = WIN_ICON_H;
        }
    } else {
        g->win_icons[team - 1][n] = type;
    }
    g->win_perfect[team - 1][n] = Team_HasFullLife(team) ? WIN_ICON_PERFECT : -1;
}

/*
 * 0x004193E0: a KO by a hit of team `attacker_team` on a player of team
 * `team`: a KO of a teammate gives the round to the other team (teammate
 * icon), a KO of team 0 gives a normal win to team 1.
 */
void Round_SetKoWinType(int attacker_team, int team, unsigned int attr)
{
    GameVars *g;
    int winner, n;

    if (team != attacker_team) {
        if (team != 0) {
            Round_SetWinType(attacker_team, -1, attr);
            return;
        }
        g = g_game;
        if (g->rounds_won[0] >= 10 || g->round_state == 3) {
            return;
        }
        n = g->rounds_won[0];
        g->win_icons[0][n] = WIN_ICON_N;
        g->win_perfect[0][n] = Team_HasFullLife(1) ? WIN_ICON_PERFECT : -1;
        return;
    }
    winner = attacker_team == 1 ? 2 : 1;
    g = g_game;
    if (g->rounds_won[winner - 1] >= 10 || g->round_state == 3) {
        return;
    }
    n = g->rounds_won[winner - 1];
    g->win_icons[winner - 1][n] = WIN_ICON_TEAMMATE;
    g->win_perfect[winner - 1][n] = Team_HasFullLife(winner) ? WIN_ICON_PERFECT : -1;
}
