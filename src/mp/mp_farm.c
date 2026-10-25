// Moemon Co-op: the MOéMON GARDEN's BERRY farm (include/mp_farm.h, docs/COOP_NOTES.md "BERRY farm").
//
// The plots are the game's own BERRY trees (object events on the Garden map, berry tree ids GARDEN_FARM_TREE_FIRST..),
// so planting, watering, growing, picking and Emerald's rules are the game's. Ours are in the save and grow on our own
// clock (clock.c). Online, in a room someone else made, the plots show the room host's farm instead: the host sends it
// (MPR_FARM_STATE) whenever it changes, and a friend asks the host before planting or picking (MPR_FARM_ASK, the
// BERRY tree script waits for MPR_FARM_ANSWER), so a plot or its BERRIES go to one Trainer only. Watering and the
// Garden Pokémon's work are sent the same way without an answer.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "berry.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "item.h"
#include "item_menu.h"
#include "main.h"
#include "mp.h"
#include "mp_farm.h"
#include "mp_garden.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "tv.h"
#include "constants/berry.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#define GARDEN_WANT_FARM
#include "mp_garden_map.h"

#define PLOTS GARDEN_FARM_PLOTS
#define FIRST GARDEN_FARM_TREE_FIRST
#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

#define MPR_FARM_STATE  (MPR_FARM_FIRST + 0)   // the host -> everyone: u8 open plots, then TREE_BYTES a plot
#define MPR_FARM_ASK    (MPR_FARM_FIRST + 1)   // a friend -> the host: u8 kind, plot, berry, stage, seq
#define MPR_FARM_ANSWER (MPR_FARM_FIRST + 2)   // the host -> that friend: u8 seq, ok, plot, berry, count
#define TREE_BYTES 7
#define STATE_BYTES (2 + PLOTS * TREE_BYTES)
#define ASK_TIMEOUT (60 * 8)

enum { ASK_PLANT, ASK_PICK, ASK_WATER, ASK_TEND, ASK_NIBBLE };

STATIC_ASSERT(sizeof(struct BerryTree) == 8, BerryTreeKeepsItsSize);   // mpCare lives in what was padding

static struct
{
    // the host's farm, as it last told us (online, as a friend)
    s8 heardFrom;           // -1: nothing heard yet
    u8 plots;
    struct BerryTree trees[PLOTS];
    // our question (the BERRY tree script waits)
    bool8 asking;
    u8 askKind, askPlot, seq;
    u16 askTimer;
    bool8 picked;           // what the host handed over
    u8 pickedPlot, pickedBerry, pickedCount;
    // the host's side
    u8 lastSent[STATE_BYTES];
    u16 sendTimer, heartbeat;
    bool8 knew[MP_MAX_PLAYERS];
    // the map
    s8 shownPlots;          // the plots stamped on the map (-1: not yet)
    u16 checkTimer;
} sF = {.heardFrom = -1, .shownPlots = -1};

// ---------------------------------------------------------------------------
// Whose farm

bool8 MP_Farm_IsFriends(void)
{
    return MP_RoomHostId() >= 0;
}

static bool8 IsPlot(u8 id)
{
    return id >= FIRST && id < FIRST + PLOTS;
}

static u8 OwnPlots(void)
{
    u16 n = 4 + 2 * MP_Badges();
    return n > PLOTS ? PLOTS : n;
}

u8 MP_Farm_Plots(void)
{
    if (MP_Farm_IsFriends())
        return sF.heardFrom == MP_RoomHostId() ? sF.plots : 0;
    return OwnPlots();
}

struct BerryTree *MP_Farm_Tree(u8 id)
{
    if (!IsPlot(id) || !MP_Farm_IsFriends())
        return NULL;
    if (sF.heardFrom != MP_RoomHostId())
    {
        // a host we haven't heard from yet: nothing planted (and nothing open: MP_Farm_Plots)
        memset(sF.trees, 0, sizeof(sF.trees));
        sF.plots = 0;
    }
    return &sF.trees[id - FIRST];
}

static struct BerryTree *OwnTree(u8 plot)
{
    return &gSaveBlock1Ptr->berryTrees[FIRST + plot];
}

// The tree a plot shows here (the host's as a friend, else ours)
static struct BerryTree *Shown(u8 plot)
{
    return GetBerryTreeInfo(FIRST + plot);
}

// ---------------------------------------------------------------------------
// What a plot needs (the same rules on the host's game and ours)

