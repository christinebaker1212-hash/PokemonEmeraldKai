// Moemon Kai: Gen 4's battle rules on top of Kai's (Gen 3) engine. See include/battle_gen4.h.
// Numbers follow Diamond / Pearl / Platinum unless a comment says otherwise.
#include "global.h"
#include "battle.h"
#include "battle_gen4.h"
#include "battle_main.h"
#include "battle_anim.h"
#include "util.h"
#include "battle_util.h"
#include "battle_scripts.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "item.h"
#include "mp.h"
#include "pokemon.h"
#include "random.h"
#include "constants/abilities.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_gen4.h"
#include "constants/battle_string_ids.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"

struct Gen4Battler gGen4Battlers[MAX_BATTLERS_COUNT];
struct Gen4Side gGen4Sides[NUM_BATTLE_SIDES];
struct Gen4Field gGen4Field;

static const u16 sPunchingMoves[] =
{
    MOVE_BULLET_PUNCH, MOVE_COMET_PUNCH, MOVE_DIZZY_PUNCH, MOVE_DRAIN_PUNCH, MOVE_DYNAMIC_PUNCH, MOVE_FIRE_PUNCH,
    MOVE_FOCUS_PUNCH, MOVE_HAMMER_ARM, MOVE_ICE_PUNCH, MOVE_MACH_PUNCH, MOVE_MEGA_PUNCH, MOVE_METEOR_MASH,
    MOVE_SHADOW_PUNCH, MOVE_SKY_UPPERCUT, MOVE_THUNDER_PUNCH,
};

static const u16 sRecoilMoves[] =   // RECKLESS
{
    MOVE_DOUBLE_EDGE, MOVE_SUBMISSION, MOVE_TAKE_DOWN, MOVE_VOLT_TACKLE, MOVE_BRAVE_BIRD, MOVE_FLARE_BLITZ,
    MOVE_WOOD_HAMMER, MOVE_HEAD_SMASH, MOVE_JUMP_KICK, MOVE_HI_JUMP_KICK,
};

// abilities MOLD BREAKER ignores (the ones that change how a move lands on their holder)
static const u16 sBreakableAbilities[] =
{
    ABILITY_BATTLE_ARMOR, ABILITY_SHELL_ARMOR, ABILITY_STURDY, ABILITY_DAMP, ABILITY_LIMBER, ABILITY_SAND_VEIL,
    ABILITY_VOLT_ABSORB, ABILITY_WATER_ABSORB, ABILITY_OBLIVIOUS, ABILITY_INSOMNIA, ABILITY_IMMUNITY,
    ABILITY_FLASH_FIRE, ABILITY_SHIELD_DUST, ABILITY_OWN_TEMPO, ABILITY_SUCTION_CUPS, ABILITY_WONDER_GUARD,
    ABILITY_LEVITATE, ABILITY_CLEAR_BODY, ABILITY_LIGHTNING_ROD, ABILITY_INNER_FOCUS, ABILITY_MAGMA_ARMOR,
    ABILITY_WATER_VEIL, ABILITY_SOUNDPROOF, ABILITY_THICK_FAT, ABILITY_KEEN_EYE, ABILITY_HYPER_CUTTER,
    ABILITY_STICKY_HOLD, ABILITY_MARVEL_SCALE, ABILITY_VITAL_SPIRIT, ABILITY_WHITE_SMOKE, ABILITY_TANGLED_FEET,
    ABILITY_MOTOR_DRIVE, ABILITY_SNOW_CLOAK, ABILITY_HEATPROOF, ABILITY_SIMPLE, ABILITY_DRY_SKIN, ABILITY_LEAF_GUARD,
    ABILITY_UNAWARE, ABILITY_FILTER, ABILITY_STORM_DRAIN, ABILITY_SOLID_ROCK, ABILITY_FLOWER_GIFT,
};

static bool8 InList(const u16 *list, u32 count, u16 value)
{
    u32 i;
    for (i = 0; i < count; i++)
        if (list[i] == value)
            return TRUE;
    return FALSE;
}

// another Pokémon on the battler's side (doubles / triples) with this ability
static bool8 AllyHasAbility(u8 battler, u16 ability)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
        if (i != battler && GetBattlerSide(i) == GetBattlerSide(battler) && !(gAbsentBattlerFlags & gBitTable[i])
         && gBattleMons[i].hp != 0 && gBattleMons[i].ability == ability)
            return TRUE;
    return FALSE;
}

bool8 MP_Gen4_IsPunchingMove(u16 move)
{
    return InList(sPunchingMoves, ARRAY_COUNT(sPunchingMoves), move);
}

// ---------------------------------------------------------------------------------------------- state

void MP_Gen4_BattleStart(void)
{
    memset(gGen4Battlers, 0, sizeof(gGen4Battlers));
    memset(gGen4Sides, 0, sizeof(gGen4Sides));
    memset(&gGen4Field, 0, sizeof(gGen4Field));
}

void MP_Gen4_SwitchIn(u8 battler, bool8 batonPass)
{
    struct Gen4Battler *b = &gGen4Battlers[battler];
    struct Gen4Battler keep = *b;
    memset(b, 0, sizeof(*b));
    if (batonPass)
    {
        // what BATON PASS hands on in Gen 4
        b->embargo = keep.embargo;
        b->healBlock = keep.healBlock;
        b->magnetRise = keep.magnetRise;
        b->aquaRing = keep.aquaRing;
        b->powerTrick = keep.powerTrick;
    }
    if (gBattleMons[battler].ability == ABILITY_SLOW_START)
        b->slowStart = 5;
}

void MP_Gen4_TurnEnd(void)
{
    u32 i;
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        gGen4Battlers[i].roosted = FALSE;
        gGen4Battlers[i].hitThisTurn = FALSE;
        gGen4Battlers[i].movedThisTurn = FALSE;
    }
}

// ---------------------------------------------------------------------------------------------- basics

bool8 MP_IsPhysicalMove(u16 move, u8 type)
{
    if (!MP_MoveSplit())
        return IS_TYPE_PHYSICAL(type);
    if (move >= MOVES_COUNT)
        return IS_TYPE_PHYSICAL(type);
    return gMoveCategories[move] == MOVE_CATEGORY_PHYSICAL;
}

u16 MP_Gen4_TargetAbility(u8 attacker, u8 target)
{
    u16 ability = gBattleMons[target].ability;
    if (attacker != target && gBattleMons[attacker].ability == ABILITY_MOLD_BREAKER
     && InList(sBreakableAbilities, ARRAY_COUNT(sBreakableAbilities), ability))
        return ABILITY_NONE;
    return ability;
}

u8 MP_Gen4_HoldEffect(u8 battler)
{
    return MP_Gen4_MonHoldEffect(battler, &gBattleMons[battler]);
}

// The same for a given Pokémon's data (damage math may run on copies of the battlers)
u8 MP_Gen4_MonHoldEffect(u8 battler, const struct BattlePokemon *mon)
{
    if (gGen4Battlers[battler].embargo || mon->ability == ABILITY_KLUTZ)
        return HOLD_EFFECT_NONE;
    if (mon->item == ITEM_ENIGMA_BERRY)
        return gEnigmaBerries[battler].holdEffect;
    return GetItemHoldEffect(mon->item);
}

bool8 MP_Gen4_HasType(u8 battler, u8 type)
{
    if (type == TYPE_FLYING && gGen4Battlers[battler].roosted)
        return FALSE;
    return gBattleMons[battler].types[0] == type || gBattleMons[battler].types[1] == type;
}

bool8 MP_Gen4_IsGrounded(u8 battler)
{
    if (gGen4Field.gravity || MP_Gen4_HoldEffect(battler) == HOLD_EFFECT_IRON_BALL || (gStatuses3[battler] & STATUS3_ROOTED))
        return TRUE;
    if (gGen4Battlers[battler].magnetRise)
        return FALSE;
    if (gBattleMons[battler].ability == ABILITY_LEVITATE)
        return FALSE;
    return !MP_Gen4_HasType(battler, TYPE_FLYING);
}

static const u8 sPlateTypes[] =
{
    TYPE_FIRE, TYPE_WATER, TYPE_ELECTRIC, TYPE_GRASS, TYPE_ICE, TYPE_FIGHTING, TYPE_POISON, TYPE_GROUND,
    TYPE_FLYING, TYPE_PSYCHIC, TYPE_BUG, TYPE_ROCK, TYPE_GHOST, TYPE_DRAGON, TYPE_DARK, TYPE_STEEL,
};

// the type a PLATE gives JUDGMENT and MULTITYPE (NORMAL without one)
u8 MP_Gen4_PlateType(u16 item)
{
    if (item >= FIRST_PLATE_ITEM && item <= LAST_PLATE_ITEM)
        return sPlateTypes[item - FIRST_PLATE_ITEM];
    return TYPE_NORMAL;
}

u8 MP_Gen4_MoveType(u16 move, u8 battler)
{
    u8 type = gBattleMoves[move].type;
    if (gBattleMoves[move].effect == EFFECT_JUDGMENT && !gGen4Battlers[battler].embargo && gBattleMons[battler].ability != ABILITY_KLUTZ)
        type = MP_Gen4_PlateType(gBattleMons[battler].item);
    if (gBattleMons[battler].ability == ABILITY_NORMALIZE && move != MOVE_STRUGGLE)
        type = TYPE_NORMAL;
    return type;
}

// ---------------------------------------------------------------------------------------------- damage

