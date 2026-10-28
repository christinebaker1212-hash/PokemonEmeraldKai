/* The card screens: the CARD CASE (decks, the deck builder, the binder,
 * booster packs), a challenge's window, the card you take after a win, the
 * CARD CLUB's counter and starter decks, and friends' duels and trades. Drawn
 * like the game's own BAG and OPTIONS: a coloured side panel, a white list
 * with a red row frame, a description strip along the bottom. */
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "item.h"
#include "money.h"
#include "mp.h"
#include "script.h"
#include "sound.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/characters.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include "tcg/integration.h"
#include "tcg/npc.h"
#include "tcg/online.h"
#include "tcg/scale.h"
#include "tcg_board.h"
#include "tcg_gfx.h"
#include "tcg_ui_kit.h"
#include "ygo/integration.h"
#include "../ygo/ygo_gfx.h"
int Tcg_GrantCase(void);
void Tcg_ResumeGame(void (*callback)(void));
void MP_Gatcha_AddTickets(s32 n);

enum
{
    UI_CLOSED,
    UI_HOME,
    UI_DECKS,
    UI_BUILDER,
    UI_BINDER,
    UI_INSPECT,
    UI_ART,
    UI_PACKS,
    UI_REVEAL,
    UI_CHALLENGE,
    UI_DUEL,
    UI_SPOILS,
    UI_COUNTER,
    UI_STARTER,
    UI_TRADE,
    UI_WAIT,
    UI_GCSHOP     /* the GAME CENTER CARD SHOP (RUSTBORO CITY): singles and structure decks for money */
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
/* What a duel is: a card player on the map, practice, the CARD CLUB's lesson, a friend. */
enum
{
    VS_NPC,
    VS_PRACTICE,
    VS_LESSON,
    VS_FRIEND
};
/* What a YES (or a dismissed message) does next. */
enum
{
    ACT_NONE,
    ACT_CLOSE,
    ACT_HOME,
    ACT_TAKE_SPOIL,
    ACT_STARTER,
    ACT_COPY,
    ACT_CLEAR,
    ACT_TRADE_IN,
    ACT_TRADE_IN_ALL,
    ACT_TRADE_IN_EVERY,   /* every spare card (the list's last row, as on the Yu-Gi-Oh! counter) */
    ACT_BUY,
    ACT_GC_CARD,
    ACT_GC_DECK
};
#define PANEL_W 104
#define STRIP_Y 112
#define LIST_X 108
#define LIST_W 128
#define ROW_H 16
#define SHOP_BINDER 2 /* U.showAll: the binder is the CARD CLUB's "card you choose" list */
#define GC_BINDER 3   /* ... the GAME CENTER CARD SHOP's singles, for money */

static struct
{
    int screen, back, cursor, scroll, frame, age;
    unsigned previousKeys, held;
    /* The card inspector */
    int card, inspectScroll, inspectMax, zoom, panX, panY;
    /* Decks and the builder */
    int deck, filter, showAll, side, deckCursor, deckScroll;
    /* Packs */
    int pack, flipped;
    uint16_t revealed[10];
    uint16_t revealNew;   /* bit per card: one the player didn't have before this pack */
    int revealComplete;   /* the pack finished its set */
    /* Duels */
    int vs, aiDelay, autoPlay, online, rewarded;
    int concedeHeld;   /* online: a concession waiting for our last move's answer */
    int counterHome;   /* the counter the CARD SHOP list goes back to: UI_COUNTER or UI_GCSHOP */
    struct TcgOpponent opp;
    struct TcgDuel duel;
    SDL_Texture *portrait;
    /* After a win: their deck's cards to choose from */
    uint16_t spoils[TCG_DECK_SIZE];
    int spoilCount;
    /* The counter */
    int tab;
    /* The starter decks */
    int starterPhase, starterChoice;
    /* Online */
    int onlineDeckChosen, onlineDeck;
    /* Messages and questions */
    char message[256];
    int messageThen, asking, askYes, askAction, askArg;
    int fromScript;
    void (*returnToGame)(void);
} U;

static float Ease(float t)
{
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    return 1 - (1 - t) * (1 - t) * (1 - t);
}
static const char *PeerName(unsigned id)
{
    static char name[4][MP_NAME_LEN * 2 + 2];
    static unsigned next;
    char *out = name[next++ % 4];
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return "FRIEND";
    MP_GameToAscii(out, gMpPlayers[id].name, MP_NAME_LEN * 2 + 1);
    return out;
}
static void Say(int then, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(U.message, sizeof(U.message), format, ap);
    va_end(ap);
    U.messageThen = then;
    U.asking = 0;
}
static void Ask(int action, int arg, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(U.message, sizeof(U.message), format, ap);
    va_end(ap);
    U.asking = 1;
    U.askYes = 1;
    U.askAction = action;
    U.askArg = arg;
    U.messageThen = ACT_NONE;
}
static void Screen(int screen)
{
    U.screen = screen;
    U.cursor = 0;
    U.scroll = 0;
    U.age = 0;
}
int Tcg_IsActive(void) { return U.screen != UI_CLOSED || YgoUi_Active(); }
int Tcg_OnlineWilling(void) { return 0; }
unsigned Tcg_HiddenFrom(void)
{
    unsigned peer = Tcg_Online()->peer;
    return (U.online && Tcg_IsActive() && peer < MP_MAX_PLAYERS ? 1u << peer : 0) | YgoUi_HiddenFrom();
}
unsigned Tcg_DuelingNow(void)
{
    return U.screen == UI_DUEL || (U.screen == UI_INSPECT && U.back == UI_DUEL) || YgoUi_DuelingNow();
}

/* The local player is always player 0 on screen: the friend who accepted
 * sees the canonical duel with both sides swapped. */
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
    int script = U.fromScript;
    if (U.online)
        TcgLink_Cancel(Tcg_Online(), "Card session closed.");
    /* a tournament match left unfinished: lost by whoever left it, won by the one left waiting */
    Tcg_TourneyResult(Tcg_Social()->peerLeft);
    if (Tcg_Social()->role == TCG_SOCIAL_DECK || Tcg_Social()->role == TCG_SOCIAL_TRADE)
        Tcg_SocialLeave();
    if (U.screen == UI_DUEL || U.back == UI_DUEL)
        TcgBoard_End();
    U.online = 0;
    U.message[0] = 0;
    U.asking = 0;
    U.showAll = 0;
    Screen(UI_CLOSED);
    U.returnToGame = NULL;
    U.fromScript = 0;
    G_Destroy();
    if (U.portrait)
        SDL_DestroyTexture(U.portrait);
    U.portrait = NULL;
    MP_Sound_EndFanfare(); /* (the music goes on: see sound.c) */
    Tcg_ResumeGame(callback);
    if (script)
        ScriptContext_Enable(); /* the CARD CLUB's script goes on */
}
static void Open(int screen)
{
    U.message[0] = 0;
    U.asking = 0;
    U.online = 0;
    U.showAll = 0;
    if (U.portrait)
        SDL_DestroyTexture(U.portrait);
    U.portrait = NULL;
    Screen(screen);
    PlaySE(SE_WIN_OPEN);
}
void Tcg_OpenCase(void (*returnToGame)(void))
{
    struct YgoCollection *y = Ygo_GetCollection();
    /* The side used last (Yu-Gi-Oh!'s when that's the only game with a deck) */
    if (y->optedIn && (y->flags & YGO_FLAG_CASE_SIDE || !Tcg_GetCollection()->optedIn))
    {
        YgoUi_OpenCase(returnToGame, 0);
        return;
    }
    U.returnToGame = returnToGame;
    U.fromScript = 0;
    Open(UI_HOME);
    if (!Tcg_GetCollection()->optedIn)
        Say(ACT_CLOSE, "The CARD CASE is empty. Visit the CARD CLUB in a POKeMON MART to get your first deck!");
}
void Tcg_OpenCaseSide(void (*returnToGame)(void), int fromScript)
{
    Ygo_GetCollection()->flags &= ~YGO_FLAG_CASE_SIDE;
    U.returnToGame = returnToGame;
    Open(UI_HOME);
    U.fromScript = fromScript;
}
void Tcg_CloseScreens(void (*returnToGame)(void), int fromScript)
{
    U.returnToGame = returnToGame;
    U.fromScript = fromScript;
    Close();
}
/* The CARD CASE's YU-GI-OH! side */
static void ToYugioh(void)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.message[0] = 0;
    U.asking = 0;
    Screen(UI_CLOSED);
    Ygo_GetCollection()->flags |= YGO_FLAG_CASE_SIDE;
    YgoUi_OpenCase(callback, script);
}
void Tcg_OpenNpcChallenge(const struct TcgOpponent *opponent)
{
    struct TcgCollection *c = Tcg_GetCollection();
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.opp = *opponent;
    U.vs = VS_NPC;
    Open(UI_CHALLENGE);
    U.cursor = c->activeDeck;
    printf("[tcg] challenge: %s, special %u\n", U.opp.name, U.opp.special);
}
void Tcg_OpenStarterReveal(void)
{
    U.returnToGame = NULL;
    U.starterPhase = 0;
    U.starterChoice = 1;
    Open(UI_STARTER);
    U.fromScript = 1;
}
void Tcg_OpenClubCounter(void)
{
    U.returnToGame = NULL;
    U.tab = 0;
    U.counterHome = UI_COUNTER;
    Open(UI_COUNTER);
    U.fromScript = 1;
}
void Tcg_OpenGameCenterShop(void)
{
    U.returnToGame = NULL;
    U.tab = 0;
    U.counterHome = UI_GCSHOP;
    Open(UI_GCSHOP);
    U.fromScript = 1;
}
void Tcg_OpenOnlineDeck(void)
{
    const struct TcgSocial *s = Tcg_Social();
    struct TcgCollection *c = Tcg_GetCollection();
    U.returnToGame = NULL;
    U.fromScript = 0;
    memset(&U.opp, 0, sizeof(U.opp) - sizeof(U.opp.portrait));
    snprintf(U.opp.name, sizeof(U.opp.name), "%s", PeerName(s->peer));
    Tcg_PortraitOfPlayer(s->peer, &U.opp);
    U.opp.special = TCG_SPECIAL_NONE;
    U.opp.pool = TCG_SCALE_POOL_NONE;
    U.opp.online = 1;
    U.opp.mat = TCG_MAT_ONLINE;
    U.opp.music = MUS_RG_UNION_ROOM;
    U.vs = VS_FRIEND;
    U.onlineDeckChosen = 0;
    Open(UI_CHALLENGE);
    U.cursor = c->activeDeck;
}
void Tcg_OpenOnlineTrade(void)
{
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.vs = VS_FRIEND;
    snprintf(U.opp.name, sizeof(U.opp.name), "%s", PeerName(Tcg_Social()->peer));
    U.spoils[0] = U.spoils[1] = 0;
    Open(UI_TRADE);
    U.filter = 9;
}

/* ------------------------------------------------------------------ */
/* Collection helpers                                                   */
/* ------------------------------------------------------------------ */
static unsigned DeckCount(unsigned deck, unsigned card)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, n = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
        n += card ? c->decks[deck].cards[i] == card : c->decks[deck].cards[i] != 0;
    return n;
}
static unsigned Mascot(const uint16_t *cards)
{
    unsigned i, best = 0, score = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *card = Tcg_Card(cards[i]);
        unsigned s;
        if (!card || card->kind != TCG_POKEMON)
            continue;
        /* The most evolved, then the rarest, then the strongest. */
        s = card->stage * 1000 + card->rarity * 300 + card->hp;
        if (s > score)
            best = card->id, score = s;
    }
    return best;
}
/* The deck's types, most first (up to 3; TCG_NONE ends). */
static void DeckTypes(const uint16_t *cards, unsigned out[3])
{
    unsigned n[TCG_TYPES] = {0}, i, k;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *c = Tcg_Card(cards[i]);
        if (c && c->kind == TCG_ENERGY && c->id <= 6)
            n[c->type] += 2;
        else if (c && c->kind == TCG_POKEMON)
            n[c->type]++;
    }
    for (k = 0; k < 3; k++)
    {
        unsigned best = TCG_NONE, most = 0;
        for (i = 0; i < TCG_TYPES; i++)
            if (n[i] > most)
                best = i, most = n[i];
        out[k] = best;
        if (best != TCG_NONE)
            n[best] = 0;
    }
}
static int DeckValid(unsigned deck)
{
    char reason[8];
    struct TcgCollection *c = Tcg_GetCollection();
    return Tcg_DeckValidate(c->decks[deck].cards, c->owned, reason, sizeof(reason));
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
static int Visible(unsigned id)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (!CardFilter(id, U.filter))
        return 0;
    if (U.screen == UI_BUILDER)
        return id <= 6 || c->owned[id] != 0;
    if (U.screen == UI_TRADE)
        return id > 6 && c->owned[id] > DeckCount(c->activeDeck, id);
    if (U.screen == UI_BINDER && U.showAll == SHOP_BINDER)
        return Tcg_PointCost(id) != 0;
    if (U.screen == UI_BINDER && U.showAll == GC_BINDER)
        return Tcg_ShopPrice(id) != 0;
    return U.showAll || c->owned[id];
}
static unsigned ListCard(unsigned index)
{
    unsigned id;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        if (Visible(id) && !index--)
            return id;
    return 0;
}
static unsigned ListSize(void)
{
    unsigned id, n = 0;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        n += Visible(id);
    return n;
}
/* Distinct cards in a deck, catalogue order (the builder's right column). */
static unsigned DeckList(unsigned deck, uint16_t *out)
{
    unsigned id, n = 0;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        if (DeckCount(deck, id))
            out[n++] = (uint16_t)id;
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
/* After cards come in: whole sets, the whole collection. */
static void CollectionChanged(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned set, total;
    for (set = 0; set < TCG_SET_COUNT; set++)
        if (Tcg_SetOwned(c, set, &total) == total)
            MP_Achieve_Unlock("cardset");
    if (Tcg_SetOwned(c, TCG_SET_COUNT, &total) == total && !(c->flags & TCG_FLAG_GOLD_CASE))
    {
        c->flags |= TCG_FLAG_GOLD_CASE;
        MP_Achieve_Unlock("cardall");
        MP_Events_QueueToast("Every card collected! The CARD CASE shines gold. Visit the CARD CLUB!");
    }
}
static void AddCard(unsigned id)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (id && id <= TCG_CARD_COUNT && c->owned[id] < TCG_COLLECTION_MAX)
        c->owned[id]++;
    CollectionChanged();
}

