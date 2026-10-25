// Moemon Co-op: the MOéMON GARDEN (docs/POKEMON_GARDEN.md 11).
//
// Every POKéMON CENTER's second floor: a meadow inside the CENTER (data/maps/MoemonGarden, built by
// tools/moemon_garden/gen_garden.py). The CENTER's escalator leads up into it, and the escalator at
// its west edge leads back down to the CENTER you came from. The old 2F maps (the CABLE CLUB and the rest) stay in
// the game, unreached. After the first visit, START > GARDEN and the PC's MOéMON GARDEN row take you there from
// outdoors or a CENTER (never from a cave, a dungeon or the middle of a story moment), and the way down leads to the
// last POKéMON CENTER you visited.
//   - A place for every type (mp_garden_map.h sGardenZones: where each type's Pokémon go for what they love), and
//     every badge the group holds adds one (sGardenGrowth: a long grass field, a marsh, a boulder field, a dig field,
//     more shiny stones, a perch grove, a second flower bed, a stone circle): the group's travels leave their marks.
//   - One open meadow, alone or with friends: everyone's Pokémon and decorations are there together.
//   - The party lives here freely (mp_garden_life.c): needs, play, naps, bonds, decorations, WATCH mode.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_life.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "heal_location.h"
#include "safari_zone.h"
#include "trainer_hill.h"
#include "text.h"
#include "main.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/map_types.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#define GARDEN_WANT_GROWTH
#include "mp_garden_map.h"

#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

static struct
{
    bool8 here;
    u8 growth;              // the badges whose additions are on the map (sGardenGrowth up to there)
    u8 groupBadges;
    bool8 warpPending;      // quick travel, once the PC script / START menu is over
    bool8 welcomePending;
    const u8 *msgPending;   // why quick travel can't go (shown once the field is free)
    u8 msgTask;
} sG = {.msgTask = TASK_NONE};

bool8 MP_Garden_IsHere(void)
{
    return gSaveBlock1Ptr != NULL && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_MOEMON_GARDEN)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_MOEMON_GARDEN);
}

bool8 MP_Garden_Visited(void)
{
    return (MP_Life_GardenSave()->flags & MP_GARDEN_VISITED) != 0;
}

// ---------------------------------------------------------------------------
// The map: growth marks

// The group's journey: the most badges among the Trainers here
static u8 GroupBadges(void)
{
    u8 i, b = MP_Badges();
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState
         && gMpPlayers[i].state.mapGroup == MAP_GROUP(MAP_MOEMON_GARDEN) && gMpPlayers[i].state.mapNum == MAP_NUM(MAP_MOEMON_GARDEN)
         && gMpPlayers[i].state.badges > b)
            b = gMpPlayers[i].state.badges;
    return b;
}

static void SetTile(s16 x, s16 y, u16 tile, bool8 draw)
{
    MapGridSetMetatileIdAt(GX(x), GY(y), tile);
    if (draw)
        CurrentMapDrawMetatileAt(GX(x), GY(y));
}

// Every badge the group holds adds a place (gen_garden.py's GROWTH table, in badge order): stamp what's new
static void ApplyGrowth(bool8 draw)
{
    u16 i;
    u8 upTo;
    sG.groupBadges = GroupBadges();
    upTo = sG.groupBadges > 8 ? 8 : sG.groupBadges;
    if (upTo <= sG.growth)
        return;
    for (i = 0; i < ARRAY_COUNT(sGardenGrowth); i++)
        if (sGardenGrowth[i].badges > sG.growth && sGardenGrowth[i].badges <= upTo)
            SetTile(sGardenGrowth[i].x, sGardenGrowth[i].y, sGardenGrowth[i].tile, draw);
    for (i = sG.growth + 1; i <= upTo; i++)
        MPNet_Log("garden: growth %u", i);
    sG.growth = upTo;
}

