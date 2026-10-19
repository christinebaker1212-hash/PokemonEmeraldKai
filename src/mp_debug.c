// Developer quick-start used by the automated multiplayer tests.
// Only active when MOEMON_DEBUG_QUICKSTART is set in the environment.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "text_window.h"
#include "daycare.h"
#include "pokemon_summary_screen.h"
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
    if (strcmp(name, "crushask") == 0)
    {
        bool8 MP_Crush_DebugAsking(void);
        return MP_Crush_DebugAsking();
    }
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
    else if (strcmp(cmd, "dumpcolors") == 0)
    {
        // dumpcolors DIR - sprites and pictures for tools/moemon_frlg/colormaps.py
        void MP_Avatar_DumpColorAssets(const char *dir);
        MP_Avatar_DumpColorAssets(arg);
    }
    else if (strcmp(cmd, "card") == 0)
    {
        // card - the player's trainer card (front of it)
        void ShowPlayerTrainerCard(void (*callback)(void));
        void CB2_ReturnToFieldContinueScript(void);
        ShowPlayerTrainerCard(CB2_ReturnToFieldContinueScript);
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
    else if (strcmp(cmd, "mkbase") == 0)
    {
        // mkbase ID - make our own Secret Base there (181 = a tree on Route 110), with a few decorations
        struct SecretBase *b = &gSaveBlock1Ptr->secretBases[0];
        int id = 181;
        sscanf(arg, "%d", &id);
        memset(b, 0, sizeof(*b));
        b->secretBaseId = id;
        b->gender = gSaveBlock2Ptr->playerGender;
        memcpy(b->trainerName, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        memcpy(b->trainerId, gSaveBlock2Ptr->playerTrainerId, TRAINER_ID_LENGTH);
        b->language = GAME_LANGUAGE;
        b->decorations[0] = 1;  b->decorationPositions[0] = 0x34;   // a desk
        b->decorations[1] = 85; b->decorationPositions[1] = 0x52;   // a doll
        FlagSet(FLAG_RECEIVED_SECRET_POWER);
        printf("[test] base %d\n", id);
    }
    else if (strcmp(cmd, "daycare") == 0)
    {
        // daycare N - leave party POKéMON N at the DAY CARE
        void StoreSelectedPokemonInDaycare(void);
        int n = 0;
        sscanf(arg, "%d", &n);
        gSpecialVar_0x8004 = n;
        StoreSelectedPokemonInDaycare();
        printf("[test] daycare: %d in it\n", CountPokemonInDaycare(&gSaveBlock1Ptr->daycare));
    }
    else if (strcmp(cmd, "coopegg") == 0)
    {
        // coopegg - walk 256 steps' worth until a co-op EGG comes (up to 200 tries)
        int i;
        for (i = 0; i < 200 && gSaveBlock1Ptr->daycare.offspringPersonality == 0; i++)
        {
            gSaveBlock1Ptr->daycare.mons[0].steps |= 0xFF;
            gSaveBlock1Ptr->daycare.mons[1].steps |= 0xFF;
            MP_Daycare_TryCoopEgg(&gSaveBlock1Ptr->daycare);
        }
        printf("[test] coopegg after %d tries: pending %d\n", i, gSaveBlock1Ptr->daycare.offspringPersonality != 0);
    }
    else if (strcmp(cmd, "takeegg") == 0)
    {
        void GiveEggFromDaycare(void);
        GiveEggFromDaycare();
        printf("[test] took the egg\n");
    }
    else if (strcmp(cmd, "hof") == 0)
    {
        // hof - straight into the Hall of Fame (it saves a record)
        void CB2_DoHallOfFameScreen(void);
        SetMainCallback2(CB2_DoHallOfFameScreen);
    }
    else if (strcmp(cmd, "roamer") == 0)
    {
        void InitRoamer(void);
        gSpecialVar_0x8004 = 0;
        InitRoamer();
        printf("[test] roamer started\n");
    }
    else if (strcmp(cmd, "world") == 0)
    {
        bool8 MP_Roamer_Get(u8 *mapGroup, u8 *mapNum);
        u8 g = 0, n = 0;
        bool8 r = MP_Roamer_Get(&g, &n);
        printf("[test] world: stage %d abnormal %d roamer %d at %d.%d bases:", gSaveBlock1Ptr->weatherCycleStage,
               VarGet(VAR_ABNORMAL_WEATHER_LOCATION), r, g, n);
        for (n = 0; n < 20; n++)
            printf(" %d", gSaveBlock1Ptr->secretBases[n].secretBaseId);
        printf(" tv0kind %d\n", gSaveBlock1Ptr->tvShows[5].common.kind);
    }
    else if (strcmp(cmd, "setstage") == 0)
    {
        int v = 0;
        sscanf(arg, "%d", &v);
        gSaveBlock1Ptr->weatherCycleStage = v;
    }
    else if (strcmp(cmd, "pickup") == 0)
    {
        // pickup N - show PICKUP number N (0-18) instead of this week's
        void MP_Gatcha_SetPickup(s8 n);
        int n = -1;
        sscanf(arg, "%d", &n);
        MP_Gatcha_SetPickup(n);
    }
    else if (strcmp(cmd, "gatcharig") == 0)
    {
        // gatcharig N - the next MoeGatcha pull is this rarity (0 N, 1 U, 2 R, 3 SR, 4 SSR, 5 UR, 6 EX); gatcharig N P also gives P pulls
        void MP_Gatcha_Rig(s8 rarity);
        void MP_Gatcha_AddPulls(s32 n);
        int r = 0, p = 0;
        sscanf(arg, "%d %d", &r, &p);
        MP_Gatcha_Rig(r);
        if (p)
            MP_Gatcha_AddPulls(p);
    }
    else if (strcmp(cmd, "sprites") == 0)
    {
        // sprites - every sprite in use: position, size, tile and palette
        int i;
        for (i = 0; i < MAX_SPRITES; i++)
        {
            struct Sprite *sp = &gSprites[i];
            if (!sp->inUse)
                continue;
            printf("[test] spr %2d x %d y %d x2 %d y2 %d shape %d size %d tile %d pal %d tag %04x ptag %04x inv %d cb %p\n", i, sp->x, sp->y, sp->x2, sp->y2,
                   sp->oam.shape, sp->oam.size, sp->oam.tileNum, sp->oam.paletteNum, sp->template ? sp->template->tileTag : 0,
                   sp->template ? sp->template->paletteTag : 0, sp->invisible, (void *)sp->callback);
        }
    }
    else if (strcmp(cmd, "objtiles") == 0)
    {
        // objtiles TILE WIDTH_TILES HEIGHT_TILES - print sprite tiles from VRAM as hex digits
        int t = 0, w = 4, h = 4, x, y;
        sscanf(arg, "%d %d %d", &t, &w, &h);
        for (y = 0; y < h * 8; y++)
        {
            char line[80];
            for (x = 0; x < w * 8 && x < 64; x++)
            {
                const u8 *tile = (const u8 *)(OBJ_VRAM0) + (t + (y / 8) * w + x / 8) * 32;
                u8 c = (tile[(y % 8) * 4 + (x % 8) / 2] >> ((x & 1) * 4)) & 15;
                line[x] = "0123456789ABCDEF"[c];
            }
            line[x] = 0;
            printf("[test] %s\n", line);
        }
    }
    else if (strcmp(cmd, "follow") == 0)
    {
        // follow N - party Pokémon N walks with you (like SELECT in the party menu)
        int n = 0;
        sscanf(arg, "%d", &n);
        printf("[test] follow %d -> %d\n", n, MP_Follower_Toggle(&gPlayerParty[n % PARTY_SIZE]));
    }
    else if (strcmp(cmd, "emote") == 0)
    {
        // emote N - like the number keys (0 !, 1 ?, 2 heart, 3 note, 4-7 phrases)
        int n = 0;
        sscanf(arg, "%d", &n);
        MP_Emote_Hotkey(n);
    }
    else if (strcmp(cmd, "social") == 0)
    {
        MP_Social_Open();
    }
    else if (strcmp(cmd, "repel") == 0)
    {
        // repel - one step of REPEL left, and another REPEL in the bag
        VarSet(VAR_REPEL_STEP_COUNT, 1);
        MP_SaveData_Set("repel:last", ITEM_REPEL);
        AddBagItem(ITEM_REPEL, 1);
    }
    else if (strcmp(cmd, "summary") == 0)
    {
        extern void CB2_ReturnToField(void);
        ShowPokemonSummaryScreen(SUMMARY_MODE_NORMAL, gPlayerParty, 0, gPlayerPartyCount - 1, CB2_ReturnToField);
    }
    else if (strcmp(cmd, "frame") == 0)
    {
        // frame N - OPTIONS > FRAME (0-28; 20+ are the FireRed/LeafGreen ones)
        int n = 0;
        sscanf(arg, "%d", &n);
        gSaveBlock2Ptr->optionsWindowFrameType = n % WINDOW_FRAMES_COUNT;
    }
    else if (strcmp(cmd, "daily") == 0)
    {
        void MP_Daily_OpenMenu(void);
        MP_Daily_OpenMenu();
    }
    else if (strcmp(cmd, "giftmenu") == 0)
    {
        void MP_Gift_OpenMenu(void);
        MP_Gift_OpenMenu();
    }
    else if (strcmp(cmd, "blender") == 0)
    {
        // blender - invite everyone online to the BERRY BLENDER
        MP_Minigame_HostBlender();
    }
    else if (strcmp(cmd, "crush") == 0)
    {
        // crush - invite everyone online to BERRY CRUSH
        MP_Minigame_HostCrush();
    }
    else if (strcmp(cmd, "pokeblocks") == 0)
    {
        int i;
        printf("[test] pokeblocks:");
        for (i = 0; i < POKEBLOCKS_COUNT; i++)
            if (gSaveBlock1Ptr->pokeblocks[i].color)
                printf(" [c%d %d/%d/%d/%d/%d f%d]", gSaveBlock1Ptr->pokeblocks[i].color, gSaveBlock1Ptr->pokeblocks[i].spicy,
                       gSaveBlock1Ptr->pokeblocks[i].dry, gSaveBlock1Ptr->pokeblocks[i].sweet, gSaveBlock1Ptr->pokeblocks[i].bitter,
                       gSaveBlock1Ptr->pokeblocks[i].sour, gSaveBlock1Ptr->pokeblocks[i].feel);
        printf("  berries: cheri %d pecha %d\n", CountTotalItemQuantityInBag(ITEM_CHERI_BERRY), CountTotalItemQuantityInBag(ITEM_PECHA_BERRY));
    }
    else if (strcmp(cmd, "union") == 0)
    {
        void MP_Linkx_OpenUnion(void);
        MP_Linkx_OpenUnion();
    }
    else if (strcmp(cmd, "gatcha") == 0)
    {
        void MP_Gatcha_Open(void);
        MP_Gatcha_Open();
    }
    else if (strcmp(cmd, "menu") == 0)
    {
        // menu PAGE - open the CO-OP menu on a page (0 main, 1 look, ...)
        void MP_OpenMenuPageAt(u8 page, u8 cursor);
        int page = 0;
        sscanf(arg, "%d", &page);
        MP_OpenMenuPageAt(page, 0);
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