/* ------------------------------------------------------------------ */
/* Duels                                                                */
/* ------------------------------------------------------------------ */
static const struct TcgDeckDefinition *DeckNamed(const char *name)
{
    unsigned i;
    for (i = 0; i < gTcgDeckDefinitionCount; i++)
        if (!strcmp(gTcgDeckDefinitions[i].name, name))
            return &gTcgDeckDefinitions[i];
    return NULL;
}
static void Apply(const struct TcgAction *a)
{
    if (U.online)
    {
        struct TcgLink *l = Tcg_Online();
        if (a->type == TCG_CONCEDE && l->state == TCG_LINK_PLAYING && l->pending && l->game.phase != TCG_FINISHED)
        {
            /* (one move at a time on the link: it goes as soon as the last one is answered) */
            U.concedeHeld = 1;
            return;
        }
        if (!TcgLink_Submit(Tcg_Online(), a, SDL_GetTicks()))
            TcgBoard_Reject(Tcg_Online()->error[0] ? Tcg_Online()->error
                                                   : "Wait for your turn and the other player's answer.");
        else
            OnlineView();
        return;
    }
    /* (the rules count a concession for whoever's turn it is: here it's always yours,
     * even on the opponent's turn) */
    if (a->type == TCG_CONCEDE)
        U.duel.current = 0;
    if (TcgDuel_Apply(&U.duel, a) != TCG_OK)
        TcgBoard_Reject(U.duel.error[0] ? U.duel.error : "Choose the required cards and targets.");
    U.aiDelay = 12;
}
static void BeginBoard(void)
{
    U.rewarded = 0;
    U.concedeHeld = 0;
    TcgBoard_SetMat(U.opp.mat, U.opp.matType);
    TcgBoard_SetMusic(U.opp.music);
    TcgBoard_Tutorial(U.vs == VS_LESSON);
    Screen(UI_DUEL);
    TcgBoard_Begin(&U.duel, U.opp.name, U.vs == VS_FRIEND);
    U.aiDelay = 20;
    printf("[tcg] duel started vs %s\n", U.opp.name);
    if (U.vs == VS_FRIEND)
        MPNet_Log("card duel: started online");
}
/* The Prize cards of this duel: what a card player likes (2, 3, 4 or 6), else the full 6 */
static unsigned Prizes(void)
{
    return U.vs == VS_NPC && U.opp.prizes >= 1 && U.opp.prizes <= 6 ? U.opp.prizes : 6;
}
static int StartDuel(const uint16_t *mine, const uint16_t *theirs)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (!TcgDuel_Init(&U.duel, mine, theirs, Tcg_Random(&c->rng), Prizes()))
    {
        Say(ACT_NONE, "This duel couldn't be set up.");
        return 0;
    }
    BeginBoard();
    return 1;
}
static int SingleType(const uint16_t *cards)
{
    unsigned i, type = TCG_NONE;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *c = Tcg_Card(cards[i]);
        if (!c || c->kind != TCG_POKEMON)
            continue;
        if (type != TCG_NONE && c->type != type)
            return 0;
        type = c->type;
    }
    return type != TCG_NONE;
}
/* What a win over a card player is worth. */
static void Rewards(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char line[200] = "";
    int n = 0;
    c->wins++;
    MP_Achieve_Count("cardwin", 1);
    if (U.duel.players[1].prizeSize == U.duel.prizeCount)
        MP_Achieve_Unlock("cardflawless");
    if (SingleType(U.duel.players[0].cards))
        MP_Achieve_Unlock("cardmono");
    Tcg_AwardPack(c, U.opp.pack);
    n += snprintf(line + n, sizeof(line) - n, "A booster pack went into your CARD CASE.");
    if (U.opp.special < TCG_SPECIAL_COUNT)
    {
        uint32_t bit = 1u << U.opp.special;
        if (!(c->specialBeaten & bit))
        {
            c->specialBeaten |= bit;
            MP_Achieve_Count("cardspecial", 1);
            if (U.opp.special < TCG_GYM_LEADERS)
                MP_Achieve_Count("cardgyms", 1);
        }
        if (U.opp.promo && !(c->promoGiven & bit))
        {
            c->promoGiven |= bit;
            AddCard(U.opp.promo);
            MP_Achieve_Unlock("cardpromo");
            snprintf(line + n, sizeof(line) - n, "\n%s handed you their promo card: %s!", U.opp.name,
                     Tcg_Card(U.opp.promo)->name);
        }
    }
    TcgBoard_Result(line);
    /* Today's full reward: one card of their deck, your pick */
    U.spoilCount = 0;
    if (Tcg_DailyAvailable(c, U.opp.key, Tcg_Today()))
    {
        unsigned i, j;
        for (i = 0; i < TCG_DECK_SIZE; i++)
        {
            unsigned id = U.opp.deck[i];
            const struct TcgCard *card = Tcg_Card(id);
            int seen = 0;
            /* Basic Energy is free; promo cards only from the trainer who holds them; holos only
             * from packs and trades. */
            if (!card || id <= 6 || (card->set == 4 && id != U.opp.promo) || Tcg_IsHolo(id))
                continue;
            for (j = 0; j < (unsigned)U.spoilCount; j++)
                seen |= U.spoils[j] == id;
            if (!seen)
                U.spoils[U.spoilCount++] = (uint16_t)id;
        }
    }
    printf("[tcg] victory vs %s: pack %u, %d cards to choose from\n", U.opp.name, U.opp.pack, U.spoilCount);
}
static void FriendRewards(int won)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (!won)
    {
        TcgBoard_Result("Good game! Nothing was lost.");
        return;
    }
    c->wins++;
    c->onlineWins++;
    MP_Achieve_Count("cardwin", 1);
    MP_Achieve_Count("cardfriend", 1);
    Tcg_AwardPack(c, Tcg_RandomBelow(&c->rng, TCG_PACK_TYPES));
    TcgBoard_Result("A booster pack went into your CARD CASE.");
}
static void DuelStep(unsigned pressed, unsigned keys)
{
    struct TcgAction a;
    unsigned card = 0;
    int r;
    if (U.online)
    {
        if (U.concedeHeld && !Tcg_Online()->pending)
        {
            memset(&a, 0, sizeof(a));
            a.type = TCG_CONCEDE;
            U.concedeHeld = 0;
            if (Tcg_Online()->state == TCG_LINK_PLAYING && Tcg_Online()->game.phase != TCG_FINISHED)
                Apply(&a);
        }
        OnlineView();
        TcgBoard_SetWaiting(Tcg_Online()->pending);
    }
    TcgBoard_Sync(&U.duel);
    /* (online, the duel is copied from the link every frame: the flag lives here) */
    if (U.duel.phase == TCG_FINISHED && !U.rewarded)
    {
        U.rewarded = 1;
        /* CARD SCALING learns from every duel against a card player, won or lost */
        if (U.vs == VS_NPC && U.opp.pool != TCG_SCALE_POOL_NONE && U.opp.rating)
        {
            struct TcgCollection *c = Tcg_GetCollection();
            TcgScale_Result(c, U.opp.deckRating, U.opp.rating, U.duel.winner == 0);
            printf("[tcg] scaling learned: %s vs %d with a deck of %d, skill now %d after %u\n",
                   U.duel.winner == 0 ? "won" : "lost", U.opp.rating, U.opp.deckRating, c->skill, c->rated);
        }
        /* the record against this card player (a concession is a loss) */
        if (U.vs == VS_NPC && U.opp.key)
        {
            Tcg_RecordResult(Tcg_GetCollection(), U.opp.key, U.duel.winner == 0, Tcg_Today());
            printf("[tcg] record vs %s: %u-%u\n", U.opp.name, Tcg_Record(Tcg_GetCollection(), U.opp.key)->wins,
                   Tcg_Record(Tcg_GetCollection(), U.opp.key)->losses);
        }
        if (U.vs == VS_FRIEND)
        {
            Tcg_TourneyResult(U.duel.winner == 0);
            FriendRewards(U.duel.winner == 0);
        }
        else if (U.vs == VS_NPC && U.duel.winner == 0)
            Rewards();
        else if (U.vs == VS_NPC)
            TcgBoard_Result("No cards, money or progress were lost.");
        else if (U.vs == VS_LESSON)
            TcgBoard_Result(U.duel.winner == 0 ? "That's the game! You're ready for real duels."
                                               : "Close one! You'll get the hang of it.");
    }
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
    if (U.online && U.autoPlay && U.duel.phase != TCG_FINISHED && TcgBoard_Idle() && !Tcg_Online()->pending &&
        (U.duel.phase == TCG_PROMOTION ? U.duel.promotePlayer : U.duel.current) == 0)
    {
        /* Harness: the AI plays your seat through the board's own path. */
        if (U.aiDelay)
            U.aiDelay--;
        else if (TcgDuel_AiAction(&U.duel, &a))
        {
            Apply(&a);
            U.aiDelay = 6;
        }
    }
    r = TcgBoard_Step(pressed, keys, &a, &card);
    if (r == BOARD_ACTION || r == BOARD_CONCEDE)
        Apply(&a);
    else if (r == BOARD_INSPECT)
        Inspect(card, UI_DUEL);
    else if (r == BOARD_LEAVE)
    {
        int won = U.duel.winner == 0;
        printf("[tcg] left the duel (%s)\n", won ? "won" : "lost");
        if (U.online)
        {
            TcgLink_Cancel(Tcg_Online(), "Duel complete.");
            U.online = 0;
        }
        TcgBoard_End();
        if (U.vs == VS_NPC && won && U.spoilCount)
        {
            Screen(UI_SPOILS);
            Say(ACT_NONE, "As the winner, you may take one card from %s's deck!", U.opp.name);
        }
        else if (U.vs == VS_PRACTICE)
        {
            Screen(UI_HOME);
            U.cursor = 3;
        }
        else
            Close();
    }
}

/* ------------------------------------------------------------------ */
/* Drawing: the game's BAG and OPTIONS look, in full colour              */
/* ------------------------------------------------------------------ */
/* The CARD CASE's colours: red like a POKe BALL, gold once every card is in. */
static unsigned Theme(void)
{
    return Tcg_GetCollection()->flags & TCG_FLAG_GOLD_CASE ? 0xd8a830 : 0xe85048;
}
static void Background(void)
{
    int y;
    for (y = 0; y < 160; y++)
        G_FillA(0, y, 240, 1, G_Mix(0x90d0a8, 0x58a078, y / 160.0f), 255);
    for (y = 0; y < 160; y += 4)
        G_FillA(0, y, 240, 1, 0xffffff, 18);
}
/* The coloured panel on the left, with the screen's name on its tab. */
static void SidePanel(const char *title, unsigned color)
{
    unsigned light = G_Mix(color, 0xffffff, 0.45f), dark = G_Mix(color, 0x000000, 0.35f);
    int w = G_Measure(title, TF_NORMAL) + 16;
    G_RoundFill(0, 0, PANEL_W, STRIP_Y - 2, light, 255);
    G_RoundFrame(0, 0, PANEL_W, STRIP_Y - 2, dark, 255);
    G_FillA(1, 1, PANEL_W - 2, 1, 0xffffff, 120);
    G_RoundFill(2, 2, w, 15, color, 255);
    G_RoundFrame(2, 2, w, 15, dark, 255);
    G_FillA(3, 3, w - 2, 1, G_Mix(color, 0xffffff, 0.5f), 255);
    G_Text(10, 2, G_WHITE, dark, title, 0, TF_NORMAL);
}
/* The white list on the right. */
static void ListBox(int x, int y, int w, int h)
{
    G_RoundFill(x + 1, y + 2, w, h, 0x000000, 50);
    G_RoundFill(x, y, w, h, 0xf8f8f8, 255);
    G_RoundFrame(x, y, w, h, 0x707880, 255);
    G_FillA(x + 2, y + 2, w - 4, 2, 0xd8e0e8, 255);
}
/* One row: red frame when chosen (as in the BAG), grey when it can't be used. */
static void Row(int x, int y, int w, int on, int enabled, const char *label, const char *right)
{
    if (on)
    {
        int pulse = (U.frame / 16) % 2;
        G_RoundFrame(x, y, w, ROW_H - 1, 0xe82820, 255);
        G_RoundFrame(x + 1, y + 1, w - 2, ROW_H - 3, pulse ? 0xf86850 : 0xe82820, 255);
    }
    G_Text(x + 6, y + 1, enabled ? G_INK : 0xa8a8a8, enabled ? G_INK_SHADOW : G_NO_SHADOW, label, w - 8, TF_NORMAL);
    if (right)
        G_TextAlign(x + w - 5, y + 2, TA_RIGHT, enabled ? 0x506070 : 0xb0b0b0, G_NO_SHADOW, right, TF_SMALL);
}
/* The strip along the bottom: what the chosen thing is or does. */
static void StripWith(void (*draw)(unsigned, float, float, float, float, double, unsigned, unsigned), unsigned card,
                      const char *text)
{
    G_FillA(0, STRIP_Y, 240, 160 - STRIP_Y, 0x5848c8, 255);
    G_FillA(0, STRIP_Y, 240, 1, 0x9888f8, 255);
    G_FillA(0, STRIP_Y + 1, 240, 1, 0x7060e0, 255);
    if (card)
    {
        G_RoundFill(3, STRIP_Y + 3, 34, 44, 0xf8f8f8, 255);
        draw(card, 5, STRIP_Y + 4, 30, 42, 0, 255, 0);
    }
    if (strchr(text, '{'))
    {
        /* With button icons: one line at a time (no wrapping) */
        char line[128];
        const char *p = text;
        int y = STRIP_Y + 4;
        while (*p)
        {
            size_t n = strcspn(p, "\n");
            snprintf(line, sizeof(line), "%.*s", (int)n, p);
            G_Hint(card ? 42 : 8, y, G_WHITE, 0x302878, line, TF_NORMAL, 1);
            y += 16;
            p += n + (p[n] == '\n');
        }
        return;
    }
    /* Two lines fit: longer text in the small font (words wrap, so leave some slack) */
    int width = card ? 194 : 226;
    G_Text(card ? 42 : 8, STRIP_Y + 4, G_WHITE, 0x302878, text, width,
           G_Measure(text, TF_NORMAL) > width * 2 - 40 ? TF_SMALL : TF_NORMAL);
}
static void Strip(unsigned card, const char *text) { StripWith(G_Card, card, text); }
static void Hints(const char *hint)
{
    int w = G_Hint(0, 0, 0, 0, hint, TF_NARROW, 0);
    G_RoundFill(236 - w - 6, 160 - 13, w + 6, 12, 0x302878, 220);
    G_Hint(236 - w - 3, 160 - 13, 0xe0e0ff, G_NO_SHADOW, hint, TF_NARROW, 1);
}
static void BigCard(unsigned id, int x, int y, int bob)
{
    G_FillA(x + 2, y + 3, 44, 61, 0x000000, 70);
    G_Card(id, (float)x, (float)(y - bob), 44, 61, 0, 255, 0);
}
static void TypeOrbs(int x, int y, const uint16_t *cards)
{
    unsigned t[3], i;
    DeckTypes(cards, t);
    for (i = 0; i < 3 && t[i] != TCG_NONE; i++)
        G_EnergyOrb(x + (int)i * 9, y, t[i], 1);
}
static void Tabs(int x, int y, int selected, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        int tx = x + i * 14, on = i == selected;
        G_RoundFill(tx, y + (on ? 0 : 1), 13, on ? 13 : 12, on ? 0xf8f8f8 : 0x405060, 255);
        G_RoundFrame(tx, y + (on ? 0 : 1), 13, on ? 13 : 12, on ? 0xe82820 : 0x202830, 255);
        if (i < 7)
            G_EnergyOrb(tx + 3, y + 3, sFilterTypes[i], 1);
        else
            G_TextAlign(tx + 7, y, TA_CENTER, on ? G_INK : 0xe0e8f0, G_NO_SHADOW, i == 7 ? "T" : i == 8 ? "E" : "*",
                        TF_NARROW);
    }
}
void TcgUi_MessageBox(const char *message, int asking, int yes)
{
    if (!message[0])
        return;
    G_Window(0, 112, 240, 48);
    G_Text(8, 116, G_INK, G_INK_SHADOW, message, asking ? 168 : 214, TF_NORMAL);
    if (asking)
    {
        G_Window(184, 74, 56, 36);
        G_Cursor(192, 82 + (yes ? 0 : 14));
        G_Text(200, 78, G_INK, G_INK_SHADOW, "YES", 0, TF_NORMAL);
        G_Text(200, 92, G_INK, G_INK_SHADOW, "NO", 0, TF_NORMAL);
    }
    else if ((U.frame / 20) % 2)
        G_ButtonIcon(226, 152, 0);
}
static void MessageBox(void) { TcgUi_MessageBox(U.message, U.asking, U.askYes); }

/* A Yu-Gi-Oh! win over someone who used to play the Pokemon cards (the ELITE FOUR): their promo card, once,
 * for a player with a Pokemon deck; the card's id, or 0 */
unsigned Tcg_GivePromoOf(unsigned special)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned card = Tcg_SpecialPromo(special);
    if (!card || !c->optedIn || special >= 32 || (c->promoGiven & (1u << special)))
        return 0;
    c->promoGiven |= 1u << special;
    AddCard(card);
    return card;
}

