// Moemon Co-op: achievements. Personal accomplishments (each save earns its own),
// shown in CO-OP menu > DAILY > ACHIEVEMENTS. Most are read straight from the
// game (badges, game stats like eggs hatched or contests won, the POKéDEX); the
// rest are counted here by hooks around the co-op code (MP_Achieve_Count) or
// unlocked by a single feat (MP_Achieve_Unlock: a flawless gym battle, a shiny...).
// Every one gives MOEGATCHA pulls: 3 for short goals, 10 for medium, 20 for long ones.
// They're sorted into categories (the menu's L/R tabs). Whatever the save already
// shows (story flags, game stats, the POKéDEX, the party and PC, money...) counts, so
// a new achievement an old save has already earned is granted at once: the first time
// a version with new ones runs, all of them together with one message.
//
// Unlocked achievements are kept next to the save ("av:<id>", mp_savedata.c),
// counters as "acnt:<name>".
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_stadium_cup.h"
#include "event_data.h"
#include "money.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "data.h"
#include "pokemon_storage_system.h"
#include "constants/trainers.h"
#include "constants/species.h"
#include "constants/flags.h"
#include "constants/game_stat.h"
#include "mp_qzphone.h"
#include "s3d_contest.h"
#include "s3d_underground.h"

s32 MP_Gatcha_Pulls(void);
void MP_Gatcha_AddPulls(s32 n);
void MP_Events_QueueToast(const char *ascii);
s32 MP_Events_StreakDays(void);
s32 MP_DayNumber(s32 yyyymmdd);
s32 MP_Today(void);

enum {
    AK_FLAG, AK_STAT, AK_COUNT, AK_CAUGHT, AK_SEEN, AK_LEVEL, AK_EVENT, AK_STREAK, AK_BADGES,
    AK_FLAGS,    // how many flags of a list are set (arg: FL_*)
    AK_DEXLIST,  // how many species of a list are caught (arg: DL_*)
    AK_HOENN,    // HOENN POKéDEX caught
    AK_TYPES,    // types among the caught species
    AK_TEAM,     // party members at Lv arg or higher
    AK_OWNED,    // POKéMON in the party and the PC
    AK_FRIEND,   // a party POKéMON at full friendship
    AK_MONEY,    // money held right now
    AK_CLEARED,  // regions whose League is beaten (arg: one region + 1, or 0 for how many)
    AK_STARTED,  // regions the player has been to
    AK_STADIUM,  // POKéMON STADIUM (arg: STADIUM_ACH_*, mp_stadium_rewards.c)
    AK_NATRANGE, // caught POKéMON numbered 1 to arg in the national POKéDEX (KANTO's 151, JOHTO's 251)
    AK_VARGE,    // a var at a value or past it (arg: VC_*)
    AK_FLAGRANGE,// how many flags of a run are set (arg: FR_*)
    AK_FUNC,     // counted by a region's own code (arg: FN_*)
};

// The menu's tabs. BATTLE, CATCHING, TOGETHER and EXTRAS are each region's own (the user, 2026-11-08: what HOENN
// earned shouldn't leave nothing to earn in the others): the table's rows are HOENN's, and JOHTO / KANTO / SINNOH
// each get a copy counted only there (RegionCopy below); the region tabs (HOENN, JOHTO...) hold its own feats.
enum { C_JOURNEY, C_BATTLE, C_CATCH, C_HOENN, C_TOGETHER, C_EXTRAS, C_CARDS, C_YUGIOH, C_MAGIC, C_TRIAD, C_DANCE, C_JOHTO, C_KANTO, C_SINNOH, C_STADIUM,
       C_BATTLE_J, C_CATCH_J, C_TOGETHER_J, C_EXTRAS_J, C_BATTLE_K, C_CATCH_K, C_TOGETHER_K, C_EXTRAS_K,
       C_BATTLE_S, C_CATCH_S, C_TOGETHER_S, C_EXTRAS_S, NUM_ACH_CATS };
#define C_GONE 0xFF   // a row kept only so later rows keep their number (TRAINER CARD titles are numbers): never listed
static const char *const sCatNames[NUM_ACH_CATS] = {"JOURNEY", "HOENN BATTLE", "HOENN CATCHING", "HOENN", "HOENN TOGETHER", "HOENN EXTRAS", "CARDS", "YU-GI-OH!", "MAGIC", "TRIPLE TRIAD", "DANCE", "JOHTO", "KANTO", "SINNOH", "STADIUM",
                                                    "JOHTO BATTLE", "JOHTO CATCHING", "JOHTO TOGETHER", "JOHTO EXTRAS", "KANTO BATTLE", "KANTO CATCHING", "KANTO TOGETHER", "KANTO EXTRAS",
                                                    "SINNOH BATTLE", "SINNOH CATCHING", "SINNOH TOGETHER", "SINNOH EXTRAS"};
// The tabs a region has: its own, then its BATTLE, CATCHING, TOGETHER and EXTRAS (G_*), by region
enum { G_REGION, G_BATTLE, G_CATCH, G_TOGETHER, G_EXTRAS, NUM_ACH_GROUPS };
static const u8 sGroupCats[NUM_ACH_GROUPS][REGION_COUNT] = {
    [G_REGION]   = {C_HOENN, C_JOHTO, C_KANTO, C_SINNOH},
    [G_BATTLE]   = {C_BATTLE, C_BATTLE_J, C_BATTLE_K, C_BATTLE_S},
    [G_CATCH]    = {C_CATCH, C_CATCH_J, C_CATCH_K, C_CATCH_S},
    [G_TOGETHER] = {C_TOGETHER, C_TOGETHER_J, C_TOGETHER_K, C_TOGETHER_S},
    [G_EXTRAS]   = {C_EXTRAS, C_EXTRAS_J, C_EXTRAS_K, C_EXTRAS_S},
};

enum { FL_HMS, FL_RODS, FL_LANDMARKS, FL_SILVER, FL_GOLD, FL_JOHTO_BEASTS, FL_KANTO_BIRDS, FL_JOHTO_BADGES, FL_KANTO_BADGES,
       FL_ALPH, FL_ARCEUS_TRIALS, FL_SINNOH_BADGES, FL_SINNOH_LAKES, FL_SINNOH_CREATION,
       FL_KANTO_SNORLAX, FL_KANTO_DRINKS, FL_KANTO_RODS, FL_KANTO_SEAFOAM, FL_KANTO_GEMS, FL_JOHTO_TOGEPI, FL_SINNOH_BARRY };
// Vars at a value (JOHTO's and SINNOH's own scripts set them: tools/quetzal_port, tools/sinnoh/ids.py)
enum { VC_JOHTO_RADIO, VC_JOHTO_SHUCKIE, VC_JOHTO_MOOMOO, VC_JOHTO_RIVAL, VC_SINNOH_SILVER, VC_SINNOH_GOLD };
static const struct { u16 var, value; } sVarChecks[] = {
    [VC_JOHTO_RADIO]   = {0x500C, 0x11},   // the RADIO TOWER freed (the CLEAR BELL given: Johto_RadioTower5F)
    [VC_JOHTO_SHUCKIE] = {0x501D, 3},      // SHUCKIE lent by the MANIA (Johto_ManiasHouse)
    [VC_JOHTO_MOOMOO]  = {0x501B, 5},      // MOOMOO's seventh ORAN BERRY (Johto_Route39Barn)
    [VC_JOHTO_RIVAL]   = {0x5029, 3},      // the rival beaten in VICTORY ROAD (Johto_VictoryRoad3F)
    [VC_SINNOH_SILVER] = {0x51A3, 2},      // Platinum's BATTLE TOWER print: silver
    [VC_SINNOH_GOLD]   = {0x51A3, 4},      // ... gold
};
// Runs of flags: JOHTO's 57 hidden items (constants/kai_regions.h), the VILLA's 20 pieces of furniture
enum { FR_JOHTO_HIDDEN, FR_SINNOH_VILLA };
static const struct { u16 first, count; } sFlagRanges[] = {
    [FR_JOHTO_HIDDEN] = {0x1601, 57},
    [FR_SINNOH_VILLA] = {0x3748, 20},
};
// Counted by a region's own code
enum { FN_JOHTO_PHONE, FN_SINNOH_PLATES, FN_SINNOH_CONTESTS, FN_SINNOH_STUDIO };
enum { DL_STARTERS, DL_FOSSILS, DL_LEGENDS, DL_STARTERS_J, DL_STARTERS_K, DL_STARTERS_S, DL_FOSSILS_K, DL_FOSSILS_S, DL_JOHTO_BEASTS };

struct Achievement
{
    const char *id;      // also the counter name for AK_COUNT
    const char *name;
    const char *desc;
    u8 kind;
    u16 arg;             // flag / game stat / list
    u32 target;
    u8 tier;             // reward tier: 0 = 3 pulls, 1 = 10 pulls, 2 = 20 pulls (AchPulls)
    u8 cat;              // C_*
};

static const u16 sFlagsHMs[] = {
    FLAG_RECEIVED_HM_CUT, FLAG_RECEIVED_HM_FLY, FLAG_RECEIVED_HM_SURF, FLAG_RECEIVED_HM_STRENGTH, FLAG_RECEIVED_HM_FLASH,
    FLAG_RECEIVED_HM_ROCK_SMASH, FLAG_RECEIVED_HM_WATERFALL, FLAG_RECEIVED_HM_DIVE,
};
static const u16 sFlagsRods[] = {FLAG_RECEIVED_OLD_ROD, FLAG_RECEIVED_GOOD_ROD, FLAG_RECEIVED_SUPER_ROD};
static const u16 sFlagsLandmarks[] = {
    FLAG_LANDMARK_MIRAGE_TOWER, FLAG_LANDMARK_FLOWER_SHOP, FLAG_LANDMARK_MR_BRINEY_HOUSE, FLAG_LANDMARK_ABANDONED_SHIP,
    FLAG_LANDMARK_SEASHORE_HOUSE, FLAG_LANDMARK_NEW_MAUVILLE, FLAG_LANDMARK_OLD_LADY_REST_SHOP, FLAG_LANDMARK_TRICK_HOUSE,
    FLAG_LANDMARK_WINSTRATE_FAMILY, FLAG_LANDMARK_GLASS_WORKSHOP, FLAG_LANDMARK_LANETTES_HOUSE, FLAG_LANDMARK_POKEMON_DAYCARE,
    FLAG_LANDMARK_SEAFLOOR_CAVERN, FLAG_LANDMARK_BATTLE_FRONTIER, FLAG_LANDMARK_SOUTHERN_ISLAND, FLAG_LANDMARK_FIERY_PATH,
    FLAG_LANDMARK_POKEMON_LEAGUE, FLAG_LANDMARK_ISLAND_CAVE, FLAG_LANDMARK_DESERT_RUINS, FLAG_LANDMARK_FOSSIL_MANIACS_HOUSE,
    FLAG_LANDMARK_SCORCHED_SLAB, FLAG_LANDMARK_ANCIENT_TOMB, FLAG_LANDMARK_TUNNELERS_REST_HOUSE, FLAG_LANDMARK_HUNTERS_HOUSE,
    FLAG_LANDMARK_SEALED_CHAMBER, FLAG_LANDMARK_SKY_PILLAR, FLAG_LANDMARK_BERRY_MASTERS_HOUSE, FLAG_LANDMARK_ALTERING_CAVE,
    FLAG_LANDMARK_DESERT_UNDERPASS, FLAG_LANDMARK_ARTISAN_CAVE, FLAG_LANDMARK_TRAINER_HILL,
};
static const u16 sFlagsSilver[] = {
    FLAG_SYS_TOWER_SILVER, FLAG_SYS_DOME_SILVER, FLAG_SYS_PALACE_SILVER, FLAG_SYS_ARENA_SILVER, FLAG_SYS_FACTORY_SILVER,
    FLAG_SYS_PIKE_SILVER, FLAG_SYS_PYRAMID_SILVER,
};
static const u16 sFlagsGold[] = {
    FLAG_SYS_TOWER_GOLD, FLAG_SYS_DOME_GOLD, FLAG_SYS_PALACE_GOLD, FLAG_SYS_ARENA_GOLD, FLAG_SYS_FACTORY_GOLD,
    FLAG_SYS_PIKE_GOLD, FLAG_SYS_PYRAMID_GOLD,
};
// JOHTO / KANTO (Quetzal's flags, kept in the region side file): badges, and the static encounters' "done" flags
// (caught or beaten: src/data/qz_static_encounters.h)
static const u16 sFlagsJohtoBeasts[] = {0x11EB, 0x11EE, 0x11F1};   // RAIKOU, ENTEI, SUICUNE
static const u16 sFlagsKantoBirds[] = {0x2133, 0x213B, 0x2138};    // ARTICUNO, ZAPDOS, MOLTRES
static const u16 sFlagsJohtoBadges[] = {0x11B2, 0x11B3, 0x11B4, 0x11B5, 0x11B6, 0x11B7, 0x11B8, 0x11B9};
static const u16 sFlagsKantoBadges[] = {0x2100, 0x2101, 0x2102, 0x2103, 0x2104, 0x2105, 0x2106, 0x2107};
// The RUINS OF ALPH's four sliding puzzles solved (KABUTO, HO-OH, OMANYTE, AERODACTYL chambers)
static const u16 sFlagsAlph[] = {0x1076, 0x1077, 0x1078, 0x1079};
// KANTO's 17 ARCEUS trials done (qz_static_encounters.h: the ARCEUS entries' done flags, not ALTERING CAVE's 0x2E16)
static const u16 sFlagsArceusTrials[] = {0x2E19, 0x2E1B, 0x2E1D, 0x2E1F, 0x2E21, 0x2E23, 0x2E25, 0x2E27, 0x2E29, 0x2E2B,
                                         0x2E2D, 0x2E2F, 0x2E31, 0x2E33, 0x2E35, 0x2E37, 0x2E39};
// SINNOH: its badges (REGION_SINNOH_FLAGS, Platinum's order) and its legendaries' "done" flags (tools/sinnoh/legends.py:
// 0x3D00 + their place in LEGENDS)
static const u16 sFlagsSinnohBadges[] = {0x3320, 0x3321, 0x3322, 0x3323, 0x3324, 0x3325, 0x3326, 0x3327};
static const u16 sFlagsSinnohLakes[] = {0x3D00, 0x3D01, 0x3D02};    // UXIE, AZELF, MESPRIT
static const u16 sFlagsSinnohCreation[] = {0x3D0A, 0x3D0B};        // DIALGA, PALKIA
// KANTO's (FireRed's flags as Quetzal kept them: tools/quetzal_rip/kanto_frlg_flags.json)
static const u16 sFlagsKantoSnorlax[] = {0x21AF, 0x21B0};          // ROUTE 12's and ROUTE 16's
static const u16 sFlagsKantoDrinks[] = {0x2169, 0x216B, 0x2170};   // the CELADON roof girl's three drinks
static const u16 sFlagsKantoRods[] = {0x215B, 0x214B, 0x2164};     // OLD, GOOD, SUPER ROD
static const u16 sFlagsKantoSeafoam[] = {0x2208, 0x2209};          // SEAFOAM's two currents stopped
static const u16 sFlagsKantoGems[] = {0x2161, 0x220A};             // the RUBY found, the SAPPHIRE brought to CELIO
static const u16 sFlagsJohtoTogepi[] = {0x1080, 0x154B};           // ELM's EGG taken, a TOGEPI hatched
static const u16 sFlagsSinnohBarry[] = {TRAINER_FLAG(TRAINER_SINNOH_RIVAL_POKEMON_LEAGUE_CHIMCHAR),
                                        TRAINER_FLAG(TRAINER_SINNOH_RIVAL_POKEMON_LEAGUE_PIPLUP),
                                        TRAINER_FLAG(TRAINER_SINNOH_RIVAL_POKEMON_LEAGUE_TURTWIG)};
