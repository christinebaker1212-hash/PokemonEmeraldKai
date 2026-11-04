// Moemon Kai: the CARD CASE's MAGIC side (hidden until the player has a deck: docs/MTG_DECISIONS.md) and the
// screens around a duel: DECKS (eight slots), the deck builder (basic lands free, the owned cards, four of a card),
// the BINDER, a card's whole text, MOSSDEEP's CARD SHOP (the starter decks), a duelist's challenge, the table
// (mtg_board.c), the booster a win brings. Drawn with the other card games' look (src/tcg/tcg_ui_kit.h).
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "sound.h"
#include "constants/songs.h"
#include "mtg/integration.h"
#include "mtg_internal.h"
#include "mtg_board.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_ui_kit.h"

enum
{
    M_CLOSED, M_HOME, M_DECKS, M_BUILDER, M_BINDER, M_INSPECT, M_STARTER, M_CHALLENGE, M_DUEL, M_SPOILS
};
enum
{
    KEY_A = 1, KEY_B = 2, KEY_SELECT = 4, KEY_START = 8, KEY_RIGHT = 16, KEY_LEFT = 32, KEY_UP = 64, KEY_DOWN = 128,
    KEY_R = 256, KEY_L = 512,
    KEY_INSPECT = 2048
};
enum { ACT_NONE, ACT_CLOSE, ACT_HOME, ACT_TAKE_STARTER, ACT_DELETE, ACT_HOWTO };

#define PANEL_W KIT_PANEL_W
#define STRIP_Y KIT_STRIP_Y
#define LIST_X KIT_LIST_X
#define LIST_W KIT_LIST_W
#define MTG_THEME 0x7a4a2a         // the case's MAGIC side: warm brown, with gold
#define MTG_GOLD 0xe0b050
#define LIST_ROWS 7
#define HOME_ROWS 4

static struct
{
    int screen, back, cursor, scroll, frame, age;
    unsigned previousKeys, held;
    char message[320];
    int messageThen, asking, askYes, askAction, askArg;
    int fromScript;
    void (*returnToGame)(void);
    int menuOn, menuCursor;
    int deck;                        // the deck being looked at or built
    uint16_t list[MTG_COLLECTION_CARDS];
    int listCount;
    int inspectCard;
    int duelist;
    uint16_t booster[MTG_BOOSTER];
    int boosterCount;
    int howto;
} U;

static MtgCollection *C(void) { return Mtg_GetCollection(); }
int Tcg_GrantCase(void);

static void Say(int then, const char *format, ...) __attribute__((format(printf, 2, 3)));
static void Say(int then, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(U.message, sizeof(U.message), format, ap);
    va_end(ap);
    U.messageThen = then;
    U.asking = 0;
}
static void Ask(int action, int arg, const char *format, ...) __attribute__((format(printf, 3, 4)));
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
    if (screen != M_SPOILS)
        MtgBoard_ReleaseMusic();   // the victory song ends with the booster
    U.screen = screen;
    U.cursor = 0;
    U.scroll = 0;
    U.age = 0;
    U.menuOn = 0;
}
int MtgUi_Active(void) { return U.screen != M_CLOSED; }
unsigned MtgUi_DuelingNow(void) { return U.screen == M_DUEL; }

static void Close(void)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    if (U.screen == M_DUEL)
        MtgBoard_End();
    U.message[0] = 0;
    U.asking = 0;
    Screen(M_CLOSED);
    U.returnToGame = NULL;
    U.fromScript = 0;
    Tcg_CloseScreens(callback, script);
}
static void Open(int screen)
{
    U.message[0] = 0;
    U.asking = 0;
    Screen(screen);
    PlaySE(SE_WIN_OPEN);
}
void MtgUi_OpenCase(void (*returnToGame)(void), int fromScript)
{
    Mtg_EnsureDb();
    U.returnToGame = returnToGame;
    U.message[0] = 0;
    U.asking = 0;
    Screen(M_HOME);
    U.fromScript = fromScript;
    C()->flags |= MTG_FLAG_CASE_SIDE;
    MPNet_Log("mtg: case open");
}

// L the side before, R the one after (POKeMON, YU-GI-OH!, TRIPLE TRIAD, MAGIC)
static void ToSide(int right)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.message[0] = 0;
    Screen(M_CLOSED);
    C()->flags &= ~MTG_FLAG_CASE_SIDE;
    Tcg_OpenCaseGame(right ? KIT_GAME_POKEMON : KIT_GAME_TRIAD, callback, script);
}

