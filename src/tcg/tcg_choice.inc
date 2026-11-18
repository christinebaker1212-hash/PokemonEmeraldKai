/* A player's choice in the middle of a card's effect (phase TCG_CHOICE; docs/TCG_REGIONS.md): what each
 * TCG_CHOICE_* takes as its answer (TCG_CHOOSE), and the AI's answer. Included by tcg_effects.c for its helpers; the
 * steps that ask it are tcg_rtrainers_flow.inc TcgFlow_ChoiceSteps.
 *
 * Stage 0 is the opponent's ("Your opponent may..."), stage 1 the player's own ("Either way, you may do the same"):
 * the player whose card it is answers last. */

static int ChoiceFail(struct TcgDuel *g, const char *message)
{
    snprintf(g->error, sizeof(g->error), "%s", message);
    return TCG_INVALID;
}
/* Is that card one the choice may take from the pile */
static int ChoiceFits(unsigned kind, const struct TcgCard *c)
{
    if (!c)
        return 0;
    switch (kind)
    {
    case TCG_CHOICE_DISCARD_POKEMON:
    case TCG_CHOICE_DISCARD_TO_DECK:
        return c->kind == TCG_POKEMON;
    case TCG_CHOICE_TIME_CAPSULE:
        return c->kind == TCG_POKEMON || (c->kind == TCG_ENERGY && c->id <= 6);
    }
    return 1;
}
/* An Energy card of type t attached to that slot (basic: basic only), or -1 */
static int ChoiceEnergyOn(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned type, int basic)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned k;
    for (k = 0; f->stackSize && k < f->attachedSize; k++)
    {
        const struct TcgCard *c = CardAt(g, who, f->attached[k]);
        if (c->kind == TCG_ENERGY && (!basic || c->id <= 6) && (type == TCG_TYPES || c->type == type))
            return (int)k;
    }
    return -1;
}
static unsigned ChoiceInPlay(const struct TcgDuel *g, unsigned who)
{
    unsigned s, n = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        n += g->players[who].field[s].stackSize != 0;
    return n;
}
static int ChoiceDamaged(const struct TcgDuel *g, unsigned who)
{
    unsigned s;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (g->players[who].field[s].stackSize && g->players[who].field[s].damage)
            return 1;
    return 0;
}
static int ChoiceEnergyHolders(const struct TcgDuel *g, unsigned who)
{
    unsigned s, n = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        n += ChoiceEnergyOn(g, who, s, TCG_TYPES, 0) >= 0;
    return n;
}
static int ChoiceDiscardBasic(const struct TcgDuel *g, unsigned who)
{
    const struct TcgPlayer *q = &g->players[who];
    unsigned k;
    if (BenchCount(g, who) >= TCG_FIELD_SIZE - 1)
        return 0;
    for (k = 0; k < q->discardSize; k++)
        if (CardAt(g, who, q->discard[k])->kind == TCG_POKEMON && !CardAt(g, who, q->discard[k])->stage)
            return 1;
    return 0;
}
static int ChoiceSporeSlot(const struct TcgDuel *g, unsigned who, unsigned s)
{
    const struct TcgCard *c = TcgDuel_Field(g, who, s);
    return c && (!strcmp(c->name, "Hoppip") || !strcmp(c->name, "Skiploom")) && !(g->choiceData & (1u << s));
}
static unsigned ChoiceBasicEnergyInDiscard(const struct TcgDuel *g, unsigned who)
{
    const struct TcgPlayer *q = &g->players[who];
    unsigned k, n = 0;
    for (k = 0; k < q->discardSize; k++)
        n += CardAt(g, who, q->discard[k])->kind == TCG_ENERGY && CardAt(g, who, q->discard[k])->id <= 6;
    return n;
}
/* Streaming Mantle for a player: the top 3 of the deck discarded, then their pick of 3 basic Energy into the deck */
static void ChoiceMantle(struct TcgDuel *g, unsigned who, unsigned card, unsigned theirs)
{
    struct TcgPlayer *q = &g->players[who];
    unsigned k;
    for (k = 0; k < 3 && q->deckSize; k++)
        q->discard[q->discardSize++] = q->deck[--q->deckSize];
    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, k, 0, 0);
    g->choiceData = (uint8_t)theirs;
    g->phase = TCG_TURN;
    TcgDuel_AskChoice(g, who, TCG_CHOICE_MANTLE_PICK, card, 1);
}
/* Has that player anything to choose (else the stage is passed over: no question with no answer) */
static int ChoiceHasAny(const struct TcgDuel *g, unsigned player, unsigned kind)
{
    const struct TcgPlayer *p = &g->players[player];
    unsigned k;
    switch (kind)
    {
    case TCG_CHOICE_DISCARD_POKEMON:
    case TCG_CHOICE_DISCARD_TO_DECK:
    case TCG_CHOICE_DISCARD_ANY:
    case TCG_CHOICE_TIME_CAPSULE:
        for (k = 0; k < p->discardSize; k++)
            if (ChoiceFits(kind, CardAt(g, player, p->discard[k])))
                return 1;
        return 0;
    case TCG_CHOICE_HAND_TO_TOP:
        return p->handSize != 0;
    case TCG_CHOICE_TOP3_ENERGY:
        for (k = 0; k < 3 && k < p->deckSize; k++)
            if (CardAt(g, player, p->deck[p->deckSize - 1 - k])->kind == TCG_ENERGY &&
                CardAt(g, player, p->deck[p->deckSize - 1 - k])->id <= 6)
                return 1;
        return 0;
    case TCG_CHOICE_EVOLVE:
    {
        unsigned s, d;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            for (d = 0; TcgDuel_Field(g, player, s) && p->field[s].stackSize < 4 && d < p->deckSize; d++)
                if (EvolvesFrom(CardAt(g, player, p->deck[d]), TcgDuel_Field(g, player, s)))
                    return 1;
        return 0;
    }
    case TCG_CHOICE_TOOL:
        for (k = 0; k < p->deckSize; k++)
            if (CardAt(g, player, p->deck[k])->kind == TCG_TRAINER && (CardAt(g, player, p->deck[k])->flags & TCG_CF_TOOL))
                return 1;
        return 0;
    case TCG_CHOICE_SWITCH_THEIRS:
        return BenchCount(g, player ^ 1) && !TcgDuel_Anchored(g, player ^ 1);
    case TCG_CHOICE_BENCH_HIT:
    case TCG_CHOICE_MOVE_ENERGY:
        return BenchCount(g, player) != 0;
    case TCG_CHOICE_BENCH_THEIRS:
    {
        const struct TcgPlayer *q = &g->players[player ^ 1];
        if (BenchCount(g, player ^ 1) >= TCG_FIELD_SIZE - 1)
            return 0;
        for (k = 0; k < q->handSize; k++)
            if (CardAt(g, player ^ 1, q->hand[k])->kind == TCG_POKEMON && !CardAt(g, player ^ 1, q->hand[k])->stage)
                return 1;
        return 0;
    }
    case TCG_CHOICE_SHUFFLE_THEIRS:
        return g->players[player ^ 1].deckSize != 0;
    case TCG_CHOICE_CHAOS_FROM_SIDE:
        return ChoiceDamaged(g, 0) || ChoiceDamaged(g, 1);
    case TCG_CHOICE_MANIPULATE:
        if (BenchCount(g, player) >= TCG_FIELD_SIZE - 1)
            return 0;
        for (k = 0; k < p->discardSize; k++)
            if (CardAt(g, player, p->discard[k])->kind == TCG_POKEMON && !CardAt(g, player, p->discard[k])->stage &&
                !(CardAt(g, player, p->discard[k])->flags & TCG_CF_BABY))
                return 1;
        return 0;
    case TCG_CHOICE_MANIPULATE_ENERGY:
        return (g->choiceData >> 3) && ChoiceBasicEnergyInDiscard(g, player);
    case TCG_CHOICE_MANTLE_PICK:
        return ChoiceBasicEnergyInDiscard(g, player) != 0;
    case TCG_CHOICE_HOWL_ATTACH:
    {
        unsigned s;
        for (s = 0; g->choiceData && s < TCG_FIELD_SIZE; s++)
            if (TcgDuel_Field(g, player, s) && TcgDuel_Type(g, player, s) == TCG_FIRE)
                return 1;
        return 0;
    }
    case TCG_CHOICE_ORDER_THEIRS:
        return g->choiceData && g->players[player ^ 1].deckSize;
    case TCG_CHOICE_STAR_PIECE:
        for (k = 0; g->choiceData < TCG_FIELD_SIZE && k < p->deckSize; k++)
            if (EvolvesFrom(CardAt(g, player, p->deck[k]), TcgDuel_Field(g, player, g->choiceData)))
                return 1;
        return 0;
    case TCG_CHOICE_FLOW_A:
        return ChoiceEnergyHolders(g, player ^ 1) >= 2;
    case TCG_CHOICE_INVIGORATE_SIDE:
        return ChoiceDiscardBasic(g, player) || ChoiceDiscardBasic(g, player ^ 1);
    case TCG_CHOICE_WISH_SWAP:
        return p->handSize != 0;
    case TCG_CHOICE_EEVEE_EVOLVE:
        for (k = 0; k < p->deckSize; k++)
        {
            const struct TcgCard *c = CardAt(g, player, p->deck[k]);
            if (EvolvesFrom(c, TcgDuel_Field(g, player, g->choiceData & 15)) && c->type == g->choiceData >> 4)
                return 1;
        }
        return 0;
    case TCG_CHOICE_SPORE:
    {
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            for (k = 0; ChoiceSporeSlot(g, player, s) && k < p->deckSize; k++)
                if (EvolvesFrom(CardAt(g, player, p->deck[k]), TcgDuel_Field(g, player, s)))
                    return 1;
        return 0;
    }
    case TCG_CHOICE_RETURN_MON:
        return ChoiceInPlay(g, player) != 0;
    case TCG_CHOICE_DISCARD2:
        return p->handSize != 0;
    case TCG_CHOICE_TEXTURE_WEAK:
    {
        const struct TcgCard *d = TcgDuel_Field(g, player ^ 1, 0);
        return d && d->kind == TCG_POKEMON && d->weakness != TCG_NONE;
    }
    case TCG_CHOICE_PURE_BODY:
        return g->choiceData < TCG_FIELD_SIZE && p->field[g->choiceData].attachedSize;
    case TCG_CHOICE_CHAIN_REACTION:
        for (k = 0; g->choiceData < TCG_FIELD_SIZE && k < p->deckSize; k++)
            if (EvolvesFrom(CardAt(g, player, p->deck[k]), TcgDuel_Field(g, player, g->choiceData)))
                return 1;
        return 0;
    case TCG_CHOICE_ARENA:
        return p->handSize > 7;
    case TCG_CHOICE_IMPOSTOR:
        return g->players[player ^ 1].prizeSize != 0;
    case TCG_CHOICE_MOVE_THEIRS_FROM:
        for (k = 0; k < TCG_FIELD_SIZE && ChoiceInPlay(g, player ^ 1) > 1; k++)
            if (ChoiceEnergyOn(g, player ^ 1, k, TCG_TYPES, 1) >= 0)
                return 1;
        return 0;
    case TCG_CHOICE_MOVE_THEIRS_TO:
        return ChoiceInPlay(g, player ^ 1) > 1;
    case TCG_CHOICE_WATER_TO_BENCH:
        return g->choiceData && BenchCount(g, player) && ChoiceEnergyOn(g, player, 0, TCG_WATER, 0) >= 0;
    case TCG_CHOICE_COUNTER_BENCH:
        return g->choiceData && BenchCount(g, player ^ 1);
    case TCG_CHOICE_ATTACH_LIGHTNING:
    {
        unsigned s;
        if (!g->choiceData)
            return 0;
        for (k = 0; k < p->deckSize && CardAt(g, player, p->deck[k])->id != TCG_CARD_LIGHTNING_ENERGY; k++)
            ;
        for (s = 0; k < p->deckSize && s < TCG_FIELD_SIZE; s++)
            if (TcgDuel_Field(g, player, s) && TcgDuel_Type(g, player, s) == TCG_LIGHTNING)
                return 1;
        return 0;
    }
    case TCG_CHOICE_HEAL_MINE:
    case TCG_CHOICE_HEAL_THEIRS:
    {
        const struct TcgPlayer *q = &g->players[kind == TCG_CHOICE_HEAL_MINE ? player : player ^ 1];
        for (k = 0; k < TCG_FIELD_SIZE; k++)
            if (q->field[k].stackSize && q->field[k].damage)
                return 1;
        return 0;
    }
    case TCG_CHOICE_PEEK_ZONE:
        return p->deckSize || p->prizeSize || g->players[player ^ 1].deckSize || g->players[player ^ 1].prizeSize;
    }
    return 1;
}
/* The cards a Noctowl-style look shows (zone TCG_PEEK_*: the top 2 of that deck, or 2 of those Prizes) */
static void ChoicePeek(struct TcgDuel *g, unsigned who, unsigned zone, unsigned most)
{
    unsigned k;
    if (zone == TCG_PEEK_MY_DECK || zone == TCG_PEEK_THEIR_DECK)
    {
        unsigned q = zone == TCG_PEEK_MY_DECK ? who : who ^ 1;
        const struct TcgPlayer *d = &g->players[q];
        for (k = 0; k < most && k < d->deckSize; k++)
            TcgDuel_Event(g, TCG_EV_PEEK, who, 0, zone, d->cards[d->deck[d->deckSize - 1 - k]], k + 1);
    }
    else if (zone == TCG_PEEK_MY_PRIZE || zone == TCG_PEEK_THEIR_PRIZE)
    {
        /* (face down in no order: any of them, side by side; not always the same ones) */
        unsigned q = zone == TCG_PEEK_MY_PRIZE ? who : who ^ 1, from;
        const struct TcgPlayer *d = &g->players[q];
        if (!d->prizeSize)
            return;
        from = most < d->prizeSize ? Tcg_RandomBelow(&g->rng, d->prizeSize) : 0;
        for (k = 0; k < most && k < d->prizeSize; k++)
            TcgDuel_Event(g, TCG_EV_PEEK, who, 0, zone, d->cards[d->prizes[(from + k) % d->prizeSize]], 0);
    }
}
/* What a passed-over stage still does: Flipper Stroke's shuffle ("Either way, each player shuffles") */
static void ChoiceSkipped(struct TcgDuel *g, unsigned player, unsigned kind)
{
    if (kind == TCG_CHOICE_TOP3_ENERGY || kind == TCG_CHOICE_ATTACH_LIGHTNING || kind == TCG_CHOICE_SPORE)
        TcgDuel_Shuffle(g, player);
    if (kind == TCG_CHOICE_MANTLE_PICK && !g->choiceData)
    {
        /* (none to put back: then the other player does the same; their own pass is the end) */
        struct TcgPlayer *q = &g->players[player ^ 1];
        unsigned k;
        for (k = 0; k < 3 && q->deckSize; k++)
            q->discard[q->discardSize++] = q->deck[--q->deckSize];
        g->choiceData = 1;
    }
}
/* Once more (choiceData left): the same question again, else done */
static int ChoiceAgain(struct TcgDuel *g, unsigned who, unsigned kind)
{
    g->phase = TCG_TURN;
    TcgDuel_AskChoice(g, who, kind, g->choiceCard, 1);
    return TCG_OK;
}
/* Is the choice the player's too after the opponent's ("Either way, you may do the same"), or theirs alone */
static int ChoiceBoth(unsigned kind)
{
    switch (kind)
    {
    case TCG_CHOICE_DISCARD_POKEMON: case TCG_CHOICE_DISCARD_TO_DECK: case TCG_CHOICE_DISCARD_ANY:
    case TCG_CHOICE_TIME_CAPSULE: case TCG_CHOICE_REDRAW4: case TCG_CHOICE_TOP3_ENERGY: case TCG_CHOICE_EVOLVE:
    case TCG_CHOICE_TOOL:
        return 1;
    }
    return 0;
}
/* The choice is done: what ends with it, and back to the turn */
static void ChoiceEnd(struct TcgDuel *g, unsigned kind)
{
    if (kind == TCG_CHOICE_TIME_CAPSULE)
        g->players[g->current].noTrainers = 1; /* ("you can't play any more Trainer cards this turn") */
    if (g->phase == TCG_CHOICE)
        g->phase = TCG_TURN;
    g->choiceKind = TCG_CHOICE_NONE;
}
void TcgDuel_AskChoice(struct TcgDuel *g, unsigned player, unsigned kind, unsigned card, unsigned stage)
{
    /* (stage 0 the opponent's, then stage 1 the player's own; a player with nothing to choose is passed over) */
    for (;;)
    {
        if (ChoiceHasAny(g, player & 1, kind))
        {
            g->phase = TCG_CHOICE;
            g->choicePlayer = (uint8_t)(player & 1);
            g->choiceKind = (uint8_t)kind;
            g->choiceCard = (uint16_t)card;
            g->choiceStage = (uint8_t)stage;
            return;
        }
        ChoiceSkipped(g, player & 1, kind);
        if (stage == 0 && (player & 1) != g->current && ChoiceBoth(kind))
            player = g->current, stage = 1;
        else
        {
            ChoiceEnd(g, kind);
            return;
        }
    }
}
/* After stage 0, the player whose card it is (stage 1); after that, back to the turn */
static void ChoiceNext(struct TcgDuel *g)
{
    unsigned kind = g->choiceKind;
    if (g->choiceStage == 0 && g->choicePlayer != g->current && ChoiceBoth(kind))
    {
        g->phase = TCG_TURN; /* (AskChoice sets it again if the player has something to choose) */
        TcgDuel_AskChoice(g, g->current, kind, g->choiceCard, 1);
    }
    else
        ChoiceEnd(g, kind);
}
int TcgDuel_Choose(struct TcgDuel *g, const struct TcgAction *a)
{
    unsigned who = g->choicePlayer, k, n = a->selectionCount;
    struct TcgPlayer *p = &g->players[who];
    switch (g->choiceKind)
    {
    case TCG_CHOICE_DISCARD_POKEMON:
    case TCG_CHOICE_DISCARD_ANY:
    case TCG_CHOICE_DISCARD_TO_DECK:
    case TCG_CHOICE_TIME_CAPSULE:
    {
        /* the cards picked in the chooser's discard pile: into the hand (up to 1), or into the deck (up to 3; Time
         * Capsule: 5, or all there are, or none) */
        unsigned most = g->choiceKind == TCG_CHOICE_DISCARD_TO_DECK ? 3 : g->choiceKind == TCG_CHOICE_TIME_CAPSULE ? 5 : 1,
                 fit = 0;
        int toDeck = g->choiceKind == TCG_CHOICE_DISCARD_TO_DECK || g->choiceKind == TCG_CHOICE_TIME_CAPSULE;
        for (k = 0; k < p->discardSize; k++)
            fit += ChoiceFits(g->choiceKind, CardAt(g, who, p->discard[k]));
        if (n > most || (g->choiceKind == TCG_CHOICE_TIME_CAPSULE && n && n != (fit < 5 ? fit : 5)))
            return ChoiceFail(g, g->choiceKind == TCG_CHOICE_TIME_CAPSULE ? "Choose 5 of them (or all, or none)." : "Too many cards.");
        for (k = 0; k < n; k++)
        {
            int at = Picked(a, k, p->discard, p->discardSize);
            if (at < 0 || !ChoiceFits(g->choiceKind, CardAt(g, who, p->discard[at])))
                return ChoiceFail(g, "That card can't be chosen.");
            if (toDeck)
                p->deck[p->deckSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
            else
            {
                TcgDuel_Log(g, "Player %u takes %s.", who + 1, CardAt(g, who, p->discard[at])->name);
                p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
            }
        }
        if (toDeck && n)
            TcgDuel_Shuffle(g, who);
        if (n)
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        break;
    }
    case TCG_CHOICE_REDRAW4:
        /* YES (target 1: the menu's answer): the hand shuffled into the deck, 4 drawn */
        if (a->target > 1)
            return ChoiceFail(g, "Choose YES or NO.");
        if (a->target)
        {
            while (p->handSize)
                p->deck[p->deckSize++] = p->hand[--p->handSize];
            TcgDuel_Shuffle(g, who);
            TcgDuel_Event(g, TCG_EV_DRAW, who, 0, TcgDuel_Draw(g, who, 4), 0, 0);
        }
        break;
    case TCG_CHOICE_HAND_TO_TOP:
    {
        /* Special Delivery: the card picked in the hand on top of the deck */
        int at = n == 1 ? Picked(a, 0, p->hand, p->handSize) : -1;
        if (at < 0)
            return ChoiceFail(g, "Choose a card from your hand.");
        p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        ChoiceEnd(g, TCG_CHOICE_HAND_TO_TOP);
        return TCG_OK;
    }
    case TCG_CHOICE_TOP3_ENERGY:
        /* the basic Energy picked among the top 3 into the hand, then the deck shuffled */
        for (k = 0; k < n; k++)
        {
            int at = Picked(a, k, p->deck, p->deckSize);
            if (at < 0 || at + 3 < (int)p->deckSize || CardAt(g, who, p->deck[at])->kind != TCG_ENERGY ||
                CardAt(g, who, p->deck[at])->id > 6)
                return ChoiceFail(g, "Only basic Energy cards among the top 3.");
            TcgDuel_Log(g, "Shown: %s.", CardAt(g, who, p->deck[at])->name);
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
        }
        TcgDuel_Shuffle(g, who);
        break;
    case TCG_CHOICE_EVOLVE:
    {
        /* the card picked from the deck onto the one of yours chosen (none: nothing), the deck shuffled */
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        if (n)
        {
            if (a->target >= TCG_FIELD_SIZE || !TcgDuel_Field(g, who, a->target) || at < 0 ||
                !EvolvesFrom(CardAt(g, who, p->deck[at]), TcgDuel_Field(g, who, a->target)))
                return ChoiceFail(g, "That card doesn't evolve from it.");
            Evolve(g, who, a->target, (unsigned)at);
        }
        TcgDuel_Shuffle(g, who);
        break;
    }
    case TCG_CHOICE_TOOL:
    {
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        if (n && (at < 0 || CardAt(g, who, p->deck[at])->kind != TCG_TRAINER || !(CardAt(g, who, p->deck[at])->flags & TCG_CF_TOOL)))
            return ChoiceFail(g, "Choose a Pokemon Tool.");
        if (n)
        {
            TcgDuel_Log(g, "Shown: %s.", CardAt(g, who, p->deck[at])->name);
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            TcgDuel_Shuffle(g, who);
        }
        break;
    }
    case TCG_CHOICE_SWITCH_THEIRS:
        /* (the other player's: their Benched one chosen comes in) */
        if (!ValidTarget(g, who ^ 1, a->target, 1))
            return ChoiceFail(g, "Choose one of their Benched Pokemon.");
        Switch(g, who ^ 1, a->target);
        break;
    case TCG_CHOICE_BENCH_HIT:
        if (!ValidTarget(g, who, a->target, 1))
            return ChoiceFail(g, "Choose one of your Benched Pokemon.");
        p->field[a->target].damage += 20; /* (no Weakness or Resistance for Benched Pokemon) */
        TcgDuel_Event(g, TCG_EV_DAMAGE, who, a->target, 20, 0, 0);
        break;
    case TCG_CHOICE_MOVE_ENERGY:
    {
        /* the Energy card chosen of the Active (choiceData) onto the Benched one chosen */
        struct TcgFieldCard *f = &p->field[0];
        if (!ValidTarget(g, who, a->target, 1))
            return ChoiceFail(g, "Choose one of your other Pokemon.");
        for (k = 0; k < f->attachedSize && f->attached[k] != g->choiceData; k++)
            ;
        if (k < f->attachedSize)
        {
            struct TcgFieldCard *to = &p->field[a->target];
            to->attached[to->attachedSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k);
            TcgDuel_Event(g, TCG_EV_ATTACH, who, a->target, 0, p->cards[to->attached[to->attachedSize - 1]], 0);
        }
        break;
    }
    case TCG_CHOICE_GUESS_KIND:
    case TCG_CHOICE_GUESS_LIGHT:
    {
        /* the card shown (the other player's, choiceData): right, the guesser draws (2 / 3); wrong, its player does */
        const struct TcgCard *c = CardAt(g, who ^ 1, g->choiceData);
        unsigned truth = g->choiceKind == TCG_CHOICE_GUESS_KIND
                             ? (c->kind == TCG_ENERGY ? 0u : c->kind == TCG_TRAINER ? 1u : 2u)
                             : (strstr(c->name, "Light") ? 0u : strstr(c->name, "Dark") ? 1u : 2u),
                 draws = g->choiceKind == TCG_CHOICE_GUESS_KIND ? 2 : 3, drawer = a->target == truth ? who : who ^ 1;
        if (a->target > 2)
            return ChoiceFail(g, "Make a guess.");
        TcgDuel_Log(g, "It was %s: %s.", c->name, a->target == truth ? "right" : "wrong");
        TcgDuel_Event(g, TCG_EV_DRAW, drawer, 0, TcgDuel_Draw(g, drawer, draws), 0, 0);
        break;
    }
    case TCG_CHOICE_BENCH_THEIRS:
    {
        /* Provoke: the Basic Pokemon picked in the other player's hand onto their Bench */
        struct TcgPlayer *q = &g->players[who ^ 1];
        for (k = 0; k < n; k++)
        {
            int at = Picked(a, k, q->hand, q->handSize);
            unsigned room;
            const struct TcgCard *c = at >= 0 ? CardAt(g, who ^ 1, q->hand[at]) : NULL;
            for (room = 1; room < TCG_FIELD_SIZE && q->field[room].stackSize; room++)
                ;
            if (!c || c->kind != TCG_POKEMON || c->stage || room == TCG_FIELD_SIZE)
                return ChoiceFail(g, "Only Basic Pokemon, while there's room.");
            memset(&q->field[room], 0, sizeof(q->field[room]));
            q->field[room].stack[q->field[room].stackSize++] = TcgPile_Remove(q->hand, &q->handSize, (unsigned)at);
            q->field[room].enteredTurn = g->turn;
            TcgDuel_Event(g, TCG_EV_PLAY, who ^ 1, room, 0, c->id, 0);
        }
        break;
    }
    case TCG_CHOICE_MANIPULATE:
    {
        /* the Basic picked onto the Bench; then 3 coins, a basic Energy from the discard pile onto it per heads */
        int at = n == 1 ? Picked(a, 0, p->discard, p->discardSize) : -1;
        const struct TcgCard *c = at >= 0 ? CardAt(g, who, p->discard[at]) : NULL;
        unsigned room, heads = 0;
        if (!n)
            break;
        for (room = 1; room < TCG_FIELD_SIZE && p->field[room].stackSize; room++)
            ;
        if (!c || c->kind != TCG_POKEMON || c->stage || (c->flags & TCG_CF_BABY) || room == TCG_FIELD_SIZE)
            return ChoiceFail(g, "Choose a Basic Pokemon (not a Baby).");
        memset(&p->field[room], 0, sizeof(p->field[room]));
        p->field[room].stack[p->field[room].stackSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
        p->field[room].enteredTurn = g->turn;
        TcgDuel_Event(g, TCG_EV_PLAY, who, room, 0, c->id, 0);
        for (k = 0; k < 3; k++)
            heads += TcgDuel_Coin(g, who);
        if (!heads)
            break;
        g->choiceData = (uint8_t)(heads << 3 | room);
        return ChoiceAgain(g, who, TCG_CHOICE_MANIPULATE_ENERGY);
    }
    case TCG_CHOICE_MANIPULATE_ENERGY:
    {
        unsigned slot = g->choiceData & 7, left = g->choiceData >> 3;
        int at = n == 1 ? Picked(a, 0, p->discard, p->discardSize) : -1;
        struct TcgFieldCard *f = &p->field[slot];
        if (at < 0 || CardAt(g, who, p->discard[at])->kind != TCG_ENERGY || CardAt(g, who, p->discard[at])->id > 6 || !f->stackSize)
            return ChoiceFail(g, "Choose a basic Energy card.");
        f->attached[f->attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_ATTACH, who, slot, 0, p->cards[f->attached[f->attachedSize - 1]], 0);
        if (--left)
        {
            g->choiceData = (uint8_t)(left << 3 | slot);
            return ChoiceAgain(g, who, TCG_CHOICE_MANIPULATE_ENERGY);
        }
        break;
    }
    case TCG_CHOICE_MANTLE_YES:
        if (a->target > 1)
            return ChoiceFail(g, "Choose YES or NO.");
        if (!a->target)
            break;
        ChoiceMantle(g, who, g->choiceCard, 0);
        if (g->phase != TCG_CHOICE && g->choiceData == 1)
            ChoiceMantle(g, who ^ 1, g->choiceCard, 1);   /* (nothing of yours to put back: theirs at once) */
        if (g->phase != TCG_CHOICE)
            ChoiceEnd(g, TCG_CHOICE_MANTLE_PICK);
        return TCG_OK;
    case TCG_CHOICE_MANTLE_PICK:
    {
        /* 3 basic Energy cards (all there are, if fewer) from the discard pile into the deck, shuffled; then theirs */
        unsigned need = ChoiceBasicEnergyInDiscard(g, who), theirs = g->choiceData;
        need = need < 3 ? need : 3;
        if (n != need)
            return ChoiceFail(g, "Choose 3 basic Energy cards (or all there are).");
        for (k = 0; k < n; k++)
        {
            int at = Picked(a, k, p->discard, p->discardSize);
            if (at < 0 || CardAt(g, who, p->discard[at])->kind != TCG_ENERGY || CardAt(g, who, p->discard[at])->id > 6)
                return ChoiceFail(g, "Only basic Energy cards.");
        }
        for (k = 0; k < n; k++)
            p->deck[p->deckSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)Picked(a, k, p->discard, p->discardSize));
        TcgDuel_Shuffle(g, who);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        if (!theirs)
        {
            ChoiceMantle(g, who ^ 1, g->choiceCard, 1);
            if (g->phase != TCG_CHOICE)
                ChoiceEnd(g, TCG_CHOICE_MANTLE_PICK);
            return TCG_OK;
        }
        break;
    }
    case TCG_CHOICE_HOWL_YES:
    {
        /* YES: the top 5 discarded, the Fire Energy among them onto Fire Pokemon of yours; the turn ends */
        unsigned fire = 0;
        if (a->target > 1)
            return ChoiceFail(g, "Choose YES or NO.");
        if (!a->target)
            break;
        for (k = 0; k < 5 && p->deckSize; k++)
        {
            fire += p->cards[p->deck[p->deckSize - 1]] == TCG_CARD_FIRE_ENERGY;
            p->discard[p->discardSize++] = p->deck[--p->deckSize];
        }
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, k, 0, 0);
        g->choiceEndTurn = 1;
        if (!fire)
            break;
        g->choiceData = (uint8_t)fire;
        return ChoiceAgain(g, who, TCG_CHOICE_HOWL_ATTACH);
    }
    case TCG_CHOICE_HOWL_ATTACH:
    {
        /* the last Fire Energy discarded (one of the 5) onto the Fire Pokemon chosen */
        struct TcgFieldCard *f = a->target < TCG_FIELD_SIZE ? &p->field[a->target] : NULL;
        unsigned at;
        if (!f || !f->stackSize || TcgDuel_Type(g, who, a->target) != TCG_FIRE)
            return ChoiceFail(g, "Choose one of your Fire Pokemon.");
        for (at = p->discardSize; at > 0 && p->cards[p->discard[at - 1]] != TCG_CARD_FIRE_ENERGY; at--)
            ;
        if (at)
        {
            f->attached[f->attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, at - 1);
            TcgDuel_Event(g, TCG_EV_ATTACH, who, a->target, 0, TCG_CARD_FIRE_ENERGY, 0);
        }
        if (--g->choiceData)
            return ChoiceAgain(g, who, TCG_CHOICE_HOWL_ATTACH);
        break;
    }
    case TCG_CHOICE_ORDER_THEIRS:
    {
        /* their top cards in the order picked, the first on top */
        struct TcgPlayer *d = &g->players[who ^ 1];
        unsigned top = d->deckSize < g->choiceData ? d->deckSize : g->choiceData, i, j;
        if (n != top)
            return ChoiceFail(g, "Put all of them in order.");
        for (i = 0; i < top; i++)
        {
            for (j = 0; j < top && d->deck[d->deckSize - 1 - j] != a->selections[i]; j++)
                ;
            for (k = 0; k < i; k++)
                if (a->selections[k] == a->selections[i])
                    j = top;
            if (j == top)
                return ChoiceFail(g, "Put all of them in order.");
        }
        for (i = 0; i < top; i++)
            d->deck[d->deckSize - 1 - i] = a->selections[i];
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who ^ 1, 0, 0, 0, 0);
        break;
    }
    case TCG_CHOICE_STAR_PIECE:
    {
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        if (n)
        {
            if (at < 0 || !EvolvesFrom(CardAt(g, who, p->deck[at]), TcgDuel_Field(g, who, g->choiceData)))
                return ChoiceFail(g, "Choose a card that evolves from it.");
            Evolve(g, who, g->choiceData, (unsigned)at);
        }
        TcgDuel_Shuffle(g, who);
        break;
    }
    case TCG_CHOICE_QUIZ1:
    case TCG_CHOICE_QUIZ3:
    {
        /* the guess (the answerer's): right, they draw (2 / 3); wrong, the quiz's player does */
        uint16_t opts[4];
        unsigned truth = TcgDuel_QuizOptions(g, opts), draws = g->choiceKind == TCG_CHOICE_QUIZ1 ? 2 : 3,
                 drawer = a->target == truth ? who : who ^ 1;
        if (a->target > 3)
            return ChoiceFail(g, "Make a guess.");
        TcgDuel_Event(g, TCG_EV_PEEK, who, 0, TCG_PEEK_THEIR_HAND, g->players[who ^ 1].cards[g->choiceData], 0);
        TcgDuel_Log(g, "%s.", a->target == truth ? "Right" : "Wrong");
        TcgDuel_Event(g, TCG_EV_DRAW, drawer, 0, TcgDuel_Draw(g, drawer, draws), 0, 0);
        break;
    }
    case TCG_CHOICE_CHAOS_FROM_SIDE:
    case TCG_CHOICE_CHAOS_TO_SIDE:
    {
        /* whose side, then which of them */
        int from = g->choiceKind == TCG_CHOICE_CHAOS_FROM_SIDE;
        if (a->target > 1)
            return ChoiceFail(g, "Choose yours or theirs.");
        return ChoiceAgain(g, who, from ? (a->target ? TCG_CHOICE_CHAOS_FROM_THEIRS : TCG_CHOICE_CHAOS_FROM_MINE)
                                        : (a->target ? TCG_CHOICE_CHAOS_TO_THEIRS : TCG_CHOICE_CHAOS_TO_MINE));
    }
    case TCG_CHOICE_CHAOS_FROM_MINE:
    case TCG_CHOICE_CHAOS_FROM_THEIRS:
    {
        unsigned side = g->choiceKind == TCG_CHOICE_CHAOS_FROM_THEIRS;
        const struct TcgFieldCard *f = a->target < TCG_FIELD_SIZE ? &g->players[who ^ side].field[a->target] : NULL;
        if (!f || !f->stackSize || !f->damage)
            return ChoiceFail(g, "Choose a Pokemon with damage counters.");
        g->choiceData = (uint8_t)(side << 3 | a->target);
        return ChoiceAgain(g, who, TCG_CHOICE_CHAOS_TO_SIDE);
    }
    case TCG_CHOICE_CHAOS_TO_MINE:
    case TCG_CHOICE_CHAOS_TO_THEIRS:
    {
        /* the damage counter moved (even if it Knocks Out that one) */
        unsigned side = g->choiceKind == TCG_CHOICE_CHAOS_TO_THEIRS, fromSide = g->choiceData >> 3 & 1,
                 fromSlot = g->choiceData & 7;
        struct TcgFieldCard *to = a->target < TCG_FIELD_SIZE ? &g->players[who ^ side].field[a->target] : NULL;
        struct TcgFieldCard *from = &g->players[who ^ fromSide].field[fromSlot];
        if (!to || !to->stackSize || (side == fromSide && a->target == fromSlot) || !from->damage)
            return ChoiceFail(g, "Choose another Pokemon.");
        from->damage -= 10;
        to->damage += 10;
        TcgDuel_Event(g, TCG_EV_HEAL, who ^ fromSide, fromSlot, 10, 0, 0);
        TcgDuel_Event(g, TCG_EV_DAMAGE, who ^ side, a->target, 10, 0, 0);
        break;
    }
    case TCG_CHOICE_FLOW_A:
    case TCG_CHOICE_FLOW_B:
    {
        /* an Energy card of one of theirs, then of another: the two switched */
        struct TcgPlayer *q = &g->players[who ^ 1];
        struct TcgFieldCard *f = a->target < TCG_FIELD_SIZE ? &q->field[a->target] : NULL;
        unsigned s, at;
        for (k = 0; f && n == 1 && k < f->attachedSize && f->attached[k] != a->selections[0]; k++)
            ;
        if (!f || n != 1 || k >= f->attachedSize || CardAt(g, who ^ 1, f->attached[k])->kind != TCG_ENERGY)
            return ChoiceFail(g, "Choose an Energy card attached to one of theirs.");
        if (g->choiceKind == TCG_CHOICE_FLOW_A)
        {
            g->choiceData = a->selections[0];
            return ChoiceAgain(g, who, TCG_CHOICE_FLOW_B);
        }
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            for (at = 0; at < q->field[s].attachedSize; at++)
                if (q->field[s].attached[at] == g->choiceData)
                {
                    if (s == a->target)
                        return ChoiceFail(g, "Choose an Energy card of another of theirs.");
                    q->field[s].attached[at] = a->selections[0];
                    f->attached[k] = g->choiceData;
                    TcgDuel_Event(g, TCG_EV_ATTACH, who ^ 1, s, 0, q->cards[a->selections[0]], 0);
                    TcgDuel_Event(g, TCG_EV_ATTACH, who ^ 1, a->target, 0, q->cards[g->choiceData], 0);
                    s = TCG_FIELD_SIZE;
                    break;
                }
        break;
    }
    case TCG_CHOICE_INVIGORATE_SIDE:
        if (a->target > 1 || !ChoiceDiscardBasic(g, who ^ a->target))
            return ChoiceFail(g, "Choose a discard pile with a Basic Pokemon and a Bench with room.");
        return ChoiceAgain(g, who, a->target ? TCG_CHOICE_INVIGORATE_THEIRS : TCG_CHOICE_INVIGORATE_MINE);
    case TCG_CHOICE_INVIGORATE_MINE:
    case TCG_CHOICE_INVIGORATE_THEIRS:
    {
        /* the Basic picked onto its player's Bench, half its HP (rounded down to 10) in damage counters */
        unsigned q = g->choiceKind == TCG_CHOICE_INVIGORATE_THEIRS ? who ^ 1 : who, room;
        struct TcgPlayer *t = &g->players[q];
        int at = n == 1 ? Picked(a, 0, t->discard, t->discardSize) : -1;
        const struct TcgCard *c = at >= 0 ? CardAt(g, q, t->discard[at]) : NULL;
        for (room = 1; room < TCG_FIELD_SIZE && t->field[room].stackSize; room++)
            ;
        if (!c || c->kind != TCG_POKEMON || c->stage || room == TCG_FIELD_SIZE)
            return ChoiceFail(g, "Choose a Basic Pokemon (with room on that Bench).");
        memset(&t->field[room], 0, sizeof(t->field[room]));
        t->field[room].stack[t->field[room].stackSize++] = TcgPile_Remove(t->discard, &t->discardSize, (unsigned)at);
        t->field[room].enteredTurn = g->turn;
        t->field[room].damage = (uint16_t)(c->hp / 2 / 10 * 10);
        TcgDuel_Event(g, TCG_EV_PLAY, q, room, 0, c->id, 0);
        if (t->field[room].damage)
            TcgDuel_Event(g, TCG_EV_DAMAGE, q, room, t->field[room].damage, 0, 0);
        break;
    }
    case TCG_CHOICE_WISH_ASK:
        /* (the other player: YES, the Wish's player switches the Prize with a card of their hand; NO, they draw) */
        if (a->target > 1)
            return ChoiceFail(g, "Choose YES or NO.");
        if (a->target)
            return ChoiceAgain(g, who ^ 1, TCG_CHOICE_WISH_SWAP);
        TcgDuel_Event(g, TCG_EV_DRAW, who ^ 1, 0, TcgDuel_Draw(g, who ^ 1, 1), 0, 0);
        break;
    case TCG_CHOICE_WISH_SWAP:
    {
        int at = n == 1 ? Picked(a, 0, p->hand, p->handSize) : -1;
        for (k = 0; k < p->prizeSize && p->prizes[k] != g->choiceData; k++)
            ;
        if (at < 0 || k >= p->prizeSize)
            return ChoiceFail(g, "Choose a card from your hand.");
        p->prizes[k] = p->hand[at];
        p->hand[at] = g->choiceData;
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        break;
    }
    case TCG_CHOICE_EEVEE_EVOLVE:
    {
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        unsigned slot = g->choiceData & 15;
        if (n)
        {
            const struct TcgCard *c = at >= 0 ? CardAt(g, who, p->deck[at]) : NULL;
            if (!c || !EvolvesFrom(c, TcgDuel_Field(g, who, slot)) || c->type != g->choiceData >> 4)
                return ChoiceFail(g, "Choose an Eevee evolution of the Energy's type.");
            Evolve(g, who, slot, (unsigned)at);
        }
        TcgDuel_Shuffle(g, who);
        break;
    }
    case TCG_CHOICE_SPORE:
    {
        /* a Hoppip or Skiploom not chosen yet and a card from the deck that evolves from it; none picked: done */
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        if (!n)
        {
            TcgDuel_Shuffle(g, who);
            break;
        }
        if (a->target >= TCG_FIELD_SIZE || !ChoiceSporeSlot(g, who, a->target) || at < 0 ||
            !EvolvesFrom(CardAt(g, who, p->deck[at]), TcgDuel_Field(g, who, a->target)))
            return ChoiceFail(g, "A Hoppip or Skiploom and a card that evolves from it.");
        Evolve(g, who, a->target, (unsigned)at);
        g->choiceData |= (uint8_t)(1u << a->target);
        return ChoiceAgain(g, who, TCG_CHOICE_SPORE);
    }
    case TCG_CHOICE_RETURN_MON:
    {
        /* one of yours and all its cards into your hand, then another (DONE: 7) */
        struct TcgFieldCard *f = a->target < TCG_FIELD_SIZE ? &p->field[a->target] : NULL;
        if (a->target == 7)
            break;
        if (!f || !f->stackSize)
            return ChoiceFail(g, "Choose one of your Pokemon.");
        while (f->stackSize)
            p->hand[p->handSize++] = f->stack[--f->stackSize];
        while (f->attachedSize)
        {
            p->attachmentExpires[f->attached[f->attachedSize - 1]] = 0;
            p->hand[p->handSize++] = f->attached[--f->attachedSize];
        }
        memset(f, 0, sizeof(*f));
        TcgDuel_Event(g, TCG_EV_TO_HAND, who, a->target, 0, 0, 0);
        return ChoiceAgain(g, who, TCG_CHOICE_RETURN_MON);
    }
    case TCG_CHOICE_DISCARD2:
    {
        unsigned need = p->handSize < 2 ? p->handSize : 2;
        if (n != need)
            return ChoiceFail(g, "Discard 2 cards from your hand.");
        for (k = 0; k < n; k++)
            if (Picked(a, k, p->hand, p->handSize) < 0 || (k && a->selections[0] == a->selections[1]))
                return ChoiceFail(g, "Choose cards in your hand.");
        for (k = 0; k < n; k++)
            p->discard[p->discardSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)Picked(a, k, p->hand, p->handSize));
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        break;
    }
    case TCG_CHOICE_CARD_FLIP:
    {
        /* the guess on their Prize choiceData (shown to both): right, draw 2 */
        const struct TcgCard *c = CardAt(g, who ^ 1, g->choiceData);
        unsigned truth = c->kind == TCG_ENERGY ? 0u : c->kind == TCG_TRAINER ? 1u : 2u;
        if (a->target > 2)
            return ChoiceFail(g, "Make a guess.");
        TcgDuel_Event(g, TCG_EV_PEEK, who, 0, TCG_PEEK_THEIR_PRIZE, c->id, 0);
        TcgDuel_Event(g, TCG_EV_PEEK, who ^ 1, 0, TCG_PEEK_MY_PRIZE, c->id, 0);
        TcgDuel_Log(g, "It was %s: %s.", c->name, a->target == truth ? "right" : "wrong");
        if (a->target == truth)
            TcgDuel_Event(g, TCG_EV_DRAW, who, 0, TcgDuel_Draw(g, who, 2), 0, 0);
        break;
    }
    case TCG_CHOICE_IMPOSTOR:
        /* YES: their Prizes into their deck, shuffled, as many new ones off the top */
        if (a->target > 1)
            return ChoiceFail(g, "Choose YES or NO.");
        if (a->target)
        {
            struct TcgPlayer *q = &g->players[who ^ 1];
            unsigned n = q->prizeSize;
            while (q->prizeSize)
                q->deck[q->deckSize++] = q->prizes[--q->prizeSize];
            TcgDuel_Shuffle(g, who ^ 1);
            while (q->prizeSize < n && q->deckSize)
                q->prizes[q->prizeSize++] = q->deck[--q->deckSize];
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who ^ 1, 0, 0, 0, 0);
        }
        break;
    case TCG_CHOICE_TEXTURE_RES:
    case TCG_CHOICE_TEXTURE_WEAK:
    {
        /* its Resistance / the Defending Pokemon's Weakness to the type chosen (9: as it is); then the other */
        int res = g->choiceKind == TCG_CHOICE_TEXTURE_RES;
        if (a->target > 9 || a->target == TCG_COLORLESS)
            return ChoiceFail(g, "Choose a type other than Colorless.");
        if (a->target < TCG_TYPES)
        {
            if (res)
                p->field[0].resistAs = (uint8_t)(a->target + 1);
            else
                g->players[who ^ 1].field[0].weakAs = (uint8_t)(a->target + 1);
        }
        if (res)
            return ChoiceAgain(g, who, TCG_CHOICE_TEXTURE_WEAK);
        break;
    }
    case TCG_CHOICE_PURE_BODY:
    {
        struct TcgFieldCard *f = g->choiceData < TCG_FIELD_SIZE ? &p->field[g->choiceData] : NULL;
        for (k = 0; f && n == 1 && k < f->attachedSize && f->attached[k] != a->selections[0]; k++)
            ;
        if (!f || n != 1 || k >= f->attachedSize)
            return ChoiceFail(g, "Choose one of its Energy cards.");
        p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        break;
    }
    case TCG_CHOICE_CHAIN_REACTION:
    {
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        if (n)
        {
            if (at < 0 || g->choiceData >= TCG_FIELD_SIZE ||
                !EvolvesFrom(CardAt(g, who, p->deck[at]), TcgDuel_Field(g, who, g->choiceData)))
                return ChoiceFail(g, "That card doesn't evolve from Eevee.");
            Evolve(g, who, g->choiceData, (unsigned)at);
        }
        TcgDuel_Shuffle(g, who);
        break;
    }
    case TCG_CHOICE_ARENA:
    {
        /* exactly down to 7 */
        if (n + 7 != p->handSize)
            return ChoiceFail(g, "Discard down to 7 cards.");
        for (k = 0; k < n; k++)
            if (Picked(a, k, p->hand, p->handSize) < 0)
                return ChoiceFail(g, "Choose cards in your hand.");
        for (k = 0; k < n; k++)
            p->discard[p->discardSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)Picked(a, k, p->hand, p->handSize));
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
        break;
    }
    case TCG_CHOICE_MOVE_THEIRS_FROM:
    {
        /* the basic Energy picked on the one of theirs chosen; then which of theirs it goes to */
        struct TcgFieldCard *f = a->target < TCG_FIELD_SIZE ? &g->players[who ^ 1].field[a->target] : NULL;
        for (k = 0; f && n == 1 && k < f->attachedSize && f->attached[k] != a->selections[0]; k++)
            ;
        if (!f || !f->stackSize || n != 1 || k >= f->attachedSize || CardAt(g, who ^ 1, f->attached[k])->kind != TCG_ENERGY ||
            CardAt(g, who ^ 1, f->attached[k])->id > 6)
            return ChoiceFail(g, "Choose a basic Energy card attached to one of theirs.");
        g->choiceData = a->selections[0];
        g->phase = TCG_TURN;
        TcgDuel_AskChoice(g, who, TCG_CHOICE_MOVE_THEIRS_TO, g->choiceCard, 1);
        return TCG_OK;
    }
    case TCG_CHOICE_MOVE_THEIRS_TO:
    {
        struct TcgPlayer *q = &g->players[who ^ 1];
        unsigned s;
        if (a->target >= TCG_FIELD_SIZE || !q->field[a->target].stackSize)
            return ChoiceFail(g, "Choose one of their Pokemon.");
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            for (k = 0; k < q->field[s].attachedSize; k++)
                if (q->field[s].attached[k] == g->choiceData)
                {
                    struct TcgFieldCard *to = &q->field[a->target];
                    if (s == a->target)
                        return ChoiceFail(g, "Choose another of their Pokemon.");
                    to->attached[to->attachedSize++] = TcgPile_Remove(q->field[s].attached, &q->field[s].attachedSize, k);
                    TcgDuel_Event(g, TCG_EV_ATTACH, who ^ 1, a->target, 0, q->cards[g->choiceData], 0);
                    s = TCG_FIELD_SIZE;
                    break;
                }
        break;
    }
    case TCG_CHOICE_WATER_TO_BENCH:
    {
        /* a Water Energy card of the Active onto the Benched one chosen, as many times as choiceData */
        int at = ChoiceEnergyOn(g, who, 0, TCG_WATER, 0);
        struct TcgFieldCard *to;
        if (!ValidTarget(g, who, a->target, 1) || at < 0)
            return ChoiceFail(g, "Choose one of your Benched Pokemon.");
        to = &p->field[a->target];
        to->attached[to->attachedSize++] = TcgPile_Remove(p->field[0].attached, &p->field[0].attachedSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_ATTACH, who, a->target, 0, p->cards[to->attached[to->attachedSize - 1]], 0);
        if (--g->choiceData)
            return ChoiceAgain(g, who, TCG_CHOICE_WATER_TO_BENCH);
        break;
    }
    case TCG_CHOICE_COUNTER_BENCH:
        /* a damage counter on the Benched one of theirs chosen, as many times as choiceData */
        if (!ValidTarget(g, who ^ 1, a->target, 1))
            return ChoiceFail(g, "Choose one of their Benched Pokemon.");
        g->players[who ^ 1].field[a->target].damage += 10;
        TcgDuel_Event(g, TCG_EV_DAMAGE, who ^ 1, a->target, 10, 0, 0);
        if (--g->choiceData)
            return ChoiceAgain(g, who, TCG_CHOICE_COUNTER_BENCH);
        break;   /* (Knock Outs once the choice is done: TcgDuel_Apply) */
    case TCG_CHOICE_ATTACH_LIGHTNING:
    {
        /* the Lightning Energy picked from the deck (none: done) onto the Lightning Pokemon chosen, as many times as
         * choiceData; the deck shuffled */
        int at = n ? Picked(a, 0, p->deck, p->deckSize) : -1;
        if (n)
        {
            struct TcgFieldCard *to = a->target < TCG_FIELD_SIZE ? &p->field[a->target] : NULL;
            if (at < 0 || CardAt(g, who, p->deck[at])->id != TCG_CARD_LIGHTNING_ENERGY || !to || !to->stackSize ||
                TcgDuel_Type(g, who, a->target) != TCG_LIGHTNING)
                return ChoiceFail(g, "A Lightning Energy onto one of your Lightning Pokemon.");
            to->attached[to->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            TcgDuel_Event(g, TCG_EV_ATTACH, who, a->target, 0, TCG_CARD_LIGHTNING_ENERGY, 0);
            if (--g->choiceData)
                return ChoiceAgain(g, who, TCG_CHOICE_ATTACH_LIGHTNING);   /* (shuffled when nothing more: ChoiceSkipped) */
        }
        TcgDuel_Shuffle(g, who);
        break;
    }
    case TCG_CHOICE_HEAL_MINE:
    case TCG_CHOICE_HEAL_THEIRS:
    {
        /* 2 damage counters (1 if it has only 1) off the damaged one chosen, the chooser's or the other player's */
        unsigned q = g->choiceKind == TCG_CHOICE_HEAL_MINE ? who : who ^ 1;
        struct TcgFieldCard *f = a->target < TCG_FIELD_SIZE ? &g->players[q].field[a->target] : NULL;
        if (!f || !f->stackSize || !f->damage)
            return ChoiceFail(g, "Choose a Pokemon with damage counters.");
        Heal(g, q, a->target, 20);
        break;
    }
    case TCG_CHOICE_SHUFFLE_THEIRS:
        /* YES (1): the other player shuffles their deck */
        if (a->target > 1)
            return ChoiceFail(g, "Choose YES or NO.");
        if (a->target)
        {
            TcgDuel_Shuffle(g, who ^ 1);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who ^ 1, 0, 0, 0, 0);
        }
        break;
    case TCG_CHOICE_PEEK_ZONE:
    {
        /* the zone chosen (TCG_PEEK_*, not a hand): its top 2 / 2 of its Prizes shown to the chooser */
        const struct TcgPlayer *q;
        if (a->target == TCG_PEEK_THEIR_HAND || a->target > TCG_PEEK_THEIR_PRIZE)
            return ChoiceFail(g, "Choose a deck or the Prizes.");
        q = &g->players[a->target == TCG_PEEK_MY_DECK || a->target == TCG_PEEK_MY_PRIZE ? who : who ^ 1];
        if (!(a->target == TCG_PEEK_MY_DECK || a->target == TCG_PEEK_THEIR_DECK ? q->deckSize : q->prizeSize))
            return ChoiceFail(g, "There's no card there to look at.");
        ChoicePeek(g, who, a->target, 2);
        break;
    }
    case TCG_CHOICE_TREATY:
    {
        /* Lt. Surge's Treaty, the other player's choice: 0 each player takes a Prize (face down: any one), 1 the
         * Treaty's player draws a card */
        unsigned q;
        if (a->target > 1)
            return ChoiceFail(g, "Choose one of the two.");
        if (a->target)
            TcgDuel_Event(g, TCG_EV_DRAW, who ^ 1, 0, TcgDuel_Draw(g, who ^ 1, 1), 0, 0);
        else
            for (q = 0; q < 2; q++)
            {
                struct TcgPlayer *t = &g->players[q];
                if (!t->prizeSize)
                    continue;
                t->hand[t->handSize++] = t->prizes[--t->prizeSize];
                TcgDuel_Event(g, TCG_EV_PRIZE, q, 0, t->prizeSize, 0, 0);
            }
        ChoiceEnd(g, TCG_CHOICE_TREATY);
        if (!a->target)
            TcgDuel_ResolveKnockouts(g); /* (a last Prize taken wins; both at once: sudden death) */
        return TCG_OK;
    }
    default:
        return ChoiceFail(g, "There is nothing to choose.");
    }
    ChoiceNext(g);
    return TCG_OK;
}
void TcgDuel_AiChoose(const struct TcgDuel *g, struct TcgAction *a)
{
    unsigned who = g->choicePlayer, k;
    const struct TcgPlayer *p = &g->players[who];
    memset(a, 0, sizeof(*a));
    a->type = TCG_CHOOSE;
    switch (g->choiceKind)
    {
    case TCG_CHOICE_DISCARD_POKEMON:
    case TCG_CHOICE_DISCARD_ANY:
        /* the last one discarded that fits (the most recent: often the best) */
        for (k = p->discardSize; k > 0 && !a->selectionCount; k--)
            if (ChoiceFits(g->choiceKind, CardAt(g, who, p->discard[k - 1])))
                a->selections[a->selectionCount++] = p->discard[k - 1];
        break;
    case TCG_CHOICE_DISCARD_TO_DECK:
    case TCG_CHOICE_TIME_CAPSULE:
    {
        /* (back into the deck: more to draw; Time Capsule's 5, or all) */
        unsigned most = g->choiceKind == TCG_CHOICE_TIME_CAPSULE ? 5 : 3;
        for (k = 0; k < p->discardSize && a->selectionCount < most; k++)
            if (ChoiceFits(g->choiceKind, CardAt(g, who, p->discard[k])))
                a->selections[a->selectionCount++] = p->discard[k];
        break;
    }
    case TCG_CHOICE_REDRAW4:
        a->target = p->handSize < 4; /* (a small hand for 4 new ones) */
        break;
    case TCG_CHOICE_TOP3_ENERGY:
        for (k = 0; k < 3 && k < p->deckSize; k++)
            if (CardAt(g, who, p->deck[p->deckSize - 1 - k])->kind == TCG_ENERGY && CardAt(g, who, p->deck[p->deckSize - 1 - k])->id <= 6)
                a->selections[a->selectionCount++] = p->deck[p->deckSize - 1 - k];
        break;
    case TCG_CHOICE_EVOLVE:
    {
        unsigned s, d;
        for (s = 0; s < TCG_FIELD_SIZE && !a->selectionCount; s++)
            for (d = 0; TcgDuel_Field(g, who, s) && p->field[s].stackSize < 4 && d < p->deckSize && !a->selectionCount; d++)
                if (EvolvesFrom(CardAt(g, who, p->deck[d]), TcgDuel_Field(g, who, s)))
                    a->target = (uint8_t)s, a->selections[a->selectionCount++] = p->deck[d];
        break;
    }
    case TCG_CHOICE_TOOL:
        for (k = 0; k < p->deckSize && !a->selectionCount; k++)
            if (CardAt(g, who, p->deck[k])->kind == TCG_TRAINER && (CardAt(g, who, p->deck[k])->flags & TCG_CF_TOOL))
                a->selections[a->selectionCount++] = p->deck[k];
        break;
    case TCG_CHOICE_SWITCH_THEIRS:
    {
        /* (theirs: the most damaged of the Bench comes in, else the first) */
        unsigned s, best = 0;
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (g->players[who ^ 1].field[s].stackSize &&
                (!best || g->players[who ^ 1].field[s].damage > g->players[who ^ 1].field[best].damage))
                best = s;
        a->target = (uint8_t)best;
        break;
    }
    case TCG_CHOICE_BENCH_HIT:
    case TCG_CHOICE_MOVE_ENERGY:
    {
        /* (yours: the healthiest of the Bench takes the 20; the Energy to the first that has Energy, else the first) */
        unsigned s, best = 0;
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (p->field[s].stackSize && (!best || (g->choiceKind == TCG_CHOICE_BENCH_HIT
                                                        ? p->field[s].damage < p->field[best].damage
                                                        : p->field[s].attachedSize > p->field[best].attachedSize)))
                best = s;
        a->target = (uint8_t)best;
        break;
    }
    case TCG_CHOICE_GUESS_KIND:
        a->target = 2; /* (Pokemon: the commonest guess) */
        break;
    case TCG_CHOICE_GUESS_LIGHT:
        a->target = 2;
        break;
    case TCG_CHOICE_BENCH_THEIRS:
        break; /* (none of theirs onto their Bench) */
    case TCG_CHOICE_CARD_FLIP:
        a->target = 2;
        break;
    case TCG_CHOICE_CHAOS_FROM_SIDE:
        a->target = !ChoiceDamaged(g, who);
        break;
    case TCG_CHOICE_MANIPULATE:
    {
        /* (the Basic with the most HP) */
        int best = -1;
        for (k = 0; k < p->discardSize; k++)
        {
            const struct TcgCard *c = CardAt(g, who, p->discard[k]);
            if (c->kind == TCG_POKEMON && !c->stage && !(c->flags & TCG_CF_BABY) &&
                (best < 0 || c->hp > CardAt(g, who, p->discard[best])->hp))
                best = (int)k;
        }
        if (best >= 0)
            a->selections[a->selectionCount++] = p->discard[best];
        break;
    }
    case TCG_CHOICE_MANIPULATE_ENERGY:
    case TCG_CHOICE_MANTLE_PICK:
    {
        unsigned most = g->choiceKind == TCG_CHOICE_MANTLE_PICK ? 3 : 1;
        for (k = 0; k < p->discardSize && a->selectionCount < most; k++)
            if (CardAt(g, who, p->discard[k])->kind == TCG_ENERGY && CardAt(g, who, p->discard[k])->id <= 6)
                a->selections[a->selectionCount++] = p->discard[k];
        break;
    }
    case TCG_CHOICE_MANTLE_YES:
        a->target = ChoiceBasicEnergyInDiscard(g, who) >= 3;   /* (only with Energy to put back) */
        break;
    case TCG_CHOICE_HOWL_YES:
        a->target = 0;   /* (the turn's attack is worth more) */
        break;
    case TCG_CHOICE_HOWL_ATTACH:
    {
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE && !(TcgDuel_Field(g, who, s) && TcgDuel_Type(g, who, s) == TCG_FIRE); s++)
            ;
        a->target = (uint8_t)s;
        break;
    }
    case TCG_CHOICE_ORDER_THEIRS:
    {
        /* (their Energy cards to the bottom of what's looked at) */
        const struct TcgPlayer *d = &g->players[who ^ 1];
        unsigned top = d->deckSize < g->choiceData ? d->deckSize : g->choiceData, pass;
        for (pass = 0; pass < 2; pass++)
            for (k = 0; k < top; k++)
                if ((CardAt(g, who ^ 1, d->deck[d->deckSize - 1 - k])->kind == TCG_ENERGY) == (pass == 1))
                    a->selections[a->selectionCount++] = d->deck[d->deckSize - 1 - k];
        break;
    }
    case TCG_CHOICE_STAR_PIECE:
        for (k = 0; k < p->deckSize && !a->selectionCount; k++)
            if (EvolvesFrom(CardAt(g, who, p->deck[k]), TcgDuel_Field(g, who, g->choiceData)))
                a->selections[a->selectionCount++] = p->deck[k];
        break;
    case TCG_CHOICE_QUIZ1:
    case TCG_CHOICE_QUIZ3:
        a->target = (uint8_t)(g->turn % 4);   /* (a guess) */
        break;
    case TCG_CHOICE_CHAOS_TO_SIDE:
        a->target = 1;
        break;
    case TCG_CHOICE_CHAOS_FROM_MINE:
    case TCG_CHOICE_CHAOS_FROM_THEIRS:
    case TCG_CHOICE_CHAOS_TO_MINE:
    case TCG_CHOICE_CHAOS_TO_THEIRS:
    {
        /* (off the most damaged of yours / the least of theirs; onto theirs nearest a Knock Out) */
        unsigned side = g->choiceKind == TCG_CHOICE_CHAOS_FROM_THEIRS || g->choiceKind == TCG_CHOICE_CHAOS_TO_THEIRS, s,
                 best = TCG_NONE, to = g->choiceKind >= TCG_CHOICE_CHAOS_TO_MINE;
        const struct TcgPlayer *q = &g->players[who ^ side];
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            unsigned left;
            if (!q->field[s].stackSize || (!to && !q->field[s].damage) ||
                (to && side == (g->choiceData >> 3 & 1u) && s == (g->choiceData & 7u)))
                continue;
            left = TcgDuel_MaxHp(g, who ^ side, s) - q->field[s].damage;
            if (best == TCG_NONE || (left < TcgDuel_MaxHp(g, who ^ side, best) - q->field[best].damage) == (side == 1))
                best = s;
        }
        a->target = (uint8_t)(best == TCG_NONE ? 0 : best);
        break;
    }
    case TCG_CHOICE_FLOW_A:
    case TCG_CHOICE_FLOW_B:
    {
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE && !a->selectionCount; s++)
        {
            int at = ChoiceEnergyOn(g, who ^ 1, s, TCG_TYPES, 0);
            unsigned j, mine = 0;
            for (j = 0; g->choiceKind == TCG_CHOICE_FLOW_B && j < g->players[who ^ 1].field[s].attachedSize; j++)
                mine |= g->players[who ^ 1].field[s].attached[j] == g->choiceData;
            if (at >= 0 && !mine)
                a->target = (uint8_t)s, a->selections[a->selectionCount++] = g->players[who ^ 1].field[s].attached[at];
        }
        break;
    }
    case TCG_CHOICE_INVIGORATE_SIDE:
        a->target = !ChoiceDiscardBasic(g, who);
        break;
    case TCG_CHOICE_INVIGORATE_MINE:
    case TCG_CHOICE_INVIGORATE_THEIRS:
    {
        const struct TcgPlayer *t = &g->players[g->choiceKind == TCG_CHOICE_INVIGORATE_THEIRS ? who ^ 1 : who];
        unsigned q = g->choiceKind == TCG_CHOICE_INVIGORATE_THEIRS ? who ^ 1 : who;
        for (k = 0; k < t->discardSize && !a->selectionCount; k++)
            if (CardAt(g, q, t->discard[k])->kind == TCG_POKEMON && !CardAt(g, q, t->discard[k])->stage)
                a->selections[a->selectionCount++] = t->discard[k];
        break;
    }
    case TCG_CHOICE_WISH_ASK:
        a->target = 0;
        break;
    case TCG_CHOICE_WISH_SWAP:
        if (p->handSize)
            a->selections[a->selectionCount++] = p->hand[0];
        break;
    case TCG_CHOICE_EEVEE_EVOLVE:
        for (k = 0; k < p->deckSize && !a->selectionCount; k++)
            if (EvolvesFrom(CardAt(g, who, p->deck[k]), TcgDuel_Field(g, who, g->choiceData & 15)) &&
                CardAt(g, who, p->deck[k])->type == g->choiceData >> 4)
                a->selections[a->selectionCount++] = p->deck[k];
        break;
    case TCG_CHOICE_SPORE:
    {
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE && !a->selectionCount; s++)
            for (k = 0; ChoiceSporeSlot(g, who, s) && k < p->deckSize && !a->selectionCount; k++)
                if (EvolvesFrom(CardAt(g, who, p->deck[k]), TcgDuel_Field(g, who, s)))
                    a->target = (uint8_t)s, a->selections[a->selectionCount++] = p->deck[k];
        break;
    }
    case TCG_CHOICE_RETURN_MON:
        a->target = 7;
        break;
    case TCG_CHOICE_DISCARD2:
        for (k = 0; k < 2 && k < p->handSize; k++)
            a->selections[a->selectionCount++] = p->hand[k];
        break;
    case TCG_CHOICE_IMPOSTOR:
        a->target = g->players[who ^ 1].prizeSize <= 2; /* (their last Prizes: shuffle them away) */
        break;
    case TCG_CHOICE_TEXTURE_RES:
    {
        /* (its Resistance to the Defending Pokemon's type; their Weakness to its own) */
        const struct TcgCard *d = TcgDuel_Field(g, who ^ 1, 0);
        a->target = d ? (uint8_t)TcgDuel_Type(g, who ^ 1, 0) : 9;
        if (a->target == TCG_COLORLESS)
            a->target = 9;
        break;
    }
    case TCG_CHOICE_TEXTURE_WEAK:
        a->target = (uint8_t)TcgDuel_Type(g, who, 0);
        if (a->target == TCG_COLORLESS)
            a->target = 9;
        break;
    case TCG_CHOICE_PURE_BODY:
    {
        /* (the first that isn't Water) */
        const struct TcgFieldCard *f = &p->field[g->choiceData < TCG_FIELD_SIZE ? g->choiceData : 0];
        for (k = 0; k < f->attachedSize && CardAt(g, who, f->attached[k])->type == TCG_WATER; k++)
            ;
        a->selections[0] = f->attached[k < f->attachedSize ? k : 0];
        a->selectionCount = 1;
        break;
    }
    case TCG_CHOICE_CHAIN_REACTION:
        for (k = 0; k < p->deckSize && !a->selectionCount; k++)
            if (EvolvesFrom(CardAt(g, who, p->deck[k]), TcgDuel_Field(g, who, g->choiceData)))
                a->selections[a->selectionCount++] = p->deck[k];
        break;
    case TCG_CHOICE_ARENA:
        /* (the last drawn kept: the first ones go) */
        for (k = 0; k + 7 < p->handSize; k++)
            a->selections[a->selectionCount++] = p->hand[k];
        break;
    case TCG_CHOICE_MOVE_THEIRS_FROM:
    {
        /* (off their Active first: its attack further away) */
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE && !a->selectionCount; s++)
        {
            int at = ChoiceEnergyOn(g, who ^ 1, s, TCG_TYPES, 1);
            if (at >= 0)
                a->target = (uint8_t)s, a->selections[a->selectionCount++] = g->players[who ^ 1].field[s].attached[at];
        }
        break;
    }
    case TCG_CHOICE_MOVE_THEIRS_TO:
    case TCG_CHOICE_COUNTER_BENCH:
    {
        /* (onto a Benched one of theirs, not where it came from; the counters on the one nearest a Knock Out) */
        unsigned s, best = TCG_NONE;
        const struct TcgPlayer *q = &g->players[who ^ 1];
        for (s = 1; s < TCG_FIELD_SIZE; s++)
        {
            int from = 0;
            unsigned k;
            for (k = 0; g->choiceKind == TCG_CHOICE_MOVE_THEIRS_TO && k < q->field[s].attachedSize; k++)
                from |= q->field[s].attached[k] == g->choiceData;
            if (!q->field[s].stackSize || from)
                continue;
            if (best == TCG_NONE || (g->choiceKind == TCG_CHOICE_COUNTER_BENCH &&
                                     TcgDuel_MaxHp(g, who ^ 1, s) - q->field[s].damage <
                                         TcgDuel_MaxHp(g, who ^ 1, best) - q->field[best].damage))
                best = s;
        }
        a->target = (uint8_t)(best == TCG_NONE ? 0 : best);
        break;
    }
    case TCG_CHOICE_WATER_TO_BENCH:
    {
        unsigned s;
        for (s = 1; s < TCG_FIELD_SIZE && !p->field[s].stackSize; s++)
            ;
        a->target = (uint8_t)s;
        break;
    }
    case TCG_CHOICE_ATTACH_LIGHTNING:
    {
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE && !(TcgDuel_Field(g, who, s) && TcgDuel_Type(g, who, s) == TCG_LIGHTNING); s++)
            ;
        a->target = (uint8_t)s;
        for (k = 0; k < p->deckSize && !a->selectionCount; k++)
            if (CardAt(g, who, p->deck[k])->id == TCG_CARD_LIGHTNING_ENERGY)
                a->selections[a->selectionCount++] = p->deck[k];
        break;
    }
    case TCG_CHOICE_HEAL_MINE:
    case TCG_CHOICE_HEAL_THEIRS:
    {
        /* (yours: the most damaged; theirs: the least damaged, the counters that matter least) */
        unsigned q = g->choiceKind == TCG_CHOICE_HEAL_MINE ? who : who ^ 1, s, best = TCG_NONE;
        const struct TcgPlayer *t = &g->players[q];
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (t->field[s].stackSize && t->field[s].damage &&
                (best == TCG_NONE || (g->choiceKind == TCG_CHOICE_HEAL_MINE ? t->field[s].damage > t->field[best].damage
                                                                            : t->field[s].damage < t->field[best].damage)))
                best = s;
        a->target = (uint8_t)(best == TCG_NONE ? 0 : best);
        break;
    }
    case TCG_CHOICE_SHUFFLE_THEIRS:
    {
        /* (the card they'll draw: shuffled away when it's a Trainer or an Evolution card, the useful ones) */
        const struct TcgPlayer *q = &g->players[who ^ 1];
        const struct TcgCard *top = q->deckSize ? CardAt(g, who ^ 1, q->deck[q->deckSize - 1]) : NULL;
        a->target = top && (top->kind == TCG_TRAINER || (top->kind == TCG_POKEMON && top->stage));
        break;
    }
    case TCG_CHOICE_PEEK_ZONE:
        /* (their deck: what they'll draw next) */
        a->target = g->players[who ^ 1].deckSize ? TCG_PEEK_THEIR_DECK : p->deckSize ? TCG_PEEK_MY_DECK
                    : g->players[who ^ 1].prizeSize ? TCG_PEEK_THEIR_PRIZE : TCG_PEEK_MY_PRIZE;
        break;
    case TCG_CHOICE_TREATY:
    {
        /* (the chooser is the Treaty's opponent: Prizes for both when they're ahead or level, else the draw) */
        a->target = g->players[who].prizeSize > g->players[who ^ 1].prizeSize;
        break;
    }
    case TCG_CHOICE_HAND_TO_TOP:
    {
        /* an Energy card back if there are 2 or more, else the last card */
        unsigned energy = 0;
        for (k = 0; k < p->handSize; k++)
            energy += CardAt(g, who, p->hand[k])->kind == TCG_ENERGY;
        for (k = 0; k < p->handSize && energy > 1; k++)
            if (CardAt(g, who, p->hand[k])->kind == TCG_ENERGY)
                break;
        if (!p->handSize)
            break;
        if (energy <= 1 || k >= p->handSize)
            k = p->handSize - 1u;
        a->selections[0] = p->hand[k];
        a->selectionCount = 1;
        break;
    }
    }
}
