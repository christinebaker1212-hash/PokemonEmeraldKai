/* Yu-Gi-Oh! opponents: every question answered from what's on the table, the way a careful player reads it. The
 * engine (EDOPro) offers only what's legal; the AI weighs those choices: removal and draws first, the strongest
 * monster it can Summon (Set when it would lose a battle), Traps Set, attacks that win their battle or go direct,
 * Traps sprung when the opponent acts. It never looks at cards it couldn't see: the opponent's hand, face-down cards
 * and Decks are unknown to it (only how many there are). It plays at a level (0 a beginner .. 3 the best): every
 * choice is weighed by how much worse it looks than the best one, e^(-loss / T), T smaller the better the level, so
 * near misses are common and blunders rare; lower levels also miss a Trap's moment, pick the second-best target or
 * the wrong battle position now and then. Its dice come from what both players can see, so a duel replays alike.
 * Before it plays an effect it reckons what the effect brings (Outcome: the cards it takes from each side, draws and
 * searches, against its costs in cards and Life Points), from the card's words and Yugipedia's tags for the card
 * (ygo_ai_tags.h, tools/ygo/gen_ai_tags.py): an effect that costs more than it brings is never played, at any level,
 * and a negated card's effect brings nothing (the user, 2026-11-24: "use its cards intelligently to ensure hand and
 * card advantage on the field, not blow up their monsters for no reason"); the lower its level, the rougher its
 * reckoning of the rest. */
#include <string.h>
#include "ygo_internal.h"
#include "ygo_ai_tags.h"

static unsigned sLevel = 3;
static uint32_t sRoll;
/* Misplays (the user, 2026-11-24: "I still think they should misplay ... children should misplay more often than
 * adults, maybe even comically so, depending on ELO"): sBlunder in percent, the chance at each such moment that the
 * duelist makes the play a careful one wouldn't (an effect that costs more than it brings, a negated monster's, an
 * attack into a monster it can't beat, a weak monster Summoned face up into a stronger one, the wrong position);
 * sComical, a child far from good: also its removal on its own cards now and then, and gleefully */
static unsigned sBlunder;
static int sComical;

