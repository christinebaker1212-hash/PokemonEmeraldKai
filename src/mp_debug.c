// Developer quick-start used by the automated multiplayer tests.
// Only active when MOEMON_DEBUG_QUICKSTART is set in the environment.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "data.h"
#include "platform.h"
#include "siirtc.h"
#include "rtc.h"
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
#include "mp_farm.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "field_effect.h"
#include "fieldmap.h"
#include "constants/field_effects.h"
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
#include "sound.h"
#include "platform/gamepad.h"
#include "money.h"
#include "item.h"
#include "string_util.h"
#include "constants/species.h"
#include "gpu_regs.h"
#include "bg.h"
#include "party_menu.h"
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
    const char *partyEnv = getenv("MOEMON_DEBUG_PARTY"); // e.g. "258:20,280:18+" (+ = about to level up, * = shiny)
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
            if (strchr(tok, '*'))   // "280:15*" = shiny (a personality that's shiny with the player's Trainer ID)
            {
                u32 otId = T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
                CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, TRUE, ((otId >> 16) ^ (otId & 0xFFFF)) << 16,
                          OT_ID_PLAYER_ID, 0);
            }
            else if (getenv("MOEMON_DEBUG_PID_SALT"))   // tests with two players: Pokémon of their own (MOéMON GARDEN keys)
                CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, TRUE,
                          (u32)strtoul(getenv("MOEMON_DEBUG_PID_SALT"), NULL, 0) * 2654435761u + slot, OT_ID_PLAYER_ID, 0);
            else
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
    if (strcmp(name, "patchlist") == 0)   // the CO-OP setup screen's PATCH NOTES list
        return MP_Setup_ScreenIs(5);
    if (strcmp(name, "patchtext") == 0)   // ... and one version line's notes
        return MP_Setup_ScreenIs(6);
    if (strcmp(name, "easychat") == 0)   // the easy chat word screen is up
    {
        bool8 MP_EasyChatOpen(void);
        return MP_EasyChatOpen();
    }
    if (strcmp(name, "yesno") == 0)   // a script's YES / NO box is up, or a menu's (menu_helpers.c)
    {
        bool8 MP_ScriptYesNoOpen(void);
        bool8 MP_MenuYesNoOpen(void);
        return MP_ScriptYesNoOpen() || MP_MenuYesNoOpen();
    }
    if (strcmp(name, "battle") == 0)
        return gMain.inBattle;
    if (strcmp(name, "surfing") == 0)   // surfing and free to move
        return (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING) && MP_IsLocalFree();
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
    if (strcmp(name, "summary") == 0)   // the summary screen is up (e.g. picking a move to forget)
    {
        bool8 MP_SummaryScreen_IsOpen(void);
        return MP_SummaryScreen_IsOpen();
    }
    if (strcmp(name, "nofade") == 0)
        return !gPaletteFade.active;
    if (strcmp(name, "action") == 0)
    {
        bool8 MP_Test_IsChoosingAction(void);
        return gMain.inBattle && MP_Test_IsChoosingAction();
    }
    if (strcmp(name, "joinprompt") == 0)   // "X wants to join your battle! Is this okay?" with its YES / NO up
    {
        bool8 MP_Join_PromptUp(void);
        return gMain.inBattle && MP_Join_PromptUp();
    }
    if (strncmp(name, "friendbattle", 12) == 0)   // friendbattle[N]: another player (N others) in a battle
    {
        int i, n = 0, want = name[12] ? atoi(name + 12) : 1;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i != gMpLocalId && gMpPlayers[i].present && (gMpPlayers[i].state.flags & MPF_IN_BATTLE))
                n++;
        return n >= want;
    }
    if (strncmp(name, "joinpending", 11) == 0)   // joinpendingN: N friends asked to join our battle (next turn's prompt)
    {
        u8 MP_Join_PendingCount(void);
        return MP_Join_PendingCount() >= atoi(name + 11);
    }
    if (strncmp(name, "slot:", 5) == 0)    // slot:BOX:POS:SPECIES - the shared PC's slot (as we see it) has it (0: empty)
    {
        struct PokemonStorage *MP_SharedBox_Mirror(void);
        int box = 1, pos = 0, species = 0;
        struct BoxPokemon *b;
        sscanf(name + 5, "%d:%d:%d", &box, &pos, &species);
        if (box < 1 || box > TOTAL_BOXES_COUNT || pos < 0 || pos >= IN_BOX_COUNT)
            return FALSE;
        b = &MP_SharedBox_Mirror()->boxes[box - 1][pos];
        if (!GetBoxMonData(b, MON_DATA_SANITY_HAS_SPECIES))
            return species == 0;
        return (int)GetBoxMonData(b, MON_DATA_SPECIES) == species;
    }
    if (strncmp(name, "boxname:", 8) == 0) // boxname:BOX:NAME - the shared PC's box (as we see it) is called NAME
    {
        struct PokemonStorage *MP_SharedBox_Mirror(void);
        void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
        int box = atoi(name + 8);
        const char *want = strchr(name + 8, ':');
        char have[16];
        if (box < 1 || box > TOTAL_BOXES_COUNT || want == NULL)
            return FALSE;
        MP_GameToAscii(have, MP_SharedBox_Mirror()->boxNames[box - 1], BOX_NAME_LENGTH);
        return strcmp(have, want + 1) == 0;
    }
    if (strcmp(name, "sharedsteady") == 0) // the shared PC's box screen, nothing waiting on the server
        return MP_SharedBox_Loaded() && !MP_SharedBox_Busy() && MP_PcStorage_Steady();
    if (strcmp(name, "sharedclosed") == 0) // the shared PC's Pokémon all sent and done with
        return !MP_SharedBox_IsOpen();
    if (strcmp(name, "pcstorage") == 0)    // a PC box screen
        return MP_PcStorage_IsOpen();
    if (strcmp(name, "jaridle") == 0)      // the CANDY JAR's menu, done pouring or converting
        return MP_Candy_JarOpen() && !MP_Candy_JarBusy();
    if (strcmp(name, "jar") == 0)          // the CANDY JAR's menu (it may still be pouring)
        return MP_Candy_JarOpen();
    if (strcmp(name, "watchmenu") == 0)    // SPECTATE / BET / JOIN / CANCEL in front of a battling friend
    {
        bool8 MP_Spectate_ChoiceUp(void);
        return MP_Spectate_ChoiceUp();
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
    else if (strcmp(cmd, "sumstate") == 0)   // the summary screen's page and input mode (mp summary opens it)
    {
        void MP_Summary_Debug(void);
        MP_Summary_Debug();
    }
    else if (strcmp(cmd, "pmd") == 0)   // mp pmd [SLOT]: a follower's PMD Collab picture (mp_pmd.c)
    {
        void MP_Follower_PmdDebug(u8 slot);
        MP_Follower_PmdDebug(arg[0] ? atoi(arg) : 0);
    }
    else if (strcmp(cmd, "landwild") == 0)
    {
        // landwild - a wild battle with a Pokémon from this map's grass table, as an encounter makes it (its level
        // through ChooseWildMonLevel: level scaling, mp_levelscale.c)
        bool8 MP_RollExtraLandMon(struct Pokemon *out);   // wild_encounter.c
        struct Pokemon m;
        if (MP_RollExtraLandMon(&m))
        {
            ZeroEnemyPartyMons();
            gEnemyParty[0] = m;
            printf("[test] landwild: %d Lv%d\n", (int)GetMonData(&m, MON_DATA_SPECIES), (int)GetMonData(&m, MON_DATA_LEVEL));
            BattleSetup_StartWildBattle();
        }
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
    else if (strcmp(cmd, "double") == 0)
    {
        // double S:L S:L - a local wild Double Battle (you command party slots 0-1)
        void BattleSetup_StartLocalDoubleWildBattle(void);
        int sp[2] = {288, 286}, lv[2] = {5, 5};
        sscanf(arg, "%d:%d %d:%d", &sp[0], &lv[0], &sp[1], &lv[1]);
        ZeroEnemyPartyMons();
        CreateMon(&gEnemyParty[0], sp[0], lv[0], 32, 0, 0, OT_ID_PLAYER_ID, 0);
        CreateMon(&gEnemyParty[1], sp[1], lv[1], 32, 0, 0, OT_ID_PLAYER_ID, 0);
        BattleSetup_StartLocalDoubleWildBattle();
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
    else if (strcmp(cmd, "farm") == 0)
    {
        // farm state|plant PLOT BERRY [STAGE]|grow MINUTES|ripe PLOT: the MOéMON GARDEN's BERRY farm (mp_farm.c)
        MP_Farm_Debug(arg);
    }
    else if (strcmp(cmd, "life") == 0)
    {
        // life show|remember|minutes|trait|count|save|load|reveal|journal ... (mp_life.c)
        MP_Life_Debug(arg);
    }
    else if (strcmp(cmd, "dexcheck") == 0)
    {
        // dexcheck [NATIONAL_NUMBER]: every species' long Pokedex text built and checked (pokedex.c)
        MP_Dex_DebugTexts((u16)atoi(arg));
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
    else if (strcmp(cmd, "berrytree") == 0)
    {
        // berrytree TREE BERRY STAGE - plant BERRY (berry number: 47+ are the e-Reader Berries) in tree TREE
        void PlantBerryTree(u8 id, u8 berry, u8 stage, bool8 allowGrowth);
        int tree = 0, berry = 1, stage = 5;
        sscanf(arg, "%d %d %d", &tree, &berry, &stage);
        PlantBerryTree(tree, berry, stage, TRUE);
        printf("[test] berrytree %d = berry %d stage %d\n", tree, berry, stage);
    }
    else if (strcmp(cmd, "berryinfo") == 0)
    {
        // berryinfo TREE - a tree's berry, stage and minutes to the next stage
        struct BerryTree *GetBerryTreeInfo(u8 id);
        int tree = 0;
        struct BerryTree *t;
        sscanf(arg, "%d", &tree);
        t = GetBerryTreeInfo(tree);
        printf("[test] berryinfo %d: berry %d stage %d next %d min yield %d regrow %d (time events %s, wall clock %s, last %d.%02d:%02d, days var %d)\n",
               tree, t->berry, t->stage, t->minutesUntilNextStage, t->berryYield, t->regrowthCount,
               FlagGet(FLAG_SYS_CLOCK_SET) ? "on" : "off", FlagGet(FLAG_SET_WALL_CLOCK) ? "set" : "not set",
               gSaveBlock2Ptr->lastBerryTreeUpdate.days, gSaveBlock2Ptr->lastBerryTreeUpdate.hours,
               gSaveBlock2Ptr->lastBerryTreeUpdate.minutes, VarGet(VAR_DAYS));
    }
    else if (strcmp(cmd, "panel") == 0)
    {
        // panel ON SIZE: the SPECTATE panel (friends' screens under ours), SIZE 1-4
        int on = 1, size = 1;
        sscanf(arg, "%d %d", &on, &size);
        gMpConfig.spectatePanel = on != 0;
        gMpConfig.spectateScale = size < 1 ? 1 : (size > 4 ? 4 : size);
        {
            int i;   // friends with their own settings take the size too
            for (i = 0; i < MP_SPEC_FRIENDS; i++)
                if (gMpConfig.specFriends[i].name[0])
                    gMpConfig.specFriends[i].size = gMpConfig.spectateScale;
        }
        printf("[test] spectate panel %s, size %dx\n", on ? "on" : "off", gMpConfig.spectateScale);
    }
    else if (strcmp(cmd, "optionstab") == 0)
    {
        // optionstab N: OPTIONS (open) shows tab N (0 GAME, 1 DISPLAY, 2 CONTROLS, 3 CO-OP)
        void MP_Options_DebugTab(u8 tab);
        MP_Options_DebugTab(atoi(arg));
    }
    else if (strcmp(cmd, "panelfriend") == 0)
    {
        // panelfriend PLAYER SIDE SIZE POPOUT SHOW: one friend's screen (PLAYER 1-4, SIDE 0 below 1 above 2 left 3 right)
        int pl = 0, side = 0, size = 1, pop = 0, show = 1, i, n;
        char name[MP_NAME_LEN + 1];
        sscanf(arg, "%d %d %d %d %d", &pl, &side, &size, &pop, &show);
        if (pl >= 1 && pl <= MP_MAX_PLAYERS && gMpPlayers[pl - 1].present)
        {
            MP_GameToAscii(name, gMpPlayers[pl - 1].name, MP_NAME_LEN);
            name[MP_NAME_LEN] = 0;
            for (i = 0; i < MP_SPEC_FRIENDS && gMpConfig.specFriends[i].name[0] && strcmp(gMpConfig.specFriends[i].name, name); i++)
                ;
            if (i == MP_SPEC_FRIENDS)
                i = MP_SPEC_FRIENDS - 1;
            n = i;
            snprintf(gMpConfig.specFriends[n].name, sizeof(gMpConfig.specFriends[n].name), "%s", name);
            gMpConfig.specFriends[n].side = side & 3;
            gMpConfig.specFriends[n].size = size < 1 ? 1 : (size > 4 ? 4 : size);
            gMpConfig.specFriends[n].popout = pop != 0;
            gMpConfig.specFriends[n].show = show != 0;
            printf("[test] panelfriend %s side %d size %d popout %d show %d\n", name, side & 3, size, pop, show);
        }
        else
            printf("[test] panelfriend: no player %d\n", pl);
    }
    else if (strcmp(cmd, "namelines") == 0)
    {
        // namelines: the CO-OP menu's players line, the shared PC's "from NAME" and the POKEDEX's "CAUGHT BY NAME"
        // for every player, each built into a buffer full of junk (0xF7 = "insert a stored string", which crashed
        // GetStringWidth): a name without its end marker shows the junk. "|" marks where the text ends.
        u8 buf[128], i;
        char ascii[160];
        memset(buf, 0xF7, sizeof(buf));
        MP_Menu_BuildPlayersLine(buf);
        MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
        printf("[test] namelines: players %s| (%d)\n", ascii, StringLength(buf));
        for (i = 0; i <= MP_MAX_PLAYERS; i++)
        {
            const u8 *name = i == MP_MAX_PLAYERS ? gSaveBlock2Ptr->playerName : gMpPlayers[i].name;
            if (i < MP_MAX_PLAYERS && (i == gMpLocalId || !gMpPlayers[i].present))
                continue;
            memset(buf, 0xF7, sizeof(buf));
            MP_SharedPc_FromLabel(buf, name);
            MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
            printf("[test] namelines: pc %s| (%d)\n", ascii, StringLength(buf));
            memset(buf, 0xF7, sizeof(buf));
            MP_Dex_CaughtByText(buf, name);
            MP_GameToAscii(ascii, buf, sizeof(ascii) - 1);
            printf("[test] namelines: dex %s| (%d)\n", ascii, StringLength(buf));
        }
    }
    else if (strcmp(cmd, "gatchaday") == 0)
    {
        // gatchaday N - STANDARD shows day N's prizes (-1: today)
        void MP_Gatcha_SetDay(s32 day);
        int n = -1;
        sscanf(arg, "%d", &n);
        MP_Gatcha_SetDay(n);
    }
    else if (strcmp(cmd, "gatchagive") == 0)
    {
        // gatchagive EV [SHINY] - an event Moemon from the MoeGatcha list (EV_* order in mp_gatcha.c)
        void MP_Gatcha_DebugGive(u8 ev, bool8 shiny);
        int ev = 0, sh = 0;
        sscanf(arg, "%d %d", &ev, &sh);
        MP_Gatcha_DebugGive(ev, sh);
    }
    else if (strcmp(cmd, "gatchareel") == 0)
    {
        void MP_Gatcha_DebugReel(void);
        MP_Gatcha_DebugReel();
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
    else if (strcmp(cmd, "cuttree") == 0 || strcmp(cmd, "smashrock") == 0)
    {
        // cuttree LOCALID / smashrock LOCALID - as if we used CUT / ROCK SMASH on it (mp_world.c tells friends)
        extern const u8 EventScript_CutTreeDown[];
        gSpecialVar_LastTalked = atoi(arg);
        if (cmd[0] == 'c')
        {
            ScriptContext_SetupScript(EventScript_CutTreeDown);
        }
        else
        {
            RemoveObjectEventByLocalIdAndMap(gSpecialVar_LastTalked, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            MP_World_RockGone();
        }
    }
    else if (strcmp(cmd, "cutgrass") == 0)
    {
        // cutgrass - CUT on the grass around where we face (SetUpFieldMove_Cut picks the tiles, as the menu does)
        bool8 SetUpFieldMove_Cut(void);
        if (SetUpFieldMove_Cut())
        {
            gFieldCallback2 = NULL;
            gPostMenuFieldCallback = NULL;
            FieldEffectStart(FLDEFF_CUT_GRASS);
        }
        else
        {
            printf("[test] cutgrass: nothing to cut here\n");
        }
    }
    else if (strcmp(cmd, "worldobj") == 0)
    {
        // worldobj LOCALID - is that object on this map there, and where
        u8 objId;
        int localId = atoi(arg);
        if (!TryGetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objId))
            printf("[test] worldobj %d at %d,%d\n", localId, gObjectEvents[objId].currentCoords.x - MAP_OFFSET, gObjectEvents[objId].currentCoords.y - MAP_OFFSET);
        else
            printf("[test] worldobj %d gone\n", localId);
    }
    else if (strcmp(cmd, "ashcount") == 0)
    {
        printf("[test] ash %d\n", VarGet(VAR_ASH_GATHER_COUNT));
    }
    else if (strcmp(cmd, "tileat") == 0)
    {
        // tileat X,Y - the metatile there (map coords without the offset)
        int x = 0, y = 0;
        sscanf(arg, "%d,%d", &x, &y);
        printf("[test] tile %d,%d = %d\n", x, y, MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET));
    }
    else if (strcmp(cmd, "lights") == 0)
    {
        // lights - friends' lights in a dark cave, as last drawn (mp_world.c)
        MP_World_DebugLights();
    }
    else if (strcmp(cmd, "gendersample") == 0)
    {
        // gendersample SPECIES - make 1000 wild-style Pokemon of SPECIES and count the genders
        static struct Pokemon mon;
        int species = atoi(arg), i, f = 0, m = 0, n = 0;
        for (i = 0; i < 1000; i++)
        {
            CreateMon(&mon, species, 5, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
            switch (GetMonGender(&mon))
            {
            case MON_FEMALE: f++; break;
            case MON_MALE: m++; break;
            default: n++; break;
            }
        }
        printf("[test] gendersample %d: female %d male %d none %d (ratio %d)\n", species, f, m, n, gSpeciesInfo[species].genderRatio);
    }
    else if (strcmp(cmd, "challenge") == 0)
    {
        // challenge N - the frontier / tent challenge status (3 = won), e.g. before walking into a tent lobby
        gSaveBlock2Ptr->frontier.challengeStatus = atoi(arg);
        printf("[test] challenge status %d\n", gSaveBlock2Ptr->frontier.challengeStatus);
    }
    else if (strcmp(cmd, "setflag") == 0)
    {
        // setflag N - an event flag (include/constants/flags.h)
        FlagSet(atoi(arg));
    }
    else if (strcmp(cmd, "clearflag") == 0)
    {
        // clearflag N - an event flag off (2144 = FLAG_SYS_POKEMON_GET)
        FlagClear(atoi(arg));
        printf("[test] clearflag %d\n", atoi(arg));
    }
    else if (strcmp(cmd, "ticketbackfill") == 0)
    {
        // ticketbackfill [reset] - the once-per-save MOEGATCHA TICKETS for story items already got (reset: as if never run)
        if (strcmp(arg, "reset") == 0)
            MP_SaveData_Set("gatcha:ticketbackfill", 0);
        MP_Gatcha_BackfillTickets();
    }
    else if (strcmp(cmd, "gatchafx") == 0)
    {
        // gatchafx N - the next MoeGatcha pull gives ball effect prize number N (-1: a random one)
        void MP_Gatcha_RigFx(s16 n);
        MP_Gatcha_RigFx(atoi(arg));
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
            printf("[test] spr %2d x %d y %d x2 %d y2 %d shape %d size %d tile %d pal %d tag %04x ptag %04x inv %d prio %d sub %d cb %p\n", i, sp->x, sp->y, sp->x2, sp->y2,
                   sp->oam.shape, sp->oam.size, sp->oam.tileNum, sp->oam.paletteNum, sp->template ? sp->template->tileTag : 0,
                   sp->template ? sp->template->paletteTag : 0, sp->invisible, sp->oam.priority, sp->subpriority, (void *)sp->callback);
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
        for (b = 0; b < gBattlersCount; b++)
            printf("[test] battler %d hp %d/%d species %d level %d (party %d)\n", b, gBattleMons[b].hp, gBattleMons[b].maxHP,
                   gBattleMons[b].species, gBattleMons[b].level,
                   GetMonData(GetBattlerSide(b) == B_SIDE_PLAYER ? &gPlayerParty[gBattlerPartyIndexes[b]] : &gEnemyParty[gBattlerPartyIndexes[b]], MON_DATA_LEVEL));
    }
    else if (strcmp(cmd, "bstatus") == 0)
    {
        // bstatus BATTLER STATUS1 [HP] - sets a battler's status (and HP) and redraws its healthbox / card
        int b = 0, st = 0, hp = -1;
        sscanf(arg, "%d %d %d", &b, &st, &hp);
        if (b >= 0 && b < gBattlersCount)
        {
            struct Pokemon *mon = GetBattlerSide(b) == B_SIDE_PLAYER ? &gPlayerParty[gBattlerPartyIndexes[b]] : &gEnemyParty[gBattlerPartyIndexes[b]];
            u32 status = st;
            SetMonData(mon, MON_DATA_STATUS, &status);
            gBattleMons[b].status1 = status;
            if (hp >= 0)
            {
                u16 h = hp;
                SetMonData(mon, MON_DATA_HP, &h);
                gBattleMons[b].hp = h;
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[b], mon, HEALTHBOX_ALL);
        }
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
    else if (strcmp(cmd, "moves") == 0)
    {
        // mp moves SLOT M1 [M2 M3 M4] - the party Moemon in SLOT knows these moves (full PP); missing ones are emptied
        int slot = 0, m[MAX_MON_MOVES] = {0}, n, i;
        n = sscanf(arg, "%d %d %d %d %d", &slot, &m[0], &m[1], &m[2], &m[3]);
        if (n >= 2 && slot >= 0 && slot < PARTY_SIZE)
        {
            for (i = 0; i < MAX_MON_MOVES; i++)
                SetMonMoveSlot(&gPlayerParty[slot], m[i], i);
            printf("[test] moves: slot %d = %d %d %d %d\n", slot, m[0], m[1], m[2], m[3]);
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
    else if (strcmp(cmd, "bagorder") == 0)
    {
        // bagorder POCKET - the item numbers in a pocket's order (a * after a new one)
        int p = 0;
        u16 i;
        sscanf(arg, "%d", &p);
        if (p >= 0 && p < POCKETS_COUNT)
        {
            printf("[test] bagorder %d:", p);
            for (i = 0; i < gBagPockets[p].capacity && gBagPockets[p].itemSlots[i].itemId != ITEM_NONE; i++)
                printf(" %d%s", gBagPockets[p].itemSlots[i].itemId, MP_Bag_IsNew(gBagPockets[p].itemSlots[i].itemId) ? "*" : "");
            printf("\n");
        }
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
    else if (strcmp(cmd, "created") == 0)
    {
        // created HEX - CUSTOM 1 = that recipe (the ini's custom1 = HEX) and wear it
        if (MP_Created_FromHex(arg, &gMpConfig.custom[0]) && MP_Avatar_ParseSpriteName("custom_1", &gMpConfig.app))
        {
            gMpConfig.app.recipe = gMpConfig.custom[0];
            gMpConfig.app.hasCustom = FALSE;
            gMpConfig.customSprite[0] = 0;
            MP_Avatar_ApplyLocal();
            MP_SendAppearance();
            printf("[test] created %s\n", arg);
        }
        else
            printf("[test] created %s: bad recipe\n", arg);
    }
    else if (strcmp(cmd, "remoteapp") == 0)
    {
        // remoteapp - each friend's look as we received it (base, CREATE recipe, LOOK > BIKE)
        int i;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i != gMpLocalId && gMpPlayers[i].present)
            {
                char hex[MP_CREATED_BYTES * 2 + 1];
                MP_Created_ToHex(&gMpPlayers[i].app.recipe, hex);
                printf("[test] remoteapp %d: base %d recipe %s bike %d\n", i + 1, gMpPlayers[i].app.base, hex,
                       gMpPlayers[i].app.bike);
            }
    }
    else if (strcmp(cmd, "sprite") == 0)
    {
        // sprite NAME - wear a look by its ini name (hiker, frlg_lass, custom_1, rs_may...)
        if (MP_Avatar_ParseSpriteName(arg, &gMpConfig.app))
        {
            if (gMpConfig.app.base == MP_BASE_CREATED)
                gMpConfig.app.recipe = gMpConfig.custom[gMpConfig.app.created - 1];
            gMpConfig.app.hasCustom = FALSE;
            gMpConfig.customSprite[0] = 0;
            MP_Avatar_ApplyLocal();
            MP_SendAppearance();
            printf("[test] sprite %s\n", arg);
        }
        else
            printf("[test] sprite %s: no such look\n", arg);
    }
    else if (strcmp(cmd, "actiondump") == 0)
    {
        // actiondump FILE - the player's look in every avatar state (bike, surf, fishing...)
        void MP_Avatar_DumpActions(const char *path);
        MP_Avatar_DumpActions(arg);
    }
    else if (strcmp(cmd, "bike") == 0)
    {
        // bike - get on the Mach Bike (bike 2: the Acro Bike; bike 0: off)
        extern void SetPlayerAvatarTransitionFlags(u16 flags);
        SetPlayerAvatarTransitionFlags(arg[0] && atoi(arg) == 0 ? PLAYER_AVATAR_FLAG_ON_FOOT
                                     : atoi(arg) == 2 ? PLAYER_AVATAR_FLAG_ACRO_BIKE : PLAYER_AVATAR_FLAG_MACH_BIKE);
    }
    else if (strcmp(cmd, "mountbike") == 0)
    {
        // mountbike N - on (1 Mach, 2 Acro) or off (0) a bike the game's own way (GetOnOffBike: the bike song too)
        void GetOnOffBike(u8 transitionFlags);
        int n = atoi(arg);
        GetOnOffBike(n == 0 ? PLAYER_AVATAR_FLAG_ON_FOOT : n == 2 ? PLAYER_AVATAR_FLAG_ACRO_BIKE : PLAYER_AVATAR_FLAG_MACH_BIKE);
    }
    else if (strcmp(cmd, "iconfamily") == 0)
    {
        // iconfamily - the button icons drawn now (0 keyboard, 1 Nintendo, 2 PlayStation, 3 Xbox) and the setting
        printf("[test] iconfamily %d set %d\n", Gamepad_IconFamily(), Gamepad_GetIconSet());
    }
    else if (strcmp(cmd, "matchcalltest") == 0)
    {
        void MP_MatchCall_DebugSelect(int n);
        MP_MatchCall_DebugSelect(arg[0] ? atoi(arg) : 5);
    }
    else if (strcmp(cmd, "ridemusic") == 0)
    {
        // ridemusic BIKE SURF - OPTIONS > GAME > BIKE MUSIC / SURF MUSIC (1 on, 0 off)
        int b = 1, sf = 1;
        sscanf(arg, "%d %d", &b, &sf);
        gMpConfig.bikeMusic = b != 0;
        gMpConfig.surfMusic = sf != 0;
        printf("[test] ridemusic bike %d surf %d\n", gMpConfig.bikeMusic, gMpConfig.surfMusic);
    }
    else if (strcmp(cmd, "music") == 0)
    {
        // music - the song playing now (and the saved bike song); music play: the map's song again, as on arrival
        if (strcmp(arg, "play") == 0)
            Overworld_PlaySpecialMapMusic();
        printf("[test] music %u saved %u\n", (unsigned)GetCurrentMapMusic(), (unsigned)gSaveBlock1Ptr->savedMusic);
    }
    else if (strcmp(cmd, "bikemode") == 0)
    {
        // bikemode N - LOOK > BIKE for a character who rides a Pokemon or a vehicle: 0 RIDE, 1 BIKE
        gMpConfig.app.bike = atoi(arg) ? 1 : 0;
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        printf("[test] bikemode %d (rides: %d)\n", gMpConfig.app.bike, MP_Avatar_HasRide(&gMpConfig.app));
    }
    else if (strcmp(cmd, "underwater") == 0)
    {
        // underwater - the diving sprite where you stand (looks only: nothing else about diving)
        extern void SetPlayerAvatarTransitionFlags(u16 flags);
        SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_UNDERWATER);
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
    else if (strcmp(cmd, "achmenu") == 0)
    {
        // achmenu TAB - CO-OP > DAILY > ACHIEVEMENTS on a tab (0 JOURNEY ... 5 EXTRAS)
        void MP_Daily_OpenAchievements(u8 tab);
        MP_Daily_OpenAchievements(atoi(arg));
    }
    else if (strcmp(cmd, "ach") == 0)
    {
        // ach - achievements done, in all and per tab
        s32 MP_Gatcha_Pulls(void);
        u8 c;
        printf("[test] achievements %d/%d:", MP_Achieve_DoneCount(), MP_Achieve_Total());
        for (c = 0; c < MP_Achieve_CatCount(); c++)
        {
            u16 done, n = MP_Achieve_CountIn(c, &done);
            printf(" %s %d/%d", MP_Achieve_CatName(c), done, n);
        }
        printf(" pulls %d\n", (int)MP_Gatcha_Pulls());
    }
    else if (strcmp(cmd, "fxunlock") == 0)
    {
        // fxunlock all | fxunlock KIND INDEX - MOEGATCHA ball pieces (0 color, 1 form, 2 particles, 3 recall)
        MP_BallUnlock_Debug(arg);
    }
    else if (strcmp(cmd, "lookskin") == 0)
    {
        // lookskin N - LOOK's skin color (0 = the sprite's own)
        gMpConfig.app.skin = atoi(arg);
    }
    else if (strcmp(cmd, "gamestat") == 0)
    {
        // gamestat N VALUE - a game stat (include/constants/game_stat.h)
        int n = 0, v = 0;
        sscanf(arg, "%d %d", &n, &v);
        SetGameStat(n, v);
    }
    else if (strcmp(cmd, "money") == 0)
    {
        // money [N] - sets the money, or without N prints it
        if (arg[0])
            SetMoney(&gSaveBlock1Ptr->money, atoi(arg));
        else
            printf("[test] money %u\n", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
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
    else if (strcmp(cmd, "stuckclock") == 0)
    {
        // stuckclock - make this save look like one from when the clock was stuck at day 0
        memset(&gSaveBlock2Ptr->lastBerryTreeUpdate, 0, sizeof(gSaveBlock2Ptr->lastBerryTreeUpdate));
        memset(&gSaveBlock2Ptr->localTimeOffset, 0, sizeof(gSaveBlock2Ptr->localTimeOffset));
        gSaveBlock2Ptr->localTimeOffset.hours = -10;   // the bedroom clock set to 10:00 against day 0
        FlagSet(FLAG_SYS_CLOCK_SET);
        FlagSet(FLAG_SET_WALL_CLOCK);
        VarSet(VAR_DAYS, 0);
        printf("[test] stuck clock save\n");
    }
    else if (strcmp(cmd, "berrywait") == 0)
    {
        // berrywait MIN - as if MIN minutes had passed for the Berry trees (then the time-based events run)
        void DoTimeBasedEvents(void);
        int min = 0;
        s32 total;
        struct Time *t = &gSaveBlock2Ptr->lastBerryTreeUpdate;
        sscanf(arg, "%d", &min);
        total = t->days * 1440 + t->hours * 60 + t->minutes - min;
        t->days = total / 1440;
        t->hours = (total / 60) % 24;
        t->minutes = total % 60;
        DoTimeBasedEvents();
        printf("[test] berrywait %d\n", min);
    }
    else if (strcmp(cmd, "gjoin") == 0)
    {
        // gjoin - join the test's room again (MOEMON_COOP_AUTO=join:ROOM) after goffline
        char buf[96], *room = NULL;
        const char *env = getenv("MOEMON_COOP_AUTO");
        if (env)
        {
            snprintf(buf, sizeof(buf), "%s", env);
            strtok(buf, ":");
            room = strtok(NULL, ":");
        }
        if (room)
        {
            MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
            MP_Lobby_JoinRoom(room, "");
            printf("[test] joining %s\n", room);
        }
    }
    else if (strcmp(cmd, "goffline") == 0)
    {
        // goffline - leave the room and the server (back to playing alone)
        MP_Lobby_Disconnect();
        printf("[test] offline\n");
    }
    else if (strcmp(cmd, "clock") == 0)
    {
        // clock - the game clock (online: the host's PC clock)
        struct SiiRtcInfo r;
        s64 off = 0;
        Platform_GetDateTime(&r);
        printf("[test] clock 20%02x-%02x-%02x %02x:%02x (%s)\n", r.year, r.month, r.day, r.hour, r.minute,
               MP_HostClockOffset(&off) ? "the host's" : "ours");
        {
            RtcCalcLocalTime();
            printf("[test] game time day %d %02d:%02d (rtc status %04x, offset %d.%02d:%02d)\n", gLocalTime.days, gLocalTime.hours,
                   gLocalTime.minutes, RtcGetErrorStatus(), gSaveBlock2Ptr->localTimeOffset.days,
                   gSaveBlock2Ptr->localTimeOffset.hours, gSaveBlock2Ptr->localTimeOffset.minutes);
        }
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
    else if (strcmp(cmd, "monmoves") == 0)
    {
        // monmoves SLOT - a party Moemon's moves and held item
        int slot = atoi(arg);
        if (slot >= 0 && slot < PARTY_SIZE)
            printf("[test] monmoves: slot %d species %d moves %d %d %d %d held %d\n", slot,
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE1), (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE2),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE3), (int)GetMonData(&gPlayerParty[slot], MON_DATA_MOVE4),
                   (int)GetMonData(&gPlayerParty[slot], MON_DATA_HELD_ITEM));
    }
    else if (strcmp(cmd, "bagqty") == 0)
    {
        // bagqty ITEM - how many the bag has
        int id = atoi(arg), n;
        for (n = 0; n < 999 && CheckBagHasItem(id, n + 1); n++)
            ;
        printf("[test] bagqty %d: %d\n", id, n);
    }
    else if (strcmp(cmd, "pc") == 0)
    {
        // pc - the POKeMON STORAGE SYSTEM, as from a PC
        void ShowPokemonStorageSystemPC(void);
        LockPlayerFieldControls();
        ShowPokemonStorageSystemPC();
    }
    else if (strcmp(cmd, "holdtest") == 0)
    {
        // holdtest - QUICK POWDER and EVIOLITE through the battle's own math on made-up battlers:
        // who moves first (GetWhoStrikesFirst) and the damage a Tackle / Water Gun does (CalculateBaseDamage).
        // In a battle only (the math reads its data); everything it touches is put back.
        u8 GetWhoStrikesFirst(u8 battler1, u8 battler2, bool8 ignoreChosenMoves);
        struct BattlePokemon saveMons[2], atk, def;
        u8 savePos[2] = {gBattlerPositions[0], gBattlerPositions[1]}, saveCount = gBattlersCount;
        u8 saveAtk = gBattlerAttacker, saveTarget = gBattlerTarget, saveCrit = gCritMultiplier;
        u16 savePower = gBattleMovePower;
        u32 saveFlags = gBattleTypeFlags;
        if (!gMain.inBattle || gBattleStruct == NULL)
        {
            printf("[test] holdtest: only in a battle\n");
            return;
        }
        static const struct { u16 species; u16 item; bool8 transformed; } sSpeed[] = {
            {SPECIES_DITTO, ITEM_NONE, FALSE}, {SPECIES_DITTO, ITEM_QUICK_POWDER, FALSE},
            {SPECIES_DITTO, ITEM_QUICK_POWDER, TRUE}, {SPECIES_PIKACHU, ITEM_QUICK_POWDER, FALSE},
        };
        static const u16 sDefenders[] = {SPECIES_BULBASAUR, SPECIES_VENUSAUR, SPECIES_DITTO};
        u32 i, j;
        memcpy(saveMons, gBattleMons, sizeof(saveMons));
        gBattlerPositions[0] = B_POSITION_PLAYER_LEFT;
        gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
        gBattlersCount = 0;   // no abilities on the field
        gBattleTypeFlags = BATTLE_TYPE_LINK;   // no badge boost
        for (i = 0; i < ARRAY_COUNT(sSpeed); i++)
        {
            memset(&gBattleMons[0], 0, sizeof(gBattleMons[0]));
            memset(&gBattleMons[1], 0, sizeof(gBattleMons[1]));
            gBattleMons[0].species = sSpeed[i].species;
            gBattleMons[0].item = sSpeed[i].item;
            gBattleMons[0].speed = 50;
            gBattleMons[0].status2 = sSpeed[i].transformed ? STATUS2_TRANSFORMED : 0;
            gBattleMons[1].species = SPECIES_RATTATA;
            gBattleMons[1].speed = 80;
            for (j = 0; j < NUM_BATTLE_STATS; j++)
                gBattleMons[0].statStages[j] = gBattleMons[1].statStages[j] = DEFAULT_STAT_STAGE;
            printf("[test] holdtest speed species %d item %d transformed %d: %s first\n", sSpeed[i].species, sSpeed[i].item,
                   sSpeed[i].transformed, GetWhoStrikesFirst(0, 1, TRUE) == 0 ? "holder" : "foe");
        }
        for (i = 0; i < ARRAY_COUNT(sDefenders); i++)
        {
            u16 items[2] = {ITEM_NONE, ITEM_EVIOLITE}, moves[2] = {MOVE_TACKLE, MOVE_WATER_GUN};
            s32 dmg[2][2];
            u32 k, m;
            for (k = 0; k < 2; k++)
                for (m = 0; m < 2; m++)
                {
                    memset(&atk, 0, sizeof(atk));
                    memset(&def, 0, sizeof(def));
                    atk.species = SPECIES_RATTATA;
                    atk.level = 50;
                    atk.attack = atk.spAttack = 120;
                    def.species = sDefenders[i];
                    def.item = items[k];
                    def.defense = def.spDefense = 100;
                    for (j = 0; j < NUM_BATTLE_STATS; j++)
                        atk.statStages[j] = def.statStages[j] = DEFAULT_STAT_STAGE;
                    gCritMultiplier = 1;
                    dmg[k][m] = CalculateBaseDamage(&atk, &def, moves[m], 0, 0, 0, 0, 1);
                }
            printf("[test] holdtest eviolite species %d canevolve %d: tackle %d -> %d, water gun %d -> %d\n", sDefenders[i],
                   SpeciesCanEvolve(sDefenders[i]), dmg[0][0], dmg[1][0], dmg[0][1], dmg[1][1]);
        }
        memcpy(gBattleMons, saveMons, sizeof(saveMons));
        gBattlerPositions[0] = savePos[0];
        gBattlerPositions[1] = savePos[1];
        gBattlersCount = saveCount;
        gBattleTypeFlags = saveFlags;
        gBattlerAttacker = saveAtk;
        gBattlerTarget = saveTarget;
        gCritMultiplier = saveCrit;
        gBattleMovePower = savePower;
    }
    else if (strcmp(cmd, "bstatus2") == 0)
    {
        // bstatus2 BATTLER STATUS2 - sets a battler's status2 (7 = confused for 7 turns; -1 = leave it), prints both
        int b = 0, st = -1;
        sscanf(arg, "%d %d", &b, &st);
        if (b >= 0 && b < gBattlersCount)
        {
            if (st >= 0)
                gBattleMons[b].status2 = st;
            printf("[test] bstatus2 %d: status1 %u status2 %u\n", b, (unsigned)gBattleMons[b].status1, (unsigned)gBattleMons[b].status2);
        }
    }
    else if (strcmp(cmd, "pstatus") == 0)
    {
        // pstatus SLOT STATUS1 - a party Moemon's status (8 = poison, 64 = paralysis, 0 = none); prints it back
        int slot = 0, st = 0;
        u32 status;
        sscanf(arg, "%d %d", &slot, &st);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            status = st;
            if (st >= 0)
                SetMonData(&gPlayerParty[slot], MON_DATA_STATUS, &status);
            printf("[test] pstatus %d = %u\n", slot, (unsigned)GetMonData(&gPlayerParty[slot], MON_DATA_STATUS));
        }
    }
    else if (strcmp(cmd, "bgshow") == 0)
    {
        // bgshow BG 0|1 - hides or shows a background layer (debugging)
        int bg = 0, on = 1;
        sscanf(arg, "%d %d", &bg, &on);
        if (on)
            ShowBg(bg);
        else
            HideBg(bg);
    }
    else if (strcmp(cmd, "bgpeek") == 0)
    {
        // bgpeek BG TX TY - a background's tilemap entry at tile (TX, TY) and its first tile row's bytes
        int bg = 0, tx = 0, ty = 0, i;
        u16 cnt, entry;
        const u8 *tiles;
        sscanf(arg, "%d %d %d", &bg, &tx, &ty);
        cnt = GetGpuReg(REG_OFFSET_BG0CNT + bg * 2);
        entry = ((u16 *)(VRAM + ((cnt >> 8) & 0x1F) * 0x800))[(ty & 31) * 32 + (tx & 31)];
        tiles = (const u8 *)(VRAM + ((cnt >> 2) & 3) * 0x4000) + (entry & 0x3FF) * ((cnt & 0x80) ? 64 : 32);
        printf("[test] bgpeek %d %d,%d: cnt %04x entry %04x row0", bg, tx, ty, cnt, entry);
        for (i = 0; i < 8; i++)
            printf(" %02x", tiles[i]);
        printf("\n");
    }
    else if (strcmp(cmd, "gpuregs") == 0)
    {
        // gpuregs - the display registers (DISPCNT, BGnCNT, blending, windows)
        printf("[test] gpuregs dispcnt %04x bg0 %04x bg1 %04x bg2 %04x bg3 %04x bldcnt %04x bldalpha %04x bldy %04x winin %04x winout %04x\n",
               GetGpuReg(REG_OFFSET_DISPCNT), GetGpuReg(REG_OFFSET_BG0CNT), GetGpuReg(REG_OFFSET_BG1CNT), GetGpuReg(REG_OFFSET_BG2CNT),
               GetGpuReg(REG_OFFSET_BG3CNT), GetGpuReg(REG_OFFSET_BLDCNT), GetGpuReg(REG_OFFSET_BLDALPHA), GetGpuReg(REG_OFFSET_BLDY),
               GetGpuReg(REG_OFFSET_WININ), GetGpuReg(REG_OFFSET_WINOUT));
    }
    else if (strcmp(cmd, "pctag") == 0)
    {
        // pctag - the TAG button in the PC's boxes (the west button / F)
        gMpPcTagPressed = TRUE;
    }
    else if (strcmp(cmd, "pccursor") == 0)
    {
        // pccursor - the box screen's hand cursor: area, position, upside down or not
        void MP_PcStorage_LogCursor(void);
        MP_PcStorage_LogCursor();
    }
    else if (strcmp(cmd, "pctags") == 0)
    {
        // pctags - the PC's tags and the group being held
        MP_PcStorage_LogTags();
    }
    else if (strcmp(cmd, "candy") == 0)
    {
        // candy TYPE N - N candies of a type (TYPE -1: every type)
        int type = 0, n = 0, t;
        sscanf(arg, "%d %d", &type, &n);
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
            if ((type == t || type < 0) && t != TYPE_MYSTERY)
                gMpCandies[t] = n;
        printf("[test] candies: %d\n", (int)MP_Candy_Total());
    }
    else if (strcmp(cmd, "mart") == 0)
    {
        // mart ITEM... - a POKé MART selling these (up to 15)
        static u16 sItems[16];
        void CreatePokemartMenu(const u16 *items);
        const char *p = arg;
        int id, used, n = 0;
        while (n < 15 && sscanf(p, "%d%n", &id, &used) == 1)
        {
            sItems[n++] = id;
            p += used;
        }
        sItems[n] = ITEM_NONE;
        CreatePokemartMenu(sItems);
    }
    else if (strcmp(cmd, "tmrefund") == 0)
    {
        // tmrefund [reset]: the TMs the LOTTERY CORNER clerk would bring back (reset: she may come again)
        MP_TmRefund_Debug(arg);
    }
    else if (strcmp(cmd, "candyreload") == 0)
    {
        // candyreload - forget the candies and the BAG, then load them back from the save and its sidecar
        int t;
        MP_Candy_Reset();
        MP_Bag_LoadFromSave();
        printf("[test] candies after reload:");
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
            printf(" %d", gMpCandies[t]);
        printf(" | jar %d\n", CheckBagHasItem(ITEM_CANDY_JAR, 1));
    }
    else if (strcmp(cmd, "itemcounts") == 0)
    {
        // itemcounts ITEM - how many in the BAG, in our own PC, and in the shared PC (as last listed)
        u16 MP_SharedPc_ListedQty(u16 item);
        int item = atoi(arg), i, pc = 0;
        for (i = 0; i < PC_ITEMS_COUNT; i++)
            if (gSaveBlock1Ptr->pcItems[i].itemId == item)
                pc += gSaveBlock1Ptr->pcItems[i].quantity;
        printf("[test] itemcounts %d: bag %d pc %d shared %d\n", item, (int)CountTotalItemQuantityInBag(item), pc,
               (int)MP_SharedPc_ListedQty(item));
    }
    else if (strcmp(cmd, "sharedpc") == 0)
    {
        // sharedpc - CO-OP menu > SHARED PC (MOEMON / ITEMS / CANCEL)
        MP_SharedPc_Open();
    }
    else if (strcmp(cmd, "sharedin") == 0)
    {
        // sharedin BOX POS SPECIES LEVEL - a new Moemon straight into the shared PC (BOX from 1)
        void MP_SharedBox_DebugIn(u8 box, u8 pos, const struct BoxPokemon *b);
        int box = 1, pos = 0, species = 1, level = 5;
        struct BoxPokemon boxMon;
        sscanf(arg, "%d %d %d %d", &box, &pos, &species, &level);
        CreateBoxMon(&boxMon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
        MP_SharedBox_DebugIn(box - 1, pos, &boxMon);
        printf("[test] sharedin %d %d: %d\n", box, pos, species);
    }
    else if (strcmp(cmd, "sharedbox") == 0)
    {
        // sharedbox BOX - the shared PC's box as this game sees it (BOX from 1)
        void MP_SharedBox_DebugDump(u8 box);
        MP_SharedBox_DebugDump(atoi(arg) - 1);
    }
    else if (strcmp(cmd, "huecheck") == 0)
    {
        // huecheck - the Stadium hue math on fixed inputs, for tools/moemon_hues/check_hues.py (compares with the page)
        static const char *const names[][2] = {
            {"ASH", "PIKA"}, {"Red", "Sparky"}, {"LUNA", "Bubbles"}, {"ALICE", "Mr. Fluff"}, {"BOB", "a"},
            {"Ziggy", "ZZZZZZZZZZ"}, {"x", "(Hi):-?!"}, {"May", "Brendan"},
        };
        static const u16 ids[] = {0, 1, 255, 256, 12345, 54321, 65535};
        static const s16 hues[] = {-180, -90, -45, -1, 1, 13, 45, 90, 179, 180};
        u8 ot[16], nick[16];
        u32 i, j, c;
        for (i = 0; i < ARRAY_COUNT(names); i++)
            for (j = 0; j < ARRAY_COUNT(ids); j++)
            {
                MP_AsciiToGame(ot, names[i][0], 7);
                MP_AsciiToGame(nick, names[i][1], 10);
                printf("[hue] nv %s|%s|%u = %u\n", names[i][0], names[i][1], ids[j], MP_Hue_Number(ot, nick, ids[j]));
            }
        for (i = 1; i < NUM_SPECIES; i++)
            for (j = 0; j < 256; j += 15)
            {
                u8 mn, mx;
                MP_Hue_Limits(i, &mn, &mx);
                printf("[hue] deg %u %u %u %u = %d\n", (unsigned)i, (unsigned)j, mn, mx, MP_Hue_Degrees(i, j));
            }
        for (i = 0; i < ARRAY_COUNT(hues); i++)
            for (c = 0; c < 0x8000; c += 97)
            {
                u16 pal[16] = {0};
                pal[1] = c;
                MP_Hue_ShiftPalette(pal, 0, hues[i]);
                printf("[hue] col %d %u = %u\n", hues[i], (unsigned)c, pal[1]);
            }
        printf("[hue] done\n");
    }
    else if (strcmp(cmd, "nick") == 0)
    {
        // nick SLOT NAME - nickname a party Pokémon (SLOT from 1; NAME up to 10 letters, "-" = its species name)
        char *sp = strchr(arg, ' ');
        int slot = atoi(arg) - 1;
        if (sp != NULL && slot >= 0 && slot < PARTY_SIZE)
        {
            u8 name[POKEMON_NAME_LENGTH + 2];
            memset(name, EOS, sizeof(name));
            if (strcmp(sp + 1, "-") == 0)
                StringCopy(name, gSpeciesNames[GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES)]);
            else
                MP_AsciiToGame(name, sp + 1, POKEMON_NAME_LENGTH);
            SetMonData(&gPlayerParty[slot], MON_DATA_NICKNAME, name);
        }
    }
    else if (strcmp(cmd, "huemon") == 0)
    {
        // huemon SLOT - a party Pokémon's Stadium hue
        int slot = atoi(arg) - 1;
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            struct Pokemon *mon = &gPlayerParty[slot];
            u16 species = GetMonData(mon, MON_DATA_SPECIES);
            u8 ot[PLAYER_NAME_LENGTH + 2], nick[POKEMON_NAME_LENGTH + 2], mn, mx;
            char aot[32], anick[32];
            void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
            memset(ot, EOS, sizeof(ot));
            memset(nick, EOS, sizeof(nick));
            GetMonData(mon, MON_DATA_OT_NAME, ot);
            GetMonData(mon, MON_DATA_NICKNAME, nick);
            MP_GameToAscii(aot, ot, PLAYER_NAME_LENGTH);
            MP_GameToAscii(anick, nick, POKEMON_NAME_LENGTH);
            MP_Hue_Limits(species, &mn, &mx);
            printf("[test] huemon %d species %d hue %d\n", slot + 1, species, MP_Hue_OfBoxMon(&mon->box));
            if (StringCompare(nick, gSpeciesNames[species]) != 0)   // nicknamed: the page's answer to compare
                printf("[hue] mon %s|%s|%u %u %u = %d\n", aot, anick, (unsigned)(GetMonData(mon, MON_DATA_OT_ID) & 0xFFFF), mn, mx,
                       MP_Hue_OfBoxMon(&mon->box));
        }
    }
    else if (strcmp(cmd, "sharedname") == 0)
    {
        // sharedname BOX NAME - rename a shared box (as the box screen's NAME does; BOX from 1, NAME up to 8 letters)
        void MP_SharedBox_DebugName(u8 box, const char *name);
        char *sp = strchr(arg, ' ');
        if (sp != NULL)
            MP_SharedBox_DebugName(atoi(arg) - 1, sp + 1);
    }
    else if (strcmp(cmd, "monids") == 0)
    {
        // monids - every Moemon of ours and of the shared PC by personality:OT ID
        void MP_SharedBox_DebugIds(void);
        MP_SharedBox_DebugIds();
    }
    else if (strcmp(cmd, "ownbox") == 0)
    {
        // ownbox BOX - our own box (even with the shared PC's on screen)
        struct PokemonStorage *own = MP_PersonalStorage();
        int box = atoi(arg) - 1, i;
        if (box >= 0 && box < TOTAL_BOXES_COUNT)
        {
            printf("[test] ownbox %d:", box + 1);
            for (i = 0; i < IN_BOX_COUNT; i++)
                if (GetBoxMonData(&own->boxes[box][i], MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d:%d", i, (int)GetBoxMonData(&own->boxes[box][i], MON_DATA_SPECIES));
            printf(" | party:");
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
            printf("\n");
        }
    }
    else if (strcmp(cmd, "candyjar") == 0)
    {
        MP_Candy_OpenJar(CB2_ReturnToField);
    }
    else if (strcmp(cmd, "jarstate") == 0)
    {
        MP_Candy_LogJar();
    }
    else if (strcmp(cmd, "boxput") == 0)
    {
        // boxput BOX POS SPECIES LEVEL - a Moemon in a box slot (BOX from 1)
        int box = 1, pos = 0, species = 1, level = 5;
        struct BoxPokemon boxMon;
        sscanf(arg, "%d %d %d %d", &box, &pos, &species, &level);
        if (box >= 1 && box <= TOTAL_BOXES_COUNT && pos >= 0 && pos < IN_BOX_COUNT)
        {
            CreateBoxMon(&boxMon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
            SetBoxMonAt(box - 1, pos, &boxMon);
            printf("[test] boxput %d %d: %d\n", box, pos, species);
        }
    }
    else if (strcmp(cmd, "boxmons") == 0)
    {
        // boxmons BOX - the box's Moemon (position:species) and the party's species
        int box = 1, i;
        sscanf(arg, "%d", &box);
        if (box >= 1 && box <= TOTAL_BOXES_COUNT)
        {
            printf("[test] boxmons %d:", box);
            for (i = 0; i < IN_BOX_COUNT; i++)
                if (GetBoxMonDataAt(box - 1, i, MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d:%d", i, (int)GetBoxMonDataAt(box - 1, i, MON_DATA_SPECIES));
            printf(" | party:");
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
                    printf(" %d", (int)GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
            printf("\n");
        }
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
        // giveforeign SPECIES[:LEVEL[:lead]] - a Pokemon made in another player's game (another OT
        // ID), given the way a co-op catch is (GiveMonToPlayer); ":lead" swaps it into slot 0
        static struct Pokemon mon;
        int species = 0, level = 5;
        char lead[8] = "";
        sscanf(arg, "%d:%d:%7s", &species, &level, lead);
        CreateMon(&mon, species > 0 ? species : 288, level > 0 && level <= 100 ? level : 5, USE_RANDOM_IVS, FALSE, 0, OT_ID_PRESET, 0x9ABC1234);
        GiveMonToPlayer(&mon);
        if (strcmp(lead, "lead") == 0 && CalculatePlayerPartyCount() > 1)
        {
            struct Pokemon tmp = gPlayerParty[0];
            gPlayerParty[0] = gPlayerParty[CalculatePlayerPartyCount() - 1];
            gPlayerParty[CalculatePlayerPartyCount() - 1] = tmp;
        }
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
    else if (strcmp(cmd, "fxget") == 0)
    {
        // fxget SLOT - a party Moemon's ball effect, markings and ball style as stored
        int slot = atoi(arg);
        struct MpBallFx fx;
        u16 style;
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            MP_BallFx_OfMon(&gPlayerParty[slot], &fx);
            style = MP_BallStyle_OfMon(&gPlayerParty[slot]);
            printf("[test] fxget: slot %d form %d part %d color %d recall %d sound %d markings %d style %04x (color %d deco %d special %d)\n",
                   slot, fx.form, fx.part, fx.color, fx.recall, fx.sound, (int)GetMonData(&gPlayerParty[slot], MON_DATA_MARKINGS),
                   style, MP_BallStyle_Color(style), MP_BallStyle_Decoration(style), MP_BallStyle_Special(style));
        }
    }
    else if (strcmp(cmd, "markings") == 0)
    {
        // markings SLOT VALUE
        int slot = 0, v = 0;
        sscanf(arg, "%d %d", &slot, &v);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u8 m = v;
            SetMonData(&gPlayerParty[slot], MON_DATA_MARKINGS, &m);
        }
    }
    else if (strcmp(cmd, "stylemake") == 0)
    {
        // stylemake SLOT COLOR DECORATION - a POKé BALL style through MP_BallStyle_Make
        int slot = 0, c = 0, d = 0;
        sscanf(arg, "%d %d %d", &slot, &c, &d);
        if (slot >= 0 && slot < PARTY_SIZE)
        {
            u16 v = MP_BallStyle_Make(c, d);
            SetMonData(&gPlayerParty[slot], MON_DATA_ENCRYPT_SEPARATOR, &v);
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
    else if (strcmp(cmd, "bflags") == 0)
    {
        // bflags - the battle's type flags and battler count (and a few co-op bits)
        printf("[test] bflags %08x battlers %d coop %d wild %d triple %d inBattle %d\n", (unsigned)gBattleTypeFlags,
               gBattlersCount, gMpCoopBattle, gMpCoopWild, gIsTripleBattle, gMain.inBattle);
    }
    else if (strcmp(cmd, "trainerflag") == 0)
    {
        // trainerflag ID [ID...] - whether each trainer counts as beaten on this game
        char *tok = strtok(arg, " ");
        while (tok != NULL)
        {
            int id = atoi(tok);
            if (id > 0 && id < TRAINERS_COUNT)
                printf("[test] trainerflag %d = %d\n", id, FlagGet(TRAINER_FLAGS_START + id) ? 1 : 0);
            tok = strtok(NULL, " ");
        }
        fflush(stdout);
    }
    else if (strcmp(cmd, "partyids") == 0)
    {
        // partyids - each party Moemon's identity (species:personality:OT ID) and level / EXP, to check nothing was
        // lost, swapped, doubled or turned into a BAD EGG
        int i;
        printf("[test] partyids:");
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
                continue;
            printf(" %d:%08x:%08x:L%d:E%u%s", (int)GetMonData(mon, MON_DATA_SPECIES), (unsigned)GetMonData(mon, MON_DATA_PERSONALITY),
                   (unsigned)GetMonData(mon, MON_DATA_OT_ID), (int)GetMonData(mon, MON_DATA_LEVEL), (unsigned)GetMonData(mon, MON_DATA_EXP),
                   GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG) ? ":BADEGG" : "");
        }
        printf("\n");
    }
    else if (strcmp(cmd, "bwshown") == 0)   // bwshown - the weather on screen and the battle's own (mp_battleweather.c)
    {
        void MP_BattleWeather_Debug(void);
        MP_BattleWeather_Debug();
    }
    else if (strcmp(cmd, "bweather") == 0)   // bweather rain|downpour|sun|sand|hail|none - the battle's weather now
    {
        static const struct { const char *name; u16 flags; } sW[] = {
            {"rain", B_WEATHER_RAIN_TEMPORARY}, {"downpour", B_WEATHER_RAIN_DOWNPOUR}, {"sun", B_WEATHER_SUN_TEMPORARY},
            {"sand", B_WEATHER_SANDSTORM_TEMPORARY}, {"hail", B_WEATHER_HAIL_TEMPORARY}, {"none", 0},
        };
        int i;
        for (i = 0; i < (int)ARRAY_COUNT(sW); i++)
            if (strcmp(arg, sW[i].name) == 0 && gMain.inBattle)
            {
                gBattleWeather = sW[i].flags;
                gWishFutureKnock.weatherDuration = sW[i].flags ? 5 : 0;
                printf("[test] battle weather: %s\n", sW[i].name);
            }
    }
    else if (strcmp(cmd, "partycursor") == 0)   // where the party screen's cursor is (7 = CANCEL) and its layout
    {
        printf("[test] party menu cursor %d layout %d type %d (triple %d co-op %d flags %x)\n", gPartyMenu.slotId, gPartyMenu.layout,
               gPartyMenu.menuType, gIsTripleBattle, gMpCoopBattle, (unsigned)gBattleTypeFlags);
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
    else if (strcmp(cmd, "useflag") == 0)
    {
        // useflag - use the BATTLE FLAG from the field ("Set phrases, or plant?"); only on the field, like the item
        if (MP_IsInOverworld())
            MP_Trainer_UseFlag(CreateTask(TaskDummy, 80));
        else
            printf("[test] useflag: not on the field\n");
    }
    else if (strcmp(cmd, "easychat") == 0)
    {
        // easychat TYPE - the phrase builder the way the game's scripts open it (special ShowEasyChatScreen)
        void ShowEasyChatScreen(void);
        gSpecialVar_0x8004 = atoi(arg);
        ShowEasyChatScreen();
    }
    else if (strcmp(cmd, "phrasedefault") == 0)
    {
        // phrasedefault - the new game's phrases (profile, battle phrases; mail words cleared)
        void InitEasyChatPhrases(void);
        InitEasyChatPhrases();
    }
    else if (strcmp(cmd, "phraseclear") == 0)
    {
        // phraseclear N - empties a battle phrase (0 CHALLENGE, 1 WIN, 2 LOSE)
        int n = atoi(arg), i;
        u16 *w = n == 0 ? gSaveBlock1Ptr->easyChatBattleStart : n == 1 ? gSaveBlock1Ptr->easyChatBattleWon
                                                                        : gSaveBlock1Ptr->easyChatBattleLost;
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            w[i] = 0xFFFF;
        printf("[test] phrase %d cleared\n", n);
    }
    else if (strcmp(cmd, "phrases") == 0)
    {
        // phrases - the three battle phrases' words
        int i;
        printf("[test] phrases:");
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            printf(" %04x", gSaveBlock1Ptr->easyChatBattleStart[i]);
        printf(" |");
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            printf(" %04x", gSaveBlock1Ptr->easyChatBattleWon[i]);
        printf(" |");
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            printf(" %04x", gSaveBlock1Ptr->easyChatBattleLost[i]);
        printf("\n");
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
