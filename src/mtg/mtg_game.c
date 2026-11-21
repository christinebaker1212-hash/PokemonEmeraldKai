// Moemon Kai: a Magic duel: the turn and its steps (PhaseHandler.java), priority and the stack (MagicStack.java),
// zone changes and state-based actions (GameAction.java), combat (Combat.java, CombatUtil.java's blocking rules,
// the damage assignment of ComputerUtilCombat's distributeAIDamage kept to the plain order rule).
// The duel runs until it needs a decision: Mtg_Query says what, Mtg_Answer gives it.
#include <string.h>
#include "mtg_internal.h"

#define MTG_HAND_SIZE   7
#define MTG_START_LIFE  20
#define MTG_TURN_LIMIT  200     // a duel this long is a draw (no card can make one loop forever yet)

static void Run(MtgDuel *d);

uint32_t Mtg_Random(MtgDuel *d, uint32_t n)
{
    uint32_t x = d->rng;   // xorshift32
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    d->rng = x;
    return n ? x % n : x;
}

void Mtg_Event(MtgDuel *d, int kind, int a, int b, int c)
{
    MtgEvent *e = &d->events[d->eventCount % MTG_EVENT_RING];
    e->kind = (uint8_t)kind;
    e->a = (int16_t)a;
    e->b = (int16_t)b;
    e->c = (int16_t)c;
    d->eventCount++;
    d->hash = (d->hash ^ ((uint32_t)kind | ((uint32_t)a & 0xFFFFu) << 8)) * 16777619u;
    d->hash = (d->hash ^ (((uint32_t)b & 0xFFFFu) | ((uint32_t)c & 0xFFFFu) << 16)) * 16777619u;
}

const MtgEvent *Mtg_EventAt(const MtgDuel *d, uint32_t n)
{
    if (n >= d->eventCount || d->eventCount - n > MTG_EVENT_RING)
        return NULL;
    return &d->events[n % MTG_EVENT_RING];
}

// (the layers: the printed value or a static's SetPower, then the statics' and pumps' changes, then the counters)
// what its other counters add to power (which 0) or toughness (1): P2P2, P1P0, M0M1...
static int CounterPT(const MtgObj *o, int which)
{
    int k, sum = 0;
    for (k = 0; k < 4; k++)
        if (o->ctrKind[k] && o->ctrNum[k]) {
            int p, t;
            Mtg_CounterPT(o->ctrKind[k], &p, &t);
            sum += (which ? t : p) * o->ctrNum[k];
        }
    return sum;
}

int Mtg_Power(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    int base = o->animUntil && o->animPower != MTG_ANIM_NO_PT ? o->animPower : Mtg_Card(o->def)->power;
    return (m.setPower != MTG_STATIC_NO_SET ? m.setPower : base) + m.addPower + o->counters11 + o->tempPower
         + o->permPower + CounterPT(o, 0);
}

int Mtg_Toughness(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    int base = o->animUntil && o->animToughness != MTG_ANIM_NO_PT ? o->animToughness : Mtg_Card(o->def)->toughness;
    return (m.setToughness != MTG_STATIC_NO_SET ? m.setToughness : base) + m.addToughness + o->counters11
         + o->tempToughness + o->permToughness + CounterPT(o, 1);
}

uint32_t Mtg_Keywords(const MtgDuel *d, int obj)
{
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return (Mtg_Card(d->objs[obj].def)->keywords | d->objs[obj].tempKeywords | d->objs[obj].permKeywords | m.keywords
            | (d->objs[obj].animUntil ? d->objs[obj].animKeywords : 0))
         & ~(d->objs[obj].loseKeywords | d->objs[obj].permLoseKeywords);   // (Debuff: lost)
}

uint8_t Mtg_ObjTypes(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    return Mtg_Card(o->def)->types | (o->animUntil && o->zone == MTG_ZONE_BATTLEFIELD ? o->animTypes : 0);
}

uint8_t Mtg_Landwalk(const MtgDuel *d, int obj)
{
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return Mtg_Card(d->objs[obj].def)->landwalk | m.landwalk
         | (((d->objs[obj].tempKeywords | d->objs[obj].permKeywords) >> MTG_KWX_WALK_SHIFT) & 0x1F);
}

uint8_t Mtg_Protection(const MtgDuel *d, int obj)
{
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return Mtg_Card(d->objs[obj].def)->protection | m.protection | d->objs[obj].tempProtection;
}

static int IsCreature(const MtgDuel *d, int obj)
{
    return d->objs[obj].zone == MTG_ZONE_BATTLEFIELD && (Mtg_ObjTypes(d, obj) & MTG_T_CREATURE);
}

int Mtg_TopOfLibrary(const MtgDuel *d, int player)
{
    const MtgPlayer *p = &d->players[player];
    return p->libraryCount ? p->library[p->libraryCount - 1] : MTG_NO_OBJ;
}

static void ResetPermanentState(MtgObj *o)
{
    o->tapped = o->sick = o->attacking = o->blocked = o->dealtFirst = 0;
    o->blocking = MTG_NO_OBJ;
    o->damage = 0;
    o->tempPower = o->tempToughness = 0;
    o->tempKeywords = 0;
    o->permPower = o->permToughness = 0;
    o->tempProtection = 0;
    o->loseKeywords = o->permLoseKeywords = 0;
    o->permKeywords = 0;
    o->counters11 = 0;
    memset(o->ctrKind, 0, sizeof(o->ctrKind));
    memset(o->ctrNum, 0, sizeof(o->ctrNum));
    o->attachedTo = MTG_NO_OBJ;
    o->regen = 0;
    o->animUntil = 0;
    o->effCtrl = o->effUntil = 0;
    o->effSource = MTG_NO_OBJ;
    o->usedAb[0] = o->usedAb[1] = 0xFFFF;
    o->usedN[0] = o->usedN[1] = 0;
    o->damagedBy[0] = o->damagedBy[1] = o->damagedBy[2] = MTG_NO_OBJ;
}

void Mtg_Remember(MtgDuel *d, int obj)
{
    if (Mtg_IsRemembered(d, obj) || d->remCount >= sizeof(d->rem) / sizeof(d->rem[0]))
        return;
    d->rem[d->remCount++] = (uint16_t)obj;
}

int Mtg_IsRememberedPlayer(const MtgDuel *d, int player)
{
    return Mtg_IsRemembered(d, MTG_REM_PLAYER + player);
}

const MtgEffect *gMtgEffCtx;

int Mtg_ChosenCard(const MtgDuel *d)
{
    uint16_t c1 = gMtgEffCtx ? gMtgEffCtx->chosen1 : d->chosen1;
    uint32_t seq = gMtgEffCtx ? gMtgEffCtx->chosenSeq : d->chosenSeq;
    if (!c1 || d->objs[c1 - 1].seq != seq)
        return MTG_NO_OBJ;   // (none, or it has changed zones since: a new object)
    return c1 - 1;
}

int Mtg_EffectAlive(const MtgDuel *d, const MtgEffect *e)
{
    int k;
    if (e->used)
        return 0;
    if (e->until == MTG_EFF_HOST_OR_EOT || e->until == MTG_EFF_HOST || e->until == MTG_EFF_UNTIL_UNTAPS) {
        const MtgObj *s = &d->objs[e->source];   // (its source left the battlefield, or untapped: over)
        if (s->zone != MTG_ZONE_BATTLEFIELD || s->seq != e->sourceSeq || (e->until == MTG_EFF_UNTIL_UNTAPS && !s->tapped))
            return 0;
    }
    if (e->exileOnMoved)   // ExileOnMoved$ Battlefield: what it remembers left the battlefield
        for (k = 0; k < e->remCount; k++)
            if (e->rem[k] < MTG_REM_PLAYER && (d->objs[e->rem[k]].seq != e->remSeq[k]
                                               || d->objs[e->rem[k]].zone != MTG_ZONE_BATTLEFIELD))
                return 0;
    return 1;
}

// Effects that ended: gone (KIND: also those lasting until it, MTG_EFF_* or 0xFF for none)
static void PruneEffects(MtgDuel *d, int kind, int player)
{
    int i, n = 0;
    for (i = 0; i < d->effCount; i++) {
        const MtgEffect *e = &d->effects[i];
        int ends = !Mtg_EffectAlive(d, e)
                || (kind == MTG_EFF_EOT && (e->until == MTG_EFF_EOT || e->until == MTG_EFF_HOST_OR_EOT))
                || (kind == MTG_EFF_NEXT_TURN && e->until == MTG_EFF_NEXT_TURN && e->controller == player)
                || (kind == MTG_EFF_END_COMBAT && e->until == MTG_EFF_END_COMBAT)
                || (kind == MTG_EFF_AFTER_UNTAP && e->until == MTG_EFF_AFTER_UNTAP && e->controller == player);
        if (!ends)
            d->effects[n++] = *e;
    }
    d->effCount = (uint8_t)n;
}

int Mtg_IsRemembered(const MtgDuel *d, int obj)
{
    int i;
    if (gMtgEffCtx) {   // (an Effect's own list: a card that has moved since is a new one)
        for (i = 0; i < gMtgEffCtx->remCount; i++)
            if (gMtgEffCtx->rem[i] == obj && (obj >= MTG_REM_PLAYER || d->objs[obj].seq == gMtgEffCtx->remSeq[i]))
                return 1;
        return 0;
    }
    for (i = 0; i < d->remCount; i++)
        if (d->rem[i] == obj)
            return 1;
    return 0;
}

// times OBJ's ABILITY (a Charm's mode counts as the charm) was activated this turn
int Mtg_AbilityUses(const MtgDuel *d, int obj, int ability)
{
    const MtgObj *o = &d->objs[obj];
    int i;
    if (gMtgAbilities[ability].charmOf != 0xFFFF)
        ability = gMtgAbilities[ability].charmOf;
    for (i = 0; i < 2; i++)
        if (o->usedAb[i] == ability)
            return o->usedN[i];
    return 0;
}

static void CountUse(MtgDuel *d, int obj, int ability)
{
    MtgObj *o = &d->objs[obj];
    int i;
    if (gMtgAbilities[ability].charmOf != 0xFFFF)
        ability = gMtgAbilities[ability].charmOf;
    if (!gMtgAbilities[ability].actLimit)
        return;
    for (i = 0; i < 2; i++)
        if (o->usedAb[i] == ability) {
            o->usedN[i]++;
            return;
        }
    for (i = 0; i < 2; i++)
        if (o->usedAb[i] == 0xFFFF || !o->usedN[i]) {
            o->usedAb[i] = (uint16_t)ability;
            o->usedN[i] = 1;
            return;
        }
}

// an activated ability's own timing (ActivationLimit$, SorcerySpeed$, PlayerTurn$, ActivationPhases$)
static int SorceryTiming(const MtgDuel *d, int player);
static int ActivationTimingOk(const MtgDuel *d, int obj, int ability, int player)
{
    const MtgAbility *ab = &gMtgAbilities[ability];
    if (ab->charmOf != 0xFFFF)
        ab = &gMtgAbilities[ab->charmOf];
    if (ab->actLimit && Mtg_AbilityUses(d, obj, ability) >= ab->actLimit)
        return 0;
    if (ab->actSorcery && !SorceryTiming(d, player))
        return 0;
    if (ab->actMyTurn && d->active != player)
        return 0;
    if (ab->actNotMyTurn && d->active == player)   // (Activator$ You.NonActive)
        return 0;
    if (ab->actSteps && !(ab->actSteps & (1u << d->step)))
        return 0;
    return 1;
}

// GameAction.changeZone: a card moves (to the library's top when it goes there); a permanent leaving forgets its state
void Mtg_MoveTo(MtgDuel *d, int obj, int zone)
{
    MtgObj *o = &d->objs[obj];
    int from = o->zone, i;
    MtgTrigEvent ev;
    if (from == MTG_ZONE_BATTLEFIELD && zone == MTG_ZONE_GRAVEYARD && Mtg_Card(o->def)->diesToLibraryTop)
        zone = MTG_ZONE_LIBRARY;   // "if it would die, put it on top of its owner's library instead" (its top: the end)
    if (from == MTG_ZONE_STACK && o->castMode == 2)
        zone = MTG_ZONE_EXILE;     // (Flashback: exiled instead, wherever it would go from the stack)
    if (from == MTG_ZONE_BATTLEFIELD || (from == MTG_ZONE_STACK && zone != MTG_ZONE_BATTLEFIELD))
        o->kicked = o->castMode = 0;   // (cast kicked: only while it's the spell, or the permanent it became)
    memset(&ev, 0, sizeof(ev));
    ev.mode = MTG_TRIG_CHANGES_ZONE;
    ev.obj = (uint16_t)obj;
    ev.obj2 = MTG_NO_OBJ;
    ev.player = 0xFF;
    ev.from = (uint8_t)from;
    ev.to = (uint8_t)zone;
    ev.objCtrl = o->controller;
    if (from == MTG_ZONE_BATTLEFIELD)   // leaving the battlefield: the triggers look back at it as it was (603.10a)
        Mtg_CheckTriggers(d, &ev);
    if (o->faceDownOf && zone != MTG_ZONE_BATTLEFIELD && zone != MTG_ZONE_STACK) {
        o->def = (uint16_t)(o->faceDownOf - 1);   // (Morph: a face-down card is turned face up as it leaves)
        o->faceDownOf = 0;
    }
    if (from == MTG_ZONE_LIBRARY) {
        MtgPlayer *p = &d->players[o->owner];
        for (i = 0; i < p->libraryCount; i++)
            if (p->library[i] == obj) {
                memmove(&p->library[i], &p->library[i + 1], (p->libraryCount - i - 1) * sizeof(p->library[0]));
                p->libraryCount--;
                break;
            }
    }
    if (from == MTG_ZONE_BATTLEFIELD || zone == MTG_ZONE_BATTLEFIELD) {
        // a blocker leaving: its attacker stays blocked; an attacker leaving: its blockers block nothing
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].blocking == obj)
                d->objs[i].blocking = MTG_NO_OBJ;
        ResetPermanentState(o);
        o->sick = zone == MTG_ZONE_BATTLEFIELD;   // (a permanent arriving: under its controller since this turn)
    }
    if (zone == MTG_ZONE_BATTLEFIELD) {   // a new object: nothing chosen for it yet ("as it enters": queued, Run)
        o->chosenType = 0;
        o->chosenColors = o->chosenPlayer = 0;
        o->chosenNumber = 0;
        o->echoDue = 1;
        for (i = 0; i < 2; i++)   // (each "as it enters" of its card: obj, the second with bit 15)
            if (Mtg_Card(o->def)->etbRepl[i] && d->etbCount < 8)
                d->etbQueue[d->etbCount++] = (uint16_t)(obj | (i << 15));
    }
    if (zone != MTG_ZONE_BATTLEFIELD)
        o->controller = o->owner, o->ctrlBase = 0;
    o->zone = (uint8_t)zone;
    if (d->remWatch && (d->remWatch == 0xFF || d->remWatch == zone + 1))   // (RememberChanged$, ...Milled$...)
        Mtg_Remember(d, obj);
    o->seq = ++d->seq;
    if (zone == MTG_ZONE_LIBRARY) {
        MtgPlayer *p = &d->players[o->owner];
        p->library[p->libraryCount++] = (uint16_t)obj;
    }
    Mtg_Event(d, MTG_EV_ZONE, obj, from, zone);
    if (zone == MTG_ZONE_BATTLEFIELD && Mtg_StaticForbids(d, MTG_STATIC_ETB_TAPPED, obj, MTG_NO_OBJ))
        Mtg_Tap(d, obj, 1);   // (it enters tapped: ETBTapped)
    if (zone == MTG_ZONE_BATTLEFIELD && Mtg_Card(o->def)->etbKind && Mtg_Card(o->def)->etbNum
        && (!Mtg_Card(o->def)->etbCheck || Mtg_CheckSvar(d, o->def, obj, o->controller, Mtg_Card(o->def)->etbCheck, NULL)))
        Mtg_AddCountersKind(d, obj, Mtg_Card(o->def)->etbKind, Mtg_Card(o->def)->etbNum);   // (etbCounter:TYPE:N)
    if (from != MTG_ZONE_BATTLEFIELD)
        Mtg_CheckTriggers(d, &ev);
}

