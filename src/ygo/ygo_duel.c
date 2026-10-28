/* Yu-Gi-Oh! rules engine: zones, summons, chains, battle, turns. See include/ygo/duel.h for the shape of it. */
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "ygo_internal.h"

/* ---------------------------------------------------------------------------------------------------------------
 * Running units and asking questions
 *
 * A unit runs on a copy of the state. When it asks a question the copy has no answer for, the engine publishes the
 * question and throws the copy away; with the answer, it runs the unit again from its start, the earlier answers
 * replayed in order. A unit that finishes is committed. */
static struct
{
    jmp_buf jump;
    struct YgoDuel *duel;
    unsigned used;
    int active;
} R;

enum { RUN_DONE, RUN_ASK, RUN_BAD };

static int ValidAnswer(const struct YgoQuery *q, const struct YgoAnswer *a, char *why, size_t size)
{
    unsigned i, j;
    if (a->count < q->min || a->count > q->max)
    {
        snprintf(why, size, "pick %u to %u", q->min, q->max);
        return 0;
    }
    for (i = 0; i < a->count; i++)
    {
        if (a->picks[i] >= q->count)
        {
            snprintf(why, size, "no choice %u", a->picks[i]);
            return 0;
        }
        for (j = 0; j < i; j++)
            if (a->picks[j] == a->picks[i])
            {
                snprintf(why, size, "choice %u twice", a->picks[i]);
                return 0;
            }
    }
    return 1;
}

/* The answer to q: from the replayed answers, or the engine stops here and asks */
static void Ask(struct YgoState *s, struct YgoQuery *q, struct YgoAnswer *out)
{
    struct YgoDuel *d = R.duel;
    (void)s;
    if (!R.active)
    {
        /* asked outside a unit (a bug): answer "nothing" */
        memset(out, 0, sizeof(*out));
        return;
    }
    if (R.used < d->answerCount)
    {
        *out = d->answers[R.used];
        if (!ValidAnswer(q, out, d->error, sizeof(d->error)))
        {
            d->query = *q;
            longjmp(R.jump, RUN_BAD);
        }
        R.used++;
        return;
    }
    d->query = *q;
    longjmp(R.jump, RUN_ASK);
}

static void QueryInit(struct YgoQuery *q, unsigned type, unsigned player, unsigned prompt, uint16_t card)
{
    memset(q, 0, sizeof(*q));
    q->type = type;
    q->player = player;
    q->prompt = prompt;
    q->card = card;
    q->min = q->max = 1;
}
static void QueryAdd(struct YgoQuery *q, unsigned kind, unsigned obj, unsigned value, unsigned flags)
{
    if (q->count >= YGO_QUERY_MAX)
        return;
    q->items[q->count].kind = kind;
    q->items[q->count].obj = obj;
    q->items[q->count].value = value;
    q->items[q->count].flags = flags;
    q->count++;
}

unsigned Ygo_AskCards(struct YgoState *s, unsigned player, unsigned prompt, uint16_t card, const uint8_t *list,
                      unsigned count, unsigned min, unsigned max, uint8_t *out)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    unsigned i;
    if (max > count)
        max = count;
    if (min > max)
        min = max;
    if (!count || !max)
        return 0;
    QueryInit(&q, YGO_Q_CARDS, player, prompt, card);
    for (i = 0; i < count && i < YGO_QUERY_MAX; i++)
        QueryAdd(&q, 0, list[i], 0, 0);
    q.min = min;
    q.max = max;
    /* nothing to choose: all of them */
    if (min == count && max == count)
    {
        for (i = 0; i < count; i++)
            out[i] = list[i];
        return count;
    }
    Ask(s, &q, &a);
    for (i = 0; i < a.count; i++)
        out[i] = list[a.picks[i]];
    return a.count;
}

int Ygo_AskYesNo(struct YgoState *s, unsigned player, unsigned prompt, uint16_t card)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    QueryInit(&q, YGO_Q_YESNO, player, prompt, card);
    QueryAdd(&q, 0, NO_OBJ, 0, 0);
    QueryAdd(&q, 0, NO_OBJ, 1, 0);
    Ask(s, &q, &a);
    return a.picks[0] == 1;
}

unsigned Ygo_AskOption(struct YgoState *s, unsigned player, uint16_t card, const uint8_t *options, unsigned count)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    unsigned i;
    if (count <= 1)
        return count ? options[0] : 0;
    QueryInit(&q, YGO_Q_OPTION, player, YGO_P_ANY, card);
    for (i = 0; i < count; i++)
        QueryAdd(&q, 0, NO_OBJ, options[i], 0);
    Ask(s, &q, &a);
    return options[a.picks[0]];
}

unsigned Ygo_AskPosition(struct YgoState *s, unsigned player, uint16_t card, unsigned allowed)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    unsigned i;
    QueryInit(&q, YGO_Q_POSITION, player, YGO_P_SUMMON, card);
    for (i = 0; i < 3; i++)
        if (allowed & (1u << i))
            QueryAdd(&q, 0, NO_OBJ, i, 0);
    if (q.count == 1)
        return q.items[0].value;
    if (!q.count)
        return 0;
    Ask(s, &q, &a);
    return q.items[a.picks[0]].value;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Small things */
uint32_t Ygo_Rand(struct YgoState *s, unsigned below)
{
    /* xorshift32: the same on every machine */
    uint32_t x = s->rng ? s->rng : 0x9E3779B9u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s->rng = x;
    return below ? x % below : x;
}

void Ygo_Event(struct YgoState *s, unsigned type, unsigned player, unsigned obj, unsigned target, int value,
               unsigned flags)
{
    struct YgoEvent *e = &s->events[s->eventSeq % YGO_EVENT_RING];
    e->type = type;
    e->player = player;
    e->obj = obj;
    e->target = target;
    e->card = obj < s->objCount ? s->objs[obj].card : 0;
    e->value = value;
    e->flags = flags;
    s->eventSeq++;
}

const struct YgoEvent *YgoDuel_GetEvent(const struct YgoState *s, uint32_t seq)
{
    if (seq >= s->eventSeq || s->eventSeq - seq > YGO_EVENT_RING)
        return NULL;
    return &s->events[seq % YGO_EVENT_RING];
}

int Ygo_OnField(const struct YgoState *s, unsigned o)
{
    return o < s->objCount && (s->objs[o].location == YGO_LOC_MZONE || s->objs[o].location == YGO_LOC_SZONE);
}
int Ygo_FaceUpMonster(const struct YgoState *s, unsigned o)
{
    return o < s->objCount && s->objs[o].location == YGO_LOC_MZONE && s->objs[o].faceUp;
}
int Ygo_IsMonsterObj(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    return c && (c->frame <= YGO_RITUAL || c->frame == YGO_TOKEN);
}
unsigned Ygo_MonsterCount(const struct YgoState *s, unsigned player)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_ZONES; i++)
        n += s->players[player].monsters[i] != 0;
    return n;
}
unsigned Ygo_FreeZones(const struct YgoState *s, unsigned player, int spells)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_ZONES; i++)
        n += (spells ? s->players[player].spells[i] : s->players[player].monsters[i]) == 0;
    return n;
}

unsigned Ygo_List(const struct YgoState *s, unsigned player, unsigned location, uint8_t *out)
{
    const struct YgoPlayer *p = &s->players[player];
    unsigned i, n = 0;
    switch (location)
    {
    case YGO_LOC_DECK:
        memcpy(out, p->deck, p->deckCount);
        return p->deckCount;
    case YGO_LOC_HAND:
        memcpy(out, p->hand, p->handCount);
        return p->handCount;
    case YGO_LOC_GRAVE:
        memcpy(out, p->grave, p->graveCount);
        return p->graveCount;
    case YGO_LOC_BANISHED:
        memcpy(out, p->banished, p->banishedCount);
        return p->banishedCount;
    case YGO_LOC_EXTRA:
        memcpy(out, p->extra, p->extraCount);
        return p->extraCount;
    case YGO_LOC_MZONE:
        for (i = 0; i < YGO_ZONES; i++)
            if (p->monsters[i])
                out[n++] = p->monsters[i] - 1;
        return n;
    case YGO_LOC_SZONE:
        for (i = 0; i <= YGO_FIELD_ZONE; i++)
            if (p->spells[i])
                out[n++] = p->spells[i] - 1;
        return n;
    }
    return 0;
}

