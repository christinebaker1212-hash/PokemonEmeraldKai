// Moemon Kai: JOHTO's fruit trees as Crystal's (pokecrystal engine/events/fruit_trees.asm, maps/KurtsHouse.asm,
// engine/events/kurt.asm), in Quetzal's berry trees' places (tools/quetzal_port/fix_fruit_trees.py):
// - the APRICORN trees (AZALEA's WHT, ROUTE 37's RED / BLU / BLK, ROUTE 42's PNK / GRN / YLW) bear one APRICORN a
//   day each: "It's a fruit-bearing tree." / "Hey! It's RED APRICORN!" / "There's nothing here…"
// - KURT makes POKé BALLS of them: the APRICORN and how many, then a day's wait (RED: LEVEL BALL, BLU: LURE,
//   YLW: MOON, GRN: FRIEND, WHT: FAST, BLK: HEAVY, PNK: LOVE)
// - Quetzal's other trees are BERRY patches (the game's own, 1x1 soil) starting with the BERRY Crystal's tree there
//   bore, in its Gen 3 name (BERRY: ORAN, PSNCUREBERRY: PECHA, PRZCUREBERRY: CHERI, BITTER BERRY: PERSIM, MINT
//   BERRY: CHESTO, ICE BERRY: ASPEAR, BURNT BERRY: RAWST, MYSTERYBERRY: LEPPA), on tree numbers of their own (the
//   GARDEN farm's 100-119 were shared before).
// JOHTO's vars (the region sidecar): 0x52E0 the trees picked today (a bit each), 0x52E1 that day, 0x52E2 KURT's
// APRICORN (0 none), 0x52E3 how many, 0x52E4 the day he took them, 0x52E5 the BERRY patches planted.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "mp.h"
#include "berry.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/berry.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "mp_region.h"

#define VAR_TREES_PICKED 0x52E0
#define VAR_TREES_DAY    0x52E1
#define VAR_KURT_ITEM    0x52E2
#define VAR_KURT_COUNT   0x52E3
#define VAR_KURT_DAY     0x52E4
#define VAR_BERRIES_SET  0x52E5

struct FruitTree
{
    u16 map;
    u8 x, y;
    u16 item;
};

// the APRICORN trees (where they stand on JOHTO's maps): Crystal's fruit tree, the same all day as in Crystal
// (OBJ_EVENT_GFX_FRUIT_TREE, data/object_events/kai_objects.h); one APRICORN a day each
static const struct FruitTree sApricornTrees[] = {
    {MAP_JOHTO_AZALEA_TOWN, 12, 7, ITEM_WHT_APRICORN},
    {MAP_JOHTO_ROUTE37, 32, 24, ITEM_RED_APRICORN},
    {MAP_JOHTO_ROUTE37, 33, 24, ITEM_BLU_APRICORN},
    {MAP_JOHTO_ROUTE37, 34, 24, ITEM_BLK_APRICORN},
    {MAP_JOHTO_ROUTE42, 39, 19, ITEM_PNK_APRICORN},
    {MAP_JOHTO_ROUTE42, 40, 19, ITEM_GRN_APRICORN},
    {MAP_JOHTO_ROUTE42, 41, 19, ITEM_YLW_APRICORN},
};

// the BERRY patches: tree number, the BERRY a new patch starts with (ITEM_NONE: soil)
static const struct {u8 tree; u16 item;} sBerryPatches[] = {
    {90, ITEM_PERSIM_BERRY},   // ROUTE 31 (BITTER BERRY)
    {91, ITEM_CHERI_BERRY},    // VIOLET CITY (PRZCUREBERRY)
    {94, ITEM_PECHA_BERRY},    // ROUTE 33 (PSNCUREBERRY)
    {96, ITEM_ORAN_BERRY},     // ROUTE 38 (BERRY)
    {97, ITEM_CHESTO_BERRY},   // ROUTE 39 (MINT BERRY)
    {98, ITEM_RAWST_BERRY},    // ROUTE 44 (BURNT BERRY)
    {99, ITEM_LEPPA_BERRY},    // ROUTE 45 (MYSTERYBERRY)
    {120, ITEM_PERSIM_BERRY},  // ROUTE 43 (BITTER BERRY)
    {121, ITEM_PERSIM_BERRY},  // ROUTE 43, the second
    {122, ITEM_ORAN_BERRY},    // ROUTE 29 (BERRY)
    {123, ITEM_ORAN_BERRY},    // ROUTE 30's south (BERRY)
    {124, ITEM_LEPPA_BERRY},   // ROUTE 35 (MYSTERYBERRY)
    {125, ITEM_ASPEAR_BERRY},  // ROUTE 36 (ICE BERRY)
    {126, ITEM_ORAN_BERRY},    // ROUTE 46 (BERRY)
    {127, ITEM_PECHA_BERRY},   // ROUTE 30's north (PSNCUREBERRY)
    {92, ITEM_NONE},           // ROUTE 47 (none in Crystal: soil)
    {93, ITEM_ASPEAR_BERRY},   // ROUTE 26 (ICE BERRY)
};

