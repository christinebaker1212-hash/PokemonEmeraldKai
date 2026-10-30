/* The last twenty attacks (1.46.0): Mix-Up, Wildfire, Amnesia, Magnetic Storm, Wail, Scavenge, Destiny Bond,
 * Prophecy, Mystery Attack, Devolution Beam, Mirror Move, Gale, Hurricane, Metronome, Morph, Conversion 1 / 2,
 * on staged boards (as test_effects.c). */
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned FindAttack(const char *effect, unsigned *attack)
{
    unsigned id, i;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        for (i = 0; i < 2; i++)
            if (gTcgCards[id].kind == TCG_POKEMON && !strcmp(gTcgCards[id].attacks[i].effect, effect))
            {
                *attack = i;
                return id;
            }
    assert(!"effect not in the catalogue");
    return 0;
}
static void Deck(uint16_t *cards, const unsigned *ids, unsigned n)
{
    unsigned i;
    for (i = 0; i < 60; i++)
        cards[i] = i < n ? ids[i] : i % 2 ? TCG_CARD_FIRE_ENERGY : TCG_CARD_WATER_ENERGY;
    for (i = 30; i < 45; i++)
        cards[i] = i % 3 == 0 ? TCG_CARD_LIGHTNING_ENERGY : i % 3 == 1 ? TCG_CARD_PSYCHIC_ENERGY : TCG_CARD_GRASS_ENERGY;
    for (i = 45; i < 55; i++)
        cards[i] = TCG_CARD_FIGHTING_ENERGY;
    for (i = 55; i < 59; i++)
        cards[i] = TCG_CARD_DIGLETT;
}
static unsigned Take(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    unsigned i;
    for (i = 0; i < p->deckSize; i++)
        if (p->cards[p->deck[i]] == id)
            return TcgPile_Remove(p->deck, &p->deckSize, i);
    assert(!"card not available");
    return 0;
}
static void Put(struct TcgDuel *g, unsigned who, unsigned slot, unsigned id)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    f->stack[f->stackSize++] = (uint8_t)Take(g, who, id);
}
static void Attach(struct TcgDuel *g, unsigned who, unsigned slot, unsigned id, unsigned count)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    while (count--)
        f->attached[f->attachedSize++] = (uint8_t)Take(g, who, id);
}
/* The attack's cost in basic Energy (Colorless as Water) */
static void Pay(struct TcgDuel *g, unsigned who, unsigned id, unsigned attack)
{
    static const unsigned energy[TCG_TYPES] = {TCG_CARD_FIRE_ENERGY, TCG_CARD_GRASS_ENERGY, TCG_CARD_LIGHTNING_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_FIGHTING_ENERGY, TCG_CARD_PSYCHIC_ENERGY,
                                               TCG_CARD_WATER_ENERGY};
    unsigned t;
    for (t = 0; t < TCG_TYPES; t++)
        Attach(g, who, 0, energy[t], gTcgCards[id].attacks[attack].cost[t]);
}
static void Stage(struct TcgDuel *g, const unsigned *mine, unsigned nm, const unsigned *theirs, unsigned nt)
{
    uint16_t a[60], b[60];
    Deck(a, mine, nm);
    Deck(b, theirs, nt);
    assert(TcgDuel_Init(g, a, b, 99, 6));
    while (g->players[0].handSize)
        g->players[0].deck[g->players[0].deckSize++] = g->players[0].hand[--g->players[0].handSize];
    while (g->players[1].handSize)
        g->players[1].deck[g->players[1].deckSize++] = g->players[1].hand[--g->players[1].handSize];
    g->phase = TCG_TURN;
    g->current = 0;
    g->turn = 3;
    g->players[0].turnsTaken = g->players[1].turnsTaken = 2;
    g->players[0].setupDone = g->players[1].setupDone = 1;
}
static int Use(struct TcgDuel *g, struct TcgAction a)
{
    a.type = TCG_ATTACK;
    return TcgDuel_Apply(g, &a);
}
static void ToHand(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    p->hand[p->handSize++] = (uint8_t)Take(g, who, id);
}
static unsigned PokemonInHand(const struct TcgDuel *g, unsigned who)
{
    const struct TcgPlayer *p = &g->players[who];
    unsigned i, n = 0;
    for (i = 0; i < p->handSize; i++)
        n += Tcg_Card(p->cards[p->hand[i]])->kind == TCG_POKEMON;
    return n;
}
int main(void)
{
    struct TcgDuel g;
    unsigned attack, id, i;
    {
        /* Mix-Up: their Pokemon cards in hand go back, as many come out of the deck */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_MACHOP, TCG_CARD_MACHOP, TCG_CARD_RATTATA};
        mine[0] = id = FindAttack("NinetalesMixUpEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 4);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        ToHand(&g, 1, TCG_CARD_MACHOP);
        ToHand(&g, 1, TCG_CARD_RATTATA);
        ToHand(&g, 1, TCG_CARD_FIRE_ENERGY);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        assert(g.players[1].handSize == 4 && PokemonInHand(&g, 1) == 2 && TcgDuel_Conserved(&g));
    }
    {
        /* Wildfire: two Fire Energy discarded, two cards off the top of their deck */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP};
        unsigned deck, discard;
        mine[0] = id = FindAttack("MoltresWildfireEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        deck = g.players[1].deckSize;
        discard = g.players[0].discardSize;
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 9}) == TCG_INVALID);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 2}) == TCG_OK);
        assert(g.players[0].discardSize == discard + 2 && g.players[1].deckSize == deck - 3 && /* (and their draw) */
               g.players[1].discardSize == 2 && TcgDuel_Conserved(&g));
    }
    {
        /* Amnesia: the Defending Pokemon can't use the chosen attack on its next turn, then can */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP};
        struct TcgAction pass = {.type = TCG_PASS};
        mine[0] = id = FindAttack("PoliwhirlAmnesiaEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        Attach(&g, 1, 0, TCG_CARD_FIGHTING_ENERGY, 3);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 0}) == TCG_OK);
        assert(g.current == 1 && g.players[1].field[0].disabled == 1 && !TcgDuel_CanAttack(&g, 1, 0));
        assert(Use(&g, (struct TcgAction){.attack = 0}) != TCG_OK);
        assert(TcgDuel_Apply(&g, &pass) == TCG_OK && g.current == 0);
        assert(TcgDuel_Apply(&g, &pass) == TCG_OK && g.current == 1 && TcgDuel_CanAttack(&g, 1, 0));
    }
    {
        /* Magnetic Storm: every Energy card dealt out again, at least one each for two Pokemon and three */
        unsigned mine[] = {0, TCG_CARD_MACHOP}, theirs[] = {TCG_CARD_MACHOP}, total;
        mine[0] = id = FindAttack("MagnemiteMagneticStormEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Attach(&g, 0, 0, TCG_CARD_LIGHTNING_ENERGY, 3 - g.players[0].field[0].attachedSize % 3);
        Put(&g, 0, 2, TCG_CARD_MACHOP);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        total = g.players[0].field[0].attachedSize;
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        assert(g.players[0].field[0].attachedSize + g.players[0].field[2].attachedSize == total);
        assert(g.players[0].field[0].attachedSize >= total / 2 && g.players[0].field[2].attachedSize >= total / 2);
        assert(TcgDuel_Conserved(&g));
    }
    {
        /* Wail: both Benches fill with Basic Pokemon from the decks */
        unsigned mine[] = {0, TCG_CARD_MACHOP}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_MACHOP, TCG_CARD_RATTATA};
        unsigned s, mineN = 0, theirN = 0;
        mine[0] = id = FindAttack("MarowakWailEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 3);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            mineN += g.players[0].field[s].stackSize != 0, theirN += g.players[1].field[s].stackSize != 0;
        /* (theirs: a Machop, a Rattata and 4 Diglett left; mine: a Machop and 4 Diglett) */
        assert(mineN == 5 && theirN == 5 && TcgDuel_Conserved(&g));
    }
    {
        /* Scavenge: a Psychic Energy discarded, the chosen Trainer back to the hand */
        unsigned mine[] = {0, TCG_CARD_POTION, TCG_CARD_BILL}, theirs[] = {TCG_CARD_MACHOP};
        uint8_t bill;
        struct TcgPlayer *p;
        mine[0] = id = FindAttack("SlowpokeScavengeEffectCommands", &attack);
        Stage(&g, mine, 3, theirs, 1);
        p = &g.players[0];
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        assert(!TcgDuel_CanAttack(&g, 0, attack)); /* (no Trainer in the discard pile) */
        p->discard[p->discardSize++] = (uint8_t)Take(&g, 0, TCG_CARD_POTION);
        p->discard[p->discardSize++] = bill = (uint8_t)Take(&g, 0, TCG_CARD_BILL);
        assert(TcgDuel_CanAttack(&g, 0, attack));
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .selections = {bill}, .selectionCount = 1}) == TCG_OK);
        assert(p->handSize == 1 && p->hand[0] == bill && TcgDuel_Conserved(&g));
    }
    {
        /* Destiny Bond: the Pokemon that Knocks Out Gastly on the next turn is Knocked Out too */
        unsigned mine[] = {0, 0}, theirs[] = {TCG_CARD_RATTATA, TCG_CARD_RATTATA};
        mine[0] = mine[1] = id = FindAttack("GastlyDestinyBondEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 2);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 0, 1, id);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        Attach(&g, 1, 0, TCG_CARD_WATER_ENERGY, 3);
        Put(&g, 1, 1, TCG_CARD_RATTATA);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK && g.players[0].field[0].destinyBond);
        g.players[0].field[0].damage = gTcgCards[id].hp - 10;
        assert(Use(&g, (struct TcgAction){.attack = 0}) == TCG_OK);
        assert(g.players[0].discardSize >= 2 && g.players[1].field[0].stackSize == 0);
        assert(TcgDuel_Conserved(&g));
    }
    {
        /* Prophecy: the top three of their deck in the order chosen */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP};
        uint8_t order[3];
        struct TcgPlayer *op;
        mine[0] = id = FindAttack("HypnoProphecyEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        op = &g.players[1];
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        for (i = 0; i < 3; i++)
            order[i] = op->deck[op->deckSize - 3 + i]; /* (bottom of the three on top) */
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 1, .selections = {order[0], order[0], order[2]},
                                         .selectionCount = 3}) == TCG_INVALID);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 1, .selections = {order[0], order[1], order[2]},
                                         .selectionCount = 3}) == TCG_OK);
        /* (their turn began: the first one chosen was drawn) */
        assert(op->hand[op->handSize - 1] == order[0] && op->deck[op->deckSize - 1] == order[1] &&
               op->deck[op->deckSize - 2] == order[2]);
    }
    {
        /* Mystery Attack: 0, 10 or 20 damage, every time */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP}, seed;
        mine[0] = id = FindAttack("MewMysteryAttackEffectCommands", &attack);
        for (seed = 0; seed < 24; seed++)
        {
            Stage(&g, mine, 1, theirs, 1);
            g.rng = seed * 7919u + 1;
            Put(&g, 0, 0, id);
            Pay(&g, 0, id, attack);
            Put(&g, 1, 0, TCG_CARD_MACHOP);
            assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
            /* (Machop is weak to Psychic: 0, 20 or 40, and 10 more if it was Poisoned) */
            assert(g.players[1].field[0].damage % 10 == 0 && g.players[1].field[0].damage <= 50);
            assert(TcgDuel_Conserved(&g));
        }
    }
    {
        /* Devolution Beam: their Ivysaur back to their hand, the Bulbasaur stays */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_BULBASAUR, TCG_CARD_IVYSAUR};
        mine[0] = id = FindAttack("MewDevolutionBeamEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 2);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_BULBASAUR);
        Put(&g, 1, 0, TCG_CARD_IVYSAUR);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 0, .target = 0}) == TCG_INVALID);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = 1, .target = 0}) == TCG_OK);
        /* (and their draw) */
        assert(TcgDuel_Field(&g, 1, 0)->id == TCG_CARD_BULBASAUR && g.players[1].handSize == 2 &&
               g.players[1].cards[g.players[1].hand[0]] == TCG_CARD_IVYSAUR && TcgDuel_Conserved(&g));
    }
    {
        /* Mirror Move: the damage Spearow took last turn, done back */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_RATTATA};
        mine[0] = id = FindAttack("SpearowMirrorMoveEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        Attach(&g, 1, 0, TCG_CARD_WATER_ENERGY, 2);
        assert(!TcgDuel_CanAttack(&g, 0, attack));
        g.current = 1;
        assert(Use(&g, (struct TcgAction){.attack = 0}) == TCG_OK && g.current == 0);
        assert(g.players[0].field[0].mirrorDamage == g.players[0].field[0].damage && g.players[0].field[0].damage);
        assert(TcgDuel_CanAttack(&g, 0, attack));
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        assert(g.players[1].field[0].damage == g.players[0].field[0].damage);
    }
    {
        /* Gale: both Active Pokemon switch with Benched ones at random */
        unsigned mine[] = {0, TCG_CARD_MACHOP}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_RATTATA};
        mine[0] = id = FindAttack("PidgeotGaleEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 2);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 0, 3, TCG_CARD_MACHOP);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        Put(&g, 1, 2, TCG_CARD_RATTATA);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        assert(TcgDuel_Field(&g, 0, 0)->id == TCG_CARD_MACHOP && TcgDuel_Field(&g, 0, 3)->id == id);
        assert(TcgDuel_Field(&g, 1, 0)->id == TCG_CARD_RATTATA && TcgDuel_Field(&g, 1, 2)->id == TCG_CARD_MACHOP);
        assert(g.players[1].field[2].damage && TcgDuel_Conserved(&g));
    }
    {
        /* Hurricane: the Defending Pokemon and its cards back to their hand; they choose a new Active */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_ONIX, TCG_CARD_RATTATA};
        mine[0] = id = FindAttack("PidgeotHurricaneEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 2);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_ONIX);
        Attach(&g, 1, 0, TCG_CARD_FIGHTING_ENERGY, 2);
        Put(&g, 1, 1, TCG_CARD_RATTATA);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        assert(!g.players[1].field[0].stackSize && g.players[1].handSize == 3); /* (no draw: they choose first) */
        assert(g.phase == TCG_PROMOTION && g.promotePlayer == 1 && TcgDuel_Conserved(&g));
    }
    {
        /* Metronome: one of the Defending Pokemon's attacks without its cost; never Metronome itself */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP};
        mine[0] = id = FindAttack("ClefairyMetronomeEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        assert(TcgDuel_IsMetronome(&gTcgCards[id].attacks[attack]));
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) != TCG_OK); /* (nothing copied) */
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .copy = 1}) == TCG_OK);
        assert(g.players[1].field[0].damage == gTcgCards[TCG_CARD_MACHOP].attacks[0].damage);
    }
    {
        /* Morph: Ditto becomes a Basic Pokemon from the deck (not Ditto), damage and conditions gone */
        unsigned mine[] = {0, TCG_CARD_BULBASAUR}, theirs[] = {TCG_CARD_MACHOP};
        unsigned top;
        mine[0] = id = FindAttack("DittoMorphEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        g.players[0].field[0].damage = 20;
        g.players[0].field[0].poison = 10;
        top = g.players[0].field[0].stack[0];
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack}) == TCG_OK);
        assert(g.players[0].cards[top] != id && !Tcg_Card(g.players[0].cards[top])->stage);
        /* (poison counted at the end of the turn) */
        assert(g.players[0].field[0].damage <= 10 && TcgDuel_Conserved(&g));
    }
    {
        /* Conversion 1 / 2: the Defending Pokemon's Weakness, Porygon's Resistance */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP}, c2;
        mine[0] = id = FindAttack("PorygonConversion1EffectCommands", &attack);
        FindAttack("PorygonConversion2EffectCommands", &c2);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Pay(&g, 0, id, c2);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = TCG_COLORLESS}) == TCG_INVALID);
        assert(Use(&g, (struct TcgAction){.attack = (uint8_t)attack, .option = TCG_FIRE}) == TCG_OK);
        assert(TcgDuel_Weakness(&g, 1, 0) == TCG_FIRE);
        g.current = 0;
        if (gTcgCards[id].resistance != TCG_NONE)
        {
            assert(Use(&g, (struct TcgAction){.attack = (uint8_t)c2, .option = TCG_WATER}) == TCG_OK);
            assert(TcgDuel_Resistance(&g, 0, 0) == TCG_WATER);
        }
    }
    {
        /* The AI can use every one of them on a staged board */
        static const char *const lists[] = {
            "NinetalesMixUpEffectCommands", "MoltresWildfireEffectCommands", "PoliwhirlAmnesiaEffectCommands",
            "MagnemiteMagneticStormEffectCommands", "MarowakWailEffectCommands", "MewMysteryAttackEffectCommands",
            "PidgeotGaleEffectCommands", "PidgeotHurricaneEffectCommands", "ClefairyMetronomeEffectCommands",
            "ClefableMetronomeEffectCommands", "DittoMorphEffectCommands", "PorygonConversion1EffectCommands"};
        for (i = 0; i < sizeof(lists) / sizeof(lists[0]); i++)
        {
            unsigned mine[] = {0, TCG_CARD_MACHOP}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_RATTATA};
            struct TcgAction a;
            mine[0] = id = FindAttack(lists[i], &attack);
            Stage(&g, mine, 2, theirs, 2);
            Put(&g, 0, 0, id);
            Pay(&g, 0, id, attack);
            Put(&g, 1, 0, TCG_CARD_MACHOP);
            Put(&g, 1, 1, TCG_CARD_RATTATA);
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            a.attack = (uint8_t)attack;
            TcgDuel_AiAttackChoices(&g, &a);
            if (TcgDuel_CanAttack(&g, 0, attack))
                assert(TcgDuel_Apply(&g, &a) == TCG_OK && TcgDuel_Conserved(&g));
        }
    }
    puts("PASS Mix-Up, Wildfire, Amnesia, Magnetic Storm, Wail, Scavenge, Destiny Bond, Prophecy, Mystery Attack, "
         "Devolution Beam, Mirror Move, Gale, Hurricane, Metronome, Morph, Conversion 1 / 2, and the AI's picks");
    return 0;
}
