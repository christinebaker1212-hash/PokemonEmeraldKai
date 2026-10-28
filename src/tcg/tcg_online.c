// Moemon Kai: card duels and card trades with friends. Ask a friend (the CARD
// CASE's button in front of them, or CO-OP > FRIENDS > their name > CARD DUEL /
// CARD TRADE); they get a YES / NO on their own map, like a trade. A yes opens
// the card screens on both games: each picks a deck (the asker's duel invite
// carries theirs, src/tcg/tcg_link.c), or each offers a card and both confirm.
// A friend who is busy or has no CARD CASE answers at once, so nobody waits.
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "item.h"
#include "main.h"
#include "map_name_popup.h"
#include "overworld.h"
#include "menu.h"
#include "mp.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "tcg/integration.h"
#include "tcg/online.h"

enum
{
    REQUEST = 0x10,   // kind
    REPLY,            // answer (TCG_ANSWER_*)
    WITHDRAW,         // the asker gave up waiting
    OFFER,            // trade: the card offered (u16, 0 none)
    CONFIRM,          // trade: mine (u16), theirs (u16)
    TRADE_CANCEL,
};

static struct TcgSocial sSocial;

const struct TcgSocial *Tcg_Social(void)
{
    return &sSocial;
}

static void Name(unsigned id, char *out, int size)
{
    if (id < MP_MAX_PLAYERS && gMpPlayers[id].present)
        MP_GameToAscii(out, gMpPlayers[id].name, size);
    else
        snprintf(out, size, "Your friend");
}

static void Send(unsigned type, const u8 *body, unsigned len)
{
    u8 p[16];
    p[0] = TCG_LINK_RELAY;
    p[1] = TCG_LINK_VERSION;
    p[2] = type;
    p[3] = sSocial.session;
    p[4] = sSocial.session >> 8;
    p[5] = sSocial.session >> 16;
    p[6] = sSocial.session >> 24;
    if (len)
        memcpy(p + 7, body, len);
    MP_SendRelay(sSocial.peer, p, 7 + len);
}

static void Reset(void)
{
    memset(&sSocial, 0, sizeof(sSocial));
    sSocial.peer = 0xFF;
}

// ---------------------------------------------------------------------------
// Whether a friend can be asked

static bool8 HasCase(void)
{
    return Tcg_GetCollection()->optedIn && CheckBagHasItem(ITEM_CARD_CASE, 1);
}

const char *Tcg_CanAskFriend(unsigned id)
{
    if (!MP_IsConnected() || id >= MP_MAX_PLAYERS || !gMpPlayers[id].present || (int)id == gMpLocalId)
        return "They aren't online right now.";
    if (!HasCase())
        return "You need a CARD CASE first: ask at the CARD CLUB in a POKeMON CENTER.";
    if (gMain.callback2 != CB2_Overworld)
        return "Head outside first.";
    if (sSocial.role != TCG_SOCIAL_NONE)
        return "You're already asking someone.";
    return NULL;
}

// ---------------------------------------------------------------------------
// Asking (the field: a message until they answer)

#define tState data[0]
#define tTimer data[1]

static u8 sText[160];

static void SayAscii(const char *ascii)
{
    MP_AsciiToGame(sText, ascii, sizeof(sText) - 1);
    HideMapNamePopUpWindow();
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
    MP_ShowFieldMessageFitted(sText);
}

