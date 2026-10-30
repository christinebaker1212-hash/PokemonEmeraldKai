#ifndef MOEMON_TCG_DUEL_H
#define MOEMON_TCG_DUEL_H
#include "tcg/tcg.h"
#define TCG_FIELD_SIZE 6
#define TCG_LOG_LINES 12
#define TCG_LOG_WIDTH 160
/* Pile/attachment entries identify one of the original sixty physical cards.
 * They are not catalogue IDs. This makes conservation and duplicate ownership
 * testable after every action, including evolutions and double-colourless costs. */
struct TcgFieldCard
{
    uint8_t stack[4], stackSize, attached[TCG_DECK_SIZE], attachedSize;
    uint16_t damage, enteredTurn;
    uint8_t condition, poison, noPrize, barrier, noRetreat, noAttack;
    uint8_t plusPower, defender, powerUsed;
    /* Original substatus: guard (on itself, until its owner's next turn:
     * TCG_GUARD_*), weaken (on a Defending Pokemon's next attacks:
     * TCG_WEAKEN_*), charged (Swords Dance / Focus Energy: 1 set, 2 this
     * turn doubles) and usedOnce (Leek Slap). barrier 1 stops damage,
     * 2 every effect of attacks. noAttack 1: it can't attack (Bone Attack);
     * 2: it can't attack the Pokemon that did it (Leer, Tail Wag). For that and
     * for weaken 10 / 20 (Growl, Pounce, Snivel), effectBy is the card that
     * did it (physical + 1, the top of its stack): "benching or evolving
     * either Pokemon ends this effect", so it only counts while that card is
     * the opponent's Active. */
    uint8_t guard, weaken, charged, usedOnce, effectBy;
    /* Pokemon Powers for the rest of the turn: burn (Charizard's Energy Burn: all its
     * Energy is Fire) and shift (Venomoth's Shift: its type + 1, 0 = its own). */
    uint8_t burn, shift;
    /* Amnesia (disabled: the attack it can't use next turn, + 1), Destiny Bond (until its owner's next turn),
     * Conversion 1 / 2 (weakAs / resistAs: the type + 1 its Weakness / Resistance became, until it leaves the
     * Active spot), and what the opponent's last attack did to it, for Mirror Move (mirror*, kept through its
     * owner's next turn; mirrorEffect TCG_MIRROR_*, mirrorWeak a Weakness type + 1) */
    uint8_t disabled, destinyBond, weakAs, resistAs;
    uint16_t mirrorDamage;
    uint8_t mirrorCondition, mirrorPoison, mirrorWeaken, mirrorNoAttack, mirrorNoRetreat, mirrorEffect, mirrorArg,
        mirrorWeak;
    /* Pokemon Card GB2's (turn numbers: TcgDuel.turn, so they end by themselves): lockTurn, the owner's turn its
     * attack lockAttack (+ 1; 3 = both, and it can't retreat: Drill Dive) can't be used (Slashing Strike, Corrosive
     * Liquid, Focused One Shot's tails); powerOffUntil, the last turn its Pokemon Power is off (Stare); food,
     * Hungry Snorlax's Food counters; mirrorShell, Dark Wartortle's Mirror Shell (the opponent's next turn);
     * charge3, Water Power's tripling (with charged); fossilBoost, Prehistoric Dream's +10 this turn */
    uint16_t lockTurn, powerOffUntil;
    uint8_t lockAttack, food, mirrorShell, charge3, fossilBoost;
};
enum
{
    TCG_MIRROR_NONE,
    TCG_MIRROR_AMNESIA,       /* mirrorArg: the attack it was used on */
    TCG_MIRROR_DISCARD_ENERGY /* Hyper Beam and the like */
};
enum
{
    TCG_GUARD_NONE,
    TCG_GUARD_REDUCE_10,
    TCG_GUARD_REDUCE_20,
    TCG_GUARD_UNDER_40,
    TCG_GUARD_HALVE,
    /* GB2's */
    TCG_GUARD_UNDER_30,      /* Harden: 20 or less is prevented */
    TCG_GUARD_DOUBLE,        /* Double-Edged Pincers: damage to it is doubled */
    TCG_GUARD_BLINK,         /* Blink: a coin, heads prevents the damage */
    TCG_GUARD_REDUCE_10_LATE /* Expand: 10 less after Weakness and Resistance */
};
enum
{
    TCG_WEAKEN_NONE,
    TCG_WEAKEN_10,
    TCG_WEAKEN_20,
    TCG_WEAKEN_SMOKESCREEN
};
enum TcgCondition
{
    TCG_HEALTHY,
    TCG_ASLEEP,
    TCG_PARALYZED,
    TCG_CONFUSED
};
enum TcgPhase
{
    TCG_SETUP,
    TCG_TURN,
    TCG_PROMOTION,
    TCG_FINISHED
};
enum TcgActionType
{
    TCG_PLAY_BASIC,
    TCG_EVOLVE,
    TCG_ATTACH,
    TCG_ATTACK,
    TCG_RETREAT,
    TCG_PLAY_TRAINER,
    TCG_USE_POWER,
    TCG_PASS,
    TCG_PROMOTE,
    TCG_FINISH_SETUP,
    TCG_CONCEDE,
    TCG_DISCARD_DOLL /* Clefairy Doll / Mysterious Fossil in slot leaves play (their text) */
};
/* Pokemon Powers (src/tcg/tcg_powers.c), by the card's power. */
enum TcgPowerId
{
    TCG_POWER_NONE,
    /* Always on */
    TCG_POWER_TOXIC_GAS,
    TCG_POWER_CLAIRVOYANCE,
    TCG_POWER_STRIKES_BACK,
    TCG_POWER_KABUTO_ARMOR,
    TCG_POWER_PREHISTORIC,
    TCG_POWER_TRANSPARENCY,
    TCG_POWER_INVISIBLE_WALL,
    TCG_POWER_NEUTRALIZING_SHIELD,
    TCG_POWER_RETREAT_AID,
    TCG_POWER_THICK_SKINNED,
    /* (GB2's) */
    TCG_POWER_GREEN_SHIELD,
    TCG_POWER_HAY_FEVER,
    TCG_POWER_STICKY_GOO,
    TCG_POWER_FINAL_BEAM,
    TCG_POWER_AURORA_VEIL,
    TCG_POWER_GO_UNDERGROUND,
    TCG_POWER_SINKHOLE,
    TCG_POWER_FRENZY,
    TCG_POWER_PUPPET_MASTER,
    TCG_POWER_DAMPENING_SHIELD,
    TCG_POWER_DARK_WAVE,
    TCG_POWER_DARKNESS_VEIL,
    TCG_POWER_SNORLAX_GUARD,
    TCG_POWER_VINE_PULL,
    /* When it's put into play */
    TCG_POWER_FIREGIVER,
    TCG_POWER_QUICKFREEZE,
    TCG_POWER_PEAL_OF_THUNDER,
    TCG_POWER_HEALING_WIND,
    /* (GB2's: played from the hand) */
    TCG_POWER_SNEAK_ATTACK,
    TCG_POWER_REEL_IN,
    TCG_POWER_POWER_OF_DARKNESS,
    TCG_POWER_SUMMON_MINIONS,
    /* Used during your turn (TCG_USE_POWER) */
    TCG_POWER_SOLAR_POWER,
    TCG_POWER_ENERGY_TRANS,
    TCG_POWER_HEAL,
    TCG_POWER_SHIFT,
    TCG_POWER_ENERGY_BURN,
    TCG_POWER_RAIN_DANCE,
    TCG_POWER_COWARDICE,
    TCG_POWER_PEEK,
    TCG_POWER_DAMAGE_SWAP,
    TCG_POWER_STRANGE_BEHAVIOR,
    TCG_POWER_CURSE,
    TCG_POWER_STEP_IN,
    /* (GB2's) target: Gather Fire's Pokemon with the Fire Energy, Fossilize's Pokemon; selections[0]: Matter
     * Exchange's card from the hand, Evolutionary Light's from the deck, Special Delivery's card put back
     * (none: the card drawn) */
    TCG_POWER_STIFFEN,
    TCG_POWER_POLLEN_STENCH,
    TCG_POWER_POISON_MIST,
    TCG_POWER_GATHER_FIRE,
    TCG_POWER_REBIRTH,
    TCG_POWER_PREHISTORIC_DREAM,
    TCG_POWER_FOSSIL_GUIDANCE,
    TCG_POWER_FOSSILIZE,
    TCG_POWER_MATTER_EXCHANGE,
    TCG_POWER_PLAY_TRICKS,
    TCG_POWER_LONG_DISTANCE_HYPNOSIS,
    TCG_POWER_TRICKERY,
    TCG_POWER_HELPING_HAND,
    TCG_POWER_EVOLUTIONARY_LIGHT,
    TCG_POWER_SPECIAL_DELIVERY,
    TCG_POWER_COUNT
};
#define TCG_POWER_FIRST_USED TCG_POWER_SOLAR_POWER
enum { TCG_IMMUNE_NO, TCG_IMMUNE_YES, TCG_IMMUNE_UNDECIDED };
/* Peek: what to look at (TcgAction.target) */
enum
{
    TCG_PEEK_MY_DECK,
    TCG_PEEK_THEIR_DECK,
    TCG_PEEK_THEIR_HAND,
    TCG_PEEK_MY_PRIZE,
    TCG_PEEK_THEIR_PRIZE
};
enum TcgResult
{
    TCG_OK = 1,
    TCG_INVALID = 0,
    TCG_UNSUPPORTED = -1
};
struct TcgPlayer
{
    uint16_t cards[TCG_DECK_SIZE];
    uint8_t deck[TCG_DECK_SIZE], hand[TCG_DECK_SIZE], discard[TCG_DECK_SIZE], prizes[6];
    uint8_t deckSize, handSize, discardSize, prizeSize;
    struct TcgFieldCard field[TCG_FIELD_SIZE];
    uint8_t energyPlayed, confusionRetreatFailed, setupDone, retreated, noTrainers;
    uint16_t turnsTaken;
    /* GB2's, as turn numbers: skipDrawTurn, the turn this player doesn't draw (Perplex); poisonMistUntil, Poison
     * Mist's 20 a turn (theirs too) until then; strangeBeamUntil, their damaged Water Pokemon have no Weakness;
     * noTrainersTurn, their turn without Trainers (Spookify). prizesUp: their Prizes face up (Here Comes Team
     * Rocket!); poisonNext: the next Active they promote is Poisoned (Gas Explosion) */
    uint16_t skipDrawTurn, poisonMistUntil, strangeBeamUntil, noTrainersTurn;
    uint8_t prizesUp, poisonNext;
    uint16_t attachmentExpires[TCG_DECK_SIZE];
};
/* Presentation events: what happened, in order, so the duel screen can show
 * it (coin flips, damage, Knock Outs...). They never carry a hidden card id:
 * draws are counts, and only face-up cards are named. Not part of the rules
 * hash; the ring keeps the last TCG_EVENT_RING events. */
