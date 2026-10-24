// Moemon Co-op: the MOéMON GARDEN (docs/POKEMON_GARDEN.md 11).
//
// PROF. BIRCH's research meadow, a real map west of LITTLEROOT (data/maps/MoemonGarden, built by
// tools/moemon_garden/gen_map.py from Littleroot's and Petalburg's own pieces). Walk there the first time;
// after that the PC offers a shortcut there and back.
//   - It grows with the journey: patches appear as the group's badges grow (tall grass, puddles, a dig patch,
//     a perch grove, a storm stone): the group's travels leave their marks.
//   - Alone it's a full small Garden (the east part); with friends there, the hedge opens to the extra meadow.
//   - The party lives here freely (mp_garden_life.c): needs, play, naps, bonds, decorations, WATCH mode.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_life.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "text.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "mp_garden_map.h"

#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

enum { GROW_TALL_GRASS, GROW_PUDDLES, GROW_SAND, GROW_GROVE, GROW_STONE, GROW_COUNT };
static const u8 sGrowBadges[GROW_COUNT] = {
    [GROW_TALL_GRASS] = 1,   // through PETALBURG WOODS: foliage
    [GROW_PUDDLES] = 2,      // SLATEPORT's sea air: a damp corner
    [GROW_SAND] = 4,         // the desert: a dig patch
    [GROW_STONE] = 5,        // the WEATHER INSTITUTE: a storm stone
    [GROW_GROVE] = 6,        // FORTREE: a perch grove
};

static struct
{
    bool8 here;
    bool8 hedgeOpen;
    u8 growth;              // GROW_* bits stamped on the map
    u8 groupBadges;
    bool8 warpPending;      // the PC's shortcut, once the PC script is over
    bool8 welcomePending;
    bool8 wasHere;          // in the Garden last frame
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
// The map: growth marks and the hedge

static bool8 FriendsHere(void)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState
         && gMpPlayers[i].state.mapGroup == MAP_GROUP(MAP_MOEMON_GARDEN) && gMpPlayers[i].state.mapNum == MAP_NUM(MAP_MOEMON_GARDEN))
            return TRUE;
    return FALSE;
}

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

static void StampRect(s16 x0, s16 y0, s16 w, s16 h, u16 tile, bool8 draw)
{
    s16 x, y;
    for (y = y0; y < y0 + h; y++)
        for (x = x0; x < x0 + w; x++)
            SetTile(x, y, tile, draw);
}

static void Grow(u8 which, bool8 draw)
{
    s16 x, y;
    switch (which)
    {
    case GROW_TALL_GRASS:
        StampRect(GROWTH_TALL_GRASS_X, GROWTH_TALL_GRASS_Y, GROWTH_TALL_GRASS_W, GROWTH_TALL_GRASS_H, GARDEN_TALL_GRASS, draw);
        break;
    case GROW_PUDDLES:
        for (y = 0; y < 3; y++)
            for (x = 0; x < 3; x++)
                SetTile(GROWTH_PUDDLES_X + x, GROWTH_PUDDLES_Y + y, sGardenPuddles[y][x], draw);
        break;
    case GROW_SAND:
        StampRect(GROWTH_SAND_X, GROWTH_SAND_Y, GROWTH_SAND_W, GROWTH_SAND_H, GARDEN_SAND, draw);
        break;
    case GROW_GROVE:
        for (x = 0; x < GROWTH_GROVE_W; x += 2)
            for (y = 0; y < 2; y++)
            {
                SetTile(GROWTH_GROVE_X + x, GROWTH_GROVE_Y + y, sGardenTree[y][0], draw);
                SetTile(GROWTH_GROVE_X + x + 1, GROWTH_GROVE_Y + y, sGardenTree[y][1], draw);
            }
        break;
    case GROW_STONE:
        break;   // a sprite (mp_garden_life.c)
    }
    sG.growth |= 1 << which;
    MPNet_Log("garden: growth %u", which);
}

static void ApplyGrowth(bool8 draw)
{
    u8 i;
    sG.groupBadges = GroupBadges();
    for (i = 0; i < GROW_COUNT; i++)
        if (!(sG.growth & (1 << i)) && sG.groupBadges >= sGrowBadges[i])
            Grow(i, draw);
}

bool8 MP_Garden_StoneShown(void)
{
    return (sG.growth & (1 << GROW_STONE)) != 0;
}

static bool8 SomeoneOnHedge(void)
{
    u8 i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *o = &gObjectEvents[i];
        if (o->active && (o->currentCoords.x == GX(GARDEN_HEDGE_X) || o->currentCoords.x == GX(GARDEN_HEDGE_X + 1)))
            return TRUE;
    }
    return FALSE;
}

static void SetHedge(bool8 open, bool8 draw)
{
    s16 y;
    for (y = GARDEN_Y0; y <= GARDEN_Y1; y++)
    {
        u16 a, b;
        if (open)
        {
            a = (y % 5) == 2 ? sGardenHedgeOpen[1] : sGardenHedgeOpen[0];
            b = (y % 7) == 4 ? sGardenHedgeOpen[1] : sGardenHedgeOpen[0];
        }
        else
        {
            // the forest pattern of the generator: top halves on even rows, bottom halves on odd ones
            a = (y % 2) == 0 ? 0x1D4 : 0x1DC;
            b = a + 1;
            a |= (3 << 10) | (GARDEN_GRASS & 0xF000);
            b |= (3 << 10) | (GARDEN_GRASS & 0xF000);
        }
        SetTile(GARDEN_HEDGE_X, y, a, draw);
        SetTile(GARDEN_HEDGE_X + 1, y, b, draw);
    }
    sG.hedgeOpen = open;
    MPNet_Log("garden: hedge %s", open ? "open" : "closed");
}

