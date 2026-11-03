// Moemon Companion: the POKéMON card game on the phone with the game's own engine (src/tcg: the rules, every card's
// effects and powers, the AI, the catalog, the deck ratings CARD SCALING uses) and the duel board's choices
// (src/tcg/tcg_flow.c: what a card, an attack or a power asks for), for the app's Java (Tcg.java). A duel lives here (a
// handle per game); Java asks for its state, what can be done, and its events as JSON, and makes moves through a flow
// (begin a move, answer its questions, apply it). Moves travel to friends as small JSON actions (actionJson).
#include <jni.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tcg/duel.h"
#include "tcg/flow.h"
#include "tcg/link.h"
#include "tcg/scale.h"

#define FN(name) Java_com_moemon_companion_Tcg_##name

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

static int Deck(JNIEnv *env, jintArray a, uint16_t out[TCG_DECK_SIZE])
{
    jint v[TCG_DECK_SIZE] = {0};
    unsigned i;
    if (a == NULL || (*env)->GetArrayLength(env, a) < TCG_DECK_SIZE)
        return 0;
    (*env)->GetIntArrayRegion(env, a, 0, TCG_DECK_SIZE, v);
    for (i = 0; i < TCG_DECK_SIZE; i++)
        out[i] = (uint16_t)v[i];
    return 1;
}

// ------------------------------------------------------------------------------------------------- the tables

JNIEXPORT jstring JNICALL FN(catalog)(JNIEnv *env, jclass cls)
{
    struct Buf b = {0};
    unsigned id, k, t;
    Add(&b, "[");
    for (id = 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        Add(&b, "%s{\"id\":%u,\"n\":", id > 1 ? "," : "", id);
        AddStr(&b, c->name);
        Add(&b, ",\"k\":%u,\"t\":%u,\"ra\":%u,\"set\":%u,\"set2\":%u,\"h\":%u,\"d\":%u", c->kind, c->type, c->rarity, c->set,
            c->set2, Tcg_IsHolo(id) ? 1 : 0, c->dark);
        if (c->kind == TCG_POKEMON)
        {
            Add(&b, ",\"st\":%u,\"hp\":%u,\"lv\":%u,\"rc\":%u,\"w\":%u,\"r\":%u,\"sp\":%u,\"ef\":", c->stage, c->hp, c->level,
                c->retreat, c->weakness, c->resistance, c->species);
            AddStr(&b, c->evolvesFrom);
            Add(&b, ",\"a\":[");
            for (k = 0; k < 2 && c->attacks[k].name && c->attacks[k].name[0]; k++)
            {
                const struct TcgAttack *a = &c->attacks[k];
                Add(&b, "%s{\"n\":", k ? "," : "");
                AddStr(&b, a->name);
                Add(&b, ",\"c\":[");
                for (t = 0; t < TCG_TYPES; t++)
                    Add(&b, "%s%u", t ? "," : "", a->cost[t]);
                Add(&b, "],\"dmg\":%u,\"cat\":%u,\"txt\":", a->damage, a->category);
                AddStr(&b, a->description);
                Add(&b, "}");
            }
            Add(&b, "]");
        }
        Add(&b, ",\"txt\":");
        AddStr(&b, c->description);
        Add(&b, "}");
    }
    Add(&b, "]");
    return Done(env, &b);
}

// Every booster's cards (Tcg_InPack), as [[ids of pack 0], [pack 1]...]
JNIEXPORT jstring JNICALL FN(packs)(JNIEnv *env, jclass cls)
{
    struct Buf b = {0};
    unsigned p, id, n;
    Add(&b, "[");
    for (p = 0; p < TCG_PACK_TYPES; p++)
    {
        Add(&b, "%s[", p ? "," : "");
        for (id = 1, n = 0; id <= TCG_CARD_COUNT; id++)
            if (Tcg_InPack(id, p))
                Add(&b, "%s%u", n++ ? "," : "", id);
        Add(&b, "]");
    }
    Add(&b, "]");
    return Done(env, &b);
}

