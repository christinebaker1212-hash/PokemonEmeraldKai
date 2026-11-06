// Moemon Maker: F5, playing from the editor (docs/MOEMON_MAKER.md decisions 13, 14, 16). The game's own window shows
// again with the player at the spot asked for, as one of: a COPY of a save file, a story point (a new game at the start,
// with four badges, or as Champion) or a quick test party. A test never writes a real save: the play's save is
// MoemonMaker/.play/play.sav (and its own side files); saving, autosaving and its flags all go there. F12, or closing the
// game's window, goes back to editing (the real save read again). While the game plays, an edit of the map the player
// is on shows at once: tiles in place; anything else by loading the map again where the player stands.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "global.h"
#include "main.h"
#include "event_data.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "item.h"
#include "money.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "load_save.h"
#include "save.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "constants/heal_locations.h"
#include "maker.h"
#include "mp_region.h"
#include "maker_api.h"
#include "battle_setup.h"

extern char gPlatformSavePath[512];
void Platform_ReopenSaveFile(void);
void MainMenu_ReloadSave(void);
void CB2_NewGame(void);
void CB2_ContinueSavedGame(void);
void Tcg_ForgetSidecars(void);
void MP_TCard_Forget(void);
void MP_DebugSetupNameAndGender(void);
void Display_SetMainWindowShown(int shown);
void DoMakerReloadWarp(void);
void MakerUi_PlayStopped(void);
const u16 *Maker_MapBlocks(const struct MapHeader *h);
void SetContinueGameWarpStatus(void);
void SetContinueGameWarp(s8 mapGroup, s8 mapNum, s8 warpId, s16 x, s16 y);
void SetLastHealLocationWarp(u8 healLocationId);

static bool8 sPlaying;
static u16 sBenchTrainer;   // (M3: the battle bench: a trainer to battle as soon as the field is up)
static bool8 sNewGamePending;     // a story point: MP_DebugQuickStartNewGame asks Maker_PlayNewGame
static u8 sKind;
static s16 sMap, sX, sY;
static char sRealSavePath[512];
static u8 sReload;                // 1: the map's tiles, 2: the whole map (Maker_EditsChangedWhilePlaying)

bool8 Maker_IsPlaying(void)
{
    return sPlaying;
}

static void MakeDir(const char *path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static void CopyFile(const char *from, const char *to)
{
    FILE *in = fopen(from, "rb"), *out;
    char buf[8192];
    size_t n;
    if (!in)
        return;
    out = fopen(to, "wb");
    if (out)
    {
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, n, out);
        fclose(out);
    }
    fclose(in);
}

// The save and its side files ("<save>.region", ".life"...) copied to the play's save, or (from = NULL) removed
static void CopySaveFiles(const char *from, const char *to)
{
    static const char *const sSuffixes[] = {"", ".region", ".life", ".boxes", ".bag", ".cards", ".ygo", ".tt", ".card",
                                            ".maker", ".phone", ".sim", ".search", ".cardnew", ".bak1", NULL};
    char a[600], b[600];
    int i;
    for (i = 0; sSuffixes[i]; i++)
    {
        snprintf(b, sizeof(b), "%s%s", to, sSuffixes[i]);
        remove(b);
        if (from)
        {
            snprintf(a, sizeof(a), "%s%s", from, sSuffixes[i]);
            CopyFile(a, b);
        }
    }
}

static void UseSave(const char *path)
{
    snprintf(gPlatformSavePath, sizeof(sRealSavePath), "%s", path);
    Platform_ReopenSaveFile();
    Tcg_ForgetSidecars();
    MP_TCard_Forget();
    MainMenu_ReloadSave();
}

