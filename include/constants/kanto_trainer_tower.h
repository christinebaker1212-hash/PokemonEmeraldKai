#ifndef GUARD_CONSTANTS_KANTO_TRAINER_TOWER_H
#define GUARD_CONSTANTS_KANTO_TRAINER_TOWER_H

// KANTO's TRAINER TOWER as FireRed's (pokefirered include/constants/trainer_tower.h; src/mp/mp_trainer_tower.c,
// data/scripts/kanto_trainer_tower.inc; audit case 10). FireRed's CHALLENGE_STATUS_* are TT_STATUS_* here (the
// BATTLE FRONTIER has its own).

#define CHALLENGE_TYPE_SINGLE     0
#define CHALLENGE_TYPE_DOUBLE     1
#define CHALLENGE_TYPE_KNOCKOUT   2
#define CHALLENGE_TYPE_MIXED      3
// Mixed challenges use one of the above 3 for each floor

#define TT_STATUS_LOST     0
#define TT_STATUS_UNK      1  // Never set, possibly e-Card related
#define TT_STATUS_NORMAL   2  // Not started or ongoing

#define TTPRIZE_HP_UP               0
#define TTPRIZE_PROTEIN             1
#define TTPRIZE_IRON                2
#define TTPRIZE_CARBOS              3
#define TTPRIZE_CALCIUM             4
#define TTPRIZE_ZINC                5
#define TTPRIZE_BRIGHT_POWDER       6
#define TTPRIZE_WHITE_HERB          7
#define TTPRIZE_MENTAL_HERB         8
#define TTPRIZE_CHOICE_BAND         9
#define TTPRIZE_KINGS_ROCK         10
#define TTPRIZE_SCOPE_LENS         11
#define TTPRIZE_METAL_COAT         12
#define TTPRIZE_DRAGON_SCALE       13
#define TTPRIZE_UP_GRADE           14

#define MAX_TRAINER_TOWER_FLOORS 8

#define MAX_TRAINERS_PER_FLOOR  3
#define NUM_TOWER_CHALLENGE_TYPES 4

#define TRAINER_TOWER_FUNC_INIT_FLOOR               0
#define TRAINER_TOWER_FUNC_GET_SPEECH               1
#define TRAINER_TOWER_FUNC_DO_BATTLE                2
#define TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE       3
#define TRAINER_TOWER_FUNC_CLEARED_FLOOR            4
#define TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED        5
#define TRAINER_TOWER_FUNC_START_CHALLENGE          6
#define TRAINER_TOWER_FUNC_GET_OWNER_STATE          7
#define TRAINER_TOWER_FUNC_GIVE_PRIZE               8
#define TRAINER_TOWER_FUNC_CHECK_FINAL_TIME         9
#define TRAINER_TOWER_FUNC_RESUME_TIMER             10
#define TRAINER_TOWER_FUNC_SET_LOST                 11
#define TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS     12
#define TRAINER_TOWER_FUNC_GET_TIME                 13
#define TRAINER_TOWER_FUNC_SHOW_RESULTS             14
#define TRAINER_TOWER_FUNC_CLOSE_RESULTS            15
#define TRAINER_TOWER_FUNC_CHECK_DOUBLES            16
#define TRAINER_TOWER_FUNC_GET_NUM_FLOORS           17
#define TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER   18
#define TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC          19
#define TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE       20

#define TRAINER_TOWER_TEXT_INTRO        2
#define TRAINER_TOWER_TEXT_PLAYER_LOST  3
#define TRAINER_TOWER_TEXT_PLAYER_WON   4
#define TRAINER_TOWER_TEXT_AFTER        5

#define TRAINER_TOWER_MAX_TIME 215999 // 60 * 60 * 60 - 1

// the save's vars (FireRed keeps these in its SaveBlock1 trainerTower[]): state bits, floors cleared, the timer
// (two vars) and the four best times (two vars each)
#define VAR_KANTO_TT_STATE      0x515C
#define VAR_KANTO_TT_FLOORS     0x515D
#define VAR_KANTO_TT_TIMER      0x515E   // and 0x515F
#define VAR_KANTO_TT_BEST       0x5160   // to 0x5167
#define VAR_MAP_SCENE_TRAINER_TOWER 0x5132   // Quetzal's VAR_QZ_0x5132, FireRed's VAR_MAP_SCENE_TRAINER_TOWER

#endif //GUARD_CONSTANTS_KANTO_TRAINER_TOWER_H
