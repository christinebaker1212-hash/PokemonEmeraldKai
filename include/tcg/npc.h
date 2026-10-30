/* Card players on the map: who they are, which deck they use, what a win over
 * them is worth (src/tcg/tcg_field.c), handed to the card screens. */
#ifndef MOEMON_TCG_NPC_H
#define MOEMON_TCG_NPC_H
#include <stdint.h>
#include "tcg/tcg.h"

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
    uint8_t prizes;        /* the Prize cards they play with: 6 or 3 mostly, some 4 or 2 (0: 6) */
    /* CARD SCALING (Tcg_FitOpponent): the kind of deck they play (TCG_SCALE_POOL_*), a number
     * of their own for variety, their deck's rating, the player deck's (without the skill) */
    uint8_t pool;
    uint32_t pick;
    int16_t rating, deckRating;
    uint8_t level;          /* how the AI plays (0 a beginner .. 3 its best; the fit may raise it) */
    uint8_t portraitW, portraitH;
    /* What they say (include/tcg/quotes.h): their kind (QUOTE_VOICE_*) and, for the special trainers, who they are */
    uint8_t voice;
    char quoteWho[16];
    uint32_t portrait[64 * 64]; /* ARGB: their sprite as they faced you (up to 32x32), or 64x64: their battle picture */
};
extern const struct TcgDeckDefinition gTcgSpecialDecks[];
extern const unsigned gTcgSpecialDeckCount;
extern const uint16_t gTcgSpecialPromos[];
/* Each special trainer's Pokemon and promo on the other Game Boy decks of their type, and whose. */
extern const struct TcgDeckDefinition gTcgSpecialVariants[];
extern const unsigned gTcgSpecialVariantCount;
extern const uint8_t gTcgSpecialVariantOwner[];

#define TCG_SCALE_POOL_NONE 0xFF    /* a fixed deck: the teaching duel, practice, a friend */
#define TCG_SCALE_POOL_SPECIAL 0xFE /* a special trainer: their own deck or one of its variants */
/* CARD SCALING (tcg_field.c): the deck an opponent brings against `mine` (NULL: the active
 * deck): OPTIONS > GAME > CARD SCALING on, the one whose rating fits the player's; off, by
 * badges. Sets deck, rating and deckRating. */
void Tcg_FitOpponent(struct TcgOpponent *o, const uint16_t *mine);

/* The card screens' side of a challenge (tcg_ui.c). */
void Tcg_OpenNpcChallenge(const struct TcgOpponent *opponent);
/* The CARD CLUB (tcg_ui.c): the starter decks, the teaching duel, the counter. */
void Tcg_OpenStarterReveal(void);
void Tcg_OpenTeachingDuel(void);
void Tcg_OpenClubCounter(void);
/* Rewards waiting at the CARD CLUB (tcg_field.c reads them, tcg_ui.c grants cards). */
unsigned Tcg_SpecialPromo(unsigned special);
void Tcg_PortraitOfPlayer(unsigned id, struct TcgOpponent *o);
#endif
