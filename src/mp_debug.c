// Developer quick-start used by the automated multiplayer tests.
// Only active when MOEMON_DEBUG_QUICKSTART is set in the environment.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "easy_chat.h"
#include "field_screen_effect.h"
#include "field_player_avatar.h"
#include "event_object_movement.h"
#include "text_window.h"
#include "daycare.h"
#include "pokemon_summary_screen.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "constants/battle.h"
#include "script.h"
#include "pokedex.h"
#include "mp.h"
#include "battle.h"
#include "save.h"
#include "palette.h"
#include "constants/characters.h"
#include "main.h"
#include "battle_setup.h"
#include "pokemon.h"
#include "constants/moves.h"
#include "event_data.h"
#include "coins.h"
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
    if (strcmp(name, "log") == 0)   // harness waitlog
    {
        extern int gMpLogSeen;
        return gMpLogSeen;
    }
    if (strcmp(name, "setup") == 0)
        return MP_Setup_IsReady();
    if (strcmp(name, "field") == 0)
        return MP_IsLocalFree();
    if (strcmp(name, "down") == 0)
        return MP_Rescue_Down();
    if (strcmp(name, "helpask") == 0)
        return MP_Rescue_Asked();
    if (strcmp(name, "photo") == 0)
        return MP_Photo_Done();
    if (strcmp(name, "notice") == 0)
    {
        bool8 MP_Notice_Active(void);
        return MP_Notice_Active();
    }
    if (strcmp(name, "naming") == 0)
    {
        bool8 NamingScreen_IsActive(void);
        return NamingScreen_IsActive() && !gPaletteFade.active;
    }
    if (strcmp(name, "introlook") == 0)
    {
        bool8 MP_IntroLook_Active(void);
        return MP_IntroLook_Active();
    }
    if (strcmp(name, "online") == 0)
        return MP_IsConnected() && MP_ConnectedCount() >= 2;
    if (strcmp(name, "inroom") == 0)
        return MP_Lobby_InRoom();
    if (strcmp(name, "joinlist") == 0)
        return MP_Setup_ScreenIs(2);
    if (strcmp(name, "message") == 0)
        return MP_Setup_ScreenIs(4);
    if (strcmp(name, "easychat") == 0)   // the easy chat word screen is up
    {
        bool8 MP_EasyChatOpen(void);
        return MP_EasyChatOpen();
    }
    if (strcmp(name, "yesno") == 0)   // a script's YES / NO box is up
    {
        bool8 MP_ScriptYesNoOpen(void);
        return MP_ScriptYesNoOpen();
    }
    if (strcmp(name, "battle") == 0)
        return gMain.inBattle;
    if (strcmp(name, "partymenu") == 0)   // the party screen is up (e.g. picking the next Moemon in battle)
    {
        bool8 MP_PartyMenu_IsOpen(void);
        return MP_PartyMenu_IsOpen() && !gPaletteFade.active;
    }
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
    else if (strcmp(cmd, "triple") == 0)
    {
        // triple S:L S:L S:L [S:L...] - a local wild Triple Battle (you command party
        // slots 0-2); up to six foes, the 4th-6th come in when one faints
        void BattleSetup_StartLocalTripleWildBattle(void);
        int sp[PARTY_SIZE] = {288, 286, 290}, lv[PARTY_SIZE] = {5, 5, 5}, i, n = 0, used;
        const char *p = arg;
        while (n < PARTY_SIZE && sscanf(p, "%d:%d%n", &sp[n], &lv[n], &used) == 2)
            p += used, n++;
        if (n < 3)
            n = 3;
        ZeroEnemyPartyMons();
        for (i = 0; i < n; i++)
            CreateMon(&gEnemyParty[i], sp[i], lv[i], 32, 0, 0, OT_ID_PLAYER_ID, 0);
        gMpDebugTriple = getenv("MOEMON_TRIPLEDEBUG") != NULL;
        BattleSetup_StartLocalTripleWildBattle();
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
    else if (strcmp(cmd, "giveitemscript") == 0)
    {
        // giveitemscript ID - an NPC-style giveitem (the obtain message, a MOEGATCHA TICKET for important items)
        extern const u8 EventScript_MpDebugGiveItem[];
        int id = 0;
        sscanf(arg, "%d", &id);
        VarSet(VAR_0x8004, id);
        ScriptContext_SetupScript(EventScript_MpDebugGiveItem);
    }
    else if (strcmp(cmd, "additem") == 0)
    {
        // additem ID N - straight into the bag
        int id = 0, n = 1;
        sscanf(arg, "%d %d", &id, &n);
        AddBagItem(id, n);
    }
    else if (strcmp(cmd, "coins") == 0)
    {
        int n = 0;
        sscanf(arg, "%d", &n);
        SetCoins(n);
    }
    else if (strcmp(cmd, "setflag") == 0)
    {
        // setflag N - an event flag (include/constants/flags.h)
        FlagSet(atoi(arg));
    }
    else if (strcmp(cmd, "ticketbackfill") == 0)
    {
        // ticketbackfill [reset] - the once-per-save MOEGATCHA TICKETS for story items already got (reset: as if never run)
        if (strcmp(arg, "reset") == 0)
            MP_SaveData_Set("gatcha:ticketbackfill", 0);
        MP_Gatcha_BackfillTickets();
    }
    else if (strcmp(cmd, "gatchastock") == 0)
    {
        printf("[test] gatcha tickets %d free %d\n", (int)MP_Gatcha_Tickets(), (int)MP_Gatcha_FreeWaiting());
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
    else if (strcmp(cmd, "palvacate") == 0)
    {
        // palvacate SLOT - the game writes an NPC reflection palette there
        void MP_VacatePaletteSlot(u8 slot);
        int n = 0;
        sscanf(arg, "%d", &n);
        MP_VacatePaletteSlot(n);
    }
    else if (strcmp(cmd, "palcoop") == 0)
    {
        unsigned t = 0;
        sscanf(arg, "%x", &t);
        printf("[test] palcoop %04x -> %d\n", t, MP_AllocCoopPalette(t, NULL));
    }
        else if (strcmp(cmd, "palfree") == 0)
    {
        unsigned t = 0;
        sscanf(arg, "%x", &t);
        FreeSpritePaletteByTag(t);
    }
        else if (strcmp(cmd, "palfill") == 0)
    {
        // palfill N - the game grabs N more sprite palettes (0 = give them back)
        static const u16 sJunk[16] = {0};
        int n = 0, i;
        sscanf(arg, "%d", &n);
        if (n == 0)
            for (i = 0; i < 8; i++)
                FreeSpritePaletteByTag(0x7700 + i);
        for (i = 0; i < n; i++)
        {
            struct SpritePalette pal = {sJunk, 0x7700 + i};
            printf("[test] palfill %04x -> %d\n", 0x7700 + i, LoadSpritePalette(&pal));
        }
    }
        else if (strcmp(cmd, "crash") == 0)
    {
        // crash - test crash_log.txt
        volatile int *p = NULL;
        *p = 1;
    }
        else if (strcmp(cmd, "dumppltt") == 0)
    {
        // dumppltt FILE - palette RAM, both palette buffers and the display registers (binary)
        FILE *f = fopen(arg, "wb");
        if (f)
        {
            fwrite((void *)PLTT, 1, PLTT_SIZE, f);
            fwrite(gPlttBufferUnfaded, 1, PLTT_SIZE, f);
            fwrite(gPlttBufferFaded, 1, PLTT_SIZE, f);
            fwrite((void *)REG_BASE, 1, 0x60, f);
            fclose(f);
        }
        printf("[test] pltt dumped: fade active %d y %d blend %04x sel %08x\n", gPaletteFade.active, gPaletteFade.y,
               gPaletteFade.blendColor, gPaletteFade.multipurpose1);
    }
        else if (strcmp(cmd, "pals") == 0)
    {
        // pals - which tag holds each sprite palette slot
        extern u8 gReservedSpritePaletteCount;
        u16 GetSpritePaletteTagByPaletteNum(u8 paletteNum);
        int i;
        printf("[test] pals reserved=%d:", gReservedSpritePaletteCount);
        for (i = 0; i < OBJ_PAL_SLOTS; i++)
            if (i < 16 || GetSpritePaletteTagByPaletteNum(i) != 0xFFFF)
                printf(" %d=%04x", i, GetSpritePaletteTagByPaletteNum(i));
        printf("\n");
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
    else if (strcmp(cmd, "emotekind") == 0)
    {
        // emotekind K - any MP_EMOTE_* (5-16: the extra emotes)
        int k = 0;
        sscanf(arg, "%d", &k);
        printf("[test] emote %d %s\n", k, MP_Emote_Name(k));
        MP_Emote_Send(k);
    }
    else if (strcmp(cmd, "battleicon") == 0)
    {
        // battleicon K - a battle bubble with emote K (5-16) over our Moemon
        int k = 0;
        sscanf(arg, "%d", &k);
        if (k >= MP_EMOTE_FIRST_EXTRA && k < MP_EMOTE_COUNT)
            MP_Emote_BattleSay(NULL, MP_BUBBLE_ICON_EXTRA + k - MP_EMOTE_FIRST_EXTRA);
    }
    else if (strcmp(cmd, "quick") == 0)
    {
        // quick PAGE - the emote / phrase quick pick, open (EMOTES 0-3, MINE 4-11, stock 12-15)
        int page = 0;
        sscanf(arg, "%d", &page);
        MP_OpenQuickPickAt(page, -1, -1);
    }
    else if (strcmp(cmd, "social") == 0)
    {
        MP_Social_Open();
    }
    else if (strcmp(cmd, "bhp") == 0)
    {
        int b;
        for (b = 0; b < 4; b++)
            printf("[test] battler %d hp %d/%d species %d\n", b, gBattleMons[b].hp, gBattleMons[b].maxHP, gBattleMons[b].species);
    }
    else if (strcmp(cmd, "register") == 0)
    {
        // mp register ITEM: adds it and registers it to SELECT
        u16 item = atoi(arg);
        AddBagItem(item, 1);
        gSaveBlock1Ptr->registeredItem = item;
    }
    else if (strcmp(cmd, "fam") == 0)
    {
        int Gamepad_IconFamily(void);
        printf("[test] icon family %d\n", Gamepad_IconFamily());
    }
    else if (strcmp(cmd, "near") == 0)
    {
        int i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gObjectEvents[i].active)
                printf("[test] obj %d gfx %d at %d,%d facing %d local %d\n", i, gObjectEvents[i].graphicsId,
                       gObjectEvents[i].currentCoords.x, gObjectEvents[i].currentCoords.y,
                       gObjectEvents[i].facingDirection, gObjectEvents[i].localId);
    }
    else if (strcmp(cmd, "lend") == 0)
        MP_Lend_Debug();
    else if (strcmp(cmd, "teach") == 0)
    {
        // mp teach SLOT MOVE: replaces the first move
        int slot = atoi(arg), move = 0;
        const char *sp = strchr(arg, ' ');
        if (sp)
            move = atoi(sp + 1);
        if (slot >= 0 && slot < PARTY_SIZE && move > 0 && move < MOVES_COUNT)
        {
            u16 m = move;
            SetMonData(&gPlayerParty[slot], MON_DATA_MOVE1, &m);
            printf("[test] teach: slot %d move %d\n", slot, move);
        }
    }
    else if (strcmp(cmd, "hold") == 0)
    {
        // mp hold SLOT ITEM - the party Moemon in SLOT holds ITEM
        int slot = atoi(arg), v = 0;
        const char *sp = strchr(arg, ' ');
        if (sp)
            v = atoi(sp + 1);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u16 item = v;
            SetMonData(&gPlayerParty[slot], MON_DATA_HELD_ITEM, &item);
            printf("[test] hold: slot %d = item %d\n", slot, v);
        }
    }
    else if (strcmp(cmd, "hp") == 0)
    {
        // mp hp SLOT VALUE
        int slot = atoi(arg), v = 0;
        const char *sp = strchr(arg, ' ');
        if (sp)
            v = atoi(sp + 1);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u16 hp = v;
            SetMonData(&gPlayerParty[slot], MON_DATA_HP, &hp);
            printf("[test] hp: slot %d = %d\n", slot, v);
        }
    }
    else if (strcmp(cmd, "givebadges") == 0)
    {
        int b;
        for (b = 0; b < 8; b++)
            FlagSet(FLAG_BADGE01_GET + b);
    }
    else if (strcmp(cmd, "photo") == 0)
    {
        const char *why = MP_Photo_Check();
        if (why)
            printf("[test] photo: %s\n", why);
        else
            MP_Photo_Start();
    }
    else if (strcmp(cmd, "options") == 0)
    {
        // options - the OPTIONS screen (back to the field)
        void CB2_InitOptionMenu(void);
        void CB2_ReturnToField(void);
        gMain.savedCallback = CB2_ReturnToField;
        SetMainCallback2(CB2_InitOptionMenu);
    }
    else if (strcmp(cmd, "creatures") == 0 || strcmp(cmd, "namecase") == 0)
    {
        // creatures 0|1 (MOEMON / POKEMON pictures and word), namecase 0|1|2 (as typed / ALL CAPS / Mixed Case)
        if (cmd[0] == 'c')
            gSaveBlock2Ptr->optionsPokemonArt = atoi(arg) != 0;
        else
            gSaveBlock2Ptr->optionsNameCase = atoi(arg) % MP_NAMECASE_COUNT;
        printf("[test] creatures %d namecase %d\n", gSaveBlock2Ptr->optionsPokemonArt, gSaveBlock2Ptr->optionsNameCase);
    }
    else if (strcmp(cmd, "namefilter") == 0)
    {
        // namefilter TEXT - log TEXT as OPTIONS > NAME CASE / CREATURES would show it (~ is é)
        static u8 in[200];
        const u8 *f;
        int i;
        MP_AsciiToGame(in, arg, sizeof(in) - 1);
        for (i = 0; arg[i] && i < (int)sizeof(in) - 1; i++)
            if (arg[i] == '~')
                in[i] = CHAR_e_ACUTE;
        f = MP_TextOpts_Filter(in, 0);
        printf("[test] namefilter <");
        for (i = 0; f[i] != EOS; i++)
        {
            u8 one[2] = {f[i], EOS};
            char c[8];
            MP_GameToAscii(c, one, sizeof(c) - 1);
            printf("%s", f[i] == CHAR_e_ACUTE ? "~" : c);
        }
        printf(">\n");
    }
    else if (strcmp(cmd, "legacyrand") == 0)
    {
        // legacyrand B,B,... - the random bytes the next GENESIS / EPOCH BALL throws use
        u8 bytes[16];
        int n = 0, v, used;
        const char *p = arg;
        while (n < 16 && sscanf(p, "%d%n", &v, &used) == 1)
        {
            bytes[n++] = (u8)v;
            p += used;
            if (*p == ',')
                p++;
        }
        MP_LegacyBall_RigRandom(bytes, n);
    }
    else if (strcmp(cmd, "typeicons") == 0)
    {
        // typeicons [0|1] - OPTIONS > TYPE ICONS
        gSaveBlock2Ptr->optionsTypeIcons = arg[0] ? atoi(arg) != 0 : 1;
    }
    else if (strcmp(cmd, "wilddouble") == 0)
    {
        extern u8 gMpWildDoublePercent;
        gMpWildDoublePercent = arg[0] ? atoi(arg) : 100;
    }
    else if (strcmp(cmd, "bag") == 0)
    {
        // bag - how many kinds of item each pocket holds, and the biggest stack
        u8 p;
        for (p = 0; p < POCKETS_COUNT; p++)
        {
            u16 i, kinds = 0, biggest = 0;
            for (i = 0; i < gBagPockets[p].capacity; i++)
                if (gBagPockets[p].itemSlots[i].itemId != ITEM_NONE)
                {
                    u16 q = gBagPockets[p].itemSlots[i].quantity ^ (u16)gSaveBlock2Ptr->encryptionKey;
                    kinds++;
                    if (q > biggest)
                        biggest = q;
                }
            printf("[test] bag pocket %d: %d kinds (of %d), biggest stack %d\n", p, kinds, gBagPockets[p].capacity, biggest);
        }
    }
    else if (strcmp(cmd, "save") == 0)
    {
        u8 TrySavingData(u8 saveType);
        TrySavingData(SAVE_NORMAL);
    }
    else if (strcmp(cmd, "fish") == 0)
    {
        // fish ROD - cast a rod (0 old, 1 good, 2 super) where you stand
        StartFishing(atoi(arg));
    }
    else if (strcmp(cmd, "avatarstate") == 0)
    {
        // avatarstate N - the player's sprite for PLAYER_AVATAR_STATE_* N (5 field move, 6 fishing, 7 watering...)
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        ObjectEventSetGraphicsId(player, GetPlayerAvatarGraphicsIdByStateId(atoi(arg)));
        StartSpriteAnim(&gSprites[player->spriteId], 0);
        printf("[test] avatar state %d\n", atoi(arg));
    }
    else if (strcmp(cmd, "bike") == 0)
    {
        // bike - get on the Mach Bike (or off: bike 0)
        extern void SetPlayerAvatarTransitionFlags(u16 flags);
        SetPlayerAvatarTransitionFlags(arg[0] && atoi(arg) == 0 ? PLAYER_AVATAR_FLAG_ON_FOOT : PLAYER_AVATAR_FLAG_MACH_BIKE);
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
    else if (strcmp(cmd, "style") == 0)
    {
        // style SLOT VALUE: the ball style (hex: color bits 0-3, decoration 4-11, special 12-15)
        int slot = atoi(arg);
        const char *sp = strchr(arg, ' ');
        u16 v = sp ? (u16)strtol(sp + 1, NULL, 16) : 0;
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            SetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR, &v);
            printf("[test] style: slot %d = %04x\n", slot, v);
        }
    }
    else if (strcmp(cmd, "team") == 0)
    {
        // team SLOT: toggle a party slot on the co-op team (like CO-OP TEAM in the party menu)
        int slot = arg ? atoi(arg) : 0;
        u8 r = MP_Team_Toggle(slot);
        printf("[test] team: slot %d -> %d (count %d)\n", slot, r, MP_Team_Count());
    }
    else if (strcmp(cmd, "hidesprites") == 0)
    {
        // hidesprites: every sprite invisible (a clean background for screenshots / mock-ups)
        int i;
        for (i = 0; i < MAX_SPRITES; i++)
            if (gSprites[i].inUse)
                gSprites[i].invisible = TRUE;
    }
    else if (strcmp(cmd, "where") == 0)
    {
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        printf("[test] where map %d.%d xy %d,%d\n", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
               me->currentCoords.x - 7, me->currentCoords.y - 7);
    }
    else if (strcmp(cmd, "friend") == 0)
    {
        // friend N: CO-OP > FRIENDS > player N (1-4)
        void MP_Linkx_OpenFriend(u8 id);
        if (atoi(arg) >= 1 && atoi(arg) <= MP_MAX_PLAYERS)
            MP_Linkx_OpenFriend(atoi(arg) - 1);
    }
    else if (strcmp(cmd, "trade") == 0)
    {
        void MP_Trade_OpenMenu(void);
        MP_Trade_OpenMenu();
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
    else if (strcmp(cmd, "badegg") == 0)
    {
        // badegg SLOT - break a party Pokemon the way co-op catches broke (OT ID overwritten)
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            gPlayerParty[slot].box.otId ^= 0x00010203;
            printf("[test] badegg: slot %d species now %d\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES));
        }
    }
    else if (strcmp(cmd, "giveforeign") == 0)
    {
        // giveforeign SPECIES - a Pokemon made in another player's game (another OT ID), given
        // the way a co-op catch is (GiveMonToPlayer)
        static struct Pokemon mon;
        int species = atoi(arg);
        CreateMon(&mon, species > 0 ? species : 288, 5, USE_RANDOM_IVS, FALSE, 0, OT_ID_PRESET, 0x9ABC1234);
        GiveMonToPlayer(&mon);
        printf("[test] giveforeign: given, bad egg %d\n", (int)MP_IsBoxMonBadEgg(&gPlayerParty[CalculatePlayerPartyCount() - 1].box));
    }
    else if (strcmp(cmd, "heap") == 0)
    {
        void MP_HeapStats(u32 *used, u32 *freeTotal, u32 *largestFree);
        u32 used, freeTotal, largest;
        MP_HeapStats(&used, &freeTotal, &largest);
        printf("[test] heap: used %u, free %u, largest free %u\n", (unsigned)used, (unsigned)freeTotal, (unsigned)largest);
    }
    else if (strcmp(cmd, "ballfx") == 0)
    {
        // mp ballfx SLOT FORM PART COLOR RECALL SOUND  |  mp ballfx SLOT preset N
        int slot = 0, f = 0, pa = 0, c = 0, r = 0, so = 0;
        struct MpBallFx fx;
        if (sscanf(arg, "%d preset %d", &slot, &f) == 2)
            MP_BallFx_Preset(f, &fx);
        else
        {
            sscanf(arg, "%d %d %d %d %d %d", &slot, &f, &pa, &c, &r, &so);
            fx.form = f; fx.part = pa; fx.color = c; fx.recall = r; fx.sound = so;
        }
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            MP_BallFx_SetBoxMon(&gPlayerParty[slot].box, &fx);
            MP_BallFx_OfMon(&gPlayerParty[slot], &fx);
            printf("[test] ballfx: slot %d form %d part %d color %d recall %d sound %d\n", slot, fx.form, fx.part, fx.color, fx.recall, fx.sound);
        }
    }
    else if (strcmp(cmd, "settrend") == 0)
    {
        // settrend SLOT W1 W2 TRENDINESS - put a phrase straight into a saved trend slot (hex words)
        unsigned slot = 0, w1 = 0, w2 = 0, t = 50;
        if (sscanf(arg, "%u %x %x %u", &slot, &w1, &w2, &t) >= 3 && slot < SAVED_TRENDS_COUNT)
        {
            gSaveBlock1Ptr->dewfordTrends[slot].words[0] = w1;
            gSaveBlock1Ptr->dewfordTrends[slot].words[1] = w2;
            gSaveBlock1Ptr->dewfordTrends[slot].trendiness = t;
            gSaveBlock1Ptr->dewfordTrends[slot].maxTrendiness = t > 127 ? 127 : (t < 1 ? 1 : t);
        }
    }
    else if (strcmp(cmd, "trends") == 0)
    {
        int i;
        for (i = 0; i < SAVED_TRENDS_COUNT; i++)
        {
            char ascii[64];
            u8 buf[64];
            ConvertEasyChatWordsToString(buf, gSaveBlock1Ptr->dewfordTrends[i].words, 2, 1);
            MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
            printf("[test] trends[%d]: %04x %04x \"%s\" t %d max %d gain %d rand %04x\n", i, gSaveBlock1Ptr->dewfordTrends[i].words[0],
                   gSaveBlock1Ptr->dewfordTrends[i].words[1], ascii, gSaveBlock1Ptr->dewfordTrends[i].trendiness,
                   gSaveBlock1Ptr->dewfordTrends[i].maxTrendiness, gSaveBlock1Ptr->dewfordTrends[i].gainingTrendiness,
                   gSaveBlock1Ptr->dewfordTrends[i].rand);
        }
    }
    else if (strcmp(cmd, "easychat") == 0)
    {
        // easychat TYPE PERSON - the word screen as an NPC would open it (5 interview, 0/1 reporter, 2 boy)
        static u16 sWords[9] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
        int type = 5, person = 0;
        sscanf(arg, "%d %d", &type, &person);
        DoEasyChatScreen(type, sWords, CB2_ReturnToField, person);
    }
    else if (strcmp(cmd, "trend") == 0)
    {
        // trend - Dewford's current trendy phrase
        char ascii[64];
        u8 buf[64];
        ConvertEasyChatWordsToString(buf, gSaveBlock1Ptr->dewfordTrends[0].words, 2, 1);
        MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
        printf("[test] trend: %04x %04x \"%s\" (trendiness %d)\n", gSaveBlock1Ptr->dewfordTrends[0].words[0],
               gSaveBlock1Ptr->dewfordTrends[0].words[1], ascii, gSaveBlock1Ptr->dewfordTrends[0].trendiness);
    }
    else if (strcmp(cmd, "warp") == 0)
    {
        // warp GROUP,NUM,X,Y - a walk-in warp to that map spot (on the field)
        int g = 0, n = 0, x = 0, y = 0;
        if (sscanf(arg, "%d,%d,%d,%d", &g, &n, &x, &y) == 4)
        {
            SetWarpDestination(g, n, WARP_ID_NONE, x, y);
            DoWarp();
            printf("[test] warp: %d,%d %d,%d\n", g, n, x, y);
        }
    }
    else if (strcmp(cmd, "ballstyle") == 0)
    {
        // ballstyle SLOT HEX - a party Moemon's POKé BALL style (c000 GENESIS BALL, d000 EPOCH BALL)
        int slot = 0;
        unsigned style = 0;
        if (sscanf(arg, "%d %x", &slot, &style) == 2 && slot >= 0 && slot < PARTY_SIZE)
        {
            u32 v = style;
            SetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR, &v);
            printf("[test] ballstyle: slot %d style %04x\n", slot, (int)GetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR));
        }
    }
    else if (strcmp(cmd, "fxedit") == 0)
    {
        // fxedit SLOT - the ball effect editor for a party Moemon (back to the field after)
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
            MP_BallFxEditor_Open(&gPlayerParty[slot].box, CB2_ReturnToField);
    }
    else if (strcmp(cmd, "budget") == 0)
    {
        void MP_SpriteBudget(u8 *freeSprites, u8 *freePalettes, u16 *freeTiles, u8 *freeMatrices);
        u8 sp, pal, mat;
        u16 tiles;
        MP_SpriteBudget(&sp, &pal, &tiles, &mat);
        printf("[test] sprites: free %d sprites, %d palettes, %d tiles, %d matrices\n", sp, pal, tiles, mat);
    }
    else if (strcmp(cmd, "compensate") == 0)
    {
        MP_CompensateBadEggs();
    }
    else if (strcmp(cmd, "party") == 0)
    {
        int i;
        printf("[test] party:");
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES))
                printf(" %d(%d/%d ball %d style %04x fx %04x/%d friend %d%s)", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_HP), (int)GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_POKEBALL), (int)GetMonData(&gPlayerParty[i], MON_DATA_ENCRYPT_SEPARATOR),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_MP_BALLFX), (int)GetMonData(&gPlayerParty[i], MON_DATA_MP_BALLFX_SOUND),
                       (int)GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP),
                       GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_BAD_EGG) ? " BAD EGG" : "");
            else
                printf(" 0");
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
            else if (strcmp(what, "wild") == 0) MP_Events_OnWildEnd(B_OUTCOME_WON, &gEnemyParty[0]);
            else if (strcmp(what, "catch") == 0) MP_Events_OnWildEnd(B_OUTCOME_CAUGHT, &gPlayerParty[0]);
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
        {
            void MP_Follower_DebugDump(void);
            MP_Follower_DebugDump();
        }
    }
    else if (strcmp(cmd, "drop") == 0)
    {
        MP_OnFileDropped(arg);
    }
    fflush(stdout);
}
