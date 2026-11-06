// Moemon Maker, M5: what a Lua hook may touch in the game (src/maker/maker_lua.cpp is the Lua side: C++ like EDOPro's
// Lua 5.4, so it never includes the game's headers and asks through these). Names are the game's (FLAG_ / VAR_ /
// ITEM_ / SPECIES_ ...) or, for flags, a name of the user's own (a Maker flag, as events use).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "maker.h"
#include "maker_api.h"

void MPNet_Log(const char *fmt, ...);

static int Named(int table, const char *name)
{
    unsigned v;
    s32 sv;
    if (MakerApi_ValueOf(table, name, &v))
        return (int)v;
    if (Maker_RawSymbol(name, &sv))   // (any constant the game's scripts name)
        return sv;
    return -1;
}

static int FlagId(const char *name)
{
    if (strncmp(name, "FLAG_", 5) == 0)
        return Named(MAKER_NAMES_FLAG, name);
    return Maker_FlagId(name);
}

int MakerLua_Flag(const char *name) { int f = FlagId(name); return f >= 0 ? FlagGet(f) != 0 : -1; }
int MakerLua_SetFlag(const char *name, int on)
{
    int f = FlagId(name);
    if (f < 0)
        return 0;
    if (on)
        FlagSet(f);
    else
        FlagClear(f);
    return 1;
}
int MakerLua_Var(const char *name) { int v = Named(MAKER_NAMES_VAR, name); return v >= 0 ? VarGet(v) : -1; }
int MakerLua_SetVar(const char *name, int value)
{
    int v = Named(MAKER_NAMES_VAR, name);
    return v >= 0 && VarSet(v, (u16)value);
}
int MakerLua_Item(const char *name) { int i = Named(MAKER_NAMES_ITEM, name); return i > 0 ? CountTotalItemQuantityInBag(i) : -1; }
int MakerLua_GiveItem(const char *name, int n)
{
    int i = Named(MAKER_NAMES_ITEM, name);
    if (i <= 0 || n <= 0)
        return 0;
    return AddBagItem(i, n);
}
int MakerLua_TakeItem(const char *name, int n)
{
    int i = Named(MAKER_NAMES_ITEM, name);
    if (i <= 0 || n <= 0 || !CheckBagHasItem(i, n))
        return 0;
    return RemoveBagItem(i, n);
}
int MakerLua_Money(void) { return (int)GetMoney(&gSaveBlock1Ptr->money); }
void MakerLua_AddMoney(int n)
{
    if (n >= 0)
        AddMoney(&gSaveBlock1Ptr->money, n);
    else
        RemoveMoney(&gSaveBlock1Ptr->money, -n);
}
int MakerLua_PartyCount(void) { return CalculatePlayerPartyCount(); }
const char *MakerLua_PartySpecies(int slot)   // (the SPECIES_ name, "" past the party)
{
    if (slot < 0 || slot >= PARTY_SIZE || !GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES))
        return "";
    return MakerApi_NameOf(MAKER_NAMES_SPECIES, GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES));
}
int MakerLua_PartyLevel(int slot)
{
    if (slot < 0 || slot >= PARTY_SIZE || !GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES))
        return 0;
    return GetMonData(&gPlayerParty[slot], MON_DATA_LEVEL);
}
const char *MakerLua_MapName(void)
{
    int m = Maker_MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    return m >= 0 ? Maker_Map(m)->name : "";
}
int MakerLua_PlayerX(void) { return gSaveBlock1Ptr->pos.x; }
int MakerLua_PlayerY(void) { return gSaveBlock1Ptr->pos.y; }
int MakerLua_Random(int n) { return n > 0 ? Random() % n : 0; }   // (the game's own: the same on every game of a link)
void MakerLua_Log(const char *s) { MPNet_Log("maker lua: %s", s); }

// A hook's answer naming an event of this map: that event starts as if its person were talked to
const u8 *Maker_MapEventScript(const char *name);
bool8 MakerLua_StartEvent(const char *name)
{
    const u8 *script = Maker_MapEventScript(name);
    if (!script)
        return FALSE;
    ScriptContext_SetupScript(script);
    return TRUE;
}
