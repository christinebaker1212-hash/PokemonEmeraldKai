/*
 * The Lua engine and the "mugen" library (original module 0x42E2F0-0x42EF4F,
 * strings "mugen" to "Vector must be a 1D vector"): Lua_Init creates
 * g_luaState with the standard libraries, the "mugen" functions below and
 * the player bindings, sets package.path and runs data/init.lua. The vector
 * helpers convert between Coord and the Lua "Vector" class of init.lua.
 *
 * The functions registered by this module are the decompiled ones (the
 * original registers its own copies at the same names).
 */
#include "mugen.h"

#define LUA_GLOBALSINDEX    (-10002)
#define LUA_TNIL            0

/* ---- the "mugen" library ---- */

/* 0x0042E400: mugen.lifereset() */
int LuaMugen_LifeReset(struct lua_State *L)
{
    Debug_ResetLifePowerTime();
    return 0;
}

/* 0x0042E410: mugen.clipboardprint(s) */
int LuaMugen_ClipboardPrint(struct lua_State *L)
{
    Clipboard_Printf("%s", luaL_checklstring(L, 1, NULL));
    return 0;
}

/* 0x0042E430: mugen.consoleprint(s) */
int LuaMugen_ConsolePrint(struct lua_State *L)
{
    Console_Printf("%s", luaL_checklstring(L, 1, NULL));
    return 0;
}

/* 0x0042E450: mugen.log(s) */
int LuaMugen_Log(struct lua_State *L)
{
    Log_Printf("%s", luaL_checklstring(L, 1, NULL));
    return 0;
}

/* 0x0042E470: mugen.winningteam() */
int LuaMugen_WinningTeam(struct lua_State *L)
{
    lua_pushinteger(L, Match_CheckOver());
    return 1;
}

/* 0x0042E490: mugen.gametime() */
int LuaMugen_GameTime(struct lua_State *L)
{
    lua_pushinteger(L, (int)g_game->frames);
    return 1;
}

/* 0x0042E4B0: mugen.roundno() */
int LuaMugen_RoundNo(struct lua_State *L)
{
    lua_pushinteger(L, g_game->round_no);
    return 1;
}

/* 0x0042E4D0: mugen.roundstate() */
int LuaMugen_RoundState(struct lua_State *L)
{
    lua_pushinteger(L, g_game->round_state);
    return 1;
}

/* 0x0042E4F0: mugen.matchno() */
int LuaMugen_MatchNo(struct lua_State *L)
{
    lua_pushinteger(L, g_game->arcade_match);
    return 1;
}

/* 0x0042E510: mugen.tickspersecond() */
int LuaMugen_TicksPerSecond(struct lua_State *L)
{
    lua_pushinteger(L, g_frameRate);
    return 1;
}

/* 0x0042E530: mugen.gamewidth(): the game width in stage units at the current zoom. */
int LuaMugen_GameWidth(struct lua_State *L)
{
    GameVars *g = g_game;

    lua_pushnumber(L, (double)Ftol(g->game_coord[0]) / g->camera_zoom);
    return 1;
}

/* 0x0042E570: mugen.gameheight() */
int LuaMugen_GameHeight(struct lua_State *L)
{
    GameVars *g = g_game;

    lua_pushnumber(L, (double)Ftol(g->game_coord[1]) / g->camera_zoom);
    return 1;
}

/* 0x0042E5B0: mugen.screenwidth() */
int LuaMugen_ScreenWidth(struct lua_State *L)
{
    lua_pushnumber(L, (double)Ftol(g_game->game_coord[0]));
    return 1;
}

/* 0x0042E5E0: mugen.screenheight() */
int LuaMugen_ScreenHeight(struct lua_State *L)
{
    lua_pushnumber(L, (double)Ftol(g_game->game_coord[1]));
    return 1;
}

/* 0x0042E610: mugen.camerapos(): a Vector. */
int LuaMugen_CameraPos(struct lua_State *L)
{
    Coord c;
    const Coord *p = Camera_GetPos(&c);

    Lua_PushVector(L, p->x, p->y, p->z, p->unit, p->type, p->pad_24);
    return 1;
}

/* 0x0042E650: mugen.camerazoom() */
int LuaMugen_CameraZoom(struct lua_State *L)
{
    lua_pushnumber(L, (double)g_game->camera_zoom);
    return 1;
}

/* 0x0042E680: mugen.leftedge(): the left edge of the screen in stage space. */
int LuaMugen_LeftEdge(struct lua_State *L)
{
    Coord c;
    const Coord *p = Camera_GetPos(&c);
    GameVars *g = g_game;

    lua_pushnumber(L, p->x - (double)Ftol(g->game_coord[0]) / g->camera_zoom * 0.5);
    return 1;
}

