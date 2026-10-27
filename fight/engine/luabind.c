/*
 * The Lua player bindings (original module 0x41E880-0x4214FF): the "player"
 * library (players by index), the methods of the "mugen.player" userdata
 * (a reference to a player by index and PlayerID, LuaPlayerRef) and the
 * helpers that store the character constants in its Lua table.
 */
#include "mugen.h"

#define LUA_REGISTRYINDEX   (-10000)
#define LUA_TNIL            0
#define LUA_TBOOLEAN        1
#define LUA_TTABLE          5
#define COORD_TYPE_1D       1   /* Coord.type of a 1D value (Vector:vec1) */

/* The userdata of a "mugen.player" (0x10 bytes). */
typedef struct LuaPlayerRef {
    int valid;                  /* 0x00 1 */
    int id;                     /* 0x04 Player.id */
    int index;                  /* 0x08 Player.index */
    int flag;                   /* 0x0C 0 (PlayerRef.flag) */
} LuaPlayerRef;

/* The player at idx of GameVars.players if it is in the used range and active, else NULL. */
static Player *LuaBind_PlayerAt(int idx)
{
    GameVars *g = g_game;
    Player *p;

    if (idx < g->players_first || idx > g->players_last) {
        return NULL;
    }
    p = g->players[idx];
    if (p == NULL || !p->active) {
        return NULL;
    }
    return p;
}

/* ---- the "player" library ---- */

/* 0x0041E880: player.getplayer(idx): a reference to the active player at idx. */
int LuaPlayer_GetPlayer(struct lua_State *L)
{
    int idx = luaL_checkinteger(L, 1);
    Player *p = LuaBind_PlayerAt(idx);
    LuaPlayerRef *ref;

    if (p == NULL) {
        return luaL_error(L, "Invalid player at idx %d\n", idx);
    }
    ref = lua_newuserdata(L, sizeof(LuaPlayerRef));
    if (p->active) {
        ref->valid = 1;
        ref->id = p->id;
        ref->index = p->index;
        ref->flag = 0;
    }
    lua_getfield(L, LUA_REGISTRYINDEX, "mugen.player");
    lua_setmetatable(L, -2);
    return 1;
}

/* 0x0041E930: player.getplayerlist(): the indices of the active players. */
int LuaPlayer_GetPlayerList(struct lua_State *L)
{
    GameVars *g;
    int n = 1;
    int i;

    lua_createtable(L, 0, 0);
    g = g_game;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            lua_pushinteger(L, i);
            lua_rawseti(L, -2, n);
            g = g_game;
            n++;
        }
    }
    return 1;
}

/* 0x0041E9B0: player.indexfromid(id): the index of the active player with the PlayerID, or nil. */
int LuaPlayer_IndexFromId(struct lua_State *L)
{
    int i = Player_FindIndexById(luaL_checkinteger(L, 1));

    if (i == -1) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, i);
    }
    return 1;
}

/* 0x0041E9F0: player.indexisvalid(idx) */
int LuaPlayer_IndexIsValid(struct lua_State *L)
{
    lua_pushboolean(L, LuaBind_PlayerAt(luaL_checkinteger(L, 1)) != NULL);
    return 1;
}

/* 0x0041EA50: player.enabled(idx): idx up to GameVars.player_slots (not checked against the used range). */
int LuaPlayer_Enabled(struct lua_State *L)
{
    int idx = luaL_checkinteger(L, 1);
    Player *p;

    if (idx < 0 || idx > g_game->player_slots) {
        luaL_error(L, "Invalid player at idx %d\n", idx);
    }
    p = g_game->players[idx];
    lua_pushboolean(L, p != NULL && p->active);
    return 1;
}

/* 0x0041EAD0: player.enableset(idx, enabled) (Players_SetActive). */
int LuaPlayer_EnableSet(struct lua_State *L)
{
    int idx = luaL_checkinteger(L, 1);
    int active;

    if (lua_type(L, 2) == LUA_TBOOLEAN) {
        active = lua_toboolean(L, 2);
    } else {
        luaL_argerror(L, 2, "(boolean expected)");
        active = 0;
    }
    if (idx < 0 || idx > g_game->player_slots) {
        luaL_error(L, "Invalid player at idx %d\n", idx);
    }
    Players_SetActive(idx, active);
    return 0;
}

/* ---- the "mugen.player" methods ---- */

/*
 * 0x0041EB40: the player of the reference at stack index 1; a Lua error when
 * it is gone (inactive, or another player at its index).
 */
Player *LuaPlayer_Check(struct lua_State *L)
{
    LuaPlayerRef *ref = luaL_checkudata(L, 1, "mugen.player");

    if (ref->valid == 1) {
        Player *p = g_game->players[ref->index];
        if (p != NULL && p->active && p->id == ref->id) {
            return p;
        }
    }
    luaL_error(L, "Invalid player reference ID %d\n", ref->id);
    return NULL;
}

/* 0x0041EBA0 */
int LuaPlayer_Life(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->life);
    return 1;
}

/* 0x0041EBD0 */
int LuaPlayer_LifeSet(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    Player_SetLife(p, luaL_checkinteger(L, 2));
    return 0;
}

/* 0x0041EC00 */
int LuaPlayer_LifeMax(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->life_max);
    return 1;
}

/*
 * The leader of p's team (players[team_side]). Quirk: for a player without a
 * team the original reads an uninitialized pointer (whatever ecx held on
 * entry); p itself is used here.
 */
static Player *LuaBind_Leader(Player *p)
{
    Player *t = p;

    if (p->team_side == 1) {
        t = g_game->players[1];
    }
    if (p->team_side == 2) {
        t = g_game->players[2];
    }
    return t;
}

/* 0x0041EC30: the team's power. */
int LuaPlayer_Power(struct lua_State *L)
{
    lua_pushinteger(L, *LuaBind_Leader(LuaPlayer_Check(L))->power_ref);
    return 1;
}

