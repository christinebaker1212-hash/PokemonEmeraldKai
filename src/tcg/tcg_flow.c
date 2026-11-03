/* What a move asks for (include/tcg/flow.h): the duel board's choices without its screen, shared by the game's board
 * (tcg_board.c) and the phone (android cpp/tcg_jni.c). */
#include "mp_version.h"
#include <stdio.h>
#include <string.h>
#include "tcg/card_ids.h"
#include "tcg/flow.h"

static const struct TcgStep sNoSteps[1] = {{0, 0, 0, 0, 0, NULL}};
static unsigned CardOf(const struct TcgDuel *g, unsigned who, unsigned physical) { return g->players[who].cards[physical]; }
static const struct TcgCard *Field(const struct TcgDuel *g, unsigned who, unsigned slot) { return TcgDuel_Field(g, who, slot); }

/* ------------------------------------------------------------------------------------------------------ steps */
const struct TcgStep *TcgFlow_TrainerSteps(unsigned id, int *count)
{
#define STEPS(...)                                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        static const struct TcgStep s[] = {__VA_ARGS__};                                                                  \
        *count = sizeof(s) / sizeof(s[0]);                                                                             \
        return s;                                                                                                      \
    } while (0)
    switch (id)
    {
    case TCG_CARD_POTION:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED, 0, 0, 0, "Remove 2 damage counters from which Pokemon?"});
    case TCG_CARD_SUPER_POTION:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED_ENERGY, 0, 0, 0, "Heal which Pokemon? (discard 1 Energy from it)"},
              {TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_ATTACHED, 1, 1, "Discard which Energy?"});
    case TCG_CARD_DEFENDER:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OCCUPIED, 0, 0, 0, "Attach DEFENDER to which Pokemon?"});
    case TCG_CARD_SWITCH:
        STEPS({TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "Switch in which Benched Pokemon?"});
    case TCG_CARD_GUST_OF_WIND:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_BENCH, 0, 0, 0, "Bring out which of their Benched Pokemon?"});
    case TCG_CARD_SCOOP_UP:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OCCUPIED, 0, 0, 0, "Return which Pokemon to your hand?"});
    case TCG_CARD_MR_FUJI:
        STEPS({TCG_ST_MY_SLOT, TCG_F_BENCH, 0, 0, 0, "Shuffle which Benched Pokemon into your deck?"});
    case TCG_CARD_CLEFAIRY_DOLL:
    case TCG_CARD_MYSTERIOUS_FOSSIL:
        STEPS({TCG_ST_MY_SLOT, TCG_F_EMPTY_BENCH, 0, 0, 0, "Put it on which Bench space?"});
    case TCG_CARD_DEVOLUTION_SPRAY:
        STEPS({TCG_ST_MY_SLOT, TCG_F_EVOLVED, 0, 0, 0, "Devolve which Pokemon?"}, {TCG_ST_STAGE, 0, 0, 0, 0, "Discard how far?"});
    case TCG_CARD_ENERGY_REMOVAL:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_HAS_ENERGY, 0, 0, 0, "Remove Energy from which Pokemon?"},
              {TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_OPP_ATTACHED, 1, 1, "Discard which Energy?"});
    case TCG_CARD_SUPER_ENERGY_REMOVAL:
        STEPS({TCG_ST_MY_SLOT, TCG_F_HAS_ENERGY, 0, 0, 0, "First, discard Energy from one of yours."},
              {TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_MY_ATTACHED, 1, 1, "Discard which of your Energy?"},
              {TCG_ST_OPP_SLOT, TCG_F_HAS_ENERGY, 0, 0, 0, "Then remove Energy from which of theirs?"},
              {TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_OPP_ATTACHED, 1, 2, "Discard up to 2 of their Energy."});
    case TCG_CARD_ENERGY_SEARCH:
        STEPS({TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_DECK, 1, 1, "Take which basic Energy?"});
    case TCG_CARD_POKE_BALL:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON, TCG_SRC_DECK, 1, 1, "If heads, take which Pokemon?"});
    case TCG_CARD_COMPUTER_SEARCH:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 2, 2, "Discard 2 cards from your hand."},
              {TCG_ST_PICK, TCG_F_ANY, TCG_SRC_DECK, 1, 1, "Take any 1 card from your deck."});
    case TCG_CARD_ITEM_FINDER:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 2, 2, "Discard 2 cards from your hand."},
              {TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_MY_DISCARD, 1, 1, "Take which Trainer from your discard pile?"});
    case TCG_CARD_ENERGY_RETRIEVAL:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 1, 1, "Discard 1 card from your hand."},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, 1, 2, "Take up to 2 basic Energy."});
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 2, 2, "Discard 2 cards from your hand."},
              {TCG_ST_PICK, TCG_F_BASIC_ENERGY, TCG_SRC_MY_DISCARD, 1, 4, "Take up to 4 basic Energy."});
    case TCG_CARD_MAINTENANCE:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 2, 2, "Shuffle 2 cards into your deck."});
    case TCG_CARD_RECYCLE:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_MY_DISCARD, 1, 1, "If heads, put which card on your deck?"});
    case TCG_CARD_POKEMON_TRADER:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON, TCG_SRC_HAND, 1, 1, "Trade away which Pokemon?"},
              {TCG_ST_PICK, TCG_F_POKEMON, TCG_SRC_DECK, 1, 1, "Take which Pokemon from your deck?"});
    case TCG_CARD_POKEMON_BREEDER:
        STEPS({TCG_ST_PICK, TCG_F_STAGE2, TCG_SRC_HAND, 1, 1, "Play which Stage 2 card?"},
              {TCG_ST_MY_SLOT, TCG_F_BREEDER_BASIC, 0, 0, 0, "Put it on which Basic Pokemon?"});
    case TCG_CARD_REVIVE:
        STEPS({TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_MY_DISCARD, 1, 1, "Revive which Basic Pokemon?"},
              {TCG_ST_MY_SLOT, TCG_F_EMPTY_BENCH, 0, 0, 0, "Put it on which Bench space?"});
    case TCG_CARD_POKEMON_FLUTE:
        STEPS({TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_OPP_DISCARD, 1, 1, "Put which of their Basic Pokemon in play?"},
              {TCG_ST_OPP_SLOT, TCG_F_EMPTY_BENCH, 0, 0, 0, "Put it on which of their Bench spaces?"});
    case TCG_CARD_POKEDEX:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_TOP5, 0, 0, "Choose the order: the first is drawn next."});
    /* GB2's */
    case TCG_CARD_POKEBALL:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON, TCG_SRC_DECK, 1, 1, "If heads, take which Pokemon?"});
    case TCG_CARD_FOSSIL_EXCAVATION:
        STEPS({TCG_ST_PICK, TCG_F_FOSSIL, TCG_SRC_DECK, 0, 1, "Take a Mysterious Fossil from your deck? (none: discard pile)"});
    case TCG_CARD_MAX_REVIVE:
        STEPS({TCG_ST_PICK, TCG_F_ENERGY, TCG_SRC_HAND, 2, 2, "Discard 2 Energy cards from your hand."},
              {TCG_ST_PICK, TCG_F_BASIC, TCG_SRC_MY_DISCARD, 1, 1, "Revive which Basic Pokemon?"},
              {TCG_ST_MY_SLOT, TCG_F_EMPTY_BENCH, 0, 0, 0, "Put it on which Bench space?"});
    case TCG_CARD_SUPER_SCOOP_UP:
        STEPS({TCG_ST_MY_SLOT, TCG_F_OCCUPIED, 0, 0, 0, "If heads, return which Pokemon to your hand?"});
    case TCG_CARD_MOON_STONE:
        STEPS({TCG_ST_PICK, TCG_F_COLORLESS_EVOLUTION, TCG_SRC_DECK, 0, 1, "Take which Colorless Evolution card?"});
    case TCG_CARD_THE_BOSSS_WAY:
        STEPS({TCG_ST_PICK, TCG_F_DARK_EVOLUTION, TCG_SRC_DECK, 0, 1, "Take which Dark Pokemon card?"});
    case TCG_CARD_IMPOSTER_OAKS_REVENGE:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 1, 1, "Discard which card to play it?"});
    case TCG_CARD_POKEMON_RECALL:
        STEPS({TCG_ST_PICK, TCG_F_EVOLUTION, TCG_SRC_MY_DISCARD, 1, 1, "Put which Evolution card on your deck?"});
    case TCG_CARD_MASTER_BALL:
        STEPS({TCG_ST_PICK, TCG_F_POKEMON, TCG_SRC_TOP7, 0, 1, "Take which Pokemon? (the rest are shuffled in)"});
    case TCG_CARD_NIGHTLY_GARBAGE_RUN:
        STEPS({TCG_ST_PICK, TCG_F_GARBAGE, TCG_SRC_MY_DISCARD, 1, 3, "Shuffle up to 3 into your deck."});
    }
    *count = 0;
    return sNoSteps;
}
/* A used Pokemon Power's choices: the first Pokemon chosen is action.target, the
 * second action.attack (src/tcg/tcg_powers.c TcgDuel_Power) */