// ---- helpers
static void PlainName(char *out, unsigned cap, int card)
{
    Mtg_PlainText(out, cap, Mtg_Card(card)->name);
}

static void CostText(char *out, unsigned cap, const MtgManaCost *c)
{
    static const char sym[] = "WUBRGC";
    unsigned n = 0, i, k;
    out[0] = 0;
    if (c->x && n + 1 < cap)
        out[n++] = 'X';
    if (c->generic && n + 4 < cap)
        n += (unsigned)snprintf(out + n, cap - n, "%u", c->generic);
    for (i = 0; i < MTG_MANA_KINDS; i++)
        for (k = 0; k < c->pip[i] && n + 1 < cap; k++)
            out[n++] = sym[i];
    out[n < cap ? n : cap - 1] = 0;
}

static const char *ColorWords(uint8_t colors)
{
    static char buf[64];
    static const char *const names[5] = {"WHITE", "BLUE", "BLACK", "RED", "GREEN"};
    unsigned i, n = 0;
    buf[0] = 0;
    for (i = 0; i < 5; i++)
        if (colors & (1u << i))
            n += (unsigned)snprintf(buf + n, sizeof(buf) - n, "%s%s", n ? " " : "", names[i]);
    return n ? buf : "COLORLESS";
}

// the game's own card back (not the printed one): a gold ring on deep brown
static void CardBack(int x, int y, int w, int h)
{
    G_RoundFill(x, y, w, h, 0x3a2414, 255);
    G_RoundFrame(x, y, w, h, 0x1a0c04, 255);
    G_RoundFill(x + 3, y + 3, w - 6, h - 6, 0x5a3a20, 255);
    G_Circle(x + w / 2, y + h / 2, (float)(w < h ? w : h) / 3.0f, MTG_GOLD, 255);
    G_Circle(x + w / 2, y + h / 2, (float)(w < h ? w : h) / 3.0f - 2.0f, 0x5a3a20, 255);
    G_Circle(x + w / 2, y + h / 2, 3, MTG_GOLD, 255);
}

static void Strip(const char *text)
{
    TcgUi_Strip(NULL, 0, text);
}

static void CardStrip(int card)
{
    char text[600], line[700], cost[16], name[64];
    const MtgCardDef *c = Mtg_Card(card);
    PlainName(name, sizeof(name), card);
    CostText(cost, sizeof(cost), &c->cost);
    Mtg_PlainText(text, sizeof(text), c->oracle);
    if (c->types & MTG_T_CREATURE)
        snprintf(line, sizeof(line), "%s %s  %d/%d\n%s", name, cost, c->power, c->toughness, text);
    else
        snprintf(line, sizeof(line), "%s %s\n%s", name, cost, text);
    TcgUi_Strip(NULL, 0, line);
}

static void MoveCursor(unsigned pressed, int count)
{
    if (!count)
        return;
    if (pressed & KEY_UP)
        U.cursor = (U.cursor + count - 1) % count, PlaySE(SE_SELECT);
    if (pressed & KEY_DOWN)
        U.cursor = (U.cursor + 1) % count, PlaySE(SE_SELECT);
    if (U.cursor < U.scroll)
        U.scroll = U.cursor;
    if (U.cursor >= U.scroll + LIST_ROWS)
        U.scroll = U.cursor - LIST_ROWS + 1;
}

// ---- lists of cards: the binder (owned) and the builder (the five basic lands, then the owned)
static int ListOrder(const void *a, const void *b)
{
    const MtgCardDef *x = Mtg_Card(*(const uint16_t *)a), *y = Mtg_Card(*(const uint16_t *)b);
    int bx = Mtg_IsBasicLand(*(const uint16_t *)a), by = Mtg_IsBasicLand(*(const uint16_t *)b), cx, cy;
    if (bx != by)
        return by - bx;   // the basic lands first
    cx = x->colors ? (x->colors & (x->colors - 1) ? 6 : __builtin_ctz(x->colors)) : 5;
    cy = y->colors ? (y->colors & (y->colors - 1) ? 6 : __builtin_ctz(y->colors)) : 5;
    if (cx != cy)
        return cx - cy;
    cx = Mtg_ManaCostTotal(&x->cost), cy = Mtg_ManaCostTotal(&y->cost);
    if (cx != cy)
        return cx - cy;
    return strcmp(x->name, y->name);
}