static void TrigEvent(MtgDuel *d, int mode, int obj, int obj2, int player)
{
    MtgTrigEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.mode = (uint8_t)mode;
    ev.obj = (uint16_t)obj;
    ev.obj2 = (uint16_t)obj2;
    ev.player = (uint8_t)player;
    ev.step = d->step;
    ev.objCtrl = obj != MTG_NO_OBJ ? d->objs[obj].controller : 0xFF;
    Mtg_CheckTriggers(d, &ev);
}

// "becomes the target of a spell or ability": T, by SOURCE (the spell, or the ability's source) of CTRL's
static void TargetEvent(MtgDuel *d, MtgTarget t, int source, int ctrl)
{
    MtgTrigEvent ev;
    if (t.kind != MTG_TGT_OBJ && t.kind != MTG_TGT_PLAYER)
        return;
    memset(&ev, 0, sizeof(ev));
    ev.mode = MTG_TRIG_BECOMES_TARGET;
    ev.obj = t.kind == MTG_TGT_OBJ ? t.id : MTG_NO_OBJ;
    ev.obj2 = (uint16_t)source;
    ev.player = t.kind == MTG_TGT_PLAYER ? (uint8_t)t.id : 0xFF;
    ev.step = d->step;
    ev.objCtrl = (uint8_t)ctrl;
    Mtg_CheckTriggers(d, &ev);
}

void Mtg_Draw(MtgDuel *d, int player, int count)
{
    while (count-- > 0) {
        int top = Mtg_TopOfLibrary(d, player);
        if (top == MTG_NO_OBJ) {
            d->players[player].drewFromEmpty = 1;
            return;
        }
        Mtg_MoveTo(d, top, MTG_ZONE_HAND);
        Mtg_Event(d, MTG_EV_DRAW, player, top, 0);
    }
}

void Mtg_Tap(MtgDuel *d, int obj, int tapped)
{
    if (d->objs[obj].tapped != tapped) {
        d->objs[obj].tapped = (uint8_t)tapped;
        Mtg_Event(d, MTG_EV_TAP, obj, tapped, 0);
        if (tapped && d->objs[obj].zone == MTG_ZONE_BATTLEFIELD)   // ("whenever ... becomes tapped": TriggerTaps)
            TrigEvent(d, MTG_TRIG_TAPS, obj, MTG_NO_OBJ, d->objs[obj].controller);
    }
}

void Mtg_GainLife(MtgDuel *d, int player, int amount)
{
    d->players[player].life += amount;
    Mtg_Event(d, MTG_EV_LIFE, player, amount, d->players[player].life);
}

// damage from a source (protection prevents it; lifelink gains it), Card.addDamage / Player.addDamage
void Mtg_DealDamage(MtgDuel *d, int sourceObj, MtgTarget to, int amount, int combat)
{
    uint8_t *shield;
    if (amount <= 0)
        return;
    // "prevent all (combat) damage ...": a Fog this turn, or a PreventDamage replacement in force (mtg_static.c)
    if ((combat && d->fog) || Mtg_DamagePrevented(d, sourceObj, to, combat, amount)) {
        Mtg_Event(d, MTG_EV_PREVENTED, sourceObj, to.kind == MTG_TGT_OBJ ? to.id : -1 - to.id, amount);
        return;
    }
    // "prevent the next n damage" (PreventDamageEffect's shield, used up first)
    shield = to.kind == MTG_TGT_OBJ ? &d->objs[to.id].prevent : to.kind == MTG_TGT_PLAYER ? &d->players[to.id].prevent : NULL;
    if (shield && *shield && (to.kind != MTG_TGT_OBJ || d->objs[to.id].zone == MTG_ZONE_BATTLEFIELD)) {
        int n = *shield < amount ? *shield : amount;
        *shield = (uint8_t)(*shield - n);
        amount -= n;
        Mtg_Event(d, MTG_EV_PREVENTED, sourceObj, to.kind == MTG_TGT_OBJ ? to.id : -1 - to.id, n);
        if (amount <= 0)
            return;
    }
    if (to.kind == MTG_TGT_OBJ) {
        if (d->objs[to.id].zone != MTG_ZONE_BATTLEFIELD)
            return;
        if (sourceObj != MTG_NO_OBJ && (Mtg_Protection(d, to.id) & Mtg_ObjColors(d, sourceObj)))
            return;
        if (!(Mtg_ObjTypes(d, to.id) & MTG_T_CREATURE))
            return;   // (planeswalkers: not yet)
        d->objs[to.id].damage += amount;
        if (sourceObj != MTG_NO_OBJ) {   // (who dealt it damage this turn: "a creature dealt damage by CARDNAME")
            uint16_t *by = d->objs[to.id].damagedBy;
            int k;
            for (k = 0; k < 3 && by[k] != sourceObj && by[k] != MTG_NO_OBJ; k++)
                ;
            if (k < 3)
                by[k] = (uint16_t)sourceObj;
        }
        Mtg_Event(d, MTG_EV_DAMAGE, sourceObj, to.id, amount);
    } else if (to.kind == MTG_TGT_PLAYER) {
        d->players[to.id].life -= amount;
        Mtg_Event(d, MTG_EV_DAMAGE, sourceObj, -1 - to.id, amount);
        Mtg_Event(d, MTG_EV_LIFE, to.id, -amount, d->players[to.id].life);
    } else
        return;
    if (sourceObj != MTG_NO_OBJ && (Mtg_Keywords(d, sourceObj) & MTG_KW_BIT(MTG_KW_LIFELINK)))
        Mtg_GainLife(d, d->objs[sourceObj].controller, amount);
    {
        MtgTrigEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.mode = MTG_TRIG_DAMAGE_DONE;
        ev.obj = (uint16_t)sourceObj;
        ev.obj2 = to.kind == MTG_TGT_OBJ ? to.id : MTG_NO_OBJ;
        ev.player = to.kind == MTG_TGT_PLAYER ? (uint8_t)to.id : 0xFF;
        ev.combat = (uint8_t)combat;
        ev.amount = (int16_t)amount;
        ev.objCtrl = sourceObj != MTG_NO_OBJ ? d->objs[sourceObj].controller : 0xFF;
        Mtg_CheckTriggers(d, &ev);
        ev.mode = MTG_TRIG_DAMAGE_DONE_ONCE;   // ("whenever it is dealt damage": each damage dealt, here)
        Mtg_CheckTriggers(d, &ev);
        if (sourceObj != MTG_NO_OBJ) {
            ev.mode = MTG_TRIG_DAMAGE_DEALT_ONCE;   // ("whenever it deals damage")
            Mtg_CheckTriggers(d, &ev);
        }
    }
}

void Mtg_Destroy(MtgDuel *d, int obj, int noRegen)
{
    MtgObj *o = &d->objs[obj];
    if (o->zone != MTG_ZONE_BATTLEFIELD)
        return;
    if (o->regen && !noRegen && !Mtg_StaticForbids(d, MTG_STATIC_CANT_REGENERATE, obj, MTG_NO_OBJ)) {
        // (701.15: tapped, removed from combat, its damage gone; the shield used up)
        int i;
        o->regen--;
        o->damage = 0;
        if (!o->tapped)
            Mtg_Tap(d, obj, 1);
        o->attacking = 0;
        o->blocking = MTG_NO_OBJ;
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].blocking == obj)
                d->objs[i].blocking = MTG_NO_OBJ;
        Mtg_Event(d, MTG_EV_REGENERATE, obj, 0, o->regen);
        return;
    }
    Mtg_MoveTo(d, obj, MTG_ZONE_GRAVEYARD);   // (indestructible: not yet)
}

// the protection an Aura gives its own creature (WHITE WARD's from white: it doesn't push itself off)
static uint8_t Mtg_AuraOwnProtection(const MtgDuel *d, int aura)
{
    const MtgCardDef *c = Mtg_Card(d->objs[aura].def);
    uint8_t p = 0;
    int k;
    for (k = 0; k < c->staticCount; k++)
        if (gMtgStatics[c->statics[k]].enchanted) {
            p |= gMtgStatics[c->statics[k]].addProtection;
            if (gMtgStatics[c->statics[k]].protChosen)   // (FLICKERING WARD: its chosen color)
                p |= d->objs[aura].chosenColors;
        }
    return p;
}

