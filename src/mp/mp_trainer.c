// Moemon Co-op: Trainer Mode (BATTLE FLAG key item)
//
// A player who plants the BATTLE FLAG stands still and looks around like an NPC
// trainer. When another player walks into their line of sight, the trainer gets
// a "!", walks up to them and a real link battle starts.
//
// Handshake (relayed through the server):
//   trainer -> target  MPR_SPOT            "you've been spotted"
//   target  -> trainer MPR_SPOT_ACK x,y    target froze at this tile
//                      (or MPR_SPOT_DENY if busy)
//   trainer walks up
//   trainer -> target  MPR_ARRIVED money
//   target  -> trainer MPR_BATTLE_GO money
//   both start the battle (lower player id is the link master)
#include "global.h"
#include "mp.h"
#include "main.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "money.h"
#include "overworld.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/songs.h"


static const u8 sText_FlagOffline[] = _("You're not connected to the co-op\nserver, so nobody can see your flag.");
static const u8 sText_FlagNobody[] = _("Nobody else is online right now.\nTry again when a friend joins!");
static const u8 sText_FlagPlanted[] = _("{PLAYER} planted the BATTLE FLAG!\nWaiting for a challenger… ({B_BUTTON} stop)");
static const u8 sText_FlagNoParty[] = _("You need a MOéMON that can battle\nbefore you can challenge anyone.");

enum {
    TM_OFF,
    TM_WATCHING,     // standing guard, looking around
    TM_SPOTTING,     // sent SPOT, waiting for an answer
    TM_WALKING,      // walking up to the target
    TM_WAIT_GO,      // arrived, waiting for the target to confirm
    TM_STARTING,     // battle is starting
};

enum {
    SP_NONE,
    SP_FREEZING,     // stopping the player before answering
    SP_WAIT_TRAINER, // frozen, waiting for the trainer to walk up
    SP_STARTING,
};

static struct
{
    u8 state;
    u8 taskId;
    u8 target;
    u16 timer;
    u16 lookTimer;
    u16 cooldown[MP_MAX_PLAYERS];
    s16 goalX, goalY;      // target's frozen position (our map frame)
    u8 steps;
    u32 oppMoney;
} sTM;

static struct
{
    u8 state;
    u8 taskId;
    u8 trainer;
    u16 timer;
    u32 oppMoney;
} sSpot;

static bool8 sGaveFlag;

bool8 MP_Trainer_IsActive(void)
{
    return sTM.state != TM_OFF;
}

bool8 MP_Trainer_IsEngaged(void)
{
    return (sTM.state >= TM_SPOTTING) || sSpot.state != SP_NONE;
}

static bool8 HasBattleableMon(void)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (species != SPECIES_NONE && species != SPECIES_EGG && GetMonData(&gPlayerParty[i], MON_DATA_HP) > 0)
            return TRUE;
    }
    return FALSE;
}

bool8 MP_Trainer_CanPlantFlag(void)
{
    return MP_IsConnected() && MP_ConnectedCount() >= 2 && HasBattleableMon() && !MP_Battle_IsActive();
}

const u8 *MP_Trainer_FlagUseMessage(void)
{
    if (!MP_IsConnected())
        return sText_FlagOffline;
    if (MP_ConnectedCount() < 2)
        return sText_FlagNobody;
    if (!HasBattleableMon())
        return sText_FlagNoParty;
    return sText_FlagPlanted;
}

static struct ObjectEvent *Player(void)
{
    return &gObjectEvents[gPlayerAvatar.objectEventId];
}

static bool8 PlayerIsStill(void)
{
    struct ObjectEvent *p = Player();
    if (p->heldMovementActive && !p->heldMovementFinished)
        return FALSE;
    return gPlayerAvatar.tileTransitionState == T_NOT_MOVING || gPlayerAvatar.tileTransitionState == T_TILE_CENTER;
}

static void PlayerFace(u8 dir)
{
    struct ObjectEvent *p = Player();
    ObjectEventClearHeldMovementIfFinished(p);
    if (!ObjectEventIsHeldMovementActive(p))
        ObjectEventSetHeldMovement(p, GetFaceDirectionMovementAction(dir));
}

static u8 DirTowards(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    s16 dx = toX - fromX, dy = toY - fromY;
    if (abs(dx) >= abs(dy))
        return dx > 0 ? DIR_EAST : DIR_WEST;
    return dy > 0 ? DIR_SOUTH : DIR_NORTH;
}

static void SendSimple(u8 target, u8 type)
{
    MP_SendRelay(target, &type, 1);
}

static void SendWithMoney(u8 target, u8 type)
{
    u8 buf[5];
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    buf[0] = type;
    buf[1] = money; buf[2] = money >> 8; buf[3] = money >> 16; buf[4] = money >> 24;
    MP_SendRelay(target, buf, 5);
}