static const struct { const u16 *list; u8 count; } sFlagLists[] = {
    [FL_KANTO_SNORLAX] = {sFlagsKantoSnorlax, ARRAY_COUNT(sFlagsKantoSnorlax)},
    [FL_KANTO_DRINKS] = {sFlagsKantoDrinks, ARRAY_COUNT(sFlagsKantoDrinks)},
    [FL_KANTO_RODS] = {sFlagsKantoRods, ARRAY_COUNT(sFlagsKantoRods)},
    [FL_KANTO_SEAFOAM] = {sFlagsKantoSeafoam, ARRAY_COUNT(sFlagsKantoSeafoam)},
    [FL_KANTO_GEMS] = {sFlagsKantoGems, ARRAY_COUNT(sFlagsKantoGems)},
    [FL_JOHTO_TOGEPI] = {sFlagsJohtoTogepi, ARRAY_COUNT(sFlagsJohtoTogepi)},
    [FL_SINNOH_BARRY] = {sFlagsSinnohBarry, ARRAY_COUNT(sFlagsSinnohBarry)},
    [FL_SINNOH_BADGES] = {sFlagsSinnohBadges, ARRAY_COUNT(sFlagsSinnohBadges)},
    [FL_SINNOH_LAKES] = {sFlagsSinnohLakes, ARRAY_COUNT(sFlagsSinnohLakes)},
    [FL_SINNOH_CREATION] = {sFlagsSinnohCreation, ARRAY_COUNT(sFlagsSinnohCreation)},
    [FL_JOHTO_BEASTS] = {sFlagsJohtoBeasts, ARRAY_COUNT(sFlagsJohtoBeasts)},
    [FL_KANTO_BIRDS] = {sFlagsKantoBirds, ARRAY_COUNT(sFlagsKantoBirds)},
    [FL_JOHTO_BADGES] = {sFlagsJohtoBadges, ARRAY_COUNT(sFlagsJohtoBadges)},
    [FL_ALPH] = {sFlagsAlph, ARRAY_COUNT(sFlagsAlph)},
    [FL_ARCEUS_TRIALS] = {sFlagsArceusTrials, ARRAY_COUNT(sFlagsArceusTrials)},
    [FL_KANTO_BADGES] = {sFlagsKantoBadges, ARRAY_COUNT(sFlagsKantoBadges)},
    [FL_HMS] = {sFlagsHMs, ARRAY_COUNT(sFlagsHMs)},
    [FL_RODS] = {sFlagsRods, ARRAY_COUNT(sFlagsRods)},
    [FL_LANDMARKS] = {sFlagsLandmarks, ARRAY_COUNT(sFlagsLandmarks)},
    [FL_SILVER] = {sFlagsSilver, ARRAY_COUNT(sFlagsSilver)},
    [FL_GOLD] = {sFlagsGold, ARRAY_COUNT(sFlagsGold)},
};

