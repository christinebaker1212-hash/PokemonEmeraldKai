// Moemon Kai: the hands Triple Triad players bring. A region's players draw from their region's cards
// (src/tt/tt_pools.c), as strong as the player's badges allow, each person starting somewhere else in the list and
// moving on by a hand a day: every card of the region turns up in time. Special players bring one of their own
// five-star cards (a different one each day), a four-star card and three of three stars. Final Fantasy XIV's deck
// limits hold for every hand.
#include <string.h>
#include "tt/npc.h"

static unsigned Count(const uint16_t *list) { unsigned n = 0; while (list[n]) n++; return n; }
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
