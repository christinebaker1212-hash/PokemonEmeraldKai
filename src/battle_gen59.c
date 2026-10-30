// Moemon Kai: the Gen 5-9 moves' effects (include/battle_gen59.h). Gen 4's hooks (battle_gen4.c) call in here;
// battle scripts reach it through the "gen4" command's G59_* modes (constants/battle_gen4.h).
#include "global.h"
#include "battle.h"
#include "battle_gen4.h"
#include "battle_gen59.h"
#include "battle_controllers.h"
#include "battle_main.h"
#include "battle_anim.h"
#include "battle_message.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "item.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "constants/abilities.h"
#include "constants/battle_gen4.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_string_ids.h"
#include "constants/battle_gen59_strings.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"

#include "data/battle_moves_gen59_data.h"
#include "data/move_flags_gen58.h"

static u32 MoveFlags(u16 move)
{
    return move < MOVES_COUNT ? sMoveFlagsGen58[move] : 0;
}

struct Gen59Battler gGen59Battlers[MAX_BATTLERS_COUNT];
struct Gen59Side gGen59Sides[2];
struct Gen59Field gGen59Field;

// the stat changes being worked through (G59_STATS_*)
static EWRAM_DATA u8 sStatNext = 0;
static EWRAM_DATA u8 sStatEnd = 0;
static EWRAM_DATA bool8 sStatDown = FALSE;

// ---------------------------------------------------------------------------------------------- messages

#define G59_TEXTS(X) \
    X(TERRAIN_GRASSY, "Grass grew to cover\nthe battlefield!") \
    X(TERRAIN_MISTY, "Mist swirled around\nthe battlefield!") \
    X(TERRAIN_ELECTRIC, "An electric current ran\nacross the battlefield!") \
    X(TERRAIN_PSYCHIC, "The battlefield got weird!") \
    X(TERRAIN_END, "The battlefield returned\nto normal.") \
    X(GRASSY_HEAL, "{B_ATK_NAME_WITH_PREFIX}'s HP was restored\nby the grassy terrain.") \
    X(TERRAIN_PROTECT, "{B_DEF_NAME_WITH_PREFIX} is protected\nby the terrain!") \
    X(WONDER_ROOM, "It created a bizarre area in which\nDEFENSE and SP. DEF stats are swapped!") \
    X(WONDER_ROOM_END, "WONDER ROOM wore off, and DEFENSE\nand SP. DEF stats returned to normal!") \
    X(MAGIC_ROOM, "It created a bizarre area in which\nPOKéMON's held items lose their effects!") \
    X(MAGIC_ROOM_END, "MAGIC ROOM wore off, and held items'\neffects returned to normal!") \
    X(TYPE_CHANGE, "{B_DEF_NAME_WITH_PREFIX} transformed\ninto the {B_BUFF1} type!") \
    X(TYPE_ADDED, "{B_BUFF1} type was added to\n{B_DEF_NAME_WITH_PREFIX}!") \
    X(REFLECT_TYPE, "{B_ATK_NAME_WITH_PREFIX}'s type became\nthe same as {B_DEF_NAME_WITH_PREFIX}'s!") \
    X(ABILITY_GIVEN, "{B_DEF_NAME_WITH_PREFIX} acquired\n{B_BUFF1}!") \
    X(GUARD_SPLIT, "{B_ATK_NAME_WITH_PREFIX} shared its\nguard with the target!") \
    X(POWER_SPLIT, "{B_ATK_NAME_WITH_PREFIX} shared its\npower with the target!") \
    X(SPEED_SWAP, "{B_ATK_NAME_WITH_PREFIX} switched SPEED\nwith its target!") \
    X(TOPSY_TURVY, "All stat changes on {B_DEF_NAME_WITH_PREFIX}\nwere inverted!") \
    X(CLEAR_SMOG, "{B_DEF_NAME_WITH_PREFIX}'s stat changes\nwere removed!") \
    X(INCINERATE, "{B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nwas burnt up!") \
    X(HP_RESTORED, "{B_DEF_NAME_WITH_PREFIX}'s HP\nwas restored.") \
    X(USER_HP_RESTORED, "{B_ATK_NAME_WITH_PREFIX}'s HP\nwas restored.") \
    X(SMACK_DOWN, "{B_DEF_NAME_WITH_PREFIX} fell\nstraight down!") \
    X(STICKY_WEB, "A sticky web spreads out on the\nground around {B_DEF_NAME_WITH_PREFIX}'s team!") \
    X(STICKY_WEB_CAUGHT, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} was caught\nin a sticky web!") \
    X(AURORA_VEIL, "AURORA VEIL made {B_ATK_NAME_WITH_PREFIX}'s team\nstronger against attacks!") \
    X(AURORA_VEIL_END, "{B_ATK_NAME_WITH_PREFIX}'s team's\nAURORA VEIL wore off!") \
    X(CELEBRATE, "Congratulations,\n{B_PLAYER_NAME}!") \
    X(HOLD_HANDS, "{B_ATK_NAME_WITH_PREFIX} and {B_DEF_NAME_WITH_PREFIX}\nare holding hands!") \
    X(HAPPY_HOUR, "Everyone is caught up in\nthe happy atmosphere!") \
    X(ION_DELUGE, "A deluge of ions showers\nthe battlefield!") \
    X(FAIRY_LOCK, "No one will be able to run away\nduring the next turn!") \
    X(PAY_HP, "{B_ATK_NAME_WITH_PREFIX} cut its own HP\nto power up!") \
    X(LASER_FOCUS, "{B_ATK_NAME_WITH_PREFIX} concentrated\nintensely!") \
    X(THROAT_CHOP, "{B_DEF_NAME_WITH_PREFIX} can't use\nsound moves for a while!") \
    X(NO_RETREAT, "{B_ATK_NAME_WITH_PREFIX} can no longer escape\nbecause it used NO RETREAT!") \
    X(TAR_SHOT, "{B_DEF_NAME_WITH_PREFIX} became\nweaker to fire!") \
    X(OCTOLOCK, "{B_DEF_NAME_WITH_PREFIX} can no longer\nescape because of OCTOLOCK!") \
    X(SALT_CURE, "{B_DEF_NAME_WITH_PREFIX} is being\nsalt cured!") \
    X(SALT_CURE_HURT, "{B_ATK_NAME_WITH_PREFIX} is hurt\nby SALT CURE!") \
    X(COURT_CHANGE, "{B_ATK_NAME_WITH_PREFIX} swapped the battle effects\naffecting each side of the field!") \
    X(CORROSIVE_GAS, "{B_ATK_NAME_WITH_PREFIX} corroded\n{B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}!") \
    X(BESTOW, "{B_DEF_NAME_WITH_PREFIX} received {B_LAST_ITEM}\nfrom {B_ATK_NAME_WITH_PREFIX}!") \
    X(STATUS_CURED, "{B_ATK_NAME_WITH_PREFIX}'s status\nreturned to normal!") \
    X(TARGET_CURED, "{B_DEF_NAME_WITH_PREFIX}'s status\nreturned to normal!") \
    X(ELECTRIFY, "{B_DEF_NAME_WITH_PREFIX}'s moves\nhave been electrified!") \
    X(POWDER, "{B_DEF_NAME_WITH_PREFIX} is covered\nin powder!") \
    X(AFTER_YOU, "{B_DEF_NAME_WITH_PREFIX} took the\nkind offer!") \
    X(QUASH, "{B_DEF_NAME_WITH_PREFIX}'s move\nwas postponed!") \
    X(TEATIME, "It's teatime! Everyone dug in\nto their BERRIES!") \
    X(STUFF_CHEEKS, "{B_ATK_NAME_WITH_PREFIX} ate its\n{B_LAST_ITEM}!") \
    X(MAGNETIC_FLUX, "{B_ATK_NAME_WITH_PREFIX}'s team's stats\nrose with the magnetic flux!") \
    X(SCREENS_BROKEN, "The wall shattered!") \
    X(DOODLE, "{B_ATK_NAME_WITH_PREFIX} copied\n{B_DEF_NAME_WITH_PREFIX}'s ability!") \
    X(HAZARDS_CLEARED, "{B_ATK_NAME_WITH_PREFIX} cleared away\nthe hazards!") \
    X(SPIKES_SCATTERED, "Spikes were scattered on the ground\naround {B_DEF_NAME_WITH_PREFIX}'s team!") \
    X(ROCKS_SCATTERED, "Pointed stones float in the air\naround {B_DEF_NAME_WITH_PREFIX}'s team!") \
    X(FELL_STINGER, "{B_ATK_NAME_WITH_PREFIX}'s ATTACK\ndrastically rose!") \
    X(INSTRUCT, "{B_DEF_NAME_WITH_PREFIX} followed\n{B_ATK_NAME_WITH_PREFIX}'s instructions!") \
    X(SNOW, "It started to snow!") \
    X(SYRUP_BOMB, "{B_DEF_NAME_WITH_PREFIX} got covered\nin sticky candy syrup!") \
    X(HEAL_BLOCKED, "{B_DEF_NAME_WITH_PREFIX} was prevented\nfrom healing!") \
    X(CANT_ESCAPE, "{B_DEF_NAME_WITH_PREFIX} can no\nlonger escape!") \
    X(REVIVED, "{B_BUFF1} was revived\nand is ready to fight again!") \
    X(DRAGON_CHEER, "{B_DEF_NAME_WITH_PREFIX} is getting\npumped!") \
    X(SHED_TAIL, "{B_ATK_NAME_WITH_PREFIX} shed its tail\nto create a decoy!") \
    X(TELEKINESIS, "{B_DEF_NAME_WITH_PREFIX} was hurled\ninto the air!") \
    X(STATS_RESET, "All stat changes were eliminated!") \
    X(ITEM_RAISED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nraised its {B_BUFF1}!") \
    X(ROCKY_HELMET, "{B_ATK_NAME_WITH_PREFIX} was hurt\nby the {B_LAST_ITEM}!") \
    X(ABILITY_RAISED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nraised its {B_BUFF1}!")

#define G59_ENUM(name, text) G59_MSG_##name,
enum { G59_TEXTS(G59_ENUM) G59_MSG_COUNT };
#define G59_TEXT(name, text) static const u8 sText_G59_##name[] = _(text);
G59_TEXTS(G59_TEXT)
#define G59_PTR(name, text) sText_G59_##name,
const u8 *const gGen59BattleTexts[] = { G59_TEXTS(G59_PTR) };
#define G59_ID(name, text) STRINGID_G59_FIRST + G59_MSG_##name,
const u16 gGen59StringIds[] = { G59_TEXTS(G59_ID) };
STATIC_ASSERT(G59_MSG_COUNT == STRINGID_G59_COUNT, Gen59StringCount);

// the message for printfromtable gGen59StringIds
static void Say(u8 msg)
{
    gBattleCommunication[MULTISTRING_CHOOSER] = msg;
}

// ---------------------------------------------------------------------------------------------- helpers

static const struct Gen59Move *Data(u16 move)
{
    return IS_GEN59_MOVE(move) ? &gGen59Moves[move - FIRST_GEN58_MOVE] : NULL;
}

static u16 Effect(u16 move)
{
    return gBattleMoves[move].effect;
}

static bool8 HasType(u8 battler, u8 type)
{
    return MP_Gen4_HasType(battler, type) || (gGen59Battlers[battler].thirdType == type && type != TYPE_MYSTERY);
}

static u32 Speed(u8 battler)
{
    u32 speed = gBattleMons[battler].speed * gStatStageRatios[gBattleMons[battler].statStages[STAT_SPEED]][0]
              / gStatStageRatios[gBattleMons[battler].statStages[STAT_SPEED]][1];
    if (gBattleMons[battler].status1 & STATUS1_PARALYSIS && gBattleMons[battler].ability != ABILITY_QUICK_FEET)
        speed /= 4;
    return speed ? speed : 1;
}

bool8 MP_Gen59_HoldsItem(u8 battler, u16 item)
{
    return gBattleMons[battler].item == item && !gGen4Battlers[battler].embargo && gBattleMons[battler].ability != ABILITY_KLUTZ
        && !gGen59Field.magicRoom;
}

static u32 Weight(u8 battler)
{
    u32 w = GetPokedexHeightWeight(SpeciesToNationalPokedexNum(gBattleMons[battler].species), 1);
    if (MP_Gen59_HoldsItem(battler, ITEM_FLOAT_STONE))
        w /= 2;
    if (gBattleMons[battler].ability == ABILITY_HEAVY_METAL)
        w *= 2;
    if (gBattleMons[battler].ability == ABILITY_LIGHT_METAL)
        w /= 2;
    return w ? w : 1;
}

static u32 PositiveStages(u8 battler)
{
    u32 i, n = 0;
    for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
        if (gBattleMons[battler].statStages[i] > DEFAULT_STAT_STAGE)
            n += gBattleMons[battler].statStages[i] - DEFAULT_STAT_STAGE;
    return n;
}

static bool8 IsDamaging(u16 move)
{
    return gBattleMoves[move].power != 0;
}

static u16 ChosenMove(u8 battler)
{
    if (gChosenActionByBattler[battler] != B_ACTION_USE_MOVE)
        return MOVE_NONE;
    return gBattleMons[battler].moves[*(gBattleStruct->chosenMovePositions + battler)];
}

static void SetItem(u8 battler, u16 item)
{
    gBattleMons[battler].item = item;
    gActiveBattler = battler;
    BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_HELDITEM_BATTLE, 0, sizeof(item), &gBattleMons[battler].item);
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

static void SetTypes(u8 battler, u8 type1, u8 type2)
{
    gBattleMons[battler].types[0] = type1;
    gBattleMons[battler].types[1] = type2;
}

static bool8 Hit(void)
{
    return !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT) && gBattleMons[gBattlerTarget].hp != 0;
}

