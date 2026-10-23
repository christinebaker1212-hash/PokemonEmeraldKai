#include <stdlib.h>
#include <stdint.h>
// Moemon Co-op: network link layer for real link battles + PvP flow and rewards.
//
// The decomp's link battle code is used unchanged. When gNetLinkActive is set,
// link.c routes the block transfer API (SendBlock, GetBlockReceivedStatus,
// standby/close barriers, link player exchange) to the functions here, which
// carry the data through the relay server to the opponent.
#include "global.h"
#include "mp.h"
#include "map_name_popup.h"
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

// Link ids: 0 = master. Two games for PvP and co-op doubles, three for a
// co-op Triple Battle (each other game has its own block queue, sync
// sequence and "ready" flag).
#define NET_MAX_LINK 3

static struct
{
    bool8 battleActive;      // from battle start until back in the overworld
    bool8 aborted;
    u8 peer;                 // network player id of the opponent (the first other game)
    u8 linkId;               // our link multiplayer id (0 = master)
    u8 linkCount;            // games in the link (2, or 3 in a triple)
    u8 peers[NET_MAX_LINK];  // network player id of each link id (ours unused)
    bool8 recvFlags[MAX_LINK_PLAYERS];
    struct QueuedBlock queue[NET_MAX_LINK][BLOCK_QUEUE];
    u8 qHead[NET_MAX_LINK], qCount[NET_MAX_LINK];
    struct QueuedBlock selfQueue[BLOCK_QUEUE];   // our own blocks, when several go out in one frame
    u8 sHead, sCount;
    u16 mySyncSeq;
    u16 peerSyncSeq[NET_MAX_LINK];
    u8 pendingSync;          // 0 none, 1 standby, 2 close
    u8 readyMask;            // link ids whose game is in the battle (sync kind 3)
    u8 remoteMask;           // link ids whose LinkPlayer we have
    bool8 opened;
    struct LinkPlayer remotePlayer[NET_MAX_LINK];
    u32 myMoney;
    u32 oppMoney;
    u8 oppName[MP_NAME_LEN + 1];
    u8 startTask;
} sNet;

static u8 sRelayBuf[BLOCK_BUFFER_SIZE + 16];
static EWRAM_DATA u8 sResultText[256];

s8 MP_Battle_PeerId(void)
{
    return sNet.battleActive ? (s8)sNet.peer : -1;
}

bool8 MP_Battle_IsActive(void)
{
    return sNet.battleActive;
}

u16 MP_Battle_OpponentFrontPic(void)
{
    if (gMpCoopBattle || !sNet.battleActive || sNet.peer >= MP_MAX_PLAYERS || !gMpPlayers[sNet.peer].present)
        return 0xFFFF;
    return MP_Avatar_RemoteFrontPic(sNet.peer);
}

// ---------------------------------------------------------------------------
// Link layer

static void PumpBlocks(void);

static bool8 DebugOn(void)
{
    static int sOn = -1;
    if (sOn < 0)
        sOn = getenv("MOEMON_LINKDEBUG") != NULL;
    return sOn;
}

// Every other game's link id as a bit mask
static u8 OthersMask(void)
{
    return ((1 << sNet.linkCount) - 1) & ~(1 << sNet.linkId);
}

static s8 LinkIdOf(u8 netId)
{
    u8 i;
    for (i = 0; i < sNet.linkCount && i < NET_MAX_LINK; i++)
        if (i != sNet.linkId && sNet.peers[i] == netId)
            return i;
    return -1;
}

static void SendAll(const u8 *buf, u16 len)
{
    u8 i;
    for (i = 0; i < sNet.linkCount && i < NET_MAX_LINK; i++)
        if (i != sNet.linkId)
            MP_SendRelay(sNet.peers[i], buf, len);
}

u8 NetLink_GetMultiplayerId(void)
{
    if (MP_Group_Active())
        return MP_Group_GetMultiplayerId();
    return sNet.linkId;
}

static u8 sMixCount;   // record mixing with up to 4 players (mp_linkx.c)

u8 NetLink_GetPlayerCount(void)
{
    if (MP_Group_Active())
        return MP_Group_GetPlayerCount();
    if (sMixCount)
        return sMixCount;
    return sNet.linkCount ? sNet.linkCount : 2;
}

// Run the vanilla record mixing code against a fake link of `count` players
bool8 NetLink_BeginRecordMix(u8 linkId, u8 count)
{
    if (sNet.battleActive || gNetLinkActive)
        return FALSE;
    sNet.linkId = linkId;
    sMixCount = count;
    gNetLinkActive = TRUE;
    return TRUE;
}

