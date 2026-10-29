/* The CARD CASE's YU-GI-OH! side: decks (a Main Deck of 40 to 60 and a Fusion Deck of up to 15), the binder, the
 * boosters with their rarities, a duelist's challenge, the duel table (ygo_board.c), the card you take after a
 * win, the CARD CLUB duelist's Starter Decks and counter. Drawn with the Pokemon card screens' look
 * (src/tcg/tcg_ui_kit.h) through their canvas. */
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "money.h"
#include "script.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg/integration.h"
#include "tcg/npc.h"
#include "tcg/online.h"
#include "tcg/tcg.h"
#include "ygo/card_ids.h"
#include "ygo/duel.h"
#include "ygo/integration.h"
#include "ygo/net.h"
#include "ygo/npc.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_ui_kit.h"
#include "ygo_board.h"
#include "ygo_gfx.h"
#include "platform_text.h"

enum
{
    Y_CLOSED,
    Y_HOME,
    Y_DECKS,
    Y_BUILDER,
    Y_BINDER,
    Y_INSPECT,
    Y_ART,
    Y_PACKS,
    Y_REVEAL,
    Y_CHALLENGE,
    Y_DUEL,
    Y_SPOILS,
    Y_COUNTER,
    Y_STARTER,
    Y_WAIT,    /* a friend's duel: waiting for them to pick a deck */
    Y_TRADE,   /* a card trade with a friend */
    Y_RECIPES, /* DECK RECIPES: a Starter Deck built again from the cards owned */
    Y_ACE,     /* a deck's ACE CARD */
    Y_CODE     /* FROM CODE: a deck from a YDKE code */
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
    KEY_L = 512,
    KEY_INSPECT = 2048   /* R2 / ZR / RT, or D on the keyboard: a card up close (gamepad.h PLATFORM_KEY_INSPECT) */
};
enum
{
    VS_NPC,
    VS_PRACTICE,
    VS_LESSON,
    VS_FRIEND
};
enum { RESOLVE_NONE, RESOLVE_GIFT, RESOLVE_LOAN };
static void ResolveNoDeck(int trade);
enum
{
    ACT_NONE,
    ACT_CLOSE,
    ACT_HOME,
    ACT_TAKE_SPOIL,
    ACT_STARTER,
    ACT_CLEAR,
    ACT_TRADE_IN,        /* one copy */
    ACT_TRADE_IN_CARD,   /* every spare copy of one card */
    ACT_TRADE_IN_ALL,    /* every spare card */
    ACT_BUY_PACK,
    ACT_BUY_CARD,
    ACT_GC_CARD,
    ACT_COPY,
    ACT_LOAN,
    ACT_GC_DECK,
    ACT_LESSON,
    ACT_RECIPE   /* build DECK RECIPE askArg into slot Y.recipeSlot */
};
#define PANEL_W KIT_PANEL_W
#define STRIP_Y KIT_STRIP_Y
#define LIST_X KIT_LIST_X
#define LIST_W KIT_LIST_W
#define ROW_H KIT_ROW_H
#define SHOP_BINDER 2   /* Y.showAll: the binder is the counter's "card you choose" list */
#define GC_BINDER 3     /* Y.showAll: the binder is the GAME CENTER clerk's singles, for money */
#define GC_DECK_PRICE 10000
#define FILTERS 7
static const char *const sFilterNames[FILTERS] = {"ALL", "NORMAL", "EFFECT", "FUSION", "RITUAL", "SPELL", "TRAP"};
#define YGO_THEME 0x6848b8   /* the case's YU-GI-OH! side: a deep violet */
#define YGO_STARTERS 4
static const uint8_t sStarters[YGO_STARTERS] = {YGO_SET_SDY, YGO_SET_SDK, YGO_SET_SDJ, YGO_SET_SDP};

static struct
{
    int screen, back, cursor, scroll, frame, age;
    unsigned previousKeys, held;
    int card, inspectScroll, inspectMax, zoom, panX, panY;
    int deck, filter, showAll, side, deckCursor;
    int pack, flipped;
    uint16_t revealed[YGO_PACK_SIZE];
    uint8_t revealRarity[YGO_PACK_SIZE];
    uint16_t revealNew;
    int vs, aiDelay, autoPlay, rewarded;
    struct YgoOpponent opp;
    struct YgoDuel duel;
    uint8_t duelShown;   /* duel is a view of the online duel's (Ygo_Online()->game): never freed from here */
    SDL_Texture *portrait;
    uint16_t spoils[YGO_DECK_MAX + YGO_FUSION_MAX];
    int spoilCount;
    int tab, counterTab, gc;   /* gc: the counter is the GAME CENTER clerk's (SINGLES and DECKS first) */
    char message[256];
    int messageThen, asking, askYes, askAction, askArg;
    int fromScript;
    void (*returnToGame)(void);
    int online, onlineDeck, onlineDeckChosen, concedeHeld;   /* a duel with a friend (ygo_net.c); onlineDeck -1: loaned */
    /* No Yu-Gi-Oh! deck to duel with (the Pokemon cards only): the Starter Deck screen first, then back to the
     * challenge (RESOLVE_GIFT: a deck to keep; RESOLVE_LOAN: one for this duel) or the trade */
    int resolve, resolveBack;
    uint16_t loanMain[YGO_DECK_MAX], loanExtra[YGO_FUSION_MAX];
    /* DECKS: a slot's menu (EDIT, RENAME...), the recipes' target slot */
    int menuOn, menuCursor, recipeSlot;
    /* FROM CODE: the code typed or pasted (the clipboard's is taken as it comes) and the slot it goes to */
    char code[4096];
    char clipSeen[4096];
    int codeSlot;
} Y;
int Platform_HasFileDialog(void);
int Platform_FileDialog(int save, const char *title, const char *filter, const char *defExt, char *path, int size);
void Platform_SetClipboardText(const char *text);

static struct YgoCollection *C(void) { return Ygo_GetCollection(); }
/* The table's background for a duel against this deck: its monsters' most common Attribute */
static unsigned ThemeOf(const uint16_t *main)
{
    static const uint8_t byAttr[] = {YGO_BG_PLAIN, YGO_BG_LIGHT, YGO_BG_DARK, YGO_BG_WATER, YGO_BG_FIRE, YGO_BG_EARTH,
                                     YGO_BG_WIND, YGO_BG_LIGHT};
    unsigned count[8] = {0}, i, best = 0;
    for (i = 0; i < YGO_DECK_MAX && main[i]; i++)
    {
        const struct YgoCard *c = Ygo_Card(main[i]);
        if (c && c->attribute < 8 && c->attribute != YGO_ATTR_NONE)
            count[c->attribute]++;
    }
    for (i = 1; i < 8; i++)
        if (count[i] > count[best])
            best = i;
    return byAttr[best];
}
static int sForceFirst = -1;   /* test harness: who goes first (-1: a coin) */
static int sMineUntil = -1;    /* test harness: answer for the player until this question (YGO_Q_*) */
static unsigned Points(void) { return Tcg_GetCollection()->dust; }
static void SpendPoints(unsigned n) { Tcg_GetCollection()->dust -= n; }
static void AddPoints(unsigned n) { Tcg_GetCollection()->dust += n; }

static void Say(int then, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(Y.message, sizeof(Y.message), format, ap);
    va_end(ap);
    Y.messageThen = then;
    Y.asking = 0;
}
static void Ask(int action, int arg, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(Y.message, sizeof(Y.message), format, ap);
    va_end(ap);
    Y.asking = 1;
    Y.askYes = 1;
    Y.askAction = action;
    Y.askArg = arg;
    Y.messageThen = ACT_NONE;
}
static void Screen(int screen)
{
    Y.screen = screen;
    Y.cursor = 0;
    Y.scroll = 0;
    Y.age = 0;
}
int YgoUi_Active(void) { return Y.screen != Y_CLOSED; }
unsigned YgoUi_HiddenFrom(void)
{
    unsigned peer = Ygo_Online()->peer;
    return Y.online && Y.screen != Y_CLOSED && peer < MP_MAX_PLAYERS ? 1u << peer : 0;
}
static const char *PeerName(unsigned id)
{
    static char name[MP_NAME_LEN * 2 + 2];
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return "FRIEND";
    MP_GameToAscii(name, gMpPlayers[id].name, MP_NAME_LEN * 2 + 1);
    return name;
}
unsigned YgoUi_DuelingNow(void) { return Y.screen == Y_DUEL || (Y.screen == Y_INSPECT && Y.back == Y_DUEL); }

static void Close(void)
{
    void (*callback)(void) = Y.returnToGame;
    int script = Y.fromScript;
    if (Y.online)
        YgoNet_Cancel(Ygo_Online(), "The duel was closed.");
    Y.online = 0;
    Y.concedeHeld = 0;
    if (Y.vs == VS_FRIEND && (Tcg_Social()->role == TCG_SOCIAL_DECK || Tcg_Social()->role == TCG_SOCIAL_TRADE))
        Tcg_SocialLeave();
    Y.vs = VS_NPC;
    Y.resolve = RESOLVE_NONE;
    if (Y.screen == Y_DUEL || Y.back == Y_DUEL)
        YgoBoard_End();
    Y.message[0] = 0;
    Y.asking = 0;
    Y.showAll = 0;
    Screen(Y_CLOSED);
    Y.returnToGame = NULL;
    Y.fromScript = 0;
    if (Y.portrait)
        SDL_DestroyTexture(Y.portrait);
    Y.portrait = NULL;
    Ygo_ForgetTextures();
    Tcg_CloseScreens(callback, script);
}
static void Open(int screen)
{
    Y.message[0] = 0;
    Y.asking = 0;
    Y.showAll = 0;
    if (Y.portrait)
        SDL_DestroyTexture(Y.portrait);
    Y.portrait = NULL;
    Screen(screen);
    PlaySE(SE_WIN_OPEN);
}
void YgoUi_OpenCase(void (*returnToGame)(void), int fromScript)
{
    Y.returnToGame = returnToGame;
    Y.message[0] = 0;
    Y.asking = 0;
    Y.showAll = 0;
    Screen(Y_HOME);
    Y.fromScript = fromScript;
}
/* To the case's other sides: L the POKeMON side, R the TRIPLE TRIAD side */
static void ToSide(unsigned game)
{
    void (*callback)(void) = Y.returnToGame;
    int script = Y.fromScript;
    Y.returnToGame = NULL;
    Y.fromScript = 0;
    Y.message[0] = 0;
    Screen(Y_CLOSED);
    Ygo_ForgetTextures();
    Tcg_OpenCaseGame(game, callback, script);
}
void YgoUi_OpenChallenge(const struct YgoOpponent *opponent)
{
    Y.returnToGame = NULL;
    Y.fromScript = 0;
    Y.opp = *opponent;
    Y.vs = opponent->online ? VS_FRIEND : VS_NPC;
    Open(Y_CHALLENGE);
    Y.cursor = C()->activeDeck;
    printf("[ygo] challenge: %s, special %u\n", Y.opp.name, Y.opp.special);
    ResolveNoDeck(0);
}
/* A friend said yes to a duel: both pick a deck (the asker's invite carries theirs, ygo_net.c) */
void YgoUi_OpenOnlineDeck(void)
{
    const struct TcgSocial *s = Tcg_Social();
    static struct TcgOpponent face;
    Y.returnToGame = NULL;
    Y.fromScript = 0;
    memset(&Y.opp, 0, sizeof(Y.opp));
    snprintf(Y.opp.name, sizeof(Y.opp.name), "%s", PeerName(s->peer));
    memset(&face, 0, sizeof(face));
    Tcg_PortraitOfPlayer(s->peer, &face);
    Y.opp.portraitW = face.portraitW;
    Y.opp.portraitH = face.portraitH;
    memcpy(Y.opp.portrait, face.portrait, sizeof(Y.opp.portrait));
    Y.opp.special = YGO_SPECIAL_NONE;
    Y.opp.set = YGO_SET_COUNT;
    Y.opp.online = 1;
    Y.opp.music = MUS_RG_UNION_ROOM;
    Open(Y_CHALLENGE);
    Y.vs = VS_FRIEND;
    Y.onlineDeckChosen = 0;
    Y.cursor = C()->activeDeck;
    printf("[ygo] friend's duel: %s, seat %u\n", Y.opp.name, s->seat);
    ResolveNoDeck(0);
}
void YgoUi_OpenOnlineTrade(void)
{
    Y.returnToGame = NULL;
    Y.fromScript = 0;
    memset(&Y.opp, 0, sizeof(Y.opp));
    snprintf(Y.opp.name, sizeof(Y.opp.name), "%s", PeerName(Tcg_Social()->peer));
    Open(Y_TRADE);
    Y.vs = VS_FRIEND;
    Y.filter = 0;
    printf("[ygo] trade with %s\n", Y.opp.name);
    ResolveNoDeck(1);
}
void YgoUi_OpenStarter(void)
{
    Y.returnToGame = NULL;
    Open(Y_STARTER);
    Y.fromScript = 1;
}
void YgoUi_OpenCounter(void)
{
    Y.returnToGame = NULL;
    Y.counterTab = 0;
    Y.gc = 0;
    Open(Y_COUNTER);
    Y.fromScript = 1;
}
void YgoUi_OpenGameCenter(void)
{
    Y.returnToGame = NULL;
    Y.counterTab = 0;
    Y.gc = 1;
    Open(Y_COUNTER);
    Y.fromScript = 1;
}

/* ------------------------------------------------------------------ */
/* Collection helpers                                                   */
/* ------------------------------------------------------------------ */
static struct YgoSavedDeck *Deck(unsigned d) { return &C()->decks[d]; }
static unsigned InDeck(unsigned d, unsigned card)
{
    const struct YgoSavedDeck *k = Deck(d);
    unsigned i, n = 0;
    for (i = 0; i < YGO_DECK_MAX && k->main[i]; i++)
        n += k->main[i] == card;
    for (i = 0; i < YGO_FUSION_MAX && k->extra[i]; i++)
        n += k->extra[i] == card;
    return n;
}
static unsigned MainCount(unsigned d) { return Ygo_DeckCount(Deck(d)->main, YGO_DECK_MAX); }
static unsigned ExtraCount(unsigned d) { return Ygo_DeckCount(Deck(d)->extra, YGO_FUSION_MAX); }
static unsigned MainCountOf(const uint16_t *cards)
{
    unsigned n = 0;
    while (n < YGO_DECK_MAX && cards[n])
        n++;
    return n;
}
static int DeckValid(unsigned d)
{
    return Ygo_DeckValidate(Deck(d)->main, Deck(d)->extra, C()->owned, NULL, 0);
}
/* Someone wants a Yu-Gi-Oh! duel (or trade) with a player who has no deck for it: no Yu-Gi-Oh! cards at all gets a
 * Starter Deck to keep (what the CARD CLUB's duelist would give), cards but no deck ready one to borrow for this duel */
static void ResolveNoDeck(int trade)
{
    unsigned d;
    struct YgoCollection *c = C();
    if (c->optedIn && trade)
        return;
    for (d = 0; d < YGO_SAVED_DECKS && c->optedIn; d++)
        if (DeckValid(d))
            return;
    Y.resolveBack = Y.screen;
    Y.resolve = c->optedIn ? RESOLVE_LOAN : RESOLVE_GIFT;
    Screen(Y_STARTER);
    if (Y.resolve == RESOLVE_GIFT)
        Say(ACT_NONE, "You don't have a Yu-Gi-Oh! deck yet! Pick a Starter Deck: it's yours to keep.");
    else
        Say(ACT_NONE, "None of your decks is ready to duel. Borrow a Starter Deck for this duel!");
    printf("[ygo] no deck: %s\n", Y.resolve == RESOLVE_GIFT ? "a Starter Deck to keep" : "a Starter Deck to borrow");
}
/* A deck's face: its strongest monster */
static unsigned Mascot(const uint16_t *main, unsigned count) { return Ygo_DeckMascot(main, count); }
static int Filter(unsigned id, int filter)
{
    const struct YgoCard *c = Ygo_Card(id);
    static const uint8_t frames[FILTERS] = {0, YGO_NORMAL, YGO_EFFECT, YGO_FUSION, YGO_RITUAL, YGO_SPELL, YGO_TRAP};
    if (!c || c->frame == YGO_TOKEN)
        return 0;
    return !filter || c->frame == frames[filter];
}
/* The cards a list shows: the binder (owned, or every card there is to get), the builder (owned and working) */
static int Listed(unsigned id)
{
    const struct YgoCard *c = Ygo_Card(id);
    if (!c || !Filter(id, Y.filter))
        return 0;
    if (Y.screen == Y_BUILDER)
        return C()->owned[id] && Ygo_CardReady(id);
    if (Y.showAll == SHOP_BINDER)
        return Ygo_PointCost(id) != 0;
    if (Y.showAll == GC_BINDER)
        return Ygo_MoneyPrice(id) != 0;
    return Y.showAll ? Ygo_CardReady(id) || C()->owned[id] : C()->owned[id] > 0;
}
static unsigned ListSize(void)
{
    unsigned id, n = 0;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
        n += Listed(id);
    return n;
}
static unsigned ListCard(int index)
{
    unsigned id;
    int n = 0;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
        if (Listed(id) && n++ == index)
            return id;
    return 0;
}
/* A deck's cards, each kind once: the Main Deck then the Fusion Deck */
static unsigned DeckList(unsigned d, uint16_t *out)
{
    const struct YgoSavedDeck *k = Deck(d);
    unsigned n = 0, i, j;
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        unsigned id = i < YGO_DECK_MAX ? k->main[i] : k->extra[i - YGO_DECK_MAX];
        int seen = 0;
        if (!id)
            continue;
        for (j = 0; j < n; j++)
            seen |= out[j] == id;
        if (!seen)
            out[n++] = (uint16_t)id;
    }
    return n;
}
static void Inspect(unsigned id, int back)
{
    Y.card = (int)id;
    Y.back = back;
    Y.inspectScroll = 0;
    Y.zoom = Y.panX = Y.panY = 0;
    Y.screen = Y_INSPECT;
    Y.age = 0;
}
static void AddCard(unsigned id)
{
    if (id && id <= YGO_CARD_COUNT && C()->owned[id] < YGO_COLLECTION_MAX)
        C()->owned[id]++;
}
/* The GAME CENTER clerk sold their one copy of this single / this Starter Deck */
static int GcSingleSold(unsigned id) { return id && id <= YGO_CARD_COUNT && (C()->gcSingles[id / 8] >> (id % 8)) & 1; }
static int GcDeckSold(unsigned k) { return (C()->gcDecks >> k) & 1; }
/* The rarity a card came out of a set's pack at */
static unsigned RarityIn(unsigned set, unsigned card)
{
    unsigned i;
    if (set < YGO_SET_COUNT)
        for (i = 0; i < gYgoSets[set].count; i++)
            if (gYgoSets[set].cards[i].card == card)
                return gYgoSets[set].cards[i].rarity;
    return Ygo_Card(card) ? Ygo_Card(card)->rarity : YGO_COMMON;
}
static const char *SetName(unsigned set)
{
    const char *n = gYgoSets[set].name;
    return strncmp(n, "Starter Deck: ", 14) ? n : n + 14;
}
static unsigned SetMascot(unsigned set)
{
    const struct YgoSet *s = &gYgoSets[set];
    unsigned i, best = 0, bestRarity = 0;
    int bestAtk = -1;
    for (i = 0; i < s->count; i++)
    {
        const struct YgoCard *c = Ygo_Card(s->cards[i].card);
        int atk = Ygo_IsMonster(c) && c->atk != YGO_STAT_UNKNOWN ? c->atk : -1;
        unsigned r = s->cards[i].rarity >= YGO_ULTRA_RARE ? 2 : s->cards[i].rarity >= YGO_SUPER_RARE ? 1 : 0;
        if (atk < 0)
            continue;
        if (r > bestRarity || (r == bestRarity && atk > bestAtk))
            best = c->id, bestAtk = atk, bestRarity = r;
    }
    return best;
}