int Ygo_Exists(const struct YgoState *s, unsigned player, unsigned location,
               int (*filter)(const struct YgoState *, unsigned))
{
    uint8_t list[YGO_OBJS];
    unsigned i, n, p;
    for (p = 0; p < 2; p++)
    {
        if (player != 2 && p != player)
            continue;
        n = Ygo_List(s, p, location, list);
        for (i = 0; i < n; i++)
            if (!filter || filter(s, list[i]))
                return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Effects that apply now */

/* Is this object's text in force (face up where its effects work, not negated)? */
int Ygo_EffectsApply(const struct YgoState *s, unsigned o)
{
    const struct YgoObj *b = &s->objs[o];
    const struct YgoCard *c = Ygo_Card(b->card);
    if (!c || b->negated)
        return 0;
    if (Ygo_OnField(s, o) && !b->faceUp)
        return 0;
    if (c->frame == YGO_TRAP && Ygo_OnField(s, o))
    {
        /* Jinzo and the like (but not its own rule check: that would loop) */
        unsigned i, p, k;
        for (p = 0; p < 2; p++)
            for (i = 0; i < YGO_ZONES; i++)
            {
                unsigned m = s->players[p].monsters[i];
                const struct YgoEffect *e;
                unsigned n;
                if (!m || !s->objs[m - 1].faceUp || s->objs[m - 1].negated)
                    continue;
                e = Ygo_Effects(s->objs[m - 1].card, &n);
                for (k = 0; k < n; k++)
                    if (e[k].rule && e[k].rule(s, m - 1, YR_TRAPS_NEGATED, o, 0))
                        return 0;
            }
    }
    return 1;
}

static int LastingMatches(const struct YgoState *s, const struct YgoLasting *l, unsigned a)
{
    switch (l->scope)
    {
    case YGO_SCOPE_PLAYER:
        return a == l->player || l->player == 2;
    case YGO_SCOPE_OBJ:
        return a == l->obj;
    case YGO_SCOPE_MONSTERS:
        return a < s->objCount && s->objs[a].location == YGO_LOC_MZONE &&
               (l->player == 2 || s->objs[a].controller == l->player);
    }
    return 0;
}

/* Rules whose hooks add up (how many more attacks, how much LP an attack costs, what is destroyed instead) */
static int Additive(unsigned rule)
{
    return rule == YR_EXTRA_ATTACK || rule == YR_ATTACK_COST || rule == YR_BATTLE_SUBSTITUTE;
}

/* Every continuous hook in force: face-up cards on the field (both sides), and lasting effects */
int Ygo_Rule(const struct YgoState *s, unsigned rule, unsigned a, unsigned b)
{
    unsigned p, i, k, n;
    int total = 0;
    for (i = 0; i < s->lastingCount; i++)
        if (s->lasting[i].kind == rule && LastingMatches(s, &s->lasting[i], a))
        {
            if (Additive(rule))
                total += s->lasting[i].value;
            else
                return 1;
        }
    for (p = 0; p < 2; p++)
        for (i = 0; i < YGO_ZONES * 2 + 1; i++)
        {
            unsigned slot = i < YGO_ZONES ? s->players[p].monsters[i] : s->players[p].spells[i - YGO_ZONES];
            const struct YgoEffect *e;
            if (!slot)
                continue;
            if (!Ygo_EffectsApply(s, slot - 1))
                continue;
            e = Ygo_Effects(s->objs[slot - 1].card, &n);
            for (k = 0; k < n; k++)
                if (e[k].rule)
                {
                    int r = e[k].rule(s, slot - 1, rule, a, b);
                    if (Additive(rule))
                        total += r;
                    else if (r)
                        return 1;
                }
        }
    return Additive(rule) ? total : 0;
}

int Ygo_CanTarget(const struct YgoState *s, unsigned o, unsigned player)
{
    return !Ygo_Rule(s, YR_UNTARGETABLE, o, player);
}

static int Stat(const struct YgoState *s, unsigned o, int which)
{
    const struct YgoObj *b = &s->objs[o];
    const struct YgoCard *c = Ygo_Card(b->card);
    unsigned p, i, k, n;
    int v;
    if (!c)
        return 0;
    int base, reverse = 0;
    if (which == YS_LEVEL)
        v = base = c->level;
    else
    {
        unsigned raw;
        int swap = b->location == YGO_LOC_MZONE && Ygo_Rule(s, YR_SWAP_STATS, o, 0);
        raw = (which == YS_ATK) != swap ? c->atk : c->def;
        v = base = raw == YGO_STAT_UNKNOWN ? 0 : (int)raw;
        if (b->location == YGO_LOC_MZONE && which == YS_ATK && Ygo_Rule(s, YR_ATK_ZERO, o, 0))
            return 0;
        reverse = b->location == YGO_LOC_MZONE && Ygo_Rule(s, YR_REVERSE_STATS, o, 0);
        v += which == YS_ATK ? b->atkMod + b->atkTurn : b->defMod + b->defTurn;
    }
    if (b->location != YGO_LOC_MZONE)
        return v < 0 ? 0 : v;
    /* its own continuous effects, then everything else on the field */
    for (p = 0; p < 2; p++)
        for (i = 0; i < YGO_ZONES * 2 + 1; i++)
        {
            unsigned slot = i < YGO_ZONES ? s->players[p].monsters[i] : s->players[p].spells[i - YGO_ZONES];
            const struct YgoEffect *e;
            if (!slot || !Ygo_EffectsApply(s, slot - 1))
                continue;
            e = Ygo_Effects(s->objs[slot - 1].card, &n);
            for (k = 0; k < n; k++)
                if (e[k].stat)
                    v = e[k].stat(s, slot - 1, o, which, v);
        }
    if (reverse)
        v = base - (v - base);   /* Reverse Trap: every addition a subtraction, and back */
    return v < 0 ? 0 : v;
}
int YgoDuel_Atk(const struct YgoState *s, unsigned o) { return Stat(s, o, YS_ATK); }
int YgoDuel_Def(const struct YgoState *s, unsigned o) { return Stat(s, o, YS_DEF); }
int YgoDuel_Level(const struct YgoState *s, unsigned o) { return Stat(s, o, YS_LEVEL); }

/* ---------------------------------------------------------------------------------------------------------------
 * Moving cards */
static uint8_t *Pile(struct YgoState *s, unsigned player, unsigned location, uint8_t **count)
{
    struct YgoPlayer *p = &s->players[player];
    switch (location)
    {
    case YGO_LOC_DECK: *count = &p->deckCount; return p->deck;
    case YGO_LOC_HAND: *count = &p->handCount; return p->hand;
    case YGO_LOC_GRAVE: *count = &p->graveCount; return p->grave;
    case YGO_LOC_BANISHED: *count = &p->banishedCount; return p->banished;
    case YGO_LOC_EXTRA: *count = &p->extraCount; return p->extra;
    }
    return NULL;
}

/* Takes o out of wherever it is */
static void Lift(struct YgoState *s, unsigned o)
{
    struct YgoObj *b = &s->objs[o];
    struct YgoPlayer *p = &s->players[b->controller];
    uint8_t *count, *pile;
    unsigned i;
    b->moves++;
    switch (b->location)
    {
    case YGO_LOC_MZONE:
        if (b->seq < YGO_ZONES && p->monsters[b->seq] == o + 1)
            p->monsters[b->seq] = 0;
        break;
    case YGO_LOC_SZONE:
        if (b->seq <= YGO_FIELD_ZONE && p->spells[b->seq] == o + 1)
            p->spells[b->seq] = 0;
        break;
    default:
        pile = Pile(s, b->location == YGO_LOC_HAND || b->location == YGO_LOC_DECK ? b->controller : b->owner,
                    b->location, &count);
        if (!pile)
            break;
        for (i = 0; i < *count; i++)
            if (pile[i] == o)
            {
                memmove(pile + i, pile + i + 1, *count - i - 1);
                (*count)--;
                break;
            }
    }
    b->location = YGO_LOC_NONE;
}

/* Leaving the field forgets everything that was done to it there */
static void Forget(struct YgoObj *b)
{
    b->atkMod = b->defMod = b->atkTurn = b->defTurn = 0;
    b->negated = 0;
    b->equipTo = 0;
    b->counters = 0;
    b->data = 0;
    b->attacks = 0;
    b->summonType = YGO_SUMMON_NONE;
}

static void PutInPile(struct YgoState *s, unsigned o, unsigned location, int top)
{
    struct YgoObj *b = &s->objs[o];
    unsigned owner = b->owner;
    uint8_t *count, *pile;
    int wasField = b->location == YGO_LOC_MZONE || b->location == YGO_LOC_SZONE;
    Lift(s, o);
    if (wasField || location != YGO_LOC_HAND)
        Forget(b);
    b->controller = owner;
    pile = Pile(s, owner, location, &count);
    if (!pile)
        return;
    if (top || location != YGO_LOC_DECK)
        pile[(*count)++] = o;
    else
    {
        memmove(pile + 1, pile, *count);
        pile[0] = o;
        (*count)++;
    }
    b->location = location;
    b->seq = 0;
    if (location == YGO_LOC_HAND || location == YGO_LOC_DECK || location == YGO_LOC_EXTRA)
        b->properly = 0;
    b->faceUp = location == YGO_LOC_GRAVE || location == YGO_LOC_BANISHED;
    b->attackPos = 0;
    b->enteredTurn = s->turn;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Triggers */
static void AddPending(struct YgoState *s, unsigned o, unsigned effect, unsigned eventObj)
{
    unsigned i, n;
    const struct YgoEffect *e = Ygo_Effects(s->objs[o].card, &n);
    if (s->over)
        return;
    for (i = 0; i < s->pendingCount; i++)
        if (s->pending[i].obj == o && s->pending[i].effect == effect)
            return;
    if (s->pendingCount >= YGO_PENDING_MAX)
        return;
    s->pending[s->pendingCount].obj = o;
    s->pending[s->pendingCount].effect = effect;
    s->pending[s->pendingCount].player = s->objs[o].controller;
    s->pending[s->pendingCount].eventObj = eventObj;
    s->pending[s->pendingCount].optional = (e[effect].flags & YF_OPTIONAL) != 0;
    s->pending[s->pendingCount].when = (e[effect].flags & YF_WHEN) != 0;
    s->pending[s->pendingCount].serial = s->serial;
    s->pendingCount++;
}

/* Something happened (YEV_*): o is what it happened to, other the card that did it (or NO_OBJ) */
static void Raise(struct YgoState *s, uint32_t event, unsigned o, unsigned other)
{
    unsigned i, k, n;
    s->serial++;
    for (i = 0; i < s->objCount; i++)
    {
        const struct YgoObj *b = &s->objs[i];
        const struct YgoEffect *e;
        int self = i == o;
        e = Ygo_Effects(b->card, &n);
        if (!n)
            continue;
        for (k = 0; k < n; k++)
        {
            uint32_t ev = (e[k].type == YE_FLIP ? YEV_FLIPPED : e[k].events) & event;
            if ((e[k].type != YE_TRIGGER && e[k].type != YE_FLIP) || !ev)
                continue;
            if (e[k].type == YE_FLIP && !self)
                continue;
            if (ev & (YEV_ANY_SUMMONED | YEV_ANY_DESTROYED | YEV_ANY_NORMAL_SUMMONED))
            {
                /* about any card: works from the field */
                if (!Ygo_OnField(s, i) || !Ygo_EffectsApply(s, i))
                    continue;
            }
            else if (ev & (YEV_STANDBY_OWN | YEV_STANDBY_ANY | YEV_END_OWN | YEV_END_ANY | YEV_DRAW_OWN))
            {
                if ((ev & (YEV_STANDBY_OWN | YEV_END_OWN | YEV_DRAW_OWN)) && b->controller != s->current)
                    continue;
                if (b->location == YGO_LOC_GRAVE ? !(e[k].flags & YF_IN_GRAVE)
                                                 : (!Ygo_OnField(s, i) || !Ygo_EffectsApply(s, i)))
                    continue;
            }
            else
            {
                if (!self)
                    continue;
                /* its own event: it may be in the Graveyard by now (Sangan, Witch of the Black Forest) */
                if (b->negated)
                    continue;
                if (b->location == YGO_LOC_MZONE && !Ygo_EffectsApply(s, i))
                    continue;
            }
            AddPending(s, i, k, other);
        }
    }
}

/* ---------------------------------------------------------------------------------------------------------------
 * Doing things */
unsigned Ygo_Draw(struct YgoState *s, unsigned player, unsigned count)
{
    struct YgoPlayer *p = &s->players[player];
    unsigned n = 0;
    while (count--)
    {
        unsigned o;
        if (!p->deckCount)
        {
            p->drewNothing = 1;
            break;
        }
        o = p->deck[p->deckCount - 1];
        PutInPile(s, o, YGO_LOC_HAND, 1);
        Ygo_Event(s, YGO_EV_DRAW, player, o, NO_OBJ, 0, 0);
        n++;
    }
    return n;
}

void Ygo_Damage(struct YgoState *s, unsigned player, int amount, int battle)
{
    if (amount <= 0)
        return;
    s->players[player].lp -= amount;
    if (s->players[player].lp < 0)
        s->players[player].lp = 0;
    Ygo_Event(s, YGO_EV_DAMAGE, player, NO_OBJ, NO_OBJ, amount, battle);
}

void Ygo_Recover(struct YgoState *s, unsigned player, int amount)
{
    if (amount <= 0)
        return;
    s->players[player].lp += amount;
    Ygo_Event(s, YGO_EV_RECOVER, player, NO_OBJ, NO_OBJ, amount, 0);
}

static void LeaveField(struct YgoState *s, unsigned o)
{
    /* Equip Spells on it lose their monster (state check sends them to the Graveyard); a monster it was equipped to
     * is free again */
    (void)s;
    (void)o;
}

void Ygo_SendToGrave(struct YgoState *s, unsigned o, unsigned reason)
{
    struct YgoObj *b = &s->objs[o];
    int fromField = Ygo_OnField(s, o);
    unsigned from = b->location;
    const struct YgoCard *c = Ygo_Card(b->card);
    if (from == YGO_LOC_GRAVE)
        return;
    if (fromField)
        LeaveField(s, o);
    if (c && c->frame == YGO_TOKEN)
    {
        Lift(s, o);
        Forget(b);
        Ygo_Event(s, YGO_EV_TO_GRAVE, b->owner, o, NO_OBJ, 0, reason);
        return;
    }
    if (fromField && from == YGO_LOC_MZONE && b->controller == b->owner)
        s->players[b->owner].monsterToGraveTurn = s->turn;
    PutInPile(s, o, YGO_LOC_GRAVE, 1);
    b->reason = reason;
    Ygo_Event(s, YGO_EV_TO_GRAVE, b->owner, o, NO_OBJ, from, reason);
    Raise(s, YEV_TO_GRAVE | (fromField ? YEV_FROM_FIELD_TO_GRAVE | YEV_SENT_FROM_FIELD : 0) |
                 ((reason & YGO_REASON_DISCARD) ? YEV_DISCARDED : 0),
          o, NO_OBJ);
}

int Ygo_Destroy(struct YgoState *s, unsigned o, unsigned reason)
{
    if (!Ygo_OnField(s, o))
        return 0;
    if ((reason & YGO_REASON_EFFECT) && Ygo_Rule(s, YR_INDESTRUCTIBLE_EFFECT, o, 0))
        return 0;
    Ygo_Event(s, YGO_EV_DESTROY, s->objs[o].controller, o, NO_OBJ, 0, reason);
    Ygo_SendToGrave(s, o, reason | YGO_REASON_DESTROY);
    if (reason & YGO_REASON_EFFECT)
        Raise(s, YEV_DESTROYED_BY_EFFECT, o, NO_OBJ);
    Raise(s, YEV_ANY_DESTROYED, NO_OBJ, o);
    return 1;
}

void Ygo_ToHand(struct YgoState *s, unsigned o, unsigned reason)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    if (c && (c->frame == YGO_TOKEN))
    {
        Lift(s, o);
        Forget(&s->objs[o]);
        return;
    }
    if (c && c->frame == YGO_FUSION)
    {
        /* a Fusion Monster goes back to the Fusion Deck */
        PutInPile(s, o, YGO_LOC_EXTRA, 1);
        Ygo_Event(s, YGO_EV_TO_DECK, s->objs[o].owner, o, NO_OBJ, 0, reason);
        return;
    }
    PutInPile(s, o, YGO_LOC_HAND, 1);
    s->objs[o].reason = reason;
    Ygo_Event(s, YGO_EV_TO_HAND, s->objs[o].owner, o, NO_OBJ, 0, reason);
}

void Ygo_ToDeck(struct YgoState *s, unsigned o, int top)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    if (c && c->frame == YGO_TOKEN)
    {
        Lift(s, o);
        Forget(&s->objs[o]);
        return;
    }
    PutInPile(s, o, c && c->frame == YGO_FUSION ? YGO_LOC_EXTRA : YGO_LOC_DECK, top);
    Ygo_Event(s, YGO_EV_TO_DECK, s->objs[o].owner, o, NO_OBJ, top, 0);
}

void Ygo_Banish(struct YgoState *s, unsigned o, unsigned reason)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    if (c && c->frame == YGO_TOKEN)
    {
        Lift(s, o);
        Forget(&s->objs[o]);
        return;
    }
    PutInPile(s, o, YGO_LOC_BANISHED, 1);
    s->objs[o].reason = reason;
    Ygo_Event(s, YGO_EV_BANISH, s->objs[o].owner, o, NO_OBJ, 0, reason);
}