// ELECTRIC's storm stone stands in the Garden from the start (a sprite: mp_garden_decor.c)
bool8 MP_Garden_StoneShown(void)
{
    return sG.here;
}

// The badges whose places are on the map now (which places the Garden's Pokémon can go to)
u8 MP_Garden_GrowthLevel(void)
{
    return sG.here ? sG.growth : 0;
}

static void NoteCenter(void);

void MP_Garden_OnMapLoad(void)
{
    NoteCenter();
    sG.here = MP_Garden_IsHere();
    if (!sG.here)
        return;
    MP_GardenLife_OnMapLoad();
    sG.growth = 0;
    ApplyGrowth(FALSE);
    MP_GardenDecor_CheckPlaces();
}

// ---------------------------------------------------------------------------
// Upstairs in every POKéMON CENTER

// The CENTERs: their 1F (whose escalator went up to the 2F), the 2F it went to, and the heal location the 1F sets
// (a save from before 1.16 knows its last CENTER only from that)
static const struct
{
    u16 center, upstairs;
    u8 heal;
} sCenters[] = {
    {MAP_OLDALE_TOWN_POKEMON_CENTER_1F, MAP_OLDALE_TOWN_POKEMON_CENTER_2F, HEAL_LOCATION_OLDALE_TOWN},
    {MAP_DEWFORD_TOWN_POKEMON_CENTER_1F, MAP_DEWFORD_TOWN_POKEMON_CENTER_2F, HEAL_LOCATION_DEWFORD_TOWN},
    {MAP_LAVARIDGE_TOWN_POKEMON_CENTER_1F, MAP_LAVARIDGE_TOWN_POKEMON_CENTER_2F, HEAL_LOCATION_LAVARIDGE_TOWN},
    {MAP_FALLARBOR_TOWN_POKEMON_CENTER_1F, MAP_FALLARBOR_TOWN_POKEMON_CENTER_2F, HEAL_LOCATION_FALLARBOR_TOWN},
    {MAP_VERDANTURF_TOWN_POKEMON_CENTER_1F, MAP_VERDANTURF_TOWN_POKEMON_CENTER_2F, HEAL_LOCATION_VERDANTURF_TOWN},
    {MAP_PACIFIDLOG_TOWN_POKEMON_CENTER_1F, MAP_PACIFIDLOG_TOWN_POKEMON_CENTER_2F, HEAL_LOCATION_PACIFIDLOG_TOWN},
    {MAP_PETALBURG_CITY_POKEMON_CENTER_1F, MAP_PETALBURG_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_PETALBURG_CITY},
    {MAP_SLATEPORT_CITY_POKEMON_CENTER_1F, MAP_SLATEPORT_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_SLATEPORT_CITY},
    {MAP_MAUVILLE_CITY_POKEMON_CENTER_1F, MAP_MAUVILLE_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_MAUVILLE_CITY},
    {MAP_RUSTBORO_CITY_POKEMON_CENTER_1F, MAP_RUSTBORO_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_RUSTBORO_CITY},
    {MAP_FORTREE_CITY_POKEMON_CENTER_1F, MAP_FORTREE_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_FORTREE_CITY},
    {MAP_LILYCOVE_CITY_POKEMON_CENTER_1F, MAP_LILYCOVE_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_LILYCOVE_CITY},
    {MAP_MOSSDEEP_CITY_POKEMON_CENTER_1F, MAP_MOSSDEEP_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_MOSSDEEP_CITY},
    {MAP_SOOTOPOLIS_CITY_POKEMON_CENTER_1F, MAP_SOOTOPOLIS_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_SOOTOPOLIS_CITY},
    {MAP_EVER_GRANDE_CITY_POKEMON_CENTER_1F, MAP_EVER_GRANDE_CITY_POKEMON_CENTER_2F, HEAL_LOCATION_EVER_GRANDE_CITY},
    {MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F, MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_2F, HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE},
    {MAP_BATTLE_FRONTIER_POKEMON_CENTER_1F, MAP_BATTLE_FRONTIER_POKEMON_CENTER_2F, HEAL_LOCATION_BATTLE_FRONTIER_OUTSIDE_EAST},
};

