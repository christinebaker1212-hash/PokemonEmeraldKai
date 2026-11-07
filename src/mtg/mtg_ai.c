// Moemon Kai: a STAND-IN computer player for the Magic duels, so duels can be played to the end while the rules are
// translated. It is replaced by forge-ai's translation (AiController, AiAttackController, AiBlockController,
// ComputerUtil*, the ability AIs: docs/MTG_HANDOFF.md), under the AI levels the three card games share.
// It plays a land, casts what helps most, removes the opponent's best creature, attacks when no blocker eats the
// attacker, blocks when it kills or survives, chump-blocks only to live.
#include <string.h>
#include "mtg_internal.h"

static int Helps(const MtgAbility *ab);
#define HelpsAb Helps

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

// ---- a look-ahead for what the scores below don't know (RepeatEach, SetLife, a Reveal's chain...): the action played
// on a copy of the duel, both passing until it has resolved (this AI answering what it asks), the board compared
static int sSimulating;   // (inside a look-ahead: no look-ahead of its own)

static int Eval(const MtgDuel *d, int me)
{
    int i, v = (d->players[me].life - d->players[me ^ 1].life) * 2;
    if (d->over)
        return d->winner == me ? 10000 : -10000;
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        int worth;
        if (o->zone == MTG_ZONE_HAND)
            v += o->owner == me ? 2 : -2;
        if (o->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        worth = (Mtg_ObjTypes(d, i) & MTG_T_CREATURE) ? Value(d, i) + 2
              : (Mtg_ObjTypes(d, i) & MTG_T_LAND) ? 3 : Mtg_ManaCostTotal(&Mtg_Card(o->def)->cost) + 1;
        v += o->controller == me ? worth : -worth;
    }
    return v;
}

void MtgAi_Answer(const MtgDuel *d, MtgAnswer *out);

static int Lookahead(const MtgDuel *d, const MtgAction *a, int me)
{
    static MtgDuel copy;
    MtgAnswer ans;
    int idx = (int)(a - d->query.actions), base = d->stackCount, guard = 0, before;
    if (sSimulating || idx < 0 || idx >= d->query.actionCount)
        return 0;
    sSimulating = 1;
    copy = *d;
    before = Eval(d, me);
    memset(&ans, 0, sizeof(ans));
    ans.action = (uint16_t)idx;
    if (Mtg_Answer(&copy, &ans))
        while (guard++ < 300 && !copy.over) {
            const MtgQuery *q = Mtg_Query(&copy);
            if (q->kind == MTG_Q_PRIORITY && !copy.resolving && copy.stackCount <= base)
                break;   // (it has resolved)
            memset(&ans, 0, sizeof(ans));
            if (q->kind != MTG_Q_PRIORITY)
                MtgAi_Answer(&copy, &ans);   // (its choices, as this AI would make them; priority: pass)
            if (!Mtg_Answer(&copy, &ans))
                break;
        }
    sSimulating = 0;
    return Eval(&copy, me) - before;
}

static int IsCreatureOnField(const MtgDuel *d, int obj)
{
    return d->objs[obj].zone == MTG_ZONE_BATTLEFIELD && (Mtg_ObjTypes(d, obj) & MTG_T_CREATURE);
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

static int ScoreBase(const MtgDuel *d, const MtgAction *a, int me);

// a creature of mine that is about to be destroyed: lethal damage coming in combat, or the opponent's destroy /
// damage on the stack aimed at it (for regeneration)
static int AboutToDie(const MtgDuel *d, int obj, int me)
{
    int i, left;
    if (obj < 0 || !IsCreatureOnField(d, obj) || d->objs[obj].controller != me || d->objs[obj].regen)
        return 0;
    left = Mtg_Toughness(d, obj) - d->objs[obj].damage;
    if (d->stackCount) {
        const MtgStackEntry *e = &d->stack[d->stackCount - 1];
        const MtgAbility *ab = e->ability != 0xFFFF ? &gMtgAbilities[e->ability] : NULL;
        if (ab && e->controller != me && e->target.kind == MTG_TGT_OBJ && e->target.id == obj
            && (ab->api == MTG_API_DESTROY || ab->api == MTG_API_DEAL_DAMAGE) && !Mtg_Param(ab, "NoRegen"))
            return 1;
        return 0;
    }
    if (d->step != MTG_STEP_COMBAT_BLOCKERS)
        return 0;
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        if (!IsCreatureOnField(d, i) || o->controller == me)
            continue;
        if ((o->blocking == obj && d->objs[obj].attacking) || (d->objs[obj].blocking == i && o->attacking))
            left -= Mtg_Power(d, i);
    }
    return left <= 0;
}

