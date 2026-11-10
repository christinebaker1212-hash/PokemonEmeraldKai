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
#include "s3d_underground.h"
void Sinnoh_Lift_MapLoaded(void);   // (s3d_lift.c)
void Sinnoh_Lift_Update(void);
void Sinnoh_Tram_MapLoaded(void);   // (s3d_tram.c)
void Sinnoh_Tram_Update(void);
#include "m4a.h"
#include "event_data.h"
#include "battle_setup.h"
#include "mp_region.h"
#include "constants/trainers_sinnoh.h"
#include "constants/trainers.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "constants/metatile_behaviors.h"
#include "mp.h"
#include "script.h"
#include "overworld.h"
#include "random.h"
#include "sound.h"
#include "task.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include <string.h>
struct SunyshoreGear { u16 model; u8 x, y, z, start, clockwise, upright; };
struct CanalavePlatform { u16 model; u8 ax, ay, az, bx, by, bz, floorA, floorB, startB; };
#include "../data/sinnoh_gyms.h"

u16 S3D_Gyms_HealBallModel(void)   // (the healing machine's balls: s3d_heal.c)
{
    return S3D_MODEL_POKECENTER_HEALING_MACHINE_MINI_POKEBALL;
}

#define VAR_SINNOH_PASTORIA 0x52F3   // the water's level + 1 (0: Platinum's start, the middle)
#define VAR_SINNOH_SUNYSHORE 0x52F4  // the gears' quarter turn in the room the player is in
#define VAR_SINNOH_CANALAVE_LO 0x52F5   // the platforms moved from where they start (bits 0-15)
#define VAR_SINNOH_CANALAVE_HI 0x52F6   // (bits 16-23)
#define VAR_SINNOH_CANALAVE_FLOOR 0x52F7   // the floor the player is on
// VEILSTONE's bags and stacks and SNOWPOINT's snowballs, kept with the save as Platinum's persisted map features are: a
// save made in the gym loads with them where the player left them (they were put back, and the bags walled in a player
// who saved beside MAYLENE). Walking in from another map puts them back as Platinum does.
#define VAR_SINNOH_VEIL_TIRES 0x52A0   // the stacks standing (bits) | 0x8000: kept
#define VAR_SINNOH_VEIL_BAGS  0x52A1   // .. 0x52A9: each bag's x | y << 5
#define VAR_SINNOH_SNOW_LO    0x52AA   // the snowballs broken (bits 0-15)
#define VAR_SINNOH_SNOW_HI    0x52AB   // (bits 16-19) | 0x8000: kept
static bool8 sFromSave;   // S3D_Gyms_SavedMapLoaded: the map is the one a save was made on

enum { GYM_NONE, GYM_PASTORIA, GYM_SUNYSHORE, GYM_CANALAVE, GYM_ETERNA, GYM_VEILSTONE, GYM_SNOWPOINT, GYM_HEARTHOME,
       GYM_VILLA };

static struct
{
    u8 gym;
    s16 lastX, lastY;      // the player's tile last frame (a step onto a button)
    float water, waterTo;  // PASTORIA: the water's height now and where it's going
    float ox, oz;          // the map's corner in its matrix's world
    u8 room;               // SUNYSHORE: the room (0-2)
    float turned, turnTo;  // ... the gears' turn now and where it's going (degrees from the quarter turn's start)
    s8 turnWay;
    u8 ride;               // CANALAVE: the platform being ridden (+1; 0 none)
    float rideY;           // ... its height now
    s16 rideSteps;         // ... the tiles still to go along (a platform that moves sideways)
    u8 rideDir;
    u8 clockShown;         // ETERNA: the state the clock shows
    float clockMinutes;    // ... its time (minutes past 12), and where it's going
    float clockTo;
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

static float CanalaveHeight(s32 x, s32 y, float near);

static float HeightHook(s32 x, s32 y, float near)
{
    return sG.gym == GYM_PASTORIA ? PastoriaHeight(x, y, near) : sG.gym == GYM_CANALAVE ? CanalaveHeight(x, y, near) : NAN;
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

// ------------------------------------------------------------------------------------------------------ CANALAVE
// Four floors, 10 tiles apart (160 units): the ground floor is the block's, the three above Platinum's run-time floor
// props, shown only once the player is up to them. 24 platforms: stepped on, a lift carries the player up or down to
// its other floor, a sideways one along to its other end (Platinum's paths). Each floor has its own walls (Platinum's
// tables: the map's own grid walls are lifted in this gym); people stand on their own floors.
#define CANALAVE_FLOOR_H 160.0f

static u32 CanalaveMoved(void)
{
    return VarGet(VAR_SINNOH_CANALAVE_LO) | (u32)VarGet(VAR_SINNOH_CANALAVE_HI) << 16;
}

static bool8 CanalaveAtB(u32 i)
{
    return sCanalavePlatforms[i].startB ^ ((CanalaveMoved() >> i) & 1);
}

static u8 CanalaveFloor(void)
{
    return VarGet(VAR_SINNOH_CANALAVE_FLOOR) % 4;
}

static void CanalavePlatformAt(u32 i, s32 *x, s32 *y, s32 *z)
{
    const struct CanalavePlatform *p = &sCanalavePlatforms[i];
    bool8 b = CanalaveAtB(i);
    *x = b ? p->bx : p->ax, *y = b ? p->by : p->ay, *z = b ? p->bz : p->az;
}

// The props: floors above shown up to the player's height, the platforms on the floors shown
static void CanalaveProps(void)
{
    float eye = sG.ride ? sG.rideY : CanalaveFloor() * CANALAVE_FLOOR_H;
    u32 i, k;
    static const u16 sFloors[3] = {S3D_MODEL_CANALAVE_GYM_SECOND_FLOOR, S3D_MODEL_CANALAVE_GYM_THIRD_FLOOR,
                                   S3D_MODEL_CANALAVE_GYM_FOURTH_FLOOR};
    for (k = 0; k < 3; k++)
    {
        if (eye >= 16.0f + CANALAVE_FLOOR_H * k)
            S3D_DynProp(k, sFloors[k], sG.ox + 256, CANALAVE_FLOOR_H * (k + 1), sG.oz + 256, 0);
        else
            S3D_DynPropHide(k);
    }
    for (i = 0; i < ARRAY_COUNT(sCanalavePlatforms) && 3 + i < S3D_DYN_PROPS; i++)
    {
        s32 x, y, z;
        float fy;
        CanalavePlatformAt(i, &x, &y, &z);
        fy = y * 16.0f;
        if (sG.ride == i + 1)
        {
            float px, pz;
            S3D_PlayerWorld(&px, &pz);
            if (sG.rideSteps >= 0 && sG.rideDir != 0)
                S3D_DynProp(3 + i, sCanalavePlatforms[i].model, px, fy, pz, 0);   // (along with the player)
            else
                S3D_DynProp(3 + i, sCanalavePlatforms[i].model, sG.ox + x * 16 + 8, sG.rideY, sG.oz + z * 16 + 8, 0);
            continue;
        }
        if (fy <= eye + 8.0f || fy < 16.0f)
            S3D_DynProp(3 + i, sCanalavePlatforms[i].model, sG.ox + x * 16 + 8, fy, sG.oz + z * 16 + 8, 0);
        else
            S3D_DynPropHide(3 + i);
    }
}

static float CanalaveHeight(s32 x, s32 y, float near)
{
    if (sG.ride && sG.rideDir == 0)
        return sG.rideY;
    return CanalaveFloor() ? CanalaveFloor() * CANALAVE_FLOOR_H : NAN;
}

static float CanalavePersonHeight(u8 localId, float h)
{
    return localId >= 1 && localId <= ARRAY_COUNT(sCanalaveFloorOf) ? sCanalaveFloorOf[localId - 1] * CANALAVE_FLOOR_H : NAN;
}

static bool8 CanalaveWall(u8 floor, s32 x, s32 y)
{
    return x < 0 || y < 0 || x >= 32 || y >= 32 || ((sCanalaveWalls[floor][y] >> x) & 1);
}

// People on floors above the player's (and theirs) aren't seen
static void CanalavePeople(void)
{
    u32 i;
    float eye = sG.ride ? sG.rideY : CanalaveFloor() * CANALAVE_FLOOR_H;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *o = &gObjectEvents[i];
        if (!o->active || o->isPlayer || o->localId < 1 || o->localId > ARRAY_COUNT(sCanalaveFloorOf))
            continue;
        o->invisible = sCanalaveFloorOf[o->localId - 1] * CANALAVE_FLOOR_H > eye + 8.0f;
    }
}

static void CanalaveLoad(void)
{
    s32 x, y;
    // (from the door: the gym as Platinum sets it up, on the ground floor)
    if (gSaveBlock1Ptr->pos.x == 16 && gSaveBlock1Ptr->pos.y >= 26)
    {
        VarSet(VAR_SINNOH_CANALAVE_LO, 0);
        VarSet(VAR_SINNOH_CANALAVE_HI, 0);
        VarSet(VAR_SINNOH_CANALAVE_FLOOR, 0);
    }
    for (y = 0; y < 32; y++)
        for (x = 0; x < 32; x++)
            MapGridSetMetatileImpassabilityAt(x + MAP_OFFSET, y + MAP_OFFSET, FALSE);
    sG.ride = 0;
    CanalaveProps();
    gS3dObjectHeightHook = CanalavePersonHeight;
}

static s32 CanalavePlatformUnder(s32 x, s32 z)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sCanalavePlatforms); i++)
    {
        s32 px, py, pz;
        CanalavePlatformAt(i, &px, &py, &pz);
        if (px == x && pz == z && py / 10 == CanalaveFloor())
            return i;
    }
    return -1;
}

