/* CARD SCALING (src/tcg/tcg_scale.c): every deck's strength as an Elo rating, measured by
 * the game's own AI playing it (tools/tcg/rate_decks.sh -> src/tcg/tcg_ratings.c), the
 * player's deck rated the same way in the game, and a correction learned from how the
 * player actually does against the people they duel. */
#ifndef MOEMON_TCG_SCALE_H
#define MOEMON_TCG_SCALE_H
#include <stdint.h>
#include "tcg/tcg.h"

#define TCG_SCALE_MEAN 1500       /* the average deck */
#define TCG_SCALE_SKILL_MAX 300   /* the correction never goes past this either way */
#define TCG_SCALE_REF_GAMES 6     /* duels against each reference deck to rate a deck */

/* Generated (tcg_ratings.c): by gTcgDeckDefinitions index, by special trainer, by variant.
 * (tools/tcg/rate_decks.c makes them, so there they're its own, filled in as it goes.) */
#ifdef TCG_RATING_TOOL
#define TCG_RATING_CONST
#else
#define TCG_RATING_CONST const
#endif
extern TCG_RATING_CONST int16_t gTcgDeckRating[];
extern TCG_RATING_CONST int16_t gTcgSpecialRating[];
extern TCG_RATING_CONST int16_t gTcgVariantRating[];
/* The Game Boy decks a deck is measured against in the game (gTcgDeckDefinitions indices). */
extern TCG_RATING_CONST uint8_t gTcgRatingReferences[];
extern TCG_RATING_CONST unsigned gTcgRatingReferenceCount;

/* What each AI level gives away against the best (tcg_levels.c, tools/tcg/rate_cards.sh), and the player counted
 * as the SKILLED level until their record says more */
extern const int16_t gTcgLevelRating[4];
#define TcgScale_LevelRating(level) gTcgLevelRating[(level) > 3 ? 3 : (level)]

/* One whole AI-against-AI duel: the winner (0 a, 1 b) or -1 when it never ended. */
int TcgScale_Play(const uint16_t a[TCG_DECK_SIZE], const uint16_t b[TCG_DECK_SIZE], uint32_t seed);
/* The chance a deck rated `rating` beats one rated `other`, in 1/1000. */
unsigned TcgScale_Expected(int rating, int other);
/* The rating that explains `score` (half points, out of `games`) against decks rated `ratings`. */
int TcgScale_Fit(const int16_t *ratings, const unsigned *games, const unsigned *halfPoints, unsigned count);
/* A deck's rating, measured against the reference decks (cached by the deck's cards). */
int TcgScale_RateDeck(const uint16_t cards[TCG_DECK_SIZE]);
/* After a duel against someone rated `opponent` with a deck rated `deck`: the player's
 * correction moves toward what the result says. */
void TcgScale_Result(struct TcgCollection *c, int deck, int opponent, int won);
#endif
