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
#include "tcg/tcg.h"
#include "ygo/card_ids.h"
#include "ygo/duel.h"
#include "ygo/integration.h"
#include "ygo/npc.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_ui_kit.h"
#include "ygo_board.h"
#include "ygo_gfx.h"

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
    Y_STARTER
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
enum
{
    VS_NPC,
    VS_PRACTICE,
    VS_LESSON,
    VS_FRIEND
};
enum
{
    ACT_NONE,
    ACT_CLOSE,
    ACT_HOME,
    ACT_TAKE_SPOIL,
    ACT_STARTER,
    ACT_CLEAR,
    ACT_TRADE_IN,
    ACT_TRADE_IN_ALL,
    ACT_BUY_PACK,
    ACT_BUY_CARD,
    ACT_GC_CARD,
    ACT_GC_DECK,
    ACT_LESSON
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
    SDL_Texture *portrait;
    uint16_t spoils[YGO_DECK_MAX + YGO_FUSION_MAX];
    int spoilCount;
    int tab, counterTab, gc;   /* gc: the counter is the GAME CENTER clerk's (SINGLES and DECKS first) */
    char message[256];
    int messageThen, asking, askYes, askAction, askArg;
    int fromScript;
    void (*returnToGame)(void);
} Y;

static struct YgoCollection *C(void) { return Ygo_GetCollection(); }
static int sForceFirst = -1;   /* test harness: who goes first (-1: a coin) */
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
unsigned YgoUi_DuelingNow(void) { return Y.screen == Y_DUEL || (Y.screen == Y_INSPECT && Y.back == Y_DUEL); }

