/* The CARD CASE's TRIPLE TRIAD side: decks of five cards (no recipes: every hand is your own), the card list sorted
 * by stars, the packs, the rules of each region of HOENN, a player's challenge, the table (tt_board.c), the cards
 * a win lets you take, the sellers' counters and the CARD QUEEN's favors. Drawn with the other card games' look
 * (src/tcg/tcg_ui_kit.h) through their canvas. Losing never costs a card: only the player takes cards. */
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "constants/flags.h"
#include "mp.h"
#include "money.h"
#include "script.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg/integration.h"
#include "tcg/npc.h"
#include "tcg/online.h"
#include "tcg/quotes.h"
#include "tt/collection.h"
#include "tt/duel.h"
#include "tt/integration.h"
#include "tt/net.h"
#include "tt/npc.h"
#include "tt/rating.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_ui_kit.h"
#include "../tcg/tcg_guide.h"
#include "../tcg/tcg_search.h"
#include "tt_board.h"
#include "tt_gfx.h"

enum
{
    T_CLOSED,
    T_HOME,
    T_DECKS,
    T_BUILDER,
    T_LIST,
    T_INSPECT,
    T_PACKS,
    T_REVEAL,
    T_RULES,
    T_CHALLENGE,
    T_DUEL,
    T_SPOILS,
    T_SHOP,
    T_STARTER,
    T_QUEEN,
    T_WAIT,       /* a friend's game: waiting for their hand */
    T_TRADE,      /* a card trade with a friend */
    T_TRAVEL    /* the CARD QUEEN: where she goes next */
};
enum
{
    KEY_A = 1, KEY_B = 2, KEY_SELECT = 4, KEY_START = 8, KEY_RIGHT = 16, KEY_LEFT = 32, KEY_UP = 64, KEY_DOWN = 128,
    KEY_R = 256, KEY_L = 512,
    KEY_INSPECT = 2048,  /* R2 / ZR / RT, or D on the keyboard: a card up close (gamepad.h PLATFORM_KEY_INSPECT) */
    KEY_SEARCH = 0x1000  /* L2 / ZL / LT, or F: SEARCH & SORT in the list and the builder (PLATFORM_KEY_SEARCH) */
};
enum { VS_NPC, VS_PRACTICE, VS_FRIEND };
enum
{
    ACT_NONE,
    ACT_CLOSE,
    ACT_HOME,
    ACT_COPY,
    ACT_CLEAR,
    ACT_BUY_CARD,
    ACT_BUY_PACK,
    ACT_OPEN_BOUGHT,
    ACT_SELL,
    ACT_TAKE,         /* the chosen spoils */
    ACT_LEAVE_DECK,   /* leave the builder with a deck that can't be played */
    ACT_QUEEN_TRAVEL, /* pay for her journey */
    ACT_QUEEN_TRADE,  /* she changes the trade rule here to askArg */
    ACT_QUEEN_ABOLISH,
    ACT_CHALLENGE_GO  /* the rules changed: on to the challenge */
};
#define PANEL_W KIT_PANEL_W
#define STRIP_Y KIT_STRIP_Y
#define LIST_X KIT_LIST_X
#define LIST_W KIT_LIST_W
#define ROW_H KIT_ROW_H
#define TT_THEME 0x1d6f86        /* the case's TRIPLE TRIAD side: deep teal, with gold */
#define TT_GOLD 0xd8b060
#define QUEEN_FARE 5000          /* the CARD QUEEN's journey */
#define GRID_COLS 7
#define GRID_ROWS 3

static struct
{
    int screen, back, cursor, scroll, frame, age;
    unsigned previousKeys, held;
    int card, stars, deck, deckSlot, inDeckRow, textScroll;
    int pack, revealT, revealCount;
    uint16_t revealed[TT_HAND];
    uint8_t revealNew;
    int vs, aiDelay, rewarded, outcome;
    struct TtOpponent opp;
    uint8_t oppLevel;          /* (how well they play before a fit raises it) */
    struct TtDuel duel;
    uint16_t dealt[2][TT_HAND];
    unsigned region;
    uint32_t rules;
    unsigned trade;
    SDL_Texture *portrait;
    /* spoils */
    int spoilAllowed;
    uint8_t spoilPicked, spoilFixed;
    /* the seller's counter */
    int shopRegion, shopTab;
    /* messages */
    char message[256];
    int messageThen, asking, askYes, askAction, askArg;
    int fromScript;
    void (*returnToGame)(void);
    int menuOn, menuCursor;
    /* with a friend (tt_net.c) */
    int online, concedeHeld, onlineDeckChosen;
} U;

static struct TtCollection *C(void) { return Tt_GetCollection(); }
int Tcg_GrantCase(void);
static int sForceFirst = -1;   /* test harness: who goes first (-1: at random) */
static int sAutoPlay;          /* test harness: the computer plays our side */

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
    if (screen != T_SPOILS && screen != T_INSPECT)
        TtBoard_ReleaseMusic();   // the victory song ends with the cards to take
    U.screen = screen;
    U.cursor = 0;
    U.scroll = 0;
    U.age = 0;
}
int TtUi_Active(void) { return U.screen != T_CLOSED; }
unsigned TtUi_HiddenFrom(void)
{
    unsigned peer = Tt_Online()->peer;
    return U.online && U.screen != T_CLOSED && peer < MP_MAX_PLAYERS ? 1u << peer : 0;
}
static const char *PeerName(unsigned id)
{
    static char name[MP_NAME_LEN * 2 + 2];
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return "FRIEND";
    MP_GameToAscii(name, gMpPlayers[id].name, MP_NAME_LEN * 2 + 1);
    return name;
}
unsigned TtUi_DuelingNow(void) { return U.screen == T_DUEL; }

static void Close(void)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    if (U.online)
        TtNet_Cancel(Tt_Online(), "The game was closed.");
    U.online = 0;
    U.concedeHeld = 0;
    if (U.vs == VS_FRIEND && (Tcg_Social()->role == TCG_SOCIAL_DECK || Tcg_Social()->role == TCG_SOCIAL_TRADE))
        Tcg_SocialLeave();
    U.vs = VS_NPC;
    if (U.screen == T_DUEL || U.back == T_DUEL)
        TtBoard_End();
    U.message[0] = 0;
    U.asking = 0;
    U.menuOn = 0;
    Screen(T_CLOSED);
    U.returnToGame = NULL;
    U.fromScript = 0;
    if (U.portrait)
        SDL_DestroyTexture(U.portrait);
    U.portrait = NULL;
    Tt_ForgetTextures();
    Tcg_CloseScreens(callback, script);
}
static void Open(int screen)
{
    U.message[0] = 0;
    U.asking = 0;
    U.menuOn = 0;
    if (U.portrait)
        SDL_DestroyTexture(U.portrait);
    U.portrait = NULL;
    Screen(screen);
    PlaySE(SE_WIN_OPEN);
}
void TtUi_OpenCase(void (*returnToGame)(void), int fromScript)
{
    U.returnToGame = returnToGame;
    U.message[0] = 0;
    U.asking = 0;
    U.menuOn = 0;
    Screen(T_HOME);
    U.fromScript = fromScript;
    if (!C()->optedIn)
        Say(ACT_NONE, "No TRIPLE TRIAD cards yet. The TRIPLE TRIAD sellers in POKeMON CENTERS and the GAME CENTER "
                      "hand out a first hand!");
}
/* To another side of the case: L the one before, R the one after (POKeMON, YU-GI-OH!, TRIPLE TRIAD) */
static void ToSide(int right)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.message[0] = 0;
    Screen(T_CLOSED);
    Tt_ForgetTextures();
    C()->flags &= ~TT_FLAG_CASE_SIDE;
    Tcg_OpenCaseGame(right ? KIT_GAME_POKEMON : KIT_GAME_YUGIOH, callback, script);
}

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */
static unsigned Owned(unsigned id) { return id && id <= TT_CARD_MAX ? C()->owned[id] : 0; }
static struct TtDeck *Deck(unsigned d) { return &C()->decks[d]; }
static unsigned DeckCount(unsigned d)
{
    unsigned i, n = 0;
    for (i = 0; i < TT_HAND; i++)
        n += Tt_Card(Deck(d)->cards[i]) != NULL;
    return n;
}
static int DeckOk(unsigned d) { return Tt_DeckReady(C(), d); }
/* The card that stands for a deck: its most stars, the first such */
static unsigned DeckFace(unsigned d)
{
    unsigned i, best = 0;
    for (i = 0; i < TT_HAND; i++)
    {
        const struct TtCard *c = Tt_Card(Deck(d)->cards[i]);
        if (c && (!best || c->stars > Tt_Card(best)->stars))
            best = Deck(d)->cards[i];
    }
    return best;
}
static void StripCard(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash)
{
    struct TtFace face;
    (void)angle;
    memset(&face, 0, sizeof(face));
    face.seat = TT_SEAT_BLUE;
    face.glow = (uint8_t)(flash ? 120 : 0);
    Tt_DrawCard(id, x, y, w, h, 1, 1, alpha, &face);
}
static void Strip(unsigned card, const char *text) { TcgUi_Strip(card ? StripCard : NULL, card, text); }
static void Card(unsigned id, float x, float y, float w, float h, unsigned seat, unsigned alpha)
{
    struct TtFace face;
    memset(&face, 0, sizeof(face));
    face.seat = (uint8_t)seat;
    G_FillA((int)x + 2, (int)y + 3, (int)w, (int)h - 2, 0x000000, alpha * 70 / 255);
    Tt_DrawCard(id, x, y, w, h, 1, 1, alpha, &face);
}
/* Cards of a star tab (1-5; 0: every star), in Final Fantasy XIV's order */
static unsigned StarList(unsigned stars, uint16_t *out, int ownedOnly)
{
    static uint16_t sorted[TT_CARD_MAX + 1];
    static int ready;
    unsigned i, n = 0, k = 0;
    if (!ready)
    {
        unsigned id, j;
        for (id = 1; id <= TT_CARD_MAX; id++)
            if (Tt_Card(id))
                sorted[k++] = (uint16_t)id;
        for (i = 1; i < k; i++)   /* by XIV's number (insertion: the list is nearly sorted already) */
            for (j = i; j > 0 && Tt_Card(sorted[j])->order < Tt_Card(sorted[j - 1])->order; j--)
            {
                uint16_t t = sorted[j];
                sorted[j] = sorted[j - 1];
                sorted[j - 1] = t;
            }
        sorted[k] = 0;
        ready = 1;
    }
    for (i = 0; sorted[i]; i++)
    {
        const struct TtCard *c = Tt_Card(sorted[i]);
        if ((stars && c->stars != stars) || (ownedOnly && !Owned(sorted[i])))
            continue;
        out[n++] = sorted[i];
    }
    return n;
}
/* SEARCH & SORT (src/tcg/tcg_search.c) over the list and the builder. Cards not met yet keep their secrets: no name
   to find, no sides or type to filter or sort by. */
enum { TK_STARS = SEARCH_KEY_GAME, TK_UP, TK_RIGHT, TK_DOWN, TK_LEFT, TK_TYPE, TK_TOTAL };
static const char *const sSearchStars[] = {"ALL", "1", "2", "3", "4", "5"};
static const int16_t sSearchStarValues[] = {SEARCH_ALL, 1, 2, 3, 4, 5};
static const char *const sSearchSides[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "A"};
static const char *const sSearchTypes[] = {"PRIMAL", "SCION", "SOCIETY", "GARLEAN", "NO TYPE"};
static const int16_t sSearchTypeValues[] = {TT_TYPE_PRIMAL, TT_TYPE_SCION, TT_TYPE_SOCIETY, TT_TYPE_GARLEAN, TT_TYPE_NONE};
enum { SEARCH_STARS_FILTER = 0 };   /* (BY TAB: the star tabs choose; ALL or a star: they step aside) */
static const struct SearchFilter sSearchFilters[] = {
    {"STARS", SF_CHOICE, TK_STARS, 6, sSearchStars, sSearchStarValues, 0, 0, "BY TAB"},
    {"MIN TOP", SF_MIN, TK_UP, 10, sSearchSides, NULL, 1, 1, NULL},
    {"MIN RIGHT", SF_MIN, TK_RIGHT, 10, sSearchSides, NULL, 1, 1, NULL},
    {"MIN BOTTOM", SF_MIN, TK_DOWN, 10, sSearchSides, NULL, 1, 1, NULL},
    {"MIN LEFT", SF_MIN, TK_LEFT, 10, sSearchSides, NULL, 1, 1, NULL},
    {"TYPE", SF_CHOICE, TK_TYPE, 5, sSearchTypes, sSearchTypeValues, 0, 0, NULL},
};
static const struct SearchSort sSearchSorts[] = {
    {"NUMBER", SEARCH_KEY_NUMBER}, {"NAME", -1}, {"STARS", SEARCH_KEY_RARITY}, {"OWNED", -2},
    {"TOP", TK_UP}, {"RIGHT", TK_RIGHT}, {"BOTTOM", TK_DOWN}, {"LEFT", TK_LEFT}, {"TOTAL", TK_TOTAL},
};
static int Met(unsigned id) { return Owned(id) || Tt_Seen(C(), id); }
static const char *SearchName(unsigned id) { return Tt_Card(id) && Met(id) ? Tt_Card(id)->name : ""; }
static int SearchValue(unsigned id, int key)
{
    const struct TtCard *c = Tt_Card(id);
    if (!c)
        return SEARCH_NA;
    if (key == SEARCH_KEY_NUMBER)
        return c->order;
    if (key == SEARCH_KEY_RARITY || key == TK_STARS)
        return c->stars;   /* (the tabs show it anyway) */
    if (!Met(id))
        return SEARCH_NA;
    switch (key)
    {
    case TK_UP: return c->side[TT_UP];
    case TK_RIGHT: return c->side[TT_RIGHT];
    case TK_DOWN: return c->side[TT_DOWN];
    case TK_LEFT: return c->side[TT_LEFT];
    case TK_TYPE: return c->type;
    case TK_TOTAL: return c->side[0] + c->side[1] + c->side[2] + c->side[3];
    }
    return SEARCH_NA;
}
static const struct SearchGame sSearchGame = {"triad", sSearchFilters, sizeof(sSearchFilters) / sizeof(sSearchFilters[0]),
                                              sSearchSorts, sizeof(sSearchSorts) / sizeof(sSearchSorts[0]), SearchName,
                                              SearchValue, Owned};
