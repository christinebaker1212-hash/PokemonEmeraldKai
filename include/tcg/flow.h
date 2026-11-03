/* What a move asks for before the rules engine can take it (src/tcg/tcg_flow.c): which Pokemon a Trainer, a Pokemon
 * Power or an attack aims at, which cards it takes from a pile, which choice from a menu; whether a card in the hand,
 * an attack or a retreat can be used now and why not. Shared by the game's duel board (tcg_board.c, the local player
 * is always 0 there) and the phone (android cpp/tcg_jni.c, either seat): `me` is the player making the move.
 * TcgFlow is the same steps as a machine without a screen: begin a move, answer its questions, take the action. */
#ifndef MOEMON_TCG_FLOW_H
#define MOEMON_TCG_FLOW_H
#include <stddef.h>
#include "tcg/duel.h"

/* A step's kind */
enum
{
    TCG_ST_MY_SLOT,
    TCG_ST_OPP_SLOT,
    TCG_ST_PICK,
    TCG_ST_STAGE,         /* Devolution Spray: how far (a Stage 2 asks; TO STAGE 1 = 2, TO BASIC = 1 in action.attack) */
    TCG_ST_CHOICE,        /* a Pokemon Power's menu: SHIFT's types, PEEK's places (action.target) */
    TCG_ST_ATTACK_OPTION  /* an attack's menu (TcgDuel_AttackOptions): Amnesia, Metronome, Wildfire, Conversion... */
};
/* Where a pick's cards come from */
enum
{
    TCG_SRC_HAND,
    TCG_SRC_DECK,
    TCG_SRC_MY_DISCARD,
    TCG_SRC_OPP_DISCARD,
    TCG_SRC_MY_ATTACHED,
    TCG_SRC_OPP_ATTACHED,
    TCG_SRC_TOP5,
    TCG_SRC_TOP7,       /* Master Ball: the top 7 of your deck */
    TCG_SRC_TOP3_MINE,  /* Prophecy: the top 3 of a deck, to put in order */
    TCG_SRC_TOP3_THEIRS
};
/* Which Pokemon in play (slot steps) or which cards (picks) can be chosen */
enum
{
    TCG_F_ANY,
    TCG_F_OCCUPIED,
    TCG_F_BENCH,
    TCG_F_EMPTY_BENCH,
    TCG_F_DAMAGED,
    TCG_F_DAMAGED_ENERGY,
    TCG_F_HAS_ENERGY,
    TCG_F_EVOLVED,
    TCG_F_BREEDER_BASIC,
    TCG_F_POKEMON,
    TCG_F_BASIC,
    TCG_F_BASIC_ENERGY,
    TCG_F_ENERGY,
    TCG_F_TRAINER,
    TCG_F_STAGE2,
    TCG_F_EVOLVE_TARGET,
    TCG_F_BASIC_SLOT,
    TCG_F_ANY_POKEMON,
    /* Pokemon Powers */
    TCG_F_HAS_GRASS,
    TCG_F_OTHER,         /* occupied, not the first choice */
    TCG_F_WATER_POKEMON,
    TCG_F_ROOM_OTHER,    /* takes a damage counter without a Knock Out, not the first choice */
    TCG_F_DAMAGED_OTHER, /* damaged, not the Pokemon using the power */
    /* GB2's */
    TCG_F_FOSSIL,          /* a Mysterious Fossil */
    TCG_F_EVOLUTION,       /* an Evolution card */
    TCG_F_COLORLESS_EVOLUTION,
    TCG_F_DARK_EVOLUTION,
    TCG_F_GARBAGE,         /* a Pokemon or basic Energy card */
    TCG_F_HAS_FIRE_OTHER,  /* has a Fire Energy card, not the Pokemon using the power */
    TCG_F_FOSSIL_EVOLVED   /* evolved from a Mysterious Fossil */
};
struct TcgStep
{
    uint8_t kind, filter, source, min, max;
    const char *prompt;
};
/* A Trainer's steps (count 0: it asks nothing); a used Pokemon Power's (the first Pokemon chosen is action.target,
 * the second action.attack: tcg_powers.c TcgDuel_Power) */
const struct TcgStep *TcgFlow_TrainerSteps(unsigned id, int *count);
const struct TcgStep *TcgFlow_PowerSteps(unsigned power, int *count);
/* An attack's steps into out[4]: its menu first when it has one (TcgDuel_AttackOptions; prompt[64] gets the words),
 * else (or, afterOption, once the menu is answered: a->option / a->copy set) its target and what it asks for */
int TcgFlow_AttackSteps(const struct TcgDuel *g, unsigned me, const struct TcgAction *a, const struct TcgAttack *attack,
                        int afterOption, struct TcgStep out[4], char prompt[64]);