// GameAction.checkStateEffects
// control from statics (Control Magic: layer 2) and GainControl effects, back when they end; 1 if any changed
static int ControlLayer(MtgDuel *d)
{
    int i, changed = 0;
    for (i = 0; i < d->objCount; i++) {
        MtgObj *o = &d->objs[i];
        int want, now;
        if (o->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        if (o->effCtrl && o->effUntil && o->effSource != MTG_NO_OBJ) {   // (LoseControl$: its source's state)
            const MtgObj *s = &d->objs[o->effSource];
            if (((o->effUntil & MTG_CTRL_LEAVES) && s->zone != MTG_ZONE_BATTLEFIELD)
                || ((o->effUntil & MTG_CTRL_LOSE) && s->zone == MTG_ZONE_BATTLEFIELD && s->controller != o->effCtrl - 1)
                || ((o->effUntil & MTG_CTRL_UNTAP) && !s->tapped))
                o->effCtrl = 0;
        }
        want = Mtg_StaticController(d, i);
        if (want < 0 && o->effCtrl)
            want = o->effCtrl - 1;
        if (want >= 0 && !o->ctrlBase)
            o->ctrlBase = (uint8_t)(o->controller + 1);
        now = want >= 0 ? want : o->ctrlBase ? o->ctrlBase - 1 : o->controller;
        if (want < 0)
            o->ctrlBase = 0;
        if (now != o->controller) {   // (a new controller: out of combat, and sick until their turn)
            int k;
            o->controller = (uint8_t)now;
            o->sick = 1;
            o->attacking = 0;
            o->blocking = MTG_NO_OBJ;
            for (k = 0; k < d->objCount; k++)
                if (d->objs[k].blocking == i)
                    d->objs[k].blocking = MTG_NO_OBJ;
            Mtg_Event(d, MTG_EV_CONTROL, i, now, 0);
            changed = 1;
        }
    }
    return changed;
}

static void StateBasedActions(MtgDuel *d)
{
    int again = 1, i, lost[MTG_PLAYERS] = {0, 0};
    PruneEffects(d, 0xFF, 0);
    while (again && !d->over) {
        again = 0;
        for (i = 0; i < MTG_PLAYERS; i++)
            if (!d->players[i].lost && (d->players[i].life <= 0 || d->players[i].drewFromEmpty)) {
                d->players[i].lost = 1;
                lost[i] = 1;
                Mtg_Event(d, MTG_EV_LOSE, i, d->players[i].life, d->players[i].drewFromEmpty);
            }
        if (lost[0] || lost[1]) {
            d->over = 1;
            d->winner = (int8_t)(lost[0] && lost[1] ? -1 : lost[0] ? 1 : 0);
            return;
        }
        if (ControlLayer(d))
            again = 1;
        for (i = 0; i < d->objCount; i++)   // a token anywhere but the battlefield ceases to exist (704.5d)
            if (d->objs[i].def >= MTG_TOKEN_BASE && d->objs[i].zone != MTG_ZONE_BATTLEFIELD
                && d->objs[i].zone != MTG_ZONE_NONE)
                Mtg_MoveTo(d, i, MTG_ZONE_NONE);
        for (i = 0; i < d->objCount; i++) {   // an Aura on nothing it may enchant goes (704.5m)
            const MtgObj *o = &d->objs[i];
            int to = o->attachedTo;
            if (o->zone != MTG_ZONE_BATTLEFIELD || !Mtg_Card(o->def)->enchant)
                continue;
            if (to == MTG_NO_OBJ || to >= d->objCount || d->objs[to].zone != MTG_ZONE_BATTLEFIELD
                || !Mtg_ValidObj(d, to, Mtg_Card(o->def)->enchantSpec ? Mtg_Card(o->def)->enchantSpec : "Creature", i, o->controller)
                || (Mtg_Protection(d, to) & Mtg_ObjColors(d, i) & ~Mtg_AuraOwnProtection(d, i))) {
                Mtg_MoveTo(d, i, MTG_ZONE_GRAVEYARD);
                again = 1;
            }
        }
        for (i = 0; i < d->objCount; i++) {
            int t;
            if (!IsCreature(d, i))
                continue;
            t = Mtg_Toughness(d, i);
            if (t <= 0) {
                Mtg_MoveTo(d, i, MTG_ZONE_GRAVEYARD);
                again = 1;
            } else if (d->objs[i].damage >= t) {
                Mtg_Destroy(d, i, 0);
                again = 1;
            }
        }
    }
}

// ---- starting the duel ----
void Mtg_DuelInit(MtgDuel *d, const MtgDeck *deck0, const MtgDeck *deck1, uint32_t seed)
{
    const MtgDeck *decks[2] = {deck0, deck1};
    int p, i;
    memset(d, 0, sizeof(*d));
    d->rng = seed ? seed : 0x9E3779B9u;
    d->hash = 2166136261u;
    d->winner = -1;
    for (p = 0; p < MTG_PLAYERS; p++) {
        MtgPlayer *pl = &d->players[p];
        pl->life = MTG_START_LIFE;
        for (i = 0; i < decks[p]->count && d->objCount < MTG_MAX_OBJS; i++) {
            MtgObj *o = &d->objs[d->objCount];
            o->def = decks[p]->cards[i];
            o->owner = o->controller = (uint8_t)p;
            o->zone = MTG_ZONE_LIBRARY;
            o->blocking = MTG_NO_OBJ;
            o->attachedTo = MTG_NO_OBJ;
            o->damagedBy[0] = o->damagedBy[1] = o->damagedBy[2] = MTG_NO_OBJ;
            o->seq = ++d->seq;
            pl->library[pl->libraryCount++] = d->objCount++;
        }
        for (i = pl->libraryCount - 1; i > 0; i--) {   // shuffle (Fisher-Yates)
            int j = (int)Mtg_Random(d, (uint32_t)i + 1);
            uint16_t t = pl->library[i];
            pl->library[i] = pl->library[j];
            pl->library[j] = t;
        }
    }
    d->startingPlayer = (uint8_t)Mtg_Random(d, 2);   // (the mulligans: not yet; seven cards each)
    d->active = d->priority = d->startingPlayer;
    for (p = 0; p < MTG_PLAYERS; p++)
        Mtg_Draw(d, p, MTG_HAND_SIZE);
    d->turn = 1;
    d->step = MTG_STEP_UNTAP;
    Mtg_Event(d, MTG_EV_TURN, d->active, d->turn, 0);
    Run(d);
}

// a card into its owner's library at LibraryPosition$: 0 the top, -1 the bottom, n with n cards above it
void Mtg_PutInLibrary(MtgDuel *d, int obj, int pos)
{
    MtgPlayer *pl = &d->players[d->objs[obj].owner];
    int at;
    Mtg_MoveTo(d, obj, MTG_ZONE_LIBRARY);   // (on top: the array's end)
    if (pos == 0 || d->objs[obj].zone != MTG_ZONE_LIBRARY || pl->libraryCount < 2)
        return;
    at = pos < 0 ? 0 : pl->libraryCount - 1 - pos;
    if (at < 0)
        at = 0;
    memmove(&pl->library[at + 1], &pl->library[at], (size_t)(pl->libraryCount - 1 - at) * sizeof(pl->library[0]));
    pl->library[at] = (uint16_t)obj;
}

// a shuffle of all but the top keepTop cards (a tutor's find stays on top)
void Mtg_ShuffleLibraryBelow(MtgDuel *d, int player, int keepTop)
{
    MtgPlayer *pl = &d->players[player];
    int i, n = pl->libraryCount - keepTop;
    for (i = n - 1; i > 0; i--) {
        int j = (int)Mtg_Random(d, (uint32_t)i + 1);
        uint16_t t = pl->library[i];
        pl->library[i] = pl->library[j];
        pl->library[j] = t;
    }
}

void Mtg_ShuffleLibrary(MtgDuel *d, int player)
{
    MtgPlayer *pl = &d->players[player];
    int i;
    for (i = pl->libraryCount - 1; i > 0; i--) {   // (Fisher-Yates, as at the start)
        int j = (int)Mtg_Random(d, (uint32_t)i + 1);
        uint16_t t = pl->library[i];
        pl->library[i] = pl->library[j];
        pl->library[j] = t;
    }
}

// ---- the stack ----
static int TargetStillLegalX(const MtgDuel *d, const MtgStackEntry *e, const MtgAbility *ab);
static int TargetStillLegal(const MtgDuel *d, const MtgStackEntry *e, const MtgAbility *ab)
{
    int r;
    gMtgValidX = e->x;   // (cmcEQX: against the X it was cast for)
    r = TargetStillLegalX(d, e, ab);
    gMtgValidX = -1;
    return r;
}

static int TargetStillLegalX(const MtgDuel *d, const MtgStackEntry *e, const MtgAbility *ab)
{
    int i;
    if (e->target.kind == MTG_TGT_NONE)
        return 1;
    if (e->target.kind == MTG_TGT_PLAYER)
        return !d->players[e->target.id].lost && Mtg_ValidPlayer(d, e->target.id, ab->tgt, e->controller);
    if (e->target.kind == MTG_TGT_STACK) {
        for (i = 0; i < d->stackCount; i++)
            if (d->stack[i].source == e->target.id && d->stack[i].isSpell)
                return Mtg_ValidObj(d, e->target.id, ab->tgt, e->source, e->controller);
        return 0;
    }
    if (d->objs[e->target.id].zone != ab->tgtZone || !Mtg_ValidObj(d, e->target.id, ab->tgt, e->source, e->controller))
        return 0;
    return ab->tgtZone != MTG_ZONE_BATTLEFIELD || Mtg_CanTarget(d, e->target.id, e->source);
}

static int MainAbility(const MtgCardDef *c)
{
    int i;
    for (i = 0; i < c->abilityCount; i++)
        if (gMtgAbilities[c->abilities[i]].kind == MTG_AB_SPELL)
            return c->abilities[i];
    return 0xFFFF;
}

void Mtg_AskChoice(MtgDuel *d, int player, int choice, const MtgManaCost *pay)
{
    MtgQuery *q = &d->query;
    memset(q, 0, sizeof(*q));
    q->kind = MTG_Q_CHOOSE;
    q->player = (uint8_t)player;
    q->choice = (uint8_t)choice;
    q->source = d->res.source;
    q->ability = d->resAbility;
    if (pay)
        q->pay = *pay;
}

void Mtg_AskOption(MtgDuel *d, int player, int kind, const uint16_t *opts, int n)
{
    Mtg_AskChoice(d, player, MTG_CHOICE_OPTION, NULL);
    if (n > MTG_MAX_OPTIONS)
        n = MTG_MAX_OPTIONS;
    d->query.optKind = (uint8_t)kind;
    d->query.optCount = (uint16_t)n;
    memcpy(d->query.opts, opts, (size_t)n * sizeof(opts[0]));
}

static void FinishResolve(MtgDuel *d)
{
    const MtgStackEntry *e = &d->res;
    const MtgCardDef *c = Mtg_Card(d->objs[e->source].def);
    d->resolving = 0;
    if (e->isSpell && d->objs[e->source].zone == MTG_ZONE_STACK) {
        if (c->types & (MTG_T_INSTANT | MTG_T_SORCERY))   // (Buyback: back to its owner's hand)
            Mtg_MoveTo(d, e->source, d->objs[e->source].castMode == 1 ? MTG_ZONE_HAND : MTG_ZONE_GRAVEYARD);
        else {
            d->objs[e->source].controller = e->controller;
            Mtg_MoveTo(d, e->source, MTG_ZONE_BATTLEFIELD);
            if (c->etbKind && c->etbX && e->x > 0)   // (etbCounter:TYPE:X: as many as X was)
                Mtg_AddCountersKind(d, e->source, c->etbKind, e->x);
            if (c->enchant && e->target.kind == MTG_TGT_OBJ)   // an Aura: attached to what it targeted
                d->objs[e->source].attachedTo = e->target.id;
        }
    }
}

// the resolving entry's chain, from where it stopped; 1 while it waits for a choice (d->query is MTG_Q_CHOOSE)
static int ContinueResolve(MtgDuel *d)
{
    MtgResolveCtx ctx;
    ctx.e = &d->res;
    ctx.controller = d->res.controller;
    ctx.source = d->res.source;
    if (d->res.optional == 1 || d->res.optional >= 3) {   // "you may" (OptionalDecider$): asked as it resolves
        if (d->resChoice < 0) {   // (3: the triggering card's / source's controller decides, 4: the triggering player)
            int who = d->res.optional == 3 ? d->res.trigCtrl : d->res.optional == 4 ? d->res.trigPlayer : d->res.controller;
            Mtg_AskChoice(d, who < MTG_PLAYERS ? who : d->res.controller, MTG_CHOICE_YESNO, NULL);
            return 1;
        }
        if (d->resChoice == 0)
            d->resAbility = 0xFFFF;
        d->res.optional = 2;
        d->resChoice = -1;
    }
    while (d->resAbility != 0xFFFF && !d->over) {
        if (!Mtg_ResolveStep(d, &ctx, d->resAbility))
            return 1;
        if (d->resSkip) {   // (its optional cost not paid: the rest of it doesn't happen)
            d->resSkip = 0;
            break;
        }
        if (d->res.extraCount && d->resExtra < d->res.extraCount
            && (d->resAbility == d->res.ability || (Mtg_Param(&gMtgAbilities[d->resAbility], "Defined")
                                                    && (strcmp(Mtg_Param(&gMtgAbilities[d->resAbility], "Defined"), "Targeted") == 0
                                                        || strcmp(Mtg_Param(&gMtgAbilities[d->resAbility], "Defined"), "ParentTarget") == 0)))) {
            // the same again, at its next target (and a sub-ability on "Targeted" likewise: HOPE AND GLORY)
            int t = d->res.extra[d->resExtra++];
            if (t >= MTG_REM_PLAYER) {   // (a player)
                d->res.target.kind = MTG_TGT_PLAYER;
                d->res.target.id = (uint16_t)(t - MTG_REM_PLAYER);
            } else {
                d->res.target.kind = MTG_TGT_OBJ;
                d->res.target.id = (uint16_t)t;
                d->res.lkiPower = (int16_t)Mtg_Power(d, t);
                d->res.lkiToughness = (int16_t)Mtg_Toughness(d, t);
                d->res.lkiController = d->objs[t].controller;
            }
            d->resStage = d->resPicks = 0;
            d->resCondAb = 0xFFFF;
            d->resChoice = -1;
            continue;
        }
        if (d->res.extraCount && d->resExtra) {   // (back to the first target for what follows)
            d->res.target.id = d->resFirst;
            d->res.target.kind = d->resFirstKind;
            d->resExtra = 0;
        }
        {
            uint16_t next = gMtgAbilities[d->resAbility].sub;
            if (gMtgAbilities[d->resAbility].api == MTG_API_FLIP_COIN && d->resFlip) {   // the coin's branch first
                uint16_t b = d->resFlip == 1 ? gMtgAbilities[d->resAbility].winSub : gMtgAbilities[d->resAbility].loseSub;
                d->resFlip = 0;
                if (b != 0xFFFF) {
                    d->resReturn = next;
                    next = b;
                }
            }
            if (gMtgAbilities[d->resAbility].api == MTG_API_GENERIC_CHOICE && d->resMode) {   // the mode chosen, then on
                uint16_t b = (uint16_t)(d->resMode - 1);
                d->resMode = 0;
                d->resReturn = next;
                next = b;
            }
            if (gMtgAbilities[d->resAbility].api == MTG_API_REPEAT_EACH && d->repStart) {   // its first item's run
                d->repStart = 0;
                d->repReturn = next;
                Mtg_Remember(d, d->repItems[0]);
                next = gMtgAbilities[d->repAb].repeatSub;
            }
            if (next == 0xFFFF && d->resReturn != 0xFFFF) {   // (the branch done: the coin's own chain goes on)
                next = d->resReturn;
                d->resReturn = 0xFFFF;
            }
            if (next == 0xFFFF && d->repAb != 0xFFFF) {   // a RepeatEach item done: the next one, or what follows it
                int k;
                for (k = 0; k < d->remCount; k++)   // (the item forgotten; what the run remembered stays)
                    if (d->rem[k] == d->repItems[d->repIdx]) {
                        memmove(&d->rem[k], &d->rem[k + 1], (d->remCount - k - 1) * sizeof(d->rem[0]));
                        d->remCount--;
                        break;
                    }
                if (++d->repIdx < d->repCount) {
                    Mtg_Remember(d, d->repItems[d->repIdx]);
                    next = gMtgAbilities[d->repAb].repeatSub;
                } else {
                    next = d->repReturn;
                    d->repAb = 0xFFFF;
                }
            }
            d->resAbility = next;
        }
        d->resStage = d->resPicks = 0;
        d->resCondAb = 0xFFFF;
        d->resChoice = -1;
    }
    FinishResolve(d);
    return 0;
}

static int ResolveTop(MtgDuel *d);
static int ManaByHand(const MtgAbility *ab);
// a mana ability just activated on its own (ManaByHand): resolved at once, before anyone gets priority
static int ResolveManaNow(MtgDuel *d)
{
    const MtgStackEntry *e;
    if (!d->stackCount)
        return 0;
    e = &d->stack[d->stackCount - 1];
    if (e->isSpell || e->trigger != 0xFFFF || e->ability == 0xFFFF || !ManaByHand(&gMtgAbilities[e->ability]))
        return 0;
    return ResolveTop(d);
}

static int ResolveTop(MtgDuel *d)
{
    MtgStackEntry e = d->stack[--d->stackCount];
    const MtgAbility *ab = e.ability != 0xFFFF ? &gMtgAbilities[e.ability] : NULL;
    if (ab && ab->tgt && e.extraCount) {   // several targets: those still legal stay; it fizzles only with none
        MtgStackEntry t = e;
        uint16_t all[5];
        uint8_t share[5];
        int n = 0, k, legal = 0;
        all[n] = e.target.kind == MTG_TGT_PLAYER ? MTG_REM_PLAYER + e.target.id : e.target.id;
        share[n] = e.divide[0];
        n++;
        for (k = 0; k < e.extraCount; k++) {
            share[n] = e.divide[k + 1];
            all[n++] = e.extra[k];
        }
        e.extraCount = 0;
        for (k = 0; k < n; k++) {   // (each share stays with its target)
            t.target.kind = all[k] >= MTG_REM_PLAYER ? MTG_TGT_PLAYER : MTG_TGT_OBJ;
            t.target.id = (uint16_t)(all[k] >= MTG_REM_PLAYER ? all[k] - MTG_REM_PLAYER : all[k]);
            if (!TargetStillLegal(d, &t, ab))
                continue;
            if (!legal++) {
                e.target = t.target;
                e.divide[0] = share[k];
            } else {
                e.divide[e.extraCount + 1] = share[k];
                e.extra[e.extraCount++] = all[k];
            }
        }
    }
    d->resExtra = 0;
    if ((ab && ab->tgt && !TargetStillLegal(d, &e, ab)) || !Mtg_TriggerStillTrue(d, &e)) {
        Mtg_Event(d, MTG_EV_FIZZLE, e.source, e.ability, 0);
        if (e.isSpell)
            Mtg_MoveTo(d, e.source, MTG_ZONE_GRAVEYARD);
        return 0;
    }
    if (e.target.kind == MTG_TGT_OBJ || e.target.kind == MTG_TGT_STACK) {   // last known information
        e.lkiPower = (int16_t)Mtg_Power(d, e.target.id);
        e.lkiToughness = (int16_t)Mtg_Toughness(d, e.target.id);
        e.lkiController = d->objs[e.target.id].controller;
    }
    Mtg_Event(d, MTG_EV_RESOLVE, e.source, e.ability, e.isSpell);
    d->resFirst = e.target.id;
    d->resFirstKind = e.target.kind;
    d->res = e;
    d->resolving = 1;
    d->resAbility = e.ability;
    d->resReturn = 0xFFFF;
    d->resFlip = 0;
    d->resCondAb = 0xFFFF;
    d->remCount = 0;   // (a new chain remembers its own; an Effect's trigger, its Effect's)
    for (; d->remCount < e.effRemCount; d->remCount++)
        d->rem[d->remCount] = e.effRem[d->remCount];
    d->repAb = 0xFFFF;
    d->repStart = 0;
    d->resStage = d->resPicks = 0;
    d->resChoice = -1;
    d->resMode = 0;
    return ContinueResolve(d);
}

// what permanents that just entered do "as they enter" (ETBReplacement: a choice...), oldest first, each resolving as
// an ability of its own (nothing to target, no stack); 1 while one asks
// an ability of OBJ's resolving at once, as its own (nothing to target, no stack): 1 while it asks something
static int ResolveNow(MtgDuel *d, int obj, int ability, int optional)
{
    memset(&d->res, 0, sizeof(d->res));
    d->res.controller = d->objs[obj].controller;
    d->res.source = (uint16_t)obj;
    d->res.ability = (uint16_t)ability;
    d->res.trigger = 0xFFFF;
    d->res.trigObj = d->res.trigObj2 = MTG_NO_OBJ;
    d->res.trigPlayer = d->res.trigCtrl = 0xFF;
    d->res.optional = (uint8_t)optional;
    d->resolving = 1;
    d->resAbility = (uint16_t)ability;
    d->resReturn = 0xFFFF;
    d->resFlip = 0;
    d->resCondAb = 0xFFFF;
    d->remCount = 0;
    d->repAb = 0xFFFF;
    d->repStart = 0;
    d->resStage = d->resPicks = 0;
    d->resExtra = 0;
    d->resChoice = -1;
    d->resMode = 0;
    return ContinueResolve(d);
}

// what mana abilities used do besides their mana (MtgAbility.manaFx), oldest first
static int RunManaFx(MtgDuel *d)
{
    while (d->fxCount && !d->over) {
        int obj = d->fxObj[0], ab = d->fxAb[0];
        d->fxCount--;
        memmove(&d->fxObj[0], &d->fxObj[1], (size_t)d->fxCount * sizeof(d->fxObj[0]));
        memmove(&d->fxAb[0], &d->fxAb[1], (size_t)d->fxCount * sizeof(d->fxAb[0]));
        if (ResolveNow(d, obj, ab, 0))
            return 1;
    }
    return 0;
}

static int RunEtb(MtgDuel *d)
{
    while (d->etbCount && !d->over) {
        int obj = d->etbQueue[0] & 0x7FFF, which = d->etbQueue[0] >> 15;
        const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
        memmove(&d->etbQueue[0], &d->etbQueue[1], (size_t)(--d->etbCount) * sizeof(d->etbQueue[0]));
        if (d->objs[obj].zone != MTG_ZONE_BATTLEFIELD || !c->etbRepl[which]
            || (c->etbCond[which] && !Mtg_ValidObj(d, obj, c->etbCond[which], obj, d->objs[obj].controller)))
            continue;
        memset(&d->res, 0, sizeof(d->res));
        d->res.controller = d->objs[obj].controller;
        d->res.source = (uint16_t)obj;
        d->res.ability = (uint16_t)(c->etbRepl[which] - 1);
        d->res.trigger = 0xFFFF;
        d->res.trigObj = d->res.trigObj2 = MTG_NO_OBJ;
        d->res.trigPlayer = d->res.trigCtrl = 0xFF;
        d->res.optional = (c->etbOptional >> which) & 1;
        d->resolving = 1;
        d->resAbility = d->res.ability;
        d->resReturn = 0xFFFF;
        d->resFlip = 0;
        d->resCondAb = 0xFFFF;
        d->remCount = 0;
        d->repAb = 0xFFFF;
        d->repStart = 0;
        d->resStage = d->resPicks = 0;
        d->resExtra = 0;
        d->resChoice = -1;
        d->resMode = 0;
        if (ContinueResolve(d))
            return 1;
    }
    return 0;
}

// ---- priority: the legal actions (what PlayerControllerHuman / AiController choose from) ----
static void AddAction(MtgQuery *q, int kind, int obj, int ability, int x, MtgTarget t)
{
    MtgAction *a;
    if (q->actionCount >= MTG_MAX_ACTIONS)
        return;
    a = &q->actions[q->actionCount++];
    a->kind = (uint8_t)kind;
    a->obj = (uint16_t)obj;
    a->ability = (uint16_t)ability;
    a->x = (int16_t)x;
    a->target = t;
    a->alt = 0;
    a->xFixed = 0;
}

// a target candidate under a spec comparing its mana value with X ("cmcEQX", "cmcLEX": Spell Blast, Detonate): legal
// with X its own mana value (at most the X that can be paid), *ax that X; otherwise as usual (*ax = x)
static int ValidAt(const MtgDuel *d, const MtgAbility *ab, int target, int obj, int player, int x, int *ax)
{
    int ok, cmc;
    const char *sv = Mtg_Svar(d->objs[obj].def, "X");
    *ax = x;
    if ((!strstr(ab->tgt, "cmcEQX") && !strstr(ab->tgt, "cmcLEX")) || (sv && strcmp(sv, "Count$xPaid")))
        return Mtg_ValidObj(d, target, ab->tgt, obj, player);   // (X counted, not paid: as it is)
    cmc = Mtg_ManaCostTotal(&Mtg_Card(d->objs[target].def)->cost);
    if (cmc > x)
        return 0;
    gMtgValidX = cmc;
    ok = Mtg_ValidObj(d, target, ab->tgt, obj, player);
    gMtgValidX = -1;
    *ax = cmc;
    return ok;
}

static int sXFixed;   // (AddWithTargets: the actions it adds have their X set by their targets)
static void AddWithTargetsX(const MtgDuel *d, MtgQuery *q, int kind, int obj, int ability, int x, int player);
static void AddWithTargets(const MtgDuel *d, MtgQuery *q, int kind, int obj, int ability, int x, int player)
{
    int before = q->actionCount, m;
    sXFixed = 0;
    AddWithTargetsX(d, q, kind, obj, ability, x, player);
    for (m = before; m < q->actionCount; m++)
        q->actions[m].xFixed = (uint8_t)sXFixed;
}

static void AddWithTargetsX(const MtgDuel *d, MtgQuery *q, int kind, int obj, int ability, int x, int player)
{
    const MtgAbility *ab = ability != 0xFFFF ? &gMtgAbilities[ability] : NULL;
    MtgTarget t = {MTG_TGT_NONE, 0};
    int i, ax;
    if (!ab || !ab->tgt) {
        AddAction(q, kind, obj, ability, x, t);
        return;
    }
    if (ab->tgtMin >= 2 || ab->tgtMax == MTG_TGT_X) {   // ("two target creatures": only with that many to choose from)
        int n = 0;
        for (i = 0; i < d->objCount; i++)
            n += d->objs[i].zone == ab->tgtZone && Mtg_ValidObj(d, i, ab->tgt, obj, player)
                 && (ab->tgtZone != MTG_ZONE_BATTLEFIELD || Mtg_CanTarget(d, i, obj));
        if (ab->tgtMax == MTG_TGT_X && x > (n < 5 ? n : 5))
            x = n < 5 ? n : 5;   // ("X target creatures": X no more than there are; the rest of the mana stays)
        if (n < (ab->tgtMin == MTG_TGT_X ? x : ab->tgtMin) || (ab->tgtMax == MTG_TGT_X && x < 1))
            return;
        if (ab->tgtMax == MTG_TGT_X)
            sXFixed = 1;   // (X is how many targets: not asked)
    }
    if ((strstr(ab->tgt, "cmcEQX") || strstr(ab->tgt, "cmcLEX"))
        && (!Mtg_Svar(d->objs[obj].def, "X") || !strcmp(Mtg_Svar(d->objs[obj].def, "X"), "Count$xPaid")))
        sXFixed = 1;   // (X is its target's mana value)
    if (ab->tgtZone != MTG_ZONE_BATTLEFIELD) {   // (cards in a graveyard)
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].zone == ab->tgtZone && ValidAt(d, ab, i, obj, player, x, &ax)) {
                t.kind = MTG_TGT_OBJ;
                t.id = (uint16_t)i;
                AddAction(q, kind, obj, ability, ax, t);
            }
        return;
    }
    if (ab->tgtStack) {
        for (i = d->stackCount - 1; i >= 0; i--)
            if (d->stack[i].isSpell && ValidAt(d, ab, d->stack[i].source, obj, player, x, &ax)) {
                t.kind = MTG_TGT_STACK;
                t.id = d->stack[i].source;
                AddAction(q, kind, obj, ability, ax, t);
            }
        return;
    }
    for (i = 0; i < MTG_PLAYERS; i++)
        if (Mtg_ValidPlayer(d, i, ab->tgt, player)) {
            t.kind = MTG_TGT_PLAYER;
            t.id = (uint16_t)i;
            AddAction(q, kind, obj, ability, x, t);
        }
    for (i = 0; i < d->objCount; i++)
        if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && ValidAt(d, ab, i, obj, player, x, &ax)
            && Mtg_CanTarget(d, i, obj)) {
            t.kind = MTG_TGT_OBJ;
            t.id = (uint16_t)i;
            AddAction(q, kind, obj, ability, ax, t);
        }
}

