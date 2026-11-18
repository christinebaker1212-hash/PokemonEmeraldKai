/* The regional sets' powers (docs/TCG_REGIONS.md; tools/tcg/regions/powers.py names each and gives the used ones'
 * scripts): when a used one can't be used, using it, and the passive ones' hooks. Included by tcg_powers.c. */
static unsigned RpCount(const struct TcgDuel *g, unsigned player, unsigned power)
{
    unsigned s, n = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        n += TcgPower_Working(g, player, s) == power;
    return n;
}
static int RpBench(const struct TcgDuel *g, unsigned player)
{
    unsigned s;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        if (g->players[player].field[s].stackSize)
            return 1;
    return 0;
}
static int RpEnergyOn(const struct TcgDuel *g, unsigned player, unsigned slot, int type, int basicOnly)
{
    const struct TcgPlayer *p = &g->players[player];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[f->attached[i]]);
        if (c && c->kind == TCG_ENERGY && (type < 0 || c->type == (unsigned)type) && (!basicOnly || c->id <= 6))
            return (int)f->attached[i];
    }
    return -1;
}
/* That Pokemon and its cards shuffled into its owner's deck (Bolt, Time Travel, Scram, Rebellion) */
static void RpToDeck(struct TcgDuel *g, unsigned player, unsigned slot)
{
    struct TcgPlayer *q = &g->players[player];
    struct TcgFieldCard *f = &q->field[slot];
    unsigned k;
    for (k = 0; k < f->stackSize + f->attachedSize; k++)
    {
        uint8_t card = k < f->stackSize ? f->stack[k] : f->attached[k - f->stackSize];
        q->attachmentExpires[card] = 0;
        q->deck[q->deckSize++] = card;
    }
    memset(f, 0, sizeof(*f));
    TcgDuel_Shuffle(g, player);
    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, player, 0, 0, 0, 0);
}
/* Why a regional used power can't be used now (NULL: it can) */
static const char *RegionBlock(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned power)
{
    const struct TcgPlayer *p = &g->players[player], *op = &g->players[player ^ 1];
    unsigned flags = TcgPower_Flags(power), s, i;
    if ((flags & 2) && slot)
        return "It works only while it's your Active Pokemon.";
    if ((flags & 4) && !slot)
        return "It works only while it's on your Bench.";
    {
        /* (the e-Card sets' words: "can't be used if it is affected by a Special Condition": Poisoned and Burned too) */
        const struct TcgCard *c = TcgDuel_Field(g, player, slot);
        const struct TcgFieldCard *f = &p->field[slot];
        const char *have;
        if (c && c->attacks[0].description && strstr(c->attacks[0].description, "affected by a Special Condition") &&
            (f->condition || f->poison || f->burned))
            return "It's affected by a Special Condition.";
        /* (Unown: "if you have Unown [F], Unown [I], Unown [N], and Unown [D] on your Bench") */
        if (c && c->attacks[0].description && (have = strstr(c->attacks[0].description, "if you have Unown [")) != NULL)
        {
            const char *end = strstr(have, "on your Bench");
            for (have = strstr(have, "Unown ["); have && end && have < end; have = strstr(have + 1, "Unown ["))
            {
                char want[16];
                unsigned k = 0, b;
                while (have[k] && have[k] != ']' && k < sizeof(want) - 2)
                    want[k] = have[k], k++;
                want[k] = ']', want[k + 1] = 0;
                for (b = 1; b < TCG_FIELD_SIZE; b++)
                    if (TcgDuel_Field(g, player, b) && !strcmp(TcgDuel_Field(g, player, b)->name, want))
                        break;
                if (b == TCG_FIELD_SIZE)
                    return "Those Unown aren't all on your Bench.";
            }
        }
    }
    switch (power)
    {
    case TCG_POWER_R_EVOLUTION_HELPER:
    {
        unsigned k;
        for (k = 0; k < p->deckSize; k++)
            if (TcgDuel_EvolvesFrom(Tcg_Card(p->cards[p->deck[k]]), TcgDuel_Field(g, player, 0)))
                break;
        if (k == p->deckSize)
            return "Nothing in your deck evolves from your Active Pokemon.";
        break;
    }
    default:
        break;
    }
    switch (power)
    {
    case TCG_POWER_R_JOIN:
        /* ([J], [O], [I] are read from its words; the fourth, printed "Unown Dragon" in the card list, is [N]: J-O-I-N) */
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (TcgDuel_Field(g, player, s) && !strcmp(TcgDuel_Field(g, player, s)->name, "Unown [N]"))
                break;
        if (s == TCG_FIELD_SIZE)
            return "Those Unown aren't all on your Bench.";
        break;
    case TCG_POWER_R_GOOD_NEIGHBOR:
        if (p->turnBuffs & TCG_BUFF_NEIGHBOR)
            return "You've used a Good Neighbor this turn.";
        break;
    case TCG_POWER_R_BACKUP:
        if (p->handSize > 2)
            return "You have more than 2 cards in your hand.";
        break;
    case TCG_POWER_R_SOAK_UP:
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (s != slot && RpEnergyOn(g, player, s, TCG_GRASS, 1) >= 0)
                break;
        if (s == TCG_FIELD_SIZE)
            return "Your other Pokemon have no Grass Energy.";
        break;
    case TCG_POWER_R_ELECTROMAGNETIC:
    {
        unsigned holders = 0, magnets = 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (TcgDuel_Field(g, player, s) && TcgFlow_InNameGroup(TcgDuel_Field(g, player, s)->name, 6))
                magnets++, holders += p->field[s].attachedSize != 0;
        if (magnets < 2 || !holders)
            return "You need two Magnemite or Magneton, one with Energy.";
        break;
    }
    case TCG_POWER_R_BEAR:
        if (p->field[slot].damage + 10u >= TcgDuel_MaxHp(g, player, slot))
            return "It has 10 HP left.";
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (s != slot && p->field[s].damage && TcgDuel_Field(g, player, s) && strstr(TcgDuel_Field(g, player, s)->name, "Unown"))
                break;
        if (s == TCG_FIELD_SIZE)
            return "None of your other Unown has damage counters.";
        break;
    case TCG_POWER_R_HEAT_UP:
    {
        unsigned mine = 0, theirs = 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            unsigned k;
            for (k = 0; k < p->field[s].attachedSize; k++)
                mine += Tcg_Card(p->cards[p->field[s].attached[k]])->kind == TCG_ENERGY;
            for (k = 0; k < op->field[s].attachedSize; k++)
                theirs += Tcg_Card(op->cards[op->field[s].attached[k]])->kind == TCG_ENERGY;
        }
        if (theirs <= mine)
            return "Your opponent doesn't have more Energy cards attached.";
        if (!RpBench(g, player))
            return "You have no Benched Pokemon.";
        break;
    }
    case TCG_POWER_R_MIRACLE_SHIFT:
        for (s = 0; s < TCG_FIELD_SIZE && RpEnergyOn(g, player, s, -1, 1) < 0; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "None of your Pokemon has basic Energy.";
        break;
    case TCG_POWER_R_RECONSTRUCTION:
        for (i = 0; i < p->handSize && p->cards[p->hand[i]] > 6; i++)
            ;
        if (i == p->handSize)
            return "You have no basic Energy card in your hand.";
        break;
    case TCG_POWER_R_STRANGE_TENTACLES:
    {
        unsigned mine = 0, theirs = 0;
        for (i = 0; i < p->field[0].attachedSize; i++)
            mine += Tcg_Card(p->cards[p->field[0].attached[i]])->kind == TCG_ENERGY;
        for (i = 0; i < op->field[0].attachedSize; i++)
            theirs += Tcg_Card(op->cards[op->field[0].attached[i]])->kind == TCG_ENERGY;
        if (theirs >= mine)
            return "The Defending Pokemon doesn't have fewer Energy cards than your Active Pokemon.";
        for (i = 0; i < op->discardSize && Tcg_Card(op->cards[op->discard[i]])->kind != TCG_ENERGY; i++)
            ;
        if (i == op->discardSize)
            return "There's no Energy card in their discard pile.";
        break;
    }
    case TCG_POWER_R_DRIVE_OFF:
    case TCG_POWER_R_MAJOR_TSUNAMI:
        if (power == TCG_POWER_R_DRIVE_OFF && !RpBench(g, player ^ 1))
            return "They have no Benched Pokemon.";
        if (power == TCG_POWER_R_DRIVE_OFF && TcgDuel_Anchored(g, player ^ 1))
            return "The Defending Pokemon can't be switched.";
        break;
    case TCG_POWER_R_DARK_MOON:
        if (!TcgDuel_Energy(g, player, slot, TCG_DARKNESS))
            return "It has no Darkness Energy.";
        break;
    case TCG_POWER_R_ANCIENT_WIND:
        if (g->bodiesOffTurn == g->turn)
            return "Poke-Bodies are already ignored this turn.";
        break;
    case TCG_POWER_R_MAGNETIC_CALL:
    case TCG_POWER_R_REVIVE_FRIENDS:
    case TCG_POWER_R_REVIVE_FOSSIL:
        for (s = 1; s < TCG_FIELD_SIZE && p->field[s].stackSize; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "Your Bench is full.";
        break;
    case TCG_POWER_R_EARTH_RAGE:
    case TCG_POWER_R_FRAGRANCE_TRAP:
    case TCG_POWER_R_FRAGRANCE_TRAP_GYM:
        if (!RpBench(g, player ^ 1))
            return "They have no Benched Pokemon.";
        break;
    case TCG_POWER_R_MIRACULOUS_POWDER:
        if (!p->field[0].condition && !p->field[0].poison && !p->field[0].burned)
            return "Your Active Pokemon has no Special Condition.";
        break;
    case TCG_POWER_R_NATURAL_HEALING:
        if (!p->field[slot].damage)
            return "It has no damage counters.";
        break;
    case TCG_POWER_R_STRANGE_SPIRAL:
        if (RpEnergyOn(g, player, slot, -1, 1) < 0)
            return "It has no basic Energy to discard.";
        break;
    case TCG_POWER_R_BEATING_WINGS:
        if (!RpBench(g, player))
            return "You have no Benched Pokemon.";
        break;
    case TCG_POWER_R_ENERGY_RETURN:
    case TCG_POWER_R_ENERGY_JUMP:
        for (s = 0; s < TCG_FIELD_SIZE && RpEnergyOn(g, player, s, -1, 0) < 0; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "None of your Pokemon has Energy.";
        break;
    case TCG_POWER_R_WATER_CYCLONE:
        if (RpEnergyOn(g, player, 0, TCG_WATER, 0) < 0 || !RpBench(g, player))
            return "Your Active Pokemon has no Water Energy to move to the Bench.";
        break;
    case TCG_POWER_R_ENERGY_CHARGE:
        for (s = 1; s < TCG_FIELD_SIZE && RpEnergyOn(g, player, s, TCG_LIGHTNING, 0) < 0; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "None of your Benched Pokemon has Lightning Energy.";
        break;
    case TCG_POWER_R_ENERGY_CONNECT:
        for (s = 1; s < TCG_FIELD_SIZE && RpEnergyOn(g, player, s, -1, 1) < 0; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "None of your Benched Pokemon has basic Energy.";
        break;
    case TCG_POWER_R_SUPER_DYNAMO:
        for (i = 0; i < p->discardSize && p->cards[p->discard[i]] != TCG_CARD_LIGHTNING_ENERGY; i++)
            ;
        if (i == p->discardSize)
            return "There's no Lightning Energy in your discard pile.";
        break;
    case TCG_POWER_R_FLOWER_SUPPLEMENT:
        for (i = 0; i < p->handSize && p->cards[p->hand[i]] > 6; i++)
            ;
        if (i == p->handSize || !RpBench(g, player))
            return "You need a basic Energy card in your hand and a Benched Pokemon.";
        break;
    case TCG_POWER_R_CARRY_OFF:
    case TCG_POWER_R_GLARING_GAZE:
        if (!op->handSize)
            return "Their hand is empty.";
        break;
    case TCG_POWER_R_SCAVENGER_HUNT:
        if (p->handSize < 2)
            return "You need 2 cards in your hand.";
        break;
    case TCG_POWER_R_SPATIAL_DISTORTION:
        for (i = 0; i < p->discardSize && !(Tcg_Card(p->cards[p->discard[i]])->flags & TCG_CF_STADIUM); i++)
            ;
        if (i == p->discardSize)
            return "There's no Stadium card in your discard pile.";
        break;
    }
    return NULL;
}
static int RegionUse(struct TcgDuel *g, const struct TcgAction *a, unsigned slot, unsigned power)
{
    const char *script = TcgPower_Script(power);
    int result;
    if (!script)
        return Fail(g, "This Pokemon Power can't be used.");
    Announce(g, g->current, slot, power);
    result = TcgDuel_RunPowerScript(g, slot, script, a);
    if (result == TCG_OK && power == TCG_POWER_R_GOOD_NEIGHBOR)
        g->players[g->current].turnBuffs |= TCG_BUFF_NEIGHBOR;
    return result;
}

/* ------------------------------------------------------------------------------------------------ passive hooks */
/* An attack's damage to that Pokemon (after Weakness and Resistance) */
static int RegionDamage(struct TcgDuel *g, unsigned player, unsigned slot, int amount)
{
    unsigned power = TcgPower_Working(g, player, slot), attackerType;
    const struct TcgCard *attacker = TcgDuel_Field(g, g->current, 0);
    if (player == g->current || amount <= 0)
        return amount;
    switch (power)
    {
    case TCG_POWER_R_ROCK_BODY:
    case TCG_POWER_R_SHELL_ARMOR:
    case TCG_POWER_R_EXOSKELETON_10:
        amount -= 10;
        break;
    case TCG_POWER_R_THICK_SHELL:
        if (attacker && attacker->stage)
            amount -= 10;
        break;
    case TCG_POWER_R_DENSE_BODY:
        if (attacker && !attacker->stage && !(attacker->flags & TCG_CF_BABY))
            amount -= 20;
        break;
    case TCG_POWER_R_ENERGY_BARRIER:
    {
        /* 10 less for each basic Energy card on it, 20 at most */
        const struct TcgFieldCard *f = &g->players[player].field[slot];
        unsigned i, n = 0;
        for (i = 0; i < f->attachedSize; i++)
            n += Tcg_Card(g->players[player].cards[f->attached[i]])->kind == TCG_ENERGY &&
                 g->players[player].cards[f->attached[i]] <= 6;
        amount -= 10 * (int)(n > 2 ? 2 : n);
        break;
    }
    case TCG_POWER_R_EXOSKELETON:
    case TCG_POWER_R_VASE_BODY:
        amount -= 20;
        break;
    case TCG_POWER_R_HARD_SHELL:
        if (amount <= 40 && amount > 10)
            amount = 10;
        break;
    case TCG_POWER_R_STRANGE_BARRIER:
        if (attacker && !attacker->stage && amount >= 20)
            amount = 10;
        break;
    }
    /* the Unown [Darkness] / [Metal] / [Normal] (one of a kind in play): that type's damage to any of yours 30 less */
    attackerType = TcgDuel_Type(g, g->current, 0);
    if ((attackerType == TCG_DARKNESS && RpCount(g, player, TCG_POWER_R_UNOWN_DARKNESS) == 1) ||
        (attackerType == TCG_METAL && RpCount(g, player, TCG_POWER_R_UNOWN_METAL) == 1) ||
        (attackerType == TCG_COLORLESS && RpCount(g, player, TCG_POWER_R_UNOWN_NORMAL) == 1))
        amount -= 30;
    /* Relaxing Scent (either Active): half, rounded up to the nearest 10 */
    if (TcgPower_Working(g, 0, 0) == TCG_POWER_R_RELAXING_SCENT || TcgPower_Working(g, 1, 0) == TCG_POWER_R_RELAXING_SCENT)
        amount = (amount / 2 + 9) / 10 * 10;
    return amount < 0 ? 0 : amount;
}
static int RegionImmune(struct TcgDuel *g, unsigned player, unsigned slot)
{
    unsigned power = TcgPower_Working(g, player, slot);
    if (player == g->current)
        return 0;
    if (power == TCG_POWER_R_IMMUNITY)
        return 1;
    if (power == TCG_POWER_R_NEUTRAL_SHIELD && g->players[g->current].field[0].stackSize > 1)
        return 1;
    if (power == TCG_POWER_R_FLUFF && g->players[player].field[slot].damage && TcgDuel_Coin(g, player))
        return 1;
    return 0;
}
/* The Retreat Cost's change (Gluey Slime: 1 more, once; Lightweight, Extreme Speed, Heavyweight) */
static int RegionRetreat(const struct TcgDuel *g, unsigned player)
{
    unsigned power = TcgPower_Working(g, player, 0), s, who;
    int delta = 0;
    for (who = 0; who < 2 && !delta; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (TcgPower_Working(g, who, s) == TCG_POWER_R_GLUEY_SLIME)
            {
                delta = 1;
                break;
            }
    /* (Unown [Z]'s Zoom on the Bench: an Unown retreats for nothing) */
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        if (TcgPower_Working(g, player, s) == TCG_POWER_R_ZOOM && TcgDuel_Field(g, player, 0) &&
            strstr(TcgDuel_Field(g, player, 0)->name, "Unown"))
            return -99;
    if (power == TCG_POWER_R_PSYCHOFLOW && RpEnergyOn(g, player, 0, TCG_PSYCHIC, 0) >= 0)
        return -99;
    if (power == TCG_POWER_R_SLIPPERY_SKIN && g->players[player ^ 1].field[0].stackSize >= 2)
        return -99;
    if (power == TCG_POWER_R_SYNCHRONIZED_LIFT && TcgDuel_Field(g, player, 0))
    {
        /* (the other two of Articuno ex, Zapdos ex and Moltres ex in play) */
        static const char *const birds[3] = {"Articuno ex", "Zapdos ex", "Moltres ex"};
        unsigned b, have = 0;
        for (b = 0; b < 3; b++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (TcgDuel_Field(g, player, s) && !strcmp(TcgDuel_Field(g, player, s)->name, birds[b]) &&
                    strcmp(TcgDuel_Field(g, player, 0)->name, birds[b]))
                {
                    have++;
                    break;
                }
        if (have == 2)
            return -99;
    }
    if (power == TCG_POWER_R_CONDUCTIVE_BODY)
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            delta -= TcgDuel_Field(g, player, s) && !strcmp(TcgDuel_Field(g, player, s)->name, "Magnemite");
    if (power == TCG_POWER_R_LIGHTWEIGHT)
        delta -= (int)TcgDuel_Energy(g, player, 0, TCG_GRASS);
    else if (power == TCG_POWER_R_EXTREME_SPEED)
        delta -= (int)TcgDuel_Energy(g, player, 0, TCG_NONE);
    else if (power == TCG_POWER_R_HEAVYWEIGHT && TcgDuel_Energy(g, player, 0, TCG_GRASS))
        delta += 2;
    return delta;
}
int TcgPower_AttachBlock(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned energy)
{
    unsigned power = TcgPower_Working(g, player, slot);
    const struct TcgCard *c = Tcg_Card(energy);
    if (!c || c->kind != TCG_ENERGY)
        return 0;
    if (power == TCG_POWER_R_PURE_BODY_WATER && c->type == TCG_WATER && !g->players[player].field[slot].attachedSize)
        return 1;   /* (Suicune's Pure Body: no Water Energy from the hand while it has no Energy) */
    return (power == TCG_POWER_R_ANTI_LIGHTNING && energy == TCG_CARD_LIGHTNING_ENERGY) ||
           (power == TCG_POWER_R_WATER_IMMUNITY && energy == TCG_CARD_WATER_ENERGY) ||
           (power == TCG_POWER_R_FIRE_IMMUNITY && energy == TCG_CARD_FIRE_ENERGY);
}
/* After an Energy card was attached from the hand to that Pokemon */
void TcgPower_AfterAttach(struct TcgDuel *g, unsigned player, unsigned slot, unsigned physical)
{
    struct TcgPlayer *p = &g->players[player];
    struct TcgFieldCard *f = &p->field[slot];
    unsigned power = TcgPower_Working(g, player, slot), id = p->cards[physical], s, who, i;
    if (power == TCG_POWER_R_PURE_BODY_WATER && Tcg_Card(id)->kind == TCG_ENERGY && Tcg_Card(id)->type == TCG_WATER)
    {
        /* Suicune's Pure Body: an Energy card of its discarded, the player's pick (tcg_choice.inc) */
        Announce(g, player, slot, power);
        g->choiceData = (uint8_t)slot;
        TcgDuel_AskChoice(g, player, TCG_CHOICE_PURE_BODY, TcgDuel_Field(g, player, slot)->id, 1);
        return;
    }
    if (power == TCG_POWER_R_CRYSTAL_TYPE && id >= 1 && id <= 6)
    {
        /* Crystal Type: a basic Energy of a type its words name makes it that type until the end of the turn (as
         * Venomoth's Shift does) */
        static const char *const types[TCG_TYPES] = {"Fire", "Grass", "Lightning", "Water", "Fighting", "Psychic",
                                                     "Colorless", "Darkness", "Metal"};
        const struct TcgCard *c = TcgDuel_Field(g, player, slot), *e = Tcg_Card(id);
        const char *words = c ? c->attacks[0].description : NULL, *end = words ? strstr(words, "basic Energy") : NULL;
        const char *at = words && e ? strstr(words, types[e->type]) : NULL;
        if (at && end && at < end)
        {
            Announce(g, player, slot, power);
            f->shift = (uint8_t)(e->type + 1);
        }
    }
    if ((power == TCG_POWER_R_PURE_BODY_WATER && id == TCG_CARD_WATER_ENERGY) ||
        (power == TCG_POWER_R_PURE_BODY_FIRE && id == TCG_CARD_FIRE_ENERGY) ||
        (power == TCG_POWER_R_PURE_BODY_LIGHTNING && id == TCG_CARD_LIGHTNING_ENERGY))
    {
        /* an Energy card already on it is discarded (the oldest) */
        for (i = 0; i < f->attachedSize; i++)
            if (f->attached[i] != physical && Tcg_Card(p->cards[f->attached[i]])->kind == TCG_ENERGY)
            {
                Announce(g, player, slot, power);
                p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i);
                break;
            }
    }
    if ((power == TCG_POWER_R_SELF_HEALING_FIRE && id == TCG_CARD_FIRE_ENERGY) ||
        (power == TCG_POWER_R_SELF_HEALING_LIGHTNING && id == TCG_CARD_LIGHTNING_ENERGY) ||
        (power == TCG_POWER_R_SELF_HEALING_WATER && id == TCG_CARD_WATER_ENERGY))
    {
        if (f->condition || f->poison || f->burned)
        {
            Announce(g, player, slot, power);
            f->condition = TCG_HEALTHY;
            f->poison = f->burned = 0;
            TcgDuel_Event(g, TCG_EV_STATUS, player, slot, TCG_HEALTHY, 0, 0);
        }
    }
    if (power == TCG_POWER_R_LIGHTNING_BURST && id == TCG_CARD_LIGHTNING_ENERGY && player == g->current &&
        TcgDuel_OpponentSwitch(g, player ^ 1))
        Announce(g, player, slot, power); /* (Lightning Burst: they choose who comes in) */
    if (power == TCG_POWER_R_HEALING_FIRE && id == TCG_CARD_FIRE_ENERGY && f->damage)
    {
        Announce(g, player, slot, power);
        f->damage -= 10;
        TcgDuel_Event(g, TCG_EV_HEAL, player, slot, 10, 0, 0);
    }
    /* Conductivity (one in play): 10 to a Pokemon the opponent attaches Energy to */
    for (who = 0; who < 2; who++)
        if (who != player && RpCount(g, who, TCG_POWER_R_CONDUCTIVITY) == 1)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (TcgPower_Working(g, who, s) == TCG_POWER_R_CONDUCTIVITY)
                {
                    Announce(g, who, s, TCG_POWER_R_CONDUCTIVITY);
                    PlainDamage(g, player, slot, 10);
                    break;
                }
}
/* What type an Energy card on that Pokemon provides (Dark Aura, Photosynthesis), how much (Wild Growth) */
unsigned TcgPower_EnergyType(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned type, unsigned *amount)
{
    unsigned power = TcgPower_Working(g, player, slot), s;
    if (power == TCG_POWER_R_DARK_AURA)
        return TCG_DARKNESS;
    if (power == TCG_POWER_R_PHOTOSYNTHESIS)
        return TCG_GRASS;
    if (power == TCG_POWER_R_RARE_METAL && type < TCG_COLORLESS)
        return TCG_METAL;
    if (type == TCG_GRASS && amount && *amount == 1)
    {
        const struct TcgCard *c = TcgDuel_Field(g, player, slot);
        if (c && c->kind == TCG_POKEMON && c->type == TCG_GRASS)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (TcgPower_Working(g, player, s) == TCG_POWER_R_WILD_GROWTH)
                {
                    *amount = 2;
                    break;
                }
    }
    return type;
}
int TcgPower_WindBlows(const struct TcgDuel *g)
{
    return TcgPower_Working(g, 0, 0) == TCG_POWER_R_MIRACULOUS_WIND ||
           TcgPower_Working(g, 1, 0) == TCG_POWER_R_MIRACULOUS_WIND;
}
/* player's Active Pokemon went to Bench slot: Magma Pool (a Fire Energy card of its onto the new Active) */
void TcgPower_ToBench(struct TcgDuel *g, unsigned player, unsigned slot)
{
    struct TcgFieldCard *f, *to;
    unsigned k;
    if (player > 1 || !slot || slot >= TCG_FIELD_SIZE || TcgPower_Working(g, player, slot) != TCG_POWER_R_MAGMA_POOL)
        return;
    f = &g->players[player].field[slot], to = &g->players[player].field[0];
    if (!to->stackSize)
        return;
    for (k = 0; k < f->attachedSize; k++)
        if (g->players[player].cards[f->attached[k]] == TCG_CARD_FIRE_ENERGY)
        {
            Announce(g, player, slot, TCG_POWER_R_MAGMA_POOL);
            to->attached[to->attachedSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k);
            TcgDuel_Event(g, TCG_EV_ATTACH, player, 0, 0, TCG_CARD_FIRE_ENERGY, 0);
            return;
        }
}
/* A Benched Pokemon of player's became the Active one during their own turn: Spikes (10 from each of the opponent's
 * Forretress whose power works; no Weakness or Resistance) */
void TcgPower_NewActive(struct TcgDuel *g, unsigned player)
{
    unsigned s;
    if (player > 1 || !g->players[player].field[0].stackSize)
        return;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (TcgPower_Working(g, player ^ 1, s) == TCG_POWER_R_SPIKES)
        {
            Announce(g, player ^ 1, s, TCG_POWER_R_SPIKES);
            PlainDamage(g, player, 0, 10);
        }
}
/* An opponent's attack damaged that Pokemon (before Knock Outs): Fortitude, Restless Sleep, Energy Drain, Shock
 * Blast */
void TcgPower_Damaged(struct TcgDuel *g, unsigned player, unsigned slot, unsigned damage)
{
    struct TcgFieldCard *f = &g->players[player].field[slot], *att = &g->players[g->current].field[0];
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    unsigned power = TcgPower_Working(g, player, slot), who;
    if (!c || !damage || player == g->current)
        return;
    if (f->reactKind && f->reactUntil >= g->turn && att->stackSize)
    {
        /* (an attack's reaction set on it: Fire Wall, Crosscounter, Counter, Secrete Poison, Endure, Armor Up) */
        unsigned kind = f->reactKind, hp = TcgDuel_MaxHp(g, player, slot);
        if (kind == TCG_REACT_FIRE_WALL)
        {
            int amount = 10;
            unsigned type = TcgDuel_Type(g, player, slot);
            if (TcgDuel_Weakness(g, g->current, 0) == type)
                amount *= 2;
            if (TcgDuel_Resistance(g, g->current, 0) == type)
                amount -= 30;
            if (amount > 0)
                PlainDamage(g, g->current, 0, (unsigned)amount);
        }
        else if ((kind == TCG_REACT_CROSSCOUNTER || kind == TCG_REACT_COUNTER) && TcgDuel_Coin(g, player))
            PlainDamage(g, g->current, 0, kind == TCG_REACT_CROSSCOUNTER ? damage * 2 : damage);
        else if (kind == TCG_REACT_SECRETE)
        {
            att->poison = 10;
            TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, att->condition, 0, att->poison);
            for (who = 1; who < TCG_FIELD_SIZE; who++)
                if (g->players[g->current].field[who].stackSize)
                    PlainDamage(g, g->current, who, 10);
        }
        else if ((kind == TCG_REACT_ENDURE || (kind == TCG_REACT_ARMOR_UP && f->damage >= hp && TcgDuel_Coin(g, player))) &&
                 f->damage >= hp)
        {
            f->damage = hp - 10u;
            TcgDuel_Log(g, "%s hangs on with 10 HP.", c->name);
        }
    }
    if (power == TCG_POWER_R_POLLEN_DEFENSE && !slot && att->stackSize && TcgDuel_Coin(g, player))
    {
        Announce(g, player, slot, power);
        att->condition = TCG_CONFUSED;
        TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, att->condition, 0, att->poison);
    }
    if (power == TCG_POWER_R_FLUFFY_WOOL && !slot && att->stackSize && TcgDuel_Coin(g, player))
    {
        Announce(g, player, slot, power);
        att->condition = TCG_ASLEEP;
        TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, att->condition, 0, att->poison);
    }
    if (power == TCG_POWER_R_BOLT && f->damage < TcgDuel_MaxHp(g, player, slot) && TcgDuel_Coin(g, player))
    {
        /* (Entei and its cards into the deck: unless Knocked Out) */
        Announce(g, player, slot, power);
        RpToDeck(g, player, slot);
        return;
    }
    if ((power == TCG_POWER_R_TIME_TRAVEL && c->kind == TCG_POKEMON && f->damage >= TcgDuel_MaxHp(g, player, slot) &&
         TcgDuel_Coin(g, player)) ||
        (power == TCG_POWER_R_SCRAM && c->kind == TCG_POKEMON && f->damage + 10u == TcgDuel_MaxHp(g, player, slot)))
    {
        Announce(g, player, slot, power);
        RpToDeck(g, player, slot);
        return;
    }
    if (power == TCG_POWER_R_FORTITUDE && c->kind == TCG_POKEMON && f->damage >= TcgDuel_MaxHp(g, player, slot) &&
        TcgDuel_Coin(g, player))
    {
        Announce(g, player, slot, power);
        f->damage = (uint16_t)(TcgDuel_MaxHp(g, player, slot) - 10u);
    }
    if (power == TCG_POWER_R_RESTLESS_SLEEP && f->condition == TCG_ASLEEP)
    {
        Announce(g, player, slot, power);
        PlainDamage(g, g->current, 0, 20);
    }
    if (power == TCG_POWER_R_ENERGY_DRAIN && TcgDuel_Coin(g, player))
    {
        int e = RpEnergyOn(g, g->current, 0, -1, 0);
        if (e >= 0)
        {
            unsigned i;
            Announce(g, player, slot, power);
            for (i = 0; i < att->attachedSize && att->attached[i] != (unsigned)e; i++)
                ;
            g->players[g->current].discard[g->players[g->current].discardSize++] =
                TcgPile_Remove(att->attached, &att->attachedSize, i);
        }
    }
    if (power == TCG_POWER_R_SHOCK_BLAST && !slot && !TcgDuel_Coin(g, player))
    {
        Announce(g, player, slot, power);
        for (who = 0; who < 2; who++)
            PlainDamage(g, who, 0, 20);
    }
}
/* Played from the hand (evolving the Active for some): Venom Spray, Softboiled, Berserk, Herbal Scent, Flame Vapor,
 * Energy Recharge */
static void RegionOnPlay(struct TcgDuel *g, unsigned player, unsigned slot, unsigned power)
{
    struct TcgPlayer *p = &g->players[player], *op = &g->players[player ^ 1];
    struct TcgFieldCard *f = &p->field[slot], *foe = &op->field[0];
    int evolvedActive = !slot && f->stackSize > 1 && g->fromHand;
    unsigned i, n, who, s;
    switch (power)
    {
    case TCG_POWER_R_TAG_TEAM:
        /* (played from the hand onto a Benched one) 3 counters off the Active, then it switches in */
        if (!slot || !g->fromHand)
            return;
        Announce(g, player, slot, power);
        if (p->field[0].stackSize && p->field[0].damage)
        {
            unsigned heal = p->field[0].damage < 30 ? p->field[0].damage : 30;
            p->field[0].damage -= heal;
            TcgDuel_Event(g, TCG_EV_HEAL, player, 0, heal, 0, 0);
        }
        if (!TcgDuel_Anchored(g, player))
        {
            struct TcgFieldCard temp = p->field[0];
            p->field[0] = *f;
            *f = temp;
            f->condition = TCG_HEALTHY;
            f->poison = f->burned = 0;
            TcgDuel_Event(g, TCG_EV_SWITCH, player, slot, 0, 0, 0);
        }
        break;
    case TCG_POWER_R_SURPRISE_BITE:
        /* (played from the hand) 20 to the one of theirs chosen (sPlay), else their Active */
        if (!g->fromHand)
            return;
        s = sPlay && sPlay->target < TCG_FIELD_SIZE && op->field[sPlay->target].stackSize ? sPlay->target : 0;
        if (!op->field[s].stackSize)
            return;
        Announce(g, player, slot, power);
        PlainDamage(g, player ^ 1, s, 20);
        break;
    case TCG_POWER_R_VENOM_SPRAY:
        if (!evolvedActive || !foe->stackSize)
            return;
        Announce(g, player, slot, power);
        foe->condition = TCG_PARALYZED;
        foe->poison = 10;
        TcgDuel_Event(g, TCG_EV_STATUS, player ^ 1, 0, TCG_PARALYZED, 0, 10);
        break;
    case TCG_POWER_R_FIRE_BOOST:
        /* (played from the hand) a coin: heads, up to 4 Fire Energy cards from the deck onto it */
        if (!g->fromHand)
            return;
        Announce(g, player, slot, power);
        if (TcgDuel_Coin(g, player))
            for (i = p->deckSize, n = 0; i > 0 && n < 4; i--)
                if (p->cards[p->deck[i - 1]] == TCG_CARD_FIRE_ENERGY)
                {
                    f->attached[f->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, i - 1);
                    TcgDuel_Event(g, TCG_EV_ATTACH, player, slot, 0, TCG_CARD_FIRE_ENERGY, 0);
                    n++;
                }
        TcgDuel_Shuffle(g, player);
        break;
    case TCG_POWER_R_CALL_THE_BOSS:
        /* (played from the hand) the Trainer named Giovanni from the deck into the hand */
        if (!g->fromHand)
            return;
        Announce(g, player, slot, power);
        for (i = p->deckSize; i > 0; i--)
            if (Tcg_Card(p->cards[p->deck[i - 1]])->kind == TCG_TRAINER &&
                !strcmp(Tcg_Card(p->cards[p->deck[i - 1]])->name, "Giovanni"))
            {
                TcgDuel_Log(g, "Shown: Giovanni.");
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, i - 1);
                break;
            }
        TcgDuel_Shuffle(g, player);
        break;
    case TCG_POWER_R_GIFT:
        /* (played from the hand) their choice of a Pokemon Tool, then yours */
        if (!g->fromHand)
            return;
        Announce(g, player, slot, power);
        TcgDuel_AskChoice(g, player ^ 1, TCG_CHOICE_TOOL, TcgDuel_Field(g, player, slot)->id, 0);
        break;
    case TCG_POWER_R_EXCHANGE:
        /* (played from the hand) their choice to redraw 4, then yours */
        if (!g->fromHand)
            return;
        Announce(g, player, slot, power);
        TcgDuel_AskChoice(g, player ^ 1, TCG_CHOICE_REDRAW4, TcgDuel_Field(g, player, slot)->id, 0);
        break;
    case TCG_POWER_R_ATTRACT_ENERGY:
    {
        /* (evolving from the hand) the basic Energy cards picked on your other Pokemon onto it */
        unsigned k, j, moved = 0;
        if (!g->fromHand || !sPlay)
            return;
        for (k = 0; k < sPlay->selectionCount; k++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                for (j = 0; s != slot && j < p->field[s].attachedSize; j++)
                    if (p->field[s].attached[j] == sPlay->selections[k] && p->cards[p->field[s].attached[j]] <= 6)
                    {
                        if (!moved++)
                            Announce(g, player, slot, power);
                        f->attached[f->attachedSize++] = TcgPile_Remove(p->field[s].attached, &p->field[s].attachedSize, j);
                        TcgDuel_Event(g, TCG_EV_ATTACH, player, slot, 0, p->cards[f->attached[f->attachedSize - 1]], 0);
                        s = TCG_FIELD_SIZE - 1;
                        break;
                    }
        break;
    }
    case TCG_POWER_R_VANISH:
    {
        /* (played from the hand) a coin: heads, the Unown chosen back to the hand, its attached cards discarded */
        unsigned t = sPlay ? sPlay->target : TCG_FIELD_SIZE;
        const struct TcgCard *c = t < TCG_FIELD_SIZE && t != slot ? TcgDuel_Field(g, player, t) : NULL;
        struct TcgFieldCard *u;
        if (!g->fromHand || !c || !strstr(c->name, "Unown"))
            return;
        Announce(g, player, slot, power);
        if (!TcgDuel_Coin(g, player))
            return;
        u = &p->field[t];
        while (u->attachedSize)
            p->discard[p->discardSize++] = u->attached[--u->attachedSize];
        while (u->stackSize)
            p->hand[p->handSize++] = u->stack[--u->stackSize];
        TcgDuel_Event(g, TCG_EV_TO_HAND, player, t, 0, 0, 0);
        memset(u, 0, sizeof(*u)); /* (the Active gone: a Benched one comes in, as after a Knock Out) */
        break;
    }
    case TCG_POWER_R_INCREASE:
        /* (played from the hand) a card with Unown in its name from the deck onto the Bench */
        if (!g->fromHand)
            return;
        for (s = 1; s < TCG_FIELD_SIZE && p->field[s].stackSize; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return;
        Announce(g, player, slot, power);
        for (i = p->deckSize; i > 0; i--)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->deck[i - 1]]);
            if (c && c->kind == TCG_POKEMON && !c->stage && strstr(c->name, "Unown"))
            {
                memset(&p->field[s], 0, sizeof(p->field[s]));
                p->field[s].stack[p->field[s].stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, i - 1);
                p->field[s].enteredTurn = g->turn;
                TcgDuel_Event(g, TCG_EV_PLAY, player, s, 0, c->id, 0);
                break;
            }
        }
        TcgDuel_Shuffle(g, player);
        break;
    case TCG_POWER_R_SOFTBOILED:
        if (!f->damage)
            return;
        Announce(g, player, slot, power);
        n = TcgDuel_Coin(g, player) ? 80 : 40;
        n = n < f->damage ? n : f->damage;
        f->damage -= n;
        TcgDuel_Event(g, TCG_EV_HEAL, player, slot, n, 0, 0);
        break;
    case TCG_POWER_R_BERSERK:
    {
        struct TcgPlayer *q = TcgDuel_Coin(g, player) ? op : p;
        Announce(g, player, slot, power);
        for (i = 0; i < 5 && q->deckSize; i++)
            q->discard[q->discardSize++] = q->deck[--q->deckSize];
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, q == p ? player : player ^ 1, 0, i, 0, 0);
        break;
    }
    case TCG_POWER_R_HERBAL_SCENT:
        Announce(g, player, slot, power);
        if (TcgDuel_Coin(g, player))
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                {
                    struct TcgFieldCard *h = &g->players[who].field[s];
                    if (h->stackSize && h->damage && TcgDuel_Type(g, who, s) == TCG_GRASS)
                    {
                        TcgDuel_Event(g, TCG_EV_HEAL, who, s, h->damage, 0, 0);
                        h->damage = 0;
                    }
                }
        break;
    case TCG_POWER_R_FLAME_VAPOR:
        if (!evolvedActive || !foe->stackSize)
            return;
        Announce(g, player, slot, power);
        n = 0;
        for (i = 0; i < 2; i++)
            n += TcgDuel_Coin(g, player) ? 1 : 0;
        while (n--)
        {
            int e = RpEnergyOn(g, player ^ 1, 0, -1, 0);
            unsigned k;
            if (e < 0)
                break;
            for (k = 0; k < foe->attachedSize && foe->attached[k] != (unsigned)e; k++)
                ;
            op->discard[op->discardSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, k);
        }
        break;
    case TCG_POWER_R_ENERGY_RECHARGE:
        if (!evolvedActive)
            return;
        Announce(g, player, slot, power);
        n = 0;
        for (i = 0; i < 3; i++)
            n += TcgDuel_Coin(g, player) ? 1 : 0;
        for (i = p->discardSize; i > 0 && n; i--)
            if (p->cards[p->discard[i - 1]] <= 6)
            {
                f->attached[f->attachedSize++] = TcgPile_Remove(p->discard, &p->discardSize, i - 1);
                TcgDuel_Event(g, TCG_EV_ATTACH, player, slot, 0, p->cards[f->attached[f->attachedSize - 1]], 0);
                n--;
            }
        break;
    }
}
/* As the Defending Pokemon retreats (Suction Cups on the other Active: its Energy discarded) */
static void RegionAfterRetreat(struct TcgDuel *g, unsigned player, unsigned benched)
{
    if (TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_R_CHASE && TcgDuel_Coin(g, player ^ 1))
    {
        /* (a damage counter on the one that retreated, Weakness and Resistance applied) */
        int amount = 10;
        unsigned type = TcgDuel_Type(g, player ^ 1, 0);
        Announce(g, player ^ 1, 0, TCG_POWER_R_CHASE);
        if (TcgDuel_Weakness(g, player, benched) == type)
            amount *= 2;
        if (TcgDuel_Resistance(g, player, benched) == type)
            amount -= 30;
        if (amount > 0)
            PlainDamage(g, player, benched, (unsigned)amount);
    }
    if (g->players[player].field[benched].pursuitUntil >= g->turn)
    {
        /* (Pursuit: 10 to it as it retreats, no Weakness or Resistance) */
        g->players[player].field[benched].pursuitUntil = 0;
        PlainDamage(g, player, benched, 10);
    }
    if (TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_R_SUCTION_CUPS)
    {
        struct TcgPlayer *p = &g->players[player];
        struct TcgFieldCard *f = &p->field[benched];
        unsigned i;
        Announce(g, player ^ 1, 0, TCG_POWER_R_SUCTION_CUPS);
        for (i = f->attachedSize; i > 0; i--)
            if (Tcg_Card(p->cards[f->attached[i - 1]])->kind == TCG_ENERGY)
                p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
    }
}
/* Between turns (TcgDuel's checkup): Rain Dish, a counter off it; Burning Aura (Active), a counter on each Active */
void TcgPower_BetweenTurns(struct TcgDuel *g)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            struct TcgFieldCard *f = &g->players[who].field[s];
            unsigned power = f->stackSize ? TcgPower_Working(g, who, s) : TCG_POWER_NONE;
            if (power == TCG_POWER_R_SCRAM && f->damage + 10u == TcgDuel_MaxHp(g, who, s))
            {
                Announce(g, who, s, power);
                RpToDeck(g, who, s);
                continue;
            }
            if (power == TCG_POWER_R_RAIN_DISH && f->damage >= 10)
            {
                Announce(g, who, s, power);
                f->damage -= 10;
                TcgDuel_Event(g, TCG_EV_HEAL, who, s, 10, 0, 0);
            }
            if (power == TCG_POWER_R_BURNING_AURA && !s)
            {
                unsigned w;
                Announce(g, who, s, power);
                for (w = 0; w < 2; w++)
                    if (g->players[w].field[0].stackSize)
                        PlainDamage(g, w, 0, 10);
            }
        }
}
/* The regional attack bonuses before Weakness and Resistance: Frenzy (Kyogre ex, Groudon ex, Rayquaza ex: 40 more while
 * the opponent has one of the other two, ex or not, in play); Final Blow (Megahorn's base damage 120 with 20 HP or less
 * left); an Unown's Hidden Power: [Anger] (10 more a counter on Unown [A]), [XXXXX] (a coin until tails, 10 more a
 * heads), [Perform] (Unown [P]'s own: what the last attack did to it) */
