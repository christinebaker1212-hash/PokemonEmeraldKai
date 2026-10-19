// Developer quick-start used by the automated multiplayer tests.
// Only active when MOEMON_DEBUG_QUICKSTART is set in the environment.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "constants/battle.h"
#include "script.h"
#include "pokedex.h"
#include "mp.h"
#include "palette.h"
#include "constants/characters.h"
#include "main.h"
#include "battle_setup.h"
#include "pokemon.h"
#include "event_data.h"
#include "overworld.h"
#include "money.h"
#include "item.h"
#include "string_util.h"
#include "constants/species.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

bool8 MP_DebugQuickStartEnabled(void)
{
    return getenv("MOEMON_DEBUG_QUICKSTART") != NULL;
}

static void SetAsciiName(u8 *dst, const char *src, int maxLen)
{
    int i;
    for (i = 0; i < maxLen && src[i]; i++)
    {
        char c = src[i];
        if (c >= 'A' && c <= 'Z') dst[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z') dst[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9') dst[i] = 0xA1 + (c - '0');
        else dst[i] = 0x00;
    }
    dst[i] = EOS;
}

void MP_DebugSetupNameAndGender(void)
{
    const char *name = getenv("MOEMON_DEBUG_NAME");
    const char *gender = getenv("MOEMON_DEBUG_GENDER");
    SetAsciiName(gSaveBlock2Ptr->playerName, name ? name : "TEST", PLAYER_NAME_LENGTH);
    gSaveBlock2Ptr->playerGender = (gender && gender[0] == 'F') ? FEMALE : MALE;
}

// Called at the end of NewGameInitData in quick-start mode
void MP_DebugQuickStartNewGame(void)
{
    const char *partyEnv = getenv("MOEMON_DEBUG_PARTY"); // e.g. "258:20,280:18+" (+ = about to level up)
    const char *mapEnv = getenv("MOEMON_DEBUG_WARP");     // "group,num,x,y"
    int group = MAP_GROUP(MAP_OLDALE_TOWN), num = MAP_NUM(MAP_OLDALE_TOWN), x = 10, y = 10;
    char buf[256];
    int slot = 0;

    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_ADVENTURE_STARTED);
    // MOEMON_DEBUG_FLAGS: extra flags to set, e.g. "0x870,0x876" or "visited" (every town)
    {
        const char *flags = getenv("MOEMON_DEBUG_FLAGS");
        if (flags && strcmp(flags, "visited") == 0)
        {
            u16 f;
            for (f = FLAG_VISITED_LITTLEROOT_TOWN; f <= FLAG_VISITED_EVER_GRANDE_CITY; f++)
                FlagSet(f);
        }
        else if (flags)
        {
            char fb[256], *tok;
            snprintf(fb, sizeof(fb), "%s", flags);
            for (tok = strtok(fb, ","); tok; tok = strtok(NULL, ","))
                FlagSet((u16)strtoul(tok, NULL, 0));
        }
    }
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    VarSet(VAR_LITTLEROOT_TOWN_STATE, 4);
    VarSet(VAR_BIRCH_LAB_STATE, 5);
    VarSet(VAR_ROUTE101_STATE, 3);
    SetMoney(&gSaveBlock1Ptr->money, 5000);
    AddBagItem(ITEM_POKE_BALL, 5);
    AddBagItem(ITEM_POTION, 5);
    AddBagItem(ITEM_BATTLE_FLAG, 1);
    gSaveBlock1Ptr->registeredItem = ITEM_BATTLE_FLAG;
    if (getenv("MOEMON_DEBUG_MONEY"))
        SetMoney(&gSaveBlock1Ptr->money, atoi(getenv("MOEMON_DEBUG_MONEY")));

    ZeroPlayerPartyMons();
    snprintf(buf, sizeof(buf), "%s", partyEnv ? partyEnv : "280:12,283:10");
    for (char *tok = strtok(buf, ","); tok && slot < PARTY_SIZE; tok = strtok(NULL, ","))
    {
        int species = 0, level = 5;
        if (sscanf(tok, "%d:%d", &species, &level) >= 1 && species > 0 && species < NUM_SPECIES)
        {
            CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
            if (strchr(tok, '+') && level < MAX_LEVEL) // "280:15+" = one exp point short of the next level
            {
                u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - 1;
                SetMonData(&gPlayerParty[slot], MON_DATA_EXP, &exp);
            }
            slot++;
        }
    }
    CalculatePlayerPartyCount();

    if (mapEnv)
        sscanf(mapEnv, "%d,%d,%d,%d", &group, &num, &x, &y);
    SetLastHealLocationWarp(HEAL_LOCATION_OLDALE_TOWN);
    SetWarpDestination(group, num, WARP_ID_NONE, x, y);
    WarpIntoMap();
}