static int Searchable(void) { return U.screen == T_LIST || U.screen == T_BUILDER; }
/* The star the list shows (0: every star): the tab, unless SEARCH & SORT's STARS says otherwise */
static unsigned ShownStars(void)
{
    int choice;
    TcgSearch_Register(SEARCH_TRIAD, &sSearchGame);
    choice = TcgSearch_Choice(SEARCH_TRIAD, SEARCH_STARS_FILTER);
    return choice == 0 ? (unsigned)U.stars : choice == 1 ? 0 : (unsigned)choice - 1;
}
/* The list's and the builder's cards as shown, kept until something they depend on changes */
static uint16_t sShown[TT_CARD_MAX + 1];
static unsigned sShownN, sShownKey[6], sShownReady;
static unsigned Shown(uint16_t *out, int ownedOnly)
{
    struct TtCollection *c = C();
    unsigned key[6], id, hash = 0;
    TcgSearch_Register(SEARCH_TRIAD, &sSearchGame);
    for (id = 1; id <= TT_CARD_MAX; id++)
        hash = hash * 31 + c->owned[id] * 2 + (unsigned)Tt_Seen(c, id);
    key[0] = (unsigned)ownedOnly, key[1] = ShownStars(), key[2] = (unsigned)U.stars;
    key[3] = TcgSearch_Generation(SEARCH_TRIAD), key[4] = hash, key[5] = 0;
    if (!sShownReady || memcmp(key, sShownKey, sizeof(key)))
    {
        sShownN = TcgSearch_Apply(SEARCH_TRIAD, sShown, StarList(key[1], sShown, ownedOnly));
        memcpy(sShownKey, key, sizeof(key));
        sShownReady = 1;
    }
    memcpy(out, sShown, sShownN * sizeof(sShown[0]));
    return sShownN;
}
static unsigned BinderList(uint16_t *out) { return Shown(out, 0); }
static void StarTabs(int x, int y, int selected)
{
    int i;
    for (i = 1; i <= TT_MAX_STARS; i++)
    {
        int on = i == selected, w = 8 + i * 7;
        unsigned fill = on ? TT_GOLD : 0x40586c;
        G_RoundFill(x, y + (on ? 0 : 2), w, on ? 13 : 11, fill, 255);
        G_RoundFrame(x, y + (on ? 0 : 2), w, on ? 13 : 11, on ? 0xfff0c0 : 0x182838, 255);
        Tt_DrawStars((unsigned)i, x + 3, y + (on ? 2 : 3), 9, on ? 255 : 160);
        x += w + 2;
    }
    G_Hint(x + 2, y, G_WHITE, G_NO_SHADOW, "{L}{R}", TF_NARROW, 1);
}
static const char *StarWord(unsigned stars)
{
    static const char *const w[] = {"", "ONE STAR", "TWO STARS", "THREE STARS", "FOUR STARS", "FIVE STARS"};
    return stars <= 5 ? w[stars] : "";
}
static void Numbers(char *out, size_t size, unsigned id)
{
    const struct TtCard *c = Tt_Card(id);
    static const char digits[] = "0123456789A";
    if (!c)
    {
        out[0] = 0;
        return;
    }
    snprintf(out, size, "%c-%c-%c-%c", digits[c->side[TT_UP]], digits[c->side[TT_RIGHT]], digits[c->side[TT_DOWN]],
             digits[c->side[TT_LEFT]]);
}
#define NO_TRADE 0xFF   /* a friend's game */
static void RulesLine(char *out, size_t size, uint32_t rules, unsigned trade)
{
    unsigned r;
    int n = 0;
    out[0] = 0;
    for (r = 0; r < TT_RULE_COUNT; r++)
        if (rules >> r & 1)
            n += snprintf(out + n, size - (size_t)n, "%s%s", n ? ", " : "", Tt_RuleName(r));
    if (!n)
        n += snprintf(out, size, "No special rules");
    if (trade != NO_TRADE)
        snprintf(out + n, size - (size_t)n, "\nTRADE RULE: %s", Tt_TradeName(trade));
    else
        snprintf(out + n, size - (size_t)n, "\nBetween friends no cards change hands.");
}
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
    if (U.opp.portraitW == 64)   /* their battle picture, cut to the window beside their card */
    {
        G_PicCut(U.portrait, U.opp.portrait, 8, 22, 41, 42);   /* (their hand starts at 50) */
        return;
    }
    dst.x = x - U.opp.portraitW;
    dst.y = y - U.opp.portraitH * 2;
    dst.w = U.opp.portraitW * 2;
    dst.h = U.opp.portraitH * 2;
    SDL_RenderCopy(G_R(), U.portrait, NULL, &dst);
}
static void MoveCursor(unsigned pressed, int count, int step)
{
    int old = U.cursor;
    if (pressed & KEY_UP)
        U.cursor -= step;
    if (pressed & KEY_DOWN)
        U.cursor += step;
    if (U.cursor < 0)
        U.cursor = count - 1;
    if (U.cursor >= count)
        U.cursor = 0;
    if (old != U.cursor)
        PlaySE(SE_SELECT);
}
static void GridMove(unsigned pressed, int count, int cols)
{
    int old = U.cursor;
    if (!count)
        return;
    if (pressed & KEY_LEFT && U.cursor > 0)
        U.cursor--;
    if (pressed & KEY_RIGHT && U.cursor < count - 1)
        U.cursor++;
    if (pressed & KEY_UP && U.cursor >= cols)
        U.cursor -= cols;
    if (pressed & KEY_DOWN)
        U.cursor = U.cursor + cols < count ? U.cursor + cols : count - 1;
    if (old != U.cursor)
        PlaySE(SE_SELECT);
}
static void StarTabStep(unsigned pressed)
{
    if (pressed & (KEY_L | KEY_R))
    {
        U.stars = (U.stars - 1 + (pressed & KEY_R ? 1 : TT_MAX_STARS - 1)) % TT_MAX_STARS + 1;
        U.cursor = 0;
        PlaySE(SE_SELECT);
    }
}
/* The list's and the builder's L / R: SEARCH & SORT's STARS gives way to the tabs again, from the star it showed */
static void ListTabStep(unsigned pressed)
{
    int choice = TcgSearch_Choice(SEARCH_TRIAD, SEARCH_STARS_FILTER);
    if (pressed & (KEY_L | KEY_R) && choice)
    {
        if (choice >= 2)
            U.stars = choice - 1;
        TcgSearch_SetChoice(SEARCH_TRIAD, SEARCH_STARS_FILTER, 0);
    }
    StarTabStep(pressed);
}
static int ListIndexOf(const uint16_t *list, unsigned n, unsigned card)
{
    unsigned i;
    for (i = 0; i < n && card; i++)
        if (list[i] == card)
            return (int)i;
    return 0;
}
/* SEARCH & SORT over the list or the builder's cards: {SEARCH} opens it; it has the buttons while it's open */
static int SearchStep(unsigned pressed)
{
    static uint16_t list[TT_CARD_MAX + 1];
    unsigned n;
    if (!Searchable())
        return 0;
    n = Shown(list, U.screen == T_BUILDER);
    if (TcgSearch_Active())
    {
        unsigned keep = n && U.cursor < (int)n ? list[U.cursor] : 0;
        if (TcgSearch_Step(pressed))
        {
            n = Shown(list, U.screen == T_BUILDER);
            U.cursor = ListIndexOf(list, n, keep);
        }
        return 1;
    }
    if (U.cursor > 0 && U.cursor >= (int)n)
        U.cursor = n ? (int)n - 1 : 0;
    if (pressed & KEY_SEARCH && (U.screen == T_LIST || !U.inDeckRow))
    {
        TcgSearch_Open(SEARCH_TRIAD);
        return 1;
    }
    return 0;
}
static void Inspect(unsigned id, int back)
{
    if (!Tt_Card(id))
        return;
    U.card = (int)id;
    U.back = back;
    U.textScroll = 0;
    {
        int keep = U.cursor;
        Screen(T_INSPECT);
        U.cursor = keep;
    }
    PlaySE(SE_SELECT);
}

/* ------------------------------------------------------------------ */
/* Games                                                                */
/* ------------------------------------------------------------------ */
static unsigned Me(void) { return U.online ? Tt_Online()->seat : 0; }
static const char *MyName(void)
{
    static char name[16];
    MP_GameToAscii(name, gSaveBlock2Ptr->playerName, sizeof(name));
    return name;
}
/* CARD SCALING: the hand they bring against deck d (fitted when they may be; under OPEN the hand shown stays) */
static int Fits(void) { return gMpConfig.cardScaling && U.opp.fit && U.vs == VS_NPC; }
static void TheirHandLevel(unsigned d, uint16_t *hand, unsigned *level)
{
    struct TtOpponent o = U.opp;
    if (Fits() && !(U.rules & TT_RULE_BIT(TT_RULE_OPEN)) && DeckOk(d))
    {
        o.level = U.oppLevel;   /* (as they first were: the fit may raise it) */
        Tt_FitOpponentHand(&o, Tt_HandPower(Deck(d)->cards) + C()->skill, U.rules, Tcg_Today(), hand);
    }
    else
        memcpy(hand, U.opp.hand, sizeof(U.opp.hand));
    if (level)
        *level = o.level;
}
static void TheirHandFor(unsigned d, uint16_t *hand)
{
    TheirHandLevel(d, hand, NULL);
}
/* How the game looks with deck d: the chance and their cards' edge */
static unsigned MatchFor(unsigned d, int *cards, unsigned *shownLevel)
{
    uint16_t theirs[TT_HAND];
    unsigned lv;
    int level;
    if (U.rules & TT_RULE_BIT(TT_RULE_RANDOM))
    {
        *cards = 0;   /* (dealt fair) */
        *shownLevel = U.opp.level;
        return Tt_Chance(Tt_OpponentFavor(&U.opp) + C()->skill, 0);
    }
    TheirHandLevel(d, theirs, &lv);
    *shownLevel = lv;
    level = Tt_LevelRating(lv, U.rules);
    *cards = Tt_HandPower(theirs) - Tt_HandPower(Deck(d)->cards);
    return Tt_Chance(Tt_HandPower(Deck(d)->cards) + C()->skill + Tt_PlayerRating(U.rules), Tt_HandPower(theirs) + level);
}
static int StartGame(const uint16_t *mine)
{
    struct TtCollection *c = C();
    uint16_t hand[TT_HAND];
    uint32_t seed = Tcg_DuelSeed(&c->rng);
    unsigned first = sForceFirst >= 0 ? (unsigned)sForceFirst : Tt_Rand(&c->rng) & 1, i;
    memcpy(hand, mine, sizeof(hand));
    if (U.rules & TT_RULE_BIT(TT_RULE_RANDOM))
    {
        /* dealt fair to both: yours as close as a few deals come to their usual hand (a touch in your favor), then
         * theirs from their region's cards as close to yours */
        int level = Tt_LevelRating(U.opp.level, U.rules) - Tt_PlayerRating(U.rules), favor = Tt_OpponentFavor(&U.opp);
        if (!Tt_DealFair(c->owned, seed ^ 0x5eed, Tt_HandPower(U.opp.hand) + level + favor - c->skill, hand))
        {
            Say(ACT_NONE, "RANDOM deals five of your own cards, and you own fewer than five.");
            return 0;
        }
        Tt_DealRegion(U.opp.region, seed ^ 0xdea1, Tt_HandPower(hand) + c->skill - favor - level, U.opp.hand);
        printf("[tt] RANDOM dealt: yours %d, theirs %d (level %u)\n", Tt_HandPower(hand), Tt_HandPower(U.opp.hand),
               U.opp.level);
    }
    memcpy(U.dealt[0], hand, sizeof(hand));
    memcpy(U.dealt[1], U.opp.hand, sizeof(hand));
    for (i = 0; i < TT_HAND; i++)
    {
        Tt_See(c, hand[i]);
        Tt_See(c, U.opp.hand[i]);
    }
    TtDuel_Start(&U.duel, U.rules, U.trade, seed, hand, U.opp.hand, first);
    U.rewarded = 0;
    U.outcome = -1;
    U.aiDelay = 0;
    TtBoard_SetFast(Tcg_GetCollection()->fastAnimations);
    TtBoard_Lesson(U.opp.lesson);
    Screen(T_DUEL);
    TtBoard_Begin(&U.duel, Me(), MyName(), U.opp.name, U.opp.music, 0);
    printf("[tt] game vs %s: rules %x trade %u first %u\n", U.opp.name, U.rules, U.trade, first);
    return 1;
}
static void Practice(void)
{
    struct TtCollection *c = C();
    unsigned region = Tt_CurrentRegion(), tier;
    u8 badges = 0, i;
    for (i = 0; i < 8; i++)
        badges += FlagGet(FLAG_BADGE01_GET + i) != 0;
    tier = badges >= 7 ? 3 : badges >= 4 ? 2 : badges >= 2 ? 1 : 0;
    if (!DeckOk(c->activeDeck))
    {
        Say(ACT_NONE, "Your deck isn't ready. Pick five cards in DECKS first.");
        return;
    }
    memset(&U.opp, 0, sizeof(U.opp));
    snprintf(U.opp.name, sizeof(U.opp.name), "PRACTICE");
    U.opp.special = TT_SPECIAL_NONE;
    U.opp.level = (uint8_t)(tier < 2 ? 1 : 2);
    U.opp.region = (uint8_t)region;
    Tt_RegionHand(region < TT_REGIONS ? region : 0, Tt_Rand(&c->rng), tier, Tcg_Today(), U.opp.hand);
    U.vs = VS_PRACTICE;
    U.region = region;
    U.rules = region < TT_REGIONS ? c->regionRules[region] : 0;
    U.trade = TT_TRADE_ONE;
    StartGame(Deck(c->activeDeck)->cards);
}
/* After the game: the records, the rules the player carries now, what was won */
static void Results(void)
{
    struct TtCollection *c = C();
    unsigned w = U.duel.winner, i;
    char line[320] = "";
    int n = 0;
    U.rewarded = 1;
    for (i = 0; i < TT_HAND; i++)
        Tt_See(c, U.dealt[1][i]);
    if (U.vs == VS_FRIEND)
    {
        /* between friends: the records, never cards */
        if (w == Me())
        {
            c->onlineWins++;
            MP_Achieve_Count("ttwin", 1);
            MP_Achieve_Count("cardfriend", 1);   /* TABLE TALK counts every card game */
        }
        else if (w == 2)
            c->draws++;
        TtBoard_Result(w == Me() ? "You won! Games between friends trade no cards."
                       : w == 2  ? "A draw! Games between friends trade no cards."
                                 : "Games between friends trade no cards.");
        printf("[tt] friend's game %s\n", w == Me() ? "won" : w == 2 ? "drawn" : "lost");
        return;
    }
    if (U.vs == VS_PRACTICE)
    {
        TtBoard_Result(w == Me() ? "A fine game! Practice games trade no cards." : "Practice games trade no cards.");
        return;
    }
    if (U.vs == VS_NPC && w != 2)
    {
        /* your record against them, and what they say (include/tcg/quotes.h) */
        CardTable_Result(U.opp.key, QUOTE_GAME_TT, w == Me());
        if ((n = CardTable_Goodbye(line, sizeof(line), U.opp.name, U.opp.key, QUOTE_GAME_TT,
                                   U.opp.quoteWho[0] ? U.opp.quoteWho : NULL, U.opp.voice, w != Me(), U.opp.hash)))
            n += snprintf(line + n, sizeof(line) - (size_t)n, "\n");
    }
    Tt_PlayedIn(c, U.region);
    Tt_SkillResult(c, Tt_HandPower(U.dealt[0]) + Tt_PlayerRating(U.rules),
                   Tt_HandPower(U.dealt[1]) + Tt_LevelRating(U.opp.level, U.rules),
                   w == Me() ? 0 : w == 2 ? 2 : 1);
    if (w == Me())
    {
        c->wins++;
        MP_Achieve_Count("ttwin", 1);
        if (TtDuel_Score(&U.duel, Me()) >= 9)
        {
            c->perfects++;
            MP_Achieve_Unlock("ttperfect");
        }
        if (U.opp.special != TT_SPECIAL_NONE && U.opp.special < 32 && !(c->specialBeaten & (1u << U.opp.special)))
        {
            c->specialBeaten |= 1u << U.opp.special;
            MP_Achieve_Count("ttspecial", 1);
            MP_Achieve_Count("cardspecial", 1);   /* LEGENDS OF THE TABLE counts every card game */
        }
        if (U.opp.special == TT_SPECIAL_QUEEN)
        {
            c->flags |= TT_FLAG_QUEEN_FAVOR;
            MP_Achieve_Unlock("ttqueen");
        }
        // what the trade rule lets you take
        U.spoilFixed = 0;
        U.spoilPicked = 0;
        switch (U.trade)
        {
        case TT_TRADE_DIRECT:
            U.spoilFixed = (uint8_t)TtDuel_DirectMask(&U.duel, Me(), U.dealt[1]);
            U.spoilAllowed = __builtin_popcount(U.spoilFixed);
            break;
        case TT_TRADE_ALL:
            U.spoilFixed = 0x1F;
            U.spoilAllowed = TT_HAND;
            break;
        default:
            U.spoilAllowed = (int)TtDuel_SpoilCount(&U.duel, Me());
            break;
        }
        if (U.spoilAllowed)
            n += snprintf(line + n, sizeof(line) - (size_t)n, "TRADE RULE %s: take %d of %s's cards!",
                          Tt_TradeName(U.trade), U.spoilAllowed, U.opp.name);
        else
            n += snprintf(line + n, sizeof(line) - (size_t)n, "TRADE RULE DIRECT: none of their cards ended in "
                                                              "your color.");
    }
    else if (w == 2)
    {
        c->draws++;
        n += snprintf(line + n, sizeof(line) - (size_t)n, "A draw. No cards change hands.");
    }
    else
        c->losses++;
    TtBoard_Result(line);
}
static void CountRules(void)
{
    unsigned i;
    for (i = 0; i < U.duel.evCount; i++)
    {
        const struct TtEvent *e = &U.duel.ev[i];
        if (e->kind != TTE_RULE || U.duel.cell[e->cell].owner != Me())
            continue;
        if (e->by == TT_BY_SAME)
            C()->sames++, MP_Achieve_Count("ttsame", 1);
        else if (e->by == TT_BY_PLUS)
            C()->pluses++, MP_Achieve_Count("ttplus", 1);
        else if (e->by == TT_BY_COMBO)
            C()->combos++, MP_Achieve_Count("ttcombo", 1);
    }
}
static void Move(unsigned slot, unsigned cell)
{
    if (!TtDuel_Play(&U.duel, slot, cell))
        return;
    if (U.duel.evCount && U.duel.ev[0].seat == Me())
        CountRules();
    TtBoard_Show(&U.duel);
}
/* The game ends without a move (a concession): the table shows the end */
static void ShowEnd(void)
{
    U.duel.evCount = 1;
    memset(&U.duel.ev[0], 0, sizeof(U.duel.ev[0]));
    U.duel.ev[0].kind = TTE_END;
    U.duel.ev[0].cell = 0xFF;
    TtBoard_Show(&U.duel);
}
/* A friend's game: their moves as they come in, one at a time while the table is still (0: nothing to do) */
static int FriendStep(void)
{
    struct TtNet *l = Tt_Online();
    unsigned slot, cell;
    if (U.concedeHeld && !U.duel.over && TtNet_Concede(l, SDL_GetTicks()))
    {
        U.concedeHeld = 0;
        TtDuel_Concede(&U.duel, Me());
        ShowEnd();
        return 1;
    }
    if (U.duel.over || !TtBoard_Idle() || !TtNet_Take(l, &slot, &cell))
        return 0;
    if (slot == 254)
        return 1;   /* our own move, taken back: their concession crossed it (the end below shows the game as agreed) */
    if (slot == 255)
    {
        U.duel = l->game;
        U.duel.noEvents = 0;
        ShowEnd();
        return 1;
    }
    Move(slot, cell);
    return 1;
}
static void DuelStep(unsigned pressed, unsigned keys)
{
    unsigned slot, cell;
    int r = TtBoard_Step(pressed, keys, &slot, &cell);
    if (r == TB_MOVE)
    {
        if (U.online && !TtNet_Move(Tt_Online(), slot, cell, SDL_GetTicks()))
        {
            printf("[tt] move refused: %s\n", Tt_Online()->error);
            return;
        }
        Move(slot, cell);
        return;
    }
    if (r == TB_CONCEDE)
    {
        /* giving up: a loss, nothing taken (online, once our last move is acknowledged) */
        if (U.online && !TtNet_Concede(Tt_Online(), SDL_GetTicks()))
        {
            U.concedeHeld = 1;
            return;
        }
        TtDuel_Concede(&U.duel, Me());
        ShowEnd();
        return;
    }
    if (r == TB_DONE)
    {
        printf("[tt] left the table (%s %u-%u)\n", U.duel.winner == Me() ? "won" : U.duel.winner == 2 ? "draw" : "lost",
               TtDuel_Score(&U.duel, Me()), TtDuel_Score(&U.duel, Me() ^ 1));
        if (U.online)
        {
            TtNet_Cancel(Tt_Online(), "Game complete.");
            U.online = 0;
        }
        if (U.vs == VS_NPC && U.duel.winner == Me() && U.spoilAllowed)
        {
            TtBoard_EndHoldMusic();   // the victory song plays on while cards are taken
            Screen(T_SPOILS);
            U.cursor = 0;
            return;
        }
        TtBoard_End();
        Close();
        return;
    }
    if (U.duel.over && !U.rewarded)
        Results();
    if (U.online)
    {
        if (!FriendStep() && !U.duel.over)
            TtBoard_SetWaiting(U.duel.toMove != Me() || Tt_Online()->pending);
        if (sAutoPlay && !U.duel.over && TtBoard_Idle() && U.duel.toMove == Me() && !Tt_Online()->pending &&
            ++U.aiDelay > 10)
        {
            TtAi_Choose(&U.duel, 3, U.frame, &slot, &cell);
            U.aiDelay = 0;
            if (TtNet_Move(Tt_Online(), slot, cell, SDL_GetTicks()))
                Move(slot, cell);
        }
        return;
    }
    if (U.duel.over || !TtBoard_Idle())
        return;
    if (U.duel.toMove != Me() || sAutoPlay)
    {
        TtBoard_SetWaiting(1);
        if (++U.aiDelay > (Tcg_GetCollection()->fastAnimations ? 10 : 26))
        {
            unsigned level = U.duel.toMove == Me() ? 3 : U.opp.level;
            TtAi_Choose(&U.duel, level, U.frame, &slot, &cell);
            U.aiDelay = 0;
            TtBoard_SetWaiting(0);
            Move(slot, cell);
        }
    }
    else
        TtBoard_SetWaiting(0);
}