// Why a deck can't be played (null: it can); OWNED (count by card id) for the cards a collection holds, or null
JNIEXPORT jstring JNICALL FN(validate)(JNIEnv *env, jclass cls, jintArray deck, jintArray owned)
{
    uint16_t d[TCG_DECK_SIZE];
    static uint16_t have[TCG_CARD_COUNT + 1];
    char reason[96];
    if (!Deck(env, deck, d))
        return (*env)->NewStringUTF(env, "A deck must contain exactly 60 valid cards.");
    if (owned != NULL)
    {
        jint v[TCG_CARD_COUNT + 1] = {0};
        unsigned i, n = (unsigned)(*env)->GetArrayLength(env, owned);
        (*env)->GetIntArrayRegion(env, owned, 0, n < TCG_CARD_COUNT + 1 ? n : TCG_CARD_COUNT + 1, v);
        for (i = 0; i <= TCG_CARD_COUNT; i++)
            have[i] = (uint16_t)(v[i] < 0 ? 0 : v[i] > 65535 ? 65535 : v[i]);
    }
    if (Tcg_DeckValidate(d, owned != NULL ? have : NULL, reason, sizeof(reason)))
        return NULL;
    return (*env)->NewStringUTF(env, reason);
}

// A deck's strength as CARD SCALING measures it (a few hundred quick AI duels: call it off the main thread)
JNIEXPORT jint JNICALL FN(rateDeck)(JNIEnv *env, jclass cls, jintArray deck)
{
    uint16_t d[TCG_DECK_SIZE];
    if (!Deck(env, deck, d) || !Tcg_DeckValidate(d, NULL, NULL, 0))
        return TCG_SCALE_MEAN;
    return TcgScale_RateDeck(d);
}

JNIEXPORT jint JNICALL FN(levelRating)(JNIEnv *env, jclass cls, jint level)
{
    return TcgScale_LevelRating(level < 0 ? 0 : (unsigned)level);
}

JNIEXPORT jint JNICALL FN(expected)(JNIEnv *env, jclass cls, jint rating, jint other)
{
    return (jint)TcgScale_Expected(rating, other);
}

// A card player for a FREE DUEL, as the game's CARD SCALING picks one (tcg_field.c Tcg_FitOpponent, for an ordinary
// card player: any of the game's decks): a touch weaker than the player (MINE: their deck's rating and skill), how
// carefully they play by the badges, capped by the badges: {"deck":[60],"name","rating","level"}
JNIEXPORT jstring JNICALL FN(opponent)(JNIEnv *env, jclass cls, jint mine, jint badges, jint pick)
{
    static const int16_t sBadgeCap[9] = {1460, 1490, 1530, 1570, 1610, 1650, 1690, 1730, 2000};
    struct Buf b = {0};
    unsigned i, best = 0xFFFF, near = 0, chosen = 1, level, p = (unsigned)pick;
    int target, cap;
    if (badges < 0)
        badges = 0;
    if (badges > 8)
        badges = 8;
    cap = sBadgeCap[badges];
    level = badges >= 4 ? 2 : 1;
    if ((p >> 17) % 4 == 0)
        level--;
again:
    target = mine + TcgScale_LevelRating(2) - 35 - TcgScale_LevelRating(level);
    if (target > cap)
    {
        // the decks they may bring this early fall well short: they play more carefully instead (SKILLED at most)
        if (target - cap > 50 && level < 2)
        {
            level++;
            goto again;
        }
        target = cap;
    }
    // (deck 0 is no one's)
    for (i = 1; i < gTcgDeckDefinitionCount; i++)
    {
        unsigned d = (unsigned)abs(gTcgDeckRating[i] - target);
        if (d < best)
            best = d;
    }
    for (i = 1; i < gTcgDeckDefinitionCount; i++)
        near += (unsigned)abs(gTcgDeckRating[i] - target) <= best + 30;
    p %= near ? near : 1;
    for (i = 1; i < gTcgDeckDefinitionCount; i++)
        if ((unsigned)abs(gTcgDeckRating[i] - target) <= best + 30 && !p--)
            chosen = i;
    Add(&b, "{\"index\":%u,\"name\":", chosen);
    AddStr(&b, gTcgDeckDefinitions[chosen].name);
    Add(&b, ",\"rating\":%d,\"level\":%u,\"deck\":[", gTcgDeckRating[chosen], level);
    for (i = 0; i < TCG_DECK_SIZE; i++)
        Add(&b, "%s%u", i ? "," : "", gTcgDeckDefinitions[chosen].cards[i]);
    Add(&b, "]}");
    return Done(env, &b);
}

