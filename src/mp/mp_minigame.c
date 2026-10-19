// Moemon Co-op: the BERRY BLENDER and BERRY CRUSH with friends, online.
//
// CO-OP > BERRY GAMES > BERRY BLENDER asks every friend online. Those who say
// yes pick a BERRY in their own BAG, and everyone plays the game's own Berry
// Blender (berry_blender.c, the NPC mode) with the friends in the NPC seats:
// their hits come over the relay instead of from the NPC tasks.
//
// The player with the lowest number in the group leads: it sends where the
// arrow is, how fast it spins and the progress bar ~30 times a second, and
// says when the game is over (with the final speed, time and scores) so
// everybody gets the same POKeBLOCK. If the leader leaves, the next one
// takes over.
//
// BERRY CRUSH (berry_crush.c) was made for the Wireless Adapter: the leader
// sends the game state every frame, the others send their A presses, and
// everyone meets at sync points and swaps small blocks. Below is a tiny
// stand-in for those calls over the relay (MP_Crush_*).
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "item.h"
#include "link.h"
#include "constants/characters.h"
#include "main.h"
#include "pokeblock.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "link_rfu.h"
#include "overworld.h"
#include "text.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_Linkx_OpenUnion(void);
extern const u8 EventScript_MpBerryBlender[];
extern const u8 EventScript_MpBerryCrush[];
void StartBerryCrush(MainCallback exitCallback);

enum {
    MG_INVITE = MPR_MINIGAME_FIRST, // host -> friend: u8 session, u8 game
    MG_ANSWER,                      // friend -> host: u8 session, u8 answer
    MG_CANCEL,                      // u8 session (host calls it off / a guest leaves)
    MG_START,                       // host -> guests: u8 session, u8 n, u8 roster[n]
    MG_BERRY,                       // -> all: u8 session, u8 slot, u16 item
    MG_GO,                          // leader -> all: u8 session, u16 items[4]
    MG_HIT,                         // -> all: u8 session, u8 slot, u16 score
    MG_STATE,                       // leader -> all: u8 session, u16 arrow, s16 speed, u16 progress
    MG_END,                         // leader -> all: u8 session, u16 rpm, u32 time, u16 scores[4][3]
    MG_SYNC,                        // crush: u8 session, u16 seq (sync point reached)
    MG_BLOCK,                       // crush: u8 session, u16 size, data
    MG_PACKET,                      // crush: u8 session, u16 packet[RFU_PACKET_SIZE]
    MG_NEWS,                        // -> all: big news to show as a toast (ASCII), e.g. a MOEGATCHA UR
    // up to MPR_MINIGAME_FIRST + 13; mp_grouplink.c has the rest
};

enum { GAME_BLENDER, GAME_CRUSH };

enum { ANS_NONE, ANS_ASKING, ANS_YES, ANS_NO, ANS_BUSY, ANS_NOCASE, ANS_NOBERRY, ANS_FULL, ANS_NOJAR };
enum { ROLE_NONE, ROLE_HOST, ROLE_INVITED, ROLE_GUEST, ROLE_PLAYING };
enum { SC_LOBBY, SC_ASKED, SC_WAITING };

#define MAX_SEATS     4
#define HITQ          16
#define ASK_TIMEOUT   (60 * 15)
#define STATE_EVERY   2

#define PKT_QUEUE     16
#define BLK_QUEUE     4

struct CrushPacket { u16 cmd[CMD_LENGTH]; };

static EWRAM_DATA struct
{
    struct CrushPacket pkt[MAX_SEATS][PKT_QUEUE];
    u8 pktHead[MAX_SEATS], pktCount[MAX_SEATS];
    u8 blk[MAX_SEATS][BLK_QUEUE][BLOCK_BUFFER_SIZE];
    u16 blkSize[MAX_SEATS][BLK_QUEUE];
    u8 blkHead[MAX_SEATS], blkCount[MAX_SEATS];
    bool8 recvFlag[MAX_SEATS];
    u16 mySeq, seq[MAX_SEATS];
    bool8 pending;
    bool8 broken;
    bool8 savedAutoScroll;
} sCrush;

static struct
{
    u8 role;
    u8 game;
    u8 session;
    u8 host;
    u8 answer[MP_MAX_PLAYERS];
    u8 roster[MAX_SEATS];
    u8 n;
    u16 berry[MAX_SEATS];
    u8 berryMask;
    bool8 go;
    u16 goItems[MAX_SEATS];
    u16 hitQ[MAX_SEATS][HITQ];
    u8 hitHead[MAX_SEATS], hitTail[MAX_SEATS];
    bool8 stateNew;
    u16 stArrow, stProgress;
    s16 stSpeed;
    bool8 endNew;
    u16 endRpm;
    u32 endTime;
    u16 endScores[MAX_SEATS][3];
    u32 frame, askedAt, sentState;
    u8 inviteFrom, inviteSession, inviteGame;
    bool8 invitePending;
    // UI
    u8 taskId;
    u8 screen;
    u16 cursor;
    char note[96];
} sMg = {.taskId = TASK_NONE};