/* The look, for the Yu-Gi-Oh! screens (tcg_ui_kit.h) */
unsigned TcgUi_Frame(void) { return (unsigned)U.frame; }
unsigned TcgUi_Theme(void) { return Theme(); }
void TcgUi_Background(void) { Background(); }
void TcgUi_SidePanel(const char *title, unsigned color) { SidePanel(title, color); }
void TcgUi_ListBox(int x, int y, int w, int h) { ListBox(x, y, w, h); }
void TcgUi_Row(int x, int y, int w, int on, int enabled, const char *label, const char *right)
{
    Row(x, y, w, on, enabled, label, right);
}
void TcgUi_Strip(void (*draw)(unsigned, float, float, float, float, double, unsigned, unsigned), unsigned card,
                 const char *text)
{
    StripWith(draw, card, text);
}
void TcgUi_Hints(const char *hint) { Hints(hint); }
void TcgUi_CaseTabs(unsigned game, unsigned color)
{
    static const char *const names[] = {"POKeMON", "YU-GI-OH!"};
    unsigned light = G_Mix(color, 0xffffff, 0.45f), dark = G_Mix(color, 0x000000, 0.35f);
    int x = 2, i;
    G_RoundFill(0, 0, PANEL_W, STRIP_Y - 2, light, 255);
    G_RoundFrame(0, 0, PANEL_W, STRIP_Y - 2, dark, 255);
    G_FillA(1, 1, PANEL_W - 2, 1, 0xffffff, 120);
    for (i = 0; i < 2; i++)
    {
        int on = (unsigned)i == game, w = (PANEL_W - 5) / 2;
        unsigned fill = on ? color : G_Mix(light, 0x808890, 0.35f);
        G_RoundFill(x, on ? 2 : 4, w, on ? 15 : 12, fill, 255);
        G_RoundFrame(x, on ? 2 : 4, w, on ? 15 : 12, on ? dark : G_Mix(light, 0x404850, 0.5f), 255);
        if (on)
            G_FillA(x + 1, 3, w - 2, 1, G_Mix(color, 0xffffff, 0.5f), 255);
        G_TextAlign(x + w / 2, on ? 3 : 4, TA_CENTER, on ? G_WHITE : 0x505860, on ? dark : G_NO_SHADOW, names[i],
                    TF_NARROW);
        x += w + 1;
    }
}
void TcgUi_TopBar(const char *title)
{
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    G_Text(8, 1, G_WHITE, 0x101820, title, 228, TF_NORMAL);
}

/* ------------------------------------------------------------------ */
/* Screens                                                              */
/* ------------------------------------------------------------------ */
static const char *const sHomeRows[] = {"DECKS", "BINDER", "BOOSTER PACKS", "PRACTICE DUEL", "DUEL SPEED", "CLOSE CASE"};
static const char *const sHomeHelp[] = {
    "Build decks and pick the one you duel with.", "Every card you own, by type.", "Open the packs you won.",
    "Against a practice deck. Nothing is won or lost.", "How quickly the cards move on the table.",
    "Put the CARD CASE away."};
#define HOME_ROWS 6
static void HomeScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, packs = 0, total, have = Tcg_SetOwned(c, TCG_SET_COUNT, &total);
    unsigned mascot = Mascot(c->decks[c->activeDeck].cards);
    char line[128];
    int bob = (int)(sinf(U.frame * 0.06f) * 1.5f);
    for (i = 0; i < TCG_PACK_TYPES; i++)
        packs += c->packs[i];
    Background();
    TcgUi_CaseTabs(KIT_GAME_POKEMON, Theme());
    if (mascot)
        BigCard(mascot, 30, 22, bob);
    G_TextAlign(52, 86, TA_CENTER, G_INK, G_INK_SHADOW,
                c->decks[c->activeDeck].name[0] ? c->decks[c->activeDeck].name : "No deck", TF_SMALL);
    TypeOrbs(38, 99, c->decks[c->activeDeck].cards);
    ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < HOME_ROWS; i++)
    {
        const char *right = NULL;
        if (i == 2)
            snprintf(line, sizeof(line), "%u", packs), right = line;
        if (i == 4)
            right = c->fastAnimations ? "FAST" : "NORMAL";
        Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)i, i != 2 || packs, sHomeRows[i], right);
    }
    snprintf(line, sizeof(line), "%s\nCARDS %u/%u   CARD POINTS %u", sHomeHelp[U.cursor], have, total, c->dust);
    Strip(0, line);
    Hints("{L}{R} YU-GI-OH!");
}

/* A challenge: who it is, what they play, what's in it for you, your deck. */
static void Portrait(int x, int y)
{
    SDL_Rect dst;
    if (!U.opp.portraitW)
        return;
    if (!U.portrait)
    {
        struct GSurface s;
        s.w = U.opp.portraitW;
        s.h = U.opp.portraitH;
        s.px = U.opp.portrait;
        U.portrait = G_SurfaceTexture(&s);
    }
    if (!U.portrait)
        return;
    dst.x = x - U.opp.portraitW;
    dst.y = y - U.opp.portraitH * 2;
    dst.w = U.opp.portraitW * 2;
    dst.h = U.opp.portraitH * 2;
    SDL_RenderCopy(G_R(), U.portrait, NULL, &dst);
}
static void ChallengeScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char line[160];
    unsigned i;
    int y, n;
    Background();
    SidePanel(U.vs == VS_FRIEND ? "CARD DUEL" : "CHALLENGE", U.opp.special < TCG_SPECIAL_COUNT ? 0x7058c0 : Theme());
    /* Them: their sprite, their best card, what they play */
    G_RoundFill(6, 20, 92, 46, 0x2c5c40, 255);
    G_RoundFrame(6, 20, 92, 46, 0x103020, 255);
    Portrait(28, 62);
    if (U.opp.deck)
        G_Card(Mascot(U.opp.deck), 58, 22, 30, 42, 0, 255, 0);
    /* Against them so far: wins and losses, and a ribbon once beaten today */
    if (U.vs == VS_NPC && U.opp.key)
    {
        const struct TcgRecord *r = Tcg_Record(c, U.opp.key);
        if (r && r->lastWin && r->lastWin == Tcg_Today())
        {
            G_RoundFill(7, 21, G_Measure("WON TODAY", TF_NARROW) + 6, 10, 0xd8a020, 245);
            G_Text(10, 20, G_WHITE, 0x805808, "WON TODAY", 0, TF_NARROW);
        }
        snprintf(line, sizeof(line), "%uW %uL", r ? r->wins : 0, r ? r->losses : 0);
        G_RoundFill(7, 55, G_Measure(line, TF_NARROW) + 6, 10, 0x103020, 220);
        G_Text(10, 54, r ? 0xf0f8e8 : 0x90a898, G_NO_SHADOW, line, 0, TF_NARROW);
    }
    y = 68;
    if (U.opp.title[0])
    {
        G_Text(6, y, 0x505868, G_NO_SHADOW, U.opp.title, 96, TF_NARROW);
        y += 9;
    }
    G_Text(6, y, G_INK, G_INK_SHADOW, U.opp.name, 96, TF_SMALL);
    y += 12;
    if (U.opp.deck)
    {
        G_Text(6, y + 1, 0x505868, G_NO_SHADOW, "PLAYS", 0, TF_NARROW);
        TypeOrbs(34, y + 2, U.opp.deck);
    }
    /* CARD SCALING: how the duel looks for the deck in use and how you've been doing */
    if (U.vs == VS_NPC && U.opp.rating)
    {
        unsigned e = TcgScale_Expected(U.opp.deckRating + c->skill, U.opp.rating);
        static const char *const words[] = {"VERY TOUGH", "TOUGH", "EVEN", "EASY"};
        static const unsigned colors[] = {0xc03028, 0xc07020, 0x387848, 0x3060b8};
        unsigned w = e >= 600 ? 3 : e >= 420 ? 2 : e >= 280 ? 1 : 0;
        G_Text(6, y + 12, 0x505868, G_NO_SHADOW, "MATCH", 0, TF_NARROW);
        G_Text(34, y + 12, colors[w], G_NO_SHADOW, words[w], 0, TF_NARROW);
    }
    /* Your decks */
    ListBox(LIST_X, 4, LIST_W, 104);
    G_Text(LIST_X + 6, 5, 0x606878, G_NO_SHADOW, "DUEL WITH", 0, TF_NARROW);
    if (U.vs == VS_NPC)
    {
        snprintf(line, sizeof(line), "%u PRIZES", Prizes());
        G_TextAlign(LIST_X + LIST_W - 6, 5, TA_RIGHT, Prizes() == 6 ? 0x606878 : 0xb05818, G_NO_SHADOW, line, TF_NARROW);
    }
    U.scroll = U.cursor < 2 ? 0 : U.cursor > TCG_SAVED_DECKS - 3 ? TCG_SAVED_DECKS - 4 : U.cursor - 2;
    for (i = 0; i < 5; i++)
    {
        unsigned d = U.scroll + i;
        if (d > TCG_SAVED_DECKS)
            break;
        if (d == TCG_SAVED_DECKS)
        {
            Row(LIST_X + 2, 18 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)d, 1, U.vs == VS_FRIEND ? "LEAVE" : "NOT NOW",
                NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", c->decks[d].name[0] ? c->decks[d].name : "---");
        Row(LIST_X + 2, 18 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)d, DeckValid(d), line,
            d == c->activeDeck ? "IN USE" : NULL);
    }
    /* What winning gets you */
    if (U.vs == VS_FRIEND)
    {
        snprintf(line, sizeof(line), "Nothing is wagered. The winner gets a booster pack.");
        Strip(0, line);
        return;
    }
    n = snprintf(line, sizeof(line), "WIN: a booster pack");
    if (Tcg_DailyAvailable(c, U.opp.key, Tcg_Today()))
        n += snprintf(line + n, sizeof(line) - n, " and one card from their deck, your pick.");
    else
        n += snprintf(line + n, sizeof(line) - n, ". (Their card again tomorrow.)");
    if (U.opp.promo && U.opp.special < TCG_SPECIAL_COUNT && !(c->promoGiven & (1u << U.opp.special)))
    {
        snprintf(line + n, sizeof(line) - n, "\nFirst win: their promo card!");
        Strip(U.opp.promo, line);
    }
    else
        Strip(0, line);
}

static void DecksScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, n, d = U.cursor < TCG_SAVED_DECKS ? (unsigned)U.cursor : c->activeDeck, mascot;
    char line[96];
    Background();
    SidePanel("DECKS", Theme());
    mascot = Mascot(c->decks[d].cards);
    if (mascot)
        BigCard(mascot, 30, 22, 0);
    n = DeckCount(d, 0);
    snprintf(line, sizeof(line), "%u/60 CARDS", n);
    G_TextAlign(52, 86, TA_CENTER, n == 60 && DeckValid(d) ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, TF_SMALL);
    TypeOrbs(38, 99, c->decks[d].cards);
    ListBox(LIST_X, 4, LIST_W, 104);
    U.scroll = U.cursor < 3 ? 0 : U.cursor > TCG_SAVED_DECKS - 3 ? TCG_SAVED_DECKS - 5 : U.cursor - 3;
    for (i = 0; i < 6; i++)
    {
        unsigned k = U.scroll + i;
        if (k > TCG_SAVED_DECKS)
            break;
        if (k == TCG_SAVED_DECKS)
        {
            Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)k, 1, "BACK", NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", c->decks[k].name[0] ? c->decks[k].name : "EMPTY SLOT");
        Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)k, c->decks[k].name[0] != 0, line,
            k == c->activeDeck ? "IN USE" : NULL);
    }
    if (U.cursor == TCG_SAVED_DECKS)
        Strip(0, "Back to the CARD CASE.");
    else if ((unsigned)U.cursor == c->activeDeck)
        Strip(0, "The deck you duel with.");
    else
        Strip(0, DeckCount(U.cursor, 0) ? "{R} DUEL WITH THIS DECK" : "An empty slot: {A} builds a new deck.");
    Hints("{A} EDIT  {START} COPY  {SELECT} DELETE  {B} BACK");
}

/* The builder starts on the deck's main type (Pokemon first). */
static int BuilderTab(unsigned deck)
{
    unsigned t[3], i;
    DeckTypes(Tcg_GetCollection()->decks[deck].cards, t);
    for (i = 0; i < 7; i++)
        if (t[0] != TCG_NONE && sFilterTypes[i] == t[0])
            return (int)i;
    return 1; /* FIRE */
}
/* The deck builder: your cards as pictures, the deck down the right. */
#define GRID_COLS 5
#define GRID_ROWS 2
static void BuilderScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    int n = (int)ListSize(), i, page = GRID_COLS * GRID_ROWS, start;
    uint16_t list[TCG_CARD_COUNT];
    unsigned listed = DeckList(U.deck, list), total = DeckCount(U.deck, 0), id;
    char line[64];
    Background();
    G_RoundFill(0, 0, 160, 17, 0x203040, 230);
    Tabs(2, 2, U.filter, 9);
    /* (the hint line has no room for it: the type buttons beside the tabs) */
    G_Hint(132, 3, 0xe0e0ff, G_NO_SHADOW, "{L}{R}", TF_NARROW, 1);
    /* The grid */
    start = U.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        unsigned card = ListCard(start + i);
        int x = 4 + (i % GRID_COLS) * 31, y = 20 + (i / GRID_COLS) * 46, on = U.side == 0 && start + i == U.cursor;
        unsigned inDeck = DeckCount(U.deck, card);
        if (on)
            y -= 2;
        G_Card(card, (float)x, (float)y, 30, 42, 0, 255, 0);
        snprintf(line, sizeof(line), card <= 6 ? "%u" : "%u/%u", inDeck, c->owned[card]);
        /* (over the picture's foot: the card's bottom row shows weakness, resistance and retreat) */
        G_RoundFill(x + 1, y + 17, 28, 9, inDeck ? 0x2858a8 : 0x404850, 235);
        G_Digits(x + 15 - G_DigitsWidth(line, 0, 1) / 2, y + 19, line, 0, 1, 0xf0f8ff, 255);
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n)
        G_TextAlign(80, 56, TA_CENTER, G_WHITE, 0x204030, "No cards of this kind.", TF_SMALL);
    if (n > page)
    {
        snprintf(line, sizeof(line), "%d/%d", start / page + 1, (n + page - 1) / page);
        G_TextAlign(156, 100, TA_RIGHT, G_WHITE, 0x204030, line, TF_NARROW);
    }
    /* The deck */
    ListBox(162, 0, 78, 110);
    snprintf(line, sizeof(line), "%u/60", total);
    G_Text(166, 1, total == 60 ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, 0, TF_SMALL);
    if (total == 60 && DeckValid(U.deck))
        G_Text(198, 2, 0x2858a8, G_NO_SHADOW, "READY", 0, TF_NARROW);
    {
        int rows = 9, top = U.deckCursor < 4 ? 0 : U.deckCursor - 4;
        if (top > (int)listed - rows)
            top = (int)listed > rows ? (int)listed - rows : 0;
        U.deckScroll = top;
        for (i = 0; i < rows && top + i < (int)listed; i++)
        {
            const struct TcgCard *card = Tcg_Card(list[top + i]);
            int y = 14 + i * 10, on = U.side == 1 && top + i == U.deckCursor;
            if (on)
                G_RoundFill(164, y, 74, 10, 0xf8d0c8, 255);
            snprintf(line, sizeof(line), "%s", card->name);
            while (G_Measure(line, TF_NARROW) > 52 && strlen(line) > 3)
                line[strlen(line) - 1] = 0;
            G_Text(167, y - 1, G_INK, G_NO_SHADOW, line, 0, TF_NARROW);
            snprintf(line, sizeof(line), "x%u", DeckCount(U.deck, card->id));
            G_Digits(236 - G_DigitsWidth(line, 0, 1), y + 2, line, 0, 1, 0x2858a8, 255);
        }
    }
    id = U.side ? (listed ? list[U.deckCursor < (int)listed ? U.deckCursor : 0] : 0) : ListCard(U.cursor);
    line[0] = 0;
    if (id)
    {
        const struct TcgCard *card = Tcg_Card(id);
        if (card->kind == TCG_POKEMON)
            snprintf(line, sizeof(line), "%s  HP%u  %s", card->name, card->hp,
                     card->stage == 0 ? "BASIC" : card->stage == 1 ? "STAGE 1" : "STAGE 2");
        else
            snprintf(line, sizeof(line), "%s", card->name);
    }
    Strip(0, line);
    Hints(U.side ? "{A} TAKE OUT  {LEFT} CARDS  {START} LOOK  {B} DONE" : "{A} ADD  {SELECT} TAKE OUT  {START} LOOK  {B} DONE");
}

