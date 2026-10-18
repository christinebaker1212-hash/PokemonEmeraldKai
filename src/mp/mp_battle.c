// Moemon Co-op: network link layer for real link battles + PvP flow and rewards.
//
// The decomp's link battle code is used unchanged. When gNetLinkActive is set,
// link.c routes the block transfer API (SendBlock, GetBlockReceivedStatus,
// standby/close barriers, link player exchange) to the functions here, which
// carry the data through the relay server to the opponent.
#include "global.h"
#include "mp.h"
#include "main.h"
#include "link.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "battle_main.h"
#include "event_data.h"
#include "field_message_box.h"
#include "field_screen_effect.h"
#include "item.h"
#include "load_save.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/characters.h"

void MPNet_Log(const char *fmt, ...);
void FreeBattleResources(void);
void FreeBattleSpritesData(void);
void FreeMonSpritesGfx(void);
void CB2_MpWhiteOut(void);
extern void (*gMpWhiteOutFieldCallback)(void);
extern struct LinkPlayer gLocalLinkPlayer;
void SaveLinkPlayers(u8 playerCount);
void ConvertLinkPlayerName(struct LinkPlayer *player);

bool8 gNetLinkActive;

#define BLOCK_QUEUE 32

struct QueuedBlock
{
    u16 size;
    u8 data[BLOCK_BUFFER_SIZE];
};

static struct
{
    bool8 battleActive;      // from battle start until back in the overworld
    bool8 aborted;
    u8 peer;                 // network player id of the opponent
    u8 linkId;               // our link multiplayer id (0 = master)
    bool8 recvFlags[2];
    struct QueuedBlock queue[BLOCK_QUEUE];
    u8 qHead, qCount;
    u16 mySyncSeq;
    u16 peerSyncSeq;
    u8 pendingSync;          // 0 none, 1 standby, 2 close
    bool8 haveRemotePlayer;
    bool8 opened;
    struct LinkPlayer remotePlayer;
    u32 myMoney;
    u32 oppMoney;
    u8 oppName[MP_NAME_LEN + 1];
    u8 startTask;
} sNet;

static u8 sRelayBuf[BLOCK_BUFFER_SIZE + 16];
static EWRAM_DATA u8 sResultText[256];

bool8 MP_Battle_IsActive(void)
{
    return sNet.battleActive;
}

u16 MP_Battle_OpponentFrontPic(void)
{
    if (!sNet.battleActive || sNet.peer >= MP_MAX_PLAYERS || !gMpPlayers[sNet.peer].present)
        return 0xFFFF;
    return MP_Avatar_FrontPicFor(&gMpPlayers[sNet.peer].app);
}

// ---------------------------------------------------------------------------
// Link layer

static u8 PeerLinkId(void)
{
    return sNet.linkId ^ 1;
}

u8 NetLink_GetMultiplayerId(void)
{
    return sNet.linkId;
}

u8 NetLink_GetPlayerCount(void)
{
    return 2;
}

bool8 NetLink_IsMaster(void)
{
    return sNet.linkId == 0;
}

static void ApplyRemotePlayer(void)
{
    u8 peer = PeerLinkId();
    gLinkPlayers[sNet.linkId] = gLocalLinkPlayer;
    gLinkPlayers[peer] = sNet.remotePlayer;
    gLinkPlayers[peer].id = peer;
    gLinkPlayers[sNet.linkId].id = sNet.linkId;
    ConvertLinkPlayerName(&gLinkPlayers[peer]);
    SaveLinkPlayers(2);
    gReceivedRemoteLinkPlayers = TRUE;
}

void NetLink_Open(void)
{
    u8 buf[1 + sizeof(struct LinkPlayer)];

    gLinkPlayers[sNet.linkId] = gLocalLinkPlayer;
    gLocalLinkPlayer.id = sNet.linkId;
    gLocalLinkPlayer.linkType = LINKTYPE_BATTLE;
    sNet.recvFlags[0] = sNet.recvFlags[1] = FALSE;
    sNet.pendingSync = 0;
    gReceivedRemoteLinkPlayers = FALSE;
    sNet.opened = TRUE;
    buf[0] = MPR_LINK_PLAYER;
    memcpy(buf + 1, &gLocalLinkPlayer, sizeof(struct LinkPlayer));
    MP_SendRelay(sNet.peer, buf, sizeof(buf));
    if (sNet.haveRemotePlayer)
        ApplyRemotePlayer();
}