static u8 PositiveStages(u8 battler)
{
    u32 i, n = 0;
    for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
        if (gBattleMons[battler].statStages[i] > DEFAULT_STAT_STAGE)
            n += gBattleMons[battler].statStages[i] - DEFAULT_STAT_STAGE;
    return n;
}

static u32 BattlerSpeed(u8 battler)
{
    u32 speed = gBattleMons[battler].speed * gStatStageRatios[gBattleMons[battler].statStages[STAT_SPEED]][0]
              / gStatStageRatios[gBattleMons[battler].statStages[STAT_SPEED]][1];
    if (gBattleMons[battler].status1 & STATUS1_PARALYSIS && gBattleMons[battler].ability != ABILITY_QUICK_FEET)
        speed /= 4;
    return speed ? speed : 1;
}

// variable powers and power changes of the moves, abilities and items Gen 4 added
u16 MP_Gen4_MovePower(u16 move, u8 attacker, u8 target, u16 power)
{
    u16 effect = gBattleMoves[move].effect;
    u8 type = MP_Gen4_MoveType(move, attacker);
    u8 hold = MP_Gen4_HoldEffect(attacker);
    u32 p = power;

    switch (effect)
    {
    case EFFECT_GYRO_BALL:
        p = 1 + 25 * BattlerSpeed(target) / BattlerSpeed(attacker);
        if (p > 150)
            p = 150;
        break;
    case EFFECT_BRINE:
        if (gBattleMons[target].hp <= gBattleMons[target].maxHP / 2)
            p *= 2;
        break;
    case EFFECT_WAKE_UP_SLAP:
        if (gBattleMons[target].status1 & STATUS1_SLEEP)
            p *= 2;
        break;
    case EFFECT_PAYBACK:
        if (gGen4Battlers[target].movedThisTurn)
            p *= 2;
        break;
    case EFFECT_ASSURANCE:
        if (gGen4Battlers[target].hitThisTurn)
            p *= 2;
        break;
    case EFFECT_WRING_OUT:
        p = 1 + 120 * gBattleMons[target].hp / gBattleMons[target].maxHP;
        break;
    case EFFECT_PUNISHMENT:
        p = 60 + 20 * PositiveStages(target);
        if (p > 200)
            p = 200;
        break;
    case EFFECT_TRUMP_CARD:
        switch (gBattleMons[attacker].pp[gCurrMovePos])
        {
        case 0:  p = 200; break;
        case 1:  p = 80; break;
        case 2:  p = 60; break;
        case 3:  p = 50; break;
        default: p = 40; break;
        }
        break;
    }

    // abilities
    if (gBattleMons[attacker].ability == ABILITY_TECHNICIAN && p <= 60)
        p = p * 15 / 10;
    if (gBattleMons[attacker].ability == ABILITY_IRON_FIST && MP_Gen4_IsPunchingMove(move))
        p = p * 12 / 10;
    if (gBattleMons[attacker].ability == ABILITY_RECKLESS && InList(sRecoilMoves, ARRAY_COUNT(sRecoilMoves), move))
        p = p * 12 / 10;
    if (gBattleMons[attacker].ability == ABILITY_RIVALRY)
    {
        u8 g1 = GetGenderFromSpeciesAndPersonality(gBattleMons[attacker].species, gBattleMons[attacker].personality);
        u8 g2 = GetGenderFromSpeciesAndPersonality(gBattleMons[target].species, gBattleMons[target].personality);
        if (g1 != MON_GENDERLESS && g2 != MON_GENDERLESS)
            p = g1 == g2 ? p * 125 / 100 : p * 75 / 100;
    }
    switch (MP_Gen4_TargetAbility(attacker, target))
    {
    case ABILITY_HEATPROOF:
        if (type == TYPE_FIRE)
            p /= 2;
        break;
    case ABILITY_DRY_SKIN:
        if (type == TYPE_FIRE)
            p = p * 125 / 100;
        break;
    }

    // items
    if (hold == HOLD_EFFECT_MUSCLE_BAND && MP_IsPhysicalMove(move, type))
        p = p * 11 / 10;
    if (hold == HOLD_EFFECT_WISE_GLASSES && !MP_IsPhysicalMove(move, type))
        p = p * 11 / 10;
    if ((hold == HOLD_EFFECT_ADAMANT_ORB && gBattleMons[attacker].species == SPECIES_DIALGA)
     || (hold == HOLD_EFFECT_LUSTROUS_ORB && gBattleMons[attacker].species == SPECIES_PALKIA)
     || (hold == HOLD_EFFECT_GRISEOUS_ORB && gBattleMons[attacker].species == SPECIES_GIRATINA))
    {
        if (type == TYPE_DRAGON || MP_Gen4_HasType(attacker, type))
            p = p * 12 / 10;
    }
    return p ? p : 1;
}

// changes to the attacking / defending stats (after Kai's own items and abilities)
void MP_Gen4_AttackStats(u16 move, u8 type, u8 attacker, u8 target, u16 *attack, u16 *spAttack, u16 *defense, u16 *spDefense)
{
    u8 hold = MP_Gen4_HoldEffect(attacker);
    bool8 sun = WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SUN);

    if (hold == HOLD_EFFECT_CHOICE_SPECS)
        *spAttack = *spAttack * 15 / 10;
    if (gBattleMons[attacker].ability == ABILITY_SOLAR_POWER && sun)
        *spAttack = *spAttack * 15 / 10;
    if (gBattleMons[attacker].ability == ABILITY_SLOW_START && gGen4Battlers[attacker].slowStart)
        *attack /= 2;
    if (sun && (gBattleMons[attacker].ability == ABILITY_FLOWER_GIFT || AllyHasAbility(attacker, ABILITY_FLOWER_GIFT)))
        *attack = *attack * 15 / 10;
    if (sun && (MP_Gen4_TargetAbility(attacker, target) == ABILITY_FLOWER_GIFT || AllyHasAbility(target, ABILITY_FLOWER_GIFT)))
        *spDefense = *spDefense * 15 / 10;
    // Gen 4: a SANDSTORM raises ROCK types' SP. DEF
    if (MP_MoveSplit() && WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SANDSTORM) && MP_Gen4_HasType(target, TYPE_ROCK))
        *spDefense = *spDefense * 15 / 10;
}

// after STAB and type effectiveness
s32 MP_Gen4_FinalDamage(s32 damage, u16 move, u8 type, u8 attacker, u8 target, u8 resultFlags)
{
    u8 hold = MP_Gen4_HoldEffect(attacker);
    u16 defAbility = MP_Gen4_TargetAbility(attacker, target);

    if (damage <= 0 || (resultFlags & MOVE_RESULT_NO_EFFECT))
        return damage;
    if ((resultFlags & MOVE_RESULT_NOT_VERY_EFFECTIVE) && gBattleMons[attacker].ability == ABILITY_TINTED_LENS)
        damage *= 2;
    if ((resultFlags & MOVE_RESULT_SUPER_EFFECTIVE) && (defAbility == ABILITY_FILTER || defAbility == ABILITY_SOLID_ROCK))
        damage = damage * 3 / 4;
    if ((resultFlags & MOVE_RESULT_SUPER_EFFECTIVE) && hold == HOLD_EFFECT_EXPERT_BELT)
        damage = damage * 12 / 10;
    if (hold == HOLD_EFFECT_LIFE_ORB)
        damage = damage * 13 / 10;
    if (MP_Gen4_HoldEffect(target) == HOLD_EFFECT_RESIST_BERRY && type == GetItemHoldEffectParam(gBattleMons[target].item)
     && ((resultFlags & MOVE_RESULT_SUPER_EFFECTIVE) || type == TYPE_NORMAL) && !(gBattleMons[target].status2 & STATUS2_SUBSTITUTE))
    {
        damage /= 2;   // the type-resist BERRIES (OCCA..BABIRI, CHILAN): eaten when the move ends
        gGen4Battlers[target].resistBerry = TRUE;
    }
    if (hold == HOLD_EFFECT_METRONOME && gGen4Battlers[attacker].metronomeMove == move && gGen4Battlers[attacker].metronomeCount)
    {
        u32 n = gGen4Battlers[attacker].metronomeCount > 10 ? 10 : gGen4Battlers[attacker].metronomeCount;
        damage = damage * (10 + n) / 10;
    }
    return damage ? damage : 1;
}

// STAB: 2x with ADAPTABILITY
s32 MP_Gen4_Stab(s32 damage, u8 attacker)
{
    if (gBattleMons[attacker].ability == ABILITY_ADAPTABILITY)
        return damage * 2;
    return damage * 15 / 10;
}

// ---------------------------------------------------------------------------------------------- accuracy, crits, order

u8 MP_Gen4_CritStage(u8 attacker)
{
    u8 stage = 0;
    if (gBattleMons[attacker].ability == ABILITY_SUPER_LUCK)
        stage++;
    if (MP_Gen4_HoldEffect(attacker) == HOLD_EFFECT_SCOPE_LENS && GetItemHoldEffect(gBattleMons[attacker].item) != HOLD_EFFECT_SCOPE_LENS)
        stage++;   // RAZOR CLAW (Kai's SCOPE LENS is counted by the engine)
    return stage;
}

u8 MP_Gen4_CritMultiplier(u8 attacker)
{
    return gBattleMons[attacker].ability == ABILITY_SNIPER ? 3 : 2;
}

bool8 MP_Gen4_IgnoresAccuracy(u16 move, u8 attacker, u8 target)
{
    return gBattleMons[attacker].ability == ABILITY_NO_GUARD || gBattleMons[target].ability == ABILITY_NO_GUARD;
}

