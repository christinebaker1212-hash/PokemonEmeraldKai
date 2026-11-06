// Moemon Kai: SUPER CONTESTS with friends (Platinum's link contests, CONTEST_MODE_LINK), over the co-op relay.
//
// The CONTEST HALL's link desk runs Platinum's own script: one player chooses BECOME LEADER, the others JOIN GROUP
// (its StartBattleServer / StartBattleClient: tools/sinnoh/gen_scripts.py). The leader's game calls every friend in the
// room every half second; a friend waiting at JOIN GROUP answers with the POKéMON they entered, and the leader starts
// when the group is ready (Platinum's "When the group is ready: A Button: Proceed  B Button: Cancel", "Contest! Is
// this group OK?"). The leader then sends a seed and everyone's entrant in seat order (the leader first); every game
// builds the same contest from them (s3d_contest.c Contest_New: the computer's entrants, judges and theme drawn from
// the seed) and runs it itself. What a person does comes over the relay: the dress-up when it's done, each dance step
// as it's made (with its grade), each turn's move and judge; the games wait for one another at the end of the
// dress-up, of each part of a dance and before each turn is performed, so every game draws the contest's random
// numbers in the same order. A friend who leaves or concedes stays on the stage doing nothing (no step, the first
// usable move): no random number is drawn for them, so the games stay together. As in Platinum, a contest with
// friends gives no ribbon, record or TV segment.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "mp.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "s3d.h"
#include "s3d_contest.h"
#include "constants/flags.h"
#include "constants/songs.h"

#define C gSinnohContest

enum { SCL_GATHER = MPR_SCONTEST_FIRST, SCL_ENTRY, SCL_START, SCL_CANCEL, SCL_PLAY };
enum { PLAY_DRESS, PLAY_STEP, PLAY_PART, PLAY_MOVE, PLAY_LEAVE };
enum { ROLE_NONE, ROLE_LEAD, ROLE_JOIN, ROLE_PLAY };
#define STEPS 96

static struct
{
    u8 role;
    u32 session;
    u8 count, ids[CONTEST_ENTRANTS];   // the group's players, in seat order (the leader first)
    u8 entry[CONTEST_ENTRANTS][CONTEST_ENTRY_BYTES];
    u8 rank, type, gameDone, mySeat;
    u32 seed;
    bool8 started, cancelled;
    u16 timer;
    // the contest
    bool8 dress[CONTEST_ENTRANTS], gone[CONTEST_ENTRANTS];
    u8 parts[CONTEST_ENTRANTS];        // the dance parts each one has finished (round * 2 + part + 1)
    struct { bool8 got; u8 turn, judge; u16 move; } moves[CONTEST_ENTRANTS][2];
    struct { u8 who, round, part, at, dir, grade; } steps[STEPS];
    u8 stepCount;
} L;

static u8 sWindow = WINDOW_NONE;
static u8 sShownCount = 0xFF;

static void Put16(u8 *p, u16 v) { p[0] = v; p[1] = v >> 8; }
static void Put32(u8 *p, u32 v) { Put16(p, v); Put16(p + 2, v >> 16); }
static u16 Get16(const u8 *p) { return p[0] | (p[1] << 8); }
static u32 Get32(const u8 *p) { return Get16(p) | ((u32)Get16(p + 2) << 16); }

// ------------------------------------------------------------------------------------------------- the entrants

void Contest_PackEntrant(u8 *p, const struct ContestEntrant *e)
{
    u8 i;
    memset(p, 0, CONTEST_ENTRY_BYTES);
    Put16(p, e->species);
    Put16(p + 2, e->item);
    Put32(p + 4, e->personality);
    Put32(p + 8, e->otId);
    for (i = 0; i < MAX_MON_MOVES; i++)
        Put16(p + 12 + i * 2, e->moves[i]);
    memcpy(p + 20, e->nick, 12);
    memcpy(p + 32, e->trainer, 12);
    memcpy(p + 44, e->cond, 6);
    p[50] = e->gender;
    p[51] = e->fame;
    Put16(p + 52, e->gfx);
}