static u16 StageMinutes(const struct BerryTree *t)
{
    return GetBerryInfo(t->berry)->stageDuration * 60;
}

static bool8 IsWatered(const struct BerryTree *t)
{
    switch (t->stage)
    {
    case BERRY_STAGE_PLANTED:   return t->watered1;
    case BERRY_STAGE_SPROUTED:  return t->watered2;
    case BERRY_STAGE_TALLER:    return t->watered3;
    case BERRY_STAGE_FLOWERING: return t->watered4;
    }
    return TRUE;
}

static bool8 Water(struct BerryTree *t)
{
    if (t->berry == 0 || IsWatered(t))
        return FALSE;
    switch (t->stage)
    {
    case BERRY_STAGE_PLANTED:   t->watered1 = TRUE; break;
    case BERRY_STAGE_SPROUTED:  t->watered2 = TRUE; break;
    case BERRY_STAGE_TALLER:    t->watered3 = TRUE; break;
    case BERRY_STAGE_FLOWERING: t->watered4 = TRUE; break;
    }
    return TRUE;
}

static bool8 Needs(const struct BerryTree *t, u8 work)
{
    if (t->berry == 0 || t->stopGrowth)
        return FALSE;
    switch (work)
    {
    case FARM_WORK_WATER:
        return t->stage >= BERRY_STAGE_PLANTED && t->stage <= BERRY_STAGE_FLOWERING && !IsWatered(t);
    case FARM_WORK_TEND:
        return t->stage >= BERRY_STAGE_PLANTED && t->stage <= BERRY_STAGE_FLOWERING
            && (t->mpCare & FARM_CARE_TENDED) != t->stage && t->minutesUntilNextStage > 1;
    case FARM_WORK_NIBBLE:
        // left too long: more than half the time it stays ripe has gone by
        return t->stage == BERRY_STAGE_BERRIES && t->berryYield >= 2 && !(t->mpCare & FARM_CARE_NIBBLED)
            && t->minutesUntilNextStage <= StageMinutes(t) * 2;
    }
    return FALSE;
}

