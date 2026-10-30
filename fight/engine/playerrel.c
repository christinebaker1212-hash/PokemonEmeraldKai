/*
 * Relations between players (original block 0x433E00-0x434B1B, the end of
 * the AI module, no strings): enemies and partners, players by id, screen
 * edges, horizontal distances (positions, body widths, edges), guard
 * distances, target lists and damage scaling.
 *
 * Several functions keep x87 intermediate results unrounded (extended
 * precision with the 53-bit precision control of the process) and round to
 * float only where the original stores a float; the float stores are kept
 * as assignments to float variables.
 */
#include "mugen.h"

/* An enemy for Player_FindEnemy: active, not p, not KOed for good, of the other team, a player or a "player" helper. */
static int Rel_IsEnemyCandidate(const Player *p, const Player *root, const Player *t)
{
    if (t == NULL || !t->active || t == p) {
        return 0;
    }
    if (t->state_no == 5150 && t->state_time != 0) {
        return 0;
    }
    if (root->team_side == t->team_side || root->team_side == 0 || t->team_side == 0) {
        return 0;
    }
    return t->helper == 0 || t->helper_player == 1;
}

/*
 * 0x00433E00: the nearest enemy of p by body distance, first among its
 * targets while p attacks, else among all players. NULL if none.
 *
 * Quirk: candidates are compared with the distance of the first one found,
 * not with the nearest so far.
 */
Player *Player_FindEnemy(Player *p)
{
    float best = 0.0f;
    Player *found = NULL;
    Player *root = p->root != NULL ? p->root : p;
    KeyedArray *targets = p->targets;
    void *elem;
    GameVars *g;
    int i;

    for (elem = KeyedArray_First(targets); elem != NULL; elem = KeyedArray_Next(targets)) {
        Player *t;
        if (p->movetype != 1) {
            break;
        }
        t = *(Player **)elem;
        if (!Rel_IsEnemyCandidate(p, root, t)) {
            continue;
        }
        if (found != NULL) {
            float d = (float)fabs(Player_BodyDistX(p, t));
            if (best > d) {
                found = t;
            }
        } else {
            found = t;
            best = (float)fabs(Player_BodyDistX(p, t));
        }
    }
    if (found != NULL) {
        return found;
    }
    g = g_game;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *t = g->players[i];
        if (!Rel_IsEnemyCandidate(p, root, t)) {
            continue;
        }
        if (found != NULL) {
            float d = (float)fabs(Player_BodyDistX(p, t));
            if (best > d) {
                found = t;
            }
        } else {
            found = t;
            best = (float)fabs(Player_BodyDistX(p, t));
        }
    }
    return found;
}

/* A partner of p: an active player (not a helper) of its team, not p or its root. */
static int Rel_IsPartner(const Player *p, const Player *t)
{
    return t != NULL && t->active && t->helper == 0 && t->team_side == p->team_side && t != p && t != p->root;
}

/* 0x00433FD0: the n-th partner of p (0-based), or NULL. */
Player *Player_GetPartner(Player *p, int n)
{
    GameVars *g = g_game;
    int i, k = 0;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *t = g->players[i];
        if (Rel_IsPartner(p, t)) {
            if (k == n) {
                return t;
            }
            k++;
        }
    }
    return NULL;
}

/* 0x00434030: the number of partners of p. */
int Player_CountPartners(Player *p)
{
    GameVars *g = g_game;
    int i, n = 0;

    for (i = g->players_first; i <= g->players_last; i++) {
        if (Rel_IsPartner(p, g->players[i])) {
            n++;
        }
    }
    return n;
}

/* An enemy of p: an active player (not a helper) of the other team. */
static int Rel_IsEnemy(const Player *p, const Player *t)
{
    return t != NULL && t->active && t->helper == 0 && p->team_side != t->team_side && p->team_side != 0 &&
           t->team_side != 0;
}

/* 0x00434090: the n-th enemy of p (0-based), or NULL. */
Player *Player_GetEnemy(Player *p, int n)
{
    GameVars *g = g_game;
    int i, k = 0;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *t = g->players[i];
        if (Rel_IsEnemy(p, t)) {
            if (k == n) {
                return t;
            }
            k++;
        }
    }
    return NULL;
}

/* 0x00434100: the number of enemies of p. */
int Player_CountEnemies(Player *p)
{
    GameVars *g = g_game;
    int i, n = 0;

    for (i = g->players_first; i <= g->players_last; i++) {
        if (Rel_IsEnemy(p, g->players[i])) {
            n++;
        }
    }
    return n;
}

/*
 * 0x00434160: the nearest (n = 0) or second nearest (n = 1) of the two
 * enemy team players by |DistX|; NULL for another n, a player without a
 * team or missing enemies.
 */