static u32 ReadMoney(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static void ShowExclamationOverPlayer(void)
{
    gFieldEffectArguments[0] = LOCALID_PLAYER;
    gFieldEffectArguments[1] = gSaveBlock1Ptr->location.mapNum;
    gFieldEffectArguments[2] = gSaveBlock1Ptr->location.mapGroup;
    FieldEffectStart(FLDEFF_EXCLAMATION_MARK_ICON);
    PlaySE(SE_PIN);
    MP_TriggerLocalEmote(MP_EMOTE_EXCLAMATION);
}

// ---------------------------------------------------------------------------
// Trainer side

static void Task_TrainerMode(u8 taskId);

static void EndTrainerMode(bool8 unlock)
{
    if (sTM.taskId != TASK_NONE && FuncIsActiveTask(Task_TrainerMode))
        DestroyTask(sTM.taskId);
    sTM.taskId = TASK_NONE;
    if (sTM.state >= TM_SPOTTING && sTM.state < TM_STARTING)
        SendSimple(sTM.target, MPR_CANCEL);
    sTM.state = TM_OFF;
    if (unlock)
    {
        Overworld_PlaySpecialMapMusic();
        UnlockPlayerFieldControls();
    }
}

void MP_Trainer_Begin(void)
{
    memset(sTM.cooldown, 0, sizeof(sTM.cooldown));
    sTM.state = TM_WATCHING;
    sTM.lookTimer = 60 + (Random() % 90);
    sTM.taskId = CreateTask(Task_TrainerMode, 80);
    LockPlayerFieldControls();
    MPNet_Log("trainer mode on");
}

void MP_Trainer_Toggle(void)
{
    if (sTM.state == TM_OFF)
        MP_Trainer_Begin();
    else if (sTM.state == TM_WATCHING)
        EndTrainerMode(TRUE);
}

// Is the remote player standing in our line of sight?
static bool8 CanSee(u8 id, s16 *outX, s16 *outY)
{
    struct MpPlayer *p = &gMpPlayers[id];
    struct ObjectEvent *me = Player();
    s16 rx, ry, x, y;
    s16 dx, dy, dist, i;
    u8 dir = me->facingDirection;

    if (!p->present || !p->hasState || sTM.cooldown[id])
        return FALSE;
    if ((p->state.flags & (MPF_IN_OVERWORLD | MPF_FREE)) != (MPF_IN_OVERWORLD | MPF_FREE))
        return FALSE;
    if (p->state.flags & (MPF_TRAINER_MODE | MPF_ENGAGED | MPF_IN_BATTLE))
        return FALSE;
    if (p->state.partyCount == 0)
        return FALSE;
    if (!MP_GetRemoteMapPos(id, &rx, &ry))
        return FALSE;

    dx = rx - me->currentCoords.x;
    dy = ry - me->currentCoords.y;
    switch (dir)
    {
    case DIR_SOUTH: if (dx != 0 || dy <= 0) return FALSE; dist = dy; break;
    case DIR_NORTH: if (dx != 0 || dy >= 0) return FALSE; dist = -dy; break;
    case DIR_EAST:  if (dy != 0 || dx <= 0) return FALSE; dist = dx; break;
    case DIR_WEST:  if (dy != 0 || dx >= 0) return FALSE; dist = -dx; break;
    default: return FALSE;
    }
    if (dist > gMpConfig.trainerRange)
        return FALSE;

    // Nothing solid in between
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    for (i = 1; i < dist; i++)
    {
        MoveCoords(dir, &x, &y);
        if (MapGridGetCollisionAt(x, y))
            return FALSE;
    }
    *outX = rx;
    *outY = ry;
    return TRUE;
}

static void TryStepTowardsGoal(void)
{
    struct ObjectEvent *me = Player();
    s16 dx = sTM.goalX - me->currentCoords.x;
    s16 dy = sTM.goalY - me->currentCoords.y;
    u8 dir;

    if (abs(dx) + abs(dy) <= 1 || sTM.steps > 16)
    {
        // Arrived next to the target
        PlayerFace(DirTowards(me->currentCoords.x, me->currentCoords.y, sTM.goalX, sTM.goalY));
        SendWithMoney(sTM.target, MPR_ARRIVED);
        sTM.state = TM_WAIT_GO;
        sTM.timer = 0;
        return;
    }
    // Walk along the axis we're facing first, like NPC trainers do
    if (dx != 0 && (dy == 0 || me->facingDirection == DIR_EAST || me->facingDirection == DIR_WEST))
        dir = dx > 0 ? DIR_EAST : DIR_WEST;
    else
        dir = dy > 0 ? DIR_SOUTH : DIR_NORTH;
    // Stop one tile short of the target
    if ((dir == DIR_EAST || dir == DIR_WEST) && dy == 0 && abs(dx) == 1)
        return;
    if ((dir == DIR_NORTH || dir == DIR_SOUTH) && dx == 0 && abs(dy) == 1)
        return;
    ObjectEventSetHeldMovement(me, GetWalkNormalMovementAction(dir));
    sTM.steps++;
}

static void Task_TrainerMode(u8 taskId)
{
    struct ObjectEvent *me;
    u8 i;

    if (!MP_IsInOverworld())
        return;
    me = Player();
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sTM.cooldown[i])
            sTM.cooldown[i]--;

    switch (sTM.state)
    {
    case TM_WATCHING:
        if (JOY_NEW(B_BUTTON) || !MP_IsConnected())
        {
            PlaySE(SE_SELECT);
            MPNet_Log("trainer mode off");
            EndTrainerMode(TRUE);
            return;
        }
        if (!PlayerIsStill())
            break;
        ObjectEventClearHeldMovementIfFinished(me);
        for (i = 0; i < MP_MAX_PLAYERS; i++)
        {
            s16 x, y;
            if (i == gMpLocalId)
                continue;
            if (CanSee(i, &x, &y))
            {
                sTM.target = i;
                sTM.state = TM_SPOTTING;
                sTM.timer = 0;
                ShowExclamationOverPlayer();
                SendSimple(i, MPR_SPOT);
                MPNet_Log("spotted player %d", i + 1);
                return;
            }
        }
        // Look around like an NPC trainer
        if (--sTM.lookTimer == 0)
        {
            u8 dir = DIR_SOUTH + (Random() % 4);
            if (dir == me->facingDirection)
                dir = (dir == DIR_EAST) ? DIR_SOUTH : dir + 1;
            PlayerFace(dir);
            sTM.lookTimer = 60 + (Random() % 120);
        }
        break;
    case TM_SPOTTING:
        if (++sTM.timer > 60 * 5)
        {
            MPNet_Log("player %d didn't answer", sTM.target + 1);
            SendSimple(sTM.target, MPR_CANCEL);
            sTM.cooldown[sTM.target] = 60 * 4;
            sTM.state = TM_WATCHING;
            sTM.lookTimer = 60;
        }
        break;
    case TM_WALKING:
        if (ObjectEventIsHeldMovementActive(me) && !ObjectEventClearHeldMovementIfFinished(me))
            break;
        TryStepTowardsGoal();
        break;
    case TM_WAIT_GO:
        if (++sTM.timer > 60 * 8)
        {
            SendSimple(sTM.target, MPR_CANCEL);
            sTM.cooldown[sTM.target] = 60 * 6;
            sTM.state = TM_WATCHING;
            sTM.lookTimer = 60;
        }
        break;
    case TM_STARTING:
        break;
    }
}

