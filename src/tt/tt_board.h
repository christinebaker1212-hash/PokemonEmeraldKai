/* The Triple Triad table (src/tt/tt_board.c): the 3x3 board between the two hands, your blue hand on the left and
 * the red one on the right as in Final Fantasy XIV, at the native 240x160. It deals the hands, spins for who goes
 * first, then plays the rules engine's events one by one (a card flying down, flips, SAME / PLUS / COMBO) and asks
 * the player for a card and a square. */
#ifndef MOEMON_TT_BOARD_H
#define MOEMON_TT_BOARD_H
#include "tt/duel.h"

enum
{
    TB_NONE,
    TB_MOVE,      /* *slot / *cell: the card to play and where */
    TB_CONCEDE,   /* the player gave up (asked first) */
    TB_DONE       /* the finished game was dismissed */
};
/* me: the seat on the left (online, the friend who accepted sits in seat 1) */
void TtBoard_Begin(const struct TtDuel *truth, unsigned me, const char *myName, const char *theirName, unsigned music,
                   int online);
/* After TtDuel_Play: show what the move did (the duel's events) */
void TtBoard_Show(const struct TtDuel *truth);
int TtBoard_Idle(void);          /* everything shown, nothing moving */
int TtBoard_Step(unsigned pressed, unsigned held, unsigned *slot, unsigned *cell);
void TtBoard_Render(void);
void TtBoard_End(void);
void TtBoard_SetFast(int fast);
void TtBoard_SetWaiting(int waiting);     /* the other side is thinking (an NPC, or a friend's move on its way) */
void TtBoard_Result(const char *text);    /* the lines under the result: what was won */
void TtBoard_Lesson(int on);              /* the CARD QUEEN's or a seller's first game: tips */
/* Test harness: the cursor on a hand card (0-4) or a square (5-13) */
void TtBoard_DebugFocus(int where);
int TtBoard_DebugPhase(void);   /* phase * 1000 + frames in it */
#endif