Player *Player_GetNearEnemy(Player *p, int n)
{
    GameVars *g;
    Player *a = NULL, *b = NULL;
    int a_ok, b_ok;
    double da, db;

    if (n >= 2) {
        return NULL;
    }
    g = g_game;
    if (p->team_side == 1) {
        a = g->players[2];
        b = g->players[4];
    } else if (p->team_side == 2) {
        a = g->players[1];
        b = g->players[3];
    }
    a_ok = a != NULL && a->active;
    b_ok = b != NULL && b->active;
    if (!a_ok && !b_ok) {
        return NULL;
    }
    if (n == 0) {
        if (a_ok && !b_ok) {
            return a;
        }
        if (!a_ok) {
            return b_ok ? b : NULL;
        }
    } else if (!a_ok || !b_ok) {
        return NULL;
    }
    da = fabs(Player_DistX(p, a));
    db = fabs(Player_DistX(p, b));
    if (db <= da || db != db || da != da) {
        return n == 0 ? b : a;
    }
    return n == 0 ? a : b;
}

/* 0x00434250: players[index] if it is in the active range and active, else NULL. */
Player *Player_GetActive(int index)
{
    GameVars *g = g_game;
    Player *p;

    if (index < g->players_first || index > g->players_last) {
        return NULL;
    }
    p = g->players[index];
    if (p == NULL || !p->active) {
        return NULL;
    }
    return p;
}

/* 0x00434280: the active player with Player.id id, or NULL. */
Player *Player_FindById(int id)
{
    int i = Player_FindIndexById(id);

    if (i == -1) {
        return NULL;
    }
    return g_game->players[i];
}

/* 0x004342A0: the index of the active player with Player.id id, or -1. */
int Player_FindIndexById(int id)
{
    GameVars *g = g_game;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active && p->id == id) {
            return i;
        }
    }
    return -1;
}

/* 0x004342F0: p is within 5 units of the left or right screen edge (Player.screen_left / screen_right). */
int Player_AtScreenEdge(Player *p)
{
    double x = p->pos.x;

    if (5.0 > fabs(x - p->screen_left)) {
        return 1;
    }
    return fabs(x - p->screen_right) < 5.0;
}

/*
 * 0x00434360: the draw position of p (screen position / draw scale, minus
 * Player.draw_offset) as a 2D coordinate in *out. The pad field is stack
 * garbage in the original (0 here).
 */
Coord *Player_GetScreenPos(Player *p, Coord *out)
{
    Coord c;

    Player_UpdateDrawParams(p, 0);
    c.x = (double)p->draw.x / p->draw_zoom - p->draw_offset.x;
    c.y = (double)p->draw.y / p->draw_zoom - p->draw_offset.y;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/* round(v) as the original does it for widths: through float, floor(v + 0.5), float again, then __ftol2. */
static int Rel_RoundWidth(double v)
{
    float f = (float)v;
    float r = (float)floor(f + 0.5);

    return Ftol(r);
}

/*
 * 0x004343D0: distance from p to the screen edge in front of it (front) or
 * behind it, positive inside the screen; with body, from the front / back
 * of its body (Player.width_edge_front / width_edge_back, in character units) and plus
 * 1 in the air, 2 lying down. Also updates the screen edges Player.screen_left
 * and screen_right from the camera and the stage's screen bounds.
 */
double Player_EdgeDist(Player *p, int body, int front)
{
    GameVars *g = g_game;
    double w = g_gameCoord[0];
    double add = 0.0;
    double x;
    float right;

    p->screen_left = (float)((double)g->stage.screenleft * (w / g->stage.unit) + g->screen_left_target);
    right = (float)(g->screen_right_target - (double)g->stage.screenright * (w / g->stage.unit));
    p->screen_right = right;
    if (body) {
        if (p->statetype == 3) {
            add = 0.0 + 1.0;
        }
        if (p->statetype == 4) {
            add = add + 2.0;
        }
    }
    if (front) {
        double edge;
        x = add;
        if (body) {
            x = (double)Rel_RoundWidth(w / p->data->localcoord[0] * p->width_edge_front) + add;
        }
        edge = p->facing == 1 ? (double)right : (double)p->screen_left;
        return ((x * p->facing + p->pos.x) - edge) * -p->facing;
    } else {
        double edge;
        x = add;
        if (body) {
            int width = p->facing == -p->facing ? p->width_edge_front : p->width_edge_back;
            x = (double)Rel_RoundWidth(w / p->data->localcoord[0] * width) + add;
        }
        edge = p->facing == 1 ? (double)p->screen_left : (double)right;
        return ((p->pos.x - x * p->facing) - edge) * p->facing;
    }
}

/* 0x004345A0: (other.x - p.x) * p.facing, as a float. */
float Player_DistX(Player *p, Player *other)
{
    float d = (float)((other->pos.x - p->pos.x) * p->facing);

    return d;
}

/*
 * 0x004345F0: the horizontal distance from the front of p's body to the
 * nearest side of other's body, along p's facing (widths in character
 * units: ground or air front and back).
 */
float Player_BodyDistX(Player *p, Player *other)
{
    double w = g_gameCoord[0];
    float front, side, o_edge, d;

    front = (float)(w / p->data->localcoord[0] * (p->statetype == 3 ? p->consts.air_front : p->consts.ground_front) *
                    p->facing + p->pos.x);
    side = (float)((p->pos.x - other->pos.x) * other->facing);
    if (!(side <= 0.0f || side != side)) {
        o_edge = (float)(w / other->data->localcoord[0] *
                         (other->statetype == 3 ? other->consts.air_front : other->consts.ground_front) *
                         other->facing + other->pos.x);
    } else {
        o_edge = (float)(other->pos.x - w / other->data->localcoord[0] *
                         (other->statetype == 3 ? other->consts.air_back : other->consts.ground_back) *
                         other->facing);
    }
    d = (float)(((double)o_edge - front) * p->facing);
    return d;
}

/* 0x00434720: an enemy of p is close enough for p to guard (Player_InGuardDist). */
int Player_InEnemyGuardDist(Player *p)
{
    GameVars *g = g_game;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *t = g->players[i];
        if (p != t && t != NULL && t->active && t->team_side != p->team_side && t->team_side != 0) {
            if (Player_InGuardDist(t, p)) {
                return 1;
            }
            g = g_game;
        }
    }
    return 0;
}

