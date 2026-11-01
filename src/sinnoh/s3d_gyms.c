// Moemon Kai: SINNOH's gyms that move, as Platinum's (its overlay008/gym_features.c: rewritten for the game's grid and
// the 3D engine; where the parts lie: src/data/sinnoh_gyms.h, tools/sinnoh/gen_gyms.py). The 3D engine draws the
// props Platinum loads at run time (S3D_DynProp: the pack's named meshes, version 4) and asks here for the moving
// ground's heights; the field engine asks here about each step (S3D_Gyms_StepBlocked). Each gym's state is a SINNOH
// variable, so it stays as the player left it.
//
// PASTORIA: the pool's water stands at one of three levels (0, 32 or 64 units: two and four tiles' height); a button
// moves it (orange low, green middle, blue high: the button stepped on, Sinnoh_EventScript_PastoriaButton). In the
// pool (Platinum's plate: tiles 1-25 x 2-39) the ground is the floor or, where the water stands above the floor, the
// water's surface: a floor that floats; on deep water (behavior 0x59) where the water stands above the floor nobody
// walks. A step between two heights 20 or more apart is blocked (Platinum's rule), and the gates (0x56-0x58) open at
// their own level.
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "s3d.h"
#include "m4a.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "mp.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include <string.h>
struct SunyshoreGear { u16 model; u8 x, y, z, start, clockwise, upright; };
#include "../data/sinnoh_gyms.h"

#define VAR_SINNOH_PASTORIA 0x52F3   // the water's level + 1 (0: Platinum's start, the middle)
#define VAR_SINNOH_SUNYSHORE 0x52F4  // the gears' quarter turn in the room the player is in

enum { GYM_NONE, GYM_PASTORIA, GYM_SUNYSHORE };

static struct
{
    u8 gym;
    s16 lastX, lastY;      // the player's tile last frame (a step onto a button)
    float water, waterTo;  // PASTORIA: the water's height now and where it's going
    float ox, oz;          // the map's corner in its matrix's world
    u8 room;               // SUNYSHORE: the room (0-2)
    float turned, turnTo;  // ... the gears' turn now and where it's going (degrees from the quarter turn's start)
    s8 turnWay;
} sG;

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

// ------------------------------------------------------------------------------------------------------ PASTORIA
#define PLATE_X0 1
#define PLATE_Y0 2
#define PLATE_X1 25
#define PLATE_Y1 39

static u8 PastoriaLevel(void)
{
    u16 v = VarGet(VAR_SINNOH_PASTORIA);
    return v >= 1 && v <= 3 ? v - 1 : 1;
}

static bool8 InPlate(s32 x, s32 y)
{
    return x >= PLATE_X0 && x <= PLATE_X1 && y >= PLATE_Y0 && y <= PLATE_Y1;
}

static bool8 PastoriaDeep(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPastoriaDeep); i++)
        if (sPastoriaDeep[i][0] == x && sPastoriaDeep[i][1] == y)
            return TRUE;
    return FALSE;
}

static s32 PastoriaGate(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPastoriaGates); i++)
        if (sPastoriaGates[i][0] == x && sPastoriaGates[i][1] == y)
            return sPastoriaGates[i][2];
    return -1;
}

// A pool tile's ground with the water at WATER: the floor, or the water's surface where it stands higher (as
// Platinum: the floor where the water is lower; else the floor or the water, whichever is nearer NEAR); NaN: not the
// pool's
static float PoolHeight(s32 x, s32 y, float near, float water)
{
    float floor;
    if (!InPlate(x, y))
        return NAN;
    floor = S3D_TileFloor(x, y, near);
    if (floor != floor)
        return water;
    if (water <= floor || fabsf(floor - near) <= fabsf(water - near))
        return floor;
    return water;
}

static float PastoriaHeight(s32 x, s32 y, float near)
{
    return PoolHeight(x, y, near, sG.water);
}