/* ------------------------------------------------------------------ */
/* Rarities                                                             */
/* ------------------------------------------------------------------ */
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
    r = (r + 255) / 2, g = (g + 255) / 2, b = (b + 255) / 2;
    return (uint32_t)(r << 16 | g << 8 | b);
}
/* The color a rarity writes the card's name in: Rare silver, Ultra gold, Secret the rainbow */
static unsigned RarityInk(unsigned rarity, int phase)
{
    switch (rarity)
    {
    case YGO_RARE:
        return 0xd8e0e8;
    case YGO_ULTRA_RARE:
        return 0xf8d050;
    case YGO_SECRET_RARE:
        return Rainbow(phase * 4);
    default:
        return G_WHITE;
    }
}
/* The foil over a card of w x h at x, y: Super and Ultra Rare a shine across the picture, Secret Rare a rainbow
 * one, Rare and Ultra Rare a glint along the name */
static void Foil(unsigned rarity, int x, int y, int w, int h, int phase)
{
    int art = w * 13 / 15, ax = x + (w - art) / 2, ay = y + h * 8 / 42;
    if (rarity >= YGO_SUPER_RARE)
    {
        int band = (phase * 2) % (art + 40) - 20, i;
        for (i = 0; i < 6; i++)
        {
            int bx = band + i;
            if (bx < 0 || bx >= art)
                continue;
            G_FillA(ax + bx, ay, 1, art, rarity == YGO_SECRET_RARE ? Rainbow(phase * 6 + i * 20) : 0xffffff,
                    (unsigned)(rarity == YGO_SUPER_RARE ? 70 : 100) - (unsigned)i * 8);
        }
        if (rarity == YGO_ULTRA_RARE)
            G_FillA(ax, ay, art, art, 0xf8d050, 22);
        if (rarity == YGO_SECRET_RARE)
            G_FillA(ax, ay, art, art, Rainbow(phase * 3), 26);
    }
    if (rarity == YGO_RARE || rarity == YGO_ULTRA_RARE || rarity == YGO_SECRET_RARE)
    {
        int g = (phase * 2) % (w + 30) - 10;
        if (g > 0 && g < w - 4)
            G_FillA(x + g, y + 2, 3, h * 4 / 42 + 1, rarity == YGO_RARE ? 0xffffff : 0xfff0a0, 150);
    }
}
static const char *RarityLabel(unsigned rarity)
{
    static const char *const labels[] = {"", "", "", "RARE", "SUPER RARE", "ULTRA RARE", "SECRET RARE"};
    return rarity < 7 ? labels[rarity] : "";
}

/* ------------------------------------------------------------------ */
/* Duels                                                                */
/* ------------------------------------------------------------------ */
/* The online duel shown: a view of the link's duel (its engine stays the link's) */
static void ShowOnline(void)
{
    if (!Y.duelShown)
        YgoDuel_Free(&Y.duel);   /* (an offline duel held before) */
    Y.duel = Ygo_Online()->game;
    Y.duelShown = 1;
}
static int StartDuel(const uint16_t *main, const uint16_t *extra, const uint16_t *theirMain, const uint16_t *theirExtra)
{
    uint32_t seed = Tcg_DuelSeed(&C()->rng);
    if (Y.duelShown)
        Y.duel.edo = NULL;   /* (the online duel's own engine) */
    Y.duelShown = 0;
    if (!YgoDuel_Init(&Y.duel, main, extra, theirMain, theirExtra, seed,
                      sForceFirst >= 0 ? (unsigned)sForceFirst : Tcg_RandomBelow(&C()->rng, 2)))
    {
        Say(ACT_NONE, "This duel couldn't be set up.");
        return 0;
    }
    Y.rewarded = 0;
    Y.aiDelay = 30;
    YgoBoard_SetFast(Tcg_GetCollection()->fastAnimations);
    YgoBoard_SetMusic(Y.opp.music);
    YgoBoard_Tutorial(Y.vs == VS_LESSON);
    YgoBoard_SetTheme(ThemeOf(Y.opp.main));
    Screen(Y_DUEL);
    YgoBoard_Begin(&Y.duel, 0, Y.opp.name, 0);
    printf("[ygo] duel started vs %s\n", Y.opp.name);
    return 1;
}
static void StarterDeckOf(unsigned set, uint16_t *main, uint16_t *extra)
{
    const struct YgoSet *s = &gYgoSets[set];
    unsigned i, k, n = 0, e = 0;
    memset(main, 0, YGO_DECK_MAX * sizeof(*main));
    memset(extra, 0, YGO_FUSION_MAX * sizeof(*extra));
    for (i = 0; i < s->count; i++)
        for (k = 0; k < s->cards[i].copies; k++)
        {
            const struct YgoCard *c = Ygo_Card(s->cards[i].card);
            if (!Ygo_CardReady(c->id))
                continue;
            if (c->frame == YGO_FUSION && e < YGO_FUSION_MAX)
                extra[e++] = c->id;
            else if (c->frame != YGO_FUSION && n < YGO_DECK_MAX)
                main[n++] = c->id;
        }
}
static void Rewards(void)
{
    struct YgoCollection *c = C();
    char line[200] = "";
    int n = 0;
    unsigned set = Y.opp.set < YGO_SET_COUNT && Ygo_SetAvailable(Y.opp.set) ? Y.opp.set : Ygo_RandomSet(&c->rng);
    c->wins++;
    MP_Achieve_Count("ygowin", 1);
    if (Y.duel.s.players[0].lp == YGO_LP)
        MP_Achieve_Unlock("ygoflawless");
    if (Y.duel.s.winReason == YGO_WIN_EXODIA)
        MP_Achieve_Unlock("ygoexodia");
    Ygo_AwardPack(c, set);
    n += snprintf(line + n, sizeof(line) - n, "A %s booster went into your CARD CASE.", gYgoSets[set].code);
    if (Y.opp.special != YGO_SPECIAL_NONE && Y.opp.special < 32)
    {
        uint32_t bit = 1u << Y.opp.special;
        if (!(c->specialBeaten & bit))
        {
            c->specialBeaten |= bit;
            MP_Achieve_Count("ygospecial", 1);
            MP_Achieve_Count("cardspecial", 1);   /* LEGENDS OF THE TABLE counts both games */
        }
        if (Y.opp.signature && !(c->promoGiven & bit))
        {
            c->promoGiven |= bit;
            AddCard(Y.opp.signature);
            MP_Achieve_Unlock("ygosignature");
            snprintf(line + n, sizeof(line) - n, "\n%s handed you their signature card: %s!", Y.opp.name,
                     Ygo_Card(Y.opp.signature)->name);
        }
    }
    if (Y.opp.tcgSpecial != 0xFF)
    {
        unsigned promo = Tcg_GivePromoOf(Y.opp.tcgSpecial);
        if (promo)
            snprintf(line + strlen(line), sizeof(line) - strlen(line), "\nAnd their MOeMON card: %s!", Tcg_Card(promo)->name);
    }
    YgoBoard_Result(line);
    /* Today's full reward: one card of their deck, your pick */
    Y.spoilCount = 0;
    if (Ygo_DailyAvailable(c, Y.opp.key, Tcg_Today()))
    {
        unsigned i, j;
        for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
        {
            unsigned id = i < YGO_DECK_MAX ? Y.opp.main[i] : Y.opp.extra[i - YGO_DECK_MAX];
            int seen = 0;
            if (!id || id == Y.opp.signature)
                continue;
            for (j = 0; j < (unsigned)Y.spoilCount; j++)
                seen |= Y.spoils[j] == id;
            if (!seen)
                Y.spoils[Y.spoilCount++] = (uint16_t)id;
        }
    }
    printf("[ygo] victory vs %s: %s booster, %d cards to choose from\n", Y.opp.name, gYgoSets[set].code, Y.spoilCount);
}
/* A duel with a friend: a booster for the winner, nothing lost */
static void FriendRewards(int won)
{
    struct YgoCollection *c = C();
    unsigned set;
    if (!won)
    {
        YgoBoard_Result("Good game! Nothing was lost.");
        return;
    }
    set = Ygo_RandomSet(&c->rng);
    c->wins++;
    c->onlineWins++;
    MP_Achieve_Count("ygowin", 1);
    MP_Achieve_Count("cardfriend", 1);
    Ygo_AwardPack(c, set);
    {
        char line[96];
        snprintf(line, sizeof(line), "A %s booster went into your CARD CASE.", gYgoSets[set].code);
        YgoBoard_Result(line);
    }
}
/* Our seat: online, the friend who accepted sits in seat 1 */
static unsigned Me(void) { return Y.online ? Ygo_Online()->seat : 0; }
static void DuelStep(unsigned pressed, unsigned keys)
{
    struct YgoAnswer a;
    unsigned card = 0;
    int r;
    if (Y.online)
    {
        struct YgoNet *l = Ygo_Online();
        if (Y.concedeHeld && !l->pending)
        {
            Y.concedeHeld = 0;
            YgoNet_Concede(l, SDL_GetTicks());
        }
        ShowOnline();
        YgoBoard_SetWaiting(l->pending);
    }
    YgoBoard_Sync(&Y.duel);
    if (sMineUntil >= 0 && !Y.online && !Y.duel.s.over && Y.duel.query.player == 0 &&
        Y.duel.query.type != YGO_Q_NONE && YgoBoard_Idle())
    {
        if (Y.duel.query.type == (unsigned)sMineUntil)
        {
            MPNet_Log("ygo mine reached %d on turn %u", sMineUntil, Y.duel.s.turn);
            sMineUntil = -1;
        }
        else
        {
            unsigned i;
            YgoDuel_AiAnswer(&Y.duel, &a);
            if (sMineUntil == YGO_Q_BATTLE && Y.duel.query.type == YGO_Q_IDLE)
                for (i = 0; i < Y.duel.query.count; i++)
                    if (Y.duel.query.items[i].kind == YGO_ACT_TO_BATTLE)
                    {
                        memset(&a, 0, sizeof(a));
                        a.count = 1;
                        a.picks[0] = (uint8_t)i;
                    }
            YgoDuel_Answer(&Y.duel, &a);
        }
    }
    if (Y.duel.s.over && !Y.rewarded && YgoBoard_Idle())
    {
        int won = Y.duel.s.winner == Me();
        Y.rewarded = 1;
        if (Y.vs == VS_FRIEND)
            FriendRewards(won);
        if (Y.vs == VS_NPC && !won)
            C()->losses++;
        if (Y.vs == VS_NPC && won)
            Rewards();
        else if (Y.vs == VS_NPC)
            YgoBoard_Result("No cards, money or progress were lost.");
        else if (Y.vs == VS_LESSON)
        {
            C()->flags |= YGO_FLAG_TUTORIAL;
            YgoBoard_Result(won ? "That's the game! You're ready to duel anyone." : "Close one! You'll get the hang of it.");
        }
        else if (Y.vs != VS_FRIEND)
            YgoBoard_Result(won ? "Nothing was wagered: a practice win!" : "Nothing was wagered.");
    }
    /* online, the harness's autoplay answers for you through the link */
    if (Y.online && Y.autoPlay && !Y.duel.s.over && Y.duel.query.type != YGO_Q_NONE && Y.duel.query.player == Me() &&
        YgoBoard_Idle() && !Ygo_Online()->pending)
    {
        if (Y.aiDelay > 0)
            Y.aiDelay--;
        else
        {
            YgoDuel_AiAnswer(&Y.duel, &a);
            if (!YgoNet_Submit(Ygo_Online(), &a, SDL_GetTicks()))
            {
                memset(&a, 0, sizeof(a));
                if (!YgoNet_Submit(Ygo_Online(), &a, SDL_GetTicks()))
                    YgoNet_Concede(Ygo_Online(), SDL_GetTicks());
            }
            ShowOnline();
            YgoBoard_SetWaiting(Ygo_Online()->pending);
            Y.aiDelay = 4;
        }
    }
    /* the opponent (and the harness's autoplay for you) */
    if (!Y.online && !Y.duel.s.over && Y.duel.query.type != YGO_Q_NONE && YgoBoard_Idle() &&
        (Y.duel.query.player == 1 || Y.autoPlay))
    {
        if (Y.aiDelay > 0)
            Y.aiDelay--;
        else
        {
            YgoDuel_AiAnswer(&Y.duel, &a);
            if (!YgoDuel_Answer(&Y.duel, &a))
            {
                printf("[ygo] AI answer refused: %s\n", Y.duel.error);
                memset(&a, 0, sizeof(a));
                if (!YgoDuel_Answer(&Y.duel, &a))
                    YgoDuel_Concede(&Y.duel, Y.duel.query.player);
            }
            Y.aiDelay = Tcg_GetCollection()->fastAnimations ? 4 : 12;
            if (Y.duel.query.type == YGO_Q_CHAIN || Y.duel.query.type == YGO_Q_CARDS)
                Y.aiDelay = 2;
        }
    }
    r = YgoBoard_Step(pressed, keys, &a, &card);
    if (r == YB_ANSWER && Y.online)
    {
        if (Ygo_Online()->pending)
            ;   /* (an answer is already on its way: the table asks again once it's acknowledged) */
        else if (!YgoNet_Submit(Ygo_Online(), &a, SDL_GetTicks()))
        {
            printf("[ygo] answer refused: %s\n", Ygo_Online()->error);
            PlaySE(SE_FAILURE);
        }
        ShowOnline();
        YgoBoard_SetWaiting(Ygo_Online()->pending);
    }
    else if (r == YB_ANSWER)
    {
        if (!YgoDuel_Answer(&Y.duel, &a))
        {
            printf("[ygo] answer refused: %s\n", Y.duel.error);
            PlaySE(SE_FAILURE);
        }
        Y.aiDelay = Tcg_GetCollection()->fastAnimations ? 6 : 16;
    }
    else if (r == YB_CONCEDE && Y.online)
    {
        /* (one answer at a time on the link: the concession goes as soon as the last one is acknowledged) */
        if (!YgoNet_Concede(Ygo_Online(), SDL_GetTicks()))
            Y.concedeHeld = 1;
    }
    else if (r == YB_CONCEDE)
        YgoDuel_Concede(&Y.duel, 0);
    else if (r == YB_INSPECT)
        Inspect(card, Y_DUEL);
    else if (r == YB_LEAVE)
    {
        int won = Y.duel.s.winner == Me();
        printf("[ygo] left the duel (%s)\n", won ? "won" : "lost");
        if (Y.online)
        {
            YgoNet_Cancel(Ygo_Online(), "Duel complete.");
            Y.online = 0;
        }
        YgoBoard_End();
        if (Y.vs == VS_NPC && won && Y.spoilCount)
        {
            Screen(Y_SPOILS);
            Say(ACT_NONE, "As the winner, you may take one card from %s's deck!", Y.opp.name);
        }
        else if (Y.vs == VS_PRACTICE)
        {
            Screen(Y_HOME);
            Y.cursor = 3;
        }
        else
            Close();
    }
}

/* ------------------------------------------------------------------ */
/* Screens                                                              */
/* ------------------------------------------------------------------ */
static void BigCard(unsigned id, int x, int y, int bob)
{
    G_FillA(x + 2, y + 3, 44, 61, 0x000000, 70);
    Ygo_DrawCard(id, (float)x, (float)(y - bob), 44, 61, 0, 255, 0);
}
static void Strip(unsigned card, const char *text) { TcgUi_Strip(Ygo_DrawCard, card, text); }
/* The kinds of card: a chip each in its frame's color, the chosen one's name beside them */
static void Tabs(int x, int y, int selected)
{
    static const unsigned colors[FILTERS] = {0xe0e0e0, 0xd8b070, 0xd88040, 0x9060c0, 0x5880d0, 0x30a890, 0xc04888};
    int i;
    for (i = 0; i < FILTERS; i++)
    {
        int on = i == selected;
        G_RoundFill(x, y + (on ? 0 : 2), 11, on ? 13 : 10, colors[i], 255);
        G_RoundFrame(x, y + (on ? 0 : 2), 11, on ? 13 : 10, on ? 0xf8f8f8 : 0x202830, 255);
        x += 12;
    }
    G_Hint(x + 2, y, G_WHITE, G_NO_SHADOW, "{L}{R}", TF_NARROW, 1);
    G_Text(x + 24, y - 1, G_WHITE, 0x101820, sFilterNames[selected], 0, TF_NARROW);
}

static const char *const sHomeRows[] = {"DECKS", "BINDER", "BOOSTER PACKS", "PRACTICE DUEL", "DUEL SPEED", "CLOSE CASE"};
static const char *const sHomeHelp[] = {
    "Build decks and pick the one you duel with.", "Every card you own.", "Open the boosters you won.",
    "Against a Starter Deck. Nothing is won or lost.", "How quickly the cards move on the table.",
    "Put the CARD CASE away."};
