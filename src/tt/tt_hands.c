// Moemon Kai: the hands Triple Triad players bring. A region's players draw from their region's cards
// (src/tt/tt_pools.c), as strong as the player's badges allow, each person starting somewhere else in the list and
// moving on by a hand a day: every card of the region turns up in time. Special players bring one of their own
// five-star cards (a different one each day), a four-star card and three of three stars. Final Fantasy XIV's deck
// limits hold for every hand.
#include <string.h>
#include "tt/npc.h"
#include "tt/duel.h"
#include "tt/rating.h"

static int In(const uint16_t *hand, unsigned n, unsigned id)
{
    unsigned i;
    for (i = 0; i < n; i++)
        if (hand[i] == id)
            return 1;
    return 0;
}
// Cards of the list with stars in [lo, hi], into out (at most max)
static unsigned Filter(const uint16_t *list, unsigned lo, unsigned hi, uint16_t *out, unsigned max)
{
    unsigned i, n = 0;
    for (i = 0; list[i] && n < max; i++)
    {
        const struct TtCard *c = Tt_Card(list[i]);
        if (c && c->stars >= lo && c->stars <= hi)
            out[n++] = list[i];
    }
    return n;
}
static unsigned AllOf(unsigned lo, unsigned hi, uint16_t *out, unsigned max)
{
    unsigned id, n = 0;
    for (id = 1; id <= TT_CARD_MAX && n < max; id++)
        if (Tt_Card(id) && Tt_Card(id)->stars >= lo && Tt_Card(id)->stars <= hi)
            out[n++] = (uint16_t)id;
    return n;
}
// Takes `want` cards from list starting at `start`, skipping any already in the hand
static unsigned Take(const uint16_t *list, unsigned n, unsigned start, unsigned want, uint16_t *hand, unsigned have)
{
    unsigned k;
    for (k = 0; k < n && want; k++)
    {
        unsigned id = list[(start + k) % n];
        if (In(hand, have, id))
            continue;
        hand[have++] = (uint16_t)id;
        want--;
    }
    return have;
}

void Tt_RegionHand(unsigned region, uint32_t h, unsigned tier, unsigned day, uint16_t hand[TT_HAND])
{
    static const uint8_t sLo[4] = {1, 1, 1, 2}, sHi[4] = {2, 3, 3, 3}, sFours[4] = {0, 0, 1, 2};
    static uint16_t low[TT_CARD_MAX + 1], four[TT_CARD_MAX + 1];
    const uint16_t *pool = gTtRegionPool[region < TT_REGIONS ? region : 0];
    unsigned nl, nf, have = 0, fours;
    if (tier > 3)
        tier = 3;
    memset(hand, 0, TT_HAND * sizeof(*hand));
    nl = Filter(pool, sLo[tier], sHi[tier], low, TT_CARD_MAX);
    nf = Filter(pool, 4, 4, four, TT_CARD_MAX);
    fours = sFours[tier] < nf ? sFours[tier] : nf;
    if (fours)
        have = Take(four, nf, h % nf + day * fours, fours, hand, have);
    if (nl)
        have = Take(low, nl, (h >> 8) % nl + day * (TT_HAND - fours), TT_HAND - have, hand, have);
    if (have < TT_HAND)
    {
        // a region with few such cards: the rest from anywhere in HOENN
        nl = AllOf(sLo[tier], sHi[tier], low, TT_CARD_MAX);
        have = Take(low, nl, (h >> 4) % nl + day * 5, TT_HAND - have, hand, have);
    }
}

void Tt_SpecialHand(unsigned special, unsigned day, uint16_t hand[TT_HAND])
{
    static uint16_t five[64], four[TT_CARD_MAX + 1], three[TT_CARD_MAX + 1];
    const uint16_t *own = gTtSpecialCards[special < TT_SPECIAL_COUNT ? special : 0];
    unsigned n5 = Filter(own, 5, 5, five, 64), n4 = Filter(own, 4, 4, four, TT_CARD_MAX), n3, have = 0;
    memset(hand, 0, TT_HAND * sizeof(*hand));
    if (n5)
        hand[have++] = five[day % n5];
    if (!n4)
        n4 = AllOf(4, 4, four, TT_CARD_MAX);
    have = Take(four, n4, n4 ? (special * 7 + day) % n4 : 0, 1, hand, have);
    n3 = AllOf(3, 3, three, TT_CARD_MAX);
    have = Take(three, n3, special * 13 + day * 3, TT_HAND - have, hand, have);
    (void)have;
}

