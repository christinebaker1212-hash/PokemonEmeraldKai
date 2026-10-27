/*
 * Player runtime (original module 0x43BF40-0x44444F, right after the
 * character loader of char.c): state and animation changes, life and power,
 * the per-tick update of a player (state machine, hit pause, get hit
 * bookkeeping, physics, KO, bindings), stage and screen bounds, player
 * pushing, hit detection and the application of HitDefs, guards,
 * reversals and projectile hits, hit sparks, the draw parameters and the
 * drawing of the players (with the debug collision boxes), the palette
 * groups of the AllPalFX/BGPalFX controllers, the precache thread of the
 * select screen and the projectiles.
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double (the FPU runs with 53-bit precision) and are rounded to
 * float exactly where the original stores a float; float to int conversions
 * truncate like __ftol2.
 */
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

/* The state list a player runs: the custom states of another player, or its own. */
static StateDefs *Player_StateList(const Player *p)
{
    return p->custom_states != NULL ? p->custom_states : p->states;
}

/* ------------------------------------------------------------------ state and animation changes */

/*
 * 0x0043BF40: switches to state `state`; state 0 is always valid. The change
 * happens even when the state does not exist (with a warning); returns 0 then.
 */
int Player_ChangeState(int state, Player *p)
{
    int ok = 1;

    if (state != 0) {
        KeyedArray *list = Player_StateList(p)->list;
        int i = KeyedArray_Find(list, state, 0);
        if (i == -1 || list->data + list->elem_size * i == NULL) {
            ok = 0;
        }
    }
    p->prev_state_no = p->state_no;
    p->state_no = state;
    p->state_time = 0;
    if (!ok) {
        Player_Warnf(p, "changed to invalid state %i (from state %i)\n", state, p->prev_state_no);
    }
    return ok;
}

/*
 * 0x0043BFC0: sets the life (LifeSet, LifeAdd), clamped to 0..lifemax. Not
 * after the KO time of a finished round. A player that reaches 0 during the
 * round makes the other team win (by the attacker's last hit, or "suicide").
 */
void Player_SetLife(Player *p, int life)
{
    GameVars *g = g_game;

    if (g->round_state == 3 && p->alive != 0 && g->round_state_time >= g->fight.round.over_hittime) {
        return;
    }
    p->life = life;
    if (life < 0) {
        p->life = 0;
    }
    if (p->life > p->life_max) {
        p->life = p->life_max;
    }
    if (g->debug_no_ko != 0 && p->life == 0) {
        p->life = 1;
    }
    if (p->life <= 0 && p->helper == 1 && p->helper_player != 1) {
        p->life = 1;
    }
    if (p->life <= 0 && p->helper == 0 && g->round_state == 2) {
        if (p->gethit_attacker != NULL) {
            Round_SetKoWinType(p->gethit_attacker->team_side, p->team_side, p->gethit_attr);
        } else {
            Round_SetWinType(p->team_side == 1 ? 2 : 1, WIN_ICON_SUICIDE, 0);
        }
    }
}

/* 0x0043C0B0: sets the power of the player's team leader, clamped to 0..max (not once the round is over). */
void Player_SetPower(Player *p, int power)
{
    GameVars *g = g_game;

    if (g->round_state != 3 && g->round_state != 4) {
        *p->power_ref = power;
        if (*p->power_ref < 0) {
            *p->power_ref = 0;
        }
        if (*p->power_ref > *p->power_max_ref) {
            *p->power_ref = *p->power_max_ref;
        }
    }
    if (g->debug_full_power != 0) {
        *p->power_ref = *p->power_max_ref;
    }
}

/* 0x0043C120: starts action `action` from its first element (even when it is the current one). */
void Player_SetAction(Player *p, int action)
{
    int ok = Anim_SetAction(p->anim, action);

    p->action_failed = ok == 0;
    if (ok) {
        AnimState *st = p->anim;
        KeyedArray *actions = st->anim->actions;
        if (st->action > st->anim->action_count) {
            st->action = 0;
        }
        st->ticks = 0;
        st->time = 0;
        st->elem = (AnimElem *)((AnimAction *)(actions->data + actions->elem_size * st->action))->elems->data;
        st->elem_index = 0;
        Anim_SeekElem(st);
        return;
    }
    Player_Warnf(p, "Changed to invalid action %i", action);
}

/*
 * 0x0043C190: starts action `action` at element `elem` (1-based). On failure
 * the action is set again (keeping its time when it is the current one).
 */
void Player_SetActionElem(Player *p, int action, int elem)
{
    AnimState *st = p->anim;
    int ok = Anim_SetAction(st, action);

    if (ok) {
        ok = Anim_SetElem(st, elem - 1);
    }
    p->action_failed = ok == 0;
    if (!ok) {
        ok = Anim_SetAction(p->anim, action);
        p->action_failed = ok == 0;
        if (!ok) {
            Player_Warnf(p, "Changed to invalid action %i", action);
        } else {
            Player_Warnf(p, "Changed to invalid animelem %i within action %i", elem, action);
        }
    }
}

/* 0x0043C220: ChangeAnim: an action of the player's own animations. */
void Player_ChangeAnimSelf(Player *p, int action)
{
    p->anim->anim = p->data->anim;
    p->anim_from_owner = 0;
    Player_SetAction(p, action);
}

/* 0x0043C240: ChangeAnim with an element. */
void Player_ChangeAnimSelfElem(Player *p, int action, int elem)
{
    p->anim->anim = p->data->anim;
    p->anim_from_owner = 0;
    Player_SetActionElem(p, action, elem);
}

/* Anim2 source: the custom state owner's animations, or the player's own outside of a custom state. */
static void Player_UseAnim2(Player *p)
{
    GameVars *g;
    Anim *anim;

    if (p->custom_states == NULL) {
        g = g_game;
        TextRing_Printf(g->clipboard, "%-4i: Warning: Player %s (%i) in state %i: %s\n", g->frames, p->name, p->id,
                        p->state_no, "used Anim2 outside of custom state");
    }
    anim = p->data->anim;
    if (p->custom_states != NULL && p->custom_anim != NULL) {
        anim = p->custom_anim;
    }
    p->anim->anim = anim;
    p->anim_from_owner = 1;
}

/* 0x0043C280: ChangeAnim2: an action of the custom state owner's animations. */
void Player_ChangeAnim2(Player *p, int action)
{
    Player_UseAnim2(p);
    Player_SetAction(p, action);
}

/* 0x0043C300: ChangeAnim2 with an element. */
void Player_ChangeAnim2Elem(Player *p, int action, int elem)
{
    Player_UseAnim2(p);
    Player_SetActionElem(p, action, elem);
}

/*
 * 0x0043C390: turns p towards `other` when p has control (or the round is
 * over) and is facing away from it, unless AssertSpecial noautoturn; standing
 * and crouching players get the turning action (5, 6).
 */
void Player_FaceTowards(Player *p, Player *other)
{
    Coord a, b;

    if (p->ctrl == 0 && g_game->round_state <= 2) {
        return;
    }
    a = p->pos;
    b = other->pos;
    if ((a.x - b.x) * p->facing > 0.0 && p->noautoturn == 0) {
        p->facing = -p->facing;
        Player_UpdateInputFacing(p);
        if (p->statetype == STATETYPE_STAND) {
            Anim_SetAction(p->anim, 5);
        } else if (p->statetype == STATETYPE_CROUCH) {
            Anim_SetAction(p->anim, 6);
        }
    }
}

/* 0x0043C460: Turn. */
void Player_Turn(Player *p)
{
    p->facing = -p->facing;
    Player_UpdateInputFacing(p);
}

/* ------------------------------------------------------------------ the per-tick update */

/* Player_Update's state switch (after the state machine ran), with the 100.0 the original keeps on the FPU. */
#define PS_HUNDRED 100.0

/*
 * 0x0043C480: one tick of a player: hit pause, HitOverride timers, the
 * TargetBind of the player that binds it, the per-tick defaults (screen
 * bound, push, draw parameters), the special states -3, -2, -1 and the state
 * machine (rerun after automatic state changes, at most 2500 times), then,
 * outside hit pause: get hit position snaps, pending damage, the automatic
 * changes of the common states (guard, get hit, down, landing), velocity,
 * corner push, friction and gravity, the KO and its sounds, the KO blinking
 * of training, the HitBy timers and the get hit counters. Finally the draw
 * offset, the stage bounds, the camera, the draw parameters and the targets
 * this player binds.
 */
void Player_Update(Player *p)
{
    GameVars *g;
    int frozen = 0;
    int loops;
    int i;
    int state;
    Player *b;
    TargetEntry *t;

    if (p->suspended != 0) {
        return;
    }
    if (p->gethit_shaketime > 0) {
        p->hitpause_time = 0;
        p->hitpause_next = 0;
    }
    if (p->hitpause_next > 0) {
        p->hitpause_time = p->hitpause_next;
        p->hitpause_next = 0;
    }
    if (p->hitpause_time > 0) {
        frozen = 1;
    }
    p->paused = frozen;
    for (i = 0; i < 8; i++) {
        HitOverride *o = &p->hit_override[i];
        if (o->active != 0 && o->time != -1) {
            o->time--;
            if (o->time <= 0) {
                o->active = 0;
            }
        }
    }
    g = g_game;
    b = p->gethit_isbound;
    if (b != NULL && (b->active == 0 || b->update_frame != (int)g->frames ||
                      (t = Targets_FindPlayer(b->targets, p)) == NULL || t->bind_active == 0)) {
        p->gethit_isbound = NULL;
    }
    if (p->data->mugenversion >= g->compat_version[1] || !frozen) {
        float angle = p->draw2.angle;
        Coord zero;

        p->screenbound = 1;
        p->movecamera_x = 1;
        p->movecamera_y = 1;
        p->playerpush = 1;
        memset(&p->draw2, 0, sizeof(DrawParams));
        p->draw2.scale2_y = 1.0f;
        p->draw2.flags = 0xFFFF;
        p->draw2.scale2_x = 1.0f;
        p->draw2.masked = 1;
        p->draw2.scale_y = 1.0f;
        p->draw2.scale_x = 1.0f;
        p->draw2.alpha_src = 1.0f;
        p->draw2.bottom_stretch = 1.0f;
        p->draw2.top_stretch = 1.0f;
        p->draw2.width_ratio = 1.0f;
        p->draw2.width_anchor = 0.5f;
        p->draw2.angle = angle;
        p->draw2.flags = 0;
        p->width_edge_back = 0;
        p->width_edge_front = 0;
        p->width_player_back = 0;
        p->width_player_front = 0;
        zero.x = 0.0;
        zero.y = 0.0;
        zero.z = 0.0;
        zero.unit = 1.0;
        zero.type = COORD_TYPE_2D;
        zero.pad_24 = 0;        /* not set in the original (stack garbage) */
        p->visible = 1;
        p->draw_offset = zero;
        if (p->helper == 1 && p->helper_player != 1) {
            p->screenbound = 0;
            p->movecamera_y = 0;
            p->movecamera_x = 0;
            p->playerpush = 0;
        }
        p->prev_pos = p->pos;
    }
    if (p->helper == 0 || (p->helper == 1 && p->helper_player == 1)) {
        Player_RunSpecialState(p, -3, 3);
        Player_RunSpecialState(p, -2, 7);
    }
    if (p->no_keyctrl == 0) {
        Player_RunSpecialState(p, -1, 3);
    }
    loops = 0;

again:
    if (Player_RunState(p)) {
        do {
            if (loops++ > 2500) {
                Error_Printf("State machine stuck in loop (stopped after %i loops): %i for %s\n"
                             "Check your states for infinite ChangeState loops.\n",
                             2500, p->state_no, p->data->name);
                Error_Exit();
            }
            if (g_keyState[SDLK_ESCAPE] != 0 || g_quitRequested != 0) {
                break;
            }
        } while (Player_RunState(p));
    }
    g = g_game;
    if (p->data->mugenversion >= g->compat_version[1] && p->invisible != 0) {
        p->visible = 0;
    }
    if (frozen) {
        goto offsets;
    }
    if (p->movetype == MOVETYPE_HIT) {
        if (p->gethit_xoff != 0.0f) {
            p->pos.x = p->gethit_xoff + p->pos.x;
            p->gethit_xoff = 0.0f;
        }
        if (p->gethit_yoff != 0.0f) {
            p->pos.y = p->gethit_yoff + p->pos.y;
            p->gethit_yoff = 0.0f;
        }
        if (p->gethit_shaketime > 0) {
            p->gethit_shaketime--;
        }
        if (p->gethit_damage != 0) {
            int life = p->life - p->gethit_damage;
            if (g->round_state != 3 || p->alive == 0 || g->round_state_time < g->fight.round.over_hittime) {
                p->life = life;
                if (life < 0) {
                    p->life = 0;
                }
                if (p->life > p->life_max) {
                    p->life = p->life_max;
                }
                if (g->debug_no_ko != 0 && p->life == 0) {
                    p->life = 1;
                }
            }
            p->gethit_damage = 0;
        }
    } else {
        p->gethit_damage = 0;
        p->gethit_id = 0;
        p->gethit_attacker = NULL;
        p->gethit_attr = 0;
        p->gethit_fall = 0;
        p->gethit_recovertime = p->consts.liedown_time;
        p->gethit_fallcount = 0;
        p->gethit_shaketime = 0;
        p->gethit_fall_envshake_time = 0;
        p->gethit_fall_kill = 1;
        p->defence_mul = (float)(PS_HUNDRED / p->consts.defence);
    }
    if (p->next_facing != 0) {
        if (p->facing != p->next_facing) {
            p->facing = -p->facing;
            Player_UpdateInputFacing(p);
            g = g_game;
        }
        p->next_facing = 0;
    }
    if (p->data->mugenversion < g->compat_version[1] && p->invisible != 0) {
        p->visible = 0;
    }

    /* automatic changes of the common states */
    state = p->state_no;
    switch (state) {
    case 40:                    /* jump start */
        if (p->state_time == 0) {
            p->air_jumps = p->consts.airjump_num;
        }
        break;
    case 120:                   /* guard start, guarding */
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
        p->in_guard_dist = 1;
        break;
    case 130:                   /* guarding */
    case 131:
    case 132:
        p->defence_mul = (float)(PS_HUNDRED / p->consts.defence);
        p->in_guard_dist = 1;
        break;
    case 140: {                 /* guard end */
        AnimState *st = p->anim;
        KeyedArray *actions = st->anim->actions;
        p->defence_mul = (float)(PS_HUNDRED / p->consts.defence);
        p->in_guard_dist = 1;
        if (p->state_time >= ((AnimAction *)(actions->data + actions->elem_size * st->action))->total_time) {
            if (p->statetype == STATETYPE_CROUCH) {
                Player_ChangeState(11, p);
            } else if (p->statetype == STATETYPE_AIR) {
                Player_ChangeState(51, p);
            } else {
                /* standing (and lying down): state 0 without the validity check */
                p->state_time = 0;
                p->state_no = 0;
                p->prev_state_no = state;
            }
            p->gethit_id = 0;
            p->gethit_attacker = NULL;
            p->gethit_attr = 0;
            goto again;
        }
        break;
    }
    case 5070:                  /* tripped */
        if (p->state_time == 0) {
            p->gethit_fallcount += 1;
            p->defence_mul = (float)((double)p->consts.fall_defence_mul * p->defence_mul);
        }
        break;
    case 5100:                  /* hit the ground */
        if (p->state_time == 0) {
            if (p->gethit_fallcount > 0) {
                p->gethit_recovertime >>= 1;
            }
            p->gethit_fallcount++;
            p->defence_mul = (float)((double)p->consts.fall_defence_mul * p->defence_mul);
        }
        if (p->gethit_recovertime <= 10) {
            p->hitby_attr = 0;
            p->hitby_time = -1;
        }
        break;
    case 5110: {                /* lying down */
        int e = p->gethit_recovertime;
        if (e > 0) {
            p->gethit_recovertime = e - 1;
        } else if (p->alive != 0 && p->state_time > 0) {
            p->gethit_recovertime = p->consts.liedown_time;
            Player_ChangeState(5120, p);
            goto again;
        }
        break;
    }
    case 5120:                  /* getting up */
        if (p->state_time == 0) {
            p->defence_mul = (float)(PS_HUNDRED / p->consts.defence);
        }
        p->gethit_id = 0;
        p->gethit_attacker = NULL;
        p->gethit_attr = 0;
        break;
    case 5200:                  /* fall recovery */
    case 5210:
        if (p->state_time == 0) {
            p->defence_mul = (float)(PS_HUNDRED / p->consts.defence);
        }
        break;
    default:
        break;
    }

    /* velocity */
    if (p->pos_freeze > 0) {
        p->pos_freeze--;
    } else {
        Coord v = p->vel;
        double r = v.unit / p->pos.unit;
        v.x = v.x * r;
        v.y = r * v.y;
        v.z = r * v.z;
        p->pos.x = v.x + p->pos.x;
        p->pos.y = p->pos.y + v.y;
        p->pos.z = v.z + p->pos.z;
    }
    /* corner push */
    if (p->push_time != 0 && g_game->pause_kind != 1) {
        Coord v = p->push_vel;
        double r = v.unit / p->pos.unit;
        double x;
        int sign = -1;

        v.x = v.x * r;
        v.y = r * v.y;
        v.z = r * v.z;
        p->pos.x = v.x + p->pos.x;
        p->pos.y = v.y + p->pos.y;
        p->pos.z = v.z + p->pos.z;
        x = p->push_decay_x * p->push_vel.x;
        p->push_vel.x = x;
        p->push_vel.y = p->push_decay_y * p->push_vel.y;
        if (!(0.0 > x)) {
            sign = 1;
        }
        if (p->push_min_speed > x * sign) {
            p->push_time = 0;
        }
        if (p->push_time > 0) {
            p->push_time--;
        }
        if (p->push_time == 0) {
            Coord zero;
            zero.x = 0.0;
            zero.y = 0.0;
            zero.z = 0.0;
            zero.unit = 1.0;
            zero.type = COORD_TYPE_2D;
            zero.pad_24 = 0;    /* not set in the original (stack garbage) */
            p->push_vel = zero;
        }
    }
    /* friction and gravity */
    if (p->physics == 1) {
        double vx = p->consts.stand_friction * p->vel.x;
        float limit;
        p->vel.x = vx;
        limit = (float)((double)Ftol(g_game->game_coord[0]) / 320.0);
        if (limit > fabs(vx)) {
            p->vel.x = 0.0;
        }
    }
    if (p->physics == 2) {
        p->vel.x = p->consts.crouch_friction * p->vel.x;
    }
    if (p->physics == 3) {
        double vy = g_gameCoord[0] / p->data->localcoord[0] * p->consts.yaccel + p->vel.y;
        p->vel.y = vy;
        if (p->state_no != 105 && vy > 0.0 && p->pos.y >= 0.0) {
            /* landing: state 52 (Player_ChangeState inlined) */
            Player_ChangeState(52, p);
            if (p->data->mugenversion < g_game->compat_version[1]) {
                p->draw_offset.x = 0.0;
            }
            goto again;
        }
    }

    /* KO */
    g = g_game;
    if (p->life <= 0 && p->alive != 0) {
        int v = g_nokoLatch;
        if (v != 0) {
            v = g->assert_noko;
            g_nokoLatch = v;
        }
        if (g->assert_nokosnd == 0) {
            if (v != 0) {
                goto ko_done;
            }
            Sound_Stop(p->index + 1000);
            Sound_PlayPanned(p->data->snd, 11, Ftol(p->draw.x), 0, p->data->volumescale);
            v = g_nokoLatch;
        }
        if (v == 0) {
            g_nokoLatch = 1;
            p->alive = 0;
            p->ko_time = 0;
            p->ctrl = 0;
            if (p->movetype != MOVETYPE_HIT) {
                int w;
                Player_ChangeState(5030, p);
                w = Ftol(g_game->game_coord[0]);
                p->gethit_vel_x = (float)((double)w * (double)-2.8f / 320.0);
                w = Ftol(g_game->game_coord[0]);
                p->gethit_fall = 1;
                p->gethit_vel_y = (float)((double)w * -3.5 / 320.0);
            }
            Targets_Release(p, NULL);
            Explods_RemoveOnHit(g_game->explods, p->id);
        }
    }
ko_done:
    g = g_game;
    if (p->alive == 0) {
        p->ko_time++;
        if (p->consts.ko_echo != 0) {
            if (p->ko_time == 20 && g->assert_nokosnd == 0) {
                Sound_PlayPanned(p->data->snd, 11, Ftol(p->draw.x), 0, p->data->volumescale * 0.5);
                g = g_game;
            }
            if (p->ko_time == 50 && g->assert_nokosnd == 0) {
                Sound_PlayPanned(p->data->snd, 11, Ftol(p->draw.x), 0, p->data->volumescale * 0.25);
                g = g_game;
            }
        }
        /* a KOed simul partner does not push */
        if ((p == g->players[1] && g->players[3] != NULL) || (p == g->players[3] && g->players[1] != NULL) ||
            (p == g->players[2] && g->players[4] != NULL) || (p == g->players[4] && g->players[2] != NULL)) {
            p->playerpush = 0;
        }
        if (p->state_no == 5150 && p->state_time >= 10 &&
            ((p == g->players[1] && g->players[3] != NULL) || (p == g->players[3] && g->players[1] != NULL) ||
             (p == g->players[2] && g->players[4] != NULL) || (p == g->players[4] && g->players[2] != NULL))) {
            p->screenbound = 0;
            p->movecamera_y = 0;
            p->movecamera_x = 0;
        }
    }
    {
        int a = p->hitby_time;
        int c = p->nothitby_time;
        if (a == 0) {
            p->hitby_attr = 0xFFF;
        }
        if (c == 0) {
            p->nothitby_attr = 0xFFF;
        }
        if (a > 0) {
            p->hitby_time = a - 1;
        }
        if (c > 0) {
            p->nothitby_time = c - 1;
        }
    }
    /* KO blinking of the "M" game type */
    if (g->game_type_m == 1 && p->state_no == 5150) {
        int tm = p->state_time;
        if (tm >= 60 && tm < 90) {
            p->visible = (tm >> 2) & 1;
        }
        if (tm >= 90 && tm < 120) {
            p->visible = (tm >> 1) & 1;
        }
        if (tm >= 120 && tm < 150) {
            p->visible = (tm & 3) == 1;
        }
        if (tm >= 150 && tm < 180) {
            p->visible = (tm & 7) == 1;
        }
        if (tm >= 180) {
            p->visible = 0;
        }
    }
    p->state_time += 1;
    if (p->movetype == MOVETYPE_IDLE) {
        p->idle_time += 1;
    } else {
        p->idle_time = 0;
    }
    if (g->team_variant == 3) {
        /* training: KO recovery and refills */
        if (p->state_no == 5150 && p->state_time >= 90) {
            p->state_no = 5120;
            p->state_time = -1;
            p->life = p->life_max;
            p->alive = 1;
            p->gethit_yveladd = 0;
            p->gethit_xveladd = 0;
        }
        if (p->idle_time > 30 && *p->power_ref < *p->power_max_ref) {
            *p->power_ref = *p->power_max_ref;
        }
        if (p->idle_time > 60 && p->life < p->life_max) {
            p->life = p->life_max;
        }
    }
    if (p->move_contact != 0) {
        p->move_contact_time += 1;
    }
    if (p->gethit_hittime >= 0 && p->gethit_shaketime == 0) {
        p->gethit_hittime--;
    }
    if (p->gethit_fall != 0 && p->movetype == MOVETYPE_HIT) {
        p->gethit_fall_time += 1;
    }
    if (p->movetype == MOVETYPE_HIT) {
        p->hit_free_time = 0;
    } else {
        p->hit_free_time += 1;
    }
    if (g->pause_kind != -1) {
        if (p->pause_movetime[g->pause_kind] > 0) {
            p->pause_movetime[g->pause_kind]--;
        }
    } else if (p->movetype == MOVETYPE_HIT) {
        p->pause_movetime[1] = 0;
        p->pause_movetime[0] = 0;
    }
    p->proj_contact_time += 1;

offsets:
    g = g_game;
    if (p->data->mugenversion >= g->compat_version[1] || !frozen) {
        /* draw offset (consts draw_offset), in the units of the state owner */
        const CharData *owner = p->data;
        if (p->custom_states != NULL && p->custom_owner != -1 && g->players[p->custom_owner] != NULL) {
            owner = g->players[p->custom_owner]->data;
        }
        p->draw_offset.x = (double)p->consts.draw_offset[0] * (g_gameCoord[0] / owner->localcoord[0]) * p->facing +
                        p->draw_offset.x;
        owner = p->data;
        if (p->custom_states != NULL && p->custom_owner != -1 && g->players[p->custom_owner] != NULL) {
            owner = g->players[p->custom_owner]->data;
        }
        p->draw_offset.y = (double)p->consts.draw_offset[1] * (g_gameCoord[0] / owner->localcoord[0]) + p->draw_offset.y;
    }
    Player_ClampToStage(p);
    Camera_Update();
    Player_UpdateDrawParams(p, 1);
    /* the targets this player binds (TargetBind) */
    if (p->targets->count != 0) {
        for (t = KeyedArray_First(p->targets); t != NULL; t = KeyedArray_Next(p->targets)) {
            Player *tp = t->player;
            t->bind_active = t->bind_time;
            if (tp->movetype == MOVETYPE_HIT && t->bind_time != 0) {
                tp->pos.x = (double)(p->facing * t->bind_x) + p->pos.x;
                tp->pos.y = (double)t->bind_y + p->pos.y;
                tp->vel = p->vel;
                tp->gethit_yoff = 0.0f;
                tp->gethit_xoff = 0.0f;
                tp->gethit_isbound = p;
                if (tp->pos_freeze <= 0) {
                    tp->pos_freeze = 1;
                }
                if (t->bind_time > 0) {
                    t->bind_time--;
                }
            }
        }
    }
    p->update_frame = g_game->frames;
}