static uint32_t Next(void)
{
    sRoll ^= sRoll << 13;
    sRoll ^= sRoll >> 17;
    sRoll ^= sRoll << 5;
    return sRoll;
}
static void Seed(const struct YgoDuel *d)
{
    const struct YgoState *s = &d->s;
    uint32_t h = 2166136261u ^ sLevel * 0x9E3779B1u;
    h = (h ^ YgoDuel_AnswerCount(d)) * 16777619u;
    h = (h ^ (s->turn | s->phase << 16)) * 16777619u;
    h = (h ^ (uint32_t)s->players[0].lp) * 16777619u;
    h = (h ^ (uint32_t)s->players[1].lp) * 16777619u;
    h = (h ^ (d->query.type | d->query.count << 8 | d->query.player << 16)) * 16777619u;
    sRoll = h ? h : 1;
    Next();
}
static int Blunder(unsigned percent) { return percent && Next() % 100 < percent; }
/* percent chance, by level */
static int Chance(const uint8_t *byLevel) { return Next() % 100 < byLevel[sLevel]; }
/* e^-x in 1/65536 for x = num / den (no libm in the game) */
static uint32_t ExpNeg(uint32_t num, uint32_t den)
{
    uint32_t x = (uint32_t)((uint64_t)num * 94548 / den), whole = x >> 16;
    uint64_t f = x & 0xFFFF;
    if (whole >= 16)
        return 0;
    return (uint32_t)((65536 - f / 2 + (f * f >> 19)) >> whole);
}
/* One of n scored choices, weighed by how far each is below the best */
static unsigned Weigh(const int *score, unsigned n)
{
    static const uint16_t sTemper[4] = {480, 300, 150, 35};
    uint32_t w[YGO_QUERY_MAX + 2];
    uint64_t total = 0, r;
    int top = score[0];
    unsigned i;
    for (i = 1; i < n; i++)
        top = score[i] > top ? score[i] : top;
    for (i = 0; i < n; i++)
        total += w[i] = ExpNeg((uint32_t)(top - score[i]), sTemper[sLevel]) + 1;
    r = ((uint64_t)Next() << 16 ^ Next()) % total;
    for (i = 0; i + 1 < n && r >= w[i]; i++)
        r -= w[i];
    return i;
}

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
    for (i = 0; i < YGO_MZONES; i++)
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
    for (i = 0; i < YGO_MZONES; i++)
        if (pl->monsters[i] && s->objs[pl->monsters[i] - 1].faceUp && Atk(s, pl->monsters[i] - 1) > best)
            best = Atk(s, pl->monsters[i] - 1);
    return best;
}
static unsigned CardsOn(const struct YgoState *s, unsigned p)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_MZONES; i++)
        n += s->players[p].monsters[i] != 0;
    for (i = 0; i <= YGO_FIELD_ZONE; i++)
        n += s->players[p].spells[i] != 0;
    return n;
}
/* Where a card's text says something (any case): the AI reads a card's purpose from its words */
static const char *Find(const struct YgoCard *c, const char *phrase)
{
    const char *t;
    size_t n = strlen(phrase);
    if (!c || !c->text)
        return NULL;
    for (t = c->text; *t; t++)
    {
        size_t k;
        for (k = 0; k < n; k++)
        {
            char a = t[k], b = phrase[k];
            if (!a)
                return NULL;
            if (a >= 'A' && a <= 'Z')
                a += 32;
            if (b >= 'A' && b <= 'Z')
                b += 32;
            if (a != b)
                break;
        }
        if (k == n)
            return t;
    }
    return NULL;
}
static int Says(const struct YgoCard *c, const char *phrase) { return Find(c, phrase) != NULL; }
/* The number right after a phrase ("pay " -> 2000 in "pay 2000 LP"), -1 when it isn't there */
static int NumberAfter(const struct YgoCard *c, const char *phrase)
{
    const char *t = Find(c, phrase);
    int n = 0, any = 0;
    if (!t)
        return -1;
    for (t += strlen(phrase); *t >= '0' && *t <= '9'; t++)
        n = n * 10 + (*t - '0'), any = 1;
    return any ? n : -1;
}
/* Yugipedia's tags for a card (AT_* bits) */
static uint64_t Tags(const struct YgoCard *c)
{
    unsigned lo = 0, hi = sizeof(sAiTags) / sizeof(sAiTags[0]);
    if (!c)
        return 0;
    while (lo < hi)
    {
        unsigned mid = (lo + hi) / 2;
        if (sAiTags[mid].password < c->password)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo < sizeof(sAiTags) / sizeof(sAiTags[0]) && sAiTags[lo].password == c->password ? sAiTags[lo].tags : 0;
}
#define AT(t) ((uint64_t)1 << (t))
/* An Equip Spell that cripples the monster it's on (Paralyzing Potion, Germ Infection, Flint, Snatch Steal...);
 * every other one is for a monster of its own */
static int HarmfulEquip(const struct YgoCard *c)
{
    return Says(c, "equipped monster cannot attack") || Says(c, "equipped with this card cannot attack") ||
           Says(c, "or declare an attack") || Says(c, "is decreased by") || Says(c, "take control");
}
/* An effect that only helps what it picks (Rush Recklessly, Reinforcements, Mist Body...) */
static int Boost(const struct YgoCard *c)
{
    if (!c)
        return 0;
    if (c->property == YGO_PROP_EQUIP)
        return !HarmfulEquip(c);
    return (Says(c, "gains") || Says(c, "increase") || Says(c, "cannot be destroyed")) && !Says(c, "destroy that") &&
           !Says(c, "destroy 1") && !Says(c, "destroy it") && !Says(c, "loses") && !Says(c, "decrease") &&
           !Says(c, "control of") && !Says(c, "banish") && !Says(c, "return") && !Says(c, "negate") &&
           !Says(c, "face-down");
}
/* A card that's only any use while a monster is attacking (Waboku, Negate Attack, Sakuretsu Armor...) */
static int ForBattle(const struct YgoCard *c)
{
    return Says(c, "battle damage") || Says(c, "declares an attack") || Says(c, "attacks") ||
           Says(c, "an attack") || Says(c, "battle phase") || Says(c, "attacking monster");
}
/* The sum of what each side has among its monsters, as each player sees them */
static int MonsterWorth(const struct YgoState *s, unsigned me, unsigned p)
{
    unsigned i;
    int v = 0;
    for (i = 0; i < YGO_MZONES; i++)
        if (s->players[p].monsters[i])
            v += Value(s, me, s->players[p].monsters[i] - 1);
    return v;
}
static unsigned SpellTraps(const struct YgoState *s, unsigned p, unsigned except)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_FIELD_ZONE; i++)
        n += s->players[p].spells[i] && s->players[p].spells[i] - 1 != except;
    return n;
}
static int FaceUpMonster(const struct YgoState *s, unsigned p)
{
    unsigned i;
    for (i = 0; i < YGO_MZONES; i++)
        if (s->players[p].monsters[i] && s->objs[s->players[p].monsters[i] - 1].faceUp)
            return 1;
    return 0;
}
/* Its deck's game plan, read from its own cards' words (all of them are its own to know): how many of them want to
 * go to the Graveyard ("sent to the GY": Sangan, Shaddolls, Mystic Tomato...) and how many are FLIP monsters. A deck
 * full of either plays to it: it sends cards to the Graveyard and discards gladly, it Sets its FLIP monsters and flips
 * them when it can. */
