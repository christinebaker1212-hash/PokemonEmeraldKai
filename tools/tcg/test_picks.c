/* The 2026-11-18 card audit: every attack that used to pick cards for the player now asks (docs/TCG_REGIONS.md
 * step 1). Each one on a staged board through the flow machine (TcgFlow, as the phone and the board ask), then the
 * action applied: the card the player chose is the one moved. */
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include "tcg/flow.h"
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
/* (a card dealt to the Prizes goes back to the deck, a deck card taking its place) */
static void FromPrizes(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    unsigned i, k;
    for (i = 0; i < p->prizeSize; i++)
        if (p->cards[p->prizes[i]] == id)
            for (k = 0; k < p->deckSize; k++)
                if (p->cards[p->deck[k]] != id)
                {
                    uint8_t t = p->deck[k];
                    p->deck[k] = p->prizes[i];
                    p->prizes[i] = t;
                    break;
                }
}
static unsigned Take(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    unsigned i;
    FromPrizes(g, who, id);
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
static uint8_t Attach(struct TcgDuel *g, unsigned who, unsigned slot, unsigned id, unsigned count)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    while (count--)
        f->attached[f->attachedSize++] = (uint8_t)Take(g, who, id);
    return f->attached[f->attachedSize - 1];
}
static void Pay(struct TcgDuel *g, unsigned who, unsigned id, unsigned attack)
{
    static const unsigned energy[TCG_TYPES] = {TCG_CARD_FIRE_ENERGY, TCG_CARD_GRASS_ENERGY, TCG_CARD_LIGHTNING_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_FIGHTING_ENERGY, TCG_CARD_PSYCHIC_ENERGY,
                                               TCG_CARD_WATER_ENERGY};
    unsigned t;
    for (t = 0; t < TCG_TYPES; t++)
        if (gTcgCards[id].attacks[attack].cost[t])
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
/* The pick's card that is `physical`: it must be offered and choosable */
static void Offered(const struct TcgFlow *f, uint8_t physical)
{
    unsigned i;
    for (i = 0; i < f->pickCount; i++)
        if (f->pick[i] == physical)
        {
            assert(f->pickOk[i]);
            return;
        }
    assert(!"the card wasn't offered");
}
static unsigned Count(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned id)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned i, n = 0;
    for (i = 0; i < f->attachedSize; i++)
        n += g->players[who].cards[f->attached[i]] == id;
    return n;
}
static unsigned ToHand(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    p->hand[p->handSize++] = (uint8_t)Take(g, who, id);
    return p->handSize - 1;
}
static uint8_t InDeck(struct TcgDuel *g, unsigned who, unsigned id)
{
    const struct TcgPlayer *p = &g->players[who];
    unsigned i;
    FromPrizes(g, who, id);
    for (i = 0; i < p->deckSize; i++)
        if (p->cards[p->deck[i]] == id)
            return p->deck[i];
    assert(!"not in the deck");
    return 0;
}
int main(void)
{
    struct TcgDuel g;
    struct TcgFlow f;
    unsigned attack, id, i;
    {
        /* Energy Spike: the basic Energy chosen from the deck, onto the Benched Pokemon chosen */
        unsigned mine[] = {0, TCG_CARD_MACHOP}, theirs[] = {TCG_CARD_RATTATA};
        uint8_t grass = 0;
        struct TcgPlayer *p = &g.players[0];
        mine[0] = id = FindAttack("ElectrodeEnergySpikeEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 0, 1, TCG_CARD_MACHOP);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        for (i = 0; i < p->deckSize; i++)
            if (p->cards[p->deck[i]] == TCG_CARD_GRASS_ENERGY)
                grass = p->deck[i];
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack) == TCG_Q_PICK && f.pickSource == TCG_SRC_DECK);
        Offered(&f, grass);
        assert(TcgFlow_Pick(&f, &g, &grass, 1) == TCG_Q_SLOT && (f.slotMask & 2));
        assert(TcgFlow_Slot(&f, &g, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(Count(&g, 0, 1, TCG_CARD_GRASS_ENERGY) == 1);
    }
    {
        /* Burning Fire: two Fire Energy, the one on the Bench and one on Magmar, as chosen */
        unsigned mine[] = {0, TCG_CARD_CHARMANDER}, theirs[] = {TCG_CARD_RATTATA};
        uint8_t chosen[2];
        mine[0] = id = FindAttack("MagmarBurningFireEffectCommands", &attack);
        Stage(&g, mine, 2, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        chosen[0] = Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 2);
        Put(&g, 0, 1, TCG_CARD_CHARMANDER);
        chosen[1] = Attach(&g, 0, 1, TCG_CARD_FIRE_ENERGY, 1);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        g.players[1].field[0].damage = 0;
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack) == TCG_Q_MENU);
        assert(TcgFlow_Menu(&f, &g, 2) == TCG_Q_PICK && f.pickSource == TCG_SRC_MY_FIELD_ATTACHED && f.pickMin == 2 &&
               f.pickMax == 2);
        Offered(&f, chosen[0]);
        Offered(&f, chosen[1]);
        assert(TcgFlow_Pick(&f, &g, chosen, 2) == TCG_Q_DONE);
        {
            unsigned onMagmar = Count(&g, 0, 0, TCG_CARD_FIRE_ENERGY);
            assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
            assert(Count(&g, 0, 1, TCG_CARD_FIRE_ENERGY) == 0 && Count(&g, 0, 0, TCG_CARD_FIRE_ENERGY) == onMagmar - 1);
        }
    }
    {
        /* Magnetic Lines: the Defending Pokemon's basic Energy chosen, onto the Benched Pokemon chosen */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_RATTATA, TCG_CARD_MACHOP, TCG_CARD_CHARMANDER};
        uint8_t water;
        mine[0] = id = FindAttack("DarkMagnetonMagneticLinesEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 3);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        Attach(&g, 1, 0, TCG_CARD_FIRE_ENERGY, 1);
        water = Attach(&g, 1, 0, TCG_CARD_WATER_ENERGY, 1);
        Put(&g, 1, 1, TCG_CARD_MACHOP);
        Put(&g, 1, 2, TCG_CARD_CHARMANDER);
        g.players[1].field[0].damage = 0;
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack) == TCG_Q_SLOT);
        assert(TcgFlow_Slot(&f, &g, 2) == TCG_Q_PICK && f.pickSource == TCG_SRC_DEFENDER_ATTACHED);
        Offered(&f, water);
        assert(TcgFlow_Pick(&f, &g, &water, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        if (g.players[1].field[0].stackSize) /* (unless the attack Knocked it Out) */
            assert(Count(&g, 1, 0, TCG_CARD_FIRE_ENERGY) == 1);
        assert(Count(&g, 1, 2, TCG_CARD_WATER_ENERGY) == 1 && Count(&g, 1, 1, TCG_CARD_WATER_ENERGY) == 0);
    }
    {
        /* Rapid Evolution: into the evolution chosen (Dark Gyarados over the Gyarados with more HP) */
        unsigned mine[] = {0, TCG_CARD_GYARADOS, TCG_CARD_DARK_GYARADOS}, theirs[] = {TCG_CARD_RATTATA};
        uint8_t dark = 0;
        struct TcgPlayer *p = &g.players[0];
        mine[0] = id = FindAttack("MagikarpRapidEvolutionEffectCommands", &attack);
        Stage(&g, mine, 3, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        for (i = 0; i < p->deckSize; i++)
            if (p->cards[p->deck[i]] == TCG_CARD_DARK_GYARADOS)
                dark = p->deck[i];
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack) == TCG_Q_PICK && f.pickSource == TCG_SRC_DECK);
        Offered(&f, dark);
        for (i = 0; i < f.pickCount; i++) /* (only the two evolutions can be chosen) */
            assert(!f.pickOk[i] || p->cards[f.pick[i]] == TCG_CARD_GYARADOS || p->cards[f.pick[i]] == TCG_CARD_DARK_GYARADOS);
        assert(TcgFlow_Pick(&f, &g, &dark, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(p->cards[p->field[0].stack[p->field[0].stackSize - 1]] == TCG_CARD_DARK_GYARADOS);
    }
    {
        /* Third Eye: the Energy chosen is the one discarded */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_RATTATA};
        uint8_t grass;
        struct TcgPlayer *p = &g.players[0];
        mine[0] = id = FindAttack("DarkGolduckThirdEyeEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        grass = Attach(&g, 0, 0, TCG_CARD_GRASS_ENERGY, 1);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack) == TCG_Q_PICK && f.pickSource == TCG_SRC_MY_ATTACHED);
        Offered(&f, grass);
        assert(TcgFlow_Pick(&f, &g, &grass, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(Count(&g, 0, 0, TCG_CARD_GRASS_ENERGY) == 0 && p->discard[0] == grass);
    }
    {
        /* Earth Wave: the 2 Benched Pokemon chosen of their 3 */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_RATTATA, TCG_CARD_MACHOP, TCG_CARD_CHARMANDER, TCG_CARD_ZUBAT};
        uint8_t chosen[2];
        mine[0] = id = FindAttack("DugtrioEarthWaveEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 4);
        Put(&g, 0, 0, id);
        Pay(&g, 0, id, attack);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        Put(&g, 1, 1, TCG_CARD_MACHOP);
        Put(&g, 1, 2, TCG_CARD_CHARMANDER);
        Put(&g, 1, 3, TCG_CARD_ZUBAT);
        g.players[1].field[2].damage = 30; /* (the least HP left: not chosen) */
        chosen[0] = g.players[1].field[1].stack[0];
        chosen[1] = g.players[1].field[3].stack[0];
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack) == TCG_Q_PICK && f.pickSource == TCG_SRC_OPP_BENCH &&
               f.pickMin == 2 && f.pickMax == 2 && f.pickCount == 3);
        assert(TcgFlow_Pick(&f, &g, chosen, 2) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(g.players[1].field[1].damage == 10 && g.players[1].field[3].damage == 10 &&
               g.players[1].field[2].damage == 30);
    }
    {
        /* Dark Golbat's SNEAK ATTACK: 10 damage to the Benched Pokemon chosen as it evolves */
        unsigned mine[] = {TCG_CARD_ZUBAT, TCG_CARD_DARK_GOLBAT}, theirs[] = {TCG_CARD_RATTATA, TCG_CARD_MACHOP};
        unsigned hand;
        Stage(&g, mine, 2, theirs, 2);
        Put(&g, 0, 0, TCG_CARD_ZUBAT);
        hand = ToHand(&g, 0, TCG_CARD_DARK_GOLBAT);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        Put(&g, 1, 2, TCG_CARD_MACHOP);
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_HAND, hand) == TCG_Q_SLOT);
        assert(TcgFlow_Slot(&f, &g, 0) == TCG_Q_SLOT && f.who == 1 && (f.slotMask & 4));
        assert(TcgFlow_Slot(&f, &g, 2) == TCG_Q_DONE && (f.action.option & TCG_PLAY_CHOSEN));
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(g.players[1].field[2].damage == 10 && g.players[1].field[0].damage == 0);
    }
    {
        /* Dark Dragonite's SUMMON MINIONS: the Basic Pokemon chosen onto the Bench (one, though there's room for two) */
        unsigned mine[] = {TCG_CARD_DARK_DRAGONAIR, TCG_CARD_DARK_DRAGONITE, TCG_CARD_MACHOP, TCG_CARD_RATTATA},
                 theirs[] = {TCG_CARD_RATTATA};
        unsigned hand;
        uint8_t rattata;
        Stage(&g, mine, 4, theirs, 1);
        Put(&g, 0, 0, TCG_CARD_DARK_DRAGONAIR);
        hand = ToHand(&g, 0, TCG_CARD_DARK_DRAGONITE);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        rattata = InDeck(&g, 0, TCG_CARD_RATTATA);
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_HAND, hand) == TCG_Q_SLOT);
        assert(TcgFlow_Slot(&f, &g, 0) == TCG_Q_PICK && f.pickSource == TCG_SRC_DECK);
        Offered(&f, rattata);
        assert(TcgFlow_Pick(&f, &g, &rattata, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(g.players[0].field[1].stackSize == 1 && g.players[0].field[1].stack[0] == rattata &&
               !g.players[0].field[2].stackSize);
    }
    {
        /* Rocket's Sneak Attack: their hand looked at, the Trainer chosen shuffled into their deck */
        unsigned mine[] = {TCG_CARD_MACHOP, TCG_CARD_ROCKETS_SNEAK_ATTACK},
                 theirs[] = {TCG_CARD_RATTATA, TCG_CARD_POTION, TCG_CARD_BILL};
        unsigned hand;
        uint8_t bill;
        struct TcgPlayer *op = &g.players[1];
        Stage(&g, mine, 2, theirs, 3);
        Put(&g, 0, 0, TCG_CARD_MACHOP);
        hand = ToHand(&g, 0, TCG_CARD_ROCKETS_SNEAK_ATTACK);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        ToHand(&g, 1, TCG_CARD_POTION);
        bill = op->hand[ToHand(&g, 1, TCG_CARD_BILL)];
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_HAND, hand) == TCG_Q_PICK && f.pickSource == TCG_SRC_OPP_HAND &&
               f.pickCount == 2 && f.pickMin == 1);
        Offered(&f, bill);
        assert(TcgFlow_Pick(&f, &g, &bill, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        assert(op->handSize == 1 && op->cards[op->hand[0]] == TCG_CARD_POTION);
    }
    {
        /* Challenge!: only the Basic Pokemon chosen go on the Bench (they fill theirs) */
        unsigned mine[] = {TCG_CARD_MACHOP, TCG_CARD_CHALLENGE, TCG_CARD_RATTATA, TCG_CARD_CHARMANDER},
                 theirs[] = {TCG_CARD_RATTATA};
        unsigned hand, s, mineBench = 0, theirBench = 0;
        uint8_t rattata;
        Stage(&g, mine, 4, theirs, 1);
        Put(&g, 0, 0, TCG_CARD_MACHOP);
        hand = ToHand(&g, 0, TCG_CARD_CHALLENGE);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        rattata = InDeck(&g, 0, TCG_CARD_RATTATA);
        assert(TcgFlow_Begin(&f, &g, 0, TCG_FLOW_HAND, hand) == TCG_Q_PICK && f.pickSource == TCG_SRC_DECK);
        assert(TcgFlow_Pick(&f, &g, &rattata, 1) == TCG_Q_DONE);
        assert(TcgDuel_Apply(&g, &f.action) == TCG_OK && TcgDuel_Conserved(&g));
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            mineBench += g.players[0].field[s].stackSize != 0, theirBench += g.players[1].field[s].stackSize != 0;
        assert(mineBench == 1 && g.players[0].field[1].stack[0] == rattata && theirBench == 4);
    }
    {
        /* Every attack that asks: the AI's choices are accepted on a full board */
        static const char *const lists[] = {
            "ElectrodeEnergySpikeEffectCommands", "MagmarBurningFireEffectCommands",
            "DarkMagnetonMagneticLinesEffectCommands", "MagikarpRapidEvolutionEffectCommands",
            "DarkGolduckThirdEyeEffectCommands", "PonytaFireworksEffectCommands", "CharizardFireSpinEffectCommands",
            "RaichuGigashockEffectCommands", "DugtrioEarthWaveEffectCommands", "BellsproutCallForFamilyEffectCommands", "DragonairHyperBeamEffectCommands",
            "MagnetonMicrowaveEffectCommands", "GastlyEnergyConversionEffectCommands"};
        for (i = 0; i < sizeof(lists) / sizeof(lists[0]); i++)
        {
            unsigned mine[] = {0, TCG_CARD_GYARADOS, TCG_CARD_BELLSPROUT}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_RATTATA};
            struct TcgAction a;
            mine[0] = id = FindAttack(lists[i], &attack);
            Stage(&g, mine, 3, theirs, 2);
            Put(&g, 0, 0, id);
            Pay(&g, 0, id, attack);
            Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
            Put(&g, 1, 0, TCG_CARD_MACHOP);
            Attach(&g, 1, 0, TCG_CARD_WATER_ENERGY, 2);
            Put(&g, 1, 1, TCG_CARD_RATTATA);
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            a.attack = (uint8_t)attack;
            a.target = 1; /* (the AI's aim is chosen elsewhere: their Benched Rattata) */
            TcgDuel_AiAttackChoices(&g, &a);
            if (TcgDuel_CanAttack(&g, 0, attack))
            {
                int r = TcgDuel_Apply(&g, &a);
                if (r != TCG_OK || !TcgDuel_Conserved(&g))
                    fprintf(stderr, "%s: %d\n", lists[i], r);
                assert(r == TCG_OK && TcgDuel_Conserved(&g));
            }
        }
    }
    puts("PASS the card audit's choices: Energy Spike, Burning Fire, Magnetic Lines, Rapid Evolution, Third Eye, "
         "Earth Wave, SNEAK ATTACK, SUMMON MINIONS, Rocket's Sneak Attack, Challenge!, through the flow, and the AI's");
    return 0;
}