const struct TcgStep *TcgFlow_PowerSteps(unsigned power, int *count)
{
    switch (power)
    {
    case TCG_POWER_ENERGY_TRANS:
        STEPS({TCG_ST_MY_SLOT, TCG_F_HAS_GRASS, 0, 0, 0, "Move a Grass Energy from which Pokemon?"},
              {TCG_ST_MY_SLOT, TCG_F_OTHER, 0, 0, 0, "Move it to which Pokemon?"});
    case TCG_POWER_HEAL:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED, 0, 0, 0, "Heal which Pokemon? (a coin: heads, 1 counter)"});
    case TCG_POWER_SHIFT:
        STEPS({TCG_ST_CHOICE, 0, 0, 0, 0, "Change its type to which?"});
    case TCG_POWER_RAIN_DANCE:
        STEPS({TCG_ST_MY_SLOT, TCG_F_WATER_POKEMON, 0, 0, 0, "Attach a Water Energy to which Water Pokemon?"});
    case TCG_POWER_PEEK:
        STEPS({TCG_ST_CHOICE, 0, 0, 0, 0, "Look at which card?"});
    case TCG_POWER_DAMAGE_SWAP:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED, 0, 0, 0, "Move a damage counter from which Pokemon?"},
              {TCG_ST_MY_SLOT, TCG_F_ROOM_OTHER, 0, 0, 0, "Move it to which Pokemon?"});
    case TCG_POWER_STRANGE_BEHAVIOR:
        STEPS({TCG_ST_MY_SLOT, TCG_F_DAMAGED_OTHER, 0, 0, 0, "Move a damage counter from which Pokemon?"});
    case TCG_POWER_CURSE:
        STEPS({TCG_ST_OPP_SLOT, TCG_F_DAMAGED, 0, 0, 0, "Move a damage counter from which of their Pokemon?"},
              {TCG_ST_OPP_SLOT, TCG_F_OTHER, 0, 0, 0, "Move it to which of their Pokemon?"});
    /* GB2's */
    case TCG_POWER_GATHER_FIRE:
        STEPS({TCG_ST_MY_SLOT, TCG_F_HAS_FIRE_OTHER, 0, 0, 0, "Take a Fire Energy from which Pokemon?"});
    case TCG_POWER_FOSSILIZE:
        STEPS({TCG_ST_MY_SLOT, TCG_F_FOSSIL_EVOLVED, 0, 0, 0, "If heads, which Pokemon goes back to a Fossil?"});
    case TCG_POWER_MATTER_EXCHANGE:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 1, 1, "Discard which card to draw one?"});
    case TCG_POWER_EVOLUTIONARY_LIGHT:
        STEPS({TCG_ST_PICK, TCG_F_EVOLUTION, TCG_SRC_DECK, 1, 1, "Take which Evolution card?"});
    case TCG_POWER_SPECIAL_DELIVERY:
        STEPS({TCG_ST_PICK, TCG_F_ANY, TCG_SRC_HAND, 0, 1, "Draw, then put which card on top? (none: the new one)"});
    }
    *count = 0;
    return sNoSteps;
}
#undef STEPS
/* The prompt for an attack's menu */
const char *TcgFlow_AttackPrompt(const char *name)
{
    static const char *const prompts[][2] = {
        {"Metronome", "Copy which of their attacks?"},
        {"Amnesia", "Which attack can't they use next turn?"},
        {"Mirror Move", "Which attack can't they use next turn?"},
        {"Wildfire", "Discard how many Fire Energy?"},
        {"Conversion 1", "Change their Weakness to which type?"},
        {"Conversion 2", "Change its Resistance to which type?"},
        {"Prophecy", "Look at the top of which deck?"},
        {"Devolution Beam", "Devolve whose Pokemon?"},
        {"Disable", "If heads, which attack can't they use?"},
        {"Mini-Metronome", "If heads, copy which of their attacks?"},
        {"Flame Inferno", "Discard how many Fire Energy?"},
        {"Burning Fire", "Discard how many Fire Energy (any of yours)?"},
        {"Flame Pillar", "Discard a Fire Energy to hit their Bench?"},
        {"Rock Blast", "Discard how many Fighting Energy?"},
        {"Rollout", "Remove how many Food counters?"},
        {"Texture Magic", "Change its Resistance to which type?"},
    };
    unsigned k;
    for (k = 0; name && k < sizeof(prompts) / sizeof(prompts[0]); k++)
        if (!strcmp(name, prompts[k][0]))
            return prompts[k][1];
    return "Choose for the attack.";
}