enum TcgEventType
{
    TCG_EV_NONE,
    TCG_EV_TURN,         /* player's turn begins */
    TCG_EV_DRAW,         /* player draws value cards */
    TCG_EV_ATTACK,       /* player's Active uses attack slot (0/1) of card */
    TCG_EV_COIN,         /* player flips a coin: value 1 = heads */
    TCG_EV_DAMAGE,       /* player/slot takes value damage, flags TCG_DMG_* */
    TCG_EV_HEAL,         /* player/slot loses value damage */
    TCG_EV_STATUS,       /* player/slot condition = value, poison = flags */
    TCG_EV_KO,           /* player/slot card is Knocked Out */
    TCG_EV_PRIZE,        /* player takes a Prize card */
    TCG_EV_TRAINER,      /* player plays Trainer card */
    TCG_EV_PLAY,         /* player puts Basic card in slot */
    TCG_EV_EVOLVE,       /* player evolves slot into card */
    TCG_EV_ATTACH,       /* player attaches card to slot */
    TCG_EV_RETREAT,      /* player's Active swaps with Bench slot */
    TCG_EV_SWITCH,       /* player's Active swaps with Bench slot (Trainer) */
    TCG_EV_PROMOTE,      /* player promotes Bench slot */
    TCG_EV_NO_DAMAGE,    /* player/slot is protected from the attack */
    TCG_EV_SETUP_DONE,   /* player has placed their Pokemon */
    TCG_EV_SUDDEN_DEATH, /* the duel restarts with one Prize */
    TCG_EV_WIN,          /* player wins; value: 0 prizes, 1 no Pokemon, 2 deck out, 3 concede */
    TCG_EV_RETREAT_FAIL, /* player's confused Active failed to retreat */
    TCG_EV_POWER,        /* player's slot (card) uses or triggers its Pokemon Power: value TCG_POWER_* */
    TCG_EV_PEEK,         /* player looked at card with Peek: value TCG_PEEK_* (the one event naming a
                          * hidden card: the duel screen shows it to that player alone) */
    TCG_EV_DEVOLVE,      /* player/slot devolved (Devolution Beam): the top card went to the hand */
    TCG_EV_TO_HAND,      /* player's Active and its cards went back to the hand (Hurricane) */
    TCG_EV_MORPH,        /* player/slot became card (Ditto's Morph) */
    TCG_EV_DECK_CHANGED  /* player's deck: value cards discarded from it (Wildfire), or 0 shuffled / looked at */
};
enum
{
    TCG_DMG_WEAK = 1,
    TCG_DMG_RESIST = 2,
    TCG_DMG_POISON = 4,
    TCG_DMG_SELF = 8
};
struct TcgEvent
{
    uint8_t type, player, slot, flags;
    uint16_t value, card;
};
#define TCG_EVENT_RING 64
struct TcgDuel
{
    struct TcgPlayer players[2];
    uint32_t rng;
    uint16_t turn;
    uint8_t current, first, phase, winner, prizeCount, promotePlayer, rewardClaimed, needsTurnStart;
    /* An attack made the defender's owner pick a Benched Pokemon to switch
     * in (Whirlwind): that choice is a TCG_PROMOTION with the Active kept. */
    uint8_t switchPending, endAfterSwitch;
    /* Within one attack only, always 0 between actions: Sonicboom ignores
     * Weakness/Resistance, a failed coin cancels the rest, a coin flipped
     * before the damage decides an effect after it. */
    uint8_t noWeakRes, attackFailed, coinResult;
    /* (also within one attack) nothing it does reaches the Defending Pokemon: Agility and
     * the like, Neutralizing Shield, a heads for Transparency */
    uint8_t foeImmune; /* TCG_IMMUNE_* */
    /* (GB2's, within one attack: Swift, untouched by any effect on the Defending Pokemon; a Pokemon Power's
     * bonus to it, Dark Wave and Prehistoric Dream) */
    uint8_t swift, bonus;
    /* GB2's: Goop Gas Attack, every Pokemon Power off until the end of this turn (TcgDuel.turn); fromHand: the
     * Pokemon being put into play comes from the hand (Sneak Attack, Reel In...) */
    uint16_t goopGasUntil;
    uint8_t fromHand;
    /* a Trainer ended the turn (Computer Error) */
    uint8_t turnOver;
    char error[TCG_LOG_WIDTH];
    char log[TCG_LOG_LINES][TCG_LOG_WIDTH];
    unsigned logCount;
    uint16_t animationCard;
    uint8_t animationKind;
    uint32_t eventSeq;
    struct TcgEvent events[TCG_EVENT_RING];
};
struct TcgAction
{
    uint8_t type, hand, slot, attack, target;
    /* An attack's choice from a list (TcgDuel_AttackOptions): Amnesia's attack, Wildfire's count, Conversion's
     * type, Prophecy's and Devolution Beam's side (0 yours, 1 theirs); copy: Metronome's attack + 1 */
    uint8_t option, copy;
    /* Caller-selected physical cards used for discards/search/retrieval.
     * Selections are validated; no hidden target is silently chosen for a player. */
    uint8_t selections[TCG_DECK_SIZE], selectionCount;
};
int TcgDuel_Init(struct TcgDuel *g, const uint16_t a[60], const uint16_t b[60], uint32_t seed, unsigned prizes);
int TcgDuel_Apply(struct TcgDuel *g, const struct TcgAction *action);
int TcgDuel_Conserved(const struct TcgDuel *g);
const struct TcgCard *TcgDuel_Field(const struct TcgDuel *g, unsigned player, unsigned slot);
unsigned TcgDuel_Energy(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned type);
int TcgDuel_CanAttack(const struct TcgDuel *g, unsigned player, unsigned attack);
/* Is the effect on this Pokemon tied to its opponent's Active (effectBy) still in force? */
int TcgDuel_BoundEffect(const struct TcgDuel *g, unsigned player, unsigned slot);
/* Leer / Tail Wag / Bone Attack: the Active Pokemon can't attack now. */
int TcgDuel_NoAttack(const struct TcgDuel *g, unsigned player);
int TcgDuel_EffectSupported(const char *name);
int TcgDuel_EffectTouchesDefender(const struct TcgAttack *attack);
/* Why an attack's effect can't be used now (NULL = it can), e.g. Dream Eater. */
const char *TcgDuel_EffectBlock(const struct TcgDuel *g, unsigned player, const struct TcgAttack *attack);
/* 0: no choice. 1: the attacker picks one of the opponent's Benched Pokemon
 * (action.target), 2: one of their own Benched (action.target), 3: any of the opponent's Pokemon (0-5),
 * 4: any of their own (0-5). */