/* 0x0041EC80 */
int LuaPlayer_PowerSet(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    Player_SetPower(p, luaL_checkinteger(L, 2));
    return 0;
}

/* 0x0041ECB0 */
int LuaPlayer_PowerMax(struct lua_State *L)
{
    lua_pushinteger(L, *LuaBind_Leader(LuaPlayer_Check(L))->power_max_ref);
    return 1;
}

/* 0x0041ED00 */
int LuaPlayer_WinPerfect(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g = g_game;

    lua_pushboolean(L, g->win_type != 0 && g->round_winner == p->team_side && Team_HasFullLife(p->team_side));
    return 1;
}

/* 0x0041ED60 */
int LuaPlayer_Win(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushboolean(L, g_game->round_winner == p->team_side);
    return 1;
}

/* 0x0041ED90 */
int LuaPlayer_WinKO(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g = g_game;

    lua_pushboolean(L, g->win_type == 1 && g->round_winner == p->team_side);
    return 1;
}

/* 0x0041EDE0 */
int LuaPlayer_WinTime(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g = g_game;

    lua_pushboolean(L, g->win_type == 3 && g->round_winner == p->team_side);
    return 1;
}

/* 0x0041EE30 */
int LuaPlayer_Lose(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int w = g_game->round_winner;

    lua_pushboolean(L, w != p->team_side && w != 0);
    return 1;
}

/* 0x0041EE80 */
int LuaPlayer_LoseKO(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g = g_game;

    lua_pushboolean(L, g->win_type == 1 && g->round_winner != p->team_side && g->round_winner != 0);
    return 1;
}

/* 0x0041EEE0 */
int LuaPlayer_LoseTime(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g = g_game;

    lua_pushboolean(L, g->win_type == 3 && g->round_winner != p->team_side && g->round_winner != 0);
    return 1;
}

/* 0x0041EF40 */
int LuaPlayer_DrawGame(struct lua_State *L)
{
    GameVars *g;

    LuaPlayer_Check(L);
    g = g_game;
    lua_pushboolean(L, g->round_winner == 0 && g->win_type != 0);
    return 1;
}

/* 0x0041EF90 */
int LuaPlayer_MatchOver(struct lua_State *L)
{
    LuaPlayer_Check(L);
    lua_pushboolean(L, Match_CheckOver() != 0);
    return 1;
}

/* The action of the animation state. */
static AnimAction *LuaBind_Action(const AnimState *st)
{
    KeyedArray *actions = st->anim->actions;

    return (AnimAction *)(actions->data + actions->elem_size * st->action);
}

/* 0x0041EFC0: action time minus the action's total time. */
int LuaPlayer_AnimTime(struct lua_State *L)
{
    AnimState *st = LuaPlayer_Check(L)->anim;

    lua_pushinteger(L, st->time - LuaBind_Action(st)->total_time);
    return 1;
}

/* 0x0041F000: the action number. */
int LuaPlayer_Anim(struct lua_State *L)
{
    AnimState *st = LuaPlayer_Check(L)->anim;

    lua_pushinteger(L, st->anim->actions->slots[st->action].key2);
    return 1;
}

/* 0x0041F040 */
int LuaPlayer_AnimElemNo(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int offset = luaL_checkinteger(L, 2);

    lua_pushinteger(L, Anim_ElemAtOffset(p->anim, offset));
    return 1;
}

/* 0x0041F080: action time minus the start of element n (1-based). */
int LuaPlayer_AnimElemTime(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int n = luaL_checkinteger(L, 2);
    AnimState *st = p->anim;
    AnimAction *act = LuaBind_Action(st);
    int i = n - 1;
    int start;

    if (i < 0 || i >= act->elem_count) {
        return luaL_error(L, "Invalid elemno %d", n);
    }
    start = ((AnimElem *)(act->elems->data + act->elems->elem_size * i))->start;
    if (start == -1) {
        return luaL_error(L, "Invalid elemno %d", n);
    }
    lua_pushinteger(L, st->time - start);
    return 1;
}

/* 0x0041F100 */
int LuaPlayer_AnimExist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int action = luaL_checkinteger(L, 2);

    lua_pushboolean(L, KeyedArray_Find(p->anim->anim->actions, 0, action) != -1);
    return 1;
}

/* 0x0041F150: in the character's own animations. */
int LuaPlayer_SelfAnimExist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int action = luaL_checkinteger(L, 2);
    Anim *saved = p->anim->anim;
    int found;

    p->anim->anim = p->data->anim;
    found = KeyedArray_Find(p->anim->anim->actions, 0, action) != -1;
    p->anim->anim = saved;
    lua_pushboolean(L, found);
    return 1;
}

static void LuaBind_PushCoord(struct lua_State *L, const Coord *c)
{
    Lua_PushVector(L, c->x, c->y, c->z, c->unit, c->type, c->pad_24);
}

/* 0x0041F1B0 */
int LuaPlayer_Pos(struct lua_State *L)
{
    Coord c = LuaPlayer_Check(L)->pos;

    LuaBind_PushCoord(L, &c);
    return 1;
}

/* 0x0041F1F0: the velocity, x towards the facing. */
int LuaPlayer_Vel(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    Coord c = p->vel;

    c.x = p->facing * c.x;
    LuaBind_PushCoord(L, &c);
    return 1;
}

/* 0x0041F250 */
int LuaPlayer_ScreenPos(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    Coord tmp;
    Coord c = *Player_GetScreenPos(p, &tmp);

    LuaBind_PushCoord(L, &c);
    return 1;
}

/* Half the game width in stage units at the current zoom. */
static double LuaBind_HalfWidth(void)
{
    GameVars *g = g_game;
    int w = Ftol(g->game_coord[0]);

    return (double)w / g->camera_zoom * 0.5;
}