void Ygo_Discard(struct YgoState *s, unsigned o, unsigned reason)
{
    Ygo_SendToGrave(s, o, reason | YGO_REASON_DISCARD);
}

void Ygo_ShuffleDeck(struct YgoState *s, unsigned player)
{
    struct YgoPlayer *p = &s->players[player];
    unsigned i;
    for (i = p->deckCount; i > 1; i--)
    {
        unsigned j = Ygo_Rand(s, i);
        uint8_t t = p->deck[i - 1];
        p->deck[i - 1] = p->deck[j];
        p->deck[j] = t;
    }
    Ygo_Event(s, YGO_EV_SHUFFLE, player, NO_OBJ, NO_OBJ, 0, 0);
}

static int FreeZone(const struct YgoState *s, unsigned player, int spells)
{
    unsigned i;
    for (i = 0; i < YGO_ZONES; i++)
        if (!(spells ? s->players[player].spells[i] : s->players[player].monsters[i]))
            return (int)i;
    return -1;
}

/* Puts o in a Monster Zone of player: 0 face-up ATK, 1 face-up DEF, 2 face-down DEF */
static int PlaceMonster(struct YgoState *s, unsigned o, unsigned player, unsigned pos)
{
    int zone = FreeZone(s, player, 0);
    struct YgoObj *b = &s->objs[o];
    if (zone < 0)
        return 0;
    if (Ygo_OnField(s, o) || b->location == YGO_LOC_GRAVE || b->location == YGO_LOC_BANISHED ||
        b->location == YGO_LOC_EXTRA || b->location == YGO_LOC_DECK)
        Forget(b);
    Lift(s, o);
    b->controller = player;
    b->location = YGO_LOC_MZONE;
    b->seq = zone;
    b->faceUp = pos != 2;
    b->attackPos = pos == 0;
    b->turnFlags |= YGO_TF_SUMMONED;
    b->enteredTurn = s->turn;
    b->attacks = 0;
    s->players[player].monsters[zone] = o + 1;
    return 1;
}

int Ygo_NeedsProper(unsigned card)
{
    const struct YgoCard *c = Ygo_Card(card);
    unsigned n, k;
    const struct YgoEffect *e;
    if (!c)
        return 0;
    if (c->frame == YGO_FUSION || c->frame == YGO_RITUAL)
        return 1;
    e = Ygo_Effects(card, &n);
    for (k = 0; k < n; k++)
        if (e[k].type == YE_SUMMON_RULE)
            return 1;
    return 0;
}

int Ygo_CanSpecialSummon(const struct YgoState *s, unsigned o, unsigned player)
{
    if (Ygo_FreeZones(s, player, 0) == 0 || Ygo_Rule(s, YR_CANNOT_SPECIAL_SUMMON, player, 0) ||
        Ygo_Rule(s, YR_NO_SUMMON, player, 0))
        return 0;
    /* a Fusion / Ritual / own-rule monster must have been Summoned properly first, unless this is that */
    return !Ygo_NeedsProper(s->objs[o].card) || s->objs[o].properly || s->objs[o].location == YGO_LOC_EXTRA ||
           s->objs[o].location == YGO_LOC_HAND;
}

int Ygo_SpecialSummon(struct YgoState *s, unsigned o, unsigned player, unsigned pos, unsigned type)
{
    if (type != YGO_SUMMON_FUSION && type != YGO_SUMMON_RITUAL && !Ygo_CanSpecialSummon(s, o, player))
        return 0;
    if (!PlaceMonster(s, o, player, pos))
        return 0;
    if (type == YGO_SUMMON_FUSION || type == YGO_SUMMON_RITUAL)
        s->objs[o].properly = 1;
    s->players[player].summonedTurn = s->turn;
    s->objs[o].summonType = type ? type : YGO_SUMMON_SPECIAL;
    Ygo_Event(s, YGO_EV_SUMMON, player, o, NO_OBJ, s->objs[o].summonType, pos);
    Raise(s, YEV_SPECIAL_SUMMONED, o, NO_OBJ);
    Raise(s, YEV_ANY_SUMMONED, NO_OBJ, o);
    return 1;
}

unsigned Ygo_MakeToken(struct YgoState *s, unsigned card, unsigned player, unsigned pos)
{
    struct YgoObj *b;
    unsigned o;
    if (s->objCount >= YGO_OBJS || Ygo_FreeZones(s, player, 0) == 0)
        return NO_OBJ;
    o = s->objCount++;
    b = &s->objs[o];
    memset(b, 0, sizeof(*b));
    b->card = card;
    b->owner = b->controller = player;
    b->location = YGO_LOC_NONE;
    if (!Ygo_SpecialSummon(s, o, player, pos, YGO_SUMMON_SPECIAL))
        return NO_OBJ;
    return o;
}

void Ygo_ChangePosition(struct YgoState *s, unsigned o, unsigned pos)
{
    struct YgoObj *b = &s->objs[o];
    int wasDown = !b->faceUp, wasAttack = b->faceUp && b->attackPos;
    if (b->location != YGO_LOC_MZONE)
        return;
    b->faceUp = pos != 2;
    b->attackPos = pos == 0;
    Ygo_Event(s, YGO_EV_POSITION, b->controller, o, NO_OBJ, pos, 0);
    if (wasAttack && pos == 1)
        Raise(s, YEV_TO_DEFENSE, o, NO_OBJ);
    if (!wasAttack && pos == 0)
        Raise(s, YEV_TO_ATTACK, o, NO_OBJ);
    if (wasDown && b->faceUp)
    {
        b->turnFlags |= YGO_TF_FLIPPED;
        Ygo_Event(s, YGO_EV_FLIP, b->controller, o, NO_OBJ, 0, 0);
        Raise(s, YEV_FLIPPED, o, NO_OBJ);
    }
}

void Ygo_FlipFaceUp(struct YgoState *s, unsigned o, int byBattle)
{
    struct YgoObj *b = &s->objs[o];
    if (b->location != YGO_LOC_MZONE || b->faceUp)
        return;
    b->faceUp = 1;
    b->turnFlags |= YGO_TF_FLIPPED;
    Ygo_Event(s, YGO_EV_FLIP, b->controller, o, NO_OBJ, byBattle, 0);
    if (!byBattle)
        Raise(s, YEV_FLIPPED, o, NO_OBJ);
}

void Ygo_TakeControl(struct YgoState *s, unsigned o, unsigned player)
{
    struct YgoObj *b = &s->objs[o];
    int zone;
    if (b->location != YGO_LOC_MZONE || b->controller == player)
        return;
    zone = FreeZone(s, player, 0);
    if (zone < 0)
        return;
    s->players[b->controller].monsters[b->seq] = 0;
    b->controller = player;
    b->seq = zone;
    s->players[player].monsters[zone] = o + 1;
    Ygo_Event(s, YGO_EV_CONTROL, player, o, NO_OBJ, 0, 0);
}

void Ygo_AddStat(struct YgoState *s, unsigned o, int atk, int def, int untilEndOfTurn)
{
    struct YgoObj *b = &s->objs[o];
    if (b->location != YGO_LOC_MZONE || !b->faceUp)
        return;
    if (untilEndOfTurn)
    {
        b->atkTurn += atk;
        b->defTurn += def;
    }
    else
    {
        b->atkMod += atk;
        b->defMod += def;
    }
    Ygo_Event(s, YGO_EV_STAT, b->controller, o, NO_OBJ, atk, def);
}

void Ygo_Equip(struct YgoState *s, unsigned equip, unsigned monster)
{
    if (!Ygo_OnField(s, equip) || !Ygo_FaceUpMonster(s, monster))
        return;
    s->objs[equip].equipTo = monster + 1;
    Ygo_Event(s, YGO_EV_EQUIP, s->objs[equip].controller, equip, monster, 0, 0);
    Raise(s, YEV_EQUIPPED, monster, equip);
    if (Ygo_Rule(s, YR_NO_EQUIP, monster, 0))
        Ygo_Destroy(s, equip, YGO_REASON_EFFECT);
}

void Ygo_AddLasting(struct YgoState *s, unsigned kind, unsigned scope, unsigned player, unsigned obj, unsigned turns,
                    int value)
{
    struct YgoLasting *l;
    if (s->lastingCount >= YGO_LASTING_MAX)
        return;
    l = &s->lasting[s->lastingCount++];
    l->kind = kind;
    l->scope = scope;
    l->player = player;
    l->obj = obj;
    l->until = turns >= YGO_UNTIL_CHAIN ? turns : s->turn + turns;
    l->value = value;
}

int Ygo_LastingValue(const struct YgoState *s, unsigned kind, unsigned player, int value)
{
    unsigned i;
    for (i = 0; i < s->lastingCount; i++)
        if (s->lasting[i].kind == kind && s->lasting[i].value == value && (player == 2 || s->lasting[i].player == player))
            return 1;
    return 0;
}