/* 0x0042E6E0: mugen.rightedge() */
int LuaMugen_RightEdge(struct lua_State *L)
{
    Coord c;
    const Coord *p = Camera_GetPos(&c);
    GameVars *g = g_game;

    lua_pushnumber(L, (double)Ftol(g->game_coord[0]) / g->camera_zoom * 0.5 + p->x);
    return 1;
}

/*
 * 0x0042E740: mugen.topedge(): the stage y of the top of the screen. The
 * pad field of the converted coordinate is stack garbage in the original
 * (not used).
 */
int LuaMugen_TopEdge(struct lua_State *L)
{
    Coord out;
    const Coord *p = Coord_ScreenToStage(&out, 0.0, 0.0, 0.0, 1.0, COORD_TYPE_2D, 0);

    lua_pushnumber(L, p->y);
    return 1;
}

/* 0x0042E7B0: mugen.bottomedge() */
int LuaMugen_BottomEdge(struct lua_State *L)
{
    lua_pushnumber(L, Screen_GetBottomEdge());
    return 1;
}

/*
 * 0x0042E7E0: mugen.random(n): 0..n-1 from the Lua random generator
 * (GameVars.match_rng = match_rng * match_rng_mul % match_rng_mod, 64 bits; the low
 * dword divided signed).
 */
int LuaMugen_Random(struct lua_State *L)
{
    int n = luaL_checkinteger(L, 1);
    GameVars *g;
    unsigned __int64 s;

    if (n <= 0) {
        luaL_error(L, "negative or zero random modulus");
    }
    g = g_game;
    s = g->match_rng * (unsigned __int64)g->match_rng_mul;
    s %= (unsigned __int64)g->match_rng_mod;
    g->match_rng = s;
    lua_pushinteger(L, (int)(unsigned int)s % n);
    return 1;
}

/* 0x0042E860: mugen.roundtimeleft() */
int LuaMugen_RoundTimeLeft(struct lua_State *L)
{
    lua_pushinteger(L, g_game->round_time);
    return 1;
}

/*
 * 0x0042E880: mugen.roundtimemax(): ([Options] Time + 1) * ticks per count
 * - 1, -1 (infinite) in survival and training modes and in demos.
 */
int LuaMugen_RoundTimeMax(struct lua_State *L)
{
    GameVars *g = g_game;
    int t = (g->time + 1) * g->fight.time.frames_per_count - 1;

    if (g->team_variant == 3 || g->team_variant == 1 || g->demo_mode != 0) {
        t = -1;
    }
    lua_pushinteger(L, t);
    return 1;
}

/* 0x0042E8D0: mugen.setroundtimeleft(n), n >= -1 */
int LuaMugen_SetRoundTimeLeft(struct lua_State *L)
{
    int n = luaL_checkinteger(L, 1);

    if (n < -1) {
        luaL_error(L, "invalid round time left");
    }
    g_game->round_time = n;
    return 0;
}

/* 0x0042E920: mugen.keybindadd(key, command) */
int LuaMugen_KeybindAdd(struct lua_State *L)
{
    const char *key = luaL_checklstring(L, 1, NULL);
    const char *cmd = luaL_checklstring(L, 2, NULL);
    KeyBind kb;

    if (!Keybind_Parse(key, &kb)) {
        luaL_error(L, "Bad keybind string: %s\n", key);
    }
    Keybind_Add(g_console->bindings, cmd, KEYBIND_ARGS(kb));
    return 0;
}

/* 0x0042E9A0: mugen.keybinddel(key) */
int LuaMugen_KeybindDel(struct lua_State *L)
{
    const char *key = luaL_checklstring(L, 1, NULL);
    KeyBind kb;

    if (!Keybind_Parse(key, &kb)) {
        luaL_error(L, "Bad keybind string: %s\n", key);
    }
    Keybind_Remove(g_console->bindings, KEYBIND_ARGS(kb));
    return 0;
}

