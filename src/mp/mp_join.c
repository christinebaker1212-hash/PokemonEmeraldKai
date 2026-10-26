// Moemon Co-op: joining a friend's battle in progress.
//
// A friend walks up to a player who is battling and picks JOIN ("Would you like to spectate, bet, or join?",
// mp_spectate.c). At the start of the battle's next turn the battling player (the master: seat 0 of a co-op
// battle) is asked "X wants to join your battle! Is this okay?". NO: the friend hears so. YES: the friend sends
// the Pokémon they bring, the master's game turns the battle into a co-op Double Battle (or a Triple Battle) and
// sends everyone the battle as it stands, and once every game has rebuilt its screen the friend's trainer slides
// in ("X has come to help!") and sends their Pokémon out; the trainer sends out another Pokémon for the new spot
// when it has one (BattleScript_MpJoinEntrance). mp_coop.c rearranges the parties (MP_Coop_Join*), battle_main.c
// starts the friend's battle from the snapshot (CB2_MpJoinBattle) and the end of the battle is the usual co-op one.
//
// Which battles: a solo battle against a wild Pokémon or an ordinary trainer (not a gym leader, admin, rival or
// other story battle), or a co-op Double Battle of that kind (it becomes a Triple Battle). Up to two friends at once.
// A Double Battle the player started alone: their second Pokémon goes back to its ball and the friend takes its spot
// (a co-op double; a second friend later makes it a triple). Two trainers who spotted the player at once count too:
// each keeps their own team (slots 0-2 / 3-5); in a Triple Battle against both, the third foe spot is filled from the
// trainer with more Pokémon left and the two share their Pokémon from then on (mp_coop.c MP_Coop_JoinConvertMaster).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_join.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_scripts.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "event_data.h"
#include "field_message_box.h"
#include "field_screen_effect.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "malloc.h"
#include "battle_anim.h"
#include "map_name_popup.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "reshow_battle_screen.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/battle.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/battle_setup.h"
#include "constants/songs.h"
#include "constants/trainers.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_StartBattleLinkTasks(void);       // battle_controllers.c
bool8 MP_BattleLinkBuffersEmpty(void);    // battle_controllers.c
void MP_Battle_JoinLink(const u8 *players, u8 count, u8 linkId, u16 seq, bool8 fresh);   // mp_battle.c
u16 MP_Battle_SyncSeq(void);
bool8 MP_Battle_LinkDrained(void);
void SaveLinkPlayers(u8 playerCount);
u8 MP_GetWildBattleTransition(void);
u8 MP_GetTrainerBattleTransition(void);
void BattleMainCB2(void);
void CB2_MpJoinBattle(void);              // battle_main.c
MainCallback MP_EndWildBattleCallback(void);      // battle_setup.c
MainCallback MP_EndTrainerBattleCallback(void);

enum {
    JOINM_ASK = MPR_JOIN_FIRST,  // friend -> battling player: may I join?
    JOINM_CANCEL,                // friend -> master: never mind
    JOINM_ANSWER,                // master -> friend: u8 kind, u8 arg, u8 arg2
    JOINM_PARTY,                 // friend -> master: u8 count, u16 EXP bonus, LinkPlayer, count Pokémon
    JOINM_STATE,                 // master -> friend: struct MpJoinSnapshot, then both parties
    JOINM_CONVERT,               // master -> partner already in the battle: the same, plus their old seat's Pokémon
    JOINM_READY,                 // friend / partner -> master: in the new battle, screen rebuilt
    JOINM_ABORT,                 // master -> friend: called off after YES (before the battle started for them)
};

enum {
    ANS_WAIT,       // asked: the answer comes at the start of the next turn
    ANS_NO,
    ANS_YES,        // arg = Pokémon each player brings (3 or 2), arg2 = players after (+ 0x80: wild), u16 trainer
    ANS_CANT,       // arg = JOIN_CANT_*
    ANS_REDIRECT,   // arg = the battle's master (we asked their partner)
};

enum {
    JOIN_CANT_OVER,     // not in a battle (any more)
    JOIN_CANT_STORY,    // a gym leader, rival or other story battle
    JOIN_CANT_FULL,     // three players already (or asked)
    JOIN_CANT_KIND,     // a kind of battle nobody can join (double, frontier, PvP, safari...)
    JOIN_CANT_BUSY,     // in the middle of letting someone else in
};

// Master's steps at the start of a turn
enum {
    MJS_IDLE,
    MJS_PROMPT_PRINT,
    MJS_PROMPT_YESNO,
    MJS_WAIT_PARTY,
    MJS_RECALL,      // a solo Double Battle: the player's second Pokémon goes back to make room
    MJS_RECALLED,
    MJS_CONVERT,
    MJS_FADE,
    MJS_REBUILD,
    MJS_WAIT_READY,
    MJS_ENTRANCE,
};

// A partner already in the battle (a co-op double becoming a triple)
enum {
    PJS_IDLE,
    PJS_WAIT_IDLE,   // CONVERT is here: wait until our screen caught up with the master
    PJS_FADE,
    PJS_REBUILD,
};

// The friend asking
enum {
    FJS_NONE,
    FJS_ASKING,      // "Asking ..." (no answer yet)
    FJS_WAITING,     // the master will answer at the start of the next turn
    FJS_ANSWERED,    // a NO / CAN'T message is up
    FJS_GOING,       // YES: into the battle
};

#define PARTY_WAIT_FRAMES   (60 * 20)
#define ASK_ANSWER_FRAMES   (60 * 15)
#define STATE_WAIT_FRAMES   (60 * 30)
#define PARTNER_IDLE_FRAMES (60 * 15)

struct MpJoinSnapshot
{
    struct MpJoinPlan plan;
    u8 seat;                           // the receiver's seat
    u8 pad;
    u16 syncSeq;
    u16 trainerId;
    u8 battleMonForms[MAX_BATTLERS_COUNT];
    struct BattleSpriteInfo spriteInfo[MAX_BATTLERS_COUNT];
    struct BattlePokemon battleMons[MAX_BATTLERS_COUNT];
    u16 sideStatuses[NUM_BATTLE_SIDES];
    u8 haveLoseText;
    u8 loseText[JOIN_LOSE_TEXT_MAX + 1];
    u8 haveLoseTextB;                  // two trainers: the second one's
    u8 loseTextB[JOIN_LOSE_TEXT_MAX + 1];
};

