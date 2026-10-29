/* The player's Yu-Gi-Oh! cards (src/ygo/ygo_collection.c): owned cards, saved decks, unopened boosters, the real
 * packs with their pull rates. Saved as "<save>.ygo" next to the save (src/ygo/ygo_integration.c). CARD POINTS are
 * the Pokemon collection's (include/tcg/tcg.h): one counter for both games. */
#ifndef MOEMON_YGO_COLLECTION_H
#define MOEMON_YGO_COLLECTION_H
#include <stddef.h>
#include <stdint.h>
#include "ygo/ygo.h"

#define YGO_SAVED_DECKS 8
#define YGO_DAILY_SLOTS 64
#define YGO_COLLECTION_MAX 999
#define YGO_PACK_SIZE 9
/* A booster's rare slot: Super Rare 1 pack in 6, Ultra Rare 1 in 12, Secret Rare 1 in 24, else a Rare (the
 * English boosters of 2002-2003). The 8 commons: Short Prints come half as often as other commons, Super Short
 * Prints a quarter as often. */
#define YGO_SUPER_ODDS 6
#define YGO_ULTRA_ODDS 12
#define YGO_SECRET_ODDS 24
#define YGO_GC_SINGLE_BYTES ((YGO_CARD_COUNT + 8) / 8)

struct YgoSavedDeck
{
    char name[YGO_NAME_LEN];
    uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];   /* 0-terminated */
};
enum
{
    YGO_FLAG_TUTORIAL = 1,   /* the teaching duel was offered */
    YGO_FLAG_CLUB_MET = 2,   /* the CARD CLUB's duelist introduced themselves */
    YGO_FLAG_CASE_SIDE = 4,  /* the CARD CASE was last on its YU-GI-OH! side */
};
struct YgoDaily
{
    uint32_t key;
    uint16_t day;
};
struct YgoCollection
{
    uint16_t owned[YGO_CARD_COUNT + 1];
    struct YgoSavedDeck decks[YGO_SAVED_DECKS];
    uint16_t packs[YGO_SET_COUNT];   /* unopened boosters */
    uint32_t wins, losses, rng, owner, flags, packsOpened, onlineWins, trades;
    uint32_t specialBeaten, promoGiven;
    uint8_t optedIn, activeDeck, starter;   /* starter: the Starter Deck chosen (YGO_SET_*) + 1 */
    struct YgoDaily daily[YGO_DAILY_SLOTS];
    /* the GAME CENTER clerk sells one copy of each single (a bit per card id) and one of each Starter Deck */
    uint8_t gcSingles[YGO_GC_SINGLE_BYTES];
    uint8_t gcDecks;
    /* format 3: each saved deck's ACE CARD, the card that stands for it on the screens (0: picked for it) */
    uint16_t ace[YGO_SAVED_DECKS];
    /* format 4: the Forbidden & Limited list the player's decks follow, by name ("": the game's own) */
    char banlist[32];
};

void Ygo_CollectionInit(struct YgoCollection *c, uint32_t seed);
int Ygo_CollectionLoad(struct YgoCollection *c, const char *path);
int Ygo_CollectionSave(const struct YgoCollection *c, const char *path);   /* never in dev builds */

/* The first deck: one of the real Starter Decks (YGO_SET_SDY / SDK / SDJ / SDP) */
int Ygo_ChooseStarter(struct YgoCollection *c, unsigned set);
/* A booster set the shop and the opponents can hand out now: most of its cards work (Ygo_CardReady) */
int Ygo_SetAvailable(unsigned set);
/* Opens one of the boosters held of set: 9 cards (8 commons, the rare slot last) into result; 0 when none */
int Ygo_OpenPack(struct YgoCollection *c, unsigned set, uint16_t result[YGO_PACK_SIZE]);
int Ygo_AwardPack(struct YgoCollection *c, unsigned set);
/* A random booster set that's available (rewards) */
unsigned Ygo_RandomSet(uint32_t *rng);
/* Copies allowed in a deck by the list in use (Ygo_UseBanlist) and the 3-copy rule */
unsigned Ygo_CopyLimit(unsigned card);
/* ... by the game's own list (Stairway's): the opponents' decks */
unsigned Ygo_GameLimit(unsigned card);
/* The Forbidden & Limited lists: 0 the game's own, then each of EDOPro's (YGO_EDO_LISTS_MAX at most) */
unsigned Ygo_BanlistCount(void);
const char *Ygo_BanlistName(unsigned list);
/* The list the player's decks follow from now (by name; "" or an unknown name: the game's own); returns its index */
unsigned Ygo_UseBanlist(const char *name);
unsigned Ygo_Banlist(void);
/* A deck that can be played: 40-60 main, Fusion Deck of 15 or less (Fusion Monsters only), the copy limits,
 * owned cards (owned NULL: don't check), every card working */