void Contest_UnpackEntrant(const u8 *p, struct ContestEntrant *e)
{
    u8 i;
    memset(e, 0, sizeof(*e));
    e->opponent = 0xFF;
    e->species = Get16(p);
    e->item = Get16(p + 2);
    e->personality = Get32(p + 4);
    e->otId = Get32(p + 8);
    for (i = 0; i < MAX_MON_MOVES; i++)
        e->moves[i] = Get16(p + 12 + i * 2);
    memcpy(e->nick, p + 20, 12);
    e->nick[11] = EOS;
    memcpy(e->trainer, p + 32, 12);
    e->trainer[11] = EOS;
    memcpy(e->cond, p + 44, 6);
    e->gender = p[50] & 1;
    e->fame = p[51];
    e->gfx = Get16(p + 52);
}

// ------------------------------------------------------------------------------------------------- the relay

static u8 Seat(u8 id)
{
    u8 i;
    for (i = 0; i < L.count; i++)
        if (L.ids[i] == id)
            return i;
    return 0xFF;
}

static void Send(u8 to, const u8 *data, u16 len)
{
    if (to < MP_MAX_PLAYERS && to != (u8)gMpLocalId && gMpPlayers[to].present)
        MP_SendRelay(to, data, len);
}

static void SendGroup(const u8 *data, u16 len)
{
    u8 i;
    for (i = 0; i < L.count; i++)
        Send(L.ids[i], data, len);
}

static void Reset(void)
{
    memset(&L, 0, sizeof(L));
}

static void SendCancel(void)
{
    u8 m[5] = {SCL_CANCEL};
    Put32(m + 1, L.session);
    if (L.role == ROLE_LEAD)
        SendGroup(m, sizeof(m));
    else if (L.count)
        Send(L.ids[0], m, sizeof(m));
}

static void SendGather(void)
{
    u8 m[16] = {SCL_GATHER}, i;
    Put32(m + 1, L.session);
    m[5] = L.rank;
    m[6] = L.type;
    m[7] = L.count;
    for (i = 0; i < L.count; i++)
        m[8 + i] = L.ids[i];
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (Seat(i) == 0xFF)
            Send(i, m, 8 + L.count);   // (friends not in the group yet: asked again)
    SendGroup(m, 8 + L.count);         // (those in it: the group as it is)
}

static void SendPlay(u8 kind, const u8 *payload, u8 len)
{
    u8 m[64] = {SCL_PLAY};
    if (!C.link || len > sizeof(m) - 7)
        return;
    Put32(m + 1, L.session);
    m[5] = L.mySeat;
    m[6] = kind;
    memcpy(m + 7, payload, len);
    SendGroup(m, 7 + len);
}

static void Gone(u8 seat)
{
    if (seat >= CONTEST_ENTRANTS || L.gone[seat] || seat == L.mySeat)
        return;
    L.gone[seat] = TRUE;
    MPNet_Log("sinnoh contest link: entrant %d left", seat);
}

static void OnPlay(u8 seat, u8 kind, const u8 *p, u16 len)
{
    if (seat >= CONTEST_ENTRANTS || seat == L.mySeat || L.ids[seat] == 0xFF)
        return;
    switch (kind)
    {
    case PLAY_DRESS:
    {
        struct ContestEntrant *e = &C.e[seat];
        u8 i, n = len >= 2 ? p[1] : 0;
        if (n > CONTEST_MAX_WORN || len < 2 + n * 3)
            break;
        e->backdrop = (s8)p[0];
        e->wornCount = n;
        for (i = 0; i < n; i++)
        {
            e->worn[i].accessory = p[2 + i * 3];
            e->worn[i].x = p[3 + i * 3];
            e->worn[i].y = p[4 + i * 3];
        }
        L.dress[seat] = TRUE;
        break;
    }
    case PLAY_STEP:
        if (len >= 5 && L.stepCount < STEPS)
        {
            L.steps[L.stepCount].who = seat;
            L.steps[L.stepCount].round = p[0];
            L.steps[L.stepCount].part = p[1];
            L.steps[L.stepCount].at = p[2];
            L.steps[L.stepCount].dir = p[3] & 3;
            L.steps[L.stepCount].grade = p[4];
            L.stepCount++;
        }
        break;
    case PLAY_PART:
        if (len >= 1 && p[0] > L.parts[seat])
            L.parts[seat] = p[0];
        break;
    case PLAY_MOVE:
        if (len >= 4)
        {
            u8 k = p[0] & 1;
            L.moves[seat][k].got = TRUE;
            L.moves[seat][k].turn = p[0];
            L.moves[seat][k].move = Get16(p + 1);
            L.moves[seat][k].judge = p[3] % 3;
        }
        break;
    case PLAY_LEAVE:
        Gone(seat);
        break;
    }
}