static EWRAM_DATA struct
{
    // --- the battle's master ---
    u8 pending[JOIN_MAX_NEW];        // friends who asked, in order (network ids)
    u8 pendingCount;
    u8 asking[JOIN_MAX_NEW];         // the ones in the prompt / let in
    u8 askingCount;
    bool8 turnStart;                 // a turn is starting: the requests come first
    u8 state;
    u16 timer;
    u8 yesNo;
    bool8 gotParty[JOIN_MAX_NEW];
    u8 partyCount[JOIN_MAX_NEW];
    u16 partyBonus[JOIN_MAX_NEW];
    struct LinkPlayer partyLink[JOIN_MAX_NEW];
    struct Pokemon partyMons[JOIN_MAX_NEW][MULTI_PARTY_SIZE];
    u8 needReady;                    // seats still to say READY
    bool8 fadeOn;                    // our fade to black has started (another fade may have been running)
    u8 shownWaiting;                 // whose name "Waiting for X…" shows (0xFF = none)
    u8 declined;                     // friends our player said NO to in this battle (they aren't asked again)
    // --- every game: battlers coming in (skipped by the screen rebuild until the entrance shows them) ---
    u8 entering;
    // --- a partner already in the battle ---
    u8 pstate;
    u16 ptimer;
    bool8 pfadeOn;
    // --- the friend ---
    u8 fstate;
    s8 target;                       // who we asked (then the master)
    u16 ftimer;
    u8 seatSize;
    bool8 wild;
    u8 picks[MULTI_PARTY_SIZE];      // our party slots we bring, in battle order
    u8 nPicks;
    bool8 gotState;
    bool8 aborted;
    bool8 fromScript;                // asked from the trainer's script (it spotted us while battling them): the
                                     // script waits; not let in, it goes on to our own battle (MP_Join_AskFromScript)
    u8 fmsg[200];
} sJoin;

static EWRAM_DATA u8 sSnapBuf[sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * (PARTY_SIZE * 2 + MULTI_PARTY_SIZE)];
static EWRAM_DATA struct Pokemon sOldSeatMons[MULTI_PARTY_SIZE];

bool8 gMpJoinedBattle;   // this battle was joined while it ran (keeps its own background, battle_bg.c)

static void Send(u8 to, const u8 *data, u16 len)
{
    if (to < MP_MAX_PLAYERS)
        MP_SendRelay(to, data, len);
}

static void SendAnswer(u8 to, u8 kind, u8 arg, u8 arg2)
{
    u8 buf[6] = {JOINM_ANSWER, kind, arg, arg2, gTrainerBattleOpponent_A & 0xFF, gTrainerBattleOpponent_A >> 8};
    Send(to, buf, sizeof(buf));
    MPNet_Log("join: answer %d (%d) to player %d", kind, arg, to + 1);
}

static bool8 PlayerThere(s8 id)
{
    return id >= 0 && id < MP_MAX_PLAYERS && id != gMpLocalId && gMpPlayers[id].present;
}

static void NameAscii(char *dst, u8 id)
{
    if (id < MP_MAX_PLAYERS)
        MP_GameToAscii(dst, gMpPlayers[id].name, MP_NAME_LEN);
    else
        dst[0] = '\0';
}

// ---------------------------------------------------------------------------
// Every game

bool8 MP_Join_IsEntering(u8 battler)
{
    return battler < MAX_BATTLERS_COUNT && (sJoin.entering & (1 << battler));
}

u8 MP_Join_EnteringMask(void)
{
    return sJoin.entering;
}

// ---------------------------------------------------------------------------
// The battle's master: who may join

// 0xFF: this battle can take a friend; otherwise JOIN_CANT_*
static u8 JoinableReason(void)
{
    u32 flags = gBattleTypeFlags & ~(BATTLE_TYPE_LINK_IN_BATTLE | BATTLE_TYPE_IS_MASTER);
    if (!gMain.inBattle || gBattleOutcome != 0 || gBattleMainFunc == NULL)
        return JOIN_CANT_OVER;
    if (gMpCoopBattle)
    {
        if (!MP_Coop_IsMaster())
            return JOIN_CANT_KIND;   // (two trainers: a third player makes it a Triple Battle against both)
        if (MP_Coop_SeatCount() >= 3)
            return JOIN_CANT_FULL;
        if (!gMpCoopWild && !MP_Coop_JoinTrainerOk())
            return JOIN_CANT_STORY;
        return 0xFF;
    }
    if (gMpFriendlyBattle || gNetLinkActive)
        return JOIN_CANT_KIND;
    if (flags == 0 || flags == BATTLE_TYPE_DOUBLE)
    {
        // a wild Pokémon met in the grass, a cave or the water (not a scripted one like KECLEON: its event goes on
        // after the battle)
        return gMain.savedCallback == MP_EndWildBattleCallback() ? 0xFF : JOIN_CANT_STORY;
    }
    // one trainer, single or double (a Double Battle the player started alone: their second Pokémon makes room), or
    // two trainers who spotted the player at once (a friend makes it a co-op battle against both)
    if ((flags != BATTLE_TYPE_TRAINER && flags != (BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE)
         && !(flags == (BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS) && gTrainerBattleOpponent_B != 0))
     || gMain.savedCallback != MP_EndTrainerBattleCallback())
        return JOIN_CANT_KIND;   // doubles, two trainers, frontier, safari, legendaries...
    if (!MP_Coop_JoinTrainerOk())
        return JOIN_CANT_STORY;
    return 0xFF;
}

static void RemovePending(u8 id)
{
    u8 i, n = 0;
    for (i = 0; i < sJoin.pendingCount; i++)
        if (sJoin.pending[i] != id)
            sJoin.pending[n++] = sJoin.pending[i];
    sJoin.pendingCount = n;
}

static s8 AskingIndex(u8 id)
{
    u8 i;
    for (i = 0; i < sJoin.askingCount; i++)
        if (sJoin.asking[i] == id)
            return i;
    return -1;
}

// Friends being let in this turn who aren't counted in the seats yet
static u8 JoiningCount(void)
{
    switch (sJoin.state)
    {
    case MJS_PROMPT_PRINT:
    case MJS_PROMPT_YESNO:
    case MJS_WAIT_PARTY:
    case MJS_RECALL:
    case MJS_RECALLED:
    case MJS_CONVERT:
        return sJoin.askingCount;
    }
    return 0;   // (from the conversion on, they're seats)
}