// A Game Boy deck by its name (the starters: CharmanderAndFriends...), for a borrowed deck: [60] or null
JNIEXPORT jintArray JNICALL FN(namedDeck)(JNIEnv *env, jclass cls, jstring name)
{
    const char *n = (*env)->GetStringUTFChars(env, name, NULL);
    unsigned i, k;
    jintArray out = NULL;
    for (i = 0; n && i < gTcgDeckDefinitionCount; i++)
        if (!strcmp(gTcgDeckDefinitions[i].name, n))
        {
            jint v[TCG_DECK_SIZE];
            for (k = 0; k < TCG_DECK_SIZE; k++)
                v[k] = gTcgDeckDefinitions[i].cards[k];
            out = (*env)->NewIntArray(env, TCG_DECK_SIZE);
            (*env)->SetIntArrayRegion(env, out, 0, TCG_DECK_SIZE, v);
            break;
        }
    if (n)
        (*env)->ReleaseStringUTFChars(env, name, n);
    return out;
}

// ------------------------------------------------------------------------------------------------------ duels

JNIEXPORT jlong JNICALL FN(start)(JNIEnv *env, jclass cls, jintArray deck0, jintArray deck1, jint seed, jint prizes)
{
    struct TcgDuel *g = calloc(1, sizeof(*g));
    uint16_t a[TCG_DECK_SIZE], b[TCG_DECK_SIZE];
    if (g == NULL)
        return 0;
    if (!Deck(env, deck0, a) || !Deck(env, deck1, b) || !TcgDuel_Init(g, a, b, (uint32_t)seed, (unsigned)prizes))
    {
        free(g);
        return 0;
    }
    return (jlong)(intptr_t)g;
}

JNIEXPORT void JNICALL FN(end)(JNIEnv *env, jclass cls, jlong h)
{
    free((void *)(intptr_t)h);
}

JNIEXPORT jlong JNICALL FN(copy)(JNIEnv *env, jclass cls, jlong h)
{
    struct TcgDuel *g = malloc(sizeof(*g));
    if (g != NULL && h)
        memcpy(g, (void *)(intptr_t)h, sizeof(*g));
    return (jlong)(intptr_t)g;
}

JNIEXPORT jint JNICALL FN(actor)(JNIEnv *env, jclass cls, jlong h)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    return g ? (jint)(TcgFlow_Actor(g) == TCG_NONE ? -1 : (int)TcgFlow_Actor(g)) : -1;
}

static void AddIds(struct Buf *b, const struct TcgPlayer *p, const uint8_t *pile, unsigned n, int visible)
{
    unsigned i;
    Add(b, "[");
    for (i = 0; i < n; i++)
        Add(b, "%s[%u,%u]", i ? "," : "", pile[i], visible ? p->cards[pile[i]] : 0);
    Add(b, "]");
}