static void BuildList(int withBasics)
{
    static const char *const basics[5] = {"Plains", "Island", "Swamp", "Mountain", "Forest"};
    MtgCollection *c = C();
    int i;
    U.listCount = 0;
    if (withBasics)
        for (i = 0; i < 5; i++)
            U.list[U.listCount++] = (uint16_t)Mtg_CardByName(basics[i]);
    for (i = 0; i < Mtg_DbCount() && i < MTG_COLLECTION_CARDS; i++)
        if (c->owned[i] && Mtg_CanUse(i) && !(withBasics && Mtg_IsBasicLand(i)))
            U.list[U.listCount++] = (uint16_t)i;
    qsort(U.list + (withBasics ? 5 : 0), (size_t)(U.listCount - (withBasics ? 5 : 0)), sizeof(U.list[0]), ListOrder);
}

static unsigned RarityColor(int card)
{
    int r = Mtg_Card(card)->rarity;
    return r == MTG_RARITY_RARE ? 0xc08818 : r == MTG_RARITY_UNCOMMON ? 0x6878a0 : G_INK;
}

static void CardRows(int builder)
{
    MtgCollection *c = C();
    int i;
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < LIST_ROWS && U.scroll + i < U.listCount; i++) {
        int card = U.list[U.scroll + i];
        char name[64], right[16];
        PlainName(name, sizeof(name), card);
        TcgUi_FitText(name, builder ? 84 : 96, TF_NARROW);
        if (builder)
            snprintf(right, sizeof(right), Mtg_IsBasicLand(card) ? "%d" : "%d/%d",
                     Mtg_DeckCopies(&c->decks[U.deck], card), c->owned[card]);
        else
            snprintf(right, sizeof(right), "x%d", c->owned[card]);
        TcgUi_Row(LIST_X + 2, 6 + i * 14, LIST_W - 4, U.cursor == U.scroll + i, 1, "", right);
        G_Text(LIST_X + 8, 8 + i * 14, RarityColor(card), G_NO_SHADOW, name, 0, TF_NARROW);
    }
    if (U.scroll > 0)
        G_Arrow(LIST_X + LIST_W / 2 - 3, 1, 0, 0xffffff);
    if (U.scroll + LIST_ROWS < U.listCount)
        G_Arrow(LIST_X + LIST_W / 2 - 3, 105, 1, 0xffffff);
}

// ---- HOME
static const char *const sHomeRows[HOME_ROWS] = {"DECKS", "BINDER", "HOW TO PLAY", "CLOSE"};
static const char *const sHomeHelp[HOME_ROWS] = {"Your decks: pick the one to play, or build one.",
                                                 "Every card you have.", "How a game of MAGIC goes.",
                                                 "Put the CARD CASE away."};

static void HomeScreen(void)
{
    MtgCollection *c = C();
    const MtgSavedDeck *d = &c->decks[c->activeDeck];
    char line[160];
    int i, bob = (int)(sinf(U.frame * 0.06f) * 1.5f), owned = 0;
    TcgUi_Background();
    TcgUi_CaseTabs(KIT_GAME_MAGIC, MTG_THEME);
    G_FillA(34 + 2, 22 + 3, 36, 50, 0x000000, 70);
    CardBack(34, 22 - bob, 36, 50);
    snprintf(line, sizeof(line), "%s", d->count ? d->name : "No deck");
    G_TextAlign(52, 76, TA_CENTER, G_INK, G_INK_SHADOW, line, TF_SMALL);
    snprintf(line, sizeof(line), "%s", ColorWords(Mtg_DeckColors(d)));
    G_TextAlign(52, 88, TA_CENTER, 0x605040, G_NO_SHADOW, line, TF_NARROW);
    snprintf(line, sizeof(line), "WON %u  LOST %u", c->wins, c->losses);
    G_TextAlign(52, 97, TA_CENTER, 0x605040, G_NO_SHADOW, line, TF_NARROW);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < HOME_ROWS; i++)
        TcgUi_Row(LIST_X + 2, 6 + i * 14, LIST_W - 4, U.cursor == i, 1, sHomeRows[i], NULL);
    for (i = 0; i < Mtg_DbCount(); i++)
        owned += c->owned[i] != 0;
    snprintf(line, sizeof(line), "%s\nDIFFERENT CARDS %d", sHomeHelp[U.cursor], owned);
    Strip(line);
    TcgUi_Hints("{L}{R} OTHER GAMES");
}

// ---- DECKS
enum { DM_USE, DM_EDIT, DM_COPY, DM_DELETE, DM_COUNT };
static const char *const sDeckMenu[DM_COUNT] = {"PLAY WITH IT", "EDIT", "COPY", "DELETE"};
static const char *const sEmptyMenu[1] = {"BUILD A DECK"};

