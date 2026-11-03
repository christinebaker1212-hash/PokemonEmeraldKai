// Moemon Companion: YU-GI-OH! on the phone with the game's own engine (EDOPro's ocgcore with every card's script, and
// the game's bridge src/ygo/ygo_edo.c: questions and answers, the state as objects, presentation events; the AI
// ygo_ai.c; the catalog, the Forbidden & Limited lists, deck checks, power ratings and the GBA games' duelists), for
// the app's Java (Ygo.java). The engine's pack (graphics/ygo/edo.pak) is one of the app's files, handed over first.
// A duel lives here (a handle each); Java asks for its state, its question and its events as JSON and answers with
// picks: the same answers travel to friends (a duel is its seed and its answers).
#include <jni.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ygo/collection.h"
#include "ygo/duel.h"
#include "ygo_edo.h"
#include "ygo_internal.h"

#define FN(name) Java_com_moemon_companion_Ygo_##name

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

// a JSON string (quotes and backslashes escaped, line breaks kept as \n, other control characters dropped; the
// catalog's words are UTF-8 and pass through)
static void AddStr(struct Buf *b, const char *s)
{
    Add(b, "\"");
    for (; s && *s; s++)
    {
        if (*s == '"' || *s == '\\')
            Add(b, "\\%c", *s);
        else if (*s == '\n')
            Add(b, "\\n");
        else if ((unsigned char)*s >= 32)
            Add(b, "%c", *s);
    }
    Add(b, "\"");
}

// (decoded by Java's own UTF-8: NewStringUTF wants Java's modified UTF-8, and EDOPro's card texts aren't always that)
static jstring Utf8(JNIEnv *env, const char *text, size_t len)
{
    static jclass sString;
    static jmethodID sMake;
    jbyteArray bytes;
    jstring s, charset;
    if (!sString)
    {
        jclass c = (*env)->FindClass(env, "java/lang/String");
        sString = (jclass)(*env)->NewGlobalRef(env, c);
        sMake = (*env)->GetMethodID(env, sString, "<init>", "([BLjava/lang/String;)V");
    }
    bytes = (*env)->NewByteArray(env, (jsize)len);
    (*env)->SetByteArrayRegion(env, bytes, 0, (jsize)len, (const jbyte *)text);
    charset = (*env)->NewStringUTF(env, "UTF-8");
    s = (jstring)(*env)->NewObject(env, sString, sMake, bytes, charset);
    (*env)->DeleteLocalRef(env, bytes);
    (*env)->DeleteLocalRef(env, charset);
    return s;
}

static jstring Done(JNIEnv *env, struct Buf *b)
{
    jstring s = b->p ? Utf8(env, b->p, b->len) : NULL;
    free(b->p);
    return s;
}

static unsigned Ids(JNIEnv *env, jintArray a, uint16_t *out, unsigned max)
{
    jint v[YGO_DECK_MAX + YGO_CARD_COUNT];
    unsigned i, n = a ? (unsigned)(*env)->GetArrayLength(env, a) : 0;
    memset(out, 0, sizeof(uint16_t) * max);
    if (n > max)
        n = max;
    if (n)
        (*env)->GetIntArrayRegion(env, a, 0, (jsize)n, v);
    for (i = 0; i < n; i++)
        out[i] = (uint16_t)v[i];
    return n;
}

// ------------------------------------------------------------------------------------------------- the pack

static void *sPak;

// The engine's pack (EDOPro's cards, scripts, strings and lists): once, before anything else
JNIEXPORT jboolean JNICALL FN(setPak)(JNIEnv *env, jclass cls, jbyteArray pak)
{
    jsize n = pak ? (*env)->GetArrayLength(env, pak) : 0;
    if (sPak || n < 12)
        return sPak != NULL;
    sPak = malloc((size_t)n);
    if (!sPak)
        return 0;
    (*env)->GetByteArrayRegion(env, pak, 0, n, (jbyte *)sPak);
    YgoEdo_SetPak(sPak, (size_t)n);
    return YgoEdo_DataInit() ? 1 : 0;
}

// ------------------------------------------------------------------------------------------------ the tables

