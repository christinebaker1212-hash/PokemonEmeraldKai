/* A game of Triple Triad (src/tt/tt_duel.c): the 3x3 board, the two hands, the rules and the round. Everything
 * follows from the seed and the moves (a hand card onto a square), so a friend's game only exchanges moves. The
 * capture rules are Final Fantasy VIII's, as its own code does them (SAME, SAME WALL and PLUS before the plain
 * capture, ELEMENTAL's +1 / -1 for the plain capture only, and the COMBO that cards taken by SAME or PLUS start). */
#ifndef MOEMON_TT_DUEL_H
#define MOEMON_TT_DUEL_H
#include "tt/tt.h"

enum
{
    TTE_PLACE,    /* a card goes down: cell, slot (hand), seat */
    TTE_RULE,     /* SAME / PLUS / COMBO fired: by (TT_BY_*), cell */
    TTE_FLIP,     /* a card changes sides: cell, dir (the side it was taken from), by, wave (0 the play, 1+ combo) */
    TTE_SHIFT,    /* ASCENSION / DESCENSION changed a type's cards: type, amount */
    TTE_ROUND,    /* SUDDEN DEATH: a new round with the cards each side holds */
    TTE_END       /* the game is over: winner (0, 1, 2 a draw) */
};
enum { TT_BY_PLAIN, TT_BY_SAME, TT_BY_PLUS, TT_BY_COMBO, TT_BY_WALL };
struct TtEvent
{
    uint8_t kind, cell, dir, by, wave, seat, slot;
    int8_t amount;
    uint16_t card;
};
struct TtCell
{
    uint16_t card;     /* 0: empty */
    uint8_t owner;     /* the seat whose color it shows */
    uint8_t origin;    /* the seat whose card it is */
    uint8_t tile;      /* ELEMENTAL: the square's type (0 none) */
    int8_t elem;       /* ELEMENTAL: +1 / -1 for the card on it */
    uint8_t flags;     /* (the rules' working bits) */
};
#define TT_MAX_EVENTS 96
#define TT_MAX_ROUNDS 5    /* SUDDEN DEATH: a draw after this many rounds stands */
struct TtDuel
{
    uint32_t rules, rng, seed;
    uint8_t trade;
    uint16_t hand[2][TT_HAND];
    uint8_t origin[2][TT_HAND];    /* whose card each hand card is */
    uint8_t played[2];             /* bit per hand slot */
    uint8_t shown[2];              /* bit per hand slot the other side can see */
    uint8_t swapped[2];            /* SWAP: the slot that changed sides (0xFF none) */
    struct TtCell cell[TT_CELLS];
    uint8_t turn, toMove, first, forced, round, over, winner;
    uint8_t placedTypes[TT_TYPE_COUNT];
    uint8_t noEvents;              /* (the AI's copies don't record) */
    uint8_t evCount;
    struct TtEvent ev[TT_MAX_EVENTS];   /* (last: the AI copies everything before it) */
};

/* first: the seat that plays first. The hands as they are (RANDOM's deal is the caller's: Tt_DealRandom). */
void TtDuel_Start(struct TtDuel *d, uint32_t rules, unsigned trade, uint32_t seed, const uint16_t *hand0,
                  const uint16_t *hand1, unsigned first);
/* Bit per hand slot toMove may play now (ORDER / CHAOS force one) */
unsigned TtDuel_Playable(const struct TtDuel *d);
int TtDuel_CanPlay(const struct TtDuel *d, unsigned slot, unsigned cell);
/* Plays it (events from ev[0], evCount); 0 when it can't */
int TtDuel_Play(struct TtDuel *d, unsigned slot, unsigned cell);
unsigned TtDuel_Score(const struct TtDuel *d, unsigned seat);
/* A side as it counts now (ASCENSION / DESCENSION; ELEMENTAL's is separate: cell.elem) */
int TtDuel_Side(const struct TtDuel *d, unsigned cell, unsigned side);
int TtDuel_TypeShift(const struct TtDuel *d, unsigned type);
uint32_t TtDuel_Hash(const struct TtDuel *d);
/* The winner's spoils (the other side's cards): which hand cards of `loser` (the cards as they first were dealt,
 * dealt[]) the trade rule lets the winner take. Returns a bit per dealt slot; ONE asks the winner to pick one,
 * DIFF as many as the score difference. */
unsigned TtDuel_SpoilCount(const struct TtDuel *d, unsigned winner);
unsigned TtDuel_DirectMask(const struct TtDuel *d, unsigned winner, const uint16_t *dealt);
/* RANDOM: five different cards dealt from those owned (counts per id), deterministic from seed */
int Tt_DealRandom(const uint8_t *owned, uint32_t seed, uint16_t *hand);
uint32_t Tt_Rand(uint32_t *state);

/* The computer player (src/tt/tt_ai.c): level 0 (a beginner) to 3 (the best); seat to move. It never reads the
 * other side's hidden cards: TtAi_View is the game as seat may know it (unknown cards as stand-ins). */
void TtAi_View(const struct TtDuel *d, unsigned seat, struct TtDuel *view);
void TtAi_Choose(const struct TtDuel *d, unsigned level, uint32_t salt, unsigned *slot, unsigned *cell);
#endif
