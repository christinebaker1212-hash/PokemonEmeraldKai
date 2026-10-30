#ifndef MUGEN_LUAUTIL_H
#define MUGEN_LUAUTIL_H

/*
 * Types of the Lua glue (luautil.c, the engine API and the bindings): Lua
 * 5.1.4 is linked into mugen.exe (third_party/lua-5.1.4).
 */
struct lua_State;

/* A C function callable from Lua (lua_CFunction). */
typedef int (__cdecl *LuaCFunction)(struct lua_State *L);

/* An entry of a function registration array (luaL_Reg), ended by { NULL, NULL }. */
typedef struct LuaReg {
    const char *name;           /* 0x00 */
    LuaCFunction func;          /* 0x04 */
} LuaReg;
MUGEN_ASSERT_SIZE(LuaReg, 0x08);

#endif /* MUGEN_LUAUTIL_H */
