// Moemon Kai: the Gen 5-9 moves' effects (include/battle_gen59.h). Gen 4's hooks (battle_gen4.c) call in here;
// battle scripts reach it through the "gen4" command's G59_* modes (constants/battle_gen4.h).
#include "global.h"
#include "battle.h"
#include "battle_gen4.h"
#include "battle_gen59.h"
#include "mail.h"
#include "battle_gimmick.h"
#include "mp.h"
#include "battle_util.h"
#include "mp_region.h"
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
    X(ABILITY_RAISED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nraised its {B_BUFF1}!") \
    X(ABILITY_LOWERED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1}\nfell!") \
    X(NERVOUS, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s foes are too\nnervous to eat BERRIES!") \
    X(SCREENS_CLEANED, "All screens on the field\nwere cleansed!") \
    X(SCR_HP_RESTORED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s HP\nwas restored.") \
    X(SUN_TURNED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} turned the\nsunlight harsh!") \
    X(SANDSTORM_UP, "A sandstorm kicked up!") \
    X(TOXIC_SPIKES, "Poison spikes were scattered on the\nground around {B_ATK_NAME_WITH_PREFIX}'s team!") \
    X(POISONED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} was poisoned!") \
    X(BADLY_POISONED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} was\nbadly poisoned!") \
    X(CONFUSED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} became\nconfused!") \
    X(STOLE_ITEM, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} stole\n{B_LAST_ITEM}!") \
    X(ABILITY_BECAME, "{B_ATK_NAME_WITH_PREFIX}'s ability\nbecame {B_BUFF1}!") \
    X(ABILITIES_SWAPPED, "{B_ATK_NAME_WITH_PREFIX} swapped abilities\nwith {B_DEF_NAME_WITH_PREFIX}!") \
    X(MOVE_DISABLED, "{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1}\nwas disabled!") \
    X(INNARDS_OUT, "{B_ATK_NAME_WITH_PREFIX} was hurt!") \
    X(DECOY, "Its disguise served it\nas a decoy!") \
    X(HEIGHTENED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1}\nwas heightened!") \
    X(SCR_CURED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s status\nreturned to normal!") \
    X(CHARGING, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} began\ncharging power!") \
    X(HARVESTED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} harvested\none {B_LAST_ITEM}!") \
    X(AURA, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} is radiating\nan aura!") \
    X(RUIN, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nweakened the POKéMON around it!") \
    X(BREAKS_MOLD, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} is radiating\na bursting aura!") \
    X(BLADE_FORME, "Changed to\nBLADE FORME!") \
    X(SHIELD_FORME, "Changed to\nSHIELD FORME!") \
    X(SCHOOL, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} formed\na school!") \
    X(SCHOOL_END, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} stopped\nschooling!") \
    X(HERO, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} underwent a\nheroic transformation!") \
    X(TERA_SHIFT, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} transformed!") \
    X(SHIELDS_DOWN, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s shields\nwent down!") \
    X(SHIELDS_UP, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s shields\nwent back up!") \
    X(COMMANDER, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} took command\nof its ally!") \
    X(CUD_CHEW, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} chewed its\n{B_LAST_ITEM} again!") \
    X(BALL_FETCH, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} found\none {B_LAST_ITEM}!") \
    X(SCR_ABILITY_BECAME, "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s ability\nbecame {B_BUFF1}!") \
    X(DANCER, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} joined the\ndance, and its stats rose!") \
    X(OPPORTUNIST, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} copied its\nfoe's stat changes!") \
    X(EMERGENCY_EXIT, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} wants\nto get away!") \
    X(FLED, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} fled\nfrom battle!") \
    X(GULP, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} caught\nsomething in its mouth!") \
    X(GULP_SPIT, "{B_ATK_NAME_WITH_PREFIX} was hit by\nwhat GULP MISSILE spat out!") \
    X(HANGRY, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} got\nhungry!") \
    X(SEEDED, "{B_DEF_NAME_WITH_PREFIX} was seeded!") \
    X(PP_REDUCED, "It reduced the PP of {B_DEF_NAME_WITH_PREFIX}'s\n{B_BUFF1} by 3!") \
    X(ABILITY_SUPPRESSED, "{B_DEF_NAME_WITH_PREFIX}'s ability\nwas suppressed!") \
    X(MEGA, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} has\nMEGA EVOLVED!") \
    X(PRIMAL, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} returned\nto its primal form!") \
    X(TERA, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} terastallized\ninto the {B_BUFF1} type!") \
    X(DYNAMAX, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} is\nDYNAMAXING!") \
    X(DYNAMAX_END, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} returned\nto its normal size!") \
    X(Z_POWER, "{B_SCR_ACTIVE_NAME_WITH_PREFIX} surrounded\nitself with its Z-POWER!") \
    X(SUN_UP, "The sunlight turned harsh!") \
    X(RAIN_UP, "It started to rain!") \
    X(HAIL_UP, "It started to hail!") \
    X(TEAM_STAT_UP, "{B_ATK_NAME_WITH_PREFIX}'s side's {B_BUFF1}\nrose!") \
    X(FOES_STAT_DOWN, "The foes' {B_BUFF1}\nfell!") \
    X(HEAVY_RAIN, "A heavy rain began to fall!") \
    X(HARSH_SUN, "The sunlight turned\nextremely harsh!") \
    X(STRONG_WINDS, "Mysterious strong winds are\nprotecting FLYING POKéMON!")

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

// ---------------------------------------------------------------------------------------------- ability helpers

// moves from before Gen 5 in the newer games' lists (the newer moves carry flags)
static const u16 sOldBallistic[] = {MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_ENERGY_BALL, MOVE_FOCUS_BLAST, MOVE_SEED_BOMB,
    MOVE_MUD_BOMB, MOVE_EGG_BOMB, MOVE_ZAP_CANNON, MOVE_WEATHER_BALL, MOVE_GYRO_BALL, MOVE_BULLET_SEED, MOVE_ROCK_BLAST,
    MOVE_ICE_BALL, MOVE_OCTAZOOKA, MOVE_AURA_SPHERE, MOVE_MIST_BALL, MOVE_BARRAGE, MOVE_MAGNET_BOMB, MOVE_ROCK_WRECKER};
static const u16 sOldPowder[] = {MOVE_SPORE, MOVE_SLEEP_POWDER, MOVE_STUN_SPORE, MOVE_POISON_POWDER, MOVE_COTTON_SPORE};
static const u16 sOldWind[] = {MOVE_GUST, MOVE_TWISTER, MOVE_AIR_CUTTER, MOVE_ICY_WIND, MOVE_OMINOUS_WIND, MOVE_SANDSTORM,
    MOVE_TAILWIND, MOVE_WHIRLWIND, MOVE_BLIZZARD, MOVE_HEAT_WAVE};

static bool8 InList16(const u16 *list, u32 n, u16 v)
{
    u32 i;
    for (i = 0; i < n; i++)
        if (list[i] == v)
            return TRUE;
    return FALSE;
}

static bool8 IsBallistic(u16 move) { return (MoveFlags(move) & MP_MOVE_FLAG_BALLISTIC_MOVE) || InList16(sOldBallistic, ARRAY_COUNT(sOldBallistic), move); }
static bool8 IsPowder(u16 move)    { return (MoveFlags(move) & MP_MOVE_FLAG_POWDER_MOVE) || InList16(sOldPowder, ARRAY_COUNT(sOldPowder), move); }
static bool8 IsWind(u16 move)      { return (MoveFlags(move) & MP_MOVE_FLAG_WIND_MOVE) || InList16(sOldWind, ARRAY_COUNT(sOldWind), move); }

static bool8 Alive(u8 b)
{
    return b < gBattlersCount && gBattleMons[b].hp != 0;
}

// another POKéMON on B's side has the ability
static bool8 AllyHas(u8 b, u16 ability)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
        if (i != b && Alive(i) && GetBattlerSide(i) == GetBattlerSide(b) && gBattleMons[i].ability == ability)
            return TRUE;
    return FALSE;
}

static bool8 SideHas(u8 b, u16 ability)
{
    return (Alive(b) && gBattleMons[b].ability == ability) || AllyHas(b, ability);
}

// any POKéMON on the field but EXCEPT has the ability
static bool8 FieldHas(u16 ability, u8 except)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
        if (i != except && Alive(i) && gBattleMons[i].ability == ability)
            return TRUE;
    return FALSE;
}

static bool8 MakesContact(u16 move, u8 attacker)
{
    return (gBattleMoves[move].flags & FLAG_MAKES_CONTACT) && gBattleMons[attacker].ability != ABILITY_LONG_REACH;
}

bool8 MP_Gen59_IgnoresAbilities(u8 attacker)
{
    u16 a = gBattleMons[attacker].ability;
    return a == ABILITY_TERAVOLT || a == ABILITY_TURBOBLAZE
        || (a == ABILITY_MYCELIUM_MIGHT && attacker == gBattlerAttacker && !IsDamaging(gCurrentMove));
}

bool8 MP_Gen59_FlowerVeil(u8 battler)
{
    return HasType(battler, TYPE_GRASS) && SideHas(battler, ABILITY_FLOWER_VEIL);
}

// PROTOSYNTHESIS / QUARK DRIVE: the stat heightened, 0 = none
static u8 Boosted(u8 b)
{
    return gGen59Battlers[b].boosted ? gGen59Battlers[b].boostStat : 0;
}

// ---------------------------------------------------------------------------------------------- battle state

static u8 ArrivalForm(u8 b);
static bool8 SetForm(u8 b, u8 form);
static u8 FormNow(u8 b);
static u8 FormSay(u8 b, u8 msg, bool8 end3);
static const u8 *CheekPouch(u8 b, bool8 end3);
static const u8 *MoveEndMore(u8 atk, u8 def);
static bool8 Unswappable(u16 a);
static const u8 *GimmickMoveEnd(u8 atk, u8 def);
static const u8 *GimmickEndTurn(void);

void MP_Gen59_BattleStart(void)
{
    u32 i;
    memset(gGen59Battlers, 0, sizeof(struct Gen59Battler) * MAX_BATTLERS_COUNT);
    memset(gGen59Sides, 0, sizeof(gGen59Sides));
    memset(&gGen59Field, 0, sizeof(gGen59Field));
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        gGen59Battlers[i].thirdType = TYPE_MYSTERY;
    MP_Gimmick_BattleStart();
}

void MP_Gen59_SwitchIn(u8 battler)
{
    memset(&gGen59Battlers[battler], 0, sizeof(gGen59Battlers[battler]));
    gGen59Battlers[battler].thirdType = TYPE_MYSTERY;
    MP_Gimmick_SwitchIn(battler);   // MEGA and TERA stay with the Pokémon, DYNAMAX ends
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
    if (damage > 0)
        gGen59Battlers[battler].hpBeforeHit = min(gBattleMons[battler].maxHP, gBattleMons[battler].hp + damage);
    if (battler != attacker && damage > 0 && gBattleMons[battler].hp != 0 && gBattleMons[battler].hp <= gBattleMons[battler].maxHP / 2
     && gGen59Battlers[battler].hpBeforeHit > gBattleMons[battler].maxHP / 2
     && (gBattleMons[battler].ability == ABILITY_EMERGENCY_EXIT || gBattleMons[battler].ability == ABILITY_WIMP_OUT))
        gGen59Battlers[battler].emergencyExit = TRUE;
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
    if (GetBattlerSide(gBattlerAttacker) != GetBattlerSide(battler)
     && (gBattleMons[battler].ability == ABILITY_DEFIANT || gBattleMons[battler].ability == ABILITY_COMPETITIVE))
        gGen59Battlers[battler].statDropPending = TRUE;
}

