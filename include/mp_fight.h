#ifndef GUARD_MP_FIGHT_H
#define GUARD_MP_FIGHT_H

// Moemon Fight: the GAME CENTER 3F's VS cabinets (src/mp/mp_fight.c, docs/FIGHT_DECISIONS.md)

// The main loop's step: TRUE while a fight is the game picture (the game itself waits)
bool8 MP_Fight_Step(void);
bool8 MP_Fight_IsActive(void);
// Every frame (mp_main.c): seats and their bubbles
void MP_Fight_Update(void);
void MP_Fight_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Fight_TestPress(u16 buttons);
// The arcade game (include/arcade.h ARCADE_*) on our screen + 1, 0 none (mp_spectate.c tags the frames with it)
u8 MP_Fight_ArcadeShown(void);

// Specials. VAR_0x8005: the cabinet; VAR_0x8004: the seat (0 west = 1P, the left side; 1 east = 2P)
void MP_Fight_SeatCheck(void);   // VAR_RESULT: 0 nobody, 1 a friend waits at the other seat, 2 this seat is taken,
                                 // 3 friends are fighting here (STR_VAR_1: the friend)
void MP_Fight_CabinetReady(void); // VAR_RESULT 0: an arcade game's ROM set is missing (STR_VAR_1 game, STR_VAR_2 zips)
void MP_Fight_Wait(void);        // waits for a friend at the other seat (B stands up)
void MP_Fight_StartCpu(void);
void MP_Fight_Challenge(void);   // asks the friend at the other seat
// VAR_RESULT: 0 nothing, 1 won, 2 lost, 3 draw, 4 cancelled, 5 the cabinet is out of order, 6 the friend couldn't,
// 7 the friend dropped out (no contest), 8 stood up (STR_VAR_1: the friend for 6 and 7)
void MP_Fight_Result(void);

#endif // GUARD_MP_FIGHT_H
