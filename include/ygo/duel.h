/* Yu-Gi-Oh! duels: the rules engine (src/ygo/ygo_duel.c, card effects in src/ygo/ygo_fx_*.c).
 *
 * Today's rules on the 2003 cards: 8000 LP, the player going first neither draws nor attacks on their first turn,
 * a Fusion Deck of up to 15, chains with Spell Speeds, simultaneous triggers ordered turn player first (SEGOC),
 * optional "when" triggers that miss the timing, the Damage Step's limits.
 *
 * The engine is a state machine that stops at questions (YgoQuery: what to do in the Main Phase, whether to chain,
 * which cards to pick...). YgoDuel_Answer gives the answer and runs on to the next question. Inside, the game moves
 * in small units (a summon, one chain link's resolution, one response window); a unit that needs more answers is
 * run again from its start with every answer given so far, so card effects are written as plain code that asks as
 * it goes (Ygo_AskCards...). Everything is deterministic from the seed and the answers: two linked games stay in
 * step by exchanging answers alone. */
#ifndef MOEMON_YGO_DUEL_H
#define MOEMON_YGO_DUEL_H
#include <stdint.h>
#include "ygo/ygo.h"

#define YGO_LP 8000
#define YGO_OBJS 200          /* physical cards: both decks, both Fusion Decks, tokens */
#define YGO_ZONES 5
#define YGO_FIELD_ZONE 5      /* spells[5]: the Field Spell zone */
#define YGO_HAND_MAX 60
#define YGO_CHAIN_MAX 16
#define YGO_PENDING_MAX 16
#define YGO_QUERY_MAX 80
#define YGO_ANSWERS_MAX 48
#define YGO_FRAMES 12
#define YGO_EVENT_RING 96
#define YGO_LASTING_MAX 24