static void OnAsk(u8 from)
{
    u8 reason, i;
    if (!gMain.inBattle)
    {
        SendAnswer(from, ANS_CANT, JOIN_CANT_OVER, 0);
        return;
    }
    if (gMpCoopBattle && !MP_Coop_IsMaster())
    {
        // they talked to us: the battle's master decides
        SendAnswer(from, ANS_REDIRECT, MP_Coop_SeatPlayer(0), 0);
        return;
    }
    reason = JoinableReason();
    if (reason != 0xFF)
    {
        SendAnswer(from, ANS_CANT, reason, 0);
        return;
    }
    if (sJoin.declined & (1 << from))
    {
        // our player already said no in this battle: not asked again
        SendAnswer(from, ANS_NO, 0, 0);
        return;
    }
    for (i = 0; i < sJoin.pendingCount; i++)
    {
        if (sJoin.pending[i] == from)
        {
            SendAnswer(from, ANS_WAIT, 0, 0);
            return;
        }
    }
    if (AskingIndex(from) >= 0 && sJoin.state != MJS_IDLE)
    {
        SendAnswer(from, ANS_CANT, JOIN_CANT_BUSY, 0);   // (already being let in)
        return;
    }
    // while others are being let in, a friend who asks waits for the next turn (if there's room left)
    if (MP_Coop_SeatCount() + sJoin.pendingCount + JoiningCount() >= 3)
    {
        SendAnswer(from, ANS_CANT, JOIN_CANT_FULL, 0);
        return;
    }
    sJoin.pending[sJoin.pendingCount++] = from;
    MPNet_Log("join: player %d asks to join our battle", from + 1);
    SendAnswer(from, ANS_WAIT, 0, 0);
}

// Battle turns: battle_main.c

void MP_Join_OnTurnStart(void)
{
    if (sJoin.pendingCount != 0 && (!gMpCoopBattle || MP_Coop_IsMaster()))
        sJoin.turnStart = TRUE;
}

static void ShowPrompt(void)
{
    char a[16], b[16], text[120];
    NameAscii(a, sJoin.asking[0]);
    if (sJoin.askingCount >= 2)
    {
        NameAscii(b, sJoin.asking[1]);
        snprintf(text, sizeof(text), "%s and %s want to join\nyour battle! Is this okay?", a, b);
    }
    else
    {
        snprintf(text, sizeof(text), "%s wants to join your\nbattle! Is this okay?", a);
    }
    MP_AsciiToGame(gDisplayedStringBattle, text, sizeof(gDisplayedStringBattle) - 1);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
}

static void ShowWaiting(void)
{
    char a[16], text[80];
    u8 i;
    for (i = 0; i < sJoin.askingCount; i++)
        if (!sJoin.gotParty[i])
            break;
    i = sJoin.asking[i < sJoin.askingCount ? i : 0];
    if (i == sJoin.shownWaiting)
        return;   // already up (printing it again starts it over)
    sJoin.shownWaiting = i;
    NameAscii(a, i);
    snprintf(text, sizeof(text), "Waiting for %s…", a);
    MP_AsciiToGame(gDisplayedStringBattle, text, sizeof(gDisplayedStringBattle) - 1);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
}

static void ClearMessage(void)
{
    BattlePutTextOnWindow(gText_EmptyString3, B_WIN_MSG);
}

// The battle goes on as it was (nobody joins this turn)
static bool8 ResumeTurn(void)
{
    sJoin.state = MJS_IDLE;
    sJoin.askingCount = 0;
    sJoin.turnStart = FALSE;
    return FALSE;
}

