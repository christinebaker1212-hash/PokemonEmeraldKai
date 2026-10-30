#ifndef MOEMON_TCG_H
#define MOEMON_TCG_H
#include <stddef.h>
#include <stdint.h>
#define TCG_CARD_COUNT 442   /* the first Game Boy game's 228, then Pokemon Card GB2's (tools/tcg/import_poketcg2.py) */
#define TCG_DECK_SIZE 60
#define TCG_SAVED_DECKS 8
#define TCG_NAME_LEN 24
#define TCG_TYPES 7
#define TCG_PACK_TYPES 11    /* the first game's 4 boosters, then GB2's 7 (TCG_PACK_GB2) */
#define TCG_PACK_GB2 4
#define TCG_GB2_PACKS 7
#define TCG_GB2_PACK_MAX 64
#define TCG_NONE 255
#define TCG_COLLECTION_MAX 9999

enum TcgKind
{
    TCG_POKEMON,
    TCG_ENERGY,
    TCG_TRAINER
};
enum TcgType
{
    TCG_FIRE,
    TCG_GRASS,
    TCG_LIGHTNING,
    TCG_WATER,
    TCG_FIGHTING,
    TCG_PSYCHIC,
    TCG_COLORLESS
};
enum TcgCategory
{
    TCG_DAMAGE_NORMAL,
    TCG_DAMAGE_PLUS,
    TCG_DAMAGE_MINUS,
    TCG_DAMAGE_X,
    TCG_POKEMON_POWER,
    TCG_RESIDUAL
};
struct TcgAttack
{
    const char *name, *description, *effect, *animation;
    uint8_t cost[TCG_TYPES], damage, category;
};
struct TcgCard
{
    const char *name, *evolvesFrom, *description, *effect;
    struct TcgAttack attacks[2];
    uint16_t id, species;
    uint8_t kind, type, rarity, set, stage, hp, retreat, weakness, resistance, level;
    /* set: its booster (TCG_SET_*: 0-3 the first game's, 4 promos, 5 energy, 6-12 GB2's); set2: the GB2 booster that
     * also holds a first-game card (255 none); dark: a Dark Pokemon (Team Rocket's) */
    uint8_t set2, dark, holo;   /* holo: printed as a holofoil (GB2's cards; the first game's: Tcg_IsHolo's list) */
};
/* Each GB2 booster's cards (0 ends a list) */
extern const uint16_t gTcgGb2PackCards[TCG_GB2_PACKS][TCG_GB2_PACK_MAX];
/* Can that booster (pack type 0-10) hold this card? */
int Tcg_InPack(unsigned card, unsigned pack);
#define TCG_SET_PROMO 4
#define TCG_SET_ENERGY 5
#define TCG_SET_GB2 6
struct TcgDeckDefinition
{
    const char *name;
    uint16_t cards[TCG_DECK_SIZE];
};
extern const struct TcgCard gTcgCards[TCG_CARD_COUNT + 1];
extern const struct TcgDeckDefinition gTcgDeckDefinitions[];
extern const unsigned gTcgDeckDefinitionCount;
const struct TcgCard *Tcg_Card(unsigned id);
uint32_t Tcg_Random(uint32_t *state);
unsigned Tcg_RandomBelow(uint32_t *state, unsigned limit);
int Tcg_DeckValidate(const uint16_t cards[TCG_DECK_SIZE], const uint16_t *owned, char *reason, size_t size);

struct TcgSavedDeck
{
    char name[TCG_NAME_LEN];
    uint16_t cards[TCG_DECK_SIZE];
};
#define TCG_DAILY_SLOTS 64 /* opponents whose full reward was taken today */
#define TCG_SET_COUNT TCG_PACK_TYPES /* the booster sets, as the packs: COLOSSEUM ... LABORATORY, then GB2's seven */
enum
{
    TCG_FLAG_TUTORIAL = 1,    /* the teaching duel was offered */
    TCG_FLAG_GOLD_CASE = 2,   /* every card collected: the CARD CASE turned gold */
    TCG_FLAG_FULL_GIFT = 4,   /* ... and the CARD CLUB's gift was handed over */
    TCG_FLAG_CLUB_MET = 8,    /* the CARD CLUB member has introduced themselves */
};
struct TcgDaily
{
    uint32_t key;  /* the opponent (TcgNpc key) */
    uint16_t day;  /* the day its full reward was taken */
};
/* The record against one card player (an NPC's TcgOpponent key) */
struct TcgRecord
{
    uint32_t key;
    uint16_t wins, losses; /* a concession is a loss */
    uint16_t lastWin;      /* the day of the last win (Tcg_Today), 0 never */
    uint16_t lastSeen;     /* the day of the last duel: the oldest record makes room */
};
#define TCG_RECORD_SLOTS 128
#define TCG_GC_SINGLE_BYTES ((TCG_CARD_COUNT + 8) / 8)
struct TcgCollection
{
    uint16_t owned[TCG_CARD_COUNT + 1];
    struct TcgSavedDeck decks[TCG_SAVED_DECKS];
    uint16_t packs[TCG_PACK_TYPES];
    uint32_t wins, dust /* CARD POINTS (spare cards traded in) */, vouchers, rng;
    uint32_t owner; /* the save it belongs to (its Trainer ID): a new game starts fresh */
    uint8_t optedIn, activeDeck, fastAnimations, quiet;
    /* Format 3 */
    struct TcgDaily daily[TCG_DAILY_SLOTS];
    uint32_t specialBeaten, promoGiven; /* bits by special trainer (TCG_SPECIAL_*) */
    uint32_t milestones;                /* bit set * 3 + n: 25 / 50 / 75 % of a set rewarded (sets 0-9) */
    uint32_t packsOpened, onlineWins, trades, flags;
    /* Format 4: CARD SCALING's correction (Elo points the player does better than their
     * deck's rating says; include/tcg/scale.h) and how many duels it has learned from. */
    int16_t skill;
    uint16_t rated;
    /* Format 5: packs of each set opened since the last holo the player didn't have (TCG_HOLO_PITY) */
    uint8_t drought[TCG_PACK_TYPES];
    /* Format 6: what the GAME CENTER CARD SHOP sold (each structure deck once, one copy of each single: bits)
     * and the records against card players */
    uint8_t gcDecks;
    uint8_t gcSingles[TCG_GC_SINGLE_BYTES];
    struct TcgRecord records[TCG_RECORD_SLOTS];
    /* Format 7: each saved deck's ACE CARD, the card that stands for it on the screens (0: picked for it) */
    uint16_t ace[TCG_SAVED_DECKS];
    /* Format 8: the milestones' bits from 32 up (set * 3 + n - 32: the last GB2 boosters) */
    uint32_t milestonesHi;
};
void Tcg_CollectionInit(struct TcgCollection *c, uint32_t seed);
int Tcg_ChooseStarter(struct TcgCollection *c, unsigned starter);
/* A pack: 5 commons, 3 uncommons, a rare (a holo one pack in TCG_HOLO_ODDS, a plain rare
 * otherwise) and a Basic Energy. TCG_HOLO_PITY packs of a set without a holo the player didn't
 * have bring one they don't have. */