// Outdoor maps that are dungeons or story places all the same: no quick travel from them
static const u16 sNoTravelMaps[] = {
    MAP_PETALBURG_WOODS, MAP_JAGGED_PASS, MAP_MT_CHIMNEY, MAP_MT_PYRE_EXTERIOR, MAP_MT_PYRE_SUMMIT,
    MAP_SKY_PILLAR_OUTSIDE, MAP_SKY_PILLAR_TOP, MAP_SOUTHERN_ISLAND_EXTERIOR, MAP_SOUTHERN_ISLAND_INTERIOR,
    MAP_SAFARI_ZONE_NORTH, MAP_SAFARI_ZONE_NORTHEAST, MAP_SAFARI_ZONE_NORTHWEST, MAP_SAFARI_ZONE_SOUTH,
    MAP_SAFARI_ZONE_SOUTHEAST, MAP_SAFARI_ZONE_SOUTHWEST,
};

static s8 CenterAt(u8 group, u8 num)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sCenters); i++)
        if (MAP_GROUP(sCenters[i].center) == group && MAP_NUM(sCenters[i].center) == num)
            return i;
    return -1;
}

static s8 CenterBelow(u8 group, u8 num)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sCenters); i++)
        if (MAP_GROUP(sCenters[i].upstairs) == group && MAP_NUM(sCenters[i].upstairs) == num)
            return i;
    return -1;
}

// Its escalator: the 1F warp that went up (from the map's own data)
static s8 EscalatorOf(u8 center)
{
    const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(sCenters[center].center), MAP_NUM(sCenters[center].center));
    s8 i;
    for (i = 0; i < h->events->warpCount; i++)
        if (h->events->warps[i].mapGroup == MAP_GROUP(sCenters[center].upstairs)
         && h->events->warps[i].mapNum == MAP_NUM(sCenters[center].upstairs))
            return i;
    return WARP_ID_NONE;
}