/*
 * 0x004347A0: p is in front of the attacker within the guard distance of
 * its attack (attacking), or of one of its active projectiles.
 */
int Player_InGuardDist(Player *attacker, Player *p)
{
    double w = g_gameCoord[0];
    KeyedArray *projs = ((ProjectileList *)attacker->projectiles)->list;
    int result = 0;
    Projectile *pr;

    if (attacker->movetype == 1) {
        float d = (float)((p->pos.x - attacker->pos.x) * attacker->facing);
        double gd = w / attacker->data->localcoord[0] * attacker->hitdef.guard_dist;
        if (gd > d && !(d < 0.0)) {
            result = 1;
        }
    }
    for (pr = KeyedArray_First(projs); pr != NULL; pr = KeyedArray_Next(projs)) {
        if (pr->hitdef.active != 0) {
            float d = (float)((p->pos.x - pr->pos.x) * pr->facing);
            double gd = w / attacker->data->localcoord[0] * pr->hitdef.guard_dist;
            if (gd > d && !(d < 0.0)) {
                return 1;
            }
        }
    }
    return result;
}
/* 0x00434900: the HitDef's affectteam (bits: 1 enemy, 2 friend, swapped here) includes team. */
int HitDef_AffectsTeam(const HitDef *hd, int team)
{
    unsigned int v = hd->affectteam;
    unsigned int sw = ((v >> 1) & 1) | ((v & 1) << 1) | (v & ~3u);

    return (sw & (unsigned int)team) != 0;
}

/* 0x00434930: the animations p uses have action `action`. */
int Player_AnimExists(Player *p, int action)
{
    return KeyedArray_Find(*(KeyedArray **)p->anim->anim, 0, action) != -1;
}

/* 0x00434960: the character's own animations have action `action`. */
int Player_SelfAnimExists(Player *p, int action)
{
    Anim *saved = p->anim->anim;
    int r;

    p->anim->anim = p->data->anim;
    r = KeyedArray_Find(*(KeyedArray **)p->anim->anim, 0, action) != -1;
    p->anim->anim = saved;
    return r;
}

/* 0x004349B0: the target entry of player p, or NULL. */
TargetEntry *Targets_FindPlayer(KeyedArray *targets, Player *p)
{
    TargetEntry *e;

    for (e = KeyedArray_First(targets); e != NULL; e = KeyedArray_Next(targets)) {
        if (e->player == p) {
            return e;
        }
    }
    return NULL;
}

/* 0x00434A00: the first target hit by HitDef id hit_id (-1: any), as its entry, or NULL. */
Player **Targets_Find(KeyedArray *targets, int hit_id)
{
    TargetEntry *e;

    for (e = KeyedArray_First(targets); e != NULL; e = KeyedArray_Next(targets)) {
        if (e->player->gethit_id == hit_id || hit_id == -1) {
            return &e->player;
        }
    }
    return NULL;
}

/* 0x00434A60: the number of targets hit by HitDef id hit_id (-1: all). */
int Targets_Count(KeyedArray *targets, int hit_id)
{
    TargetEntry *e;
    int n = 0;

    for (e = KeyedArray_First(targets); e != NULL; e = KeyedArray_Next(targets)) {
        if (e->player->gethit_id == hit_id || hit_id == -1) {
            n++;
        }
    }
    return n;
}

/*
 * 0x00434AC0: damage times p's attack multiplier, rounded; at least 1 when
 * the damage is positive and the multiplier not 0.
 */
int Player_ScaleDamage(Player *p, int damage)
{
    float f = (float)(damage * p->attack_mul);
    float r = (float)floor(f + 0.5);
    int n = Ftol(r);

    if (damage >= 1 && n == 0 && !(0.0f == p->attack_mul)) {
        return n + 1;
    }
    return n;
}