void MP_Gen59_OnFainted(u8 battler)
{
    struct Gen59Side *side = &gGen59Sides[GetBattlerSide(battler)];
    side->faintedThisTurn = TRUE;
    gGen59Field.faintedThisMove = TRUE;
    if (side->fainted < 100)
        side->fainted++;
    {
        u32 i;
        u16 ability = gBattleMons[battler].ability;
        for (i = 0; i < gBattlersCount; i++)
            if (i != battler && Alive(i) && GetBattlerSide(i) == GetBattlerSide(battler)
             && (gBattleMons[i].ability == ABILITY_RECEIVER || gBattleMons[i].ability == ABILITY_POWER_OF_ALCHEMY)
             && !Unswappable(ability) && ability != ABILITY_WONDER_GUARD && ability != ABILITY_TRACE
             && ability != ABILITY_RECEIVER && ability != ABILITY_POWER_OF_ALCHEMY && ability != ABILITY_NONE)
            {
                gBattleMons[i].ability = ability;
                gGen59Battlers[i].received = TRUE;
                break;
            }
    }
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
    bool8 status = moveEffect >= MOVE_EFFECT_SLEEP && moveEffect <= MOVE_EFFECT_TOXIC;
    if (moveEffect == MOVE_EFFECT_FLINCH && MP_Gimmick_Dynamaxed(battler))
        return TRUE;
    switch (MP_Gen4_TargetAbility(gBattlerAttacker, battler))
    {
    case ABILITY_COMATOSE:
    case ABILITY_PURIFYING_SALT:
        if (status)
            return TRUE;
        break;
    case ABILITY_SHIELDS_DOWN:
        if (status && gBattleMons[battler].hp > gBattleMons[battler].maxHP / 2)
            return TRUE;
        break;
    case ABILITY_WATER_BUBBLE:
    case ABILITY_THERMAL_EXCHANGE:
        if (moveEffect == MOVE_EFFECT_BURN)
            return TRUE;
        break;
    }
    if (status && MP_Gen59_FlowerVeil(battler))
        return TRUE;
    if (moveEffect == MOVE_EFFECT_SLEEP && SideHas(battler, ABILITY_SWEET_VEIL))
        return TRUE;
    if ((moveEffect == MOVE_EFFECT_POISON || moveEffect == MOVE_EFFECT_TOXIC) && SideHas(battler, ABILITY_PASTEL_VEIL))
        return TRUE;
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
        case 7:
        {
            const u8 *script = GimmickEndTurn();   // DYNAMAX counts down
            if (script)
            {
                sEndStep = 7;   // (again, for the next battler)
                return script;
            }
            break;
        }
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
    if (move == MOVE_AURA_WHEEL && gGen59Battlers[battler].hangry)
        return TYPE_DARK;
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
    if (gBattleMons[battler].ability == ABILITY_LIQUID_VOICE && (MoveFlags(move) & MP_MOVE_FLAG_SOUND_MOVE))
        type = TYPE_WATER;
    type = MP_Gen59_AteType(move, battler, type);
    type = MP_Gen_MoveType(type);   // FAIRY moves are NORMAL before Gen 6
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

static u16 MovePower(u16 move, u8 attacker, u8 target, u16 power);

u16 MP_Gen59_MovePower(u16 move, u8 attacker, u8 target, u16 power)
{
    return MP_Gimmick_MovePower(move, attacker, MovePower(move, attacker, target, power));   // Z-MOVES, MAX MOVES, TERA
}

static u16 MovePower(u16 move, u8 attacker, u8 target, u16 power)
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
    // Gen 5-9 abilities on the power
    {
        bool8 physical = MP_IsPhysicalMove(move, type);
        u32 i;
        switch (gBattleMons[attacker].ability)
        {
        case ABILITY_TOXIC_BOOST:
            if (physical && (gBattleMons[attacker].status1 & STATUS1_PSN_ANY))
                p = p * 15 / 10;
            break;
        case ABILITY_FLARE_BOOST:
            if (!physical && (gBattleMons[attacker].status1 & STATUS1_BURN))
                p = p * 15 / 10;
            break;
        case ABILITY_WATER_BUBBLE:
            if (type == TYPE_WATER)
                p *= 2;
            break;
        case ABILITY_STAKEOUT:
            if (gDisableStructs[target].isFirstTurn)
                p *= 2;
            break;
        case ABILITY_SUPREME_OVERLORD:
            p = p * (10 + min(5, gGen59Sides[GetBattlerSide(attacker)].fainted)) / 10;
            break;
        }
        if (type == TYPE_STEEL && SideHas(attacker, ABILITY_STEELY_SPIRIT))
            p = p * 15 / 10;
        if (!physical && AllyHas(attacker, ABILITY_BATTERY))
            p = p * 13 / 10;
        if (AllyHas(attacker, ABILITY_POWER_SPOT))
            p = p * 13 / 10;
        for (i = 0; i < 2; i++)   // DARK AURA / FAIRY AURA, turned around by AURA BREAK
        {
            u16 aura = i ? ABILITY_FAIRY_AURA : ABILITY_DARK_AURA;
            if (type == (i ? TYPE_FAIRY : TYPE_DARK) && FieldHas(aura, MAX_BATTLERS_COUNT))
                p = FieldHas(ABILITY_AURA_BREAK, MAX_BATTLERS_COUNT) ? p * 3 / 4 : p * 4 / 3;
        }
    }
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
    // Gen 5-9 abilities on the stats
    {
        u16 tAbility = MP_Gen4_TargetAbility(attacker, target);
        if (gBattleMons[attacker].ability == ABILITY_DEFEATIST && gBattleMons[attacker].hp <= gBattleMons[attacker].maxHP / 2)
            *attack /= 2, *spAttack /= 2;
        if (gBattleMons[attacker].ability == ABILITY_GORILLA_TACTICS)
            *attack = *attack * 15 / 10;
        if (gBattleMons[attacker].ability == ABILITY_ORICHALCUM_PULSE && WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SUN))
            *attack = *attack * 4 / 3;
        if (gBattleMons[attacker].ability == ABILITY_HADRON_ENGINE && gGen59Field.terrain == G59_TERRAIN_ELECTRIC)
            *spAttack = *spAttack * 4 / 3;
        if (Boosted(attacker) == STAT_ATK)
            *attack = *attack * 13 / 10;
        if (Boosted(attacker) == STAT_SPATK)
            *spAttack = *spAttack * 13 / 10;
        if (Boosted(target) == STAT_DEF)
            *defense = *defense * 13 / 10;
        if (Boosted(target) == STAT_SPDEF)
            *spDefense = *spDefense * 13 / 10;
        if (tAbility == ABILITY_FUR_COAT)
            *defense *= 2;
        if (tAbility == ABILITY_GRASS_PELT && gGen59Field.terrain == G59_TERRAIN_GRASSY)
            *defense = *defense * 15 / 10;
        // the RUIN abilities weaken everyone else around
        if (FieldHas(ABILITY_SWORD_OF_RUIN, target) && gBattleMons[target].ability != ABILITY_SWORD_OF_RUIN)
            *defense = *defense * 3 / 4;
        if (FieldHas(ABILITY_BEADS_OF_RUIN, target) && gBattleMons[target].ability != ABILITY_BEADS_OF_RUIN)
            *spDefense = *spDefense * 3 / 4;
        if (FieldHas(ABILITY_TABLETS_OF_RUIN, attacker) && gBattleMons[attacker].ability != ABILITY_TABLETS_OF_RUIN)
            *attack = *attack * 3 / 4;
        if (FieldHas(ABILITY_VESSEL_OF_RUIN, attacker) && gBattleMons[attacker].ability != ABILITY_VESSEL_OF_RUIN)
            *spAttack = *spAttack * 3 / 4;
    }
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
    if (gGen59Battlers[target].glaiveRush)
        damage *= 2;
    if (MP_Gimmick_Dynamaxed(target) && damage > 1)
        damage /= 2;   // (DYNAMAX: twice the HP)
    if (gBattleMons[attacker].ability == ABILITY_PARENTAL_BOND && gBattleMoves[move].effect != EFFECT_MULTI_HIT
     && gBattleMoves[move].effect != EFFECT_DOUBLE_HIT && gBattleMoves[move].effect != EFFECT_TRIPLE_KICK
     && !(gBattleTypeFlags & BATTLE_TYPE_DOUBLE))
        damage = damage * 5 / 4;   // (its second hit at a quarter, as part of the one)
    if (type == TYPE_FIRE && gGen59Battlers[target].tarShot)
        damage *= 2;
    // AURORA VEIL: like REFLECT and LIGHT SCREEN at once (not on top of them; INFILTRATOR goes through)
    if (gGen59Sides[GetBattlerSide(target)].auroraVeil && gCritMultiplier == 1 && gBattleMons[attacker].ability != ABILITY_INFILTRATOR
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
    switch (MP_Gen4_TargetAbility(attacker, target))
    {
    case ABILITY_PURIFYING_SALT:
        if (type == TYPE_GHOST)
            damage /= 2;
        break;
    case ABILITY_WATER_BUBBLE:
        if (type == TYPE_FIRE)
            damage /= 2;
        break;
    case ABILITY_DISGUISE:
    case ABILITY_ICE_FACE:   // (its first hit, ICE FACE's a physical one, is taken by the costume)
        if (!gGen59Battlers[target].disguiseUsed && gBattlerTarget == target
         && (gBattleMons[target].ability == ABILITY_DISGUISE || MP_IsPhysicalMove(move, type)))
        {
            gGen59Battlers[target].disguiseUsed = TRUE;
            gGen59Battlers[target].disguiseJust = TRUE;
            damage = gBattleMons[target].ability == ABILITY_DISGUISE ? max(1, gBattleMons[target].maxHP / 8) : 1;
        }
        break;
    }
    if (AllyHas(target, ABILITY_FRIEND_GUARD))
        damage = damage * 3 / 4;
    if (gBattleMons[attacker].ability == ABILITY_INFILTRATOR && gCritMultiplier == 1)   // through REFLECT / LIGHT SCREEN
    {
        u32 screens = gSideStatuses[GetBattlerSide(target)];
        if ((MP_IsPhysicalMove(move, type) && (screens & SIDE_STATUS_REFLECT))
         || (!MP_IsPhysicalMove(move, type) && (screens & SIDE_STATUS_LIGHTSCREEN)))
            damage = (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? damage * 3 / 2 : damage * 2;
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
    if (gBattleMons[attacker].ability == ABILITY_MERCILESS && gBattlerTarget < gBattlersCount
     && (gBattleMons[gBattlerTarget].status1 & STATUS1_PSN_ANY))
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
    case EFFECT_SAPPY_SEED:
        if (gBattleMons[def].hp == 0 || TargetBehindSubstitute() || (gStatuses3[def] & STATUS3_LEECHSEED) || HasType(def, TYPE_GRASS))
            return 1;
        gStatuses3[def] |= STATUS3_LEECHSEED | STATUS3_LEECHSEED_FROM(atk);
        Say(G59_MSG_SEEDED);
        return 0;
    case EFFECT_EERIE_SPELL:
        if (gBattleMons[def].hp == 0 || gLastMoves[def] == MOVE_NONE || gLastMoves[def] == 0xFFFF)
            return 1;
        for (i = 0; i < MAX_MON_MOVES; i++)
            if (gBattleMons[def].moves[i] == gLastMoves[def] && gBattleMons[def].pp[i] != 0)
            {
                gBattleMons[def].pp[i] = gBattleMons[def].pp[i] > 3 ? gBattleMons[def].pp[i] - 3 : 0;
                gActiveBattler = def;
                BtlController_EmitSetMonData(B_COMM_TO_CONTROLLER, REQUEST_PPMOVE1_BATTLE + i, 0, sizeof(gBattleMons[def].pp[i]), &gBattleMons[def].pp[i]);
                MarkBattlerForControllerExec(def);
                PREPARE_MOVE_BUFFER(gBattleTextBuff1, gLastMoves[def]);
                Say(G59_MSG_PP_REDUCED);
                return 0;
            }
        return 1;
    case EFFECT_CORE_ENFORCER:
        if (gBattleMons[def].hp == 0 || !gGen4Battlers[def].movedThisTurn || Unswappable(gBattleMons[def].ability)
         || gBattleMons[def].ability == ABILITY_NONE)
            return 1;
        gBattleMons[def].ability = ABILITY_NONE;
        Say(G59_MSG_ABILITY_SUPPRESSED);
        return 0;
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
    if (Boosted(battler) == STAT_SPEED)
        speed = speed * 15 / 10;
    return speed;
}

// PROTEAN / LIBERO: the user becomes the type of the move it's about to use
void MP_Gen59_OnMoveStart(u8 battler, u16 move)
{
    u16 ability = gBattleMons[battler].ability;
    gGen59Field.moveEndDone = 0;
    gGen59Field.danced = 0;
    gGen59Battlers[battler].glaiveRush = move == MOVE_GLAIVE_RUSH;   // (until its next move)
    if (ability == ABILITY_STANCE_CHANGE && move != MOVE_NONE)
    {
        if (move == MOVE_KINGS_SHIELD)   // (quietly: a script can't start here)
            SetForm(battler, 0);
        else if (IsDamaging(move))
            SetForm(battler, 1);
    }
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

static u8 SayOnArrival(u8 battler, u8 msg)
{
    if (msg != G59_MSG_STATS_RESET)
        gBattleScripting.battler = battler;
    Say(msg);
    BattleScriptPushCursorAndCallback(BattleScript_G59AbilitySay);
    return 1;
}

// PROTOSYNTHESIS in sunshine / QUARK DRIVE on ELECTRIC TERRAIN (or BOOSTER ENERGY): its highest stat x1.3 (SPEED x1.5)
static u8 TryBoost(u8 battler)
{
    u32 i, best = 0;
    u16 ability = gBattleMons[battler].ability;
    bool8 field = (ability == ABILITY_PROTOSYNTHESIS && WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SUN))
               || (ability == ABILITY_QUARK_DRIVE && gGen59Field.terrain == G59_TERRAIN_ELECTRIC);
    if (gGen59Battlers[battler].boosted)
        return 0;
    if (!field)
    {
        if (!MP_Gen59_HoldsItem(battler, ITEM_BOOSTER_ENERGY))
            return 0;
        gLastUsedItem = ITEM_BOOSTER_ENERGY;
        SetItem(battler, ITEM_NONE);
    }
    gGen59Battlers[battler].boosted = TRUE;
    for (i = STAT_ATK; i <= STAT_SPDEF; i++)
    {
        u16 v = i == STAT_SPEED ? gBattleMons[battler].speed : (&gBattleMons[battler].attack)[i - 1];
        if (v > best)
        {
            best = v;
            gGen59Battlers[battler].boostStat = i;
        }
    }
    gBattleScripting.battler = battler;
    PREPARE_STAT_BUFFER(gBattleTextBuff1, gGen59Battlers[battler].boostStat);
    Say(G59_MSG_HEIGHTENED);
    BattleScriptPushCursorAndCallback(BattleScript_G59AbilitySay);
    return 1;
}

// the Gen 5-9 abilities that act as their Pokémon comes in: 1 if a script was started
u8 MP_Gen59_SwitchInAbility(u8 battler)
{
    if (gBattleMons[battler].hp == 0 || gGen59Battlers[battler].arrived)
        return 0;
    gGen59Battlers[battler].arrived = TRUE;
    gLastUsedAbility = gBattleMons[battler].ability;
    if (ArrivalForm(battler))
        return 1;
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
    case ABILITY_EMBODY_ASPECT_TEAL_MASK:
        return RaiseOnArrival(battler, STAT_SPEED);
    case ABILITY_EMBODY_ASPECT_HEARTHFLAME_MASK:
        return RaiseOnArrival(battler, STAT_ATK);
    case ABILITY_EMBODY_ASPECT_WELLSPRING_MASK:
        return RaiseOnArrival(battler, STAT_SPDEF);
    case ABILITY_EMBODY_ASPECT_CORNERSTONE_MASK:
        return RaiseOnArrival(battler, STAT_DEF);
    case ABILITY_UNNERVE:
    case ABILITY_AS_ONE_ICE_RIDER:
    case ABILITY_AS_ONE_SHADOW_RIDER:
        return SayOnArrival(battler, G59_MSG_NERVOUS);
    case ABILITY_DARK_AURA:
    case ABILITY_FAIRY_AURA:
    case ABILITY_AURA_BREAK:
        return SayOnArrival(battler, G59_MSG_AURA);
    case ABILITY_TERAVOLT:
    case ABILITY_TURBOBLAZE:
        return SayOnArrival(battler, G59_MSG_BREAKS_MOLD);
    case ABILITY_SWORD_OF_RUIN:
    case ABILITY_BEADS_OF_RUIN:
    case ABILITY_TABLETS_OF_RUIN:
    case ABILITY_VESSEL_OF_RUIN:
        return SayOnArrival(battler, G59_MSG_RUIN);
    case ABILITY_SCREEN_CLEANER:
    {
        u32 side;
        bool8 any = FALSE;
        for (side = 0; side < 2; side++)
        {
            any |= (gSideStatuses[side] & (SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN)) || gGen59Sides[side].auroraVeil;
            gSideStatuses[side] &= ~(SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN);
            gSideTimers[side].reflectTimer = gSideTimers[side].lightscreenTimer = 0;
            gGen59Sides[side].auroraVeil = 0;
        }
        return any ? SayOnArrival(battler, G59_MSG_SCREENS_CLEANED) : 0;
    }
    case ABILITY_ORICHALCUM_PULSE:
        if (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SUN))
            return 0;
        gBattleWeather = B_WEATHER_SUN_TEMPORARY;
        gWishFutureKnock.weatherDuration = 5;
        return SayOnArrival(battler, G59_MSG_SUN_TURNED);
    case ABILITY_SUPERSWEET_SYRUP:
    {
        u32 i;
        u8 first = MAX_BATTLERS_COUNT;
        for (i = 0; i < gBattlersCount; i++)
            if (Alive(i) && GetBattlerSide(i) != GetBattlerSide(battler) && gBattleMons[i].statStages[STAT_EVASION] > MIN_STAT_STAGE)
            {
                gBattleMons[i].statStages[STAT_EVASION]--;
                if (first == MAX_BATTLERS_COUNT)
                    first = i;
            }
        if (first == MAX_BATTLERS_COUNT)
            return 0;
        gBattleScripting.battler = first;
        PREPARE_STAT_BUFFER(gBattleTextBuff1, STAT_EVASION);
        gBattleScripting.animArg1 = STAT_ANIM_MINUS1 + STAT_EVASION - 1;
        gBattleScripting.animArg2 = 0;
        Say(G59_MSG_ABILITY_LOWERED);
        BattleScriptPushCursorAndCallback(BattleScript_G59AbilityStat);
        return 1;
    }
    case ABILITY_HOSPITALITY:
    case ABILITY_CURIOUS_MEDICINE:
    {
        u32 i;
        for (i = 0; i < gBattlersCount; i++)
        {
            if (i == battler || !Alive(i) || GetBattlerSide(i) != GetBattlerSide(battler))
                continue;
            if (gBattleMons[battler].ability == ABILITY_CURIOUS_MEDICINE)
            {
                u32 k;
                for (k = 0; k < NUM_BATTLE_STATS; k++)
                    gBattleMons[i].statStages[k] = DEFAULT_STAT_STAGE;
                gBattleScripting.battler = i;
                return SayOnArrival(battler, G59_MSG_STATS_RESET);
            }
            if (gBattleMons[i].hp < gBattleMons[i].maxHP)
            {
                gBattleMoveDamage = Heal(i, 1, 4);
                gBattleScripting.battler = i;
                Say(G59_MSG_SCR_HP_RESTORED);
                BattleScriptPushCursorAndCallback(BattleScript_G59HealEnd3);
                return 1;
            }
        }
        return 0;
    }
    case ABILITY_PROTOSYNTHESIS:
    case ABILITY_QUARK_DRIVE:
        return TryBoost(battler);
    case ABILITY_PRIMORDIAL_SEA:   // (heavy rain and harsh sun as lasting rain and sun)
        gBattleWeather = B_WEATHER_RAIN_PERMANENT;
        return SayOnArrival(battler, G59_MSG_HEAVY_RAIN);
    case ABILITY_DESOLATE_LAND:
        gBattleWeather = B_WEATHER_SUN_PERMANENT;
        return SayOnArrival(battler, G59_MSG_HARSH_SUN);
    case ABILITY_DELTA_STREAM:
        gBattleWeather = 0;
        return SayOnArrival(battler, G59_MSG_STRONG_WINDS);
    }
    return 0;
}