static const u16 sDexStarters[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
static const u16 sDexFossils[] = {SPECIES_LILEEP, SPECIES_ANORITH};
static const u16 sDexLegends[] = {
    SPECIES_ARTICUNO, SPECIES_ZAPDOS, SPECIES_MOLTRES, SPECIES_MEWTWO, SPECIES_MEW, SPECIES_RAIKOU, SPECIES_ENTEI,
    SPECIES_SUICUNE, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI, SPECIES_REGIROCK, SPECIES_REGICE, SPECIES_REGISTEEL,
    SPECIES_LATIAS, SPECIES_LATIOS, SPECIES_KYOGRE, SPECIES_GROUDON, SPECIES_RAYQUAZA, SPECIES_JIRACHI, SPECIES_DEOXYS,
    // SINNOH's (and ARCEUS, met in KANTO too)
    SPECIES_UXIE, SPECIES_MESPRIT, SPECIES_AZELF, SPECIES_DIALGA, SPECIES_PALKIA, SPECIES_HEATRAN, SPECIES_REGIGIGAS,
    SPECIES_GIRATINA, SPECIES_CRESSELIA, SPECIES_ARCEUS,
};
// The other regions' starters and fossils (their CATCHING tabs' STARTER SET and FOSSIL REVIVAL)
static const u16 sDexStartersJ[] = {SPECIES_CHIKORITA, SPECIES_CYNDAQUIL, SPECIES_TOTODILE};
static const u16 sDexStartersK[] = {SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE};
static const u16 sDexStartersS[] = {SPECIES_TURTWIG, SPECIES_CHIMCHAR, SPECIES_PIPLUP};
static const u16 sDexFossilsK[] = {SPECIES_OMANYTE, SPECIES_KABUTO, SPECIES_AERODACTYL};
static const u16 sDexFossilsS[] = {SPECIES_CRANIDOS, SPECIES_SHIELDON};
static const u16 sDexJohtoBeasts[] = {SPECIES_RAIKOU, SPECIES_ENTEI, SPECIES_SUICUNE};   // (Crystal's: two roam, one at TIN TOWER)
static const struct { const u16 *list; u8 count; } sDexLists[] = {
    [DL_STARTERS] = {sDexStarters, ARRAY_COUNT(sDexStarters)},
    [DL_FOSSILS] = {sDexFossils, ARRAY_COUNT(sDexFossils)},
    [DL_LEGENDS] = {sDexLegends, ARRAY_COUNT(sDexLegends)},
    [DL_STARTERS_J] = {sDexStartersJ, ARRAY_COUNT(sDexStartersJ)},
    [DL_STARTERS_K] = {sDexStartersK, ARRAY_COUNT(sDexStartersK)},
    [DL_STARTERS_S] = {sDexStartersS, ARRAY_COUNT(sDexStartersS)},
    [DL_FOSSILS_K] = {sDexFossilsK, ARRAY_COUNT(sDexFossilsK)},
    [DL_FOSSILS_S] = {sDexFossilsS, ARRAY_COUNT(sDexFossilsS)},
    [DL_JOHTO_BEASTS] = {sDexJohtoBeasts, ARRAY_COUNT(sDexJohtoBeasts)},
};

static u8 AchPulls(u8 tier)
{
    static const u8 sPulls[] = {3, 10, 20};
    return sPulls[tier < ARRAY_COUNT(sPulls) ? tier : 0];
}

static const struct Achievement sAch[] = {
    // ---- The journey (in story order)
    {"start", "FIRST PARTNER", "Get your first POK~MON.", AK_FLAG, FLAG_SYS_POKEMON_GET, 1, 0, C_JOURNEY},
    {"pokedex", "THE POK~DEX", "Receive a POK~DEX from PROF. BIRCH.", AK_FLAG, FLAG_SYS_POKEDEX_GET, 1, 0, C_JOURNEY},
    {"rival103", "FRIENDLY RIVALRY", "Beat your rival on ROUTE 103.", AK_FLAG, FLAG_DEFEATED_RIVAL_ROUTE103, 1, 0, C_JOURNEY},
    {"shoes", "RUNNING SHOES", "Get the RUNNING SHOES from MOM.", AK_FLAG, FLAG_SYS_B_DASH, 1, 0, C_JOURNEY},
    {"badge1", "STONE BADGE", "Beat ROXANNE in RUSTBORO CITY.", AK_FLAG, FLAG_BADGE01_GET, 1, 0, C_JOURNEY},
    {"pokenav", "POK~NAV", "Get the POK~NAV in RUSTBORO CITY.", AK_FLAG, FLAG_RECEIVED_POKENAV, 1, 0, C_JOURNEY},
    {"devon", "SPECIAL DELIVERY", "Deliver the DEVON GOODS in SLATEPORT CITY.", AK_FLAG, FLAG_DELIVERED_DEVON_GOODS, 1, 0, C_JOURNEY},
    {"badge2", "KNUCKLE BADGE", "Beat BRAWLY in DEWFORD TOWN.", AK_FLAG, FLAG_BADGE02_GET, 1, 0, C_JOURNEY},
    {"badge3", "DYNAMO BADGE", "Beat WATTSON in MAUVILLE CITY.", AK_FLAG, FLAG_BADGE03_GET, 1, 0, C_JOURNEY},
    {"bike", "ON TWO WHEELS", "Get a BIKE from RYDEL in MAUVILLE CITY.", AK_FLAG, FLAG_RECEIVED_BIKE, 1, 0, C_JOURNEY},
    {"chimney", "MOUNTAIN SHOWDOWN", "Stop the villains at the top of MT. CHIMNEY.", AK_FLAG, FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY, 1, 0, C_JOURNEY},
    {"badge4", "HEAT BADGE", "Beat FLANNERY in LAVARIDGE TOWN.", AK_FLAG, FLAG_BADGE04_GET, 1, 1, C_JOURNEY},
    {"secretpower", "HIDEOUT", "Get SECRET POWER on ROUTE 111.", AK_FLAG, FLAG_RECEIVED_SECRET_POWER, 1, 0, C_JOURNEY},
    {"badge5", "BALANCE BADGE", "Beat NORMAN in PETALBURG CITY.", AK_FLAG, FLAG_BADGE05_GET, 1, 0, C_JOURNEY},
    {"surf", "SURF'S UP", "Get HM03 SURF.", AK_FLAG, FLAG_RECEIVED_HM_SURF, 1, 0, C_JOURNEY},
    {"badge6", "FEATHER BADGE", "Beat WINONA in FORTREE CITY.", AK_FLAG, FLAG_BADGE06_GET, 1, 0, C_JOURNEY},
    {"orb", "THE ORBS", "Get an ORB at the summit of MT. PYRE.", AK_FLAG, FLAG_RECEIVED_RED_OR_BLUE_ORB, 1, 0, C_JOURNEY},
    {"badge7", "MIND BADGE", "Beat TATE & LIZA in MOSSDEEP CITY.", AK_FLAG, FLAG_BADGE07_GET, 1, 0, C_JOURNEY},
    {"spacecenter", "LIFTOFF", "Save the MOSSDEEP SPACE CENTER.", AK_FLAG, FLAG_DEFEATED_MAGMA_SPACE_CENTER, 1, 1, C_JOURNEY},
    {"rayquaza", "SKY HIGH", "Face RAYQUAZA at the top of SKY PILLAR.", AK_FLAG, FLAG_DEFEATED_RAYQUAZA, 1, 1, C_JOURNEY},
    {"badge8", "RAIN BADGE", "Beat JUAN in SOOTOPOLIS CITY.", AK_FLAG, FLAG_BADGE08_GET, 1, 1, C_JOURNEY},
    {"hms", "HM COLLECTOR", "Get all eight HMs.", AK_FLAGS, FL_HMS, 8, 1, C_JOURNEY},
    {"rods", "FISHING TRIP", "Get the OLD ROD, GOOD ROD and SUPER ROD.", AK_FLAGS, FL_RODS, 3, 1, C_JOURNEY},
    {"wally", "FINAL GATEKEEPER", "Beat WALLY in VICTORY ROAD.", AK_FLAG, FLAG_DEFEATED_WALLY_VICTORY_ROAD, 1, 1, C_JOURNEY},
    {"champion", "CHAMPION", "Enter the HALL OF FAME.", AK_FLAG, FLAG_SYS_GAME_CLEAR, 1, 2, C_JOURNEY},
    {"natdex", "NATIONAL DEX", "Get the NATIONAL POK~DEX.", AK_FLAG, FLAG_SYS_NATIONAL_DEX, 1, 1, C_JOURNEY},
    {"ssticket", "BON VOYAGE", "Get the S.S. TICKET.", AK_FLAG, FLAG_RECEIVED_SS_TICKET, 1, 0, C_JOURNEY},
    {"steven", "STEVEN'S CHALLENGE", "Beat STEVEN in METEOR FALLS.", AK_FLAG, FLAG_DEFEATED_METEOR_FALLS_STEVEN, 1, 2, C_JOURNEY},
    {"kyogre", "SEA BASIN", "Face KYOGRE in the CAVE OF ORIGIN.", AK_FLAG, FLAG_DEFEATED_KYOGRE, 1, 1, C_JOURNEY},
    {"groudon", "CONTINENT", "Face GROUDON.", AK_FLAG, FLAG_DEFEATED_GROUDON, 1, 1, C_JOURNEY},
    {"regis", "THE SEALED ONES", "Face REGIROCK, REGICE and REGISTEEL.", AK_EVENT, 0, 1, 2, C_JOURNEY},
    {"landmark10", "SIGHTSEER", "Find 10 landmarks around HOENN.", AK_FLAGS, FL_LANDMARKS, 10, 0, C_JOURNEY},
    {"landmark25", "EXPLORER", "Find 25 landmarks around HOENN.", AK_FLAGS, FL_LANDMARKS, 25, 2, C_JOURNEY},
    {"symbol", "FRONTIER BRAIN", "Earn a symbol in the BATTLE FRONTIER.", AK_FLAGS, FL_SILVER, 1, 1, C_JOURNEY},
    {"silver", "SILVER COLLECTION", "Earn all seven silver BATTLE FRONTIER symbols.", AK_FLAGS, FL_SILVER, 7, 2, C_JOURNEY},
    {"gold", "GOLDEN AGE", "Earn all seven gold BATTLE FRONTIER symbols.", AK_FLAGS, FL_GOLD, 7, 2, C_JOURNEY},
    {"sudowoodo", "THAT'S NO TREE", "Face the strange tree in the BATTLE FRONTIER.", AK_FLAG, FLAG_DEFEATED_SUDOWOODO, 1, 1, C_JOURNEY},
    {"hof5", "HALL OF FAMER", "Enter the HALL OF FAME 5 times.", AK_STAT, GAME_STAT_ENTERED_HOF, 5, 2, C_JOURNEY},

    // ---- Battle feats
    {"flawless", "FLAWLESS", "Beat a gym leader without one of your POK~MON fainting.", AK_EVENT, 0, 1, 1, C_BATTLE},
    {"underdog", "UNDERDOG", "Beat a gym leader while their ace outlevels all of your POK~MON.", AK_EVENT, 0, 1, 1, C_BATTLE},
    {"solo", "ONE-MAN ARMY", "Beat a gym leader with just one POK~MON in your party.", AK_EVENT, 0, 1, 2, C_BATTLE},
    {"trainers50", "ROOKIE", "Battle 50 trainers.", AK_STAT, GAME_STAT_TRAINER_BATTLES, 50, 0, C_BATTLE},
    {"trainers200", "VETERAN", "Battle 200 trainers.", AK_STAT, GAME_STAT_TRAINER_BATTLES, 200, 1, C_BATTLE},
    {"wild250", "WILD AT HEART", "Battle 250 wild POK~MON.", AK_STAT, GAME_STAT_WILD_BATTLES, 250, 0, C_BATTLE},
    {"lv50", "HALFWAY THERE", "Raise a POK~MON to Lv 50.", AK_LEVEL, 0, 50, 1, C_BATTLE},
    {"lv100", "LEVEL 100", "Raise a POK~MON to Lv 100.", AK_LEVEL, 0, 100, 2, C_BATTLE},
    {"evolve1", "IT'S EVOLVING!", "Evolve a POK~MON.", AK_STAT, GAME_STAT_EVOLVED_POKEMON, 1, 0, C_BATTLE},
    {"evolve20", "EVOLUTIONIST", "Evolve 20 POK~MON.", AK_STAT, GAME_STAT_EVOLVED_POKEMON, 20, 1, C_BATTLE},
    {"struggle", "LAST RESORT", "Run out of PP and use STRUGGLE.", AK_STAT, GAME_STAT_USED_STRUGGLE, 1, 0, C_BATTLE},
    // (added with the achievements update)
    {"trainers10", "FIRST CHALLENGERS", "Battle 10 trainers.", AK_STAT, GAME_STAT_TRAINER_BATTLES, 10, 0, C_BATTLE},
    {"trainers400", "ACE TRAINER", "Battle 400 trainers.", AK_STAT, GAME_STAT_TRAINER_BATTLES, 400, 2, C_BATTLE},
    {"wild50", "INTO THE GRASS", "Battle 50 wild POK~MON.", AK_STAT, GAME_STAT_WILD_BATTLES, 50, 0, C_BATTLE},
    {"wild1000", "WILDLIFE EXPERT", "Battle 1,000 wild POK~MON.", AK_STAT, GAME_STAT_WILD_BATTLES, 1000, 2, C_BATTLE},
    {"battles100", "BATTLE-READY", "Fight 100 battles.", AK_STAT, GAME_STAT_TOTAL_BATTLES, 100, 0, C_BATTLE},
    {"battles1000", "BATTLE LEGEND", "Fight 1,000 battles.", AK_STAT, GAME_STAT_TOTAL_BATTLES, 1000, 2, C_BATTLE},
    {"lv30", "GROWING STRONG", "Raise a POK~MON to Lv 30.", AK_LEVEL, 0, 30, 0, C_BATTLE},
    {"lv70", "POWERHOUSE", "Raise a POK~MON to Lv 70.", AK_LEVEL, 0, 70, 1, C_BATTLE},
    {"team30", "FULL SQUAD", "Have six POK~MON at Lv 30 or higher in your party.", AK_TEAM, 30, 6, 0, C_BATTLE},
    {"team50", "BALANCED TEAM", "Have six POK~MON at Lv 50 or higher in your party.", AK_TEAM, 50, 6, 1, C_BATTLE},
    {"team100", "PERFECT TEAM", "Have six POK~MON at Lv 100 in your party.", AK_TEAM, 100, 6, 2, C_BATTLE},
    {"evolve50", "METAMORPHOSIS", "Evolve 50 POK~MON.", AK_STAT, GAME_STAT_EVOLVED_POKEMON, 50, 2, C_BATTLE},
    {"splash", "MAGIKARP'S SECRET", "Use SPLASH in battle.", AK_STAT, GAME_STAT_USED_SPLASH, 1, 0, C_BATTLE},
    {"tower", "TOWERING", "Enter the BATTLE TOWER.", AK_STAT, GAME_STAT_ENTERED_BATTLE_TOWER, 1, 0, C_BATTLE},
    {"tower7", "SEVEN STRAIGHT", "Win 7 BATTLE TOWER single battles in a row.", AK_STAT, GAME_STAT_BATTLE_TOWER_SINGLES_STREAK, 7, 1, C_BATTLE},
    {"tower50", "TOWER TYCOON", "Win 50 BATTLE TOWER single battles in a row.", AK_STAT, GAME_STAT_BATTLE_TOWER_SINGLES_STREAK, 50, 2, C_BATTLE},

    // ---- Catching
    {"dex25", "COLLECTOR", "Catch 25 kinds of POK~MON.", AK_CAUGHT, 0, 25, 0, C_CATCH},
    {"dex60", "RESEARCHER", "Catch 60 kinds of POK~MON.", AK_CAUGHT, 0, 60, 1, C_CATCH},
    {"dex120", "PROFESSOR'S PRIDE", "Catch 120 kinds of POK~MON.", AK_CAUGHT, 0, 120, 1, C_CATCH},
    {"dex200", "LIVING DEX", "Catch 200 kinds of POK~MON.", AK_CAUGHT, 0, 200, 2, C_CATCH},
    {"seen150", "WATCHER", "See 150 kinds of POK~MON.", AK_SEEN, 0, 150, 0, C_CATCH},
    {"shiny", "SHINY!", "Catch a shiny POK~MON.", AK_EVENT, 0, 1, 2, C_CATCH},
    {"legend", "LEGENDARY", "Catch a legendary POK~MON.", AK_EVENT, 0, 1, 1, C_CATCH},
    {"outbreak", "OUTBREAK HUNTER", "Catch the day's outbreak POK~MON.", AK_EVENT, 0, 1, 0, C_CATCH},
    {"bigcatch", "BIG GAME", "Catch a wild POK~MON at Lv 40 or higher.", AK_EVENT, 0, 1, 0, C_CATCH},
    {"fish50", "ANGLER", "Hook 50 POK~MON with a fishing rod.", AK_STAT, GAME_STAT_FISHING_ENCOUNTERS, 50, 0, C_CATCH},
    {"hatch1", "NEW LIFE", "Hatch an EGG.", AK_STAT, GAME_STAT_HATCHED_EGGS, 1, 0, C_CATCH},
    {"hatch20", "BREEDER", "Hatch 20 EGGS.", AK_STAT, GAME_STAT_HATCHED_EGGS, 20, 1, C_CATCH},
    // (added with the achievements update)
    {"captures10", "BALL TOSSER", "Catch 10 POK~MON.", AK_STAT, GAME_STAT_POKEMON_CAPTURES, 10, 0, C_CATCH},
    {"captures100", "TRAPPER", "Catch 100 POK~MON.", AK_STAT, GAME_STAT_POKEMON_CAPTURES, 100, 1, C_CATCH},
    {"captures500", "MASTER CATCHER", "Catch 500 POK~MON.", AK_STAT, GAME_STAT_POKEMON_CAPTURES, 500, 2, C_CATCH},
    {"dex10", "FIRST PAGES", "Catch 10 kinds of POK~MON.", AK_CAUGHT, 0, 10, 0, C_CATCH},
    {"dex150", "KANTO SIZED", "Catch 150 kinds of POK~MON.", AK_CAUGHT, 0, 150, 1, C_CATCH},
    {"dex300", "ENCYCLOPEDIA", "Catch 300 kinds of POK~MON.", AK_CAUGHT, 0, 300, 2, C_CATCH},
    {"hoenn", "HOENN COMPLETE", "Catch all 200 POK~MON of the HOENN POK~DEX.", AK_HOENN, 0, 200, 2, C_CATCH},
    {"seen50", "SPOTTER", "See 50 kinds of POK~MON.", AK_SEEN, 0, 50, 0, C_CATCH},
    {"seen300", "EYEWITNESS", "See 300 kinds of POK~MON.", AK_SEEN, 0, 300, 1, C_CATCH},
    {"types9", "VARIETY", "Catch POK~MON of 9 different types.", AK_TYPES, 0, 9, 0, C_CATCH},
    {"types17", "TYPE MASTER", "Catch POK~MON of all 17 types.", AK_TYPES, 0, 17, 1, C_CATCH},
    {"starters", "STARTER SET", "Catch TREECKO, TORCHIC and MUDKIP.", AK_DEXLIST, DL_STARTERS, 3, 1, C_CATCH},
    {"fossil", "FOSSIL REVIVAL", "Catch LILEEP or ANORITH.", AK_DEXLIST, DL_FOSSILS, 1, 0, C_CATCH},
    {"legends3", "MYTH CHASER", "Catch 3 legendary POK~MON.", AK_DEXLIST, DL_LEGENDS, 3, 1, C_CATCH},
    {"legends10", "LEGEND KEEPER", "Catch 10 legendary POK~MON.", AK_DEXLIST, DL_LEGENDS, 10, 2, C_CATCH},
    {"owned100", "HOUSEFUL", "Own 100 POK~MON (party and PC).", AK_OWNED, 0, 100, 1, C_CATCH},
    {"owned300", "PC POWER USER", "Own 300 POK~MON (party and PC).", AK_OWNED, 0, 300, 2, C_CATCH},
    {"friend", "BEST BUDDIES", "Have a POK~MON in your party love you completely.", AK_FRIEND, 0, 1, 1, C_CATCH},
    {"fish10", "NIBBLE", "Hook 10 POK~MON with a fishing rod.", AK_STAT, GAME_STAT_FISHING_ENCOUNTERS, 10, 0, C_CATCH},
    {"fish200", "MASTER ANGLER", "Hook 200 POK~MON with a fishing rod.", AK_STAT, GAME_STAT_FISHING_ENCOUNTERS, 200, 1, C_CATCH},
    {"daycare", "DAY CARE REGULAR", "Leave POK~MON at the DAY CARE 5 times.", AK_STAT, GAME_STAT_USED_DAYCARE, 5, 0, C_CATCH},
    {"hatch50", "EGG EXPERT", "Hatch 50 EGGS.", AK_STAT, GAME_STAT_HATCHED_EGGS, 50, 2, C_CATCH},

    // ---- Around HOENN
    {"steps10k", "ON THE ROAD", "Walk 10,000 steps.", AK_STAT, GAME_STAT_STEPS, 10000, 0, C_HOENN},
    {"steps60k", "GLOBETROTTER", "Walk 60,000 steps.", AK_STAT, GAME_STAT_STEPS, 60000, 1, C_HOENN},
    {"contest1", "STAR OF THE STAGE", "Win a POK~MON CONTEST.", AK_STAT, GAME_STAT_WON_CONTEST, 1, 0, C_HOENN},
    {"contest10", "CONTEST IDOL", "Win 10 POK~MON CONTESTS.", AK_STAT, GAME_STAT_WON_CONTEST, 10, 1, C_HOENN},
    {"ribbons", "DECORATED", "Earn 10 ribbons.", AK_STAT, GAME_STAT_RECEIVED_RIBBONS, 10, 1, C_HOENN},
    {"jackpot", "JACKPOT!", "Hit the jackpot at the GAME CORNER slots.", AK_STAT, GAME_STAT_SLOT_JACKPOTS, 1, 1, C_HOENN},
    {"berries", "GREEN THUMB", "Plant 30 BERRIES.", AK_STAT, GAME_STAT_PLANTED_BERRIES, 30, 0, C_HOENN},
    {"safari", "SAFARI GOER", "Visit the SAFARI ZONE 5 times.", AK_STAT, GAME_STAT_ENTERED_SAFARI_ZONE, 5, 0, C_HOENN},
    {"cablecar", "HIGH ROAD", "Ride the cable car 10 times.", AK_STAT, GAME_STAT_RODE_CABLE_CAR, 10, 0, C_HOENN},
    {"ledges", "PARKOUR", "Jump down 300 ledges.", AK_STAT, GAME_STAT_JUMPED_DOWN_LEDGES, 300, 0, C_HOENN},
    // (added with the achievements update)
    {"center10", "CHECK-UP", "Heal at a POK~MON CENTER 10 times.", AK_STAT, GAME_STAT_USED_POKECENTER, 10, 0, C_HOENN},
    {"center100", "NURSE'S FAVORITE", "Heal at a POK~MON CENTER 100 times.", AK_STAT, GAME_STAT_USED_POKECENTER, 100, 1, C_HOENN},
    {"shop10", "SHOPPER", "Shop at a POK~MON MART 10 times.", AK_STAT, GAME_STAT_SHOPPED, 10, 0, C_HOENN},
    {"shop100", "BIG SPENDER", "Shop at a POK~MON MART 100 times.", AK_STAT, GAME_STAT_SHOPPED, 100, 1, C_HOENN},
    {"money100k", "WELL OFF", "Have $100,000 at once.", AK_MONEY, 0, 100000, 1, C_HOENN},
    {"money999k", "TYCOON", "Have $999,999 at once.", AK_MONEY, 0, 999999, 2, C_HOENN},
    {"home", "HOME SWEET HOME", "Rest at home 5 times.", AK_STAT, GAME_STAT_RESTED_AT_HOME, 5, 0, C_HOENN},
    {"cut50", "LUMBERJACK", "Use CUT 50 times.", AK_STAT, GAME_STAT_USED_CUT, 50, 1, C_HOENN},
    {"smash50", "ROCK BREAKER", "Use ROCK SMASH 50 times.", AK_STAT, GAME_STAT_USED_ROCK_SMASH, 50, 1, C_HOENN},
    {"rain", "UNDER THE WEATHER", "Get caught in the rain.", AK_STAT, GAME_STAT_GOT_RAINED_ON, 1, 0, C_HOENN},
    {"tv", "COUCH POTATO", "Watch TV 20 times.", AK_STAT, GAME_STAT_WATCHED_TV, 20, 0, C_HOENN},
    {"interview", "ON THE AIR", "Get interviewed 3 times.", AK_STAT, GAME_STAT_GOT_INTERVIEWED, 3, 0, C_HOENN},
    {"trend", "TRENDSETTER", "Start a trend.", AK_STAT, GAME_STAT_STARTED_TRENDS, 1, 0, C_HOENN},
    {"lottery", "LUCKY NUMBER", "Win the POK~MON LOTTERY.", AK_STAT, GAME_STAT_WON_POKEMON_LOTTERY, 1, 1, C_HOENN},
    {"springs", "HOT SPRINGS", "Relax in the LAVARIDGE hot springs.", AK_STAT, GAME_STAT_ENTERED_HOT_SPRINGS, 1, 0, C_HOENN},
    {"bikes", "BIKE SWAP", "Trade your BIKE at RYDEL'S CYCLES.", AK_STAT, GAME_STAT_TRADED_BIKES, 1, 0, C_HOENN},
    {"basemove", "NEW DIGS", "Move your SECRET BASE.", AK_STAT, GAME_STAT_MOVED_SECRET_BASE, 1, 0, C_HOENN},
    {"blocks50", "POK~BLOCK CHEF", "Make 50 POK~BLOCKS.", AK_STAT, GAME_STAT_POKEBLOCKS, 50, 1, C_HOENN},
    {"contests20", "PERFORMER", "Enter 20 POK~MON CONTESTS.", AK_STAT, GAME_STAT_ENTERED_CONTEST, 20, 1, C_HOENN},
    {"ribbons30", "RIBBON COLLECTOR", "Earn 30 ribbons.", AK_STAT, GAME_STAT_RECEIVED_RIBBONS, 30, 2, C_HOENN},
    {"finder", "TREASURE HUNTER", "Use the ITEMFINDER 30 times.", AK_STAT, GAME_STAT_USED_ITEMFINDER, 30, 1, C_HOENN},
    {"roulette", "HOT STREAK", "Win 5 games of roulette in a row.", AK_STAT, GAME_STAT_CONSECUTIVE_ROULETTE_WINS, 5, 1, C_HOENN},
    {"berries100", "BERRY FARMER", "Plant 100 BERRIES.", AK_STAT, GAME_STAT_PLANTED_BERRIES, 100, 1, C_HOENN},
    {"steps150k", "WORLD TRAVELER", "Walk 150,000 steps.", AK_STAT, GAME_STAT_STEPS, 150000, 2, C_HOENN},

    // ---- Together
    {"coop1", "BETTER TOGETHER", "Win a co-op trainer battle with a friend.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"coop25", "DYNAMIC DUO", "Win 25 co-op trainer battles.", AK_COUNT, 0, 25, 1, C_TOGETHER},
    {"coopgym", "TAG-TEAM GYM", "Beat a gym leader in a co-op battle.", AK_COUNT, 0, 1, 1, C_TOGETHER},
    {"coopcatch", "CATCH TOGETHER", "Catch a POK~MON in a co-op wild battle.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"coopcatch20", "PACK HUNTERS", "Catch 20 POK~MON in co-op wild battles.", AK_COUNT, 0, 20, 1, C_TOGETHER},
    {"rescue", "LIFESAVER", "Help a downed friend back up.", AK_COUNT, 0, 1, 1, C_TOGETHER},
    {"selfrevive", "SECOND WIND", "Get back up with your own REVIVE.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"lend", "HELPING HAND", "Have friends use your field moves 10 times.", AK_COUNT, 0, 10, 1, C_TOGETHER},
    {"trade", "TRADER", "Trade 5 POK~MON with friends.", AK_COUNT, 0, 5, 1, C_TOGETHER},
    {"gift", "GENEROUS", "Send 30 daily gifts.", AK_COUNT, 0, 30, 1, C_TOGETHER},
    {"photo3", "SAY CHEESE!", "Take a TEAM PHOTO with 3 or more players.", AK_EVENT, 0, 1, 0, C_TOGETHER},
    {"fullhouse", "FULL HOUSE", "Play with four players online at once.", AK_EVENT, 0, 1, 0, C_TOGETHER},
    {"pvpwin", "FRIENDLY RIVALS", "Win a battle against a friend.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"pvpwin10", "CHAMPION OF FRIENDS", "Win 10 battles against friends.", AK_COUNT, 0, 10, 1, C_TOGETHER},
    {"bet", "HIGH ROLLER", "Win 5 bets while watching friends battle.", AK_COUNT, 0, 5, 0, C_TOGETHER},
    {"tourwin", "TOURNAMENT CHAMP", "Win a battle tournament.", AK_COUNT, 0, 1, 2, C_TOGETHER},
    {"rematch", "GYM CIRCUIT", "Win 5 gym team rematches.", AK_COUNT, 0, 5, 1, C_TOGETHER},
    {"blend", "BERRY BLENDERS", "Blend BERRIES with friends 5 times.", AK_COUNT, 0, 5, 0, C_TOGETHER},
    {"base", "HOUSE CALL", "Visit a friend's SECRET BASE.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"card", "PEN PALS", "Swap TRAINER CARDS with a friend.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"streak7", "A WEEK TOGETHER", "Keep a play-together streak for 7 days.", AK_STREAK, 0, 7, 1, C_TOGETHER},
    {"streak30", "BEST FRIENDS", "Keep a play-together streak for 30 days.", AK_STREAK, 0, 30, 2, C_TOGETHER},
    {"goals10", "GOAL GETTERS", "Finish all three daily goals on 10 days.", AK_COUNT, 0, 10, 1, C_TOGETHER},
    {"racewin", "PHOTO FINISH", "Win a weekly race against friends.", AK_COUNT, 0, 1, 1, C_TOGETHER},
    // (added with the achievements update)
    {"coop100", "UNBREAKABLE BOND", "Win 100 co-op trainer battles.", AK_COUNT, 0, 100, 2, C_TOGETHER},
    {"triple1", "TRIPLE THREAT", "Win a TRIPLE BATTLE with two friends.", AK_COUNT, 0, 1, 1, C_TOGETHER},
    {"triple25", "THREE MUSKETEERS", "Win 25 TRIPLE BATTLES with friends.", AK_COUNT, 0, 25, 2, C_TOGETHER},
    {"coopcatch100", "WOLF PACK", "Catch 100 POK~MON in co-op wild battles.", AK_COUNT, 0, 100, 2, C_TOGETHER},
    {"rescue10", "GUARDIAN ANGEL", "Help downed friends back up 10 times.", AK_COUNT, 0, 10, 1, C_TOGETHER},
    {"lend50", "SWISS ARMY TEAM", "Have friends use your field moves 50 times.", AK_COUNT, 0, 50, 2, C_TOGETHER},
    {"trade25", "TRADE BROKER", "Trade 25 POK~MON with friends.", AK_COUNT, 0, 25, 2, C_TOGETHER},
    {"gift100", "SECRET SANTA", "Send 100 daily gifts.", AK_COUNT, 0, 100, 2, C_TOGETHER},
    {"pvpwin50", "UNDISPUTED", "Win 50 battles against friends.", AK_COUNT, 0, 50, 2, C_TOGETHER},
    {"rematch20", "GYM CIRCUIT PRO", "Win 20 gym team rematches.", AK_COUNT, 0, 20, 2, C_TOGETHER},
    {"blend25", "SMOOTHIE BAR", "Blend BERRIES with friends 25 times.", AK_COUNT, 0, 25, 1, C_TOGETHER},
    {"crush10", "BERRY CRUSHERS", "Play BERRY CRUSH 10 times.", AK_STAT, GAME_STAT_PLAYED_BERRY_CRUSH, 10, 1, C_TOGETHER},
    {"linkcontest", "CROWD FAVORITE", "Win a POK~MON CONTEST against friends.", AK_STAT, GAME_STAT_WON_LINK_CONTEST, 1, 1, C_TOGETHER},
    {"goals50", "GOAL MACHINE", "Finish all three daily goals on 50 days.", AK_COUNT, 0, 50, 2, C_TOGETHER},
    {"racewin5", "SPEEDSTER", "Win 5 weekly races against friends.", AK_COUNT, 0, 5, 2, C_TOGETHER},
    {"streak100", "INSEPARABLE", "Keep a play-together streak for 100 days.", AK_STREAK, 0, 100, 2, C_TOGETHER},
    {"emote100", "EXPRESSIVE", "Send 100 emotes or phrases.", AK_COUNT, 0, 100, 0, C_TOGETHER},

    // ---- MOEGATCHA, looks and buddies
    // (added with the achievements update)
    {"pull10", "FIRST SPINS", "Make 10 MOEGATCHA pulls.", AK_COUNT, 0, 10, 0, C_EXTRAS},
    {"pull100", "WHEEL OF FORTUNE", "Make 100 MOEGATCHA pulls.", AK_COUNT, 0, 100, 1, C_EXTRAS},
    {"pull500", "GACHA DEVOTEE", "Make 500 MOEGATCHA pulls.", AK_COUNT, 0, 500, 2, C_EXTRAS},
    {"sr", "SUPER RARE", "Win an SR prize from the MOEGATCHA.", AK_EVENT, 0, 1, 0, C_EXTRAS},
    {"ssr", "SUPER SUPER RARE", "Win an SSR prize from the MOEGATCHA.", AK_EVENT, 0, 1, 1, C_EXTRAS},
    {"ur", "ULTRA RARE", "Win a UR prize from the MOEGATCHA.", AK_EVENT, 0, 1, 2, C_EXTRAS},
    {"eventmon", "SPECIAL GUEST", "Get an event POK~MON from the MOEGATCHA.", AK_EVENT, 0, 1, 1, C_EXTRAS},
    {"ballstyle", "BALL STYLIST", "Change a POK~MON's POK~ BALL look.", AK_EVENT, 0, 1, 0, C_EXTRAS},
    {"follower", "WALKING BUDDY", "Have a POK~MON follow you.", AK_EVENT, 0, 1, 0, C_EXTRAS},
    {"newlook", "NEW LOOK", "Change your look in LOOK or CREATE.", AK_EVENT, 0, 1, 0, C_EXTRAS},
    // VS cabinet fights (mp_fight.c): counted when a fight ends
    {"fightcpu1", "INSERT COIN", "Beat the CPU at a VS cabinet.", AK_COUNT, 0, 1, 0, C_EXTRAS},
    {"fightcpu25", "ARCADE MODE", "Beat the CPU 25 times at VS cabinets.", AK_COUNT, 0, 25, 1, C_EXTRAS},
    {"fightwin1", "A NEW CHALLENGER", "Win a fight against a friend at a VS cabinet.", AK_COUNT, 0, 1, 0, C_TOGETHER},
    {"fightwin10", "WORLD WARRIOR", "Win 10 fights against friends at VS cabinets.", AK_COUNT, 0, 10, 1, C_TOGETHER},
    {"fightwin50", "GRAND MASTER", "Win 50 fights against friends at VS cabinets.", AK_COUNT, 0, 50, 2, C_TOGETHER},
    // Card duels (src/tcg): counted and unlocked by the card screens
    {"cardwin1", "FIRST DRAW", "Win your first card duel.", AK_COUNT, 0, 1, 0, C_CARDS},
    {"cardwin100", "CARD SHARK", "Win 100 card duels.", AK_COUNT, 0, 100, 2, C_CARDS},
    {"cardgyms8", "GYM CARD MASTER", "Beat every GYM LEADER's card deck.", AK_COUNT, 0, 8, 1, C_CARDS},
    {"cardspecial15", "LEGENDS OF THE TABLE", "Beat 15 special trainers at cards (either game).", AK_COUNT, 0, 15, 2, C_CARDS},
    {"cardset", "SET COMPLETE", "Own every card of a booster set.", AK_EVENT, 0, 1, 1, C_CARDS},
    {"cardall", "CARD MASTER", "Own every POK~MON card.", AK_EVENT, 0, 1, 2, C_CARDS},
    {"cardpack50", "PACK RAT", "Open 50 booster packs.", AK_COUNT, 0, 50, 1, C_CARDS},
    {"cardflawless", "CLEAN SWEEP", "Win a card duel without giving up a Prize.", AK_EVENT, 0, 1, 1, C_CARDS},
    {"cardfriend1", "TABLE TALK", "Win a card duel against a friend.", AK_COUNT, 0, 1, 0, C_CARDS},
    {"cardtrade", "CARD SWAP", "Trade a card with a friend.", AK_EVENT, 0, 1, 0, C_CARDS},
    {"cardmono", "PURE ENERGY", "Win a card duel with a deck of one type.", AK_EVENT, 0, 1, 1, C_CARDS},
    {"cardpromo", "PROMO HUNTER", "Win a special trainer's promo card.", AK_EVENT, 0, 1, 1, C_CARDS},
    // Yu-Gi-Oh! duels (src/ygo): counted and unlocked by the card screens
    {"ygostart", "IT'S TIME TO DUEL", "Get a Yu-Gi-Oh! Starter Deck.", AK_EVENT, 0, 1, 0, C_YUGIOH},
    {"ygowin1", "FIRST DUEL", "Win your first Yu-Gi-Oh! duel.", AK_COUNT, 0, 1, 0, C_YUGIOH},
    {"ygowin100", "KING OF GAMES", "Win 100 Yu-Gi-Oh! duels.", AK_COUNT, 0, 100, 2, C_YUGIOH},
    {"ygospecial12", "DUELIST KINGDOM", "Beat 12 special duelists at Yu-Gi-Oh!.", AK_COUNT, 0, 12, 2, C_YUGIOH},
    {"ygosignature", "SIGNATURE MOVE", "Win a special duelist's signature card.", AK_EVENT, 0, 1, 1, C_YUGIOH},
    {"ygopack50", "BOOSTER BOX", "Open 50 Yu-Gi-Oh! booster packs.", AK_COUNT, 0, 50, 1, C_YUGIOH},
    {"ygosecret", "SECRET RARE", "Pull a Secret Rare from a booster pack.", AK_EVENT, 0, 1, 1, C_YUGIOH},
    {"ygoflawless", "UNTOUCHED", "Win a Yu-Gi-Oh! duel without losing any Life Points.", AK_EVENT, 0, 1, 1, C_YUGIOH},
    {"ygoexodia", "OBLITERATE!", "Win a duel with all five pieces of Exodia.", AK_EVENT, 0, 1, 2, C_YUGIOH},
    // MAGIC duels (src/mtg): counted and unlocked by the card screens (mtg_ui.c) and the table (mtg_board.c)
    {"mtgstart", "THE GATHERING", "Get a MAGIC starter deck.", AK_EVENT, 0, 1, 0, C_MAGIC},
    {"mtgwin1", "UNTAP, UPKEEP, DRAW", "Win your first MAGIC duel.", AK_COUNT, 0, 1, 0, C_MAGIC},
    {"mtgwin100", "PLANESWALKER", "Win 100 MAGIC duels.", AK_COUNT, 0, 100, 2, C_MAGIC},
    {"mtgregulars6", "MOSSDEEP REGULAR", "Beat all 6 regulars at MOSSDEEP's CARD SHOP.", AK_COUNT, 0, 6, 1, C_MAGIC},
    {"mtgpack50", "BOOSTER DRAFT", "Open 50 MAGIC booster packs.", AK_COUNT, 0, 50, 1, C_MAGIC},
    {"mtgsets5", "SET HOPPER", "Open boosters of 5 different MAGIC sets.", AK_EVENT, 0, 1, 1, C_MAGIC},
    {"mtgkinds250", "THE LIBRARY", "Own 250 different MAGIC cards.", AK_EVENT, 0, 1, 2, C_MAGIC},
    {"mtgflawless", "NOT A SCRATCH", "Win a MAGIC duel at 20 life or more.", AK_EVENT, 0, 1, 1, C_MAGIC},
    {"mtgfast", "QUICK DRAW", "Win a MAGIC duel within your first five turns.", AK_EVENT, 0, 1, 1, C_MAGIC},
    {"mtgdecked", "DECKED", "Win as your opponent runs out of cards to draw.", AK_EVENT, 0, 1, 1, C_MAGIC},
    {"mtgcounter", "NOT SO FAST", "Counter one of your opponent's spells.", AK_EVENT, 0, 1, 0, C_MAGIC},
    {"mtgmono", "TRUE COLORS", "Win a MAGIC duel with a deck of one color.", AK_EVENT, 0, 1, 0, C_MAGIC},
    {"mtgdomain", "ALL FIVE", "Win a MAGIC duel with a deck of all five colors.", AK_EVENT, 0, 1, 2, C_MAGIC},
    // Triple Triad (src/tt): counted and unlocked by the card screens
    {"ttstart", "FIRST HAND", "Get your first TRIPLE TRIAD cards.", AK_EVENT, 0, 1, 0, C_TRIAD},
    {"ttwin1", "OPEN RULES", "Win a game of TRIPLE TRIAD.", AK_COUNT, 0, 1, 0, C_TRIAD},
    {"ttwin50", "SHUFFLE OR BOOGIE", "Win 50 games of TRIPLE TRIAD.", AK_COUNT, 0, 50, 1, C_TRIAD},
    {"ttwin200", "MASTER OF THE TABLE", "Win 200 games of TRIPLE TRIAD.", AK_COUNT, 0, 200, 2, C_TRIAD},
    {"ttperfect", "PERFECT HAND", "Win a game holding nine of the ten cards.", AK_EVENT, 0, 1, 1, C_TRIAD},
    {"ttsame10", "SAME AS IT EVER WAS", "Take cards with SAME 10 times.", AK_COUNT, 0, 10, 1, C_TRIAD},
    {"ttplus10", "PLUS ONE", "Take cards with PLUS 10 times.", AK_COUNT, 0, 10, 1, C_TRIAD},
    {"ttcombo1", "COMBO!", "Start a COMBO.", AK_COUNT, 0, 1, 0, C_TRIAD},
    {"ttcombo25", "CHAIN REACTION", "Start 25 COMBOs.", AK_COUNT, 0, 25, 2, C_TRIAD},
    {"ttspread1", "RULES ON THE MOVE", "Carry a rule to a region that didn't play it.", AK_COUNT, 0, 1, 0, C_TRIAD},
    {"ttspread10", "TRENDSETTER", "Spread rules to other regions 10 times.", AK_COUNT, 0, 10, 1, C_TRIAD},
    {"ttabolish1", "ABOLISHED!", "See a region's rule abolished.", AK_COUNT, 0, 1, 0, C_TRIAD},
    {"ttqueen", "BY ROYAL DECREE", "Beat the CARD QUEEN.", AK_EVENT, 0, 1, 1, C_TRIAD},
    {"ttspecial10", "TRIAD LEGENDS", "Beat 10 special players at TRIPLE TRIAD.", AK_COUNT, 0, 10, 2, C_TRIAD},
    {"ttcards100", "CARD CARRIER", "Win 100 cards at the table.", AK_COUNT, 0, 100, 1, C_TRIAD},
    {"tt5star", "FIVE STARS", "Win a five-star card.", AK_EVENT, 0, 1, 1, C_TRIAD},
    {"ttpack25", "PACK OPENER", "Open 25 TRIPLE TRIAD packs.", AK_COUNT, 0, 25, 1, C_TRIAD},
    {"ttall", "TRIPLE TRIAD MASTER", "Own every TRIPLE TRIAD card.", AK_EVENT, 0, 1, 2, C_TRIAD},
    // The DANCE machine (mp_rhythm.c MP_Rhythm_StageDone): made for players who know their DDR
    {"ddrclear1", "FIRST STEPS", "Clear a song on the DANCE machine.", AK_COUNT, 0, 1, 0, C_DANCE},
    {"ddrclear100", "DANCE MANIA", "Clear 100 songs on the DANCE machine.", AK_COUNT, 0, 100, 1, C_DANCE},
    {"ddrclear1000", "BEMANI LIFER", "Clear 1000 songs on the DANCE machine.", AK_COUNT, 0, 1000, 2, C_DANCE},
    {"ddrsongs50", "SONG HUNTER", "Clear 50 different songs.", AK_COUNT, 0, 50, 1, C_DANCE},
    {"ddrsongs240", "THE WHOLE EXTREME", "Clear every one of the 240 EXTREME songs.", AK_COUNT, 0, 240, 2, C_DANCE},
    {"ddrchallenge", "CHALLENGER", "Clear a CHALLENGE chart.", AK_EVENT, 0, 1, 0, C_DANCE},
    {"ddrfc1", "FULL COMBO", "Full combo a song (GREAT or better).", AK_COUNT, 0, 1, 0, C_DANCE},
    {"ddrfc100", "COMBO KING", "Full combo 100 songs.", AK_COUNT, 0, 100, 2, C_DANCE},
    {"ddrpfc1", "PERFECT FULL COMBO", "Full combo a song with PERFECT or better.", AK_COUNT, 0, 1, 1, C_DANCE},
    {"ddrpfc50", "CRYSTAL CLEAR", "PERFECT full combo 50 songs.", AK_COUNT, 0, 50, 2, C_DANCE},
    {"ddrmfc", "MARVELOUS!", "MARVELOUS full combo a song (MARVELOUS on).", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrmfcten", "BEYOND HUMAN", "MARVELOUS full combo a 10-foot chart.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddraaanine", "NINE LIVES", "Get an AAA on a 9-foot chart.", AK_EVENT, 0, 1, 1, C_DANCE},
    {"ddraaaten", "PERFECT TEN", "Get an AAA on a 10-foot chart.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrmaxthreehundred", "MAX 300", "Clear MAX 300 on HEAVY or CHALLENGE.", AK_EVENT, 0, 1, 1, C_DANCE},
    {"ddrmaxaa", "MAXIMUM AA", "AA or better on MAX 300, HEAVY or harder.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrsurvivor", "SURVIVOR", "Clear PARANOiA survivor MAX, HEAVY or harder.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrboss10", "BOSS RUSH", "Clear all 10 bosses on HEAVY or CHALLENGE.", AK_COUNT, 0, 10, 2, C_DANCE},
    {"ddrcombo", "1000 COMBO", "Reach a 1000 combo in one song.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrfourfc", "FOUR FOR FOUR", "Full combo all four stages of a credit.", AK_EVENT, 0, 1, 1, C_DANCE},
    {"ddrfouraaa", "FLAWLESS CREDIT", "Get AAA on all four stages of a credit.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrstealth", "BLIND FAITH", "Clear a 9-foot chart or harder on STEALTH.", AK_EVENT, 0, 1, 2, C_DANCE},
    {"ddrspeed", "LUDICROUS SPEED", "Clear a song at x8.", AK_EVENT, 0, 1, 0, C_DANCE},
    {"ddrrecord", "MACHINE RECORD", "Set the server's best score on a chart.", AK_EVENT, 0, 1, 1, C_DANCE},
    {"ddrvs1", "DANCE BATTLE", "Beat a friend in VERSUS.", AK_COUNT, 0, 1, 0, C_DANCE},
    {"ddrvs25", "DANCE FLOOR LEGEND", "Beat friends in VERSUS 25 times.", AK_COUNT, 0, 25, 2, C_DANCE},
    {"ddrcredit50", "REGULAR", "Play 50 credits on the DANCE machine.", AK_COUNT, 0, 50, 1, C_DANCE},
    // JOHTO and KANTO (1.61.3)
    {"travel3", "WORLD TRAVELER", "Visit HOENN, JOHTO and KANTO.", AK_STARTED, 0, 3, 1, C_JOURNEY},
    {"champion3", "TRIPLE CROWN", "Become CHAMPION of HOENN, JOHTO and KANTO.", AK_CLEARED, 0, 3, 2, C_JOURNEY},
    {"jstart", "NEW BARK DAYS", "Get your first POK~MON from PROF. ELM.", AK_FLAG, 0x11D1, 1, 0, C_JOHTO},
    {"jbadge1", "ZEPHYR BADGE", "Beat FALKNER in VIOLET CITY.", AK_FLAG, 0x11B2, 1, 0, C_JOHTO},
    {"jbadge2", "HIVE BADGE", "Beat BUGSY in AZALEA TOWN.", AK_FLAG, 0x11B3, 1, 0, C_JOHTO},
    {"jbadge3", "PLAIN BADGE", "Beat WHITNEY in GOLDENROD CITY.", AK_FLAG, 0x11B4, 1, 0, C_JOHTO},
    {"jbadge4", "FOG BADGE", "Beat MORTY in ECRUTEAK CITY.", AK_FLAG, 0x11B5, 1, 1, C_JOHTO},
    {"jbadge5", "STORM BADGE", "Beat CHUCK in CIANWOOD CITY.", AK_FLAG, 0x11B6, 1, 0, C_JOHTO},
    {"jbadge6", "MINERAL BADGE", "Beat JASMINE in OLIVINE CITY.", AK_FLAG, 0x11B7, 1, 0, C_JOHTO},
    {"jbadge7", "GLACIER BADGE", "Beat PRYCE in MAHOGANY TOWN.", AK_FLAG, 0x11B8, 1, 0, C_JOHTO},
    {"jbadge8", "RISING BADGE", "Earn CLAIR's badge in BLACKTHORN CITY.", AK_FLAG, 0x11B9, 1, 1, C_JOHTO},
    {"jbadges", "JOHTO'S EIGHT", "Hold all eight JOHTO badges.", AK_FLAGS, FL_JOHTO_BADGES, 8, 1, C_JOHTO},
    {"jchampion", "JOHTO CHAMPION", "Beat JOHTO's ELITE FOUR and CHAMPION.", AK_CLEARED, 1 + 1, 1, 2, C_JOHTO},
    // (jbeasts / jcelebi counted Moemon Quetzal's standing beasts and CELEBI, gone from JOHTO: Crystal's own now)
    {"jbeasts", "LEGENDARY BEASTS", "Catch RAIKOU, ENTEI and SUICUNE.", AK_DEXLIST, DL_JOHTO_BEASTS, 3, 2, C_JOHTO},
    {"jlugia", "SILVER WING", "Face LUGIA in JOHTO.", AK_FLAG, 0x11F4, 1, 1, C_JOHTO},
    {"jhooh", "RAINBOW WING", "Face HO-OH in JOHTO.", AK_FLAG, 0x11F7, 1, 1, C_JOHTO},
    {"jcelebi", "VOICE OF THE FOREST", "Catch CELEBI at ILEX FOREST's shrine.", AK_FLAG, 0x1150, 1, 1, C_JOHTO},
    {"jalph", "ANCIENT PUZZLES", "Solve all four RUINS OF ALPH puzzles.", AK_FLAGS, FL_ALPH, 4, 1, C_JOHTO},
    {"jbugwin", "BUG-CATCHING CHAMP", "Win first place in the Bug-Catching Contest.", AK_EVENT, 0, 1, 1, C_JOHTO},
    // (MELOETTA and DARKRAI were Moemon Quetzal's: no longer met in JOHTO or KANTO, their achievements went with them,
    // the user 2026-11-08: unlisted rows, one already earned stays a TRAINER CARD title)
    {"jmeloetta", "THE MELODY", "Face MELOETTA in JOHTO.", AK_FLAG, 0x1263, 1, 1, C_GONE},
    {"jdarkrai", "A BAD DREAM", "Face DARKRAI in JOHTO.", AK_FLAG, 0x1239, 1, 1, C_GONE},
    {"kstart", "PALLET BEGINNINGS", "Get your first POK~MON from PROF. OAK.", AK_FLAG, 0x21E8, 1, 0, C_KANTO},
    {"kbadge1", "BOULDER BADGE", "Beat BROCK in PEWTER CITY.", AK_FLAG, 0x2100, 1, 0, C_KANTO},
    {"kbadge2", "CASCADE BADGE", "Beat MISTY in CERULEAN CITY.", AK_FLAG, 0x2101, 1, 0, C_KANTO},
    {"kbadge3", "THUNDER BADGE", "Beat LT. SURGE in VERMILION CITY.", AK_FLAG, 0x2102, 1, 0, C_KANTO},
    {"kbadge4", "RAINBOW BADGE", "Beat ERIKA in CELADON CITY.", AK_FLAG, 0x2103, 1, 1, C_KANTO},
    {"kbadge5", "SOUL BADGE", "Beat KOGA in FUCHSIA CITY.", AK_FLAG, 0x2104, 1, 0, C_KANTO},
    {"kbadge6", "MARSH BADGE", "Beat SABRINA in SAFFRON CITY.", AK_FLAG, 0x2105, 1, 0, C_KANTO},
    {"kbadge7", "VOLCANO BADGE", "Beat BLAINE on CINNABAR ISLAND.", AK_FLAG, 0x2106, 1, 0, C_KANTO},
    {"kbadge8", "EARTH BADGE", "Beat the leader of VIRIDIAN CITY.", AK_FLAG, 0x2107, 1, 1, C_KANTO},
    {"kbadges", "KANTO'S EIGHT", "Hold all eight KANTO badges.", AK_FLAGS, FL_KANTO_BADGES, 8, 1, C_KANTO},
    {"kchampion", "KANTO CHAMPION", "Beat KANTO's ELITE FOUR and CHAMPION.", AK_CLEARED, 2 + 1, 1, 2, C_KANTO},
    {"kbirds", "LEGENDARY BIRDS", "Face ARTICUNO, ZAPDOS and MOLTRES in KANTO.", AK_FLAGS, FL_KANTO_BIRDS, 3, 2, C_KANTO},
    {"kmewtwo", "GENETIC MASTERPIECE", "Face MEWTWO in KANTO.", AK_FLAG, 0x2137, 1, 2, C_KANTO},
    {"kmew", "UNDER THE TRUCK", "Find MEW in KANTO.", AK_FLAG, 0x292F, 1, 2, C_KANTO},
    {"kmeloetta", "CHANSEY DANCE", "Face MELOETTA in KANTO.", AK_FLAG, 0x2922, 1, 1, C_GONE},
    {"kdarkrai", "NIGHTMARE AT SEA", "Face DARKRAI in KANTO.", AK_FLAG, 0x2929, 1, 1, C_GONE},
    {"travel4", "WORLD TOUR", "Visit HOENN, JOHTO, KANTO and SINNOH.", AK_STARTED, 0xFF, 4, 1, C_JOURNEY},
    {"champion4", "GRAND CHAMPION", "Become CHAMPION of all four regions.", AK_CLEARED, 0xFF, 4, 2, C_JOURNEY},
    {"sarrive", "ACROSS THE SEA", "Arrive in SINNOH at CANALAVE's port.", AK_STARTED, 3 + 1, 1, 0, C_SINNOH},
    {"sbadge1", "COAL BADGE", "Beat ROARK in OREBURGH CITY.", AK_FLAG, 0x3320, 1, 0, C_SINNOH},
    {"sbadge2", "FOREST BADGE", "Beat GARDENIA in ETERNA CITY.", AK_FLAG, 0x3321, 1, 0, C_SINNOH},
    {"sbadge3", "COBBLE BADGE", "Beat MAYLENE in VEILSTONE CITY.", AK_FLAG, 0x3322, 1, 0, C_SINNOH},
    {"sbadge4", "FEN BADGE", "Beat CRASHER WAKE in PASTORIA CITY.", AK_FLAG, 0x3323, 1, 1, C_SINNOH},
    {"sbadge5", "RELIC BADGE", "Beat FANTINA in HEARTHOME CITY.", AK_FLAG, 0x3324, 1, 0, C_SINNOH},
    {"sbadge6", "MINE BADGE", "Beat BYRON in CANALAVE CITY.", AK_FLAG, 0x3325, 1, 0, C_SINNOH},
    {"sbadge7", "ICICLE BADGE", "Beat CANDICE in SNOWPOINT CITY.", AK_FLAG, 0x3326, 1, 0, C_SINNOH},
    {"sbadge8", "BEACON BADGE", "Beat VOLKNER in SUNYSHORE CITY.", AK_FLAG, 0x3327, 1, 1, C_SINNOH},
    {"sbadges", "SINNOH'S EIGHT", "Hold all eight SINNOH badges.", AK_FLAGS, FL_SINNOH_BADGES, 8, 1, C_SINNOH},
    {"schampion", "SINNOH CHAMPION", "Beat SINNOH's ELITE FOUR and CYNTHIA.", AK_CLEARED, 3 + 1, 1, 2, C_SINNOH},
    {"slakes", "LAKE GUARDIANS", "Face UXIE, AZELF and MESPRIT in their caverns.", AK_FLAGS, FL_SINNOH_LAKES, 3, 2, C_SINNOH},
    {"sgiratina", "THE TORN WORLD", "Face GIRATINA in TURNBACK CAVE.", AK_FLAG, 0x3D03, 1, 1, C_SINNOH},
    {"screation", "TIME AND SPACE", "Face DIALGA and PALKIA at SPEAR PILLAR.", AK_FLAGS, FL_SINNOH_CREATION, 2, 2, C_SINNOH},
    {"sheatran", "LAVA DOME", "Face HEATRAN in STARK MOUNTAIN.", AK_FLAG, 0x3D04, 1, 1, C_SINNOH},
    {"sregigigas", "THE COLOSSAL", "Face REGIGIGAS under SNOWPOINT TEMPLE.", AK_FLAG, 0x3D05, 1, 1, C_SINNOH},
    {"scresselia", "LUNAR WING", "Face CRESSELIA on FULLMOON ISLAND.", AK_FLAG, 0x3D06, 1, 1, C_SINNOH},
    {"sarceus", "THE ORIGIN", "Face ARCEUS in the HALL OF ORIGIN.", AK_FLAG, 0x3D09, 1, 2, C_SINNOH},
    // POKéMON STADIUM on BATTLE ISLAND (mp_stadium_rewards.c StadiumRewards_Progress)
    {"stwin", "STADIUM DEBUT", "Win a cup at the POK~MON STADIUM.", AK_STADIUM, STADIUM_ACH_ANY, 1, 0, C_STADIUM},
    {"stpika", "PIKA CUP", "Win the PIKA CUP.", AK_STADIUM, STADIUM_ACH_PIKA, 1, 0, C_STADIUM},
    {"stpetit", "PETIT CUP", "Win the PETIT CUP.", AK_STADIUM, STADIUM_ACH_PETIT, 1, 0, C_STADIUM},
    {"stpoke", "POK~ CUP", "Win all four classes of the POK~ CUP.", AK_STADIUM, STADIUM_ACH_POKE, 4, 1, C_STADIUM},
    {"stprime", "PRIME CUP", "Win all four classes of the PRIME CUP.", AK_STADIUM, STADIUM_ACH_PRIME, 4, 1, C_STADIUM},
    {"stcastle", "CASTLE CONQUEROR", "Beat the GYM LEADER CASTLE.", AK_STADIUM, STADIUM_ACH_CASTLE, 1, 1, C_STADIUM},
    {"stmewtwo", "VS. MEWTWO", "Beat MEWTWO at the STADIUM.", AK_STADIUM, STADIUM_ACH_MEWTWO, 1, 2, C_STADIUM},
    {"stchallenge", "CHALLENGE ACCEPTED", "Win a class of the CHALLENGE CUP.", AK_STADIUM, STADIUM_ACH_CHALLENGE, 1, 0, C_STADIUM},
    {"stchallenge4", "CHALLENGE CHAMPION", "Win all four classes of the CHALLENGE CUP.", AK_STADIUM, STADIUM_ACH_CHALLENGE, 4, 1, C_STADIUM},
    {"stround2", "ROUND 2", "Win every cup and class of ROUND 2.", AK_STADIUM, STADIUM_ACH_ROUND2, 12, 2, C_STADIUM},
    {"strental", "RENTAL HERO", "Win a cup with rental POK~MON only.", AK_STADIUM, STADIUM_ACH_RENTALS, 1, 1, C_STADIUM},
    {"strental10", "RENTAL LEGEND", "Win 10 cup classes with rental POK~MON only.", AK_STADIUM, STADIUM_ACH_RENTALS, 10, 2, C_STADIUM},
    {"sthard", "NO HANDICAPS", "Win a cup with every restriction in place.", AK_STADIUM, STADIUM_ACH_HARD, 1, 1, C_STADIUM},
    {"sthard12", "STADIUM LEGEND", "Win every class of ROUND 1 with every restriction.", AK_STADIUM, STADIUM_ACH_HARD_R1, 12, 2, C_STADIUM},
    {"stbattles", "STADIUM REGULAR", "Win 100 battles at the STADIUM.", AK_STADIUM, STADIUM_ACH_BATTLES, 100, 1, C_STADIUM},
    // (the ARCEUS trials were Moemon Quetzal's, removed from KANTO (docs/REGIONS_AUDIT.md case 19): unlisted rows, the
    // user 2026-11-08)
    {"karceus", "ARCEUS TRIALS", "Complete all seventeen ARCEUS trials in KANTO.", AK_FLAGS, FL_ARCEUS_TRIALS, 17, 2, C_GONE},
    {"karceusn", "THE ORIGINAL ONE", "Face the Normal-type ARCEUS in ALTERING CAVE.", AK_FLAG, 0x2E16, 1, 2, C_GONE},
    // (1.182.0: KANTO, JOHTO and SINNOH at par with HOENN, the user 2026-11-08; their tabs also copy HOENN's own that
    // every region has: steps, POKéMON CENTERS, shopping... sTabCopy)
    {"kssanne", "ALL ABOARD", "Get HM01 from the S.S. ANNE's captain.", AK_FLAG, 0x214E, 1, 0, C_KANTO},
    {"kbill", "POK~MANIAC", "Help BILL at his SEA COTTAGE.", AK_FLAG, 0x2177, 1, 0, C_KANTO},
    {"kbike", "BIKE VOUCHER", "Get a BICYCLE in CERULEAN CITY.", AK_FLAG, 0x2141, 1, 0, C_KANTO},
    {"kfossil", "OLD AMBER AGE", "Take a fossil from MT. MOON.", AK_FLAG, 0x2149, 1, 0, C_KANTO},
    {"khideout", "GAME OVER, ROCKET", "Beat GIOVANNI in the ROCKET HIDEOUT.", AK_FLAG, 0x217C, 1, 1, C_KANTO},
    {"kfuji", "LAVENDER'S GHOST", "Rescue MR. FUJI from the POK~MON TOWER.", AK_FLAG, 0x21ED, 1, 1, C_KANTO},
    {"kflute", "WAKE-UP CALL", "Get the POK~ FLUTE.", AK_FLAG, 0x215D, 1, 0, C_KANTO},
    {"ksnorlax", "ROADBLOCKS", "Wake both SNORLAX.", AK_FLAGS, FL_KANTO_SNORLAX, 2, 1, C_KANTO},
    {"ksilph", "HOSTILE TAKEOVER", "Free SILPH CO. from TEAM ROCKET.", AK_FLAG, 0x21BA, 1, 1, C_KANTO},
    {"kmaster", "PRESIDENT'S THANKS", "Get a MASTER BALL from SILPH's president.", AK_FLAG, 0x2156, 1, 1, C_KANTO},
    {"klapras", "A GENTLE GIANT", "Get LAPRAS in SILPH CO.", AK_FLAG, 0x2155, 1, 0, C_KANTO},
    {"keevee", "ROOFTOP SURPRISE", "Get EEVEE in CELADON CITY.", AK_FLAG, 0x2145, 1, 0, C_KANTO},
    {"kdojo", "DOJO PRIZE", "Win a POK~MON at SAFFRON's FIGHTING DOJO.", AK_FLAG, 0x214D, 1, 0, C_KANTO},
    {"kthirsty", "THIRST QUENCHER", "Give the girl on CELADON's roof all three drinks.", AK_FLAGS, FL_KANTO_DRINKS, 3, 0, C_KANTO},
    {"ksafari", "SECRET HOUSE", "Reach the SAFARI ZONE's SECRET HOUSE.", AK_FLAG, 0x2150, 1, 1, C_KANTO},
    {"krods", "KANTO ANGLER", "Get KANTO's OLD ROD, GOOD ROD and SUPER ROD.", AK_FLAGS, FL_KANTO_RODS, 3, 1, C_KANTO},
    {"kseafoam", "CURRENT AFFAIRS", "Stop both currents in the SEAFOAM ISLANDS.", AK_FLAGS, FL_KANTO_SEAFOAM, 2, 1, C_KANTO},
    {"kgems", "THE NETWORK", "Bring CELIO the RUBY and SAPPHIRE.", AK_FLAGS, FL_KANTO_GEMS, 2, 2, C_KANTO},
    {"kwarehouse", "WAREHOUSE RAID", "Beat TEAM ROCKET in FIVE ISLAND's warehouse.", AK_FLAG, 0x2124, 1, 1, C_KANTO},
    {"klostelle", "LOST IN THE WOODS", "Find LOSTELLE in the BERRY FOREST.", AK_FLAG, 0x21EC, 1, 0, C_KANTO},
    {"ktogepi", "LABYRINTH EGG", "Get the EGG in the WATER LABYRINTH.", AK_FLAG, 0x2175, 1, 1, C_KANTO},
    {"ktanoby", "TANOBY'S SECRET", "Open the TANOBY RUINS.", AK_FLAG, 0x2216, 1, 1, C_KANTO},
    {"kdex", "KANTO COMPLETE", "Catch all 151 POK~MON of the KANTO POK~DEX.", AK_NATRANGE, 151, 151, 2, C_KANTO},
    {"jsprout", "SWAYING TOWER", "Beat the ELDER at the top of SPROUT TOWER.", AK_FLAG, 0x1004, 1, 0, C_JOHTO},
    {"jwell", "SLOWPOKE RESCUE", "Drive TEAM ROCKET out of SLOWPOKE WELL.", AK_FLAG, 0x1084, 1, 0, C_JOHTO},
    {"jsudowoodo", "ODD TREE", "Face the odd tree on ROUTE 36.", AK_FLAG, 0x102E, 1, 0, C_JOHTO},
    {"jamphy", "LIGHT OF OLIVINE", "Bring AMPHY the SECRETPOTION.", AK_FLAG, 0x10B7, 1, 1, C_JOHTO},
    {"jgyarados", "THE RED GYARADOS", "Face the red GYARADOS at the LAKE OF RAGE.", AK_FLAG, 0x104C, 1, 1, C_JOHTO},
    {"jhideout", "MAHOGANY SECRETS", "Clear TEAM ROCKET's hideout in MAHOGANY TOWN.", AK_FLAG, 0x104E, 1, 1, C_JOHTO},
    {"jradio", "ON THE AIRWAVES", "Free the RADIO TOWER from TEAM ROCKET.", AK_VARGE, VC_JOHTO_RADIO, 1, 1, C_JOHTO},
    {"jtogepi", "SPIKY SURPRISE", "Hatch the EGG from PROF. ELM's aide.", AK_FLAGS, FL_JOHTO_TOGEPI, 2, 0, C_JOHTO},
    {"jdratini", "DRAGON'S BLESSING", "Earn the DRATINI at the DRAGON SHRINE.", AK_FLAG, 0x105A, 1, 1, C_JOHTO},
    {"jshuckle", "SHUCKIE SITTER", "Look after the MANIA's SHUCKIE.", AK_VARGE, VC_JOHTO_SHUCKIE, 1, 0, C_JOHTO},
    {"jeevee", "BILL'S GIFT", "Get EEVEE from BILL's family.", AK_FLAG, 0x103B, 1, 0, C_JOHTO},
    {"jtyrogue", "KARATE KING", "Beat the KARATE KING in MT. MORTAR.", AK_FLAG, 0x10BA, 1, 1, C_JOHTO},
    {"jmoomoo", "MOOMOO MILK", "Nurse MOOMOO back to health.", AK_VARGE, VC_JOHTO_MOOMOO, 1, 0, C_JOHTO},
    {"jradiocard", "TUNED IN", "Get the RADIO CARD.", AK_FLAG, 0x1064, 1, 0, C_JOHTO},
    {"jbluecard", "BUENA'S PASSWORD", "Get BUENA's BLUE CARD.", AK_FLAG, 0x155D, 1, 0, C_JOHTO},
    {"jmagikarp", "ONE THAT GOT AWAY", "Show off a record MAGIKARP at the LAKE OF RAGE.", AK_FLAG, 0x104A, 1, 0, C_JOHTO},
    {"jitemfinder", "FINDERS KEEPERS", "Get the ITEMFINDER in ECRUTEAK CITY.", AK_FLAG, 0x10A3, 1, 0, C_JOHTO},
    {"jhidden", "SHARP EYES", "Find 30 hidden items in JOHTO.", AK_FLAGRANGE, FR_JOHTO_HIDDEN, 30, 1, C_JOHTO},
    {"jphone", "PHONE BOOK", "Register 10 trainers in your POK~GEAR.", AK_FUNC, FN_JOHTO_PHONE, 10, 1, C_JOHTO},
    {"jkurt", "MADE BY KURT", "Have KURT make a ball from an APRICORN.", AK_EVENT, 0, 1, 0, C_JOHTO},
    {"jtower", "TOWER TAMER", "Win seven battles in a row in JOHTO's BATTLE TOWER.", AK_EVENT, 0, 1, 1, C_JOHTO},
    {"jrival", "LAST STAND", "Beat your rival in VICTORY ROAD.", AK_VARGE, VC_JOHTO_RIVAL, 1, 1, C_JOHTO},
    {"jred", "THE SUMMIT", "Face RED at the top of MT. SILVER.", AK_FLAG, 0x11B0, 1, 2, C_JOHTO},
    {"jdex", "JOHTO COMPLETE", "Catch all 251 POK~MON of the JOHTO POK~DEX.", AK_NATRANGE, 251, 251, 2, C_JOHTO},
    {"spoketch", "POK~TCH", "Get a POK~TCH in JUBILIFE CITY.", AK_FLAG, 0x36D9, 1, 0, C_SINNOH},
    {"swindworks", "WINDWORKS", "Beat COMMANDER MARS at the VALLEY WINDWORKS.", AK_FLAG, TRAINER_FLAG(TRAINER_SINNOH_COMMANDER_MARS_VALLEY_WINDWORKS), 1, 0, C_SINNOH},
    {"seterna", "ETERNA'S BUILDING", "Beat COMMANDER JUPITER in ETERNA CITY.", AK_FLAG, TRAINER_FLAG(TRAINER_SINNOH_COMMANDER_JUPITER_TEAM_GALACTIC_ETERNA_BUILDING), 1, 0, C_SINNOH},
    {"shq", "GALACTIC HQ", "Beat CYRUS in TEAM GALACTIC's HQ.", AK_FLAG, TRAINER_FLAG(TRAINER_SINNOH_GALACTIC_BOSS_CYRUS_GALACTIC_HQ), 1, 1, C_SINNOH},
    {"sdistortion", "DISTORTION WORLD", "Beat CYRUS in the DISTORTION WORLD.", AK_FLAG, TRAINER_FLAG(TRAINER_SINNOH_GALACTIC_BOSS_CYRUS_DISTORTION_WORLD), 1, 2, C_SINNOH},
    {"sbarry", "BEST OF FRIENDS", "Beat BARRY on the way to the POK~MON LEAGUE.", AK_FLAGS, FL_SINNOH_BARRY, 1, 1, C_SINNOH},
    {"sdig", "UNDERGROUND", "Dig up treasure in the Underground.", AK_FLAG, 0x3393, 1, 0, C_SINNOH},
    {"splates", "PLATE COLLECTOR", "Dig up all 16 plates in the Underground.", AK_FUNC, FN_SINNOH_PLATES, 16, 2, C_SINNOH},
    {"scontest", "SUPER STAR", "Win a SUPER CONTEST.", AK_FUNC, FN_SINNOH_CONTESTS, 1, 0, C_SINNOH},
    {"scontest5", "ALL-AROUND STAR", "Win a SUPER CONTEST in all five conditions.", AK_FUNC, FN_SINNOH_CONTESTS, 5, 2, C_SINNOH},
    {"sstudio", "DRESSED UP", "Have a photo taken at JUBILIFE TV's studio.", AK_FUNC, FN_SINNOH_STUDIO, 1, 0, C_SINNOH},
    {"samity", "AMITY STROLL", "Visit HEARTHOME's AMITY SQUARE.", AK_FLAG, 0x339B, 1, 0, C_SINNOH},
    {"srotom", "CHATEAU GHOST", "Catch the ghost in the OLD CHATEAU.", AK_FLAG, 0x3340, 1, 1, C_SINNOH},
    {"sdarkrai", "NEW MOON", "Face DARKRAI on NEWMOON ISLAND.", AK_FLAG, 0x3D07, 1, 1, C_SINNOH},
    {"sshaymin", "GRATITUDE", "Face SHAYMIN at the FLOWER PARADISE.", AK_FLAG, 0x3D08, 1, 1, C_SINNOH},
    {"sbattlezone", "BATTLE ZONE", "Sail to the BATTLE ZONE.", AK_FLAG, 0x36FE, 1, 0, C_SINNOH},
    {"sprint", "PALMER'S PRINT", "Earn the BATTLE TOWER's silver print.", AK_VARGE, VC_SINNOH_SILVER, 1, 1, C_SINNOH},
    {"sprintgold", "TOWER TYCOON PRINT", "Earn the BATTLE TOWER's gold print.", AK_VARGE, VC_SINNOH_GOLD, 1, 2, C_SINNOH},
    {"svilla", "DREAM VILLA", "Furnish the VILLA with all 20 pieces.", AK_FLAGRANGE, FR_SINNOH_VILLA, 20, 2, C_SINNOH},
    {"sdex", "SINNOH COMPLETE", "Show PROF. ROWAN a complete SINNOH POK~DEX.", AK_FLAG, 0x3700, 1, 2, C_SINNOH},
};

#define NUM_ACH ARRAY_COUNT(sAch)

// ---------------------------------------------------------------------------------------------------- each region's own
// An achievement's number: HOENN's are their row in sAch; JOHTO's, KANTO's and SINNOH's copies of a row are
// region * ACH_REGION_STRIDE + the row, so a row added later never moves one (TRAINER CARD titles are numbers).
// A copy counts only what happens in its region: game stats, counters, the POKéDEX kinds caught and seen there,
// levels gained there... (struct AchRegional, kept in the region side file: mp_region.c chunk "ACHR"). HOENN's own
// rows count the rest (what the save counted before 1.182.0 included), so nothing earned so far is lost.
#define ACH_REGION_STRIDE 1024
#define ACH_SPACE (ACH_REGION_STRIDE * REGION_COUNT)
#define NO_COPY 0xFF

#define RB(r) (1 << (r))
#define RB_OTHERS (RB(REGION_JOHTO) | RB(REGION_KANTO) | RB(REGION_SINNOH))
// HOENN's BATTLE / CATCHING / TOGETHER / EXTRAS rows a region has no copy of: what only HOENN has (its BATTLE TOWER,
// outbreaks on its routes, its BERRY BLENDERS, SECRET BASES, BERRY CRUSH, contests with friends, gym rematches)
static const struct { const char *id; u8 regions; } sNoCopy[] = {
    {"tower", RB_OTHERS}, {"tower7", RB_OTHERS}, {"tower50", RB_OTHERS}, {"hoenn", RB_OTHERS}, {"outbreak", RB_OTHERS},
    {"fossil", RB(REGION_JOHTO)}, {"rematch", RB_OTHERS}, {"rematch20", RB_OTHERS}, {"blend", RB_OTHERS},
    {"blend25", RB_OTHERS}, {"base", RB_OTHERS}, {"crush10", RB_OTHERS}, {"linkcontest", RB_OTHERS},
    {"daycare", RB(REGION_SINNOH)},   // (SOLACEON's DAY-CARE isn't the game's)
};
// The HOENN tab's rows a region copies into its own tab: what every region has
static const struct { const char *id; u8 regions; } sTabCopy[] = {
    {"steps10k", RB_OTHERS}, {"steps60k", RB_OTHERS}, {"steps150k", RB_OTHERS},
    {"center10", RB_OTHERS}, {"center100", RB_OTHERS}, {"shop10", RB_OTHERS}, {"shop100", RB_OTHERS},
    {"money100k", RB_OTHERS}, {"money999k", RB_OTHERS}, {"ledges", RB_OTHERS}, {"jackpot", RB_OTHERS},
    {"finder", RB_OTHERS}, {"ribbons", RB_OTHERS}, {"ribbons30", RB_OTHERS},
    {"roulette", RB(REGION_JOHTO)}, {"rain", RB(REGION_JOHTO)},   // (GOLDENROD's roulette; JOHTO's rainy towns)
    {"berries", RB(REGION_JOHTO) | RB(REGION_SINNOH)}, {"berries100", RB(REGION_JOHTO) | RB(REGION_SINNOH)},
    {"safari", RB(REGION_KANTO) | RB(REGION_SINNOH)},                // (the SAFARI ZONE, the GREAT MARSH)
    {"contest1", RB(REGION_SINNOH)}, {"contest10", RB(REGION_SINNOH)}, {"contests20", RB(REGION_SINNOH)},
    {"cut50", RB(REGION_SINNOH)}, {"smash50", RB(REGION_SINNOH)},    // (KANTO's / JOHTO's trees and rocks: their own)
};
// A copy's own words (and list) where HOENN's don't fit; region 0xFF: every region's copy
static const struct { const char *id; u8 region; const char *desc; u16 arg; } sCopyText[] = {
    {"starters", REGION_JOHTO, "Catch CHIKORITA, CYNDAQUIL and TOTODILE.", DL_STARTERS_J},
    {"starters", REGION_KANTO, "Catch BULBASAUR, CHARMANDER and SQUIRTLE.", DL_STARTERS_K},
    {"starters", REGION_SINNOH, "Catch TURTWIG, CHIMCHAR and PIPLUP.", DL_STARTERS_S},
    {"fossil", REGION_KANTO, "Catch OMANYTE, KABUTO or AERODACTYL.", DL_FOSSILS_K},
    {"fossil", REGION_SINNOH, "Catch CRANIDOS or SHIELDON.", DL_FOSSILS_S},
    {"team30", 0xFF, "Raise six POK~MON to Lv 30 or higher.", 0},
    {"team50", 0xFF, "Raise six POK~MON to Lv 50 or higher.", 0},
    {"team100", 0xFF, "Raise six POK~MON to Lv 100.", 0},
    {"friend", 0xFF, "Make a POK~MON love you completely.", 0},
    {"streak7", 0xFF, "Play with friends on 7 different days.", 0},
    {"streak30", 0xFF, "Play with friends on 30 different days.", 0},
    {"streak100", 0xFF, "Play with friends on 100 different days.", 0},
    {"money100k", 0xFF, "Earn $100,000.", 0},
    {"money999k", 0xFF, "Earn $999,999.", 0},
};
static const char sRegionLetter[REGION_COUNT] = {'h', 'j', 'k', 's'};
static const u8 sTeamLevels[] = {30, 50, 100};

#define DEX_WORDS ((NATIONAL_DEX_COUNT + 31) / 32)
#define ACHR_VERSION 1
struct AchRegional
{
    u32 version;
    u32 stat[REGION_COUNT][NUM_USED_GAME_STATS];   // game stats counted there (records: the best there)
    u32 seen[REGION_COUNT][DEX_WORDS];             // national numbers seen / caught there
    u32 caught[REGION_COUNT][DEX_WORDS];
    u32 earned[REGION_COUNT];                      // money earned there
    s32 lastDay[REGION_COUNT];                     // the last day played with friends there
    u16 team[REGION_COUNT][ARRAY_COUNT(sTeamLevels)];   // POKéMON raised there to Lv 30 / 50 / 100
    u16 owned[REGION_COUNT];                       // the most POKéMON owned there (party and that region's PC)
    u16 days[REGION_COUNT];                        // days played with friends there
    u16 friends[REGION_COUNT];                     // POKéMON that came to love the player there
    u8 top[REGION_COUNT];                          // the highest level a POKéMON reached there
};
static struct AchRegional sReg;
static bool8 sRegSeed;          // no record came with the save: HOENN's POKéDEX lists start as the game's
static u8 sCopyCat[REGION_COUNT][NUM_ACH];   // each row's copy's tab in a region (NO_COPY: none)
static bool8 sCopiesBuilt;
static s32 sDoneCount = -1;   // MP_Achieve_DoneCount's (counted again after a grant or another save: the phone asks often)

static u8 GroupOf(u8 cat)   // the group a HOENN tab heads (0xFF: none)
{
    u8 g;
    for (g = 0; g < NUM_ACH_GROUPS; g++)
        if (sGroupCats[g][REGION_HOENN] == cat)
            return g;
    return 0xFF;
}

static void BuildCopies(void)
{
    u16 row;
    u8 r, k, g;
    if (sCopiesBuilt)
        return;
    sCopiesBuilt = TRUE;
    memset(sCopyCat, NO_COPY, sizeof(sCopyCat));
    for (row = 0; row < NUM_ACH; row++)
    {
        g = GroupOf(sAch[row].cat);
        if (g == 0xFF)
            continue;
        for (r = REGION_HOENN + 1; r < REGION_COUNT; r++)
        {
            bool8 copy = g != G_REGION;
            for (k = 0; k < ARRAY_COUNT(sNoCopy); k++)
                if ((sNoCopy[k].regions & RB(r)) && strcmp(sNoCopy[k].id, sAch[row].id) == 0)
                    copy = FALSE;
            for (k = 0; k < ARRAY_COUNT(sTabCopy); k++)
                if ((sTabCopy[k].regions & RB(r)) && strcmp(sTabCopy[k].id, sAch[row].id) == 0)
                    copy = TRUE;
            if (copy)
                sCopyCat[r][row] = sGroupCats[g][r];
        }
    }
}

// An achievement by number, as the menu shows it
struct AchView
{
    const struct Achievement *a;
    u16 row;
    u8 region;     // HOENN: the table's own row
    u8 cat;
    const char *desc;
    u16 arg;
};

static bool8 View(u16 idx, struct AchView *v)
{
    u8 r = idx / ACH_REGION_STRIDE, k;
    u16 row = idx % ACH_REGION_STRIDE;
    BuildCopies();
    if (r >= REGION_COUNT || row >= NUM_ACH || (r != REGION_HOENN && sCopyCat[r][row] == NO_COPY))
        return FALSE;
    v->a = &sAch[row];
    v->row = row;
    v->region = r;
    v->cat = r == REGION_HOENN ? sAch[row].cat : sCopyCat[r][row];
    v->desc = sAch[row].desc;
    v->arg = sAch[row].arg;
    if (r != REGION_HOENN)
        for (k = 0; k < ARRAY_COUNT(sCopyText); k++)
            if ((sCopyText[k].region == r || sCopyText[k].region == 0xFF) && strcmp(sCopyText[k].id, sAch[row].id) == 0)
            {
                v->desc = sCopyText[k].desc;
                if (sCopyText[k].arg)
                    v->arg = sCopyText[k].arg;
            }
    return TRUE;
}

// HOENN's rows that share their count with the other regions' copies (HOENN counts what the others don't)
static bool8 Split(const struct AchView *v)
{
    return v->region == REGION_HOENN && GroupOf(v->a->cat) != 0xFF;
}

static bool8 IsRecordStat(u16 stat)
{
    return stat == GAME_STAT_BATTLE_TOWER_SINGLES_STREAK || stat == GAME_STAT_CONSECUTIVE_ROULETTE_WINS;
}

static u8 Here(void)
{
    u8 r = MP_Region_Current();
    return r < REGION_COUNT ? r : REGION_HOENN;
}

// A game stat moved (overworld.c IncrementGameStat / SetGameStat): counted for the region the player is in
void MP_Achieve_OnStat(u8 index, u32 value, bool8 set)
{
    u8 r;
    if (index >= NUM_USED_GAME_STATS || gSaveBlock1Ptr == NULL)
        return;
    r = Here();
    if (IsRecordStat(index))
    {
        if (set && value > sReg.stat[r][index])
            sReg.stat[r][index] = value;
    }
    else if (!set && sReg.stat[r][index] < 0xFFFFFF)
        sReg.stat[r][index]++;
}

// A POKéDEX flag was set (pokedex.c GetSetPokedexFlag): seen / caught in the region the player is in
void MP_Achieve_OnDex(u16 nationalNum, bool8 caught)
{
    u8 r;
    if (nationalNum == 0 || nationalNum > NATIONAL_DEX_COUNT || gSaveBlock1Ptr == NULL)
        return;
    r = Here();
    nationalNum--;
    sReg.seen[r][nationalNum / 32] |= 1u << (nationalNum % 32);
    if (caught)
        sReg.caught[r][nationalNum / 32] |= 1u << (nationalNum % 32);
}

// The region side file (mp_region.c): its "ACHR" chunk
void MP_Achieve_RegionalReset(void)
{
    memset(&sReg, 0, sizeof(sReg));
    sReg.version = ACHR_VERSION;
    sRegSeed = TRUE;
}

const void *MP_Achieve_RegionalSave(u32 *size)
{
    sReg.version = ACHR_VERSION;
    *size = sizeof(sReg);
    return &sReg;
}

void MP_Achieve_RegionalLoad(const void *data, u32 size)
{
    memset(&sReg, 0, sizeof(sReg));
    memcpy(&sReg, data, size < sizeof(sReg) ? size : sizeof(sReg));
    sReg.version = ACHR_VERSION;
    sRegSeed = FALSE;
}

// A save from before 1.182.0: everything its POKéDEX shows was HOENN's
static void SeedHoenn(void)
{
    u16 n;
    if (!sRegSeed)
        return;
    sRegSeed = FALSE;
    for (n = 1; n <= NATIONAL_DEX_COUNT; n++)
    {
        if (GetSetPokedexFlag(n, FLAG_GET_SEEN))
            sReg.seen[REGION_HOENN][(n - 1) / 32] |= 1u << ((n - 1) % 32);
        if (GetSetPokedexFlag(n, FLAG_GET_CAUGHT))
            sReg.caught[REGION_HOENN][(n - 1) / 32] |= 1u << ((n - 1) % 32);
    }
    MPNet_Log("achievements: HOENN's POKéDEX lists started from the save's");
}

static bool8 InDex(const u32 *bits, u16 nationalNum)
{
    return nationalNum && nationalNum <= NATIONAL_DEX_COUNT && ((bits[(nationalNum - 1) / 32] >> ((nationalNum - 1) % 32)) & 1);
}

static s32 DexCount(const u32 *bits)
{
    s32 n = 0;
    u16 w;
    for (w = 0; w < DEX_WORDS; w++)
    {
        u32 x = bits[w];
        while (x)
        {
            x &= x - 1;
            n++;
        }
    }
    return n;
}

static s32 DexTypes(const u32 *bits)
{
    u32 mask = 0;
    u16 n;
    s32 count = 0, t;
    for (n = 1; n <= NATIONAL_DEX_COUNT; n++)
    {
        u16 species;
        if (!InDex(bits, n))
            continue;
        species = NationalPokedexNumToSpecies(n);
        mask |= 1u << gSpeciesInfo[species].types[0];
        mask |= 1u << gSpeciesInfo[species].types[1];
    }
    mask &= ~(1u << TYPE_MYSTERY);
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        count += (mask >> t) & 1;
    return count;
}

static s32 DexListIn(const u32 *bits, u16 list)
{
    s32 i, n = 0;
    for (i = 0; i < sDexLists[list].count; i++)
        n += InDex(bits, SpeciesToNationalPokedexNum(sDexLists[list].list[i]));
    return n;
}

static void UnlockKey(char *out, const struct AchView *v)
{
    if (v->region == REGION_HOENN)
        snprintf(out, 48, "av:%s", v->a->id);
    else
        snprintf(out, 48, "av:%c.%s", sRegionLetter[v->region], v->a->id);
}

static void CountKey(char *out, const char *name)
{
    snprintf(out, 48, "acnt:%s", name);
}

static void RegionCountKey(char *out, u8 region, const char *name)
{
    snprintf(out, 48, "acnt:%c.%s", sRegionLetter[region], name);
}

static u8 TopLevel(void)
{
    u8 i, top = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            continue;
        if (GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) > top)
            top = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
    }
    return top;
}

static s32 FlagsSet(u16 list)
{
    s32 i, n = 0;
    for (i = 0; i < sFlagLists[list].count; i++)
        n += FlagGet(sFlagLists[list].list[i]) ? 1 : 0;
    return n;
}

static bool8 Caught(u16 species)
{
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT) ? TRUE : FALSE;
}

static s32 DexListCaught(u16 list)
{
    s32 i, n = 0;
    for (i = 0; i < sDexLists[list].count; i++)
        n += Caught(sDexLists[list].list[i]);
    return n;
}

// Every caught species with a national number up to LAST (the KANTO / JOHTO POKéDEX)
static s32 CaughtUpTo(u16 last)
{
    s32 n = 0;
    u16 i;
    for (i = 1; i <= last; i++)
        n += GetSetPokedexFlag(i, FLAG_GET_CAUGHT) ? 1 : 0;
    return n;
}

static bool8 PartyMon(u8 i)
{
    return GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG);
}

static s32 TeamAtLevel(u8 level)
{
    u8 i;
    s32 n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
        if (PartyMon(i) && GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) >= level)
            n++;
    return n;
}

static u8 TeamIndex(u16 level)
{
    u8 k;
    for (k = 0; k < ARRAY_COUNT(sTeamLevels); k++)
        if (sTeamLevels[k] == level)
            return k;
    return 0;
}

static s32 Owned(void)
{
    s32 n = 0, box, pos;
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        n += PartyMon(i);
    // our own boxes (not the shared PC's, shown in the box screen)
    struct PokemonStorage *own = MP_PersonalStorage();
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            if (GetBoxMonData(&own->boxes[box][pos], MON_DATA_SPECIES) != SPECIES_NONE
             && !GetBoxMonData(&own->boxes[box][pos], MON_DATA_IS_EGG))
                n++;
    return n;
}

static s32 FullFriendship(void)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (PartyMon(i) && GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP) >= MAX_FRIENDSHIP)
            return 1;
    return 0;
}