static void Task_CanalaveRide(u8 taskId)
{
    const struct CanalavePlatform *p = &sCanalavePlatforms[sG.ride - 1];
    bool8 b = CanalaveAtB(sG.ride - 1);   // (where it goes: already toggled)
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (sG.rideDir == 0)
    {
        float to = (b ? p->by : p->ay) * 16.0f;
        sG.rideY += sG.rideY < to ? 8.0f : -8.0f;
        if ((sG.rideY >= to && (b ? p->by : p->ay) * 16.0f >= (b ? p->ay : p->by) * 16.0f)
         || (sG.rideY <= to && (b ? p->by : p->ay) * 16.0f <= (b ? p->ay : p->by) * 16.0f))
        {
            sG.rideY = to;
            VarSet(VAR_SINNOH_CANALAVE_FLOOR, (b ? p->floorB : p->floorA) % 4);
            goto done;
        }
    }
    else if (!ObjectEventIsHeldMovementActive(player) || ObjectEventClearHeldMovementIfFinished(player))
    {
        if (sG.rideSteps <= 0)
            goto done;
        ObjectEventSetHeldMovement(player, MOVEMENT_ACTION_WALK_FAST_DOWN + sG.rideDir - 1);
        sG.rideSteps--;
    }
    CanalaveProps();
    CanalavePeople();
    return;
done:
    m4aSongNumStop(SE_ELEVATOR);
    PlaySE(SE_CLICK);
    sG.ride = 0;
    PlayerGetDestCoords(&sG.lastX, &sG.lastY);
    sG.lastX -= MAP_OFFSET, sG.lastY -= MAP_OFFSET;
    CanalaveProps();
    CanalavePeople();
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// The platform under the player moves (its script: data/scripts/sinnoh_places.inc; VAR_0x8004 the platform)
u16 Sinnoh_CanalaveRide(void)
{
    u32 i = gSpecialVar_0x8004, moved;
    const struct CanalavePlatform *p;
    s32 x, y, z;
    if (sG.gym != GYM_CANALAVE || i >= ARRAY_COUNT(sCanalavePlatforms))
    {
        ScriptContext_Enable();
        return 0;
    }
    p = &sCanalavePlatforms[i];
    CanalavePlatformAt(i, &x, &y, &z);
    moved = CanalaveMoved() ^ (1u << i);
    VarSet(VAR_SINNOH_CANALAVE_LO, moved & 0xFFFF);
    VarSet(VAR_SINNOH_CANALAVE_HI, moved >> 16);
    sG.ride = i + 1;
    sG.rideY = y * 16.0f;
    sG.rideDir = 0;
    sG.rideSteps = 0;
    if (p->ax != p->bx || p->az != p->bz)
    {
        // (sideways: the player is carried tile by tile to the other end; DIR_SOUTH 1, NORTH 2, WEST 3, EAST 4)
        s32 tx = CanalaveAtB(i) ? p->bx : p->ax, tz = CanalaveAtB(i) ? p->bz : p->az;
        sG.rideDir = tx > x ? DIR_EAST : tx < x ? DIR_WEST : tz > z ? DIR_SOUTH : DIR_NORTH;
        sG.rideSteps = tx > x ? tx - x : tx < x ? x - tx : tz > z ? tz - z : z - tz;
    }
    MPNet_Log("gym: CANALAVE platform %d from %d,%d,%d (floor %d)", i, x, y, z, CanalaveFloor());
    PlaySE(SE_ELEVATOR);
    CreateTask(Task_CanalaveRide, 80);
    return 0;
}

// ------------------------------------------------------------------------------------------------------ ETERNA
// A flower clock: each of the three trainers beaten, then the leader, moves its hands to Platinum's next time (state
// 0-4), opening another path across it (Platinum's collision of the clock and of the fountain row by state); the
// hour hand lies across one tile of the path, hopped over (a ledge jump) along its axis.
#define ETERNA_X0 5
#define ETERNA_Y0 7
#define ETERNA_CENTER_X 11
#define ETERNA_CENTER_Y 13

static u8 EternaState(void)
{
    u8 n = HasTrainerBeenFought(TRAINER_SINNOH_LASS_CAROLINE) + HasTrainerBeenFought(TRAINER_SINNOH_AROMA_LADY_JENNA)
         + HasTrainerBeenFought(TRAINER_SINNOH_AROMA_LADY_ANGELA);
    return n + (n == 3 && FlagGet(FLAG_SINNOH_BADGE01 + 1));
}

// Platinum's hand angles (65536 a turn) for a time, as degrees for the props
static void EternaHands(void)
{
    static const u16 sHour[12] = {0, 0x1559, 0x1559 * 2, 0x4000, 0x1559 * 4, 0x1559 * 5, 0x8000, 0x1559 * 7, 0x1559 * 8,
                                  0xC000, 0x1559 * 10, 0x1559 * 11};
    s32 total = (s32)(sG.clockMinutes + 0.5f), hour = (total / 60) % 12, minute = total % 60;
    u32 m = (minute % 15 == 0 ? sHour[minute / 5] : minute * 1093) + 0x10000 - 0x4000;
    u32 h = sHour[hour] + (5 * 1093 / 60) * minute + 0x8000;
    m = (0x10000 - (m & 0xFFFF)) & 0xFFFF, h = (0x10000 - (h & 0xFFFF)) & 0xFFFF;
    S3D_DynProp(0, S3D_MODEL_ETERNA_GYM_MODEL_CLOCK_HOUR_HAND, sG.ox + ETERNA_CENTER_X * 16 + 8, 12, sG.oz + ETERNA_CENTER_Y * 16 + 8,
                h * 360.0f / 65536.0f);
    S3D_DynProp(1, S3D_MODEL_ETERNA_GYM_MODEL_CLOCK_MINUTE_HAND, sG.ox + ETERNA_CENTER_X * 16 + 8, 13, sG.oz + ETERNA_CENTER_Y * 16 + 8,
                m * 360.0f / 65536.0f);
}

static float EternaTime(u8 state)
{
    return sEternaTimes[state][0] * 60.0f + sEternaTimes[state][1];
}

static void EternaLoad(void)
{
    s32 x, y;
    sG.clockShown = EternaState();
    sG.clockMinutes = sG.clockTo = EternaTime(sG.clockShown);
    // (the clock's and the fountain row's own walls replace the map's there)
    for (y = ETERNA_Y0; y < ETERNA_Y0 + 13; y++)
        for (x = ETERNA_X0; x < ETERNA_X0 + 13; x++)
            MapGridSetMetatileImpassabilityAt(x + MAP_OFFSET, y + MAP_OFFSET, FALSE);
    for (x = 1; x <= 21; x++)
        MapGridSetMetatileImpassabilityAt(x + MAP_OFFSET, 19 + MAP_OFFSET, FALSE);
    EternaHands();
}

static bool8 EternaBlocked(s32 x, s32 y)
{
    u8 st = sG.clockShown;
    if (x >= ETERNA_X0 && x < ETERNA_X0 + 13 && y >= ETERNA_Y0 && y < ETERNA_Y0 + 13
     && ((sEternaClock[st][y - ETERNA_Y0] >> (x - ETERNA_X0)) & 1))
        return TRUE;
    return y == 19 && x >= 1 && x <= 21 && ((sEternaFountain[st] >> (x - 1)) & 1);
}

static void Task_EternaClock(u8 taskId)
{
    float step = 4.0f, d = sG.clockTo - sG.clockMinutes;
    if (d > 360.0f)
        d -= 720.0f;
    else if (d < -360.0f)
        d += 720.0f;
    if (d > step)
        sG.clockMinutes += step;
    else if (d < -step)
        sG.clockMinutes -= step;
    else
        sG.clockMinutes = sG.clockTo;
    if (sG.clockMinutes < 0)
        sG.clockMinutes += 720.0f;
    if (sG.clockMinutes >= 720.0f)
        sG.clockMinutes -= 720.0f;
    EternaHands();
    if (sG.clockMinutes == sG.clockTo)
    {
        m4aSongNumStop(SE_M_STRENGTH);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

// The clock moves to its state now (a trainer or the leader beaten: data/scripts/sinnoh_places.inc)
u16 Sinnoh_EternaClock(void)
{
    if (sG.gym != GYM_ETERNA)
    {
        ScriptContext_Enable();
        return 0;
    }
    sG.clockShown = EternaState();
    sG.clockTo = EternaTime(sG.clockShown);
    MPNet_Log("gym: ETERNA's clock to state %d (%d:%02d)", sG.clockShown, sEternaTimes[sG.clockShown][0], sEternaTimes[sG.clockShown][1]);
    PlaySE(SE_M_STRENGTH);
    CreateTask(Task_EternaClock, 80);
    return 0;
}

bool8 S3D_Gyms_JumpAt(s16 x, s16 y, u8 direction)
{
    const u8 *j;
    if (sG.gym != GYM_ETERNA || sG.clockShown == 0)
        return FALSE;
    j = sEternaJump[sG.clockShown];
    if (x - MAP_OFFSET != j[0] || y - MAP_OFFSET != j[1])
        return FALSE;
    return j[2] ? (direction == DIR_NORTH || direction == DIR_SOUTH) : (direction == DIR_WEST || direction == DIR_EAST);
}

// ------------------------------------------------------------------------------------------------------ VEILSTONE
// Punching bags on tracks: walking into one hits it, and it slides the way it was hit along its track until a stop
// (Platinum's table: sVeilstoneTrack), knocking over a tire stack in its way. The bags and stacks stay as the player
// left them until the player comes in from somewhere else (as Platinum's: a battle keeps them).
#define VEIL_BAG_TOP 67.0f   // (the bag's model hangs from its top)
#define VEIL_SLOT_TIRE 9

static struct
{
    u8 x[ARRAY_COUNT(sVeilstoneBags)], y[ARRAY_COUNT(sVeilstoneBags)];
    u16 tires;                // the stacks still standing (bits)
    u16 lastMap;              // the map the gym's state is for
    s8 bag, d;                // a bag moving (-1 none) and its way (Platinum's order: north, south, west, east)
    float off, speed, swing;  // ... how far it has slid from its tile, its speed, its lean (degrees)
    u16 frames;
    u8 tiles, moving;
    s8 tire;                  // a stack falling (-1 none)
    u8 tireFrames;
} sVeil;

static const s8 sVeilDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

static u8 VeilFromDir(u8 direction)
{
    return direction == DIR_NORTH ? 0 : direction == DIR_SOUTH ? 1 : direction == DIR_WEST ? 2 : 3;
}

static s32 VeilBagAt(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
        if (sVeil.x[i] == x && sVeil.y[i] == y)
            return i;
    return -1;
}

static s32 VeilTireAt(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sVeilstoneTires); i++)
        if (((sVeil.tires >> i) & 1) && sVeilstoneTires[i][0] == x && sVeilstoneTires[i][1] == y)
            return i;
    return -1;
}

static u8 VeilTrack(s32 x, s32 y)
{
    return (x < 0 || y < 0 || x >= 32 || y >= 32) ? 1 : sVeilstoneTrack[y][x] - '0';
}

// Platinum's flags for a bag on X, Y going D: 1 it can't, 2 a stack ahead, 4 it stops here
static u8 VeilFlags(s32 x, s32 y, u8 d)
{
    u8 b = VeilTrack(x, y), f = 0;
    if (b == 4)
        f |= 4;
    else if ((b == 2 && d < 2) || (b == 3 && d >= 2) || (b == 5 && d == 2))
        f |= 1;
    if (VeilTrack(x + sVeilDir[d][0], y + sVeilDir[d][1]) == 1)
        f |= 1;
    if (VeilTireAt(x + sVeilDir[d][0], y + sVeilDir[d][1]) >= 0)
        f |= 2;
    return f;
}

static u8 VeilDistance(s32 x, s32 y, u8 d)
{
    u8 f = VeilFlags(x, y, d), n = 0;
    if (f != 0 && f != 4)
        return 0;
    do
    {
        x += sVeilDir[d][0], y += sVeilDir[d][1];
        n++;
        f = VeilFlags(x, y, d);
    } while (f == 0 && n < 32);
    return n;
}

static float VeilFloor(s32 x, s32 y)
{
    float h = S3D_TileFloor(x, y, 0);
    return (h == h ? h : 0) + 6;   // (Platinum's: a little over the floor)
}

// A bag hanging from its top, leaning LEAN degrees the way D (its bottom swung that way)
static void VeilBagProp(u32 i, float px, float pz, u8 d, float lean)
{
    float top = VeilFloor(sVeil.x[i], sVeil.y[i]) + VEIL_BAG_TOP, a = lean * 3.14159265f / 180.0f;
    float ox = 0, oz = 0, tilt, turn;
    if (d < 2)   // (north / south: the lean is about the east-west axis)
        turn = 0, tilt = d == 0 ? lean : -lean, oz = (d == 0 ? -1 : 1) * VEIL_BAG_TOP * sinf(a);
    else
        turn = 90, tilt = d == 2 ? lean : -lean, ox = (d == 2 ? -1 : 1) * VEIL_BAG_TOP * sinf(a);
    S3D_DynPropTurned(i, S3D_MODEL_VEILSTONE_GYM_SANDBAG, px + ox, top - VEIL_BAG_TOP * cosf(a), pz + oz, tilt, turn);
}

static void VeilProps(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
    {
        float px = sG.ox + sVeil.x[i] * 16 + 8, pz = sG.oz + sVeil.y[i] * 16 + 8;
        if ((s32)i == sVeil.bag)
            VeilBagProp(i, px + sVeilDir[sVeil.d][0] * sVeil.off, pz + sVeilDir[sVeil.d][1] * sVeil.off, sVeil.d, sVeil.swing);
        else
            VeilBagProp(i, px, pz, 0, 0);
    }
    for (i = 0; i < ARRAY_COUNT(sVeilstoneTires); i++)
    {
        u8 x = sVeilstoneTires[i][0], y = sVeilstoneTires[i][1];
        if ((s32)i == sVeil.tire)
        {
            // (falling over the way the bag came: tipped about its base, then gone)
            float t = sVeil.tireFrames * 90.0f / 12.0f, sink = sVeil.tireFrames > 12 ? (sVeil.tireFrames - 12) * 2.0f : 0;
            u8 d = sVeil.d;
            S3D_DynPropTurned(VEIL_SLOT_TIRE + i, S3D_MODEL_VEILSTONE_GYM_TIRE_STACK, sG.ox + x * 16 + 8,
                              VeilFloor(x, y) - 6 - sink, sG.oz + y * 16 + 8, d == 0 || d == 3 ? -fminf(t, 90) : fminf(t, 90),
                              d < 2 ? 0 : 90);
        }
        else if ((sVeil.tires >> i) & 1)
            S3D_DynProp(VEIL_SLOT_TIRE + i, S3D_MODEL_VEILSTONE_GYM_TIRE_STACK, sG.ox + x * 16 + 8, VeilFloor(x, y) - 6,
                        sG.oz + y * 16 + 8, 0);
        else
            S3D_DynPropHide(VEIL_SLOT_TIRE + i);
    }
}

static void VeilStore(void)
{
    u32 i;
    VarSet(VAR_SINNOH_VEIL_TIRES, sVeil.tires | 0x8000);
    for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
        VarSet(VAR_SINNOH_VEIL_BAGS + i, sVeil.x[i] | sVeil.y[i] << 5);
}

static void VeilLoad(u16 map)
{
    u32 i;
    if (sFromSave && (VarGet(VAR_SINNOH_VEIL_TIRES) & 0x8000))
    {
        // (a save made here: everything where the player left it)
        for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
        {
            u16 v = VarGet(VAR_SINNOH_VEIL_BAGS + i);
            sVeil.x[i] = v & 31, sVeil.y[i] = (v >> 5) & 31;
        }
        sVeil.tires = VarGet(VAR_SINNOH_VEIL_TIRES) & ((1 << ARRAY_COUNT(sVeilstoneTires)) - 1);
    }
    else if (sFromSave)
    {
        // (a save made here by a build that didn't keep them: the bags as the puzzle's solution leaves them, every
        // stack they knock over down, so wherever the player stands they can walk to the door and to MAYLENE)
        static const u8 sSolved[ARRAY_COUNT(sVeilstoneBags)][2] = {{7, 14}, {7, 12}, {11, 7}, {3, 20}, {19, 26}, {9, 10},
                                                                    {20, 12}, {21, 24}, {23, 27}};
        for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
            sVeil.x[i] = sSolved[i][0], sVeil.y[i] = sSolved[i][1];
        sVeil.tires = 0x240;   // (the stacks at 19,10 and 22,7, out of everyone's way)
    }
    else if (sVeil.lastMap != map)
    {
        // (in from somewhere else: everything back where Platinum puts it)
        for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
            sVeil.x[i] = sVeilstoneBags[i][0], sVeil.y[i] = sVeilstoneBags[i][1];
        sVeil.tires = (1 << ARRAY_COUNT(sVeilstoneTires)) - 1;
    }
    VeilStore();
    sVeil.lastMap = map;
    sVeil.bag = sVeil.tire = -1;
    VeilProps();
}

static bool8 VeilBlocked(s32 x, s32 y)
{
    return VeilBagAt(x, y) >= 0 || VeilTireAt(x, y) >= 0;
}

static void Task_VeilBag(u8 taskId)
{
    s32 i = sVeil.bag;
    u8 d = sVeil.d;
    sVeil.frames++;
    if (sVeil.moving == 0)
    {
        // (hit where it can't go: it swings and comes back)
        sVeil.swing = 14.0f * sinf(sVeil.frames * 0.35f) * expf(-sVeil.frames / 14.0f);
        if (sVeil.frames >= 40)
            goto done;
    }
    else if (sVeil.moving == 1)
    {
        // (Platinum's slide: slow for half a second, then faster up to 2 units a frame; a stop on each tile it reaches)
        if (sVeil.frames < 30)
            sVeil.speed += 1.0f / 60;
        else if (sVeil.frames < 36)
        {
            if (sVeil.speed < 1)
                sVeil.speed += sVeil.speed;
        }
        else if (sVeil.speed < 2)
            sVeil.speed = 2;
        if (sVeil.frames == 30)
            PlaySE(SE_M_STRENGTH);
        sVeil.off += sVeil.speed;
        sVeil.swing = -fminf(sVeil.frames, 12) * 1.5f;   // (the bottom trails)
        if ((u8)(sVeil.off / 16) > sVeil.tiles)
        {
            u8 f;
            sVeil.tiles++;
            sVeil.x[i] += sVeilDir[d][0], sVeil.y[i] += sVeilDir[d][1];
            sVeil.off -= 16;
            f = VeilFlags(sVeil.x[i], sVeil.y[i], d);
            if (f)
            {
                sVeil.off = 0;
                sVeil.moving = 2;
                sVeil.frames = 0;
                m4aSongNumStop(SE_M_STRENGTH);
                PlaySE(SE_M_MEGA_KICK);
                if (f & 2)
                {
                    sVeil.tire = VeilTireAt(sVeil.x[i] + sVeilDir[d][0], sVeil.y[i] + sVeilDir[d][1]);
                    sVeil.tireFrames = 0;
                    PlaySE(SE_M_ROCK_THROW);
                }
            }
        }
    }
    else
    {
        // (stopped: the bag swings on, a stack in its way falls)
        sVeil.swing = 16.0f * sinf(sVeil.frames * 0.35f) * expf(-sVeil.frames / 12.0f);
        if (sVeil.tire >= 0 && ++sVeil.tireFrames > 20)
        {
            sVeil.tires &= ~(1 << sVeil.tire);
            MPNet_Log("gym: VEILSTONE's stack %d knocked over", sVeil.tire);
            VeilStore();
            sVeil.tire = -1;
        }
        if (sVeil.frames >= 36 && sVeil.tire < 0)
            goto done;
    }
    VeilProps();
    return;
done:
    MPNet_Log("gym: VEILSTONE's bag %d at %d,%d", i, sVeil.x[i], sVeil.y[i]);
    VeilStore();
    sVeil.bag = -1;
    sVeil.swing = 0;
    VeilProps();
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// The player walks into a bag: it's hit (the script waits for it: Sinnoh_EventScript_VeilstoneBag)
bool8 S3D_Gyms_PushAt(s16 x, s16 y, u8 direction)
{
    s32 i;
    if (sG.gym != GYM_VEILSTONE || sVeil.bag >= 0 || !MP_IsLocalFree())
        return FALSE;
    i = VeilBagAt(x - MAP_OFFSET, y - MAP_OFFSET);
    if (i < 0)
        return FALSE;
    {
        extern const u8 Sinnoh_EventScript_VeilstoneBag[];
        gSpecialVar_0x8004 = i;
        gSpecialVar_0x8005 = VeilFromDir(direction);
        ScriptContext_SetupScript(Sinnoh_EventScript_VeilstoneBag);
    }
    return TRUE;
}

u16 Sinnoh_VeilstoneBag(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 d = gSpecialVar_0x8005;
    if (sG.gym != GYM_VEILSTONE || gSpecialVar_0x8004 >= ARRAY_COUNT(sVeilstoneBags))
    {
        ScriptContext_Enable();
        return 0;
    }
    sVeil.bag = gSpecialVar_0x8004, sVeil.d = d;
    sVeil.off = sVeil.swing = 0, sVeil.speed = 1.0f / 60, sVeil.frames = 0, sVeil.tiles = 0;
    sVeil.moving = VeilDistance(sVeil.x[sVeil.bag], sVeil.y[sVeil.bag], d) ? 1 : 0;
    sVeil.tire = -1;
    ObjectEventClearHeldMovementIfFinished(player);
    ObjectEventSetHeldMovement(player, (sVeil.moving ? GetWalkInPlaceSlowMovementAction : GetWalkInPlaceFastMovementAction)(
        d == 0 ? DIR_NORTH : d == 1 ? DIR_SOUTH : d == 2 ? DIR_WEST : DIR_EAST));
    PlaySE(SE_M_COMET_PUNCH);
    CreateTask(Task_VeilBag, 80);
    return 0;
}

// ------------------------------------------------------------------------------------------------------ SNOWPOINT
// An ice bowl (Platinum's): a slide speeds up down a slope and slows up one, and slides back down when it runs out of
// speed (field_player_avatar.c SnowpointSlip asks S3D_Gyms_IceSlope); a slide at speed (1+) or a moving bike breaks a
// snowball it runs into. Fences let steps by only one way. The snowballs stay broken until the player comes in from
// somewhere else (Platinum's objects: a battle keeps them).
#define SNOW_SLOT 0

static struct
{
    u32 broken;     // the snowballs broken (bits)
    u16 lastMap;
    u8 frames[32];  // each snowball breaking: its frame (0: not breaking)
    u8 task;        // the task that sinks them (0xFF none)
} sSnow;

static s32 SnowBallAt(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSnowpointBalls); i++)
        if (!((sSnow.broken >> i) & 1) && sSnowpointBalls[i][0] == x && sSnowpointBalls[i][1] == y)
            return i;
    return -1;
}