static void ExpireLasting(struct YgoState *s, unsigned until)
{
    unsigned i, k = 0;
    struct YgoLasting ended[YGO_LASTING_MAX];
    unsigned endedCount = 0;
    for (i = 0; i < s->lastingCount; i++)
    {
        int ends = until < YGO_UNTIL_CHAIN ? s->lasting[i].until <= until : s->lasting[i].until == until;
        if (ends)
            ended[endedCount++] = s->lasting[i];
        else
            s->lasting[k++] = s->lasting[i];
    }
    s->lastingCount = k;
    for (i = 0; i < endedCount; i++)
    {
        unsigned o = ended[i].obj;
        if (ended[i].kind == YL_RETURN_CONTROL && s->objs[o].location == YGO_LOC_MZONE &&
            s->objs[o].controller != s->objs[o].owner)
            Ygo_TakeControl(s, o, s->objs[o].owner);
        if (ended[i].kind == YL_DESTROY_AT && Ygo_OnField(s, o) && s->objs[o].moves == (uint8_t)ended[i].value)
            Ygo_Destroy(s, o, YGO_REASON_EFFECT);
    }
}

void Ygo_Reveal(struct YgoState *s, unsigned o)
{
    Ygo_Event(s, YGO_EV_REVEAL, s->objs[o].controller, o, NO_OBJ, 0, 0);
}

void Ygo_Look(struct YgoState *s, unsigned player, unsigned o)
{
    Ygo_Event(s, YGO_EV_LOOK, player, o, NO_OBJ, 0, 0);
}

void Ygo_NegateSummon(struct YgoState *s, unsigned o)
{
    if (s->objs[o].location != YGO_LOC_MZONE)
        return;
    Ygo_Event(s, YGO_EV_NEGATE, s->objs[o].controller, o, NO_OBJ, 0, 1);
    Ygo_Event(s, YGO_EV_DESTROY, s->objs[o].controller, o, NO_OBJ, 0, YGO_REASON_EFFECT);
    Ygo_SendToGrave(s, o, YGO_REASON_EFFECT | YGO_REASON_DESTROY);
}

int Ygo_Lasting(const struct YgoState *s, unsigned kind, unsigned player)
{
    unsigned i;
    for (i = 0; i < s->lastingCount; i++)
        if (s->lasting[i].kind == kind && (player == 2 || s->lasting[i].player == player))
            return 1 + i;
    return 0;
}

unsigned Ygo_Coin(struct YgoState *s, unsigned player)
{
    unsigned heads = Ygo_Rand(s, 2);
    Ygo_Event(s, YGO_EV_COIN, player, NO_OBJ, NO_OBJ, heads, 0);
    return heads;
}

unsigned Ygo_Dice(struct YgoState *s, unsigned player)
{
    unsigned v = 1 + Ygo_Rand(s, 6);
    Ygo_Event(s, YGO_EV_DICE, player, NO_OBJ, NO_OBJ, v, 0);
    return v;
}

void Ygo_Win(struct YgoState *s, unsigned player, unsigned reason)
{
    if (s->over)
        return;
    s->over = 1;
    s->winner = player;
    s->winReason = reason;
    Ygo_Event(s, YGO_EV_WIN, player, NO_OBJ, NO_OBJ, reason, 0);
}

void Ygo_NegateLink(struct YgoState *s, unsigned link, int destroy)
{
    struct YgoLink *l;
    if (link >= s->chainCount)
        return;
    l = &s->chain[link];
    l->negated = 1 + (destroy != 0);
    Ygo_Event(s, YGO_EV_NEGATE, l->player, l->obj, NO_OBJ, link + 1, 0);
}

/* ---------------------------------------------------------------------------------------------------------------
 * State checks: after every unit */
static void CheckState(struct YgoState *s)
{
    unsigned p, i;
    static const uint16_t exodia[5] = {YGO_CARD_EXODIA_THE_FORBIDDEN_ONE, YGO_CARD_LEFT_ARM_OF_THE_FORBIDDEN_ONE,
                                       YGO_CARD_RIGHT_ARM_OF_THE_FORBIDDEN_ONE, YGO_CARD_LEFT_LEG_OF_THE_FORBIDDEN_ONE,
                                       YGO_CARD_RIGHT_LEG_OF_THE_FORBIDDEN_ONE};
    if (s->over)
        return;
    /* Equip Spells whose monster left (or turned face down) */
    for (p = 0; p < 2; p++)
        for (i = 0; i < YGO_ZONES; i++)
        {
            unsigned e = s->players[p].spells[i];
            if (e && s->objs[e - 1].equipTo && !Ygo_FaceUpMonster(s, s->objs[e - 1].equipTo - 1))
            {
                Ygo_Destroy(s, e - 1, YGO_REASON_RULE);
            }
        }
    /* Exodia */
    for (p = 0; p < 2; p++)
    {
        unsigned k, j, have = 0;
        for (k = 0; k < 5; k++)
            for (j = 0; j < s->players[p].handCount; j++)
                if (s->objs[s->players[p].hand[j]].card == exodia[k])
                {
                    have++;
                    break;
                }
        if (have == 5)
        {
            Ygo_Win(s, p, YGO_WIN_EXODIA);
            return;
        }
    }
    for (p = 0; p < 2; p++)
        if (s->players[p].lp <= 0)
        {
            if (s->players[p ^ 1].lp <= 0)
                Ygo_Win(s, s->current ^ 1, YGO_WIN_LP); /* both at 0: rare; the turn player's opponent wins */
            else
                Ygo_Win(s, p ^ 1, YGO_WIN_LP);
            return;
        }
    for (p = 0; p < 2; p++)
        if (s->players[p].drewNothing)
        {
            Ygo_Win(s, p ^ 1, YGO_WIN_DECK_OUT);
            return;
        }
}

/* ---------------------------------------------------------------------------------------------------------------
 * Activations */

/* Can o's effect k be activated by player now (no questions asked)? idle: the turn player's open Main Phase
 * (Spell Speed 1 allowed). */
static int CanActivate(struct YgoState *s, unsigned player, unsigned o, unsigned k, int idle)
{
    struct YgoObj *b = &s->objs[o];
    const struct YgoCard *c = Ygo_Card(b->card);
    unsigned n, speed;
    const struct YgoEffect *e = Ygo_Effects(b->card, &n);
    struct YgoCtx ctx;
    if (!c || k >= n || s->over)
        return 0;
    e += k;
    if (b->controller != player && b->location != YGO_LOC_HAND)
        return 0;
    if (b->location == YGO_LOC_HAND && b->owner != player)
        return 0;
    if ((e->flags & YF_ONCE_PER_TURN) && (b->usedTurn & (1u << k)))
        return 0;
    if (s->lastingCount && Ygo_LastingValue(s, YL_NO_ACTIVATE_NAME, player, b->card))
        return 0;
    speed = e->speed ? e->speed : 1;
    switch (e->type)
    {
    case YE_ACTIVATE:
        if (c->frame == YGO_SPELL)
        {
            if (Ygo_Rule(s, YR_NO_SPELLS, player, 0))
                return 0;
            if (b->location == YGO_LOC_HAND)
            {
                if (c->property != YGO_PROP_FIELD && !Ygo_FreeZones(s, player, 1))
                    return 0;
                if (c->property == YGO_PROP_QUICK)
                    speed = 2;
                if (c->property == YGO_PROP_QUICK && s->current != player)
                    return 0; /* a Quick-Play Spell from the hand: only on your own turn */
            }
            else if (b->location == YGO_LOC_SZONE && !b->faceUp)
            {
                if (c->property == YGO_PROP_QUICK)
                {
                    speed = 2;
                    if (b->enteredTurn == s->turn)
                        return 0;
                }
            }
            else
                return 0;
            if (c->property != YGO_PROP_QUICK)
                speed = 1;
        }
        else if (c->frame == YGO_TRAP)
        {
            if (Ygo_Rule(s, YR_NO_TRAPS, player, 0) || Ygo_Rule(s, YR_NO_TRAPS, 2, 0))
                return 0;
            if (b->location != YGO_LOC_SZONE || b->faceUp || b->enteredTurn == s->turn)
                return 0;
            speed = c->property == YGO_PROP_COUNTER ? 3 : 2;
        }
        else
            return 0;
        break;
    case YE_IGNITION:
    case YE_QUICK:
        if (e->flags & YF_FROM_HAND)
        {
            if (b->location != YGO_LOC_HAND)
                return 0;
        }
        else if (!Ygo_OnField(s, o) || !b->faceUp || !Ygo_EffectsApply(s, o))
            return 0;
        if ((e->flags & YF_HARD_ONCE) && (b->data & 0x80000000u))
            return 0;
        if (e->type == YE_IGNITION)
            speed = 1;
        else if (speed < 2)
            speed = 2;
        break;
    default:
        return 0;
    }
    /* Spell Speed: 1 only in your own open Main Phase with no chain */
    if (speed == 1 && !(idle && s->chainCount == 0 && s->current == player &&
                        (s->phase == YGO_PHASE_MAIN1 || s->phase == YGO_PHASE_MAIN2)))
        return 0;
    if (s->chainCount)
    {
        unsigned top = s->chain[s->chainCount - 1].obj;
        unsigned tn;
        const struct YgoEffect *te = Ygo_Effects(s->objs[top].card, &tn);
        unsigned topSpeed = 2;
        const struct YgoCard *tc = Ygo_ObjCard(s, top);
        if (s->chain[s->chainCount - 1].effect < tn)
        {
            te += s->chain[s->chainCount - 1].effect;
            topSpeed = te->speed ? te->speed : 1;
            if (tc && tc->frame == YGO_TRAP)
                topSpeed = tc->property == YGO_PROP_COUNTER ? 3 : 2;
            if (tc && tc->frame == YGO_SPELL && tc->property == YGO_PROP_QUICK)
                topSpeed = 2;
        }
        if (speed < 2 || speed < topSpeed)
            return 0;
    }
    /* The Damage Step and summon-negation windows take only some cards */
    if (s->damageStep && !(e->flags & YF_DAMAGE_STEP) && speed != 3)
        return 0;
    if (s->window == YW_SUMMONING && !(e->flags & YF_SUMMON_NEGATE))
        return 0;
    /* a card already activated in this chain can't go again */
    {
        unsigned i;
        for (i = 0; i < s->chainCount; i++)
            if (s->chain[i].obj == o && s->chain[i].effect == k)
                return 0;
    }
    if (!e->can)
        return 1;
    ctx.s = s;
    ctx.e = e;
    ctx.obj = o;
    ctx.player = player;
    ctx.eventObj = NO_OBJ;
    ctx.link = NULL;
    return e->can(&ctx);
}

/* Every effect player can activate now: obj/effect pairs */
static unsigned Activations(struct YgoState *s, unsigned player, int idle, struct YgoChoice *out, unsigned max)
{
    unsigned o, k, n = 0, count;
    for (o = 0; o < s->objCount && n < max; o++)
    {
        struct YgoObj *b = &s->objs[o];
        if (b->location != YGO_LOC_HAND && b->location != YGO_LOC_MZONE && b->location != YGO_LOC_SZONE)
            continue;
        Ygo_Effects(b->card, &count);
        for (k = 0; k < count && n < max; k++)
            if (CanActivate(s, player, o, k, idle))
            {
                out[n].kind = YGO_ACT_ACTIVATE;
                out[n].obj = o;
                out[n].value = k;
                out[n].flags = 0;
                n++;
            }
    }
    return n;
}

