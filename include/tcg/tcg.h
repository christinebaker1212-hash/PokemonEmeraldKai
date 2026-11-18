#ifndef MOEMON_TCG_H
#define MOEMON_TCG_H
#include <stddef.h>
#include <stdint.h>
#define TCG_CARD_COUNT 1669  /* the first Game Boy game's 228, then Pokemon Card GB2's (tools/tcg/import_poketcg2.py), then
                              * the regional sets' 1,227 (tools/tcg/regions/import_sets.py: docs/TCG_REGIONS.md) */
#define TCG_CARD_GB_COUNT 442 /* (the two Game Boy games' cards: every one of them plays) */
#define TCG_DECK_SIZE 60
#define TCG_SAVED_DECKS 8
#define TCG_NAME_LEN 24
#define TCG_TYPES 9   /* (Darkness and Metal from Neo on: docs/TCG_REGIONS.md) */
#define TCG_ATTACKS 3 /* a Pokemon's attacks and Pokemon Power / Poke-Power / Poke-Body (3: Ho-oh, Tyranitar...) */
#define TCG_PACK_TYPES 20    /* the first game's 4 boosters, GB2's 7 (TCG_PACK_GB2), the regional sets' 9 (TCG_PACK_REGIONAL) */
#define TCG_PACK_REGIONAL 11 /* Gym Heroes, Gym Challenge (KANTO), Neo Genesis .. Destiny (JOHTO), Expedition, Aquapolis,
                              * Skyridge (HOENN, SINNOH, UNOVA): the cards' set (TCG_ORIGIN_*) less 2 */
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
    TCG_COLORLESS,
    TCG_DARKNESS,
    TCG_METAL
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
    struct TcgAttack attacks[TCG_ATTACKS];
    uint16_t id, species;
    uint8_t kind, type, rarity, set, stage, hp, retreat, weakness, resistance, level;
    /* set: its booster (TCG_SET_*: 0-3 the first game's, 4 promos, 5 energy, 6-12 GB2's); set2: the GB2 booster that
     * also holds a first-game card (255 none); dark: a Dark Pokemon (Team Rocket's) */
    uint8_t set2, dark, holo;   /* holo: printed as a holofoil (GB2's cards; the first game's: Tcg_IsHolo's list) */
    /* The regional sets' cards (docs/TCG_REGIONS.md): TCG_CF_* and the set it was printed in (TCG_ORIGIN_*; 0 the Game
     * Boy games') */
    uint16_t flags;
    uint8_t origin;
    /* a reprint's first printing (the same name and rules: one handler for all; 0 itself) */
    uint16_t canon;
};
enum
{
    TCG_CF_STADIUM = 1 << 0,
    TCG_CF_TOOL = 1 << 1,
    TCG_CF_SUPPORTER = 1 << 2,
    TCG_CF_TM = 1 << 3,           /* a Technical Machine */
    TCG_CF_BABY = 1 << 4,
    TCG_CF_EX = 1 << 5,           /* a Pokemon-ex */
    TCG_CF_RSM = 1 << 6,          /* a Rocket's Secret Machine */
    TCG_CF_GAME_CORNER = 1 << 7,  /* a Goldenrod Game Corner card */
    TCG_CF_SPECIAL = 1 << 8,      /* a special Energy card */
    TCG_CF_SHINING = 1 << 9,
    TCG_CF_LIGHT = 1 << 10,
    TCG_CF_CRYSTAL = 1 << 11,
    TCG_CF_OWNER = 1 << 12,       /* an owner's Pokemon (Brock's, Misty's...) */
    TCG_CF_BODY = 1 << 13,        /* its attacks[0] is a Poke-Body (always on), not a Poke-Power / Pokemon Power */
    TCG_CF_READY = 1 << 15        /* every rule, attack and power on it plays: only these appear (decision 2) */
};
/* TcgCard.origin: the sets after the Game Boy games */
enum
{
    TCG_ORIGIN_GB = 0,
    TCG_ORIGIN_GYM_HEROES = 13,
    TCG_ORIGIN_GYM_CHALLENGE,
    TCG_ORIGIN_NEO_GENESIS,
    TCG_ORIGIN_NEO_DISCOVERY,
    TCG_ORIGIN_NEO_REVELATION,
    TCG_ORIGIN_NEO_DESTINY,
    TCG_ORIGIN_EXPEDITION,
    TCG_ORIGIN_AQUAPOLIS,
    TCG_ORIGIN_SKYRIDGE,
    TCG_ORIGIN_WIZARDS_PROMO,
    TCG_ORIGIN_NINTENDO_PROMO
};
/* Every card, finished or not (the catalogue as data): tests and the regional sets' tools. Tcg_Card gives only the
 * cards that play (TCG_CF_READY, the Game Boy games' all) unless gTcgAllCards is set. */
