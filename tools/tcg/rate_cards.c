/* The Pokemon TCG power audit (tools/tcg/rate_cards.sh): AI duels on the game's own rules.
 * 1. How much each AI level gives away against the best: pairs of the Game Boy decks, both seats.
 *    Writes src/tcg/tcg_levels.c.
 * 2. Every card's worth: in the median Game Boy deck of its type (Trainers and Colorless: the median deck), three
 *    copies (an Evolution with two of each stage below it) in place of its plainest Pokemon, against the deck as it
 *    was; blended with the card's standing in the 1999-2000 tournament lists of its sets (the era's staples) and
 *    printed with the deck ratings it predicts: docs/audit_tcg.txt.
 * Deterministic. */
#include "mp_version.h"
#include "tcg/duel.h"
#include "tcg/npc.h"
#include "tcg/scale.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int Play(const uint16_t *a, const uint16_t *b, unsigned la, unsigned lb, uint32_t seed)
{
    static struct TcgDuel g;
    struct TcgAction act;
    unsigned steps = 0;
    if (!TcgDuel_Init(&g, a, b, seed, 6))
        return -1;
    while (g.phase != TCG_FINISHED && steps++ < 3000)
    {
        unsigned who = g.phase == TCG_PROMOTION ? g.promotePlayer : g.current;
        if (!TcgDuel_AiActionLevel(&g, who ? lb : la, &act) || TcgDuel_Apply(&g, &act) != TCG_OK)
        {
            memset(&act, 0, sizeof(act));
            act.type = g.phase == TCG_SETUP ? TCG_FINISH_SETUP : TCG_PASS;
            if (TcgDuel_Apply(&g, &act) != TCG_OK)
                break;
        }
    }
    return g.phase == TCG_FINISHED ? (int)g.winner : -1;
}
static double Score(int w, int forA) { return w < 0 ? 0.5 : (w == 0) == forA ? 1 : 0; }
static double Elo(double s)
{
    s = s < 0.02 ? 0.02 : s > 0.98 ? 0.98 : s;
    return -400 * log10(1 / s - 1);
}

/* The era's staples (Base Set, Jungle, Fossil: the Haymaker, Rain Dance, Damage Swap and Fossil decks, 1999-2000) */
static const struct
{
    const char *name;
    int worth;
} sHistory[] = {
    {"Professor Oak", 150}, {"Computer Search", 150}, {"Energy Removal", 150}, {"Super Energy Removal", 150},
    {"Gust of Wind", 150}, {"Item Finder", 150}, {"Bill", 150}, {"PlusPower", 150}, {"Scoop Up", 150},
    {"Pokemon Breeder", 150}, {"Pokemon Trader", 150}, {"Hitmonchan", 150}, {"Electabuzz", 150}, {"Scyther", 150},
    {"Blastoise", 150}, {"Alakazam", 150}, {"Mr. Mime", 150}, {"Chansey", 150}, {"Wigglytuff", 150}, {"Muk", 150},
    {"Aerodactyl", 150}, {"Gengar", 150}, {"Lapras", 150}, {"Zapdos", 150}, {"Double Colorless Energy", 150},
    {"Pokemon Center", 150}, {"Imposter Professor Oak", 150}, {"Kangaskhan", 150}, {"Venusaur", 150},
    {"Dragonite", 150}, {"Lass", 100}, {"Super Potion", 100}, {"Switch", 100}, {"Defender", 100},
    {"Energy Retrieval", 100}, {"Mysterious Fossil", 100}, {"Clefairy Doll", 100}, {"Pokedex", 100},
    {"Recycle", 100}, {"Mr.Fuji", 100}, {"Dodrio", 100}, {"Electrode", 100}, {"Ninetails", 100},
    {"Charizard", 100}, {"Machamp", 100}, {"Haunter", 100}, {"Jynx", 100}, {"Magmar", 100}, {"Hypno", 100},
    {"Snorlax", 100}, {"Ditto", 100}, {"Gyarados", 100}, {"Starmie", 100}, {"Articuno", 100}, {"Moltres", 100},
    {"Dugtrio", 100}, {"Mewtwo", 100}, {"Mew", 100}, {"Rapidash", 100}, {"Arcanine", 100}, {"Pidgeot", 100},
    {"Kabutops", 100}, {"Raichu", 100}, {"Nidoking", 100}, {"Kadabra", 100}, {"Potion", 50},
    {"Energy Search", 50}, {"Full Heal", 50}, {"Maintenance", 50}, {"Devolution Spray", 50}, {"Pokemon Flute", 50},
    {"Super Energy Retrieval", 50}, {"Wartortle", 50}, {"Golem", 50}, {"Clefable", 50}, {"Vileplume", 50},
    {"Venomoth", 50}, {"Tauros", 50}, {"Dewgong", 50}, {"Golduck", 50}, {"Magneton", 50}, {"Jolteon", 50},
    {"Flareon", 50}, {"Vaporeon", 50}, {"Omastar", 50}, {"Hitmonlee", 50}, {"Slowbro", 50},
};
static int History(const struct TcgCard *c)
{
    unsigned i;
    for (i = 0; i < sizeof(sHistory) / sizeof(sHistory[0]); i++)
        if (!strcmp(sHistory[i].name, c->name))
            return sHistory[i].worth;
    return 0;
}
static unsigned Named(const char *name)
{
    unsigned id;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        if (Tcg_Card(id) && !strcmp(Tcg_Card(id)->name, name) && Tcg_Card(id)->kind == TCG_POKEMON)
            return id;
    return 0;
}
static unsigned TypeOf(const uint16_t *deck)
{
    unsigned n[7] = {0}, i, best = 6;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *c = Tcg_Card(deck[i]);
        if (c && c->kind == TCG_ENERGY && c->type < 6)
            n[c->type]++;
    }
    for (i = 0; i < 6; i++)
        if (n[i] > (best < 6 ? n[best] : 0))
            best = i;
    return best;
}
static unsigned Plain(const struct TcgCard *c) { return c->hp + c->attacks[0].damage + c->attacks[1].damage; }