void Contest_LinkOnRelay(u8 from, const u8 *data, u16 len)
{
    u32 session;
    if (len < 5 || from >= MP_MAX_PLAYERS)
        return;
    session = Get32(data + 1);
    switch (data[0])
    {
    case SCL_GATHER:
        // (a friend leading: a game waiting at JOIN GROUP joins the first one it hears; in it, the group as it is)
        if (len < 8 || data[7] > CONTEST_ENTRANTS || len < 8 + data[7])
            break;
        if (L.role == ROLE_JOIN && L.count == 0)
        {
            u8 m[5 + CONTEST_ENTRY_BYTES] = {SCL_ENTRY};
            L.session = session;
            L.rank = data[5];
            L.type = data[6];
            L.count = 1;
            L.ids[0] = from;
            Put32(m + 1, session);
            memcpy(m + 5, L.entry[CONTEST_ENTRANTS - 1], CONTEST_ENTRY_BYTES);   // (its own, kept at the end)
            Send(from, m, sizeof(m));
            MPNet_Log("sinnoh contest link: joining %d's group", from);
        }
        if (L.role == ROLE_JOIN && session == L.session && L.ids[0] == from)
        {
            u8 i;
            L.count = data[7];
            for (i = 0; i < L.count; i++)
                L.ids[i] = data[8 + i];
        }
        break;
    case SCL_ENTRY:
        if (L.role != ROLE_LEAD || session != L.session || len < 5 + CONTEST_ENTRY_BYTES || Seat(from) != 0xFF
            || L.count >= CONTEST_ENTRANTS)
            break;
        L.ids[L.count] = from;
        memcpy(L.entry[L.count], data + 5, CONTEST_ENTRY_BYTES);
        L.count++;
        PlaySE(SE_SELECT);
        MPNet_Log("sinnoh contest link: %d joined (%d in the group)", from, L.count);
        break;
    case SCL_START:
    {
        u8 n, i;
        if (L.role != ROLE_JOIN || session != L.session || from != L.ids[0] || len < 16)
            break;
        n = data[12];
        if (n < 2 || n > CONTEST_ENTRANTS || len < 13 + n + n * CONTEST_ENTRY_BYTES)
            break;
        L.seed = Get32(data + 5);
        L.rank = data[9];
        L.type = data[10];
        L.gameDone = data[11];
        L.count = n;
        L.mySeat = 0xFF;
        for (i = 0; i < n; i++)
        {
            L.ids[i] = data[13 + i];
            if (L.ids[i] == (u8)gMpLocalId)
                L.mySeat = i;
            memcpy(L.entry[i], data + 13 + n + i * CONTEST_ENTRY_BYTES, CONTEST_ENTRY_BYTES);
        }
        if (L.mySeat == 0xFF)
            break;
        L.started = TRUE;
        L.role = ROLE_PLAY;
        MPNet_Log("sinnoh contest link: the group's contest begins (seat %d of %d, seed %08X)", L.mySeat, n, L.seed);
        break;
    }
    case SCL_CANCEL:
        if (session != L.session)
            break;
        if (L.role == ROLE_LEAD)
        {
            // (a friend left before the start: out of the group)
            u8 s = Seat(from), i;
            if (s == 0xFF || s == 0)
                break;
            for (i = s; i + 1 < L.count; i++)
            {
                L.ids[i] = L.ids[i + 1];
                memcpy(L.entry[i], L.entry[i + 1], CONTEST_ENTRY_BYTES);
            }
            L.count--;
        }
        else if (L.role == ROLE_JOIN && L.count && from == L.ids[0])
            L.cancelled = TRUE;
        else if (L.role == ROLE_PLAY)
            Gone(Seat(from));
        break;
    case SCL_PLAY:
        if (L.role == ROLE_PLAY && session == L.session && len >= 7 && Seat(from) == data[5])
            OnPlay(data[5], data[6], data + 7, len - 7);
        break;
    }
}

// ------------------------------------------------------------------------------------------------- the words