u16 MP_Gen4_Accuracy(u16 move, u8 attacker, u8 target, u16 calc)
{
    u8 hold = MP_Gen4_HoldEffect(attacker);
    u16 defAbility = MP_Gen4_TargetAbility(attacker, target);
    if (gGen4Field.gravity)
        calc = calc * 5 / 3;
    if (gGen4Battlers[attacker].micle)
        calc = calc * 12 / 10;
    if (hold == HOLD_EFFECT_WIDE_LENS)
        calc = calc * 11 / 10;
    if (hold == HOLD_EFFECT_ZOOM_LENS && gGen4Battlers[target].movedThisTurn)
        calc = calc * 12 / 10;
    if (defAbility == ABILITY_TANGLED_FEET && (gBattleMons[target].status2 & STATUS2_CONFUSION))
        calc /= 2;
    if (defAbility == ABILITY_SNOW_CLOAK && WEATHER_HAS_EFFECT && (gBattleWeather & B_WEATHER_HAIL))
        calc = calc * 80 / 100;
    return calc;
}

// a key for GetWhoStrikesFirst: bigger moves first within a priority. QUICK CLAW first, then the "always last"
// ones (STALL, LAGGING TAIL, FULL INCENSE) last; TRICK ROOM turns the speeds around
u32 MP_Gen4_OrderKey(u8 battler, u32 speed, u8 holdEffect)
{
    u8 hold = MP_Gen4_HoldEffect(battler);
    if (speed == UINT_MAX)
        return UINT_MAX;   // QUICK CLAW
    if (hold == HOLD_EFFECT_CUSTAP_BERRY && gBattleMons[battler].hp != 0 && gBattleMons[battler].hp <= gBattleMons[battler].maxHP / 4)
        return UINT_MAX;   // CUSTAP BERRY (eaten when it moves: MP_Gen4_BeforeMove)
    if (gBattleMons[battler].ability == ABILITY_STALL || hold == HOLD_EFFECT_LAGGING_TAIL)
        return 0;
    if (hold == HOLD_EFFECT_CHOICE_SCARF)
        speed = speed * 15 / 10;
    if (hold == HOLD_EFFECT_IRON_BALL || hold == HOLD_EFFECT_POWER_ITEM)
        speed /= 2;
    if (gBattleMons[battler].ability == ABILITY_QUICK_FEET && gBattleMons[battler].status1)
        speed = (gBattleMons[battler].status1 & STATUS1_PARALYSIS) ? speed * 4 * 15 / 10 : speed * 15 / 10;
    if (gBattleMons[battler].ability == ABILITY_SLOW_START && gGen4Battlers[battler].slowStart)
        speed /= 2;
    if (gBattleMons[battler].ability == ABILITY_UNBURDEN && gGen4Battlers[battler].unburden)
        speed *= 2;
    if (gGen4Sides[GetBattlerSide(battler)].tailwind)
        speed *= 2;
    if (speed > 0x7FFFFFF0)
        speed = 0x7FFFFFF0;
    if (gGen4Field.trickRoom)
        speed = 0x7FFFFFF0 - speed;
    return speed + 1;
}

// ---------------------------------------------------------------------------------------------- move use

static const u16 sCopycatBanned[] =
{
    MOVE_NONE, MOVE_STRUGGLE, MOVE_COPYCAT, MOVE_ME_FIRST, MOVE_MIRROR_MOVE, MOVE_METRONOME, MOVE_ASSIST,
    MOVE_SLEEP_TALK, MOVE_COUNTER, MOVE_MIRROR_COAT, MOVE_COVET, MOVE_THIEF, MOVE_ENDURE, MOVE_PROTECT,
    MOVE_DETECT, MOVE_FOCUS_PUNCH, MOVE_FOLLOW_ME, MOVE_HELPING_HAND, MOVE_TRICK, MOVE_SWITCHEROO, MOVE_FEINT,
    MOVE_TRANSFORM, MOVE_SKETCH, MOVE_CHATTER, MOVE_UNAVAILABLE,
};

void MP_Gen4_OnMoveStart(u8 battler, u16 move)
{
    struct Gen4Battler *b = &gGen4Battlers[battler];
    b->movedThisTurn = TRUE;
    b->meFirst = FALSE;
    gGen4Field.moveEndDone = 0;
    gGen4Field.dealtDamage = FALSE;
    if (b->metronomeMove == move)
    {
        if (b->metronomeCount < 10)
            b->metronomeCount++;
    }
    else
    {
        b->metronomeMove = move;
        b->metronomeCount = 0;
    }
    if (gCurrMovePos < MAX_MON_MOVES && gBattleMons[battler].moves[gCurrMovePos] == move)
        b->usedMoves |= 1 << gCurrMovePos;
    if (!InList(sCopycatBanned, ARRAY_COUNT(sCopycatBanned), move))
        gGen4Field.lastMoveUsed = move;
}

void MP_Gen4_OnDamaged(u8 battler, u8 attacker, s32 damage)
{
    gGen4Battlers[battler].hitThisTurn = TRUE;
    if (damage > 0 && attacker != battler)
    {
        gGen4Field.dealtDamage = TRUE;
        gGen4Battlers[battler].lastDamage = damage > 0xFFFF ? 0xFFFF : damage;
        gGen4Battlers[battler].lastAttacker = attacker;
    }
}

// ---------------------------------------------------------------------------------------------- the "gen4" command

extern const u8 *const gBattleScriptsForMoveEffects[];
#include "data/gen4_item_battle.h"

static void SetItem(u8 battler, u16 item)
{
    gBattleMons[battler].item = item;
    gActiveBattler = battler;
    BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_HELDITEM_BATTLE, 0, sizeof(gBattleMons[battler].item), &gBattleMons[battler].item);
    MarkBattlerForControllerExec(battler);
    if (item == ITEM_NONE)
        MP_Gen4_ItemLost(battler);
}

static void SetStatus(u8 battler, u32 status)
{
    gBattleMons[battler].status1 = status;
    gActiveBattler = battler;
    BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, sizeof(gBattleMons[battler].status1), &gBattleMons[battler].status1);
    MarkBattlerForControllerExec(battler);
}

static bool8 IsDamagingMove(u16 move)
{
    return move != MOVE_NONE && move < MOVES_COUNT && gBattleMoves[move].power != 0;
}

static u8 RunMove(u16 move)
{
    gHitMarker &= ~HITMARKER_ATTACKSTRING_PRINTED;
    gCurrentMove = move;
    gBattlerTarget = GetMoveTarget(gCurrentMove, NO_TARGET_OVERRIDE);
    gBattlescriptCurrInstr = gBattleScriptsForMoveEffects[gBattleMoves[gCurrentMove].effect];
    return 2;
}

static void SwapStages(u8 a, u8 b, u8 stat)
{
    s8 t = gBattleMons[a].statStages[stat];
    gBattleMons[a].statStages[stat] = gBattleMons[b].statStages[stat];
    gBattleMons[b].statStages[stat] = t;
}