/* The GAME CENTER CARD SHOP sold its one copy of this single */
static int GcSingleSold(unsigned id) { return id <= TCG_CARD_COUNT && (Tcg_GetCollection()->gcSingles[id / 8] >> (id % 8)) & 1; }
/* The binder: every card by type, owned or all. */
static void BinderScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    int n = (int)ListSize(), i, cols = 7, page = 14, start;
    unsigned id = ListCard(U.cursor), total, have = Tcg_SetOwned(c, TCG_SET_COUNT, &total);
    char line[96];
    Background();
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    Tabs(2, 2, U.filter, 10);
    snprintf(line, sizeof(line), "%u/%u", have, total);
    G_TextAlign(236, 2, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_SMALL);
    start = U.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        unsigned card = ListCard(start + i);
        int x = 8 + (i % cols) * 33, y = 21 + (i / cols) * 45, on = start + i == U.cursor;
        unsigned count = c->owned[card];
        if (on)
            y -= 2;
        G_Card(card, (float)x, (float)y, 30, 42, 0, count || card <= 6 ? 255 : 80, 0);
        if (count && card > 6)
        {
            /* (on the picture's corner: the bottom row shows weakness, resistance and retreat) */
            snprintf(line, sizeof(line), "x%u", count);
            G_RoundFill(x + 14, y + 17, G_DigitsWidth(line, 0, 1) + 4, 9, 0x1c2c48, 230);
            G_Digits(x + 16, y + 19, line, 0, 1, 0xe0f0ff, 255);
        }
        if (U.showAll == GC_BINDER && GcSingleSold(card))
        {
            /* the shop's one copy is gone */
            G_RoundFill(x - 1, y - 1, 22, 10, 0xc82828, 240);
            G_Text(x + 1, y - 2, G_WHITE, 0x701010, "SOLD", 0, TF_NARROW);
        }
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n)
        G_TextAlign(120, 56, TA_CENTER, G_WHITE, 0x204030, "No cards of this kind yet.", TF_SMALL);
    if (U.showAll == GC_BINDER)
    {
        if (id && GcSingleSold(id))
            snprintf(line, sizeof(line), "%s: SOLD\nYou bought your copy. You own %u.", Tcg_Card(id)->name, c->owned[id]);
        else
            snprintf(line, sizeof(line), "%s: \xa5%u\nYou have \xa5%u. You own %u.", id ? Tcg_Card(id)->name : "",
                     Tcg_ShopPrice(id), (unsigned)GetMoney(&gSaveBlock1Ptr->money), id ? c->owned[id] : 0);
        Strip(0, line);
        Hints("{A} BUY  {L}{R} TYPE  {B} BACK");
        return;
    }
    if (U.showAll == SHOP_BINDER)
    {
        unsigned cost = Tcg_PointCost(id);
        snprintf(line, sizeof(line), "%s: %u CARD POINTS\nYou have %u. You own %u.", id ? Tcg_Card(id)->name : "", cost,
                 c->dust, id ? c->owned[id] : 0);
        Strip(0, line);
        Hints("{A} BUY  {L}{R} TYPE  {B} BACK");
        return;
    }
    line[0] = 0;
    if (id)
    {
        const struct TcgCard *card = Tcg_Card(id);
        if (card->kind == TCG_POKEMON)
            snprintf(line, sizeof(line), "%s  Lv%u  HP%u\n%s", card->name, card->level, card->hp,
                     c->owned[id] ? "" : "Not in your collection yet.");
        else
            snprintf(line, sizeof(line), "%s\n%s", card->name, c->owned[id] || id <= 6 ? "" : "Not in your collection yet.");
    }
    Strip(0, line);
    Hints(U.showAll ? "{A} LOOK  {L}{R} TYPE  {SELECT} OWNED  {B} BACK" : "{A} LOOK  {L}{R} TYPE  {SELECT} ALL  {B} BACK");
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
        Background();
    if (!card)
        return;
    G_FillA(10, 34 - bob, 72, 100, 0x000000, 90);
    G_Card(card->id, 8, (float)(30 - bob), 72, 100, 0, 255, 0);
    ListBox(86, 4, 152, 140);
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
        G_HpPlate(196, y - 12, (int)card->hp, card->hp, 0, 255);
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
    Hints(U.inspectMax ? "{A} BIG PICTURE  {UPDOWN} SCROLL  {B} BACK" : "{A} BIG PICTURE  {B} BACK");
}
static void ArtScreen(void)
{
    int w, h, scan;
    SDL_Texture *art = G_ArtTexture(U.card, &w, &h, &scan);
    G_Fill(0, 0, 240, 160, 0x101418);
    if (art)
    {
        /* A printed picture fits the screen; the Game Boy's grows by whole pixels. */
        float fit = scan ? G_MinF(156.0f / h, 228.0f / w) : (float)(w * 3 <= 228 ? 3 : 2);
        SDL_FRect dst;
        if (U.zoom)
            fit *= scan ? 1.8f : 1.33f;
        dst.w = w * fit;
        dst.h = h * fit;
        dst.x = 120 - dst.w / 2 - U.panX;
        dst.y = 80 - dst.h / 2 - U.panY;
        SDL_RenderCopyF(G_R(), art, NULL, &dst);
    }
    Hints(U.zoom ? "{A} FIT  {DPAD} MOVE  {B} BACK" : "{A} ZOOM  {DPAD} MOVE  {B} BACK");
}

static void PackArt(int x, int y, unsigned pack, int focus)
{
    static const unsigned colors[][2] = {{0xd04838, 0x801818}, {0x38a048, 0x185820}, {0x6048c0, 0x302070},
                                         {0x3078c8, 0x183870}};
    static const char *const names[] = {"COLOSSEUM", "EVOLUTION", "MYSTERY", "LABORATORY"};
    static const unsigned mascots[] = {TCG_CARD_CHARIZARD, TCG_CARD_VENUSAUR_LV67, TCG_CARD_MEWTWO_LV60,
                                       TCG_CARD_ZAPDOS_LV64};
    int w = 44, h = 58, i;
    G_FillA(x + 2, y + 3, w, h, 0x000000, 80);
    for (i = 0; i < h; i++)
        G_FillA(x, y + i, w, 1, G_Mix(colors[pack][0], colors[pack][1], i / (float)h), 255);
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
}
static void TopBar(const char *title)
{
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    G_Text(8, 1, G_WHITE, 0x101820, title, 228, TF_NORMAL);
}
static void PacksScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i;
    char line[96];
    Background();
    TopBar("BOOSTER PACKS");
    for (i = 0; i < 4; i++)
    {
        int x = 10 + i * 57, on = (int)i == U.cursor;
        int bob = on ? (int)(fabsf(sinf(U.frame * 0.1f)) * 3) : 0;
        PackArt(x, 26 - bob, i, on);
        snprintf(line, sizeof(line), "x%u", c->packs[i]);
        G_TextAlign(x + 22, 88, TA_CENTER, c->packs[i] ? G_WHITE : 0xc0d8c8, 0x204030, line, TF_SMALL);
        if (on)
            G_Frame(x - 3, 26 - bob - 3, 50, 64, 0xe82820, 255);
    }
    Strip(0, c->packs[U.cursor] ? "Open this pack!" : "Win card duels to earn booster packs.");
    Hints("{LEFTRIGHT} CHOOSE  {A} OPEN  {B} BACK");
}
#define REVEAL_TEAR 34
/* A color around the rainbow (t in 0..359) */
static uint32_t Rainbow(int t)
{
    int h = ((t % 360) + 360) % 360, x = (h % 60) * 255 / 60, r, g, b;
    switch (h / 60)
    {
    case 0: r = 255, g = x, b = 0; break;
    case 1: r = 255 - x, g = 255, b = 0; break;
    case 2: r = 0, g = 255, b = x; break;
    case 3: r = 0, g = 255 - x, b = 255; break;
    case 4: r = x, g = 0, b = 255; break;
    default: r = 255, g = 0, b = 255 - x; break;
    }
    /* softened toward white */
    r = (r + 255) / 2, g = (g + 255) / 2, b = (b + 255) / 2;
    return (uint32_t)(r << 16 | g << 8 | b);
}
static void RevealScreen(void)
{
    unsigned i;
    int t = (int)U.age;
    Background();
    if (t < REVEAL_TEAR)
    {
        /* The pack shakes, its top tears off, the stack slides out. */
        int shake = t < 14 ? ((t / 2) % 2 ? 2 : -2) : 0;
        float tear = t < 14 ? 0 : Ease((t - 14) / 12.0f);
        if (tear > 0)
            G_Card(0, 105, 52 - tear * 22, 30, 42, 0, 255, 0);
        PackArt(98 + shake, 50 + (int)(tear * 30), (unsigned)U.pack, 0);
        if (tear > 0)
            G_FillA(98 + (int)(tear * 40), 46 - (int)(tear * 24), 44, 5, 0xd8d8e0, (unsigned)(255 * (1 - tear)));
        Hints("{A} OPEN");
        return;
    }
    for (i = 0; i < 10; i++)
    {
        int x = 12 + (int)(i % 5) * 46, y = 8 + (int)(i / 5) * 52;
        float in = Ease((float)(t - REVEAL_TEAR - (int)i * 2) / 10.0f);
        float sx = 105 + (x - 105) * in, sy = 40 + (y - 40) * in;
        const struct TcgCard *c = Tcg_Card(U.revealed[i]);
        int faceUp = (int)i < U.flipped, on = (int)i == U.cursor && U.flipped >= 10;
        int holo = faceUp && c && c->rarity == 2 && Tcg_IsHolo(U.revealed[i]);
        if (faceUp && c && c->rarity >= 2)
        {
            int glow = (int)(fabsf(sinf(U.frame * 0.12f)) * 60);
            G_RoundFill((int)sx - 3, (int)sy - 3, 36, 48, holo ? Rainbow((int)U.frame * 4 + (int)i * 40) : 0xf8e070,
                        (unsigned)(90 + glow));
        }
        G_Card(faceUp ? U.revealed[i] : 0, sx, sy - (on ? 3 : 0), 30, 42, 0, 255, 0);
        if (faceUp && c && c->rarity >= 2)
        {
            int s = (int)((U.frame * 2) % 80) - 20;
            if (s > 0 && s < 30)
                G_FillA((int)sx + s, (int)sy + 1, 2, 40, 0xffffff, 120);
            if (holo)
            {
                /* the foil: a band of color sweeping across the picture */
                int b = (int)((U.frame * 3) % 90) - 30;
                if (b > -6 && b < 30)
                    G_FillA((int)sx + (b < 0 ? 0 : b), (int)sy + 4, b < 0 ? 6 + b : 6, 18,
                            Rainbow((int)U.frame * 6), 90);
                G_TextAlign((int)sx + 15, (int)sy + 42, TA_CENTER, Rainbow((int)U.frame * 5), 0x302040, "HOLO!",
                            TF_NARROW);
            }
            else
                G_TextAlign((int)sx + 15, (int)sy + 42, TA_CENTER, 0xf8e070, 0x403000, "RARE!", TF_NARROW);
        }
        if (faceUp && (U.revealNew >> i) & 1)
        {
            G_RoundFill((int)sx - 2, (int)sy - 3 - (on ? 3 : 0), 17, 8, 0xe82820, 230);
            G_Text((int)sx, (int)sy - 3 - (on ? 3 : 0), 0xffffff, 0x801010, "NEW", 17, TF_NARROW);
        }
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame((int)sx - 2 - o, (int)sy - 5 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (U.flipped >= 10)
    {
        const struct TcgCard *c = Tcg_Card(U.revealed[U.cursor]);
        static const char *const setNames[] = {"COLOSSEUM", "EVOLUTION", "MYSTERY", "LABORATORY"};
        char line[64];
        if (U.revealComplete && (U.age / 60) % 3 != 2)
            snprintf(line, sizeof(line), "%s COMPLETE! Every card!", setNames[U.pack & 3]);
        else
            snprintf(line, sizeof(line), "%s%s", c ? c->name : "", c && Tcg_IsHolo(U.revealed[U.cursor]) ? " (holo)" : "");
        Strip(0, line);
        Hints("{A} LOOK  {DPAD} CHOOSE  {B} DONE");
    }
    else
        Hints("{A} NEXT CARD  {B} SHOW ALL");
}

/* After a win over a card player: one card of their deck, face up, your pick. */
static void SpoilsScreen(void)
{
    int i, cols = 7, page = 14, start = U.cursor / page * page;
    char line[96];
    Background();
    snprintf(line, sizeof(line), "%s'S DECK", U.opp.name);
    TopBar(line);
    for (i = 0; i < page && start + i < U.spoilCount; i++)
    {
        int x = 8 + (i % cols) * 33, y = 21 + (i / cols) * 45, on = start + i == U.cursor;
        const struct TcgCard *c = Tcg_Card(U.spoils[start + i]);
        if (on)
            y -= 2;
        if (c && c->rarity >= 2)
            G_RoundFill(x - 2, y - 2, 34, 46, 0xf8e070, (unsigned)(80 + fabsf(sinf(U.frame * 0.1f)) * 60));
        G_Card(U.spoils[start + i], (float)x, (float)y, 30, 42, 0, 255, 0);
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    {
        const struct TcgCard *c = Tcg_Card(U.spoils[U.cursor]);
        snprintf(line, sizeof(line), "%s\nYou own %u.", c ? c->name : "", c ? Tcg_GetCollection()->owned[c->id] : 0);
        Strip(0, line);
    }
    Hints("{A} TAKE  {START} LOOK");
}

/* The CARD CLUB's counter: TRADE IN spare cards for CARD POINTS, spend them, prizes. */
static const char *const sCounterTabs[] = {"TRADE IN", "CARD SHOP", "PRIZES"};
enum
{
    BUY_PACK0,
    BUY_PACK1,
    BUY_PACK2,
    BUY_PACK3,
    BUY_CARD,
    BUY_RARE_CANDY,
    BUY_PP_UP,
    BUY_TICKET,
    BUY_COUNT
};
static const char *const sBuyNames[BUY_COUNT] = {"COLOSSEUM PACK", "EVOLUTION PACK", "MYSTERY PACK", "LABORATORY PACK",
                                                 "A CARD YOU PICK", "RARE CANDY", "PP UP", "MOEGATCHA TICKET"};
static const unsigned sBuyCost[BUY_COUNT] = {40, 40, 40, 40, 0, 20, 25, 30};
/* Cards with spare copies, for TRADE IN. */
static unsigned SpareList(uint16_t *out)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned id, n = 0;
    for (id = 7; id <= TCG_CARD_COUNT; id++)
        if (Tcg_Spare(c, id))
            out[n++] = (uint16_t)id;
    return n;
}
/* Prizes: 25 / 50 / 75 % of each set, and every Gym Leader's deck beaten. */
#define PRIZE_ROWS (TCG_SET_COUNT * 3 + 2)
static const char *const sSetNames[] = {"COLOSSEUM", "EVOLUTION", "MYSTERY", "LABORATORY"};
static const unsigned sSetPromos[] = {TCG_CARD_MOLTRES_LV37, TCG_CARD_ZAPDOS_LV68, TCG_CARD_SLOWPOKE_LV9,
                                      TCG_CARD_IMAKUNI_CARD};
#define GYM_SWEEP_BIT (1u << 30)
/* 0 not yet, 1 ready to claim, 2 done */
static int PrizeState(unsigned row, char *what, size_t size)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (row < TCG_SET_COUNT * 3)
    {
        unsigned set = row / 3, step = row % 3, total, have = Tcg_SetOwned(c, set, &total);
        if (step == 2)
            snprintf(what, size, "%s 75%% (%u/%u): the promo card %s", sSetNames[set], have, total,
                     Tcg_Card(sSetPromos[set])->name);
        else
            snprintf(what, size, "%s %u%% (%u/%u): %s", sSetNames[set], 25 * (step + 1), have, total,
                     step ? "3 booster packs + 30 CP" : "2 booster packs");
        if (c->milestones & (1u << row))
            return 2;
        return have * 4 >= total * (step + 1);
    }
    if (row == TCG_SET_COUNT * 3)
    {
        unsigned i, beaten = 0;
        for (i = 0; i < TCG_GYM_LEADERS; i++)
            beaten += (c->specialBeaten >> i) & 1;
        snprintf(what, size, "Beat all 8 GYM LEADERS' decks (%u/8): a promo card", beaten);
        if (c->milestones & GYM_SWEEP_BIT)
            return 2;
        return beaten == TCG_GYM_LEADERS;
    }
    {
        unsigned total, have = Tcg_SetOwned(c, TCG_SET_COUNT, &total);
        snprintf(what, size, "Every card (%u/%u): a gift from the CARD CLUB", have, total);
        return c->flags & TCG_FLAG_FULL_GIFT ? 2 : 0;
    }
}
static void ClaimPrize(unsigned row)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (row < TCG_SET_COUNT * 3)
    {
        unsigned set = row / 3, step = row % 3;
        c->milestones |= 1u << row;
        if (step == 0)
        {
            c->packs[set] += 2;
            Say(ACT_NONE, "You received 2 %s booster packs!", sSetNames[set]);
        }
        else if (step == 1)
        {
            c->packs[set] += 3;
            c->dust += 30;
            Say(ACT_NONE, "You received 3 %s booster packs and 30 CARD POINTS!", sSetNames[set]);
        }
        else
        {
            AddCard(sSetPromos[set]);
            Say(ACT_NONE, "You received the promo card %s!", Tcg_Card(sSetPromos[set])->name);
        }
    }
    else if (row == TCG_SET_COUNT * 3)
    {
        c->milestones |= GYM_SWEEP_BIT;
        AddCard(TCG_CARD_SURFING_PIKACHU_ALT_LV13);
        Say(ACT_NONE, "You received the promo card %s!", Tcg_Card(TCG_CARD_SURFING_PIKACHU_ALT_LV13)->name);
    }
    PlayFanfare(MUS_OBTAIN_ITEM);
}
/* The GAME CENTER CARD SHOP (RUSTBORO CITY): singles for money (the rares the CARD CLUB doesn't have) and the
 * three structure decks. */
