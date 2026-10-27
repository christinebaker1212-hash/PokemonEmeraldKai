/* Pokemon Powers: every one of the 228 cards' 26, as their text and the original rules
 * read (include/tcg/duel.h TCG_POWER_*). A power works unless Muk's Toxic Gas is in play
 * (it only leaves other Toxic Gases) or its Pokemon is Asleep, Confused or Paralyzed (only
 * an Active can be). Used powers are TCG_USE_POWER; the others hook into the rules. */
#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <stdio.h>
#include <string.h>

static const struct
{
    const char *effect, *name;
} sPowers[TCG_POWER_COUNT] = {
    [TCG_POWER_TOXIC_GAS] = {"MukToxicGasEffectCommands", "TOXIC GAS"},
    [TCG_POWER_CLAIRVOYANCE] = {"OmanyteClairvoyanceEffectCommands", "CLAIRVOYANCE"},
    [TCG_POWER_STRIKES_BACK] = {"MachampStrikesBackEffectCommands", "STRIKES BACK"},
    [TCG_POWER_KABUTO_ARMOR] = {"KabutoKabutoArmorEffectCommands", "KABUTO ARMOR"},
    [TCG_POWER_PREHISTORIC] = {"AerodactylPrehistoricPowerEffectCommands", "PREHISTORIC POWER"},
    [TCG_POWER_TRANSPARENCY] = {"HaunterTransparencyEffectCommands", "TRANSPARENCY"},
    [TCG_POWER_INVISIBLE_WALL] = {"MrMimeInvisibleWallEffectCommands", "INVISIBLE WALL"},
    [TCG_POWER_NEUTRALIZING_SHIELD] = {"MewNeutralizingShieldEffectCommands", "NEUTRALIZING SHIELD"},
    [TCG_POWER_RETREAT_AID] = {"DodrioRetreatAidEffectCommands", "RETREAT AID"},
    [TCG_POWER_THICK_SKINNED] = {"SnorlaxThickSkinnedEffectCommands", "THICK SKINNED"},
    [TCG_POWER_FIREGIVER] = {"MoltresFiregiverEffectCommands", "FIREGIVER"},
    [TCG_POWER_QUICKFREEZE] = {"ArticunoQuickfreezeEffectCommands", "QUICKFREEZE"},
    [TCG_POWER_PEAL_OF_THUNDER] = {"ZapdosPealOfThunderEffectCommands", "PEAL OF THUNDER"},
    [TCG_POWER_HEALING_WIND] = {"DragoniteHealingWindEffectCommands", "HEALING WIND"},
    [TCG_POWER_SOLAR_POWER] = {"VenusaurSolarPowerEffectCommands", "SOLAR POWER"},
    [TCG_POWER_ENERGY_TRANS] = {"VenusaurEnergyTransEffectCommands", "ENERGY TRANS"},
    [TCG_POWER_HEAL] = {"VileplumeHealEffectCommands", "HEAL"},
    [TCG_POWER_SHIFT] = {"VenomothShiftEffectCommands", "SHIFT"},
    [TCG_POWER_ENERGY_BURN] = {"CharizardEnergyBurnEffectCommands", "ENERGY BURN"},
    [TCG_POWER_RAIN_DANCE] = {"BlastoiseRainDanceEffectCommands", "RAIN DANCE"},
    [TCG_POWER_COWARDICE] = {"TentacoolCowardiceEffectCommands", "COWARDICE"},
    [TCG_POWER_PEEK] = {"MankeyPeekEffectCommands", "PEEK"},
    [TCG_POWER_DAMAGE_SWAP] = {"AlakazamDamageSwapEffectCommands", "DAMAGE SWAP"},
    [TCG_POWER_STRANGE_BEHAVIOR] = {"SlowbroStrangeBehaviorEffectCommands", "STRANGE BEHAVIOR"},
    [TCG_POWER_CURSE] = {"GengarCurseEffectCommands", "CURSE"},
    [TCG_POWER_STEP_IN] = {"DragoniteStepInEffectCommands", "STEP IN"},
};

unsigned TcgPower_Id(const struct TcgCard *c)
{
    unsigned i;
    if (!c || c->kind != TCG_POKEMON || c->attacks[0].category != TCG_POKEMON_POWER || !c->attacks[0].effect)
        return TCG_POWER_NONE;
    for (i = 1; i < TCG_POWER_COUNT; i++)
        if (!strcmp(c->attacks[0].effect, sPowers[i].effect))
            return i;
    return TCG_POWER_NONE;
}
const char *TcgPower_Name(unsigned power) { return power && power < TCG_POWER_COUNT ? sPowers[power].name : ""; }

