/* Triple Triad's power ratings (src/tt/tt_ratings.c, made by tools/tt/rate_cards.sh) and what they say about a game:
 * a hand is as strong as its cards' average, the computer levels below the best give some of it away. Elo-style
 * numbers, never shown: the screens turn them into words (src/tt/tt_rating.c). */
#ifndef MOEMON_TT_RATING_H
#define MOEMON_TT_RATING_H
#include <stdint.h>
#include "tt/tt.h"

extern const int16_t gTtCardRating[TT_CARD_MAX + 1];
/* what each level gives away by the table: [hands hidden, OPEN, hidden with SAME / PLUS, OPEN with SAME / PLUS] */
extern const int16_t gTtLevelRating[4][4];
extern const int16_t gTtFirstMove;

/* A hand's power: its cards' average rating */
int Tt_HandPower(const uint16_t *hand);
/* The chance (in 1/1000) that a side `mine` strong beats one `theirs` strong (powers plus level or skill) */
unsigned Tt_Chance(int mine, int theirs);
/* What a computer player of `level` gives away (0 for the best) under rules (OPEN widens the gaps) */
int Tt_LevelRating(unsigned level, uint32_t rules);
/* The player, before their own record says more: about as good as the SKILLED level, the CASUAL one when SAME or PLUS
 * is played (people miss those far more often than the computer's levels do) */
#define Tt_SamePlus(rules) (((rules) & (TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_PLUS))) != 0)
#define Tt_PlayerRating(rules) Tt_LevelRating(Tt_SamePlus(rules) ? 1 : 2, (rules))

/* Hands that fit (src/tt/tt_hands.c). A region's player today, as close to `target` as their region's cards and the
 * badges (tierCap) allow; a special player's: their five-star card kept, the rest chosen to come close. */
void Tt_FitRegionHand(unsigned region, uint32_t h, unsigned tierCap, unsigned day, int target, uint16_t hand[TT_HAND]);
void Tt_FitSpecialHand(unsigned special, unsigned day, int target, uint16_t hand[TT_HAND]);
/* RANDOM, fair to both: five of the cards owned as close to target as a few deals come (0: fewer than five owned);
 * five of a region's cards as close to target */
int Tt_DealFair(const uint8_t *owned, uint32_t seed, int target, uint16_t hand[TT_HAND]);
void Tt_DealRegion(unsigned region, uint32_t seed, int target, uint16_t hand[TT_HAND]);
#endif
