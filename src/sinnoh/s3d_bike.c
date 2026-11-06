// Moemon Kai: SINNOH's BICYCLE as Platinum's (the user, 2026-11-06: "Return the unique Sinnoh Bicycle's functionality to
// normal"). In SINNOH the BICYCLE (item 360, ridden as the MACH BIKE elsewhere) is Platinum's own:
// - two gears, B switches between them (a click): 3rd rides fast, 4th faster; it stops when let go (no MACH BIKE glide);
// - muddy bike slopes are ridden up in 4th gear only (s3d_misc.c S3D_BikeSlopeBlocked);
// - the narrow bike bridges (ROUTEs 210, 212, 227, 228: Platinum's behaviors 0x76-0x7D, the pack's 0x54 north-south /
//   0x55 east-west, tools/sinnoh/build_pack.py) are ridden only along them, never on foot, and nobody gets off there
//   (item_use.c ItemUseOutOfBattle_Bike);
// - the jump ramps (0xD7 / 0xD8, the pack's 0x56 eastward / 0x57 westward: ROUTE 227) jump the bike over, on foot
//   they're in the way.
#include "global.h"
#include "bike.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "sound.h"
#include "mp.h"
#include "s3d.h"
#include "constants/event_object_movement.h"
#include "constants/metatile_behaviors.h"
#include "constants/songs.h"

static u8 sGear = 4;   // (Platinum's 3rd or 4th gear; 4th to start with, as fast as the bike rode before)

u8 Sinnoh_BikeGear(void)
{
    return sGear;
}

static bool8 OnSinnohBicycle(void)
{
    return S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) && (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_MACH_BIKE);
}

static bool8 AlongBridge(u8 behavior, u8 direction)
{
    if (behavior == MB_UNUSED_54)
        return direction == DIR_NORTH || direction == DIR_SOUTH;
    if (behavior == MB_UNUSED_55)
        return direction == DIR_EAST || direction == DIR_WEST;
    return TRUE;
}

static void Still(u8 direction)
{
    PlayerFaceDirection(direction);
    gPlayerAvatar.bikeFrameCounter = 0;
    gPlayerAvatar.bikeSpeed = PLAYER_SPEED_STANDING;
    gPlayerAvatar.runningState = NOT_MOVING;
}

// bike.c MovePlayerOnBike: TRUE when the BICYCLE moved the player its own way here (in SINNOH)
bool8 Sinnoh_MovePlayerOnBike(u8 direction, u16 newKeys, u16 heldKeys)
{
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = pl->currentCoords.x, y = pl->currentCoords.y;
    u8 ahead, collision;
    if (!OnSinnohBicycle())
        return FALSE;
    if (newKeys & B_BUTTON)
    {
        sGear = sGear == 4 ? 3 : 4;
        PlaySE(SE_CLICK);
        MPNet_Log("sinnoh bike: gear %d", sGear);
    }
    if (direction == DIR_NONE)
    {
        Still(pl->facingDirection);
        return TRUE;
    }
    if (!AlongBridge(pl->currentMetatileBehavior, direction))
    {
        Still(pl->facingDirection);   // (on a bike bridge: only along it)
        return TRUE;
    }
    MoveCoords(direction, &x, &y);
    ahead = MapGridGetMetatileBehaviorAt(x, y);
    if ((ahead == MB_UNUSED_56 && direction == DIR_EAST) || (ahead == MB_UNUSED_57 && direction == DIR_WEST))
    {
        PlayerJumpLedge(direction);   // (a ramp: over it)
        MPNet_Log("sinnoh bike: ramp jump");
        return TRUE;
    }
    collision = CheckForObjectEventCollision(pl, x, y, direction, ahead);
    if (collision == COLLISION_LEDGE_JUMP)
    {
        PlayerJumpLedge(direction);
        return TRUE;
    }
    if (collision != COLLISION_NONE)
    {
        if (collision < COLLISION_STOP_SURFING || collision > COLLISION_ROTATING_GATE)
            PlayerOnBikeCollide(direction);
        else
            PlayerFaceDirection(direction);
        gPlayerAvatar.bikeSpeed = PLAYER_SPEED_STANDING;
        gPlayerAvatar.runningState = NOT_MOVING;
        return TRUE;
    }
    if (sGear == 4)
        PlayerWalkFaster(direction);
    else
        PlayerWalkFast(direction);
    gPlayerAvatar.bikeSpeed = sGear == 4 ? PLAYER_SPEED_FASTEST : PLAYER_SPEED_FAST;
    gPlayerAvatar.runningState = MOVING;
    return TRUE;
}

// s3d_gyms.c S3D_Gyms_StepBlocked, for the player's own steps: the bike bridges (on a bike and along them only) and
// the ramps (not on foot)
bool8 Sinnoh_BikeTileBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    u8 to = MapGridGetMetatileBehaviorAt(toX, toY), from = MapGridGetMetatileBehaviorAt(fromX, fromY), dir;
    bool8 biking = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE);
    if (toX > fromX)
        dir = DIR_EAST;
    else if (toX < fromX)
        dir = DIR_WEST;
    else if (toY > fromY)
        dir = DIR_SOUTH;
    else
        dir = DIR_NORTH;
    if (to == MB_UNUSED_54 || to == MB_UNUSED_55)
    {
        if (!biking && from != MB_UNUSED_54 && from != MB_UNUSED_55)
            return TRUE;   // (not on foot: Platinum's bridges are a bicycle's)
        if (!AlongBridge(to, dir))
            return TRUE;
    }
    if (!AlongBridge(from, dir))
        return TRUE;
    if ((to == MB_UNUSED_56 || to == MB_UNUSED_57) && !biking)
        return TRUE;
    return FALSE;
}