/*
 * 0x0043D650: BindToParent / BindToRoot: places the player relative to the
 * player in bind_to while the binding lasts; the binding ends when that
 * player is gone.
 */
void Player_UpdateBind(Player *p)
{
    int time = p->bind_time;
    Player *o;
    float x, y;
    int facing;

    if (time == 0) {
        return;
    }
    if (p->bind_to.valid != 1 || (o = g_game->players[p->bind_to.index]) == NULL || o->active == 0 ||
        o->id != p->bind_to.id || !PlayerRef_GetPos(&p->bind_to.id, &x, &y, &facing)) {
        p->bind_time = 0;
        p->bind_y = 0.0f;
        p->bind_facing = 0;
        p->bind_x = 0.0f;
        p->unused_247C = 0;
        p->bind_to.valid = 0;
        return;
    }
    if (p->bind_facing != 0) {
        p->facing = p->bind_facing * facing;
    }
    p->pos.x = (double)p->bind_x * facing + x;
    p->pos.y = (double)p->bind_y + y;
    if (time > 0) {
        p->bind_time = time - 1;
    }
    Player_ClampToStage(p);
    Camera_Update();
    Player_UpdateDrawParams(p, 1);
}

/*
 * 0x0043D750: ScreenBound: keeps the player inside the screen, with the edge
 * widths of the Width controller rounded to whole pixels; also stores the
 * screen edges in screen_left / screen_right.
 */
void Player_ScreenBound(Player *p)
{
    GameVars *g;
    int facing;
    double unit;
    float f;
    int left, right;

    if (p->screenbound == 0) {
        return;
    }
    g = g_game;
    if (g->game_type_m == 1 && p->alive == 0) {
        return;
    }
    facing = p->facing;
    unit = g_gameCoord[0] / p->data->localcoord[0];
    f = (float)(unit * (facing == -1 ? p->width_edge_front : p->width_edge_back));
    f = (float)floor(f + 0.5);
    left = Ftol(f);
    f = (float)((facing == 1 ? p->width_edge_front : p->width_edge_back) * unit);
    f = (float)floor(f + 0.5);
    right = Ftol(f);
    p->screen_left = (float)((double)g->stage.screenleft * (g_gameCoord[0] / g->stage.unit) + g->screen_left_target);
    p->screen_right = (float)(g->screen_right_target - (double)g->stage.screenright * (g_gameCoord[0] / g->stage.unit));
    if ((double)left + p->screen_left > p->pos.x) {
        p->pos.x = (double)left + p->screen_left;
    }
    if (p->screen_right - (double)right < p->pos.x) {
        p->pos.x = p->screen_right - (double)right;
    }
}

/*
 * 0x0043D8C0: keeps the player between the stage's player bounds (always for
 * characters before mugenversion 1.1, else with ScreenBound); not for KOed
 * players in the "M" game type.
 */
void Player_ClampToStage(Player *p)
{
    GameVars *g = g_game;
    double v;

    if (p->data->mugenversion >= g->compat_version[2] && p->screenbound == 0) {
        return;
    }
    if (g->game_type_m == 1 && p->alive == 0) {
        return;
    }
    v = (double)g->stage.leftbound * (g_gameCoord[0] / g->stage.unit);
    if (v > p->pos.x) {
        p->pos.x = v;
    }
    v = (double)g->stage.rightbound * (g_gameCoord[0] / g->stage.unit);
    if (v < p->pos.x) {
        p->pos.x = v;
    }
}

/* ------------------------------------------------------------------ hits */

/* The HitDef (or ReversalDef) of a player. */

/* -FLT_MAX: HitDef velocities that keep the current one. */
#define PS_NEG_FLT_MAX      (-3.402823466e+38F)

/* HitDef.mindist / maxdist "not given". */
#define HITDEF_NO_DIST      (-1025)

/* The player whose localcoord the state owner uses: the custom state owner, or the player itself. */
static const CharData *Player_StateOwnerData(const Player *p)
{
    if (p->custom_states != NULL && p->custom_owner != -1 && g_game->players[p->custom_owner] != NULL) {
        return g_game->players[p->custom_owner]->data;
    }
    return p->data;
}

/* Character units to game units in the state owner's localcoord (Player_FromLocalCoord without the multiplication). */
static double Player_StateUnit(const Player *p)
{
    return g_gameCoord[0] / Player_StateOwnerData(p)->localcoord[0];
}

/* (a - b) as an int, for the Clsn tests: the positions plus the draw offsets (draw_offset). */
static int PS_DeltaX(const Player *a, const Player *b)
{
    return Ftol((a->draw_offset.x + a->pos.x) - (b->draw_offset.x + b->pos.x));
}

static int PS_DeltaY(const Player *a, const Player *b)
{
    return Ftol((a->draw_offset.y + a->pos.y) - (b->draw_offset.y + b->pos.y));
}

/* x * scale rounded to a whole number as the original does: through float, floor(x + 0.5), float again. */
static int PS_RoundWidth(double scaled)
{
    float f = (float)scaled;
    f = (float)floor(f + 0.5);
    return Ftol(f);
}

/*
 * 0x0043D950: collisions of the tick. Collects the active players with
 * collision boxes or live projectiles, then for every pair of different
 * teams: pushing, reversals, which HitDefs connect (priorities), the hits
 * (the order of the targets alternates every frame), projectiles against
 * HitDefs, players and other projectiles, and pushing again.
 */
void Players_CheckHits(void)
{
    GameVars *g = g_game;
    Player **list = g->hit_players;
    Player **pa, **pb;
    int n = 0;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        int has_clsn = 0;
        AnimElem *e;
        if (p == NULL || p->active == 0) {
            continue;
        }
        e = p->anim->elem;
        if (e != NULL && (e->clsn1 != NULL || e->clsn2 != NULL)) {
            has_clsn = 1;
        }
        if (Projectiles_Count(p->projectiles) > 0 || has_clsn) {
            list[n++] = p;
        }
    }
    list[n] = NULL;
    g->hit_player_count = n;
    for (pa = list; *pa != NULL; pa++) {
        if ((*pa)->nohit_time > 0) {
            (*pa)->nohit_time--;
        }
        (*pa)->hitdef_juggle = 0;
    }
    /* pushing */
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            if ((*pa)->team_side != (*pb)->team_side && (*pa)->team_side != 0 && (*pb)->team_side != 0) {
                Players_Push(*pa, *pb);
            }
        }
    }
    /* reversals */
    list = g_game->hit_players;
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            Player *a = *pa, *b = *pb;
            if (a->team_side != b->team_side && a->team_side != 0 && b->team_side != 0) {
                if (Player_CanReverse(a, b)) {
                    Player_Reverse(a, b);
                }
                if (Player_CanReverse(b, a)) {
                    Player_Reverse(b, a);
                }
            }
        }
    }
    /* which HitDefs connect: hit_pending[attacker index] of the target */
    list = g_game->hit_players;
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            Player *a = *pa, *b = *pb;
            int v = 0;
            if (Player_CanHit(a, b)) {
                if (!Player_CanHit(b, a) || a->hitdef.priority > b->hitdef.priority ||
                    (a->hitdef.priority == b->hitdef.priority &&
                     a->hitdef.priority_type == 0 && b->hitdef.priority_type != 2)) {
                    v = 1;
                }
            }
            b->hit_pending[a->index] = v;
            v = 0;
            if (Player_CanHit(b, a)) {
                if (!Player_CanHit(a, b) || b->hitdef.priority > a->hitdef.priority ||
                    (b->hitdef.priority == a->hitdef.priority &&
                     b->hitdef.priority_type == 0 && a->hitdef.priority_type != 2)) {
                    v = 1;
                }
            }
            a->hit_pending[b->index] = v;
        }
    }
    /* the hits: targets in list order on odd frames, in reverse order on even frames */
    g = g_game;
    if (g->frames & 1) {
        for (pb = list; *pb != NULL; pb++) {
            Player *t = *pb;
            for (pa = g_game->hit_players; *pa != NULL; pa++) {
                Player *a = *pa;
                if (t->hit_pending[a->index] != 0) {
                    Player_Hit(a, t);
                    t->hit_pending[a->index] = 0;
                }
            }
        }
    } else {
        for (pb = &g->hit_players[g->hit_player_count - 1]; pb >= list; pb--) {
            Player *t = *pb;
            for (pa = list; *pa != NULL; pa++) {
                Player *a = *pa;
                if (t->hit_pending[a->index] != 0) {
                    Player_Hit(a, t);
                    t->hit_pending[a->index] = 0;
                }
            }
            list = g_game->hit_players;
        }
    }
    /* HitDefs against projectiles */
    list = g_game->hit_players;
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            Player_HitProjectiles(*pa, *pb);
            Player_HitProjectiles(*pb, *pa);
        }
    }
    /* projectiles against players */
    list = g_game->hit_players;
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            Player *a = *pa, *b = *pb;
            if ((b->hitby_attr & b->nothitby_attr) != 0) {
                Projectiles_HitPlayer(a, b);
            }
            if ((a->hitby_attr & a->nothitby_attr) != 0) {
                Projectiles_HitPlayer(b, a);
            }
        }
    }
    list = g_game->hit_players;
    for (pa = list; *pa != NULL; pa++) {
        Projectiles_ApplyHits(*pa);
    }
    /* projectiles against projectiles */
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            Projectiles_Collide(*pa, *pb);
        }
    }
    /* pushing again */
    for (pa = list; *pa != NULL; pa++) {
        for (pb = pa + 1; *pb != NULL; pb++) {
            if ((*pa)->team_side != (*pb)->team_side && (*pa)->team_side != 0 && (*pb)->team_side != 0) {
                Players_Push(*pa, *pb);
            }
        }
    }
}

