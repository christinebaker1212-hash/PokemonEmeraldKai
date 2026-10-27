#ifndef MOEMON_TCG_H
#define MOEMON_TCG_H
#include <stddef.h>
#include <stdint.h>
#define TCG_CARD_COUNT 228
#define TCG_DECK_SIZE 60
#define TCG_SAVED_DECKS 8
#define TCG_NAME_LEN 24
#define TCG_TYPES 7
#define TCG_PACK_TYPES 4
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
};
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
#define TCG_SET_COUNT 4     /* COLOSSEUM, EVOLUTION, MYSTERY, LABORATORY: the booster sets */
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
    uint32_t milestones;                /* bit set * 3 + n: 25 / 50 / 75 % of a set rewarded */
    uint32_t packsOpened, onlineWins, trades, flags;
};
void Tcg_CollectionInit(struct TcgCollection *c, uint32_t seed);
int Tcg_ChooseStarter(struct TcgCollection *c, unsigned starter);
int Tcg_OpenPack(struct TcgCollection *c, unsigned pack, uint16_t result[10]);
int Tcg_AwardPack(struct TcgCollection *c, unsigned pack);
int Tcg_ExchangeDuplicates(struct TcgCollection *c, unsigned card, unsigned count);
int Tcg_CraftCard(struct TcgCollection *c, unsigned card);
int Tcg_DeckAdd(struct TcgCollection *c, unsigned deck, unsigned card, char *reason, size_t size);
int Tcg_DeckRemove(struct TcgCollection *c, unsigned deck, unsigned card);
int Tcg_DeckCopy(struct TcgCollection *c, unsigned from, unsigned to);
/* Today's full reward from an opponent (FF8: one card of their deck): once a day each. */
int Tcg_DailyAvailable(const struct TcgCollection *c, uint32_t key, unsigned day);
void Tcg_DailyTake(struct TcgCollection *c, uint32_t key, unsigned day);
/* Distinct cards owned of a booster set (0-3), or of all 228 (set TCG_SET_COUNT). */
unsigned Tcg_SetOwned(const struct TcgCollection *c, unsigned set, unsigned *total);
/* Set milestones newly reached (25 / 50 / 75 %): marks them and returns their bits. */
uint32_t Tcg_TakeMilestones(struct TcgCollection *c);
/* Spare copies a card can give up: beyond 4 and what the saved decks use. */
unsigned Tcg_Spare(const struct TcgCollection *c, unsigned card);
unsigned Tcg_PointValue(unsigned card);
unsigned Tcg_PointCost(unsigned card); /* 0: can't be bought (promos, energy) */
int Tcg_CollectionLoad(struct TcgCollection *c, const char *path);
/* Dev sessions must never write a collection, even when explicitly requested. */
int Tcg_CollectionSave(const struct TcgCollection *c, const char *path);
#endif
