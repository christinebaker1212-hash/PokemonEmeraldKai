// Moemon Kai: the CARD CASE's MAGIC side (hidden until the player has a deck: docs/MTG_DECISIONS.md) and the
// screens around a duel, each laid out, worded and keyed as the POKeMON and YU-GI-OH! sides' own (tcg_ui.c,
// ygo_ui.c, with the kit of tcg_ui_kit.h): HOME (the deck's ACE CARD floating; SELECT: HOW TO PLAY), DECKS (the
// slots, DECK RECIPES, BACK; EDIT / RENAME / ACE CARD / DUEL WITH IT / COPY / DELETE), the recipes (the starter decks
// built again from the cards owned), ACE CARD, the deck builder (a grid of the basic lands and the owned cards beside
// the deck's list), the BINDER (a grid of faces by color, SELECT: every card, NEW on new ones), a card's view (the big
// face and its words, A for the big picture), MOSSDEEP's CARD SHOP (the starter decks), a duelist's challenge (their
// picture, best card, DUEL WITH a deck), the table (mtg_board.c), the booster a win brings, opened card by card.
// Faces by mtg_gfx.c.
#include "mp_version.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "mp.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg/integration.h"
#include "tcg/tcg.h"
#include "mtg/integration.h"
#include "mtg_internal.h"
#include "mtg_board.h"
#include "mtg_gfx.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_foil.h"
#include "../tcg/tcg_guide.h"
#include "../tcg/tcg_new.h"
#include "../tcg/tcg_ui_kit.h"

enum
{
    M_CLOSED, M_HOME, M_DECKS, M_RECIPES, M_ACE, M_BUILDER, M_BINDER, M_INSPECT, M_ART, M_STARTER, M_CHALLENGE, M_DUEL,
    M_SPOILS, M_PACKS, M_SHOP
};
enum
{
    KEY_A = 1, KEY_B = 2, KEY_SELECT = 4, KEY_START = 8, KEY_RIGHT = 16, KEY_LEFT = 32, KEY_UP = 64, KEY_DOWN = 128,
    KEY_R = 256, KEY_L = 512,
    KEY_INSPECT = 2048
};
enum { ACT_NONE, ACT_CLOSE, ACT_HOME, ACT_TAKE_STARTER, ACT_CLEAR, ACT_COPY, ACT_RECIPE, ACT_BUY_PACK };
enum { VS_NPC, VS_PRACTICE };

#define PANEL_W KIT_PANEL_W
#define STRIP_Y KIT_STRIP_Y
#define LIST_X KIT_LIST_X
#define LIST_W KIT_LIST_W
#define ROW_H KIT_ROW_H
#define MTG_THEME 0x7a4a2a         // the case's MAGIC side: warm brown, with gold
#define MTG_GOLD 0xe0b050
#define FILTERS 9                  // the five colors, colorless, more than one color, lands; and (the binder) all
#define REVEAL_TEAR 26

static struct
{
    int screen, back, cursor, scroll, frame, age;
    unsigned previousKeys, held;
    char message[320];
    int messageThen, asking, askYes, askAction, askArg;
    int fromScript;
    void (*returnToGame)(void);
    int menuOn, menuCursor;
    int deck, recipeSlot;            // the deck being looked at or built; where a recipe goes (-1 the first empty)
    uint16_t list[MTG_COLLECTION_CARDS];
    int listCount, withBasics, filter, showAll;
    int side, deckCursor;            // the builder: 0 the cards, 1 the deck's list
    int inspectCard, inspectScroll, inspectMax;
    int zoom, panX, panY;            // the big picture
    int duelist, vs;
    uint32_t portrait[64 * 64];      // the duelist as they faced you (their map sprite)
    uint8_t portraitW, portraitH;
    SDL_Texture *portraitTexture;
    uint16_t booster[MTG_PACK_MAX];
    uint8_t boosterRarity[MTG_PACK_MAX];   // each card's rarity in its set
    int boosterCount, flipped, revealAge, revealNext, pack;   // pack: the set being opened
    int rewarded;                    // the duel's end counted (its words on the result screen)
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
    if (screen != M_INSPECT && screen != M_ART)
        MtgBoard_ReleaseMusic();   // the victory song ends with the result
    U.screen = screen;
    U.cursor = 0;
    U.scroll = 0;
    U.age = 0;
    U.menuOn = 0;
}
int MtgUi_Active(void) { return U.screen != M_CLOSED; }
unsigned MtgUi_DuelingNow(void) { return U.screen == M_DUEL; }

static void ForgetPortrait(void)
{
    if (U.portraitTexture)
        SDL_DestroyTexture(U.portraitTexture);
    U.portraitTexture = NULL;
}

static void Close(void)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    if (U.screen == M_DUEL)
        MtgBoard_End();
    U.message[0] = 0;
    U.asking = 0;
    Screen(M_CLOSED);
    MtgBoard_ReleaseMusic();
    ForgetPortrait();
    MtgGfx_Forget();
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

// L the side before, R the one after (POKeMON, YU-GI-OH!, MAGIC, TRIPLE TRIAD: TRIPLE TRIAD always last)
static void ToSide(int right)
{
    void (*callback)(void) = U.returnToGame;
    int script = U.fromScript;
    U.returnToGame = NULL;
    U.fromScript = 0;
    U.message[0] = 0;
    Screen(M_CLOSED);
    MtgGfx_Forget();
    C()->flags &= ~MTG_FLAG_CASE_SIDE;
    Tcg_OpenCaseGame(right ? KIT_GAME_TRIAD : KIT_GAME_YUGIOH, callback, script);
}