static int SorceryTiming(const MtgDuel *d, int player)
{
    return player == d->active && d->stackCount == 0
        && (d->step == MTG_STEP_MAIN1 || d->step == MTG_STEP_MAIN2);
}

static int MainAbility(const MtgCardDef *c);

// a spell's casts for its alternative cost (StaticAbilityAlternativeCost): while its IsPresent$ holds and the cost can
// be paid; never with an additional cost of its own beside it (none in the pool does both)
static void AddAltCasts(const MtgDuel *d, MtgQuery *q, int obj, int p)
{
    const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
    const MtgAltCost *alt = &gMtgAltCosts[c->altCost - 1];
    int ability = MainAbility(c), before = q->actionCount, k, m;
    if (alt->present) {
        int have = 0;
        for (k = 0; k < d->objCount && !have; k++)
            have = d->objs[k].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, k, alt->present, obj, p);
        if (!have)
            return;
    }
    for (k = 0; k < 2; k++)   // (CheckSVar$: "if an opponent controls a Swamp and you control a Plains")
        if (alt->check[k] && !Mtg_CheckSvar(d, d->objs[obj].def, obj, p, alt->check[k], alt->compare[k]))
            return;
    if (!Mtg_CostPayable(d, p, &alt->cost, obj, obj))
        return;
    if (ability != 0xFFFF) {
        const MtgCost *own = &gMtgAbilities[ability].cost;
        if (own->sacN || own->discardN || own->payLife || own->exileHandN || own->returnN || own->tapTypeN || own->exileGraveN)
            return;
    }
    if (ability != 0xFFFF && gMtgAbilities[ability].api == MTG_API_CHARM) {
        const MtgAbility *ch = &gMtgAbilities[ability];
        for (k = 0; k < ch->choiceCount; k++)
            AddWithTargets(d, q, MTG_ACT_CAST, obj, ch->choices[k], 0, p);
    } else
        AddWithTargets(d, q, MTG_ACT_CAST, obj, ability, 0, p);
    for (m = before; m < q->actionCount; m++)
        q->actions[m].alt = 1;
}

// a cost's parts that are picked or paid besides mana (sacrifices, discards, life...)
static int HasOther(const MtgCost *c)
{
    return c->sacN || c->discardN || c->payLife || c->exileHandN || c->returnN || c->tapTypeN || c->exileGraveN
        || c->discardHand || c->millN || c->exileTopN || c->subCtrAll || c->xPart;
}

int Mtg_CostMaxX(const MtgDuel *d, int player, const MtgCost *c, int source)
{
    switch (c->xPart) {
    case MTG_XPART_LIFE: return d->players[player].life > 0 ? d->players[player].life : 0;
    case MTG_XPART_DISCARD: return Mtg_CostCandidates(d, player, c, MTG_COSTK_DISCARD, source, NULL);
    case MTG_XPART_SAC: return Mtg_CostCandidates(d, player, c, MTG_COSTK_SAC, source, NULL);
    case MTG_XPART_EXILE_GRAVE: return Mtg_CostCandidates(d, player, c, MTG_COSTK_EXILE_GRAVE, source, NULL);
    case MTG_XPART_COUNTERS: return source != MTG_NO_OBJ ? Mtg_Counters(d, source, c->subCtrKind) : 0;
    }
    return 0;
}

// a mana ability that paying never uses (it costs mana, sacrifices itself, or more than {T}): activated on its own,
// resolving at once (mana abilities don't wait on the stack: Mtg_Answer)
static int ManaByHand(const MtgAbility *ab)
{
    return ab->api == MTG_API_MANA && (!ab->cost.tap || ab->cost.sacSelf || Mtg_ManaCostTotal(&ab->cost.mana)
                                       || ab->cost.mana.x || HasOther(&ab->cost) || ab->cost.exileSelf || ab->cost.returnSelf
                                       || ab->cost.xPart);
}

// the extra cost a cast pays (gMtgAltCosts + 1: its alternative one, a kicker, buyback, flashback; 0 none or two)
static int ExtraIdx(const MtgCardDef *c, int alt)
{
    switch (alt) {
    case MTG_ALT_COST: return c->altCost;
    case MTG_ALT_KICK1: return c->kicker[0];
    case MTG_ALT_KICK2: return c->kicker[1];
    case MTG_ALT_BUYBACK: return c->buyback;
    case MTG_ALT_FLASHBACK: return c->flashback;
    }
    return 0;
}

// the mana a cast pays: its own (with Raise / ReduceCost), and its kickers' or buyback's besides
static void CastMana(const MtgCardDef *c, int alt, MtgManaCost *cost)
{
    int k, j;
    for (j = 0; j < 2; j++) {
        int idx = alt == MTG_ALT_KICK12 ? c->kicker[j] : j ? 0 : ExtraIdx(c, alt);
        const MtgManaCost *m;
        if (!idx || alt == MTG_ALT_COST || alt == MTG_ALT_FLASHBACK)
            continue;
        m = &gMtgAltCosts[idx - 1].cost.mana;
        cost->generic = (uint8_t)(cost->generic + m->generic);
        for (k = 0; k < MTG_MANA_KINDS; k++)
            cost->pip[k] = (uint8_t)(cost->pip[k] + m->pip[k]);
    }
}

// a spell's kicked and buyback casts beside its plain one (its cost already payable)
static void AddExtraCasts(const MtgDuel *d, MtgQuery *q, int obj, int p, const MtgManaCost *base, int ability)
{
    const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
    const MtgCost *own = ability != 0xFFFF ? &gMtgAbilities[ability].cost : NULL;
    static const uint8_t sAlts[4] = {MTG_ALT_KICK1, MTG_ALT_KICK2, MTG_ALT_KICK12, MTG_ALT_BUYBACK};
    int k, m;
    for (k = 0; k < 4; k++) {
        int alt = sAlts[k], idx = ExtraIdx(c, alt), before = q->actionCount, x = 0;
        MtgManaCost cost = *base;
        const MtgCost *extra = idx ? &gMtgAltCosts[idx - 1].cost : NULL;
        if (alt == MTG_ALT_KICK12) {
            if (!c->kicker[0] || !c->kicker[1] || HasOther(&gMtgAltCosts[c->kicker[0] - 1].cost)
                || HasOther(&gMtgAltCosts[c->kicker[1] - 1].cost))
                continue;
        } else if (!extra)
            continue;
        CastMana(c, alt, &cost);
        if (cost.x && (x = Mtg_MaxX(d, p, &cost, MTG_NO_OBJ)) == 0)   // (X: the most that can be paid besides it)
            continue;
        if (!Mtg_CanPay(d, p, &cost, x, MTG_NO_OBJ))
            continue;
        if (extra && HasOther(extra) && ((own && HasOther(own)) || !Mtg_CostPayable(d, p, extra, obj, obj)))
            continue;
        AddWithTargets(d, q, MTG_ACT_CAST, obj, ability, x, p);
        for (m = before; m < q->actionCount; m++)
            q->actions[m].alt = (uint8_t)alt;
    }
}

// a card's flashback cast from its owner's graveyard (its timing, its flashback cost instead of its mana)
static void AddFlashback(const MtgDuel *d, MtgQuery *q, int obj, int p)
{
    const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
    const MtgCost *fb = &gMtgAltCosts[c->flashback - 1].cost;
    int ability = MainAbility(c), before = q->actionCount, m;
    if (!(c->types & MTG_T_INSTANT) && !(c->keywords & MTG_KW_BIT(MTG_KW_FLASH)) && !SorceryTiming(d, p))
        return;
    if (Mtg_StaticForbids(d, MTG_STATIC_CANT_BE_CAST, obj, p) || !Mtg_CanPay(d, p, &fb->mana, 0, MTG_NO_OBJ))
        return;
    if (HasOther(fb) && !Mtg_CostPayable(d, p, fb, obj, obj))
        return;
    if (ability != 0xFFFF) {
        const MtgAbility *ab = &gMtgAbilities[ability];
        if (ab->api == MTG_API_CHARM || (HasOther(fb) && HasOther(&ab->cost)) || !Mtg_CostPayable(d, p, &ab->cost, obj, obj)
            || (ab->actMyTurn && d->active != p) || (ab->actSteps && !(ab->actSteps & (1u << d->step))))
            return;
    }
    AddWithTargets(d, q, MTG_ACT_CAST, obj, ability, 0, p);
    for (m = before; m < q->actionCount; m++)
        q->actions[m].alt = MTG_ALT_FLASHBACK;
}