/*
 * 0x0043DE40: PlayerPush between two players of different teams whose
 * collision boxes (Clsn2) overlap: pushes them apart (alive players, or any
 * outside of the "M" game type) and applies the stage and screen bounds.
 */
void Players_Push(Player *a, Player *b)
{
    int flip_a = a->facing != 1;
    int flip_b = b->facing != 1;
    const Clsn *clsn_a = a->anim->elem->clsn2;
    const Clsn *clsn_b = b->anim->elem->clsn2;
    int dx = PS_DeltaX(b, a);
    int dy = PS_DeltaY(b, a);

    if (a->playerpush == 0 || b->playerpush == 0 || !Clsn_Collide(flip_a, flip_b, clsn_a, clsn_b, dx, dy)) {
        return;
    }
    if ((a->alive != 0 && b->alive != 0) || g_game->game_type_m != 1) {
        Players_PushApart(a, b);
    }
    Player_ClampToStage(a);
    Player_ClampToStage(b);
    if (a->screenbound != 0) {
        Player_ScreenBound(a);
    }
    if (b->screenbound != 0) {
        Player_ScreenBound(b);
    }
}

/*
 * 0x0043DF60: moves two overlapping players apart by their widths (the
 * front of the left one, the back or front of the right one). A standing
 * player that is more than its height below the other is left alone. Players
 * closer than 10 pixels (in 320 units) are first set 5 pixels each side of
 * their middle.
 */
void Players_PushApart(Player *a, Player *b)
{
    Player *l = a, *r = b;
    int wl, wr;
    float dist, limit;

    if (a->statetype == STATETYPE_STAND &&
        g_gameCoord[0] / a->data->localcoord[0] * a->consts.height < a->pos.y - b->pos.y) {
        return;
    }
    if (b->statetype == STATETYPE_STAND &&
        g_gameCoord[0] / b->data->localcoord[0] * b->consts.height < b->pos.y - a->pos.y) {
        return;
    }
    /* l: the player on the left */
    if (b->pos.x < a->pos.x) {
        l = b;
        r = a;
    } else if (b->pos.x == a->pos.x) {
        int swap = 0;
        if (a->facing != b->facing) {
            swap = b->facing == 1;
        } else if (a->facing == 1) {
            swap = b->movetype == MOVETYPE_ATTACK || b->statetype == STATETYPE_AIR;
        } else if (a->facing == -1) {
            swap = a->movetype == MOVETYPE_ATTACK || a->statetype == STATETYPE_AIR;
        }
        if (swap) {
            l = b;
            r = a;
        }
    }
    {
        int facing = l->facing;
        int base, extra;
        if (l->statetype == STATETYPE_AIR) {
            base = facing == 1 ? l->consts.air_front : l->consts.air_back;
        } else {
            base = facing == 1 ? l->consts.ground_front : l->consts.ground_back;
        }
        extra = facing == 1 ? l->width_player_front : l->width_player_back;
        wl = PS_RoundWidth(g_gameCoord[0] / l->data->localcoord[0] * (extra + base));
    }
    {
        int facing = r->facing;
        int base, extra;
        if (r->statetype == STATETYPE_AIR) {
            base = facing == -1 ? r->consts.air_front : r->consts.air_back;
        } else {
            base = facing == -1 ? r->consts.ground_front : r->consts.ground_back;
        }
        extra = facing == -1 ? r->width_player_front : r->width_player_back;
        wr = PS_RoundWidth(g_gameCoord[0] / r->data->localcoord[0] * (extra + base));
    }
    dist = (float)(r->pos.x - l->pos.x);
    limit = (float)((double)Ftol(g_game->game_coord[0]) * 10.0 / 320.0);
    if (limit > dist) {
        float mid = (float)((r->pos.x + l->pos.x) * 0.5);
        float off = (float)((double)Ftol(g_game->game_coord[0]) * 5.0 / 320.0);
        l->pos.x = (double)mid - off;
        off = (float)(5.0 * Ftol(g_game->game_coord[0]) / 320.0);
        r->pos.x = (double)mid + off;
    }
    {
        float width = (float)(wl + wr);
        if (width > dist) {
            float half = (float)(((double)width - dist) * 0.5);
            l->pos.x = l->pos.x - half;
            r->pos.x = half + r->pos.x;
        }
    }
}

/*
 * 0x0043E280: whether att's HitDef can hit tgt this tick: active HitDef of an
 * attacking player (not a ReversalDef), hittable target of an affected team,
 * chain IDs, not already hit by this HitDef, Clsn1 against Clsn2, juggle
 * points, hit flags and HitBy attributes. Sets att's juggle cost and the
 * state type bits of its HitDef attributes.
 */
int Player_CanHit(Player *att, Player *tgt)
{
    int flip_a = att->facing != 1;
    int flip_t = tgt->facing != 1;
    const Clsn *clsn_a = att->anim->elem->clsn1;
    const Clsn *clsn_t = tgt->anim->elem->clsn2;
    HitDef *hd = &att->hitdef;
    TargetEntry *entry;
    int ok;
    int dx, dy;

    if (hd->active == 0 || att->movetype == MOVETYPE_IDLE || hd->reversal_attr != 0) {
        return 0;
    }
    if (tgt->nohit_time > 0 || (tgt->hitby_attr & tgt->nothitby_attr) == 0) {
        return 0;
    }
    if (att->team_side == 2) {
        ok = HitDef_AffectsTeam(hd, tgt->team_side);
    } else {
        ok = (hd->affectteam & (unsigned int)tgt->team_side) != 0;
    }
    if (!ok) {
        return 0;
    }
    if (hd->chainid != -1 && (tgt->gethit_id != hd->chainid || tgt->gethit_attacker != att)) {
        return 0;
    }
    if (hd->nochainid[0] != -1 && tgt->gethit_attacker == att) {
        if (tgt->gethit_id == hd->nochainid[0]) {
            return 0;
        }
        if (hd->nochainid[1] != -1 && tgt->gethit_id == hd->nochainid[1]) {
            return 0;
        }
    }
    entry = Targets_FindPlayer(att->targets, tgt);
    if (entry != NULL && entry->hit_id == hd->serial) {
        return 0;
    }
    if (tgt->hit_ids[att->index] == hd->serial) {
        return 0;
    }
    dy = PS_DeltaY(tgt, att);
    dx = PS_DeltaX(tgt, att);
    if (!Clsn_Collide(flip_a, flip_t, clsn_a, clsn_t, dx, dy)) {
        return 0;
    }
    if (att->nojugglecheck == 0) {
        if (entry != NULL && hd->juggle != -1 && (int)((unsigned int)entry->juggle - (unsigned int)hd->juggle) < 0) {
            return 0;
        }
        att->hitdef_juggle = hd->juggle;
    }
    if (!Player_HitFlagMatches(tgt, hd->hitflag)) {
        return 0;
    }
    HitDef_SetStateAttr(att);
    return Player_HitByAllows(tgt, hd->attr) != 0;
}

/*
 * Position of a hit spark: sparkxy from the front edge of the target (the
 * back edge when both face the same way) and the attacker's height.
 */
static void Player_SparkPos(Player *att, Player *tgt, const HitDef *hd, int *out_x, int *out_y)
{
    int tgt_facing = tgt->facing;
    double base;
    int w;

    if (tgt_facing != att->facing) {
        double sx = (double)hd->sparkxy[0] * Player_StateUnit(att);
        w = tgt->statetype == STATETYPE_AIR ? tgt->consts.air_front : tgt->consts.ground_front;
        base = (double)PS_RoundWidth(g_gameCoord[0] / tgt->data->localcoord[0] * w) + sx;
    } else {
        double sx = (double)-hd->sparkxy[0] * Player_StateUnit(att);
        if (tgt->statetype == STATETYPE_AIR) {
            w = tgt_facing == -tgt_facing ? tgt->consts.air_front : tgt->consts.air_back;
        } else {
            w = tgt_facing == -tgt_facing ? tgt->consts.ground_front : tgt->consts.ground_back;
        }
        base = sx - (double)PS_RoundWidth(g_gameCoord[0] / tgt->data->localcoord[0] * w);
    }
    *out_x = Ftol(((double)Ftol(base) - tgt->draw_offset.x) * tgt_facing + tgt->pos.x);
    *out_y = Ftol((double)hd->sparkxy[1] * Player_StateUnit(att) - att->draw_offset.y + att->pos.y);
}

/* A target entry for tgt in att's target list (added with att's airjuggle points if missing), or NULL. */
static TargetEntry *Player_AddTarget(Player *att, Player *tgt)
{
    TargetEntry *entry = Targets_FindPlayer(att->targets, tgt);

    if (entry == NULL) {
        TargetEntry e;
        int k;
        memset(&e, 0, sizeof(e));
        e.player = tgt;
        e.juggle = att->consts.airjuggle;
        k = KeyedArray_Add(att->targets, &e, 0, 0);
        if (k == -1) {
            return NULL;
        }
        entry = (TargetEntry *)(att->targets->data + att->targets->elem_size * k);
    }
    return entry;
}

/* The power of a player's team after a hit (Player_SetPower inlined by Player_Reverse: added then clamped). */
static void Player_AddPower(Player *p, int amount)
{
    GameVars *g = g_game;

    if (g->round_state != 3 && g->round_state != 4) {
        *p->power_ref += amount;
        if (*p->power_ref < 0) {
            *p->power_ref = 0;
        }
        if (*p->power_ref > *p->power_max_ref) {
            *p->power_ref = *p->power_max_ref;
        }
    }
    if (g->debug_full_power != 0) {
        *p->power_ref = *p->power_max_ref;
    }
}

/* The attacker is pushed back by the cornerpush velocity when the target is at a screen edge. */
static void Player_CornerPush(Player *att, Player *tgt)
{
    Coord c;

    c.x = (double)tgt->gethit_cornerpush * att->facing;
    c.y = 0.0;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;               /* not set in the original (stack garbage) */
    att->push_time = -1;
    att->push_decay_x = 0.7f;
    att->push_vel = c;
    att->push_min_speed = 1.0f;
}

/*
 * 0x0043E480: att's HitDef hits tgt (Player_CanHit said so): guard or hit,
 * HitOverride, the target entry, sprite priorities, the spark, facing
 * changes, HitDef_Apply, pause times, power, hit counters, palfx, throws,
 * p1stateno / p2stateno / p2getp1state, the snap binding and the corner push.
 */
void Player_Hit(Player *att, Player *tgt)
{
    HitDef *hd = &att->hitdef;
    int dir = att->facing;
    int flip = att->facing != 1;
    int wins = (&g_game->round_no)[att->team_side];
    int ko = 0;
    int guarded = 0;
    int overridden = 0;
    HitOverride *ov = NULL;
    TargetEntry *entry = NULL;
    int spark_x, spark_y;
    int hit;
    int i;

    if (tgt->nohit_time > 0) {
        return;
    }
    if (hd->hitonce != 0) {
        if (hd->active == 0) {
            return;
        }
        Targets_Release(att, tgt);
    }
    if (tgt->in_guard_dist != 0 && Player_HitFlagMatches(tgt, hd->guardflag) && att->unguardable == 0) {
        int dmg = Player_HitDamage(tgt, hd->guard_damage, hd->guard_kill);
        if ((int)((unsigned int)tgt->life - tgt->gethit_damage - dmg) > 0) {
            guarded = 1;
        } else if (g_game->guard_ko != 0) {
            ko = 1;
            if (g_game->round_state != 3 && wins < 10) {
                Round_SetWinType(att->team_side, WIN_ICON_C, 0);
            }
        }
    } else {
        int dmg = Player_HitDamage(tgt, hd->damage, hd->kill);
        if ((int)((unsigned int)tgt->life - tgt->gethit_damage - dmg) <= 0 && tgt->helper != 1) {
            ko = 1;
            Round_SetKoWinType(att->team_side, tgt->team_side, hd->attr);
        }
    }
    if (g_game->debug_no_ko != 0) {
        ko = 0;
    }
    for (i = 0; i < 8; i++) {
        HitOverride *o = &tgt->hit_override[i];
        unsigned int m;
        if (o->active == 0) {
            continue;
        }
        m = o->attr & hd->attr;
        if ((m & HITATTR_STATES) != 0 && (m & ~HITATTR_STATES) != 0) {
            overridden = 1;
            guarded = 0;
            ko = 0;
            if (hd->p2getp1state != 0 || hd->p1stateno != -1) {
                return;
            }
            ov = o;
            break;
        }
    }
    if (!overridden) {
        entry = Player_AddTarget(att, tgt);
        if (entry == NULL) {
            return;
        }
        entry->bind_time = 0;
        entry->bind_y = 0;
        entry->bind_x = 0;
        entry->hit_id = hd->serial;
    }
    att->sprpriority = hd->p1sprpriority;
    tgt->sprpriority = hd->p2sprpriority;
    Player_SparkPos(att, tgt, hd, &spark_x, &spark_y);

    if (guarded) {
        att->move_contact_time = 0;
        att->hitpause_next = hd->guard_p1_pausetime;
        att->move_contact = 1;
        hit = Player_Guard(tgt, att, dir, hd);
        Player_SetPower(att, *att->power_ref + hd->guard_getpower);
        Spark_Create(hd->guard_sparkno, att, spark_x, spark_y, hd->guard_spark_own, flip, 1);
    } else {
        if (!overridden) {
            if (tgt->gethit_fall != 0 || (hd->hitflag & 1) != 0 ||
                (hd->air_fall != 0 && tgt->statetype == STATETYPE_AIR)) {
                entry->juggle -= att->hitdef_juggle;
                hd->juggle = 0;
            }
            if (hd->p1facing != 0) {
                dir = -att->facing;
                att->next_facing = dir;
            }
            if (hd->p1getp2facing != 0) {
                if (hd->p1getp2facing == 1 ? dir != tgt->facing : dir == tgt->facing) {
                    dir = -att->facing;
                    att->next_facing = dir;
                }
            }
        }
        hit = HitDef_Apply(ov, tgt, att, hd, (float)att->pos.x, (float)att->pos.y, dir, ko);
        if (hit != 0 || overridden) {
            att->hitpause_next = hd->p1_pausetime;
            Player_SetPower(att, *att->power_ref + hd->getpower);
            att->combo_hits += hd->numhits;
            if (att->hit_frame < (int)g_game->frames) {
                att->hitcount += hd->numhits;
            }
            att->uniqhitcount += hd->numhits;
            att->move_contact = 2;
            att->move_contact_time = 0;
            Combo_AddHits(att->team_side, hd->numhits);
            if (!overridden) {
                if (hd->palfx[1] > 0 || hd->palfx[1] == -1) {
                    memcpy(&tgt->palfx_def, hd->palfx, sizeof(hd->palfx));
                }
                if ((hd->attr & HITATTR_THROWS) != 0) {
                    tgt->nohit_time = hd->p1_pausetime + 1;
                    att->nohit_time = hd->p1_pausetime + 1;
                }
            }
        }
        Spark_Create(hd->sparkno, att, spark_x, spark_y, hd->spark_own, flip, 0);
        if (!overridden) {
            if (hd->p1stateno != -1) {
                Player_ChangeState(hd->p1stateno, att);
            }
            if (hd->p2getp1state != 0) {
                Player_SetCustomState(tgt, &hd->custom);
                tgt->movetype = MOVETYPE_HIT;
            }
            if (hd->p2stateno != -1) {
                Targets_Release(tgt, NULL);
                Explods_RemoveOnHit(g_game->explods, tgt->id);
                Player_ChangeState(hd->p2stateno, tgt);
                tgt->ctrl = 0;
            }
            entry->bind_time = hd->snap_time;
            entry->bind_x = Ftol(Player_FromLocalCoord(att, (double)hd->maxdist[0]));
            entry->bind_y = Ftol(Player_FromLocalCoord(att, (double)hd->maxdist[1]));
        } else if (ov->stateno != -1) {
            Player_ChangeState(ov->stateno, tgt);
        }
    }
    if (hit != 0 && Player_AtScreenEdge(tgt)) {
        Player_CornerPush(att, tgt);
    }
    att->hit_frame = g_game->frames;
    if (hd->hitonce != 0) {
        hd->active = 0;
    }
}

/* Units_From320 as a float (the original stores its st0 result where noted). */
#define PS_UNITS(v)         Units_From320(v)

/* Extra hit velocity of a KO hit: x away from the attacker, y up (at least `ymin` up in 320 units). */
static void Player_KoVelocity(Player *tgt, float ymin)
{
    if (tgt->gethit_vel_x != 0.0f) {
        float sign = Math_Sign(tgt->gethit_vel_x);
        tgt->gethit_vel_x = (float)((double)PS_UNITS(2.5f) * sign + tgt->gethit_vel_x);
    }
    if (tgt->gethit_vel_y <= 0.0f) {
        double vy = tgt->gethit_vel_y;
        tgt->gethit_vel_y = (float)(vy - PS_UNITS(2.0f));
    }
    if (tgt->gethit_vel_y <= 0.0f) {
        float limit = PS_UNITS(ymin);
        if (limit < tgt->gethit_vel_y) {
            tgt->gethit_vel_y = limit;
        }
    }
}