/* ------------------------------------------------------------------ */
/* Hands that fit (include/tt/rating.h)                                 */
/* ------------------------------------------------------------------ */
static unsigned Off(int power, int target) { return (unsigned)(power > target ? power - target : target - power); }

void Tt_FitRegionHand(unsigned region, uint32_t h, unsigned tierCap, unsigned day, int target, uint16_t hand[TT_HAND])
{
    // the hands they could bring today: each tier up to the badges', a few days' worth of their region's cards; when
    // those can't come close, the other regions' too (a traveler's hand), counted a little further off
    enum { DAYS = 6, AWAY = 40, NEAR = 20 };
    static struct
    {
        uint8_t region, tier, k;
        uint16_t off;
    } list[TT_REGIONS * 4 * DAYS];
    uint16_t cand[TT_HAND];
    unsigned n = 0, r, t, k, i, best = ~0u, near = 0, pick, regions = 1;
    if (tierCap > 3)
        tierCap = 3;
    for (r = 0; r < regions; r++)
    {
        unsigned from = (region + r) % TT_REGIONS;
        for (t = 0; t <= tierCap; t++)
            for (k = 0; k < DAYS; k++)
            {
                unsigned off;
                Tt_RegionHand(from, h ^ (k * 0x9E3779B1u), t, day + k * 3, cand);
                off = Off(Tt_HandPower(cand), target) + (r ? AWAY : 0);
                list[n].region = (uint8_t)from, list[n].tier = (uint8_t)t, list[n].k = (uint8_t)k;
                list[n++].off = (uint16_t)(off < 0xFFFF ? off : 0xFFFF);
                best = off < best ? off : best;
            }
        if (r == 0 && best > 60)
            regions = TT_REGIONS;
    }
    for (i = 0; i < n; i++)
        near += list[i].off <= best + NEAR;
    pick = (h >> 5) % (near ? near : 1);
    for (i = 0; i < n; i++)
        if (list[i].off <= best + NEAR && !pick--)
            break;
    if (i == n)
        i = 0;
    Tt_RegionHand(list[i].region, h ^ (list[i].k * 0x9E3779B1u), list[i].tier, day + list[i].k * 3, hand);
}

void Tt_FitSpecialHand(unsigned special, unsigned day, int target, uint16_t hand[TT_HAND])
{
    // the five-star card stays theirs (unless that can't come close); the other four go from a four-star and three-stars
    // down to one-stars
    static const uint8_t sShapes[][4] = {{4, 3, 3, 3}, {3, 3, 3, 3}, {3, 3, 3, 2}, {3, 3, 2, 2}, {3, 2, 2, 2},
                                         {2, 2, 2, 2}, {2, 2, 2, 1}, {2, 2, 1, 1}, {2, 1, 1, 1}, {1, 1, 1, 1}};
    static uint16_t list[TT_MAX_STARS + 1][TT_CARD_MAX + 1];
    static unsigned count[TT_MAX_STARS + 1];
    const uint16_t *own = gTtSpecialCards[special < TT_SPECIAL_COUNT ? special : 0];
    uint16_t base[TT_HAND], cand[TT_HAND];
    unsigned shape, k, j, s, best = ~0u;
    Tt_SpecialHand(special, day, base);
    memcpy(hand, base, sizeof(base));
    best = Off(Tt_HandPower(base), target);
    for (s = 1; s <= TT_MAX_STARS; s++)
    {
        // their own cards of those stars first, then everyone's
        count[s] = Filter(own, s, s, list[s], TT_CARD_MAX);
        if (count[s] < 4)
            count[s] = AllOf(s, s, list[s], TT_CARD_MAX);
    }
    // (their five-star card stays unless even their gentlest hand with it is well above the player: then a four-star
    // of theirs leads instead, counted a little further off)
    for (shape = 0; shape < 2 * sizeof(sShapes) / sizeof(sShapes[0]); shape++)
        for (k = 0; k < 3; k++)
        {
            unsigned have = 1, off, lead = shape >= sizeof(sShapes) / sizeof(sShapes[0]);
            memset(cand, 0, sizeof(cand));
            cand[0] = base[0];
            if (lead)
            {
                if (!count[4])
                    continue;
                cand[0] = list[4][(special * 5 + day + k) % count[4]];
            }
            for (j = 0; j < 4; j++)
            {
                s = sShapes[shape % (sizeof(sShapes) / sizeof(sShapes[0]))][j];
                if (lead && s == 4)
                    s = 3;   /* (the four-star lead stays their only big card) */
                if (count[s])
                    have = Take(list[s], count[s], special * 13 + day * 3 + k * 7 + j, 1, cand, have);
            }
            if (have < TT_HAND)
                continue;
            off = Off(Tt_HandPower(cand), target) + (lead ? 60 : 0);
            if (off < best)
                best = off, memcpy(hand, cand, sizeof(cand));
        }
}