static int BenchOn(const struct TcgDuel *g, unsigned who)
{
    unsigned s;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        if (Field(g, who, s))
            return 1;
    return 0;
}

int TcgFlow_AttackSteps(const struct TcgDuel *g, unsigned me, const struct TcgAction *a, const struct TcgAttack *attack,
                        int afterOption, struct TcgStep out[4], char prompt[64])
{
    const char *labels[TCG_MAX_OPTIONS];
    uint8_t ok[TCG_MAX_OPTIONS];
    unsigned choice, ask;
    int n = 0;
    if (!afterOption && attack && TcgDuel_AttackOptions(g, me, attack, labels, ok))
    {
        struct TcgStep st = {TCG_ST_ATTACK_OPTION, 0, 0, 0, 0, prompt};
        snprintf(prompt, 64, "%s", TcgFlow_AttackPrompt(attack->name));
        out[0] = st;
        return 1;
    }
    choice = attack ? TcgDuel_AttackChoice(attack, NULL) : 0;
    ask = attack ? TcgDuel_AttackAsk(attack) : TCG_ASK_NONE;
    if ((choice == 1 && BenchOn(g, me ^ 1)) || (choice == 2 && BenchOn(g, me)) || choice == 3 || choice == 4)
    {
        const char *words = TcgDuel_AttackTargetPrompt(attack);
        struct TcgStep st = {choice == 1 || choice == 3 ? TCG_ST_OPP_SLOT : TCG_ST_MY_SLOT,
                             choice >= 3 ? TCG_F_OCCUPIED : TCG_F_BENCH, 0, 0, 0,
                             words         ? words
                             : choice == 1 ? "Aim at which of their Benched Pokemon?"
                             : choice == 2 ? "Switch with which of your Benched Pokemon?"
                             : choice == 3 ? "Aim at which of their Pokemon?"
                                           : "Choose one of your Pokemon."};
        out[n++] = st;
    }
    if (ask == TCG_ASK_DISCARD_TRAINER)
    {
        struct TcgStep st = {TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_MY_DISCARD, 1, 1, "Put which Trainer into your hand?"};
        out[n++] = st;
    }
    else if (ask == TCG_ASK_ORDER_TOP3)
    {
        struct TcgStep st = {TCG_ST_PICK, TCG_F_ANY, a->option ? TCG_SRC_TOP3_THEIRS : TCG_SRC_TOP3_MINE, 0, 3,
                             "Put them back in order: the first chosen goes on top."};
        out[n++] = st;
    }
    else if (ask == TCG_ASK_DECK_TRAINER)
    {
        struct TcgStep st = {TCG_ST_PICK, TCG_F_TRAINER, TCG_SRC_DECK, 0, 1, "If heads, take which Trainer?"};
        out[n++] = st;
    }
    else if (ask == TCG_ASK_EVOLVED)
    {
        struct TcgStep st = {a->option ? TCG_ST_OPP_SLOT : TCG_ST_MY_SLOT, TCG_F_EVOLVED, 0, 0, 0, "Devolve which Pokemon?"};
        out[n++] = st;
    }
    return n;
}

/* ---------------------------------------------------------------------------------------------------- filters */
unsigned TcgFlow_EnergyValue(unsigned id) { return id == TCG_CARD_DOUBLE_COLORLESS_ENERGY ? 2 : 1; }

int TcgFlow_HasEnergy(const struct TcgDuel *g, unsigned who, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (Tcg_Card(CardOf(g, who, f->attached[i]))->kind == TCG_ENERGY)
            return 1;
    return 0;
}

int TcgFlow_CardMatches(const struct TcgDuel *g, unsigned owner, unsigned physical, int filter)
{
    const struct TcgCard *c = Tcg_Card(CardOf(g, owner, physical));
    if (!c)
        return 0;
    switch (filter)
    {
    case TCG_F_POKEMON:
        return c->kind == TCG_POKEMON;
    case TCG_F_BASIC:
        return c->kind == TCG_POKEMON && !c->stage;
    case TCG_F_BASIC_ENERGY:
        return c->kind == TCG_ENERGY && c->id <= 6;
    case TCG_F_ENERGY:
        return c->kind == TCG_ENERGY;
    case TCG_F_TRAINER:
        return c->kind == TCG_TRAINER;
    case TCG_F_STAGE2:
        return c->kind == TCG_POKEMON && c->stage == 2;
    case TCG_F_FOSSIL:
        return c->id == TCG_CARD_MYSTERIOUS_FOSSIL;
    case TCG_F_EVOLUTION:
        return c->kind == TCG_POKEMON && c->stage;
    case TCG_F_COLORLESS_EVOLUTION:
        return c->kind == TCG_POKEMON && c->stage && c->type == TCG_COLORLESS;
    case TCG_F_DARK_EVOLUTION:
        return c->kind == TCG_POKEMON && c->stage && (c->dark || !strncmp(c->name, "Dark ", 5));
    case TCG_F_GARBAGE:
        return c->kind == TCG_POKEMON || (c->kind == TCG_ENERGY && c->id <= 6);
    default:
        return 1;
    }
}

