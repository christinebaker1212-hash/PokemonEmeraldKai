// Moemon Kai: Triple Triad's rules. The capture rules follow Final Fantasy VIII's own code (its tripletriad overlay,
// as the ff8-decomp project reads it: resetTriadBoard, placeCard, applySameRule, applyPlusRule, applyBasicCapture
// and applyCardRules): SAME (with SAME WALL) and PLUS look first, then the plain capture; ELEMENTAL's +1 / -1 counts
// for the plain capture only; the cards SAME or PLUS took start a COMBO: a plain capture from each of them, then from
// each card that took, again and again while any card is taken (the placed card's own plain captures never combo). XIV's rules sit beside them: REVERSE and FALLEN ACE
// for the plain capture, ASCENSION and DESCENSION for every rule, ORDER, CHAOS, SWAP and THREE OPEN.
// Deterministic: the same seed and moves always play out the same (friends' games send only the moves).
#include <string.h>
#include "tt/duel.h"

enum { CF_JUST = 1, CF_TAKEN = 2 };   // placed or taken this step; taken in this play (for the COMBO)

uint32_t Tt_Rand(uint32_t *state)
{
    uint32_t x = *state ? *state : 0x9e3779b9u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static int Has(const struct TtDuel *d, unsigned rule) { return (d->rules >> rule) & 1; }

static int Neighbor(unsigned cell, unsigned dir)
{
    int row = cell / 3, col = cell % 3;
    switch (dir)
    {
    case TT_UP: row--; break;
    case TT_RIGHT: col++; break;
    case TT_DOWN: row++; break;
    default: col--; break;
    }
    return row < 0 || row > 2 || col < 0 || col > 2 ? -1 : row * 3 + col;
}

static void Emit(struct TtDuel *d, unsigned kind, unsigned cell, unsigned dir, unsigned by, unsigned wave)
{
    struct TtEvent *e;
    if (d->noEvents || d->evCount >= TT_MAX_EVENTS)
        return;
    e = &d->ev[d->evCount++];
    memset(e, 0, sizeof(*e));
    e->kind = (uint8_t)kind;
    e->cell = (uint8_t)cell;
    e->dir = (uint8_t)dir;
    e->by = (uint8_t)by;
    e->wave = (uint8_t)wave;
    if (cell < TT_CELLS)
    {
        e->card = d->cell[cell].card;
        e->seat = d->cell[cell].owner;
    }
}

int TtDuel_TypeShift(const struct TtDuel *d, unsigned type)
{
    if (!type || type >= TT_TYPE_COUNT)
        return 0;
    if (Has(d, TT_RULE_ASCENSION))
        return d->placedTypes[type];
    if (Has(d, TT_RULE_DESCENSION))
        return -(int)d->placedTypes[type];
    return 0;
}

int TtDuel_Side(const struct TtDuel *d, unsigned cell, unsigned side)
{
    const struct TtCard *c = Tt_Card(d->cell[cell].card);
    int v;
    if (!c)
        return 0;
    v = c->side[side] + TtDuel_TypeShift(d, c->type);
    return v < 1 ? 1 : v > TT_RANK_A ? TT_RANK_A : v;
}

static void Take(struct TtDuel *d, unsigned cell, unsigned seat, unsigned dir, unsigned by, unsigned wave)
{
    d->cell[cell].owner = (uint8_t)seat;
    d->cell[cell].flags |= CF_TAKEN;
    Emit(d, TTE_FLIP, cell, dir, by, wave);
}

// SAME (applySameRule): two or more sides equal to the facing sides (SAME WALL: an A against the edge counts);
// then every facing card of the other side whose number is not above ours is taken. Returns how many.
static unsigned Same(struct TtDuel *d, unsigned cell)
{
    unsigned dir, matches = 0, taken = 0, seat = d->cell[cell].owner;
    for (dir = 0; dir < 4; dir++)
    {
        int n = Neighbor(cell, dir);
        if (n < 0)
        {
            if (Has(d, TT_RULE_SAME_WALL) && TtDuel_Side(d, cell, dir) == TT_RANK_A)
                matches++;
            continue;
        }
        if (d->cell[n].card && TtDuel_Side(d, cell, dir) == TtDuel_Side(d, n, (dir + 2) & 3))
            matches++;
    }
    if (matches < 2)
        return 0;
    for (dir = 0; dir < 4; dir++)
    {
        int n = Neighbor(cell, dir);
        if (n < 0 || !d->cell[n].card || d->cell[n].owner == seat)
            continue;
        if (TtDuel_Side(d, cell, dir) < TtDuel_Side(d, n, (dir + 2) & 3))
            continue;
        if (!taken)
            Emit(d, TTE_RULE, cell, 0, TT_BY_SAME, 0);
        Take(d, n, seat, (dir + 2) & 3, TT_BY_SAME, 0);
        taken++;
    }
    return taken;
}

// PLUS (applyPlusRule): the sums of each side and its neighbor's facing side; the sum hit most often (the first
// to reach that count), when hit twice or more, takes those neighbors that are the other side's.
static unsigned Plus(struct TtDuel *d, unsigned cell)
{
    uint8_t count[21] = {0}, mask[21] = {0};
    unsigned dir, best = 0, bestSum = 0, taken = 0, seat = d->cell[cell].owner;
    for (dir = 0; dir < 4; dir++)
    {
        int n = Neighbor(cell, dir);
        unsigned sum;
        if (n < 0 || !d->cell[n].card)
            continue;
        sum = TtDuel_Side(d, cell, dir) + TtDuel_Side(d, n, (dir + 2) & 3);
        count[sum]++;
        mask[sum] |= 1 << dir;
        if (count[sum] > best)
            best = count[sum], bestSum = sum;
    }
    if (best < 2)
        return 0;
    for (dir = 0; dir < 4; dir++)
    {
        int n = Neighbor(cell, dir);
        if (n < 0 || !(mask[bestSum] >> dir & 1) || d->cell[n].owner == seat)
            continue;
        if (!taken)
            Emit(d, TTE_RULE, cell, 0, TT_BY_PLUS, 0);
        Take(d, n, seat, (dir + 2) & 3, TT_BY_PLUS, 0);
        taken++;
    }
    return taken;
}

// The plain capture (applyBasicCapture) from every card placed or taken this step
static unsigned Plain(struct TtDuel *d, unsigned by, unsigned wave)
{
    unsigned cell, dir, taken = 0;
    uint8_t from[TT_CELLS];
    for (cell = 0; cell < TT_CELLS; cell++)
        from[cell] = d->cell[cell].flags & CF_JUST;
    for (cell = 0; cell < TT_CELLS; cell++)
    {
        unsigned seat = d->cell[cell].owner;
        if (!from[cell])
            continue;
        for (dir = 0; dir < 4; dir++)
        {
            int n = Neighbor(cell, dir), mine, theirs;
            if (n < 0 || !d->cell[n].card || d->cell[n].owner == seat)
                continue;
            mine = TtDuel_Side(d, cell, dir) + d->cell[cell].elem;
            theirs = TtDuel_Side(d, n, (dir + 2) & 3) + d->cell[n].elem;
            if (Has(d, TT_RULE_FALLEN_ACE) && (Has(d, TT_RULE_REVERSE) ? mine == TT_RANK_A && theirs == 1
                                                                        : mine == 1 && theirs == TT_RANK_A))
                ;   // the ace falls
            else if (Has(d, TT_RULE_REVERSE) ? mine >= theirs : mine <= theirs)
                continue;
            Take(d, n, seat, (dir + 2) & 3, by, wave);
            taken++;
        }
    }
    return taken;
}

static void PickForced(struct TtDuel *d)
{
    unsigned s, left = 0, pick;
    d->forced = 0xFF;
    for (s = 0; s < TT_HAND; s++)
        left += !(d->played[d->toMove] >> s & 1);
    if (!left)
        return;
    if (Has(d, TT_RULE_ORDER))
    {
        for (s = 0; s < TT_HAND; s++)
            if (!(d->played[d->toMove] >> s & 1))
                break;
        d->forced = (uint8_t)s;
    }
    else if (Has(d, TT_RULE_CHAOS))
    {
        pick = Tt_Rand(&d->rng) % left;
        for (s = 0; s < TT_HAND; s++)
            if (!(d->played[d->toMove] >> s & 1) && !pick--)
                break;
        d->forced = (uint8_t)s;
    }
}

static void SetupRound(struct TtDuel *d)
{
    unsigned i, seat;
    memset(d->cell, 0, sizeof(d->cell));
    memset(d->placedTypes, 0, sizeof(d->placedTypes));
    d->played[0] = d->played[1] = 0;
    d->turn = 0;
    // ELEMENTAL (resetTriadBoard): each square has a 30% chance of a type
    if (Has(d, TT_RULE_ELEMENTAL))
        for (i = 0; i < TT_CELLS; i++)
            if (Tt_Rand(&d->rng) % 100 < 30)
                d->cell[i].tile = (uint8_t)(1 + Tt_Rand(&d->rng) % (TT_TYPE_COUNT - 1));
    // What each side sees of the other's hand
    for (seat = 0; seat < 2; seat++)
    {
        d->shown[seat] = 0;
        if (Has(d, TT_RULE_OPEN))
            d->shown[seat] = (1 << TT_HAND) - 1;
        else if (Has(d, TT_RULE_THREE_OPEN))
            while (__builtin_popcount(d->shown[seat]) < 3)
                d->shown[seat] |= 1 << (Tt_Rand(&d->rng) % TT_HAND);
    }
    d->toMove = d->first;
    PickForced(d);
}

void TtDuel_Start(struct TtDuel *d, uint32_t rules, unsigned trade, uint32_t seed, const uint16_t *hand0,
                  const uint16_t *hand1, unsigned first)
{
    unsigned i;
    memset(d, 0, sizeof(*d));
    d->rules = rules;
    d->trade = (uint8_t)(trade < TT_TRADE_COUNT ? trade : TT_TRADE_ONE);
    d->seed = seed;
    d->rng = seed ? seed : 1;
    for (i = 0; i < TT_HAND; i++)
    {
        d->hand[0][i] = hand0[i];
        d->hand[1][i] = hand1[i];
        d->origin[0][i] = 0;
        d->origin[1][i] = 1;
    }
    d->swapped[0] = d->swapped[1] = 0xFF;
    // SWAP: one card of each hand, chosen at random, changes sides for the game
    if (Has(d, TT_RULE_SWAP))
    {
        unsigned a = Tt_Rand(&d->rng) % TT_HAND, b = Tt_Rand(&d->rng) % TT_HAND;
        uint16_t c = d->hand[0][a];
        d->hand[0][a] = d->hand[1][b];
        d->hand[1][b] = c;
        d->origin[0][a] = 1;
        d->origin[1][b] = 0;
        d->swapped[0] = (uint8_t)a;
        d->swapped[1] = (uint8_t)b;
    }
    d->first = (uint8_t)(first & 1);
    d->round = 1;
    SetupRound(d);
}

unsigned TtDuel_Playable(const struct TtDuel *d)
{
    unsigned left;
    if (d->over)
        return 0;
    left = ~d->played[d->toMove] & ((1 << TT_HAND) - 1);
    if (d->forced < TT_HAND)
        return left & (1u << d->forced);
    return left;
}

int TtDuel_CanPlay(const struct TtDuel *d, unsigned slot, unsigned cell)
{
    return slot < TT_HAND && cell < TT_CELLS && (TtDuel_Playable(d) >> slot & 1) && !d->cell[cell].card &&
           d->hand[d->toMove][slot];
}

unsigned TtDuel_Score(const struct TtDuel *d, unsigned seat)
{
    unsigned i, n = 0;
    for (i = 0; i < TT_CELLS; i++)
        n += d->cell[i].card && d->cell[i].owner == seat;
    for (i = 0; i < TT_HAND; i++)
        n += !(d->played[seat] >> i & 1) && d->hand[seat][i];
    return n;
}

// The round is over: a winner, or SUDDEN DEATH deals each side the cards it holds and plays again
static void EndRound(struct TtDuel *d)
{
    unsigned a = TtDuel_Score(d, 0), b = TtDuel_Score(d, 1);
    if (a == b && Has(d, TT_RULE_SUDDEN_DEATH) && d->round < TT_MAX_ROUNDS)
    {
        uint16_t hand[2][TT_HAND];
        uint8_t origin[2][TT_HAND], n[2] = {0, 0};
        unsigned i, seat;
        for (i = 0; i < TT_CELLS; i++)
        {
            seat = d->cell[i].owner;
            if (d->cell[i].card && n[seat] < TT_HAND)
            {
                hand[seat][n[seat]] = d->cell[i].card;
                origin[seat][n[seat]++] = d->cell[i].origin;
            }
        }
        for (seat = 0; seat < 2; seat++)
            for (i = 0; i < TT_HAND; i++)
                if (!(d->played[seat] >> i & 1) && d->hand[seat][i] && n[seat] < TT_HAND)
                {
                    hand[seat][n[seat]] = d->hand[seat][i];
                    origin[seat][n[seat]++] = d->origin[seat][i];
                }
        memcpy(d->hand, hand, sizeof(hand));
        memcpy(d->origin, origin, sizeof(origin));
        d->round++;
        d->first ^= 1;   // the other side starts the new round
        SetupRound(d);
        Emit(d, TTE_ROUND, 0xFF, 0, 0, 0);
        return;
    }
    d->over = 1;
    d->winner = a > b ? 0 : b > a ? 1 : 2;
    Emit(d, TTE_END, 0xFF, 0, 0, 0);
    if (!d->noEvents && d->evCount)
        d->ev[d->evCount - 1].seat = d->winner;
}

int TtDuel_Play(struct TtDuel *d, unsigned slot, unsigned cell)
{
    struct TtCell *c;
    const struct TtCard *card;
    unsigned i, seat = d->toMove, combo = 0, wave;
    if (!TtDuel_CanPlay(d, slot, cell))
        return 0;
    d->evCount = 0;
    card = Tt_Card(d->hand[seat][slot]);
    for (i = 0; i < TT_CELLS; i++)
        d->cell[i].flags = 0;
    c = &d->cell[cell];
    c->card = d->hand[seat][slot];
    c->owner = (uint8_t)seat;
    c->origin = d->origin[seat][slot];
    c->flags = CF_JUST;
    // ELEMENTAL (placeCard): a typed square gives its own type +1 and anything else -1
    if (Has(d, TT_RULE_ELEMENTAL) && c->tile)
        c->elem = card && card->type == c->tile ? 1 : -1;
    d->played[seat] |= 1 << slot;
    if (!d->noEvents && d->evCount < TT_MAX_EVENTS)
    {
        Emit(d, TTE_PLACE, cell, 0, 0, 0);
        d->ev[d->evCount - 1].slot = (uint8_t)slot;
    }
    // ASCENSION / DESCENSION: the type's count goes up as the card lands
    if (card && card->type && (Has(d, TT_RULE_ASCENSION) || Has(d, TT_RULE_DESCENSION)))
    {
        d->placedTypes[card->type]++;
        if (!d->noEvents && d->evCount < TT_MAX_EVENTS)
        {
            Emit(d, TTE_SHIFT, cell, 0, 0, 0);
            d->ev[d->evCount - 1].amount = (int8_t)TtDuel_TypeShift(d, card->type);
            d->ev[d->evCount - 1].dir = card->type;
        }
    }
    // applyCardRules: SAME and PLUS, then the plain capture
    if (Has(d, TT_RULE_SAME))
        combo += Same(d, cell);
    if (Has(d, TT_RULE_PLUS))
        combo += Plus(d, cell);
    {
        uint8_t ruled[TT_CELLS];
        for (i = 0; i < TT_CELLS; i++)
        {
            ruled[i] = d->cell[i].flags & CF_TAKEN;   /* (what SAME / PLUS took: the COMBO starts there) */
            d->cell[i].flags &= (uint8_t)~CF_TAKEN;
        }
        Plain(d, TT_BY_PLAIN, 0);
        for (i = 0; i < TT_CELLS; i++)
            d->cell[i].flags = (uint8_t)((d->cell[i].flags & ~CF_TAKEN) | ruled[i]);
    }
    // The COMBO: each card taken by SAME or PLUS takes from where it lies, then each card that took, while anything
    // is taken
    for (wave = 1; combo; wave++)
    {
        unsigned any = 0;
        for (i = 0; i < TT_CELLS; i++)
        {
            d->cell[i].flags = d->cell[i].flags & CF_TAKEN ? CF_JUST : 0;   /* (only the last wave's cards) */
            any |= d->cell[i].flags;
        }
        if (!any)
            break;
        {
            unsigned before = d->evCount;
            if (wave == 1)
                Emit(d, TTE_RULE, cell, 0, TT_BY_COMBO, 1);
            if (!Plain(d, TT_BY_COMBO, wave))
            {
                d->evCount = (uint8_t)before;   // (nothing taken: no COMBO to show)
                break;
            }
        }
    }
    for (i = 0; i < TT_CELLS; i++)
        d->cell[i].flags = 0;
    d->turn++;
    if (d->turn >= TT_CELLS)
    {
        EndRound(d);
        return 1;
    }
    d->toMove ^= 1;
    PickForced(d);
    return 1;
}

uint32_t TtDuel_Hash(const struct TtDuel *d)
{
    uint32_t h = 2166136261u;
    unsigned i, s;
#define MIX(v) (h = (h ^ (uint32_t)(v)) * 16777619u)
    MIX(d->rules);
    MIX(d->rng);
    MIX(d->turn | d->toMove << 8 | d->round << 16 | d->over << 24);
    for (i = 0; i < TT_CELLS; i++)
        MIX(d->cell[i].card | d->cell[i].owner << 16 | d->cell[i].tile << 20);
    for (s = 0; s < 2; s++)
    {
        MIX(d->played[s]);
        for (i = 0; i < TT_HAND; i++)
            MIX(d->hand[s][i]);
    }
#undef MIX
    return h;
}

unsigned TtDuel_SpoilCount(const struct TtDuel *d, unsigned winner)
{
    int diff = (int)TtDuel_Score(d, winner) - (int)TtDuel_Score(d, winner ^ 1);
    if (!d->over || d->winner != winner)
        return 0;
    switch (d->trade)
    {
    case TT_TRADE_DIFF:
        return diff < 1 ? 1 : diff > TT_HAND ? TT_HAND : (unsigned)diff;
    case TT_TRADE_ALL:
        return TT_HAND;
    case TT_TRADE_DIRECT:
        return 0;   // (the cards themselves: TtDuel_DirectMask)
    default:
        return 1;
    }
}

unsigned TtDuel_DirectMask(const struct TtDuel *d, unsigned winner, const uint16_t *dealt)
{
    unsigned i, k, mask = 0;
    for (i = 0; i < TT_CELLS; i++)
    {
        const struct TtCell *c = &d->cell[i];
        if (!c->card || c->owner != winner || c->origin == winner)
            continue;
        for (k = 0; k < TT_HAND; k++)
            if (dealt[k] == c->card && !(mask >> k & 1))
            {
                mask |= 1 << k;
                break;
            }
    }
    return mask;
}

int Tt_DealRandom(const uint8_t *owned, uint32_t seed, uint16_t *hand)
{
    unsigned id, total = 0, n = 0, k;
    uint32_t rng = seed ? seed : 7;
    for (id = 1; id <= TT_CARD_MAX; id++)
        total += owned[id] && Tt_Card(id);
    if (total < TT_HAND)
        return 0;
    while (n < TT_HAND)
    {
        unsigned pick = Tt_Rand(&rng) % total;
        for (id = 1; id <= TT_CARD_MAX; id++)
            if (owned[id] && Tt_Card(id) && !pick--)
                break;
        for (k = 0; k < n && hand[k] != id; k++)
            ;
        if (k == n)
            hand[n++] = (uint16_t)id;
    }
    return 1;
}