#define HOME_ROWS 6
static unsigned PacksHeld(void)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_SET_COUNT; i++)
        n += C()->packs[i];
    return n;
}
static void HomeScreen(void)
{
    struct YgoCollection *c = C();
    unsigned i, packs = PacksHeld(), total, have = Ygo_SetOwned(c, YGO_SET_COUNT, &total);
    unsigned mascot = c->optedIn ? Ygo_DeckAce(c, c->activeDeck) : 0;
    char line[128];
    int bob = (int)(sinf(Y.frame * 0.06f) * 1.5f);
    TcgUi_Background();
    TcgUi_CaseTabs(KIT_GAME_YUGIOH, YGO_THEME);
    if (mascot)
        BigCard(mascot, 30, 22, bob);
    G_TextAlign(52, 86, TA_CENTER, G_INK, G_INK_SHADOW,
                c->optedIn && Deck(c->activeDeck)->name[0] ? Deck(c->activeDeck)->name : "No deck", TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < HOME_ROWS; i++)
    {
        const char *right = NULL;
        int enabled = c->optedIn || i >= 4;
        if (i == 2)
            snprintf(line, sizeof(line), "%u", packs), right = line;
        if (i == 4)
            right = Tcg_GetCollection()->fastAnimations ? "FAST" : "NORMAL";
        TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)i, enabled && (i != 2 || packs),
                  sHomeRows[i], right);
    }
    if (!c->optedIn)
        Strip(0, "No Yu-Gi-Oh! deck yet. The CARD CLUB's duelist in a POKeMON MART hands out Starter Decks!");
    else
    {
        snprintf(line, sizeof(line), "%s\nCARDS %u/%u   CARD POINTS %u", sHomeHelp[Y.cursor], have, total, Points());
        Strip(0, line);
    }
    TcgUi_Hints("{L}{R} OTHER GAMES");
}

static void Portrait(int x, int y)
{
    SDL_Rect dst;
    if (!Y.opp.portraitW)
        return;
    if (!Y.portrait)
    {
        struct GSurface s;
        s.w = Y.opp.portraitW;
        s.h = Y.opp.portraitH;
        s.px = Y.opp.portrait;
        Y.portrait = G_SurfaceTexture(&s);
    }
    if (!Y.portrait)
        return;
    if (Y.opp.portraitW == 64)   /* their battle picture, cut to the window beside their card */
    {
        G_PicCut(Y.portrait, Y.opp.portrait, 8, 22, 48, 42);
        return;
    }
    dst.x = x - Y.opp.portraitW;
    dst.y = y - Y.opp.portraitH * 2;
    dst.w = Y.opp.portraitW * 2;
    dst.h = Y.opp.portraitH * 2;
    SDL_RenderCopy(G_R(), Y.portrait, NULL, &dst);
}
static void ChallengeScreen(void)
{
    struct YgoCollection *c = C();
    char line[160];
    unsigned i;
    int y, n;
    TcgUi_Background();
    TcgUi_SidePanel(Y.vs == VS_FRIEND ? "DUEL" : "CHALLENGE", Y.opp.special != YGO_SPECIAL_NONE ? 0x9038a0 : YGO_THEME);
    G_RoundFill(6, 20, 92, 46, 0x2c2c5c, 255);
    G_RoundFrame(6, 20, 92, 46, 0x101030, 255);
    Portrait(28, 62);
    if (Y.opp.main[0])
        Ygo_DrawCard(Mascot(Y.opp.main, YGO_DECK_MAX), 58, 22, 30, 42, 0, 255, 0);
    y = 68;
    if (Y.opp.title[0])
    {
        G_Text(6, y, 0x505868, G_NO_SHADOW, Y.opp.title, 96, TF_NARROW);
        y += 9;
    }
    G_Text(6, y, G_INK, G_INK_SHADOW, Y.opp.name, 96, TF_SMALL);
    y += 12;
    G_Text(6, y + 1, 0x505868, G_NO_SHADOW, "YU-GI-OH!  8000 LP", 96, TF_NARROW);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    G_Text(LIST_X + 6, 5, 0x606878, G_NO_SHADOW, "DUEL WITH", 0, TF_NARROW);
    Y.scroll = Y.cursor < 2 ? 0 : Y.cursor > YGO_SAVED_DECKS - 3 ? YGO_SAVED_DECKS - 4 : Y.cursor - 2;
    for (i = 0; i < 5; i++)
    {
        unsigned d = (unsigned)Y.scroll + i;
        if (d > YGO_SAVED_DECKS)
            break;
        if (d == YGO_SAVED_DECKS)
        {
            TcgUi_Row(LIST_X + 2, 18 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)d, 1, Y.vs == VS_FRIEND ? "LEAVE" : "NOT NOW", NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", Deck(d)->name[0] ? Deck(d)->name : "---");
        TcgUi_Row(LIST_X + 2, 18 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)d, DeckValid(d), line,
                  d == c->activeDeck ? "IN USE" : NULL);
    }
    if (Y.vs == VS_FRIEND)
    {
        Strip(0, "A duel between friends: the winner gets a booster. Nothing is lost.");
        return;
    }
    n = snprintf(line, sizeof(line), "WIN: a booster");
    if (Ygo_DailyAvailable(c, Y.opp.key, Tcg_Today()))
        n += snprintf(line + n, sizeof(line) - n, " and one card from their deck, your pick.");
    else
        n += snprintf(line + n, sizeof(line) - n, ". (Their card again tomorrow.)");
    if (Y.opp.signature && Y.opp.special < 32 && !(c->promoGiven & (1u << Y.opp.special)))
    {
        snprintf(line + n, sizeof(line) - n, "\nFirst win: their signature card!");
        Strip(Y.opp.signature, line);
    }
    else
        Strip(0, line);
}

/* DECK RECIPES: the Starter Decks, card for card, built again from the cards owned */
#define RECIPE_COUNT YGO_STARTERS
static void RecipeCards(int k, uint16_t *main, uint16_t *extra) { StarterDeckOf(sStarters[k], main, extra); }
static unsigned EmptySlot(void)
{
    unsigned d;
    for (d = 0; d < YGO_SAVED_DECKS; d++)
        if (!MainCount(d) && !ExtraCount(d))
            return d;
    return YGO_SAVED_DECKS;
}
/* A slot's menu (A on it), as on the Pokemon side */
enum { DM_EDIT, DM_RENAME, DM_ACE, DM_USE, DM_COPY, DM_CODE, DM_DELETE, DM_EXPORT, DM_COUNT };
enum { EM_NEW, EM_RECIPE, EM_CODE, EM_FILE, EM_COUNT };
static const char *const sDeckMenu[DM_COUNT] = {"EDIT", "RENAME", "ACE CARD", "DUEL WITH IT", "COPY", "DECK CODE",
                                                "DELETE", "EXPORT .YDK"};
static const char *const sEmptyMenu[EM_COUNT] = {"BUILD A DECK", "FROM RECIPE", "FROM CODE", "FROM .YDK FILE"};
/* (.ydk files through the system's file windows: Windows only) */
static int DeckMenuCount(void) { return Platform_HasFileDialog() ? DM_COUNT : DM_COUNT - 1; }
static int EmptyMenuCount(void) { return Platform_HasFileDialog() ? EM_COUNT : EM_COUNT - 1; }
static int SlotEmpty(unsigned d) { return !MainCount(d) && !ExtraCount(d); }
static unsigned DeckMenuOff(unsigned d)
{
    return d == C()->activeDeck || !DeckValid(d) ? 1u << DM_USE : 0;
}
#define DECK_ROWS (YGO_SAVED_DECKS + 3)   /* the slots, the Forbidden & Limited list, DECK RECIPES, BACK */
#define DECK_ROW_LIST YGO_SAVED_DECKS
#define DECK_ROW_RECIPES (YGO_SAVED_DECKS + 1)
#define DECK_ROW_BACK (YGO_SAVED_DECKS + 2)
/* A deck from a code or a .ydk file into slot: made of the game's cards (a card it hasn't got: refused), cards not
 * owned yet in it too (the deck is ready once they are) */