// The link desk's words where Platinum's speak of DS wireless: the group meets in the CO-OP room (the user, 2026-11-06:
// "it is okay to change the dialogue to match player expectations of how group contests work")
static const u8 sWelcome[] = _("Welcome to the Contest Hall!\pThis is the reception counter for\nSuper Contests with friends.\pYou and friends in your CO-OP room\ncan enter the same Super Contest.");
static const u8 sExplain[] = _("Two to four players in the same\nCO-OP room may compete together.\pFirst, choose the Contest and the\nMOéMON you wish to enter.\pThen one of you chooses BECOME\nLEADER, and the others JOIN GROUP.\pThe leader starts once everyone\nhas joined.");
static const u8 sLaunch[] = _("Your friends in this CO-OP room\nwill see your group. Is that OK?");
static const u8 sLead[] = _("Waiting for friends to join...\nA Button: Start  B Button: Cancel");
static const u8 sJoin[] = _("Waiting for a friend in this room\nto lead a group... B Button: Cancel");
static const u8 sSetup[] = _("Getting the Contest ready\nwith your friends...");
static const u8 sWaiting[] = _("Waiting for your friends...");

const u8 *Contest_LinkText(u8 which)
{
    static const u8 *const sTexts[] = {sWelcome, sExplain, sLaunch, sLead, sJoin, sSetup, sWaiting};
    return which < ARRAY_COUNT(sTexts) ? sTexts[which] : sWaiting;
}

// ------------------------------------------------------------------------------------------------- the lobby

static void Begin(u8 role)
{
    struct ContestEntrant e;
    u8 slot = VarGet(VAR_TEMP_2);   // (NewContest's slot: Platinum's VAR_MAP_LOCAL_0x02)
    Reset();
    L.role = role;
    L.rank = VarGet(VAR_0x8004) > 3 ? 3 : VarGet(VAR_0x8004);
    L.type = VarGet(VAR_0x8005) % 5;
    C.type = L.type;   // (the fame it brings is the type's)
    Contest_FillPlayerEntrant(&e, slot);
    if (role == ROLE_LEAD)
    {
        L.session = Random() | (u32)Random() << 16;
        L.count = 1;
        L.ids[0] = gMpLocalId;
        Contest_PackEntrant(L.entry[0], &e);
    }
    else
        Contest_PackEntrant(L.entry[CONTEST_ENTRANTS - 1], &e);   // (sent to a leader once one calls)
    L.timer = 0;
    MPNet_Log("sinnoh contest link: %s (rank %d type %d, slot %d)", role == ROLE_LEAD ? "leading" : "joining", L.rank,
              L.type, slot);
}

static void CloseNames(void)
{
    if (sWindow != WINDOW_NONE)
    {
        ClearStdWindowAndFrameToTransparent(sWindow, TRUE);
        RemoveWindow(sWindow);
        sWindow = WINDOW_NONE;
    }
    sShownCount = 0xFF;
}

