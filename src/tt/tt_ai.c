// Moemon Kai: the computer's Triple Triad. A search of the moves ahead (negamax with alpha-beta, Final Fantasy
// VIII's searchBestMoveStack did the same with its own depth), to the end of the game once few squares are left,
// scored by the cards held and how safe each card's open sides are. Fair: the other side's cards it can't see (no
// OPEN, not among THREE OPEN's, not yet played) are guessed, never read; and it slips like a person (ExpNeg below:
// near misses often, blunders rarely; beginners look one move ahead and slip more).
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "tt/duel.h"

#define COPY_LEN offsetof(struct TtDuel, ev)
#define WIN 100000

static int Eval(const struct TtDuel *d, unsigned seat)
{
    int score = ((int)TtDuel_Score(d, seat) - (int)TtDuel_Score(d, seat ^ 1)) * 100;
    unsigned cell, dir;
    if (d->over)
        return d->winner == 2 ? 0 : d->winner == seat ? WIN : -WIN;
    // An open side facing an empty square: a big number there keeps the card safe
    for (cell = 0; cell < TT_CELLS; cell++)
    {
        const struct TtCell *c = &d->cell[cell];
        int row = cell / 3, col = cell % 3, risk = 0;
        if (!c->card)
            continue;
        for (dir = 0; dir < 4; dir++)
        {
            int r = row + (dir == TT_DOWN) - (dir == TT_UP), k = col + (dir == TT_RIGHT) - (dir == TT_LEFT);
            if (r < 0 || r > 2 || k < 0 || k > 2 || d->cell[r * 3 + k].card)
                continue;
            risk += TT_RANK_A - TtDuel_Side(d, cell, dir);
        }
        score += c->owner == seat ? -risk * 2 : risk * 2;
    }
    return score;
}

static int Search(const struct TtDuel *d, int depth, int alpha, int beta, unsigned long *budget)
{
    unsigned seat = d->toMove, slot, cell, playable;
    int best = -WIN * 2, any = 0;
    struct TtDuel child;
    if (d->over || depth <= 0 || !*budget)
        return Eval(d, seat);
    playable = TtDuel_Playable(d);
    for (slot = 0; slot < TT_HAND; slot++)
    {
        unsigned prev;
        if (!(playable >> slot & 1))
            continue;
        // the same card in two slots plays alike: try it once
        for (prev = 0; prev < slot; prev++)
            if ((playable >> prev & 1) && d->hand[seat][prev] == d->hand[seat][slot])
                break;
        if (prev < slot)
            continue;
        for (cell = 0; cell < TT_CELLS; cell++)
        {
            int v;
            if (d->cell[cell].card)
                continue;
            memcpy(&child, d, COPY_LEN);
            child.noEvents = 1;
            TtDuel_Play(&child, slot, cell);
            if (*budget)
                (*budget)--;
            // (a new SUDDEN DEATH round or the end: score it from here)
            v = child.over || child.toMove == seat ? Eval(&child, seat)
                                                   : -Search(&child, depth - 1, -beta, -alpha, budget);
            any = 1;
            if (v > best)
                best = v;
            if (v > alpha)
                alpha = v;
            if (alpha >= beta)
                return best;
        }
    }
    return any ? best : Eval(d, seat);
}

/* A stand-in for a card it can't see: a plain card of that many stars (the most middling of them) */
static uint16_t Typical(unsigned stars)
{
    static uint16_t sTypical[TT_MAX_STARS + 1];
    unsigned id, s;
    if (stars < 1)
        stars = 1;
    if (stars > TT_MAX_STARS)
        stars = TT_MAX_STARS;
    if (!sTypical[1])
        for (s = 1; s <= TT_MAX_STARS; s++)
        {
            unsigned n = 0, total = 0, best = 0, bestScore = ~0u;
            for (id = 1; id <= TT_CARD_MAX; id++)
                if (Tt_Card(id) && Tt_Card(id)->stars == s)
                {
                    const uint8_t *v = Tt_Card(id)->side;
                    total += v[0] + v[1] + v[2] + v[3];
                    n++;
                }
            for (id = 1; n && id <= TT_CARD_MAX; id++)
                if (Tt_Card(id) && Tt_Card(id)->stars == s)
                {
                    const uint8_t *v = Tt_Card(id)->side;
                    unsigned sum = v[0] + v[1] + v[2] + v[3], hi = v[0], lo = v[0], k, score;
                    for (k = 1; k < 4; k++)
                    {
                        hi = v[k] > hi ? v[k] : hi;
                        lo = v[k] < lo ? v[k] : lo;
                    }
                    score = (unsigned)abs((int)(sum * n) - (int)total) * 4 + (hi - lo) * n;
                    if (score < bestScore)
                        bestScore = score, best = id;
                }
            sTypical[s] = (uint16_t)best;
        }
    return sTypical[stars];
}

