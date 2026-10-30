/* Yu-Gi-Oh! duels: the rules engine, EDOPro's (edopro/ocgcore: every card with every effect, today's rules), and the
 * bridge between it and the game (src/ygo/ygo_edo.c).
 *
 * The engine stops at questions (YgoQuery: what to do in the Main Phase, whether to chain, which cards to pick...);
 * YgoDuel_Answer gives the answer and runs on to the next question. As it runs, what it does becomes presentation
 * events for the duel screen and its state is mirrored into YgoState (the cards as objects that keep their number as
 * they move, so the screen can follow each one). Everything is deterministic from the seed and the answers: two
 * linked games stay in step by exchanging answers alone, and a duel can be rebuilt from its answers (YgoDuel_Copy). */
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
#define YGO_QUERY_MAX 128
#define YGO_PICKS_MAX 64
#define YGO_EVENT_RING 128
#define YGO_NAMES_MAX 2048    /* a card name to declare: the cards it can be */

enum YgoLocation
{
    YGO_LOC_NONE,      /* a token that left the field, or a card under another (Xyz Material) */
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
struct YgoObj
{
    uint16_t card;
    uint8_t owner, controller, location, seq;
    uint8_t faceUp, attackPos, summonType;
    uint8_t equipTo;            /* Equip Spell: the monster's object + 1 */
    uint8_t counters, negated, reason;
    uint8_t attacked;           /* it declared an attack this turn */
    int32_t atk, def;           /* now, with every effect (-1: "?") */
    uint8_t level;
    uint16_t enteredTurn;       /* the turn it reached its location */
};
struct YgoPlayer
{
    int32_t lp;
    uint8_t deck[YGO_DECK_MAX + 20], hand[YGO_HAND_MAX], grave[YGO_OBJS / 2], banished[YGO_OBJS / 2],
        extra[YGO_FUSION_MAX + 20];
    uint8_t deckCount, handCount, graveCount, banishedCount, extraCount;
    uint8_t monsters[YGO_ZONES];        /* object + 1, 0 empty */
    uint8_t spells[YGO_ZONES + 1];
    uint8_t normalSummoned;             /* (as far as the events tell) */
};
/* What the engine is asking (YgoDuel.query) */
enum YgoQueryType
{
    YGO_Q_NONE,
    YGO_Q_IDLE,      /* Main Phase: one of items (YGO_ACT_*) */
    YGO_Q_BATTLE,    /* Battle Phase: attack with one of items, or go on */
    YGO_Q_CHAIN,     /* chain one of items (an effect) or pass (no pick) */
    YGO_Q_YESNO,     /* pick item 0 (no) or 1 (yes) */
    YGO_Q_CARDS,     /* min..max of items (objects; or cards by id when obj is 0xFF) */
    YGO_Q_OPTION,    /* min..max of items (labels: YgoDuel_ItemText) */
    YGO_Q_POSITION,  /* one of items: value = YGO_POS_* */
    YGO_Q_ZONE,      /* min..max empty zones: value = seq, flags YGO_ZF_* */
    YGO_Q_SUM,       /* cards whose param adds up to sum (YgoQuery.sum, YGO_SUM_*); items flagged YGO_IF_MUST always count */
    YGO_Q_SORT,      /* every item, in the order picked */
    YGO_Q_COUNTER,   /* remove sum counters in all from items (param: what each has): picks[i] = how many from item i */
    YGO_Q_NAME       /* declare a card name: one of YgoDuel.names (picks 0-1: its index, low byte first) */
};
/* Main / Battle Phase actions (YgoChoice.kind) */
enum YgoActionKind
{
    YGO_ACT_SUMMON,      /* Normal / Tribute Summon obj */
    YGO_ACT_SET_MONSTER,
    YGO_ACT_FLIP_SUMMON,
    YGO_ACT_POSITION,    /* change obj's battle position */
    YGO_ACT_ACTIVATE,    /* activate one of obj's effects (YgoDuel_ItemText: which) */
    YGO_ACT_SET_SPELL,   /* set a Spell / Trap from the hand */
    YGO_ACT_SPECIAL,     /* special summon obj by its own rule */
    YGO_ACT_TO_BATTLE,
    YGO_ACT_TO_MAIN2,
    YGO_ACT_END_TURN,
    YGO_ACT_ATTACK,      /* Battle Phase: obj attacks (flags 1: it can attack directly) */
    YGO_ACT_CONCEDE
};
enum { YGO_POS_ATTACK = 1, YGO_POS_FACEDOWN_ATTACK = 2, YGO_POS_DEFENSE = 4, YGO_POS_FACEDOWN_DEFENSE = 8 };
enum { YGO_ZF_SPELL = 1, YGO_ZF_OPPONENT = 2 };
enum { YGO_IF_PICKED = 1, YGO_IF_MUST = 2 };   /* YgoChoice.flags in CARDS / SUM questions */
enum { YGO_SUM_EXACT, YGO_SUM_AT_LEAST };
struct YgoChoice
{
    uint8_t kind, obj;   /* obj: object index (0xFF none) */
    uint8_t value, flags;
    uint16_t card;       /* the card (also when there's no object) */
    uint16_t param;      /* SUM: what it counts (low 16 bits and, when it can count two ways, the other in the high);
                          * TRIBUTE: how many Tributes it's worth; COUNTER: its counters */
    uint64_t desc;       /* EDOPro's description of the effect / option */
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
    YGO_P_COIN_CALL,     /* OPTION: heads or tails */
    YGO_P_SUMMON_OR_SET, /* OPTION */
    YGO_P_ANY,
    YGO_P_TO_DECK,
    YGO_P_TO_HAND,       /* return to the hand */
    YGO_P_POSITION,
    YGO_P_CONTROL,
    YGO_P_DECLARE        /* a Type, an Attribute, a number, a card name */
};
struct YgoQuery
{
    uint8_t type, player, prompt, min, max, count;
    uint16_t card;       /* the card asking (its effect), 0 none */
    uint64_t desc;       /* EDOPro's description of the question (YgoDuel_QueryText words it) */
    uint32_t sum;        /* SUM: the total; COUNTER: how many to remove; CARDS: how many the engine wants (min is 0
                          * when it can be backed out of) */
    uint8_t sumMode;
    uint8_t cancelable;  /* CARDS / SUM: no pick backs out */
    struct YgoChoice items[YGO_QUERY_MAX];
};
struct YgoAnswer
{
    uint8_t count;
    uint8_t picks[YGO_PICKS_MAX];   /* item indices (COUNTER: amounts per item) */
    uint8_t zone;   /* a Main Phase answer that puts a card from the hand on the field: the zone it goes to + 1
                     * (bit 7: a Spell & Trap Zone; 0: the first empty one). Ignored for the rest. */
};
/* A chain link */
struct YgoLink
{
    uint8_t obj, player;
    uint8_t negated;
    uint16_t card;
    uint64_t desc;
};
/* Presentation events: in order, for the duel screen (never part of the hash). */
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
    YGO_EV_BATTLE,        /* damage calculation: obj vs target */
    YGO_EV_DESTROY,       /* obj destroyed (flags: reason) */
    YGO_EV_TO_GRAVE,      /* obj sent to the Graveyard (flags: reason) */
    YGO_EV_TO_HAND,       /* obj returned / added to the hand */
    YGO_EV_TO_DECK,
    YGO_EV_BANISH,
    YGO_EV_DAMAGE,        /* player loses value LP (flags 1: battle) */
    YGO_EV_RECOVER,       /* player gains value LP */
    YGO_EV_STAT,          /* obj's ATK/DEF changed (value: the ATK change) */
    YGO_EV_EQUIP,         /* obj equipped to target */
    YGO_EV_COIN,          /* player's coin: value 1 heads */
    YGO_EV_DICE,          /* player's die: value 1-6 */
    YGO_EV_SHUFFLE,
    YGO_EV_REVEAL,        /* obj shown to the opponent */
    YGO_EV_TARGET,        /* obj targets target */
    YGO_EV_CONTROL,       /* player takes control of obj */
    YGO_EV_LOOK,          /* player looks at obj (only player's screen shows it) */
    YGO_EV_WIN,           /* player wins: value YGO_WIN_* */
    YGO_EV_MOVE,          /* obj moved on the field, or put there by an effect (no summon, set or activation) */
    YGO_EV_DECLARE,       /* player declared value: flags 1 a card (card), 2 a number, 3 a Type, 4 an Attribute */
    YGO_EV_COUNTER        /* obj's counters changed (value: now) */
};
enum { YGO_WIN_LP, YGO_WIN_DECK_OUT, YGO_WIN_EXODIA, YGO_WIN_CONCEDE, YGO_WIN_EFFECT };
struct YgoEvent
{
    uint8_t type, player, obj, target;
    uint16_t card, flags;
    int32_t value;
};
/* The state now: everything a screen shows (hashed for online play) */
struct YgoState
{
    struct YgoObj objs[YGO_OBJS];
    uint8_t objCount;
    struct YgoPlayer players[2];
    uint16_t turn;
    uint8_t current, first, phase, winner, winReason, over;
    /* Battle */
    uint8_t attacker, attackTarget; /* object + 1 */
    /* Chain */
    struct YgoLink chain[YGO_CHAIN_MAX];
    uint8_t chainCount;
    /* Presentation */
    uint32_t eventSeq;
    struct YgoEvent events[YGO_EVENT_RING];
};
struct YgoEdoDuel;   /* (src/ygo/ygo_edo.c) */
struct YgoDuel
{
    struct YgoState s;
    struct YgoQuery query;          /* the question now (type YGO_Q_NONE: the duel is over) */
    uint16_t names[YGO_NAMES_MAX];  /* YGO_Q_NAME: the card ids that can be declared */
    uint16_t nameCount;
    char error[96];
    struct YgoEdoDuel *edo;         /* the engine's duel: never copy a YgoDuel by value (YgoDuel_Copy) */
};