unsigned TcgDuel_AttackChoice(const struct TcgAttack *attack, int *required);
/* What to ask for that choice (NULL: the usual words) */
const char *TcgDuel_AttackTargetPrompt(const struct TcgAttack *attack);
/* Big Snore: used only while Asleep; Mini-Metronome: Metronome on heads */
int TcgDuel_SleepAttack(const struct TcgAttack *attack);
int TcgDuel_MetronomeHeads(const struct TcgAttack *attack);
/* Can the Active Pokemon attack at all now (its conditions, Leer, Slashing Strike...)? */
int TcgDuel_CanAttackNow(const struct TcgDuel *g, unsigned player, unsigned attack);
/* The card whose attacks the Active Pokemon uses (its own; Hypno's for a Clefairy Doll: Puppet Master) */
const struct TcgCard *TcgDuel_Attacker(const struct TcgDuel *g, unsigned player);
/* Attacks that ask for a choice from a list (TcgAction.option): the labels and which can be picked, and how many
 * (0: none). Metronome lists the Defending Pokemon's attacks for TcgAction.copy instead (TcgDuel_IsMetronome). */
#define TCG_MAX_OPTIONS 8
unsigned TcgDuel_AttackOptions(const struct TcgDuel *g, unsigned player, const struct TcgAttack *attack,
                               const char *labels[TCG_MAX_OPTIONS], uint8_t ok[TCG_MAX_OPTIONS]);
