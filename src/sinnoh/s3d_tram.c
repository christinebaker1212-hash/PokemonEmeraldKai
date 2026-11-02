#include <math.h>
// Moemon Kai: the GREAT MARSH's quick tram, as Platinum has it (pret/pokeplatinum src/overlay006/great_marsh_tram.c):
// its car (Platinum's great_marsh_train model, loaded at run time) waits at one of three stops along the track (tile
// x 66: z 40 by AREAS 1 / 2, 82 by 3 / 4, 108 by 5 / 6); a station's script (Platinum's safari_game.s, `sinnoh
// SINNOH_TRAM`) calls it there when it's elsewhere (it glides in, slowing to a stop) and rides it to another stop (the
// player walks the track on it, faster then slower, the car under them). Its stop is kept while the SAFARI GAME goes on
// (InitGreatMarshTram: AREAS 1 / 2).
#include <stdio.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_player_avatar.h"
#include "m4a.h"
#include "script.h"
#include "sound.h"
#include "constants/event_object_movement.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "mp.h"
#include "s3d.h"

#define TRAM_X 1056.0f
#define TRAM_Y 16.0f
#define AT_LOCATION 5
#define NOT_AT_LOCATION 6
#define MOVE_CALL 3

void S3D_Gyms_TramProp(float x, float y, float z);   // (s3d_gyms.c: the tram's model number is its table's)

static const float sStops[3] = {16.0f * (32 * 1 + 8), 16.0f * (32 * 2 + 18), 16.0f * (32 * 3 + 12)};
static const u8 sSlowFrames[] = {2, 4, 4, 8, 8, 16, 32};
static const float sCallSpeeds[] = {8, 4, 4, 2, 2, 1, 0.5f};
static const u8 sRideNorth[] = {MOVEMENT_ACTION_WALK_SLOW_UP, MOVEMENT_ACTION_WALK_SLOW_UP, MOVEMENT_ACTION_WALK_NORMAL_UP,
                                MOVEMENT_ACTION_WALK_NORMAL_UP, MOVEMENT_ACTION_WALK_FAST_UP, MOVEMENT_ACTION_WALK_FAST_UP,
                                MOVEMENT_ACTION_WALK_FASTER_UP};
static const u8 sRideSouth[] = {MOVEMENT_ACTION_WALK_SLOW_DOWN, MOVEMENT_ACTION_WALK_SLOW_DOWN,
                                MOVEMENT_ACTION_WALK_NORMAL_DOWN, MOVEMENT_ACTION_WALK_NORMAL_DOWN,
                                MOVEMENT_ACTION_WALK_FAST_DOWN, MOVEMENT_ACTION_WALK_FAST_DOWN,
                                MOVEMENT_ACTION_WALK_FASTER_DOWN};

static struct
{
    u8 location;           // (0: AREAS 1 / 2, 1: 3 / 4, 2: 5 / 6)
    bool8 here;            // (on a GREAT MARSH map)
    u8 moving;             // (0 no, 3 called, 4 ridden)
    bool8 south;
    u8 speed, frames;
    float z, dest;
    s32 slowDownZ;
    float frozenH;         // (Platinum's SetPlayerHeightCalculationEnabled FALSE: the player keeps the platform's height
                           // on the car; NaN: the ground's)
} sT;

extern float (*gS3dHeightHook)(s32 mapX, s32 mapY, float h);

// the track's tiles while the player rides: the height they boarded at (the car's deck), as Platinum's
static float Height(s32 x, s32 y, float near)
{
    if (sT.frozenH == sT.frozenH && x >= 64 && x <= 68)
        return sT.frozenH;
    return NAN;
}

static bool8 InMarsh(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    return map == MAP_SINNOH_GREAT_MARSH1 || map == MAP_SINNOH_GREAT_MARSH2 || map == MAP_SINNOH_GREAT_MARSH3
        || map == MAP_SINNOH_GREAT_MARSH4 || map == MAP_SINNOH_GREAT_MARSH5 || map == MAP_SINNOH_GREAT_MARSH6;
}

static void Show(void)
{
    if (sT.here)
        S3D_Gyms_TramProp(TRAM_X, TRAM_Y, sT.z);
}

void Sinnoh_Tram_MapLoaded(void)
{
    sT.here = InMarsh();
    sT.moving = 0;
    sT.frozenH = NAN;
    if (sT.here)
        gS3dHeightHook = Height;
    sT.z = sStops[sT.location % 3];
    Show();
}

