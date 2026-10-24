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
//   trainer -> target  MPR_ARRIVED money, phrases
//   target  -> trainer MPR_BATTLE_GO money, phrases
//   both show the trainer's challenge phrase (A or a few seconds closes it)
//   each side   ->     MPR_PHRASE_READY when its phrase box is closed
//   both start the battle when both are ready (lower player id is the link master)
// Phrases: the save's battle phrases (gSaveBlock1Ptr->easyChatBattleStart / Won / Lost, also the Battle Tower's),
// 3 x 6 easy chat words. The winner's result message quotes the loser's LOSE phrase and the loser's the winner's WIN.
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
#include "constants/easy_chat.h"
#include "easy_chat.h"
#include "event_object_lock.h"
#include "constants/rgb.h"
#include "field_message_box.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "item_menu.h"
#include "map_name_popup.h"
#include "menu.h"
#include "palette.h"
#include "string_util.h"
#include "text.h"
#include "window.h"


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
    TM_PHRASE,       // our challenge phrase is up / waiting for the target to close it
    TM_STARTING,     // battle is starting
};

enum {
    SP_NONE,
    SP_FREEZING,     // stopping the player before answering
    SP_WAIT_TRAINER, // frozen, waiting for the trainer to walk up
    SP_PHRASE,       // the trainer's challenge phrase is up / waiting for the trainer to close it
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
    u16 oppPhrases[3][EASY_CHAT_BATTLE_WORDS_COUNT];
    bool8 boxClosed, otherReady;
} sTM;