u8 MP_Gen4_Command(u8 mode)
{
    u8 atk = gBattlerAttacker, def = gBattlerTarget;
    struct Gen4Side *defSide = &gGen4Sides[GetBattlerSide(def)], *atkSide = &gGen4Sides[GetBattlerSide(atk)];
    u32 i;

    switch (mode)
    {
    case G4_ROOST:
        gGen4Battlers[atk].roosted = TRUE;
        return 0;
    case G4_GRAVITY:
        if (gGen4Field.gravity)
            return 1;
        gGen4Field.gravity = 5;
        return 0;
    case G4_MIRACLE_EYE:
        gGen4Battlers[def].miracleEye = TRUE;
        return 0;
    case G4_WAKE_TARGET:
        if ((gBattleMons[def].status1 & STATUS1_SLEEP) && gBattleMons[def].hp != 0 && !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT))
        {
            SetStatus(def, gBattleMons[def].status1 & ~STATUS1_SLEEP);
            gBattleMons[def].status2 &= ~STATUS2_NIGHTMARE;
            return 1;
        }
        return 0;
    case G4_HEALING_WISH:
        atkSide->healingWish = TRUE;
        return 0;
    case G4_LUNAR_DANCE:
        atkSide->healingWish = atkSide->lunarDance = TRUE;
        return 0;
    case G4_FEINT:
        if (!gProtectStructs[def].protected)
            return 1;
        gProtectStructs[def].protected = FALSE;
        return 0;
    case G4_LIFT_PROTECT:
        gProtectStructs[def].protected = FALSE;
        return 0;
    case G4_BUG_BITE:
    {
        u16 item = gBattleMons[def].item;
        u8 hold;
        if (gBattleMons[def].hp == 0 || (gMoveResultFlags & MOVE_RESULT_NO_EFFECT) || !IS_BERRY_ITEM(item)
         || gBattleMons[def].ability == ABILITY_STICKY_HOLD)
            return 0;
        hold = GetItemHoldEffect(item);
        gLastUsedItem = item;
        SetItem(def, ITEM_NONE);
        gBattleMoveDamage = 0;
        switch (hold)
        {
        case HOLD_EFFECT_RESTORE_HP:
            gBattleMoveDamage = -(s32)GetItemHoldEffectParam(item);
            break;
        case HOLD_EFFECT_CURE_PAR: SetStatus(atk, gBattleMons[atk].status1 & ~STATUS1_PARALYSIS); break;
        case HOLD_EFFECT_CURE_SLP: SetStatus(atk, gBattleMons[atk].status1 & ~STATUS1_SLEEP); break;
        case HOLD_EFFECT_CURE_PSN: SetStatus(atk, gBattleMons[atk].status1 & ~STATUS1_PSN_ANY); break;
        case HOLD_EFFECT_CURE_BRN: SetStatus(atk, gBattleMons[atk].status1 & ~STATUS1_BURN); break;
        case HOLD_EFFECT_CURE_FRZ: SetStatus(atk, gBattleMons[atk].status1 & ~STATUS1_FREEZE); break;
        case HOLD_EFFECT_CURE_STATUS: SetStatus(atk, 0); break;
        }
        if (gBattleMoveDamage != 0 && gBattleMons[atk].hp - gBattleMoveDamage > gBattleMons[atk].maxHP)
            gBattleMoveDamage = gBattleMons[atk].hp - gBattleMons[atk].maxHP;
        return 1;
    }
    case G4_TAILWIND:
        if (atkSide->tailwind)
            return 1;
        atkSide->tailwind = 3;
        return 0;
    case G4_ACUPRESSURE:
    {
        u8 stats[NUM_BATTLE_STATS], n = 0;
        for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
            if (gBattleMons[def].statStages[i] < MAX_STAT_STAGE)
                stats[n++] = i;
        if (n == 0)
            return 1;
        i = stats[Random() % n];
        SET_STATCHANGER(i, 2, FALSE);
        PREPARE_STAT_BUFFER(gBattleTextBuff1, i);
        return 0;
    }
    case G4_METAL_BURST:
    {
        struct Gen4Battler *b = &gGen4Battlers[atk];
        if (!b->hitThisTurn || b->lastDamage == 0 || GetBattlerSide(b->lastAttacker) == GetBattlerSide(atk)
         || gBattleMons[b->lastAttacker].hp == 0)
            return 1;
        gBattleMoveDamage = b->lastDamage * 3 / 2;
        gBattlerTarget = b->lastAttacker;
        return 0;
    }
    case G4_EMBARGO:
        if (gGen4Battlers[def].embargo)
            return 1;
        gGen4Battlers[def].embargo = 5;
        return 0;
    case G4_FLING:
    {
        u16 item = gBattleMons[atk].item;
        if (item == ITEM_NONE || gGen4Battlers[atk].embargo || gBattleMons[atk].ability == ABILITY_KLUTZ || sFlingPower[item] == 0)
            return 1;
        gDynamicBasePower = sFlingPower[item];
        gLastUsedItem = item;
        switch (GetItemHoldEffect(item))
        {
        case HOLD_EFFECT_FLAME_ORB:  gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_BURN; break;
        case HOLD_EFFECT_TOXIC_ORB:  gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_TOXIC; break;
        case HOLD_EFFECT_FLINCH:     gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_FLINCH; break;
        case HOLD_EFFECT_LIGHT_BALL: gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_PARALYSIS; break;
        case HOLD_EFFECT_POISON_POWER: gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_POISON; break;
        }
        if (gBattleCommunication[MOVE_EFFECT_BYTE])
            gBattleCommunication[MOVE_EFFECT_BYTE] |= MOVE_EFFECT_CERTAIN;
        return 0;
    }
    case G4_CONSUME_ITEM:
        if (gBattleMons[atk].item != ITEM_NONE)
            SetItem(atk, ITEM_NONE);
        return 0;
    case G4_PSYCHO_SHIFT:
    {
        u32 st = gBattleMons[atk].status1;
        if (!(st & STATUS1_ANY) || (gBattleMons[def].status1 & STATUS1_ANY))
            return 1;
        if (st & STATUS1_SLEEP)               gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_SLEEP;
        else if (st & STATUS1_TOXIC_POISON)   gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_TOXIC;
        else if (st & STATUS1_POISON)         gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_POISON;
        else if (st & STATUS1_BURN)           gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_BURN;
        else if (st & STATUS1_PARALYSIS)      gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_PARALYSIS;
        else if (st & STATUS1_FREEZE)         gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_FREEZE;
        return 0;
    }
    case G4_PSYCHO_SHIFT_CURE:
        if (gBattleMons[def].status1 & STATUS1_ANY)
            SetStatus(atk, 0);
        return 0;
    case G4_HEAL_BLOCK:
        if (gGen4Battlers[def].healBlock)
            return 1;
        gGen4Battlers[def].healBlock = 5;
        return 0;
    case G4_POWER_TRICK:
    {
        u16 t = gBattleMons[atk].attack;
        gBattleMons[atk].attack = gBattleMons[atk].defense;
        gBattleMons[atk].defense = t;
        gGen4Battlers[atk].powerTrick ^= 1;
        return 0;
    }
    case G4_GASTRO_ACID:
        if (gBattleMons[def].ability == ABILITY_MULTITYPE || gBattleMons[def].ability == ABILITY_NONE)
            return 1;
        gBattleMons[def].ability = ABILITY_NONE;
        return 0;
    case G4_LUCKY_CHANT:
        if (atkSide->luckyChant)
            return 1;
        atkSide->luckyChant = 5;
        return 0;
    case G4_ME_FIRST:
    {
        u16 move = MOVE_NONE;
        if (gChosenActionByBattler[def] == B_ACTION_USE_MOVE && !gGen4Battlers[def].movedThisTurn)
            move = gBattleMons[def].moves[*(gBattleStruct->chosenMovePositions + def)];
        if (!IsDamagingMove(move) || InList(sCopycatBanned, ARRAY_COUNT(sCopycatBanned), move))
            return 1;
        RunMove(move);
        gBattlerTarget = def;
        gGen4Battlers[atk].meFirst = TRUE;
        return 2;
    }
    case G4_COPYCAT:
        if (gGen4Field.lastMoveUsed == MOVE_NONE || InList(sCopycatBanned, ARRAY_COUNT(sCopycatBanned), gGen4Field.lastMoveUsed))
            return 1;
        return RunMove(gGen4Field.lastMoveUsed);
    case G4_POWER_SWAP:
        SwapStages(atk, def, STAT_ATK);
        SwapStages(atk, def, STAT_SPATK);
        return 0;
    case G4_GUARD_SWAP:
        SwapStages(atk, def, STAT_DEF);
        SwapStages(atk, def, STAT_SPDEF);
        return 0;
    case G4_HEART_SWAP:
        for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
            SwapStages(atk, def, i);
        return 0;
    case G4_LAST_RESORT:
    {
        u8 known = 0, used = gGen4Battlers[atk].usedMoves | (1 << gCurrMovePos);
        bool8 others = FALSE;
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            if (gBattleMons[atk].moves[i] == MOVE_NONE)
                continue;
            known |= 1 << i;
            if (i != gCurrMovePos)
                others = TRUE;
        }
        return (!others || (known & used) != known) ? 1 : 0;
    }
    case G4_WORRY_SEED:
        if (gBattleMons[def].ability == ABILITY_TRUANT || gBattleMons[def].ability == ABILITY_MULTITYPE
         || gBattleMons[def].ability == ABILITY_INSOMNIA)
            return 1;
        gBattleMons[def].ability = ABILITY_INSOMNIA;
        return 0;
    case G4_SUCKER_PUNCH:
    {
        u16 move;
        if (gGen4Battlers[def].movedThisTurn || gChosenActionByBattler[def] != B_ACTION_USE_MOVE)
            return 1;
        move = gBattleMons[def].moves[*(gBattleStruct->chosenMovePositions + def)];
        return IsDamagingMove(move) ? 0 : 1;
    }
    case G4_TOXIC_SPIKES:
        if (defSide->toxicSpikes >= 2)
            return 1;
        defSide->toxicSpikes++;
        return 0;
    case G4_AQUA_RING:
        if (gGen4Battlers[atk].aquaRing)
            return 1;
        gGen4Battlers[atk].aquaRing = TRUE;
        return 0;
    case G4_MAGNET_RISE:
        if (gGen4Battlers[atk].magnetRise || gGen4Field.gravity || (gStatuses3[atk] & STATUS3_ROOTED))
            return 1;
        gGen4Battlers[atk].magnetRise = 5;
        return 0;
    case G4_FANG_STATUS:
    {
        u32 chance = gBattleMons[atk].ability == ABILITY_SERENE_GRACE ? 20 : 10;
        if ((gMoveResultFlags & MOVE_RESULT_NO_EFFECT) || Random() % 100 >= chance)
            return 1;
        switch (gBattleMoves[gCurrentMove].effect)
        {
        case EFFECT_THUNDER_FANG: gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_PARALYSIS; break;
        case EFFECT_ICE_FANG:     gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_FREEZE; break;
        default:                  gBattleCommunication[MOVE_EFFECT_BYTE] = MOVE_EFFECT_BURN; break;
        }
        return 0;
    }
    case G4_DEFOG:
    {
        u8 side = GetBattlerSide(def);
        bool8 any = (gSideStatuses[side] & (SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN | SIDE_STATUS_SAFEGUARD
                                            | SIDE_STATUS_MIST | SIDE_STATUS_SPIKES))
                 || defSide->toxicSpikes || defSide->stealthRock;
        gSideStatuses[side] &= ~(SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN | SIDE_STATUS_SAFEGUARD | SIDE_STATUS_MIST
                                 | SIDE_STATUS_SPIKES | SIDE_STATUS_SPIKES_DAMAGED);
        gSideTimers[side].reflectTimer = gSideTimers[side].lightscreenTimer = 0;
        gSideTimers[side].safeguardTimer = gSideTimers[side].mistTimer = 0;
        gSideTimers[side].spikesAmount = 0;
        defSide->toxicSpikes = 0;
        defSide->stealthRock = FALSE;
        return any ? 0 : 1;
    }
    case G4_TRICK_ROOM:
        if (gGen4Field.trickRoom)
        {
            gGen4Field.trickRoom = 0;
            return 1;
        }
        gGen4Field.trickRoom = 5;
        return 0;
    case G4_CAPTIVATE:
    {
        u8 g1 = GetGenderFromSpeciesAndPersonality(gBattleMons[atk].species, gBattleMons[atk].personality);
        u8 g2 = GetGenderFromSpeciesAndPersonality(gBattleMons[def].species, gBattleMons[def].personality);
        if (g1 == MON_GENDERLESS || g2 == MON_GENDERLESS || g1 == g2 || MP_Gen4_TargetAbility(atk, def) == ABILITY_OBLIVIOUS)
            return 1;
        return 0;
    }
    case G4_STEALTH_ROCK:
        if (defSide->stealthRock)
            return 1;
        defSide->stealthRock = TRUE;
        return 0;
    case G4_VANISH:
        gGen4Battlers[atk].vanished = TRUE;
        return 0;
    case G4_UNVANISH:
        gGen4Battlers[atk].vanished = FALSE;
        return 0;
    case G4_FLARE_THAW:
        if (gBattleMons[atk].status1 & STATUS1_FREEZE)
            SetStatus(atk, gBattleMons[atk].status1 & ~STATUS1_FREEZE);
        return 0;
    case G4_NATURAL_GIFT:
    {
        u16 item = gBattleMons[atk].item;
        if (item == ITEM_NONE || gGen4Battlers[atk].embargo || gBattleMons[atk].ability == ABILITY_KLUTZ || sNaturalGift[item][0] == 0)
            return 1;
        gDynamicBasePower = sNaturalGift[item][0];
        gBattleStruct->dynamicMoveType = sNaturalGift[item][1] | F_DYNAMIC_TYPE_SET;
        gLastUsedItem = item;
        return 0;
    }
    case G4_RERUN_MOVE:
        BattleScriptPop();
        gBattlescriptCurrInstr = gBattleScriptsForMoveEffects[gBattleMoves[gCurrentMove].effect];
        return 2;
    case G4_ARRIVE_SYNC:
    {
        // HEALING WISH / LUNAR DANCE: send the cured status, then (LUNAR DANCE) the PP
        u8 b = gBattleScripting.battler;
        gActiveBattler = b;
        if (gGen4Field.arriveSyncStep == 0)
        {
            BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, sizeof(gBattleMons[b].status1), &gBattleMons[b].status1);
            MarkBattlerForControllerExec(b);
            if (gGen4Field.arriveLunar)
            {
                gGen4Field.arriveSyncStep = 1;
                return 1;   // back to this command (ptr) once the status went out
            }
            return 0;
        }
        else
        {
            u8 data[MAX_MON_MOVES + 1];
            for (i = 0; i < MAX_MON_MOVES; i++)
            {
                if (gBattleMons[b].moves[i] != MOVE_NONE)
                    gBattleMons[b].pp[i] = CalculatePPWithBonus(gBattleMons[b].moves[i], gBattleMons[b].ppBonuses, i);
                data[i] = gBattleMons[b].pp[i];
            }
            data[MAX_MON_MOVES] = gBattleMons[b].ppBonuses;
            BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_PP_DATA_BATTLE, 0, sizeof(data), data);
            MarkBattlerForControllerExec(b);
            gGen4Field.arriveSyncStep = 0;
            gGen4Field.arriveLunar = FALSE;
            return 0;
        }
    }
    case G4_CHARGE_HERB:
        if (MP_Gen4_HoldEffect(atk) != HOLD_EFFECT_POWER_HERB)
            return 0;
        gLastUsedItem = gBattleMons[atk].item;
        SetItem(atk, ITEM_NONE);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------- arriving

