/* Yu-Gi-Oh! power audit (tools/ygo/rate_cards.sh): EDOPro duels the AI plays against itself.
 *   rate_cards levels DECKS.txt GAMES          how much each AI level gives away against the best (pairs of the
 *                                              Game Boy opponents' decks, both seats)
 *   rate_cards cards DECKS.txt FROM TO GAMES  each card id in [FROM, TO): three copies in place of the shell's three
 *                                              plainest cards, against the shell itself (both seats, both going first):
 *                                              the Elo the card adds. The shell: the median-rated Game Boy deck.
 * Prints "level N ELO" / "card ID ELO" lines. Deterministic. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../src/ygo/ygo_internal.h"
#include "../../src/ygo/ygo_edo.h"

#define MAX_DECKS 400
static uint16_t sMain[MAX_DECKS][YGO_DECK_MAX], sExtra[MAX_DECKS][YGO_FUSION_MAX];
static unsigned sCount;

/* 0: the first deck won, 1: the second, 2: a draw */
static int Duel(const uint16_t *ma, const uint16_t *xa, const uint16_t *mb, const uint16_t *xb, unsigned la,
                unsigned lb, uint32_t seed, unsigned first)
{
    static struct YgoDuel d;
    unsigned steps = 0;
    if (!YgoDuel_Init(&d, ma, xa, mb, xb, seed, first))
        return 2;
    while (d.query.type != YGO_Q_NONE && steps++ < 4000 && d.s.turn < 80)
    {
        struct YgoAnswer ans;
        YgoDuel_AiAnswerLevel(&d, d.query.player ? lb : la, &ans);
        if (!YgoDuel_Answer(&d, &ans))
        {
            memset(&ans, 0, sizeof(ans));
            if (!YgoDuel_Answer(&d, &ans))
                break;
        }
    }
    if (!d.s.over || d.s.winner > 1)
        return d.s.players[0].lp > d.s.players[1].lp ? 0 : d.s.players[1].lp > d.s.players[0].lp ? 1 : 2;
    return d.s.winner;
}
static double Score(int w, unsigned forFirst) { return w == 2 ? 0.5 : (unsigned)w == forFirst ? 1 : 0; }
static double Elo(double s)
{
    s = s < 0.02 ? 0.02 : s > 0.98 ? 0.98 : s;
    return -400 * log10(1 / s - 1);
}
static int Load(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[8192];
    if (!f)
        return 0;
    while (sCount < MAX_DECKS && fgets(line, sizeof(line), f))
    {
        char *p = line, *end;
        unsigned m = 0, x = 0;
        for (;;)
        {
            unsigned long id = strtoul(p, &end, 10);
            const struct YgoCard *c;
            if (end == p)
                break;
            p = end;
            if (!(c = Ygo_Card((unsigned)id)))
                continue;
            if (c->frame == YGO_FUSION && x < YGO_FUSION_MAX)
                sExtra[sCount][x++] = (uint16_t)id;
            else if (c->frame != YGO_FUSION && m < YGO_DECK_MAX)
                sMain[sCount][m++] = (uint16_t)id;
        }
        sCount++;
    }
    fclose(f);
    return 1;
}
int main(int argc, char **argv)
{
    uint32_t rng = 777;
    if (argc >= 4 && !strcmp(argv[1], "levels") && Load(argv[2]))
    {
        unsigned games = (unsigned)atoi(argv[3]), lv, g;
        for (lv = 0; lv < 3; lv++)
        {
            double t = 0;
            for (g = 0; g < games; g++)
            {
                unsigned a, b;
                rng = rng * 1103515245u + 12345u;
                a = (rng >> 8) % sCount;
                rng = rng * 1103515245u + 12345u;
                b = (rng >> 8) % sCount;
                t += Score(Duel(sMain[a], sExtra[a], sMain[b], sExtra[b], lv, 3, rng, g & 1), 0);
                t += Score(Duel(sMain[b], sExtra[b], sMain[a], sExtra[a], 3, lv, rng ^ 1, g & 1), 1);
            }
            printf("level %u %.0f\n", lv, Elo(t / (2.0 * games)));
            fflush(stdout);
        }
        return 0;
    }
    if (argc >= 6 && !strcmp(argv[1], "cards") && Load(argv[2]))
    {
        unsigned from = (unsigned)atoi(argv[3]), to = (unsigned)atoi(argv[4]), games = (unsigned)atoi(argv[5]);
        unsigned shells[2] = {sCount / 2, sCount * 3 / 4}, sh, id, g, i, n, slots[2][3], k;
        static uint16_t base[2][YGO_DECK_MAX];
        uint16_t test[YGO_DECK_MAX], extra[YGO_FUSION_MAX], noExtra[YGO_FUSION_MAX];
        memset(noExtra, 0, sizeof(noExtra));
        for (sh = 0; sh < 2; sh++)
        {
            memcpy(base[sh], sMain[shells[sh]], sizeof(base[sh]));
            for (n = 0; n < YGO_DECK_MAX && base[sh][n]; n++)
                ;
            /* the three plainest cards: Normal Monsters with the lowest ATK */
            for (k = 0; k < 3; k++)
            {
                int best = -1;
                for (i = 0; i < n; i++)
                {
                    const struct YgoCard *c = Ygo_Card(base[sh][i]), *b;
                    unsigned j, used = 0, ca, cb;
                    for (j = 0; j < k; j++)
                        used |= slots[sh][j] == i;
                    if (used || !c)
                        continue;
                    if (best < 0)
                    {
                        best = (int)i;
                        continue;
                    }
                    b = Ygo_Card(base[sh][best]);
                    ca = c->atk == YGO_STAT_UNKNOWN ? 0 : c->atk;
                    cb = b->atk == YGO_STAT_UNKNOWN ? 0 : b->atk;
                    if ((c->frame == YGO_NORMAL) > (b->frame == YGO_NORMAL) ||
                        ((c->frame == YGO_NORMAL) == (b->frame == YGO_NORMAL) && ca < cb))
                        best = (int)i;
                }
                slots[sh][k] = (unsigned)best;
            }
        }
        for (id = from; id < to && id <= YGO_CARD_COUNT; id++)
        {
            const struct YgoCard *c = Ygo_Card(id);
            double t = 0;
            if (!c || !Ygo_CardReady(id) || c->frame == YGO_TOKEN)
                continue;
            for (g = 0; g < games; g++)
            {
                uint32_t seed = id * 7919u + g * 104729u;
                sh = g & 1;
                memcpy(test, base[sh], sizeof(test));
                memset(extra, 0, sizeof(extra));
                if (c->frame == YGO_FUSION)
                    extra[0] = (uint16_t)id;   /* (a Fusion: in the Extra Deck, the shell as it is) */
                else
                    for (k = 0; k < 3; k++)
                        test[slots[sh][k]] = (uint16_t)id;
                t += Score(Duel(test, extra, base[sh], noExtra, 3, 3, seed, (g >> 1) & 1), 0);
            }
            printf("card %u %.0f\n", id, Elo(t / games));
            fflush(stdout);
        }
        return 0;
    }
    if (argc >= 3 && !strcmp(argv[1], "lflist"))
    {
        /* one of EDOPro's list files as it is (lflist/NAME.lflist.conf) */
        char name[128];
        uint32_t size = 0;
        uint8_t *data;
        snprintf(name, sizeof(name), "lflist/%s.lflist.conf", argv[2]);
        if (!(data = YgoEdo_File(name, &size)))
            return 1;
        fwrite(data, 1, size, stdout);
        free(data);
        return 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "limits"))
    {
        /* every card's copies allowed in each of EDOPro's lists: "lists NAME|NAME..." then "limits ID L0 L1 ..." */
        unsigned id, l, n = YgoEdo_ListCount();
        printf("lists");
        for (l = 0; l < n; l++)
            printf("%s%s", l ? "|" : " ", YgoEdo_ListName(l));
        printf("\n");
        for (id = 1; id <= YGO_CARD_COUNT; id++)
        {
            printf("limits %u", id);
            for (l = 0; l < n; l++)
                printf(" %u", YgoEdo_ListLimit(l, YgoEdo_CodeOfId(id)));
            printf("\n");
        }
        return 0;
    }
    fprintf(stderr, "usage: rate_cards levels DECKS GAMES | cards DECKS FROM TO GAMES | limits\n");
    return 1;
}
