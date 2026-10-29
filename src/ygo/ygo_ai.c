/* Yu-Gi-Oh! opponents: every question answered by trying each choice on a copy of the duel, playing on with a
 * plain default until the AI's next decision (or the turn changes), and scoring what's left. */
#include <string.h>
#include "ygo_internal.h"

#define SIM_STEPS 400
#define SIM_CHOICES 24

/* How good the state is for player: LP, the field, the hands */
static int Eval(const struct YgoState *s, unsigned me)
{
    int score = 0, side;
    unsigned p, i;
    if (s->over)
        return s->winner == me ? 10000000 : -10000000;
    for (p = 0; p < 2; p++)
    {
        const struct YgoPlayer *pl = &s->players[p];
        int v = pl->lp;
        side = p == me ? 1 : -1;
        for (i = 0; i < YGO_ZONES; i++)
        {
            unsigned m = pl->monsters[i];
            const struct YgoObj *b;
            if (!m)
                continue;
            b = &s->objs[m - 1];
            if (!b->faceUp)
                v += 900;   /* a face-down monster: unknown, worth something */
            else if (b->attackPos)
                v += 600 + YgoDuel_Atk(s, m - 1);
            else
                v += 400 + YgoDuel_Def(s, m - 1) * 2 / 3;
        }
        for (i = 0; i <= YGO_FIELD_ZONE; i++)
            if (pl->spells[i])
                v += s->objs[pl->spells[i] - 1].faceUp ? 300 : 500;
        v += 450 * pl->handCount;
        if (pl->deckCount < 5)
            v -= 400 * (5 - pl->deckCount);
        score += side * v;
    }
    return score;
}

/* The value of an object to the one choosing it, for prompts where more is better (targets of the opponent's,
 * cards to add) or worse (tributes, discards of one's own) */
static int ObjValue(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    if (!c)
        return 0;
    if (s->objs[o].location == YGO_LOC_MZONE && s->objs[o].faceUp)
        return YgoDuel_Atk(s, o) + YgoDuel_Def(s, o) / 4 + 100;
    if (c->frame <= YGO_RITUAL)
        return (c->atk == YGO_STAT_UNKNOWN ? 1000 : c->atk) + c->level * 50;
    if (c->frame == YGO_TOKEN)
        return 10;
    return 1200;   /* Spells and Traps */
}

/* The plain answer used inside simulations (no lookahead) */
static void Default(const struct YgoDuel *d, struct YgoAnswer *a)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned i;
    memset(a, 0, sizeof(*a));
    switch (q->type)
    {
    case YGO_Q_CHAIN:
        return;   /* pass */
    case YGO_Q_YESNO:
        a->count = 1;
        a->picks[0] = 1;
        return;
    case YGO_Q_IDLE:
    case YGO_Q_BATTLE:
        for (i = 0; i < q->count; i++)
            if (q->items[i].kind == YGO_ACT_END_TURN || q->items[i].kind == YGO_ACT_TO_MAIN2)
            {
                a->count = 1;
                a->picks[0] = i;
                return;
            }
        a->count = 1;
        a->picks[0] = q->count - 1;
        return;
    case YGO_Q_CARDS:
    {
        /* best first for the chooser: the opponent's best things, one's own worst */
        uint8_t order[YGO_QUERY_MAX];
        int value[YGO_QUERY_MAX];
        unsigned n = q->count, k, j;
        for (i = 0; i < n; i++)
        {
            unsigned o = q->items[i].obj;
            int v = o == NO_OBJ ? 2500 : ObjValue(s, o);
            int mine = o != NO_OBJ && (s->objs[o].controller == q->player || s->objs[o].location == YGO_LOC_HAND);
            switch (q->prompt)
            {
            case YGO_P_TRIBUTE:
            case YGO_P_DISCARD:
                v = -v;
                break;
            case YGO_P_ADD_TO_HAND:
            case YGO_P_SUMMON:
            case YGO_P_FUSION:
            case YGO_P_RITUAL:
            case YGO_P_EQUIP:
                break;
            default:
                if (mine)
                    v = -v;
                break;
            }
            value[i] = v;
            order[i] = i;
        }
        for (i = 0; i < n; i++)
            for (j = i + 1; j < n; j++)
                if (value[order[j]] > value[order[i]])
                {
                    uint8_t t = order[i];
                    order[i] = order[j];
                    order[j] = t;
                }
        k = q->min ? q->min : (q->max && value[order[0]] > 0 ? 1 : 0);
        a->count = k;
        for (i = 0; i < k; i++)
            a->picks[i] = order[i];
        return;
    }
    case YGO_Q_POSITION:
    case YGO_Q_OPTION:
    case YGO_Q_ZONE:
    default:
        a->count = 1;
        a->picks[0] = 0;
        return;
    }
}

