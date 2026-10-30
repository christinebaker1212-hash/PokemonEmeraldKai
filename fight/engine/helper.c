/*
 * Helpers (original module 0x419A50-0x41A4F7, strings "Helper invalid
 * remappal %i,%i" to "illegal helper supermovetime"): the Helper and
 * DestroySelf controllers and the NumHelper / helper(ID) lookups.
 *
 * A helper takes the first inactive player slot from index 5 up to
 * GameVars.player_slots (HelperMax + 4) and shares the character data, states,
 * sprites and (unless ownpal) the PalFX and palette remapping of its parent.
 */
#include "mugen.h"

/*
 * 0x00419A50: the Helper controller. Returns the new helper, or NULL when
 * every helper slot is taken. The remappal values are evaluated in the
 * context of the new helper, keyctrl twice, and the y offset of the position
 * origin is not applied (quirks of the original). A NULL player in the slots
 * is not checked for (the original crashes).
 */
Player *Helper_Create(Player *p, HelperParams *hp, StateRunCtx *ctx, StateCtrl *sc)
{
    GameVars *g = g_game;
    Player *h;
    double k;
    float x, y, off_x, off_y, fx, fy;
    int i, idx, ownpal, rg, rn, v;

    if (g->player_slots < 5) {
        return NULL;
    }
    for (i = 5;; i++) {
        if (!g->players[i]->active) {
            break;
        }
        if (i + 1 > g->player_slots) {
            return NULL;
        }
    }
    h = g->players[i];
    if (h == NULL) {
        return NULL;
    }
    h->data = p->data;
    h->cmds = &p->data->cmds;
    h->states = p->states;
    idx = h->index;
    if (g->players[idx]->active != 1) {
        if (g->players_active == 0) {
            g->players_first = idx;
            g->players_last = idx;
        } else {
            if (idx < g->players_first) {
                g->players_first = idx;
            }
            if (idx > g->players_last) {
                g->players_last = idx;
            }
        }
        g->players[idx]->active = 1;
        g->players_active++;
    }
    h->sprites = p->sprites;
    ownpal = Expr_EvalInt(p, &hp->ownpal, 0);
    rg = Expr_EvalInt(h, &hp->remappal[0], 0);
    rn = Expr_EvalInt(h, &hp->remappal[1], 0);
    if (ownpal == 0) {
        h->palfx = p->palfx;
        h->remap = p->remap;
        h->remap_own = *p->remap;
    } else {
        h->palfx = &h->palfx_own;
        if (rg == -1) {
            h->remap_own = *p->remap;
        } else {
            KeyedArray *pals = h->sprites->palettes;
            int slot = -1;
            if (pals != NULL) {
                slot = KeyedArray_Find(pals, rg, rn);
            }
            if (slot != -1 && pals->data + pals->elem_size * slot != NULL) {
                PalRemapEntry en;
                en.src_group = -1;
                en.src_index = 0;
                en.dst_group = rg;
                en.dst_index = rn;
                memset(&h->remap_own, 0, sizeof(PalRemap));
                PalRemap_Set(&h->remap_own, &en);
            } else {
                Player_WarnAt(p, sc, "Helper invalid remappal %i,%i", rg, rn);
            }
        }
        h->remap = &h->remap_own;
    }
    h->anim->anim = h->data->anim;
    h->anim->sprites = h->sprites;
    ((ProjectileList *)h->projectiles)->anim = h->data->anim;
    ((ProjectileList *)h->projectiles)->sprites = h->sprites;
    if (hp->name[0] != '\0') {
        strcpy(h->name, hp->name);
    } else {
        sprintf(h->name, "%s's helper", p->name);
    }
    h->id = g_game->player_id_next++;
    h->consts = p->consts;

    fx = Expr_EvalFloat(p, &hp->xscale);
    if (fx != -FLT_MAX) {
        h->consts.xscale = fx;
    }
    fy = Expr_EvalFloat(p, &hp->yscale);
    if (fy != -FLT_MAX) {
        h->consts.yscale = fy;
    }
    v = Expr_EvalInt(p, &hp->ground_back, 1);
    if (v != INT_MIN) {
        h->consts.ground_back = v;
    }
    v = Expr_EvalInt(p, &hp->ground_front, 1);
    if (v != INT_MIN) {
        h->consts.ground_front = v;
    }
    v = Expr_EvalInt(p, &hp->air_back, 1);
    if (v != INT_MIN) {
        h->consts.air_back = v;
    }
    v = Expr_EvalInt(p, &hp->air_front, 1);
    if (v != INT_MIN) {
        h->consts.air_front = v;
    }
    v = Expr_EvalInt(p, &hp->height, 1);
    if (v != INT_MIN) {
        h->consts.height = v;
    }
    v = Expr_EvalInt(p, &hp->proj_doscale, 1);
    if (v != INT_MIN) {
        h->consts.proj_doscale = v;
    }
    v = Expr_EvalInt(p, &hp->head_pos[0], 1);
    if (v != INT_MIN) {
        h->consts.head_pos[0] = v;
    }
    v = Expr_EvalInt(p, &hp->head_pos[1], 1);
    if (v != INT_MIN) {
        h->consts.head_pos[1] = v;
    }
    v = Expr_EvalInt(p, &hp->mid_pos[0], 1);
    if (v != INT_MIN) {
        h->consts.mid_pos[0] = v;
    }
    v = Expr_EvalInt(p, &hp->mid_pos[1], 1);
    if (v != INT_MIN) {
        h->consts.mid_pos[1] = v;
    }
    v = Expr_EvalInt(p, &hp->shadowoffset, 1);
    if (v != INT_MIN) {
        h->consts.shadowoffset = v;
    }
    h->pal_no = p->pal_no;
    Player_Reset(h);
    memset(h->var, 0, sizeof(h->var));
    memset(h->fvar, 0, sizeof(h->fvar));
    v = Expr_EvalInt(p, &hp->stateno, 1);
    if (v < 0) {
        v = 0;
        Player_WarnAt(p, sc, "%s", "illegal helper stateno");
    }
    Player_ChangeState(v, h);
    h->pos = p->pos;
    h->team_side = p->team_side;
    h->input_keys = p->input_keys;
    h->no_keyctrl = Expr_EvalInt(p, &hp->keyctrl, 0) == 0;
    if (Expr_EvalInt(p, &hp->keyctrl, 0) != 0) {
        h->input = p->input;
        h->cmd_state = p->cmd_state;
    }
    v = Expr_EvalInt(p, &hp->id, 1);
    if (v < 0) {
        v = 0;
        Player_WarnAt(p, sc, "%s", "negative helper ID");
    }
    h->helper_id = v;
    h->parent_id = p->id;
    h->parent = p;
    h->root = p->root != NULL ? p->root : p;
    h->helper_player = hp->helpertype;

    Player_GetPosOrigin(NULL, p, hp->postype, &ctx->enemy, 0, &x, &y, &off_x, &off_y, &h->facing);
    if (hp->postype == POSTYPE_FRONT || hp->postype == POSTYPE_BACK || hp->postype == POSTYPE_RIGHT ||
        hp->postype == POSTYPE_LEFT) {
        y = 0.0f;
    }
    /* positions are in the units of the character whose states run (a custom state owner) */
    fx = Expr_EvalFloat(p, &hp->pos[0]);
    if (p->custom_states != NULL && p->custom_owner != -1 && g_game->players[p->custom_owner] != NULL) {
        k = g_gameCoord[0] / g_game->players[p->custom_owner]->data->localcoord[0];
    } else {
        k = g_gameCoord[0] / p->data->localcoord[0];
    }
    h->pos.x = ((double)x + off_x) + fx * k * (double)h->facing;
    fy = Expr_EvalFloat(p, &hp->pos[1]);
    if (p->custom_states != NULL && p->custom_owner != -1 && g_game->players[p->custom_owner] != NULL) {
        k = g_gameCoord[0] / g_game->players[p->custom_owner]->data->localcoord[0];
    } else {
        k = g_gameCoord[0] / p->data->localcoord[0];
    }
    h->pos.y = fy * k + y;
    if (Expr_EvalInt(p, &hp->facing, 1) == -1) {
        h->facing = -h->facing;
    }
    Input_SetFlipped(&h->input, h->facing == -1);
    if (h->helper_player != 1) {
        h->screenbound = 0;
        h->movecamera_y = 0;
        h->movecamera_x = 0;
        h->playerpush = 0;
    }
    v = Expr_EvalInt(p, &hp->pausemovetime, 1);
    if (v < 0) {
        v = 0;
        Player_WarnAt(p, sc, "%s", "illegal helper pausemovetime");
    }
    h->pause_movetime[0] = v;
    v = Expr_EvalInt(p, &hp->supermovetime, 1);
    if (v < 0) {
        v = 0;
        Player_WarnAt(p, sc, "%s", "illegal helper supermovetime");
    }
    h->pause_movetime[1] = v;
    h->hitby_attr = 0;
    h->hitby_time = 1;
    h->enemy = Player_FindEnemy(h);
    *(int *)&h->nostandguard = 0;
    *(int *)&h->noshadow = 0;
    h->invisible = 0;
    g = g_game;
    g->player_tick[h->index] = 1;
    g->player_tick[64] = 1;           /* 0x12688 */
    return h;
}