// Every card of the catalog: {id, n, t, atk, def, f, a, r, lv, p, ab, lim, ra, ok (playable), s (set bits as text)}
JNIEXPORT jstring JNICALL FN(catalog)(JNIEnv *env, jclass cls)
{
    struct Buf b = {0};
    unsigned id;
    Add(&b, "[");
    for (id = 1; id <= YGO_CARD_COUNT; id++)
    {
        const struct YgoCard *c = Ygo_Card(id);
        if (!c)
            continue;
        Add(&b, "%s{\"id\":%u,\"n\":", id > 1 ? "," : "", id);
        AddStr(&b, c->name);
        Add(&b, ",\"t\":");
        AddStr(&b, c->text);
        Add(&b, ",\"atk\":%d,\"def\":%d,\"f\":%u,\"a\":%u,\"r\":%u,\"lv\":%u,\"p\":%u,\"ab\":%u,\"lim\":%u,\"ra\":%u,\"ok\":%d,"
                "\"s\":\"%llx\"}",
            c->atk == YGO_STAT_UNKNOWN ? -1 : c->atk, c->def == YGO_STAT_UNKNOWN ? -1 : c->def, c->frame, c->attribute,
            c->race, c->level, c->property, c->abilities, Ygo_CopyLimit(id), c->rarity, Ygo_CardReady(id),
            (unsigned long long)c->sets);
    }
    Add(&b, "]");
    return Done(env, &b);
}

// A card outside the catalog a duel brought in (a token...): {n, t, atk, def, f, a, r, lv}, null if none
JNIEXPORT jstring JNICALL FN(extraCard)(JNIEnv *env, jclass cls, jint id)
{
    const struct YgoCard *c = id > YGO_CARD_COUNT ? Ygo_Card((unsigned)id) : NULL;
    struct Buf b = {0};
    if (!c)
        return NULL;
    Add(&b, "{\"id\":%d,\"n\":", id);
    AddStr(&b, c->name);
    Add(&b, ",\"t\":");
    AddStr(&b, c->text);
    Add(&b, ",\"atk\":%d,\"def\":%d,\"f\":%u,\"a\":%u,\"r\":%u,\"lv\":%u}", c->atk == YGO_STAT_UNKNOWN ? -1 : c->atk,
        c->def == YGO_STAT_UNKNOWN ? -1 : c->def, c->frame, c->attribute, c->race, c->level);
    return Done(env, &b);
}

// The sets: [{code, name, starter, ok (its boosters can be had), cards: [[id, rarity, copies]...]}]
JNIEXPORT jstring JNICALL FN(sets)(JNIEnv *env, jclass cls)
{
    struct Buf b = {0};
    unsigned s, i;
    Add(&b, "[");
    for (s = 0; s < YGO_SET_COUNT; s++)
    {
        const struct YgoSet *set = &gYgoSets[s];
        Add(&b, "%s{\"code\":", s ? "," : "");
        AddStr(&b, set->code);
        Add(&b, ",\"name\":");
        AddStr(&b, set->name);
        Add(&b, ",\"short\":");
        AddStr(&b, Ygo_SetShortName(s));
        Add(&b, ",\"starter\":%u,\"ok\":%d,\"cards\":[", set->starter, Ygo_SetAvailable(s));
        for (i = 0; i < set->count; i++)
            Add(&b, "%s[%u,%u,%u]", i ? "," : "", set->cards[i].card, set->cards[i].rarity, set->cards[i].copies);
        Add(&b, "]}");
    }
    Add(&b, "]");
    return Done(env, &b);
}

// Why a deck can't be played (null: it can); OWNED (count by card id) for a collection's cards, or null
JNIEXPORT jstring JNICALL FN(validate)(JNIEnv *env, jclass cls, jintArray main, jintArray extra, jintArray owned)
{
    static uint16_t m[YGO_DECK_MAX], x[YGO_FUSION_MAX], have[YGO_CARD_COUNT + 1];
    char reason[128];
    Ids(env, main, m, YGO_DECK_MAX);
    Ids(env, extra, x, YGO_FUSION_MAX);
    if (owned)
        Ids(env, owned, have, YGO_CARD_COUNT + 1);
    if (Ygo_DeckValidate(m, x, owned ? have : NULL, reason, sizeof(reason)))
        return NULL;
    return Utf8(env, reason, strlen(reason));
}

// The Forbidden & Limited list decks follow (Ygo_UseBanlist: "" the game's own) and its name
JNIEXPORT jstring JNICALL FN(useBanlist)(JNIEnv *env, jclass cls, jstring name)
{
    const char *n = name ? (*env)->GetStringUTFChars(env, name, NULL) : NULL;
    unsigned list = Ygo_UseBanlist(n ? n : "");
    if (n)
        (*env)->ReleaseStringUTFChars(env, name, n);
    return Utf8(env, Ygo_BanlistName(list), strlen(Ygo_BanlistName(list)));
}