// F5 (the editor): play MAP at X, Y as KIND (MAKER_PLAY_*); SLOT: the save file copied (1-3, 0 = the one in use)
int MakerApi_Play(int map, int x, int y, int kind, int slot)
{
    const struct MakerMapName *m = Maker_Map(map);
    char dir[600], playSave[700], from[600];
    if (sPlaying || m == NULL)
        return 0;
    snprintf(sRealSavePath, sizeof(sRealSavePath), "%s", gPlatformSavePath);
    snprintf(dir, sizeof(dir), "%s", MakerApi_EditsDir());
    MakeDir(dir);
    snprintf(dir + strlen(dir), sizeof(dir) - strlen(dir), "/.play");
    MakeDir(dir);
    snprintf(playSave, sizeof(playSave), "%s/play.sav", dir);
    if (kind == MAKER_PLAY_SAVE && slot >= 1 && slot <= 3)
        snprintf(from, sizeof(from), "saves/slot%d/pokeemerald.sav", slot);
    else
        snprintf(from, sizeof(from), "%s", sRealSavePath);
    CopySaveFiles(kind == MAKER_PLAY_SAVE ? from : NULL, playSave);
    sPlaying = TRUE;
    sKind = kind;
    sMap = map;
    sX = x;
    sY = y;
    sReload = 0;
    UseSave(playSave);
    printf("[maker] playing %s at %d, %d (%s)\n", m->name, x, y, kind == MAKER_PLAY_SAVE ? "a copy of a save" : "a story point");
    Display_SetMainWindowShown(TRUE);
    if (kind == MAKER_PLAY_SAVE && gSaveFileStatus == SAVE_STATUS_OK)
    {
        // the save as it was, then straight to the spot (the save's continue warp, as the BATTLE FRONTIER uses)
        SetContinueGameWarp(m->mapGroup, m->mapNum, WARP_ID_NONE, x, y);
        SetContinueGameWarpStatus();
        SetMainCallback2(CB2_ContinueSavedGame);
    }
    else
    {
        // a new game, set up as the story point asked (MP_DebugQuickStartNewGame -> Maker_PlayNewGame)
        sNewGamePending = TRUE;
        MP_DebugSetupNameAndGender();
        SetMainCallback2(CB2_NewGame);
    }
    return 1;
}

// The battle bench (M3, the TRAINERS window): play as KIND and battle TRAINER at once (its "already beaten" flag cleared)
int MakerApi_PlayBattle(int map, int x, int y, int kind, int slot, int trainer)
{
    if (trainer <= 0 || !MakerApi_Play(map, x, y, kind, slot))
        return 0;
    sBenchTrainer = trainer;
    return 1;
}

static void BenchFrame(void)
{
    const struct MakerMapName *m = Maker_Map(sMap);
    const char *name = MakerApi_NameOf(MAKER_NAMES_TRAINER, sBenchTrainer);
    char src[200];
    if (!sBenchTrainer || sNewGamePending || gMain.callback2 != CB2_Overworld || ArePlayerFieldControlsLocked()
     || ScriptContext_IsEnabled() || m == NULL)
        return;
    snprintf(src, sizeof(src), "battle %s \"The battle bench!\" \"That was the bench battle.\"\n", name);
    ClearTrainerFlag(sBenchTrainer);
    ScriptContext_SetupScript(Maker_EventScript(m->name, "BATTLE_BENCH", MAKER_EVENT_ON_SIGN, src));
    printf("[maker] battle bench: %s\n", name);
    sBenchTrainer = 0;
}

bool8 Maker_PlayNewGamePending(void)
{
    return sNewGamePending;
}

