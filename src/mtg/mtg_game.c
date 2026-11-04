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

int Mtg_Power(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    return Mtg_Card(o->def)->power + o->counters11 + o->tempPower;
}

int Mtg_Toughness(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    return Mtg_Card(o->def)->toughness + o->counters11 + o->tempToughness;
}

uint32_t Mtg_Keywords(const MtgDuel *d, int obj)
{
    return Mtg_Card(d->objs[obj].def)->keywords | d->objs[obj].tempKeywords;
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
}

// GameAction.changeZone: a card moves (to the library's top when it goes there); a permanent leaving forgets its state
void Mtg_MoveTo(MtgDuel *d, int obj, int zone)
{
    MtgObj *o = &d->objs[obj];
    int from = o->zone, i;
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
        if (sourceObj != MTG_NO_OBJ && (Mtg_Card(d->objs[to.id].def)->protection & Mtg_ObjColors(d, sourceObj)))
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
    (void)combat;
    if (sourceObj != MTG_NO_OBJ && (Mtg_Keywords(d, sourceObj) & MTG_KW_BIT(MTG_KW_LIFELINK)))
        Mtg_GainLife(d, d->objs[sourceObj].controller, amount);
}

void Mtg_Destroy(MtgDuel *d, int obj)
{
    if (d->objs[obj].zone == MTG_ZONE_BATTLEFIELD)
        Mtg_MoveTo(d, obj, MTG_ZONE_GRAVEYARD);   // (regeneration and indestructible: not yet)
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
        for (i = 0; i < d->objCount; i++) {
            int t;
            if (!IsCreature(d, i))
                continue;
            t = Mtg_Toughness(d, i);
            if (t <= 0) {
                Mtg_MoveTo(d, i, MTG_ZONE_GRAVEYARD);
                again = 1;
            } else if (d->objs[i].damage >= t) {
                Mtg_Destroy(d, i);
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
    return d->objs[e->target.id].zone == MTG_ZONE_BATTLEFIELD
        && Mtg_ValidObj(d, e->target.id, ab->tgt, e->source, e->controller)
        && Mtg_CanTarget(d, e->target.id, e->source);
}

static int MainAbility(const MtgCardDef *c)
{
    int i;
    for (i = 0; i < c->abilityCount; i++)
        if (gMtgAbilities[c->abilities[i]].kind == MTG_AB_SPELL)
            return c->abilities[i];
    return 0xFFFF;
}

static void ResolveTop(MtgDuel *d)
{
    MtgStackEntry e = d->stack[--d->stackCount];
    const MtgCardDef *c = Mtg_Card(d->objs[e.source].def);
    const MtgAbility *ab = e.ability != 0xFFFF ? &gMtgAbilities[e.ability] : NULL;
    if (ab && ab->tgt && !TargetStillLegal(d, &e, ab)) {
        Mtg_Event(d, MTG_EV_FIZZLE, e.source, e.ability, 0);
        if (e.isSpell)
            Mtg_MoveTo(d, e.source, MTG_ZONE_GRAVEYARD);
        return;
    }
    if (e.target.kind == MTG_TGT_OBJ || e.target.kind == MTG_TGT_STACK) {   // last known information
        e.lkiPower = (int16_t)Mtg_Power(d, e.target.id);
        e.lkiToughness = (int16_t)Mtg_Toughness(d, e.target.id);
        e.lkiController = d->objs[e.target.id].controller;
    }
    Mtg_Event(d, MTG_EV_RESOLVE, e.source, e.ability, e.isSpell);
    if (ab) {
        MtgResolveCtx ctx;
        ctx.e = &e;
        ctx.controller = e.controller;
        ctx.source = e.source;
        Mtg_ResolveAbility(d, &ctx, e.ability);
    }
    if (e.isSpell && d->objs[e.source].zone == MTG_ZONE_STACK) {
        if (c->types & (MTG_T_INSTANT | MTG_T_SORCERY))
            Mtg_MoveTo(d, e.source, MTG_ZONE_GRAVEYARD);
        else {
            Mtg_MoveTo(d, e.source, MTG_ZONE_BATTLEFIELD);
            d->objs[e.source].controller = e.controller;
            d->objs[e.source].sick = 1;
        }
    }
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
            if (c->types & MTG_T_LAND) {
                if (SorceryTiming(d, p) && d->players[p].landsPlayed == 0)
                    AddAction(q, MTG_ACT_LAND, i, 0xFFFF, 0, none);
                continue;
            }
            if (!(c->types & MTG_T_INSTANT) && !(c->keywords & MTG_KW_BIT(MTG_KW_FLASH)) && !SorceryTiming(d, p))
                continue;
            if (c->cost.x) {   // (X: the most that can be paid; a player's own pick is the UI's to come)
                x = Mtg_MaxX(d, p, &c->cost, MTG_NO_OBJ);
                if (x == 0)
                    continue;
            }
            if (!Mtg_CanPay(d, p, &c->cost, x, MTG_NO_OBJ))
                continue;
            ability = MainAbility(c);
            AddWithTargets(d, q, MTG_ACT_CAST, i, ability, x, p);
        } else if (o->zone == MTG_ZONE_BATTLEFIELD && o->controller == p) {
            for (k = 0; k < c->abilityCount; k++) {
                const MtgAbility *ab = &gMtgAbilities[c->abilities[k]];
                int x = 0;
                if (ab->kind != MTG_AB_ACTIVATED || ab->api == MTG_API_MANA)
                    continue;   // (mana abilities are used by paying, never on their own)
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
                AddWithTargets(d, q, MTG_ACT_ACTIVATE, i, c->abilities[k], x, p);
            }
        }
    }
}