int TcgFlow_SlotMatches(const struct TcgDuel *g, unsigned me, const struct TcgAction *a, unsigned flowCard, unsigned who,
                        unsigned slot, int filter)
{
    const struct TcgPlayer *p = &g->players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    const struct TcgCard *c = Field(g, who, slot);
    switch (filter)
    {
    case TCG_F_ANY:
        return 1;
    case TCG_F_OCCUPIED:
        return c != NULL;
    case TCG_F_BENCH:
        return slot && c;
    case TCG_F_EMPTY_BENCH:
        return slot && !c;
    case TCG_F_DAMAGED:
        return c && f->damage;
    case TCG_F_DAMAGED_ENERGY:
        return c && f->damage && TcgFlow_HasEnergy(g, who, slot);
    case TCG_F_HAS_ENERGY:
        return c && TcgFlow_HasEnergy(g, who, slot);
    case TCG_F_EVOLVED:
        return c && f->stackSize >= 2;
    case TCG_F_BREEDER_BASIC:
    {
        const struct TcgCard *stage2 = a->selectionCount ? Tcg_Card(CardOf(g, me, a->selections[0])) : NULL;
        unsigned i;
        if (!c || c->kind != TCG_POKEMON || c->stage || !stage2)
            return 0;
        for (i = 1; i <= TCG_CARD_COUNT; i++)
            if (gTcgCards[i].kind == TCG_POKEMON && gTcgCards[i].stage == 1 &&
                !strcmp(gTcgCards[i].name, stage2->evolvesFrom) && !strcmp(gTcgCards[i].evolvesFrom, c->name))
                return 1;
        return 0;
    }
    case TCG_F_EVOLVE_TARGET:
    {
        /* (not on its first turn in play) */
        const struct TcgCard *e = Tcg_Card(flowCard);
        return c && e && c->kind == TCG_POKEMON && !strcmp(e->evolvesFrom, c->name) && p->turnsTaken >= 2 &&
               f->enteredTurn != g->turn;
    }
    case TCG_F_BASIC_SLOT:
        /* (while setting up, the Active spot first: a Basic on the Bench with nothing Active could leave no way on) */
        if (g->phase == TCG_SETUP)
            return !c && (!slot || p->field[0].stackSize);
        return !c && slot;
    case TCG_F_ANY_POKEMON:
        return c && c->kind == TCG_POKEMON;
    case TCG_F_HAS_GRASS:
    {
        unsigned i;
        for (i = 0; c && i < f->attachedSize; i++)
            if (CardOf(g, who, f->attached[i]) == TCG_CARD_GRASS_ENERGY)
                return 1;
        return 0;
    }
    case TCG_F_OTHER:
        return c && slot != a->target;
    case TCG_F_WATER_POKEMON:
        return c && c->kind == TCG_POKEMON && TcgDuel_Type(g, who, slot) == TCG_WATER;
    case TCG_F_ROOM_OTHER:
        return c && slot != a->target && f->damage + 10u < (c->kind == TCG_TRAINER ? 10u : c->hp);
    case TCG_F_DAMAGED_OTHER:
        return c && f->damage && slot != a->slot;
    case TCG_F_HAS_FIRE_OTHER:
    {
        unsigned i;
        for (i = 0; c && slot != a->slot && i < f->attachedSize; i++)
            if (CardOf(g, who, f->attached[i]) == TCG_CARD_FIRE_ENERGY)
                return 1;
        return 0;
    }
    case TCG_F_FOSSIL_EVOLVED:
        return c && f->stackSize > 1 && CardOf(g, who, f->stack[0]) == TCG_CARD_MYSTERIOUS_FOSSIL;
    }
    return 0;
}

/* ---------------------------------------------------------------------------------------------- can it be used */
const char *TcgFlow_HandVerb(const struct TcgCard *c)
{
    return c->kind == TCG_ENERGY ? "ATTACH" : c->kind == TCG_TRAINER ? "PLAY" : c->stage ? "EVOLVE" : "PLAY";
}

int TcgFlow_HasPower(const struct TcgCard *c)
{
    return c && c->kind == TCG_POKEMON && c->attacks[0].category == TCG_POKEMON_POWER && c->attacks[0].name[0];
}

int TcgFlow_HandPlayable(const struct TcgDuel *g, unsigned me, const struct TcgCard *c, char *reason, size_t size)
{
    const struct TcgPlayer *p = &g->players[me];
    unsigned s;
    if (g->phase == TCG_PROMOTION)
        return snprintf(reason, size, "Choose your next Active Pokemon first."), 0;
    if (g->phase == TCG_SETUP)
    {
        if (c->kind != TCG_POKEMON || c->stage)
            return snprintf(reason, size, "Only Basic Pokemon are placed now."), 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (!p->field[s].stackSize)
                return 1;
        return snprintf(reason, size, "Your Bench is full."), 0;
    }
    if (c->kind == TCG_ENERGY && p->energyPlayed)
        return snprintf(reason, size, "You already attached Energy this turn."), 0;
    if (c->kind == TCG_POKEMON && !c->stage)
    {
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (!p->field[s].stackSize)
                return 1;
        return snprintf(reason, size, "Your Bench is full."), 0;
    }
    if (c->kind == TCG_POKEMON)
    {
        const struct TcgCard *young = NULL;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *f = Field(g, me, s);
            if (f && f->kind == TCG_POKEMON && !strcmp(c->evolvesFrom, f->name))
            {
                if (p->turnsTaken >= 2 && p->field[s].enteredTurn != g->turn)
                    return 1;
                young = f;
            }
        }
        if (young)
            return snprintf(reason, size, "%s can't evolve on its first turn in play.", young->name), 0;
        return snprintf(reason, size, "You have no %s in play.", c->evolvesFrom), 0;
    }
    if (c->kind == TCG_TRAINER && p->noTrainers)
        return snprintf(reason, size, "Headache: no Trainer cards this turn."), 0;
    return 1;
}

/* Would this Trainer do anything now? Only the plain cases say no (nothing to heal, nothing to switch to, an empty
   deck...); anything unsure counts as playable. */
static int Has(const struct TcgDuel *g, unsigned who, int (*test)(const struct TcgDuel *, const struct TcgFieldCard *, unsigned),
               int benchOnly)
{
    unsigned s;
    for (s = benchOnly ? 1 : 0; s < TCG_FIELD_SIZE; s++)
        if (g->players[who].field[s].stackSize && test(g, &g->players[who].field[s], who))
            return 1;
    return 0;
}
static int AnyCard(const struct TcgDuel *g, const struct TcgFieldCard *f, unsigned who) { (void)g, (void)f, (void)who; return 1; }
static int Damaged(const struct TcgDuel *g, const struct TcgFieldCard *f, unsigned who) { (void)g, (void)who; return f->damage > 0; }
static int Evolved(const struct TcgDuel *g, const struct TcgFieldCard *f, unsigned who) { (void)g, (void)who; return f->stackSize > 1; }
static int WithEnergy(const struct TcgDuel *g, const struct TcgFieldCard *f, unsigned who)
{
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (Tcg_Card(g->players[who].cards[f->attached[i]])->kind == TCG_ENERGY)
            return 1;
    return 0;
}
static int DamagedWithEnergy(const struct TcgDuel *g, const struct TcgFieldCard *f, unsigned who)
{
    return f->damage && WithEnergy(g, f, who);
}
static int InDiscard(const struct TcgDuel *g, unsigned me, int kind, int basicOnly)
{
    const struct TcgPlayer *p = &g->players[me];
    unsigned i;
    for (i = 0; i < p->discardSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->discard[i]]);
        if (c->kind == kind && (!basicOnly || (kind == TCG_POKEMON ? !c->stage : p->cards[p->discard[i]] <= 6)))
            return 1;
    }
    return 0;
}
int TcgFlow_TrainerHasUse(const struct TcgDuel *g, unsigned me, unsigned id)
{
    const struct TcgPlayer *p = &g->players[me];
    int bench = 0;
    unsigned s, them = me ^ 1;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        bench += !p->field[s].stackSize;
    switch (id)
    {
    case TCG_CARD_POTION:
    case TCG_CARD_POKEMON_CENTER:
        return Has(g, me, Damaged, 0);
    case TCG_CARD_SUPER_POTION:
        return Has(g, me, DamagedWithEnergy, 0);
    case TCG_CARD_FULL_HEAL:
        return p->field[0].stackSize && (p->field[0].condition || p->field[0].poison);
    case TCG_CARD_SWITCH:
        return Has(g, me, AnyCard, 1);
    case TCG_CARD_GUST_OF_WIND:
        return Has(g, them, AnyCard, 1);
    case TCG_CARD_ENERGY_REMOVAL:
        return Has(g, them, WithEnergy, 0);
    case TCG_CARD_SUPER_ENERGY_REMOVAL:
        return Has(g, them, WithEnergy, 0) && Has(g, me, WithEnergy, 0);
    case TCG_CARD_DEVOLUTION_SPRAY:
        return Has(g, me, Evolved, 0);
    case TCG_CARD_SCOOP_UP:
    case TCG_CARD_SUPER_SCOOP_UP:
        return 6 - bench >= 2;   /* (never the last Pokemon in play) */
    case TCG_CARD_CLEFAIRY_DOLL:
    case TCG_CARD_MYSTERIOUS_FOSSIL:
        return bench > 0;
    case TCG_CARD_REVIVE:
        return bench > 0 && InDiscard(g, me, TCG_POKEMON, 1);
    case TCG_CARD_BILL:
    case TCG_CARD_POKEDEX:
    case TCG_CARD_POKE_BALL:
    case TCG_CARD_ENERGY_SEARCH:
        return p->deckSize > 0;
    case TCG_CARD_COMPUTER_SEARCH:
        return p->deckSize > 0 && p->handSize >= 3;
    case TCG_CARD_ITEM_FINDER:
        return p->handSize >= 3 && InDiscard(g, me, TCG_TRAINER, 0);
    case TCG_CARD_ENERGY_RETRIEVAL:
        return p->handSize >= 2 && InDiscard(g, me, TCG_ENERGY, 1);
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
        return p->handSize >= 3 && InDiscard(g, me, TCG_ENERGY, 1);
    case TCG_CARD_MAINTENANCE:
        return p->handSize >= 3;
    }
    return 1;
}