static bool8 TargetBehindSubstitute(void)
{
    return (gBattleMons[gBattlerTarget].status2 & STATUS2_SUBSTITUTE) && gBattlerTarget != gBattlerAttacker;
}

static s32 Heal(u8 battler, u32 numerator, u32 denominator)
{
    s32 hp = gBattleMons[battler].maxHP * numerator / denominator;
    if (hp == 0)
        hp = 1;
    if (gBattleMons[battler].hp + hp > gBattleMons[battler].maxHP)
        hp = gBattleMons[battler].maxHP - gBattleMons[battler].hp;
    return -hp;
}

// ---------------------------------------------------------------------------------------------- battle state

void MP_Gen59_BattleStart(void)
{
    u32 i;
    memset(gGen59Battlers, 0, sizeof(struct Gen59Battler) * MAX_BATTLERS_COUNT);
    memset(gGen59Sides, 0, sizeof(gGen59Sides));
    memset(&gGen59Field, 0, sizeof(gGen59Field));
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        gGen59Battlers[i].thirdType = TYPE_MYSTERY;
}

void MP_Gen59_SwitchIn(u8 battler)
{
    memset(&gGen59Battlers[battler], 0, sizeof(gGen59Battlers[battler]));
    gGen59Battlers[battler].thirdType = TYPE_MYSTERY;
}

void MP_Gen59_TurnEnd(void)
{
    u32 i;
    for (i = 0; i < 2; i++)
    {
        gGen59Sides[i].faintedLastTurn = gGen59Sides[i].faintedThisTurn;
        gGen59Sides[i].faintedThisTurn = FALSE;
    }
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        gGen59Battlers[i].statsLowered = FALSE;
        if (gGen59Battlers[i].throatChop)
            gGen59Battlers[i].throatChop--;
        if (gGen59Battlers[i].laserFocus)
            gGen59Battlers[i].laserFocus--;
    }
    if (gGen59Field.echoedThisTurn)
    {
        if (gGen59Field.echoedVoice < 4)
            gGen59Field.echoedVoice++;
    }
    else
    {
        gGen59Field.echoedVoice = 0;
    }
    gGen59Field.echoedThisTurn = FALSE;
    gGen59Field.ionDeluge = FALSE;
    if (gGen59Field.fairyLock)
        gGen59Field.fairyLock--;
}

void MP_Gen59_OnDamaged(u8 battler, u8 attacker, s32 damage)
{
    if (battler != attacker && damage > 0 && gBattleMons[battler].item == ITEM_AIR_BALLOON)
        SetItem(battler, ITEM_NONE);   // popped
    if (battler != attacker && damage > 0 && gGen59Battlers[battler].hitsTaken < 6)
        gGen59Battlers[battler].hitsTaken++;
}

void MP_Gen59_MoveDone(u8 battler, u16 move, u8 resultFlags)
{
    gGen59Battlers[battler].lastMoveFailed = (resultFlags & MOVE_RESULT_NO_EFFECT) != 0;
    if (Effect(move) == EFFECT_ECHOED_VOICE && !(resultFlags & MOVE_RESULT_NO_EFFECT))
        gGen59Field.echoedThisTurn = TRUE;
}

void MP_Gen59_OnStatLowered(u8 battler)
{
    gGen59Battlers[battler].statsLowered = TRUE;
}

void MP_Gen59_OnFainted(u8 battler)
{
    struct Gen59Side *side = &gGen59Sides[GetBattlerSide(battler)];
    side->faintedThisTurn = TRUE;
    if (side->fainted < 100)
        side->fainted++;
}

u8 MP_Gen59_AteType(u16 move, u8 battler, u8 type);
u32 MP_Gen59_AbilityPower(u16 move, u8 attacker, u32 p);

// ---------------------------------------------------------------------------------------------- terrain

bool8 MP_Gen59_IsGrounded(u8 battler)
{
    if (gGen59Battlers[battler].smackedDown)
        return TRUE;
    return MP_Gen4_IsGrounded(battler);
}

bool8 MP_Gen59_StatusBlocked(u8 battler, u8 moveEffect)
{
    if (!gGen59Field.terrain || !MP_Gen59_IsGrounded(battler))
        return FALSE;
    if (gGen59Field.terrain == G59_TERRAIN_MISTY)
        return moveEffect <= MOVE_EFFECT_CONFUSION && moveEffect != 0;
    if (gGen59Field.terrain == G59_TERRAIN_ELECTRIC)
        return moveEffect == MOVE_EFFECT_SLEEP;
    return FALSE;
}

static const u8 sTerrainTypes[] = {TYPE_MYSTERY, TYPE_GRASS, TYPE_FAIRY, TYPE_ELECTRIC, TYPE_PSYCHIC};

// one end-of-turn field effect at a time (GRASSY TERRAIN's healing, SALT CURE, the rooms, terrain and AURORA VEIL
// running out); NULL when done
static EWRAM_DATA u8 sEndStep = 0;
static EWRAM_DATA u8 sEndBattler = 0;

const u8 *MP_Gen59_FieldEndTurn(void)
{
    u32 i;
    for (;;)
    {
        u8 step = sEndStep;
        if (step <= 1)
        {
            // 0: GRASSY TERRAIN heals the grounded, 1: SALT CURE hurts
            while (sEndBattler < gBattlersCount)
            {
                u8 b = gBattlerByTurnOrder[sEndBattler++];
                if (gBattleMons[b].hp == 0)
                    continue;
                if (step == 0 && gGen59Field.terrain == G59_TERRAIN_GRASSY && gBattleMons[b].hp < gBattleMons[b].maxHP
                 && MP_Gen59_IsGrounded(b) && !gGen4Battlers[b].healBlock)
                {
                    gBattlerAttacker = b;
                    gBattleMoveDamage = Heal(b, 1, 16);
                    Say(G59_MSG_GRASSY_HEAL);
                    return BattleScript_G59EndTurnHeal;
                }
                if (step == 1 && gGen59Battlers[b].saltCure && !MP_Gen4_NoIndirectDamage(b))
                {
                    gBattlerAttacker = b;
                    gBattleMoveDamage = gBattleMons[b].maxHP / (HasType(b, TYPE_WATER) || HasType(b, TYPE_STEEL) ? 4 : 8);
                    if (gBattleMoveDamage == 0)
                        gBattleMoveDamage = 1;
                    Say(G59_MSG_SALT_CURE_HURT);
                    return BattleScript_G59EndTurnHurt;
                }
            }
            sEndBattler = 0;
            sEndStep++;
            continue;
        }
        sEndStep++;
        switch (step)
        {
        case 2:
            if (gGen59Field.terrainTurns && --gGen59Field.terrainTurns == 0)
            {
                gGen59Field.terrain = G59_TERRAIN_NONE;
                Say(G59_MSG_TERRAIN_END);
                return BattleScript_G59EndTurnSay;
            }
            break;
        case 3:
            if (gGen59Field.wonderRoom && --gGen59Field.wonderRoom == 0)
            {
                Say(G59_MSG_WONDER_ROOM_END);
                return BattleScript_G59EndTurnSay;
            }
            break;
        case 4:
            if (gGen59Field.magicRoom && --gGen59Field.magicRoom == 0)
            {
                Say(G59_MSG_MAGIC_ROOM_END);
                return BattleScript_G59EndTurnSay;
            }
            break;
        case 5:
        case 6:
            i = step - 5;
            if (gGen59Sides[i].auroraVeil && --gGen59Sides[i].auroraVeil == 0)
            {
                gBattlerAttacker = GetBattlerAtPosition(i == B_SIDE_PLAYER ? B_POSITION_PLAYER_LEFT : B_POSITION_OPPONENT_LEFT);
                Say(G59_MSG_AURORA_VEIL_END);
                return BattleScript_G59EndTurnSay;
            }
            break;
        default:
            sEndStep = 0;
            sEndBattler = 0;
            return NULL;
        }
    }
}