// Counted by a region's own code (AK_FUNC)
static s32 FuncCount(u16 fn)
{
    s32 n = 0;
    u16 i;
    switch (fn)
    {
    case FN_JOHTO_PHONE:
    {
        u8 numbers[64];
        return MP_QzPhone_Numbers(numbers, ARRAY_COUNT(numbers));
    }
    case FN_SINNOH_PLATES:   // (each of the 16 dug once: s3d_underground.c)
        for (i = 0; i < 16; i++)
            n += Sinnoh_UndergroundPlateDug(i) ? 1 : 0;
        return n;
    case FN_SINNOH_CONTESTS:   // (a photo kept for each condition won: s3d_contest.c)
        for (i = 0; i < PHOTO_STUDIO; i++)
            n += Sinnoh_ContestPhoto(i) != NULL;
        return n;
    case FN_SINNOH_STUDIO:
        return Sinnoh_ContestPhoto(PHOTO_STUDIO) != NULL;
    }
    return 0;
}

// What the save shows right now rather than a count: HOENN's rows of these are earned only in HOENN
static bool8 StateKind(u8 kind)
{
    return kind == AK_TEAM || kind == AK_OWNED || kind == AK_FRIEND || kind == AK_LEVEL || kind == AK_MONEY;
}

// "coop25" counts "coop": tiers of one counter share it
static void CounterName(const char *id, char *name)
{
    int n = 0;
    while (id[n] && (id[n] < '0' || id[n] > '9') && n < 23)
    {
        name[n] = id[n];
        n++;
    }
    name[n] = 0;
}

