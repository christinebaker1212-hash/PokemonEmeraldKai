/* The Yu-Gi-Oh! duel table (src/ygo/ygo_board.c): today's layout at the native 240x160. Five Monster Zones and
 * five Spell & Trap Zones a side, the Field Zone, the Deck, the Graveyard and the Fusion Deck, your hand along the
 * bottom; the opponent's side turned around at the top. The table follows the rules engine through its events
 * (one at a time: a summon, an activation, an attack...) and asks the player only the engine's questions. */
#ifndef MOEMON_YGO_BOARD_H
#define MOEMON_YGO_BOARD_H
#include "ygo/duel.h"

enum
{
    YB_NONE,
    YB_ANSWER,   /* *answer answers the duel's question */
    YB_LEAVE,    /* the finished duel was dismissed */
    YB_INSPECT,  /* look at *card up close */
    YB_CONCEDE   /* the player confirmed conceding */
};
/* me: the seat at the bottom of the table (online, the friend who accepted sits in seat 1) */
void YgoBoard_Begin(const struct YgoDuel *truth, unsigned me, const char *opponent, int online);
/* The duel as it is now, every frame (the pointer is kept until the next call) */
void YgoBoard_Sync(const struct YgoDuel *truth);
int YgoBoard_Step(unsigned pressed, unsigned held, struct YgoAnswer *answer, unsigned *card);
/* Everything that happened has been shown: the opponent waits for it */
int YgoBoard_Idle(void);
void YgoBoard_SetFast(int fast);
/* The table's background for the next duel */
enum { YGO_BG_PLAIN, YGO_BG_LIGHT, YGO_BG_DARK, YGO_BG_WATER, YGO_BG_FIRE, YGO_BG_EARTH, YGO_BG_WIND, YGO_BG_ONLINE,
       YGO_BG_COUNT };
void YgoBoard_SetTheme(unsigned theme);
void YgoBoard_TestFieldBg(unsigned id);
void YgoBoard_SetWaiting(int waiting);   /* online: our answer is on its way */
void YgoBoard_SetMusic(unsigned song);
void YgoBoard_Tutorial(int on);          /* the lesson: a tip for each new kind of question */
void YgoBoard_Result(const char *message);
void YgoBoard_Render(void);
void YgoBoard_End(void);
/* Test harness: put the cursor on a card (kind 0 hand, 1 my monsters, 2 my Spells & Traps, 3 theirs) */
void YgoBoard_DebugFocus(int kind, int index);
#endif