static void DecksScreen(void)
{
    MtgCollection *c = C();
    int i;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("DECKS", MTG_THEME);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < MTG_DECKS; i++) {
        const MtgSavedDeck *d = &c->decks[i];
        char right[16];
        snprintf(right, sizeof(right), "%s%d", i == c->activeDeck ? "* " : "", d->count);
        TcgUi_Row(LIST_X + 2, 6 + i * 12, LIST_W - 4, U.cursor == i, 1, d->count ? d->name : "- EMPTY -",
                  d->count ? right : NULL);
    }
    {
        const MtgSavedDeck *d = &c->decks[U.cursor];
        const char *problem = d->count ? Mtg_DeckProblem(c, d) : NULL;
        if (!d->count)
            snprintf(line, sizeof(line), "An empty slot: build a deck here.");
        else
            snprintf(line, sizeof(line), "%s, %d cards%s\n%s", ColorWords(Mtg_DeckColors(d)), d->count,
                     U.cursor == c->activeDeck ? " (in play)" : "", problem ? problem : "Ready to play.");
        Strip(line);
    }
    if (U.menuOn) {
        if (c->decks[U.cursor].count)
            TcgUi_PopMenu(118, 40, 110, sDeckMenu, DM_COUNT, U.menuCursor, 0);
        else
            TcgUi_PopMenu(118, 40, 110, sEmptyMenu, 1, U.menuCursor, 0);
    }
    TcgUi_Hints("{A} CHOOSE  {B} BACK");
}

// ---- the builder
static void BuilderScreen(void)
{
    MtgCollection *c = C();
    const MtgSavedDeck *d = &c->decks[U.deck];
    const char *problem = Mtg_DeckProblem(c, d);
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("BUILD", MTG_THEME);
    snprintf(line, sizeof(line), "%s", d->name);
    G_TextAlign(52, 22, TA_CENTER, G_INK, G_INK_SHADOW, line, TF_SMALL);
    snprintf(line, sizeof(line), "%d CARDS", d->count);
    G_TextAlign(52, 36, TA_CENTER, d->count >= MTG_DECK_MIN ? 0x306030 : 0xa03020, G_NO_SHADOW, line, TF_NORMAL);
    G_TextAlign(52, 52, TA_CENTER, 0x605040, G_NO_SHADOW, ColorWords(Mtg_DeckColors(d)), TF_NARROW);
    G_Text(6, 64, problem ? 0xa03020 : 0x306030, G_NO_SHADOW, problem ? problem : "Ready to play.", 92, TF_NARROW);
    CardRows(1);
    if (U.cursor < U.listCount)
        CardStrip(U.list[U.cursor]);
    TcgUi_Hints("{LEFT}{RIGHT} TAKE OUT / PUT IN  {B} DONE");
}

// ---- the binder
static void BinderScreen(void)
{
    char line[64];
    TcgUi_Background();
    TcgUi_SidePanel("BINDER", MTG_THEME);
    snprintf(line, sizeof(line), "%d CARDS", U.listCount);
    G_TextAlign(52, 30, TA_CENTER, G_INK, G_INK_SHADOW, line, TF_SMALL);
    G_Text(8, 48, 0x605040, G_NO_SHADOW, "Gold names are rare, grey-blue uncommon.", 90, TF_NARROW);
    CardRows(0);
    if (U.cursor < U.listCount)
        CardStrip(U.list[U.cursor]);
    TcgUi_Hints("{A} LOOK  {B} BACK");
}

