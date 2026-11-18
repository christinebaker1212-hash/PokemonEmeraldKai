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
    TCG_SRC_TOP3_THEIRS,
    TCG_SRC_OPP_BENCH,  /* Gigashock: their Benched Pokemon (each one's top card) */
    TCG_SRC_DEFENDER_ATTACHED, /* Magnetic Lines: the Defending Pokemon's cards (whatever the target) */
    TCG_SRC_MY_FIELD_ATTACHED, /* Burning Fire: every one of your Pokemon's cards, the Active first */
    TCG_SRC_OPP_HAND,          /* Rocket's Sneak Attack: their hand, looked at (min drops to what can be chosen) */
    TCG_SRC_HAND_TOP2,         /* Mary: your hand and the 2 cards you'll draw (the top 2 of your deck) */
    TCG_SRC_BOTTOM4,           /* Underground Expedition: the bottom 4 of your deck */
    TCG_SRC_TOP2,              /* Radio Tower: the top 2 of your deck, only looked at */
    TCG_SRC_MY_BENCH_ATTACHED, /* Energy Connect: your Benched Pokemon's cards */
    TCG_SRC_TOP3_PICK          /* Flipper Stroke: the top 3 of your deck, picked from as any pile */
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
    TCG_F_FOSSIL_EVOLVED,  /* evolved from a Mysterious Fossil */
    /* an attack's own pick (TcgDuel_AttackCardOk): the attacker's first / second attack */
    TCG_F_ATTACK_CARD0,
    TCG_F_ATTACK_CARD1,
    TCG_F_FIRE_ENERGY,
    /* the regional sets' Trainers (src/tcg/tcg_rtrainers_flow.inc) */
    TCG_F_NO_TOOL,              /* (slot) a Pokemon without a Pokemon Tool */
    TCG_F_OTHER_MINE,           /* (slot) another of yours than the first chosen: the action's target */
    TCG_F_HAS_BASIC_ENERGY,     /* (slot) with a basic Energy card */
    TCG_F_DAMAGED_BASIC_ENERGY, /* (slot) damaged, with a basic Energy card */
    TCG_F_WATER_ENERGY,         /* a Water Energy card */
    TCG_F_BASIC_NOT_BABY,       /* a Basic Pokemon card, not a Baby */
    TCG_F_POKEMON_CARD,         /* any Pokemon card */
    TCG_F_TM_TOOL,              /* a Technical Machine or Pokemon Tool */
    TCG_F_SUPPORTER_STADIUM,
    TCG_F_BALL,                 /* a Trainer with Ball in its name */
    TCG_F_UNOWN,                /* an Unown */
    TCG_F_LIGHTNING_ENERGY,     /* a Lightning Energy card */
    TCG_F_OTHER_HOLDER,         /* (slot) occupied, not the one the card picked is attached to */
    TCG_F_SAME_TYPE_PICK,       /* (slot) of the type of the card picked first (Friend Ball) */
    TCG_F_LATER_STAGE,          /* an Evolution card of a later Stage of one of yours in play, not a Baby */
    TCG_F_OMANYTE_KABUTO,       /* an Omanyte or Kabuto card */
    TCG_F_BASIC_TYPE_PICK,      /* a Basic Pokemon card of the type of the card picked first (Apricorn Forest) */
    TCG_F_FIRE_POKEMON,         /* (slot) a Fire Pokemon */
    TCG_F_BENCH_CLEAN,          /* (slot) a Benched Pokemon with no damage counters */
    TCG_F_GRASS_ENERGY,         /* a Grass Energy card */
    TCG_F_SUPPORTER,            /* a Supporter */
    TCG_F_BABY,                 /* a Baby Pokemon card */
    TCG_F_HAS_POWER,            /* (slot) a Pokemon with a Pokemon Power, on the Bench */
    TCG_F_SPECIAL_ENERGY,       /* a Special Energy card */
    TCG_F_TM_FITS,
    TCG_F_EVOLVES_FROM_USER,    /* a card that evolves from the Pokemon using the power (action.slot) */
    TCG_F_EVOLVES_FROM_ACTIVE,  /* a card that evolves from the Active Pokemon */
    TCG_F_EVOLVES_FROM_TARGET,  /* a card that evolves from the Pokemon of yours chosen (action.target) */
    TCG_F_SAME_NAME_BASIC,      /* a Basic Pokemon card named as your Active Pokemon (the attacker) */
    TCG_F_METAL_BASIC,          /* a Metal Basic Pokemon card */
    TCG_F_FOSSIL_KID,           /* a card that evolves from Mysterious Fossil */
    TCG_F_GRASS_ENERGY_OTHER,   /* a Grass Energy card attached to another of yours than the power's (Soak Up) */
    TCG_F_MAGNET_ENERGY,        /* an Energy card attached to a Magnemite, Magneton or Dark Magneton of yours */
    TCG_F_MAGNET_OTHER,         /* (slot) a Magnemite, Magneton or Dark Magneton, not the holder of the card picked */
    TCG_F_UNOWN_SLOT,           /* (slot) a Pokemon with Unown in its name */
    TCG_F_UNOWN_DAMAGED_OTHER,  /* (slot) a damaged Pokemon with Unown in its name, not the power's */
    TCG_F_PSYCHIC_ENERGY,       /* a Psychic Energy card */
    TCG_F_POKEMON_OTHER_NAME,   /* a Pokemon card not named as your Active Pokemon (Messenger) */
    TCG_F_GRASS_BENCH,          /* (slot) a Benched Grass Pokemon */
    TCG_F_TOOL,                 /* a Pokemon Tool */
    TCG_F_LIGHTNING_POKEMON,    /* (slot) a Lightning Pokemon */
    TCG_F_SHINING_LIGHT,        /* (slot) a Shining or Light Pokemon (Miracle Energy) */
    TCG_F_STADIUM,              /* a Stadium card */
    TCG_F_ON_DATA_SLOT,         /* an Energy card attached to the Pokemon of yours in slot choiceData */
    TCG_F_EVOLVES_FROM_DATA,    /* a card that evolves from the Pokemon of yours in slot choiceData */
    TCG_F_EEVEE_TYPE,           /* a card that evolves from Eevee in slot choiceData & 15, of the type choiceData >> 4 */
    TCG_F_SPORE_SLOT,           /* (slot) a Hoppip or Skiploom not chosen yet (choiceData's bits) */
    TCG_F_ACTIVE_NO_TOOL,       /* (slot) the Active Pokemon, without a Pokemon Tool */              /* (slot) a Pokemon the Technical Machine being played may go on (TcgDuel_TmFits) */
    /* "with Brock in its name" and the like (+ TCG_OWNER_*): a Pokemon card; one of yours in play (slot); another
     * one of yours than the first chosen (slot: the action's target) */
    TCG_F_OWNER_CARD = 100,
    TCG_F_OWNER_SLOT = 110,
    TCG_F_OWNER_OTHER = 120,
    TCG_F_OWNER_BASIC = 130, /* (+ TCG_OWNER_*) a Basic Pokemon card with that name in it */
    TCG_F_NAMED = 140        /* (+ a name group: TcgFlow_InNameGroup) a card of one of the group's names */
};
enum
{
    TCG_OWNER_BROCK,
    TCG_OWNER_MISTY,
    TCG_OWNER_ERIKA,
    TCG_OWNER_SURGE,
    TCG_OWNER_SABRINA,
    TCG_OWNER_BLAINE,
    TCG_OWNER_GIOVANNI,
    TCG_OWNER_KOGA,
    TCG_OWNERS
};
/* A pick's min: as many as there are, up to its max */
#define TCG_MIN_ALL 99
/* The owner's word in a Pokemon's name ("Brock", "Lt. Surge"...) */
const char *TcgFlow_OwnerWord(unsigned owner);
/* Whether a card's name is one of a name group's (the regional attacks that name cards: "a card named Gyarados, Dark
 * Gyarados, or Shining Gyarados"): 0 the Gyarados, 1 the Tentacool line and Misty's, 2 Sabrina's Gastly line, 3 the
 * Omanyte and Kabuto lines, 4 Giovanni's Nidoking, 5 Murkrow */