/* 0x0041F3A0: the screen edge behind the player (camera x -/+ half the width). */
int LuaPlayer_BackEdge(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    Coord tmp;
    const Coord *c;
    double v;

    if (p->facing == 1) {
        c = Camera_GetPos(&tmp);
        v = c->x - LuaBind_HalfWidth();
    } else {
        c = Camera_GetPos(&tmp);
        v = LuaBind_HalfWidth() + c->x;
    }
    lua_pushnumber(L, v);
    return 1;
}

/* 0x0041F440: the screen edge in front of the player. */
int LuaPlayer_FrontEdge(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    Coord tmp;
    const Coord *c;
    double v;

    if (p->facing == -1) {
        c = Camera_GetPos(&tmp);
        v = c->x - LuaBind_HalfWidth();
    } else {
        c = Camera_GetPos(&tmp);
        v = LuaBind_HalfWidth() + c->x;
    }
    lua_pushnumber(L, v);
    return 1;
}

/* 0x0041F4E0: Player_EdgeDist(p, 0, 1) inlined. */
int LuaPlayer_FrontEdgeDist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushnumber(L, Player_EdgeDist(p, 0, 1));
    return 1;
}

/* 0x0041F590: Player_EdgeDist(p, 0, 0) inlined. */
int LuaPlayer_BackEdgeDist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushnumber(L, Player_EdgeDist(p, 0, 0));
    return 1;
}

/* 0x0041F640 */
int LuaPlayer_FrontEdgeBodyDist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushnumber(L, Player_EdgeDist(p, 1, 1));
    return 1;
}

/* 0x0041F680 */
int LuaPlayer_BackEdgeBodyDist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushnumber(L, Player_EdgeDist(p, 1, 0));
    return 1;
}

/* 0x0041F2A0 */
int LuaPlayer_RoundsExisted(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->rounds_existed);
    return 1;
}

/* 0x0041F2D0 */
int LuaPlayer_TeamSide(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->team_side);
    return 1;
}

/*
 * 0x0041F2F0: "single", "simul", "tag" or "turns" (the TeamMode of the
 * player's team; quirk: "turns" whenever the team has 2 or more members).
 */
int LuaPlayer_TeamMode(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g = g_game;
    int mode = g->match_members[p->team_index].mode;

    if (g->teams[p->team_index].member_count >= 2) {
        mode = 3;
    }
    lua_pushstring(L, g_teamModeNames[mode]);
    return 1;
}

/* 0x0041F340: the human side in arcade modes, else team 1. */
int LuaPlayer_IsHomeTeam(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int side = g_game->arcade_side;

    if (side != 0) {
        lua_pushboolean(L, p->team_side == 3 - side);
    } else {
        lua_pushboolean(L, p->team_side == 1);
    }
    return 1;
}

/* 0x0041F6C0 */
int LuaPlayer_Alive(struct lua_State *L)
{
    lua_pushboolean(L, LuaPlayer_Check(L)->alive);
    return 1;
}

/* 0x0041F6F0 */
int LuaPlayer_AiEnabled(struct lua_State *L)
{
    lua_pushboolean(L, LuaPlayer_Check(L)->ai);
    return 1;
}

/* 0x0041F720 */
int LuaPlayer_AiEnableSet(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    if (lua_type(L, 2) == LUA_TBOOLEAN) {
        p->ai = lua_toboolean(L, 2);
        return 0;
    }
    luaL_argerror(L, 2, "(boolean expected)");
    p->ai = 0;
    return 0;
}

/* 0x0041F770: 0 for a human player or a cooperative training dummy. */
int LuaPlayer_AiLevel(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    if (p->ai && !(g_game->team_variant == 3 && p->dummy_control == 0)) {
        lua_pushinteger(L, p->ai_level);
    } else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

/* 0x0041F7D0 */
int LuaPlayer_HitCount(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->hitcount);
    return 1;
}

/* 0x0041F800 */
int LuaPlayer_UniqHitCount(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->uniqhitcount);
    return 1;
}

/* Ticks since the move contact of the kind (Player.move_contact / move_contact_time), 0 for another one. */
static int LuaBind_MoveTime(Player *p, int kind)
{
    return p->move_contact == kind ? p->move_contact_time + 1 : 0;
}

/* 0x0041F830 */
int LuaPlayer_MoveHit(struct lua_State *L)
{
    lua_pushinteger(L, LuaBind_MoveTime(LuaPlayer_Check(L), 2));
    return 1;
}

/* 0x0041F870 */
int LuaPlayer_MoveGuarded(struct lua_State *L)
{
    lua_pushinteger(L, LuaBind_MoveTime(LuaPlayer_Check(L), 1));
    return 1;
}

/* 0x0041F8B0 */
int LuaPlayer_MoveContact(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int c = p->move_contact;

    lua_pushinteger(L, c == 2 || c == 1 ? p->move_contact_time + 1 : 0);
    return 1;
}

/* 0x0041F900 */
int LuaPlayer_MoveReversed(struct lua_State *L)
{
    lua_pushinteger(L, LuaBind_MoveTime(LuaPlayer_Check(L), 4));
    return 1;
}

/* 0x0041F940 */
int LuaPlayer_InGuardDist(struct lua_State *L)
{
    lua_pushboolean(L, Player_InEnemyGuardDist(LuaPlayer_Check(L)));
    return 1;
}

/* 0x0041F970 */
int LuaPlayer_StateNo(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->state_no);
    return 1;
}

/* 0x0041F9A0 */
int LuaPlayer_PrevStateNo(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->prev_state_no);
    return 1;
}

/* 0x0041F9D0: "S", "C", "A", "L" or "unknown". */
int LuaPlayer_StateType(struct lua_State *L)
{
    const char *s;

    switch (LuaPlayer_Check(L)->statetype) {
    case STATETYPE_STAND:
        s = "S";
        break;
    case STATETYPE_CROUCH:
        s = "C";
        break;
    case STATETYPE_AIR:
        s = "A";
        break;
    case STATETYPE_LIEDOWN:
        s = "L";
        break;
    default:
        s = "unknown";
        break;
    }
    lua_pushstring(L, s);
    return 1;
}

