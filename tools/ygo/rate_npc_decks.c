/* Ratings for the opponents' decks (tools/ygo/rate_npc_decks.sh): AI-against-AI duels on EDOPro's engine, rounds
 * of pairings, Elo updated after each duel (both seats, both going first). Deterministic from the deck list.
 * usage: rate_npc_decks DECKS.txt ROUNDS  (a line per deck: card ids separated by spaces; prints a rating per line) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../src/ygo/ygo_internal.h"

#define MAX_DECKS 400
static uint16_t sMain[MAX_DECKS][YGO_DECK_MAX], sExtra[MAX_DECKS][YGO_FUSION_MAX];
static double sElo[MAX_DECKS];
static unsigned sWins[MAX_DECKS], sGames[MAX_DECKS];

/* 0: the first deck won, 1: the second, 2: a draw */
static int Duel(unsigned a, unsigned b, uint32_t seed, unsigned first)
{
    static struct YgoDuel d;
    unsigned steps = 0;
    if (!YgoDuel_Init(&d, sMain[a], sExtra[a], sMain[b], sExtra[b], seed, first))
        return 2;
    while (d.query.type != YGO_Q_NONE && steps++ < 4000 && d.s.turn < 80)
    {
        struct YgoAnswer ans;
        YgoDuel_AiAnswer(&d, &ans);
        if (!YgoDuel_Answer(&d, &ans))
            break;
    }
    if (!d.s.over || d.s.winner > 1)
    {
        /* (no end: the one with more LP) */
        return d.s.players[0].lp > d.s.players[1].lp ? 0 : d.s.players[1].lp > d.s.players[0].lp ? 1 : 2;
    }
    return d.s.winner;
}
int main(int argc, char **argv)
{
    FILE *f;
    char line[8192];
    unsigned n = 0, rounds = argc > 2 ? (unsigned)atoi(argv[2]) : 20, r, i;
    uint32_t rng = 12345;
    if (argc < 2 || !(f = fopen(argv[1], "r")))
        return 1;
    while (n < MAX_DECKS && fgets(line, sizeof(line), f))
    {
        char *p = line, *end;
        unsigned m = 0, x = 0;
        memset(sMain[n], 0, sizeof(sMain[n]));
        memset(sExtra[n], 0, sizeof(sExtra[n]));
        for (;;)
        {
            unsigned long id = strtoul(p, &end, 10);
            const struct YgoCard *c;
            if (end == p)
                break;
            p = end;
            c = Ygo_Card((unsigned)id);
            if (!c)
                continue;
            if (c->frame == YGO_FUSION && x < YGO_FUSION_MAX)
                sExtra[n][x++] = (uint16_t)id;
            else if (c->frame != YGO_FUSION && m < YGO_DECK_MAX)
                sMain[n][m++] = (uint16_t)id;
        }
        sElo[n] = 1500;
        n++;
    }
    fclose(f);
    for (r = 0; r < rounds; r++)
        for (i = 0; i < n; i++)
        {
            unsigned j, w;
            double ea;
            rng = rng * 1103515245u + 12345u;
            j = (rng >> 8) % n;
            if (j == i)
                continue;
            w = (unsigned)Duel(i, j, rng ^ (r * 7919u), r & 1);
            ea = 1 / (1 + pow(10, (sElo[j] - sElo[i]) / 400));
            sElo[i] += 24 * ((w == 0 ? 1 : w == 2 ? 0.5 : 0) - ea);
            sElo[j] -= 24 * ((w == 0 ? 1 : w == 2 ? 0.5 : 0) - ea);
            sWins[i] += w == 0;
            sWins[j] += w == 1;
            sGames[i]++;
            sGames[j]++;
        }
    for (i = 0; i < n; i++)
        printf("%d %u %u\n", (int)lround(sElo[i]), sWins[i], sGames[i]);
    return 0;
}
