// Moemon Kai: SINNOH's platform lifts, as Platinum has them (pret/pokeplatinum src/platform_lift.c): IRON ISLAND's
// three and the POKéMON LEAGUE's six (the elevators up to each of the ELITE FOUR's rooms and the CHAMPION's, the
// CHAMPION's room's own). A 3 x 2 plate of tiles at the lift's floor height; Platinum's map scripts start it
// (InitPersistedMapFeaturesForPlatformLift, CheckPlatformLiftNotUsedWhenEnteredMap, TriggerPlatformLift: `sinnoh
// SINNOH_LIFT`, tools/sinnoh/gen_scripts.py), the player rides it up or down (the LEAGUE's go up only), the lift's prop
// with them: the map's own prop raised by s3d.c S3D_SetLift, or on IRON ISLAND B2F's left room Platinum's run-time one.
// Which floor it stands at on arrival: where the player came in (the bottom floor's warp row or not).
#include <math.h>
#include <stdio.h>
#include "global.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "global.fieldmap.h"
#include "m4a.h"
#include "main.h"
#include "script.h"
#include "sound.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "mp.h"
#include "s3d.h"

#define TILE 16.0f
#define LIFT_W 3
#define LIFT_H 2

enum { KIND_IRON_ISLAND, KIND_LEAGUE };

static const struct
{
    u16 map;
    u8 floors[2];        // (tiles up: the bottom floor's, the top's)
    u8 x, z;             // (the plate's corner, Platinum's tiles in its matrix)
    bool8 upOnly, runtimeProp;
    u8 kind;
    u8 bottomWarpZ;      // (the bottom floor's warp row, Platinum's tiles: arriving there, the lift is down)
} sLifts[] = {
    {MAP_SINNOH_IRON_ISLAND_B1F_RIGHT_ROOM, {1, 9}, 10, 23, FALSE, FALSE, KIND_IRON_ISLAND, 26},
    {MAP_SINNOH_IRON_ISLAND_B2F_LEFT_ROOM, {1, 9}, 18, 44, FALSE, TRUE, KIND_IRON_ISLAND, 32 + 16},
    {MAP_SINNOH_IRON_ISLAND_B3F, {1, 9}, 8, 11, FALSE, FALSE, KIND_IRON_ISLAND, 15},
    {MAP_SINNOH_POKEMON_LEAGUE_ELEVATOR_TO_AARON_ROOM, {0, 10}, 3, 11, TRUE, FALSE, KIND_LEAGUE, 15},
    {MAP_SINNOH_POKEMON_LEAGUE_ELEVATOR_TO_BERTHA_ROOM, {0, 5}, 3, 11, TRUE, FALSE, KIND_LEAGUE, 15},
    {MAP_SINNOH_POKEMON_LEAGUE_ELEVATOR_TO_FLINT_ROOM, {0, 5}, 3, 11, TRUE, FALSE, KIND_LEAGUE, 15},
    {MAP_SINNOH_POKEMON_LEAGUE_ELEVATOR_TO_LUCIAN_ROOM, {0, 5}, 3, 11, TRUE, FALSE, KIND_LEAGUE, 15},
    {MAP_SINNOH_POKEMON_LEAGUE_ELEVATOR_TO_CHAMPION_ROOM, {0, 5}, 3, 19, TRUE, FALSE, KIND_LEAGUE, 23},
    {MAP_SINNOH_POKEMON_LEAGUE_CHAMPION_ROOM, {0, 15}, 7, 8, TRUE, FALSE, KIND_LEAGUE, 18},
};

static struct
{
    s8 lift;             // (sLifts' index; -1: no lift here)
    u8 floor;            // (0 bottom, 1 top)
    bool8 notUsedOnArrival, riding;
    float y, target;
    s32 cx, cz;          // (the map's corner cell in its matrix)
    s16 restX, restY;    // (where the player stood when the ride ended: no new ride before they step off)
} sL = {-1, 0, 0, 0, 0, 0, 0, 0, -1, -1};

extern float (*gS3dHeightHook)(s32 mapX, s32 mapY, float h);
void S3D_Gyms_LiftProp(float x, float y, float z);   // (s3d_gyms.c: the run-time lift's model number is its table's)

static bool8 OnPlate(s32 x, s32 y)
{
    s32 px = x + sL.cx * 32, pz = y + sL.cz * 32;
    return px >= sLifts[sL.lift].x && px < sLifts[sL.lift].x + LIFT_W && pz >= sLifts[sL.lift].z
        && pz < sLifts[sL.lift].z + LIFT_H;
}

// the plate's tiles stand at the lift's height (Platinum's DynamicTerrainHeightManager plate)
static float Height(s32 x, s32 y, float near)
{
    if (sL.lift < 0 || !OnPlate(x, y))
        return NAN;
    return sL.y;
}

