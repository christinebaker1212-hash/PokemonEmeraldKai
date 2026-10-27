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
#include "event_data.h"
#include "main.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
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
};

enum
{
    MPR_FIGHT_SEAT = MPR_FIGHT_FIRST,   // u8 cabinet, seat, state (SEAT_*), mapGroup, mapNum
    MPR_FIGHT_ASK,                      // u32 session, u8 cabinet, seat (the asker's), u16 the asker's frame
    MPR_FIGHT_ANSWER,                   // u32 session, u8 yes, u32 seed, u16 the ping stamp back
    MPR_FIGHT_INPUT,                    // u32 session, u32 last frame, u8 count, u16 buttons * count, u32 ack,
                                        // u32 frame, u32 hash frame, u32 hash, u16 tick, u16 their tick back,
                                        // u16 frames since it came, s8 frames ahead
    MPR_FIGHT_END,                      // u32 session, u8 result (MF_RESULT_*)
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
#define HASH_EVERY        30      // frames between the checks that both games agree
#define HASH_KEEP         8

static const u8 sText_FightMe[] = _("Fight me!");
static const u8 sText_Fighting[] = _("FIGHT!");
static const u8 sText_YourFriend[] = _("Your friend");

static bool8 sRunning;      // a fight is the game picture; the cabinet's script waits for it
static u8 sSitAt;           // the seat we sit at: 0 west, 1 east
static u8 sSeat;            // the side we play: 0 left (P1), 1 right (the CPU fight is always the left)
static u8 sSeatState;       // SEAT_*: ours
static u16 sSeatTimer;
static u8 sResult;
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
    bool8 dropped;
    bool8 peerEnded;
    u8 peerResult;
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

static void Finish(u8 result)
{
    sResult = result;
    sRunning = FALSE;
    sNet.stage = NET_OFF;
    SetSeatState(SEAT_EMPTY);
    ScriptContext_Enable();
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
    u8 p[1 + 4 + 4 + 1 + 2 * 8 + 4 + 4 + 4 + 4 + 7];
    s32 ahead = FramesAhead();
    u32 first, f;
    u8 n = 0, *q;

    first = sNet.peerAck + 1;
    if (first + 8 <= sNet.localLast)
        first = sNet.localLast - 7;
    if (first > sNet.localLast)
        first = sNet.localLast;
    p[0] = MPR_FIGHT_INPUT;
    Put32(p + 1, sNet.session);
    Put32(p + 5, sNet.localLast);
    q = p + 10;
    for (f = first; f <= sNet.localLast; f++)
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
    q += 23;
    MP_SendRelay(sNet.peer, p, q - p);
}

static void KeepHash(u32 frame)
{
    u8 i = (frame / HASH_EVERY) % HASH_KEEP;

    sNet.hashFrame[i] = frame;
    sNet.hashVal[i] = MfEngine_Hash();
    sNet.sentHashFrame = frame;
    sNet.sentHash = sNet.hashVal[i];
}

static void CheckHash(u32 frame, u32 hash)
{
    u8 i = (frame / HASH_EVERY) % HASH_KEEP;

    if (frame == 0 || sNet.hashFrame[i] != frame)
        return;
    if (sNet.hashVal[i] != hash)
    {
        sNet.desyncs++;
        MPNet_Log("fight: desync at frame %u (%08x here, %08x there)", frame, sNet.hashVal[i], hash);
    }
    else
    {
        sNet.checks++;
    }
    sNet.hashFrame[i] = 0;   // checked
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
    strcpy(session.cabinet, "mauville_gc");
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
    sNet.dropped = FALSE;
    sNet.peerEnded = FALSE;
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
    MPNet_Log("fight: online result %d (engine %d, side %d), %u rollbacks (%u frames), %u checks agreed, %u not",
              result, engine, sNet.side, sNet.rollbacks, sNet.framesBack, sNet.checks, sNet.desyncs);
    if (sNet.peerEnded && sNet.peerResult != engine && !sNet.dropped)
        MPNet_Log("fight: the other game ended with %d", sNet.peerResult);
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
    for (f = to + 1; f <= cur; f++)
    {
        ButtonsFor(f, buttons);
        MfEngine_StepQuiet(buttons);
        if (f <= sNet.remoteLast && f % HASH_EVERY == 0)
            KeepHash(f);
    }
}

static bool8 NetStep(void)
{
    u32 cur, next, oldest = 0;
    u16 buttons[2];

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
    if (!sNet.dropped && (++sNet.heard >= DROP_FRAMES || !FriendValid(sNet.peer)))
    {
        // they're gone: no contest
        sNet.dropped = TRUE;
        MPNet_Log("fight: player %d dropped out", sNet.peer);
        MfEngine_Cancel();
    }
    if (sNet.dropped)
        return TRUE;

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

    next = cur + 1;
    if (MfEngine_Ready())
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
    if (n == 0 || n > 8 || len < 10 + 2 * n + 23)
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
}

// ---------------------------------------------------------------------------
// Online: asking and answering

static void SendAnswer(u8 to, u32 session, bool8 yes, u32 seed, u16 stamp)
{
    u8 p[12];

    p[0] = MPR_FIGHT_ANSWER;
    Put32(p + 1, session);
    p[5] = yes;
    Put32(p + 6, seed);
    p[10] = stamp;
    p[11] = stamp >> 8;
    MP_SendRelay(to, p, sizeof(p));
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
        SendAnswer(from, session, FALSE, 0, d[7] | (d[8] << 8));
        return;
    }
    sNet.peer = from;
    sNet.session = session;
    sNet.seed = ((u32)rand() << 16) ^ (u32)rand() ^ sTick;
    sNet.side = sSitAt;
    sNet.delay = 1;
    SendAnswer(from, session, TRUE, sNet.seed, d[7] | (d[8] << 8));
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
    if (!d[5])
    {
        MPNet_Log("fight: player %d can't fight now", from);
        sNet.stage = NET_OFF;
        if (!sNet.fromWait)
            Finish(FIGHT_RESULT_REFUSED);
        return;
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
    }
}

static void Ask(s8 peer, bool8 fromWait)
{
    u8 p[9];
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
    MP_SendRelay(peer, p, sizeof(p));
    MPNet_Log("fight: asking player %d", peer);
}

// ---------------------------------------------------------------------------
// The game's frame

bool8 MP_Fight_Step(void)
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
    if (sResult == FIGHT_RESULT_NO_CONTEST || sResult == FIGHT_RESULT_REFUSED)
        SetNameVar(sNet.peer);
}