/* A duel between two decks (main deck then Fusion Deck card ids, 0-terminated or full). A duel held before is
 * freed first. 0 when the engine can't start (d->error says why). */
int YgoDuel_Init(struct YgoDuel *d, const uint16_t main0[YGO_DECK_MAX], const uint16_t extra0[YGO_FUSION_MAX],
                 const uint16_t main1[YGO_DECK_MAX], const uint16_t extra1[YGO_FUSION_MAX], uint32_t seed,
                 unsigned first);
void YgoDuel_Free(struct YgoDuel *d);
/* dst becomes the same duel as src, rebuilt from its seed and answers (dst's own duel is freed first) */
int YgoDuel_Copy(struct YgoDuel *dst, const struct YgoDuel *src);
/* d goes back to how it was after its first `answers` answers (rebuilt; its events go on numbering from where they
 * were) */
int YgoDuel_Rewind(struct YgoDuel *d, unsigned answers);
/* Answer the current question. 1 done (the next question is in d->query), 0 refused (d->error says why). */
int YgoDuel_Answer(struct YgoDuel *d, const struct YgoAnswer *answer);
/* Would the answer be taken? (d is left as it is) */
int YgoDuel_Check(const struct YgoDuel *d, const struct YgoAnswer *answer);
int YgoDuel_Concede(struct YgoDuel *d, unsigned player);
/* After d->s was changed from outside (tests, debug commands): ask the question again */
int YgoDuel_Refresh(struct YgoDuel *d);
uint32_t YgoDuel_Hash(const struct YgoDuel *d);
/* Counts every physical card exactly once (tests) */
int YgoDuel_Conserved(const struct YgoDuel *d);
/* The question is a chance to chain at a moment nothing is happening (a phase changing: no chain, summon, attack
 * or damage): a screen can pass for its player */