int TcgFlow_AttackReady(const struct TcgDuel *g, unsigned me, unsigned attack, char *reason, size_t size)
{
    const struct TcgCard *c = TcgDuel_Attacker(g, me), *active = Field(g, me, 0);
    const struct TcgFieldCard *f = &g->players[me].field[0];
    if (!c || c->kind != TCG_POKEMON || !c->attacks[attack].name[0])
        return snprintf(reason, size, "No attack."), 0;
    if (c->attacks[attack].category == TCG_POKEMON_POWER)
        return snprintf(reason, size, "This is a Pokemon Power."), 0;
    if (!TcgDuel_EffectSupported(c->attacks[attack].effect))
        return snprintf(reason, size, "This attack's effect isn't ready yet."), 0;
    if (TcgDuel_EffectBlock(g, me, &c->attacks[attack]))
        return snprintf(reason, size, "%s", TcgDuel_EffectBlock(g, me, &c->attacks[attack])), 0;
    if (!TcgDuel_CanAttack(g, me, attack))
        return snprintf(reason, size, "Not enough Energy attached."), 0;
    if (f->condition == TCG_ASLEEP && !TcgDuel_SleepAttack(&c->attacks[attack]))
        return snprintf(reason, size, "%s is Asleep.", active->name), 0;
    if (f->condition == TCG_PARALYZED)
        return snprintf(reason, size, "%s is Paralyzed.", active->name), 0;
    if (!TcgDuel_CanAttackNow(g, me, attack))
        return snprintf(reason, size, "%s can't use that attack this turn.", active->name), 0;
    if (!Field(g, me ^ 1, 0))
        return snprintf(reason, size, "There is no Defending Pokemon."), 0;
    return 1;
}

int TcgFlow_CanRetreat(const struct TcgDuel *g, unsigned me, char *reason, size_t size)
{
    const struct TcgCard *c = Field(g, me, 0);
    const struct TcgPlayer *p = &g->players[me];
    const struct TcgFieldCard *f = &p->field[0];
    unsigned s, bench = 0, cost = TcgDuel_RetreatCost(g, me);
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        bench += p->field[s].stackSize != 0;
    if (!c)
        return snprintf(reason, size, "No Active Pokemon."), 0;
    if (!bench)
        return snprintf(reason, size, "No Benched Pokemon to switch in."), 0;
    if (p->retreated || p->confusionRetreatFailed)
        return snprintf(reason, size, "You can only retreat once each turn."), 0;
    if (c->kind == TCG_TRAINER || f->noRetreat)
        return snprintf(reason, size, "%s can't retreat.", c->name), 0;
    if (f->condition == TCG_ASLEEP || f->condition == TCG_PARALYZED)
        return snprintf(reason, size, "%s is %s.", c->name, f->condition == TCG_ASLEEP ? "Asleep" : "Paralyzed"), 0;
    if (f->lockTurn == g->turn && f->lockAttack == 3)
        return snprintf(reason, size, "%s can't retreat this turn.", c->name), 0;
    if (TcgPower_RetreatBlock(g, me))
        return snprintf(reason, size, "%s", TcgPower_RetreatBlock(g, me)), 0;
    if (TcgDuel_Energy(g, me, 0, TCG_NONE) < cost)
        return snprintf(reason, size, "Retreating costs %u Energy.", cost), 0;
    return 1;
}

int TcgFlow_RetreatAutoPay(const struct TcgDuel *g, unsigned me, struct TcgAction *a)
{
    const struct TcgFieldCard *f = &g->players[me].field[0];
    unsigned i, total = 0, types = 0, first = TCG_NONE, cost = TcgDuel_RetreatCost(g, me), paid = 0;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *e = Tcg_Card(CardOf(g, me, f->attached[i]));
        if (e->kind != TCG_ENERGY)
            continue;
        total += TcgFlow_EnergyValue(e->id);
        if (first == TCG_NONE || e->id != first)
            types++;
        if (first == TCG_NONE)
            first = e->id;
    }
    a->selectionCount = 0;
    if (cost && total != cost && types != 1)
        return 0;
    /* Nothing to choose: pay with the first Energy that covers it. */
    for (i = 0; i < f->attachedSize && paid < cost; i++)
    {
        const struct TcgCard *e = Tcg_Card(CardOf(g, me, f->attached[i]));
        if (e->kind == TCG_ENERGY)
        {
            a->selections[a->selectionCount++] = f->attached[i];
            paid += TcgFlow_EnergyValue(e->id);
        }
    }
    return 1;
}

static void MenuAdd(struct TcgFlowMenu *m, const char *label, unsigned value, int ok, const char *why)
{
    if (m->count >= TCG_FLOW_MENU)
        return;
    m->label[m->count] = label;
    m->value[m->count] = (uint8_t)value;
    m->ok[m->count] = (uint8_t)(ok != 0);
    m->why[m->count] = ok ? NULL : why;
    m->count++;
}

