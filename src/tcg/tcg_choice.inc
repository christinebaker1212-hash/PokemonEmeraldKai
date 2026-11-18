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
    }
    return 1;
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
        if (stage == 0 && (player & 1) != g->current)
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
    if (g->choiceStage == 0 && g->choicePlayer != g->current)
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