// ---- a card's whole text
static void InspectScreen(void)
{
    const MtgCardDef *c = Mtg_Card(U.inspectCard);
    char name[64], text[700], line[128], cost[16];
    static const char *const sTypes[] = {"Land", "Creature", "Artifact", "Enchantment", "Instant", "Sorcery",
                                         "Planeswalker", "Tribal"};
    static const char *const sRarity[] = {"COMMON", "UNCOMMON", "RARE"};
    unsigned n = 0, i;
    TcgUi_Background();
    G_Window(6, 4, 228, 140);
    PlainName(name, sizeof(name), U.inspectCard);
    G_Text(14, 9, RarityColor(U.inspectCard), G_INK_SHADOW, name, 170, TF_NORMAL);
    CostText(cost, sizeof(cost), &c->cost);
    if (!(c->types & MTG_T_LAND))
        G_TextAlign(226, 10, TA_RIGHT, G_INK, G_INK_SHADOW, cost, TF_NORMAL);
    line[0] = 0;
    if (c->supertypes & MTG_ST_LEGENDARY)
        n += (unsigned)snprintf(line + n, sizeof(line) - n, "Legendary ");
    if (c->supertypes & MTG_ST_BASIC)
        n += (unsigned)snprintf(line + n, sizeof(line) - n, "Basic ");
    for (i = 0; i < 8; i++)
        if (c->types & (1u << i))
            n += (unsigned)snprintf(line + n, sizeof(line) - n, "%s ", sTypes[i]);
    if (c->subtypes[0] && n < sizeof(line))
        snprintf(line + n, sizeof(line) - n, "- %s", c->subtypes);
    Mtg_PlainText(text, sizeof(text), line);
    G_Text(14, 25, 0x506070, G_NO_SHADOW, text, 210, TF_NARROW);
    Mtg_PlainText(text, sizeof(text), c->oracle);
    G_Text(14, 38, G_INK, G_NO_SHADOW, text, 212, TF_NARROW);
    snprintf(line, sizeof(line), "%s  OWNED %d", sRarity[c->rarity < 3 ? c->rarity : 0], C()->owned[U.inspectCard]);
    G_Text(14, 128, 0x606870, G_NO_SHADOW, line, 0, TF_NARROW);
    if (c->types & MTG_T_CREATURE) {
        snprintf(line, sizeof(line), "%d/%d", c->power, c->toughness);
        G_TextAlign(226, 126, TA_RIGHT, G_INK, G_INK_SHADOW, line, TF_NORMAL);
    }
    TcgUi_Hints("{B} BACK");
}

// ---- the CARD SHOP's starters
static void StarterScreen(void)
{
    int i;
    char line[200];
    TcgUi_Background();
    TcgUi_SidePanel("STARTER", MTG_THEME);
    CardBack(34, 30, 36, 50);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < MTG_STARTERS; i++)
        TcgUi_Row(LIST_X + 2, 6 + i * 14, LIST_W - 4, U.cursor == i, 1, gMtgStarters[i].name,
                  ColorWords(gMtgStarters[i].colors));
    snprintf(line, sizeof(line), "%s\n40 cards. Pick the one you like: it's yours to keep.", gMtgStarters[U.cursor].about);
    Strip(line);
    TcgUi_Hints("{A} TAKE IT  {B} NOT NOW");
}

// ---- a duelist's challenge
static int UsableDeck(int k)
{
    return C()->decks[k].count && !Mtg_DeckProblem(C(), &C()->decks[k]);
}

static void ChallengeScreen(void)
{
    const MtgDuelist *who = &gMtgDuelists[U.duelist];
    MtgCollection *c = C();
    char line[200];
    TcgUi_Background();
    TcgUi_SidePanel(who->name, MTG_THEME);
    CardBack(34, 24, 36, 50);
    G_Text(6, 80, 0x605040, G_NO_SHADOW, who->title, 92, TF_NARROW);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    G_Text(LIST_X + 6, 8, G_INK, G_NO_SHADOW, who->hello, LIST_W - 12, TF_NARROW);
    snprintf(line, sizeof(line), "YOUR DECK: %s", UsableDeck(c->activeDeck) ? c->decks[c->activeDeck].name : "none");
    G_Text(LIST_X + 6, 74, 0x305030, G_NO_SHADOW, line, LIST_W - 12, TF_NARROW);
    G_Text(LIST_X + 6, 86, 0x605040, G_NO_SHADOW, ColorWords(Mtg_DeckColors(&c->decks[c->activeDeck])), LIST_W - 12,
           TF_NARROW);
    Strip(c->beaten & (1u << U.duelist) ? "You've beaten them before. A win brings a booster pack every time."
                                        : "A win brings a booster pack: eight cards for your BINDER.");
    TcgUi_Hints("{A} DUEL  {L}{R} OTHER DECK  {B} NOT NOW");
}

// ---- the booster a win brought
static void SpoilsScreen(void)
{
    int i;
    TcgUi_Background();
    TcgUi_SidePanel("BOOSTER", MTG_THEME);
    CardBack(34, 30, 36, 50);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < U.boosterCount; i++) {
        char name[64];
        PlainName(name, sizeof(name), U.booster[i]);
        TcgUi_FitText(name, LIST_W - 16, TF_NARROW);
        if (i == U.cursor)
            G_FillA(LIST_X + 3, 6 + i * 12, LIST_W - 6, 12, 0xf8e8a0, 255);
        G_Text(LIST_X + 6, 7 + i * 12, RarityColor(U.booster[i]), G_NO_SHADOW, name, 0, TF_NARROW);
    }
    if (U.cursor < U.boosterCount)
        CardStrip(U.booster[U.cursor]);
    TcgUi_Hints("{UP}{DOWN} LOOK  {A} DONE");
}