/* ------------------------------------------------------------------ */
/* Screens                                                              */
/* ------------------------------------------------------------------ */
static const char *const sHomeRows[] = {"DECKS", "CARD LIST", "CARD PACKS", "RULES OF HOENN", "PRACTICE GAME",
                                        "GAME SPEED", "CLOSE CASE"};
static const char *const sHomeHelp[] = {
    "Five cards make a hand. Pick the deck you play with.", "Every card, sorted by stars.",
    "Open the packs you bought or won.", "Each region plays its own rules, and rules travel with you.",
    "Against a hand from around here. No cards change hands.", "How quickly the cards move on the table.",
    "Put the CARD CASE away."};
#define HOME_ROWS 7
static unsigned PacksHeld(void)
{
    unsigned i, n = 0;
    for (i = 0; i < TT_PACK_TYPES; i++)
        n += C()->packs[i];
    return n;
}
static void HomeScreen(void)
{
    struct TtCollection *c = C();
    unsigned i, packs = PacksHeld(), total, have = Tt_OwnedCount(c, 0, &total), face = c->optedIn ? DeckFace(c->activeDeck) : 0;
    char line[160];
    /* the deck's card floats as the other two games' do: whole pixels over a still shadow */
    int bob = (int)(sinf(U.frame * 0.06f) * 1.5f);
    TcgUi_Background();
    TcgUi_CaseTabs(KIT_GAME_TRIAD, TT_THEME);
    if (face)
    {
        G_FillA(31 + 2, 22 + 3, TT_BOARD_W + 2, TT_BOARD_H + 2, 0x000000, 70);
        Card(face, 31, (float)(22 - bob), TT_BOARD_W + 2, TT_BOARD_H + 2, TT_SEAT_BLUE, 255);
    }
    G_TextAlign(52, 76, TA_CENTER, G_INK, G_INK_SHADOW, c->optedIn ? Deck(c->activeDeck)->name : "No cards", TF_SMALL);
    if (c->optedIn)
    {
        unsigned region = Tt_CurrentRegion();
        snprintf(line, sizeof(line), "%s RULES", Tt_RegionName(region));
        G_TextAlign(52, 88, TA_CENTER, 0x405060, G_NO_SHADOW, line, TF_NARROW);
        snprintf(line, sizeof(line), "WON %u  LOST %u", c->wins, c->losses);
        G_TextAlign(52, 97, TA_CENTER, 0x405060, G_NO_SHADOW, line, TF_NARROW);
    }
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < HOME_ROWS; i++)
    {
        const char *right = NULL;
        int enabled = c->optedIn || i >= 3;
        if (i == 2)
            snprintf(line, sizeof(line), "%u", packs), right = line;
        if (i == 5)
            right = Tcg_GetCollection()->fastAnimations ? "FAST" : "NORMAL";
        TcgUi_Row(LIST_X + 2, 6 + (int)i * 14, LIST_W - 4, U.cursor == (int)i, enabled && (i != 2 || packs),
                  sHomeRows[i], right);
    }
    if (!c->optedIn)
        Strip(0, "No TRIPLE TRIAD cards yet. A TRIPLE TRIAD seller in a POKeMON CENTER hands out a first hand!");
    else
    {
        snprintf(line, sizeof(line), "%s\nCARDS %u/%u", sHomeHelp[U.cursor], have, total);
        Strip(0, line);
    }
    TcgUi_Hints("{SELECT} HOW TO PLAY  {L}{R} OTHER GAMES");
}

/* DECKS: six slots of five cards */
enum { DM_EDIT, DM_RENAME, DM_USE, DM_COPY, DM_DELETE, DM_COUNT };
static const char *const sDeckMenu[DM_COUNT] = {"EDIT", "RENAME", "PLAY WITH IT", "COPY", "DELETE"};
static const char *const sEmptyMenu[1] = {"BUILD A DECK"};
#define DECK_ROWS (TT_DECKS + 1)
static void DecksScreen(void)
{
    struct TtCollection *c = C();
    unsigned i;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("DECKS", TT_THEME);
    if (U.cursor < TT_DECKS)
    {
        /* the slot's five cards, fanned */
        for (i = 0; i < TT_HAND; i++)
        {
            unsigned id = Deck((unsigned)U.cursor)->cards[i];
            float x = 6 + i * 17, y = 26 + (i % 2) * 4;
            if (id)
                Card(id, x, y, 30, 37, TT_SEAT_BLUE, 255);
            else
            {
                G_RoundFill((int)x, (int)y, 30, 37, 0x304050, 160);
                G_RoundFrame((int)x, (int)y, 30, 37, 0x80a0b8, 200);
            }
        }
    }
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < DECK_ROWS; i++)
    {
        if (i == TT_DECKS)
        {
            TcgUi_Row(LIST_X + 2, 6 + (int)i * 14, LIST_W - 4, U.cursor == (int)i, 1, "BACK", NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%u/5", DeckCount(i));
        TcgUi_Row(LIST_X + 2, 6 + (int)i * 14, LIST_W - 4, U.cursor == (int)i, DeckCount(i) > 0, Deck(i)->name,
                  i == c->activeDeck ? "IN USE" : line);
    }
    if (U.cursor < TT_DECKS)
    {
        char why[96] = "";
        if (DeckCount((unsigned)U.cursor) && !Tt_DeckValid(Deck((unsigned)U.cursor)->cards, why, sizeof(why)))
            ;
        else if (DeckCount((unsigned)U.cursor) && !DeckOk((unsigned)U.cursor))
            snprintf(why, sizeof(why), "Some of its cards aren't in your collection any more.");
        snprintf(line, sizeof(line), "%s", why[0] ? why : DeckCount((unsigned)U.cursor) ? "Ready to play." : "An empty slot.");
        Strip(0, line);
    }
    else
        Strip(0, "Back to the CARD CASE.");
    if (U.menuOn)
    {
        int empty = !DeckCount((unsigned)U.cursor);
        if (empty)
            TcgUi_PopMenu(118, 40, 114, sEmptyMenu, 1, 0, 0);
        else
            TcgUi_PopMenu(134, 20, 98, sDeckMenu, DM_COUNT, U.menuCursor, 0);
    }
    TcgUi_NameDraw(TT_THEME);
    TcgUi_Hints("{A} CHOOSE  {R} PLAY WITH IT  {B} BACK");
}

/* The builder: the deck's five slots along the top, the cards owned of a star below */
static unsigned BuilderList(uint16_t *out) { return Shown(out, 1); }
static void BuilderScreen(void)
{
    static uint16_t list[TT_CARD_MAX + 1];
    struct TtDeck *d = Deck((unsigned)U.deck);
    unsigned n = BuilderList(list), i, id;
    int page = GRID_COLS * GRID_ROWS, start = U.cursor / page * page;
    char line[160], why[96];
    TcgUi_Background();
    G_RoundFill(0, 0, 240, 56, 0x183040, 235);
    G_Text(4, 1, G_WHITE, 0x101820, d->name, 100, TF_NARROW);
    for (i = 0; i < TT_HAND; i++)
    {
        float x = 100 + i * 28, y = 3;
        int on = U.inDeckRow && U.deckSlot == (int)i, target = !U.inDeckRow && U.deckSlot == (int)i;
        if (d->cards[i])
            Card(d->cards[i], x, y - (on ? 2 : 0), 26, 32, TT_SEAT_BLUE, 255);
        else
        {
            G_RoundFill((int)x, (int)y, 26, 32, 0x304858, 200);
            G_RoundFrame((int)x, (int)y, 26, 32, 0x6890a8, 255);
        }
        if (on || target)
        {
            int o = (U.frame / 8) % 2;
            G_Frame((int)x - 2 - o, (int)y - 2 - o - (on ? 2 : 0), 30 + 2 * o, 36 + 2 * o, on ? 0xe82820 : 0xf8d048, 255);
        }
    }
    /* how the deck stands against the star limits */
    {
        unsigned four = 0, five = 0;
        for (i = 0; i < TT_HAND; i++)
            if (Tt_Card(d->cards[i]))
                four += Tt_Card(d->cards[i])->stars >= 4, five += Tt_Card(d->cards[i])->stars == 5;
        G_Text(4, 11, 0x90a8b8, G_NO_SHADOW, "5 DIFFERENT CARDS", 0, TF_NARROW);
        snprintf(line, sizeof(line), "4+ STARS %u/2", four);
        G_Text(4, 20, four > 2 ? 0xff8070 : 0xc8d8e8, G_NO_SHADOW, line, 0, TF_NARROW);
        snprintf(line, sizeof(line), "5 STARS %u/1", five);
        G_Text(4, 29, five > 1 ? 0xff8070 : 0xc8d8e8, G_NO_SHADOW, line, 0, TF_NARROW);
    }
    StarTabs(4, 40, (int)ShownStars());
    for (i = 0; i < (unsigned)page && start + (int)i < (int)n; i++)
    {
        int x = 6 + (int)(i % GRID_COLS) * 33, y = 58 + (int)(i / GRID_COLS) * 18, on = !U.inDeckRow && start + (int)i == U.cursor;
        unsigned card = list[start + (int)i], used = 0, k;
        for (k = 0; k < TT_HAND; k++)
            used |= d->cards[k] == card;
        Tt_DrawIcon(card, (float)x + 7, (float)y, 16, 255, used);
        if (used)
            G_Text(x + 6, y + 3, 0xf8d048, 0x101010, "IN", 0, TF_NARROW);
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x + 6 - o, y - 1 - o, 18 + 2 * o, 18 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n && TcgSearch_Narrowed(SEARCH_TRIAD))
        TcgSearch_Empty(SEARCH_TRIAD, 120, 68);
    else if (!n)
        G_TextAlign(120, 70, TA_CENTER, G_WHITE, 0x204030, "You own none of these yet.", TF_SMALL);
    id = U.inDeckRow ? d->cards[U.deckSlot] : n ? list[U.cursor] : 0;
    if (id)
    {
        Numbers(why, sizeof(why), id);
        snprintf(line, sizeof(line), "%s  %s\n%s%s", Tt_Card(id)->name, why, StarWord(Tt_Card(id)->stars),
                 Tt_Card(id)->type ? "" : "");
        Strip(id, line);
    }
    else
        Strip(0, U.inDeckRow ? "An empty slot: pick a card below." : "");
    TcgSearch_Badge(SEARCH_TRIAD, -236, 135, n);
    TcgUi_Hints(U.inDeckRow ? "{A} TAKE OUT  {INSPECT} LOOK  {DOWN} CARDS  {START} DONE"
                            : "{A} PUT IN  {INSPECT} LOOK  {SEARCH} SEARCH  {START} DONE");   /* ({L}{R}: by the tabs) */
}

static void ListScreen(void)
{
    static uint16_t list[TT_CARD_MAX + 1];
    struct TtCollection *c = C();
    unsigned n = BinderList(list), i, total, have = Tt_OwnedCount(c, (unsigned)U.stars, &total);
    int page = GRID_COLS * 4, start = U.cursor / page * page;
    char line[160], nums[16];
    TcgUi_Background();
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    StarTabs(2, 2, (int)ShownStars());
    snprintf(line, sizeof(line), "%u/%u", have, total);
    G_TextAlign(236, 2, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
    for (i = 0; i < (unsigned)page && start + (int)i < (int)n; i++)
    {
        unsigned card = list[start + (int)i];
        int x = 6 + (int)(i % GRID_COLS) * 33, y = 20 + (int)(i / GRID_COLS) * 23, on = start + (int)i == U.cursor;
        if (on)
            y -= 1;
        if (Owned(card))
            Tt_DrawIcon(card, (float)x + 5, (float)y, 20, 255, 0);
        else if (Tt_Seen(c, card))
            Tt_DrawIcon(card, (float)x + 5, (float)y, 20, 255, 1);
        else
        {
            G_RoundFill(x + 5, y, 20, 20, 0x30404c, 220);
            G_TextAlign(x + 15, y + 4, TA_CENTER, 0x8098a8, G_NO_SHADOW, "?", TF_NARROW);
        }
        if (Owned(card) > 1)
        {
            snprintf(line, sizeof(line), "x%u", Owned(card));
            G_RoundFill(x + 15, y + 13, G_DigitsWidth(line, 0, 1) + 4, 9, 0x1c2c48, 230);
            G_Digits(x + 17, y + 15, line, 0, 1, 0xe0f0ff, 255);
        }
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x + 4 - o, y - o, 22 + 2 * o, 22 + 2 * o, 0xe82820, 255);
        }
    }
    if (n)
    {
        unsigned id = list[U.cursor];
        if (Owned(id) || Tt_Seen(c, id))
        {
            Numbers(nums, sizeof(nums), id);
            snprintf(line, sizeof(line), "No. %u %s  %s\n%s", Tt_Card(id)->order, Tt_Card(id)->name, nums,
                     Owned(id) ? "" : "Seen on a table. Not in your collection yet.");
            Strip(Owned(id) ? id : 0, line);
        }
        else
        {
            snprintf(line, sizeof(line), "No. %u ???\nNot met yet.", Tt_Card(id)->order);
            Strip(0, line);
        }
    }
    else if (TcgSearch_Narrowed(SEARCH_TRIAD))
        TcgSearch_Empty(SEARCH_TRIAD, 120, 50);
    TcgSearch_Badge(SEARCH_TRIAD, -236, 135, n);
    TcgUi_Hints("{A} LOOK  {L}{R} STARS  {SEARCH} SEARCH  {B} BACK");
}

