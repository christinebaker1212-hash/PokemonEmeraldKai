#include <stdlib.h>
// Moemon Kai: the boat to ORRE (docs/ORRE_DECISIONS.md "A post-game place", the user 2026-11-07: "Orre will not be
// selectable as a region to start in, rather, it is always a post-game region, and a special one at that - it goes on
// the BATTLE FRONTIER, BATTLE ISLAND, FARADAY ISLAND, etc menu"). The ORRE TICKET comes after any region's ELITE FOUR
// (src/mp/mp_battleisland.c's courier, old saves too); HOENN's ferries list ORRE, every other port offers it
// (data/scripts/orre.inc). The boat lands at the OUTSKIRT STAND, Colosseum's own start, in front of its sailor (Kai's
// one person in ORRE: the user's choice), who sails back to the port the player came from (ORRE's maps are shared,
// mp_region.c MP_Region_IsSharedMap: the player stays in their region). A Shadow Pokémon can't leave ORRE
// (docs/SHADOW_POKEMON.md decision 2).
#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "mp.h"
#include "mp_region.h"
#include "orre.h"
#include "orre_shadow.h"
#include "o3d.h"
#include "field_message_box.h"
#include "main.h"
#include "menu.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/maps.h"

void MP_Voyage_Begin(bool8 eastbound, bool8 faded);   // mp_voyage.c
bool8 MP_Region_PortOf(u8 region, u16 *map, s16 *x, s16 *y);   // qz_specials.c