static float HeightHook(s32 x, s32 y, float near)
{
    return sG.gym == GYM_PASTORIA ? PastoriaHeight(x, y, near) : NAN;
}

static void PastoriaGates(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPastoriaGates); i++)
        MapGridSetMetatileImpassabilityAt(sPastoriaGates[i][0] + MAP_OFFSET, sPastoriaGates[i][1] + MAP_OFFSET,
                                          sPastoriaGates[i][2] != PastoriaLevel());
}

static void PastoriaProp(void)
{
    S3D_DynProp(0, S3D_MODEL_PASTORIA_GYM_WATER_FLOOR, sG.ox + 256, sG.water, sG.oz + 256, 0);
}

static void PastoriaLoad(void)
{
    sG.water = sG.waterTo = 32.0f * PastoriaLevel();
    PastoriaProp();
    PastoriaGates();
}

// The ground on tile (X, Y) for someone at height NEAR (the pool's, or the block's)
static float GroundFor(s32 x, s32 y, float near, float water)
{
    return InPlate(x, y) ? PoolHeight(x, y, near, water) : S3D_TileFloor(x, y, near);
}

// A step from (FX, FY) at height FROMH to (TX, TY) with the water at LEVEL: blocked by the gym's rules (the gates'
// levels, deep water, heights 20 apart); *TOH the height there
static bool8 PoolStepBlocked(s32 fx, s32 fy, float fromH, s32 tx, s32 ty, u8 level, float *toH)
{
    float water = 32.0f * level, b;
    s32 gate = PastoriaGate(tx, ty);
    if (gate >= 0 && gate != level)
        return TRUE;
    b = GroundFor(tx, ty, fromH, water);
    if (toH != NULL)
        *toH = b;
    if (!InPlate(tx, ty) && !InPlate(fx, fy))
        return FALSE;
    if (InPlate(tx, ty) && PastoriaDeep(tx, ty))
    {
        // (deep water: walked only where the ground there is the floor, not the water)
        float floor = S3D_TileFloor(tx, ty, fromH);
        if (floor != floor || b != floor)
            return TRUE;
    }
    return fromH == fromH && b == b && (fromH - b >= 20.0f || b - fromH >= 20.0f);
}

static bool8 PastoriaBlocked(s32 fx, s32 fy, s32 tx, s32 ty)
{
    float near = S3D_PlayerHeight();
    if (near != near)
        near = sG.water;
    return PoolStepBlocked(fx, fy, GroundFor(fx, fy, near, sG.water), tx, ty, PastoriaLevel(), NULL);
}

// The button under the player (its level; -1 none)
static s32 PastoriaButtonAt(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPastoriaButtons); i++)
        if (sPastoriaButtons[i][0] == x && sPastoriaButtons[i][1] == y)
            return sPastoriaButtons[i][2];
    return -1;
}