static int DeckFromCards(int slot, const uint16_t *main, const uint16_t *extra, unsigned notHere, int ok, const char *from)
{
    struct YgoSavedDeck *d;
    unsigned missing, total = MainCountOf(main) + MainCountOf(extra);
    if (!ok || slot < 0 || slot >= YGO_SAVED_DECKS)
    {
        PlaySE(SE_FAILURE);
        Say(ACT_NONE, "That isn't a deck code. A YDKE code starts with ydke://");
        return 0;
    }
    if (notHere)
    {
        PlaySE(SE_FAILURE);
        printf("[ygo] deck %s: %u cards not in the game\n", from, notHere);
        Say(ACT_NONE, "This deck contains cards that are not currently implemented. Please try another code.");
        return 0;
    }
    d = Deck((unsigned)slot);
    memcpy(d->main, main, sizeof(d->main));
    memcpy(d->extra, extra, sizeof(d->extra));
    snprintf(d->name, YGO_NAME_LEN, "%s %d", from, slot + 1);
    C()->ace[slot] = 0;
    missing = Ygo_DeckMissing(C(), d->main, d->extra);
    printf("[ygo] deck %s into slot %d: %u cards, %u missing\n", from, slot, total, missing);
    PlaySE(SE_SUCCESS);
    Screen(Y_DECKS);
    Y.cursor = slot;
    if (missing)
        Say(ACT_NONE, "The deck is in slot %d. You have %u of its %u cards: the rest are shown gray, and the deck is "
            "ready once you have them all.", slot + 1, total - missing, total);
    else
        Say(ACT_NONE, "The deck is in slot %d, ready to duel.", slot + 1);
    return 1;
}
static void TakeClipboardCode(void)
{
    char *clip = SDL_HasClipboardText() ? SDL_GetClipboardText() : NULL;
    if (clip && strstr(clip, "ydke://") && strcmp(clip, Y.clipSeen))
    {
        snprintf(Y.clipSeen, sizeof(Y.clipSeen), "%s", clip);
        snprintf(Y.code, sizeof(Y.code), "%s", strstr(clip, "ydke://"));
        Y.code[strcspn(Y.code, "\r\n \t")] = 0;
        PlaySE(SE_SELECT);
        printf("[ygo] code: pasted %u characters\n", (unsigned)strlen(Y.code));
    }
    if (clip)
        SDL_free(clip);
}
static void OpenCode(int slot)
{
    Screen(Y_CODE);
    Y.codeSlot = slot;
    Y.code[0] = 0;
    Y.clipSeen[0] = 0;
    TakeClipboardCode();
    Platform_SetTextInput(1);
}
static void CloseCode(void)
{
    Platform_SetTextInput(0);
    Screen(Y_DECKS);
    Y.cursor = Y.codeSlot;
}
static void UseCode(void)
{
    uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    unsigned notHere = 0;
    int ok = Ygo_ReadYdke(Y.code, main, extra, &notHere);
    if (DeckFromCards(Y.codeSlot, main, extra, notHere, ok, "CODE DECK"))
        Platform_SetTextInput(0);
}
static void CodeScreen(void)
{
    char line[64];
    size_t len = strlen(Y.code), at;
    int row;
    TcgUi_Background();
    TcgUi_SidePanel("DECK CODE", YGO_THEME);
    BigCard(0, 30, 22, 0);
    snprintf(line, sizeof(line), "%u CHARACTERS", (unsigned)len);
    G_TextAlign(52, 86, TA_CENTER, len ? 0x2858a8 : 0xa0a0a0, G_NO_SHADOW, line, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    if (!len)
        G_TextAlign(LIST_X + LIST_W / 2, 46, TA_CENTER, 0x8090a0, G_NO_SHADOW, "Copy a YDKE code: it shows here.",
                    TF_SMALL);
    /* the code, broken into lines; its end when it's long */
    at = len > 24 * 7 ? len - 24 * 7 : 0;
    for (row = 0; row < 7 && at < len; row++, at += 24)
    {
        snprintf(line, sizeof(line), "%.24s", Y.code + at);
        G_Text(LIST_X + 6, 7 + row * 14, G_INK, G_NO_SHADOW, line, LIST_W - 10, TF_NARROW);
    }
    Strip(0, "Paste a YDKE code (ydke://...). A code you copy shows up here by itself.");
    TcgUi_Hints("{A} MAKE THE DECK  {SELECT} CLEAR  {B} BACK");
}
/* .ydk files (Windows: the system's file windows) */
static void ImportYdk(int slot)
{
    static char path[1024];
    uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    unsigned notHere = 0;
    FILE *f;
    char *text;
    long size;
    int ok = 0;
    path[0] = 0;
    if (!Platform_FileDialog(0, "Open a deck", "YGOPro deck (*.ydk)\0*.ydk\0All files\0*.*\0\0", "ydk", path,
                             (int)sizeof(path)))
        return;
    if (!(f = fopen(path, "rb")))
    {
        Say(ACT_NONE, "That file couldn't be opened.");
        return;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    text = size > 0 && size < 1000000 ? malloc((size_t)size + 1) : NULL;
    if (text && fread(text, 1, (size_t)size, f) == (size_t)size)
    {
        text[size] = 0;
        ok = Ygo_ReadYdk(text, main, extra, &notHere);
    }
    fclose(f);
    free(text);
    if (!ok)
    {
        PlaySE(SE_FAILURE);
        Say(ACT_NONE, "That file isn't a deck (.ydk).");
        return;
    }
    DeckFromCards(slot, main, extra, notHere, 1, "YDK DECK");
}
static void ExportYdk(int slot)
{
    static char path[1024], text[8192];
    FILE *f;
    int ok;
    const struct YgoSavedDeck *d = Deck((unsigned)slot);
    if (!Ygo_WriteYdk(d->main, d->extra, d->name, text, sizeof(text)))
        return;
    snprintf(path, sizeof(path), "%s.ydk", d->name);
    if (!Platform_FileDialog(1, "Save the deck", "YGOPro deck (*.ydk)\0*.ydk\0\0", "ydk", path, (int)sizeof(path)))
        return;
    f = fopen(path, "wb");
    ok = f && fwrite(text, 1, strlen(text), f) == strlen(text);
    if (f && fclose(f))
        ok = 0;
    if (ok)
        Say(ACT_NONE, "%s was saved as a .ydk file.", d->name);
    else
        Say(ACT_NONE, "The deck couldn't be saved there.");
}
static void DecksScreen(void)
{
    struct YgoCollection *c = C();
    unsigned i, d = Y.cursor < YGO_SAVED_DECKS ? (unsigned)Y.cursor : c->activeDeck, mascot;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("DECKS", YGO_THEME);
    mascot = Ygo_DeckAce(c, d);
    if (mascot)
        BigCard(mascot, 30, 22, 0);
    snprintf(line, sizeof(line), "%u CARDS  +%u", MainCount(d), ExtraCount(d));
    G_TextAlign(52, 86, TA_CENTER, DeckValid(d) ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    Y.scroll = Y.cursor - 3 < 0 ? 0 : Y.cursor - 3 > DECK_ROWS - 6 ? DECK_ROWS - 6 : Y.cursor - 3;
    for (i = 0; i < 6; i++)
    {
        unsigned k = (unsigned)Y.scroll + i;
        if (k >= DECK_ROWS)
            break;
        if (k == DECK_ROW_BACK || k == DECK_ROW_RECIPES)
        {
            TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)k, 1,
                      k == DECK_ROW_BACK ? "BACK" : "DECK RECIPES", NULL);
            continue;
        }
        if (k == DECK_ROW_LIST)
        {
            snprintf(line, sizeof(line), "LIST: %s", Ygo_BanlistName(Ygo_Banlist()));
            TcgUi_FitText(line, LIST_W - 12, TF_NORMAL);
            TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)k, 1, line, NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", Deck(k)->name[0] ? Deck(k)->name : "EMPTY SLOT");
        TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)k, Deck(k)->name[0] != 0, line,
                  k == c->activeDeck ? "IN USE" : NULL);
    }
    if (Y.cursor == DECK_ROW_BACK)
        Strip(0, "Back to the CARD CASE.");
    else if (Y.cursor == DECK_ROW_LIST)
        Strip(0, "The Forbidden & Limited list for your decks. {LEFT} {RIGHT} changes it.");
    else if (Y.cursor == DECK_ROW_RECIPES)
        Strip(0, "The Starter Decks, built again from the cards you have.");
    else if ((unsigned)Y.cursor == c->activeDeck)
        Strip(0, "The deck you duel with. A Main Deck holds 40 to 60 cards, the Fusion Deck up to 15.");
    else
        Strip(0, MainCount((unsigned)Y.cursor) ? "{R} DUEL WITH THIS DECK" : "An empty slot: build a deck, or one from a recipe.");
    if (Y.menuOn && Y.cursor < YGO_SAVED_DECKS)
    {
        if (SlotEmpty((unsigned)Y.cursor))
            TcgUi_PopMenu(118, 30, 114, sEmptyMenu, EmptyMenuCount(), Y.menuCursor, 0);
        else
            TcgUi_PopMenu(134, 6, 98, sDeckMenu, DeckMenuCount(), Y.menuCursor, DeckMenuOff((unsigned)Y.cursor));
        TcgUi_Hints("{A} CHOOSE  {B} BACK");
    }
    else
        TcgUi_Hints("{A} MENU  {START} COPY  {SELECT} DELETE  {B} BACK");
    TcgUi_NameDraw(YGO_THEME);
}
static void RecipeMissing(const uint16_t *main, const uint16_t *extra, char *out, size_t size, unsigned *lost)
{
    static uint16_t used[YGO_CARD_COUNT + 1], missing[YGO_CARD_COUNT + 1];
    struct YgoCollection *c = C();
    unsigned i, n = 0;
    size_t len;
    memset(used, 0, sizeof(used));
    memset(missing, 0, sizeof(missing));
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        unsigned id = i < YGO_DECK_MAX ? main[i] : extra[i - YGO_DECK_MAX];
        if (!id || id > YGO_CARD_COUNT)
            continue;
        if (used[id]++ >= c->owned[id])
            missing[id]++, n++;
    }
    /* (as many as fit the strip's two lines, then how many more) */
    snprintf(out, size, "Missing:");
    for (i = 1; i <= YGO_CARD_COUNT; i++)
    {
        char more[48];
        unsigned left = 0, j;
        if (!missing[i])
            continue;
        for (j = i + 1; j <= YGO_CARD_COUNT; j++)
            left += missing[j] != 0;
        len = strlen(out);
        snprintf(out + len, size - len, "%s %u %s", len > 8 ? "," : "", missing[i], Ygo_Card(i)->name);
        snprintf(more, sizeof(more), " and %u more", left);
        if (G_Measure(out, TF_SMALL) + (left ? G_Measure(more, TF_SMALL) : 0) > 226 * 2 - 70 || strlen(out) + 40 > size)
        {
            out[len] = 0;
            snprintf(out + len, size - len, " and %u more", left + 1);
            break;
        }
    }
    *lost = n;
}
static void RecipesScreen(void)
{
    uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    unsigned have, total, lost;
    int i;
    char line[200];
    TcgUi_Background();
    TcgUi_SidePanel("RECIPES", YGO_THEME);
    RecipeCards(Y.cursor, main, extra);
    have = Ygo_RecipeOwned(C(), main, extra, &total);
    BigCard(SetMascot(sStarters[Y.cursor]), 30, 22, 0);
    snprintf(line, sizeof(line), "%u/%u OWNED", have, total);
    G_TextAlign(52, 86, TA_CENTER, have == total ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < RECIPE_COUNT && i < 6; i++)
    {
        uint16_t m[YGO_DECK_MAX], e[YGO_FUSION_MAX];
        unsigned h, t;
        char right[16];
        RecipeCards(i, m, e);
        h = Ygo_RecipeOwned(C(), m, e, &t);
        snprintf(right, sizeof(right), "%u/%u", h, t);
        TcgUi_Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, Y.cursor == i, 1, SetName(sStarters[i]), right);
    }
    RecipeMissing(main, extra, line, sizeof(line), &lost);
    if (!lost)
        snprintf(line, sizeof(line), "The %s. You have every card.", gYgoSets[sStarters[Y.cursor]].name);
    Strip(0, line);
    TcgUi_Hints("{A} BUILD  {B} BACK");
}
/* The cards a deck holds, once each, monsters first: the ACE CARD grid */
static unsigned AceList(unsigned deck, uint16_t *out)
{
    unsigned i, j, n = 0, pass;
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
        {
            unsigned id = i < YGO_DECK_MAX ? Deck(deck)->main[i] : Deck(deck)->extra[i - YGO_DECK_MAX];
            const struct YgoCard *k = id ? Ygo_Card(id) : NULL;
            if (!k || (Ygo_IsMonster(k) != 0) != (pass == 0))
                continue;
            for (j = 0; j < n && out[j] != id; j++)
                ;
            if (j == n)
                out[n++] = (uint16_t)id;
        }
    return n;
}
#define ACE_COLS 4
#define ACE_ROWS 2
static void AceScreen(void)
{
    struct YgoCollection *c = C();
    uint16_t list[YGO_DECK_MAX + YGO_FUSION_MAX];
    unsigned n = AceList((unsigned)Y.deck, list), ace = Ygo_DeckAce(c, (unsigned)Y.deck);
    int page = ACE_COLS * ACE_ROWS, start = Y.cursor / page * page, i;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("ACE CARD", YGO_THEME);
    if (n)
        BigCard(list[Y.cursor], 30, 22, (int)(sinf(TcgUi_Frame() * 0.06f) * 1.5f));
    G_TextAlign(52, 86, TA_CENTER, G_INK, G_INK_SHADOW, Deck((unsigned)Y.deck)->name, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < page && start + i < (int)n; i++)
    {
        int on = start + i == Y.cursor, x = LIST_X + 4 + (i % ACE_COLS) * 31, y = 10 + (i / ACE_COLS) * 48 - on * 2;
        Ygo_DrawCard(list[start + i], (float)x, (float)y, 30, 42, 0, 255, 0);
        if (list[start + i] == ace)
        {
            G_RoundFill(x + 3, y + 32, 24, 9, 0xe8a020, 240);
            G_TextAlign(x + 15, y + 31, TA_CENTER, G_WHITE, G_NO_SHADOW, "ACE", TF_NARROW);
        }
        if (on)
        {
            int o = (TcgUi_Frame() / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if ((int)n > page)
    {
        snprintf(line, sizeof(line), "%d/%d", start / page + 1, ((int)n + page - 1) / page);
        G_TextAlign(LIST_X + LIST_W - 5, 95, TA_RIGHT, G_INK, G_NO_SHADOW, line, TF_NARROW);
    }
    if (n)
    {
        snprintf(line, sizeof(line), "%s%s", Ygo_Card(list[Y.cursor])->name,
                 list[Y.cursor] == ace ? ": this deck's ACE CARD." : "");
        Strip(0, line);
    }
    else
        Strip(0, "This deck has no cards to choose from.");
    TcgUi_Hints("{A} MAKE IT THE ACE  {INSPECT} LOOK  {B} BACK");
}

#define GRID_COLS 5
#define GRID_ROWS 2
static void BuilderScreen(void)
{
    struct YgoCollection *c = C();
    int n = (int)ListSize(), i, page = GRID_COLS * GRID_ROWS, start;
    uint16_t list[YGO_DECK_MAX + YGO_FUSION_MAX];
    unsigned listed = DeckList((unsigned)Y.deck, list), id, mainN = MainCount((unsigned)Y.deck),
             extraN = ExtraCount((unsigned)Y.deck);
    char line[64];
    TcgUi_Background();
    G_RoundFill(0, 0, 160, 17, 0x203040, 230);
    Tabs(2, 2, Y.filter);
    start = Y.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        unsigned card = ListCard(start + i);
        int x = 4 + (i % GRID_COLS) * 31, y = 20 + (i / GRID_COLS) * 46, on = Y.side == 0 && start + i == Y.cursor;
        unsigned inDeck = InDeck((unsigned)Y.deck, card);
        if (on)
            y -= 2;
        Ygo_DrawCard(card, (float)x, (float)y, 30, 42, 0, 255, 0);
        snprintf(line, sizeof(line), "%u/%u", inDeck, c->owned[card]);
        G_RoundFill(x + 1, y + 33, 28, 9, inDeck ? 0x2858a8 : 0x404850, 235);
        G_Digits(x + 15 - G_DigitsWidth(line, 0, 1) / 2, y + 35, line, 0, 1, 0xf0f8ff, 255);
        if (Ygo_CopyLimit(card) < YGO_COPIES)
        {
            snprintf(line, sizeof(line), "%u", Ygo_CopyLimit(card));
            G_RoundFill(x + 21, y + 1, 8, 9, Ygo_CopyLimit(card) ? 0xd88020 : 0xc02020, 255);
            G_Digits(x + 23, y + 2, line, 0, 1, 0xffffff, 255);
        }
        if (on)
        {
            int o = (Y.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n)
        G_TextAlign(80, 56, TA_CENTER, G_WHITE, 0x204030, "No cards of this kind.", TF_SMALL);
    if (n > page)
    {
        snprintf(line, sizeof(line), "%d/%d", start / page + 1, (n + page - 1) / page);
        G_TextAlign(157, 3, TA_RIGHT, G_WHITE, 0x101820, line, TF_NARROW);
    }
    TcgUi_ListBox(162, 0, 78, 110);
    snprintf(line, sizeof(line), "%u", mainN);
    G_Text(166, 1, mainN >= YGO_DECK_MIN ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, 0, TF_SMALL);
    snprintf(line, sizeof(line), "+%u", extraN);
    G_Text(184, 2, 0x7048a0, G_NO_SHADOW, line, 0, TF_NARROW);
    if (DeckValid((unsigned)Y.deck))
        G_Text(206, 2, 0x2858a8, G_NO_SHADOW, "READY", 0, TF_NARROW);
    else if (Ygo_DeckMissing(c, Deck((unsigned)Y.deck)->main, Deck((unsigned)Y.deck)->extra))
    {
        snprintf(line, sizeof(line), "-%u", Ygo_DeckMissing(c, Deck((unsigned)Y.deck)->main, Deck((unsigned)Y.deck)->extra));
        G_Text(206, 2, 0xa0a0a0, G_NO_SHADOW, line, 0, TF_NARROW);
    }
    {
        int rows = 9, top = Y.deckCursor < 4 ? 0 : Y.deckCursor - 4;
        if (top > (int)listed - rows)
            top = (int)listed > rows ? (int)listed - rows : 0;
        for (i = 0; i < rows && top + i < (int)listed; i++)
        {
            const struct YgoCard *card = Ygo_Card(list[top + i]);
            int y = 14 + i * 10, on = Y.side == 1 && top + i == Y.deckCursor;
            if (on)
                G_RoundFill(164, y, 74, 10, 0xf8d0c8, 255);
            G_FillA(165, y + 2, 2, 6, Ygo_FrameColor(card->id, 0), 255);
            /* (gray: more in the deck than are owned, a deck made from a code) */
            int short_ = InDeck((unsigned)Y.deck, card->id) > c->owned[card->id];
            snprintf(line, sizeof(line), "%s", card->name);
            TcgUi_FitText(line, 52, TF_NARROW);
            G_Text(169, y - 1, short_ ? 0xa0a0a0 : G_INK, G_NO_SHADOW, line, 0, TF_NARROW);
            snprintf(line, sizeof(line), "x%u", InDeck((unsigned)Y.deck, card->id));
            G_Digits(236 - G_DigitsWidth(line, 0, 1), y + 2, line, 0, 1, short_ ? 0xa0a0a0 : 0x2858a8, 255);
        }
    }
    id = Y.side ? (listed ? list[Y.deckCursor < (int)listed ? Y.deckCursor : 0] : 0) : ListCard(Y.cursor);
    line[0] = 0;
    if (id)
    {
        const struct YgoCard *card = Ygo_Card(id);
        if (Ygo_IsMonster(card))
            snprintf(line, sizeof(line), "%s  ATK %d  DEF %d", card->name, card->atk, card->def);
        else
            snprintf(line, sizeof(line), "%s", card->name);
    }
    Strip(0, line);
    TcgUi_Hints(Y.side ? "{A} TAKE OUT  {LEFT} CARDS  {INSPECT} LOOK  {B} DONE"
                       : "{A} ADD  {SELECT} TAKE OUT  {INSPECT} LOOK  {B} DONE");
}

static void BinderScreen(void)
{
    struct YgoCollection *c = C();
    int n = (int)ListSize(), i, cols = 7, page = 14, start;
    unsigned id = ListCard(Y.cursor), total, have = Ygo_SetOwned(c, YGO_SET_COUNT, &total);
    char line[128];
    TcgUi_Background();
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    Tabs(2, 2, Y.filter);
    snprintf(line, sizeof(line), "%u/%u", have, total);
    G_TextAlign(236, 2, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
    start = Y.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        unsigned card = ListCard(start + i);
        int x = 8 + (i % cols) * 33, y = 21 + (i / cols) * 45, on = start + i == Y.cursor;
        unsigned count = c->owned[card];
        if (on)
            y -= 2;
        Ygo_DrawCard(card, (float)x, (float)y, 30, 42, 0, count ? 255 : 80, 0);
        if (count)
            Foil(Ygo_Card(card)->rarity, x, y, 30, 42, Y.frame + i * 13);
        if (count)
        {
            snprintf(line, sizeof(line), "x%u", count);
            G_RoundFill(x + 14, y + 33, G_DigitsWidth(line, 0, 1) + 4, 9, 0x1c2c48, 230);
            G_Digits(x + 16, y + 35, line, 0, 1, 0xe0f0ff, 255);
        }
        if (Y.showAll == GC_BINDER && GcSingleSold(card))
        {
            /* the clerk's one copy is gone */
            G_RoundFill(x - 1, y - 1, 22, 10, 0xc82828, 240);
            G_Text(x + 1, y - 2, G_WHITE, 0x701010, "SOLD", 0, TF_NARROW);
        }
        if (on)
        {
            int o = (Y.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n)
        G_TextAlign(120, 56, TA_CENTER, G_WHITE, 0x204030, "No cards of this kind yet.", TF_SMALL);
    if (Y.showAll == SHOP_BINDER)
    {
        snprintf(line, sizeof(line), "%s: %u CARD POINTS\nYou have %u. You own %u.", id ? Ygo_Card(id)->name : "",
                 Ygo_PointCost(id), Points(), id ? c->owned[id] : 0);
        Strip(0, line);
        TcgUi_Hints("{A} BUY  {L}{R} TYPE  {INSPECT} LOOK  {B} BACK");
        return;
    }
    if (Y.showAll == GC_BINDER)
    {
        if (id && GcSingleSold(id))
            snprintf(line, sizeof(line), "%s: SOLD\nOne per customer. You own %u.", Ygo_Card(id)->name,
                     c->owned[id]);
        else
            snprintf(line, sizeof(line), "%s: \xa5%u\nYou have \xa5%u. You own %u.", id ? Ygo_Card(id)->name : "",
                     Ygo_MoneyPrice(id), (unsigned)GetMoney(&gSaveBlock1Ptr->money), id ? c->owned[id] : 0);
        Strip(0, line);
        TcgUi_Hints("{A} BUY  {L}{R} TYPE  {INSPECT} LOOK  {B} BACK");
        return;
    }
    line[0] = 0;
    if (id)
    {
        const struct YgoCard *card = Ygo_Card(id);
        snprintf(line, sizeof(line), "%s\n%s%s", card->name, c->owned[id] ? RarityLabel(card->rarity) : "Not in your collection yet.",
                 c->owned[id] && !Ygo_CardReady(id) ? " (not playable yet)" : "");
    }
    Strip(0, line);
    TcgUi_Hints(Y.showAll ? "{A} LOOK  {L}{R} TYPE  {SELECT} OWNED  {B} BACK" : "{A} LOOK  {L}{R} TYPE  {SELECT} ALL  {B} BACK");
}

static void InspectScreen(void)
{
    const struct YgoCard *card = Ygo_Card((unsigned)Y.card);
    char line[200];
    int y, bob = Y.age < 12 ? (int)((12 - Y.age) * 1.5f) : 0, i;
    if (Y.back == Y_DUEL)
    {
        YgoBoard_Render();
        G_FillA(0, 0, 240, 160, 0x000000, 150);
    }
    else
        TcgUi_Background();
    if (!card)
        return;
    G_FillA(10, 34 - bob, 72, 100, 0x000000, 90);
    Ygo_DrawCard(card->id, 8, (float)(30 - bob), 72, 100, 0, 255, 0);
    if (C()->owned[card->id] && Y.back != Y_DUEL)
        Foil(card->rarity, 8, 30 - bob, 72, 100, Y.frame);
    TcgUi_ListBox(86, 4, 152, 140);
    {
        SDL_Rect clip = {90, 8, 144, 132};
        SDL_RenderSetClipRect(G_R(), &clip);
    }
    y = 8 - Y.inspectScroll;
    G_Text(94, y, G_INK, G_INK_SHADOW, card->name, 136, TF_NORMAL);
    y += 15;
    if (Ygo_IsMonster(card))
    {
        static const char *const kinds[] = {"Normal", "Effect", "Fusion", "Ritual"};
        int n = snprintf(line, sizeof(line), "%s  %s / ", Ygo_AttributeName(card->attribute), Ygo_RaceName(card->race));
        if (card->abilities & YGO_ABILITY_FLIP)
            n += snprintf(line + n, sizeof(line) - n, "Flip / ");
        if (card->abilities & YGO_ABILITY_TOON)
            n += snprintf(line + n, sizeof(line) - n, "Toon / ");
        if (card->abilities & YGO_ABILITY_SPIRIT)
            n += snprintf(line + n, sizeof(line) - n, "Spirit / ");
        snprintf(line + n, sizeof(line) - n, "%s", kinds[card->frame]);
        y = G_Text(94, y, 0x586878, G_NO_SHADOW, line, 136, TF_NARROW);
        for (i = 0; i < card->level && i < 12; i++)
            G_FillA(94 + i * 7, y + 2, 5, 5, 0xe85818, 255);
        y += 9;
    }
    else
    {
        static const char *const props[] = {"", "Continuous ", "Equip ", "Field ", "Quick-Play ", "Ritual ", "Counter "};
        snprintf(line, sizeof(line), "%s%s CARD", card->property < 7 ? props[card->property] : "",
                 card->frame == YGO_SPELL ? "SPELL" : "TRAP");
        G_Text(94, y, card->frame == YGO_SPELL ? 0x208868 : 0xb02870, G_NO_SHADOW, line, 136, TF_NARROW);
        y += 11;
    }
    G_FillA(92, y, 140, 1, 0xd8d8d0, 255);
    y = G_Text(94, y + 2, 0x404040, G_NO_SHADOW, card->text, 136, TF_NARROW);
    if (Ygo_IsMonster(card))
    {
        char atk[8], def[8];
        if (card->atk == YGO_STAT_UNKNOWN)
            snprintf(atk, sizeof(atk), "?");
        else
            snprintf(atk, sizeof(atk), "%u", card->atk);
        if (card->def == YGO_STAT_UNKNOWN)
            snprintf(def, sizeof(def), "?");
        else
            snprintf(def, sizeof(def), "%u", card->def);
        snprintf(line, sizeof(line), "ATK/%s  DEF/%s", atk, def);
        G_TextAlign(230, y + 1, TA_RIGHT, G_INK, G_INK_SHADOW, line, TF_SMALL);
        y += 13;
    }
    G_FillA(92, y, 140, 1, 0xd8d8d0, 255);
    y += 2;
    if (card->limit < YGO_COPIES)
    {
        static const char *const limits[] = {"FORBIDDEN", "LIMITED (1 a deck)", "SEMI-LIMITED (2 a deck)"};
        G_Text(94, y, 0xc03020, G_NO_SHADOW, limits[card->limit], 136, TF_NARROW);
        y += 10;
    }
    if (!Ygo_CardReady(card->id))
    {
        y = G_Text(94, y, 0xc07020, G_NO_SHADOW, "(This card can't be played yet.)", 136, TF_NARROW);
    }
    {
        int n = snprintf(line, sizeof(line), "%s", card->sets ? "" : "Not in a 2003 booster");
        unsigned s;
        for (s = 0; s < YGO_SET_COUNT; s++)
            if (card->sets >> s & 1)
                n += snprintf(line + n, sizeof(line) - n, "%s%s", n ? " " : "", gYgoSets[s].code);
        if (card->rarity >= YGO_RARE)
            snprintf(line + n, sizeof(line) - n, "  %s", RarityLabel(card->rarity));
        y = G_Text(94, y, 0x707878, G_NO_SHADOW, line, 136, TF_NARROW);
        snprintf(line, sizeof(line), "You own %u.", C()->owned[card->id]);
        y = G_Text(94, y, 0x707878, G_NO_SHADOW, line, 136, TF_NARROW);
    }
    Y.inspectMax = y + Y.inspectScroll - 136;
    if (Y.inspectMax < 0)
        Y.inspectMax = 0;
    SDL_RenderSetClipRect(G_R(), NULL);
    if (Y.inspectMax > 0)
    {
        if (Y.inspectScroll > 0)
            G_Arrow(226, 7, 0, 0x707070);
        if (Y.inspectScroll < Y.inspectMax)
            G_Arrow(226, 136, 1, 0x707070);
    }
    TcgUi_Hints(Y.inspectMax ? "{A} BIG PICTURE  {UPDOWN} SCROLL  {B} BACK" : "{A} BIG PICTURE  {B} BACK");
}
static void ArtScreen(void)
{
    int w, h, smooth;
    SDL_Texture *art = Ygo_ArtTexture((unsigned)Y.card, &w, &h, &smooth);
    G_Fill(0, 0, 240, 160, 0x101418);
    if (art)
    {
        float fit = smooth ? G_MinF(156.0f / h, 228.0f / w) : (float)(h * 2 <= 156 ? 2 : 1);
        SDL_FRect dst;
        if (Y.zoom)
            fit *= smooth ? 1.8f : 1.5f;
        dst.w = w * fit;
        dst.h = h * fit;
        dst.x = 120 - dst.w / 2 - Y.panX;
        dst.y = 80 - dst.h / 2 - Y.panY;
        SDL_RenderCopyF(G_R(), art, NULL, &dst);
    }
    TcgUi_Hints(Y.zoom ? "{A} FIT  {DPAD} MOVE  {B} BACK" : "{A} ZOOM  {DPAD} MOVE  {B} BACK");
}

/* A booster: the set's colors, its best monster, its code */
static void PackArt(int x, int y, unsigned set, int focus)
{
    static const unsigned colors[YGO_SET_COUNT][2] = {
        [YGO_SET_LOB] = {0x3058c8, 0x102060}, [YGO_SET_MRD] = {0x808898, 0x303440}, [YGO_SET_SRL] = {0x9040b8, 0x401860},
        [YGO_SET_PSV] = {0xd09028, 0x604010}, [YGO_SET_LON] = {0x30a070, 0x104830}, [YGO_SET_LOD] = {0xc83030, 0x581010},
        [YGO_SET_PGD] = {0xd8b040, 0x685010}, [YGO_SET_MFC] = {0x6050d0, 0x201860}, [YGO_SET_DCR] = {0x484050, 0x101018},
        [YGO_SET_SDY] = {0x8040a0, 0x301040}, [YGO_SET_SDK] = {0x4070d0, 0x183060}, [YGO_SET_SDJ] = {0xd04838, 0x601810},
        [YGO_SET_SDP] = {0xd0a040, 0x604818}};
    int w = 44, h = 58, i;
    G_FillA(x + 2, y + 3, w, h, 0x000000, 80);
    for (i = 0; i < h; i++)
        G_FillA(x, y + i, w, 1, G_Mix(colors[set][0], colors[set][1], i / (float)h), 255);
    for (i = 0; i < w; i += 2)
    {
        G_FillA(x + i, y, 1, 3, 0xd8d8e0, 255);
        G_FillA(x + i, y + h - 3, 1, 3, 0xd8d8e0, 255);
    }
    Ygo_DrawCard(SetMascot(set), (float)x + 7, (float)y + 5, 30, 42, 0, 255, 0);
    G_FillA(x, y + 47, w, 8, 0x101010, 150);
    G_TextAlign(x + w / 2, y + 45, TA_CENTER, G_WHITE, G_NO_SHADOW, gYgoSets[set].code, TF_NARROW);
    if (focus)
    {
        int s = (int)(Y.frame % 60) - 10;
        if (s >= 0 && s < w + 8)
            G_FillA(x + s - 8, y + 3, 3, h - 6, 0xffffff, 70);
    }
}
/* The boosters held, and every set that can be had, in release order */
static unsigned PackSets(uint8_t *out)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_SET_COUNT; i++)
        if (!gYgoSets[i].starter && (C()->packs[i] || Ygo_SetAvailable(i)))
            out[n++] = (uint8_t)i;
    return n;
}
static void PacksScreen(void)
{
    uint8_t sets[YGO_SET_COUNT];
    unsigned n = PackSets(sets), i;
    int first = Y.cursor < 2 ? 0 : Y.cursor - 2;
    char line[128];
    TcgUi_Background();
    TcgUi_TopBar("BOOSTER PACKS");
    if (first > (int)n - 4)
        first = (int)n > 4 ? (int)n - 4 : 0;
    for (i = 0; i < 4 && first + (int)i < (int)n; i++)
    {
        unsigned set = sets[first + (int)i];
        int x = 10 + (int)i * 57, on = first + (int)i == Y.cursor;
        int bob = on ? (int)(fabsf(sinf(Y.frame * 0.1f)) * 3) : 0;
        PackArt(x, 26 - bob, set, on);
        snprintf(line, sizeof(line), "x%u", C()->packs[set]);
        G_TextAlign(x + 22, 88, TA_CENTER, C()->packs[set] ? G_WHITE : 0xc0d8c8, 0x204030, line, TF_SMALL);
        if (on)
            G_Frame(x - 3, 26 - bob - 3, 50, 64, 0xe82820, 255);
    }
    if (n)
    {
        unsigned set = sets[Y.cursor < (int)n ? Y.cursor : 0], total, have = Ygo_SetOwned(C(), set, &total);
        snprintf(line, sizeof(line), "%s\n%s  (%u/%u)", gYgoSets[set].name,
                 C()->packs[set] ? "Open this pack!" : "Win duels to earn boosters.", have, total);
        Strip(0, line);
    }
    TcgUi_Hints("{LEFTRIGHT} CHOOSE  {A} OPEN  {B} BACK");
}
#define REVEAL_TEAR 34
static void RevealScreen(void)
{
    unsigned i;
    int t = Y.age;
    TcgUi_Background();
    if (t < REVEAL_TEAR)
    {
        int shake = t < 14 ? ((t / 2) % 2 ? 2 : -2) : 0;
        float tear = t < 14 ? 0 : 1 - (1 - (t - 14) / 12.0f) * (1 - (t - 14) / 12.0f);
        if (tear > 1)
            tear = 1;
        if (tear > 0)
            Ygo_DrawCard(0, 105, 52 - tear * 22, 30, 42, 0, 255, 0);
        PackArt(98 + shake, 50 + (int)(tear * 30), (unsigned)Y.pack, 0);
        TcgUi_Hints("{A} OPEN");
        return;
    }
    for (i = 0; i < YGO_PACK_SIZE; i++)
    {
        /* five on top, four below, the rare slot last */
        int x = i < 5 ? 12 + (int)i * 46 : 35 + (int)(i - 5) * 46, y = i < 5 ? 6 : 55;
        float in = (float)(t - REVEAL_TEAR - (int)i * 2) / 10.0f, sx, sy;
        int faceUp = (int)i < Y.flipped, on = (int)i == Y.cursor && Y.flipped >= YGO_PACK_SIZE;
        unsigned rarity = Y.revealRarity[i];
        if (in < 0)
            in = 0;
        if (in > 1)
            in = 1;
        sx = 105 + (x - 105) * in;
        sy = 40 + (y - 40) * in;
        if (faceUp && rarity >= YGO_RARE)
        {
            int glow = (int)(fabsf(sinf(Y.frame * 0.12f)) * 60);
            G_RoundFill((int)sx - 3, (int)sy - 3, 36, 48,
                        rarity == YGO_SECRET_RARE ? Rainbow(Y.frame * 4 + (int)i * 40)
                        : rarity == YGO_ULTRA_RARE ? 0xf8d050
                        : rarity == YGO_SUPER_RARE ? 0xc0e0ff
                                                  : 0xd8e0e8,
                        (unsigned)(70 + glow));
        }
        Ygo_DrawCard(faceUp ? Y.revealed[i] : 0, sx, sy - (on ? 3 : 0), 30, 42, 0, 255, 0);
        if (faceUp)
            Foil(rarity, (int)sx, (int)sy - (on ? 3 : 0), 30, 42, Y.frame + (int)i * 17);
        if (faceUp && rarity >= YGO_RARE)
        {
            /* (on the card's foot, as on the Pokemon side) */
            G_RoundFill((int)sx + 1, (int)sy + 33 - (on ? 3 : 0), 28, 9, 0x101828, 210);
            G_TextAlign((int)sx + 15, (int)sy + 32 - (on ? 3 : 0), TA_CENTER, RarityInk(rarity, Y.frame), 0x302040,
                        rarity == YGO_SECRET_RARE ? "SECRET!" : rarity == YGO_ULTRA_RARE ? "ULTRA!" : rarity == YGO_SUPER_RARE ? "SUPER!" : "RARE!",
                        TF_NARROW);
        }
        if (faceUp && (Y.revealNew >> i) & 1)
        {
            G_RoundFill((int)sx - 2, (int)sy - 3 - (on ? 3 : 0), 17, 8, 0xe82820, 230);
            G_Text((int)sx, (int)sy - 3 - (on ? 3 : 0), 0xffffff, 0x801010, "NEW", 17, TF_NARROW);
        }
        if (on)
        {
            int o = (Y.frame / 8) % 2;
            G_Frame((int)sx - 2 - o, (int)sy - 5 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (Y.flipped >= YGO_PACK_SIZE)
    {
        const struct YgoCard *c = Ygo_Card(Y.revealed[Y.cursor]);
        unsigned rarity = Y.revealRarity[Y.cursor];
        char line[96];
        G_FillA(0, STRIP_Y, 240, 160 - STRIP_Y, 0x5848c8, 255);
        G_FillA(0, STRIP_Y, 240, 1, 0x9888f8, 255);
        if (c)
        {
            G_Text(8, STRIP_Y + 4, RarityInk(rarity, Y.frame), 0x302878, c->name, 226, TF_NORMAL);
            snprintf(line, sizeof(line), "%s%s", rarity >= YGO_RARE ? RarityLabel(rarity) : "Common",
                     (Y.revealNew >> Y.cursor) & 1 ? "  NEW!" : "");
            G_Text(8, STRIP_Y + 20, 0xd8d8f8, 0x302878, line, 226, TF_SMALL);
        }
        TcgUi_Hints("{A} LOOK  {DPAD} CHOOSE  {B} DONE");
    }
    else
        TcgUi_Hints("{A} NEXT CARD  {B} SHOW ALL");
}
static void SpoilsScreen(void)
{
    int i, cols = 7, page = 14, start = Y.cursor / page * page;
    char line[96];
    TcgUi_Background();
    snprintf(line, sizeof(line), "%s'S DECK", Y.opp.name);
    TcgUi_TopBar(line);
    for (i = 0; i < page && start + i < Y.spoilCount; i++)
    {
        int x = 8 + (i % cols) * 33, y = 21 + (i / cols) * 45, on = start + i == Y.cursor;
        const struct YgoCard *c = Ygo_Card(Y.spoils[start + i]);
        if (on)
            y -= 2;
        if (c && c->rarity >= YGO_SUPER_RARE)
            G_RoundFill(x - 2, y - 2, 34, 46, 0xf8e070, (unsigned)(80 + fabsf(sinf(Y.frame * 0.1f)) * 60));
        Ygo_DrawCard(Y.spoils[start + i], (float)x, (float)y, 30, 42, 0, 255, 0);
        if (on)
        {
            int o = (Y.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    {
        const struct YgoCard *c = Ygo_Card(Y.spoils[Y.cursor]);
        snprintf(line, sizeof(line), "%s\nYou own %u.", c ? c->name : "", c ? C()->owned[c->id] : 0);
        Strip(0, line);
    }
    TcgUi_Hints("{A} TAKE  {INSPECT} LOOK");
}

/* The CARD CLUB duelist's counter: TRADE IN spares for CARD POINTS (shared with the Pokemon cards), the CARD SHOP */
static const char *const sCounterTabs[] = {"SINGLES", "DECKS", "TRADE IN", "CARD SHOP"};
/* The counter's tabs: the CARD CLUB's TRADE IN and CARD SHOP; the GAME CENTER clerk's SINGLES and DECKS first */
static int CounterTabs(void) { return Y.gc ? 4 : 2; }
static int CounterKind(void) { return Y.gc ? Y.counterTab : Y.counterTab + 2; }
static unsigned SpareList(uint16_t *out)
{
    unsigned id, n = 0;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
        if (Ygo_Spare(C(), id))
            out[n++] = (uint16_t)id;
    return n;
}
static unsigned ShopRows(uint8_t *sets)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_SET_COUNT; i++)
        if (Ygo_SetAvailable(i))
            sets[n++] = (uint8_t)i;
    return n;   /* + 1: A CARD YOU PICK */
}
static void CounterScreen(void)
{
    char line[128];
    int i;
    /* (the Pokemon counters' look: the tabs down the side panel, what you spend under them) */
    TcgUi_Background();
    TcgUi_SidePanel(Y.gc ? "GAME CENTER" : "CARD CLUB", YGO_THEME);
    if (CounterKind() < 2)
        snprintf(line, sizeof(line), "\xa5%u", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
    else
        snprintf(line, sizeof(line), "%u CP", Points());
    TcgUi_ShopTabs(sCounterTabs + (Y.gc ? 0 : 2), CounterTabs(), Y.counterTab, 0xb8a8e8, 0x483090, 0x302060, line,
                   CounterKind() < 2 ? NULL : "CARD POINTS", CounterKind() < 2 ? 0x483090 : 0x2858a8);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    if (CounterKind() == 0)
    {
        TcgUi_Row(LIST_X + 2, 8, LIST_W - 4, 1, 1, "SINGLES", "300-4000");
        Strip(0, "Any card that can be played, one copy each, for money. Rares too!");
    }
    else if (CounterKind() == 1)
    {
        for (i = 0; i < YGO_STARTERS; i++)
        {
            int sold = GcDeckSold((unsigned)i);
            char right[24];
            snprintf(right, sizeof(right), sold ? "SOLD OUT" : "\xa5%u", GC_DECK_PRICE);
            TcgUi_Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, i == Y.cursor,
                      !sold && IsEnoughMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE), SetName(sStarters[i]), right);
        }
        snprintf(line, sizeof(line), "%s\n%s", gYgoSets[sStarters[Y.cursor]].name,
                 GcDeckSold((unsigned)Y.cursor) ? "SOLD OUT: one of each deck per customer." : "Ready to duel in an empty deck slot.");
        Strip(0, line);
    }
    else if (CounterKind() == 2)
    {
        uint16_t list[YGO_CARD_COUNT];
        unsigned n = SpareList(list), k;
        int top = Y.cursor < 3 ? 0 : Y.cursor - 3;
        if (!n)
            G_Text(LIST_X + 8, 12, 0x707070, G_NO_SHADOW, "No spare cards yet.", LIST_W - 14, TF_SMALL);
        for (k = 0; k < 6 && top + (int)k <= (int)n; k++)
        {
            int row = top + (int)k;
            if (row == (int)n)
            {
                if (n)
                    TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, 1, "ALL SPARES", NULL);
                continue;
            }
            snprintf(line, sizeof(line), "x%u", Ygo_Spare(C(), list[row]));
            TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, 1, Ygo_Card(list[row])->name, line);
        }
        if (!n)
            Strip(0, "Copies past three (and past what your decks use) trade for CARD POINTS.");
        else if (Y.cursor >= (int)n)
            Strip(0, "Every spare card at once: copies past three and past what your decks use.");
        else
        {
            snprintf(line, sizeof(line), "%s: %u CP each, %u spare.\n{A} ONE  {SELECT} ALL SPARE",
                     Ygo_Card(list[Y.cursor])->name, Ygo_PointValue(list[Y.cursor]), Ygo_Spare(C(), list[Y.cursor]));
            Strip(list[Y.cursor], line);
        }
    }
    else
    {
        uint8_t sets[YGO_SET_COUNT];
        unsigned n = ShopRows(sets), k;
        int top = Y.cursor < 3 ? 0 : Y.cursor - 3;
        for (k = 0; k < 6 && top + (int)k <= (int)n; k++)
        {
            int row = top + (int)k;
            if (row == (int)n)
            {
                TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, 1, "A CARD YOU PICK", "5-150");
                continue;
            }
            snprintf(line, sizeof(line), "%u", YGO_PACK_COST);
            TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, Points() >= YGO_PACK_COST,
                      gYgoSets[sets[row]].name, line);
        }
        if (Y.cursor < (int)n)
        {
            snprintf(line, sizeof(line), "A %s booster: 9 cards, a Rare or better in each.", gYgoSets[sets[Y.cursor]].code);
            Strip(0, line);
        }
        else
            Strip(0, "Any card that can be played, 5-150 CP by rarity.");
    }
    TcgUi_Hints(CounterKind() < 2 ? "{A} CHOOSE  {L}{R} COUNTER  {B} LEAVE" : "{L}{R} COUNTER  {B} LEAVE");
}
static void StarterScreen(void)
{
    int i;
    char line[160];
    TcgUi_Background();
    TcgUi_TopBar("CHOOSE A STARTER DECK");
    for (i = 0; i < YGO_STARTERS; i++)
    {
        unsigned set = sStarters[i];
        uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
        int x = 10 + i * 57, on = i == Y.cursor, bob = on ? (int)(fabsf(sinf(Y.frame * 0.1f)) * 3) : 0;
        StarterDeckOf(set, main, extra);
        PackArt(x, 26 - bob, set, on);
        G_TextAlign(x + 22, 88, TA_CENTER, G_WHITE, 0x204030, SetName(set), TF_NARROW);
        if (on)
            G_Frame(x - 3, 26 - bob - 3, 50, 64, 0xe82820, 255);
    }
    {
        static const char *const blurbs[YGO_STARTERS] = {
            "Dark Magician and friends: Spellcasters and Warriors.",
            "Blue-Eyes White Dragon: big monsters, and ways to Summon them.",
            "Red-Eyes B. Dragon: luck, Warriors and the Flame Swordsman.",
            "Toons and tricks: Pegasus's deck of illusions."};
        uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
        StarterDeckOf(sStarters[Y.cursor], main, extra);
        snprintf(line, sizeof(line), "%u cards. %s", Ygo_DeckCount(main, YGO_DECK_MAX) + Ygo_DeckCount(extra, YGO_FUSION_MAX),
                 blurbs[Y.cursor]);
        Strip(0, line);
    }
    TcgUi_Hints("{LEFTRIGHT} CHOOSE  {A} TAKE  {INSPECT} LOOK");
}

/* ------------------------------------------------------------------ */
/* With a friend                                                        */
/* ------------------------------------------------------------------ */
static void FilterTab(unsigned pressed);
static void OnlineStep(void)
{
    struct YgoNet *l = Ygo_Online();
    const struct TcgSocial *s = Tcg_Social();
    if (Y.vs != VS_FRIEND || Y.screen == Y_TRADE || Y.screen == Y_CLOSED)
        return;
    if (s->peerLeft && Y.screen != Y_DUEL && !Y.message[0])
    {
        Say(ACT_CLOSE, "%s left. Nothing was lost.", Y.opp.name);
        return;
    }
    /* The friend who said yes accepts the asker's invite with the deck they picked */
    if (s->seat == 1 && Y.onlineDeckChosen && l->state == YGO_NET_INVITED)
        YgoNet_Accept(l, Y.onlineDeck < 0 ? Y.loanMain : Deck((unsigned)Y.onlineDeck)->main,
                      Y.onlineDeck < 0 ? Y.loanExtra : Deck((unsigned)Y.onlineDeck)->extra, SDL_GetTicks());
    if (Y.screen == Y_WAIT && l->state == YGO_NET_PLAYING)
    {
        Y.online = 1;
        ShowOnline();
        Y.rewarded = 0;
        Y.concedeHeld = 0;
        Y.aiDelay = 30;
        YgoBoard_SetFast(Tcg_GetCollection()->fastAnimations);
        YgoBoard_SetMusic(Y.opp.music);
        YgoBoard_Tutorial(0);
        Screen(Y_DUEL);
        YgoBoard_Begin(&Y.duel, l->seat, Y.opp.name, 1);
        printf("[ygo] duel started vs %s (online, seat %u)\n", Y.opp.name, l->seat);
    }
    if (Y.online && l->state == YGO_NET_ENDED && !l->game.s.over && !Y.message[0])
    {
        Y.online = 0;
        if (Y.screen == Y_DUEL || Y.back == Y_DUEL)
            YgoBoard_End();
        Y.screen = Y_WAIT;
        Say(ACT_CLOSE, "%s", l->error[0] ? l->error : "The duel ended.");
        printf("[ygo] online duel ended: %s\n", l->error);
    }
}
static void WaitScreen(void)
{
    char line[96];
    int dots = (Y.frame / 20) % 4;
    TcgUi_Background();
    TcgUi_SidePanel("DUEL", YGO_THEME);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    snprintf(line, sizeof(line), "Waiting for %s%.*s", Y.opp.name, dots, "...");
    G_Text(LIST_X + 8, 20, G_INK, G_INK_SHADOW, line, LIST_W - 14, TF_SMALL);
    Ygo_DrawCard(0, (float)(LIST_X + LIST_W / 2 - 15), 44.0f, 30, 42, (Y.frame % 120) * 3.0, 255, 0);
    Strip(0, "They're choosing their deck.");
    TcgUi_Hints("{B} LEAVE");
}
/* Trading: cards past what the deck in use needs */
static int Tradable(unsigned id)
{
    const struct YgoCard *card = Ygo_Card(id);
    return card && card->frame != YGO_TOKEN && Filter(id, Y.filter) && C()->owned[id] > InDeck(C()->activeDeck, id);
}
static unsigned TradeCount(void)
{
    unsigned id, n = 0;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
        n += Tradable(id);
    return n;
}
static unsigned TradeCard(int index)
{
    unsigned id;
    for (id = 1; id <= YGO_CARD_COUNT; id++)
        if (Tradable(id) && !index--)
            return id;
    return 0;
}
static void TradeScreen(void)
{
    const struct TcgSocial *s = Tcg_Social();
    unsigned mine = s->myOffer, theirs = s->theirOffer <= YGO_CARD_COUNT ? s->theirOffer : 0;
    int n = (int)TradeCount(), i, page = 7, start;
    char line[160];
    TcgUi_Background();
    snprintf(line, sizeof(line), "YU-GI-OH! TRADE WITH %s", Y.opp.name);
    TcgUi_TopBar(line);
    TcgUi_ListBox(4, 20, 112, 62);
    TcgUi_ListBox(124, 20, 112, 62);
    G_Text(10, 22, 0x606878, G_NO_SHADOW, "YOU OFFER", 0, TF_NARROW);
    G_Text(130, 22, 0x606878, G_NO_SHADOW, "THEY OFFER", 0, TF_NARROW);
    if (mine)
    {
        Ygo_DrawCard(mine, 10, 33, 30, 42, 0, 255, 0);
        G_Text(44, 38, G_INK, G_INK_SHADOW, Ygo_Card(mine)->name, 70, TF_SMALL);
        if (s->myConfirm)
            G_Text(44, 62, 0x2858a8, G_NO_SHADOW, "READY!", 0, TF_SMALL);
    }
    if (theirs)
    {
        Ygo_DrawCard(theirs, 130, 33, 30, 42, 0, 255, 0);
        G_Text(164, 38, G_INK, G_INK_SHADOW, Ygo_Card(theirs)->name, 70, TF_SMALL);
        if (s->theirConfirm)
            G_Text(164, 62, 0x2858a8, G_NO_SHADOW, "READY!", 0, TF_SMALL);
    }
    else
        G_Text(164, 46, 0x909090, G_NO_SHADOW, "Choosing...", 0, TF_SMALL);
    G_RoundFill(0, 84, 240, 15, 0x203040, 230);
    Tabs(4, 85, Y.filter);
    start = Y.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        unsigned card = TradeCard(start + i);
        int x = 8 + i * 33, on = start + i == Y.cursor;
        Ygo_DrawCard(card, (float)x, (float)(103 - (on ? 2 : 0)), 30, 42, 0, 255, 0);
        if (on)
            G_Frame(x - 2, 99, 34, 46, 0xe82820, 255);
    }
    if (!n)
        G_Text(10, 108, G_WHITE, 0x204030, "No cards of this kind to offer.", 0, TF_SMALL);
    TcgUi_Hints(s->myOffer && s->theirOffer ? (s->myConfirm ? "WAITING FOR THEM  {B} LEAVE" : "{A} OFFER  {START} TRADE!  {B} LEAVE")
                                            : "{A} OFFER  {L}{R} TYPE  {B} LEAVE");
}
static void TradeStep(unsigned pressed)
{
    const struct TcgSocial *s = Tcg_Social();
    struct YgoCollection *c = C();
    int n = (int)TradeCount();
    /* Both confirmed: the trade is done, even when the friend's leaving came in with their confirm */
    if (s->myConfirm && s->theirConfirm && s->myOffer && s->theirOffer)
    {
        unsigned mine = s->myOffer, theirs = s->theirOffer;
        if (theirs > YGO_CARD_COUNT || Ygo_Card(theirs)->frame == YGO_TOKEN || !c->owned[mine])
        {
            Tcg_SocialLeave();
            Say(ACT_CLOSE, "That trade couldn't be made.");
            return;
        }
        c->owned[mine]--;
        AddCard(theirs);
        c->trades++;
        MP_Achieve_Unlock("cardtrade");
        printf("[ygo] traded card %u for %u\n", mine, theirs);
        Tcg_SocialLeave();
        PlayFanfare(MUS_OBTAIN_ITEM);
        Say(ACT_CLOSE, "You traded your %s for %s!", Ygo_Card(mine)->name, Ygo_Card(theirs)->name);
        return;
    }
    if (s->peerLeft)
    {
        Say(ACT_CLOSE, "%s left the trade.", Y.opp.name);
        Tcg_SocialLeave();
        return;
    }
    FilterTab(pressed);
    {
        int old = Y.cursor;
        if (pressed & KEY_LEFT)
            Y.cursor--;
        if (pressed & KEY_RIGHT)
            Y.cursor++;
        if (Y.cursor < 0)
            Y.cursor = 0;
        if (Y.cursor >= n)
            Y.cursor = n ? n - 1 : 0;
        if (old != Y.cursor)
            PlaySE(SE_SELECT);
    }
    if (pressed & KEY_A && n)
    {
        Tcg_SocialOffer(TradeCard(Y.cursor));
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

/* ------------------------------------------------------------------ */
/* Input                                                                */
/* ------------------------------------------------------------------ */
static void MoveCursor(unsigned pressed, int count, int step)
{
    if (count <= 0)
        return;
    if (pressed & (step == 1 ? KEY_UP : KEY_LEFT))
        Y.cursor = (Y.cursor + count - 1) % count, PlaySE(SE_SELECT);
    if (pressed & (step == 1 ? KEY_DOWN : KEY_RIGHT))
        Y.cursor = (Y.cursor + 1) % count, PlaySE(SE_SELECT);
}
static void GridMove(unsigned pressed, int count, int cols)
{
    if (count <= 0)
        return;
    if (pressed & KEY_LEFT && Y.cursor > 0)
        Y.cursor--, PlaySE(SE_SELECT);
    if (pressed & KEY_RIGHT && Y.cursor < count - 1)
        Y.cursor++, PlaySE(SE_SELECT);
    if (pressed & KEY_UP && Y.cursor >= cols)
        Y.cursor -= cols, PlaySE(SE_SELECT);
    if (pressed & KEY_DOWN && Y.cursor + cols < count)
        Y.cursor += cols, PlaySE(SE_SELECT);
    if (Y.cursor >= count)
        Y.cursor = count - 1;
}
static void FilterTab(unsigned pressed)
{
    if (pressed & KEY_L)
        Y.filter = (Y.filter + FILTERS - 1) % FILTERS, Y.cursor = 0, PlaySE(SE_SELECT);
    if (pressed & KEY_R)
        Y.filter = (Y.filter + 1) % FILTERS, Y.cursor = 0, PlaySE(SE_SELECT);
}
static void Practice(void)
{
    struct YgoCollection *c = C();
    char reason[160];
    if (!Ygo_DeckValidate(Deck(c->activeDeck)->main, Deck(c->activeDeck)->extra, c->owned, reason, sizeof(reason)))
    {
        Say(ACT_NONE, "%s", reason);
        return;
    }
    memset(&Y.opp, 0, sizeof(Y.opp));
    snprintf(Y.opp.name, sizeof(Y.opp.name), "PRACTICE DECK");
    /* a Starter Deck that isn't yours */
    {
        unsigned pick = (unsigned)Tcg_RandomBelow(&c->rng, YGO_STARTERS);
        if (sStarters[pick] + 1 == c->starter)
            pick = (pick + 1) % YGO_STARTERS;
        StarterDeckOf(sStarters[pick], Y.opp.main, Y.opp.extra);
    }
    Y.opp.special = YGO_SPECIAL_NONE;
    Y.vs = VS_PRACTICE;
    StartDuel(Deck(c->activeDeck)->main, Deck(c->activeDeck)->extra, Y.opp.main, Y.opp.extra);
}
void YgoUi_OpenLesson(void)
{
    uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    Y.returnToGame = NULL;
    memset(&Y.opp, 0, sizeof(Y.opp));
    snprintf(Y.opp.name, sizeof(Y.opp.name), "CARD CLUB DUELIST");
    Y.opp.special = YGO_SPECIAL_NONE;
    Y.opp.lesson = 1;
    StarterDeckOf(YGO_SET_SDK, Y.opp.main, Y.opp.extra);
    StarterDeckOf(YGO_SET_SDY, main, extra);
    Y.vs = VS_LESSON;
    Open(Y_DUEL);
    Y.fromScript = 1;
    StartDuel(main, extra, Y.opp.main, Y.opp.extra);
}
static void ChooseDeck(void)
{
    struct YgoCollection *c = C();
    char reason[160];
    if (Y.cursor == YGO_SAVED_DECKS)
    {
        Close();
        return;
    }
    if (!Ygo_DeckValidate(Deck((unsigned)Y.cursor)->main, Deck((unsigned)Y.cursor)->extra, c->owned, reason,
                          sizeof(reason)))
    {
        Say(ACT_NONE, "%s", reason);
        return;
    }
    c->activeDeck = (uint8_t)Y.cursor;
    if (Y.vs == VS_FRIEND)
    {
        const struct TcgSocial *s = Tcg_Social();
        Y.onlineDeck = Y.cursor;
        Y.onlineDeckChosen = 1;
        if (s->seat == 0 &&
            !YgoNet_Invite(Ygo_Online(), s->peer, s->session, Deck(c->activeDeck)->main, Deck(c->activeDeck)->extra,
                           SDL_GetTicks()))
        {
            Say(ACT_CLOSE, "The duel couldn't start.");
            return;
        }
        Y.online = 1;
        Screen(Y_WAIT);
        return;
    }
    StartDuel(Deck(c->activeDeck)->main, Deck(c->activeDeck)->extra, Y.opp.main, Y.opp.extra);
}
/* The pack tears open on its own, then the cards turn over one by one (A: the next one now), rares slower */
static void RevealStep(unsigned pressed)
{
    static int sNext;
    unsigned rarity;
    if (Y.age < REVEAL_TEAR)
    {
        if (Y.age == 14)
            PlaySE(SE_RG_CARD_FLIPPING);
        if (pressed & KEY_A)
            Y.age = REVEAL_TEAR;
        return;
    }
    if (Y.age == REVEAL_TEAR)
        sNext = Y.age + 8;
    if (pressed & KEY_A)
        sNext = Y.age;
    if (Y.age >= sNext)
    {
        rarity = Y.revealRarity[Y.flipped];
        PlaySE(rarity >= YGO_SUPER_RARE ? SE_SHINY : rarity >= YGO_RARE ? SE_SHINY : SE_RG_CARD_FLIP);
        Y.flipped++;
        Y.cursor = Y.flipped - 1;
        sNext = Y.age + (Tcg_GetCollection()->fastAnimations ? 5 : rarity >= YGO_SUPER_RARE ? 40 : rarity >= YGO_RARE ? 26 : 11);
    }
}
static void OpenPackNow(unsigned set)
{
    struct YgoCollection *c = C();
    unsigned i;
    if (!c->packs[set])
        return;
    if (!Ygo_OpenPack(c, set, Y.revealed))
        return;
    Y.revealNew = 0;
    for (i = 0; i < YGO_PACK_SIZE; i++)
    {
        unsigned j, copies = 0, earlier = 0;
        for (j = 0; j < YGO_PACK_SIZE; j++)
            copies += Y.revealed[j] == Y.revealed[i], earlier += j < i && Y.revealed[j] == Y.revealed[i];
        /* new: none owned before this pack (its first copy in it) */
        if (c->owned[Y.revealed[i]] == copies && !earlier)
            Y.revealNew |= 1u << i;
        Y.revealRarity[i] = (uint8_t)RarityIn(set, Y.revealed[i]);
    }
    Y.pack = (int)set;
    Y.flipped = 0;
    Screen(Y_REVEAL);
    PlaySE(SE_RG_CARD_OPEN);
    MP_Achieve_Count("ygopack", 1);
    if (Y.revealRarity[YGO_PACK_SIZE - 1] == YGO_SECRET_RARE)
        MP_Achieve_Unlock("ygosecret");
    printf("[ygo] opened %s: rare slot %s (%s)\n", gYgoSets[set].code, Ygo_Card(Y.revealed[YGO_PACK_SIZE - 1])->name,
           RarityLabel(Y.revealRarity[YGO_PACK_SIZE - 1]));
}
static void Act(int action)
{
    struct YgoCollection *c = C();
    switch (action)
    {
    case ACT_CLOSE:
        Close();
        break;
    case ACT_HOME:
        Screen(Y_HOME);
        break;
    case ACT_TAKE_SPOIL:
    {
        unsigned id = Y.spoils[Y.cursor];
        AddCard(id);
        Ygo_DailyTake(c, Y.opp.key, Tcg_Today());
        Say(ACT_CLOSE, "%s went into your CARD CASE!", Ygo_Card(id)->name);
        break;
    }
    case ACT_STARTER:
    {
        unsigned set = sStarters[Y.askArg];
        if (Ygo_ChooseStarter(c, set))
        {
            MP_Achieve_Unlock("ygostart");
            if (Y.resolve)
            {
                /* back to the duel (the new deck is the one in use) or the trade */
                int back = Y.resolveBack;
                Y.resolve = RESOLVE_NONE;
                Screen(back);
                Y.cursor = c->activeDeck;
                Say(ACT_NONE, "You got the %s! It's yours to keep, on the CARD CASE's YU-GI-OH! side.",
                    gYgoSets[set].name);
                printf("[ygo] no deck: took the %s\n", gYgoSets[set].code);
            }
            else
                Say(ACT_CLOSE, "You got the %s! It's in your CARD CASE, on its YU-GI-OH! side.", gYgoSets[set].name);
        }
        break;
    }
    case ACT_LOAN:
    {
        unsigned set = sStarters[Y.askArg];
        Y.resolve = RESOLVE_NONE;
        StarterDeckOf(set, Y.loanMain, Y.loanExtra);
        Screen(Y.resolveBack);
        printf("[ygo] no deck: borrowed the %s\n", gYgoSets[set].code);
        if (Y.vs == VS_FRIEND)
        {
            const struct TcgSocial *s = Tcg_Social();
            Y.onlineDeck = -1;
            Y.onlineDeckChosen = 1;
            if (s->seat == 0 && !YgoNet_Invite(Ygo_Online(), s->peer, s->session, Y.loanMain, Y.loanExtra, SDL_GetTicks()))
            {
                Say(ACT_CLOSE, "The duel couldn't start.");
                break;
            }
            Y.online = 1;
            Screen(Y_WAIT);
        }
        else
            StartDuel(Y.loanMain, Y.loanExtra, Y.opp.main, Y.opp.extra);
        break;
    }
    case ACT_COPY:
    {
        unsigned d;
        for (d = 0; d < YGO_SAVED_DECKS; d++)
            if (!Deck(d)->name[0] && !MainCount(d) && !ExtraCount(d) && d != (unsigned)Y.askArg)
                break;
        if (d == YGO_SAVED_DECKS)
            Say(ACT_NONE, "There's no empty slot to copy into.");
        else
        {
            *Deck(d) = *Deck((unsigned)Y.askArg);
            c->ace[d] = c->ace[Y.askArg];
            snprintf(Deck(d)->name, YGO_NAME_LEN, "%.18s copy", Deck((unsigned)Y.askArg)->name);
            Say(ACT_NONE, "Copied into slot %u.", d + 1);
        }
        break;
    }
    case ACT_CLEAR:
        memset(Deck((unsigned)Y.askArg), 0, sizeof(struct YgoSavedDeck));
        c->ace[Y.askArg] = 0;
        if (c->activeDeck == Y.askArg)
        {
            unsigned d;
            for (d = 0; d < YGO_SAVED_DECKS; d++)
                if (DeckValid(d))
                {
                    c->activeDeck = (uint8_t)d;
                    break;
                }
        }
        break;
    case ACT_TRADE_IN:
    case ACT_TRADE_IN_CARD:
    {
        unsigned id = (unsigned)Y.askArg, spare = Ygo_Spare(c, id), n = action == ACT_TRADE_IN ? 1 : spare;
        if (!spare || n > spare)
            break;
        c->owned[id] -= n;
        AddPoints(n * Ygo_PointValue(id));
        PlaySE(SE_SHOP);
        Say(ACT_NONE, "Traded in %u x %s for %u CARD POINTS.", n, Ygo_Card(id)->name, n * Ygo_PointValue(id));
        break;
    }
    case ACT_TRADE_IN_ALL:
    {
        uint16_t list[YGO_CARD_COUNT];
        unsigned n = SpareList(list), i, points = 0, count = 0;
        for (i = 0; i < n; i++)
        {
            unsigned k = Ygo_Spare(c, list[i]);
            c->owned[list[i]] -= k;
            points += k * Ygo_PointValue(list[i]);
            count += k;
        }
        AddPoints(points);
        Y.cursor = 0;
        PlaySE(SE_SHOP);
        Say(ACT_NONE, "Traded in %u spare cards for %u CARD POINTS.", count, points);
        break;
    }
    case ACT_BUY_PACK:
        if (Points() >= YGO_PACK_COST && Ygo_AwardPack(c, (unsigned)Y.askArg))
        {
            SpendPoints(YGO_PACK_COST);
            PlaySE(SE_RG_SHOP);
            Say(ACT_NONE, "A %s booster went into your CARD CASE.", gYgoSets[Y.askArg].code);
        }
        break;
    case ACT_GC_CARD:
    {
        unsigned id = (unsigned)Y.askArg, price = Ygo_MoneyPrice(id);
        if (price && !GcSingleSold(id) && IsEnoughMoney(&gSaveBlock1Ptr->money, price))
        {
            RemoveMoney(&gSaveBlock1Ptr->money, price);
            c->gcSingles[id / 8] |= 1 << (id % 8);
            AddCard(id);
            PlaySE(SE_RG_SHOP);
            Say(ACT_NONE, "%s went into your CARD CASE.", Ygo_Card(id)->name);
            printf("[ygo] game center: bought card %u for %u\n", id, price);
        }
        break;
    }
    case ACT_RECIPE:
    {
        uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
        unsigned slot = Y.recipeSlot >= 0 ? (unsigned)Y.recipeSlot : EmptySlot(), lost = 0;
        if (Y.askArg < 0 || Y.askArg >= RECIPE_COUNT || slot >= YGO_SAVED_DECKS)
            break;
        RecipeCards(Y.askArg, main, extra);
        Ygo_DeckFromRecipe(c, slot, main, extra, SetName(sStarters[Y.askArg]), &lost);
        printf("[ygo] recipe %d into slot %u, %u missing\n", Y.askArg, slot, lost);
        Screen(Y_DECKS);
        Y.cursor = (int)slot;
        if (lost)
            Say(ACT_NONE, "%s is in slot %u. %u of its cards are missing: add them when you have them.",
                gYgoSets[sStarters[Y.askArg]].name, slot + 1, lost);
        else
            Say(ACT_NONE, "%s is in slot %u, ready to duel.", gYgoSets[sStarters[Y.askArg]].name, slot + 1);
        break;
    }
    case ACT_GC_DECK:
    {
        unsigned k = (unsigned)Y.askArg, i, slot;
        uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
        if (k >= YGO_STARTERS || GcDeckSold(k) || !IsEnoughMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE))
            break;
        RemoveMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE);
        c->gcDecks |= 1 << k;
        StarterDeckOf(sStarters[k], main, extra);
        for (i = 0; i < YGO_DECK_MAX && main[i]; i++)
            AddCard(main[i]);
        for (i = 0; i < YGO_FUSION_MAX && extra[i]; i++)
            AddCard(extra[i]);
        for (slot = 0; slot < YGO_SAVED_DECKS; slot++)
            if (!MainCount(slot) && !ExtraCount(slot))
                break;
        PlaySE(SE_RG_SHOP);
        PlayFanfare(MUS_OBTAIN_ITEM);
        printf("[ygo] game center: bought deck %u, slot %u\n", k, slot);
        if (slot < YGO_SAVED_DECKS)
        {
            memcpy(Deck(slot)->main, main, sizeof(main));
            memcpy(Deck(slot)->extra, extra, sizeof(extra));
            snprintf(Deck(slot)->name, YGO_NAME_LEN, "%s", SetName(sStarters[k]));
            c->ace[slot] = 0;
            Say(ACT_NONE, "You got the %s! It's in DECKS, slot %u, ready to duel.", gYgoSets[sStarters[k]].name, slot + 1);
        }
        else
            Say(ACT_NONE, "You got the %s! Its cards are in your CARD CASE (every deck slot is in use).",
                gYgoSets[sStarters[k]].name);
        break;
    }
    case ACT_BUY_CARD:
    {
        unsigned id = (unsigned)Y.askArg, cost = Ygo_PointCost(id);
        if (cost && Points() >= cost)
        {
            SpendPoints(cost);
            AddCard(id);
            PlaySE(SE_RG_SHOP);
            Say(ACT_NONE, "%s went into your CARD CASE.", Ygo_Card(id)->name);
        }
        break;
    }
    default:
        break;
    }
}

int YgoUi_Step(unsigned keys)
{
    struct YgoCollection *c = C();
    unsigned pressed = keys & ~Y.previousKeys;
    Y.previousKeys = keys;
    if (!YgoUi_Active())
        return 0;
    Y.age++;
    Y.frame++;
    if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
    {
        if (++Y.held > 18 && Y.held % 4 == 0 && Y.screen != Y_DUEL)
            pressed |= keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
    }
    else
        Y.held = 0;
    OnlineStep();
    if (Y.screen == Y_DUEL && !Y.message[0])
    {
        DuelStep(pressed, keys);
        return 1;
    }
    if (Y.message[0])
    {
        if (Y.asking)
        {
            if (pressed & (KEY_UP | KEY_DOWN))
                Y.askYes ^= 1, PlaySE(SE_SELECT);
            if (pressed & (KEY_A | KEY_B))
            {
                int yes = (pressed & KEY_A) && Y.askYes;
                Y.message[0] = 0;
                Y.asking = 0;
                PlaySE(SE_SELECT);
                if (yes)
                    Act(Y.askAction);
            }
        }
        else if (pressed & (KEY_A | KEY_B))
        {
            int then = Y.messageThen;
            Y.message[0] = 0;
            PlaySE(SE_SELECT);
            Act(then);
        }
        return 1;
    }
    switch (Y.screen)
    {
    case Y_HOME:
        MoveCursor(pressed, HOME_ROWS, 1);
        if (pressed & (KEY_L | KEY_R))
        {
            PlaySE(SE_SELECT);
            ToSide(pressed & KEY_R ? KIT_GAME_TRIAD : KIT_GAME_POKEMON);
        }
        else if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            if (!c->optedIn && Y.cursor < 4)
            {
                Say(ACT_NONE, "Get a Starter Deck from the CARD CLUB's duelist in a POKeMON MART first!");
                break;
            }
            switch (Y.cursor)
            {
            case 0:
                Screen(Y_DECKS);
                Y.cursor = c->activeDeck;
                break;
            case 1:
                Y.filter = 0;
                Y.showAll = 0;
                Screen(Y_BINDER);
                break;
            case 2:
                Screen(Y_PACKS);
                {
                    uint8_t sets[YGO_SET_COUNT];
                    unsigned n = PackSets(sets), i;
                    for (i = 0; i < n; i++)
                        if (c->packs[sets[i]])
                        {
                            Y.cursor = (int)i;
                            break;
                        }
                }
                break;
            case 3:
                Practice();
                break;
            case 4:
                Tcg_GetCollection()->fastAnimations = !Tcg_GetCollection()->fastAnimations;
                break;
            default:
                Close();
                break;
            }
        }
        break;
    case Y_CHALLENGE:
        MoveCursor(pressed, YGO_SAVED_DECKS + 1, 1);
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
    case Y_DECKS:
        if (TcgUi_NameActive())
        {
            if (TcgUi_NameStep(pressed) == 1 && Y.cursor < YGO_SAVED_DECKS)
            {
                snprintf(Deck((unsigned)Y.cursor)->name, YGO_NAME_LEN, "%s", TcgUi_NameText());
                printf("[ygo] deck %d renamed %s\n", Y.cursor, Deck((unsigned)Y.cursor)->name);
            }
            break;
        }
        if (Y.menuOn)
        {
            int empty = SlotEmpty((unsigned)Y.cursor), count = empty ? EmptyMenuCount() : DeckMenuCount();
            unsigned off = empty ? 0 : DeckMenuOff((unsigned)Y.cursor);
            Y.menuCursor = TcgUi_PopMenuMove(pressed, Y.menuCursor, count, off);
            if (pressed & KEY_B)
            {
                PlaySE(SE_SELECT);
                Y.menuOn = 0;
            }
            else if (pressed & KEY_A && !((off >> Y.menuCursor) & 1))
            {
                int slot = Y.cursor;
                PlaySE(SE_SELECT);
                Y.menuOn = 0;
                if (empty && Y.menuCursor == EM_RECIPE)
                {
                    Screen(Y_RECIPES);
                    Y.recipeSlot = slot;
                }
                else if (empty && Y.menuCursor == EM_CODE)
                    OpenCode(slot);
                else if (empty && Y.menuCursor == EM_FILE)
                    ImportYdk(slot);
                else if (!empty && Y.menuCursor == DM_CODE)
                {
                    char code[2048];
                    if (Ygo_WriteYdke(Deck((unsigned)slot)->main, Deck((unsigned)slot)->extra, code, sizeof(code)))
                    {
                        Platform_SetClipboardText(code);
                        printf("[ygo] deck code copied: %s\n", code);
                        Say(ACT_NONE, "%s's deck code was copied: paste it anywhere (a YDKE code).",
                            Deck((unsigned)slot)->name);
                    }
                }
                else if (!empty && Y.menuCursor == DM_EXPORT)
                    ExportYdk(slot);
                else if (empty || Y.menuCursor == DM_EDIT)
                {
                    Y.deck = slot;
                    if (!Deck((unsigned)Y.deck)->name[0])
                        snprintf(Deck((unsigned)Y.deck)->name, YGO_NAME_LEN, "Deck %d", Y.deck + 1);
                    Y.side = 0;
                    Y.deckCursor = 0;
                    Y.filter = 0;
                    Screen(Y_BUILDER);
                }
                else if (Y.menuCursor == DM_RENAME)
                    TcgUi_NameOpen("RENAME DECK", Deck((unsigned)slot)->name);
                else if (Y.menuCursor == DM_ACE)
                {
                    uint16_t list[YGO_DECK_MAX + YGO_FUSION_MAX];
                    unsigned n = AceList((unsigned)slot, list), ace = Ygo_DeckAce(c, (unsigned)slot), k;
                    Y.deck = slot;
                    Screen(Y_ACE);
                    for (k = 0; k < n; k++)
                        if (list[k] == ace)
                            Y.cursor = (int)k;
                }
                else if (Y.menuCursor == DM_USE)
                {
                    c->activeDeck = (uint8_t)slot;
                    PlaySE(SE_SUCCESS);
                }
                else if (Y.menuCursor == DM_COPY)
                    Ask(ACT_COPY, slot, "Copy %s into an empty slot?", Deck((unsigned)slot)->name);
                else if (Y.menuCursor == DM_DELETE)
                    Ask(ACT_CLEAR, slot, "Delete %s? Its cards stay in your collection.", Deck((unsigned)slot)->name);
            }
            break;
        }
        MoveCursor(pressed, DECK_ROWS, 1);
        if (Y.cursor == DECK_ROW_LIST && pressed & (KEY_LEFT | KEY_RIGHT | KEY_A))
        {
            /* the next (or the last) Forbidden & Limited list */
            unsigned n = Ygo_BanlistCount(), now = Ygo_Banlist();
            now = (pressed & KEY_LEFT ? now + n - 1 : now + 1) % n;
            snprintf(c->banlist, sizeof(c->banlist), "%s", now ? Ygo_BanlistName(now) : "");
            Ygo_UseBanlist(c->banlist);
            printf("[ygo] list: %s\n", Ygo_BanlistName(Ygo_Banlist()));
            PlaySE(SE_SELECT);
        }
        else if (pressed & KEY_B || (pressed & KEY_A && Y.cursor == DECK_ROW_BACK))
        {
            PlaySE(SE_SELECT);
            Screen(Y_HOME);
        }
        else if (pressed & KEY_A && Y.cursor == DECK_ROW_RECIPES)
        {
            PlaySE(SE_SELECT);
            Screen(Y_RECIPES);
            Y.recipeSlot = -1;
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            Y.menuOn = 1;
            Y.menuCursor = 0;
        }
        else if (pressed & KEY_R && Y.cursor < YGO_SAVED_DECKS && (unsigned)Y.cursor != c->activeDeck &&
                 MainCount((unsigned)Y.cursor))
        {
            char reason[160];
            if (Ygo_DeckValidate(Deck((unsigned)Y.cursor)->main, Deck((unsigned)Y.cursor)->extra, c->owned, reason,
                                 sizeof(reason)))
            {
                c->activeDeck = (uint8_t)Y.cursor;
                PlaySE(SE_SUCCESS);
            }
            else
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "%s", reason);
            }
        }
        else if (pressed & KEY_START && Y.cursor < YGO_SAVED_DECKS && MainCount((unsigned)Y.cursor))
            Ask(ACT_COPY, Y.cursor, "Copy %s into an empty slot?", Deck((unsigned)Y.cursor)->name);
        else if (pressed & KEY_SELECT && Y.cursor < YGO_SAVED_DECKS && Deck((unsigned)Y.cursor)->name[0])
            Ask(ACT_CLEAR, Y.cursor, "Delete %s? Its cards stay in your collection.", Deck((unsigned)Y.cursor)->name);
        break;
    case Y_CODE:
    {
        int ch;
        TakeClipboardCode();
        while ((ch = Platform_PopTypedChar()) != 0)
        {
            size_t n = strlen(Y.code);
            if (ch == TYPED_ENTER)
                pressed |= KEY_A;
            else if (ch == TYPED_BACKSPACE)
            {
                if (n)
                    Y.code[n - 1] = 0;
            }
            else if (ch > ' ' && ch < 127 && n + 1 < sizeof(Y.code))
                Y.code[n] = (char)ch, Y.code[n + 1] = 0;
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            CloseCode();
        }
        else if (pressed & KEY_SELECT)
        {
            Y.code[0] = 0;
            PlaySE(SE_SELECT);
        }
        else if (pressed & (KEY_A | KEY_START))
            UseCode();
        break;
    }
    case Y_RECIPES:
        MoveCursor(pressed, RECIPE_COUNT, 1);
        if (pressed & KEY_B)
        {
            int back = Y.recipeSlot >= 0 ? Y.recipeSlot : DECK_ROW_RECIPES;
            PlaySE(SE_SELECT);
            Screen(Y_DECKS);
            Y.cursor = back;
        }
        else if (pressed & KEY_A)
        {
            uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
            unsigned slot = Y.recipeSlot >= 0 ? (unsigned)Y.recipeSlot : EmptySlot(), total, lost;
            RecipeCards(Y.cursor, main, extra);
            lost = Ygo_RecipeOwned(c, main, extra, &total);
            lost = total - lost;
            PlaySE(SE_SELECT);
            if (slot >= YGO_SAVED_DECKS)
                Say(ACT_NONE, "There's no empty deck slot. Delete a deck first.");
            else if (lost)
                Ask(ACT_RECIPE, Y.cursor, "Build %s in slot %u? %u of its cards are missing.", gYgoSets[sStarters[Y.cursor]].name,
                    slot + 1, lost);
            else
                Ask(ACT_RECIPE, Y.cursor, "Build %s in slot %u?", gYgoSets[sStarters[Y.cursor]].name, slot + 1);
        }
        break;
    case Y_ACE:
    {
        uint16_t list[YGO_DECK_MAX + YGO_FUSION_MAX];
        unsigned n = AceList((unsigned)Y.deck, list);
        GridMove(pressed, (int)n, ACE_COLS);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(Y_DECKS);
            Y.cursor = Y.deck;
        }
        else if (pressed & KEY_INSPECT && n)
            Inspect(list[Y.cursor], Y_ACE);
        else if (pressed & KEY_A && n)
        {
            c->ace[Y.deck] = list[Y.cursor];
            PlaySE(SE_SUCCESS);
            printf("[ygo] deck %d ace %u\n", Y.deck, c->ace[Y.deck]);
            Screen(Y_DECKS);
            Y.cursor = Y.deck;
        }
        break;
    }
    case Y_BUILDER:
    {
        uint16_t list[YGO_DECK_MAX + YGO_FUSION_MAX];
        unsigned listed = DeckList((unsigned)Y.deck, list);
        char reason[160];
        int n = (int)ListSize();
        FilterTab(pressed);
        n = (int)ListSize();
        if (Y.side == 0)
        {
            if (pressed & KEY_RIGHT && (Y.cursor % GRID_COLS == GRID_COLS - 1 || Y.cursor + 1 >= n) && listed)
            {
                Y.side = 1;
                if (Y.deckCursor >= (int)listed)
                    Y.deckCursor = (int)listed - 1;
                PlaySE(SE_SELECT);
                break;
            }
            GridMove(pressed, n, GRID_COLS);
            if (pressed & KEY_A && n)
            {
                if (Ygo_DeckAdd(c, (unsigned)Y.deck, ListCard(Y.cursor), reason, sizeof(reason)))
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
                unsigned id = ListCard(Y.cursor);
                if (Ygo_DeckRemove(c, (unsigned)Y.deck, id))
                    PlaySE(SE_RG_BAG_POCKET);
                else
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "There's no %s in this deck.", Ygo_Card(id)->name);
                }
            }
            if (pressed & (KEY_START | KEY_INSPECT) && n)
                Inspect(ListCard(Y.cursor), Y_BUILDER);
        }
        else
        {
            if (pressed & KEY_UP && Y.deckCursor > 0)
                Y.deckCursor--, PlaySE(SE_SELECT);
            if (pressed & KEY_DOWN && Y.deckCursor < (int)listed - 1)
                Y.deckCursor++, PlaySE(SE_SELECT);
            if (pressed & KEY_LEFT || !listed)
            {
                /* back on the card the list points at, when this tab shows it */
                int k;
                for (k = 0; listed && k < n; k++)
                    if (ListCard(k) == list[Y.deckCursor < (int)listed ? Y.deckCursor : 0])
                        Y.cursor = k;
                Y.side = 0;
                PlaySE(SE_SELECT);
            }
            else if (pressed & KEY_A && listed)
            {
                Ygo_DeckRemove(c, (unsigned)Y.deck, list[Y.deckCursor]);
                PlaySE(SE_RG_BAG_POCKET);
                listed = DeckList((unsigned)Y.deck, list);
                if (Y.deckCursor >= (int)listed)
                    Y.deckCursor = listed ? (int)listed - 1 : 0;
            }
            if (pressed & (KEY_START | KEY_INSPECT) && listed)
                Inspect(list[Y.deckCursor], Y_BUILDER);
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(Y_DECKS);
            Y.cursor = Y.deck;
            if (!MainCount((unsigned)Y.deck) && !ExtraCount((unsigned)Y.deck))
                Deck((unsigned)Y.deck)->name[0] = 0;
            else if (!DeckValid((unsigned)Y.deck))
            {
                Ygo_DeckValidate(Deck((unsigned)Y.deck)->main, Deck((unsigned)Y.deck)->extra, c->owned, reason,
                                 sizeof(reason));
                Say(ACT_NONE, "This deck can't be played yet: %s", reason);
            }
        }
        break;
    }
    case Y_BINDER:
    {
        int n = (int)ListSize();
        FilterTab(pressed);
        GridMove(pressed, n, 7);
        if (pressed & KEY_SELECT && Y.showAll != SHOP_BINDER && Y.showAll != GC_BINDER)
        {
            Y.showAll = !Y.showAll;
            Y.cursor = 0;
            PlaySE(SE_SELECT);
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            if (Y.showAll == SHOP_BINDER || Y.showAll == GC_BINDER)
            {
                Y.counterTab = Y.showAll == GC_BINDER ? 0 : Y.gc ? 3 : 1;
                Y.showAll = 0;
                Screen(Y_COUNTER);
            }
            else
                Screen(Y_HOME), Y.cursor = 1;
        }
        else if (pressed & KEY_A && n)
        {
            unsigned id = ListCard(Y.cursor);
            if (Y.showAll == GC_BINDER)
            {
                unsigned price = Ygo_MoneyPrice(id);
                if (GcSingleSold(id))
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "SOLD: one copy of each card per customer.");
                }
                else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s is \xa5%u. You don't have enough money.", Ygo_Card(id)->name, price);
                }
                else if (c->owned[id] >= YGO_COLLECTION_MAX)
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "You can't carry more %s.", Ygo_Card(id)->name);
                }
                else
                    Ask(ACT_GC_CARD, (int)id, "%s for \xa5%u?", Ygo_Card(id)->name, price);
            }
            else if (Y.showAll == SHOP_BINDER)
            {
                unsigned cost = Ygo_PointCost(id);
                if (Points() >= cost)
                    Ask(ACT_BUY_CARD, (int)id, "%s for %u CP?", Ygo_Card(id)->name, cost);
                else
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s costs %u CP. You have %u.", Ygo_Card(id)->name, cost, Points());
                }
            }
            else
                Inspect(id, Y_BINDER);
        }
        else if (pressed & (KEY_START | KEY_INSPECT) && n)
            Inspect(ListCard(Y.cursor), Y_BINDER);
        break;
    }
    case Y_INSPECT:
        if (pressed & KEY_UP)
            Y.inspectScroll -= 12;
        if (pressed & KEY_DOWN)
            Y.inspectScroll += 12;
        if (Y.inspectScroll > Y.inspectMax)
            Y.inspectScroll = Y.inspectMax;
        if (Y.inspectScroll < 0)
            Y.inspectScroll = 0;
        if (pressed & KEY_A)
        {
            Y.screen = Y_ART;
            PlaySE(SE_SELECT);
        }
        else if (pressed & (KEY_B | KEY_START | KEY_INSPECT))
        {
            int cursor = Y.cursor;
            PlaySE(SE_SELECT);
            Y.screen = Y.back;
            Y.cursor = cursor;
        }
        break;
    case Y_ART:
        if (pressed & KEY_A)
            Y.zoom ^= 1, Y.panX = Y.panY = 0;
        if (keys & KEY_LEFT)
            Y.panX -= 2;
        if (keys & KEY_RIGHT)
            Y.panX += 2;
        if (keys & KEY_UP)
            Y.panY -= 2;
        if (keys & KEY_DOWN)
            Y.panY += 2;
        Y.panY = Y.panY < -80 ? -80 : Y.panY > 80 ? 80 : Y.panY;
        Y.panX = Y.panX < -60 ? -60 : Y.panX > 60 ? 60 : Y.panX;
        if (pressed & KEY_B)
        {
            Y.screen = Y_INSPECT;
            Y.zoom = Y.panX = Y.panY = 0;
            PlaySE(SE_SELECT);
        }
        break;
    case Y_PACKS:
    {
        uint8_t sets[YGO_SET_COUNT];
        unsigned n = PackSets(sets);
        MoveCursor(pressed, (int)n, 0);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(Y_HOME);
            Y.cursor = 2;
        }
        else if (pressed & KEY_A && n)
        {
            unsigned set = sets[Y.cursor];
            if (c->packs[set])
                OpenPackNow(set);
            else
            {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "You have no unopened packs of this kind.");
            }
        }
        break;
    }
    case Y_REVEAL:
        if (Y.flipped < YGO_PACK_SIZE)
        {
            RevealStep(pressed);
            if (pressed & KEY_B)
            {
                Y.flipped = YGO_PACK_SIZE;
                Y.cursor = YGO_PACK_SIZE - 1;
            }
            break;
        }
        if (pressed & KEY_LEFT && Y.cursor > 0)
            Y.cursor--, PlaySE(SE_SELECT);
        if (pressed & KEY_RIGHT && Y.cursor < YGO_PACK_SIZE - 1)
            Y.cursor++, PlaySE(SE_SELECT);
        if (pressed & KEY_UP && Y.cursor >= 5)
            Y.cursor -= 5, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN && Y.cursor < 4)
            Y.cursor += 5, PlaySE(SE_SELECT);
        if (pressed & (KEY_A | KEY_START))
            Inspect(Y.revealed[Y.cursor], Y_REVEAL);
        else if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(Y_PACKS);
            {
                uint8_t sets[YGO_SET_COUNT];
                unsigned n = PackSets(sets), i;
                for (i = 0; i < n; i++)
                    if (sets[i] == Y.pack)
                        Y.cursor = (int)i;
            }
        }
        break;
    case Y_SPOILS:
        GridMove(pressed, Y.spoilCount, 7);
        if (pressed & KEY_A && Y.spoilCount)
            Ask(ACT_TAKE_SPOIL, Y.cursor, "Take %s?", Ygo_Card(Y.spoils[Y.cursor])->name);
        else if (pressed & KEY_START && Y.spoilCount)
            Inspect(Y.spoils[Y.cursor], Y_SPOILS);
        break;
    case Y_COUNTER:
        if (pressed & (KEY_L | KEY_R))
        {
            Y.counterTab = (Y.counterTab + (pressed & KEY_R ? 1 : CounterTabs() - 1)) % CounterTabs();
            Y.cursor = 0;
            PlaySE(SE_RG_BAG_POCKET);
            break;
        }
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
            break;
        }
        if (CounterKind() == 0)
        {
            if (pressed & KEY_A)
            {
                PlaySE(SE_SELECT);
                Y.filter = 0;
                Screen(Y_BINDER);
                Y.showAll = GC_BINDER;
            }
        }
        else if (CounterKind() == 1)
        {
            MoveCursor(pressed, YGO_STARTERS, 1);
            if (pressed & KEY_A)
            {
                if (GcDeckSold((unsigned)Y.cursor))
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "The %s is sold out. One of each deck per customer!", gYgoSets[sStarters[Y.cursor]].name);
                }
                else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, GC_DECK_PRICE))
                    Say(ACT_NONE, "A Starter Deck is \xa5%u. You don't have enough money.", GC_DECK_PRICE);
                else
                    Ask(ACT_GC_DECK, Y.cursor, "The %s for \xa5%u?", gYgoSets[sStarters[Y.cursor]].name, GC_DECK_PRICE);
            }
            else if (pressed & KEY_START)
            {
                uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
                StarterDeckOf(sStarters[Y.cursor], main, extra);
                Inspect(Mascot(main, YGO_DECK_MAX), Y_COUNTER);
            }
        }
        else if (CounterKind() == 2)
        {
            uint16_t list[YGO_CARD_COUNT];
            unsigned n = SpareList(list);
            MoveCursor(pressed, n ? (int)n + 1 : 0, 1);
            if (pressed & KEY_A && n && Y.cursor == (int)n)
            {
                unsigned i, points = 0;
                for (i = 0; i < n; i++)
                    points += Ygo_Spare(c, list[i]) * Ygo_PointValue(list[i]);
                Ask(ACT_TRADE_IN_ALL, 0, "Trade in every spare card for %u CP?", points);
            }
            else if (Y.cursor == (int)n)
                ;
            else if (pressed & KEY_A && n)
                Ask(ACT_TRADE_IN, list[Y.cursor], "Trade in one %s for %u CP?", Ygo_Card(list[Y.cursor])->name,
                    Ygo_PointValue(list[Y.cursor]));
            else if (pressed & KEY_SELECT && n)
                Ask(ACT_TRADE_IN_CARD, list[Y.cursor], "Trade in all %u spare %s for %u CP?", Ygo_Spare(c, list[Y.cursor]),
                    Ygo_Card(list[Y.cursor])->name, Ygo_Spare(c, list[Y.cursor]) * Ygo_PointValue(list[Y.cursor]));
        }
        else
        {
            uint8_t sets[YGO_SET_COUNT];
            unsigned n = ShopRows(sets);
            MoveCursor(pressed, (int)n + 1, 1);
            if (pressed & KEY_A)
            {
                if (Y.cursor == (int)n)
                {
                    Y.filter = 0;
                    Screen(Y_BINDER);
                    Y.showAll = SHOP_BINDER;
                }   /* (B there comes back to this tab) */
                else if (Points() >= YGO_PACK_COST)
                    Ask(ACT_BUY_PACK, sets[Y.cursor], "A %s booster for %u CARD POINTS?", gYgoSets[sets[Y.cursor]].code,
                        YGO_PACK_COST);
                else
                    Say(ACT_NONE, "A booster takes %u CARD POINTS. You have %u.", YGO_PACK_COST, Points());
            }
        }
        break;
    case Y_WAIT:
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        break;
    case Y_TRADE:
        TradeStep(pressed);
        break;
    case Y_STARTER:
        MoveCursor(pressed, YGO_STARTERS, 0);
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            if (Y.resolve == RESOLVE_LOAN)
                Ask(ACT_LOAN, Y.cursor, "Borrow the %s for this duel?", gYgoSets[sStarters[Y.cursor]].name);
            else
                Ask(ACT_STARTER, Y.cursor, "Take the %s?", gYgoSets[sStarters[Y.cursor]].name);
        }
        else if (pressed & KEY_START)
        {
            uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
            StarterDeckOf(sStarters[Y.cursor], main, extra);
            Inspect(Mascot(main, YGO_DECK_MAX), Y_STARTER);
        }
        else if (pressed & KEY_B && (c->starter || Y.resolve))
            Close();
        break;
    default:
        break;
    }
    return 1;
}