int main(int argc, char **argv)
{
    unsigned games = argc > 1 ? (unsigned)atoi(argv[1]) : 80, levelGames = argc > 2 ? (unsigned)atoi(argv[2]) : 400;
    unsigned n = 0, i, k, lv, id, idx[128];
    double level[4] = {0, 0, 0, 0}, measured[TCG_CARD_COUNT + 1];
    int rating[TCG_CARD_COUNT + 1], tested[TCG_CARD_COUNT + 1];
    unsigned shell[7];
    FILE *doc;
    for (i = 0; i < gTcgDeckDefinitionCount && n < 128; i++)
        if (strcmp(gTcgDeckDefinitions[i].name, "Unnamed") && Tcg_DeckValidate(gTcgDeckDefinitions[i].cards, NULL, NULL, 0))
            idx[n++] = i;
    /* 1. the levels */
    for (lv = 0; lv < 3; lv++)
    {
        double t = 0;
        uint32_t rng = 99;
        for (k = 0; k < levelGames; k++)
        {
            unsigned a, b;
            rng = rng * 1103515245u + 12345u;
            a = idx[(rng >> 8) % n];
            rng = rng * 1103515245u + 12345u;
            b = idx[(rng >> 8) % n];
            t += Score(Play(gTcgDeckDefinitions[a].cards, gTcgDeckDefinitions[b].cards, lv, 3, rng), 1);
            t += Score(Play(gTcgDeckDefinitions[b].cards, gTcgDeckDefinitions[a].cards, 3, lv, rng ^ 7), 0);
        }
        level[lv] = Elo(t / (2.0 * levelGames));
        fprintf(stderr, "level %u: %+.0f\n", lv, level[lv]);
    }
    {
        FILE *f = fopen("src/tcg/tcg_levels.c", "w");
        if (!f)
            return 1;
        fprintf(f, "/* Generated by tools/tcg/rate_cards.sh (tools/tcg/rate_cards.c, %u duels a level): what each AI level\n"
                   " * gives away against the best, in rating points (Game Boy decks, both seats). Do not edit. */\n"
                   "#include \"tcg/scale.h\"\n\nconst int16_t gTcgLevelRating[4] = {%.0f, %.0f, %.0f, 0};\n",
                levelGames * 2, level[0], level[1], level[2]);
        fclose(f);
    }
    /* 2. the cards: the median Game Boy deck of each type (6: overall) */
    for (k = 0; k < 7; k++)
    {
        unsigned list[128], m = 0, j;
        for (i = 0; i < n; i++)
            if (k == 6 || TypeOf(gTcgDeckDefinitions[idx[i]].cards) == k)
                list[m++] = idx[i];
        for (i = 0; i < m; i++)
            for (j = i + 1; j < m; j++)
                if (gTcgDeckRating[list[j]] < gTcgDeckRating[list[i]])
                {
                    unsigned t = list[i];
                    list[i] = list[j];
                    list[j] = t;
                }
        shell[k] = m ? list[m / 2] : 0;
    }
    for (k = 0; k < 7; k++)
        if (k < 6 && !shell[k])
            shell[k] = shell[6];
    for (id = 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        uint16_t test[TCG_DECK_SIZE], put[6];
        const uint16_t *base;
        unsigned np = 0, slot[6], ns = 0, g;
        double t = 0;
        tested[id] = 0;
        measured[id] = 0;
        if (!c || (c->kind == TCG_ENERGY && id <= 6))
            continue;
        base = gTcgDeckDefinitions[shell[c->kind == TCG_POKEMON && c->type < 6 ? c->type : 6]].cards;
        /* already in the deck as many times as allowed: the deck against itself without them (Basic Energy of its
         * type in their place), scaled to three copies */
        {
            unsigned have = 0, energy = 1;
            for (i = 0; i < TCG_DECK_SIZE; i++)
                have += Tcg_Card(base[i]) && !strcmp(Tcg_Card(base[i])->name, c->name);
            for (i = 1; i <= 6; i++)
                if (Tcg_Card(i) && Tcg_Card(i)->type == TypeOf(base))
                    energy = i;
            if (have + (c->kind == TCG_POKEMON && c->stage ? 2 : 3) > 4)
            {
                if (!have)
                    continue;
                memcpy(test, base, sizeof(test));
                for (i = 0; i < TCG_DECK_SIZE; i++)
                    if (Tcg_Card(test[i]) && !strcmp(Tcg_Card(test[i])->name, c->name))
                        test[i] = (uint16_t)energy;
                if (!Tcg_DeckValidate(test, NULL, NULL, 0))
                    continue;
                for (g = 0; g < games; g++)
                {
                    uint32_t seed = id * 7919u + g * 104729u + 5;
                    t += g & 1 ? Score(Play(test, base, 3, 3, seed), 0) : Score(Play(base, test, 3, 3, seed), 1);
                }
                measured[id] = Elo(t / games) * 3.0 / have;
                tested[id] = 1;
                continue;
            }
        }
        /* what goes in: three of it, or for an Evolution two of each stage */
        if (c->kind == TCG_POKEMON && c->stage)
        {
            unsigned below = Named(c->evolvesFrom), below2 = c->stage == 2 && below ? Named(Tcg_Card(below)->evolvesFrom) : 0;
            if (below2)
                put[np++] = (uint16_t)below2, put[np++] = (uint16_t)below2;
            if (below)
                put[np++] = (uint16_t)below, put[np++] = (uint16_t)below;
            put[np++] = (uint16_t)id, put[np++] = (uint16_t)id;
        }
        else
            put[np++] = (uint16_t)id, put[np++] = (uint16_t)id, put[np++] = (uint16_t)id;
        /* in place of the plainest Pokemon (lowest HP and damage) */
        memcpy(test, base, sizeof(test));
        while (ns < np)
        {
            int best = -1;
            for (i = 0; i < TCG_DECK_SIZE; i++)
            {
                const struct TcgCard *b = Tcg_Card(test[i]);
                unsigned j, used = 0;
                for (j = 0; j < ns; j++)
                    used |= slot[j] == i;
                if (used || !b || b->kind != TCG_POKEMON)
                    continue;
                if (best < 0 || Plain(b) < Plain(Tcg_Card(test[best])))
                    best = (int)i;
            }
            if (best < 0)
                break;
            slot[ns++] = (unsigned)best;
        }
        for (i = 0; i < ns && i < np; i++)
            test[slot[i]] = put[i];
        if (!Tcg_DeckValidate(test, NULL, NULL, 0))
            continue;   /* (the stages below over the copies allowed) */
        for (g = 0; g < games; g++)
        {
            uint32_t seed = id * 7919u + g * 104729u + 5;
            t += g & 1 ? Score(Play(base, test, 3, 3, seed), 0) : Score(Play(test, base, 3, 3, seed), 1);
        }
        measured[id] = Elo(t / games);
        tested[id] = 1;
    }
    /* the blend, and the deck ratings it predicts */
    for (id = 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        double w;
        if (!c)
            continue;
        w = tested[id] ? 0.65 * measured[id] + 0.35 * History(c) : History(c);
        rating[id] = (int)lround(1500 + w * (60 / 3.0) * 0.5);
    }
    doc = fopen("docs/audit_tcg.txt", "w");
    if (!doc)
        return 1;
    {
        double xs[128], ys[128], mx = 0, my = 0, sxx = 0, sxy = 0, syy = 0, a, b, err = 0;
        unsigned m = 0;
        for (i = 0; i < n; i++)
        {
            double sum = 0;
            unsigned cnt = 0;
            for (k = 0; k < TCG_DECK_SIZE; k++)
                if (gTcgDeckDefinitions[idx[i]].cards[k] > 6)
                    sum += rating[gTcgDeckDefinitions[idx[i]].cards[k]], cnt++;
            xs[m] = cnt ? sum / cnt : 1500;
            ys[m++] = gTcgDeckRating[idx[i]];
        }
        for (i = 0; i < m; i++)
            mx += xs[i] / m, my += ys[i] / m;
        for (i = 0; i < m; i++)
            sxx += (xs[i] - mx) * (xs[i] - mx), sxy += (xs[i] - mx) * (ys[i] - my), syy += (ys[i] - my) * (ys[i] - my);
        b = sxx ? sxy / sxx : 0;
        a = my - b * mx;
        for (i = 0; i < m; i++)
            err += (a + b * xs[i] - ys[i]) * (a + b * xs[i] - ys[i]) / m;
        fprintf(doc, "Pokemon TCG power audit (tools/tcg/rate_cards.sh)\n\n");
        fprintf(doc, "AI levels against the best (Game Boy decks, both seats, %u duels a level): beginner %+.0f, "
                     "casual %+.0f, skilled %+.0f\n", levelGames * 2, level[0], level[1], level[2]);
        fprintf(doc, "cards: %u duels each in the median deck of their type; ratings blend 65%% measured worth with 35%% "
                     "the era's tournament staples\n", games);
        fprintf(doc, "deck rating from its cards, against the %u Game Boy decks' measured ratings: r = %.2f, typical "
                     "error %.0f\n\n", m, sxx && syy ? sxy / sqrt(sxx * syy) : 0, sqrt(err));
    }
    {
        unsigned order[TCG_CARD_COUNT + 1], m = 0, j;
        for (id = 1; id <= TCG_CARD_COUNT; id++)
            if (Tcg_Card(id) && id > 6)
                order[m++] = id;
        for (i = 0; i < m; i++)
            for (j = i + 1; j < m; j++)
                if (rating[order[j]] > rating[order[i]])
                {
                    unsigned t = order[i];
                    order[i] = order[j];
                    order[j] = t;
                }
        fprintf(doc, "Every card, strongest first: rating, measured worth (Elo of three copies), the era's standing\n");
        for (i = 0; i < m; i++)
        {
            const struct TcgCard *c = Tcg_Card(order[i]);
            char what[40];
            if (tested[order[i]])
                snprintf(what, sizeof(what), "%+5.0f", measured[order[i]]);
            else
                snprintf(what, sizeof(what), "    -");
            fprintf(doc, "  %5d  %-26s %3u  measured %s  era %+4d\n", rating[order[i]], c->name, order[i], what, History(c));
        }
    }
    fclose(doc);
    return 0;
}