static void SnowProps(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSnowpointBalls); i++)
    {
        u8 x = sSnowpointBalls[i][0], y = sSnowpointBalls[i][1];
        float h = S3D_TileFloor(x, y, 0);
        h = h == h ? h : 0;
        if (sSnow.frames[i])
            S3D_DynProp(SNOW_SLOT + i, S3D_MODEL_SNOWPOINT_GYM_SNOWBALL, sG.ox + x * 16 + 8, h - sSnow.frames[i] * 2.5f,
                        sG.oz + y * 16 + 8, sSnow.frames[i] * 9.0f);
        else if ((sSnow.broken >> i) & 1)
            S3D_DynPropHide(SNOW_SLOT + i);
        else
            S3D_DynProp(SNOW_SLOT + i, S3D_MODEL_SNOWPOINT_GYM_SNOWBALL, sG.ox + x * 16 + 8, h, sG.oz + y * 16 + 8, 0);
    }
}

static void SnowStore(void)
{
    VarSet(VAR_SINNOH_SNOW_LO, sSnow.broken & 0xFFFF);
    VarSet(VAR_SINNOH_SNOW_HI, ((sSnow.broken >> 16) & 0x7FFF) | 0x8000);
}

static void SnowLoad(u16 map)
{
    if (sFromSave && (VarGet(VAR_SINNOH_SNOW_HI) & 0x8000))
        sSnow.broken = VarGet(VAR_SINNOH_SNOW_LO) | (u32)(VarGet(VAR_SINNOH_SNOW_HI) & 0x7FFF) << 16;   // (a save made here)
    else if (sFromSave)
        sSnow.broken = (1 << ARRAY_COUNT(sSnowpointBalls)) - 1;   // (a save made here by a build that didn't keep them:
                                                                  // every snowball broken, so nothing walls the player in)
    else if (sSnow.lastMap != map)
        sSnow.broken = 0;
    SnowStore();
    sSnow.lastMap = map;
    memset(sSnow.frames, 0, sizeof(sSnow.frames));
    sSnow.task = 0xFF;
    SnowProps();
}

