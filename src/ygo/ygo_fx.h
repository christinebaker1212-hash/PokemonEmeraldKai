/* Writing Yu-Gi-Oh! card effects (ygo_fx_*.c): the table rows and the helpers most cards share (ygo_fx_lib.c). */
#ifndef MOEMON_YGO_FX_H
#define MOEMON_YGO_FX_H
#include "ygo_internal.h"

/* Table rows: card, kind, Spell Speed, flags, events, can, cost, target, op */
#define FX(card, type, speed, flags, events, can, cost, target, op) \
    {card, type, speed, flags, events, can, cost, target, op, NULL, NULL, 0}
/* A Spell/Trap card's activation */
#define ACT(card, flags, can, cost, target, op) FX(card, YE_ACTIVATE, 0, flags, 0, can, cost, target, op)
/* A trigger (events) */
#define TRIG(card, flags, events, can, target, op) FX(card, YE_TRIGGER, 0, flags, events, can, NULL, target, op)
/* FLIP: */
#define FLIP(card, flags, can, target, op) FX(card, YE_FLIP, 0, flags, 0, can, NULL, target, op)
/* Always on (stat / rule hooks) */
#define PASSIVE(card, stat, rule) {card, YE_CONTINUOUS, 0, 0, 0, NULL, NULL, NULL, NULL, stat, rule, 0}
/* An activation that also keeps hooks (Equip Spells, Field Spells, Continuous cards) */
#define ACT_HOOKS(card, flags, can, cost, target, op, stat, rule) \
    {card, YE_ACTIVATE, 0, flags, 0, can, cost, target, op, stat, rule, 0}

#define S (c->s)
#define ME (c->player)
#define OPP (c->player ^ 1)
#define CARD (c->s->objs[c->obj].card)
#define TARGET(i) (c->link ? c->link->targets[i] : NO_OBJ)

/* Filters (state, object) */
int F_Monster(const struct YgoState *s, unsigned o);         /* a monster on the field */
int F_FaceUp(const struct YgoState *s, unsigned o);          /* face up on the field */
int F_FaceUpMonster(const struct YgoState *s, unsigned o);
int F_FaceDownMonster(const struct YgoState *s, unsigned o);
int F_AttackPos(const struct YgoState *s, unsigned o);       /* face-up Attack Position monster */
int F_DefensePos(const struct YgoState *s, unsigned o);
int F_SpellTrap(const struct YgoState *s, unsigned o);       /* a Spell/Trap card on the field */
int F_Spell(const struct YgoState *s, unsigned o);           /* a Spell card (anywhere) */
int F_Trap(const struct YgoState *s, unsigned o);
int F_MonsterCard(const struct YgoState *s, unsigned o);     /* a Monster card (anywhere) */
int F_FaceUpTrap(const struct YgoState *s, unsigned o);
int F_FaceUpSpell(const struct YgoState *s, unsigned o);
int F_Any(const struct YgoState *s, unsigned o);

/* The objects of player (2: both) in locations (YL_*) passing filter; returns the count */
unsigned Fx_Collect(const struct YgoState *s, unsigned player, unsigned locations,
                    int (*filter)(const struct YgoState *, unsigned), uint8_t *out);
/* Targetable ones only (the effect's controller can target them) */
unsigned Fx_Targets(const struct YgoState *s, unsigned asker, unsigned player, unsigned locations,
                    int (*filter)(const struct YgoState *, unsigned), uint8_t *out);
int Fx_HasTarget(const struct YgoState *s, unsigned asker, unsigned player, unsigned locations,
                 int (*filter)(const struct YgoState *, unsigned));
/* Ask the activator to target count of them into the link (min = count) */
void Fx_Target(struct YgoCtx *c, unsigned player, unsigned locations, int (*filter)(const struct YgoState *, unsigned),
               unsigned count, unsigned prompt);
/* Is the link's target i still there and still fits filter (a targeted card that left can't be affected)? */
int Fx_TargetOk(struct YgoCtx *c, unsigned i, int (*filter)(const struct YgoState *, unsigned));
/* Destroy every object of player (2 both) in locations passing filter; returns how many */
unsigned Fx_DestroyAll(struct YgoState *s, unsigned player, unsigned locations,
                       int (*filter)(const struct YgoState *, unsigned), unsigned reason);
/* Search the deck: the player picks one passing filter and adds it to the hand (shuffles after) */
unsigned Fx_Search(struct YgoState *s, unsigned player, int (*filter)(const struct YgoState *, unsigned), uint16_t card);
int Fx_Name(const struct YgoState *s, unsigned o, unsigned card);   /* that card (by id) */
/* Discard count cards chosen by player (random: chosen at random) */
unsigned Fx_Discard(struct YgoState *s, unsigned player, unsigned count, int random, unsigned reason);
/* Is a face-up card with that id on the player's field (2 either)? */
int Fx_OnFieldFaceUp(const struct YgoState *s, unsigned player, unsigned card);
/* The monster an Equip Card on this object is equipped to (NO_OBJ none) */
unsigned Fx_EquippedTo(const struct YgoState *s, unsigned equip);
unsigned Fx_EquipCount(const struct YgoState *s, unsigned monster);
/* Pay LP as a cost */
int Fx_CanPay(const struct YgoState *s, unsigned player, int lp);
void Fx_Pay(struct YgoState *s, unsigned player, int lp);

/* Generic Equip Spells (ygo_fx_lib.c's table: race / attribute, ATK, DEF) and Field Spells */
int Equip_Can(struct YgoCtx *c);
void Equip_Target(struct YgoCtx *c);
void Equip_Op(struct YgoCtx *c);
int Equip_Stat(const struct YgoState *s, unsigned src, unsigned target, int which, int value);
int Field_Stat(const struct YgoState *s, unsigned src, unsigned target, int which, int value);
#define EQUIP(card) ACT_HOOKS(card, YF_TARGETS, Equip_Can, NULL, Equip_Target, Equip_Op, Equip_Stat, NULL)
#define FIELD(card) ACT_HOOKS(card, 0, NULL, NULL, NULL, NULL, Field_Stat, NULL)

/* Polymerization / ritual spells */
int Fx_CanFusion(const struct YgoState *s, unsigned player, unsigned exclude);
int Fx_CanRitual(const struct YgoState *s, unsigned player, unsigned ritualCard, unsigned exclude);
#endif
