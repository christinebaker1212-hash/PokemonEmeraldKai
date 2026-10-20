// Moemon Co-op: CO-OP menu > TOURNAMENT. A quick battle tournament
// between everyone online, using the PvP link battles (friendly: no money
// changes hands and nobody blacks out).
//
// It's a round robin: everyone battles everyone once, a few matches at a
// time. With an odd number of players someone has a break each round, but
// everyone plays the same number of matches, so nobody's left out. The
// player who starts it keeps score. The champion gets the best prize,
// second place a MoeGatcha pull, and everyone else a consolation prize.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "item.h"
#include "pokemon.h"
#include "main.h"
#include "money.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/songs.h"

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_Gatcha_AddPulls(s32 n);
void MP_Events_Consolation(const char *why);

enum {
    MPR_T_INVITE = MPR_TOURNEY_FIRST,   // u8 tid
    MPR_T_ANSWER,                       // u8 tid, u8 yes
    MPR_T_BEGIN,                        // u8 tid, u8 n, ids[n]
    MPR_T_READY,                        // u8 tid, u8 round
    MPR_T_FORFEIT,                      // u8 tid, u8 round
    MPR_T_RESULT,                       // u8 tid, u8 round, u8 opponent, u8 result (1 won, 0 lost, 2 draw)
    MPR_T_ROUND,                        // u8 tid, u8 round, u8 n, wins[n]
    MPR_T_FINAL,                        // u8 tid, u8 n, (id, wins)[n] best first
    MPR_T_CANCEL,                       // u8 tid
};

#define MAX_P   MP_MAX_PLAYERS
#define BYE     0xFF

enum { TS_NONE, TS_GATHERING, TS_INVITED, TS_RUNNING, TS_DONE };
enum { ANS_WAIT, ANS_YES, ANS_NO };
enum { ME_IDLE, ME_PROMPT, ME_WAITING, ME_BATTLING, ME_REPORTED };

bool8 gMpFriendlyBattle;   // read by mp_battle.c: no money, no blacking out

static EWRAM_DATA struct
{
    u8 state;
    u8 tid;               // tournament id, so old messages are ignored
    u8 organizer;
    u8 n;
    u8 ids[MAX_P];        // participants
    u8 answers[MAX_P];    // per player id while gathering
    u8 round, rounds;
    u8 wins[MAX_P];       // per participant index
    s8 results[MAX_P][MAX_P];   // organizer: [i][j] = 1 if ids[i] beat ids[j], -1 unknown, 0 lost, 2 draw
    u8 me;                // ME_*
    u8 myOpponent;
    bool8 iReady, theyReady;
    u8 taskId;
    u16 cursor;
    bool8 dirty;
    char note[72];
    bool8 finalShown;
    u8 finalIds[MAX_P], finalWins[MAX_P];
} sT = {0};

static bool8 sInit;

static void Task_Tourney(u8 taskId);

static void InitIfNeeded(void)
{
    if (!sInit)
    {
        memset(&sT, 0, sizeof(sT));
        sT.taskId = TASK_NONE;
        sInit = TRUE;
    }
}