static s32 SnowFence(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSnowpointFences); i++)
        if (sSnowpointFences[i][0] == x && sSnowpointFences[i][1] == y)
            return sSnowpointFences[i][2];
    return -1;
}

static bool8 SnowBlocked(s32 fx, s32 fy, s32 tx, s32 ty)
{
    s32 a = SnowFence(fx, fy), b = SnowFence(tx, ty);
    bool8 northSouth = fx == tx;
    // (Platinum's: a step is blocked by the tile it leaves or the one it enters)
    if ((a == 0 || b == 0) && northSouth)
        return TRUE;
    if ((a == 1 || b == 1) && !northSouth)
        return TRUE;
    return SnowBallAt(tx, ty) >= 0;
}

// Where a slide going DIRECTION from X, Y (map coords + 7) goes: +1 up a slope, -1 down, 0 level (Platinum's look:
// the floor half a step ahead, from its plates: sSnowpointSlope)
s8 S3D_Gyms_IceSlope(s16 x, s16 y, u8 direction)
{
    u8 d = VeilFromDir(direction), v;
    if (sG.gym != GYM_SNOWPOINT)
        return 0;
    x -= MAP_OFFSET, y -= MAP_OFFSET;
    if (x < 0 || y < 0 || x >= 32 || y >= 32)
        return 0;
    v = sSnowpointSlope[y][x * 2] , v = (u8)(((v <= '9' ? v - '0' : v - 'A' + 10) << 4)
        | (sSnowpointSlope[y][x * 2 + 1] <= '9' ? sSnowpointSlope[y][x * 2 + 1] - '0' : sSnowpointSlope[y][x * 2 + 1] - 'A' + 10));
    v = (v >> (2 * d)) & 3;
    return v == 1 ? 1 : v == 2 ? -1 : 0;
}

static void Task_SnowBreak(u8 taskId)
{
    u32 i, busy = 0;
    for (i = 0; i < ARRAY_COUNT(sSnowpointBalls); i++)
        if (sSnow.frames[i] && ++sSnow.frames[i] > 10)
            sSnow.frames[i] = 0;
        else if (sSnow.frames[i])
            busy++;
    SnowProps();
    if (busy == 0)
    {
        sSnow.task = 0xFF;
        DestroyTask(taskId);
    }
}

// Something runs into X, Y (map coords + 7) fast: a snowball there breaks
void S3D_Gyms_BreakAt(s16 x, s16 y)
{
    s32 i;
    if (sG.gym != GYM_SNOWPOINT)
        return;
    i = SnowBallAt(x - MAP_OFFSET, y - MAP_OFFSET);
    if (i < 0)
        return;
    MPNet_Log("gym: SNOWPOINT's snowball %d broken", i);
    sSnow.broken |= 1 << i;   // (out of the way at once; the pieces sink as the player goes through)
    SnowStore();
    sSnow.frames[i] = 1;
    PlaySE(SE_M_ROCK_THROW);
    if (sSnow.task == 0xFF || !FuncIsActiveTask(Task_SnowBreak))
        sSnow.task = CreateTask(Task_SnowBreak, 80);
    SnowProps();
}

// ------------------------------------------------------------------------------------------------------ HEARTHOME
// Dark rooms (Platinum's fog: the floor a fifth as bright, less with FANTINA's badge, more in the second room) where
// each person carries a lamp lighting ahead; in each trainer room one door is right (Platinum's: a new one each time
// the player comes in) and a clue on the floor somewhere in the dark shows its symbol; a wrong door leads back to the
// entrance room (S3D_Gyms_RedirectWarp from field_control_avatar.c SetupWarp).
static struct
{
    u16 lastMap;
    s8 room;          // the trainer room (0-1; -1 another of the gym's rooms)
    u8 door;          // ... its right door's symbol, the clue's tile
    u8 clueX, clueY;
} sHearth;

static bool8 HearthMap(u16 map)
{
    return map == MAP_SINNOH_HEARTHOME_CITY_GYM_ENTRANCE_ROOM || map == MAP_SINNOH_HEARTHOME_CITY_GYM_LEADER_ROOM
        || map == MAP_SINNOH_HEARTHOME_CITY_GYM_TRAINER_ROOM1 || map == MAP_SINNOH_HEARTHOME_CITY_GYM_TRAINER_ROOM2;
}

static bool8 HearthClueOk(s32 room, s32 x, s32 y)
{
    const u8 *r = sHearthRooms[room];
    u32 i;
    if (x < r[0] || y < r[1] || x >= r[0] + r[2] || y >= r[1] + r[3])
        return FALSE;
    if ((room == 0 ? sHearthClue1[y - r[1]][x - r[0]] : sHearthClue2[y - r[1]][x - r[0]]) != '0')
        return FALSE;
    for (i = 0; i < gMapHeader.events->objectEventCount; i++)   // (not under anyone: Platinum's)
        if (gMapHeader.events->objectEvents[i].x == x && gMapHeader.events->objectEvents[i].y == y)
            return FALSE;
    return TRUE;
}

static void HearthLoad(u16 map)
{
    s32 room = map == MAP_SINNOH_HEARTHOME_CITY_GYM_TRAINER_ROOM1 ? 0 : map == MAP_SINNOH_HEARTHOME_CITY_GYM_TRAINER_ROOM2 ? 1 : -1;
    if (sHearth.lastMap != map && room >= 0)
    {
        // (in from another room: Platinum draws the right door and the clue's place again)
        const u8 *r = sHearthRooms[room];
        u32 tries = 0;
        sHearth.door = r[4] + Random() % r[5];
        do
            sHearth.clueX = r[0] + Random() % r[2], sHearth.clueY = r[1] + Random() % r[3];
        while (!HearthClueOk(room, sHearth.clueX, sHearth.clueY) && ++tries < 10000);
        MPNet_Log("gym: HEARTHOME's room %d: door %d, clue at %d,%d", room + 1, sHearth.door, sHearth.clueX, sHearth.clueY);
    }
    sHearth.lastMap = map;
    sHearth.room = room;
    if (room >= 0)
    {
        float h = S3D_TileFloor(sHearth.clueX, sHearth.clueY, 0);
        S3D_DynProp(0, S3D_MODEL_HEARTHOME_GYM_HINT_CIRCLE + sHearth.door, sG.ox + sHearth.clueX * 16 + 8, (h == h ? h : 0) - 3,
                    sG.oz + sHearth.clueY * 16 + 8 - 4, 0);
    }
}