/*
 * 0x0043ECE0: applies a HitDef to its target (from a player, a projectile or
 * a ReversalDef): the hit sound, the get hit state (5000/5010 ground, 5020
 * air, 5070 trip, 5080 down) with its velocities and times, fall
 * parameters, the KO velocity boost, the get hit animation type turned
 * around for hits from behind, the y acceleration, EnvShake, the hit pause,
 * the mindist / maxdist snaps and the damage. Returns 0 when the HitDef has
 * no get hit state for the target's state type (and is not a ReversalDef).
 */
int HitDef_Apply(HitOverride *ov, Player *tgt, Player *att, const HitDef *hd, float x, float y, int dir, int ko)
{
    GameVars *g;
    int ndir = -dir;
    int no_state = 1;
    int bounce = 1;
    int fall;
    int st;
    int v;
    double d, dy;

    tgt->gethit_cornerpush = 0.0f;
    if (ov != NULL && ov->forceair != 0) {
        tgt->statetype = STATETYPE_AIR;
    }
    if (hd->hitsound[2] == 0) {
        Sound_PlayPanned(att->data->snd, hd->hitsound[0], Ftol(tgt->draw.x), hd->hitsound[1], att->data->volumescale);
    } else {
        Sound_PlayPanned(g_game->fight.common_snd, hd->hitsound[0], Ftol(tgt->draw.x), hd->hitsound[1], 100.0);
    }
    if (hd->p2facing == -(tgt->facing * ndir)) {
        tgt->next_facing = -tgt->facing;
    }
    tgt->gethit_attacker = att;
    tgt->gethit_attr = hd->attr;
    if (hd->serial != -1) {
        tgt->hit_ids[att->index] = hd->serial;
    }
    tgt->gethit_id = hd->id;
    tgt->gethit_vel_y = 0.0f;
    tgt->gethit_vel_x = 0.0f;
    tgt->gethit_guarded = 0;
    tgt->gethit_groundtype = hd->ground_type;
    tgt->gethit_airtype = hd->air_type;
    if ((hd->hitflag & 1) != 0 || ko != 0) {
        tgt->gethit_fall = 1;
    } else if (hd->forcenofall != 0) {
        tgt->gethit_fall = 0;
    }
    fall = tgt->gethit_fall;
    tgt->gethit_animtype = fall == 0 ? hd->animtype : hd->fall_animtype;

    st = tgt->statetype;
    if (st == STATETYPE_STAND || st == STATETYPE_CROUCH) {
        /* on the ground */
        if (hd->ground_type == 0 || hd->reversal_attr != 0) {
            goto common;
        }
        tgt->prev_state_no = tgt->state_no;
        no_state = 0;
        if (hd->forcestand != 0 || ko != 0) {
            tgt->statetype = STATETYPE_STAND;
        }
        tgt->state_no = tgt->statetype != STATETYPE_STAND ? 5010 : 5000;
        if (hd->ground_type == 3) {
            tgt->state_no = 5070;
        }
        if (hd->ground_velocity[0] == PS_NEG_FLT_MAX) {
            tgt->gethit_vel_x = (float)tgt->vel.x;
        } else {
            tgt->gethit_vel_x = (float)(Player_FromLocalCoord(att, hd->ground_velocity[0]) * ndir);
        }
        if (hd->ground_velocity[1] == PS_NEG_FLT_MAX) {
            tgt->gethit_vel_y = (float)tgt->vel.y;
        } else {
            tgt->gethit_vel_y = (float)Player_FromLocalCoord(att, hd->ground_velocity[1]);
        }
        tgt->gethit_cornerpush = (float)Player_FromLocalCoord(att, hd->ground_cornerpush);
        if (ko != 0) {
            if (hd->ground_velocity[1] == 0.0f) {
                tgt->gethit_vel_x = (float)(tgt->gethit_vel_x * 0.66);
            }
            Player_KoVelocity(tgt, -6.0f);
        }
        if (fall != 0 && tgt->gethit_vel_y == 0.0f) {
            tgt->gethit_vel_y = -0.001f;
        }
        if (tgt->gethit_vel_y != 0.0f) {
            tgt->gethit_hittime = hd->air_hittime;
            goto common;
        }
        tgt->gethit_hittime = hd->ground_hittime;
        tgt->gethit_slidetime = hd->ground_slidetime;
        if (tgt->gethit_animtype == 3) {
            tgt->gethit_animtype = 2;
        }
    } else if (st == STATETYPE_LIEDOWN && tgt->pos.y == 0.0) {
        /* lying down */
        if (hd->ground_type == 0 || hd->reversal_attr != 0) {
            goto common;
        }
        tgt->prev_state_no = tgt->state_no;
        no_state = 0;
        if (hd->down_velocity[0] == PS_NEG_FLT_MAX) {
            tgt->gethit_vel_x = (float)tgt->vel.x;
        } else {
            tgt->gethit_vel_x = (float)Player_FromLocalCoord(att, (double)ndir * hd->down_velocity[0]);
        }
        /* original bug kept: the y velocity is "kept" when the x velocity is PS_NEG_FLT_MAX */
        if (hd->down_velocity[0] == PS_NEG_FLT_MAX) {
            tgt->gethit_vel_y = (float)tgt->vel.y;
        } else {
            tgt->gethit_vel_y = (float)Player_FromLocalCoord(att, hd->down_velocity[1]);
        }
        tgt->gethit_cornerpush = (float)Player_FromLocalCoord(att, hd->down_cornerpush);
        tgt->state_no = 5080;
        if (tgt->gethit_vel_y == 0.0f) {
            tgt->gethit_hittime = hd->down_hittime;
            goto common;
        }
        tgt->gethit_yoff = (float)(PS_UNITS(15.0f) + tgt->gethit_yoff);
        bounce = hd->down_bounce;
        tgt->gethit_hittime = hd->air_hittime;
    } else {
        /* in the air */
        if (hd->air_type == 0 || hd->reversal_attr != 0) {
            goto common;
        }
        tgt->prev_state_no = tgt->state_no;
        tgt->state_no = 5020;
        no_state = 0;
        if (hd->air_type == 3) {
            tgt->state_no = 5070;
        }
        if (hd->air_velocity[0] == PS_NEG_FLT_MAX) {
            tgt->gethit_vel_x = (float)tgt->vel.x;
        } else {
            tgt->gethit_vel_x = (float)(Player_FromLocalCoord(att, hd->air_velocity[0]) * ndir);
        }
        if (hd->air_velocity[1] == PS_NEG_FLT_MAX) {
            tgt->gethit_vel_y = (float)tgt->vel.y;
        } else {
            tgt->gethit_vel_y = (float)Player_FromLocalCoord(att, hd->air_velocity[1]);
        }
        tgt->gethit_cornerpush = (float)Player_FromLocalCoord(att, hd->air_cornerpush);
        tgt->gethit_hittime = hd->air_hittime;
        if (hd->air_fall != 0) {
            tgt->gethit_fall = 1;
        }
        tgt->gethit_animtype = tgt->gethit_fall == 0 ? hd->air_animtype : hd->fall_animtype;
        if (ko != 0) {
            Player_KoVelocity(tgt, -3.0f);
        }
    }

common:
    /* hit from behind: back and up types swapped */
    {
        int facing = tgt->next_facing != 0 ? tgt->next_facing : tgt->facing;
        if (facing != (hd->p2facing != -1 ? 1 : -1) * ndir && tgt->gethit_animtype >= 0 && tgt->gethit_animtype <= 2) {
            if (tgt->gethit_groundtype == 1) {
                tgt->gethit_groundtype = 2;
            } else if (tgt->gethit_groundtype == 2) {
                tgt->gethit_groundtype = 1;
            }
            if (tgt->gethit_airtype == 1) {
                tgt->gethit_airtype = 2;
            } else if (tgt->gethit_airtype == 2) {
                tgt->gethit_airtype = 1;
            }
        }
    }
    if (no_state && hd->reversal_attr == 0) {
        return 0;
    }
    g = g_game;
    if (hd->yaccel == PS_NEG_FLT_MAX) {
        tgt->gethit_yaccel = (float)((double)Ftol(g->game_coord[0]) * (double)0.35f / 320.0);
    } else {
        tgt->gethit_yaccel = (float)Player_FromLocalCoord(att, hd->yaccel);
    }
    if (hd->envshake[1] != 0) {
        memcpy(g->envshake, hd->envshake, sizeof(g->envshake));
    }
    if (hd->fall_xvel == PS_NEG_FLT_MAX) {
        tgt->gethit_fall_xvel = hd->fall_xvel;
    } else {
        tgt->gethit_fall_xvel = (float)Player_FromLocalCoord(att, hd->fall_xvel);
    }
    tgt->gethit_fall_yvel = (float)(hd->fall_yvel * Player_StateUnit(att));
    tgt->gethit_fall_recover = hd->fall_recover;
    tgt->gethit_fall_time = 0;
    tgt->gethit_fall_recovertime = hd->fall_recovertime;
    tgt->gethit_fall_kill = hd->fall_kill;
    memcpy(&tgt->gethit_fall_envshake_unused, hd->fall_envshake, sizeof(hd->fall_envshake));    /* gethit_fall_envshake_unused .. _phase */
    if (bounce == 0) {
        tgt->gethit_fall_xvel = PS_NEG_FLT_MAX;
        tgt->gethit_fall_yvel = 0.0f;
    }
    tgt->gethit_fall_damage = hd->fall_damage;
    if (hd->reversal_attr != 0) {
        return 1;
    }
    Targets_Release(tgt, NULL);
    Explods_RemoveOnHit(g_game->explods, tgt->id);
    Sound_Stop(tgt->index + 1000);
    Player_ClearCustomState(tgt);
    if (tgt->movetype != MOVETYPE_HIT && tgt->hit_free_time > 1) {
        tgt->lifebar_red = tgt->life;
        tgt->lifebar_life = tgt->life;
        tgt->gethit_hitcount = 0;
    }
    tgt->gethit_hitcount++;
    g = g_game;
    tgt->in_guard_dist = 0;
    tgt->movetype = MOVETYPE_HIT;
    tgt->air_jumps = 0;
    tgt->state_time = 0;
    tgt->ctrl = 0;
    tgt->gethit_shaketime = hd->p2_pausetime;
    if (g->pause_kind != -1 && tgt->pause_movetime[g->pause_kind] < hd->p2_pausetime) {
        tgt->pause_movetime[g->pause_kind] = hd->p2_pausetime;
    }
    tgt->hitpause_next = 0;
    tgt->hitpause_time = 0;
    /* mindist / maxdist */
    v = hd->mindist[0];
    d = Player_FromLocalCoord(att, (double)v);
    if (fabs(tgt->pos.x - x) < d && v != HITDEF_NO_DIST) {
        tgt->gethit_xoff = (float)(((double)x - ndir * d - tgt->pos.x) + tgt->gethit_xoff);
    }
    v = hd->mindist[1];
    dy = tgt->pos.y - y;
    d = Player_FromLocalCoord(att, (double)v);
    if (d > dy && v != HITDEF_NO_DIST) {
        tgt->gethit_yoff = (float)((d + y - tgt->pos.y) + tgt->gethit_yoff);
    }
    v = hd->maxdist[0];
    d = Player_FromLocalCoord(att, (double)v);
    if (fabs(x - tgt->pos.x) > d && v != HITDEF_NO_DIST) {
        tgt->gethit_xoff = (float)(((double)x - ndir * d - tgt->pos.x) + tgt->gethit_xoff);
    }
    v = hd->maxdist[1];
    d = Player_FromLocalCoord(att, (double)v);
    if (d < dy && v != HITDEF_NO_DIST) {
        tgt->gethit_yoff = (float)((d + y - tgt->pos.y) + tgt->gethit_yoff);
    }
    tgt->gethit_damage += Player_HitDamage(tgt, hd->damage, hd->kill);
    if (ko != 0) {
        tgt->gethit_damage = tgt->life;
    }
    if (tgt->helper != 1 || tgt->helper_player == 1) {
        Player_SetPower(tgt, *tgt->power_ref + hd->givepower);
    }
    return 1;
}

/*
 * 0x0043F8D0: takes `damage` off the life (LifeAdd and TargetLifeAdd with the
 * value negated): scaled by the defence unless absolute, at least 1 point,
 * and not below 1 life unless kill.
 */
void Player_ApplyDamage(Player *p, int damage, int kill, int absolute)
{
    int v = -damage;
    int d = v;

    if (absolute == 0) {
        if (v < 0) {
            d = Ftol((double)p->defence_mul * v - 0.5);
        }
        if (v <= -1 && d == 0) {
            d = -1;
        }
    }
    d += p->life;
    if (d < 1 && kill == 0) {
        d = 1;
    }
    Player_SetLife(p, d);
}

/*
 * 0x0043F930: the damage a hit does to p (on top of the pending damage),
 * scaled by the defence: at least 1 for a positive damage, and leaving 1
 * life to a living player when the hit cannot kill.
 */
int Player_HitDamage(Player *p, int damage, int kill)
{
    int pending = p->gethit_damage;
    int d = Ftol((double)p->defence_mul * damage + 0.5) + pending;

    if (damage >= 1 && d == 0) {
        d = 1;
    }
    if (kill == 0) {
        int left = p->life - pending;
        if (left - d < 1 && p->alive != 0) {
            d = left - 1;
        }
    }
    return d - pending;
}

/*
 * 0x0043F990: tgt guards att's HitDef: guard sound, guard state (150, 152,
 * 154 by state type, without the state check), guard pause, velocities,
 * times and the guard damage. Fatal for a lying down target.
 */
int Player_Guard(Player *tgt, Player *att, int dir, const HitDef *hd)
{
    int ndir = -dir;
    int st;

    tgt->gethit_cornerpush = 0.0f;
    if (hd->guardsound[2] == 0) {
        Sound_PlayPanned(att->data->snd, hd->guardsound[0], Ftol(tgt->draw.x), hd->guardsound[1],
                         att->data->volumescale);
    } else {
        Sound_PlayPanned(g_game->fight.common_snd, hd->guardsound[0], Ftol(tgt->draw.x), hd->guardsound[1],
                         100.0);
    }
    tgt->gethit_attacker = att;
    tgt->gethit_attr = hd->attr;
    if (hd->serial != -1) {
        tgt->hit_ids[att->index] = hd->serial;
    }
    tgt->gethit_vel_y = 0.0f;
    tgt->gethit_id = hd->id;
    tgt->gethit_vel_x = 0.0f;
    tgt->gethit_guarded = 1;
    Player_ClearCustomState(tgt);
    st = tgt->statetype;
    switch (st) {
    case STATETYPE_STAND:
        tgt->state_no = 150;
        break;
    case STATETYPE_CROUCH:
        tgt->state_no = 152;
        break;
    case STATETYPE_AIR:
        tgt->state_no = 154;
        break;
    default:
        Error_Printf("Guard state error: wrong move type\n");
        Error_Exit();
    }
    tgt->gethit_shaketime = hd->guard_p2_pausetime;
    if (tgt->movetype != MOVETYPE_HIT) {
        tgt->lifebar_red = tgt->life;
        tgt->lifebar_life = tgt->life;
    }
    tgt->in_guard_dist = 1;
    tgt->movetype = MOVETYPE_HIT;
    tgt->air_jumps = 0;
    tgt->state_time = 0;
    tgt->ctrl = 0;
    tgt->hitpause_next = 0;
    tgt->hitpause_time = 0;
    if (st == STATETYPE_STAND || st == STATETYPE_CROUCH) {
        tgt->gethit_hittime = hd->guard_hittime;
        tgt->gethit_slidetime = hd->guard_slidetime;
        tgt->gethit_vel_x = (float)(hd->guard_velocity * Player_StateUnit(att) * ndir);
        tgt->gethit_ctrltime = hd->guard_ctrltime;
        tgt->gethit_cornerpush = (float)(hd->guard_cornerpush * Player_StateUnit(att));
    }
    if (st == STATETYPE_AIR) {
        tgt->gethit_vel_x = (float)(Player_FromLocalCoord(att, hd->airguard_velocity[0]) * ndir);
        tgt->gethit_vel_y = (float)Player_FromLocalCoord(att, hd->airguard_velocity[1]);
        tgt->gethit_ctrltime = hd->airguard_ctrltime;
        tgt->gethit_cornerpush = (float)Player_FromLocalCoord(att, hd->airguard_cornerpush);
    }
    tgt->gethit_damage += Player_HitDamage(tgt, hd->guard_damage, hd->guard_kill);
    if (tgt->helper != 1 || tgt->helper_player == 1) {
        Player_SetPower(tgt, *tgt->power_ref + hd->guard_givepower);
    }
    return 1;
}

/*
 * 0x0043FC60: whether rev's ReversalDef reverses att's HitDef: both active,
 * neither already hit by the other's current HitDef, Clsn1 against Clsn1
 * and the reversal attributes.
 */
int Player_CanReverse(Player *rev, Player *att)
{
    unsigned int rattr;
    unsigned int m;
    int dy, dx;
    int flip_rev, flip_att;

    if (rev->hitdef.active == 0 || att->hitdef.active == 0) {
        return 0;
    }
    rattr = rev->hitdef.reversal_attr;
    if (rattr == 0) {
        return 0;
    }
    if (att->hit_ids[rev->index] == rev->hitdef.serial ||
        rev->hit_ids[att->index] == att->hitdef.serial) {
        return 0;
    }
    flip_att = att->facing != 1;
    flip_rev = rev->facing != 1;
    dy = PS_DeltaY(att, rev);
    dx = PS_DeltaX(att, rev);
    if (!Clsn_Collide(flip_rev, flip_att, rev->anim->elem->clsn1, att->anim->elem->clsn1, dx, dy)) {
        return 0;
    }
    m = att->hitdef.attr & rattr;
    if ((m & HITATTR_STATES) == 0) {
        return 0;
    }
    return (m & ~HITATTR_STATES) != 0;
}