static void Name(u8 id, char *out)
{
    MP_GameToAscii(out, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
}

static void Send(u8 target, const u8 *data, u16 len)
{
    MP_SendRelay(target, data, len);
}

static void SendToRoster(const u8 *data, u16 len)
{
    u8 i;
    for (i = 0; i < sMg.n; i++)
        if (sMg.roster[i] != gMpLocalId && gMpPlayers[sMg.roster[i]].present)
            Send(sMg.roster[i], data, len);
}

static bool8 InRoster(u8 id, u8 *slot)
{
    u8 i;
    for (i = 0; i < sMg.n; i++)
        if (sMg.roster[i] == id)
        {
            if (slot)
                *slot = i;
            return TRUE;
        }
    return FALSE;
}

static bool8 SeatHere(u8 slot)
{
    u8 id = sMg.roster[slot];
    return id == gMpLocalId || (MP_IsConnected() && gMpPlayers[id].present);
}

static const char *GameTitle(u8 game)
{
    return game == GAME_CRUSH ? "BERRY CRUSH" : "BERRY BLENDER";
}

// Can we play right now? (same checks as the game's own NPCs)
static u8 GameCheck(u8 game)
{
    if (game == GAME_CRUSH)
    {
        if (!CheckBagHasItem(ITEM_POWDER_JAR, 1))
            return ANS_NOJAR;
        if (!IsBagPocketNonEmpty(POCKET_BERRIES))
            return ANS_NOBERRY;
        return ANS_YES;
    }
    if (!CheckBagHasItem(ITEM_POKEBLOCK_CASE, 1))
        return ANS_NOCASE;
    if (!IsBagPocketNonEmpty(POCKET_BERRIES))
        return ANS_NOBERRY;
    if (GetFirstFreePokeblockSlot() == -1)
        return ANS_FULL;
    return ANS_YES;
}

static const char *WhyText(u8 ans)
{
    switch (ans)
    {
    case ANS_NOCASE:  return "You need a POKEBLOCK CASE (Lilycove CONTEST HALL).";
    case ANS_NOBERRY: return "You don't have any BERRIES.";
    case ANS_FULL:    return "Your POKEBLOCK CASE is full.";
    case ANS_NOJAR:   return "You need the POWDER JAR (Slateport MARKET).";
    default:          return "";
    }
}

static void ToastAscii(const char *text)
{
    u8 buf[96];
    MP_AsciiToGame(buf, text, sizeof(buf) - 1);
    MP_Toast(buf);
}

static void ResetSession(void)
{
    u8 keepTask = sMg.taskId;
    u32 frame = sMg.frame;
    bool8 inv = sMg.invitePending;
    u8 invFrom = sMg.inviteFrom, invSes = sMg.inviteSession, invGame = sMg.inviteGame;
    u32 askedAt = sMg.askedAt;
    memset(&sMg, 0, sizeof(sMg));
    sMg.taskId = keepTask;
    sMg.frame = frame;
    sMg.invitePending = inv;
    sMg.inviteFrom = invFrom;
    sMg.inviteSession = invSes;
    sMg.inviteGame = invGame;
    sMg.askedAt = askedAt;
}

// ---------------------------------------------------------------------------
// Inside the blender (berry_blender.c calls these)

bool8 MP_Blender_Active(void)
{
    return sMg.role == ROLE_PLAYING && sMg.game == GAME_BLENDER;
}

u8 MP_Blender_LocalSlot(void)
{
    u8 slot = 0;
    InRoster(gMpLocalId, &slot);
    return slot;
}

u8 MP_Blender_NumPlayers(void)
{
    return sMg.n;
}

bool8 MP_Blender_IsLeader(void)
{
    u8 i;
    for (i = 0; i < sMg.n; i++)
        if (SeatHere(i))
            return sMg.roster[i] == gMpLocalId;
    return TRUE;
}

static void FillNames(void)
{
    u8 i;
    for (i = 0; i < sMg.n; i++)
    {
        if (sMg.roster[i] == gMpLocalId)
            StringCopy(gLinkPlayers[i].name, gSaveBlock2Ptr->playerName);
        else
        {
            memcpy(gLinkPlayers[i].name, gMpPlayers[sMg.roster[i]].name, PLAYER_NAME_LENGTH);
            gLinkPlayers[i].name[PLAYER_NAME_LENGTH] = EOS;
        }
        gLinkPlayers[i].language = GAME_LANGUAGE;
    }
}

void MP_Blender_FillNames(void)
{
    FillNames();
}

void MP_Blender_SendBerry(u16 item)
{
    u8 msg[5], slot = MP_Blender_LocalSlot();
    sMg.berry[slot] = item;
    sMg.berryMask |= 1 << slot;
    msg[0] = MG_BERRY;
    msg[1] = sMg.session;
    msg[2] = slot;
    msg[3] = item & 0xFF;
    msg[4] = item >> 8;
    SendToRoster(msg, sizeof(msg));
}

// Everyone's BERRY, once the leader says go
bool8 MP_Blender_Ready(u16 *items)
{
    u8 i;
    if (!sMg.go)
        return FALSE;
    for (i = 0; i < MAX_SEATS; i++)
        items[i] = sMg.goItems[i];
    return TRUE;
}

void MP_Blender_SendHit(u16 score)
{
    u8 msg[5];
    msg[0] = MG_HIT;
    msg[1] = sMg.session;
    msg[2] = MP_Blender_LocalSlot();
    msg[3] = score & 0xFF;
    msg[4] = score >> 8;
    SendToRoster(msg, sizeof(msg));
}

u16 MP_Blender_TakeHit(u8 slot)
{
    u16 s;
    if (slot >= MAX_SEATS || sMg.hitHead[slot] == sMg.hitTail[slot])
        return 0;
    s = sMg.hitQ[slot][sMg.hitHead[slot]];
    sMg.hitHead[slot] = (sMg.hitHead[slot] + 1) % HITQ;
    return s;
}

void MP_Blender_SendState(u16 arrow, s16 speed, u16 progress)
{
    u8 msg[8];
    if (sMg.frame - sMg.sentState < STATE_EVERY)
        return;
    sMg.sentState = sMg.frame;
    msg[0] = MG_STATE;
    msg[1] = sMg.session;
    msg[2] = arrow & 0xFF;
    msg[3] = arrow >> 8;
    msg[4] = (u16)speed & 0xFF;
    msg[5] = (u16)speed >> 8;
    msg[6] = progress & 0xFF;
    msg[7] = progress >> 8;
    SendToRoster(msg, sizeof(msg));
}

bool8 MP_Blender_TakeState(u16 *arrow, s16 *speed, u16 *progress)
{
    if (!sMg.stateNew)
        return FALSE;
    sMg.stateNew = FALSE;
    *arrow = sMg.stArrow;
    *speed = sMg.stSpeed;
    *progress = sMg.stProgress;
    return TRUE;
}

void MP_Blender_SendEnd(u16 rpm, u32 time, const u16 *scores)
{
    u8 msg[2 + 2 + 4 + MAX_SEATS * 3 * 2];
    u8 i, p = 0;
    msg[p++] = MG_END;
    msg[p++] = sMg.session;
    msg[p++] = rpm & 0xFF;
    msg[p++] = rpm >> 8;
    for (i = 0; i < 4; i++)
        msg[p++] = time >> (8 * i);
    for (i = 0; i < MAX_SEATS * 3; i++)
    {
        msg[p++] = scores[i] & 0xFF;
        msg[p++] = scores[i] >> 8;
    }
    SendToRoster(msg, p);
    printf("[mp] blender: end sent (rpm %d, time %u)\n", rpm, (unsigned)time);
}

bool8 MP_Blender_TakeEnd(u16 *rpm, u32 *time, u16 *scores)
{
    if (!sMg.endNew)
        return FALSE;
    sMg.endNew = FALSE;
    *rpm = sMg.endRpm;
    *time = sMg.endTime;
    memcpy(scores, sMg.endScores, sizeof(sMg.endScores));
    return TRUE;
}

// The blender closed
void MP_Blender_Finish(void)
{
    if (sMg.role == ROLE_PLAYING && sMg.game == GAME_BLENDER)
        ResetSession();
}

// ---------------------------------------------------------------------------
// BERRY CRUSH over the relay (berry_crush.c calls these instead of the link)

bool8 MP_Crush_Active(void)
{
    return sMg.role == ROLE_PLAYING && sMg.game == GAME_CRUSH;
}

u8 MP_Crush_NumPlayers(void)
{
    return sMg.n;
}

u8 MP_Crush_LocalSlot(void)
{
    return MP_Blender_LocalSlot();
}

// A friend left: the game can't go on (their sync points never come)
bool8 MP_Crush_Broken(void)
{
    return MP_Crush_Active() && sCrush.broken;
}

void MP_Crush_Standby(void)
{
    u8 msg[4];
    if (sCrush.pending)
        return;
    sCrush.mySeq++;
    sCrush.pending = TRUE;
    if (getenv("MOEMON_CRUSH_LOG"))
        printf("[mp] crush: sync %d (frame %u)\n", sCrush.mySeq, (unsigned)sMg.frame);
    msg[0] = MG_SYNC;
    msg[1] = sMg.session;
    msg[2] = sCrush.mySeq & 0xFF;
    msg[3] = sCrush.mySeq >> 8;
    SendToRoster(msg, sizeof(msg));
}

bool8 MP_Crush_TaskFinished(void)
{
    u8 i;
    if (!sCrush.pending)
        return TRUE;
    for (i = 0; i < sMg.n; i++)
    {
        if (sMg.roster[i] == gMpLocalId || !SeatHere(i))
            continue;
        if ((s16)(sCrush.seq[i] - sCrush.mySeq) < 0)
            return FALSE;
    }
    sCrush.pending = FALSE;
    return TRUE;
}

static void PumpBlocks(void)
{
    u8 i;
    for (i = 0; i < sMg.n; i++)
    {
        if (sCrush.recvFlag[i] || sCrush.blkCount[i] == 0)
            continue;
        memcpy(gBlockRecvBuffer[i], sCrush.blk[i][sCrush.blkHead[i]], sCrush.blkSize[i][sCrush.blkHead[i]]);
        sCrush.recvFlag[i] = TRUE;
        sCrush.blkHead[i] = (sCrush.blkHead[i] + 1) % BLK_QUEUE;
        sCrush.blkCount[i]--;
    }
}

static void QueueBlock(u8 slot, const u8 *data, u16 size)
{
    u8 at;
    if (size > BLOCK_BUFFER_SIZE)
        size = BLOCK_BUFFER_SIZE;
    if (sCrush.blkCount[slot] >= BLK_QUEUE)
        return;
    at = (sCrush.blkHead[slot] + sCrush.blkCount[slot]) % BLK_QUEUE;
    memcpy(sCrush.blk[slot][at], data, size);
    sCrush.blkSize[slot][at] = size;
    sCrush.blkCount[slot]++;
    PumpBlocks();
}

bool8 MP_Crush_SendBlock(const void *src, u16 size)
{
    static u8 msg[4 + BLOCK_BUFFER_SIZE];
    if (size > BLOCK_BUFFER_SIZE)
        size = BLOCK_BUFFER_SIZE;
    QueueBlock(MP_Crush_LocalSlot(), src, size);   // the link hands senders their own block too
    if (getenv("MOEMON_CRUSH_LOG"))
        printf("[mp] crush: block out size %d first %d (frame %u)\n", size, ((const u8 *)src)[0], (unsigned)sMg.frame);
    msg[0] = MG_BLOCK;
    msg[1] = sMg.session;
    msg[2] = size & 0xFF;
    msg[3] = size >> 8;
    memcpy(msg + 4, src, size);
    SendToRoster(msg, size + 4);
    return TRUE;
}

u8 MP_Crush_BlockStatus(void)
{
    u8 i, m = 0;
    PumpBlocks();
    for (i = 0; i < sMg.n; i++)
    {
        if (sCrush.recvFlag[i])
            m |= 1 << i;
        else if (!SeatHere(i))
        {
            // Gone: an empty block so the others don't wait forever
            memset(gBlockRecvBuffer[i], 0, BLOCK_BUFFER_SIZE);
            m |= 1 << i;
        }
    }
    return m;
}

void MP_Crush_ResetBlocks(void)
{
    memset(sCrush.recvFlag, 0, sizeof(sCrush.recvFlag));
    PumpBlocks();
}

static void QueuePacket(u8 slot, const u16 *packet)
{
    u8 at;
    if (sCrush.pktCount[slot] >= PKT_QUEUE)
    {
        sCrush.pktHead[slot] = (sCrush.pktHead[slot] + 1) % PKT_QUEUE;
        sCrush.pktCount[slot]--;
    }
    at = (sCrush.pktHead[slot] + sCrush.pktCount[slot]) % PKT_QUEUE;
    memset(&sCrush.pkt[slot][at], 0, sizeof(struct CrushPacket));
    sCrush.pkt[slot][at].cmd[0] = RFUCMD_SEND_PACKET;
    memcpy(&sCrush.pkt[slot][at].cmd[1], packet, RFU_PACKET_SIZE * 2);
    sCrush.pktCount[slot]++;
}

// The leader's packet goes to everyone; the others' only matter to the leader
void MP_Crush_SendPacket(void *data)
{
    u8 msg[2 + RFU_PACKET_SIZE * 2];
    u8 slot = MP_Crush_LocalSlot();
    QueuePacket(slot, data);
    msg[0] = MG_PACKET;
    msg[1] = sMg.session;
    memcpy(msg + 2, data, RFU_PACKET_SIZE * 2);
    if (slot == 0)
        SendToRoster(msg, sizeof(msg));
    else if (gMpPlayers[sMg.roster[0]].present)
        Send(sMg.roster[0], msg, sizeof(msg));
}

// Once a frame: one packet per player into gRecvCmds, like the adapter does
static void PumpPackets(void)
{
    u8 i;
    for (i = 0; i < MAX_SEATS; i++)
    {
        if (i >= sMg.n || sCrush.pktCount[i] == 0)
        {
            if (i < sMg.n)
                memset(gRecvCmds[i], 0, sizeof(gRecvCmds[i]));
            continue;
        }
        // Don't fall far behind: skip ahead when too many are waiting
        while (sCrush.pktCount[i] > 6)
        {
            sCrush.pktHead[i] = (sCrush.pktHead[i] + 1) % PKT_QUEUE;
            sCrush.pktCount[i]--;
        }
        memcpy(gRecvCmds[i], sCrush.pkt[i][sCrush.pktHead[i]].cmd, sizeof(gRecvCmds[i]));
        sCrush.pktHead[i] = (sCrush.pktHead[i] + 1) % PKT_QUEUE;
        sCrush.pktCount[i]--;
    }
}

// The special in EventScript_MpBerryCrush
void MP_StartBerryCrush(void)
{
    memset(&sCrush, 0, sizeof(sCrush));
    sCrush.savedAutoScroll = gTextFlags.autoScroll;
    FillNames();
    StartBerryCrush(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void MP_Crush_Finish(void)
{
    if (!MP_Crush_Active())
        return;
    gTextFlags.autoScroll = sCrush.savedAutoScroll;
    if (sCrush.broken)
        ToastAscii("A friend left, so BERRY CRUSH ended.");
    ResetSession();
}

// Leader: send GO once every seat still here has a BERRY. A friend who left
// before choosing gets a PECHA BERRY so the seats stay the same for everyone.
static void TryGo(void)
{
    u8 i, msg[2 + MAX_SEATS * 2];
    if (sMg.go || !MP_Blender_IsLeader())
        return;
    for (i = 0; i < sMg.n; i++)
        if (SeatHere(i) && !(sMg.berryMask & (1 << i)))
            return;
    msg[0] = MG_GO;
    msg[1] = sMg.session;
    for (i = 0; i < MAX_SEATS; i++)
    {
        u16 item = ITEM_NONE;
        if (i < sMg.n)
            item = (sMg.berryMask & (1 << i)) ? sMg.berry[i] : ITEM_PECHA_BERRY;
        sMg.goItems[i] = item;
        msg[2 + i * 2] = item & 0xFF;
        msg[3 + i * 2] = item >> 8;
    }
    sMg.go = TRUE;
    SendToRoster(msg, sizeof(msg));
}

// ---------------------------------------------------------------------------
// Invites (CO-OP > BERRY GAMES)

static void Draw(void);
static void Task_Minigame(u8 taskId);

static u8 JoinedCount(void)
{
    u8 i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && sMg.answer[i] == ANS_YES && gMpPlayers[i].present)
            n++;
    return n;
}

static void OpenUi(u8 screen)
{
    sMg.screen = screen;
    sMg.cursor = 0;
    sMg.note[0] = 0;
    if (sMg.taskId == TASK_NONE)
    {
        MpUi_Open();
        sMg.taskId = CreateTask(Task_Minigame, 80);
    }
    Draw();
}

static void CloseUi(void)
{
    if (sMg.taskId != TASK_NONE)
    {
        DestroyTask(sMg.taskId);
        sMg.taskId = TASK_NONE;
        MpUi_Close();
    }
}

// From the BERRY GAMES page (which has already closed)
static void HostGame(u8 game)
{
    u8 i, msg[3];
    u8 why;
    ResetSession();
    sMg.game = game;
    why = GameCheck(game);
    if (why != ANS_YES)
    {
        OpenUi(SC_LOBBY);
        snprintf(sMg.note, sizeof(sMg.note), "%s", WhyText(why));
        sMg.role = ROLE_NONE;
        Draw();
        return;
    }
    sMg.role = ROLE_HOST;
    sMg.host = gMpLocalId;
    sMg.session = (Random() & 0xFE) + 1;
    msg[0] = MG_INVITE;
    msg[1] = sMg.session;
    msg[2] = game;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (i == gMpLocalId || !gMpPlayers[i].present)
            continue;
        sMg.answer[i] = ANS_ASKING;
        Send(i, msg, sizeof(msg));
    }
    OpenUi(SC_LOBBY);
}

void MP_Minigame_HostBlender(void)
{
    HostGame(GAME_BLENDER);
}

void MP_Minigame_HostCrush(void)
{
    HostGame(GAME_CRUSH);
}

static void Launch(void)
{
    CloseUi();
    sMg.role = ROLE_PLAYING;
    sMg.berryMask = 0;
    sMg.go = FALSE;
    if (sMg.game == GAME_CRUSH)
    {
        ScriptContext_SetupScript(EventScript_MpBerryCrush);
        return;
    }
    gSpecialVar_0x8004 = sMg.n - 1;
    ScriptContext_SetupScript(EventScript_MpBerryBlender);
}

static void HostStart(void)
{
    u8 i, msg[3 + MAX_SEATS];
    sMg.n = 0;
    sMg.roster[sMg.n++] = gMpLocalId;
    for (i = 0; i < MP_MAX_PLAYERS && sMg.n < MAX_SEATS; i++)
        if (i != gMpLocalId && sMg.answer[i] == ANS_YES && gMpPlayers[i].present)
            sMg.roster[sMg.n++] = i;
    msg[0] = MG_START;
    msg[1] = sMg.session;
    msg[2] = sMg.n;
    for (i = 0; i < sMg.n; i++)
        msg[3 + i] = sMg.roster[i];
    // Friends who were asked but didn't join hear it's off
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (i == gMpLocalId || !gMpPlayers[i].present || sMg.answer[i] == ANS_NONE)
            continue;
        if (InRoster(i, NULL))
            Send(i, msg, 3 + sMg.n);
        else if (sMg.answer[i] == ANS_ASKING)
        {
            u8 c[2] = {MG_CANCEL, sMg.session};
            Send(i, c, 2);
        }
    }
    Launch();
}

