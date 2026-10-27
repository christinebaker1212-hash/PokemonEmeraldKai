// Moemon Fight: the VS cabinet. Two slot machines back to back in MAUVILLE's GAME CORNER are a
// fighting game cabinet (the engine: fight/, include/mf_engine.h). Sitting down offers WAIT FOR 2P
// or FIGHT CPU. A fight is the game picture until it ends (MP_Fight_Step, like the card screens),
// then the cabinet's script goes on with the result (docs/FIGHT_DECISIONS.md).
//
// Online: a seated player repeats where they sit (MPR_FIGHT_SEAT) and everyone shows Fight me!
// over them. A friend at the other seat is asked Fight NAME?; YES asks the seated player's game
// (MPR_FIGHT_ASK), which says yes at once (ending a CPU fight if one is on) with the session's
// seed. Both games then run the same VS session, sending their buttons every frame
// (MPR_FIGHT_INPUT: the last 8 frames, so a lost packet costs nothing). Each player's own buttons
// are used a few frames late (the input delay, from the ping), the other player's are guessed
// (their last ones) while the match is on screen and corrected by going back (rollback,
// MfEngine_RollBack) when they arrive different; the select and VS screens wait for both.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_fight.h"
#include "mp_net.h"
#include "event_data.h"
#include "main.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "mf_engine.h"
#include "platform/fight.h"

void MPNet_Log(const char *fmt, ...);

enum
{
    FIGHT_RESULT_NONE,
    FIGHT_RESULT_WON,
    FIGHT_RESULT_LOST,
    FIGHT_RESULT_DRAW,
    FIGHT_RESULT_CANCELLED,
    FIGHT_RESULT_BROKEN,
    FIGHT_RESULT_REFUSED,     // the friend couldn't fight (busy, gone, no answer)
    FIGHT_RESULT_NO_CONTEST,  // the friend dropped out of the fight
    FIGHT_RESULT_STOOD_UP,    // stopped waiting
    FIGHT_RESULT_PACKS,       // the friend's fight files aren't the same (engine, screenpack or select list)
};

enum
{
    MPR_FIGHT_SEAT = MPR_FIGHT_FIRST,   // u8 cabinet, seat, state (SEAT_*), mapGroup, mapNum
    MPR_FIGHT_ASK,                      // u32 session, u8 cabinet, seat (the asker's), u16 the asker's frame, pack
    MPR_FIGHT_ANSWER,                   // u32 session, u8 yes (2: the packs differ), u32 seed, u16 the ping stamp
                                        // back, pack, u16 length + the cabinet's rules (cabinets.ini text)
    MPR_FIGHT_INPUT,                    // u32 session, u32 last frame, u8 count (up to 12: the first frames they don't
                                        // have), u16 buttons * count, u32 ack,
                                        // u32 frame, u32 hash frame, u32 hash, u16 tick, u16 their tick back,
                                        // u16 frames since it came, s8 frames ahead
    MPR_FIGHT_END,                      // u32 session, u8 result (MF_RESULT_*)
    MPR_FIGHT_UDP,                      // u32 session, u32 + u16 our address from outside, u32 + u16 on the local
    MPR_FIGHT_RESYNC,                   // u8 RS_MSG_*, u32 session, ...: desync recovery (the host's state)
                                        // network (network order; 0: not known)
};

enum { SEAT_EMPTY, SEAT_WAITING, SEAT_CPU, SEAT_FIGHTING };
enum { NET_OFF, NET_ASKING, NET_STARTING, NET_RUNNING };

#define CABINET_ID        0       // MAUVILLE's GAME CORNER (the only one so far)
#define SEAT_RESEND       60      // frames between our seat messages
#define SEAT_TTL          150     // a friend's seat without a message for this long is empty
#define ASK_FRAMES        (60 * 6)
#define DROP_FRAMES       (60 * 10)
#define HIST              256     // frames of buttons kept (a ring)
#define MAX_AHEAD         8       // frames run on guesses past the other player's last buttons
#define SEND_MAX          12      // frames of our buttons in a message: from the first they don't have
#define HASH_EVERY        30      // frames between the checks that both games agree
#define HASH_KEEP         8
#define PUNCH_FRAMES      120     // a direct connection has this long to get through (else the relay carries on)
#define UDP_QUIET         60      // the direct connection silent this long: back to the relay
#define RESYNC_AHEAD      30      // a desync: both games stop this many frames ahead and the host's state is loaded
#define RESYNC_GIVE_UP    (60 * 20)
#define RESYNC_MAX_PAGES  2048    // more pages differ than this: no use (it goes on as it is)
#define RESYNC_HEAP_PAGES 65536
#define RESYNC_GLOBAL_PAGES 4096

static const u8 sText_FightMe[] = _("Fight me!");
static const u8 sText_Fighting[] = _("FIGHT!");
static const u8 sText_YourFriend[] = _("Your friend");

static bool8 sRunning;      // a fight is the game picture; the cabinet's script waits for it
// The fight's picture comes and goes like a CRT (docs/FIGHT_DECISIONS.md #23): the game fades to black, then the
// picture opens from a line; after the result it collapses into a line and a point, and the game fades back in
enum { CRT_NONE, CRT_ON, CRT_OFF };
#define CRT_ON_FRAMES   24
#define CRT_OFF_FRAMES  24
static bool8 sShown;        // the game faded to black for the fight (sHoldGame: still fading)
static bool8 sHoldGame;
static u8 sCrt, sCrtTime;
static u8 sPendingResult;   // the result while the CRT turns off
static u8 sSitAt;           // the seat we sit at: 0 west, 1 east
static u8 sSeat;            // the side we play: 0 left (P1), 1 right (the CPU fight is always the left)
static u8 sSeatState;       // SEAT_*: ours
static u16 sSeatTimer;
static u8 sResult;
// The fight files (include/mf_engine.h MfEngine_PackSummary): ours, read when we sit down, and the friend's;
// online, the answering player's cabinet rules
static struct MfPack sMyPack, sPeerPack;

// Desync recovery (docs/FIGHT_DECISIONS.md #12): both games stop at the same frame, the other player sends a hash
// of each page of their game's state, the host (side 0, P1) sends back the pages that differ, and both go on
enum { RS_MSG_INFO = 2, RS_MSG_HASHES, RS_MSG_PAGE, RS_MSG_END, RS_MSG_DONE };   // (the stop goes with the buttons)
enum { RS_NONE, RS_HOLD, RS_SENT };
static struct
{
    u8 stage;               // RS_*
    u32 at;                 // the frame both games stop after
    u16 timer;
    u32 askFrame;           // the other game: a desync seen at this frame (or the stop came too late), 0 none
    u32 lastAt;             // the other game: the last stop it went through (the host's packets still say it)
    // the host: the other game's state, as it comes
    struct MfStateInfo theirs;
    bool8 haveInfo;
    u32 got[MF_STATE_REGIONS];
    // the other game: pages written
    u32 pages;
    bool8 begun;
    u32 done;               // recoveries that went through
} sRs;
static u32 sRsHeap[RESYNC_HEAP_PAGES], sRsGlobals[RESYNC_GLOBAL_PAGES];
static bool8 sHaveMyPack;
static char sCabinetText[2048];
static u16 sTestButtons;    // the test harness's buttons (MP_Fight_TestPress)
static u32 sTick;

static struct
{
    u8 cabinet, seat, state, mapGroup, mapNum;
    u16 ttl;
} sFriends[MP_MAX_PLAYERS];