/*
 * 0x0043FD80: rev's ReversalDef reverses att's HitDef: target entry, sprite
 * priorities, the spark (from both sparkxy), HitDef_Apply of the
 * ReversalDef on att (no get hit state), pause times, power, hit counters
 * and the "reversed" move contact, palfx, p1stateno / p2stateno /
 * p2getp1state and the snap binding. att's HitDef is switched off.
 */
void Player_Reverse(Player *rev, Player *att)
{
    HitDef *hd = &rev->hitdef;
    int dir = rev->facing;
    int flip = att->facing != 1;
    int found = 0;
    int overridden;
    TargetEntry *entry = NULL;
    int spark_x, spark_y;
    double base;
    int i;

    if (hd->hitonce != 0) {
        if (hd->hitonce_used != 0) {
            return;
        }
        Targets_Release(rev, att);
        hd->hitonce_used = 1;
    }
    for (i = 0; i < 8; i++) {
        HitOverride *o = &att->hit_override[i];
        unsigned int m;
        if (o->active == 0) {
            continue;
        }
        m = o->attr & hd->attr;
        if ((m & HITATTR_STATES) != 0 && (m & ~HITATTR_STATES) != 0) {
            found = 1;
            break;
        }
    }
    overridden = found;
    if (hd->p2stateno == -1) {
        overridden = 0;
    }
    if (hd->p2stateno == -1 || !found) {
        entry = Player_AddTarget(rev, att);
        if (entry == NULL) {
            return;
        }
        entry->bind_time = 0;
        entry->bind_y = 0;
        entry->bind_x = 0;
        entry->hit_id = hd->serial;
    }
    rev->sprpriority = hd->p1sprpriority;
    att->sprpriority = hd->p2sprpriority;
    /* the spark: both sparkxy, from rev's edge */
    if (att->facing != dir) {
        double a = (double)att->hitdef.sparkxy[0] * Player_StateUnit(att);
        double b = (double)hd->sparkxy[0] * Player_StateUnit(rev);
        int w = rev->statetype == STATETYPE_AIR ? rev->consts.air_front : rev->consts.ground_front;
        base = (double)PS_RoundWidth(g_gameCoord[0] / rev->data->localcoord[0] * w) + (b + a);
    } else {
        double a = (double)-att->hitdef.sparkxy[0] * Player_StateUnit(att);
        double b = (double)hd->sparkxy[0] * Player_StateUnit(rev);
        int f = rev->facing;
        int w;
        if (rev->statetype == STATETYPE_AIR) {
            w = f == -f ? rev->consts.air_front : rev->consts.air_back;
        } else {
            w = f == -f ? rev->consts.ground_front : rev->consts.ground_back;
        }
        base = (a - b) - (double)PS_RoundWidth(g_gameCoord[0] / rev->data->localcoord[0] * w);
    }
    spark_x = Ftol(((double)Ftol(base) - rev->draw_offset.x) * rev->facing + rev->pos.x);
    spark_y = Ftol((double)att->hitdef.sparkxy[1] * Player_StateUnit(att) +
                      (double)hd->sparkxy[1] * Player_StateUnit(rev) - att->draw_offset.y + att->pos.y);
    if (!overridden) {
        HitDef_Apply(NULL, att, rev, hd, (float)rev->pos.x, (float)rev->pos.y, dir, 0);
        if (att->gethit_fall != 0 || (hd->hitflag & 1) != 0) {
            entry->juggle -= hd->juggle;
            hd->juggle = 0;
        }
        if (hd->p1facing != 0) {
            rev->next_facing = -rev->facing;
        }
    }
    rev->hitpause_next = hd->p1_pausetime;
    att->hitpause_next = hd->p2_pausetime;
    Player_AddPower(rev, hd->getpower);
    att->move_contact = 4;
    att->move_contact_time = 0;
    att->nothitby_attr = 0xFFF;
    att->hitby_attr = 0xFFF;
    att->hitby_time = 0;
    att->nothitby_time = 0;
    att->nohit_time = hd->p1_pausetime + 1;
    if ((hd->attr & HITATTR_THROWS) != 0) {
        rev->nohit_time = hd->p1_pausetime + 1;
    }
    rev->combo_hits += hd->numhits;
    rev->hitcount += hd->numhits;
    rev->uniqhitcount += hd->numhits;
    rev->move_contact = 2;
    rev->move_contact_time = 0;
    Combo_AddHits(rev->team_side, hd->numhits);
    if (!overridden && (hd->palfx[1] > 0 || hd->palfx[1] == -1)) {
        memcpy(&att->palfx_def, hd->palfx, sizeof(hd->palfx));
    }
    Spark_Create(hd->sparkno, rev, spark_x, spark_y, hd->spark_own, flip, 0);
    if (!overridden) {
        if (hd->p1stateno != -1) {
            Player_ChangeState(hd->p1stateno, rev);
        }
        if (hd->p2getp1state != 0) {
            Player_SetCustomState(att, &hd->custom);
            att->movetype = MOVETYPE_HIT;
        }
        if (hd->p2stateno != -1) {
            Targets_Release(att, NULL);
            Explods_RemoveOnHit(g_game->explods, att->id);
            Player_ChangeState(hd->p2stateno, att);
            att->ctrl = 0;
        }
        entry->bind_time = hd->snap_time;
        entry->bind_x = Ftol((double)hd->maxdist[0] * Player_StateUnit(rev));
        entry->bind_y = Ftol((double)hd->maxdist[1] * Player_StateUnit(rev));
    }
    att->hitdef.active = 0;
    if (hd->hitonce != 0) {
        hd->hitonce_used = 1;
    }
}

/*
 * 0x00441200: whether a hitflag / guardflag allows hitting p in its current
 * state: lying down (D), in a get hit state (+, and - only while guarding),
 * falling (F), and standing (H), crouching (L) or in the air (A).
 */
int Player_HitFlagMatches(Player *p, unsigned int flags)
{
    int st = p->statetype;

    if (st == STATETYPE_LIEDOWN && (flags & 0x40) == 0) {
        return 0;
    }
    if (p->movetype == MOVETYPE_HIT) {
        if ((flags & 2) == 0) {
            return 0;
        }
        if (p->in_guard_dist == 0) {
            goto falling;
        }
    }
    if ((flags & 0x100) != 0) {
        return 0;
    }
falling:
    if (p->gethit_fall != 0 && (flags & 0x20) == 0) {
        return 0;
    }
    switch (st) {
    case STATETYPE_STAND:
        return (flags & 4) != 0;
    case STATETYPE_CROUCH:
        return (flags & 8) != 0;
    case STATETYPE_AIR:
        return (flags & 0x10) != 0;
    case STATETYPE_LIEDOWN:
        return (flags & 0x40) != 0;
    default:
        return 0;
    }
}

/* 0x004412A0: whether the HitBy / NotHitBy attributes of p let a hit with attributes `attr` through. */
int Player_HitByAllows(Player *p, unsigned int attr)
{
    unsigned int m = p->nothitby_attr & p->hitby_attr & attr;

    if ((m & HITATTR_STATES) == 0) {
        return 0;
    }
    return (m & ~HITATTR_STATES) != 0;
}

/* 0x004412D0: the state type bits of p's HitDef attributes become those of p's current state type. */
void HitDef_SetStateAttr(Player *p)
{
    HitDef *hd = &p->hitdef;

    hd->attr &= ~HITATTR_STATES;
    switch (p->statetype) {
    case STATETYPE_STAND:
        hd->attr |= HITATTR_STAND;
        break;
    case STATETYPE_CROUCH:
        hd->attr |= HITATTR_CROUCH;
        break;
    case STATETYPE_AIR:
        hd->attr |= HITATTR_AIR;
        break;
    default:
        break;
    }
}

/*
 * 0x00441310: lets go of every target of p except `keep`: bound
 * characters (not helpers) drop to state 5050 out of the custom state.
 */
void Targets_Release(Player *p, Player *keep)
{
    TargetEntry *e;

    for (e = KeyedArray_First(p->targets); e != NULL; e = KeyedArray_Next(p->targets)) {
        Player *t = e->player;
        if (t == keep) {
            continue;
        }
        if (e->bind_active != 0 && t->helper == 0) {
            Player_ClearCustomState(t);
            Player_ChangeState(5050, t);
        }
        KeyedArray_Remove(p->targets->iter, p->targets);
    }
}

/*
 * 0x00441410: a hit spark at x, y (screen space): from the fight.def
 * sparks, or the character's own sprites and animations (keeping its palette
 * remapping); -2 is the character's default spark (or guard spark).
 * Returns the explod slot, or -1.
 */
int Spark_Create(int sparkno, Player *p, int x, int y, int own, int flip, int guard)
{
    GameVars *g;
    SysDrawItem *e;
    AnimState *st;
    KeyedArray *actions;
    Coord c;
    double scale;
    int idx;

    if (sparkno == -2) {
        sparkno = guard != 0 ? p->consts.guard_sparkno : p->consts.sparkno;
    }
    if (sparkno == -1) {
        return -1;
    }
    g = g_game;
    if (own != 0) {
        idx = Explods_Add(p->sprites, g->explods, sparkno, p->data->anim);
    } else {
        idx = Explods_Add(g->fightfx_sff, g->explods, sparkno, g->fightfx_anim);
    }
    if (idx == -1) {
        return -1;
    }
    e = &g->explods->items[idx];
    if (own != 0) {
        e->remap_own = *p->remap;
        e->remap = &e->remap_own;
    }
    e->id = 0;
    e->owner_id = 0;
    e->space = 0;
    e->flip = flip;
    c.x = (double)x;
    c.y = (double)y;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;               /* not set in the original (stack garbage) */
    e->postype = 3;
    e->layer = 2;
    e->pos = c;
    e->offset_x = (float)e->pos.x;
    e->offset_y = (float)e->pos.y;
    st = e->anim;
    actions = st->anim->actions;
    e->time = ((AnimAction *)(actions->data + actions->elem_size * st->action))->total_time;
    e->pause_movetime[1] = e->time;
    e->pause_movetime[0] = e->time;
    st->queue = NULL;
    if (own != 0) {
        scale = g_gameCoord[0] / p->data->localcoord[0];
    } else {
        scale = g->fightfx_scale * g->motif_scale;
    }
    c.x = scale;
    c.y = scale;
    c.z = 0.0;
    c.unit = 0.0;
    c.type = COORD_TYPE_2D;
    e->scale = c;
    return idx;
}

/*
 * 0x004415D0: every third tick of the round state, a sliding player (or one
 * landing, state 52) raises dust (fight.def action 120) at its feet, a
 * random pixel off.
 */
void Player_MakeDust(Player *p)
{
    GameVars *g = g_game;
    int dust = 0;
    int x, y, r, idx;
    SysDrawItem *e;
    AnimState *st;
    KeyedArray *actions;
    Coord c, out;

    if (g->round_state_time % 3 != 0) {
        return;
    }
    if (p->pos.y == 0.0 && p->statetype != STATETYPE_AIR && p->statetype != STATETYPE_LIEDOWN &&
        p->movetype != MOVETYPE_IDLE && p->movetype != MOVETYPE_ATTACK) {
        dust = 1;
    }
    if (p->state_no != 52 && !dust) {
        return;
    }
    r = Match_Random() % 3;
    x = Ftol(((double)r + p->draw.x) - 1.0);
    r = Match_Random() % 3;
    y = Ftol(((double)r + p->draw.y) - 1.0);
    idx = Explods_Add(g->fightfx_sff, g->explods, 120, g->fightfx_anim);
    if (idx == -1) {
        return;
    }
    e = &g->explods->items[idx];
    c.x = (double)x;
    c.y = (double)y;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;               /* not set in the original (stack garbage) */
    out = c;
    e->pos = *Coord_ScreenToGame(&out, c.x, c.y, c.z, c.unit, c.type, c.pad_24);
    e->space = 0;
    st = e->anim;
    actions = st->anim->actions;
    e->time = ((AnimAction *)(actions->data + actions->elem_size * st->action))->total_time;
    st->layer = 10;
    e->anim->queue = g->sprite_layer1;
    {
        double scale = g->fightfx_scale * g->motif_scale;
        c.x = scale;
        c.y = scale;
        c.z = 0.0;
        c.unit = 0.0;
        c.type = COORD_TYPE_2D;
        c.pad_24 = 0;           /* not set in the original (stack garbage) */
        e->scale = c;
    }
}

/* ------------------------------------------------------------------ drawing */

/* The renderer entry points the debug display uses (g_gfxPutPixel, g_gfxInvertPixel, g_gfxDrawLineAdd). */
typedef void (__cdecl *PS_PixelColorFn)(int x, int y, unsigned int color);
typedef void (__cdecl *PS_PixelInvertFn)(int x, int y);
typedef void (__cdecl *PS_LineFn)(float x1, float y1, float x2, float y2, unsigned int color);

/* Resets a DrawParams to the defaults (scale 1, alpha 1, all fields valid). */
static void PS_DrawDefaults(DrawParams *dp, unsigned int flags)
{
    memset(dp, 0, sizeof(*dp));
    dp->scale2_y = 1.0f;
    dp->flags = flags;
    dp->masked = 1;
    dp->scale2_x = 1.0f;
    dp->scale_y = 1.0f;
    dp->scale_x = 1.0f;
    dp->alpha_src = 1.0f;
    dp->bottom_stretch = 1.0f;
    dp->top_stretch = 1.0f;
    dp->width_ratio = 1.0f;
    dp->width_anchor = 0.5f;
}

/*
 * 0x00441820: the screen position, scale and facing of a player for this
 * tick (Player.draw, combined with the controllers' draw2), applying
 * ScreenBound first when asked; screen shake moves it.
 */
DrawParams *Player_UpdateDrawParams(Player *p, int bound)
{
    GameVars *g = g_game;
    DrawParams *dp = &p->draw;
    float half_w = (float)(Ftol(g->game_coord[0]) / 2);
    float cam_y = (float)(g_gameCoord[0] / g->stage.unit * g->camera_y + (double)g->camera_offset_y * g->camera_zoom);
    float top = (float)((double)g->draw_offset_y - cam_y);
    float zoom, s, cam_x;

    PS_DrawDefaults(dp, 0xFFFF);
    zoom = (float)((double)g->stage_scale * g->camera_zoom);
    p->draw_zoom = zoom;
    s = (float)(g_gameCoord[0] / p->data->localcoord[0] * p->consts.xscale);
    dp->scale_x = (float)((double)zoom * s);
    s = (float)(g_gameCoord[0] / p->data->localcoord[0] * p->consts.yscale);
    dp->scale_y = (float)((double)s * p->draw_zoom);
    if (bound != 0 && p->screenbound != 0) {
        Player_ScreenBound(p);
    }
    cam_x = (float)(g_gameCoord[0] / g->stage.unit * g->camera_x + g->camera_offset_x);
    dp->x = (float)(((p->draw_offset.x + p->pos.x) - cam_x) * p->draw_zoom + half_w);
    dp->y = (float)((p->draw_offset.y + p->pos.y) * p->draw_zoom + top);
    if (g->zoom_active != 0.0f) {
        dp->x = (float)(dp->x - (double)g->shake_x * g->stage_scale);
        dp->y = (float)(dp->y - (double)g->shake_y * g->stage_scale);
    }
    dp->flags = 0xF;
    dp->flip2 = p->facing != 1;
    return DrawParams_Combine(&p->draw2, dp);
}

/*
 * 0x00441A20: draws a player's sprite, scaled to the size of the custom
 * state owner's character when it uses that character's animations, and
 * shaking while in hit shake.
 */
void Player_Draw(Player *p)
{
    float sx = 1.0f, sy = 1.0f;
    DrawParams dp = p->draw;
    GameVars *g = g_game;
    Player *o;

    if (p->anim_owner != -1 && p->anim_from_owner != 0 && (o = g->players[p->anim_owner]) != NULL) {
        double ro = g_gameCoord[0] / o->data->localcoord[0];
        double rp = g_gameCoord[0] / p->data->localcoord[0];
        float a = (float)(o->consts.xscale * ro);
        float b = (float)(p->consts.xscale * rp);
        sx = (float)((double)a / b);
        a = (float)(o->consts.yscale * ro);
        b = (float)(p->consts.yscale * rp);
        sy = (float)((double)a / b);
    }
    if (p->movetype == MOVETYPE_HIT && p->gethit_shaketime > 0) {
        int w = Ftol(g->game_coord[0]);
        float shake = (float)((double)w * (p->state_time % 2) / 320.0);
        dp.x = (float)((double)shake * p->facing + dp.x);
    }
    p->anim->layer = p->sprpriority;
    g_palRemapCur = p->remap;
    g_palFxCur = p->palfx;
    Anim_Draw(p->anim, &dp, sx, sy);
    g_palFxCur = NULL;
    g_palRemapCur = NULL;
}

/* 0x00441B90: draws a player's shadow (stage shadow parameters a, b), scaled like Player_Draw. */
void Player_DrawShadow(Player *p, const int *a, const int *b)
{
    Player *o = p;
    double r, ro, rp;
    float sx, sy, off, y, num, den;

    if (p->anim_owner != -1 && p->anim_from_owner != 0) {
        o = g_game->players[p->anim_owner];
    }
    r = g_gameCoord[0] / p->data->localcoord[0];
    sx = (float)(p->consts.xscale * r);
    if (sx == 0.0f) {
        return;
    }
    sy = (float)(p->consts.yscale * r);
    if (sy == 0.0f) {
        return;
    }
    g_palRemapCur = p->remap;
    g_palFxCur = p->palfx;
    ro = g_gameCoord[0] / o->data->localcoord[0];
    rp = g_gameCoord[0] / p->data->localcoord[0];
    num = (float)(o->consts.yscale * ro);
    den = (float)(p->consts.yscale * rp);
    sy = (float)((double)num / den);
    num = (float)(o->consts.xscale * ro);
    den = (float)(p->consts.xscale * rp);
    sx = (float)((double)num / den);
    off = (float)(r * p->consts.shadowoffset);
    y = (float)p->pos.y;
    Shadow_Draw(p->anim, &p->draw, y, off, sx, sy, a, b);
    g_palFxCur = NULL;
    g_palRemapCur = NULL;
}