enum { PLAN_GRAVE = 1, PLAN_FLIP = 2 };
static unsigned Plan(const struct YgoState *s, unsigned me)
{
    unsigned o, grave = 0, flip = 0, total = 0;
    for (o = 0; o < s->objCount; o++)
    {
        const struct YgoCard *c;
        if (s->objs[o].owner != me || !(c = Ygo_Card(s->objs[o].card)))
            continue;
        total++;
        if (Says(c, "sent to the graveyard") || Says(c, "sent to the gy") || Says(c, "is in your graveyard") ||
            Says(c, "is discarded"))
            grave++;
        if (Ygo_IsMonster(c) && (c->abilities & YGO_ABILITY_FLIP))
            flip++;
    }
    return (total && grave * 6 >= total ? PLAN_GRAVE : 0) | (total && flip * 6 >= total ? PLAN_FLIP : 0);
}
/* A card that sends its own cards to the Graveyard: from the Deck, or by discarding */
static int FillsGrave(const struct YgoCard *c)
{
    return Says(c, "from your deck to the graveyard") || Says(c, "from your deck to the gy") ||
           Says(c, "send 1") || Says(c, "discard");
}
/* Is playing this Spell or Trap now a waste, or worse? (each check is a card's own words: a mistake no player
 * would make) */
static int Pointless(const struct YgoState *s, unsigned me, unsigned o, const struct YgoCard *c, int myTurn)
{
    if (!c || Ygo_IsMonster(c))
        return 0;
    /* Life Points with plenty left */
    if ((Says(c, "increase your life points") || Says(c, "gain") ) && Says(c, "life points") &&
        !Says(c, "destroy") && !Says(c, "draw") && s->players[me].lp >= 6000)
        return 1;
    /* clearing the field when it takes more of ours than theirs */
    if (Says(c, "destroy all monsters on the field") && MonsterWorth(s, me, me) >= MonsterWorth(s, me, me ^ 1))
        return 1;
    if ((Says(c, "destroy all spell and trap cards on the field") || Says(c, "destroy all spell/trap cards on the field")) &&
        SpellTraps(s, me, o) >= SpellTraps(s, me ^ 1, NO_OBJ))
        return 1;
    /* an Equip Spell with nothing of the right side to go on */
    if (c->property == YGO_PROP_EQUIP && !Says(c, "special summon"))
        return HarmfulEquip(c) ? !FaceUpMonster(s, me ^ 1) : !FaceUpMonster(s, me);
    /* battle tricks away from a battle; a boost on its own turn only before its attacks, when it turns a battle its
       monster would lose into one it wins */
    if (myTurn && (c->frame == YGO_TRAP || c->property == YGO_PROP_QUICK) && ForBattle(c))
        return 1;
    if (myTurn && (c->frame == YGO_TRAP || c->property == YGO_PROP_QUICK) && Boost(c))
    {
        int theirBest = TheirBest(s, me, NULL);
        unsigned i;
        if (s->phase != YGO_PHASE_MAIN1)
            return 1;
        for (i = 0; i < YGO_MZONES; i++)
        {
            unsigned m = s->players[me].monsters[i];
            if (m && s->objs[m - 1].faceUp && s->objs[m - 1].attackPos && Atk(s, m - 1) <= theirBest &&
                Atk(s, m - 1) + 800 > theirBest)
                return 0;
        }
        return 1;
    }
    return 0;
}

/* What a side's cards on the field are worth as `me` sees them: its monsters (KIND_MON), its Spells and Traps
 * (KIND_ST), all but `except`; *best the most one of them is worth, *count how many */
enum { KIND_MON = 1, KIND_ST = 2 };
static int FieldWorth(const struct YgoState *s, unsigned me, unsigned p, unsigned kinds, unsigned except, int *best,
                      unsigned *count)
{
    unsigned i;
    int v = 0, top = 0;
    if (count)
        *count = 0;
    for (i = 0; i < YGO_MZONES && (kinds & KIND_MON); i++)
        if (s->players[p].monsters[i] && s->players[p].monsters[i] - 1 != except)
        {
            int w = Value(s, me, s->players[p].monsters[i] - 1);
            v += w, top = w > top ? w : top;
            if (count)
                (*count)++;
        }
    for (i = 0; i <= YGO_FIELD_ZONE && (kinds & KIND_ST); i++)
        if (s->players[p].spells[i] && s->players[p].spells[i] - 1 != except)
        {
            /* (a Spell left face up on the field has done its work: a Continuous one or Equip is still worth keeping) */
            int w = Value(s, me, s->players[p].spells[i] - 1);
            v += w, top = w > top ? w : top;
            if (count)
                (*count)++;
        }
    if (best)
        *best = top;
    return v;
}
/* The least a monster of its own is worth (a Tribute for a cost), `except` left out; 0 when there's none */
static int Cheapest(const struct YgoState *s, unsigned me, unsigned except)
{
    unsigned i;
    int low = 0;
    for (i = 0; i < YGO_MZONES; i++)
        if (s->players[me].monsters[i] && s->players[me].monsters[i] - 1 != except)
        {
            int w = Value(s, me, s->players[me].monsters[i] - 1);
            low = !low || w < low ? w : low;
        }
    return low;
}
/* Life Points as cards: paying n is little with plenty left, a card's worth when it halves them, out of the question
 * near the end */
