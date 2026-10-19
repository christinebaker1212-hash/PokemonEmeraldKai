// Moemon Co-op: a 2-4 player "link cable" over the relay, for the game's own
// link CONTESTS.
//
// The battle link (mp_battle.c NetLink_*) only knows two players. While a
// group is open, those NetLink_* calls come here instead, so everything in
// link.c the contest code uses works for up to 4 friends: GetMultiplayerId,
// SendBlock / GetBlockReceivedStatus (every player gets every block, their
// own too, like the real cable), the standby and close sync points, and
// gLinkPlayers.
//
// Matchmaking: friends talk to the LINK CONTEST receptionist in Lilycove's
// CONTEST HALL and pick the same contest. TryContestEModeLinkup (cable_club.c)
// asks MP_Contest_TryLinkup first: it waits for friends online who are ready
// for the same contest, and the one with the lowest number starts the group.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "link.h"
#include "main.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/cable_club.h"
#include "constants/songs.h"
#include "constants/vars.h"

void MPNet_Log(const char *fmt, ...);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void NetLink_FillLocalLinkPlayer(struct LinkPlayer *out);
void SaveLinkPlayers(u8 playerCount);
void ConvertLinkPlayerName(struct LinkPlayer *player);
extern struct LinkPlayer gLocalLinkPlayer;
extern u8 gFieldLinkPlayerCount;
extern u8 gLocalLinkPlayerId;

enum {
    GL_PLAYER = MPR_MINIGAME_FIRST + 14, // u8 session, struct LinkPlayer (200-213 are mp_minigame.c)
    GL_BLOCK,                            // u8 session, u16 size, data
    GL_SYNC,                             // u8 session, u8 kind, u16 seq
    CT_READY,                            // u8 category (0xFF = not ready any more)
    CT_GO,                               // leader -> group: u8 session, u8 n, u8 roster[n]
};

#define SEATS       MAX_LINK_PLAYERS
#define BLK_QUEUE   8
#define LINK_TIMEOUT (60 * 15)

static EWRAM_DATA struct
{
    bool8 active;
    u8 session;
    u8 n, slot;
    u8 roster[SEATS];
    struct LinkPlayer players[SEATS];
    bool8 havePlayer[SEATS];
    bool8 applied;
    bool8 recvFlag[SEATS];
    u8 blk[SEATS][BLK_QUEUE][BLOCK_BUFFER_SIZE];
    u16 blkSize[SEATS][BLK_QUEUE];
    u8 blkHead[SEATS], blkCount[SEATS];
    u16 mySeq, seq[SEATS];
    u8 pending;              // 0, 1 standby, 2 close
} sG = {0};

static bool8 SeatHere(u8 slot)
{
    u8 id = sG.roster[slot];
    return slot == sG.slot || (MP_IsConnected() && gMpPlayers[id].present);
}

static bool8 SlotOf(u8 id, u8 *slot)
{
    u8 i;
    for (i = 0; i < sG.n; i++)
        if (sG.roster[i] == id)
        {
            *slot = i;
            return TRUE;
        }
    return FALSE;
}

static void SendToGroup(const u8 *data, u16 len)
{
    u8 i;
    for (i = 0; i < sG.n; i++)
        if (i != sG.slot && gMpPlayers[sG.roster[i]].present)
            MP_SendRelay(sG.roster[i], data, len);
}

bool8 MP_Group_Active(void)
{
    return sG.active;
}

static void TryApplyPlayers(void)
{
    u8 i;
    if (sG.applied)
        return;
    for (i = 0; i < sG.n; i++)
        if (i != sG.slot && !sG.havePlayer[i] && SeatHere(i))
            return;
    for (i = 0; i < sG.n; i++)
    {
        if (i == sG.slot)
            gLinkPlayers[i] = gLocalLinkPlayer;
        else
        {
            gLinkPlayers[i] = sG.players[i];
            ConvertLinkPlayerName(&gLinkPlayers[i]);
        }
        gLinkPlayers[i].id = i;
    }
    sG.applied = TRUE;
    gFieldLinkPlayerCount = sG.n;
    gLocalLinkPlayerId = sG.slot;
    SaveLinkPlayers(sG.n);
    gReceivedRemoteLinkPlayers = TRUE;
    MPNet_Log("group link: %d players, we are %d", sG.n, sG.slot);
}