/*
 * 0x00441CF0: the debug display of a player: collision boxes (Clsn2 coloured
 * by hittability, Clsn1 red), the axis, the widths, the attributes the
 * player cannot be hit by ("MUTEKI" when none), name, ID, " H" with an
 * active HitDef, and the targets.
 */
void Player_DrawDebug(Player *p)
{
    AnimElem *elem = p->anim->elem;
    int flip = p->facing != 1;
    int x = Ftol(p->draw.x);
    int y = Ftol(p->draw.y);
    unsigned int m = p->nothitby_attr & p->hitby_attr;
    int muteki = 0;
    char text[0x40];
    char buf[0x100];
    const Clsn *c1, *c2;
    unsigned int color;
    float fx, fy, fyp, fym, fxl, fxr;
    int left, right, wl, wr, ia, ib, x0;

    if (elem == NULL) {
        return;
    }
    c1 = elem->clsn1;
    c2 = elem->clsn2;
    sprintf(buf, "%c%c%c|%c%c%c|%c%c%c|%c%c%c", (m & 0x001) ? ' ' : 'S', (m & 0x002) ? ' ' : 'C',
            (m & 0x004) ? ' ' : 'A', (m & 0x008) ? ' ' : 'N', (m & 0x010) ? ' ' : 'S', (m & 0x020) ? ' ' : 'H',
            (m & 0x040) ? ' ' : 'N', (m & 0x080) ? ' ' : 'S', (m & 0x100) ? ' ' : 'H', (m & 0x200) ? ' ' : 'N',
            (m & 0x400) ? ' ' : 'S', (m & 0x800) ? ' ' : 'H');
    if ((m & HITATTR_STATES) == 0 || p->nohit_time > 0 || (m & 0xFF8) == 0) {
        memcpy(buf, "MUTEKI", 7);
        muteki = 1;
    }
    if (m != 0xFFF || muteki) {
        Font_DrawText(buf, g_game->font_small, x - 0x1C, y - 2, 1, 0, 0x100, 0x100, 0xD4);
        color = Video_MapRGB(0, 0x7F, 0x7F);
    } else if (p->in_guard_dist != 0) {
        color = Video_MapRGB(0x40, 0, 0xC0);
    } else {
        color = Video_MapRGB(0, 0, 0xFF);
    }
    fy = (float)y;
    fx = (float)x;
    Clsn_Draw(c2, fx, fy, p->draw_zoom, p->draw_zoom, color, flip, g_game->clsn_draw_mode == 2);
    color = Video_MapRGB(0xFF, 0, 0);
    Clsn_Draw(c1, fx, fy, p->draw_zoom, p->draw_zoom, color, flip, g_game->clsn_draw_mode == 2);
    /* the axis */
    if (g_renderMode == 2 || g_renderMode == 3) {
        PS_PixelColorFn plot = (PS_PixelColorFn)g_gfxPutPixel;
        color = Video_MapRGB(0xC0, 0, 0);
        plot(x, y, color);
        plot(x - 1, y, color);
        plot(x + 1, y, color);
        plot(x, y - 1, color);
        plot(x, y + 1, color);
    } else {
        PS_PixelInvertFn invert = (PS_PixelInvertFn)g_gfxInvertPixel;
        invert(x, y);
        invert(x - 1, y);
        invert(x + 1, y);
        invert(x, y - 1);
        invert(x, y + 1);
    }
    /* the widths: body (ground or air) and the Width controller edges */
    {
        int back, front;
        if (p->statetype == STATETYPE_AIR) {
            back = p->consts.air_back;
            front = p->consts.air_front;
        } else {
            back = p->consts.ground_back;
            front = p->consts.ground_front;
        }
        if (p->facing == 1) {
            left = p->width_edge_back + back;
            wl = back + p->width_player_back;
            right = p->width_edge_front + front;
            wr = front + p->width_player_front;
        } else {
            left = p->width_edge_front + front;
            wl = front + p->width_player_front;
            right = p->width_edge_back + back;
            wr = back + p->width_player_back;
        }
    }
    {
        double r = g_gameCoord[0] / p->data->localcoord[0];
        PS_LineFn line = (PS_LineFn)g_gfxDrawLineAdd;
        ia = Ftol(wl * r);
        ib = Ftol(r * wr);
        fxr = (float)(ib + x);
        fxl = (float)(x - ia);
        color = Video_MapRGB(0x40, 0x7F, 0);
        line(fxl, fy, fxr, fy, color);
        fyp = (float)(y + 1);
        fym = (float)(y - 1);
        color = Video_MapRGB(0x40, 0x7F, 0);
        line(fxl, fym, fxl, fyp, color);
        color = Video_MapRGB(0x40, 0x7F, 0);
        line(fxr, fym, fxr, fyp, color);
        if (left != ia) {
            float xx = (float)(x - left);
            color = Video_MapRGB(0x7F, 0x40, 0);
            line(xx, fym, xx, fyp, color);
        }
        if (right != ib) {
            float xx = (float)(right + x);
            color = Video_MapRGB(0x7F, 0x40, 0);
            line(xx, fym, xx, fyp, color);
        }
        if (left > ia) {
            color = Video_MapRGB(0x7F, 0x40, 0);
            line(fxl, fy, (float)(x - left), fy, color);
        }
        if (right > ib) {
            color = Video_MapRGB(0x7F, 0x40, 0);
            line(fxr, fy, (float)(right + x), fy, color);
        }
    }
    /* name, ID and HitDef */
    sprintf(buf, "%s, %i", p->name, p->id);
    if (p->hitdef.active != 0) {
        memcpy(buf + strlen(buf), " H", 3);
    }
    x0 = x - 0x14;
    Font_DrawText(buf, g_game->font_small, x0, y + 6, 1, 0, 0x100, 0x100, 0x100);
    /* targets: "Target <id> <juggle>, ..." */
    if (p->targets->count > 0) {
        TargetEntry *e;
        memcpy(buf, "Target ", 8);
        e = KeyedArray_First(p->targets);
        while (e != NULL) {
            if (e->player != NULL && e->player->active != 0) {
                sprintf(text, "%i", e->player->id);
                strcat(buf, text);
            }
            sprintf(text, " %i", e->juggle);
            strcat(buf, text);
            e = KeyedArray_Next(p->targets);
            if (e == NULL) {
                break;
            }
            memcpy(buf + strlen(buf), ", ", 3);
        }
        Font_DrawText(buf, g_game->font_small, x0, y + 0xC, 1, 0, 0x100, 0xC0, 0x100);
    }
}

/* ------------------------------------------------------------------ palette groups (AllPalFX, BGPalFX) */

/* The group list of GameVars.palgroups. */
#define PS_PALGROUPS()  ((PalGroups *)g_game->palgroups)

/* Whether any of the five group masks overlap. */
static int PalGroups_Match(const unsigned int *a, const PalGroupEntry *e)
{
    int k;

    for (k = 0; k < 5; k++) {
        if ((a[k] & e->mask[k]) != 0) {
            return 1;
        }
    }
    return 0;
}

/* Resets a PalFx to the identity (as the group functions do it). */
static void PalFx_ResetSerial(PalFx *fx, unsigned int serial)
{
    fx->add[0] = 0;
    fx->add[1] = 0;
    fx->add[2] = 0;
    fx->unused_1c = 0;
    fx->invertall = 0;
    fx->serial = serial;
    fx->mul[2] = 0x100;
    fx->mul[1] = 0x100;
    fx->mul[0] = 0x100;
    fx->color = 0x100;
}

/*
 * 0x00442560: adds a PalFx to the palette groups with its group masks
 * (nothing when the same PalFx is already there with the same masks).
 * Counts the groups with only the stage bit (0x100) in GameVars.stage_palgroups.
 */
int PalGroups_Add(PalFx *fx, unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4)
{
    GameVars *g = g_game;
    PalGroups *pg = (PalGroups *)g->palgroups;
    int n = pg->count;
    int i;
    PalGroupEntry *e;

    if (fx == NULL) {
        return 0;
    }
    for (i = 0; i < pg->count; i++) {
        e = &pg->entries[i];
        if (fx == e->fx) {
            if (e->mask[0] == m0 && e->mask[1] == m1 && e->mask[2] == m2 && e->mask[3] == m3 && e->mask[4] == m4) {
                return 1;
            }
            n = pg->count;
        }
    }
    if (n >= pg->capacity) {
        size_t size = (size_t)(pg->capacity + 16) * sizeof(PalGroupEntry);
        PalGroupEntry *p;
        if (pg->entries == NULL) {
            p = calloc(1, size);
            if (p == NULL) {
                g_outOfMemory = 1;
            }
        } else {
            p = realloc(pg->entries, size);
        }
        if (p == NULL) {
            return 0;
        }
        memset(&p[pg->capacity], 0, 16 * sizeof(PalGroupEntry));
        pg->capacity += 16;
        pg->entries = p;
    }
    e = &pg->entries[n];
    e->fx = fx;
    e->mask[0] = m0;
    e->mask[1] = m1;
    e->mask[2] = m2;
    e->mask[3] = m3;
    e->mask[4] = m4;
    pg->count++;
    if (m0 == 0x100 && m1 == 0 && m2 == 0 && m3 == 0 && m4 == 0) {
        g->stage_palgroups += 1;
    }
    return 1;
}

/*
 * 0x00442760: builds the palette groups of a match: every player with group
 * masks, the stage (0x100) and the second global group (0x200).
 */
void PalGroups_Build(void)
{
    GameVars *g;
    PalGroupEntry *entries;
    int i;

    Match_FreeAssets();
    g = g_game;
    g->stage_palgroups = 0;
    entries = calloc(0x40, sizeof(PalGroupEntry));
    if (entries == NULL) {
        g_outOfMemory = 1;
    }
    ((PalGroups *)g->palgroups)->entries = entries;
    if (((PalGroups *)g->palgroups)->entries == NULL) {
        return;
    }
    ((PalGroups *)g->palgroups)->capacity = 0x40;
    for (i = 1; i <= g->player_slots; i++) {
        Player *p = g->players[i];
        int k;
        if (p == NULL) {
            continue;
        }
        for (k = 0; k < 5; k++) {
            if (p->pal_groups[k] != 0) {
                PalGroups_Add(&p->palfx_own, p->pal_groups[0], p->pal_groups[1], p->pal_groups[2], p->pal_groups[3],
                              p->pal_groups[4]);
                g = g_game;
                break;
            }
        }
    }
    PalGroups_Add(&g->stage_palfx, 0x100, 0, 0, 0, 0);
    PalGroups_Add(&g_game->palgroup_200_palfx, 0x200, 0, 0, 0, 0);
}

/* 0x004428E0: frees the palette group list (and empties it). */
void Match_FreeAssets(void)
{
    GameVars *g = g_game;
    PalGroups *pg = (PalGroups *)g->palgroups;

    if (pg->entries != NULL) {
        free(pg->entries);
        ((PalGroups *)g->palgroups)->entries = NULL;
        ((PalGroups *)g->palgroups)->count = 0;
        ((PalGroups *)g->palgroups)->capacity = 0;
    }
    pg = (PalGroups *)g->palgroups;
    pg->entries = NULL;
    pg->capacity = 0;
    pg->count = 0;
}

/* 0x00442940: resets the PalFx of every palette group. */
void PalGroups_ResetAll(void)
{
    PalGroups *pg = PS_PALGROUPS();
    unsigned int serial;
    int i;

    if (pg->count <= 0) {
        return;
    }
    serial = g_palFxSerial;
    for (i = 0; i < pg->count; i++) {
        PalFx_ResetSerial(pg->entries[i].fx, serial++);
    }
    g_palFxSerial = serial;
}

/* 0x004429A0: AllPalFX / BGPalFX add: adds r, g, b to the groups matching the masks. */
void PalGroups_AddColor(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int r,
                        int g, int b)
{
    PalGroups *pg = PS_PALGROUPS();
    unsigned int mask[5];
    unsigned int serial;
    int i;

    if (r == 0 && g == 0 && b == 0) {
        return;
    }
    if (pg->count <= 0) {
        return;
    }
    mask[0] = m0;
    mask[1] = m1;
    mask[2] = m2;
    mask[3] = m3;
    mask[4] = m4;
    serial = g_palFxSerial;
    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        if (PalGroups_Match(mask, e)) {
            PalFx *fx = e->fx;
            if (r != 0 || g != 0 || b != 0) {
                fx->add[1] += g;
                fx->add[0] += r;
                fx->add[2] += b;
                fx->serial = serial++;
            }
        }
    }
    g_palFxSerial = serial;
}

/* 0x00442A90: AllPalFX / BGPalFX mul (256 = 1.0). */
void PalGroups_MulColor(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int r,
                        int g, int b)
{
    PalGroups *pg = PS_PALGROUPS();
    unsigned int mask[5];
    unsigned int serial;
    int i;

    if (r == 0x100 && g == 0x100 && b == 0x100) {
        return;
    }
    if (pg->count <= 0) {
        return;
    }
    mask[0] = m0;
    mask[1] = m1;
    mask[2] = m2;
    mask[3] = m3;
    mask[4] = m4;
    serial = g_palFxSerial;
    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        if (PalGroups_Match(mask, e)) {
            PalFx *fx = e->fx;
            if (r != 0x100 || g != 0x100 || b != 0x100) {
                fx->mul[0] = fx->mul[0] * r >> 8;
                fx->mul[1] = fx->mul[1] * g >> 8;
                fx->serial = serial++;
                fx->mul[2] = fx->mul[2] * b >> 8;
            }
        }
    }
    g_palFxSerial = serial;
}

/* 0x00442BA0: AllPalFX / BGPalFX invertall: toggles the inversion. */
void PalGroups_Invert(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int invert)
{
    PalGroups *pg = PS_PALGROUPS();
    unsigned int mask[5];
    unsigned int serial;
    int i;

    if (invert == 0 || pg->count <= 0) {
        return;
    }
    mask[0] = m0;
    mask[1] = m1;
    mask[2] = m2;
    mask[3] = m3;
    mask[4] = m4;
    serial = g_palFxSerial;
    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        if (PalGroups_Match(mask, e)) {
            PalFx *fx = e->fx;
            fx->serial = serial++;
            fx->invertall = fx->invertall == 0;
        }
    }
    g_palFxSerial = serial;
}

/* 0x00442C60: AllPalFX / BGPalFX color (saturation, 256 = full). */
void PalGroups_SetColor(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int color)
{
    PalGroups *pg = PS_PALGROUPS();
    unsigned int mask[5];
    unsigned int serial;
    int i;

    if (color == 0x100) {
        return;
    }
    if (pg->count <= 0) {
        return;
    }
    mask[0] = m0;
    mask[1] = m1;
    mask[2] = m2;
    mask[3] = m3;
    mask[4] = m4;
    serial = g_palFxSerial;
    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        if (PalGroups_Match(mask, e)) {
            PalFx *fx = e->fx;
            fx->serial = serial++;
            fx->color = fx->color * color >> 8;
        }
    }
    g_palFxSerial = serial;
}

/* 0x00442D30: resets the PalFx of the groups matching the masks. */
void PalGroups_Reset(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4)
{
    PalGroups *pg = PS_PALGROUPS();
    unsigned int mask[5];
    unsigned int serial;
    int i;

    if (pg->count <= 0) {
        return;
    }
    mask[0] = m0;
    mask[1] = m1;
    mask[2] = m2;
    mask[3] = m3;
    mask[4] = m4;
    serial = g_palFxSerial;
    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        if (PalGroups_Match(mask, e)) {
            PalFx_ResetSerial(e->fx, serial++);
        }
    }
    g_palFxSerial = serial;
}

/* sin(time / period * pi * 2) rounded to float, as the original computes and stores it. */
static float PalGroups_Sine(int ticks, int divisor)
{
    return (float)sin((double)ticks / divisor * 3.141592653589793 * 2);
}

/*
 * 0x00442E10: applies an AllPalFX / BGPalFX definition: to the global PalFX
 * when `global` (AllPalFX of the whole screen), else to the palette groups
 * matching the masks (not while paused by defocus). "sinadd" adds a sine of
 * the time to the color offsets.
 */