static void InspectScreen(void)
{
    const struct TtCard *card = Tt_Card((unsigned)U.card);
    char line[160];
    int y, bob = U.age < 12 ? (int)((12 - U.age) * 1.5f) : 0, lines;
    if (!card)
        return;
    TcgUi_Background();
    G_FillA(0, 0, 240, 160, 0x101828, 120);
    {
        struct TtFace face;
        float pulse = 0.5f + 0.5f * sinf(U.frame * 0.05f);
        memset(&face, 0, sizeof(face));
        face.seat = TT_SEAT_BLUE;
        G_FillA(12, 18 - bob, TT_BIG_W, TT_BIG_H, 0x000000, 70);
        Tt_DrawCard((unsigned)U.card, 8, (float)(12 - bob), TT_BIG_W, TT_BIG_H, 1, 1, 255, &face);
        if (card->stars >= 4)
        {
            /* a shine runs over the rarest cards now and then */
            int sx = (int)((U.frame * 2) % 300) - 40, k;
            for (k = 0; k < 10; k++)
                if (sx + k > 4 && sx + k < TT_BIG_W - 4)
                    G_FillA(8 + sx + k, 14 - bob, 1, TT_BIG_H - 4, 0xffffff, (unsigned)((20 + 6 * k) * (0.6f + 0.4f * pulse)));
        }
    }
    G_RoundFill(118, 4, 118, 140, 0x182838, 235);
    G_RoundFrame(118, 4, 118, 140, TT_GOLD, 255);
    y = G_Text(123, 8, G_WHITE, 0x101820, card->name, 108, TF_NARROW);
    Tt_DrawStars(card->stars, 123, y + 1, 9, 255);
    snprintf(line, sizeof(line), "No. %u", card->order);
    G_TextAlign(230, y + 1, TA_RIGHT, 0xa0b0c0, G_NO_SHADOW, line, TF_NARROW);
    y += 12;
    if (card->type)
    {
        Tt_DrawTypeIcon(card->type, 123, (float)y, 12, 255);
        G_Text(137, y, 0xd0e0f0, 0x101820, Tt_TypeName(card->type), 90, TF_NARROW);
        y += 13;
    }
    snprintf(line, sizeof(line), "OWNED %u  SELLS FOR \xa5%u", Owned((unsigned)U.card), card->sale);
    G_Text(123, y, 0xf8e0a0, G_NO_SHADOW, line, 108, TF_NARROW);
    y += 11;
    /* the description, scrolled a line at a time */
    lines = TcgUi_WrapLines(card->text, 108, TF_SMALL);
    {
        int bottom = 142, line9 = G_LineHeight(TF_SMALL), most = lines * line9 - (bottom - y);
        SDL_Rect clip = {120, y, 114, bottom - y};
        if (most < 0)
            most = 0;
        if (U.textScroll * line9 > most)
            U.textScroll = (most + line9 - 1) / line9;
        SDL_RenderSetClipRect(G_R(), &clip);
        G_Text(123, y - U.textScroll * line9, 0xc0c8d0, G_NO_SHADOW, card->text, 108, TF_SMALL);
        SDL_RenderSetClipRect(G_R(), NULL);
        if (most > 0)
            G_Arrow(226, 136, U.textScroll * line9 < most ? 1 : 0, 0xf8d048);
    }
    G_Hint(130, 150, G_WHITE, 0x101820, "{UP}{DOWN} READ  {B} BACK", TF_NARROW, 1);
}

static void PacksScreen(void)
{
    struct TtCollection *c = C();
    unsigned i;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("PACKS", TT_THEME);
    /* the chosen pack, its metal shining */
    {
        static const unsigned metal[TT_PACK_TYPES] = {0xb87838, 0xc0c8d0, 0xe8c048, 0x70d0c8, 0xd070e0};
        float t = U.frame * 0.05f;
        int p = U.cursor < TT_PACK_TYPES ? U.cursor : 0;
        G_RoundFill(30, 22, 44, 58, G_Mix(metal[p], 0x000000, 0.35f), 255);
        G_RoundFill(32, 24, 40, 54, metal[p], 255);
        G_FillA(32 + (int)((sinf(t) * 0.5f + 0.5f) * 34), 24, 6, 54, 0xffffff, 90);
        Tt_DrawStars((unsigned)p + 1, 52 - (int)(p + 1) * 4, 46, 9, 255);
        G_TextAlign(52, 30, TA_CENTER, 0x101010, G_NO_SHADOW, Tt_PackName((unsigned)p), TF_NARROW);
    }
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i <= TT_PACK_TYPES; i++)
    {
        if (i == TT_PACK_TYPES)
        {
            TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)i, 1, "BACK", NULL);
            continue;
        }
        snprintf(line, sizeof(line), "x%u", c->packs[i]);
        TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)i, c->packs[i] > 0, Tt_PackName(i), line);
    }
    if (U.cursor < TT_PACK_TYPES)
    {
        snprintf(line, sizeof(line), "%s PACK: three cards of %s.%s", Tt_PackName((unsigned)U.cursor),
                 StarWord((unsigned)U.cursor + 1), U.cursor >= TT_PACK_SOLD ? " Never sold: only won." : "");
        Strip(0, line);
    }
    else
        Strip(0, "");
    TcgUi_Hints("{A} OPEN  {B} BACK");
}
static void StartReveal(const uint16_t *cards, unsigned n, uint8_t fresh)
{
    unsigned i;
    memset(U.revealed, 0, sizeof(U.revealed));
    for (i = 0; i < n && i < TT_HAND; i++)
        U.revealed[i] = cards[i];
    U.revealCount = (int)n;
    U.revealNew = fresh;
    U.revealT = 0;
    Screen(T_REVEAL);
}
static void RevealScreen(void)
{
    int i, n = U.revealCount, w = n > 3 ? 40 : 56, h = n > 3 ? 49 : 69, gap = n > 3 ? 6 : 14;
    int x0 = 120 - (n * w + (n - 1) * gap) / 2;
    TcgUi_Background();
    G_FillA(0, 0, 240, 160, 0x081018, 150);
    for (i = 0; i < n; i++)
    {
        int at = 12 + i * 20, t = U.revealT - at;
        float x = (float)(x0 + i * (w + gap)), y = n > 3 ? 34 : 20, k;
        struct TtFace face;
        const struct TtCard *c = Tt_Card(U.revealed[i]);
        memset(&face, 0, sizeof(face));
        face.seat = TT_SEAT_BLUE;
        if (t < 0)
        {
            face.back = 1;
            Tt_DrawCard(U.revealed[i], x, y + sinf(U.frame * 0.1f + i) * 1.5f, (float)w, (float)h, 1, 1, 255, &face);
            continue;
        }
        k = t < 12 ? cosf(t / 12.0f * 3.14159f) : -1;
        face.back = t < 6;
        if (t >= 6 && t < 30)
            face.glow = (uint8_t)(200 - (t - 6) * 8);
        if (t >= 6 && c && c->stars >= 3)
        {
            float pulse = 0.5f + 0.5f * sinf(U.frame * 0.15f);
            G_RoundFill((int)x - 3, (int)y - 3, w + 6, h + 6, c->stars >= 5 ? 0xf070e0 : c->stars == 4 ? 0x70e0d0 : 0xf8d048,
                        (unsigned)(60 + 60 * pulse));
        }
        Tt_DrawCard(U.revealed[i], x, y, (float)w, (float)h, fabsf(k), 1, 255, &face);
        if (t >= 8 && c)
        {
            Tt_DrawStars(c->stars, (int)x + w / 2 - (int)c->stars * 4 + 1, (int)y + h + 3, 9, 255);
            if (U.revealNew >> i & 1)
                Tt_Text(x + w / 2 - Tt_TextWidth("NEW!", 1) / 2, y - 10 - (t < 20 ? (20 - t) * 0.5f : 0), "NEW!", 1,
                        0xf8e048, 0x302008, 255);
        }
    }
    {
        int all = U.revealT > 12 + (n - 1) * 20 + 14;
        Strip(0, all ? "Into your CARD CASE!" : "");
        TcgUi_Hints(all ? "{A} OK" : "{A} NEXT  {B} ALL");
    }
}

/* RULES OF HOENN: each region's rules, where the CARD QUEEN is, the rules you carry */
static void RulesScreen(void)
{
    struct TtCollection *c = C();
    unsigned here = Tt_CurrentRegion(), r, k;
    int y, x;
    char line[200];
    TcgUi_Background();
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    G_Text(6, 1, G_WHITE, 0x101820, "RULES OF HOENN", 0, TF_NORMAL);
    if (c->carriedFrom < TT_REGIONS)
        snprintf(line, sizeof(line), "CARRYING %s'S", Tt_RegionName(c->carriedFrom));
    else
        snprintf(line, sizeof(line), "CARRYING NONE YET");
    G_TextAlign(236, 3, TA_RIGHT, 0xf8d878, G_NO_SHADOW, line, TF_NARROW);
    /* the regions down the left */
    for (r = 0; r < TT_REGIONS; r++)
    {
        int on = U.cursor == (int)r;
        y = 19 + (int)r * 11;
        G_RoundFill(2, y, 94, 10, on ? TT_THEME : 0x203448, on ? 255 : 200);
        if (on)
            G_RoundFrame(2, y, 94, 10, TT_GOLD, 255);
        G_Text(6, y, r == here ? 0xf8e070 : G_WHITE, G_NO_SHADOW, Tt_RegionName(r), 0, TF_NARROW);
        if (c->queenRegion == r)
            Tt_DrawStar(84, (float)y, 9, 255);
    }
    /* the chosen one's rules */
    r = (unsigned)U.cursor;
    G_RoundFill(100, 19, 136, 89, 0x182838, 235);
    G_RoundFrame(100, 19, 136, 89, TT_GOLD, 255);
    G_Text(106, 22, G_WHITE, 0x101820, Tt_RegionName(r), 0, TF_NORMAL);
    y = 38;
    x = 106;
    for (k = 0; k < TT_RULE_COUNT; k++)
    {
        int w, lean;
        unsigned fill;
        if (!(c->regionRules[r] >> k & 1))
            continue;
        w = G_Measure(Tt_RuleName(k), TF_NARROW) + 8;
        if (x + w > 230)
        {
            x = 106;
            y += 14;
        }
        lean = Tt_RuleLean(k);
        fill = lean > 0 ? 0x2c6898 : lean < 0 ? 0x985830 : 0x5c5c78;
        G_RoundFill(x, y, w, 12, fill, 255);
        G_TextAlign(x + w / 2, y - 1, TA_CENTER, G_WHITE, G_NO_SHADOW, Tt_RuleName(k), TF_NARROW);
        if (!(c->carried >> k & 1) && c->carriedFrom < TT_REGIONS && c->carriedFrom != r)
            G_FillA(x + 1, y + 10, w - 2, 1, 0xff8070, 255);   /* (could be abolished: you don't carry it) */
        x += w + 2;
    }
    if (!c->regionRules[r])
        G_Text(106, y, 0xa0b0c0, G_NO_SHADOW, "No special rules", 0, TF_NARROW);
    y += 18;
    snprintf(line, sizeof(line), "TRADE RULE: %s", Tt_TradeName(c->regionTrade[r]));
    G_Text(106, y, 0xf8e0a0, G_NO_SHADOW, line, 126, TF_NARROW);
    y += 10;
    snprintf(line, sizeof(line), "GAMES PLAYED HERE: %u", c->regionGames[r]);
    G_Text(106, y, 0xa0b0c0, G_NO_SHADOW, line, 126, TF_NARROW);
    y += 10;
    if (c->queenRegion == r)
        G_Text(106, y, 0xf0a0f0, G_NO_SHADOW, "The CARD QUEEN is here.", 126, TF_NARROW);
    /* what could happen: what you carry that they lack */
    {
        uint32_t spread = c->carried & ~c->regionRules[r];
        int n = 0;
        line[0] = 0;
        if (c->carriedFrom < TT_REGIONS && c->carriedFrom != r)
        {
            for (k = 0; k < TT_RULE_COUNT; k++)
                if (spread >> k & 1)
                    n += snprintf(line + n, sizeof(line) - (size_t)n, "%s%s", n ? ", " : "Could spread here: ", Tt_RuleName(k));
            if (!n)
                snprintf(line, sizeof(line), "Nothing new to spread. Underlined rules could be abolished.");
        }
        else
            snprintf(line, sizeof(line), "Play somewhere, and you carry its rules on to the next region you play "
                                         "in.");
        Strip(0, line);
    }
    TcgUi_Hints("{UP}{DOWN} REGION  {B} BACK");
}