// On every map load: in a CENTER, it's the last one visited
static void NoteCenter(void)
{
    struct MpGardenSave *gs;
    s8 c;
    if (gSaveBlock1Ptr == NULL)
        return;
    c = CenterAt(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (c < 0)
        return;
    gs = MP_Life_GardenSave();
    gs->lastPcGroup = MAP_GROUP(sCenters[c].center);
    gs->lastPcNum = MAP_NUM(sCenters[c].center);
    gs->lastPcWarp = EscalatorOf(c);
    gs->flags |= MP_GARDEN_LASTPC;
}

// The CENTER the way down leads to: the last one visited (-1: none known)
static s8 ReturnCenter(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    const struct WarpData *w = &gSaveBlock1Ptr->lastHealLocation;
    s8 i;
    if (gs->flags & MP_GARDEN_LASTPC)
    {
        i = CenterAt(gs->lastPcGroup, gs->lastPcNum);
        if (i >= 0 && EscalatorOf(i) != WARP_ID_NONE)
            return i;
    }
    // (a save from before 1.16: the CENTER the game would rush you to after a loss)
    for (i = 0; i < (s8)ARRAY_COUNT(sCenters); i++)
    {
        const struct HealLocation *h = GetHealLocation(sCenters[i].heal);
        if (h != NULL && w->mapGroup == h->mapGroup && w->mapNum == h->mapNum && w->x == h->x && w->y == h->y
         && EscalatorOf(i) != WARP_ID_NONE)
            return i;
    }
    return -1;
}

// field_control_avatar.c SetupWarp, after it set the destination of a warp the player stepped on: going up a
// CENTER's escalator leads into the Garden; the Garden's escalator leads down to the CENTER you came from
void MP_Garden_RedirectWarp(s8 destGroup, s8 destNum)
{
    if (gSaveBlock1Ptr == NULL)
        return;
    if (MP_Garden_IsHere())
    {
        s8 c = ReturnCenter();
        if (c >= 0)
        {
            SetWarpDestination(MAP_GROUP(sCenters[c].center), MAP_NUM(sCenters[c].center), EscalatorOf(c), -1, -1);
            MPNet_Log("garden: down to center %d", c);
        }
        else
        {
            SetWarpDestinationToLastHealLocation();   // (never: a CENTER is always known by now; still somewhere real)
            MPNet_Log("garden: down to the last heal location");
        }
        return;
    }
    if (CenterBelow(destGroup, destNum) >= 0)
    {
        NoteCenter();
        SetWarpDestination(MAP_GROUP(MAP_MOEMON_GARDEN), MAP_NUM(MAP_MOEMON_GARDEN), 0, -1, -1);
        MPNet_Log("garden: up the escalator of center %d", CenterAt(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum));
    }
}

// ---------------------------------------------------------------------------
// Coming and going

static const u8 sText_Welcome[] = _("The POKéMON CENTER's second floor: the\nMOéMON GARDEN. Your MOéMON can live\pfreely here, in every CENTER. The GARDEN\nBOARD has more. From now on, START >\pGARDEN brings you here from outdoors.");
static const u8 sText_NotNow[] = _("This isn't the time to go to the\nMOéMON GARDEN.");
static const u8 sText_NotHere[] = _("The way to the MOéMON GARDEN can't be\nfound from here. Try from outdoors or a\nPOKéMON CENTER.");
static const u8 sText_NoCenter[] = _("The MOéMON GARDEN is upstairs in every\nPOKéMON CENTER. Visit one first!");

static void Task_Message(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (data[0])
    {
    case 0:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            data[0] = 1;
        }
        break;
    case 1:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            sG.msgTask = TASK_NONE;
        }
        break;
    }
}

static void ShowMessage(const u8 *text)
{
    if (sG.msgTask != TASK_NONE)
        return;
    LockPlayerFieldControls();
    StringExpandPlaceholders(gStringVar4, text);
    MP_ShowFieldMessageFitted(gStringVar4);
    sG.msgTask = CreateTask(Task_Message, 80);
}

// The first visit: a memory for everyone in the party, and quick travel from then on
static void FirstVisit(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i;
    gs->flags |= MP_GARDEN_VISITED;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct MpLifeRec *rec = MP_Life_Get(&gPlayerParty[i].box, TRUE);
        if (rec)
        {
            MP_Life_Remember(rec, MEM_GARDEN, 180, TIER_PART, 0, 0);
            MP_Life_Feel(rec, EMO_JOY, 40);
        }
    }
    sG.welcomePending = TRUE;
    MPNet_Log("garden: first visit");
}

static bool8 InList(const u16 *maps, u8 count)
{
    u8 i;
    for (i = 0; i < count; i++)
        if (MAP_GROUP(maps[i]) == gSaveBlock1Ptr->location.mapGroup && MAP_NUM(maps[i]) == gSaveBlock1Ptr->location.mapNum)
            return TRUE;
    return FALSE;
}

// Why quick travel can't go from here (NULL: it can). From the START menu: only outdoors where flying would work
// (not a cave, the sea floor or a building) and not a dungeon-like outdoor map, or in a CENTER; from a PC: the PC
// is in a CENTER or a home. Never in the middle of something (a story moment the game is running, the SAFARI ZONE,
// a BATTLE FRONTIER challenge, TRAINER HILL, a co-op battle, riding piggyback), and only once a CENTER is known,
// so the way down always leads somewhere real.
static const u8 *QuickTravelBlocked(bool8 fromPc)
{
    if (!FlagGet(FLAG_SYS_POKEMON_GET) || GetSafariZoneFlag() || InBattlePyramid_() || InBattlePike()
     || InTrainerHillChallenge() || MP_Coop_IsBusy() || MP_Piggy_IsRiding())
        return sText_NotNow;
    if (!fromPc && CenterAt(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) < 0
     && (!Overworld_MapTypeAllowsTeleportAndFly(gMapHeader.mapType) || InList(sNoTravelMaps, ARRAY_COUNT(sNoTravelMaps))))
        return sText_NotHere;
    if (ReturnCenter() < 0)
        return sText_NoCenter;
    return NULL;
}

