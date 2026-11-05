#ifndef GUARD_CONSTANTS_JOHTO_BATTLE_TOWER_H
#define GUARD_CONSTANTS_JOHTO_BATTLE_TOWER_H

// JOHTO's BATTLE TOWER as Crystal's (src/mp/mp_johto_tower.c, data/scripts/johto_battle_tower.inc; audit case 37)

// CallJohtoBattleTowerFunc's VAR_0x8004
#define JBT_FUNC_GET_STATE         0
#define JBT_FUNC_SET_STATE         1   // VAR_0x8005: JBT_*
#define JBT_FUNC_EXPLANATION_READ  2
#define JBT_FUNC_SET_EXPLAINED     3
#define JBT_FUNC_CHECK_RULES       4   // VAR_RESULT: JBT_RULE_* bits
#define JBT_FUNC_LEVEL_MENU        5   // VAR_RESULT: the level group, 0 CANCEL
#define JBT_FUNC_LEVEL_CHECK       6   // VAR_0x8005: the group; VAR_RESULT 0 / 1 level / 2 uber
#define JBT_FUNC_START             7   // VAR_0x8005: the group
#define JBT_FUNC_LOAD_OPPONENT     8
#define JBT_FUNC_GET_TEXT          9   // VAR_0x8005: 1 greeting, 2 the player lost, 3 the player won
#define JBT_FUNC_DO_BATTLE         10
#define JBT_FUNC_GET_BEATEN        11
#define JBT_FUNC_GIVE_REWARD       12
#define JBT_FUNC_SAVE_AND_QUIT     13
#define JBT_FUNC_GET_GROUP         14
#define JBT_FUNC_QUICK_SAVE        15   // Crystal's TryQuickSave

// Crystal's BATTLETOWER_* challenge states
#define JBT_NO_CHALLENGE           0
#define JBT_SAVED_AND_LEFT         1
#define JBT_CHALLENGE_IN_PROGRESS  2
#define JBT_WON_CHALLENGE          3
#define JBT_RECEIVED_REWARD        4

// Crystal's four rules (rules.asm CheckBTRule_*)
#define JBT_RULE_THREE    1
#define JBT_RULE_SPECIES  2
#define JBT_RULE_ITEMS    4
#define JBT_RULE_EGG      8

#define BATTLETOWER_STREAK_LENGTH    7
#define BATTLETOWER_REWARD_QUANTITY  5

// the save's vars (Crystal keeps these in SRAM): the state (challenge, explanation read, level group, trainers
// beaten, reward) and the streak's trainers (a byte each)
#define VAR_JOHTO_BT_STATE  0x5168
#define VAR_JOHTO_BT_SEEN   0x5169   // to 0x516C

#endif // GUARD_CONSTANTS_JOHTO_BATTLE_TOWER_H
