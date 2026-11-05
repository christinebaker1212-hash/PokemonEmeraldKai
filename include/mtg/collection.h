// Moemon Kai: a save's Magic cards and decks ("<save>.mtg", src/mtg/mtg_integration.c), the starter decks and the
// duelists' decks (src/mtg/mtg_decks.c), boosters. Plain C, no SDL: tools/mtg/test.sh tests it.
#ifndef GUARD_MTG_COLLECTION_H
#define GUARD_MTG_COLLECTION_H

#include "mtg/mtg.h"

#define MTG_COLLECTION_CARDS 6144      // card ids (the pack has 5408)
#define MTG_DECKS 8
#define MTG_DECK_NAME 16
#define MTG_DECK_MIN 40                // the game's decks: 40 cards at least (the starters' size)
#define MTG_COPIES 4                   // copies of a card in a deck, but for basic lands
#define MTG_OWNED_MAX 99
#define MTG_BOOSTER 8                  // a won duel's booster: five commons, two uncommons, an uncommon or rare

#define MTG_FLAG_CASE_SIDE 1           // the CARD CASE opens on its MAGIC side

typedef struct {
    char name[MTG_DECK_NAME];
    uint16_t count;
    uint16_t cards[MTG_DECK_MAX];
} MtgSavedDeck;

typedef struct {
    uint32_t owner, rng;
    uint8_t optedIn;                   // has a deck: Magic shows in the CARD CASE and NPCs offer it
    uint8_t flags, activeDeck, starter;
    uint16_t wins, losses;
    uint32_t beaten;                   // duelists beaten (bit per MtgDuelist)
    uint8_t owned[MTG_COLLECTION_CARDS];
    MtgSavedDeck decks[MTG_DECKS];
    int16_t ace[MTG_DECKS];            // each deck's ACE CARD (a card id), -1: the game picks one (Mtg_DeckAce)
    uint8_t tray[MTG_DECKS];           // each deck's hand tray color (TCG_TRAY_* of include/tcg/tcg.h: 0 the deck's own)
} MtgCollection;

void Mtg_CollectionInit(MtgCollection *c, uint32_t seed);
int Mtg_CollectionSave(const MtgCollection *c, const char *path);
int Mtg_CollectionLoad(MtgCollection *c, const char *path);    // 0: no file, or not one of ours
// the same in memory (the file's bytes); Write returns the size (0 if it doesn't fit)
uint32_t Mtg_CollectionWrite(const MtgCollection *c, uint8_t *out, uint32_t cap);
int Mtg_CollectionRead(MtgCollection *c, const uint8_t *in, uint32_t size);

int Mtg_IsBasicLand(int card);
int Mtg_CanUse(int card);                                       // playable, not banned
uint32_t Mtg_NameHash(const char *name);
int Mtg_CardByHash(uint32_t hash);
uint32_t Mtg_Rand(uint32_t *rng);

// a deck the game can play: MTG_DECK_MIN+, MTG_COPIES of a card but basic lands, every card usable and owned
// (basic lands are always there). Returns NULL, or what's wrong with it.
const char *Mtg_DeckProblem(const MtgCollection *c, const MtgSavedDeck *d);
int Mtg_DeckCopies(const MtgSavedDeck *d, int card);
void Mtg_DeckAdd(MtgSavedDeck *d, int card);
void Mtg_DeckRemove(MtgSavedDeck *d, int card);
void Mtg_DeckSort(MtgSavedDeck *d);                             // lands last, then by mana value and name
uint8_t Mtg_DeckColors(const MtgSavedDeck *d);
// The card that stands for a deck: its ACE CARD if chosen and still in it, else its rarest, then costliest card (not a
// basic land); -1 for an empty deck
int Mtg_DeckAce(const MtgCollection *c, int deck);
int Mtg_AutoAce(const uint16_t *cards, int count);
void Mtg_DeckToDuel(const MtgSavedDeck *d, MtgDeck *out);

// ---- recipes: the starters and the duelists' decks (src/mtg/mtg_decks.c)
typedef struct {
    const char *name, *about;
    uint8_t colors;
    const char *const *cards;          // "17 Mountain", "2 Raging Goblin"... ending with NULL
} MtgRecipe;
#define MTG_STARTERS 5
extern const MtgRecipe gMtgStarters[MTG_STARTERS];
int Mtg_RecipeDeck(const MtgRecipe *r, MtgSavedDeck *out);     // the number of cards it couldn't find (0)

typedef struct {
    const char *name, *title;          // "AUGUST", "the CARD SHOP's oldest regular"
    const char *const *cards;          // a recipe as above
    const char *hello, *win, *lose;    // their words: offering a duel, when they win, when they lose
    uint8_t level;                     // 0-3 (the shared AI levels, once forge-ai's translation is in)
} MtgDuelist;
#define MTG_DUELISTS 6
extern const MtgDuelist gMtgDuelists[MTG_DUELISTS];

// The first deck: the chosen starter in deck slot 0, its cards owned; optedIn from now on
void Mtg_GiveStarter(MtgCollection *c, unsigned starter);
// A won duel's booster: MTG_BOOSTER usable cards (no basic lands) into the collection; returns how many
int Mtg_OpenBooster(MtgCollection *c, uint16_t out[MTG_BOOSTER]);

#endif