static void BuildPriority(MtgDuel *d)
{
    MtgQuery *q = &d->query;
    int p = d->priority, i, k;
    MtgTarget none = {MTG_TGT_NONE, 0};
    memset(q, 0, sizeof(*q));
    q->kind = MTG_Q_PRIORITY;
    q->player = (uint8_t)p;
    AddAction(q, MTG_ACT_PASS, MTG_NO_OBJ, 0xFFFF, 0, none);
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        const MtgCardDef *c = Mtg_Card(o->def);
        if (!c->supported)
            continue;
        if (o->zone == MTG_ZONE_HAND && o->owner == p) {
            int ability, x = 0;
            MtgManaCost cost;
            for (k = 0; k < c->abilityCount; k++) {   // abilities from the hand (Cycling): any time with priority
                const MtgAbility *ab = &gMtgAbilities[c->abilities[k]];
                if (!ab->fromHand || ab->kind != MTG_AB_ACTIVATED || ab->cost.mana.x
                    || Mtg_StaticForbids(d, MTG_STATIC_CANT_BE_ACTIVATED, i, c->abilities[k])   // (Stabilizer)
                    || !ActivationTimingOk(d, i, c->abilities[k], p))
                    continue;
                if (!Mtg_CanPay(d, p, &ab->cost.mana, 0, MTG_NO_OBJ) || !Mtg_CostPayable(d, p, &ab->cost, i, MTG_NO_OBJ))
                    continue;
                AddWithTargets(d, q, MTG_ACT_ACTIVATE, i, c->abilities[k], 0, p);
            }
            if (c->morph && SorceryTiming(d, p)) {   // (Morph: face down for {3}, a 2/2 with no name or abilities)
                MtgManaCost three;
                memset(&three, 0, sizeof(three));
                three.generic = 3;
                if (Mtg_CanPay(d, p, &three, 0, MTG_NO_OBJ)) {
                    AddAction(q, MTG_ACT_CAST, i, 0xFFFF, 0, none);
                    q->actions[q->actionCount - 1].alt = MTG_ALT_MORPH;
                }
            }
            if (c->types & MTG_T_LAND) {
                if (SorceryTiming(d, p) && d->players[p].landsPlayed == 0
                    && !Mtg_StaticForbids(d, MTG_STATIC_CANT_PLAY_LAND, i, p))   // (Territorial Dispute...)
                    AddAction(q, MTG_ACT_LAND, i, 0xFFFF, 0, none);
                continue;
            }
            if (!(c->types & MTG_T_INSTANT) && !(c->keywords & MTG_KW_BIT(MTG_KW_FLASH)) && !SorceryTiming(d, p))
                continue;
            if (Mtg_StaticForbids(d, MTG_STATIC_CANT_BE_CAST, i, p))   // ("creature spells can't be cast"...)
                continue;
            if (c->altCost && !c->cost.x)   // its alternative cost too ("rather than pay this spell's mana cost")
                AddAltCasts(d, q, i, p);
            Mtg_SpellCost(d, p, i, &cost);   // (Raise / ReduceCost statics)
            if (cost.x) {   // (X: the most that can be paid; a player's own pick is the UI's to come)
                x = Mtg_MaxX(d, p, &cost, MTG_NO_OBJ);
                if (x == 0)
                    continue;
            }
            if (!Mtg_CanPay(d, p, &cost, x, MTG_NO_OBJ))
                continue;
            ability = MainAbility(c);
            if (ability != 0xFFFF && !Mtg_CostPayable(d, p, &gMtgAbilities[ability].cost, i, i))
                continue;
            if (ability != 0xFFFF && gMtgAbilities[ability].cost.xPart) {   // (X also in an additional cost: no more than it pays)
                int most = Mtg_CostMaxX(d, p, &gMtgAbilities[ability].cost, i);
                x = cost.x ? (x < most ? x : most) : most;
                if (gMtgAbilities[ability].cost.xPart == MTG_XPART_DISCARD && x > 0)
                    x--;   // (the card being cast pays nothing)
                if (x == 0 && !cost.x)
                    continue;
            }
            if (ability != 0xFFFF && ((gMtgAbilities[ability].actMyTurn && d->active != p)
                                      || (gMtgAbilities[ability].actSteps
                                          && !(gMtgAbilities[ability].actSteps & (1u << d->step)))))
                continue;   // ("cast only during combat before blockers are declared"...)
            if (ability == 0xFFFF || gMtgAbilities[ability].api != MTG_API_CHARM)
                AddExtraCasts(d, q, i, p, &cost, ability);   // (kicked, with buyback)
            if (ability != 0xFFFF && gMtgAbilities[ability].api == MTG_API_CHARM) {   // one action per mode
                const MtgAbility *ch = &gMtgAbilities[ability];
                for (k = 0; k < ch->choiceCount; k++)
                    AddWithTargets(d, q, MTG_ACT_CAST, i, ch->choices[k], x, p);
                continue;
            }
            AddWithTargets(d, q, MTG_ACT_CAST, i, ability, x, p);
        } else if (o->zone == MTG_ZONE_GRAVEYARD && o->owner == p && c->flashback) {
            AddFlashback(d, q, i, p);
        } else if (o->zone == MTG_ZONE_BATTLEFIELD && o->controller == p && o->faceDownOf
                   && Mtg_Card(o->faceDownOf - 1)->morph) {   // (turned face up any time it has priority)
            const MtgCost *mc = &gMtgAltCosts[Mtg_Card(o->faceDownOf - 1)->morph - 1].cost;
            if (Mtg_CanPay(d, p, &mc->mana, 0, MTG_NO_OBJ) && (!HasOther(mc) || Mtg_CostPayable(d, p, mc, i, MTG_NO_OBJ)))
                AddAction(q, MTG_ACT_TURN_UP, i, 0xFFFF, 0, none);
        } else if (o->zone == MTG_ZONE_BATTLEFIELD) {   // (another's: only what any player may activate)
            uint16_t abil[16];
            int abilCount = Mtg_ObjAbilities(d, i, abil, 16), mine = o->controller == p;   // (its card's and statics')
            for (k = 0; k < abilCount; k++) {
                const MtgAbility *ab = &gMtgAbilities[abil[k]];
                int x = 0;
                if (!mine && !ab->anyPlayer)
                    continue;
                if (ab->kind != MTG_AB_ACTIVATED || ab->fromHand || (ab->api == MTG_API_MANA && !ManaByHand(ab)))
                    continue;   // (mana abilities are used by paying, except those paying can't; Cycling only from the hand)
                if (Mtg_StaticForbids(d, MTG_STATIC_CANT_BE_ACTIVATED, i, abil[k]))
                    continue;   // (Arrest, Null Rod...)
                if (!ActivationTimingOk(d, i, abil[k], p))
                    continue;   // (once each turn, only as a sorcery, only during upkeep...)
                if (ab->cost.tap && (o->tapped || ((Mtg_ObjTypes(d, i) & MTG_T_CREATURE) && o->sick
                                                   && !(Mtg_Keywords(d, i) & MTG_KW_BIT(MTG_KW_HASTE)))))
                    continue;
                if (ab->cost.mana.x) {
                    x = Mtg_MaxX(d, p, &ab->cost.mana, ab->cost.tap ? i : MTG_NO_OBJ);
                    if (x == 0)
                        continue;
                }
                if (ab->cost.xPart) {   // (X in a part besides mana: no more than that part can pay)
                    int most = Mtg_CostMaxX(d, p, &ab->cost, i);
                    x = ab->cost.mana.x ? (x < most ? x : most) : most;
                    if (x == 0)
                        continue;
                }
                if (!Mtg_CanPay(d, p, &ab->cost.mana, x, ab->cost.tap ? i : MTG_NO_OBJ))
                    continue;
                if (!Mtg_CostPayable(d, p, &ab->cost, i, MTG_NO_OBJ))
                    continue;
                if (ab->api == MTG_API_CHARM) {
                    int m;
                    for (m = 0; m < ab->choiceCount; m++)
                        AddWithTargets(d, q, MTG_ACT_ACTIVATE, i, ab->choices[m], x, p);
                    continue;
                }
                AddWithTargets(d, q, MTG_ACT_ACTIVATE, i, abil[k], x, p);
            }
        }
    }
}

// ---- costs beyond mana and {T} (CostSacrifice, CostDiscard, CostPayLife, CostExile, CostReturn, CostTapType)
// what may pay a part of KIND (MTG_COSTK_*): the player's permanents (sacrifice, return, tap: untapped ones) or hand
// cards (discard, exile) of its spec; a card being cast is on the stack by then. Returns how many (out may be NULL)
int Mtg_CostCandidates(const MtgDuel *d, int player, const MtgCost *c, int kind, int source, uint16_t *out)
{
    int i, n = 0, hand = kind == MTG_COSTK_DISCARD || kind == MTG_COSTK_EXILE_HAND, grave = kind == MTG_COSTK_EXILE_GRAVE;
    const char *spec = kind == MTG_COSTK_SAC ? c->sacSpec : kind == MTG_COSTK_DISCARD ? c->discardSpec
                     : kind == MTG_COSTK_EXILE_HAND ? c->exileHandSpec : kind == MTG_COSTK_RETURN ? c->returnSpec
                     : kind == MTG_COSTK_EXILE_GRAVE ? c->exileGraveSpec : c->tapTypeSpec;
    if (!spec)
        return 0;
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        if (hand || grave ? (o->zone != (hand ? MTG_ZONE_HAND : MTG_ZONE_GRAVEYARD) || o->owner != player)
                          : (o->zone != MTG_ZONE_BATTLEFIELD || o->controller != player))
            continue;
        if (kind == MTG_COSTK_TAP && o->tapped)
            continue;
        if (!Mtg_ValidObj(d, i, spec, source, player))
            continue;
        if (out && n < MTG_MAX_COMBAT)
            out[n] = (uint16_t)i;
        n++;
    }
    return n;
}

int Mtg_CostPayable(const MtgDuel *d, int player, const MtgCost *c, int source, int casting)
{
    static const uint8_t sKinds[] = {MTG_COSTK_SAC, MTG_COSTK_DISCARD, MTG_COSTK_EXILE_HAND, MTG_COSTK_RETURN, MTG_COSTK_TAP,
                                     MTG_COSTK_EXILE_GRAVE};
    unsigned k;
    if (c->payLife && d->players[player].life < c->payLife)
        return 0;
    if (c->subCtrN && (source == MTG_NO_OBJ || Mtg_Counters(d, source, c->subCtrKind) < c->subCtrN))
        return 0;
    if ((c->millN || c->exileTopN) && d->players[player].libraryCount < c->millN + c->exileTopN)
        return 0;
    for (k = 0; k < sizeof(sKinds); k++) {
        int kind = sKinds[k], need = kind == MTG_COSTK_SAC ? c->sacN : kind == MTG_COSTK_DISCARD ? (c->discardN)
                 : kind == MTG_COSTK_EXILE_HAND ? c->exileHandN : kind == MTG_COSTK_RETURN ? c->returnN
                 : kind == MTG_COSTK_EXILE_GRAVE ? c->exileGraveN : c->tapTypeN, n;
        const char *spec = kind == MTG_COSTK_DISCARD ? c->discardSpec : c->exileHandSpec;
        if (!need)
            continue;
        n = Mtg_CostCandidates(d, player, c, kind, source, NULL);
        if ((kind == MTG_COSTK_DISCARD || kind == MTG_COSTK_EXILE_HAND) && casting != MTG_NO_OBJ
            && d->objs[casting].zone == MTG_ZONE_HAND && Mtg_ValidObj(d, casting, spec, source, player))
            n--;   // (the card being cast pays nothing: it's on the stack by then)
        if (n < need)
            return 0;
    }
    return 1;
}

// the cost being paid now (an ability's, or a spell's alternative one)
static const MtgCost *CostNow(const MtgDuel *d)
{
    if (d->costAlt == 2)   // (an UnlessCost$ paid as its ability resolves)
        return &gMtgUnlessCosts[gMtgAbilities[d->costAbility].unless - 1];
    if (d->costAlt == 4)   // (a trigger's own cost, paid as it resolves)
        return &gMtgAbilities[d->costAbility].cost;
    if (d->costAlt)
        return &gMtgAltCosts[d->costAltIdx - 1].cost;
    return &gMtgAbilities[d->costAbility].cost;
}

static int TargetCandidates(const MtgDuel *d, uint16_t *out);

// the next pick a cost asks for (1), or nothing left (0): sacrifices, discards, exiles, returns, taps
static int AskCost(MtgDuel *d)
{
    MtgQuery *q = &d->query;
    int kind;
    if (d->costSac)
        kind = MTG_COSTK_SAC;
    else if (d->costDiscard)
        kind = MTG_COSTK_DISCARD;
    else if (d->costExile)
        kind = MTG_COSTK_EXILE_HAND;
    else if (d->costReturn)
        kind = MTG_COSTK_RETURN;
    else if (d->costTap)
        kind = MTG_COSTK_TAP;
    else if (d->costExileGrave)
        kind = MTG_COSTK_EXILE_GRAVE;
    else if ((d->tgtLeft || d->tgtOpt) && d->stackCount) {   // its other targets (TargetMax$)
        memset(q, 0, sizeof(*q));
        q->kind = MTG_Q_CHOOSE;
        q->choice = MTG_CHOICE_PICK;
        q->player = d->stack[d->stackCount - 1].controller;
        q->source = d->stack[d->stackCount - 1].source;
        q->ability = d->stack[d->stackCount - 1].ability;
        q->forCost = 7;
        q->candidateCount = (uint8_t)TargetCandidates(d, q->candidates);
        if (!q->candidateCount) {
            d->tgtLeft = d->tgtOpt = 0;
            return AskCost(d);   // (the division, if any)
        }
        return 1;
    } else if (d->divPending && d->stackCount) {   // DividedAsYouChoose$: each target's share but the last's, asked
        MtgStackEntry *e = &d->stack[d->stackCount - 1];
        int count = 1 + e->extraCount, total, given = 0, k, ok;
        MtgResolveCtx ctx;
        ctx.e = e;
        ctx.controller = e->controller;
        ctx.source = e->source;
        total = Mtg_Amount(d, &ctx, gMtgAbilities[e->ability].card,
                           Mtg_Param(&gMtgAbilities[e->ability], "DividedAsYouChoose"), &ok);
        for (k = 0; k < d->divIdx; k++)
            given += e->divide[k];
        if (count == 1 || d->divIdx >= count - 1 || total - given <= count - 1 - d->divIdx) {
            for (k = d->divIdx; k < count && k < 5; k++)   // (the last one: the rest; none left to give: 1 each, as rules
                e->divide[k] = (uint8_t)(k == count - 1 ? (total - given > 0 ? total - given : 0) : 1);   // demand)
            d->divPending = 0;
            return 0;
        }
        {
            uint16_t opts[MTG_MAX_OPTIONS];
            int most = total - given - (count - 1 - d->divIdx), n = 0, who;
            for (k = 1; k <= most && n < MTG_MAX_OPTIONS; k++)
                opts[n++] = (uint16_t)k;
            who = d->divIdx == 0 ? (e->target.kind == MTG_TGT_PLAYER ? MTG_REM_PLAYER + e->target.id : e->target.id)
                                 : e->extra[d->divIdx - 1];
            Mtg_AskOption(d, e->controller, MTG_OPT_NUMBER, opts, n);
            d->query.forCost = 9;
            d->query.source = e->source;
            d->query.ability = e->ability;
            d->query.candidates[0] = (uint16_t)who;
            d->query.candidateCount = 1;
        }
        return 1;
    } else
        return 0;
    memset(q, 0, sizeof(*q));
    q->kind = MTG_Q_CHOOSE;
    q->choice = MTG_CHOICE_PICK;
    q->player = d->costPlayer;
    q->source = d->costSource;
    q->ability = d->costAbility;
    q->forCost = (uint8_t)(kind == MTG_COSTK_SAC ? 1 : kind == MTG_COSTK_DISCARD ? 2 : kind + 1);
    q->candidateCount = (uint8_t)Mtg_CostCandidates(d, d->costPlayer, CostNow(d), kind, d->costSource, q->candidates);
    if (q->candidateCount > MTG_MAX_COMBAT)
        q->candidateCount = MTG_MAX_COMBAT;
    if (!q->candidateCount) {   // (nothing left to pay with: checked before, so never; the rest is waived)
        d->costSac = d->costDiscard = d->costExile = d->costReturn = d->costTap = d->costExileGrave = 0;
        return 0;
    }
    return 1;
}

