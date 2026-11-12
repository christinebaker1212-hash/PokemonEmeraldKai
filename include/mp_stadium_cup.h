#ifndef GUARD_MP_STADIUM_CUP_H
#define GUARD_MP_STADIUM_CUP_H

#include <stddef.h>

// POKéMON STADIUM's cups as the player meets them (src/mp/mp_stadium_cup.c; docs/stadium/RULES.md)
enum
{
    STADIUM_CUP_PIKA,
    STADIUM_CUP_PETIT,
    STADIUM_CUP_POKE,
    STADIUM_CUP_PRIME,
    STADIUM_CUP_CHALLENGE,
    STADIUM_CUP_CASTLE,
    STADIUM_CUP_MEWTWO,
    STADIUM_CUP_COUNT
};

// The user's switches (docs/STADIUM.md): set = on
#define STADIUM_OPT_LEVEL_CAP  (1 << 0)   // entries adjusted to the cup's level
#define STADIUM_OPT_SPECIES    (1 << 1)   // the first 151 and their moves only
#define STADIUM_OPT_HELD_ITEMS (1 << 2)   // held items come along
#define STADIUM_OPT_RENTALS    (1 << 3)   // rentals only (and so no LEVEL CAP: rentals are made for the cup; the user)
#define STADIUM_OPT_DEFAULT    (STADIUM_OPT_LEVEL_CAP | STADIUM_OPT_SPECIES)

// The hard side of each (the rewards grow with them)
#define STADIUM_HARD_LEVEL   (1 << 0)
#define STADIUM_HARD_SPECIES (1 << 1)
#define STADIUM_HARD_ITEMS   (1 << 2)
#define STADIUM_HARD_RENTALS (1 << 3)
#define STADIUM_HARD_ALL     0xF

struct StadiumCupInfo
{
    const char *name;
    u8 tiers;
    u8 levelMin, levelMax;
    u16 levelSum;          // the three chosen's levels at most (0: no total)
    bool8 petit;           // height, weight and the lowest stage
    bool8 allowsMewtwo;    // MEWTWO and MEW (and, SPECIES off, the restricted legendaries)
    u8 division[4];        // STADIUM_* of each class (mp_stadium_data.h)
};

#define STADIUM_TEAM_SIZE 6
#define STADIUM_PICK 3

// Where a registered POKéMON came from
enum { STADIUM_SRC_NONE, STADIUM_SRC_PARTY, STADIUM_SRC_BOX, STADIUM_SRC_SHARED, STADIUM_SRC_GLOBAL, STADIUM_SRC_RENTAL };

struct StadiumTeamSlot
{
    u8 source;             // STADIUM_SRC_*
    u8 pad;
    u16 rental;            // its index in the cup's rental list (STADIUM_SRC_RENTAL)
    struct BoxPokemon mon; // a copy (the PCs' own stay where they are)
};

struct StadiumTeam
{
    u8 count;
    u8 pad[3];
    struct StadiumTeamSlot slot[STADIUM_TEAM_SIZE];
};

struct StadiumSave
{
    u16 cleared[2][STADIUM_CUP_COUNT][4];   // per round / cup / class: bit h = won with hardness h (STADIUM_HARD_*)
    u8 options[STADIUM_CUP_COUNT];          // the switches last used, per cup (0 = never set: the default)
    u8 lastCup;
    u8 pad;
    u16 battlesWon;
    u16 cupsWon;
    struct StadiumTeam team[STADIUM_CUP_COUNT];   // the registered team of each cup
};

struct StadiumRun
{
    bool8 active;
    u8 round, cup, tier, options;
    u8 trainer;            // the next opponent (0..)
    u8 continues;
    u8 rival;              // the castle's RIVAL's team (0..6)
    bool8 rentalsOnly;
    u8 picks[STADIUM_PICK];   // the team's slots chosen for this battle, in order
    u8 foePicks[STADIUM_PICK];
    bool8 lostNone;        // this battle won without a faint (a continue)
    u8 stage;              // the castle's gym (0-7) or STADIUM_CASTLE_E4 (the ELITE FOUR and the RIVAL)
};

// The GYM LEADER CASTLE (docs/stadium/RULES.md): eight gyms of three trainers and the leader, each its own run won
// for a badge (a new team allowed at each), then the ELITE FOUR and the RIVAL in one run once all eight are won.
// The badges: bits 0-7 of cleared[round][STADIUM_CUP_CASTLE][3] (the castle has one class, so its other three are free)
#define STADIUM_CASTLE_GYMS 8
#define STADIUM_CASTLE_E4   8
#define STADIUM_CASTLE_STAGES 9
#define STADIUM_CASTLE_BADGE_SLOT 3