// The people's lamps where they are on the screen this frame
static void HearthLights(void)
{
    static s16 sLights[S3D_DARK_LIGHTS][3];
    u32 i, n = 0;
    float level = 0.2f;   // (Platinum's fog: 109 of 127)
    if (FlagGet(FLAG_SINNOH_BADGE01 + 4))
        level = 0.35f;    // (91: the RELIC BADGE)
    else if (sHearth.room == 1)
        level = 0.12f;    // (119)
    for (i = 0; i < OBJECT_EVENTS_COUNT && n + 2 <= S3D_DARK_LIGHTS; i++)
    {
        struct ObjectEvent *o = &gObjectEvents[i];
        struct Sprite *sp;
        s16 x, y, dx = 0, dy = 0;
        if (!o->active || o->invisible || o->spriteId >= MAX_SPRITES)
            continue;
        if (!o->isPlayer && o->trainerType == 0)
            continue;   // (the lamps are the player's and the trainers')
        sp = &gSprites[o->spriteId];
        x = sp->x + sp->x2 + gSpriteCoordOffsetX;
        y = sp->y + sp->y2 + gSpriteCoordOffsetY - sp->centerToCornerVecY - 8;
        switch (o->facingDirection)
        {
        case DIR_NORTH: dy = -1; break;
        case DIR_SOUTH: dy = 1; break;
        case DIR_WEST: dx = -1; break;
        default: dx = 1; break;
        }
        sLights[n][0] = x, sLights[n][1] = y, sLights[n][2] = 22, n++;
        sLights[n][0] = x + dx * 26, sLights[n][1] = y + dy * 22, sLights[n][2] = 30, n++;   // (the lamp's light ahead)
    }
    S3D_SetDarkness(level, (const s16 (*)[3])sLights, n);
}

void S3D_Gyms_RedirectWarp(s16 x, s16 y)
{
    const u8 (*doors)[3];
    u32 i, n;
    if (sG.gym != GYM_HEARTHOME || sHearth.room < 0)
        return;
    doors = sHearth.room == 0 ? sHearthDoors1 : sHearthDoors2;
    n = sHearth.room == 0 ? ARRAY_COUNT(sHearthDoors1) : ARRAY_COUNT(sHearthDoors2);
    for (i = 0; i < n; i++)
        if (doors[i][0] == x - MAP_OFFSET && doors[i][1] == y - MAP_OFFSET && doors[i][2] != sHearth.door)
        {
            MPNet_Log("gym: HEARTHOME's wrong door (%d): back to the entrance", doors[i][2]);
            SetWarpDestinationToMapWarp(MAP_SINNOH_HEARTHOME_CITY_GYM_ENTRANCE_ROOM >> 8,
                                        MAP_SINNOH_HEARTHOME_CITY_GYM_ENTRANCE_ROOM & 0xFF, 1);
        }
}

// A wrong door of HEARTHOME's trainer room the player is in (map coords): the walk harness's player has read the clue
bool8 S3D_Gyms_WrongDoor(s16 x, s16 y)
{
    const u8 (*doors)[3];
    u32 i, n;
    if (sG.gym != GYM_HEARTHOME || sHearth.room < 0)
        return FALSE;
    doors = sHearth.room == 0 ? sHearthDoors1 : sHearthDoors2;
    n = sHearth.room == 0 ? ARRAY_COUNT(sHearthDoors1) : ARRAY_COUNT(sHearthDoors2);
    for (i = 0; i < n; i++)
        if (doors[i][0] == x && doors[i][1] == y)
            return doors[i][2] != sHearth.door;
    return FALSE;
}

// The GREAT MARSH's tram, Platinum's run-time prop (s3d_tram.c)
void S3D_Gyms_TramProp(float x, float y, float z)
{
    S3D_DynProp(1, S3D_MODEL_GREAT_MARSH_TRAIN, x, y, z, 0);
}

// IRON ISLAND B2F's platform lift, Platinum's run-time prop (s3d_lift.c)
void S3D_Gyms_LiftProp(float x, float y, float z)
{
    S3D_DynProp(0, S3D_MODEL_IRON_ISLAND_PLATFORM_LIFT, x, y, z, 0);
}

// ------------------------------------------------------------------------------------------------------ all gyms
// ------------------------------------------------------------------------------------------------------ the VILLA
// Not a gym's, but Platinum's furniture works the same way (its villa_furniture.c, the table in sinnoh_gyms.h): each piece
// bought (its FLAG_VILLA_FURNITURE_*) stands as its model, blocks its box of tiles and, faced with A, runs its own script
// of Platinum's (scripts_villa's; the TV: SINNOH's TV broadcast, from below only)
static bool8 VillaOwns(u8 furniture)
{
    return furniture < ARRAY_COUNT(sVillaFlags) && FlagGet(sVillaFlags[furniture]);
}

static void VillaLoad(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sVillaSlots); i++)
        if (VillaOwns(sVillaSlots[i][0]))
            S3D_DynProp(i, sVillaModels[sVillaSlots[i][0]], sG.ox + sVillaSlots[i][2], 0, sG.oz + sVillaSlots[i][3], 0);
}

static s32 VillaSlotAt(s32 x, s32 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sVillaSlots); i++)
    {
        const u16 *v = sVillaSlots[i];
        if (v[1] && VillaOwns(v[0]) && x >= v[4] && x <= v[6] && y >= v[5] && y <= v[7])
            return i;
    }
    return -1;
}

#define VILLA_SCRIPT(n) extern const u8 SinnohP_villa_##n[] __attribute__((weak))
VILLA_SCRIPT(21); VILLA_SCRIPT(22); VILLA_SCRIPT(23); VILLA_SCRIPT(24); VILLA_SCRIPT(25); VILLA_SCRIPT(27);
VILLA_SCRIPT(28); VILLA_SCRIPT(29); VILLA_SCRIPT(30); VILLA_SCRIPT(31); VILLA_SCRIPT(32); VILLA_SCRIPT(33);
VILLA_SCRIPT(34); VILLA_SCRIPT(35); VILLA_SCRIPT(36); VILLA_SCRIPT(37); VILLA_SCRIPT(38); VILLA_SCRIPT(39);
VILLA_SCRIPT(40);

// (field_control_avatar.c, A facing a tile) the furniture's own script there, NULL if none
const u8 *S3D_Gyms_VillaScript(s16 x, s16 y, u8 direction)
{
    extern const u8 gSinnohTvBroadcast[];
    static const u8 *const sScripts[41] = {
        [21] = SinnohP_villa_21, [22] = SinnohP_villa_22, [23] = SinnohP_villa_23, [24] = SinnohP_villa_24,
        [25] = SinnohP_villa_25, [27] = SinnohP_villa_27, [28] = SinnohP_villa_28, [29] = SinnohP_villa_29,
        [30] = SinnohP_villa_30, [31] = SinnohP_villa_31, [32] = SinnohP_villa_32, [33] = SinnohP_villa_33,
        [34] = SinnohP_villa_34, [35] = SinnohP_villa_35, [36] = SinnohP_villa_36, [37] = SinnohP_villa_37,
        [38] = SinnohP_villa_38, [39] = SinnohP_villa_39, [40] = SinnohP_villa_40,
    };
    s32 slot;
    u8 entry;
    if (sG.gym != GYM_VILLA || (slot = VillaSlotAt(x - MAP_OFFSET, y - MAP_OFFSET)) < 0)
        return NULL;
    entry = sVillaScripts[sVillaSlots[slot][0]];
    if (entry == 0)
        return direction == DIR_NORTH ? gSinnohTvBroadcast : NULL;
    return entry < ARRAY_COUNT(sScripts) ? sScripts[entry] : NULL;
}

void Sinnoh_Signs_MapLoaded(float ox, float oz);

// The map a save was made on, loaded with it (overworld.c's continue): the gyms that keep their state with the save
// take it from there
void S3D_Gyms_SavedMapLoaded(void)
{
    sFromSave = TRUE;
    S3D_Gyms_MapLoaded();
    sFromSave = FALSE;
}

void S3D_Gyms_MapLoaded(void)
{
    u16 map = CurrentMap();
    Sinnoh_Underground_MapLoaded();   // (the Underground's dig spots: s3d_underground.c)
    S3D_DynPropsClear();
    S3D_SetDarkness(0, NULL, 0);
    gS3dHeightHook = NULL;
    gS3dObjectHeightHook = NULL;
    sG.gym = GYM_NONE;
    sG.lastX = sG.lastY = -1;
    Sinnoh_Lift_MapLoaded();   // (the platform lifts: s3d_lift.c)
    Sinnoh_Tram_MapLoaded();   // (the GREAT MARSH's tram: s3d_tram.c)
    if (!S3D_MapOrigin(&sG.ox, &sG.oz))
        return;
    Sinnoh_Signs_MapLoaded(sG.ox, sG.oz);   // (the signposts, signboards and mailboxes: s3d_signs.c)
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
    else if (map == MAP_SINNOH_CANALAVE_CITY_GYM)
    {
        sG.gym = GYM_CANALAVE;
        CanalaveLoad();
    }
    else if (map == MAP_SINNOH_ETERNA_CITY_GYM)
    {
        sG.gym = GYM_ETERNA;
        EternaLoad();
    }
    else if (map == MAP_SINNOH_VEILSTONE_CITY_GYM)
    {
        sG.gym = GYM_VEILSTONE;
        VeilLoad(map);
    }
    else if (map == MAP_SINNOH_SNOWPOINT_CITY_GYM)
    {
        sG.gym = GYM_SNOWPOINT;
        SnowLoad(map);
    }
    else if (HearthMap(map))
    {
        sG.gym = GYM_HEARTHOME;
        HearthLoad(map);
    }
    else if (map == MAP_SINNOH_VILLA)
    {
        sG.gym = GYM_VILLA;
        VillaLoad();
    }
    if (sG.gym != GYM_VEILSTONE)
        sVeil.lastMap = map;
    if (sG.gym != GYM_HEARTHOME)
        sHearth.lastMap = map;
    if (sG.gym != GYM_SNOWPOINT)
        sSnow.lastMap = map;
    if (sG.gym == GYM_PASTORIA || sG.gym == GYM_CANALAVE)
        gS3dHeightHook = HeightHook;
}