void PalGroups_Apply(PalFxDef *def, unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3,
                     unsigned int m4, int global)
{
    GameVars *g = g_game;
    int r, gg, b;

    if (def->reset != 0) {
        if (global != 0) {
            g->all_palfx.add[0] = 0;
            g->all_palfx.add[1] = 0;
            g->all_palfx.add[2] = 0;
            g->all_palfx.unused_1c = 0;
            g->all_palfx.invertall = 0;
            g->all_palfx.mul[2] = 0x100;
            g->all_palfx.mul[1] = 0x100;
            g->all_palfx.mul[0] = 0x100;
            g->all_palfx.color = 0x100;
            g->all_palfx.serial = g_palFxSerial++;
        } else {
            PalGroups_Reset(m0, m1, m2, m3, m4);
        }
        def->reset = 0;
    }
    if (def->active == 0) {
        return;
    }
    r = def->add[0];
    gg = def->add[1];
    b = def->add[2];
    if (def->period != 0) {
        float s = PalGroups_Sine(def->time, def->period);
        r = Ftol((double)def->sinadd[0] * s + r);
        gg = Ftol((double)def->sinadd[1] * s + gg);
        b = Ftol((double)s * def->sinadd[2] + b);
    }
    if (global != 0) {
        if (r != 0 || gg != 0 || b != 0) {
            g_palFx.add[2] += b;
            g_palFx.add[0] += r;
            g_palFx.add[1] += gg;
            g_palFx.serial = g_palFxSerial++;
        }
        PalFx_ScaleGlobalMul(def->mul[0], def->mul[1], def->mul[2]);
        if (def->invertall != 0) {
            g_palFx.serial = g_palFxSerial++;
            g_palFx.invertall = g_palFx.invertall == 0;
        }
        if (def->color != 0x100) {
            g_palFx.color = g_palFx.color * def->color >> 8;
            g_palFx.serial = g_palFxSerial++;
        }
        return;
    }
    if (g->defocus_paused != 0) {
        return;
    }
    PalGroups_AddColor(m0, m1, m2, m3, m4, r, gg, b);
    PalGroups_MulColor(m0, m1, m2, m3, m4, def->mul[0], def->mul[1], def->mul[2]);
    PalGroups_Invert(m0, m1, m2, m3, m4, def->invertall);
    PalGroups_SetColor(m0, m1, m2, m3, m4, def->color);
}

/* ------------------------------------------------------------------ precache (select screen) */

/*
 * The precache thread loads the characters queued while the select screen
 * runs (GameVars.precache_*, guarded by precache_mutex): Precache_Start
 * starts the thread (or wakes it up), Precache_ThreadMain loads the queue
 * and finally lowers the cache count of what it loaded, Precache_Stop waits
 * for it to finish.
 */
#define PS_PRECACHE_MUTEX(g) ((pthread_mutex_t *)&(g)->precache_mutex)

/*
 * 0x004435F0: appends a character to the precache queue (4 entries, -1
 * terminated): 2 if it is already there, 0 when the queue is full.
 */
int Precache_QueueAdd(int *queue, int index)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (queue[i] == -1) {
            break;
        }
        if (queue[i] == index) {
            return 2;
        }
    }
    if (i == 4) {
        return 0;
    }
    queue[i] = index;
    queue[i + 1] = -1;
    return 1;
}

/* 0x004430C0: sets up the precache state; 0 when the mutex cannot be created. */
int Precache_Init(int enabled)
{
    GameVars *g = g_game;

    g->precache_enabled = enabled;
    if (pthread_mutex_init(PS_PRECACHE_MUTEX(g), NULL) != 0) {
        return 0;
    }
    g->precache_state = 0;
    g->precache_done = 0;
    g->precache_stop = 0;
    g->precache_queue[0] = -1;
    return 1;
}

/* 0x00443110: queues the characters of a team member's slots (with the lock held). */
int Precache_QueueTeam(TeamMember *m)
{
    int ok = 1;
    int i;

    if (g_game->precache_enabled == 0) {
        return 0;
    }
    pthread_mutex_lock(PS_PRECACHE_MUTEX(g_game));
    for (i = 0; i < m->count; i++) {
        if (m->slots[i].char_index != -1 && !Precache_QueueChar(m->slots[i].char_index)) {
            ok = 0;
            break;
        }
    }
    pthread_mutex_unlock(PS_PRECACHE_MUTEX(g_game));
    return ok;
}

/* 0x00443180: queues the next arcade opponents (at most as many as the opposing team has members). */
int Precache_QueueArcade(int skip_current)
{
    GameVars *g = g_game;
    int side = g->arcade_side;
    int chars[12];
    int n, limit, i;

    if (side == 0) {
        return 1;
    }
    n = Arcade_PeekNext(chars, skip_current);
    limit = g->teams[2 - side].members[0].count;
    if (n > limit) {
        n = limit;
    }
    for (i = 0; i < n; i++) {
        if (!Precache_QueueChar(chars[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * 0x004431F0: queues a character for precaching; a character already in the
 * cache only gets its release serial refreshed.
 */
int Precache_QueueChar(int char_index)
{
    GameVars *g = g_game;

    if (g->precache_enabled == 0) {
        return 0;
    }
    if (char_index >= 0 && char_index < g->char_count && g->char_info[char_index].data != NULL) {
        g->char_info[char_index].cache_serial = g_playerCacheSerial + 4;
        return 1;
    }
    return Precache_QueueAdd(g->precache_queue, char_index) != 0;
}

/*
 * 0x00443410: the precache thread: loads the queued characters while the
 * cache has room, then lowers the cache count of the ones it loaded.
 *
 * Original bug kept: after a load error the mutex is unlocked twice.
 */
void *Precache_ThreadMain(void *arg)
{
    GameVars *g = g_game;
    pthread_mutex_t *mutex = PS_PRECACHE_MUTEX(g);
    int loaded[5];
    int index = 0;
    int i;

    pthread_mutex_lock(mutex);
    loaded[0] = -1;
    g->precache_state = 1;
    pthread_mutex_unlock(mutex);
    while (g->precache_state == 1) {
        Sleep(1);
    }
    pthread_mutex_lock(mutex);
    g->precache_state = 3;
    while (g->precache_queue[0] != -1) {
        if (g->precache_queue[0] != -1) {
            index = g->precache_queue[0];
        }
        pthread_mutex_unlock(mutex);
        if (PlayerCache_HasRoom()) {
            if (PlayerCache_Load(index) == NULL) {
                Error_Printf("Error while precaching\n");
                pthread_mutex_lock(mutex);
                g->precache_done = 1;
                g->precache_queue[0] = -1;
                pthread_mutex_unlock(mutex);
                break;
            }
            {
                int known = 0;
                for (i = 0; i < 4; i++) {
                    if (loaded[i] == -1) {
                        break;
                    }
                    if (loaded[i] == index) {
                        known = 1;
                        break;
                    }
                }
                if (!known && i != 4) {
                    loaded[i] = index;
                    loaded[i + 1] = -1;
                }
            }
        }
        pthread_mutex_lock(mutex);
        if (g->precache_stop != 0) {
            g->precache_queue[0] = -1;
            break;
        }
        if (g->precache_queue[0] == -1) {
            break;
        }
        for (i = 0; i < 4; i++) {
            if (g->precache_queue[i] == -1) {
                break;
            }
            g->precache_queue[i] = g->precache_queue[i + 1];
        }
    }
    while (loaded[0] != -1) {
        CharInfo *info;
        index = loaded[0];
        for (i = 0; i < 4; i++) {
            if (loaded[i] == -1) {
                break;
            }
            loaded[i] = loaded[i + 1];
        }
        info = &g_game->char_info[index];
        if (info->cache_count > 0) {
            info->cache_count--;
            Log_Printf("Player %s cache count reduced: %i\n", (const char *)info->data, info->cache_count);
            if (info->cache_count == 0) {
                info->cache_serial = g_playerCacheSerial + 4;
                g_playerCacheSerial++;
            }
        }
    }
    g->precache_done = 1;
    g->precache_stop = 0;
    g->precache_state = 0;
    pthread_mutex_unlock(mutex);
    return NULL;
}

/*
 * 0x00443270: starts the precache thread (or lets the idle one go on); 0
 * when precaching is off, nothing is queued or the thread cannot start.
 *
 * Original bug kept: when pthread_attr_init fails the mutex stays locked.
 */
int Precache_Start(void)
{
    GameVars *g = g_game;
    pthread_mutex_t *mutex = PS_PRECACHE_MUTEX(g);
    pthread_attr_t attr;
    pthread_t thread;

    if (g->precache_enabled == 0) {
        return 0;
    }
    if (g->precache_state != 1 && g->precache_state != 0) {
        return 1;
    }
    pthread_mutex_lock(mutex);
    if (g->precache_queue[0] == -1) {
        pthread_mutex_unlock(mutex);
        return 0;
    }
    if (pthread_attr_init(&attr) != 0) {
        return 0;
    }
    if (g->precache_state == 0 && pthread_create(&thread, &attr, Precache_ThreadMain, NULL) != 0) {
        pthread_mutex_unlock(mutex);
        return 0;
    }
    pthread_attr_destroy(&attr);
    g->precache_running = 1;
    g->precache_done = 0;
    g->precache_stop = 0;
    pthread_mutex_unlock(mutex);
    while (g->precache_state != 1) {
        Sleep(0);
    }
    pthread_mutex_lock(mutex);
    g->precache_state = 2;
    pthread_mutex_unlock(mutex);
    return 1;
}

/* 0x00443360: asks a running precache thread to stop after the current character. */
void Precache_RequestStop(void)
{
    GameVars *g = g_game;

    pthread_mutex_lock(PS_PRECACHE_MUTEX(g));
    if (g->precache_running != 0) {
        g->precache_stop = 1;
    }
    pthread_mutex_unlock(PS_PRECACHE_MUTEX(g));
}

/* 0x004433A0: waits for a running precache thread to finish. */
void Precache_Stop(void)
{
    GameVars *g = g_game;
    pthread_mutex_t *mutex;

    if (g->precache_enabled == 0) {
        return;
    }
    mutex = PS_PRECACHE_MUTEX(g);
    pthread_mutex_lock(mutex);
    if (g->precache_running != 0) {
        while (g->precache_done == 0) {
            pthread_mutex_unlock(mutex);
            Sleep(5);
            pthread_mutex_lock(mutex);
        }
        g->precache_running = 0;
        g->precache_done = 0;
        g->precache_stop = 0;
    }
    pthread_mutex_unlock(mutex);
}

/* ------------------------------------------------------------------ projectiles */

/*
 * 0x00443630: a projectile list of `count` slots for a player (its
 * animations and sprites). The PalFx / remap reset of existing projectiles
 * never runs: the list is new.
 */
ProjectileList *Projectiles_Create(int count, Anim *anim, SpriteSet *sprites)
{
    ProjectileList *pl = calloc(1, sizeof(ProjectileList));
    Projectile *e;
    unsigned int serial;

    if (pl == NULL) {
        return NULL;
    }
    pl->list = KeyedArray_Create(count, sizeof(Projectile));
    if (pl->list == NULL) {
        Projectiles_Free(pl);
        return NULL;
    }
    pl->anim = anim;
    pl->sprites = sprites;
    if (pl->list->count == 0) {
        return pl;
    }
    serial = g_palFxSerial;
    for (e = KeyedArray_First(pl->list); e != NULL; e = KeyedArray_Next(pl->list)) {
        PalFx_ResetSerial(&e->palfx_own, serial++);
        memset(&e->remap_own, 0, sizeof(e->remap_own));
    }
    g_palFxSerial = serial;
    return pl;
}

/*
 * 0x00443740: frees a projectile list.
 *
 * Original bug kept: a list whose KeyedArray could not be created (from
 * Projectiles_Create) is dereferenced.
 */
void Projectiles_Free(ProjectileList *pl)
{
    KeyedArray *list;
    Projectile *e;

    if (pl == NULL) {
        return;
    }
    if (pl->list->count != 0) {
        for (e = KeyedArray_First(pl->list); e != NULL; e = KeyedArray_Next(pl->list)) {
            memset(&e->remap_own, 0, sizeof(e->remap_own));
        }
    }
    list = pl->list;
    if (list != NULL) {
        if (list->data != NULL) {
            free(list->data);
        }
        if (list->slots != NULL) {
            free(list->slots);
        }
        free(list);
    }
    free(pl);
}

/*
 * 0x004437E0: adds a projectile (a copy of `proj`) for player p, with p's
 * animation state started at action `action` of p's animations. Returns
 * the slot, or -1.
 */
int Projectiles_Add(ProjectileList *pl, Player *p, const Projectile *proj, int action)
{
    AnimState *src = p->anim;
    KeyedArray *list = pl->list;
    Projectile *e;
    AnimState *st;
    KeyedArray *actions;
    int slot, k;

    slot = KeyedArray_FreeSlot(list);
    if (slot == -1) {
        return -1;
    }
    e = (Projectile *)(list->data + list->elem_size * slot);
    k = KeyedArray_Add(list, proj, 0, 0);
    if (k == -1) {
        return -1;
    }
    st = &e->anim;
    *st = *src;
    st->anim = p->data->anim;
    Anim_SetAction(st, action);
    actions = st->anim->actions;
    if (st->action > st->anim->action_count) {
        st->action = 0;
    }
    st->ticks = 0;
    st->time = 0;
    st->elem = (AnimElem *)((AnimAction *)(actions->data + actions->elem_size * st->action))->elems->data;
    st->elem_index = 0;
    Anim_SeekElem(st);
    return k;
}

/* 0x004438A0: removes the projectile in `slot` (detaching what it is bound to). */
void Projectiles_Remove(int slot, ProjectileList *pl)
{
    Projectile *e = (Projectile *)(pl->list->data + pl->list->elem_size * slot);

    if (e == NULL) {
        return;
    }
    if (e->afterimage != NULL) {
        e->afterimage->running = 0;
        e->afterimage->anim = 0;
        e->afterimage = NULL;
    }
    KeyedArray_Remove(slot, pl->list);
}

/*
 * 0x004438E0: one tick of a player's projectiles: hit pauses, movement with
 * velocity multiplier and acceleration, facing, the edge and stage bounds,
 * removetime, and the remove / cancel / hit animations, after which the
 * projectile is removed (it is still stepped once in that tick).
 */
void Projectiles_Update(ProjectileList *pl)
{
    KeyedArray *list = pl->list;
    Projectile *e;

    if (list->count == 0) {
        return;
    }
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        GameVars *g = g_game;
        int paused = 0;
        int st;

        e->paused = 0;
        if (g->pause_kind != -1) {
            int v = e->pause_movetime[g->pause_kind];
            if (e->ticks > v && v != -1) {
                paused = 1;
                e->paused = 1;
            }
        }
        if (e->hitpause_time != 0) {
            if (paused) {
                continue;
            }
            goto step;
        }
        if (paused) {
            continue;
        }
        if (e->state == 1) {
            Coord p, v, w;
            double r;
            double stage_bound = (double)e->bound[1];
            int remove = 0;

            /* move */
            p = e->pos;
            v = e->vel;
            r = v.unit / p.unit;
            v.x = r * v.x;
            v.y = r * v.y;
            v.z = r * v.z;
            v.unit = p.unit;
            w = v;
            w.x = v.x + p.x;
            w.y = v.y + p.y;
            w.z = v.z + p.z;
            e->pos = w;
            e->vel.x = e->velmul.x * e->vel.x;
            e->vel.y = e->vel.y * e->velmul.y;
            p = e->accel;
            v = e->vel;
            r = p.unit / v.unit;
            p.x = r * p.x;
            p.y = r * p.y;
            p.z = r * p.z;
            p.unit = v.unit;
            w = p;
            w.x = p.x + v.x;
            w.y = p.y + v.y;
            w.z = p.z + v.z;
            e->vel = w;
            if (0.0 < e->vel.x) {
                e->facing = 1;
            } else if (0.0 > e->vel.x) {
                e->facing = -1;
            }
            /* bounds */
            if (stage_bound > e->pos.x && 0.0 > e->vel.x) {
                remove = 1;
            } else if ((double)e->bound[2] > e->pos.y && 0.0 > e->vel.y) {
                remove = 1;
            } else if ((double)e->bound[3] < e->pos.x && 0.0 < e->vel.x) {
                remove = 1;
            } else if ((double)e->bound[4] < e->pos.y && 0.0 < e->vel.y) {
                remove = 1;
            } else {
                float edge = (float)e->bound[0];
                double half = (double)Ftol(g->game_coord[0]) / g->camera_zoom * 0.5;
                if (((double)g->camera_offset_x - edge) - half > e->pos.x) {
                    remove = 1;
                } else if (((double)edge + g->camera_offset_x) + half < e->pos.x) {
                    remove = 1;
                } else {
                    if (e->miss_time > 0) {
                        e->miss_time--;
                    }
                    if (!(e->time < e->removetime || e->removetime == -1)) {
                        remove = 1;
                    }
                }
            }
            if (!remove) {
                goto step;
            }
            e->state = 2;
            e->time = 0;
        }
        st = e->state;
        if (st >= 2 && st <= 4) {
            Coord out;
            e->pos = *Coord_Add(&out, e->pos.x, e->pos.y, e->pos.z, e->pos.unit, e->pos.type, e->pos.pad_24,
                                e->vel.x, e->vel.y, e->vel.z, e->vel.unit, e->vel.type, e->vel.pad_24);
            st = e->state;
            if ((st == 2 && e->anim_remove == -1) || (st == 3 && e->anim_cancel == -1) ||
                (st == 4 && e->anim_hit == -1)) {
                Projectiles_Remove(list->iter, pl);
                goto step;
            }
            if (e->time == 0) {
                AnimState *as = &e->anim;
                e->vel = e->remvel;
                Anim_SetAction(as, st == 2 ? e->anim_remove : st == 4 ? e->anim_hit : e->anim_cancel);
                {
                    KeyedArray *actions = as->anim->actions;
                    if (((AnimAction *)(actions->data + actions->elem_size * as->action))->total_time <= 0) {
                        Projectiles_Remove(list->iter, pl);
                        goto step;
                    }
                }
            }
            {
                KeyedArray *actions = e->anim.anim->actions;
                if (e->anim.time == ((AnimAction *)(actions->data + actions->elem_size * e->anim.action))->total_time) {
                    Projectiles_Remove(list->iter, pl);
                }
            }
        } else {
            /* any other state: removed */
            Projectile *x = (Projectile *)(pl->list->data + pl->list->elem_size * list->iter);
            if (x != NULL) {
                if (x->afterimage != NULL) {
                    x->afterimage->running = 0;
                    x->afterimage->anim = 0;
                    x->afterimage = NULL;
                }
                KeyedArray_Remove(list->iter, pl->list);
            }
        }
    step:
        e->anim.layer = e->sprpriority;
        Anim_Step(&e->anim);
        if (e->hitpause_time > 0) {
            e->hitpause_time--;
        } else {
            e->ticks += 1;
            e->time += 1;
        }
    }
}

/* 0x00443E30: the screen position and scale of every live projectile for this tick. */
void Projectiles_UpdateDraw(ProjectileList *pl)
{
    KeyedArray *list = pl->list;
    Projectile *e;

    if (list->count == 0) {
        return;
    }
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        GameVars *g = g_game;
        DrawParams *dp = &e->draw;
        float zoom, cam, t;

        if ((unsigned int)(e->state - 1) > 3) {
            continue;
        }
        PS_DrawDefaults(dp, 0x100F);
        zoom = (float)((double)g->stage_scale * g->camera_zoom);
        e->scale = zoom;
        cam = (float)(g_gameCoord[0] / g->stage.unit * g->camera_x + g->camera_offset_x);
        t = (float)((e->pos.x - cam) * zoom + g->game_coord[0] * 0.5);
        t = (float)floor(t + 0.5);
        dp->x = (float)Ftol(t);
        cam = (float)(g_gameCoord[0] / g->stage.unit * g->camera_y + (double)g->camera_offset_y * g->camera_zoom);
        t = (float)((double)e->scale * e->pos.y - cam + g->draw_offset_y);
        t = (float)floor(t + 0.5);
        dp->y = (float)Ftol(t);
        dp->flip2 = e->facing != 1;
        dp->scale_x = (float)(e->scale * e->draw_scale_x);
        dp->scale_y = (float)(e->scale * e->draw_scale_y);
        if (g->zoom_active != 0.0f) {
            dp->x = (float)(dp->x - (double)g->shake_x * g->stage_scale);
            dp->y = (float)(dp->y - (double)g->shake_y * g->stage_scale);
        }
    }
}

/* 0x00444040: the live (state 1) projectiles of a list. */
int Projectiles_Count(ProjectileList *pl)
{
    KeyedArray *list = pl->list;
    Projectile *e;
    int n = 0;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->state == 1) {
            n++;
        }
    }
    return n;
}

/* 0x004440A0: the live projectiles with ID `id`. */
int Projectiles_CountId(ProjectileList *pl, int id)
{
    KeyedArray *list = pl->list;
    Projectile *e;
    int n = 0;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->state == 1 && e->id == id) {
            n++;
        }
    }
    return n;
}

