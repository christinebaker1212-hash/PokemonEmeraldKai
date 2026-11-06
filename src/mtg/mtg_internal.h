// Moemon Kai: the Magic engine's own declarations (src/mtg/*.c). Names follow Forge's Java so each part can be
// checked against the class it comes from (docs/MTG_HANDOFF.md "Java to C").
#ifndef GUARD_MTG_INTERNAL_H
#define GUARD_MTG_INTERNAL_H

#include <stddef.h>
#include "mtg/mtg.h"

// ApiType.java: the effects translated so far (the rest make a card unsupported)
enum {
    MTG_API_NONE,
    MTG_API_DEAL_DAMAGE,      // DamageDealEffect
    MTG_API_DAMAGE_ALL,       // DamageAllEffect
    MTG_API_DRAW,             // DrawEffect
    MTG_API_DESTROY,          // DestroyEffect
    MTG_API_DESTROY_ALL,      // DestroyAllEffect
    MTG_API_GAIN_LIFE,        // LifeGainEffect
    MTG_API_LOSE_LIFE,        // LifeLoseEffect
    MTG_API_PUMP,             // PumpEffect
    MTG_API_PUMP_ALL,         // PumpAllEffect
    MTG_API_CHANGE_ZONE,      // ChangeZoneEffect (the targeted, "known" origin form)
    MTG_API_MANA,             // ManaEffect
    MTG_API_DISCARD,          // DiscardEffect (Mode$ Random)
    MTG_API_TAP,              // TapEffect
    MTG_API_UNTAP,            // UntapEffect
    MTG_API_COUNTER,          // CounterEffect
    MTG_API_SACRIFICE,        // SacrificeEffect (Defined$ Self, or one SacValid$ permanent the player chooses)
    MTG_API_PUT_COUNTER,      // CountersPutEffect (P1P1 / M1M1)
    MTG_API_ATTACH,           // AttachEffect: an Aura spell (K:Enchant:Creature) enters attached to its target
    MTG_API_REGENERATE,       // RegenerateEffect: a shield on each creature (Defined$ Self or the target)
    MTG_API_TOKEN,            // TokenEffect: TokenAmount$ tokens of TokenScript$ for TokenOwner$ (Mtg_TokenDef)
    MTG_API_CHARM,            // CharmEffect: "choose one" (Choices$): each mode is offered as its own action
    MTG_API_COUNT
};

enum { MTG_AB_SPELL, MTG_AB_ACTIVATED, MTG_AB_SUB,      // A:SP$, A:AB$, SVar:...:DB$
       MTG_AB_TRIGGERED };                                // a trigger's Execute$ (it may target, as its root)

typedef struct { const char *key, *value; } MtgParam;

// an ability's cost (Cost.java): mana, {T}, sacrificing itself; Sac<n/Type>, Discard<n/Type> (or n at random),
// PayLife<n>, Exile<1/CARDNAME>, Return<1/CARDNAME> (the player picks what to sacrifice / discard: Mtg_Answer)
typedef struct {
    MtgManaCost mana;
    uint8_t tap, sacSelf, exileSelf, returnSelf;
    uint8_t sacN, discardN, discardRandom, payLife;
    const char *sacSpec, *discardSpec;   // Valid specs ("Creature", "Land", "Goblin", "Card")
} MtgCost;
int Mtg_CostPayable(const MtgDuel *d, int player, const MtgCost *c, int source, int casting);
int Mtg_CostCandidates(const MtgDuel *d, int player, const MtgCost *c, int sac, int source, uint16_t *out);

typedef struct {
    uint8_t kind, api;
    uint16_t card;                  // the card it belongs to
    uint16_t sub;                   // SubAbility$: the next ability in the chain (0xFFFF none)
    uint16_t paramFirst, paramCount;
    MtgCost cost;
    const char *tgt;                // ValidTgts$ (NULL: no target)
    uint8_t tgtStack;               // TargetType$ Spell: targets a spell on the stack
    uint8_t tgtZone;                // where its targets are (MTG_ZONE_*: the battlefield, or ChangeZone's Origin$)
    uint8_t choiceCount;            // Charm: its modes (Choices$), each an ability of the charm's kind and cost
    uint16_t choices[5];
    uint16_t charmOf;               // a Charm's mode: the charm (0xFFFF: none)
} MtgAbility;

// mtg_script.c
extern MtgAbility *gMtgAbilities;
extern MtgParam *gMtgParams;
extern MtgParam *gMtgSvars;
const char *Mtg_Param(const MtgAbility *ab, const char *key);   // NULL if absent
const char *Mtg_Svar(int card, const char *name);
int Mtg_ParseManaCost(const char *s, MtgManaCost *out);         // 0 if it has symbols the engine doesn't know
int Mtg_KeywordByName(const char *name);                        // MTG_KW_* or -1
int Mtg_IsSubtype(const char *t, size_t n);                      // a subtype some card of the pool has
char *Mtg_Token(char **p, char sep);                            // splits text in place, like strtok_r
int Mtg_ParseParams(char *s, int *first);                       // "K$ V | K$ V" -> gMtgParams; returns the count
extern const char *const gMtgApiNames[MTG_API_COUNT];