#include "tcg_structure_decks.h"
#define GC_DECK_PRICE 15000
static const char *const sGcTabs[] = {"SINGLES", "DECKS", "TRADE IN", "CARD SHOP", "PRIZES"};
#define GC_TABS 5
static void MoveCursor(unsigned pressed, int count, int step);
static void CounterLists(int tab);
static void ShopTabs(const char *const *names, int count, unsigned fill, unsigned edge, unsigned ink, const char *balance,
                     const char *what, unsigned color);
/* Sold: each structure deck once, one copy of each single */
static int GcDeckSold(unsigned k) { return (Tcg_GetCollection()->gcDecks >> k) & 1; }
static void GcShopScreen(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char line[160], right[24];
    int i;
    Background();
    SidePanel("GAME CENTER", 0x8848b8);
    if (U.tab < 2)
        snprintf(line, sizeof(line), "\xa5%u", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
    else
        snprintf(line, sizeof(line), "%u CP", c->dust);
    ShopTabs(sGcTabs, GC_TABS, 0xc0a0e0, 0x603090, 0x402060, line, NULL, U.tab < 2 ? 0x603090 : 0x2858a8);
    if (U.tab >= 2)
    {
        /* the CARD CLUB's counter, as every member runs it */
        CounterLists(U.tab - 2);
        Hints("{L}{R} COUNTER  {B} LEAVE");
        return;
    }
    ListBox(LIST_X, 4, LIST_W, 104);
    if (U.tab == 0)
    {
        Row(LIST_X + 2, 8, LIST_W - 4, 1, 1, "SINGLES", "300-4000");
        Strip(0, "Any card of the four sets, holos aside, one copy each. Rares too: the CARD CLUB counters don't sell "
                 "them.");
    }
    else
    {
        for (i = 0; i < (int)ARRAY_COUNT(sStructureDecks); i++)
        {
            int sold = GcDeckSold((unsigned)i);
            snprintf(right, sizeof(right), sold ? "SOLD OUT" : "\xa5%u", GC_DECK_PRICE);
            Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, i == U.cursor,
                !sold && IsEnoughMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE), sStructureDecks[i].name, right);
        }
        snprintf(line, sizeof(line), "%s\n%s", sStructureDecks[U.cursor].blurb,
                 GcDeckSold((unsigned)U.cursor) ? "SOLD OUT: one of each deck." : "The 1999 theme deck, all 60 cards.");
        Strip(0, line);
    }
    Hints("{A} CHOOSE  {L}{R} COUNTER  {B} LEAVE");
}
static void CounterStep(unsigned pressed, int tab);
static void GcShopStep(unsigned pressed)
{
    if (pressed & (KEY_L | KEY_R))
    {
        U.tab = (U.tab + (pressed & KEY_R ? 1 : GC_TABS - 1)) % GC_TABS;
        U.cursor = 0;
        PlaySE(SE_RG_BAG_POCKET);
    }
    else if (U.tab >= 2)
        CounterStep(pressed, U.tab - 2);
    else if (U.tab == 0)
    {
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            Screen(UI_BINDER);
            U.filter = 9;
            U.showAll = GC_BINDER;
        }
    }
    else
    {
        MoveCursor(pressed, (int)ARRAY_COUNT(sStructureDecks), 1);
        if (pressed & KEY_A)
        {
            if (GcDeckSold((unsigned)U.cursor))
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "The %s deck is sold out. One of each deck per customer!", sStructureDecks[U.cursor].name);
                printf("[tcg] game center: deck %d sold out\n", U.cursor);
            }
            else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE))
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "A structure deck is \xa5%u. You don't have enough money.", GC_DECK_PRICE);
            }
            else
                Ask(ACT_GC_DECK, U.cursor, "The %s deck for \xa5%u?", sStructureDecks[U.cursor].name, GC_DECK_PRICE);
        }
    }
    if (pressed & KEY_B)
    {
        PlaySE(SE_SELECT);
        Close();
    }
}
/* The counter's lists (TRADE IN, CARD SHOP, PRIZES): the CARD CLUB's, and the GAME CENTER CARD SHOP's last three tabs */
static void CounterLists(int tab)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char line[160], right[24];
    uint16_t list[TCG_CARD_COUNT];
    int i, n, rows = 6;
    ListBox(LIST_X, 4, LIST_W, 104);
    U.scroll = U.cursor < 3 ? 0 : U.cursor - 3;
    if (tab == 0)
    {
        n = (int)SpareList(list);
        if (U.scroll > n - rows)
            U.scroll = n > rows ? n - rows : 0;
        if (n && U.scroll > n + 1 - rows)
            U.scroll = n + 1 > rows ? n + 1 - rows : 0;
        for (i = 0; i < rows && U.scroll + i <= n; i++)
        {
            unsigned id;
            if (U.scroll + i == n)
            {
                if (n)
                    Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, U.scroll + i == U.cursor, 1, "ALL SPARES", NULL);
                continue;
            }
            id = list[U.scroll + i];
            snprintf(right, sizeof(right), "x%u", Tcg_Spare(c, id));
            Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, U.scroll + i == U.cursor, 1, Tcg_Card(id)->name, right);
        }
        if (!n)
        {
            G_Text(LIST_X + 8, 12, 0x707070, G_NO_SHADOW, "No spare cards yet.", LIST_W - 14, TF_SMALL);
            Strip(0, "Copies past four (and past what your decks use) trade for CARD POINTS.");
        }
        else if (U.cursor >= n)
            Strip(0, "Every spare card at once: copies past four and past what your decks use.");
        else
        {
            unsigned id = list[U.cursor];
            snprintf(line, sizeof(line), "%s: %u CP each, %u spare.\n{A} ONE  {SELECT} ALL SPARE", Tcg_Card(id)->name,
                     Tcg_PointValue(id), Tcg_Spare(c, id));
            Strip(id, line);
        }
    }
    else if (tab == 1)
    {
        n = BUY_COUNT;
        if (U.scroll > n - rows)
            U.scroll = n - rows;
        for (i = 0; i < rows && U.scroll + i < n; i++)
        {
            int k = U.scroll + i;
            if (k == BUY_CARD)
                snprintf(right, sizeof(right), "5-15");
            else
                snprintf(right, sizeof(right), "%u", sBuyCost[k]);
            Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, k == U.cursor, k == BUY_CARD || c->dust >= sBuyCost[k],
                sBuyNames[k], right);
        }
        Strip(0, U.cursor == BUY_CARD ? "Commons and uncommons, 5 or 15 CP. Rares: the GAME CENTER in RUSTBORO."
                 : U.cursor < 4       ? "A booster pack of 10 cards from this set."
                 : U.cursor == BUY_TICKET ? "One pull on any MOEGATCHA banner."
                                          : "It goes into your BAG.");
    }
    else
    {
        n = PRIZE_ROWS;
        if (U.scroll > n - rows)
            U.scroll = n - rows;
        for (i = 0; i < rows && U.scroll + i < n; i++)
        {
            int k = U.scroll + i, state = PrizeState(k, line, sizeof(line));
            char label[24];
            snprintf(label, sizeof(label), "%s", k < TCG_SET_COUNT * 3 ? sSetNames[k / 3]
                                                 : k == TCG_SET_COUNT * 3 ? "GYM DECKS" : "EVERY CARD");
            snprintf(right, sizeof(right), "%s", state == 2 ? "DONE" : state == 1 ? "READY!"
                                                 : k < TCG_SET_COUNT * 3 ? (k % 3 == 0 ? "25%" : k % 3 == 1 ? "50%" : "75%")
                                                                         : "");
            Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, k == U.cursor, state != 0, label, right);
        }
        PrizeState(U.cursor, line, sizeof(line));
        Strip(0, line);
    }
}
/* The side panel's tabs (5 fit when short) and what you spend */
void TcgUi_ShopTabs(const char *const *names, int count, int tab, unsigned fill, unsigned edge, unsigned ink,
                    const char *balance, const char *what, unsigned color)
{
    int i, h = count > 3 ? 13 : 15, step = h + 2;
    for (i = 0; i < count; i++)
    {
        int on = i == tab;
        G_RoundFill(6, 22 + i * step, 92, h, on ? 0xf8f8f8 : fill, 255);
        G_RoundFrame(6, 22 + i * step, 92, h, on ? 0xe82820 : edge, 255);
        G_Text(12, 22 + i * step - (h < 15), on ? G_INK : ink, on ? G_INK_SHADOW : G_NO_SHADOW, names[i], 0, TF_SMALL);
    }
    G_TextAlign(52, count > 3 ? 98 : 78, TA_CENTER, color, G_INK_SHADOW, balance, count > 3 ? TF_SMALL : TF_NORMAL);
    if (count <= 3)
        G_TextAlign(52, 93, TA_CENTER, 0x506078, G_NO_SHADOW, what, TF_NARROW);
}
static void ShopTabs(const char *const *names, int count, unsigned fill, unsigned edge, unsigned ink, const char *balance,
                     const char *what, unsigned color)
{
    TcgUi_ShopTabs(names, count, U.tab, fill, edge, ink, balance, what, color);
}
static void CounterScreen(void)
{
    char line[32];
    Background();
    SidePanel("CARD CLUB", 0x3878c8);
    snprintf(line, sizeof(line), "%u CP", Tcg_GetCollection()->dust);
    ShopTabs(sCounterTabs, 3, 0x88a8d8, 0x305088, 0x203860, line, "CARD POINTS", 0x2858a8);
    CounterLists(U.tab);
    Hints("{L}{R} COUNTER  {B} LEAVE");
}

/* The first deck: three face-down decks on the table, turned over one by one. */
static const unsigned sStarterIds[] = {TCG_CARD_CHARMANDER, TCG_CARD_SQUIRTLE, TCG_CARD_BULBASAUR};
static const char *const sStarterNames[] = {"EMBER FRIENDS", "TIDAL FRIENDS", "GARDEN FRIENDS"};
static const char *const sStarterNotes[] = {"CHARMANDER leads a hot, hard-hitting FIRE deck.",
                                            "SQUIRTLE and friends wear foes down with WATER.",
                                            "BULBASAUR's GRASS deck poisons and outlasts."};
#define STARTER_READY 76
static void StarterScreen(void)
{
    unsigned i;
    int t = U.age;
    G_Fill(0, 0, 240, 160, 0x1c7a44);
    for (i = 0; i < 160; i += 2)
        G_FillA(0, (int)i, 240, 1, 0x000000, 16);
    for (i = 0; i < 12; i++)
    {
        /* sparkles drifting over the table */
        int sx = (int)((i * 53 + U.frame / 2) % 240), sy = (int)((i * 37 + U.frame / 3) % 112);
        G_FillA(sx, sy, 1, 1, 0xf8f0a0, 120 + (i % 3) * 40);
    }
    TopBar("CHOOSE YOUR FIRST DECK");
    for (i = 0; i < 3; i++)
    {
        int x = 22 + (int)i * 72, on = (int)i == U.starterChoice && t >= STARTER_READY;
        float in = Ease((t - (int)i * 8) / 18.0f), flip = Ease((t - 40 - (int)i * 10) / 10.0f);
        int y = 26 + (int)((1 - in) * -90), bob = on ? (int)(fabsf(sinf(U.frame * 0.1f)) * 4) : 0;
        float w = 44 * fabsf(1 - 2 * flip);
        unsigned face = flip >= 0.5f ? sStarterIds[i] : 0;
        G_Card(0, (float)x + 3, (float)y + 3, 44, 61, 0, 200, 0);
        G_Card(0, (float)x + 1.5f, (float)y + 1.5f, 44, 61, 0, 230, 0);
        if (on)
            G_RoundFill(x - 4, y - 4 - bob, 52, 69, 0xf8e070, (unsigned)(90 + fabsf(sinf(U.frame * 0.1f)) * 80));
        G_Card(face, x + (44 - w) / 2, (float)(y - bob), w, 61, 0, 255, flip > 0.9f && t < 62 + (int)i * 10 ? 160 : 0);
        if (flip >= 1)
            G_TextAlign(x + 22, y + 66, TA_CENTER, on ? 0xf8e070 : G_WHITE, 0x103020, sStarterNames[i], TF_SMALL);
        if (t == 40 + (int)i * 10)
            PlaySE(SE_RG_CARD_FLIP);
        if (t == 52 + (int)i * 10)
            PlaySE(SE_SHINY);
    }
    if (t >= STARTER_READY)
    {
        Strip(sStarterIds[U.starterChoice], sStarterNotes[U.starterChoice]);
        Hints("{LEFTRIGHT} CHOOSE  {A} TAKE  {START} LOOK");
    }
}

