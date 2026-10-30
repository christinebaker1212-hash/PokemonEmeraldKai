/* Triple Triad players on the map (src/tt/tt_field.c): who they are, the hand they bring today, handed to the card
 * screens (src/tt/tt_ui.c); and where each card is won or sold (src/tt/tt_pools.c, tools/tt/gen_pools.py). */
#ifndef MOEMON_TT_NPC_H
#define MOEMON_TT_NPC_H
#include <stdint.h>
#include "tt/tt.h"
#include "tt/collection.h"

enum
{
    TT_SPECIAL_STEVEN,
    TT_SPECIAL_RIVAL,
    TT_SPECIAL_WALLY,
    TT_SPECIAL_WALLACE,
    TT_SPECIAL_JUAN,
    TT_SPECIAL_WINONA,
    TT_SPECIAL_NORMAN,
    TT_SPECIAL_ROXANNE,
    TT_SPECIAL_TATE_LIZA,
    TT_SPECIAL_MR_STONE,
    TT_SPECIAL_MR_BRINEY,
    TT_SPECIAL_SCOTT,
    TT_SPECIAL_BIRCH,
    TT_SPECIAL_MOM,
    TT_SPECIAL_QUEEN,
    TT_SPECIAL_COUNT
};
#define TT_SPECIAL_NONE 0xFF
/* 0-terminated lists */
extern const uint16_t *const gTtRegionPool[TT_REGIONS];      /* the cards a region's players carry */
extern const uint16_t *const gTtSellerStock[TT_REGIONS];     /* each region's seller's singles (3 stars at most) */
extern const uint16_t *const gTtSpecialCards[TT_SPECIAL_COUNT];   /* a special player's own rare cards */

struct TtOpponent
{
    char name[28];
    char title[24];
    uint16_t hand[TT_HAND];
    uint32_t key;              /* (map + person, or the special player) */
    uint16_t music;
    uint8_t special, level, region, online, lesson;
    uint8_t fit, tier;         /* fit: their hand may be fitted to the player's (tier: the badges' cap) */
    uint32_t hash;             /* (who they are: their hands come from it) */
    uint8_t portraitW, portraitH;
    uint8_t voice;             /* what they say: include/tcg/quotes.h QUOTE_VOICE_*, and who a special player is */
    char quoteWho[16];
    uint32_t portrait[64 * 64]; /* ARGB: their sprite as they faced you (up to 32x32), or 64x64: their battle picture */
};

/* Today's hand for a player of `region` (h: who they are) at a strength (0-3, from the badges) */
void Tt_RegionHand(unsigned region, uint32_t h, unsigned tier, unsigned day, uint16_t hand[TT_HAND]);
/* A special player's hand today: one of their five-star cards, one four-star, three three-star */
void Tt_SpecialHand(unsigned special, unsigned day, uint16_t hand[TT_HAND]);
/* Fitting (CARD SCALING): the hand they bring against a player `mine` strong (deck power plus skill; the player
 * counted as playing like the SKILLED level, Tt_PlayerRating): a touch
 * weaker than the player's for everyone, a real test from the special players and more from the CARD QUEEN, counting
 * how well they play (an ordinary player whose cards fall well short plays a level better: o->level). Returns the hand's
 * power. */
int Tt_FitOpponentHand(struct TtOpponent *o, int mine, uint32_t rules, unsigned day, uint16_t hand[TT_HAND]);
/* How the player is favored in rating points (the chance it gives: Tt_Chance(favor, 0)) */
int Tt_OpponentFavor(const struct TtOpponent *o);

/* The card screens' side (tt_ui.c) */
void TtUi_OpenChallenge(const struct TtOpponent *opponent);
void TtUi_OpenSeller(unsigned region);   /* a seller's counter (a script waits) */
void TtUi_OpenStarter(void);             /* the first hand, from a seller (a script waits) */
void TtUi_OpenQueen(const struct TtOpponent *queen);   /* the CARD QUEEN's favors (a script waits) */

/* Who plays what (tt_field.c): 0 not Triple Triad; TT_PLAYS_ONLY only Triple Triad (*line, *refused as Yu-Gi-Oh!'s);
 * TT_PLAYS_ALSO Triple Triad besides the game(s) they already play */
#define TT_PLAYS_ONLY 1
#define TT_PLAYS_ALSO 2
struct ObjectEvent;
int Tt_FieldPick(struct ObjectEvent *npc, const uint8_t *script, uint32_t hash, uint32_t key, int clubStaff,
                 struct TtOpponent *opponent, const uint8_t **line, int *refused);
/* What a player who plays several games says when asked (games: bits 1 Pokemon cards, 2 Yu-Gi-Oh!, 4 Triple
 * Triad); NULL: the usual line */
const uint8_t *Tt_WhichLine(unsigned games, uint32_t hash);
/* The region the player stands in (TT_REGION_*) */
unsigned Tt_CurrentRegion(void);
/* The CARD QUEEN at the table (tt_field.c), and after she moves: the maps show her in her region */
void Tt_QueenOpponent(struct TtOpponent *o);
void Tt_QueenPlaced(void);
#endif