static int LpWorth(const struct YgoState *s, unsigned me, int n)
{
    int left = s->players[me].lp - n;
    if (n <= 0)
        return 0;
    if (left <= 0)
        return 100000;
    return (int)((int64_t)n * 1000 / (left < 400 ? 400 : left));
}
static int FreeMonsterZone(const struct YgoState *s, unsigned me)
{
    unsigned i;
    for (i = 0; i < 5; i++)
        if (!s->players[me].monsters[i])
            return 1;
    return 0;
}
/* A card of several effects (its tags may be another effect's), or left to a coin or a die */
static int Multi(const struct YgoCard *c)
{
    const char *t, *first;
    if (!c)
        return 0;
    if (Says(c, "\xb7") || Says(c, "pendulum effect") || Says(c, "toss a coin") || Says(c, "roll a") ||
        Says(c, "if this card is in your gy") || Says(c, "if this card is in your graveyard"))
        return 1;
    first = Find(c, "you can ");
    if (!first)
        return 0;
    for (t = first + 1; *t; t++)
        if ((t[0] == 'Y' || t[0] == 'y') && !strncmp(t + 1, "ou can ", 7))
            return 1;
    return 0;
}
/* The other side doing something now: their effect in the chain, a Summon of theirs, their attack */
static int TheyAct(const struct YgoState *s, unsigned me)
{
    uint32_t seq;
    if (s->chainCount)
        return s->chain[s->chainCount - 1].player != me;
    if (s->attacker && s->objs[s->attacker - 1].controller != me)
        return 1;
    for (seq = s->eventSeq; seq > 0 && s->eventSeq - seq < 6; seq--)
    {
        const struct YgoEvent *e = YgoDuel_GetEvent(s, seq - 1);
        if (!e || e->type == YGO_EV_PHASE || e->type == YGO_EV_TURN)
            break;
        if (e->type == YGO_EV_SUMMON && e->player != me)
            return 1;
    }
    return 0;
}
/* What playing this card's effect is likely to bring `me`, in card worth (a card is about 1000): the cards it takes
 * from the other side and its own, what it draws, searches or Summons, less what it costs. A card's words say how
 * (the numbers: "pay 2000 LP", "draw 2 cards"; "all" for the whole field, "other" sparing itself), Yugipedia's tags
 * whose cards it touches ("Destroys your opponent's Monster Cards" against "Destroys Monster Cards") and its costs.
 * An effect nothing here recognises brings a little: its Deck wanted it. */