void MtgUi_Draw(void)
{
    switch (U.screen) {
    case M_HOME: TcgUi_RowDown(1), HomeScreen(), TcgUi_RowDown(0); break;
    case M_DECKS: TcgUi_RowDown(1), DecksScreen(), TcgUi_RowDown(0); break;
    case M_BUILDER: BuilderScreen(); break;
    case M_BINDER: BinderScreen(); break;
    case M_INSPECT: InspectScreen(); break;
    case M_STARTER: TcgUi_RowDown(1), StarterScreen(), TcgUi_RowDown(0); break;
    case M_CHALLENGE: ChallengeScreen(); break;
    case M_DUEL: MtgBoard_Draw(); break;
    case M_SPOILS: SpoilsScreen(); break;
    default: break;
    }
    TcgUi_MessageBox(U.message, U.asking, U.askYes);
}

// ---- opening from the map
void MtgUi_OpenStarter(void)
{
    Mtg_EnsureDb();
    U.returnToGame = NULL;
    Open(M_STARTER);
    U.fromScript = 1;
    MPNet_Log("mtg: starter screen");
}

void MtgUi_OpenChallenge(unsigned duelist)
{
    MtgCollection *c = C();
    int k;
    U.returnToGame = NULL;
    U.duelist = (int)(duelist < MTG_DUELISTS ? duelist : 0);
    Open(M_CHALLENGE);
    U.fromScript = 1;
    if (!UsableDeck(c->activeDeck))
        for (k = 0; k < MTG_DECKS; k++)
            if (UsableDeck(k)) {
                c->activeDeck = (uint8_t)k;
                break;
            }
    MPNet_Log("mtg: challenge from %s", gMtgDuelists[U.duelist].name);
}

static void StartDuel(void)
{
    MtgCollection *c = C();
    MtgSavedDeck theirs;
    MtgRecipe r;
    static MtgDeck mine, them;
    r.name = gMtgDuelists[U.duelist].name;
    r.about = "";
    r.colors = 0;
    r.cards = gMtgDuelists[U.duelist].cards;
    Mtg_RecipeDeck(&r, &theirs);
    Mtg_DeckToDuel(&c->decks[c->activeDeck], &mine);
    Mtg_DeckToDuel(&theirs, &them);
    Screen(M_DUEL);
    MtgBoard_Start(&mine, &them, Mtg_Rand(&c->rng), gMtgDuelists[U.duelist].name);
}

static void DuelOver(int result)
{
    MtgCollection *c = C();
    const MtgDuelist *who = &gMtgDuelists[U.duelist];
    if (result == 1) {
        c->wins++;
        c->beaten |= 1u << U.duelist;
        U.boosterCount = Mtg_OpenBooster(c, U.booster);
        MtgBoard_EndHoldMusic();
        Screen(M_SPOILS);
        MPNet_Log("mtg: won against %s, booster of %d", who->name, U.boosterCount);
        Say(ACT_NONE, "%s: \"%s\"", who->name, who->lose);
    } else {
        if (result == 2)
            c->losses++;
        MtgBoard_End();
        MPNet_Log("mtg: %s against %s", result == 2 ? "lost" : "a draw", who->name);
        Screen(M_CHALLENGE);
        Say(ACT_CLOSE, "%s: \"%s\"", who->name, result == 2 ? who->win : "A draw! Let's play again some time.");
    }
}

void MtgUi_DebugAutoplay(int on) { MtgBoard_Autoplay(on); }
void MtgUi_DebugEnd(int win) { MtgBoard_ForceEnd(win); }