static void SendSnapshots(const struct MpJoinPlan *plan)
{
    struct MpJoinSnapshot *snap = (struct MpJoinSnapshot *)sSnapBuf;
    u8 *parties = sSnapBuf + sizeof(struct MpJoinSnapshot);
    const u8 *lose = NULL;
    u8 seat, b;
    u16 len;

    memset(snap, 0, sizeof(*snap));
    snap->plan = *plan;
    snap->syncSeq = MP_Battle_SyncSeq();
    snap->trainerId = gTrainerBattleOpponent_A;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
    {
        snap->battleMonForms[b] = gBattleMonForms[b];
        snap->spriteInfo[b] = gBattleSpritesDataPtr->battlerData[b];
        snap->battleMons[b] = gBattleMons[b];
    }
    snap->sideStatuses[0] = gSideStatuses[0];
    snap->sideStatuses[1] = gSideStatuses[1];
    if (!gMpCoopWild && gTrainerBattleOpponent_A != 0)
        lose = MP_Coop_JoinLoseText();
    if (lose != NULL)
    {
        u16 i;
        for (i = 0; i < JOIN_LOSE_TEXT_MAX && lose[i] != EOS; i++)
            snap->loseText[i] = lose[i];
        snap->loseText[i] = EOS;
        snap->haveLoseText = TRUE;
    }
    lose = plan->trainerIdB != 0 ? MP_Coop_JoinLoseTextB() : NULL;
    if (lose != NULL)
    {
        u16 i;
        for (i = 0; i < JOIN_LOSE_TEXT_MAX && lose[i] != EOS; i++)
            snap->loseTextB[i] = lose[i];
        snap->loseTextB[i] = EOS;
        snap->haveLoseTextB = TRUE;
    }
    memcpy(parties, gPlayerParty, sizeof(struct Pokemon) * PARTY_SIZE);
    memcpy(parties + sizeof(struct Pokemon) * PARTY_SIZE, gEnemyParty, sizeof(struct Pokemon) * PARTY_SIZE);
    len = sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * PARTY_SIZE * 2;
    for (seat = 1; seat < plan->count; seat++)
    {
        u8 *msg = Alloc(1 + len + sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        u16 n = 1 + len;
        snap->seat = seat;
        msg[0] = seat < plan->oldCount ? JOINM_CONVERT : JOINM_STATE;
        memcpy(msg + 1, sSnapBuf, len);
        if (seat < plan->oldCount)
        {
            // their Pokémon as the master saw them before (the ones left out go back to their real party)
            memcpy(msg + n, sOldSeatMons, sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
            n += sizeof(struct Pokemon) * MULTI_PARTY_SIZE;
        }
        Send(plan->seatPlayer[seat], msg, n);
        Free(msg);
        MPNet_Log("join: sent the battle to seat %d (player %d, %s)", seat, plan->seatPlayer[seat] + 1,
                  seat < plan->oldCount ? "partner" : "joining");
    }
}

// Everyone's PARTY is here: the battle becomes a co-op battle with the friends in it
static void ConvertBattle(void)
{
    struct MpJoinPlan plan;
    u8 joiners[JOIN_MAX_NEW], counts[JOIN_MAX_NEW], n = 0, i, seat;
    u16 bonus[JOIN_MAX_NEW];
    struct LinkPlayer links[JOIN_MAX_NEW];
    struct Pokemon (*mons)[MULTI_PARTY_SIZE] = Alloc(sizeof(struct Pokemon) * MULTI_PARTY_SIZE * JOIN_MAX_NEW);
    bool8 wasCoop = gMpCoopBattle;

    for (i = 0; i < sJoin.askingCount; i++)
    {
        if (!sJoin.gotParty[i] || !PlayerThere(sJoin.asking[i]))
            continue;
        joiners[n] = sJoin.asking[i];
        counts[n] = sJoin.partyCount[i];
        bonus[n] = sJoin.partyBonus[i];
        links[n] = sJoin.partyLink[i];
        memcpy(mons[n], sJoin.partyMons[i], sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
        n++;
    }
    // the partner's Pokémon as they are now (sent back to them with CONVERT)
    if (wasCoop)
        memcpy(sOldSeatMons, &gPlayerParty[MULTI_PARTY_SIZE], sizeof(sOldSeatMons));
    MP_Coop_JoinConvertMaster(n, joiners, mons, counts, links, bonus, &plan);
    Free(mons);
    sJoin.entering = plan.entering;
    gMpJoinedBattle = TRUE;
    MP_Battle_JoinLink(plan.seatPlayer, plan.count, 0, 0, !wasCoop);
    SaveLinkPlayers(plan.count);
    if (!wasCoop)
    {
        MP_StartBattleLinkTasks();
        gBattleTypeFlags |= BATTLE_TYPE_LINK_IN_BATTLE;
    }
    gBattleScripting.multiplayerId = GetMultiplayerId();
    sJoin.needReady = 0;
    for (seat = 1; seat < plan.count; seat++)
        sJoin.needReady |= 1 << seat;
    SendSnapshots(&plan);
    MP_Coop_SetBattleStatus(MP_BST_NONE);
}

static void TryReadyStep(void);

// Top of HandleTurnActionSelectionState on the master: TRUE holds the turn
bool8 MP_Join_TurnHook(void)
{
    u8 i;
    if (!sJoin.turnStart)
        return FALSE;
    switch (sJoin.state)
    {
    case MJS_IDLE:
    {
        u8 reason, room;
        if (sJoin.pendingCount == 0)
            return ResumeTurn();
        if (gBattleControllerExecFlags)
            return TRUE;
        reason = JoinableReason();
        room = 3 - MP_Coop_SeatCount();
        sJoin.askingCount = 0;
        for (i = 0; i < sJoin.pendingCount; i++)
        {
            u8 id = sJoin.pending[i];
            if (!PlayerThere(id))
                continue;
            if (reason != 0xFF)
                SendAnswer(id, ANS_CANT, reason, 0);
            else if (sJoin.askingCount >= room)
                SendAnswer(id, ANS_CANT, JOIN_CANT_FULL, 0);
            else
                sJoin.asking[sJoin.askingCount++] = id;
        }
        sJoin.pendingCount = 0;
        if (sJoin.askingCount == 0)
            return ResumeTurn();
        memset(sJoin.gotParty, 0, sizeof(sJoin.gotParty));
        ShowPrompt();
        sJoin.state = MJS_PROMPT_PRINT;
        MPNet_Log("join: asking our player about %d friend(s)", sJoin.askingCount);
        return TRUE;
    }
    case MJS_PROMPT_PRINT:
        if (!IsTextPrinterActive(B_WIN_MSG))
        {
            HandleBattleWindow(YESNOBOX_X_Y, 0);
            BattlePutTextOnWindow(gText_BattleYesNoChoice, B_WIN_YESNO);
            sJoin.yesNo = 0;
            BattleCreateYesNoCursorAt(0);
            sJoin.state = MJS_PROMPT_YESNO;
        }
        return TRUE;
    case MJS_PROMPT_YESNO:
        if (JOY_NEW(DPAD_UP) && sJoin.yesNo != 0)
        {
            PlaySE(SE_SELECT);
            BattleDestroyYesNoCursorAt(sJoin.yesNo);
            sJoin.yesNo = 0;
            BattleCreateYesNoCursorAt(0);
        }
        else if (JOY_NEW(DPAD_DOWN) && sJoin.yesNo == 0)
        {
            PlaySE(SE_SELECT);
            BattleDestroyYesNoCursorAt(sJoin.yesNo);
            sJoin.yesNo = 1;
            BattleCreateYesNoCursorAt(1);
        }
        else if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            bool8 yes = JOY_NEW(A_BUTTON) && sJoin.yesNo == 0;
            u8 count = MP_Coop_SeatCount() + sJoin.askingCount;
            PlaySE(SE_SELECT);
            HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
            if (!yes)
            {
                for (i = 0; i < sJoin.askingCount; i++)
                {
                    SendAnswer(sJoin.asking[i], ANS_NO, 0, 0);
                    sJoin.declined |= 1 << sJoin.asking[i];
                }
                MPNet_Log("join: our player said no");
                ClearMessage();
                return ResumeTurn();
            }
            for (i = 0; i < sJoin.askingCount; i++)
                SendAnswer(sJoin.asking[i], ANS_YES, count == 3 ? 2 : 3, count | (gMpCoopWild || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER) ? 0x80 : 0));
            MPNet_Log("join: our player said yes (%d players)", count);
            sJoin.timer = 0;
            sJoin.state = MJS_WAIT_PARTY;
            sJoin.shownWaiting = 0xFF;
            ShowWaiting();
        }
        return TRUE;
    case MJS_WAIT_PARTY:
    {
        bool8 all = TRUE;
        u8 n = 0;
        for (i = 0; i < sJoin.askingCount; i++)
            if (PlayerThere(sJoin.asking[i]) && !sJoin.gotParty[i])
                all = FALSE;
        if (!all && ++sJoin.timer < PARTY_WAIT_FRAMES)
            return TRUE;
        // the ones who never sent their Pokémon (or left) stay out
        for (i = 0; i < sJoin.askingCount; i++)
        {
            if (sJoin.gotParty[i] && PlayerThere(sJoin.asking[i]))
            {
                if (n != i)
                {
                    sJoin.asking[n] = sJoin.asking[i];
                    sJoin.gotParty[n] = TRUE;
                    sJoin.partyCount[n] = sJoin.partyCount[i];
                    sJoin.partyBonus[n] = sJoin.partyBonus[i];
                    sJoin.partyLink[n] = sJoin.partyLink[i];
                    memcpy(sJoin.partyMons[n], sJoin.partyMons[i], sizeof(sJoin.partyMons[n]));
                }
                n++;
            }
            else if (PlayerThere(sJoin.asking[i]))
            {
                u8 buf[1] = {JOINM_ABORT};
                Send(sJoin.asking[i], buf, 1);
                MPNet_Log("join: no Pokémon from player %d, left out", sJoin.asking[i] + 1);
            }
        }
        sJoin.askingCount = n;
        if (n == 0)
        {
            MPNet_Log("join: nobody came, the battle goes on");
            ClearMessage();
            return ResumeTurn();
        }
        sJoin.state = MJS_CONVERT;
        if (!gMpCoopBattle && (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
         && !(gAbsentBattlerFlags & (1 << 2)) && gBattleMons[2].hp != 0)
            sJoin.state = MJS_RECALL;
        return TRUE;
    }
    case MJS_RECALL:
        if (IsTextPrinterActive(B_WIN_MSG) || gBattleControllerExecFlags)
            return TRUE;
        // "MARILL, come back!": battler 2's spot is the friend's (BattleScript_MpJoinRecall ends back here)
        MPNet_Log("join: our second Moemon (party slot %d) makes room", gBattlerPartyIndexes[2]);
        gBattlerAttacker = 2;
        sJoin.state = MJS_RECALLED;
        BattleScriptExecute(BattleScript_MpJoinRecall);
        return TRUE;
    case MJS_RECALLED:
        sJoin.state = MJS_CONVERT;
        return TRUE;
    case MJS_CONVERT:
        if (IsTextPrinterActive(B_WIN_MSG))
            return TRUE;
        ClearMessage();
        ConvertBattle();
        sJoin.fadeOn = FALSE;
        sJoin.state = MJS_FADE;
        // fall through
    case MJS_FADE:
        // The screen is rebuilt behind black. A fade still running (a solo Double Battle's recall leaves one) would
        // turn ours down: wait for it, or the rebuild happens in plain view.
        if (!sJoin.fadeOn)
        {
            if (BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK))
                sJoin.fadeOn = TRUE;
            return TRUE;
        }
        if (!gPaletteFade.active)
        {
            MPNet_Log("join: rebuilding the screen behind black (fade %d)", gPaletteFade.y);
            ReshowBattleScreenAfterMenu();
            sJoin.state = MJS_REBUILD;
        }
        return TRUE;
    case MJS_REBUILD:
        if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
        {
            sJoin.timer = 0;
            sJoin.state = MJS_WAIT_READY;
        }
        return TRUE;
    case MJS_WAIT_READY:
        TryReadyStep();
        return TRUE;
    case MJS_ENTRANCE:
        // the entrance script is done: the turn starts with everyone in it
        for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
            gBattleCommunication[i] = 0;
        for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        {
            gChosenActionByBattler[i] = B_ACTION_NONE;
            gChosenMoveByBattler[i] = MOVE_NONE;
            *(gBattleStruct->monToSwitchIntoId + i) = PARTY_SIZE;
        }
        gBattleStruct->absentBattlerFlags = gAbsentBattlerFlags;
        sJoin.entering = 0;
        sJoin.askingCount = 0;
        sJoin.state = MJS_IDLE;
        sJoin.turnStart = FALSE;
        ClearMessage();
        MPNet_Log("join: entrance done, absent %02x, the turn starts", gAbsentBattlerFlags);
        return FALSE;
    }
    return FALSE;
}

static void TryReadyStep(void)
{
    if (sJoin.needReady != 0)
    {
        // a seat whose player left can't say it (the battle is called off by the link anyway)
        u8 seat;
        for (seat = 1; seat < MP_Coop_SeatCount(); seat++)
            if ((sJoin.needReady & (1 << seat)) && !PlayerThere(MP_Coop_SeatPlayer(seat)))
                sJoin.needReady &= ~(1 << seat);
        if (sJoin.needReady != 0)
        {
            if (++sJoin.timer % (60 * 10) == 0)
                MPNet_Log("join: still waiting for seats %02x to be ready", sJoin.needReady);
            return;
        }
    }
    if (gBattleControllerExecFlags)
        return;
    MPNet_Log("join: everyone is in, the entrance starts (entering %02x)", sJoin.entering);
    sJoin.state = MJS_ENTRANCE;
    BattleScriptExecute(BattleScript_MpJoinEntrance);
}

// ---------------------------------------------------------------------------
// Entrance (BattleScript_MpJoinEntrance, battle_script_commands.c Cmd_mpjoin)

// MPJOIN_NEXT_PLAYER / MPJOIN_NEXT_FOE: the next battler coming in (gBattleScripting.battler), FALSE when none
bool8 MP_Join_NextEntering(bool8 foes)
{
    u8 b;
    for (b = 0; b < gBattlersCount; b++)
    {
        if (!(sJoin.entering & (1 << b)) || (GetBattlerSide(b) == B_SIDE_OPPONENT) != foes)
            continue;
        sJoin.entering &= ~(1 << b);
        gBattleScripting.battler = b;
        gActiveBattler = b;
        gCurrentMove = MOVE_NONE;   // switchindataupdate keeps the old stat stages after a BATON PASS
        *(gBattleStruct->monToSwitchIntoId + b) = gBattlerPartyIndexes[b];
        MPNet_Log("join: battler %d comes in (party slot %d)", b, gBattlerPartyIndexes[b]);
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// A partner already in the battle (a co-op double becoming a triple)

static void PartnerUpdate(void)
{
    switch (sJoin.pstate)
    {
    case PJS_WAIT_IDLE:
        if (!gMain.inBattle || !gMpCoopBattle)
        {
            sJoin.pstate = PJS_IDLE;
            return;
        }
        if (gMain.callback2 != BattleMainCB2)
            return;
        if ((gBattleControllerExecFlags != 0 || !MP_Battle_LinkDrained() || !MP_BattleLinkBuffersEmpty())
         && ++sJoin.ptimer < PARTNER_IDLE_FRAMES)
            return;
        if (sJoin.ptimer >= PARTNER_IDLE_FRAMES)
            MPNet_Log("join: our screen never went idle (exec %08x), joining anyway", gBattleControllerExecFlags);
        {
            struct MpJoinSnapshot *snap = (struct MpJoinSnapshot *)sSnapBuf;
            const struct Pokemon *parties = (const struct Pokemon *)(sSnapBuf + sizeof(struct MpJoinSnapshot));
            MP_Coop_JoinApplyPartner(&snap->plan, snap->seat, parties, parties + PARTY_SIZE, sOldSeatMons);
            sJoin.entering = snap->plan.entering;
            gMpJoinedBattle = TRUE;
            MP_Battle_JoinLink(snap->plan.seatPlayer, snap->plan.count, snap->seat, 0, FALSE);
            SaveLinkPlayers(snap->plan.count);
            gBattleScripting.multiplayerId = GetMultiplayerId();
        }
        sJoin.pfadeOn = FALSE;
        sJoin.pstate = PJS_FADE;
        // fall through
    case PJS_FADE:
        // behind black (see MJS_FADE)
        if (!sJoin.pfadeOn)
        {
            if (BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK))
                sJoin.pfadeOn = TRUE;
            break;
        }
        if (!gPaletteFade.active)
        {
            MPNet_Log("join: rebuilding the screen behind black (fade %d)", gPaletteFade.y);
            ReshowBattleScreenAfterMenu();
            sJoin.pstate = PJS_REBUILD;
        }
        break;
    case PJS_REBUILD:
        if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
        {
            u8 buf[1] = {JOINM_READY};
            Send(MP_Coop_SeatPlayer(0), buf, 1);
            sJoin.entering = 0;
            sJoin.pstate = PJS_IDLE;
            MPNet_Log("join: our screen is rebuilt for %d players", MP_Coop_SeatCount());
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// The friend: field side

#define tState data[0]

static const char *CantText(u8 reason)
{
    switch (reason)
    {
    case JOIN_CANT_STORY: return "%s's battle is an important one.\nNobody can join it.";
    case JOIN_CANT_FULL:  return "%s's battle is full.";
    case JOIN_CANT_KIND:  return "%s's battle can't be joined.";
    case JOIN_CANT_BUSY:  return "%s is busy letting someone\nelse in. Try again in a moment.";
    default:              return "%s's battle is already over.";
    }
}

static void FriendMessage(const char *fmt, u8 who)
{
    char name[16], text[160];
    NameAscii(name, who);
    snprintf(text, sizeof(text), fmt, name);
    MP_AsciiToGame(sJoin.fmsg, text, sizeof(sJoin.fmsg) - 1);
    HideMapNamePopUpWindow();
    MP_ShowFieldMessageFitted(sJoin.fmsg);
}

static void FriendDone(u8 taskId)
{
    sJoin.fstate = FJS_NONE;
    sJoin.target = -1;
    DestroyTask(taskId);
    if (sJoin.fromScript)
    {
        // not let in: the trainer's script goes on to our own battle with them
        sJoin.fromScript = FALSE;
        MPNet_Log("join: not joined, battling the trainer alone");
        ScriptContext_Enable();
        return;
    }
    UnlockPlayerFieldControls();
}

static void FriendShowEnd(u8 taskId, const char *fmt, u8 who)
{
    FriendMessage(fmt, who);
    sJoin.fstate = FJS_ANSWERED;
    gTasks[taskId].tState = 1;
}

static void CB2_JoinWaitState(void);

static void Task_JoinFriend(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:   // asking / waiting
        if (sJoin.fstate == FJS_GOING)
        {
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            {
                // the battle's music (as the battle will pick it: battle_main.c keeps it playing)
                u32 flags = gBattleTypeFlags;
                bool8 wild = gMpCoopWild;
                gBattleTypeFlags = sJoin.wild ? 0 : BATTLE_TYPE_TRAINER;
                gMpCoopWild = FALSE;
                PlayBattleBGM();
                gBattleTypeFlags = flags;
                gMpCoopWild = wild;
            }
            BattleTransition_StartOnField(sJoin.wild ? MP_GetWildBattleTransition() : MP_GetTrainerBattleTransition());
            tState = 2;
            break;
        }
        if (sJoin.fstate == FJS_ANSWERED)
        {
            tState = 1;
            break;
        }
        if (!PlayerThere(sJoin.target) || !(gMpPlayers[sJoin.target].state.flags & MPF_IN_BATTLE))
        {
            FriendShowEnd(taskId, "%s's battle is already over.", sJoin.target);
            break;
        }
        if (sJoin.fstate == FJS_ASKING && ++sJoin.ftimer > ASK_ANSWER_FRAMES)
        {
            u8 buf[1] = {JOINM_CANCEL};
            Send(sJoin.target, buf, 1);
            FriendShowEnd(taskId, "No answer from %s.", sJoin.target);
            break;
        }
        if (JOY_NEW(B_BUTTON) && !IsTextPrinterActive(0))
        {
            u8 buf[1] = {JOINM_CANCEL};
            Send(sJoin.target, buf, 1);
            MPNet_Log("join: we stopped asking");
            HideFieldMessageBox();
            FriendDone(taskId);
        }
        break;
    case 1:   // a NO / CAN'T message: close it
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            FriendDone(taskId);
        }
        break;
    case 2:   // YES: into the battle
        if (IsBattleTransitionDone())
        {
            CleanupOverworldWindowsAndTilemaps();
            sJoin.ftimer = 0;
            SetMainCallback2(CB2_JoinWaitState);
            if (!sJoin.fromScript)
                UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
}

#undef tState

// The friend's party for the battle: their CO-OP TEAM first, then the first that can battle
static void SendParty(u8 to, u8 seatSize)
{
    u8 *buf = Alloc(4 + sizeof(struct LinkPlayer) + sizeof(struct Pokemon) * MULTI_PARTY_SIZE);
    u16 len = 0, pct = MP_ExpBonusPercent();
    struct LinkPlayer lp;
    u8 i;

    sJoin.nPicks = MP_Coop_JoinerPick(seatSize, sJoin.picks);
    buf[len++] = JOINM_PARTY;
    buf[len++] = sJoin.nPicks;
    buf[len++] = pct & 0xFF;
    buf[len++] = pct >> 8;
    NetLink_FillLocalLinkPlayer(&lp);
    memcpy(buf + len, &lp, sizeof(lp));
    len += sizeof(lp);
    for (i = 0; i < sJoin.nPicks; i++)
    {
        memcpy(buf + len, &gPlayerParty[sJoin.picks[i]], sizeof(struct Pokemon));
        len += sizeof(struct Pokemon);
    }
    Send(to, buf, len);
    Free(buf);
    MPNet_Log("join: sent our %d Pokémon to player %d", sJoin.nPicks, to + 1);
}

// mp_spectate.c: JOIN picked in front of a battling player
void MP_Join_Ask(u8 target)
{
    u8 slots[MULTI_PARTY_SIZE], buf[1] = {JOINM_ASK}, taskId;
    if (MP_Coop_JoinerPick(1, slots) == 0)
    {
        MP_ShowNotice("You have no MOéMON that can battle.");
        return;
    }
    if (FuncIsActiveTask(Task_JoinFriend))
        return;
    sJoin.target = target;
    sJoin.fstate = FJS_ASKING;
    sJoin.ftimer = 0;
    sJoin.gotState = FALSE;
    sJoin.aborted = FALSE;
    Send(target, buf, 1);
    MPNet_Log("join: asking player %d to let us join", target + 1);
    if (!sJoin.fromScript)
        LockPlayerFieldControls();
    FriendMessage("Asking %s to let you\njoin… ({B_BUTTON} never mind)", target);
    taskId = CreateTask(Task_JoinFriend, 80);
    gTasks[taskId].data[0] = 0;
}

// mp_coop.c: JOIN BATTLE picked when a trainer battling this friend spotted us. The trainer's script waits (waitstate):
// let in, it goes on after the battle (MP_Coop_TakeJoinedOther); not, it goes on to our own battle with the trainer.
// FALSE: couldn't ask (the script goes on at once)
bool8 MP_Join_AskFromScript(u8 target)
{
    if (FuncIsActiveTask(Task_JoinFriend) || sJoin.fstate != FJS_NONE)
        return FALSE;
    sJoin.fromScript = TRUE;
    MP_Join_Ask(target);
    if (!FuncIsActiveTask(Task_JoinFriend))
    {
        sJoin.fromScript = FALSE;
        return FALSE;
    }
    return TRUE;
}

// mp_coop.c: can this battle be joined now (a friend its trainer spots may choose to)
bool8 MP_Join_CanBeJoined(void)
{
    return JoinableReason() == 0xFF;
}

bool8 MP_Join_IsAsking(void)
{
    return sJoin.fstate != FJS_NONE;
}

u8 MP_Join_PendingCount(void)
{
    return sJoin.pendingCount;
}

bool8 MP_Join_PromptUp(void)
{
    return sJoin.state == MJS_PROMPT_YESNO;
}

static void OnAnswer(u8 from, const u8 *data, u16 len)
{
    u8 kind, arg;
    if (len < 6 || (sJoin.fstate != FJS_ASKING && sJoin.fstate != FJS_WAITING) || from != (u8)sJoin.target)
        return;
    kind = data[1];
    arg = data[2];
    switch (kind)
    {
    case ANS_WAIT:
        if (sJoin.fstate == FJS_ASKING)
        {
            sJoin.fstate = FJS_WAITING;
            FriendMessage("%s will answer at the start\nof the next turn… ({B_BUTTON} never mind)", from);
        }
        break;
    case ANS_REDIRECT:
        if (arg < MP_MAX_PLAYERS && arg != gMpLocalId)
        {
            u8 buf[1] = {JOINM_ASK};
            sJoin.target = arg;
            sJoin.ftimer = 0;
            Send(arg, buf, 1);
            MPNet_Log("join: asking the battle's leader, player %d", arg + 1);
        }
        break;
    case ANS_NO:
        MPNet_Log("join: player %d said no", from + 1);
        FriendMessage("%s has declined your\nrequest to join them.", from);
        sJoin.fstate = FJS_ANSWERED;
        break;
    case ANS_CANT:
        MPNet_Log("join: player %d's battle can't be joined (%d)", from + 1, arg);
        FriendMessage(CantText(arg), from);
        sJoin.fstate = FJS_ANSWERED;
        break;
    case ANS_YES:
        if ((arg != 2 && arg != 3) || len < 6)
            return;
        MPNet_Log("join: player %d said yes (%d each, %d players%s)", from + 1, arg, data[3] & 0x7F, (data[3] & 0x80) ? ", wild" : "");
        sJoin.seatSize = arg;
        sJoin.wild = (data[3] & 0x80) != 0;
        gTrainerBattleOpponent_A = sJoin.wild ? 0 : (data[4] | (data[5] << 8));
        SendParty(from, arg);
        sJoin.fstate = FJS_GOING;
        break;
    }
}

// After the transition: wait for the battle as it stands, then go in
static void CB2_JoinWaitState(void)
{
    // (a black screen: the field's tasks wait, CB2_MpJoinBattle resets them like any battle start)
    if (sJoin.gotState)
    {
        struct MpJoinSnapshot *snap = (struct MpJoinSnapshot *)sSnapBuf;
        const struct Pokemon *parties = (const struct Pokemon *)(sSnapBuf + sizeof(struct MpJoinSnapshot));
        sJoin.gotState = FALSE;
        sJoin.fstate = FJS_NONE;
        MP_Coop_JoinApplyJoiner(&snap->plan, snap->seat, sJoin.picks, sJoin.nPicks,
                                snap->haveLoseText ? snap->loseText : NULL, snap->haveLoseTextB ? snap->loseTextB : NULL);
        memcpy(gPlayerParty, parties, sizeof(struct Pokemon) * PARTY_SIZE);
        memcpy(gEnemyParty, parties + PARTY_SIZE, sizeof(struct Pokemon) * PARTY_SIZE);
        sJoin.entering = snap->plan.entering;
        gMpJoinedBattle = TRUE;
        if (sJoin.fromScript)
            MP_Coop_JoinedFromTrainerScript();   // (the trainer's script goes on after the battle)
        sJoin.fromScript = FALSE;
        SetMainCallback2(CB2_MpJoinBattle);
        return;
    }
    if (sJoin.aborted || ++sJoin.ftimer > STATE_WAIT_FRAMES || !PlayerThere(sJoin.target))
    {
        char name[16], text[96];
        NameAscii(name, sJoin.target);
        snprintf(text, sizeof(text), "Couldn't join %s's battle.", name);
        MPNet_Log("join: no battle came (%s)", sJoin.aborted ? "called off" : "timed out");
        sJoin.fstate = FJS_NONE;
        sJoin.aborted = FALSE;
        MP_Events_QueueToast(text);
        Overworld_ResetMapMusic();
        if (sJoin.fromScript)
        {
            // the trainer's script goes on to our own battle with them
            sJoin.fromScript = FALSE;
            SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
            return;
        }
        SetMainCallback2(CB2_ReturnToField);
    }
}

// battle_main.c CB2_MpJoinBattle: the battle as the master sent it
void MP_Join_ApplySnapshot(void)
{
    struct MpJoinSnapshot *snap = (struct MpJoinSnapshot *)sSnapBuf;
    u8 b;

    gBattleEnvironment = snap->plan.environment;
    gTrainerBattleOpponent_A = snap->trainerId;
    gTrainerBattleOpponent_B = snap->plan.trainerIdB;   // two trainers: the second
    MP_Coop_JoinSetBattlers(&snap->plan, snap->seat);
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
    {
        gBattleMonForms[b] = snap->battleMonForms[b];
        gBattleSpritesDataPtr->battlerData[b] = snap->spriteInfo[b];
        gBattleMons[b] = snap->battleMons[b];
    }
    gSideStatuses[0] = snap->sideStatuses[0];
    gSideStatuses[1] = snap->sideStatuses[1];
    gBattleWeather = snap->plan.weather;
    MP_BattleWeather_TurnStart();   // (the battle has its own weather by now: never the field's)
    MP_Battle_JoinLink(snap->plan.seatPlayer, snap->plan.count, snap->seat, snap->syncSeq, TRUE);
    SaveLinkPlayers(snap->plan.count);
    gBattleScripting.multiplayerId = GetMultiplayerId();
}

// battle_main.c, once the screen is up: tell the master
static void Task_JoinerReady(u8 taskId)
{
    if (gMain.callback2 != BattleMainCB2 || gPaletteFade.active)
        return;
    {
        u8 buf[1] = {JOINM_READY};
        Send(MP_Coop_SeatPlayer(0), buf, 1);
    }
    sJoin.entering = 0;
    MPNet_Log("join: we're in the battle (seat %d of %d)", MP_Coop_Seat(), MP_Coop_SeatCount());
    DestroyTask(taskId);
}

void MP_Join_CreateReadyTask(void)
{
    CreateTask(Task_JoinerReady, 0);
}

// ---------------------------------------------------------------------------
// Messages

void MP_Join_OnRelay(u8 from, const u8 *data, u16 len)
{
    s8 k;
    switch (data[0])
    {
    case JOINM_ASK:
        OnAsk(from);
        break;
    case JOINM_CANCEL:
        MPNet_Log("join: player %d stopped asking", from + 1);
        RemovePending(from);
        k = AskingIndex(from);
        if (k >= 0 && (sJoin.state == MJS_PROMPT_PRINT || sJoin.state == MJS_PROMPT_YESNO || sJoin.state == MJS_WAIT_PARTY))
        {
            // they gave up while we were asking: the prompt goes on for the other one, or closes
            u8 i;
            for (i = k; i + 1 < sJoin.askingCount; i++)
            {
                sJoin.asking[i] = sJoin.asking[i + 1];
                sJoin.gotParty[i] = sJoin.gotParty[i + 1];
                sJoin.partyCount[i] = sJoin.partyCount[i + 1];
                sJoin.partyBonus[i] = sJoin.partyBonus[i + 1];
                sJoin.partyLink[i] = sJoin.partyLink[i + 1];
                memcpy(sJoin.partyMons[i], sJoin.partyMons[i + 1], sizeof(sJoin.partyMons[i]));
            }
            sJoin.askingCount--;
            if (sJoin.askingCount == 0 && sJoin.state != MJS_WAIT_PARTY)
            {
                if (sJoin.state == MJS_PROMPT_YESNO)
                    HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
                ClearMessage();
                sJoin.state = MJS_IDLE;
                sJoin.turnStart = FALSE;
            }
            else if (sJoin.askingCount > 0 && sJoin.state != MJS_WAIT_PARTY)
            {
                if (sJoin.state == MJS_PROMPT_YESNO)
                    HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
                ShowPrompt();
                sJoin.state = MJS_PROMPT_PRINT;
            }
        }
        break;
    case JOINM_ANSWER:
        OnAnswer(from, data, len);
        break;
    case JOINM_PARTY:
    {
        u8 n;
        k = AskingIndex(from);
        if (k < 0 || sJoin.state != MJS_WAIT_PARTY || len < 4 + sizeof(struct LinkPlayer))
            break;
        n = data[1];
        if (n == 0 || n > MULTI_PARTY_SIZE || len < 4 + sizeof(struct LinkPlayer) + sizeof(struct Pokemon) * n)
            break;
        sJoin.partyCount[k] = n;
        sJoin.partyBonus[k] = data[2] | (data[3] << 8);
        memcpy(&sJoin.partyLink[k], data + 4, sizeof(struct LinkPlayer));
        memset(sJoin.partyMons[k], 0, sizeof(sJoin.partyMons[k]));
        memcpy(sJoin.partyMons[k], data + 4 + sizeof(struct LinkPlayer), sizeof(struct Pokemon) * n);
        sJoin.gotParty[k] = TRUE;
        MPNet_Log("join: player %d brings %d Pokémon", from + 1, n);
        if (sJoin.state == MJS_WAIT_PARTY)
            ShowWaiting();
        break;
    }
    case JOINM_STATE:
        if (len < 1 + sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * PARTY_SIZE * 2
         || from != (u8)sJoin.target || (sJoin.fstate != FJS_GOING && gMain.callback2 != CB2_JoinWaitState))
            break;
        memcpy(sSnapBuf, data + 1, sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * PARTY_SIZE * 2);
        sJoin.gotState = TRUE;
        MPNet_Log("join: got the battle (seat %d of %d)", ((struct MpJoinSnapshot *)sSnapBuf)->seat,
                  ((struct MpJoinSnapshot *)sSnapBuf)->plan.count);
        break;
    case JOINM_CONVERT:
    {
        u16 need = 1 + sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * (PARTY_SIZE * 2 + MULTI_PARTY_SIZE);
        if (len < need || !gMpCoopBattle || MP_Coop_IsMaster() || from != MP_Coop_SeatPlayer(0))
            break;
        memcpy(sSnapBuf, data + 1, sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * PARTY_SIZE * 2);
        memcpy(sOldSeatMons, data + 1 + sizeof(struct MpJoinSnapshot) + sizeof(struct Pokemon) * PARTY_SIZE * 2,
               sizeof(sOldSeatMons));
        sJoin.pstate = PJS_WAIT_IDLE;
        sJoin.ptimer = 0;
        MPNet_Log("join: the battle grows to %d players", ((struct MpJoinSnapshot *)sSnapBuf)->plan.count);
        break;
    }
    case JOINM_READY:
    {
        u8 seat;
        if (!gMpCoopBattle || !MP_Coop_IsMaster())
            break;
        for (seat = 1; seat < MP_Coop_SeatCount(); seat++)
        {
            if (MP_Coop_SeatPlayer(seat) == from)
            {
                sJoin.needReady &= ~(1 << seat);
                MPNet_Log("join: seat %d is ready", seat);
            }
        }
        break;
    }
    case JOINM_ABORT:
        if (from == (u8)sJoin.target && (sJoin.fstate == FJS_GOING || gMain.callback2 == CB2_JoinWaitState))
            sJoin.aborted = TRUE;
        break;
    }
}

void MP_Join_OnPlayerLeft(u8 id)
{
    RemovePending(id);
    if (sJoin.fstate != FJS_NONE && id == (u8)sJoin.target)
        ;   // Task_JoinFriend / CB2_JoinWaitState notice
}

// Every frame (mp_main.c)
void MP_Join_Update(void)
{
    u8 i;
    // a battle that ended (or can't take anyone any more) with friends still asking
    if (sJoin.pendingCount != 0 && sJoin.state == MJS_IDLE)
    {
        u8 reason = JoinableReason();
        if (reason != 0xFF && reason != JOIN_CANT_FULL)
        {
            for (i = 0; i < sJoin.pendingCount; i++)
                if (PlayerThere(sJoin.pending[i]))
                    SendAnswer(sJoin.pending[i], ANS_CANT, reason, 0);
            sJoin.pendingCount = 0;
            sJoin.turnStart = FALSE;
        }
    }
    if (!gMain.inBattle)
        sJoin.declined = 0;
    if (!gMain.inBattle && sJoin.state != MJS_IDLE)
    {
        sJoin.state = MJS_IDLE;
        sJoin.askingCount = 0;
        sJoin.turnStart = FALSE;
        sJoin.entering = 0;
    }
    if (sJoin.pstate != PJS_IDLE)
        PartnerUpdate();
}

// The battle is over (mp_coop.c FinishLink, or called off)
void MP_Join_OnBattleEnd(void)
{
    gMpJoinedBattle = FALSE;
    sJoin.entering = 0;
    sJoin.state = MJS_IDLE;
    sJoin.pstate = PJS_IDLE;
    sJoin.askingCount = 0;
    sJoin.turnStart = FALSE;
}
