/* The regional sets' Trainers (docs/TCG_REGIONS.md): their rules, after the Game Boy games' in TcgDuel_Trainer
 * (included by tcg_trainers.c for its helpers). id is the card's first printing (Tcg_Canon); the action's slot /
 * target are the Pokemon chosen and its selections the cards picked, in the order of the card's steps
 * (tcg_rtrainers_flow.inc). A Supporter, a Stadium and a Pokemon Tool are put where they go by TcgDuel_Trainer;
 * RT_PLACED: the card was put somewhere here (not discarded). The opponent's "may" (draw up to, guess...) is decided
 * for them as a sensible player would: noted with each card. */
#define RT_PLACED 100
static int RtNamed(const struct TcgCard *c, const char *word) { return c && strstr(c->name, word) != NULL; }
static int RtPokemon(const struct TcgCard *c) { return c && c->kind == TCG_POKEMON; }
static int RtBasic(const struct TcgCard *c) { return RtPokemon(c) && !c->stage; }
static int RtEvolution(const struct TcgCard *c) { return RtPokemon(c) && c->stage; }
static int RtBasicNotBaby(const struct TcgCard *c) { return RtBasic(c) && !(c->flags & TCG_CF_BABY); }
static int RtTrainer(const struct TcgCard *c) { return c && c->kind == TCG_TRAINER; }
static int RtEnergy(const struct TcgCard *c) { return c && c->kind == TCG_ENERGY; }
static int RtBasicEnergy(const struct TcgCard *c) { return c && c->kind == TCG_ENERGY && c->id >= 1 && c->id <= 6; }
static int RtFireEnergy(const struct TcgCard *c) { return c && c->id == TCG_CARD_FIRE_ENERGY; }
static int RtWaterEnergy(const struct TcgCard *c) { return c && c->id == TCG_CARD_WATER_ENERGY; }
static int RtBrock(const struct TcgCard *c) { return RtPokemon(c) && RtNamed(c, "Brock"); }
static int RtErika(const struct TcgCard *c) { return RtPokemon(c) && RtNamed(c, "Erika"); }
static int RtUnown(const struct TcgCard *c) { return RtPokemon(c) && RtNamed(c, "Unown"); }
static int RtGarbage(const struct TcgCard *c) { return RtPokemon(c) || RtBasicEnergy(c); }
static int RtSupporterStadium(const struct TcgCard *c)
{
    return RtTrainer(c) && (c->flags & (TCG_CF_SUPPORTER | TCG_CF_STADIUM));
}
static int RtTmTool(const struct TcgCard *c) { return RtTrainer(c) && (c->flags & (TCG_CF_TM | TCG_CF_TOOL)); }
static int RtBall(const struct TcgCard *c) { return RtTrainer(c) && strstr(c->name, "Ball") != NULL; }
static int RtAny(const struct TcgCard *c) { return c != NULL; }
static unsigned RtHeal(struct TcgDuel *g, unsigned who, unsigned slot, unsigned amount)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned n = f->damage < amount ? f->damage : amount;
    if (!f->stackSize || !n)
        return 0;
    f->damage -= n;
    TcgDuel_Event(g, TCG_EV_HEAL, who, slot, n, 0, 0);
    return n;
}
static int RtEmptyBench(const struct TcgPlayer *p)
{
    unsigned s;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        if (!p->field[s].stackSize)
            return (int)s;
    return -1;
}
/* Selections [from, from + n): distinct cards of that pile, each one ok() */
static int RtPicked(const struct TcgPlayer *p, const uint8_t *pile, unsigned count, const struct TcgAction *a,
                    unsigned from, unsigned n, int (*ok)(const struct TcgCard *))
{
    unsigned i, j;
    for (i = from; i < from + n; i++)
    {
        if (i >= a->selectionCount || a->selections[i] >= TCG_DECK_SIZE || Index(pile, count, a->selections[i]) < 0)
            return 0;
        for (j = from; j < i; j++)
            if (a->selections[j] == a->selections[i])
                return 0;
        if (ok && !ok(Tcg_Card(p->cards[a->selections[i]])))
            return 0;
    }
    return 1;
}
static unsigned RtCount(const struct TcgPlayer *p, const uint8_t *pile, unsigned count, int (*ok)(const struct TcgCard *))
{
    unsigned i, n = 0;
    for (i = 0; i < count; i++)
        n += ok(Tcg_Card(p->cards[pile[i]]));
    return n;
}
static void RtDraw(struct TcgDuel *g, unsigned who, unsigned n)
{
    if (n)
        TcgDuel_Event(g, TCG_EV_DRAW, who, 0, TcgDuel_Draw(g, who, n), 0, 0);
}
static void RtHandIntoDeck(struct TcgDuel *g, unsigned who)
{
    struct TcgPlayer *q = &g->players[who];
    while (q->handSize)
        q->deck[q->deckSize++] = q->hand[--q->handSize];
    TcgDuel_Shuffle(g, who);
    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
}
/* (the opponent's "may draw up to n": as many as leave their deck a card) */
static unsigned RtTheirDraw(const struct TcgPlayer *q, unsigned n)
{
    unsigned room = q->deckSize > 1 ? q->deckSize - 1u : 0;
    return n < room ? n : room;
}
static unsigned RtFlips(struct TcgDuel *g, unsigned n)
{
    unsigned heads = 0;
    while (n--)
        heads += TcgDuel_Coin(g, g->current) ? 1 : 0;
    return heads;
}
static unsigned RtUntilTails(struct TcgDuel *g)
{
    unsigned heads = 0;
    while (heads < 60 && TcgDuel_Coin(g, g->current))
        heads++;
    return heads;
}
/* The card back into its owner's hand (Pokemon and all its cards: Minion of Team Rocket) */
static void RtReturnToHand(struct TcgDuel *g, unsigned who, unsigned slot)
{
    struct TcgPlayer *q = &g->players[who];
    struct TcgFieldCard *f = &q->field[slot];
    unsigned i;
    for (i = 0; i < f->stackSize; i++)
        q->hand[q->handSize++] = f->stack[i];
    for (i = 0; i < f->attachedSize; i++)
        q->hand[q->handSize++] = f->attached[i];
    memset(f, 0, sizeof(*f));
}
/* Put a Basic Pokemon card from that pile onto the Bench (Pokemon Fan Club, Ruin Wall, Erika's Perfume) */
static int RtBench(struct TcgDuel *g, unsigned who, uint8_t *pile, uint8_t *count, unsigned card)
{
    struct TcgPlayer *q = &g->players[who];
    int s = RtEmptyBench(q), at = Index(pile, *count, card);
    if (s < 0 || at < 0)
        return 0;
    memset(&q->field[s], 0, sizeof(q->field[s]));
    q->field[s].stack[q->field[s].stackSize++] = TcgPile_Remove(pile, count, (unsigned)at);
    q->field[s].enteredTurn = g->turn;
    TcgDuel_Event(g, TCG_EV_PLAY, who, (unsigned)s, 0, q->cards[card], 0);
    return 1;
}
static void RtDiscardEnergy(struct TcgDuel *g, unsigned who, unsigned slot)
{
    struct TcgPlayer *q = &g->players[who];
    struct TcgFieldCard *f = &q->field[slot];
    unsigned j;
    for (j = f->attachedSize; j > 0; j--)
        if (RtEnergy(Tcg_Card(q->cards[f->attached[j - 1]])))
            q->discard[q->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, j - 1);
}