static void NameOf(u8 id, char *out)
{
    if (id == gMpLocalId)
        MP_GameToAscii(out, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    else if (id < MP_MAX_PLAYERS)
        MP_GameToAscii(out, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
    else
        snprintf(out, 8, "?");
}

static bool8 Present(u8 id)
{
    return id == gMpLocalId || (id < MP_MAX_PLAYERS && gMpPlayers[id].present);
}

static void SendTo(u8 to, const u8 *data, u16 len)
{
    if (to == gMpLocalId)
        return;
    MP_SendRelay(to, data, len);
}

static void SendAll(const u8 *data, u16 len)
{
    MP_SendRelay(0xFF, data, len);
}

static s8 IndexOf(u8 id)
{
    u8 i;
    for (i = 0; i < sT.n; i++)
        if (sT.ids[i] == id)
            return i;
    return -1;
}

// Round robin by the circle method. Returns the opponent's id or BYE.
static u8 OpponentIn(u8 round, u8 id)
{
    u8 m = sT.n + (sT.n & 1), slots[MAX_P + 1], i, pos = 0xFF;
    s8 idx = IndexOf(id);
    if (idx < 0)
        return BYE;
    // slot 0 stays, the rest rotate by round
    slots[0] = 0;
    for (i = 1; i < m; i++)
        slots[i] = 1 + ((i - 1 + round) % (m - 1));
    for (i = 0; i < m; i++)
        if (slots[i] == idx)
            pos = i;
    if (pos == 0xFF)
        return BYE;
    {
        u8 other = slots[m - 1 - pos];
        return other < sT.n ? sT.ids[other] : BYE;
    }
}

// ---------------------------------------------------------------------------
// Drawing

// Ready check: how the local team looks ("4 can battle, 72% HP")
static u8 TeamStatus(char *out, int size)
{
    u8 i, able = 0, fainted = 0;
    u32 hp = 0, maxHp = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        hp += GetMonData(mon, MON_DATA_HP);
        maxHp += GetMonData(mon, MON_DATA_MAX_HP);
        if (GetMonData(mon, MON_DATA_HP) == 0)
            fainted++;
        else
            able++;
    }
    if (able == 0)
        snprintf(out, size, "Your team can't battle! Heal first.");
    else if (fainted || (maxHp && hp * 2 < maxHp))
        snprintf(out, size, "Team: %d can battle, %d%% HP. Heal first?", able, maxHp ? (int)(hp * 100 / maxHp) : 0);
    else
        snprintf(out, size, "Team: %d can battle, %d%% HP. Looking good!", able, maxHp ? (int)(hp * 100 / maxHp) : 0);
    return able;
}

static bool8 HasAbleMon(void)
{
    char tmp[64];
    return TeamStatus(tmp, sizeof(tmp)) > 0;
}

static void Draw(void)
{
    char line[80], name[16], team[64];
    u8 i;
    sT.dirty = FALSE;
    if (!MpUi_IsOpen())
        return;
    switch (sT.state)
    {
    case TS_GATHERING:
        MpUi_Begin("TOURNAMENT: WHO'S PLAYING?");
        for (i = 0; i < MP_MAX_PLAYERS && i < MPUI_ROWS - 2; i++)
        {
            const char *st;
            if (!Present(i))
                continue;
            NameOf(i, name);
            st = (i == gMpLocalId) ? "you" : sT.answers[i] == ANS_YES ? "joined!" : sT.answers[i] == ANS_NO ? "no thanks" : "asked...";
            MpUi_RowAscii(i, FALSE, name, st);
        }
        MpUi_RowAscii(MPUI_ROWS - 2, sT.cursor == 0, "BEGIN!", NULL);
        MpUi_RowAscii(MPUI_ROWS - 1, sT.cursor == 1, "CALL IT OFF", NULL);
        MpUi_End();
        MpUi_HelpAscii(sT.note[0] ? sT.note : "Everyone battles everyone once.  {A} SELECT");
        break;
    case TS_INVITED:
        NameOf(sT.organizer, name);
        MpUi_Begin("BATTLE TOURNAMENT");
        snprintf(line, sizeof(line), "%s is starting a battle tournament!", name);
        MpUi_TextAscii(0, line);
        MpUi_TextAscii(1, "Friendly battles: no money lost, no blacking out.");
        TeamStatus(team, sizeof(team));
        MpUi_TextAscii(2, team);
        MpUi_RowAscii(3, sT.cursor == 0, "JOIN", NULL);
        MpUi_RowAscii(4, sT.cursor == 1, "NO THANKS", NULL);
        MpUi_End();
        MpUi_HelpAscii("{A} SELECT");
        break;
    case TS_RUNNING:
    case TS_DONE:
    {
        u8 order[MAX_P], j;
        for (i = 0; i < sT.n; i++)
            order[i] = i;
        for (i = 0; i < sT.n; i++)
            for (j = i + 1; j < sT.n; j++)
                if (sT.wins[order[j]] > sT.wins[order[i]])
                {
                    u8 t = order[i];
                    order[i] = order[j];
                    order[j] = t;
                }
        if (sT.state == TS_DONE)
            snprintf(line, sizeof(line), "TOURNAMENT: FINAL STANDINGS");
        else
            snprintf(line, sizeof(line), "TOURNAMENT: ROUND %d OF %d", sT.round + 1, sT.rounds);
        MpUi_Begin(line);
        for (i = 0; i < sT.n && i < MPUI_ROWS - 2; i++)
        {
            char l[32], r[16];
            NameOf(sT.ids[order[i]], name);
            snprintf(l, sizeof(l), "%d. %s", i + 1, name);
            snprintf(r, sizeof(r), "%d win%s", sT.wins[order[i]], sT.wins[order[i]] == 1 ? "" : "s");
            MpUi_RowAscii(i, FALSE, l, r);
        }
        if (sT.state == TS_RUNNING && sT.me == ME_PROMPT)
        {
            NameOf(sT.myOpponent, name);
            snprintf(line, sizeof(line), "Your match: vs %s", name);
            MpUi_RowAscii(MPUI_ROWS - 2, sT.cursor == 0, line, "READY");
            MpUi_RowAscii(MPUI_ROWS - 1, sT.cursor == 1, "FORFEIT THIS MATCH", NULL);
        }
        else if (sT.state == TS_RUNNING && sT.me == ME_WAITING)
        {
            NameOf(sT.myOpponent, name);
            snprintf(line, sizeof(line), "Waiting for %s to be ready...", name);
            MpUi_TextAscii(MPUI_ROWS - 1, line);
        }
        else if (sT.state == TS_RUNNING)
            MpUi_TextAscii(MPUI_ROWS - 1, OpponentIn(sT.round, gMpLocalId) == BYE ? "You have a break this round." : "Waiting for the other matches...");
        MpUi_End();
        TeamStatus(team, sizeof(team));
        MpUi_HelpAscii(sT.note[0] ? sT.note : (sT.me == ME_PROMPT ? team : "{B} CLOSE (the tournament goes on)"));
        break;
    }
    }
}

static void CloseUi(void)
{
    if (MpUi_IsOpen())
        MpUi_Close();
    if (sT.taskId != TASK_NONE && FuncIsActiveTask(Task_Tourney))
        DestroyTask(sT.taskId);
    sT.taskId = TASK_NONE;
}

static void OpenUi(void)
{
    if (sT.taskId != TASK_NONE)
        return;
    MpUi_Open();
    sT.taskId = CreateTask(Task_Tourney, 80);
    sT.cursor = 0;
    Draw();
}

static void EndTourney(const char *notice)
{
    CloseUi();
    sT.state = TS_NONE;
    sT.me = ME_IDLE;
    if (notice)
        MP_ShowNotice(notice);
}

// ---------------------------------------------------------------------------
// Organizer

static void BroadcastRound(void)
{
    u8 buf[4 + MAX_P], i;
    buf[0] = MPR_T_ROUND;
    buf[1] = sT.tid;
    buf[2] = sT.round;
    buf[3] = sT.n;
    for (i = 0; i < sT.n; i++)
        buf[4 + i] = sT.wins[i];
    SendAll(buf, 4 + sT.n);
}

static void StartRound(void);

static void FinishTourney(void)
{
    u8 buf[3 + MAX_P * 2], order[MAX_P], i, j;
    for (i = 0; i < sT.n; i++)
        order[i] = i;
    for (i = 0; i < sT.n; i++)
        for (j = i + 1; j < sT.n; j++)
        {
            u8 a = order[i], b = order[j];
            // more wins first; a tie goes to whoever won their match
            if (sT.wins[b] > sT.wins[a] || (sT.wins[b] == sT.wins[a] && sT.results[b][a] == 1))
            {
                order[i] = b;
                order[j] = a;
            }
        }
    buf[0] = MPR_T_FINAL;
    buf[1] = sT.tid;
    buf[2] = sT.n;
    for (i = 0; i < sT.n; i++)
    {
        buf[3 + i * 2] = sT.ids[order[i]];
        buf[4 + i * 2] = sT.wins[order[i]];
    }
    SendAll(buf, 3 + sT.n * 2);
    MP_Tourney_OnRelay(gMpLocalId, buf, 3 + sT.n * 2);
}

// Every match of this round has a result (or a player left): next round
static void CheckRoundDone(void)
{
    u8 i;
    if (sT.organizer != gMpLocalId || sT.state != TS_RUNNING)
        return;
    for (i = 0; i < sT.n; i++)
    {
        u8 opp = OpponentIn(sT.round, sT.ids[i]);
        s8 j;
        if (opp == BYE)
            continue;
        j = IndexOf(opp);
        if (sT.results[i][j] != -1)
            continue;
        // Someone left: the one still here wins
        if (!Present(sT.ids[i]) || !Present(opp))
        {
            sT.results[i][j] = Present(sT.ids[i]) ? 1 : 0;
            sT.results[j][i] = Present(opp) ? 1 : 0;
            if (sT.results[i][j] == 1)
                sT.wins[i]++;
            if (sT.results[j][i] == 1)
                sT.wins[j]++;
            continue;
        }
        return;
    }
    if (sT.round + 1 >= sT.rounds)
        FinishTourney();
    else
    {
        sT.round++;
        BroadcastRound();
        StartRound();
    }
}

static void RecordResult(u8 who, u8 opp, u8 result)
{
    s8 i = IndexOf(who), j = IndexOf(opp);
    if (i < 0 || j < 0 || sT.results[i][j] != -1)
        return;
    if (result == 2)
        sT.results[i][j] = sT.results[j][i] = 2;
    else
    {
        u8 w = result == 1 ? i : j, l = result == 1 ? j : i;
        sT.results[w][l] = 1;
        sT.results[l][w] = 0;
        sT.wins[w]++;
    }
    CheckRoundDone();
}

static void Begin(void)
{
    u8 buf[3 + MAX_P], i;
    sT.n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (Present(i) && (i == gMpLocalId || sT.answers[i] == ANS_YES))
            sT.ids[sT.n++] = i;
    buf[0] = MPR_T_BEGIN;
    buf[1] = sT.tid;
    buf[2] = sT.n;
    for (i = 0; i < sT.n; i++)
        buf[3 + i] = sT.ids[i];
    SendAll(buf, 3 + sT.n);
    MP_Tourney_OnRelay(gMpLocalId, buf, 3 + sT.n);
}

// ---------------------------------------------------------------------------
// Players (the organizer is one too)

static void StartRound(void)
{
    char msg[64], name[16];
    sT.myOpponent = OpponentIn(sT.round, gMpLocalId);
    sT.iReady = sT.theyReady = FALSE;
    if (IndexOf(gMpLocalId) < 0)
        return;
    if (sT.myOpponent == BYE)
    {
        sT.me = ME_REPORTED;
        snprintf(msg, sizeof(msg), "Tournament round %d:\nyou have a break.", sT.round + 1);
    }
    else
    {
        sT.me = ME_PROMPT;
        NameOf(sT.myOpponent, name);
        snprintf(msg, sizeof(msg), "Tournament round %d:\nyour match vs %s!", sT.round + 1, name);
    }
    {
        u8 text[72];
        MP_AsciiToGame(text, msg, 70);
        MP_Toast(text);
    }
    sT.dirty = TRUE;
}

static void Report(u8 result)
{
    u8 buf[5] = {MPR_T_RESULT, sT.tid, sT.round, sT.myOpponent, result};
    sT.me = ME_REPORTED;
    if (sT.organizer == gMpLocalId)
        RecordResult(gMpLocalId, sT.myOpponent, result);
    else
        SendTo(sT.organizer, buf, 5);
}

static void TryStartMatch(void)
{
    if (sT.me != ME_WAITING || !sT.iReady || !sT.theyReady)
        return;
    CloseUi();   // our own window locks the controls; close it first
    if (!MP_IsLocalFree())
        return;  // tried again from MP_Tourney_Update once free
    sT.me = ME_BATTLING;
    gMpFriendlyBattle = TRUE;
    MP_Battle_Start(sT.myOpponent, gMpLocalId < sT.myOpponent, 0);
}

// From mp_battle.c when a friendly battle ends
void MP_Tourney_OnBattleResult(u8 outcome)
{
    gMpFriendlyBattle = FALSE;
    if (sT.state != TS_RUNNING || sT.me != ME_BATTLING)
        return;
    if (outcome == B_OUTCOME_WON)
        Report(1);
    else if (outcome == B_OUTCOME_DREW)
        Report(2);
    else
        Report(0);
}

static void GivePrizes(void)
{
    u8 i, rank = 0xFF;
    for (i = 0; i < sT.n; i++)
        if (sT.finalIds[i] == gMpLocalId)
            rank = i;
    if (rank == 0xFF)
        return;
    if (rank == 0)
    {
        u8 text[80];
        AddBagItem(ITEM_RARE_CANDY, 3);
        MP_Gatcha_AddPulls(2);
        MP_AsciiToGame(text, "Tournament champion!\nRARE CANDY x3, +2 MOEGATCHA", 76);
        MP_Toast(text);
        PlaySE(SE_SUCCESS);
    }
    else if (rank == 1)
    {
        u8 text[80];
        MP_Gatcha_AddPulls(1);
        MP_AsciiToGame(text, "2nd place in the tournament!\n+1 MOEGATCHA pull", 76);
        MP_Toast(text);
    }
    else
        MP_Events_Consolation("Thanks for battling!");
}

static void Task_Tourney(u8 taskId)
{
    if (sT.note[0] && JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
    {
        sT.note[0] = 0;
        Draw();
        return;
    }
    switch (sT.state)
    {
    case TS_GATHERING:
        if (MpUi_MoveCursor(&sT.cursor, 2))
            sT.dirty = TRUE;
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sT.cursor == 1))
        {
            u8 buf[2] = {MPR_T_CANCEL, sT.tid};
            SendAll(buf, 2);
            PlaySE(SE_SELECT);
            EndTourney(NULL);
            return;
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 i, joined = 0;
            for (i = 0; i < MP_MAX_PLAYERS; i++)
                if (i != gMpLocalId && Present(i) && sT.answers[i] == ANS_YES)
                    joined++;
            if (joined == 0)
            {
                PlaySE(SE_FAILURE);
                snprintf(sT.note, sizeof(sT.note), "Wait for at least one friend to join.");
                sT.dirty = TRUE;
            }
            else
            {
                PlaySE(SE_SELECT);
                Begin();
            }
        }
        break;
    case TS_INVITED:
        if (MpUi_MoveCursor(&sT.cursor, 2))
            sT.dirty = TRUE;
        else if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            bool8 yes = JOY_NEW(A_BUTTON) && sT.cursor == 0;
            u8 buf[3] = {MPR_T_ANSWER, sT.tid, yes};
            SendTo(sT.organizer, buf, 3);
            PlaySE(SE_SELECT);
            if (!yes)
            {
                EndTourney(NULL);
                return;
            }
            CloseUi();
            sT.state = TS_NONE;   // waiting for BEGIN
            sT.me = ME_IDLE;
            MP_ShowNotice("You joined the tournament! It starts when the host begins it.");
            return;
        }
        break;
    case TS_RUNNING:
        if (sT.me == ME_PROMPT)
        {
            if (MpUi_MoveCursor(&sT.cursor, 2))
                sT.dirty = TRUE;
            else if (JOY_NEW(A_BUTTON) && sT.cursor == 0 && !HasAbleMon())
            {
                // Ready check: nobody can battle
                TeamStatus(sT.note, sizeof(sT.note));
                PlaySE(SE_FAILURE);
                sT.dirty = TRUE;
            }
            else if (JOY_NEW(A_BUTTON) && sT.cursor == 0)
            {
                u8 buf[3] = {MPR_T_READY, sT.tid, sT.round};
                PlaySE(SE_SELECT);
                SendTo(sT.myOpponent, buf, 3);
                sT.iReady = TRUE;
                sT.me = ME_WAITING;
                sT.dirty = TRUE;
            }
            else if (JOY_NEW(A_BUTTON) && sT.cursor == 1)
            {
                u8 buf[3] = {MPR_T_FORFEIT, sT.tid, sT.round};
                PlaySE(SE_SELECT);
                SendTo(sT.myOpponent, buf, 3);
                Report(0);
                sT.dirty = TRUE;
            }
            else if (JOY_NEW(B_BUTTON))
            {
                CloseUi();
                return;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            CloseUi();
            return;
        }
        break;
    case TS_DONE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            EndTourney(NULL);
            return;
        }
        break;
    default:
        CloseUi();
        return;
    }
    if (sT.taskId != TASK_NONE)
    {
        TryStartMatch();
        if (sT.dirty && sT.taskId != TASK_NONE)
            Draw();
    }
}

