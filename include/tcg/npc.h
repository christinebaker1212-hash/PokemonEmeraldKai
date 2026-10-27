/* Card players on the map: who they are, which deck they use, what a win over
 * them is worth (src/tcg/tcg_field.c), handed to the card screens. */
#ifndef MOEMON_TCG_NPC_H
#define MOEMON_TCG_NPC_H
#include <stdint.h>

/* Trainers with decks of their own (tools/tcg/gen_special_decks.py, in this order). */
enum
{
    TCG_SPECIAL_ROXANNE,
    TCG_SPECIAL_BRAWLY,
    TCG_SPECIAL_WATTSON,
    TCG_SPECIAL_FLANNERY,
    TCG_SPECIAL_NORMAN,
    TCG_SPECIAL_WINONA,
    TCG_SPECIAL_TATE_LIZA,
    TCG_SPECIAL_JUAN,
    TCG_SPECIAL_SIDNEY,
    TCG_SPECIAL_PHOEBE,
    TCG_SPECIAL_GLACIA,
    TCG_SPECIAL_DRAKE,
    TCG_SPECIAL_WALLACE,
    TCG_SPECIAL_STEVEN,
    TCG_SPECIAL_RIVAL,
    TCG_SPECIAL_WALLY,
    TCG_SPECIAL_MAXIE,
    TCG_SPECIAL_ARCHIE,
    TCG_SPECIAL_ANABEL,
    TCG_SPECIAL_TUCKER,
    TCG_SPECIAL_SPENSER,
    TCG_SPECIAL_GRETA,
    TCG_SPECIAL_NOLAND,
    TCG_SPECIAL_LUCY,
    TCG_SPECIAL_COUNT,
    TCG_SPECIAL_NONE = 0xFF
};
#define TCG_GYM_LEADERS 8 /* TCG_SPECIAL_ROXANNE..JUAN */

struct TcgOpponent
{
    char name[28];         /* "BUG CATCHER RICK", "ROXANNE" */
    char title[24];        /* "LEADER", "ELITE FOUR"... ("" for most) */
    const uint16_t *deck;  /* 60 cards */
    uint32_t key;          /* the daily reward's (map + person, or the special trainer) */
    uint16_t music, promo; /* promo: the card a first win hands over (0 none) */
    uint8_t mat, matType, special, pack, online;
    uint8_t portraitW, portraitH;
    uint32_t portrait[32 * 32]; /* their sprite as they faced you, ARGB */
};
extern const struct TcgDeckDefinition gTcgSpecialDecks[];
extern const unsigned gTcgSpecialDeckCount;

/* The card screens' side of a challenge (tcg_ui.c). */
void Tcg_OpenNpcChallenge(const struct TcgOpponent *opponent);
/* The CARD CLUB (tcg_ui.c): the starter decks, the teaching duel, the counter. */
void Tcg_OpenStarterReveal(void);
void Tcg_OpenTeachingDuel(void);
void Tcg_OpenClubCounter(void);
/* Rewards waiting at the CARD CLUB (tcg_field.c reads them, tcg_ui.c grants cards). */
unsigned Tcg_SpecialPromo(unsigned special);
#endif