static void Answer(u8 ans)
{
    u8 msg[3] = {MG_ANSWER, sMg.inviteSession, ans};
    Send(sMg.inviteFrom, msg, sizeof(msg));
}

static void Draw(void)
{
    char line[96], name[16];
    u8 i, row = 0;
    switch (sMg.screen)
    {
    case SC_LOBBY:
    {
        MpUi_Begin(GameTitle(sMg.game));
        if (sMg.role != ROLE_HOST)
        {
            MpUi_TextAscii(1, "You can't play right now.");
            MpUi_End();
            MpUi_HelpAscii(sMg.note[0] ? sMg.note : "{B} BACK");
            break;
        }
        for (i = 0; i < MP_MAX_PLAYERS && row < MPUI_ROWS - 2; i++)
        {
            static const char *const sAns[] = {"", "ASKING", "JOINED", "NO THANKS", "BUSY", "NO CASE", "NO BERRIES", "CASE FULL", "NO JAR"};
            if (i == gMpLocalId || sMg.answer[i] == ANS_NONE)
                continue;
            Name(i, name);
            MpUi_RowAscii(row++, FALSE, name, gMpPlayers[i].present ? sAns[sMg.answer[i]] : "LEFT");
        }
        if (row == 0)
            MpUi_TextAscii(row++, "Nobody else is online right now.");
        MpUi_RowAscii(MPUI_ROWS - 2, sMg.cursor == 0, "START!", NULL);
        MpUi_RowAscii(MPUI_ROWS - 1, sMg.cursor == 1, "CALL IT OFF", NULL);
        MpUi_End();
        if (sMg.note[0])
            MpUi_HelpAscii(sMg.note);
        else if (JoinedCount() == 0)
            MpUi_HelpAscii("Asking your friends... START when someone joins.");
        else
            MpUi_HelpAscii(sMg.game == GAME_CRUSH ? "Everyone picks a BERRY, then crush it together!" : "Everyone picks a BERRY, then blend together!");
        break;
    }
    case SC_ASKED:
        Name(sMg.inviteFrom, name);
        MpUi_Begin(GameTitle(sMg.game));
        snprintf(line, sizeof(line), sMg.game == GAME_CRUSH ? "%s wants to play BERRY CRUSH!" : "%s wants to blend BERRIES!", name);
        MpUi_TextAscii(0, line);
        MpUi_RowAscii(2, sMg.cursor == 0, "JOIN", NULL);
        MpUi_RowAscii(3, sMg.cursor == 1, "NO THANKS", NULL);
        MpUi_End();
        MpUi_HelpAscii("You'll pick a BERRY from your BAG.");
        break;
    case SC_WAITING:
        Name(sMg.host, name);
        MpUi_Begin(GameTitle(sMg.game));
        snprintf(line, sizeof(line), "Waiting for %s to start...", name);
        MpUi_TextAscii(1, line);
        MpUi_End();
        MpUi_HelpAscii("{B} LEAVE");
        break;
    }
}

