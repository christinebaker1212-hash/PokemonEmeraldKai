// Moemon Companion: TRIPLE TRIAD on the phone with the game's own engine (src/tt: the rules, the AI, the catalog, the
// region hands and the ratings that fit an opponent to the player), for the app's Java (Tt.java). A duel lives here
// (a handle per game); Java asks for its state and events as JSON. The daily puzzle is made here too: a position
// three cards from the end where only one or two plays win against the best defense, the same for everyone that day.
#include <jni.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tt/duel.h"
#include "tt/npc.h"
#include "tt/rating.h"

#define FN(name) Java_com_moemon_companion_Tt_##name

struct Buf
{
    char *p;
    size_t len, cap;
};

static void Add(struct Buf *b, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void Add(struct Buf *b, const char *fmt, ...)
{
    va_list ap;
    int n;
    for (;;)
    {
        size_t room = b->cap - b->len;
        va_start(ap, fmt);
        n = vsnprintf(b->p ? b->p + b->len : NULL, b->p ? room : 0, fmt, ap);
        va_end(ap);
        if (n >= 0 && b->p && (size_t)n < room)
        {
            b->len += (size_t)n;
            return;
        }
        b->cap = b->cap * 2 + (size_t)n + 256;
        b->p = realloc(b->p, b->cap);
        if (b->p == NULL)
            return;
    }
}

// a JSON string (the catalog's words are plain ASCII; quotes and backslashes escaped, control characters dropped)
static void AddStr(struct Buf *b, const char *s)
{
    Add(b, "\"");
    for (; s && *s; s++)
    {
        if (*s == '"' || *s == '\\')
            Add(b, "\\%c", *s);
        else if ((unsigned char)*s >= 32)
            Add(b, "%c", *s);
    }
    Add(b, "\"");
}

static jstring Done(JNIEnv *env, struct Buf *b)
{
    jstring s = b->p ? (*env)->NewStringUTF(env, b->p) : NULL;
    free(b->p);
    return s;
}

static void Hand(JNIEnv *env, jintArray a, uint16_t out[TT_HAND])
{
    jint v[TT_HAND] = {0};
    unsigned i;
    if (a != NULL && (*env)->GetArrayLength(env, a) >= TT_HAND)
        (*env)->GetIntArrayRegion(env, a, 0, TT_HAND, v);
    for (i = 0; i < TT_HAND; i++)
        out[i] = (uint16_t)v[i];
}

// ------------------------------------------------------------------------------------------------- the tables

JNIEXPORT jstring JNICALL FN(catalog)(JNIEnv *env, jclass c)
{
    struct Buf b = {0};
    unsigned id, n = 0;
    Add(&b, "[");
    for (id = 1; id <= TT_CARD_MAX; id++)
    {
        const struct TtCard *k = Tt_Card(id);
        if (k == NULL)
            continue;
        Add(&b, "%s{\"id\":%u,\"n\":", n++ ? "," : "", id);
        AddStr(&b, k->name);
        Add(&b, ",\"s\":[%u,%u,%u,%u],\"st\":%u,\"ty\":%u,\"o\":%u,\"r\":%d,\"t\":", k->side[0], k->side[1], k->side[2],
            k->side[3], k->stars, k->type, k->order, gTtCardRating[id]);
        AddStr(&b, k->text);
        Add(&b, "}");
    }
    Add(&b, "]");
    return Done(env, &b);
}

JNIEXPORT jstring JNICALL FN(words)(JNIEnv *env, jclass c)
{
    struct Buf b = {0};
    unsigned i;
    Add(&b, "{\"rules\":[");
    for (i = 0; i < TT_RULE_COUNT; i++)
    {
        Add(&b, "%s[", i ? "," : "");
        AddStr(&b, Tt_RuleName(i));
        Add(&b, ",");
        AddStr(&b, Tt_RuleHelp(i));
        Add(&b, "]");
    }
    Add(&b, "],\"trades\":[");
    for (i = 0; i < TT_TRADE_COUNT; i++)
    {
        Add(&b, "%s[", i ? "," : "");
        AddStr(&b, Tt_TradeName(i));
        Add(&b, ",");
        AddStr(&b, Tt_TradeHelp(i));
        Add(&b, "]");
    }
    Add(&b, "],\"types\":[");
    for (i = 0; i < TT_TYPE_COUNT; i++)
    {
        Add(&b, "%s", i ? "," : "");
        AddStr(&b, Tt_TypeName(i));
    }
    Add(&b, "],\"packs\":[");
    for (i = 0; i < TT_PACK_TYPES; i++)
    {
        Add(&b, "%s", i ? "," : "");
        AddStr(&b, Tt_PackName(i));
    }
    Add(&b, "],\"regions\":[");
    for (i = 0; i < TT_REGIONS; i++)
    {
        Add(&b, "%s[", i ? "," : "");
        AddStr(&b, Tt_RegionName(i));
        Add(&b, ",%u,%u]", (unsigned)Tt_RegionStartRules(i), Tt_RegionStartTrade(i));
    }
    Add(&b, "]}");
    return Done(env, &b);
}

JNIEXPORT jint JNICALL FN(handPower)(JNIEnv *env, jclass c, jintArray hand)
{
    uint16_t h[TT_HAND];
    Hand(env, hand, h);
    return Tt_HandPower(h);
}

JNIEXPORT jint JNICALL FN(chance)(JNIEnv *env, jclass c, jint mine, jint theirs)
{
    return (jint)Tt_Chance(mine, theirs);
}

// A duelist of REGION as the game makes them (tt_field.c Fill), their hand fitted to MINE (the player's hand power and
// skill) under RULES as the game's CARD SCALING does: {"hand":[...],"level":n,"power":n}
JNIEXPORT jstring JNICALL FN(opponent)(JNIEnv *env, jclass c, jint region, jint hash, jint tier, jint day, jint mine,
                                       jint rules)
{
    static struct TtOpponent o;
    struct Buf b = {0};
    uint16_t hand[TT_HAND];
    unsigned i;
    int power;
    memset(&o, 0, sizeof(o) - sizeof(o.portrait));
    o.region = (uint8_t)(region >= 0 && region < TT_REGIONS ? region : 0);
    o.hash = (uint32_t)hash;
    o.tier = (uint8_t)(tier < 0 ? 0 : tier > 3 ? 3 : tier);
    o.fit = 1;
    o.special = TT_SPECIAL_NONE;
    o.level = (uint8_t)(o.tier >= 2 ? 2 : 1);
    if (((uint32_t)hash >> 13) % 4 == 0 && o.level > 0)
        o.level--;
    Tt_RegionHand(o.region, o.hash, o.tier, (unsigned)day, o.hand);
    memcpy(hand, o.hand, sizeof(hand));
    power = Tt_FitOpponentHand(&o, mine, (uint32_t)rules, (unsigned)day, hand);
    Add(&b, "{\"hand\":[");
    for (i = 0; i < TT_HAND; i++)
        Add(&b, "%s%u", i ? "," : "", hand[i]);
    Add(&b, "],\"level\":%u,\"power\":%d,\"favor\":%d,\"levelRating\":%d,\"playerRating\":%d}", o.level, power,
        Tt_OpponentFavor(&o), Tt_LevelRating(o.level, (uint32_t)rules), Tt_PlayerRating((uint32_t)rules));
    return Done(env, &b);
}

// ------------------------------------------------------------------------------------------------------ duels

JNIEXPORT jlong JNICALL FN(start)(JNIEnv *env, jclass c, jint rules, jint trade, jint seed, jintArray hand0,
                                  jintArray hand1, jint first)
{
    struct TtDuel *d = calloc(1, sizeof(*d));
    uint16_t h0[TT_HAND], h1[TT_HAND];
    if (d == NULL)
        return 0;
    Hand(env, hand0, h0);
    Hand(env, hand1, h1);
    TtDuel_Start(d, (uint32_t)rules, (unsigned)trade, (uint32_t)seed, h0, h1, (unsigned)first);
    return (jlong)(intptr_t)d;
}

JNIEXPORT void JNICALL FN(end)(JNIEnv *env, jclass c, jlong h)
{
    free((void *)(intptr_t)h);
}

JNIEXPORT jlong JNICALL FN(copy)(JNIEnv *env, jclass c, jlong h)
{
    struct TtDuel *d = malloc(sizeof(*d));
    if (d != NULL && h)
        memcpy(d, (void *)(intptr_t)h, sizeof(*d));
    return (jlong)(intptr_t)d;
}

static void AddEvents(struct Buf *b, const struct TtDuel *d)
{
    unsigned i;
    Add(b, "[");
    for (i = 0; i < d->evCount; i++)
    {
        const struct TtEvent *e = &d->ev[i];
        Add(b, "%s{\"k\":%u,\"cell\":%u,\"dir\":%u,\"by\":%u,\"wave\":%u,\"seat\":%u,\"slot\":%u,\"n\":%d,\"card\":%u}",
            i ? "," : "", e->kind, e->cell, e->dir, e->by, e->wave, e->seat, e->slot, e->amount, e->card);
    }
    Add(b, "]");
}

// Plays SLOT onto CELL for the side to move: its events, or null when it can't be played
JNIEXPORT jstring JNICALL FN(play)(JNIEnv *env, jclass c, jlong h, jint slot, jint cell)
{
    struct TtDuel *d = (struct TtDuel *)(intptr_t)h;
    struct Buf b = {0};
    if (d == NULL || slot < 0 || cell < 0 || !TtDuel_Play(d, (unsigned)slot, (unsigned)cell))
        return NULL;
    AddEvents(&b, d);
    return Done(env, &b);
}

JNIEXPORT jint JNICALL FN(playable)(JNIEnv *env, jclass c, jlong h)
{
    return h ? (jint)TtDuel_Playable((const struct TtDuel *)(intptr_t)h) : 0;
}

// The board and the hands as SEAT may see them (the other side's hidden cards as 0)
JNIEXPORT jstring JNICALL FN(state)(JNIEnv *env, jclass c, jlong h, jint seat)
{
    const struct TtDuel *d = (const struct TtDuel *)(intptr_t)h;
    struct Buf b = {0};
    unsigned i, s;
    if (d == NULL)
        return NULL;
    Add(&b, "{\"rules\":%u,\"trade\":%u,\"turn\":%u,\"toMove\":%u,\"first\":%u,\"round\":%u,\"over\":%u,\"winner\":%u,"
            "\"forced\":%u,\"score\":[%u,%u],\"shift\":[",
        (unsigned)d->rules, d->trade, d->turn, d->toMove, d->first, d->round, d->over, d->winner, d->forced,
        TtDuel_Score(d, 0), TtDuel_Score(d, 1));
    for (i = 0; i < TT_TYPE_COUNT; i++)
        Add(&b, "%s%d", i ? "," : "", TtDuel_TypeShift(d, i));
    Add(&b, "],\"cells\":[");
    for (i = 0; i < TT_CELLS; i++)
        Add(&b, "%s[%u,%u,%u,%u,%d]", i ? "," : "", d->cell[i].card, d->cell[i].owner, d->cell[i].origin, d->cell[i].tile,
            d->cell[i].elem);
    Add(&b, "],\"hands\":[");
    for (s = 0; s < 2; s++)
    {
        Add(&b, "%s[", s ? "," : "");
        for (i = 0; i < TT_HAND; i++)
        {
            int visible = s == (unsigned)seat || (d->shown[s] >> i & 1);
            Add(&b, "%s[%u,%u,%u]", i ? "," : "", visible ? d->hand[s][i] : 0, d->played[s] >> i & 1, d->origin[s][i]);
        }
        Add(&b, "]");
    }
    Add(&b, "],\"hash\":%u}", (unsigned)TtDuel_Hash(d));
    return Done(env, &b);
}

// The computer's play for the side to move (it sees only what that side may: TtAi_View): [slot, cell]
JNIEXPORT jintArray JNICALL FN(ai)(JNIEnv *env, jclass c, jlong h, jint level, jint salt)
{
    const struct TtDuel *d = (const struct TtDuel *)(intptr_t)h;
    static struct TtDuel view;
    unsigned slot = 0, cell = 0;
    jint out[2];
    jintArray a;
    if (d == NULL || d->over)
        return NULL;
    TtAi_View(d, d->toMove, &view);
    TtAi_Choose(&view, (unsigned)level, (uint32_t)salt, &slot, &cell);
    out[0] = (jint)slot;
    out[1] = (jint)cell;
    a = (*env)->NewIntArray(env, 2);
    (*env)->SetIntArrayRegion(env, a, 0, 2, out);
    return a;
}

// ------------------------------------------------------------------------------------------------ the puzzle

// The result for seat 0 with best play from here (2 a win, 1 a draw, 0 a loss), seat 0 maximizing, seat 1 minimizing
static int Best(const struct TtDuel *d, unsigned *bestSlot, unsigned *bestCell)
{
    unsigned slot, cell, can = TtDuel_Playable(d);
    int best = d->toMove == 0 ? -1 : 3;
    if (d->over)
        return d->winner == 0 ? 2 : d->winner == 2 ? 1 : 0;
    for (slot = 0; slot < TT_HAND; slot++)
        for (cell = 0; cell < TT_CELLS; cell++)
        {
            struct TtDuel next;
            int r;
            if (!(can >> slot & 1) || !TtDuel_CanPlay(d, slot, cell))
                continue;
            next = *d;
            next.noEvents = 1;
            TtDuel_Play(&next, slot, cell);
            r = Best(&next, NULL, NULL);
            if (d->toMove == 0 ? r > best : r < best)
            {
                best = r;
                if (bestSlot)
                    *bestSlot = slot, *bestCell = cell;
            }
        }
    return best;
}

static uint32_t Mix(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// DAY's puzzle: {"rules","seed","hand0","hand1","first":0,"moves":[[slot,cell]...], "wins":[[slot,cell]...]}: replay
// the moves from the start, then seat 0 (the player) is to move and only the plays in "wins" win against best defense
JNIEXPORT jstring JNICALL FN(puzzle)(JNIEnv *env, jclass c, jint day)
{
    static const uint32_t sRuleSets[] = {
        TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_SAME),
        TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_PLUS),
        TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_PLUS),
        TT_RULE_BIT(TT_RULE_OPEN),
        TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_SAME_WALL),
        TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_REVERSE),
        TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_FALLEN_ACE) | TT_RULE_BIT(TT_RULE_PLUS),
    };
    struct Buf b = {0};
    unsigned k, i, region = (unsigned)day % 8;
    uint32_t rules = sRuleSets[(unsigned)day % (sizeof(sRuleSets) / sizeof(sRuleSets[0]))];
    for (k = 0; k < 4000; k++)
    {
        struct TtDuel d, start;
        uint16_t h0[TT_HAND], h1[TT_HAND];
        unsigned moves[6][2], slot, cell, wins = 0, plays = 0, can;
        uint32_t seed = Mix((uint32_t)day * 7919u + k);
        Tt_RegionHand(region, seed, 3, (unsigned)day, h0);
        Tt_RegionHand((region + 3) % 8, Mix(seed), 3, (unsigned)day, h1);
        TtDuel_Start(&d, rules, TT_TRADE_ONE, seed, h0, h1, 0);
        start = d;
        // six plays by the computer at its best (both sides), then the player's turn with three squares left
        for (i = 0; i < 6 && !d.over; i++)
        {
            TtAi_Choose(&d, i % 2 ? 3 : 2, seed + i, &slot, &cell);
            if (!TtDuel_Play(&d, slot, cell))
                break;
            moves[i][0] = slot, moves[i][1] = cell;
        }
        if (i < 6 || d.over || d.toMove != 0)
            continue;
        d.noEvents = 1;
        can = TtDuel_Playable(&d);
        b.len = 0;
        if (b.p)
            b.p[0] = '\0';
        Add(&b, "{\"rules\":%u,\"seed\":%u,\"region\":%u,\"hand0\":[", (unsigned)rules, (unsigned)seed, region);
        for (i = 0; i < TT_HAND; i++)
            Add(&b, "%s%u", i ? "," : "", start.hand[0][i]);
        Add(&b, "],\"hand1\":[");
        for (i = 0; i < TT_HAND; i++)
            Add(&b, "%s%u", i ? "," : "", start.hand[1][i]);
        Add(&b, "],\"first\":0,\"moves\":[");
        for (i = 0; i < 6; i++)
            Add(&b, "%s[%u,%u]", i ? "," : "", moves[i][0], moves[i][1]);
        Add(&b, "],\"wins\":[");
        for (slot = 0; slot < TT_HAND; slot++)
            for (cell = 0; cell < TT_CELLS; cell++)
            {
                struct TtDuel next;
                if (!(can >> slot & 1) || !TtDuel_CanPlay(&d, slot, cell))
                    continue;
                plays++;
                next = d;
                TtDuel_Play(&next, slot, cell);
                if (Best(&next, NULL, NULL) == 2)
                    Add(&b, "%s[%u,%u]", wins++ ? "," : "", slot, cell);
            }
        Add(&b, "],\"plays\":%u}", plays);
        // a puzzle: a way to win, but most plays don't
        if (wins >= 1 && wins <= 2 && plays >= 5)
            return Done(env, &b);
    }
    free(b.p);
    return NULL;
}

// The defense's best reply in a puzzle (the computer never gives the win away): [slot, cell]
JNIEXPORT jintArray JNICALL FN(best)(JNIEnv *env, jclass c, jlong h)
{
    struct TtDuel d;
    unsigned slot = 0, cell = 0;
    jint out[2];
    jintArray a;
    if (!h)
        return NULL;
    d = *(const struct TtDuel *)(intptr_t)h;
    d.noEvents = 1;
    if (d.over)
        return NULL;
    Best(&d, &slot, &cell);
    out[0] = (jint)slot;
    out[1] = (jint)cell;
    a = (*env)->NewIntArray(env, 2);
    (*env)->SetIntArrayRegion(env, a, 0, 2, out);
    return a;
}