void YgoUi_Draw(void)
{
    Ygo_SetArtMode(gMpConfig.cardArt);
    Ygo_SetBackMode(gMpConfig.cardBack);
    switch (Y.screen)
    {
    case Y_HOME:
        HomeScreen();
        break;
    case Y_CHALLENGE:
        ChallengeScreen();
        break;
    case Y_DECKS:
        DecksScreen();
        break;
    case Y_RECIPES:
        RecipesScreen();
        break;
    case Y_CODE:
        CodeScreen();
        break;
    case Y_ACE:
        AceScreen();
        break;
    case Y_BUILDER:
        BuilderScreen();
        break;
    case Y_BINDER:
        BinderScreen();
        break;
    case Y_INSPECT:
        InspectScreen();
        break;
    case Y_ART:
        ArtScreen();
        break;
    case Y_PACKS:
        PacksScreen();
        break;
    case Y_REVEAL:
        RevealScreen();
        break;
    case Y_SPOILS:
        SpoilsScreen();
        break;
    case Y_COUNTER:
        CounterScreen();
        break;
    case Y_STARTER:
        StarterScreen();
        break;
    case Y_WAIT:
        WaitScreen();
        break;
    case Y_TRADE:
        TradeScreen();
        break;
    case Y_DUEL:
        YgoBoard_Render();
        break;
    default:
        break;
    }
    TcgUi_MessageBox(Y.message, Y.asking, Y.askYes);
}

