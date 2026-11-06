// Moemon Kai: a STAND-IN computer player for the Magic duels, so duels can be played to the end while the rules are
// translated. It is replaced by forge-ai's translation (AiController, AiAttackController, AiBlockController,
// ComputerUtil*, the ability AIs: docs/MTG_HANDOFF.md), under the AI levels the three card games share.
// It plays a land, casts what helps most, removes the opponent's best creature, attacks when no blocker eats the
// attacker, blocks when it kills or survives, chump-blocks only to live.
#include <string.h>
#include "mtg_internal.h"

static int Value(const MtgDuel *d, int obj)   // a creature's rough worth
{
    uint32_t kw = Mtg_Keywords(d, obj);
    int v = Mtg_Power(d, obj) * 2 + Mtg_Toughness(d, obj);
    if (kw & MTG_KW_BIT(MTG_KW_FLYING))
        v += 2;
    if (kw & (MTG_KW_BIT(MTG_KW_FIRST_STRIKE) | MTG_KW_BIT(MTG_KW_TRAMPLE)))
        v += 1;
    return v;
}

static int IsCreatureOnField(const MtgDuel *d, int obj)
{
    return d->objs[obj].zone == MTG_ZONE_BATTLEFIELD && (Mtg_Card(d->objs[obj].def)->types & MTG_T_CREATURE);
}

static int SideValue(const MtgDuel *d, int player)
{
    int i, v = 0;
    for (i = 0; i < d->objCount; i++)
        if (IsCreatureOnField(d, i) && d->objs[i].controller == player)
            v += Value(d, i);
    return v;
}

static int Amount(const MtgDuel *d, const MtgAbility *ab, const char *key, int x, int src)
{
    MtgStackEntry e;
    MtgResolveCtx ctx;
    int ok;
    memset(&e, 0, sizeof(e));
    e.x = (int16_t)x;
    ctx.e = &e;
    ctx.controller = 0;
    ctx.source = src;
    return Mtg_Amount(d, &ctx, ab->card, Mtg_Param(ab, key), &ok);
}

static int ScoreAction(const MtgDuel *d, const MtgAction *a, int me)
{
    const MtgCardDef *c = Mtg_Card(d->objs[a->obj].def);
    const MtgAbility *ab = a->ability != 0xFFFF ? &gMtgAbilities[a->ability] : NULL;
    int opp = me ^ 1, main = d->active == me && d->stackCount == 0
                                 && (d->step == MTG_STEP_MAIN1 || d->step == MTG_STEP_MAIN2);
    int tgtObj = a->target.kind == MTG_TGT_OBJ ? a->target.id : -1;
    int tgtMine = tgtObj >= 0 ? d->objs[tgtObj].controller == me : a->target.kind == MTG_TGT_PLAYER && a->target.id == me;
    if (a->kind == MTG_ACT_LAND)
        return main ? 1000 : 0;
    if (!ab) {   // a permanent spell
        if (!main)
            return 0;
        return (c->types & MTG_T_CREATURE) ? 10 + Value(d, a->obj) : 6;
    }
    switch (ab->api) {
    case MTG_API_DEAL_DAMAGE: {
        int dmg = Amount(d, ab, "NumDmg", a->x, a->obj);
        if (tgtMine)
            return 0;
        if (tgtObj >= 0)
            return Mtg_Toughness(d, tgtObj) - d->objs[tgtObj].damage <= dmg ? 20 + Value(d, tgtObj) : 0;
        if (a->target.kind == MTG_TGT_PLAYER || !ab->tgt)   // face: when it ends the duel, or at the end of their turn
            return (d->players[opp].life <= dmg) ? 500
                 : (a->kind == MTG_ACT_ACTIVATE && d->active == opp && d->step == MTG_STEP_END) ? 5 : 0;
        return 0;
    }
    case MTG_API_DESTROY:
    case MTG_API_CHANGE_ZONE:
        if (tgtObj < 0 || tgtMine || !IsCreatureOnField(d, tgtObj))
            return 0;
        return 20 + Value(d, tgtObj);
    case MTG_API_TAP:
        return 0;
    case MTG_API_DESTROY_ALL:
    case MTG_API_DAMAGE_ALL:
        return main && SideValue(d, opp) > SideValue(d, me) + 6 ? 30 + SideValue(d, opp) - SideValue(d, me) : 0;
    case MTG_API_PUMP:   // on its own attacker or blocker in the damage-to-come window
        if (tgtObj >= 0 && !tgtMine)
            return 0;
        if (d->step == MTG_STEP_COMBAT_BLOCKERS && tgtObj >= 0
            && (d->objs[tgtObj].attacking || d->objs[tgtObj].blocking != MTG_NO_OBJ))
            return 8;
        return 0;
    case MTG_API_PUMP_ALL:
        return d->step == MTG_STEP_COMBAT_BLOCKERS && d->active == me ? 8 : 0;
    case MTG_API_DRAW:
        return main && !tgtMine && a->target.kind == MTG_TGT_PLAYER ? 0 : (main ? 7 : 0);
    case MTG_API_GAIN_LIFE:
        return main && (a->target.kind != MTG_TGT_PLAYER || tgtMine) ? 3 : 0;
    case MTG_API_LOSE_LIFE:
    case MTG_API_DISCARD:
        return main && (a->target.kind == MTG_TGT_PLAYER ? !tgtMine : 1) ? 6 : 0;
    case MTG_API_COUNTER:
        if (d->stackCount && d->stack[d->stackCount - 1].controller == opp && a->target.id == d->stack[d->stackCount - 1].source)
            return Mtg_ManaCostTotal(&Mtg_Card(d->objs[a->target.id].def)->cost) >= 3 ? 40 : 0;
        return 0;
    case MTG_API_UNTAP:
    case MTG_API_MANA:
        return 0;
    case MTG_API_ATTACH: {   // an Aura: Forge's AttachAILogic says whether it helps (Pump) or hurts (Curse)
        const char *logic = Mtg_Svar(ab->card, "AttachAILogic");
        int curse = logic && strcmp(logic, "Curse") == 0;
        if (!main || tgtObj < 0 || !IsCreatureOnField(d, tgtObj) || curse != !tgtMine)
            return 0;
        return curse ? 15 + Value(d, tgtObj) : 9 + Value(d, tgtObj) / 2;
    }
    }
    return 0;
}