int YgoDuel_QuietChance(const struct YgoDuel *d);
/* Values now, with every effect */
int YgoDuel_Atk(const struct YgoState *s, unsigned obj);
int YgoDuel_Def(const struct YgoState *s, unsigned obj);
int YgoDuel_Level(const struct YgoState *s, unsigned obj);
const struct YgoEvent *YgoDuel_GetEvent(const struct YgoState *s, uint32_t seq);
/* Words for the question and its items (EDOPro's strings: the card's own effect texts, or the system's); buf is
 * used when one has to be made. "" when there's nothing better than the screen's own words. */
const char *YgoDuel_QueryText(const struct YgoDuel *d, char *buf, unsigned size);
const char *YgoDuel_ItemText(const struct YgoDuel *d, unsigned item, char *buf, unsigned size);
/* Answers given so far (tests, logs) */
unsigned YgoDuel_AnswerCount(const struct YgoDuel *d);
/* The AI's answer to d->query for its player (src/ygo/ygo_ai.c): at its best, or at a level (0 a beginner .. 3) */
void YgoDuel_AiAnswer(const struct YgoDuel *d, struct YgoAnswer *answer);
void YgoDuel_AiAnswerLevel(const struct YgoDuel *d, unsigned level, struct YgoAnswer *answer);
/* Can the card be played: EDOPro has it whole (src/ygo/ygo_edo_data.c)? Cards that can't don't show up in packs,
 * shops or decks. */
int Ygo_CardReady(unsigned card);
#endif