enum YgoLocation
{
    YGO_LOC_NONE,      /* a token that left the field */
    YGO_LOC_DECK,
    YGO_LOC_HAND,
    YGO_LOC_MZONE,
    YGO_LOC_SZONE,     /* seq 5: the Field Spell zone */
    YGO_LOC_GRAVE,
    YGO_LOC_BANISHED,
    YGO_LOC_EXTRA
};
enum YgoPhase
{
    YGO_PHASE_DRAW,
    YGO_PHASE_STANDBY,
    YGO_PHASE_MAIN1,
    YGO_PHASE_BATTLE,
    YGO_PHASE_MAIN2,
    YGO_PHASE_END
};
enum YgoSummonType
{
    YGO_SUMMON_NONE,
    YGO_SUMMON_NORMAL,
    YGO_SUMMON_TRIBUTE,
    YGO_SUMMON_FLIP,
    YGO_SUMMON_SPECIAL,
    YGO_SUMMON_FUSION,
    YGO_SUMMON_RITUAL
};
/* Why a card left or moved (YgoObj.reason and events) */
enum
{
    YGO_REASON_BATTLE = 1,
    YGO_REASON_EFFECT = 2,
    YGO_REASON_COST = 4,
    YGO_REASON_TRIBUTE = 8,
    YGO_REASON_MATERIAL = 16,
    YGO_REASON_DESTROY = 32,
    YGO_REASON_RULE = 64,
    YGO_REASON_DISCARD = 128
};
/* Per-object turn flags (YgoObj.turnFlags, cleared as each turn begins) */
enum
{
    YGO_TF_SUMMONED = 1,        /* summoned / set this turn: no position change */
    YGO_TF_POS_CHANGED = 2,
    YGO_TF_ATTACKED = 4,
    YGO_TF_SET = 8,             /* a Spell/Trap set this turn: a Trap can't be activated yet */
    YGO_TF_FLIPPED = 16
};
struct YgoObj
{
    uint16_t card;
    uint8_t owner, controller, location, seq;
    uint8_t faceUp, attackPos, turnFlags, summonType;
    uint8_t attacks;            /* attacks declared this turn */
    uint8_t equipTo;            /* Equip Spell / equipped monster: the monster's object + 1 */
    uint8_t counters, usedTurn; /* counters; usedTurn: once-per-turn effects (bit per effect) */
    uint8_t negated, reason;    /* negated: its effects don't apply (until it leaves the field) */
    int16_t atkMod, defMod;     /* lasting changes from resolved effects */
    int16_t atkTurn, defTurn;   /* until the end of this turn */
    uint16_t enteredTurn;       /* the turn it reached its location */
    uint32_t data;              /* card-specific memory (targets, choices), cleared when it moves */
    uint8_t moves;              /* times it changed location: a card that moved is a new card to what targeted it */
    uint8_t properly;           /* it was once Summoned properly (Fusion, Ritual, its own rule): it can be revived */
};
struct YgoPlayer
{
    int32_t lp;
    uint8_t deck[YGO_DECK_MAX], hand[YGO_HAND_MAX], grave[YGO_OBJS / 2], banished[YGO_OBJS / 2], extra[YGO_FUSION_MAX];
    uint8_t deckCount, handCount, graveCount, banishedCount, extraCount;
    uint8_t monsters[YGO_ZONES];        /* object + 1, 0 empty */
    uint8_t spells[YGO_ZONES + 1];
    uint8_t normalSummoned, drewNothing, turns;
    uint16_t monsterToGraveTurn;   /* the last turn a monster of theirs went from their field to their Graveyard */
    uint16_t summonedTurn;         /* the last turn they Summoned a monster (Scapegoat) */
};
/* What the engine is asking (YgoDuel.query) */
enum YgoQueryType
{
    YGO_Q_NONE,
    YGO_Q_IDLE,      /* Main Phase: one of items (YGO_ACT_*) */
    YGO_Q_BATTLE,    /* Battle Phase: attack with one of items, or go on */
    YGO_Q_CHAIN,     /* chain one of items (an effect) or pass (no pick) */
    YGO_Q_YESNO,     /* pick item 0 (no) or 1 (yes) */
    YGO_Q_CARDS,     /* min..max of items (objects) */
    YGO_Q_OPTION,    /* one of items (value = the option) */
    YGO_Q_POSITION,  /* item 0 face-up ATK, 1 face-up DEF, 2 face-down DEF (only those allowed are listed) */
    YGO_Q_ZONE       /* an empty zone (value = seq) */
};
/* Main / Battle Phase actions (YgoChoice.kind) */
enum YgoActionKind
{
    YGO_ACT_SUMMON,      /* Normal / Tribute Summon obj */
    YGO_ACT_SET_MONSTER,
    YGO_ACT_FLIP_SUMMON,
    YGO_ACT_POSITION,    /* change obj's battle position */
    YGO_ACT_ACTIVATE,    /* activate effect value of obj */
    YGO_ACT_SET_SPELL,   /* set a Spell / Trap from the hand */
    YGO_ACT_SPECIAL,     /* special summon obj by its own rule (value: the procedure) */
    YGO_ACT_TO_BATTLE,
    YGO_ACT_TO_MAIN2,
    YGO_ACT_END_TURN,
    YGO_ACT_ATTACK,      /* Battle Phase: obj attacks */
    YGO_ACT_CONCEDE
};
struct YgoChoice
{
    uint8_t kind, obj;   /* obj: object index (0xFF none) */
    uint8_t value, flags;
};
/* Prompts (YgoQuery.prompt): what the question is for, so a screen can word it */
enum YgoPrompt
{
    YGO_P_NONE,
    YGO_P_TRIBUTE,
    YGO_P_TARGET,
    YGO_P_ATTACK_TARGET,
    YGO_P_DISCARD,
    YGO_P_DESTROY,
    YGO_P_SUMMON,
    YGO_P_ADD_TO_HAND,
    YGO_P_TO_GRAVE,
    YGO_P_BANISH,
    YGO_P_EQUIP,
    YGO_P_MATERIAL,
    YGO_P_FUSION,
    YGO_P_RITUAL,
    YGO_P_EFFECT_YESNO,  /* use an optional effect? (card) */
    YGO_P_CHAIN,
    YGO_P_ORDER,         /* which trigger goes first in the chain */
    YGO_P_COIN_CALL,
    YGO_P_ANY
};
struct YgoQuery
{
    uint8_t type, player, prompt, min, max, count;
    uint16_t card;       /* the card asking (its effect), 0 none */
    struct YgoChoice items[YGO_QUERY_MAX];
};
struct YgoAnswer
{
    uint8_t count;
    uint8_t picks[YGO_QUERY_MAX / 2];   /* item indices */
};
/* A chain link, and a trigger waiting to join the next chain */
struct YgoLink
{
    uint8_t obj, effect, player, targetCount;
    uint8_t targets[8], targetMoves[8];
    uint8_t negated, activation;   /* activation: a Spell/Trap card's activation (not only an effect) */
    uint8_t eventObj;              /* triggers: the card that set it off (0xFF none) */
    int16_t value;                 /* chosen at activation (a number, an option) */
    uint16_t card;
};
struct YgoPending
{
    uint8_t obj, effect, player, eventObj;
    uint8_t optional, when;        /* when: "When... you can" (it misses the timing) */
    uint16_t serial;               /* the event that set it off */
};
/* Lasting effects that outlive the effect that made them (Waboku's "this turn", Kuriboh's "that battle"): each
 * works as a rule (YR_* in ygo_internal.h, kind) for player, an object or that player's monsters (scope YGO_SCOPE_*) */