static const u16 sBalls[] = {ITEM_LEVEL_BALL, ITEM_LURE_BALL, ITEM_MOON_BALL, ITEM_FRIEND_BALL, ITEM_FAST_BALL,
                             ITEM_HEAVY_BALL, ITEM_LOVE_BALL};   // (by APRICORN: RED, BLU, YLW, GRN, WHT, BLK, PNK)

static u16 Today(void)
{
    RtcCalcLocalTime();
    return (u16)gLocalTime.days;
}

static u16 CurMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

// the tree the player talked to (by the object's place on the map), or -1
static s32 TalkedTree(void)
{
    u32 i, j;
    for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
    {
        const struct ObjectEventTemplate *t = &gSaveBlock1Ptr->objectEventTemplates[i];
        if (t->localId != gSpecialVar_LastTalked)
            continue;
        for (j = 0; j < ARRAY_COUNT(sApricornTrees); j++)
            if (sApricornTrees[j].map == CurMap() && sApricornTrees[j].x == t->x && sApricornTrees[j].y == t->y)
                return j;
    }
    return -1;
}

static void NewDay(void)
{
    if (VarGet(VAR_TREES_DAY) != Today())
    {
        VarSet(VAR_TREES_DAY, Today());
        VarSet(VAR_TREES_PICKED, 0);
    }
}


// The tree: VAR_0x8005 its APRICORN (named in STR_VAR_1), VAR_RESULT 1 when one hangs there today
u16 QzFruitTreeCheck(void)
{
    s32 t = TalkedTree();
    gSpecialVar_Result = FALSE;
    if (t < 0)
        return 0;
    NewDay();
    gSpecialVar_0x8005 = sApricornTrees[t].item;
    CopyItemName(gSpecialVar_0x8005, gStringVar1);
    gSpecialVar_Result = !(VarGet(VAR_TREES_PICKED) & (1 << t));
    return 0;
}

u16 QzFruitTreePick(void)
{
    s32 t = TalkedTree();
    if (t >= 0)
        VarSet(VAR_TREES_PICKED, VarGet(VAR_TREES_PICKED) | (1 << t));
    return 0;
}

// JOHTO's BERRY patches stand on Crystal's fruit trees (the tree is the map's): the BERRY tree's own sprite never shows
// (as in Crystal, the tree looks the same ripe or not; the user, 2026-11-04), nobody plants or waters them (Crystal's "There's nothing here…" instead), and a picked
// patch grows its BERRY again on the game's own timer (the user, 2026-11-03)
bool8 MP_JohtoBerries_IsPatch(u8 tree)
{
    u32 i;
    if (MP_Region_Current() != REGION_JOHTO)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sBerryPatches); i++)
        if (sBerryPatches[i].tree == tree)
            return TRUE;
    return FALSE;
}

// After its BERRIES are picked (berry.c ObjectEventInteractionRemoveBerryTree): the same BERRY again, from the start
// (the patch's own when it had none: Crystal's)
void MP_JohtoBerries_Picked(u8 tree, u8 berry)
{
    u32 i;
    if (!MP_JohtoBerries_IsPatch(tree))
        return;
    if (berry == 0)
        for (i = 0; i < ARRAY_COUNT(sBerryPatches); i++)
            if (sBerryPatches[i].tree == tree && sBerryPatches[i].item != ITEM_NONE)
                berry = ItemIdToBerryType(sBerryPatches[i].item);
    if (berry != 0)
        PlantBerryTree(tree, berry, BERRY_STAGE_PLANTED, TRUE);
}