/* 0x0041FA70: "I", "A", "H" or "unknown". */
int LuaPlayer_MoveType(struct lua_State *L)
{
    const char *s;

    switch (LuaPlayer_Check(L)->movetype) {
    case MOVETYPE_IDLE:
        s = "I";
        break;
    case MOVETYPE_ATTACK:
        s = "A";
        break;
    case MOVETYPE_HIT:
        s = "H";
        break;
    default:
        s = "unknown";
        break;
    }
    lua_pushstring(L, s);
    return 1;
}

/* 0x0041FAF0 */
int LuaPlayer_Ctrl(struct lua_State *L)
{
    lua_pushboolean(L, LuaPlayer_Check(L)->ctrl);
    return 1;
}

/* 0x0041FB20 */
int LuaPlayer_Var(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    unsigned int i = (unsigned int)luaL_checkinteger(L, 2);

    if (i > PLAYER_VAR_COUNT - 1) {
        return luaL_error(L, "%s", "var index out of range");
    }
    lua_pushinteger(L, p->var[i]);
    return 1;
}

/* 0x0041FB70 */
int LuaPlayer_FVar(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    unsigned int i = (unsigned int)luaL_checkinteger(L, 2);

    if (i > PLAYER_FVAR_COUNT - 1) {
        return luaL_error(L, "%s", "fvar index out of range");
    }
    lua_pushnumber(L, p->fvar[i]);
    return 1;
}

/* 0x0041FBD0 */
int LuaPlayer_SysVar(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    unsigned int i = (unsigned int)luaL_checkinteger(L, 2);

    if (i > 4) {
        return luaL_error(L, "%s", "sysvar index out of range");
    }
    lua_pushinteger(L, p->sysvar[i]);
    return 1;
}

/* 0x0041FC20: quirk: its range error says "fvar". */
int LuaPlayer_SysFVar(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    unsigned int i = (unsigned int)luaL_checkinteger(L, 2);

    if (i > 4) {
        return luaL_error(L, "%s", "fvar index out of range");
    }
    lua_pushnumber(L, ((float *)&p->sysfvar[0])[i]);
    return 1;
}

/* 0x0041FC80: hitdefattrmatch(attr string): attacking with a HitDef of those attributes. */
int LuaPlayer_HitDefAttrMatch(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    const char *end = NULL;
    int match = 0;
    int attr = HitDefAttr_Parse(luaL_checklstring(L, 2, NULL), &end);

    if (attr == -1 || (end != NULL && *end != '\0')) {
        return luaL_error(L, "%s", "Invalid HitDefAttr string");
    }
    if (p->movetype == MOVETYPE_ATTACK) {
        unsigned int m = p->hitdef.attr & (unsigned int)attr;
        if ((m & 7) && (m & ~7u)) {
            match = 1;
        }
    }
    lua_pushboolean(L, match);
    return 1;
}

/* 0x0041FD10: numproj([projid]) */
int LuaPlayer_NumProj(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int id;

    if (lua_gettop(L) < 2 || (id = luaL_checkinteger(L, 2)) == 0) {
        lua_pushinteger(L, Projectiles_Count((ProjectileList *)p->projectiles));
        return 1;
    }
    if (id > 0) {
        lua_pushinteger(L, Projectiles_CountId((ProjectileList *)p->projectiles, id));
        return 1;
    }
    luaL_error(L, "negative projid %d", id);
    lua_pushinteger(L, 0);
    return 1;
}

/* The ticks since the last projectile contact of the kinds (ProjHitTime... with projid). */
static int LuaBind_ProjTime(struct lua_State *L, int kind)
{
    Player *p = LuaPlayer_Check(L);
    int id = 0;
    int c;
    int hit;

    if (lua_gettop(L) >= 2) {
        id = luaL_checkinteger(L, 2);
        if (id < 0) {
            luaL_error(L, "negative projid %d", id);
            lua_pushinteger(L, 0);
            return 1;
        }
    }
    c = p->proj_contact;
    hit = kind == 0 ? c != 0 && c != 3 : c == kind;
    if (hit && (p->proj_contact_id == id || id == 0)) {
        lua_pushinteger(L, p->proj_contact_time);
    } else {
        lua_pushinteger(L, -1);
    }
    return 1;
}

/* 0x0041FDB0 */
int LuaPlayer_ProjCancelTime(struct lua_State *L)
{
    return LuaBind_ProjTime(L, 3);
}

/* 0x0041FE50 */
int LuaPlayer_ProjHitTime(struct lua_State *L)
{
    return LuaBind_ProjTime(L, 2);
}

/* 0x0041FEF0 */
int LuaPlayer_ProjGuardedTime(struct lua_State *L)
{
    return LuaBind_ProjTime(L, 1);
}

/* 0x0041FF90: hit or guarded (not cancelled). */
int LuaPlayer_ProjContactTime(struct lua_State *L)
{
    return LuaBind_ProjTime(L, 0);
}

/* 0x00420030 */
int LuaPlayer_Name(struct lua_State *L)
{
    lua_pushstring(L, LuaPlayer_Check(L)->data->name);
    return 1;
}

/* 0x00420050 */
int LuaPlayer_AuthorName(struct lua_State *L)
{
    lua_pushstring(L, LuaPlayer_Check(L)->data->author);
    return 1;
}

/* 0x00420070 */
int LuaPlayer_NumPartner(struct lua_State *L)
{
    lua_pushinteger(L, Player_CountPartners(LuaPlayer_Check(L)));
    return 1;
}

/* 0x004200A0 */
int LuaPlayer_NumEnemy(struct lua_State *L)
{
    lua_pushinteger(L, Player_CountEnemies(LuaPlayer_Check(L)));
    return 1;
}

