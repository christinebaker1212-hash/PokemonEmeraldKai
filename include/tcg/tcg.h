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
struct TcgCollection
{
    uint16_t owned[TCG_CARD_COUNT + 1];
    struct TcgSavedDeck decks[TCG_SAVED_DECKS];
    uint16_t packs[TCG_PACK_TYPES];
    uint32_t wins, dust, vouchers, rng;
    uint32_t owner; /* the save it belongs to (its Trainer ID): a new game starts fresh */
    uint8_t optedIn, activeDeck, fastAnimations, quiet;
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
int Tcg_CollectionLoad(struct TcgCollection *c, const char *path);
/* Dev sessions must never write a collection, even when explicitly requested. */
int Tcg_CollectionSave(const struct TcgCollection *c, const char *path);
#endif
