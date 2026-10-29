/* Yu-Gi-Oh! opponents: every question answered from what's on the table, the way a careful player reads it. The
 * engine (EDOPro) offers only what's legal; the AI weighs those choices: removal and draws first, the strongest
 * monster it can Summon (Set when it would lose a battle), Traps Set, attacks that win their battle or go direct,
 * Traps sprung when the opponent acts. It never looks at cards it couldn't see: the opponent's hand, face-down cards
 * and Decks are unknown to it (only how many there are). */
#include <string.h>
#include "ygo_internal.h"

static const struct YgoCard *Card(const struct YgoState *s, unsigned o)
{
    return o < s->objCount ? Ygo_Card(s->objs[o].card) : NULL;
}
/* What the AI may know about an object: its card when it's face up, its own, or public */
static const struct YgoCard *Seen(const struct YgoState *s, unsigned me, unsigned o)
{
    const struct YgoObj *ob;
    if (o >= s->objCount)
        return NULL;
    ob = &s->objs[o];
    switch (ob->location)
    {
    case YGO_LOC_HAND:
    case YGO_LOC_DECK:
    case YGO_LOC_EXTRA:
        return ob->controller == me ? Card(s, o) : NULL;
    case YGO_LOC_MZONE:
    case YGO_LOC_SZONE:
        return ob->faceUp || ob->controller == me ? Card(s, o) : NULL;
    default:
        return Card(s, o);
    }
}
static int Atk(const struct YgoState *s, unsigned o) { return YgoDuel_Atk(s, o) < 0 ? 0 : YgoDuel_Atk(s, o); }
static int Def(const struct YgoState *s, unsigned o) { return YgoDuel_Def(s, o) < 0 ? 0 : YgoDuel_Def(s, o); }

/* A card's worth to its owner */
static int Value(const struct YgoState *s, unsigned me, unsigned o)
{
    const struct YgoCard *c = Seen(s, me, o);
    const struct YgoObj *ob = &s->objs[o];
    if (o >= s->objCount)
        return 0;
    if (!c)
        return ob->location == YGO_LOC_MZONE ? 1500 : 1200;   /* unknown: a fair guess */
    if (c->frame == YGO_TOKEN)
        return 200;
    if (ob->location == YGO_LOC_MZONE)
        return 300 + (ob->attackPos ? Atk(s, o) : (Atk(s, o) + Def(s, o)) / 2);
    if (Ygo_IsMonster(c))
        return 200 + (c->atk == YGO_STAT_UNKNOWN ? 1000 : c->atk) + c->level * 40;
    return 1100;   /* Spells and Traps */
}
/* The strongest face-up ATK the other side has, and how many monsters */
static int TheirBest(const struct YgoState *s, unsigned me, int *count)
{
    const struct YgoPlayer *pl = &s->players[me ^ 1];
    int best = 0, n = 0;
    unsigned i;
    for (i = 0; i < YGO_ZONES; i++)
        if (pl->monsters[i])
        {
            unsigned o = pl->monsters[i] - 1;
            n++;
            if (s->objs[o].faceUp && s->objs[o].attackPos && Atk(s, o) > best)
                best = Atk(s, o);
            if (!s->objs[o].faceUp && best < 1500)
                best = 1500;
        }
    if (count)
        *count = n;
    return best;
}
static int MyBest(const struct YgoState *s, unsigned me)
{
    const struct YgoPlayer *pl = &s->players[me];
    int best = 0;
    unsigned i;
    for (i = 0; i < YGO_ZONES; i++)
        if (pl->monsters[i] && s->objs[pl->monsters[i] - 1].faceUp && Atk(s, pl->monsters[i] - 1) > best)
            best = Atk(s, pl->monsters[i] - 1);
    return best;
}
static unsigned CardsOn(const struct YgoState *s, unsigned p)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_ZONES; i++)
        n += s->players[p].monsters[i] != 0;
    for (i = 0; i <= YGO_FIELD_ZONE; i++)
        n += s->players[p].spells[i] != 0;
    return n;
}
/* The best target an attacker has: object, NO_OBJ for a direct attack, or -1 when no attack is worth it */
static int BestTarget(const struct YgoState *s, unsigned me, unsigned attacker, int direct)
{
    const struct YgoPlayer *pl = &s->players[me ^ 1];
    int atk = Atk(s, attacker), best = -1, bestScore = 0;
    unsigned i, monsters = 0;
    for (i = 0; i < YGO_ZONES; i++)
    {
        unsigned o;
        int stat, score;
        if (!pl->monsters[i])
            continue;
        o = pl->monsters[i] - 1;
        monsters++;
        if (!s->objs[o].faceUp)
            score = atk >= 1600 ? 1000 : -1;   /* a face-down one: worth a try with a strong monster */
        else
        {
            stat = s->objs[o].attackPos ? Atk(s, o) : Def(s, o);
            if (atk > stat)
                score = 2000 + stat;
            else if (atk == stat && s->objs[o].attackPos && atk > 0)
                score = 500;   /* both destroyed */
            else
                score = -1;
        }
        if (score > bestScore)
            bestScore = score, best = (int)o;
    }
    if ((direct || !monsters) && atk > 0 && bestScore < 3000)
        return NO_OBJ;
    return best;
}