/* 0x004200D0 */
int LuaPlayer_Id(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->id);
    return 1;
}

/* 0x004200F0 */
int LuaPlayer_Facing(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->facing);
    return 1;
}

/* 0x00420120 */
int LuaPlayer_HitFall(struct lua_State *L)
{
    lua_pushboolean(L, LuaPlayer_Check(L)->gethit_fall);
    return 1;
}

/* 0x00420150 */
int LuaPlayer_HitShakeOver(struct lua_State *L)
{
    lua_pushboolean(L, LuaPlayer_Check(L)->gethit_shaketime <= 0);
    return 1;
}

/* 0x00420180 */
int LuaPlayer_HitOver(struct lua_State *L)
{
    lua_pushboolean(L, LuaPlayer_Check(L)->gethit_hittime < 0);
    return 1;
}

/* 0x004201B0: a 2D Vector (x towards the facing). */
int LuaPlayer_HitVel(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    Lua_PushVector(L, (double)p->facing * p->gethit_vel_x, p->gethit_vel_y, 0.0, 1.0, COORD_TYPE_2D, 0);
    return 1;
}

/* 0x00420220 */
int LuaPlayer_HitPauseTime(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->hitpause_time);
    return 1;
}

/* 0x00420250 */
int LuaPlayer_CanRecover(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushboolean(L, p->gethit_fall_recover != 0 && p->gethit_fall_time >= p->gethit_fall_recovertime);
    return 1;
}

/* 0x004202A0: 1-based. */
int LuaPlayer_PalNo(struct lua_State *L)
{
    lua_pushinteger(L, LuaPlayer_Check(L)->pal_no + 1);
    return 1;
}

/* 0x004202D0: numexplod([id]): the player's explods (all for id -1). */
int LuaPlayer_NumExplod(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    SysDrawList *list;
    int id = -1;
    int n = 0;
    int i;

    if (lua_gettop(L) >= 2) {
        id = luaL_checkinteger(L, 2);
        if (id < -1) {
            Player_Warn(p, "illegal explod ID for numexplod");
            id = -1;
        }
    }
    list = g_game->explods;
    for (i = 0; i < list->capacity; i++) {
        const SysDrawItem *e = &list->items[i];
        if (e->used && (e->id == id || id == -1) && e->owner_id == p->id) {
            n++;
        }
    }
    lua_pushinteger(L, n);
    return 1;
}

/* 0x00420390: numtarget([id]) */
int LuaPlayer_NumTarget(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int id = -1;

    if (lua_gettop(L) >= 2) {
        id = luaL_checkinteger(L, 2);
        if (id < -1) {
            Player_Warn(p, "illegal target ID for numtarget");
            id = -1;
        }
    }
    lua_pushinteger(L, Targets_Count(p->targets, id));
    return 1;
}

/*
 * 0x00420420: gethitvar(name): a number, an integer or (for the values
 * scaled to the character's units) a 1D Vector.
 */
int LuaPlayer_GetHitVar(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int scaled = 0;
    int is_float = 0;
    union {
        int index;
        float f;
    } u;
    int ival;
    const char *name;

    u.index = 0;
    name = luaL_checklstring(L, 2, NULL);
    if (!GetHitVar_Lookup(name, p->data->mugenversion < g_game->compat_version[1], &u.index, &is_float)) {
        Player_Warnf(p, "invalid gethitvar name %s", name);
        return luaL_error(L, "invalid gethitvar name %s", name);
    }
    if (!GetHitVar_Get(p, u.index, &ival, &u.f, &is_float, &scaled)) {
        return luaL_error(L, "illegal gethitvar");
    }
    if (scaled) {
        if (!is_float) {
            u.f = (float)ival;
        }
        Lua_PushVector(L, u.f, 0.0, 0.0, 1.0, COORD_TYPE_1D, 0);
        return 1;
    }
    if (is_float) {
        lua_pushnumber(L, u.f);
    } else {
        lua_pushinteger(L, ival);
    }
    return 1;
}

/* 0x00420590: ishelper([id]) */
int LuaPlayer_IsHelper(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int id;

    if (lua_gettop(L) >= 2) {
        id = luaL_checkinteger(L, 2);
        if (id < 0) {
            Player_Warn(p, "illegal helper ID for numhelper");
        } else if (id != 0) {
            lua_pushboolean(L, id == p->helper_id);
            return 1;
        }
    }
    lua_pushboolean(L, p->helper == 1);
    return 1;
}

/* 0x00420650: numhelper([id]) */
int LuaPlayer_NumHelper(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    int id = 0;

    if (lua_gettop(L) >= 2) {
        id = luaL_checkinteger(L, 2);
        if (id < 0) {
            Player_Warn(p, "illegal helper ID for numhelper");
            id = 0;
        }
    }
    lua_pushinteger(L, Helpers_Count(p, id));
    return 1;
}

/* A 2D Vector from p to other (x towards p's facing, through a float). */
static void LuaBind_PushDist(struct lua_State *L, Player *p, Player *other)
{
    float dx = (float)((other->pos.x - p->pos.x) * p->facing);

    Lua_PushVector(L, dx, other->pos.y - p->pos.y, 0.0, 1.0, COORD_TYPE_2D, 0);
}

/* 0x004206E0 */
int LuaPlayer_ParentDist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    Player *parent = p->parent;

    if (p->helper != 1 || parent == NULL || parent->id != p->parent_id || !parent->active) {
        return luaL_error(L, "not helper or no parent");
    }
    LuaBind_PushDist(L, p, parent);
    return 1;
}

/* 0x004207C0 */
int LuaPlayer_RootDist(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    if (p->root == NULL) {
        return luaL_error(L, "not helper or no root");
    }
    LuaBind_PushDist(L, p, p->root);
    return 1;
}

