/*
 * State definitions and the state machine (original module
 * 0x45B210-0x45C4CF; string literals from "statedef " to "ctrl in statedef
 * should be 1 or 0").
 *
 * StateDefs_Load/LoadMore read every [Statedef n] group of a CNS file with
 * the [State n, ...] groups that follow it (each parsed by StateCtrl_Parse,
 * the state controller module) into the character's StateDefs. At run time
 * Player_RunState applies the statedef parameters when a state is entered
 * and runs its controllers; Player_RunSpecialState runs the -1/-2/-3 states.
 *
 * Quirks kept from the original:
 * - States numbered below 0 keep no parameters (type, movetype, physics are 0
 *   and the Exprs empty), only controllers.
 * - The controllers of a statedef are found by selecting the group by its
 *   name: with two statedefs of the same number the first one's controllers
 *   are parsed twice.
 * - Errors leak what was allocated so far (the statedef, its controllers, the
 *   StateDefs block).
 * - A persistence counter decremented below 0 (or from a persistent value of
 *   128 or more, stored in a byte) becomes 0.
 */
#include "mugen.h"

/* 0x0045B210: the players a running state of p refers to. */
void StateRunCtx_Init(StateRunCtx *ctx, Player *p)
{
    GameVars *g = g_game;
    Player *parent;

    ctx->enemy = NULL;
    ctx->partner = NULL;
    ctx->parent = NULL;
    ctx->root = NULL;
    ctx->leader = p;
    if (!g->game_type_m) {
        ctx->enemy = p->enemy;
        if (p->team_side == 1) {
            ctx->leader = g->players[1];
        }
        if (p->team_side == 2) {
            ctx->leader = g->players[2];
        }
    }
    if (p == g->players[1]) {
        ctx->partner = g->players[3];
    } else if (p == g->players[3]) {
        ctx->partner = g->players[1];
    } else if (p == g->players[2]) {
        ctx->partner = g->players[4];
    } else if (p == g->players[4]) {
        ctx->partner = g->players[2];
    }
    parent = NULL;
    if (p->helper == 1 && p->parent != NULL && p->parent->id == p->parent_id && p->parent->active != 0) {
        parent = p->parent;
    }
    ctx->parent = parent;
    ctx->root = p->root;
}

/* ------------------------------------------------------------ loading */

/*
 * 0x0045B5E0: the [Statedef n] group whose header line is header (the
 * selected group of ini) and the [State ...] groups after it. ini->group is
 * restored on success.
 */
