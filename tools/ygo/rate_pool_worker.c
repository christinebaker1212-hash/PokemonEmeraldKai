/* The generic duelists' decks rated by duels (tools/ygo/rate_pool.py; docs/YGO_DECISIONS.md "NPC decks from
 * YGOPRODeck"): a worker holding every deck of the pool, playing the duels it is given, AI against AI on EDOPro's
 * engine. Reads the pool (a line a deck: Main Deck ids, "|", Extra Deck ids), then lines "A B SEED FIRST" on stdin;
 * prints "A B RESULT TURNS" for each (RESULT 0: A won, 1: B won, 2: a draw), flushed, until stdin ends.
 *   rate_pool_worker POOL.txt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ygo/ygo_internal.h"

static uint16_t (*sMain)[YGO_DECK_MAX];
static uint16_t (*sExtra)[YGO_FUSION_MAX];

static int Duel(unsigned a, unsigned b, uint32_t seed, unsigned first, unsigned *turns)
{
    static struct YgoDuel d;
    unsigned steps = 0;
    *turns = 0;
    if (!YgoDuel_Init(&d, sMain[a], sExtra[a], sMain[b], sExtra[b], seed, first))
        return 2;
    while (d.query.type != YGO_Q_NONE && steps++ < 6000 && d.s.turn < 60)
    {
        struct YgoAnswer ans;
        YgoDuel_AiAnswer(&d, &ans);
        if (!YgoDuel_Answer(&d, &ans))
            break;
    }
    *turns = d.s.turn;
    if (!d.s.over || d.s.winner > 1)   /* (no end: the one with more LP) */
        return d.s.players[0].lp > d.s.players[1].lp ? 0 : d.s.players[1].lp > d.s.players[0].lp ? 1 : 2;
    return d.s.winner;
}

int main(int argc, char **argv)
{
    FILE *f;
    static char line[16384];
    unsigned n = 0, cap = 1024;
    if (argc < 2 || !(f = fopen(argv[1], "r")))
        return 1;
    sMain = calloc(cap, sizeof(*sMain));
    sExtra = calloc(cap, sizeof(*sExtra));
    while (fgets(line, sizeof(line), f))
    {
        char *p = line, *end;
        unsigned m = 0, x = 0, inExtra = 0;
        if (n == cap)
        {
            cap *= 2;
            sMain = realloc(sMain, cap * sizeof(*sMain));
            sExtra = realloc(sExtra, cap * sizeof(*sExtra));
        }
        memset(sMain[n], 0, sizeof(sMain[n]));
        memset(sExtra[n], 0, sizeof(sExtra[n]));
        for (;;)
        {
            unsigned long id;
            while (*p == ' ')
                p++;
            if (*p == '|')
            {
                inExtra = 1;
                p++;
                continue;
            }
            id = strtoul(p, &end, 10);
            if (end == p)
                break;
            p = end;
            if (!Ygo_Card((unsigned)id))
                continue;
            if (inExtra && x < YGO_FUSION_MAX)
                sExtra[n][x++] = (uint16_t)id;
            else if (!inExtra && m < YGO_DECK_MAX)
                sMain[n][m++] = (uint16_t)id;
        }
        n++;
    }
    fclose(f);
    fprintf(stderr, "[pool] %u decks\n", n);
    while (fgets(line, sizeof(line), stdin))
    {
        unsigned a, b, seed, first, turns;
        int r;
        if (sscanf(line, "%u %u %u %u", &a, &b, &seed, &first) != 4 || a >= n || b >= n)
            continue;
        r = Duel(a, b, seed, first, &turns);
        printf("%u %u %d %u\n", a, b, r, turns);
        fflush(stdout);
    }
    return 0;
}