static void Pick(struct YgoAnswer *a, unsigned item)
{
    memset(a, 0, sizeof(*a));
    a->count = 1;
    a->picks[0] = (uint8_t)item;
}
/* (no endless loops: a card whose effect was used three times this turn rests; read from the events, so the same
 * duel always gets the same answers) */
static int UsedOften(const struct YgoState *s, const struct YgoChoice *c, int note)
{
    uint32_t seq;
    unsigned n = 0;
    (void)note;
    if (c->obj == NO_OBJ)
        return 0;
    for (seq = s->eventSeq; seq > 0 && s->eventSeq - seq < YGO_EVENT_RING; seq--)
    {
        const struct YgoEvent *e = YgoDuel_GetEvent(s, seq - 1);
        if (!e || e->type == YGO_EV_TURN)
            break;
        n += e->type == YGO_EV_ACTIVATE && e->obj == c->obj;
    }
    return n >= 3;
}

static void Idle(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned me = q->player, i;
    int theirMonsters, theirBest = TheirBest(s, me, &theirMonsters), myBest = MyBest(s, me);
    int best = -1, bestScore = 0, moveOn = -1, toBattle = -1;
    for (i = 0; i < q->count; i++)
    {
        const struct YgoChoice *c = &q->items[i];
        const struct YgoCard *card = Card(s, c->obj);
        int score = 0;
        switch (c->kind)
        {
        case YGO_ACT_ACTIVATE:
            if (UsedOften(s, c, 0))
                break;
            score = 900;
            /* a Spell that clears the field hurts its user when they have more there */
            if (card && card->frame == YGO_SPELL && CardsOn(s, me) > CardsOn(s, me ^ 1) + 1)
                score = 300;
            break;
        case YGO_ACT_SPECIAL:
            score = 800;
            break;
        case YGO_ACT_SUMMON:
            if (!card)
                break;
            /* the strongest that holds its own; a Tribute only for one worth more than what it costs */
            if (card->level >= 5 && (card->atk == YGO_STAT_UNKNOWN ? 1500 : card->atk) <= myBest + 400)
                break;
            score = 500 + (card->atk == YGO_STAT_UNKNOWN ? 1000 : card->atk) / 10;
            if ((card->abilities & YGO_ABILITY_FLIP) ||
                (card->atk != YGO_STAT_UNKNOWN && card->atk < theirBest && card->def > card->atk))
                score = 0;   /* (Set it instead) */
            break;
        case YGO_ACT_SET_MONSTER:
            if (!card)
                break;
            score = 400 + (card->def == YGO_STAT_UNKNOWN ? 1000 : card->def) / 20;
            if (card->level >= 5)
                score = 0;
            break;
        case YGO_ACT_SET_SPELL:
            if (!card)
                break;
            if (card->frame == YGO_TRAP || card->property == YGO_PROP_QUICK)
                score = 300;
            break;
        case YGO_ACT_FLIP_SUMMON:
            if (card && ((card->abilities & YGO_ABILITY_FLIP) || card->atk > theirBest))
                score = 350;
            break;
        case YGO_ACT_POSITION:
            if (c->obj < s->objCount)
            {
                const struct YgoObj *ob = &s->objs[c->obj];
                if (ob->attackPos && Atk(s, c->obj) < theirBest && Def(s, c->obj) > Atk(s, c->obj))
                    score = 200;
                else if (!ob->attackPos && ob->faceUp && Atk(s, c->obj) > theirBest && Atk(s, c->obj) >= Def(s, c->obj))
                    score = 200;
            }
            break;
        case YGO_ACT_TO_BATTLE:
            toBattle = (int)i;
            break;
        case YGO_ACT_END_TURN:
        case YGO_ACT_TO_MAIN2:
            if (moveOn < 0 || c->kind == YGO_ACT_TO_MAIN2)
                moveOn = (int)i;
            break;
        default:
            break;
        }
        if (score > bestScore)
            bestScore = score, best = (int)i;
    }
    if (best >= 0)
    {
        if (q->items[best].kind == YGO_ACT_ACTIVATE)
            UsedOften(s, &q->items[best], 1);
        Pick(out, (unsigned)best);
        return;
    }
    /* nothing to play: battle when a monster can do something there */
    if (toBattle >= 0)
    {
        const struct YgoPlayer *pl = &s->players[me];
        for (i = 0; i < YGO_ZONES; i++)
            if (pl->monsters[i] && s->objs[pl->monsters[i] - 1].faceUp && s->objs[pl->monsters[i] - 1].attackPos &&
                BestTarget(s, me, pl->monsters[i] - 1, 0) != -1)
            {
                Pick(out, (unsigned)toBattle);
                return;
            }
    }
    Pick(out, moveOn >= 0 ? (unsigned)moveOn : q->count - 1);
}
static void Battle(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned me = q->player, i;
    int best = -1, bestAtk = -1, moveOn = -1;
    for (i = 0; i < q->count; i++)
    {
        const struct YgoChoice *c = &q->items[i];
        if (c->kind == YGO_ACT_ATTACK && c->obj < s->objCount)
        {
            if (BestTarget(s, me, c->obj, c->flags & 1) == -1)
                continue;
            if (Atk(s, c->obj) > bestAtk)
                bestAtk = Atk(s, c->obj), best = (int)i;
        }
        else if (c->kind == YGO_ACT_TO_MAIN2 || (c->kind == YGO_ACT_END_TURN && moveOn < 0))
            moveOn = (int)i;
    }
    Pick(out, best >= 0 ? (unsigned)best : moveOn >= 0 ? (unsigned)moveOn : q->count - 1);
}
/* Should it chain now? When the opponent is acting: an effect in the chain, an attack on it, a Summon */
static int Pressure(const struct YgoDuel *d, unsigned me)
{
    const struct YgoState *s = &d->s;
    uint32_t seq;
    if (s->chainCount && s->chain[s->chainCount - 1].player != me)
        return 1;
    if (s->attacker && s->objs[s->attacker - 1].controller != me)
        return 1;
    for (seq = s->eventSeq; seq > 0 && s->eventSeq - seq < 6; seq--)
    {
        const struct YgoEvent *e = YgoDuel_GetEvent(s, seq - 1);
        if (!e)
            break;
        if (e->type == YGO_EV_SUMMON && e->player != me)
            return 1;
        if (e->type == YGO_EV_PHASE || e->type == YGO_EV_TURN)
            break;
    }
    return 0;
}
static void Chain(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned me = q->player, i;
    memset(out, 0, sizeof(*out));
    if (q->min)
    {
        Pick(out, 0);
        return;
    }
    for (i = 0; i < q->count; i++)
    {
        const struct YgoChoice *c = &q->items[i];
        const struct YgoCard *card = Card(s, c->obj);
        if (UsedOften(s, c, 0))
            continue;
        /* monsters' effects and Spells / Traps that answer the opponent; never a chain to itself */
        if (Pressure(d, me) || (card && card->frame != YGO_TRAP && card->property != YGO_PROP_QUICK &&
                                s->chainCount == 0 && !YgoDuel_QuietChance(d)))
        {
            UsedOften(s, c, 1);
            Pick(out, i);
            return;
        }
    }
}
static void Cards(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned me = q->player, i, j, n = q->count, k;
    uint8_t order[YGO_QUERY_MAX];
    int value[YGO_QUERY_MAX];
    memset(out, 0, sizeof(*out));
    if (q->prompt == YGO_P_ATTACK_TARGET && s->attacker)
    {
        int t = BestTarget(s, me, s->attacker - 1, 1);
        for (i = 0; i < n; i++)
            if (q->items[i].obj == (t < 0 ? NO_OBJ : (unsigned)t))
            {
                Pick(out, i);
                return;
            }
    }
    for (i = 0; i < n; i++)
    {
        unsigned o = q->items[i].obj;
        const struct YgoCard *c = Ygo_Card(q->items[i].card);
        int v = o == NO_OBJ ? (c ? 1000 + (c->atk == YGO_STAT_UNKNOWN ? 0 : c->atk) : 2500) : Value(s, me, o);
        int mine = o != NO_OBJ && o < s->objCount && s->objs[o].controller == me;
        switch (q->prompt)
        {
        case YGO_P_TRIBUTE:
        case YGO_P_DISCARD:
        case YGO_P_MATERIAL:
            v = -v;
            break;
        case YGO_P_ADD_TO_HAND:
        case YGO_P_SUMMON:
        case YGO_P_FUSION:
        case YGO_P_RITUAL:
        case YGO_P_EQUIP:
            if (!mine && o != NO_OBJ && s->objs[o].controller != me && q->prompt == YGO_P_EQUIP)
                v = -v;
            break;
        default:
            if (mine)
                v = -v;
            break;
        }
        if (q->items[i].flags & YGO_IF_PICKED)
            v -= 100000;   /* (already picked: picking it again takes it back) */
        value[i] = v;
        order[i] = (uint8_t)i;
    }
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++)
            if (value[order[j]] > value[order[i]])
            {
                uint8_t t = order[i];
                order[i] = order[j];
                order[j] = t;
            }
    /* as many as the engine wants (a question it can be backed out of comes from a choice the AI made: go on) */
    k = q->cancelable ? q->sum : q->min;
    if (!k && q->max && n && value[order[0]] > 0 && !(q->items[order[0]].flags & YGO_IF_PICKED) && !q->cancelable)
        k = 1;
    if (k > n)
        k = n;
    if (q->prompt == YGO_P_TRIBUTE && q->max)
    {
        /* Tributes: the cheapest that are worth enough */
        unsigned worth = 0, need = q->cancelable ? q->sum : q->min;
        for (k = 0; k < n && worth < need; k++)
            worth += q->items[order[k]].param;
    }
    if (k > YGO_PICKS_MAX)
        k = YGO_PICKS_MAX;
    out->count = (uint8_t)k;
    for (i = 0; i < k; i++)
        out->picks[i] = order[i];
}
static void Sum(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    unsigned picks[YGO_QUERY_MAX], n = 0, i;
    uint32_t mask, bestMask = 0;
    int found = 0, bestCost = 0x7fffffff;
    for (i = 0; i < q->count; i++)
        if (!(q->items[i].flags & YGO_IF_MUST))
            picks[n++] = i;
    memset(out, 0, sizeof(*out));
    if (n > 18)
        n = 18;
    for (mask = 1; mask < (1u << n); mask++)
    {
        struct YgoAnswer a;
        int cost = 0;
        memset(&a, 0, sizeof(a));
        for (i = 0; i < n; i++)
            if (mask >> i & 1)
            {
                unsigned o = q->items[picks[i]].obj;
                a.picks[a.count++] = (uint8_t)picks[i];
                cost += o < d->s.objCount ? Value(&d->s, q->player, o) : 1000;
            }
        if (cost < bestCost && YgoDuel_Check(d, &a))
            bestCost = cost, bestMask = mask, found = 1;
    }
    if (!found)
        return;
    for (i = 0; i < n; i++)
        if (bestMask >> i & 1)
            out->picks[out->count++] = (uint8_t)picks[i];
}