#define G4_ARRIVE_WISH   (1 << 0)
#define G4_ARRIVE_ROCK   (1 << 1)
#define G4_ARRIVE_TSPIKE (1 << 2)

// how many 16ths of 1x STEALTH ROCK (ROCK) does to this battler's types
static u32 RockEffectiveness16(u8 battler)
{
    u32 f = 16, i = 0;
    while (TYPE_EFFECT_ATK_TYPE(i) != TYPE_ENDTABLE)
    {
        if (TYPE_EFFECT_ATK_TYPE(i) == TYPE_ROCK)
        {
            u8 def = TYPE_EFFECT_DEF_TYPE(i), mul = TYPE_EFFECT_MULTIPLIER(i);
            u8 t0 = gBattleMons[battler].types[0], t1 = gBattleMons[battler].types[1];
            u32 hits = (def == t0) + (def == t1 && t1 != t0);
            while (hits--)
                f = mul == TYPE_MUL_SUPER_EFFECTIVE ? f * 2 : mul == TYPE_MUL_NOT_EFFECTIVE ? f / 2 : f;
        }
        i += 3;
    }
    return f;
}

// what happens to a Pokémon arriving on the field (once per arrival); NULL when nothing (more) does
const u8 *MP_Gen4_ArrivalEffect(u8 battler, u8 bsArg)
{
    struct Gen4Battler *b = &gGen4Battlers[battler];
    struct Gen4Side *side = &gGen4Sides[GetBattlerSide(battler)];
    u32 i;

    if (gBattleMons[battler].hp == 0)
        return NULL;
    if (!(b->switchInDone & G4_ARRIVE_WISH))
    {
        b->switchInDone |= G4_ARRIVE_WISH;
        if (side->healingWish)
        {
            bool8 lunar = side->lunarDance;
            side->healingWish = side->lunarDance = FALSE;
            gBattleMoveDamage = -(s32)(gBattleMons[battler].maxHP - gBattleMons[battler].hp);
            gBattleMons[battler].status1 = 0;   // sent to the games by G4_ARRIVE_SYNC
            gGen4Field.arriveLunar = lunar;
            gBattleScripting.battler = battler;
            return lunar ? BattleScript_G4LunarDanceArrive : BattleScript_G4HealingWishArrive;
        }
    }
    if (!(b->switchInDone & G4_ARRIVE_ROCK))
    {
        b->switchInDone |= G4_ARRIVE_ROCK;
        if (side->stealthRock && gBattleMons[battler].ability != ABILITY_MAGIC_GUARD)
        {
            gBattleMoveDamage = gBattleMons[battler].maxHP * RockEffectiveness16(battler) / 128;
            if (gBattleMoveDamage == 0)
                gBattleMoveDamage = 1;
            gBattleScripting.battler = battler;
            return bsArg == BS_TARGET ? BattleScript_G4RockOnTarget : bsArg == BS_ATTACKER ? BattleScript_G4RockOnAttacker
                 : BattleScript_G4RockOnFainted;
        }
    }
    if (!(b->switchInDone & G4_ARRIVE_TSPIKE))
    {
        b->switchInDone |= G4_ARRIVE_TSPIKE;
        if (side->toxicSpikes && MP_Gen4_IsGrounded(battler))
        {
            gBattleScripting.battler = battler;
            gEffectBattler = battler;
            if (MP_Gen4_HasType(battler, TYPE_POISON))
            {
                side->toxicSpikes = 0;
                return BattleScript_G4ToxicSpikesGone;
            }
            if (!MP_Gen4_HasType(battler, TYPE_STEEL) && !(gBattleMons[battler].status1 & STATUS1_ANY)
             && !(gSideStatuses[GetBattlerSide(battler)] & SIDE_STATUS_SAFEGUARD)
             && gBattleMons[battler].ability != ABILITY_IMMUNITY)
            {
                SetStatus(battler, side->toxicSpikes >= 2 ? STATUS1_TOXIC_POISON : STATUS1_POISON);
                return side->toxicSpikes >= 2 ? BattleScript_G4ToxicSpikesBadlyPoisoned : BattleScript_G4ToxicSpikesPoisoned;
            }
        }
    }
    return NULL;
}

void MP_Gen4_ArrivalDone(u8 battler)
{
    gGen4Battlers[battler].switchInDone = 0;
}

// ---------------------------------------------------------------------------------------------- end of turn

enum {
    G4T_AQUA_RING, G4T_ABILITY_HEAL, G4T_ABILITY_HURT, G4T_ITEM_HEAL, G4T_ITEM_HURT, G4T_BAD_DREAMS,
    G4T_ITEM_POISON, G4T_ITEM_BURN, G4T_ABILITY_CURED, G4T_EMBARGO_END, G4T_HEAL_BLOCK_END, G4T_MAGNET_RISE_END,
    G4T_SLOW_START_END, G4T_GRAVITY_END, G4T_TRICK_ROOM_END, G4T_TAILWIND_END, G4T_LUCKY_CHANT_END, G4T_POISON_HEAL,
    G4T_DOWNLOAD, G4T_ANTICIPATION, G4T_FOREWARN, G4T_FRISK, G4T_MOLD_BREAKER, G4T_SLOW_START, G4T_SNOW_WARNING,
    G4T_GRAVITY_CANT, G4T_HEAL_BLOCKED, G4T_LIFE_ORB, G4T_AFTERMATH, G4T_ANGER_POINT,
    G4T_ITEM_WEAKENED, G4T_ITEM_RECOIL, G4T_MICLE, G4T_CUSTAP,
};

