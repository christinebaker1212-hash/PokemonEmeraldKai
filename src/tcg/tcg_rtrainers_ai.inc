/* The regional sets' Trainers: when the AI plays each and what it chooses (included by tcg_ai.c; the rules are
 * tcg_rtrainers.inc's, the steps tcg_rtrainers_flow.inc's, selections in the same order). Only what both players can
 * see is weighed, as for the Game Boy games' Trainers. */
static int AiNamed(const struct TcgCard *c, const char *word) { return c && strstr(c->name, word) != NULL; }
static int AiBasic(const struct TcgCard *c) { return c->kind == TCG_POKEMON && !c->stage; }
static int AiEvolution(const struct TcgCard *c) { return c->kind == TCG_POKEMON && c->stage; }
static int AiPokemonCard(const struct TcgCard *c) { return c->kind == TCG_POKEMON; }
static int AiTrainerCard(const struct TcgCard *c) { return c->kind == TCG_TRAINER; }
static int AiEnergyCard(const struct TcgCard *c) { return c->kind == TCG_ENERGY; }
static int AiBasicEnergy(const struct TcgCard *c) { return c->kind == TCG_ENERGY && c->id >= 1 && c->id <= 6; }
static int AiWater(const struct TcgCard *c) { return c->id == TCG_CARD_WATER_ENERGY; }
static int AiFire(const struct TcgCard *c) { return c->id == TCG_CARD_FIRE_ENERGY; }
static int AiBasicNotBaby(const struct TcgCard *c) { return AiBasic(c) && !(c->flags & TCG_CF_BABY); }
static int AiErika(const struct TcgCard *c) { return c->kind == TCG_POKEMON && AiNamed(c, "Erika"); }
static int AiBrock(const struct TcgCard *c) { return c->kind == TCG_POKEMON && AiNamed(c, "Brock"); }
static int AiUnown(const struct TcgCard *c) { return c->kind == TCG_POKEMON && AiNamed(c, "Unown"); }
static int AiGarbage(const struct TcgCard *c) { return c->kind == TCG_POKEMON || AiBasicEnergy(c); }
static int AiTmTool(const struct TcgCard *c) { return c->kind == TCG_TRAINER && (c->flags & (TCG_CF_TM | TCG_CF_TOOL)); }
static int AiSupStadium(const struct TcgCard *c)
{
    return c->kind == TCG_TRAINER && (c->flags & (TCG_CF_SUPPORTER | TCG_CF_STADIUM));
}
static int AiBall(const struct TcgCard *c) { return c->kind == TCG_TRAINER && strstr(c->name, "Ball"); }
static int AiAnyCard(const struct TcgCard *c) { return c != NULL; }
/* Up to max cards of a pile that pass ok into the selections: the best (best != 0) or the least (Score) first, never
 * `skip` (the card being played); how many */
static unsigned AiPick(const struct TcgPlayer *p, const uint8_t *pile, unsigned count, int (*ok)(const struct TcgCard *),
                       unsigned max, int best, unsigned skip, struct TcgAction *a)
{
    uint8_t used[TCG_DECK_SIZE] = {0};
    unsigned n = 0, i;
    for (i = 0; i < a->selectionCount; i++)
        used[a->selections[i]] = 1;
    while (n < max)
    {
        int pickAt = -1;
        unsigned pickScore = 0;
        for (i = 0; i < count; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[pile[i]]);
            unsigned sc;
            if (pile[i] == skip || used[pile[i]] || !c || !ok(c))
                continue;
            sc = Score(c);
            if (pickAt < 0 || (best ? sc > pickScore : sc < pickScore))
                pickAt = (int)i, pickScore = sc;
        }
        if (pickAt < 0)
            break;
        used[pile[pickAt]] = 1;
        a->selections[a->selectionCount++] = pile[pickAt];
        n++;
    }
    return n;
}
static unsigned AiDamagedMost(const struct TcgDuel *g, unsigned who, int (*ok)(const struct TcgDuel *, unsigned, unsigned))
{
    unsigned s, best = TCG_NONE;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (g->players[who].field[s].stackSize && g->players[who].field[s].damage && (!ok || ok(g, who, s)) &&
            (best == TCG_NONE || g->players[who].field[s].damage > g->players[who].field[best].damage))
            best = s;
    return best;
}
static int AiHasBasicEnergy(const struct TcgDuel *g, unsigned who, unsigned s)
{
    const struct TcgPlayer *p = &g->players[who];
    unsigned i;
    for (i = 0; i < p->field[s].attachedSize; i++)
        if (p->cards[p->field[s].attached[i]] >= 1 && p->cards[p->field[s].attached[i]] <= 6)
            return 1;
    return 0;
}
static unsigned AiBenchRoom(const struct TcgPlayer *p)
{
    unsigned s, n = 0;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        n += !p->field[s].stackSize;
    return n;
}
static unsigned AiNamedSlot(const struct TcgDuel *g, unsigned who, const char *word, unsigned skip)
{
    unsigned s;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (s != skip && AiNamed(TcgDuel_Field(g, who, s), word))
            return s;
    return TCG_NONE;
}
static int AiCanAttack(const struct TcgDuel *g)
{
    unsigned k;
    for (k = 0; k < TCG_ATTACKS; k++)
        if (TcgDuel_CanAttack(g, g->current, k) && TcgDuel_CanAttackNow(g, g->current, k))
            return 1;
    return 0;
}