#define G59M_KO      (1 << 0)
#define G59M_ITEM    (1 << 1)
#define G59M_HELMET  (1 << 2)
#define G59M_DEFAB   (1 << 3)   // the target's ability on being hit
#define G59M_ATKAB   (1 << 4)   // the attacker's on hitting
#define G59M_DECOY   (1 << 5)
#define G59M_INNARDS (1 << 6)
#define G59M_DEFIANT (1 << 7)
#define G59M_SOUL    (1 << 8)
#define G59M_MORE    (1 << 9)
#define G59M_GIMMICK (1 << 10)  // a MAX MOVE's effect   // MoveEndMore: CHEEK POUCH, DANCER, OPPORTUNIST, EMERGENCY EXIT...

// a stat change by an ability at a move's end (BY < 0 lowers): its message on gBattleScripting.battler
static const u8 *AbilityStat(u8 battler, u8 stat, s8 by, u8 ability)
{
    s8 *stage = &gBattleMons[battler].statStages[stat];
    if ((by > 0 && *stage >= MAX_STAT_STAGE) || (by < 0 && *stage <= MIN_STAT_STAGE))
        return NULL;
    *stage = by > 0 ? min(MAX_STAT_STAGE, *stage + by) : max(MIN_STAT_STAGE, *stage + by);
    gLastUsedAbility = ability;
    gBattleScripting.battler = battler;
    PREPARE_STAT_BUFFER(gBattleTextBuff1, stat);
    gBattleScripting.animArg1 = (by > 0 ? (by >= 2 ? STAT_ANIM_PLUS2 : STAT_ANIM_PLUS1) : (by <= -2 ? STAT_ANIM_MINUS2 : STAT_ANIM_MINUS1)) + stat - 1;
    gBattleScripting.animArg2 = 0;
    Say(by > 0 ? G59_MSG_ABILITY_RAISED : G59_MSG_ABILITY_LOWERED);
    return BattleScript_G59AbilityStatMoveEnd;
}

static bool8 CanTakeStatus(u8 b, u8 type1, u8 type2)
{
    return Alive(b) && !(gBattleMons[b].status1 & STATUS1_ANY) && !(gBattleMons[b].status2 & STATUS2_SUBSTITUTE)
        && !HasType(b, type1) && !HasType(b, type2) && !MP_Gen59_StatusBlocked(b, MOVE_EFFECT_POISON)
        && gBattleMons[b].ability != ABILITY_IMMUNITY;
}