static void Task_Minigame(u8 taskId)
{
    switch (sMg.screen)
    {
    case SC_LOBBY:
        if (sMg.role != ROLE_HOST)
        {
            if (JOY_NEW(A_BUTTON | B_BUTTON))
            {
                PlaySE(SE_SELECT);
                CloseUi();
                MP_Linkx_OpenUnion();
            }
            return;
        }
        if (MpUi_MoveCursor(&sMg.cursor, 2))
        {
            sMg.note[0] = 0;
            Draw();
            return;
        }
        if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sMg.cursor == 1))
        {
            u8 i, c[2] = {MG_CANCEL, sMg.session};
            PlaySE(SE_SELECT);
            for (i = 0; i < MP_MAX_PLAYERS; i++)
                if (i != gMpLocalId && sMg.answer[i] != ANS_NONE && gMpPlayers[i].present)
                    Send(i, c, 2);
            ResetSession();
            CloseUi();
            MP_Linkx_OpenUnion();
            return;
        }
        if (JOY_NEW(A_BUTTON))
        {
            if (JoinedCount() == 0)
            {
                PlaySE(SE_FAILURE);
                snprintf(sMg.note, sizeof(sMg.note), "Wait until at least one friend joins.");
                Draw();
                return;
            }
            if (GameCheck(sMg.game) != ANS_YES)
            {
                PlaySE(SE_FAILURE);
                snprintf(sMg.note, sizeof(sMg.note), "%s", WhyText(GameCheck(sMg.game)));
                Draw();
                return;
            }
            PlaySE(SE_SELECT);
            HostStart();
        }
        return;
    case SC_ASKED:
        if (MpUi_MoveCursor(&sMg.cursor, 2))
        {
            Draw();
            return;
        }
        if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (JOY_NEW(A_BUTTON) && sMg.cursor == 0)
            {
                u8 why = GameCheck(sMg.game);
                if (why != ANS_YES)
                {
                    Answer(why);
                    ResetSession();
                    CloseUi();
                    ToastAscii(WhyText(why));
                    return;
                }
                Answer(ANS_YES);
                sMg.role = ROLE_GUEST;
                sMg.host = sMg.inviteFrom;
                sMg.session = sMg.inviteSession;
                sMg.screen = SC_WAITING;
                Draw();
                return;
            }
            Answer(ANS_NO);
            ResetSession();
            CloseUi();
        }
        return;
    case SC_WAITING:
        if (JOY_NEW(B_BUTTON))
        {
            u8 c[2] = {MG_CANCEL, sMg.session};
            PlaySE(SE_SELECT);
            Send(sMg.host, c, 2);
            ResetSession();
            CloseUi();
        }
        return;
    }
}