static int RegionTrainer(struct TcgDuel *g, const struct TcgAction *a, unsigned played, unsigned id)
{
    struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    unsigned me = g->current, i, n = 0, s;
    struct TcgFieldCard *f;
    (void)played;
    switch (id)
    {
    case TCG_CARD_GYM1_15: /* Brock: 1 counter off each of yours */
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            n += RtHeal(g, me, s, 10);
        if (!n)
            return Fail(g, "None of your Pokemon are damaged.");
        break;
    case TCG_CARD_GYM1_16: /* Erika: you draw up to 3 (action.target), then they may (as many as leave a card) */
        if (a->target > 3)
            return TCG_INVALID;
        RtDraw(g, me, a->target);
        RtDraw(g, me ^ 1, RtTheirDraw(op, 3));
        break;
    case TCG_CARD_GYM1_17: /* Lt. Surge: a Basic from the hand becomes the Active, the old one goes to the Bench */
    {
        int bench = RtEmptyBench(p), at;
        if (bench < 0 || !RtPicked(p, p->hand, p->handSize, a, 0, 1, RtBasic))
            return Fail(g, "Choose a Basic Pokemon in your hand (and have room on your Bench).");
        if (TcgDuel_Anchored(g, me))
            return Fail(g, "Your Active Pokemon can't be switched out now.");
        p->field[bench] = p->field[0];
        Cure(&p->field[bench]);
        memset(&p->field[0], 0, sizeof(p->field[0]));
        at = Index(p->hand, p->handSize, a->selections[0]);
        p->field[0].stack[p->field[0].stackSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
        p->field[0].enteredTurn = g->turn;
        TcgDuel_Event(g, TCG_EV_PLAY, me, 0, 0, p->cards[a->selections[0]], 0);
        break;
    }
    case TCG_CARD_GYM1_18: /* Misty: discard 2 others; a Misty Pokemon's damage this turn 20 more */
        if (!RtPicked(p, p->hand, p->handSize, a, 0, 2, RtAny))
            return Fail(g, "Choose two other cards from your hand to discard.");
        for (i = 0; i < 2; i++)
            Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]);
        p->turnBuffs |= TCG_BUFF_MISTY;
        break;
    case TCG_CARD_GYM1_105: /* Blaine's Last Resort: the only card in your hand; draw 5 */
        if (p->handSize)
            return Fail(g, "Blaine's Last Resort must be the only card in your hand.");
        RtDraw(g, me, 5);
        break;
    case TCG_CARD_GYM1_106: /* Brock's Training Method: a Brock Pokemon from the deck */
        if (a->selectionCount && !RtPicked(p, p->deck, p->deckSize, a, 0, 1, RtBrock))
            return TCG_INVALID;
        if (a->selectionCount)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_GYM1_109: /* Erika's Maids: 2 others for up to 2 Erika Pokemon from the deck */
        if (!RtPicked(p, p->hand, p->handSize, a, 0, 2, RtAny) || a->selectionCount > 4 ||
            !RtPicked(p, p->deck, p->deckSize, a, 2, a->selectionCount - 2u, RtErika))
            return Fail(g, "Choose two other cards from your hand, then up to two Erika Pokemon.");
        for (i = 0; i < 2; i++)
            Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]);
        for (i = 2; i < a->selectionCount; i++)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_GYM1_110: /* Erika's Perfume: their Basics from their hand onto their Bench */
        if (!RtPicked(op, op->hand, op->handSize, a, 0, a->selectionCount, RtBasic))
            return TCG_INVALID;
        for (i = 0; i < a->selectionCount; i++)
            if (!RtBench(g, me ^ 1, op->hand, &op->handSize, a->selections[i]))
                return Fail(g, "There's no room on their Bench.");
        break;
    case TCG_CARD_GYM1_111: /* Good Manners: no Basic in your hand; a Basic from the deck */
        if (RtCount(p, p->hand, p->handSize, RtBasic))
            return Fail(g, "You can't play this with a Basic Pokemon in your hand.");
        if (a->selectionCount && !RtPicked(p, p->deck, p->deckSize, a, 0, 1, RtBasic))
            return TCG_INVALID;
        if (a->selectionCount)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_GYM1_113: /* Minion of Team Rocket: both heads, their chosen Benched Pokemon back to their hand */
        if (a->target < 1 || a->target >= TCG_FIELD_SIZE || !op->field[a->target].stackSize)
            return Fail(g, "Choose one of their Benched Pokemon.");
        if (RtFlips(g, 2) == 2)
            RtReturnToHand(g, me ^ 1, a->target);
        else
            g->turnOver = 1;
        break;
    case TCG_CARD_GYM1_114: /* Misty's Wrath: 2 of the top 7 into your hand, the rest discarded */
    {
        unsigned top = p->deckSize < 7 ? p->deckSize : 7, want = top < 2 ? top : 2;
        uint8_t look[7];
        for (i = 0; i < top; i++)
            look[i] = p->deck[p->deckSize - 1 - i];
        if (a->selectionCount != want || !RtPicked(p, look, top, a, 0, want, RtAny))
            return Fail(g, "Choose two of the top 7 cards.");
        for (i = 0; i < top; i++)
        {
            uint8_t card = p->deck[--p->deckSize];
            if (Index(a->selections, a->selectionCount, card) >= 0)
                p->hand[p->handSize++] = card;
            else
                p->discard[p->discardSize++] = card;
        }
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, top - want, 0, 0);
        break;
    }
    case TCG_CARD_GYM1_118: /* Secret Mission: discard any of yours, draw as many */
    case TCG_CARD_GYM1_121: /* Blaine's Gamble: discard any of yours; heads, draw twice as many */
        if (!RtPicked(p, p->hand, p->handSize, a, 0, a->selectionCount, RtAny))
            return TCG_INVALID;
        for (i = 0; i < a->selectionCount; i++)
            Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]);
        if (id == TCG_CARD_GYM1_118)
            RtDraw(g, me, a->selectionCount);
        else if (TcgDuel_Coin(g, me))
            RtDraw(g, me, 2u * a->selectionCount);
        break;
    case TCG_CARD_GYM1_122: /* Energy Flow: Energy from your Pokemon back to your hand */
        for (i = 0; i < a->selectionCount; i++)
        {
            int found = 0;
            for (s = 0; s < TCG_FIELD_SIZE && !found; s++)
            {
                f = &p->field[s];
                if (Index(f->attached, f->attachedSize, a->selections[i]) >= 0 &&
                    RtEnergy(Tcg_Card(p->cards[a->selections[i]])))
                    found = Move(f->attached, &f->attachedSize, p->hand, &p->handSize, a->selections[i]);
            }
            if (!found)
                return TCG_INVALID;
        }
        break;
    case TCG_CARD_GYM1_123: /* Misty's Duel: (a coin for Rock-Paper-Scissors) the winner draws a new hand of 5 */
    {
        unsigned winner = TcgDuel_Coin(g, me) ? me : me ^ 1;
        RtHandIntoDeck(g, winner);
        RtDraw(g, winner, 5);
        TcgDuel_Log(g, "Player %u won Misty's Duel.", winner + 1);
        break;
    }
    case TCG_CARD_GYM1_125: /* Sabrina's Gaze: each player a new hand of as many cards */
    {
        unsigned who, had[2] = {p->handSize, op->handSize};
        for (who = 0; who < 2; who++)
        {
            unsigned w = who ? me ^ 1 : me;
            RtHandIntoDeck(g, w);
            RtDraw(g, w, had[who]);
        }
        break;
    }
    case TCG_CARD_GYM1_126: /* Trash Exchange: the discard pile into the deck, then as many off the top discarded */
        n = p->discardSize;
        while (p->discardSize)
            p->deck[p->deckSize++] = p->discard[--p->discardSize];
        TcgDuel_Shuffle(g, me);
        for (i = 0; i < n && p->deckSize; i++)
            p->discard[p->discardSize++] = p->deck[--p->deckSize];
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, n, 0, 0);
        break;
    case TCG_CARD_GYM2_17: /* Blaine: instead of the turn's Energy, 2 Fire Energy onto a Blaine Pokemon */
        if (p->energyPlayed)
            return Fail(g, "You already attached Energy this turn.");
        if (a->slot >= TCG_FIELD_SIZE || !RtNamed(TcgDuel_Field(g, me, a->slot), "Blaine") ||
            !RtPicked(p, p->hand, p->handSize, a, 0, 2, RtFireEnergy) || a->selectionCount != 2)
            return Fail(g, "Choose a Pokemon with Blaine in its name and 2 Fire Energy cards in your hand.");
        f = &p->field[a->slot];
        for (i = 0; i < 2; i++)
        {
            Move(p->hand, &p->handSize, f->attached, &f->attachedSize, a->selections[i]);
            TcgDuel_Event(g, TCG_EV_ATTACH, me, a->slot, 0, TCG_CARD_FIRE_ENERGY, 0);
        }
        p->energyPlayed = 1;
        break;
    case TCG_CARD_GYM2_19: /* Koga: a Koga Pokemon's damage this turn Poisons */
        p->turnBuffs |= TCG_BUFF_KOGA;
        break;
    case TCG_CARD_GYM2_20: /* Sabrina: all Energy from one Sabrina Pokemon to another */
    {
        struct TcgFieldCard *to;
        if (a->slot >= TCG_FIELD_SIZE || a->target >= TCG_FIELD_SIZE || a->slot == a->target ||
            !RtNamed(TcgDuel_Field(g, me, a->slot), "Sabrina") || !RtNamed(TcgDuel_Field(g, me, a->target), "Sabrina"))
            return Fail(g, "Choose two Pokemon with Sabrina in their names.");
        f = &p->field[a->slot];
        to = &p->field[a->target];
        for (i = f->attachedSize; i > 0; i--)
            if (RtEnergy(Tcg_Card(p->cards[f->attached[i - 1]])))
            {
                to->attached[to->attachedSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
                TcgDuel_Event(g, TCG_EV_ATTACH, me, a->target, 0, p->cards[to->attached[to->attachedSize - 1]], 0);
            }
        break;
    }
    case TCG_CARD_GYM2_103: /* Erika's Kindness: 2 counters off every Pokemon */
    {
        unsigned who;
        for (who = 0; who < 2; who++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                n += RtHeal(g, who, s, 20);
        break;
    }
    case TCG_CARD_GYM2_105: /* Giovanni's Last Resort: every counter off a Giovanni Pokemon, then discard your hand */
        if (a->slot >= TCG_FIELD_SIZE || !RtNamed(TcgDuel_Field(g, me, a->slot), "Giovanni"))
            return Fail(g, "Choose a Pokemon with Giovanni in its name.");
        RtHeal(g, me, a->slot, p->field[a->slot].damage);
        while (p->handSize)
            p->discard[p->discardSize++] = p->hand[--p->handSize];
        break;
    case TCG_CARD_GYM2_118: /* Misty's Tears: discard 1 other, up to 2 Water Energy from the deck */
        if (!RtPicked(p, p->hand, p->handSize, a, 0, 1, RtAny) || a->selectionCount > 3 ||
            !RtPicked(p, p->deck, p->deckSize, a, 1, a->selectionCount - 1u, RtWaterEnergy))
            return Fail(g, "Choose another card to discard, then up to 2 Water Energy.");
        Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[0]);
        for (i = 1; i < a->selectionCount; i++)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_GYM2_120: /* Rocket's Secret Experiment: heads, any card from the deck; tails, no Trainers to the
                               end of your next turn */
        if (a->selectionCount > 1 || (a->selectionCount && !RtPicked(p, p->deck, p->deckSize, a, 0, 1, RtAny)))
            return TCG_INVALID;
        if (TcgDuel_Coin(g, me))
        {
            if (a->selectionCount)
                Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
            TcgDuel_Shuffle(g, me);
        }
        else
        {
            p->noTrainers = 1;
            p->noTrainersTurn = (uint16_t)(g->turn + 2);
        }
        break;
    case TCG_CARD_GYM2_124: /* Fervor: the top 3's Fire Energy into your hand, the rest discarded */
        for (i = 0; i < 3 && p->deckSize; i++)
        {
            uint8_t card = p->deck[--p->deckSize];
            if (RtFireEnergy(Tcg_Card(p->cards[card])))
                p->hand[p->handSize++] = card;
            else
                p->discard[p->discardSize++] = card;
        }
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, i, 0, 0);
        break;
    case TCG_CARD_NEO1_83: /* Arcade Game: the top 3; two or more of a name go to your hand */
    {
        uint8_t top[3];
        unsigned k, took = 0;
        TcgDuel_Shuffle(g, me);
        n = p->deckSize < 3 ? p->deckSize : 3;
        for (i = 0; i < n; i++)
            top[i] = p->deck[p->deckSize - 1 - i];
        for (i = 0; i < n && !took; i++)
        {
            unsigned same = 0;
            for (k = 0; k < n; k++)
                same += !strcmp(Tcg_Card(p->cards[top[k]])->name, Tcg_Card(p->cards[top[i]])->name);
            if (same >= 2)
            {
                const char *name = Tcg_Card(p->cards[top[i]])->name;
                for (k = 0; k < n; k++)
                    if (!strcmp(Tcg_Card(p->cards[top[k]])->name, name))
                    {
                        Move(p->deck, &p->deckSize, p->hand, &p->handSize, top[k]);
                        took++;
                    }
            }
        }
        TcgDuel_Log(g, took ? "Arcade Game: a match!" : "Arcade Game: no match.");
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_NEO1_85: /* Energy Charge: heads, up to 2 Energy from the discard pile into the deck */
    case TCG_CARD_ECARD1_147: /* Power Charge: the same, 2 (1 if only 1) */
        if (a->selectionCount > 2 || !RtPicked(p, p->discard, p->discardSize, a, 0, a->selectionCount, RtEnergy))
            return TCG_INVALID;
        if (TcgDuel_Coin(g, me))
        {
            for (i = 0; i < a->selectionCount; i++)
                Move(p->discard, &p->discardSize, p->deck, &p->deckSize, a->selections[i]);
            TcgDuel_Shuffle(g, me);
        }
        break;
    case TCG_CARD_NEO1_88: /* PokeGear: a Trainer from the top 7; no more Trainers this turn */
    {
        unsigned top = p->deckSize < 7 ? p->deckSize : 7;
        uint8_t look[7];
        for (i = 0; i < top; i++)
            look[i] = p->deck[p->deckSize - 1 - i];
        if (a->selectionCount > 1 || !RtPicked(p, look, top, a, 0, a->selectionCount, RtTrainer))
            return TCG_INVALID;
        if (a->selectionCount)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
        TcgDuel_Shuffle(g, me);
        p->noTrainers = 1;
        break;
    }
    case TCG_CARD_NEO1_96: /* Professor Elm: hand into the deck, draw 7; no more Trainers this turn */
        RtHandIntoDeck(g, me);
        RtDraw(g, me, 7);
        p->noTrainers = 1;
        break;
    case TCG_CARD_NEO1_101: /* Moo-Moo Milk: 2 coins, 2 counters off per heads */
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].stackSize)
            return Fail(g, "Choose one of your Pokemon.");
        RtHeal(g, me, a->slot, 20 * RtFlips(g, 2));
        break;
    case TCG_CARD_NEO1_103: /* Super Rod: heads, the Evolution card chosen; tails, the Basic chosen */
    case TCG_CARD_NEO3_64: /* Old Rod: both heads, the Pokemon card chosen; both tails, the Trainer chosen */
        if (a->selectionCount > 2 || !RtPicked(p, p->discard, p->discardSize, a, 0, a->selectionCount, RtAny))
            return TCG_INVALID;
        {
            int (*want)(const struct TcgCard *) = NULL;
            if (id == TCG_CARD_NEO1_103)
                want = TcgDuel_Coin(g, me) ? RtEvolution : RtBasic;
            else
            {
                unsigned heads = RtFlips(g, 2);
                want = heads == 2 ? RtPokemon : heads == 0 ? RtTrainer : NULL;
            }
            for (i = 0; want && i < a->selectionCount; i++)
                if (want(Tcg_Card(p->cards[a->selections[i]])))
                {
                    Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[i]);
                    break;
                }
        }
        break;
    case TCG_CARD_NEO2_73: /* Hyper Devolution Spray: the top Evolution card back to the hand */
        if (a->slot >= TCG_FIELD_SIZE || p->field[a->slot].stackSize < 2)
            return Fail(g, "Choose one of your evolved Pokemon.");
        f = &p->field[a->slot];
        p->hand[p->handSize++] = f->stack[--f->stackSize];
        f->enteredTurn = g->turn; /* (it can't evolve this turn) */
        Cure(f);
        TcgDuel_Event(g, TCG_EV_DEVOLVE, me, a->slot, 0, 0, 0);
        break;
    case TCG_CARD_NEO2_74: /* Ruin Wall: an Unown from the deck onto the Bench */
        if (RtEmptyBench(p) < 0)
            return Fail(g, "Your Bench is full.");
        if (a->selectionCount > 1 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, RtUnown))
            return TCG_INVALID;
        if (a->selectionCount)
            RtBench(g, me, p->deck, &p->deckSize, a->selections[0]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_NEO2_75: /* Energy Ark: 2 coins, a basic Energy per heads (of those chosen) */
    case TCG_CARD_ECARD1_139: /* Dual Ball: 2 coins, a Basic (not a Baby) per heads */
    {
        int (*ok)(const struct TcgCard *) = id == TCG_CARD_NEO2_75 ? RtBasicEnergy : RtBasicNotBaby;
        if (a->selectionCount > 2 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, ok))
            return TCG_INVALID;
        n = RtFlips(g, 2);
        for (i = 0; i < a->selectionCount && i < n; i++)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_NEO4_96: /* Thought Wave Machine: an Energy of their Active's back to their hand per heads; the turn
                              is over */
        n = RtUntilTails(g);
        f = &op->field[0];
        for (i = f->attachedSize; i > 0 && n; i--)
            if (RtEnergy(Tcg_Card(op->cards[f->attached[i - 1]])))
            {
                op->hand[op->handSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
                n--;
            }
        g->turnOver = 1;
        break;
    case TCG_CARD_NEO4_98: /* Energy Amplifier: an Energy from the hand into the deck; heads, up to 3 basic Energy */
        if (!RtPicked(p, p->hand, p->handSize, a, 0, 1, RtEnergy) || a->selectionCount > 4 ||
            !RtPicked(p, p->deck, p->deckSize, a, 1, a->selectionCount - 1u, RtBasicEnergy))
            return Fail(g, "Choose an Energy card in your hand, then up to 3 basic Energy.");
        Move(p->hand, &p->handSize, p->deck, &p->deckSize, a->selections[0]);
        if (TcgDuel_Coin(g, me))
            for (i = 1; i < a->selectionCount; i++)
                Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_NEO4_101: /* Magnifier: attached until the end of the turn; its attack ignores Resistance */
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].stackSize)
            return Fail(g, "Choose one of your Pokemon.");
        f = &p->field[a->slot];
        f->attached[f->attachedSize++] = (uint8_t)played;
        p->attachmentExpires[played] = g->turn;
        return RT_PLACED;
    case TCG_CARD_NEO4_103: /* Team Rocket's Evil Deeds: a card of their hand into their deck; they may draw 2 */
        if (op->handSize && !RtPicked(op, op->hand, op->handSize, a, 0, 1, RtAny))
            return Fail(g, "Choose a card in their hand.");
        if (op->handSize)
        {
            Move(op->hand, &op->handSize, op->deck, &op->deckSize, a->selections[0]);
            TcgDuel_Shuffle(g, me ^ 1);
        }
        RtDraw(g, me ^ 1, RtTheirDraw(op, 2));
        break;
    case TCG_CARD_NEO4_104: /* Heal Powder: heads, the Active cured and 2 counters off */
        if (TcgDuel_Coin(g, me))
        {
            Cure(&p->field[0]);
            TcgDuel_Event(g, TCG_EV_STATUS, me, 0, TCG_HEALTHY, 0, 0);
            RtHeal(g, me, 0, 20);
        }
        break;
    case TCG_CARD_NEO4_105: /* Mail from Bill: fewer than 5 in the hand; draw to 4 */
        if (p->handSize >= 4)
            return Fail(g, "You have too many cards in your hand.");
        RtDraw(g, me, 4u - p->handSize);
        break;
    case TCG_CARD_ECARD1_137: /* Bill's Maintenance: one from the hand into the deck, draw 3 */
        if (p->handSize)
        {
            if (!RtPicked(p, p->hand, p->handSize, a, 0, 1, RtAny))
                return Fail(g, "Choose a card to shuffle into your deck.");
            Move(p->hand, &p->handSize, p->deck, &p->deckSize, a->selections[0]);
            TcgDuel_Shuffle(g, me);
            RtDraw(g, me, 3);
        }
        break;
    case TCG_CARD_ECARD1_138: /* Copycat: hand into the deck, draw as many as they hold */
        n = op->handSize;
        RtHandIntoDeck(g, me);
        RtDraw(g, me, n);
        break;
    case TCG_CARD_ECARD1_140: /* Energy Removal 2: heads, the Energy chosen discarded */
        f = &op->field[a->target < TCG_FIELD_SIZE ? a->target : 0];
        if (a->target >= TCG_FIELD_SIZE || !RtPicked(op, f->attached, f->attachedSize, a, 0, 1, RtEnergy))
            return Fail(g, "Choose an Energy card on one of their Pokemon.");
        if (TcgDuel_Coin(g, me))
            Move(f->attached, &f->attachedSize, op->discard, &op->discardSize, a->selections[0]);
        break;
    case TCG_CARD_ECARD1_141: /* Energy Restore: 3 coins, a basic Energy from the discard pile per heads */
    case TCG_CARD_ECARD3_128: /* Lure Ball: 3 coins, an Evolution card from the discard pile per heads */
    {
        int (*ok)(const struct TcgCard *) = id == TCG_CARD_ECARD1_141 ? RtBasicEnergy : RtEvolution;
        if (a->selectionCount > 3 || !RtPicked(p, p->discard, p->discardSize, a, 0, a->selectionCount, ok))
            return TCG_INVALID;
        n = RtFlips(g, 3);
        for (i = 0; i < a->selectionCount && i < n; i++)
            Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[i]);
        break;
    }
    case TCG_CARD_ECARD1_142: /* Mary's Impulse: 2 cards per heads until tails */
        RtDraw(g, me, 2 * RtUntilTails(g));
        break;
    case TCG_CARD_ECARD1_145: /* Pokemon Nurse: every counter off one of yours, its Energy discarded */
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].damage)
            return Fail(g, "Choose one of your damaged Pokemon.");
        RtHeal(g, me, a->slot, p->field[a->slot].damage);
        RtDiscardEnergy(g, me, a->slot);
        break;
    case TCG_CARD_ECARD1_146: /* Pokemon Reversal: heads, their chosen Benched Pokemon becomes the Defending one */
        if (a->target < 1 || a->target >= TCG_FIELD_SIZE || !op->field[a->target].stackSize)
            return Fail(g, "Choose one of their Benched Pokemon.");
        if (TcgDuel_Coin(g, me) && !TcgDuel_Anchored(g, me ^ 1))
        {
            struct TcgFieldCard temp = op->field[0];
            op->field[0] = op->field[a->target];
            op->field[a->target] = temp;
            Cure(&op->field[a->target]);
            TcgDuel_Event(g, TCG_EV_SWITCH, me ^ 1, a->target, 0, 0, 0);
        }
        break;
    case TCG_CARD_ECARD1_148: /* Professor Elm's Training Method: an Evolution card from the deck */
        if (a->selectionCount > 1 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, RtEvolution))
            return TCG_INVALID;
        if (a->selectionCount)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_ECARD1_149: /* Professor Oak's Research: hand into the deck, draw 5 */
        RtHandIntoDeck(g, me);
        RtDraw(g, me, 5);
        break;
    case TCG_CARD_ECARD1_150: /* Strength Charm (a Tool: its 10 more in the damage) */
    case TCG_CARD_NEO1_86:    /* Focus Band */
    case TCG_CARD_NEO1_93:    /* Gold Berry */
    case TCG_CARD_NEO1_94:    /* Miracle Berry */
    case TCG_CARD_NEO1_99:    /* Berry */
    case TCG_CARD_NEO3_60:    /* Balloon Berry */
    case TCG_CARD_NEO4_97:    /* Counterattack Claws */
    case TCG_CARD_ECARD2_125: /* Healing Berry */
    case TCG_CARD_ECARD3_122: /* Crystal Shard */
    case TCG_CARD_ECARD2_128: /* Memory Berry (its attacks: tcg_duel.c Borrowed) */
        break;
    case TCG_CARD_GYM1_116: /* Recall: the Active's lower stages' attacks this turn */
        p->turnBuffs |= TCG_BUFF_RECALL;
        break;
    case TCG_CARD_ECARD2_120: /* Energy Switch: a basic Energy from one of yours to another */
    {
        struct TcgFieldCard *to;
        if (a->slot >= TCG_FIELD_SIZE || a->target >= TCG_FIELD_SIZE || a->slot == a->target ||
            !p->field[a->target].stackSize)
            return Fail(g, "Choose two of your Pokemon.");
        f = &p->field[a->slot];
        to = &p->field[a->target];
        if (!RtPicked(p, f->attached, f->attachedSize, a, 0, 1, RtBasicEnergy))
            return Fail(g, "Choose a basic Energy card to move.");
        Move(f->attached, &f->attachedSize, to->attached, &to->attachedSize, a->selections[0]);
        TcgDuel_Event(g, TCG_EV_ATTACH, me, a->target, 0, p->cards[a->selections[0]], 0);
        break;
    }
    case TCG_CARD_ECARD2_126: /* Juggler: 1 basic Energy discarded, draw 3; 2, draw 5 */
        if (a->selectionCount > 2 || !RtPicked(p, p->hand, p->handSize, a, 0, a->selectionCount, RtBasicEnergy))
            return TCG_INVALID;
        for (i = 0; i < a->selectionCount; i++)
            Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[i]);
        RtDraw(g, me, a->selectionCount == 2 ? 5 : a->selectionCount == 1 ? 3 : 0);
        break;
    case TCG_CARD_ECARD2_130: /* Pokemon Fan Club: up to 2 Basics from the deck onto the Bench */
        if (a->selectionCount > 2 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, RtBasic))
            return TCG_INVALID;
        for (i = 0; i < a->selectionCount; i++)
            if (!RtBench(g, me, p->deck, &p->deckSize, a->selections[i]))
                return Fail(g, "There's no room on your Bench.");
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_ECARD2_133: /* Seer: the top 6's basic Energy into your hand */
    {
        uint8_t look[6];
        n = p->deckSize < 6 ? p->deckSize : 6;
        for (i = 0; i < n; i++)
            look[i] = p->deck[p->deckSize - 1 - i];
        for (i = 0; i < n; i++)
            if (RtBasicEnergy(Tcg_Card(p->cards[look[i]])))
                Move(p->deck, &p->deckSize, p->hand, &p->handSize, look[i]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_ECARD2_134: /* Super Energy Removal 2: both heads, the Defending's Energy; both tails, your Active's */
        n = RtFlips(g, 2);
        if (n == 2)
            RtDiscardEnergy(g, me ^ 1, 0);
        else if (n == 0)
            RtDiscardEnergy(g, me, 0);
        break;
    case TCG_CARD_ECARD2_136: /* Town Volunteers: 5 Pokemon / basic Energy from the discard pile into the deck */
    {
        unsigned want = RtCount(p, p->discard, p->discardSize, RtGarbage);
        want = want < 5 ? want : 5;
        if (a->selectionCount != want || !RtPicked(p, p->discard, p->discardSize, a, 0, want, RtGarbage))
            return Fail(g, "Choose 5 Pokemon and basic Energy cards (all of them if fewer).");
        for (i = 0; i < want; i++)
            Move(p->discard, &p->discardSize, p->deck, &p->deckSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_ECARD2_137: /* Traveling Salesman: up to 2 TMs / Tools from the deck */
    case TCG_CARD_ECARD3_120: /* Relic Hunter: up to 2 Supporters / Stadiums */
    case TCG_CARD_ECARD3_121: /* Apricorn Maker: up to 2 Trainers with Ball in their names */
    {
        int (*ok)(const struct TcgCard *) = id == TCG_CARD_ECARD2_137 ? RtTmTool
                                          : id == TCG_CARD_ECARD3_120 ? RtSupporterStadium
                                                                      : RtBall;
        if (a->selectionCount > 2 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, ok))
            return TCG_INVALID;
        for (i = 0; i < a->selectionCount; i++)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_ECARD2_141: /* Weakness Guard: no Weakness to the end of their next turn */
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].stackSize)
            return Fail(g, "Choose one of your Pokemon.");
        f = &p->field[a->slot];
        f->attached[f->attachedSize++] = (uint8_t)played;
        p->attachmentExpires[played] = (uint16_t)(g->turn + 1);
        return RT_PLACED;
    case TCG_CARD_ECARD3_123: /* Desert Shaman: hand into the deck, draw 4; they do the same */
        RtHandIntoDeck(g, me);
        RtDraw(g, me, 4);
        RtHandIntoDeck(g, me ^ 1);
        RtDraw(g, me ^ 1, RtTheirDraw(op, 4));
        break;
    case TCG_CARD_ECARD3_124: /* Fast Ball: revealed until an Evolution card, which goes to your hand */
        for (i = p->deckSize; i > 0; i--)
            if (RtEvolution(Tcg_Card(p->cards[p->deck[i - 1]])))
            {
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, i - 1);
                break;
            }
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_ECARD3_125: /* Fisherman: 4 basic Energy from the discard pile (all if fewer) */
    {
        unsigned want = RtCount(p, p->discard, p->discardSize, RtBasicEnergy);
        want = want < 4 ? want : 4;
        if (!want)
            return Fail(g, "There's no basic Energy in your discard pile.");
        if (a->selectionCount != want || !RtPicked(p, p->discard, p->discardSize, a, 0, want, RtBasicEnergy))
            return Fail(g, "Choose 4 basic Energy cards (all of them if fewer).");
        for (i = 0; i < want; i++)
            Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[i]);
        break;
    }
    case TCG_CARD_ECARD3_127: /* Hyper Potion: 1 or 2 basic Energy discarded; 3 / 5 counters off */
        if (a->slot >= TCG_FIELD_SIZE || !p->field[a->slot].damage)
            return Fail(g, "Choose one of your damaged Pokemon.");
        f = &p->field[a->slot];
        if (a->selectionCount < 1 || a->selectionCount > 2 ||
            !RtPicked(p, f->attached, f->attachedSize, a, 0, a->selectionCount, RtBasicEnergy))
            return Fail(g, "Choose 1 or 2 basic Energy cards on it.");
        for (i = 0; i < a->selectionCount; i++)
            Move(f->attached, &f->attachedSize, p->discard, &p->discardSize, a->selections[i]);
        RtHeal(g, me, a->slot, a->selectionCount == 2 ? 50 : 30);
        break;
    case TCG_CARD_ECARD3_138: /* Oracle: 2 cards of the deck on top (the first chosen on top) */
    {
        unsigned want = p->deckSize < 2 ? p->deckSize : 2;
        if (a->selectionCount != want || !RtPicked(p, p->deck, p->deckSize, a, 0, want, RtAny))
            return Fail(g, "Choose 2 cards from your deck.");
        for (i = 0; i < want; i++)
            TcgPile_Remove(p->deck, &p->deckSize, (unsigned)Index(p->deck, p->deckSize, a->selections[i]));
        TcgDuel_Shuffle(g, me);
        for (i = want; i > 0; i--)
            p->deck[p->deckSize++] = a->selections[i - 1];
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        break;
    }
    case TCG_CARD_NP_26: /* Tropical Wind: heads, 2 counters off each Active; tails, each Active Asleep */
        if (TcgDuel_Coin(g, me))
        {
            RtHeal(g, me, 0, 20);
            RtHeal(g, me ^ 1, 0, 20);
        }
        else
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                if (g->players[who].field[0].stackSize)
                {
                    g->players[who].field[0].condition = TCG_ASLEEP;
                    TcgDuel_Event(g, TCG_EV_STATUS, who, 0, TCG_ASLEEP, 0, g->players[who].field[0].poison);
                }
        }
        break;
    case TCG_CARD_NP_27: /* Tropical Tidal Wave: heads, their Trainers in play; tails, yours (not Supporters) */
    case TCG_CARD_NP_36:
    {
        unsigned who = TcgDuel_Coin(g, me) ? me ^ 1 : me;
        struct TcgPlayer *q = &g->players[who];
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            f = &q->field[s];
            for (i = f->attachedSize; i > 0; i--)
                if (RtTrainer(Tcg_Card(q->cards[f->attached[i - 1]])))
                {
                    q->attachmentExpires[f->attached[i - 1]] = 0;
                    q->discard[q->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
                }
        }
        if (q->stadium)
        {
            q->discard[q->discardSize++] = (uint8_t)(q->stadium - 1);
            q->stadium = 0;
        }
        if (who != me && q->supporterCard)
        {
            q->discard[q->discardSize++] = (uint8_t)(q->supporterCard - 1);
            q->supporterCard = 0;
        }
        break;
    }
    case TCG_CARD_NEO1_87: /* Mary: draw 2, then 2 of the hand into the deck (picked from the hand and the 2 drawn) */
    {
        uint8_t pool[TCG_DECK_SIZE + 2];
        unsigned count = 0, want;
        for (i = 0; i < p->handSize; i++)
            pool[count++] = p->hand[i];
        for (i = 0; i < 2 && i < p->deckSize; i++)
            pool[count++] = p->deck[p->deckSize - 1 - i];
        want = count < 2 ? count : 2;
        if (a->selectionCount != want || !RtPicked(p, pool, count, a, 0, want, RtAny))
            return Fail(g, "Choose 2 cards to shuffle into your deck.");
        RtDraw(g, me, 2);
        for (i = 0; i < want; i++)
            Move(p->hand, &p->handSize, p->deck, &p->deckSize, a->selections[i]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_NEO1_95: /* New Pokedex: shuffle; the top 5 in the order chosen */
        n = p->deckSize < 5 ? p->deckSize : 5;
        if (a->selectionCount != n || !RtPicked(p, p->deck + p->deckSize - n, n, a, 0, n, RtAny))
            return Fail(g, "Choose the order of the top cards of your deck.");
        TcgDuel_Shuffle(g, me);
        for (i = 0; i < n; i++)
            TcgPile_Remove(p->deck, &p->deckSize, (unsigned)Index(p->deck, p->deckSize, a->selections[i]));
        for (i = n; i > 0; i--)
            p->deck[p->deckSize++] = a->selections[i - 1];
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        break;
    case TCG_CARD_ECARD2_123: /* Forest Guardian: 1 of the top 7 into your hand, the rest shuffled in */
    {
        unsigned top = p->deckSize < 7 ? p->deckSize : 7, want = top ? 1 : 0;
        if (a->selectionCount != want || !RtPicked(p, p->deck + p->deckSize - top, top, a, 0, want, RtAny))
            return Fail(g, "Choose 1 of the top 7 cards.");
        if (want)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_ECARD3_140: /* Underground Expedition: 2 of the bottom 4 into your hand */
    {
        unsigned bottom = p->deckSize < 4 ? p->deckSize : 4, want = bottom < 2 ? bottom : 2;
        if (a->selectionCount != want || !RtPicked(p, p->deck, bottom, a, 0, want, RtAny))
            return Fail(g, "Choose 2 of the bottom 4 cards.");
        for (i = 0; i < want; i++)
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        break;
    }
    case TCG_CARD_ECARD3_126: /* Friend Ball: a Pokemon card of the type of one of theirs */
    {
        const struct TcgCard *theirs = a->target < TCG_FIELD_SIZE ? TcgDuel_Field(g, me ^ 1, a->target) : NULL;
        if (!theirs || a->selectionCount > 1 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, RtPokemon))
            return Fail(g, "Choose a Pokemon card in your deck and one of their Pokemon of its type.");
        if (a->selectionCount && Tcg_Card(p->cards[a->selections[0]])->type != theirs->type)
            return Fail(g, "That card isn't the same type as their Pokemon.");
        if (a->selectionCount)
        {
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
            TcgDuel_Log(g, "Shown: %s.", Tcg_Card(p->cards[a->selections[0]])->name);
        }
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_NEO3_62: /* Pokemon Breeder Fields: a coin for each of 1 or 2 later-Stage cards (each for one of
                            * yours in play, not a Baby); heads, into your hand */
    {
        unsigned used = 0, k;
        if (a->selectionCount > 2 || !RtPicked(p, p->deck, p->deckSize, a, 0, a->selectionCount, RtEvolution))
            return TCG_INVALID;
        for (i = 0; i < a->selectionCount; i++)
        {
            const struct TcgCard *e = Tcg_Card(p->cards[a->selections[i]]);
            for (k = 0; k < TCG_FIELD_SIZE; k++)
            {
                const struct TcgCard *t = TcgDuel_Field(g, me, k);
                if (!(used & (1u << k)) && t && t->kind == TCG_POKEMON && !(t->flags & TCG_CF_BABY) &&
                    TcgDuel_LaterStage(e, t))
                    break;
            }
            if (k == TCG_FIELD_SIZE)
                return Fail(g, "Each card must be a later Stage of a different Pokemon of yours.");
            used |= 1u << k;
        }
        for (i = 0; i < a->selectionCount; i++)
            if (TcgDuel_Coin(g, me))
            {
                Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[i]);
                TcgDuel_Log(g, "Shown: %s.", Tcg_Card(p->cards[a->selections[i]])->name);
            }
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_GYM2_125: /* Transparent Walls: no damage from attacks to your Bench until their next turn ends */
        p->benchShieldUntil = (uint16_t)(g->turn + 1);
        TcgDuel_Log(g, "Your Benched Pokemon are protected from damage until the end of their next turn.");
        break;
    case TCG_CARD_GYM2_126: /* Warp Point: yours switched with the one chosen, then theirs by their own choice */
    {
        unsigned k, bench = 0;
        for (k = 1; k < TCG_FIELD_SIZE; k++)
            bench += op->field[k].stackSize != 0;
        if (a->slot && a->slot < TCG_FIELD_SIZE && p->field[a->slot].stackSize && !TcgDuel_Anchored(g, me))
        {
            struct TcgFieldCard temp = p->field[0];
            p->field[0] = p->field[a->slot];
            p->field[a->slot] = temp;
            Cure(&p->field[a->slot]);
            TcgDuel_Event(g, TCG_EV_SWITCH, me, a->slot, 0, 0, 0);
        }
        if (bench && !TcgDuel_Anchored(g, me ^ 1))
        {
            g->phase = TCG_PROMOTION;
            g->promotePlayer = (uint8_t)(me ^ 1);
            g->switchPending = 1;
        }
        break;
    }
    case TCG_CARD_NEO1_102: /* Pokemon March: a Basic onto each Bench from the decks, theirs first */
    {
        unsigned k;
        for (k = op->deckSize; k > 0; k--)
            if (RtBasicNotBaby(Tcg_Card(op->cards[op->deck[k - 1]])))
            {
                RtBench(g, me ^ 1, op->deck, &op->deckSize, op->deck[k - 1]);
                break;
            }
        TcgDuel_Shuffle(g, me ^ 1);
        if (a->selectionCount && (!RtBasicNotBaby(Tcg_Card(p->cards[a->selections[0]])) ||
                                  !RtBench(g, me, p->deck, &p->deckSize, a->selections[0])))
            return Fail(g, "Choose a Basic Pokemon from your deck.");
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_GYM2_18: /* Giovanni: a Giovanni Pokemon may evolve whenever this turn */
        if (a->slot >= TCG_FIELD_SIZE || !RtNamed(TcgDuel_Field(g, me, a->slot), "Giovanni"))
            return Fail(g, "Choose a Pokemon with Giovanni in its name.");
        p->field[a->slot].freeEvolve = (uint16_t)(g->turn + 1);
        break;
    /* Stadiums whose rules are elsewhere (TcgDuel_RetreatCost, the damage, evolving...) */
    case TCG_CARD_GYM1_104: /* The Rocket's Training Gym */
    case TCG_CARD_GYM1_108: /* Cerulean City Gym */
    case TCG_CARD_GYM1_115: /* Pewter City Gym */
    case TCG_CARD_GYM2_109: /* Resistance Gym */
    case TCG_CARD_GYM2_113: /* Cinnabar City Gym */
    case TCG_CARD_GYM2_123: /* Viridian City Gym */
    case TCG_CARD_NEO1_97:  /* Sprout Tower */
    case TCG_CARD_NEO4_92:  /* Broken Ground Gym */
    case TCG_CARD_NEO3_63:  /* Rocket's Hideout (TcgDuel_MaxHp) */
    case TCG_CARD_GYM2_119: /* Rocket's Minefield Gym (TcgPower_BenchedFromHand) */
    case TCG_CARD_ECARD2_131: /* Pokemon Park */
    case TCG_CARD_ECARD3_132: /* Mirage Stadium */
    /* (and the ones used during a turn: RegionStadiumUse) */
    case TCG_CARD_GYM1_107: case TCG_CARD_GYM2_114: case TCG_CARD_GYM2_122: case TCG_CARD_NEO3_61:
    case TCG_CARD_NEO4_95: case TCG_CARD_NEO4_99: case TCG_CARD_NEO4_100: case TCG_CARD_ECARD2_118:
    case TCG_CARD_ECARD2_138: case TCG_CARD_ECARD2_139: case TCG_CARD_ECARD3_119: case TCG_CARD_ECARD3_137:
    case TCG_CARD_ECARD3_141:
        break;
    default:
        if (Tcg_Card(id)->flags & TCG_CF_TM)
            break; /* (a Technical Machine: attached by TcgDuel_Trainer, its attack used as TCG_ATTACK_TM) */
        snprintf(g->error, sizeof(g->error), "Trainer effect not yet ported: %s", Tcg_Card(id)->name);
        return TCG_UNSUPPORTED;
    }
    return TCG_OK;
}

/* ------------------------------------------------------------------------------------------------ Stadiums used
 * Stadiums a player uses during their turn (TCG_USE_STADIUM, asking as a Trainer does): once a turn, but Celadon City Gym and
 * Saffron City Gym as often as they like. The Stadium in play is either player's. */
static int RtStadiumUnlimited(unsigned stadium)
{
    return stadium == TCG_CARD_GYM1_107 || stadium == TCG_CARD_GYM2_122;
}
static int RtStadiumUsable(unsigned stadium)
{
    switch (stadium)
    {
    case TCG_CARD_GYM1_107: case TCG_CARD_GYM2_114: case TCG_CARD_GYM2_122: case TCG_CARD_NEO3_61:
    case TCG_CARD_NEO4_95: case TCG_CARD_NEO4_99: case TCG_CARD_NEO4_100: case TCG_CARD_ECARD2_118:
    case TCG_CARD_ECARD2_138: case TCG_CARD_ECARD2_139: case TCG_CARD_ECARD3_119: case TCG_CARD_ECARD3_137:
    case TCG_CARD_ECARD3_141:
        return 1;
    }
    return 0;
}
static int RtOmanyteKabuto(const struct TcgCard *c)
{
    return RtPokemon(c) && (!strcmp(c->name, "Omanyte") || !strcmp(c->name, "Kabuto"));
}
static const char *RegionStadiumBlock(const struct TcgDuel *g, unsigned who)
{
    const struct TcgPlayer *p = &g->players[who];
    unsigned stadium = TcgDuel_Stadium(g, NULL), s, n = 0;
    if (!stadium)
        return "There is no Stadium in play.";
    if (!RtStadiumUsable(stadium))
        return "This Stadium works on its own.";
    if (g->phase != TCG_TURN || g->current != who)
        return "A Stadium is used during your turn.";
    if (p->stadiumUsed && !RtStadiumUnlimited(stadium))
        return "You already used the Stadium this turn.";
    switch (stadium)
    {
    case TCG_CARD_GYM1_107: /* Celadon City Gym */
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            n += RtErika(TcgDuel_Field(g, who, s)) && RtCount(p, p->field[s].attached, p->field[s].attachedSize, RtEnergy);
        return n ? NULL : "No Erika Pokemon of yours has Energy attached.";
    case TCG_CARD_GYM2_114: /* Fuchsia City Gym */
    {
        unsigned others = 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            n += RtNamed(TcgDuel_Field(g, who, s), "Koga");
            others += p->field[s].stackSize != 0;
        }
        return !n ? "You have no Pokemon with Koga in its name." : others < 2 ? "It's your only Pokemon in play." : NULL;
    }
    case TCG_CARD_GYM2_122: /* Saffron City Gym */
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (RtNamed(TcgDuel_Field(g, who, s), "Sabrina"))
            {
                unsigned i;
                for (i = 0; i < p->field[s].attachedSize; i++)
                    n += RtBasicEnergy(Tcg_Card(p->cards[p->field[s].attached[i]]));
            }
        return n ? NULL : "No Sabrina Pokemon of yours has basic Energy attached.";
    case TCG_CARD_NEO3_61: /* Healing Field */
        return p->field[0].damage ? NULL : "Your Active Pokemon isn't damaged.";
    case TCG_CARD_NEO4_95: /* Radio Tower */
    case TCG_CARD_NEO4_100: /* Lucky Stadium */
        return p->deckSize ? NULL : "Your deck is empty.";
    case TCG_CARD_NEO4_99: /* Energy Stadium */
        return RtCount(p, p->discard, p->discardSize, RtBasicEnergy) ? NULL : "No basic Energy in your discard pile.";
    case TCG_CARD_ECARD2_118: /* Apricorn Forest */
        if (RtEmptyBench(p) < 0)
            return "Your Bench is full.";
        return RtCount(p, p->hand, p->handSize, RtBasicEnergy) ? NULL : "No basic Energy card in your hand.";
    case TCG_CARD_ECARD2_138: /* Undersea Ruins */
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            n += p->field[s].stackSize >= 2;
        return n ? NULL : "You have no Evolved Pokemon.";
    case TCG_CARD_ECARD2_139: /* Power Plant */
        if (!RtCount(p, p->hand, p->handSize, RtBasicEnergy))
            return "No basic Energy card in your hand.";
        return RtCount(p, p->discard, p->discardSize, RtBasicEnergy) ? NULL : "No basic Energy in your discard pile.";
    case TCG_CARD_ECARD3_119: /* Ancient Ruins */
        return p->supporterPlayed ? "You played a Supporter this turn." : !p->deckSize ? "Your deck is empty." : NULL;
    case TCG_CARD_ECARD3_137: /* Mystery Zone */
        return RtCount(p, p->hand, p->handSize, RtEvolution) ? NULL : "No Evolution card in your hand.";
    case TCG_CARD_ECARD3_141: /* Underground Lake */
        if (RtEmptyBench(p) < 0)
            return "Your Bench is full.";
        return RtCount(p, p->discard, p->discardSize, RtOmanyteKabuto) ? NULL : "No Omanyte or Kabuto in your discard pile.";
    }
    return NULL;
}
static int RegionStadiumUse(struct TcgDuel *g, const struct TcgAction *a)
{
    unsigned me = g->current, stadium = TcgDuel_Stadium(g, NULL), i;
    struct TcgPlayer *p = &g->players[me];
    struct TcgFieldCard *f;
    const char *blocked = RegionStadiumBlock(g, me);
    if (blocked)
        return Fail(g, blocked);
    switch (stadium)
    {
    case TCG_CARD_GYM1_107: /* Celadon City Gym: an Energy off an Erika Pokemon, which is cured */
        if (a->slot >= TCG_FIELD_SIZE || !RtErika(TcgDuel_Field(g, me, a->slot)) || a->selectionCount != 1 ||
            Index(p->field[a->slot].attached, p->field[a->slot].attachedSize, a->selections[0]) < 0 ||
            !RtEnergy(Tcg_Card(p->cards[a->selections[0]])))
            return Fail(g, "Choose an Erika Pokemon and one of its Energy cards.");
        f = &p->field[a->slot];
        Move(f->attached, &f->attachedSize, p->discard, &p->discardSize, a->selections[0]);
        f->condition = TCG_HEALTHY;
        f->poison = 0;
        TcgDuel_Event(g, TCG_EV_STATUS, me, a->slot, f->condition, 0, 0);
        break;
    case TCG_CARD_GYM2_114: /* Fuchsia City Gym: heads, a Koga Pokemon and its cards into the deck */
    {
        unsigned slot = a->slot, k;
        if (slot >= TCG_FIELD_SIZE || !RtNamed(TcgDuel_Field(g, me, slot), "Koga"))
            return Fail(g, "Choose a Pokemon with Koga in its name.");
        p->stadiumUsed = 1;
        if (!TcgDuel_Coin(g, me))
            break;
        f = &p->field[slot];
        for (k = 0; k < f->stackSize; k++)
            p->deck[p->deckSize++] = f->stack[k];
        for (k = 0; k < f->attachedSize; k++)
        {
            p->attachmentExpires[f->attached[k]] = 0;
            p->deck[p->deckSize++] = f->attached[k];
        }
        memset(f, 0, sizeof(*f));
        TcgDuel_Shuffle(g, me);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        if (!slot)
        {
            /* (its owner brings out another: the empty Active spot switched with a Benched Pokemon) */
            g->phase = TCG_PROMOTION;
            g->promotePlayer = (uint8_t)me;
            g->switchPending = 1;
        }
        break;
    }
    case TCG_CARD_GYM2_122: /* Saffron City Gym: a basic Energy off a Sabrina Pokemon into the hand */
        if (a->slot >= TCG_FIELD_SIZE || !RtNamed(TcgDuel_Field(g, me, a->slot), "Sabrina") || a->selectionCount != 1 ||
            Index(p->field[a->slot].attached, p->field[a->slot].attachedSize, a->selections[0]) < 0 ||
            !RtBasicEnergy(Tcg_Card(p->cards[a->selections[0]])))
            return Fail(g, "Choose a Sabrina Pokemon and one of its basic Energy cards.");
        f = &p->field[a->slot];
        Move(f->attached, &f->attachedSize, p->hand, &p->handSize, a->selections[0]);
        break;
    case TCG_CARD_NEO3_61: /* Healing Field: heads, 2 counters off the Active */
        p->stadiumUsed = 1;
        if (TcgDuel_Coin(g, me))
            RtHeal(g, me, 0, 20);
        break;
    case TCG_CARD_NEO4_95: /* Radio Tower: the top 2 looked at, left as they are */
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        break;
    case TCG_CARD_NEO4_99: /* Energy Stadium: heads, a basic Energy from the discard pile into the hand */
        if (a->selectionCount != 1 || !RtPicked(p, p->discard, p->discardSize, a, 0, 1, RtBasicEnergy))
            return Fail(g, "Choose a basic Energy card in your discard pile.");
        p->stadiumUsed = 1;
        if (TcgDuel_Coin(g, me))
            Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[0]);
        break;
    case TCG_CARD_NEO4_100: /* Lucky Stadium: heads, draw a card */
        p->stadiumUsed = 1;
        if (TcgDuel_Coin(g, me))
            RtDraw(g, me, 1);
        break;
    case TCG_CARD_ECARD2_118: /* Apricorn Forest: heads, a Basic of the shown basic Energy's type onto the Bench */
    {
        const struct TcgCard *energy;
        if (!RtPicked(p, p->hand, p->handSize, a, 0, 1, RtBasicEnergy) || a->selectionCount > 2 ||
            (a->selectionCount == 2 && !RtPicked(p, p->deck, p->deckSize, a, 1, 1, RtBasic)))
            return Fail(g, "Choose a basic Energy card in your hand and a Basic Pokemon in your deck.");
        energy = Tcg_Card(p->cards[a->selections[0]]);
        if (a->selectionCount == 2 && Tcg_Card(p->cards[a->selections[1]])->type != energy->type)
            return Fail(g, "That Pokemon isn't the same type as the Energy.");
        p->stadiumUsed = 1;
        if (!TcgDuel_Coin(g, me))
            break;
        TcgDuel_Log(g, "Shown: %s.", energy->name);
        if (a->selectionCount == 2)
            RtBench(g, me, p->deck, &p->deckSize, a->selections[1]);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_CARD_ECARD2_138: /* Undersea Ruins: heads, an Evolved Pokemon's top card discarded */
        if (a->slot >= TCG_FIELD_SIZE || p->field[a->slot].stackSize < 2)
            return Fail(g, "Choose one of your Evolved Pokemon.");
        p->stadiumUsed = 1;
        if (!TcgDuel_Coin(g, me))
            break;
        f = &p->field[a->slot];
        p->discard[p->discardSize++] = f->stack[--f->stackSize];
        Cure(f);
        TcgDuel_Event(g, TCG_EV_DEVOLVE, me, a->slot, 0, 0, 0);
        TcgDuel_ResolveKnockouts(g);
        break;
    case TCG_CARD_ECARD2_139: /* Power Plant: a basic Energy from the hand discarded, one from the discard pile taken */
        if (a->selectionCount != 2 || !RtPicked(p, p->hand, p->handSize, a, 0, 1, RtBasicEnergy) ||
            !RtPicked(p, p->discard, p->discardSize, a, 1, 1, RtBasicEnergy))
            return Fail(g, "Choose a basic Energy card in your hand and one in your discard pile.");
        p->stadiumUsed = 1;
        Move(p->discard, &p->discardSize, p->hand, &p->handSize, a->selections[1]);
        Move(p->hand, &p->handSize, p->discard, &p->discardSize, a->selections[0]);
        TcgDuel_Log(g, "Shown: %s.", Tcg_Card(p->cards[a->selections[1]])->name);
        break;
    case TCG_CARD_ECARD3_119: /* Ancient Ruins: the hand shown; no Supporter there, draw a card */
    {
        unsigned supporters = 0;
        p->stadiumUsed = 1;
        for (i = 0; i < p->handSize; i++)
            supporters += (Tcg_Card(p->cards[p->hand[i]])->flags & TCG_CF_SUPPORTER) != 0;
        TcgDuel_Log(g, "Player %u shows their hand: %s.", me + 1, supporters ? "a Supporter is there" : "no Supporter");
        if (!supporters)
            RtDraw(g, me, 1);
        break;
    }
    case TCG_CARD_ECARD3_137: /* Mystery Zone: a basic Energy from the deck; an Evolution card from the hand into it */
        if (a->selectionCount < 1 || a->selectionCount > 2 ||
            !RtPicked(p, p->hand, p->handSize, a, a->selectionCount - 1, 1, RtEvolution) ||
            (a->selectionCount == 2 && !RtPicked(p, p->deck, p->deckSize, a, 0, 1, RtBasicEnergy)))
            return Fail(g, "Choose an Evolution card in your hand (and a basic Energy card in your deck).");
        p->stadiumUsed = 1;
        if (a->selectionCount == 2)
        {
            Move(p->deck, &p->deckSize, p->hand, &p->handSize, a->selections[0]);
            TcgDuel_Log(g, "Shown: %s.", Tcg_Card(p->cards[a->selections[0]])->name);
        }
        Move(p->hand, &p->handSize, p->deck, &p->deckSize, a->selections[a->selectionCount - 1]);
        TcgDuel_Log(g, "Shown: %s.", Tcg_Card(p->cards[a->selections[a->selectionCount - 1]])->name);
        TcgDuel_Shuffle(g, me);
        break;
    case TCG_CARD_ECARD3_141: /* Underground Lake: an Omanyte or Kabuto from the discard pile onto the Bench */
        if (a->selectionCount != 1 || !RtPicked(p, p->discard, p->discardSize, a, 0, 1, RtOmanyteKabuto))
            return Fail(g, "Choose an Omanyte or Kabuto in your discard pile.");
        p->stadiumUsed = 1;
        RtBench(g, me, p->discard, &p->discardSize, a->selections[0]);
        break;
    }
    p->stadiumUsed = 1;
    TcgDuel_Log(g, "Player %u uses %s.", me + 1, Tcg_Card(stadium)->name);
    return TCG_OK;
}