/* ------------------------------------------------------------------ */
/* Does it work?                                                        */
/* ------------------------------------------------------------------ */
static int Disabled(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    /* (only the Active Pokemon can be Asleep, Confused or Paralyzed) */
    return !slot && g->players[player].field[0].condition != TCG_HEALTHY;
}
static int ToxicGas(const struct TcgDuel *g)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (TcgPower_Id(TcgDuel_Field(g, who, s)) == TCG_POWER_TOXIC_GAS && !Disabled(g, who, s))
                return 1;
    return 0;
}
unsigned TcgPower_Working(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    unsigned id;
    if (player > 1 || slot >= TCG_FIELD_SIZE)
        return TCG_POWER_NONE;
    id = TcgPower_Id(TcgDuel_Field(g, player, slot));
    if (!id || Disabled(g, player, slot) || (id != TCG_POWER_TOXIC_GAS && ToxicGas(g)))
        return TCG_POWER_NONE;
    return id;
}
int TcgPower_InPlay(const struct TcgDuel *g, unsigned player, unsigned power)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        if (player == 2 || player == who)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (TcgPower_Working(g, who, s) == power)
                    return 1;
    return 0;
}
unsigned TcgDuel_Type(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    unsigned shift = g->players[player].field[slot].shift;
    return shift ? shift - 1 : c ? c->type : TCG_COLORLESS;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */
static unsigned MaxHp(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    return !c ? 0 : c->kind == TCG_TRAINER ? 10 : c->hp;
}
static unsigned InPlay(const struct TcgDuel *g, unsigned player)
{
    unsigned s, n = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        n += g->players[player].field[s].stackSize != 0;
    return n;
}
static int GrassCard(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (g->players[player].cards[f->attached[i]] == TCG_CARD_GRASS_ENERGY)
            return (int)i;
    return -1;
}
static int WaterInHand(const struct TcgDuel *g, unsigned player)
{
    const struct TcgPlayer *p = &g->players[player];
    unsigned i;
    for (i = 0; i < p->handSize; i++)
        if (p->cards[p->hand[i]] == TCG_CARD_WATER_ENERGY)
            return (int)i;
    return -1;
}
/* Can it take one more damage counter without being Knocked Out? */
static int Room(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    return f->stackSize && f->damage + 10u < MaxHp(g, player, slot);
}
static void Cure(struct TcgDuel *g, unsigned player, unsigned slot)
{
    struct TcgFieldCard *f = &g->players[player].field[slot];
    if (!f->stackSize || (f->condition == TCG_HEALTHY && !f->poison))
        return;
    f->condition = TCG_HEALTHY;
    f->poison = 0;
    TcgDuel_Event(g, TCG_EV_STATUS, player, slot, TCG_HEALTHY, 0, 0);
}
static void PlainDamage(struct TcgDuel *g, unsigned player, unsigned slot, unsigned amount)
{
    if (!g->players[player].field[slot].stackSize || !amount)
        return;
    g->players[player].field[slot].damage += amount;
    TcgDuel_Event(g, TCG_EV_DAMAGE, player, slot, amount, 0, 0);
}
static void Announce(struct TcgDuel *g, unsigned player, unsigned slot, unsigned power)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    TcgDuel_Event(g, TCG_EV_POWER, player, slot, power, c ? c->id : 0, 0);
    TcgDuel_Log(g, "%s's %s.", c ? c->name : "?", TcgPower_Name(power));
}
static int Fail(struct TcgDuel *g, const char *message)
{
    snprintf(g->error, sizeof(g->error), "%s", message);
    return TCG_INVALID;
}