/* 0x00444100: draws the projectiles (states 1 to 4). */
void Projectiles_Draw(ProjectileList *pl)
{
    PalFx *saved_fx = g_palFxCur;
    PalRemap *saved_remap = g_palRemapCur;
    KeyedArray *list = pl->list;
    Projectile *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if ((unsigned int)(e->state - 1) <= 3) {
            g_palRemapCur = e->remap;
            g_palFxCur = e->palfx;
            Anim_Draw(&e->anim, &e->draw, 1.0f, 1.0f);
        }
    }
    g_palRemapCur = saved_remap;
    g_palFxCur = saved_fx;
}

/* 0x004441B0: draws the shadows of the projectiles that have one. */
void Projectiles_DrawShadow(ProjectileList *pl)
{
    PalFx *saved_fx = g_palFxCur;
    PalRemap *saved_remap = g_palRemapCur;
    KeyedArray *list = pl->list;
    Projectile *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if ((unsigned int)(e->state - 1) <= 3 && e->shadow != 0) {
            GameVars *g = g_game;
            g_palRemapCur = e->remap;
            g_palFxCur = e->palfx;
            Shadow_Draw(&e->anim, &e->draw, (float)e->pos.y, 0.0f, 1.0f, 1.0f, g->stage.shadow, &g->stage.reflection_intensity);
        }
    }
    g_palRemapCur = saved_remap;
    g_palFxCur = saved_fx;
}

/*
 * 0x004442F0: draws the collision boxes of a projectile (Clsn2 blue, Clsn1
 * red). The original has Video_MapRGB inlined.
 */
void Projectile_DrawClsn(Projectile *e)
{
    int flip = e->facing != 1;
    unsigned int color;

    color = Video_MapRGB(0, 0, 0xFF);
    Clsn_Draw(e->anim.elem->clsn2, e->draw.x, e->draw.y, e->scale, e->scale, color, flip, g_game->clsn_draw_mode == 2);
    color = Video_MapRGB(0xFF, 0, 0);
    Clsn_Draw(e->anim.elem->clsn1, e->draw.x, e->draw.y, e->scale, e->scale, color, flip, g_game->clsn_draw_mode == 2);
}

/* 0x004442A0: draws the collision boxes of every projectile of a list. */
void Projectiles_DrawClsn(ProjectileList *pl)
{
    KeyedArray *list = pl->list;
    Projectile *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        Projectile_DrawClsn(e);
    }
}

/* Team bits of a HitDef as seen from team side `side` (side 2 swaps the enemy and friend bits). */
static unsigned int PS_AffectTeam(unsigned int affectteam, int side)
{
    if (side == 2) {
        return (affectteam & ~3u) | ((affectteam >> 1) & 1) | ((affectteam & 1) << 1);
    }
    return affectteam;
}

/*
 * 0x004405C0: p's HitDef with the P hit flag hits owner's live projectiles:
 * the projectile's guard sound, p1stateno, and the projectile loses a hit
 * (removed with its remove animation at 0 when removeonhit).
 */
void Player_HitProjectiles(Player *p, Player *owner)
{
    KeyedArray *list = ((ProjectileList *)owner->projectiles)->list;
    int flip = p->facing != 1;
    const Clsn *clsn = p->anim->elem->clsn1;
    Projectile *e;

    if ((p->hitdef.hitflag & 0x80) == 0 || p->movetype == MOVETYPE_IDLE || p->hitdef.active == 0 ||
        list->count == 0) {
        return;
    }
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        int pf = e->facing;
        int pflip = pf != 1;
        int dx, dy, x, y;
        Coord c, screen;

        if (e->state != 1) {
            continue;
        }
        if (((unsigned int)e->team_side & PS_AffectTeam(p->hitdef.affectteam, p->team_side)) == 0 ||
            e->hitpause_time > 0 || e->miss_time > 0) {
            continue;
        }
        dy = Ftol((p->draw_offset.y + p->pos.y) - e->pos.y);
        dx = Ftol((p->draw_offset.x + p->pos.x) - e->pos.x);
        if (!Clsn_Collide(pflip, flip, e->anim.elem->clsn2, clsn, dx, dy) || e->hits <= 0) {
            continue;
        }
        x = Ftol(Player_FromLocalCoord(owner, (double)e->hitdef.sparkxy[0]) * pf + e->pos.x);
        y = Ftol(Player_FromLocalCoord(owner, (double)e->hitdef.sparkxy[1]) + e->pos.y);
        p->hitdef.active = 0;
        Coord_MakeUnit(&c, (double)x, (double)y);
        Coord_GameToScreen(&screen, c.x, c.y, c.z, c.unit, c.type, c.pad_24);
        if (e->hitdef.guardsound[2] == 0) {
            Sound_PlayPanned(owner->data->snd, e->hitdef.guardsound[0], Ftol(screen.x), e->hitdef.guardsound[1],
                             owner->data->volumescale);
        } else {
            Sound_PlayPanned(g_game->fight.common_snd, e->hitdef.guardsound[0], Ftol(screen.x),
                             e->hitdef.guardsound[1], 100.0);
        }
        if (p->hitdef.p1stateno != -1) {
            Player_ChangeState(p->hitdef.p1stateno, p);
        }
        e->hits--;
        if (e->hits <= 0 && e->removeonhit != 0) {
            e->state = 2;
            e->time = 0;
        } else {
            e->hitpause_time = e->hitdef.p1_pausetime;
        }
    }
}

/*
 * 0x00440880: owner's live projectiles hit tgt (like Player_Hit for a
 * projectile's HitDef): guard or hit, HitOverride, target entry and juggle,
 * the spark, HitDef_Apply, power, hit counters, the projectile contact of
 * the owner (ProjHit...), palfx, p1stateno / p2stateno / p2getp1state and
 * the snap binding. The projectile loses its hit at the end of the tick
 * (Projectiles_ApplyHits).
 */
void Projectiles_HitPlayer(Player *owner, Player *tgt)
{
    KeyedArray *list = ((ProjectileList *)owner->projectiles)->list;
    int flip_t = tgt->facing != 1;
    int wins = (&g_game->round_no)[owner->team_side];
    const Clsn *clsn_t = tgt->anim->elem->clsn2;
    Projectile *e;

    if (tgt->nohit_time > 0 || list->count == 0) {
        return;
    }
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        HitDef *hd = &e->hitdef;
        int pflip = e->facing != 1;
        int ko = 0, guarded = 0, overridden = 0;
        HitOverride *ov = NULL;
        TargetEntry *entry = NULL;
        unsigned int m;
        int dx, dy, x, y, pf, i;

        if ((unsigned int)(e->state - 1) > 1 || e->hitpause_time > 0) {
            continue;
        }
        if (((unsigned int)tgt->team_side & PS_AffectTeam(hd->affectteam, e->team_side)) == 0) {
            continue;
        }
        m = tgt->hitby_attr & hd->attr & tgt->nothitby_attr;
        if ((m & HITATTR_STATES) == 0 || (m & ~HITATTR_STATES) == 0 || e->miss_time > 0) {
            continue;
        }
        dy = Ftol((tgt->draw_offset.y + tgt->pos.y) - e->pos.y);
        dx = Ftol((tgt->draw_offset.x + tgt->pos.x) - e->pos.x);
        if (!Clsn_Collide(pflip, flip_t, e->anim.elem->clsn1, clsn_t, dx, dy)) {
            continue;
        }
        if (!Player_HitFlagMatches(tgt, hd->hitflag) || e->hits <= 0) {
            continue;
        }
        if (Player_HitFlagMatches(tgt, hd->guardflag) && tgt->in_guard_dist != 0 && owner->unguardable == 0) {
            int dmg = Player_HitDamage(tgt, hd->guard_damage, hd->guard_kill);
            if ((int)((unsigned int)tgt->life - tgt->gethit_damage - dmg) > 0) {
                guarded = 1;
            } else {
                ko = 1;
                if (g_game->round_state != 3 && wins < 10) {
                    Round_SetWinType(owner->team_side, WIN_ICON_C, 0);
                }
            }
        } else {
            int dmg = Player_HitDamage(tgt, hd->damage, hd->kill);
            if ((int)((unsigned int)tgt->life - tgt->gethit_damage - dmg) <= 0) {
                ko = 1;
                Round_SetKoWinType(owner->team_side, tgt->team_side, hd->attr);
            }
        }
        if (g_game->debug_no_ko != 0) {
            ko = 0;
        }
        for (i = 0; i < 8; i++) {
            HitOverride *o = &tgt->hit_override[i];
            unsigned int om;
            if (o->active == 0) {
                continue;
            }
            om = o->attr & hd->attr;
            if ((om & HITATTR_STATES) != 0 && (om & ~HITATTR_STATES) != 0) {
                overridden = 1;
                guarded = 0;
                ko = 0;
                ov = o;
                break;
            }
        }
        if (!overridden) {
            tgt->sprpriority = hd->p2sprpriority;
            entry = Player_AddTarget(owner, tgt);
            if (entry == NULL) {
                continue;
            }
            entry->bind_time = 0;
            entry->bind_y = 0;
            entry->bind_x = 0;
            if (owner->nojugglecheck == 0 && (int)((unsigned int)entry->juggle - (unsigned int)hd->juggle) < 0) {
                continue;
            }
        }
        pf = e->facing;
        x = Ftol(Player_FromLocalCoord(owner, (double)hd->sparkxy[0]) * pf + e->pos.x);
        y = Ftol(Player_FromLocalCoord(owner, (double)hd->sparkxy[1]) + e->pos.y);
        if (guarded) {
            Player_Guard(tgt, owner, pf, hd);
            Player_SetPower(owner, *owner->power_ref + hd->guard_getpower);
            Spark_Create(hd->guard_sparkno, owner, x, y, hd->guard_spark_own, pflip, 1);
            owner->proj_contact_id = e->id;
            owner->proj_contact = 1;
            owner->proj_contact_time = 0;
        } else {
            HitDef_Apply(ov, tgt, owner, hd, (float)e->pos.x, (float)e->pos.y, pf, ko);
            Player_SetPower(owner, *owner->power_ref + hd->getpower);
            if (!overridden && owner->nojugglecheck == 0 && tgt->gethit_fall != 0 && hd->juggle != -1) {
                entry->juggle -= hd->juggle;
                hd->juggle = 0;
            }
            owner->combo_hits += hd->numhits;
            Combo_AddHits(owner->team_side, hd->numhits);
            owner->proj_contact_id = e->id;
            owner->proj_contact = 2;
            owner->proj_contact_time = 0;
            tgt->nothitby_attr = 0x1FF;
            tgt->nothitby_time = 0;
            if (!overridden && (hd->palfx[1] > 0 || hd->palfx[1] == -1)) {
                memcpy(&tgt->palfx_def, hd->palfx, sizeof(hd->palfx));
            }
            Spark_Create(hd->sparkno, owner, x, y, hd->spark_own, pflip, 0);
            if (!overridden) {
                if (hd->p1stateno != -1) {
                    Player_ChangeState(hd->p1stateno, owner);
                }
                if (hd->p2getp1state != 0) {
                    Player_SetCustomState(tgt, &hd->custom);
                    tgt->movetype = MOVETYPE_HIT;
                }
                if (hd->p2stateno != -1) {
                    Targets_Release(tgt, NULL);
                    Explods_RemoveOnHit(g_game->explods, tgt->id);
                    Player_ChangeState(hd->p2stateno, tgt);
                    tgt->ctrl = 0;
                }
                entry->bind_time = hd->snap_time;
                entry->bind_x = Ftol(Player_FromLocalCoord(owner, (double)hd->maxdist[0]));
                entry->bind_y = Ftol(Player_FromLocalCoord(owner, (double)hd->maxdist[1]));
            } else if (ov->stateno != -1) {
                Player_ChangeState(ov->stateno, tgt);
            }
        }
        e->hit_pending = 1;
        if (e->hits - 1 <= 0 && e->removeonhit != 0) {
            e->hitpause_next = 0;
            e->miss_time_next = 0;
        } else {
            e->hitpause_next = hd->p1_pausetime;
            e->miss_time_next = e->projmisstime;
        }
    }
}

/* 0x00440F50: the hits of the tick: each projectile that hit loses a hit (removed with its hit animation at 0). */
void Projectiles_ApplyHits(Player *p)
{
    KeyedArray *list = ((ProjectileList *)p->projectiles)->list;
    Projectile *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->hit_pending != 0) {
            e->hits--;
            e->miss_time = e->miss_time_next;
            e->hit_pending = 0;
            e->hitpause_time = e->hitpause_next;
            if (e->hits <= 0 && e->removeonhit != 0) {
                e->state = 4;
                e->time = 0;
            }
        }
    }
}

/*
 * 0x00440FD0: projectiles of two players cancel each other: each loses a
 * hit against one of lower or equal priority (and a priority point), at 0
 * hits it is cancelled (state 3). A projectile collides with one other per
 * tick at most.
 */
void Projectiles_Collide(Player *a, Player *b)
{
    KeyedArray *la = ((ProjectileList *)a->projectiles)->list;
    KeyedArray *lb = ((ProjectileList *)b->projectiles)->list;
    Projectile *pa, *pb;

    if (la->count == 0) {
        return;
    }
    for (pa = KeyedArray_First(la); pa != NULL; pa = KeyedArray_Next(la)) {
        const Clsn *clsn_a = pa->anim.elem->clsn2;
        int flip_a = pa->facing != 1;

        if (clsn_a == NULL || pa->state != 1 || lb->count == 0) {
            continue;
        }
        for (pb = KeyedArray_First(lb); pb != NULL; pb = KeyedArray_Next(lb)) {
            int dx, dy, flip_b, dec_a;

            if (pb->state != 1) {
                continue;
            }
            if ((PS_AffectTeam(pa->hitdef.affectteam, pa->team_side) & (unsigned int)pb->team_side) == 0 &&
                (PS_AffectTeam(pb->hitdef.affectteam, pb->team_side) & (unsigned int)pa->team_side) == 0) {
                continue;
            }
            dy = Ftol(pb->pos.y - pa->pos.y);
            dx = Ftol(pb->pos.x - pa->pos.x);
            flip_b = pb->facing != 1;
            if (!Clsn_Collide(flip_a, flip_b, clsn_a, pb->anim.elem->clsn2, dx, dy)) {
                continue;
            }
            dec_a = 0;
            if (pa->priority <= pb->priority) {
                pa->hits--;
                dec_a = 1;
            }
            if (pa->hits <= 0) {
                pa->state = 3;
                pa->time = 0;
                a->proj_contact = 3;
                a->proj_contact_time = 0;
            } else {
                pa->hitpause_time = pa->hitdef.p1_pausetime;
            }
            if (pb->priority <= pa->priority) {
                pa->priority--;
                pb->hits--;
            }
            if (pb->hits <= 0) {
                pb->state = 3;
                pb->time = 0;
                b->proj_contact = 3;
                b->proj_contact_time = 0;
            } else {
                pb->hitpause_time = pb->hitdef.p1_pausetime;
            }
            if (dec_a) {
                pb->priority--;
            }
            break;
        }
    }
}