// ---------------------------------------------------------------------------
// Target side (we got spotted)

static void Task_Spotted(u8 taskId);

static void EndSpotted(bool8 unlock)
{
    if (sSpot.taskId != TASK_NONE && FuncIsActiveTask(Task_Spotted))
        DestroyTask(sSpot.taskId);
    sSpot.taskId = TASK_NONE;
    sSpot.state = SP_NONE;
    if (unlock)
    {
        Overworld_PlaySpecialMapMusic();
        UnlockPlayerFieldControls();
    }
}

static void Task_Spotted(u8 taskId)
{
    struct ObjectEvent *me = Player();
    switch (sSpot.state)
    {
    case SP_FREEZING:
        if (!PlayerIsStill())
            break;
        {
            u8 buf[7];
            buf[0] = MPR_SPOT_ACK;
            buf[1] = gSaveBlock1Ptr->location.mapGroup;
            buf[2] = gSaveBlock1Ptr->location.mapNum;
            buf[3] = me->currentCoords.x & 0xFF;
            buf[4] = (u16)me->currentCoords.x >> 8;
            buf[5] = me->currentCoords.y & 0xFF;
            buf[6] = (u16)me->currentCoords.y >> 8;
            MP_SendRelay(sSpot.trainer, buf, sizeof(buf));
        }
        PlayNewMapMusic(MUS_ENCOUNTER_INTENSE);
        sSpot.state = SP_WAIT_TRAINER;
        sSpot.timer = 0;
        break;
    case SP_WAIT_TRAINER:
    {
        s16 tx, ty;
        // Keep facing the trainer as they approach
        if (MP_GetRemoteMapPos(sSpot.trainer, &tx, &ty) && (sSpot.timer % 8) == 0)
        {
            u8 dir = DirTowards(me->currentCoords.x, me->currentCoords.y, tx, ty);
            if (dir != me->facingDirection)
                PlayerFace(dir);
        }
        if (++sSpot.timer > 60 * 20)
        {
            MPNet_Log("trainer never arrived, giving up");
            SendSimple(sSpot.trainer, MPR_CANCEL);
            EndSpotted(TRUE);
        }
        break;
    }
    case SP_STARTING:
        break;
    }
}