// what an action's other costs give up: the cheapest permanents to sacrifice, cards from the hand, life
static int CostPriceOf(const MtgDuel *d, const MtgCost *c, int me, int src);
static int CostPrice(const MtgDuel *d, const MtgAbility *ab, int me, int src)
{
    return CostPriceOf(d, &ab->cost, me, src);
}

static int CostPriceOf(const MtgDuel *d, const MtgCost *c, int me, int src)
{
    int price = 0, k;
    if (c->sacN) {
        uint16_t list[MTG_MAX_COMBAT];
        int n = Mtg_CostCandidates(d, me, c, 1, src, list), used[MTG_MAX_COMBAT] = {0};
        if (n > MTG_MAX_COMBAT)
            n = MTG_MAX_COMBAT;
        for (k = 0; k < c->sacN; k++) {   // (the cheapest ones, as Choose picks them)
            int i, best = -1, bestV = 1 << 30;
            for (i = 0; i < n; i++) {
                const MtgCardDef *cd = Mtg_Card(d->objs[list[i]].def);
                int v = (cd->types & MTG_T_CREATURE) ? Value(d, list[i]) : (cd->types & MTG_T_LAND) ? 5 : 4;
                if (!used[i] && v < bestV)
                    bestV = v, best = i;
            }
            if (best >= 0)
                used[best] = 1, price += bestV;
        }
    }
    price += c->discardN * 4;
    price += c->exileHandN * 5 + c->returnN * 3 + c->tapTypeN * 2 + c->exileGraveN;   // (a card gone for good; a land back; a creature tapped)
    if (c->payLife)
        price += d->players[me].life <= c->payLife + 5 ? 50 : c->payLife;
    return price;
}

static int ScoreAction(const MtgDuel *d, const MtgAction *a, int me)
{
    int s = ScoreBase(d, a, me);
    if (s > 0 && s < 500 && a->ability != 0xFFFF && !a->alt) {
        int price = CostPrice(d, &gMtgAbilities[a->ability], me, a->obj);
        s = s > price ? s - price : 0;
    }
    if (s > 0 && s < 500 && a->alt) {   // the alternative cost's price: the mana cast wins when both are there
        int price = 1 + CostPriceOf(d, &gMtgAltCosts[Mtg_Card(d->objs[a->obj].def)->altCost - 1].cost, me, a->obj);
        s = s > price ? s - price : 0;
    }
    return s;
}