static void ChallengeScreen(void)
{
    struct TtCollection *c = C();
    char line[200];
    unsigned i;
    int y, random = (U.rules & TT_RULE_BIT(TT_RULE_RANDOM)) != 0, friend = U.vs == VS_FRIEND;
    /* a friend's rules: the asker's region's; the one asked sees them when the asker's hand comes in */
    int rulesKnown = !friend || Tcg_Social()->seat == 0 || Tt_Online()->state >= TT_NET_INVITED;
    if (friend)
        random = 0;   /* (a friend's game: always the deck pick; RANDOM deals from the whole collection after) */
    TcgUi_Background();
    TcgUi_SidePanel(friend ? "GAME" : "CHALLENGE", U.opp.special != TT_SPECIAL_NONE ? 0x9a6a20 : TT_THEME);
    G_RoundFill(6, 20, 92, 46, 0x1c3c4c, 255);
    G_RoundFrame(6, 20, 92, 46, 0x0c1820, 255);
    Portrait(28, 62);
    /* their hand, face down (or face up under OPEN) */
    for (i = 0; i < TT_HAND; i++)
    {
        struct TtFace face;
        memset(&face, 0, sizeof(face));
        face.seat = TT_SEAT_RED;
        face.back = friend || !(U.rules & TT_RULE_BIT(TT_RULE_OPEN));
        /* five 22x27 cards 6 apart: x 50-96, inside the box (6-98) */
        Tt_DrawCard(friend ? 1 : U.opp.hand[i], 50 + i * 6, 27 + (i % 2) * 3, 22, 27, 1, 1, 255, &face);
    }
    y = 68;
    if (U.opp.title[0])
    {
        G_Text(6, y, 0x505868, G_NO_SHADOW, U.opp.title, 96, TF_NARROW);
        y += 9;
    }
    G_Text(6, y, G_INK, G_INK_SHADOW, U.opp.name, 96, TF_SMALL);
    y += 12;
    /* how the game looks with the deck under the cursor (the region's name goes with its rules in the strip) */
    if (U.vs == VS_NPC && c->optedIn &&
        (random || DeckOk(random || U.cursor >= TT_DECKS || !DeckOk((unsigned)U.cursor) ? c->activeDeck : (unsigned)U.cursor)))
    {
        unsigned d = random || U.cursor >= TT_DECKS || !DeckOk((unsigned)U.cursor) ? c->activeDeck : (unsigned)U.cursor;
        int cards;
        unsigned lv, chance = MatchFor(d, &cards, &lv);
        TcgUi_Match(6, y + 1, 92, chance, cards, (int)lv, 2);
    }
    else
    {
        if (friend && Tcg_Social()->seat == 1)
            snprintf(line, sizeof(line), "THEIR RULES");
        else
            snprintf(line, sizeof(line), "%s RULES", Tt_RegionName(U.region));
        G_Text(6, y + 1, 0x505868, G_NO_SHADOW, line, 96, TF_NARROW);
    }
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    G_Text(LIST_X + 6, 5, 0x606878, G_NO_SHADOW, random ? "RANDOM: A HAND IS DEALT" : "PLAY WITH", 0, TF_NARROW);
    if (random)
    {
        TcgUi_Row(LIST_X + 2, 18, LIST_W - 4, U.cursor == 0, 1, "PLAY", NULL);
        TcgUi_Row(LIST_X + 2, 18 + ROW_H, LIST_W - 4, U.cursor == 1, 1, "NOT NOW", NULL);
    }
    else
        for (i = 0; i <= TT_DECKS; i++)
        {
            if (i == TT_DECKS)
            {
                TcgUi_Row(LIST_X + 2, 16 + (int)i * 13, LIST_W - 4, U.cursor == (int)i, 1, friend ? "LEAVE" : "NOT NOW",
                          NULL);
                continue;
            }
            TcgUi_Row(LIST_X + 2, 16 + (int)i * 13, LIST_W - 4, U.cursor == (int)i, DeckOk(i), Deck(i)->name,
                      i == c->activeDeck ? "IN USE" : NULL);
        }
    if (!rulesKnown)
        snprintf(line, sizeof(line), "%s plays by the rules where they are: they come with their hand.", U.opp.name);
    else
    {
        int n = friend ? 0 : snprintf(line, sizeof(line), "%s: ", Tt_RegionName(U.region));
        RulesLine(line + n, sizeof(line) - (size_t)n, U.rules, friend ? NO_TRADE : U.trade);
    }
    Strip(0, line);
    TcgUi_Hints(friend ? "{A} PLAY  {B} LEAVE" : "{A} PLAY  {B} NOT NOW");
}

static void SpoilsScreen(void)
{
    int i, picked = __builtin_popcount(U.spoilPicked | U.spoilFixed);
    char line[160], nums[16];
    TcgUi_Background();
    snprintf(line, sizeof(line), "%s'S HAND", U.opp.name);
    TcgUi_TopBar(line);
    for (i = 0; i < TT_HAND; i++)
    {
        int x = 10 + i * 45, y = 32, on = i == U.cursor, take = (U.spoilPicked | U.spoilFixed) >> i & 1;
        unsigned id = U.dealt[1][i];
        if (on)
            y -= 4;
        if (take)
        {
            float pulse = 0.5f + 0.5f * sinf(U.frame * 0.15f + i);
            G_RoundFill(x - 3, y - 3, TT_BOARD_W + 6, TT_BOARD_H + 6, 0xf8d048, (unsigned)(90 + 80 * pulse));
        }
        Card(id, (float)x, (float)y, TT_BOARD_W, TT_BOARD_H, TT_SEAT_RED, U.spoilFixed && !take ? 110 : 255);
        if (!Owned(id))
            Tt_Text((float)x + 20 - Tt_TextWidth("NEW", 1) / 2, (float)y + TT_BOARD_H + 3, "NEW", 1, 0xf8e048, 0x302008, 255);
        if (on)
        {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, TT_BOARD_W + 4 + 2 * o, TT_BOARD_H + 4 + 2 * o, 0xe82820, 255);
        }
    }
    {
        unsigned id = U.dealt[1][U.cursor];
        Numbers(nums, sizeof(nums), id);
        if (U.spoilFixed)
            snprintf(line, sizeof(line), "TRADE RULE %s: these %d are yours.\n%s  %s", Tt_TradeName(U.trade),
                     U.spoilAllowed, Tt_Card(id) ? Tt_Card(id)->name : "", nums);
        else
            snprintf(line, sizeof(line), "Pick %d (%d picked). You own %u.\n%s  %s", U.spoilAllowed, picked,
                     Owned(id), Tt_Card(id) ? Tt_Card(id)->name : "", nums);
        Strip(0, line);
    }
    TcgUi_Hints(U.spoilFixed ? "{A} TAKE THEM  {INSPECT} LOOK" : "{A} PICK  {START} TAKE  {INSPECT} LOOK");
}

/* A seller's counter: SINGLES (a curated few, one copy each), PACKS by stars (the highest never sold), SELL spares */
static const char *const sShopTabs[] = {"SINGLES", "PACKS", "SELL"};
#define SHOP_TABS 3
static unsigned ShopList(uint16_t *out)
{
    const uint16_t *stock = gTtSellerStock[U.shopRegion < TT_REGIONS ? U.shopRegion : 0];
    unsigned n = 0, i;
    if (U.shopTab == 0)
        for (i = 0; stock[i]; i++)
            out[n++] = stock[i];
    else if (U.shopTab == 2)
    {
        unsigned id;
        for (id = 1; id <= TT_CARD_MAX; id++)
            if (Owned(id) > 1)
                out[n++] = (uint16_t)id;
    }
    return n;
}
static int Bought(unsigned id) { return id && id <= TT_CARD_MAX && (C()->bought[id / 8] >> (id % 8) & 1); }
static void ShopScreen(void)
{
    static uint16_t list[TT_CARD_MAX + 1];
    char line[160], money[24], nums[16];
    unsigned n = ShopList(list), i;
    snprintf(money, sizeof(money), "\xa5%u", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
    TcgUi_Background();
    TcgUi_ShopTabs(sShopTabs, SHOP_TABS, U.shopTab, TT_THEME, 0x0c2830, G_WHITE, money, "MONEY", TT_THEME);
    if (U.shopTab == 1)
    {
        TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
        for (i = 0; i < TT_PACK_TYPES; i++)
        {
            unsigned price = Tt_PackPrice(i);
            if (price)
                snprintf(line, sizeof(line), "\xa5%u", price);
            else
                snprintf(line, sizeof(line), "---");
            TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)i,
                      price && IsEnoughMoney(&gSaveBlock1Ptr->money, price), Tt_PackName(i), line);
        }
        if (U.cursor >= TT_PACK_SOLD)
            snprintf(line, sizeof(line), "%s PACK: three cards of %s. Not for sale: the strongest packs are only "
                                         "ever won.", Tt_PackName((unsigned)U.cursor), StarWord((unsigned)U.cursor + 1));
        else
            snprintf(line, sizeof(line), "%s PACK: three cards of %s.", Tt_PackName((unsigned)U.cursor),
                     StarWord((unsigned)U.cursor + 1));
        Strip(0, line);
        TcgUi_Hints("{A} BUY  {L}{R} COUNTER  {B} LEAVE");
        return;
    }
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    {
        int rows = 6, start = U.cursor < rows / 2 ? 0 : U.cursor - rows / 2;
        if (start > (int)n - rows)
            start = (int)n - rows > 0 ? (int)n - rows : 0;
        for (i = 0; i < (unsigned)rows && start + (int)i < (int)n; i++)
        {
            unsigned id = list[start + (int)i];
            int y = 8 + (int)i * ROW_H;
            if (U.shopTab == 0)
                snprintf(line, sizeof(line), Bought(id) ? "SOLD" : "\xa5%u", Tt_SinglePrice(id));
            else
                snprintf(line, sizeof(line), "x%u", Owned(id) - 1);
            TcgUi_Row(LIST_X + 2, y, LIST_W - 4, U.cursor == start + (int)i, U.shopTab != 0 || !Bought(id),
                      Tt_Card(id)->name, line);
        }
        if (n > (unsigned)rows)
        {
            if (start > 0)
                G_Arrow(LIST_X + LIST_W - 10, 2, 0, 0x404850);
            if (start + rows < (int)n)
                G_Arrow(LIST_X + LIST_W - 10, 104, 1, 0x404850);
        }
    }
    if (n)
    {
        unsigned id = list[U.cursor < (int)n ? U.cursor : 0];
        Numbers(nums, sizeof(nums), id);
        if (U.shopTab == 0)
            snprintf(line, sizeof(line), "%s  %s  %s\n%s You own %u.", Tt_Card(id)->name, nums,
                     StarWord(Tt_Card(id)->stars), Bought(id) ? "SOLD: one per customer." : "", Owned(id));
        else
            snprintf(line, sizeof(line), "%s  %s\nA spare sells for \xa5%u.", Tt_Card(id)->name, nums,
                     Tt_Card(id)->sale);
        Strip(id, line);
    }
    else
        Strip(0, U.shopTab == 2 ? "No spare copies to sell. (Your first copy of a card is never sold.)" : "");
    TcgUi_Hints(U.shopTab == 0 ? "{A} BUY  {INSPECT} LOOK  {L}{R} COUNTER" : "{A} SELL ONE  {INSPECT} LOOK  {L}{R} COUNTER");
}

static void StarterScreen(void)
{
    TcgUi_Background();
    G_FillA(0, 0, 240, 160, 0x081018, 140);
    RevealScreen();
}

/* The CARD QUEEN */
enum { Q_PLAY, Q_TRAVEL, Q_TRADE, Q_ABOLISH, Q_LEAVE, Q_COUNT };
static const char *const sQueenRows[Q_COUNT] = {"PLAY A GAME", "SEND HER AWAY", "CHANGE TRADE RULE", "ABOLISH A RULE",
                                                "GOODBYE"};
static unsigned QueenOff(void)
{
    unsigned off = 0;
    if (!(C()->flags & TT_FLAG_QUEEN_FAVOR))
        off |= 1 << Q_TRADE | 1 << Q_ABOLISH;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, QUEEN_FARE))
        off |= 1 << Q_TRAVEL;
    if (!DeckOk(C()->activeDeck))
        off |= 1 << Q_PLAY;
    return off;
}
static void QueenScreen(void)
{
    struct TtCollection *c = C();
    char line[200];
    unsigned i, off = QueenOff();
    TcgUi_Background();
    TcgUi_SidePanel("CARD QUEEN", 0x9a3a8a);
    Portrait(52, 70);
    G_TextAlign(52, 76, TA_CENTER, G_INK, G_INK_SHADOW, "CARD QUEEN", TF_SMALL);
    snprintf(line, sizeof(line), "IN %s", Tt_RegionName(c->queenRegion));
    G_TextAlign(52, 90, TA_CENTER, 0x505868, G_NO_SHADOW, line, TF_NARROW);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < Q_COUNT; i++)
        TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, U.cursor == (int)i, !(off >> i & 1), sQueenRows[i],
                  i == Q_TRAVEL ? "\xa5" "5000" : NULL);
    switch (U.cursor)
    {
    case Q_PLAY: Strip(0, "A game by this region's rules. Wherever I am, rules spread and fall away more easily."); break;
    case Q_TRAVEL: Strip(0, "Pay my way and I'll go where you like. I may bring a rule from a far-off land."); break;
    case Q_TRADE: Strip(0, (off >> Q_TRADE & 1) ? "Beat me first, and I'll change how cards are traded here." : "You beat me: choose how cards are traded here."); break;
    case Q_ABOLISH: Strip(0, (off >> Q_ABOLISH & 1) ? "Beat me first, and I'll see a rule of this region gone." : "You beat me: choose a rule this region will forget."); break;
    default: Strip(0, "Until we meet again."); break;
    }
    if (U.menuOn)
    {
        /* the choices of a favor or a journey */
        static const char *items[TT_RULE_COUNT + 1];
        int n = 0;
        if (U.menuOn == Q_TRADE)
            for (i = 0; i < TT_TRADE_COUNT; i++)
                items[n++] = Tt_TradeName(i);
        else if (U.menuOn == Q_ABOLISH)
        {
            for (i = 0; i < TT_RULE_COUNT; i++)
                if (c->regionRules[c->queenRegion] >> i & 1)
                    items[n++] = Tt_RuleName(i);
        }
        else
            for (i = 0; i < TT_REGIONS; i++)
                items[n++] = Tt_RegionName(i);
        if (n)
            TcgUi_PopMenu(120, 2, 112, items, n, U.menuCursor, 0);
    }
}