static s32 Clamp0(s64 n)
{
    return n < 0 ? 0 : (s32)n;
}

// Progress toward an achievement (for the menu); unlocked ones read as done
static s32 Progress(const struct AchView *v)
{
    const struct Achievement *a = v->a;
    u8 r = v->region;
    bool8 split = Split(v);
    char key[48], name[24];
    switch (a->kind)
    {
    case AK_FLAGS:   return FlagsSet(v->arg);
    case AK_DEXLIST: return r != REGION_HOENN || split ? DexListIn(sReg.caught[r], v->arg) : DexListCaught(v->arg);
    case AK_HOENN:   return GetHoennPokedexCount(FLAG_GET_CAUGHT);
    case AK_NATRANGE: return CaughtUpTo(v->arg);
    case AK_VARGE:   return VarGet(sVarChecks[v->arg].var) >= sVarChecks[v->arg].value ? 1 : 0;
    case AK_FLAGRANGE:
    {
        s32 n = 0;
        u16 f;
        for (f = 0; f < sFlagRanges[v->arg].count; f++)
            n += FlagGet(sFlagRanges[v->arg].first + f) ? 1 : 0;
        return n;
    }
    case AK_FUNC:    return FuncCount(v->arg);
    case AK_TYPES:   return r != REGION_HOENN || split ? DexTypes(sReg.caught[r]) : 0;
    case AK_TEAM:    return r != REGION_HOENN ? sReg.team[r][TeamIndex(v->arg)] : TeamAtLevel(v->arg);
    case AK_OWNED:   return r != REGION_HOENN ? sReg.owned[r] : Owned();
    case AK_FRIEND:  return r != REGION_HOENN ? (sReg.friends[r] ? 1 : 0) : FullFriendship();
    case AK_MONEY:   return r != REGION_HOENN ? (s32)sReg.earned[r] : (s32)GetMoney(&gSaveBlock1Ptr->money);
    case AK_CLEARED:
        if (v->arg == 0xFF)   // (all four)
            return MP_Region_Cleared(REGION_HOENN) + MP_Region_Cleared(REGION_JOHTO) + MP_Region_Cleared(REGION_KANTO)
                 + MP_Region_Cleared(REGION_SINNOH);
        if (v->arg)
            return MP_Region_Cleared(v->arg - 1) ? 1 : 0;
        return MP_Region_Cleared(REGION_HOENN) + MP_Region_Cleared(REGION_JOHTO) + MP_Region_Cleared(REGION_KANTO);
    case AK_STARTED:   // (arg: one region + 1, 0xFF all four, 0 the first three)
        if (v->arg == 0xFF)
            return ((gRegionState.started >> REGION_HOENN) & 1) + ((gRegionState.started >> REGION_JOHTO) & 1)
                 + ((gRegionState.started >> REGION_KANTO) & 1) + ((gRegionState.started >> REGION_SINNOH) & 1);
        if (v->arg)
            return (gRegionState.started >> (v->arg - 1)) & 1;
        return ((gRegionState.started >> REGION_HOENN) & 1) + ((gRegionState.started >> REGION_JOHTO) & 1)
             + ((gRegionState.started >> REGION_KANTO) & 1);
    case AK_STADIUM: return StadiumRewards_Progress(v->arg);
    case AK_FLAG:   return FlagGet(v->arg) ? 1 : 0;
    case AK_STAT:
        if (r != REGION_HOENN)
            return sReg.stat[r][v->arg];
        if (split && !IsRecordStat(v->arg))
            return Clamp0((s64)GetGameStat(v->arg) - sReg.stat[REGION_JOHTO][v->arg] - sReg.stat[REGION_KANTO][v->arg]
                          - sReg.stat[REGION_SINNOH][v->arg]);
        return GetGameStat(v->arg);
    case AK_CAUGHT: return r != REGION_HOENN || split ? DexCount(sReg.caught[r]) : GetNationalPokedexCount(FLAG_GET_CAUGHT);
    case AK_SEEN:   return r != REGION_HOENN || split ? DexCount(sReg.seen[r]) : GetNationalPokedexCount(FLAG_GET_SEEN);
    case AK_LEVEL:  return r != REGION_HOENN ? sReg.top[r] : TopLevel();
    case AK_STREAK: return r != REGION_HOENN ? sReg.days[r] : MP_Events_StreakDays();
    case AK_COUNT:
        CounterName(a->id, name);
        if (r != REGION_HOENN)
        {
            RegionCountKey(key, r, name);
            return MP_SaveData_Get(key);
        }
        CountKey(key, name);
        if (split)
        {
            s64 n = MP_SaveData_Get(key);
            u8 o;
            for (o = REGION_HOENN + 1; o < REGION_COUNT; o++)
            {
                RegionCountKey(key, o, name);
                n -= MP_SaveData_Get(key);
            }
            return Clamp0(n);
        }
        return MP_SaveData_Get(key);
    }
    return 0;
}

