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
/* Why a regional used power can't be used now (NULL: it can) */
static const char *RegionBlock(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned power)
{
    const struct TcgPlayer *p = &g->players[player], *op = &g->players[player ^ 1];
    unsigned flags = TcgPower_Flags(power), s, i;
    if ((flags & 2) && slot)
        return "It works only while it's your Active Pokemon.";
    if ((flags & 4) && !slot)
        return "It works only while it's on your Bench.";
    switch (power)
    {
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
        amount -= 10;
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
/* An opponent's attack damaged that Pokemon (before Knock Outs): Fortitude, Restless Sleep, Energy Drain, Shock
 * Blast */
void TcgPower_Damaged(struct TcgDuel *g, unsigned player, unsigned slot, unsigned damage)
{
    struct TcgFieldCard *f = &g->players[player].field[slot], *att = &g->players[g->current].field[0];
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    unsigned power = TcgPower_Working(g, player, slot), who;
    if (!c || !damage || player == g->current)
        return;
    if (power == TCG_POWER_R_FORTITUDE && c->kind == TCG_POKEMON && f->damage >= c->hp && TcgDuel_Coin(g, player))
    {
        Announce(g, player, slot, power);
        f->damage = c->hp - 10u;
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
    case TCG_POWER_R_VENOM_SPRAY:
        if (!evolvedActive || !foe->stackSize)
            return;
        Announce(g, player, slot, power);
        foe->condition = TCG_PARALYZED;
        foe->poison = 10;
        TcgDuel_Event(g, TCG_EV_STATUS, player ^ 1, 0, TCG_PARALYZED, 0, 10);
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
