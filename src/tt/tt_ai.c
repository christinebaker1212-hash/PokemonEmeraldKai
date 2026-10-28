// Moemon Kai: the computer's Triple Triad. A search of the moves ahead (negamax with alpha-beta, Final Fantasy
// VIII's searchBestMoveStack did the same with its own depth), to the end of the game once few squares are left,
// scored by the cards held and how safe each card's open sides are. Beginners look one move ahead and slip.
#include <limits.h>
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

void TtAi_Choose(const struct TtDuel *d, unsigned level, uint32_t salt, unsigned *slot, unsigned *cell)
{
    static const int sDepth[4] = {1, 2, 3, 3}, sEndgame[4] = {0, 3, 5, 6}, sNoise[4] = {160, 60, 15, 0};
    unsigned seat = d->toMove, s, c, empty = 0, playable = TtDuel_Playable(d);
    int best = INT_MIN, depth;
    uint32_t rng = salt ^ TtDuel_Hash(d);
    unsigned long budget = 400000;
    struct TtDuel child;
    if (level > 3)
        level = 3;
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
            memcpy(&child, d, COPY_LEN);
            child.noEvents = 1;
            TtDuel_Play(&child, s, c);
            v = child.over || child.toMove == seat || depth <= 1
                    ? Eval(&child, seat)
                    : -Search(&child, depth - 1, -WIN * 2, WIN * 2, &budget);
            if (sNoise[level])
                v += (int)(Tt_Rand(&rng) % (unsigned)(sNoise[level] + 1));
            else
                v = v * 8 + (int)(Tt_Rand(&rng) % 4);   // (ties broken, not always the same corner)
            if (v > best)
                best = v, *slot = s, *cell = c;
        }
    }
}
