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

s32 MP_Gatcha_Pulls(void);
void MP_Gatcha_AddPulls(s32 n);
void MP_Events_QueueToast(const char *ascii);
s32 MP_Events_StreakDays(void);

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
};

// The menu's tabs
enum { C_JOURNEY, C_BATTLE, C_CATCH, C_HOENN, C_TOGETHER, C_EXTRAS, NUM_ACH_CATS };
static const char *const sCatNames[NUM_ACH_CATS] = {"JOURNEY", "BATTLE", "CATCHING", "HOENN", "TOGETHER", "EXTRAS"};

enum { FL_HMS, FL_RODS, FL_LANDMARKS, FL_SILVER, FL_GOLD };
enum { DL_STARTERS, DL_FOSSILS, DL_LEGENDS };

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
static const struct { const u16 *list; u8 count; } sFlagLists[] = {
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
};
static const struct { const u16 *list; u8 count; } sDexLists[] = {
    [DL_STARTERS] = {sDexStarters, ARRAY_COUNT(sDexStarters)},
    [DL_FOSSILS] = {sDexFossils, ARRAY_COUNT(sDexFossils)},
    [DL_LEGENDS] = {sDexLegends, ARRAY_COUNT(sDexLegends)},
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
};

#define NUM_ACH ARRAY_COUNT(sAch)

static void UnlockKey(char *out, const struct Achievement *a)
{
    snprintf(out, 48, "av:%s", a->id);
}

