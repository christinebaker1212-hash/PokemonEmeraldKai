#include "mp_version.h"
#if MOEMON_DEV_BUILD
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include "tcg/integration.h"
#include "tcg_board.h"
#include "tcg_gfx.h"
int Tcg_GrantCase(void);
void Tcg_ResumeGame(void (*callback)(void));

enum
{
    UI_CLOSED,
    UI_INVITE,
    UI_STARTER,
    UI_CASE,
    UI_COLLECTION,
    UI_INSPECT,
    UI_DECKS,
    UI_EDIT,
    UI_PACKS,
    UI_REVEAL,
    UI_DUEL,
    UI_ART,
    UI_DECK_ACTIONS,
    UI_COPY_DECK,
    UI_COPY_CONFIRM,
    UI_ONLINE,
    UI_ONLINE_INVITE,
    UI_ONLINE_WAIT
};
enum
{
    KEY_A = 1,
    KEY_B = 2,
    KEY_SELECT = 4,
    KEY_START = 8,
    KEY_RIGHT = 16,
    KEY_LEFT = 32,
    KEY_UP = 64,
    KEY_DOWN = 128,
    KEY_R = 256,
    KEY_L = 512
};
static struct
{
    int screen, back, cursor, scroll, card, deck, pack, editing, showAll, frame;
    int inspectScroll, inspectMax, zoom, panX, panY, copyTarget, filter, online;
    unsigned previousKeys, age, aiDelay, held, repeat;
    const char *opponent, *opponentDeck;
    unsigned rewardPack;
    void (*returnToGame)(void);
    struct TcgDuel duel;
    uint16_t revealed[10];
    int flipped, revealCursor, autoPlay;
    char message[256];
} U;

static float Ease(float t)
{
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    return 1 - (1 - t) * (1 - t) * (1 - t);
}
static void Message(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(U.message, sizeof(U.message), format, ap);
    va_end(ap);
}
static void Screen(int screen)
{
    U.screen = screen;
    U.cursor = 0;
    U.scroll = 0;
    U.age = 0;
    U.message[0] = 0;
}
int Tcg_IsActive(void) { return U.screen != UI_CLOSED; }
int Tcg_OnlineWilling(void) { return U.screen == UI_ONLINE && Tcg_GetCollection()->optedIn; }
/* The local player is always player 0 on screen: the recipient of an
 * invitation sees the canonical duel with both sides swapped. */
static void OnlineView(void)
{
    struct TcgLink *l = Tcg_Online();
    unsigned i;
    U.duel = l->game;
    if (l->seat)
    {
        struct TcgPlayer temp = U.duel.players[0];
        U.duel.players[0] = U.duel.players[1];
        U.duel.players[1] = temp;
        U.duel.current ^= 1;
        U.duel.first ^= 1;
        if (U.duel.promotePlayer < 2)
            U.duel.promotePlayer ^= 1;
        if (U.duel.winner < 2)
            U.duel.winner ^= 1;
        for (i = 0; i < TCG_EVENT_RING; i++)
            U.duel.events[i].player ^= 1;
    }
}
static void Close(void)
{
    void (*callback)(void) = U.returnToGame;
    if (U.online)
        TcgLink_Cancel(Tcg_Online(), "Card session closed.");
    if (U.screen == UI_DUEL || U.back == UI_DUEL)
        TcgBoard_End();
    U.online = 0;
    Screen(UI_CLOSED);
    U.returnToGame = NULL;
    G_Destroy();
    Tcg_ResumeGame(callback);
}
void Tcg_OpenChallenge(const char *name, const char *deck, unsigned pack)
{
    struct TcgCollection *c = Tcg_GetCollection();
    U.opponent = name;
    U.opponentDeck = deck;
    U.rewardPack = pack;
    U.returnToGame = NULL;
    Screen(c->optedIn ? UI_CASE : UI_INVITE);
    PlaySE(SE_WIN_OPEN);
    printf("[tcg] explicit challenge: %s (%s), opted in %u\n", name, deck, c->optedIn);
}
void Tcg_OpenCase(void (*returnToGame)(void))
{
    struct TcgCollection *c = Tcg_GetCollection();
    U.returnToGame = returnToGame;
    U.opponent = NULL;
    U.opponentDeck = NULL;
    U.rewardPack = 0;
    if (!c->optedIn)
    {
        U.opponent = "Practice Collector";
        U.opponentDeck = "PracticePlayer";
        U.rewardPack = 0;
    }
    Screen(c->optedIn ? UI_CASE : UI_INVITE);
}
void Tcg_DestroyTextures(void) { G_Destroy(); }

/* ------------------------------------------------------------------ */
/* Shared drawing for the menus                                         */
/* ------------------------------------------------------------------ */
static void Backdrop(void)
{
    int x, y;
    G_Fill(0, 0, 240, 160, 0x017a3e);
    for (y = 0; y < 160; y += 16)
        for (x = (y / 16) % 2 * 16; x < 240; x += 32)
            G_FillA(x, y, 16, 16, 0x05904a, 255);
    G_FillA(0, 0, 240, 160, 0x002010, 40);
}
/* A DS-style title bar: a pale gradient with the title on it. */
static void Header(const char *title, const char *right)
{
    int i;
    for (i = 0; i < 18; i++)
        G_FillA(0, i, 240, 1, G_Mix(0xf0f0f0, 0xa8a8b0, i / 17.0f), 255);
    G_FillA(0, 18, 240, 1, 0x505058, 255);
    G_FillA(0, 19, 240, 1, 0x003818, 120);
    G_Text(8, 1, 0x303038, 0xf8f8f8, title, 170, TF_NORMAL);
    if (right)
        G_TextAlign(232, 4, TA_RIGHT, 0x5c6878, G_NO_SHADOW, right, TF_SMALL);
}
static void Footer(const char *text)
{
    G_FillA(0, 148, 240, 12, 0x081c10, 210);
    G_FillA(0, 148, 240, 1, 0x1bc55c, 255);
    G_Hint(6, 148, 0xd0f0d8, G_NO_SHADOW, text, TF_NARROW, 1);
}
static void Option(int index, int x, int y, const char *text, int enabled)
{
    if (index == U.cursor)
        G_Cursor(x - 8, y + 4);
    G_Text(x, y, enabled ? G_INK : 0xa8a8a8, enabled ? G_INK_SHADOW : G_NO_SHADOW, text, 160, TF_NORMAL);
}
static void SmallOption(int index, int x, int y, const char *text, int enabled)
{
    if (index == U.cursor)
        G_Cursor(x - 8, y + 3);
    G_Text(x, y, enabled ? G_INK : 0xa8a8a8, enabled ? G_INK_SHADOW : G_NO_SHADOW, text, 0, TF_SMALL);
}
static void BigCard(unsigned id, int x, int y, int bob)
{
    G_FillA(x + 2, y + 3, 44, 61, 0x002010, 110);
    G_Card(id, (float)x, (float)(y - bob), 44, 61, 0, 255, 0);
}
static unsigned DeckCount(unsigned deck, unsigned card)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, n = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
        n += card ? c->decks[deck].cards[i] == card : c->decks[deck].cards[i] != 0;
    return n;
}
static unsigned DeckMascot(unsigned deck)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, best = 0, hp = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *card = Tcg_Card(c->decks[deck].cards[i]);
        if (card && card->kind == TCG_POKEMON && (card->hp > hp || !best))
            best = card->id, hp = card->hp;
    }
    return best;
}
/* deck_configuration.asm groups Pokemon by type, then Trainers and Energy. */
static const unsigned sFilterTypes[] = {TCG_GRASS, TCG_FIRE, TCG_WATER, TCG_LIGHTNING, TCG_FIGHTING, TCG_PSYCHIC,
                                        TCG_COLORLESS};
