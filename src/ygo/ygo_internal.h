/* Yu-Gi-Oh! rules internals shared by the engine (ygo_duel.c) and the card effects (ygo_fx_*.c). */
#ifndef MOEMON_YGO_INTERNAL_H
#define MOEMON_YGO_INTERNAL_H
#include "ygo/duel.h"

#define NO_OBJ 0xFF

/* Effect kinds */
enum
{
    YE_ACTIVATE,    /* the activation of a Spell/Trap card (Equip: target the monster, attached as it resolves) */
    YE_IGNITION,    /* a monster's effect you activate in your Main Phase (Spell Speed 1) */
    YE_QUICK,       /* a monster's effect you can activate at Spell Speed 2 */
    YE_TRIGGER,     /* set off by events (YgoEffect.events) */
    YE_FLIP,        /* FLIP: when it is flipped face up */
    YE_CONTINUOUS,  /* always on while it's face up on the field (or where range says): only stat / rule hooks */
    YE_SUMMON_RULE  /* a monster's own way to be Special Summoned from the hand (YGO_ACT_SPECIAL) */
};
/* Effect flags */
enum
{
    YF_OPTIONAL = 1,         /* "you can" */
    YF_WHEN = 2,             /* "When ...": an optional one misses the timing */
    YF_ONCE_PER_TURN = 4,
    YF_DAMAGE_STEP = 8,      /* can be activated during the Damage Step */
    YF_SUMMON_NEGATE = 16,   /* can answer a summon (Solemn Judgment's window) */
    YF_TARGETS = 32,
    YF_FROM_HAND = 64,       /* a monster effect activated from the hand (Kuriboh) */
    YF_IN_GRAVE = 128,       /* a trigger that works from the Graveyard (Sangan's "sent to the GY" works by default) */
    YF_STAYS = 256,          /* a Normal Spell / Trap that stays on the field after resolving (Swords of Revealing Light) */
    YF_HARD_ONCE = 512,      /* only once while face up (Sanga of the Thunder): kept in the object's data bit 31 */
    YF_PHASE = 1024          /* made for a quiet moment (a phase: Blast Juggler's Standby Phase): asked then too */
};
/* Trigger events (YgoEffect.events): about the effect's own card unless marked ANY */
enum
{
    YEV_NORMAL_SUMMONED = 1 << 0,
    YEV_FLIP_SUMMONED = 1 << 1,
    YEV_SPECIAL_SUMMONED = 1 << 2,
    YEV_FLIPPED = 1 << 3,             /* turned face up (FLIP effects) */
    YEV_DESTROYED_BATTLE = 1 << 4,    /* destroyed by battle and sent to the Graveyard */
    YEV_TO_GRAVE = 1 << 5,            /* sent to the Graveyard (from anywhere) */
    YEV_FROM_FIELD_TO_GRAVE = 1 << 6,
    YEV_BATTLE_DAMAGE = 1 << 7,       /* inflicted battle damage to the opponent */
    YEV_DESTROYED_BY_EFFECT = 1 << 8,
    YEV_DISCARDED = 1 << 9,
    YEV_STANDBY_OWN = 1 << 10,        /* its controller's Standby Phase */
    YEV_STANDBY_ANY = 1 << 11,
    YEV_END_OWN = 1 << 12,            /* its controller's End Phase */
    YEV_DESTROYED_OPP_BATTLE = 1 << 13, /* it destroyed an opponent's monster by battle */
    YEV_ATTACKED = 1 << 14,           /* it was chosen as an attack target */
    YEV_ANY_SUMMONED = 1 << 15,       /* ANY: a monster was summoned (eventObj) */
    YEV_ANY_DESTROYED = 1 << 16,      /* ANY: a monster was destroyed (eventObj) */
    YEV_BATTLED = 1 << 17,            /* after damage calculation, it battled (eventObj: the other monster) */
    YEV_DRAW_OWN = 1 << 18,           /* its controller drew in their Draw Phase */
    YEV_END_ANY = 1 << 19,            /* either player's End Phase */
    YEV_ANY_NORMAL_SUMMONED = 1 << 20, /* ANY: a monster was Normal or Flip Summoned (eventObj) */
    YEV_DECLARED_ATTACK = 1 << 21,    /* it declared an attack */
    YEV_TO_DEFENSE = 1 << 22,         /* changed from Attack to face-up Defense Position */
    YEV_TO_ATTACK = 1 << 23,          /* changed from Defense to Attack Position */
    YEV_EQUIPPED = 1 << 24,           /* an Equip Card was equipped to it (eventObj: the equip) */
    YEV_SENT_FROM_FIELD = 1 << 25,    /* left the field (to anywhere) */
    YEV_ANY_BATTLE_DAMAGE = 1 << 26   /* ANY: a monster (eventObj) inflicted battle damage to its controller's opponent */
};
/* Response windows (YgoState.window): what just happened, for "When..." activations */
enum
{
    YW_OPEN,            /* nothing in particular */
    YW_SUMMONING,       /* a monster is being Normal / Flip / Special Summoned (negation only); windowObj */
    YW_SUMMONED,        /* a Normal / Tribute / Flip Summon succeeded; windowObj */
    YW_SPECIAL_SUMMONED,
    YW_ATTACK,          /* an attack was declared: windowObj attacks (state attacker / attackTarget) */
    YW_DAMAGE_STEP,     /* the Damage Step (start, before damage calculation) */
    YW_AFTER_DAMAGE,
    YW_END_DAMAGE,
    YW_DRAW,
    YW_STANDBY,
    YW_BATTLE_START,
    YW_END
};
/* Rules asked of every continuous effect (YgoEffect.rule): 1 when that effect makes it so */
enum
{
    YR_CANNOT_ATTACK,            /* a: the monster */
    YR_CANNOT_ATTACK_DIRECT,
    YR_DIRECT_ATTACK,            /* a can attack directly */
    YR_INDESTRUCTIBLE_BATTLE,
    YR_INDESTRUCTIBLE_EFFECT,
    YR_NO_BATTLE_DAMAGE,         /* a: the player who would take it, b: the monster dealing it */
    YR_PIERCE,                   /* a inflicts piercing damage */
    YR_CANNOT_CHANGE_POSITION,
    YR_NO_TRAPS,                 /* a: the player who can't activate Traps (2: nobody may) */
    YR_NO_SPELLS,
    YR_TRAPS_NEGATED,            /* Jinzo: Trap effects on the field don't apply */
    YR_CANNOT_NORMAL_SUMMON,     /* a: the card id */
    YR_CANNOT_SPECIAL_SUMMON,    /* a: the player */
    YR_NO_ATTACKS,               /* a: the player whose monsters can't attack (Swords of Revealing Light) */
    YR_UNTARGETABLE,             /* a: the monster, b: the effect's controller */
    YR_EXTRA_ATTACK,             /* a: the monster; returns how many more attacks */
    YR_NO_DRAW,
    YR_ATTACK_ONLY,              /* a: a monster the opponent's monsters must attack, if they attack */
    YR_ATTACK_COST,              /* a: the attacker; returns the LP it costs to declare an attack */
    YR_NO_BATTLE_PHASE,          /* a: the player */
    YR_NO_SUMMON,                /* a: the player who can't Summon (Normal Set still allowed) */
    YR_NOT_TRIBUTE,              /* a: a monster that can't be Tributed for a Tribute Summon */
    YR_NO_EQUIP,                 /* a: a monster equips are destroyed on (Gearfried) */
    YR_ATK_ZERO,                 /* a: its ATK is 0 (during this damage calculation: Sanga) */
    YR_REVERSE_STATS,            /* everyone: additions and subtractions to ATK/DEF reversed (Reverse Trap) */
    YR_SWAP_STATS,               /* everyone: original ATK and DEF switched (Shield & Sword) */
    YR_EXTRA_TRIBUTE,            /* a: a monster of the opponent's the player (b) can Tribute (Soul Exchange) */
    YR_BATTLE_SUBSTITUTE,        /* a: a monster about to be destroyed by battle; returns what is destroyed instead + 1 */
    YR_MIRROR_DAMAGE,            /* a: a monster whose battles' damage to its controller also hits the opponent */
    YR_COUNT
};
/* Lasting kinds past the rules */
#define YL_RETURN_CONTROL 200    /* obj: control comes back to its owner as the turn ends (Change of Heart) */
#define YL_DESTROY_AT 201        /* obj (moves in value): destroyed at the end of the turn (Steel Scorpion) */
#define YL_NO_ACTIVATE_NAME 202  /* player can't activate cards with that name (value: card id) this turn */
#define YL_NAME_USED 203         /* player used the once-per-turn effect of the card with id value this turn */
int Ygo_LastingValue(const struct YgoState *s, unsigned kind, unsigned player, int value);
/* Normal Summon / Set one more monster now (Ultimate Offering): player's, from the hand */
void Ygo_ExtraNormalSummon(struct YgoState *s, unsigned player, unsigned o, int set);
int Ygo_CanNormalSummonNow(const struct YgoState *s, unsigned player, unsigned o, int set);
/* Must this monster be Summoned properly before an effect can Special Summon it (Fusion, Ritual, its own rule)? */
int Ygo_NeedsProper(unsigned card);
enum { YS_ATK, YS_DEF, YS_LEVEL };