static const u8 *GiveStatus(u8 b, u32 status, u8 msg)
{
    SetStatus(b, status);
    gBattleScripting.battler = b;
    Say(msg);
    return BattleScript_G59StatusMoveEnd;
}

// abilities nobody can lose or copy (their POKéMON need them)
static bool8 Unswappable(u16 a)
{
    return a == ABILITY_MULTITYPE || a == ABILITY_STANCE_CHANGE || a == ABILITY_SCHOOLING || a == ABILITY_COMATOSE
        || a == ABILITY_SHIELDS_DOWN || a == ABILITY_DISGUISE || a == ABILITY_RKS_SYSTEM || a == ABILITY_BATTLE_BOND
        || a == ABILITY_POWER_CONSTRUCT || a == ABILITY_ICE_FACE || a == ABILITY_GULP_MISSILE || a == ABILITY_ZERO_TO_HERO
        || a == ABILITY_COMMANDER || a == ABILITY_TERA_SHIFT || a == ABILITY_WONDER_GUARD;
}

// the target's ability as a move hits it
static const u8 *TargetAbilityOnHit(u8 atk, u8 def, u8 type)
{
    u16 ability = gBattleMons[def].ability;
    bool8 physical = MP_IsPhysicalMove(gCurrentMove, type), contact = MakesContact(gCurrentMove, atk);
    u16 half = gBattleMons[def].maxHP / 2;
    bool8 crossedHalf = gGen59Battlers[def].hpBeforeHit > half && gBattleMons[def].hp <= half && gBattleMons[def].hp != 0;
    const u8 *script = NULL;

    // GULP MISSILE: what it caught hits the attacker (a quarter of its HP, and DEFENSE down or paralysis)
    if (ability == ABILITY_GULP_MISSILE && gGen59Battlers[def].gulp && Alive(atk) && gBattleMons[atk].ability != ABILITY_MAGIC_GUARD)
    {
        if (gGen59Battlers[def].gulp == 1 && gBattleMons[atk].statStages[STAT_DEF] > MIN_STAT_STAGE)
            gBattleMons[atk].statStages[STAT_DEF]--;
        else if (gGen59Battlers[def].gulp == 2 && CanTakeStatus(atk, TYPE_ELECTRIC, TYPE_ELECTRIC) && gBattleMons[atk].ability != ABILITY_LIMBER)
            SetStatus(atk, STATUS1_PARALYSIS);
        gGen59Battlers[def].gulp = 0;
        gBattleMoveDamage = max(1, gBattleMons[atk].maxHP / 4);
        Say(G59_MSG_GULP_SPIT);
        return BattleScript_G59RockyHelmet;
    }
    if (gBattleMons[def].hp != 0 && !(gBattleMons[def].status2 & STATUS2_SUBSTITUTE))
    {
        switch (ability)
        {
        case ABILITY_STAMINA:
            return AbilityStat(def, STAT_DEF, 1, ability);
        case ABILITY_JUSTIFIED:
            return type == TYPE_DARK ? AbilityStat(def, STAT_ATK, 1, ability) : NULL;
        case ABILITY_RATTLED:
            return (type == TYPE_BUG || type == TYPE_GHOST || type == TYPE_DARK) ? AbilityStat(def, STAT_SPEED, 1, ability) : NULL;
        case ABILITY_WEAK_ARMOR:
            if (!physical)
                return NULL;
            if (gBattleMons[def].statStages[STAT_DEF] > MIN_STAT_STAGE)
                gBattleMons[def].statStages[STAT_DEF]--;
            return AbilityStat(def, STAT_SPEED, 2, ability);
        case ABILITY_WATER_COMPACTION:
            return type == TYPE_WATER ? AbilityStat(def, STAT_DEF, 2, ability) : NULL;
        case ABILITY_STEAM_ENGINE:
            return (type == TYPE_FIRE || type == TYPE_WATER) ? AbilityStat(def, STAT_SPEED, 6, ability) : NULL;
        case ABILITY_THERMAL_EXCHANGE:
            return type == TYPE_FIRE ? AbilityStat(def, STAT_ATK, 1, ability) : NULL;
        case ABILITY_BERSERK:
            return crossedHalf ? AbilityStat(def, STAT_SPATK, 1, ability) : NULL;
        case ABILITY_ANGER_SHELL:
            if (!crossedHalf)
                return NULL;
            if (gBattleMons[def].statStages[STAT_DEF] > MIN_STAT_STAGE)
                gBattleMons[def].statStages[STAT_DEF]--;
            if (gBattleMons[def].statStages[STAT_SPDEF] > MIN_STAT_STAGE)
                gBattleMons[def].statStages[STAT_SPDEF]--;
            if (gBattleMons[def].statStages[STAT_SPATK] < MAX_STAT_STAGE)
                gBattleMons[def].statStages[STAT_SPATK]++;
            if (gBattleMons[def].statStages[STAT_SPEED] < MAX_STAT_STAGE)
                gBattleMons[def].statStages[STAT_SPEED]++;
            return AbilityStat(def, STAT_ATK, 1, ability);
        case ABILITY_ELECTROMORPHOSIS:
        case ABILITY_WIND_POWER:
            if (ability == ABILITY_WIND_POWER && !IsWind(gCurrentMove))
                return NULL;
            gStatuses3[def] |= STATUS3_CHARGED_UP;
            gBattleScripting.battler = def;
            Say(G59_MSG_CHARGING);
            return BattleScript_G59AbilitySayMoveEnd;
        case ABILITY_COTTON_DOWN:
        {
            u32 i;
            for (i = 0; i < gBattlersCount; i++)
                if (i != def && i != atk && Alive(i) && gBattleMons[i].statStages[STAT_SPEED] > MIN_STAT_STAGE)
                    gBattleMons[i].statStages[STAT_SPEED]--;
            return Alive(atk) ? AbilityStat(atk, STAT_SPEED, -1, ability) : NULL;
        }
        case ABILITY_SAND_SPIT:
            if (WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SANDSTORM))
                return NULL;
            gBattleWeather = B_WEATHER_SANDSTORM_TEMPORARY;
            gWishFutureKnock.weatherDuration = 5;
            Say(G59_MSG_SANDSTORM_UP);
            return BattleScript_G59WeatherMoveEnd;
        case ABILITY_SEED_SOWER:
            if (gGen59Field.terrain == G59_TERRAIN_GRASSY)
                return NULL;
            gGen59Field.terrain = G59_TERRAIN_GRASSY;
            gGen59Field.terrainTurns = 5;
            gBattleScripting.battler = def;
            Say(G59_MSG_TERRAIN_GRASSY);
            return BattleScript_G59AbilitySayMoveEnd;
        case ABILITY_TOXIC_DEBRIS:
        {
            struct Gen4Side *side = &gGen4Sides[GetBattlerSide(atk)];
            if (!physical || side->toxicSpikes >= 2)
                return NULL;
            side->toxicSpikes++;
            Say(G59_MSG_TOXIC_SPIKES);
            return BattleScript_G59AbilitySayMoveEnd;
        }
        case ABILITY_PICKPOCKET:
            if (contact && Alive(atk) && gBattleMons[def].item == ITEM_NONE && gBattleMons[atk].item != ITEM_NONE
             && gBattleMons[atk].ability != ABILITY_STICKY_HOLD && !ItemIsMail(gBattleMons[atk].item))
            {
                gLastUsedItem = gBattleMons[atk].item;
                SetItem(atk, ITEM_NONE);
                SetItem(def, gLastUsedItem);
                gBattleScripting.battler = def;
                Say(G59_MSG_STOLE_ITEM);
                return BattleScript_G59AbilitySayMoveEnd;
            }
            return NULL;
        }
    }
    // the ones that act on the attacker (the target may have fainted)
    if (!Alive(atk))
        return NULL;
    switch (ability)
    {
    case ABILITY_GOOEY:
    case ABILITY_TANGLING_HAIR:
        return contact ? AbilityStat(atk, STAT_SPEED, -1, ability) : NULL;
    case ABILITY_CURSED_BODY:
        if (gDisableStructs[atk].disabledMove == MOVE_NONE && Random() % 10 < 3 && gCurrentMove != MOVE_STRUGGLE)
        {
            u32 i;
            for (i = 0; i < MAX_MON_MOVES && gBattleMons[atk].moves[i] != gCurrentMove; i++)
                ;
            if (i == MAX_MON_MOVES)
                return NULL;
            gDisableStructs[atk].disabledMove = gCurrentMove;
            gDisableStructs[atk].disableTimer = gDisableStructs[atk].disableTimerStartValue = 4;
            PREPARE_MOVE_BUFFER(gBattleTextBuff1, gCurrentMove);
            Say(G59_MSG_MOVE_DISABLED);
            return BattleScript_G59AbilitySayMoveEnd;
        }
        return NULL;
    case ABILITY_MUMMY:
    case ABILITY_LINGERING_AROMA:
        if (contact && gBattleMons[atk].ability != ability && !Unswappable(gBattleMons[atk].ability))
        {
            gBattleMons[atk].ability = ability;
            PREPARE_ABILITY_BUFFER(gBattleTextBuff1, ability);
            Say(G59_MSG_ABILITY_BECAME);
            return BattleScript_G59AbilitySayMoveEnd;
        }
        return NULL;
    case ABILITY_WANDERING_SPIRIT:
        if (contact && gBattleMons[def].hp != 0 && !Unswappable(gBattleMons[atk].ability))
        {
            gBattleMons[def].ability = gBattleMons[atk].ability;
            gBattleMons[atk].ability = ABILITY_WANDERING_SPIRIT;
            Say(G59_MSG_ABILITIES_SWAPPED);
            return BattleScript_G59AbilitySayMoveEnd;
        }
        return NULL;
    }
    return script;
}