void S3D_Gyms_Update(void)
{
    s16 x, y;
    Sinnoh_Underground_Update();
    Sinnoh_Lift_Update();
    Sinnoh_Tram_Update();
    if (sG.gym == GYM_NONE)
        return;
    if (sG.gym == GYM_CANALAVE)
        CanalavePeople();
    if (sG.gym == GYM_HEARTHOME)
        HearthLights();
    if (sG.gym == GYM_ETERNA && sG.clockShown != EternaState() && MP_IsLocalFree())
    {
        // (a trainer or the leader just beaten: the clock moves)
        extern const u8 Sinnoh_EventScript_EternaClock[];
        ScriptContext_SetupScript(Sinnoh_EventScript_EternaClock);
        return;
    }
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
    else if (sG.gym == GYM_CANALAVE)
    {
        s32 i = CanalavePlatformUnder(x, y);
        if (i >= 0)
        {
            extern const u8 Sinnoh_EventScript_CanalaveRide[];
            gSpecialVar_0x8004 = i;
            ScriptContext_SetupScript(Sinnoh_EventScript_CanalaveRide);
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

// The height a step starts from: the player's, or for a step the walk harness's search plans from another tile, the
// height its plan has there (gS3dPlanHeight: NaN when no plan is being made)
float gS3dPlanHeight = NAN;

float S3D_StepFromHeight(s16 fromX, s16 fromY)
{
    if (gS3dPlanHeight == gS3dPlanHeight
     && (fromX != gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x
      || fromY != gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y))
        return gS3dPlanHeight;
    return S3D_PlayerHeight();
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
    case GYM_CANALAVE:
        blocked = sG.ride == 0 && CanalaveWall(CanalaveFloor(), toX - MAP_OFFSET, toY - MAP_OFFSET);
        break;
    case GYM_ETERNA:
        blocked = EternaBlocked(toX - MAP_OFFSET, toY - MAP_OFFSET);
        break;
    case GYM_VEILSTONE:
        blocked = VeilBlocked(toX - MAP_OFFSET, toY - MAP_OFFSET);
        break;
    case GYM_SNOWPOINT:
        blocked = SnowBlocked(fromX - MAP_OFFSET, fromY - MAP_OFFSET, toX - MAP_OFFSET, toY - MAP_OFFSET);
        break;
    case GYM_VILLA:
        blocked = VillaSlotAt(toX - MAP_OFFSET, toY - MAP_OFFSET) >= 0;
        break;
    }
    // (the rest is SINNOH's ground only: KANTO's FireRed arrow tiles share behaviors 0x54-0x57 with the bike
    // bridges, and ROCKET HIDEOUT B3F's floor walled the player in)
    if (!S3D_Active())
        return blocked;
    // Platinum's bridges (the pack's behavior 0x1D, tools/sinnoh/build_pack.py S3D_BRIDGE): up on the deck, a step
    // off its side onto ground far below is blocked (CYCLING ROAD's lanes over ROUTE 206's valley)
    if (!blocked && MapGridGetMetatileBehaviorAt(fromX, fromY) == MB_UNUSED_1D
     && MapGridGetMetatileBehaviorAt(toX, toY) != MB_UNUSED_1D)
    {
        float h = S3D_StepFromHeight(fromX, fromY), f = S3D_TileFloor(toX - MAP_OFFSET, toY - MAP_OFFSET, h);
        if (!isnan(h) && (isnan(f) || h - f > 12))
            blocked = TRUE;
    }
    // ... and the other way: under a bridge, a step onto a deck tile whose only ground is the deck far above is blocked
    // (ROUTE 211's high bridge over the river bank: (15, 21) has no ground under the deck in Platinum's BDHC, so the
    // player climbed onto it from below)
    if (!blocked && MapGridGetMetatileBehaviorAt(toX, toY) == MB_UNUSED_1D
     && ((fromX == gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x
       && fromY == gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y) || gS3dPlanHeight == gS3dPlanHeight))
    {
        float h = S3D_StepFromHeight(fromX, fromY), f = S3D_TileFloor(toX - MAP_OFFSET, toY - MAP_OFFSET, h);
        if (!isnan(h) && !isnan(f) && f - h > 12)
            blocked = TRUE;
    }
    if (!blocked)
    {
        bool8 S3D_BikeSlopeBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY);
        bool8 Sinnoh_BikeTileBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY);
        blocked = S3D_BikeSlopeBlocked(fromX, fromY, toX, toY);   // (Platinum's bike slopes: s3d_misc.c; also a step the
                                                                  // walk's search plans, from that step's own floor)
        if (!blocked && ((fromX == gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x
                       && fromY == gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y) || gS3dPlanHeight == gS3dPlanHeight))
            blocked = Sinnoh_BikeTileBlocked(fromX, fromY, toX, toY);   // (its bike bridges and ramps: s3d_bike.c; the
                                                                        // player's steps and the walk's search's)
    }
    if (getenv("MOEMON_GYM_TRACE"))
        printf("[gym] step %d,%d -> %d,%d: %s (player at %.0f)\n", fromX - MAP_OFFSET, fromY - MAP_OFFSET, toX - MAP_OFFSET,
               toY - MAP_OFFSET, blocked ? "blocked" : "free", S3D_PlayerHeight());
    return blocked;
}

static void ReportReached(const u8 *seen, u32 states);

// A trainer sees the player only on the player's floor (CANALAVE)
bool8 S3D_Gyms_SeesPlayer(u8 localId)
{
    if (sG.gym == GYM_CANALAVE && localId >= 1 && localId <= ARRAY_COUNT(sCanalaveFloorOf))
        return sCanalaveFloorOf[localId - 1] == CanalaveFloor() && !sG.ride;
    return TRUE;
}

// (tests: `mp gym`: the gym's state; `mp gym solve`: PASTORIA walked from the door by every step and button, with the
// gym's own rules and the map's walls: which trainers and the leader are reached, after how many buttons)
// ------------------------------------------------------------------------------------------------- `mp gym solve`
// SNOWPOINT for the walk-only runs: every step and slide as the game makes them (field_player_avatar.c SnowpointSlip,
// line for line, with the game's own collision, slopes and ice), from where the player stands and the snowballs
// broken now, to CANDICE: the moves and where each ends. The player's coords and the snowballs are the search's while
// it asks the game, and put back.
static bool8 SnowSimIce(s16 x, s16 y)
{
    return MetatileBehavior_IsIce_2(MapGridGetMetatileBehaviorAt(x, y));
}

static u8 SnowSimCollide(struct ObjectEvent *pl, s16 x, s16 y, u8 dir)
{
    s16 nx = x, ny = y;
    u32 i;
    pl->previousCoords = pl->currentCoords;
    pl->currentCoords.x = x, pl->currentCoords.y = y;
    ObjectEventUpdateElevation(pl);   // (as the game does with each step: SINNOH's floors change it)
    MoveCoords(dir, &nx, &ny);
    // (the gym's people, loaded or not: CANDICE, the guide, the trainers; its other objects are the snowballs' places)
    for (i = 0; i < gMapHeader.events->objectEventCount && i < 8; i++)
        if (gMapHeader.events->objectEvents[i].x + MAP_OFFSET == nx && gMapHeader.events->objectEvents[i].y + MAP_OFFSET == ny)
            return COLLISION_OBJECT_EVENT;
    return GetCollisionAtCoords(pl, nx, ny, dir);
}

// A move DIR from X, Y (map coords + 7) with the snowballs *BROKEN: where it ends (FALSE: no step at all)
static bool8 SnowSimMove(struct ObjectEvent *pl, s16 *x, s16 *y, u32 *broken, u8 dir)
{
    s32 speed = 0, steps, first = TRUE, backed = FALSE;
    s16 cx = *x, cy = *y;
    sSnow.broken = *broken;
    if (SnowSimCollide(pl, cx, cy, dir))
        return FALSE;
    MoveCoords(dir, &cx, &cy);   // (a step, as the player walks; then the ice takes over)
    for (steps = 0; steps < 200 && SnowSimIce(cx, cy); steps++)
    {
        s8 slope = S3D_Gyms_IceSlope(cx, cy, dir);
        bool8 back = FALSE;
        if (getenv("MOEMON_SNOW_TRACE"))
            printf("[snow] top %d,%d dir %d steps %d\n", cx - MAP_OFFSET, cy - MAP_OFFSET, dir, (int)steps);
        u8 collision;
        s16 nx = cx, ny = cy;
        MoveCoords(dir, &nx, &ny);
        if (!first && speed >= 1)
        {
            s32 i = SnowBallAt(nx - MAP_OFFSET, ny - MAP_OFFSET);
            if (i >= 0)
                sSnow.broken |= 1u << i;   // (CheckForObjectEventCollision: run into fast)
        }
        first = FALSE;
        collision = SnowSimCollide(pl, cx, cy, dir);
        if (getenv("MOEMON_SNOW_TRACE"))
            printf("[snow] at %d,%d dir %d slope %d speed %d collision %d elev %d\n", cx - MAP_OFFSET, cy - MAP_OFFSET, dir,
                   slope, (int)speed, collision, pl->currentElevation);
        if (backed && !collision && slope >= 0)
            break;   // (slid back to the foot of the slope: SnowpointSlip)
        if (collision)
        {
            if (slope <= 0 || collision >= COLLISION_STOP_SURFING)
                break;
            back = TRUE;
        }
        else if (slope > 0)
        {
            if (speed == 0)
                back = TRUE;
            else
                speed--;
        }
        else if (slope < 0 && speed < 3)
            speed++;
        if (back)
        {
            dir = GetOppositeDirection(dir);
            speed = 0, backed = TRUE;
            collision = SnowSimCollide(pl, cx, cy, dir);
            if (getenv("MOEMON_SNOW_TRACE"))
                printf("[snow] back from %d,%d dir %d: collision %d\n", cx - MAP_OFFSET, cy - MAP_OFFSET, dir, collision);
            if (collision)
                break;
        }
        MoveCoords(dir, &cx, &cy);
        if (getenv("MOEMON_SNOW_TRACE"))
            printf("[snow] to %d,%d ice %d behavior %02X\n", cx - MAP_OFFSET, cy - MAP_OFFSET, SnowSimIce(cx, cy),
                   MapGridGetMetatileBehaviorAt(cx, cy));
    }
    *x = cx, *y = cy, *broken = sSnow.broken;
    return TRUE;
}