struct YgoCtx;
struct YgoEffect
{
    uint16_t card;
    uint8_t type, speed;
    uint16_t flags;
    uint32_t events;
    /* activation: can it be activated now (no questions), cost, targets (stored in the link), what it does */
    int (*can)(struct YgoCtx *c);
    void (*cost)(struct YgoCtx *c);
    void (*target)(struct YgoCtx *c);
    void (*op)(struct YgoCtx *c);
    /* continuous: the value of `which` (YS_*) for obj target once this effect (on src) applies */
    int (*stat)(const struct YgoState *s, unsigned src, unsigned target, int which, int value);
    /* continuous: does the rule (YR_*) apply to a, b? (for YR_EXTRA_ATTACK: how many more) */
    int (*rule)(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b);
    int8_t ai;      /* how much the AI wants it (0: let the AI work it out; negative: rather not) */
};
struct YgoCtx
{
    struct YgoState *s;
    const struct YgoEffect *e;
    uint8_t obj, player, eventObj;
    struct YgoLink *link;
};

/* ---- the effect table (ygo_fx.c) ---- */
const struct YgoEffect *Ygo_Effects(unsigned card, unsigned *count);
int Ygo_CardHasRules(unsigned card);   /* any effect listed for it */

/* ---- looking (ygo_duel.c) ---- */
static inline const struct YgoCard *Ygo_ObjCard(const struct YgoState *s, unsigned o) { return Ygo_Card(s->objs[o].card); }
static inline unsigned Ygo_Opp(unsigned p) { return p ^ 1; }
int Ygo_OnField(const struct YgoState *s, unsigned o);
int Ygo_FaceUpMonster(const struct YgoState *s, unsigned o);
int Ygo_IsMonsterObj(const struct YgoState *s, unsigned o);   /* a monster card (anywhere) */
unsigned Ygo_MonsterCount(const struct YgoState *s, unsigned player);
unsigned Ygo_FreeZones(const struct YgoState *s, unsigned player, int spells);
int Ygo_Rule(const struct YgoState *s, unsigned rule, unsigned a, unsigned b);
int Ygo_EffectsApply(const struct YgoState *s, unsigned o);   /* its effects aren't negated */
/* The objects a player has in a location (returns how many, into out) */
unsigned Ygo_List(const struct YgoState *s, unsigned player, unsigned location, uint8_t *out);
int Ygo_Exists(const struct YgoState *s, unsigned player, unsigned location, int (*filter)(const struct YgoState *, unsigned));
int Ygo_CanTarget(const struct YgoState *s, unsigned o, unsigned player);

