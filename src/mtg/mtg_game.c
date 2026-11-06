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
int Mtg_Power(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return (m.setPower != MTG_STATIC_NO_SET ? m.setPower : Mtg_Card(o->def)->power) + m.addPower + o->counters11
         + o->tempPower;
}

int Mtg_Toughness(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return (m.setToughness != MTG_STATIC_NO_SET ? m.setToughness : Mtg_Card(o->def)->toughness) + m.addToughness
         + o->counters11 + o->tempToughness;
}

uint32_t Mtg_Keywords(const MtgDuel *d, int obj)
{
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return Mtg_Card(d->objs[obj].def)->keywords | d->objs[obj].tempKeywords | m.keywords;
}

uint8_t Mtg_Landwalk(const MtgDuel *d, int obj)
{
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return Mtg_Card(d->objs[obj].def)->landwalk | m.landwalk;
}

uint8_t Mtg_Protection(const MtgDuel *d, int obj)
{
    MtgStaticMods m;
    Mtg_StaticMods(d, obj, &m);
    return Mtg_Card(d->objs[obj].def)->protection | m.protection;
}

static int IsCreature(const MtgDuel *d, int obj)
{
    return d->objs[obj].zone == MTG_ZONE_BATTLEFIELD && (Mtg_Card(d->objs[obj].def)->types & MTG_T_CREATURE);
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
    o->counters11 = 0;
    o->attachedTo = MTG_NO_OBJ;
    o->regen = 0;
}

// GameAction.changeZone: a card moves (to the library's top when it goes there); a permanent leaving forgets its state
void Mtg_MoveTo(MtgDuel *d, int obj, int zone)
{
    MtgObj *o = &d->objs[obj];
    int from = o->zone, i;
    MtgTrigEvent ev;
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
    if (zone != MTG_ZONE_BATTLEFIELD)
        o->controller = o->owner;
    o->zone = (uint8_t)zone;
    o->seq = ++d->seq;
    if (zone == MTG_ZONE_LIBRARY) {
        MtgPlayer *p = &d->players[o->owner];
        p->library[p->libraryCount++] = (uint16_t)obj;
    }
    Mtg_Event(d, MTG_EV_ZONE, obj, from, zone);
    if (zone == MTG_ZONE_BATTLEFIELD && Mtg_StaticForbids(d, MTG_STATIC_ETB_TAPPED, obj, MTG_NO_OBJ))
        Mtg_Tap(d, obj, 1);   // (it enters tapped: ETBTapped)
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
    if (amount <= 0)
        return;
    if (to.kind == MTG_TGT_OBJ) {
        if (d->objs[to.id].zone != MTG_ZONE_BATTLEFIELD)
            return;
        if (sourceObj != MTG_NO_OBJ && (Mtg_Protection(d, to.id) & Mtg_ObjColors(d, sourceObj)))
            return;
        if (!(Mtg_Card(d->objs[to.id].def)->types & MTG_T_CREATURE))
            return;   // (planeswalkers: not yet)
        d->objs[to.id].damage += amount;
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
    }
}

