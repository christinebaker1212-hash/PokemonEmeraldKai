#include "mp_version.h"
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
/* Benched or evolved: no longer anything attacks or powers did to it */
static void Cure(struct TcgFieldCard *f)
{
    f->condition = TCG_HEALTHY;
    f->poison = 0;
    f->barrier = 0;
    f->noRetreat = 0;
    f->noAttack = 0;
    f->effectBy = 0;
    f->guard = f->weaken = f->charged = f->burn = f->shift = 0;
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
    TcgDuel_Event(g, TCG_EV_TRAINER, g->current, a->slot, 0, id, 0);
    switch (id)
    {
    case TCG_CARD_PROFESSOR_OAK:
        while (p->handSize)
            p->discard[p->discardSize++] = p->hand[--p->handSize];
        TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, 7), 0, 0);
        break;
    case TCG_CARD_IMPOSTER_PROFESSOR_OAK:
        while (op->handSize)
            op->deck[op->deckSize++] = op->hand[--op->handSize];
        TcgDuel_Shuffle(g, g->current ^ 1);
        TcgDuel_Event(g, TCG_EV_DRAW, g->current ^ 1, 0, TcgDuel_Draw(g, g->current ^ 1, 7), 0, 0);
        break;
    case TCG_CARD_BILL:
        if (!p->deckSize)
            return Fail(g, "Your deck is empty.");
        TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, 2), 0, 0);
        break;
    case TCG_CARD_IMAKUNI_CARD:
        if (!p->field[0].stackSize)
            return TCG_INVALID;
        p->field[0].condition = TCG_CONFUSED;
        TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, TCG_CONFUSED, 0, p->field[0].poison);
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
        n = f->damage > n ? n : f->damage;
        f->damage -= n;
        TcgDuel_Event(g, TCG_EV_HEAL, g->current, a->slot, n, 0, 0);
        break;
    case TCG_CARD_FULL_HEAL:
        if (!p->field[0].condition && !p->field[0].poison)
            return Fail(g, "Your Active Pokemon has no special condition.");
        /* Only "Asleep, Confused, Paralyzed, or Poisoned": not what other attacks left on it
         * (Withdraw's protection, Acid's "can't retreat") */
        p->field[0].condition = TCG_HEALTHY;
        p->field[0].poison = 0;
        TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, TCG_HEALTHY, 0, 0);
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
                TcgDuel_Event(g, TCG_EV_HEAL, g->current, i, f->damage, 0, 0);
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
        TcgDuel_Event(g, TCG_EV_SWITCH, q == p ? g->current : g->current ^ 1, s, 0, 0, 0);
        break;
    }
    case TCG_CARD_ENERGY_SEARCH:
    case TCG_CARD_POKE_BALL:
    case TCG_CARD_POKEBALL: /* (GB2's: the same) */
        if (a->selectionCount != 1 || a->selections[0] >= 60)
            return Fail(g, "Select a card from your deck.");
        card = Tcg_Card(p->cards[a->selections[0]]);
        if (id == TCG_CARD_ENERGY_SEARCH && card->id > 6)
            return Fail(g, "Energy Search requires a basic Energy card.");
        if ((id == TCG_CARD_POKE_BALL || id == TCG_CARD_POKEBALL) && card->kind != TCG_POKEMON)
            return Fail(g, "Poke Ball requires a Pokemon card.");
        at = Index(p->deck, p->deckSize, a->selections[0]);
        if (at < 0)
            return TCG_INVALID;
        if (id == TCG_CARD_ENERGY_SEARCH || TcgDuel_Coin(g, g->current))
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
        /* A trade: the Energy comes from what was already in the discard pile, not the
         * card just given up for it */
        for (i = n; i < a->selectionCount; i++)
            if (a->selections[i] >= 60 || p->cards[a->selections[i]] > 6 ||
                !Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[i]))
                return TCG_INVALID;
        for (i = 0; i < n; i++)
        {
            unsigned k;
            for (k = n; k < a->selectionCount; k++)
                if (a->selections[k] == a->selections[i])
                    return TCG_INVALID;
            if (!Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]))
                return TCG_INVALID;
        }
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
        TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, 1), 0, 0);
        break;
    case TCG_CARD_GAMBLER:
        while (p->handSize)
            p->deck[p->deckSize++] = p->hand[--p->handSize];
        TcgDuel_Shuffle(g, g->current);
        n = TcgDuel_Coin(g, g->current) ? 8 : 1;
        TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, n), 0, 0);
        break;
    case TCG_CARD_RECYCLE:
        if (a->selectionCount != 1 || Index(p->discard, p->discardSize, a->selections[0]) < 0)
            return Fail(g, "Select a card from your discard pile.");
        if (TcgDuel_Coin(g, g->current))
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
        if (TcgPower_InPlay(g, 2, TCG_POWER_PREHISTORIC))
            return Fail(g, "Aerodactyl's Prehistoric Power: no Evolution cards can be played.");
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
        TcgPower_OnPlay(g, g->current, a->slot); /* (Healing Wind) */
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
        {
            f->damage = (card->hp / 20) * 10;
            TcgPower_OnPlay(g, g->current, slot); /* (put into play during your turn) */
        }
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
        /* (as if it had evolved; it can evolve again this turn unless it already did) */
        Cure(f);
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
    /* ---------------- Pokemon Card GB2's ---------------- */
    case TCG_CARD_IMAKUNI_CARD_422:
        if (!p->field[0].stackSize)
            return TCG_INVALID;
        p->field[0].condition = TCG_CONFUSED;
        TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, TCG_CONFUSED, 0, p->field[0].poison);
        break;
    case TCG_CARD_FOSSIL_EXCAVATION:
    {
        /* a Mysterious Fossil from the deck or the discard pile (selections[0]; none: the first one) */
        int fromDeck = -1, fromDiscard = -1;
        for (i = 0; i < p->deckSize; i++)
            if (p->cards[p->deck[i]] == TCG_CARD_MYSTERIOUS_FOSSIL &&
                (a->selectionCount ? p->deck[i] == a->selections[0] : fromDeck < 0))
                fromDeck = (int)i;
        for (i = 0; i < p->discardSize; i++)
            if (p->cards[p->discard[i]] == TCG_CARD_MYSTERIOUS_FOSSIL &&
                (a->selectionCount ? p->discard[i] == a->selections[0] : fromDiscard < 0))
                fromDiscard = (int)i;
        if (fromDeck >= 0)
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)fromDeck);
        else if (fromDiscard >= 0)
            p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)fromDiscard);
        else if (a->selectionCount)
            return Fail(g, "Choose a Mysterious Fossil in your deck or discard pile.");
        if (fromDeck >= 0 || fromDiscard >= 0)
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, 1, 0, 0);
        TcgDuel_Shuffle(g, g->current);
        break;
    }
    case TCG_CARD_MAX_REVIVE:
    {
        /* selections: 2 Energy cards from the hand, then a Basic Pokemon from the discard pile; target: the Bench space */
        unsigned slot = target;
        if (a->selectionCount != 3)
            return Fail(g, "Discard 2 Energy cards, then choose a Basic Pokemon in your discard pile.");
        for (i = 0; i < 2; i++)
        {
            card = Tcg_Card(p->cards[a->selections[i]]);
            if (card->kind != TCG_ENERGY || a->selections[0] == a->selections[1] ||
                Index(p->hand, p->handSize, a->selections[i]) < 0)
                return Fail(g, "Discard 2 Energy cards from your hand.");
        }
        card = Tcg_Card(p->cards[a->selections[2]]);
        if (card->kind != TCG_POKEMON || card->stage || Index(p->discard, p->discardSize, a->selections[2]) < 0)
            return Fail(g, "Choose a Basic Pokemon in your discard pile.");
        if (slot < 1 || slot >= TCG_FIELD_SIZE || p->field[slot].stackSize)
            for (slot = 1; slot < TCG_FIELD_SIZE && p->field[slot].stackSize; slot++)
                ;
        if (slot >= TCG_FIELD_SIZE)
            return Fail(g, "Your Bench is full.");
        for (i = 0; i < 2; i++)
            Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]);
        f = &p->field[slot];
        memset(f, 0, sizeof(*f));
        Move(p->discard, &p->discardSize, f->stack, &f->stackSize, a->selections[2]);
        f->enteredTurn = g->turn;
        TcgDuel_Event(g, TCG_EV_PLAY, g->current, slot, 0, card->id, 0);
        break;
    }
    case TCG_CARD_SUPER_SCOOP_UP:
    {
        unsigned others = 0, k;
        if (target >= TCG_FIELD_SIZE || !p->field[target].stackSize)
            return Fail(g, "Choose one of your Pokemon.");
        for (k = 0; k < TCG_FIELD_SIZE; k++)
            others += k != target && p->field[k].stackSize;
        if (!others)
            return Fail(g, "It's your only Pokemon in play.");
        if (TcgDuel_Coin(g, g->current))
        {
            f = &p->field[target];
            for (i = 0; i < f->stackSize; i++)
                p->hand[p->handSize++] = f->stack[i];
            for (i = 0; i < f->attachedSize; i++)
                p->hand[p->handSize++] = f->attached[i];
            TcgDuel_Event(g, TCG_EV_TO_HAND, g->current, target, 0, p->cards[f->stack[f->stackSize - 1]], 0);
            memset(f, 0, sizeof(*f));
        }
        break;
    }
    case TCG_CARD_CHALLENGE:
    {
        /* They accept if they have room and Basic Pokemon to put down; then both fill their Benches with the
         * strongest Basic Pokemon of their decks. Otherwise: draw 2. */
        unsigned room[2] = {0, 0}, basics[2] = {0, 0};
        for (who = 0; who < 2; who++)
        {
            struct TcgPlayer *q = &g->players[who];
            for (i = 1; i < TCG_FIELD_SIZE; i++)
                room[who] += !q->field[i].stackSize;
            for (i = 0; i < q->deckSize; i++)
                basics[who] += Tcg_Card(q->cards[q->deck[i]])->kind == TCG_POKEMON && !Tcg_Card(q->cards[q->deck[i]])->stage;
        }
        if (!room[g->current ^ 1] || !basics[g->current ^ 1] || (!room[0] && !room[1]))
        {
            TcgDuel_Log(g, "Challenge! declined: draw 2 cards.");
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, 2), 0, 0);
            break;
        }
        TcgDuel_Log(g, "Challenge! accepted.");
        for (n = 0; n < 2; n++)
        {
            struct TcgPlayer *q;
            unsigned slot;
            who = g->current ^ n;
            q = &g->players[who];
            for (slot = 1; slot < TCG_FIELD_SIZE; slot++)
            {
                int best = -1;
                if (q->field[slot].stackSize)
                    continue;
                for (i = 0; i < q->deckSize; i++)
                {
                    const struct TcgCard *c = Tcg_Card(q->cards[q->deck[i]]);
                    if (c->kind == TCG_POKEMON && !c->stage && (best < 0 || c->hp > Tcg_Card(q->cards[q->deck[best]])->hp))
                        best = (int)i;
                }
                if (best < 0)
                    break;
                memset(&q->field[slot], 0, sizeof(q->field[slot]));
                q->field[slot].stack[q->field[slot].stackSize++] = TcgPile_Remove(q->deck, &q->deckSize, (unsigned)best);
                q->field[slot].enteredTurn = g->turn;
                TcgDuel_Event(g, TCG_EV_PLAY, who, slot, 0, q->cards[q->field[slot].stack[0]], 0);
            }
            TcgDuel_Shuffle(g, who);
        }
        break;
    }
    case TCG_CARD_MOON_STONE:
    case TCG_CARD_THE_BOSSS_WAY:
    {
        /* a Colorless Evolution card (Moon Stone) / a Dark Pokemon's Evolution card (The Boss's Way) from the deck */
        int pick = -1;
        if (!p->deckSize)
            return Fail(g, "Your deck is empty.");
        for (i = 0; i < p->deckSize; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->deck[i]]);
            int ok = c->kind == TCG_POKEMON && c->stage &&
                     (id == TCG_CARD_MOON_STONE ? c->type == TCG_COLORLESS : (c->dark || !strncmp(c->name, "Dark ", 5)));
            if (ok && (a->selectionCount ? p->deck[i] == a->selections[0] : pick < 0))
                pick = (int)i;
        }
        if (pick >= 0)
        {
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)pick);
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, 1, 0, 0);
            TcgDuel_Log(g, "%s to the hand.", Tcg_Card(p->cards[p->hand[p->handSize - 1]])->name);
        }
        else if (a->selectionCount)
            return Fail(g, "That card can't be taken.");
        TcgDuel_Shuffle(g, g->current);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current, 0, 0, 0, 0);
        break;
    }
    case TCG_CARD_IMPOSTER_OAKS_REVENGE:
        /* a card from the hand discarded; they shuffle their hand in and draw 4 */
        if (a->selectionCount != 1 || !Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[0]))
            return Fail(g, "Discard a card from your hand to play this.");
        while (op->handSize)
            op->deck[op->deckSize++] = op->hand[--op->handSize];
        TcgDuel_Shuffle(g, g->current ^ 1);
        TcgDuel_Event(g, TCG_EV_DRAW, g->current ^ 1, 0, TcgDuel_Draw(g, g->current ^ 1, 4), 0, 0);
        break;
    case TCG_CARD_SLEEP:
        if (op->field[0].stackSize && TcgDuel_Coin(g, g->current))
        {
            op->field[0].condition = TCG_ASLEEP;
            TcgPower_Sanitize(g);
            TcgDuel_Event(g, TCG_EV_STATUS, g->current ^ 1, 0, op->field[0].condition, 0, op->field[0].poison);
        }
        break;
    case TCG_CARD_COMPUTER_ERROR:
        /* up to 5 each (always leaving a card in each deck), then the turn is over */
        for (n = 0; n < 2; n++)
        {
            struct TcgPlayer *q = &g->players[g->current ^ n];
            unsigned k = q->deckSize > 5 ? 5 : q->deckSize ? q->deckSize - 1 : 0;
            if (k)
                TcgDuel_Event(g, TCG_EV_DRAW, g->current ^ n, 0, TcgDuel_Draw(g, g->current ^ n, k), 0, 0);
        }
        g->turnOver = 1;
        break;
    case TCG_CARD_DIGGER:
    {
        /* coins in turn until someone flips tails: that player's Active takes 10 */
        unsigned flipper = g->current;
        for (n = 0; n < 60; n++, flipper ^= 1)
            if (!TcgDuel_Coin(g, flipper))
            {
                struct TcgFieldCard *hit = &g->players[flipper].field[0];
                if (hit->stackSize)
                {
                    hit->damage += 10;
                    TcgDuel_Event(g, TCG_EV_DAMAGE, flipper, 0, 10, 0, 0);
                }
                break;
            }
        break;
    }
    case TCG_CARD_POKEMON_RECALL:
    {
        /* an Evolution card from the discard pile on top of the deck */
        int pick = -1;
        for (i = 0; i < p->discardSize; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->discard[i]]);
            if (c->kind == TCG_POKEMON && c->stage && (a->selectionCount ? p->discard[i] == a->selections[0] : pick < 0))
                pick = (int)i;
        }
        if (pick < 0)
            return Fail(g, "Choose an Evolution card in your discard pile.");
        p->deck[p->deckSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)pick);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current, 0, 0, 0, 0);
        break;
    }
    case TCG_CARD_GOOP_GAS_ATTACK:
        g->goopGasUntil = (uint16_t)(g->turn + 1);
        TcgDuel_Log(g, "Every Pokemon Power stops working until the end of the next turn.");
        break;
    case TCG_CARD_BILLS_TELEPORTER:
        if (TcgDuel_Coin(g, g->current))
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, 4), 0, 0);
        break;
    case TCG_CARD_BILLS_COMPUTER:
        return Fail(g, "Bill's PC is used for trades, not in a duel.");
    case TCG_CARD_MASTER_BALL:
    {
        /* a Pokemon card from the top 7 of the deck (selections[0], optional), the rest shuffled in */
        unsigned top = p->deckSize < 7 ? p->deckSize : 7;
        if (!p->deckSize)
            return Fail(g, "Your deck is empty.");
        if (a->selectionCount)
        {
            int at = Index(p->deck, p->deckSize, a->selections[0]);
            if (at < 0 || at < (int)(p->deckSize - top) || Tcg_Card(p->cards[a->selections[0]])->kind != TCG_POKEMON)
                return Fail(g, "Choose a Pokemon card among the top 7 cards.");
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, 1, 0, 0);
        }
        TcgDuel_Shuffle(g, g->current);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current, 0, 0, 0, 0);
        break;
    }
    case TCG_CARD_NIGHTLY_GARBAGE_RUN:
        /* up to 3 Pokemon and basic Energy cards from the discard pile into the deck */
        if (!a->selectionCount || a->selectionCount > 3)
            return Fail(g, "Choose up to 3 Pokemon or basic Energy cards in your discard pile.");
        for (i = 0; i < a->selectionCount; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[a->selections[i]]);
            if ((c->kind != TCG_POKEMON && !(c->kind == TCG_ENERGY && c->id <= 6)) ||
                !Move(p->discard, &p->discardSize, p->deck, &p->deckSize, a->selections[i]))
                return Fail(g, "Choose up to 3 Pokemon or basic Energy cards in your discard pile.");
        }
        TcgDuel_Shuffle(g, g->current);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current, 0, 0, 0, 0);
        break;
    case TCG_CARD_ROCKETS_SNEAK_ATTACK:
    {
        /* their hand looked at: a Trainer there (the rarest) goes into their deck */
        int pick = -1;
        for (i = 0; i < op->handSize; i++)
        {
            const struct TcgCard *c = Tcg_Card(op->cards[op->hand[i]]);
            if (c->kind == TCG_TRAINER && (pick < 0 || c->rarity > Tcg_Card(op->cards[op->hand[pick]])->rarity))
                pick = (int)i;
        }
        if (pick >= 0)
        {
            TcgDuel_Log(g, "%s from their hand went into their deck.", Tcg_Card(op->cards[op->hand[pick]])->name);
            op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, (unsigned)pick);
            TcgDuel_Shuffle(g, g->current ^ 1);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current ^ 1, 0, 0, 0, 0);
        }
        else
            TcgDuel_Log(g, "There's no Trainer card in their hand.");
        break;
    }
    case TCG_CARD_HERE_COMES_TEAM_ROCKET:
        p->prizesUp = op->prizesUp = 1;
        TcgDuel_Log(g, "Both players' Prize cards are face up for the rest of the duel.");
        break;
    case TCG_CARD_THE_ROCKETS_TRAP:
        if (TcgDuel_Coin(g, g->current))
        {
            for (n = 0; n < 3 && op->handSize; n++)
                op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, Tcg_RandomBelow(&g->rng, op->handSize));
            if (n)
            {
                TcgDuel_Shuffle(g, g->current ^ 1);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current ^ 1, 0, 0, 0, 0);
                TcgDuel_Log(g, "%u cards from their hand went into their deck.", n);
            }
        }
        break;
    default:
        snprintf(g->error, sizeof(g->error), "Trainer effect not yet ported: %s", Tcg_Card(id)->name);
        return TCG_UNSUPPORTED;
    }
    p->discard[p->discardSize++] = played;
    TcgPower_Sanitize(g); /* (Imakuni? on Snorlax) */
    TcgDuel_ResolveKnockouts(g);
    TcgDuel_Log(g, "Player %u plays %s.", g->current + 1, Tcg_Card(id)->name);
    return TCG_OK;
}