static u8 SnowpointSolveRoute(bool8 print);
static void SnowpointSolve(void)
{
    SnowpointSolveRoute(TRUE);
}

static s16 sSnowGoalX = -1, sSnowGoalY;   // (gymwalk X Y: that tile; else beside CANDICE)

void MP_TestGymGoal(s16 x, s16 y)
{
    sSnowGoalX = x, sSnowGoalY = y;
}

// The first move of the way to CANDICE, or to the tile gymwalk asked for (0: there already, 0xFF: no way)
static u8 SnowpointSolveRoute(bool8 print)
{
    enum { SQ = 1 << 16, SH = 1 << 18 };
    static struct { u8 x, y, dir; u32 broken, parent; } sQ[SQ];
    static u32 sHash[SH];
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Coords16 keepCur = pl->currentCoords, keepPrev = pl->previousCoords;
    u8 keepElev = pl->currentElevation, keepPrevElev = pl->previousElevation;
    u32 keepBroken = sSnow.broken, head = 0, tail = 1, best = 0xFFFFFFFF, i, n = 0, path[256];
    const struct ObjectEventTemplate *goal = &gMapHeader.events->objectEvents[0];   // (CANDICE)
    memset(sHash, 0xFF, sizeof(sHash));
    sQ[0].x = keepCur.x, sQ[0].y = keepCur.y, sQ[0].dir = 0, sQ[0].broken = sSnow.broken, sQ[0].parent = 0xFFFFFFFF;
#define SKEY(x, y, b) (((u32)(x) - MAP_OFFSET) | ((u32)(y) - MAP_OFFSET) << 5 | (b) << 10)
#define SSEEN(k) ({ u32 hh = ((k) * 2654435761u) >> 14, hit = 0; while (sHash[hh] != 0xFFFFFFFFu) { \
        if (sHash[hh] == (k)) { hit = 1; break; } hh = (hh + 1) & (SH - 1); } if (!hit) sHash[hh] = (k); hit; })
    SSEEN(SKEY(sQ[0].x, sQ[0].y, sQ[0].broken));
    while (head < tail && tail < SQ - 4 && best == 0xFFFFFFFF)
    {
        u32 q = head++;
        u8 d;
        if (sSnowGoalX >= 0 ? (sQ[q].x - MAP_OFFSET == sSnowGoalX && sQ[q].y - MAP_OFFSET == sSnowGoalY)
                            : abs(sQ[q].x - MAP_OFFSET - goal->x) + abs(sQ[q].y - MAP_OFFSET - goal->y) == 1)
        {
            best = q;
            break;
        }
        for (d = DIR_SOUTH; d <= DIR_EAST; d++)
        {
            s16 x = sQ[q].x, y = sQ[q].y;
            u32 b = sQ[q].broken;
            if (!SnowSimMove(pl, &x, &y, &b, d) || SSEEN(SKEY(x, y, b)))
                continue;
            sQ[tail].x = x, sQ[tail].y = y, sQ[tail].dir = d, sQ[tail].broken = b, sQ[tail].parent = q;
            tail++;
        }
    }
#undef SKEY
#undef SSEEN
    pl->currentCoords = keepCur, pl->previousCoords = keepPrev;
    pl->currentElevation = keepElev, pl->previousElevation = keepPrevElev;
    sSnow.broken = keepBroken;
    if (print)
        printf("[test] gym solve: %u states\n", (unsigned)tail);
    if (best == 0xFFFFFFFF)
    {
        if (print)
            printf("[test] gym route to CANDICE: NOT reached\n");
        return 0xFF;
    }
    for (i = best; i != 0xFFFFFFFF && n < ARRAY_COUNT(path); i = sQ[i].parent)
        path[n++] = i;
    if (print)
    {
        printf("[test] gym route to CANDICE:");
        for (i = n - 1; i-- > 0;)
            printf(" %c%d,%d", "?SNWE"[sQ[path[i]].dir], sQ[path[i]].x - MAP_OFFSET, sQ[path[i]].y - MAP_OFFSET);
        printf("\n");
    }
    return n >= 2 ? sQ[path[n - 2]].dir : 0;
}

// The harness's "gymwalk" (SNOWPOINT): the key toward CANDICE whenever the player stands still in the field (0: beside
// her, 0xFFFF: none now, moving or a script), solved again each time from where the player is and what's broken
// SUNYSHORE (gymwalk FRAMES X Y): the first move toward tile X, Y of the room, every step and button (a button turns
// the room's gears as its script does) from where the player stands at the room's turn now. 0: there, 0xFF: no way.
// With no tile: on toward VOLKNER, or once his badge is won, back out room by room
static u8 SunyshoreSolveRoute(void)
{
    static u16 sFrom[4][64][32];   // (parent + 1: index into the queue; 0 not reached)
    static struct { u8 x, y, turn, dir; u16 parent; } sQ[4 * 64 * 32];
    static const s8 sDir[4][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};   // (down, up, left, right: sKeys' order - 1)
    u32 head = 0, tail = 0, i;
    s16 px, py, gx = sSnowGoalX, gy = sSnowGoalY;
    if (gx < 0)
    {
        // (no tile asked for: the room's far door, the warp nearest the top; in the last room the tile below VOLKNER,
        // the leader: the room's person nearest the top)
        u32 w;
        gy = 0x7FFF;
        if (FlagGet(MP_Region_BadgeFlag(REGION_SINNOH, 7)))
        {
            // (VOLKNER beaten: the way out, the room's door nearest the bottom, back through the gears)
            for (w = 0, gy = -1; w < gMapHeader.events->warpCount; w++)
                if (gMapHeader.events->warps[w].y > gy)
                    gx = gMapHeader.events->warps[w].x, gy = gMapHeader.events->warps[w].y;
        }
        else if (sG.room < 2)
            for (w = 0; w < gMapHeader.events->warpCount; w++)
            {
                if (gMapHeader.events->warps[w].y < gy)
                    gx = gMapHeader.events->warps[w].x, gy = gMapHeader.events->warps[w].y;
            }
        else
            for (w = 0; w < gMapHeader.events->objectEventCount; w++)
                if (gMapHeader.events->objectEvents[w].y + 1 < gy)
                    gx = gMapHeader.events->objectEvents[w].x, gy = gMapHeader.events->objectEvents[w].y + 1;
    }
    memset(sFrom, 0, sizeof(sFrom));
    PlayerGetDestCoords(&px, &py);
    sQ[0].x = px - MAP_OFFSET, sQ[0].y = py - MAP_OFFSET, sQ[0].turn = SunyshoreTurn(), sQ[0].dir = 0, sQ[0].parent = 0xFFFF;
    if (sQ[0].x == gx && sQ[0].y == gy)
        return 0;
    sFrom[sQ[0].turn][sQ[0].y][sQ[0].x] = 1;
    tail = 1;
    while (head < tail)
    {
        u8 x = sQ[head].x, y = sQ[head].y, turn = sQ[head].turn;
        for (i = 0; i < 4; i++)
        {
            s32 nx = x + sDir[i][0], ny = y + sDir[i][1], q, nt;
            u8 id;
            if (nx < 0 || ny < 0 || nx >= 32 || ny >= 64 || MapGridGetCollisionAt(nx + MAP_OFFSET, ny + MAP_OFFSET)
             || SunyshoreBlocked(nx, ny, turn))
                continue;
            id = GetObjectEventIdByXY(nx + MAP_OFFSET, ny + MAP_OFFSET);
            if (id != OBJECT_EVENTS_COUNT && id != gPlayerAvatar.objectEventId)
                continue;
            q = SunyshoreButtonAt(nx, ny);
            nt = q > 0 ? (turn + q) % 4 : turn;
            if (sFrom[nt][ny][nx])
                continue;
            sFrom[nt][ny][nx] = head + 1;
            sQ[tail].x = nx, sQ[tail].y = ny, sQ[tail].turn = nt, sQ[tail].dir = i + 1, sQ[tail].parent = head;
            if (nx == gx && ny == gy)
            {
                u32 at = tail;
                while (sQ[at].parent != 0)
                    at = sQ[at].parent;
                return sQ[at].dir;
            }
            tail++;
        }
        head++;
    }
    return 0xFF;
}

u16 MP_TestGymKey(void)
{
    static const u16 sKeys[5] = {0, DPAD_DOWN, DPAD_UP, DPAD_LEFT, DPAD_RIGHT};
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 d;
    if (sG.gym != GYM_SNOWPOINT && sG.gym != GYM_SUNYSHORE)
        return 0;
    if (!MP_IsInOverworld() || ScriptContext_IsEnabled() || (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_FORCED_MOVE)
     || (pl->heldMovementActive && !pl->heldMovementFinished))
        return 0xFFFF;
    d = sG.gym == GYM_SUNYSHORE ? SunyshoreSolveRoute() : SnowpointSolveRoute(FALSE);
    if (d == 0xFF)
        return 0xFFFF;
    return sKeys[d];
}