int Tt_DealFair(const uint8_t *owned, uint32_t seed, int target, uint16_t hand[TT_HAND])
{
    uint16_t cand[TT_HAND];
    unsigned i, best = ~0u;
    for (i = 0; i < 48; i++)
    {
        unsigned off;
        if (!Tt_DealRandom(owned, seed + i * 7919u, cand))
            return 0;
        off = Off(Tt_HandPower(cand), target);
        if (off < best)
            best = off, memcpy(hand, cand, sizeof(cand));
    }
    return 1;
}

void Tt_DealRegion(unsigned region, uint32_t seed, int target, uint16_t hand[TT_HAND])
{
    static uint8_t pool[TT_CARD_MAX + 1];
    const uint16_t *list = gTtRegionPool[region < TT_REGIONS ? region : 0];
    unsigned i, n = 0;
    memset(pool, 0, sizeof(pool));
    for (i = 0; list[i]; i++)
        n += !pool[list[i]], pool[list[i]] = 1;
    if (n < 12)   // (a thin region: everyone's cards)
        for (i = 1; i <= TT_CARD_MAX; i++)
            pool[i] = Tt_Card(i) != NULL;
    Tt_DealFair(pool, seed, target, hand);
    if (Off(Tt_HandPower(hand), target) > 60)
    {
        // their region's cards can't come close: a traveler's five from anywhere in HOENN
        uint16_t other[TT_HAND];
        for (i = 1; i <= TT_CARD_MAX; i++)
            pool[i] = Tt_Card(i) != NULL;
        Tt_DealFair(pool, seed ^ 0x77, target, other);
        if (Off(Tt_HandPower(other), target) < Off(Tt_HandPower(hand), target))
            memcpy(hand, other, sizeof(other));
    }
}

int Tt_OpponentFavor(const struct TtOpponent *o)
{
    // ordinary players about 55% for the player, the special ones about 47%, the CARD QUEEN about 45%
    return o->special == TT_SPECIAL_QUEEN ? -35 : o->special != TT_SPECIAL_NONE ? -20 : 35;
}

int Tt_FitOpponentHand(struct TtOpponent *o, int mine, uint32_t rules, unsigned day, uint16_t hand[TT_HAND])
{
    for (;;)
    {
        int target = mine + Tt_PlayerRating(rules) - Tt_OpponentFavor(o) - Tt_LevelRating(o->level, rules);
        if (o->special != TT_SPECIAL_NONE)
        {
            Tt_FitSpecialHand(o->special, day, target, hand);
            break;
        }
        Tt_FitRegionHand(o->region < TT_REGIONS ? o->region : 0, o->hash, o->tier, day, target, hand);
        // the cards they may carry this early fall well short: they play more carefully instead (SKILLED at most)
        if (target - Tt_HandPower(hand) <= 50 || o->level >= 2)
            break;
        o->level++;
    }
    return Tt_HandPower(hand);
}