static void PayCostPick(MtgDuel *d, int obj)
{
    if (!d->costSac && !d->costDiscard && !d->costExile && !d->costReturn && !d->costTap && !d->costExileGrave
        && (d->tgtLeft || d->tgtOpt)) {   // another target
        MtgStackEntry *e = &d->stack[d->stackCount - 1];
        if (e->extraCount < 4) {
            MtgTarget t;
            e->extra[e->extraCount++] = (uint16_t)obj;
            t.kind = obj >= MTG_REM_PLAYER ? MTG_TGT_PLAYER : MTG_TGT_OBJ;
            t.id = (uint16_t)(obj >= MTG_REM_PLAYER ? obj - MTG_REM_PLAYER : obj);
            TargetEvent(d, t, e->source, e->controller);
        }
        if (d->tgtLeft)
            d->tgtLeft--;
        else
            d->tgtOpt--;
        return;
    }
    if (d->costSac) {
        if (d->costAlt == 4) {   // (its last known power and toughness, for Sacrificed$CardPower / CardToughness)
            d->res.sacPower = (int16_t)Mtg_Power(d, obj);
            d->res.sacToughness = (int16_t)Mtg_Toughness(d, obj);
        } else if (d->stackCount && d->costAlt != 2 && d->costAlt != 3) {
            d->stack[d->stackCount - 1].sacPower = (int16_t)Mtg_Power(d, obj);
            d->stack[d->stackCount - 1].sacToughness = (int16_t)Mtg_Toughness(d, obj);
        }
        Mtg_Sacrifice(d, obj);
        d->costSac--;
    } else if (d->costDiscard) {
        Mtg_MoveTo(d, obj, MTG_ZONE_GRAVEYARD);
        d->costDiscard--;
    } else if (d->costExile) {
        Mtg_MoveTo(d, obj, MTG_ZONE_EXILE);
        d->costExile--;
    } else if (d->costReturn) {
        Mtg_MoveTo(d, obj, MTG_ZONE_HAND);
        d->costReturn--;
    } else if (d->costTap) {
        Mtg_Tap(d, obj, 1);
        d->costTap--;
    } else if (d->costExileGrave) {
        Mtg_MoveTo(d, obj, MTG_ZONE_EXILE);
        d->costExileGrave--;
    }
}

// the costs paid at once (life, a random discard, the source exiled or returned); the picks are set to be asked
static int sCostX;   // (the X chosen for the cost being paid: its X part's amount)
static void PayCost(MtgDuel *d, int p, const MtgCost *c, int ability, int source, int alt)
{
    if (c->xPart == MTG_XPART_LIFE && sCostX > 0) {
        d->players[p].life = (int16_t)(d->players[p].life - sCostX);
        Mtg_Event(d, MTG_EV_LIFE, p, -sCostX, d->players[p].life);
    }
    if (c->xPart == MTG_XPART_COUNTERS && sCostX > 0)
        Mtg_AddCountersKind(d, source, c->subCtrKind, -sCostX);
    if (c->payLife) {
        d->players[p].life -= c->payLife;
        Mtg_Event(d, MTG_EV_LIFE, p, -c->payLife, d->players[p].life);
    }
    if (c->subCtrN)   // (SubCounter<n/TYPE>: off its source)
        Mtg_AddCountersKind(d, source, c->subCtrKind, -(int)c->subCtrN);
    if (c->subCtrAll)   // (SubCounter<All/TYPE>: every one; how many it was, for the ability)
        Mtg_AddCountersKind(d, source, c->subCtrKind, -Mtg_Counters(d, source, c->subCtrKind));
    if (c->discardHand) {   // (Discard<1/Hand>: the whole hand)
        int k;
        for (k = 0; k < d->objCount; k++)
            if (d->objs[k].zone == MTG_ZONE_HAND && d->objs[k].owner == p)
                Mtg_MoveTo(d, k, MTG_ZONE_GRAVEYARD);
    }
    if (c->millN || c->exileTopN) {   // (Mill<n>, ExileFromTop<n>: from the top of its player's library)
        int k;
        for (k = 0; k < c->millN + c->exileTopN; k++) {
            int top = Mtg_TopOfLibrary(d, p);
            if (top == MTG_NO_OBJ || top < 0)
                break;
            Mtg_MoveTo(d, top, k < c->millN ? MTG_ZONE_GRAVEYARD : MTG_ZONE_EXILE);
        }
    }
    if (c->damageYou) {
        MtgTarget you;
        you.kind = MTG_TGT_PLAYER;
        you.id = (uint16_t)p;
        Mtg_DealDamage(d, source, you, c->damageYou, 0);
    }
    if (c->exileSelf)
        Mtg_MoveTo(d, source, MTG_ZONE_EXILE);
    if (c->returnSelf)
        Mtg_MoveTo(d, source, MTG_ZONE_HAND);
    if (c->discardN && c->discardRandom) {
        int k;
        for (k = 0; k < c->discardN; k++) {
            uint16_t hand[MTG_MAX_COMBAT];
            int n = Mtg_CostCandidates(d, p, c, MTG_COSTK_DISCARD, source, hand);
            if (n > MTG_MAX_COMBAT)
                n = MTG_MAX_COMBAT;
            if (n)
                Mtg_MoveTo(d, hand[Mtg_Random(d, (uint32_t)n)], MTG_ZONE_GRAVEYARD);
        }
    }
    d->costSac = c->xPart == MTG_XPART_SAC ? (uint8_t)sCostX : c->sacN;
    d->costDiscard = c->xPart == MTG_XPART_DISCARD ? (uint8_t)sCostX : c->discardRandom ? 0 : c->discardN;
    d->costExile = c->exileHandN;
    d->costReturn = c->returnN;
    d->costTap = c->tapTypeN;
    d->costExileGrave = c->xPart == MTG_XPART_EXILE_GRAVE ? (uint8_t)sCostX : c->exileGraveN;
    d->costPlayer = (uint8_t)p;
    d->costSource = (uint16_t)source;
    d->costAbility = (uint16_t)ability;
    d->costAlt = (uint8_t)alt;
}

static void PayOtherCosts(MtgDuel *d, int p, const MtgAbility *ab, int source)
{
    PayCost(d, p, &ab->cost, (int)(ab - gMtgAbilities), source, 0);
}

int Mtg_PayOptCost(MtgDuel *d, int p, int ability, int source)
{
    PayCost(d, p, &gMtgAbilities[ability].cost, ability, source, 4);
    return AskCost(d);
}

int Mtg_PayUnlessCost(MtgDuel *d, int p, int ability, int source)
{
    const MtgCost *c = &gMtgUnlessCosts[gMtgAbilities[ability].unless - 1];
    Mtg_Pay(d, p, &c->mana, 0, MTG_NO_OBJ);
    PayCost(d, p, c, ability, source, 2);
    return AskCost(d);
}

static void DoAction(MtgDuel *d, const MtgAction *a)
{
    int p = d->priority;
    MtgStackEntry *e;
    if (a->kind == MTG_ACT_LAND) {
        Mtg_MoveTo(d, a->obj, MTG_ZONE_BATTLEFIELD);
        d->players[p].landsPlayed++;
        Mtg_Event(d, MTG_EV_LAND, a->obj, p, 0);
        TrigEvent(d, MTG_TRIG_LAND_PLAYED, a->obj, MTG_NO_OBJ, p);   // ("whenever a land is played": TriggerLandPlayed)
        return;
    }
    if (a->kind == MTG_ACT_TURN_UP) {   // Morph: its cost paid, its real card face up (no stack); "when turned face up"
        MtgObj *o = &d->objs[a->obj];
        int idx = Mtg_Card(o->faceDownOf - 1)->morph;
        const MtgCost *mc = &gMtgAltCosts[idx - 1].cost;
        Mtg_Pay(d, p, &mc->mana, 0, MTG_NO_OBJ);
        o->def = (uint16_t)(o->faceDownOf - 1);
        o->faceDownOf = 0;
        Mtg_Event(d, MTG_EV_TURNED_UP, a->obj, p, 0);
        if (HasOther(mc)) {
            d->costAltIdx = (uint16_t)idx;
            PayCost(d, p, mc, 0xFFFF, a->obj, 3);
        }
        TrigEvent(d, MTG_TRIG_TURN_FACE_UP, a->obj, MTG_NO_OBJ, p);
        return;
    }
    if (d->stackCount >= MTG_MAX_STACK)
        return;
    if (a->kind == MTG_ACT_CAST && a->alt == MTG_ALT_MORPH) {   // face down: {3}, then a nameless 2/2 on the stack
        MtgManaCost three;
        MtgObj *o = &d->objs[a->obj];
        memset(&three, 0, sizeof(three));
        three.generic = 3;
        Mtg_Pay(d, p, &three, 0, MTG_NO_OBJ);
        Mtg_MoveTo(d, a->obj, MTG_ZONE_STACK);
        o->faceDownOf = (uint16_t)(o->def + 1);
        o->def = MTG_FACE_DOWN;
    } else if (a->kind == MTG_ACT_CAST && (a->alt == MTG_ALT_COST || a->alt == MTG_ALT_FLASHBACK)) {
        if (a->alt == MTG_ALT_FLASHBACK)   // (its flashback cost's mana instead of its own; the rest paid below)
            Mtg_Pay(d, p, &gMtgAltCosts[Mtg_Card(d->objs[a->obj].def)->flashback - 1].cost.mana, 0, MTG_NO_OBJ);
        Mtg_MoveTo(d, a->obj, MTG_ZONE_STACK);   // (its alternative cost instead of its mana: paid below)
    } else if (a->kind == MTG_ACT_CAST) {
        MtgManaCost cost;
        Mtg_SpellCost(d, p, a->obj, &cost);
        CastMana(Mtg_Card(d->objs[a->obj].def), a->alt, &cost);   // (kicked, buyback: theirs besides)
        Mtg_Pay(d, p, &cost, a->x, MTG_NO_OBJ);
        Mtg_MoveTo(d, a->obj, MTG_ZONE_STACK);
    } else {
        const MtgAbility *ab = &gMtgAbilities[a->ability];
        Mtg_Pay(d, p, &ab->cost.mana, a->x, ab->cost.tap ? a->obj : MTG_NO_OBJ);
        if (ab->cost.tap)
            Mtg_Tap(d, a->obj, 1);
        CountUse(d, a->obj, a->ability);
    }
    if (a->kind == MTG_ACT_CAST) {   // (how it was cast: before "whenever a player kicks a spell" looks)
        MtgObj *o = &d->objs[a->obj];
        o->kicked = (uint8_t)(a->alt == MTG_ALT_KICK1 ? 1 : a->alt == MTG_ALT_KICK2 ? 2 : a->alt == MTG_ALT_KICK12 ? 3 : 0);
        o->castMode = (uint8_t)(a->alt == MTG_ALT_BUYBACK ? 1 : a->alt == MTG_ALT_FLASHBACK ? 2 : 0);
    }
    e = &d->stack[d->stackCount++];
    memset(e, 0, sizeof(*e));
    e->isSpell = a->kind == MTG_ACT_CAST;
    e->controller = (uint8_t)p;
    e->source = a->obj;
    e->ability = a->ability;
    e->x = a->x;
    sCostX = a->x;   // (an X part of its costs pays this much)
    e->target = a->target;
    e->trigger = 0xFFFF;
    Mtg_Event(d, e->isSpell ? MTG_EV_CAST : MTG_EV_ACTIVATE, a->obj, a->target.kind, a->target.id);
    TargetEvent(d, a->target, a->obj, p);
    if (e->isSpell)
        TrigEvent(d, MTG_TRIG_SPELL_CAST, a->obj, MTG_NO_OBJ, p);
    if (!e->isSpell && (gMtgAbilities[a->ability].cost.sacSelf || gMtgAbilities[a->ability].cost.discardSelf))
        Mtg_MoveTo(d, a->obj, MTG_ZONE_GRAVEYARD);   // (the ability keeps its source's last known state; Cycling's discard)
    if (!e->isSpell && gMtgAbilities[a->ability].fromHand && gMtgAbilities[a->ability].cost.discardSelf)
        TrigEvent(d, MTG_TRIG_CYCLED, a->obj, MTG_NO_OBJ, p);   // ("when you cycle ...": TriggerCycled)
    if (a->kind == MTG_ACT_CAST && a->alt) {
        const MtgCardDef *c = Mtg_Card(d->objs[a->obj].def);
        int idx = ExtraIdx(c, a->alt);
        if (a->alt == MTG_ALT_COST || (idx && HasOther(&gMtgAltCosts[idx - 1].cost))) {
            d->costAltIdx = (uint16_t)idx;   // (that cost's picks: CostNow)
            PayCost(d, p, &gMtgAltCosts[idx - 1].cost, a->ability, a->obj, 1);
        } else if (a->ability != 0xFFFF)
            PayOtherCosts(d, p, &gMtgAbilities[a->ability], a->obj);
    } else if (a->ability != 0xFFFF)
        PayOtherCosts(d, p, &gMtgAbilities[a->ability], a->obj);
    d->tgtLeft = d->tgtOpt = 0;
    d->divPending = a->ability != 0xFFFF && Mtg_Param(&gMtgAbilities[a->ability], "DividedAsYouChoose") != NULL;
    d->divIdx = 0;
    if (a->ability != 0xFFFF && gMtgAbilities[a->ability].tgtMax > 1
        && (a->target.kind == MTG_TGT_OBJ || a->target.kind == MTG_TGT_PLAYER)) {
        const MtgAbility *ab = &gMtgAbilities[a->ability];   // (its other targets: asked after the costs)
        int most = ab->tgtMax == MTG_TGT_X ? a->x : ab->tgtMax, least = ab->tgtMin == MTG_TGT_X ? a->x : ab->tgtMin;
        if (ab->tgtMax == MTG_TGT_SVAR) {   // (MaxTgts: counted now)
            MtgResolveCtx ctx;
            int ok;
            ctx.e = e;
            ctx.controller = p;
            ctx.source = a->obj;
            most = Mtg_Amount(d, &ctx, ab->card, ab->tgtMaxSv, &ok);
        }
        int need = least > 1 ? least : 1;
        if (most > 5)
            most = 5;   // (the first and four more)
        if (need > most)
            need = most;
        d->tgtLeft = (uint8_t)(need - 1);
        d->tgtOpt = (uint8_t)(most - need);
    }
}

// the casting spell's (or ability's) other targets that may be picked now: not picked yet, legal as its first
static int TargetCandidates(const MtgDuel *d, uint16_t *out)
{
    const MtgStackEntry *e = &d->stack[d->stackCount - 1];
    const MtgAbility *ab = &gMtgAbilities[e->ability];
    int i, k, n = 0;
    for (i = 0; i < MTG_PLAYERS && ab->tgtZone == MTG_ZONE_BATTLEFIELD && !ab->tgtStack; i++) {   // (players: "Any")
        int taken = e->target.kind == MTG_TGT_PLAYER && e->target.id == i;
        for (k = 0; k < e->extraCount; k++)
            taken |= e->extra[k] == MTG_REM_PLAYER + i;
        if (!taken && !d->players[i].lost && Mtg_ValidPlayer(d, i, ab->tgt, e->controller))
            out[n++] = (uint16_t)(MTG_REM_PLAYER + i);
    }
    for (i = 0; i < d->objCount && n < MTG_MAX_COMBAT; i++) {
        int taken = e->target.kind == MTG_TGT_OBJ && e->target.id == i;
        for (k = 0; k < e->extraCount; k++)
            taken |= e->extra[k] == i;
        if (taken || d->objs[i].zone != ab->tgtZone || !Mtg_ValidObj(d, i, ab->tgt, e->source, e->controller)
            || (ab->tgtZone == MTG_ZONE_BATTLEFIELD && !Mtg_CanTarget(d, i, e->source)))
            continue;
        out[n++] = (uint16_t)i;
    }
    return n;
}