const struct TcgCard *Tcg_CardAny(unsigned id);
extern int gTcgAllCards;
/* What an Energy card's rules are (the same card in two sets has one rule) and how much Energy it provides */
enum
{
    TCG_ER_BASIC,
    TCG_ER_DOUBLE_COLORLESS,
    TCG_ER_POTION,
    TCG_ER_FULL_HEAL,
    TCG_ER_RAINBOW,
    TCG_ER_RECYCLE,
    TCG_ER_DARKNESS,
    TCG_ER_METAL,      /* a non-Metal Pokemon's damage less 10 before Weakness and Resistance */
    TCG_ER_METAL_LATE, /* (Expedition's: after) */
    TCG_ER_MIRACLE,
    TCG_ER_CRYSTAL,
    TCG_ER_BOOST,
    TCG_ER_WARP,
    TCG_ER_BOUNCE,
    TCG_ER_CYCLONE,
    TCG_ER_RETRO
};
unsigned Tcg_EnergyRule(unsigned id);
/* A card's first printing with the same rules (TcgCard.canon), else itself */
unsigned Tcg_Canon(unsigned id);
unsigned Tcg_EnergyProvides(unsigned id);
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
/* The card GEN switch (OPTIONS > GAME > POKeMON CARD GEN; online the room host's): the sets in release order, each card's
 * set among them (tcg_catalog.c). GEN 0 is ALL; GEN g allows the cards of steps below g. */
#define TCG_GEN_STEPS 13
extern const char *const gTcgGenNames[TCG_GEN_STEPS];
extern const uint8_t gTcgCardGen[TCG_CARD_COUNT + 1];
/* Whether every card of a deck is inside GEN gen (0 ALL); the first card past it in *outside */
int Tcg_DeckInGen(const uint16_t cards[TCG_DECK_SIZE], unsigned gen, unsigned *outside);
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
#define TCG_SET_COUNT TCG_PACK_TYPES /* the booster sets, as the packs: COLOSSEUM ... LABORATORY, GB2's seven, the regional nine */
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
    uint32_t gcDecks; /* (bits past 7: format 8) */
    uint8_t gcSingles[TCG_GC_SINGLE_BYTES];
    struct TcgRecord records[TCG_RECORD_SLOTS];
    /* Format 7: each saved deck's ACE CARD, the card that stands for it on the screens (0: picked for it) */
    uint16_t ace[TCG_SAVED_DECKS];
    /* Format 8: the milestones' bits from 32 up (set * 3 + n - 32: the last GB2 boosters) */
    uint32_t milestonesHi;
    /* Format 9: the promo cards claimed for card milestones (duels won, packs opened...: tcg_ui.c sMilestonePromos) */
    uint32_t rewards;
    /* Format 10: each saved deck's hand tray color on the duel table (TCG_TRAY_*: the deck's main type, one type, or
     * a POKe BALL color) */
    uint8_t tray[TCG_SAVED_DECKS];
    /* Format 11: the regional boosters' milestones ((pack - TCG_PACK_REGIONAL) * 3 + n); the special trainers past
     * the first 32 beaten and promos given (bit special - 32) */
    uint32_t milestonesRegion;
    uint32_t specialBeatenHi[3], promoGivenHi[3];
};
/* The special trainers' bits (TcgCollection.specialBeaten / promoGiven and their Hi words: up to 128) */
int Tcg_SpecialBeaten(const struct TcgCollection *c, unsigned special);
void Tcg_SetSpecialBeaten(struct TcgCollection *c, unsigned special);
int Tcg_PromoGiven(const struct TcgCollection *c, unsigned special);
void Tcg_SetPromoGiven(struct TcgCollection *c, unsigned special);
/* Is a booster sold in a region (REGION_* of include/mp_region.h): today's 11 everywhere, the regional ones in theirs */
int Tcg_PackSoldIn(unsigned pack, unsigned region);
/* The set (TcgCard.set) a booster holds */
unsigned Tcg_PackSet(unsigned pack);
/* A deck's tray color (TcgCollection.tray, MtgCollection.tray): the deck's own main color, one of the game's colors
 * (TCG_TRAY_GAME + n), or a POKe BALL color (TCG_TRAY_BALL + n) */
#define TCG_TRAY_AUTO 0
#define TCG_TRAY_GAME 1
#define TCG_TRAY_BALL 0x40
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