/* ---- asking (only inside a unit: the engine runs the unit again with the answer) ---- */
/* min..max objects from list (count); returns how many were picked into out */
unsigned Ygo_AskCards(struct YgoState *s, unsigned player, unsigned prompt, uint16_t card, const uint8_t *list,
                      unsigned count, unsigned min, unsigned max, uint8_t *out);
/* one object matching filter in the location(s) of player(s) (2 = both); NO_OBJ if none (and optional) */
unsigned Ygo_AskOne(struct YgoState *s, unsigned asker, unsigned prompt, uint16_t card, unsigned player, unsigned locations,
                    int (*filter)(const struct YgoState *, unsigned), int optional);
int Ygo_AskYesNo(struct YgoState *s, unsigned player, unsigned prompt, uint16_t card);
unsigned Ygo_AskOption(struct YgoState *s, unsigned player, unsigned prompt, uint16_t card, const uint8_t *options,
                       unsigned count);   /* prompt: what the options are (YGO_P_COIN_CALL: 1 heads 0 tails...) */
unsigned Ygo_AskPosition(struct YgoState *s, unsigned player, uint16_t card, unsigned allowed /* bit per item */);
/* locations for Ygo_AskOne / filters: bits */
enum { YL_HAND = 1, YL_MZONE = 2, YL_SZONE = 4, YL_GRAVE = 8, YL_DECK = 16, YL_BANISHED = 32, YL_EXTRA = 64 };