static int RegionTrainerAi(const struct TcgDuel *g, unsigned hand, struct TcgAction *a, unsigned id)
{
    const struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    const struct TcgCard *card = Tcg_Card(p->cards[p->hand[hand]]);
    unsigned played = p->hand[hand], me = g->current, s, i;
    const struct TcgCard *active = TcgDuel_Field(g, me, 0), *foe = TcgDuel_Field(g, me ^ 1, 0);
    /* Pokemon Tools: on the Active (Balloon Berry only where retreating costs) */
    if (card->flags & TCG_CF_TOOL)
    {
        if (!active || TcgDuel_Tool(g, me, 0, NULL) || id == TCG_CARD_ECARD3_122 ||
            (id == TCG_CARD_NEO3_60 && TcgDuel_RetreatCost(g, me) < 2) ||
            ((id == TCG_CARD_NEO1_99 || id == TCG_CARD_NEO1_93 || id == TCG_CARD_ECARD2_125) && !p->field[0].damage &&
             !op->field[0].attachedSize))
            return 0;
        a->slot = 0;
        return Legal(g, a);
    }
    /* Stadiums: when none is in play (and this one does something for this side) */
    if (card->flags & TCG_CF_STADIUM)
    {
        if (TcgDuel_Stadium(g, NULL))
            return 0;
        switch (id)
        {
        case TCG_CARD_GYM1_108:
            if (AiNamedSlot(g, me, "Misty", TCG_NONE) == TCG_NONE)
                return 0;
            break;
        case TCG_CARD_GYM1_115:
            if (AiNamedSlot(g, me, "Brock", TCG_NONE) == TCG_NONE)
                return 0;
            break;
        case TCG_CARD_GYM2_123:
            if (AiNamedSlot(g, me, "Giovanni", TCG_NONE) == TCG_NONE)
                return 0;
            break;
        case TCG_CARD_GYM2_113:
            if (AiNamedSlot(g, me, "Blaine", TCG_NONE) == TCG_NONE)
                return 0;
            break;
        case TCG_CARD_NEO1_97:
            if (!foe || TcgDuel_Type(g, me ^ 1, 0) != TCG_COLORLESS || TcgDuel_Type(g, me, 0) == TCG_COLORLESS)
                return 0;
            break;
        case TCG_CARD_ECARD2_131:
            if (AiBenchRoom(p) == TCG_FIELD_SIZE - 1)
                return 0;
            break;
        case TCG_CARD_GYM1_104:
        case TCG_CARD_NEO4_92:
        case TCG_CARD_ECARD3_132:
            /* (it weighs on both: when their Active is the one stuck) */
            if (!foe || foe->retreat < (active ? active->retreat : 0) + 1u)
                return 0;
            break;
        }
        return Legal(g, a);
    }
    switch (id)
    {
    case TCG_CARD_GYM1_15: /* Brock */
    case TCG_CARD_GYM2_103: /* Erika's Kindness: when ours are hurt more */
    {
        unsigned mine = 0, theirs = 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            mine += p->field[s].stackSize && p->field[s].damage;
            theirs += op->field[s].stackSize && op->field[s].damage;
        }
        if (!mine || (id == TCG_CARD_GYM2_103 && theirs >= mine))
            return 0;
        break;
    }
    case TCG_CARD_GYM1_16: /* Erika: draw 3 */
        if (p->deckSize < 6)
            return 0;
        a->target = 3;
        break;
    case TCG_CARD_GYM1_17: /* Lt. Surge: when the Active is in trouble */
        if (!active || p->field[0].damage * 2 < active->hp || !AiBenchRoom(p) ||
            !AiPick(p, p->hand, p->handSize, AiBasic, 1, 1, played, a))
            return 0;
        break;
    case TCG_CARD_GYM1_18: /* Misty: for a Misty Pokemon that can attack */
        if (!AiNamed(active, "Misty") || !AiCanAttack(g) || !SelectCost(p, played, a->selections, 2))
            return 0;
        a->selectionCount = 2;
        break;
    case TCG_CARD_GYM2_19: /* Koga */
        if (!AiNamed(active, "Koga") || !AiCanAttack(g) || !foe || op->field[0].poison)
            return 0;
        break;
    case TCG_CARD_GYM1_105: /* Blaine's Last Resort */
    case TCG_CARD_NEO4_105: /* Mail from Bill */
    case TCG_CARD_ECARD1_142: /* Mary's Impulse */
        break;
    case TCG_CARD_GYM1_106: /* Brock's Training Method */
        if (!AiPick(p, p->deck, p->deckSize, AiBrock, 1, 1, played, a))
            return 0;
        break;
    case TCG_CARD_GYM1_109: /* Erika's Maids */
        if (!SelectCost(p, played, a->selections, 2))
            return 0;
        a->selectionCount = 2;
        if (!AiPick(p, p->deck, p->deckSize, AiErika, 2, 1, played, a))
            return 0;
        break;
    case TCG_CARD_GYM1_110: /* Erika's Perfume: their Basics out where ours can reach them (when their Bench has room) */
        if (!AiBenchRoom(op))
            return 0;
        AiPick(op, op->hand, op->handSize, AiBasic, AiBenchRoom(op) < 2 ? AiBenchRoom(op) : 2, 0, TCG_NONE, a);
        break;
    case TCG_CARD_GYM1_111: /* Good Manners */
    case TCG_CARD_ECARD1_148: /* Professor Elm's Training Method */
        if (!AiPick(p, p->deck, p->deckSize, id == TCG_CARD_GYM1_111 ? AiBasic : AiEvolution, 1, 1, played, a))
            return 0;
        break;
    case TCG_CARD_GYM1_114: /* Misty's Wrath (the top 7 aren't seen: the first two) */
    {
        unsigned top = p->deckSize < 7 ? p->deckSize : 7;
        if (top < 2 || p->deckSize < 12)
            return 0;
        a->selections[a->selectionCount++] = p->deck[p->deckSize - 1];
        a->selections[a->selectionCount++] = p->deck[p->deckSize - 2];
        break;
    }
    case TCG_CARD_GYM1_118: /* Secret Mission: the least useful Trainers back for new cards */
    case TCG_CARD_GYM1_121: /* Blaine's Gamble */
        if (p->deckSize < 10)
            return 0;
        AiPick(p, p->hand, p->handSize, AiTrainerCard, 2, 0, played, a);
        if (!a->selectionCount)
            return 0;
        break;
    case TCG_CARD_GYM1_123: /* Misty's Duel, Sabrina's Gaze, Desert Shaman: with a small hand */
    case TCG_CARD_GYM1_125:
    case TCG_CARD_ECARD3_123:
        if (p->handSize > 2 || op->handSize < 3)
            return 0;
        break;
    case TCG_CARD_GYM2_17: /* Blaine */
        s = AiNamedSlot(g, me, "Blaine", TCG_NONE);
        if (s == TCG_NONE || p->energyPlayed || AiPick(p, p->hand, p->handSize, AiFire, 2, 1, played, a) != 2)
            return 0;
        a->slot = (uint8_t)s;
        break;
    case TCG_CARD_GYM2_105: /* Giovanni's Last Resort: a big heal with little in the hand */
        s = AiDamagedMost(g, me, NULL);
        if (s == TCG_NONE || !AiNamed(TcgDuel_Field(g, me, s), "Giovanni") || p->field[s].damage < 50 || p->handSize > 2)
            return 0;
        a->slot = (uint8_t)s;
        break;
    case TCG_CARD_GYM2_118: /* Misty's Tears */
        if (!SelectCost(p, played, a->selections, 1))
            return 0;
        a->selectionCount = 1;
        if (!AiPick(p, p->deck, p->deckSize, AiWater, 2, 1, played, a))
            return 0;
        break;
    case TCG_CARD_GYM2_124: /* Fervor: a Fire deck's */
    case TCG_CARD_NEO1_83:  /* Arcade Game */
    case TCG_CARD_ECARD3_124: /* Fast Ball */
    case TCG_CARD_ECARD2_133: /* Seer */
        if (p->deckSize < 10)
            return 0;
        break;
    case TCG_CARD_NEO1_85: /* Energy Charge, Power Charge: when the discard pile holds Energy */
    case TCG_CARD_ECARD1_147:
        if (!AiPick(p, p->discard, p->discardSize, AiEnergyCard, 2, 1, played, a))
            return 0;
        break;
    case TCG_CARD_NEO1_96: /* Professor Elm, Professor Oak's Research, Copycat: with a small hand */
    case TCG_CARD_ECARD1_149:
        if (p->handSize > 3 || p->deckSize < 10)
            return 0;
        break;
    case TCG_CARD_ECARD1_138:
        if (op->handSize < p->handSize + 3 || p->deckSize < op->handSize + 4)
            return 0;
        break;
    case TCG_CARD_NEO1_101: /* Moo-Moo Milk */
    case TCG_CARD_ECARD1_145: /* Pokemon Nurse: a badly hurt Pokemon without much Energy */
        s = AiDamagedMost(g, me, NULL);
        if (s == TCG_NONE || p->field[s].damage < 30 ||
            (id == TCG_CARD_ECARD1_145 && TcgDuel_Energy(g, me, s, TCG_NONE) > 1))
            return 0;
        a->slot = (uint8_t)s;
        break;
    case TCG_CARD_ECARD3_127: /* Hyper Potion: one basic Energy for 3 counters */
        s = AiDamagedMost(g, me, AiHasBasicEnergy);
        if (s == TCG_NONE || p->field[s].damage < 30)
            return 0;
        a->slot = (uint8_t)s;
        AiPick(p, p->field[s].attached, p->field[s].attachedSize, AiBasicEnergy, 1, 0, TCG_NONE, a);
        break;
    case TCG_CARD_NEO4_104: /* Heal Powder */
        if (!p->field[0].condition && !p->field[0].poison && p->field[0].damage < 20)
            return 0;
        break;
    case TCG_CARD_NEO1_88: /* PokeGear: the top card's a Trainer (none seen: it looks at what's on top) */
        return 0;
    case TCG_CARD_NEO1_103: /* Super Rod, Old Rod */
        AiPick(p, p->discard, p->discardSize, AiEvolution, 1, 1, played, a);
        AiPick(p, p->discard, p->discardSize, AiBasic, 1, 1, played, a);
        if (!a->selectionCount)
            return 0;
        break;
    case TCG_CARD_NEO3_64:
        AiPick(p, p->discard, p->discardSize, AiPokemonCard, 1, 1, played, a);
        AiPick(p, p->discard, p->discardSize, AiTrainerCard, 1, 1, played, a);
        if (!a->selectionCount)
            return 0;
        break;
    case TCG_CARD_NEO2_74: /* Ruin Wall */
        if (!AiBenchRoom(p) || !AiPick(p, p->deck, p->deckSize, AiUnown, 1, 1, played, a))
            return 0;
        break;
    case TCG_CARD_NEO2_75: /* Energy Ark, Dual Ball */
    case TCG_CARD_ECARD1_139:
        if (!AiPick(p, p->deck, p->deckSize, id == TCG_CARD_NEO2_75 ? AiBasicEnergy : AiBasicNotBaby, 2, 1, played, a))
            return 0;
        break;
    case TCG_CARD_NEO4_98: /* Energy Amplifier */
        if (!AiPick(p, p->hand, p->handSize, AiEnergyCard, 1, 0, played, a) ||
            !AiPick(p, p->deck, p->deckSize, AiBasicEnergy, 3, 1, played, a))
            return 0;
        break;
    case TCG_CARD_NEO4_101: /* Magnifier: against a Defending Pokemon that resists */
        if (!active || !foe || !AiCanAttack(g) || TcgDuel_Resistance(g, me ^ 1, 0) != TcgDuel_Type(g, me, 0))
            return 0;
        a->slot = 0;
        break;
    case TCG_CARD_ECARD2_141: /* Weakness Guard: when their Active hits our Weakness */
        if (!active || !foe || TcgDuel_Weakness(g, me, 0) != TcgDuel_Type(g, me ^ 1, 0))
            return 0;
        a->slot = 0;
        break;
    case TCG_CARD_NEO4_103: /* Team Rocket's Evil Deeds: their best card */
        if (op->handSize < 3)
            return 0;
        a->selections[a->selectionCount++] = op->hand[0];
        break;
    case TCG_CARD_ECARD1_137: /* Bill's Maintenance */
        if (p->handSize < 2 || p->deckSize < 8)
            return 0;
        AiPick(p, p->hand, p->handSize, AiAnyCard, 1, 0, played, a);
        break;
    case TCG_CARD_ECARD1_140: /* Energy Removal 2: the Defending Pokemon's */
        if (!foe || !TcgDuel_Energy(g, me ^ 1, 0, TCG_NONE))
            return 0;
        a->target = 0;
        AiPick(op, op->field[0].attached, op->field[0].attachedSize, AiEnergyCard, 1, 1, TCG_NONE, a);
        break;
    case TCG_CARD_ECARD2_134: /* Super Energy Removal 2: when they have far more on their Active */
        if (TcgDuel_Energy(g, me ^ 1, 0, TCG_NONE) < TcgDuel_Energy(g, me, 0, TCG_NONE) + 2)
            return 0;
        break;
    case TCG_CARD_ECARD1_141: /* Energy Restore, Lure Ball, Fisherman */
    case TCG_CARD_ECARD3_128:
    case TCG_CARD_ECARD3_125:
    {
        int (*ok)(const struct TcgCard *) = id == TCG_CARD_ECARD3_128 ? AiEvolution : AiBasicEnergy;
        if (!AiPick(p, p->discard, p->discardSize, ok, id == TCG_CARD_ECARD3_125 ? 4 : 3, 1, played, a))
            return 0;
        break;
    }
    case TCG_CARD_ECARD1_146: /* Pokemon Reversal: their weakest Benched Pokemon */
    {
        unsigned best = TCG_NONE, left = 0;
        for (s = 1; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, me ^ 1, s);
            unsigned l;
            if (!c || c->kind != TCG_POKEMON)
                continue;
            l = c->hp > op->field[s].damage ? c->hp - op->field[s].damage : 0;
            if (best == TCG_NONE || l < left)
                best = s, left = l;
        }
        if (best == TCG_NONE || !AiCanAttack(g) || left > 40)
            return 0;
        a->target = (uint8_t)best;
        break;
    }
    case TCG_CARD_ECARD2_120: /* Energy Switch: from a Benched Pokemon to the Active that needs it */
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (p->field[s].stackSize && AiHasBasicEnergy(g, me, s))
                break;
        if (s == TCG_FIELD_SIZE || AiCanAttack(g) || !active)
            return 0;
        a->slot = (uint8_t)s;
        a->target = 0;
        AiPick(p, p->field[s].attached, p->field[s].attachedSize, AiBasicEnergy, 1, 1, TCG_NONE, a);
        break;
    case TCG_CARD_ECARD2_126: /* Juggler: spare basic Energy for cards */
        if (AiPick(p, p->hand, p->handSize, AiBasicEnergy, 2, 0, played, a) < 1 || p->deckSize < 10)
            return 0;
        break;
    case TCG_CARD_ECARD2_130: /* Pokemon Fan Club */
    {
        unsigned room = AiBenchRoom(p);
        if (!room || !AiPick(p, p->deck, p->deckSize, AiBasic, room < 2 ? room : 2, 1, played, a))
            return 0;
        break;
    }
    case TCG_CARD_ECARD2_136: /* Town Volunteers: when the deck runs low */
        if (p->deckSize > 15 || !AiPick(p, p->discard, p->discardSize, AiGarbage, 5, 1, played, a))
            return 0;
        break;
    case TCG_CARD_ECARD2_137: /* Traveling Salesman, Relic Hunter, Apricorn Maker */
    case TCG_CARD_ECARD3_120:
    case TCG_CARD_ECARD3_121:
    {
        int (*ok)(const struct TcgCard *) = id == TCG_CARD_ECARD2_137 ? AiTmTool : id == TCG_CARD_ECARD3_120 ? AiSupStadium : AiBall;
        if (!AiPick(p, p->deck, p->deckSize, ok, 2, 1, played, a))
            return 0;
        break;
    }
    case TCG_CARD_ECARD3_138: /* Oracle: the two best cards on top */
        if (AiPick(p, p->deck, p->deckSize, AiPokemonCard, 2, 1, played, a) < 2)
            return 0;
        break;
    case TCG_CARD_NP_27: /* Tropical Tidal Wave: when they have Trainers in play and we have none */
    case TCG_CARD_NP_36:
    {
        unsigned theirs = op->stadium != 0, ours = p->stadium != 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            for (i = 0; i < op->field[s].attachedSize; i++)
                theirs += Tcg_Card(op->cards[op->field[s].attached[i]])->kind == TCG_TRAINER;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            for (i = 0; i < p->field[s].attachedSize; i++)
                ours += Tcg_Card(p->cards[p->field[s].attached[i]])->kind == TCG_TRAINER;
        if (theirs < 2 || ours)
            return 0;
        break;
    }
    default:
        /* (Minion of Team Rocket, Thought Wave Machine, Tropical Wind, Energy Flow, Sabrina, Trash Exchange,
         * Rocket's Secret Experiment, Hyper Devolution Spray: the AI keeps them) */
        return 0;
    }
    return Legal(g, a);
}

