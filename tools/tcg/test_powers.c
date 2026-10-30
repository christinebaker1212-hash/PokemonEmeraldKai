/* Every Pokemon Power (src/tcg/tcg_powers.c) and the effects audit's fixes, on staged
 * boards, against the cards' text. Coins: read from the duel's own events. */
#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

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
    for (i = 0; i < p->handSize; i++)
        if (p->cards[p->hand[i]] == id)
            return TcgPile_Remove(p->hand, &p->handSize, i);
    /* Not in the list: a Water Energy still in the deck becomes it */
    for (i = 0; i < p->deckSize; i++)
        if (p->cards[p->deck[i]] == TCG_CARD_WATER_ENERGY && id != TCG_CARD_WATER_ENERGY)
        {
            p->cards[p->deck[i]] = (uint16_t)id;
            return TcgPile_Remove(p->deck, &p->deckSize, i);
        }
    fprintf(stderr, "card %u not available\n", id);
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
/* A card into the hand; its index there */
static unsigned Hand(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    p->hand[p->handSize++] = (uint8_t)Take(g, who, id);
    return p->handSize - 1u;
}
static void Stage(struct TcgDuel *g, const unsigned *mine, unsigned nm, const unsigned *theirs, unsigned nt, uint32_t seed)
{
    uint16_t a[60], b[60];
    Deck(a, mine, nm);
    Deck(b, theirs, nt);
    assert(TcgDuel_Init(g, a, b, seed, 6));
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
static struct TcgAction Act(unsigned type)
{
    struct TcgAction a;
    memset(&a, 0, sizeof(a));
    a.type = (uint8_t)type;
    return a;
}
static int Attack(struct TcgDuel *g, unsigned attack, unsigned target)
{
    struct TcgAction a = Act(TCG_ATTACK);
    a.attack = (uint8_t)attack;
    a.target = (uint8_t)target;
    return TcgDuel_Apply(g, &a);
}
static int Power(struct TcgDuel *g, unsigned slot, unsigned target, unsigned second)
{
    struct TcgAction a = Act(TCG_USE_POWER);
    a.slot = (uint8_t)slot;
    a.target = (uint8_t)target;
    a.attack = (uint8_t)second;
    return TcgDuel_Apply(g, &a);
}
/* The last coin flipped since event seq (1 heads, 0 tails, -1 none) */
static int LastCoin(const struct TcgDuel *g, uint32_t since)
{
    int coin = -1;
    for (; since < g->eventSeq; since++)
    {
        const struct TcgEvent *e = TcgDuel_GetEvent(g, since);
        if (e && e->type == TCG_EV_COIN)
            coin = e->value;
    }
    return coin;
}
static unsigned Damage(const struct TcgDuel *g, unsigned who, unsigned slot) { return g->players[who].field[slot].damage; }
/* A plain attack against the Defending Pokemon: Rattata's Bite (20, no Weakness /
 * Resistance against most), for checking what a power does to an attack's damage */
static void Bite(struct TcgDuel *g)
{
    assert(Attack(g, 0, 0) == TCG_OK);
}

int main(void)
{
    struct TcgDuel g;
    unsigned seed, heads = 0, tails = 0;
    /* ---------------- The audit's fixes ---------------- */
    {
        /* Kangaskhan's Fetch: only the attacker draws (the opponent then draws for their turn) */
        unsigned mine[] = {TCG_CARD_KANGASKHAN}, theirs[] = {TCG_CARD_RATTATA};
        unsigned h0, h1;
        Stage(&g, mine, 1, theirs, 1, 11);
        Put(&g, 0, 0, TCG_CARD_KANGASKHAN);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        h0 = g.players[0].handSize, h1 = g.players[1].handSize;
        assert(Attack(&g, 0, 0) == TCG_OK);
        assert(g.players[0].handSize == h0 + 1 && g.players[1].handSize == h1 + 1 && g.current == 1);
    }
    {
        /* Zubat's Leech Life heals the damage done (after Weakness), not a fixed 10 */
        unsigned mine[] = {TCG_CARD_ZUBAT}, theirs[] = {TCG_CARD_GEODUDE};
        unsigned done;
        Stage(&g, mine, 1, theirs, 1, 12);
        Put(&g, 0, 0, TCG_CARD_ZUBAT);
        Attach(&g, 0, 0, TCG_CARD_GRASS_ENERGY, 2);
        g.players[0].field[0].damage = 30;
        Put(&g, 1, 0, TCG_CARD_GEODUDE);
        assert(Attack(&g, 1, 0) == TCG_OK);
        done = Damage(&g, 1, 0);
        assert(done == 20); /* Geodude is weak to Grass */
        assert(Damage(&g, 0, 0) == 10);
    }
    for (seed = 0; seed < 40; seed++)
    {
        /* Doduo's Fury Attack: 2 coins, so never more than 20 */
        unsigned mine[] = {TCG_CARD_DODUO}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 1, theirs, 1, 100 + seed);
        Put(&g, 0, 0, TCG_CARD_DODUO);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(Attack(&g, 0, 0) == TCG_OK);
        assert(Damage(&g, 1, 0) <= 20);
    }
    {
        /* Mewtwo's Psychic: 10 more for each Energy card: a Double Colorless is one card */
        unsigned mine[] = {TCG_CARD_MEWTWO_LV53}, theirs[] = {TCG_CARD_DODUO, TCG_CARD_DOUBLE_COLORLESS_ENERGY};
        Stage(&g, mine, 1, theirs, 2, 13);
        Put(&g, 0, 0, TCG_CARD_MEWTWO_LV53);
        Attach(&g, 0, 0, TCG_CARD_PSYCHIC_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_DODUO);
        Attach(&g, 1, 0, TCG_CARD_DOUBLE_COLORLESS_ENERGY, 1);
        Attach(&g, 1, 0, TCG_CARD_FIRE_ENERGY, 1);
        assert(Attack(&g, 0, 0) == TCG_OK);
        assert(Damage(&g, 1, 0) == 30);
    }
    {
        /* Seadra's Water Gun (W + C): two Water pay for it all, no extra; a third adds 10;
         * with a Fire Energy for the Colorless, the second Water adds 10 */
        unsigned mine[] = {TCG_CARD_SEADRA, TCG_CARD_HORSEA}, theirs[] = {TCG_CARD_DODUO};
        unsigned k;
        static const unsigned water[] = {2, 3, 2}, fire[] = {0, 0, 1}, expect[] = {20, 30, 30};
        for (k = 0; k < 3; k++)
        {
            Stage(&g, mine, 2, theirs, 1, 14 + k);
            Put(&g, 0, 0, TCG_CARD_HORSEA);
            Put(&g, 0, 0, TCG_CARD_SEADRA);
            Attach(&g, 0, 0, TCG_CARD_WATER_ENERGY, water[k]);
            Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, fire[k]);
            Put(&g, 1, 0, TCG_CARD_DODUO);
            assert(Attack(&g, 0, 0) == TCG_OK);
            assert(Damage(&g, 1, 0) == expect[k]);
        }
    }
    {
        /* Zapdos's Big Thunder: any Pokemon but Zapdos, each as likely (it used to pick a side
         * first: a lone opposing Active took half the hits) */
        unsigned mine[] = {TCG_CARD_ZAPDOS_LV68, TCG_CARD_RATTATA, TCG_CARD_RATTATA, TCG_CARD_RATTATA, TCG_CARD_RATTATA},
                 theirs[] = {TCG_CARD_MACHOP};
        unsigned hits = 0;
        for (seed = 0; seed < 300; seed++)
        {
            unsigned s;
            Stage(&g, mine, 5, theirs, 1, 1000 + seed);
            Put(&g, 0, 0, TCG_CARD_ZAPDOS_LV68);
            Attach(&g, 0, 0, TCG_CARD_LIGHTNING_ENERGY, 3);
            for (s = 1; s <= 4; s++)
                Put(&g, 0, s, TCG_CARD_RATTATA);
            Put(&g, 1, 0, TCG_CARD_MACHOP);
            assert(Attack(&g, 1, 0) == TCG_OK);
            hits += Damage(&g, 1, 0) > 0 || g.players[1].field[0].stackSize == 0;
        }
        assert(hits > 30 && hits < 100); /* about 1 in 5 */
    }
    {
        /* Pikachu's Growl: only attacks on that Pikachu are 10 weaker; benching it ends that */
        unsigned mine[] = {TCG_CARD_PIKACHU_LV16, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_RATTATA};
        unsigned k;
        for (k = 0; k < 2; k++)
        {
            struct TcgFieldCard temp;
            Stage(&g, mine, 2, theirs, 1, 20 + k);
            Put(&g, 0, 0, TCG_CARD_PIKACHU_LV16);
            Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
            Put(&g, 0, 1, TCG_CARD_RATTATA);
            Put(&g, 1, 0, TCG_CARD_RATTATA);
            Attach(&g, 1, 0, TCG_CARD_FIRE_ENERGY, 1);
            assert(Attack(&g, 0, 0) == TCG_OK && g.current == 1);
            if (k)
            {
                /* (Pikachu goes to the Bench) */
                temp = g.players[0].field[0];
                g.players[0].field[0] = g.players[0].field[1];
                g.players[0].field[1] = temp;
            }
            Bite(&g);
            assert(Damage(&g, 0, 0) == (k ? 20u : 10u));
        }
    }
    {
        /* Eevee's Tail Wag: "can't attack Eevee"; another Active can be attacked */
        unsigned mine[] = {TCG_CARD_EEVEE, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_RATTATA};
        for (seed = 0; seed < 40; seed++)
        {
            uint32_t before;
            Stage(&g, mine, 2, theirs, 1, 300 + seed);
            Put(&g, 0, 0, TCG_CARD_EEVEE);
            Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
            Put(&g, 0, 1, TCG_CARD_RATTATA);
            Put(&g, 1, 0, TCG_CARD_RATTATA);
            Attach(&g, 1, 0, TCG_CARD_FIRE_ENERGY, 1);
            before = g.eventSeq;
            assert(Attack(&g, 0, 0) == TCG_OK);
            if (LastCoin(&g, before) != 1)
                continue;
            assert(TcgDuel_NoAttack(&g, 1));
            {
                struct TcgFieldCard temp = g.players[0].field[0];
                g.players[0].field[0] = g.players[0].field[1];
                g.players[0].field[1] = temp;
            }
            assert(!TcgDuel_NoAttack(&g, 1));
            Bite(&g);
            break;
        }
        assert(seed < 40);
    }
    {
        /* Rhydon's Ram switches "even if Rhydon is Knocked Out" */
        unsigned mine[] = {TCG_CARD_RHYHORN, TCG_CARD_RHYDON, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_ONIX, TCG_CARD_RATTATA};
        struct TcgAction promote = Act(TCG_PROMOTE);
        Stage(&g, mine, 3, theirs, 2, 31);
        Put(&g, 0, 0, TCG_CARD_RHYHORN);
        Put(&g, 0, 0, TCG_CARD_RHYDON);
        Attach(&g, 0, 0, TCG_CARD_FIGHTING_ENERGY, 4);
        g.players[0].field[0].damage = 90; /* (Ram's 20 to itself Knocks it Out) */
        Put(&g, 0, 1, TCG_CARD_RATTATA);
        Put(&g, 1, 0, TCG_CARD_ONIX);
        Put(&g, 1, 2, TCG_CARD_RATTATA);
        assert(Attack(&g, 1, 0) == TCG_OK);
        assert(g.phase == TCG_PROMOTION && g.promotePlayer == 1 && g.switchPending);
        promote.slot = 2;
        assert(TcgDuel_Apply(&g, &promote) == TCG_OK);
        assert(TcgDuel_Field(&g, 1, 0)->id == TCG_CARD_RATTATA);
        /* then Rhydon's Knock Out: their Prize, our promotion */
        assert(g.phase == TCG_PROMOTION && g.promotePlayer == 0 && g.players[1].prizeSize == 5);
        assert(TcgDuel_Conserved(&g));
    }
    {
        /* Full Heal: only Asleep / Confused / Paralyzed / Poisoned, not Withdraw's protection */
        unsigned mine[] = {TCG_CARD_SQUIRTLE, TCG_CARD_FULL_HEAL}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction a = Act(TCG_PLAY_TRAINER);
        Stage(&g, mine, 2, theirs, 1, 32);
        Put(&g, 0, 0, TCG_CARD_SQUIRTLE);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        g.players[0].field[0].condition = TCG_ASLEEP;
        g.players[0].field[0].barrier = 1;
        a.hand = (uint8_t)Hand(&g, 0, TCG_CARD_FULL_HEAL);
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        assert(g.players[0].field[0].condition == TCG_HEALTHY && g.players[0].field[0].barrier == 1);
    }
    {
        /* Energy Retrieval is a trade: not the Energy just discarded for it */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_ENERGY_RETRIEVAL}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction a = Act(TCG_PLAY_TRAINER);
        unsigned fire;
        Stage(&g, mine, 2, theirs, 1, 33);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        a.hand = (uint8_t)Hand(&g, 0, TCG_CARD_ENERGY_RETRIEVAL);
        fire = Hand(&g, 0, TCG_CARD_FIRE_ENERGY);
        a.selections[0] = a.selections[1] = g.players[0].hand[fire];
        a.selectionCount = 2;
        assert(TcgDuel_Apply(&g, &a) != TCG_OK);
    }
    {
        /* Devolution Spray: the Pokemon can evolve again the same turn (it didn't evolve yet) */
        unsigned mine[] = {TCG_CARD_CHARMANDER, TCG_CARD_CHARMELEON, TCG_CARD_DEVOLUTION_SPRAY}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction a = Act(TCG_PLAY_TRAINER), evolve = Act(TCG_EVOLVE);
        Stage(&g, mine, 3, theirs, 1, 34);
        Put(&g, 0, 0, TCG_CARD_CHARMANDER);
        Put(&g, 0, 0, TCG_CARD_CHARMELEON);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        a.hand = (uint8_t)Hand(&g, 0, TCG_CARD_DEVOLUTION_SPRAY);
        a.slot = 0;
        a.attack = 1;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK && g.players[0].field[0].stackSize == 1);
        /* (back from the discard pile into the hand, then onto it again) */
        {
            struct TcgPlayer *p = &g.players[0];
            unsigned i;
            for (i = 0; i < p->discardSize; i++)
                if (p->cards[p->discard[i]] == TCG_CARD_CHARMELEON)
                    p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, i);
            evolve.hand = (uint8_t)(p->handSize - 1);
        }
        assert(TcgDuel_Apply(&g, &evolve) == TCG_OK);
    }
    {
        /* Clefairy Doll leaves play when its player wants: no Prize for it */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_CLEFAIRY_DOLL}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction a = Act(TCG_PLAY_TRAINER), drop = Act(TCG_DISCARD_DOLL);
        Stage(&g, mine, 2, theirs, 1, 35);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        a.hand = (uint8_t)Hand(&g, 0, TCG_CARD_CLEFAIRY_DOLL);
        a.slot = 2;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK && g.players[0].field[2].stackSize);
        drop.slot = 2;
        assert(TcgDuel_Apply(&g, &drop) == TCG_OK && !g.players[0].field[2].stackSize);
        assert(g.players[1].prizeSize == 6 && TcgDuel_Conserved(&g));
    }
    puts("PASS the effects audit: Fetch, Leech Life, Fury Attack, Psychic, Water Gun, Big Thunder, Growl, Tail Wag, "
         "Ram, Full Heal, Energy Retrieval, Devolution Spray, Clefairy Doll");

    /* ---------------- Always-on powers ---------------- */
    {
        /* Mr. Mime's Invisible Wall: 30 or more is prevented, less gets through; Muk's Toxic Gas
         * turns it off */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_MUK, TCG_CARD_GRIMER}, theirs[] = {TCG_CARD_MR_MIME};
        unsigned k;
        for (k = 0; k < 3; k++)
        {
            Stage(&g, mine, 3, theirs, 1, 40 + k);
            Put(&g, 0, 0, TCG_CARD_RATTATA);
            Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
            if (k == 2)
            {
                Put(&g, 0, 1, TCG_CARD_GRIMER);
                Put(&g, 0, 1, TCG_CARD_MUK);
            }
            Put(&g, 1, 0, TCG_CARD_MR_MIME);
            g.players[0].field[0].plusPower = 0;
            if (k)
                g.players[0].field[0].attached[g.players[0].field[0].attachedSize++] = (uint8_t)Take(&g, 0, TCG_CARD_PLUSPOWER);
            Bite(&g); /* 20, or 30 with PlusPower */
            assert(Damage(&g, 1, 0) == (k == 0 ? 20u : k == 1 ? 0u : 30u));
        }
    }
    {
        /* Kabuto Armor: half, rounded down to 10 */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_PLUSPOWER}, theirs[] = {TCG_CARD_KABUTO, TCG_CARD_MYSTERIOUS_FOSSIL};
        Stage(&g, mine, 2, theirs, 2, 45);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
        g.players[0].field[0].attached[g.players[0].field[0].attachedSize++] = (uint8_t)Take(&g, 0, TCG_CARD_PLUSPOWER);
        Put(&g, 1, 0, TCG_CARD_MYSTERIOUS_FOSSIL);
        Put(&g, 1, 0, TCG_CARD_KABUTO);
        Bite(&g);
        assert(Damage(&g, 1, 0) == 10); /* 30 -> 15 -> 10 */
    }
    {
        /* Mew's Neutralizing Shield: nothing from an evolved Pokemon, a Basic gets through */
        unsigned mine[] = {TCG_CARD_ODDISH, TCG_CARD_GLOOM}, theirs[] = {TCG_CARD_MEW_LV8};
        unsigned k;
        for (k = 0; k < 2; k++)
        {
            Stage(&g, mine, 2, theirs, 1, 46 + k);
            Put(&g, 0, 0, TCG_CARD_ODDISH);
            if (k)
                Put(&g, 0, 0, TCG_CARD_GLOOM);
            Attach(&g, 0, 0, TCG_CARD_GRASS_ENERGY, 2);
            Put(&g, 1, 0, TCG_CARD_MEW_LV8);
            /* Oddish's Stun Spore / Gloom's Poisonpowder */
            assert(Attack(&g, 0, 0) == TCG_OK);
            if (k)
                assert(Damage(&g, 1, 0) == 0 && !g.players[1].field[0].poison);
            else
                assert(Damage(&g, 1, 0) == 10);
        }
    }
    {
        /* Haunter's Transparency: a coin; heads, nothing reaches it */
        unsigned mine[] = {TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_HAUNTER_LV17, TCG_CARD_GASTLY_LV8};
        for (seed = 0; seed < 30; seed++)
        {
            uint32_t before;
            Stage(&g, mine, 1, theirs, 2, 500 + seed);
            Put(&g, 0, 0, TCG_CARD_RATTATA);
            Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
            Put(&g, 1, 0, TCG_CARD_GASTLY_LV8);
            Put(&g, 1, 0, TCG_CARD_HAUNTER_LV17);
            before = g.eventSeq;
            Bite(&g);
            if (LastCoin(&g, before) == 1)
                assert(Damage(&g, 1, 0) == 0), heads++;
            else
                assert(LastCoin(&g, before) == 0 && Damage(&g, 1, 0) == 20), tails++;
        }
        assert(heads && tails);
    }
    {
        /* Machamp's Strikes Back: 10 to the attacker */
        unsigned mine[] = {TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_MACHOKE, TCG_CARD_MACHAMP};
        Stage(&g, mine, 1, theirs, 3, 50);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        Put(&g, 1, 0, TCG_CARD_MACHOKE);
        Put(&g, 1, 0, TCG_CARD_MACHAMP);
        Bite(&g);
        assert(Damage(&g, 0, 0) == 10);
    }
    {
        /* Aerodactyl's Prehistoric Power: no evolving (either player) */
        unsigned mine[] = {TCG_CARD_CHARMANDER, TCG_CARD_CHARMELEON}, theirs[] = {TCG_CARD_AERODACTYL, TCG_CARD_MYSTERIOUS_FOSSIL};
        struct TcgAction evolve = Act(TCG_EVOLVE);
        Stage(&g, mine, 2, theirs, 2, 51);
        Put(&g, 0, 0, TCG_CARD_CHARMANDER);
        Put(&g, 1, 0, TCG_CARD_MYSTERIOUS_FOSSIL);
        Put(&g, 1, 0, TCG_CARD_AERODACTYL);
        evolve.hand = (uint8_t)Hand(&g, 0, TCG_CARD_CHARMELEON);
        assert(TcgDuel_Apply(&g, &evolve) != TCG_OK);
    }
    {
        /* Dodrio's Retreat Aid: one Colorless less to retreat */
        unsigned mine[] = {TCG_CARD_MACHOP, TCG_CARD_DODUO, TCG_CARD_DODRIO}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction retreat = Act(TCG_RETREAT);
        Stage(&g, mine, 3, theirs, 1, 52);
        Put(&g, 0, 0, TCG_CARD_MACHOP); /* (retreat 1) */
        Put(&g, 0, 1, TCG_CARD_DODUO);
        Put(&g, 0, 1, TCG_CARD_DODRIO);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        retreat.slot = 1;
        assert(TcgDuel_Apply(&g, &retreat) == TCG_OK && TcgDuel_Field(&g, 0, 0)->id == TCG_CARD_DODRIO);
    }
    {
        /* Snorlax's Thick Skinned: Spore doesn't put it to sleep */
        unsigned mine[] = {TCG_CARD_PARAS}, theirs[] = {TCG_CARD_SNORLAX};
        Stage(&g, mine, 1, theirs, 1, 53);
        Put(&g, 0, 0, TCG_CARD_PARAS);
        Attach(&g, 0, 0, TCG_CARD_GRASS_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_SNORLAX);
        assert(Attack(&g, 1, 0) == TCG_OK);
        assert(g.players[1].field[0].condition == TCG_HEALTHY);
    }
    {
        /* Omanyte's Clairvoyance is in play for its player */
        unsigned mine[] = {TCG_CARD_OMANYTE, TCG_CARD_MYSTERIOUS_FOSSIL}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 2, theirs, 1, 54);
        Put(&g, 0, 0, TCG_CARD_MYSTERIOUS_FOSSIL);
        Put(&g, 0, 0, TCG_CARD_OMANYTE);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(TcgPower_InPlay(&g, 0, TCG_POWER_CLAIRVOYANCE) && !TcgPower_InPlay(&g, 1, TCG_POWER_CLAIRVOYANCE));
    }
    puts("PASS always-on powers: Invisible Wall, Toxic Gas, Kabuto Armor, Neutralizing Shield, Transparency, Strikes Back, "
         "Prehistoric Power, Retreat Aid, Thick Skinned, Clairvoyance");

    /* ---------------- When put into play ---------------- */
    {
        /* Moltres's Firegiver: 1 to 4 Fire Energy cards from the deck */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_MOLTRES_LV37}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction play = Act(TCG_PLAY_BASIC);
        unsigned before, fire = 0, i;
        Stage(&g, mine, 2, theirs, 1, 60);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        play.hand = (uint8_t)Hand(&g, 0, TCG_CARD_MOLTRES_LV37);
        play.slot = 1;
        before = g.players[0].handSize;
        assert(TcgDuel_Apply(&g, &play) == TCG_OK);
        for (i = 0; i < g.players[0].handSize; i++)
            fire += g.players[0].cards[g.players[0].hand[i]] == TCG_CARD_FIRE_ENERGY;
        assert(fire >= 1 && fire <= 4 && g.players[0].handSize == before - 1 + fire);
    }
    {
        /* Articuno's Quickfreeze: heads, the Defending Pokemon is Paralyzed */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_ARTICUNO_LV37}, theirs[] = {TCG_CARD_RATTATA};
        for (heads = tails = 0, seed = 0; seed < 30; seed++)
        {
            struct TcgAction play = Act(TCG_PLAY_BASIC);
            uint32_t before;
            Stage(&g, mine, 2, theirs, 1, 600 + seed);
            Put(&g, 0, 0, TCG_CARD_RATTATA);
            Put(&g, 1, 0, TCG_CARD_RATTATA);
            play.hand = (uint8_t)Hand(&g, 0, TCG_CARD_ARTICUNO_LV37);
            play.slot = 1;
            before = g.eventSeq;
            assert(TcgDuel_Apply(&g, &play) == TCG_OK);
            if (LastCoin(&g, before) == 1)
                assert(g.players[1].field[0].condition == TCG_PARALYZED), heads++;
            else
                assert(g.players[1].field[0].condition == TCG_HEALTHY), tails++;
        }
        assert(heads && tails);
    }
    {
        /* Zapdos's Peal of Thunder: 30 to some other Pokemon */
        unsigned mine[] = {TCG_CARD_ONIX, TCG_CARD_ZAPDOS_LV68}, theirs[] = {TCG_CARD_ONIX};
        struct TcgAction play = Act(TCG_PLAY_BASIC);
        Stage(&g, mine, 2, theirs, 1, 61);
        Put(&g, 0, 0, TCG_CARD_ONIX);
        Put(&g, 1, 0, TCG_CARD_ONIX);
        play.hand = (uint8_t)Hand(&g, 0, TCG_CARD_ZAPDOS_LV68);
        play.slot = 1;
        assert(TcgDuel_Apply(&g, &play) == TCG_OK);
        assert(Damage(&g, 0, 0) + Damage(&g, 1, 0) == 30 && !Damage(&g, 0, 1));
    }
    {
        /* Dragonite's Healing Wind: 2 counters off each of its player's Pokemon */
        unsigned mine[] = {TCG_CARD_DRATINI, TCG_CARD_DRAGONAIR, TCG_CARD_DRAGONITE_LV41, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction evolve = Act(TCG_EVOLVE);
        Stage(&g, mine, 4, theirs, 1, 62);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 0, 1, TCG_CARD_DRATINI);
        Put(&g, 0, 1, TCG_CARD_DRAGONAIR);
        g.players[0].field[0].damage = 30;
        g.players[0].field[1].damage = 10;
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        evolve.hand = (uint8_t)Hand(&g, 0, TCG_CARD_DRAGONITE_LV41);
        evolve.slot = 1;
        assert(TcgDuel_Apply(&g, &evolve) == TCG_OK);
        assert(Damage(&g, 0, 0) == 10 && Damage(&g, 0, 1) == 0);
    }
    puts("PASS powers when put into play: Firegiver, Quickfreeze, Peal of Thunder, Healing Wind");

    /* ---------------- Used powers ---------------- */
    {
        /* Venusaur's Solar Power cures both Active Pokemon, once a turn */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_BULBASAUR, TCG_CARD_IVYSAUR, TCG_CARD_VENUSAUR_LV64}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 4, theirs, 1, 70);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 0, 1, TCG_CARD_BULBASAUR);
        Put(&g, 0, 1, TCG_CARD_IVYSAUR);
        Put(&g, 0, 1, TCG_CARD_VENUSAUR_LV64);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        g.players[0].field[0].condition = TCG_PARALYZED;
        g.players[1].field[0].poison = 10;
        assert(Power(&g, 1, 0, 0) == TCG_OK);
        assert(!g.players[0].field[0].condition && !g.players[1].field[0].poison);
        g.players[0].field[0].condition = TCG_ASLEEP;
        assert(Power(&g, 1, 0, 0) != TCG_OK); /* (once a turn) */
    }
    {
        /* Venusaur's Energy Trans moves Grass Energy, as often as you like */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_BULBASAUR, TCG_CARD_IVYSAUR, TCG_CARD_VENUSAUR_LV67}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 4, theirs, 1, 71);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 0, 1, TCG_CARD_BULBASAUR);
        Put(&g, 0, 1, TCG_CARD_IVYSAUR);
        Put(&g, 0, 1, TCG_CARD_VENUSAUR_LV67);
        Attach(&g, 0, 1, TCG_CARD_GRASS_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(Power(&g, 1, 1, 0) == TCG_OK && Power(&g, 1, 1, 0) == TCG_OK);
        assert(TcgDuel_Energy(&g, 0, 0, TCG_GRASS) == 2 && Power(&g, 1, 1, 0) != TCG_OK);
    }
    {
        /* Vileplume's Heal: heads, 1 counter off */
        unsigned mine[] = {TCG_CARD_ODDISH, TCG_CARD_GLOOM, TCG_CARD_VILEPLUME}, theirs[] = {TCG_CARD_RATTATA};
        for (heads = tails = 0, seed = 0; seed < 30; seed++)
        {
            uint32_t before;
            Stage(&g, mine, 3, theirs, 1, 700 + seed);
            Put(&g, 0, 0, TCG_CARD_ODDISH);
            Put(&g, 0, 0, TCG_CARD_GLOOM);
            Put(&g, 0, 0, TCG_CARD_VILEPLUME);
            g.players[0].field[0].damage = 30;
            Put(&g, 1, 0, TCG_CARD_RATTATA);
            before = g.eventSeq;
            assert(Power(&g, 0, 0, 0) == TCG_OK);
            if (LastCoin(&g, before) == 1)
                assert(Damage(&g, 0, 0) == 20), heads++;
            else
                assert(Damage(&g, 0, 0) == 30), tails++;
        }
        assert(heads && tails);
    }
    {
        /* Venomoth's Shift: its type becomes another's (here Fire: Geodude isn't weak to it,
         * Bulbasaur is: Venom Powder does double) */
        unsigned mine[] = {TCG_CARD_VENONAT, TCG_CARD_VENOMOTH, TCG_CARD_CHARMANDER}, theirs[] = {TCG_CARD_BULBASAUR};
        Stage(&g, mine, 3, theirs, 1, 72);
        Put(&g, 0, 0, TCG_CARD_VENONAT);
        Put(&g, 0, 0, TCG_CARD_VENOMOTH);
        Attach(&g, 0, 0, TCG_CARD_GRASS_ENERGY, 2);
        Put(&g, 0, 1, TCG_CARD_CHARMANDER);
        Put(&g, 1, 0, TCG_CARD_BULBASAUR);
        assert(Power(&g, 0, TCG_FIRE, 0) == TCG_OK && TcgDuel_Type(&g, 0, 0) == TCG_FIRE);
        /* (20, and Poison's 10 between turns when the coin poisoned it) */
        assert(Attack(&g, 1, 0) == TCG_OK);
        assert(Damage(&g, 1, 0) == (g.players[1].field[0].poison ? 30u : 20u));
        assert(TcgDuel_Type(&g, 0, 0) == TCG_GRASS); /* (the rest of the turn only) */
    }
    {
        /* Charizard's Energy Burn: Water Energy pays for Fire Spin */
        unsigned mine[] = {TCG_CARD_CHARMANDER, TCG_CARD_CHARMELEON, TCG_CARD_CHARIZARD}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 3, theirs, 1, 73);
        Put(&g, 0, 0, TCG_CARD_CHARMANDER);
        Put(&g, 0, 0, TCG_CARD_CHARMELEON);
        Put(&g, 0, 0, TCG_CARD_CHARIZARD);
        Attach(&g, 0, 0, TCG_CARD_WATER_ENERGY, 4);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(!TcgDuel_CanAttack(&g, 0, 1));
        assert(Power(&g, 0, 0, 0) == TCG_OK && TcgDuel_CanAttack(&g, 0, 1));
    }
    {
        /* Blastoise's Rain Dance: Water Energy onto Water Pokemon, not using the turn's attachment */
        unsigned mine[] = {TCG_CARD_SQUIRTLE, TCG_CARD_WARTORTLE, TCG_CARD_BLASTOISE, TCG_CARD_STARYU}, theirs[] = {TCG_CARD_RATTATA};
        struct TcgAction a = Act(TCG_USE_POWER);
        Stage(&g, mine, 4, theirs, 1, 74);
        Put(&g, 0, 0, TCG_CARD_STARYU);
        Put(&g, 0, 1, TCG_CARD_SQUIRTLE);
        Put(&g, 0, 1, TCG_CARD_WARTORTLE);
        Put(&g, 0, 1, TCG_CARD_BLASTOISE);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        a.slot = 1;
        a.target = 0;
        a.hand = (uint8_t)Hand(&g, 0, TCG_CARD_WATER_ENERGY);
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        a.hand = (uint8_t)Hand(&g, 0, TCG_CARD_WATER_ENERGY);
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        assert(TcgDuel_Energy(&g, 0, 0, TCG_WATER) == 2 && !g.players[0].energyPlayed);
    }
    {
        /* Tentacool's Cowardice: back to the hand, its cards discarded; not the turn it came */
        unsigned mine[] = {TCG_CARD_TENTACOOL, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 2, theirs, 1, 75);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        Put(&g, 0, 1, TCG_CARD_TENTACOOL);
        Attach(&g, 0, 1, TCG_CARD_WATER_ENERGY, 1);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        g.players[0].field[1].enteredTurn = g.turn;
        assert(Power(&g, 1, 0, 0) != TCG_OK);
        g.players[0].field[1].enteredTurn = 1;
        assert(Power(&g, 1, 0, 0) == TCG_OK && !g.players[0].field[1].stackSize && TcgDuel_Conserved(&g));
    }
    {
        /* Mankey's Peek: the top card of the opponent's deck, shown to its player */
        unsigned mine[] = {TCG_CARD_MANKEY}, theirs[] = {TCG_CARD_RATTATA};
        uint32_t before;
        unsigned i, seen = 0;
        Stage(&g, mine, 1, theirs, 1, 76);
        Put(&g, 0, 0, TCG_CARD_MANKEY);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        before = g.eventSeq;
        assert(Power(&g, 0, TCG_PEEK_THEIR_DECK, 0) == TCG_OK);
        for (i = before; i < g.eventSeq; i++)
            if (TcgDuel_GetEvent(&g, i)->type == TCG_EV_PEEK)
                seen = TcgDuel_GetEvent(&g, i)->card;
        assert(seen == g.players[1].cards[g.players[1].deck[g.players[1].deckSize - 1]]);
        assert(Power(&g, 0, TCG_PEEK_MY_DECK, 0) != TCG_OK); /* (once a turn) */
    }
    {
        /* Alakazam's Damage Swap: counters move, never onto a Knock Out */
        unsigned mine[] = {TCG_CARD_ABRA, TCG_CARD_KADABRA, TCG_CARD_ALAKAZAM, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 4, theirs, 1, 77);
        Put(&g, 0, 0, TCG_CARD_RATTATA); /* 30 HP */
        Put(&g, 0, 1, TCG_CARD_ABRA);
        Put(&g, 0, 1, TCG_CARD_KADABRA);
        Put(&g, 0, 1, TCG_CARD_ALAKAZAM);
        g.players[0].field[1].damage = 30;
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(Power(&g, 1, 1, 0) == TCG_OK && Power(&g, 1, 1, 0) == TCG_OK);
        assert(Power(&g, 1, 1, 0) != TCG_OK); /* (a third would Knock Out Rattata) */
        assert(Damage(&g, 0, 0) == 20 && Damage(&g, 0, 1) == 10);
    }
    {
        /* Slowbro's Strange Behavior: counters onto Slowbro */
        unsigned mine[] = {TCG_CARD_SLOWPOKE_LV18, TCG_CARD_SLOWBRO, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 3, theirs, 1, 78);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        g.players[0].field[0].damage = 20;
        Put(&g, 0, 1, TCG_CARD_SLOWPOKE_LV18);
        Put(&g, 0, 1, TCG_CARD_SLOWBRO);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(Power(&g, 1, 0, 0) == TCG_OK && Damage(&g, 0, 0) == 10 && Damage(&g, 0, 1) == 10);
    }
    {
        /* Gengar's Curse: a counter between the opponent's Pokemon, even for a Knock Out */
        unsigned mine[] = {TCG_CARD_GASTLY_LV8, TCG_CARD_HAUNTER_LV22, TCG_CARD_GENGAR}, theirs[] = {TCG_CARD_RATTATA, TCG_CARD_RATTATA};
        Stage(&g, mine, 3, theirs, 2, 79);
        Put(&g, 0, 0, TCG_CARD_GASTLY_LV8);
        Put(&g, 0, 0, TCG_CARD_HAUNTER_LV22);
        Put(&g, 0, 0, TCG_CARD_GENGAR);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        Put(&g, 1, 1, TCG_CARD_RATTATA);
        g.players[1].field[0].damage = 10;
        g.players[1].field[1].damage = 20;
        assert(Power(&g, 0, 0, 1) == TCG_OK);
        assert(!g.players[1].field[1].stackSize && g.players[0].prizeSize == 5 && TcgDuel_Conserved(&g));
    }
    {
        /* Dragonite's Step In: from the Bench to the Active spot, once a turn */
        unsigned mine[] = {TCG_CARD_RATTATA, TCG_CARD_DRATINI, TCG_CARD_DRAGONAIR, TCG_CARD_DRAGONITE_LV45}, theirs[] = {TCG_CARD_RATTATA};
        Stage(&g, mine, 4, theirs, 1, 80);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        g.players[0].field[0].condition = TCG_CONFUSED;
        Put(&g, 0, 2, TCG_CARD_DRATINI);
        Put(&g, 0, 2, TCG_CARD_DRAGONAIR);
        Put(&g, 0, 2, TCG_CARD_DRAGONITE_LV45);
        Put(&g, 1, 0, TCG_CARD_RATTATA);
        assert(Power(&g, 2, 0, 0) == TCG_OK && TcgDuel_Field(&g, 0, 0)->id == TCG_CARD_DRAGONITE_LV45);
        assert(g.players[0].field[2].condition == TCG_HEALTHY); /* (Rattata, benched, is no longer Confused) */
    }
    {
        /* Toxic Gas: no other power can be used */
        unsigned mine[] = {TCG_CARD_ABRA, TCG_CARD_KADABRA, TCG_CARD_ALAKAZAM, TCG_CARD_RATTATA}, theirs[] = {TCG_CARD_GRIMER, TCG_CARD_MUK};
        Stage(&g, mine, 4, theirs, 2, 81);
        Put(&g, 0, 0, TCG_CARD_RATTATA);
        g.players[0].field[0].damage = 10;
        Put(&g, 0, 1, TCG_CARD_ABRA);
        Put(&g, 0, 1, TCG_CARD_KADABRA);
        Put(&g, 0, 1, TCG_CARD_ALAKAZAM);
        Put(&g, 1, 0, TCG_CARD_GRIMER);
        Put(&g, 1, 0, TCG_CARD_MUK);
        assert(TcgPower_Block(&g, 0, 1) && Power(&g, 1, 0, 1) != TCG_OK);
        g.players[1].field[0].condition = TCG_ASLEEP; /* (it stops while Muk is Asleep) */
        assert(!TcgPower_Block(&g, 0, 1));
    }
    puts("PASS used powers: Solar Power, Energy Trans, Heal, Shift, Energy Burn, Rain Dance, Cowardice, Peek, "
         "Damage Swap, Strange Behavior, Curse, Step In, and Toxic Gas stopping them");
    return 0;
}