/* What seat may know of the other hand: shown cards, played ones, the card SWAP gave away, every card of a SUDDEN
 * DEATH round (they were all on the board); the rest become stand-ins as strong as the cards seen so far */
void TtAi_View(const struct TtDuel *d, unsigned seat, struct TtDuel *view)
{
    unsigned o = seat ^ 1, slot, known = 0, stars = 0, n = 0;
    memcpy(view, d, COPY_LEN);
    view->noEvents = 1;
    if (d->round > 1)
        return;
    for (slot = 0; slot < TT_HAND; slot++)
        if ((d->shown[o] >> slot & 1) || (d->played[o] >> slot & 1) || d->swapped[o] == slot || !d->hand[o][slot])
            known |= 1u << slot;
    if (known == (1u << TT_HAND) - 1)
        return;
    for (slot = 0; slot < TT_HAND; slot++)
        if ((known >> slot & 1) && d->hand[o][slot] && Tt_Card(d->hand[o][slot]) && d->swapped[o] != slot)
            stars += Tt_Card(d->hand[o][slot])->stars, n++;
    if (!n)   // (nothing seen yet: about as strong as its own hand)
        for (slot = 0; slot < TT_HAND; slot++)
            if (Tt_Card(d->hand[seat][slot]))
                stars += Tt_Card(d->hand[seat][slot])->stars, n++;
    for (slot = 0; slot < TT_HAND; slot++)
        if (!(known >> slot & 1))
            view->hand[o][slot] = Typical(n ? (stars + n / 2) / n : 2);
}

/* Plausible slips: every move is weighed by how much worse it is than the best one found, e^(-loss / T), so a move
 * nearly as good is often taken, a real blunder hardly ever; the better the player, the smaller T (and the further it
 * looks ahead). One card held is 100. (tools/tt/rate_cards.sh measures what each level gives away.) */
static const uint16_t sTemper[4] = {170, 100, 70, 30};
/* e^-x in 1/65536, x = num / den (no libm in the game) */
static uint32_t ExpNeg(uint32_t num, uint32_t den)
{
    uint32_t x = (uint32_t)((uint64_t)num * 94548 / den);   /* x / ln 2, in 1/65536 */
    uint32_t whole = x >> 16, frac = x & 0xFFFF;
    if (whole >= 16)
        return 0;
    /* 2^-f ~ 1 - f/2 + f^2/8 (close enough for a weight) */
    {
        uint64_t f = frac, v = 65536 - f / 2 + (f * f >> 19);
        return (uint32_t)(v >> whole);
    }
}

void TtAi_Choose(const struct TtDuel *d, unsigned level, uint32_t salt, unsigned *slot, unsigned *cell)
{
    static const int sDepth[4] = {1, 2, 3, 3}, sEndgame[4] = {0, 3, 5, 6};
    struct Move
    {
        int v;
        uint8_t slot, cell;
    } moves[TT_HAND * TT_CELLS];
    unsigned seat = d->toMove, s, c, empty = 0, playable = TtDuel_Playable(d), n = 0, i, pick;
    int depth;
    uint32_t rng = salt ^ TtDuel_Hash(d);
    unsigned long budget = 400000;
    struct TtDuel view, child;
    if (level > 3)
        level = 3;
    TtAi_View(d, seat, &view);
    for (c = 0; c < TT_CELLS; c++)
        empty += !d->cell[c].card;
    depth = empty <= (unsigned)sEndgame[level] ? (int)empty : sDepth[level];
    *slot = *cell = 0xFF;
    for (s = 0; s < TT_HAND; s++)
    {
        if (!(playable >> s & 1))
            continue;
        for (c = 0; c < TT_CELLS; c++)
        {
            int v;
            if (d->cell[c].card)
                continue;
            memcpy(&child, &view, COPY_LEN);
            TtDuel_Play(&child, s, c);
            v = child.over || child.toMove == seat || depth <= 1
                    ? Eval(&child, seat)
                    : -Search(&child, depth - 1, -WIN * 2, WIN * 2, &budget);
            moves[n].v = v;
            moves[n].slot = (uint8_t)s;
            moves[n++].cell = (uint8_t)c;
        }
    }
    if (!n)
        return;
    {
        uint64_t total = 0, roll;
        uint32_t w[TT_HAND * TT_CELLS];
        int top = moves[0].v;
        for (i = 1; i < n; i++)
            top = moves[i].v > top ? moves[i].v : top;
        for (i = 0; i < n; i++)
            total += w[i] = ExpNeg((uint32_t)(top - moves[i].v), sTemper[level]) + 1;
        roll = ((uint64_t)Tt_Rand(&rng) << 16 ^ Tt_Rand(&rng)) % total;
        for (pick = 0; pick + 1 < n && roll >= w[pick]; pick++)
            roll -= w[pick];
    }
    *slot = moves[pick].slot;
    *cell = moves[pick].cell;
}