static struct
{
    u8 stage;               // NET_*
    s8 peer;
    u8 side;                // our side: 0 P1 (west)
    u8 delay;               // frames our buttons are used late
    u32 session;
    u32 seed;
    u16 askTimer;
    bool8 fromWait;         // asked while waiting ourselves: a no just goes on waiting
    u16 local[HIST], remote[HIST], used[HIST];  // used: the other player's buttons each frame was run with
    u32 localLast, remoteLast;                  // the newest frame with buttons
    u32 peerAck;            // the newest of our frames they have
    u32 peerFrame;          // the frame their engine is at
    u32 rollbackFrom;       // the first frame run with a wrong guess (0: none)
    u32 hashFrame[HASH_KEEP], hashVal[HASH_KEEP];
    u32 sentHashFrame, sentHash;
    u32 heard;              // frames since their last message
    u16 echoTick, echoAt;   // their last tick and when it came (sent back: the time there and back)
    u16 rtt16;              // frames there and back, x16 (smoothed)
    s8 peerAhead;           // how far ahead of us they think they are
    u32 rollbacks, framesBack, desyncs, checks;
    u16 badTimer;           // frames the bad connection icon still shows
    bool8 dropped;
    bool8 peerEnded;
    u8 peerResult;
    // the direct connection (UDP): their addresses, the one that answered, whether it's up
    u32 udpIp[2];
    u16 udpPort[2];
    u32 udpPeerIp;
    u16 udpPeerPort;
    bool8 udpOff;           // no direct connection (tests through the lag proxy, or no socket)
    bool8 udpUp, udpSentPublic;
    u16 udpTimer, udpQuiet;
} sNet;

static bool8 FriendValid(s8 id)
{
    return id >= 0 && id < MP_MAX_PLAYERS && id != gMpLocalId && gMpPlayers[id].present;
}

static bool8 SameMap(u8 mapGroup, u8 mapNum)
{
    return gSaveBlock1Ptr->location.mapGroup == mapGroup && gSaveBlock1Ptr->location.mapNum == mapNum;
}

static void Put32(u8 *p, u32 v)
{
    p[0] = v;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;
}

static u32 Get32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static void SetNameVar(s8 id)
{
    if (id >= 0 && id < MP_MAX_PLAYERS && gMpPlayers[id].present)
        StringCopyN(gStringVar1, gMpPlayers[id].name, MP_NAME_LEN);
    else
        StringCopy(gStringVar1, sText_YourFriend);
    gStringVar1[MP_NAME_LEN] = EOS;
}

void MP_Fight_TestPress(u16 buttons)
{
    sTestButtons = buttons;
}

bool8 MP_Fight_IsActive(void)
{
    return sRunning;
}

// ---------------------------------------------------------------------------
// Seats

static void SendSeat(void)
{
    u8 p[6];

    if (!MP_IsConnected())
        return;
    p[0] = MPR_FIGHT_SEAT;
    p[1] = CABINET_ID;
    p[2] = sSitAt;
    p[3] = sSeatState;
    p[4] = gSaveBlock1Ptr->location.mapGroup;
    p[5] = gSaveBlock1Ptr->location.mapNum;
    MP_SendRelay(0xFF, p, sizeof(p));
}

static void SetSeatState(u8 state)
{
    if (sSeatState != state)
    {
        sSeatState = state;
        sSeatTimer = 0;
        SendSeat();
        if (state == SEAT_EMPTY && gMpLocalId >= 0)
            MP_Emote_ClearBubbleFor(gMpLocalId);
    }
}

// The friend in a seat of our cabinet on our map (-1 nobody)
static s8 FriendInSeat(u8 seat, bool8 waitingOnly)
{
    s8 i;

    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (!FriendValid(i) || !sFriends[i].ttl || sFriends[i].cabinet != CABINET_ID || sFriends[i].seat != seat)
            continue;
        if (!SameMap(sFriends[i].mapGroup, sFriends[i].mapNum) || sFriends[i].state == SEAT_EMPTY)
            continue;
        if (waitingOnly && sFriends[i].state != SEAT_WAITING && sFriends[i].state != SEAT_CPU)
            continue;
        return i;
    }
    return -1;
}

// Every frame (mp_main.c): our seat repeated, friends' seats aging, the bubbles
void MP_Fight_Update(void)
{
    u8 i;

    sTick++;
    if (sSeatState != SEAT_EMPTY && ++sSeatTimer >= SEAT_RESEND)
    {
        sSeatTimer = 0;
        SendSeat();
    }
    if (sSeatState == SEAT_WAITING || sSeatState == SEAT_CPU)
    {
        if (gMpLocalId >= 0)
            MP_Emote_KeepBubbleFor(gMpLocalId, sText_FightMe);
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (!sFriends[i].ttl)
            continue;
        if (--sFriends[i].ttl == 0 || !FriendValid(i) || sFriends[i].state == SEAT_EMPTY)
        {
            sFriends[i].ttl = 0;
            MP_Emote_ClearBubbleFor(i);
        }
        else if (SameMap(sFriends[i].mapGroup, sFriends[i].mapNum))
        {
            MP_Emote_KeepBubbleFor(i, sFriends[i].state == SEAT_FIGHTING ? sText_Fighting : sText_FightMe);
        }
    }
}

// ---------------------------------------------------------------------------
// Results

static u8 ResultFor(int result)
{
    switch (result)
    {
    case MF_RESULT_P1:
        return sSeat == 0 ? FIGHT_RESULT_WON : FIGHT_RESULT_LOST;
    case MF_RESULT_P2:
        return sSeat == 1 ? FIGHT_RESULT_WON : FIGHT_RESULT_LOST;
    case MF_RESULT_DRAW:
        return FIGHT_RESULT_DRAW;
    case MF_RESULT_ERROR:
        return FIGHT_RESULT_BROKEN;
    default:
        return FIGHT_RESULT_CANCELLED;
    }
}

// ---------------------------------------------------------------------------
// The fight files

#define CABINET_NAME "mauville_gc"

static void ReadMyPack(bool8 again)
{
    if (sHaveMyPack && !again)
        return;
    MfEngine_PackSummary(CABINET_NAME, &sMyPack);
    sHaveMyPack = TRUE;
    MPNet_Log("fight: pack %08x, %d characters, %d stages", sMyPack.core, sMyPack.chars, sMyPack.stages);
}

// core u32, chars u8, stages u8, the hashes; returns the bytes written
static u16 PutPack(u8 *p, const struct MfPack *pack)
{
    u16 n = 6;
    u8 i;

    Put32(p, pack->core);
    p[4] = pack->chars;
    p[5] = pack->stages;
    for (i = 0; i < pack->chars; i++, n += 4)
        Put32(p + n, pack->charHash[i]);
    for (i = 0; i < pack->stages; i++, n += 4)
        Put32(p + n, pack->stageHash[i]);
    return n;
}

// Reads a pack at d (len bytes there); returns the bytes read, 0 when it doesn't fit
static u16 GetPack(const u8 *d, u16 len, struct MfPack *pack)
{
    u16 n = 6;
    u8 i;

    memset(pack, 0, sizeof(*pack));
    if (len < 6 || d[4] > MF_PACK_MAX || d[5] > MF_PACK_MAX || len < 6 + (d[4] + d[5]) * 4)
        return 0;
    pack->core = Get32(d);
    pack->chars = d[4];
    pack->stages = d[5];
    for (i = 0; i < pack->chars; i++, n += 4)
        pack->charHash[i] = Get32(d + n);
    for (i = 0; i < pack->stages; i++, n += 4)
        pack->stageHash[i] = Get32(d + n);
    return n;
}

// What one of us lacks: greyed out and never picked at random (the same lists on both sides)
static void PackOffLists(struct MfSession *session)
{
    u8 i, off = 0;

    for (i = 0; i < MF_PACK_MAX; i++)
    {
        session->charOff[i] = i >= sMyPack.chars || i >= sPeerPack.chars || sMyPack.charHash[i] != sPeerPack.charHash[i];
        session->stageOff[i] = i >= sMyPack.stages || i >= sPeerPack.stages || sMyPack.stageHash[i] != sPeerPack.stageHash[i];
        if (i < sMyPack.chars)
            off += session->charOff[i];
    }
    if (off)
        MPNet_Log("fight: %d characters greyed out (not the same as player %d's)", off, sNet.peer);
}