const u16 gGen4TurnStringIds[] =
{
    [G4T_AQUA_RING]       = STRINGID_G4_AQUA_RING_HEAL,
    [G4T_ABILITY_HEAL]    = STRINGID_G4_ABILITY_HEAL,
    [G4T_ABILITY_HURT]    = STRINGID_G4_ABILITY_HURT,
    [G4T_ITEM_HEAL]       = STRINGID_G4_ITEM_HEAL,
    [G4T_ITEM_HURT]       = STRINGID_G4_ITEM_HURT,
    [G4T_BAD_DREAMS]      = STRINGID_G4_BAD_DREAMS,
    [G4T_ITEM_POISON]     = STRINGID_G4_ITEM_POISON,
    [G4T_ITEM_BURN]       = STRINGID_G4_ITEM_BURN,
    [G4T_ABILITY_CURED]   = STRINGID_G4_ABILITY_CURED,
    [G4T_EMBARGO_END]     = STRINGID_G4_EMBARGO_END,
    [G4T_HEAL_BLOCK_END]  = STRINGID_G4_HEAL_BLOCK_END,
    [G4T_MAGNET_RISE_END] = STRINGID_G4_MAGNET_RISE_END,
    [G4T_SLOW_START_END]  = STRINGID_G4_SLOW_START_END,
    [G4T_GRAVITY_END]     = STRINGID_G4_GRAVITY_END,
    [G4T_TRICK_ROOM_END]  = STRINGID_G4_TRICK_ROOM_END,
    [G4T_TAILWIND_END]    = STRINGID_G4_TAILWIND_END,
    [G4T_LUCKY_CHANT_END] = STRINGID_G4_LUCKY_CHANT_END,
    [G4T_POISON_HEAL]     = STRINGID_G4_ABILITY_HEAL,
    [G4T_DOWNLOAD]        = STRINGID_G4_DOWNLOAD,
    [G4T_ANTICIPATION]    = STRINGID_G4_ANTICIPATION,
    [G4T_FOREWARN]        = STRINGID_G4_FOREWARN,
    [G4T_FRISK]           = STRINGID_G4_FRISK,
    [G4T_MOLD_BREAKER]    = STRINGID_G4_MOLD_BREAKER,
    [G4T_SLOW_START]      = STRINGID_G4_SLOW_START,
    [G4T_SNOW_WARNING]    = STRINGID_G4_SNOW_WARNING,
    [G4T_GRAVITY_CANT]    = STRINGID_G4_GRAVITY_CANT,
    [G4T_HEAL_BLOCKED]    = STRINGID_G4_HEAL_BLOCKED,
    [G4T_LIFE_ORB]        = STRINGID_G4_LIFE_ORB,
    [G4T_AFTERMATH]       = STRINGID_G4_AFTERMATH,
    [G4T_ANGER_POINT]     = STRINGID_G4_ANGER_POINT,
    [G4T_ITEM_WEAKENED]   = STRINGID_G4_ITEM_WEAKENED,
    [G4T_ITEM_RECOIL]     = STRINGID_G4_ITEM_RECOIL,
    [G4T_MICLE]           = STRINGID_G4_MICLE,
    [G4T_CUSTAP]          = STRINGID_G4_CUSTAP,
};

bool8 MP_Gen4_NoIndirectDamage(u8 battler)
{
    return gBattleMons[battler].ability == ABILITY_MAGIC_GUARD;
}

// HP to heal (as a negative damage), or 0 when it can't heal
static s32 HealAmount(u8 battler, u32 divisor)
{
    s32 heal;
    if (gBattleMons[battler].hp == 0 || gBattleMons[battler].hp >= gBattleMons[battler].maxHP || gGen4Battlers[battler].healBlock)
        return 0;
    heal = gBattleMons[battler].maxHP / divisor;
    if (heal == 0)
        heal = 1;
    if (gBattleMons[battler].hp + heal > gBattleMons[battler].maxHP)
        heal = gBattleMons[battler].maxHP - gBattleMons[battler].hp;
    return -heal;
}

static s32 HurtAmount(u8 battler, u32 divisor)
{
    s32 dmg;
    if (gBattleMons[battler].hp == 0 || MP_Gen4_NoIndirectDamage(battler))
        return 0;
    dmg = gBattleMons[battler].maxHP / divisor;
    return dmg == 0 ? 1 : dmg;
}

static const u8 *TurnScript(const u8 *script, u8 string)
{
    gBattleCommunication[MULTISTRING_CHOOSER] = string;
    return script;
}

bool8 MP_Gen4_PoisonHeal(u8 battler)
{
    return gBattleMons[battler].ability == ABILITY_POISON_HEAL;
}

static bool8 AbilityPresentOnField(u16 ability)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
        if (gBattleMons[i].hp != 0 && gBattleMons[i].ability == ability)
            return TRUE;
    return FALSE;
}

static bool8 WeatherOn(u32 flags)
{
    return (gBattleWeather & flags) && !AbilityPresentOnField(ABILITY_CLOUD_NINE) && !AbilityPresentOnField(ABILITY_AIR_LOCK);
}

enum {
    G4E_AQUA_RING, G4E_POISON_HEAL, G4E_WEATHER_ABILITY, G4E_HYDRATION, G4E_BLACK_SLUDGE, G4E_BAD_DREAMS,
    G4E_ORB, G4E_STICKY_BARB, G4E_EMBARGO, G4E_HEAL_BLOCK, G4E_MAGNET_RISE, G4E_SLOW_START, G4E_DONE,
};

const u8 *MP_Gen4_BattlerEndTurn(u8 battler)
{
    struct Gen4Battler *b = &gGen4Battlers[battler];
    struct BattlePokemon *mon = &gBattleMons[battler];
    u8 hold = MP_Gen4_HoldEffect(battler);

    while (b->endStep < G4E_DONE)
    {
        switch (b->endStep++)
        {
        case G4E_AQUA_RING:
            if (b->aquaRing && (gBattleMoveDamage = HealAmount(battler, 16)) != 0)
                return TurnScript(BattleScript_G4TurnHeal, G4T_AQUA_RING);
            break;
        case G4E_POISON_HEAL:
            if (mon->ability == ABILITY_POISON_HEAL && (mon->status1 & STATUS1_PSN_ANY)
             && (gBattleMoveDamage = HealAmount(battler, 8)) != 0)
            {
                gLastUsedAbility = mon->ability;
                return TurnScript(BattleScript_G4TurnHeal, G4T_POISON_HEAL);
            }
            break;
        case G4E_WEATHER_ABILITY:
            gLastUsedAbility = mon->ability;
            switch (mon->ability)
            {
            case ABILITY_DRY_SKIN:
                if (WeatherOn(B_WEATHER_RAIN) && (gBattleMoveDamage = HealAmount(battler, 8)) != 0)
                    return TurnScript(BattleScript_G4TurnHeal, G4T_ABILITY_HEAL);
                if (WeatherOn(B_WEATHER_SUN) && (gBattleMoveDamage = HurtAmount(battler, 8)) != 0)
                    return TurnScript(BattleScript_G4TurnHurt, G4T_ABILITY_HURT);
                break;
            case ABILITY_SOLAR_POWER:
                if (WeatherOn(B_WEATHER_SUN) && (gBattleMoveDamage = HurtAmount(battler, 8)) != 0)
                    return TurnScript(BattleScript_G4TurnHurt, G4T_ABILITY_HURT);
                break;
            case ABILITY_ICE_BODY:
                if (WeatherOn(B_WEATHER_HAIL) && (gBattleMoveDamage = HealAmount(battler, 16)) != 0)
                    return TurnScript(BattleScript_G4TurnHeal, G4T_ABILITY_HEAL);
                break;
            }
            break;
        case G4E_HYDRATION:
            if (mon->ability == ABILITY_HYDRATION && WeatherOn(B_WEATHER_RAIN) && (mon->status1 & STATUS1_ANY) && mon->hp != 0)
            {
                gLastUsedAbility = mon->ability;
                SetStatus(battler, 0);
                return TurnScript(BattleScript_G4TurnStatus, G4T_ABILITY_CURED);
            }
            break;
        case G4E_BLACK_SLUDGE:
            if (hold != HOLD_EFFECT_BLACK_SLUDGE)
                break;
            gLastUsedItem = mon->item;
            if (MP_Gen4_HasType(battler, TYPE_POISON))
            {
                if ((gBattleMoveDamage = HealAmount(battler, 16)) != 0)
                    return TurnScript(BattleScript_G4TurnHeal, G4T_ITEM_HEAL);
            }
            else if ((gBattleMoveDamage = HurtAmount(battler, 8)) != 0)
            {
                return TurnScript(BattleScript_G4TurnHurt, G4T_ITEM_HURT);
            }
            break;
        case G4E_BAD_DREAMS:
        {
            u32 i;
            if (!(mon->status1 & STATUS1_SLEEP))
                break;
            for (i = 0; i < gBattlersCount; i++)
            {
                if (GetBattlerSide(i) != GetBattlerSide(battler) && gBattleMons[i].hp != 0 && gBattleMons[i].ability == ABILITY_BAD_DREAMS)
                {
                    if ((gBattleMoveDamage = HurtAmount(battler, 8)) != 0)
                        return TurnScript(BattleScript_G4TurnHurt, G4T_BAD_DREAMS);
                    break;
                }
            }
            break;
        }
        case G4E_ORB:
            if (mon->hp == 0 || (mon->status1 & STATUS1_ANY))
                break;
            gLastUsedItem = mon->item;
            if (hold == HOLD_EFFECT_TOXIC_ORB && !MP_Gen4_HasType(battler, TYPE_POISON) && !MP_Gen4_HasType(battler, TYPE_STEEL)
             && mon->ability != ABILITY_IMMUNITY)
            {
                SetStatus(battler, STATUS1_TOXIC_POISON);
                return TurnScript(BattleScript_G4TurnStatusAnim, G4T_ITEM_POISON);
            }
            if (hold == HOLD_EFFECT_FLAME_ORB && !MP_Gen4_HasType(battler, TYPE_FIRE) && mon->ability != ABILITY_WATER_VEIL)
            {
                SetStatus(battler, STATUS1_BURN);
                return TurnScript(BattleScript_G4TurnStatusAnim, G4T_ITEM_BURN);
            }
            break;
        case G4E_STICKY_BARB:
            if (hold == HOLD_EFFECT_STICKY_BARB && (gBattleMoveDamage = HurtAmount(battler, 8)) != 0)
            {
                gLastUsedItem = mon->item;
                return TurnScript(BattleScript_G4TurnHurt, G4T_ITEM_HURT);
            }
            break;
        case G4E_EMBARGO:
            if (b->embargo && --b->embargo == 0)
                return TurnScript(BattleScript_G4TurnMsg, G4T_EMBARGO_END);
            break;
        case G4E_HEAL_BLOCK:
            if (b->healBlock && --b->healBlock == 0)
                return TurnScript(BattleScript_G4TurnMsg, G4T_HEAL_BLOCK_END);
            break;
        case G4E_MAGNET_RISE:
            if (b->magnetRise && --b->magnetRise == 0)
                return TurnScript(BattleScript_G4TurnMsg, G4T_MAGNET_RISE_END);
            break;
        case G4E_SLOW_START:
            if (b->slowStart && --b->slowStart == 0 && mon->ability == ABILITY_SLOW_START)
                return TurnScript(BattleScript_G4TurnMsg, G4T_SLOW_START_END);
            break;
        }
    }
    b->endStep = 0;
    return NULL;
}