/* What else an attack asks for after its option (TCG_ASK_*) */
enum
{
    TCG_ASK_NONE,
    TCG_ASK_DISCARD_TRAINER, /* Scavenge: selections[0], a Trainer in your discard pile */
    TCG_ASK_ORDER_TOP3,      /* Prophecy: selections, the top cards of that deck in their new order (first on top) */
    TCG_ASK_EVOLVED,         /* Devolution Beam: target, an evolved Pokemon on the option's side */
    TCG_ASK_DECK_TRAINER     /* Errand-Running: selections[0], a Trainer in your deck (taken on heads) */
};
unsigned TcgDuel_AttackAsk(const struct TcgAttack *attack);
int TcgDuel_IsMetronome(const struct TcgAttack *attack);
/* The attack Metronome copies (copy = the Defending Pokemon's attack + 1), NULL if it can't */
const struct TcgAttack *TcgDuel_MetronomeAttack(const struct TcgDuel *g, unsigned player, unsigned copy);
/* The AI's picks for those choices (option, copy, selections, target) */
void TcgDuel_AiAttackChoices(const struct TcgDuel *g, struct TcgAction *a);
/* Weakness / Resistance now (Conversion 1 / 2), TCG_NONE for none */
unsigned TcgDuel_Weakness(const struct TcgDuel *g, unsigned player, unsigned slot);
unsigned TcgDuel_Resistance(const struct TcgDuel *g, unsigned player, unsigned slot);
/* Damage from an effect to any Pokemon in play: no Weakness/Resistance;
 * Defender, and for an Active Pokemon PlusPower and substatus, apply. */