// START > GARDEN is there once the Garden's been found (and not while you're in it)
bool8 MP_Garden_QuickTravelShown(void)
{
    return gSaveBlock1Ptr != NULL && MP_Garden_Visited() && !MP_Garden_IsHere();
}

// START > GARDEN (fromPc FALSE) and the PC's MOéMON GARDEN row (TRUE): off once the menu or the PC is closed, or
// a message saying why not
void MP_Garden_QuickTravel(bool8 fromPc)
{
    const u8 *why = QuickTravelBlocked(fromPc);
    if (why != NULL)
    {
        sG.msgPending = why;
        MPNet_Log("garden: quick travel not from here (%s)", why == sText_NotNow ? "not now" : why == sText_NotHere ? "place" : "no center");
        return;
    }
    sG.warpPending = TRUE;
}

static void GoTo(u8 group, u8 num, s16 x, s16 y)
{
    SetWarpDestination(group, num, WARP_ID_NONE, x, y);
    LockPlayerFieldControls();
    DoWarp();
}

// Standing where nobody could (a save from an older Garden whose layout changed): back to the landing
static bool8 StuckHere(void)
{
    static u8 sStuckFrames;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = me->currentCoords.x - MAP_OFFSET, y = me->currentCoords.y - MAP_OFFSET;
    bool8 stuck;
    if (x >= GARDEN_PAD_X0 && x <= GARDEN_PAD_X1 && y >= GARDEN_PAD_Y0 && y <= GARDEN_PAD_Y1)
        stuck = FALSE;
    else if (x < 1 || x >= GARDEN_W - 1 || y < 1 || y >= GARDEN_H - 1)
        stuck = TRUE;
    else
        // (a jump down the ledge passes over the ledge's tile: never stuck there, nor while on the move)
        stuck = MapGridGetCollisionAt(me->currentCoords.x, me->currentCoords.y) != 0
             && !MetatileBehavior_IsJumpSouth(MapGridGetMetatileBehaviorAt(me->currentCoords.x, me->currentCoords.y))
             && gPlayerAvatar.tileTransitionState == T_NOT_MOVING
             && !(me->heldMovementActive && !me->heldMovementFinished);   // (the engine leaves heldMovementActive set once a step is done)
    // standing there a moment first (half a second): a step or a jump in progress never counts
    if (!stuck)
        sStuckFrames = 0;
    else if (sStuckFrames < 30)
        sStuckFrames++;
    return sStuckFrames >= 30;
}