enum { YGO_SCOPE_PLAYER, YGO_SCOPE_OBJ, YGO_SCOPE_MONSTERS };
#define YGO_UNTIL_REMOVED 0xFFFF
#define YGO_UNTIL_BATTLE 0xFFFE    /* the end of this battle's Damage Step */
#define YGO_UNTIL_CHAIN 0xFFFD     /* the end of the chain resolving now */
struct YgoLasting
{
    uint8_t kind, player, obj, scope;
    uint16_t until;                /* the turn it ends at the end of, or YGO_UNTIL_* */
    int16_t value;
};
/* Engine flow frames (internal) */
struct YgoFlow
{
    uint8_t proc, step;
    uint8_t a, b;
};
/* Presentation events: in order, for the duel screen (never part of the hash). Hidden cards are not named
 * except to their owner's screen (the card field is set only when that player may see it: see YgoDuel_EventCard). */
enum YgoEventType
{
    YGO_EV_NONE,
    YGO_EV_TURN,          /* player's turn begins (value: turn number) */
    YGO_EV_PHASE,         /* value: YGO_PHASE_* */
    YGO_EV_DRAW,          /* player draws obj */
    YGO_EV_SUMMON,        /* obj summoned; value YGO_SUMMON_* */
    YGO_EV_SET,           /* obj set (monster or Spell/Trap) */
    YGO_EV_FLIP,          /* obj turned face up */
    YGO_EV_POSITION,      /* obj's battle position changed */
    YGO_EV_ACTIVATE,      /* obj's effect activates as chain link value */
    YGO_EV_RESOLVE,       /* chain link value (obj) resolves */
    YGO_EV_NEGATE,        /* obj's activation / effect negated */
    YGO_EV_ATTACK,        /* obj attacks target (0xFF: directly) */
    YGO_EV_BATTLE,        /* damage calculation: obj vs target, value = damage to flags' player */
    YGO_EV_DESTROY,       /* obj destroyed (flags: reason) */
    YGO_EV_TO_GRAVE,      /* obj sent to the Graveyard (flags: reason) */
    YGO_EV_TO_HAND,       /* obj returned / added to the hand */
    YGO_EV_TO_DECK,
    YGO_EV_BANISH,
    YGO_EV_DAMAGE,        /* player loses value LP (flags 1: battle) */
    YGO_EV_RECOVER,       /* player gains value LP */
    YGO_EV_STAT,          /* obj's ATK/DEF changed */
    YGO_EV_EQUIP,         /* obj equipped to target */
    YGO_EV_COIN,          /* player's coin: value 1 heads */
    YGO_EV_DICE,          /* player's die: value 1-6 */
    YGO_EV_SHUFFLE,
    YGO_EV_REVEAL,        /* obj shown to the opponent */
    YGO_EV_TARGET,        /* obj targets target */
    YGO_EV_CONTROL,       /* player takes control of obj */
    YGO_EV_LOOK,          /* player looks at obj (only player's screen shows it) */
    YGO_EV_WIN            /* player wins: value YGO_WIN_* */
};
enum { YGO_WIN_LP, YGO_WIN_DECK_OUT, YGO_WIN_EXODIA, YGO_WIN_CONCEDE, YGO_WIN_EFFECT };
struct YgoEvent
{
    uint8_t type, player, obj, target;
    uint16_t card, flags;
    int32_t value;
};
/* The committed state: everything that decides the duel (hashed for online play) */
struct YgoState
{
    struct YgoObj objs[YGO_OBJS];
    uint8_t objCount;
    struct YgoPlayer players[2];
    uint32_t rng;
    uint16_t turn, serial;          /* serial: game events so far (missing the timing) */
    uint8_t current, first, phase, winner, winReason, over;
    /* Battle */
    uint8_t attacker, attackTarget; /* object + 1 */
    uint8_t battleStep, damageStep, directAttack, attackNegated, flippedByBattle, endBattle;
    uint8_t battleDestroyed[2];     /* object + 1: destroyed by this battle, sent to the Graveyard at its end */
    /* The response window open now (YW_* in ygo_internal.h): what "When..." cards answer */
    uint8_t window, windowObj, windowPlayer;
    /* Chain */
    struct YgoLink chain[YGO_CHAIN_MAX];
    uint8_t chainCount, chainPasses, chainTurn, resolving;
    struct YgoPending pending[YGO_PENDING_MAX];
    uint8_t pendingCount;
    struct YgoLasting lasting[YGO_LASTING_MAX];
    uint8_t lastingCount;
    struct YgoFlow frames[YGO_FRAMES];
    uint8_t frameCount;
    /* Presentation */
    uint32_t eventSeq;
    struct YgoEvent events[YGO_EVENT_RING];
};
struct YgoDuel
{
    struct YgoState s;              /* as the current unit began */
    struct YgoAnswer answers[YGO_ANSWERS_MAX];
    uint8_t answerCount;
    struct YgoQuery query;          /* the question now (type YGO_Q_NONE: the duel is over) */
    char error[96];
};