void NetLink_EndRecordMix(void)
{
    sMixCount = 0;
    gNetLinkActive = FALSE;
}

void NetLink_FillLocalLinkPlayer(struct LinkPlayer *out)
{
    memset(out, 0, sizeof(*out));
    out->trainerId = gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
                   | (gSaveBlock2Ptr->playerTrainerId[2] << 16) | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
    StringCopy(out->name, gSaveBlock2Ptr->playerName);
    out->gender = gSaveBlock2Ptr->playerGender;
    out->language = gGameLanguage;
    out->version = gGameVersion + 0x4000;
    out->lp_field_2 = 0x8000;
}

bool8 NetLink_IsMaster(void)
{
    if (MP_Group_Active())
        return MP_Group_IsMaster();
    return sNet.linkId == 0;
}

// Once every other game's LinkPlayer is here
static void ApplyRemotePlayer(void)
{
    u8 i;
    if ((sNet.remoteMask & OthersMask()) != OthersMask())
        return;
    gLinkPlayers[sNet.linkId] = gLocalLinkPlayer;
    gLinkPlayers[sNet.linkId].id = sNet.linkId;
    for (i = 0; i < sNet.linkCount; i++)
    {
        if (i == sNet.linkId)
            continue;
        gLinkPlayers[i] = sNet.remotePlayer[i];
        gLinkPlayers[i].id = i;
        ConvertLinkPlayerName(&gLinkPlayers[i]);
    }
    SaveLinkPlayers(sNet.linkCount);
    gReceivedRemoteLinkPlayers = TRUE;
}

void NetLink_Open(void)
{
    u8 buf[1 + sizeof(struct LinkPlayer)];

    if (MP_Group_Active())
        return;
    gLinkPlayers[sNet.linkId] = gLocalLinkPlayer;
    gLocalLinkPlayer.id = sNet.linkId;
    gLocalLinkPlayer.linkType = LINKTYPE_BATTLE;
    memset(sNet.recvFlags, 0, sizeof(sNet.recvFlags));
    sNet.pendingSync = 0;
    gReceivedRemoteLinkPlayers = FALSE;
    sNet.opened = TRUE;
    buf[0] = MPR_LINK_PLAYER;
    memcpy(buf + 1, &gLocalLinkPlayer, sizeof(struct LinkPlayer));
    SendAll(buf, sizeof(buf));
    ApplyRemotePlayer();
}

void NetLink_Close(void)
{
    if (MP_Group_Active())
    {
        MP_Group_Close();
        return;
    }
    gReceivedRemoteLinkPlayers = FALSE;
}

bool8 NetLink_SendBlock(const void *src, u16 size)
{
    if (MP_Group_Active())
        return MP_Group_SendBlock(src, size);
    if (size > BLOCK_BUFFER_SIZE)
        size = BLOCK_BUFFER_SIZE;
    // The GBA link also hands every sender its own block. Several can go out
    // in one frame now (battle_controllers.c), so queue it if the last one
    // hasn't been taken yet instead of overwriting it.
    if (!sNet.recvFlags[sNet.linkId] && sNet.sCount == 0)
    {
        memcpy(gBlockRecvBuffer[sNet.linkId], src, size);
        sNet.recvFlags[sNet.linkId] = TRUE;
    }
    else if (sNet.sCount < BLOCK_QUEUE)
    {
        struct QueuedBlock *b = &sNet.selfQueue[(sNet.sHead + sNet.sCount) % BLOCK_QUEUE];
        b->size = size;
        memcpy(b->data, src, size);
        sNet.sCount++;
    }
    else
    {
        MPNet_Log("own link block queue overflow");
    }
    if (DebugOn())
        MPNet_Log("blk send %d: %02x %02x .. cmd %02x", size, ((const u8 *)src)[0], ((const u8 *)src)[1], size > 8 ? ((const u8 *)src)[8] : 0);
    sRelayBuf[0] = MPR_LINK_BLOCK;
    sRelayBuf[1] = size & 0xFF;
    sRelayBuf[2] = size >> 8;
    memcpy(sRelayBuf + 3, src, size);
    SendAll(sRelayBuf, size + 3);
    return TRUE;
}

// Both games tell each other when they're in the battle (battle_controllers.c
// waits for it before sending battle blocks)
void NetLink_SendReady(void)
{
    u8 buf[4] = {MPR_LINK_SYNC, 3, 0, 0};
    SendAll(buf, sizeof(buf));
}

bool8 NetLink_PeerReady(void)
{
    return (sNet.readyMask & OthersMask()) == OthersMask();
}