unsigned TcgFlow_PowerChoices(const struct TcgDuel *g, unsigned me, unsigned slot, struct TcgFlowMenu *m,
                              const char **reason)
{
    unsigned power = TcgPower_Id(Field(g, me, slot)), t, who, s;
    memset(m, 0, sizeof(*m));
    *reason = NULL;
    if (power == TCG_POWER_SHIFT)
    {
        static const char *const names[TCG_COLORLESS] = {"FIRE", "GRASS", "LIGHTNING", "WATER", "FIGHTING", "PSYCHIC"};
        for (t = 0; t < TCG_COLORLESS; t++)
        {
            int found = 0;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    found |= (who != me || s != slot) && Field(g, who, s) && Field(g, who, s)->kind == TCG_POKEMON &&
                             TcgDuel_Type(g, who, s) == t;
            if (found && t != TcgDuel_Type(g, me, slot))
                MenuAdd(m, names[t], t, 1, NULL);
        }
        if (!m->count)
            *reason = "Every other type in play is already its own.";
    }
    else
    {
        const struct TcgPlayer *mine = &g->players[me], *them = &g->players[me ^ 1];
        MenuAdd(m, "TOP OF YOUR DECK", TCG_PEEK_MY_DECK, mine->deckSize != 0, "Your deck is empty.");
        MenuAdd(m, "TOP OF THEIR DECK", TCG_PEEK_THEIR_DECK, them->deckSize != 0, "Their deck is empty.");
        MenuAdd(m, "A CARD IN THEIR HAND", TCG_PEEK_THEIR_HAND, them->handSize != 0, "Their hand is empty.");
        MenuAdd(m, "ONE OF YOUR PRIZES", TCG_PEEK_MY_PRIZE, mine->prizeSize != 0, "You have no Prizes left.");
        MenuAdd(m, "ONE OF THEIR PRIZES", TCG_PEEK_THEIR_PRIZE, them->prizeSize != 0, "They have no Prizes left.");
    }
    return m->count;
}

unsigned TcgFlow_Actor(const struct TcgDuel *g)
{
    if (g->phase == TCG_FINISHED)
        return TCG_NONE;
    return g->phase == TCG_PROMOTION ? g->promotePlayer : g->current;
}

/* -------------------------------------------------------------------------------------- the machine (no screen) */
static unsigned Fail(struct TcgFlow *f, const char *why)
{
    snprintf(f->error, sizeof(f->error), "%s", why);
    return f->q = TCG_Q_FAIL;
}

static unsigned Done(struct TcgFlow *f)
{
    return f->q = TCG_Q_DONE;
}

/* A slot question for `who` with `filter` (nothing to choose: it fails) */
static unsigned AskSlot(struct TcgFlow *f, const struct TcgDuel *g, unsigned who, int filter, const char *prompt)
{
    unsigned s;
    f->who = who;
    f->slotMask = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (TcgFlow_SlotMatches(g, f->me, &f->action, f->flowCard, who, s, filter))
            f->slotMask |= (uint8_t)(1 << s);
    f->prompt = prompt;
    if (!f->slotMask)
        return Fail(f, "There is nothing that can be chosen.");
    return f->q = TCG_Q_SLOT;
}

/* A pick over a pile (as the board's OpenPick: a searched deck in catalogue order, never its real order) */
static unsigned AskPick(struct TcgFlow *f, const struct TcgDuel *g, int source, int filter, int min, int max,
                        const char *prompt)
{
    unsigned owner = source == TCG_SRC_OPP_DISCARD || source == TCG_SRC_OPP_ATTACHED || source == TCG_SRC_TOP3_THEIRS
                         ? f->me ^ 1
                         : f->me,
             i, n = 0, count = 0, valid = 0;
    const struct TcgPlayer *p = &g->players[owner];
    const uint8_t *pile = NULL;
    uint8_t sorted[TCG_DECK_SIZE];
    switch (source)
    {
    case TCG_SRC_HAND:
        pile = p->hand, count = p->handSize;
        break;
    case TCG_SRC_DECK:
        pile = p->deck, count = p->deckSize;
        break;
    case TCG_SRC_MY_DISCARD:
    case TCG_SRC_OPP_DISCARD:
        pile = p->discard, count = p->discardSize;
        break;
    case TCG_SRC_MY_ATTACHED:
        pile = p->field[f->action.slot].attached, count = p->field[f->action.slot].attachedSize;
        break;
    case TCG_SRC_OPP_ATTACHED:
        pile = p->field[f->action.target].attached, count = p->field[f->action.target].attachedSize;
        break;
    case TCG_SRC_TOP7:
        count = p->deckSize < 7 ? p->deckSize : 7;
        for (i = 0; i < count; i++)
            sorted[i] = p->deck[p->deckSize - 1 - i];
        pile = sorted;
        break;
    case TCG_SRC_TOP5:
    case TCG_SRC_TOP3_MINE:
    case TCG_SRC_TOP3_THEIRS:
        for (i = 0; i < (unsigned)max; i++)
            sorted[i] = p->deck[p->deckSize - 1 - i];
        pile = sorted, count = (unsigned)max;
        break;
    }
    for (i = 0; i < count; i++)
    {
        if (source == TCG_SRC_HAND && f->kind == TCG_FLOW_HAND && i == f->action.hand)
            continue;
        f->pick[n++] = pile[i];
    }
    if (source == TCG_SRC_DECK)
    {
        unsigned a, b;
        for (a = 0; a < n; a++)
            for (b = a + 1; b < n; b++)
                if (CardOf(g, owner, f->pick[b]) < CardOf(g, owner, f->pick[a]))
                {
                    uint8_t t = f->pick[a];
                    f->pick[a] = f->pick[b];
                    f->pick[b] = t;
                }
    }
    for (i = 0; i < n; i++)
        valid += f->pickOk[i] = (uint8_t)TcgFlow_CardMatches(g, owner, f->pick[i], filter);
    f->pickCount = (uint8_t)n;
    f->pickOwner = (uint8_t)owner;
    f->pickSource = (uint8_t)source;
    f->pickMin = (uint8_t)min;
    f->pickMax = (uint8_t)max;
    f->pickValue = 0;
    f->prompt = prompt;
    if (min > 0 && valid < (unsigned)min && source != TCG_SRC_DECK)
        return Fail(f, "There aren't enough cards to choose.");
    return f->q = TCG_Q_PICK;
}

static unsigned AttackMenu(struct TcgFlow *f, const struct TcgDuel *g)
{
    const struct TcgAttack *a = &TcgDuel_Attacker(g, f->me)->attacks[f->action.attack];
    const char *labels[TCG_MAX_OPTIONS];
    uint8_t ok[TCG_MAX_OPTIONS];
    unsigned n, i;
    if (TcgDuel_IsMetronome(a) && f->action.copy)
        a = TcgDuel_MetronomeAttack(g, f->me, f->action.copy);
    n = a ? TcgDuel_AttackOptions(g, f->me, a, labels, ok) : 0;
    memset(&f->menu, 0, sizeof(f->menu));
    for (i = 0; i < n; i++)
        MenuAdd(&f->menu, labels[i], i, ok[i], "That can't be chosen.");
    f->prompt = f->steps[f->step].prompt;
    return f->q = TCG_Q_MENU;
}