void MP_Garden_Update(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    if (!MP_IsInOverworld() || gSaveBlock1Ptr == NULL)
        return;
    if (sG.msgPending != NULL && MP_IsLocalFree())
    {
        ShowMessage(sG.msgPending);
        sG.msgPending = NULL;
        return;
    }
    if (sG.warpPending && MP_IsLocalFree())
    {
        s8 c = ReturnCenter();
        sG.warpPending = FALSE;
        if (c >= 0)
        {
            // the way down leads to this CENTER (kept, for a save from before 1.16 too)
            gs->lastPcGroup = MAP_GROUP(sCenters[c].center);
            gs->lastPcNum = MAP_NUM(sCenters[c].center);
            gs->lastPcWarp = EscalatorOf(c);
            gs->flags |= MP_GARDEN_LASTPC;
        }
        MPNet_Log("garden: quick travel there (back to center %d)", c);
        GoTo(MAP_GROUP(MAP_MOEMON_GARDEN), MAP_NUM(MAP_MOEMON_GARDEN), GARDEN_ENTRY_X, GARDEN_ENTRY_Y);
        return;
    }
    if (MP_Garden_IsHere() != sG.here)
        MP_Garden_OnMapLoad();   // (in case a map was entered without a full load)
    if (!sG.here)
        return;
    gs->flags &= ~MP_GARDEN_SHORTCUT;   // (1.15's flag)
    if (!(gs->flags & MP_GARDEN_VISITED))
        FirstVisit();
    if (MP_IsLocalFree() && StuckHere())
    {
        MPNet_Log("garden: stuck, back to the landing");
        GoTo(MAP_GROUP(MAP_MOEMON_GARDEN), MAP_NUM(MAP_MOEMON_GARDEN), GARDEN_ENTRY_X, GARDEN_ENTRY_Y);
        return;
    }
    if (sG.welcomePending && MP_IsLocalFree())
    {
        sG.welcomePending = FALSE;
        ShowMessage(sText_Welcome);
    }
    if (GroupBadges() != sG.groupBadges)
    {
        ApplyGrowth(TRUE);
        MP_GardenDecor_CheckPlaces();
    }
}

void MP_Garden_FrameUpdate(void)
{
    MP_Garden_Update();
    MP_GardenLife_Update();
}

// ---------------------------------------------------------------------------
// The GARDEN BOARD (a sign by the pond)

enum { BOARD_DECORATE, BOARD_WATCH, BOARD_CANCEL, BOARD_COUNT };
static const char *const sBoardRows[BOARD_COUNT] = {"DECORATE", "WATCH", "CANCEL"};

static struct
{
    u8 rows[BOARD_COUNT];
    u8 count;
    u16 cursor;
} sBoard;

static void DrawBoard(void)
{
    u8 i;
    MpUi_Begin("MOéMON GARDEN");
    for (i = 0; i < sBoard.count; i++)
        MpUi_RowAscii(i, i == sBoard.cursor, sBoardRows[sBoard.rows[i]], NULL);
    MpUi_End();
    MpUi_HelpAscii("{A} CHOOSE   {B} CANCEL");
}

void MP_Garden_StartWatch(void);
void MP_Garden_OpenDecorate(void);

static void Task_Board(u8 taskId)
{
    u8 pick;
    if (MpUi_MoveCursor(&sBoard.cursor, sBoard.count))
    {
        DrawBoard();
        return;
    }
    if (!JOY_NEW(A_BUTTON | B_BUTTON))
        return;
    pick = JOY_NEW(B_BUTTON) ? BOARD_CANCEL : sBoard.rows[sBoard.cursor];
    PlaySE(SE_SELECT);
    MpUi_Close();
    DestroyTask(taskId);
    switch (pick)
    {
    case BOARD_DECORATE:
        MP_Garden_OpenDecorate();   // its own menu; it ends the script
        return;
    case BOARD_WATCH:
        ScriptContext_Enable();
        MP_Garden_StartWatch();
        return;
    }
    ScriptContext_Enable();
}

void MP_Garden_OpenBoard(void)
{
    memset(&sBoard, 0, sizeof(sBoard));
    sBoard.rows[sBoard.count++] = BOARD_DECORATE;
    sBoard.rows[sBoard.count++] = BOARD_WATCH;
    sBoard.rows[sBoard.count++] = BOARD_CANCEL;
    PlaySE(SE_WIN_OPEN);
    MpUi_Open();
    CreateTask(Task_Board, 80);
    DrawBoard();
    MPNet_Log("garden: board");
}

