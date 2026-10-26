#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned Find(const struct TcgPlayer *p, unsigned id)
{
    unsigned i;
    for (i = 0; i < 60; i++)
        if (p->cards[i] == id)
            return i;
    assert(0);
    return 0;
}
static unsigned Take(struct TcgPlayer *p, unsigned physical)
{
    unsigned i;
    for (i = 0; i < p->deckSize; i++)
        if (p->deck[i] == physical)
            return TcgPile_Remove(p->deck, &p->deckSize, i);
    assert(0);
    return 0;
}
static void Hand(struct TcgPlayer *p, unsigned physical) { p->hand[p->handSize++] = Take(p, physical); }
static void Discard(struct TcgPlayer *p, unsigned physical) { p->discard[p->discardSize++] = Take(p, physical); }
static void Attach(struct TcgPlayer *p, unsigned slot, unsigned physical)
{
    p->field[slot].attached[p->field[slot].attachedSize++] = Take(p, physical);
}
static void Fixture(struct TcgDuel *g)
{
    unsigned who, i;
    memset(g, 0, sizeof(*g));
    g->rng = 123;
    g->phase = TCG_TURN;
    g->turn = 5;
    g->winner = TCG_NONE;
    g->prizeCount = 6;
    for (who = 0; who < 2; who++)
    {
        struct TcgPlayer *p = &g->players[who];
        p->turnsTaken = 3;
        for (i = 0; i < 60; i++)
        {
            p->deck[p->deckSize++] = i;
            p->cards[i] = TCG_CARD_FIRE_ENERGY;
        }
        p->cards[0] = TCG_CARD_CHARMANDER;
        p->cards[1] = TCG_CARD_CHARMELEON;
        p->cards[2] = TCG_CARD_CHARIZARD;
        p->cards[3] = TCG_CARD_SQUIRTLE;
        p->cards[4] = TCG_CARD_WARTORTLE;
        p->cards[5] = TCG_CARD_BLASTOISE;
        p->cards[6] = TCG_CARD_MAGIKARP;
        p->cards[7] = TCG_CARD_ONIX;
        p->cards[9] = TCG_CARD_WATER_ENERGY;
        p->cards[10] = TCG_CARD_DOUBLE_COLORLESS_ENERGY;
        for (i = 0; i < 34; i++)
            p->cards[11 + i] = 195 + i;
        p->field[0].stack[p->field[0].stackSize++] = Take(p, 0);
        p->field[0].enteredTurn = 1;
        p->field[1].stack[p->field[1].stackSize++] = Take(p, 3);
        p->field[1].stack[p->field[1].stackSize++] = Take(p, 4);
        p->field[1].enteredTurn = 1;
        Attach(p, 0, 8);
        Attach(p, 1, 9);
        for (i = 45; i < 51; i++)
            p->prizes[p->prizeSize++] = Take(p, i);
        Hand(p, 52);
        Hand(p, 53);
    }
    assert(TcgDuel_Conserved(g));
}
int main(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned id, played, i, n = 0;
    for (id = 195; id <= 228; id++)
    {
        struct TcgPlayer *p, *op;
        Fixture(&g);
        p = &g.players[0];
        op = &g.players[1];
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_TRAINER;
        played = Find(p, id);
        Hand(p, played);
        a.hand = p->handSize - 1;
        switch (id)
        {
        case TCG_CARD_MR_FUJI:
        case TCG_CARD_SCOOP_UP:
            a.slot = 1;
            break;
        case TCG_CARD_LASS:
            Hand(p, Find(p, TCG_CARD_POTION));
            Hand(op, Find(op, TCG_CARD_POTION));
            break;
        case TCG_CARD_POKEMON_TRADER:
            Hand(p, 6);
            a.selections[0] = 6;
            a.selections[1] = 7;
            a.selectionCount = 2;
            break;
        case TCG_CARD_POKEMON_BREEDER:
            Hand(p, 2);
            a.selections[0] = 2;
            a.selectionCount = 1;
            break;
        case TCG_CARD_CLEFAIRY_DOLL:
        case TCG_CARD_MYSTERIOUS_FOSSIL:
            a.slot = 2;
            break;
        case TCG_CARD_ENERGY_RETRIEVAL:
            Discard(p, 55);
            Discard(p, 56);
            a.selections[0] = 52;
            a.selections[1] = 55;
            a.selections[2] = 56;
            a.selectionCount = 3;
            break;
        case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
            for (i = 55; i < 59; i++)
                Discard(p, i);
            a.selections[0] = 52;
            a.selections[1] = 53;
            for (i = 0; i < 4; i++)
                a.selections[i + 2] = 55 + i;
            a.selectionCount = 6;
            break;
        case TCG_CARD_ENERGY_SEARCH:
            a.selections[0] = 54;
            a.selectionCount = 1;
            break;
        case TCG_CARD_ENERGY_REMOVAL:
            a.target = 1;
            a.selections[0] = 9;
            a.selectionCount = 1;
            break;
        case TCG_CARD_SUPER_ENERGY_REMOVAL:
            a.slot = 0;
            a.target = 1;
            Attach(op, 1, 10);
            a.selections[0] = 8;
            a.selections[1] = 9;
            a.selections[2] = 10;
            a.selectionCount = 3;
            break;
        case TCG_CARD_SWITCH:
            a.slot = 1;
            break;
        case TCG_CARD_POKEMON_CENTER:
            p->field[0].damage = 20;
            p->field[1].damage = 10;
            break;
        case TCG_CARD_POKE_BALL:
            a.selections[0] = 7;
            a.selectionCount = 1;
            break;
        case TCG_CARD_COMPUTER_SEARCH:
            a.selections[0] = 52;
            a.selections[1] = 53;
            a.selections[2] = 2;
            a.selectionCount = 3;
            break;
        case TCG_CARD_POKEDEX:
            a.selectionCount = 5;
            for (i = 0; i < 5; i++)
                a.selections[i] = p->deck[p->deckSize - 5 + i];
            break;
        case TCG_CARD_ITEM_FINDER:
            i = Find(p, TCG_CARD_POTION);
            Discard(p, i);
            a.selections[0] = 52;
            a.selections[1] = 53;
            a.selections[2] = i;
            a.selectionCount = 3;
            break;
        case TCG_CARD_GUST_OF_WIND:
            a.target = 1;
            break;
        case TCG_CARD_DEVOLUTION_SPRAY:
            a.slot = 1;
            a.attack = 1;
            break;
        case TCG_CARD_POTION:
            p->field[0].damage = 20;
            break;
        case TCG_CARD_SUPER_POTION:
            p->field[0].damage = 40;
            a.selections[0] = 8;
            a.selectionCount = 1;
            break;
        case TCG_CARD_FULL_HEAL:
            p->field[0].condition = TCG_CONFUSED;
            p->field[0].poison = 10;
            break;
        case TCG_CARD_REVIVE:
            Discard(p, 6);
            a.slot = 2;
            a.selections[0] = 6;
            a.selectionCount = 1;
            break;
        case TCG_CARD_MAINTENANCE:
            a.selections[0] = 52;
            a.selections[1] = 53;
            a.selectionCount = 2;
            break;
        case TCG_CARD_POKEMON_FLUTE:
            Discard(op, 6);
            a.target = 2;
            a.selections[0] = 6;
            a.selectionCount = 1;
            break;
        case TCG_CARD_RECYCLE:
            i = Find(p, TCG_CARD_POTION);
            Discard(p, i);
            a.selections[0] = i;
            a.selectionCount = 1;
            break;
        }
        assert(TcgDuel_Conserved(&g));
        i = TcgDuel_Apply(&g, &a);
        if (i != TCG_OK)
            fprintf(stderr, "Trainer %u %s failed: %s (result %d)\n", id, Tcg_Card(id)->name, g.error, (int)i);
        assert(i == TCG_OK);
        assert(TcgDuel_Conserved(&g));
        n++;
        if (id == TCG_CARD_PLUSPOWER || id == TCG_CARD_DEFENDER)
        {
            int index = -1;
            for (i = 0; i < p->field[0].attachedSize; i++)
                if (p->field[0].attached[i] == played)
                    index = (int)i;
            assert(index >= 0);
            a.type = TCG_PASS;
            assert(TcgDuel_Apply(&g, &a) == TCG_OK);
            if (id == TCG_CARD_DEFENDER)
                assert(TcgDuel_Apply(&g, &a) == TCG_OK);
            for (i = 0; i < p->field[0].attachedSize; i++)
                assert(p->field[0].attached[i] != played);
        }
        if (id == TCG_CARD_REVIVE)
            assert(p->field[2].damage == 10);
        if (id == TCG_CARD_POKEMON_BREEDER)
            assert(TcgDuel_Field(&g, 0, 0)->id == TCG_CARD_CHARIZARD);
    }
    printf("PASS %u Trainer effects: valid selections, card conservation and attachment expiration\n", n);
    return 0;
}