/* Activates o's effect k as a new chain link (cost and targets asked now) */
static void Activate(struct YgoState *s, unsigned player, unsigned o, unsigned k, unsigned eventObj, int trigger)
{
    struct YgoObj *b = &s->objs[o];
    const struct YgoCard *c = Ygo_Card(b->card);
    unsigned n;
    const struct YgoEffect *e = Ygo_Effects(b->card, &n) + k;
    struct YgoLink *l;
    struct YgoCtx ctx;
    if (s->chainCount >= YGO_CHAIN_MAX)
        return;
    l = &s->chain[s->chainCount];
    memset(l, 0, sizeof(*l));
    l->obj = o;
    l->effect = k;
    l->player = player;
    l->eventObj = eventObj;
    l->card = b->card;
    if (e->type == YE_ACTIVATE)
    {
        l->activation = 1;
        if (b->location == YGO_LOC_HAND)
        {
            /* onto the field, face up */
            int zone = c->property == YGO_PROP_FIELD ? YGO_FIELD_ZONE : FreeZone(s, player, 1);
            if (c->property == YGO_PROP_FIELD && s->players[player].spells[YGO_FIELD_ZONE])
                Ygo_SendToGrave(s, s->players[player].spells[YGO_FIELD_ZONE] - 1, YGO_REASON_RULE);
            if (zone < 0)
                return;
            Lift(s, o);
            b->location = YGO_LOC_SZONE;
            b->seq = zone;
            b->controller = player;
            b->enteredTurn = s->turn;
            s->players[player].spells[zone] = o + 1;
        }
        b->faceUp = 1;
    }
    if (e->flags & YF_ONCE_PER_TURN)
        b->usedTurn |= 1u << k;
    if (e->flags & YF_HARD_ONCE)
        b->data |= 0x80000000u;
    s->chainCount++;
    ctx.s = s;
    ctx.e = e;
    ctx.obj = o;
    ctx.player = player;
    ctx.eventObj = eventObj;
    ctx.link = l;
    if (e->cost)
        e->cost(&ctx);
    if (e->target)
        e->target(&ctx);
    Ygo_Event(s, YGO_EV_ACTIVATE, player, o, l->targetCount ? l->targets[0] : NO_OBJ, s->chainCount, trigger);
    {
        unsigned i;
        for (i = 0; i < l->targetCount; i++)
            Ygo_Event(s, YGO_EV_TARGET, player, o, l->targets[i], 0, 0);
    }
}

static void ResolveTop(struct YgoState *s)
{
    struct YgoLink l = s->chain[s->chainCount - 1];
    struct YgoObj *b = &s->objs[l.obj];
    const struct YgoCard *c = Ygo_Card(l.card);
    unsigned n;
    const struct YgoEffect *e = Ygo_Effects(l.card, &n);
    s->resolving = s->chainCount;
    if (l.effect < n)
        e += l.effect;
    else
        e = NULL;
    if (!l.negated && e && e->op)
    {
        struct YgoCtx ctx;
        ctx.s = s;
        ctx.e = e;
        ctx.obj = l.obj;
        ctx.player = l.player;
        ctx.eventObj = l.eventObj;
        ctx.link = &s->chain[s->chainCount - 1];
        e->op(&ctx);
    }
    Ygo_Event(s, YGO_EV_RESOLVE, l.player, l.obj, NO_OBJ, s->chainCount, l.negated);
    s->chainCount--;
    s->resolving = 0;
    /* A Spell / Trap card that resolved (or was negated) leaves, unless it stays (Continuous, Equip, Field) */
    if (l.activation && c && Ygo_OnField(s, l.obj) && b->card == l.card)
    {
        int stays = c->property == YGO_PROP_CONTINUOUS || c->property == YGO_PROP_FIELD ||
                    (c->property == YGO_PROP_EQUIP && b->equipTo) || (e && (e->flags & YF_STAYS));
        if (l.negated || !stays)
            Ygo_SendToGrave(s, l.obj, l.negated == 2 ? YGO_REASON_DESTROY | YGO_REASON_EFFECT : YGO_REASON_RULE);
    }
}

/* ---------------------------------------------------------------------------------------------------------------
 * Summoning */
static int CanNormalSummonCard(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    unsigned n, k;
    const struct YgoEffect *e;
    if (!c || c->frame == YGO_FUSION || c->frame == YGO_RITUAL || c->frame == YGO_TOKEN || c->frame >= YGO_SPELL)
        return 0;
    e = Ygo_Effects(c->id, &n);
    for (k = 0; k < n; k++)
        if (e[k].rule && e[k].rule(s, o, YR_CANNOT_NORMAL_SUMMON, c->id, 0))
            return 0;
    return 1;
}

static unsigned TributesNeeded(const struct YgoCard *c)
{
    return c->level >= 7 ? 2 : c->level >= 5 ? 1 : 0;
}

unsigned Ygo_Tributes(const struct YgoState *s, unsigned player, uint8_t *out)
{
    unsigned i, n = 0, p;
    for (p = 0; p < 2; p++)
        for (i = 0; i < YGO_ZONES; i++)
        {
            unsigned m = s->players[p].monsters[i];
            if (!m)
                continue;
            if (p != player && !Ygo_Rule(s, YR_EXTRA_TRIBUTE, m - 1, player))
                continue;
            if (Ygo_Rule(s, YR_NOT_TRIBUTE, m - 1, 0))
                continue;
            out[n++] = m - 1;
        }
    return n;
}

/* set: a Normal Set (still allowed when Summons aren't) */
static int CanNormalSummon(const struct YgoState *s, unsigned player, unsigned o, int set)
{
    unsigned need;
    uint8_t list[YGO_ZONES * 2];
    if (s->players[player].normalSummoned || !CanNormalSummonCard(s, o))
        return 0;
    if (!set && Ygo_Rule(s, YR_NO_SUMMON, player, 0))
        return 0;
    need = TributesNeeded(Ygo_ObjCard(s, o));
    if (need)
        return Ygo_Tributes(s, player, list) >= need;
    return Ygo_FreeZones(s, player, 0) > 0;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Flow: frames, each step one unit */
enum
{
    PR_TURN,
    PR_MAIN,
    PR_BATTLE,
    PR_DAMAGE,
    PR_WINDOW,  /* a: the window kind */
    PR_CHAIN,   /* a: passes in a row, b: who has priority */
    PR_SUMMON,  /* a: the object, b: YGO_SUMMON_* */
    PR_END
};
/* PR_TURN's step values */
enum { TS_START, TS_DRAW, TS_STANDBY, TS_MAIN1, TS_AFTER_MAIN1, TS_BATTLE_DONE, TS_MAIN2, TS_END, TS_CLEANUP };
/* PR_MAIN's answer to PR_TURN (frame b of PR_TURN) */
enum { MAIN_TO_BATTLE = 1, MAIN_END = 2 };

static struct YgoFlow *Top(struct YgoState *s) { return &s->frames[s->frameCount - 1]; }
static void Push(struct YgoState *s, unsigned proc, unsigned a, unsigned b)
{
    struct YgoFlow *f;
    if (s->frameCount >= YGO_FRAMES)
        return;
    f = &s->frames[s->frameCount++];
    f->proc = proc;
    f->step = 0;
    f->a = a;
    f->b = b;
}
static void Pop(struct YgoState *s)
{
    if (s->frameCount)
        s->frameCount--;
}

static void OpenWindow(struct YgoState *s, unsigned kind, unsigned obj, unsigned player)
{
    s->window = kind;
    s->windowObj = obj;
    s->windowPlayer = player;
    Push(s, PR_WINDOW, kind, 0);
}

/* One player's chance to activate something (in a window or a chain): 1 if they did */
static int OfferChain(struct YgoState *s, unsigned player)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    struct YgoChoice list[YGO_QUERY_MAX];
    unsigned n = Activations(s, player, 0, list, YGO_QUERY_MAX), i;
    if (!n)
        return 0;
    QueryInit(&q, YGO_Q_CHAIN, player, YGO_P_CHAIN, s->chainCount ? s->chain[s->chainCount - 1].card : 0);
    for (i = 0; i < n; i++)
        q.items[q.count++] = list[i];
    q.min = 0;
    q.max = 1;
    Ask(s, &q, &a);
    if (!a.count)
        return 0;
    Activate(s, player, list[a.picks[0]].obj, list[a.picks[0]].value, NO_OBJ, 0);
    return 1;
}

/* Triggers waiting go on a new chain, simultaneous ones in the right order (SEGOC): the turn player's mandatory,
 * the opponent's mandatory, the turn player's optional, the opponent's optional. Players order their own. */
static void ChainTriggers(struct YgoState *s)
{
    static const uint8_t order[4][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}}; /* {opponent?, optional?} */
    struct YgoPending list[YGO_PENDING_MAX];
    unsigned count = s->pendingCount, g, i;
    memcpy(list, s->pending, sizeof(list));
    s->pendingCount = 0;
    for (g = 0; g < 4; g++)
    {
        unsigned player = order[g][0] ? s->current ^ 1 : s->current;
        uint8_t mine[YGO_PENDING_MAX], objs[YGO_PENDING_MAX];
        unsigned m = 0;
        for (i = 0; i < count; i++)
            if (list[i].player == player && list[i].optional == order[g][1])
            {
                const struct YgoEffect *e;
                unsigned n;
                struct YgoCtx ctx;
                e = Ygo_Effects(s->objs[list[i].obj].card, &n) + list[i].effect;
                /* still possible? */
                if (e->can)
                {
                    ctx.s = s;
                    ctx.e = e;
                    ctx.obj = list[i].obj;
                    ctx.player = player;
                    ctx.eventObj = list[i].eventObj;
                    ctx.link = NULL;
                    if (!e->can(&ctx))
                        continue;
                }
                if (list[i].optional && !Ygo_AskYesNo(s, player, YGO_P_EFFECT_YESNO, s->objs[list[i].obj].card))
                    continue;
                mine[m] = i;
                objs[m] = list[i].obj;
                m++;
            }
        while (m)
        {
            uint8_t pick = objs[0];
            unsigned j, at = 0;
            if (m > 1)
            {
                Ygo_AskCards(s, player, YGO_P_ORDER, 0, objs, m, 1, 1, &pick);
                for (j = 0; j < m; j++)
                    if (objs[j] == pick)
                        at = j;
            }
            Activate(s, player, list[mine[at]].obj, list[mine[at]].effect, list[mine[at]].eventObj, 1);
            memmove(mine + at, mine + at + 1, m - at - 1);
            memmove(objs + at, objs + at + 1, m - at - 1);
            m--;
        }
    }
}

static void Idle(struct YgoState *s);
static void Battle(struct YgoState *s);
static void Damage(struct YgoState *s);

static void Window(struct YgoState *s)
{
    struct YgoFlow *f = Top(s);
    switch (f->step)
    {
    case 0:
        if (s->pendingCount)
        {
            Push(s, PR_CHAIN, 0, 0xFF);
            return;
        }
        f->step = 1;
        f->b = s->current;
        return;
    case 1:
        if (OfferChain(s, f->b))
        {
            /* the other player answers first */
            unsigned responder = f->b ^ 1;
            f->step = 0;
            Push(s, PR_CHAIN, 0, responder);
            return;
        }
        if (f->b == s->current)
        {
            f->b ^= 1;
            return;
        }
        Pop(s);
        return;
    }
}

static void Chain(struct YgoState *s)
{
    struct YgoFlow *f = Top(s);
    switch (f->step)
    {
    case 0:
        /* triggers waiting join (a new chain, or this one being built) */
        if (s->pendingCount)
        {
            ChainTriggers(s);
            if (!s->chainCount)
            {
                Pop(s);
                return;
            }
            f->b = s->chain[s->chainCount - 1].player ^ 1;
        }
        if (!s->chainCount)
        {
            Pop(s);
            return;
        }
        if (f->b == 0xFF)
            f->b = s->chain[s->chainCount - 1].player ^ 1;
        f->a = 0;
        f->step = 1;
        return;
    case 1:
        if (OfferChain(s, f->b))
            f->a = 0;
        else
            f->a++;
        f->b ^= 1;
        if (f->a >= 2)
            f->step = 2;
        return;
    case 2:
        ResolveTop(s);
        if (!s->chainCount)
            f->step = 3;
        return;
    case 3:
    {
        /* optional "when" triggers miss the timing unless theirs was the last thing that happened */
        unsigned i, k = 0;
        for (i = 0; i < s->pendingCount; i++)
            if (!(s->pending[i].optional && s->pending[i].when && s->pending[i].serial != s->serial))
                s->pending[k++] = s->pending[i];
        s->pendingCount = k;
        ExpireLasting(s, YGO_UNTIL_CHAIN);
        if (s->pendingCount)
        {
            f->step = 0;
            f->b = 0xFF;
            return;
        }
        Pop(s);
        return;
    }
    }
}