static void Release(u8 taskId)
{
    HideFieldMessageBox();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

static void OpenTogether(void)
{
    // A yes: both games open the card screens
    sSocial.role = sSocial.kind == TCG_ASK_DUEL ? TCG_SOCIAL_DECK : TCG_SOCIAL_TRADE;
    sSocial.deadline = SDL_GetTicks() + 300000;
    if (sSocial.kind == TCG_ASK_DUEL)
        Tcg_OpenOnlineDeck();
    else
        Tcg_OpenOnlineTrade();
}

static void SayWaiting(void)
{
    char line[96], name[16];
    Name(sSocial.peer, name, sizeof(name));
    snprintf(line, sizeof(line), sSocial.kind == TCG_ASK_DUEL ? "You asked %s for a card duel...\nB: stop waiting"
                                                              : "You asked %s for a card trade...\nB: stop waiting", name);
    SayAscii(line);
}

static void Withdraw(u8 taskId, bool8 gaveUp)
{
    char line[96], name[16];
    Send(WITHDRAW, NULL, 0);
    Name(sSocial.peer, name, sizeof(name));
    snprintf(line, sizeof(line), gaveUp ? "You stopped waiting." : "%s didn't answer. They may be busy.", name);
    Reset();
    PlaySE(SE_SELECT);
    SayAscii(line);
    gTasks[taskId].tState = 2;
}

static void Task_Ask(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    char line[96], name[16];
    switch (tState)
    {
    case 0:
        if (sSocial.answer == TCG_ANSWER_WAITING)
        {
            if ((s32)(SDL_GetTicks() - sSocial.deadline) >= 0 || !gMpPlayers[sSocial.peer].present)
                Withdraw(taskId, FALSE);
            else if (JOY_NEW(B_BUTTON))
            {
                // (B moves text along everywhere else: asked first, so a tap meant for the message doesn't call it off)
                Name(sSocial.peer, name, sizeof(name));
                snprintf(line, sizeof(line), "Stop waiting for %s?", name);
                PlaySE(SE_SELECT);
                SayAscii(line);
                tState = 3;
            }
            break;
        }
        Name(sSocial.peer, name, sizeof(name));
        if (sSocial.answer == TCG_ANSWER_YES)
        {
            HideFieldMessageBox();
            PlaySE(SE_SUCCESS);
            OpenTogether();
            tState = 1;
            break;
        }
        snprintf(line, sizeof(line), sSocial.answer == TCG_ANSWER_BUSY   ? "%s is busy right now. Try again in a moment."
                                     : sSocial.answer == TCG_ANSWER_NOCASE ? "%s doesn't have a CARD CASE yet."
                                                                           : "%s said no this time.", name);
        Reset();
        PlaySE(SE_FAILURE);
        SayAscii(line);
        tState = 2;
        break;
    case 1:
        // (the card screens run instead of the game until they close)
        if (!Tcg_IsActive())
        {
            Reset();
            Release(taskId);
        }
        break;
    case 2:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            Release(taskId);
        }
        break;
    case 3:
        // Stop waiting? (their answer may still come in meanwhile)
        if (sSocial.answer != TCG_ANSWER_WAITING)
            tState = 0;
        else if (!IsTextPrinterActive(0))
        {
            DisplayYesNoMenuWithDefault(1);
            tState = 4;
        }
        break;
    case 4:
    {
        s8 answer;
        if (sSocial.answer != TCG_ANSWER_WAITING || (s32)(SDL_GetTicks() - sSocial.deadline) >= 0)
        {
            EraseYesNoWindow();
            tState = 0;
            break;
        }
        answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        if (answer == 0)
            Withdraw(taskId, TRUE);
        else
        {
            SayWaiting();
            tState = 0;
        }
        break;
    }
    }
}

void Tcg_AskFriend(unsigned id, unsigned kind)
{
    u8 body = (u8)kind, taskId;
    Reset();
    sSocial.role = TCG_SOCIAL_ASKING;
    sSocial.kind = kind;
    sSocial.peer = id;
    sSocial.seat = 0;
    sSocial.session = Tcg_Random(&Tcg_GetCollection()->rng) | 1;
    sSocial.answer = TCG_ANSWER_WAITING;
    sSocial.deadline = SDL_GetTicks() + 30000;
    Send(REQUEST, &body, 1);
    if (!ArePlayerFieldControlsLocked())
        LockPlayerFieldControls();
    FreezeObjectEvents();
    SayWaiting();
    taskId = CreateTask(Task_Ask, 80);
    gTasks[taskId].tState = 0;
    printf("[tcg] asked player %u for a card %s\n", id, kind == TCG_ASK_DUEL ? "duel" : "trade");
}