void MP_Tourney_OpenMenu(void)
{
    InitIfNeeded();
    if (!MP_IsConnected() || gMpLocalId < 0)
    {
        MP_ShowNotice("Go online (CO-OP menu: {COOP}) to hold a battle tournament.");
        return;
    }
    if (sT.state == TS_RUNNING || sT.state == TS_DONE)
    {
        OpenUi();
        return;
    }
    if (sT.state != TS_NONE)
        return;
    {
        u8 ids[MP_MAX_PLAYERS], i, buf[2];
        if (MP_Friends_List(ids) == 0)
        {
            MP_ShowNotice("A tournament needs at least two players online.");
            return;
        }
        memset(sT.answers, ANS_WAIT, sizeof(sT.answers));
        sT.tid++;
        sT.organizer = gMpLocalId;
        sT.state = TS_GATHERING;
        sT.note[0] = 0;
        buf[0] = MPR_T_INVITE;
        buf[1] = sT.tid;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            (void)i;
        SendAll(buf, 2);
        OpenUi();
    }
}

void MP_Tourney_OnRelay(u8 from, const u8 *data, u16 len)
{
    InitIfNeeded();
    if (len < 2 || from >= MP_MAX_PLAYERS)
        return;
    switch (data[0])
    {
    case MPR_T_INVITE:
        if (sT.state == TS_NONE && sT.me == ME_IDLE)
        {
            sT.tid = data[1];
            sT.organizer = from;
            sT.state = TS_INVITED;
            sT.cursor = 0;
            // shown once we're free (MP_Tourney_Update)
        }
        break;
    case MPR_T_ANSWER:
        if (sT.state == TS_GATHERING && data[1] == sT.tid && len >= 3)
        {
            sT.answers[from] = data[2] ? ANS_YES : ANS_NO;
            sT.dirty = TRUE;
            if (sT.taskId != TASK_NONE)
                Draw();
        }
        break;
    case MPR_T_BEGIN:
        if (from == sT.organizer && data[1] == sT.tid && len >= 3 && data[2] <= MAX_P && len >= 3 + data[2])
        {
            u8 i, j;
            bool8 meIn = FALSE;
            sT.n = data[2];
            for (i = 0; i < sT.n; i++)
            {
                sT.ids[i] = data[3 + i];
                if (sT.ids[i] == gMpLocalId)
                    meIn = TRUE;
            }
            if (!meIn)
            {
                if (sT.state == TS_INVITED)
                    EndTourney(NULL);
                return;
            }
            for (i = 0; i < MAX_P; i++)
            {
                sT.wins[i] = 0;
                for (j = 0; j < MAX_P; j++)
                    sT.results[i][j] = -1;
            }
            sT.round = 0;
            sT.rounds = (sT.n & 1) ? sT.n : sT.n - 1;
            sT.state = TS_RUNNING;
            sT.finalShown = FALSE;
            CloseUi();
            StartRound();
        }
        break;
    case MPR_T_READY:
        if (sT.state == TS_RUNNING && data[1] == sT.tid && len >= 3 && data[2] == sT.round && from == sT.myOpponent)
        {
            sT.theyReady = TRUE;
            sT.dirty = TRUE;
        }
        break;
    case MPR_T_FORFEIT:
        if (sT.state == TS_RUNNING && data[1] == sT.tid && len >= 3 && data[2] == sT.round && from == sT.myOpponent
         && (sT.me == ME_PROMPT || sT.me == ME_WAITING))
        {
            u8 text[64];
            Report(1);
            MP_AsciiToGame(text, "Your opponent forfeited.\nYou win this match!", 60);
            MP_Toast(text);
            sT.dirty = TRUE;
        }
        break;
    case MPR_T_RESULT:
        if (sT.organizer == gMpLocalId && sT.state == TS_RUNNING && data[1] == sT.tid && len >= 5 && data[2] == sT.round)
            RecordResult(from, data[3], data[4]);
        break;
    case MPR_T_ROUND:
        if (from == sT.organizer && data[1] == sT.tid && len >= 4 && sT.state == TS_RUNNING)
        {
            u8 i;
            sT.round = data[2];
            for (i = 0; i < data[3] && i < MAX_P && 4 + i < len; i++)
                sT.wins[i] = data[4 + i];
            StartRound();
        }
        break;
    case MPR_T_FINAL:
        if (from == sT.organizer && data[1] == sT.tid && len >= 3 && sT.state == TS_RUNNING)
        {
            u8 i, n = data[2];
            for (i = 0; i < n && i < MAX_P && 4 + i * 2 < len; i++)
            {
                s8 k;
                sT.finalIds[i] = data[3 + i * 2];
                sT.finalWins[i] = data[4 + i * 2];
                k = IndexOf(sT.finalIds[i]);
                if (k >= 0)
                    sT.wins[k] = sT.finalWins[i];
            }
            sT.state = TS_DONE;
            sT.me = ME_IDLE;
            GivePrizes();
            sT.dirty = TRUE;
        }
        break;
    case MPR_T_CANCEL:
        if (from == sT.organizer && data[1] == sT.tid && sT.state != TS_NONE)
            EndTourney("The tournament was called off.");
        break;
    }
}

// Every frame while online
void MP_Tourney_Update(void)
{
    InitIfNeeded();
    if (sT.state == TS_NONE)
        return;
    // The organizer left: nothing can go on
    if (sT.state != TS_GATHERING && sT.organizer != gMpLocalId && !Present(sT.organizer))
    {
        EndTourney("The tournament host left, so the tournament ended.");
        return;
    }
    if (sT.organizer == gMpLocalId && sT.state == TS_RUNNING)
        CheckRoundDone();
    if (sT.taskId != TASK_NONE)
        return;
    // Show what needs the player once they're free
    if (!MP_IsLocalFree() || MpUi_IsOpen())
        return;
    if (sT.state == TS_INVITED)
    {
        PlaySE(SE_PIN);
        OpenUi();
    }
    else if (sT.state == TS_RUNNING && sT.me == ME_PROMPT)
        OpenUi();
    else if (sT.state == TS_RUNNING && sT.me == ME_WAITING)
        TryStartMatch();
    else if (sT.state == TS_DONE && !sT.finalShown)
    {
        sT.finalShown = TRUE;
        OpenUi();
    }
}
