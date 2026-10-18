// Developer quick-start used by the automated multiplayer tests.
// Only active when MOEMON_DEBUG_QUICKSTART is set in the environment.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "palette.h"
#include "constants/characters.h"
#include "main.h"
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
    const char *partyEnv = getenv("MOEMON_DEBUG_PARTY"); // e.g. "258:20,280:18"
    const char *mapEnv = getenv("MOEMON_DEBUG_WARP");     // "group,num,x,y"
    int group = MAP_GROUP(MAP_OLDALE_TOWN), num = MAP_NUM(MAP_OLDALE_TOWN), x = 10, y = 10;
    char buf[256];
    int slot = 0;

    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_ADVENTURE_STARTED);
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
    return 0;
}