// The table as SEAT may see it (the other side's hand and both decks hidden; Clairvoyance shows their hand)
JNIEXPORT jstring JNICALL FN(state)(JNIEnv *env, jclass cls, jlong h, jint seat)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    struct Buf b = {0};
    unsigned who, s, i, actor;
    if (g == NULL)
        return NULL;
    actor = TcgFlow_Actor(g);
    Add(&b, "{\"phase\":%u,\"turn\":%u,\"current\":%u,\"first\":%u,\"winner\":%d,\"prizes\":%u,\"actor\":%d,\"seq\":%u,"
            "\"switch\":%u,\"p\":[",
        g->phase, g->turn, g->current, g->first, g->winner == TCG_NONE ? -1 : (int)g->winner, g->prizeCount,
        actor == TCG_NONE ? -1 : (int)actor, (unsigned)g->eventSeq, g->switchPending);
    for (who = 0; who < 2; who++)
    {
        const struct TcgPlayer *p = &g->players[who];
        int seeHand = who == (unsigned)seat || TcgPower_InPlay(g, (unsigned)seat, TCG_POWER_CLAIRVOYANCE);
        Add(&b, "%s{\"deck\":%u,\"hand\":", who ? "," : "", p->deckSize);
        AddIds(&b, p, p->hand, p->handSize, seeHand);
        Add(&b, ",\"discard\":");
        AddIds(&b, p, p->discard, p->discardSize, 1);
        Add(&b, ",\"prize\":");
        AddIds(&b, p, p->prizes, p->prizeSize, p->prizesUp);
        Add(&b, ",\"energy\":%u,\"retreated\":%u,\"turns\":%u,\"noTrainers\":%u,\"setup\":%u,\"field\":[", p->energyPlayed,
            p->retreated, p->turnsTaken, p->noTrainers, p->setupDone);
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgFieldCard *f = &p->field[s];
            const struct TcgCard *top = TcgDuel_Field(g, who, s);
            // (while setting up, the other side's Pokemon are face down)
            int hidden = g->phase == TCG_SETUP && who != (unsigned)seat;
            if (!top)
            {
                Add(&b, "%snull", s ? "," : "");
                continue;
            }
            Add(&b, "%s{\"id\":%u,\"stack\":[", s ? "," : "", hidden ? 0 : top->id);
            for (i = 0; i < f->stackSize && !hidden; i++)
                Add(&b, "%s%u", i ? "," : "", p->cards[f->stack[i]]);
            Add(&b, "],\"att\":[");
            for (i = 0; i < f->attachedSize && !hidden; i++)
                Add(&b, "%s[%u,%u]", i ? "," : "", f->attached[i], p->cards[f->attached[i]]);
            Add(&b, "],\"dmg\":%u,\"hp\":%u,\"cond\":%u,\"poison\":%u,\"type\":%u,\"weak\":%u,\"resist\":%u,\"plus\":%u,"
                    "\"defender\":%u,\"new\":%u,\"power\":%u}",
                hidden ? 0 : f->damage, hidden ? 0 : top->kind == TCG_TRAINER ? 10 : top->hp, f->condition, f->poison,
                hidden ? 0 : TcgDuel_Type(g, who, s), TcgDuel_Weakness(g, who, s), TcgDuel_Resistance(g, who, s),
                f->plusPower, f->defender, f->enteredTurn == g->turn, hidden ? 0 : TcgPower_Working(g, who, s));
        }
        Add(&b, "]}");
    }
    Add(&b, "],\"error\":");
    AddStr(&b, g->error);
    Add(&b, ",\"log\":[");
    for (i = 0; i < g->logCount && i < TCG_LOG_LINES; i++)
    {
        Add(&b, "%s", i ? "," : "");
        AddStr(&b, g->log[(g->logCount - (g->logCount < TCG_LOG_LINES ? g->logCount : TCG_LOG_LINES) + i) % TCG_LOG_LINES]);
    }
    Add(&b, "]}");
    return Done(env, &b);
}

// The events after FROM (TcgDuel.eventSeq) as SEAT may see them (a Peek names a hidden card: only to its player):
// [[seq, type, player, slot, flags, value, card]...]
JNIEXPORT jstring JNICALL FN(events)(JNIEnv *env, jclass cls, jlong h, jint from, jint seat)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    struct Buf b = {0};
    uint32_t seq, n = 0;
    if (g == NULL)
        return NULL;
    Add(&b, "[");
    for (seq = (uint32_t)from + 1; seq <= g->eventSeq; seq++)
    {
        const struct TcgEvent *e = TcgDuel_GetEvent(g, seq);
        unsigned card;
        if (e == NULL)
            continue;
        card = e->type == TCG_EV_PEEK && e->player != (unsigned)seat ? 0 : e->card;
        Add(&b, "%s[%u,%u,%u,%u,%u,%u,%u]", n++ ? "," : "", (unsigned)seq, e->type, e->player, e->slot, e->flags, e->value,
            card);
    }
    Add(&b, "]");
    return Done(env, &b);
}