/* Plays a copy on from answer with the default answers until the AI's next Main / Battle Phase decision, the turn
 * changes or the duel ends; returns the score */
static int Simulate(const struct YgoDuel *d, const struct YgoAnswer *answer, unsigned me)
{
    static struct YgoDuel sim;
    unsigned steps = 0, turn = d->s.turn;
    sim = *d;
    if (!YgoDuel_Answer(&sim, answer))
        return -20000000;
    while (sim.query.type != YGO_Q_NONE && steps++ < SIM_STEPS)
    {
        struct YgoAnswer a;
        if (sim.s.turn != turn)
            break;
        if (sim.query.player == me && (sim.query.type == YGO_Q_IDLE || sim.query.type == YGO_Q_BATTLE))
            break;
        Default(&sim, &a);
        if (!YgoDuel_Answer(&sim, &a))
            break;
    }
    return Eval(&sim.s, me);
}

static void Single(struct YgoAnswer *a, unsigned pick)
{
    memset(a, 0, sizeof(*a));
    a->count = 1;
    a->picks[0] = pick;
}

void YgoDuel_AiAnswer(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    unsigned me = q->player, i, tried = 0;
    int best = -2147483647, base;
    struct YgoAnswer a;
    Default(d, out);
    switch (q->type)
    {
    case YGO_Q_IDLE:
    case YGO_Q_BATTLE:
    {
        /* the baseline: move on (to battle when there is one: the battle itself is judged in the Battle Phase) */
        unsigned moveOn = q->count - 1, k;
        for (i = 0; i < q->count; i++)
            if (q->items[i].kind == YGO_ACT_TO_BATTLE)
                moveOn = i;
        if (moveOn == q->count - 1u)
            for (i = 0; i < q->count; i++)
                if (q->items[i].kind == YGO_ACT_END_TURN || q->items[i].kind == YGO_ACT_TO_MAIN2)
                {
                    /* (out of the Battle Phase: MAIN PHASE 2, where it can still play cards) */
                    moveOn = i;
                    if (q->items[i].kind == YGO_ACT_TO_MAIN2)
                        break;
                }
        Single(out, moveOn);
        base = q->items[moveOn].kind == YGO_ACT_TO_BATTLE ? Eval(&d->s, me) : Simulate(d, out, me);
        best = base;
        for (i = 0; i < q->count && tried < SIM_CHOICES; i++)
        {
            int v;
            k = q->items[i].kind;
            if (i == moveOn || k == YGO_ACT_END_TURN || k == YGO_ACT_TO_MAIN2 || k == YGO_ACT_TO_BATTLE ||
                k == YGO_ACT_CONCEDE)
                continue;
            Single(&a, i);
            v = Simulate(d, &a, me);
            tried++;
            /* a little in favor of developing the board */
            if (k == YGO_ACT_SUMMON || k == YGO_ACT_SET_MONSTER || k == YGO_ACT_SPECIAL)
                v += 50;
            if (k == YGO_ACT_SET_SPELL)
                v += 20;
            if (k == YGO_ACT_ATTACK)
                v += 10;
            if (v > best)
            {
                best = v;
                *out = a;
            }
        }
        return;
    }
    case YGO_Q_CHAIN:
    {
        struct YgoAnswer pass;
        memset(&pass, 0, sizeof(pass));
        best = Simulate(d, &pass, me);
        *out = pass;
        for (i = 0; i < q->count && i < SIM_CHOICES; i++)
        {
            int v;
            Single(&a, i);
            v = Simulate(d, &a, me);
            if (v > best + 100)
            {
                best = v;
                *out = a;
            }
        }
        return;
    }
    case YGO_Q_YESNO:
    case YGO_Q_POSITION:
    case YGO_Q_OPTION:
        for (i = 0; i < q->count; i++)
        {
            int v;
            Single(&a, i);
            v = Simulate(d, &a, me);
            if (v > best)
            {
                best = v;
                *out = a;
            }
        }
        return;
    case YGO_Q_CARDS:
        /* one pick from a short list: try each; otherwise the plain choice */
        if (q->min <= 1 && q->max == 1 && q->count <= SIM_CHOICES)
        {
            for (i = 0; i < q->count; i++)
            {
                int v;
                Single(&a, i);
                v = Simulate(d, &a, me);
                if (v > best)
                {
                    best = v;
                    *out = a;
                }
            }
            if (q->min == 0)
            {
                struct YgoAnswer none;
                memset(&none, 0, sizeof(none));
                if (Simulate(d, &none, me) > best)
                    *out = none;
            }
        }
        return;
    default:
        return;
    }
}
