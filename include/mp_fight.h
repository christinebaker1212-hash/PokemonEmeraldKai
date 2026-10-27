#ifndef GUARD_MP_FIGHT_H
#define GUARD_MP_FIGHT_H

// Moemon Fight: the VS cabinet in MAUVILLE's GAME CORNER (src/mp/mp_fight.c, docs/FIGHT_DECISIONS.md)

// The main loop's step: TRUE while a fight is the game picture (the game itself waits)
bool8 MP_Fight_Step(void);
bool8 MP_Fight_IsActive(void);

// Specials. VAR_0x8004: the seat (0 west = the left side, 1 east)
void MP_Fight_StartCpu(void);
// VAR_RESULT: 0 nothing, 1 won, 2 lost, 3 draw, 4 cancelled, 5 the cabinet is out of order
void MP_Fight_Result(void);

#endif // GUARD_MP_FIGHT_H