static struct
{
    u8 state;
    u8 taskId;
    u8 trainer;
    u16 timer;
    u32 oppMoney;
    u16 oppPhrases[3][EASY_CHAT_BATTLE_WORDS_COUNT];
    bool8 boxClosed, otherReady;
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

#define PHRASE_BYTES (3 * EASY_CHAT_BATTLE_WORDS_COUNT * 2)
enum { PHRASE_START, PHRASE_WON, PHRASE_LOST };

static const u16 *OwnPhrase(u8 which)
{
    if (which == PHRASE_START)
        return gSaveBlock1Ptr->easyChatBattleStart;
    if (which == PHRASE_WON)
        return gSaveBlock1Ptr->easyChatBattleWon;
    return gSaveBlock1Ptr->easyChatBattleLost;
}

// MPR_ARRIVED / MPR_BATTLE_GO: money, then our three phrases (u16 words)
static void SendWithMoney(u8 target, u8 type)
{
    u8 buf[5 + PHRASE_BYTES];
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u8 which, i, n = 5;
    buf[0] = type;
    buf[1] = money; buf[2] = money >> 8; buf[3] = money >> 16; buf[4] = money >> 24;
    for (which = 0; which < 3; which++)
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
        {
            buf[n++] = OwnPhrase(which)[i];
            buf[n++] = OwnPhrase(which)[i] >> 8;
        }
    MP_SendRelay(target, buf, n);
}

static u32 ReadMoney(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static void ReadPhrases(const u8 *p, u16 len, u16 out[3][EASY_CHAT_BATTLE_WORDS_COUNT])
{
    u8 which, i;
    for (which = 0; which < 3; which++)
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
        {
            u16 at = 5 + (which * EASY_CHAT_BATTLE_WORDS_COUNT + i) * 2;
            out[which][i] = at + 1 < len ? (p[at] | (p[at + 1] << 8)) : EC_EMPTY_WORD;
        }
}

static bool8 PhraseEmpty(const u16 *words)
{
    u8 i;
    for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
        if (words[i] != EC_EMPTY_WORD)
            return FALSE;
    return TRUE;
}

// "NAME: the phrase" (one line: the message box wraps it). FALSE for an empty phrase.
bool8 MP_Trainer_QuotePhrase(u8 *dest, const u8 *name, const u16 *words)
{
    static const u8 sColon[] = _(": ");
    u8 *end;
    if (PhraseEmpty(words))
        return FALSE;
    end = StringCopyN(dest, name, MP_NAME_LEN);
    *end = EOS;
    end = StringAppend(dest, sColon);
    ConvertEasyChatWordsToString(end, words, EASY_CHAT_BATTLE_WORDS_COUNT, 1);
    return TRUE;
}

static EWRAM_DATA u8 sPhraseText[160] = {0};

// The challenge phrase box before a flag battle (both games show the trainer's). FALSE: nothing to show.
static bool8 ShowChallenge(const u8 *name, const u16 *words)
{
    if (!MP_Trainer_QuotePhrase(sPhraseText, name, words))
        return FALSE;
    HideMapNamePopUpWindow();
    MP_ShowFieldMessageFitted(sPhraseText);
    MPNet_Log("challenge phrase shown");
    return TRUE;
}

// TRUE once the box is closed (A / B, or about three seconds after the text is out)
static bool8 UpdateChallenge(bool8 *closed, u16 *timer)
{
    if (*closed)
        return TRUE;
    if (IsTextPrinterActive(0))
        return FALSE;
    if (JOY_NEW(A_BUTTON | B_BUTTON) || ++*timer > 60 * 3)
    {
        HideFieldMessageBox();
        *closed = TRUE;
    }
    return *closed;
}

static void CloseChallengeBox(void)
{
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
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
    if (sTM.state == TM_PHRASE)
        CloseChallengeBox();
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
    case TM_PHRASE:
    {
        bool8 wasClosed = sTM.boxClosed;
        if (UpdateChallenge(&sTM.boxClosed, &sTM.timer) && !wasClosed)
        {
            SendSimple(sTM.target, MPR_PHRASE_READY);
            sTM.timer = 0;
        }
        if (sTM.boxClosed && sTM.otherReady)
        {
            u8 target = sTM.target;
            sTM.state = TM_STARTING;
            DestroyTask(taskId);
            sTM.taskId = TASK_NONE;
            sTM.state = TM_OFF;
            MP_Battle_Start(target, gMpLocalId < target, sTM.oppMoney);
            MP_Battle_SetOppPhrases(sTM.oppPhrases[PHRASE_WON], sTM.oppPhrases[PHRASE_LOST]);
            return;
        }
        if (sTM.boxClosed && ++sTM.timer > 60 * 20)
        {
            MPNet_Log("player %d never closed the challenge", sTM.target + 1);
            SendSimple(sTM.target, MPR_CANCEL);
            sTM.cooldown[sTM.target] = 60 * 6;
            sTM.state = TM_WATCHING;
            sTM.lookTimer = 60;
        }
        break;
    }
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
    if (sSpot.state == SP_PHRASE)
        CloseChallengeBox();
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
    case SP_PHRASE:
    {
        bool8 wasClosed = sSpot.boxClosed;
        if (UpdateChallenge(&sSpot.boxClosed, &sSpot.timer) && !wasClosed)
        {
            SendSimple(sSpot.trainer, MPR_PHRASE_READY);
            sSpot.timer = 0;
        }
        if (sSpot.boxClosed && sSpot.otherReady)
        {
            u8 trainer = sSpot.trainer;
            sSpot.state = SP_STARTING;
            EndSpotted(FALSE);
            MP_Battle_Start(trainer, gMpLocalId < trainer, sSpot.oppMoney);
            MP_Battle_SetOppPhrases(sSpot.oppPhrases[PHRASE_WON], sSpot.oppPhrases[PHRASE_LOST]);
            return;
        }
        if (sSpot.boxClosed && ++sSpot.timer > 60 * 20)
        {
            MPNet_Log("the trainer never closed the challenge, giving up");
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
            u8 name[MP_NAME_LEN + 1];
            sSpot.oppMoney = ReadMoney(data + 1);
            ReadPhrases(data, len, sSpot.oppPhrases);
            SendWithMoney(from, MPR_BATTLE_GO);
            // The trainer's challenge phrase, then the battle once both boxes are closed
            StringCopyN(name, gMpPlayers[from].name, MP_NAME_LEN);
            name[MP_NAME_LEN] = EOS;
            sSpot.state = SP_PHRASE;
            sSpot.timer = 0;
            sSpot.otherReady = FALSE;
            sSpot.boxClosed = !ShowChallenge(name, sSpot.oppPhrases[PHRASE_START]);
            if (sSpot.boxClosed)
                SendSimple(from, MPR_PHRASE_READY);
        }
        break;
    case MPR_BATTLE_GO:
        if (sTM.state == TM_WAIT_GO && from == sTM.target && len >= 5)
        {
            sTM.oppMoney = ReadMoney(data + 1);
            ReadPhrases(data, len, sTM.oppPhrases);
            sTM.state = TM_PHRASE;
            sTM.timer = 0;
            sTM.otherReady = FALSE;
            sTM.boxClosed = !ShowChallenge(gSaveBlock2Ptr->playerName, OwnPhrase(PHRASE_START));
            if (sTM.boxClosed)
                SendSimple(from, MPR_PHRASE_READY);
        }
        break;
    case MPR_PHRASE_READY:
        if (sTM.state == TM_PHRASE && from == sTM.target)
            sTM.otherReady = TRUE;
        if (sSpot.state == SP_PHRASE && from == sSpot.trainer)
            sSpot.otherReady = TRUE;
        break;
    case MPR_CANCEL:
        if (sSpot.state != SP_NONE && from == sSpot.trainer)
            EndSpotted(TRUE);
        if (sTM.state >= TM_SPOTTING && sTM.state < TM_STARTING && from == sTM.target)
        {
            if (sTM.state == TM_PHRASE)
                CloseChallengeBox();
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
        if (sTM.state == TM_PHRASE)
            CloseChallengeBox();
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

// ---------------------------------------------------------------------------
// Using the BATTLE FLAG: "Set phrases, or plant?" PHRASES picks CHALLENGE / WIN / LOSE and opens the phrase builder
// for it (then comes back to the list); PLANT goes on guard as before (item_use.c MP_BattleFlag_Plant).

static const u8 sText_FlagAsk[] = _("Set phrases, or plant?");
static const u8 sText_FlagWhich[] = _("Which phrase do you want to set?");
static const u8 sText_ChPhrases[] = _("PHRASES");
static const u8 sText_ChPlant[] = _("PLANT");
static const u8 sText_ChCancel[] = _("CANCEL");
static const u8 sText_ChChallenge[] = _("CHALLENGE");
static const u8 sText_ChWin[] = _("WIN");
static const u8 sText_ChLose[] = _("LOSE");
static const u8 sText_ChDone[] = _("DONE");
static const struct MenuAction sFlagChoices[] = {{sText_ChPhrases, {NULL}}, {sText_ChPlant, {NULL}}, {sText_ChCancel, {NULL}}};
static const struct MenuAction sPhraseChoices[] = {{sText_ChChallenge, {NULL}}, {sText_ChWin, {NULL}}, {sText_ChLose, {NULL}},
                                                   {sText_ChDone, {NULL}}};
enum { FLAG_PHRASES, FLAG_PLANT, FLAG_CANCEL };
enum { PH_CHALLENGE, PH_WIN, PH_LOSE, PH_DONE };
static const u8 sPhraseTypes[] = {EASY_CHAT_TYPE_BATTLE_START, EASY_CHAT_TYPE_BATTLE_WON, EASY_CHAT_TYPE_BATTLE_LOST};

static const struct WindowTemplate sFlagChoiceWindow = {
    .bg = 0, .tilemapLeft = 20, .tilemapTop = 7, .width = 9, .height = 6, .paletteNum = 15, .baseBlock = 0x125,
};
static const struct WindowTemplate sPhraseChoiceWindow = {
    .bg = 0, .tilemapLeft = 19, .tilemapTop = 5, .width = 10, .height = 8, .paletteNum = 15, .baseBlock = 0x125,
};

static EWRAM_DATA u8 sFlagWindowId = 0;
static EWRAM_DATA u8 sPhraseCursor = 0;

#define tState data[0]

enum {
    FM_ASK,          // "Set phrases, or plant?"
    FM_ASK_MENU,
    FM_ASK_INPUT,
    FM_WHICH,        // "Which phrase...?"
    FM_WHICH_MENU,
    FM_WHICH_INPUT,
    FM_FADE_OUT,     // to the phrase builder
    FM_BACK,         // back from it: wait for the field to fade in
};

static void Task_FlagMenu(u8 taskId);

static void OpenChoices(const struct WindowTemplate *t, u8 count, const struct MenuAction *items, u8 cursor)
{
    sFlagWindowId = AddWindow(t);
    DrawStdWindowFrame(sFlagWindowId, FALSE);
    PrintMenuTable(sFlagWindowId, count, items);
    InitMenuInUpperLeftCornerNormal(sFlagWindowId, count, cursor);
    CopyWindowToVram(sFlagWindowId, COPYWIN_FULL);
}

static void CloseChoices(void)
{
    ClearStdWindowAndFrameToTransparent(sFlagWindowId, TRUE);
    RemoveWindow(sFlagWindowId);
}

static void EndFlagMenu(u8 taskId)
{
    HideFieldMessageBox();
    DestroyTask(taskId);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

void MP_FieldClearVBlankHBlankCallbacks(void);   // overworld.c

static void CB2_ReturnFromPhrases(void)
{
    MP_FieldClearVBlankHBlankCallbacks();
    gFieldCallback = MP_Trainer_FieldCB_Phrases;
    CB2_ReturnToField();
}

void MP_Trainer_FieldCB_Phrases(void)
{
    u8 taskId;
    LockPlayerFieldControls();
    FadeInFromBlack();
    taskId = CreateTask(Task_FlagMenu, 80);
    gTasks[taskId].tState = FM_BACK;
}

static void Task_FlagMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s8 answer;

    switch (tState)
    {
    case FM_ASK:
    case FM_WHICH:
        HideMapNamePopUpWindow();   // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(tState == FM_ASK ? sText_FlagAsk : sText_FlagWhich);
        tState++;
        break;
    case FM_ASK_MENU:
        if (!IsTextPrinterActive(0))
        {
            OpenChoices(&sFlagChoiceWindow, ARRAY_COUNT(sFlagChoices), sFlagChoices, 0);
            tState++;
        }
        break;
    case FM_WHICH_MENU:
        if (!IsTextPrinterActive(0))
        {
            OpenChoices(&sPhraseChoiceWindow, ARRAY_COUNT(sPhraseChoices), sPhraseChoices, sPhraseCursor);
            tState++;
        }
        break;
    case FM_ASK_INPUT:
        answer = Menu_ProcessInputNoWrap();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        CloseChoices();
        if (answer == FLAG_PHRASES)
        {
            MPNet_Log("battle flag: phrases");
            sPhraseCursor = 0;
            tState = FM_WHICH;
        }
        else if (answer == FLAG_PLANT)
        {
            MPNet_Log("battle flag: plant");
            HideFieldMessageBox();
            MP_BattleFlag_Plant(taskId);   // takes over the task (the old flag message)
        }
        else
        {
            EndFlagMenu(taskId);
        }
        break;
    case FM_WHICH_INPUT:
        answer = Menu_ProcessInputNoWrap();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        CloseChoices();
        if (answer == MENU_B_PRESSED || answer == PH_DONE)
        {
            EndFlagMenu(taskId);
            break;
        }
        sPhraseCursor = answer;
        MPNet_Log("battle flag: set phrase %d", answer);
        HideFieldMessageBox();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        tState = FM_FADE_OUT;
        break;
    case FM_FADE_OUT:
        if (!gPaletteFade.active)
        {
            u8 which = sPhraseCursor;
            CleanupOverworldWindowsAndTilemaps();
            DoEasyChatScreen(sPhraseTypes[which], (u16 *)OwnPhrase(which), CB2_ReturnFromPhrases,
                             EASY_CHAT_PERSON_DISPLAY_NONE);   // resets the tasks (this one too)
        }
        break;
    case FM_BACK:
        if (IsWeatherNotFadingIn())
            tState = FM_WHICH;
        break;
    }
}

#undef tState

// ItemUseOnFieldCB for the BATTLE FLAG (item_use.c): the task becomes the flag menu
void MP_Trainer_UseFlag(u8 taskId)
{
    LockPlayerFieldControls();
    gTasks[taskId].func = Task_FlagMenu;
    gTasks[taskId].data[0] = FM_ASK;
}