void TcgDuel_EffectDamage(struct TcgDuel *g, unsigned player, unsigned slot, int amount, unsigned flags);
void TcgDuel_SelfDamage(struct TcgDuel *g, int amount);
int TcgDuel_AiAction(const struct TcgDuel *g, struct TcgAction *action);
/* ... at a level: 0 a beginner .. 3 its best (the same as TcgDuel_AiAction) */
int TcgDuel_AiActionLevel(const struct TcgDuel *g, unsigned level, struct TcgAction *action);
/* Internal, shared by the rules/effect modules. */
void TcgDuel_Log(struct TcgDuel *g, const char *format, ...);
void TcgDuel_Event(struct TcgDuel *g, unsigned type, unsigned player, unsigned slot, unsigned value, unsigned card,
                   unsigned flags);
const struct TcgEvent *TcgDuel_GetEvent(const struct TcgDuel *g, uint32_t seq);
/* Flips a coin for player with the duel's own RNG and records it. */
unsigned TcgDuel_Coin(struct TcgDuel *g, unsigned player);
void TcgDuel_Shuffle(struct TcgDuel *g, unsigned player);
unsigned TcgDuel_Draw(struct TcgDuel *g, unsigned player, unsigned count);
void TcgDuel_DiscardField(struct TcgDuel *g, unsigned player, unsigned slot);
void TcgDuel_Damage(struct TcgDuel *g, unsigned player, unsigned slot, unsigned amount);
void TcgDuel_ResolveKnockouts(struct TcgDuel *g);
int TcgDuel_Trainer(struct TcgDuel *g, const struct TcgAction *action);
int TcgDuel_AttackEffect(struct TcgDuel *g, const struct TcgAttack *attack, const struct TcgAction *action,
                         unsigned *damage, int after);