/* ------------------------------------------------------------------ */
/* With a friend                                                        */
/* ------------------------------------------------------------------ */
/* The hand we bring: the deck in use, or under RANDOM five dealt from the whole collection */
static int FriendHand(uint16_t *hand)
{
    struct TtCollection *c = C();
    memcpy(hand, Deck(c->activeDeck)->cards, sizeof(uint16_t) * TT_HAND);
    if (U.rules & TT_RULE_BIT(TT_RULE_RANDOM))
        return Tt_DealRandom(c->owned, Tt_Rand(&c->rng), hand);
    return 1;
}
static void StartFriendGame(void)
{
    struct TtNet *l = Tt_Online();
    struct TtCollection *c = C();
    unsigned me = l->seat, i;
    U.online = 1;
    U.duel = l->game;
    U.duel.noEvents = 0;
    U.duel.evCount = 0;
    U.rules = l->rules;
    memcpy(U.dealt[0], l->hand[me], sizeof(U.dealt[0]));
    memcpy(U.dealt[1], l->hand[me ^ 1], sizeof(U.dealt[1]));
    memcpy(U.opp.hand, l->hand[me ^ 1], sizeof(U.opp.hand));
    for (i = 0; i < TT_HAND; i++)
        Tt_See(c, U.dealt[1][i]);
    U.rewarded = 0;
    U.outcome = -1;
    U.aiDelay = 0;
    U.concedeHeld = 0;
    TtBoard_SetFast(Tcg_GetCollection()->fastAnimations);
    TtBoard_Lesson(0);
    Screen(T_DUEL);
    TtBoard_Begin(&U.duel, me, MyName(), U.opp.name, U.opp.music, 1);
    printf("[tt] game vs %s (online, seat %u, rules %x)\n", U.opp.name, me, U.rules);
}
static void OnlineStep(void)
{
    struct TtNet *l = Tt_Online();
    const struct TcgSocial *s = Tcg_Social();
    if (U.vs != VS_FRIEND || U.screen == T_TRADE || U.screen == T_CLOSED)
        return;
    if (s->peerLeft && U.screen != T_DUEL && !U.message[0])
    {
        Say(ACT_CLOSE, "%s left.", U.opp.name);
        return;
    }
    /* The friend who said yes: the asker's rules come with their hand, and ours goes back once a deck is picked */
    if (s->seat == 1 && l->state == TT_NET_INVITED && l->peer == s->peer && l->session == s->session)
    {
        U.rules = l->rules;
        if (U.onlineDeckChosen)
        {
            uint16_t hand[TT_HAND];
            if (!FriendHand(hand))
            {
                TtNet_Cancel(l, "RANDOM couldn't deal a hand.");
                Say(ACT_CLOSE, "RANDOM deals five of your own cards, and you own fewer than five.");
                return;
            }
            TtNet_Accept(l, hand, SDL_GetTicks());
        }
    }
    if (U.screen == T_WAIT && l->state == TT_NET_PLAYING)
        StartFriendGame();
    if (U.online && l->state == TT_NET_ENDED && !l->game.over && !U.message[0])
    {
        U.online = 0;
        if (U.screen == T_DUEL)
            TtBoard_End();
        Screen(T_WAIT);
        Say(ACT_CLOSE, "%s", l->error[0] ? l->error : "The game ended.");
        printf("[tt] online game ended: %s\n", l->error);
    }
}
static void PickFriendDeck(void)
{
    const struct TcgSocial *s = Tcg_Social();
    struct TtCollection *c = C();
    if (!DeckOk((unsigned)U.cursor))
    {
        Say(ACT_NONE, "That deck isn't ready.");
        return;
    }
    c->activeDeck = (uint8_t)U.cursor;
    U.onlineDeckChosen = 1;
    if (s->seat == 0)
    {
        uint16_t hand[TT_HAND];
        if (!FriendHand(hand))
        {
            Say(ACT_NONE, "RANDOM deals five of your own cards, and you own fewer than five.");
            return;
        }
        if (!TtNet_Invite(Tt_Online(), s->peer, s->session, U.rules, hand, SDL_GetTicks()))
        {
            Say(ACT_CLOSE, "The game couldn't start.");
            return;
        }
    }
    U.online = 1;
    Screen(T_WAIT);
}
static void WaitScreen(void)
{
    char line[96];
    int dots = (U.frame / 20) % 4;
    struct TtFace face;
    TcgUi_Background();
    TcgUi_SidePanel("GAME", TT_THEME);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    snprintf(line, sizeof(line), "Waiting for %s%.*s", U.opp.name, dots, "...");
    G_Text(LIST_X + 8, 20, G_INK, G_INK_SHADOW, line, LIST_W - 14, TF_SMALL);
    memset(&face, 0, sizeof(face));
    face.seat = TT_SEAT_RED;
    face.back = 1;
    /* a card turning over and over */
    {
        float turn = (float)((U.frame % 90) / 90.0 * 6.2831853);
        float w = 30.0f * (float)SDL_fabs(SDL_cos(turn));
        Tt_DrawCard(1, LIST_X + LIST_W / 2 - w / 2, 44.0f, w < 1 ? 1 : w, 37, 1, 1, 255, &face);
    }
    Strip(0, "They're choosing their hand.");
    TcgUi_Hints("{B} LEAVE");
}
/* Trading: copies past the one the deck in use needs */
static int Tradable(unsigned id)
{
    const struct TtDeck *d = Deck(C()->activeDeck);
    unsigned i, inDeck = 0;
    if (!Tt_Card(id) || Tt_Card(id)->stars != (unsigned)U.stars)
        return 0;
    for (i = 0; i < TT_HAND; i++)
        inDeck |= d->cards[i] == id;
    return Owned(id) > inDeck;
}
static unsigned TradeList(uint16_t *out)
{
    uint16_t all[TT_CARD_MAX + 1];
    unsigned n = StarList((unsigned)U.stars, all, 1), i, k = 0;
    for (i = 0; i < n; i++)
        if (Tradable(all[i]))
            out[k++] = all[i];
    return k;
}
static void TradeScreen(void)
{
    static uint16_t list[TT_CARD_MAX + 1];
    const struct TcgSocial *s = Tcg_Social();
    unsigned mine = s->myOffer, theirs = Tt_Card(s->theirOffer) ? s->theirOffer : 0;
    int n = (int)TradeList(list), i, page = 7, start;
    char line[160];
    TcgUi_Background();
    snprintf(line, sizeof(line), "TRIPLE TRIAD TRADE WITH %s", U.opp.name);
    TcgUi_TopBar(line);
    TcgUi_ListBox(4, 20, 112, 62);
    TcgUi_ListBox(124, 20, 112, 62);
    G_Text(10, 22, 0x606878, G_NO_SHADOW, "YOU OFFER", 0, TF_NARROW);
    G_Text(130, 22, 0x606878, G_NO_SHADOW, "THEY OFFER", 0, TF_NARROW);
    if (mine)
    {
        Card(mine, 10, 32, 36, 45, TT_SEAT_BLUE, 255);
        G_Text(50, 36, G_INK, G_INK_SHADOW, Tt_Card(mine)->name, 64, TF_SMALL);
        Tt_DrawStars(Tt_Card(mine)->stars, 50, 50, 9, 255);
        if (s->myConfirm)
            G_Text(50, 64, 0x2858a8, G_NO_SHADOW, "READY!", 0, TF_SMALL);
    }
    if (theirs)
    {
        Card(theirs, 130, 32, 36, 45, TT_SEAT_RED, 255);
        G_Text(170, 36, G_INK, G_INK_SHADOW, Tt_Card(theirs)->name, 64, TF_SMALL);
        Tt_DrawStars(Tt_Card(theirs)->stars, 170, 50, 9, 255);
        if (s->theirConfirm)
            G_Text(170, 64, 0x2858a8, G_NO_SHADOW, "READY!", 0, TF_SMALL);
    }
    else
        G_Text(170, 46, 0x909090, G_NO_SHADOW, "Choosing...", 0, TF_SMALL);
    G_RoundFill(0, 84, 240, 15, 0x203040, 230);
    StarTabs(4, 85, U.stars);
    start = U.cursor / page * page;
    for (i = 0; i < page && start + i < n; i++)
    {
        int x = 8 + i * 33, on = start + i == U.cursor;
        Card(list[start + i], (float)x, (float)(103 - (on ? 2 : 0)), 30, 37, TT_SEAT_BLUE, 255);
        if (on)
            G_Frame(x - 2, 99, 34, 41, 0xe82820, 255);
    }
    if (!n)
        G_Text(10, 108, G_WHITE, 0x204030, "No spare cards of these stars to offer.", 0, TF_SMALL);
    TcgUi_Hints(s->myOffer && s->theirOffer ? (s->myConfirm ? "WAITING FOR THEM  {B} LEAVE" : "{A} OFFER  {START} TRADE!  {B} LEAVE")
                                            : "{A} OFFER  {L}{R} STARS  {B} LEAVE");
}
static void TradeStep(unsigned pressed)
{
    static uint16_t list[TT_CARD_MAX + 1];
    const struct TcgSocial *s = Tcg_Social();
    struct TtCollection *c = C();
    int n = (int)TradeList(list);
    /* Both confirmed: the trade is done, even when the friend's leaving came in with their confirm */
    if (s->myConfirm && s->theirConfirm && s->myOffer && s->theirOffer)
    {
        unsigned mine = s->myOffer, theirs = s->theirOffer;
        int fresh;
        if (!Tt_Card(theirs) || mine > TT_CARD_MAX || !c->owned[mine])
        {
            Tcg_SocialLeave();
            Say(ACT_CLOSE, "That trade couldn't be made.");
            return;
        }
        c->owned[mine]--;
        fresh = Tt_Give(c, theirs);
        c->trades++;
        MP_Achieve_Unlock("cardtrade");
        if (Tt_Card(theirs)->stars == 5)
            MP_Achieve_Unlock("tt5star");
        if (Tt_OwnedCount(c, 0, NULL) >= TT_CARD_COUNT)
            MP_Achieve_Unlock("ttall");
        printf("[tt] traded card %u for %u\n", mine, theirs);
        Tcg_SocialLeave();
        PlayFanfare(MUS_OBTAIN_ITEM);
        Say(ACT_CLOSE, "You traded your %s for %s!%s", Tt_Card(mine)->name, Tt_Card(theirs)->name,
            fresh ? " New to your collection!" : "");
        return;
    }
    if (s->peerLeft)
    {
        Say(ACT_CLOSE, "%s left the trade.", U.opp.name);
        Tcg_SocialLeave();
        return;
    }
    if (pressed & (KEY_L | KEY_R))
    {
        U.stars = (U.stars + (pressed & KEY_R ? 0 : TT_MAX_STARS - 2)) % TT_MAX_STARS + 1;
        U.cursor = 0;
        PlaySE(SE_SELECT);
        return;
    }
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
        Tcg_SocialOffer(list[U.cursor]);
        PlaySE(SE_RG_CARD_FLIPPING);
    }
    else if (pressed & KEY_START && s->myOffer && s->theirOffer && !s->myConfirm)
    {
        Tcg_SocialConfirm();
        PlaySE(SE_SELECT);
    }
    else if (pressed & KEY_B)
    {
        PlaySE(SE_SELECT);
        Close();
    }
}

void TtUi_Draw(void)
{
    switch (U.screen)
    {
    case T_HOME: TcgUi_RowDown(1), HomeScreen(), TcgUi_RowDown(0); break;   // (the row cursor 1 px lower)
    case T_DECKS: TcgUi_RowDown(1), DecksScreen(), TcgUi_RowDown(0); break;   // (the row cursor 1 px lower)
    case T_BUILDER: BuilderScreen(); break;
    case T_LIST: ListScreen(); break;
    case T_INSPECT: InspectScreen(); break;
    case T_PACKS: TcgUi_RowDown(1), PacksScreen(), TcgUi_RowDown(0); break;   // (the row cursor 1 px lower)
    case T_REVEAL: RevealScreen(); break;
    case T_RULES: RulesScreen(); break;
    case T_CHALLENGE: ChallengeScreen(); break;
    case T_DUEL: TtBoard_Render(); break;
    case T_SPOILS: SpoilsScreen(); break;
    case T_SHOP: ShopScreen(); break;
    case T_STARTER: StarterScreen(); break;
    case T_QUEEN: QueenScreen(); break;
    case T_WAIT: WaitScreen(); break;
    case T_TRADE: TradeScreen(); break;
    default: break;
    }
    if (TcgSearch_Active() && Searchable() && !TcgGuide_Active())
    {
        static uint16_t list[TT_CARD_MAX + 1];
        TcgSearch_Draw(Shown(list, U.screen == T_BUILDER));
    }
    TcgUi_MessageBox(U.message, U.asking, U.askYes);
}

/* ------------------------------------------------------------------ */
/* Opening from the map                                                 */
/* ------------------------------------------------------------------ */
void TtUi_OpenChallenge(const struct TtOpponent *opponent)
{
    struct TtCollection *c = C();
    struct TtMix mix;
    unsigned region;
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.opp = *opponent;
    U.vs = VS_NPC;
    region = opponent->region < TT_REGIONS ? opponent->region : Tt_CurrentRegion();
    U.region = region < TT_REGIONS ? region : TT_REGION_PETALBURG;
    Open(T_CHALLENGE);
    U.cursor = c->activeDeck;
    if (!c->optedIn)
    {
        U.rules = c->regionRules[U.region];
        U.trade = c->regionTrade[U.region];
        Say(ACT_CLOSE, "You have no TRIPLE TRIAD cards yet. A TRIPLE TRIAD seller in a POKeMON CENTER has a hand "
                       "for you!");
        return;
    }
    /* Rules on the move: a carried rule may spread here, or one of theirs fall away */
    mix = Tt_MixRules(c, U.region, Tt_Rand(&c->rng));
    U.rules = c->regionRules[U.region];
    U.trade = c->regionTrade[U.region];
    /* CARD SCALING: the hand they bring fits the deck in use (refitted to the deck picked, unless OPEN shows it) */
    U.oppLevel = U.opp.level;
    if (Fits() && DeckOk(c->activeDeck))
    {
        Tt_FitOpponentHand(&U.opp, Tt_HandPower(Deck(c->activeDeck)->cards) + c->skill, U.rules, Tcg_Today(),
                           U.opp.hand);
        printf("[tt] %s fits a hand of power %d to yours %d (skill %d)\n", U.opp.name, Tt_HandPower(U.opp.hand),
               Tt_HandPower(Deck(c->activeDeck)->cards), c->skill);
    }
    if (U.rules & TT_RULE_BIT(TT_RULE_RANDOM))
        U.cursor = 0;
    if (mix.kind == TT_MIX_SPREAD)
    {
        Say(ACT_NONE, "Rules on the move! %s, the rule you carried from %s, spread to %s!", Tt_RuleName(mix.rule),
            Tt_RegionName(c->carriedFrom), Tt_RegionName(U.region));
        MP_Achieve_Count("ttspread", 1);
        PlaySE(SE_M_STAT_INCREASE);
    }
    else if (mix.kind == TT_MIX_ABOLISH)
    {
        Say(ACT_NONE, "Rules on the move! The rule %s was abolished in %s!", Tt_RuleName(mix.rule),
            Tt_RegionName(U.region));
        MP_Achieve_Count("ttabolish", 1);
        PlaySE(SE_M_STAT_DECREASE);
    }
    printf("[tt] challenge: %s in %s, rules %x trade %u, mix %u %u\n", U.opp.name, Tt_RegionName(U.region), U.rules,
           U.trade, mix.kind, mix.rule);
}
/* A friend said yes to a game: both pick a hand (the asker's invite carries their rules and theirs, tt_net.c) */
void TtUi_OpenOnlineDeck(void)
{
    const struct TcgSocial *s = Tcg_Social();
    static struct TcgOpponent face;
    unsigned region = Tt_CurrentRegion();
    U.returnToGame = NULL;
    U.fromScript = 0;
    memset(&U.opp, 0, sizeof(U.opp));
    snprintf(U.opp.name, sizeof(U.opp.name), "%s", PeerName(s->peer));
    memset(&face, 0, sizeof(face));
    Tcg_PortraitOfPlayer(s->peer, &face);
    U.opp.portraitW = face.portraitW;
    U.opp.portraitH = face.portraitH;
    memcpy(U.opp.portrait, face.portrait, sizeof(U.opp.portrait));
    U.opp.special = TT_SPECIAL_NONE;
    U.opp.online = 1;
    U.opp.music = MUS_RG_VS_TRAINER;
    U.region = region < TT_REGIONS ? region : TT_REGION_PETALBURG;
    U.rules = s->seat == 0 ? C()->regionRules[U.region] : 0;
    U.trade = TT_TRADE_ONE;
    Open(T_CHALLENGE);
    U.vs = VS_FRIEND;
    U.online = 0;
    U.onlineDeckChosen = 0;
    U.cursor = C()->activeDeck;
    printf("[tt] friend's game: %s, seat %u\n", U.opp.name, s->seat);
}
void TtUi_OpenOnlineTrade(void)
{
    U.returnToGame = NULL;
    U.fromScript = 0;
    memset(&U.opp, 0, sizeof(U.opp));
    snprintf(U.opp.name, sizeof(U.opp.name), "%s", PeerName(Tcg_Social()->peer));
    Open(T_TRADE);
    U.vs = VS_FRIEND;
    U.stars = 1;
    printf("[tt] trade with %s\n", U.opp.name);
}
void TtUi_OpenSeller(unsigned region)
{
    U.returnToGame = NULL;
    U.shopRegion = (int)(region < TT_REGIONS ? region : 0);
    U.shopTab = 0;
    Open(T_SHOP);
    U.fromScript = 1;
}
void TtUi_OpenStarter(void)
{
    struct TtCollection *c = C();
    U.returnToGame = NULL;
    Open(T_STARTER);
    U.fromScript = 1;
    Tt_GiveStarter(c);
    MP_Achieve_Unlock("ttstart");
    StartReveal(c->decks[0].cards, TT_HAND, 0x1F);
    U.screen = T_STARTER;
}
void TtUi_OpenQueen(const struct TtOpponent *queen)
{
    U.returnToGame = NULL;
    U.opp = *queen;
    Open(T_QUEEN);
    U.fromScript = 1;
}