// ---- helpers
static void PlainName(char *out, unsigned cap, int card)
{
    Mtg_PlainText(out, cap, Mtg_Card(card)->name);
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

// a mana symbol: the POKeMON side's Energy orbs' shape (G_EnergyOrb), in the mana's color
static void ManaOrb(int x, int y, int kind)
{
    unsigned base = MtgGfx_ManaColor(kind), light = G_Mix(base, 0xffffff, 0.5f), dark = G_Mix(base, 0x000000, 0.35f);
    G_Fill(x + 2, y, 3, 1, 0x202020);
    G_Fill(x + 1, y + 1, 5, 5, base);
    G_Fill(x, y + 2, 7, 3, base);
    G_Fill(x + 1, y + 1, 1, 1, 0x202020);
    G_Fill(x + 5, y + 1, 1, 1, 0x202020);
    G_Fill(x, y + 2, 1, 3, 0x202020);
    G_Fill(x + 6, y + 2, 1, 3, 0x202020);
    G_Fill(x + 1, y + 5, 1, 1, 0x202020);
    G_Fill(x + 5, y + 5, 1, 1, 0x202020);
    G_Fill(x + 2, y + 6, 3, 1, 0x202020);
    G_Fill(x + 4, y + 4, 2, 1, dark);
    G_Fill(x + 2, y + 2, 2, 1, light);
    G_Fill(x + 2, y + 3, 1, 1, light);
}
// a deck's colors as orbs from x (as the POKeMON DECKS page shows a deck's types: TypeOrbs)
static void ColorOrbs(int x, int y, uint8_t colors)
{
    int k;
    for (k = 0; k < 5; k++)
        if (colors & (1u << k)) {
            ManaOrb(x, y, k);
            x += 8;
        }
}

static const char *RarityWord(int card)
{
    static const char *const words[] = {"COMMON", "UNCOMMON", "RARE"};
    int r = Mtg_Card(card)->rarity;
    return words[r >= 0 && r < 3 ? r : 0];
}
static unsigned RarityInk(int card)
{
    int r = Mtg_Card(card)->rarity;
    return r == MTG_RARITY_RARE ? 0xf8d060 : r == MTG_RARITY_UNCOMMON ? 0xd0dcf0 : 0xf0f0f0;
}

static void TypeLine(char *out, unsigned cap, int card)
{
    static const char *const sTypes[] = {"Land", "Creature", "Artifact", "Enchantment", "Instant", "Sorcery",
                                         "Planeswalker", "Tribal"};
    const MtgCardDef *c = Mtg_Card(card);
    char line[160];
    unsigned n = 0, i;
    line[0] = 0;
    if (c->supertypes & MTG_ST_LEGENDARY)
        n += (unsigned)snprintf(line + n, sizeof(line) - n, "Legendary ");
    if (c->supertypes & MTG_ST_BASIC)
        n += (unsigned)snprintf(line + n, sizeof(line) - n, "Basic ");
    if (c->supertypes & MTG_ST_SNOW)
        n += (unsigned)snprintf(line + n, sizeof(line) - n, "Snow ");
    for (i = 0; i < 8; i++)
        if (c->types & (1u << i))
            n += (unsigned)snprintf(line + n, sizeof(line) - n, "%s ", sTypes[i]);
    if (c->subtypes[0] && n < sizeof(line))
        snprintf(line + n, sizeof(line) - n, "- %s", c->subtypes);
    Mtg_PlainText(out, cap, line);
}

static void RecipeOf(int k, MtgSavedDeck *d)
{
    MtgRecipe r = gMtgStarters[k];
    Mtg_RecipeDeck(&r, d);
}
static int RecipeAce(int k)
{
    MtgSavedDeck d;
    RecipeOf(k, &d);
    return Mtg_AutoAce(d.cards, d.count);
}
static int DuelistAce(const char *const *lines)
{
    MtgRecipe r;
    MtgSavedDeck d;
    memset(&r, 0, sizeof(r));
    r.name = "";
    r.about = "";
    r.cards = lines;
    Mtg_RecipeDeck(&r, &d);
    return Mtg_AutoAce(d.cards, d.count);
}
// what of a recipe the player has (basic lands are always there); returns how many cards it holds
static int RecipeOwned(int k, int *have)
{
    MtgSavedDeck d;
    uint8_t used[MTG_COLLECTION_CARDS];
    int i;
    RecipeOf(k, &d);
    memset(used, 0, sizeof(used));
    *have = 0;
    for (i = 0; i < d.count; i++)
        if (Mtg_IsBasicLand(d.cards[i]) || used[d.cards[i]]++ < C()->owned[d.cards[i]])
            ++*have;
    return d.count;
}

static void Foil(int card, int x, int y, int w, int h, unsigned phase)
{
    int ax, ay, aw, ah;
    if (card < 0 || Mtg_Card(card)->rarity != MTG_RARITY_RARE)
        return;
    MtgGfx_ArtRect(card, w, h, &ax, &ay, &aw, &ah);
    G_Foil(FOIL_GLINT, x, y, w, h, x + ax, y + ay, aw, ah, (unsigned)(x * 7 + y * 13), phase);
}

static void Strip(const char *text)
{
    TcgUi_Strip(NULL, 0, text);
}

static void BigCard(int card, int x, int y, int bob)
{
    G_FillA(x + 2, y + 3, 44, 61, 0x000000, 70);
    MtgGfx_DrawCard(card, (float)x, (float)(y - bob), 44, 61, 0, 255, 0);
    Foil(card, x, y - bob, 44, 61, (unsigned)U.frame);
}

static int DeckValid(int k) { return C()->decks[k].count && !Mtg_DeckProblem(C(), &C()->decks[k]); }
static int SlotEmpty(int k) { return !C()->decks[k].count; }
static int EmptySlot(void)
{
    int k;
    for (k = 0; k < MTG_DECKS; k++)
        if (SlotEmpty(k))
            return k;
    return MTG_DECKS;
}

// up and down a list (with wrap, as the other games' lists); held, a page at a time is the d-pad's repeat
static void MoveCursor(unsigned pressed, int count)
{
    if (!count)
        return;
    if (pressed & KEY_UP)
        U.cursor = (U.cursor + count - 1) % count, PlaySE(SE_SELECT);
    if (pressed & KEY_DOWN)
        U.cursor = (U.cursor + 1) % count, PlaySE(SE_SELECT);
}

// the d-pad over a grid of cols
static void GridMove(unsigned pressed, int count, int cols)
{
    int c = U.cursor;
    if (!count)
        return;
    if (pressed & KEY_RIGHT && c + 1 < count)
        c++;
    if (pressed & KEY_LEFT && c > 0)
        c--;
    if (pressed & KEY_DOWN)
        c = c + cols < count ? c + cols : count - 1;
    if (pressed & KEY_UP)
        c = c - cols >= 0 ? c - cols : 0;
    if (c != U.cursor)
        PlaySE(SE_SELECT);
    U.cursor = c;
}

// ---- lists of cards: the binder (owned, or every card) and the builder (the five basic lands, then the owned)
static const char *const sFilterNames[FILTERS] = {"WHITE", "BLUE", "BLACK", "RED", "GREEN", "COLORLESS", "MULTICOLOR",
                                                  "LANDS", "ALL"};
static int InFilter(int card)
{
    const MtgCardDef *c = Mtg_Card(card);
    int land = (c->types & MTG_T_LAND) != 0;
    switch (U.filter) {
    case 0: case 1: case 2: case 3: case 4: return !land && c->colors == MTG_COLOR_BIT(U.filter);
    case 5: return !land && !c->colors;
    case 6: return !land && (c->colors & (c->colors - 1));
    case 7: return land;
    default: return 1;
    }
}

static int ListOrder(const void *a, const void *b)
{
    const MtgCardDef *x = Mtg_Card(*(const uint16_t *)a), *y = Mtg_Card(*(const uint16_t *)b);
    int bx = Mtg_IsBasicLand(*(const uint16_t *)a), by = Mtg_IsBasicLand(*(const uint16_t *)b), cx, cy;
    if (bx != by)
        return by - bx;   // the basic lands first
    cx = x->types & MTG_T_LAND ? 8 : x->colors ? (x->colors & (x->colors - 1) ? 7 : __builtin_ctz(x->colors)) : 6;
    cy = y->types & MTG_T_LAND ? 8 : y->colors ? (y->colors & (y->colors - 1) ? 7 : __builtin_ctz(y->colors)) : 6;
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
    int i, first;
    U.withBasics = withBasics;
    U.listCount = 0;
    if (withBasics)
        for (i = 0; i < 5; i++)
            if (InFilter(Mtg_CardByName(basics[i])))
                U.list[U.listCount++] = (uint16_t)Mtg_CardByName(basics[i]);
    first = U.listCount;
    for (i = 0; i < Mtg_DbCount() && i < MTG_COLLECTION_CARDS; i++)
        if ((c->owned[i] || (U.showAll && !withBasics)) && Mtg_CanUse(i) && !(withBasics && Mtg_IsBasicLand(i))
            && InFilter(i))
            U.list[U.listCount++] = (uint16_t)i;
    qsort(U.list + first, (size_t)(U.listCount - first), sizeof(U.list[0]), ListOrder);
    if (U.cursor >= U.listCount)
        U.cursor = U.listCount ? U.listCount - 1 : 0;
}

static void OwnedCounts(int *have, int *total)
{
    MtgCollection *c = C();
    int i;
    *have = *total = 0;
    for (i = 0; i < Mtg_DbCount() && i < MTG_COLLECTION_CARDS; i++)
        if (Mtg_CanUse(i) && !Mtg_IsBasicLand(i)) {
            ++*total;
            *have += c->owned[i] != 0;
        }
}

/* The chips, as the POKeMON side's (tcg_ui.c Tabs): a tab each, the chosen one white with the red frame; a mana orb
 * in the five colors', C / M / L / * for colorless, more than one color, lands and every card */
static void Chips(int x, int y, int count)
{
    static const char *const letters[FILTERS] = {"", "", "", "", "", "C", "M", "L", "*"};
    int i;
    for (i = 0; i < count; i++) {
        int tx = x + i * 14, on = i == U.filter;
        G_RoundFill(tx, y + (on ? 0 : 1), 13, on ? 13 : 12, on ? 0xf8f8f8 : 0x405060, 255);
        G_RoundFrame(tx, y + (on ? 0 : 1), 13, on ? 13 : 12, on ? 0xe82820 : 0x202830, 255);
        if (i < 5)
            ManaOrb(tx + 3, y + 3, i);
        else
            G_TextAlign(tx + 7, y, TA_CENTER, on ? G_INK : 0xe0e8f0, G_NO_SHADOW, letters[i], TF_NARROW);
    }
}

static void CursorFrame(int x, int y, int w, int h)
{
    int o = (U.frame / 8) % 2;
    G_Frame(x - 2 - o, y - 2 - o, w + 4 + 2 * o, h + 4 + 2 * o, 0xe82820, 255);
}

// ---- HOME
enum { HOME_DECKS, HOME_BINDER, HOME_PACKS, HOME_PRACTICE, HOME_SPEED, HOME_CLOSE, HOME_ROWS };
static const char *const sHomeRows[HOME_ROWS] = {"DECKS", "BINDER", "BOOSTER PACKS", "PRACTICE DUEL", "DUEL SPEED",
                                                 "CLOSE CASE"};
static const char *const sHomeHelp[HOME_ROWS] = {
    "Build decks and pick the one you duel with.", "Every card you own.", "Open the boosters you won.",
    "Against a starter deck. Nothing is won or lost.", "How quickly the cards move on the table.",
    "Put the CARD CASE away."};

static void HomeScreen(void)
{
    MtgCollection *c = C();
    const MtgSavedDeck *d = &c->decks[c->activeDeck];
    char line[160];
    int i, bob = (int)(sinf(U.frame * 0.06f) * 1.5f), have, total;
    TcgUi_Background();
    TcgUi_CaseTabs(KIT_GAME_MAGIC, MTG_THEME);
    if (d->count)
        BigCard(Mtg_DeckAce(c, c->activeDeck), 30, 22, bob);
    G_TextAlign(52, 86, TA_CENTER, G_INK, G_INK_SHADOW, d->count && d->name[0] ? d->name : "No deck", TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < HOME_ROWS; i++)
        TcgUi_Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, U.cursor == i, 1, sHomeRows[i],
                  i == HOME_SPEED ? (Tcg_GetCollection()->fastAnimations ? "FAST" : "NORMAL") : NULL);
    OwnedCounts(&have, &total);
    snprintf(line, sizeof(line), "%s\nCARDS %d/%d   WON %u  LOST %u", sHomeHelp[U.cursor], have, total, c->wins,
             c->losses);
    Strip(line);
    TcgUi_Hints("{SELECT} HOW TO PLAY  {L}{R} OTHER GAMES");
}

// ---- DECKS: the slots, DECK RECIPES, BACK
enum { DM_EDIT, DM_RENAME, DM_ACE, DM_USE, DM_COPY, DM_DELETE, DM_TRAY, DM_COUNT };
enum { EM_NEW, EM_RECIPE, EM_COUNT };
static const char *const sDeckMenu[DM_COUNT] = {"EDIT", "RENAME", "ACE CARD", "DUEL WITH IT", "COPY", "DELETE",
                                                 "TRAY COLOR"};
/* The hand tray's colors (TcgUi_Tray*): the deck's main color counts its cards' colors and its lands' mana */
static const unsigned sTrayRgb[6] = {0xd8c890, 0x3878d0, 0x5a5266, 0xd04830, 0x3a9a4a, 0x9aa4ac};
static const char *const sTrayNames[6] = {"WHITE", "BLUE", "BLACK", "RED", "GREEN", "COLORLESS"};
static unsigned DeckMainRgb(const MtgSavedDeck *d)
{
    int count[5] = {0, 0, 0, 0, 0}, i, k, best = -1, top = 0;
    for (i = 0; i < d->count; i++) {
        const MtgCardDef *c = Mtg_Card(d->cards[i]);
        uint8_t colors = c->types & MTG_T_LAND ? c->intrinsicMana : c->colors;
        for (k = 0; k < 5; k++)
            if (colors & MTG_COLOR_BIT(k))
                count[k]++;
    }
    for (k = 0; k < 5; k++)   // (the most; a tie goes to the first in W U B R G)
        if (count[k] > top)
            top = count[k], best = k;
    return sTrayRgb[best < 0 ? 5 : best];
}
static unsigned DeckTrayRgb(int k)
{
    return TcgUi_TrayRgb(C()->tray[k], sTrayRgb, 6, DeckMainRgb(&C()->decks[k]));
}
static const char *const sEmptyMenu[EM_COUNT] = {"BUILD A DECK", "FROM A RECIPE"};
#define DECK_ROWS (MTG_DECKS + 2)
#define DECK_ROW_RECIPES MTG_DECKS
#define DECK_ROW_BACK (MTG_DECKS + 1)
static unsigned DeckMenuOff(int d) { return d == C()->activeDeck || !DeckValid(d) ? 1u << DM_USE : 0; }