// What SEAT can do now and why not: the hand's cards, the attacks, retreat, the Pokemon Powers, END TURN / READY
JNIEXPORT jstring JNICALL FN(options)(JNIEnv *env, jclass cls, jlong h, jint seat)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    struct Buf b = {0};
    char why[96];
    unsigned me = (unsigned)seat & 1, i, s, n = 0;
    int mine;
    const struct TcgPlayer *p;
    const struct TcgCard *attacker;
    if (g == NULL)
        return NULL;
    p = &g->players[me];
    mine = TcgFlow_Actor(g) == me;
    attacker = TcgDuel_Attacker(g, me);
    Add(&b, "{\"mine\":%d,\"hand\":[", mine);
    for (i = 0; i < p->handSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->hand[i]]);
        int ok = mine && TcgFlow_HandPlayable(g, me, c, why, sizeof(why));
        if (!mine)
            snprintf(why, sizeof(why), "%s", g->phase == TCG_FINISHED ? "The duel is over." : "It isn't your turn.");
        Add(&b, "%s{\"ok\":%d,\"use\":%d,\"verb\":", i ? "," : "", ok,
            c->kind != TCG_TRAINER || TcgFlow_TrainerHasUse(g, me, c->id));
        AddStr(&b, TcgFlow_HandVerb(c));
        Add(&b, ",\"why\":");
        AddStr(&b, ok ? "" : why);
        Add(&b, "}");
    }
    Add(&b, "],\"attacks\":[");
    for (i = 0; attacker && i < 2 && attacker->attacks[i].name[0]; i++)
    {
        int ok = mine && g->phase == TCG_TURN && TcgFlow_AttackReady(g, me, i, why, sizeof(why));
        if (!mine || g->phase != TCG_TURN)
            snprintf(why, sizeof(why), "%s", !mine ? "It isn't your turn." : "You can't attack now.");
        Add(&b, "%s{\"ok\":%d,\"power\":%d,\"n\":", i ? "," : "", ok, attacker->attacks[i].category == TCG_POKEMON_POWER);
        AddStr(&b, attacker->attacks[i].name);
        Add(&b, ",\"why\":");
        AddStr(&b, ok ? "" : why);
        Add(&b, "}");
    }
    {
        int ok = mine && g->phase == TCG_TURN && TcgFlow_CanRetreat(g, me, why, sizeof(why));
        if (!mine || g->phase != TCG_TURN)
            snprintf(why, sizeof(why), "%s", !mine ? "It isn't your turn." : "You can't retreat now.");
        Add(&b, "],\"retreat\":{\"ok\":%d,\"cost\":%u,\"why\":", ok, TcgDuel_RetreatCost(g, me));
        AddStr(&b, ok ? "" : why);
    }
    Add(&b, "},\"powers\":[");
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        const struct TcgCard *c = TcgDuel_Field(g, me, s);
        const char *block;
        if (!TcgFlow_HasPower(c))
            continue;
        block = !mine ? "It isn't your turn." : g->phase != TCG_TURN ? "Not now." : TcgPower_Block(g, me, s);
        Add(&b, "%s{\"slot\":%u,\"ok\":%d,\"n\":", n++ ? "," : "", s, !block);
        AddStr(&b, c->attacks[0].name);
        Add(&b, ",\"why\":");
        AddStr(&b, block ? block : "");
        Add(&b, "}");
    }
    {
        unsigned inPlay = 0, ends = 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            inPlay += TcgDuel_Field(g, me, s) != NULL;
        Add(&b, "],\"discard\":[");
        for (s = 0, n = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, me, s);
            if (c && c->kind == TCG_TRAINER)
                Add(&b, "%s{\"slot\":%u,\"ok\":%d}", n++ ? "," : "", s, mine && g->phase == TCG_TURN && inPlay >= 2);
        }
        // END TURN without the Energy attachment that's still possible asks first (the board's EndTurn)
        for (i = 0; !p->energyPlayed && i < p->handSize; i++)
            ends |= Tcg_Card(p->cards[p->hand[i]])->kind == TCG_ENERGY;
        Add(&b, "],\"end\":%d,\"energyLeft\":%u,\"ready\":%d,\"promote\":%d,\"setup\":%d}",
            mine && g->phase == TCG_TURN, ends, mine && g->phase == TCG_SETUP && p->field[0].stackSize,
            mine && g->phase == TCG_PROMOTION, g->phase == TCG_SETUP);
    }
    return Done(env, &b);
}

// ------------------------------------------------------------------------------------------------- the moves

static void AddAction(struct Buf *b, const struct TcgAction *a)
{
    unsigned i;
    Add(b, "{\"t\":%u,\"h\":%u,\"s\":%u,\"a\":%u,\"g\":%u,\"o\":%u,\"c\":%u,\"x\":[", a->type, a->hand, a->slot, a->attack,
        a->target, a->option, a->copy);
    for (i = 0; i < a->selectionCount; i++)
        Add(b, "%s%u", i ? "," : "", a->selections[i]);
    Add(b, "]}");
}