#define TCG_HOLO_ODDS 3
#define TCG_HOLO_PITY 20
int Tcg_OpenPack(struct TcgCollection *c, unsigned pack, uint16_t result[10]);
/* Printed as a holofoil: drawn one pack in three, never picked with CARD POINTS or as a reward. */
int Tcg_IsHolo(unsigned card);
int Tcg_AwardPack(struct TcgCollection *c, unsigned pack);
int Tcg_ExchangeDuplicates(struct TcgCollection *c, unsigned card, unsigned count);
int Tcg_CraftCard(struct TcgCollection *c, unsigned card);
int Tcg_DeckAdd(struct TcgCollection *c, unsigned deck, unsigned card, char *reason, size_t size);
int Tcg_DeckRemove(struct TcgCollection *c, unsigned deck, unsigned card);
int Tcg_DeckCopy(struct TcgCollection *c, unsigned from, unsigned to);
/* The card that stands for a saved deck: its ACE CARD while the deck holds it, else the most evolved, rarest,
 * strongest Pokemon (0: no Pokemon) */
unsigned Tcg_DeckAce(const struct TcgCollection *c, unsigned deck);
unsigned Tcg_DeckMascot(const uint16_t cards[TCG_DECK_SIZE]);
/* A DECK RECIPE (a starter or structure deck, card for card) built into deck slot `deck` from the cards owned
 * (copies other saved decks use count: they're shared, as in the builder). Returns the cards placed; *missing
 * gets how many of the recipe's cards couldn't be. */
unsigned Tcg_DeckFromRecipe(struct TcgCollection *c, unsigned deck, const uint16_t recipe[TCG_DECK_SIZE],
                            const char *name, unsigned *missing);
/* How many of the recipe's cards the collection holds (up to the recipe's count of each) */
unsigned Tcg_RecipeOwned(const struct TcgCollection *c, const uint16_t recipe[TCG_DECK_SIZE]);
/* Today's full reward from an opponent (FF8: one card of their deck): once a day each. */
int Tcg_DailyAvailable(const struct TcgCollection *c, uint32_t key, unsigned day);
void Tcg_DailyTake(struct TcgCollection *c, uint32_t key, unsigned day);
/* Distinct cards owned that a booster (0-10) can hold, or of all of them (set TCG_SET_COUNT). */
unsigned Tcg_SetOwned(const struct TcgCollection *c, unsigned set, unsigned *total);
/* Set milestones (25 / 50 / 75 %: n 0-2) claimed; newly reached ones: marks them and returns how many */
int Tcg_Milestone(const struct TcgCollection *c, unsigned set, unsigned n);
void Tcg_SetMilestone(struct TcgCollection *c, unsigned set, unsigned n);
unsigned Tcg_TakeMilestones(struct TcgCollection *c);
/* Spare copies a card can give up: beyond 4 and what the saved decks use. */
unsigned Tcg_Spare(const struct TcgCollection *c, unsigned card);
unsigned Tcg_PointValue(unsigned card);
unsigned Tcg_PointCost(unsigned card); /* at the CARD CLUB, commons and uncommons; 0: not sold there */
unsigned Tcg_ShopPrice(unsigned card); /* money at the GAME CENTER CARD SHOP; 0: not sold (holos, promos, energy) */
/* Records against card players: NULL when never played */
const struct TcgRecord *Tcg_Record(const struct TcgCollection *c, uint32_t key);
void Tcg_RecordResult(struct TcgCollection *c, uint32_t key, int won, unsigned day);
int Tcg_CollectionLoad(struct TcgCollection *c, const char *path);
/* Dev sessions must never write a collection, even when explicitly requested. */
int Tcg_CollectionSave(const struct TcgCollection *c, const char *path);
#endif