void YgoDuel_AiAnswer(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned i;
    memset(out, 0, sizeof(*out));
    switch (q->type)
    {
    case YGO_Q_IDLE:
        Idle(d, out);
        return;
    case YGO_Q_BATTLE:
        Battle(d, out);
        return;
    case YGO_Q_CHAIN:
        Chain(d, out);
        return;
    case YGO_Q_YESNO:
        Pick(out, 1);
        return;
    case YGO_Q_CARDS:
        Cards(d, out);
        return;
    case YGO_Q_SUM:
        Sum(d, out);
        return;
    case YGO_Q_POSITION:
    {
        const struct YgoCard *c = Ygo_Card(q->card);
        int theirBest = TheirBest(s, q->player, NULL);
        unsigned want = c && c->atk != YGO_STAT_UNKNOWN && (c->atk >= theirBest || c->atk >= c->def) ? YGO_POS_ATTACK
                                                                                                      : YGO_POS_DEFENSE;
        for (i = 0; i < q->count; i++)
            if (q->items[i].value == want)
            {
                Pick(out, i);
                return;
            }
        Pick(out, 0);
        return;
    }
    case YGO_Q_ZONE:
        /* its own zones first, in order; the opponent's when asked for theirs */
        for (i = 0; i < q->count && out->count < q->min; i++)
            if (!(q->items[i].flags & YGO_ZF_OPPONENT))
                out->picks[out->count++] = (uint8_t)i;
        for (i = 0; i < q->count && out->count < q->min; i++)
            if (q->items[i].flags & YGO_ZF_OPPONENT)
                out->picks[out->count++] = (uint8_t)i;
        return;
    case YGO_Q_SORT:
        for (i = 0; i < q->count && i < YGO_PICKS_MAX; i++)
            out->picks[out->count++] = (uint8_t)i;
        return;
    case YGO_Q_COUNTER:
    {
        unsigned left = q->sum;
        out->count = q->count;
        for (i = 0; i < q->count; i++)
        {
            unsigned take = q->items[i].param < left ? q->items[i].param : left;
            out->picks[i] = (uint8_t)take;
            left -= take;
        }
        return;
    }
    case YGO_Q_NAME:
    {
        /* a card the opponent has shown, else the first one that can be named */
        unsigned best = 0, o, k;
        for (o = 0; o < s->objCount && !best; o++)
            if (s->objs[o].owner != q->player && (s->objs[o].location == YGO_LOC_GRAVE || s->objs[o].faceUp))
                for (k = 0; k < d->nameCount; k++)
                    if (d->names[k] == s->objs[o].card)
                    {
                        best = k;
                        break;
                    }
        out->count = 2;
        out->picks[0] = (uint8_t)(best & 0xffu);
        out->picks[1] = (uint8_t)(best >> 8);
        return;
    }
    case YGO_Q_OPTION:
    default:
        for (i = 0; i < q->min && i < q->count; i++)
            out->picks[out->count++] = (uint8_t)i;
        if (!out->count && q->count)
            Pick(out, 0);
        return;
    }
}
