// Moemon Co-op: piggyback. Face a friend and press A to hop on their back:
// you ride along wherever they go (walking, biking, SURF, through doors and
// on FLY), drawn on top of them. Press B to hop off onto a free spot of land.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "mp.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/songs.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

enum {
    MPR_PIG_ON = MPR_SOCIAL_FIRST + 5,   // rider -> carrier: I hopped on
    MPR_PIG_OFF,                         // rider -> carrier: I hopped off
};

#define RIDE_Y_OFFSET   (-12)
#define WARP_SETTLE     30   // frames the carrier must stay put on another map before we follow

static s8 sCarrier = -1;          // who we ride, -1 = nobody
static u16 sAwayFrames;
static bool8 sWarping;
static bool8 sRemoteRiding[MP_MAX_PLAYERS];

bool8 MP_Piggy_IsRiding(void)
{
    return sCarrier >= 0;
}

static void Name(u8 id, char *out)
{
    MP_GameToAscii(out, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
}

static bool8 CarrierValid(void)
{
    return sCarrier >= 0 && sCarrier < MP_MAX_PLAYERS && sCarrier != gMpLocalId
        && gMpPlayers[sCarrier].present && gMpPlayers[sCarrier].hasState;
}

static bool8 IsLandTile(s16 x, s16 y)
{
    u16 behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MapGridGetCollisionAt(x, y) != 0)
        return FALSE;
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || MetatileBehavior_IsDeepOrOceanWater(behavior))
        return FALSE;
    return GetObjectEventIdByXY(x, y) == OBJECT_EVENTS_COUNT;
}

static void StopRiding(bool8 tell)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (tell && CarrierValid())
    {
        u8 b = MPR_PIG_OFF;
        MP_SendRelay(sCarrier, &b, 1);
    }
    sCarrier = -1;
    sAwayFrames = 0;
    if (gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT)
        gSprites[me->spriteId].y2 = 0;
}

// B while riding: jump down next to them
static void TryHopOff(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    static const u8 sOrder[4] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH};
    u8 i, first = GetOppositeDirection(me->facingDirection);
    for (i = 0; i < 5; i++)
    {
        u8 dir = (i == 0) ? first : sOrder[i - 1];
        s16 x = me->currentCoords.x, y = me->currentCoords.y;
        MoveCoords(dir, &x, &y);
        if (!IsLandTile(x, y))
            continue;
        if (ObjectEventIsMovementOverridden(me) && !ObjectEventClearHeldMovementIfFinished(me))
            return;   // wait for the current step to finish
        StopRiding(TRUE);
        ObjectEventSetHeldMovement(me, GetJumpMovementAction(dir));
        PlaySE(SE_LEDGE);
        return;
    }
    PlaySE(SE_FAILURE);
    MP_Events_QueueToast("No land to hop down onto here!");
}

// DoCB1_Overworld while riding: only B does anything
void MP_Piggy_FieldInput(u16 newKeys)
{
    if (newKeys & B_BUTTON)
        TryHopOff();
}

// Follow the carrier one step at a time (or warp to them on another map)
static void Follow(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct MpState *st = &gMpPlayers[sCarrier].state;
    s16 tx, ty, dx, dy;

    if (!MP_GetRemoteMapPos(sCarrier, &tx, &ty))
    {
        // They went through a door, flew, or fell into a hole: follow once they're settled
        if ((st->flags & MPF_IN_OVERWORLD) && ++sAwayFrames >= WARP_SETTLE && !sWarping && MP_IsLocalFree())
        {
            sWarping = TRUE;
            SetWarpDestination(st->mapGroup, st->mapNum, WARP_ID_NONE, st->x - MAP_OFFSET, st->y - MAP_OFFSET);
            LockPlayerFieldControls();
            DoWarp();
        }
        return;
    }
    sAwayFrames = 0;
    sWarping = FALSE;
    dx = tx - me->currentCoords.x;
    dy = ty - me->currentCoords.y;
    if (dx == 0 && dy == 0)
    {
        if (!ObjectEventIsMovementOverridden(me) && me->facingDirection != st->facing && st->facing >= DIR_SOUTH && st->facing <= DIR_EAST)
            SetObjectEventDirection(me, st->facing);
        return;
    }
    if (ObjectEventIsMovementOverridden(me) && !ObjectEventClearHeldMovementIfFinished(me))
        return;
    if (abs(dx) + abs(dy) == 1)
    {
        u8 dir = dx > 0 ? DIR_EAST : dx < 0 ? DIR_WEST : dy > 0 ? DIR_SOUTH : DIR_NORTH;
        ObjectEventSetHeldMovement(me, st->speed >= MP_SPEED_FAST ? GetWalkFastMovementAction(dir) : GetWalkNormalMovementAction(dir));
    }
    else if (MP_IsLocalFree())
    {
        // Fell behind: catch up with a quick warp on the same map
        sWarping = TRUE;
        SetWarpDestination(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE,
                           tx - MAP_OFFSET, ty - MAP_OFFSET);
        LockPlayerFieldControls();
        DoWarp();
    }
}