/* Filters: `a` is the move so far, `flowCard` the card being played from the hand (an evolution's) */
int TcgFlow_CardMatches(const struct TcgDuel *g, unsigned owner, unsigned physical, int filter);
int TcgFlow_SlotMatches(const struct TcgDuel *g, unsigned me, const struct TcgAction *a, unsigned flowCard, unsigned who,
                        unsigned slot, int filter);
/* Can it be used now (the rules' side: whose turn it is is the caller's), and why not */
int TcgFlow_HandPlayable(const struct TcgDuel *g, unsigned me, const struct TcgCard *c, char *reason, size_t size);
int TcgFlow_TrainerHasUse(const struct TcgDuel *g, unsigned me, unsigned id); /* would it do anything now? */
int TcgFlow_AttackReady(const struct TcgDuel *g, unsigned me, unsigned attack, char *reason, size_t size);
int TcgFlow_CanRetreat(const struct TcgDuel *g, unsigned me, char *reason, size_t size);
int TcgFlow_HasPower(const struct TcgCard *c);
int TcgFlow_HasEnergy(const struct TcgDuel *g, unsigned who, unsigned slot);
unsigned TcgFlow_EnergyValue(unsigned id); /* Double Colorless: 2 */
const char *TcgFlow_HandVerb(const struct TcgCard *c); /* ATTACH, EVOLVE, PLAY */
/* A retreat's Energy when there's nothing to choose (no cost, just enough, one kind): into a->selections, 1; else 0 */
int TcgFlow_RetreatAutoPay(const struct TcgDuel *g, unsigned me, struct TcgAction *a);
/* SHIFT's types in play / PEEK's places, as a menu (TCG_ST_CHOICE): the labels, the values (action.target), which can
 * be chosen and why not; 0 and *reason when nothing can be */
#define TCG_FLOW_MENU 8
struct TcgFlowMenu
{
    const char *label[TCG_FLOW_MENU], *why[TCG_FLOW_MENU];
    uint8_t value[TCG_FLOW_MENU], ok[TCG_FLOW_MENU];
    unsigned count;
};
unsigned TcgFlow_PowerChoices(const struct TcgDuel *g, unsigned me, unsigned slot, struct TcgFlowMenu *m,
                              const char **reason);
/* The prompt for an attack's menu */
const char *TcgFlow_AttackPrompt(const char *attackName);

/* ----------------------------------------------------------------------------------- the machine (no screen) */
enum
{
    TCG_FLOW_HAND,     /* index: the card's place in the hand */
    TCG_FLOW_ATTACK,   /* index: the attack (0 / 1) */
    TCG_FLOW_RETREAT,
    TCG_FLOW_POWER,    /* index: the Pokemon using it */
    TCG_FLOW_PROMOTE,  /* index: the Benched Pokemon */
    TCG_FLOW_DISCARD,  /* index: the Clefairy Doll / Mysterious Fossil in play */
    TCG_FLOW_PASS,     /* END TURN */
    TCG_FLOW_READY,    /* setup done */
    TCG_FLOW_CONCEDE
};
/* What it asks now */
enum
{
    TCG_Q_DONE,  /* the action is ready: TcgFlow.action */
    TCG_Q_SLOT,  /* one of `who`'s Pokemon in play: slotMask */
    TCG_Q_PICK,  /* cards from a pile: pick[pickCount] (physical, of pickOwner), pickOk; pickMin..pickMax, or
                  * Energy worth pickValue for a retreat */
    TCG_Q_MENU,  /* menu */
    TCG_Q_FAIL   /* it can't be done: error */
};
struct TcgFlow
{
    struct TcgAction action;
    unsigned me, kind, flowCard, q, who;
    const struct TcgStep *steps;
    int nsteps, step;
    struct TcgStep attackSteps[4];
    char attackPrompt[64], error[96];
    const char *prompt;
    uint8_t slotMask;
    uint8_t pick[TCG_DECK_SIZE], pickOk[TCG_DECK_SIZE], pickCount, pickOwner, pickMin, pickMax, pickValue, pickSource;
    struct TcgFlowMenu menu;
};
/* Begin a move; then answer while q is SLOT / PICK / MENU. Each returns q. */
unsigned TcgFlow_Begin(struct TcgFlow *f, const struct TcgDuel *g, unsigned me, unsigned kind, unsigned index);
unsigned TcgFlow_Slot(struct TcgFlow *f, const struct TcgDuel *g, unsigned slot);
unsigned TcgFlow_Pick(struct TcgFlow *f, const struct TcgDuel *g, const uint8_t *chosen, unsigned count);
unsigned TcgFlow_Menu(struct TcgFlow *f, const struct TcgDuel *g, unsigned value);
/* Whose move it is now (the one promoting, or the one whose turn it is), TCG_NONE when it's over */
unsigned TcgFlow_Actor(const struct TcgDuel *g);
#endif
