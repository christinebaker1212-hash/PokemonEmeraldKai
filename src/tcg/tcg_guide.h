/* HOW TO PLAY (src/tcg/tcg_guide.c): a short guide to each card game, opened with SELECT on the CARD CASE's first
 * screen of that game. Pages of a few lines each over a stage that acts the rule out with the games' own cards. */
#ifndef MOEMON_TCG_GUIDE_H
#define MOEMON_TCG_GUIDE_H
void TcgGuide_Open(unsigned game);   /* KIT_GAME_* */
int TcgGuide_Active(void);
/* keys: the buttons held (the guide closes once B is let go, so the screen under it doesn't see the press) */
void TcgGuide_Step(unsigned pressed, unsigned keys);
void TcgGuide_Draw(void);
int TcgGuide_Pages(unsigned game);
void TcgGuide_Goto(int page);        /* (the test harness) */
#endif