static void Priority(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    int i, best = 0, bestScore = 0;
    for (i = 1; i < q->actionCount; i++) {
        int s = ScoreAction(d, &q->actions[i], q->player);
        if (s > bestScore) {
            bestScore = s;
            best = i;
        }
    }
    out->action = (uint16_t)best;
}

static void Attackers(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    int i, j, opp = q->player ^ 1, total = 0;
    for (i = 0; i < q->candidateCount; i++)
        total += Mtg_Power(d, q->candidates[i]);
    out->attack = 0;
    for (i = 0; i < q->candidateCount; i++) {
        int a = q->candidates[i], safe = 1;
        if (total >= d->players[opp].life) {   // everything at once ends it
            out->attack |= 1u << i;
            continue;
        }
        for (j = 0; j < d->objCount && safe; j++)
            if (IsCreatureOnField(d, j) && d->objs[j].controller == opp && !d->objs[j].tapped && Mtg_CanBlock(d, j, a)
                && Mtg_Power(d, j) >= Mtg_Toughness(d, a) && Mtg_Power(d, a) < Mtg_Toughness(d, j))
                safe = 0;   // a blocker kills it and lives
        if (safe && Mtg_Power(d, a) > 0)
            out->attack |= 1u << i;
    }
}

static void Blockers(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    uint8_t used[MTG_MAX_COMBAT] = {0};
    int i, j, incoming = 0, me = q->player;
    for (i = 0; i < MTG_MAX_COMBAT; i++)
        out->block[i] = -1;
    for (i = 0; i < q->attackerCount; i++)
        incoming += Mtg_Power(d, q->attackers[i]);
    for (i = 0; i < q->attackerCount; i++) {
        int a = q->attackers[i], pick = -1, pickKind = 0;
        for (j = 0; j < q->candidateCount; j++) {
            int b = q->candidates[j], kills, lives, kind;
            if (used[j] || !Mtg_CanBlock(d, b, a))
                continue;
            kills = Mtg_Power(d, b) >= Mtg_Toughness(d, a);
            lives = Mtg_Power(d, a) < Mtg_Toughness(d, b);
            kind = kills && lives ? 3 : lives ? 2 : kills && Value(d, a) >= Value(d, b) ? 1 : 0;
            if (kind > pickKind) {
                pickKind = kind;
                pick = j;
            }
        }
        if (pick < 0 && incoming >= d->players[me].life) {   // chump to live: the smallest that can
            for (j = 0; j < q->candidateCount; j++)
                if (!used[j] && Mtg_CanBlock(d, q->candidates[j], a)
                    && (pick < 0 || Value(d, q->candidates[j]) < Value(d, q->candidates[pick])))
                    pick = j;
        }
        if (pick >= 0) {
            used[pick] = 1;
            out->block[pick] = (int8_t)i;
            incoming -= Mtg_Power(d, a);
        }
    }
}