static void Close(void)
{
    void (*callback)(void) = Y.returnToGame;
    int script = Y.fromScript;
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
/* Back to the Pokemon side of the case */
static void ToPokemon(void)
{
    void (*callback)(void) = Y.returnToGame;
    int script = Y.fromScript;
    Y.returnToGame = NULL;
    Y.fromScript = 0;
    Y.message[0] = 0;
    Screen(Y_CLOSED);
    Ygo_ForgetTextures();
    Tcg_OpenCaseSide(callback, script);
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
static int DeckValid(unsigned d)
{
    return Ygo_DeckValidate(Deck(d)->main, Deck(d)->extra, C()->owned, NULL, 0);
}
/* A deck's face: its strongest monster */
static unsigned Mascot(const uint16_t *main, unsigned count)
{
    unsigned i, best = 0;
    int bestAtk = -1;
    for (i = 0; i < count && main[i]; i++)
    {
        const struct YgoCard *c = Ygo_Card(main[i]);
        int atk = c && Ygo_IsMonster(c) && c->atk != YGO_STAT_UNKNOWN ? c->atk : -1;
        if (atk > bestAtk)
            bestAtk = atk, best = main[i];
    }
    return best;
}
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
static int StartDuel(const uint16_t *main, const uint16_t *extra, const uint16_t *theirMain, const uint16_t *theirExtra)
{
    uint32_t seed = Tcg_Random(&C()->rng);
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
static void DuelStep(unsigned pressed, unsigned keys)
{
    struct YgoAnswer a;
    unsigned card = 0;
    int r;
    YgoBoard_Sync(&Y.duel);
    if (Y.duel.s.over && !Y.rewarded && YgoBoard_Idle())
    {
        int won = Y.duel.s.winner == 0;
        Y.rewarded = 1;
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
        else
            YgoBoard_Result(won ? "Nothing was wagered: a practice win!" : "Nothing was wagered.");
    }
    /* the opponent (and the harness's autoplay for you) */
    if (!Y.duel.s.over && Y.duel.query.type != YGO_Q_NONE && YgoBoard_Idle() &&
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
    if (r == YB_ANSWER)
    {
        if (!YgoDuel_Answer(&Y.duel, &a))
        {
            printf("[ygo] answer refused: %s\n", Y.duel.error);
            PlaySE(SE_FAILURE);
        }
        Y.aiDelay = Tcg_GetCollection()->fastAnimations ? 6 : 16;
    }
    else if (r == YB_CONCEDE)
        YgoDuel_Concede(&Y.duel, 0);
    else if (r == YB_INSPECT)
        Inspect(card, Y_DUEL);
    else if (r == YB_LEAVE)
    {
        int won = Y.duel.s.winner == 0;
        printf("[ygo] left the duel (%s)\n", won ? "won" : "lost");
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
    unsigned mascot = c->optedIn ? Mascot(Deck(c->activeDeck)->main, YGO_DECK_MAX) : 0;
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
    TcgUi_Hints("{L}{R} POKeMON");
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

static void DecksScreen(void)
{
    struct YgoCollection *c = C();
    unsigned i, d = Y.cursor < YGO_SAVED_DECKS ? (unsigned)Y.cursor : c->activeDeck, mascot;
    char line[96];
    TcgUi_Background();
    TcgUi_SidePanel("DECKS", YGO_THEME);
    mascot = Mascot(Deck(d)->main, YGO_DECK_MAX);
    if (mascot)
        BigCard(mascot, 30, 22, 0);
    snprintf(line, sizeof(line), "%u CARDS  +%u", MainCount(d), ExtraCount(d));
    G_TextAlign(52, 86, TA_CENTER, DeckValid(d) ? 0x2858a8 : 0xc04040, G_NO_SHADOW, line, TF_SMALL);
    TcgUi_ListBox(LIST_X, 4, LIST_W, 104);
    Y.scroll = Y.cursor < 3 ? 0 : Y.cursor > YGO_SAVED_DECKS - 3 ? YGO_SAVED_DECKS - 5 : Y.cursor - 3;
    for (i = 0; i < 6; i++)
    {
        unsigned k = (unsigned)Y.scroll + i;
        if (k > YGO_SAVED_DECKS)
            break;
        if (k == YGO_SAVED_DECKS)
        {
            TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)k, 1, "BACK", NULL);
            continue;
        }
        snprintf(line, sizeof(line), "%s", Deck(k)->name[0] ? Deck(k)->name : "EMPTY SLOT");
        TcgUi_Row(LIST_X + 2, 8 + (int)i * ROW_H, LIST_W - 4, Y.cursor == (int)k, Deck(k)->name[0] != 0, line,
                  k == c->activeDeck ? "IN USE" : NULL);
    }
    if (Y.cursor == YGO_SAVED_DECKS)
        Strip(0, "Back to the CARD CASE.");
    else if ((unsigned)Y.cursor == c->activeDeck)
        Strip(0, "The deck you duel with. A Main Deck holds 40 to 60 cards, the Fusion Deck up to 15.");
    else
        Strip(0, MainCount((unsigned)Y.cursor) ? "{R} DUEL WITH THIS DECK" : "An empty slot: {A} builds a new deck.");
    TcgUi_Hints("{A} EDIT  {START} COPY  {SELECT} DELETE  {B} BACK");
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
            snprintf(line, sizeof(line), "%s", card->name);
            while (G_Measure(line, TF_NARROW) > 52 && strlen(line) > 3)
                line[strlen(line) - 1] = 0;
            G_Text(169, y - 1, G_INK, G_NO_SHADOW, line, 0, TF_NARROW);
            snprintf(line, sizeof(line), "x%u", InDeck((unsigned)Y.deck, card->id));
            G_Digits(236 - G_DigitsWidth(line, 0, 1), y + 2, line, 0, 1, 0x2858a8, 255);
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
    TcgUi_Hints(Y.side ? "{A} TAKE OUT  {LEFT} CARDS  {START} READ  {B} DONE"
                       : "{A} ADD  {SELECT} TAKE OUT  {START} READ  {B} DONE");
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
        TcgUi_Hints("{A} BUY  {L}{R} KIND  {START} READ  {B} BACK");
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
        TcgUi_Hints("{A} BUY  {L}{R} KIND  {START} READ  {B} BACK");
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
    TcgUi_Hints(Y.showAll ? "{A} READ  {L}{R} KIND  {SELECT} OWNED  {B} BACK" : "{A} READ  {L}{R} KIND  {SELECT} ALL  {B} BACK");
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
            G_TextAlign((int)sx + 15, (int)sy + 42, TA_CENTER, RarityInk(rarity, Y.frame), 0x302040,
                        rarity == YGO_SECRET_RARE ? "SECRET!" : rarity == YGO_ULTRA_RARE ? "ULTRA!" : rarity == YGO_SUPER_RARE ? "SUPER!" : "RARE!",
                        TF_NARROW);
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
        TcgUi_Hints("{A} READ  {DPAD} CHOOSE  {B} DONE");
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
    TcgUi_Hints("{A} TAKE  {START} READ");
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
    int i, x = 4;
    TcgUi_Background();
    TcgUi_SidePanel(Y.gc ? "GAME CENTER" : "CARD CLUB", YGO_THEME);
    for (i = 0; i < CounterTabs(); i++)
    {
        const char *name = sCounterTabs[Y.gc ? i : i + 2];
        int w = G_Measure(name, TF_NARROW) + 8, on = i == Y.counterTab;
        if (Y.gc && i == 2)
            x = 4;   /* two rows of two */
        G_RoundFill(x, Y.gc && i >= 2 ? 34 : 20, w, 12, on ? 0xf8f8f8 : 0x504070, 255);
        G_Text(x + 4, Y.gc && i >= 2 ? 34 : 20, on ? G_INK : 0xe0e0f0, G_NO_SHADOW, name, 0, TF_NARROW);
        x += w + 2;
    }
    if (CounterKind() < 2)
    {
        snprintf(line, sizeof(line), "\xa5%u", (unsigned)GetMoney(&gSaveBlock1Ptr->money));
        G_Text(8, Y.gc ? 54 : 40, 0x505868, G_NO_SHADOW, "MONEY", 0, TF_NARROW);
    }
    else
    {
        snprintf(line, sizeof(line), "%u", Points());
        G_Text(8, Y.gc ? 54 : 40, 0x505868, G_NO_SHADOW, "CARD POINTS", 0, TF_NARROW);
    }
    G_Text(8, Y.gc ? 64 : 50, G_INK, G_INK_SHADOW, line, 0, TF_NORMAL);
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
            G_Text(LIST_X + 6, 10, 0x707878, G_NO_SHADOW, "No spare cards.", LIST_W - 10, TF_SMALL);
        for (k = 0; k < 6 && top + (int)k <= (int)n; k++)
        {
            int row = top + (int)k;
            if (row == (int)n)
            {
                if (n)
                    TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, 1, "ALL SPARES", NULL);
                continue;
            }
            snprintf(line, sizeof(line), "x%u +%u", Ygo_Spare(C(), list[row]), Ygo_PointValue(list[row]));
            TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, 1, Ygo_Card(list[row])->name, line);
        }
        Strip(Y.cursor < (int)n ? list[Y.cursor] : 0,
              n ? "Cards past 3 copies and past what your decks use, traded for CARD POINTS." : "Nothing to trade in.");
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
                TcgUi_Row(LIST_X + 2, 8 + (int)k * ROW_H, LIST_W - 4, Y.cursor == row, 1, "A CARD YOU PICK", NULL);
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
            Strip(0, "Any card that can be played, for its CARD POINTS (by rarity).");
    }
    TcgUi_Hints("{L}{R} TAB  {A} OK  {B} LEAVE");
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
    TcgUi_Hints("{LEFTRIGHT} CHOOSE  {A} TAKE  {START} LOOK");
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
    StartDuel(Deck(c->activeDeck)->main, Deck(c->activeDeck)->extra, Y.opp.main, Y.opp.extra);
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
    PlaySE(SE_RG_CARD_FLIP);
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
            Say(ACT_CLOSE, "You got the %s! It's in your CARD CASE, on its YU-GI-OH! side.", gYgoSets[set].name);
        }
        break;
    }
    case ACT_CLEAR:
        memset(Deck((unsigned)Y.askArg), 0, sizeof(struct YgoSavedDeck));
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
    {
        unsigned id = (unsigned)Y.askArg, n = Ygo_Spare(c, id);
        c->owned[id] -= n;
        AddPoints(n * Ygo_PointValue(id));
        PlaySE(SE_RG_SHOP);
        break;
    }
    case ACT_TRADE_IN_ALL:
    {
        uint16_t list[YGO_CARD_COUNT];
        unsigned n = SpareList(list), i, points = 0;
        for (i = 0; i < n; i++)
        {
            unsigned k = Ygo_Spare(c, list[i]);
            c->owned[list[i]] -= k;
            points += k * Ygo_PointValue(list[i]);
        }
        AddPoints(points);
        Y.cursor = 0;
        PlaySE(SE_RG_SHOP);
        Say(ACT_NONE, "Traded in for %u CARD POINTS.", points);
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
            ToPokemon();
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
        MoveCursor(pressed, YGO_SAVED_DECKS + 1, 1);
        if (pressed & KEY_B || (pressed & KEY_A && Y.cursor == YGO_SAVED_DECKS))
        {
            PlaySE(SE_SELECT);
            Screen(Y_HOME);
        }
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            Y.deck = Y.cursor;
            if (!Deck((unsigned)Y.deck)->name[0])
                snprintf(Deck((unsigned)Y.deck)->name, YGO_NAME_LEN, "Deck %d", Y.deck + 1);
            Y.side = 0;
            Y.deckCursor = 0;
            Y.filter = 0;
            Screen(Y_BUILDER);
        }
        else if (pressed & KEY_R && Y.cursor < YGO_SAVED_DECKS && (unsigned)Y.cursor != c->activeDeck &&
                 MainCount((unsigned)Y.cursor))
        {
            char reason[160];
            if (Ygo_DeckValidate(Deck((unsigned)Y.cursor)->main, Deck((unsigned)Y.cursor)->extra, c->owned, reason,
                                 sizeof(reason)))
            {
                c->activeDeck = (uint8_t)Y.cursor;
                PlaySE(SE_SELECT);
            }
            else
                Say(ACT_NONE, "%s", reason);
        }
        else if (pressed & KEY_START && Y.cursor < YGO_SAVED_DECKS && MainCount((unsigned)Y.cursor))
        {
            unsigned d;
            for (d = 0; d < YGO_SAVED_DECKS; d++)
                if (!Deck(d)->name[0] && !MainCount(d))
                {
                    *Deck(d) = *Deck((unsigned)Y.cursor);
                    snprintf(Deck(d)->name, YGO_NAME_LEN, "%.18s copy", Deck((unsigned)Y.cursor)->name);
                    Say(ACT_NONE, "Copied to slot %u.", d + 1);
                    break;
                }
            if (d == YGO_SAVED_DECKS)
                Say(ACT_NONE, "No empty slot to copy into.");
        }
        else if (pressed & KEY_SELECT && Y.cursor < YGO_SAVED_DECKS && Deck((unsigned)Y.cursor)->name[0])
            Ask(ACT_CLEAR, Y.cursor, "Delete %s?", Deck((unsigned)Y.cursor)->name);
        break;
    case Y_BUILDER:
    {
        uint16_t list[YGO_DECK_MAX + YGO_FUSION_MAX];
        unsigned listed = DeckList((unsigned)Y.deck, list);
        char reason[160];
        if (Y.side == 0)
        {
            int n = (int)ListSize();
            FilterTab(pressed);
            if (pressed & KEY_RIGHT && (Y.cursor % GRID_COLS == GRID_COLS - 1 || Y.cursor == n - 1) && listed)
            {
                Y.side = 1;
                PlaySE(SE_SELECT);
                break;
            }
            GridMove(pressed, n, GRID_COLS);
            if (pressed & KEY_A && n)
            {
                if (Ygo_DeckAdd(c, (unsigned)Y.deck, ListCard(Y.cursor), reason, sizeof(reason)))
                    PlaySE(SE_SELECT);
                else
                {
                    PlaySE(SE_FAILURE);
                    Say(ACT_NONE, "%s", reason);
                }
            }
            if (pressed & KEY_SELECT && n && Ygo_DeckRemove(c, (unsigned)Y.deck, ListCard(Y.cursor)))
                PlaySE(SE_SELECT);
            if (pressed & KEY_START && n)
                Inspect(ListCard(Y.cursor), Y_BUILDER);
        }
        else
        {
            if (pressed & KEY_UP && Y.deckCursor > 0)
                Y.deckCursor--, PlaySE(SE_SELECT);
            if (pressed & KEY_DOWN && Y.deckCursor < (int)listed - 1)
                Y.deckCursor++, PlaySE(SE_SELECT);
            if (pressed & KEY_LEFT || !listed)
                Y.side = 0;
            if (pressed & KEY_A && listed)
            {
                Ygo_DeckRemove(c, (unsigned)Y.deck, list[Y.deckCursor]);
                PlaySE(SE_SELECT);
                listed = DeckList((unsigned)Y.deck, list);
                if (Y.deckCursor >= (int)listed)
                    Y.deckCursor = listed ? (int)listed - 1 : 0;
            }
            if (pressed & KEY_START && listed)
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
                Say(ACT_NONE, "Saved, but it can't duel yet: %s", reason);
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
                    Say(ACT_NONE, "That one's sold. One copy of each card per customer!");
                }
                else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
                    Say(ACT_NONE, "%s is \xa5%u. You don't have enough money.", Ygo_Card(id)->name, price);
                else
                    Ask(ACT_GC_CARD, (int)id, "%s for \xa5%u?", Ygo_Card(id)->name, price);
            }
            else if (Y.showAll == SHOP_BINDER)
            {
                unsigned cost = Ygo_PointCost(id);
                if (Points() >= cost)
                    Ask(ACT_BUY_CARD, (int)id, "%s for %u CARD POINTS?", Ygo_Card(id)->name, cost);
                else
                    Say(ACT_NONE, "That takes %u CARD POINTS. You have %u.", cost, Points());
            }
            else
                Inspect(id, Y_BINDER);
        }
        else if (pressed & KEY_START && n)
            Inspect(ListCard(Y.cursor), Y_BINDER);
        break;
    }
    case Y_INSPECT:
        if (pressed & KEY_UP && Y.inspectScroll > 0)
            Y.inspectScroll -= 8;
        if (pressed & KEY_DOWN && Y.inspectScroll < Y.inspectMax)
            Y.inspectScroll += 8;
        if (Y.inspectScroll < 0)
            Y.inspectScroll = 0;
        if (pressed & KEY_A)
        {
            Y.screen = Y_ART;
            PlaySE(SE_SELECT);
        }
        else if (pressed & (KEY_B | KEY_START | KEY_R))
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
                PlaySE(SE_FAILURE);
        }
        break;
    }
    case Y_REVEAL:
        if (Y.age < REVEAL_TEAR + 14)
        {
            if (pressed & KEY_B)
                Y.age = REVEAL_TEAR + 14;
            break;
        }
        if (Y.flipped < YGO_PACK_SIZE)
        {
            if (pressed & KEY_A)
            {
                Y.flipped++;
                PlaySE(Y.flipped == YGO_PACK_SIZE && Y.revealRarity[YGO_PACK_SIZE - 1] >= YGO_SUPER_RARE ? SE_SHINY
                                                                                                          : SE_RG_CARD_FLIP);
                if (Y.flipped == YGO_PACK_SIZE)
                    Y.cursor = YGO_PACK_SIZE - 1;
            }
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
        if (pressed & KEY_A)
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
            PlaySE(SE_SELECT);
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
            if (pressed & KEY_A && n)
            {
                if (Y.cursor == (int)n)
                    Ask(ACT_TRADE_IN_ALL, 0, "Trade in every spare card?");
                else
                    Ask(ACT_TRADE_IN, list[Y.cursor], "Trade in %u %s for %u CARD POINTS?", Ygo_Spare(c, list[Y.cursor]),
                        Ygo_Card(list[Y.cursor])->name, Ygo_Spare(c, list[Y.cursor]) * Ygo_PointValue(list[Y.cursor]));
            }
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
    case Y_STARTER:
        MoveCursor(pressed, YGO_STARTERS, 0);
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            Ask(ACT_STARTER, Y.cursor, "Take the %s?", gYgoSets[sStarters[Y.cursor]].name);
        }
        else if (pressed & KEY_START)
        {
            uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
            StarterDeckOf(sStarters[Y.cursor], main, extra);
            Inspect(Mascot(main, YGO_DECK_MAX), Y_STARTER);
        }
        else if (pressed & KEY_B && c->starter)
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
    if (!strcmp(command, "win"))
    {
        YgoDuel_Concede(&Y.duel, 1);
        printf("[ygo] opponent conceded\n");
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
               "message %d conserved %d\n",
               Y.screen, c->optedIn, c->starter, MainCount(c->activeDeck), ExtraCount(c->activeDeck),
               DeckValid(c->activeDeck), have, total, Points(), PacksHeld(), c->wins, s->turn, s->phase, s->current,
               Y.duel.query.type, Y.duel.query.player, s->players[0].lp, s->players[1].lp, s->players[0].handCount,
               s->players[1].handCount, s->over, s->winner, Y.screen == Y_DUEL ? YgoBoard_Idle() : -1,
               Y.message[0] != 0, Y.screen == Y_DUEL ? YgoDuel_Conserved(&Y.duel) : -1);
        return;
    }
}