/* The next step of a Trainer / Pokemon Power / attack (the board's ContinueTrainer) */
static unsigned Next(struct TcgFlow *f, const struct TcgDuel *g)
{
    const struct TcgStep *st;
    if (f->step >= f->nsteps)
        return Done(f);
    st = &f->steps[f->step];
    if (st->kind == TCG_ST_MY_SLOT || st->kind == TCG_ST_OPP_SLOT)
        return AskSlot(f, g, st->kind == TCG_ST_OPP_SLOT ? f->me ^ 1 : f->me, st->filter, st->prompt);
    if (st->kind == TCG_ST_PICK)
    {
        int n = st->max, all = st->source == TCG_SRC_TOP5 || st->source == TCG_SRC_TOP3_MINE || st->source == TCG_SRC_TOP3_THEIRS;
        if (st->source == TCG_SRC_TOP5)
            n = g->players[f->me].deckSize < 5 ? g->players[f->me].deckSize : 5;
        if (st->source == TCG_SRC_TOP7 && !n)
            n = 1;
        if (st->source == TCG_SRC_TOP3_MINE || st->source == TCG_SRC_TOP3_THEIRS)
        {
            n = g->players[st->source == TCG_SRC_TOP3_THEIRS ? f->me ^ 1 : f->me].deckSize;
            n = n < 3 ? n : 3;
        }
        return AskPick(f, g, st->source, st->filter, all ? n : st->min, n, st->prompt);
    }
    if (st->kind == TCG_ST_ATTACK_OPTION)
        return AttackMenu(f, g);
    if (st->kind == TCG_ST_STAGE)
    {
        const struct TcgCard *top = Field(g, f->me, f->action.slot);
        if (top && top->stage == 2)
        {
            memset(&f->menu, 0, sizeof(f->menu));
            MenuAdd(&f->menu, "TO STAGE 1", 2, 1, NULL);
            MenuAdd(&f->menu, "TO BASIC", 1, 1, NULL);
            f->prompt = st->prompt;
            return f->q = TCG_Q_MENU;
        }
        f->action.attack = 1;
        f->step++;
        return Next(f, g);
    }
    if (st->kind == TCG_ST_CHOICE)
    {
        const char *why;
        if (!TcgFlow_PowerChoices(g, f->me, f->action.slot, &f->menu, &why))
            return Fail(f, why ? why : "There is nothing that can be chosen.");
        f->prompt = st->prompt;
        return f->q = TCG_Q_MENU;
    }
    return Fail(f, "That can't be done.");
}

unsigned TcgFlow_Begin(struct TcgFlow *f, const struct TcgDuel *g, unsigned me, unsigned kind, unsigned index)
{
    const struct TcgPlayer *p = &g->players[me];
    char reason[96];
    memset(f, 0, sizeof(*f));
    f->me = me & 1;
    f->kind = kind;
    f->steps = sNoSteps;
    if (kind == TCG_FLOW_CONCEDE)
    {
        f->action.type = TCG_CONCEDE;
        return Done(f);
    }
    if (TcgFlow_Actor(g) != f->me)
        return Fail(f, g->phase == TCG_FINISHED ? "The duel is over." : "It isn't your turn.");
    switch (kind)
    {
    case TCG_FLOW_READY:
        if (g->phase != TCG_SETUP)
            return Fail(f, "The duel has begun.");
        if (!p->field[0].stackSize)
            return Fail(f, "Choose your Active Pokemon first.");
        f->action.type = TCG_FINISH_SETUP;
        return Done(f);
    case TCG_FLOW_PASS:
        if (g->phase != TCG_TURN)
            return Fail(f, g->phase == TCG_SETUP ? "Finish setting up your Pokemon first." : "Choose your next Active Pokemon first.");
        f->action.type = TCG_PASS;
        return Done(f);
    case TCG_FLOW_PROMOTE:
        if (g->phase != TCG_PROMOTION || !index || index >= TCG_FIELD_SIZE || !Field(g, f->me, index))
            return Fail(f, "Choose a Benched Pokemon to become your Active Pokemon.");
        f->action.type = TCG_PROMOTE;
        f->action.slot = (uint8_t)index;
        return Done(f);
    case TCG_FLOW_DISCARD:
    {
        unsigned s, n = 0;
        const struct TcgCard *c = index < TCG_FIELD_SIZE ? Field(g, f->me, index) : NULL;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            n += Field(g, f->me, s) != NULL;
        if (g->phase != TCG_TURN || !c || c->kind != TCG_TRAINER)
            return Fail(f, "That can't be discarded.");
        if (n < 2)
            return Fail(f, "It's your only Pokemon in play.");
        f->action.type = TCG_DISCARD_DOLL;
        f->action.slot = (uint8_t)index;
        return Done(f);
    }
    case TCG_FLOW_HAND:
    {
        const struct TcgCard *c;
        if (index >= p->handSize)
            return Fail(f, "That card isn't in your hand.");
        c = Tcg_Card(p->cards[p->hand[index]]);
        if (!TcgFlow_HandPlayable(g, f->me, c, reason, sizeof(reason)))
            return Fail(f, reason);
        f->action.hand = (uint8_t)index;
        f->flowCard = c->id;
        if (c->kind == TCG_ENERGY)
        {
            f->action.type = TCG_ATTACH;
            return AskSlot(f, g, f->me, TCG_F_ANY_POKEMON, "Attach it to which Pokemon?");
        }
        if (c->kind == TCG_POKEMON)
        {
            f->action.type = c->stage ? TCG_EVOLVE : TCG_PLAY_BASIC;
            return AskSlot(f, g, f->me, c->stage ? TCG_F_EVOLVE_TARGET : TCG_F_BASIC_SLOT,
                           c->stage ? "Evolve which Pokemon?" : "Put it in which space?");
        }
        f->action.type = TCG_PLAY_TRAINER;
        f->steps = TcgFlow_TrainerSteps(c->id, &f->nsteps);
        return Next(f, g);
    }
    case TCG_FLOW_ATTACK:
    {
        const struct TcgCard *c = TcgDuel_Attacker(g, f->me);
        if (g->phase != TCG_TURN || index > 1)
            return Fail(f, "You can't attack now.");
        if (!TcgFlow_AttackReady(g, f->me, index, reason, sizeof(reason)))
            return Fail(f, reason);
        f->action.type = TCG_ATTACK;
        f->action.attack = (uint8_t)index;
        f->nsteps = TcgFlow_AttackSteps(g, f->me, &f->action, &c->attacks[index], 0, f->attackSteps, f->attackPrompt);
        f->steps = f->attackSteps;
        return Next(f, g);
    }
    case TCG_FLOW_RETREAT:
        if (g->phase != TCG_TURN)
            return Fail(f, "You can't retreat now.");
        if (!TcgFlow_CanRetreat(g, f->me, reason, sizeof(reason)))
            return Fail(f, reason);
        f->action.type = TCG_RETREAT;
        return AskSlot(f, g, f->me, TCG_F_BENCH, "Switch in which Benched Pokemon?");
    case TCG_FLOW_POWER:
    {
        const struct TcgCard *c = index < TCG_FIELD_SIZE ? Field(g, f->me, index) : NULL;
        const char *block;
        unsigned power;
        if (!TcgFlow_HasPower(c))
            return Fail(f, "It has no Pokemon Power.");
        if ((block = TcgPower_Block(g, f->me, index)) != NULL)
            return Fail(f, block);
        power = TcgPower_Id(c);
        f->action.type = TCG_USE_POWER;
        f->action.slot = (uint8_t)index;
        f->steps = TcgFlow_PowerSteps(power, &f->nsteps);
        if (power == TCG_POWER_RAIN_DANCE)
        {
            /* (any Water Energy card in the hand is as good as another) */
            unsigned i;
            for (i = 0; i < p->handSize && p->cards[p->hand[i]] != TCG_CARD_WATER_ENERGY; i++)
                ;
            f->action.hand = (uint8_t)i;
        }
        return Next(f, g);
    }
    }
    return Fail(f, "That can't be done.");
}