// Every overworld frame, after the camera moved
void MP_Piggy_Update(void)
{
    u8 i;
    if (!MP_IsInOverworld())
        return;
    // Friends riding on someone sit higher up, on top of them
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        u8 obj = MP_GetRemoteObjectEventId(i);
        bool8 riding = i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState
                    && (gMpPlayers[i].state.flags & MPF_RIDING);
        if (obj < OBJECT_EVENTS_COUNT && gObjectEvents[obj].active)
        {
            struct Sprite *s = &gSprites[gObjectEvents[obj].spriteId];
            if (riding)
            {
                s->y2 = RIDE_Y_OFFSET;
                s->subpriority = 0;
            }
            else if (sRemoteRiding[i])
                s->y2 = 0;
        }
        sRemoteRiding[i] = riding;
    }
    if (sCarrier < 0)
        return;
    if (!CarrierValid() || !MP_IsConnected())
    {
        StopRiding(FALSE);
        MP_ShowNotice("Your ride left!");
        return;
    }
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return;
    Follow();
    gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].y2 = RIDE_Y_OFFSET;
    gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].subpriority = 0;
}

void MP_Piggy_OnRelay(u8 from, const u8 *data, u16 len)
{
    char name[16], msg[64];
    if (from >= MP_MAX_PLAYERS)
        return;
    Name(from, name);
    if (data[0] == MPR_PIG_ON)
        snprintf(msg, sizeof(msg), "%s hopped on your back!", name);
    else if (data[0] == MPR_PIG_OFF)
        snprintf(msg, sizeof(msg), "%s hopped off.", name);
    else
        return;
    MP_Events_QueueToast(msg);
    MP_Social_Log(name, data[0] == MPR_PIG_ON ? "hopped on for a piggyback ride" : "hopped off");
}

void MP_Piggy_OnPlayerLeft(u8 id)
{
    sRemoteRiding[id] = FALSE;
}

// ---------------------------------------------------------------------------
// "Hop on BOB's back?" when pressing A in front of a friend

#define tState  data[0]
#define tTarget data[1]

static EWRAM_DATA u8 sPrompt[64] = {0};

static void Task_RidePrompt(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        HideMapNamePopUpWindow();
        MP_ShowFieldMessageFitted(sPrompt);
        tState++;
        break;
    case 1:
        if (!IsTextPrinterActive(0))
        {
            DisplayYesNoMenuDefaultYes();
            tState++;
        }
        break;
    case 2:
    {
        s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        HideFieldMessageBox();
        UnlockPlayerFieldControls();
        if (answer == 0 && gMpPlayers[tTarget].present && !(gMpPlayers[tTarget].state.flags & (MPF_IN_BATTLE | MPF_RIDING)))
        {
            u8 b = MPR_PIG_ON;
            sCarrier = tTarget;
            sAwayFrames = 0;
            sWarping = FALSE;
            MP_SendRelay(tTarget, &b, 1);
            PlaySE(SE_LEDGE);
        }
        DestroyTask(taskId);
        break;
    }
    }
}

// From the overworld update: A facing a friend who isn't battling
bool8 MP_Piggy_TryInteract(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;

    if (!JOY_NEW(A_BUTTON) || sCarrier >= 0 || !MP_IsLocalFree() || !MP_IsConnected() || FuncIsActiveTask(Task_RidePrompt))
        return FALSE;
    if (gPlayerAvatar.flags & (PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER | PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 rx, ry;
        char name[16], text[64];
        u8 taskId;
        if (i == gMpLocalId || !gMpPlayers[i].present || !gMpPlayers[i].hasState)
            continue;
        if (gMpPlayers[i].state.flags & (MPF_IN_BATTLE | MPF_RIDING | MPF_CHALLENGED | MPF_TRAINER_MODE))
            continue;
        if (!MP_GetRemoteMapPos(i, &rx, &ry) || rx != x || ry != y)
            continue;
        Name(i, name);
        snprintf(text, sizeof(text), "Hop on %s's back for a ride?\n({B} hops off)", name);
        MP_AsciiToGame(sPrompt, text, sizeof(sPrompt) - 1);
        LockPlayerFieldControls();
        taskId = CreateTask(Task_RidePrompt, 80);
        gTasks[taskId].tTarget = i;
        return TRUE;
    }
    return FALSE;
}