// Wins and losses for the achievements and the trainer card: against friends, and against the CPU
static void Tally(u8 result, bool8 online)
{
    const char *name = result == FIGHT_RESULT_WON ? (online ? "fightwin" : "fightcpu")
                     : result == FIGHT_RESULT_LOST ? (online ? "fightloss" : "fightcpuloss") : NULL;
    if (name == NULL)
        return;
    MP_Achieve_Count(name, 1);
    MPNet_Log("fight: %s now %d", name, (int)MP_Achieve_GetCount(name));
}

// A fight begins: the game fades to black for its picture
static void ShowFight(void)
{
    if (sShown)
        return;
    sShown = TRUE;
    sHoldGame = TRUE;
    sCrt = CRT_ON;
    sCrtTime = 0;
    Fight_Crt(CRT_ON, 0, CRT_ON_FRAMES);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

// Back to the GAME CORNER: the script goes on with the result
static void EndNow(u8 result)
{
    Fight_Hold(FALSE);
    Fight_Crt(CRT_NONE, 0, 1);
    Fight_Show(FALSE);
    sCrt = CRT_NONE;
    if (sShown)
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    sShown = sHoldGame = FALSE;
    sResult = result;
    sRunning = FALSE;
    ScriptContext_Enable();
}

static void Finish(u8 result)
{
    Fight_SetOverlay("", FALSE);
    sNet.stage = NET_OFF;
    SetSeatState(SEAT_EMPTY);
    if (sShown && !sHoldGame)
    {
        // the picture turns off first (MP_Fight_Step), then the game comes back
        Fight_Hold(TRUE);
        sCrt = CRT_OFF;
        sCrtTime = 0;
        sPendingResult = result;
        sRunning = TRUE;
        return;
    }
    EndNow(result);
}

// ---------------------------------------------------------------------------
// Online: the session

// How many frames ahead of them we are: our frame against theirs as it must be by now
static s32 FramesAhead(void)
{
    return (s32)MfEngine_FrameCount() - (s32)(sNet.peerFrame + sNet.rtt16 / 32);
}

static void SendInput(void)
{
    // tests: MOEMON_FIGHT_TEST_MUTE=FRAME goes quiet from that frame on (a friend whose game stalls)
    static int sMuteAt = -1;
    if (sMuteAt < 0)
        sMuteAt = getenv("MOEMON_FIGHT_TEST_MUTE") != NULL ? atoi(getenv("MOEMON_FIGHT_TEST_MUTE")) : 0;
    if (sMuteAt > 0 && MfEngine_FrameCount() >= (u32)sMuteAt)
        return;

    u8 p[1 + 4 + 4 + 1 + 2 * SEND_MAX + 4 + 4 + 4 + 4 + 7 + 8];
    s32 ahead = FramesAhead();
    u32 first, last, f;
    u8 n = 0, *q;

    // from the first frame they don't have (a message that didn't arrive is in the next one); at least the last
    first = sNet.peerAck + 1;
    if (first > sNet.localLast)
        first = sNet.localLast;
    last = first + SEND_MAX - 1;
    if (last > sNet.localLast)
        last = sNet.localLast;
    p[0] = MPR_FIGHT_INPUT;
    Put32(p + 1, sNet.session);
    Put32(p + 5, last);
    q = p + 10;
    for (f = first; f <= last; f++)
    {
        q[0] = sNet.local[f % HIST];
        q[1] = sNet.local[f % HIST] >> 8;
        q += 2;
        n++;
    }
    p[9] = n;
    Put32(q, sNet.remoteLast);
    Put32(q + 4, MfEngine_FrameCount());
    Put32(q + 8, sNet.sentHashFrame);
    Put32(q + 12, sNet.sentHash);
    q[16] = (u16)sTick;
    q[17] = (u16)sTick >> 8;
    q[18] = sNet.echoTick;
    q[19] = sNet.echoTick >> 8;
    q[20] = (u16)(sTick - sNet.echoAt);
    q[21] = (u16)(sTick - sNet.echoAt) >> 8;
    q[22] = (s8)(ahead > 100 ? 100 : ahead < -100 ? -100 : ahead);
    // desync recovery, by the fastest way, every frame: the host's frame to stop after, the other game's request
    Put32(q + 23, sNet.side == 0 && sRs.stage != RS_NONE ? sRs.at : 0);
    Put32(q + 27, sNet.side != 0 ? sRs.askFrame : 0);
    q += 31;
    if (sNet.udpUp)
    {
        u8 d[7 + sizeof(p)];
        d[0] = 'M';
        d[1] = 'F';
        d[2] = 'I';
        Put32(d + 3, sNet.session);
        memcpy(d + 7, p, q - p);
        MPUdp_SendTo(sNet.udpPeerIp, sNet.udpPeerPort, d, 7 + (q - p));
    }
    else
    {
        MP_SendRelay(sNet.peer, p, q - p);
    }
}

static void SendUdpInfo(void)
{
    u8 p[17];
    u32 ip = 0, lan = MPUdp_LocalIp();
    u16 port = 0, lanPort = MPUdp_Port();

    MPUdp_Public(&ip, &port);
    p[0] = MPR_FIGHT_UDP;
    Put32(p + 1, sNet.session);
    memcpy(p + 5, &ip, 4);
    memcpy(p + 9, &port, 2);
    memcpy(p + 11, &lan, 4);
    memcpy(p + 15, &lanPort, 2);
    MP_SendRelay(sNet.peer, p, sizeof(p));
    sNet.udpSentPublic = ip != 0;
}

static void OnUdpInfo(u8 from, const u8 *d, u16 len)
{
    if (len < 17 || from != sNet.peer || Get32(d + 1) != sNet.session)
        return;
    memcpy(&sNet.udpIp[0], d + 5, 4);
    memcpy(&sNet.udpPort[0], d + 9, 2);
    memcpy(&sNet.udpIp[1], d + 11, 4);
    memcpy(&sNet.udpPort[1], d + 15, 2);
}

static void OnInput(u8 from, const u8 *d, u16 len);

// The direct connection: its messages in (from whichever of their addresses answers), knocking on their
// addresses (both sides do: the routers then let the answers through), the relay meanwhile; back to the relay
// when it goes quiet, and back to it if it answers again
static void Knock(void)
{
    u8 p[7];
    u8 i;

    p[0] = 'M';
    p[1] = 'F';
    p[2] = 'P';
    Put32(p + 3, sNet.session);
    for (i = 0; i < 2; i++)
        MPUdp_SendTo(sNet.udpIp[i], sNet.udpPort[i], p, sizeof(p));
    if (sNet.udpPeerIp != 0)
        MPUdp_SendTo(sNet.udpPeerIp, sNet.udpPeerPort, p, sizeof(p));
}

static void PumpUdp(void)
{
    u8 buf[128];
    u32 ip;
    u16 port;
    int n;

    if (sNet.udpOff)
        return;
    while ((n = MPUdp_Recv(buf, sizeof(buf), &ip, &port)) > 0)
    {
        if (n < 7 || buf[0] != 'M' || buf[1] != 'F' || Get32(buf + 3) != sNet.session)
            continue;
        if (!sNet.udpUp)
            MPNet_Log("fight: direct connection to player %d", sNet.peer);
        sNet.udpUp = TRUE;
        sNet.udpQuiet = 0;
        sNet.udpPeerIp = ip;
        sNet.udpPeerPort = port;
        if (buf[2] == 'I')
            OnInput(sNet.peer, buf + 7, n - 7);
    }
    if (!sNet.udpSentPublic)
    {
        u32 pub;
        u16 pubPort;
        if (MPUdp_Public(&pub, &pubPort))
            SendUdpInfo();
        else if (sNet.udpTimer % 30 == 0)
            MPUdp_AskPublic();
    }
    sNet.udpTimer++;
    if (sNet.udpUp)
    {
        if (++sNet.udpQuiet >= UDP_QUIET)
        {
            MPNet_Log("fight: the direct connection went quiet: through the relay");
            sNet.udpUp = FALSE;
        }
        else if (sNet.udpTimer % 30 == 0)
        {
            Knock();    // they hear from us even when we have nothing new to send
        }
        return;
    }
    if (sNet.udpTimer == PUNCH_FRAMES && sNet.udpPeerIp == 0)
        MPNet_Log("fight: no direct connection yet: through the relay");
    if (sNet.udpTimer % (sNet.udpTimer < PUNCH_FRAMES ? 6 : 30) == 0)
        Knock();
}

static void KeepHash(u32 frame)
{
    u8 i = (frame / HASH_EVERY) % HASH_KEEP;

    sNet.hashFrame[i] = frame;
    sNet.hashVal[i] = MfEngine_Hash();
    sNet.sentHashFrame = frame;
    sNet.sentHash = sNet.hashVal[i];
}

static void RequestResync(void);

static void CheckHash(u32 frame, u32 hash)
{
    u8 i = (frame / HASH_EVERY) % HASH_KEEP;

    if (frame == 0 || sNet.hashFrame[i] != frame)
        return;
    if (sNet.hashVal[i] != hash)
    {
        sNet.desyncs++;
        MPNet_Log("fight: desync at frame %u (%08x here, %08x there)", frame, sNet.hashVal[i], hash);
        RequestResync();
    }
    else
    {
        sNet.checks++;
    }
    sNet.hashFrame[i] = 0;   // checked
}

// ---------------------------------------------------------------------------
// Online: desync recovery

#define RS_INFO_BYTES (28 + 64)
#define RS_HASH_CHUNK 1800

static void RsSend(u8 kind, const u8 *body, u16 len)
{
    static u8 p[6 + 7 + RS_HASH_CHUNK * 4 + MF_STATE_PAGE];

    p[0] = MPR_FIGHT_RESYNC;
    p[1] = kind;
    Put32(p + 2, sNet.session);
    if (len > 0)
        memcpy(p + 6, body, len);
    if (FriendValid(sNet.peer))
        MP_SendRelay(sNet.peer, p, 6 + len);
}

static void PutInfo(u8 *p, const struct MfStateInfo *info)
{
    Put32(p, (u32)info->heapBase);
    Put32(p + 4, (u32)(info->heapBase >> 32));
    Put32(p + 8, info->heapUsed);
    Put32(p + 12, info->globalsSize);
    Put32(p + 16, info->layout);
    Put32(p + 20, info->pages[MF_STATE_GLOBALS]);
    Put32(p + 24, info->pages[MF_STATE_HEAP]);
    memcpy(p + 28, info->host, 64);
}

static void GetInfo(const u8 *p, struct MfStateInfo *info)
{
    memset(info, 0, sizeof(*info));
    info->heapBase = Get32(p) | ((unsigned long long)Get32(p + 4) << 32);
    info->heapUsed = Get32(p + 8);
    info->globalsSize = Get32(p + 12);
    info->layout = Get32(p + 16);
    info->pages[MF_STATE_GLOBALS] = Get32(p + 20);
    info->pages[MF_STATE_HEAP] = Get32(p + 24);
    memcpy(info->host, p + 28, 64);
}

static void RsForgetHashes(void)
{
    memset(sNet.hashFrame, 0, sizeof(sNet.hashFrame));
}

// The host: both games stop after frame `at` (from `from`: far enough ahead for the message to get there first)
static void RsStart(u32 from)
{
    u32 at = from + RESYNC_AHEAD + sNet.rtt16 / 16 + sNet.delay;

    sRs.stage = RS_HOLD;
    sRs.at = at;
    sRs.timer = 0;
    sRs.haveInfo = FALSE;
    sRs.got[0] = sRs.got[1] = 0;
    MPNet_Log("fight: desync: both games stop after frame %u for the host's state", at);
}

static void RequestResync(void)
{
    u32 cur = MfEngine_FrameCount();

    if (sRs.stage != RS_NONE || sNet.stage != NET_RUNNING || !MfEngine_Kept(NULL))
        return;   // only during the match (its frames are kept)
    if (sNet.side == 0)
        RsStart(cur > sNet.peerFrame ? cur : sNet.peerFrame);
    else if (sRs.askFrame == 0)
        sRs.askFrame = cur;
}

// What the other game's button packets say about it
static void RsOnInput(u32 at, u32 ask)
{
    u32 cur = MfEngine_FrameCount();

    if (sNet.side == 0)
    {
        // the other game saw a desync, or our stop came too late for it
        if (ask != 0 && (sRs.stage == RS_NONE || (sRs.stage == RS_HOLD && sRs.at < ask + 2)))
            RsStart(cur > ask ? cur : ask);
        return;
    }
    if (at == 0 || at == sRs.lastAt || (sRs.stage != RS_NONE && at == sRs.at))
        return;
    if (cur > at)
    {
        sRs.askFrame = cur;     // too late here: again, from here
        return;
    }
    sRs.stage = RS_HOLD;
    sRs.at = at;
    sRs.timer = 0;
    sRs.askFrame = 0;
}

// The other game, stopped after the frame: its state's info and a hash of each page
static void RsSendState(void)
{
    static u8 body[7 + RS_HASH_CHUNK * 4];
    static u32 hashes[RS_HASH_CHUNK];
    struct MfStateInfo info;
    u32 first, i;
    int r;

    if (!MfEngine_StateInfo(&info))
        return;
    Put32(body, sRs.at);
    PutInfo(body + 4, &info);
    RsSend(RS_MSG_INFO, body, 4 + RS_INFO_BYTES);
    for (r = 0; r < MF_STATE_REGIONS; r++)
    {
        for (first = 0; first < info.pages[r]; first += RS_HASH_CHUNK)
        {
            u16 count = info.pages[r] - first < RS_HASH_CHUNK ? info.pages[r] - first : RS_HASH_CHUNK;
            MfEngine_StateHashes(r, first, count, hashes);
            body[0] = r;
            Put32(body + 1, first);
            body[5] = count;
            body[6] = count >> 8;
            for (i = 0; i < count; i++)
                Put32(body + 7 + i * 4, hashes[i]);
            RsSend(RS_MSG_HASHES, body, 7 + count * 4);
        }
    }
    sRs.stage = RS_SENT;
    sRs.begun = FALSE;
    sRs.pages = 0;
    MPNet_Log("fight: desync: state after frame %u sent to the host (%u + %u pages)", sRs.at,
              info.pages[MF_STATE_GLOBALS], info.pages[MF_STATE_HEAP]);
}

// The host, stopped after the frame with the other game's hashes: the pages that differ, then the end
static void RsServe(void)
{
    static u32 diff[RESYNC_MAX_PAGES];
    static u32 mine[RS_HASH_CHUNK];
    static u8 body[5 + MF_STATE_PAGE];
    u8 end[4 + RS_INFO_BYTES];
    struct MfStateInfo info;
    u32 count = 0, i, first;
    bool8 ok, globals;
    int r;

    if (!MfEngine_StateInfo(&info))
        return;
    ok = info.heapBase == sRs.theirs.heapBase;
    globals = ok && info.layout == sRs.theirs.layout && info.globalsSize == sRs.theirs.globalsSize;
    for (r = globals ? MF_STATE_GLOBALS : MF_STATE_HEAP; ok && r < MF_STATE_REGIONS; r++)
    {
        u32 *theirs = r == MF_STATE_HEAP ? sRsHeap : sRsGlobals;
        u32 cap = r == MF_STATE_HEAP ? RESYNC_HEAP_PAGES : RESYNC_GLOBAL_PAGES;
        for (first = 0; ok && first < info.pages[r]; first += RS_HASH_CHUNK)
        {
            u32 n = info.pages[r] - first < RS_HASH_CHUNK ? info.pages[r] - first : RS_HASH_CHUNK;
            MfEngine_StateHashes(r, first, n, mine);
            for (i = 0; i < n; i++)
            {
                u32 page = first + i;
                if (page < sRs.theirs.pages[r] && page < cap && theirs[page] == mine[i])
                    continue;
                if (count == RESYNC_MAX_PAGES)
                {
                    ok = FALSE;
                    break;
                }
                diff[count++] = page | ((u32)r << 31);
            }
        }
    }
    for (i = 0; ok && i < count; i++)
    {
        r = diff[i] >> 31;
        body[0] = r;
        Put32(body + 1, diff[i] & 0x7FFFFFFF);
        MfEngine_StatePage(r, diff[i] & 0x7FFFFFFF, body + 5);
        RsSend(RS_MSG_PAGE, body, 5 + MF_STATE_PAGE);
    }
    Put32(end, ok ? count : 0xFFFFFFFF);
    PutInfo(end + 4, &info);
    RsSend(RS_MSG_END, end, sizeof(end));
    sRs.stage = RS_SENT;
    MPNet_Log("fight: desync: %s after frame %u (%u pages%s)", ok ? "the state sent" : "no use going back",
              sRs.at, count, globals ? "" : ", the heap only: another build");
}

static void OnResync(u8 from, const u8 *d, u16 len)
{
    u8 kind;

    if (len < 6 || sNet.stage != NET_RUNNING || from != sNet.peer || Get32(d + 2) != sNet.session)
        return;
    sNet.heard = 0;
    kind = d[1];
    d += 6;
    len -= 6;
    switch (kind)
    {
    case RS_MSG_INFO:
        if (sNet.side != 0 || sRs.stage == RS_NONE || len < 4 + RS_INFO_BYTES || Get32(d) != sRs.at)
            break;
        GetInfo(d + 4, &sRs.theirs);
        sRs.haveInfo = TRUE;
        sRs.got[0] = sRs.got[1] = 0;
        break;
    case RS_MSG_HASHES:
        if (sNet.side != 0 || !sRs.haveInfo || len < 7 || d[0] >= MF_STATE_REGIONS)
        {
            break;
        }
        else
        {
            u32 *to = d[0] == MF_STATE_HEAP ? sRsHeap : sRsGlobals;
            u32 cap = d[0] == MF_STATE_HEAP ? RESYNC_HEAP_PAGES : RESYNC_GLOBAL_PAGES;
            u32 first = Get32(d + 1), i;
            u16 count = d[5] | (d[6] << 8);
            if (len < 7 + count * 4)
                break;
            for (i = 0; i < count; i++)
                if (first + i < cap)
                    to[first + i] = Get32(d + 7 + i * 4);
            sRs.got[d[0]] += count;
        }
        break;
    case RS_MSG_PAGE:
        if (sNet.side == 0 || sRs.stage != RS_SENT || len < 5 + MF_STATE_PAGE)
            break;
        if (!sRs.begun)
            sRs.begun = MfEngine_StateBegin();
        if (MfEngine_StatePut(d[0], Get32(d + 1), d + 5))
            sRs.pages++;
        break;
    case RS_MSG_END:
        if (sNet.side == 0 || sRs.stage != RS_SENT || len < 4 + RS_INFO_BYTES)
            break;
        if (Get32(d) != 0xFFFFFFFF)
        {
            struct MfStateInfo info;
            GetInfo(d + 4, &info);
            if (!sRs.begun)
                sRs.begun = MfEngine_StateBegin();
            MfEngine_StateEnd(&info);
            sRs.done++;
            sRs.lastAt = sRs.at;
            MPNet_Log("fight: desync: the host's state loaded after frame %u (%u pages)", sRs.at, sRs.pages);
        }
        RsSend(RS_MSG_DONE, NULL, 0);
        RsForgetHashes();
        sRs.stage = RS_NONE;
        break;
    case RS_MSG_DONE:
        if (sNet.side != 0 || sRs.stage != RS_SENT)
            break;
        RsForgetHashes();
        sRs.stage = RS_NONE;
        sRs.done++;
        MPNet_Log("fight: desync: both games go on from frame %u", sRs.at);
        break;
    }
}

// Every frame: stopped after the frame (its buttons all known, nothing to go back for), the state goes over
static void RsStep(u32 cur)
{
    if (sRs.stage == RS_NONE)
        return;
    if (++sRs.timer >= RESYNC_GIVE_UP)
    {
        MPNet_Log("fight: desync: no state after %d s, going on as it is", RESYNC_GIVE_UP / 60);
        sRs.stage = RS_NONE;
        RsForgetHashes();
        return;
    }
    if (sRs.stage != RS_HOLD || cur != sRs.at || sNet.remoteLast < sRs.at || sNet.rollbackFrom != 0 || !MfEngine_Ready())
        return;
    if (sNet.side != 0)
        RsSendState();
    else if (sRs.haveInfo && sRs.got[MF_STATE_GLOBALS] >= sRs.theirs.pages[MF_STATE_GLOBALS]
             && sRs.got[MF_STATE_HEAP] >= sRs.theirs.pages[MF_STATE_HEAP])
        RsServe();
}

static void ButtonsFor(u32 frame, u16 *buttons)
{
    u16 theirs = frame <= sNet.remoteLast ? sNet.remote[frame % HIST] : sNet.remote[sNet.remoteLast % HIST];

    sNet.used[frame % HIST] = theirs;
    buttons[sNet.side] = sNet.local[frame % HIST];
    buttons[sNet.side ^ 1] = theirs;
}

static void BeginOnline(void)
{
    struct MfSession session;

    memset(&session, 0, sizeof(session));
    session.mode = MF_MODE_VERSUS;
    session.seed = sNet.seed;
    session.rollback = TRUE;
    strcpy(session.cabinet, CABINET_NAME);
    PackOffLists(&session);
    memcpy(session.cabinetText, sCabinetText, sizeof(session.cabinetText));
    session.cabinetText[sizeof(session.cabinetText) - 1] = 0;
    memset(sNet.local, 0, sizeof(sNet.local));
    memset(sNet.remote, 0, sizeof(sNet.remote));
    memset(sNet.used, 0, sizeof(sNet.used));
    memset(sNet.hashFrame, 0, sizeof(sNet.hashFrame));
    sNet.localLast = sNet.remoteLast = 1;   // frame 1 runs by itself, with nothing pressed
    sNet.peerAck = sNet.peerFrame = 0;
    sNet.rollbackFrom = 0;
    sNet.sentHashFrame = sNet.sentHash = 0;
    sNet.heard = 0;
    sNet.echoTick = sNet.echoAt = 0;
    sNet.rtt16 = 0;
    sNet.peerAhead = 0;
    sNet.rollbacks = sNet.framesBack = sNet.desyncs = sNet.checks = 0;
    memset(&sRs, 0, sizeof(sRs));
    sNet.dropped = FALSE;
    sNet.peerEnded = FALSE;
    memset(sNet.udpIp, 0, sizeof(sNet.udpIp));
    memset(sNet.udpPort, 0, sizeof(sNet.udpPort));
    sNet.udpUp = sNet.udpSentPublic = FALSE;
    sNet.udpTimer = sNet.udpQuiet = 0;
    sNet.udpPeerIp = 0;
    sNet.udpPeerPort = 0;
    sNet.udpOff = getenv("MOEMON_FIGHT_NO_UDP") != NULL || MPUdp_Open() != 0;   // tests through the lag proxy
    if (!sNet.udpOff)
    {
        MPUdp_AskPublic();
        SendUdpInfo();
    }
    sSeat = sNet.side;
    SetSeatState(SEAT_FIGHTING);
    MPNet_Log("fight: online with player %d, side %d, seed %u, input delay %d", sNet.peer, sNet.side, sNet.seed,
              sNet.delay);
    if (!MfEngine_Begin(&session))
    {
        Finish(FIGHT_RESULT_BROKEN);
        return;
    }
    sNet.stage = NET_RUNNING;
    sRunning = TRUE;
    ShowFight();
}

static void EndOnline(void)
{
    u8 p[6];
    int engine = MfEngine_Result();
    u8 result = sNet.dropped ? FIGHT_RESULT_NO_CONTEST : ResultFor(engine);

    p[0] = MPR_FIGHT_END;
    Put32(p + 1, sNet.session);
    p[5] = engine;
    if (FriendValid(sNet.peer))
        MP_SendRelay(sNet.peer, p, sizeof(p));
    MPNet_Log("fight: online result %d (engine %d, side %d), %u rollbacks (%u frames), %u checks agreed, %u not, "
              "%u recovered", result, engine, sNet.side, sNet.rollbacks, sNet.framesBack, sNet.checks, sNet.desyncs,
              sRs.done);
    if (sNet.peerEnded && sNet.peerResult != engine && !sNet.dropped)
        MPNet_Log("fight: the other game ended with %d", sNet.peerResult);
    Tally(result, TRUE);
    Finish(result);
}

// Goes back to the frame before the first wrong guess and runs the frames since again
static void RollBack(void)
{
    u32 cur = MfEngine_FrameCount(), to = sNet.rollbackFrom - 1, oldest = 0, f;
    u16 buttons[2];

    sNet.rollbackFrom = 0;
    if (!MfEngine_Kept(&oldest) || to < oldest || !MfEngine_RollBack(to))
    {
        sNet.desyncs++;
        MPNet_Log("fight: couldn't go back to frame %u (at %u, oldest %u)", to, cur, oldest);
        return;
    }
    sNet.rollbacks++;
    sNet.framesBack += cur - to;
    if (cur - to >= 5)
        sNet.badTimer = 120;    // a deep correction: the connection is poor
    for (f = to + 1; f <= cur; f++)
    {
        ButtonsFor(f, buttons);
        MfEngine_StepQuiet(buttons);
        if (f <= sNet.remoteLast && f % HASH_EVERY == 0)
            KeepHash(f);
    }
}

// "WAITING FOR NAME... 9" over the fight when they've been quiet a second, counting down to no contest; the bad
// connection icon after a deep correction or while their messages are late
static void Overlay(void)
{
    char text[48];

    if (sNet.heard > 20)
        sNet.badTimer = 120;
    else if (sNet.badTimer > 0)
        sNet.badTimer--;
    text[0] = '\0';
    if (sNet.heard >= 60)
    {
        char name[MP_NAME_LEN + 1];
        if (FriendValid(sNet.peer))
            MP_GameToAscii(name, gMpPlayers[sNet.peer].name, sizeof(name));
        else
            snprintf(name, sizeof(name), "your friend");
        snprintf(text, sizeof(text), "Waiting for %s... %u", name, (DROP_FRAMES - sNet.heard) / 60 + 1);
    }
    Fight_SetOverlay(text, sNet.badTimer > 0);
}

// Tests (MOEMON_FIGHT_TEST_DESYNC=N): N frames into the match, player 1's life changes in this game alone
static void TestDesync(u32 cur)
{
    static u32 sPokeAt;
    const char *env = getenv("MOEMON_FIGHT_TEST_DESYNC");

    if (env == NULL || cur < 2)
    {
        sPokeAt = 0;
        return;
    }
    if (sPokeAt == 0 && MfEngine_Kept(NULL))
        sPokeAt = cur + atoi(env);
    if (sPokeAt != 0 && cur == sPokeAt && MfEngine_Ready())
    {
        MfEngine_TestPoke(-7);
        MPNet_Log("fight: test: player 1's life changed here alone at frame %u", cur);
        sPokeAt = 0xFFFFFFFF;
    }
}

static bool8 NetStep(void)
{
    u32 cur, next, oldest = 0;
    u16 buttons[2];
    static u32 sLastMs;
    u32 ms = Fight_Ms();

    if (sLastMs != 0 && ms - sLastMs > 250)
        MPNet_Log("fight: %u ms since the last frame (frame %u, ready %d)", ms - sLastMs, MfEngine_FrameCount(),
                  MfEngine_Ready());
    sLastMs = ms;

    if (sNet.stage == NET_STARTING)
    {
        // our CPU fight is ending (MfEngine_Cancel): then the friend's fight
        if (!MfEngine_IsActive())
            BeginOnline();
        return TRUE;
    }
    if (!MfEngine_IsActive())
    {
        EndOnline();
        return FALSE;
    }
    if (sNet.heard == 120)
        MPNet_Log("fight: nothing from player %d for 2 s (frame %u, theirs to %u, ours to %u, ready %d, udp %d, "
                  "%d bytes queued)", sNet.peer, MfEngine_FrameCount(), sNet.remoteLast, sNet.localLast,
                  MfEngine_Ready(), sNet.udpUp, MPNet_ClientQueued());
    if (!sNet.dropped && (++sNet.heard >= DROP_FRAMES || !FriendValid(sNet.peer)))
    {
        // they're gone: no contest
        sNet.dropped = TRUE;
        MPNet_Log("fight: player %d dropped out", sNet.peer);
        MfEngine_Cancel();
    }
    if (sNet.dropped)
        return TRUE;
    Overlay();

    PumpUdp();
    cur = MfEngine_FrameCount();
    if (sTick % 600 == 0)
        MPNet_Log("fight: frame %u, theirs to %u (their frame %u), ours to %u, %u rollbacks, %u frames there and back,"
                  " %d ahead (they're %d)", cur, sNet.remoteLast, sNet.peerFrame, sNet.localLast, sNet.rollbacks,
                  sNet.rtt16 / 16, FramesAhead(), sNet.peerAhead);
    // the input delay follows the ping on the select screen (the frames a message takes, 1 to 3); it only grows, as
    // buttons already sent can't be taken back
    if (sNet.rtt16 != 0 && !MfEngine_Kept(NULL))
    {
        u8 want = (sNet.rtt16 / 16 + 1) / 2;
        want = want < 1 ? 1 : want > 3 ? 3 : want;
        if (want > sNet.delay)
        {
            MPNet_Log("fight: input delay %d (%u frames there and back)", want, sNet.rtt16 / 16);
            sNet.delay = want;
        }
    }
    // our buttons for the frame `delay` frames ahead
    while (sNet.localLast < cur + 1 + sNet.delay)
    {
        sNet.localLast++;
        sNet.local[sNet.localLast % HIST] = sNet.localLast <= 1 + sNet.delay ? 0 : (Fight_PollButtons() | sTestButtons);
    }
    if (sNet.rollbackFrom != 0 && sNet.rollbackFrom <= cur)
        RollBack();
    else
        sNet.rollbackFrom = 0;

    TestDesync(cur);
    RsStep(cur);
    next = cur + 1;
    if (MfEngine_Ready() && (sRs.stage == RS_NONE || next <= sRs.at))
    {
        bool8 known = next <= sNet.remoteLast;
        bool8 guess = !known && next - sNet.remoteLast <= MAX_AHEAD && MfEngine_Kept(&oldest)
                      && oldest <= sNet.remoteLast;
        // further ahead of them than they are of us: wait a frame now and then so they catch up
        bool8 ahead = FramesAhead() - sNet.peerAhead >= 2 && sTick % 4 == 0;
        if ((known || guess) && !ahead)
        {
            ButtonsFor(next, buttons);
            if (MfEngine_Step(buttons) && known && next % HASH_EVERY == 0)
                KeepHash(next);
        }
    }
    SendInput();
    return TRUE;
}

static void OnInput(u8 from, const u8 *d, u16 len)
{
    u32 last, first, f, simulated;
    u8 n, i;
    const u8 *q;

    if (len < 10 || sNet.stage != NET_RUNNING || from != sNet.peer || Get32(d + 1) != sNet.session)
        return;
    last = Get32(d + 5);
    n = d[9];
    if (n == 0 || n > SEND_MAX || len < 10 + 2 * n + 31)
        return;
    sNet.heard = 0;
    first = last - n + 1;
    simulated = MfEngine_FrameCount();
    for (i = 0, f = first; i < n; i++, f++)
    {
        u16 b = d[10 + 2 * i] | (d[11 + 2 * i] << 8);
        if (f != sNet.remoteLast + 1)
            continue;
        sNet.remote[f % HIST] = b;
        sNet.remoteLast = f;
        if (f <= simulated && sNet.used[f % HIST] != b && (sNet.rollbackFrom == 0 || f < sNet.rollbackFrom))
            sNet.rollbackFrom = f;
    }
    q = d + 10 + 2 * n;
    if (Get32(q) > sNet.peerAck)
        sNet.peerAck = Get32(q);
    sNet.peerFrame = Get32(q + 4);
    CheckHash(Get32(q + 8), Get32(q + 12));
    // the time there and back: now, less our tick they sent back and how long they held it
    {
        u16 back = q[18] | (q[19] << 8), held = q[20] | (q[21] << 8);
        u16 rtt = (u16)((u16)sTick - back - held);
        if (back != 0 && rtt < 600)
            sNet.rtt16 = sNet.rtt16 ? (sNet.rtt16 * 7 + rtt * 16) / 8 : rtt * 16;
    }
    sNet.echoTick = q[16] | (q[17] << 8);
    sNet.echoAt = sTick;
    sNet.peerAhead = (s8)q[22];
    RsOnInput(Get32(q + 23), Get32(q + 27));
}

// ---------------------------------------------------------------------------
// Online: asking and answering

// yes: 0 no, 1 yes (with our pack and the cabinet's rules), 2 the packs differ
static void SendAnswer(u8 to, u32 session, u8 yes, u32 seed, u16 stamp)
{
    static u8 p[12 + 6 + MF_PACK_MAX * 8 + 2 + sizeof(sCabinetText)];
    u16 n = 12, text;

    p[0] = MPR_FIGHT_ANSWER;
    Put32(p + 1, session);
    p[5] = yes;
    Put32(p + 6, seed);
    p[10] = stamp;
    p[11] = stamp >> 8;
    if (yes == 1)
    {
        n += PutPack(p + n, &sMyPack);
        text = strlen(sCabinetText);
        p[n] = text;
        p[n + 1] = text >> 8;
        memcpy(p + n + 2, sCabinetText, text);
        n += 2 + text;
    }
    MP_SendRelay(to, p, n);
}

static void OnAsk(u8 from, const u8 *d, u16 len)
{
    u32 session;
    u8 seat;

    if (len < 9)
        return;
    session = Get32(d + 1);
    seat = d[6];
    if (d[5] != CABINET_ID || seat == sSitAt || (sSeatState != SEAT_WAITING && sSeatState != SEAT_CPU)
        || sNet.stage != NET_OFF)
    {
        SendAnswer(from, session, 0, 0, d[7] | (d[8] << 8));
        return;
    }
    ReadMyPack(FALSE);
    if (!GetPack(d + 9, len - 9, &sPeerPack) || sPeerPack.core != sMyPack.core)
    {
        MPNet_Log("fight: player %d's fight files aren't ours (%08x, ours %08x)", from, sPeerPack.core, sMyPack.core);
        SendAnswer(from, session, 2, 0, d[7] | (d[8] << 8));
        return;
    }
    // our rules for both of us
    if (!MfEngine_CabinetText(CABINET_NAME, sCabinetText, sizeof(sCabinetText)))
        sCabinetText[0] = 0;
    sNet.peer = from;
    sNet.session = session;
    sNet.seed = ((u32)rand() << 16) ^ (u32)rand() ^ sTick;
    sNet.side = sSitAt;
    sNet.delay = 1;
    SendAnswer(from, session, 1, sNet.seed, d[7] | (d[8] << 8));
    PlaySE(SE_SUCCESS);
    if (sSeatState == SEAT_CPU && MfEngine_IsActive())
    {
        // a friend sat down during our CPU fight: it ends, theirs begins
        MPNet_Log("fight: player %d joins, the CPU fight ends", from);
        MfEngine_Cancel();
        sNet.stage = NET_STARTING;
        sRunning = TRUE;
    }
    else
    {
        BeginOnline();
    }
}

static void OnAnswer(u8 from, const u8 *d, u16 len)
{
    u16 rtt;

    if (len < 12 || sNet.stage != NET_ASKING || from != sNet.peer || Get32(d + 1) != sNet.session)
        return;
    if (d[5] != 1)
    {
        MPNet_Log("fight: player %d can't fight now%s", from, d[5] == 2 ? " (their fight files aren't ours)" : "");
        sNet.stage = NET_OFF;
        if (!sNet.fromWait)
            Finish(d[5] == 2 ? FIGHT_RESULT_PACKS : FIGHT_RESULT_REFUSED);
        return;
    }
    {
        u16 n = 12, got = GetPack(d + 12, len - 12, &sPeerPack), text;
        n += got;
        text = got && len >= n + 2 ? d[n] | (d[n + 1] << 8) : 0;
        if (!got || len < n + 2 + text || text >= sizeof(sCabinetText))
        {
            MPNet_Log("fight: player %d's answer is cut short", from);
            sNet.stage = NET_OFF;
            if (!sNet.fromWait)
                Finish(FIGHT_RESULT_REFUSED);
            return;
        }
        memcpy(sCabinetText, d + n + 2, text);
        sCabinetText[text] = 0;
    }
    // the input delay starts at 1 frame; the ping raises it (NetStep)
    rtt = (u16)((u16)sTick - (d[10] | (d[11] << 8)));
    sNet.delay = 1;
    sNet.seed = Get32(d + 6);
    sNet.side = sSitAt;
    MPNet_Log("fight: player %d said yes (%u frames there and back)", from, rtt);
    BeginOnline();
}

static void OnEnd(u8 from, const u8 *d, u16 len)
{
    if (len < 6 || from != sNet.peer || Get32(d + 1) != sNet.session)
        return;
    sNet.peerEnded = TRUE;
    sNet.peerResult = d[5];
    sNet.heard = 0;
}

void MP_Fight_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || len < 1)
        return;
    switch (data[0])
    {
    case MPR_FIGHT_SEAT:
        if (len >= 6)
        {
            sFriends[from].cabinet = data[1];
            sFriends[from].seat = data[2];
            sFriends[from].state = data[3];
            sFriends[from].mapGroup = data[4];
            sFriends[from].mapNum = data[5];
            sFriends[from].ttl = data[3] == SEAT_EMPTY ? 1 : SEAT_TTL;
        }
        break;
    case MPR_FIGHT_ASK:
        OnAsk(from, data, len);
        break;
    case MPR_FIGHT_ANSWER:
        OnAnswer(from, data, len);
        break;
    case MPR_FIGHT_INPUT:
        OnInput(from, data, len);
        break;
    case MPR_FIGHT_END:
        OnEnd(from, data, len);
        break;
    case MPR_FIGHT_UDP:
        OnUdpInfo(from, data, len);
        break;
    case MPR_FIGHT_RESYNC:
        OnResync(from, data, len);
        break;
    }
}