void MP_Garden_OnMapLoad(void)
{
    sG.here = MP_Garden_IsHere();
    if (!sG.here)
        return;
    MP_GardenLife_OnMapLoad();
    sG.growth = 0;
    ApplyGrowth(FALSE);
    if (FriendsHere())
        SetHedge(TRUE, FALSE);
    else
        sG.hedgeOpen = FALSE;
}

// ---------------------------------------------------------------------------
// Coming and going

static const u8 sText_Welcome[] = _("PROF. BIRCH's research meadow: the\nMOéMON GARDEN. Your MOéMON can live\pfreely here. The GARDEN BOARD has more,\nand your PC knows the way back here.");

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

// The first visit: a memory for everyone in the party, and the shortcut from then on
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

void MP_Garden_ShortcutFromPC(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    gs->retGroup = gSaveBlock1Ptr->location.mapGroup;
    gs->retNum = gSaveBlock1Ptr->location.mapNum;
    gs->retX = me->currentCoords.x - MAP_OFFSET;
    gs->retY = me->currentCoords.y - MAP_OFFSET;
    sG.warpPending = TRUE;
}

static void GoTo(u8 group, u8 num, s16 x, s16 y)
{
    SetWarpDestination(group, num, WARP_ID_NONE, x, y);
    LockPlayerFieldControls();
    DoWarp();
}

void MP_Garden_Update(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    if (!MP_IsInOverworld() || gSaveBlock1Ptr == NULL)
        return;
    if (sG.warpPending && MP_IsLocalFree())
    {
        sG.warpPending = FALSE;
        gs->flags |= MP_GARDEN_SHORTCUT;
        MPNet_Log("garden: shortcut there");
        GoTo(MAP_GROUP(MAP_MOEMON_GARDEN), MAP_NUM(MAP_MOEMON_GARDEN), GARDEN_ENTRY_X, GARDEN_ENTRY_Y);
        return;
    }
    if (MP_Garden_IsHere() != sG.here)
        MP_Garden_OnMapLoad();   // (a connection crossed without a full load)
    if (!sG.here)
    {
        // walked off elsewhere (not just arriving by the shortcut): the board's way back is only for the shortcut
        if (sG.wasHere)
            gs->flags &= ~MP_GARDEN_SHORTCUT;
        sG.wasHere = FALSE;
        return;
    }
    sG.wasHere = TRUE;
    if (!(gs->flags & MP_GARDEN_VISITED))
        FirstVisit();
    if (sG.welcomePending && MP_IsLocalFree())
    {
        sG.welcomePending = FALSE;
        ShowMessage(sText_Welcome);
    }
    // friends here: the hedge opens to the extra meadow (and closes when they've gone and nobody stands there)
    if (FriendsHere() != sG.hedgeOpen && (!sG.hedgeOpen || !SomeoneOnHedge()))
        SetHedge(!sG.hedgeOpen, TRUE);
    if (GroupBadges() != sG.groupBadges)
        ApplyGrowth(TRUE);
}

void MP_Garden_FrameUpdate(void)
{
    MP_Garden_Update();
    MP_GardenLife_Update();
}

// ---------------------------------------------------------------------------
// The GARDEN BOARD (a sign by the pond)

enum { BOARD_DECORATE, BOARD_WATCH, BOARD_BACK, BOARD_CANCEL, BOARD_COUNT };
static const char *const sBoardRows[BOARD_COUNT] = {"DECORATE", "WATCH", "GO BACK", "CANCEL"};

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
    case BOARD_BACK:
    {
        struct MpGardenSave *gs = MP_Life_GardenSave();
        gs->flags &= ~MP_GARDEN_SHORTCUT;
        ScriptContext_Enable();
        MPNet_Log("garden: shortcut back");
        GoTo(gs->retGroup, gs->retNum, gs->retX, gs->retY);
        return;
    }
    }
    ScriptContext_Enable();
}

void MP_Garden_OpenBoard(void)
{
    memset(&sBoard, 0, sizeof(sBoard));
    sBoard.rows[sBoard.count++] = BOARD_DECORATE;
    sBoard.rows[sBoard.count++] = BOARD_WATCH;
    if (MP_Life_GardenSave()->flags & MP_GARDEN_SHORTCUT)
        sBoard.rows[sBoard.count++] = BOARD_BACK;
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
        printf("[test] garden here %d hedge %d growth %02x badges %u visited %d shortcut %d\n", sG.here, sG.hedgeOpen,
               sG.growth, sG.groupBadges, MP_Garden_Visited(), (MP_Life_GardenSave()->flags & MP_GARDEN_SHORTCUT) != 0);
    }
    else if (strcmp(cmd, "shortcut") == 0)
    {
        MP_Garden_ShortcutFromPC();
        printf("[test] garden shortcut\n");
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