static void Discard(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    int i, lands = 0, best = 0, bestCost = -1;
    for (i = 0; i < q->candidateCount; i++)
        lands += (Mtg_Card(d->objs[q->candidates[i]].def)->types & MTG_T_LAND) != 0;
    for (i = 0; i < q->candidateCount; i++) {
        const MtgCardDef *c = Mtg_Card(d->objs[q->candidates[i]].def);
        int cost = (c->types & MTG_T_LAND) ? (lands > 3 ? 99 : -1) : Mtg_ManaCostTotal(&c->cost);
        if (cost > bestCost) {
            bestCost = cost;
            best = i;
        }
    }
    out->card = (uint8_t)best;
}

// a triggered ability's target: the opponent's best for what harms, its own best for what helps
static int Helps(const MtgAbility *ab)
{
    const char *v;
    switch (ab->api) {
    case MTG_API_PUMP: case MTG_API_PUMP_ALL: case MTG_API_GAIN_LIFE: case MTG_API_DRAW: case MTG_API_UNTAP:
        return !Mtg_Param(ab, "IsCurse");
    case MTG_API_PUT_COUNTER:
        return (v = Mtg_Param(ab, "CounterType")) && strcmp(v, "P1P1") == 0;
    case MTG_API_CHANGE_ZONE:
        return (v = Mtg_Param(ab, "Origin")) && strcmp(v, "Graveyard") == 0
            && (v = Mtg_Param(ab, "Destination")) && strcmp(v, "Exile") != 0;
    }
    return 0;
}

static void TriggerTarget(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    const MtgAbility *ab = &gMtgAbilities[q->ability];
    int i, best = 0, bestScore = -1000, helps = Helps(ab);
    for (i = 0; i < q->actionCount; i++) {
        const MtgTarget *t = &q->actions[i].target;
        int mine, worth, s;
        if (t->kind == MTG_TGT_PLAYER) {
            mine = t->id == q->player;
            worth = 5;
        } else {
            const MtgObj *o = &d->objs[t->id];
            mine = (o->zone == MTG_ZONE_BATTLEFIELD ? o->controller : o->owner) == q->player;
            worth = (Mtg_Card(o->def)->types & MTG_T_CREATURE) ? Mtg_Card(o->def)->power * 2 + Mtg_Card(o->def)->toughness
                                                                : Mtg_ManaCostTotal(&Mtg_Card(o->def)->cost);
        }
        s = (mine == helps ? 100 : 0) + (mine == helps ? worth : -worth);
        if (s > bestScore) {
            bestScore = s;
            best = i;
        }
    }
    out->action = (uint16_t)best;
}

// a choice while resolving: "you may" yes, pay to keep its own things or to save its spell, the least to lose
static void Choose(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    int i, best = 0, bestV = 1 << 30;
    if (q->choice != MTG_CHOICE_PICK) {
        out->card = 1;
        return;
    }
    for (i = 0; i < q->candidateCount; i++) {
        const MtgObj *o = &d->objs[q->candidates[i]];
        const MtgCardDef *c = Mtg_Card(o->def);
        int v = o->zone == MTG_ZONE_BATTLEFIELD && (c->types & MTG_T_CREATURE) ? Value(d, q->candidates[i])
              : Mtg_ManaCostTotal(&c->cost) + ((c->types & MTG_T_LAND) ? 2 : 0);
        if (v < bestV) {
            bestV = v;
            best = i;
        }
    }
    out->card = (uint8_t)best;
}

void MtgAi_Answer(const MtgDuel *d, MtgAnswer *out)
{
    const MtgQuery *q = Mtg_Query(d);
    memset(out, 0, sizeof(*out));
    switch (q->kind) {
    case MTG_Q_PRIORITY:
        Priority(d, q, out);
        break;
    case MTG_Q_ATTACKERS:
        Attackers(d, q, out);
        break;
    case MTG_Q_BLOCKERS:
        Blockers(d, q, out);
        break;
    case MTG_Q_DISCARD:
        Discard(d, q, out);
        break;
    case MTG_Q_TRIGGER_TARGET:
        TriggerTarget(d, q, out);
        break;
    case MTG_Q_CHOOSE:
        Choose(d, q, out);
        break;
    }
}
