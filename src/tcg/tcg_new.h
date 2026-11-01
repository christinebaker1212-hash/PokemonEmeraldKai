/* Moemon Kai: NEW on cards in the BINDER of all three card games (tcg_new.c) */
#ifndef TCG_NEW_H
#define TCG_NEW_H
enum { CARDNEW_POKEMON, CARDNEW_YUGIOH, CARDNEW_TRIAD, CARDNEW_GAMES };
int CardNew_Is(unsigned game, unsigned card, unsigned owned);     /* more copies than the BINDER last showed */
void CardNew_Saw(unsigned game, unsigned card, unsigned owned);   /* the cursor on it: no longer NEW */
void CardNew_Badge(int x, int y, unsigned frame);                 /* the tag, its top-left at (x, y) */
void CardNew_Forget(void);                                        /* another save file: read its own next time */
#endif