// the small JSON numbers parsed by hand (the app's own actions: {"t":..,"x":[..]})
static long Num(const char *json, const char *key, long otherwise)
{
    char pat[16];
    const char *p;
    snprintf(pat, sizeof(pat), "\"%s\":", key);
    if ((p = strstr(json, pat)) == NULL)
        return otherwise;
    return strtol(p + strlen(pat), NULL, 10);
}

static int ParseAction(const char *json, struct TcgAction *a)
{
    const char *p;
    memset(a, 0, sizeof(*a));
    a->type = (uint8_t)Num(json, "t", 255);
    a->hand = (uint8_t)Num(json, "h", 0);
    a->slot = (uint8_t)Num(json, "s", 0);
    a->attack = (uint8_t)Num(json, "a", 0);
    a->target = (uint8_t)Num(json, "g", 0);
    a->option = (uint8_t)Num(json, "o", 0);
    a->copy = (uint8_t)Num(json, "c", 0);
    if ((p = strstr(json, "\"x\":[")) != NULL)
    {
        p += 5;
        while (*p && *p != ']' && a->selectionCount < TCG_DECK_SIZE)
        {
            char *end;
            long v = strtol(p, &end, 10);
            if (end == p)
                break;
            a->selections[a->selectionCount++] = (uint8_t)v;
            p = end;
            while (*p == ',' || *p == ' ')
                p++;
        }
    }
    return a->type <= TCG_DISCARD_DOLL;
}

// Applies an action (actionJson) for whoever is to move: {"ok":1} or {"ok":0,"error":...}
JNIEXPORT jstring JNICALL FN(apply)(JNIEnv *env, jclass cls, jlong h, jstring action)
{
    struct TcgDuel *g = (struct TcgDuel *)(intptr_t)h;
    struct TcgAction a;
    struct Buf b = {0};
    const char *json = (*env)->GetStringUTFChars(env, action, NULL);
    int r = TCG_INVALID;
    if (g != NULL && json != NULL && ParseAction(json, &a))
    {
        g->error[0] = 0;
        r = TcgDuel_Apply(g, &a);
    }
    if (json)
        (*env)->ReleaseStringUTFChars(env, action, json);
    Add(&b, "{\"ok\":%d,\"error\":", r == TCG_OK);
    AddStr(&b, g ? (g->error[0] ? g->error : r == TCG_OK ? "" : "That can't be done.") : "No duel.");
    Add(&b, "}");
    return Done(env, &b);
}

// The computer's move for the side to move, at LEVEL (0 a beginner .. 3 its best): an actionJson
JNIEXPORT jstring JNICALL FN(ai)(JNIEnv *env, jclass cls, jlong h, jint level)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    struct TcgAction a;
    struct Buf b = {0};
    if (g == NULL || g->phase == TCG_FINISHED)
        return NULL;
    if (!TcgDuel_AiActionLevel(g, (unsigned)(level < 0 ? 0 : level), &a))
    {
        memset(&a, 0, sizeof(a));
        a.type = g->phase == TCG_SETUP ? TCG_FINISH_SETUP : TCG_PASS;
    }
    AddAction(&b, &a);
    return Done(env, &b);
}

