/*
 * Lua utilities (original module 0x478050-0x478606, strings "package" to
 * "Boolean expected"): the package path, calling a Lua function with C
 * arguments and results described by a signature string, and reading typed
 * fields of the table on top of the stack.
 *
 * Errors go through luaL_error, which does not return (longjmp to the
 * protected call that ran the script).
 */
#include "mugen.h"

#define LUA_GLOBALSINDEX    (-10002)
#define LUA_TNIL            0
#define LUA_TBOOLEAN        1
#define LUA_TTABLE          5
#define LUA_TFUNCTION       6

/*
 * 0x00478050: package.path = "data/?.lua" in g_luaState (scripts are
 * required from data/). Returns 0.
 */
int Lua_SetPackagePath(void)
{
    struct lua_State *L = g_luaState;

    lua_getfield(L, LUA_GLOBALSINDEX, "package");
    lua_getfield(L, -1, "path");
    lua_settop(L, -2);
    lua_pushstring(L, "data/?.lua");
    lua_setfield(L, -2, "path");
    lua_settop(L, -2);
    return 0;
}

/*
 * 0x004780B0: calls the global function func, or table.func with a table
 * (luaL_error when either is missing), with the arguments described by sig
 * before '>' and the results after it:
 *   arguments: f double, i int, s string, g the value of a global (name),
 *              t the table itself;
 *   results:   f double *, i int * (truncated), s const char ** (owned by
 *              Lua), - skipped.
 * The table is removed from the stack before the call; the results stay on
 * it.
 *
 * Quirk: 't' without a table pushes nothing but is still counted as an
 * argument.
 */
void Lua_CallFunction(struct lua_State *L, const char *table, const char *func, const char *sig, ...)
{
    va_list ap;
    int narg = 0, nres, idx;

    va_start(ap, sig);
    if (table != NULL) {
        lua_getfield(L, LUA_GLOBALSINDEX, table);
        if (lua_type(L, -1) != LUA_TTABLE) {
            luaL_error(L, "%s not a valid table", table);
        }
        lua_pushstring(L, func);
        lua_gettable(L, -2);
        if (lua_type(L, -1) != LUA_TFUNCTION) {
            luaL_error(L, "%s.%s not a valid function", table, func);
        }
    } else {
        lua_getfield(L, LUA_GLOBALSINDEX, func);
    }
    if (*sig != '\0') {
        int self_idx = -2;
        do {
            char c = *sig++;
            if (c == '>') {
                break;
            }
            switch (c) {
            case 'f':
                lua_pushnumber(L, va_arg(ap, double));
                break;
            case 'i':
                lua_pushnumber(L, (double)va_arg(ap, int));
                break;
            case 's':
                lua_pushstring(L, va_arg(ap, const char *));
                break;
            case 't':
                if (table != NULL) {
                    lua_pushvalue(L, self_idx);
                }
                break;
            case 'g':
                lua_getfield(L, LUA_GLOBALSINDEX, va_arg(ap, const char *));
                break;
            default:
                luaL_error(L, "invalid option (%c)", c);
                break;
            }
            narg++;
            self_idx--;
            luaL_checkstack(L, 1, "too many arguments");
        } while (*sig != '\0');
    }
    if (table != NULL) {
        lua_remove(L, -2 - narg);
    }
    nres = (int)strlen(sig);
    lua_call(L, narg, nres);
    idx = -nres;
    while (*sig != '\0') {
        char c = *sig++;
        switch (c) {
        case 'f':
            if (!lua_isnumber(L, idx)) {
                luaL_error(L, "wrong result type");
            }
            *va_arg(ap, double *) = lua_tonumber(L, idx);
            break;
        case 'i':
            if (!lua_isnumber(L, idx)) {
                luaL_error(L, "wrong result type");
            }
            *va_arg(ap, int *) = Ftol(lua_tonumber(L, idx));
            break;
        case 's':
            if (!lua_isstring(L, idx)) {
                luaL_error(L, "wrong result type");
            }
            *va_arg(ap, const char **) = lua_tolstring(L, idx, NULL);
            break;
        case '-':
            break;
        default:
            luaL_error(L, "invalid option (%c)", c);
            break;
        }
        idx++;
    }
    va_end(ap);
}

/*
 * 0x004783F0: table[key] (table on top of the stack) as a number in *out.
 * Returns 0, or -1 when the field is nil (out unchanged); luaL_error when
 * it is not a number.
 */
int LuaTable_GetNumber(struct lua_State *L, const char *key, double *out)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (!lua_isnumber(L, -1)) {
        return luaL_error(L, "Number expected");
    }
    *out = lua_tonumber(L, -1);
    lua_settop(L, -2);
    return 0;
}

/* 0x00478460: table[key] as an integer (lua_tointeger), as LuaTable_GetNumber. */
int LuaTable_GetInt(struct lua_State *L, const char *key, int *out)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (!lua_isnumber(L, -1)) {
        return luaL_error(L, "Number expected");
    }
    *out = lua_tointeger(L, -1);
    lua_settop(L, -2);
    return 0;
}

/* 0x004784D0: table[key] as a boolean (must be one), as LuaTable_GetNumber. */
int LuaTable_GetBool(struct lua_State *L, const char *key, int *out)
{
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (lua_type(L, -1) != LUA_TBOOLEAN) {
        return luaL_error(L, "Boolean expected");
    }
    *out = lua_toboolean(L, -1);
    lua_settop(L, -2);
    return 0;
}

/*
 * 0x00478540: pushes table.remappal, which must be a table (luaL_error
 * otherwise). Returns 0, or -1 (nothing pushed) when it is nil.
 */
int LuaTable_GetRemapPal(struct lua_State *L)
{
    lua_pushstring(L, "remappal");
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (lua_type(L, -1) != LUA_TTABLE) {
        return luaL_error(L, "Table expected");
    }
    return 0;
}

/* 0x004785A0: table[index] as an integer, as LuaTable_GetInt. */
int LuaTable_GetIntAt(struct lua_State *L, int index, int *out)
{
    lua_pushinteger(L, index);
    lua_gettable(L, -2);
    if (lua_type(L, -1) == LUA_TNIL) {
        lua_settop(L, -2);
        return -1;
    }
    if (!lua_isnumber(L, -1)) {
        return luaL_error(L, "Number expected");
    }
    *out = lua_tointeger(L, -1);
    lua_settop(L, -2);
    return 0;
}