// mtg_trigger.c: triggered abilities (TriggerHandler.java, TriggerType and each Trigger<Mode>.java)
enum {
    MTG_TRIG_NONE, MTG_TRIG_CHANGES_ZONE, MTG_TRIG_PHASE, MTG_TRIG_SPELL_CAST, MTG_TRIG_DAMAGE_DONE, MTG_TRIG_ATTACKS,
    MTG_TRIG_ATTACKER_BLOCKED, MTG_TRIG_ATTACKER_BLOCKED_BY, MTG_TRIG_BLOCKS, MTG_TRIG_ATTACKER_UNBLOCKED,
    MTG_TRIG_COUNT
};
typedef struct {
    uint8_t mode, optional, zones;  // zones: where its card must be (bits 1 << MTG_ZONE_*)
    uint16_t card, execute;         // its card; the ability it puts on the stack
    uint16_t paramFirst, paramCount;
} MtgTrigger;
extern MtgTrigger *gMtgTriggers;
extern const char *const gMtgTriggerModes[MTG_TRIG_COUNT];
// what happened, for the triggers to look at
typedef struct {
    uint8_t mode;
    uint16_t obj, obj2;             // the card it is about (moved, cast, attacking, blocking, the damage's source);
                                    // obj2: the other one (the blocker, the attacker blocked, the damaged creature)
    uint8_t player;                 // Phase: the active player; SpellCast: the caster; DamageDone: the damaged player
                                    // (0xFF: a creature); combat: the defending player
    uint8_t from, to, step, combat; // ChangesZone's zones; Phase's step; DamageDone: combat damage
    uint8_t objCtrl;                // obj's controller as it happened
    int16_t amount;                 // DamageDone: how much
} MtgTrigEvent;
// T: line text -> a trigger of card c (its Execute$ parsed as an MTG_AB_TRIGGERED ability); a part it doesn't
// translate makes the card unsupported
void Mtg_ParseTrigger(MtgCardDef *c, int cardId, char *text);
void Mtg_CheckTriggers(MtgDuel *d, const MtgTrigEvent *ev);     // the triggers it sets off join d->pending
int Mtg_TriggerStillTrue(const MtgDuel *d, const MtgStackEntry *e);   // an intervening "if" (IsPresent$) on resolving
const char *Mtg_TriggerParam(const MtgTrigger *t, const char *key);
int Mtg_ParseAbilityText(MtgCardDef *c, int cardId, char *text, int kind);   // (mtg_script.c, for the triggers)
void Mtg_Unsupported(MtgCardDef *c, const char *why);

// mtg_static.c: static abilities (StaticAbilityContinuous.java: power, toughness, keywords of what they affect)
#define MTG_STATIC_NO_SET (-1000)
enum { MTG_STATIC_CONTINUOUS, MTG_STATIC_CANT_BLOCK, MTG_STATIC_CANT_ATTACK, MTG_STATIC_CANT_BLOCK_BY,
       MTG_STATIC_RAISE_COST, MTG_STATIC_REDUCE_COST,
       MTG_STATIC_ETB_TAPPED, MTG_STATIC_NO_UNTAP };   // (two replacement effects, R: lines, kept here: they test alike)
