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

/* Each game region's towns, each with its own rules (like Final Fantasy VIII's Balamb, Galbadia, Dollet...): HOENN's
 * eight, then eight each for JOHTO, KANTO, SINNOH and UNOVA (the user, 2026-11-20: "show specifically the region you're
 * in's towns"). The numbers are kept in the .tt file and online: 8-11 were JOHTO / KANTO / SINNOH / UNOVA when each was
 * one region, so they're each one's first town now (its rules carried on), the rest follow from 12. */
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
    TT_REGION_NEW_BARK,    /* JOHTO: NEW BARK, CHERRYGROVE, ROUTES 26-27, 29-30, 46 (was all of JOHTO) */
    TT_REGION_PALLET,      /* KANTO: PALLET, VIRIDIAN, ROUTES 1, 2, 22, VIRIDIAN FOREST (was all of KANTO) */
    TT_REGION_TWINLEAF,    /* SINNOH: TWINLEAF, SANDGEM, ROUTES 201-202, LAKE VERITY, the southern seas (was all of SINNOH) */
    TT_REGION_NUVEMA,      /* UNOVA: NUVEMA, ACCUMULA, STRIATON, ROUTES 1-3, the DREAMYARD (was all of UNOVA) */
    TT_REGION_VIOLET,      /* JOHTO: VIOLET, ROUTES 31, 32, 36, SPROUT TOWER, the RUINS OF ALPH, DARK CAVE */
    TT_REGION_AZALEA,      /* JOHTO: AZALEA, ROUTE 33, UNION CAVE, SLOWPOKE WELL, ILEX FOREST */
    TT_REGION_GOLDENROD,   /* JOHTO: GOLDENROD, ROUTES 34-35, the NATIONAL PARK */
    TT_REGION_ECRUTEAK,    /* JOHTO: ECRUTEAK, ROUTES 37-38, the BURNED TOWER, TIN TOWER */
    TT_REGION_OLIVINE,     /* JOHTO: OLIVINE, CIANWOOD, ROUTES 39-41, 47-48, the LIGHTHOUSE, WHIRL ISLANDS */
    TT_REGION_MAHOGANY,    /* JOHTO: MAHOGANY, ROUTES 42-44, the LAKE OF RAGE, MT. MORTAR */
    TT_REGION_BLACKTHORN,  /* JOHTO: BLACKTHORN, ROUTES 28, 45, ICE PATH, DRAGON'S DEN, VICTORY ROAD, MT. SILVER */
    TT_REGION_PEWTER,      /* KANTO: PEWTER, ROUTES 3-4, MT. MOON */
    TT_REGION_CERULEAN,    /* KANTO: CERULEAN, LAVENDER, ROUTES 9, 10, 24, 25, ROCK TUNNEL, the POKeMON TOWER */
    TT_REGION_VERMILION,   /* KANTO: VERMILION, ROUTES 5, 6, 11, 12, DIGLETT'S CAVE, the S.S. ANNE */
    TT_REGION_CELADON,     /* KANTO: CELADON, SAFFRON, ROUTES 7, 8, 16, 17, SILPH CO. */
    TT_REGION_FUCHSIA,     /* KANTO: FUCHSIA, ROUTES 13-15, 18-19, the SAFARI ZONE */
    TT_REGION_CINNABAR,    /* KANTO: CINNABAR, ROUTES 20-21, SEAFOAM ISLANDS, the SEVII ISLANDS */
    TT_REGION_INDIGO,      /* KANTO: the INDIGO PLATEAU, ROUTE 23, VICTORY ROAD */
    TT_REGION_OREBURGH,    /* SINNOH: JUBILIFE, OREBURGH, ROUTES 203-204, the RAVAGED PATH */
    TT_REGION_ETERNA,      /* SINNOH: FLOAROMA, ETERNA, ROUTES 205-207, ETERNA FOREST, MT. CORONET */
    TT_REGION_HEARTHOME,   /* SINNOH: HEARTHOME, SOLACEON, ROUTES 208-210, the LOST TOWER */
    TT_REGION_VEILSTONE,   /* SINNOH: VEILSTONE, PASTORIA, CELESTIC, ROUTES 211-215, the GREAT MARSH, LAKE VALOR */
    TT_REGION_CANALAVE,    /* SINNOH: CANALAVE, ROUTE 218, IRON ISLAND, FULLMOON ISLAND */
    TT_REGION_SNOWPOINT,   /* SINNOH: SNOWPOINT, ROUTES 216-217, LAKE ACUITY */
    TT_REGION_SUNYSHORE,   /* SINNOH: SUNYSHORE, ROUTES 222-230, VICTORY ROAD, the BATTLE ZONE */
    TT_REGION_NACRENE,     /* UNOVA: NACRENE, PINWHEEL FOREST, SKYARROW BRIDGE */
    TT_REGION_CASTELIA,    /* UNOVA: CASTELIA, ROUTE 4, the DESERT RESORT, RELIC CASTLE */
    TT_REGION_NIMBASA,     /* UNOVA: NIMBASA, ANVILLE, ROUTES 5, 16, LOSTLORN FOREST */
    TT_REGION_DRIFTVEIL,   /* UNOVA: DRIFTVEIL, ROUTE 6, CHARGESTONE CAVE, COLD STORAGE */
    TT_REGION_MISTRALTON,  /* UNOVA: MISTRALTON, ROUTE 7, CELESTIAL TOWER, TWIST MOUNTAIN */
    TT_REGION_ICIRRUS,     /* UNOVA: ICIRRUS, ROUTE 8, the MOOR OF ICIRRUS, DRAGONSPIRAL TOWER */
    TT_REGION_OPELUCID,    /* UNOVA: OPELUCID, ROUTES 9-15, UNDELLA, LACUNOSA, VICTORY ROAD, N's CASTLE */
    TT_REGIONS,
    TT_REGION_NONE = 0xFF
};
/* The game regions the towns belong to (REGION_HOENN... in mp_region.h, in that order) */
enum { TT_WORLD_HOENN, TT_WORLD_JOHTO, TT_WORLD_KANTO, TT_WORLD_SINNOH, TT_WORLD_UNOVA, TT_WORLDS };
#define TT_WORLD_TOWNS 8
const char *Tt_RegionName(unsigned region);
unsigned Tt_WorldOf(unsigned region);                  /* TT_WORLD_* (HOENN for none) */
unsigned Tt_TownIndex(unsigned region);                /* its place in its region's list, 0-7 */
unsigned Tt_WorldTown(unsigned world, unsigned k);     /* the region's k-th town */
const char *Tt_WorldTitle(unsigned world);             /* "HOENN", "JOHTO"... */
const char *Tt_WorldName(unsigned region);   /* the game region a town is in */
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