static int Outcome(const struct YgoState *s, unsigned me, unsigned o, const struct YgoCard *c, unsigned plan)
{
    const struct YgoObj *ob = o < s->objCount ? &s->objs[o] : NULL;
    uint64_t t = Tags(c);
    unsigned they = me ^ 1, mineKinds = 0, theirKinds = 0, self = NO_OBJ, n;
    int gain = 0, cost = 0, known = 0, k, onField, sure = 0;
    if (!c)
        return 0;
    onField = ob && (ob->location == YGO_LOC_MZONE || ob->location == YGO_LOC_SZONE);
    /* its costs */
    if ((k = NumberAfter(c, "pay ")) > 0 && (Says(c, " lp") || Says(c, "life points")))
        cost += LpWorth(s, me, k);
    else if (Says(c, "pay half your lp") || Says(c, "pay half your life points"))
        cost += LpWorth(s, me, s->players[me].lp / 2);
    else if (t & AT(AT_PAY_LP))
        cost += LpWorth(s, me, 1000);
    if (t & AT(AT_DISCARD_COST))
        cost += (plan & PLAN_GRAVE) ? 150 : 500;
    if (t & AT(AT_TRIBUTE_COST))
        cost += Cheapest(s, me, o);
    if ((t & AT(AT_SELF_COST)) && onField)
        cost += (plan & PLAN_GRAVE) ? Value(s, me, o) / 3 : Value(s, me, o);
    /* a negated card's effect does nothing; its costs are still paid */
    if (onField && ob->negated)
        return -cost - 400;
    /* destroying, banishing, sending to the Graveyard or returning cards on the field: whose, and which */
    if (t & (AT(AT_DMON_THEIRS) | AT(AT_DMON_ANY) | AT(AT_REMOVE_THEIRS) | AT(AT_REMOVE_ANY)))
        theirKinds |= KIND_MON;
    if (t & (AT(AT_DST_THEIRS) | AT(AT_DST_ANY) | AT(AT_REMOVE_THEIRS) | AT(AT_REMOVE_ANY)))
        theirKinds |= KIND_ST;
    if (t & (AT(AT_DMON_ANY) | AT(AT_REMOVE_ANY)))
        mineKinds |= KIND_MON;
    if (t & (AT(AT_DST_ANY) | AT(AT_REMOVE_ANY)))
        mineKinds |= KIND_ST;
    /* (its own cards alone: a whole field of them, else a cost or a drawback the Deck plays it for: Instant Fusion,
       Limiter Removal) */
    if (Says(c, "destroy all") && (t & AT(AT_DMON_MINE)))
        mineKinds |= KIND_MON;
    if (Says(c, "destroy all") && (t & AT(AT_DST_MINE)))
        mineKinds |= KIND_ST;
    if (!t)
    {
        /* no tags for it: its words alone */
        int all = Says(c, "destroy all") || Says(c, "banish all");
        if (all && (Says(c, "your opponent controls") || Says(c, "your opponent's")))
            theirKinds = (Says(c, "monster") ? KIND_MON : 0) | (Says(c, "spell") || Says(c, "trap") ? KIND_ST : 0);
        else if (all && Says(c, "on the field"))
        {
            unsigned kinds = Says(c, "cards on the field") ? KIND_MON | KIND_ST :
                             (Says(c, "monsters") ? KIND_MON : 0) | (Says(c, "spell") || Says(c, "trap") ? KIND_ST : 0);
            theirKinds = mineKinds = kinds;
        }
    }
    if (theirKinds | mineKinds)
    {
        int mass = Says(c, "destroy all") || Says(c, "banish all") || Says(c, "return all") || Says(c, "send all") ||
                   Says(c, "as many");
        int bounce = (t & AT(AT_BOUNCE)) && !(t & (AT(AT_DMON_ANY) | AT(AT_DMON_THEIRS) | AT(AT_DST_ANY) |
                                                    AT(AT_DST_THEIRS)));
        int theirBest, mineBest, theirs, mine;
        unsigned theirCount, mineCount;
        known = 1;
        /* (the card itself: spared when it says "other", else it goes with the rest) */
        if (Says(c, "other cards") || Says(c, "other monsters") || Says(c, "except this card") || !onField)
            self = o;
        theirs = FieldWorth(s, me, they, theirKinds, NO_OBJ, &theirBest, &theirCount);
        mine = FieldWorth(s, me, me, mineKinds, self, &mineBest, &mineCount);
        if (mass)
            gain += theirs - mine, sure = theirKinds && mineKinds && !Says(c, "toss a coin") && !Says(c, "roll a");
                                                                                /* (the whole field: Dark Hole, Demise) */
        else if (theirCount)
            gain += theirBest * 9 / 10;   /* one picked: the best of theirs */
        else if (mineCount)
            gain -= Cheapest(s, me, self), sure = !Multi(c);   /* only its own to pick (Mystical Space Typhoon) */
        if (bounce)
            gain = gain * 3 / 5;   /* (a card returned to the hand comes back) */
    }
    /* cards drawn, searched, brought back or Summoned */
    n = (unsigned)(NumberAfter(c, "draw ") > 0 ? NumberAfter(c, "draw ") : 1);
    if (Says(c, "same number"))
        known = 1;   /* (both discard and draw as many: Card Destruction) */
    else
    {
        if (t & AT(AT_DRAW))
            gain += 1000 * (int)n, known = 1;
        /* (what both players draw or discard alike, the Deck plays for its own side: One Day of Peace, Dark World
           Dealings) */
        int both = Says(c, "each player") || Says(c, "both players");
        if ((t & AT(AT_THEY_DRAW)) && !both)
            gain -= 1000 * (int)n, known = 1;
        if (t & AT(AT_DISCARD_EFFECT))
        {
            int d = NumberAfter(c, "discard ");
            gain -= ((plan & PLAN_GRAVE) ? 150 : 500) * (d > 0 ? d : 1), known = 1;
        }
        if ((t & AT(AT_THEY_DISCARD)) && !both)
            gain += 600, known = 1;
    }
    if (t & AT(AT_SEARCH))
    {
        /* ("add up to 2", "add 1") */
        int a = NumberAfter(c, "add up to ") > 0 ? NumberAfter(c, "add up to ") : NumberAfter(c, "add ");
        gain += 900 * (a > 1 ? (a > 3 ? 3 : a) : 1), known = 1;
    }
    if (t & AT(AT_FROM_GY_TO_HAND))
        gain += 800, known = 1;
    if ((t & AT(AT_SUMMON)) && FreeMonsterZone(s, me))
    {
        /* from a Graveyard: the best monster there it could bring back (Monster Reborn...); else a fair monster */
        int best = 0;
        if (Says(c, "graveyard") || Says(c, "gy"))
        {
            unsigned p, i;
            for (p = 0; p < 2; p++)
                if (p == me || Says(c, "either") || Says(c, "your opponent's graveyard") || Says(c, "your opponent's gy"))
                    for (i = 0; i < s->players[p].graveCount; i++)
                    {
                        const struct YgoCard *m = Card(s, s->players[p].grave[i]);
                        if (m && Ygo_IsMonster(m))
                        {
                            int w = Value(s, me, s->players[p].grave[i]);
                            best = w > best ? w : best;
                        }
                    }
        }
        gain += best ? best : 700, known = 1;
    }
    if (t & AT(AT_NEGATE))
    {
        /* worth it against something of theirs going on (their effect in the chain, their Summon, their attack) */
        gain += TheyAct(s, me) ? 1500 : FaceUpMonster(s, they) ? 300 : 0;
        known = 1;
    }
    if (t & AT(AT_DAMAGE))
    {
        k = NumberAfter(c, "inflict ");
        gain += k >= s->players[they].lp ? 100000 : k > 0 ? k / 4 : 200;
        known = 1;
    }
    if (t & AT(AT_STATS))
        gain += 250, known = 1;
    /* an effect it can't read: worth what it costs (its Deck plays it), unless the cost is ruinous */
    if (!known)
        return cost >= 2500 ? 1000 - cost : 300;
    if (cost >= 2500)
        sure = 1;
    if (!Ygo_IsMonster(c) && c->property != YGO_PROP_CONTINUOUS && c->property != YGO_PROP_EQUIP &&
        c->property != YGO_PROP_FIELD && !Says(c, "same number"))
        gain -= 300;   /* (a Spell or Trap that's spent: a part of a card, the rest its moment) */
    /* a loss only when it's sure: the tags are the whole card's, and a card of several effects (or a coin's) may owe
       its costs and drawbacks to another one than this */
    if (!sure && gain - cost < 50)
        return 50;
    return gain - cost;
}
/* The reckoning at its level: exact at the best, rougher below (never so rough that a loss looks like a gain: that
 * check is made on the exact one) */