JNIEXPORT jint JNICALL FN(deckRating)(JNIEnv *env, jclass cls, jintArray main)
{
    uint16_t m[YGO_DECK_MAX];
    Ids(env, main, m, YGO_DECK_MAX);
    return Ygo_DeckRating(m);
}

JNIEXPORT jint JNICALL FN(levelRating)(JNIEnv *env, jclass cls, jint level)
{
    return gYgoLevelRating[level < 0 ? 0 : level > 3 ? 3 : level];
}

JNIEXPORT jint JNICALL FN(chance)(JNIEnv *env, jclass cls, jint mine, jint theirs)
{
    return (jint)Ygo_Chance(mine, theirs);
}

static void AddDeck(struct Buf *b, const uint16_t *main, const uint16_t *extra)
{
    unsigned i, n = 0;
    Add(b, "\"main\":[");
    for (i = 0; i < YGO_DECK_MAX && main[i]; i++)
        Add(b, "%s%u", n++ ? "," : "", main[i]);
    Add(b, "],\"extra\":[");
    for (i = 0, n = 0; i < YGO_FUSION_MAX && extra[i]; i++)
        Add(b, "%s%u", n++ ? "," : "", extra[i]);
    Add(b, "]");
}

// A duelist for a FREE DUEL, as the game's CARD SCALING picks one (ygo_field.c FitOnce, the Game Boy decks): the
// closest to TARGET among the decks up to the quarter the player's badges reach (QUARTER 0-3), PICK among those nearly
// as close: {"name", "game", "rating", "main", "extra"}
JNIEXPORT jstring JNICALL FN(opponent)(JNIEnv *env, jclass cls, jint target, jint quarter, jint pick)
{
    static uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    struct Buf b = {0};
    unsigned i, hi, best = ~0u, near = 0, p = (unsigned)pick, tries;
    if (gYgoNpcDeckCount < 4)
        return NULL;
    hi = gYgoNpcDeckCount * ((unsigned)(quarter < 0 ? 0 : quarter > 3 ? 3 : quarter) + 1) / 4;
    for (i = 0; i < hi; i++)
    {
        unsigned off = (unsigned)abs((int)gYgoNpcDecks[i].rating - target);
        best = off < best ? off : best;
    }
    for (i = 0; i < hi; i++)
        near += (unsigned)abs((int)gYgoNpcDecks[i].rating - target) <= best + 25;
    p %= near ? near : 1;
    for (i = 0; i < hi; i++)
        if ((unsigned)abs((int)gYgoNpcDecks[i].rating - target) <= best + 25 && !p--)
            break;
    // (a deck that can't be made of cards that can be played: the next one up)
    for (tries = 0; tries < gYgoNpcDeckCount && !Ygo_NpcDeckCards(i % gYgoNpcDeckCount, main, extra); tries++)
        i++;
    if (tries == gYgoNpcDeckCount)
        return NULL;
    i %= gYgoNpcDeckCount;
    Add(&b, "{\"index\":%u,\"name\":", i);
    AddStr(&b, gYgoNpcDecks[i].name);
    Add(&b, ",\"game\":");
    AddStr(&b, gYgoNpcDecks[i].gameName);
    Add(&b, ",\"rating\":%d,", Ygo_DeckRating(main));
    AddDeck(&b, main, extra);
    Add(&b, "}");
    return Done(env, &b);
}

// A set's deck as a list (a Starter / Structure Deck), for a borrowed deck: {"main", "extra"}
JNIEXPORT jstring JNICALL FN(setDeck)(JNIEnv *env, jclass cls, jint set)
{
    static uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
    struct Buf b = {0};
    unsigned i, k, m = 0, x = 0;
    if (set < 0 || set >= YGO_SET_COUNT || !gYgoSets[set].starter)
        return NULL;
    memset(main, 0, sizeof(main));
    memset(extra, 0, sizeof(extra));
    for (i = 0; i < gYgoSets[set].count; i++)
        for (k = 0; k < (gYgoSets[set].cards[i].copies ? gYgoSets[set].cards[i].copies : 1u); k++)
        {
            const struct YgoCard *c = Ygo_Card(gYgoSets[set].cards[i].card);
            if (Ygo_IsExtra(c) && x < YGO_FUSION_MAX)
                extra[x++] = c->id;
            else if (c && !Ygo_IsExtra(c) && m < YGO_DECK_MAX)
                main[m++] = c->id;
        }
    Add(&b, "{");
    AddDeck(&b, main, extra);
    Add(&b, "}");
    return Done(env, &b);
}

