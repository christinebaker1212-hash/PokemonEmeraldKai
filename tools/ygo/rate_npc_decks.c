/* Ratings for the opponents' decks (tools/ygo/rate_npc_decks.sh): AI-against-AI duels on EDOPro's engine, then a
 * Bradley-Terry fit of every result at once (as the POKeMON card game's tools/tcg/rate_decks.c), as Elo with the
 * average deck at 1500: the order the duels come in no longer matters (a running Elo update drifted with it).
 * Each round every deck meets a different opponent (a fixed stride round the list, so the pairings spread evenly),
 * going first on alternate rounds. Deterministic from the deck list.
 * usage: rate_npc_decks DECKS.txt ROUNDS  (a line per deck: card ids separated by spaces; prints a rating per line) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../src/ygo/ygo_internal.h"

#define MAX_DECKS 640
static uint16_t sMain[MAX_DECKS][YGO_DECK_MAX], sExtra[MAX_DECKS][YGO_FUSION_MAX];
static double sElo[MAX_DECKS];
static unsigned sWins[MAX_DECKS], sGames[MAX_DECKS];
static float sScore[MAX_DECKS][MAX_DECKS];      /* i's points against j (a win 1, a draw 0.5) */
static uint16_t sMet[MAX_DECKS][MAX_DECKS];     /* duels between i and j */

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
    if (getenv("RATE_ONE"))
    {
        /* RATE_ONE="A B SEED FIRST": just that duel (the trace's line, to look into one) */
        unsigned a, b, seed, first;
        if (sscanf(getenv("RATE_ONE"), "%u %u %u %u", &a, &b, &seed, &first) == 4 && a < n && b < n)
            printf("winner %d\n", Duel(a, b, seed, first));
        return 0;
    }
    for (r = 0; r < rounds; r++)
        for (i = 0; i < n; i++)
        {
            unsigned j, w;
            /* (a stride that changes each round: the opponents spread over the list, never the deck itself) */
            rng = rng * 1103515245u + 12345u;
            j = (i + 1 + (r * 97u + (rng >> 16) % 7u) % (n - 1)) % n;
            if (getenv("RATE_TRACE"))
                fprintf(stderr, "duel r%u %u vs %u seed %u first %u\n", r, i, j, rng ^ (r * 7919u), r & 1);
            w = (unsigned)Duel(i, j, rng ^ (r * 7919u), r & 1);
            sScore[i][j] += w == 0 ? 1 : w == 2 ? 0.5f : 0;
            sScore[j][i] += w == 1 ? 1 : w == 2 ? 0.5f : 0;
            sMet[i][j]++, sMet[j][i]++;
            sWins[i] += w == 0;
            sWins[j] += w == 1;
            sGames[i]++;
            sGames[j]++;
        }
    /* Bradley-Terry by its usual fixed-point steps; each deck also holds a draw against an average one (the prior
       that keeps a deck that won or lost everything finite) */
    {
        static double strength[MAX_DECKS];
        double mean = 0;
        unsigned it;
        for (i = 0; i < n; i++)
            strength[i] = 1;
        for (it = 0; it < 400; it++)
            for (i = 0; i < n; i++)
            {
                double won = 0.5, d = 1 / (strength[i] + 1);
                unsigned j;
                for (j = 0; j < n; j++)
                    if (sMet[i][j])
                        won += sScore[i][j], d += sMet[i][j] / (strength[i] + strength[j]);
                strength[i] = won / d;
            }
        for (i = 0; i < n; i++)
            mean += 400 * log10(strength[i]);
        mean /= n;
        for (i = 0; i < n; i++)
            sElo[i] = 1500 + 400 * log10(strength[i]) - mean;
    }
    for (i = 0; i < n; i++)
        printf("%d %u %u\n", (int)lround(sElo[i]), sWins[i], sGames[i]);
    return 0;
}