static int Reckon(int outcome)
{
    static const uint16_t sRough[4] = {600, 350, 150, 0};
    if (!sRough[sLevel])
        return outcome;
    return outcome + (int)(Next() % (2u * sRough[sLevel] + 1)) - sRough[sLevel];
}
/* (an effect that costs more than it brings: a loss) */
#define LOSS (-150)

/* The best target an attacker has: object, NO_OBJ for a direct attack, or -1 when no attack is worth it */
static int BestTarget(const struct YgoState *s, unsigned me, unsigned attacker, int direct)
{
    const struct YgoPlayer *pl = &s->players[me ^ 1];
    int atk = Atk(s, attacker), best = -1, bestScore = 0;
    unsigned i, monsters = 0;
    for (i = 0; i < YGO_MZONES; i++)
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
    int best = -1, bestScore = 0, moveOn = -1, toBattle = -1, scores[YGO_QUERY_MAX + 1], items[YGO_QUERY_MAX + 1];
    unsigned n = 0, plan = Plan(s, me);
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
            {
                int out = Outcome(s, me, c->obj, card, plan);
                score = 900 + Reckon(out) / 4;
                score = score < 450 ? 450 : score > 1500 ? 1500 : score;
                if (out < LOSS || Pointless(s, me, c->obj, card, 1))
                    score = Blunder(sBlunder) ? (sComical ? 1300 : 950) : 0;   /* (a misplay: "it does something!") */
                else if ((plan & PLAN_GRAVE) && FillsGrave(card) && score < 950)
                    score = 950;   /* (its plan: the Graveyard) */
            }
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
                score = Blunder(sBlunder / 2) ? score : 0;   /* (Set it instead) */
            /* a monster that would just be run over in Attack Position, with no effect for being Summoned: Set it */
            else if (card->atk != YGO_STAT_UNKNOWN && theirMonsters && card->atk < theirBest &&
                     !Says(card, "summoned") && !Says(card, "when this card is normal summoned"))
                score = Blunder(sBlunder) ? score : 0;
            break;
        case YGO_ACT_SET_MONSTER:
            if (!card)
                break;
            score = 400 + (card->def == YGO_STAT_UNKNOWN ? 1000 : card->def) / 20;
            if ((plan & PLAN_FLIP) && (card->abilities & YGO_ABILITY_FLIP))
                score = 560;   /* (its plan: FLIP effects) */
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
            /* a FLIP effect when it has something to work on (a monster-destroying one with their monsters there),
               or a monster that wins its battles */
            if (card && card->atk != YGO_STAT_UNKNOWN && card->atk > theirBest && theirMonsters)
                score = 350;
            else if (card && (card->abilities & YGO_ABILITY_FLIP))
            {
                int needsTheirs = Says(card, "destroy") || Says(card, "return") || Says(card, "banish") ||
                                  Says(card, "control");
                score = !needsTheirs || theirMonsters || CardsOn(s, me ^ 1) ? ((plan & PLAN_FLIP) ? 600 : 380) : 0;
            }
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
        if (score > 0 && n < YGO_QUERY_MAX)
            scores[n] = score, items[n++] = (int)i;
        if (score > bestScore)
            bestScore = score, best = (int)i;
    }
    if (best >= 0)
    {
        // the plays it sees, and moving on (a card held back: a plausible slip), weighed
        unsigned k;
        scores[n] = 150;
        items[n] = -1;
        k = Weigh(scores, n + 1);
        if (items[k] >= 0)
        {
            Pick(out, (unsigned)items[k]);
            return;
        }
    }
    /* nothing to play: battle when a monster can do something there */
    if (toBattle >= 0)
    {
        const struct YgoPlayer *pl = &s->players[me];
        for (i = 0; i < YGO_MZONES; i++)
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
    static const uint8_t sRash[4] = {40, 20, 6, 0};   /* an attack into a monster it can't beat */
    int scores[YGO_QUERY_MAX + 1], items[YGO_QUERY_MAX + 1], moveOn = -1;
    unsigned n = 0, k, setBack = 0;
    for (i = 0; i < YGO_FIELD_ZONE; i++)
    {
        unsigned st = s->players[me ^ 1].spells[i];
        setBack += st && !s->objs[st - 1].faceUp;
    }
    for (i = 0; i < q->count; i++)
    {
        const struct YgoChoice *c = &q->items[i];
        if (c->kind == YGO_ACT_ATTACK && c->obj < s->objCount && n < YGO_QUERY_MAX)
        {
            if (BestTarget(s, me, c->obj, c->flags & 1) == -1 && !Chance(sRash) && !Blunder(sBlunder / 2))
                continue;
            /* with face-down Spells and Traps waiting (Mirror Force, Sakuretsu Armor...), the monster it can best
               spare attacks first to draw them out; otherwise the strongest */
            scores[n] = setBack ? 400 + (4000 - Atk(s, c->obj)) / 10 : 400 + Atk(s, c->obj) / 10;
            items[n++] = (int)i;
        }
        else if (c->kind == YGO_ACT_TO_MAIN2 || (c->kind == YGO_ACT_END_TURN && moveOn < 0))
            moveOn = (int)i;
    }
    if (n)
    {
        scores[n] = 150;   /* (holding back) */
        items[n] = -1;
        k = Weigh(scores, n + 1);
        if (items[k] >= 0)
        {
            Pick(out, (unsigned)items[k]);
            return;
        }
    }
    Pick(out, moveOn >= 0 ? (unsigned)moveOn : q->count - 1);
}
/* Should it chain now? When the opponent is acting: an effect in the chain, an attack on it, a Summon */
static int Pressure(const struct YgoDuel *d, unsigned me)
{
    /* (never onto its own link: one answer to an attack or an effect is enough, Magic Cylinder and Mirror Force
       aren't both spent on one attack) */
    return TheyAct(&d->s, me);
}
static void Chain(const struct YgoDuel *d, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned me = q->player, i, plan = Plan(s, me);
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
        /* (never an effect that costs more than it brings, nor a negated card's) */
        if (Outcome(s, me, c->obj, card, plan) < LOSS && !Blunder(sBlunder))
            continue;
        /* monsters' effects and Spells / Traps that answer the opponent; never a chain to itself (lower levels miss
         * the moment now and then) */
        static const uint8_t sMiss[4] = {60, 38, 16, 3};
        if (Chance(sMiss))
            continue;
        /* battle tricks only in a battle, and a boost only for a monster of its own in it */
        if (card && (card->frame == YGO_TRAP || card->property == YGO_PROP_QUICK))
        {
            int battling = s->attacker != 0;
            int mineIn = battling && ((s->objs[s->attacker - 1].controller == me) ||
                                      (s->attackTarget && s->objs[s->attackTarget - 1].controller == me));
            int slip = Blunder(sBlunder / 3);   /* (a Trap sprung at nothing) */
            if (ForBattle(card) && !battling && !slip)
                continue;
            if (Boost(card) && !mineIn && !slip)
                continue;
            if (Pointless(s, me, c->obj, card, 0) && !slip)
                continue;
        }
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
        static const uint8_t sOther[4] = {45, 25, 8, 0};
        int t = BestTarget(s, me, s->attacker - 1, 1);
        if (Chance(sOther) && n > 1)
        {
            Pick(out, Next() % n);
            return;
        }
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
        const struct YgoCard *c = Ygo_Card(q->items[i].card), *source = Ygo_Card(q->card);
        int v = o == NO_OBJ ? (c ? 1000 + (c->atk == YGO_STAT_UNKNOWN ? 0 : c->atk) : 2500) : Value(s, me, o);
        int mine = o != NO_OBJ && o < s->objCount && s->objs[o].controller == me;
        int onField = o != NO_OBJ && o < s->objCount &&
                      (s->objs[o].location == YGO_LOC_MZONE || s->objs[o].location == YGO_LOC_SZONE);
        /* an effect that helps what it picks: its own monsters (the stronger the better), never theirs; one that
           hurts: theirs */
        if (onField && (q->prompt == YGO_P_EQUIP || q->prompt == YGO_P_TARGET || q->prompt == YGO_P_NONE) &&
            source && (q->prompt == YGO_P_EQUIP ? 1 : Boost(source)))
        {
            int helps = q->prompt == YGO_P_EQUIP ? !HarmfulEquip(source) : 1;
            value[i] = (mine == helps ? v : -v) - (q->items[i].flags & YGO_IF_PICKED ? 100000 : 0);
            order[i] = (uint8_t)i;
            continue;
        }
        switch (q->prompt)
        {
        case YGO_P_TRIBUTE:
        case YGO_P_DISCARD:
        case YGO_P_MATERIAL:
        case YGO_P_SYNCHRO_MATERIAL:
        case YGO_P_XYZ_MATERIAL:
        case YGO_P_LINK_MATERIAL:
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
    /* a child at it: now and then the choice turned on its head (its own monster destroyed, the opponent's equipped) */
    if (sComical && n >= 2 && q->prompt != YGO_P_ATTACK_TARGET && Blunder(sBlunder / 3))
        for (i = 0; i < n / 2; i++)
        {
            uint8_t t = order[i];
            order[i] = order[n - 1 - i];
            order[n - 1 - i] = t;
        }
    /* now and then the second best first (a target misjudged) */
    {
        static const uint8_t sSwap[4] = {50, 32, 15, 5};
        /* (only between two that are close: never a target on the wrong side) */
        int a0 = value[order[0]], a1 = n >= 2 ? value[order[1]] : 0;
        if (n >= 2 && a1 > -50000 && (a0 >= 0) == (a1 >= 0) && a0 - a1 <= (a0 < 0 ? -a0 : a0) / 4 + 200 &&
            Chance(sSwap))
        {
            uint8_t t = order[0];
            order[0] = order[1];
            order[1] = t;
        }
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
            worth += q->items[order[k]].param ? q->items[order[k]].param : 1;   /* (a worth not given counts as one) */
    }
    if (q->max && k > q->max)
        k = q->max;   /* (never more than asked) */
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

void YgoDuel_AiAnswer(const struct YgoDuel *d, struct YgoAnswer *out) { YgoDuel_AiAnswerLevel(d, 3, out); }
/* A level's own misplays, for a duelist of no particular age (a grown-up of middling rating: Ygo_Misplays) */
static const uint8_t sLevelBlunder[4] = {30, 12, 4, 0};
void YgoDuel_AiAnswerLevel(const struct YgoDuel *d, unsigned level, struct YgoAnswer *out)
{
    YgoDuel_AiAnswerAs(d, level, sLevelBlunder[level > 3 ? 3 : level], 0, out);
}
/* How often a duelist misplays (percent) and whether comically: their level, their age (a child far more, a grown-up
 * less) and their own rating (the weaker, the more) */
unsigned Ygo_Misplays(unsigned level, unsigned age, int elo, int *comical)
{
    static const uint8_t sBase[4] = {45, 20, 6, 0};
    int p = sBase[level > 3 ? 3 : level];
    if (age == YGO_AGE_CHILD)
        p = p * 8 / 5;
    else if (age == YGO_AGE_ADULT)
        p = p * 3 / 5;
    if (elo > 0 && p)
        p += elo < 1350 ? 12 : elo < 1450 ? 6 : elo > 1750 ? -p / 2 : elo > 1650 ? -p / 4 : 0;
    p = p < 0 ? 0 : p > 80 ? 80 : p;
    if (comical)
        *comical = age == YGO_AGE_CHILD && level <= 1 && (elo <= 0 || elo < 1600);
    return (unsigned)p;
}
void YgoDuel_AiAnswerAs(const struct YgoDuel *d, unsigned level, unsigned blunder, int comical, struct YgoAnswer *out)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned i;
    memset(out, 0, sizeof(*out));
    sLevel = level > 3 ? 3 : level;
    sBlunder = level >= 3 ? 0 : blunder;   /* (the best never misplay) */
    sComical = comical && sBlunder;
    Seed(d);
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
        /* "use this effect?": yes, unless it costs more than it brings */
        if (q->prompt == YGO_P_EFFECT_YESNO && q->count == 2 &&
            Outcome(s, q->player, q->items[1].obj, Ygo_Card(q->card), Plan(s, q->player)) < LOSS)
        {
            Pick(out, 0);
            return;
        }
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
        static const uint8_t sWrong[4] = {30, 15, 5, 0};
        unsigned want = c && c->atk != YGO_STAT_UNKNOWN && (c->atk >= theirBest || c->atk >= c->def) ? YGO_POS_ATTACK
                                                                                                      : YGO_POS_DEFENSE;
        if (Chance(sWrong) || Blunder(sBlunder / 3))
            want = want == YGO_POS_ATTACK ? YGO_POS_DEFENSE : YGO_POS_ATTACK;
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
#ifdef YGO_AI_DEBUG
/* (tools: what the AI reckons an offered effect brings) */
int YgoAi_DebugOutcome(const struct YgoDuel *d, unsigned item)
{
    const struct YgoChoice *c = &d->query.items[item];
    return Outcome(&d->s, d->query.player, c->obj, Card(&d->s, c->obj), Plan(&d->s, d->query.player));
}
#endif