static s16 IndexOfId(const char *id)
{
    u16 i;
    for (i = 0; i < NUM_ACH; i++)
        if (strcmp(sAch[i].id, id) == 0)
            return i;
    return -1;
}

bool8 MP_Achieve_IsDoneById(const char *id)
{
    s16 i = IndexOfId(id);
    return i >= 0 && MP_Achieve_IsDone(i);
}

const char *MP_Achieve_NameById(const char *id)
{
    s16 i = IndexOfId(id);
    return i >= 0 ? sAch[i].name : id;
}

static bool8 DoneView(const struct AchView *v)
{
    char key[48];
    UnlockKey(key, v);
    return MP_SaveData_Get(key) != 0;
}

bool8 MP_Achieve_IsDone(u16 i)
{
    struct AchView v;
    return View(i, &v) && v.cat != C_GONE && DoneView(&v);   // (a retired row: no TRAINER CARD title either)
}

// Unlocks it and gives its pulls; returns the pulls given (0: it was unlocked already)
static s32 GrantQuiet(const struct AchView *v)
{
    char key[48];
    UnlockKey(key, v);
    if (MP_SaveData_Get(key))
        return 0;
    MP_SaveData_Set(key, 1);
    sDoneCount = -1;
    MP_Gatcha_AddPulls(AchPulls(v->a->tier));
    if (v->region != REGION_HOENN)
        MPNet_Log("achievement: %c.%s (+%d pulls)", sRegionLetter[v->region], v->a->id, AchPulls(v->a->tier));
    else
        MPNet_Log("achievement: %s (+%d pulls)", v->a->id, AchPulls(v->a->tier));
    return AchPulls(v->a->tier);
}