/* Pokemon Powers (tcg_powers.c). TCG_USE_POWER: slot = the Pokemon using it; target / attack
 * = the Pokemon it moves from / to (Damage Swap, Energy Trans, Curse: target then attack),
 * the one it heals or attaches to (Heal, Rain Dance: hand = the Water Energy), the type
 * (Shift), what to look at (Peek, TCG_PEEK_*). */
int TcgDuel_Power(struct TcgDuel *g, const struct TcgAction *action);
unsigned TcgPower_Id(const struct TcgCard *c);
const char *TcgPower_Name(unsigned power);
/* The power of the Pokemon in that slot, if it works now (Toxic Gas, Asleep / Confused /
 * Paralyzed), else TCG_POWER_NONE. */
unsigned TcgPower_Working(const struct TcgDuel *g, unsigned player, unsigned slot);
/* A working power of this kind anywhere on that player's side (or either side: player 2). */
int TcgPower_InPlay(const struct TcgDuel *g, unsigned player, unsigned power);
/* Why the power in that slot can't be used now (NULL: it can), for player. */
const char *TcgPower_Block(const struct TcgDuel *g, unsigned player, unsigned slot);
/* The Pokemon's type now (Venomoth's Shift) */
unsigned TcgDuel_Type(const struct TcgDuel *g, unsigned player, unsigned slot);
/* Hooks: a Pokemon put into play during its owner's turn; damage an attack does to a
 * Pokemon (Kabuto Armor, Invisible Wall, Neutralizing Shield; Transparency flips);
 * conditions it can't have (Thick Skinned, Clefairy Doll, Mysterious Fossil). */
void TcgPower_OnPlay(struct TcgDuel *g, unsigned player, unsigned slot);
int TcgPower_AttackDamage(struct TcgDuel *g, unsigned player, unsigned slot, int amount, int defending);
int TcgPower_Immune(struct TcgDuel *g, unsigned player, unsigned slot);
void TcgPower_Sanitize(struct TcgDuel *g);
/* After an attack: Machamp's Strikes Back for each that took damage (before: the damage
 * and conditions as the attack began). */
void TcgPower_StrikesBack(struct TcgDuel *g, const uint16_t before[TCG_FIELD_SIZE], unsigned activeCondition);
unsigned TcgPower_RetreatAid(const struct TcgDuel *g, unsigned player);
/* GB2's hooks: no Weakness (Green Shield, Dampening Shield) / Resistance (Dampening Shield) for that Pokemon;
 * the retreat cost now (Sticky Goo, Retreat Aid) and whether it can retreat at all (Snorlax's Guard); after a
 * retreat (Sinkhole, Vine Pull); an attack's damage (Frenzy, Dark Wave, Prehistoric Dream); a Knock Out by an
 * attack (Final Beam); Trainers (Hay Fever) */
int TcgPower_NoWeakness(const struct TcgDuel *g, unsigned player, unsigned slot);
int TcgPower_NoResistance(const struct TcgDuel *g);
unsigned TcgDuel_RetreatCost(const struct TcgDuel *g, unsigned player);
const char *TcgPower_RetreatBlock(const struct TcgDuel *g, unsigned player);
void TcgPower_AfterRetreat(struct TcgDuel *g, unsigned player, unsigned benched);
int TcgPower_AttackBonus(struct TcgDuel *g, int amount);
int TcgPower_Frenzy(const struct TcgDuel *g, unsigned player);
void TcgPower_KnockedOut(struct TcgDuel *g, unsigned player, unsigned slot);
int TcgPower_NoTrainers(const struct TcgDuel *g);
/* Hypno's Puppet Master: the Pokemon whose attacks that player's Active Clefairy Doll uses (TCG_NONE: none) */
unsigned TcgPower_PuppetSlot(const struct TcgDuel *g, unsigned player);
uint8_t TcgPile_Remove(uint8_t *pile, uint8_t *count, unsigned index);
#endif