/* ------------------------------------------------------------------ */
/* Acting on a YES                                                      */
/* ------------------------------------------------------------------ */
static void TakeSpoils(void)
{
    struct TtCollection *c = C();
    char line[200];
    int n = 0, i, fresh = 0;
    uint8_t take = U.spoilPicked | U.spoilFixed;
    for (i = 0; i < TT_HAND; i++)
        if (take >> i & 1)
        {
            unsigned id = U.dealt[1][i];
            fresh += Tt_Give(c, id);
            n += snprintf(line + n, sizeof(line) - (size_t)n, "%s%s", n ? ", " : "", Tt_Card(id)->name);
            if (Tt_Card(id)->stars == 5)
                MP_Achieve_Unlock("tt5star");
        }
    MP_Achieve_Count("ttcards", __builtin_popcount(take));
    if (Tt_OwnedCount(c, 0, NULL) >= TT_CARD_COUNT)
        MP_Achieve_Unlock("ttall");
    Say(ACT_CLOSE, "You took %s!%s", line, fresh ? " New to your collection!" : "");
    PlaySE(SE_SUCCESS);
}
static void Act(int action)
{
    struct TtCollection *c = C();
    switch (action)
    {
    case ACT_CLOSE:
        Close();
        break;
    case ACT_HOME:
        Screen(T_HOME);
        break;
    case ACT_COPY:
    {
        unsigned d;
        for (d = 0; d < TT_DECKS; d++)
            if (!DeckCount(d))
            {
                memcpy(Deck(d)->cards, Deck((unsigned)U.askArg)->cards, sizeof(Deck(d)->cards));
                snprintf(Deck(d)->name, sizeof(Deck(d)->name), "%.13s 2", Deck((unsigned)U.askArg)->name);
                PlaySE(SE_SUCCESS);
                return;
            }
        Say(ACT_NONE, "There's no empty slot to copy it into.");
        break;
    }
    case ACT_CLEAR:
        memset(Deck((unsigned)U.askArg)->cards, 0, sizeof(Deck(0)->cards));
        snprintf(Deck((unsigned)U.askArg)->name, sizeof(Deck(0)->name), "DECK %d", U.askArg + 1);
        break;
    case ACT_BUY_CARD:
    {
        unsigned id = (unsigned)U.askArg, price = Tt_SinglePrice(id);
        if (!price || Bought(id) || !IsEnoughMoney(&gSaveBlock1Ptr->money, price))
            break;
        RemoveMoney(&gSaveBlock1Ptr->money, price);
        c->bought[id / 8] |= 1 << (id % 8);
        Tt_Give(c, id);
        PlaySE(SE_SHOP);
        break;
    }
    case ACT_BUY_PACK:
    {
        unsigned pack = (unsigned)U.askArg, price = Tt_PackPrice(pack);
        if (!price || !IsEnoughMoney(&gSaveBlock1Ptr->money, price))
            break;
        RemoveMoney(&gSaveBlock1Ptr->money, price);
        c->packs[pack]++;
        PlaySE(SE_SHOP);
        Ask(ACT_OPEN_BOUGHT, (int)pack, "A %s PACK! Open it now?", Tt_PackName(pack));
        break;
    }
    case ACT_OPEN_BOUGHT:
    {
        static uint8_t before[TT_CARD_MAX + 1];
        uint16_t got[TT_PACK_SIZE];
        uint8_t fresh = 0;
        unsigned i, pack = (unsigned)U.askArg;
        memcpy(before, c->owned, sizeof(before));   /* (which are new: looked at before they go in) */
        if (!Tt_OpenPack(c, pack, got))
            break;
        for (i = 0; i < TT_PACK_SIZE; i++)
            fresh |= (uint8_t)(!before[got[i]] << i);
        MP_Achieve_Count("ttpack", 1);
        U.back = U.screen;
        StartReveal(got, TT_PACK_SIZE, fresh);
        break;
    }
    case ACT_SELL:
    {
        unsigned id = (unsigned)U.askArg;
        if (Owned(id) < 2)
            break;
        c->owned[id]--;
        AddMoney(&gSaveBlock1Ptr->money, Tt_Card(id)->sale);
        PlaySE(SE_SHOP);
        break;
    }
    case ACT_TAKE:
        TakeSpoils();
        break;
    case ACT_LEAVE_DECK:
        Screen(T_DECKS);
        U.cursor = U.deck;
        break;
    case ACT_QUEEN_TRAVEL:
    {
        unsigned to = (unsigned)U.askArg, rule;
        if (!IsEnoughMoney(&gSaveBlock1Ptr->money, QUEEN_FARE))
            break;
        RemoveMoney(&gSaveBlock1Ptr->money, QUEEN_FARE);
        rule = Tt_QueenMoves(c, to, Tcg_Today(), Tt_Rand(&c->rng));
        Tt_QueenPlaced();
        if (rule != 0xFF)
            Say(ACT_CLOSE, "To %s, then! ...And I'll teach them a rule from a far-off land: %s.", Tt_RegionName(to),
                Tt_RuleName(rule));
        else
            Say(ACT_CLOSE, "To %s, then! Look for me there.", Tt_RegionName(to));
        PlaySE(SE_SHOP);
        break;
    }
    case ACT_QUEEN_TRADE:
        c->regionTrade[c->queenRegion] = (uint8_t)U.askArg;
        c->flags &= ~TT_FLAG_QUEEN_FAVOR;
        Say(ACT_NONE, "Done. In %s, the TRADE RULE is %s now.", Tt_RegionName(c->queenRegion), Tt_TradeName((unsigned)U.askArg));
        PlaySE(SE_SUCCESS);
        break;
    case ACT_QUEEN_ABOLISH:
        c->regionRules[c->queenRegion] &= ~(1u << U.askArg);
        c->flags &= ~TT_FLAG_QUEEN_FAVOR;
        c->abolishes++;
        MP_Achieve_Count("ttabolish", 1);
        Say(ACT_NONE, "%s is no more in %s.", Tt_RuleName((unsigned)U.askArg), Tt_RegionName(c->queenRegion));
        PlaySE(SE_SUCCESS);
        break;
    default:
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Buttons                                                              */
/* ------------------------------------------------------------------ */
int TtUi_Step(unsigned keys)
{
    static uint16_t list[TT_CARD_MAX + 1];
    struct TtCollection *c = C();
    unsigned pressed = keys & ~U.previousKeys;
    U.previousKeys = keys;
    if (!TtUi_Active())
        return 0;
    U.age++;
    U.frame++;
    if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
    {
        if (++U.held > 18 && U.held % 4 == 0 && U.screen != T_DUEL)
            pressed |= keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
    }
    else
        U.held = 0;
    OnlineStep();
    if (U.screen == T_DUEL && !U.message[0])
    {
        DuelStep(pressed, keys);
        return 1;
    }
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
    if (SearchStep(pressed))
        return 1;
    switch (U.screen)
    {
    case T_HOME:
        MoveCursor(pressed, HOME_ROWS, 1);
        if (pressed & KEY_SELECT)
            TcgGuide_Open(KIT_GAME_TRIAD);
        else if (pressed & (KEY_L | KEY_R))
        {
            PlaySE(SE_SELECT);
            ToSide((pressed & KEY_R) != 0);
        }
        else if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            if (!c->optedIn && U.cursor < 3)
            {
                Say(ACT_NONE, "Get a first hand from a TRIPLE TRIAD seller in a POKeMON CENTER first!");
                break;
            }
            switch (U.cursor)
            {
            case 0:
                Screen(T_DECKS);
                U.cursor = c->activeDeck;
                break;
            case 1:
                Screen(T_LIST);
                U.stars = 1;
                break;
            case 2:
                Screen(T_PACKS);
                for (U.cursor = 0; U.cursor < TT_PACK_TYPES && !c->packs[U.cursor]; U.cursor++)
                    ;
                if (U.cursor >= TT_PACK_TYPES)
                    U.cursor = 0;
                break;
            case 3:
                Screen(T_RULES);
                U.cursor = (int)(Tt_CurrentRegion() < TT_REGIONS ? Tt_CurrentRegion() : 0);
                break;
            case 4:
                if (c->optedIn)
                    Practice();
                else
                    Say(ACT_NONE, "Get a first hand from a TRIPLE TRIAD seller first!");
                break;
            case 5:
                Tcg_GetCollection()->fastAnimations = !Tcg_GetCollection()->fastAnimations;
                break;
            default:
                Close();
                break;
            }
        }
        break;
    case T_DECKS:
        if (TcgUi_NameActive())
        {
            if (TcgUi_NameStep(pressed) == 1 && U.cursor < TT_DECKS)
                snprintf(Deck((unsigned)U.cursor)->name, sizeof(Deck(0)->name), "%.*s", TT_DECK_NAME, TcgUi_NameText());
            break;
        }
        if (U.menuOn)
        {
            int empty = !DeckCount((unsigned)U.cursor), count = empty ? 1 : DM_COUNT;
            U.menuCursor = TcgUi_PopMenuMove(pressed, U.menuCursor, count, 0);
            if (pressed & KEY_B)
            {
                PlaySE(SE_SELECT);
                U.menuOn = 0;
            }
            else if (pressed & KEY_A)
            {
                int slot = U.cursor;
                PlaySE(SE_SELECT);
                U.menuOn = 0;
                if (empty || U.menuCursor == DM_EDIT)
                {
                    U.deck = slot;
                    U.deckSlot = 0;
                    while (U.deckSlot < TT_HAND - 1 && Deck((unsigned)slot)->cards[U.deckSlot])
                        U.deckSlot++;
                    U.inDeckRow = 0;
                    U.stars = 1;
                    Screen(T_BUILDER);
                }
                else if (U.menuCursor == DM_RENAME)
                    TcgUi_NameOpen("RENAME DECK", Deck((unsigned)slot)->name);
                else if (U.menuCursor == DM_USE)
                {
                    if (DeckOk((unsigned)slot))
                        c->activeDeck = (uint8_t)slot, PlaySE(SE_SUCCESS);
                    else
                        Say(ACT_NONE, "This deck isn't ready: it needs five cards that fit the star limits.");
                }
                else if (U.menuCursor == DM_COPY)
                    Ask(ACT_COPY, slot, "Copy %s into an empty slot?", Deck((unsigned)slot)->name);
                else if (U.menuCursor == DM_DELETE)
                    Ask(ACT_CLEAR, slot, "Empty %s? Its cards stay in your collection.", Deck((unsigned)slot)->name);
            }
            break;
        }
        MoveCursor(pressed, DECK_ROWS, 1);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == TT_DECKS))
        {
            PlaySE(SE_SELECT);
            Screen(T_HOME);
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            U.menuOn = 1;
            U.menuCursor = 0;
        }
        else if (pressed & KEY_R && U.cursor < TT_DECKS && DeckOk((unsigned)U.cursor))
        {
            c->activeDeck = (uint8_t)U.cursor;
            PlaySE(SE_SUCCESS);
        }
        break;
    case T_BUILDER:
    {
        struct TtDeck *d = Deck((unsigned)U.deck);
        unsigned n = BuilderList(list);
        ListTabStep(pressed);
        if (U.inDeckRow)
        {
            if (pressed & KEY_LEFT && U.deckSlot > 0)
                U.deckSlot--, PlaySE(SE_SELECT);
            else if (pressed & KEY_RIGHT && U.deckSlot < TT_HAND - 1)
                U.deckSlot++, PlaySE(SE_SELECT);
            else if (pressed & KEY_DOWN)
                U.inDeckRow = 0, PlaySE(SE_SELECT);
            else if (pressed & KEY_A && d->cards[U.deckSlot])
            {
                d->cards[U.deckSlot] = 0;
                PlaySE(SE_RG_CARD_FLIPPING);
            }
            else if (pressed & KEY_INSPECT && d->cards[U.deckSlot])
                Inspect(d->cards[U.deckSlot], T_BUILDER);
        }
        else
        {
            if (pressed & KEY_UP && U.cursor < GRID_COLS)
            {
                U.inDeckRow = 1;
                PlaySE(SE_SELECT);
            }
            else
                GridMove(pressed, (int)n, GRID_COLS);
            if (pressed & KEY_A && n)
            {
                unsigned id = list[U.cursor], k;
                char why[96];
                for (k = 0; k < TT_HAND && d->cards[k] != id; k++)
                    ;
                if (k < TT_HAND)
                {
                    d->cards[k] = 0;   /* in already: A takes it out again */
                    U.deckSlot = (int)k;
                    PlaySE(SE_RG_CARD_FLIPPING);
                }
                else if (!Tt_DeckFits(d->cards, (unsigned)U.deckSlot, id, why, sizeof(why)))
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s", why);
                }
                else
                {
                    d->cards[U.deckSlot] = (uint16_t)id;
                    PlaySE(SE_RG_CARD_FLIPPING);
                    for (k = 0; k < TT_HAND && d->cards[U.deckSlot]; k++)
                        U.deckSlot = (U.deckSlot + 1) % TT_HAND;
                }
            }
            else if (pressed & KEY_INSPECT && n)
                Inspect(list[U.cursor], T_BUILDER);
        }
        if (pressed & (KEY_START | KEY_B))
        {
            char why[96];
            PlaySE(SE_SELECT);
            if (DeckCount((unsigned)U.deck) && !Tt_DeckValid(d->cards, why, sizeof(why)))
                Ask(ACT_LEAVE_DECK, 0, "%s Leave it unfinished?", why);
            else
            {
                if (DeckOk((unsigned)U.deck) && !DeckOk(c->activeDeck))
                    c->activeDeck = (uint8_t)U.deck;
                Screen(T_DECKS);
                U.cursor = U.deck;
            }
        }
        break;
    }
    case T_LIST:
    {
        unsigned n = BinderList(list);
        ListTabStep(pressed);
        GridMove(pressed, (int)n, GRID_COLS);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Screen(T_HOME);
            U.cursor = 1;
        }
        else if (pressed & (KEY_A | KEY_INSPECT) && n && (Owned(list[U.cursor]) || Tt_Seen(c, list[U.cursor])))
            Inspect(list[U.cursor], T_LIST);
        break;
    }
    case T_INSPECT:
        if (pressed & KEY_DOWN)
            U.textScroll++, PlaySE(SE_SELECT);
        if (pressed & KEY_UP && U.textScroll > 0)
            U.textScroll--, PlaySE(SE_SELECT);
        if (pressed & (KEY_A | KEY_B | KEY_INSPECT))
        {
            int keep = U.cursor;
            PlaySE(SE_SELECT);
            Screen(U.back);
            U.cursor = keep;
        }
        break;
    case T_PACKS:
        MoveCursor(pressed, TT_PACK_TYPES + 1, 1);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == TT_PACK_TYPES))
        {
            PlaySE(SE_SELECT);
            Screen(T_HOME);
            U.cursor = 2;
        }
        else if (pressed & KEY_A && c->packs[U.cursor])
        {
            U.askArg = U.cursor;
            Act(ACT_OPEN_BOUGHT);
            U.back = T_PACKS;
        }
        break;
    case T_REVEAL:
    case T_STARTER:
    {
        int end = 12 + (U.revealCount - 1) * 20 + 14;
        U.revealT++;
        if (U.revealT % 20 == 12 && U.revealT < end)
            PlaySE(SE_RG_CARD_FLIP);
        if (pressed & KEY_B && U.revealT < end)
            U.revealT = end;
        else if (pressed & KEY_A)
        {
            if (U.revealT < end)
                U.revealT = ((U.revealT - 12) / 20 + 1) * 20 + 12;   /* the next card now */
            else if (U.screen == T_STARTER)
                Say(ACT_CLOSE, "Your first hand: five cards, ready to play!");
            else
            {
                PlaySE(SE_SELECT);
                Screen(U.back == T_SHOP ? T_SHOP : T_PACKS);
                if (U.back == T_SHOP)
                    U.cursor = U.askArg;
            }
        }
        break;
    }
    case T_RULES:
        MoveCursor(pressed, TT_REGIONS, 1);
        if (pressed & (KEY_A | KEY_B))
        {
            PlaySE(SE_SELECT);
            Screen(T_HOME);
            U.cursor = 3;
        }
        break;
    case T_CHALLENGE:
    {
        int random = (U.rules & TT_RULE_BIT(TT_RULE_RANDOM)) != 0 && U.vs != VS_FRIEND,
            rows = random ? 2 : TT_DECKS + 1;
        MoveCursor(pressed, rows, 1);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == rows - 1))
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            if (U.vs == VS_FRIEND)
                PickFriendDeck();
            else if (random)
            {
                uint16_t none[TT_HAND] = {0};
                StartGame(DeckOk(c->activeDeck) ? Deck(c->activeDeck)->cards : none);
            }
            else if (!DeckOk((unsigned)U.cursor))
                Say(ACT_NONE, "That deck isn't ready.");
            else
            {
                unsigned lv;
                c->activeDeck = (uint8_t)U.cursor;
                TheirHandLevel((unsigned)U.cursor, U.opp.hand, &lv);
                U.opp.level = (uint8_t)lv;
                StartGame(Deck((unsigned)U.cursor)->cards);
            }
        }
        break;
    }
    case T_WAIT:
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        break;
    case T_TRADE:
        TradeStep(pressed);
        break;
    case T_SPOILS:
        if (pressed & KEY_LEFT && U.cursor > 0)
            U.cursor--, PlaySE(SE_SELECT);
        if (pressed & KEY_RIGHT && U.cursor < TT_HAND - 1)
            U.cursor++, PlaySE(SE_SELECT);
        if (pressed & KEY_INSPECT)
            Inspect(U.dealt[1][U.cursor], T_SPOILS);
        else if (U.spoilFixed && pressed & (KEY_A | KEY_START))
            Ask(ACT_TAKE, 0, "Take them?");
        else if (!U.spoilFixed && pressed & KEY_A)
        {
            uint8_t bit = (uint8_t)(1 << U.cursor);
            if (U.spoilAllowed == 1)
            {
                U.spoilPicked = bit;
                Ask(ACT_TAKE, 0, "Take %s?", Tt_Card(U.dealt[1][U.cursor])->name);
            }
            else if (U.spoilPicked & bit)
                U.spoilPicked &= (uint8_t)~bit, PlaySE(SE_SELECT);
            else if (__builtin_popcount(U.spoilPicked) < U.spoilAllowed)
            {
                U.spoilPicked |= bit;
                PlaySE(SE_RG_CARD_FLIPPING);
                if (__builtin_popcount(U.spoilPicked) == U.spoilAllowed)
                    Ask(ACT_TAKE, 0, "Take these %d?", U.spoilAllowed);
            }
        }
        else if (!U.spoilFixed && pressed & KEY_START && U.spoilPicked)
            Ask(ACT_TAKE, 0, "Take the ones you picked?");
        break;
    case T_SHOP:
    {
        unsigned n = U.shopTab == 1 ? TT_PACK_TYPES : ShopList(list);
        if (pressed & (KEY_L | KEY_R))
        {
            U.shopTab = (U.shopTab + (pressed & KEY_R ? 1 : SHOP_TABS - 1)) % SHOP_TABS;
            U.cursor = 0;
            PlaySE(SE_SELECT);
            break;
        }
        MoveCursor(pressed, n ? (int)n : 1, 1);
        if (pressed & KEY_B)
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A && U.shopTab == 1)
        {
            unsigned price = Tt_PackPrice((unsigned)U.cursor);
            PlaySE(SE_SELECT);
            if (!price)
                Say(ACT_NONE, "The %s PACK is never sold. Win it!", Tt_PackName((unsigned)U.cursor));
            else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
                Say(ACT_NONE, "You don't have enough money.");
            else
            {
                U.back = T_SHOP;
                Ask(ACT_BUY_PACK, U.cursor, "A %s PACK for \xa5%u?", Tt_PackName((unsigned)U.cursor), price);
            }
        }
        else if (pressed & KEY_A && n)
        {
            unsigned id = list[U.cursor];
            PlaySE(SE_SELECT);
            if (U.shopTab == 0)
            {
                if (Bought(id))
                    Say(ACT_NONE, "Sold out: one per customer!");
                else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, Tt_SinglePrice(id)))
                    Say(ACT_NONE, "You don't have enough money.");
                else
                    Ask(ACT_BUY_CARD, (int)id, "%s for \xa5%u?", Tt_Card(id)->name, Tt_SinglePrice(id));
            }
            else
                Ask(ACT_SELL, (int)id, "Sell a spare %s for \xa5%u?", Tt_Card(id)->name, Tt_Card(id)->sale);
        }
        else if (pressed & KEY_INSPECT && n && U.shopTab != 1)
            Inspect(list[U.cursor], T_SHOP);
        break;
    }
    case T_QUEEN:
    {
        unsigned off = QueenOff();
        if (U.menuOn)
        {
            int count = U.menuOn == Q_TRADE ? TT_TRADE_COUNT : U.menuOn == Q_TRAVEL ? TT_REGIONS
                      : __builtin_popcount(c->regionRules[c->queenRegion]);
            U.menuCursor = TcgUi_PopMenuMove(pressed, U.menuCursor, count, 0);
            if (pressed & KEY_B)
                U.menuOn = 0, PlaySE(SE_SELECT);
            else if (pressed & KEY_A && count)
            {
                int pick = U.menuCursor, kind = U.menuOn;
                U.menuOn = 0;
                PlaySE(SE_SELECT);
                if (kind == Q_TRAVEL)
                {
                    if ((unsigned)pick == c->queenRegion)
                        Say(ACT_NONE, "But I'm in %s already!", Tt_RegionName((unsigned)pick));
                    else
                        Ask(ACT_QUEEN_TRAVEL, pick, "Pay \xa5%u for my journey to %s?", QUEEN_FARE,
                            Tt_RegionName((unsigned)pick));
                }
                else if (kind == Q_TRADE)
                    Ask(ACT_QUEEN_TRADE, pick, "Make the TRADE RULE in %s %s? %s", Tt_RegionName(c->queenRegion),
                        Tt_TradeName((unsigned)pick), Tt_TradeHelp((unsigned)pick));
                else
                {
                    unsigned r, k = 0;
                    for (r = 0; r < TT_RULE_COUNT; r++)
                        if (c->regionRules[c->queenRegion] >> r & 1 && k++ == (unsigned)pick)
                            break;
                    Ask(ACT_QUEEN_ABOLISH, (int)r, "Abolish %s in %s?", Tt_RuleName(r), Tt_RegionName(c->queenRegion));
                }
            }
            break;
        }
        MoveCursor(pressed, Q_COUNT, 1);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == Q_LEAVE))
        {
            PlaySE(SE_SELECT);
            Close();
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            if (off >> U.cursor & 1)
            {
                if (U.cursor == Q_PLAY)
                    Say(ACT_NONE, "Come back with a deck of five, darling.");
                else if (U.cursor == Q_TRAVEL)
                    Say(ACT_NONE, "A queen doesn't travel for less than \xa5%u.", QUEEN_FARE);
                else
                    Say(ACT_NONE, "Beat me at the table first. Then we'll talk favors.");
            }
            else if (U.cursor == Q_PLAY)
            {
                struct TtOpponent queen = U.opp;
                int script = U.fromScript;
                TtUi_OpenChallenge(&queen);
                U.fromScript = script;
            }
            else
            {
                U.menuOn = U.cursor;
                U.menuCursor = U.cursor == Q_TRAVEL ? (int)c->queenRegion : 0;
                if (U.cursor == Q_ABOLISH && !c->regionRules[c->queenRegion])
                {
                    U.menuOn = 0;
                    Say(ACT_NONE, "This region plays no special rules at all. Nothing to abolish!");
                }
            }
        }
        break;
    }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* The test harness (mp tt ...)                                         */
