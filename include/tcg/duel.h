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
     * 2 every effect of attacks. */
    uint8_t guard, weaken, charged, usedOnce;
};
enum
{
    TCG_GUARD_NONE,
    TCG_GUARD_REDUCE_10,
    TCG_GUARD_REDUCE_20,
    TCG_GUARD_UNDER_40,
    TCG_GUARD_HALVE
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
    TCG_CONCEDE
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
    TCG_EV_RETREAT_FAIL  /* player's confused Active failed to retreat */
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
int TcgDuel_EffectSupported(const char *name);
/* Why an attack's effect can't be used now (NULL = it can), e.g. Dream Eater. */
const char *TcgDuel_EffectBlock(const struct TcgDuel *g, unsigned player, const struct TcgAttack *attack);
/* 0: no choice. 1: the attacker picks one of the opponent's Benched Pokemon
 * (action.target), 2: one of their own (action.target). */
unsigned TcgDuel_AttackChoice(const struct TcgAttack *attack, int *required);
/* Damage from an effect to any Pokemon in play: no Weakness/Resistance;
 * Defender, and for an Active Pokemon PlusPower and substatus, apply. */
void TcgDuel_EffectDamage(struct TcgDuel *g, unsigned player, unsigned slot, int amount, unsigned flags);
void TcgDuel_SelfDamage(struct TcgDuel *g, int amount);
int TcgDuel_AiAction(const struct TcgDuel *g, struct TcgAction *action);
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
int TcgDuel_Power(struct TcgDuel *g, const struct TcgAction *action);
uint8_t TcgPile_Remove(uint8_t *pile, uint8_t *count, unsigned index);
#endif
