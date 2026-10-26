#include "mp_version.h"
#if MOEMON_DEV_BUILD || defined(TCG_TEST)
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <stdio.h>
#include <string.h>
static int Index(const uint8_t *pile, unsigned count, unsigned card)
{
    unsigned i;
    for (i = 0; i < count; i++)
        if (pile[i] == card)
            return i;
    return -1;
}
static int Move(uint8_t *from, uint8_t *count, uint8_t *to, uint8_t *toCount, unsigned card)
{
    int at = Index(from, *count, card);
    if (at < 0)
        return 0;
    to[(*toCount)++] = TcgPile_Remove(from, count, (unsigned)at);
    return 1;
}
static void Cure(struct TcgFieldCard *f)
{
    f->condition = TCG_HEALTHY;
    f->poison = 0;
    f->barrier = 0;
    f->noRetreat = 0;
    f->noAttack = 0;
}
static int Fail(struct TcgDuel *g, const char *message)
{
    snprintf(g->error, sizeof(g->error), "%s", message);
    return TCG_INVALID;
}
int TcgDuel_Trainer(struct TcgDuel *g, const struct TcgAction *a)
{
    struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    struct TcgFieldCard *f;
    const struct TcgCard *card;
    unsigned played, id, i, n, who, target = a->target;
    int at;
    if (a->hand >= p->handSize)
        return TCG_INVALID;
    played = p->hand[a->hand];
    card = Tcg_Card(p->cards[played]);
    id = card->id;
    if (card->kind != TCG_TRAINER)
        return Fail(g, "Choose a Trainer card.");
    /* The transactional caller restores all piles if any selection is invalid. */
    TcgPile_Remove(p->hand, &p->handSize, a->hand);
    switch (id)
    {
    case TCG_CARD_PROFESSOR_OAK:
        while (p->handSize)
            p->discard[p->discardSize++] = p->hand[--p->handSize];
        TcgDuel_Draw(g, g->current, 7);
        break;
    case TCG_CARD_IMPOSTER_PROFESSOR_OAK:
        while (op->handSize)
            op->deck[op->deckSize++] = op->hand[--op->handSize];
        TcgDuel_Shuffle(g, g->current ^ 1);
        TcgDuel_Draw(g, g->current ^ 1, 7);
        break;
    case TCG_CARD_BILL:
        if (!p->deckSize)
            return Fail(g, "Your deck is empty.");
        TcgDuel_Draw(g, g->current, 2);
        break;
    case TCG_CARD_IMAKUNI_CARD:
        if (!p->field[0].stackSize)
            return TCG_INVALID;
        p->field[0].condition = TCG_CONFUSED;
        break;
    case TCG_CARD_LASS:
        for (who = 0; who < 2; who++)
        {
            struct TcgPlayer *q = &g->players[who];
            for (i = q->handSize; i > 0; i--)
                if (Tcg_Card(q->cards[q->hand[i - 1]])->kind == TCG_TRAINER)
                    q->deck[q->deckSize++] = TcgPile_Remove(q->hand, &q->handSize, i - 1);
            TcgDuel_Shuffle(g, who);
        }
        break;
    case TCG_CARD_POTION:
    case TCG_CARD_SUPER_POTION:
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].stackSize || !p->field[a->slot].damage)
            return Fail(g, "Choose one of your damaged Pokemon.");
        f = &p->field[a->slot];
        if (id == TCG_CARD_SUPER_POTION)
        {
            if (a->selectionCount != 1)
                return Fail(g, "Select one attached Energy to discard.");
            at = Index(f->attached, f->attachedSize, a->selections[0]);
            if (at < 0 || Tcg_Card(p->cards[a->selections[0]])->kind != TCG_ENERGY)
                return TCG_INVALID;
            p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, at);
        }
        n = id == TCG_CARD_POTION ? 20 : 40;
        f->damage = f->damage > n ? f->damage - n : 0;
        break;
    case TCG_CARD_FULL_HEAL:
        if (!p->field[0].condition && !p->field[0].poison)
            return Fail(g, "Your Active Pokemon has no special condition.");
        Cure(&p->field[0]);
        break;
    case TCG_CARD_POKEMON_CENTER:
        n = 0;
        for (i = 0; i < TCG_FIELD_SIZE; i++)
        {
            f = &p->field[i];
            if (f->stackSize && f->damage)
            {
                unsigned j;
                n++;
                f->damage = 0;
                for (j = f->attachedSize; j > 0; j--)
                    if (Tcg_Card(p->cards[f->attached[j - 1]])->kind == TCG_ENERGY)
                        p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, j - 1);
            }
        }
        if (!n)
            return Fail(g, "None of your Pokemon are damaged.");
        break;
    case TCG_CARD_SWITCH:
    case TCG_CARD_GUST_OF_WIND:
    {
        struct TcgPlayer *q = id == TCG_CARD_SWITCH ? p : op;
        struct TcgFieldCard temp;
        unsigned s = id == TCG_CARD_SWITCH ? a->slot : target;
        if (s < 1 || s >= TCG_FIELD_SIZE || !q->field[s].stackSize)
            return Fail(g, "Choose a Benched Pokemon.");
        temp = q->field[0];
        q->field[0] = q->field[s];
        q->field[s] = temp;
        Cure(&q->field[s]);
        break;
    }
    case TCG_CARD_ENERGY_SEARCH:
    case TCG_CARD_POKE_BALL:
        if (a->selectionCount != 1 || a->selections[0] >= 60)
            return Fail(g, "Select a card from your deck.");
        card = Tcg_Card(p->cards[a->selections[0]]);
        if (id == TCG_CARD_ENERGY_SEARCH && card->id > 6)
            return Fail(g, "Energy Search requires a basic Energy card.");
        if (id == TCG_CARD_POKE_BALL && card->kind != TCG_POKEMON)
            return Fail(g, "Poke Ball requires a Pokemon card.");
        at = Index(p->deck, p->deckSize, a->selections[0]);
        if (at < 0)
            return TCG_INVALID;
        if (id == TCG_CARD_ENERGY_SEARCH || Tcg_RandomBelow(&g->rng, 2))
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, at);
        TcgDuel_Shuffle(g, g->current);
        break;
    case TCG_CARD_COMPUTER_SEARCH:
    case TCG_CARD_ITEM_FINDER:
        if (a->selectionCount != 3)
            return Fail(g, "Select two cards to discard and one card to retrieve.");
        for (i = 0; i < 2; i++)
            if (!Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]))
                return Fail(g, "Discard two different cards from your hand.");
        if (a->selections[2] >= 60)
            return TCG_INVALID;
        if (id == TCG_CARD_COMPUTER_SEARCH)
        {
            if (!Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[2]))
                return Fail(g, "Select a card from your deck.");
            TcgDuel_Shuffle(g, g->current);
        }
        else
        {
            if (Tcg_Card(p->cards[a->selections[2]])->kind != TCG_TRAINER ||
                !Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[2]))
                return Fail(g, "Select a Trainer card from your discard pile.");
        }
        break;
    case TCG_CARD_ENERGY_RETRIEVAL:
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
        n = id == TCG_CARD_ENERGY_RETRIEVAL ? 1 : 2;
        if (a->selectionCount <= n || a->selectionCount > n + (n == 1 ? 2 : 4))
            return Fail(g, "Select discard costs and up to the allowed basic Energy cards.");
        for (i = 0; i < n; i++)
            if (!Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]))
                return TCG_INVALID;
        for (i = n; i < a->selectionCount; i++)
            if (a->selections[i] >= 60 || p->cards[a->selections[i]] > 6 ||
                !Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[i]))
                return TCG_INVALID;
        break;
    case TCG_CARD_ENERGY_REMOVAL:
    case TCG_CARD_SUPER_ENERGY_REMOVAL:
        if (target >= TCG_FIELD_SIZE || !op->field[target].stackSize)
            return Fail(g, "Choose an opponent's Pokemon with Energy attached.");
        i = 0;
        if (id == TCG_CARD_SUPER_ENERGY_REMOVAL)
        {
            if (a->slot >= TCG_FIELD_SIZE || a->selectionCount < 2 || a->selectionCount > 3)
                return TCG_INVALID;
            f = &p->field[a->slot];
            at = Index(f->attached, f->attachedSize, a->selections[0]);
            if (at < 0 || Tcg_Card(p->cards[a->selections[0]])->kind != TCG_ENERGY)
                return TCG_INVALID;
            p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, at);
            i = 1;
        }
        else if (a->selectionCount != 1)
            return TCG_INVALID;
        f = &op->field[target];
        for (; i < a->selectionCount; i++)
        {
            at = Index(f->attached, f->attachedSize, a->selections[i]);
            if (at < 0 || Tcg_Card(op->cards[a->selections[i]])->kind != TCG_ENERGY)
                return TCG_INVALID;
            op->discard[op->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, at);
        }
        break;
    case TCG_CARD_MAINTENANCE:
        if (a->selectionCount != 2)
            return Fail(g, "Select two other cards from your hand.");
        for (i = 0; i < 2; i++)
            if (!Move(p->hand, &p->handSize, p->deck, &p->deckSize, a->selections[i]))
                return TCG_INVALID;
        TcgDuel_Shuffle(g, g->current);
        TcgDuel_Draw(g, g->current, 1);
        break;
    case TCG_CARD_GAMBLER:
        while (p->handSize)
            p->deck[p->deckSize++] = p->hand[--p->handSize];
        TcgDuel_Shuffle(g, g->current);
        TcgDuel_Draw(g, g->current, Tcg_RandomBelow(&g->rng, 2) ? 8 : 1);
        break;
    case TCG_CARD_RECYCLE:
        if (a->selectionCount != 1 || Index(p->discard, p->discardSize, a->selections[0]) < 0)
            return Fail(g, "Select a card from your discard pile.");
        if (Tcg_RandomBelow(&g->rng, 2))
            Move(p->discard, &p->discardSize, p->deck, &p->deckSize, a->selections[0]);
        break;
    case TCG_CARD_PLUSPOWER:
    case TCG_CARD_DEFENDER:
        i = id == TCG_CARD_PLUSPOWER ? 0 : a->slot;
        if (i >= TCG_FIELD_SIZE || !p->field[i].stackSize)
            return Fail(g, "Choose one of your Pokemon.");
        p->field[i].attached[p->field[i].attachedSize++] = played;
        p->attachmentExpires[played] = g->turn + (id == TCG_CARD_DEFENDER ? 1 : 0);
        TcgDuel_Log(g, "Attached %s.", Tcg_Card(id)->name);
        return TCG_OK;
    case TCG_CARD_CLEFAIRY_DOLL:
    case TCG_CARD_MYSTERIOUS_FOSSIL:
        if (a->slot < 1 || a->slot >= TCG_FIELD_SIZE || p->field[a->slot].stackSize)
            return Fail(g, "Choose an empty Bench slot.");
        f = &p->field[a->slot];
        memset(f, 0, sizeof(*f));
        f->stack[f->stackSize++] = played;
        f->enteredTurn = g->turn;
        f->noPrize = 1;
        return TCG_OK;
    case TCG_CARD_POKEMON_TRADER:
        if (a->selectionCount != 2 || a->selections[0] >= 60 || a->selections[1] >= 60 ||
            Tcg_Card(p->cards[a->selections[0]])->kind != TCG_POKEMON ||
            Tcg_Card(p->cards[a->selections[1]])->kind != TCG_POKEMON)
            return Fail(g, "Choose a Pokemon in your hand and one in your deck.");
        if (Index(p->hand, p->handSize, a->selections[0]) < 0 || Index(p->deck, p->deckSize, a->selections[1]) < 0)
            return TCG_INVALID;
        Move(p->hand, &p->handSize, p->deck, &p->deckSize, a->selections[0]);
        Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[1]);
        TcgDuel_Shuffle(g, g->current);
        break;
    case TCG_CARD_POKEMON_BREEDER:
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].stackSize || a->selectionCount != 1 ||
            a->selections[0] >= 60)
            return Fail(g, "Choose a matching Basic Pokemon and a Stage 2 card from your hand.");
        f = &p->field[a->slot];
        card = Tcg_Card(p->cards[a->selections[0]]);
        if (card->kind != TCG_POKEMON || card->stage != 2 || TcgDuel_Field(g, g->current, a->slot)->stage != 0 ||
            p->turnsTaken < 2 || f->enteredTurn == g->turn)
            return TCG_INVALID;
        n = 0;
        for (i = 1; i <= TCG_CARD_COUNT; i++)
            if (gTcgCards[i].kind == TCG_POKEMON && gTcgCards[i].stage == 1 &&
                !strcmp(gTcgCards[i].name, card->evolvesFrom) &&
                !strcmp(gTcgCards[i].evolvesFrom, TcgDuel_Field(g, g->current, a->slot)->name))
                n = 1;
        at = Index(p->hand, p->handSize, a->selections[0]);
        if (!n || at < 0)
            return TCG_INVALID;
        f->stack[f->stackSize++] = TcgPile_Remove(p->hand, &p->handSize, at);
        f->enteredTurn = g->turn;
        Cure(f);
        break;
    case TCG_CARD_REVIVE:
    case TCG_CARD_POKEMON_FLUTE:
    {
        unsigned owner = id == TCG_CARD_REVIVE ? g->current : g->current ^ 1;
        struct TcgPlayer *q = &g->players[owner];
        unsigned slot = id == TCG_CARD_REVIVE ? a->slot : a->target;
        if (slot < 1 || slot >= TCG_FIELD_SIZE || q->field[slot].stackSize || a->selectionCount != 1 ||
            a->selections[0] >= 60)
            return Fail(g, "Choose a Basic Pokemon from the discard pile and an empty Bench slot.");
        card = Tcg_Card(q->cards[a->selections[0]]);
        at = Index(q->discard, q->discardSize, a->selections[0]);
        if (at < 0 || card->kind != TCG_POKEMON || card->stage)
            return TCG_INVALID;
        f = &q->field[slot];
        memset(f, 0, sizeof(*f));
        f->stack[f->stackSize++] = TcgPile_Remove(q->discard, &q->discardSize, at);
        f->enteredTurn = g->turn;
        if (id == TCG_CARD_REVIVE)
            f->damage = (card->hp / 20) * 10;
        break;
    }
    case TCG_CARD_MR_FUJI:
    case TCG_CARD_SCOOP_UP:
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].stackSize || (id == TCG_CARD_MR_FUJI && !a->slot))
            return Fail(g, "Choose one of your Pokemon (Mr. Fuji requires a Benched Pokemon).");
        if (!a->slot)
        {
            n = 0;
            for (i = 1; i < TCG_FIELD_SIZE; i++)
                n += p->field[i].stackSize != 0;
            if (!n)
                return Fail(g, "Your last Pokemon cannot be returned to your hand.");
        }
        f = &p->field[a->slot];
        if (id == TCG_CARD_MR_FUJI)
        {
            for (i = 0; i < f->stackSize; i++)
                p->deck[p->deckSize++] = f->stack[i];
            for (i = 0; i < f->attachedSize; i++)
                p->deck[p->deckSize++] = f->attached[i];
            memset(f, 0, sizeof(*f));
            TcgDuel_Shuffle(g, g->current);
        }
        else
        {
            unsigned basic = f->stack[0];
            for (i = 1; i < f->stackSize; i++)
                p->discard[p->discardSize++] = f->stack[i];
            for (i = 0; i < f->attachedSize; i++)
                p->discard[p->discardSize++] = f->attached[i];
            memset(f, 0, sizeof(*f));
            p->hand[p->handSize++] = basic;
        }
        break;
    case TCG_CARD_DEVOLUTION_SPRAY:
        if (a->slot >= TCG_FIELD_SIZE || p->field[a->slot].stackSize < 2 || a->attack < 1 || a->attack > 2)
            return Fail(g, "Choose an evolved Pokemon and a stage to discard.");
        f = &p->field[a->slot];
        n = f->stackSize;
        while (f->stackSize > 1 && Tcg_Card(p->cards[f->stack[f->stackSize - 1]])->stage >= a->attack)
            p->discard[p->discardSize++] = f->stack[--f->stackSize];
        if (n == f->stackSize)
            return TCG_INVALID;
        Cure(f);
        f->enteredTurn = g->turn;
        break;
    case TCG_CARD_POKEDEX:
        n = p->deckSize < 5 ? p->deckSize : 5;
        if (!n || a->selectionCount != n)
            return Fail(g, "Choose the order of the top cards of your deck.");
        for (i = 0; i < n; i++)
        {
            unsigned j;
            if (Index(p->deck + p->deckSize - n, n, a->selections[i]) < 0)
                return TCG_INVALID;
            for (j = 0; j < i; j++)
                if (a->selections[j] == a->selections[i])
                    return TCG_INVALID;
        }
        for (i = 0; i < n; i++)
            p->deck[p->deckSize - i - 1] = a->selections[i];
        break;
    default:
        snprintf(g->error, sizeof(g->error), "Trainer effect not yet ported: %s", Tcg_Card(id)->name);
        return TCG_UNSUPPORTED;
    }
    p->discard[p->discardSize++] = played;
    TcgDuel_ResolveKnockouts(g);
    TcgDuel_Log(g, "Player %u plays %s.", g->current + 1, Tcg_Card(id)->name);
    return TCG_OK;
}
#endif