typedef struct {
    uint16_t card;
    uint8_t mode;                   // MTG_STATIC_*
    uint8_t self;                   // AffectedDefined$ Self (Affected$, if any, is then a condition on itself)
    uint8_t enchanted;              // AffectedDefined$ Enchanted: the creature its Aura is attached to
    const char *affected;           // Affected$ (CantBlock / CantAttack: ValidCard$; CantBlockBy: ValidAttacker$)
    const char *blocker;            // CantBlockBy: ValidBlocker$
    const char *present;            // IsPresent$: only while the battlefield has (PresentCompare$, default GE1)...
    uint8_t presentOp;              // 0 GE, 1 EQ, 2 LE
    uint8_t presentNum;
    int8_t addPower, addToughness;
    const char *powerX, *toughX;    // AddPower$ X / AddToughness$ -X: an SVar counted now (Count$Valid ..., Number$)
    const char *setPowerX, *setToughX;   // SetPower$ X / SetToughness$ X counted (a */* creature's CharacteristicDefining$)
    int16_t setPower, setToughness; // MTG_STATIC_NO_SET: none
    uint32_t addKeywords;
    uint8_t addLandwalk, addProtection;
    uint8_t activator;              // Raise / ReduceCost: 0 anyone, 1 its controller ("You"), 2 the opponent
    uint8_t costGeneric, costPip;   // ...: Amount$ (generic) or Cost$ (one colored symbol: costPip = MTG_W.. + 1)
} MtgStatic;
typedef struct {
    int addPower, addToughness, setPower, setToughness;
    uint32_t keywords;
    uint8_t landwalk, protection;
} MtgStaticMods;
extern MtgStatic *gMtgStatics;
void Mtg_ParseStatic(MtgCardDef *c, int cardId, char *text);
void Mtg_ParseReplacement(MtgCardDef *c, int cardId, char *text);   // R: lines (only the forms above)
void Mtg_StaticMods(const MtgDuel *d, int obj, MtgStaticMods *m);
int Mtg_StaticForbids(const MtgDuel *d, int mode, int obj, int other);
void Mtg_SpellCost(const MtgDuel *d, int player, int obj, MtgManaCost *out);   // its mana cost with Raise / ReduceCost   // CantBlock / CantAttack (obj), CantBlockBy
                                                                          // (obj attacking, other blocking)

// mtg_valid.c: Forge's "Valid" properties (CardProperty.java, PlayerProperty.java)
int Mtg_ValidObj(const MtgDuel *d, int obj, const char *spec, int sourceObj, int controller);
int Mtg_ValidPlayer(const MtgDuel *d, int player, const char *spec, int controller);
int Mtg_ValidSpecKnown(const char *spec);                        // every alternative and property understood
uint8_t Mtg_ObjColors(const MtgDuel *d, int obj);
int Mtg_CanTarget(const MtgDuel *d, int obj, int sourceObj);     // shroud, protection

// mtg_mana.c: paying costs from the pool and untapped sources (ManaPool.java, ComputerUtilMana's auto-pay)
int Mtg_CanPay(const MtgDuel *d, int player, const MtgManaCost *cost, int x, int excludeObj);
int Mtg_Pay(MtgDuel *d, int player, const MtgManaCost *cost, int x, int excludeObj);
int Mtg_MaxX(const MtgDuel *d, int player, const MtgManaCost *cost, int excludeObj);
int Mtg_ManaAbilityColors(const MtgDuel *d, int obj, int *amount);   // a permanent's {T}: add mana (0: none usable)
int Mtg_ProducedColors(const char *produced);                       // Produced$ "G", "Combo W U", "Any" (-1 unknown)

// mtg_game.c: GameAction.java (zone changes, state-based actions), the turn, the stack
void Mtg_Event(MtgDuel *d, int kind, int a, int b, int c);
void Mtg_MoveTo(MtgDuel *d, int obj, int zone);
void Mtg_Draw(MtgDuel *d, int player, int count);
void Mtg_DealDamage(MtgDuel *d, int sourceObj, MtgTarget to, int amount, int combat);
void Mtg_GainLife(MtgDuel *d, int player, int amount);
void Mtg_Destroy(MtgDuel *d, int obj, int noRegen);   // a regeneration shield saves it unless noRegen
void Mtg_Tap(MtgDuel *d, int obj, int tapped);
uint32_t Mtg_Random(MtgDuel *d, uint32_t n);
int Mtg_TopOfLibrary(const MtgDuel *d, int player);
void Mtg_RefreshQuery(MtgDuel *d);
int Mtg_CanBlock(const MtgDuel *d, int blocker, int attacker);   // CombatUtil.canBlock
// a choice while resolving (d->query becomes MTG_Q_CHOOSE for player)
void Mtg_AskChoice(MtgDuel *d, int player, int choice, const MtgManaCost *pay);

// mtg_effects.c: one resolve per API (forge/game/ability/effects)
typedef struct {
    MtgStackEntry *e;
    int controller, source;
} MtgResolveCtx;
// one ability of a resolving chain: 1 done, 0 waiting for a choice (d->query is set; d->resChoice will hold it)
int Mtg_ResolveStep(MtgDuel *d, MtgResolveCtx *ctx, int ability);
void Mtg_AddCounters(MtgDuel *d, int obj, int n);
// a Defined$ name of a triggered ability ("TriggeredCard", "TriggeredTargetController"...) understood for this mode
int Mtg_TriggeredDefinedKnown(int mode, const char *name);
void Mtg_Sacrifice(MtgDuel *d, int obj);
int Mtg_Amount(const MtgDuel *d, const MtgResolveCtx *ctx, int card, const char *value, int *ok);
int Mtg_ApiKnown(const char *name);           // MTG_API_* or 0
int Mtg_ApiParamsKnown(const MtgAbility *ab, const char **why);   // 0: something it doesn't handle yet (*why)

#endif