static int RegionAttackBonus(struct TcgDuel *g, const struct TcgAttack *attack)
{
    static const char *const weather[3] = {"Kyogre", "Groudon", "Rayquaza"};
    const struct TcgCard *it = TcgDuel_Field(g, g->current, 0);
    const struct TcgFieldCard *f = &g->players[g->current].field[0];
    unsigned power = TcgPower_Working(g, g->current, 0), k, s;
    int bonus = 0;
    if (!it)
        return 0;
    if (power == TCG_POWER_R_FINAL_BLOW && attack && !strcmp(attack->name, "Megahorn") && TcgDuel_MaxHp(g, g->current, 0) <= f->damage + 20u)
    {
        Announce(g, g->current, 0, power);
        bonus += 120 - (int)attack->damage;
    }
    if (attack && !strcmp(attack->name, "Hidden Power") && strstr(it->name, "Unown"))
    {
        int anger = 0, x = 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            unsigned p2 = TcgPower_Working(g, g->current, s);
            if (p2 == TCG_POWER_R_ANGER && !anger)
            {
                anger = 1;
                Announce(g, g->current, s, p2);
                bonus += (int)g->players[g->current].field[s].damage;
            }
            if (p2 == TCG_POWER_R_XXXXX && !x)
            {
                x = 1;
                Announce(g, g->current, s, p2);
                while (TcgDuel_Coin(g, g->current))
                    bonus += 10;
            }
        }
        if (power == TCG_POWER_R_PERFORM && f->mirrorDamage)
        {
            Announce(g, g->current, 0, power);
            bonus += (int)f->mirrorDamage;
        }
    }
    if (power != TCG_POWER_R_FRENZY_EX)
        return bonus;
    for (k = 0; k < 3; k++)
    {
        size_t n = strlen(weather[k]);
        if (!strncmp(it->name, weather[k], n))
            continue;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, s);
            if (c && !strncmp(c->name, weather[k], n) && (!c->name[n] || !strcmp(c->name + n, " ex")))
            {
                Announce(g, g->current, 0, TCG_POWER_R_FRENZY_EX);
                return bonus + 40;
            }
        }
    }
    return bonus;
}
/* Can't that Pokemon be evolved from the hand now: Primal Aura (either Active Kabutops: no Benched one), Primal Stare
 * (their Active Omastar: not your Active one) */