// A flow: a move being made (include/tcg/flow.h); Java holds the handle and answers its questions
static jstring Question(JNIEnv *env, const struct TcgFlow *f, const struct TcgDuel *g)
{
    static const char *const kinds[] = {"done", "slot", "pick", "menu", "fail"};
    struct Buf b = {0};
    unsigned i;
    Add(&b, "{\"q\":\"%s\",\"prompt\":", kinds[f->q]);
    AddStr(&b, f->prompt ? f->prompt : "");
    Add(&b, ",\"error\":");
    AddStr(&b, f->error);
    if (f->q == TCG_Q_SLOT)
        Add(&b, ",\"who\":%u,\"mask\":%u", f->who, f->slotMask);
    if (f->q == TCG_Q_PICK)
    {
        Add(&b, ",\"owner\":%u,\"source\":%u,\"min\":%u,\"max\":%u,\"value\":%u,\"cards\":[", f->pickOwner, f->pickSource,
            f->pickMin, f->pickMax, f->pickValue);
        for (i = 0; i < f->pickCount; i++)
            Add(&b, "%s[%u,%u,%u]", i ? "," : "", f->pick[i], g->players[f->pickOwner].cards[f->pick[i]], f->pickOk[i]);
        Add(&b, "]");
    }
    if (f->q == TCG_Q_MENU)
    {
        Add(&b, ",\"menu\":[");
        for (i = 0; i < f->menu.count; i++)
        {
            Add(&b, "%s[", i ? "," : "");
            AddStr(&b, f->menu.label[i]);
            Add(&b, ",%u,%u,", f->menu.value[i], f->menu.ok[i]);
            AddStr(&b, f->menu.why[i] ? f->menu.why[i] : "");
            Add(&b, "]");
        }
        Add(&b, "]");
    }
    if (f->q == TCG_Q_DONE)
    {
        Add(&b, ",\"action\":");
        AddAction(&b, &f->action);
    }
    Add(&b, "}");
    return Done(env, &b);
}

JNIEXPORT jlong JNICALL FN(flowNew)(JNIEnv *env, jclass cls)
{
    return (jlong)(intptr_t)calloc(1, sizeof(struct TcgFlow));
}

JNIEXPORT void JNICALL FN(flowEnd)(JNIEnv *env, jclass cls, jlong flow)
{
    free((void *)(intptr_t)flow);
}

JNIEXPORT jstring JNICALL FN(flowBegin)(JNIEnv *env, jclass cls, jlong flow, jlong h, jint me, jint kind, jint index)
{
    struct TcgFlow *f = (struct TcgFlow *)(intptr_t)flow;
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    if (f == NULL || g == NULL)
        return NULL;
    TcgFlow_Begin(f, g, (unsigned)me, (unsigned)kind, (unsigned)index);
    return Question(env, f, g);
}

JNIEXPORT jstring JNICALL FN(flowSlot)(JNIEnv *env, jclass cls, jlong flow, jlong h, jint slot)
{
    struct TcgFlow *f = (struct TcgFlow *)(intptr_t)flow;
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    if (f == NULL || g == NULL)
        return NULL;
    TcgFlow_Slot(f, g, (unsigned)slot);
    return Question(env, f, g);
}

JNIEXPORT jstring JNICALL FN(flowPick)(JNIEnv *env, jclass cls, jlong flow, jlong h, jintArray chosen)
{
    struct TcgFlow *f = (struct TcgFlow *)(intptr_t)flow;
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    jint v[TCG_DECK_SIZE];
    uint8_t c[TCG_DECK_SIZE];
    unsigned i, n = chosen ? (unsigned)(*env)->GetArrayLength(env, chosen) : 0;
    if (f == NULL || g == NULL)
        return NULL;
    if (n > TCG_DECK_SIZE)
        n = TCG_DECK_SIZE;
    if (n)
        (*env)->GetIntArrayRegion(env, chosen, 0, (jsize)n, v);
    for (i = 0; i < n; i++)
        c[i] = (uint8_t)v[i];
    TcgFlow_Pick(f, g, c, n);
    return Question(env, f, g);
}

JNIEXPORT jstring JNICALL FN(flowMenu)(JNIEnv *env, jclass cls, jlong flow, jlong h, jint value)
{
    struct TcgFlow *f = (struct TcgFlow *)(intptr_t)flow;
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    if (f == NULL || g == NULL)
        return NULL;
    TcgFlow_Menu(f, g, (unsigned)value);
    return Question(env, f, g);
}

// The card of a player's physical card (a pick shows them face up: they're the player's to see)
JNIEXPORT jint JNICALL FN(cardOf)(JNIEnv *env, jclass cls, jlong h, jint player, jint physical)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    if (g == NULL || physical < 0 || physical >= TCG_DECK_SIZE)
        return 0;
    return g->players[player & 1].cards[physical];
}

// The rules state's hash (tcg_link.c: two phones replaying the same moves agree on it)
JNIEXPORT jint JNICALL FN(hash)(JNIEnv *env, jclass cls, jlong h)
{
    const struct TcgDuel *g = (const struct TcgDuel *)(intptr_t)h;
    return g ? (jint)TcgLink_Hash(g) : 0;
}