static void DecksScreen(void)
{
    MtgCollection *c = C();
    int i, d = U.cursor < MTG_DECKS ? U.cursor : c->activeDeck;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("DECKS", MTG_THEME);
    if (c->decks[d].count)
        BigCard(Mtg_DeckAce(c, d), 30, 22, 0);
    snprintf(line, sizeof(line), "%d CARDS", c->decks[d].count);
    G_TextAlign(52, 86, TA_CENTER, DeckValid(d) ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, TF_SMALL);
    ColorOrbs(52 - (__builtin_popcount(Mtg_DeckColors(&c->decks[d]) & 31) * 8 - 1) / 2, 99, Mtg_DeckColors(&c->decks[d]));
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    U.scroll = U.cursor - 3 < 0 ? 0 : U.cursor - 3 > DECK_ROWS - 6 ? DECK_ROWS - 6 : U.cursor - 3;
    for (i = 0; i < 6; i++) {
        int k = U.scroll + i;
        if (k >= DECK_ROWS)
            break;
        if (k == DECK_ROW_BACK || k == DECK_ROW_RECIPES) {
            TcgUi_Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, U.cursor == k, 1, k == DECK_ROW_BACK ? "BACK" : "DECK RECIPES",
                      NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", c->decks[k].count && c->decks[k].name[0] ? c->decks[k].name : "EMPTY SLOT");
        TcgUi_Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, U.cursor == k, c->decks[k].count != 0, line,
                  k == c->activeDeck && c->decks[k].count ? "IN USE" : NULL);
    }
    if (U.cursor == DECK_ROW_BACK)
        Strip("Back to the CARD CASE.");
    else if (U.cursor == DECK_ROW_RECIPES)
        Strip("The starter decks, built again from the cards you have.");
    else if (U.cursor == c->activeDeck && c->decks[U.cursor].count)
        Strip(Mtg_DeckProblem(c, &c->decks[U.cursor]) ? Mtg_DeckProblem(c, &c->decks[U.cursor])
                                                      : "The deck you duel with. A deck holds 40 cards or more.");
    else
        Strip(c->decks[U.cursor].count ? "{R} DUEL WITH THIS DECK" : "An empty slot: build a deck, or one from a recipe.");
    if (U.menuOn && U.cursor < MTG_DECKS) {
        if (SlotEmpty(U.cursor))
            TcgUi_PopMenu(118, 40, 114, sEmptyMenu, EM_COUNT, U.menuCursor, 0);
        else
            TcgUi_PopMenu(134, 20, 98, sDeckMenu, DM_COUNT, U.menuCursor, DeckMenuOff(U.cursor));
        TcgUi_Hints("{A} CHOOSE  {B} BACK");
    } else
        TcgUi_Hints("{A} MENU  {START} COPY  {SELECT} DELETE  {B} BACK");
    TcgUi_NameDraw(MTG_THEME);
    TcgUi_TrayDraw();
}