int TcgPower_EvolveBlocked(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    if (slot && (TcgPower_Working(g, 0, 0) == TCG_POWER_R_PRIMAL_AURA || TcgPower_Working(g, 1, 0) == TCG_POWER_R_PRIMAL_AURA))
        return 1;
    return !slot && TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_R_PRIMAL_STARE;
}
/* A Basic put onto the Bench from the hand: Hot Plate (either Active Dark Magcargo), 10 to it */
void TcgPower_BenchedFromHand(struct TcgDuel *g, unsigned player, unsigned slot)
{
    unsigned who;
    /* Rocket's Minefield Gym: its player's coin, tails 2 damage counters on it */
    if (slot && TcgDuel_Stadium(g, NULL) == TCG_CARD_GYM2_119 && !TcgDuel_Coin(g, player))
        PlainDamage(g, player, slot, 20);
    for (who = 0; who < 2; who++)
        if (slot && TcgPower_Working(g, who, 0) == TCG_POWER_R_HOT_PLATE)
        {
            Announce(g, who, 0, TCG_POWER_R_HOT_PLATE);
            PlainDamage(g, player, slot, 10);
            break;
        }
}
/* As the attack begins: Rebellion (Misty's Gyarados, while not Asleep or Paralyzed): 2 coins, both tails and the attack
 * does nothing, it and its cards into the deck. 1: the attack is over */
int TcgPower_BeforeAttack(struct TcgDuel *g)
{
    const struct TcgFieldCard *f = &g->players[g->current].field[0];
    if (TcgPower_Id(TcgDuel_Field(g, g->current, 0)) != TCG_POWER_R_REBELLION || f->condition == TCG_ASLEEP ||
        f->condition == TCG_PARALYZED || ToxicGas(g))
        return 0;
    Announce(g, g->current, 0, TCG_POWER_R_REBELLION);
    if (TcgDuel_Coin(g, g->current) | TcgDuel_Coin(g, g->current))
        return 0;
    RpToDeck(g, g->current, 0);
    return 1;
}
