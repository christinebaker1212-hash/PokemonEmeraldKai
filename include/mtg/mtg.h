// Moemon Kai: Magic: The Gathering duels, Forge's rules translated into C (docs/MTG_DECISIONS.md, docs/MTG_HANDOFF.md).
// The cards are Forge's own scripts (graphics/mtg/forge.pak, tools/mtg/build_pack.py), read by src/mtg/mtg_script.c.
// A duel is one plain struct (no pointers: copying it with memcpy copies the game, for the AI and for rewinds) that
// runs until it needs a decision (Mtg_Query), takes the answer (Mtg_Answer) and runs on. The same answers and seed
// always play the same duel (online duels send answers, as the Yu-Gi-Oh! side does).
#ifndef GUARD_MTG_H
#define GUARD_MTG_H

#include <stdint.h>

// ---- the card database (read-only, shared by every duel) ----
enum { MTG_W, MTG_U, MTG_B, MTG_R, MTG_G, MTG_C, MTG_MANA_KINDS };   // mana and color order (Forge's WUBRG, then colorless)
#define MTG_COLOR_BIT(c) (1u << (c))

// card types (CardType.CoreType) and supertypes
#define MTG_T_LAND         0x01
#define MTG_T_CREATURE     0x02
#define MTG_T_ARTIFACT     0x04
#define MTG_T_ENCHANTMENT  0x08
#define MTG_T_INSTANT      0x10
#define MTG_T_SORCERY      0x20
#define MTG_T_PLANESWALKER 0x40
#define MTG_T_TRIBAL       0x80
#define MTG_ST_BASIC       0x01
#define MTG_ST_LEGENDARY   0x02
#define MTG_ST_SNOW        0x04
#define MTG_ST_WORLD       0x08

// keywords the engine knows (Keyword.java's names)
enum {
    MTG_KW_FLYING, MTG_KW_REACH, MTG_KW_HASTE, MTG_KW_VIGILANCE, MTG_KW_DEFENDER, MTG_KW_FIRST_STRIKE,
    MTG_KW_DOUBLE_STRIKE, MTG_KW_TRAMPLE, MTG_KW_SHROUD, MTG_KW_FEAR, MTG_KW_FLASH, MTG_KW_LIFELINK,
    MTG_KW_COUNT
};
#define MTG_KW_BIT(k) (1u << (k))

typedef struct {
    uint8_t generic, x;               // generic mana, X symbols
    uint8_t pip[MTG_MANA_KINDS];      // colored (and {C}) symbols
} MtgManaCost;

int Mtg_ManaCostTotal(const MtgManaCost *c);   // mana value with X = 0

typedef struct MtgCardDef {
    const char *name, *oracle, *subtypes;    // subtypes: the Types line's words after the card types, space-separated
    MtgManaCost cost;
    uint8_t types, supertypes, colors;
    int8_t power, toughness;                 // -1 = a '*' (computed: not yet supported)
    uint32_t keywords;                       // MTG_KW_BIT
    uint8_t protection, landwalk;            // protection from colors (MTG_COLOR_BIT); landwalk by basic type W..G
    uint8_t intrinsicMana;                   // basic land types' {T}: add one (MTG_COLOR_BIT), Forge's intrinsic abilities
    uint8_t abilityCount;
    uint16_t abilities[8];                   // indexes into the ability table (A: lines)
    uint16_t svarFirst, svarCount;           // the card's SVar: lines
    uint8_t supported;                       // every line understood by the engine
    const char *unsupported;                 // else the first thing it doesn't know (for docs/MTG_SCOPE.md)
} MtgCardDef;

// Load the pack (graphics/mtg/forge.pak's bytes; its text is copied, the bytes can go). Returns the card count.
int Mtg_DbLoad(const void *pak, uint32_t size);
int Mtg_DbCount(void);
const MtgCardDef *Mtg_Card(int id);
int Mtg_CardByName(const char *name);       // -1 if not in the pool

// ---- a duel ----
#define MTG_PLAYERS     2
#define MTG_MAX_OBJS    320                  // cards and tokens
#define MTG_DECK_MAX    100
#define MTG_MAX_STACK   24
#define MTG_MAX_ACTIONS 160
#define MTG_MAX_COMBAT  32                   // attacker / blocker candidates
#define MTG_EVENT_RING  128
#define MTG_NO_OBJ      0xFFFF

enum { MTG_ZONE_NONE, MTG_ZONE_LIBRARY, MTG_ZONE_HAND, MTG_ZONE_BATTLEFIELD, MTG_ZONE_GRAVEYARD, MTG_ZONE_EXILE,
       MTG_ZONE_STACK };

// steps (PhaseType.java)
enum {
    MTG_STEP_UNTAP, MTG_STEP_UPKEEP, MTG_STEP_DRAW, MTG_STEP_MAIN1, MTG_STEP_COMBAT_BEGIN,
    MTG_STEP_COMBAT_ATTACKERS, MTG_STEP_COMBAT_BLOCKERS, MTG_STEP_COMBAT_FIRST_STRIKE, MTG_STEP_COMBAT_DAMAGE,
    MTG_STEP_COMBAT_END, MTG_STEP_MAIN2, MTG_STEP_END, MTG_STEP_CLEANUP, MTG_STEP_COUNT
};