static void Task_PastoriaWater(u8 taskId)
{
    if (sG.water < sG.waterTo)
        sG.water = sG.water + 1.0f > sG.waterTo ? sG.waterTo : sG.water + 1.0f;
    else if (sG.water > sG.waterTo)
        sG.water = sG.water - 1.0f < sG.waterTo ? sG.waterTo : sG.water - 1.0f;
    PastoriaProp();
    if (sG.water == sG.waterTo)
    {
        m4aSongNumStop(SE_M_WATERFALL);
        PastoriaGates();
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

// The button's script (data/scripts/sinnoh_places.inc): the water moves to VAR_0x8004's level (waitstate)
u16 Sinnoh_PastoriaWater(void)
{
    u8 level = gSpecialVar_0x8004 % 3;
    VarSet(VAR_SINNOH_PASTORIA, level + 1);
    sG.waterTo = 32.0f * level;
    MPNet_Log("gym: PASTORIA's water to level %d", level);
    if (sG.gym != GYM_PASTORIA || sG.water == sG.waterTo)
    {
        sG.water = sG.waterTo;
        if (sG.gym == GYM_PASTORIA)
            PastoriaGates();
        ScriptContext_Enable();
        return 0;
    }
    PlaySE(SE_M_WATERFALL);
    CreateTask(Task_PastoriaWater, 80);
    return 0;
}

// ------------------------------------------------------------------------------------------------------ SUNYSHORE
// Three rooms of gears on bridges: a button turns every gear in the room a quarter turn (or back one, or a half turn),
// each its own way; where the bridges don't meet Platinum's regions block (by the quarter turn: 0-3). A room starts at
// its own turn (2, 1, 0), or at 0 from its entrance (Platinum's).
static const struct { const struct SunyshoreGear *gears; u8 ngears; const u8 (*regions)[4]; const u8 *block; u8 nblock;
                      const u8 (*buttons)[3]; u8 nbuttons; u8 start, entranceY; } sRooms[3] = {
    {sSunyshoreGears1, ARRAY_COUNT(sSunyshoreGears1), sSunyshoreRegions1, sSunyshoreBlock1[0], ARRAY_COUNT(sSunyshoreBlock1[0]),
     sSunyshoreButtons1, ARRAY_COUNT(sSunyshoreButtons1), 2, 14},
    {sSunyshoreGears2, ARRAY_COUNT(sSunyshoreGears2), sSunyshoreRegions2, sSunyshoreBlock2[0], ARRAY_COUNT(sSunyshoreBlock2[0]),
     sSunyshoreButtons2, ARRAY_COUNT(sSunyshoreButtons2), 1, 21},
    {sSunyshoreGears3, ARRAY_COUNT(sSunyshoreGears3), sSunyshoreRegions3, sSunyshoreBlock3[0], ARRAY_COUNT(sSunyshoreBlock3[0]),
     sSunyshoreButtons3, ARRAY_COUNT(sSunyshoreButtons3), 0, 25},
};

static u8 SunyshoreTurn(void)
{
    return VarGet(VAR_SINNOH_SUNYSHORE) % 4;
}

// The gears drawn: each at its quarter turn (its start plus the room's, its own way), plus EXTRA degrees of the turn
// going on
static void SunyshoreProps(void)
{
    u32 i;
    for (i = 0; i < sRooms[sG.room].ngears && i < S3D_DYN_PROPS; i++)
    {
        const struct SunyshoreGear *g = &sRooms[sG.room].gears[i];
        float angle = 90.0f * g->start + (g->clockwise ? -1.0f : 1.0f) * (90.0f * SunyshoreTurn() + sG.turned);
        float x = sG.ox + g->x * 16 + 8, y = g->y * 16 + (g->upright ? 8 : 0), z = sG.oz + g->z * 16 + 8;
        if (g->upright)
            S3D_DynPropTurned(i, g->model, x, y, z, angle, 0);
        else
            S3D_DynProp(i, g->model, x, y, z, angle);
    }
}

static bool8 SunyshoreBlocked(s32 x, s32 y, u8 turn)
{
    u32 i;
    const u8 *list = sRooms[sG.room].block + turn * sRooms[sG.room].nblock;
    for (i = 0; i < sRooms[sG.room].nblock; i++)
    {
        const u8 *r = sRooms[sG.room].regions[list[i]];
        if (x >= r[0] && x < r[0] + r[2] && y >= r[1] && y < r[1] + r[3])
            return TRUE;
    }
    return FALSE;
}

static s32 SunyshoreButtonAt(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < sRooms[sG.room].nbuttons; i++)
        if (sRooms[sG.room].buttons[i][0] == x && sRooms[sG.room].buttons[i][1] == y)
            return sRooms[sG.room].buttons[i][2];
    return -1;
}

static void SunyshoreLoad(u8 room)
{
    sG.room = room;
    // (where the player comes in: the save's position, set by the warp before the map loads)
    VarSet(VAR_SINNOH_SUNYSHORE, gSaveBlock1Ptr->pos.y == sRooms[room].entranceY ? 0 : sRooms[room].start);
    sG.turned = sG.turnTo = 0;
    SunyshoreProps();
}

static void Task_SunyshoreGears(u8 taskId)
{
    sG.turned += 5.625f * sG.turnWay;
    if ((sG.turnWay > 0 && sG.turned >= sG.turnTo) || (sG.turnWay < 0 && sG.turned <= sG.turnTo))
    {
        // (the turn done: the room's quarter turn moves on, the gears' extra back to 0)
        VarSet(VAR_SINNOH_SUNYSHORE, (SunyshoreTurn() + 4 + (s32)(sG.turnTo / 90.0f)) % 4);
        sG.turned = sG.turnTo = 0;
        SunyshoreProps();
        m4aSongNumStop(SE_M_STRENGTH);
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    SunyshoreProps();
}

// A button's script (data/scripts/sinnoh_places.inc): the room's gears turn VAR_0x8004 quarter turns (3: one back)
u16 Sinnoh_SunyshoreTurn(void)
{
    u8 q = gSpecialVar_0x8004 % 4;
    if (sG.gym != GYM_SUNYSHORE || q == 0)
    {
        ScriptContext_Enable();
        return 0;
    }
    sG.turnTo = q == 3 ? -90.0f : 90.0f * q;
    sG.turnWay = q == 3 ? -1 : 1;
    sG.turned = 0;
    MPNet_Log("gym: SUNYSHORE room %d gears turn %d (from %d)", sG.room + 1, q, SunyshoreTurn());
    PlaySE(SE_M_STRENGTH);
    CreateTask(Task_SunyshoreGears, 80);
    return 0;
}

// ------------------------------------------------------------------------------------------------------ all gyms
void S3D_Gyms_MapLoaded(void)
{
    u16 map = CurrentMap();
    S3D_DynPropsClear();
    gS3dHeightHook = NULL;
    sG.gym = GYM_NONE;
    sG.lastX = sG.lastY = -1;
    if (!S3D_MapOrigin(&sG.ox, &sG.oz))
        return;
    if (map == MAP_SINNOH_PASTORIA_CITY_GYM)
    {
        sG.gym = GYM_PASTORIA;
        PastoriaLoad();
    }
    else if (map == MAP_SINNOH_SUNYSHORE_CITY_GYM_ROOM1 || map == MAP_SINNOH_SUNYSHORE_CITY_GYM_ROOM2
          || map == MAP_SINNOH_SUNYSHORE_CITY_GYM_ROOM3)
    {
        sG.gym = GYM_SUNYSHORE;
        SunyshoreLoad(map == MAP_SINNOH_SUNYSHORE_CITY_GYM_ROOM1 ? 0 : map == MAP_SINNOH_SUNYSHORE_CITY_GYM_ROOM2 ? 1 : 2);
    }
    if (sG.gym == GYM_PASTORIA)
        gS3dHeightHook = HeightHook;
}

void S3D_Gyms_Update(void)
{
    s16 x, y;
    if (sG.gym == GYM_NONE)
        return;
    if (gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x != gObjectEvents[gPlayerAvatar.objectEventId].previousCoords.x
     || gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y != gObjectEvents[gPlayerAvatar.objectEventId].previousCoords.y)
        ;   // (mid-step)
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET, y -= MAP_OFFSET;
    if (x == sG.lastX && y == sG.lastY)
        return;
    if (!MP_IsLocalFree())
        return;
    sG.lastX = x, sG.lastY = y;
    if (sG.gym == GYM_PASTORIA)
    {
        s32 level = PastoriaButtonAt(x, y);
        if (level >= 0 && level != PastoriaLevel())
        {
            extern const u8 Sinnoh_EventScript_PastoriaButton[];
            gSpecialVar_0x8004 = level;
            ScriptContext_SetupScript(Sinnoh_EventScript_PastoriaButton);
        }
    }
    else if (sG.gym == GYM_SUNYSHORE)
    {
        s32 q = SunyshoreButtonAt(x, y);
        if (q > 0)
        {
            extern const u8 Sinnoh_EventScript_SunyshoreButton[];
            gSpecialVar_0x8004 = q;
            ScriptContext_SetupScript(Sinnoh_EventScript_SunyshoreButton);
        }
    }
}

bool8 S3D_Gyms_StepBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    bool8 blocked = FALSE;
    switch (sG.gym)
    {
    case GYM_PASTORIA:
        blocked = PastoriaBlocked(fromX - MAP_OFFSET, fromY - MAP_OFFSET, toX - MAP_OFFSET, toY - MAP_OFFSET);
        break;
    case GYM_SUNYSHORE:
        blocked = SunyshoreBlocked(toX - MAP_OFFSET, toY - MAP_OFFSET, SunyshoreTurn());
        break;
    }
    if (getenv("MOEMON_GYM_TRACE"))
        printf("[gym] step %d,%d -> %d,%d: %s (player at %.0f)\n", fromX - MAP_OFFSET, fromY - MAP_OFFSET, toX - MAP_OFFSET,
               toY - MAP_OFFSET, blocked ? "blocked" : "free", S3D_PlayerHeight());
    return blocked;
}

static void ReportReached(const u8 *seen, u32 states);

// (tests: `mp gym`: the gym's state; `mp gym solve`: PASTORIA walked from the door by every step and button, with the
// gym's own rules and the map's walls: which trainers and the leader are reached, after how many buttons)
void S3D_Gyms_Debug(const char *arg)
{
    printf("[test] gym %d: water %.0f (level %d) room %d turn %d\n", sG.gym, sG.water, PastoriaLevel(), sG.room + 1,
           SunyshoreTurn());
    if (sG.gym == GYM_PASTORIA && strcmp(arg, "solve") == 0)
    {
        static u8 sSeen[3][64][32];
        static struct { u8 x, y, level, presses; float h; } sQueue[3 * 64 * 32];
        static const s8 sDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        u32 head = 0, tail = 0, i;
        s32 w = gBackupMapLayout.width - 2 * MAP_OFFSET, hgt = gBackupMapLayout.height - 2 * MAP_OFFSET;
        memset(sSeen, 0, sizeof(sSeen));
        // from the door (13, 41), the water as Platinum starts it (middle)
        sQueue[tail].x = 13, sQueue[tail].y = 41, sQueue[tail].level = 1, sQueue[tail].presses = 0;
        sQueue[tail].h = S3D_TileFloor(13, 41, 64.0f);
        tail++;
        sSeen[1][41][13] = 1;
        while (head < tail)
        {
            u8 x = sQueue[head].x, y = sQueue[head].y, level = sQueue[head].level, presses = sQueue[head].presses;
            float h = sQueue[head].h;
            head++;
            for (i = 0; i < 4; i++)
            {
                s32 nx = x + sDir[i][0], ny = y + sDir[i][1];
                float nh;
                s32 b, nl;
                if (nx < 0 || ny < 0 || nx >= w || ny >= hgt || nx >= 32 || ny >= 64)
                    continue;
                if (MapGridGetCollisionAt(nx + MAP_OFFSET, ny + MAP_OFFSET) && PastoriaGate(nx, ny) < 0)
                    continue;
                if (PoolStepBlocked(x, y, h, nx, ny, level, &nh))
                    continue;
                b = PastoriaButtonAt(nx, ny);
                nl = b >= 0 ? b : level;
                if (nl != level)
                    nh = GroundFor(nx, ny, nh, 32.0f * nl);
                if (sSeen[nl][ny][nx])
                    continue;
                sSeen[nl][ny][nx] = presses + (nl != level) + 1;
                sQueue[tail].x = nx, sQueue[tail].y = ny, sQueue[tail].level = nl;
                sQueue[tail].presses = presses + (nl != level), sQueue[tail].h = nh;
                tail++;
            }
        }
        ReportReached(&sSeen[0][0][0], 3);
    }
    if (sG.gym == GYM_SUNYSHORE && strcmp(arg, "solve") == 0)
    {
        // from the room's entrance at turn 0, every step and button
        static u8 sSeen[4][64][32];
        static struct { u8 x, y, turn, presses; } sQueue[4 * 64 * 32];
        static const s8 sDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        u32 head = 0, tail = 0, i, w;
        s16 px, py;
        memset(sSeen, 0, sizeof(sSeen));
        PlayerGetDestCoords(&px, &py);
        sQueue[tail].x = px - MAP_OFFSET, sQueue[tail].y = py - MAP_OFFSET, sQueue[tail].turn = SunyshoreTurn();
        sQueue[tail].presses = 0;
        sSeen[sQueue[tail].turn][sQueue[tail].y][sQueue[tail].x] = 1;
        tail++;
        while (head < tail)
        {
            u8 x = sQueue[head].x, y = sQueue[head].y, turn = sQueue[head].turn, presses = sQueue[head].presses;
            head++;
            for (i = 0; i < 4; i++)
            {
                s32 nx = x + sDir[i][0], ny = y + sDir[i][1], q, nt;
                if (nx < 0 || ny < 0 || nx >= 32 || ny >= 64)
                    continue;
                if (MapGridGetCollisionAt(nx + MAP_OFFSET, ny + MAP_OFFSET) || SunyshoreBlocked(nx, ny, turn))
                    continue;
                q = SunyshoreButtonAt(nx, ny);
                nt = q > 0 ? (turn + q) % 4 : turn;
                if (sSeen[nt][ny][nx])
                    continue;
                sSeen[nt][ny][nx] = presses + (q > 0) + 1;
                sQueue[tail].x = nx, sQueue[tail].y = ny, sQueue[tail].turn = nt, sQueue[tail].presses = presses + (q > 0);
                tail++;
            }
        }
        ReportReached(&sSeen[0][0][0], 4);
        for (w = 0; w < gMapHeader.events->warpCount; w++)
        {
            const struct WarpEvent *e = &gMapHeader.events->warps[w];
            u32 l, best = 0;
            for (l = 0; l < 4; l++)
                if (e->x < 32 && e->y < 64 && sSeen[l][e->y][e->x] && (best == 0 || sSeen[l][e->y][e->x] < best))
                    best = sSeen[l][e->y][e->x];
            printf("[test] gym solve: door %u at %d,%d %s\n", w, e->x, e->y, best ? "reached" : "NOT reached");
        }
    }
}

// The people a walk reached (SEEN: [states][64][32], 0 not reached, else 1 + the buttons it took)
static void ReportReached(const u8 *seen, u32 states)
{
    static const s8 sDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    u32 o, i;
    {
        // who is reached: a free tile next to each person
        for (o = 0; o < gMapHeader.events->objectEventCount; o++)
        {
            const struct ObjectEventTemplate *t = &gMapHeader.events->objectEvents[o];
            u32 best = 0;
            if (t->flagId == 0x3FFF)
                continue;
            for (i = 0; i < 4; i++)
            {
                s32 nx = t->x + sDir[i][0], ny = t->y + sDir[i][1], l;
                if (nx < 0 || ny < 0 || nx >= 32 || ny >= 64)
                    continue;
                for (l = 0; l < states; l++)
                    if (seen[(l * 64 + ny) * 32 + nx] && (best == 0 || seen[(l * 64 + ny) * 32 + nx] < best))
                        best = seen[(l * 64 + ny) * 32 + nx];
            }
            printf("[test] gym solve: person %u at %d,%d %s", o + 1, t->x, t->y, best ? "reached" : "NOT reached");
            if (best)
                printf(" after %u buttons", best - 1);
            printf("\n");
        }
    }
}