static void Ask(s8 peer, bool8 fromWait)
{
    static u8 p[9 + 6 + MF_PACK_MAX * 8];
    u16 stamp = sTick;

    sNet.peer = peer;
    sNet.fromWait = fromWait;
    sNet.session = ((u32)rand() << 16) ^ (u32)rand() ^ (sTick << 3);
    sNet.stage = NET_ASKING;
    sNet.askTimer = 0;
    sNet.peerEnded = FALSE;
    p[0] = MPR_FIGHT_ASK;
    Put32(p + 1, sNet.session);
    p[5] = CABINET_ID;
    p[6] = sSitAt;
    p[7] = stamp;
    p[8] = stamp >> 8;
    ReadMyPack(FALSE);
    MP_SendRelay(peer, p, 9 + PutPack(p + 9, &sMyPack));
    MPNet_Log("fight: asking player %d", peer);
}

// ---------------------------------------------------------------------------
// The game's frame

static bool8 Step(void);

bool8 MP_Fight_Step(void)
{
    bool8 on;

    if (sCrt == CRT_OFF)
    {
        Fight_Crt(CRT_OFF, sCrtTime, CRT_OFF_FRAMES);
        if (++sCrtTime > CRT_OFF_FRAMES)
        {
            EndNow(sPendingResult);
            return FALSE;
        }
        return TRUE;
    }
    on = Step();
    if (!on || sCrt == CRT_OFF)
        return on;
    if (sHoldGame)
    {
        // the game's picture fades to black first (the engine runs meanwhile)
        if (gPaletteFade.active)
            return FALSE;
        sHoldGame = FALSE;
        Fight_Show(TRUE);
    }
    if (sCrt == CRT_ON && MfEngine_Frame() != NULL)
    {
        Fight_Crt(CRT_ON, sCrtTime, CRT_ON_FRAMES);
        if (++sCrtTime > CRT_ON_FRAMES)
        {
            sCrt = CRT_NONE;
            Fight_Crt(CRT_NONE, 0, 1);
        }
    }
    return TRUE;
}