// The CARD CASE's button in front of a friend
int Tcg_FieldAskFriend(void)
{
    s16 x, y, rx, ry;
    unsigned i;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        const char *why;
        if ((int)i == gMpLocalId || !gMpPlayers[i].present || !MP_GetRemoteMapPos(i, &rx, &ry) || rx != x || ry != y)
            continue;
        why = Tcg_CanAskFriend(i);
        if (why)
        {
            MP_ShowNotice(why);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            return 1;
        }
        Tcg_AskFriend(i, TCG_ASK_DUEL);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Being asked (their message, YES / NO, on our map)

static void Task_Asked(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    char line[96], name[16];
    u8 answer;
    switch (tState)
    {
    case 0:
        Name(sSocial.peer, name, sizeof(name));
        snprintf(line, sizeof(line), sSocial.kind == TCG_ASK_DUEL ? "%s wants to have a card duel!\nWill you play?"
                                                                  : "%s wants to trade a card!\nWill you trade?", name);
        PlaySE(SE_POKENAV_CALL);
        SayAscii(line);
        tState++;
        break;
    case 1:
        if (sSocial.role != TCG_SOCIAL_ASKED)
        {
            // They stopped waiting
            Name(sSocial.peer, name, sizeof(name));
            snprintf(line, sizeof(line), "%s stopped waiting.", name);
            SayAscii(line);
            tState = 4;
            break;
        }
        if (!IsTextPrinterActive(0))
        {
            DisplayYesNoMenuDefaultYes();
            tState++;
        }
        break;
    case 2:
        if (sSocial.role != TCG_SOCIAL_ASKED)
        {
            EraseYesNoWindow();
            tState = 1;
            break;
        }
        answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == (u8)MENU_NOTHING_CHOSEN)
            break;
        if (answer == 0)
        {
            u8 yes = TCG_ANSWER_YES;
            Send(REPLY, &yes, 1);
            HideFieldMessageBox();
            OpenTogether();
            tState = 3;
        }
        else
        {
            u8 no = TCG_ANSWER_NO;
            Send(REPLY, &no, 1);
            Reset();
            Release(taskId);
        }
        break;
    case 3:
        if (!Tcg_IsActive())
        {
            Reset();
            Release(taskId);
        }
        break;
    case 4:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            Reset();
            Release(taskId);
        }
        break;
    }
}

#undef tState
#undef tTimer

// Every frame (mp_main.c): a request waiting for us to be free, and timeouts
void Tcg_SocialTick(void)
{
    if (sSocial.role == TCG_SOCIAL_INCOMING)
    {
        if (!gMpPlayers[sSocial.peer].present)
            Reset();
        else if (MP_IsLocalFree() && !FuncIsActiveTask(Task_Asked))
        {
            sSocial.role = TCG_SOCIAL_ASKED;
            LockPlayerFieldControls();
            FreezeObjectEvents();
            PlayerFreeze();
            StopPlayerAvatar();
            CreateTask(Task_Asked, 80);
        }
        else if ((s32)(SDL_GetTicks() - sSocial.deadline) >= 0)
        {
            u8 busy = TCG_ANSWER_BUSY;
            Send(REPLY, &busy, 1);
            Reset();
        }
    }
    if ((sSocial.role == TCG_SOCIAL_DECK || sSocial.role == TCG_SOCIAL_TRADE) && !gMpPlayers[sSocial.peer].present)
        sSocial.peerLeft = 1;
}

