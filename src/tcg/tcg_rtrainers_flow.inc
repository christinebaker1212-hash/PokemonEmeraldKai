/* The regional sets' Trainers (docs/TCG_REGIONS.md): what each asks for (included by tcg_flow.c). The rules are
 * tcg_rtrainers.inc's, in the same order; selections go in step order. TCG_MIN_ALL: as many as there are, up to
 * max. A Pokemon Tool with nothing of its own asks only where it goes. */
static const struct TcgStep *RegionTrainerSteps(unsigned id, int *count)
{
    switch (id)
    {
    case TCG_CARD_GYM1_16:
        STEPS({TCG_ST_CHOICE, 0, 0, 0, 0, "Draw how many cards?"});
    case TCG_CARD_GYM1_17:
        STEPS({TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_HAND, 1, 1, "Which Basic Pokemon becomes your Active Pokemon?"});
    case TCG_CARD_GYM1_18:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 2, 2, "Discard which 2 other cards?"});
    case TCG_CARD_GYM1_106:
        STEPS({TCG_ST_PICK, TCG_F_OWNER_CARD + TCG_OWNER_BROCK, TCG_SRC_DECK, 0, 1, "Take which Brock Pokemon?"});
    case TCG_CARD_GYM1_109:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 2, 2, "Discard which 2 other cards?"},
              {TCG_ST_PICK, TCG_F_OWNER_CARD + TCG_OWNER_ERIKA, TCG_SRC_DECK, 0, 2, "Take up to 2 Erika Pokemon."});
    case TCG_CARD_GYM1_110:
        STEPS({TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_OPP_HAND, 0, TCG_FIELD_SIZE - 1,
               "Put which of their Basic Pokemon onto their Bench?"});
    case TCG_CARD_GYM1_111:
        STEPS({TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_DECK, 0, 1, "Take which Basic Pokemon?"});
    case TCG_CARD_GYM1_113:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_BENCH, 0, 0, 0, "If both coins are heads: return which of theirs?"});
    case TCG_CARD_GYM1_114:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_TOP7, TCG_MIN_ALL, 2, "Keep which 2? (the rest are discarded)"});
    case TCG_CARD_GYM1_118:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 0, TCG_DECK_SIZE, "Discard which cards? (draw as many)"});
    case TCG_CARD_GYM1_121:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 0, TCG_DECK_SIZE, "Discard which cards? (heads: draw twice as many)"});
    case TCG_CARD_GYM1_122:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_FIELD_ATTACHED, 0, TCG_DECK_SIZE, "Return which Energy to your hand?"});
    case TCG_CARD_GYM2_17:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_BLAINE, 0, 0, 0, "Attach to which Blaine Pokemon?"},
              {TCG_ST_PICK, TCG_F_FIRE_ENERGY, TCG_SRC_HAND, 2, 2, "Attach which 2 Fire Energy?"});
    case TCG_CARD_GYM2_20:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_SABRINA, 0, 0, 0, "Move Energy from which Sabrina Pokemon?"},
              {TCG_ST_MY_SLOT, TCG_F_OWNER_OTHER + TCG_OWNER_SABRINA, 0, 0, 0, "Move it to which Sabrina Pokemon?"});
    case TCG_CARD_GYM2_105:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_GIOVANNI, 0, 0, 0, "Heal which Giovanni Pokemon?"});
    case TCG_CARD_GYM2_118:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 1, 1, "Discard which other card?"},
              {TCG_ST_PICK, TCG_F_WATER_ENERGY, TCG_SRC_DECK, 0, 2, "Take up to 2 Water Energy."});
    case TCG_CARD_GYM2_120:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_DECK, 0, 1, "If heads: take which card?"});
    case TCG_CARD_NEO1_85:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_DISCARD, 0, 2, "If heads: shuffle which Energy into your deck?"});
    case TCG_CARD_ECARD1_147:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_DISCARD, TCG_MIN_ALL, 2, "If heads: shuffle which 2 Energy into your deck?"});
    case TCG_CARD_NEO1_88:
        STEPS({TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_TOP7, 0, 1, "Take which Trainer?"});
    case TCG_CARD_NEO1_101:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED, 0, 0, 0, "Heal which Pokemon? (2 coins)"});
    case TCG_CARD_NEO1_103:
        STEPS({TCG_ST_PICK, TCG_F_EVOLUTION, TCG_SRC_MY_DISCARD, 0, 1, "Heads: which Evolution card?"},
              {TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_MY_DISCARD, 0, 1, "Tails: which Basic Pokemon?"});
    case TCG_CARD_NEO3_64:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON_CARD, TCG_SRC_MY_DISCARD, 0, 1, "Both heads: which Pokemon card?"},
              {TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_MY_DISCARD, 0, 1, "Both tails: which Trainer card?"});
    case TCG_CARD_NEO2_73:
        STEPS({TCG_ST_MY_SLOT, TCG_F_EVOLVED, 0, 0, 0, "Devolve which Pokemon?"});
    case TCG_CARD_NEO2_74:
        STEPS({TCG_ST_PICK, TCG_F_UNOWN, TCG_SRC_DECK, 0, 1, "Put which Unown onto your Bench?"});
    case TCG_CARD_NEO2_75:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_DECK, 0, 2, "Take which basic Energy? (one per heads)"});
    case TCG_CARD_ECARD1_139:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_NOT_BABY, TCG_SRC_DECK, 0, 2, "Take which Basic Pokemon? (one per heads)"});
    case TCG_CARD_NEO4_98:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_HAND, 1, 1, "Shuffle which Energy into your deck?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_DECK, 0, 3, "If heads: take up to 3 basic Energy."});
    case TCG_CARD_NEO4_101:
    case TCG_CARD_ECARD2_141:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OCCUPIED, 0, 0, 0, "Attach it to which Pokemon?"});
    case TCG_CARD_NEO4_103:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_OPP_HAND, 1, 1, "Shuffle which of their cards into their deck?"});
    case TCG_CARD_ECARD1_137:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, TCG_MIN_ALL, 1, "Shuffle which card into your deck?"});
    case TCG_CARD_ECARD1_140:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_HAS_ENERGY, 0, 0, 0, "Remove Energy from which Pokemon? (heads)"},
              {TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_OPP_ATTACHED, 1, 1, "Discard which Energy?"});
    case TCG_CARD_ECARD1_141:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, 0, 3, "Take which basic Energy? (one per heads)"});
    case TCG_CARD_ECARD3_128:
        STEPS({TCG_ST_PICK, TCG_F_EVOLUTION, TCG_SRC_MY_DISCARD, 0, 3, "Take which Evolution cards? (one per heads)"});
    case TCG_CARD_ECARD1_145:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED, 0, 0, 0, "Heal which Pokemon? (its Energy is discarded)"});
    case TCG_CARD_ECARD1_146:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_BENCH, 0, 0, 0, "If heads: bring out which of theirs?"});
    case TCG_CARD_ECARD1_148:
        STEPS({TCG_ST_PICK, TCG_F_EVOLUTION, TCG_SRC_DECK, 0, 1, "Take which Evolution card?"});
    case TCG_CARD_ECARD2_120:
        STEPS({TCG_ST_MY_SLOT, TCG_F_HAS_BASIC_ENERGY, 0, 0, 0, "Move a basic Energy from which Pokemon?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_ATTACHED, 1, 1, "Move which Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_OTHER_MINE, 0, 0, 0, "Move it to which Pokemon?"});
    case TCG_CARD_ECARD2_126:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_HAND, 0, 2, "Discard 1 (draw 3) or 2 (draw 5) basic Energy."});
    case TCG_CARD_ECARD2_130:
        STEPS({TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_DECK, 0, 2, "Put up to 2 Basic Pokemon onto your Bench."});
    case TCG_CARD_ECARD2_136:
        STEPS({TCG_ST_PICK, TCG_F_GARBAGE, TCG_SRC_MY_DISCARD, TCG_MIN_ALL, 5, "Shuffle which 5 into your deck?"});
    case TCG_CARD_ECARD2_137:
        STEPS({TCG_ST_PICK, TCG_F_TM_TOOL, TCG_SRC_DECK, 0, 2, "Take up to 2 Technical Machines / Pokemon Tools."});
    case TCG_CARD_ECARD3_120:
        STEPS({TCG_ST_PICK, TCG_F_SUPPORTER_STADIUM, TCG_SRC_DECK, 0, 2, "Take up to 2 Supporters / Stadiums."});
    case TCG_CARD_ECARD3_121:
        STEPS({TCG_ST_PICK, TCG_F_BALL, TCG_SRC_DECK, 0, 2, "Take up to 2 Trainers with Ball in their names."});
    case TCG_CARD_ECARD3_125:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, TCG_MIN_ALL, 4, "Take which 4 basic Energy?"});
    case TCG_CARD_ECARD3_127:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED_BASIC_ENERGY, 0, 0, 0, "Heal which Pokemon?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_ATTACHED, 1, 2, "Discard 1 (3 counters) or 2 (5 counters)."});
    case TCG_CARD_ECARD3_138:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_DECK, TCG_MIN_ALL, 2, "Put which 2 on top? (the first one on top)"});
    case TCG_CARD_NEO1_87:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND_TOP2, TCG_MIN_ALL, 2,
               "Draw 2, then shuffle which 2 into your deck?"});
    case TCG_CARD_NEO1_95:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_TOP5, 0, 0, "Choose the order: the first is drawn next."});
    case TCG_CARD_ECARD2_123:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_TOP7, TCG_MIN_ALL, 1, "Take which card? (the rest are shuffled in)"});
    case TCG_CARD_ECARD3_140:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_BOTTOM4, TCG_MIN_ALL, 2, "Take which 2? (the rest stay at the bottom)"});
    case TCG_CARD_ECARD3_126:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON_CARD, TCG_SRC_DECK, 0, 1, "Take which Pokemon card?"},
              {TCG_ST_OPP_SLOT, TCG_F_SAME_TYPE_PICK, 0, 0, 0, "Of the type of which of their Pokemon?"});
    case TCG_CARD_NEO3_62:
        STEPS({TCG_ST_PICK, TCG_F_LATER_STAGE, TCG_SRC_DECK, 0, 2, "Flip for which later-Stage cards? (heads: kept)"});
    case TCG_CARD_GYM2_126:
        STEPS({TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "Switch in which of your Benched Pokemon?"});
    case TCG_CARD_NEO1_102:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_NOT_BABY, TCG_SRC_DECK, 0, 1, "Put which Basic Pokemon onto your Bench?"});
    case TCG_CARD_GYM2_18:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_GIOVANNI, 0, 0, 0, "Which Giovanni Pokemon may evolve?"});
    }
    if (Tcg_Card(id) && (Tcg_Card(id)->flags & TCG_CF_TOOL))
        STEPS({TCG_ST_MY_SLOT, TCG_F_NO_TOOL, 0, 0, 0, "Attach it to which Pokemon?"});
    if (Tcg_Card(id) && (Tcg_Card(id)->flags & TCG_CF_TM))
        STEPS({TCG_ST_MY_SLOT, TCG_F_TM_FITS, 0, 0, 0, "Attach the Technical Machine to which Pokemon?"});
    *count = 0;
    return sNoSteps;
}
/* A regional Trainer's own condition to be played at all (beyond what its steps ask) */
static int RegionTrainerPlayable(const struct TcgDuel *g, unsigned me, const struct TcgCard *c, char *reason, size_t size)
{
    const struct TcgPlayer *p = &g->players[me];
    unsigned id = Tcg_Canon(c->id), s, bench = 0, i, n;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        bench += !p->field[s].stackSize;
    if ((c->flags & TCG_CF_SUPPORTER) && p->supporterPlayed)
        return snprintf(reason, size, "You already played a Supporter this turn."), 0;
    if ((c->flags & TCG_CF_STADIUM) && TcgDuel_Stadium(g, NULL) == id)
        return snprintf(reason, size, "That Stadium is already in play."), 0;
    switch (id)
    {
    case TCG_CARD_GYM1_17:
    case TCG_CARD_NEO2_74:
    case TCG_CARD_ECARD2_130:
        if (!bench)
            return snprintf(reason, size, "Your Bench is full."), 0;
        break;
    case TCG_CARD_GYM1_105:
        if (p->handSize > 1)
            return snprintf(reason, size, "It must be the only card in your hand."), 0;
        break;
    case TCG_CARD_GYM1_111:
        for (i = 0; i < p->handSize; i++)
            if (Tcg_Card(p->cards[p->hand[i]])->kind == TCG_POKEMON && !Tcg_Card(p->cards[p->hand[i]])->stage)
                return snprintf(reason, size, "You have a Basic Pokemon in your hand."), 0;
        break;
    case TCG_CARD_GYM2_17:
        if (p->energyPlayed)
            return snprintf(reason, size, "You already attached Energy this turn."), 0;
        for (i = 0, n = 0; i < p->handSize; i++)
            n += p->cards[p->hand[i]] == TCG_CARD_FIRE_ENERGY;
        if (n < 2)
            return snprintf(reason, size, "You need 2 Fire Energy cards in your hand."), 0;
        break;
    case TCG_CARD_NEO4_105:
        if (p->handSize >= 5)
            return snprintf(reason, size, "You have 5 or more cards in your hand."), 0;
        break;
    case TCG_CARD_GYM1_18:
    case TCG_CARD_GYM1_109:
        if (p->handSize < 3)
            return snprintf(reason, size, "You need 2 other cards in your hand."), 0;
        break;
    case TCG_CARD_GYM2_118:
        if (p->handSize < 2)
            return snprintf(reason, size, "You need another card in your hand."), 0;
        break;
    }
    return 1;
}
/* A regional Trainer's menu (TCG_ST_CHOICE): Erika's draw */
static unsigned RegionTrainerChoices(const struct TcgDuel *g, unsigned me, unsigned id, struct TcgFlowMenu *m,
                                     const char **reason)
{
    static const char *const draws[4] = {"DRAW 3", "DRAW 2", "DRAW 1", "DRAW NONE"};
    unsigned i, deck = g->players[me].deckSize;
    memset(m, 0, sizeof(*m));
    *reason = NULL;
    if (Tcg_Canon(id) == TCG_CARD_GYM1_16)
        for (i = 0; i < 4; i++)
            MenuAdd(m, draws[i], 3 - i, 3 - i <= deck, "Not that many cards in your deck.");
    if (!m->count)
        *reason = "There is nothing that can be chosen.";
    return m->count;
}
/* The regional sets' used powers: what each asks for (tcg_rpowers.inc; the scripts' ops in tcg_script.inc) */
static const struct TcgStep *RegionPowerSteps(unsigned power, int *count)
{
    switch (power)
    {
    case TCG_POWER_R_FRAGRANCE_TRAP:
    case TCG_POWER_R_FRAGRANCE_TRAP_GYM:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_BENCH, 0, 0, 0, "If heads: bring out which of their Benched Pokemon?"});
    case TCG_POWER_R_JET_STREAM:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_DEFENDER_ATTACHED, TCG_MIN_ALL, 1, "If heads: discard which of its Energy?"});
    case TCG_POWER_R_BEATING_WINGS:
        STEPS({TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "Shuffle which Benched Pokemon into your deck?"});
    case TCG_POWER_R_ENERGY_RETURN:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_FIELD_ATTACHED, 1, 1, "Return which Energy to your hand?"});
    case TCG_POWER_R_WATER_CYCLONE:
        STEPS({TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "Move a Water Energy to which Benched Pokemon?"});
    case TCG_POWER_R_SUPER_DYNAMO:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OCCUPIED, 0, 0, 0, "If heads: attach a Lightning Energy to which Pokemon?"});
    case TCG_POWER_R_FLOWER_SUPPLEMENT:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_HAND, 1, 1, "If heads: attach which basic Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "To which Benched Pokemon?"});
    case TCG_POWER_R_CARRY_OFF:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON_CARD, TCG_SRC_OPP_HAND, 1, 1, "If heads: shuffle which into their deck?"});
    case TCG_POWER_R_GLARING_GAZE:
        STEPS({TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_OPP_HAND, 1, 1, "If heads: shuffle which Trainer into their deck?"});
    case TCG_POWER_R_ENERGY_CHARGE:
        STEPS({TCG_ST_PICK, TCG_F_LIGHTNING_ENERGY, TCG_SRC_MY_BENCH_ATTACHED, 1, 1, "Move which Lightning Energy to it?"});
    case TCG_POWER_R_ENERGY_JUMP:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_FIELD_ATTACHED, 1, 1, "Move which Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_OTHER_HOLDER, 0, 0, 0, "Move it to which Pokemon?"});
    case TCG_POWER_R_HAPPY_HEALING:
        STEPS({TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "Heal which Benched Pokemon? (a coin)"});
    case TCG_POWER_R_FIRE_RECHARGE:
        STEPS({TCG_ST_MY_SLOT, TCG_F_FIRE_POKEMON, 0, 0, 0, "If heads, attach a Fire Energy to which Fire Pokemon?"});
    case TCG_POWER_R_MOONLIGHT:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 1, 1, "Put which card back on your deck?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_DECK, 0, 1, "Take which basic Energy?"});
    case TCG_POWER_R_GAZE:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_HAS_POWER, 0, 0, 0, "Stop which of their Benched Pokemon's power?"});
    case TCG_POWER_R_DOWNPOUR:
        STEPS({TCG_ST_PICK, TCG_F_WATER_ENERGY, TCG_SRC_HAND, 1, 1, "Discard which Water Energy?"});
    case TCG_POWER_R_FIND:
        STEPS({TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_DECK, 0, 1, "Take which Trainer?"});
    case TCG_POWER_R_WANT:
        STEPS({TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_MY_DISCARD, 0, 1, "If heads: take which Trainer?"});
    case TCG_POWER_R_GIVE:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_DECK, 0, 1, "If heads: attach which basic Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_OCCUPIED, 0, 0, 0, "To which of your Pokemon?"});
    case TCG_POWER_R_EMERGE:
        STEPS({TCG_ST_PICK, TCG_F_EVOLVES_FROM_USER, TCG_SRC_DECK, 0, 1, "If heads: evolve it into which card?"});
    case TCG_POWER_R_EVOLUTION_HELPER:
        STEPS({TCG_ST_PICK, TCG_F_EVOLVES_FROM_ACTIVE, TCG_SRC_DECK, 1, 1, "Evolve your Active Pokemon into which card?"});
    case TCG_POWER_R_SOAK_UP:
        STEPS({TCG_ST_PICK, TCG_F_GRASS_ENERGY_OTHER, TCG_SRC_MY_FIELD_ATTACHED, 1, 2,
               "Take which Grass Energy (up to 2) from your other Pokemon?"});
    case TCG_POWER_R_ELECTROMAGNETIC:
        STEPS({TCG_ST_PICK, TCG_F_MAGNET_ENERGY, TCG_SRC_MY_FIELD_ATTACHED, 1, 1, "Move which Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_MAGNET_OTHER, 0, 0, 0, "Move it to which Magnemite or Magneton?"});
    case TCG_POWER_R_BEAR:
        STEPS({TCG_ST_MY_SLOT, TCG_F_UNOWN_DAMAGED_OTHER, 0, 0, 0, "Move a damage counter from which Unown?"});
    case TCG_POWER_R_YIELD:
        STEPS({TCG_ST_PICK, TCG_F_PSYCHIC_ENERGY, TCG_SRC_DECK, 0, 1, "If heads: attach which Psychic Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_UNOWN_SLOT, 0, 0, 0, "To which Unown?"});
    case TCG_POWER_R_HEAT_UP:
        STEPS({TCG_ST_PICK, TCG_F_FIRE_ENERGY, TCG_SRC_DECK, 0, 1, "Attach which Fire Energy?"},
              {TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "To which Benched Pokemon?"});
    case TCG_POWER_R_MIRACLE_SHIFT:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_FIELD_ATTACHED, 1, 1, "Discard which basic Energy?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, 0, 1,
               "Attach which basic Energy from your discard pile? (none: the one discarded)"});
    case TCG_POWER_R_RECONSTRUCTION:
        STEPS({TCG_ST_PICK, TCG_F_OMANYTE_KABUTO, TCG_SRC_DECK, 0, 1, "Take which Omanyte or Kabuto?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_HAND, 1, 1, "Put which basic Energy into your deck?"});
    case TCG_POWER_R_ENERGY_CONNECT:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_BENCH_ATTACHED, 1, 1, "Move which basic Energy to your Active?"});
    }
    *count = 0;
    return sNoSteps;
}
/* What using the Stadium in play asks for (tcg_rtrainers.inc RegionStadiumUse) */
const struct TcgStep *TcgFlow_StadiumSteps(unsigned stadium, int *count)
{
    switch (stadium)
    {
    case TCG_CARD_GYM1_107:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_ERIKA, 0, 0, 0, "Cure which Erika Pokemon?"},
              {TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_ATTACHED, 1, 1, "Discard which of its Energy?"});
    case TCG_CARD_GYM2_114:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_KOGA, 0, 0, 0,
               "If heads: shuffle which Koga Pokemon into your deck?"});
    case TCG_CARD_GYM2_122:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OWNER_SLOT + TCG_OWNER_SABRINA, 0, 0, 0, "From which Sabrina Pokemon?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_ATTACHED, 1, 1, "Return which basic Energy to your hand?"});
    case TCG_CARD_NEO4_95:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_TOP2, 0, 0, "The top 2 cards of your deck (they stay as they are)."});
    case TCG_CARD_NEO4_99:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, 1, 1, "If heads: take which basic Energy?"});
    case TCG_CARD_ECARD2_118:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_HAND, 1, 1, "If heads: show which basic Energy?"},
              {TCG_ST_PICK, TCG_F_BASIC_TYPE_PICK, TCG_SRC_DECK, 0, 1, "Bench which Basic Pokemon of its type?"});
    case TCG_CARD_ECARD2_138:
        STEPS({TCG_ST_MY_SLOT, TCG_F_EVOLVED, 0, 0, 0, "If heads: devolve which Pokemon?"});
    case TCG_CARD_ECARD2_139:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_HAND, 1, 1, "Discard which basic Energy from your hand?"},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, 1, 1, "Take which basic Energy?"});
    case TCG_CARD_ECARD3_137:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_DECK, 0, 1, "Take which basic Energy?"},
              {TCG_ST_PICK, TCG_F_EVOLUTION, TCG_SRC_HAND, 1, 1, "Shuffle which Evolution card into your deck?"});
    case TCG_CARD_ECARD3_141:
        STEPS({TCG_ST_PICK, TCG_F_OMANYTE_KABUTO, TCG_SRC_MY_DISCARD, 1, 1, "Put which onto your Bench?"});
    }
    *count = 0;
    return sNoSteps;
}