static void Show(void)
{
    float x0 = sLifts[sL.lift].x * TILE, z0 = sLifts[sL.lift].z * TILE;
    if (sLifts[sL.lift].runtimeProp)
    {
        // (Platinum's position for it: 16 * 19 + 8, 16 * (32 + 12) + 8)
        S3D_Gyms_LiftProp(16 * 19 + 8, sL.y, 16 * (32 + 12) + 8);
        S3D_SetLift(FALSE, 0, 0, 0, 0, 0);
    }
    else
        S3D_SetLift(TRUE, x0, z0, x0 + LIFT_W * TILE, z0 + LIFT_H * TILE, sL.y);   // (its prop stands on the plate)
}

// (InitPersistedMapFeaturesForPlatformLift) the lift's floor by where the player came in
static void Init(void)
{
    s32 cx = 0, cz = 0;
    S3D_MapCells(&cx, &cz);
    sL.cx = cx, sL.cz = cz;
    sL.floor = gSaveBlock1Ptr->pos.y + cz * 32 == sLifts[sL.lift].bottomWarpZ ? 0 : 1;
    sL.notUsedOnArrival = sL.floor == 0 || sLifts[sL.lift].kind != KIND_LEAGUE;
    sL.y = sLifts[sL.lift].floors[sL.floor] * TILE;
    sL.riding = FALSE;
    sL.restX = sL.restY = -1;
    Show();
}

// the lift of the map the player is on (-1: none); a new map's is set up by where the player came in, the same map's
// (back from a battle or a menu) kept as it was, as Platinum's persisted map features
static s8 Find(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    if (sL.lift >= 0 && sLifts[sL.lift].map == map)
        return sL.lift;
    sL.lift = -1;
    for (i = 0; i < ARRAY_COUNT(sLifts); i++)
        if (sLifts[i].map == map)
        {
            sL.lift = i;
            Init();
            break;
        }
    return sL.lift;
}

void Sinnoh_Lift_MapLoaded(void)
{
    S3D_SetLift(FALSE, 0, 0, 0, 0, 0);
    if (Find() >= 0)
    {
        gS3dHeightHook = Height;
        Show();
    }
}

void Sinnoh_Lift_Update(void)
{
    float step;
    if (sL.lift < 0)
        return;
    Show();
    if (sL.restX >= 0 && (gSaveBlock1Ptr->pos.x != sL.restX || gSaveBlock1Ptr->pos.y != sL.restY))
        sL.restX = sL.restY = -1;
    if (!sL.riding)
        return;
    step = sLifts[sL.lift].kind == KIND_IRON_ISLAND ? 1.0f : 2.0f;
    if (sL.target > sL.y)
        sL.y = sL.y + step >= sL.target ? sL.target : sL.y + step;
    else
        sL.y = sL.y - step <= sL.target ? sL.target : sL.y - step;
    if (sL.y == sL.target)
    {
        sL.riding = FALSE;
        sL.restX = gSaveBlock1Ptr->pos.x;   // (the step onto the plate counts once: the game asks the tile again as
        sL.restY = gSaveBlock1Ptr->pos.y;   // the script ends, Platinum doesn't)
        gPlayerAvatar.runningState = NOT_MOVING;   // (... every frame, the player frozen: that step is over)
        m4aSongNumStop(SE_ELEVATOR);
        PlaySE(SE_BANG);
        MPNet_Log("sinnoh lift: at floor %u (%d)", sL.floor, (int)sL.y);
        ScriptContext_Enable();
    }
}

// `sinnoh SINNOH_LIFT, OP, VAR`: 0 Init (InitPersistedMapFeaturesForPlatformLift), 1 VAR = not used when the player
// came in (CheckPlatformLiftNotUsedWhenEnteredMap), 2 the ride (TriggerPlatformLift: TRUE while it goes, the script
// waits for it)
bool8 Sinnoh_LiftCall(u16 op, u16 var)
{
    if (Find() < 0)   // (a map's transition script runs before its own load: s3d_gyms.c)
        return FALSE;
    switch (op)
    {
    case 0:
        Init();
        break;
    case 1:
        VarSet(var, sL.notUsedOnArrival);
        break;
    case 2:
        if ((sLifts[sL.lift].upOnly && sL.floor == 1)
         || (gSaveBlock1Ptr->pos.x == sL.restX && gSaveBlock1Ptr->pos.y == sL.restY))
            return FALSE;
        sL.floor ^= 1;
        sL.target = sLifts[sL.lift].floors[sL.floor] * TILE;
        sL.riding = TRUE;
        PlaySE(SE_ELEVATOR);
        MPNet_Log("sinnoh lift: going %s", sL.floor ? "up" : "down");
        return TRUE;
    }
    return FALSE;
}

void Sinnoh_LiftDebug(void)
{
    printf("[test] lift: %d floor %u y %d riding %d\n", sL.lift, sL.floor, (int)sL.y, sL.riding);
}