int Ygo_DeckValidate(const uint16_t *main, const uint16_t *extra, const uint16_t *owned, char *reason, size_t size);
unsigned Ygo_DeckCount(const uint16_t *cards, unsigned max);
int Ygo_DeckAdd(struct YgoCollection *c, unsigned deck, unsigned card, char *reason, size_t size);
int Ygo_DeckRemove(struct YgoCollection *c, unsigned deck, unsigned card);
/* The card that stands for a saved deck: its ACE CARD while the deck holds it, else the strongest monster */
unsigned Ygo_DeckAce(const struct YgoCollection *c, unsigned deck);
unsigned Ygo_DeckMascot(const uint16_t *main, unsigned count);
/* A DECK RECIPE (a Starter Deck, card for card) built into deck slot `deck` from the cards owned; returns the
 * cards placed, *missing how many of the recipe's couldn't be */
unsigned Ygo_DeckFromRecipe(struct YgoCollection *c, unsigned deck, const uint16_t *main, const uint16_t *extra,
                            const char *name, unsigned *missing);
/* How many of the recipe's cards (main and Fusion) are owned, and how many it has */
unsigned Ygo_RecipeOwned(const struct YgoCollection *c, const uint16_t *main, const uint16_t *extra, unsigned *total);
/* Spares (past 3 and what the saved decks use) and what they're worth in CARD POINTS; the shop's prices */
unsigned Ygo_Spare(const struct YgoCollection *c, unsigned card);
unsigned Ygo_PointValue(unsigned card);
unsigned Ygo_PointCost(unsigned card);   /* 0: not sold */
#define YGO_PACK_COST 12
/* The GAME CENTER clerk's price in money for a single (0: not sold), by the same rarity as the CARD POINTS */
unsigned Ygo_MoneyPrice(unsigned card);
/* Distinct cards owned of a set (YGO_SET_COUNT: the whole pool) */
unsigned Ygo_SetOwned(const struct YgoCollection *c, unsigned set, unsigned *total);
/* Deck codes: YDKE ("ydke://" and each part's passwords in base64) and .ydk files (#main / #extra / !side, one
 * password a line). Reading: the main and Extra Deck as card ids (0-terminated), *notHere how many of its cards the
 * game hasn't got (not in the catalog or not playable: such a deck can't be made); 0 when it isn't a deck code at
 * all. The Side Deck is left out. Writing: buf filled, 0 when it doesn't fit. */
int Ygo_ReadYdke(const char *text, uint16_t main[YGO_DECK_MAX], uint16_t extra[YGO_FUSION_MAX], unsigned *notHere);
int Ygo_ReadYdk(const char *text, uint16_t main[YGO_DECK_MAX], uint16_t extra[YGO_FUSION_MAX], unsigned *notHere);
int Ygo_WriteYdke(const uint16_t *main, const uint16_t *extra, char *buf, size_t size);
int Ygo_WriteYdk(const uint16_t *main, const uint16_t *extra, const char *name, char *buf, size_t size);
/* Cards of a deck the collection doesn't have (counting copies) */
unsigned Ygo_DeckMissing(const struct YgoCollection *c, const uint16_t *main, const uint16_t *extra);
int Ygo_DailyAvailable(const struct YgoCollection *c, uint32_t key, unsigned day);
void Ygo_DailyTake(struct YgoCollection *c, uint32_t key, unsigned day);
#endif