// STICKY WEB on arrival: the script, or NULL
const u8 *MP_Gen59_ArrivalEffect(u8 battler)
{
    if (gGen59Sides[GetBattlerSide(battler)].stickyWeb && MP_Gen59_IsGrounded(battler) && !MP_Gen59_HoldsItem(battler, ITEM_HEAVY_DUTY_BOOTS)
     && gBattleMons[battler].statStages[STAT_SPEED] > MIN_STAT_STAGE && gBattleMons[battler].hp != 0)
    {
        gBattleScripting.battler = battler;
        gBattleMons[battler].statStages[STAT_SPEED]--;
        Say(G59_MSG_STICKY_WEB_CAUGHT);
        return BattleScript_G59StickyWeb;
    }
    return NULL;
}

// ---------------------------------------------------------------------------------------------- damage

u8 MP_Gen59_MoveType(u16 move, u8 battler, u8 type)
{
    switch (Effect(move))
    {
    case EFFECT_TERRAIN_PULSE:
        if (gGen59Field.terrain && MP_Gen59_IsGrounded(battler))
            type = sTerrainTypes[gGen59Field.terrain];
        break;
    case EFFECT_REVELATION_DANCE:
        type = gBattleMons[battler].types[0];
        break;
    case EFFECT_CHANGE_TYPE_ON_ITEM:   // TECHNO BLAST / MULTI-ATTACK: the DRIVE or MEMORY (none yet: their own type)
        break;
    case EFFECT_IVY_CUDGEL:
        break;
    }
    type = MP_Gen59_AteType(move, battler, type);
    if (gGen59Field.ionDeluge && type == TYPE_NORMAL)
        type = TYPE_ELECTRIC;
    return type;
}

s8 MP_Gen59_IsPhysical(u16 move, u8 battler)
{
    switch (Effect(move))
    {
    case EFFECT_PHOTON_GEYSER:
    case EFFECT_TERA_BLAST:
    case EFFECT_TERA_STARSTORM:
        return gBattleMons[battler].attack * gStatStageRatios[gBattleMons[battler].statStages[STAT_ATK]][0] / gStatStageRatios[gBattleMons[battler].statStages[STAT_ATK]][1]
             > gBattleMons[battler].spAttack * gStatStageRatios[gBattleMons[battler].statStages[STAT_SPATK]][0] / gStatStageRatios[gBattleMons[battler].statStages[STAT_SPATK]][1];
    case EFFECT_SHELL_SIDE_ARM:
    {
        u8 t = gBattlerTarget;
        return gBattleMons[battler].attack * gBattleMons[t].spDefense > gBattleMons[battler].spAttack * gBattleMons[t].defense;
    }
    }
    return -1;
}

u16 MP_Gen59_MovePower(u16 move, u8 attacker, u8 target, u16 power)
{
    const struct Gen59Move *d = Data(move);
    u32 p = power, r;
    u8 type = MP_Gen4_MoveType(move, attacker);

    switch (Effect(move))
    {
    case EFFECT_HEAT_CRASH:
        r = Weight(attacker) / Weight(target);
        p = r >= 5 ? 120 : r >= 4 ? 100 : r >= 3 ? 80 : r >= 2 ? 60 : 40;
        break;
    case EFFECT_ELECTRO_BALL:
        r = Speed(attacker) / Speed(target);
        p = r >= 4 ? 150 : r >= 3 ? 120 : r >= 2 ? 80 : r >= 1 ? 60 : 40;
        break;
    case EFFECT_STORED_POWER:
        p = 20 + 20 * PositiveStages(attacker);
        break;
    case EFFECT_ACROBATICS:
        if (gBattleMons[attacker].item == ITEM_NONE)
            p *= 2;
        break;
    case EFFECT_RETALIATE:
        if (gGen59Sides[GetBattlerSide(attacker)].faintedLastTurn)
            p *= 2;
        break;
    case EFFECT_ECHOED_VOICE:
        p = 40 * (1 + gGen59Field.echoedVoice);
        break;
    case EFFECT_DOUBLE_POWER_ON_ARG_STATUS:
        if (d && (gBattleMons[target].status1 & d->arg))
            p *= 2;
        break;
    case EFFECT_BOLT_BEAK:
        if (!gGen4Battlers[target].movedThisTurn)
            p *= 2;
        break;
    case EFFECT_LASH_OUT:
        if (gGen59Battlers[attacker].statsLowered)
            p *= 2;
        break;
    case EFFECT_STOMPING_TANTRUM:
        if (gGen59Battlers[attacker].lastMoveFailed)
            p *= 2;
        break;
    case EFFECT_POWER_BASED_ON_USER_HP:
        p = p * gBattleMons[attacker].hp / gBattleMons[attacker].maxHP;
        break;
    case EFFECT_POWER_BASED_ON_TARGET_HP:
        p = p * gBattleMons[target].hp / gBattleMons[target].maxHP;
        break;
    case EFFECT_RAGE_FIST:
        p = 50 + 50 * gGen59Battlers[attacker].hitsTaken;
        if (p > 350)
            p = 350;
        break;
    case EFFECT_LAST_RESPECTS:
        p = 50 + 50 * gGen59Sides[GetBattlerSide(attacker)].fainted;
        break;
    case EFFECT_GRAV_APPLE:
        if (gGen4Field.gravity)
            p = p * 15 / 10;
        break;
    case EFFECT_TERRAIN_BOOST:
        if ((move == MOVE_EXPANDING_FORCE && gGen59Field.terrain == G59_TERRAIN_PSYCHIC && MP_Gen59_IsGrounded(attacker))
         || (move == MOVE_MISTY_EXPLOSION && gGen59Field.terrain == G59_TERRAIN_MISTY && MP_Gen59_IsGrounded(attacker))
         || (move == MOVE_PSYBLADE && gGen59Field.terrain == G59_TERRAIN_ELECTRIC))
            p = p * 15 / 10;
        if (move == MOVE_RISING_VOLTAGE && gGen59Field.terrain == G59_TERRAIN_ELECTRIC && MP_Gen59_IsGrounded(target))
            p *= 2;
        break;
    case EFFECT_TERRAIN_PULSE:
        if (gGen59Field.terrain && MP_Gen59_IsGrounded(attacker))
            p *= 2;
        break;
    case EFFECT_FICKLE_BEAM:
        if (Random() % 10 < 3)
            p *= 2;
        break;
    case EFFECT_HYDRO_STEAM:   // in sunshine: 1.5x instead of halved
        if (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SUN))
            p *= 3;
        break;
    case EFFECT_FUSION_COMBO:
        break;
    case EFFECT_ROUND:
        break;
    }

    // terrain: 1.3x for the grounded user's moves of its type, GRASSY halves EARTHQUAKE-like moves, MISTY DRAGON ones
    if (gGen59Field.terrain && MP_Gen59_IsGrounded(attacker) && type == sTerrainTypes[gGen59Field.terrain]
     && gGen59Field.terrain != G59_TERRAIN_MISTY)
        p = p * 13 / 10;
    if (gGen59Field.terrain == G59_TERRAIN_GRASSY && MP_Gen59_IsGrounded(target)
     && (move == MOVE_EARTHQUAKE || move == MOVE_MAGNITUDE || move == MOVE_BULLDOZE))
        p /= 2;
    if (gGen59Field.terrain == G59_TERRAIN_MISTY && MP_Gen59_IsGrounded(target) && type == TYPE_DRAGON)
        p /= 2;
    // abilities
    switch (gBattleMons[attacker].ability)
    {
    case ABILITY_SHEER_FORCE:
        if (d && (d->status || d->targetChance || d->userChance))
            p = p * 13 / 10;
        break;
    case ABILITY_TOUGH_CLAWS:
        if (gBattleMoves[move].flags & FLAG_MAKES_CONTACT)
            p = p * 13 / 10;
        break;
    case ABILITY_STRONG_JAW:
        break;
    case ABILITY_SAND_FORCE:
        if (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SANDSTORM)
         && (type == TYPE_ROCK || type == TYPE_GROUND || type == TYPE_STEEL))
            p = p * 13 / 10;
        break;
    case ABILITY_ANALYTIC:
        if (gGen4Battlers[target].movedThisTurn)
            p = p * 13 / 10;
        break;
    case ABILITY_STEELWORKER:
        if (type == TYPE_STEEL)
            p = p * 15 / 10;
        break;
    case ABILITY_TRANSISTOR:
        if (type == TYPE_ELECTRIC)
            p = p * 13 / 10;
        break;
    case ABILITY_DRAGONS_MAW:
        if (type == TYPE_DRAGON)
            p = p * 15 / 10;
        break;
    case ABILITY_ROCKY_PAYLOAD:
        if (type == TYPE_ROCK)
            p = p * 15 / 10;
        break;
    }
    p = MP_Gen59_AbilityPower(move, attacker, p);
    if (type == TYPE_FAIRY && (MP_Gen59_HoldsItem(attacker, ITEM_PIXIE_PLATE) || MP_Gen59_HoldsItem(attacker, ITEM_FAIRY_FEATHER)))
        p = p * 12 / 10;
    return p ? p : 1;
}