/* 0x00420880: command(name): the command is active. */
int LuaPlayer_Command(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    const char *end = NULL;
    const char *name = luaL_checklstring(L, 2, NULL);
    int label;

    if (!Cmd_FindLabel(name, &label, p->data, &end) || (end != NULL && *end != '\0')) {
        return luaL_error(L, "Undefined command label: %s", name);
    }
    lua_pushboolean(L, p->cmd_active[label] != 0);
    return 1;
}

/* 0x00420910: game units per character unit. */
int LuaPlayer_UnitSize(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushnumber(L, g_gameCoord[0] / p->data->localcoord[0]);
    return 1;
}

/*
 * 0x00420940: changestate{value = n, selfstate, autoturn, ctrl, anim}. The
 * table is the second argument (read from the top of the stack).
 */
int LuaPlayer_ChangeState(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    Player *enemy = p->enemy;
    int value = 0;
    int selfstate = 0;
    int autoturn = 1;
    int ctrl = -1;
    int anim = -1;

    if (LuaTable_GetInt(L, "value", &value) != 0) {
        return luaL_error(L, "required value field is missing");
    }
    LuaTable_GetBool(L, "selfstate", &selfstate);
    if (selfstate) {
        Player_ClearCustomState(p);
    }
    if (value < 0) {
        return luaL_error(L, "attempted change to negative state");
    }
    Player_ChangeState(value, p);
    LuaTable_GetBool(L, "autoturn", &autoturn);
    if (autoturn && !p->helper && p->ctrl && (p->statetype == STATETYPE_STAND || p->statetype == STATETYPE_CROUCH) &&
        g_game->stage.autoturn && !Screen_IsSpecial() && Player_IsActive(enemy)) {
        Player_FaceTowards(p, enemy);
    }
    LuaTable_GetBool(L, "ctrl", &ctrl);
    if (ctrl != -1) {
        p->ctrl = ctrl != 0;
    }
    LuaTable_GetInt(L, "anim", &anim);
    if (anim != -1) {
        if (anim < 0) {
            Player_Warn(p, "attempted change to negative anim");
            anim = 0;
        }
        p->anim->anim = p->data->anim;
        p->anim_from_owner = 0;
        Player_SetAction(p, anim);
    }
    return 0;
}

/* Converts a vector read from an explod table to unit 1. */
static void LuaBind_ToUnit(Coord *c)
{
    Coord v = *c;
    double unit = v.unit;

    v.x = v.x * unit;
    v.y = v.y * unit;
    v.z = unit * v.z;
    v.unit = 1.0;
    *c = v;
}

/* The localcoord width of the states p runs (the custom state owner's, or its own). */
static double LuaBind_LocalWidth(const Player *p)
{
    if (p->custom_states != NULL && p->custom_owner != -1 && g_game->players[p->custom_owner] != NULL) {
        return g_game->players[p->custom_owner]->data->localcoord[0];
    }
    return p->data->localcoord[0];
}

/*
 * 0x00420AE0: reads the explod parameters of the table on top of the stack
 * into e (owner p): id, pos, vel, accel, scale, angles, removetime,
 * supermovetime, pausemovetime, sprpriority, shadow, removeongethit,
 * hitpausemove. Returns 0.
 */
int LuaBind_ReadExplod(struct lua_State *L, Player *p, SysDrawItem *e)
{
    int v;

    LuaTable_GetInt(L, "id", &e->id);
    if (e->id < 0) {
        e->id = 0;
        Player_Warn(p, "negative explod ID");
    }
    e->owner_id = p->id;
    LuaTable_GetVector(L, "pos", &e->pos);
    LuaBind_ToUnit(&e->pos);
    LuaTable_GetVector(L, "vel", &e->vel);
    LuaBind_ToUnit(&e->vel);
    LuaTable_GetVector(L, "accel", &e->accel);
    LuaBind_ToUnit(&e->accel);
    if (LuaTable_GetVector(L, "scale", &e->scale) == 0) {
        e->scale.x = g_gameCoord[0] / LuaBind_LocalWidth(p) * e->scale_mul * e->scale.x;
        e->scale.y = g_gameCoord[0] / LuaBind_LocalWidth(p) * e->scale_mul * e->scale.y;
    }
    LuaTable_GetNumber(L, "angle", &e->angle);
    LuaTable_GetNumber(L, "yangle", &e->yangle);
    LuaTable_GetNumber(L, "xangle", &e->xangle);
    if (LuaTable_GetInt(L, "removetime", &e->time) == 0) {
        if (e->time < -2) {
            e->time = -2;
            Player_Warn(p, "Illegal removetime for explod");
        }
        if (e->time == -2) {
            AnimState *st = e->anim;
            KeyedArray *actions = st->anim->actions;
            e->time = ((AnimAction *)(actions->data + actions->elem_size * st->action))->total_time - e->started;
        }
    }
    v = 0;
    if (LuaTable_GetInt(L, "supermovetime", &v) == 0) {
        e->pause_movetime[1] = v == -2 ? e->time : v;
    }
    v = 0;
    if (LuaTable_GetInt(L, "pausemovetime", &v) == 0) {
        e->pause_movetime[0] = v == -2 ? e->time : v;
    }
    if (e->anim != NULL) {
        v = 0;
        if (LuaTable_GetInt(L, "sprpriority", &v) == 0) {
            e->anim->layer = v;
        }
        e->anim->queue = e->layer == 0 ? g_game->sprite_layer1 : NULL;
    }
    v = 0;
    if (LuaTable_GetBool(L, "shadow", &v) == 0) {
        e->shadow = v != 0 ? -1 : 0;
    }
    LuaTable_GetBool(L, "removeongethit", &e->removeongethit);
    LuaTable_GetBool(L, "hitpausemove", &e->ignorehitpause);
    return 0;
}

/*
 * 0x00420ED0: explod{anim = n, usefightfx, ownpal, remappal = {g, n}, ...}:
 * adds an explod of the player (or of fightfx.air) with LuaBind_ReadExplod's
 * parameters.
 */