static int CardFilter(unsigned id, unsigned filter)
{
    const struct TcgCard *card = Tcg_Card(id);
    if (!card)
        return 0;
    if (filter < 7)
        return card->kind == TCG_POKEMON && card->type == sFilterTypes[filter];
    if (filter == 9)
        return 1;
    return card->kind == (filter == 7 ? TCG_TRAINER : TCG_ENERGY);
}
static void FilterTabs(int y, int selected, int countDeck, int all)
{
    unsigned i;
    struct TcgCollection *c = Tcg_GetCollection();
    for (i = 0; i < (all ? 10u : 9u); i++)
    {
        int x = 4 + (int)i * (all ? 23 : 26), w = all ? 21 : 24;
        unsigned count = 0, j;
        char n[8];
        int on = (int)i == selected;
        G_RoundFill(x, y + (on ? 0 : 2), w, on ? 16 : 14, on ? 0xf8f8f0 : 0x2a6040, 255);
        G_RoundFrame(x, y + (on ? 0 : 2), w, on ? 16 : 14, on ? 0x404040 : 0x1bc55c, 255);
        if (i < 7)
            G_EnergyOrb(x + 2, y + 4, sFilterTypes[i], 1);
        else if (i == 7)
            G_Text(x + 2, y + 1, on ? 0x383838 : 0xd8f0e0, G_NO_SHADOW, "T", 0, TF_NARROW);
        else if (i == 8)
            G_Text(x + 2, y + 1, on ? 0x383838 : 0xd8f0e0, G_NO_SHADOW, "E", 0, TF_NARROW);
        else
            G_Text(x + 2, y + 1, on ? 0x383838 : 0xd8f0e0, G_NO_SHADOW, "*", 0, TF_NARROW);
        if (countDeck)
        {
            for (j = 0; j < TCG_DECK_SIZE; j++)
                count += CardFilter(c->decks[U.deck].cards[j], i);
            snprintf(n, sizeof(n), "%u", count);
            G_Digits(x + w - 2 - G_DigitsWidth(n, 0, 1), y + 6, n, 0, 1, on ? 0x385068 : 0xc0e8d0, 255);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Collection lists                                                     */
/* ------------------------------------------------------------------ */
static int VisibleCard(unsigned id)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (U.screen == UI_EDIT)
        return CardFilter(id, U.filter) && (U.editing ? DeckCount(U.deck, id) != 0 : id <= 6 || c->owned[id] != 0);
    if (!CardFilter(id, U.filter))
        return 0;
    return U.showAll || c->owned[id];
}
static unsigned CollectionCard(unsigned index)
{
    unsigned id;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        if (VisibleCard(id) && !index--)
            return id;
    return 0;
}
static unsigned CollectionSize(void)
{
    unsigned id, n = 0;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        n += VisibleCard(id);
    return n;
}
static void Inspect(unsigned id, int back)
{
    if (!id)
        return;
    U.card = (int)id;
    U.back = back;
    U.inspectScroll = U.inspectMax = U.zoom = U.panX = U.panY = 0;
    U.screen = UI_INSPECT;
    U.age = 0;
    PlaySE(SE_RG_CARD_OPEN);
}

/* ------------------------------------------------------------------ */
/* Duels                                                                */
/* ------------------------------------------------------------------ */
static void Apply(const struct TcgAction *a)
{
    if (U.online)
    {
        if (!TcgLink_Submit(Tcg_Online(), a, SDL_GetTicks()))
            TcgBoard_Reject(Tcg_Online()->error[0] ? Tcg_Online()->error
                                                   : "Wait for your turn and the other player's acknowledgement.");
        else
            OnlineView();
        return;
    }
    if (TcgDuel_Apply(&U.duel, a) != TCG_OK)
        TcgBoard_Reject(U.duel.error[0] ? U.duel.error : "Choose the required cards and targets.");
    U.aiDelay = 12;
}
static void StartDuel(int practice)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i;
    char reason[160];
    const char *name = practice ? "PracticePlayer" : U.opponentDeck;
    if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
    {
        Message("%s", reason);
        return;
    }
    for (i = 0; i < gTcgDeckDefinitionCount; i++)
        if (name && !strcmp(name, gTcgDeckDefinitions[i].name))
            break;
    if (i == gTcgDeckDefinitionCount)
    {
        Message("The opponent's deck is unavailable.");
        return;
    }
    if (!TcgDuel_Init(&U.duel, c->decks[c->activeDeck].cards, gTcgDeckDefinitions[i].cards, Tcg_Random(&c->rng), 6))
    {
        Message("Unable to set up this duel.");
        return;
    }
    if (practice)
        U.opponent = "Practice Collector";
    Screen(UI_DUEL);
    TcgBoard_Begin(&U.duel, U.opponent, 0);
    U.aiDelay = 20;
}
static void DuelStep(unsigned pressed, unsigned keys)
{
    struct TcgAction a;
    unsigned card = 0;
    int r;
    if (U.online)
    {
        OnlineView();
        TcgBoard_SetWaiting(Tcg_Online()->pending);
    }
    TcgBoard_Sync(&U.duel);
    if (!U.online && U.duel.phase == TCG_FINISHED && !U.duel.rewardClaimed)
    {
        U.duel.rewardClaimed = 1;
        if (U.duel.winner == 0)
        {
            Tcg_AwardPack(Tcg_GetCollection(), U.rewardPack);
            printf("[tcg] victory: one booster awarded\n");
            TcgBoard_Result("A booster pack was added to your Card Case!");
        }
        else
            TcgBoard_Result("No cards, money or progress were lost.");
    }
    if (U.online && U.duel.phase == TCG_FINISHED)
        TcgBoard_Result("Online duels don't wager or award cards.");
    if (!U.online && U.duel.phase != TCG_FINISHED &&
        ((U.duel.phase == TCG_PROMOTION ? U.duel.promotePlayer : U.duel.current) == 1 || U.autoPlay) &&
        TcgBoard_Idle())
    {
        if (U.aiDelay)
            U.aiDelay--;
        else
        {
            if (TcgDuel_AiAction(&U.duel, &a) && TcgDuel_Apply(&U.duel, &a) == TCG_OK)
                U.aiDelay = Tcg_GetCollection()->fastAnimations ? 4 : 14;
            else
            {
                /* Never stall: an opponent with nothing legal ends its turn. */
                memset(&a, 0, sizeof(a));
                a.type = U.duel.phase == TCG_SETUP ? TCG_FINISH_SETUP : TCG_PASS;
                if (TcgDuel_Apply(&U.duel, &a) != TCG_OK)
                {
                    a.type = TCG_CONCEDE;
                    TcgDuel_Apply(&U.duel, &a);
                }
                U.aiDelay = 20;
            }
        }
    }
    r = TcgBoard_Step(pressed, keys, &a, &card);
    if (r == BOARD_ACTION || r == BOARD_CONCEDE)
        Apply(&a);
    else if (r == BOARD_INSPECT)
        Inspect(card, UI_DUEL);
    else if (r == BOARD_LEAVE)
    {
        if (U.online)
        {
            TcgLink_Cancel(Tcg_Online(), "Duel complete.");
            U.online = 0;
        }
        TcgBoard_End();
        Screen(UI_CASE);
    }
}

/* ------------------------------------------------------------------ */
/* Menus                                                                */
/* ------------------------------------------------------------------ */
static int MenuSize(void)
{
    switch (U.screen)
    {
    case UI_INVITE:
        return 2;
    case UI_STARTER:
        return 3;
    case UI_CASE:
        return 9;
    case UI_ONLINE:
        return MP_MAX_PLAYERS;
    case UI_ONLINE_INVITE:
        return 2;
    case UI_COLLECTION:
    case UI_EDIT:
        return CollectionSize();
    case UI_DECK_ACTIONS:
        return 3;
    case UI_COPY_CONFIRM:
        return 2;
    case UI_DECKS:
    case UI_COPY_DECK:
        return TCG_SAVED_DECKS;
    case UI_PACKS:
        return 4;
    case UI_REVEAL:
        return 10;
    default:
        return 1;
    }
}
static void Choose(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned id;
    char reason[160];
    switch (U.screen)
    {
    case UI_INVITE:
        if (U.cursor)
            Close();
        else
            Screen(UI_STARTER);
        break;
    case UI_STARTER:
        if (!Tcg_GrantCase())
            Message("Your Key Items pocket needs space for the Card Case.");
        else if (Tcg_ChooseStarter(c, U.cursor))
        {
            PlaySE(SE_RG_CARD_OPEN);
            Screen(UI_CASE);
            Message("You got a CARD CASE and the %s deck!", c->decks[0].name);
        }
        break;
    case UI_CASE:
        switch (U.cursor)
        {
        case 0:
            if (U.opponentDeck && U.opponent && strcmp(U.opponent, "Practice Collector"))
                StartDuel(0);
            else
                Message("Talk to a card player and press the challenge button to ask for a duel.");
            break;
        case 1:
            U.filter = 9;
            Screen(UI_COLLECTION);
            break;
        case 2:
            Screen(UI_DECKS);
            U.cursor = c->activeDeck;
            break;
        case 3:
            Screen(UI_PACKS);
            break;
        case 4:
            StartDuel(1);
            break;
        case 5:
            U.showAll = !U.showAll;
            break;
        case 6:
            c->fastAnimations = !c->fastAnimations;
            break;
        case 7:
            Screen(UI_ONLINE);
            break;
        case 8:
            Close();
            break;
        }
        break;
    case UI_ONLINE:
    {
        struct TcgLink *l = Tcg_Online();
        if (!MP_IsConnected() || U.cursor == (int)gMpLocalId || !gMpPlayers[U.cursor].present)
            Message("Choose a connected friend. Both players must open ONLINE CARDS.");
        else if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
            Message("%s", reason);
        else if (TcgLink_Invite(l, U.cursor, Tcg_Random(&c->rng), c->decks[c->activeDeck].cards, SDL_GetTicks()))
        {
            U.online = 1;
            Screen(UI_ONLINE_WAIT);
        }
        break;
    }
    case UI_ONLINE_INVITE:
        if (U.cursor)
        {
            TcgLink_Cancel(Tcg_Online(), "Invitation declined.");
            U.online = 0;
            Screen(UI_ONLINE);
        }
        else if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
            Message("%s", reason);
        else if (TcgLink_Accept(Tcg_Online(), c->decks[c->activeDeck].cards, SDL_GetTicks()))
        {
            U.online = 1;
            Screen(UI_ONLINE_WAIT);
        }
        break;
    case UI_INSPECT:
        U.screen = UI_ART;
        U.zoom = U.panX = U.panY = 0;
        break;
    case UI_ART:
        U.zoom = !U.zoom;
        U.panX = U.panY = 0;
        break;
    case UI_COLLECTION:
        Inspect(CollectionCard(U.cursor), UI_COLLECTION);
        break;
    case UI_DECKS:
        U.deck = U.cursor;
        Screen(UI_DECK_ACTIONS);
        break;
    case UI_DECK_ACTIONS:
        if (U.cursor == 0)
        {
            if (Tcg_DeckValidate(c->decks[U.deck].cards, c->owned, reason, sizeof(reason)))
            {
                c->activeDeck = U.deck;
                PlaySE(SE_SUCCESS);
                Message("%s is now your active deck.", c->decks[U.deck].name);
            }
            else
                Message("%s", reason);
        }
        else if (U.cursor == 1)
        {
            if (!c->decks[U.deck].name[0])
                snprintf(c->decks[U.deck].name, TCG_NAME_LEN, "Deck %d", U.deck + 1);
            U.editing = 0;
            U.filter = 1;
            Screen(UI_EDIT);
        }
        else
            Screen(UI_COPY_DECK);
        break;
    case UI_COPY_DECK:
        if (U.cursor == U.deck)
            Message("Choose a different deck slot.");
        else
        {
            U.copyTarget = U.cursor;
            if (DeckCount(U.copyTarget, 0))
                Screen(UI_COPY_CONFIRM);
            else
            {
                Tcg_DeckCopy(c, U.deck, U.copyTarget);
                Screen(UI_DECK_ACTIONS);
                Message("Deck copied to slot %u.", U.copyTarget + 1);
            }
        }
        break;
    case UI_COPY_CONFIRM:
        if (U.cursor == 1)
        {
            Tcg_DeckCopy(c, U.deck, U.copyTarget);
            Screen(UI_DECK_ACTIONS);
            Message("Deck copied to slot %u.", U.copyTarget + 1);
        }
        else
            Screen(UI_COPY_DECK);
        break;
    case UI_EDIT:
        id = CollectionCard(U.cursor);
        if (id)
            Inspect(id, UI_EDIT);
        break;
    case UI_PACKS:
        U.pack = U.cursor;
        if (Tcg_OpenPack(c, U.pack, U.revealed))
        {
            Screen(UI_REVEAL);
            U.flipped = 0;
            U.revealCursor = 0;
            PlaySE(SE_RG_CARD_OPEN);
            printf("[tcg] opened pack %d\n", U.pack);
        }
        else
            Message("You have no unopened packs of this kind.");
        break;
    case UI_REVEAL:
        if (U.flipped < 10)
            U.flipped = 10;
        else
            Inspect(U.revealed[U.cursor], UI_REVEAL);
        break;
    default:
        break;
    }
}

/* Pack opening: the wrapper tears, then each card flips in turn. */
#define REVEAL_TEAR 34
static void RevealStep(unsigned pressed)
{
    const struct TcgCard *c;
    if (U.age < REVEAL_TEAR)
    {
        if (U.age == 14)
            PlaySE(SE_RG_CARD_FLIPPING);
        if (pressed & KEY_A)
            U.age = REVEAL_TEAR;
        return;
    }
    if (U.flipped < 10)
    {
        /* Each card turns over on its own; A hurries the next one. */
        static unsigned sNext;
        if (U.age == REVEAL_TEAR)
            sNext = U.age + 8;
        if (pressed & KEY_A)
            sNext = U.age;
        if (U.age >= sNext)
        {
            c = Tcg_Card(U.revealed[U.flipped]);
            PlaySE(c && c->rarity >= 2 ? SE_SHINY : SE_RG_CARD_FLIP);
            U.flipped++;
            U.cursor = U.flipped - 1;
            sNext = U.age + (Tcg_GetCollection()->fastAnimations ? 5 : c && c->rarity >= 2 ? 26 : 11);
        }
        return;
    }
}
int Tcg_Step(unsigned keys)
{
    unsigned pressed = keys & ~U.previousKeys;
    U.previousKeys = keys;
    if (!Tcg_IsActive())
        return 0;
    U.age++;
    U.frame++;
    /* Key repeat for long lists. */
    if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
    {
        if (++U.held > 18 && U.held % 4 == 0 && U.screen != UI_DUEL)
            pressed |= keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
    }
    else
        U.held = 0;
    {
        struct TcgLink *l = Tcg_Online();
        if (U.screen == UI_ONLINE && l->state == TCG_LINK_INVITED)
        {
            U.online = 1;
            Screen(UI_ONLINE_INVITE);
            PlaySE(SE_POKENAV_CALL);
        }
        if (U.online && l->state == TCG_LINK_PLAYING && U.screen == UI_ONLINE_WAIT)
        {
            OnlineView();
            Screen(UI_DUEL);
            TcgBoard_Begin(&U.duel, l->peer < MP_MAX_PLAYERS ? (const char *)gMpPlayers[l->peer].name : "FRIEND", 1);
        }
        if (U.online && l->state == TCG_LINK_ENDED)
        {
            U.online = 0;
            if (U.screen == UI_DUEL || U.back == UI_DUEL)
                TcgBoard_End();
            Screen(UI_CASE);
            Message("%s", l->error);
        }
    }
    if (U.screen == UI_DUEL)
    {
        DuelStep(pressed, keys);
        return 1;
    }
    if (U.message[0] && (pressed & (KEY_A | KEY_B)))
    {
        U.message[0] = 0;
        PlaySE(SE_SELECT);
        return 1;
    }
    if (U.screen == UI_REVEAL && U.flipped < 10)
    {
        RevealStep(pressed);
        if (pressed & KEY_B)
            U.flipped = 10;
        return 1;
    }
    if (pressed & KEY_B)
    {
        PlaySE(SE_SELECT);
        switch (U.screen)
        {
        case UI_INVITE:
        case UI_STARTER:
        case UI_CASE:
            Close();
            break;
        case UI_ART:
            U.screen = UI_INSPECT;
            break;
        case UI_INSPECT:
            U.screen = U.back;
            U.age = 0;
            break;
        case UI_EDIT:
        case UI_COPY_DECK:
            Screen(UI_DECK_ACTIONS);
            U.cursor = U.screen == UI_COPY_DECK ? 2 : 1;
            break;
        case UI_COPY_CONFIRM:
            Screen(UI_COPY_DECK);
            break;
        case UI_DECK_ACTIONS:
            Screen(UI_DECKS);
            U.cursor = U.deck;
            break;
        case UI_ONLINE_WAIT:
        case UI_ONLINE_INVITE:
            TcgLink_Cancel(Tcg_Online(), "Card session cancelled.");
            U.online = 0;
            Screen(UI_ONLINE);
            break;
        default:
            Screen(UI_CASE);
            break;
        }
        return 1;
    }
    if (U.screen == UI_INSPECT)
    {
        if (pressed & KEY_UP)
            U.inspectScroll -= 12;
        if (pressed & KEY_DOWN)
            U.inspectScroll += 12;
        if (U.inspectScroll < 0)
            U.inspectScroll = 0;
        if (U.inspectScroll > U.inspectMax)
            U.inspectScroll = U.inspectMax;
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            Choose();
        }
        return 1;
    }
    if (U.screen == UI_ART)
    {
        if (keys & KEY_UP)
            U.panY -= 2;
        if (keys & KEY_DOWN)
            U.panY += 2;
        if (keys & KEY_LEFT)
            U.panX -= 2;
        if (keys & KEY_RIGHT)
            U.panX += 2;
        if (U.panY < -60)
            U.panY = -60;
        if (U.panY > 80)
            U.panY = 80;
        if (U.panX < -60)
            U.panX = -60;
        if (U.panX > 60)
            U.panX = 60;
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            Choose();
        }
        return 1;
    }
    if (U.screen == UI_EDIT)
    {
        char reason[160];
        unsigned id;
        if (pressed & KEY_SELECT)
        {
            U.editing = !U.editing;
            U.cursor = 0;
            U.message[0] = 0;
            PlaySE(SE_SELECT);
        }
        if (pressed & (KEY_L | KEY_R))
        {
            U.filter = (U.filter + (pressed & KEY_R ? 1 : 8)) % 9;
            U.cursor = 0;
            U.message[0] = 0;
            PlaySE(SE_RG_BAG_POCKET);
        }
        if (pressed & KEY_START)
        {
            if (Tcg_DeckValidate(Tcg_GetCollection()->decks[U.deck].cards, Tcg_GetCollection()->owned, reason,
                                 sizeof(reason)))
            {
                PlaySE(SE_SUCCESS);
                Message("This deck is ready to play.");
            }
            else
                Message("%s", reason);
        }
        if (pressed & KEY_UP && U.cursor > 0)
            U.cursor--, PlaySE(SE_RG_BAG_CURSOR);
        if (pressed & KEY_DOWN && U.cursor + 1 < (int)CollectionSize())
            U.cursor++, PlaySE(SE_RG_BAG_CURSOR);
        id = CollectionCard(U.cursor);
        if (id && (pressed & KEY_LEFT))
        {
            if (Tcg_DeckRemove(Tcg_GetCollection(), U.deck, id))
                PlaySE(SE_RG_BAG_POCKET);
        }
        if (id && (pressed & KEY_RIGHT))
        {
            if (!Tcg_DeckAdd(Tcg_GetCollection(), U.deck, id, reason, sizeof(reason)))
                Message("%s", reason);
            else
                PlaySE(SE_RG_CARD_FLIP);
        }
        if (U.cursor >= (int)CollectionSize() && U.cursor)
            U.cursor--;
        if (pressed & KEY_A)
            Choose();
        return 1;
    }
    if (U.screen == UI_COLLECTION)
    {
        int n = (int)CollectionSize(), old = U.cursor;
        if (pressed & (KEY_L | KEY_R))
        {
            U.filter = (U.filter + (pressed & KEY_R ? 1 : 9)) % 10;
            U.cursor = 0;
            PlaySE(SE_RG_BAG_POCKET);
        }
        if (pressed & KEY_LEFT)
            U.cursor--;
        if (pressed & KEY_RIGHT)
            U.cursor++;
        if (pressed & KEY_UP)
            U.cursor -= 7;
        if (pressed & KEY_DOWN)
            U.cursor += 7;
        if (U.cursor < 0)
            U.cursor = 0;
        if (U.cursor >= n)
            U.cursor = n ? n - 1 : 0;
        if (old != U.cursor)
            PlaySE(SE_RG_BAG_CURSOR);
        if (pressed & (KEY_A | KEY_START) && n)
            Inspect(CollectionCard(U.cursor), UI_COLLECTION);
        if (pressed & KEY_SELECT)
            U.showAll = !U.showAll, U.cursor = 0, PlaySE(SE_SELECT);
        return 1;
    }
    {
        int old = U.cursor, horizontal = U.screen == UI_STARTER || U.screen == UI_REVEAL;
        if (pressed & KEY_UP)
            U.cursor -= U.screen == UI_REVEAL ? 5 : 1;
        if (pressed & KEY_DOWN)
            U.cursor += U.screen == UI_REVEAL ? 5 : 1;
        if (horizontal && pressed & KEY_LEFT)
            U.cursor--;
        if (horizontal && pressed & KEY_RIGHT)
            U.cursor++;
        {
            int count = MenuSize();
            if (count > 0)
            {
                while (U.cursor < 0)
                    U.cursor += count;
                U.cursor %= count;
            }
            else
                U.cursor = 0;
        }
        if (old != U.cursor)
            PlaySE(SE_RG_BAG_CURSOR);
    }
    if (pressed & (KEY_SELECT | KEY_START) && U.screen == UI_REVEAL)
        Inspect(U.revealed[U.cursor], UI_REVEAL);
    if (pressed & KEY_A)
    {
        PlaySE(SE_SELECT);
        Choose();
    }
    return 1;
}
int Tcg_HandleEvent(const union SDL_Event *event)
{
    (void)event;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Screens                                                              */
/* ------------------------------------------------------------------ */
static void DeckIllustration(unsigned id)
{
    int w, h, scan;
    SDL_Texture *art = G_ArtTexture(id, 1, &w, &h, &scan);
    const struct TcgCard *card = Tcg_Card(id);
    SDL_Rect src, dst;
    int width = 56, height, contentH, panelY;
    if (!art || !card)
        return;
    src.x = src.y = 0;
    src.w = w;
    src.h = h;
    if (scan)
    {
        /* Illustration window of the 600x825 printed card. */
        int x = 64, y = 99, cw = 474, ch = 328;
        if (card->kind == TCG_TRAINER)
            x = 64, y = 199, cw = 480, ch = 255;
        else if (card->kind == TCG_ENERGY)
            x = 136, y = 410, cw = ch = 330;
        src.x = x * w / 600;
        src.y = y * h / 825;
        src.w = cw * w / 600;
        src.h = ch * h / 825;
    }
    else
        width = 64;
    height = (width * src.h + src.w / 2) / src.w;
    contentH = (height + 7) / 8 * 8;
    panelY = 90 - (contentH + 16) / 2;
    G_Window(166, panelY, width + 16, contentH + 16);
    dst.x = 174;
    dst.y = panelY + 8 + (contentH - height) / 2;
    dst.w = width;
    dst.h = height;
    SDL_RenderCopy(G_R(), art, &src, &dst);
}
static void InspectScreen(void)
{
    const struct TcgCard *card = Tcg_Card(U.card);
    char line[160];
    int y, i, bob = U.age < 12 ? (int)((12 - U.age) * 1.5f) : 0;
    unsigned t;
    if (U.back == UI_DUEL)
    {
        TcgBoard_Render();
        G_FillA(0, 0, 240, 160, 0x000000, 150);
    }
    else
        Backdrop();
    if (!card)
        return;
    G_FillA(10, 34 - bob, 72, 100, 0x000000, 90);
    G_Card(card->id, 8, (float)(30 - bob), 72, 100, 0, 255, 0);
    G_Window(86, 4, 152, 140);
    {
        SDL_Rect clip = {90, 8, 144, 132};
        SDL_RenderSetClipRect(G_R(), &clip);
    }
    y = 8 - U.inspectScroll;
    G_Text(94, y, G_INK, G_INK_SHADOW, card->name, 136, TF_NORMAL);
    y += 15;
    if (card->kind == TCG_POKEMON)
    {
        snprintf(line, sizeof(line), "%s  Lv%u  %s", card->stage == 0 ? "BASIC" : card->stage == 1 ? "STAGE 1" : "STAGE 2",
                 card->level, G_TypeName(card->type));
        G_Text(94, y, 0x586878, G_NO_SHADOW, line, 136, TF_NARROW);
        G_HpPlate(196, y - 12 + U.inspectScroll * 0, (int)card->hp, card->hp, 0, 255);
        y += 11;
        if (card->stage)
        {
            snprintf(line, sizeof(line), "Evolves from %s", card->evolvesFrom);
            G_Text(94, y, 0x586878, G_NO_SHADOW, line, 136, TF_NARROW);
            y += 11;
        }
        y += 2;
        for (i = 0; i < 2; i++)
        {
            const struct TcgAttack *a = &card->attacks[i];
            int x = 94;
            if (!a->name[0])
                continue;
            G_FillA(92, y - 1, 140, 1, 0xd8d8d0, 255);
            if (a->category == TCG_POKEMON_POWER)
            {
                G_Text(x, y, 0xc03838, G_NO_SHADOW, "POKEMON POWER", 0, TF_NARROW);
                y += 10;
            }
            for (t = 0; t < TCG_TYPES; t++)
            {
                unsigned k;
                for (k = 0; k < a->cost[t]; k++, x += 8)
                    G_EnergyOrb(x, y + 3, t, 1);
            }
            G_Text(x + 2, y, G_INK, G_INK_SHADOW, a->name, 140 - (x - 94), TF_SMALL);
            if (a->damage)
            {
                snprintf(line, sizeof(line), "%u%s", a->damage,
                         a->category == TCG_DAMAGE_PLUS ? "+" : a->category == TCG_DAMAGE_X ? "x" : a->category == TCG_DAMAGE_MINUS ? "-" : "");
                G_TextAlign(230, y, TA_RIGHT, G_INK, G_INK_SHADOW, line, TF_SMALL);
            }
            y += 12;
            if (a->description[0])
                y = G_Text(94, y, 0x505050, G_NO_SHADOW, a->description, 136, TF_NARROW);
            if (a->category != TCG_POKEMON_POWER && !TcgDuel_EffectSupported(a->effect))
                y = G_Text(94, y, 0xc07020, G_NO_SHADOW, "(This effect isn't ready yet.)", 136, TF_NARROW);
            y += 3;
        }
        G_FillA(92, y - 1, 140, 1, 0xd8d8d0, 255);
        G_Text(94, y, 0x586878, G_NO_SHADOW, "WEAK", 0, TF_NARROW);
        if (card->weakness < 7)
            G_EnergyOrb(94 + 24, y + 2, card->weakness, 1);
        G_Text(132, y, 0x586878, G_NO_SHADOW, "RESIST", 0, TF_NARROW);
        if (card->resistance < 7)
            G_EnergyOrb(132 + 30, y + 2, card->resistance, 1);
        G_Text(176, y, 0x586878, G_NO_SHADOW, "RETREAT", 0, TF_NARROW);
        for (t = 0; t < card->retreat; t++)
            G_EnergyOrb(176 + 34 + t * 7, y + 2, TCG_COLORLESS, 0);
        y += 12;
        if (card->description[0])
            y = G_Text(94, y + 2, 0x707878, G_NO_SHADOW, card->description, 136, TF_NARROW);
    }
    else
    {
        G_Text(94, y, 0x586878, G_NO_SHADOW, card->kind == TCG_TRAINER ? "TRAINER" : "ENERGY", 0, TF_NARROW);
        y += 13;
        y = G_Text(94, y, 0x404040, G_NO_SHADOW, card->description, 136, TF_SMALL);
    }
    U.inspectMax = y + U.inspectScroll - 136;
    if (U.inspectMax < 0)
        U.inspectMax = 0;
    SDL_RenderSetClipRect(G_R(), NULL);
    if (U.inspectMax > 0)
    {
        if (U.inspectScroll > 0)
            G_Arrow(226, 7, 0, 0x707070);
        if (U.inspectScroll < U.inspectMax)
            G_Arrow(226, 136, 1, 0x707070);
    }
    Footer(U.inspectMax ? "{A} BIG PICTURE   {UPDOWN} SCROLL   {B} BACK" : "{A} BIG PICTURE   {B} BACK");
}
static void ArtScreen(void)
{
    int w, h, scan;
    SDL_Texture *art = G_ArtTexture(U.card, 1, &w, &h, &scan);
    G_Fill(0, 0, 240, 160, 0x101418);
    if (art)
    {
        float fit = scan ? 146.0f / h : (float)(w * 3 <= 228 ? 3 : 2);
        SDL_FRect dst;
        if (U.zoom)
            fit *= scan ? 1.8f : 1.33f;
        dst.w = w * fit;
        dst.h = h * fit;
        dst.x = 120 - dst.w / 2 - U.panX;
        dst.y = (scan ? 74 - (U.zoom ? dst.h / 2 : dst.h / 2) : 74 - dst.h / 2) - U.panY;
        SDL_RenderCopyF(G_R(), art, NULL, &dst);
    }
    Footer(U.zoom ? "{A} FIT   {DPAD} MOVE   {B} BACK" : "{A} ZOOM   {DPAD} MOVE   {B} BACK");
}
static void CaseScreen(void)
{
    static const char *const names[] = {"CHALLENGE",       "COLLECTION",  "DECKS",
                                        "BOOSTER PACKS",   "PRACTICE",    "SHOW: OWNED",
                                        "ANIMATIONS: NORMAL", "ONLINE CARDS", "PUT CASE AWAY"};
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, packs = 0, mascot = DeckMascot(c->activeDeck), owned = 0;
    char line[80];
    int bob = (int)(sinf(U.frame * 0.06f) * 1.5f);
    for (i = 0; i < TCG_PACK_TYPES; i++)
        packs += c->packs[i];
    for (i = 7; i <= TCG_CARD_COUNT; i++)
        owned += c->owned[i] != 0;
    Backdrop();
    Header("CARD CASE", "DEV");
    G_Window(0, 22, 136, 124);
    U.scroll = U.cursor < 3 ? 0 : U.cursor > 5 ? 3 : U.cursor - 2;
    if (U.scroll > 2)
        U.scroll = 3;
    for (i = 0; i < 6 && U.scroll + (int)i < 9; i++)
    {
        unsigned at = U.scroll + i;
        const char *label = names[at];
        int enabled = at != 0 || (U.opponent && strcmp(U.opponent, "Practice Collector"));
        if (at == 5)
            label = U.showAll ? "SHOW: ALL CARDS" : "SHOW: OWNED";
        if (at == 6)
            label = c->fastAnimations ? "ANIMATIONS: FAST" : "ANIMATIONS: NORMAL";
        if (at == 3 && packs)
            snprintf(line, sizeof(line), "BOOSTER PACKS (%u)", packs), label = line;
        Option(at, 16, 28 + 19 * i, label, enabled);
    }
    if (U.scroll > 0)
        G_Arrow(122, 27, 0, 0x707070);
    if (U.scroll + 6 < 9)
        G_Arrow(122, 138, 1, 0x707070);
    /* Your deck, on the right. */
    G_Window(136, 22, 104, 124);
    if (mascot)
        BigCard(mascot, 166, 32, bob);
    snprintf(line, sizeof(line), "%s", c->decks[c->activeDeck].name[0] ? c->decks[c->activeDeck].name : "No deck");
    G_TextAlign(188, 98, TA_CENTER, G_INK, G_INK_SHADOW, line, TF_SMALL);
    snprintf(line, sizeof(line), "WINS %u   CARDS %u", c->wins, owned);
    G_TextAlign(188, 112, TA_CENTER, 0x586878, G_NO_SHADOW, line, TF_NARROW);
    if (U.opponent && U.opponentDeck && strcmp(U.opponent, "Practice Collector"))
    {
        snprintf(line, sizeof(line), "vs %s", U.opponent);
        G_Text(144, 124, 0xb04030, G_NO_SHADOW, line, 92, TF_NARROW);
    }
    Footer("{A} SELECT   {B} CLOSE");
}
static void StarterScreen(void)
{
    static const unsigned ids[] = {TCG_CARD_CHARMANDER, TCG_CARD_SQUIRTLE, TCG_CARD_BULBASAUR};
    static const char *const names[] = {"EMBER FRIENDS", "TIDAL FRIENDS", "GARDEN FRIENDS"};
    static const char *const notes[] = {"Charmander leads a hot,\nhard-hitting Fire deck.",
                                        "Squirtle and friends wear\nfoes down with Water.",
                                        "Bulbasaur's Grass deck\npoisons and outlasts."};
    unsigned i;
    Backdrop();
    Header("CHOOSE A STARTER DECK", NULL);
    for (i = 0; i < 3; i++)
    {
        int x = 8 + i * 78, on = (int)i == U.cursor;
        int bob = on ? (int)(fabsf(sinf(U.frame * 0.1f)) * 4) : 0;
        if (on)
            G_RoundFill(x - 2, 26, 72, 80, 0xf8f8a0, 70);
        BigCard(ids[i], x + 12, 34, bob);
        G_TextAlign(x + 34, 98, TA_CENTER, on ? G_WHITE : 0xa8d8b8, on ? 0x104020 : G_NO_SHADOW, names[i], TF_SMALL);
    }
    G_Window(0, 112, 240, 36);
    G_Text(8, 116, G_INK, G_INK_SHADOW, notes[U.cursor], 224, TF_SMALL);
    Footer("{LEFTRIGHT} CHOOSE   {A} TAKE THIS DECK   {B} NOT NOW");
}
static void CollectionScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    int n = (int)CollectionSize(), i, cols = 7, rows = 2, page = cols * rows;
    unsigned id = CollectionCard(U.cursor);
    char line[80];
    Backdrop();
    snprintf(line, sizeof(line), "%u", n);
    Header("COLLECTION", U.showAll ? "ALL CARDS" : "OWNED");
    FilterTabs(21, U.filter, 0, 1);
    U.scroll = U.cursor / page * page;
    for (i = 0; i < page && U.scroll + i < n; i++)
    {
        unsigned card = CollectionCard(U.scroll + i);
        int x = 8 + (i % cols) * 33, y = 40 + (i / cols) * 51, on = U.scroll + i == U.cursor;
        unsigned count = c->owned[card];
        if (on)
            y -= 2;
        G_Card(card, (float)x, (float)y, 30, 42, 0, count || card <= 6 ? 255 : 90, 0);
        if (count && card > 6)
        {
            snprintf(line, sizeof(line), "x%u", count);
            G_RoundFill(x + 14, y + 36, G_DigitsWidth(line, 0, 1) + 4, 9, 0x1c2c48, 230);
            G_Digits(x + 16, y + 38, line, 0, 1, 0xe0f0ff, 255);
        }
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, G_WHITE, 255);
        }
    }
    if (!n)
        G_TextAlign(120, 80, TA_CENTER, 0xc8f0d8, G_NO_SHADOW, "No cards of this kind yet.", TF_SMALL);
    G_FillA(0, 136, 240, 12, 0x081c10, 160);
    if (id)
    {
        const struct TcgCard *card = Tcg_Card(id);
        if (card->kind == TCG_POKEMON)
            snprintf(line, sizeof(line), "%s  Lv%u  HP%u", card->name, card->level, card->hp);
        else
            snprintf(line, sizeof(line), "%s", card->name);
        G_Text(6, 136, G_WHITE, 0x104020, line, 170, TF_NARROW);
        snprintf(line, sizeof(line), "%d/%d", U.cursor + 1, n);
        G_TextAlign(234, 136, TA_RIGHT, 0xa8d8b8, G_NO_SHADOW, line, TF_NARROW);
    }
    Footer("{A} LOOK   {L}{R} KIND   {SELECT} OWNED/ALL   {B} BACK");
}
static void EditScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char line[80];
    unsigned i, id, total = DeckCount(U.deck, 0);
    Backdrop();
    snprintf(line, sizeof(line), "%s", U.editing ? "CARDS IN DECK" : "BUILD DECK");
    {
        char right[16];
        snprintf(right, sizeof(right), "%u/60", total);
        Header(line, right);
    }
    FilterTabs(21, U.filter, 1, 0);
    G_Window(0, 38, 166, 110);
    U.scroll = U.cursor < 3 ? 0 : U.cursor - 3;
    {
        int size = (int)CollectionSize();
        if (U.scroll > size - 7)
            U.scroll = size > 7 ? size - 7 : 0;
    }
    for (i = 0; i < 7; i++)
    {
        const struct TcgCard *card;
        int y = 43 + (int)i * 14;
        id = CollectionCard(U.scroll + i);
        if (!id)
            break;
        card = Tcg_Card(id);
        if ((int)(U.scroll + i) == U.cursor)
        {
            G_FillA(6, y, 154, 13, 0xc8e0f8, 255);
            G_Cursor(8, y + 3);
        }
        if (card->kind == TCG_POKEMON)
            snprintf(line, sizeof(line), "%s", card->name);
        else
            snprintf(line, sizeof(line), "%s", card->name);
        G_Text(16, y, G_INK, G_INK_SHADOW, line, 100, TF_SMALL);
        if (card->kind == TCG_POKEMON)
        {
            snprintf(line, sizeof(line), "Lv%u", card->level);
            G_Text(104, y + 1, 0x607080, G_NO_SHADOW, line, 0, TF_NARROW);
        }
        if (id <= 6)
            snprintf(line, sizeof(line), "%u/--", DeckCount(U.deck, id));
        else
            snprintf(line, sizeof(line), "%u/%u", DeckCount(U.deck, id), c->owned[id]);
        G_TextAlign(156, y + 1, TA_RIGHT, DeckCount(U.deck, id) ? 0x2050a0 : 0x707070, G_NO_SHADOW, line, TF_NARROW);
    }
    id = CollectionCard(U.cursor);
    if (id)
        DeckIllustration(id);
    else
        G_Text(16, 50, 0x707070, G_NO_SHADOW, "No cards in this category.", 145, TF_SMALL);
    Footer(U.editing ? "{LEFT}-  {RIGHT}+   {L}{R} KIND   {SELECT} ALL CARDS   {START} CHECK"
                     : "{LEFT}-  {RIGHT}+   {L}{R} KIND   {SELECT} THIS DECK   {START} CHECK");
}
static void DecksScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i;
    char line[80];
    Backdrop();
    Header(U.screen == UI_COPY_DECK ? "COPY TO WHICH SLOT?" : "SAVED DECKS", NULL);
    G_Window(0, 22, 148, 124);
    U.scroll = U.cursor < 3 ? 0 : U.cursor > TCG_SAVED_DECKS - 4 ? TCG_SAVED_DECKS - 6 : U.cursor - 2;
    for (i = 0; i < 6 && U.scroll + (int)i < TCG_SAVED_DECKS; i++)
    {
        unsigned d = U.scroll + i, n = DeckCount(d, 0);
        char reason[8];
        int valid = n == 60 && Tcg_DeckValidate(c->decks[d].cards, c->owned, reason, sizeof(reason));
        int y = 28 + 19 * (int)i;
        snprintf(line, sizeof(line), "%s", c->decks[d].name[0] ? c->decks[d].name : "EMPTY SLOT");
        SmallOption(d, 22, y + 2, line, c->decks[d].name[0] != 0);
        if (d == c->activeDeck)
            G_EnergyOrb(10, y + 4, TCG_FIRE, 1);
        if (n)
        {
            snprintf(line, sizeof(line), "%u", n);
            G_TextAlign(140, y + 3, TA_RIGHT, valid ? 0x2050a0 : 0xc04040, G_NO_SHADOW, line, TF_NARROW);
        }
    }
    G_Window(148, 22, 92, 124);
    {
        unsigned mascot = DeckMascot(U.cursor);
        if (mascot)
            BigCard(mascot, 172, 32, 0);
        snprintf(line, sizeof(line), "%u/60 CARDS", DeckCount(U.cursor, 0));
        G_TextAlign(194, 100, TA_CENTER, G_INK, G_INK_SHADOW, line, TF_NARROW);
        if (U.cursor == (int)c->activeDeck)
            G_TextAlign(194, 112, TA_CENTER, 0xb04030, G_NO_SHADOW, "ACTIVE DECK", TF_NARROW);
    }
    Footer("{A} SELECT   {B} BACK");
}
static void PackArt(int x, int y, unsigned pack, float tear, int focus)
{
    static const unsigned colors[][2] = {{0xd04838, 0x801818}, {0x38a048, 0x185820}, {0x6048c0, 0x302070},
                                         {0x3078c8, 0x183870}};
    static const char *const names[] = {"COLOSSEUM", "EVOLUTION", "MYSTERY", "LABORATORY"};
    static const unsigned mascots[] = {TCG_CARD_CHARIZARD, TCG_CARD_VENUSAUR_LV67, TCG_CARD_MEWTWO_LV60,
                                       TCG_CARD_ZAPDOS_LV64};
    int w = 44, h = 58, i;
    G_FillA(x + 2, y + 3, w, h, 0x002010, 110);
    for (i = 0; i < h; i++)
        G_FillA(x, y + i, w, 1, G_Mix(colors[pack][0], colors[pack][1], i / (float)h), 255);
    /* Crimped foil ends. */
    for (i = 0; i < w; i += 2)
    {
        G_FillA(x + i, y, 1, 3, 0xd8d8e0, 255);
        G_FillA(x + i, y + h - 3, 1, 3, 0xd8d8e0, 255);
    }
    G_Card(mascots[pack], (float)x + 7, (float)y + 6, 30, 42, 0, 255, 0);
    G_FillA(x, y + 47, w, 8, 0x101010, 150);
    G_TextAlign(x + w / 2, y + 45, TA_CENTER, G_WHITE, G_NO_SHADOW, names[pack], TF_NARROW);
    if (focus)
    {
        int s = (int)(U.frame % 60) - 10;
        if (s >= 0 && s < w + 8)
            G_FillA(x + s - 8, y + 3, 3, h - 6, 0xffffff, 70);
    }
    if (tear > 0)
        G_FillA(x, y, w, 6, 0x000000, 0);
}
static void PacksScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i;
    char line[16];
    Backdrop();
    Header("BOOSTER PACKS", NULL);
    for (i = 0; i < 4; i++)
    {
        int x = 10 + i * 57, on = (int)i == U.cursor;
        int bob = on ? (int)(fabsf(sinf(U.frame * 0.1f)) * 3) : 0;
        PackArt(x, 34 - bob, i, 0, on);
        snprintf(line, sizeof(line), "x%u", c->packs[i]);
        G_TextAlign(x + 22, 96, TA_CENTER, c->packs[i] ? G_WHITE : 0x70a080, c->packs[i] ? 0x104020 : G_NO_SHADOW,
                    line, TF_SMALL);
        if (on)
            G_Arrow(x + 19, 110, 0, 0xf8f8f8);
    }
    G_Window(0, 118, 240, 28);
    G_Text(8, 122, G_INK, G_INK_SHADOW,
           c->packs[U.cursor] ? "Open this pack!" : "Win card duels to earn booster packs.", 224, TF_SMALL);
    Footer("{LEFTRIGHT} CHOOSE   {A} OPEN   {B} BACK");
}
static void RevealScreen(void)
{
    unsigned i;
    int t = (int)U.age;
    Backdrop();
    Header("NEW CARDS", NULL);
    if (t < REVEAL_TEAR)
    {
        /* The pack shakes, its top tears off, the stack slides out. */
        int shake = t < 14 ? ((t / 2) % 2 ? 2 : -2) : 0;
        float tear = t < 14 ? 0 : Ease((t - 14) / 12.0f);
        if (tear > 0)
            G_Card(0, 105, 52 - tear * 22, 30, 42, 0, 255, 0);
        PackArt(98 + shake, 50 + (int)(tear * 30), (unsigned)U.pack, 0, 0);
        if (tear > 0)
        {
            /* The torn strip flies off. */
            G_FillA(98, 50 + (int)(tear * 30), 44, 5, 0x017a3e, 0);
            G_FillA(98 + (int)(tear * 40), 46 - (int)(tear * 24), 44, 5, 0xd8d8e0, (unsigned)(255 * (1 - tear)));
        }
        Footer("{A} OPEN");
        return;
    }
    for (i = 0; i < 10; i++)
    {
        int x = 12 + (int)(i % 5) * 46, y = 27 + (int)(i / 5) * 54;
        float in = Ease((float)(t - REVEAL_TEAR - (int)i * 2) / 10.0f);
        float sx = 105 + (x - 105) * in, sy = 40 + (y - 40) * in;
        const struct TcgCard *c = Tcg_Card(U.revealed[i]);
        int faceUp = (int)i < U.flipped, on = (int)i == U.cursor && U.flipped >= 10;
        if (faceUp && c && c->rarity >= 2)
        {
            int glow = (int)(fabsf(sinf(U.frame * 0.12f)) * 60);
            G_RoundFill((int)sx - 3, (int)sy - 3, 36, 48, 0xf8e070, (unsigned)(90 + glow));
        }
        G_Card(faceUp ? U.revealed[i] : 0, sx, sy - (on ? 3 : 0), 30, 42, 0, 255, 0);
        if (faceUp && c && c->rarity >= 2)
        {
            int s = (int)((U.frame * 2) % 80) - 20;
            if (s > 0 && s < 30)
                G_FillA((int)sx + s, (int)sy + 1, 2, 40, 0xffffff, 120);
            G_TextAlign((int)sx + 15, (int)sy + 42, TA_CENTER, 0xf8e070, 0x403000, "RARE!", TF_NARROW);
        }
        else if (faceUp && c && c->rarity == 1)
        {
            /* A small silver diamond marks an uncommon. */
            int cx = (int)sx + 15, cy = (int)sy + 47;
            G_Fill(cx - 1, cy - 2, 2, 1, 0xe0f0f8);
            G_Fill(cx - 2, cy - 1, 4, 2, 0xb0c8d8);
            G_Fill(cx - 1, cy + 1, 2, 1, 0x8098a8);
        }
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame((int)sx - 2 - o, (int)sy - 5 - o, 34 + 2 * o, 46 + 2 * o, G_WHITE, 255);
        }
    }
    if (U.flipped >= 10)
    {
        const struct TcgCard *c = Tcg_Card(U.revealed[U.cursor]);
        G_FillA(0, 136, 240, 12, 0x081c10, 160);
        if (c)
            G_Text(6, 136, G_WHITE, 0x104020, c->name, 228, TF_NARROW);
        Footer("{A} LOOK   {DPAD} CHOOSE   {B} CARD CASE");
    }
    else
        Footer("{A} NEXT CARD   {B} SHOW ALL");
}
void Tcg_Render(struct SDL_Renderer *r, int width, int height)
{
    static SDL_Texture *sCanvas;
    static SDL_Renderer *sCanvasOwner;
    struct TcgCollection *c;
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    char line[160];
    unsigned i;
    int scale;
    if (!Tcg_IsActive())
        return;
    G_Begin(r);
    if (sCanvas && sCanvasOwner != r)
        sCanvas = NULL;
    if (!sCanvas)
    {
        sCanvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 240, 160);
        sCanvasOwner = r;
        if (!sCanvas)
            return;
        SDL_SetTextureBlendMode(sCanvas, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sCanvas, SDL_ScaleModeNearest);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, sCanvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    c = Tcg_GetCollection();
    switch (U.screen)
    {
    case UI_INVITE:
        G_FillA(0, 0, 240, 160, 0x000000, 80);
        G_Window(176, 56, 64, 46);
        SmallOption(0, 194, 64, "YES", 1);
        SmallOption(1, 194, 80, "NO", 1);
        G_Window(0, 104, 240, 56);
        G_Text(8, 110, G_INK, G_INK_SHADOW,
               U.opponent && strcmp(U.opponent, "Practice Collector")
                   ? "This person plays Pokemon cards! Would you\nlike a CARD CASE and a free starter deck?\nCard duels are entirely optional."
                   : "Would you like a CARD CASE and a free\nstarter deck? Card duels are an entirely\noptional pastime.",
               224, TF_SMALL);
        break;
    case UI_STARTER:
        StarterScreen();
        break;
    case UI_CASE:
        CaseScreen();
        break;
    case UI_COLLECTION:
        CollectionScreen();
        break;
    case UI_EDIT:
        EditScreen();
        break;
    case UI_INSPECT:
        InspectScreen();
        break;
    case UI_ART:
        ArtScreen();
        break;
    case UI_DECKS:
    case UI_COPY_DECK:
        DecksScreen();
        break;
    case UI_DECK_ACTIONS:
        Backdrop();
        Header(c->decks[U.deck].name[0] ? c->decks[U.deck].name : "EMPTY SLOT", NULL);
        G_Window(0, 22, 148, 124);
        Option(0, 20, 32, U.deck == (int)c->activeDeck ? "ACTIVE DECK" : "SET ACTIVE", U.deck != (int)c->activeDeck);
        Option(1, 20, 55, "EDIT CARDS", 1);
        Option(2, 20, 78, "COPY DECK", 1);
        G_Window(148, 22, 92, 124);
        if (DeckMascot(U.deck))
            BigCard(DeckMascot(U.deck), 172, 32, 0);
        snprintf(line, sizeof(line), "%u/60 CARDS", DeckCount(U.deck, 0));
        G_TextAlign(194, 100, TA_CENTER, G_INK, G_INK_SHADOW, line, TF_NARROW);
        Footer("{A} SELECT   {B} BACK");
        break;
    case UI_COPY_CONFIRM:
        Backdrop();
        Header("REPLACE DECK?", NULL);
        G_Window(0, 22, 240, 124);
        G_Text(8, 30, G_INK, G_INK_SHADOW, "Replace this saved deck list?\nYour owned cards stay available.", 224,
               TF_SMALL);
        Option(0, 20, 74, "CANCEL", 1);
        Option(1, 20, 97, "REPLACE", 1);
        Footer("{A} SELECT   {B} CANCEL");
        break;
    case UI_ONLINE:
        Backdrop();
        Header("ONLINE CARDS", NULL);
        G_Window(0, 22, 240, 124);
        for (i = 0; i < MP_MAX_PLAYERS; i++)
        {
            snprintf(line, sizeof(line), "%s%s", i == (unsigned)gMpLocalId ? "YOU: " : "",
                     gMpPlayers[i].present ? (const char *)gMpPlayers[i].name : "---");
            Option(i, 20, 30 + i * 20, line, gMpPlayers[i].present && i != (unsigned)gMpLocalId);
        }
        G_Text(8, 114, 0x707070, G_NO_SHADOW, "Your friend must open ONLINE CARDS too.\nNo cards or money are wagered.", 224,
               TF_NARROW);
        Footer("{A} INVITE   {B} BACK");
        break;
    case UI_ONLINE_INVITE:
        Backdrop();
        Header("CARD DUEL INVITATION", NULL);
        G_Window(0, 22, 240, 124);
        snprintf(line, sizeof(line), "%s would like a card duel!", gMpPlayers[Tcg_Online()->peer].name);
        G_Text(8, 30, G_INK, G_INK_SHADOW, line, 224, TF_SMALL);
        G_Text(8, 46, 0x707070, G_NO_SHADOW, "No cards or money are wagered.", 224, TF_SMALL);
        Option(0, 20, 76, "ACCEPT", 1);
        Option(1, 20, 97, "DECLINE", 1);
        Footer("{A} SELECT   {B} DECLINE");
        break;
    case UI_ONLINE_WAIT:
    {
        int dots = (U.frame / 20) % 4;
        Backdrop();
        Header("ONLINE CARDS", NULL);
        G_Window(0, 40, 240, 70);
        snprintf(line, sizeof(line), "Waiting for the other player%.*s", dots, "...");
        G_Text(8, 48, G_INK, G_INK_SHADOW, line, 224, TF_SMALL);
        G_Text(8, 66, 0x707070, G_NO_SHADOW, "Both players need this dev build and\nONLINE CARDS open.", 224, TF_SMALL);
        G_Coin(210, 84, (U.frame % 60) / 60.0f, 1, 255);
        Footer("{B} CANCEL");
        break;
    }
    case UI_PACKS:
        PacksScreen();
        break;
    case UI_REVEAL:
        RevealScreen();
        break;
    case UI_DUEL:
        TcgBoard_Render();
        break;
    default:
        break;
    }
    if (U.message[0] && U.screen != UI_DUEL)
    {
        G_Window(0, 112, 240, 48);
        G_Text(8, 118, G_INK, G_INK_SHADOW, U.message, 210, TF_SMALL);
        if ((U.frame / 20) % 2)
            G_ButtonIcon(222, 148, 0);
    }
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    scale = width / 240;
    if (height / 160 < scale)
        scale = height / 160;
    if (scale < 1)
        scale = 1;
    {
        SDL_Rect dest = {(width - 240 * scale) / 2, (height - 160 * scale) / 2, 240 * scale, 160 * scale};
        SDL_RenderCopy(r, sCanvas, NULL, &dest);
    }
}

