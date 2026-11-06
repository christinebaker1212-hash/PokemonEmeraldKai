// Moemon Maker, M5: Lua hooks (docs/MOEMON_MAKER.md principle 5). The scripts in MoemonMaker/lua/*.lua (the published
// pack's lua/ for players) are loaded into one Lua state, EDOPro's Lua 5.4 (built as C++, so this file is C++ with the
// same flags: Makefile MAKER_LUA_*). Sandboxed and deterministic: only the base, string, table and math libraries,
// without print / dofile / loadfile / load / require / math.random (game.random is the game's own), and a step
// budget so a loop can't hang the game. What a script may touch is the `game` table (src/maker/maker_luagame.c):
//
//   game.flag(NAME)  game.setflag(NAME, on)  game.var(NAME)  game.setvar(NAME, n)  game.item(NAME)
//   game.giveitem(NAME, n)  game.takeitem(NAME, n)  game.money()  game.addmoney(n)  game.partycount()
//   game.species(slot)  game.level(slot)  game.map()  game.x()  game.y()  game.random(n)  game.log(text)
//
// Hooks the game calls when a script defines them: OnStep(x, y, map) after each step of the player and
// OnMapEnter(map) when a map loads; OnStep may return the name of an event of the map to start it. An event line
// `lua FUNCTION args...` calls a function (arguments are numbers or names) and puts what it returns in VAR_RESULT
// (true 1, false / nil 0), for `if result N`.
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

extern "C" {
const char *Maker_ReadDir(void);
int MakerLua_Flag(const char *name);
int MakerLua_SetFlag(const char *name, int on);
int MakerLua_Var(const char *name);
int MakerLua_SetVar(const char *name, int value);
int MakerLua_Item(const char *name);
int MakerLua_GiveItem(const char *name, int n);
int MakerLua_TakeItem(const char *name, int n);
int MakerLua_Money(void);
void MakerLua_AddMoney(int n);
int MakerLua_PartyCount(void);
const char *MakerLua_PartySpecies(int slot);
int MakerLua_PartyLevel(int slot);
const char *MakerLua_MapName(void);
int MakerLua_PlayerX(void);
int MakerLua_PlayerY(void);
int MakerLua_Random(int n);
void MakerLua_Log(const char *s);
unsigned char MakerLua_StartEvent(const char *name);
}

static lua_State *sL;
static bool sLoaded;
static std::string sError;   // the last load's or call's problem ("" when fine)

static const int kStepBudget = 5000000;   // instructions a call may run before it's stopped

static void Budget(lua_State *L, lua_Debug *)
{
    luaL_error(L, "the script ran too long (a loop that never ends?)");
}

static int G_flag(lua_State *L) { int v = MakerLua_Flag(luaL_checkstring(L, 1)); if (v < 0) lua_pushnil(L); else lua_pushboolean(L, v); return 1; }
static int G_setflag(lua_State *L) { lua_pushboolean(L, MakerLua_SetFlag(luaL_checkstring(L, 1), lua_isnoneornil(L, 2) ? 1 : lua_toboolean(L, 2))); return 1; }
static int G_var(lua_State *L) { int v = MakerLua_Var(luaL_checkstring(L, 1)); if (v < 0) lua_pushnil(L); else lua_pushinteger(L, v); return 1; }
static int G_setvar(lua_State *L) { lua_pushboolean(L, MakerLua_SetVar(luaL_checkstring(L, 1), (int)luaL_checkinteger(L, 2))); return 1; }
static int G_item(lua_State *L) { int v = MakerLua_Item(luaL_checkstring(L, 1)); if (v < 0) lua_pushnil(L); else lua_pushinteger(L, v); return 1; }
static int G_giveitem(lua_State *L) { lua_pushboolean(L, MakerLua_GiveItem(luaL_checkstring(L, 1), (int)luaL_optinteger(L, 2, 1))); return 1; }
static int G_takeitem(lua_State *L) { lua_pushboolean(L, MakerLua_TakeItem(luaL_checkstring(L, 1), (int)luaL_optinteger(L, 2, 1))); return 1; }
static int G_money(lua_State *L) { lua_pushinteger(L, MakerLua_Money()); return 1; }
static int G_addmoney(lua_State *L) { MakerLua_AddMoney((int)luaL_checkinteger(L, 1)); return 0; }
static int G_partycount(lua_State *L) { lua_pushinteger(L, MakerLua_PartyCount()); return 1; }
static int G_species(lua_State *L) { lua_pushstring(L, MakerLua_PartySpecies((int)luaL_checkinteger(L, 1) - 1)); return 1; }
static int G_level(lua_State *L) { lua_pushinteger(L, MakerLua_PartyLevel((int)luaL_checkinteger(L, 1) - 1)); return 1; }
static int G_map(lua_State *L) { lua_pushstring(L, MakerLua_MapName()); return 1; }
static int G_x(lua_State *L) { lua_pushinteger(L, MakerLua_PlayerX()); return 1; }
static int G_y(lua_State *L) { lua_pushinteger(L, MakerLua_PlayerY()); return 1; }
static int G_random(lua_State *L) { lua_pushinteger(L, MakerLua_Random((int)luaL_checkinteger(L, 1))); return 1; }
static int G_log(lua_State *L) { MakerLua_Log(luaL_tolstring(L, 1, NULL)); return 0; }

static const luaL_Reg kGame[] = {
    {"flag", G_flag}, {"setflag", G_setflag}, {"var", G_var}, {"setvar", G_setvar}, {"item", G_item},
    {"giveitem", G_giveitem}, {"takeitem", G_takeitem}, {"money", G_money}, {"addmoney", G_addmoney},
    {"partycount", G_partycount}, {"species", G_species}, {"level", G_level}, {"map", G_map}, {"x", G_x}, {"y", G_y},
    {"random", G_random}, {"log", G_log}, {NULL, NULL}};