int LuaPlayer_Explod(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    GameVars *g;
    int anim = -1;
    int fightfx = 0;
    int ownpal = 1;
    int group = -1;
    int number = 0;
    int item;
    SysDrawItem *e;

    if (LuaTable_GetInt(L, "anim", &anim) != 0) {
        return luaL_error(L, "required anim field is missing");
    }
    LuaTable_GetBool(L, "usefightfx", &fightfx);
    g = g_game;
    if (fightfx) {
        item = Explods_Add(g->fightfx_sff, g->explods, anim, g->fightfx_anim);
    } else {
        item = Explods_Add(p->sprites, g->explods, anim, p->data->anim);
    }
    LuaTable_GetBool(L, "ownpal", &ownpal);
    if (LuaTable_GetRemapPal(L) == 0) {
        LuaTable_GetIntAt(L, 1, &group);
        LuaTable_GetIntAt(L, 2, &number);
        lua_settop(L, -2);
    }
    if (item == -1) {
        return 0;
    }
    g = g_game;
    e = &g->explods->items[item];
    if (fightfx) {
        e->scale_mul = (float)(Player_ToLocalCoord(p, g->fightfx_scale) * g_game->motif_scale);
    }
    LuaBind_ReadExplod(L, p, e);
    if (!ownpal) {
        e->palfx = p->palfx;
        e->remap = fightfx ? NULL : p->remap;
        return 0;
    }
    e->palfx = NULL;
    if (fightfx) {
        e->remap = NULL;
        return 0;
    }
    if (group == -1) {
        e->remap_own = *p->remap;
        e->remap = &e->remap_own;
        return 0;
    }
    if (SpriteSet_FindPalette(e->draw_sprites, group, number) != NULL) {
        PalRemapEntry entry;
        memset(&e->remap_own, 0, sizeof(e->remap_own));
        entry.src_group = -1;
        entry.src_index = 0;
        entry.dst_group = group;
        entry.dst_index = number;
        PalRemap_Set(&e->remap_own, &entry);
        e->remap = &e->remap_own;
        return 0;
    }
    Player_Warnf(p, "Explod invalid remappal %i,%i", group, number);
    e->remap = &e->remap_own;
    return 0;
}

/* 0x00421150: velset{x, y} (numbers or 1D Vectors; x towards the facing). */
int LuaPlayer_VelSet(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);
    double x = 0.0;
    double y = 0.0;

    if (LuaTable_GetVectorNumber("x", L, &x) == 0) {
        p->vel.x = p->facing * x;
    }
    if (LuaTable_GetVectorNumber("y", L, &y) == 0) {
        p->vel.y = y;
    }
    return 0;
}

/* 0x004211C0: isvalid(): 1 or 0 (integers), no error. */
int LuaPlayer_IsValid(struct lua_State *L)
{
    LuaPlayerRef *ref = luaL_checkudata(L, 1, "mugen.player");

    if (ref->valid == 1) {
        Player *p = g_game->players[ref->index];
        if (p != NULL && p->active && p->id == ref->id) {
            lua_pushinteger(L, 1);
            return 1;
        }
    }
    lua_pushinteger(L, 0);
    return 1;
}

/* 0x00421230: "Player <name>:<id>" */
int LuaPlayer_ToString(struct lua_State *L)
{
    Player *p = LuaPlayer_Check(L);

    lua_pushfstring(L, "Player %s:%d", p->data->name, p->id);
    return 1;
}

/* 0x00421260: rc(): the character's Lua table. */
int LuaPlayer_Rc(struct lua_State *L)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, LuaPlayer_Check(L)->data->lua_ref);
    return 1;
}

/* The "player" library (0x004F21CC). */
static const LuaReg luaplayer_lib[] = {
    { "getplayer", LuaPlayer_GetPlayer },
    { "getplayerlist", LuaPlayer_GetPlayerList },
    { "indexisvalid", LuaPlayer_IndexIsValid },
    { "indexfromid", LuaPlayer_IndexFromId },
    { "enabled", LuaPlayer_Enabled },
    { "enableset", LuaPlayer_EnableSet },
    { NULL, NULL }
};