void S3D_Gyms_Debug(const char *arg)
{
    printf("[test] gym %d: water %.0f (level %d) room %d turn %d floor %d moved %06X clock %d\n", sG.gym, sG.water,
           PastoriaLevel(), sG.room + 1, SunyshoreTurn(), CanalaveFloor(), CanalaveMoved(), sG.clockShown);
    if (sG.gym == GYM_VEILSTONE)
    {
        char line[256];
        u32 i, n = 0;
        for (i = 0; i < ARRAY_COUNT(sVeilstoneBags); i++)
            n += snprintf(line + n, sizeof(line) - n, " %d,%d", sVeil.x[i], sVeil.y[i]);
        printf("[test] VEILSTONE bags%s stacks %03X\n", line, sVeil.tires);
    }
    if (sG.gym == GYM_SNOWPOINT)
        printf("[test] SNOWPOINT snowballs broken %05X\n", sSnow.broken);
    if (sG.gym == GYM_HEARTHOME)
        printf("[test] HEARTHOME room %d door %d clue %d,%d\n", sHearth.room + 1, sHearth.door, sHearth.clueX, sHearth.clueY);
    if (sG.gym == GYM_PASTORIA && strcmp(arg, "solve") == 0)
    {
        static u8 sSeen[3][64][32];
        static struct { u8 x, y, level, presses; float h; u16 parent; } sQueue[3 * 64 * 32];
        static const s8 sDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        u32 head = 0, tail = 0, i;
        s32 w = gBackupMapLayout.width - 2 * MAP_OFFSET, hgt = gBackupMapLayout.height - 2 * MAP_OFFSET;
        memset(sSeen, 0, sizeof(sSeen));
        // from the door (13, 41), the water as Platinum starts it (middle)
        sQueue[tail].x = 13, sQueue[tail].y = 41, sQueue[tail].level = 1, sQueue[tail].presses = 0;
        sQueue[tail].h = S3D_TileFloor(13, 41, 64.0f);
        sQueue[tail].parent = 0xFFFF;
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
                sQueue[tail].parent = head - 1;
                tail++;
            }
        }
        ReportReached(&sSeen[0][0][0], 3);
        // (and the way to each: the buttons in the order they're stepped on, then the tile beside the person; a walk
        // goes button to button, the water at one level between them)
        for (i = 0; i < gMapHeader.events->objectEventCount; i++)
        {
            const struct ObjectEventTemplate *t = &gMapHeader.events->objectEvents[i];
            u32 q, best = 0xFFFF, n = 0, k;
            u16 path[64];
            for (q = 0; q < tail; q++)
                if (abs(sQueue[q].x - t->x) + abs(sQueue[q].y - t->y) == 1
                 && (best == 0xFFFF || sQueue[q].presses < sQueue[best].presses))
                    best = q;
            if (best == 0xFFFF || t->flagId == 0x3FFF)
                continue;
            for (q = best; q != 0xFFFF && n < ARRAY_COUNT(path); q = sQueue[q].parent)
                if (q == best || (sQueue[q].parent != 0xFFFF && sQueue[sQueue[q].parent].level != sQueue[q].level))
                    path[n++] = q;
            printf("[test] gym route to person %u:", (unsigned)i + 1);
            for (k = n; k-- > 0;)
                printf(" %d,%d", sQueue[path[k]].x, sQueue[path[k]].y);
            printf("\n");
        }
    }
    if (sG.gym == GYM_CANALAVE && strcmp(arg, "solve") == 0)
    {
        // from where the player stands, as the gym moves (each floor's walls; a platform stepped on carries the player
        // to its other end): the way to each person as the tiles a walk aims for, the corners of each straight run
        // (one way only to each) and each platform's tile ("+": it rides from there)
        enum { CQ = 1 << 16, CH = 1 << 18 };
        static struct { u8 x, y, fl, ride; u32 moved, parent; } sQ[CQ];
        static u64 sHash[CH];
        static const s8 sDir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        u32 head = 0, tail = 0, i, o;
        s16 px, py;
        memset(sHash, 0xFF, sizeof(sHash));
        PlayerGetDestCoords(&px, &py);
        sQ[0].x = px - MAP_OFFSET, sQ[0].y = py - MAP_OFFSET, sQ[0].fl = CanalaveFloor(), sQ[0].ride = 0;
        sQ[0].moved = CanalaveMoved(), sQ[0].parent = 0xFFFFFFFF;
        tail = 1;
#define CKEY(x, y, f, m) ((u64)(x) | (u64)(y) << 5 | (u64)(f) << 10 | (u64)(m) << 12)
#define CSEEN(k, add) ({ u32 hh = (u32)(((k) * 0x9E3779B97F4A7C15ull) >> 46), hit = 0; \
        while (sHash[hh] != ~0ull) { if (sHash[hh] == (k)) { hit = 1; break; } hh = (hh + 1) & (CH - 1); } \
        if (!hit && (add)) sHash[hh] = (k); hit; })
        CSEEN(CKEY(sQ[0].x, sQ[0].y, sQ[0].fl, sQ[0].moved), 1);
        while (head < tail && tail < CQ - 4)
        {
            u8 x = sQ[head].x, y = sQ[head].y, fl = sQ[head].fl;
            u32 moved = sQ[head].moved;
            head++;
            for (i = 0; i < 4; i++)
            {
                s32 nx = x + sDir[i][0], ny = y + sDir[i][1], k, lx, ly, lf = fl, ride = 0;
                u32 nm = moved;
                if (CanalaveWall(fl, nx, ny))
                    continue;
                lx = nx, ly = ny;
                for (k = 0; k < (s32)ARRAY_COUNT(sCanalavePlatforms); k++)
                {
                    const struct CanalavePlatform *p = &sCanalavePlatforms[k];
                    bool8 b = p->startB ^ ((moved >> k) & 1);
                    if ((b ? p->bx : p->ax) == nx && (b ? p->bz : p->az) == ny && (b ? p->by : p->ay) / 10 == fl)
                    {
                        lx = b ? p->ax : p->bx, ly = b ? p->az : p->bz, lf = (b ? p->ay : p->by) / 10;
                        nm = moved ^ (1u << k), ride = 1;
                        break;
                    }
                }
                if (CSEEN(CKEY(lx, ly, lf, nm), 1))
                    continue;
                sQ[tail].x = lx, sQ[tail].y = ly, sQ[tail].fl = lf, sQ[tail].moved = nm, sQ[tail].parent = head - 1;
                sQ[tail].ride = ride ? 1 + i : 0;   // (the step that rode: its direction + 1)
                tail++;
            }
        }
        printf("[test] gym solve: %u states\n", (unsigned)tail);
        // (each person: a tile beside them on their floor; then each door: its own tile on the ground floor)
        for (o = 0; o < gMapHeader.events->objectEventCount + gMapHeader.events->warpCount; o++)
        {
            bool8 door = o >= gMapHeader.events->objectEventCount;
            const struct ObjectEventTemplate *t = door ? NULL : &gMapHeader.events->objectEvents[o];
            const struct WarpEvent *e = door ? &gMapHeader.events->warps[o - gMapHeader.events->objectEventCount] : NULL;
            s32 tx = door ? e->x : t->x, ty = door ? e->y : t->y;
            u8 f = door ? 0 : o < ARRAY_COUNT(sCanalaveFloorOf) ? sCanalaveFloorOf[o] : 0;
            u32 q, best = 0xFFFFFFFF, n = 0, path[256], k, num = door ? o - gMapHeader.events->objectEventCount : o + 1;
            for (q = 0; q < tail && best == 0xFFFFFFFF; q++)
                if (sQ[q].fl == f && (door ? abs(sQ[q].x - tx) + abs(sQ[q].y - ty) <= 1 : abs(sQ[q].x - tx) + abs(sQ[q].y - ty) == 1))
                    best = q;
            if (best == 0xFFFFFFFF)
            {
                printf("[test] gym route to %s %u: NOT reached\n", door ? "door" : "person", (unsigned)num);
                continue;
            }
            for (q = best; q != 0xFFFFFFFF && n < ARRAY_COUNT(path); q = sQ[q].parent)
                path[n++] = q;
            printf("[test] gym route to %s %u (floor %u):", door ? "door" : "person", (unsigned)num, f);
            // (path[n-1] the start ... path[0] the end)
            for (k = n - 1; k-- > 0;)
            {
                u32 s = path[k], prev = path[k + 1];
                if (sQ[s].ride)
                {
                    // the platform's tile: one step from the last state, the way it rode
                    printf(" %d,%d+", sQ[prev].x + sDir[sQ[s].ride - 1][0], sQ[prev].y + sDir[sQ[s].ride - 1][1]);
                    continue;
                }
                // a corner: where the next step turns, rides, or the end
                if (k == 0 || sQ[path[k - 1]].ride
                 || (sQ[path[k - 1]].x - sQ[s].x) != (sQ[s].x - sQ[prev].x)
                 || (sQ[path[k - 1]].y - sQ[s].y) != (sQ[s].y - sQ[prev].y))
                    printf(" %d,%d", sQ[s].x, sQ[s].y);
            }
            printf("\n");
        }
#undef CKEY
#undef CSEEN
    }
    if (sG.gym == GYM_SNOWPOINT && strcmp(arg, "solve") == 0)
        SnowpointSolve();
    if (sG.gym == GYM_SNOWPOINT && strncmp(arg, "sim ", 4) == 0)
    {
        // (mp gym sim MOVES: the search's own moves from here, N S W E, to check them against the game's)
        struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
        struct Coords16 keepCur = pl->currentCoords, keepPrev = pl->previousCoords;
        u8 keepElev = pl->currentElevation, keepPrevElev = pl->previousElevation;
        u32 keep = sSnow.broken, b = sSnow.broken;
        s16 x = keepCur.x, y = keepCur.y;
        const char *m;
        printf("[test] gym sim:");
        for (m = arg + 4; *m; m++)
        {
            u8 d = *m == 'N' ? DIR_NORTH : *m == 'S' ? DIR_SOUTH : *m == 'W' ? DIR_WEST : *m == 'E' ? DIR_EAST : 0;
            if (d == 0)
                continue;
            SnowSimMove(pl, &x, &y, &b, d);
            printf(" %c%d,%d", *m, x - MAP_OFFSET, y - MAP_OFFSET);
        }
        printf(" (broken %05X)\n", b);
        pl->currentCoords = keepCur, pl->previousCoords = keepPrev;
        pl->currentElevation = keepElev, pl->previousElevation = keepPrevElev;
        sSnow.broken = keep;
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