/* ------------------------------------------------------------------ */
void Tt_Debug(const char *command)
{
    struct TtCollection *c = C();
    unsigned a = 0, b = 0;
    if (!strcmp(command, "starter"))
    {
        if (!c->optedIn)
            Tt_GiveStarter(c);
        Tcg_GrantCase();   /* (and the CARD CASE to keep them in) */
        printf("[tt] starter: deck 0 ready %d\n", DeckOk(0));
    }
    else if (!strncmp(command, "search ", 7))
    {
        TcgSearch_Register(SEARCH_TRIAD, &sSearchGame);
        TcgSearch_Debug(SEARCH_TRIAD, command + 7);
    }
    else if (!strcmp(command, "binder") || !strcmp(command, "builder"))
    {
        if (U.screen == T_CLOSED)
            TtUi_OpenCase(NULL, 0);
        U.deck = c->activeDeck;
        U.inDeckRow = 0;
        Screen(command[1] == 'i' ? T_LIST : T_BUILDER);
        if (!U.stars)
            U.stars = 1;
    }
    else if (!strcmp(command, "shown"))
    {
        static uint16_t list[TT_CARD_MAX + 1];
        unsigned i, n = Shown(list, U.screen == T_BUILDER);
        printf("[tt] shown %u (screen %d cursor %d card %u deck %u):", n, U.screen, U.cursor, n ? list[U.cursor] : 0,
               DeckCount((unsigned)U.deck));
        for (i = 0; i < n && i < 12; i++)
            printf(" %s,", Met(list[i]) ? Tt_Card(list[i])->name : "???");
        printf("\n");
    }
    else if (!strcmp(command, "giveall"))
    {
        unsigned id;
        for (id = 1; id <= TT_CARD_MAX; id++)
            if (Tt_Card(id))
                Tt_Give(c, id);
        c->optedIn = 1;
        printf("[tt] every card given\n");
    }
    else if (sscanf(command, "give %u %u", &a, &b) >= 1)
    {
        unsigned k;
        for (k = 0; k < (b ? b : 1); k++)
            Tt_Give(c, a);
        printf("[tt] gave %u x%u\n", a, b ? b : 1);
    }
    else if (sscanf(command, "packs %u %u", &a, &b) == 2 && a < TT_PACK_TYPES)
        c->packs[a] = (uint16_t)b;
    else if (sscanf(command, "first %u", &a) == 1)
        sForceFirst = (int)a;
    else if (!strcmp(command, "auto"))
        sAutoPlay = !sAutoPlay, printf("[tt] autoplay %d\n", sAutoPlay);
    else if (sscanf(command, "rules %u %x", &a, &b) == 2 && a < TT_REGIONS)
        c->regionRules[a] = b, printf("[tt] %s rules %x\n", Tt_RegionName(a), b);
    else if (sscanf(command, "trade %u %u", &a, &b) == 2 && a < TT_REGIONS && b < TT_TRADE_COUNT)
        c->regionTrade[a] = (uint8_t)b;
    else if (sscanf(command, "carry %u", &a) == 1)
        Tt_PlayedIn(c, a), printf("[tt] carrying %s's rules %x\n", Tt_RegionName(a), c->carried);
    else if (sscanf(command, "queen %u", &a) == 1 && a < TT_REGIONS)
        c->queenRegion = (uint8_t)a, Tt_QueenPlaced();
    else if (!strcmp(command, "favor"))
        c->flags |= TT_FLAG_QUEEN_FAVOR;
    else if (!strcmp(command, "world"))
    {
        /* HOENN's rules as this game sees them (the host's in a friend's room) */
        unsigned r;
        printf("[tt] world%s: rules", Tt_WorldFromHost() ? " (the host's)" : "");
        for (r = 0; r < TT_REGIONS; r++)
            printf(" %x", c->regionRules[r]);
        printf(" trade");
        for (r = 0; r < TT_REGIONS; r++)
            printf(" %u", c->regionTrade[r]);
        printf(" queen %s\n", Tt_RegionName(c->queenRegion));
    }
    else if (!strcmp(command, "practice"))
    {
        if (!TtUi_Active())
            TtUi_OpenCase(NULL, 0);
        Practice();
    }
    else if (!strcmp(command, "case"))
        TtUi_OpenCase(NULL, 0);
    else if (sscanf(command, "shop %u", &a) == 1)
        TtUi_OpenSeller(a);
    else if (!strcmp(command, "queenmenu"))
    {
        static struct TtOpponent queen;
        Tt_QueenOpponent(&queen);
        TtUi_OpenQueen(&queen);
    }
    else if (sscanf(command, "focus %u", &a) == 1)
        TtBoard_DebugFocus((int)a);
    else if (!strcmp(command, "win") && U.screen == T_DUEL && !U.duel.over)
    {
        /* the game ends at once, won: every card on the table turns blue */
        unsigned i;
        for (i = 0; i < TT_CELLS; i++)
            if (U.duel.cell[i].card)
                U.duel.cell[i].owner = (uint8_t)Me();
        U.duel.over = 1;
        U.duel.winner = Me();
        U.duel.evCount = 1;
        memset(&U.duel.ev[0], 0, sizeof(U.duel.ev[0]));
        U.duel.ev[0].kind = TTE_END;
        U.duel.ev[0].cell = 0xFF;
        TtBoard_Show(&U.duel);
        printf("[tt] won by the harness (table %d)\n", TtBoard_DebugPhase());
    }
    else if (!strcmp(command, "net"))
    {
        struct TtNet *l = Tt_Online();
        printf("[tt-net] state %u seat %u seq %u pending %u hash %08x local %08x over %u winner %u score %u-%u\n",
               l->state, l->seat, l->sequence, l->pending, (unsigned)TtDuel_Hash(&l->game), (unsigned)TtDuel_Hash(&U.duel),
               l->game.over, l->game.winner, TtDuel_Score(&l->game, 0), TtDuel_Score(&l->game, 1));
    }
    else if (!strcmp(command, "concede") && U.screen == T_DUEL && !U.duel.over)
    {
        if (U.online && !TtNet_Concede(Tt_Online(), SDL_GetTicks()))
            U.concedeHeld = 1;
        else
        {
            TtDuel_Concede(&U.duel, Me());
            ShowEnd();
        }
        printf("[tt] conceded by the harness\n");
    }
    else if (!strcmp(command, "state"))
        printf("[tt] screen %d cards %u decks ready %d region %s rules %x trade %u carried %x from %u queen %s wins %u "
               "losses %u\n", U.screen, Tt_OwnedCount(c, 0, NULL), DeckOk(c->activeDeck), Tt_RegionName(Tt_CurrentRegion()),
               Tt_CurrentRegion() < TT_REGIONS ? c->regionRules[Tt_CurrentRegion()] : 0,
               Tt_CurrentRegion() < TT_REGIONS ? c->regionTrade[Tt_CurrentRegion()] : 0, c->carried, c->carriedFrom,
               Tt_RegionName(c->queenRegion), c->wins, c->losses);
    else
        printf("[tt] unknown: %s\n", command);
}