static void SummonSuccess(struct YgoState *s)
{
    struct YgoFlow *f = Top(s);
    unsigned o = f->a;
    switch (f->step)
    {
    case 0:
    {
        /* Solemn Judgment's window: only if someone could answer */
        struct YgoChoice list[4];
        f->step = 1;
        s->window = YW_SUMMONING;
        s->windowObj = o;
        s->windowPlayer = s->objs[o].controller;
        if (Activations(s, 0, 0, list, 1) || Activations(s, 1, 0, list, 1))
            Push(s, PR_WINDOW, YW_SUMMONING, 0);
        return;
    }
    case 1:
        s->window = YW_OPEN;
        f->step = 2;
        if (s->objs[o].location != YGO_LOC_MZONE)
        {
            /* negated */
            Pop(s);
            return;
        }
        Ygo_Event(s, YGO_EV_SUMMON, s->objs[o].controller, o, NO_OBJ, f->b, 0);
        if (f->b == YGO_SUMMON_FLIP)
        {
            Raise(s, YEV_FLIP_SUMMONED, o, NO_OBJ);
            Raise(s, YEV_FLIPPED, o, NO_OBJ);
        }
        else if (f->b == YGO_SUMMON_NORMAL || f->b == YGO_SUMMON_TRIBUTE)
            Raise(s, YEV_NORMAL_SUMMONED, o, NO_OBJ);
        else
            Raise(s, YEV_SPECIAL_SUMMONED, o, NO_OBJ);
        Raise(s, YEV_ANY_SUMMONED, NO_OBJ, o);
        if (f->b <= YGO_SUMMON_FLIP)
            Raise(s, YEV_ANY_NORMAL_SUMMONED, NO_OBJ, o);
        OpenWindow(s, f->b >= YGO_SUMMON_SPECIAL ? YW_SPECIAL_SUMMONED : YW_SUMMONED, o, s->objs[o].controller);
        return;
    case 2:
        s->window = YW_OPEN;
        Pop(s);
        return;
    }
}

static void Turn(struct YgoState *s)
{
    struct YgoFlow *f = Top(s);
    struct YgoPlayer *p;
    unsigned i;
    switch (f->step)
    {
    case TS_START:
        s->turn++;
        if (s->turn > 1)
            s->current ^= 1;
        p = &s->players[s->current];
        p->normalSummoned = 0;
        p->turns++;
        for (i = 0; i < s->objCount; i++)
        {
            s->objs[i].turnFlags = 0;
            s->objs[i].attacks = 0;
            s->objs[i].usedTurn = 0;
        }
        f->b = 0;
        Ygo_Event(s, YGO_EV_TURN, s->current, NO_OBJ, NO_OBJ, s->turn, 0);
        f->step = TS_DRAW;
        return;
    case TS_DRAW:
        s->phase = YGO_PHASE_DRAW;
        Ygo_Event(s, YGO_EV_PHASE, s->current, NO_OBJ, NO_OBJ, s->phase, 0);
        f->step = TS_STANDBY;
        if (s->turn > 1 && !Ygo_Rule(s, YR_NO_DRAW, s->current, 0))
        {
            Ygo_Draw(s, s->current, 1);
            Raise(s, YEV_DRAW_OWN, NO_OBJ, NO_OBJ);
        }
        OpenWindow(s, YW_DRAW, NO_OBJ, s->current);
        return;
    case TS_STANDBY:
        s->phase = YGO_PHASE_STANDBY;
        Ygo_Event(s, YGO_EV_PHASE, s->current, NO_OBJ, NO_OBJ, s->phase, 0);
        Raise(s, YEV_STANDBY_OWN | YEV_STANDBY_ANY, NO_OBJ, NO_OBJ);
        f->step = TS_MAIN1;
        OpenWindow(s, YW_STANDBY, NO_OBJ, s->current);
        return;
    case TS_MAIN1:
        s->window = YW_OPEN;
        s->phase = YGO_PHASE_MAIN1;
        Ygo_Event(s, YGO_EV_PHASE, s->current, NO_OBJ, NO_OBJ, s->phase, 0);
        f->step = TS_AFTER_MAIN1;
        Push(s, PR_MAIN, 0, 0);
        return;
    case TS_AFTER_MAIN1:
        if (f->b == MAIN_TO_BATTLE)
        {
            f->b = 0;
            f->step = TS_BATTLE_DONE;
            Push(s, PR_BATTLE, 0, 0);
            return;
        }
        f->step = TS_END;
        return;
    case TS_BATTLE_DONE:
        s->phase = YGO_PHASE_MAIN2;
        Ygo_Event(s, YGO_EV_PHASE, s->current, NO_OBJ, NO_OBJ, s->phase, 0);
        f->step = TS_END;
        Push(s, PR_MAIN, 0, 0);
        return;
    case TS_END:
        s->phase = YGO_PHASE_END;
        Ygo_Event(s, YGO_EV_PHASE, s->current, NO_OBJ, NO_OBJ, s->phase, 0);
        Raise(s, YEV_END_OWN | YEV_END_ANY, NO_OBJ, NO_OBJ);
        f->step = TS_CLEANUP;
        OpenWindow(s, YW_END, NO_OBJ, s->current);
        return;
    case TS_CLEANUP:
    {
        /* hand size: 6 at the end of the turn */
        p = &s->players[s->current];
        s->window = YW_OPEN;
        if (p->handCount > 6)
        {
            uint8_t picks[YGO_HAND_MAX], hand[YGO_HAND_MAX];
            unsigned n = p->handCount - 6;
            memcpy(hand, p->hand, p->handCount);
            Ygo_AskCards(s, s->current, YGO_P_DISCARD, 0, hand, p->handCount, n, n, picks);
            for (i = 0; i < n; i++)
                Ygo_Discard(s, picks[i], YGO_REASON_RULE);
        }
        for (i = 0; i < s->objCount; i++)
            s->objs[i].atkTurn = s->objs[i].defTurn = 0;
        ExpireLasting(s, s->turn);
        f->step = TS_START;
        return;
    }
    }
}

/* The Main Phase's actions for the turn player */
static unsigned IdleChoices(struct YgoState *s, struct YgoQuery *q)
{
    unsigned player = s->current, i, n;
    struct YgoPlayer *p = &s->players[player];
    struct YgoChoice acts[YGO_QUERY_MAX];
    QueryInit(q, YGO_Q_IDLE, player, YGO_P_NONE, 0);
    for (i = 0; i < p->handCount; i++)
    {
        unsigned o = p->hand[i];
        const struct YgoCard *c = Ygo_ObjCard(s, o);
        unsigned count, k;
        const struct YgoEffect *e = Ygo_Effects(c->id, &count);
        if (CanNormalSummon(s, player, o, 0))
            QueryAdd(q, YGO_ACT_SUMMON, o, 0, 0);
        if (CanNormalSummon(s, player, o, 1))
            QueryAdd(q, YGO_ACT_SET_MONSTER, o, 0, 0);
        for (k = 0; k < count; k++)
            if (e[k].type == YE_SUMMON_RULE && !Ygo_Rule(s, YR_NO_SUMMON, player, 0) &&
                !Ygo_Rule(s, YR_CANNOT_SPECIAL_SUMMON, player, 0))
            {
                struct YgoCtx ctx = {s, e + k, (uint8_t)o, (uint8_t)player, NO_OBJ, NULL};
                if (!e[k].can || e[k].can(&ctx))
                    QueryAdd(q, YGO_ACT_SPECIAL, o, k, 0);
            }
        if ((c->frame == YGO_SPELL || c->frame == YGO_TRAP) && c->property != YGO_PROP_FIELD &&
            Ygo_FreeZones(s, player, 1))
            QueryAdd(q, YGO_ACT_SET_SPELL, o, 0, 0);
    }
    for (i = 0; i < YGO_ZONES; i++)
    {
        unsigned m = p->monsters[i];
        struct YgoObj *b;
        if (!m)
            continue;
        b = &s->objs[m - 1];
        if (b->turnFlags & (YGO_TF_SUMMONED | YGO_TF_POS_CHANGED))
            continue;
        if (b->enteredTurn == s->turn)
            continue;
        if (!b->faceUp && !Ygo_Rule(s, YR_NO_SUMMON, player, 0))
            QueryAdd(q, YGO_ACT_FLIP_SUMMON, m - 1, 0, 0);
        else if (!Ygo_Rule(s, YR_CANNOT_CHANGE_POSITION, m - 1, 0))
            QueryAdd(q, YGO_ACT_POSITION, m - 1, 0, 0);
    }
    n = Activations(s, player, 1, acts, YGO_QUERY_MAX);
    for (i = 0; i < n; i++)
        QueryAdd(q, acts[i].kind, acts[i].obj, acts[i].value, 0);
    if (s->phase == YGO_PHASE_MAIN1 && s->turn > 1 && !Ygo_Rule(s, YR_NO_BATTLE_PHASE, player, 0))
        QueryAdd(q, YGO_ACT_TO_BATTLE, NO_OBJ, 0, 0);
    QueryAdd(q, YGO_ACT_END_TURN, NO_OBJ, 0, 0);
    return q->count;
}

static void Summon(struct YgoState *s, unsigned o, int set)
{
    unsigned player = s->current, need = TributesNeeded(Ygo_ObjCard(s, o)), i;
    uint8_t list[YGO_ZONES * 2], picks[YGO_ZONES * 2];
    if (need)
    {
        unsigned n = Ygo_Tributes(s, player, list);
        Ygo_AskCards(s, player, YGO_P_TRIBUTE, s->objs[o].card, list, n, need, need, picks);
        for (i = 0; i < need; i++)
            Ygo_SendToGrave(s, picks[i], YGO_REASON_TRIBUTE);
    }
    s->players[player].normalSummoned = 1;
    PlaceMonster(s, o, player, set ? 2 : 0);
    if (set)
    {
        s->objs[o].summonType = YGO_SUMMON_NONE;
        Ygo_Event(s, YGO_EV_SET, player, o, NO_OBJ, 0, 0);
        return;
    }
    s->objs[o].summonType = need ? YGO_SUMMON_TRIBUTE : YGO_SUMMON_NORMAL;
    s->players[player].summonedTurn = s->turn;
    Push(s, PR_SUMMON, o, s->objs[o].summonType);
}

int Ygo_CanNormalSummonNow(const struct YgoState *s, unsigned player, unsigned o, int set)
{
    uint8_t list[YGO_ZONES * 2];
    unsigned need;
    if (s->objs[o].location != YGO_LOC_HAND || s->objs[o].owner != player || !CanNormalSummonCard(s, o))
        return 0;
    if (!set && Ygo_Rule(s, YR_NO_SUMMON, player, 0))
        return 0;
    need = TributesNeeded(Ygo_ObjCard(s, o));
    return need ? Ygo_Tributes(s, player, list) >= need : Ygo_FreeZones(s, player, 0) > 0;
}

void Ygo_ExtraNormalSummon(struct YgoState *s, unsigned player, unsigned o, int set)
{
    unsigned keep = s->players[player].normalSummoned, current = s->current;
    /* Summon() works for the turn player and uses up their Normal Summon: borrow both */
    s->current = player;
    s->players[player].normalSummoned = 0;
    Summon(s, o, set);
    s->current = current;
    s->players[player].normalSummoned = keep;
}