// ---------------------------------------------------------------------------
// Network

void MP_Minigame_OnRelay(u8 from, const u8 *data, u16 len)
{
    u8 slot;
    if (len < 2 || from >= MP_MAX_PLAYERS)
        return;
    switch (data[0])
    {
    case MG_INVITE:
        if (sMg.role == ROLE_NONE && !sMg.invitePending)
        {
            sMg.invitePending = TRUE;
            sMg.inviteFrom = from;
            sMg.inviteSession = data[1];
            sMg.inviteGame = len >= 3 && data[2] == GAME_CRUSH ? GAME_CRUSH : GAME_BLENDER;
            sMg.askedAt = sMg.frame;
        }
        else
        {
            u8 msg[3] = {MG_ANSWER, data[1], ANS_BUSY};
            Send(from, msg, sizeof(msg));
        }
        break;
    case MG_ANSWER:
        if (len >= 3 && sMg.role == ROLE_HOST && data[1] == sMg.session)
        {
            sMg.answer[from] = data[2];
            sMg.note[0] = 0;
            if (sMg.taskId != TASK_NONE)
                Draw();
        }
        break;
    case MG_CANCEL:
        if (data[1] != sMg.session && !(sMg.invitePending && data[1] == sMg.inviteSession))
            break;
        if (sMg.role == ROLE_HOST)
        {
            sMg.answer[from] = ANS_NO;
            if (sMg.taskId != TASK_NONE)
                Draw();
        }
        else if (sMg.invitePending && from == sMg.inviteFrom)
            sMg.invitePending = FALSE;
        else if ((sMg.role == ROLE_GUEST || sMg.role == ROLE_INVITED) && from == sMg.host)
        {
            char name[16], line[64];
            Name(from, name);
            ResetSession();
            CloseUi();
            snprintf(line, sizeof(line), "%s called it off.", name);
            ToastAscii(line);
        }
        break;
    case MG_START:
        if (sMg.role == ROLE_GUEST && from == sMg.host && data[1] == sMg.session && len >= 3)
        {
            u8 i, n = data[2];
            if (n < 2 || n > MAX_SEATS || len < 3 + n)
                break;
            sMg.n = n;
            for (i = 0; i < n; i++)
                sMg.roster[i] = data[3 + i];
            if (!InRoster(gMpLocalId, NULL))
            {
                ResetSession();
                CloseUi();
                break;
            }
            Launch();
        }
        break;
    case MG_BERRY:
        if (sMg.role == ROLE_PLAYING && data[1] == sMg.session && len >= 5 && data[2] < sMg.n)
        {
            sMg.berry[data[2]] = data[3] | (data[4] << 8);
            sMg.berryMask |= 1 << data[2];
        }
        break;
    case MG_GO:
        if (sMg.role == ROLE_PLAYING && data[1] == sMg.session && len >= 2 + MAX_SEATS * 2 && !sMg.go)
        {
            u8 i;
            for (i = 0; i < MAX_SEATS; i++)
                sMg.goItems[i] = data[2 + i * 2] | (data[3 + i * 2] << 8);
            sMg.go = TRUE;
        }
        break;
    case MG_HIT:
        if (sMg.role == ROLE_PLAYING && data[1] == sMg.session && len >= 5 && InRoster(from, &slot))
        {
            u8 next = (sMg.hitTail[slot] + 1) % HITQ;
            if (next != sMg.hitHead[slot])
            {
                sMg.hitQ[slot][sMg.hitTail[slot]] = data[3] | (data[4] << 8);
                sMg.hitTail[slot] = next;
            }
        }
        break;
    case MG_STATE:
        if (sMg.role == ROLE_PLAYING && data[1] == sMg.session && len >= 8)
        {
            sMg.stArrow = data[2] | (data[3] << 8);
            sMg.stSpeed = (s16)(data[4] | (data[5] << 8));
            sMg.stProgress = data[6] | (data[7] << 8);
            sMg.stateNew = TRUE;
        }
        break;
    case MG_SYNC:
        if (MP_Crush_Active() && data[1] == sMg.session && len >= 4 && InRoster(from, &slot))
            sCrush.seq[slot] = data[2] | (data[3] << 8);
        break;
    case MG_BLOCK:
        if (MP_Crush_Active() && data[1] == sMg.session && len >= 4 && InRoster(from, &slot))
        {
            u16 size = data[2] | (data[3] << 8);
            if (size <= len - 4)
                QueueBlock(slot, data + 4, size);
            if (getenv("MOEMON_CRUSH_LOG"))
                printf("[mp] crush: block in from slot %d size %d first %d (frame %u)\n", slot, size, data[4], (unsigned)sMg.frame);
        }
        break;
    case MG_PACKET:
        if (MP_Crush_Active() && data[1] == sMg.session && len >= 2 + RFU_PACKET_SIZE * 2 && InRoster(from, &slot))
        {
            u16 packet[RFU_PACKET_SIZE];
            memcpy(packet, data + 2, sizeof(packet));
            QueuePacket(slot, packet);
        }
        break;
    case MG_NEWS:
    {
        char text[80];
        u8 buf[80];
        u16 n = len - 1 < sizeof(text) - 1 ? len - 1 : sizeof(text) - 1;
        memcpy(text, data + 1, n);
        text[n] = 0;
        MP_AsciiToGame(buf, text, sizeof(buf) - 1);
        MP_Toast(buf);
        PlaySE(SE_M_COSMIC_POWER);
        break;
    }
    case MG_END:
        if (sMg.role == ROLE_PLAYING && data[1] == sMg.session && len >= 8 + MAX_SEATS * 3 * 2)
        {
            u8 i;
            const u8 *p = data + 2;
            sMg.endRpm = p[0] | (p[1] << 8);
            sMg.endTime = p[2] | (p[3] << 8) | (p[4] << 16) | ((u32)p[5] << 24);
            p += 6;
            for (i = 0; i < MAX_SEATS * 3; i++, p += 2)
                (&sMg.endScores[0][0])[i] = p[0] | (p[1] << 8);
            sMg.endNew = TRUE;
            printf("[mp] blender: end from %d (rpm %d)\n", from, sMg.endRpm);
        }
        break;
    }
}