// A Garden Pokémon's work on a tree: water it, help it grow a little (a tenth of the stage, once a stage), or nibble
// one BERRY. FALSE: it didn't need it any more.
static bool8 Work(struct BerryTree *t, u8 work)
{
    if (!Needs(t, work))
        return FALSE;
    switch (work)
    {
    case FARM_WORK_WATER:
        return Water(t);
    case FARM_WORK_TEND:
    {
        u16 cut = StageMinutes(t) / 10;
        if (cut >= t->minutesUntilNextStage)
            cut = t->minutesUntilNextStage - 1;
        t->minutesUntilNextStage -= cut;
        t->mpCare = (t->mpCare & ~FARM_CARE_TENDED) | t->stage;
        return TRUE;
    }
    case FARM_WORK_NIBBLE:
        t->berryYield--;
        t->mpCare |= FARM_CARE_NIBBLED;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// The network

static void PackTree(u8 *p, const struct BerryTree *t)
{
    p[0] = t->berry;
    p[1] = t->stage | (t->stopGrowth << 7);
    p[2] = t->minutesUntilNextStage;
    p[3] = t->minutesUntilNextStage >> 8;
    p[4] = t->berryYield;
    p[5] = t->regrowthCount | (t->watered1 << 4) | (t->watered2 << 5) | (t->watered3 << 6) | (t->watered4 << 7);
    p[6] = t->mpCare;
}

static void UnpackTree(const u8 *p, struct BerryTree *t)
{
    memset(t, 0, sizeof(*t));
    t->berry = p[0];
    t->stage = p[1] & 0x7F;
    t->stopGrowth = p[1] >> 7;
    t->minutesUntilNextStage = p[2] | (p[3] << 8);
    t->berryYield = p[4];
    t->regrowthCount = p[5] & 15;
    t->watered1 = (p[5] >> 4) & 1;
    t->watered2 = (p[5] >> 5) & 1;
    t->watered3 = (p[5] >> 6) & 1;
    t->watered4 = (p[5] >> 7) & 1;
    t->mpCare = p[6];
    if (t->berry != 0 && !IsValidBerryType(t->berry))
        memset(t, 0, sizeof(*t));   // (never from a game of the same version)
}

static void Ask(u8 kind, u8 plot, u8 berry, u8 stage)
{
    u8 msg[6] = {MPR_FARM_ASK, kind, plot, berry, stage, sF.seq};
    s8 host = MP_RoomHostId();
    if (host >= 0)
        MP_SendRelay(host, msg, sizeof(msg));
}

// The host: our farm for everyone in the room, whenever it changes (and every few seconds, and at once for someone new)
static void SendState(void)
{
    u8 buf[STATE_BYTES], i;
    bool8 someoneNew = FALSE, anyone = FALSE;
    if (!MP_Lobby_IsCreator() || !MP_IsConnected() || gMpLocalId < 0 || gSaveBlock1Ptr == NULL)
    {
        memset(sF.knew, 0, sizeof(sF.knew));
        sF.lastSent[0] = 0;
        return;
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        bool8 here = i != gMpLocalId && gMpPlayers[i].present;
        if (here && !sF.knew[i])
            someoneNew = TRUE;
        sF.knew[i] = here;
        anyone |= here;
    }
    if (!anyone)
        return;
    sF.heartbeat++;
    if (++sF.sendTimer < 10 && !someoneNew)
        return;
    sF.sendTimer = 0;
    buf[0] = MPR_FARM_STATE;
    buf[1] = OwnPlots();
    for (i = 0; i < PLOTS; i++)
        PackTree(&buf[2 + i * TREE_BYTES], OwnTree(i));
    if (!someoneNew && sF.heartbeat < 60 * 5 && memcmp(buf, sF.lastSent, sizeof(buf)) == 0)
        return;
    memcpy(sF.lastSent, buf, sizeof(buf));
    sF.heartbeat = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sF.knew[i])
            MP_SendRelay(i, buf, sizeof(buf));
}

static void Answer(u8 to, u8 seq, bool8 ok, u8 plot, u8 berry, u8 count)
{
    u8 msg[6] = {MPR_FARM_ANSWER, seq, ok, plot, berry, count};
    MP_SendRelay(to, msg, sizeof(msg));
}

// The host: a friend's question about our farm
static void OnAsk(u8 from, const u8 *d)
{
    u8 kind = d[0], plot = d[1], berry = d[2], stage = d[3], seq = d[4];
    struct BerryTree *t;
    bool8 ok = FALSE;
    if (plot >= PLOTS || MP_Farm_IsFriends())
        return;
    t = OwnTree(plot);
    switch (kind)
    {
    case ASK_PLANT:
        ok = plot < OwnPlots() && t->stage == BERRY_STAGE_NO_BERRY && IsValidBerryType(berry);
        if (ok)
            PlantBerryTree(FIRST + plot, berry, BERRY_STAGE_PLANTED, TRUE);
        MPNet_Log("farm: player %d plants berry %u in plot %u: %s", from + 1, berry, plot, ok ? "yes" : "taken");
        Answer(from, seq, ok, plot, berry, 0);
        break;
    case ASK_PICK:
    {
        u8 count = t->berryYield;
        ok = t->stage == BERRY_STAGE_BERRIES && t->berry == berry && count > 0;
        if (ok)
            RemoveBerryTree(FIRST + plot);
        MPNet_Log("farm: player %d picks plot %u: %s (%u)", from + 1, plot, ok ? "yes" : "gone", ok ? count : 0);
        Answer(from, seq, ok, plot, berry, ok ? count : 0);
        break;
    }
    case ASK_WATER:
        // (the stage they saw: a late message never waters the next stage)
        if (t->berry == berry && t->stage == stage && Water(t))
            MPNet_Log("farm: player %d waters plot %u", from + 1, plot);
        break;
    case ASK_TEND:
    case ASK_NIBBLE:
        if (t->berry == berry && t->stage == stage
         && Work(t, kind == ASK_TEND ? FARM_WORK_TEND : FARM_WORK_NIBBLE))
            MPNet_Log("farm: player %d's Pokemon %s plot %u", from + 1, kind == ASK_TEND ? "tends" : "nibbles", plot);
        break;
    }
    sF.sendTimer = 10;   // (everyone sees it at once)
}

static void EndAsk(u16 result)
{
    sF.asking = FALSE;
    gSpecialVar_Result = result;
    ScriptContext_Enable();
}

void MP_Farm_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS)
        return;
    switch (data[0])
    {
    case MPR_FARM_STATE:
    {
        u8 i;
        if (len < STATE_BYTES || from != MP_RoomHostId())
            return;
        if (sF.heardFrom != from)
            MPNet_Log("farm: the host's farm: %u plots", data[1]);
        sF.heardFrom = from;
        sF.plots = data[1] > PLOTS ? PLOTS : data[1];
        for (i = 0; i < PLOTS; i++)
            UnpackTree(&data[2 + i * TREE_BYTES], &sF.trees[i]);
        break;
    }
    case MPR_FARM_ASK:
        if (len >= 6)
            OnAsk(from, data + 1);
        break;
    case MPR_FARM_ANSWER:
    {
        u8 seq = data[1], ok = data[2], plot = data[3], berry = data[4], count = data[5];
        if (len < 6 || !sF.asking || seq != sF.seq || plot != sF.askPlot || from != MP_RoomHostId())
            return;
        if (ok && sF.askKind == ASK_PICK)
        {
            // what we really got (a Pokémon may have nibbled one since we looked)
            sF.picked = TRUE;
            sF.pickedPlot = plot;
            sF.pickedBerry = berry;
            sF.pickedCount = count;
            gSpecialVar_0x8006 = count;
            ConvertIntToDecimalStringN(gStringVar2, count, STR_CONV_MODE_LEFT_ALIGN, CountDigits(count));
            GetBerryCountStringByBerryType(berry, gStringVar1, count);
        }
        MPNet_Log("farm: the host says %s to plot %u", ok ? "yes" : "no", plot);
        EndAsk(ok ? MP_FARM_GO : MP_FARM_TAKEN);
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// The BERRY tree script and berry.c

static s8 PlotInFront(void)
{
    u8 id = GetObjectEventBerryTreeId(gSelectedObjectEvent);
    return IsPlot(id) ? id - FIRST : -1;
}

// (A farm plot can change while its message box is up: a friend, or a friend's game for the host's farm, got there
// first. Our own farm: settled here; the host's: the host decides.)
u16 MP_Farm_AskPlant(void)
{
    s8 plot = PlotInFront();
    if (plot < 0)
        return MP_FARM_GO;
    if (Shown(plot)->stage != BERRY_STAGE_NO_BERRY)
        return MP_FARM_TAKEN;
    if (!MP_Farm_IsFriends())
        return MP_FARM_GO;
    sF.asking = TRUE;
    sF.askKind = ASK_PLANT;
    sF.askPlot = plot;
    sF.askTimer = 0;
    sF.seq++;
    Ask(ASK_PLANT, plot, ItemIdToBerryType(gSpecialVar_ItemId), 0);
    MPNet_Log("farm: asking the host to plant in plot %d", plot);
    return MP_FARM_WAIT;
}

u16 MP_Farm_AskPick(void)
{
    s8 plot = PlotInFront();
    struct BerryTree *t;
    if (plot < 0)
        return MP_FARM_GO;
    t = Shown(plot);
    if (t->stage != BERRY_STAGE_BERRIES || t->berryYield == 0 || !IsValidBerryType(t->berry))
        return MP_FARM_TAKEN;
    if (!MP_Farm_IsFriends())
        return MP_FARM_GO;
    // no room in the BAG: the game says so as always (nothing is asked, the BERRIES stay)
    if (!CheckBagHasSpace(BERRY_TO_ITEM(t->berry), t->berryYield))
        return MP_FARM_GO;
    sF.asking = TRUE;
    sF.askKind = ASK_PICK;
    sF.askPlot = plot;
    sF.askTimer = 0;
    sF.picked = FALSE;
    sF.seq++;
    Ask(ASK_PICK, plot, t->berry, t->stage);
    MPNet_Log("farm: asking the host to pick plot %d", plot);
    return MP_FARM_WAIT;
}

void MP_Farm_PickedCount(u8 id, u8 *berry, u8 *count)
{
    if (sF.picked && IsPlot(id) && id - FIRST == sF.pickedPlot)
    {
        *berry = sF.pickedBerry;
        *count = sF.pickedCount;
    }
}

void MP_Farm_Watered(u8 id)
{
    struct BerryTree *t;
    if (!IsPlot(id) || !MP_Farm_IsFriends())
        return;
    t = Shown(id - FIRST);
    Ask(ASK_WATER, id - FIRST, t->berry, t->stage);   // (no answer: seq stays the one a plant / pick waits on)
}

void MP_Farm_Changed(u8 id)
{
    if (IsPlot(id) && sF.picked && id - FIRST == sF.pickedPlot && Shown(id - FIRST)->berry == 0)
        sF.picked = FALSE;   // the BERRIES are in the BAG
}

// ---------------------------------------------------------------------------
// Garden Pokémon's work

void MP_Farm_PlotPos(u8 plot, s16 *x, s16 *y)
{
    u8 r = 0;
    while (r < ARRAY_COUNT(sGardenFarmRows) - 1 && plot >= sGardenFarmRows[r].count)
        plot -= sGardenFarmRows[r++].count;
    *x = GX(sGardenFarmRows[r].x + plot);
    *y = GY(sGardenFarmRows[r].y);
}

s8 MP_Farm_FindWork(u8 work, s16 x, s16 y, u32 busy)
{
    u8 i, n = MP_Farm_Plots();
    s8 best = -1;
    int bestD = 0x7FFF;
    if (!MP_Garden_IsHere())
        return -1;
    for (i = 0; i < n; i++)
    {
        s16 px, py;
        int d;
        if ((busy >> i) & 1 || !Needs(Shown(i), work))
            continue;
        MP_Farm_PlotPos(i, &px, &py);
        d = abs(px - x) + abs(py - y);
        if (d < bestD)
            bestD = d, best = i;
    }
    return best;
}

bool8 MP_Farm_DoWork(u8 work, u8 plot, u16 dex)
{
    static const char *const sWork[] = {"waters", "tends", "nibbles a berry of"};
    struct BerryTree *t;
    if (plot >= MP_Farm_Plots())
        return FALSE;
    t = Shown(plot);
    if (!Needs(t, work))
        return FALSE;
    if (MP_Farm_IsFriends())
    {
        // the host's farm: ask (and show it here at once; the host's next state settles it)
        Ask(work == FARM_WORK_WATER ? ASK_WATER : work == FARM_WORK_TEND ? ASK_TEND : ASK_NIBBLE, plot, t->berry, t->stage);
    }
    Work(t, work);
    MPNet_Log("farm: %u %s plot %u", dex, sWork[work], plot);
    return TRUE;
}

bool8 MP_Farm_IsFarmGround(s16 x, s16 y)
{
    u8 r;
    for (r = 0; r < ARRAY_COUNT(sGardenFarmRows); r++)
        if (x >= sGardenFarmRows[r].x - 1 && x <= sGardenFarmRows[r].x + sGardenFarmRows[r].count
         && y >= sGardenFarmRows[r].y - 1 && y <= sGardenFarmRows[r].y + 1)
            return TRUE;
    return FALSE;
}

// ---------------------------------------------------------------------------
// The map: soil framed as the routes frame theirs round the open plots, a BERRY tree on each

static void SetTile(s16 x, s16 y, u16 tile, bool8 draw)
{
    MapGridSetMetatileIdAt(GX(x), GY(y), tile);
    if (draw)
        CurrentMapDrawMetatileAt(GX(x), GY(y));
}

static void StampSoil(u8 plots, bool8 draw)
{
    u8 r, left = plots;
    for (r = 0; r < ARRAY_COUNT(sGardenFarmRows); r++)
    {
        const struct GardenFarmRow *row = &sGardenFarmRows[r];
        u8 open = left < row->count ? left : row->count, i;
        left -= open;
        for (i = 0; i <= row->count + 1; i++)
        {
            s16 x = row->x - 1 + i;
            u16 top = GARDEN_GRASS, low = GARDEN_GRASS;
            if (open > 0 && i == 0)
                top = GARDEN_SOIL_L, low = GARDEN_LIP_L;
            else if (open > 0 && i <= open)
                top = GARDEN_SOIL, low = GARDEN_LIP;
            else if (open > 0 && i == open + 1)
                top = GARDEN_SOIL_R, low = GARDEN_LIP_R;
            SetTile(x, row->y, top, draw);
            SetTile(x, row->y + 1, low, draw);
        }
    }
}

static void SetPlotFlags(u8 plots)
{
    u8 i;
    for (i = 0; i < PLOTS; i++)
    {
        if (i < plots)
            FlagClear(FLAG_TEMP_1 + i);
        else
            FlagSet(FLAG_TEMP_1 + i);
    }
}

// The BERRY tree objects: only on open plots (an object is in view, as TrySpawnObjectEvents would have it)
static void CheckObjects(u8 plots)
{
    u8 i, num = gSaveBlock1Ptr->location.mapNum, group = gSaveBlock1Ptr->location.mapGroup;
    s16 left = gSaveBlock1Ptr->pos.x - 2, right = gSaveBlock1Ptr->pos.x + MAP_OFFSET_W + 2;
    s16 top = gSaveBlock1Ptr->pos.y, bottom = gSaveBlock1Ptr->pos.y + MAP_OFFSET_H + 2;
    for (i = 0; i < PLOTS && i < gMapHeader.events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *tmpl = &gSaveBlock1Ptr->objectEventTemplates[i];
        bool8 exists = GetObjectEventIdByLocalIdAndMap(tmpl->localId, num, group) != OBJECT_EVENTS_COUNT;
        s16 x = tmpl->x + MAP_OFFSET, y = tmpl->y + MAP_OFFSET;
        if (i >= plots && exists)
            RemoveObjectEventByLocalIdAndMap(tmpl->localId, num, group);
        else if (i < plots && !exists && top <= y && y <= bottom && left <= x && x <= right)
            TrySpawnObjectEvent(tmpl->localId, num, group);
    }
}

void MP_Farm_OnMapLoad(void)
{
    u8 plots = MP_Farm_Plots();
    StampSoil(plots, FALSE);
    SetPlotFlags(plots);
    sF.shownPlots = plots;
    sF.checkTimer = 0;   // (a game loaded from a save brings its objects as they were: checked soon)
}

void MP_Farm_FrameUpdate(void)
{
    SendState();
    if (sF.asking && (++sF.askTimer >= ASK_TIMEOUT || !MP_Farm_IsFriends()))
    {
        MPNet_Log("farm: no answer from the host");
        EndAsk(MP_FARM_NO_ANSWER);
    }
    if (!MP_Garden_IsHere() || !MP_IsInOverworld() || sF.shownPlots < 0)
        return;
    if (MP_Farm_Plots() != sF.shownPlots && MP_IsLocalFree())
    {
        u8 plots = MP_Farm_Plots();
        MPNet_Log("farm: %u plots open (was %d)", plots, sF.shownPlots);
        StampSoil(plots, TRUE);
        SetPlotFlags(plots);
        CheckObjects(plots);
        sF.shownPlots = plots;
        MP_GardenDecor_CheckPlaces();
    }
    if (++sF.checkTimer >= 30 && MP_IsLocalFree())
    {
        sF.checkTimer = 0;
        CheckObjects(sF.shownPlots);
    }
}

// ---------------------------------------------------------------------------

void MP_Farm_Debug(const char *args)
{
    char cmd[16] = "";
    int a = 0, b = 0, c = 0;
    u8 i;
    sscanf(args, "%15s %d %d %d", cmd, &a, &b, &c);
    if (strcmp(cmd, "state") == 0)
    {
        printf("[test] farm plots %u friends %d heard %d shown %d\n", MP_Farm_Plots(), MP_Farm_IsFriends(), sF.heardFrom,
               sF.shownPlots);
        for (i = 0; i < MP_Farm_Plots(); i++)
        {
            struct BerryTree *t = Shown(i);
            printf("[test] farm plot %u: berry %u stage %u yield %u min %u watered %u%u%u%u care %u\n", i, t->berry,
                   t->stage, t->berryYield, t->minutesUntilNextStage, t->watered1, t->watered2, t->watered3, t->watered4,
                   t->mpCare);
        }
    }
    else if (strcmp(cmd, "plant") == 0)
    {
        // plant PLOT BERRY [STAGE]: on our own farm (not while it shows a friend's)
        if (a >= 0 && a < PLOTS && IsValidBerryType(b) && !MP_Farm_IsFriends())
            PlantBerryTree(FIRST + a, b, c >= BERRY_STAGE_PLANTED && c <= BERRY_STAGE_BERRIES ? c : BERRY_STAGE_PLANTED, TRUE);
        printf("[test] farm plant %d %d %d\n", a, b, c);
    }
    else if (strcmp(cmd, "grow") == 0)
    {
        // grow MINUTES: every tree of ours, as the clock would
        BerryTreeTimeUpdate(a);
        printf("[test] farm grow %d\n", a);
    }
    else if (strcmp(cmd, "ripe") == 0)
    {
        // ripe PLOT: our own plot's BERRIES left a long while (a Pokémon may nibble)
        if (a >= 0 && a < PLOTS && OwnTree(a)->stage == BERRY_STAGE_BERRIES)
            OwnTree(a)->minutesUntilNextStage = StageMinutes(OwnTree(a));
        printf("[test] farm ripe %d\n", a);
    }
}