static void Act(int action)
{
    MtgCollection *c = C();
    switch (action) {
    case ACT_CLOSE:
        Close();
        break;
    case ACT_HOME:
        Screen(M_HOME);
        break;
    case ACT_TAKE_STARTER:
        Mtg_GiveStarter(c, (unsigned)U.askArg);
        Tcg_GrantCase();
        MPNet_Log("mtg: starter %s chosen", gMtgStarters[c->starter].name);
        Say(ACT_CLOSE, "The %s deck is yours! MAGIC is on your CARD CASE now: L and R go round its games.",
            gMtgStarters[c->starter].name);
        break;
    case ACT_DELETE:
        memset(&c->decks[U.askArg], 0, sizeof(c->decks[0]));
        if (c->activeDeck == U.askArg)
            for (c->activeDeck = 0; c->activeDeck < MTG_DECKS - 1 && !c->decks[c->activeDeck].count; c->activeDeck++)
                ;
        break;
    case ACT_HOWTO:
        if (++U.howto < 5) {
            static const char *const sPages[5] = {
                "Each player starts with 20 life. Bring the other player's life to 0 to win (or let them run out "
                "of cards to draw).",
                "LANDS make mana. Play one land each turn, then tap lands to pay for spells: a spell's cost is on "
                "its card, (R) red mana, (2) any two.",
                "CREATURES fight. They can't attack the turn they arrive. Attack: the other player blocks with "
                "their creatures, or takes the damage.",
                "INSTANTS can be cast any time you could act, even in the other player's turn: answer a spell "
                "before it happens!",
                "{A} plays a card, {START} passes. Hold {L} to stop at every chance to act. {SELECT} shows a "
                "card's whole text."};
            Say(ACT_HOWTO, "%s", sPages[U.howto]);
        }
        break;
    }
}

static void StepBuilder(unsigned pressed)
{
    MtgCollection *c = C();
    MtgSavedDeck *d = &c->decks[U.deck];
    int card = U.cursor < U.listCount ? U.list[U.cursor] : -1;
    MoveCursor(pressed, U.listCount);
    if (pressed & (KEY_L | KEY_R)) {   // a page at a time
        U.cursor = pressed & KEY_L ? (U.cursor > LIST_ROWS ? U.cursor - LIST_ROWS : 0)
                                   : (U.cursor + LIST_ROWS < U.listCount ? U.cursor + LIST_ROWS : U.listCount - 1);
        MoveCursor(0, U.listCount);
        PlaySE(SE_SELECT);
    }
    if (card < 0)
        return;
    if (pressed & (KEY_RIGHT | KEY_A)) {
        int copies = Mtg_DeckCopies(d, card);
        if (d->count >= MTG_DECK_MAX || (!Mtg_IsBasicLand(card) && (copies >= MTG_COPIES || copies >= c->owned[card])))
            PlaySE(SE_FAILURE);
        else {
            Mtg_DeckAdd(d, card);
            PlaySE(SE_SELECT);
        }
    }
    if (pressed & KEY_LEFT) {
        if (Mtg_DeckCopies(d, card)) {
            Mtg_DeckRemove(d, card);
            PlaySE(SE_SELECT);
        } else
            PlaySE(SE_FAILURE);
    }
    if (pressed & KEY_INSPECT) {
        U.inspectCard = card;
        U.back = M_BUILDER;
        U.screen = M_INSPECT;
        PlaySE(SE_SELECT);
    }
    if (pressed & KEY_B) {
        Mtg_DeckSort(d);
        PlaySE(SE_SELECT);
        Screen(M_DECKS);
        U.cursor = U.deck;
        if (Mtg_DeckProblem(c, d))
            Say(ACT_NONE, "This deck can't be played yet: %s", Mtg_DeckProblem(c, d));
    }
}