void MP_Garden_Debug(const char *args)
{
    char cmd[32] = "";
    int a = 0;
    sscanf(args, "%31s %d", cmd, &a);
    if (strcmp(cmd, "state") == 0)
    {
        struct MpGardenSave *gs = MP_Life_GardenSave();
        printf("[test] garden here %d growth %u badges %u visited %d lastpc %d %u.%u warp %u return %d\n", sG.here,
               sG.growth, sG.groupBadges, MP_Garden_Visited(), (gs->flags & MP_GARDEN_LASTPC) != 0, gs->lastPcGroup,
               gs->lastPcNum, gs->lastPcWarp, ReturnCenter());
    }
    else if (strcmp(cmd, "travel") == 0)
    {
        // travel 1: the PC's MOéMON GARDEN row; travel 0: START > GARDEN
        MP_Garden_QuickTravel(a != 0);
        printf("[test] garden travel %d\n", a);
    }
    else if (strcmp(cmd, "forget") == 0)
    {
        // forget: no CENTER known (a save from before 1.16: only the last heal location tells)
        MP_Life_GardenSave()->flags &= ~MP_GARDEN_LASTPC;
        printf("[test] garden forget\n");
    }
    else if (strcmp(cmd, "actors") == 0)
    {
        void MP_GardenLife_Debug(const char *args);
        MP_GardenLife_Debug(args);
    }
    else if (strcmp(cmd, "force") == 0)
    {
        // force ACTOR ACT: that Garden Pokémon's next activity (its need raised for it)
        void MP_GardenLife_DebugForce(u8 actor, u8 act);
        int b = 0;
        sscanf(args, "%31s %d %d", cmd, &a, &b);
        MP_GardenLife_DebugForce(a, b);
        printf("[test] garden force %d %d\n", a, b);
    }
    else if (strcmp(cmd, "place") == 0)
    {
        // place ACTOR NEED: that Garden Pokémon goes to its type's place for TNEED_* NEED
        void MP_GardenLife_DebugPlace(u8 actor, u8 need);
        int b = 0;
        sscanf(args, "%31s %d %d", cmd, &a, &b);
        MP_GardenLife_DebugPlace(a, b);
    }
    else if (strcmp(cmd, "rare") == 0)
    {
        // rare ACTOR KIND: a rare moment (LIFE_RARE_*) with everything lined up for it
        void MP_GardenLife_DebugRare(u8 actor, u8 kind);
        int b = 0;
        sscanf(args, "%31s %d %d", cmd, &a, &b);
        MP_GardenLife_DebugRare(a, b);
    }
    else if (strcmp(cmd, "pursuit") == 0)
    {
        // pursuit ACTOR LEVEL: its pursuit (one that fits it), at that level
        void MP_GardenLife_DebugPursuit(u8 actor, u8 level);
        int b = 0;
        sscanf(args, "%31s %d %d", cmd, &a, &b);
        MP_GardenLife_DebugPursuit(a, b);
    }
    else if (strcmp(cmd, "talk") == 0)
    {
        void MP_GardenLife_DebugTalk(u8 actor);
        MP_GardenLife_DebugTalk(a);
        printf("[test] garden talk %d\n", a);
    }
    else if (strcmp(cmd, "decor") == 0)
    {
        void MP_GardenDecor_Debug(const char *args);
        MP_GardenDecor_Debug(args + 6);
    }
    else if (strcmp(cmd, "watch") == 0)
    {
        void MP_GardenWatch_Debug(void);
        MP_GardenWatch_Debug();
    }
    else if (strcmp(cmd, "pc") == 0)
    {
        // pc: as if A were pressed on a Pokémon Center's PC
        extern const u8 EventScript_PC[];
        ScriptContext_SetupScript(EventScript_PC);
        printf("[test] garden pc\n");
    }
    else if (strcmp(cmd, "board") == 0)
    {
        // board: as if A were pressed on the GARDEN BOARD (its script)
        extern const u8 MoemonGarden_EventScript_Board[];
        ScriptContext_SetupScript(MoemonGarden_EventScript_Board);
        printf("[test] garden board\n");
    }
}