/* ------------------------------------------------------------------ */
/* The test harness (mp ygo ...)                                        */
/* ------------------------------------------------------------------ */
void Ygo_FieldDebugDeck(unsigned theme, unsigned tier);
void Ygo_Debug(const char *command)
{
    struct YgoCollection *c = C();
    unsigned n = 0, m = 0;
    if (sscanf(command, "starter %u", &n) == 1)
    {
        Ygo_ChooseStarter(c, sStarters[n % YGO_STARTERS]);
        printf("[ygo] starter %s, deck %u+%u\n", gYgoSets[sStarters[n % YGO_STARTERS]].code, MainCount(0), ExtraCount(0));
        return;
    }
    if (!strcmp(command, "case"))
    {
        YgoUi_OpenCase(NULL, 0);
        return;
    }
    if (!strcmp(command, "club"))
    {
        YgoUi_OpenCounter();
        Y.fromScript = 0;
        return;
    }
    if (!strcmp(command, "pickstarter"))
    {
        YgoUi_OpenStarter();
        Y.fromScript = 0;
        return;
    }
    if (!strcmp(command, "lesson"))
    {
        YgoUi_OpenLesson();
        Y.fromScript = 0;
        return;
    }
    if (!strcmp(command, "practice"))
    {
        if (Y.screen == Y_CLOSED)
            YgoUi_OpenCase(NULL, 0);
        Practice();
        return;
    }
    if (!strcmp(command, "open"))
    {
        struct YgoOpponent o;
        memset(&o, 0, sizeof(o));
        snprintf(o.name, sizeof(o.name), "TEST DUELIST");
        StarterDeckOf(YGO_SET_SDJ, o.main, o.extra);
        o.special = YGO_SPECIAL_NONE;
        o.key = 0x7e58;
        o.set = YGO_SET_LOB;
        YgoUi_OpenChallenge(&o);
        return;
    }
    if (sscanf(command, "points %u", &n) == 1)
    {
        Tcg_GetCollection()->dust = n;
        return;
    }
    if (sscanf(command, "packs %u %u", &n, &m) == 2 && n < YGO_SET_COUNT)
    {
        c->packs[n] = (uint16_t)m;
        return;
    }
    if (sscanf(command, "own %u %u", &n, &m) == 2 && n && n <= YGO_CARD_COUNT)
    {
        c->owned[n] = (uint16_t)m;
        return;
    }
    if (!strcmp(command, "giveall"))
    {
        unsigned id;
        for (id = 1; id <= YGO_CARD_COUNT; id++)
            if (!c->owned[id] && Ygo_CardReady(id))
                c->owned[id] = 1;
        return;
    }
    if (sscanf(command, "inspect %u", &n) == 1)
    {
        if (Y.screen == Y_CLOSED)
            YgoUi_OpenCase(NULL, 0);
        Inspect(n, Y.screen == Y_INSPECT ? Y_HOME : Y.screen);
        return;
    }
    if (!strcmp(command, "binder"))
    {
        if (Y.screen == Y_CLOSED)
            YgoUi_OpenCase(NULL, 0);
        Screen(Y_BINDER);
        Y.filter = 0;
        return;
    }
    if (!strcmp(command, "builder"))
    {
        if (Y.screen == Y_CLOSED)
            YgoUi_OpenCase(NULL, 0);
        Y.deck = c->activeDeck;
        Y.side = 0;
        Y.filter = 0;
        Screen(Y_BUILDER);
        return;
    }
    if (sscanf(command, "pack %u", &n) == 1 && n < YGO_SET_COUNT)
    {
        if (Y.screen == Y_CLOSED)
            YgoUi_OpenCase(NULL, 0);
        if (!c->packs[n])
            c->packs[n] = 1;
        OpenPackNow(n);
        return;
    }
    if (sscanf(command, "deck %u %u", &n, &m) == 2)
    {
        Ygo_FieldDebugDeck(n, m);
        return;
    }
    if (sscanf(command, "first %d", (int *)&n) == 1)
    {
        sForceFirst = (int)n;
        return;
    }
    if (sscanf(command, "fieldbg %u", &n) == 1)
    {
        YgoBoard_TestFieldBg(n);
        return;
    }
    if (sscanf(command, "seed %u", &n) == 1)
    {
        /* the next duels' shuffles from a known seed (tests: the same duel every run) */
        C()->rng = n ? n : 1;
        return;
    }
    if (!strcmp(command, "auto"))
    {
        Y.autoPlay = !Y.autoPlay;
        return;
    }
    if (!strcmp(command, "fast"))
    {
        Tcg_GetCollection()->fastAnimations = 1;
        YgoBoard_SetFast(1);
        return;
    }
    if (!strcmp(command, "nodecks"))
    {
        /* every saved deck emptied (the cards stay): the "no deck ready" case */
        memset(c->decks, 0, sizeof(c->decks));
        return;
    }
    if (!strcmp(command, "concede"))
    {
        /* (online through the link, as the table's CONCEDE) */
        if (Y.online && !YgoNet_Concede(Ygo_Online(), SDL_GetTicks()))
            Y.concedeHeld = 1;
        else if (!Y.online)
            YgoDuel_Concede(&Y.duel, 0);
        printf("[ygo] conceded\n");
        return;
    }
    if (!strcmp(command, "win"))
    {
        YgoDuel_Concede(&Y.duel, 1);
        printf("[ygo] opponent conceded\n");
        return;
    }
    if (!strncmp(command, "code ", 5))
    {
        /* FROM CODE on the first empty slot, with this code as if pasted */
        if (Y.screen == Y_CLOSED)
            YgoUi_OpenCase(NULL, 0);
        OpenCode((int)EmptySlot());
        snprintf(Y.code, sizeof(Y.code), "%s", command + 5);
        return;
    }
    if (sscanf(command, "mineuntil %u", &n) == 1)
    {
        /* your questions answered for you (the AI's answers; the Battle Phase taken when it's offered) until one of
         * type n (YGO_Q_*) is asked: then "[mp] ygo mine reached" */
        sMineUntil = (int)n;
        return;
    }
    if (!strcmp(command, "mine"))
    {
        /* one AI answer for you, shown like your own */
        struct YgoAnswer a;
        if (Y.screen == Y_DUEL && YgoBoard_Idle() && Y.duel.query.player == 0 && Y.duel.query.type != YGO_Q_NONE)
        {
            YgoDuel_AiAnswer(&Y.duel, &a);
            YgoDuel_Answer(&Y.duel, &a);
        }
        printf("[ygo] mine: query %u player %u\n", Y.duel.query.type, Y.duel.query.player);
        return;
    }
    if (sscanf(command, "focus %u %u", &n, &m) == 2)
    {
        YgoBoard_DebugFocus((int)n, (int)m);
        return;
    }
    if (!strcmp(command, "state"))
    {
        const struct YgoState *s = &Y.duel.s;
        unsigned total, have = Ygo_SetOwned(c, YGO_SET_COUNT, &total);
        printf("[ygo] screen %d opted %u starter %u deck %u+%u valid %d owned %u/%u points %u packs %u wins %u "
               "duel: turn %u phase %u current %u query %u player %u lp %d/%d hands %u/%u over %u winner %u idle %d "
               "message %d conserved %d online %d hash %08x\n",
               Y.screen, c->optedIn, c->starter, MainCount(c->activeDeck), ExtraCount(c->activeDeck),
               DeckValid(c->activeDeck), have, total, Points(), PacksHeld(), c->wins, s->turn, s->phase, s->current,
               Y.duel.query.type, Y.duel.query.player, s->players[0].lp, s->players[1].lp, s->players[0].handCount,
               s->players[1].handCount, s->over, s->winner, Y.screen == Y_DUEL ? YgoBoard_Idle() : -1,
               Y.message[0] != 0, Y.screen == Y_DUEL ? YgoDuel_Conserved(&Y.duel) : -1, Y.online,
               (unsigned)YgoNet_Hash(&Y.duel));
        return;
    }
}