void NetLink_Close(void)
{
    gReceivedRemoteLinkPlayers = FALSE;
}

bool8 NetLink_SendBlock(const void *src, u16 size)
{
    if (size > BLOCK_BUFFER_SIZE)
        size = BLOCK_BUFFER_SIZE;
    // The GBA link also hands every sender its own block
    memcpy(gBlockRecvBuffer[sNet.linkId], src, size);
    sNet.recvFlags[sNet.linkId] = TRUE;
    sRelayBuf[0] = MPR_LINK_BLOCK;
    sRelayBuf[1] = size & 0xFF;
    sRelayBuf[2] = size >> 8;
    memcpy(sRelayBuf + 3, src, size);
    MP_SendRelay(sNet.peer, sRelayBuf, size + 3);
    return TRUE;
}

static void UpdateSync(void)
{
    if (sNet.pendingSync && (s16)(sNet.peerSyncSeq - sNet.mySyncSeq) >= 0)
    {
        if (sNet.pendingSync == 2)
            gReceivedRemoteLinkPlayers = FALSE;
        sNet.pendingSync = 0;
    }
}

bool8 NetLink_IsTaskFinished(void)
{
    UpdateSync();
    return sNet.pendingSync == 0;
}

u8 NetLink_GetBlockReceivedStatus(void)
{
    return (sNet.recvFlags[0] ? 1 : 0) | (sNet.recvFlags[1] ? 2 : 0);
}

void NetLink_ResetBlockReceivedFlag(u8 who)
{
    if (who < 2)
        sNet.recvFlags[who] = FALSE;
}

static void SendSync(u8 kind)
{
    u8 buf[4];
    sNet.mySyncSeq++;
    sNet.pendingSync = kind;
    buf[0] = MPR_LINK_SYNC;
    buf[1] = kind;
    buf[2] = sNet.mySyncSeq & 0xFF;
    buf[3] = sNet.mySyncSeq >> 8;
    MP_SendRelay(sNet.peer, buf, 4);
    UpdateSync();
}

void NetLink_SetStandbyCallback(void)
{
    if (sNet.pendingSync == 0)
        SendSync(1);
}

void NetLink_SetCloseCallback(void)
{
    if (sNet.pendingSync == 0)
        SendSync(2);
}

// Moves the next queued block into gBlockRecvBuffer once the game consumed the previous one
static void PumpBlocks(void)
{
    u8 peer = PeerLinkId();
    if (sNet.qCount > 0 && !sNet.recvFlags[peer])
    {
        struct QueuedBlock *b = &sNet.queue[sNet.qHead];
        memcpy(gBlockRecvBuffer[peer], b->data, b->size);
        sNet.recvFlags[peer] = TRUE;
        sNet.qHead = (sNet.qHead + 1) % BLOCK_QUEUE;
        sNet.qCount--;
    }
    UpdateSync();
}

// ---------------------------------------------------------------------------
// Rewards

static const u16 sRewardBalls[] = {
    ITEM_POKE_BALL, ITEM_POKE_BALL, ITEM_POKE_BALL, ITEM_POKE_BALL, ITEM_POKE_BALL, ITEM_POKE_BALL,
    ITEM_GREAT_BALL, ITEM_GREAT_BALL, ITEM_GREAT_BALL, ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL, ITEM_ULTRA_BALL, ITEM_ULTRA_BALL,
    ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_NEST_BALL, ITEM_REPEAT_BALL, ITEM_TIMER_BALL,
    ITEM_LUXURY_BALL, ITEM_PREMIER_BALL,
};

static void AppendMoney(u8 *dst, u32 amount)
{
    u8 num[12];
    u8 yen[] = {CHAR_CURRENCY, EOS};
    ConvertIntToDecimalStringN(num, amount, STR_CONV_MODE_LEFT_ALIGN, 7);
    StringAppend(dst, yen);
    StringAppend(dst, num);
}

static void Append(u8 *dst, const char *ascii)
{
    u8 tmp[96];
    void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
    MP_AsciiToGame(tmp, ascii, 90);
    StringAppend(dst, tmp);
}