/* ------------------------------------------------------------------ */
/* Used powers                                                          */
/* ------------------------------------------------------------------ */
static int OncePerTurn(unsigned power)
{
    return power == TCG_POWER_SOLAR_POWER || power == TCG_POWER_HEAL || power == TCG_POWER_SHIFT ||
           power == TCG_POWER_PEEK || power == TCG_POWER_CURSE || power == TCG_POWER_STEP_IN;
}
const char *TcgPower_Block(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgPlayer *p = &g->players[player], *op = &g->players[player ^ 1];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned power = TcgPower_Id(TcgDuel_Field(g, player, slot)), s, t;
    if (!power)
        return "This Pokemon has no Pokemon Power.";
    if (power < TCG_POWER_FIRST_USED)
        return "This Pokemon Power works on its own.";
    if (g->phase != TCG_TURN || g->current != player)
        return "Pokemon Powers are used during your turn.";
    if (Disabled(g, player, slot))
        return "It can't be used while it's Asleep, Confused or Paralyzed.";
    if (ToxicGas(g))
        return "Muk's Toxic Gas stops every other Pokemon Power.";
    if (OncePerTurn(power) && f->powerUsed)
        return "It has been used this turn.";
    switch (power)
    {
    case TCG_POWER_SOLAR_POWER:
        if (!p->field[0].condition && !p->field[0].poison && !op->field[0].condition && !op->field[0].poison)
            return "Neither Active Pokemon is Asleep, Confused, Paralyzed or Poisoned.";
        break;
    case TCG_POWER_ENERGY_TRANS:
        for (s = 0; s < TCG_FIELD_SIZE && GrassCard(g, player, s) < 0; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "None of your Pokemon has a Grass Energy card.";
        if (InPlay(g, player) < 2)
            return "There is no other Pokemon to move it to.";
        break;
    case TCG_POWER_HEAL:
        for (s = 0; s < TCG_FIELD_SIZE && !p->field[s].damage; s++)
            ;
        if (s == TCG_FIELD_SIZE)
            return "None of your Pokemon has damage counters.";
        break;
    case TCG_POWER_SHIFT:
        for (t = 0; t < TCG_COLORLESS; t++)
        {
            unsigned who;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    if ((who != player || s != slot) && TcgDuel_Field(g, who, s) &&
                        TcgDuel_Field(g, who, s)->kind == TCG_POKEMON && TcgDuel_Type(g, who, s) == t)
                        return NULL;
        }
        return "There is no other type of Pokemon in play.";
    case TCG_POWER_ENERGY_BURN:
        if (f->burn)
            return "Its Energy is already Fire Energy this turn.";
        break;
    case TCG_POWER_RAIN_DANCE:
        if (WaterInHand(g, player) < 0)
            return "There is no Water Energy card in your hand.";
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (TcgDuel_Field(g, player, s) && TcgDuel_Field(g, player, s)->kind == TCG_POKEMON &&
                TcgDuel_Type(g, player, s) == TCG_WATER)
                return NULL;
        return "You have no Water Pokemon.";
    case TCG_POWER_COWARDICE:
        if (f->enteredTurn == g->turn)
            return "It can't be used the turn it was put into play.";
        if (InPlay(g, player) < 2)
            return "It's your only Pokemon in play.";
        break;
    case TCG_POWER_DAMAGE_SWAP:
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (p->field[s].damage)
                for (t = 0; t < TCG_FIELD_SIZE; t++)
                    if (t != s && Room(g, player, t))
                        return NULL;
        return "No damage counter can be moved.";
    case TCG_POWER_STRANGE_BEHAVIOR:
        if (!Room(g, player, slot))
            return "Slowbro can't take another damage counter.";
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (s != slot && p->field[s].damage)
                return NULL;
        return "None of your other Pokemon has damage counters.";
    case TCG_POWER_CURSE:
        if (InPlay(g, player ^ 1) < 2)
            return "Your opponent has only one Pokemon in play.";
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (op->field[s].damage)
                return NULL;
        return "None of your opponent's Pokemon has damage counters.";
    case TCG_POWER_STEP_IN:
        if (!slot)
            return "Dragonite has to be on your Bench.";
        break;
    }
    return NULL;
}
int TcgDuel_Power(struct TcgDuel *g, const struct TcgAction *a)
{
    unsigned me = g->current, slot = a->slot, power, i;
    struct TcgPlayer *p = &g->players[me], *op = &g->players[me ^ 1];
    struct TcgFieldCard *f;
    const char *blocked;
    if (slot >= TCG_FIELD_SIZE)
        return Fail(g, "Choose one of your Pokemon.");
    if ((blocked = TcgPower_Block(g, me, slot)))
        return Fail(g, blocked);
    power = TcgPower_Id(TcgDuel_Field(g, me, slot));
    f = &p->field[slot];
    switch (power)
    {
    case TCG_POWER_SOLAR_POWER:
        Announce(g, me, slot, power);
        Cure(g, me, 0);
        Cure(g, me ^ 1, 0);
        break;
    case TCG_POWER_ENERGY_TRANS:
    {
        int at;
        if (a->target >= TCG_FIELD_SIZE || a->attack >= TCG_FIELD_SIZE || a->target == a->attack ||
            !p->field[a->attack].stackSize || (at = GrassCard(g, me, a->target)) < 0)
            return Fail(g, "Move a Grass Energy card from one of your Pokemon to another.");
        Announce(g, me, slot, power);
        p->field[a->attack].attached[p->field[a->attack].attachedSize++] =
            TcgPile_Remove(p->field[a->target].attached, &p->field[a->target].attachedSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_ATTACH, me, a->attack, 0, TCG_CARD_GRASS_ENERGY, 0);
        break;
    }
    case TCG_POWER_HEAL:
        if (a->target >= TCG_FIELD_SIZE || !p->field[a->target].damage)
            return Fail(g, "Choose one of your Pokemon with damage counters.");
        Announce(g, me, slot, power);
        if (TcgDuel_Coin(g, me))
        {
            p->field[a->target].damage -= 10;
            TcgDuel_Event(g, TCG_EV_HEAL, me, a->target, 10, 0, 0);
        }
        break;
    case TCG_POWER_SHIFT:
    {
        unsigned who, s, found = 0;
        for (who = 0; who < 2; who++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                found |= (who != me || s != slot) && TcgDuel_Field(g, who, s) &&
                         TcgDuel_Field(g, who, s)->kind == TCG_POKEMON && TcgDuel_Type(g, who, s) == a->target;
        if (a->target >= TCG_COLORLESS || !found)
            return Fail(g, "Choose the type of another Pokemon in play (not Colorless).");
        Announce(g, me, slot, power);
        f->shift = (uint8_t)(a->target + 1);
        break;
    }
    case TCG_POWER_ENERGY_BURN:
        Announce(g, me, slot, power);
        f->burn = 1;
        break;
    case TCG_POWER_RAIN_DANCE:
        if (a->hand >= p->handSize || p->cards[p->hand[a->hand]] != TCG_CARD_WATER_ENERGY ||
            a->target >= TCG_FIELD_SIZE || !TcgDuel_Field(g, me, a->target) ||
            TcgDuel_Field(g, me, a->target)->kind != TCG_POKEMON || TcgDuel_Type(g, me, a->target) != TCG_WATER)
            return Fail(g, "Attach a Water Energy card from your hand to one of your Water Pokemon.");
        Announce(g, me, slot, power);
        p->field[a->target].attached[p->field[a->target].attachedSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        TcgDuel_Event(g, TCG_EV_ATTACH, me, a->target, 0, TCG_CARD_WATER_ENERGY, 0);
        break;
    case TCG_POWER_COWARDICE:
    {
        uint8_t basic = f->stack[0];
        Announce(g, me, slot, power);
        for (i = 1; i < f->stackSize; i++)
            p->discard[p->discardSize++] = f->stack[i];
        for (i = 0; i < f->attachedSize; i++)
            p->discard[p->discardSize++] = f->attached[i];
        memset(f, 0, sizeof(*f));
        p->hand[p->handSize++] = basic;
        TcgDuel_ResolveKnockouts(g); /* (from the Active spot: promote a Benched Pokemon) */
        break;
    }
    case TCG_POWER_PEEK:
    {
        unsigned card = 0;
        switch (a->target)
        {
        case TCG_PEEK_MY_DECK:
        case TCG_PEEK_THEIR_DECK:
        {
            struct TcgPlayer *q = a->target == TCG_PEEK_MY_DECK ? p : op;
            if (q->deckSize)
                card = q->cards[q->deck[q->deckSize - 1]];
            break;
        }
        case TCG_PEEK_THEIR_HAND:
            if (op->handSize)
                card = op->cards[op->hand[Tcg_RandomBelow(&g->rng, op->handSize)]];
            break;
        case TCG_PEEK_MY_PRIZE:
        case TCG_PEEK_THEIR_PRIZE:
        {
            /* (the Prizes lie face down in no order: one is as good as another) */
            struct TcgPlayer *q = a->target == TCG_PEEK_MY_PRIZE ? p : op;
            if (q->prizeSize)
                card = q->cards[q->prizes[Tcg_RandomBelow(&g->rng, q->prizeSize)]];
            break;
        }
        default:
            return Fail(g, "Choose what to look at.");
        }
        if (!card)
            return Fail(g, "There's no card there to look at.");
        Announce(g, me, slot, power);
        TcgDuel_Event(g, TCG_EV_PEEK, me, slot, a->target, card, 0);
        break;
    }
    case TCG_POWER_DAMAGE_SWAP:
        if (a->target >= TCG_FIELD_SIZE || a->attack >= TCG_FIELD_SIZE || a->target == a->attack ||
            !p->field[a->target].damage || !Room(g, me, a->attack))
            return Fail(g, "Move a damage counter to a Pokemon it won't Knock Out.");
        Announce(g, me, slot, power);
        p->field[a->target].damage -= 10;
        p->field[a->attack].damage += 10;
        TcgDuel_Event(g, TCG_EV_HEAL, me, a->target, 10, 0, 0);
        TcgDuel_Event(g, TCG_EV_DAMAGE, me, a->attack, 10, 0, 0);
        break;
    case TCG_POWER_STRANGE_BEHAVIOR:
        if (a->target >= TCG_FIELD_SIZE || a->target == slot || !p->field[a->target].damage)
            return Fail(g, "Choose another of your Pokemon with damage counters.");
        Announce(g, me, slot, power);
        p->field[a->target].damage -= 10;
        f->damage += 10;
        TcgDuel_Event(g, TCG_EV_HEAL, me, a->target, 10, 0, 0);
        TcgDuel_Event(g, TCG_EV_DAMAGE, me, slot, 10, 0, 0);
        break;
    case TCG_POWER_CURSE:
        if (a->target >= TCG_FIELD_SIZE || a->attack >= TCG_FIELD_SIZE || a->target == a->attack ||
            !op->field[a->target].damage || !op->field[a->attack].stackSize)
            return Fail(g, "Move a damage counter from one of their Pokemon to another.");
        Announce(g, me, slot, power);
        op->field[a->target].damage -= 10;
        op->field[a->attack].damage += 10;
        TcgDuel_Event(g, TCG_EV_HEAL, me ^ 1, a->target, 10, 0, 0);
        TcgDuel_Event(g, TCG_EV_DAMAGE, me ^ 1, a->attack, 10, 0, 0);
        TcgDuel_ResolveKnockouts(g); /* "(even if it would Knock Out the other Pokemon)" */
        break;
    case TCG_POWER_STEP_IN:
    {
        struct TcgFieldCard temp = p->field[0];
        Announce(g, me, slot, power);
        p->field[0] = *f;
        *f = temp;
        /* The one that went to the Bench loses what attacks did to it */
        f->condition = TCG_HEALTHY;
        f->poison = f->barrier = f->noRetreat = f->noAttack = f->guard = f->weaken = f->charged = 0;
        f->effectBy = f->burn = f->shift = 0;
        TcgDuel_Event(g, TCG_EV_SWITCH, me, slot, 0, 0, 0);
        slot = 0; /* (Dragonite is the Active now: it's the one that used its power) */
        break;
    }
    default:
        return Fail(g, "This Pokemon Power can't be used.");
    }
    if (p->field[slot].stackSize && OncePerTurn(power))
        p->field[slot].powerUsed = 1;
    TcgPower_Sanitize(g);
    return TCG_OK;
}

/* ------------------------------------------------------------------ */
/* Into play                                                            */
/* ------------------------------------------------------------------ */
void TcgPower_OnPlay(struct TcgDuel *g, unsigned player, unsigned slot)
{
    struct TcgPlayer *p = &g->players[player], *op = &g->players[player ^ 1];
    unsigned power = TcgPower_Working(g, player, slot), i, n, s, who;
    if (g->phase == TCG_SETUP || g->current != player)
        return;
    switch (power)
    {
    case TCG_POWER_FIREGIVER:
        /* 1 to 4 Fire Energy cards (chosen at random) from the deck into the hand */
        Announce(g, player, slot, power);
        n = 1 + Tcg_RandomBelow(&g->rng, 4);
        for (i = p->deckSize; i > 0 && n; i--)
            if (p->cards[p->deck[i - 1]] == TCG_CARD_FIRE_ENERGY)
            {
                p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, i - 1);
                n--;
            }
        TcgDuel_Shuffle(g, player);
        break;
    case TCG_POWER_QUICKFREEZE:
        Announce(g, player, slot, power);
        if (TcgDuel_Coin(g, player) && op->field[0].stackSize)
        {
            op->field[0].condition = TCG_PARALYZED;
            TcgPower_Sanitize(g);
            if (op->field[0].condition == TCG_PARALYZED)
                TcgDuel_Event(g, TCG_EV_STATUS, player ^ 1, 0, TCG_PARALYZED, 0, op->field[0].poison);
        }
        break;
    case TCG_POWER_PEAL_OF_THUNDER:
    {
        /* 30 to a Pokemon other than this Zapdos, at random (no Weakness, no Resistance) */
        unsigned pool[2 * TCG_FIELD_SIZE];
        Announce(g, player, slot, power);
        n = 0;
        for (who = 0; who < 2; who++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (g->players[who].field[s].stackSize && (who != player || s != slot))
                    pool[n++] = who * TCG_FIELD_SIZE + s;
        if (n)
        {
            s = pool[Tcg_RandomBelow(&g->rng, n)];
            PlainDamage(g, s / TCG_FIELD_SIZE, s % TCG_FIELD_SIZE, 30);
        }
        break;
    }
    case TCG_POWER_HEALING_WIND:
        Announce(g, player, slot, power);
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (p->field[s].damage)
            {
                n = p->field[s].damage < 20 ? p->field[s].damage : 20;
                p->field[s].damage -= n;
                TcgDuel_Event(g, TCG_EV_HEAL, player, s, n, 0, 0);
            }
        break;
    }
}

/* ------------------------------------------------------------------ */
/* What attacks do to them                                              */
/* ------------------------------------------------------------------ */
int TcgPower_AttackDamage(struct TcgDuel *g, unsigned player, unsigned slot, int amount, int defending)
{
    unsigned power = TcgPower_Working(g, player, slot);
    (void)defending;
    if (amount <= 0)
        return amount;
    if (power == TCG_POWER_INVISIBLE_WALL && amount >= 30)
    {
        Announce(g, player, slot, power);
        TcgDuel_Event(g, TCG_EV_NO_DAMAGE, player, slot, 0, 0, 0);
        return 0;
    }
    if (power == TCG_POWER_KABUTO_ARMOR)
    {
        Announce(g, player, slot, power);
        return amount / 2 / 10 * 10;
    }
    return amount;
}
int TcgPower_Immune(struct TcgDuel *g, unsigned player, unsigned slot)
{
    unsigned power = TcgPower_Working(g, player, slot);
    const struct TcgFieldCard *attacker = &g->players[g->current].field[0];
    /* Mew: nothing an opponent's evolved Pokemon does reaches it */
    if (power == TCG_POWER_NEUTRALIZING_SHIELD && player != g->current && attacker->stackSize > 1)
    {
        Announce(g, player, slot, power);
        return 1;
    }
    /* Haunter: a coin for each attack that does something to it */
    if (power == TCG_POWER_TRANSPARENCY)
    {
        Announce(g, player, slot, power);
        return (int)TcgDuel_Coin(g, player);
    }
    return 0;
}
void TcgPower_Sanitize(struct TcgDuel *g)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            struct TcgFieldCard *f = &g->players[who].field[s];
            const struct TcgCard *c = TcgDuel_Field(g, who, s);
            /* Snorlax can't become Asleep, Confused, Paralyzed or Poisoned; nor can
             * Clefairy Doll and Mysterious Fossil (their text) */
            if (c && (c->kind == TCG_TRAINER || TcgPower_Working(g, who, s) == TCG_POWER_THICK_SKINNED ||
                      (TcgPower_Id(c) == TCG_POWER_THICK_SKINNED && !ToxicGas(g))))
            {
                f->condition = TCG_HEALTHY;
                f->poison = 0;
            }
        }
}
void TcgPower_StrikesBack(struct TcgDuel *g, const uint16_t before[TCG_FIELD_SIZE], unsigned activeCondition)
{
    unsigned them = g->current ^ 1, s;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        const struct TcgFieldCard *f = &g->players[them].field[s];
        /* (it doesn't work if Machamp was already Asleep, Confused or Paralyzed) */
        if (TcgPower_Id(TcgDuel_Field(g, them, s)) != TCG_POWER_STRIKES_BACK || ToxicGas(g) ||
            (!s && activeCondition != TCG_HEALTHY) || f->damage <= before[s])
            continue;
        Announce(g, them, s, TCG_POWER_STRIKES_BACK);
        PlainDamage(g, g->current, 0, 10);
    }
}
unsigned TcgPower_RetreatAid(const struct TcgDuel *g, unsigned player)
{
    unsigned s, n = 0;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        n += TcgPower_Working(g, player, s) == TCG_POWER_RETREAT_AID;
    return n;
}