// Every other game reached our barrier
static bool8 SyncReached(void)
{
    u8 i;
    for (i = 0; i < sNet.linkCount; i++)
        if (i != sNet.linkId && (s16)(sNet.peerSyncSeq[i] - sNet.mySyncSeq) < 0)
            return FALSE;
    return TRUE;
}

static void UpdateSync(void)
{
    if (sNet.pendingSync && SyncReached())
    {
        if (sNet.pendingSync == 2)
            gReceivedRemoteLinkPlayers = FALSE;
        sNet.pendingSync = 0;
    }
}

bool8 NetLink_IsTaskFinished(void)
{
    if (MP_Group_Active())
        return MP_Group_IsTaskFinished();
    UpdateSync();
    return sNet.pendingSync == 0;
}

u8 NetLink_GetBlockReceivedStatus(void)
{
    if (MP_Group_Active())
        return MP_Group_GetBlockReceivedStatus();
    // The next waiting block is there as soon as the last one was taken (no frame lost)
    if (sNet.battleActive && gNetLinkActive)
        PumpBlocks();
    {
        u8 i, bits = 0;
        for (i = 0; i < MAX_LINK_PLAYERS; i++)
            if (sNet.recvFlags[i])
                bits |= 1 << i;
        return bits;
    }
}

void NetLink_ResetBlockReceivedFlag(u8 who)
{
    if (MP_Group_Active())
    {
        MP_Group_ResetBlockReceivedFlag(who);
        return;
    }
    if (who < MAX_LINK_PLAYERS)
        sNet.recvFlags[who] = FALSE;
    // Not refilled here: the game reads gBlockRecvBuffer right after clearing
    // the flag, so the next block waits until it asks again (below)
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
    SendAll(buf, 4);
    UpdateSync();
}

void NetLink_SetStandbyCallback(void)
{
    if (MP_Group_Active())
    {
        MP_Group_SetStandbyCallback();
        return;
    }
    if (sNet.pendingSync == 0)
        SendSync(1);
}

void NetLink_SetCloseCallback(void)
{
    if (MP_Group_Active())
    {
        MP_Group_SetCloseCallback();
        return;
    }
    if (sNet.pendingSync == 0)
        SendSync(2);
}

// Moves the next queued block into gBlockRecvBuffer once the game consumed the previous one
static void PumpBlocks(void)
{
    u8 peer;
    if (sNet.sCount > 0 && !sNet.recvFlags[sNet.linkId])
    {
        struct QueuedBlock *b = &sNet.selfQueue[sNet.sHead];
        memcpy(gBlockRecvBuffer[sNet.linkId], b->data, b->size);
        sNet.recvFlags[sNet.linkId] = TRUE;
        sNet.sHead = (sNet.sHead + 1) % BLOCK_QUEUE;
        sNet.sCount--;
    }
    for (peer = 0; peer < sNet.linkCount && peer < NET_MAX_LINK; peer++)
    {
        if (peer != sNet.linkId && sNet.qCount[peer] > 0 && !sNet.recvFlags[peer])
        {
            struct QueuedBlock *b = &sNet.queue[peer][sNet.qHead[peer]];
            memcpy(gBlockRecvBuffer[peer], b->data, b->size);
            sNet.recvFlags[peer] = TRUE;
            sNet.qHead[peer] = (sNet.qHead[peer] + 1) % BLOCK_QUEUE;
            sNet.qCount[peer]--;
        }
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
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sResultText);
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
    memset(sNet.qCount, 0, sizeof(sNet.qCount));
    sNet.sCount = 0;
}

