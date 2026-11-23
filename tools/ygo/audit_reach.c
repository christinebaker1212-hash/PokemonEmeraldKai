/* Which of the engine's offers the duel table can't reach (the user, 2026-11-23: "There may be other effects that
 * should be able to trigger that don't. Find them"): AI duels between the generic duelists' decks (the pool file
 * tools/ygo/rate_pool.py writes: a line a deck, Main Deck ids, "|", Extra Deck ids). At every Main / Battle Phase
 * question, each offer is checked against what ygo_board.c can open: a card in the hand or on the field (its spot), in
 * the player's Graveyard, face-up Banished or Extra Deck (the pile, opened on it), the Deck (its menu), or the phase
 * menu. A chain chance
 * the table passes by itself (YgoDuel_QuietChance with no monster effect and nothing meant for the moment: ygo_board.c) is counted too, with what it
 * offered. Prints a line per kind of offer that couldn't be reached, most first.
 *   audit_reach POOL.txt DUELS [SEED] */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ygo/ygo_internal.h"

static uint16_t (*sMain)[YGO_DECK_MAX];
static uint16_t (*sExtra)[YGO_FUSION_MAX];

#define KINDS 4096
static struct
{
    char what[96];
    unsigned count;
    uint16_t card;
} sMiss[KINDS];
static unsigned sMissCount, sOffers, sQuiet, sQuietOffers;

static void Note(const char *what, unsigned card)
{
    unsigned i;
    for (i = 0; i < sMissCount; i++)
        if (!strcmp(sMiss[i].what, what))
        {
            sMiss[i].count++;
            return;
        }
    if (sMissCount < KINDS)
    {
        snprintf(sMiss[sMissCount].what, sizeof(sMiss[0].what), "%s", what);
        sMiss[sMissCount].count = 1;
        sMiss[sMissCount++].card = (uint16_t)card;
    }
}

static const char *const sLoc[] = {"NONE", "DECK", "HAND", "MZONE", "SZONE", "GRAVE", "BANISHED", "EXTRA"};
static const char *const sKind[] = {"SUMMON", "SET_MONSTER", "FLIP_SUMMON", "POSITION", "ACTIVATE", "SET_SPELL",
                                    "SPECIAL", "TO_BATTLE", "TO_MAIN2", "END_TURN", "ATTACK", "CONCEDE"};

static void Check(const struct YgoDuel *d)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoState *s = &d->s;
    unsigned i;
    if (q->type == YGO_Q_CHAIN && YgoDuel_QuietChance(d))
    {
        int monster = 0;
        for (i = 0; i < q->count; i++)
        {
            unsigned o = q->items[i].obj, loc = o < s->objCount ? s->objs[o].location : YGO_LOC_NONE;
            monster |= Ygo_IsMonster(Ygo_Card(q->items[i].card)) || loc == YGO_LOC_GRAVE || loc == YGO_LOC_BANISHED ||
                       loc == YGO_LOC_EXTRA;
        }
        if (!monster && q->count && !q->meant)
        {
            char what[96];
            sQuiet++;
            for (i = 0; i < q->count; i++)
            {
                unsigned o = q->items[i].obj;
                const struct YgoCard *c = Ygo_Card(q->items[i].card);
                sQuietOffers++;
                snprintf(what, sizeof(what), "quiet pass: %s in %s (%s)", c ? c->name : "?",
                         o < s->objCount ? sLoc[s->objs[o].location & 7] : "-", o < s->objCount && s->objs[o].faceUp ? "up" : "down");
                Note(what, q->items[i].card);
            }
        }
        return;
    }
    if (q->type != YGO_Q_IDLE && q->type != YGO_Q_BATTLE)
        return;
    for (i = 0; i < q->count; i++)
    {
        const struct YgoChoice *it = &q->items[i];
        unsigned o = it->obj, loc, ok;
        const struct YgoCard *c;
        char what[96];
        sOffers++;
        if (it->kind == YGO_ACT_TO_BATTLE || it->kind == YGO_ACT_TO_MAIN2 || it->kind == YGO_ACT_END_TURN)
            continue;
        c = Ygo_Card(it->card ? it->card : o < s->objCount ? s->objs[o].card : 0);
        if (o >= s->objCount)
        {
            snprintf(what, sizeof(what), "%s with no card: %s", it->kind < 12 ? sKind[it->kind] : "?", c ? c->name : "?");
            Note(what, it->card);
            continue;
        }
        loc = s->objs[o].location;
        ok = loc == YGO_LOC_HAND || loc == YGO_LOC_MZONE || loc == YGO_LOC_SZONE ||
             ((loc == YGO_LOC_GRAVE || loc == YGO_LOC_EXTRA || loc == YGO_LOC_DECK) && s->objs[o].owner == q->player) ||
             (loc == YGO_LOC_BANISHED && s->objs[o].owner == q->player && s->objs[o].faceUp);
        if (ok && (loc == YGO_LOC_MZONE || loc == YGO_LOC_SZONE) && s->objs[o].controller != q->player)
            ok = 1;   /* (the other side's field: its spots are there too) */
        if (!ok)
        {
            snprintf(what, sizeof(what), "%s %s from %s%s%s", it->kind < 12 ? sKind[it->kind] : "?", c ? c->name : "?",
                     sLoc[loc & 7], s->objs[o].owner != q->player ? " (the other side's)" : "",
                     loc == YGO_LOC_BANISHED && !s->objs[o].faceUp ? " (face-down)" : "");
            Note(what, c ? c->id : 0);
        }
    }
}

static void Duel(unsigned a, unsigned b, uint32_t seed, unsigned first)
{
    static struct YgoDuel d;
    unsigned steps = 0;
    if (!YgoDuel_Init(&d, sMain[a], sExtra[a], sMain[b], sExtra[b], seed, first))
        return;
    while (d.query.type != YGO_Q_NONE && steps++ < 6000 && d.s.turn < 40)
    {
        struct YgoAnswer ans;
        Check(&d);
        YgoDuel_AiAnswer(&d, &ans);
        if (!YgoDuel_Answer(&d, &ans))
            break;
    }
}

static int ByCount(const void *x, const void *y)
{
    return (int)((const typeof(sMiss[0]) *)y)->count - (int)((const typeof(sMiss[0]) *)x)->count;
}

int main(int argc, char **argv)
{
    FILE *f;
    static char line[16384];
    unsigned n = 0, cap = 1024, duels, k, i;
    uint32_t rng;
    if (argc < 3 || !(f = fopen(argv[1], "r")))
        return 1;
    duels = (unsigned)atoi(argv[2]);
    rng = argc > 3 ? (uint32_t)atoi(argv[3]) : 1;
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
    for (k = 0; k < duels && n > 1; k++)
    {
        unsigned a, b;
        rng = rng * 1103515245u + 12345u;
        a = (rng >> 8) % n;
        rng = rng * 1103515245u + 12345u;
        b = (rng >> 8) % n;
        Duel(a, b, rng, k & 1);
        if ((k + 1) % 200 == 0)
            fprintf(stderr, "[audit] %u duels\n", k + 1);
    }
    qsort(sMiss, sMissCount, sizeof(sMiss[0]), ByCount);
    printf("%u duels, %u Main / Battle Phase offers; %u chain chances passed by the table (%u offers)\n", duels, sOffers,
           sQuiet, sQuietOffers);
    for (i = 0; i < sMissCount; i++)
        printf("%6u  %s\n", sMiss[i].count, sMiss[i].what);
    return 0;
}