void MP_Gen59_AttackStats(u16 move, u8 attacker, u8 target, u16 *attack, u16 *spAttack, u16 *defense, u16 *spDefense)
{
    switch (Effect(move))
    {
    case EFFECT_PSYSHOCK:
        *spDefense = *defense;
        break;
    case EFFECT_FOUL_PLAY:
        *attack = gBattleMons[target].attack;
        break;
    case EFFECT_BODY_PRESS:
        *attack = gBattleMons[attacker].defense;
        break;
    }
    if (MP_Gen59_HoldsItem(target, ITEM_ASSAULT_VEST))
        *spDefense = *spDefense * 15 / 10;
    if (gGen59Field.wonderRoom)
    {
        u16 t = *defense;
        *defense = *spDefense;
        *spDefense = t;
    }
}

s32 MP_Gen59_FinalDamage(s32 damage, u16 move, u8 type, u8 attacker, u8 target, u8 resultFlags)
{
    if (damage <= 0 || (resultFlags & MOVE_RESULT_NO_EFFECT))
        return damage;
    if (Effect(move) == EFFECT_COLLISION_COURSE && (resultFlags & MOVE_RESULT_SUPER_EFFECTIVE))
        damage = damage * 4 / 3;
    if (type == TYPE_FIRE && gGen59Battlers[target].tarShot)
        damage *= 2;
    // AURORA VEIL: like REFLECT and LIGHT SCREEN at once (not on top of them)
    if (gGen59Sides[GetBattlerSide(target)].auroraVeil && gCritMultiplier == 1
     && !(gSideStatuses[GetBattlerSide(target)] & (SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN)))
        damage = (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? damage * 2 / 3 : damage / 2;
    switch (MP_Gen4_TargetAbility(attacker, target))
    {
    case ABILITY_MULTISCALE:
    case ABILITY_SHADOW_SHIELD:
        if (gBattleMons[target].hp == gBattleMons[target].maxHP)
            damage /= 2;
        break;
    case ABILITY_FLUFFY:
        if (type == TYPE_FIRE)
            damage *= 2;
        if (gBattleMoves[move].flags & FLAG_MAKES_CONTACT)
            damage /= 2;
        break;
    case ABILITY_PRISM_ARMOR:
        if (resultFlags & MOVE_RESULT_SUPER_EFFECTIVE)
            damage = damage * 3 / 4;
        break;
    case ABILITY_ICE_SCALES:
        if (!MP_IsPhysicalMove(move, type))
            damage /= 2;
        break;
    case ABILITY_PUNK_ROCK:
        if (MoveFlags(move) & MP_MOVE_FLAG_SOUND_MOVE)
            damage /= 2;
        break;
    }
    if (gBattleMons[attacker].ability == ABILITY_PUNK_ROCK && (MoveFlags(move) & MP_MOVE_FLAG_SOUND_MOVE))
        damage = damage * 13 / 10;
    if (gBattleMons[attacker].ability == ABILITY_NEUROFORCE && (resultFlags & MOVE_RESULT_SUPER_EFFECTIVE))
        damage = damage * 5 / 4;
    return damage ? damage : 1;
}

bool8 MP_Gen59_AlwaysCrit(u16 move, u8 attacker)
{
    if (gGen59Battlers[attacker].laserFocus)
        return TRUE;
    switch (Effect(move))
    {
    case EFFECT_STORM_THROW:
    case EFFECT_FROST_BREATH:
    case EFFECT_WICKED_BLOW:
    case EFFECT_SURGING_STRIKES:
    case EFFECT_FLOWER_TRICK:
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------- stat changes

static const s8 *Stages(const struct Gen59Move *d, u8 i)
{
    return i < NUM_BATTLE_STATS - 1 ? &d->userStages[i] : &d->targetStages[i - (NUM_BATTLE_STATS - 1)];
}

static bool8 CanChange(u8 battler, u8 stat, s8 by)
{
    s8 stage = gBattleMons[battler].statStages[stat];
    return by > 0 ? stage < MAX_STAT_STAGE : stage > MIN_STAT_STAGE;
}

static bool8 AnyChangePossible(const struct Gen59Move *d)
{
    u32 i;
    for (i = 0; i < NUM_BATTLE_STATS - 1; i++)
    {
        if (d->userStages[i] && CanChange(gBattlerAttacker, STAT_ATK + i, d->userStages[i]))
            return TRUE;
        if (d->targetStages[i] && !TargetBehindSubstitute() && CanChange(gBattlerTarget, STAT_ATK + i, d->targetStages[i]))
            return TRUE;
    }
    return FALSE;
}

static bool8 Roll(u8 chance)
{
    u32 c = chance;
    if (c == 0)
        return TRUE;
    if (gBattleMons[gBattlerAttacker].ability == ABILITY_SERENE_GRACE)
        c *= 2;
    if (gBattleMons[gBattlerAttacker].ability == ABILITY_SHEER_FORCE)
        return FALSE;
    return Random() % 100 < c;
}

// METEOR BEAM / ELECTRO SHOT raise SP. ATK as they charge (ELECTRO SHOT in rain hits at once), GEOMANCY its stats
// on the second turn
static bool8 ChargeStatsNow(u16 move)
{
    bool8 charging = (gBattleMons[gBattlerAttacker].status2 & STATUS2_MULTIPLETURNS) != 0;
    switch (Effect(move))
    {
    case EFFECT_METEOR_BEAM:
        return charging;
    case EFFECT_ELECTRO_SHOT:
        return charging || (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_RAIN));
    case EFFECT_GEOMANCY:
        return !charging;
    }
    return TRUE;
}

// ---------------------------------------------------------------------------------------------- the commands

static u8 StatusMove(void);
static u8 BeforeHit(void);
static u8 AfterHit(void);

u8 MP_Gen59_Command(u8 mode)
{
    u16 move = gCurrentMove;
    const struct Gen59Move *d = Data(move);
    u8 atk = gBattlerAttacker;

    switch (mode)
    {
    case G59_STATS_CANT:
        return d == NULL || !AnyChangePossible(d);
    case G59_STATS_SELF:
        sStatNext = sStatEnd = 0;
        if (d && (IsDamaging(move) ? Roll(d->userChance) : TRUE) && gBattleMons[atk].hp != 0 && ChargeStatsNow(move))
            sStatEnd = NUM_BATTLE_STATS - 1;
        return 0;
    case G59_ONE_TURN_NOW:
        return Effect(move) == EFFECT_ELECTRO_SHOT && WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_RAIN);
    case G59_IS_STATUS:
        return !IsDamaging(move);
    case G59_STATS_TARGET:
        sStatNext = sStatEnd = NUM_BATTLE_STATS - 1;
        if (d && gBattleMons[gBattlerTarget].hp != 0 && !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
         && (IsDamaging(move) ? Roll(d->targetChance) && !TargetBehindSubstitute() : TRUE))
            sStatEnd = 2 * (NUM_BATTLE_STATS - 1);
        return 0;
    case G59_STATS_NEXT:
        while (sStatNext < sStatEnd)
        {
            s8 by = *Stages(d, sStatNext);
            u8 stat = STAT_ATK + sStatNext % (NUM_BATTLE_STATS - 1);
            sStatNext++;
            if (by)
            {
                sStatDown = by < 0;
                SET_STATCHANGER(stat, by < 0 ? -by : by, sStatDown);
                return 0;
            }
        }
        return 1;
    case G59_STAT_WAS_DOWN:
        return sStatDown;
    case G59_ROLL_STATUS:
        gBattleCommunication[MOVE_EFFECT_BYTE] = 0;
        if (d && d->status && Roll(d->statusChance) && !MP_Gen59_StatusBlocked(gBattlerTarget, d->status))
            gBattleCommunication[MOVE_EFFECT_BYTE] = d->status | MOVE_EFFECT_CERTAIN;
        return 0;
    case G59_BEFORE_HIT:
        return BeforeHit();
    case G59_AFTER_HIT:
        return AfterHit();
    case G59_STATUS_MOVE:
        return StatusMove();
    case G59_HALF_RECOIL:
        if (gBattleMons[atk].hp == 0 || gBattleMons[atk].ability == ABILITY_MAGIC_GUARD)
            return 1;
        gBattleMoveDamage = (gBattleMons[atk].maxHP + 1) / 2;
        return 0;
    case G59_CAN_DRAG:
        return gBattleMons[gBattlerTarget].hp == 0 || (gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
            || TargetBehindSubstitute() || gBattleMons[gBattlerTarget].ability == ABILITY_SUCTION_CUPS
            || (gStatuses3[gBattlerTarget] & STATUS3_ROOTED);
    case G59_GAMBIT:
        if (gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
            return 1;
        gBattleMoveDamage = gBattleMons[atk].hp;
        return 0;
    case G59_GAMBIT_SELF:
        gBattleMoveDamage = gBattleMons[atk].hp;
        return 0;
    case G59_PAY_HP:
    {
        u32 cost = gBattleMons[atk].maxHP / (Effect(move) == EFFECT_CLANGOROUS_SOUL ? 3 : 2);
        if (gBattleMons[atk].hp <= cost || (d && !AnyChangePossible(d)))
            return 1;
        gBattleMoveDamage = cost;
        Say(G59_MSG_PAY_HP);
        return 0;
    }
    }
    return 0;
}

// before the hit: TRUE (jump) if the move fails
static u8 BeforeHit(void)
{
    u8 atk = gBattlerAttacker, def = gBattlerTarget;
    u16 move = gCurrentMove;
    const struct Gen59Move *d = Data(move);

    switch (Effect(move))
    {
    case EFFECT_SYNCHRONOISE:
        return !(HasType(def, gBattleMons[atk].types[0]) || HasType(def, gBattleMons[atk].types[1]));
    case EFFECT_POLTERGEIST:
        return gBattleMons[def].item == ITEM_NONE;
    case EFFECT_FIRST_TURN_ONLY:
        return !gDisableStructs[atk].isFirstTurn;
    case EFFECT_FAIL_IF_NOT_ARG_TYPE:
        return d && !HasType(atk, d->arg);
    case EFFECT_THUNDERCLAP:
        return gGen4Battlers[def].movedThisTurn || !IsDamaging(ChosenMove(def));
    case EFFECT_UPPER_HAND:
        return gGen4Battlers[def].movedThisTurn || ChosenMove(def) == MOVE_NONE || gBattleMoves[ChosenMove(def)].priority <= 0;
    case EFFECT_HYPERSPACE_HOLE:
    case EFFECT_HYPERSPACE_FURY:
    case EFFECT_PHANTOM_FORCE:
        gProtectStructs[def].protected = FALSE;
        return 0;
    case EFFECT_PSYCHIC_FANGS:
    case EFFECT_RAGING_BULL:
        gSideStatuses[GetBattlerSide(def)] &= ~(SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN);
        gGen59Sides[GetBattlerSide(def)].auroraVeil = 0;
        return 0;
    case EFFECT_SPECTRAL_THIEF:
    {
        u32 i;
        for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
        {
            if (gBattleMons[def].statStages[i] > DEFAULT_STAT_STAGE)
            {
                s32 s = gBattleMons[atk].statStages[i] + gBattleMons[def].statStages[i] - DEFAULT_STAT_STAGE;
                gBattleMons[atk].statStages[i] = s > MAX_STAT_STAGE ? MAX_STAT_STAGE : s;
                gBattleMons[def].statStages[i] = DEFAULT_STAT_STAGE;
            }
        }
        return 0;
    }
    }
    return 0;
}

// after the hit: the move's own effect; 1 (jump) when there's nothing to say, else a message is chosen
static u8 AfterHit(void)
{
    u8 atk = gBattlerAttacker, def = gBattlerTarget;
    u16 move = gCurrentMove;
    u32 i;

    if (gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
        return 1;
    switch (Effect(move))
    {
    case EFFECT_SMACK_DOWN:
        if (gBattleMons[def].hp == 0 || MP_Gen59_IsGrounded(def))
            return 1;
        gGen59Battlers[def].smackedDown = TRUE;
        gGen4Battlers[def].magnetRise = 0;
        Say(G59_MSG_SMACK_DOWN);
        return 0;
    case EFFECT_CLEAR_SMOG:
        if (gBattleMons[def].hp == 0 || TargetBehindSubstitute())
            return 1;
        for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
            gBattleMons[def].statStages[i] = DEFAULT_STAT_STAGE;
        Say(G59_MSG_CLEAR_SMOG);
        return 0;
    case EFFECT_INCINERATE:
        if (TargetBehindSubstitute() || !IS_BERRY_ITEM(gBattleMons[def].item))
            return 1;
        gLastUsedItem = gBattleMons[def].item;
        SetItem(def, ITEM_NONE);
        Say(G59_MSG_INCINERATE);
        return 0;
    case EFFECT_FELL_STINGER:
        if (gBattleMons[def].hp != 0 || gBattleMons[atk].statStages[STAT_ATK] >= MAX_STAT_STAGE)
            return 1;
        gBattleMons[atk].statStages[STAT_ATK] += 3;
        if (gBattleMons[atk].statStages[STAT_ATK] > MAX_STAT_STAGE)
            gBattleMons[atk].statStages[STAT_ATK] = MAX_STAT_STAGE;
        Say(G59_MSG_FELL_STINGER);
        return 0;
    case EFFECT_THROAT_CHOP:
        if (gBattleMons[def].hp == 0)
            return 1;
        gGen59Battlers[def].throatChop = 2;
        return 1;
    case EFFECT_SALT_CURE:
        if (gBattleMons[def].hp == 0 || gGen59Battlers[def].saltCure)
            return 1;
        gGen59Battlers[def].saltCure = TRUE;
        Say(G59_MSG_SALT_CURE);
        return 0;
    case EFFECT_SPARKLING_ARIA:
        if (gBattleMons[def].hp == 0 || !(gBattleMons[def].status1 & STATUS1_BURN))
            return 1;
        SetStatus(def, gBattleMons[def].status1 & ~STATUS1_BURN);
        Say(G59_MSG_TARGET_CURED);
        return 0;
    case EFFECT_ICE_SPINNER:
    case EFFECT_STEEL_ROLLER:
        if (!gGen59Field.terrain)
            return 1;
        gGen59Field.terrain = G59_TERRAIN_NONE;
        gGen59Field.terrainTurns = 0;
        Say(G59_MSG_TERRAIN_END);
        return 0;
    case EFFECT_CEASELESS_EDGE:
        if (gSideTimers[GetBattlerSide(def)].spikesAmount >= 3)
            return 1;
        gSideStatuses[GetBattlerSide(def)] |= SIDE_STATUS_SPIKES;
        gSideTimers[GetBattlerSide(def)].spikesAmount++;
        Say(G59_MSG_SPIKES_SCATTERED);
        return 0;
    case EFFECT_STONE_AXE:
        if (gGen4Sides[GetBattlerSide(def)].stealthRock)
            return 1;
        gGen4Sides[GetBattlerSide(def)].stealthRock = TRUE;
        Say(G59_MSG_ROCKS_SCATTERED);
        return 0;
    case EFFECT_SPIRIT_SHACKLE:
    case EFFECT_ANCHOR_SHOT:
    case EFFECT_THOUSAND_WAVES:
    case EFFECT_JAW_LOCK:
        if (gBattleMons[def].hp == 0 || (gBattleMons[def].status2 & STATUS2_ESCAPE_PREVENTION) || HasType(def, TYPE_GHOST))
            return 1;
        gBattleMons[def].status2 |= STATUS2_ESCAPE_PREVENTION;
        gDisableStructs[def].battlerPreventingEscape = atk;
        Say(G59_MSG_CANT_ESCAPE);
        return 0;
    case EFFECT_SYRUP_BOMB:
        return 1;
    case EFFECT_PSYCHIC_NOISE:
        if (gBattleMons[def].hp == 0 || gGen4Battlers[def].healBlock)
            return 1;
        gGen4Battlers[def].healBlock = 2;
        Say(G59_MSG_HEAL_BLOCKED);
        return 0;
    case EFFECT_GLITZY_GLOW:
        if (gSideStatuses[GetBattlerSide(atk)] & SIDE_STATUS_LIGHTSCREEN)
            return 1;
        gSideStatuses[GetBattlerSide(atk)] |= SIDE_STATUS_LIGHTSCREEN;
        gSideTimers[GetBattlerSide(atk)].lightscreenTimer = 5;
        gSideTimers[GetBattlerSide(atk)].lightscreenBattlerId = atk;
        return 1;
    case EFFECT_BADDY_BAD:
        if (gSideStatuses[GetBattlerSide(atk)] & SIDE_STATUS_REFLECT)
            return 1;
        gSideStatuses[GetBattlerSide(atk)] |= SIDE_STATUS_REFLECT;
        gSideTimers[GetBattlerSide(atk)].reflectTimer = 5;
        gSideTimers[GetBattlerSide(atk)].reflectBattlerId = atk;
        return 1;
    case EFFECT_FREEZY_FROST:
        for (i = 0; i < gBattlersCount; i++)
        {
            u32 j;
            for (j = STAT_ATK; j < NUM_BATTLE_STATS; j++)
                gBattleMons[i].statStages[j] = DEFAULT_STAT_STAGE;
        }
        Say(G59_MSG_STATS_RESET);
        return 0;
    case EFFECT_SPARKLY_SWIRL:
        for (i = 0; i < gBattlersCount; i++)
        {
            if (GetBattlerSide(i) == GetBattlerSide(atk) && gBattleMons[i].status1 && gBattleMons[i].hp)
            {
                SetStatus(i, 0);
            }
        }
        return 1;
    case EFFECT_PLASMA_FISTS:
        if (gGen59Field.ionDeluge)
            return 1;
        gGen59Field.ionDeluge = TRUE;
        Say(G59_MSG_ION_DELUGE);
        return 0;
    case EFFECT_FAIL_IF_NOT_ARG_TYPE:   // BURN UP / DOUBLE SHOCK: the user loses that type
    {
        const struct Gen59Move *d = Data(move);
        if (d == NULL)
            return 1;
        for (i = 0; i < 2; i++)
            if (gBattleMons[atk].types[i] == d->arg)
                gBattleMons[atk].types[i] = TYPE_MYSTERY;
        return 1;
    }
    }
    return 1;
}

// a status move's own effect: 1 (jump) when it fails; a message may be chosen (MULTISTRING_CHOOSER, 0xFF = none)
static u8 StatusMove(void)
{
    u8 atk = gBattlerAttacker, def = gBattlerTarget;
    u16 move = gCurrentMove;
    const struct Gen59Move *d = Data(move);
    u32 i;
    u8 side = GetBattlerSide(atk), defSide = GetBattlerSide(def);

    Say(0xFF);
    switch (Effect(move))
    {
    case EFFECT_TERRAIN:
        if (!d || gGen59Field.terrain == d->arg)
            return 1;
        gGen59Field.terrain = d->arg;
        gGen59Field.terrainTurns = 5;
        Say(G59_MSG_TERRAIN_GRASSY + d->arg - G59_TERRAIN_GRASSY);
        return 0;
    case EFFECT_WONDER_ROOM:
        gGen59Field.wonderRoom = gGen59Field.wonderRoom ? 0 : 5;
        Say(gGen59Field.wonderRoom ? G59_MSG_WONDER_ROOM : G59_MSG_WONDER_ROOM_END);
        return 0;
    case EFFECT_MAGIC_ROOM:
        gGen59Field.magicRoom = gGen59Field.magicRoom ? 0 : 5;
        Say(gGen59Field.magicRoom ? G59_MSG_MAGIC_ROOM : G59_MSG_MAGIC_ROOM_END);
        return 0;
    case EFFECT_SOAK:
        if (!d || TargetBehindSubstitute() || (gBattleMons[def].types[0] == d->arg && gBattleMons[def].types[1] == d->arg))
            return 1;
        SetTypes(def, d->arg, d->arg);
        gGen59Battlers[def].thirdType = TYPE_MYSTERY;
        PREPARE_TYPE_BUFFER(gBattleTextBuff1, d->arg);
        Say(G59_MSG_TYPE_CHANGE);
        return 0;
    case EFFECT_THIRD_TYPE:
        if (!d || HasType(def, d->arg))
            return 1;
        gGen59Battlers[def].thirdType = d->arg;
        PREPARE_TYPE_BUFFER(gBattleTextBuff1, d->arg);
        Say(G59_MSG_TYPE_ADDED);
        return 0;
    case EFFECT_REFLECT_TYPE:
        SetTypes(atk, gBattleMons[def].types[0], gBattleMons[def].types[1]);
        gGen59Battlers[atk].thirdType = gGen59Battlers[def].thirdType;
        Say(G59_MSG_REFLECT_TYPE);
        return 0;
    case EFFECT_OVERWRITE_ABILITY:
        if (!d || gBattleMons[def].ability == d->arg || gBattleMons[def].ability == ABILITY_TRUANT)
            return 1;
        gBattleMons[def].ability = d->arg;
        PREPARE_ABILITY_BUFFER(gBattleTextBuff1, d->arg);
        Say(G59_MSG_ABILITY_GIVEN);
        return 0;
    case EFFECT_ENTRAINMENT:
    case EFFECT_DOODLE:
        if (gBattleMons[def].ability == gBattleMons[atk].ability || gBattleMons[atk].ability == ABILITY_TRUANT)
            return 1;
        if (Effect(move) == EFFECT_DOODLE)
        {
            gBattleMons[atk].ability = gBattleMons[def].ability;
            Say(G59_MSG_DOODLE);
        }
        else
        {
            gBattleMons[def].ability = gBattleMons[atk].ability;
            PREPARE_ABILITY_BUFFER(gBattleTextBuff1, gBattleMons[def].ability);
            Say(G59_MSG_ABILITY_GIVEN);
        }
        return 0;
    case EFFECT_GUARD_SPLIT:
    {
        u16 def2 = (gBattleMons[atk].defense + gBattleMons[def].defense) / 2;
        u16 spd2 = (gBattleMons[atk].spDefense + gBattleMons[def].spDefense) / 2;
        gBattleMons[atk].defense = gBattleMons[def].defense = def2;
        gBattleMons[atk].spDefense = gBattleMons[def].spDefense = spd2;
        Say(G59_MSG_GUARD_SPLIT);
        return 0;
    }
    case EFFECT_POWER_SPLIT:
    {
        u16 a2 = (gBattleMons[atk].attack + gBattleMons[def].attack) / 2;
        u16 s2 = (gBattleMons[atk].spAttack + gBattleMons[def].spAttack) / 2;
        gBattleMons[atk].attack = gBattleMons[def].attack = a2;
        gBattleMons[atk].spAttack = gBattleMons[def].spAttack = s2;
        Say(G59_MSG_POWER_SPLIT);
        return 0;
    }
    case EFFECT_SPEED_SWAP:
    {
        u16 t = gBattleMons[atk].speed;
        gBattleMons[atk].speed = gBattleMons[def].speed;
        gBattleMons[def].speed = t;
        Say(G59_MSG_SPEED_SWAP);
        return 0;
    }
    case EFFECT_TOPSY_TURVY:
    {
        bool8 any = FALSE;
        for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
        {
            if (gBattleMons[def].statStages[i] != DEFAULT_STAT_STAGE)
                any = TRUE;
            gBattleMons[def].statStages[i] = 2 * DEFAULT_STAT_STAGE - gBattleMons[def].statStages[i];
        }
        if (!any)
            return 1;
        Say(G59_MSG_TOPSY_TURVY);
        return 0;
    }
    case EFFECT_HEAL_PULSE:
        if (gBattleMons[def].hp == gBattleMons[def].maxHP || TargetBehindSubstitute() || gGen4Battlers[def].healBlock)
            return 1;
        gBattleMoveDamage = Heal(def, (move == MOVE_FLORAL_HEALING && gGen59Field.terrain == G59_TERRAIN_GRASSY) ? 2 : 1, 2);
        if (gBattleMons[atk].ability == ABILITY_MEGA_LAUNCHER)
            gBattleMoveDamage = Heal(def, 3, 4);
        Say(G59_MSG_HP_RESTORED);
        return 0;
    case EFFECT_LIFE_DEW:
    case EFFECT_JUNGLE_HEALING:
        // the user, by a quarter (its ally isn't healed yet)
        if (gBattleMons[atk].hp == gBattleMons[atk].maxHP && !(Effect(move) == EFFECT_JUNGLE_HEALING && gBattleMons[atk].status1))
            return 1;
        gBattleMoveDamage = Heal(atk, 1, 4);
        if (Effect(move) == EFFECT_JUNGLE_HEALING && gBattleMons[atk].status1)
        {
            SetStatus(atk, 0);
        }
        Say(G59_MSG_USER_HP_RESTORED);
        return 0;
    case EFFECT_SHORE_UP:
        if (gBattleMons[atk].hp == gBattleMons[atk].maxHP)
            return 1;
        gBattleMoveDamage = Heal(atk, (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SANDSTORM)) ? 2 : 1, (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SANDSTORM)) ? 3 : 2);
        Say(G59_MSG_USER_HP_RESTORED);
        return 0;
    case EFFECT_STRENGTH_SAP:
    {
        u32 a = gBattleMons[def].attack * gStatStageRatios[gBattleMons[def].statStages[STAT_ATK]][0] / gStatStageRatios[gBattleMons[def].statStages[STAT_ATK]][1];
        if (gBattleMons[def].statStages[STAT_ATK] == MIN_STAT_STAGE || TargetBehindSubstitute())
            return 1;
        gBattleMoveDamage = -(s32)(a ? a : 1);
        if (gBattleMons[atk].hp - gBattleMoveDamage > gBattleMons[atk].maxHP)
            gBattleMoveDamage = gBattleMons[atk].hp - gBattleMons[atk].maxHP;
        return 0;
    }
    case EFFECT_PURIFY:
        if (!gBattleMons[def].status1 || TargetBehindSubstitute())
            return 1;
        SetStatus(def, 0);
        gBattleMoveDamage = Heal(atk, 1, 2);
        Say(G59_MSG_USER_HP_RESTORED);
        return 0;
    case EFFECT_TAKE_HEART:
        if (gBattleMons[atk].status1)
        {
            SetStatus(atk, 0);
            Say(G59_MSG_STATUS_CURED);
            return 0;
        }
        return d && !AnyChangePossible(d);
    case EFFECT_STICKY_WEB:
        if (gGen59Sides[defSide].stickyWeb)
            return 1;
        gGen59Sides[defSide].stickyWeb = TRUE;
        Say(G59_MSG_STICKY_WEB);
        return 0;
    case EFFECT_AURORA_VEIL:
        if (gGen59Sides[side].auroraVeil || !(WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_HAIL)))
            return 1;
        gGen59Sides[side].auroraVeil = MP_Gen4_ScreenTurns(atk);
        Say(G59_MSG_AURORA_VEIL);
        return 0;
    case EFFECT_CELEBRATE:
        Say(G59_MSG_CELEBRATE);
        return 0;
    case EFFECT_HOLD_HANDS:
        if (!(gBattleTypeFlags & BATTLE_TYPE_DOUBLE) || def == atk)
            return 1;
        Say(G59_MSG_HOLD_HANDS);
        return 0;
    case EFFECT_HAPPY_HOUR:
        Say(G59_MSG_HAPPY_HOUR);
        return 0;
    case EFFECT_ION_DELUGE:
        gGen59Field.ionDeluge = TRUE;
        Say(G59_MSG_ION_DELUGE);
        return 0;
    case EFFECT_ELECTRIFY:
        if (gGen4Battlers[def].movedThisTurn)
            return 1;
        Say(G59_MSG_ELECTRIFY);
        return 0;
    case EFFECT_FAIRY_LOCK:
        if (gGen59Field.fairyLock)
            return 1;
        gGen59Field.fairyLock = 2;
        Say(G59_MSG_FAIRY_LOCK);
        return 0;
    case EFFECT_LASER_FOCUS:
        gGen59Battlers[atk].laserFocus = 2;
        Say(G59_MSG_LASER_FOCUS);
        return 0;
    case EFFECT_NO_RETREAT:
        if (gGen59Battlers[atk].noRetreat)
            return 1;
        gGen59Battlers[atk].noRetreat = TRUE;
        return d && !AnyChangePossible(d);
    case EFFECT_TAR_SHOT:
        if (TargetBehindSubstitute())
            return 1;
        if (gGen59Battlers[def].tarShot)
            return d && !AnyChangePossible(d);
        gGen59Battlers[def].tarShot = TRUE;
        Say(G59_MSG_TAR_SHOT);
        return 0;
    case EFFECT_OCTOLOCK:
        if (gGen59Battlers[def].octolock || HasType(def, TYPE_GHOST) || TargetBehindSubstitute())
            return 1;
        gGen59Battlers[def].octolock = TRUE;
        gBattleMons[def].status2 |= STATUS2_ESCAPE_PREVENTION;
        gDisableStructs[def].battlerPreventingEscape = atk;
        Say(G59_MSG_OCTOLOCK);
        return 0;
    case EFFECT_COURT_CHANGE:
    {
        u32 s = gSideStatuses[0];
        struct SideTimer t = gSideTimers[0];
        struct Gen4Side g4 = gGen4Sides[0];
        struct Gen59Side g59 = gGen59Sides[0];
        gSideStatuses[0] = gSideStatuses[1]; gSideStatuses[1] = s;
        gSideTimers[0] = gSideTimers[1]; gSideTimers[1] = t;
        gGen4Sides[0] = gGen4Sides[1]; gGen4Sides[1] = g4;
        gGen59Sides[0] = gGen59Sides[1]; gGen59Sides[1] = g59;
        Say(G59_MSG_COURT_CHANGE);
        return 0;
    }
    case EFFECT_CORROSIVE_GAS:
        if (gBattleMons[def].item == ITEM_NONE || gBattleMons[def].ability == ABILITY_STICKY_HOLD)
            return 1;
        gLastUsedItem = gBattleMons[def].item;
        SetItem(def, ITEM_NONE);
        Say(G59_MSG_CORROSIVE_GAS);
        return 0;
    case EFFECT_BESTOW:
        if (gBattleMons[atk].item == ITEM_NONE || gBattleMons[def].item != ITEM_NONE)
            return 1;
        gLastUsedItem = gBattleMons[atk].item;
        SetItem(def, gLastUsedItem);
        SetItem(atk, ITEM_NONE);
        Say(G59_MSG_BESTOW);
        return 0;
    case EFFECT_POWDER:
        Say(G59_MSG_POWDER);
        return 0;
    case EFFECT_AFTER_YOU:
        if (gGen4Battlers[def].movedThisTurn)
            return 1;
        Say(G59_MSG_AFTER_YOU);
        return 0;
    case EFFECT_QUASH:
        if (gGen4Battlers[def].movedThisTurn)
            return 1;
        Say(G59_MSG_QUASH);
        return 0;
    case EFFECT_TELEKINESIS:
        if (gGen4Battlers[def].magnetRise || gGen4Field.gravity)
            return 1;
        gGen4Battlers[def].magnetRise = 3;
        Say(G59_MSG_TELEKINESIS);
        return 0;
    case EFFECT_TEATIME:
    case EFFECT_STUFF_CHEEKS:
        if (!IS_BERRY_ITEM(gBattleMons[atk].item))
            return 1;
        gLastUsedItem = gBattleMons[atk].item;
        SetItem(atk, ITEM_NONE);
        gGen59Battlers[atk].ateBerry = TRUE;
        Say(Effect(move) == EFFECT_TEATIME ? G59_MSG_TEATIME : G59_MSG_STUFF_CHEEKS);
        return 0;
    case EFFECT_FLOWER_SHIELD:
    case EFFECT_ROTOTILLER:
        // only GRASS types (and for ROTOTILLER, grounded ones) are boosted: the user if it qualifies
        if (!HasType(atk, TYPE_GRASS) || (Effect(move) == EFFECT_ROTOTILLER && !MP_Gen59_IsGrounded(atk)))
            return 1;
        return d && !AnyChangePossible(d);
    case EFFECT_STAT_CHANGE_MAGNETIC:
        if (gBattleMons[atk].ability != ABILITY_PLUS && gBattleMons[atk].ability != ABILITY_MINUS)
            return 1;
        return d && !AnyChangePossible(d);
    case EFFECT_STAT_CHANGE_ON_STATUS:   // VENOM DRENCH: only a poisoned target
        if (!d || !(gBattleMons[def].status1 & d->arg))
            return 1;
        return !AnyChangePossible(d);
    case EFFECT_TIDY_UP:
        gSideStatuses[0] &= ~SIDE_STATUS_SPIKES;
        gSideStatuses[1] &= ~SIDE_STATUS_SPIKES;
        gSideTimers[0].spikesAmount = gSideTimers[1].spikesAmount = 0;
        for (i = 0; i < 2; i++)
        {
            gGen4Sides[i].stealthRock = FALSE;
            gGen4Sides[i].toxicSpikes = 0;
            gGen59Sides[i].stickyWeb = FALSE;
        }
        return d && !AnyChangePossible(d);
    case EFFECT_WEATHER:   // SNOWSCAPE: hail here
    case EFFECT_WEATHER_AND_SWITCH:   // CHILLY RECEPTION (no switch yet)
        if (gBattleWeather & B_WEATHER_HAIL_TEMPORARY)
            return 1;
        gBattleWeather = B_WEATHER_HAIL_TEMPORARY;
        gWishFutureKnock.weatherDuration = MP_Gen4_WeatherTurns(atk, HOLD_EFFECT_ICY_ROCK);
        Say(G59_MSG_SNOW);
        return 0;
    case EFFECT_DRAGON_CHEER:
        if (!(gBattleTypeFlags & BATTLE_TYPE_DOUBLE) || def == atk || (gBattleMons[def].status2 & STATUS2_FOCUS_ENERGY))
            return 1;
        gBattleMons[def].status2 |= STATUS2_FOCUS_ENERGY;
        Say(G59_MSG_DRAGON_CHEER);
        return 0;
    case EFFECT_TOXIC_THREAD:
    {
        bool8 poison = !(gBattleMons[def].status1 & STATUS1_ANY) && !HasType(def, TYPE_POISON) && !HasType(def, TYPE_STEEL)
                    && gBattleMons[def].ability != ABILITY_IMMUNITY && !TargetBehindSubstitute()
                    && !(gSideStatuses[defSide] & SIDE_STATUS_SAFEGUARD) && !MP_Gen59_StatusBlocked(def, MOVE_EFFECT_POISON);
        if (!poison)
            return d && !AnyChangePossible(d);
        SetStatus(def, gBattleMons[def].status1 | STATUS1_POISON);
        return 0;
    }
    case EFFECT_INSTRUCT:
        return 1;
    case EFFECT_ALLY_SWITCH:
        return 1;
    case EFFECT_MAT_BLOCK:
        return 1;
    case EFFECT_REVIVAL_BLESSING:
        return 1;
    }
    // stat moves
    if (d && (d->userStages[0] | d->userStages[1] | d->userStages[2] | d->userStages[3] | d->userStages[4] | d->userStages[5] | d->userStages[6]
            | d->targetStages[0] | d->targetStages[1] | d->targetStages[2] | d->targetStages[3] | d->targetStages[4] | d->targetStages[5] | d->targetStages[6]))
        return !AnyChangePossible(d);
    return 1;
}

// ---------------------------------------------------------------------------------------------- abilities

// moves from before Gen 5 that count as biting / pulse / slicing / healing moves (the newer ones carry flags)
static const u16 sOldBiting[] = {MOVE_BITE, MOVE_CRUNCH, MOVE_HYPER_FANG, MOVE_FIRE_FANG, MOVE_ICE_FANG, MOVE_THUNDER_FANG, MOVE_POISON_FANG};
static const u16 sOldPulse[] = {MOVE_WATER_PULSE, MOVE_DARK_PULSE, MOVE_DRAGON_PULSE, MOVE_AURA_SPHERE};
static const u16 sOldSlicing[] = {MOVE_CUT, MOVE_RAZOR_LEAF, MOVE_SLASH, MOVE_FURY_CUTTER, MOVE_AIR_SLASH, MOVE_LEAF_BLADE,
                                  MOVE_NIGHT_SLASH, MOVE_PSYCHO_CUT, MOVE_X_SCISSOR, MOVE_AIR_CUTTER, MOVE_AERIAL_ACE};
static const u16 sOldHealing[] = {MOVE_RECOVER, MOVE_SOFT_BOILED, MOVE_MILK_DRINK, MOVE_MOONLIGHT, MOVE_MORNING_SUN,
                                  MOVE_SYNTHESIS, MOVE_SLACK_OFF, MOVE_ROOST, MOVE_WISH, MOVE_HEAL_ORDER, MOVE_REST,
                                  MOVE_ABSORB, MOVE_MEGA_DRAIN, MOVE_GIGA_DRAIN, MOVE_DRAIN_PUNCH, MOVE_LEECH_LIFE,
                                  MOVE_DREAM_EATER, MOVE_SWALLOW, MOVE_HEALING_WISH, MOVE_LUNAR_DANCE};

static bool8 InMoves(const u16 *list, u32 n, u16 move)
{
    u32 i;
    for (i = 0; i < n; i++)
        if (list[i] == move)
            return TRUE;
    return FALSE;
}

static bool8 IsBiting(u16 move)  { return (MoveFlags(move) & MP_MOVE_FLAG_BITING_MOVE) || InMoves(sOldBiting, ARRAY_COUNT(sOldBiting), move); }
static bool8 IsPulse(u16 move)   { return (MoveFlags(move) & MP_MOVE_FLAG_PULSE_MOVE) || InMoves(sOldPulse, ARRAY_COUNT(sOldPulse), move); }
static bool8 IsSlicing(u16 move) { return (MoveFlags(move) & MP_MOVE_FLAG_SLICING_MOVE) || InMoves(sOldSlicing, ARRAY_COUNT(sOldSlicing), move); }
static bool8 IsHealing(u16 move) { return (MoveFlags(move) & MP_MOVE_FLAG_HEALING_MOVE) || InMoves(sOldHealing, ARRAY_COUNT(sOldHealing), move); }

// the "-ATE" abilities: a NORMAL move takes the ability's type
static u8 AteType(u16 ability)
{
    switch (ability)
    {
    case ABILITY_PIXILATE:    return TYPE_FAIRY;
    case ABILITY_REFRIGERATE: return TYPE_ICE;
    case ABILITY_AERILATE:    return TYPE_FLYING;
    case ABILITY_GALVANIZE:   return TYPE_ELECTRIC;
    }
    return TYPE_MYSTERY;
}

u8 MP_Gen59_AteType(u16 move, u8 battler, u8 type)
{
    u8 ate = AteType(gBattleMons[battler].ability);
    if (ate != TYPE_MYSTERY && type == TYPE_NORMAL && move != MOVE_STRUGGLE && IsDamaging(move))
        return ate;
    return type;
}

// the ability's boost to a move's power (after the move's own formula)
u32 MP_Gen59_AbilityPower(u16 move, u8 attacker, u32 p)
{
    switch (gBattleMons[attacker].ability)
    {
    case ABILITY_PIXILATE:
    case ABILITY_REFRIGERATE:
    case ABILITY_AERILATE:
    case ABILITY_GALVANIZE:
        if (gBattleMoves[move].type == TYPE_NORMAL && move != MOVE_STRUGGLE)
            p = p * 12 / 10;
        break;
    case ABILITY_STRONG_JAW:
        if (IsBiting(move))
            p = p * 15 / 10;
        break;
    case ABILITY_MEGA_LAUNCHER:
        if (IsPulse(move))
            p = p * 15 / 10;
        break;
    case ABILITY_SHARPNESS:
        if (IsSlicing(move))
            p = p * 15 / 10;
        break;
    }
    return p;
}

s8 MP_Gen59_Priority(u8 battler, u16 move)
{
    s8 priority = gBattleMoves[move].priority;
    if (move == MOVE_NONE)
        return priority;
    switch (gBattleMons[battler].ability)
    {
    case ABILITY_PRANKSTER:
        if (!IsDamaging(move))
            priority++;
        break;
    case ABILITY_GALE_WINGS:
        if (gBattleMoves[move].type == TYPE_FLYING && gBattleMons[battler].hp == gBattleMons[battler].maxHP)
            priority++;
        break;
    case ABILITY_TRIAGE:
        if (IsHealing(move))
            priority += 3;
        break;
    }
    if (Effect(move) == EFFECT_GRASSY_GLIDE && gGen59Field.terrain == G59_TERRAIN_GRASSY && MP_Gen59_IsGrounded(battler))
        priority++;
    return priority;
}

u32 MP_Gen59_Speed(u8 battler, u32 speed)
{
    bool8 weather = WEATHER_HAS_EFFECT2;
    switch (gBattleMons[battler].ability)
    {
    case ABILITY_SAND_RUSH:
        if (weather && (gBattleWeather & B_WEATHER_SANDSTORM))
            speed *= 2;
        break;
    case ABILITY_SLUSH_RUSH:
        if (weather && (gBattleWeather & B_WEATHER_HAIL))
            speed *= 2;
        break;
    case ABILITY_SURGE_SURFER:
        if (gGen59Field.terrain == G59_TERRAIN_ELECTRIC)
            speed *= 2;
        break;
    }
    return speed;
}

// PROTEAN / LIBERO: the user becomes the type of the move it's about to use
void MP_Gen59_OnMoveStart(u8 battler, u16 move)
{
    u16 ability = gBattleMons[battler].ability;
    gGen59Field.moveEndDone = 0;
    if ((ability == ABILITY_PROTEAN || ability == ABILITY_LIBERO) && move != MOVE_STRUGGLE && move != MOVE_NONE)
    {
        u8 type = MP_Gen4_MoveType(move, battler);
        SetTypes(battler, type, type);
        gGen59Battlers[battler].thirdType = TYPE_MYSTERY;
    }
}

static const u8 sTerrainMsg[] = {0, G59_MSG_TERRAIN_GRASSY, G59_MSG_TERRAIN_MISTY, G59_MSG_TERRAIN_ELECTRIC, G59_MSG_TERRAIN_PSYCHIC};

static u8 SetTerrain(u8 battler, u8 terrain)
{
    if (gGen59Field.terrain == terrain)
        return 0;
    gGen59Field.terrain = terrain;
    gGen59Field.terrainTurns = 5;
    gBattleScripting.battler = battler;
    Say(sTerrainMsg[terrain]);
    BattleScriptPushCursorAndCallback(BattleScript_G59AbilitySay);
    return 1;
}

static u8 RaiseOnArrival(u8 battler, u8 stat)
{
    if (gBattleMons[battler].statStages[stat] >= MAX_STAT_STAGE)
        return 0;
    gBattleMons[battler].statStages[stat]++;
    gBattleScripting.battler = battler;
    PREPARE_STAT_BUFFER(gBattleTextBuff1, stat);
    gBattleScripting.animArg1 = STAT_ANIM_PLUS1 + stat - 1;
    gBattleScripting.animArg2 = 0;
    Say(G59_MSG_ABILITY_RAISED);
    BattleScriptPushCursorAndCallback(BattleScript_G59AbilityStat);
    return 1;
}

// the Gen 5-9 abilities that act as their Pokémon comes in: 1 if a script was started
u8 MP_Gen59_SwitchInAbility(u8 battler)
{
    if (gBattleMons[battler].hp == 0 || gGen59Battlers[battler].arrived)
        return 0;
    gGen59Battlers[battler].arrived = TRUE;
    gLastUsedAbility = gBattleMons[battler].ability;
    switch (gBattleMons[battler].ability)
    {
    case ABILITY_ELECTRIC_SURGE:
    case ABILITY_HADRON_ENGINE:
        return SetTerrain(battler, G59_TERRAIN_ELECTRIC);
    case ABILITY_PSYCHIC_SURGE:
        return SetTerrain(battler, G59_TERRAIN_PSYCHIC);
    case ABILITY_GRASSY_SURGE:
        return SetTerrain(battler, G59_TERRAIN_GRASSY);
    case ABILITY_MISTY_SURGE:
        return SetTerrain(battler, G59_TERRAIN_MISTY);
    case ABILITY_INTREPID_SWORD:
        return RaiseOnArrival(battler, STAT_ATK);
    case ABILITY_DAUNTLESS_SHIELD:
        return RaiseOnArrival(battler, STAT_DEF);
    }
    return 0;
}

#define G59M_KO      (1 << 0)
#define G59M_ITEM    (1 << 1)
#define G59M_HELMET  (1 << 2)

static const u8 *ItemStatUp(u8 battler, u8 stat, u8 stat2)
{
    gLastUsedItem = gBattleMons[battler].item;
    gBattleScripting.battler = battler;
    if (gBattleMons[battler].statStages[stat] < MAX_STAT_STAGE)
        gBattleMons[battler].statStages[stat] = min(MAX_STAT_STAGE, gBattleMons[battler].statStages[stat] + (stat2 ? 2 : 1));
    if (stat2 && gBattleMons[battler].statStages[stat2] < MAX_STAT_STAGE)
        gBattleMons[battler].statStages[stat2] = min(MAX_STAT_STAGE, gBattleMons[battler].statStages[stat2] + 2);
    PREPARE_STAT_BUFFER(gBattleTextBuff1, stat);
    gBattleScripting.animArg1 = (stat2 ? STAT_ANIM_PLUS2 : STAT_ANIM_PLUS1) + stat - 1;
    gBattleScripting.animArg2 = 0;
    SetItem(battler, ITEM_NONE);
    Say(G59_MSG_ITEM_RAISED);
    return BattleScript_G59ItemStatMoveEnd;
}

// after a move, one effect at a time (the script, or NULL when done): MOXIE / BEAST BOOST for a knock-out, the
// target's WEAKNESS POLICY / ABSORB BULB / CELL BATTERY / SNOWBALL, the ROCKY HELMET
const u8 *MP_Gen59_MoveEnd(void)
{
    u8 atk = gBattlerAttacker, def = gBattlerTarget, stat = 0;
    u32 i, best = 0;
    bool8 hit = def < gBattlersCount && atk != def && IsDamaging(gCurrentMove) && !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
             && gGen4Battlers[def].hitThisTurn;
    u8 type = MP_Gen4_MoveType(gCurrentMove, atk);

    if (!(gGen59Field.moveEndDone & G59M_ITEM))
    {
        gGen59Field.moveEndDone |= G59M_ITEM;
        if (hit && gBattleMons[def].hp != 0 && !(gBattleMons[def].status2 & STATUS2_SUBSTITUTE))
        {
            if (MP_Gen59_HoldsItem(def, ITEM_WEAKNESS_POLICY) && (gMoveResultFlags & MOVE_RESULT_SUPER_EFFECTIVE)
             && (gBattleMons[def].statStages[STAT_ATK] < MAX_STAT_STAGE || gBattleMons[def].statStages[STAT_SPATK] < MAX_STAT_STAGE))
                return ItemStatUp(def, STAT_ATK, STAT_SPATK);
            if ((MP_Gen59_HoldsItem(def, ITEM_ABSORB_BULB) && type == TYPE_WATER && gBattleMons[def].statStages[STAT_SPATK] < MAX_STAT_STAGE))
                return ItemStatUp(def, STAT_SPATK, 0);
            if ((MP_Gen59_HoldsItem(def, ITEM_CELL_BATTERY) && type == TYPE_ELECTRIC && gBattleMons[def].statStages[STAT_ATK] < MAX_STAT_STAGE))
                return ItemStatUp(def, STAT_ATK, 0);
            if ((MP_Gen59_HoldsItem(def, ITEM_SNOWBALL) && type == TYPE_ICE && gBattleMons[def].statStages[STAT_ATK] < MAX_STAT_STAGE))
                return ItemStatUp(def, STAT_ATK, 0);
        }
    }
    if (!(gGen59Field.moveEndDone & G59M_HELMET))
    {
        gGen59Field.moveEndDone |= G59M_HELMET;
        if (hit && MP_Gen59_HoldsItem(def, ITEM_ROCKY_HELMET) && (gBattleMoves[gCurrentMove].flags & FLAG_MAKES_CONTACT)
         && gBattleMons[atk].hp != 0 && gBattleMons[atk].ability != ABILITY_MAGIC_GUARD)
        {
            gLastUsedItem = ITEM_ROCKY_HELMET;
            gBattleMoveDamage = gBattleMons[atk].maxHP / 6;
            if (gBattleMoveDamage == 0)
                gBattleMoveDamage = 1;
            Say(G59_MSG_ROCKY_HELMET);
            return BattleScript_G59RockyHelmet;
        }
    }
    if (gGen59Field.moveEndDone & G59M_KO)
        return NULL;
    gGen59Field.moveEndDone |= G59M_KO;
    if (def >= gBattlersCount || gBattleMons[def].hp != 0 || gBattleMons[atk].hp == 0 || atk == def
     || !IsDamaging(gCurrentMove) || (gMoveResultFlags & MOVE_RESULT_NO_EFFECT))
        return NULL;
    switch (gBattleMons[atk].ability)
    {
    case ABILITY_MOXIE:
        stat = STAT_ATK;
        break;
    case ABILITY_BEAST_BOOST:
        for (i = STAT_ATK; i <= STAT_SPDEF; i++)
        {
            u16 v = (&gBattleMons[atk].attack)[i - 1];
            if (v > best)
            {
                best = v;
                stat = i;
            }
        }
        break;
    default:
        return NULL;
    }
    if (stat == 0 || gBattleMons[atk].statStages[stat] >= MAX_STAT_STAGE)
        return NULL;
    gBattleMons[atk].statStages[stat]++;
    gLastUsedAbility = gBattleMons[atk].ability;
    gBattleScripting.battler = atk;
    PREPARE_STAT_BUFFER(gBattleTextBuff1, stat);
    gBattleScripting.animArg1 = STAT_ANIM_PLUS1 + stat - 1;
    gBattleScripting.animArg2 = 0;
    Say(G59_MSG_ABILITY_RAISED);
    return BattleScript_G59AbilityStatMoveEnd;
}