static void HandleSpot(u8 from)
{
    if (sSpot.state != SP_NONE || sTM.state != TM_OFF || MP_Battle_IsActive()
     || !MP_IsLocalFree() || !HasBattleableMon())
    {
        SendSimple(from, MPR_SPOT_DENY);
        return;
    }
    sSpot.trainer = from;
    sSpot.state = SP_FREEZING;
    sSpot.timer = 0;
    LockPlayerFieldControls();
    sSpot.taskId = CreateTask(Task_Spotted, 80);
    MPNet_Log("spotted by player %d!", from + 1);
}

// ---------------------------------------------------------------------------

void MP_Trainer_OnRelay(u8 from, const u8 *data, u16 len)
{
    switch (data[0])
    {
    case MPR_SPOT:
        HandleSpot(from);
        break;
    case MPR_SPOT_ACK:
        if (sTM.state == TM_SPOTTING && from == sTM.target && len >= 7)
        {
            u8 group = data[1], num = data[2];
            s16 x = (s16)(data[3] | (data[4] << 8));
            s16 y = (s16)(data[5] | (data[6] << 8));
            s16 gx, gy;
            // Convert the frozen position into our map frame through the remote's state
            struct MpState saved = gMpPlayers[from].state;
            gMpPlayers[from].state.mapGroup = group;
            gMpPlayers[from].state.mapNum = num;
            gMpPlayers[from].state.x = x;
            gMpPlayers[from].state.y = y;
            if (!MP_GetRemoteMapPos(from, &gx, &gy))
            {
                gMpPlayers[from].state = saved;
                SendSimple(from, MPR_CANCEL);
                sTM.state = TM_WATCHING;
                sTM.lookTimer = 60;
                break;
            }
            gMpPlayers[from].state = saved;
            sTM.goalX = gx;
            sTM.goalY = gy;
            sTM.steps = 0;
            sTM.state = TM_WALKING;
            PlayNewMapMusic(MUS_ENCOUNTER_INTENSE);
        }
        break;
    case MPR_SPOT_DENY:
        if (sTM.state == TM_SPOTTING && from == sTM.target)
        {
            sTM.cooldown[from] = 60 * 4;
            sTM.state = TM_WATCHING;
            sTM.lookTimer = 60;
        }
        break;
    case MPR_ARRIVED:
        if (sSpot.state == SP_WAIT_TRAINER && from == sSpot.trainer && len >= 5)
        {
            sSpot.oppMoney = ReadMoney(data + 1);
            SendWithMoney(from, MPR_BATTLE_GO);
            sSpot.state = SP_STARTING;
            EndSpotted(FALSE);
            MP_Battle_Start(from, gMpLocalId < from, sSpot.oppMoney);
        }
        break;
    case MPR_BATTLE_GO:
        if (sTM.state == TM_WAIT_GO && from == sTM.target && len >= 5)
        {
            sTM.oppMoney = ReadMoney(data + 1);
            sTM.state = TM_STARTING;
            if (sTM.taskId != TASK_NONE && FuncIsActiveTask(Task_TrainerMode))
                DestroyTask(sTM.taskId);
            sTM.taskId = TASK_NONE;
            sTM.state = TM_OFF;
            MP_Battle_Start(from, gMpLocalId < from, sTM.oppMoney);
        }
        break;
    case MPR_CANCEL:
        if (sSpot.state != SP_NONE && from == sSpot.trainer)
            EndSpotted(TRUE);
        if (sTM.state >= TM_SPOTTING && sTM.state < TM_STARTING && from == sTM.target)
        {
            sTM.cooldown[from] = 60 * 4;
            sTM.state = TM_WATCHING;
            sTM.lookTimer = 60;
        }
        break;
    }
}

void MP_Trainer_OnPlayerLeft(u8 id)
{
    if (sSpot.state != SP_NONE && sSpot.trainer == id)
        EndSpotted(TRUE);
    if (sTM.state >= TM_SPOTTING && sTM.state < TM_STARTING && sTM.target == id)
    {
        sTM.state = TM_WATCHING;
        sTM.lookTimer = 60;
    }
}

void MP_Trainer_Update(void)
{
    if (!MP_IsInOverworld())
        return;
    // Hand out the BATTLE FLAG once we're online
    if (!sGaveFlag && MP_IsConnected())
    {
        sGaveFlag = TRUE;
        if (!CheckBagHasItem(ITEM_BATTLE_FLAG, 1))
        {
            AddBagItem(ITEM_BATTLE_FLAG, 1);
            MPNet_Log("added the BATTLE FLAG to your key items");
        }
    }
    // Disconnected while on guard
    if (sTM.state == TM_WATCHING && !MP_IsConnected())
        EndTrainerMode(TRUE);
}