// ------------------------------------------------------------------------------------------------------ duels

JNIEXPORT jlong JNICALL FN(start)(JNIEnv *env, jclass cls, jintArray main0, jintArray extra0, jintArray main1,
                                  jintArray extra1, jint seed, jint first)
{
    uint16_t m0[YGO_DECK_MAX], x0[YGO_FUSION_MAX], m1[YGO_DECK_MAX], x1[YGO_FUSION_MAX];
    struct YgoDuel *d = calloc(1, sizeof(*d));
    if (!d || !sPak)
    {
        free(d);
        return 0;
    }
    Ids(env, main0, m0, YGO_DECK_MAX);
    Ids(env, extra0, x0, YGO_FUSION_MAX);
    Ids(env, main1, m1, YGO_DECK_MAX);
    Ids(env, extra1, x1, YGO_FUSION_MAX);
    if (!YgoDuel_Init(d, m0, x0, m1, x1, (uint32_t)seed, (unsigned)first & 1))
    {
        YgoDuel_Free(d);
        free(d);
        return 0;
    }
    return (jlong)(intptr_t)d;
}

JNIEXPORT void JNICALL FN(end)(JNIEnv *env, jclass cls, jlong h)
{
    struct YgoDuel *d = (struct YgoDuel *)(intptr_t)h;
    if (d)
    {
        YgoDuel_Free(d);
        free(d);
    }
}

// Can SEAT see the card of OBJ (in the hand, a deck, face down...)?
static int Sees(const struct YgoState *s, unsigned obj, unsigned seat)
{
    const struct YgoObj *o = &s->objs[obj];
    switch (o->location)
    {
    case YGO_LOC_DECK:
        return 0;
    case YGO_LOC_HAND:
    case YGO_LOC_EXTRA:
        return o->controller == seat || o->faceUp;
    case YGO_LOC_MZONE:
    case YGO_LOC_SZONE:
        return o->faceUp || o->controller == seat;
    default:
        return 1;
    }
}

static void AddObjs(struct Buf *b, const struct YgoState *s, const uint8_t *list, unsigned n, unsigned seat)
{
    unsigned i;
    Add(b, "[");
    for (i = 0; i < n; i++)
        Add(b, "%s%u", i ? "," : "", list[i]);
    Add(b, "]");
    (void)s;
    (void)seat;
}

// The duel as SEAT may see it: {turn, current, first, phase, over, winner, why, attacker, target, players[2] {lp,
// deck, hand[], grave[], banished[], extra[] (SEAT's own; the other side's a count), monsters[5], spells[6]} (objects;
// zones -1 empty), objs[] {o, c (0: hidden), up, atk (attack position), at, df, lv, cn, ng, eq, loc, ctl, seq},
// chain [[obj, player, card, negated]]}
JNIEXPORT jstring JNICALL FN(state)(JNIEnv *env, jclass cls, jlong h, jint seat)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    const struct YgoState *s;
    struct Buf b = {0};
    unsigned p, i;
    if (!d)
        return NULL;
    s = &d->s;
    Add(&b, "{\"turn\":%u,\"current\":%u,\"first\":%u,\"phase\":%u,\"over\":%u,\"winner\":%u,\"why\":%u,\"attacker\":%d,"
            "\"target\":%d,\"asks\":%d,\"seq\":%u,\"players\":[",
        s->turn, s->current, s->first, s->phase, s->over, s->winner, s->winReason, (int)s->attacker - 1,
        (int)s->attackTarget - 1, d->query.type == YGO_Q_NONE ? -1 : (int)d->query.player, (unsigned)s->eventSeq);
    for (p = 0; p < 2; p++)
    {
        const struct YgoPlayer *pl = &s->players[p];
        Add(&b, "%s{\"lp\":%d,\"deck\":%u,\"hand\":", p ? "," : "", (int)pl->lp, pl->deckCount);
        AddObjs(&b, s, pl->hand, pl->handCount, (unsigned)seat);
        Add(&b, ",\"grave\":");
        AddObjs(&b, s, pl->grave, pl->graveCount, (unsigned)seat);
        Add(&b, ",\"banished\":");
        AddObjs(&b, s, pl->banished, pl->banishedCount, (unsigned)seat);
        Add(&b, ",\"extraCount\":%u,\"extra\":", pl->extraCount);
        AddObjs(&b, s, pl->extra, p == (unsigned)seat ? pl->extraCount : 0, (unsigned)seat);
        Add(&b, ",\"monsters\":[");
        for (i = 0; i < YGO_ZONES; i++)
            Add(&b, "%s%d", i ? "," : "", (int)pl->monsters[i] - 1);
        Add(&b, "],\"spells\":[");
        for (i = 0; i < YGO_ZONES + 1; i++)
            Add(&b, "%s%d", i ? "," : "", (int)pl->spells[i] - 1);
        Add(&b, "],\"summoned\":%u}", pl->normalSummoned);
    }
    Add(&b, "],\"objs\":[");
    for (i = 0; i < s->objCount; i++)
    {
        const struct YgoObj *o = &s->objs[i];
        int see = Sees(s, i, (unsigned)seat);
        Add(&b, "%s{\"c\":%u,\"up\":%u,\"ap\":%u,\"at\":%d,\"df\":%d,\"lv\":%u,\"cn\":%u,\"ng\":%u,\"eq\":%d,\"loc\":%u,"
                "\"ctl\":%u,\"own\":%u,\"seq\":%u,\"new\":%u}",
            i ? "," : "", see ? o->card : 0, o->faceUp, o->attackPos, see ? YgoDuel_Atk(s, i) : 0,
            see ? YgoDuel_Def(s, i) : 0, see ? YgoDuel_Level(s, i) : 0, o->counters, o->negated, (int)o->equipTo - 1,
            o->location, o->controller, o->owner, o->seq, o->enteredTurn == s->turn);
    }
    Add(&b, "],\"chain\":[");
    for (i = 0; i < s->chainCount; i++)
        Add(&b, "%s[%u,%u,%u,%u]", i ? "," : "", s->chain[i].obj, s->chain[i].player, s->chain[i].card,
            s->chain[i].negated);
    Add(&b, "]}");
    return Done(env, &b);
}