// StadiumCup_Advance: on to the next trainer, a castle gym won (its badge), the cup won (its rewards)
enum { STADIUM_NEXT, STADIUM_STAGE_WON, STADIUM_CUP_WON };

const struct StadiumCupInfo *StadiumCup_Info(u8 cup);
u16 StadiumCup_RentalCount(u8 round, u8 cup);
const struct StadiumMon *StadiumCup_Rental(u8 round, u8 cup, u16 i);
u8 StadiumCup_CapLevel(u8 cup, u8 tier);
const char *StadiumCup_Refuse(u8 cup, u8 tier, u8 options, u16 species, u8 level, const u16 *moves, u16 item);
struct StadiumSave *StadiumCup_Save(void);
void StadiumCup_SaveNow(void);
bool8 StadiumCup_Cleared(u8 round, u8 cup, u8 tier);
bool8 StadiumCup_Open(u8 round, u8 cup, u8 tier);
u16 StadiumCup_WinsTotal(void);
void StadiumRecords_News(char *out, size_t size);
struct StadiumRun *StadiumCup_Run(void);
u8 StadiumCup_TrainerCount(u8 round, u8 cup, u8 tier);
const struct StadiumTrainer *StadiumCup_CastleTrainer(u8 round, u8 n, u8 rival);
u8 StadiumCup_PickRival(u8 round, const struct Pokemon *team, u8 count);
void StadiumCup_Entry(struct Pokemon *out, u8 slot);
u8 StadiumCup_EntryCount(void);
const struct StadiumTrainer *StadiumCup_Foe(void);
struct Pokemon *StadiumCup_FoeTeam(u8 *count);
// The last battle's three on each side (0 the player, 1 the opponent) and their HP at its end
struct StadiumFought { u16 species, hp, maxHp; u32 personality, otId; };
const struct StadiumFought *StadiumCup_Fought(u8 side);
void StadiumCup_PrepareFoe(void);
void StadiumCup_FoePicks(void);
void StadiumCup_Begin(u8 round, u8 cup, u8 tier, u8 options);
u8 StadiumCup_Advance(void);
void StadiumCastle_SetStage(u8 stage);   // the castle's next run (before StadiumCup_Begin)
u8 StadiumCastle_Stage(void);
u8 StadiumCastle_Badges(u8 round);       // bit g: gym g won
u8 StadiumCastle_BadgeCount(u8 round);
bool8 StadiumCastle_StageOpen(u8 round, u8 stage);
u8 StadiumCastle_Index(void);            // the battle now in the castle's order: gym * 4 + n, the ELITE FOUR 32-35, the RIVAL 36
const char *StadiumCastle_StageName(u8 stage);   // "PEWTER GYM" .. "ELITE FOUR"
const char *StadiumCastle_BadgeName(u8 gym);     // "BOULDER BADGE" ..
bool8 StadiumCup_UseContinue(void);
void StadiumCup_StartBattle(void (*after)(bool8 won));
void MP_Stadium_FieldStaysBlack(void);   // the field back from a battle / the SUMMARY stays black (mp_stadium_ui.c)

// The rewards (src/mp/mp_stadium_rewards.c): what the last cup won gave, for the result screen
struct StadiumPrize
{
    bool8 first;           // the class's first win at this hardness (else money and a RARE CANDY)
    bool8 shiny;
    u8 level, perfect;     // the prize POKéMON's level and perfect IVs
    u16 species;           // SPECIES_NONE: none
    u32 personality, otId;
    u8 count;
    char line[8][40];      // "$40000", "RARE CANDY x8", "STADIUM RIBBON x6"...
};
void StadiumRewards_CupWon(u8 round, u8 cup, u8 tier, u8 hardness, bool8 first);
const struct StadiumPrize *StadiumRewards_Last(void);

// The achievements' progress (mp_achieve.c AK_STADIUM)
enum
{
    STADIUM_ACH_ANY, STADIUM_ACH_PIKA, STADIUM_ACH_PETIT, STADIUM_ACH_POKE, STADIUM_ACH_PRIME, STADIUM_ACH_CASTLE,
    STADIUM_ACH_MEWTWO, STADIUM_ACH_CHALLENGE, STADIUM_ACH_ROUND2, STADIUM_ACH_RENTALS, STADIUM_ACH_HARD,
    STADIUM_ACH_HARD_R1, STADIUM_ACH_BATTLES,
};
s32 StadiumRewards_Progress(u16 what);

#endif // GUARD_MP_STADIUM_CUP_H