void Orre_Sail(void)
{
    s16 x = 33, y = 40;
    Orre_MapStart(MAP_ORRE_OUTSKIRT_STAND, &x, &y);
    SetWarpDestination(MAP_GROUP(MAP_ORRE_OUTSKIRT_STAND), MAP_NUM(MAP_ORRE_OUTSKIRT_STAND), WARP_ID_NONE, x, y);
    MPNet_Log("orre: sailing from map %d.%d", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    MP_Voyage_Begin(FALSE, FALSE);   // (westward)
}

void Orre_SailBack(void)
{
    if (gRegionState.originValid && !O3D_IsOrreGroup(gRegionState.originGroup)
     && gRegionState.originGroup != MAP_GROUP(MAP_BATTLE_ISLAND))
        SetWarpDestination(gRegionState.originGroup, gRegionState.originNum, WARP_ID_NONE, gRegionState.originX,
                           gRegionState.originY);
    else
    {
        u16 map;
        s16 x, y;
        if (!MP_Region_PortOf(MP_Region_Current(), &map, &x, &y))
            map = MAP_LILYCOVE_CITY_HARBOR, x = 8, y = 11;
        SetWarpDestination(map >> 8, map & 0xFF, WARP_ID_NONE, x, y);
    }
    MPNet_Log("orre: sailing back");
    MP_Voyage_Begin(TRUE, FALSE);
}

void Orre_ShadowAboard(void)
{
    gSpecialVar_Result = Orre_Shadow_PartyHasShadow();
    if (getenv("MOEMON_TEST_STAY"))
        gSpecialVar_Result = TRUE;   // (tests that press A everywhere: the sailor won't take them home)
}

// ---- ORRE's packs before the boat sails (the user, 2026-11-07: "Let's go ahead and ship it"): the world, text and
// people of ORRE come in MoemonOrre.zip (docs/ORRE_DECISIONS.md "As their own zip"), not with the game. The boat
// checks they're here; if not, the latest release is asked (src/platform/updater.c Updater_Orre*, as the DANCE
// machine's songs) and the clerk offers the download.
enum { UPD_IDLE, UPD_CHECKING, UPD_AVAILABLE, UPD_UPTODATE, UPD_NONE, UPD_DOWNLOADING, UPD_INSTALLING, UPD_DONE,
       UPD_FAILED };   // (updater.c's SONGS_*)
void Updater_OrreCheck(void);
void Updater_OrreDownload(void);
void Updater_OrreCancel(void);
int Updater_OrreState(long *done, long *total, const char **error);

#define tTimer data[0]

// (special, waitstate) VAR_RESULT: 1 ready to sail; 2 a download is needed (its size in STR_VAR_1, in MB); 3 the boat
// isn't sailing yet (no release has ORRE's packs); 4 the release couldn't be asked (why in STR_VAR_1); 5 the launch's
// download is still under way
static void Task_OrreCheckBoat(u8 taskId)
{
    long done, total;
    const char *err;
    int st = Updater_OrreState(&done, &total, &err);
    if (st == UPD_CHECKING || (st == UPD_IDLE && gTasks[taskId].tTimer++ < 2))
        return;
    if (st == UPD_DOWNLOADING || st == UPD_INSTALLING)
        gSpecialVar_Result = 5;   // (the launch's own download is still under way: platform/updater.c OrrePreWorker)
    else if (st == UPD_AVAILABLE)
    {
        ConvertIntToDecimalStringN(gStringVar1, (total + (1 << 20) - 1) >> 20, STR_CONV_MODE_LEFT_ALIGN, 5);
        gSpecialVar_Result = 2;
    }
    else if (st == UPD_FAILED)
    {
        MP_AsciiToGame(gStringVar1, err && *err ? err : "NO CONNECTION", 60);
        gSpecialVar_Result = 4;
    }
    else
        gSpecialVar_Result = 3;
    MPNet_Log("orre: the boat checked ORRE's packs: %d (state %d)", gSpecialVar_Result, st);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// (the script waits: it goes on from a task, the next frame at the soonest)
static void Task_OrreBoatReady(u8 taskId)
{
    DestroyTask(taskId);
    ScriptContext_Enable();
}

void Orre_CheckBoat(void)
{
    if (O3D_PackPresent())
    {
        gSpecialVar_Result = 1;
        CreateTask(Task_OrreBoatReady, 80);
        return;
    }
    Updater_OrreCheck();
    CreateTask(Task_OrreCheckBoat, 80);
}

// (special, waitstate) the download, its progress in the message box (B calls it off); VAR_RESULT TRUE when ORRE's
// packs are in place
static void ShowProgress(const char *ascii)
{
    static u8 sText[120];
    MP_AsciiToGame(sText, ascii, sizeof(sText) - 1);
    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    AddTextPrinterParameterized(0, FONT_NORMAL, sText, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void Task_OrreDownload(u8 taskId)
{
    long done, total;
    const char *err;
    char line[120];
    int st = Updater_OrreState(&done, &total, &err);
    if (JOY_NEW(B_BUTTON) && (st == UPD_DOWNLOADING || st == UPD_INSTALLING))
        Updater_OrreCancel();
    if (st == UPD_DOWNLOADING || st == UPD_INSTALLING || st == UPD_AVAILABLE)
    {
        if (++gTasks[taskId].tTimer % 15 != 1)
            return;
        if (st == UPD_INSTALLING)
            snprintf(line, sizeof(line), "Unpacking ORRE... %ld of %ld files", done, total);
        else
            snprintf(line, sizeof(line), "Downloading ORRE... %ld%%\n(B: stop)", total > 0 ? done * 100 / total : 0);
        ShowProgress(line);
        return;
    }
    gSpecialVar_Result = st == UPD_DONE && O3D_PackPresent();
    MPNet_Log("orre: ORRE's packs downloaded: %d (state %d%s%s)", gSpecialVar_Result, st, err && *err ? ", " : "",
              err ? err : "");
    if (!gSpecialVar_Result)
        MP_AsciiToGame(gStringVar1, err && *err ? err : "IT WAS STOPPED", 60);
    HideFieldMessageBox();
    DestroyTask(taskId);
    ScriptContext_Enable();
}

void Orre_DownloadPacks(void)
{
    Updater_OrreDownload();
    ShowFieldMessage(gText_EmptyString2);
    CreateTask(Task_OrreDownload, 80);
}