// ---- combat ----
static int Defender(const MtgDuel *d) { return d->active ^ 1; }

static int CanAttack(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    uint32_t kw = Mtg_Keywords(d, obj);
    return IsCreature(d, obj) && o->controller == d->active && !o->tapped
        && (!o->sick || (kw & MTG_KW_BIT(MTG_KW_HASTE))) && !(kw & MTG_KW_BIT(MTG_KW_DEFENDER)) && !(kw & MTG_KWX_CANT_ATTACK)
        && !Mtg_StaticForbids(d, MTG_STATIC_CANT_ATTACK, obj, MTG_NO_OBJ);
}

static int ControlsLandType(const MtgDuel *d, int player, int basic)
{
    int i;
    for (i = 0; i < d->objCount; i++)
        if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && d->objs[i].controller == player
            && (Mtg_Card(d->objs[i].def)->intrinsicMana & MTG_COLOR_BIT(basic)))
            return 1;
    return 0;
}

// CombatUtil.canBlock(attacker, blocker)
int Mtg_CanBlock(const MtgDuel *d, int blocker, int attacker)
{
    uint32_t akw = Mtg_Keywords(d, attacker), bkw = Mtg_Keywords(d, blocker);
    const MtgCardDef *ac = Mtg_Card(d->objs[attacker].def), *bc = Mtg_Card(d->objs[blocker].def);
    int i;
    if (bkw & MTG_KWX_CANT_BLOCK)   // (a Pump's "can't block" this turn)
        return 0;
    if ((akw & MTG_KW_BIT(MTG_KW_FLYING)) && !(bkw & (MTG_KW_BIT(MTG_KW_FLYING) | MTG_KW_BIT(MTG_KW_REACH))))
        return 0;
    if ((akw & MTG_KW_BIT(MTG_KW_FEAR)) && !(bc->types & MTG_T_ARTIFACT) && !(bc->colors & MTG_COLOR_BIT(MTG_B)))
        return 0;
    // (shadow: "can block or be blocked by only creatures with shadow", both ways)
    if (((akw ^ bkw) & MTG_KW_BIT(MTG_KW_SHADOW)) != 0)
        return 0;
    uint8_t walk = Mtg_Landwalk(d, attacker);
    (void)ac;
    if (Mtg_Protection(d, attacker) & bc->colors)
        return 0;
    if (Mtg_StaticForbids(d, MTG_STATIC_CANT_BLOCK, blocker, MTG_NO_OBJ)
        || Mtg_StaticForbids(d, MTG_STATIC_CANT_BLOCK_BY, attacker, blocker))
        return 0;
    for (i = 0; i < 5; i++)
        if ((walk & MTG_COLOR_BIT(i)) && ControlsLandType(d, d->objs[blocker].controller, i))
            return 0;
    return 1;
}

static void BuildAttackers(MtgDuel *d)
{
    MtgQuery *q = &d->query;
    int i;
    memset(q, 0, sizeof(*q));
    q->kind = MTG_Q_ATTACKERS;
    q->player = d->active;
    for (i = 0; i < d->objCount && q->candidateCount < MTG_MAX_COMBAT; i++)
        if (CanAttack(d, i))
            q->candidates[q->candidateCount++] = (uint16_t)i;
}

static void BuildBlockers(MtgDuel *d)
{
    MtgQuery *q = &d->query;
    int i, j;
    memset(q, 0, sizeof(*q));
    q->kind = MTG_Q_BLOCKERS;
    q->player = (uint8_t)Defender(d);
    for (i = 0; i < d->objCount && q->attackerCount < MTG_MAX_COMBAT; i++)
        if (d->objs[i].attacking && d->objs[i].zone == MTG_ZONE_BATTLEFIELD)
            q->attackers[q->attackerCount++] = (uint16_t)i;
    for (i = 0; i < d->objCount && q->candidateCount < MTG_MAX_COMBAT; i++) {
        if (!IsCreature(d, i) || d->objs[i].controller != q->player || d->objs[i].tapped)
            continue;
        for (j = 0; j < q->attackerCount; j++)
            if (Mtg_CanBlock(d, i, q->attackers[j])) {
                q->candidates[q->candidateCount++] = (uint16_t)i;
                break;
            }
    }
}

static int HasFirstStrike(const MtgDuel *d, int obj)
{
    return (Mtg_Keywords(d, obj) & (MTG_KW_BIT(MTG_KW_FIRST_STRIKE) | MTG_KW_BIT(MTG_KW_DOUBLE_STRIKE))) != 0;
}

static int DealsInStep(const MtgDuel *d, int obj, int firstStrikeStep)
{
    uint32_t kw = Mtg_Keywords(d, obj);
    if (firstStrikeStep)
        return HasFirstStrike(d, obj);
    return !(kw & MTG_KW_BIT(MTG_KW_FIRST_STRIKE)) || (kw & MTG_KW_BIT(MTG_KW_DOUBLE_STRIKE))
        || !d->objs[obj].dealtFirst;
}

// all combat damage of a step at once (510.2): gathered first, then dealt
static void CombatDamage(MtgDuel *d, int firstStrikeStep)
{
    struct { uint16_t src; MtgTarget to; int16_t amount; } hits[MTG_MAX_OBJS];
    int n = 0, i, j;
    MtgTarget player = {MTG_TGT_PLAYER, 0};
    player.id = (uint16_t)Defender(d);
    for (i = 0; i < d->objCount; i++) {
        int power;
        if (!IsCreature(d, i) || !DealsInStep(d, i, firstStrikeStep))
            continue;
        power = Mtg_Power(d, i);
        if (power <= 0)
            continue;
        if (d->objs[i].attacking) {
            int left = power, trample = (Mtg_Keywords(d, i) & MTG_KW_BIT(MTG_KW_TRAMPLE)) != 0, last = -1;
            if (!d->objs[i].blocked) {
                hits[n].src = (uint16_t)i, hits[n].to = player, hits[n].amount = (int16_t)power, n++;
            } else {
                // blockers in the order they were declared (seq): lethal damage to each in turn
                for (j = 0; j < d->objCount && left > 0; j++) {
                    int lethal;
                    if (d->objs[j].blocking != i || !IsCreature(d, j))
                        continue;
                    lethal = Mtg_Toughness(d, j) - d->objs[j].damage;
                    if (lethal < 0)
                        lethal = 0;
                    if (lethal > left)
                        lethal = left;
                    hits[n].src = (uint16_t)i, hits[n].to.kind = MTG_TGT_OBJ, hits[n].to.id = (uint16_t)j;
                    hits[n].amount = (int16_t)lethal;
                    last = n++;
                    left -= lethal;
                }
                if (left > 0) {
                    if (trample)
                        hits[n].src = (uint16_t)i, hits[n].to = player, hits[n].amount = (int16_t)left, n++;
                    else if (last >= 0)
                        hits[last].amount += (int16_t)left;
                }
            }
        } else if (d->objs[i].blocking != MTG_NO_OBJ) {
            hits[n].src = (uint16_t)i, hits[n].to.kind = MTG_TGT_OBJ, hits[n].to.id = d->objs[i].blocking;
            hits[n].amount = (int16_t)power, n++;
        } else
            continue;
        if (firstStrikeStep)
            d->objs[i].dealtFirst = 1;
    }
    for (i = 0; i < n; i++)
        Mtg_DealDamage(d, hits[i].src, hits[i].to, hits[i].amount, 1);
}

static int AnyFirstStrike(const MtgDuel *d)
{
    int i;
    for (i = 0; i < d->objCount; i++)
        if (IsCreature(d, i) && (d->objs[i].attacking || d->objs[i].blocking != MTG_NO_OBJ) && HasFirstStrike(d, i))
            return 1;
    return 0;
}

// ---- the turn ----
static void EndOfTurnCleanup(MtgDuel *d)
{
    int i;
    PruneEffects(d, MTG_EFF_EOT, 0);
    d->fog = 0;
    for (i = 0; i < MTG_PLAYERS; i++)
        d->players[i].prevent = 0;
    for (i = 0; i < d->objCount; i++) {
        MtgObj *o = &d->objs[i];
        o->damage = 0;
        o->damagedBy[0] = o->damagedBy[1] = o->damagedBy[2] = MTG_NO_OBJ;
        o->tempPower = o->tempToughness = 0;
        o->tempKeywords = 0;
        o->tempProtection = 0;
        o->loseKeywords = 0;
        o->regen = 0;
        o->prevent = 0;
        if (o->animUntil == 1)
            o->animUntil = 0;   // (an Animate until end of turn wears off)
        if (o->effUntil & MTG_CTRL_EOT)
            o->effCtrl = 0;     // ("until end of turn": Threaten's)
    }
    ControlLayer(d);   // (back before the next untap step)
}

static void NextStep(MtgDuel *d)
{
    int p;
    for (p = 0; p < MTG_PLAYERS; p++)
        memset(d->players[p].pool, 0, sizeof(d->players[p].pool));   // mana empties between steps
    d->stepStarted = 0;
    d->passes = 0;
    if (d->step == MTG_STEP_CLEANUP) {
        d->active ^= 1;
        d->turn++;
        d->step = MTG_STEP_UNTAP;
        d->players[d->active].landsPlayed = 0;
        Mtg_Event(d, MTG_EV_TURN, d->active, d->turn, 0);
        if (d->turn > MTG_TURN_LIMIT) {
            d->over = 1;
            d->winner = -1;
        }
    } else
        d->step++;
    d->priority = d->active;
    Mtg_Event(d, MTG_EV_STEP, d->step, d->active, 0);
}

static void EndCombat(MtgDuel *d)
{
    int i;
    for (i = 0; i < d->objCount; i++) {
        d->objs[i].attacking = d->objs[i].blocked = d->objs[i].dealtFirst = 0;
        d->objs[i].blocking = MTG_NO_OBJ;
    }
}

static int HandCount(const MtgDuel *d, int player)
{
    int i, n = 0;
    for (i = 0; i < d->objCount; i++)
        n += d->objs[i].zone == MTG_ZONE_HAND && d->objs[i].owner == player;
    return n;
}

static void BuildDiscard(MtgDuel *d)
{
    MtgQuery *q = &d->query;
    int i;
    memset(q, 0, sizeof(*q));
    q->kind = MTG_Q_DISCARD;
    q->player = d->active;
    q->discard = (uint8_t)(HandCount(d, d->active) - MTG_HAND_SIZE);
    for (i = 0; i < d->objCount && q->candidateCount < MTG_MAX_COMBAT; i++)
        if (d->objs[i].zone == MTG_ZONE_HAND && d->objs[i].owner == d->active)
            q->candidates[q->candidateCount++] = (uint16_t)i;
}

// blocks are declared: Blocks, AttackerBlocked, AttackerBlockedByCreature, AttackerUnblocked
static void BlocksDeclared(MtgDuel *d)
{
    int i, j, def = Defender(d);
    for (i = 0; i < d->objCount; i++)
        if (d->objs[i].blocking != MTG_NO_OBJ && d->objs[i].zone == MTG_ZONE_BATTLEFIELD)
            TrigEvent(d, MTG_TRIG_BLOCKS, i, d->objs[i].blocking, def);
    for (i = 0; i < d->objCount; i++) {
        int first = MTG_NO_OBJ;
        if (!d->objs[i].attacking || d->objs[i].zone != MTG_ZONE_BATTLEFIELD)
            continue;
        if (!d->objs[i].blocked) {
            TrigEvent(d, MTG_TRIG_ATTACKER_UNBLOCKED, i, MTG_NO_OBJ, def);
            continue;
        }
        for (j = 0; j < d->objCount; j++)
            if (d->objs[j].blocking == i) {
                if (first == MTG_NO_OBJ)
                    first = j;
                // Flanking (Keyword.FLANKING's trigger, resolved at once: nothing can answer it that matters): a blocker
                // without flanking gets -1/-1 until end of turn
                if ((Mtg_Keywords(d, i) & MTG_KW_BIT(MTG_KW_FLANKING)) && !(Mtg_Keywords(d, j) & MTG_KW_BIT(MTG_KW_FLANKING))) {
                    d->objs[j].tempPower--;
                    d->objs[j].tempToughness--;
                }
                TrigEvent(d, MTG_TRIG_ATTACKER_BLOCKED_BY, i, j, def);
            }
        TrigEvent(d, MTG_TRIG_ATTACKER_BLOCKED, i, first, def);
        // Rampage N (its "becomes blocked" trigger, resolved at once): +N/+N for each blocker past the first
        if (Mtg_Card(d->objs[i].def)->rampage) {
            int blockers = 0;
            for (j = 0; j < d->objCount; j++)
                blockers += d->objs[j].blocking == i;
            if (blockers > 1) {
                d->objs[i].tempPower += (int8_t)(Mtg_Card(d->objs[i].def)->rampage * (blockers - 1));
                d->objs[i].tempToughness += (int8_t)(Mtg_Card(d->objs[i].def)->rampage * (blockers - 1));
            }
        }
    }
}

// "You may choose not to untap CARDNAME": chosen while an UntilUntaps Effect of its own holds something
static int HoldsLock(const MtgDuel *d, int obj)
{
    int k;
    if (!Mtg_Card(d->objs[obj].def)->mayNotUntap)
        return 0;
    for (k = 0; k < d->effCount; k++)
        if (d->effects[k].until == MTG_EFF_UNTIL_UNTAPS && d->effects[k].source == obj && Mtg_EffectAlive(d, &d->effects[k]))
            return 1;
    return 0;
}