static void GiveMon(u8 slot, u16 species, u8 level)
{
    CreateMon(&gPlayerParty[slot], species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
}

// mp_debug.c MP_DebugQuickStartNewGame: a story point's new game (TRUE: it's done here)
bool8 Maker_PlayNewGame(void)
{
    const struct MakerMapName *m = Maker_Map(sMap);
    u16 f;
    if (!sNewGamePending || m == NULL)
        return FALSE;
    // (sNewGamePending stays until the field is up: CB2_NewGame asks MP_DebugQuickStartEnabled after this, to skip the
    // truck: Maker_PlayFrame clears it)
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_ADVENTURE_STARTED);
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    VarSet(VAR_LITTLEROOT_TOWN_STATE, 4);
    VarSet(VAR_BIRCH_LAB_STATE, 5);
    VarSet(VAR_ROUTE101_STATE, 3);
    AddBagItem(ITEM_POKE_BALL, 10);
    AddBagItem(ITEM_POTION, 10);
    ZeroPlayerPartyMons();
    switch (sKind)
    {
    default:   // MAKER_PLAY_START
        SetMoney(&gSaveBlock1Ptr->money, 3000);
        GiveMon(0, SPECIES_TREECKO, 7);
        break;
    case MAKER_PLAY_BADGES4:
        SetMoney(&gSaveBlock1Ptr->money, 30000);
        for (f = FLAG_BADGE01_GET; f <= FLAG_BADGE04_GET; f++)
            FlagSet(f);
        GiveMon(0, SPECIES_GROVYLE, 30);
        GiveMon(1, SPECIES_KIRLIA, 28);
        GiveMon(2, SPECIES_MARSHTOMP, 27);
        AddBagItem(ITEM_SUPER_POTION, 10);
        break;
    case MAKER_PLAY_CHAMPION:
        SetMoney(&gSaveBlock1Ptr->money, 200000);
        for (f = FLAG_BADGE01_GET; f <= FLAG_BADGE08_GET; f++)
            FlagSet(f);
        FlagSet(FLAG_SYS_GAME_CLEAR);
        GiveMon(0, SPECIES_SCEPTILE, 60);
        GiveMon(1, SPECIES_GARDEVOIR, 58);
        GiveMon(2, SPECIES_SALAMENCE, 58);
        GiveMon(3, SPECIES_METAGROSS, 58);
        GiveMon(4, SPECIES_SWAMPERT, 57);
        GiveMon(5, SPECIES_MILOTIC, 57);
        AddBagItem(ITEM_FULL_RESTORE, 20);
        break;
    case MAKER_PLAY_TEST:
        // (strong, every badge and HM: nothing in the way of testing)
        SetMoney(&gSaveBlock1Ptr->money, 999999);
        for (f = FLAG_BADGE01_GET; f <= FLAG_BADGE08_GET; f++)
            FlagSet(f);
        GiveMon(0, SPECIES_MEWTWO, 80);
        GiveMon(1, SPECIES_SALAMENCE, 75);
        GiveMon(2, SPECIES_SWAMPERT, 75);
        for (f = ITEM_HM01; f <= ITEM_HM08; f++)
            AddBagItem(f, 1);
        for (f = ITEM_HM_ROCK_CLIMB; f <= ITEM_HM_WHIRLPOOL; f++)
            AddBagItem(f, 1);
        AddBagItem(ITEM_FULL_RESTORE, 50);
        AddBagItem(ITEM_MAX_REPEL, 20);
        break;
    }
    CalculatePlayerPartyCount();
    // the map's own region: arrived in, its badges, and (CHAMPION, TEST) its Champion, the boats to every region open
    MP_Region_SetUpForPlay(MP_Region_OfGroup(m->mapGroup),
                           sKind == MAKER_PLAY_BADGES4 ? 4 : sKind == MAKER_PLAY_CHAMPION || sKind == MAKER_PLAY_TEST ? 8 : 0,
                           sKind == MAKER_PLAY_CHAMPION || sKind == MAKER_PLAY_TEST);
    SetLastHealLocationWarp(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    SetWarpDestination(m->mapGroup, m->mapNum, WARP_ID_NONE, sX, sY);
    WarpIntoMap();
    return TRUE;
}

// F12 or the game's window closed: back to editing, the real save read again
void Maker_StopPlay(void)
{
    if (!sPlaying)
        return;
    sPlaying = FALSE;
    sNewGamePending = FALSE;
    sBenchTrainer = 0;
    Maker_DebugReset();
    UseSave(sRealSavePath);
    Display_SetMainWindowShown(FALSE);
    Maker_BackToWaiting();
    MakerUi_PlayStopped();
    printf("[maker] back to editing\n");
}

void MakerApi_StopPlay(void)
{
    Maker_StopPlay();
}

int MakerApi_IsPlaying(void)
{
    return sPlaying;
}

// maker_overrides.c: the edits of the map the player is on changed (blocksOnly: just its tiles)
void Maker_EditsChangedWhilePlaying(int map, bool8 blocksOnly)
{
    if (!sPlaying || map != Maker_MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum))
        return;
    if (!blocksOnly)
        sReload = 2;
    else if (sReload == 0)
        sReload = 1;
}

// Every frame (maker_gate.c Maker_FrameUpdate): a waiting reload, once the player is free in the field
void Maker_PlayFrame(void)
{
    if (sNewGamePending && gMain.callback2 == CB2_Overworld)
        sNewGamePending = FALSE;
    if (sPlaying)
        BenchFrame();
    if (!sPlaying || !sReload || gMain.callback2 != CB2_Overworld || ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled())
        return;
    if (sReload == 1)
    {
        // the map's tiles as they are now, drawn again where the player stands
        const u16 *blocks = Maker_MapBlocks(&gMapHeader);
        s32 x, y, w = gMapHeader.mapLayout->width, h = gMapHeader.mapLayout->height;
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
                if (MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET) != (blocks[y * w + x] & MAPGRID_METATILE_ID_MASK)
                 || MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) != ((blocks[y * w + x] & MAPGRID_COLLISION_MASK) >> MAPGRID_COLLISION_SHIFT))
                    MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, blocks[y * w + x]);
        DrawWholeMapView();
    }
    else
    {
        // the map loaded again where the player stands (its people, warps, music...)
        SetWarpDestination(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE,
                           gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
        DoMakerReloadWarp();
    }
    sReload = 0;
}