// The question now (for its player): {type, player, prompt, min, max, card, sum, mode, cancel, quiet, text, items
// [{k, o, v, f, c, p, t}]}; names (a card to declare) separately
JNIEXPORT jstring JNICALL FN(query)(JNIEnv *env, jclass cls, jlong h)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    const struct YgoQuery *q;
    struct Buf b = {0};
    char buf[256];
    unsigned i;
    if (!d)
        return NULL;
    q = &d->query;
    Add(&b, "{\"type\":%u,\"player\":%u,\"prompt\":%u,\"min\":%u,\"max\":%u,\"card\":%u,\"sum\":%u,\"mode\":%u,"
            "\"cancel\":%u,\"quiet\":%d,\"text\":",
        q->type, q->player, q->prompt, q->min, q->max, q->card, (unsigned)q->sum, q->sumMode, q->cancelable,
        YgoDuel_QuietChance(d));
    AddStr(&b, YgoDuel_QueryText(d, buf, sizeof(buf)));
    Add(&b, ",\"items\":[");
    for (i = 0; i < q->count; i++)
    {
        const struct YgoChoice *c = &q->items[i];
        Add(&b, "%s{\"k\":%u,\"o\":%d,\"v\":%u,\"f\":%u,\"c\":%u,\"p\":%u,\"t\":", i ? "," : "", c->kind,
            c->obj == 0xFF ? -1 : (int)c->obj, c->value, c->flags, c->card, c->param);
        AddStr(&b, YgoDuel_ItemText(d, i, buf, sizeof(buf)));
        Add(&b, "}");
    }
    Add(&b, "],\"names\":%u}", d->nameCount);
    return Done(env, &b);
}

// YGO_Q_NAME: the cards that can be declared
JNIEXPORT jintArray JNICALL FN(names)(JNIEnv *env, jclass cls, jlong h)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    jint v[YGO_NAMES_MAX];
    unsigned i;
    jintArray a;
    if (!d)
        return NULL;
    for (i = 0; i < d->nameCount; i++)
        v[i] = d->names[i];
    a = (*env)->NewIntArray(env, (jsize)d->nameCount);
    (*env)->SetIntArrayRegion(env, a, 0, (jsize)d->nameCount, v);
    return a;
}