static float PlayerZ(void)
{
    // (where the player is in the marsh's world, smooth while walking: the marsh's maps start at its matrix's corner)
    float x, z;
    S3D_PlayerWorld(&x, &z);
    return z;
}

static void Arrive(void)
{
    sT.moving = 0;
    sT.z = sT.dest;
    m4aSongNumStop(SE_TRUCK_MOVE);
    PlaySE(SE_TRUCK_STOP);
    MPNet_Log("sinnoh tram: at stop %u", sT.location);
    ScriptContext_Enable();
}

void Sinnoh_Tram_Update(void)
{
    if (!sT.here)
        return;
    if (sT.moving == MOVE_CALL)
    {
        // (Platinum's CallTramToPlayer: full speed, then slower steps past the stop's slow-down point)
        float tileZ = (sT.z - 8.0f) / 16.0f;
        if (sT.south)
        {
            sT.z += sCallSpeeds[sT.speed];
            if (sT.speed < ARRAY_COUNT(sSlowFrames) - 1 && tileZ >= sT.slowDownZ && ++sT.frames >= sSlowFrames[sT.speed])
                sT.speed++, sT.frames = 0;
            if (sT.z >= sT.dest)
                Arrive();
        }
        else
        {
            sT.z -= sCallSpeeds[sT.speed];
            if (sT.speed < ARRAY_COUNT(sSlowFrames) - 1 && tileZ <= sT.slowDownZ && ++sT.frames >= sSlowFrames[sT.speed])
                sT.speed++, sT.frames = 0;
            if (sT.z <= sT.dest)
                Arrive();
        }
    }
    else if (sT.moving)
    {
        // (Platinum's RideTramToArea: the player walks the track, the car under them, faster, then slower)
        struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
        s32 tile = pl->currentCoords.y - MAP_OFFSET;
        sT.z = PlayerZ() + 8.0f;
        if (ObjectEventIsHeldMovementActive(pl) && !ObjectEventClearHeldMovementIfFinished(pl))
        {
            Show();
            return;
        }
        if ((sT.south && (tile + 1) * 16 >= sT.dest) || (!sT.south && (tile + 1) * 16 <= sT.dest))
        {
            Arrive();
            Show();
            return;
        }
        if (sT.south ? tile < sT.slowDownZ : tile > sT.slowDownZ)
        {
            if (sT.speed < ARRAY_COUNT(sRideSouth) - 1)
                sT.speed++;
        }
        else if (sT.speed)
            sT.speed--;
        ObjectEventSetHeldMovement(pl, sT.south ? sRideSouth[sT.speed] : sRideNorth[sT.speed]);
    }
    Show();
}

// `sinnoh SINNOH_TRAM, OP, A, B`: 0 InitGreatMarshTram, 1 MoveGreatMarshTram A (a variable: the stop), B (3 call, 4 ride:
// TRUE, the script waits), 2 CheckGreatMarshTramLocation A (a stop), B (a variable <- 5 there, 6 elsewhere), 3
// SetPlayerHeightCalculationEnabled A
bool8 Sinnoh_TramCall(u16 op, u16 a, u16 b)
{
    static const u8 sSlowDown[3][2] = {{32 * 1 + 14, 32 * 1 + 14}, {32 * 2 + 10, 32 * 2 + 24}, {32 * 3 + 4, 32 * 3 + 4}};
    switch (op)
    {
    case 0:
        sT.location = 0;
        sT.z = sStops[0];
        break;
    case 1:
    {
        u8 dest = VarGet(a) % 3;
        if (dest == sT.location || !sT.here)
            return FALSE;
        sT.south = dest > sT.location;
        sT.dest = sStops[dest];
        sT.slowDownZ = dest == 1 ? sSlowDown[1][sT.south ? 0 : 1] : sSlowDown[dest][0];
        sT.location = dest;
        sT.moving = b;
        sT.speed = 0;
        sT.frames = 0;
        if (b != MOVE_CALL)
            PlaySE(SE_TRUCK_MOVE);
        MPNet_Log("sinnoh tram: %s to stop %u", b == MOVE_CALL ? "called" : "ridden", dest);
        return TRUE;
    }
    case 2:
        VarSet(b, sT.location == a ? AT_LOCATION : NOT_AT_LOCATION);
        break;
    case 3:   // SetPlayerHeightCalculationEnabled A
        sT.frozenH = a ? NAN : S3D_PlayerHeight();
        break;
    }
    return FALSE;
}

void Sinnoh_TramDebug(void)
{
    printf("[test] tram: stop %u z %d moving %u here %d\n", sT.location, (int)sT.z, sT.moving, sT.here);
}