// the attacker's ability as its move hits
static const u8 *AttackerAbilityOnHit(u8 atk, u8 def)
{
    switch (gBattleMons[atk].ability)
    {
    case ABILITY_POISON_TOUCH:
        if (MakesContact(gCurrentMove, atk) && Random() % 10 < 3 && CanTakeStatus(def, TYPE_POISON, TYPE_STEEL))
            return GiveStatus(def, STATUS1_POISON, G59_MSG_POISONED);
        break;
    case ABILITY_TOXIC_CHAIN:
        if (Random() % 10 < 3 && CanTakeStatus(def, TYPE_POISON, TYPE_STEEL))
            return GiveStatus(def, STATUS1_TOXIC_POISON, G59_MSG_BADLY_POISONED);
        break;
    case ABILITY_POISON_PUPPETEER:
        if ((gBattleMons[def].status1 & STATUS1_PSN_ANY) && gBattleMons[def].hp != 0
         && !(gBattleMons[def].status2 & STATUS2_CONFUSION) && gBattleMons[def].ability != ABILITY_OWN_TEMPO)
        {
            gBattleMons[def].status2 |= STATUS2_CONFUSION_TURN(((Random()) % 4) + 2);
            gBattleScripting.battler = def;
            Say(G59_MSG_CONFUSED);
            return BattleScript_G59AbilitySayMoveEnd;
        }
        break;
    case ABILITY_MAGICIAN:
        if (gBattleMons[atk].item == ITEM_NONE && gBattleMons[def].item != ITEM_NONE
         && gBattleMons[def].ability != ABILITY_STICKY_HOLD && !ItemIsMail(gBattleMons[def].item))
        {
            gLastUsedItem = gBattleMons[def].item;
            SetItem(def, ITEM_NONE);
            SetItem(atk, gLastUsedItem);
            gBattleScripting.battler = atk;
            Say(G59_MSG_STOLE_ITEM);
            return BattleScript_G59AbilitySayMoveEnd;
        }
        break;
    }
    return NULL;
}

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
    if (!(gGen59Field.moveEndDone & G59M_DECOY))
    {
        gGen59Field.moveEndDone |= G59M_DECOY;
        if (def < gBattlersCount && gGen59Battlers[def].disguiseJust)
        {
            gGen59Battlers[def].disguiseJust = FALSE;
            Say(G59_MSG_DECOY);
            return BattleScript_G59AbilitySayMoveEnd;
        }
    }
    if (!(gGen59Field.moveEndDone & G59M_DEFAB))
    {
        gGen59Field.moveEndDone |= G59M_DEFAB;
        if (hit && !MP_Gen59_IgnoresAbilities(atk) && gBattleMons[atk].ability != ABILITY_MOLD_BREAKER)
        {
            const u8 *script = TargetAbilityOnHit(atk, def, type);
            if (script)
                return script;
        }
    }
    if (!(gGen59Field.moveEndDone & G59M_ATKAB))
    {
        gGen59Field.moveEndDone |= G59M_ATKAB;
        if (hit && Alive(atk))
        {
            const u8 *script = AttackerAbilityOnHit(atk, def);
            if (script)
                return script;
        }
    }
    if (!(gGen59Field.moveEndDone & G59M_INNARDS))
    {
        gGen59Field.moveEndDone |= G59M_INNARDS;
        if (hit && gBattleMons[def].hp == 0 && gBattleMons[def].ability == ABILITY_INNARDS_OUT && Alive(atk)
         && gBattleMons[atk].ability != ABILITY_MAGIC_GUARD)
        {
            gBattleMoveDamage = gGen59Battlers[def].hpBeforeHit;
            Say(G59_MSG_INNARDS_OUT);
            return BattleScript_G59RockyHelmet;
        }
    }
    if (!(gGen59Field.moveEndDone & G59M_DEFIANT))
    {
        for (i = 0; i < gBattlersCount; i++)
        {
            if (!gGen59Battlers[i].statDropPending)
                continue;
            gGen59Battlers[i].statDropPending = FALSE;
            if (Alive(i))
            {
                const u8 *script = AbilityStat(i, gBattleMons[i].ability == ABILITY_DEFIANT ? STAT_ATK : STAT_SPATK, 2, gBattleMons[i].ability);
                if (script)
                    return script;
            }
        }
        gGen59Field.moveEndDone |= G59M_DEFIANT;
    }
    if (!(gGen59Field.moveEndDone & G59M_SOUL))
    {
        gGen59Field.moveEndDone |= G59M_SOUL;
        if (gGen59Field.faintedThisMove)
        {
            gGen59Field.faintedThisMove = FALSE;
            for (i = 0; i < gBattlersCount; i++)
                if (Alive(i) && gBattleMons[i].ability == ABILITY_SOUL_HEART)
                {
                    const u8 *script = AbilityStat(i, STAT_SPATK, 1, ABILITY_SOUL_HEART);
                    if (script)
                        return script;
                }
        }
    }
    if (!(gGen59Field.moveEndDone & G59M_GIMMICK))
    {
        const u8 *script = GimmickMoveEnd(atk, def);
        gGen59Field.moveEndDone |= G59M_GIMMICK;
        if (script)
            return script;
    }
    if (!(gGen59Field.moveEndDone & G59M_MORE))
    {
        const u8 *script = MoveEndMore(atk, def);
        if (script)
            return script;   // (each part clears what it did, so this comes back until nothing is left)
        gGen59Field.moveEndDone |= G59M_MORE;
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
    case ABILITY_CHILLING_NEIGH:
    case ABILITY_AS_ONE_ICE_RIDER:
        stat = STAT_ATK;
        break;
    case ABILITY_GRIM_NEIGH:
    case ABILITY_AS_ONE_SHADOW_RIDER:
        stat = STAT_SPATK;
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

// ---------------------------------------------------------------------------------------------- abilities: the turn's end

// MOODY, HEALER, HARVEST: 1 if a script was started (ABILITYEFFECT_ENDTURN)
u8 MP_Gen59_EndTurnAbility(u8 battler)
{
    u32 i;
    const u8 *script;
    if ((script = CheekPouch(battler, TRUE)) != NULL)
    {
        BattleScriptPushCursorAndCallback(script);
        return 1;
    }
    switch (gBattleMons[battler].ability)
    {
    case ABILITY_SCHOOLING:
    case ABILITY_SHIELDS_DOWN:
    {
        u8 form = FormNow(battler);
        if (!SetForm(battler, form))
            return 0;
        if (gBattleMons[battler].ability == ABILITY_SCHOOLING)
            return FormSay(battler, form ? G59_MSG_SCHOOL : G59_MSG_SCHOOL_END, TRUE);
        return FormSay(battler, form ? G59_MSG_SHIELDS_DOWN : G59_MSG_SHIELDS_UP, TRUE);
    }
    case ABILITY_HUNGER_SWITCH:
        gGen59Battlers[battler].hangry ^= 1;
        if (gGen59Battlers[battler].hangry)
            return FormSay(battler, G59_MSG_HANGRY, TRUE);
        break;
    case ABILITY_CUD_CHEW:
        if (gGen59Battlers[battler].cudChewTurns && --gGen59Battlers[battler].cudChewTurns == 0
         && gGen59Battlers[battler].eatenBerry && gBattleMons[battler].item == ITEM_NONE)
        {
            gLastUsedItem = FIRST_BERRY_INDEX + gGen59Battlers[battler].eatenBerry - 1;
            gGen59Battlers[battler].eatenBerry = 0;
            gBattleStruct->usedHeldItems[battler] = ITEM_NONE;
            SetItem(battler, gLastUsedItem);   // eaten again as soon as it can be
            return FormSay(battler, G59_MSG_CUD_CHEW, TRUE);
        }
        break;
    case ABILITY_MOODY:
    {
        u8 up[NUM_BATTLE_STATS], down[NUM_BATTLE_STATS], nUp = 0, nDown = 0, raise, lower;
        for (i = STAT_ATK; i <= STAT_SPDEF; i++)
        {
            if (gBattleMons[battler].statStages[i] < MAX_STAT_STAGE)
                up[nUp++] = i;
            if (gBattleMons[battler].statStages[i] > MIN_STAT_STAGE)
                down[nDown++] = i;
        }
        if (nUp == 0)
            return 0;
        raise = up[Random() % nUp];
        gBattleMons[battler].statStages[raise] = min(MAX_STAT_STAGE, gBattleMons[battler].statStages[raise] + 2);
        if (nDown != 0)
        {
            lower = down[Random() % nDown];
            if (lower != raise)
                gBattleMons[battler].statStages[lower]--;
        }
        gBattleScripting.battler = battler;
        PREPARE_STAT_BUFFER(gBattleTextBuff1, raise);
        gBattleScripting.animArg1 = STAT_ANIM_PLUS2 + raise - 1;
        gBattleScripting.animArg2 = 0;
        Say(G59_MSG_ABILITY_RAISED);
        BattleScriptPushCursorAndCallback(BattleScript_G59AbilityStat);
        return 1;
    }
    case ABILITY_HEALER:
        for (i = 0; i < gBattlersCount; i++)
            if (i != battler && Alive(i) && GetBattlerSide(i) == GetBattlerSide(battler)
             && (gBattleMons[i].status1 & STATUS1_ANY) && Random() % 10 < 3)
            {
                SetStatus(i, 0);
                gBattleScripting.battler = i;
                Say(G59_MSG_SCR_CURED);
                BattleScriptPushCursorAndCallback(BattleScript_G59StatusEnd3);
                return 1;
            }
        break;
    case ABILITY_HARVEST:
    {
        u16 used = gBattleStruct->usedHeldItems[battler];
        if (gBattleMons[battler].item == ITEM_NONE && used >= FIRST_BERRY_INDEX && used <= LAST_BERRY_INDEX
         && ((WEATHER_HAS_EFFECT2 && (gBattleWeather & B_WEATHER_SUN)) || Random() % 2 == 0))
        {
            gBattleStruct->usedHeldItems[battler] = ITEM_NONE;
            gLastUsedItem = used;
            SetItem(battler, used);
            gBattleScripting.battler = battler;
            Say(G59_MSG_HARVESTED);
            BattleScriptPushCursorAndCallback(BattleScript_G59AbilitySay);
            return 1;
        }
        break;
    }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------- abilities: moves taken in

// SAP SIPPER, WELL-BAKED BODY, WIND RIDER (a stat up), EARTH EATER (HP like VOLT ABSORB): 0, or 1 = heal by the
// caller's VOLT ABSORB code, 2 = a stat script set (ABILITYEFFECT_ABSORBING)
u8 MP_Gen59_AbsorbAbility(u8 battler, u16 move, u8 type)
{
    u8 stat = 0, by = 1;
    switch (gBattleMons[battler].ability)
    {
    case ABILITY_SAP_SIPPER:
        if (type == TYPE_GRASS)
            stat = STAT_ATK;
        break;
    case ABILITY_WELL_BAKED_BODY:
        if (type == TYPE_FIRE)
            stat = STAT_DEF, by = 2;
        break;
    case ABILITY_WIND_RIDER:
        if (IsWind(move))
            stat = STAT_ATK;
        break;
    case ABILITY_EARTH_EATER:
        if (type == TYPE_GROUND && gBattleMoves[move].power != 0)
        {
            gBattlescriptCurrInstr = gProtectStructs[gBattlerAttacker].notFirstStrike ? BattleScript_MoveHPDrain : BattleScript_MoveHPDrain_PPLoss;
            return 1;
        }
        break;
    }
    if (stat == 0)
        return 0;
    gBattleScripting.battler = battler;
    gBattleScripting.statChanger = SET_STATCHANGER(stat, by, FALSE);
    gBattlescriptCurrInstr = gProtectStructs[gBattlerAttacker].notFirstStrike ? BattleScript_G59AbsorbStat : BattleScript_G59AbsorbStat_PPLoss;
    return 2;
}

// BULLETPROOF, OVERCOAT, GOOD AS GOLD, QUEENLY MAJESTY / DAZZLING / ARMOR TAIL, TELEPATHY keep a move off BATTLER:
// TRUE with the script set (ABILITYEFFECT_MOVES_BLOCK)
bool8 MP_Gen59_BlockMove(u8 battler, u16 move)
{
    u8 atk = gBattlerAttacker;
    bool8 block = FALSE;
    if (atk == battler || MP_Gen59_IgnoresAbilities(atk) || gBattleMons[atk].ability == ABILITY_MOLD_BREAKER)
        return FALSE;
    switch (gBattleMons[battler].ability)
    {
    case ABILITY_BULLETPROOF:
        block = IsBallistic(move);
        break;
    case ABILITY_OVERCOAT:
        block = IsPowder(move);
        break;
    case ABILITY_GOOD_AS_GOLD:
        block = gBattleMoves[move].power == 0 && gBattleMoves[move].target != MOVE_TARGET_USER;
        break;
    case ABILITY_TELEPATHY:
        block = GetBattlerSide(atk) == GetBattlerSide(battler) && gBattleMoves[move].power != 0;
        break;
    case ABILITY_MAGIC_BOUNCE:   // (the move is kept off, not sent back)
        block = GetBattlerSide(atk) != GetBattlerSide(battler) && gBattleMoves[move].power == 0
             && gBattleMoves[move].target != MOVE_TARGET_USER;
        break;
    }
    if (!block && GetBattlerSide(atk) != GetBattlerSide(battler) && MP_Gen59_Priority(atk, move) > 0
     && (SideHas(battler, ABILITY_QUEENLY_MAJESTY) || SideHas(battler, ABILITY_DAZZLING) || SideHas(battler, ABILITY_ARMOR_TAIL)))
        block = TRUE;
    if (!block)
        return FALSE;
    gLastUsedAbility = gBattleMons[battler].ability;
    if (gBattleMons[atk].status2 & STATUS2_MULTIPLETURNS)
        gHitMarker |= HITMARKER_NO_PPDEDUCT;
    gBattlescriptCurrInstr = BattleScript_SoundproofProtected;
    return TRUE;
}

// VICTORY STAR (the user's side x1.1), WONDER SKIN (a status move at most 50%)
u32 MP_Gen59_Accuracy(u8 attacker, u8 target, u16 move, u32 acc)
{
    if (gGen59Battlers[target].glaiveRush)
        return 100 * 2;   // (GLAIVE RUSH: can't miss it)
    if (SideHas(attacker, ABILITY_VICTORY_STAR))
        acc = acc * 11 / 10;
    if (gBattleMoves[move].power == 0 && MP_Gen4_TargetAbility(attacker, target) == ABILITY_WONDER_SKIN && acc > 50)
        acc = 50;
    return acc;
}

bool8 MP_Gen59_IntimidateRaises(u8 battler)
{
    return gBattleMons[battler].ability == ABILITY_GUARD_DOG;
}

// ---------------------------------------------------------------------------------------------- forms in battle

// a form's ATTACK, DEFENSE, SPEED, SP. ATK, SP. DEF (the HP stays): the other form's stats, worked out from the
// Pokémon's own IVs, EVs, level and nature; the party keeps the base form (a switch brings it back)
struct FormStats { u16 species, ability; u8 base[5], alt[5]; };
static const struct FormStats sForms[] =
{
    {SPECIES_AEGISLASH,  ABILITY_STANCE_CHANGE, { 50, 140,  60,  50, 140}, {140,  50,  60, 140,  50}},
    {SPECIES_WISHIWASHI, ABILITY_SCHOOLING,     { 20,  20,  40,  25,  25}, {140, 130,  30, 140, 135}},
    {SPECIES_PALAFIN,    ABILITY_ZERO_TO_HERO,  { 70,  72, 100,  53,  62}, {160,  97, 100, 106,  87}},
    {SPECIES_MINIOR,     ABILITY_SHIELDS_DOWN,  { 60, 100,  60,  60, 100}, {100,  60, 120, 100,  60}},
    {SPECIES_TERAPAGOS,  ABILITY_TERA_SHIFT,    { 65,  85,  60,  65,  85}, { 95, 110,  85, 105, 110}},
};

static struct Pokemon *BattlerMon(u8 b)
{
    return &(GetBattlerSide(b) == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty)[gBattlerPartyIndexes[b]];
}

// FORM 0 / 1: TRUE if it changed (the battler's form ability on its species)
static bool8 SetForm(u8 b, u8 form)
{
    u32 i, k;
    struct Pokemon *mon = BattlerMon(b);
    static const u8 sStatOf[5] = {STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF};
    if (gGen59Battlers[b].form == form || gBattleMons[b].status2 & STATUS2_TRANSFORMED)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sForms); i++)
        if (sForms[i].species == gBattleMons[b].species && sForms[i].ability == gBattleMons[b].ability)
            break;
    if (i == ARRAY_COUNT(sForms))
        return FALSE;
    gGen59Battlers[b].form = form;
    for (k = 0; k < 5; k++)
    {
        u8 st = sStatOf[k];
        u32 base = form ? sForms[i].alt[k] : sForms[i].base[k];
        u32 iv = GetMonData(mon, MON_DATA_HP_IV + st, NULL), ev = GetMonData(mon, MON_DATA_HP_EV + st, NULL);
        u16 v = ModifyStatByNature(GetNature(mon), ((2 * base + iv + ev / 4) * gBattleMons[b].level) / 100 + 5, st);
        switch (st)
        {
        case STAT_ATK:   gBattleMons[b].attack = v;    break;
        case STAT_DEF:   gBattleMons[b].defense = v;   break;
        case STAT_SPEED: gBattleMons[b].speed = v;     break;
        case STAT_SPATK: gBattleMons[b].spAttack = v;  break;
        case STAT_SPDEF: gBattleMons[b].spDefense = v; break;
        }
    }
    return TRUE;
}

static u8 FormSay(u8 b, u8 msg, bool8 end3)
{
    gBattleScripting.battler = b;
    gLastUsedAbility = gBattleMons[b].ability;
    Say(msg);
    if (end3)
        BattleScriptPushCursorAndCallback(BattleScript_G59AbilitySay);
    return 1;
}

// SCHOOLING (Lv. 20+, over a quarter of its HP) / SHIELDS DOWN (half or less: its core): the form it should be in now
static u8 FormNow(u8 b)
{
    switch (gBattleMons[b].ability)
    {
    case ABILITY_SCHOOLING:
        return gBattleMons[b].level >= 20 && gBattleMons[b].hp > gBattleMons[b].maxHP / 4;
    case ABILITY_SHIELDS_DOWN:
        return gBattleMons[b].hp <= gBattleMons[b].maxHP / 2;
    }
    return gGen59Battlers[b].form;
}

// the arrival form changes (from MP_Gen59_SwitchInAbility): 1 if a script was started
static u8 ArrivalForm(u8 b)
{
    u8 side = GetBattlerSide(b);
    switch (gBattleMons[b].ability)
    {
    case ABILITY_SCHOOLING:
        if (FormNow(b) && SetForm(b, 1))
            return FormSay(b, G59_MSG_SCHOOL, TRUE);
        break;
    case ABILITY_SHIELDS_DOWN:
        if (FormNow(b) && SetForm(b, 1))
            return FormSay(b, G59_MSG_SHIELDS_DOWN, TRUE);
        break;
    case ABILITY_TERA_SHIFT:
        if (SetForm(b, 1))
            return FormSay(b, G59_MSG_TERA_SHIFT, TRUE);
        break;
    case ABILITY_ZERO_TO_HERO:
        if ((gGen59Field.heroSlots[side] & (1 << gBattlerPartyIndexes[b])) && SetForm(b, 1))
            return FormSay(b, G59_MSG_HERO, TRUE);
        break;
    case ABILITY_COMMANDER:
    {
        u32 i, k;
        for (i = 0; i < gBattlersCount; i++)
            if (i != b && Alive(i) && GetBattlerSide(i) == side && gBattleMons[i].species == SPECIES_DONDOZO)
            {
                for (k = STAT_ATK; k < NUM_BATTLE_STATS; k++)
                    gBattleMons[i].statStages[k] = min(MAX_STAT_STAGE, gBattleMons[i].statStages[k] + 2);
                return FormSay(b, G59_MSG_COMMANDER, TRUE);
            }
        break;
    }
    }
    return 0;
}

void MP_Gen59_SwitchedOut(u8 battler)
{
    if (battler < gBattlersCount && gBattleMons[battler].ability == ABILITY_ZERO_TO_HERO)
        gGen59Field.heroSlots[GetBattlerSide(battler)] |= 1 << gBattlerPartyIndexes[battler];
}

// ---------------------------------------------------------------------------------------------- BERRIES

bool8 MP_Gen59_BerriesBlocked(u8 battler)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
        if (Alive(i) && GetBattlerSide(i) != GetBattlerSide(battler)
         && (gBattleMons[i].ability == ABILITY_UNNERVE || gBattleMons[i].ability == ABILITY_AS_ONE_ICE_RIDER
          || gBattleMons[i].ability == ABILITY_AS_ONE_SHADOW_RIDER))
            return TRUE;
    return FALSE;
}