// the field's and sides' counters: GRAVITY, TRICK ROOM, TAILWIND, LUCKY CHANT
const u8 *MP_Gen4_FieldEndTurn(void)
{
    while (gGen4Field.endStep < 6)
    {
        u8 step = gGen4Field.endStep++;
        u8 side = step & 1;
        switch (step)
        {
        case 0:
            if (gGen4Field.gravity && --gGen4Field.gravity == 0)
                return TurnScript(BattleScript_G4TurnMsg, G4T_GRAVITY_END);
            break;
        case 1:
            if (gGen4Field.trickRoom && --gGen4Field.trickRoom == 0)
                return TurnScript(BattleScript_G4TurnMsg, G4T_TRICK_ROOM_END);
            break;
        case 2:
        case 3:
            if (gGen4Sides[side].tailwind && --gGen4Sides[side].tailwind == 0)
            {
                gBattlerAttacker = GetBattlerAtPosition(side == B_SIDE_PLAYER ? B_POSITION_PLAYER_LEFT : B_POSITION_OPPONENT_LEFT);
                return TurnScript(BattleScript_G4TurnMsg, G4T_TAILWIND_END);
            }
            break;
        case 4:
        case 5:
            if (gGen4Sides[side].luckyChant && --gGen4Sides[side].luckyChant == 0)
            {
                gBattlerAttacker = GetBattlerAtPosition(side == B_SIDE_PLAYER ? B_POSITION_PLAYER_LEFT : B_POSITION_OPPONENT_LEFT);
                return TurnScript(BattleScript_G4TurnMsg, G4T_LUCKY_CHANT_END);
            }
            break;
        }
    }
    gGen4Field.endStep = 0;
    return NULL;
}

// ------------------------------------------------------------------------------------- arrival abilities

// how well a move of this type hits the battler: 0 (immune), or 1/4 .. 4 as 10ths x4 (40 = 1x)
static u32 TypeEffect40(u8 moveType, u8 battler)
{
    u32 f = 40, i = 0;
    u8 t0 = gBattleMons[battler].types[0], t1 = gBattleMons[battler].types[1];
    while (TYPE_EFFECT_ATK_TYPE(i) != TYPE_ENDTABLE)
    {
        if (TYPE_EFFECT_ATK_TYPE(i) == moveType)
        {
            u8 def = TYPE_EFFECT_DEF_TYPE(i), mul = TYPE_EFFECT_MULTIPLIER(i);
            u32 hits = (def == t0) + (def == t1 && t1 != t0);
            while (hits--)
                f = f * mul / 10;
        }
        i += 3;
    }
    return f;
}

static u8 FoeOf(u8 battler, u32 n)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
    {
        if (GetBattlerSide(i) != GetBattlerSide(battler) && gBattleMons[i].hp != 0 && !(gAbsentBattlerFlags & gBitTable[i]))
        {
            if (n-- == 0)
                return i;
        }
    }
    return MAX_BATTLERS_COUNT;
}

static u32 CountFoes(u8 battler)
{
    u32 n = 0;
    while (FoeOf(battler, n) != MAX_BATTLERS_COUNT)
        n++;
    return n;
}

static u8 SwitchInScript(u8 battler, const u8 *script, u8 string)
{
    gBattleScripting.battler = battler;
    gBattleCommunication[MULTISTRING_CHOOSER] = string;
    BattleScriptPushCursorAndCallback(script);
    return 1;
}

u8 MP_Gen4_SwitchInAbility(u8 battler)
{
    struct Gen4Battler *b = &gGen4Battlers[battler];
    u32 i, j, n = CountFoes(battler);

    if (b->abilityAnnounced || gBattleMons[battler].hp == 0)
        return 0;
    b->abilityAnnounced = TRUE;
    gLastUsedAbility = gBattleMons[battler].ability;

    switch (gBattleMons[battler].ability)
    {
    case ABILITY_DOWNLOAD:
    {
        u32 def = 0, spDef = 0;
        u8 stat;
        if (n == 0)
            return 0;
        for (i = 0; i < n; i++)
        {
            u8 foe = FoeOf(battler, i);
            def += gBattleMons[foe].defense;
            spDef += gBattleMons[foe].spDefense;
        }
        stat = def < spDef ? STAT_ATK : STAT_SPATK;
        if (gBattleMons[battler].statStages[stat] >= MAX_STAT_STAGE)
            return 0;
        gBattleMons[battler].statStages[stat]++;
        PREPARE_STAT_BUFFER(gBattleTextBuff1, stat);
        gBattleScripting.animArg1 = STAT_ANIM_PLUS1 + stat - 1;
        gBattleScripting.animArg2 = 0;
        return SwitchInScript(battler, BattleScript_G4AbilityStat, G4T_DOWNLOAD);
    }
    case ABILITY_ANTICIPATION:
        for (i = 0; i < n; i++)
        {
            u8 foe = FoeOf(battler, i);
            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                u16 move = gBattleMons[foe].moves[j];
                if (move == MOVE_NONE)
                    continue;
                if (gBattleMoves[move].effect == EFFECT_OHKO || gBattleMoves[move].effect == EFFECT_EXPLOSION
                 || (gBattleMoves[move].power != 0 && TypeEffect40(MP_Gen4_MoveType(move, foe), battler) > 40))
                    return SwitchInScript(battler, BattleScript_G4AbilityMsg, G4T_ANTICIPATION);
            }
        }
        return 0;
    case ABILITY_FOREWARN:
    {
        u16 best = MOVE_NONE;
        u8 bestFoe = 0;
        u32 bestPower = 0;
        for (i = 0; i < n; i++)
        {
            u8 foe = FoeOf(battler, i);
            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                u16 move;
                u32 power;
                if (gBattleMons[foe].moves[j] == MOVE_NONE)
                    continue;
                move = gBattleMons[foe].moves[j];
                power = gBattleMoves[move].power;
                if (gBattleMoves[move].effect == EFFECT_OHKO)
                    power = 150;
                else if (gBattleMoves[move].effect == EFFECT_COUNTER || gBattleMoves[move].effect == EFFECT_MIRROR_COAT
                      || gBattleMoves[move].effect == EFFECT_METAL_BURST)
                    power = 120;
                else if (power == 1)
                    power = 80;
                if (power > bestPower || (power == bestPower && (Random() & 1)))
                {
                    bestPower = power;
                    best = move;
                    bestFoe = foe;
                }
            }
        }
        if (best == MOVE_NONE)
            return 0;
        PREPARE_MOVE_BUFFER(gBattleTextBuff1, best);
        PREPARE_MON_NICK_BUFFER(gBattleTextBuff2, bestFoe, gBattlerPartyIndexes[bestFoe]);
        return SwitchInScript(battler, BattleScript_G4AbilityMsg, G4T_FOREWARN);
    }
    case ABILITY_FRISK:
    {
        u8 withItem[MAX_BATTLERS_COUNT], k = 0;
        for (i = 0; i < n; i++)
            if (gBattleMons[FoeOf(battler, i)].item != ITEM_NONE)
                withItem[k++] = FoeOf(battler, i);
        if (k == 0)
            return 0;
        gLastUsedItem = gBattleMons[withItem[Random() % k]].item;
        return SwitchInScript(battler, BattleScript_G4AbilityMsg, G4T_FRISK);
    }
    case ABILITY_SNOW_WARNING:
        if (gBattleWeather & B_WEATHER_HAIL_PERMANENT)
            return 0;
        gBattleWeather = B_WEATHER_HAIL_PERMANENT | B_WEATHER_HAIL_TEMPORARY;
        return SwitchInScript(battler, BattleScript_G4SnowWarning, G4T_SNOW_WARNING);
    case ABILITY_MOLD_BREAKER:
        return SwitchInScript(battler, BattleScript_G4AbilityMsg, G4T_MOLD_BREAKER);
    case ABILITY_SLOW_START:
        return SwitchInScript(battler, BattleScript_G4AbilityMsg, G4T_SLOW_START);
    }
    return 0;
}

