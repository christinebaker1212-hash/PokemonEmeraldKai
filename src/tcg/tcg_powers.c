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
    /* Pokemon Card GB2's */
    [TCG_POWER_GREEN_SHIELD] = {"MetapodGreenShieldEffectCommands", "GREEN SHIELD"},
    [TCG_POWER_HAY_FEVER] = {"DarkVileplumeHayFeverEffectCommands", "HAY FEVER"},
    [TCG_POWER_STICKY_GOO] = {"DarkMukStickyGooEffectCommands", "STICKY GOO"},
    [TCG_POWER_FINAL_BEAM] = {"FinalBeamEffectCommands", "FINAL BEAM"},
    [TCG_POWER_AURORA_VEIL] = {"ArticunoAuroraVeilEffectCommands", "AURORA VEIL"},
    [TCG_POWER_GO_UNDERGROUND] = {"DugtrioGoUndergroundEffectCommands", "GO UNDERGROUND"},
    [TCG_POWER_SINKHOLE] = {"DarkDugtrioSinkholeEffectCommands", "SINKHOLE"},
    [TCG_POWER_FRENZY] = {"DarkPrimeapeFrenzyEffectCommands", "FRENZY"},
    [TCG_POWER_PUPPET_MASTER] = {"HypnoPuppetMasterEffectCommands", "PUPPET MASTER"},
    [TCG_POWER_DAMPENING_SHIELD] = {"MrMimeDampeningShieldEffectCommands", "DAMPENING SHIELD"},
    [TCG_POWER_DARK_WAVE] = {"GRsMewtwoDarkWaveEffectCommands", "DARK WAVE"},
    [TCG_POWER_DARKNESS_VEIL] = {"DarkClefableDarknessVeilEffectCommands", "DARKNESS VEIL"},
    [TCG_POWER_SNORLAX_GUARD] = {"SnorlaxGuardEffectCommands", "GUARD"},
    [TCG_POWER_VINE_PULL] = {"DarkIvysaurVinePullEffectCommands", "VINE PULL"},
    [TCG_POWER_SNEAK_ATTACK] = {"DarkGolbatSneakAttackEffectCommands", "SNEAK ATTACK"},
    [TCG_POWER_REEL_IN] = {"DarkSlowbroReelInEffectCommands", "REEL IN"},
    [TCG_POWER_POWER_OF_DARKNESS] = {"GengarPowerOfDarknessEffectCommands", "POWER OF DARKNESS"},
    [TCG_POWER_SUMMON_MINIONS] = {"DarkDragoniteSummonMinionsEffectCommands", "SUMMON MINIONS"},
    [TCG_POWER_STIFFEN] = {"KakunaPoisonFluidEffectCommands", "STIFFEN"},
    [TCG_POWER_POLLEN_STENCH] = {"DarkGloomPollenStenchEffectCommands", "POLLEN STENCH"},
    [TCG_POWER_POISON_MIST] = {"WeezingPoisonMistEffectCommands", "POISON MIST"},
    [TCG_POWER_GATHER_FIRE] = {"CharmanderGatherFireEffectCommands", "GATHER FIRE"},
    [TCG_POWER_REBIRTH] = {"DarkStarmieRebirthEffectCommands", "REBIRTH"},
    [TCG_POWER_PREHISTORIC_DREAM] = {"OmanytePrehistoricDreamEffectCommands", "PREHISTORIC DREAM"},
    [TCG_POWER_FOSSIL_GUIDANCE] = {"OmanyteFossilGuidanceEffectCommands", "FOSSIL GUIDANCE"},
    [TCG_POWER_FOSSILIZE] = {"KabutoFossilizeEffectCommands", "FOSSILIZE"},
    [TCG_POWER_MATTER_EXCHANGE] = {"DarkKadabraMatterExchangeEffectCommands", "MATTER EXCHANGE"},
    [TCG_POWER_PLAY_TRICKS] = {"DarkGengarPlayTricksEffectCommands", "PLAY TRICKS"},
    [TCG_POWER_LONG_DISTANCE_HYPNOSIS] = {"DrowzeeLongDistanceHypnosisEffectCommands", "LONG-DISTANCE HYPNOSIS"},
    [TCG_POWER_TRICKERY] = {"RattataTrickeryEffectCommands", "TRICKERY"},
    [TCG_POWER_HELPING_HAND] = {"WigglytuffHelpingHandEffectCommands", "HELPING HAND"},
    [TCG_POWER_EVOLUTIONARY_LIGHT] = {"DarkDragonairEvolutionaryLightEffectCommands", "EVOLUTIONARY LIGHT"},
    [TCG_POWER_SPECIAL_DELIVERY] = {"DragoniteSpecialDeliveryEffectCommands", "SPECIAL DELIVERY"},
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
    /* GB2's: Goop Gas Attack (every power), Dark Arbok's Stare (that Pokemon's) */
    if (g->goopGasUntil >= g->turn || g->players[player].field[slot].powerOffUntil >= g->turn)
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
static int FireCard(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (g->players[player].cards[f->attached[i]] == TCG_CARD_FIRE_ENERGY)
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
/* Pokemon Card GB2's powers                                            */
/* ------------------------------------------------------------------ */
static int IsDarkCard(const struct TcgCard *c)
{
    return c && c->kind == TCG_POKEMON && (c->dark || !strncmp(c->name, "Dark ", 5));
}
/* An evolved Pokemon on a Mysterious Fossil (Omanyte, Kabuto, Aerodactyl and theirs) */
static int FromFossil(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    return f->stackSize > 1 && g->players[player].cards[f->stack[0]] == TCG_CARD_MYSTERIOUS_FOSSIL;
}
static int OnBench(const struct TcgDuel *g, unsigned player, unsigned power)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        if (player == 2 || player == who)
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (TcgPower_Working(g, who, s) == power)
                    return 1;
    return 0;
}
int TcgPower_NoWeakness(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    /* Mr. Mime's Dampening Shield (on a Bench); Metapod's Green Shield for its side's Grass Pokemon */
    return OnBench(g, 2, TCG_POWER_DAMPENING_SHIELD) ||
           (TcgDuel_Type(g, player, slot) == TCG_GRASS && TcgPower_InPlay(g, player, TCG_POWER_GREEN_SHIELD));
}
int TcgPower_NoResistance(const struct TcgDuel *g) { return OnBench(g, 2, TCG_POWER_DAMPENING_SHIELD); }
int TcgPower_NoTrainers(const struct TcgDuel *g) { return TcgPower_InPlay(g, 2, TCG_POWER_HAY_FEVER); }
unsigned TcgDuel_RetreatCost(const struct TcgDuel *g, unsigned player)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, 0);
    unsigned cost = c && c->kind == TCG_POKEMON ? c->retreat : 0, aid = TcgPower_RetreatAid(g, player);
    /* Dark Muk's Sticky Goo, the other side's Active: 2 more */
    if (TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_STICKY_GOO)
        cost += 2;
    return cost > aid ? cost - aid : 0;
}
const char *TcgPower_RetreatBlock(const struct TcgDuel *g, unsigned player)
{
    const struct TcgFieldCard *snorlax = &g->players[player ^ 1].field[0];
    if (TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_SNORLAX_GUARD && !snorlax->poison)
        return "Snorlax's Guard: the Defending Pokemon can't retreat.";
    return NULL;
}
void TcgPower_AfterRetreat(struct TcgDuel *g, unsigned player, unsigned benched)
{
    struct TcgPlayer *op = &g->players[player ^ 1];
    unsigned s;
    /* Dark Dugtrio's Sinkhole: they flip, tails 20 to the Pokemon that retreated */
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (TcgPower_Working(g, player ^ 1, s) == TCG_POWER_SINKHOLE)
        {
            Announce(g, player ^ 1, s, TCG_POWER_SINKHOLE);
            if (!TcgDuel_Coin(g, player))
                PlainDamage(g, player, benched, 20);
            break;
        }
    /* Dark Ivysaur's Vine Pull: as it retreats, one of their Benched Pokemon (at random) comes out */
    if (TcgPower_Working(g, player, benched) == TCG_POWER_VINE_PULL && op->field[0].stackSize)
    {
        unsigned slots[TCG_FIELD_SIZE], n = 0;
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (op->field[s].stackSize)
                slots[n++] = s;
        if (n)
        {
            struct TcgFieldCard temp = op->field[0];
            s = slots[Tcg_RandomBelow(&g->rng, n)];
            Announce(g, player, benched, TCG_POWER_VINE_PULL);
            op->field[0] = op->field[s];
            op->field[s] = temp;
            op->field[s].condition = TCG_HEALTHY;
            op->field[s].poison = op->field[s].barrier = op->field[s].noRetreat = op->field[s].noAttack = 0;
            op->field[s].guard = op->field[s].weaken = op->field[s].charged = op->field[s].effectBy = 0;
            op->field[s].burn = op->field[s].shift = op->field[s].disabled = 0;
            op->field[s].weakAs = op->field[s].resistAs = op->field[s].lockTurn = op->field[s].lockAttack = 0;
            TcgDuel_Event(g, TCG_EV_SWITCH, player ^ 1, s, 0, 0, 0);
        }
    }
}
int TcgPower_Frenzy(const struct TcgDuel *g, unsigned player)
{
    /* (Dark Primeape: it works while Confused, the one condition it's about) */
    const struct TcgFieldCard *f = &g->players[player].field[0];
    return f->condition == TCG_CONFUSED && TcgPower_Id(TcgDuel_Field(g, player, 0)) == TCG_POWER_FRENZY &&
           !ToxicGas(g) && g->goopGasUntil < g->turn && f->powerOffUntil < g->turn;
}
int TcgPower_AttackBonus(struct TcgDuel *g, int amount)
{
    unsigned me = g->current, s;
    int bonus = 0;
    if (amount <= 0)
        return 0;
    if (TcgPower_Frenzy(g, me))
    {
        Announce(g, me, 0, TCG_POWER_FRENZY);
        bonus += 30;
    }
    /* GR's Mewtwo's Dark Wave: a Dark Pokemon's attack, heads then heads 20 / tails 10 more */
    if (IsDarkCard(TcgDuel_Field(g, me, 0)))
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (TcgPower_Working(g, me, s) == TCG_POWER_DARK_WAVE)
            {
                Announce(g, me, s, TCG_POWER_DARK_WAVE);
                if (TcgDuel_Coin(g, me))
                    bonus += TcgDuel_Coin(g, me) ? 20 : 10;
                break;
            }
    /* Omanyte's Prehistoric Dream (heads this turn): a Pokemon that evolved from Mysterious Fossil, 10 more */
    if (FromFossil(g, me, 0))
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (g->players[me].field[s].fossilBoost)
            {
                bonus += 10;
                break;
            }
    return bonus;
}
void TcgPower_KnockedOut(struct TcgDuel *g, unsigned player, unsigned slot)
{
    /* Dark Gyarados's Final Beam: heads, 20 for each Water Energy on it to the attacker (Weakness and Resistance) */
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    unsigned water = 0, i, me = g->current;
    int amount;
    if (slot || player == me || TcgPower_Working(g, player, slot) != TCG_POWER_FINAL_BEAM ||
        !g->players[me].field[0].stackSize)
        return;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(g->players[player].cards[f->attached[i]]);
        water += c->kind == TCG_ENERGY && c->id == TCG_CARD_WATER_ENERGY;
    }
    Announce(g, player, slot, TCG_POWER_FINAL_BEAM);
    if (!water || !TcgDuel_Coin(g, player))
        return;
    amount = 20 * (int)water;
    if (TcgDuel_Weakness(g, me, 0) == TCG_WATER)
        amount *= 2;
    if (TcgDuel_Resistance(g, me, 0) == TCG_WATER)
        amount -= 30;
    if (amount > 0)
        PlainDamage(g, me, 0, (unsigned)amount);
}
unsigned TcgPower_PuppetSlot(const struct TcgDuel *g, unsigned player)
{
    unsigned s;
    const struct TcgCard *c = TcgDuel_Field(g, player, 0);
    if (!c || c->id != TCG_CARD_CLEFAIRY_DOLL)
        return TCG_NONE;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        if (TcgPower_Working(g, player, s) == TCG_POWER_PUPPET_MASTER)
            return s;
    return TCG_NONE;
}