u8 MP_Gen59_Ripen(u8 battler)
{
    return gBattleMons[battler].ability == ABILITY_RIPEN ? 2 : 1;
}

void MP_Gen59_AteBerry(u8 battler, u16 berry)
{
    if (battler >= gBattlersCount || berry < FIRST_BERRY_INDEX || berry > LAST_BERRY_INDEX)
        return;
    if (gBattleMons[battler].ability == ABILITY_CHEEK_POUCH)
        gGen59Battlers[battler].cheekPouch = TRUE;
    if (gBattleMons[battler].ability == ABILITY_CUD_CHEW && gGen59Battlers[battler].cudChewTurns == 0)
    {
        gGen59Battlers[battler].eatenBerry = berry - FIRST_BERRY_INDEX + 1;
        gGen59Battlers[battler].cudChewTurns = 2;   // the end of the next turn
    }
}

// CHEEK POUCH: a third of its HP after a BERRY (NULL if nothing to do)
static const u8 *CheekPouch(u8 b, bool8 end3)
{
    if (!gGen59Battlers[b].cheekPouch)
        return NULL;
    gGen59Battlers[b].cheekPouch = FALSE;
    if (!Alive(b) || gBattleMons[b].hp >= gBattleMons[b].maxHP || gGen4Battlers[b].healBlock)
        return NULL;
    gBattleMoveDamage = Heal(b, 1, 3);
    gBattleScripting.battler = b;
    Say(G59_MSG_SCR_HP_RESTORED);
    return end3 ? BattleScript_G59HealEnd3 : BattleScript_G59HealMoveEnd;
}

// ---------------------------------------------------------------------------------------------- OPPORTUNIST, DANCER

void MP_Gen59_OnStatRaised(u8 battler, u8 stat, s8 by)
{
    u32 i;
    if (by <= 0 || stat >= NUM_BATTLE_STATS)
        return;
    for (i = 0; i < gBattlersCount; i++)
        if (Alive(i) && GetBattlerSide(i) != GetBattlerSide(battler) && gBattleMons[i].ability == ABILITY_OPPORTUNIST)
        {
            gGen59Battlers[i].copyPending = TRUE;
            gGen59Battlers[i].copyStages[stat] += by;
        }
}

static const u8 *Opportunist(void)
{
    u32 i, k;
    for (i = 0; i < gBattlersCount; i++)
    {
        bool8 any = FALSE;
        if (!gGen59Battlers[i].copyPending)
            continue;
        gGen59Battlers[i].copyPending = FALSE;
        for (k = 0; k < NUM_BATTLE_STATS; k++)
        {
            s8 by = gGen59Battlers[i].copyStages[k];
            gGen59Battlers[i].copyStages[k] = 0;
            if (by > 0 && Alive(i) && gBattleMons[i].statStages[k] < MAX_STAT_STAGE)
            {
                gBattleMons[i].statStages[k] = min(MAX_STAT_STAGE, gBattleMons[i].statStages[k] + by);
                any = TRUE;
            }
        }
        if (any)
        {
            gBattleScripting.battler = i;
            gLastUsedAbility = ABILITY_OPPORTUNIST;
            Say(G59_MSG_OPPORTUNIST);
            return BattleScript_G59AbilitySayMoveEnd;
        }
    }
    return NULL;
}

// the dances a DANCER copies: what they raise on the user (ATK DEF SPEED SPATK SPDEF)
struct Dance { u16 move; s8 stages[5]; };
static const struct Dance sDances[] =
{
    {MOVE_SWORDS_DANCE,  {2, 0, 0, 0, 0}},
    {MOVE_DRAGON_DANCE,  {1, 0, 1, 0, 0}},
    {MOVE_QUIVER_DANCE,  {0, 0, 1, 1, 1}},
    {MOVE_VICTORY_DANCE, {1, 1, 1, 0, 0}},
    {MOVE_SHELL_SMASH,   {0, 0, 0, 0, 0}},   // (not a dance)
};