/*
 * 0x0041A230: DestroySelf / the removal of a helper: it lets go of its
 * targets, its explods (with remove_explods) and the explods and players
 * bound to it; players using its PalFX or remapping get their own copy, its
 * own helpers are removed too with `recursive`, and its slot is freed.
 */
void Helper_Remove(Player *p, int recursive, int remove_explods)
{
    GameVars *g;
    int i;

    if (p->helper != 1) {
        return;
    }
    Targets_Release(p, NULL);
    if (remove_explods) {
        Explods_RemoveOwner(g_game->explods, p->id, -1);
    }
    g = g_game;
    Explods_Unbind(p, g->explods);
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *o = g->players[i];
        Player *q;

        if (o == NULL || !o->active) {
            continue;
        }
        if (o->bind_to.valid == 1 && (q = g->players[o->bind_to.index]) != NULL && q->active &&
            q->id == o->bind_to.id && p->active && o->bind_to.id == p->id && o->bind_time != 0) {
            o->bind_y = 0.0f;
            o->bind_time = 0;
            o->bind_x = 0.0f;
            o->bind_facing = 0;
            o->unused_247C = 0;
            o->bind_to.valid = 0;
        }
        if (o->parent_id != p->id) {
            continue;
        }
        if (o->palfx == &p->palfx_own) {
            o->palfx_own = p->palfx_own;
            o->palfx = &o->palfx_own;
        }
        if (o->remap == &p->remap_own) {
            o->remap_own = p->remap_own;
            o->remap = &o->remap_own;
        }
        if (recursive) {
            Helper_Remove(o, recursive, remove_explods);
            g = g_game;
        }
    }
    if (p->afterimage != NULL) {
        (p->afterimage)->running = 0;
        (p->afterimage)->anim = NULL;
        p->afterimage = NULL;
    }
    p->sprites = NULL;
    p->helper_id = 0;
    p->parent_id = 0;
    p->parent = NULL;
    p->root = NULL;
    p->helper_player = 0;
    Players_SetActive(p->index, 0);
}

/* 0x0041A410: NumHelper: the helpers of p's root with helper ID `id` (0: any). */
int Helpers_Count(Player *p, int id)
{
    GameVars *g = g_game;
    int n = 0;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *o = g->players[i];
        if (o != NULL && o->active && (o->root == p || o->root == p->root) && o->helper == 1 &&
            (o->helper_id == id || id == 0)) {
            n++;
        }
    }
    return n;
}

/* 0x0041A480: helper(ID): the first helper of p's root with helper ID `id` (0: any), or NULL. */
Player *Helper_Find(Player *p, int id)
{
    GameVars *g = g_game;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *o = g->players[i];
        if (o != NULL && o->active && (o->root == p || o->root == p->root) && o->helper == 1 &&
            (o->helper_id == id || id == 0)) {
            return g->players[i];
        }
    }
    return NULL;
}