static void AppendRaw(u8 *dst, u8 c)
{
    u8 tmp[2] = {c, EOS};
    StringAppend(dst, tmp);
}

// Field message shown after returning from a battle
static void Task_ShowResultMessage(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (data[0])
    {
    case 0:
        if (gPaletteFade.active || !MP_IsInOverworld() || ScriptContext_IsEnabled())
            break;
        LockPlayerFieldControls();
        ShowFieldMessage(sResultText);
        data[0]++;
        break;
    case 1:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            data[0]++;
        }
        break;
    case 2:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
}

static void FieldCB_MpResult(void)
{
    FieldCB_WarpExitFadeFromBlack();
    if (sResultText[0] != EOS)
        CreateTask(Task_ShowResultMessage, 80);
}

static void FinishBattle(void)
{
    sNet.battleActive = FALSE;
    gNetLinkActive = FALSE;
    gReceivedRemoteLinkPlayers = FALSE;
    sNet.qCount = 0;
}

static void CB2_MpReturnFromBattle(void)
{
    u8 outcome = gBattleOutcome;

    gBattleTypeFlags &= ~BATTLE_TYPE_LINK_IN_BATTLE;
    LoadPlayerParty();
    Overworld_ResetMapMusic();
    FinishBattle();
    sResultText[0] = EOS;

    if (outcome == B_OUTCOME_WON)
    {
        u32 prize = sNet.oppMoney / 10;
        u16 ball = sRewardBalls[Random() % ARRAY_COUNT(sRewardBalls)];
        u8 ballName[ITEM_NAME_LENGTH + 4];

        AddMoney(&gSaveBlock1Ptr->money, prize);
        AddBagItem(ball, 1);
        CopyItemName(ball, ballName);
        StringCopy(sResultText, gSaveBlock2Ptr->playerName);
        Append(sResultText, " defeated ");
        StringAppend(sResultText, sNet.oppName);
        AppendRaw(sResultText, CHAR_EXCL_MARK);
        AppendRaw(sResultText, CHAR_NEWLINE);
        Append(sResultText, "Got ");
        AppendMoney(sResultText, prize);
        Append(sResultText, " and a ");
        StringAppend(sResultText, ballName);
        AppendRaw(sResultText, CHAR_EXCL_MARK);
        PlaySE(SE_SHOP);
        MPNet_Log("won the battle: +%u money", (unsigned)prize);
        gFieldCallback = FieldCB_MpResult;
        SetMainCallback2(CB2_ReturnToField);
    }
    else if (outcome == B_OUTCOME_LOST || outcome == B_OUTCOME_FORFEITED || (outcome & B_OUTCOME_LINK_BATTLE_RAN))
    {
        u32 loss = sNet.myMoney / 10;
        if (loss > GetMoney(&gSaveBlock1Ptr->money))
            loss = GetMoney(&gSaveBlock1Ptr->money);
        RemoveMoney(&gSaveBlock1Ptr->money, loss);
        StringCopy(sResultText, gSaveBlock2Ptr->playerName);
        Append(sResultText, " lost to ");
        StringAppend(sResultText, sNet.oppName);
        Append(sResultText, " and paid ");
        AppendMoney(sResultText, loss);
        AppendRaw(sResultText, CHAR_PERIOD);
        AppendRaw(sResultText, CHAR_NEWLINE);
        Append(sResultText, "You hurried back to safety...");
        MPNet_Log("lost the battle: -%u money", (unsigned)loss);
        gMpWhiteOutFieldCallback = FieldCB_MpResult;
        gMain.state = 0;
        SetMainCallback2(CB2_MpWhiteOut);
    }
    else
    {
        Append(sResultText, "The battle ended in a draw.");
        gFieldCallback = FieldCB_MpResult;
        SetMainCallback2(CB2_ReturnToField);
    }
}

// ---------------------------------------------------------------------------
// Starting the battle