static bool8 Step(void)
{
    unsigned short buttons[2] = {0, 0};

    if (sNet.stage == NET_RUNNING || sNet.stage == NET_STARTING)
        return NetStep();
    if (!sRunning)
        return FALSE;
    if (!MfEngine_IsActive())
    {
        // the fight is over: back to the GAME CORNER, the script goes on
        u8 result = ResultFor(MfEngine_Result());
        MPNet_Log("fight: result %d (engine %d, seat %d)", result, MfEngine_Result(), sSeat);
        if (result == FIGHT_RESULT_BROKEN)
            MPNet_Log("fight: %s", MfEngine_Error());
        Tally(result, FALSE);
        Finish(result);
        return FALSE;
    }
    buttons[sSeat] = Fight_PollButtons() | sTestButtons;
    MfEngine_Step(buttons);
    return TRUE;
}

// ---------------------------------------------------------------------------
// Specials

// VAR_0x8004: the seat. VAR_RESULT: 0 nobody else here, 1 a friend waits at the other seat (STR_VAR_1),
// 2 a friend is in this seat, 3 friends are fighting here
void MP_Fight_SeatCheck(void)
{
    u8 seat = gSpecialVar_0x8004 & 1;
    s8 other, here;

    sSitAt = seat;
    gSpecialVar_Result = 0;
    if (!MP_IsConnected())
        return;
    ReadMyPack(TRUE);   // the files are read again each time someone sits down
    here = FriendInSeat(seat, FALSE);
    other = FriendInSeat(seat ^ 1, FALSE);
    if (here >= 0)
    {
        SetNameVar(here);
        gSpecialVar_Result = sFriends[here].state == SEAT_FIGHTING ? 3 : 2;
    }
    else if (other >= 0)
    {
        SetNameVar(other);
        gSpecialVar_Result = sFriends[other].state == SEAT_FIGHTING ? 3 : 1;
    }
}