/* The regional sets' used powers: when the AI uses one, what it chooses (a->type / slot set; target, selections
 * here). Ones that cost it something or reshuffle its board are kept for people. */
static int RegionPowerAi(const struct TcgDuel *g, struct TcgAction *a, unsigned slot, unsigned power)
{
    const struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    const struct TcgFieldCard *foe = &op->field[0], *mine = &p->field[0];
    unsigned me = g->current, s, best = TCG_NONE, left = 0;
    switch (power)
    {
    case TCG_POWER_R_POISON_POLLEN:
        return foe->stackSize && !foe->poison;
    case TCG_POWER_R_SLEEP_PENDULUM:
        return foe->stackSize && foe->condition != TCG_ASLEEP;
    case TCG_POWER_R_MIRACULOUS_POWDER:
        return mine->condition || mine->poison || mine->burned;
    case TCG_POWER_R_SOOTHING_AROMA:
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (p->field[s].stackSize && p->field[s].damage)
                return 1;
        return 0;
    case TCG_POWER_R_NATURAL_HEALING:
        return p->field[slot].damage != 0;
    case TCG_POWER_R_EARTH_RAGE:
        return 1;
    case TCG_POWER_R_FRAGRANCE_TRAP:
    case TCG_POWER_R_FRAGRANCE_TRAP_GYM:
        /* their weakest Benched Pokemon, if weaker than the Defending one */
        for (s = 1; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, me ^ 1, s);
            unsigned l;
            if (!c || c->kind != TCG_POKEMON)
                continue;
            l = c->hp > op->field[s].damage ? c->hp - op->field[s].damage : 0;
            if (best == TCG_NONE || l < left)
                best = s, left = l;
        }
        {
            const struct TcgCard *d = TcgDuel_Field(g, me ^ 1, 0);
            if (best == TCG_NONE || !d || left + 20 > (d->hp > foe->damage ? (unsigned)(d->hp - foe->damage) : 0u))
                return 0;
        }
        a->target = (uint8_t)best;
        return 1;
    case TCG_POWER_R_JET_STREAM:
        if (!foe->attachedSize || TcgDuel_Energy(g, me, slot, TCG_NONE) >= TcgDuel_Energy(g, me ^ 1, 0, TCG_NONE))
            return 0;
        AiPick(op, foe->attachedSize ? foe->attached : NULL, foe->attachedSize, AiEnergyCard, 1, 1, TCG_NONE, a);
        return 1;
    case TCG_POWER_R_SUPER_DYNAMO:
        a->target = 0;
        return 1;
    case TCG_POWER_R_FLOWER_SUPPLEMENT:
        for (s = 1; s < TCG_FIELD_SIZE && !p->field[s].stackSize; s++)
            ;
        if (s == TCG_FIELD_SIZE || !AiPick(p, p->hand, p->handSize, AiBasicEnergy, 1, 0, TCG_NONE, a))
            return 0;
        a->target = (uint8_t)s;
        return 1;
    case TCG_POWER_R_CARRY_OFF:
        return op->handSize && AiPick(op, op->hand, op->handSize, AiPokemonCard, 1, 1, TCG_NONE, a);
    case TCG_POWER_R_GLARING_GAZE:
        return op->handSize && AiPick(op, op->hand, op->handSize, AiTrainerCard, 1, 1, TCG_NONE, a);
    }
    return 0;
}