/* Trading a card with a friend: each offers one, both confirm. */
static void TradeScreen(void)
{
    const struct TcgSocial *s = Tcg_Social();
    int n = (int)ListSize(), i, page = 7, start;
    char line[160];
    Background();
    unsigned mine = s->myOffer ? s->myOffer : U.spoils[0], theirs = s->theirOffer ? s->theirOffer : U.spoils[1];
    snprintf(line, sizeof(line), "CARD TRADE WITH %s", U.opp.name);
    TopBar(line);
    ListBox(4, 20, 112, 62);
    ListBox(124, 20, 112, 62);
    G_Text(10, 22, 0x606878, G_NO_SHADOW, "YOU OFFER", 0, TF_NARROW);
    G_Text(130, 22, 0x606878, G_NO_SHADOW, "THEY OFFER", 0, TF_NARROW);
    if (mine)
    {
        G_Card(mine, 10, 33, 30, 42, 0, 255, 0);
        G_Text(44, 38, G_INK, G_INK_SHADOW, Tcg_Card(mine)->name, 70, TF_SMALL);
        if (s->myConfirm)
            G_Text(44, 62, 0x2858a8, G_NO_SHADOW, "READY!", 0, TF_SMALL);
    }
    if (theirs)
    {
        G_Card(theirs, 130, 33, 30, 42, 0, 255, 0);
        G_Text(164, 38, G_INK, G_INK_SHADOW, Tcg_Card(theirs)->name, 70, TF_SMALL);
        if (s->theirConfirm)
            G_Text(164, 62, 0x2858a8, G_NO_SHADOW, "READY!", 0, TF_SMALL);
    }
    else
        G_Text(164, 46, 0x909090, G_NO_SHADOW, "Choosing...", 0, TF_SMALL);
    G_RoundFill(0, 84, 240, 15, 0x203040, 230);
    Tabs(4, 85, U.filter, 10);
    start = U.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        unsigned card = ListCard(start + i);
        int x = 8 + i * 33, on = start + i == U.cursor;
        G_Card(card, (float)x, (float)(103 - (on ? 2 : 0)), 30, 42, 0, 255, 0);
        if (on)
            G_Frame(x - 2, 99, 34, 46, 0xe82820, 255);
    }
    if (!n)
        G_Text(10, 108, G_WHITE, 0x204030, "No cards of this kind to offer.", 0, TF_SMALL);
    Hints(s->myOffer && s->theirOffer ? (s->myConfirm ? "WAITING FOR THEM  {B} LEAVE" : "{A} OFFER  {START} TRADE!  {B} LEAVE")
                                      : "{A} OFFER  {L}{R} TYPE  {B} LEAVE");
}

static void WaitScreen(void)
{
    char line[96];
    int dots = (U.frame / 20) % 4;
    Background();
    SidePanel("CARD DUEL", Theme());
    ListBox(LIST_X, 4, LIST_W, 104);
    snprintf(line, sizeof(line), "Waiting for %s%.*s", U.opp.name, dots, "...");
    G_Text(LIST_X + 8, 20, G_INK, G_INK_SHADOW, line, LIST_W - 14, TF_SMALL);
    G_Coin(LIST_X + LIST_W / 2, 70, (U.frame % 60) / 60.0f, 1, 255);
    Strip(0, "They're choosing their deck.");
    Hints("{B} LEAVE");
}

/* ------------------------------------------------------------------ */
/* Input                                                                */
/* ------------------------------------------------------------------ */
static void Act(int action)
{
    struct TcgCollection *c = Tcg_GetCollection();
    switch (action)
    {
    case ACT_CLOSE:
        Close();
        break;
    case ACT_HOME:
        Screen(UI_HOME);
        break;
    case ACT_TAKE_SPOIL:
    {
        unsigned id = U.spoils[U.askArg];
        AddCard(id);
        Tcg_DailyTake(c, U.opp.key, Tcg_Today());
        PlayFanfare(MUS_OBTAIN_ITEM);
        printf("[tcg] took card %u from %s\n", id, U.opp.name);
        Say(ACT_CLOSE, "You took %s's %s!", U.opp.name, Tcg_Card(id)->name);
        break;
    }
    case ACT_STARTER:
        if (!Tcg_GrantCase())
        {
            Say(ACT_CLOSE, "There's no room for the CARD CASE in your KEY ITEMS.");
            break;
        }
        Tcg_ChooseStarter(c, (unsigned)U.starterChoice);
        c->flags |= TCG_FLAG_CLUB_MET;
        PlayFanfare(MUS_OBTAIN_ITEM);
        printf("[tcg] starter deck %d\n", U.starterChoice);
        Say(ACT_CLOSE, "You got the %s deck! It's in your CARD CASE.", sStarterNames[U.starterChoice]);
        break;
    case ACT_COPY:
    {
        unsigned to;
        for (to = 0; to < TCG_SAVED_DECKS; to++)
            if (!DeckCount(to, 0) && to != (unsigned)U.askArg)
                break;
        if (to == TCG_SAVED_DECKS)
            Say(ACT_NONE, "There's no empty slot to copy into.");
        else
        {
            Tcg_DeckCopy(c, (unsigned)U.askArg, to);
            snprintf(c->decks[to].name, TCG_NAME_LEN, "%.18s copy", c->decks[U.askArg].name);
            Say(ACT_NONE, "Copied into slot %u.", to + 1);
        }
        break;
    }
    case ACT_CLEAR:
    {
        char reason[160];
        unsigned d, before = c->activeDeck;
        memset(c->decks[U.askArg].cards, 0, sizeof(c->decks[U.askArg].cards));
        if ((unsigned)U.askArg == c->activeDeck)
        {
            /* The deck you duel with: the next one that can be played takes over */
            for (d = 0; d < TCG_SAVED_DECKS; d++)
                if (d != c->activeDeck && DeckCount(d, 0) && Tcg_DeckValidate(c->decks[d].cards, c->owned, reason, sizeof(reason)))
                {
                    c->activeDeck = (uint8_t)d;
                    break;
                }
        }
        if ((unsigned)U.askArg != c->activeDeck)
            c->decks[U.askArg].name[0] = 0;
        PlaySE(SE_SELECT);
        if (c->activeDeck != before)
            Say(ACT_NONE, "You'll duel with %s now.", c->decks[c->activeDeck].name);
        break;
    }
    case ACT_TRADE_IN:
    case ACT_TRADE_IN_ALL:
    {
        unsigned id = (unsigned)U.askArg, count = action == ACT_TRADE_IN ? 1 : Tcg_Spare(c, id), before = c->dust;
        if (Tcg_ExchangeDuplicates(c, id, count))
        {
            PlaySE(SE_SHOP);
            Say(ACT_NONE, "Traded in %u x %s for %u CARD POINTS.", count, Tcg_Card(id)->name, c->dust - before);
        }
        break;
    }
    case ACT_TRADE_IN_EVERY:
    {
        uint16_t list[TCG_CARD_COUNT];
        unsigned n = SpareList(list), i, count = 0, before = c->dust;
        for (i = 0; i < n; i++)
        {
            unsigned spare = Tcg_Spare(c, list[i]);
            if (Tcg_ExchangeDuplicates(c, list[i], spare))
                count += spare;
        }
        U.cursor = 0;
        if (count)
        {
            PlaySE(SE_SHOP);
            Say(ACT_NONE, "Traded in %u spare cards for %u CARD POINTS.", count, c->dust - before);
        }
        break;
    }
    case ACT_GC_CARD:
    {
        unsigned id = (unsigned)U.askArg, price = Tcg_ShopPrice(id);
        if (price && !GcSingleSold(id) && IsEnoughMoney(&gSaveBlock1Ptr->money, price) && c->owned[id] < TCG_COLLECTION_MAX)
        {
            RemoveMoney(&gSaveBlock1Ptr->money, price);
            c->gcSingles[id / 8] |= 1 << (id % 8);
            AddCard(id);
            PlaySE(SE_SHOP);
            Say(ACT_NONE, "You got %s!", Tcg_Card(id)->name);
            printf("[tcg] game center: bought card %u for %u\n", id, price);
        }
        break;
    }
    case ACT_GC_DECK:
    {
        unsigned k = (unsigned)U.askArg, i, slot;
        if (k >= ARRAY_COUNT(sStructureDecks) || GcDeckSold(k) || !IsEnoughMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE))
            break;
        RemoveMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE);
        c->gcDecks |= 1 << k;
        for (i = 0; i < TCG_DECK_SIZE; i++)
            if (sStructureDecks[k].cards[i] > 6)
                AddCard(sStructureDecks[k].cards[i]);
        for (slot = 0; slot < TCG_SAVED_DECKS; slot++)
            if (!DeckCount(slot, 0))
                break;
        PlaySE(SE_SHOP);
        PlayFanfare(MUS_OBTAIN_ITEM);
        printf("[tcg] game center: bought deck %u, slot %u\n", k, slot);
        if (slot < TCG_SAVED_DECKS)
        {
            memcpy(c->decks[slot].cards, sStructureDecks[k].cards, sizeof(c->decks[slot].cards));
            snprintf(c->decks[slot].name, TCG_NAME_LEN, "%s", sStructureDecks[k].name);
            Say(ACT_NONE, "You got the %s deck! It's in DECKS, slot %u, ready to play.", sStructureDecks[k].name,
                slot + 1);
        }
        else
            Say(ACT_NONE, "You got the %s deck! Its cards are in your collection (your deck slots are full).",
                sStructureDecks[k].name);
        CollectionChanged();
        break;
    }
    case ACT_BUY:
    {
        int k = U.askArg;
        if (k >= 1000)
        {
            if (Tcg_CraftCard(c, (unsigned)(k - 1000)))
            {
                PlaySE(SE_SHOP);
                CollectionChanged();
                Say(ACT_NONE, "You got %s!", Tcg_Card(k - 1000)->name);
            }
            break;
        }
        if (c->dust < sBuyCost[k])
            break;
        if (k == BUY_RARE_CANDY || k == BUY_PP_UP)
        {
            if (!AddBagItem(k == BUY_RARE_CANDY ? ITEM_RARE_CANDY : ITEM_PP_UP, 1))
            {
                Say(ACT_NONE, "Your BAG is full.");
                break;
            }
        }
        else if (k == BUY_TICKET)
            MP_Gatcha_AddTickets(1);
        else
            c->packs[k]++;
        c->dust -= sBuyCost[k];
        PlaySE(SE_SHOP);
        Say(ACT_NONE, "You got the %s!", sBuyNames[k]);
        break;
    }
    }
}
/* The challenge (an NPC's or a friend's) once a deck is picked. */
static void ChooseDeck(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char reason[160];
    if (U.cursor == TCG_SAVED_DECKS)
    {
        Close();
        return;
    }
    if (!Tcg_DeckValidate(c->decks[U.cursor].cards, c->owned, reason, sizeof(reason)))
    {
        PlaySE(SE_FAILURE);
        Say(ACT_NONE, "%s", reason);
        return;
    }
    c->activeDeck = (uint8_t)U.cursor;
    if (U.vs == VS_FRIEND)
    {
        const struct TcgSocial *s = Tcg_Social();
        U.onlineDeck = U.cursor;
        U.onlineDeckChosen = 1;
        if (s->seat == 0 && !TcgLink_Invite(Tcg_Online(), s->peer, s->session, c->decks[U.cursor].cards, SDL_GetTicks()))
        {
            Say(ACT_CLOSE, "The duel couldn't start.");
            return;
        }
        U.online = 1;
        Screen(UI_WAIT);
        return;
    }
    /* CARD SCALING: the deck they bring fits the one you picked */
    if (U.vs == VS_NPC)
        Tcg_FitOpponent(&U.opp, c->decks[U.cursor].cards);
    StartDuel(c->decks[U.cursor].cards, U.opp.deck);
}
static void MoveCursor(unsigned pressed, int count, int step)
{
    int old = U.cursor;
    if (pressed & KEY_UP)
        U.cursor -= step;
    if (pressed & KEY_DOWN)
        U.cursor += step;
    if (count > 0)
    {
        while (U.cursor < 0)
            U.cursor += count;
        U.cursor %= count;
    }
    else
        U.cursor = 0;
    if (old != U.cursor)
        PlaySE(SE_SELECT);
}
static void GridMove(unsigned pressed, int count, int cols)
{
    int old = U.cursor;
    if (pressed & KEY_LEFT)
        U.cursor--;
    if (pressed & KEY_RIGHT)
        U.cursor++;
    if (pressed & KEY_UP)
        U.cursor -= cols;
    if (pressed & KEY_DOWN)
        U.cursor += cols;
    if (U.cursor < 0)
        U.cursor = 0;
    if (U.cursor >= count)
        U.cursor = count ? count - 1 : 0;
    if (old != U.cursor)
        PlaySE(SE_SELECT);
}
static void FilterTab(unsigned pressed, int tabs)
{
    if (pressed & (KEY_L | KEY_R))
    {
        U.filter = (U.filter + (pressed & KEY_R ? 1 : tabs - 1)) % tabs;
        U.cursor = 0;
        PlaySE(SE_RG_BAG_POCKET);
    }
}
/* Pack opening: the wrapper tears, then each card flips in turn. */
static void RevealStep(unsigned pressed)
{
    static int sNext;
    const struct TcgCard *c;
    if (U.age < REVEAL_TEAR)
    {
        if (U.age == 14)
            PlaySE(SE_RG_CARD_FLIPPING);
        if (pressed & KEY_A)
            U.age = REVEAL_TEAR;
        return;
    }
    if (U.age == REVEAL_TEAR)
        sNext = U.age + 8;
    if (pressed & KEY_A)
        sNext = U.age;
    if (U.age >= sNext)
    {
        int holo;
        c = Tcg_Card(U.revealed[U.flipped]);
        holo = c && c->rarity == 2 && Tcg_IsHolo(U.revealed[U.flipped]);
        PlaySE(c && c->rarity >= 2 ? SE_SHINY : SE_RG_CARD_FLIP);
        U.flipped++;
        U.cursor = U.flipped - 1;
        sNext = U.age + (Tcg_GetCollection()->fastAnimations ? 5 : holo ? 40 : c && c->rarity >= 2 ? 26 : 11);
        if (U.flipped >= 10 && U.revealComplete)
            PlayFanfare(MUS_OBTAIN_ITEM);
    }
}
static void OnlineStep(void)
{
    struct TcgLink *l = Tcg_Online();
    const struct TcgSocial *s = Tcg_Social();
    struct TcgCollection *c = Tcg_GetCollection();
    if (U.vs != VS_FRIEND || U.screen == UI_TRADE)
        return;
    if (s->peerLeft && U.screen != UI_DUEL && !U.message[0])
    {
        Say(ACT_CLOSE, "%s left. Nothing was lost.", U.opp.name);
        return;
    }
    /* The friend who said yes accepts the asker's invite with the deck they picked */
    if (s->seat == 1 && U.onlineDeckChosen && l->state == TCG_LINK_INVITED)
        TcgLink_Accept(l, c->decks[U.onlineDeck].cards, SDL_GetTicks());
    if (U.screen == UI_WAIT && l->state == TCG_LINK_PLAYING)
    {
        U.online = 1;
        OnlineView();
        BeginBoard();
    }
    if (U.online && l->state == TCG_LINK_ENDED && U.duel.phase != TCG_FINISHED && !U.message[0])
    {
        U.online = 0;
        if (U.screen == UI_DUEL)
            TcgBoard_End();
        U.screen = UI_WAIT;
        Say(ACT_CLOSE, "%s", l->error[0] ? l->error : "The card session ended.");
    }
}
static void TradeStep(unsigned pressed)
{
    const struct TcgSocial *s = Tcg_Social();
    struct TcgCollection *c = Tcg_GetCollection();
    int n = (int)ListSize();
    /* Both confirmed: the trade is done, even when the friend's leaving came in with their
     * confirm (they leave the moment they see ours, and both arrive in one frame). */
    if (s->myConfirm && s->theirConfirm && s->myOffer && s->theirOffer)
    {
        unsigned mine = s->myOffer, theirs = s->theirOffer;
        if (c->owned[mine])
            c->owned[mine]--;
        AddCard(theirs);
        c->trades++;
        MP_Achieve_Unlock("cardtrade");
        printf("[tcg] traded card %u for %u\n", mine, theirs);
        U.spoils[0] = (uint16_t)mine;
        U.spoils[1] = (uint16_t)theirs;
        Tcg_SocialLeave();
        PlayFanfare(MUS_OBTAIN_ITEM);
        Say(ACT_CLOSE, "You traded your %s for %s!", Tcg_Card(mine)->name, Tcg_Card(theirs)->name);
        return;
    }
    if (s->peerLeft)
    {
        Say(ACT_CLOSE, "%s left the trade.", PeerName(s->peer));
        Tcg_SocialLeave();
        return;
    }
    FilterTab(pressed, 10);
    {
        int old = U.cursor;
        if (pressed & KEY_LEFT)
            U.cursor--;
        if (pressed & KEY_RIGHT)
            U.cursor++;
        if (U.cursor < 0)
            U.cursor = 0;
        if (U.cursor >= n)
            U.cursor = n ? n - 1 : 0;
        if (old != U.cursor)
            PlaySE(SE_SELECT);
    }
    if (pressed & KEY_A && n)
    {
        Tcg_SocialOffer(ListCard(U.cursor));
        PlaySE(SE_RG_CARD_FLIP);
    }
    if (pressed & KEY_START && s->myOffer && s->theirOffer && !s->myConfirm)
    {
        Tcg_SocialConfirm();
        PlaySE(SE_SELECT);
    }
    if (pressed & KEY_B)
    {
        PlaySE(SE_SELECT);
        Close();
    }
}