static void Begin(u8 session, const u8 *roster, u8 n)
{
    u8 i, msg[2 + sizeof(struct LinkPlayer)];
    memset(&sG, 0, sizeof(sG));
    sG.session = session;
    sG.n = n;
    for (i = 0; i < n; i++)
    {
        sG.roster[i] = roster[i];
        if (roster[i] == gMpLocalId)
            sG.slot = i;
    }
    NetLink_FillLocalLinkPlayer(&gLocalLinkPlayer);
    gLocalLinkPlayer.linkType = gLinkType;
    gLocalLinkPlayer.id = sG.slot;
    gReceivedRemoteLinkPlayers = FALSE;
    sG.active = TRUE;
    gNetLinkActive = TRUE;
    msg[0] = GL_PLAYER;
    msg[1] = session;
    memcpy(msg + 2, &gLocalLinkPlayer, sizeof(struct LinkPlayer));
    SendToGroup(msg, sizeof(msg));
}

static void End(void)
{
    if (!sG.active)
        return;
    sG.active = FALSE;
    gNetLinkActive = FALSE;
    gReceivedRemoteLinkPlayers = FALSE;
    MPNet_Log("group link closed");
}

// ---- link.c entry points (through NetLink_*) ----

u8 MP_Group_GetMultiplayerId(void) { return sG.slot; }
u8 MP_Group_GetPlayerCount(void)   { return sG.n; }
bool8 MP_Group_IsMaster(void)      { return sG.slot == 0; }

static void PumpBlocks(void)
{
    u8 i;
    for (i = 0; i < sG.n; i++)
    {
        if (sG.recvFlag[i] || sG.blkCount[i] == 0)
            continue;
        memcpy(gBlockRecvBuffer[i], sG.blk[i][sG.blkHead[i]], sG.blkSize[i][sG.blkHead[i]]);
        sG.recvFlag[i] = TRUE;
        sG.blkHead[i] = (sG.blkHead[i] + 1) % BLK_QUEUE;
        sG.blkCount[i]--;
    }
}

static void QueueBlock(u8 slot, const u8 *data, u16 size)
{
    u8 at;
    if (size > BLOCK_BUFFER_SIZE)
        size = BLOCK_BUFFER_SIZE;
    if (sG.blkCount[slot] >= BLK_QUEUE)
    {
        MPNet_Log("group link: block queue full (slot %d)", slot);
        return;
    }
    at = (sG.blkHead[slot] + sG.blkCount[slot]) % BLK_QUEUE;
    memcpy(sG.blk[slot][at], data, size);
    sG.blkSize[slot][at] = size;
    sG.blkCount[slot]++;
    PumpBlocks();
}

bool8 MP_Group_SendBlock(const void *src, u16 size)
{
    static u8 msg[4 + BLOCK_BUFFER_SIZE];
    if (size > BLOCK_BUFFER_SIZE)
        size = BLOCK_BUFFER_SIZE;
    QueueBlock(sG.slot, src, size);
    msg[0] = GL_BLOCK;
    msg[1] = sG.session;
    msg[2] = size & 0xFF;
    msg[3] = size >> 8;
    memcpy(msg + 4, src, size);
    SendToGroup(msg, size + 4);
    return TRUE;
}

u8 MP_Group_GetBlockReceivedStatus(void)
{
    u8 i, m = 0;
    PumpBlocks();
    for (i = 0; i < sG.n; i++)
    {
        if (sG.recvFlag[i])
            m |= 1 << i;
        else if (!SeatHere(i))
        {
            // A friend who left: an empty block so nobody waits forever
            memset(gBlockRecvBuffer[i], 0, BLOCK_BUFFER_SIZE);
            m |= 1 << i;
        }
    }
    return m;
}

void MP_Group_ResetBlockReceivedFlag(u8 who)
{
    if (who < SEATS)
        sG.recvFlag[who] = FALSE;
    PumpBlocks();
}