/* ---- doing ---- */
uint32_t Ygo_Rand(struct YgoState *s, unsigned below);
void Ygo_Event(struct YgoState *s, unsigned type, unsigned player, unsigned obj, unsigned target, int value, unsigned flags);
unsigned Ygo_Draw(struct YgoState *s, unsigned player, unsigned count);
void Ygo_Damage(struct YgoState *s, unsigned player, int amount, int battle);
void Ygo_Recover(struct YgoState *s, unsigned player, int amount);
int Ygo_Destroy(struct YgoState *s, unsigned o, unsigned reason);     /* 1 if destroyed */
void Ygo_SendToGrave(struct YgoState *s, unsigned o, unsigned reason);
void Ygo_ToHand(struct YgoState *s, unsigned o, unsigned reason);
void Ygo_ToDeck(struct YgoState *s, unsigned o, int top);
void Ygo_Banish(struct YgoState *s, unsigned o, unsigned reason);
void Ygo_Discard(struct YgoState *s, unsigned o, unsigned reason);
/* Special Summon o for player in pos (0 ATK, 1 DEF, 2 face-down DEF) from wherever it is */
int Ygo_SpecialSummon(struct YgoState *s, unsigned o, unsigned player, unsigned pos, unsigned type);
int Ygo_CanSpecialSummon(const struct YgoState *s, unsigned o, unsigned player);
unsigned Ygo_MakeToken(struct YgoState *s, unsigned card, unsigned player, unsigned pos);
void Ygo_ChangePosition(struct YgoState *s, unsigned o, unsigned pos);   /* 0 ATK, 1 DEF, 2 face-down DEF */
void Ygo_FlipFaceUp(struct YgoState *s, unsigned o, int byBattle);
void Ygo_TakeControl(struct YgoState *s, unsigned o, unsigned player);
void Ygo_AddStat(struct YgoState *s, unsigned o, int atk, int def, int untilEndOfTurn);
void Ygo_Equip(struct YgoState *s, unsigned equip, unsigned monster);
void Ygo_ShuffleDeck(struct YgoState *s, unsigned player);
/* A lasting rule (YR_*) for scope (YGO_SCOPE_*: player, obj or player's monsters) until the end of the turn turns from
 * now (0: this turn) or YGO_UNTIL_* */
void Ygo_AddLasting(struct YgoState *s, unsigned kind, unsigned scope, unsigned player, unsigned obj, unsigned turns,
                    int value);
int Ygo_Lasting(const struct YgoState *s, unsigned kind, unsigned player);
void Ygo_NegateSummon(struct YgoState *s, unsigned o);
/* The monsters a player may Tribute for a Tribute Summon */
unsigned Ygo_Tributes(const struct YgoState *s, unsigned player, uint8_t *out);
/* Reveal / let player look at cards */
void Ygo_Reveal(struct YgoState *s, unsigned o);
void Ygo_Look(struct YgoState *s, unsigned player, unsigned o);
unsigned Ygo_Coin(struct YgoState *s, unsigned player);
unsigned Ygo_Dice(struct YgoState *s, unsigned player);
void Ygo_NegateLink(struct YgoState *s, unsigned link, int destroy);
/* Win right now (Exodia, effects) */
void Ygo_Win(struct YgoState *s, unsigned player, unsigned reason);
/* Fusion and Ritual Summons (Polymerization, ritual spells) */
void Ygo_FusionSummon(struct YgoCtx *c);
void Ygo_RitualSummon(struct YgoCtx *c, unsigned ritualCard);
/* Fusion materials of a Fusion Monster (ygo_fusions.c, generated): up to 3 card ids (or YGO_MAT_* groups) */
#define YGO_MAT_ANY 0x8000      /* | race: any monster of that race (YGO_RACE_*) */
#define YGO_MAT_ATTR 0x4000     /* | attribute */
struct YgoFusion
{
    uint16_t card;
    uint16_t materials[5];
};
const struct YgoFusion *Ygo_FusionOf(unsigned card);
#endif