#define TCG_NAME_GROUPS 7 /* (6: Magnemite, Magneton, Dark Magneton) */
int TcgFlow_InNameGroup(const char *name, unsigned group);

struct TcgStep
{
    uint8_t kind, filter, source, min, max;
    const char *prompt;
};
/* Does that slot step aim at the action's target (the opponent's Pokemon, or a Trainer's second Pokemon of yours)? */
int TcgFlow_StepIsTarget(const struct TcgStep *st);
/* A used power's slot step: does it choose the action's target (the first Pokemon chosen) or its attack field (the
 * second)? */
int TcgFlow_PowerSlotIsTarget(const struct TcgStep *steps, int step);
/* A Trainer's steps (count 0: it asks nothing); a used Pokemon Power's (the first Pokemon chosen is action.target,
 * the second action.attack: tcg_powers.c TcgDuel_Power) */
const struct TcgStep *TcgFlow_TrainerSteps(unsigned id, int *count);
const struct TcgStep *TcgFlow_PowerSteps(unsigned power, int *count);
/* What using the Stadium in play asks for (its canon id) */
const struct TcgStep *TcgFlow_StadiumSteps(unsigned stadium, int *count);
/* What the pending choice asks (phase TCG_CHOICE: TcgDuel.choiceKind) */
const struct TcgStep *TcgFlow_ChoiceSteps(const struct TcgDuel *g, int *count);
/* A Pokemon played from the hand whose power asks when it comes into play (Sneak Attack, Power of Darkness: target;
 * Reel In, Summon Minions: selections), after its space; the action gets TCG_PLAY_CHOSEN */
const struct TcgStep *TcgFlow_PlaySteps(unsigned cardId, int *count);
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
int TcgFlow_NeedsDeck(unsigned id);  /* a Trainer that takes from the deck (none with an empty one) */
int TcgFlow_TrainerHasUse(const struct TcgDuel *g, unsigned me, unsigned id); /* would it do anything now? */
int TcgFlow_AttackReady(const struct TcgDuel *g, unsigned me, unsigned attack, char *reason, size_t size);
int TcgFlow_CanRetreat(const struct TcgDuel *g, unsigned me, char *reason, size_t size);
int TcgFlow_HasPower(const struct TcgCard *c);
/* A pick's card filter where it depends on what was picked before (the action so far) */
int TcgFlow_PickMatches(const struct TcgDuel *g, unsigned me, const struct TcgAction *a, unsigned owner, unsigned physical,
                        int filter);
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
/* A Trainer's menu (TCG_ST_CHOICE: the value is action.target): the regional Erika's draw */
unsigned TcgFlow_TrainerChoices(const struct TcgDuel *g, unsigned me, unsigned cardId, struct TcgFlowMenu *m,
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
    TCG_FLOW_CONCEDE,
    TCG_FLOW_STADIUM,  /* the Stadium in play used (TCG_USE_STADIUM) */
    TCG_FLOW_CHOICE    /* the choice a card's effect asks (phase TCG_CHOICE: TCG_CHOOSE) */
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
/* An attack's sound (an SE_ number, constants/songs.h) and its sparks' color (0: the type's) by its animation */
unsigned TcgFlow_AttackSound(const char *anim, unsigned *color);

#endif