/* The counter's keys on one of its lists (the CARD CLUB, the GAME CENTER CARD SHOP's last three tabs) */
static void CounterStep(unsigned pressed, int tab)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (tab == 0)
    {
        uint16_t list[TCG_CARD_COUNT];
        unsigned n = SpareList(list);
        MoveCursor(pressed, n ? (int)n + 1 : 0, 1);
        if (pressed & KEY_A && n && U.cursor == (int)n)
        {
            unsigned i, points = 0;
            for (i = 0; i < n; i++)
                points += Tcg_Spare(c, list[i]) * Tcg_PointValue(list[i]);
            Ask(ACT_TRADE_IN_EVERY, 0, "Trade in every spare card for %u CP?", points);
        }
        else if (U.cursor == (int)n)
            ;
        else if (pressed & KEY_A && n)
            Ask(ACT_TRADE_IN, list[U.cursor], "Trade in one %s for %u CP?", Tcg_Card(list[U.cursor])->name,
                Tcg_PointValue(list[U.cursor]));
        else if (pressed & KEY_SELECT && n)
            Ask(ACT_TRADE_IN_ALL, list[U.cursor], "Trade in all %u spare %s for %u CP?", Tcg_Spare(c, list[U.cursor]),
                Tcg_Card(list[U.cursor])->name, Tcg_Spare(c, list[U.cursor]) * Tcg_PointValue(list[U.cursor]));
    }
    else if (tab == 1)
    {
        MoveCursor(pressed, BUY_COUNT, 1);
        if (pressed & KEY_A)
        {
            if (U.cursor == BUY_CARD)
            {
                Screen(UI_BINDER);
                U.filter = 9;
                U.showAll = SHOP_BINDER;
            }
            else if (c->dust < sBuyCost[U.cursor])
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "You need %u CARD POINTS for that.", sBuyCost[U.cursor]);
            }
            else
                Ask(ACT_BUY, U.cursor, "%s for %u CP?", sBuyNames[U.cursor], sBuyCost[U.cursor]);
        }
    }
    else
    {
        char what[128];
        MoveCursor(pressed, PRIZE_ROWS, 1);
        if (pressed & KEY_A)
        {
            if (PrizeState((unsigned)U.cursor, what, sizeof(what)) == 1)
                ClaimPrize((unsigned)U.cursor);
            else
                PlaySE(SE_FAILURE);
        }
    }
}

int Tcg_Step(unsigned keys)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned pressed = keys & ~U.previousKeys;
    U.previousKeys = keys;
    if (YgoUi_Active())
    {
        U.frame++;
        MP_Sound_TickFanfare();
        return YgoUi_Step(keys);
    }
    if (!Tcg_IsActive())
        return 0;
    MP_Sound_TickFanfare(); /* (the game's tasks don't run while the cards are open) */
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
    OnlineStep();
    if (U.screen == UI_DUEL && !U.message[0])
    {
        DuelStep(pressed, keys);
        return 1;
    }
    /* A message or a question first */
    if (U.message[0])
    {
        if (U.asking)
        {
            if (pressed & (KEY_UP | KEY_DOWN))
                U.askYes ^= 1, PlaySE(SE_SELECT);
            if (pressed & (KEY_A | KEY_B))
            {
                int yes = (pressed & KEY_A) && U.askYes;
                U.message[0] = 0;
                U.asking = 0;
                PlaySE(SE_SELECT);
                if (yes)
                    Act(U.askAction);
            }
        }
        else if (pressed & (KEY_A | KEY_B))
        {
            int then = U.messageThen;
            U.message[0] = 0;
            PlaySE(SE_SELECT);
            Act(then);
        }
        return 1;
    }
    switch (U.screen)
    {
    case UI_HOME:
        MoveCursor(pressed, HOME_ROWS, 1);
        if (pressed & (KEY_L | KEY_R))
        {
            PlaySE(SE_SELECT);
            ToYugioh();
        }
        else if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            switch (U.cursor)
            {
            case 0:
                Screen(UI_DECKS);
                U.cursor = c->activeDeck;
                break;
            case 1:
                U.filter = 9;
                U.showAll = 0;
                Screen(UI_BINDER);
                break;
            case 2:
                Screen(UI_PACKS);
                break;
            case 3:
            {
                const struct TcgDeckDefinition *practice = DeckNamed("PracticePlayer");
                char reason[160];
                if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
                {
                    Say(ACT_NONE, "%s", reason);
                    break;
                }
                memset(&U.opp, 0, sizeof(U.opp) - sizeof(U.opp.portrait));
                snprintf(U.opp.name, sizeof(U.opp.name), "PRACTICE DECK");
                U.opp.deck = practice->cards;
                U.opp.special = TCG_SPECIAL_NONE;
                U.vs = VS_PRACTICE;
                StartDuel(c->decks[c->activeDeck].cards, practice->cards);
                break;
            }
            case 4:
                c->fastAnimations = !c->fastAnimations;
                break;
            case 5:
                Close();
                break;
            }
        }
        break;
    case UI_CHALLENGE:
        MoveCursor(pressed, TCG_SAVED_DECKS + 1, 1);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            ChooseDeck();
        }
        break;
    case UI_DECKS:
        MoveCursor(pressed, TCG_SAVED_DECKS + 1, 1);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == TCG_SAVED_DECKS))
        {
            PlaySE(SE_SELECT);
            Screen(UI_HOME);
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            U.deck = U.cursor;
            if (!c->decks[U.deck].name[0])
                snprintf(c->decks[U.deck].name, TCG_NAME_LEN, "Deck %d", U.deck + 1);
            U.side = 0;
            U.deckCursor = 0;
            Screen(UI_BUILDER);
            U.filter = BuilderTab(U.deck);
        }
        else if (pressed & KEY_R && U.cursor < TCG_SAVED_DECKS && (unsigned)U.cursor != c->activeDeck &&
                 DeckCount(U.cursor, 0))
        {
            char reason[160];
            if (Tcg_DeckValidate(c->decks[U.cursor].cards, c->owned, reason, sizeof(reason)))
            {
                c->activeDeck = (uint8_t)U.cursor;
                PlaySE(SE_SUCCESS);
            }
            else
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "%s", reason);
            }
        }
        else if (pressed & KEY_START && U.cursor < TCG_SAVED_DECKS && DeckCount(U.cursor, 0))
            Ask(ACT_COPY, U.cursor, "Copy %s into an empty slot?", c->decks[U.cursor].name);
        else if (pressed & KEY_SELECT && U.cursor < TCG_SAVED_DECKS && DeckCount(U.cursor, 0))
            Ask(ACT_CLEAR, U.cursor, "Delete %s? Its cards stay in your collection.", c->decks[U.cursor].name);
        break;
    case UI_BUILDER:
    {
        uint16_t list[TCG_CARD_COUNT];
        unsigned listed = DeckList(U.deck, list);
        char reason[160];
        int n = (int)ListSize();
        FilterTab(pressed, 9);
        if (U.side == 0)
        {
            int col = U.cursor % GRID_COLS;
            if (pressed & KEY_RIGHT && (col == GRID_COLS - 1 || U.cursor + 1 >= n) && listed)
            {
                U.side = 1;
                if (U.deckCursor >= (int)listed)
                    U.deckCursor = (int)listed - 1;
                PlaySE(SE_SELECT);
            }
            else
                GridMove(pressed, n, GRID_COLS);
            if (pressed & KEY_A && n)
            {
                unsigned id = ListCard(U.cursor);
                if (Tcg_DeckAdd(c, U.deck, id, reason, sizeof(reason)))
                    PlaySE(SE_RG_CARD_FLIP);
                else
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s", reason);
                }
            }
            /* SELECT takes one back out, without going over to the deck list */
            if (pressed & KEY_SELECT && n)
            {
                unsigned id = ListCard(U.cursor);
                if (Tcg_DeckRemove(c, U.deck, id))
                    PlaySE(SE_RG_BAG_POCKET);
                else
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "There's no %s in this deck.", Tcg_Card(id)->name);
                }
            }
            if (pressed & KEY_START && n)
                Inspect(ListCard(U.cursor), UI_BUILDER);
        }
        else
        {
            int old = U.deckCursor;
            if (pressed & KEY_UP)
                U.deckCursor--;
            if (pressed & KEY_DOWN)
                U.deckCursor++;
            if (U.deckCursor < 0)
                U.deckCursor = 0;
            if (U.deckCursor >= (int)listed)
                U.deckCursor = listed ? (int)listed - 1 : 0;
            if (old != U.deckCursor)
                PlaySE(SE_SELECT);
            if (pressed & KEY_LEFT || !listed)
            {
                /* Back on the card the list points at, when this tab shows it. */
                for (int k = 0; listed && k < n; k++)
                    if (ListCard(k) == list[U.deckCursor])
                        U.cursor = k;
                U.side = 0;
                PlaySE(SE_SELECT);
            }
            else if (pressed & KEY_A)
            {
                Tcg_DeckRemove(c, U.deck, list[U.deckCursor]);
                PlaySE(SE_RG_BAG_POCKET);
                if (DeckList(U.deck, list) <= (unsigned)U.deckCursor && U.deckCursor)
                    U.deckCursor--;
            }
            else if (pressed & KEY_START)
                Inspect(list[U.deckCursor], UI_BUILDER);
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(UI_DECKS);
            U.cursor = U.deck;
            if (DeckCount(U.deck, 0) && !Tcg_DeckValidate(c->decks[U.deck].cards, c->owned, reason, sizeof(reason)))
                Say(ACT_NONE, "This deck can't be played yet: %s", reason);
        }
        break;
    }
    case UI_BINDER:
        FilterTab(pressed, 10);
        GridMove(pressed, (int)ListSize(), 7);
        if (U.showAll == GC_BINDER)
        {
            if (pressed & KEY_A && ListSize())
            {
                unsigned id = ListCard(U.cursor), price = Tcg_ShopPrice(id);
                if (GcSingleSold(id))
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "SOLD: one copy of each card per customer.");
                    printf("[tcg] game center: card %u sold\n", id);
                }
                else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s is \xa5%u. You don't have enough money.", Tcg_Card(id)->name, price);
                }
                else if (c->owned[id] >= TCG_COLLECTION_MAX)
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "You can't carry more %s.", Tcg_Card(id)->name);
                }
                else
                    Ask(ACT_GC_CARD, (int)id, "%s for \xa5%u?", Tcg_Card(id)->name, price);
            }
            if (pressed & KEY_START && ListSize())
                Inspect(ListCard(U.cursor), UI_BINDER);
            if (pressed & KEY_B)
            {
                PlaySE(SE_SELECT);
                U.showAll = 0;
                Screen(UI_GCSHOP);
                U.tab = 0;
            }
            break;
        }
        if (U.showAll == SHOP_BINDER)
        {
            if (pressed & KEY_A && ListSize())
            {
                unsigned id = ListCard(U.cursor), cost = Tcg_PointCost(id);
                if (c->dust < cost)
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s costs %u CP. You have %u.", Tcg_Card(id)->name, cost, c->dust);
                }
                else
                    Ask(ACT_BUY, 1000 + (int)id, "%s for %u CP?", Tcg_Card(id)->name, cost);
            }
            if (pressed & KEY_START && ListSize())
                Inspect(ListCard(U.cursor), UI_BINDER);
            if (pressed & KEY_B)
            {
                /* back to the counter it came from: the CARD CLUB's, or the GAME CENTER's fourth tab */
                PlaySE(SE_SELECT);
                U.showAll = 0;
                Screen(U.counterHome == UI_GCSHOP ? UI_GCSHOP : UI_COUNTER);
                U.tab = U.counterHome == UI_GCSHOP ? 3 : 1;
                U.cursor = BUY_CARD;
            }
            break;
        }
        if (pressed & KEY_SELECT)
        {
            U.showAll = !U.showAll;
            U.cursor = 0;
            PlaySE(SE_SELECT);
        }
        if (pressed & (KEY_A | KEY_START) && ListSize())
            Inspect(ListCard(U.cursor), UI_BINDER);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(UI_HOME);
            U.cursor = 1;
        }
        break;
    case UI_INSPECT:
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
            U.screen = UI_ART;
            U.zoom = U.panX = U.panY = 0;
        }
        else if (pressed & (KEY_B | KEY_START | KEY_R))
        {
            /* (START or R, which opened it, close it too: as on the Yu-Gi-Oh! side) */
            PlaySE(SE_SELECT);
            U.screen = U.back;
            U.age = 100;
        }
        break;
    case UI_ART:
        if (keys & KEY_UP)
            U.panY -= 2;
        if (keys & KEY_DOWN)
            U.panY += 2;
        if (keys & KEY_LEFT)
            U.panX -= 2;
        if (keys & KEY_RIGHT)
            U.panX += 2;
        U.panY = U.panY < -80 ? -80 : U.panY > 80 ? 80 : U.panY;
        U.panX = U.panX < -60 ? -60 : U.panX > 60 ? 60 : U.panX;
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            U.zoom = !U.zoom;
            U.panX = U.panY = 0;
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            U.screen = UI_INSPECT;
        }
        break;
    case UI_PACKS:
    {
        int old = U.cursor;
        if (pressed & KEY_LEFT)
            U.cursor = (U.cursor + 3) % 4;
        if (pressed & KEY_RIGHT)
            U.cursor = (U.cursor + 1) % 4;
        if (old != U.cursor)
            PlaySE(SE_SELECT);
        if (pressed & KEY_A)
        {
            U.pack = U.cursor;
            unsigned total, before = Tcg_SetOwned(c, U.pack, &total);
            if (Tcg_OpenPack(c, U.pack, U.revealed))
            {
                unsigned k, j;
                Screen(UI_REVEAL);
                U.flipped = 0;
                /* NEW: the first copy of a card the player didn't have */
                U.revealNew = 0;
                for (k = 0; k < 10; k++)
                {
                    unsigned copies = 0, earlier = 0;
                    for (j = 0; j < 10; j++)
                        copies += U.revealed[j] == U.revealed[k];
                    for (j = 0; j < k; j++)
                        earlier |= U.revealed[j] == U.revealed[k];
                    if (U.revealed[k] > 6 && !earlier && c->owned[U.revealed[k]] == copies)
                        U.revealNew |= 1u << k;
                }
                U.revealComplete = before < total && Tcg_SetOwned(c, U.pack, &total) == total;
                if (U.revealComplete)
                    printf("[tcg] set complete: %d\n", U.pack);
                PlaySE(SE_RG_CARD_OPEN);
                MP_Achieve_Count("cardpack", 1);
                CollectionChanged();
                printf("[tcg] opened pack %d\n", U.pack);
            }
            else
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "You have no unopened packs of this kind.");
            }
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(UI_HOME);
            U.cursor = 2;
        }
        break;
    }
    case UI_REVEAL:
        if (U.flipped < 10)
        {
            RevealStep(pressed);
            if (pressed & KEY_B)
                U.flipped = 10;
            break;
        }
        GridMove(pressed, 10, 5);
        if (pressed & (KEY_A | KEY_START))
            Inspect(U.revealed[U.cursor], UI_REVEAL);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(UI_PACKS);
            U.cursor = U.pack;
        }
        break;
    case UI_SPOILS:
        GridMove(pressed, U.spoilCount, 7);
        if (pressed & KEY_A && U.spoilCount)
            Ask(ACT_TAKE_SPOIL, U.cursor, "Take %s?", Tcg_Card(U.spoils[U.cursor])->name);
        if (pressed & KEY_START && U.spoilCount)
            Inspect(U.spoils[U.cursor], UI_SPOILS);
        break;
    case UI_GCSHOP:
        GcShopStep(pressed);
        break;
    case UI_COUNTER:
        if (pressed & (KEY_L | KEY_R))
        {
            U.tab = (U.tab + (pressed & KEY_R ? 1 : 2)) % 3;
            U.cursor = 0;
            PlaySE(SE_RG_BAG_POCKET);
        }
        else
            CounterStep(pressed, U.tab);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        break;
    case UI_STARTER:
        if (U.age >= STARTER_READY)
        {
            int old = U.starterChoice;
            if (pressed & KEY_LEFT)
                U.starterChoice = (U.starterChoice + 2) % 3;
            if (pressed & KEY_RIGHT)
                U.starterChoice = (U.starterChoice + 1) % 3;
            if (old != U.starterChoice)
                PlaySE(SE_SELECT);
            if (pressed & KEY_A)
                Ask(ACT_STARTER, U.starterChoice, "Take the %s deck?", sStarterNames[U.starterChoice]);
            else if (pressed & KEY_START)
                Inspect(sStarterIds[U.starterChoice], UI_STARTER);
        }
        else if (pressed & KEY_A && U.age > 30)
            U.age = STARTER_READY;
        break;
    case UI_TRADE:
        TradeStep(pressed);
        break;
    case UI_WAIT:
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        break;
    }
    return 1;
}
int Tcg_HandleEvent(const union SDL_Event *event)
{
    (void)event;
    return 0;
}