// ", BALL COLOR: SAND" for the ball pieces it unlocks (mp_ballunlock.c: HOENN's rows), or ""
static void UnlockText(const struct AchView *v, char *out, u32 size)
{
    u8 kinds[2], indexes[2], n, k;
    u32 len;
    out[0] = 0;
    if (v->region != REGION_HOENN)
        return;
    n = MP_BallUnlock_ForAchievement(v->a->id, kinds, indexes, 2);
    for (k = 0; k < n; k++)
    {
        len = strlen(out);
        if (len + 4 >= size)
            break;
        strcat(out, ", ");
        MP_BallUnlock_PieceName(kinds[k], indexes[k], out + len + 2, size - len - 2);
    }
}

static const char *const sRegionWord[REGION_COUNT] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};

static void Grant(const struct AchView *v)
{
    char msg[128], unlock[64];
    if (GrantQuiet(v))
    {
        UnlockText(v, unlock, sizeof(unlock));
        if (v->region != REGION_HOENN)
            snprintf(msg, sizeof(msg), "%s achievement: %s!\n+%d MOEGATCHA pulls", sRegionWord[v->region], v->a->name, AchPulls(v->a->tier));
        else
            snprintf(msg, sizeof(msg), "Achievement: %s!\n+%d MOEGATCHA pulls%s", v->a->name, AchPulls(v->a->tier), unlock);
        MP_Events_QueueToast(msg);
    }
}

static bool8 CheckView(const struct AchView *v);

// Once per version that adds achievements: everything the save has already done, with one message
#define RETRO_KEY "ach:retro4"
static void GrantEarnedSoFar(void)
{
    u16 n = 0;
    u8 r;
    u16 row;
    s32 pulls = 0, got;
    char msg[96];
    if (MP_SaveData_Get(RETRO_KEY))
        return;
    MP_SaveData_Set(RETRO_KEY, 1);
    for (r = 0; r < REGION_COUNT; r++)
        for (row = 0; row < NUM_ACH; row++)
        {
            struct AchView v;
            if (!View(r * ACH_REGION_STRIDE + row, &v) || !CheckView(&v))
                continue;
            got = GrantQuiet(&v);
            if (got)
            {
                n++;
                pulls += got;
            }
        }
    MPNet_Log("achievements: %d already earned by this save, +%d pulls", n, (int)pulls);
    if (n == 0)
        return;
    snprintf(msg, sizeof(msg), "%d achievement%s from your adventure so far!\n+%d MOEGATCHA pulls", n, n == 1 ? "" : "s", (int)pulls);
    MP_Events_QueueToast(msg);
}

