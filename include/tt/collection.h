/* The player's Triple Triad cards and HOENN's rules (src/tt/tt_collection.c): the cards owned, the decks, the packs
 * held, each region's rules and trade rule, the rules the player carries from the last region played in, and where
 * the CARD QUEEN is. Saved as "<save>.tt" next to the save. */
#ifndef MOEMON_TT_COLLECTION_H
#define MOEMON_TT_COLLECTION_H
#include <stddef.h>
#include <stdint.h>
#include "tt/tt.h"

#define TT_MAX_COPIES 99
#define TT_CARD_BYTES ((TT_CARD_MAX + 8) / 8)
#define TT_REGION_MAX_RULES 7   /* a region never holds more (Final Fantasy VIII's busiest held all seven) */

/* HOENN's regions (then JOHTO and KANTO, one each), each with its own rules (like Final Fantasy VIII's Balamb, Galbadia, Dollet...) */
enum
{
    TT_REGION_PETALBURG,   /* LITTLEROOT, OLDALE, PETALBURG and their routes */
    TT_REGION_RUSTBORO,    /* RUSTBORO, the woods, ROUTES 115-116, RUSTURF TUNNEL */
    TT_REGION_SLATEPORT,   /* DEWFORD, SLATEPORT, the southern seas, ROUTES 109-110 */
    TT_REGION_MAUVILLE,    /* MAUVILLE, VERDANTURF, ROUTES 111 (south), 117, 118 */
    TT_REGION_LAVARIDGE,   /* LAVARIDGE, FALLARBOR, the desert, MT. CHIMNEY, METEOR FALLS */
    TT_REGION_LILYCOVE,    /* FORTREE, LILYCOVE, ROUTES 119-121, MT. PYRE, the SAFARI ZONE */
    TT_REGION_SOOTOPOLIS,  /* MOSSDEEP, SOOTOPOLIS, PACIFIDLOG and the eastern seas */
    TT_REGION_EVER_GRANDE, /* EVER GRANDE, VICTORY ROAD, the BATTLE FRONTIER */
    TT_REGION_JOHTO,       /* Moemon Kai: all of JOHTO */
    TT_REGION_KANTO,       /* Moemon Kai: all of KANTO and the SEVII ISLANDS */
    TT_REGIONS,
    TT_REGION_NONE = 0xFF
};
const char *Tt_RegionName(unsigned region);
uint32_t Tt_RegionStartRules(unsigned region);
unsigned Tt_RegionStartTrade(unsigned region);

enum
{
    TT_FLAG_CASE_SIDE = 1,     /* the CARD CASE was last on its TRIPLE TRIAD side */
    TT_FLAG_MET_CLERK = 2,     /* a TRIPLE TRIAD seller explained the game */
    TT_FLAG_QUEEN_MET = 4,     /* the CARD QUEEN introduced herself */
    TT_FLAG_RULES_SEEN = 8,    /* the rules of a region were shown once (the long explanation) */
    TT_FLAG_QUEEN_FAVOR = 16,  /* the CARD QUEEN was beaten: she owes a favor (a trade rule, a rule abolished) */
};
struct TtDeck
{
    char name[TT_DECK_NAME + 1];
    uint16_t cards[TT_HAND];
};
struct TtCollection
{
    uint32_t owner, flags, rng;
    uint8_t optedIn, activeDeck;
    uint8_t owned[TT_CARD_MAX + 1];          /* copies of each card */
    uint8_t seen[TT_CARD_BYTES];             /* a card met on a table (the binder shows it) */
    uint8_t bought[TT_CARD_BYTES];           /* the shops sell one copy of each single */
    struct TtDeck decks[TT_DECKS];
    uint16_t packs[TT_PACK_TYPES];           /* unopened packs */
    /* HOENN's rules */
    uint32_t regionRules[TT_REGIONS];
    uint8_t regionTrade[TT_REGIONS];
    uint16_t regionGames[TT_REGIONS];
    uint32_t carried;                        /* the rules of the region last played in */
    uint8_t carriedFrom;                     /* that region (TT_REGION_NONE: none yet) */
    uint8_t queenRegion;
    uint16_t queenDay;                       /* the day she arrived */
    /* records */
    uint32_t wins, losses, draws, packsOpened, onlineWins, trades, combos, sames, pluses, spreads, abolishes;
    uint32_t specialBeaten;                  /* a bit per special player (tt/npc.h) */
    uint32_t perfects;                       /* games won 9-1 / with every square */
    uint16_t lastDaily;                      /* the day of the last daily reward */
    int16_t skill;                           /* how the player does against the computer, past what the cards say */
    uint16_t rated;                          /* games that taught it */
};
#define TT_SKILL_MAX 300

void Tt_CollectionInit(struct TtCollection *c, uint32_t seed);
int Tt_CollectionLoad(struct TtCollection *c, const char *path);
int Tt_CollectionSave(const struct TtCollection *c, const char *path);   /* never in dev builds */

/* A card into the collection (capped); returns 1 when it's the first copy */
int Tt_Give(struct TtCollection *c, unsigned id);
int Tt_Has(const struct TtCollection *c, unsigned id);
void Tt_See(struct TtCollection *c, unsigned id);
int Tt_Seen(const struct TtCollection *c, unsigned id);
/* Copies a deck uses (a copy in a deck can't be sold or traded away) */
unsigned Tt_InDecks(const struct TtCollection *c, unsigned id);
unsigned Tt_OwnedCount(const struct TtCollection *c, unsigned stars, unsigned *total);   /* stars 0: all */
/* The first deck: five cards of one or two stars, from the starter pool (tt/npc.h), into deck 0 */
void Tt_GiveStarter(struct TtCollection *c);
int Tt_DeckReady(const struct TtCollection *c, unsigned deck);
/* A pack of `pack` (TT_PACK_*: its stars are pack + 1): TT_PACK_SIZE cards of those stars */
int Tt_OpenPack(struct TtCollection *c, unsigned pack, uint16_t out[TT_PACK_SIZE]);

/* After a game against the computer (mine, theirs: the powers with the level; result 0 won, 1 lost, 2 a draw) */
void Tt_SkillResult(struct TtCollection *c, int mine, int theirs, unsigned result);

/* The rules spreading. Challenging someone in `region`: one of the carried rules the region lacks may spread to
 * it, or one of its rules the player doesn't carry may be abolished (more often where the CARD QUEEN is). */
enum { TT_MIX_NONE, TT_MIX_SPREAD, TT_MIX_ABOLISH };
struct TtMix
{
    uint8_t kind, rule;
};
struct TtMix Tt_MixRules(struct TtCollection *c, unsigned region, uint32_t roll);
/* A game played in region: its rules are the ones carried from now on */
void Tt_PlayedIn(struct TtCollection *c, unsigned region);
/* The CARD QUEEN moves (day: today); she may bring a rule from afar to her new region: returns it or 0xFF */
unsigned Tt_QueenMoves(struct TtCollection *c, unsigned region, unsigned day, uint32_t roll);
#endif