// the group's names in a window at the top left (the members Platinum's group screen lists)
static void ShowNames(void)
{
    u8 i;
    if (sShownCount == L.count && sWindow != WINDOW_NONE)
        return;
    CloseNames();
    {
        struct WindowTemplate t = CreateWindowTemplate(0, 1, 1, 9, 2 * CONTEST_ENTRANTS, 15, 300);
        sWindow = AddWindow(&t);
    }
    if (sWindow == WINDOW_NONE)
        return;
    PutWindowTilemap(sWindow);
    DrawStdWindowFrame(sWindow, FALSE);
    FillWindowPixelBuffer(sWindow, PIXEL_FILL(1));
    for (i = 0; i < L.count; i++)
    {
        u8 id = L.ids[i];
        const u8 *name = id == (u8)gMpLocalId ? gSaveBlock2Ptr->playerName : id < MP_MAX_PLAYERS ? gMpPlayers[id].name
                                                                                                   : gText_EmptyString2;
        AddTextPrinterParameterized(sWindow, FONT_NORMAL, name, 0, 1 + i * 16, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(sWindow, COPYWIN_FULL);
    sShownCount = L.count;
}

static void Finish(u8 taskId, u16 result)
{
    CloseNames();
    gSpecialVar_Result = result;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Wait(u8 taskId)
{
    u8 i;
    if (!MP_IsConnected())
    {
        Finish(taskId, 1);
        return;
    }
    if (L.role == ROLE_LEAD)
    {
        // (friends who left the room: out of the group)
        for (i = 1; i < L.count; i++)
            if (!gMpPlayers[L.ids[i]].present)
            {
                memmove(&L.ids[i], &L.ids[i + 1], L.count - i - 1);
                memmove(L.entry[i], L.entry[i + 1], (L.count - i - 1) * CONTEST_ENTRY_BYTES);
                L.count--;
                i--;
            }
        if (L.timer++ % 30 == 0)
            SendGather();
        ShowNames();
        if (JOY_NEW(A_BUTTON) && L.count >= 2)
        {
            PlaySE(SE_SELECT);
            Finish(taskId, 0);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            Finish(taskId, 1);
        }
        return;
    }
    if (L.count)
        ShowNames();
    if (L.started)
        Finish(taskId, 0);
    else if (L.cancelled)
        Finish(taskId, 1);
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Finish(taskId, 1);
    }
}

// (special Sinnoh_ContestLinkWait, waitstate) the leader: VAR_RESULT 0 A with a group, 1 B; a friend: 0 the group's
// contest began, 1 B or the leader called it off
void Sinnoh_ContestLinkWait(void)
{
    if (L.role != ROLE_LEAD && L.role != ROLE_JOIN)
    {
        gSpecialVar_Result = 1;
        ScriptContext_Enable();
        return;
    }
    CreateTask(Task_Wait, 80);
}

static void Start(u16 resultVar)
{
    u8 m[17 + CONTEST_ENTRANTS * (1 + CONTEST_ENTRY_BYTES)] = {SCL_START}, i, n;
    for (i = 1; i < L.count; i++)
        if (!gMpPlayers[L.ids[i]].present)
        {
            memmove(&L.ids[i], &L.ids[i + 1], L.count - i - 1);
            memmove(L.entry[i], L.entry[i + 1], (L.count - i - 1) * CONTEST_ENTRY_BYTES);
            L.count--;
            i--;
        }
    if (L.count < 2)
    {
        VarSet(resultVar, 3);   // (COMM_CLUB_RET_ERROR: no friend left in the group)
        SendCancel();
        Reset();
        return;
    }
    n = L.count;
    L.seed = Random() | (u32)Random() << 16;
    if (getenv("MOEMON_TEST_CONTEST_SEED"))
        L.seed = strtoul(getenv("MOEMON_TEST_CONTEST_SEED"), NULL, 0);
    L.gameDone = FlagGet(FLAG_SYS_GAME_CLEAR);
    Put32(m + 1, L.session);
    Put32(m + 5, L.seed);
    m[9] = L.rank;
    m[10] = L.type;
    m[11] = L.gameDone;
    m[12] = n;
    for (i = 0; i < n; i++)
    {
        m[13 + i] = L.ids[i];
        memcpy(m + 13 + n + i * CONTEST_ENTRY_BYTES, L.entry[i], CONTEST_ENTRY_BYTES);
    }
    SendGroup(m, 13 + n + n * CONTEST_ENTRY_BYTES);
    L.mySeat = 0;
    L.started = TRUE;
    L.role = ROLE_PLAY;
    VarSet(resultVar, 0);
    MPNet_Log("sinnoh contest link: the group's contest begins (%d entrants, seed %08X)", n, L.seed);
}

void Contest_LinkCall(u16 call, u16 a, u16 b)
{
    switch (call)
    {
    case CC_LINK_LEAD:
        if (L.role != ROLE_LEAD)
            Begin(ROLE_LEAD);
        break;
    case CC_LINK_JOIN:
        Begin(ROLE_JOIN);
        break;
    case CC_LINK_START:
        if (L.role == ROLE_LEAD)
            Start(a);
        else
            VarSet(a, L.started ? 0 : 3);
        break;
    case CC_LINK_END:
        SendCancel();
        Reset();
        break;
    case CC_LINK_TEXT:
        StringCopy(gStringVar4, Contest_LinkText(a));
        break;
    }
}

// ------------------------------------------------------------------------------------------------- the contest

bool8 Contest_LinkStarted(void)
{
    return L.started && L.role == ROLE_PLAY;
}

// Contest_New with friends (s3d_contest.c calls it when the group's contest began): the seed, the rank and type, the
// people's entrants in their seats; the computer's in the rest, drawn by Contest_New from the seed
bool8 Contest_LinkPlan(u32 *seed, u8 *rank, u8 *type, u8 *gameDone, u8 *humans, u8 *me)
{
    u8 i;
    if (!Contest_LinkStarted())
        return FALSE;
    *seed = L.seed;
    *rank = L.rank;
    *type = L.type;
    *gameDone = L.gameDone;
    *me = L.mySeat;
    *humans = 0;
    for (i = 0; i < L.count; i++)
        *humans |= 1 << i;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        L.dress[i] = L.gone[i] = FALSE;
        L.parts[i] = 0;
        memset(L.moves[i], 0, sizeof(L.moves[i]));
    }
    for (i = L.count; i < CONTEST_ENTRANTS; i++)
        L.ids[i] = 0xFF;
    L.stepCount = 0;
    return TRUE;
}

const u8 *Contest_LinkEntry(u8 seat)
{
    return L.entry[seat % CONTEST_ENTRANTS];
}

bool8 Contest_IsHuman(u8 who)
{
    return who < CONTEST_ENTRANTS && (C.humans >> who) & 1;
}

// someone it can't wait for: gone, or no longer in the room
static bool8 Away(u8 seat)
{
    if (seat == L.mySeat)
        return FALSE;
    if (!L.gone[seat] && (L.ids[seat] >= MP_MAX_PLAYERS || !gMpPlayers[L.ids[seat]].present))
        Gone(seat);
    return L.gone[seat];
}

void Contest_LinkSendDress(void)
{
    u8 p[2 + CONTEST_MAX_WORN * 3], i;
    struct ContestEntrant *e = &C.e[C.me];
    p[0] = (u8)e->backdrop;
    p[1] = e->wornCount;
    for (i = 0; i < e->wornCount && i < CONTEST_MAX_WORN; i++)
    {
        p[2 + i * 3] = e->worn[i].accessory;
        p[3 + i * 3] = e->worn[i].x;
        p[4 + i * 3] = e->worn[i].y;
    }
    SendPlay(PLAY_DRESS, p, 2 + i * 3);
}

bool8 Contest_LinkDressReady(void)
{
    u8 i;
    if (!C.link)
        return TRUE;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (Contest_IsHuman(i) && i != C.me && !L.dress[i] && !Away(i))
            return FALSE;
    return TRUE;
}

void Contest_LinkSendStep(u8 round, u8 part, u8 at, u8 dir, u8 grade)
{
    u8 p[5] = {round, part, at, dir, grade};
    SendPlay(PLAY_STEP, p, sizeof(p));
}

// the next of the friends' steps for this round and part (others wait in the list for theirs)
bool8 Contest_LinkTakeStep(u8 *who, u8 *round, u8 *part, u8 *at, u8 *dir, u8 *grade)
{
    u8 i;
    for (i = 0; i < L.stepCount; i++)
        if (L.steps[i].round == *round && L.steps[i].part == *part)
        {
            *who = L.steps[i].who;
            *at = L.steps[i].at;
            *dir = L.steps[i].dir;
            *grade = L.steps[i].grade;
            memmove(&L.steps[i], &L.steps[i + 1], (L.stepCount - i - 1) * sizeof(L.steps[0]));
            L.stepCount--;
            return TRUE;
        }
    return FALSE;
}

void Contest_LinkSendPart(u8 round, u8 part)
{
    u8 p[1] = {round * 2 + part + 1};
    SendPlay(PLAY_PART, p, 1);
}

bool8 Contest_LinkPartReady(u8 round, u8 part)
{
    u8 i, need = round * 2 + part + 1;
    if (!C.link)
        return TRUE;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (Contest_IsHuman(i) && i != C.me && L.parts[i] < need && !Away(i))
            return FALSE;
    return TRUE;
}

void Contest_LinkSendMove(u8 turn, u16 move, u8 judge)
{
    u8 p[4] = {turn, move, move >> 8, judge};
    SendPlay(PLAY_MOVE, p, sizeof(p));
}

bool8 Contest_LinkMovesReady(u8 turn)
{
    u8 i;
    if (!C.link)
        return TRUE;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (Contest_IsHuman(i) && i != C.me && !Away(i)
            && !(L.moves[i][turn & 1].got && L.moves[i][turn & 1].turn == turn))
            return FALSE;
    return TRUE;
}

// a friend's move this turn (FALSE: gone: the caller picks for them without drawing a random number)
bool8 Contest_LinkTakeMove(u8 who, u8 turn, u16 *move, u8 *judge)
{
    if (who >= CONTEST_ENTRANTS || !L.moves[who][turn & 1].got || L.moves[who][turn & 1].turn != turn)
        return FALSE;
    *move = L.moves[who][turn & 1].move;
    *judge = L.moves[who][turn & 1].judge;
    L.moves[who][turn & 1].got = FALSE;
    return TRUE;
}

// this game is out of the group's contest (conceded, or the contest is over)
void Contest_LinkLeave(void)
{
    if (L.role == ROLE_PLAY)
    {
        u8 p[1] = {0};
        SendPlay(PLAY_LEAVE, p, 1);
    }
    Reset();
}