static void Task_StartMpBattle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (sNet.aborted)
    {
        FinishBattle();
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        sNet.startTask = TASK_NONE;
        return;
    }
    switch (data[0])
    {
    case 0:
        LockPlayerFieldControls();
        PlayMapChosenOrBattleBGM(MUS_VS_TRAINER);
        BattleTransition_StartOnField(B_TRANSITION_POKEBALLS_TRAIL);
        data[0]++;
        break;
    case 1:
        if (IsBattleTransitionDone())
        {
            gLinkType = LINKTYPE_BATTLE;
            SavePlayerParty();
            gBattleTypeFlags = BATTLE_TYPE_LINK | BATTLE_TYPE_TRAINER;
            gTrainerBattleOpponent_A = TRAINER_LINK_OPPONENT;
            CleanupOverworldWindowsAndTilemaps();
            gMain.savedCallback = CB2_MpReturnFromBattle;
            SetMainCallback2(CB2_InitBattle);
            UnlockPlayerFieldControls();
            sNet.startTask = TASK_NONE;
            DestroyTask(taskId);
        }
        break;
    }
}

void MP_Battle_Start(u8 opponentId, bool8 isMaster, u32 opponentMoney)
{
    memset(&sNet, 0, sizeof(sNet));
    sNet.peer = opponentId;
    sNet.linkId = isMaster ? 0 : 1;
    sNet.battleActive = TRUE;
    sNet.myMoney = GetMoney(&gSaveBlock1Ptr->money);
    sNet.oppMoney = opponentMoney;
    StringCopyN(sNet.oppName, gMpPlayers[opponentId].name, MP_NAME_LEN);
    sNet.oppName[MP_NAME_LEN] = EOS;
    gNetLinkActive = TRUE;
    gWirelessCommType = 0;
    gReceivedRemoteLinkPlayers = FALSE;
    gLinkPlayers[0].trainerId = 0;
    MPNet_Log("starting PvP battle against player %d (we are link %s)", opponentId + 1, isMaster ? "master" : "slave");
    sNet.startTask = CreateTask(Task_StartMpBattle, 1);
}

// ---------------------------------------------------------------------------
// Network messages

void MP_Battle_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (!sNet.battleActive || from != sNet.peer)
        return;
    switch (data[0])
    {
    case MPR_LINK_PLAYER:
        if (len < 1 + sizeof(struct LinkPlayer))
            return;
        memcpy(&sNet.remotePlayer, data + 1, sizeof(struct LinkPlayer));
        sNet.haveRemotePlayer = TRUE;
        // Only apply once the game opened its side of the link
        if (sNet.opened)
            ApplyRemotePlayer();
        break;
    case MPR_LINK_BLOCK:
    {
        u16 size;
        struct QueuedBlock *b;
        if (len < 3)
            return;
        size = data[1] | (data[2] << 8);
        if (size > BLOCK_BUFFER_SIZE || len < 3 + size)
            return;
        if (sNet.qCount == BLOCK_QUEUE)
        {
            MPNet_Log("link block queue overflow");
            return;
        }
        b = &sNet.queue[(sNet.qHead + sNet.qCount) % BLOCK_QUEUE];
        b->size = size;
        memcpy(b->data, data + 3, size);
        sNet.qCount++;
        PumpBlocks();
        break;
    }
    case MPR_LINK_SYNC:
        if (len < 4)
            return;
        sNet.peerSyncSeq = data[2] | (data[3] << 8);
        UpdateSync();
        break;
    }
}

static void AbortBattleToField(void)
{
    MPNet_Log("opponent disconnected, leaving the battle");
    if (gMain.inBattle || gMain.callback2 != CB2_Overworld)
    {
        gMain.inBattle = FALSE;
        if (gPreBattleCallback1)
            gMain.callback1 = gPreBattleCallback1;
        FreeBattleResources();
        FreeBattleSpritesData();
        FreeMonSpritesGfx();
        gBattleTypeFlags = 0;
        LoadPlayerParty();
        FinishBattle();
        StringCopy(sResultText, sNet.oppName);
        Append(sResultText, " disconnected. The battle was called off.");
        gFieldCallback = FieldCB_MpResult;
        SetMainCallback2(CB2_ReturnToField);
    }
    else
    {
        FinishBattle();
    }
}

void MP_Battle_OnPlayerLeft(u8 id)
{
    if (!sNet.battleActive || id != sNet.peer)
        return;
    sNet.aborted = TRUE;
    if (sNet.startTask != TASK_NONE && FuncIsActiveTask(Task_StartMpBattle))
        return; // the start task cleans up
    AbortBattleToField();
}

void MP_Battle_FrameUpdate(void)
{
    if (sNet.battleActive && gNetLinkActive)
        PumpBlocks();
}