unsigned TcgFlow_Slot(struct TcgFlow *f, const struct TcgDuel *g, unsigned slot)
{
    if (f->q != TCG_Q_SLOT)
        return f->q;
    if (slot >= TCG_FIELD_SIZE || !(f->slotMask >> slot & 1))
    {
        snprintf(f->error, sizeof(f->error), "Choose one of the highlighted spaces.");
        return f->q;
    }
    f->error[0] = 0;
    switch (f->kind)
    {
    case TCG_FLOW_POWER:
        if (f->step == 0)
            f->action.target = (uint8_t)slot;
        else
            f->action.attack = (uint8_t)slot;
        f->step++;
        return Next(f, g);
    case TCG_FLOW_ATTACK:
        f->action.target = (uint8_t)slot;
        f->step++;
        return Next(f, g);
    case TCG_FLOW_RETREAT:
        f->action.slot = (uint8_t)slot;
        if (TcgFlow_RetreatAutoPay(g, f->me, &f->action))
            return Done(f);
        {
            unsigned q, cost = TcgDuel_RetreatCost(g, f->me);
            f->action.slot = 0;   /* (the Energy on the Active Pokemon) */
            q = AskPick(f, g, TCG_SRC_MY_ATTACHED, TCG_F_ENERGY, 1, (int)cost, "Discard Energy to retreat.");
            f->action.slot = (uint8_t)slot;
            f->pickValue = (uint8_t)cost;
            return q;
        }
    case TCG_FLOW_HAND:
        if (f->action.type == TCG_PLAY_TRAINER)
        {
            if (f->steps[f->step].kind == TCG_ST_OPP_SLOT)
                f->action.target = (uint8_t)slot;
            else
                f->action.slot = (uint8_t)slot;
            f->step++;
            return Next(f, g);
        }
        f->action.slot = (uint8_t)slot;
        return Done(f);
    }
    return Fail(f, "That can't be done.");
}

unsigned TcgFlow_Pick(struct TcgFlow *f, const struct TcgDuel *g, const uint8_t *chosen, unsigned count)
{
    unsigned i, k, value = 0;
    if (f->q != TCG_Q_PICK)
        return f->q;
    f->error[0] = 0;
    for (i = 0; i < count; i++)
    {
        int found = 0;
        for (k = 0; k < f->pickCount; k++)
            found |= f->pick[k] == chosen[i] && f->pickOk[k];
        for (k = 0; k < i; k++)
            found &= chosen[k] != chosen[i];
        if (!found)
        {
            snprintf(f->error, sizeof(f->error), "That card can't be chosen here.");
            return f->q;
        }
        value += TcgFlow_EnergyValue(CardOf(g, f->pickOwner, chosen[i]));
    }
    if (f->pickValue ? value < f->pickValue : (count < f->pickMin || count > f->pickMax))
    {
        snprintf(f->error, sizeof(f->error), "%s", f->pickValue ? "Discard enough Energy to pay the Retreat Cost."
                                                   : "Choose the required number of cards.");
        return f->q;
    }
    if (f->pickValue)
    {
        f->action.selectionCount = 0;
        for (i = 0; i < count; i++)
            f->action.selections[f->action.selectionCount++] = chosen[i];
        return Done(f);
    }
    for (i = 0; i < count && f->action.selectionCount < TCG_DECK_SIZE; i++)
        f->action.selections[f->action.selectionCount++] = chosen[i];
    f->step++;
    return Next(f, g);
}

unsigned TcgFlow_Menu(struct TcgFlow *f, const struct TcgDuel *g, unsigned value)
{
    unsigned i, ok = 0;
    const struct TcgStep *st;
    if (f->q != TCG_Q_MENU)
        return f->q;
    for (i = 0; i < f->menu.count; i++)
        if (f->menu.value[i] == value)
            ok = f->menu.ok[i] ? 1 : 2;
    if (ok != 1)
    {
        snprintf(f->error, sizeof(f->error), "That can't be chosen.");
        return f->q;
    }
    f->error[0] = 0;
    st = &f->steps[f->step];
    if (st->kind == TCG_ST_STAGE)
    {
        f->action.attack = (uint8_t)value;
        f->step++;
        return Next(f, g);
    }
    if (st->kind == TCG_ST_CHOICE)
    {
        f->action.target = (uint8_t)value;
        f->step++;
        return Next(f, g);
    }
    if (st->kind == TCG_ST_ATTACK_OPTION)
    {
        /* The pick decides what comes next (Metronome's copied attack, Prophecy's deck, the side to devolve) */
        const struct TcgAttack *a = &TcgDuel_Attacker(g, f->me)->attacks[f->action.attack];
        if (TcgDuel_IsMetronome(a) && !f->action.copy)
        {
            f->action.copy = (uint8_t)(value + 1);
            a = TcgDuel_MetronomeAttack(g, f->me, f->action.copy);
            f->nsteps = TcgFlow_AttackSteps(g, f->me, &f->action, a, 0, f->attackSteps, f->attackPrompt);
        }
        else
        {
            f->action.option = (uint8_t)value;
            f->nsteps = TcgFlow_AttackSteps(g, f->me, &f->action,
                                            TcgDuel_IsMetronome(a) ? TcgDuel_MetronomeAttack(g, f->me, f->action.copy) : a,
                                            1, f->attackSteps, f->attackPrompt);
        }
        f->steps = f->attackSteps;
        f->step = 0;
        return Next(f, g);
    }
    return Fail(f, "That can't be done.");
}