/* ------------------------------------------------------------------ */
/* The canvas                                                           */
/* ------------------------------------------------------------------ */
static SDL_Texture *sCanvas;
static SDL_Renderer *sCanvasOwner;
static uint16_t sFrame[240 * 160];
static int sFrameRead;
void Tcg_DestroyTextures(void)
{
    Ygo_ForgetTextures();
    G_Destroy();
    if (sCanvas)
        SDL_DestroyTexture(sCanvas);
    sCanvas = NULL;
    sCanvasOwner = NULL;
    if (U.portrait)
        SDL_DestroyTexture(U.portrait);
    U.portrait = NULL;
}
SDL_Texture *Tcg_Canvas(SDL_Renderer *r, SDL_Texture *game)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!Tcg_IsActive())
        return NULL;
    G_Begin(r);
    G_SetArtMode(gMpConfig.cardArt);
    G_SetBackMode(gMpConfig.cardBack);
    if (sCanvas && sCanvasOwner != r)
        sCanvas = NULL;
    if (!sCanvas)
    {
        sCanvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 240, 160);
        sCanvasOwner = r;
        if (!sCanvas)
            return NULL;
        SDL_SetTextureBlendMode(sCanvas, SDL_BLENDMODE_NONE);
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
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    if (game)
        SDL_RenderCopy(r, game, NULL, NULL);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    switch (U.screen)
    {
    case UI_HOME:
        HomeScreen();
        break;
    case UI_CHALLENGE:
        ChallengeScreen();
        break;
    case UI_DECKS:
        DecksScreen();
        break;
    case UI_BUILDER:
        BuilderScreen();
        break;
    case UI_BINDER:
        BinderScreen();
        break;
    case UI_INSPECT:
        InspectScreen();
        break;
    case UI_ART:
        ArtScreen();
        break;
    case UI_PACKS:
        PacksScreen();
        break;
    case UI_REVEAL:
        RevealScreen();
        break;
    case UI_SPOILS:
        SpoilsScreen();
        break;
    case UI_COUNTER:
        CounterScreen();
        break;
    case UI_GCSHOP:
        GcShopScreen();
        break;
    case UI_STARTER:
        StarterScreen();
        break;
    case UI_TRADE:
        TradeScreen();
        break;
    case UI_WAIT:
        WaitScreen();
        break;
    case UI_DUEL:
        TcgBoard_Render();
        break;
    default:
        if (YgoUi_Active())
            YgoUi_Draw();
        break;
    }
    MessageBox();
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    sFrameRead = 0;
    return sCanvas;
}
/* The last card screen as a game frame (15-bit, like the GBA picture). */
const unsigned short *Tcg_Frame(void)
{
    SDL_Texture *previous;
    if (!Tcg_IsActive() || !sCanvas || !sCanvasOwner)
        return NULL;
    if (!sFrameRead)
    {
        previous = SDL_GetRenderTarget(sCanvasOwner);
        SDL_SetRenderTarget(sCanvasOwner, sCanvas);
        if (SDL_RenderReadPixels(sCanvasOwner, NULL, SDL_PIXELFORMAT_ABGR1555, sFrame, 240 * 2) != 0)
            memset(sFrame, 0, sizeof(sFrame));
        SDL_SetRenderTarget(sCanvasOwner, previous);
        sFrameRead = 1;
    }
    return sFrame;
}

/* The CARD CLUB's lesson: a loaner deck against CARD CLUB SAM, with tips. */
void Tcg_OpenTeachingDuel(void)
{
    const struct TcgDeckDefinition *mine = DeckNamed("PracticePlayer"), *theirs = DeckNamed("SamsPractice");
    U.returnToGame = NULL;
    memset(&U.opp, 0, sizeof(U.opp) - sizeof(U.opp.portrait));
    snprintf(U.opp.name, sizeof(U.opp.name), "CARD CLUB SAM");
    U.opp.deck = theirs->cards;
    U.opp.special = TCG_SPECIAL_NONE;
    U.opp.mat = TCG_MAT_CLASSIC;
    U.vs = VS_LESSON;
    Open(UI_DUEL);
    U.fromScript = 1;
    StartDuel(mine->cards, theirs->cards);
}

/* ------------------------------------------------------------------ */
/* The test harness (mp cards ...)                                      */
/* ------------------------------------------------------------------ */
void Tcg_Debug(const char *command)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned n = 0, m = 0;
    int skill;
    if (!strcmp(command, "challenge"))
    {
        printf("[tcg] field challenge: %d\n", Tcg_FieldChallenge());
        return;
    }
    if (!strncmp(command, "open", 4) && (!command[4] || command[4] == ' '))
    {
        /* open [PRIZES] */
        struct TcgOpponent o;
        const struct TcgDeckDefinition *d = DeckNamed("PracticePlayer");
        memset(&o, 0, sizeof(o));
        snprintf(o.name, sizeof(o.name), "TEST COLLECTOR");
        o.deck = d->cards;
        o.special = TCG_SPECIAL_NONE;
        o.pool = TCG_SCALE_POOL_NONE;
        o.key = 0x7e57;
        o.prizes = command[4] ? (uint8_t)atoi(command + 5) : 0;
        Tcg_OpenNpcChallenge(&o);
        return;
    }
    if (sscanf(command, "skill %d", &skill) == 1)
    {
        c->skill = (int16_t)skill;
        return;
    }
    if (!strcmp(command, "rate"))
    {
        printf("[tcg] deck %u rated %d, skill %d after %u duels\n", c->activeDeck,
               TcgScale_RateDeck(c->decks[c->activeDeck].cards), c->skill, c->rated);
        return;
    }
    if (sscanf(command, "special %u", &n) == 1 && n < TCG_SPECIAL_COUNT)
    {
        struct TcgOpponent o;
        memset(&o, 0, sizeof(o));
        snprintf(o.name, sizeof(o.name), "%s", gTcgSpecialDecks[n].name);
        o.deck = gTcgSpecialDecks[n].cards;
        o.special = n;
        o.key = 0xFF000000u | n;
        o.promo = Tcg_SpecialPromo(n);
        o.mat = n < TCG_GYM_LEADERS ? TCG_MAT_GYM : TCG_MAT_LEAGUE;
        o.pool = TCG_SCALE_POOL_SPECIAL;
        Tcg_FitOpponent(&o, NULL);
        Tcg_OpenNpcChallenge(&o);
        return;
    }
    if (sscanf(command, "starter %u", &n) == 1)
    {
        Tcg_ChooseStarter(c, n % 3);
        Tcg_GrantCase();
        return;
    }
    if (!strcmp(command, "case"))
    {
        Tcg_OpenCase(NULL);
        return;
    }
    if (!strcmp(command, "reveal"))
    {
        Tcg_OpenStarterReveal();
        U.fromScript = 0;
        return;
    }
    if (!strcmp(command, "club"))
    {
        Tcg_OpenClubCounter();
        U.fromScript = 0;
        return;
    }
    if (!strcmp(command, "lesson"))
    {
        Tcg_OpenTeachingDuel();
        U.fromScript = 0;
        return;
    }
    if (sscanf(command, "points %u", &n) == 1)
    {
        c->dust = n;
        return;
    }
    if (sscanf(command, "drought %u %u", &n, &m) == 2 && n < TCG_PACK_TYPES)
    {
        c->drought[n] = (uint8_t)m;   /* packs of set n since its last new holo */
        return;
    }
    if (!strcmp(command, "giveall"))
    {
        unsigned id;
        for (id = 1; id <= TCG_CARD_COUNT; id++)
            if (!c->owned[id])
                c->owned[id] = 1;
        CollectionChanged();
        return;
    }
    if (sscanf(command, "own %u %u", &n, &m) == 2 && n && n <= TCG_CARD_COUNT)
    {
        c->owned[n] = (uint16_t)m;
        return;
    }
    if (sscanf(command, "inspect %u", &n) == 1)
    {
        if (U.screen == UI_CLOSED)
            Open(UI_HOME);
        Inspect(n, U.screen == UI_INSPECT ? UI_HOME : U.screen);
        return;
    }
    if (!strcmp(command, "collection"))
    {
        if (U.screen == UI_CLOSED)
            Open(UI_HOME);
        Screen(UI_BINDER);
        U.showAll = 1;
        U.filter = 9;
        return;
    }
    if (!strcmp(command, "packs"))
    {
        if (c->optedIn)
        {
            Tcg_AwardPack(c, 0);
            if (U.screen == UI_CLOSED)
                Open(UI_HOME);
            Screen(UI_PACKS);
        }
        return;
    }
    if (!strcmp(command, "builder"))
    {
        if (U.screen == UI_CLOSED)
            Open(UI_HOME);
        U.deck = c->activeDeck;
        U.side = 0;
        Screen(UI_BUILDER);
        U.filter = BuilderTab(U.deck);
        return;
    }
    if (!strcmp(command, "concede"))
    {
        /* The board's CONCEDE, confirmed (online or not) */
        struct TcgAction a = {0};
        a.type = TCG_CONCEDE;
        if (U.screen == UI_DUEL)
            Apply(&a);
        printf("[tcg] concede: screen %d online %d link %u pending %u held %d phase %u\n", U.screen, U.online,
               Tcg_Online()->state, Tcg_Online()->pending, U.concedeHeld, U.duel.phase);
        return;
    }
    if (!strcmp(command, "netconcede"))
    {
        struct TcgAction a = {0};
        a.type = TCG_CONCEDE;
        TcgLink_Submit(Tcg_Online(), &a, SDL_GetTicks());
        return;
    }
    if (!strcmp(command, "netstate"))
    {
        struct TcgLink *l = Tcg_Online();
        printf("[tcg-net] state %u seq %u phase %u hash %08x winner %u social %u\n", l->state, l->sequence, l->game.phase,
               TcgLink_Hash(&l->game), l->game.winner, Tcg_Social()->role);
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
    if (sscanf(command, "ask %u %u", &n, &m) == 2)
    {
        Tcg_AskFriend(n, m);
        return;
    }
    if (sscanf(command, "offer %u", &n) == 1)
    {
        Tcg_SocialOffer(n);
        return;
    }
    if (!strcmp(command, "confirm"))
    {
        Tcg_SocialConfirm();
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
    if (!strcmp(command, "clear"))
    {
        /* Harness helper: every Pokemon in play back into its deck, for staging a board
         * with put / attach / damage */
        unsigned who, s, i;
        for (who = 0; who < 2; who++)
        {
            struct TcgPlayer *p = &U.duel.players[who];
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                struct TcgFieldCard *f = &p->field[s];
                for (i = 0; i < f->stackSize; i++)
                    p->deck[p->deckSize++] = f->stack[i];
                for (i = 0; i < f->attachedSize; i++)
                    p->deck[p->deckSize++] = f->attached[i];
                memset(f, 0, sizeof(*f));
            }
        }
        return;
    }
    {
        unsigned who, slot, id, count;
        int put = sscanf(command, "put %u %u %u", &who, &slot, &id) == 3;
        int attach = sscanf(command, "attach %u %u %u %u", &who, &slot, &id, &count) == 4;
        int hand = sscanf(command, "hand %u", &id) == 1;
        if (hand)
            who = 0, slot = 0, count = 1;
        if (put)
            count = 1;
        if ((put || attach || hand) && who < 2 && slot < TCG_FIELD_SIZE && id && id <= TCG_CARD_COUNT)
        {
            /* Harness helper: a card of this kind from the deck (a basic Energy card there
             * becomes one if none is left) on top of a Pokemon in play, attached, or in the hand */
            struct TcgPlayer *p = &U.duel.players[who];
            struct TcgFieldCard *f = &p->field[slot];
            while (count--)
            {
                unsigned i, k = p->deckSize;
                uint8_t card;
                for (i = 0; i < p->deckSize && k == p->deckSize; i++)
                    if (p->cards[p->deck[i]] == id)
                        k = i;
                for (i = 0; i < p->deckSize && k == p->deckSize; i++)
                    if (p->cards[p->deck[i]] <= 6 && p->cards[p->deck[i]] != id)
                        k = i, p->cards[p->deck[i]] = (uint16_t)id;
                if (k == p->deckSize)
                    break;
                card = TcgPile_Remove(p->deck, &p->deckSize, k);
                if (hand)
                    p->hand[p->handSize++] = card;
                else if (put)
                {
                    f->stack[f->stackSize++] = card;
                    f->enteredTurn = 1;
                }
                else
                    f->attached[f->attachedSize++] = card;
            }
            printf("[tcg] staged %s %u %u card %u conserved %d hand %u deck %u\n", hand ? "hand" : put ? "put" : "attach", who,
                   slot, id, TcgDuel_Conserved(&U.duel), p->handSize, p->deckSize);
            return;
        }
        if (sscanf(command, "damage %u %u %u", &who, &slot, &count) == 3 && who < 2 && slot < TCG_FIELD_SIZE)
        {
            U.duel.players[who].field[slot].damage = (uint16_t)count;
            return;
        }
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
    if (!strcmp(command, "win"))
    {
        /* Harness helper: the opponent concedes (a win without playing it out). */
        struct TcgAction a = {0};
        a.type = TCG_CONCEDE;
        U.duel.current = 1;
        TcgDuel_Apply(&U.duel, &a);
        printf("[tcg] opponent conceded\n");
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
        unsigned total, have = Tcg_SetOwned(c, TCG_SET_COUNT, &total);
        printf("[tcg] screen %d opted %u phase %u current %u wins %u hand %u field %u bench %u%u%u%u%u idle %d "
               "energy %u discard %u owned %u points %u packs %u%u%u%u flags %x special %x message %d\n",
               U.screen, c->optedIn, U.duel.phase, U.duel.current, c->wins, p->handSize, p->field[0].stackSize,
               p->field[1].stackSize, p->field[2].stackSize, p->field[3].stackSize, p->field[4].stackSize,
               p->field[5].stackSize, U.screen == UI_DUEL ? TcgBoard_Idle() : -1,
               TcgDuel_Energy(&U.duel, 0, 0, TCG_NONE), p->discardSize, have, c->dust, c->packs[0], c->packs[1],
               c->packs[2], c->packs[3], c->flags, c->specialBeaten, U.message[0] != 0);
    }
}