// the step's turn-based actions; returns 1 if it is waiting for a decision (the query is set)
static int StartStep(MtgDuel *d)
{
    int i;
    d->stepStarted = 1;
    switch (d->step) {
    case MTG_STEP_UNTAP:
        PruneEffects(d, MTG_EFF_NEXT_TURN, d->active);   // ("until your next turn")
        for (i = 0; i < d->objCount; i++)
            d->objs[i].usedN[0] = d->objs[i].usedN[1] = 0;   // (ActivationLimit$: each turn)
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].controller == d->active && d->objs[i].zone == MTG_ZONE_BATTLEFIELD) {
                if (!d->objs[i].tapped || (!Mtg_StaticForbids(d, MTG_STATIC_NO_UNTAP, i, MTG_NO_OBJ) && !HoldsLock(d, i)))
                    Mtg_Tap(d, i, 0);
                d->objs[i].sick = 0;
            }
        PruneEffects(d, MTG_EFF_AFTER_UNTAP, d->active);   // ("doesn't untap during your next untap step": done)
        break;
    case MTG_STEP_UPKEEP:
        for (i = 0; i < d->objCount; i++)   // (Echo: what came under the player since their last upkeep is due now)
            if (d->objs[i].controller == d->active && d->objs[i].echoDue == 1)
                d->objs[i].echoDue = 2;
        // Fading (its upkeep trigger, done as the step begins): a fade counter off, or sacrificed when there's none
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].controller == d->active && d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_Card(d->objs[i].def)->fading) {
                int kind = Mtg_Card(d->objs[i].def)->etbKind;
                if (Mtg_Counters(d, i, kind) > 0)
                    Mtg_AddCountersKind(d, i, kind, -1);
                else
                    Mtg_Sacrifice(d, i);
            }
        break;
    case MTG_STEP_DRAW:
        for (i = 0; i < d->objCount; i++)   // (Echo: due only in that upkeep)
            if (d->objs[i].controller == d->active && d->objs[i].echoDue == 2)
                d->objs[i].echoDue = 0;
        if (d->turn > 1)   // the starting player skips the first draw
            Mtg_Draw(d, d->active, 1);
        break;
    case MTG_STEP_COMBAT_ATTACKERS:
        BuildAttackers(d);
        if (d->query.candidateCount) {
            d->stepStarted = 2;
            return 1;
        }
        d->step = MTG_STEP_COMBAT_END - 1;   // nobody can attack: no combat damage steps
        d->stepStarted = 3;
        break;
    case MTG_STEP_COMBAT_BLOCKERS:
        BuildBlockers(d);
        if (d->query.candidateCount) {
            d->stepStarted = 2;
            return 1;
        }
        BlocksDeclared(d);   // (nobody could block)
        break;
    case MTG_STEP_COMBAT_FIRST_STRIKE:
        d->combatHadFirstStrike = (uint8_t)AnyFirstStrike(d);
        if (!d->combatHadFirstStrike)
            d->stepStarted = 3;   // (skipped)
        else
            CombatDamage(d, 1);
        break;
    case MTG_STEP_COMBAT_DAMAGE:
        CombatDamage(d, 0);
        break;
    case MTG_STEP_COMBAT_END:
        EndCombat(d);
        PruneEffects(d, MTG_EFF_END_COMBAT, 0);
        break;
    case MTG_STEP_CLEANUP:
        if (HandCount(d, d->active) > MTG_HAND_SIZE) {
            BuildDiscard(d);
            d->stepStarted = 2;
            return 1;
        }
        EndOfTurnCleanup(d);
        d->stepStarted = 3;
        break;
    }
    switch (d->step) {   // "at the beginning of ..." (TriggerPhase), after the step's own actions
    case MTG_STEP_UPKEEP: case MTG_STEP_DRAW: case MTG_STEP_MAIN1: case MTG_STEP_COMBAT_BEGIN:
    case MTG_STEP_COMBAT_END: case MTG_STEP_MAIN2: case MTG_STEP_END:
        TrigEvent(d, MTG_TRIG_PHASE, MTG_NO_OBJ, MTG_NO_OBJ, d->active);
        Mtg_FireDelayed(d);   // ("at the beginning of the next ...": DelayedTrigger)
        break;
    }
    if (d->step == MTG_STEP_END && d->eotCount) {   // AtEOT$: what was to go at the beginning of the next end step
        int k, n = d->eotCount;
        d->eotCount = 0;
        for (k = 0; k < n; k++) {
            int o = d->eotObj[k];
            if (d->objs[o].zone != MTG_ZONE_BATTLEFIELD || d->objs[o].seq != d->eotSeq[k])
                continue;   // (gone, or a new object since)
            if (d->eotAct[k] == 1)
                Mtg_Sacrifice(d, o);
            else if (d->eotAct[k] == 2)
                Mtg_Destroy(d, o, 0);
            else
                Mtg_MoveTo(d, o, d->eotAct[k] == 3 ? MTG_ZONE_EXILE : MTG_ZONE_HAND);
        }
    }
    return 0;
}

// a trigger waiting goes on the stack with its target (TriggerHandler.runWaitingTrigger, WrappedAbility)
static void PushTrigger(MtgDuel *d, int i, MtgTarget target)
{
    MtgPending pd = d->pending[i];
    const MtgTrigger *t = &gMtgTriggers[pd.trigger];
    MtgStackEntry *e;
    memmove(&d->pending[i], &d->pending[i + 1], (d->pendingCount - i - 1) * sizeof(d->pending[0]));
    d->pendingCount--;
    if (d->stackCount >= MTG_MAX_STACK)
        return;
    e = &d->stack[d->stackCount++];
    memset(e, 0, sizeof(*e));
    e->controller = pd.controller;
    e->source = pd.source;
    e->ability = t->execute;
    e->target = target;
    e->trigger = pd.trigger;
    e->trigObj = pd.trigObj;
    e->trigObj2 = pd.trigObj2;
    e->trigPlayer = pd.trigPlayer;
    e->trigCtrl = pd.trigCtrl;
    e->trigAmount = pd.trigAmount;
    e->optional = t->optional;
    e->effRemCount = pd.effRemCount;
    memcpy(e->effRem, pd.effRem, sizeof(e->effRem));
    d->passes = 0;
    Mtg_Event(d, MTG_EV_TRIGGER, pd.source, target.kind, target.id);
    TargetEvent(d, target, pd.source, pd.controller);
}

static int NextPending(const MtgDuel *d)   // the active player's first (101.4: APNAP), in the order they happened
{
    int i;
    for (i = 0; i < d->pendingCount; i++)
        if (d->pending[i].controller == d->active)
            return i;
    return 0;
}

// the waiting triggers onto the stack; 1 while one waits for its target to be chosen (MTG_Q_TRIGGER_TARGET)
static int PlacePending(MtgDuel *d)
{
    while (d->pendingCount && !d->over) {
        int i = NextPending(d);
        const MtgPending *pd = &d->pending[i];
        const MtgTrigger *t = &gMtgTriggers[pd->trigger];
        MtgTarget target = {MTG_TGT_NONE, 0};
        if (gMtgAbilities[t->execute].tgt) {
            MtgQuery *q = &d->query;
            memset(q, 0, sizeof(*q));
            AddWithTargets(d, q, MTG_ACT_ACTIVATE, pd->source, t->execute, 0, pd->controller);
            if (q->actionCount == 0) {   // no legal target: it does nothing (603.3d)
                memmove(&d->pending[i], &d->pending[i + 1], (d->pendingCount - i - 1) * sizeof(d->pending[0]));
                d->pendingCount--;
                continue;
            }
            if (q->actionCount > 1) {
                q->kind = MTG_Q_TRIGGER_TARGET;
                q->player = pd->controller;
                q->source = pd->source;
                q->ability = t->execute;
                return 1;
            }
            target = q->actions[0].target;
        }
        PushTrigger(d, i, target);
    }
    return 0;
}

static void Run(MtgDuel *d)
{
    int guard = 0;
    while (guard++ < 100000) {
        if (d->over) {
            memset(&d->query, 0, sizeof(d->query));
            d->query.kind = MTG_Q_GAME_OVER;
            d->query.winner = d->winner;
            return;
        }
        if (d->etbCount && RunEtb(d))
            return;
        if (d->fxCount && RunManaFx(d))
            return;
        if (d->stepStarted == 0 && StartStep(d))
            return;
        if (d->stepStarted == 3 || d->step == MTG_STEP_UNTAP || d->step == MTG_STEP_CLEANUP) {
            NextStep(d);   // (no priority in this step)
            continue;
        }
        StateBasedActions(d);
        if (d->over)
            continue;
        Mtg_CheckStateTriggers(d);   // (Mode$ Always)
        if (d->pendingCount && PlacePending(d))
            return;
        if (d->over)
            continue;
        BuildPriority(d);
        return;
    }
    d->over = 1;   // (a runaway loop: never seen; ends the duel as a draw rather than hang the game)
    d->winner = -1;
}

const MtgQuery *Mtg_Query(const MtgDuel *d) { return &d->query; }

void Mtg_RefreshQuery(MtgDuel *d)   // (after the state was changed from outside: tests, debug commands)
{
    if (d->query.kind == MTG_Q_PRIORITY) {
        StateBasedActions(d);
        if (d->over || d->etbCount || d->fxCount)
            Run(d);
        else
            BuildPriority(d);
    }
}

int Mtg_Answer(MtgDuel *d, const MtgAnswer *a)
{
    MtgQuery *q = &d->query;
    int i;
    switch (q->kind) {
    case MTG_Q_PRIORITY: {
        MtgAction act;
        if (a->action >= q->actionCount)
            return 0;
        act = q->actions[a->action];
        if (act.kind == MTG_ACT_PASS) {
            if (++d->passes >= MTG_PLAYERS) {
                d->passes = 0;
                if (d->stackCount) {
                    d->priority = d->active;
                    if (ResolveTop(d))
                        return 1;   // (it asks something as it resolves)
                } else
                    NextStep(d);
            } else
                d->priority ^= 1;
        } else {
            if (act.x > 0 && !act.xFixed) {   // an X: asked first (0 up to the most it can pay)
                uint16_t opts[MTG_MAX_OPTIONS];
                int k, n = act.x + 1 < MTG_MAX_OPTIONS ? act.x + 1 : MTG_MAX_OPTIONS;
                for (k = 0; k < n; k++)
                    opts[k] = (uint16_t)k;
                d->pendAct = act;
                Mtg_AskOption(d, d->priority, MTG_OPT_NUMBER, opts, n);
                d->query.forCost = 8;
                d->query.source = act.obj;
                d->query.ability = act.ability;
                return 1;
            }
            DoAction(d, &act);
            d->passes = 0;
            if (AskCost(d))
                return 1;
            if (ResolveManaNow(d))
                return 1;
        }
        break;
    }
    case MTG_Q_ATTACKERS: {
        MtgAnswer forced = *a;   // ("attacks each combat if able": StaticAbilityMustAttack, whatever was answered)
        for (i = 0; i < q->candidateCount && i < 32; i++)
            if (Mtg_StaticForbids(d, MTG_STATIC_MUST_ATTACK, q->candidates[i], MTG_NO_OBJ))
                forced.attack |= 1u << i;
        a = &forced;
        {   // attack taxes (Propaganda): what can't be paid for is dropped, the last chosen first, then the rest paid
            int tax, k;
            MtgManaCost m;
            for (;;) {
                tax = 0;
                for (k = 0; k < q->candidateCount && k < 32; k++)
                    if (forced.attack & (1u << k))
                        tax += Mtg_AttackTax(d, q->candidates[k]);
                memset(&m, 0, sizeof(m));
                m.generic = (uint8_t)(tax > 255 ? 255 : tax);
                if (!tax || Mtg_CanPay(d, q->player, &m, 0, MTG_NO_OBJ))
                    break;
                for (k = (q->candidateCount < 32 ? q->candidateCount : 32) - 1; k >= 0; k--)
                    if ((forced.attack & (1u << k)) && Mtg_AttackTax(d, q->candidates[k])) {
                        forced.attack &= ~(1u << k);
                        break;
                    }
                if (k < 0)
                    break;
            }
            if (tax)
                Mtg_Pay(d, q->player, &m, 0, MTG_NO_OBJ);
        }
        for (i = 0; i < q->candidateCount; i++)
            if (a->attack & (1u << i)) {
                int o = q->candidates[i];
                d->objs[o].attacking = 1;
                d->objs[o].attackedTurn = (uint16_t)(d->turn + 1);
                if (!(Mtg_Keywords(d, o) & MTG_KW_BIT(MTG_KW_VIGILANCE)))
                    Mtg_Tap(d, o, 1);
                Mtg_Event(d, MTG_EV_ATTACK, o, 0, 0);
            }
        for (i = 0; i < q->candidateCount; i++)
            if (a->attack & (1u << i))
                TrigEvent(d, MTG_TRIG_ATTACKS, q->candidates[i], MTG_NO_OBJ, Defender(d));
        d->stepStarted = 1;
        if (!(a->attack & ((q->candidateCount >= 32 ? 0u : (1u << q->candidateCount)) - 1u))) {
            d->step = MTG_STEP_COMBAT_END - 1;   // no attackers: straight to the end of combat
            d->stepStarted = 3;
        }
        break;
    }
    case MTG_Q_BLOCKERS: {
        static MtgAnswer fixedB;
        for (i = 0; i < q->candidateCount; i++) {
            int b = q->candidates[i], k = a->block[i];
            if (k < 0)
                continue;
            if (k >= q->attackerCount || !Mtg_CanBlock(d, b, q->attackers[k]))
                return 0;
        }
        fixedB = *a;   // (menace: a lone blocker of one doesn't block)
        for (i = 0; i < q->attackerCount; i++) {
            int n = 0, k;
            if (!(Mtg_Keywords(d, q->attackers[i]) & MTG_KW_BIT(MTG_KW_MENACE)))
                continue;
            for (k = 0; k < q->candidateCount; k++)
                n += fixedB.block[k] == i;
            if (n == 1)
                for (k = 0; k < q->candidateCount; k++)
                    if (fixedB.block[k] == i)
                        fixedB.block[k] = -1;
        }
        a = &fixedB;
        for (i = 0; i < q->candidateCount; i++) {
            int b = q->candidates[i], k = a->block[i];
            if (k < 0)
                continue;
            d->objs[b].blocking = q->attackers[k];
            d->objs[q->attackers[k]].blocked = 1;
            Mtg_Event(d, MTG_EV_BLOCK, b, q->attackers[k], 0);
        }
        BlocksDeclared(d);
        d->stepStarted = 1;
        break;
    }
    case MTG_Q_DISCARD:
        if (a->card >= q->candidateCount)
            return 0;
        Mtg_MoveTo(d, q->candidates[a->card], MTG_ZONE_GRAVEYARD);
        d->stepStarted = 0;   // (again: still too many, or the rest of the cleanup)
        break;
    case MTG_Q_TRIGGER_TARGET:
        if (a->action >= q->actionCount)
            return 0;
        PushTrigger(d, NextPending(d), q->actions[a->action].target);
        break;
    case MTG_Q_CHOOSE:
        if (q->forCost == 9) {   // a share of a divided amount, to the next target
            if (a->card >= q->optCount || !d->stackCount)
                return 0;
            d->stack[d->stackCount - 1].divide[d->divIdx++] = (uint8_t)q->opts[a->card];
            if (AskCost(d))
                return 1;
            break;
        }
        if (q->forCost == 8) {   // the X of the action chosen: now it's done
            MtgAction act = d->pendAct;
            if (a->card >= q->optCount)
                return 0;
            act.x = (int16_t)q->opts[a->card];
            DoAction(d, &act);
            d->passes = 0;
            if (AskCost(d))
                return 1;
            if (ResolveManaNow(d))
                return 1;
            break;
        }
        if (q->forCost) {   // a cost's sacrifice / discard (or another target)
            if (q->forCost == 7 && a->card == q->candidateCount && !d->tgtLeft) {
                d->tgtOpt = 0;   // (no more targets: it may stop here; then its division, if any)
                if (AskCost(d))
                    return 1;
                break;
            }
            if (a->card >= q->candidateCount)
                return 0;
            PayCostPick(d, q->candidates[a->card]);
            if (AskCost(d))
                return 1;
            if (!d->costAlt && ResolveManaNow(d))
                return 1;
            if (d->costAlt == 2 || d->costAlt == 4) {   // an UnlessCost$ (or a trigger's own cost) paid: it goes on resolving
                d->costAlt = 0;
                if (d->resolving && ContinueResolve(d))
                    return 1;
                d->priority = d->active;
                d->passes = 0;
            }
            break;
        }
        if (q->choice == MTG_CHOICE_PICK) {
            if (a->card >= q->candidateCount)
                return 0;
            d->resChoice = (int16_t)q->candidates[a->card];
        } else if (q->choice == MTG_CHOICE_OPTION) {
            if (a->card >= q->optCount)
                return 0;
            d->resChoice = (int16_t)q->opts[a->card];
        } else
            d->resChoice = a->card ? 1 : 0;
        if (ContinueResolve(d))
            return 1;
        d->priority = d->active;
        d->passes = 0;
        break;
    default:
        return 0;
    }
    Run(d);
    return 1;
}