static lua_State *NewState(void)
{
    lua_State *L = luaL_newstate();
    luaL_requiref(L, LUA_GNAME, luaopen_base, 1);
    luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, 1);
    luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, 1);
    luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_settop(L, 0);
    static const char *const kGone[] = {"print", "dofile", "loadfile", "load", "require", "collectgarbage"};
    for (const char *g : kGone)
    {
        lua_pushnil(L);
        lua_setglobal(L, g);
    }
    lua_getglobal(L, "math");
    lua_pushnil(L);
    lua_setfield(L, -2, "random");
    lua_pushnil(L);
    lua_setfield(L, -2, "randomseed");
    lua_pop(L, 1);
    luaL_newlib(L, kGame);
    lua_setglobal(L, "game");
    return L;
}

// Runs the function on the stack's top with NARGS under it, the budget on; false (and sError) when it failed
static bool Run(lua_State *L, int nargs, int nres)
{
    lua_sethook(L, Budget, LUA_MASKCOUNT, kStepBudget);
    int r = lua_pcall(L, nargs, nres, 0);
    lua_sethook(L, NULL, 0, 0);
    if (r != LUA_OK)
    {
        sError = lua_tostring(L, -1) ? lua_tostring(L, -1) : "a Lua error";
        MakerLua_Log(sError.c_str());
        lua_pop(L, 1);
        return false;
    }
    return true;
}

static void Load(void)
{
    if (sLoaded)
        return;
    sLoaded = true;
    sError.clear();
    if (sL)
        lua_close(sL);
    sL = NewState();
    std::string dir = std::string(Maker_ReadDir()) + "/lua";
    std::vector<std::string> files;
    if (DIR *d = opendir(dir.c_str()))
    {
        while (struct dirent *e = readdir(d))
        {
            size_t n = strlen(e->d_name);
            if (n > 4 && strcmp(e->d_name + n - 4, ".lua") == 0)
                files.push_back(e->d_name);
        }
        closedir(d);
    }
    std::sort(files.begin(), files.end());   // (the same order on every machine)
    for (const std::string &f : files)
    {
        std::string path = dir + "/" + f;
        if (luaL_loadfilex(sL, path.c_str(), "t") != LUA_OK)
        {
            sError = lua_tostring(sL, -1);
            MakerLua_Log(sError.c_str());
            lua_pop(sL, 1);
            continue;
        }
        Run(sL, 0, 0);
    }
    if (!files.empty())
        printf("[maker] lua: %d script%s\n", (int)files.size(), files.size() == 1 ? "" : "s");
}

// Pushes global NAME if it's a function
static bool Function(const char *name)
{
    Load();
    lua_getglobal(sL, name);
    if (lua_isfunction(sL, -1))
        return true;
    lua_pop(sL, 1);
    return false;
}

extern "C" {

void MakerApi_LuaReload(void) { sLoaded = false; }

// The editor's check of a script's text: "" when it compiles, else the problem
const char *MakerApi_LuaCheck(const char *name, const char *text)
{
    static std::string out;
    lua_State *L = luaL_newstate();
    out.clear();
    if (luaL_loadbufferx(L, text, strlen(text), name, "t") != LUA_OK)
        out = lua_tostring(L, -1);
    lua_close(L);
    return out.c_str();
}

// The last problem a script had when the game loaded or ran it ("" when none)
const char *MakerApi_LuaError(void) { Load(); return sError.c_str(); }

// An event's `lua FUNCTION args...`: what it returns, as a number (true 1, false / nil 0); -1 when it failed
int MakerLua_Call(const char *line)
{
    char buf[512], *w[16];
    int n = 0;
    snprintf(buf, sizeof(buf), "%s", line);
    for (char *p = strtok(buf, " \t"); p && n < 16; p = strtok(NULL, " \t"))
        w[n++] = p;
    if (n == 0 || !Function(w[0]))
    {
        sError = std::string("no Lua function ") + (n ? w[0] : "");
        MakerLua_Log(sError.c_str());
        return -1;
    }
    for (int i = 1; i < n; i++)
    {
        char *end;
        long v = strtol(w[i], &end, 0);
        if (*end == 0)
            lua_pushinteger(sL, v);
        else
            lua_pushstring(sL, w[i]);
    }
    if (!Run(sL, n - 1, 1))
        return -1;
    int r = lua_isinteger(sL, -1) ? (int)lua_tointeger(sL, -1) : lua_isnumber(sL, -1) ? (int)lua_tonumber(sL, -1)
          : lua_toboolean(sL, -1);
    lua_pop(sL, 1);
    return r;
}

// field_control_avatar.c, after each step: OnStep(x, y, map); an event's name back starts it (TRUE)
unsigned char MakerLua_OnStep(void)
{
    if (!Function("OnStep"))
        return 0;
    lua_pushinteger(sL, MakerLua_PlayerX());
    lua_pushinteger(sL, MakerLua_PlayerY());
    lua_pushstring(sL, MakerLua_MapName());
    if (!Run(sL, 3, 1))
        return 0;
    unsigned char started = 0;
    if (lua_type(sL, -1) == LUA_TSTRING)
        started = MakerLua_StartEvent(lua_tostring(sL, -1));
    lua_pop(sL, 1);
    return started;
}

// overworld.c, when a map loads (beside its ON_TRANSITION script): OnMapEnter(map)
void MakerLua_OnMapEnter(void)
{
    if (!Function("OnMapEnter"))
        return;
    lua_pushstring(sL, MakerLua_MapName());
    Run(sL, 1, 0);
}

}