void Tcg_Debug(const char *command)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned n = 0, m = 0;
    if (!strcmp(command, "challenge"))
    {
        printf("[tcg] field challenge accepted: %d\n", Tcg_TryChallenge());
        return;
    }
    if (!strcmp(command, "open"))
    {
        Tcg_OpenChallenge("Development Collector", "PracticePlayer", 0);
        return;
    }
    if (sscanf(command, "starter %u", &n) == 1)
    {
        Tcg_ChooseStarter(c, n % 3);
        Tcg_GrantCase();
        return;
    }
    if (sscanf(command, "inspect %u", &n) == 1)
    {
        if (U.screen == UI_CLOSED)
            Screen(UI_CASE);
        Inspect(n, U.screen == UI_INSPECT ? UI_CASE : U.screen);
        return;
    }
    if (!strcmp(command, "collection"))
    {
        U.showAll = 1;
        U.filter = 9;
        Screen(UI_COLLECTION);
        return;
    }
    if (!strcmp(command, "packs"))
    {
        if (c->optedIn)
        {
            Tcg_AwardPack(c, 0);
            Screen(UI_PACKS);
        }
        return;
    }
    if (!strcmp(command, "online"))
    {
        Screen(UI_ONLINE);
        return;
    }
    if (sscanf(command, "invite %u", &n) == 1)
    {
        Screen(UI_ONLINE);
        U.cursor = n % MP_MAX_PLAYERS;
        Choose();
        return;
    }
    if (!strcmp(command, "netconcede"))
    {
        struct TcgAction a = {0};
        a.type = TCG_CONCEDE;
        TcgLink_Submit(Tcg_Online(), &a, SDL_GetTicks());
        return;
    }
    if (!strcmp(command, "netstep"))
    {
        struct TcgAction a;
        struct TcgLink *l = Tcg_Online();
        if (l->state == TCG_LINK_PLAYING && TcgDuel_AiAction(&l->game, &a))
            TcgLink_Submit(l, &a, SDL_GetTicks());
        printf("[tcg-net] state %u seq %u phase %u hash %08x winner %u\n", l->state, l->sequence, l->game.phase,
               TcgLink_Hash(&l->game), l->game.winner);
        return;
    }
    if (!strcmp(command, "ready"))
    {
        /* Harness helper: use legal AI actions for setup, then wait for the
         * player's first turn. It never relocates or reveals hidden cards. */
        struct TcgAction a;
        unsigned steps = 0;
        while (steps++ < 128 && U.duel.phase != TCG_FINISHED && (U.duel.phase == TCG_SETUP || U.duel.current == 1))
            if (!TcgDuel_AiAction(&U.duel, &a) || TcgDuel_Apply(&U.duel, &a) != TCG_OK)
                break;
        printf("[tcg] ready phase %u current %u conserved %d\n", U.duel.phase, U.duel.current,
               TcgDuel_Conserved(&U.duel));
        return;
    }
    if (sscanf(command, "give %u %u", &n, &m) >= 1)
    {
        /* Harness helper: move a card of this kind from your deck to your
         * hand (or to a field slot when m is 1..6) so a scene can be staged. */
        struct TcgPlayer *p = &U.duel.players[0];
        unsigned i;
        for (i = 0; i < p->deckSize; i++)
            if (p->cards[p->deck[i]] == n)
            {
                uint8_t card = TcgPile_Remove(p->deck, &p->deckSize, i);
                if (m >= 1 && m <= TCG_FIELD_SIZE && p->field[m - 1].stackSize)
                    p->field[m - 1].attached[p->field[m - 1].attachedSize++] = card;
                else
                    p->hand[p->handSize++] = card;
                break;
            }
        printf("[tcg] give %u -> %u conserved %d\n", n, m, TcgDuel_Conserved(&U.duel));
        return;
    }
    if (sscanf(command, "hurt %u %u", &n, &m) == 2)
    {
        /* Harness helper: damage the opponent's Pokemon in slot n by m. */
        if (n < TCG_FIELD_SIZE && U.duel.players[1].field[n].stackSize)
            U.duel.players[1].field[n].damage += m;
        return;
    }
    if (!strcmp(command, "snapshot"))
    {
        uint32_t hash = TcgLink_Hash(&U.duel);
        printf("[tcg] rules snapshot %08x conserved %d\n", hash, TcgDuel_Conserved(&U.duel));
        return;
    }
    if (!strcmp(command, "ai"))
    {
        unsigned steps = 0;
        struct TcgAction a;
        while (U.duel.phase != TCG_FINISHED && steps++ < 2000)
        {
            if (!TcgDuel_AiAction(&U.duel, &a) || TcgDuel_Apply(&U.duel, &a) != TCG_OK)
                break;
        }
        printf("[tcg] AI test: %u actions, phase %u, winner %u\n", steps, U.duel.phase, U.duel.winner);
        return;
    }
    if (sscanf(command, "focus %u %u", &n, &m) == 2)
    {
        TcgBoard_DebugFocus((int)n, (int)m);
        return;
    }
    if (!strcmp(command, "mine"))
    {
        /* Harness helper: one legal AI move for you, shown like your own. */
        struct TcgAction a;
        if (U.screen == UI_DUEL && TcgBoard_Idle() && TcgDuel_AiAction(&U.duel, &a))
            Apply(&a);
        printf("[tcg] mine phase %u current %u\n", U.duel.phase, U.duel.current);
        return;
    }
    if (!strcmp(command, "auto"))
    {
        /* Harness helper: the AI plays your side too, shown normally. */
        U.autoPlay = !U.autoPlay;
        return;
    }
    if (!strcmp(command, "fast"))
    {
        c->fastAnimations = 1;
        return;
    }
    if (sscanf(command, "deck %u", &n) == 1 && n < TCG_SAVED_DECKS)
    {
        char reason[160];
        int valid = Tcg_DeckValidate(c->decks[n].cards, c->owned, reason, sizeof(reason));
        printf("[tcg] deck %u count %u valid %d active %u\n", n, DeckCount(n, 0), valid, c->activeDeck);
        return;
    }
    if (!strcmp(command, "state"))
    {
        struct TcgPlayer *p = &U.duel.players[0];
        printf("[tcg] screen %d opted %u phase %u current %u wins %u hand %u field %u bench %u%u%u%u%u idle %d\n",
               U.screen, c->optedIn, U.duel.phase, U.duel.current, c->wins, p->handSize, p->field[0].stackSize,
               p->field[1].stackSize, p->field[2].stackSize, p->field[3].stackSize, p->field[4].stackSize,
               p->field[5].stackSize, U.screen == UI_DUEL ? TcgBoard_Idle() : -1);
    }
}
#endif