void MP_Fight_StartCpu(void)
{
    struct MfSession session;

    memset(&session, 0, sizeof(session));
    // against the CPU the player is always on the left (the engine's arcade mode)
    sSeat = 0;
    session.mode = MF_MODE_CPU;
    session.seed = (unsigned int)rand();
    strcpy(session.cabinet, "mauville_gc");
    sResult = FIGHT_RESULT_NONE;
    if (!MfEngine_Begin(&session))
    {
        sResult = FIGHT_RESULT_BROKEN;
        ScriptContext_Enable();
        return;
    }
    // friends see us at our seat with Fight me!: sitting down at the other seat ends the CPU fight and
    // starts theirs
    sSitAt = gSpecialVar_0x8004 & 1;
    SetSeatState(SEAT_CPU);
    sRunning = TRUE;
    ShowFight();
}

#define tTimer data[0]

static void Task_Wait(u8 taskId)
{
    s8 other;

    if (sNet.stage == NET_RUNNING || sNet.stage == NET_STARTING)
    {
        DestroyTask(taskId);   // a friend sat down: the fight is the picture now
        return;
    }
    if (sNet.stage == NET_ASKING)
    {
        if (++sNet.askTimer >= ASK_FRAMES || !FriendValid(sNet.peer))
            sNet.stage = NET_OFF;
        if (sNet.stage == NET_ASKING)
            return;
    }
    // two friends waiting at both seats: the one who joined the room first asks
    other = FriendInSeat(sSitAt ^ 1, TRUE);
    if (other >= 0 && sFriends[other].state == SEAT_WAITING && gMpLocalId >= 0 && gMpLocalId < other
        && ++gTasks[taskId].tTimer >= 30)
    {
        gTasks[taskId].tTimer = 0;
        Ask(other, TRUE);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sResult = FIGHT_RESULT_STOOD_UP;
        SetSeatState(SEAT_EMPTY);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

// VAR_0x8004: the seat. Waits for a friend at the other seat (B stands up)
void MP_Fight_Wait(void)
{
    sSitAt = gSpecialVar_0x8004 & 1;
    sResult = FIGHT_RESULT_NONE;
    sNet.stage = NET_OFF;
    SetSeatState(SEAT_WAITING);
    CreateTask(Task_Wait, 80);
}

static void Task_Challenge(u8 taskId)
{
    if (sNet.stage == NET_RUNNING)
    {
        DestroyTask(taskId);
        return;
    }
    if (sNet.stage != NET_ASKING)
    {
        DestroyTask(taskId);   // refused (Finish went on with the script)
        return;
    }
    if (++sNet.askTimer >= ASK_FRAMES || !FriendValid(sNet.peer))
    {
        MPNet_Log("fight: no answer from player %d", sNet.peer);
        sNet.stage = NET_OFF;
        DestroyTask(taskId);
        Finish(FIGHT_RESULT_REFUSED);
    }
}

// VAR_0x8004: the seat. Asks the friend at the other seat (MP_Fight_SeatCheck said 1) to fight
void MP_Fight_Challenge(void)
{
    s8 other;

    sSitAt = gSpecialVar_0x8004 & 1;
    sResult = FIGHT_RESULT_NONE;
    other = FriendInSeat(sSitAt ^ 1, TRUE);
    if (other < 0)
    {
        sResult = FIGHT_RESULT_REFUSED;
        ScriptContext_Enable();
        return;
    }
    Ask(other, FALSE);
    CreateTask(Task_Challenge, 80);
}

void MP_Fight_Result(void)
{
    gSpecialVar_Result = sResult;
    if (sResult == FIGHT_RESULT_NO_CONTEST || sResult == FIGHT_RESULT_REFUSED || sResult == FIGHT_RESULT_PACKS)
        SetNameVar(sNet.peer);
}