/* A duel between two decks (main deck then Fusion Deck card ids, 0-terminated or full). */
int YgoDuel_Init(struct YgoDuel *d, const uint16_t main0[YGO_DECK_MAX], const uint16_t extra0[YGO_FUSION_MAX],
                 const uint16_t main1[YGO_DECK_MAX], const uint16_t extra1[YGO_FUSION_MAX], uint32_t seed,
                 unsigned first);
/* Answer the current question. 1 done (the next question is in d->query), 0 refused (d->error says why). */
int YgoDuel_Answer(struct YgoDuel *d, const struct YgoAnswer *answer);
int YgoDuel_Concede(struct YgoDuel *d, unsigned player);
/* After d->s was changed from outside (tests, debug commands): ask the question again */
int YgoDuel_Refresh(struct YgoDuel *d);
uint32_t YgoDuel_Hash(const struct YgoDuel *d);
/* Counts every physical card exactly once (tests) */
int YgoDuel_Conserved(const struct YgoDuel *d);
/* Values now, with every lasting and continuous effect */
int YgoDuel_Atk(const struct YgoState *s, unsigned obj);
int YgoDuel_Def(const struct YgoState *s, unsigned obj);
int YgoDuel_Level(const struct YgoState *s, unsigned obj);
const struct YgoEvent *YgoDuel_GetEvent(const struct YgoState *s, uint32_t seq);
/* The AI's answer to d->query for its player (src/ygo/ygo_ai.c) */
void YgoDuel_AiAnswer(const struct YgoDuel *d, struct YgoAnswer *answer);
/* Can the card's effects be used yet (stage by stage, src/ygo/ygo_fx_*.c)? Cards that can't don't show up in
 * packs, shops or decks. Normal Monsters always can. */
int Ygo_CardReady(unsigned card);
#endif