int StateDef_Parse(const char *header, StateDef *sd, Ini *ini, CharData *owner)
{
    char prefix[0x40];
    char name[0x40];
    const char *value;
    const char *end;
    const char *h;
    int saved_group = ini->group;
    int number, count, i, k, n;

    number = atoi(header + 10);
    memset(sd, 0, sizeof(StateDef));
    if (g_game->keep_sources != 0) {
        StateCtrlSource *src = calloc(1, sizeof(StateCtrlSource));
        if (src == NULL) {
            g_outOfMemory = 1;
        }
        sd->source = src;
        if (src == NULL) {
            Error_OutOfMemory();
        }
        sd->source->line = Ini_SourceLine(ini->cursor, ini);
    }
    /* the group name without the brackets (not terminated for 64 characters or more) */
    strncpy(name, header + 1, sizeof(name));
    name[strlen(name) - 1] = '\0';
    if (Ini_SelectGroup(ini, name) == -1) {
        Error_Printf("Can't access %s\n", header);
        return 0;
    }
    count = 0;
    h = Ini_NextGroup(ini);
    sprintf(prefix, "state ");
    while (h != NULL && Str_StartsWithI(h + 1, prefix)) {
        count++;
        h = Ini_NextGroup(ini);
    }
    Ini_SelectGroup(ini, name);
    if (count <= 0) {
        Error_Printf("Need at least one state controller\n");
        return 0;
    }
    sd->ctrls = KeyedArray_Create(count, sizeof(StateCtrl));
    if (sd->ctrls == NULL) {
        return 0;
    }

    if (number >= 0) {
        sd->type = STATETYPE_STAND;
        value = Ini_GetValue(ini, "type");
        if (value != NULL) {
            switch (Char_ToLower(*value)) {
            case 'a':
                sd->type = STATETYPE_AIR;
                break;
            case 'c':
                sd->type = STATETYPE_CROUCH;
                break;
            case 'l':
                sd->type = STATETYPE_LIEDOWN;
                break;
            case 's':
                sd->type = STATETYPE_STAND;
                break;
            case 'u':
                sd->type = STATEDEF_UNCHANGED;
                break;
            default:
                Error_Printf("Invalid state type: %s\n", value);
                return 0;
            }
        }
        sd->movetype = MOVETYPE_IDLE;
        value = Ini_GetValue(ini, "movetype");
        if (value != NULL) {
            switch (Char_ToLower(*value)) {
            case 'a':
                sd->movetype = MOVETYPE_ATTACK;
                break;
            case 'h':
                sd->movetype = MOVETYPE_HIT;
                break;
            case 'i':
                sd->movetype = MOVETYPE_IDLE;
                break;
            case 'u':
                sd->movetype = STATEDEF_UNCHANGED;
                break;
            default:
                Error_Printf("Invalid state movetype: %s\n", value);
                return 0;
            }
        }
        Expr_SetInt(&sd->juggle, -1);
        if (!Expr_Compile(Ini_GetValue(ini, "juggle"), &sd->juggle, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_SetInt(&sd->facep2, 0);
        if (!Expr_Compile(Ini_GetValue(ini, "facep2"), &sd->facep2, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        if (!Cns_CompileInt(ini, "hitdefpersist", &sd->hitdefpersist, owner, 0)) {
            return 0;
        }
        if (!Cns_CompileInt(ini, "hitcountpersist", &sd->hitcountpersist, owner, 0)) {
            return 0;
        }
        if (!Cns_CompileInt(ini, "movehitpersist", &sd->movehitpersist, owner, 0)) {
            return 0;
        }
        value = Ini_GetValue(ini, "sprpriority");
        if (value != NULL) {
            if (!Expr_Compile(value, &sd->sprpriority, owner, NULL, EXPR_WANT_INT)) {
                return 0;
            }
            sd->has_sprpriority = 1;
        }
        sd->physics = PHYSICS_NONE;
        value = Ini_GetValue(ini, "physics");
        if (value != NULL) {
            switch (Char_ToLower(*value)) {
            case 'a':
                sd->physics = PHYSICS_AIR;
                break;
            case 'c':
                sd->physics = PHYSICS_CROUCH;
                break;
            case 'n':
                sd->physics = PHYSICS_NONE;
                break;
            case 's':
                sd->physics = PHYSICS_STAND;
                break;
            case 'u':
                sd->physics = STATEDEF_UNCHANGED;
                break;
            default:
                Error_Printf("Invalid state physics: %s\n", value);
                return 0;
            }
        }
        Expr_SetFloat(&sd->velset_x, -FLT_MAX);
        Expr_SetFloat(&sd->velset_y, -FLT_MAX);
        Expr_SetInt(&sd->ctrl, -1);
        Expr_SetInt(&sd->anim, -1);
        Expr_SetInt(&sd->poweradd, 0);
        value = Ini_GetValue(ini, "velset");
        if (value != NULL) {
            if (!Cns_ParseValues(value, "ff", owner, &end, &sd->velset_x, &sd->velset_y)) {
                return 0;
            }
            /* a third value (z) is an error from mugenversion 1.0 on */
            if (*end != '\0' && owner->mugenversion >= g_game->compat_version[1]) {
                return 0;
            }
        }
        if (!Expr_Compile(Ini_GetValue(ini, "ctrl"), &sd->ctrl, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        if (!Expr_Compile(Ini_GetValue(ini, "anim"), &sd->anim, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        if (!Expr_Compile(Ini_GetValue(ini, "poweradd"), &sd->poweradd, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
    }

    sprintf(prefix, "state ");
    for (i = 0; i < count; i++) {
        h = Ini_NextGroup(ini);
        if (h == NULL || !Str_StartsWithI(h + 1, prefix)) {
            Error_Printf("Can't access [%s] controller %i\n", prefix, i + 1);
            return 0;
        }
        /* "[State n, number]": the controller number after the comma, else -1 */
        for (k = 7; h[k] != ',' && h[k] != '\0'; k++) {
        }
        n = h[k] != '\0' ? atoi(h + k + 1) : -1;
        if (!StateCtrl_Parse(ini, sd, n, owner)) {
            Error_Printf("Error parsing %s\n", h);
            return 0;
        }
    }
    ini->group = saved_group;
    return 1;
}

/* Reads the statedefs of ini into list (count of them). */
static int load_statedefs(Ini *ini, KeyedArray *list, int count, CharData *owner)
{
    StateDef sd;
    const char *h;
    int i, number;

    ini->group = -1;
    h = Ini_NextGroup(ini);
    for (i = 0; i < count; i++) {
        while (!Str_StartsWithI(h + 1, "statedef ")) {
            h = Ini_NextGroup(ini);
        }
        number = atoi(h + 10);
        if (!StateDef_Parse(h, &sd, ini, owner)) {
            Error_Printf("Error in %s\n", h);
            return 0;
        }
        KeyedArray_Add(list, &sd, number, 0);
        h = Ini_NextGroup(ini);
    }
    return 1;
}

/* 0x0045B2D0: the statedefs of a CNS file, or NULL. */
StateDefs *StateDefs_Load(Ini *ini, CharData *owner)
{
    StateDefs *defs;
    int count;

    count = Ini_CountGroups(ini, "statedef ");
    if (count == 0) {
        Error_Printf("Need at least one statedef\n");
        return NULL;
    }
    defs = calloc(1, sizeof(StateDefs));
    if (defs == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    defs->list = KeyedArray_Create(count, sizeof(StateDef));
    if (defs->list == NULL) {
        return NULL;
    }
    if (!load_statedefs(ini, defs->list, count, owner)) {
        return NULL;
    }
    return defs;
}

/* 0x0045B470: adds the statedefs of another CNS file; 0 on errors or when it has none. */
int StateDefs_LoadMore(Ini *ini, StateDefs *defs, CharData *owner)
{
    int count;

    count = Ini_CountGroups(ini, "statedef ");
    if (count == 0) {
        return 0;
    }
    if (!KeyedArray_Grow(defs->list, defs->list->count + count)) {
        return 0;
    }
    return load_statedefs(ini, defs->list, count, owner);
}

/* 0x0045BCE0 */
void StateDefs_Free(StateDefs *defs)
{
    KeyedArray *list;
    StateDef *sd;
    StateCtrl *sc;

    if (defs == NULL) {
        return;
    }
    list = defs->list;
    if (list->count != 0) {
        for (sd = KeyedArray_First(list); sd != NULL; sd = KeyedArray_Next(list)) {
            if (sd->source != NULL) {
                if (sd->source->statedef != NULL) {
                    free(sd->source->statedef);
                }
                free(sd->source);
                sd->source = NULL;
            }
            if (sd->ctrls->count != 0) {
                for (sc = KeyedArray_First(sd->ctrls); sc != NULL; sc = KeyedArray_Next(sd->ctrls)) {
                    StateCtrl_Free(sc);
                }
            }
            Expr_Free(&sd->juggle);
            Expr_Free(&sd->facep2);
            Expr_Free(&sd->hitdefpersist);
            Expr_Free(&sd->hitcountpersist);
            Expr_Free(&sd->movehitpersist);
            Expr_Free(&sd->sprpriority);
            Expr_Free(&sd->velset_x);
            Expr_Free(&sd->velset_y);
            Expr_Free(&sd->ctrl);
            Expr_Free(&sd->anim);
            Expr_Free(&sd->poweradd);
            KeyedArray_Free(sd->ctrls);
        }
    }
    KeyedArray_Free(list);
    free(defs);
}

/* ------------------------------------------------------------ running */

/*
 * 0x0045BE80: runs the current state of p: on its first tick (Time 0) the
 * statedef parameters are applied, then every controller whose triggers hold
 * (and that is not waiting for its persistent count) is run until one stops
 * the state. During a hit pause only ignorehitpause controllers run. Returns
 * 1 when a controller changed the state.
 */
int Player_RunState(Player *p)
{
    StateRunCtx ctx;
    StateDefs *states;
    StateDef *sd = NULL;
    StateCtrl *sc;
    unsigned char *timer;
    int result = 0;
    int index, hitdefpersist, value, r;
    float vel;

    StateRunCtx_Init(&ctx, p);
    states = p->custom_states != NULL ? p->custom_states : p->states;
    index = KeyedArray_Find(states->list, p->state_no, 0);
    if (index != -1) {
        sd = (StateDef *)(states->list->data + states->list->elem_size * index);
    }
    if (p->hitpause_time <= 0) {
        if (sd == NULL) {
            p->state_exists = 0;
            if (p->state_time == 0) {
                p->uniqhitcount = 0;
                p->hitcount = 0;
                p->move_contact = 0;
                p->move_contact_time = 0;
                p->hitdef.attr = 0;
            }
            return 0;
        }
        p->state_exists = 1;
        if (p->state_time == 0) {
            /* entering the state */
            if (!Expr_EvalInt(p, &sd->hitcountpersist, 1)) {
                p->hitcount = 0;
                p->uniqhitcount = 0;
            }
            if (!Expr_EvalInt(p, &sd->movehitpersist, 1)) {
                p->move_contact = 0;
                p->move_contact_time = 0;
            }
            hitdefpersist = Expr_EvalInt(p, &sd->hitdefpersist, 1);
            if (hitdefpersist == 0) {
                p->hitdef.attr = 0;
                p->hitdef.active = 0;
            }
            if (sd->has_sprpriority) {
                p->sprpriority = Expr_EvalInt(p, &sd->sprpriority, 1);
            }
            for (index = 0; index < sd->ctrls->count; index++) {
                p->sctrl_wait[index] = 0;
            }
            if (sd->ctrls->count > STATEDEF_MAX_CTRLS) {
                Error_Printf("State %i can only have max of %i controllers\n", p->state_no, STATEDEF_MAX_CTRLS);
                Error_Exit();
            }
            if (sd->type != STATEDEF_UNCHANGED) {
                p->statetype = sd->type;
            }
            if (sd->movetype != STATEDEF_UNCHANGED) {
                p->movetype = sd->movetype;
            }
            if (sd->physics != STATEDEF_UNCHANGED) {
                p->physics = sd->physics;
            }
            if (ctx.enemy != NULL && Expr_EvalInt(p, &sd->facep2, 1) && Player_DistX(p, ctx.enemy) < 0.0f) {
                p->facing = -p->facing;
                Player_UpdateInputFacing(p);
            }
            value = Expr_EvalInt(p, &sd->juggle, 1);
            if (value < -1) {
                Player_Warn(p, "Illegal juggle points");
            } else if (value != -1) {
                p->hitdef.juggle = value;
            }
            if (hitdefpersist == 0) {
                p->hitdef.guard_dist = p->consts.attack_dist;
            }
            vel = Expr_EvalFloat(p, &sd->velset_x);
            if (vel != -FLT_MAX) {
                p->vel.x = Player_FromLocalCoord(p, vel) * p->facing;
            }
            vel = Expr_EvalFloat(p, &sd->velset_y);
            if (vel != -FLT_MAX) {
                p->vel.y = Player_FromLocalCoord(p, vel);
            }
            value = Expr_EvalInt(p, &sd->anim, 1);
            if (value < -1) {
                Player_Warn(p, "illegal anim number");
            } else if (value != -1) {
                p->anim->anim = p->data->anim;
                p->anim_from_owner = 0;
                Player_SetAction(p, value);
            }
            value = Expr_EvalInt(p, &sd->ctrl, 1);
            if (value < -1) {
                Player_Warn(p, "ctrl in statedef should be 1 or 0");
            } else if (value > 1) {
                Player_Warn(p, "ctrl in statedef should be 1 or 0");
                p->ctrl = 1;
            } else if (value != -1) {
                p->ctrl = value;
            }
            value = Expr_EvalInt(p, &sd->poweradd, 1);
            Player_SetPower(ctx.leader, *ctx.leader->power_ref + value);
        }
    }

    if (sd == NULL || sd->ctrls->count == 0) {
        return result;
    }
    timer = p->sctrl_wait;
    for (sc = KeyedArray_First(sd->ctrls); sc != NULL; sc = KeyedArray_Next(sd->ctrls), timer++) {
        if (*timer == 0 && (p->hitpause_time == 0 || sc->ignorehitpause != 0)) {
            p->ai_cmds[0] = -1;
            if (Trigger_TestList(sc->triggers, p, sc->trigger_groups)) {
                *timer = sc->persistent > 0 ? (unsigned char)sc->persistent : 1;
                r = StateCtrl_Run(p, sc, &ctx);
                if (r & STATECTRL_CHANGED) {
                    result = 1;
                }
                if (r & STATECTRL_STOP) {
                    return result;
                }
            }
        }
        if (sc->persistent > 0 && (p->hitpause_time == 0 || sc->ignorehitpause != 0)) {
            unsigned char t = (unsigned char)(*timer - 1);
            *timer = (t & 0x80) ? 0 : t;
        }
    }
    return result;
}

/*
 * 0x0045C2C0: runs the controllers of the special state number (-1, -2, -3)
 * of p, from its own states with RUNSTATE_OWN, else from the custom states
 * if any; RUNSTATE_ALWAYS / RUNSTATE_CUSTOM select when it runs at all.
 */
void Player_RunSpecialState(Player *p, int state, int flags)
{
    StateRunCtx ctx;
    StateDefs *states;
    StateDef *sd;
    StateCtrl *sc;
    int index;

    StateRunCtx_Init(&ctx, p);
    states = p->custom_states != NULL ? p->custom_states : p->states;
    if (flags & RUNSTATE_OWN) {
        states = p->states;
    }
    if (states == NULL) {
        return;
    }
    if (!(flags & RUNSTATE_ALWAYS) && p->custom_states == NULL) {
        return;
    }
    if (!(flags & RUNSTATE_CUSTOM) && p->custom_states != NULL) {
        return;
    }
    index = KeyedArray_Find(states->list, state, 0);
    if (index == -1) {
        return;
    }
    sd = (StateDef *)(states->list->data + states->list->elem_size * index);
    if (sd == NULL || sd->ctrls->count == 0) {
        return;
    }
    for (sc = KeyedArray_First(sd->ctrls); sc != NULL; sc = KeyedArray_Next(sd->ctrls)) {
        if (p->hitpause_time == 0 || sc->ignorehitpause != 0) {
            p->ai_cmds[0] = -1;
            if (Trigger_TestList(sc->triggers, p, sc->trigger_groups) && (StateCtrl_Run(p, sc, &ctx) & STATECTRL_STOP)) {
                return;
            }
        }
    }
}

/* 0x0045C3D0: makes p use the states (and commands) described by info. */
void Player_SetCustomState(Player *p, const CustomStateInfo *info)
{
    CmdSet *cmds = p->cmds;

    p->custom_states = info->states;
    p->custom_owner = info->index;
    p->custom_anim = info->anim;
    p->custom_cmds = info->cmds;
    if (cmds != info->cmds) {
        memset(p->cmd_active, 0, sizeof(p->cmd_active));
        p->cmds = info->cmds;
    }
}

/* 0x0045C420: back to the player's own states and commands. */
void Player_ClearCustomState(Player *p)
{
    if (p->cmds == p->custom_cmds) {
        memset(p->cmd_active, 0, sizeof(p->cmd_active));
        p->cmds = &p->data->cmds;
    }
    p->custom_states = NULL;
    p->custom_owner = -1;
    p->custom_anim = NULL;
    p->custom_cmds = NULL;
}

/* 0x0045C480: where the states p uses come from (its own, or those of a custom state). */
void Player_GetCustomState(Player *p, CustomStateInfo *out)
{
    if (p->custom_states == NULL) {
        out->states = p->data->states;
        out->index = p->index;
        out->anim = p->data->anim;
        out->cmds = &p->data->cmds;
        return;
    }
    out->states = p->custom_states;
    out->index = p->custom_owner;
    out->anim = p->custom_anim;
    out->cmds = p->custom_cmds;
}
