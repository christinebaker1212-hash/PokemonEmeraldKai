#ifndef GUARD_SINNOH_TOWER_H
#define GUARD_SINNOH_TOWER_H

// SINNOH's BATTLE TOWER and the other facilities' opponents: Platinum's frontier trainers and their sets
// (src/sinnoh/s3d_tower.c, data src/data/sinnoh_frontier.h from tools/sinnoh/gen_frontier.py, docs/SINNOH_TOWER_PORT.md)

#include "constants/trainers.h"

struct SinnohFrontierSet
{
    u16 species;
    u16 moves[4];
    u16 item;
    u8 nature;
    u8 evFlags;   // bit 0 HP, 1 ATTACK, 2 DEFENSE, 3 SPEED, 4 SP. ATK, 5 SP. DEF (Platinum's order)
};

struct SinnohFrontierTrainer
{
    u8 trainerClass;
    u8 trainerPic;     // the game's stand-in picture
    u8 sinnohPic;      // Platinum's own (SINNOH_PIC_*, the graphics pack)
    u8 female;
    u16 firstSet;      // in sSinnohFrontierSetLists
    u16 setCount;
    u16 objGfx;        // its sprite in the battle room (Platinum's BattleTower_GetObjectIDFromTrainerClass)
};

// The battle's opponents as these trainers: gTrainerBattleOpponent_A / _B (battle_tower.c and frontier_util.c ask
// s3d_tower.c for their names, classes, pictures and lines)
#define TRAINER_SINNOH_FRONTIER_A (TRAINER_STEVEN_PARTNER + 2)
#define TRAINER_SINNOH_FRONTIER_B (TRAINER_STEVEN_PARTNER + 3)
#define IS_SINNOH_FRONTIER_TRAINER(id) ((id) == TRAINER_SINNOH_FRONTIER_A || (id) == TRAINER_SINNOH_FRONTIER_B)

enum { SINNOH_FRONTIER_SPEECH_INTRO, SINNOH_FRONTIER_SPEECH_WIN, SINNOH_FRONTIER_SPEECH_DEFEAT };

void Sinnoh_Frontier_SetOpponent(u16 battleTrainerId, u16 frontierTrainer);
u16 Sinnoh_Frontier_OpponentOf(u16 battleTrainerId);
u8 Sinnoh_Frontier_Class(u16 battleTrainerId);
u8 Sinnoh_Frontier_Pic(u16 battleTrainerId);
u8 Sinnoh_Frontier_SinnohPic(u16 battleTrainerId);
void Sinnoh_Frontier_Name(u8 *dst, u16 battleTrainerId);
void Sinnoh_Frontier_Speech(u16 battleTrainerId, u8 kind);   // into gStringVar4
u16 Sinnoh_Frontier_Brain(u8 facility, u8 gold);   // 0 HALL, 1 CASTLE, 2 ARCADE: Platinum's Frontier Brain
u8 Sinnoh_Frontier_FillParty(struct Pokemon *party, u16 frontierTrainer, u8 count, const u16 *avoidSpecies,
                             const u16 *avoidItems, u8 avoidCount, u8 level);


// Platinum's BATTLE TOWER (its BattleTower struct and BT_FUNC_* functions): `sinnoh SINNOH_TOWER, FUNC, ARG, DESTVAR`
// (FUNC Platinum's BT_FUNC_* number or SINNOH_TOWER_* below; ARG a value or a variable; DESTVAR a variable). TRUE: the
// script waits (the party menu, a battle: their end starts it again)
enum
{
    SINNOH_TOWER_INIT = 200,        // InitBattleTower 0, ARG (the challenge mode)
    SINNOH_TOWER_INIT_RESUME,       // InitBattleTower 1: the challenge saved by REST
    SINNOH_TOWER_FREE,              // FreeBattleTower / FreeTowerStruct
    SINNOH_TOWER_INTRO,             // gStringVar4 <- opponent ARG's line before battle (PrintBattleTowerIntroMessage)
    SINNOH_TOWER_BATTLE,            // the battle (BattleTower_StartBattle / BT_FUNC_UNK_42)
    SINNOH_TOWER_WON,               // DESTVAR <- the battle was won (BattleTower_CheckWonBattle / CheckWonBattle)
    SINNOH_TOWER_CHOOSE,            // the party menu for the challenge's POKéMON (BT_FUNC_UNK_30)
    SINNOH_TOWER_SAVE,              // the game saved as it stands, no question asked (TrySaveGame, FrontierScrCmd_72)
    SINNOH_TOWER_PARTNER_SPECIES = 210, // (+ n) DESTVAR <- BATTLE SALON partner ARG's POKéMON n's kind
    SINNOH_TOWER_PARTNER_MOVE = 212,    // (+ n) ... its first move
    SINNOH_TOWER_BUFFER_SPECIES = 214,  // the name of kind ARG into text slot DESTVAR (0-5)
    SINNOH_TOWER_BUFFER_MOVE,           // the name of move ARG into text slot DESTVAR (0-5)
    SINNOH_TOWER_JUDGE_TOTAL,           // the STATS JUDGE: DESTVAR <- party slot ARG's IVs' total
    SINNOH_TOWER_JUDGE_STAT,            // ... its best stat's line (Platinum's battle tower message 122 + the stat)
    SINNOH_TOWER_JUDGE_BEST,            // ... that stat's IV
    SINNOH_TOWER_RANKINGS = 220,        // JUBILIFE TV's rankings machine ARG (s3d_rankings.c; Platinum's StartRankingsMachine)
};
bool8 Sinnoh_TowerCall(u16 func, u16 arg, u16 destVar);
bool8 Sinnoh_Tower_Choosing(void);   // (party_menu.c: Platinum's banned list, any level, its own duplicate checks)
bool8 Sinnoh_Tower_Banned(u16 species);
const void *Sinnoh_TowerData(u32 *size);   // "<save>.region" chunk TOWR (mp_region.c)
void Sinnoh_TowerLoad(const void *data, u32 size);
void Sinnoh_TowerReset(void);

#endif // GUARD_SINNOH_TOWER_H