static void Idle(struct YgoState *s)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    const struct YgoChoice *c;
    unsigned player = s->current;
    if (s->pendingCount)
    {
        Push(s, PR_CHAIN, 0, 0xFF);
        return;
    }
    IdleChoices(s, &q);
    Ask(s, &q, &a);
    c = &q.items[a.picks[0]];
    switch (c->kind)
    {
    case YGO_ACT_SUMMON:
        Summon(s, c->obj, 0);
        return;
    case YGO_ACT_SET_MONSTER:
        Summon(s, c->obj, 1);
        return;
    case YGO_ACT_FLIP_SUMMON:
        s->objs[c->obj].faceUp = 1;
        s->objs[c->obj].attackPos = 1;
        s->objs[c->obj].turnFlags |= YGO_TF_POS_CHANGED | YGO_TF_FLIPPED;
        s->objs[c->obj].summonType = YGO_SUMMON_FLIP;
        s->players[player].summonedTurn = s->turn;
        Ygo_Event(s, YGO_EV_FLIP, player, c->obj, NO_OBJ, 0, 0);
        Push(s, PR_SUMMON, c->obj, YGO_SUMMON_FLIP);
        return;
    case YGO_ACT_POSITION:
        s->objs[c->obj].turnFlags |= YGO_TF_POS_CHANGED;
        Ygo_ChangePosition(s, c->obj, s->objs[c->obj].attackPos ? 1 : 0);
        return;
    case YGO_ACT_ACTIVATE:
        Activate(s, player, c->obj, c->value, NO_OBJ, 0);
        Push(s, PR_CHAIN, 0, player ^ 1);
        return;
    case YGO_ACT_SET_SPELL:
    {
        int zone = FreeZone(s, player, 1);
        struct YgoObj *b = &s->objs[c->obj];
        if (zone < 0)
            return;
        Lift(s, c->obj);
        b->location = YGO_LOC_SZONE;
        b->seq = zone;
        b->faceUp = 0;
        b->controller = player;
        b->enteredTurn = s->turn;
        b->turnFlags |= YGO_TF_SET;
        s->players[player].spells[zone] = c->obj + 1;
        Ygo_Event(s, YGO_EV_SET, player, c->obj, NO_OBJ, 1, 0);
        return;
    }
    case YGO_ACT_SPECIAL:
    {
        unsigned n;
        const struct YgoEffect *e = Ygo_Effects(s->objs[c->obj].card, &n) + c->value;
        struct YgoCtx ctx = {s, e, c->obj, (uint8_t)player, NO_OBJ, NULL};
        if (e->cost)
            e->cost(&ctx);
        PlaceMonster(s, c->obj, player, 0);
        s->objs[c->obj].summonType = YGO_SUMMON_SPECIAL;
        s->objs[c->obj].properly = 1;
        s->players[player].summonedTurn = s->turn;
        Push(s, PR_SUMMON, c->obj, YGO_SUMMON_SPECIAL);
        return;
    }
    case YGO_ACT_TO_BATTLE:
        s->frames[s->frameCount - 2].b = MAIN_TO_BATTLE;
        Pop(s);
        return;
    case YGO_ACT_END_TURN:
        s->frames[s->frameCount - 2].b = MAIN_END;
        Pop(s);
        return;
    }
}

static int CanAttackWith(const struct YgoState *s, unsigned o)
{
    const struct YgoObj *b = &s->objs[o];
    if (b->location != YGO_LOC_MZONE || b->controller != s->current || !b->faceUp || !b->attackPos)
        return 0;
    if (b->attacks >= 1 + Ygo_Rule(s, YR_EXTRA_ATTACK, o, 0))
        return 0;
    if (Ygo_Rule(s, YR_CANNOT_ATTACK, o, 0) || Ygo_Rule(s, YR_NO_ATTACKS, s->current, 0))
        return 0;
    {
        int cost = Ygo_Rule(s, YR_ATTACK_COST, o, 0);
        if (cost && s->players[s->current].lp < cost)
            return 0;
    }
    return 1;
}

/* Who it can attack: the opponent's monsters (and NO_OBJ for directly) */
static unsigned AttackTargets(const struct YgoState *s, unsigned o, uint8_t *out)
{
    unsigned n = Ygo_List(s, s->current ^ 1, YGO_LOC_MZONE, out), i, k = 0, forced = 0;
    /* Ring of Magnetism and the like: only those, if any */
    for (i = 0; i < n; i++)
        forced += Ygo_Rule(s, YR_ATTACK_ONLY, out[i], 0) != 0;
    if (forced)
    {
        for (i = 0; i < n; i++)
            if (Ygo_Rule(s, YR_ATTACK_ONLY, out[i], 0))
                out[k++] = out[i];
        return k;
    }
    if ((n == 0 && !Ygo_Rule(s, YR_CANNOT_ATTACK_DIRECT, o, 0)) || Ygo_Rule(s, YR_DIRECT_ATTACK, o, 0))
        out[n++] = NO_OBJ;
    return n;
}

static void DeclareAttack(struct YgoState *s, unsigned o, int replay)
{
    struct YgoQuery q;
    struct YgoAnswer a;
    uint8_t targets[YGO_ZONES + 1];
    unsigned n = AttackTargets(s, o, targets), i, target;
    if (!n)
        return;
    QueryInit(&q, YGO_Q_CARDS, s->current, YGO_P_ATTACK_TARGET, s->objs[o].card);
    for (i = 0; i < n; i++)
        QueryAdd(&q, 0, targets[i], targets[i] == NO_OBJ, 0);
    if (replay)
        q.min = 0;
    if (n == 1 && !replay)
        target = targets[0];
    else
    {
        Ask(s, &q, &a);
        if (!a.count)
        {
            s->attacker = 0;
            return;
        }
        target = targets[a.picks[0]];
    }
    s->attacker = o + 1;
    s->attackTarget = target == NO_OBJ ? 0 : target + 1;
    s->directAttack = target == NO_OBJ;
    if (!replay)
    {
        int cost = Ygo_Rule(s, YR_ATTACK_COST, o, 0);
        s->objs[o].attacks++;
        s->objs[o].turnFlags |= YGO_TF_ATTACKED;
        if (cost)
        {
            s->players[s->current].lp -= cost;
            Ygo_Event(s, YGO_EV_DAMAGE, s->current, NO_OBJ, NO_OBJ, cost, 2);
        }
    }
    Ygo_Event(s, YGO_EV_ATTACK, s->current, o, target, replay, 0);
    if (!replay)
        Raise(s, YEV_DECLARED_ATTACK, o, target);
    if (target != NO_OBJ)
        Raise(s, YEV_ATTACKED, target, o);
}

static void Battle(struct YgoState *s)
{
    struct YgoFlow *f = Top(s);
    switch (f->step)
    {
    case 0:
        s->endBattle = 0;
        s->phase = YGO_PHASE_BATTLE;
        Ygo_Event(s, YGO_EV_PHASE, s->current, NO_OBJ, NO_OBJ, s->phase, 0);
        f->step = 1;
        OpenWindow(s, YW_BATTLE_START, NO_OBJ, s->current);
        return;
    case 1:
    {
        struct YgoQuery q;
        struct YgoAnswer a;
        struct YgoChoice acts[YGO_QUERY_MAX];
        unsigned i, n;
        s->window = YW_OPEN;
        if (s->pendingCount)
        {
            Push(s, PR_CHAIN, 0, 0xFF);
            return;
        }
        if (s->endBattle)
        {
            s->endBattle = 0;
            s->attacker = s->attackTarget = 0;
            Pop(s);
            return;
        }
        QueryInit(&q, YGO_Q_BATTLE, s->current, YGO_P_NONE, 0);
        for (i = 0; i < YGO_ZONES; i++)
        {
            unsigned m = s->players[s->current].monsters[i];
            uint8_t t[YGO_ZONES + 1];
            if (m && CanAttackWith(s, m - 1) && AttackTargets(s, m - 1, t))
                QueryAdd(&q, YGO_ACT_ATTACK, m - 1, 0, 0);
        }
        n = Activations(s, s->current, 0, acts, YGO_QUERY_MAX);
        for (i = 0; i < n; i++)
            QueryAdd(&q, acts[i].kind, acts[i].obj, acts[i].value, 0);
        QueryAdd(&q, YGO_ACT_TO_MAIN2, NO_OBJ, 0, 0);
        Ask(s, &q, &a);
        switch (q.items[a.picks[0]].kind)
        {
        case YGO_ACT_ATTACK:
            s->attackNegated = 0;
            DeclareAttack(s, q.items[a.picks[0]].obj, 0);
            if (!s->attacker)
                return;
            f->step = 2;
            OpenWindow(s, YW_ATTACK, s->attacker - 1, s->current);
            return;
        case YGO_ACT_ACTIVATE:
            Activate(s, s->current, q.items[a.picks[0]].obj, q.items[a.picks[0]].value, NO_OBJ, 0);
            Push(s, PR_CHAIN, 0, s->current ^ 1);
            return;
        default:
            s->attacker = s->attackTarget = 0;
            Pop(s);
            return;
        }
    }
    case 2:
    {
        unsigned o = s->attacker ? s->attacker - 1 : NO_OBJ;
        s->window = YW_OPEN;
        f->step = 1;
        if (o == NO_OBJ || s->attackNegated || s->objs[o].location != YGO_LOC_MZONE || s->objs[o].controller != s->current ||
            !s->objs[o].faceUp || !s->objs[o].attackPos)
        {
            s->attacker = s->attackTarget = 0;
            return;
        }
        /* the target is gone (or a monster appeared before a direct attack): a replay */
        if ((s->attackTarget && (s->objs[s->attackTarget - 1].location != YGO_LOC_MZONE ||
                                 s->objs[s->attackTarget - 1].controller == s->current)) ||
            (s->directAttack && Ygo_MonsterCount(s, s->current ^ 1) && !Ygo_Rule(s, YR_DIRECT_ATTACK, o, 0)))
        {
            DeclareAttack(s, o, 1);
            if (!s->attacker)
                return;
        }
        Push(s, PR_DAMAGE, 0, 0);
        return;
    }
    }
}