static void CB2_MpReturnFromBattle(void)
{
    u8 outcome = gBattleOutcome;

    gBattleTypeFlags &= ~BATTLE_TYPE_LINK_IN_BATTLE;
    LoadPlayerParty();
    Overworld_ResetMapMusic();
    FinishBattle();
    sResultText[0] = EOS;

    // Tournament matches are friendly: no money, nobody blacks out
    if (gMpFriendlyBattle)
    {
        StringCopy(sResultText, gSaveBlock2Ptr->playerName);
        if (outcome == B_OUTCOME_WON)
            Append(sResultText, " won the match against ");
        else if (outcome == B_OUTCOME_DREW)
            Append(sResultText, " tied the match against ");
        else
            Append(sResultText, " lost the match against ");
        StringAppend(sResultText, sNet.oppName);
        AppendRaw(sResultText, CHAR_EXCL_MARK);
        MP_Tourney_OnBattleResult(outcome == B_OUTCOME_WON || outcome == B_OUTCOME_DREW ? outcome : B_OUTCOME_LOST);
        MP_Events_Did(MP_DID_PVP);
        if (outcome == B_OUTCOME_WON)
            MP_Events_Did(MP_DID_PVPWIN);
        gFieldCallback = FieldCB_MpResult;
        SetMainCallback2(CB2_ReturnToField);
        return;
    }

    MP_Events_Did(MP_DID_PVP);
    if (outcome == B_OUTCOME_WON)
        MP_Events_Did(MP_DID_PVPWIN);
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
    sNet.linkCount = 2;
    sNet.peers[sNet.linkId ^ 1] = opponentId;
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

// Co-op battles start their own battle; they only need the link layer.
// peers[] = the network player id of each link id (ours is skipped); count 2
// for a co-op double, 3 for a triple.
void MP_Battle_OpenLinkGroup(const u8 *peers, u8 count, u8 linkId)
{
    u8 i;
    memset(&sNet, 0, sizeof(sNet));
    sNet.linkId = linkId;
    sNet.linkCount = count;
    for (i = 0; i < count && i < NET_MAX_LINK; i++)
        sNet.peers[i] = peers[i];
    sNet.peer = peers[linkId == 0 ? 1 : 0];
    sNet.battleActive = TRUE;
    sNet.startTask = TASK_NONE;
    StringCopyN(sNet.oppName, gMpPlayers[sNet.peer].name, MP_NAME_LEN);
    sNet.oppName[MP_NAME_LEN] = EOS;
    gNetLinkActive = TRUE;
    gWirelessCommType = 0;
    gReceivedRemoteLinkPlayers = FALSE;
    MPNet_Log("co-op battle link: %d games, we are link %d", count, linkId);
}

void MP_Battle_OpenLink(u8 peer, bool8 isMaster)
{
    memset(&sNet, 0, sizeof(sNet));
    sNet.peer = peer;
    sNet.linkId = isMaster ? 0 : 1;
    sNet.linkCount = 2;
    sNet.peers[sNet.linkId ^ 1] = peer;
    sNet.battleActive = TRUE;
    sNet.startTask = TASK_NONE;
    StringCopyN(sNet.oppName, gMpPlayers[peer].name, MP_NAME_LEN);
    sNet.oppName[MP_NAME_LEN] = EOS;
    gNetLinkActive = TRUE;
    gWirelessCommType = 0;
    gReceivedRemoteLinkPlayers = FALSE;
    MPNet_Log("co-op battle link with player %d (we are link %s)", peer + 1, isMaster ? "master" : "slave");
}

void MP_Battle_CloseLink(void)
{
    FinishBattle();
}

// Friends joining a battle in progress (mp_join.c). The games already in the battle's link keep everything queued
// and add the newcomers' link ids; a solo battle's game (fresh) opens its link here, and so does a joiner's. All of
// them are in the battle already, so every game counts as ready (no start-up wait), and the sync barriers count
// on from the battle's own (seq: the master's, which every game in the battle has reached).
void MP_Battle_JoinLink(const u8 *players, u8 count, u8 linkId, u16 seq, bool8 fresh)
{
    u8 i, oldCount = fresh ? 0 : sNet.linkCount;
    if (fresh)
    {
        memset(&sNet, 0, sizeof(sNet));
        sNet.battleActive = TRUE;
        sNet.startTask = TASK_NONE;
        sNet.opened = TRUE;
        sNet.mySyncSeq = seq;
    }
    sNet.linkId = linkId;
    sNet.linkCount = count;
    for (i = 0; i < count && i < NET_MAX_LINK; i++)
    {
        sNet.peers[i] = players[i];
        if (i != linkId && i >= oldCount)
        {
            sNet.peerSyncSeq[i] = sNet.mySyncSeq;
            sNet.qHead[i] = sNet.qCount[i] = 0;
        }
    }
    sNet.peer = players[linkId == 0 ? 1 : 0];
    StringCopyN(sNet.oppName, gMpPlayers[sNet.peer].name, MP_NAME_LEN);
    sNet.oppName[MP_NAME_LEN] = EOS;
    sNet.readyMask = OthersMask();
    sNet.remoteMask = OthersMask();
    gNetLinkActive = TRUE;
    gWirelessCommType = 0;
    gReceivedRemoteLinkPlayers = TRUE;
    MPNet_Log("join link: %d games (was %d), we are link %d, sync %d", count, oldCount, linkId, sNet.mySyncSeq);
}

u16 MP_Battle_SyncSeq(void)
{
    return sNet.mySyncSeq;
}

// Nothing from the battle's link waiting to be shown (the game has caught up with the master)
bool8 MP_Battle_LinkDrained(void)
{
    u8 i;
    if (sNet.sCount != 0)
        return FALSE;
    for (i = 0; i < MAX_LINK_PLAYERS; i++)
        if (sNet.recvFlags[i])
            return FALSE;
    for (i = 0; i < NET_MAX_LINK; i++)
        if (sNet.qCount[i] != 0)
            return FALSE;
    return sNet.pendingSync == 0;
}

// ---------------------------------------------------------------------------
// Network messages

void MP_Battle_OnRelay(u8 from, const u8 *data, u16 len)
{
    s8 lid;
    if (!sNet.battleActive || (lid = LinkIdOf(from)) < 0)
        return;
    switch (data[0])
    {
    case MPR_LINK_PLAYER:
        if (len < 1 + sizeof(struct LinkPlayer))
            return;
        memcpy(&sNet.remotePlayer[lid], data + 1, sizeof(struct LinkPlayer));
        sNet.remoteMask |= 1 << lid;
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
        if (sNet.qCount[lid] == BLOCK_QUEUE)
        {
            MPNet_Log("link block queue overflow (link %d)", lid);
            return;
        }
        if (DebugOn())
            MPNet_Log("blk recv %d from %d: %02x %02x .. cmd %02x (q %d)", size, lid, data[3], data[4], size > 8 ? data[11] : 0, sNet.qCount[lid]);
        b = &sNet.queue[lid][(sNet.qHead[lid] + sNet.qCount[lid]) % BLOCK_QUEUE];
        b->size = size;
        memcpy(b->data, data + 3, size);
        sNet.qCount[lid]++;
        PumpBlocks();
        break;
    }
    case MPR_LINK_SYNC:
        if (len < 4)
            return;
        if (data[1] == 3)
        {
            sNet.readyMask |= 1 << lid;
            break;
        }
        sNet.peerSyncSeq[lid] = data[2] | (data[3] << 8);
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
        MP_Coop_OnBattleAborted();
        if (gMpFriendlyBattle)
            MP_Tourney_OnBattleResult(B_OUTCOME_WON); // they left the match
        StringCopy(sResultText, sNet.oppName);
        Append(sResultText, " disconnected. The battle was called off.");
        gFieldCallback = FieldCB_MpResult;
        SetMainCallback2(CB2_ReturnToField);
    }
    else
    {
        FinishBattle();
        MP_Coop_OnBattleAborted();
    }
}