void Mtg_Destroy(MtgDuel *d, int obj, int noRegen)
{
    MtgObj *o = &d->objs[obj];
    if (o->zone != MTG_ZONE_BATTLEFIELD)
        return;
    if (o->regen && !noRegen) {   // (701.15: tapped, removed from combat, its damage gone; the shield used up)
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
static uint8_t Mtg_AuraOwnProtection(int def)
{
    const MtgCardDef *c = Mtg_Card(def);
    uint8_t p = 0;
    int k;
    for (k = 0; k < c->staticCount; k++)
        if (gMtgStatics[c->statics[k]].enchanted)
            p |= gMtgStatics[c->statics[k]].addProtection;
    return p;
}

// GameAction.checkStateEffects
static void StateBasedActions(MtgDuel *d)
{
    int again = 1, i, lost[MTG_PLAYERS] = {0, 0};
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
                || (Mtg_Protection(d, to) & Mtg_ObjColors(d, i) & ~Mtg_AuraOwnProtection(o->def))) {
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

// ---- the stack ----
static int TargetStillLegal(const MtgDuel *d, const MtgStackEntry *e, const MtgAbility *ab)
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

static void FinishResolve(MtgDuel *d)
{
    const MtgStackEntry *e = &d->res;
    const MtgCardDef *c = Mtg_Card(d->objs[e->source].def);
    d->resolving = 0;
    if (e->isSpell && d->objs[e->source].zone == MTG_ZONE_STACK) {
        if (c->types & (MTG_T_INSTANT | MTG_T_SORCERY))
            Mtg_MoveTo(d, e->source, MTG_ZONE_GRAVEYARD);
        else {
            d->objs[e->source].controller = e->controller;
            Mtg_MoveTo(d, e->source, MTG_ZONE_BATTLEFIELD);
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
    if (d->res.optional == 1) {   // "you may" (OptionalDecider$ You): asked as it resolves
        if (d->resChoice < 0) {
            Mtg_AskChoice(d, d->res.controller, MTG_CHOICE_YESNO, NULL);
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
        d->resAbility = gMtgAbilities[d->resAbility].sub;
        d->resStage = d->resPicks = 0;
        d->resChoice = -1;
    }
    FinishResolve(d);
    return 0;
}

static int ResolveTop(MtgDuel *d)
{
    MtgStackEntry e = d->stack[--d->stackCount];
    const MtgAbility *ab = e.ability != 0xFFFF ? &gMtgAbilities[e.ability] : NULL;
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
    d->res = e;
    d->resolving = 1;
    d->resAbility = e.ability;
    d->resStage = d->resPicks = 0;
    d->resChoice = -1;
    return ContinueResolve(d);
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
}

static void AddWithTargets(const MtgDuel *d, MtgQuery *q, int kind, int obj, int ability, int x, int player)
{
    const MtgAbility *ab = ability != 0xFFFF ? &gMtgAbilities[ability] : NULL;
    MtgTarget t = {MTG_TGT_NONE, 0};
    int i;
    if (!ab || !ab->tgt) {
        AddAction(q, kind, obj, ability, x, t);
        return;
    }
    if (ab->tgtZone != MTG_ZONE_BATTLEFIELD) {   // (cards in a graveyard)
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].zone == ab->tgtZone && Mtg_ValidObj(d, i, ab->tgt, obj, player)) {
                t.kind = MTG_TGT_OBJ;
                t.id = (uint16_t)i;
                AddAction(q, kind, obj, ability, x, t);
            }
        return;
    }
    if (ab->tgtStack) {
        for (i = d->stackCount - 1; i >= 0; i--)
            if (d->stack[i].isSpell && Mtg_ValidObj(d, d->stack[i].source, ab->tgt, obj, player)) {
                t.kind = MTG_TGT_STACK;
                t.id = d->stack[i].source;
                AddAction(q, kind, obj, ability, x, t);
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
        if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, ab->tgt, obj, player)
            && Mtg_CanTarget(d, i, obj)) {
            t.kind = MTG_TGT_OBJ;
            t.id = (uint16_t)i;
            AddAction(q, kind, obj, ability, x, t);
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
    if (!Mtg_CostPayable(d, p, &alt->cost, obj, obj))
        return;
    if (ability != 0xFFFF) {
        const MtgCost *own = &gMtgAbilities[ability].cost;
        if (own->sacN || own->discardN || own->payLife || own->exileHandN || own->returnN || own->tapTypeN)
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
                if (!ab->fromHand || ab->kind != MTG_AB_ACTIVATED || ab->cost.mana.x)
                    continue;
                if (!Mtg_CanPay(d, p, &ab->cost.mana, 0, MTG_NO_OBJ) || !Mtg_CostPayable(d, p, &ab->cost, i, MTG_NO_OBJ))
                    continue;
                AddWithTargets(d, q, MTG_ACT_ACTIVATE, i, c->abilities[k], 0, p);
            }
            if (c->types & MTG_T_LAND) {
                if (SorceryTiming(d, p) && d->players[p].landsPlayed == 0)
                    AddAction(q, MTG_ACT_LAND, i, 0xFFFF, 0, none);
                continue;
            }
            if (!(c->types & MTG_T_INSTANT) && !(c->keywords & MTG_KW_BIT(MTG_KW_FLASH)) && !SorceryTiming(d, p))
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
            if (ability != 0xFFFF && gMtgAbilities[ability].api == MTG_API_CHARM) {   // one action per mode
                const MtgAbility *ch = &gMtgAbilities[ability];
                for (k = 0; k < ch->choiceCount; k++)
                    AddWithTargets(d, q, MTG_ACT_CAST, i, ch->choices[k], x, p);
                continue;
            }
            AddWithTargets(d, q, MTG_ACT_CAST, i, ability, x, p);
        } else if (o->zone == MTG_ZONE_BATTLEFIELD && o->controller == p) {
            for (k = 0; k < c->abilityCount; k++) {
                const MtgAbility *ab = &gMtgAbilities[c->abilities[k]];
                int x = 0;
                if (ab->kind != MTG_AB_ACTIVATED || ab->api == MTG_API_MANA || ab->fromHand)
                    continue;   // (mana abilities are used by paying, never on their own; Cycling only from the hand)
                if (ab->cost.tap && (o->tapped || ((c->types & MTG_T_CREATURE) && o->sick
                                                   && !(Mtg_Keywords(d, i) & MTG_KW_BIT(MTG_KW_HASTE)))))
                    continue;
                if (ab->cost.mana.x) {
                    x = Mtg_MaxX(d, p, &ab->cost.mana, ab->cost.tap ? i : MTG_NO_OBJ);
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
                AddWithTargets(d, q, MTG_ACT_ACTIVATE, i, c->abilities[k], x, p);
            }
        }
    }
}

// ---- costs beyond mana and {T} (CostSacrifice, CostDiscard, CostPayLife, CostExile, CostReturn, CostTapType)
// what may pay a part of KIND (MTG_COSTK_*): the player's permanents (sacrifice, return, tap: untapped ones) or hand
// cards (discard, exile) of its spec; a card being cast is on the stack by then. Returns how many (out may be NULL)
int Mtg_CostCandidates(const MtgDuel *d, int player, const MtgCost *c, int kind, int source, uint16_t *out)
{
    int i, n = 0, hand = kind == MTG_COSTK_DISCARD || kind == MTG_COSTK_EXILE_HAND;
    const char *spec = kind == MTG_COSTK_SAC ? c->sacSpec : kind == MTG_COSTK_DISCARD ? c->discardSpec
                     : kind == MTG_COSTK_EXILE_HAND ? c->exileHandSpec : kind == MTG_COSTK_RETURN ? c->returnSpec
                     : c->tapTypeSpec;
    if (!spec)
        return 0;
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        if (hand ? (o->zone != MTG_ZONE_HAND || o->owner != player)
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
    static const uint8_t sKinds[] = {MTG_COSTK_SAC, MTG_COSTK_DISCARD, MTG_COSTK_EXILE_HAND, MTG_COSTK_RETURN, MTG_COSTK_TAP};
    unsigned k;
    if (c->payLife && d->players[player].life < c->payLife)
        return 0;
    for (k = 0; k < sizeof(sKinds); k++) {
        int kind = sKinds[k], need = kind == MTG_COSTK_SAC ? c->sacN : kind == MTG_COSTK_DISCARD ? (c->discardN)
                 : kind == MTG_COSTK_EXILE_HAND ? c->exileHandN : kind == MTG_COSTK_RETURN ? c->returnN : c->tapTypeN, n;
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
    if (d->costAlt)
        return &gMtgAltCosts[Mtg_Card(d->objs[d->costSource].def)->altCost - 1].cost;
    return &gMtgAbilities[d->costAbility].cost;
}

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
    else
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
        d->costSac = d->costDiscard = d->costExile = d->costReturn = d->costTap = 0;
        return 0;
    }
    return 1;
}

static void PayCostPick(MtgDuel *d, int obj)
{
    if (d->costSac) {
        if (d->stackCount)   // (its last known power, for Sacrificed$CardPower)
            d->stack[d->stackCount - 1].sacPower = (int16_t)Mtg_Power(d, obj);
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
    }
}

// the costs paid at once (life, a random discard, the source exiled or returned); the picks are set to be asked
static void PayCost(MtgDuel *d, int p, const MtgCost *c, int ability, int source, int alt)
{
    if (c->payLife) {
        d->players[p].life -= c->payLife;
        Mtg_Event(d, MTG_EV_LIFE, p, -c->payLife, d->players[p].life);
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
    d->costSac = c->sacN;
    d->costDiscard = c->discardRandom ? 0 : c->discardN;
    d->costExile = c->exileHandN;
    d->costReturn = c->returnN;
    d->costTap = c->tapTypeN;
    d->costPlayer = (uint8_t)p;
    d->costSource = (uint16_t)source;
    d->costAbility = (uint16_t)ability;
    d->costAlt = (uint8_t)alt;
}

static void PayOtherCosts(MtgDuel *d, int p, const MtgAbility *ab, int source)
{
    PayCost(d, p, &ab->cost, (int)(ab - gMtgAbilities), source, 0);
}

static void DoAction(MtgDuel *d, const MtgAction *a)
{
    int p = d->priority;
    MtgStackEntry *e;
    if (a->kind == MTG_ACT_LAND) {
        Mtg_MoveTo(d, a->obj, MTG_ZONE_BATTLEFIELD);
        d->players[p].landsPlayed++;
        Mtg_Event(d, MTG_EV_LAND, a->obj, p, 0);
        return;
    }
    if (d->stackCount >= MTG_MAX_STACK)
        return;
    if (a->kind == MTG_ACT_CAST && a->alt) {   // (its alternative cost instead of its mana: paid below)
        Mtg_MoveTo(d, a->obj, MTG_ZONE_STACK);
    } else if (a->kind == MTG_ACT_CAST) {
        MtgManaCost cost;
        Mtg_SpellCost(d, p, a->obj, &cost);
        Mtg_Pay(d, p, &cost, a->x, MTG_NO_OBJ);
        Mtg_MoveTo(d, a->obj, MTG_ZONE_STACK);
    } else {
        const MtgAbility *ab = &gMtgAbilities[a->ability];
        Mtg_Pay(d, p, &ab->cost.mana, a->x, ab->cost.tap ? a->obj : MTG_NO_OBJ);
        if (ab->cost.tap)
            Mtg_Tap(d, a->obj, 1);
    }
    e = &d->stack[d->stackCount++];
    memset(e, 0, sizeof(*e));
    e->isSpell = a->kind == MTG_ACT_CAST;
    e->controller = (uint8_t)p;
    e->source = a->obj;
    e->ability = a->ability;
    e->x = a->x;
    e->target = a->target;
    e->trigger = 0xFFFF;
    Mtg_Event(d, e->isSpell ? MTG_EV_CAST : MTG_EV_ACTIVATE, a->obj, a->target.kind, a->target.id);
    if (e->isSpell)
        TrigEvent(d, MTG_TRIG_SPELL_CAST, a->obj, MTG_NO_OBJ, p);
    if (!e->isSpell && (gMtgAbilities[a->ability].cost.sacSelf || gMtgAbilities[a->ability].cost.discardSelf))
        Mtg_MoveTo(d, a->obj, MTG_ZONE_GRAVEYARD);   // (the ability keeps its source's last known state; Cycling's discard)
    if (!e->isSpell && gMtgAbilities[a->ability].fromHand && gMtgAbilities[a->ability].cost.discardSelf)
        TrigEvent(d, MTG_TRIG_CYCLED, a->obj, MTG_NO_OBJ, p);   // ("when you cycle ...": TriggerCycled)
    if (a->kind == MTG_ACT_CAST && a->alt) {
        const MtgAltCost *alt = &gMtgAltCosts[Mtg_Card(d->objs[a->obj].def)->altCost - 1];
        PayCost(d, p, &alt->cost, a->ability, a->obj, 1);
    } else if (a->ability != 0xFFFF)
        PayOtherCosts(d, p, &gMtgAbilities[a->ability], a->obj);
}

// ---- combat ----
static int Defender(const MtgDuel *d) { return d->active ^ 1; }

static int CanAttack(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    uint32_t kw = Mtg_Keywords(d, obj);
    return IsCreature(d, obj) && o->controller == d->active && !o->tapped
        && (!o->sick || (kw & MTG_KW_BIT(MTG_KW_HASTE))) && !(kw & MTG_KW_BIT(MTG_KW_DEFENDER))
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
    if ((akw & MTG_KW_BIT(MTG_KW_FLYING)) && !(bkw & (MTG_KW_BIT(MTG_KW_FLYING) | MTG_KW_BIT(MTG_KW_REACH))))
        return 0;
    if ((akw & MTG_KW_BIT(MTG_KW_FEAR)) && !(bc->types & MTG_T_ARTIFACT) && !(bc->colors & MTG_COLOR_BIT(MTG_B)))
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
    for (i = 0; i < d->objCount; i++) {
        MtgObj *o = &d->objs[i];
        o->damage = 0;
        o->tempPower = o->tempToughness = 0;
        o->tempKeywords = 0;
        o->regen = 0;
    }
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
                TrigEvent(d, MTG_TRIG_ATTACKER_BLOCKED_BY, i, j, def);
            }
        TrigEvent(d, MTG_TRIG_ATTACKER_BLOCKED, i, first, def);
    }
}

// the step's turn-based actions; returns 1 if it is waiting for a decision (the query is set)
static int StartStep(MtgDuel *d)
{
    int i;
    d->stepStarted = 1;
    switch (d->step) {
    case MTG_STEP_UNTAP:
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].controller == d->active && d->objs[i].zone == MTG_ZONE_BATTLEFIELD) {
                if (!d->objs[i].tapped || !Mtg_StaticForbids(d, MTG_STATIC_NO_UNTAP, i, MTG_NO_OBJ))
                    Mtg_Tap(d, i, 0);
                d->objs[i].sick = 0;
            }
        break;
    case MTG_STEP_DRAW:
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
        break;
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
    d->passes = 0;
    Mtg_Event(d, MTG_EV_TRIGGER, pd.source, target.kind, target.id);
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
        if (d->stepStarted == 0 && StartStep(d))
            return;
        if (d->stepStarted == 3 || d->step == MTG_STEP_UNTAP || d->step == MTG_STEP_CLEANUP) {
            NextStep(d);   // (no priority in this step)
            continue;
        }
        StateBasedActions(d);
        if (d->over)
            continue;
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
        if (d->over)
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
            DoAction(d, &act);
            d->passes = 0;
            if (AskCost(d))
                return 1;
        }
        break;
    }
    case MTG_Q_ATTACKERS:
        for (i = 0; i < q->candidateCount; i++)
            if (a->attack & (1u << i)) {
                int o = q->candidates[i];
                d->objs[o].attacking = 1;
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
    case MTG_Q_BLOCKERS:
        for (i = 0; i < q->candidateCount; i++) {
            int b = q->candidates[i], k = a->block[i];
            if (k < 0)
                continue;
            if (k >= q->attackerCount || !Mtg_CanBlock(d, b, q->attackers[k]))
                return 0;
        }
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
        if (q->forCost) {   // a cost's sacrifice / discard
            if (a->card >= q->candidateCount)
                return 0;
            PayCostPick(d, q->candidates[a->card]);
            if (AskCost(d))
                return 1;
            break;
        }
        if (q->choice == MTG_CHOICE_PICK) {
            if (a->card >= q->candidateCount)
                return 0;
            d->resChoice = (int16_t)q->candidates[a->card];
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