/* ------------------------------------------------------------------ */
/* Used powers                                                          */
/* ------------------------------------------------------------------ */
static int OncePerTurn(unsigned power)
{
    return power == TCG_POWER_SOLAR_POWER || power == TCG_POWER_HEAL || power == TCG_POWER_SHIFT ||
           power == TCG_POWER_PEEK || power == TCG_POWER_CURSE || power == TCG_POWER_STEP_IN ||
           (power >= TCG_POWER_STIFFEN && power != TCG_POWER_REBIRTH); /* (GB2's: all but Rebirth) */
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
    if (!TcgPower_Working(g, player, slot))
        return "Its Pokemon Power isn't working now.";
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
    /* GB2's */
    case TCG_POWER_POLLEN_STENCH:
    case TCG_POWER_LONG_DISTANCE_HYPNOSIS:
        if (!op->field[0].stackSize)
            return "There's no Defending Pokemon.";
        break;
    case TCG_POWER_GATHER_FIRE:
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (s != slot && FireCard(g, player, s) >= 0)
                return NULL;
        return "None of your other Pokemon has a Fire Energy card.";
    case TCG_POWER_REBIRTH:
        for (s = 0; s < p->deckSize; s++)
            if (!strcmp(Tcg_Card(p->cards[p->deck[s]])->name, "Staryu"))
                return NULL;
        return "There's no Staryu in your deck.";
    case TCG_POWER_FOSSIL_GUIDANCE:
        for (s = 0; s < p->discardSize; s++)
            if (p->cards[p->discard[s]] == TCG_CARD_MYSTERIOUS_FOSSIL)
                return NULL;
        return "There's no Mysterious Fossil in your discard pile.";
    case TCG_POWER_FOSSILIZE:
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (FromFossil(g, player, s))
                return NULL;
        return "None of your Pokemon evolved from a Mysterious Fossil.";
    case TCG_POWER_MATTER_EXCHANGE:
        if (!p->handSize)
            return "There's no card in your hand to discard.";
        break;
    case TCG_POWER_PLAY_TRICKS:
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (op->field[s].damage)
                return NULL;
        return "None of your opponent's Pokemon has damage counters.";
    case TCG_POWER_TRICKERY:
        if (!p->prizeSize || !p->deckSize)
            return "There's no Prize card or no deck to switch.";
        break;
    case TCG_POWER_HELPING_HAND:
        if (!slot)
            return "Wigglytuff has to be on your Bench.";
        if (!p->field[0].condition && !p->field[0].poison)
            return "Your Active Pokemon has no Special Condition.";
        break;
    case TCG_POWER_EVOLUTIONARY_LIGHT:
        for (s = 0; s < p->deckSize; s++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->deck[s]]);
            if (c->kind == TCG_POKEMON && c->stage)
                return NULL;
        }
        return "There's no Evolution card in your deck.";
    case TCG_POWER_SPECIAL_DELIVERY:
        if (!p->deckSize)
            return "Your deck is empty.";
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
    /* GB2's */
    case TCG_POWER_STIFFEN:
        Announce(g, me, slot, power);
        if (TcgDuel_Coin(g, me))
            f->barrier = 1;
        break;
    case TCG_POWER_POLLEN_STENCH:
    case TCG_POWER_LONG_DISTANCE_HYPNOSIS:
    {
        unsigned who, condition = power == TCG_POWER_POLLEN_STENCH ? TCG_CONFUSED : TCG_ASLEEP;
        Announce(g, me, slot, power);
        who = TcgDuel_Coin(g, me) ? me ^ 1 : me;
        if (g->players[who].field[0].stackSize)
        {
            g->players[who].field[0].condition = (uint8_t)condition;
            TcgPower_Sanitize(g);
            TcgDuel_Event(g, TCG_EV_STATUS, who, 0, g->players[who].field[0].condition, 0, g->players[who].field[0].poison);
        }
        break;
    }
    case TCG_POWER_POISON_MIST:
        Announce(g, me, slot, power);
        if (TcgDuel_Coin(g, me))
        {
            p->poisonMistUntil = (uint16_t)(g->turn + 2);
            TcgDuel_Log(g, "Poison does 20 damage until the end of the next turn but one.");
        }
        break;
    case TCG_POWER_GATHER_FIRE:
    {
        int at;
        if (a->target >= TCG_FIELD_SIZE || a->target == slot || (at = FireCard(g, me, a->target)) < 0)
            return Fail(g, "Take a Fire Energy card from another of your Pokemon.");
        Announce(g, me, slot, power);
        f->attached[f->attachedSize++] = TcgPile_Remove(p->field[a->target].attached, &p->field[a->target].attachedSize,
                                                        (unsigned)at);
        TcgDuel_Event(g, TCG_EV_ATTACH, me, slot, 0, TCG_CARD_FIRE_ENERGY, 0);
        break;
    }
    case TCG_POWER_REBIRTH:
    {
        uint8_t staryu[TCG_DECK_SIZE];
        unsigned n = 0, pick;
        for (i = 0; i < p->deckSize; i++)
            if (!strcmp(Tcg_Card(p->cards[p->deck[i]])->name, "Staryu"))
                staryu[n++] = (uint8_t)i;
        Announce(g, me, slot, power);
        pick = staryu[Tcg_RandomBelow(&g->rng, n)];
        TcgDuel_DiscardField(g, me, slot);
        f->stack[f->stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, pick);
        f->enteredTurn = g->turn;
        TcgDuel_Event(g, TCG_EV_PLAY, me, slot, 0, p->cards[f->stack[0]], 0);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_POWER_PREHISTORIC_DREAM:
        Announce(g, me, slot, power);
        if (TcgDuel_Coin(g, me))
            f->fossilBoost = 1;
        break;
    case TCG_POWER_FOSSIL_GUIDANCE:
        Announce(g, me, slot, power);
        if (TcgDuel_Coin(g, me))
            for (i = 0; i < p->discardSize; i++)
                if (p->cards[p->discard[i]] == TCG_CARD_MYSTERIOUS_FOSSIL)
                {
                    p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, i);
                    TcgDuel_Event(g, TCG_EV_DRAW, me, 0, 1, 0, 0);
                    break;
                }
        break;
    case TCG_POWER_FOSSILIZE:
    {
        struct TcgFieldCard *t;
        if (a->target >= TCG_FIELD_SIZE || !FromFossil(g, me, a->target))
            return Fail(g, "Choose one of your Pokemon that evolved from a Mysterious Fossil.");
        Announce(g, me, slot, power);
        if (!TcgDuel_Coin(g, me))
            break;
        t = &p->field[a->target];
        while (t->stackSize > 1)
            p->hand[p->handSize++] = t->stack[--t->stackSize];
        t->condition = TCG_HEALTHY;
        t->poison = 0;
        TcgDuel_Event(g, TCG_EV_DEVOLVE, me, a->target, 0, p->cards[t->stack[0]], 0);
        break;
    }
    case TCG_POWER_MATTER_EXCHANGE:
    {
        int at = -1;
        for (i = 0; i < p->handSize; i++)
            if (a->selectionCount && p->hand[i] == a->selections[0])
                at = (int)i;
        if (at < 0)
            return Fail(g, "Choose a card in your hand to discard.");
        Announce(g, me, slot, power);
        p->discard[p->discardSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_DRAW, me, 0, TcgDuel_Draw(g, me, 1), 0, 0);
        break;
    }
    case TCG_POWER_PLAY_TRICKS:
    {
        /* every damage counter on their Pokemon put back at random, none Knocked Out */
        unsigned counters = 0, s, room[TCG_FIELD_SIZE], n;
        Announce(g, me, slot, power);
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            counters += op->field[s].damage / 10;
            op->field[s].damage = 0;
        }
        while (counters)
        {
            for (s = 0, n = 0; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize && op->field[s].damage + 10u < MaxHp(g, me ^ 1, s))
                    room[n++] = s;
            if (!n)
                break;
            op->field[room[Tcg_RandomBelow(&g->rng, n)]].damage += 10;
            counters--;
        }
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (op->field[s].stackSize)
                TcgDuel_Event(g, TCG_EV_DAMAGE, me ^ 1, s, 0, 0, 0);
        break;
    }
    case TCG_POWER_TRICKERY:
    {
        unsigned k = Tcg_RandomBelow(&g->rng, p->prizeSize);
        uint8_t prize = p->prizes[k];
        Announce(g, me, slot, power);
        p->prizes[k] = p->deck[p->deckSize - 1];
        p->deck[p->deckSize - 1] = prize;
        break;
    }
    case TCG_POWER_HELPING_HAND:
        Announce(g, me, slot, power);
        if (TcgDuel_Coin(g, me))
        {
            if (p->field[0].condition)
                p->field[0].condition = TCG_HEALTHY;
            else
                p->field[0].poison = 0;
            TcgDuel_Event(g, TCG_EV_STATUS, me, 0, p->field[0].condition, 0, p->field[0].poison);
        }
        break;
    case TCG_POWER_EVOLUTIONARY_LIGHT:
    {
        int at = -1;
        for (i = 0; i < p->deckSize; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->deck[i]]);
            if (c->kind == TCG_POKEMON && c->stage && (a->selectionCount ? p->deck[i] == a->selections[0] : at < 0))
                at = (int)i;
        }
        if (at < 0)
            return Fail(g, "Choose an Evolution card in your deck.");
        Announce(g, me, slot, power);
        p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_DRAW, me, 0, 1, 0, 0);
        TcgDuel_Log(g, "%s to the hand.", Tcg_Card(p->cards[p->hand[p->handSize - 1]])->name);
        TcgDuel_Shuffle(g, me);
        break;
    }
    case TCG_POWER_SPECIAL_DELIVERY:
    {
        int at = -1;
        Announce(g, me, slot, power);
        TcgDuel_Event(g, TCG_EV_DRAW, me, 0, TcgDuel_Draw(g, me, 1), 0, 0);
        for (i = 0; i < p->handSize; i++)
            if (a->selectionCount && p->hand[i] == a->selections[0])
                at = (int)i;
        if (at < 0)
            at = (int)p->handSize - 1; /* (the card drawn) */
        p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, (unsigned)at);
        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
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
    /* GB2's, played from the hand: the choices made for the player */
    case TCG_POWER_SNEAK_ATTACK:
    {
        /* 10 to the one of theirs with the least HP left (no Weakness or Resistance) */
        unsigned best = TCG_NONE;
        if (!g->fromHand)
            break;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (op->field[s].stackSize &&
                (best == TCG_NONE || MaxHp(g, player ^ 1, s) - op->field[s].damage <
                                         MaxHp(g, player ^ 1, best) - op->field[best].damage))
                best = s;
        if (best != TCG_NONE)
        {
            Announce(g, player, slot, power);
            PlainDamage(g, player ^ 1, best, 10);
            TcgDuel_ResolveKnockouts(g);
        }
        break;
    }
    case TCG_POWER_REEL_IN:
        /* up to 3 Pokemon cards from the discard pile into the hand (the last ones discarded) */
        if (!g->fromHand)
            break;
        Announce(g, player, slot, power);
        for (i = p->discardSize, n = 0; i > 0 && n < 3; i--)
            if (Tcg_Card(p->cards[p->discard[i - 1]])->kind == TCG_POKEMON)
            {
                p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, i - 1);
                n++;
            }
        if (n)
            TcgDuel_Event(g, TCG_EV_DRAW, player, 0, n, 0, 0);
        break;
    case TCG_POWER_POWER_OF_DARKNESS:
    {
        /* heads: the one of theirs with the most cards on it back to their hand (not their only Pokemon) */
        unsigned best = TCG_NONE, k;
        if (!g->fromHand || InPlay(g, player ^ 1) < 2)
            break;
        Announce(g, player, slot, power);
        if (!TcgDuel_Coin(g, player))
            break;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (op->field[s].stackSize && (best == TCG_NONE || op->field[s].attachedSize + op->field[s].stackSize >
                                                                   op->field[best].attachedSize + op->field[best].stackSize))
                best = s;
        if (best == TCG_NONE || (best ? TcgPower_Immune(g, player ^ 1, best) : 0))
            break;
        {
            struct TcgFieldCard *t = &op->field[best];
            unsigned card = op->cards[t->stack[t->stackSize - 1]];
            for (k = 0; k < t->stackSize; k++)
                op->hand[op->handSize++] = t->stack[k];
            for (k = 0; k < t->attachedSize; k++)
                op->hand[op->handSize++] = t->attached[k];
            memset(t, 0, sizeof(*t));
            TcgDuel_Event(g, TCG_EV_TO_HAND, player ^ 1, best, 0, card, 0);
        }
        TcgDuel_ResolveKnockouts(g); /* (their Active: they choose a new one) */
        break;
    }
    case TCG_POWER_SUMMON_MINIONS:
        /* up to 2 Basic Pokemon from the deck onto the Bench (the strongest) */
        if (!g->fromHand)
            break;
        Announce(g, player, slot, power);
        for (n = 0; n < 2; n++)
        {
            int best = -1;
            unsigned empty = TCG_NONE;
            for (s = 1; s < TCG_FIELD_SIZE && empty == TCG_NONE; s++)
                if (!p->field[s].stackSize)
                    empty = s;
            if (empty == TCG_NONE)
                break;
            for (i = 0; i < p->deckSize; i++)
            {
                const struct TcgCard *c = Tcg_Card(p->cards[p->deck[i]]);
                if (c->kind == TCG_POKEMON && !c->stage &&
                    (best < 0 || c->hp > Tcg_Card(p->cards[p->deck[best]])->hp))
                    best = (int)i;
            }
            if (best < 0)
                break;
            memset(&p->field[empty], 0, sizeof(p->field[empty]));
            p->field[empty].stack[p->field[empty].stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)best);
            p->field[empty].enteredTurn = g->turn;
            TcgDuel_Event(g, TCG_EV_PLAY, player, empty, 0, p->cards[p->field[empty].stack[0]], 0);
        }
        TcgDuel_Shuffle(g, player);
        break;
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
    /* Dark Clefable's Darkness Veil: a Dark Active Pokemon takes 0, 10 or 20 less (at random) */
    if (defending && !slot && player != g->current && IsDarkCard(TcgDuel_Field(g, player, 0)))
    {
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (TcgPower_Working(g, player, s) == TCG_POWER_DARKNESS_VEIL)
            {
                Announce(g, player, s, TCG_POWER_DARKNESS_VEIL);
                amount -= 10 * (int)Tcg_RandomBelow(&g->rng, 3);
                break;
            }
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
    /* GB2's: Dugtrio's Go Underground on the Bench (every attack's damage); Articuno's Aurora Veil as the Active
     * (their attacks on its Bench) */
    if (slot && power == TCG_POWER_GO_UNDERGROUND)
        return 1;
    if (slot && player != g->current && TcgPower_Working(g, player, 0) == TCG_POWER_AURORA_VEIL)
        return 1;
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