static const u8 *Dancer(u8 atk)
{
    u32 i, k, d;
    for (d = 0; d < ARRAY_COUNT(sDances) - 1; d++)
        if (sDances[d].move == gCurrentMove)
            break;
    if (d == ARRAY_COUNT(sDances) - 1 || (gMoveResultFlags & MOVE_RESULT_NO_EFFECT) || gGen59Battlers[atk].lastMoveFailed)
        return NULL;
    for (i = 0; i < gBattlersCount; i++)
    {
        static const u8 sStatOf[5] = {STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF};
        bool8 any = FALSE;
        if (i == atk || !Alive(i) || gBattleMons[i].ability != ABILITY_DANCER || (gGen59Field.danced & (1 << i)))
            continue;
        gGen59Field.danced |= 1 << i;
        for (k = 0; k < 5; k++)
        {
            u8 st = sStatOf[k];
            if (sDances[d].stages[k] > 0 && gBattleMons[i].statStages[st] < MAX_STAT_STAGE)
            {
                gBattleMons[i].statStages[st] = min(MAX_STAT_STAGE, gBattleMons[i].statStages[st] + sDances[d].stages[k]);
                any = TRUE;
            }
        }
        if (any)
        {
            gBattleScripting.battler = i;
            gLastUsedAbility = ABILITY_DANCER;
            Say(G59_MSG_DANCER);
            return BattleScript_G59AbilitySayMoveEnd;
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------------------------- the rest

bool8 MP_Gen59_MovesLast(u8 battler, u16 move)
{
    return move != MOVE_NONE && gBattleMons[battler].ability == ABILITY_MYCELIUM_MIGHT && !IsDamaging(move);
}

void MP_Gen59_BallFailed(u8 battler, u16 ball)
{
    u32 i;
    if (gGen59Field.ballFetched || (gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return;
    for (i = 0; i < gBattlersCount; i++)
        if (GetBattlerSide(i) == B_SIDE_PLAYER && Alive(i) && gBattleMons[i].ability == ABILITY_BALL_FETCH
         && gBattleMons[i].item == ITEM_NONE)
        {
            gGen59Field.ballFetched = TRUE;
            SetItem(i, ball);
            return;
        }
}

// the move-end steps of this part: CHEEK POUCH, GULP MISSILE's catch, DANCER, OPPORTUNIST, EMERGENCY EXIT
static const u8 *MoveEndMore(u8 atk, u8 def)
{
    u32 i;
    const u8 *script;
    for (i = 0; i < gBattlersCount; i++)
        if ((script = CheekPouch(i, FALSE)) != NULL)
            return script;
    for (i = 0; i < gBattlersCount; i++)
        if (gGen59Battlers[i].received)
        {
            gGen59Battlers[i].received = FALSE;
            if (!Alive(i))
                continue;
            gBattleScripting.battler = i;
            PREPARE_ABILITY_BUFFER(gBattleTextBuff1, gBattleMons[i].ability);
            Say(G59_MSG_SCR_ABILITY_BECAME);
            return BattleScript_G59AbilitySayMoveEnd;
        }
    if (gBattleMons[atk].ability == ABILITY_GULP_MISSILE && Alive(atk) && gGen59Battlers[atk].gulp == 0
     && (gCurrentMove == MOVE_SURF || gCurrentMove == MOVE_DIVE) && !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT)
     && !(gBattleMons[atk].status2 & STATUS2_MULTIPLETURNS) && gBattleMons[atk].species == SPECIES_CRAMORANT)
    {
        gGen59Battlers[atk].gulp = gBattleMons[atk].hp > gBattleMons[atk].maxHP / 2 ? 1 : 2;
        gBattleScripting.battler = atk;
        Say(G59_MSG_GULP);
        return BattleScript_G59AbilitySayMoveEnd;
    }
    if ((script = Dancer(atk)) != NULL)
        return script;
    if ((script = Opportunist()) != NULL)
        return script;
    if (def < gBattlersCount && gGen59Battlers[def].emergencyExit)
    {
        gGen59Battlers[def].emergencyExit = FALSE;
        if (!Alive(def) || def == atk)
            return NULL;
        gBattleScripting.battler = def;
        gLastUsedAbility = gBattleMons[def].ability;
        if (!(gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE)) && GetBattlerSide(def) == B_SIDE_OPPONENT)
        {
            Say(G59_MSG_FLED);
            return BattleScript_G59Flee;   // (it sets the outcome and ends the action, as ROAR does)
        }
        if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) && GetBattlerSide(def) == B_SIDE_OPPONENT)
            return NULL;
        Say(G59_MSG_EMERGENCY_EXIT);
        return BattleScript_G59EmergencyExit;
    }
    return NULL;
}

// ---------------------------------------------------------------------------------------------- GIMMICKS (the battle)
// MEGA EVOLUTION, Z-MOVES, DYNAMAX, TERASTALLIZATION: each trainer one of them once a battle (include/battle_gimmick.h).
// A MEGA form isn't a species: the battler takes its types, ability and stats (and every screen its picture, through
// the animation); MEGA and TERA stay with the Pokémon (by party slot) when it switches, DYNAMAX ends.

struct GimmickState
{
    u8 wanted[MAX_BATTLERS_COUNT];   // the gimmick chosen with this turn's move
    u8 dynamax[MAX_BATTLERS_COUNT];  // turns left
    u8 zNow[MAX_BATTLERS_COUNT];     // Z-POWER on this turn's move (the crystal's type + 1)
    u8 used;                         // trainers (Owner bits) that used theirs
    u8 next;                         // MP_Gimmick_TurnStart's place in the turn order
    u8 abilityNext;                  // a new MEGA ability to start (battler + 1)
    u8 dynaDone;                     // the end of turn's DYNAMAX count (battler bits)
    u8 megaOf[2][PARTY_SIZE];        // MEGA form + 1, by side and party slot
    u8 teraOf[2][PARTY_SIZE];        // TERA type + 1
};
static EWRAM_DATA struct GimmickState sG = {0};

static u8 Owner(u8 b)
{
    if (GetBattlerSide(b) == B_SIDE_PLAYER)
        return (gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER)) || gMpCoopBattle || IS_TRIPLE_BATTLE() ? b : 0;
    return (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) ? b : 1;
}

static bool8 GimmickBattle(void)
{
    return !(gBattleTypeFlags & (BATTLE_TYPE_SAFARI | BATTLE_TYPE_WALLY_TUTORIAL | BATTLE_TYPE_FIRST_BATTLE
                                 | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_RECORDED));
}

static u8 FindMega(u8 b)
{
    u32 i, k;
    for (i = 0; i < MP_Mega_Count(); i++)
    {
        const struct MegaForm *f = MP_Mega_Get(i);
        if (f->species != gBattleMons[b].species)
            continue;
        if (f->stone != ITEM_NONE && f->stone == gBattleMons[b].item)
            return i;
        if ((f->stone == ITEM_RED_ORB || f->stone == ITEM_BLUE_ORB) && GetBattlerSide(b) == B_SIDE_PLAYER
         && CheckBagHasItem(f->stone, 1))
            return i;   // (Emerald's orbs are KEY ITEMS: in the bag is enough)
        if (f->stone == ITEM_NONE)
            for (k = 0; k < MAX_MON_MOVES; k++)
                if (gBattleMons[b].moves[k] == MOVE_DRAGON_ASCENT)
                    return i;
    }
    return 0xFF;
}

static u8 *MegaSlot(u8 b) { return &sG.megaOf[GetBattlerSide(b)][gBattlerPartyIndexes[b] % PARTY_SIZE]; }
static u8 *TeraSlot(u8 b) { return &sG.teraOf[GetBattlerSide(b)][gBattlerPartyIndexes[b] % PARTY_SIZE]; }

static u8 ZType(u16 item)
{
    u8 i;
    if (item < ITEM_NORMALIUM_Z || item > ITEM_FAIRIUM_Z)
        return TYPE_MYSTERY;
    i = item - ITEM_NORMALIUM_Z;
    return i < TYPE_MYSTERY ? i : i + 1;   // (the crystals skip MYSTERY)
}

u8 MP_Gimmick_TeraType(u8 b)
{
    u8 t = b < gBattlersCount ? *TeraSlot(b) : 0;
    return t ? t - 1 : TYPE_MYSTERY;
}

bool8 MP_Gimmick_Dynamaxed(u8 b)
{
    return b < gBattlersCount && sG.dynamax[b] != 0;
}

u8 MP_Gimmick_Available(u8 b)
{
    u8 avail = 0, z;
    if (!GimmickBattle() || (sG.used & (1 << Owner(b))) || (gBattleMons[b].status2 & STATUS2_TRANSFORMED))
        return 0;
    if (MP_Gimmick_On(GIMMICK_MEGA) && !*MegaSlot(b) && FindMega(b) != 0xFF)
        avail |= GIMMICK_AVAIL_MEGA;
    if (MP_Gimmick_On(GIMMICK_DYNAMAX) && !*MegaSlot(b) && !sG.dynamax[b])
        avail |= GIMMICK_AVAIL_DYNAMAX;
    if (MP_Gimmick_On(GIMMICK_TERA) && !*TeraSlot(b))
        avail |= GIMMICK_AVAIL_TERA;
    z = ZType(gBattleMons[b].item);
    if (MP_Gimmick_On(GIMMICK_Z) && z != TYPE_MYSTERY && !gGen4Battlers[b].embargo)
        avail |= (z + 1) << 3;
    return avail;
}

void MP_Gimmick_Choose(u8 b, u8 gimmick)
{
    if (b < MAX_BATTLERS_COUNT)
        sG.wanted[b] = gimmick < GIMMICK_COUNT ? gimmick : GIMMICK_NONE;
}

void MP_Gimmick_BattleStart(void)
{
    memset(&sG, 0, sizeof(sG));
}

void MP_Gimmick_TurnStartReset(void)
{
    sG.next = 0;
}

// what MEGA and TERA leave on the Pokémon: again each time it comes in
static void ApplyMega(u8 b, u8 form)
{
    const struct MegaForm *f = MP_Mega_Get(form);
    struct Pokemon *mon = BattlerMon(b);
    static const u8 sStatOf[5] = {STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF};
    u32 k;
    gBattleMons[b].types[0] = f->types[0];
    gBattleMons[b].types[1] = f->types[1];
    if (f->ability != ABILITY_NONE)
        gBattleMons[b].ability = f->ability;
    for (k = 0; k < 5; k++)
    {
        u8 st = sStatOf[k];
        u32 iv = GetMonData(mon, MON_DATA_HP_IV + st, NULL), ev = GetMonData(mon, MON_DATA_HP_EV + st, NULL);
        u16 v = ModifyStatByNature(GetNature(mon), ((2 * f->stats[k] + iv + ev / 4) * gBattleMons[b].level) / 100 + 5, st);
        switch (st)
        {
        case STAT_ATK:   gBattleMons[b].attack = v;    break;
        case STAT_DEF:   gBattleMons[b].defense = v;   break;
        case STAT_SPEED: gBattleMons[b].speed = v;     break;
        case STAT_SPATK: gBattleMons[b].spAttack = v;  break;
        case STAT_SPDEF: gBattleMons[b].spDefense = v; break;
        }
    }
}

void MP_Gimmick_SwitchIn(u8 b)
{
    u8 m, t;
    if (b >= MAX_BATTLERS_COUNT)
        return;
    sG.dynamax[b] = 0;
    sG.zNow[b] = 0;
    if (b >= gBattlersCount || gBattleMons[b].species == SPECIES_NONE)
        return;
    m = *MegaSlot(b);
    t = *TeraSlot(b);
    if (m && m - 1 < MP_Mega_Count() && MP_Mega_Get(m - 1)->species == gBattleMons[b].species)
        ApplyMega(b, m - 1);
    if (t)
        SetTypes(b, t - 1, t - 1);
}