static int ScoreBase(const MtgDuel *d, const MtgAction *a, int me)
{
    const MtgCardDef *c = Mtg_Card(d->objs[a->obj].def);
    const MtgAbility *ab = a->ability != 0xFFFF ? &gMtgAbilities[a->ability] : NULL;
    int opp = me ^ 1, main = d->active == me && d->stackCount == 0
                                 && (d->step == MTG_STEP_MAIN1 || d->step == MTG_STEP_MAIN2);
    int tgtObj = a->target.kind == MTG_TGT_OBJ ? a->target.id : -1;
    int tgtMine = tgtObj >= 0 ? d->objs[tgtObj].controller == me : a->target.kind == MTG_TGT_PLAYER && a->target.id == me;
    if (ab && ab->tgtFromSub) {   // (the target is a later ability's: weighed as that one: Scent of Ivy's Pump)
        uint16_t k;
        for (k = ab->sub; k != 0xFFFF; k = gMtgAbilities[k].sub)
            if (gMtgAbilities[k].tgtShared) {
                ab = &gMtgAbilities[k];
                break;
            }
    }
    if (a->kind == MTG_ACT_LAND)
        return main ? 1000 : 0;
    if (ab && ab->fromHand) {
        // Cycling: a land when there are enough, a spell too dear for a while; best at the end of the other's turn
        int k, lands = 0, good;
        for (k = 0; k < d->objCount; k++)
            lands += d->objs[k].zone == MTG_ZONE_BATTLEFIELD && d->objs[k].controller == me
                  && (Mtg_Card(d->objs[k].def)->types & MTG_T_LAND);
        good = (c->types & MTG_T_LAND) ? lands >= 5 : Mtg_ManaCostTotal(&c->cost) > lands + 2;
        if (!good)
            return 0;
        return (d->active == opp && d->step == MTG_STEP_END) ? 7 : main ? 4 : 0;
    }
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
    case MTG_API_CHANGE_ZONE: {
        const char *o = Mtg_Param(ab, "Origin"), *dz = Mtg_Param(ab, "Destination");
        if (Mtg_Param(ab, "ChangeType"))   // a search: worth it in its main phase (a land one early)
            return main ? (dz && strcmp(dz, "Battlefield") == 0 ? 14 : 11) : 0;
        if (o && strcmp(o, "Graveyard") == 0 && tgtObj >= 0 && tgtMine && dz && strcmp(dz, "Exile") != 0)
            return main ? 8 + Mtg_ManaCostTotal(&Mtg_Card(d->objs[tgtObj].def)->cost) : 0;   // (its own card back)
        if (tgtObj < 0 || tgtMine || !IsCreatureOnField(d, tgtObj))
            return 0;
        return 20 + Value(d, tgtObj);
    }
    case MTG_API_TAP:
        return 0;
    case MTG_API_ANIMATE: {   // a land of its own made a creature: before its attack, on its own turn
        const char *def = Mtg_Param(ab, "Defined");
        if (!def || strcmp(def, "Self") || d->objs[a->obj].animUntil || d->active != me || d->step != MTG_STEP_MAIN1
            || d->objs[a->obj].tapped || !Mtg_Param(ab, "Power"))
            return 0;
        return 5;
    }
    case MTG_API_CHANGE_ZONE_ALL: {   // off the battlefield: as DestroyAll; a graveyard's: a little, on its own turn
        const char *o = Mtg_Param(ab, "Origin");
        if (o && strstr(o, "Battlefield"))
            return main && SideValue(d, opp) > SideValue(d, me) + 6 ? 25 + SideValue(d, opp) - SideValue(d, me) : 0;
        return main ? 3 : 0;
    }
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
    case MTG_API_TOKEN:   // creatures for its side, in a main phase (or at the end of the other's turn, activated)
        if (Mtg_Param(ab, "TokenOwner") && strcmp(Mtg_Param(ab, "TokenOwner"), "You") != 0)
            return 0;
        return main ? 9 : (a->kind == MTG_ACT_ACTIVATE && d->active == opp && d->step == MTG_STEP_END) ? 6 : 0;
    case MTG_API_GAIN_CONTROL: {   // their best thing: for good, or as long as its source lasts; Threaten on its own turn
        const char *nc = Mtg_Param(ab, "NewController"), *lc = Mtg_Param(ab, "LoseControl");
        if (a->target.kind == MTG_TGT_PLAYER && !tgtMine)   // (giving its own curse away: Jinxed Idol)
            return main ? 6 : 0;
        if ((nc && strcmp(nc, "You")) || tgtObj < 0 || tgtMine || !main)
            return 0;
        if (lc && strstr(lc, "EOT"))   // (until end of turn: only to attack with it, before combat)
            return d->active == me && d->step == MTG_STEP_MAIN1 && IsCreatureOnField(d, tgtObj)
                && (!d->objs[tgtObj].tapped || Mtg_Param(ab, "Untap")) ? 8 + Value(d, tgtObj) / 2 : 0;
        return IsCreatureOnField(d, tgtObj) ? 20 + Value(d, tgtObj) : 12 + Mtg_ManaCostTotal(&Mtg_Card(d->objs[tgtObj].def)->cost);
    }
    case MTG_API_MILL:   // the opponent's library into their graveyard; its own only as part of more (MENTAL NOTE)
        if (a->target.kind == MTG_TGT_PLAYER)
            return main && !tgtMine ? 3 : 0;
        return main && ab->sub != 0xFFFF && (!Mtg_Param(ab, "Defined") || !strcmp(Mtg_Param(ab, "Defined"), "You")) ? 4 : 0;
    case MTG_API_FLIP_COIN: {   // a gamble: its targets where the winning side wants them
        const MtgAbility *w = ab->winSub != 0xFFFF ? &gMtgAbilities[ab->winSub] : NULL;
        if (tgtObj >= 0 && (w && HelpsAb(w)) != tgtMine)
            return 0;
        if (a->target.kind == MTG_TGT_PLAYER && tgtMine)
            return 0;
        return main ? 4 : 0;
    }
    case MTG_API_EFFECT: {   // rules for a while: a curse on theirs, a help on its own before combat
        int curse = Mtg_Param(ab, "IsCurse") != NULL;
        if (a->target.kind == MTG_TGT_PLAYER)   // (Cease-Fire, Abeyance: at the other player)
            return main && !tgtMine ? 4 : 0;
        if (tgtObj >= 0)
            return curse ? (main && !tgtMine ? 4 : 0)
                         : (d->active == me && d->step == MTG_STEP_MAIN1 && tgtMine && IsCreatureOnField(d, tgtObj)
                            && !d->objs[tgtObj].tapped ? 5 : 0);
        return d->active == me && d->step == MTG_STEP_MAIN1 ? 4 : 0;   // (Falter: before its attack)
    }
    case MTG_API_DIG:   // looking for cards: in its own main phase (a target player's: the other's)
        return main && (a->target.kind != MTG_TGT_PLAYER || tgtMine) ? 7 : 0;
    case MTG_API_PUT_COUNTER: {   // counters that grow its own creature, curse counters on theirs, charges at their end step
        int kind = Mtg_CounterKind(Mtg_Param(ab, "CounterType")), pp, tt;
        Mtg_CounterPT(kind, &pp, &tt);
        if (kind == MTG_CTR_P1P1)
            pp = tt = 1;
        else if (kind == MTG_CTR_M1M1)
            pp = tt = -1;
        if (Mtg_Param(ab, "IsCurse") || pp + tt < 0)
            return main && tgtObj >= 0 && !tgtMine ? 5 + Value(d, tgtObj) / 3 : 0;
        if (pp + tt > 0)
            return main && (tgtObj < 0 || tgtMine) ? 6 : 0;
        return a->kind == MTG_ACT_ACTIVATE && d->active == opp && d->step == MTG_STEP_END ? 4 : 0;
    }
    case MTG_API_PREVENT_DAMAGE:   // its own creature about to die, or itself low on life in their attack
        if (tgtObj >= 0)
            return tgtMine && AboutToDie(d, tgtObj, me) && !d->objs[tgtObj].prevent ? 22 + Value(d, tgtObj) : 0;
        if (a->target.kind == MTG_TGT_PLAYER && tgtMine && d->active == opp && d->step == MTG_STEP_COMBAT_BLOCKERS
            && d->players[me].life <= 6 && !d->players[me].prevent)
            return 12;
        return 0;
    case MTG_API_REGENERATE: {   // only when it would die now
        const char *def = Mtg_Param(ab, "Defined");
        int who = tgtObj >= 0 ? tgtObj : def && strcmp(def, "Enchanted") == 0 ? d->objs[a->obj].attachedTo : a->obj;
        if (who == MTG_NO_OBJ)
            return 0;
        return AboutToDie(d, who, me) ? 25 + Value(d, who) : 0;
    }
    case MTG_API_ATTACH: {   // an Aura: Forge's AttachAILogic says whether it helps (Pump) or hurts (Curse)
        const char *logic = Mtg_Svar(ab->card, "AttachAILogic");
        int curse = logic && (strcmp(logic, "Curse") == 0 || strcmp(logic, "GainControl") == 0);
        if (!main || tgtObj < 0 || curse != !tgtMine)
            return 0;
        if (logic && strcmp(logic, "GainControl") == 0)   // (Control Magic: the best thing they have becomes ours)
            return 20 + Value(d, tgtObj);
        if (!IsCreatureOnField(d, tgtObj))   // (a land's or an artifact's Aura)
            return 6;
        return curse ? 15 + Value(d, tgtObj) : 9 + Value(d, tgtObj) / 2;
    }
    }
    if (main && ab) {   // (a kind the scores above don't know: tried on a copy, played when it comes out ahead)
        int gain = Lookahead(d, a, me);
        if (gain > 2)
            return 3 + (gain > 30 ? 30 : gain);
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
    case MTG_API_PREVENT_DAMAGE:
        return 1;
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
            worth = (Mtg_ObjTypes(d, t->id) & MTG_T_CREATURE) ? Mtg_Card(o->def)->power * 2 + Mtg_Card(o->def)->toughness
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

// a ChangeType$ choice among its own permanents, or its own graveyard's cards to somewhere but the battlefield: what it
// gives up (the least to lose, below), not a find
static int GivesUpOwn(const MtgAbility *ab)
{
    const char *o = Mtg_Param(ab, "Origin"), *dz = Mtg_Param(ab, "Destination");
    return o && (strcmp(o, "Battlefield") == 0 || (strcmp(o, "Graveyard") == 0 && (!dz || strcmp(dz, "Battlefield"))));
}

// a choice while resolving: "you may" yes, pay to keep its own things or to save its spell, the least to lose
static void Choose(const MtgDuel *d, const MtgQuery *q, MtgAnswer *out)
{
    int i, best = 0, bestV = 1 << 30;
    if (q->choice == MTG_CHOICE_PAY && q->ability != 0xFFFF && gMtgAbilities[q->ability].unless) {
        // a cost that isn't only mana: paid unless it costs too much life or too many permanents
        const MtgCost *c = &gMtgUnlessCosts[gMtgAbilities[q->ability].unless - 1];
        int life = d->players[q->player].life - c->payLife - c->damageYou;
        out->card = (uint8_t)(life > 5 && c->sacN < 2 && c->returnN < 3);
        return;
    }
    if (q->choice != MTG_CHOICE_PICK) {
        out->card = 1;
        return;
    }
    if (q->ability != 0xFFFF && ((gMtgAbilities[q->ability].api == MTG_API_CHANGE_ZONE
                                  && Mtg_Param(&gMtgAbilities[q->ability], "ChangeType") && !GivesUpOwn(&gMtgAbilities[q->ability]))
                                 || gMtgAbilities[q->ability].api == MTG_API_DIG)) {   // a search or a dig: the most useful find
        int bestS = -1;
        for (i = 0; i < q->candidateCount; i++) {
            const MtgCardDef *c = Mtg_Card(d->objs[q->candidates[i]].def);
            int sc = Mtg_ManaCostTotal(&c->cost) * 2 + ((c->types & MTG_T_CREATURE) ? c->power + c->toughness : 0), k;
            if (c->types & MTG_T_LAND) {   // (a land: the colors its hand asks for)
                sc = 1;
                for (k = 0; k < d->objCount; k++) {
                    const MtgObj *h = &d->objs[k];
                    int col;
                    if (h->zone != MTG_ZONE_HAND || h->owner != q->player)
                        continue;
                    for (col = 0; col < 5; col++)
                        if ((c->intrinsicMana & MTG_COLOR_BIT(col)) && Mtg_Card(h->def)->cost.pip[col])
                            sc += 2;
                }
            }
            if (sc > bestS)
                bestS = sc, best = i;
        }
        out->card = (uint8_t)best;
        return;
    }
    if (q->forCost == 7) {   // another target: the opponent's best for what harms, its own best for what helps
        int helps = Helps(&gMtgAbilities[q->ability]), bestS = -1000;
        for (i = 0; i < q->candidateCount; i++) {
            const MtgObj *o = &d->objs[q->candidates[i]];
            int mine = (o->zone == MTG_ZONE_BATTLEFIELD ? o->controller : o->owner) == q->player;
            int worth = o->zone == MTG_ZONE_BATTLEFIELD && (Mtg_ObjTypes(d, q->candidates[i]) & MTG_T_CREATURE)
                      ? Value(d, q->candidates[i]) : Mtg_ManaCostTotal(&Mtg_Card(o->def)->cost);
            int sc = mine == helps ? 100 + worth : -worth;
            if (sc > bestS)
                bestS = sc, best = i;
        }
        out->card = (uint8_t)(bestS < 100 && !d->tgtLeft ? q->candidateCount : best);   // (only the wrong side left: stop)
        return;
    }
    for (i = 0; i < q->candidateCount; i++) {
        const MtgObj *o = &d->objs[q->candidates[i]];
        const MtgCardDef *c = Mtg_Card(o->def);
        int v = o->zone == MTG_ZONE_BATTLEFIELD && (Mtg_ObjTypes(d, q->candidates[i]) & MTG_T_CREATURE) ? Value(d, q->candidates[i])
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