// A counted achievement: name like "coop", "trade", "lend"... (counted for the region the player is in too)
void MP_Achieve_Count(const char *name, s32 n)
{
    char key[48];
    u8 r = Here();
    CountKey(key, name);
    MP_SaveData_Add(key, n);
    if (r != REGION_HOENN)
    {
        RegionCountKey(key, r, name);
        MP_SaveData_Add(key, n);
    }
}

s32 MP_Achieve_GetCount(const char *name)
{
    char key[48];
    CountKey(key, name);
    return MP_SaveData_Get(key);
}

// A feat (AK_EVENT achievements: "flawless", "shiny"...): the region's own where it has one
void MP_Achieve_Unlock(const char *id)
{
    u16 row;
    u8 r = Here();
    struct AchView v;
    BuildCopies();
    for (row = 0; row < NUM_ACH; row++)
        if (strcmp(sAch[row].id, id) == 0)
        {
            if (r != REGION_HOENN && sCopyCat[r][row] != NO_COPY)
                View(r * ACH_REGION_STRIDE + row, &v);
            else
                View(row, &v);
            Grant(&v);
        }
}

static void TopUpOldRewards(void);

static bool8 LookChanged(void)
{
    const struct MpAppearance *a = &gMpConfig.app;
    return a->base != MP_BASE_DEFAULT || a->skin || a->outfit || a->accent || a->hair || a->hasCustom || a->frontSel || a->backSel;
}

static bool8 PartyBallStyled(void)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (PartyMon(i) && MP_BallStyle_OfMon(&gPlayerParty[i]) != 0)
            return TRUE;
    return FALSE;
}

// What changes in the region the player is in: levels and friendship gained, POKé BALL looks, the follower and the
// look changed, money earned, POKéMON owned, days with friends (the save's state is noted first, then followed)
static struct
{
    bool8 init;
    u32 pid[PARTY_SIZE];
    u8 level[PARTY_SIZE];
    u8 friendship[PARTY_SIZE];
    u16 ball[PARTY_SIZE];
    u32 money, follower;
    struct MpAppearance look;
    u16 ownedWait;
} sTrack;

static void TrackRegion(void)
{
    u8 r = Here(), i, j, k;
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    if (!sTrack.init)
    {
        sTrack.init = TRUE;
        sTrack.money = money;
        sTrack.follower = gMpConfig.followerPersonality;
        sTrack.look = gMpConfig.app;
        for (i = 0; i < PARTY_SIZE; i++)
            sTrack.pid[i] = 0;
    }
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 pid;
        u8 level, friendship;
        u16 ball;
        if (!PartyMon(i))
            continue;
        pid = GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY);
        level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
        friendship = GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP);
        ball = MP_BallStyle_OfMon(&gPlayerParty[i]);
        for (j = 0; j < PARTY_SIZE; j++)
            if (sTrack.pid[j] == pid && pid != 0)
                break;
        if (j < PARTY_SIZE)
        {
            if (level > sTrack.level[j])
            {
                if (level > sReg.top[r])
                    sReg.top[r] = level;
                for (k = 0; k < ARRAY_COUNT(sTeamLevels); k++)
                    if (sTrack.level[j] < sTeamLevels[k] && level >= sTeamLevels[k] && sReg.team[r][k] < 0xFFFF)
                        sReg.team[r][k]++;
            }
            if (friendship >= MAX_FRIENDSHIP && sTrack.friendship[j] < MAX_FRIENDSHIP && sReg.friends[r] < 0xFFFF)
                sReg.friends[r]++;
            if (ball != sTrack.ball[j] && ball != 0)
                MP_Achieve_Unlock("ballstyle");
        }
    }
    for (i = 0; i < PARTY_SIZE; i++)
    {
        sTrack.pid[i] = PartyMon(i) ? GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) : 0;
        sTrack.level[i] = sTrack.pid[i] ? GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) : 0;
        sTrack.friendship[i] = sTrack.pid[i] ? GetMonData(&gPlayerParty[i], MON_DATA_FRIENDSHIP) : 0;
        sTrack.ball[i] = sTrack.pid[i] ? MP_BallStyle_OfMon(&gPlayerParty[i]) : 0;
    }
    if (money > sTrack.money)
        sReg.earned[r] += money - sTrack.money;
    sTrack.money = money;
    if (gMpConfig.followerPersonality != sTrack.follower)
    {
        if (gMpConfig.followerPersonality)
            MP_Achieve_Unlock("follower");
        sTrack.follower = gMpConfig.followerPersonality;
    }
    if (memcmp(&sTrack.look, &gMpConfig.app, sizeof(sTrack.look)) != 0)
    {
        MP_Achieve_Unlock("newlook");
        sTrack.look = gMpConfig.app;
    }
    if (r != REGION_HOENN && ++sTrack.ownedWait >= 600)   // (every 10 seconds: the PC is 1,200 POKéMON to look at)
    {
        s32 owned = Owned();
        sTrack.ownedWait = 0;
        if (owned > sReg.owned[r])
            sReg.owned[r] = owned > 0xFFFF ? 0xFFFF : owned;
    }
    if (MP_ConnectedCount() >= 2)
    {
        s32 today = MP_DayNumber(MP_Today());
        if (today != sReg.lastDay[r])
        {
            sReg.lastDay[r] = today;
            if (sReg.days[r] < 0xFFFF)
                sReg.days[r]++;
        }
    }
}

// A new save or the region side file read again: the save's state is noted afresh
void MP_Achieve_RegionalTrackReset(void)
{
    sTrack.init = FALSE;
    sDoneCount = -1;
}

// Is it time to grant this one here?
static bool8 CheckView(const struct AchView *v)
{
    if (v->cat == C_GONE || v->a->kind == AK_EVENT || DoneView(v))
        return FALSE;
    if (Split(v) && StateKind(v->a->kind) && Here() != REGION_HOENN)   // (HOENN's "have six at Lv 50": in HOENN)
        return FALSE;
    return Progress(v) >= (s32)v->a->target;
}

// Every frame on the field: a few achievements checked each frame (HOENN's rows, and the region's copies)
void MP_Achieve_Update(void)
{
    static u16 sNext, sNextCopy;
    u8 k, r;
    struct AchView v;
    if (!MP_IsInOverworld())
        return;
    BuildCopies();
    SeedHoenn();
    TrackRegion();
    TopUpOldRewards();
    GrantEarnedSoFar();
    r = Here();
    if (FlagGet(FLAG_DEFEATED_REGIROCK) && FlagGet(FLAG_DEFEATED_REGICE) && FlagGet(FLAG_DEFEATED_REGISTEEL))
        MP_Achieve_Unlock("regis");
    if (MP_ConnectedCount() >= 4)
        MP_Achieve_Unlock("fullhouse");
    // feats that show in the save too, in HOENN (elsewhere a change made there: TrackRegion): a follower out, a
    // changed look, a styled POKé BALL in the party
    if (r == REGION_HOENN)
    {
        if (gMpConfig.followerPersonality)
            MP_Achieve_Unlock("follower");
        if (LookChanged())
            MP_Achieve_Unlock("newlook");
        if (PartyBallStyled())
            MP_Achieve_Unlock("ballstyle");
    }
    for (k = 0; k < 4; k++, sNext = (sNext + 1) % NUM_ACH)
        if (View(sNext, &v) && CheckView(&v))
            Grant(&v);
    if (r != REGION_HOENN)
        for (k = 0; k < 8; k++, sNextCopy = (sNextCopy + 1) % NUM_ACH)
            if (View(r * ACH_REGION_STRIDE + sNextCopy, &v) && CheckView(&v))
                Grant(&v);
}

// ---- Feats noticed at the end of battles (battle_setup.c, mp_events.c)

static bool8 IsGymLeader(u16 trainerId)
{
    return gTrainers[trainerId].trainerClass == TRAINER_CLASS_LEADER;
}

// A solo trainer battle was won (gEnemyParty and gPlayerParty as they ended)
void MP_Achieve_OnTrainerWin(u16 trainerId)
{
    u8 i, fainted = 0, count = 0, top = 0, ace = 0;
    if (gMpCoopBattle || !IsGymLeader(trainerId))
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        count++;
        if (GetMonData(mon, MON_DATA_HP) == 0)
            fainted++;
        if (GetMonData(mon, MON_DATA_LEVEL) > top)
            top = GetMonData(mon, MON_DATA_LEVEL);
    }
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(&gEnemyParty[i], MON_DATA_LEVEL) > ace)
            ace = GetMonData(&gEnemyParty[i], MON_DATA_LEVEL);
    if (fainted == 0)
        MP_Achieve_Unlock("flawless");
    if (top < ace)
        MP_Achieve_Unlock("underdog");
    if (count == 1)
        MP_Achieve_Unlock("solo");
}

// A wild POKéMON was caught (by us)
void MP_Achieve_OnCatch(struct Pokemon *mon, bool8 outbreak)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 i;
    if (IsMonShiny(mon))
        MP_Achieve_Unlock("shiny");
    for (i = 0; i < ARRAY_COUNT(sDexLegends); i++)
        if (species == sDexLegends[i])
            MP_Achieve_Unlock("legend");
    if (GetMonData(mon, MON_DATA_LEVEL) >= 40)
        MP_Achieve_Unlock("bigcatch");
    if (outbreak)
        MP_Achieve_Unlock("outbreak");
}

// ---- For the menu

// The numbers achievements can have (TRAINER CARD titles go round them; most aren't one: MP_Achieve_IsDone is FALSE)
u16 MP_Achieve_Total(void)
{
    return ACH_SPACE;
}

u8 MP_Achieve_CatCount(void)
{
    return NUM_ACH_CATS;
}

const char *MP_Achieve_CatName(u8 cat)
{
    return cat < NUM_ACH_CATS ? sCatNames[cat] : "";
}

// Every listed achievement in order (HOENN's rows, then each region's copies): the next number after IDX
// (ACH_SPACE to start), ACH_SPACE when past the last
static u16 NextListed(u16 idx)
{
    u8 r;
    u16 row;
    BuildCopies();
    if (idx == ACH_SPACE)
    {
        r = REGION_HOENN;
        row = 0;
    }
    else
    {
        r = idx / ACH_REGION_STRIDE;
        row = idx % ACH_REGION_STRIDE + 1;
    }
    for (; r < REGION_COUNT; r++, row = 0)
        for (; row < NUM_ACH; row++)
            if (r == REGION_HOENN ? sAch[row].cat != C_GONE : sCopyCat[r][row] != NO_COPY)
                return r * ACH_REGION_STRIDE + row;
    return ACH_SPACE;
}

#define FOR_EACH_LISTED(idx) for (idx = NextListed(ACH_SPACE); idx != ACH_SPACE; idx = NextListed(idx))

// How many achievements are listed (the menu's "ALL n/m")
u16 MP_Achieve_ListedTotal(void)
{
    u16 n = 0, idx;
    FOR_EACH_LISTED(idx)
        n++;
    return n;
}

u16 MP_Achieve_DoneCount(void)
{
    u16 n = 0, idx;
    if (sDoneCount >= 0)
        return sDoneCount;
    FOR_EACH_LISTED(idx)
        n += MP_Achieve_IsDone(idx);
    sDoneCount = n;
    return n;
}

// The index of a tab's row-th achievement (the table's rows, then the region copies'), 0 when none
u16 MP_Achieve_IndexIn(u8 cat, u16 row)
{
    u16 idx;
    struct AchView v;
    FOR_EACH_LISTED(idx)
        if (View(idx, &v) && v.cat == cat && row-- == 0)
            return idx;
    return 0;
}

// How many achievements a tab has, and how many of them are done
u16 MP_Achieve_CountIn(u8 cat, u16 *done)
{
    u16 n = 0, d = 0, idx;
    struct AchView v;
    FOR_EACH_LISTED(idx)
    {
        if (!View(idx, &v) || v.cat != cat)
            continue;
        n++;
        if (done)
            d += DoneView(&v);
    }
    if (done)
        *done = d;
    return n;
}

// The menu's tabs, L / R: JOURNEY, a region's own, its BATTLE / CATCHING / TOGETHER / EXTRAS, then the games'.
// A region's tabs show the region the player is in first; SELECT goes round the regions (MP_Achieve_MenuRegion).
static const u8 sMenuTabs[] = {C_JOURNEY, 0x80 | G_REGION, 0x80 | G_BATTLE, 0x80 | G_CATCH, 0x80 | G_TOGETHER,
                               0x80 | G_EXTRAS, C_CARDS, C_YUGIOH, C_MAGIC, C_TRIAD, C_DANCE, C_STADIUM};

static bool8 CatPlace(u8 cat, u8 *group, u8 *region)
{
    u8 g, r;
    for (g = 0; g < NUM_ACH_GROUPS; g++)
        for (r = 0; r < REGION_COUNT; r++)
            if (sGroupCats[g][r] == cat)
            {
                *group = g;
                *region = r;
                return TRUE;
            }
    return FALSE;
}

bool8 MP_Achieve_CatHasRegions(u8 cat)
{
    u8 g, r;
    return CatPlace(cat, &g, &r);
}

// The tab after (DIR 1) or before (-1) CAT
u8 MP_Achieve_MenuNext(u8 cat, s8 dir)
{
    u8 g, r, at = 0, i, t;
    if (!CatPlace(cat, &g, &r))
    {
        g = 0xFF;
        r = Here();
    }
    for (i = 0; i < ARRAY_COUNT(sMenuTabs); i++)
        if (sMenuTabs[i] == (g != 0xFF ? (0x80 | g) : cat))
            at = i;
    t = sMenuTabs[(at + ARRAY_COUNT(sMenuTabs) + dir) % ARRAY_COUNT(sMenuTabs)];
    return (t & 0x80) ? sGroupCats[t & 0x7F][r] : t;
}

// The same tab of the next region (a tab without regions: itself)
u8 MP_Achieve_MenuRegion(u8 cat, s8 dir)
{
    u8 g, r;
    if (!CatPlace(cat, &g, &r))
        return cat;
    return sGroupCats[g][(r + REGION_COUNT + dir) % REGION_COUNT];
}

// name, description, "12/25" or "DONE", reward ("+10 MOEGATCHA pulls")
void MP_Achieve_Info(u16 i, const char **name, const char **desc, char *status, int statusLen, char *reward, int rewardLen)
{
    struct AchView v;
    const struct Achievement *a;
    if (!View(i, &v))
    {
        *name = *desc = "";
        snprintf(status, statusLen, "-");
        snprintf(reward, rewardLen, "-");
        return;
    }
    a = v.a;
    *name = a->name;
    *desc = v.desc;
    if (DoneView(&v))
        snprintf(status, statusLen, "DONE");
    else if (a->target > 1)
    {
        s32 p = Progress(&v);
        if (p > (s32)a->target)
            p = a->target;
        if (a->target >= 10000)   // money, steps: in thousands so the column stays short
            snprintf(status, statusLen, "%dk/%dk", (int)(p / 1000), (int)(a->target / 1000));
        else
            snprintf(status, statusLen, "%d/%d", (int)p, (int)a->target);
    }
    else
        snprintf(status, statusLen, "-");
    {
        char unlock[64];
        UnlockText(&v, unlock, sizeof(unlock));
        snprintf(reward, rewardLen, "+%d MOEGATCHA pulls%s", AchPulls(a->tier), unlock);
    }
}

// Achievements unlocked before the pull rewards went up (they gave $1500, 1 or 2
// pulls) get the difference once, with one toast
static void TopUpOldRewards(void)
{
    static const u8 sOldPulls[] = {0, 1, 2};
    u16 i;
    s32 extra = 0;
    char msg[96];
    if (MP_SaveData_Get("ach:rewards2"))
        return;
    MP_SaveData_Set("ach:rewards2", 1);
    for (i = 0; i < NUM_ACH; i++)
        if (MP_Achieve_IsDone(i))
            extra += AchPulls(sAch[i].tier) - sOldPulls[sAch[i].tier];
    if (extra <= 0)
        return;
    MP_Gatcha_AddPulls(extra);
    snprintf(msg, sizeof(msg), "Achievement rewards went up!\n+%d MOEGATCHA pulls", (int)extra);
    MP_Events_QueueToast(msg);
}

// RARE CANDIES can't raise a POKéMON past the next gym leader's ace (party_menu.c)
u8 MP_RareCandyCap(void)
{
    static const u8 sCap[9] = {15, 20, 27, 33, 36, 41, 47, 51, MAX_LEVEL};
    u8 i, badges = 0;
    for (i = 0; i < 8; i++)
        badges += FlagGet(FLAG_BADGE01_GET + i) ? 1 : 0;
    return sCap[badges];
}