// ----------------------------------------------------------------------------------------- move limits

static const u16 sGravityBanned[] = { MOVE_FLY, MOVE_BOUNCE, MOVE_HI_JUMP_KICK, MOVE_JUMP_KICK, MOVE_SPLASH, MOVE_MAGNET_RISE };
static const u16 sHealBlockBanned[] =
{
    MOVE_RECOVER, MOVE_SOFT_BOILED, MOVE_MILK_DRINK, MOVE_SLACK_OFF, MOVE_MOONLIGHT, MOVE_MORNING_SUN, MOVE_SYNTHESIS,
    MOVE_ROOST, MOVE_WISH, MOVE_REST, MOVE_SWALLOW, MOVE_HEAL_ORDER, MOVE_HEALING_WISH, MOVE_LUNAR_DANCE,
};

const u8 *MP_Gen4_MoveBanned(u8 battler, u16 move)
{
    if (gGen4Field.gravity && InList(sGravityBanned, ARRAY_COUNT(sGravityBanned), move))
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = G4T_GRAVITY_CANT;
        return BattleScript_G4MoveBanned;
    }
    if (gGen4Battlers[battler].healBlock && InList(sHealBlockBanned, ARRAY_COUNT(sHealBlockBanned), move))
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = G4T_HEAL_BLOCKED;
        return BattleScript_G4MoveBanned;
    }
    return NULL;
}

// ------------------------------------------------------------------------------------------- held items

bool8 MP_Gen4_FocusSash(u8 target)
{
    if (MP_Gen4_HoldEffect(target) == HOLD_EFFECT_FOCUS_SASH && gBattleMons[target].hp == gBattleMons[target].maxHP
     && gBattleMons[target].hp <= gBattleMoveDamage && !(gBattleMons[target].status2 & STATUS2_SUBSTITUTE))
    {
        gGen4Battlers[target].sashUsed = TRUE;
        return TRUE;
    }
    return FALSE;
}

void MP_Gen4_ItemLost(u8 battler)
{
    if (gBattleMons[battler].ability == ABILITY_UNBURDEN)
        gGen4Battlers[battler].unburden = TRUE;
}

u8 MP_Gen4_ScreenTurns(u8 battler)
{
    return MP_Gen4_HoldEffect(battler) == HOLD_EFFECT_LIGHT_CLAY ? 8 : 5;
}

u8 MP_Gen4_WeatherTurns(u8 battler, u8 rockHoldEffect)
{
    return MP_Gen4_HoldEffect(battler) == rockHoldEffect ? 8 : 5;
}

// ---------------------------------------------------------------------------------------------- move end

#define G4M_SASH        (1 << 0)
#define G4M_ANGER_POINT (1 << 1)
#define G4M_AFTERMATH   (1 << 2)
#define G4M_LIFE_ORB    (1 << 3)
#define G4M_RESIST      (1 << 4)
#define G4M_JABOCA      (1 << 5)
#define G4M_MICLE       (1 << 6)

const u8 *MP_Gen4_MoveEnd(void)
{
    u8 atk = gBattlerAttacker, def = gBattlerTarget;
    u8 *done = &gGen4Field.moveEndDone;

    if (*done == 0)
        gGen4Battlers[atk].micle = FALSE;   // MICLE BERRY: this move had the boost
    if (!(*done & G4M_RESIST))
    {
        *done |= G4M_RESIST;
        if (def < gBattlersCount && gGen4Battlers[def].resistBerry)
        {
            gGen4Battlers[def].resistBerry = FALSE;
            if (gBattleMons[def].item != ITEM_NONE)
            {
                gLastUsedItem = gBattleMons[def].item;
                gBattleCommunication[MULTISTRING_CHOOSER] = G4T_ITEM_WEAKENED;
                return BattleScript_G4TargetBerryMsg;
            }
        }
    }
    if (!(*done & G4M_JABOCA))
    {
        u8 hold = def < gBattlersCount ? MP_Gen4_HoldEffect(def) : HOLD_EFFECT_NONE;
        *done |= G4M_JABOCA;
        if ((hold == HOLD_EFFECT_JABOCA_BERRY || hold == HOLD_EFFECT_ROWAP_BERRY) && def != atk && gGen4Field.dealtDamage
         && (hold == HOLD_EFFECT_JABOCA_BERRY) == MP_IsPhysicalMove(gCurrentMove, MP_Gen4_MoveType(gCurrentMove, atk))
         && gBattleMons[atk].hp != 0 && !MP_Gen4_NoIndirectDamage(atk))
        {
            gLastUsedItem = gBattleMons[def].item;
            gBattleMoveDamage = gBattleMons[atk].maxHP / 8;
            if (gBattleMoveDamage == 0)
                gBattleMoveDamage = 1;
            gBattleCommunication[MULTISTRING_CHOOSER] = G4T_ITEM_RECOIL;
            return BattleScript_G4BerryHurtsAttacker;
        }
    }
    if (!(*done & G4M_MICLE))
    {
        *done |= G4M_MICLE;
        if (def < gBattlersCount && def != atk && MP_Gen4_HoldEffect(def) == HOLD_EFFECT_MICLE_BERRY && gBattleMons[def].hp != 0
         && gBattleMons[def].hp <= gBattleMons[def].maxHP / 4)
        {
            gGen4Battlers[def].micle = TRUE;
            gLastUsedItem = gBattleMons[def].item;
            gBattleScripting.battler = def;
            gBattleCommunication[MULTISTRING_CHOOSER] = G4T_MICLE;
            return BattleScript_G4TargetBerryMsg;
        }
    }

    if (!(*done & G4M_SASH))
    {
        *done |= G4M_SASH;
        if (def < gBattlersCount && gGen4Battlers[def].sashUsed)
        {
            gGen4Battlers[def].sashUsed = FALSE;
            if (gBattleMons[def].item != ITEM_NONE)
            {
                gLastUsedItem = gBattleMons[def].item;
                return BattleScript_G4RemoveTargetItem;
            }
        }
    }
    if (!(*done & G4M_ANGER_POINT))
    {
        *done |= G4M_ANGER_POINT;
        if (def < gBattlersCount && def != atk && gBattleMons[def].hp != 0 && gBattleMons[def].ability == ABILITY_ANGER_POINT
         && gCritMultiplier >= 2 && gGen4Field.dealtDamage && !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
         && gBattleMons[def].statStages[STAT_ATK] < MAX_STAT_STAGE)
        {
            gBattleMons[def].statStages[STAT_ATK] = MAX_STAT_STAGE;
            gBattleScripting.animArg1 = STAT_ANIM_PLUS2 + STAT_ATK - 1;
            gBattleScripting.animArg2 = 0;
            return BattleScript_G4AngerPoint;
        }
    }
    if (!(*done & G4M_AFTERMATH))
    {
        *done |= G4M_AFTERMATH;
        if (def < gBattlersCount && def != atk && gBattleMons[def].hp == 0 && gBattleMons[def].ability == ABILITY_AFTERMATH
         && (gBattleMoves[gCurrentMove].flags & FLAG_MAKES_CONTACT) && gGen4Field.dealtDamage
         && gBattleMons[atk].hp != 0 && !MP_Gen4_NoIndirectDamage(atk) && !AbilityPresentOnField(ABILITY_DAMP))
        {
            gBattleMoveDamage = gBattleMons[atk].maxHP / 4;
            if (gBattleMoveDamage == 0)
                gBattleMoveDamage = 1;
            gBattleCommunication[MULTISTRING_CHOOSER] = G4T_AFTERMATH;
            return BattleScript_G4AttackerHurt;
        }
    }
    if (!(*done & G4M_LIFE_ORB))
    {
        *done |= G4M_LIFE_ORB;
        if (MP_Gen4_HoldEffect(atk) == HOLD_EFFECT_LIFE_ORB && gGen4Field.dealtDamage && gBattleMons[atk].hp != 0
         && !MP_Gen4_NoIndirectDamage(atk))
        {
            gBattleMoveDamage = gBattleMons[atk].maxHP / 10;
            if (gBattleMoveDamage == 0)
                gBattleMoveDamage = 1;
            gBattleCommunication[MULTISTRING_CHOOSER] = G4T_LIFE_ORB;
            return BattleScript_G4AttackerHurt;
        }
    }
    return NULL;
}

// CUSTAP BERRY: eaten as its holder moves first
const u8 *MP_Gen4_BeforeMove(u8 battler)
{
    if (MP_Gen4_HoldEffect(battler) == HOLD_EFFECT_CUSTAP_BERRY && gBattleMons[battler].hp != 0
     && gBattleMons[battler].hp <= gBattleMons[battler].maxHP / 4)
    {
        gLastUsedItem = gBattleMons[battler].item;
        gBattleCommunication[MULTISTRING_CHOOSER] = G4T_CUSTAP;
        return BattleScript_G4Custap;
    }
    return NULL;
}