void MP_Battle_OnPlayerLeft(u8 id)
{
    if (!sNet.battleActive || LinkIdOf(id) < 0)
        return;
    // the one who left (a triple has two other games)
    StringCopyN(sNet.oppName, gMpPlayers[id].name, MP_NAME_LEN);
    sNet.oppName[MP_NAME_LEN] = EOS;
    sNet.aborted = TRUE;
    if (sNet.startTask != TASK_NONE && FuncIsActiveTask(Task_StartMpBattle))
        return; // the start task cleans up
    AbortBattleToField();
}

// Debug (env MOEMON_LINKDEBUG): the link's state once a second during a link battle
static void DebugDump(void)
{
    static int sOn = -1;
    static u32 sTick;
    extern u32 gBattleControllerExecFlags;
    extern void (*gBattleMainFunc)(void);
    if (sOn < 0)
        sOn = getenv("MOEMON_LINKDEBUG") != NULL;
    if (!sOn || ++sTick % 60)
        return;
    MPNet_Log("linkdbg cmds %d %d %d %d", gBattleBufferA[0][0], gBattleBufferA[1][0], gBattleBufferA[2][0], gBattleBufferA[3][0]);
    MPNet_Log("linkdbg cb2=%p main=%p ms=%d exec=%08x recv=%d%d q=%d s=%d sync=%d my=%d peer=%d rlp=%d have=%d open=%d",
              (void *)(uintptr_t)((uintptr_t)gMain.callback2 - (uintptr_t)CB2_Overworld),
              (void *)(uintptr_t)((uintptr_t)gBattleMainFunc - (uintptr_t)CB2_Overworld),
              gBattleCommunication[0], gBattleControllerExecFlags, sNet.recvFlags[0], sNet.recvFlags[1],
              sNet.qCount[0] + sNet.qCount[1] + sNet.qCount[2], sNet.sCount, sNet.pendingSync, sNet.mySyncSeq, sNet.peerSyncSeq[sNet.linkId ? 0 : 1],
              gReceivedRemoteLinkPlayers, sNet.remoteMask, sNet.opened);
}

void MP_Battle_FrameUpdate(void)
{
    if (sNet.battleActive && gNetLinkActive)
    {
        PumpBlocks();
        DebugDump();
    }
}