// Test harness conditions (src/platform/sdl2.c: waitfor / pressuntil)
bool8 MP_Setup_IsReady(void);
bool8 MP_Setup_ScreenIs(int screen);
int MP_TestCondition(const char *name)
{
    if (strcmp(name, "setup") == 0)
        return MP_Setup_IsReady();
    if (strcmp(name, "field") == 0)
        return MP_IsLocalFree();
    if (strcmp(name, "online") == 0)
        return MP_IsConnected() && MP_ConnectedCount() >= 2;
    if (strcmp(name, "inroom") == 0)
        return MP_Lobby_InRoom();
    if (strcmp(name, "joinlist") == 0)
        return MP_Setup_ScreenIs(2);
    if (strcmp(name, "message") == 0)
        return MP_Setup_ScreenIs(4);
    if (strcmp(name, "battle") == 0)
        return gMain.inBattle;
    if (strcmp(name, "nofade") == 0)
        return !gPaletteFade.active;
    if (strcmp(name, "action") == 0)
    {
        bool8 MP_Test_IsChoosingAction(void);
        return gMain.inBattle && MP_Test_IsChoosingAction();
    }
    return 0;
}

// Test harness: "mp COMMAND ARG"
void MP_OnFileDropped(const char *path);
void MP_TestCommand(const char *cmd, const char *arg)
{
    if (strcmp(cmd, "export") == 0)
    {
        char path[200];
        bool8 ok = MP_Avatar_ExportLocal(path, sizeof(path));
        printf("[test] export %s: %s\n", path, ok ? "ok" : "FAILED");
    }
    else if (strcmp(cmd, "wild") == 0)
    {
        // wild SPECIES:LEVEL - start a wild battle right now
        int species = 263, level = 3;
        sscanf(arg, "%d:%d", &species, &level);
        ZeroEnemyPartyMons();
        CreateMon(&gEnemyParty[0], species, level, 32, 0, 0, OT_ID_PLAYER_ID, 0);
        BattleSetup_StartWildBattle();
    }
    else if (strcmp(cmd, "townmap") == 0 || strcmp(cmd, "flymap") == 0)
    {
        extern void FieldInitRegionMap(MainCallback callback);
        extern void CB2_OpenFlyMap(void);
        extern void CB2_ReturnToField(void);
        if (cmd[0] == 't')
            FieldInitRegionMap(CB2_ReturnToField);
        else
            SetMainCallback2(CB2_OpenFlyMap);
    }
    else if (strcmp(cmd, "dexcatch") == 0 || strcmp(cmd, "dexsee") == 0)
    {
        // dexcatch NATIONAL_NUMBER: as if you caught (or saw) it
        u16 nat = (u16)atoi(arg);
        GetSetPokedexFlag(nat, FLAG_SET_SEEN);
        if (cmd[3] == 'c')
            GetSetPokedexFlag(nat, FLAG_SET_CAUGHT);
        printf("[test] dex %d seen %d caught %d\n", nat, GetSetPokedexFlag(nat, FLAG_GET_SEEN), GetSetPokedexFlag(nat, FLAG_GET_CAUGHT));
    }
    else if (strcmp(cmd, "dexget") == 0)
    {
        u16 nat = (u16)atoi(arg);
        printf("[test] dex %d caught by %s\n", nat, MP_Dex_CaughtBy(nat) ? "a friend" : "-");
        printf("[test] dex %d seen %d caught %d\n", nat, GetSetPokedexFlag(nat, FLAG_GET_SEEN), GetSetPokedexFlag(nat, FLAG_GET_CAUGHT));
    }
    else if (strcmp(cmd, "item") == 0)
    {
        // item ID QTY - put items in the bag
        int id = 1, qty = 1;
        sscanf(arg, "%d %d", &id, &qty);
        printf("[test] item %d x%d: %s\n", id, qty, AddBagItem(id, qty) ? "ok" : "FAILED");
    }
    else if (strcmp(cmd, "party") == 0)
    {
        int i;
        printf("[test] party:");
        for (i = 0; i < PARTY_SIZE; i++)
            printf(" %d", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
        printf("  pc box1 slot1 %d\n", (int)GetBoxMonDataAt(0, 0, MON_DATA_SPECIES));
    }
    else if (strcmp(cmd, "badges") == 0)
    {
        int b;
        printf("[test] badges:");
        for (b = 0; b < 8; b++)
            printf(" %d", FlagGet(FLAG_BADGE01_GET + b));
        printf("  money %u\n", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
    }
    else if (strcmp(cmd, "ctx") == 0)
    {
        extern int Platform_InputContext(void);
        printf("[test] ctx %d free %d overworld %d locked %d script %d fade %d\n", Platform_InputContext(), MP_IsLocalFree(),
               MP_IsInOverworld(), ArePlayerFieldControlsLocked(), ScriptContext_IsEnabled(), gPaletteFade.active);
    }
    else if (strcmp(cmd, "event") == 0)
    {
        // event trainer|coop|wild|catch LEVEL|trade|gift [count]
        char what[16] = "";
        int n = 1, level = 10, i;
        sscanf(arg, "%15s %d %d", what, &n, &level);
        for (i = 0; i < n; i++)
        {
            if (strcmp(what, "trainer") == 0) MP_Events_OnTrainerWin();
            else if (strcmp(what, "coop") == 0) MP_Events_OnCoopWin((u8)level);
            else if (strcmp(what, "wild") == 0) MP_Events_OnWildEnd(B_OUTCOME_WON, level);
            else if (strcmp(what, "catch") == 0) MP_Events_OnWildEnd(B_OUTCOME_CAUGHT, level);
            else if (strcmp(what, "trade") == 0) MP_Events_OnTrade();
            else if (strcmp(what, "gift") == 0) MP_Events_OnGiftSent();
        }
    }
    else if (strcmp(cmd, "savedata") == 0)
    {
        // savedata KEY VALUE (KEY may contain TODAY, replaced by today's date)
        char key[64] = "", out[64];
        int v = 0;
        char *t;
        sscanf(arg, "%63s %d", key, &v);
        if ((t = strstr(key, "TODAY")))
        {
            *t = 0;
            snprintf(out, sizeof(out), "%s%d%s", key, (int)MP_Today(), t + 5);
        }
        else
            snprintf(out, sizeof(out), "%s", key);
        MP_SaveData_Set(out, v);
        printf("[test] savedata %s = %d\n", out, v);
    }
    else if (strcmp(cmd, "goals") == 0)
    {
        void MP_Events_DebugGoals(void);
        MP_Events_DebugGoals();
    }
    else if (strcmp(cmd, "outbreak") == 0)
    {
        int i, hits = 0;
        for (i = 0; i < 1000; i++)
            hits += MP_Events_MaybeOutbreak(SPECIES_ZIGZAGOON) != SPECIES_ZIGZAGOON;
        printf("[test] outbreak hits %d/1000 here\n", hits);
    }
    else if (strcmp(cmd, "battleflag") == 0)
    {
        MP_Trainer_Toggle();
    }
    else if (strcmp(cmd, "objs") == 0)
    {
        void MP_Overworld_DebugDump(void);
        MP_Overworld_DebugDump();
    }
    else if (strcmp(cmd, "drop") == 0)
    {
        MP_OnFileDropped(arg);
    }
    fflush(stdout);
}