void Tcg_SocialReceive(unsigned from, const u8 *p, unsigned len)
{
    unsigned type = p[2];
    u32 session = p[3] | p[4] << 8 | p[5] << 16 | (u32)p[6] << 24;
    if (type == REQUEST && len == 8 && sSocial.role == TCG_SOCIAL_ASKING && from == sSocial.peer
     && sSocial.answer == TCG_ANSWER_WAITING && (p[7] ? TCG_ASK_TRADE : TCG_ASK_DUEL) == sSocial.kind)
    {
        // We asked each other at once: the request of the one first in the room stands, and the other says yes to it
        if ((int)from < gMpLocalId)
        {
            u8 yes = TCG_ANSWER_YES;
            sSocial.session = session;
            sSocial.seat = 1;
            Send(REPLY, &yes, 1);
            sSocial.answer = TCG_ANSWER_YES;
            printf("[tcg] asked each other: playing on %u's request\n", from);
        }
        return;
    }
    if (type == REQUEST && len == 8)
    {
        u8 answer = 0xFF;
        if (sSocial.role != TCG_SOCIAL_NONE || Tcg_IsActive())
            answer = TCG_ANSWER_BUSY;
        else if (!HasCase())
            answer = TCG_ANSWER_NOCASE;
        if (answer != 0xFF)
        {
            // (answered at once, without touching our own state)
            struct TcgSocial keep = sSocial;
            sSocial.peer = from;
            sSocial.session = session;
            Send(REPLY, &answer, 1);
            sSocial = keep;
            return;
        }
        Reset();
        sSocial.role = TCG_SOCIAL_INCOMING;
        sSocial.kind = p[7] ? TCG_ASK_TRADE : TCG_ASK_DUEL;
        sSocial.peer = from;
        sSocial.seat = 1;
        sSocial.session = session;
        // Busy for a few seconds (a battle, a menu): then they're told so
        sSocial.deadline = SDL_GetTicks() + 6000;
        return;
    }
    if (from != sSocial.peer || session != sSocial.session)
        return;
    switch (type)
    {
    case REPLY:
        if (len == 8 && sSocial.role == TCG_SOCIAL_ASKING)
            sSocial.answer = p[7];
        break;
    case WITHDRAW:
        if (sSocial.role == TCG_SOCIAL_INCOMING || sSocial.role == TCG_SOCIAL_ASKED)
            sSocial.role = TCG_SOCIAL_NONE;
        break;
    case OFFER:
        if (len == 9 && sSocial.role == TCG_SOCIAL_TRADE)
        {
            sSocial.theirOffer = p[7] | p[8] << 8;
            sSocial.theirConfirm = 0;
            sSocial.myConfirm = 0;
        }
        break;
    case CONFIRM:
        if (len == 11 && sSocial.role == TCG_SOCIAL_TRADE)
        {
            // Their confirm names the same two cards the other way round
            u16 theirs = p[7] | p[8] << 8, mine = p[9] | p[10] << 8;
            sSocial.theirConfirm = theirs == sSocial.theirOffer && mine == sSocial.myOffer;
        }
        break;
    case TRADE_CANCEL:
        if (sSocial.role == TCG_SOCIAL_TRADE || sSocial.role == TCG_SOCIAL_DECK)
            sSocial.peerLeft = 1;
        break;
    }
}

// Duels: the screens pick a deck; the asker invites with theirs, the other
// accepts with theirs when the invite is in (src/tcg/tcg_link.c)
int Tcg_SocialExpectsInvite(unsigned from)
{
    return sSocial.role == TCG_SOCIAL_DECK && sSocial.seat == 1 && from == sSocial.peer;
}

// Trades
void Tcg_SocialOffer(unsigned card)
{
    u8 body[2] = {card & 0xFF, card >> 8};
    sSocial.myOffer = card;
    sSocial.myConfirm = sSocial.theirConfirm = 0;
    Send(OFFER, body, 2);
}

void Tcg_SocialConfirm(void)
{
    u8 body[4] = {sSocial.myOffer & 0xFF, sSocial.myOffer >> 8, sSocial.theirOffer & 0xFF, sSocial.theirOffer >> 8};
    sSocial.myConfirm = 1;
    Send(CONFIRM, body, 4);
}

void Tcg_SocialLeave(void)
{
    if (sSocial.role == TCG_SOCIAL_TRADE || sSocial.role == TCG_SOCIAL_DECK)
        Send(TRADE_CANCEL, NULL, 0);
    Reset();
}

void Tcg_SocialInit(void)
{
    Reset();
}

// ---------------------------------------------------------------------------
// Tournament matches (CO-OP > TOURNAMENT, card duels: mp_tourney.c)

void Tcg_TourneyDuel(unsigned peer, unsigned seat, uint32_t session)
{
    Reset();
    sSocial.kind = TCG_ASK_DUEL;
    sSocial.peer = peer;
    sSocial.seat = seat;
    sSocial.session = session;
    sSocial.tourney = 1;
    printf("[tcg] tournament match vs player %u, seat %u\n", peer, seat);
    OpenTogether();
}

void Tcg_TourneyResult(int won)
{
    if (!sSocial.tourney)
        return;
    sSocial.tourney = 0;
    printf("[tcg] tournament match %s\n", won ? "won" : "lost");
    MP_Tourney_OnCardResult(won);
}

const char *Tcg_TourneyNotReady(void)
{
    const struct TcgCollection *c = Tcg_GetCollection();
    if (!HasCase())
        return "You need a CARD CASE first: ask at the CARD CLUB in a POKeMON CENTER.";
    if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, NULL, 0))
        return "Your deck in use isn't ready: finish it in the CARD CASE.";
    return NULL;
}