static void ToAnswer(JNIEnv *env, jintArray picks, jint zone, struct YgoAnswer *a)
{
    jint v[YGO_PICKS_MAX];
    unsigned i, n = picks ? (unsigned)(*env)->GetArrayLength(env, picks) : 0;
    memset(a, 0, sizeof(*a));
    if (n > YGO_PICKS_MAX)
        n = YGO_PICKS_MAX;
    if (n)
        (*env)->GetIntArrayRegion(env, picks, 0, (jsize)n, v);
    for (i = 0; i < n; i++)
        a->picks[i] = (uint8_t)v[i];
    a->count = (uint8_t)n;
    a->zone = (uint8_t)zone;
}

// Answers the question: {"ok", "error"}
JNIEXPORT jstring JNICALL FN(answer)(JNIEnv *env, jclass cls, jlong h, jintArray picks, jint zone)
{
    struct YgoDuel *d = (struct YgoDuel *)(intptr_t)h;
    struct YgoAnswer a;
    struct Buf b = {0};
    int ok;
    if (!d)
        return NULL;
    ToAnswer(env, picks, zone, &a);
    d->error[0] = 0;
    ok = YgoDuel_Answer(d, &a);
    Add(&b, "{\"ok\":%d,\"error\":", ok);
    AddStr(&b, ok ? "" : d->error[0] ? d->error : "That can't be done.");
    Add(&b, "}");
    return Done(env, &b);
}

JNIEXPORT jboolean JNICALL FN(check)(JNIEnv *env, jclass cls, jlong h, jintArray picks, jint zone)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    struct YgoAnswer a;
    if (!d)
        return 0;
    ToAnswer(env, picks, zone, &a);
    return YgoDuel_Check(d, &a) ? 1 : 0;
}

// The computer's answer at LEVEL (0 a beginner .. 3): {"p": [picks], "z": zone}
JNIEXPORT jstring JNICALL FN(ai)(JNIEnv *env, jclass cls, jlong h, jint level)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    struct YgoAnswer a;
    struct Buf b = {0};
    unsigned i;
    if (!d || d->query.type == YGO_Q_NONE)
        return NULL;
    YgoDuel_AiAnswerLevel(d, (unsigned)(level < 0 ? 0 : level), &a);
    Add(&b, "{\"p\":[");
    for (i = 0; i < a.count; i++)
        Add(&b, "%s%u", i ? "," : "", a.picks[i]);
    Add(&b, "],\"z\":%u}", a.zone);
    return Done(env, &b);
}

JNIEXPORT jboolean JNICALL FN(concede)(JNIEnv *env, jclass cls, jlong h, jint player)
{
    struct YgoDuel *d = (struct YgoDuel *)(intptr_t)h;
    return d && YgoDuel_Concede(d, (unsigned)player & 1) ? 1 : 0;
}

JNIEXPORT jint JNICALL FN(hash)(JNIEnv *env, jclass cls, jlong h)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    return d ? (jint)YgoDuel_Hash(d) : 0;
}

JNIEXPORT jint JNICALL FN(answers)(JNIEnv *env, jclass cls, jlong h)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    return d ? (jint)YgoDuel_AnswerCount(d) : 0;
}

// The events after FROM as SEAT may see them: [[seq, type, player, obj, target, card, flags, value]...] (a card SEAT
// can't see as 0: a draw, a set, a look that's the other side's)
JNIEXPORT jstring JNICALL FN(events)(JNIEnv *env, jclass cls, jlong h, jint from, jint seat)
{
    const struct YgoDuel *d = (const struct YgoDuel *)(intptr_t)h;
    struct Buf b = {0};
    uint32_t seq, n = 0;
    if (!d)
        return NULL;
    Add(&b, "[");
    for (seq = (uint32_t)from + 1; seq <= d->s.eventSeq; seq++)
    {
        const struct YgoEvent *e = YgoDuel_GetEvent(&d->s, seq);
        unsigned card;
        int hidden;
        if (!e)
            continue;
        hidden = e->player != (unsigned)seat &&
                 (e->type == YGO_EV_DRAW || e->type == YGO_EV_SET || e->type == YGO_EV_LOOK ||
                  (e->type == YGO_EV_TO_HAND && e->obj < d->s.objCount && !Sees(&d->s, e->obj, (unsigned)seat)));
        card = hidden ? 0 : e->card;
        Add(&b, "%s[%u,%u,%u,%d,%d,%u,%u,%d]", n++ ? "," : "", (unsigned)seq, e->type, e->player,
            e->obj == 0xFF ? -1 : (int)e->obj, e->target == 0xFF ? -1 : (int)e->target, card, e->flags, (int)e->value);
    }
    Add(&b, "]");
    return Done(env, &b);
}