static void SendSync(u8 kind)
{
    u8 msg[5];
    sG.mySeq++;
    sG.pending = kind;
    msg[0] = GL_SYNC;
    msg[1] = sG.session;
    msg[2] = kind;
    msg[3] = sG.mySeq & 0xFF;
    msg[4] = sG.mySeq >> 8;
    SendToGroup(msg, sizeof(msg));
}

bool8 MP_Group_IsTaskFinished(void)
{
    u8 i;
    if (!sG.pending)
        return TRUE;
    for (i = 0; i < sG.n; i++)
        if (i != sG.slot && SeatHere(i) && (s16)(sG.seq[i] - sG.mySeq) < 0)
            return FALSE;
    if (sG.pending == 2)
    {
        sG.pending = 0;
        End();
        return TRUE;
    }
    sG.pending = 0;
    return TRUE;
}

void MP_Group_SetStandbyCallback(void)
{
    if (!sG.pending)
        SendSync(1);
}

void MP_Group_SetCloseCallback(void)
{
    if (!sG.pending)
        SendSync(2);
}

void MP_Group_Close(void)
{
    End();
}

// Every frame: the close sync point finishes by itself (the game only
// watches gReceivedRemoteLinkPlayers after asking to close)
static void UpdateGroup(void)
{
    if (!sG.active)
        return;
    if (!MP_IsConnected())
    {
        End();
        return;
    }
    TryApplyPlayers();
    PumpBlocks();
    if (sG.pending == 2)
        MP_Group_IsTaskFinished();
}

// ---------------------------------------------------------------------------
// LINK CONTEST matchmaking

static struct
{
    u8 taskId;
    u8 category;
    u8 readyCat[MP_MAX_PLAYERS];
    u8 lastMask;
    u32 maskSince, frame, sentAt, linkStart;
    bool8 linking;
} sCt = {.taskId = TASK_NONE, .readyCat = {0xFF, 0xFF, 0xFF, 0xFF}};

static void SendReady(u8 cat)
{
    u8 msg[2] = {CT_READY, cat};
    MP_SendRelay(0xFF, msg, sizeof(msg));
}

static u8 ReadyMask(void)
{
    u8 i, m = 1 << gMpLocalId;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && sCt.readyCat[i] == sCt.category)
            m |= 1 << i;
    return m;
}

static u8 Count(u8 m)
{
    u8 n = 0;
    while (m)
    {
        n += m & 1;
        m >>= 1;
    }
    return n;
}

static void FinishLinkup(u8 taskId, u16 result)
{
    gSpecialVar_Result = result;
    SendReady(0xFF);
    DestroyTask(taskId);
    sCt.taskId = TASK_NONE;
    sCt.linking = FALSE;
    if (result != LINKUP_SUCCESS)
        End();
    ScriptContext_Enable();
}

static void Task_CoopLinkup(u8 taskId)
{
    u8 mask;
    sCt.frame++;
    if (sCt.linking)
    {
        if (gReceivedRemoteLinkPlayers)
        {
            PlaySE(SE_PIN);
            FinishLinkup(taskId, LINKUP_SUCCESS);
        }
        else if (!sG.active || sCt.frame - sCt.linkStart > LINK_TIMEOUT)
            FinishLinkup(taskId, LINKUP_CONNECTION_ERROR);
        return;
    }
    if (JOY_NEW(B_BUTTON) || !MP_IsConnected())
    {
        PlaySE(SE_SELECT);
        FinishLinkup(taskId, LINKUP_FAILED);
        return;
    }
    if (sCt.frame - sCt.sentAt > 60 * 2)
    {
        sCt.sentAt = sCt.frame;
        SendReady(sCt.category); // for friends who just got here
    }
    mask = ReadyMask();
    if (mask != sCt.lastMask)
    {
        sCt.lastMask = mask;
        sCt.maskSince = sCt.frame;
    }
    // The lowest-numbered ready player starts once everyone online is ready,
    // or 10 seconds after the last friend got ready
    if (Count(mask) >= 2 && (mask & ((1 << gMpLocalId) - 1)) == 0)
    {
        u8 i, everyone = 0;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i == gMpLocalId || gMpPlayers[i].present)
                everyone |= 1 << i;
        if (mask == everyone || Count(mask) == SEATS || sCt.frame - sCt.maskSince > 60 * 10)
        {
            u8 roster[SEATS], n = 0, msg[3 + SEATS];
            u8 session = (u8)(sCt.frame * 37 + gMpLocalId * 11 + 1);
            for (i = 0; i < MP_MAX_PLAYERS && n < SEATS; i++)
                if (mask & (1 << i))
                    roster[n++] = i;
            msg[0] = CT_GO;
            msg[1] = session;
            msg[2] = n;
            memcpy(msg + 3, roster, n);
            for (i = 0; i < n; i++)
                if (roster[i] != gMpLocalId)
                    MP_SendRelay(roster[i], msg, 3 + n);
            Begin(session, roster, n);
            sCt.linking = TRUE;
            sCt.linkStart = sCt.frame;
        }
    }
}