static u8 GimmickSay(u8 b, u8 msg, u8 animKind, u8 animValue)
{
    gBattleScripting.battler = gBattlerAttacker = b;
    gBattleScripting.animArg1 = animValue;
    gBattleScripting.animArg2 = animKind;
    Say(msg);
    BattleScriptExecute(animKind ? BattleScript_G59Gimmick : BattleScript_G59GimmickSay);
    return TRUE;
}

// at the turn's start, in the turn's order (after switches and items, before any move)
bool8 MP_Gimmick_TurnStart(void)
{
    if (sG.abilityNext)
    {
        u8 b = sG.abilityNext - 1;
        sG.abilityNext = 0;
        gGen59Battlers[b].arrived = FALSE;
        if (AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, b, 0, 0, 0))
            return TRUE;
    }
    while (sG.next < gBattlersCount)
    {
        u8 b = gBattlerByTurnOrder[sG.next++], want = sG.wanted[b], avail;
        sG.wanted[b] = GIMMICK_NONE;
        if (want == GIMMICK_NONE || gBattleMons[b].hp == 0 || gChosenActionByBattler[b] != B_ACTION_USE_MOVE)
            continue;
        avail = MP_Gimmick_Available(b);
        switch (want)
        {
        case GIMMICK_MEGA:
        {
            u8 form = FindMega(b);
            if (!(avail & GIMMICK_AVAIL_MEGA) || form == 0xFF)
                break;
            sG.used |= 1 << Owner(b);
            *MegaSlot(b) = form + 1;
            ApplyMega(b, form);
            sG.abilityNext = b + 1;
            gLastUsedItem = gBattleMons[b].item;
            return GimmickSay(b, MP_Mega_Get(form)->stone == ITEM_RED_ORB || MP_Mega_Get(form)->stone == ITEM_BLUE_ORB
                                 ? G59_MSG_PRIMAL : G59_MSG_MEGA, GIMMICK_ANIM_MEGA, form);
        }
        case GIMMICK_TERA:
        {
            u8 type = gSpeciesInfo[gBattleMons[b].species].types[0];
            if (!(avail & GIMMICK_AVAIL_TERA))
                break;
            if (type == TYPE_MYSTERY)
                type = TYPE_NORMAL;
            sG.used |= 1 << Owner(b);
            *TeraSlot(b) = type + 1;
            SetTypes(b, type, type);
            gGen59Battlers[b].thirdType = TYPE_MYSTERY;
            PREPARE_TYPE_BUFFER(gBattleTextBuff1, type);
            return GimmickSay(b, G59_MSG_TERA, GIMMICK_ANIM_TERA, type);
        }
        case GIMMICK_DYNAMAX:
            if (!(avail & GIMMICK_AVAIL_DYNAMAX))
                break;
            sG.used |= 1 << Owner(b);
            sG.dynamax[b] = 3;
            return GimmickSay(b, G59_MSG_DYNAMAX, GIMMICK_ANIM_DYNAMAX, 0);
        case GIMMICK_Z:
        {
            u16 move = gBattleMons[b].moves[gBattleStruct->chosenMovePositions[b] % MAX_MON_MOVES];
            if (!(avail >> 3) || !IsDamaging(move) || MP_Gen4_MoveType(move, b) != GIMMICK_AVAIL_Z_TYPE(avail))
                break;
            sG.used |= 1 << Owner(b);
            sG.zNow[b] = GIMMICK_AVAIL_Z_TYPE(avail) + 1;
            return GimmickSay(b, G59_MSG_Z_POWER, 0, 0);
        }
        }
    }
    return FALSE;
}

s32 MP_Gimmick_Stab(s32 damage, u8 atk, u8 moveType)
{
    u8 tera = MP_Gimmick_TeraType(atk);
    if (tera != TYPE_MYSTERY)
    {
        const u8 *own = gSpeciesInfo[gBattleMons[atk].species].types;
        bool8 orig = own[0] == moveType || own[1] == moveType;
        if (moveType == tera)
            return orig ? damage * 2 : damage * 15 / 10;
        return orig ? damage * 15 / 10 : damage;
    }
    if (IS_BATTLER_OF_TYPE(atk, moveType))
        return MP_Gen4_Stab(damage, atk);
    return damage;
}

// Z-MOVES' and MAX MOVES' power from the move's own
static u16 ZPower(u16 p)
{
    static const u8 sUpTo[] = {55, 65, 75, 85, 95, 100, 110, 125, 130};
    static const u8 sZ[] = {100, 120, 140, 160, 175, 180, 185, 190, 195};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUpTo); i++)
        if (p <= sUpTo[i])
            return sZ[i];
    return 200;
}

static u16 MaxPower(u16 p, u8 type)
{
    static const u8 sUpTo[] = {40, 50, 60, 70, 100, 140};
    static const u8 sMax[] = {90, 100, 110, 120, 130, 140};
    static const u8 sWeak[] = {70, 75, 80, 85, 90, 95};   // FIGHTING and POISON
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUpTo); i++)
        if (p <= sUpTo[i])
            return (type == TYPE_FIGHTING || type == TYPE_POISON) ? sWeak[i] : sMax[i];
    return (type == TYPE_FIGHTING || type == TYPE_POISON) ? 100 : 150;
}

u16 MP_Gimmick_MovePower(u16 move, u8 atk, u16 power)
{
    u8 type;
    if (power == 0 || atk >= gBattlersCount)
        return power;
    type = MP_Gen4_MoveType(move, atk);
    if (sG.dynamax[atk])
        return MaxPower(gBattleMoves[move].power > 1 ? gBattleMoves[move].power : 100, type);
    if (sG.zNow[atk] && sG.zNow[atk] - 1 == type)
        return ZPower(gBattleMoves[move].power > 1 ? gBattleMoves[move].power : 100);
    if (MP_Gimmick_TeraType(atk) == type && power < 60 && MP_Gen59_Priority(atk, move) <= 0
     && gBattleMoves[move].effect != EFFECT_MULTI_HIT && gBattleMoves[move].effect != EFFECT_DOUBLE_HIT
     && gBattleMoves[move].effect != EFFECT_TRIPLE_KICK)
        return 60;
    return power;
}

u16 MP_Gimmick_MoveUsed(u8 b, u16 move)
{
    if (b < gBattlersCount && sG.dynamax[b] && move != MOVE_NONE && move != MOVE_STRUGGLE && !IsDamaging(move))
        return MOVE_PROTECT;   // MAX GUARD
    return move;
}

// a MAX MOVE's own effect after it hit (its type's): NULL when none
static const u8 *MaxMoveEffect(u8 atk)
{
    static const s8 sStat[NUMBER_OF_MON_TYPES][2] = {   // {stat, +1 the user's side / -1 the foes'}
        [TYPE_NORMAL] = {STAT_SPEED, -1}, [TYPE_FIGHTING] = {STAT_ATK, 1}, [TYPE_FLYING] = {STAT_SPEED, 1},
        [TYPE_POISON] = {STAT_SPATK, 1}, [TYPE_GROUND] = {STAT_SPDEF, 1}, [TYPE_BUG] = {STAT_SPATK, -1},
        [TYPE_GHOST] = {STAT_DEF, -1}, [TYPE_STEEL] = {STAT_DEF, 1}, [TYPE_DRAGON] = {STAT_ATK, -1},
        [TYPE_DARK] = {STAT_SPDEF, -1},
    };
    u8 type = MP_Gen4_MoveType(gCurrentMove, atk), side = GetBattlerSide(atk);
    u32 i;
    bool8 weather = WEATHER_HAS_EFFECT2;
    switch (type)
    {
    case TYPE_FIRE:
    case TYPE_WATER:
    case TYPE_ROCK:
    case TYPE_ICE:
    {
        u16 w = type == TYPE_FIRE ? B_WEATHER_SUN_TEMPORARY : type == TYPE_WATER ? B_WEATHER_RAIN_TEMPORARY
              : type == TYPE_ROCK ? B_WEATHER_SANDSTORM_TEMPORARY : B_WEATHER_HAIL_TEMPORARY;
        if (!weather || (gBattleWeather & w))
            return NULL;
        gBattleWeather = w;
        gWishFutureKnock.weatherDuration = 5;
        Say(type == TYPE_FIRE ? G59_MSG_SUN_UP : type == TYPE_WATER ? G59_MSG_RAIN_UP
            : type == TYPE_ROCK ? G59_MSG_SANDSTORM_UP : G59_MSG_HAIL_UP);
        return BattleScript_G59AbilitySayMoveEnd;
    }
    case TYPE_GRASS:
    case TYPE_ELECTRIC:
    case TYPE_PSYCHIC:
    case TYPE_FAIRY:
    {
        u8 t = type == TYPE_GRASS ? G59_TERRAIN_GRASSY : type == TYPE_ELECTRIC ? G59_TERRAIN_ELECTRIC
             : type == TYPE_PSYCHIC ? G59_TERRAIN_PSYCHIC : G59_TERRAIN_MISTY;
        if (gGen59Field.terrain == t)
            return NULL;
        gGen59Field.terrain = t;
        gGen59Field.terrainTurns = 5;
        Say(sTerrainMsg[t]);
        return BattleScript_G59AbilitySayMoveEnd;
    }
    }
    if (type >= NUMBER_OF_MON_TYPES || sStat[type][1] == 0)
        return NULL;
    {
        u8 stat = sStat[type][0];
        bool8 any = FALSE, up = sStat[type][1] > 0;
        for (i = 0; i < gBattlersCount; i++)
        {
            if (!Alive(i) || (GetBattlerSide(i) == side) != up)
                continue;
            if (up && gBattleMons[i].statStages[stat] < MAX_STAT_STAGE)
                gBattleMons[i].statStages[stat]++, any = TRUE;
            else if (!up && gBattleMons[i].statStages[stat] > MIN_STAT_STAGE && !(gBattleMons[i].status2 & STATUS2_SUBSTITUTE))
                gBattleMons[i].statStages[stat]--, any = TRUE;
        }
        if (!any)
            return NULL;
        PREPARE_STAT_BUFFER(gBattleTextBuff1, stat);
        Say(up ? G59_MSG_TEAM_STAT_UP : G59_MSG_FOES_STAT_DOWN);
        return BattleScript_G59AbilitySayMoveEnd;
    }
}

// the move's end: a MAX MOVE's effect, Z-POWER spent
static const u8 *GimmickMoveEnd(u8 atk, u8 def)
{
    bool8 hit = def < gBattlersCount && atk != def && IsDamaging(gCurrentMove) && !(gMoveResultFlags & MOVE_RESULT_NO_EFFECT);
    sG.zNow[atk] = 0;
    if (sG.dynamax[atk] && hit && Alive(atk))
        return MaxMoveEffect(atk);
    return NULL;
}

// the end of the turn: DYNAMAX counts down (NULL when nothing is left to say)
static const u8 *GimmickEndTurn(void)
{
    u32 i;
    for (i = 0; i < gBattlersCount; i++)
    {
        if (!sG.dynamax[i] || (sG.dynaDone & (1 << i)))
            continue;
        sG.dynaDone |= 1 << i;
        if (--sG.dynamax[i] == 0 && gBattleMons[i].hp != 0)
        {
            gBattlerAttacker = gBattleScripting.battler = i;
            gBattleScripting.animArg1 = 0;
            gBattleScripting.animArg2 = GIMMICK_ANIM_DYNAMAX_END;
            Say(G59_MSG_DYNAMAX_END);
            return BattleScript_G59DynamaxEnd;
        }
    }
    sG.dynaDone = 0;
    return NULL;
}