static void Damage(struct YgoState *s)
{
    struct YgoFlow *f = Top(s);
    unsigned a = s->attacker ? s->attacker - 1 : NO_OBJ, t = s->attackTarget ? s->attackTarget - 1 : NO_OBJ;
    switch (f->step)
    {
    case 0:
        s->damageStep = 1;
        s->flippedByBattle = 0;
        s->battleDestroyed[0] = s->battleDestroyed[1] = 0;
        if (t != NO_OBJ && !s->objs[t].faceUp)
        {
            Ygo_FlipFaceUp(s, t, 1);
            s->flippedByBattle = t + 1;
        }
        f->step = 1;
        OpenWindow(s, YW_DAMAGE_STEP, a, s->current);
        return;
    case 1:
    {
        int atk, other, damage = 0;
        unsigned hurt = 0;
        s->window = YW_OPEN;
        f->step = 2;
        if (a == NO_OBJ || s->objs[a].location != YGO_LOC_MZONE || s->attackNegated)
            return;
        if (t != NO_OBJ && s->objs[t].location != YGO_LOC_MZONE)
            return;
        atk = YgoDuel_Atk(s, a);
        if (t == NO_OBJ)
        {
            damage = atk;
            hurt = s->current ^ 1;
            if (!Ygo_Rule(s, YR_NO_BATTLE_DAMAGE, hurt, a))
                Ygo_Damage(s, hurt, damage, 1);
            else
                damage = 0;
            Ygo_Event(s, YGO_EV_BATTLE, s->current, a, NO_OBJ, damage, hurt);
            if (damage)
            {
                Raise(s, YEV_BATTLE_DAMAGE, a, NO_OBJ);
                Raise(s, YEV_ANY_BATTLE_DAMAGE, NO_OBJ, a);
            }
        }
        else if (s->objs[t].attackPos)
        {
            other = YgoDuel_Atk(s, t);
            if (atk > other)
            {
                damage = atk - other;
                hurt = s->current ^ 1;
                if (!Ygo_Rule(s, YR_INDESTRUCTIBLE_BATTLE, t, a))
                    s->battleDestroyed[1] = t + 1;
            }
            else if (atk < other)
            {
                damage = other - atk;
                hurt = s->current;
                if (!Ygo_Rule(s, YR_INDESTRUCTIBLE_BATTLE, a, t))
                    s->battleDestroyed[0] = a + 1;
            }
            else if (atk > 0)
            {
                if (!Ygo_Rule(s, YR_INDESTRUCTIBLE_BATTLE, t, a))
                    s->battleDestroyed[1] = t + 1;
                if (!Ygo_Rule(s, YR_INDESTRUCTIBLE_BATTLE, a, t))
                    s->battleDestroyed[0] = a + 1;
            }
            if (damage && Ygo_Rule(s, YR_NO_BATTLE_DAMAGE, hurt, hurt == s->current ? t : a))
                damage = 0;
            Ygo_Damage(s, hurt, damage, 1);
            Ygo_Event(s, YGO_EV_BATTLE, s->current, a, t, damage, hurt);
            if (damage)
            {
                Raise(s, YEV_BATTLE_DAMAGE, hurt == s->current ? t : a, NO_OBJ);
                Raise(s, YEV_ANY_BATTLE_DAMAGE, NO_OBJ, hurt == s->current ? t : a);
            }
        }
        else
        {
            other = YgoDuel_Def(s, t);
            if (atk > other)
            {
                if (!Ygo_Rule(s, YR_INDESTRUCTIBLE_BATTLE, t, a))
                    s->battleDestroyed[1] = t + 1;
                if (Ygo_Rule(s, YR_PIERCE, a, 0))
                {
                    damage = atk - other;
                    hurt = s->current ^ 1;
                }
            }
            else if (atk < other)
            {
                damage = other - atk;
                hurt = s->current;
            }
            if (damage && Ygo_Rule(s, YR_NO_BATTLE_DAMAGE, hurt, hurt == s->current ? t : a))
                damage = 0;
            Ygo_Damage(s, hurt, damage, 1);
            Ygo_Event(s, YGO_EV_BATTLE, s->current, a, t, damage, hurt);
            if (damage && hurt != s->current)
            {
                Raise(s, YEV_BATTLE_DAMAGE, a, NO_OBJ);
                Raise(s, YEV_ANY_BATTLE_DAMAGE, NO_OBJ, a);
            }
        }
        /* Relinquished: battle damage its controller takes hits the opponent as well */
        if (damage && t != NO_OBJ)
        {
            unsigned mine = hurt == s->current ? a : t;
            if (Ygo_Rule(s, YR_MIRROR_DAMAGE, mine, 0))
                Ygo_Damage(s, hurt ^ 1, damage, 0);
        }
        if (t != NO_OBJ)
        {
            Raise(s, YEV_BATTLED, a, t);
            Raise(s, YEV_BATTLED, t, a);
        }
        if (s->flippedByBattle)
            Raise(s, YEV_FLIPPED, s->flippedByBattle - 1, NO_OBJ);
        OpenWindow(s, YW_AFTER_DAMAGE, a, s->current);
        return;
    }
    case 2:
    {
        unsigned k;
        s->window = YW_OPEN;
        for (k = 0; k < 2; k++)
        {
            unsigned d = s->battleDestroyed[k];
            if (d && s->objs[d - 1].location == YGO_LOC_MZONE)
            {
                int sub = Ygo_Rule(s, YR_BATTLE_SUBSTITUTE, d - 1, 0);
                if (sub > 0 && Ygo_OnField(s, sub - 1))
                {
                    /* something else is destroyed instead */
                    Ygo_Destroy(s, sub - 1, YGO_REASON_EFFECT);
                    continue;
                }
                unsigned winner = k == 0 ? t : a;
                Ygo_Event(s, YGO_EV_DESTROY, s->objs[d - 1].controller, d - 1, winner, 0, YGO_REASON_BATTLE);
                Ygo_SendToGrave(s, d - 1, YGO_REASON_BATTLE | YGO_REASON_DESTROY);
                Raise(s, YEV_DESTROYED_BATTLE, d - 1, winner);
                Raise(s, YEV_ANY_DESTROYED, NO_OBJ, d - 1);
                if (winner != NO_OBJ)
                    Raise(s, YEV_DESTROYED_OPP_BATTLE, winner, d - 1);
            }
        }
        f->step = 3;
        OpenWindow(s, YW_END_DAMAGE, a, s->current);
        return;
    }
    case 3:
        s->window = YW_OPEN;
        s->damageStep = 0;
        ExpireLasting(s, YGO_UNTIL_BATTLE);
        s->attacker = s->attackTarget = 0;
        s->directAttack = 0;
        s->battleDestroyed[0] = s->battleDestroyed[1] = 0;
        Pop(s);
        return;
    }
}

static void RunUnit(struct YgoState *s)
{
    if (!s->frameCount)
        Push(s, PR_TURN, 0, 0);
    switch (Top(s)->proc)
    {
    case PR_TURN: Turn(s); break;
    case PR_MAIN: Idle(s); break;
    case PR_BATTLE: Battle(s); break;
    case PR_DAMAGE: Damage(s); break;
    case PR_WINDOW: Window(s); break;
    case PR_CHAIN: Chain(s); break;
    case PR_SUMMON: SummonSuccess(s); break;
    default: Pop(s); break;
    }
    CheckState(s);
}

/* Runs units until one asks a question (or the duel ends). 0 when an answer was refused. */
static int Run(struct YgoDuel *d)
{
    static struct YgoState work;   /* big: kept off the stack */
    unsigned guard = 0;
    for (;;)
    {
        int r;
        if (d->s.over)
        {
            d->query.type = YGO_Q_NONE;
            d->answerCount = 0;
            return 1;
        }
        if (++guard > 100000)
        {
            snprintf(d->error, sizeof(d->error), "the duel stopped moving");
            d->s.over = 1;
            d->query.type = YGO_Q_NONE;
            return 0;
        }
        work = d->s;
        R.duel = d;
        R.used = 0;
        R.active = 1;
        r = setjmp(R.jump);
        if (r == 0)
        {
            RunUnit(&work);
            R.active = 0;
            d->s = work;
            d->answerCount = 0;
            continue;
        }
        R.active = 0;
        if (r == RUN_BAD)
        {
            if (d->answerCount)
                d->answerCount--;
            return 0;
        }
        return 1;   /* asked: d->query */
    }
}

/* ---------------------------------------------------------------------------------------------------------------
 * The outside */
static void LoadDeck(struct YgoState *s, unsigned player, const uint16_t *main, const uint16_t *extra)
{
    struct YgoPlayer *p = &s->players[player];
    unsigned i;
    for (i = 0; i < YGO_DECK_MAX && main[i] && s->objCount < YGO_OBJS; i++)
    {
        struct YgoObj *b = &s->objs[s->objCount];
        memset(b, 0, sizeof(*b));
        b->card = main[i];
        b->owner = b->controller = player;
        b->location = YGO_LOC_DECK;
        p->deck[p->deckCount++] = s->objCount++;
    }
    for (i = 0; extra && i < YGO_FUSION_MAX && extra[i] && s->objCount < YGO_OBJS; i++)
    {
        struct YgoObj *b = &s->objs[s->objCount];
        memset(b, 0, sizeof(*b));
        b->card = extra[i];
        b->owner = b->controller = player;
        b->location = YGO_LOC_EXTRA;
        p->extra[p->extraCount++] = s->objCount++;
    }
}

int YgoDuel_Init(struct YgoDuel *d, const uint16_t main0[YGO_DECK_MAX], const uint16_t extra0[YGO_FUSION_MAX],
                 const uint16_t main1[YGO_DECK_MAX], const uint16_t extra1[YGO_FUSION_MAX], uint32_t seed,
                 unsigned first)
{
    struct YgoState *s = &d->s;
    unsigned p;
    memset(d, 0, sizeof(*d));
    s->rng = seed ? seed : 1;
    s->first = s->current = first & 1;
    LoadDeck(s, 0, main0, extra0);
    LoadDeck(s, 1, main1, extra1);
    for (p = 0; p < 2; p++)
    {
        s->players[p].lp = YGO_LP;
        if (s->players[p].deckCount < YGO_DECK_MIN)
        {
            snprintf(d->error, sizeof(d->error), "deck %u has %u cards", p, s->players[p].deckCount);
            return 0;
        }
    }
    for (p = 0; p < 2; p++)
    {
        Ygo_ShuffleDeck(s, p);
        Ygo_Draw(s, p, 5);
    }
    s->window = YW_OPEN;
    return Run(d);
}

int YgoDuel_Answer(struct YgoDuel *d, const struct YgoAnswer *answer)
{
    if (d->s.over || d->query.type == YGO_Q_NONE)
    {
        snprintf(d->error, sizeof(d->error), "the duel is over");
        return 0;
    }
    if (!ValidAnswer(&d->query, answer, d->error, sizeof(d->error)))
        return 0;
    if (d->answerCount >= YGO_ANSWERS_MAX)
    {
        snprintf(d->error, sizeof(d->error), "too many answers in one step");
        return 0;
    }
    d->answers[d->answerCount++] = *answer;
    d->error[0] = 0;
    return Run(d);
}

int YgoDuel_Refresh(struct YgoDuel *d)
{
    d->answerCount = 0;
    d->error[0] = 0;
    return Run(d);
}

int YgoDuel_Concede(struct YgoDuel *d, unsigned player)
{
    if (d->s.over)
        return 0;
    Ygo_Win(&d->s, player ^ 1, YGO_WIN_CONCEDE);
    d->query.type = YGO_Q_NONE;
    d->answerCount = 0;
    return 1;
}

uint32_t YgoDuel_Hash(const struct YgoDuel *d)
{
    const uint8_t *p = (const uint8_t *)&d->s;
    size_t n = offsetof(struct YgoState, eventSeq), i;
    uint32_t h = 2166136261u;
    for (i = 0; i < n; i++)
        h = (h ^ p[i]) * 16777619u;
    return h;
}

int YgoDuel_Conserved(const struct YgoDuel *d)
{
    const struct YgoState *s = &d->s;
    unsigned o, p, i;
    uint8_t seen[YGO_OBJS];
    memset(seen, 0, sizeof(seen));
    for (p = 0; p < 2; p++)
    {
        const struct YgoPlayer *pl = &s->players[p];
        const uint8_t *piles[5] = {pl->deck, pl->hand, pl->grave, pl->banished, pl->extra};
        const uint8_t counts[5] = {pl->deckCount, pl->handCount, pl->graveCount, pl->banishedCount, pl->extraCount};
        unsigned k;
        for (k = 0; k < 5; k++)
            for (i = 0; i < counts[k]; i++)
                seen[piles[k][i]]++;
        for (i = 0; i < YGO_ZONES; i++)
            if (pl->monsters[i])
                seen[pl->monsters[i] - 1]++;
        for (i = 0; i <= YGO_FIELD_ZONE; i++)
            if (pl->spells[i])
                seen[pl->spells[i] - 1]++;
    }
    for (o = 0; o < s->objCount; o++)
    {
        const struct YgoCard *c = Ygo_Card(s->objs[o].card);
        int token = c && c->frame == YGO_TOKEN;
        if (seen[o] > 1 || (!token && seen[o] != 1) || (token && seen[o] != (s->objs[o].location != YGO_LOC_NONE)))
            return 0;
    }
    return 1;
}
