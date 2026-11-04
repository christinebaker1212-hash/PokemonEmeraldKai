// Moemon Kai: the Magic engine's own declarations (src/mtg/*.c). Names follow Forge's Java so each part can be
// checked against the class it comes from (docs/MTG_HANDOFF.md "Java to C").
#ifndef GUARD_MTG_INTERNAL_H
#define GUARD_MTG_INTERNAL_H

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
    MTG_API_COUNT
};

enum { MTG_AB_SPELL, MTG_AB_ACTIVATED, MTG_AB_SUB };     // A:SP$, A:AB$, SVar:...:DB$

typedef struct { const char *key, *value; } MtgParam;

// an ability's cost (Cost.java): mana, {T}, sacrificing itself
typedef struct {
    MtgManaCost mana;
    uint8_t tap, sacSelf;
} MtgCost;

typedef struct {
    uint8_t kind, api;
    uint16_t card;                  // the card it belongs to
    uint16_t sub;                   // SubAbility$: the next ability in the chain (0xFFFF none)
    uint16_t paramFirst, paramCount;
    MtgCost cost;
    const char *tgt;                // ValidTgts$ (NULL: no target)
    uint8_t tgtStack;               // TargetType$ Spell: targets a spell on the stack
} MtgAbility;

// mtg_script.c
extern MtgAbility *gMtgAbilities;
extern MtgParam *gMtgParams;
extern MtgParam *gMtgSvars;
const char *Mtg_Param(const MtgAbility *ab, const char *key);   // NULL if absent
const char *Mtg_Svar(int card, const char *name);
int Mtg_ParseManaCost(const char *s, MtgManaCost *out);         // 0 if it has symbols the engine doesn't know
int Mtg_KeywordByName(const char *name);
char *Mtg_Token(char **p, char sep);                            // splits text in place, like strtok_r                         // MTG_KW_* or -1
extern const char *const gMtgApiNames[MTG_API_COUNT];

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
void Mtg_Destroy(MtgDuel *d, int obj);
void Mtg_Tap(MtgDuel *d, int obj, int tapped);
uint32_t Mtg_Random(MtgDuel *d, uint32_t n);
int Mtg_TopOfLibrary(const MtgDuel *d, int player);
void Mtg_RefreshQuery(MtgDuel *d);
int Mtg_CanBlock(const MtgDuel *d, int blocker, int attacker);   // CombatUtil.canBlock

// mtg_effects.c: one resolve per API (forge/game/ability/effects)
typedef struct {
    MtgStackEntry *e;
    int controller, source;
} MtgResolveCtx;
void Mtg_ResolveAbility(MtgDuel *d, MtgResolveCtx *ctx, int ability);
int Mtg_Amount(const MtgDuel *d, const MtgResolveCtx *ctx, int card, const char *value, int *ok);
int Mtg_ApiKnown(const char *name);           // MTG_API_* or 0
int Mtg_ApiParamsKnown(const MtgAbility *ab, const char **why);   // 0: something it doesn't handle yet (*why)

#endif