// cable_club.c TryContestEModeLinkup / TryContestGModeLinkup: TRUE = handled here
bool8 MP_Contest_TryLinkup(void)
{
    if (!MP_IsConnected() || MP_ConnectedCount() < 2)
        return FALSE;
    if (sCt.taskId != TASK_NONE)
        return TRUE;
    End();
    sCt.category = VarGet(VAR_CONTEST_CATEGORY);
    sCt.lastMask = 0;
    sCt.frame = sCt.sentAt = 0;
    sCt.maskSince = 0;
    sCt.linking = FALSE;
    SendReady(sCt.category);
    sCt.taskId = CreateTask(Task_CoopLinkup, 80);
    return TRUE;
}

// ---------------------------------------------------------------------------

void MP_Group_OnRelay(u8 from, const u8 *data, u16 len)
{
    u8 slot;
    if (len < 2 || from >= MP_MAX_PLAYERS)
        return;
    switch (data[0])
    {
    case CT_READY:
        // Let friends know someone's waiting at the counter (once per visit)
        if (data[1] < 5 && sCt.readyCat[from] != data[1] && sCt.taskId == TASK_NONE)
        {
            static const char *const sCats[] = {"COOL", "BEAUTY", "CUTE", "SMART", "TOUGH"};
            char name[16], line[80];
            u8 buf[80];
            MP_GameToAscii(name, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
            snprintf(line, sizeof(line), "%s is waiting at the LINK CONTEST counter (%s).", name, sCats[data[1]]);
            MP_AsciiToGame(buf, line, sizeof(buf) - 1);
            MP_Toast(buf);
        }
        sCt.readyCat[from] = data[1];
        break;
    case CT_GO:
        if (sCt.taskId != TASK_NONE && !sCt.linking && len >= 3 && data[2] >= 2 && data[2] <= SEATS && len >= 3 + data[2])
        {
            u8 i;
            for (i = 0; i < data[2]; i++)
                if (data[3 + i] == gMpLocalId)
                    break;
            if (i == data[2])
                break;
            Begin(data[1], data + 3, data[2]);
            sCt.linking = TRUE;
            sCt.linkStart = sCt.frame;
        }
        break;
    case GL_PLAYER:
        if (sG.active && data[1] == sG.session && len >= 2 + sizeof(struct LinkPlayer) && SlotOf(from, &slot))
        {
            memcpy(&sG.players[slot], data + 2, sizeof(struct LinkPlayer));
            sG.havePlayer[slot] = TRUE;
            TryApplyPlayers();
        }
        break;
    case GL_BLOCK:
        if (sG.active && data[1] == sG.session && len >= 4 && SlotOf(from, &slot))
        {
            u16 size = data[2] | (data[3] << 8);
            if (size <= len - 4)
                QueueBlock(slot, data + 4, size);
        }
        break;
    case GL_SYNC:
        if (sG.active && data[1] == sG.session && len >= 5 && SlotOf(from, &slot))
            sG.seq[slot] = data[3] | (data[4] << 8);
        break;
    }
}

void MP_Group_OnPlayerLeft(u8 id)
{
    if (id < MP_MAX_PLAYERS)
        sCt.readyCat[id] = 0xFF;
}

void MP_Group_Update(void)
{
    UpdateGroup();
}