// ---- DECK RECIPES: the starter decks, built again from the cards owned
static void RecipeMissing(int k, char *out, size_t size, int *lost)
{
    MtgSavedDeck d;
    uint8_t used[MTG_COLLECTION_CARDS], missing[MTG_COLLECTION_CARDS];
    int i, n = 0;
    size_t len;
    RecipeOf(k, &d);
    memset(used, 0, sizeof(used));
    memset(missing, 0, sizeof(missing));
    for (i = 0; i < d.count; i++)
        if (!Mtg_IsBasicLand(d.cards[i]) && used[d.cards[i]]++ >= C()->owned[d.cards[i]])
            missing[d.cards[i]]++, n++;
    /* (as many as fit the strip's two lines, then how many more) */
    snprintf(out, size, "Missing:");
    for (i = 0; i < Mtg_DbCount() && i < MTG_COLLECTION_CARDS; i++) {
        char more[48], name[64];
        int left = 0, j;
        if (!missing[i])
            continue;
        for (j = i + 1; j < Mtg_DbCount() && j < MTG_COLLECTION_CARDS; j++)
            left += missing[j] != 0;
        PlainName(name, sizeof(name), i);
        len = strlen(out);
        snprintf(out + len, size - len, "%s %u %s", len > 8 ? "," : "", missing[i], name);
        snprintf(more, sizeof(more), " and %d more", left);
        if (G_Measure(out, TF_SMALL) + (left ? G_Measure(more, TF_SMALL) : 0) > 226 * 2 - 70 || strlen(out) + 40 > size) {
            out[len] = 0;
            snprintf(out + len, size - len, " and %d more", left + 1);
            break;
        }
    }
    *lost = n;
}
static void RecipesScreen(void)
{
    int have, total, lost, i, top = U.cursor < 3 ? 0 : U.cursor - 3;
    char line[200];
    TcgUi_Background();
    TcgUi_SidePanel("RECIPES", MTG_THEME);
    total = RecipeOwned(U.cursor, &have);
    BigCard(RecipeAce(U.cursor), 30, 22, 0);
    snprintf(line, sizeof(line), "%d/%d OWNED", have, total);
    G_TextAlign(52, 86, TA_CENTER, have == total ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    if (top > MTG_STARTERS - 6)
        top = MTG_STARTERS > 6 ? MTG_STARTERS - 6 : 0;
    for (i = 0; i < 6 && top + i < MTG_STARTERS; i++) {
        int h, t = RecipeOwned(top + i, &h);
        char right[16];
        snprintf(right, sizeof(right), "%d/%d", h, t);
        TcgUi_Row(LIST_X + 2, 8 + i * ROW_H, LIST_W - 4, U.cursor == top + i, 1, gMtgStarters[top + i].name, right);
    }
    RecipeMissing(U.cursor, line, sizeof(line), &lost);
    if (!lost)
        snprintf(line, sizeof(line), "The %s deck. You have every card.", gMtgStarters[U.cursor].name);
    Strip(line);
    TcgUi_Hints("{A} BUILD  {B} BACK");
}

// ---- ACE CARD: the card that stands for the deck
static int AceList(int deck, uint16_t *out)   // the deck's cards once each, creatures first (no basic lands)
{
    const MtgSavedDeck *d = &C()->decks[deck];
    int i, j, n = 0, pass;
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < d->count; i++) {
            int id = d->cards[i];
            if (Mtg_IsBasicLand(id) || ((Mtg_Card(id)->types & MTG_T_CREATURE) != 0) != (pass == 0))
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
    MtgCollection *c = C();
    uint16_t list[MTG_DECK_MAX];
    int n = AceList(U.deck, list), ace = Mtg_DeckAce(c, U.deck), page = ACE_COLS * ACE_ROWS, start = U.cursor / page * page;
    int i;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("ACE CARD", MTG_THEME);
    if (n)
        BigCard(list[U.cursor], 30, 22, (int)(sinf(TcgUi_Frame() * 0.06f) * 1.5f));
    G_TextAlign(52, 86, TA_CENTER, G_INK, G_INK_SHADOW, c->decks[U.deck].name, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    for (i = 0; i < page && start + i < n; i++) {
        int on = start + i == U.cursor, x = LIST_X + 4 + (i % ACE_COLS) * 31, y = 10 + (i / ACE_COLS) * 48 - on * 2;
        MtgGfx_DrawCard(list[start + i], (float)x, (float)y, 30, 42, 0, 255, 0);
        if (list[start + i] == ace) {
            G_RoundFill(x + 3, y + 32, 24, 9, 0xe8a020, 240);
            G_TextAlign(x + 15, y + 31, TA_CENTER, G_WHITE, G_NO_SHADOW, "ACE", TF_NARROW);
        }
        if (on) {
            int o = (TcgUi_Frame() / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (n > page) {
        snprintf(line, sizeof(line), "%d/%d", start / page + 1, (n + page - 1) / page);
        G_TextAlign(LIST_X + LIST_W - 5, 95, TA_RIGHT, G_INK, G_NO_SHADOW, line, TF_NARROW);
    }
    if (n) {
        char name[64];
        PlainName(name, sizeof(name), list[U.cursor]);
        snprintf(line, sizeof(line), "%s%s", name, list[U.cursor] == ace ? ": this deck's ACE CARD." : "");
        Strip(line);
    } else
        Strip("This deck has no cards to choose from.");
    TcgUi_Hints("{A} MAKE IT THE ACE  {INSPECT} LOOK  {B} BACK");
}

// ---- the builder: the cards (5 x 2 faces) and the deck's list
#define GRID_COLS 5
#define GRID_ROWS 2
#define LIST_ROWS 9

static int DeckUnique(const MtgSavedDeck *d, uint16_t *out)
{
    int n = 0, i, k;
    for (i = 0; i < d->count; i++) {
        for (k = 0; k < n && out[k] != d->cards[i]; k++)
            ;
        if (k == n)
            out[n++] = d->cards[i];
    }
    qsort(out, (size_t)n, sizeof(out[0]), ListOrder);
    return n;
}

static void CardLine(char *line, unsigned cap, int card)   // the strip's words for a card: its name, P/T, cost
{
    const MtgCardDef *def = Mtg_Card(card);
    char name[64], cost[16];
    unsigned n = 0, i, k;
    static const char sym[] = "WUBRGC";
    PlainName(name, sizeof(name), card);
    cost[0] = 0;
    if (!(def->types & MTG_T_LAND)) {
        if (def->cost.x)
            cost[n++] = 'X';
        if (def->cost.generic)
            n += (unsigned)snprintf(cost + n, sizeof(cost) - n, "%u", def->cost.generic);
        for (i = 0; i < MTG_MANA_KINDS; i++)
            for (k = 0; k < def->cost.pip[i] && n + 1 < sizeof(cost); k++)
                cost[n++] = sym[i];
        cost[n] = 0;
    }
    if (def->types & MTG_T_CREATURE)
        snprintf(line, cap, "%s  %d/%d  %s", name, def->power, def->toughness, cost);
    else
        snprintf(line, cap, "%s  %s", name, cost);
}

static void BuilderScreen(void)
{
    MtgCollection *c = C();
    const MtgSavedDeck *d = &c->decks[U.deck];
    uint16_t listed[MTG_DECK_MAX];
    int n = U.listCount, i, page = GRID_COLS * GRID_ROWS, start = U.cursor / page * page, uniq = DeckUnique(d, listed), card;
    char line[96];
    TcgUi_Background();
    G_RoundFill(0, 0, 160, 17, 0x203040, 230);
    Chips(2, 2, FILTERS - 1);
    G_Hint(118, 3, 0xe0e0ff, G_NO_SHADOW, "{L}{R}", TF_NARROW, 1);
    for (i = 0; i < page && start + i < n; i++) {
        int id = U.list[start + i], x = 4 + (i % GRID_COLS) * 31, y = 20 + (i / GRID_COLS) * 46;
        int on = U.side == 0 && start + i == U.cursor, in = Mtg_DeckCopies(d, id);
        if (on)
            y -= 2;
        MtgGfx_DrawCard(id, (float)x, (float)y, 30, 42, 0, 255, 0);
        snprintf(line, sizeof(line), Mtg_IsBasicLand(id) ? "%d" : "%d/%d", in, c->owned[id]);
        G_RoundFill(x + 1, y + 33, 28, 9, in ? 0x2858a8 : 0x404850, 235);
        G_Digits(x + 15 - G_DigitsWidth(line, 0, 1) / 2, y + 35, line, 0, 1, 0xf0f8ff, 255);
        if (on) {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n)
        G_TextAlign(80, 56, TA_CENTER, G_WHITE, 0x204030, "No cards of this kind.", TF_SMALL);
    if (n > page) {
        snprintf(line, sizeof(line), "%d/%d", start / page + 1, (n + page - 1) / page);
        G_TextAlign(157, 3, TA_RIGHT, G_WHITE, 0x101820, line, TF_NARROW);
    }
    // the deck: how many, READY, then a row a card
    TcgUi_ListBox(162, 0, 78, 110);
    snprintf(line, sizeof(line), "%d", d->count);
    G_Text(166, 1, d->count >= MTG_DECK_MIN ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, 0, TF_SMALL);
    if (!Mtg_DeckProblem(c, d))
        G_Text(206, 2, 0x2858a8, G_NO_SHADOW, "READY", 0, TF_NARROW);
    if (U.deckCursor >= uniq)
        U.deckCursor = uniq ? uniq - 1 : 0;
    {
        int top = U.deckCursor < 4 ? 0 : U.deckCursor - 4;
        if (top > uniq - LIST_ROWS)
            top = uniq > LIST_ROWS ? uniq - LIST_ROWS : 0;
        for (i = 0; i < LIST_ROWS && top + i < uniq; i++) {
            int id = listed[top + i], y = 14 + i * 10, on = U.side == 1 && top + i == U.deckCursor;
            int short_ = !Mtg_IsBasicLand(id) && Mtg_DeckCopies(d, id) > c->owned[id];
            char name[64];
            if (on)
                G_RoundFill(164, y, 74, 10, 0xf8d0c8, 255);
            G_FillA(165, y + 2, 2, 6, MtgGfx_FrameColor(id, 0), 255);
            PlainName(name, sizeof(name), id);
            TcgUi_FitText(name, 52, TF_NARROW);
            G_Text(169, y - 1, short_ ? 0xa0a0a0 : G_INK, G_NO_SHADOW, name, 0, TF_NARROW);
            snprintf(line, sizeof(line), "x%d", Mtg_DeckCopies(d, id));
            G_Digits(236 - G_DigitsWidth(line, 0, 1), y + 2, line, 0, 1, short_ ? 0xa0a0a0 : 0x2858a8, 255);
        }
    }
    card = U.side ? (uniq ? listed[U.deckCursor] : -1) : (U.cursor < n ? U.list[U.cursor] : -1);
    line[0] = 0;
    if (card >= 0)
        CardLine(line, sizeof(line), card);
    Strip(line);
    TcgUi_Hints(U.side ? "{A} TAKE OUT  {LEFT} CARDS  {INSPECT} LOOK  {B} DONE"
                       : "{A} ADD  {SELECT} OUT  {INSPECT} LOOK  {B} DONE");
}

// ---- the binder: 7 x 2 faces
static void BinderScreen(void)
{
    MtgCollection *c = C();
    int n = U.listCount, i, cols = 7, page = 14, start = U.cursor / page * page, have, total;
    char line[128];
    TcgUi_Background();
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    Chips(2, 2, FILTERS);
    OwnedCounts(&have, &total);
    snprintf(line, sizeof(line), "%d/%d", have, total);
    G_TextAlign(236, 2, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_SMALL);
    for (i = 0; i < page && start + i < n; i++) {
        int id = U.list[start + i], x = 8 + (i % cols) * 33, y = 21 + (i / cols) * 45, on = start + i == U.cursor;
        int count = c->owned[id];
        if (on)
            y -= 2;
        MtgGfx_DrawCard(id, (float)x, (float)y, 30, 42, 0, count ? 255 : 80, 0);
        if (count)
            Foil(id, x, y, 30, 42, (unsigned)(U.frame + i * 13));
        if (count) {
            snprintf(line, sizeof(line), "x%d", count);
            G_RoundFill(x + 14, y + 33, G_DigitsWidth(line, 0, 1) + 4, 9, 0x1c2c48, 230);
            G_Digits(x + 16, y + 35, line, 0, 1, 0xe0f0ff, 255);
        }
        /* NEW until the cursor has been on it (tcg_new.c) */
        if (on && count)
            CardNew_Saw(CARDNEW_MAGIC, (unsigned)id, (unsigned)count);
        else if (CardNew_Is(CARDNEW_MAGIC, (unsigned)id, (unsigned)count))
            CardNew_Badge(x - 2, y - 2, U.frame);
        if (on) {
            int o = (U.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 1 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (!n)
        G_TextAlign(120, 56, TA_CENTER, G_WHITE, 0x204030, "No cards of this kind yet.", TF_SMALL);
    line[0] = 0;
    if (U.cursor < n) {
        int id = U.list[U.cursor];
        char name[64];
        PlainName(name, sizeof(name), id);
        snprintf(line, sizeof(line), "%s\n%s", name, c->owned[id] ? RarityWord(id) : "Not in your collection yet.");
    }
    Strip(line);
    TcgUi_Hints(U.showAll ? "{A} LOOK  {L}{R} COLOR  {SELECT} OWNED  {B} BACK" : "{A} LOOK  {L}{R} COLOR  {SELECT} ALL  {B} BACK");
}

// ---- a card's view: the big face, its words; A the big picture
static void InspectScreen(void)
{
    const MtgCardDef *c = Mtg_Card(U.inspectCard);
    char name[64], text[800], line[200];
    int y, bob = U.age < 12 ? (int)((12 - U.age) * 1.5f) : 0;
    const char *artist = MtgGfx_Artist(U.inspectCard);
    TcgUi_Background();
    G_FillA(10, 34 - bob, 72, 100, 0x000000, 90);
    MtgGfx_DrawCard(U.inspectCard, 8, (float)(30 - bob), 72, 100, 0, 255, 0);
    if (C()->owned[U.inspectCard])
        Foil(U.inspectCard, 8, 30 - bob, 72, 100, (unsigned)U.frame);
    TcgUi_ListBox(86, 4, 152, 140);
    {
        SDL_Rect clip = {90, 8, 144, 132};
        SDL_RenderSetClipRect(G_R(), &clip);
    }
    y = 8 - U.inspectScroll;
    PlainName(name, sizeof(name), U.inspectCard);
    {
        int costW = MtgGfx_Cost(U.inspectCard, 232, y + 4, 1, 1);
        int end = G_Text(94, y, G_INK, G_INK_SHADOW, name, 136 - (costW ? costW + 4 : 0), TF_NORMAL);
        char keys[120];
        y = end + 2 > y + 15 ? end + 2 : y + 15;
        MtgGfx_KeywordLine(U.inspectCard, c->keywords, keys, sizeof(keys));
        if (keys[0])   // (its keywords under its name)
            y = G_Text(94, y - 2, 0xb07818, G_NO_SHADOW, keys, 136, TF_NARROW) + 1;
    }
    TypeLine(text, sizeof(text), U.inspectCard);
    y = G_Text(94, y, 0x586878, G_NO_SHADOW, text, 136, TF_NARROW) + 1;
    G_FillA(92, y, 140, 1, 0xd8d8d0, 255);
    Mtg_PlainText(text, sizeof(text), c->oracle);
    y = G_Text(94, y + 2, 0x404040, G_NO_SHADOW, text[0] ? text : "(No rules text.)", 136, TF_NARROW);
    if (c->types & MTG_T_CREATURE) {
        snprintf(line, sizeof(line), "%d/%d", c->power, c->toughness);
        G_TextAlign(230, y + 1, TA_RIGHT, G_INK, G_INK_SHADOW, line, TF_SMALL);
        y += 13;
    }
    G_FillA(92, y, 140, 1, 0xd8d8d0, 255);
    y += 2;
    if (c->banned)
        y = G_Text(94, y, 0xc03020, G_NO_SHADOW, "BANNED (not in decks)", 136, TF_NARROW);
    y = G_Text(94, y, 0x707878, G_NO_SHADOW, RarityWord(U.inspectCard), 136, TF_NARROW);
    if (artist && artist[0]) {   // (Scryfall asks that the illustration shown credit its artist)
        char plain[96];
        Mtg_PlainText(plain, sizeof(plain), artist);
        snprintf(line, sizeof(line), "Art by %s", plain);
        y = G_Text(94, y, 0x707878, G_NO_SHADOW, line, 136, TF_NARROW);
    }
    snprintf(line, sizeof(line), "You own %d.", C()->owned[U.inspectCard]);
    y = G_Text(94, y, 0x707878, G_NO_SHADOW, line, 136, TF_NARROW);
    U.inspectMax = y + U.inspectScroll - 136;
    if (U.inspectMax < 0)
        U.inspectMax = 0;
    SDL_RenderSetClipRect(G_R(), NULL);
    if (U.inspectMax > 0) {
        if (U.inspectScroll > 0)
            G_Arrow(226, 7, 0, 0x707070);
        if (U.inspectScroll < U.inspectMax)
            G_Arrow(226, 136, 1, 0x707070);
    }
    TcgUi_Hints(U.inspectMax ? "{A} BIG PICTURE  {UPDOWN} SCROLL  {B} BACK" : "{A} BIG PICTURE  {B} BACK");
}

static void ArtScreen(void)
{
    int w, h;
    SDL_Texture *art = MtgGfx_ArtTexture(U.inspectCard, &w, &h);
    const char *artist = MtgGfx_Artist(U.inspectCard);
    G_Fill(0, 0, 240, 160, 0x101418);
    if (art) {
        float fit = G_MinF(156.0f / h, 228.0f / w);
        SDL_FRect dst;
        if (U.zoom)
            fit *= 1.8f;
        dst.w = w * fit;
        dst.h = h * fit;
        dst.x = 120 - dst.w / 2 - U.panX;
        dst.y = 80 - dst.h / 2 - U.panY;
        SDL_RenderCopyF(G_R(), art, NULL, &dst);
    } else {   // (a card without a printed picture: its face)
        MtgGfx_DrawCard(U.inspectCard, 84, 30, 72, 100, 0, 255, 0);
    }
    if (artist && artist[0]) {   // (Scryfall asks that the illustration shown without its card credit the artist)
        char plain[96], line[120];
        Mtg_PlainText(plain, sizeof(plain), artist);
        snprintf(line, sizeof(line), "Art by %s", plain);
        G_Text(4, 1, 0xc0c4c8, 0x000000, line, 0, TF_NARROW);
    }
    TcgUi_Hints(U.zoom ? "{A} FIT  {DPAD} MOVE  {B} BACK" : "{A} ZOOM  {DPAD} MOVE  {B} BACK");
}

// ---- the CARD SHOP's starters: each deck by its best card, in a row (as the other games' first decks)
static void StarterScreen(void)
{
    int i;
    char line[200];
    TcgUi_Background();
    TcgUi_TopBar("CHOOSE A STARTER DECK");
    for (i = 0; i < MTG_STARTERS; i++) {
        int x = 6 + i * 47, on = i == U.cursor, bob = on ? (int)(fabsf(sinf(U.frame * 0.1f)) * 3) : 0;
        G_FillA(x + 2, 30 - bob + 3, 44, 61, 0x000000, 70);
        MtgGfx_DrawCard(RecipeAce(i), (float)x, (float)(30 - bob), 44, 61, 0, 255, 0);
        G_TextAlign(x + 22, 96, TA_CENTER, on ? 0xf8e070 : G_WHITE, 0x204030, gMtgStarters[i].name, TF_NARROW);
        if (on)
            G_Frame(x - 3, 30 - bob - 3, 50, 67, 0xe82820, 255);
    }
    snprintf(line, sizeof(line), "40 cards. %s", gMtgStarters[U.cursor].about);
    Strip(line);
    TcgUi_Hints("{LEFTRIGHT} CHOOSE  {A} TAKE  {INSPECT} LOOK  {B} NOT NOW");
}

// ---- a duelist's challenge: their picture and best card, DUEL WITH one of your decks
static void Portrait(int x, int y)
{
    SDL_Rect dst;
    if (!U.portraitW)
        return;
    if (!U.portraitTexture) {
        struct GSurface s;
        s.w = U.portraitW;
        s.h = U.portraitH;
        s.px = U.portrait;
        U.portraitTexture = G_SurfaceTexture(&s);
    }
    if (!U.portraitTexture)
        return;
    if (U.portraitW == 64) {   /* a battle picture, cut to the window beside their card */
        G_PicCut(U.portraitTexture, U.portrait, 8, 22, 48, 42);
        return;
    }
    dst.x = x - U.portraitW;
    dst.y = y - U.portraitH * 2;
    dst.w = U.portraitW * 2;
    dst.h = U.portraitH * 2;
    SDL_RenderCopy(G_R(), U.portraitTexture, NULL, &dst);
}

static void ChallengeScreen(void)
{
    const MtgDuelist *who = &gMtgDuelists[U.duelist];
    MtgCollection *c = C();
    char line[200];
    int i, y = 68;
    TcgUi_Background();
    TcgUi_SidePanel("CHALLENGE", MTG_THEME);
    G_RoundFill(6, 20, 92, 46, 0x2c2c5c, 255);
    G_RoundFrame(6, 20, 92, 46, 0x101030, 255);
    Portrait(28, 62);
    MtgGfx_DrawCard(DuelistAce(who->cards), 58, 22, 30, 42, 0, 255, 0);
    G_Text(6, y, G_INK, G_INK_SHADOW, who->name, 96, TF_SMALL);
    y += 12;
    G_Text(6, y + 1, 0x505868, G_NO_SHADOW, "MAGIC  20 LIFE", 96, TF_NARROW);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    G_Text(LIST_X + 6, 5, 0x606878, G_NO_SHADOW, "DUEL WITH", 0, TF_NARROW);
    U.scroll = U.cursor < 2 ? 0 : U.cursor > MTG_DECKS - 3 ? MTG_DECKS - 4 : U.cursor - 2;
    for (i = 0; i < 5; i++) {
        int d = U.scroll + i;
        if (d > MTG_DECKS)
            break;
        if (d == MTG_DECKS) {
            TcgUi_Row(LIST_X + 2, 18 + i * ROW_H, LIST_W - 4, U.cursor == d, 1, "NOT NOW", NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", c->decks[d].count && c->decks[d].name[0] ? c->decks[d].name : "---");
        TcgUi_Row(LIST_X + 2, 18 + i * ROW_H, LIST_W - 4, U.cursor == d, DeckValid(d), line,
                  d == c->activeDeck && c->decks[d].count ? "IN USE" : NULL);
    }
    (void)who->hello;
    Strip(c->beaten & (1u << U.duelist) ? "WIN: a booster pack, eight cards for your BINDER. (Every time you win.)"
                                        : "WIN: a booster pack, eight cards for your BINDER.");
}

// ---- BOOSTER PACKS (the YU-GI-OH! side's screen): the boosters held by set, the chosen one in the middle; one opened:
// the pack torn open, the cards dealt out face down and turned one by one
// a booster: the set's colors, its card (Mtg_SetMascot), its code
static void PackArt(int x, int y, int set, int focus)
{
    const MtgSet *s = &gMtgSets[set];
    int w = 44, h = 58, i, mascot = Mtg_SetMascot(set);
    G_FillA(x + 2, y + 3, w, h, 0x000000, 80);
    for (i = 0; i < h; i++)
        G_FillA(x, y + i, w, 1, G_Mix(s->top, s->bottom, i / (float)h), 255);
    for (i = 0; i < w; i += 2) {   // (the crimped ends)
        G_FillA(x + i, y, 1, 3, 0xd8d8e0, 255);
        G_FillA(x + i, y + h - 3, 1, 3, 0xd8d8e0, 255);
    }
    if (mascot >= 0)
        MtgGfx_DrawCard(mascot, (float)x + 7, (float)y + 5, 30, 42, 0, 255, 0);
    G_FillA(x, y + 47, w, 8, 0x101010, 150);
    G_TextAlign(x + w / 2, y + 45, TA_CENTER, MTG_GOLD, G_NO_SHADOW, s->code, TF_NARROW);
    if (focus) {
        int k = (int)(U.frame % 60) - 10;
        if (k >= 0 && k < w + 8)
            G_FillA(x + k - 8, y + 3, 3, h - 6, 0xffffff, 70);
    }
}

// the boosters held, and every set that can be had, in release order
static int PackSets(uint8_t *out)
{
    int i, n = 0;
    for (i = 0; i < MTG_SETS; i++)
        if (C()->packs[i] || Mtg_SetAvailable(i))
            out[n++] = (uint8_t)i;
    return n;
}

static void PacksScreen(void)
{
    uint8_t sets[MTG_SETS];
    int n = PackSets(sets);
    char line[128];
    TcgUi_Background();
    TcgUi_TopBar("BOOSTER PACKS");
    {   // the chosen pack in the middle, the others on both sides, gliding over as the cursor moves
        static float sPos;
        static int sLast;
        int k, base;
        if (U.frame != sLast + 1)
            sPos = (float)U.cursor;   // (the screen just opened: no glide in)
        sLast = U.frame;
        sPos += ((float)U.cursor - sPos) * 0.35f;
        if (fabsf((float)U.cursor - sPos) < 0.02f)
            sPos = (float)U.cursor;
        base = (int)floorf(sPos);
        for (k = base - 3; k <= base + 3; k++) {
            int set, x = 120 - 22 + (int)lroundf(((float)k - sPos) * 57), on = k == U.cursor, bob;
            if (k < 0 || k >= n || x < -50 || x > 240)
                continue;
            set = sets[k];
            bob = on ? (int)(fabsf(sinf(U.frame * 0.1f)) * 3) : 0;
            PackArt(x, 26 - bob, set, on);
            snprintf(line, sizeof(line), "x%u", C()->packs[set]);
            G_TextAlign(x + 22, 88, TA_CENTER, C()->packs[set] ? G_WHITE : 0xc0d8c8, 0x204030, line, TF_SMALL);
            if (on)
                G_Frame(x - 3, 26 - bob - 2, 50, 64, 0xe82820, 255);
        }
        if (U.cursor > 0)
            G_TextAlign(4, 50, TA_CENTER, G_WHITE, 0x204030, "<", TF_SMALL);
        if (U.cursor < n - 1)
            G_TextAlign(236, 50, TA_CENTER, G_WHITE, 0x204030, ">", TF_SMALL);
    }
    if (n) {
        int set = sets[U.cursor < n ? U.cursor : 0], total, have = Mtg_SetOwned(C(), set, &total);
        snprintf(line, sizeof(line), "%s (%u)\n%s  (%d/%d)", gMtgSets[set].name, gMtgSets[set].year,
                 C()->packs[set] ? "Open this pack!" : "Win duels to earn boosters.", have, total);
        Strip(line);
    }
    TcgUi_Hints("{LEFTRIGHT} CHOOSE  {A} OPEN  {B} BACK");
}

// where card i of n goes: up to 8 in two rows of four; more (15) in rows of eight and seven, a little smaller
static void BoosterSlot(int i, int n, int *x, int *y, int *w, int *h)
{
    if (n <= 8) {
        *w = 30, *h = 42;
        *x = 30 + (i % 4) * 46;
        *y = i < 4 ? 6 : 55;
        return;
    }
    *w = 27, *h = 38;
    *x = i < 8 ? 5 + i * 29 : 19 + (i - 8) * 29;
    *y = i < 8 ? 8 : 56;
}

static const char *SetRarityWord(int r)
{
    return r == MTG_RARITY_RARE ? "RARE" : r == MTG_RARITY_UNCOMMON ? "Uncommon" : "Common";
}

static void SpoilsScreen(void)
{
    int i, t = U.revealAge;
    TcgUi_Background();
    if (t < REVEAL_TEAR) {
        int shake = t < 14 ? ((t / 2) % 2 ? 2 : -2) : 0;
        float tear = t < 14 ? 0 : 1 - (1 - (t - 14) / 12.0f) * (1 - (t - 14) / 12.0f);
        if (tear > 1)
            tear = 1;
        if (tear > 0)
            MtgGfx_DrawCard(MTG_GFX_BACK, 105, 52 - tear * 22, 30, 42, 0, 255, 0);
        PackArt(98 + shake, 50 + (int)(tear * 30), U.pack, 0);
        TcgUi_Hints("{A} OPEN");
        return;
    }
    for (i = 0; i < U.boosterCount; i++) {
        int x, y, w, h, faceUp = i < U.flipped, on = i == U.cursor && U.flipped >= U.boosterCount;
        float in = (float)(t - REVEAL_TEAR - i * 2) / 10.0f, sx, sy;
        int card = U.booster[i], rare = U.boosterRarity[i] == MTG_RARITY_RARE;
        BoosterSlot(i, U.boosterCount, &x, &y, &w, &h);
        if (in < 0)
            in = 0;
        if (in > 1)
            in = 1;
        sx = 105 + (x - 105) * in;
        sy = 40 + (y - 40) * in;
        if (faceUp && rare)
            G_FoilHalo((int)sx, (int)sy - (on ? 3 : 0), w, h, 0xf0c050, (unsigned)(U.frame + i * 17), 0);
        MtgGfx_DrawCard(faceUp ? card : MTG_GFX_BACK, sx, sy - (on ? 3 : 0), (float)w, (float)h, 0, 255, 0);
        if (faceUp)
            Foil(card, (int)sx, (int)sy - (on ? 3 : 0), w, h, (unsigned)(U.frame + i * 17));
        if (faceUp && rare) {   // (on the card's foot, as on the other sides)
            G_RoundFill((int)sx + 1, (int)sy + h - 9 - (on ? 3 : 0), w - 2, 9, 0x101828, 210);
            G_TextAlign((int)sx + w / 2, (int)sy + h - 10 - (on ? 3 : 0), TA_CENTER, (U.frame / 6) % 2 ? 0xf8d060 : 0xfff0b0,
                        0x302040, "RARE!", TF_NARROW);
        }
        if (faceUp && CardNew_Is(CARDNEW_MAGIC, (unsigned)card, C()->owned[card]) && C()->owned[card] == 1) {
            G_RoundFill((int)sx - 2, (int)sy - 3 - (on ? 3 : 0), 17, 8, 0xe82820, 230);
            G_Text((int)sx, (int)sy - 3 - (on ? 3 : 0), 0xffffff, 0x801010, "NEW", 17, TF_NARROW);
        }
        if (on) {
            int o = (U.frame / 8) % 2;
            G_Frame((int)sx - 2 - o, (int)sy - 5 - o, w + 4 + 2 * o, h + 4 + 2 * o, 0xe82820, 255);
        }
    }
    if (U.flipped >= U.boosterCount && U.cursor < U.boosterCount) {
        int card = U.booster[U.cursor];
        char name[64], line[96];
        G_FillA(0, STRIP_Y, 240, 160 - STRIP_Y, 0x5848c8, 255);
        G_FillA(0, STRIP_Y, 240, 1, 0x9888f8, 255);
        PlainName(name, sizeof(name), card);
        G_Text(8, STRIP_Y + 4, U.boosterRarity[U.cursor] == MTG_RARITY_RARE ? 0xf8d060
                               : U.boosterRarity[U.cursor] == MTG_RARITY_UNCOMMON ? 0xd0dcf0 : 0xf0f0f0,
               0x302878, name, 226, TF_NORMAL);
        snprintf(line, sizeof(line), "%s, %s%s", SetRarityWord(U.boosterRarity[U.cursor]), gMtgSets[U.pack].name,
                 C()->owned[card] == 1 ? "  NEW!" : "");
        G_Text(8, STRIP_Y + 20, 0xd8d8f8, 0x302878, line, 226, TF_SMALL);
        TcgUi_Hints("{A} LOOK  {DPAD} CHOOSE  {B} DONE");
    } else
        TcgUi_Hints("{A} NEXT CARD  {B} SHOW ALL");
}

// a booster of the set opened: the reveal starts (the achievements counted)
static void OpenPack(int set)
{
    MtgCollection *c = C();
    int k, sets = 0, kinds = 0;
    U.boosterCount = Mtg_OpenPack(c, set, U.booster, U.boosterRarity);
    if (!U.boosterCount)
        return;
    U.pack = set;
    Screen(M_SPOILS);
    U.flipped = 0;
    U.revealAge = 0;
    MP_Achieve_Count("mtgpack", 1);
    for (k = 0; k < MTG_SETS; k++)
        sets += c->opened[k] != 0;
    if (sets >= 5)
        MP_Achieve_Unlock("mtgsets5");
    for (k = 0; k < Mtg_DbCount() && k < MTG_COLLECTION_CARDS; k++)
        kinds += c->owned[k] != 0;
    if (kinds >= 250)
        MP_Achieve_Unlock("mtgkinds250");
    MPNet_Log("mtg: opened a %s booster: %d cards, %d %s booster%s left", gMtgSets[set].code, U.boosterCount,
              c->packs[set], gMtgSets[set].code, c->packs[set] == 1 ? "" : "s");
}

// ---- the CARD SHOP's counter (the YU-GI-OH! side's): boosters for CARD POINTS
static unsigned Points(void) { return Tcg_GetCollection()->dust; }
static int ShopRows(uint8_t *sets)
{
    int i, n = 0;
    for (i = 0; i < MTG_SETS; i++)
        if (Mtg_SetAvailable(i))
            sets[n++] = (uint8_t)i;
    return n;
}
static void ShopScreen(void)
{
    static const char *const sTabs[] = {"BOOSTERS"};
    uint8_t sets[MTG_SETS];
    int n = ShopRows(sets), k, top = U.cursor < 3 ? 0 : U.cursor - 3;
    char line[128];
    TcgUi_Background();
    TcgUi_SidePanel("CARD SHOP", MTG_THEME);
    snprintf(line, sizeof(line), "%u CP", Points());
    TcgUi_ShopTabs(sTabs, 1, 0, 0xe8c898, 0x7a4a2a, 0x4a2a10, line, "CARD POINTS", 0x2858a8);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    if (top > n - 6)
        top = n > 6 ? n - 6 : 0;
    for (k = 0; k < 6 && top + k < n; k++) {
        int row = top + k;
        snprintf(line, sizeof(line), "%u", MTG_PACK_COST);
        TcgUi_Row(LIST_X + 2, 8 + k * ROW_H, LIST_W - 4, U.cursor == row, Points() >= MTG_PACK_COST,
                  gMtgSets[sets[row]].name, line);
    }
    if (U.cursor < n) {
        const MtgSet *st = &gMtgSets[sets[U.cursor]];
        snprintf(line, sizeof(line), "A %s booster: %d cards, a rare in each. You have %u.", st->code,
                 st->commons + st->uncommons + st->rares, C()->packs[sets[U.cursor]]);
        Strip(line);
    }
    TcgUi_Hints("{A} BUY  {B} LEAVE");
}

static void Inspect(int card, int back)
{
    U.inspectCard = card;
    U.inspectScroll = 0;
    U.back = back;
    U.screen = M_INSPECT;
    U.age = 0;
    PlaySE(SE_SELECT);
}

static void SpoilsStep(unsigned pressed)
{
    if (U.revealAge < REVEAL_TEAR) {
        if (U.revealAge == 14)
            PlaySE(SE_RG_CARD_FLIPPING);
        if (pressed & KEY_A)
            U.revealAge = REVEAL_TEAR;
        return;
    }
    if (U.flipped < U.boosterCount) {
        if (U.revealAge == REVEAL_TEAR)
            U.revealNext = U.revealAge + 8;
        if (pressed & KEY_B) {   // all at once
            U.flipped = U.boosterCount;
            U.cursor = 0;
            PlaySE(SE_RG_CARD_FLIP);
            return;
        }
        if (pressed & KEY_A)
            U.revealNext = U.revealAge;
        if (U.revealAge >= U.revealNext) {
            int rare = U.boosterRarity[U.flipped] == MTG_RARITY_RARE;
            PlaySE(rare ? SE_SHINY : SE_RG_CARD_FLIP);
            U.flipped++;
            U.cursor = U.flipped - 1;
            U.revealNext = U.revealAge + (Tcg_GetCollection()->fastAnimations ? 5 : rare ? 26 : 11);
            if (U.flipped == U.boosterCount)
                U.cursor = 0;
        }
        return;
    }
    {
        int c = U.cursor;
        if (pressed & KEY_RIGHT && c + 1 < U.boosterCount)
            c++;
        if (pressed & KEY_LEFT && c > 0)
            c--;
        {
            int row = U.boosterCount <= 8 ? 4 : 8;   // (the rows of BoosterSlot)
            if (pressed & KEY_DOWN && c + row < U.boosterCount)
                c += row;
            else if (pressed & KEY_DOWN && c < row && U.boosterCount > row)
                c = U.boosterCount - 1;
            if (pressed & KEY_UP && c >= row)
                c -= row;
        }
        if (c != U.cursor)
            U.cursor = c, PlaySE(SE_SELECT);
    }
    if (pressed & (KEY_A | KEY_INSPECT)) {
        int keep = U.cursor;
        Inspect(U.booster[U.cursor], M_SPOILS);
        U.cursor = keep;
    } else if (pressed & KEY_B) {
        int k, at = 0;
        uint8_t sets[MTG_SETS];
        int n = PackSets(sets);
        PlaySE(SE_SELECT);
        for (k = 0; k < U.boosterCount; k++)   // (seen here: no longer NEW in the BINDER)
            CardNew_Saw(CARDNEW_MAGIC, U.booster[k], C()->owned[U.booster[k]]);
        for (k = 0; k < n; k++)
            if (sets[k] == U.pack)
                at = k;
        Screen(M_PACKS);   // (back to the boosters, on the same set)
        U.cursor = at;
    }
}

void MtgUi_Draw(void)
{
    MtgGfx_SetModes(gMpConfig.cardArt, gMpConfig.cardBack);
    switch (U.screen) {
    case M_HOME: TcgUi_RowDown(1), HomeScreen(), TcgUi_RowDown(0); break;
    case M_DECKS: TcgUi_RowDown(1), DecksScreen(), TcgUi_RowDown(0); break;
    case M_RECIPES: RecipesScreen(); break;
    case M_ACE: AceScreen(); break;
    case M_BUILDER: BuilderScreen(); break;
    case M_BINDER: BinderScreen(); break;
    case M_INSPECT: InspectScreen(); break;
    case M_ART: ArtScreen(); break;
    case M_STARTER: StarterScreen(); break;
    case M_CHALLENGE: ChallengeScreen(); break;
    case M_DUEL: MtgBoard_Draw(); break;
    case M_SPOILS: SpoilsScreen(); break;
    case M_PACKS: PacksScreen(); break;
    case M_SHOP: ShopScreen(); break;
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

void MtgUi_OpenShop(void)   // the CARD SHOP's clerk: boosters for CARD POINTS
{
    Mtg_EnsureDb();
    U.returnToGame = NULL;
    Open(M_SHOP);
    U.fromScript = 1;
    MPNet_Log("mtg: card shop");
}

void MtgUi_OpenChallenge(unsigned duelist)
{
    MtgCollection *c = C();
    int k;
    U.returnToGame = NULL;
    U.duelist = (int)(duelist < MTG_DUELISTS ? duelist : 0);
    U.vs = VS_NPC;
    Open(M_CHALLENGE);
    U.fromScript = 1;
    ForgetPortrait();
    // (the one who asked, as they face you: their sprite on the map)
    Tcg_PortraitOfObject(GetObjectEventIdByLocalIdAndMap((u8)gSpecialVar_LastTalked, gSaveBlock1Ptr->location.mapNum,
                                                         gSaveBlock1Ptr->location.mapGroup),
                         U.portrait, &U.portraitW, &U.portraitH);
    if (!DeckValid(c->activeDeck))
        for (k = 0; k < MTG_DECKS; k++)
            if (DeckValid(k)) {
                c->activeDeck = (uint8_t)k;
                break;
            }
    U.cursor = c->activeDeck;
    MPNet_Log("mtg: challenge from %s", gMtgDuelists[U.duelist].name);
}

static void StartDuel(const char *const *theirCards, const char *name)
{
    MtgCollection *c = C();
    MtgSavedDeck theirs;
    MtgRecipe r;
    static MtgDeck mine, them;
    r.name = name;
    r.about = "";
    r.colors = 0;
    r.cards = theirCards;
    Mtg_RecipeDeck(&r, &theirs);
    Mtg_DeckToDuel(&c->decks[c->activeDeck], &mine);
    Mtg_DeckToDuel(&theirs, &them);
    MtgBoard_SetTray(DeckTrayRgb(c->activeDeck));
    Screen(M_DUEL);
    MtgBoard_Start(&mine, &them, Mtg_Rand(&c->rng), name);
    U.rewarded = 0;
}

static void Practice(void)   // against a starter deck that isn't yours: nothing won or lost
{
    MtgCollection *c = C();
    int pick;
    if (!DeckValid(c->activeDeck)) {
        Say(ACT_NONE, "%s", c->decks[c->activeDeck].count ? Mtg_DeckProblem(c, &c->decks[c->activeDeck])
                                                           : "Build a deck first.");
        return;
    }
    pick = (int)(Mtg_Rand(&c->rng) % MTG_STARTERS);
    if (pick == c->starter)
        pick = (pick + 1) % MTG_STARTERS;
    U.vs = VS_PRACTICE;
    MPNet_Log("mtg: practice against %s", gMtgStarters[pick].name);
    StartDuel(gMtgStarters[pick].cards, "PRACTICE DECK");
}

// the duel is over (its result screen just came up): the record, the booster a win brings, the achievements, and the
// words for the result screen (the POKeMON table's way: the opponent's quote, then what you got)
static void DuelEnded(int result)
{
    MtgCollection *c = C();
    const MtgDuelist *who = &gMtgDuelists[U.duelist];
    char line[320];
    if (U.vs == VS_PRACTICE) {
        MPNet_Log("mtg: practice over: %s", result == 1 ? "won" : result == 2 ? "lost" : "a draw");
        MtgBoard_Result("A practice duel: nothing is won or lost.");
        return;
    }
    if (result == 1) {
        const MtgDuel *d = MtgBoard_Duel();
        uint8_t colors = Mtg_DeckColors(&c->decks[c->activeDeck]);
        int set = Mtg_RandomSet(&c->rng), got = Mtg_AwardPack(c, set);
        c->wins++;
        MP_Achieve_Count("mtgwin", 1);
        if (!(c->beaten & (1u << U.duelist)))
            MP_Achieve_Count("mtgregulars", 1);
        c->beaten |= 1u << U.duelist;
        if (d->players[0].life >= 20)
            MP_Achieve_Unlock("mtgflawless");
        if (d->players[1].drewFromEmpty && d->players[1].life > 0)
            MP_Achieve_Unlock("mtgdecked");
        if (d->turn <= 10)   // (five turns of yours at most)
            MP_Achieve_Unlock("mtgfast");
        if (__builtin_popcount(colors) == 1)
            MP_Achieve_Unlock("mtgmono");
        if (colors == 0x1F)
            MP_Achieve_Unlock("mtgdomain");
        MPNet_Log("mtg: won against %s, a %s booster", who->name, gMtgSets[set].code);
        if (got)
            snprintf(line, sizeof(line), "%s: \"%s\"\nA %s booster went into your CARD CASE.", who->name, who->lose,
                     gMtgSets[set].code);
        else
            snprintf(line, sizeof(line), "%s: \"%s\"\nYour CARD CASE can't hold more %s boosters.", who->name, who->lose,
                     gMtgSets[set].code);
    } else {
        if (result == 2)
            c->losses++;
        MPNet_Log("mtg: %s against %s", result == 2 ? "lost" : "a draw", who->name);
        snprintf(line, sizeof(line), "%s: \"%s\"", who->name, result == 2 ? who->win : "A draw! Let's play again some time.");
    }
    MtgBoard_Result(line);
}

// the result screen left: back to the map (a practice: to the case)
static void DuelOver(int result)
{
    (void)result;
    MtgBoard_End();
    if (U.vs == VS_PRACTICE) {
        Screen(M_HOME);
        U.cursor = HOME_PRACTICE;
        return;
    }
    Close();
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
        MP_Achieve_Unlock("mtgstart");
        MPNet_Log("mtg: starter %s chosen", gMtgStarters[c->starter].name);
        Say(ACT_CLOSE, "The %s deck is yours! MAGIC is on your CARD CASE now: L and R go round its games.",
            gMtgStarters[c->starter].name);
        break;
    case ACT_BUY_PACK:
        if (Points() >= MTG_PACK_COST && Mtg_AwardPack(c, U.askArg)) {
            Tcg_GetCollection()->dust -= MTG_PACK_COST;
            PlaySE(SE_SHOP);
            MPNet_Log("mtg: bought a %s booster", gMtgSets[U.askArg].code);
            Say(ACT_NONE, "A %s booster went into your CARD CASE.", gMtgSets[U.askArg].code);
        } else
            Say(ACT_NONE, "You can't carry more %s boosters.", gMtgSets[U.askArg].code);
        break;
    case ACT_COPY: {
        int d = EmptySlot();
        if (d == U.askArg || d >= MTG_DECKS)
            Say(ACT_NONE, "There's no empty slot to copy into.");
        else {
            c->decks[d] = c->decks[U.askArg];
            c->ace[d] = c->ace[U.askArg];
            c->tray[d] = c->tray[U.askArg];
            snprintf(c->decks[d].name, MTG_DECK_NAME, "%.10s copy", c->decks[U.askArg].name);
            Say(ACT_NONE, "Copied into slot %d.", d + 1);
        }
        break;
    }
    case ACT_CLEAR:
        memset(&c->decks[U.askArg], 0, sizeof(c->decks[0]));
        c->ace[U.askArg] = -1;
        c->tray[U.askArg] = 0;
        if (c->activeDeck == U.askArg) {
            int d;
            for (d = 0; d < MTG_DECKS; d++)
                if (DeckValid(d)) {
                    c->activeDeck = (uint8_t)d;
                    break;
                }
        }
        break;
    case ACT_RECIPE: {
        int slot = U.recipeSlot >= 0 ? U.recipeSlot : EmptySlot(), i, lost;
        MtgSavedDeck d, built;
        uint8_t used[MTG_COLLECTION_CARDS];
        if (U.askArg < 0 || U.askArg >= MTG_STARTERS || slot >= MTG_DECKS)
            break;
        RecipeOf(U.askArg, &d);
        memset(&built, 0, sizeof(built));
        memset(used, 0, sizeof(used));
        for (i = 0; i < d.count; i++)   // (the cards owned; the missing ones are left out)
            if (Mtg_IsBasicLand(d.cards[i]) || used[d.cards[i]]++ < c->owned[d.cards[i]])
                Mtg_DeckAdd(&built, d.cards[i]);
        lost = d.count - built.count;
        snprintf(built.name, MTG_DECK_NAME, "%s", gMtgStarters[U.askArg].name);
        c->decks[slot] = built;
        c->ace[slot] = -1;
        c->tray[slot] = 0;
        MPNet_Log("mtg: recipe %s into slot %d, %d missing", gMtgStarters[U.askArg].name, slot, lost);
        Screen(M_DECKS);
        U.cursor = slot;
        if (lost)
            Say(ACT_NONE, "The %s deck is in slot %d. %d of its cards are missing: add them when you have them.",
                gMtgStarters[U.askArg].name, slot + 1, lost);
        else
            Say(ACT_NONE, "The %s deck is in slot %d, ready to duel.", gMtgStarters[U.askArg].name, slot + 1);
        break;
    }
    }
}

static void Filter(unsigned pressed, int count)
{
    U.filter = (U.filter + (pressed & KEY_R ? 1 : count - 1)) % count;
    U.cursor = 0;
    BuildList(U.withBasics);
    PlaySE(SE_SELECT);
}

static void StepBuilder(unsigned pressed)
{
    MtgCollection *c = C();
    MtgSavedDeck *d = &c->decks[U.deck];
    uint16_t listed[MTG_DECK_MAX];
    int uniq = DeckUnique(d, listed), page = GRID_COLS * GRID_ROWS;
    if (pressed & (KEY_L | KEY_R)) {
        Filter(pressed, FILTERS - 1);
        return;
    }
    if (pressed & KEY_B) {
        Mtg_DeckSort(d);
        PlaySE(SE_SELECT);
        Screen(M_DECKS);
        U.cursor = U.deck;
        if (d->count && Mtg_DeckProblem(c, d))
            Say(ACT_NONE, "This deck can't be played yet: %s", Mtg_DeckProblem(c, d));
        return;
    }
    if (U.side == 1) {   // the deck's list
        if (pressed & KEY_LEFT || !uniq) {
            U.side = 0;
            PlaySE(SE_SELECT);
            return;
        }
        if (pressed & KEY_UP && U.deckCursor > 0)
            U.deckCursor--, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN && U.deckCursor + 1 < uniq)
            U.deckCursor++, PlaySE(SE_SELECT);
        if (U.deckCursor >= uniq)
            U.deckCursor = uniq - 1;
        if (pressed & KEY_A) {
            Mtg_DeckRemove(d, listed[U.deckCursor]);
            PlaySE(SE_SELECT);
        } else if (pressed & KEY_INSPECT)
            Inspect(listed[U.deckCursor], M_BUILDER);
        return;
    }
    if (pressed & KEY_RIGHT && (U.cursor % GRID_COLS == GRID_COLS - 1 || U.cursor + 1 >= U.listCount) && uniq) {
        U.side = 1;
        PlaySE(SE_SELECT);
        return;
    }
    GridMove(pressed, U.listCount, GRID_COLS);
    (void)page;
    if (U.cursor >= U.listCount)
        return;
    {
        int card = U.list[U.cursor];
        if (pressed & KEY_A) {
            int copies = Mtg_DeckCopies(d, card);
            if (d->count >= MTG_DECK_MAX || (!Mtg_IsBasicLand(card) && (copies >= MTG_COPIES || copies >= c->owned[card])))
                PlaySE(SE_FAILURE);
            else {
                Mtg_DeckAdd(d, card);
                PlaySE(SE_SELECT);
            }
        }
        if (pressed & KEY_SELECT) {
            if (Mtg_DeckCopies(d, card)) {
                Mtg_DeckRemove(d, card);
                PlaySE(SE_SELECT);
            } else
                PlaySE(SE_FAILURE);
        }
        if (pressed & KEY_INSPECT) {
            int keep = U.cursor;
            Inspect(card, M_BUILDER);
            U.cursor = keep;
        }
    }
}

static void OpenBuilder(int k)
{
    MtgCollection *c = C();
    U.deck = k;
    if (!c->decks[k].name[0])
        snprintf(c->decks[k].name, MTG_DECK_NAME, "Deck %d", k + 1);
    U.filter = FILTERS - 1;   // (the builder's chips: the five colors, colorless, multicolor, lands; first all of them)
    U.side = 0;
    U.deckCursor = 0;
    U.showAll = 0;
    Screen(M_BUILDER);
    U.filter = 0;
    BuildList(1);
    {   // start on the deck's own color, as the POKeMON builder starts on a deck's type
        uint8_t colors = Mtg_DeckColors(&c->decks[k]);
        if (colors && !(colors & (colors - 1)))
            U.filter = __builtin_ctz(colors), BuildList(1);
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
        if (!U.rewarded && MtgBoard_Over()) {
            U.rewarded = 1;
            DuelEnded(MtgBoard_Over());
        }
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
        if (pressed & KEY_SELECT)
            TcgGuide_Open(KIT_GAME_MAGIC);
        else if (pressed & (KEY_L | KEY_R)) {
            PlaySE(SE_SELECT);
            ToSide((pressed & KEY_R) != 0);
        } else if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            switch (U.cursor) {
            case HOME_DECKS:
                Screen(M_DECKS);
                U.cursor = c->activeDeck;
                break;
            case HOME_BINDER:
                U.filter = FILTERS - 1;
                U.showAll = 0;
                Screen(M_BINDER);
                BuildList(0);
                break;
            case HOME_PACKS: {
                uint8_t sets[MTG_SETS];
                int n = PackSets(sets), k;
                Screen(M_PACKS);
                for (k = 0; k < n; k++)   // (on the first set with a booster to open)
                    if (c->packs[sets[k]]) {
                        U.cursor = k;
                        break;
                    }
                break;
            }
            case HOME_PRACTICE:
                Practice();
                break;
            case HOME_SPEED:
                Tcg_GetCollection()->fastAnimations = !Tcg_GetCollection()->fastAnimations;
                break;
            default:
                Close();
                break;
            }
        }
        break;
    case M_DECKS:
        if (TcgUi_TrayActive()) {
            if (TcgUi_TrayStep(pressed) == 1 && U.cursor < MTG_DECKS) {
                c->tray[U.cursor] = (uint8_t)TcgUi_TrayValue();
                MPNet_Log("mtg: deck %d tray %u", U.cursor, c->tray[U.cursor]);
            }
            break;
        }
        if (TcgUi_NameActive()) {
            if (TcgUi_NameStep(pressed) == 1 && U.cursor < MTG_DECKS) {
                snprintf(c->decks[U.cursor].name, MTG_DECK_NAME, "%s", TcgUi_NameText());
                MPNet_Log("mtg: deck %d renamed %s", U.cursor, c->decks[U.cursor].name);
            }
            break;
        }
        if (U.menuOn) {
            int empty = SlotEmpty(U.cursor), count = empty ? EM_COUNT : DM_COUNT;
            unsigned off = empty ? 0 : DeckMenuOff(U.cursor);
            U.menuCursor = TcgUi_PopMenuMove(pressed, U.menuCursor, count, off);
            if (pressed & KEY_B) {
                PlaySE(SE_SELECT);
                U.menuOn = 0;
            } else if (pressed & KEY_A && !((off >> U.menuCursor) & 1)) {
                int slot = U.cursor;
                PlaySE(SE_SELECT);
                U.menuOn = 0;
                if (empty && U.menuCursor == EM_RECIPE) {
                    Screen(M_RECIPES);
                    U.recipeSlot = slot;
                } else if (empty || U.menuCursor == DM_EDIT)
                    OpenBuilder(slot);
                else if (U.menuCursor == DM_RENAME)
                    TcgUi_NameOpen("RENAME DECK", c->decks[slot].name);
                else if (U.menuCursor == DM_ACE) {
                    uint16_t list[MTG_DECK_MAX];
                    int n = AceList(slot, list), ace = Mtg_DeckAce(c, slot), k;
                    U.deck = slot;
                    Screen(M_ACE);
                    for (k = 0; k < n; k++)
                        if (list[k] == ace)
                            U.cursor = k;
                } else if (U.menuCursor == DM_USE) {
                    c->activeDeck = (uint8_t)slot;
                    PlaySE(SE_SUCCESS);
                } else if (U.menuCursor == DM_COPY)
                    Ask(ACT_COPY, slot, "Copy %s into an empty slot?", c->decks[slot].name);
                else if (U.menuCursor == DM_DELETE)
                    Ask(ACT_CLEAR, slot, "Delete %s? Its cards stay in your collection.", c->decks[slot].name);
                else if (U.menuCursor == DM_TRAY)
                    TcgUi_TrayOpen(c->tray[slot], sTrayRgb, sTrayNames, 6, DeckMainRgb(&c->decks[slot]), "MAGIC");
            }
            break;
        }
        MoveCursor(pressed, DECK_ROWS);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == DECK_ROW_BACK)) {
            PlaySE(SE_SELECT);
            Screen(M_HOME);
        } else if (pressed & KEY_A && U.cursor == DECK_ROW_RECIPES) {
            PlaySE(SE_SELECT);
            Screen(M_RECIPES);
            U.recipeSlot = -1;
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            U.menuOn = 1;
            U.menuCursor = 0;
        } else if (pressed & KEY_R && U.cursor < MTG_DECKS && U.cursor != c->activeDeck && c->decks[U.cursor].count) {
            if (DeckValid(U.cursor)) {
                c->activeDeck = (uint8_t)U.cursor;
                PlaySE(SE_SUCCESS);
            } else {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "%s", Mtg_DeckProblem(c, &c->decks[U.cursor]));
            }
        } else if (pressed & KEY_START && U.cursor < MTG_DECKS && c->decks[U.cursor].count)
            Ask(ACT_COPY, U.cursor, "Copy %s into an empty slot?", c->decks[U.cursor].name);
        else if (pressed & KEY_SELECT && U.cursor < MTG_DECKS && c->decks[U.cursor].count)
            Ask(ACT_CLEAR, U.cursor, "Delete %s? Its cards stay in your collection.", c->decks[U.cursor].name);
        break;
    case M_RECIPES:
        MoveCursor(pressed, MTG_STARTERS);
        if (pressed & KEY_B) {
            int back = U.recipeSlot >= 0 ? U.recipeSlot : DECK_ROW_RECIPES;
            PlaySE(SE_SELECT);
            Screen(M_DECKS);
            U.cursor = back;
        } else if (pressed & KEY_A) {
            int slot = U.recipeSlot >= 0 ? U.recipeSlot : EmptySlot(), have, total = RecipeOwned(U.cursor, &have);
            PlaySE(SE_SELECT);
            if (slot >= MTG_DECKS)
                Say(ACT_NONE, "There's no empty deck slot. Delete a deck first.");
            else if (total - have)
                Ask(ACT_RECIPE, U.cursor, "Build the %s deck in slot %d? %d of its cards are missing.",
                    gMtgStarters[U.cursor].name, slot + 1, total - have);
            else
                Ask(ACT_RECIPE, U.cursor, "Build the %s deck in slot %d?", gMtgStarters[U.cursor].name, slot + 1);
        }
        break;
    case M_ACE: {
        uint16_t list[MTG_DECK_MAX];
        int n = AceList(U.deck, list);
        GridMove(pressed, n, ACE_COLS);
        if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Screen(M_DECKS);
            U.cursor = U.deck;
        } else if (pressed & KEY_INSPECT && n) {
            int keep = U.cursor;
            Inspect(list[U.cursor], M_ACE);
            U.cursor = keep;
        } else if (pressed & KEY_A && n) {
            c->ace[U.deck] = (int16_t)list[U.cursor];
            PlaySE(SE_SUCCESS);
            MPNet_Log("mtg: deck %d ace %s", U.deck, Mtg_Card(list[U.cursor])->name);
            Screen(M_DECKS);
            U.cursor = U.deck;
        }
        break;
    }
    case M_BUILDER:
        StepBuilder(pressed);
        break;
    case M_BINDER:
        if (pressed & (KEY_L | KEY_R))
            Filter(pressed, FILTERS);
        else if (pressed & KEY_SELECT) {   // every card, or the owned ones
            U.showAll ^= 1;
            U.cursor = 0;
            BuildList(0);
            PlaySE(SE_SELECT);
        } else if ((pressed & (KEY_A | KEY_INSPECT)) && U.cursor < U.listCount) {
            int keep = U.cursor;
            Inspect(U.list[U.cursor], M_BINDER);
            U.cursor = keep;
        } else if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Screen(M_HOME);
            U.cursor = HOME_BINDER;
        } else
            GridMove(pressed, U.listCount, 7);
        break;
    case M_INSPECT:
        if (pressed & KEY_UP && U.inspectScroll > 0)
            U.inspectScroll = U.inspectScroll > 8 ? U.inspectScroll - 8 : 0;
        if (pressed & KEY_DOWN && U.inspectScroll < U.inspectMax)
            U.inspectScroll = U.inspectScroll + 8 < U.inspectMax ? U.inspectScroll + 8 : U.inspectMax;
        if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            U.zoom = U.panX = U.panY = 0;
            U.screen = M_ART;
        } else if (pressed & (KEY_B | KEY_INSPECT)) {
            PlaySE(SE_SELECT);
            U.screen = U.back;
        }
        break;
    case M_ART:
        if (pressed & KEY_A)
            U.zoom ^= 1, U.panX = U.panY = 0, PlaySE(SE_SELECT);
        if (U.zoom) {
            if (keys & KEY_LEFT && U.panX > -90)
                U.panX -= 2;
            if (keys & KEY_RIGHT && U.panX < 90)
                U.panX += 2;
            if (keys & KEY_UP && U.panY > -70)
                U.panY -= 2;
            if (keys & KEY_DOWN && U.panY < 70)
                U.panY += 2;
        }
        if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            U.screen = M_INSPECT;
        }
        break;
    case M_STARTER:
        if (pressed & KEY_LEFT)
            U.cursor = (U.cursor + MTG_STARTERS - 1) % MTG_STARTERS, PlaySE(SE_SELECT);
        if (pressed & KEY_RIGHT)
            U.cursor = (U.cursor + 1) % MTG_STARTERS, PlaySE(SE_SELECT);
        if (pressed & KEY_B) {   // (not now: the clerk will ask again)
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            Ask(ACT_TAKE_STARTER, U.cursor, "Take the %s deck? (%s)", gMtgStarters[U.cursor].name,
                ColorWords(gMtgStarters[U.cursor].colors));
        } else if (pressed & KEY_INSPECT) {
            int keep = U.cursor;
            Inspect(RecipeAce(U.cursor), M_STARTER);
            U.cursor = keep;
        }
        break;
    case M_CHALLENGE:
        MoveCursor(pressed, MTG_DECKS + 1);
        if (pressed & KEY_B || (pressed & KEY_A && U.cursor == MTG_DECKS)) {
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            if (!c->decks[U.cursor].count)
                Say(ACT_NONE, "That slot is empty.");
            else if (!DeckValid(U.cursor))
                Say(ACT_NONE, "%s", Mtg_DeckProblem(c, &c->decks[U.cursor]));
            else {
                c->activeDeck = (uint8_t)U.cursor;
                StartDuel(gMtgDuelists[U.duelist].cards, gMtgDuelists[U.duelist].name);
            }
        }
        break;
    case M_SPOILS:
        U.revealAge++;
        SpoilsStep(pressed);
        break;
    case M_PACKS: {
        uint8_t sets[MTG_SETS];
        int n = PackSets(sets);
        if (pressed & KEY_LEFT && U.cursor > 0)
            U.cursor--, PlaySE(SE_SELECT);
        if (pressed & KEY_RIGHT && U.cursor + 1 < n)
            U.cursor++, PlaySE(SE_SELECT);
        if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Screen(M_HOME);
            U.cursor = HOME_PACKS;
        } else if (pressed & KEY_A && n) {
            int set = sets[U.cursor];
            if (c->packs[set]) {
                PlaySE(SE_SELECT);
                OpenPack(set);
            } else {
                PlaySE(SE_FAILURE);
                Say(ACT_NONE, "No %s boosters yet: win duels to earn them, or buy them at MOSSDEEP's CARD SHOP.",
                    gMtgSets[set].code);
            }
        }
        break;
    }
    case M_SHOP: {
        uint8_t sets[MTG_SETS];
        int n = ShopRows(sets);
        MoveCursor(pressed, n);
        if (pressed & KEY_B) {
            PlaySE(SE_SELECT);
            Close();
        } else if (pressed & KEY_A && n) {
            PlaySE(SE_SELECT);
            if (Points() >= MTG_PACK_COST)
                Ask(ACT_BUY_PACK, sets[U.cursor], "A %s booster for %u CARD POINTS?", gMtgSets[sets[U.cursor]].code,
                    MTG_PACK_COST);
            else
                Say(ACT_NONE, "A booster takes %u CARD POINTS. You have %u.", MTG_PACK_COST, Points());
        }
        break;
    }
    }
    return 1;
}
