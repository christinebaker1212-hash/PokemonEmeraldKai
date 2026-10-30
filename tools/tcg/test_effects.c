/* Ported attack effects (generate_effects.py "lists"), on staged boards. */
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
    /* Every deck needs a Basic Pokemon even when it stages evolutions. */
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
/* A duel already in player 0's turn with empty fields. */
static void Stage(struct TcgDuel *g, const unsigned *mine, unsigned nm, const unsigned *theirs, unsigned nt)
{
    uint16_t a[60], b[60];
    Deck(a, mine, nm);
    Deck(b, theirs, nt);
    assert(TcgDuel_Init(g, a, b, 99, 6));
    /* Put both hands back so every card can be staged from the deck. */
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
static int Attack(struct TcgDuel *g, unsigned attack, unsigned target)
{
    struct TcgAction a;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = (uint8_t)attack;
    a.target = (uint8_t)target;
    return TcgDuel_Apply(g, &a);
}
int main(void)
{
    struct TcgDuel g;
    unsigned attack, id, discard;
    {
        /* Ember: 30 damage, and one Fire Energy card is discarded. */
        unsigned mine[] = {TCG_CARD_CHARMANDER}, theirs[] = {TCG_CARD_MACHOP};
        id = FindAttack("CharmanderEmberEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        discard = g.players[0].discardSize;
        assert(TcgDuel_Apply(&g, &(struct TcgAction){.type = TCG_ATTACK, .attack = (uint8_t)attack}) == TCG_OK);
        assert(g.players[1].field[0].damage == 30);
        assert(g.players[0].field[0].attachedSize == 1 && g.players[0].discardSize == discard + 1);
        assert(TcgDuel_Conserved(&g) && g.current == 1);
    }
    {
        /* Take Down: double damage for Weakness, 30 recoil, a Knock Out. */
        unsigned mine[] = {TCG_CARD_ARCANINE_LV45}, theirs[] = {TCG_CARD_BULBASAUR, TCG_CARD_BULBASAUR};
        id = FindAttack("ArcanineTakeDownEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 2);
        Put(&g, 0, 0, id);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 4);
        Put(&g, 1, 0, TCG_CARD_BULBASAUR);
        Put(&g, 1, 1, TCG_CARD_BULBASAUR);
        assert(Attack(&g, attack, 0) == TCG_OK);
        assert(g.players[0].field[0].damage == 30);
        assert(g.phase == TCG_PROMOTION && g.promotePlayer == 1 && g.players[0].prizeSize == 5);
        assert(TcgDuel_Conserved(&g));
    }
    {
        /* Whirlwind: the defender's owner picks the Pokemon that comes in,
         * then the attacker's turn ends. */
        unsigned mine[] = {TCG_CARD_PIDGEY}, theirs[] = {TCG_CARD_MACHOP, TCG_CARD_RATTATA};
        struct TcgAction a = {0};
        id = FindAttack("PidgeyWhirlwindEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 2);
        Put(&g, 0, 0, id);
        Attach(&g, 0, 0, TCG_CARD_FIRE_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        Put(&g, 1, 2, TCG_CARD_RATTATA);
        assert(Attack(&g, attack, 0) == TCG_OK);
        assert(g.phase == TCG_PROMOTION && g.promotePlayer == 1 && g.switchPending);
        a.type = TCG_PROMOTE;
        a.slot = 2;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        assert(TcgDuel_Field(&g, 1, 0)->id == TCG_CARD_RATTATA && TcgDuel_Field(&g, 1, 2)->id == TCG_CARD_MACHOP);
        assert(g.players[1].field[2].damage > 0 && !g.switchPending && g.current == 1 && g.phase == TCG_TURN);
        assert(TcgDuel_Conserved(&g));
    }
    {
        /* Sonicboom: no Weakness. Spark: a chosen Benched Pokemon takes 10. */
        unsigned mine[] = {TCG_CARD_ELECTRODE_LV35}, theirs[] = {TCG_CARD_SQUIRTLE, TCG_CARD_SQUIRTLE};
        unsigned spark, sparkAttack;
        id = FindAttack("ElectrodeSonicboomEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 2);
        Put(&g, 0, 0, id);
        Attach(&g, 0, 0, TCG_CARD_LIGHTNING_ENERGY, 4);
        Put(&g, 1, 0, TCG_CARD_SQUIRTLE);
        Put(&g, 1, 1, TCG_CARD_SQUIRTLE);
        assert(Attack(&g, attack, 0) == TCG_OK);
        assert(g.players[1].field[0].damage == gTcgCards[id].attacks[attack].damage);
        spark = FindAttack("PikachuSparkEffectCommands", &sparkAttack);
        Stage(&g, &spark, 1, theirs, 2);
        Put(&g, 0, 0, spark);
        Attach(&g, 0, 0, TCG_CARD_LIGHTNING_ENERGY, 3);
        Put(&g, 1, 0, TCG_CARD_SQUIRTLE);
        Put(&g, 1, 3, TCG_CARD_SQUIRTLE);
        assert(TcgDuel_AttackChoice(&gTcgCards[spark].attacks[sparkAttack], NULL) == 1);
        assert(Attack(&g, sparkAttack, 0) == TCG_INVALID);
        assert(Attack(&g, sparkAttack, 3) == TCG_OK);
        assert(g.players[1].field[3].damage == 10);
    }
    {
        /* Harden: 30 or less is prevented during the next turn only. */
        unsigned mine[] = {TCG_CARD_ONIX}, theirs[] = {TCG_CARD_MACHOP};
        struct TcgAction pass = {0};
        id = FindAttack("OnixHardenEffectCommands", &attack);
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Attach(&g, 0, 0, TCG_CARD_FIGHTING_ENERGY, 2);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        Attach(&g, 1, 0, TCG_CARD_FIGHTING_ENERGY, 1);
        assert(Attack(&g, attack, 0) == TCG_OK);
        assert(g.current == 1 && g.players[0].field[0].guard == TCG_GUARD_UNDER_40);
        assert(Attack(&g, 0, 0) == TCG_OK);
        assert(g.players[0].field[0].damage == 0 && g.current == 0);
        assert(g.players[0].field[0].guard == 0);
        pass.type = TCG_PASS;
        assert(TcgDuel_Apply(&g, &pass) == TCG_OK);
        assert(Attack(&g, 0, 0) == TCG_OK);
        assert(g.players[0].field[0].damage == 20);
    }
    {
        /* Dream Eater needs a sleeping Defending Pokemon. */
        unsigned mine[] = {0}, theirs[] = {TCG_CARD_MACHOP};
        id = FindAttack("HaunterDreamEaterEffectCommands", &attack);
        mine[0] = id;
        Stage(&g, mine, 1, theirs, 1);
        Put(&g, 0, 0, id);
        Attach(&g, 0, 0, TCG_CARD_PSYCHIC_ENERGY, 4);
        Put(&g, 1, 0, TCG_CARD_MACHOP);
        assert(!TcgDuel_CanAttack(&g, 0, attack));
        assert(TcgDuel_EffectBlock(&g, 0, &gTcgCards[id].attacks[attack]));
        g.players[1].field[0].condition = TCG_ASLEEP;
        assert(TcgDuel_CanAttack(&g, 0, attack));
        assert(Attack(&g, attack, 0) == TCG_OK);
    }
    puts("PASS Ember, Take Down, Whirlwind switch choice, Sonicboom, Spark target, Harden, Dream Eater");
    return 0;
}