// The BERRY tree script's first question (data/scripts/berry_tree.inc): VAR_RESULT TRUE when this is a JOHTO patch
// whose BERRIES aren't ripe (it says Crystal's words and stops)
u16 MP_JohtoBerries_NotRipe(void)
{
    u8 tree = GetObjectEventBerryTreeId(gSelectedObjectEvent);
    gSpecialVar_Result = MP_JohtoBerries_IsPatch(tree) && GetStageByBerryTreeId(tree) != BERRY_STAGE_BERRIES;
    return gSpecialVar_Result;
}

// After a JOHTO map's transition scripts (overworld.c): the BERRY patches start with Crystal's BERRIES, once
void MP_JohtoBerries_Init(void)
{
    u32 i;
    if (MP_Region_Current() != REGION_JOHTO)
        return;
    if (VarGet(VAR_BERRIES_SET))
        return;
    for (i = 0; i < ARRAY_COUNT(sBerryPatches); i++)
    {
        struct BerryTree *tree = GetBerryTreeInfo(sBerryPatches[i].tree);
        memset(tree, 0, sizeof(*tree));
        if (sBerryPatches[i].item != ITEM_NONE)
        {
            PlantBerryTree(sBerryPatches[i].tree, ItemIdToBerryType(sBerryPatches[i].item), BERRY_STAGE_BERRIES, TRUE);
            tree->berryYield = 2;
        }
    }
    VarSet(VAR_BERRIES_SET, 1);
}

// KURT. VAR_RESULT: 0 nothing with him, 1 still making (the same day), 2 ready; VAR_0x8005 the BALL, VAR_0x8006 how
// many
u16 QzKurtBallState(void)
{
    u16 item = VarGet(VAR_KURT_ITEM);
    if (item < FIRST_APRICORN || item > LAST_APRICORN)
        return gSpecialVar_Result = 0;
    gSpecialVar_0x8005 = sBalls[item - FIRST_APRICORN];
    gSpecialVar_0x8006 = VarGet(VAR_KURT_COUNT);
    return gSpecialVar_Result = VarGet(VAR_KURT_DAY) == Today() ? 1 : 2;
}

u16 QzKurtBallTaken(void)
{
    MP_Achieve_Unlock("jkurt");   // (JOHTO's achievement: KURT made a ball)
    VarSet(VAR_KURT_ITEM, 0);
    VarSet(VAR_KURT_COUNT, 0);
    return 0;
}

// VAR_RESULT: TRUE when the BAG holds an APRICORN
u16 QzHasApricorn(void)
{
    u16 i;
    gSpecialVar_Result = FALSE;
    for (i = FIRST_APRICORN; i <= LAST_APRICORN; i++)
        if (CheckBagHasItem(i, 1))
            gSpecialVar_Result = TRUE;
    return 0;
}

// "Which APRICORN should I use?" then "How many should I make?": the APRICORNs the BAG holds and CANCEL; then
// the number (UP / DOWN one, LEFT / RIGHT ten). VAR_RESULT after: FALSE cancelled, else the APRICORN (taken from
// the BAG, with KURT until tomorrow)
static EWRAM_DATA struct
{
    u8 window, cursor, count, picking, rows;
    u16 items[LAST_APRICORN - FIRST_APRICORN + 1];
    u16 quantity, max;
} sK = {0};

static const u8 sText_Cancel[] = _("CANCEL");
static const u8 sText_Times[] = _("×");