// Big news for everyone online (shown as a toast)
void MP_Minigame_SendNews(const char *ascii)
{
    u8 msg[80];
    u16 n = strlen(ascii);
    if (!MP_IsConnected())
        return;
    if (n > sizeof(msg) - 2)
        n = sizeof(msg) - 2;
    msg[0] = MG_NEWS;
    memcpy(msg + 1, ascii, n);
    MP_SendRelay(0xFF, msg, n + 1);
}

void MP_Minigame_OnPlayerLeft(u8 id)
{
    if (MP_Crush_Active() && InRoster(id, NULL))
        sCrush.broken = TRUE;
    if (sMg.invitePending && sMg.inviteFrom == id)
    {
        sMg.invitePending = FALSE;
        if (sMg.role == ROLE_INVITED)
        {
            ResetSession();
            CloseUi();
        }
    }
    if (sMg.role == ROLE_HOST && sMg.taskId != TASK_NONE)
        Draw();
    if (sMg.role == ROLE_GUEST && sMg.host == id)
    {
        ResetSession();
        CloseUi();
        ToastAscii("Your friend who asked you to play left.");
    }
}

// Every frame
void MP_Minigame_Update(void)
{
    sMg.frame++;
    if (sMg.role == ROLE_PLAYING)
    {
        if (sMg.game == GAME_CRUSH)
        {
            if (!MP_IsConnected())
                sCrush.broken = TRUE;
            PumpPackets();
            if (getenv("MOEMON_CRUSH_LOG") && sMg.frame % 120 == 0)
                printf("[mp] crush: frame %u pending %d seq %d/%d/%d recv0 %04x %04x %04x\n", (unsigned)sMg.frame, sCrush.pending,
                       sCrush.mySeq, sCrush.seq[0], sCrush.seq[1], gRecvCmds[0][0], gRecvCmds[0][1], gRecvCmds[0][4]);
        }
        else
            TryGo();
        return;
    }
    if (!MP_IsConnected())
    {
        if (sMg.role != ROLE_NONE || sMg.invitePending)
        {
            sMg.invitePending = FALSE;
            ResetSession();
            CloseUi();
        }
        return;
    }
    if (sMg.invitePending && sMg.role == ROLE_NONE)
    {
        if (!gMpPlayers[sMg.inviteFrom].present)
            sMg.invitePending = FALSE;
        else if (MP_IsLocalFree() && !MpUi_IsOpen())
        {
            u8 why;
            sMg.game = sMg.inviteGame;
            why = GameCheck(sMg.game);
            sMg.invitePending = FALSE;
            if (why != ANS_YES)
            {
                char name[16], line[96];
                Answer(why);
                Name(sMg.inviteFrom, name);
                snprintf(line, sizeof(line), "%s asked you to play %s. %s", name, GameTitle(sMg.game), WhyText(why));
                ToastAscii(line);
                return;
            }
            sMg.role = ROLE_INVITED;
            sMg.host = sMg.inviteFrom;
            sMg.session = sMg.inviteSession;
            PlaySE(SE_PIN);
            OpenUi(SC_ASKED);
        }
        else if (sMg.frame - sMg.askedAt > ASK_TIMEOUT)
        {
            Answer(ANS_BUSY);
            sMg.invitePending = FALSE;
        }
    }
}
