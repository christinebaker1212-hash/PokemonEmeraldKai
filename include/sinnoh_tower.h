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
u8 Sinnoh_Frontier_FillParty(struct Pokemon *party, u16 frontierTrainer, u8 count, const u16 *avoidSpecies,
                             const u16 *avoidItems, u8 avoidCount, u8 level);

#endif // GUARD_SINNOH_TOWER_H