/* 0x0042EA10: mugen.keybindget(key): the bound command, or nil. */
int LuaMugen_KeybindGet(struct lua_State *L)
{
    const char *key = luaL_checklstring(L, 1, NULL);
    const char *cmd;
    KeyBind kb;

    if (!Keybind_Parse(key, &kb)) {
        luaL_error(L, "Bad keybind string: %s\n", key);
    }
    cmd = Keybind_Find(g_console->bindings, KEYBIND_ARGS(kb));
    if (cmd != NULL) {
        lua_pushstring(L, cmd);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* 0x0042EAB0: mugen.gamemodecode() */
int LuaMugen_GameModeCode(struct lua_State *L)
{
    lua_pushinteger(L, g_game->screen_kind);
    return 1;
}

/* 0x0042EAD0: mugen.language() */
int LuaMugen_Language(struct lua_State *L)
{
    lua_pushstring(L, g_language);
    return 1;
}

/* 0x0042EAF0: mugen.toggleconsole() */
int LuaMugen_ToggleConsole(struct lua_State *L)
{
    Console_Toggle();
    return 0;
}

/* 0x0042EB00: mugen.togglemaxpowermode() */
int LuaMugen_ToggleMaxPowerMode(struct lua_State *L)
{
    GameVars *g = g_game;

    g->debug_full_power = g->debug_full_power == 0;
    Log_Printf("Max power mode set to %i\n", g->debug_full_power);
    return 0;
}

/* 0x0042EB30: mugen.roundreset(): during a fight. */
int LuaMugen_RoundReset(struct lua_State *L)
{
    Log_Printf("Requesting round reset\n");
    if (g_game->screen_kind == 4) {
        g_game->round_reset_pending = 1;
    }
    return 0;
}

/* 0x0042EB60: mugen.matchreload(): during a fight. */
int LuaMugen_MatchReload(struct lua_State *L)
{
    Log_Printf("Requesting match reload\n");
    if (g_game->screen_kind == 4) {
        g_game->match_reload = 1;
    }
    return 0;
}

/* 0x0042EB90: mugen.clearclipboards() */
int LuaMugen_ClearClipboards(struct lua_State *L)
{
    Debug_ClearClipboards();
    return 0;
}

/* 0x0042EBA0: mugen.togglebardisplay() */
int LuaMugen_ToggleBarDisplay(struct lua_State *L)
{
    GameVars *g = g_game;

    g->hud_enabled = g->hud_enabled == 0;
    return 0;
}

/* 0x0042EBC0: mugen.toggleclsndisplay(): off, boxes, filled boxes. */
int LuaMugen_ToggleClsnDisplay(struct lua_State *L)
{
    GameVars *g = g_game;

    g->clsn_draw_mode++;
    if (g->clsn_draw_mode > 2) {
        g->clsn_draw_mode = 0;
    }
    return 0;
}

/* 0x0042EBF0: mugen.toggletrainingmode() */
int LuaMugen_ToggleTrainingMode(struct lua_State *L)
{
    Debug_ToggleTrainingMode();
    return 0;
}

/* The "mugen" library (0x004F1F08 in the original, with its own functions). */
static const LuaReg luamugen_funcs[] = {
    { "lifereset", LuaMugen_LifeReset },
    { "winningteam", LuaMugen_WinningTeam },
    { "gametime", LuaMugen_GameTime },
    { "roundno", LuaMugen_RoundNo },
    { "roundstate", LuaMugen_RoundState },
    { "matchno", LuaMugen_MatchNo },
    { "tickspersecond", LuaMugen_TicksPerSecond },
    { "gamewidth", LuaMugen_GameWidth },
    { "gameheight", LuaMugen_GameHeight },
    { "screenwidth", LuaMugen_ScreenWidth },
    { "screenheight", LuaMugen_ScreenHeight },
    { "camerapos", LuaMugen_CameraPos },
    { "camerazoom", LuaMugen_CameraZoom },
    { "leftedge", LuaMugen_LeftEdge },
    { "rightedge", LuaMugen_RightEdge },
    { "topedge", LuaMugen_TopEdge },
    { "bottomedge", LuaMugen_BottomEdge },
    { "random", LuaMugen_Random },
    { "roundtimeleft", LuaMugen_RoundTimeLeft },
    { "roundtimemax", LuaMugen_RoundTimeMax },
    { "setroundtimeleft", LuaMugen_SetRoundTimeLeft },
    { "keybindadd", LuaMugen_KeybindAdd },
    { "keybinddel", LuaMugen_KeybindDel },
    { "keybindget", LuaMugen_KeybindGet },
    { "gamemodecode", LuaMugen_GameModeCode },
    { "language", LuaMugen_Language },
    { "toggleconsole", LuaMugen_ToggleConsole },
    { "togglemaxpowermode", LuaMugen_ToggleMaxPowerMode },
    { "roundreset", LuaMugen_RoundReset },
    { "matchreload", LuaMugen_MatchReload },
    { "clearclipboards", LuaMugen_ClearClipboards },
    { "togglebardisplay", LuaMugen_ToggleBarDisplay },
    { "toggleclsndisplay", LuaMugen_ToggleClsnDisplay },
    { "toggletrainingmode", LuaMugen_ToggleTrainingMode },
    { "clipboardprint", LuaMugen_ClipboardPrint },
    { "consoleprint", LuaMugen_ConsolePrint },
    { "log", LuaMugen_Log },
    { NULL, NULL }
};

/* ---- the engine ---- */

/* 0x0042EC00: g_luaHook1: logs the error message on top of the stack. Returns 0. */
int Lua_LogError(struct lua_State *L)
{
    Log_Printf("%s\n", luaL_checklstring(L, -1, NULL));
    return 0;
}

/* 0x0042EC20: the panic function of g_luaState: a fatal error with the message. */
int Lua_Panic(struct lua_State *L)
{
    Error_Fatal("Lua fatal error: %s\n", luaL_checklstring(L, -1, NULL));
    return 0;
}

/*
 * 0x0042E390: runs data/init.lua; the error message goes to the log.
 * Returns 0, or the luaL_loadfile / lua_pcall error code.
 */
int Lua_RunInit(void)
{
    int r = luaL_loadfile(g_luaState, "data/init.lua");

    if (r == 0) {
        r = lua_pcall(g_luaState, 0, 0, 0);
        if (r == 0) {
            return 0;
        }
    }
    Log_Printf("%s\n", luaL_checklstring(g_luaState, -1, NULL));
    return r;
}

/*
 * 0x0042E2F0: creates g_luaState once (standard libraries, "mugen", the
 * player bindings, package.path, data/init.lua). Returns 0 when the state
 * cannot be created, else 1 (init.lua errors are only logged).
 */
int Lua_Init(void)
{
    if (g_luaState != NULL) {
        return 1;
    }
    g_luaHook1 = (void *)Lua_LogError;
    g_luaHook2 = (void *)Log_Printf;
    g_luaState = luaL_newstate();
    if (g_luaState == NULL) {
        return 0;
    }
    lua_atpanic(g_luaState, Lua_Panic);
    luaL_openlibs(g_luaState);
    luaL_register(g_luaState, "mugen", luamugen_funcs);
    LuaBind_Register();
    Lua_SetPackagePath();
    Lua_RunInit();
    return 1;
}

/* ---- vectors ---- */

/*
 * 0x0042EC40: pushes Vector.new(x, y, z, unit), Vector.vec2(x, y, unit) or
 * Vector.vec1(x, unit) for a 3, 2 or 1 dimensional coordinate (type).
 * Returns 1, or 0 (nothing pushed) for another type.
 */
int Lua_PushVector(struct lua_State *L, double x, double y, double z, double unit, int type, int pad)
{
    switch (type) {
    case 1:
        Lua_CallFunction(L, "Vector", "vec1", "tff>-", x, unit);
        return 1;
    case 2:
        Lua_CallFunction(L, "Vector", "vec2", "tfff>-", x, y, unit);
        return 1;
    case 3:
        Lua_CallFunction(L, "Vector", "new", "tffff>-", x, y, z, unit);
        return 1;
    default:
        return 0;
    }
}

/*
 * 0x0042ED20: reads the Vector on top of the stack (x, y, z, unitsize,
 * dimensions) into *out (type = dimensions). Returns 0, or -1 when a field
 * is missing (out unchanged).
 *
 * Quirk: out->pad_24 gets stack garbage in the original; 0 here.
 */
int Lua_ReadVector(struct lua_State *L, Coord *out)
{
    double x, y, z, unit;
    int dims;
    Coord c;

    if (LuaTable_GetNumber(L, "x", &x) != 0 || LuaTable_GetNumber(L, "y", &y) != 0 ||
        LuaTable_GetNumber(L, "z", &z) != 0 || LuaTable_GetNumber(L, "unitsize", &unit) != 0 ||
        LuaTable_GetInt(L, "dimensions", &dims) != 0) {
        return -1;
    }
    c.x = x;
    c.y = y;
    c.z = z;
    c.unit = unit;
    c.type = 3;
    c.pad_24 = 0;
    *out = c;
    out->type = dims;
    return 0;
}

/*
 * 0x0042EDE0: table[key] (table on top of the stack) as a Vector in *out.
 * Returns 0, -1 when the field is nil; luaL_error when it is not a Vector.
 */
int LuaTable_GetVector(struct lua_State *L, const char *key, Coord *out)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (Lua_ReadVector(L, out) != 0) {
        return luaL_error(L, "Vector expected");
    }
    lua_settop(L, -2);
    return 0;
}

/*
 * 0x0042EE40: table[key] as a number: a number, or a 1D Vector rescaled to
 * unit 1 (a zero unit is an error). Returns 0, -1 when the field is nil;
 * luaL_error otherwise.
 */
int LuaTable_GetVectorNumber(const char *key, struct lua_State *L, double *out)
{
    Coord c;

    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (lua_isnumber(L, -1)) {
        *out = lua_tonumber(L, -1);
        lua_settop(L, -2);
        return 0;
    }
    if (Lua_ReadVector(L, &c) != 0) {
        return luaL_error(L, "Vector or number expected");
    }
    if (c.type == 1 && !(0.0 == c.unit)) {
        Coord r;
        *out = Coord_Rescale(&r, c.x, c.y, c.z, c.unit, c.type, c.pad_24, 1.0)->x;
        lua_settop(L, -2);
        return 0;
    }
    return luaL_error(L, "Vector must be a 1D vector");
}