typedef struct {
    uint16_t def;                 // card id
    uint8_t owner, controller, zone;
    uint8_t tapped, sick;         // sick: came under its controller's control since their turn began
    uint8_t attacking;            // declared as an attacker this combat
    uint16_t blocking;            // the attacker it blocks (MTG_NO_OBJ)
    uint8_t blocked;              // an attacker that was blocked (stays blocked if its blocker leaves)
    uint8_t dealtFirst;           // dealt first-strike damage this combat
    int16_t damage;               // marked damage
    int8_t tempPower, tempToughness;   // until end of turn (Pump)
    uint32_t tempKeywords;
    int16_t counters11;           // +1/+1 counters (-1/-1 as negative)
    uint32_t seq;                 // order within its zone (higher = newer; the library's top is the highest)
} MtgObj;

typedef struct {
    uint8_t kind;                 // MTG_TGT_*
    uint16_t id;                  // object, player or stack entry
} MtgTarget;
enum { MTG_TGT_NONE, MTG_TGT_OBJ, MTG_TGT_PLAYER, MTG_TGT_STACK };

typedef struct {
    uint8_t isSpell, controller;
    uint16_t source;              // the object (a spell is its own card, on the stack)
    uint16_t ability;             // ability table index
    int16_t x;
    MtgTarget target;
    // last known information about the target as it resolves (Forge's LKI copies)
    int16_t lkiPower, lkiToughness;
    uint8_t lkiController;
} MtgStackEntry;

typedef struct {
    int16_t life;
    uint8_t landsPlayed, lost, drewFromEmpty;
    uint8_t pool[MTG_MANA_KINDS];  // floating mana (empties at the end of each step)
    uint16_t library[MTG_DECK_MAX + 20];
    uint16_t libraryCount;
} MtgPlayer;

// what the duel waits for
enum { MTG_Q_NONE, MTG_Q_PRIORITY, MTG_Q_ATTACKERS, MTG_Q_BLOCKERS, MTG_Q_DISCARD, MTG_Q_GAME_OVER };
enum { MTG_ACT_PASS, MTG_ACT_LAND, MTG_ACT_CAST, MTG_ACT_ACTIVATE };

typedef struct {
    uint8_t kind;
    uint16_t obj, ability;
    int16_t x;
    MtgTarget target;
} MtgAction;

typedef struct {
    uint8_t kind, player;
    uint16_t actionCount;
    MtgAction actions[MTG_MAX_ACTIONS];              // MTG_Q_PRIORITY: actions[0] is always PASS
    uint8_t candidateCount;
    uint16_t candidates[MTG_MAX_COMBAT];             // ATTACKERS: who may attack; BLOCKERS: who may block; DISCARD: hand
    uint8_t attackerCount;
    uint16_t attackers[MTG_MAX_COMBAT];              // BLOCKERS: the attacking creatures
    uint8_t discard;                                 // DISCARD: how many to discard
    int8_t winner;                                   // GAME_OVER: 0/1, -1 a draw
} MtgQuery;

typedef struct {
    uint16_t action;                                 // PRIORITY: index into actions
    uint32_t attack;                                 // ATTACKERS: bit i = candidates[i] attacks
    int8_t block[MTG_MAX_COMBAT];                    // BLOCKERS: candidate i blocks attackers[block[i]] or -1
    uint8_t card;                                    // DISCARD: index into candidates
} MtgAnswer;

enum {
    MTG_EV_NONE, MTG_EV_TURN, MTG_EV_STEP, MTG_EV_DRAW, MTG_EV_LAND, MTG_EV_CAST, MTG_EV_ACTIVATE, MTG_EV_RESOLVE,
    MTG_EV_ZONE, MTG_EV_DAMAGE, MTG_EV_LIFE, MTG_EV_ATTACK, MTG_EV_BLOCK, MTG_EV_TAP, MTG_EV_COUNTERED,
    MTG_EV_FIZZLE, MTG_EV_LOSE
};
typedef struct { uint8_t kind; int16_t a, b, c; } MtgEvent;   // a, b, c by kind (mtg_game.c Mtg_Event)

typedef struct MtgDuel {
    uint32_t rng;
    uint32_t seq;
    uint16_t turn;
    uint8_t active, priority, step, passes;
    uint8_t stepStarted, startingPlayer, combatHadFirstStrike;
    uint8_t over;
    int8_t winner;
    MtgPlayer players[MTG_PLAYERS];
    uint16_t objCount;
    MtgObj objs[MTG_MAX_OBJS];
    uint8_t stackCount;
    MtgStackEntry stack[MTG_MAX_STACK];
    MtgQuery query;
    uint32_t eventCount;                             // every event so far (the ring keeps the last MTG_EVENT_RING)
    MtgEvent events[MTG_EVENT_RING];
    uint32_t hash;                                   // running hash of every event (determinism checks)
} MtgDuel;

typedef struct { uint16_t count; uint16_t cards[MTG_DECK_MAX]; } MtgDeck;

void Mtg_DuelInit(MtgDuel *d, const MtgDeck *deck0, const MtgDeck *deck1, uint32_t seed);
const MtgQuery *Mtg_Query(const MtgDuel *d);
int Mtg_Answer(MtgDuel *d, const MtgAnswer *a);      // 0 if the answer was refused (the query stays)
const MtgEvent *Mtg_EventAt(const MtgDuel *d, uint32_t n);   // n < eventCount; NULL once out of the ring

// card state as rules see it (counters, until-end-of-turn effects)
int Mtg_Power(const MtgDuel *d, int obj);
int Mtg_Toughness(const MtgDuel *d, int obj);
uint32_t Mtg_Keywords(const MtgDuel *d, int obj);

// the stand-in computer player (src/mtg/mtg_ai.c) until forge-ai's translation
void MtgAi_Answer(const MtgDuel *d, MtgAnswer *out);

#endif