static void CountKey(char *out, const char *name)
{
    snprintf(out, 48, "acnt:%s", name);
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

// Types among every caught species (TYPE_MYSTERY isn't one)
static s32 TypesCaught(void)
{
    u32 mask = 0;
    u16 n;
    s32 count = 0, t;
    for (n = 1; n <= NATIONAL_DEX_COUNT; n++)
    {
        u16 species;
        if (!GetSetPokedexFlag(n, FLAG_GET_CAUGHT))
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

static s32 Owned(void)
{
    s32 n = 0, box, pos;
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        n += PartyMon(i);
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            if (GetBoxMonDataAt(box, pos, MON_DATA_SPECIES) != SPECIES_NONE && !GetBoxMonDataAt(box, pos, MON_DATA_IS_EGG))
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

// Progress toward an achievement (for the menu); unlocked ones read as done
static s32 Progress(const struct Achievement *a)
{
    char key[48];
    switch (a->kind)
    {
    case AK_FLAGS:   return FlagsSet(a->arg);
    case AK_DEXLIST: return DexListCaught(a->arg);
    case AK_HOENN:   return GetHoennPokedexCount(FLAG_GET_CAUGHT);
    case AK_TYPES:   return TypesCaught();
    case AK_TEAM:    return TeamAtLevel(a->arg);
    case AK_OWNED:   return Owned();
    case AK_FRIEND:  return FullFriendship();
    case AK_MONEY:   return GetMoney(&gSaveBlock1Ptr->money);
    case AK_FLAG:   return FlagGet(a->arg) ? 1 : 0;
    case AK_STAT:   return GetGameStat(a->arg);
    case AK_CAUGHT: return GetNationalPokedexCount(FLAG_GET_CAUGHT);
    case AK_SEEN:   return GetNationalPokedexCount(FLAG_GET_SEEN);
    case AK_LEVEL:  return TopLevel();
    case AK_STREAK: return MP_Events_StreakDays();
    case AK_COUNT:
        // Tiers of one counter share it ("coop1" and "coop25" both count "coop")
        {
            char name[24];
            int n = 0;
            while (a->id[n] && (a->id[n] < '0' || a->id[n] > '9') && n < 23)
            {
                name[n] = a->id[n];
                n++;
            }
            name[n] = 0;
            CountKey(key, name);
            return MP_SaveData_Get(key);
        }
    }
    return 0;
}

bool8 MP_Achieve_IsDone(u16 i)
{
    char key[48];
    if (i >= NUM_ACH)
        return FALSE;
    UnlockKey(key, &sAch[i]);
    return MP_SaveData_Get(key) != 0;
}

// Unlocks it and gives its pulls; returns the pulls given (0: it was unlocked already)
static s32 GrantQuiet(u16 i)
{
    char key[48];
    const struct Achievement *a = &sAch[i];
    UnlockKey(key, a);
    if (MP_SaveData_Get(key))
        return 0;
    MP_SaveData_Set(key, 1);
    MP_Gatcha_AddPulls(AchPulls(a->tier));
    MPNet_Log("achievement: %s (+%d pulls)", a->id, AchPulls(a->tier));
    return AchPulls(a->tier);
}

static void Grant(u16 i)
{
    char msg[96];
    if (GrantQuiet(i))
    {
        snprintf(msg, sizeof(msg), "Achievement: %s!\n+%d MOEGATCHA pulls", sAch[i].name, AchPulls(sAch[i].tier));
        MP_Events_QueueToast(msg);
    }
}

// Once per version that adds achievements: everything the save has already done, with one message
#define RETRO_KEY "ach:retro3"
static void GrantEarnedSoFar(void)
{
    u16 i, n = 0;
    s32 pulls = 0, got;
    char msg[96];
    if (MP_SaveData_Get(RETRO_KEY))
        return;
    MP_SaveData_Set(RETRO_KEY, 1);
    for (i = 0; i < NUM_ACH; i++)
    {
        if (sAch[i].kind == AK_EVENT || MP_Achieve_IsDone(i) || Progress(&sAch[i]) < (s32)sAch[i].target)
            continue;
        got = GrantQuiet(i);
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

// A counted achievement: name like "coop", "trade", "lend"...
void MP_Achieve_Count(const char *name, s32 n)
{
    char key[48];
    CountKey(key, name);
    MP_SaveData_Add(key, n);
}

// A feat (AK_EVENT achievements: "flawless", "shiny"...)
void MP_Achieve_Unlock(const char *id)
{
    u16 i;
    for (i = 0; i < NUM_ACH; i++)
        if (strcmp(sAch[i].id, id) == 0)
            Grant(i);
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

// Every frame on the field: a few achievements checked each frame
void MP_Achieve_Update(void)
{
    static u16 sNext;
    u8 k;
    if (!MP_IsInOverworld())
        return;
    TopUpOldRewards();
    GrantEarnedSoFar();
    if (FlagGet(FLAG_DEFEATED_REGIROCK) && FlagGet(FLAG_DEFEATED_REGICE) && FlagGet(FLAG_DEFEATED_REGISTEEL))
        MP_Achieve_Unlock("regis");
    if (MP_ConnectedCount() >= 4)
        MP_Achieve_Unlock("fullhouse");
    // feats that show in the save too: a follower out, a changed look, a styled POKé BALL in the party
    if (gMpConfig.followerPersonality)
        MP_Achieve_Unlock("follower");
    if (LookChanged())
        MP_Achieve_Unlock("newlook");
    if (PartyBallStyled())
        MP_Achieve_Unlock("ballstyle");
    for (k = 0; k < 4; k++, sNext = (sNext + 1) % NUM_ACH)
    {
        const struct Achievement *a = &sAch[sNext];
        if (a->kind == AK_EVENT || MP_Achieve_IsDone(sNext))
            continue;
        if (Progress(a) >= (s32)a->target)
            Grant(sNext);
    }
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

u16 MP_Achieve_Total(void)
{
    return NUM_ACH;
}

u8 MP_Achieve_CatCount(void)
{
    return NUM_ACH_CATS;
}

const char *MP_Achieve_CatName(u8 cat)
{
    return cat < NUM_ACH_CATS ? sCatNames[cat] : "";
}

// How many achievements a tab has, and how many of them are done
u16 MP_Achieve_CountIn(u8 cat, u16 *done)
{
    u16 i, n = 0, d = 0;
    for (i = 0; i < NUM_ACH; i++)
    {
        if (sAch[i].cat != cat)
            continue;
        n++;
        d += MP_Achieve_IsDone(i);
    }
    if (done)
        *done = d;
    return n;
}

// The index of a tab's row-th achievement (table order)
u16 MP_Achieve_IndexIn(u8 cat, u16 row)
{
    u16 i;
    for (i = 0; i < NUM_ACH; i++)
        if (sAch[i].cat == cat && row-- == 0)
            return i;
    return 0;
}

u16 MP_Achieve_DoneCount(void)
{
    u16 i, n = 0;
    for (i = 0; i < NUM_ACH; i++)
        n += MP_Achieve_IsDone(i);
    return n;
}

// name, description, "12/25" or "DONE", reward ("+10 MOEGATCHA pulls")
void MP_Achieve_Info(u16 i, const char **name, const char **desc, char *status, int statusLen, char *reward, int rewardLen)
{
    const struct Achievement *a = &sAch[i];
    *name = a->name;
    *desc = a->desc;
    if (MP_Achieve_IsDone(i))
        snprintf(status, statusLen, "DONE");
    else if (a->target > 1)
    {
        s32 p = Progress(a);
        if (p > (s32)a->target)
            p = a->target;
        if (a->target >= 10000)   // money, steps: in thousands so the column stays short
            snprintf(status, statusLen, "%dk/%dk", (int)(p / 1000), (int)(a->target / 1000));
        else
            snprintf(status, statusLen, "%d/%d", (int)p, (int)a->target);
    }
    else
        snprintf(status, statusLen, "-");
    snprintf(reward, rewardLen, "+%d MOEGATCHA pulls", AchPulls(a->tier));
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