static void DrawList(void)
{
    u8 i, buf[32], *p;
    FillWindowPixelBuffer(sK.window, PIXEL_FILL(1));
    for (i = 0; i < sK.rows; i++)
    {
        if (i < sK.count)
        {
            CopyItemName(sK.items[i], buf);
            AddTextPrinterParameterized(sK.window, FONT_NORMAL, buf, 8, i * 16, TEXT_SKIP_DRAW, NULL);
            p = StringCopy(buf, sText_Times);
            ConvertIntToDecimalStringN(p, CountTotalItemQuantityInBag(sK.items[i]), STR_CONV_MODE_LEFT_ALIGN, 3);
            AddTextPrinterParameterized(sK.window, FONT_NORMAL, buf, 104, i * 16, TEXT_SKIP_DRAW, NULL);
        }
        else
        {
            AddTextPrinterParameterized(sK.window, FONT_NORMAL, sText_Cancel, 8, i * 16, TEXT_SKIP_DRAW, NULL);
        }
    }
    if (sK.picking)
    {
        p = StringCopy(buf, sText_Times);
        ConvertIntToDecimalStringN(p, sK.quantity, STR_CONV_MODE_LEADING_ZEROS, 2);
        AddTextPrinterParameterized(sK.window, FONT_NORMAL, buf, 104, sK.cursor * 16, TEXT_SKIP_DRAW, NULL);
        FillWindowPixelRect(sK.window, PIXEL_FILL(2), 104, sK.cursor * 16 + 13, 20, 1);
    }
    else
    {
        AddTextPrinterParameterized(sK.window, FONT_NORMAL, gText_SelectorArrow2, 0, sK.cursor * 16, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(sK.window, COPYWIN_FULL);
}

static void Finish(u8 taskId, u16 result)
{
    ClearStdWindowAndFrameToTransparent(sK.window, TRUE);
    RemoveWindow(sK.window);
    gSpecialVar_Result = result;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Kurt(u8 taskId)
{
    if (!sK.picking)
    {
        if (JOY_NEW(DPAD_UP) && sK.cursor > 0)
            sK.cursor--, PlaySE(SE_SELECT), DrawList();
        else if (JOY_NEW(DPAD_DOWN) && sK.cursor < sK.rows - 1)
            sK.cursor++, PlaySE(SE_SELECT), DrawList();
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sK.cursor == sK.count))
            PlaySE(SE_SELECT), Finish(taskId, FALSE);
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            sK.picking = TRUE;
            sK.quantity = 1;
            sK.max = min(CountTotalItemQuantityInBag(sK.items[sK.cursor]), 99);
            DrawList();
        }
        return;
    }
    if (JOY_NEW(DPAD_UP))
        sK.quantity = sK.quantity >= sK.max ? 1 : sK.quantity + 1, PlaySE(SE_SELECT), DrawList();
    else if (JOY_NEW(DPAD_DOWN))
        sK.quantity = sK.quantity <= 1 ? sK.max : sK.quantity - 1, PlaySE(SE_SELECT), DrawList();
    else if (JOY_NEW(DPAD_RIGHT))
        sK.quantity = min(sK.quantity + 10, sK.max), PlaySE(SE_SELECT), DrawList();
    else if (JOY_NEW(DPAD_LEFT))
        sK.quantity = sK.quantity > 10 ? sK.quantity - 10 : 1, PlaySE(SE_SELECT), DrawList();
    else if (JOY_NEW(B_BUTTON))
        sK.picking = FALSE, PlaySE(SE_SELECT), DrawList();
    else if (JOY_NEW(A_BUTTON))
    {
        u16 item = sK.items[sK.cursor];
        PlaySE(SE_SELECT);
        RemoveBagItem(item, sK.quantity);
        VarSet(VAR_KURT_ITEM, item);
        VarSet(VAR_KURT_COUNT, sK.quantity);
        VarSet(VAR_KURT_DAY, Today());
        Finish(taskId, item);
    }
}

u16 QzKurtChooseApricorn(void)
{
    struct WindowTemplate t = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 17, .height = 2,
                               .paletteNum = 15, .baseBlock = 100};   // (clear of the message box)
    u16 i;
    sK.count = 0;
    for (i = FIRST_APRICORN; i <= LAST_APRICORN; i++)
        if (CheckBagHasItem(i, 1))
            sK.items[sK.count++] = i;
    sK.rows = sK.count + 1;
    sK.cursor = 0;
    sK.picking = FALSE;
    t.height = sK.rows * 2;
    sK.window = AddWindow(&t);
    DrawStdWindowFrame(sK.window, FALSE);
    PutWindowTilemap(sK.window);
    DrawList();
    CreateTask(Task_Kurt, 80);
    return 0;
}

// Tests (mp apricorn): the trees picked today, KURT's state
void MP_Apricorn_Debug(void)
{
    printf("[test] apricorn: picked %u day %u, kurt item %u count %u day %u (today %u), berries %u\n",
           VarGet(VAR_TREES_PICKED), VarGet(VAR_TREES_DAY), VarGet(VAR_KURT_ITEM), VarGet(VAR_KURT_COUNT),
           VarGet(VAR_KURT_DAY), Today(), VarGet(VAR_BERRIES_SET));
}