/* The methods of "mugen.player" (0x004F2208). */
static const LuaReg luaplayer_methods[] = {
    { "life", LuaPlayer_Life },
    { "lifeset", LuaPlayer_LifeSet },
    { "lifemax", LuaPlayer_LifeMax },
    { "power", LuaPlayer_Power },
    { "powerset", LuaPlayer_PowerSet },
    { "powermax", LuaPlayer_PowerMax },
    { "winperfect", LuaPlayer_WinPerfect },
    { "win", LuaPlayer_Win },
    { "winko", LuaPlayer_WinKO },
    { "wintime", LuaPlayer_WinTime },
    { "lose", LuaPlayer_Lose },
    { "loseko", LuaPlayer_LoseKO },
    { "losetime", LuaPlayer_LoseTime },
    { "drawgame", LuaPlayer_DrawGame },
    { "matchover", LuaPlayer_MatchOver },
    { "animtime", LuaPlayer_AnimTime },
    { "anim", LuaPlayer_Anim },
    { "animelemno", LuaPlayer_AnimElemNo },
    { "animelemtime", LuaPlayer_AnimElemTime },
    { "animexist", LuaPlayer_AnimExist },
    { "selfanimexist", LuaPlayer_SelfAnimExist },
    { "roundsexisted", LuaPlayer_RoundsExisted },
    { "teamside", LuaPlayer_TeamSide },
    { "teammode", LuaPlayer_TeamMode },
    { "ishometeam", LuaPlayer_IsHomeTeam },
    { "alive", LuaPlayer_Alive },
    { "aienabled", LuaPlayer_AiEnabled },
    { "aienableset", LuaPlayer_AiEnableSet },
    { "ailevel", LuaPlayer_AiLevel },
    { "hitcount", LuaPlayer_HitCount },
    { "uniqhitcount", LuaPlayer_UniqHitCount },
    { "movehit", LuaPlayer_MoveHit },
    { "moveguarded", LuaPlayer_MoveGuarded },
    { "movecontact", LuaPlayer_MoveContact },
    { "movereversed", LuaPlayer_MoveReversed },
    { "inguarddist", LuaPlayer_InGuardDist },
    { "stateno", LuaPlayer_StateNo },
    { "prevstateno", LuaPlayer_PrevStateNo },
    { "statetype", LuaPlayer_StateType },
    { "movetype", LuaPlayer_MoveType },
    { "ctrl", LuaPlayer_Ctrl },
    { "var", LuaPlayer_Var },
    { "fvar", LuaPlayer_FVar },
    { "sysvar", LuaPlayer_SysVar },
    { "sysfvar", LuaPlayer_SysFVar },
    { "hitdefattrmatch", LuaPlayer_HitDefAttrMatch },
    { "numproj", LuaPlayer_NumProj },
    { "projcanceltime", LuaPlayer_ProjCancelTime },
    { "projhittime", LuaPlayer_ProjHitTime },
    { "projguardedtime", LuaPlayer_ProjGuardedTime },
    { "projcontacttime", LuaPlayer_ProjContactTime },
    { "name", LuaPlayer_Name },
    { "authorname", LuaPlayer_AuthorName },
    { "numpartner", LuaPlayer_NumPartner },
    { "numenemy", LuaPlayer_NumEnemy },
    { "id", LuaPlayer_Id },
    { "facing", LuaPlayer_Facing },
    { "hitfall", LuaPlayer_HitFall },
    { "hitshakeover", LuaPlayer_HitShakeOver },
    { "hitover", LuaPlayer_HitOver },
    { "hitpausetime", LuaPlayer_HitPauseTime },
    { "canrecover", LuaPlayer_CanRecover },
    { "palno", LuaPlayer_PalNo },
    { "numexplod", LuaPlayer_NumExplod },
    { "numtarget", LuaPlayer_NumTarget },
    { "ishelper", LuaPlayer_IsHelper },
    { "numhelper", LuaPlayer_NumHelper },
    { "command", LuaPlayer_Command },
    { "unitsize", LuaPlayer_UnitSize },
    { "vel", LuaPlayer_Vel },
    { "pos", LuaPlayer_Pos },
    { "screenpos", LuaPlayer_ScreenPos },
    { "backedge", LuaPlayer_BackEdge },
    { "frontedge", LuaPlayer_FrontEdge },
    { "frontedgedist", LuaPlayer_FrontEdgeDist },
    { "backedgedist", LuaPlayer_BackEdgeDist },
    { "frontedgebodydist", LuaPlayer_FrontEdgeBodyDist },
    { "backedgebodydist", LuaPlayer_BackEdgeBodyDist },
    { "hitvel", LuaPlayer_HitVel },
    { "parentdist", LuaPlayer_ParentDist },
    { "rootdist", LuaPlayer_RootDist },
    { "gethitvar", LuaPlayer_GetHitVar },
    { "changestate", LuaPlayer_ChangeState },
    { "explod", LuaPlayer_Explod },
    { "velset", LuaPlayer_VelSet },
    { "isvalid", LuaPlayer_IsValid },
    { "__tostring", LuaPlayer_ToString },
    { "rc", LuaPlayer_Rc },
    { NULL, NULL }
};

/*
 * 0x00421290: creates the "mugen.player" metatable (its own __index, the
 * methods) and the "player" library (with _meta). Quirk: both stay on the
 * stack. Returns 1.
 */
int LuaBind_Register(void)
{
    struct lua_State *L = g_luaState;

    luaL_newmetatable(L, "mugen.player");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, luaplayer_methods);
    luaL_register(L, "player", luaplayer_lib);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "_meta");
    return 1;
}

/* ---- the character constants in its Lua table ---- */

/* Pushes the "const" table of the character's Lua table; 0 (error raised) when it is not a table. */
static int CharLua_PushConst(CharData *c, struct lua_State *L)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, c->lua_ref);
    lua_pushstring(L, "const");
    lua_gettable(L, -2);
    return lua_type(L, -1) == LUA_TTABLE;
}

/* 0x00421300: const[name] = value (integer). */
int CharLua_SetConstInt(CharData *c, struct lua_State *L, const char *name, int value)
{
    if (!CharLua_PushConst(c, L)) {
        return luaL_error(L, "Table expected");
    }
    lua_pushstring(L, name);
    lua_pushinteger(L, value);
    lua_settable(L, -3);
    lua_settop(L, -3);
    return 0;
}

/* 0x00421380: const[name] = value (number). */
int CharLua_SetConstFloat(CharData *c, struct lua_State *L, const char *name, double value)
{
    if (!CharLua_PushConst(c, L)) {
        return luaL_error(L, "Table expected");
    }
    lua_pushstring(L, name);
    lua_pushnumber(L, value);
    lua_settable(L, -3);
    lua_settop(L, -3);
    return 0;
}

/* 0x00421400: const[name] = Vector. */
int CharLua_SetConstCoord(CharData *c, struct lua_State *L, const char *name, double x, double y, double z,
                          double unit, int type, int pad)
{
    if (!CharLua_PushConst(c, L)) {
        return luaL_error(L, "Table expected");
    }
    lua_pushstring(L, name);
    Lua_PushVector(L, x, y, z, unit, type, pad);
    lua_settable(L, -3);
    lua_settop(L, -3);
    return 0;
}

/* 0x00421490: *out = const[name] (a Vector); the result of Lua_ReadVector is not checked. */
int CharLua_GetConstCoord(CharData *c, struct lua_State *L, Coord *out, const char *name)
{
    if (!CharLua_PushConst(c, L)) {
        return luaL_error(L, "Table expected");
    }
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    Lua_ReadVector(L, out);
    lua_settop(L, -4);
    return 0;
}
