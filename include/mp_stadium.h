#ifndef GUARD_MP_STADIUM_H
#define GUARD_MP_STADIUM_H

// POKéMON STADIUM on BATTLE ISLAND (docs/STADIUM.md, docs/stadium/RULES.md): the data from the user's ROM
// (tools/stadium/gen_data.py -> src/data/mp_stadium_data.h) and its conversion to the game's POKéMON.

struct Pokemon;

// One POKéMON as STADIUM stores it (Generation I): DVs and stat EXP, not IVs and EVs
struct StadiumMon
{
    u8 species;      // dex number = the game's species (1-151)
    u8 level;
    u8 moves[4];     // Generation I move numbers = the game's
    u16 dvs;         // ATK << 12 | DEF << 8 | SPD << 4 | SPC; HP from their low bits
    u16 statExp[5];  // HP, ATK, DEF, SPD, SPC
    u16 otId;
    u8 nickname[11];
    u8 otName[11];
};

struct StadiumTrainer
{
    u8 name[12];
    u8 pic;          // TRAINER_PIC_* (0: MEWTWO, drawn from its own picture)
    u8 ai;           // STADIUM's own byte (+0x35 of the trainer header)
    u16 firstMon;
    u8 count;
};

struct StadiumRange
{
    u16 first;
    u16 count;
};

#define STADIUM_ROUNDS 2

// The CHALLENGE CUP (STADIUM 2's: tools/stadium/gen_challenge.py -> src/data/mp_stadium_cc.h)
struct StadiumCcMon
{
    u16 species;
    u8 split;       // the nickname's case changes here ("BULBAsaur": 5)
    u16 bst;
    u16 rate;       // Bulbapedia's appearance rate x 10
    u8 types[2];
};

struct StadiumCcClass
{
    u8 level;
    u16 bstTarget;
    u16 first, count;   // its pool in sStadiumCcPool
};

struct StadiumCcTrainer
{
    u8 className[12];
    u8 name[12];
    u8 pic;
    u8 tmpl[6];         // a type for each slot, TYPE_NONE = a type not on the team yet
    u16 moves[7];       // the moves they like (the third move when it can be learned)
    u16 statExp[2];     // Round 1 / 2, every stat
    u16 speedExp[2];    // a SPEED of their own (0: the same)
};

#define STADIUM_CC_CLASSES 4
#define STADIUM_CC_PLAYER 0xFF

// Six random POKéMON for the CHALLENGE CUP's class / round: the player's (trainer STADIUM_CC_PLAYER) or a trainer's
void StadiumCc_MakeTeam(struct Pokemon *party, u8 cls, u8 round, u8 trainer);

// The game's IVs, EVs and nature for a STADIUM POKéMON (the user's rules: DVs to their closest IVs, stat EXP to EVs,
// a nature that raises the attacking stat its moves use)
void Stadium_Convert(const struct StadiumMon *sm, u8 ivs[6], u8 evs[6], u8 *nature);
// Builds it: species, level, moves, IVs, EVs, nature, nickname, OT name and ID as STADIUM has them
void Stadium_BuildMon(struct Pokemon *mon, const struct StadiumMon *sm);

const struct StadiumTrainer *Stadium_Trainer(u8 round, u8 division, u8 index);
u8 Stadium_TrainerCount(u8 round, u8 division);
const struct StadiumMon *Stadium_TrainerMon(const struct StadiumTrainer *t, u8 i);
u16 Stadium_RentalCount(u8 round, u8 list);
const struct StadiumMon *Stadium_Rental(u8 round, u8 list, u16 i);

#endif // GUARD_MP_STADIUM_H