int MtgUi_Step(unsigned keys)
{
    MtgCollection *c = C();
    unsigned pressed = keys & ~U.previousKeys;
    U.previousKeys = keys;
    if (!MtgUi_Active())
        return 0;
    U.age++;
    U.frame++;
    if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
        if (++U.held > 18 && U.held % 4 == 0 && U.screen != M_DUEL)
            pressed |= keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
    } else
        U.held = 0;
    if (U.screen == M_DUEL && !U.message[0]) {
        int result = MtgBoard_Step(pressed, keys);
        if (result)
            DuelOver(result);
        return 1;
    }
    if (U.message[0]) {
        if (U.asking) {
            if (pressed & (KEY_UP | KEY_DOWN))
                U.askYes ^= 1, PlaySE(SE_SELECT);
            if (pressed & (KEY_A | KEY_B)) {
                int yes = (pressed & KEY_A) && U.askYes;
                U.message[0] = 0;
                U.asking = 0;
                PlaySE(SE_SELECT);
                if (yes)
                    Act(U.askAction);
            }
        } else if (pressed & (KEY_A | KEY_B)) {
            int then = U.messageThen;
            U.message[0] = 0;
            PlaySE(SE_SELECT);
            Act(then);
        }
        return 1;
    }
    switch (U.screen) {
    case M_HOME:
        MoveCursor(pressed, HOME_ROWS);
        if (pressed & (KEY_L | KEY_R)) {
            PlaySE(SE_SELECT);
            ToSide((pressed & KEY_R) != 0);
        } else if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            switch (U.cursor) {
            case 0:
                Screen(M_DECKS);
                U.cursor = c->activeDeck;
                break;
            case 1:
                BuildList(0);
                Screen(M_BINDER);
                break;
            case 2:
                U.howto = -1;
                Act(ACT_HOWTO);
                break;
            default:
                Close();
                break;
            }
        }
        break;
    case M_DECKS:
        if (U.menuOn) {
            int filled = c->decks[U.cursor].count != 0, n = filled ? DM_COUNT : 1;
            U.menuCursor = TcgUi_PopMenuMove(pressed, U.menuCursor, n, 0);
            if (pressed & KEY_B)
                U.menuOn = 0, PlaySE(SE_SELECT);
            else if (pressed & KEY_A) {
                int k = U.cursor, j;
                U.menuOn = 0;
                PlaySE(SE_SELECT);
                if (!filled) {   // a new deck here
                    snprintf(c->decks[k].name, MTG_DECK_NAME, "DECK %d", k + 1);
                    U.deck = k;
                    BuildList(1);
                    Screen(M_BUILDER);
                    break;
                }
                switch (U.menuCursor) {
                case DM_USE:
                    if (Mtg_DeckProblem(c, &c->decks[k]))
                        Say(ACT_NONE, "%s", Mtg_DeckProblem(c, &c->decks[k]));
                    else
                        c->activeDeck = (uint8_t)k;
                    break;
                case DM_EDIT:
                    U.deck = k;
                    BuildList(1);
                    Screen(M_BUILDER);
                    break;
                case DM_COPY:
                    for (j = 0; j < MTG_DECKS && c->decks[j].count; j++)
                        ;
                    if (j == MTG_DECKS)
                        Say(ACT_NONE, "Every deck slot is taken.");
                    else {
                        c->decks[j] = c->decks[k];
                        snprintf(c->decks[j].name, MTG_DECK_NAME, "COPY %d", j + 1);
                        U.cursor = j;
                    }
                    break;
                case DM_DELETE:
                    Ask(ACT_DELETE, k, "Delete %s? The cards stay in your BINDER.", c->decks[k].name);
                    break;
                }
            }
            break;
        }
        MoveCursor(pressed, MTG_DECKS);
        if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Screen(M_HOME);
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            U.menuOn = 1;
            U.menuCursor = 0;
        }
        break;
    case M_BUILDER:
        StepBuilder(pressed);
        break;
    case M_BINDER:
        MoveCursor(pressed, U.listCount);
        if ((pressed & (KEY_A | KEY_INSPECT)) && U.cursor < U.listCount) {
            U.inspectCard = U.list[U.cursor];
            U.back = M_BINDER;
            U.screen = M_INSPECT;
            PlaySE(SE_SELECT);
        } else if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Screen(M_HOME);
            U.cursor = 1;
        }
        break;
    case M_INSPECT:
        if (pressed & (KEY_A | KEY_B | KEY_INSPECT)) {
            PlaySE(SE_SELECT);
            U.screen = U.back;
        }
        break;
    case M_STARTER:
        MoveCursor(pressed, MTG_STARTERS);
        if (pressed & KEY_B) {   // (not now: the clerk will ask again)
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            Ask(ACT_TAKE_STARTER, U.cursor, "Take the %s deck? (%s)", gMtgStarters[U.cursor].name,
                ColorWords(gMtgStarters[U.cursor].colors));
        }
        break;
    case M_CHALLENGE:
        if (pressed & (KEY_L | KEY_R)) {   // another usable deck
            int k, step = pressed & KEY_L ? MTG_DECKS - 1 : 1;
            for (k = (c->activeDeck + step) % MTG_DECKS; k != c->activeDeck; k = (k + step) % MTG_DECKS)
                if (UsableDeck(k)) {
                    c->activeDeck = (uint8_t)k;
                    PlaySE(SE_SELECT);
                    break;
                }
        } else if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            if (!UsableDeck(c->activeDeck))
                Say(ACT_NONE, "None of your decks can be played: open MAGIC on your CARD CASE to fix one.");
            else
                StartDuel();
        }
        break;
    case M_SPOILS:
        MoveCursor(pressed, U.boosterCount);
        if (pressed & (KEY_A | KEY_B)) {
            PlaySE(SE_SELECT);
            Close();
        }
        break;
    }
    return 1;
}
