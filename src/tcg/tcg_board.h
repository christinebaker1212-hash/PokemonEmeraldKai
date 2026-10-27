/* The duel screen: a DS-style play mat at the native 240x160, drawn from
 * a presented copy of the rules state that follows the real one through
 * its events (coin flips, damage, Knock Outs...). Player 0 is always the
 * local player, at the bottom. */
#ifndef MOEMON_TCG_BOARD_H
#define MOEMON_TCG_BOARD_H
#include "tcg/duel.h"

enum
{
    BOARD_NONE,
    BOARD_ACTION,  /* *action holds a move for the rules engine */
    BOARD_LEAVE,   /* the finished duel was dismissed */
    BOARD_INSPECT, /* open the card inspector for *card */
    BOARD_CONCEDE  /* the player confirmed conceding (action holds TCG_CONCEDE) */
};
void TcgBoard_Begin(const struct TcgDuel *truth, const char *opponent, int online);
/* Feed the current rules state each frame; new states queue up for display. */
void TcgBoard_Sync(const struct TcgDuel *truth);
int TcgBoard_Step(unsigned pressed, unsigned held, struct TcgAction *action, unsigned *card);
/* True once everything that happened has been shown: the AI waits for it. */
int TcgBoard_Idle(void);
void TcgBoard_SetWaiting(int waiting);
void TcgBoard_Reject(const char *message);
void TcgBoard_Result(const char *message);
void TcgBoard_Render(void);
void TcgBoard_End(void);
void TcgBoard_DebugFocus(int kind, int index);
#endif