static void DoAction(MtgDuel *d, const MtgAction *a)
{
    int p = d->priority;
    MtgStackEntry *e;
    MtgObj *o = &d->objs[a->obj];
    const MtgCardDef *c = Mtg_Card(o->def);
    if (a->kind == MTG_ACT_LAND) {
        Mtg_MoveTo(d, a->obj, MTG_ZONE_BATTLEFIELD);
        d->players[p].landsPlayed++;
        Mtg_Event(d, MTG_EV_LAND, a->obj, p, 0);
        return;
    }
    if (d->stackCount >= MTG_MAX_STACK)
        return;
    if (a->kind == MTG_ACT_CAST) {
        Mtg_Pay(d, p, &c->cost, a->x, MTG_NO_OBJ);
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
    Mtg_Event(d, e->isSpell ? MTG_EV_CAST : MTG_EV_ACTIVATE, a->obj, a->target.kind, a->target.id);
    if (!e->isSpell && gMtgAbilities[a->ability].cost.sacSelf)
        Mtg_MoveTo(d, a->obj, MTG_ZONE_GRAVEYARD);   // (the ability keeps its source's last known state)
}

// ---- combat ----
static int Defender(const MtgDuel *d) { return d->active ^ 1; }

static int CanAttack(const MtgDuel *d, int obj)
{
    const MtgObj *o = &d->objs[obj];
    uint32_t kw = Mtg_Keywords(d, obj);
    return IsCreature(d, obj) && o->controller == d->active && !o->tapped
        && (!o->sick || (kw & MTG_KW_BIT(MTG_KW_HASTE))) && !(kw & MTG_KW_BIT(MTG_KW_DEFENDER));
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
    if (ac->protection & bc->colors)
        return 0;
    for (i = 0; i < 5; i++)
        if ((ac->landwalk & MTG_COLOR_BIT(i)) && ControlsLandType(d, d->objs[blocker].controller, i))
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

// the step's turn-based actions; returns 1 if it is waiting for a decision (the query is set)
static int StartStep(MtgDuel *d)
{
    int i;
    d->stepStarted = 1;
    switch (d->step) {
    case MTG_STEP_UNTAP:
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].controller == d->active && d->objs[i].zone == MTG_ZONE_BATTLEFIELD) {
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
                    ResolveTop(d);
                    d->priority = d->active;
                } else
                    NextStep(d);
            } else
                d->priority ^= 1;
        } else {
            DoAction(d, &act);
            d->passes = 0;
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
        d->stepStarted = 1;
        break;
    case MTG_Q_DISCARD:
        if (a->card >= q->candidateCount)
            return 0;
        Mtg_MoveTo(d, q->candidates[a->card], MTG_ZONE_GRAVEYARD);
        d->stepStarted = 0;   // (again: still too many, or the rest of the cleanup)
        break;
    default:
        return 0;
    }
    Run(d);
    return 1;
}
