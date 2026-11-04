// Moemon Kai: the effects translated so far, one per Forge class in forge/game/ability/effects (DamageDealEffect,
// DrawEffect, PumpEffect...), and what each understands of its parameters. A parameter, Defined$ or amount a
// translation doesn't handle makes the card unsupported (Mtg_ApiParamsKnown), never played wrong.
#include <stdlib.h>
#include <string.h>
#include "mtg_internal.h"

static const struct { const char *name; uint8_t api; } sApis[] = {
    {"DealDamage", MTG_API_DEAL_DAMAGE}, {"DamageAll", MTG_API_DAMAGE_ALL}, {"Draw", MTG_API_DRAW},
    {"Destroy", MTG_API_DESTROY}, {"DestroyAll", MTG_API_DESTROY_ALL}, {"GainLife", MTG_API_GAIN_LIFE},
    {"LoseLife", MTG_API_LOSE_LIFE}, {"Pump", MTG_API_PUMP}, {"PumpAll", MTG_API_PUMP_ALL},
    {"ChangeZone", MTG_API_CHANGE_ZONE}, {"Mana", MTG_API_MANA}, {"Discard", MTG_API_DISCARD},
    {"Tap", MTG_API_TAP}, {"Untap", MTG_API_UNTAP}, {"Counter", MTG_API_COUNTER},
};

const char *const gMtgApiNames[MTG_API_COUNT] = {
    "", "DealDamage", "DamageAll", "Draw", "Destroy", "DestroyAll", "GainLife", "LoseLife", "Pump", "PumpAll",
    "ChangeZone", "Mana", "Discard", "Tap", "Untap", "Counter",
};

int Mtg_ApiKnown(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof(sApis) / sizeof(sApis[0]); i++)
        if (strcmp(name, sApis[i].name) == 0)
            return sApis[i].api;
    return 0;
}

// ---- amounts: "3", "+2", "X" with SVar:X:Count$xPaid, Targeted$CardPower, Count$Valid ... (AbilityUtils.calculateAmount)
int Mtg_Amount(const MtgDuel *d, const MtgResolveCtx *ctx, int card, const char *value, int *ok)
{
    const char *sv;
    char *end;
    long n;
    *ok = 1;
    if (!value)
        return 0;
    if (value[0] == '+')
        value++;
    n = strtol(value, &end, 10);
    if (end != value && *end == 0)
        return (int)n;
    if (value[0] == '-' && value[1] && !(value[1] >= '0' && value[1] <= '9')) {   // "-X"
        int v = Mtg_Amount(d, ctx, card, value + 1, ok);
        return -v;
    }
    sv = Mtg_Svar(card, value);
    if (!sv) {
        *ok = 0;
        return 0;
    }
    if (strcmp(sv, "Count$xPaid") == 0)
        return d ? ctx->e->x : 0;
    if (strcmp(sv, "Targeted$CardPower") == 0)
        return d ? ctx->e->lkiPower : 0;
    if (strcmp(sv, "Targeted$CardToughness") == 0)
        return d ? ctx->e->lkiToughness : 0;
    if (strcmp(sv, "Count$CardPower") == 0)
        return d ? Mtg_Power(d, ctx->source) : 0;
    if (strcmp(sv, "Count$CardToughness") == 0)
        return d ? Mtg_Toughness(d, ctx->source) : 0;
    if (strncmp(sv, "Number$", 7) == 0) {
        n = strtol(sv + 7, &end, 10);
        if (*end) {
            *ok = 0;
            return 0;
        }
        return (int)n;
    }
    if (strncmp(sv, "Count$Valid ", 12) == 0) {
        int i, count = 0;
        if (!Mtg_ValidSpecKnown(sv + 12)) {
            *ok = 0;
            return 0;
        }
        if (!d)
            return 0;
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, sv + 12, ctx->source, ctx->controller))
                count++;
        return count;
    }
    *ok = 0;
    return 0;
}

// ---- who or what an effect acts on (AbilityUtils.getDefinedCards / getDefinedPlayers)
#define MTG_MAX_DEFINED 4
typedef struct { int n; MtgTarget t[MTG_MAX_DEFINED]; } Defined;

static const char *const sDefinedKnown[] = {
    "You", "Opponent", "Player", "Self", "Targeted", "ParentTarget", "TargetedController", "TargetedPlayer",
};

static void Add(Defined *out, int kind, int id)
{
    if (out->n < MTG_MAX_DEFINED) {
        out->t[out->n].kind = (uint8_t)kind;
        out->t[out->n].id = (uint16_t)id;
        out->n++;
    }
}

enum { DEF_PLAYER, DEF_CARD };   // what the effect acts on when nothing is targeted or Defined$

static void Targets(const MtgDuel *d, const MtgResolveCtx *ctx, const MtgAbility *ab, int deflt, Defined *out)
{
    const char *def = Mtg_Param(ab, "Defined");
    const MtgTarget *t = &ctx->e->target;
    out->n = 0;
    if (ab->tgt) {
        if (t->kind != MTG_TGT_NONE)
            Add(out, t->kind, t->id);
        return;
    }
    if (!def)
        def = deflt == DEF_PLAYER ? "You" : "Self";
    if (strcmp(def, "You") == 0)
        Add(out, MTG_TGT_PLAYER, ctx->controller);
    else if (strcmp(def, "Opponent") == 0)
        Add(out, MTG_TGT_PLAYER, ctx->controller ^ 1);
    else if (strcmp(def, "Player") == 0) {
        Add(out, MTG_TGT_PLAYER, d->active);   // (in turn order, APNAP)
        Add(out, MTG_TGT_PLAYER, d->active ^ 1);
    } else if (strcmp(def, "Self") == 0)
        Add(out, MTG_TGT_OBJ, ctx->source);
    else if (strcmp(def, "Targeted") == 0 || strcmp(def, "ParentTarget") == 0) {
        if (t->kind != MTG_TGT_NONE)
            Add(out, t->kind, t->id);
    } else if (strcmp(def, "TargetedController") == 0) {
        if (t->kind == MTG_TGT_OBJ || t->kind == MTG_TGT_STACK)
            Add(out, MTG_TGT_PLAYER, ctx->e->lkiController);
    } else if (strcmp(def, "TargetedPlayer") == 0) {
        if (t->kind == MTG_TGT_PLAYER)
            Add(out, MTG_TGT_PLAYER, t->id);
    }
}

static int OnBattlefield(const MtgDuel *d, const MtgTarget *t)
{
    return t->kind == MTG_TGT_OBJ && d->objs[t->id].zone == MTG_ZONE_BATTLEFIELD;
}

static uint32_t KeywordList(const char *s, int *ok)   // "Flying & First Strike"
{
    uint32_t mask = 0;
    char buf[96], *tok, *save;
    *ok = 1;
    if (!s)
        return 0;
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    for (save = buf, tok = Mtg_Token(&save, '&'); tok; tok = Mtg_Token(&save, '&')) {
        char *e;
        int k;
        while (*tok == ' ')
            tok++;
        e = tok + strlen(tok);
        while (e > tok && e[-1] == ' ')
            *--e = 0;
        k = Mtg_KeywordByName(tok);
        if (k < 0) {
            *ok = 0;
            return 0;
        }
        mask |= MTG_KW_BIT(k);
    }
    return mask;
}

static void Pump(MtgDuel *d, int obj, int att, int def, uint32_t kw)
{
    MtgObj *o = &d->objs[obj];
    o->tempPower = (int8_t)(o->tempPower + att);
    o->tempToughness = (int8_t)(o->tempToughness + def);
    o->tempKeywords |= kw;
}

static void CounterSpell(MtgDuel *d, int sourceObj)
{
    int i;
    for (i = 0; i < d->stackCount; i++)
        if (d->stack[i].source == sourceObj && d->stack[i].isSpell) {
            memmove(&d->stack[i], &d->stack[i + 1], (d->stackCount - i - 1) * sizeof(d->stack[0]));
            d->stackCount--;
            Mtg_MoveTo(d, sourceObj, MTG_ZONE_GRAVEYARD);
            Mtg_Event(d, MTG_EV_COUNTERED, sourceObj, 0, 0);
            return;
        }
}

static void Resolve(MtgDuel *d, MtgResolveCtx *ctx, const MtgAbility *ab)
{
    Defined who;
    int i, ok, card = ab->card;
    switch (ab->api) {
    case MTG_API_DEAL_DAMAGE: {
        int dmg = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "NumDmg"), &ok);
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            Mtg_DealDamage(d, ctx->source, who.t[i], dmg, 0);
        break;
    }
    case MTG_API_DAMAGE_ALL: {
        int dmg = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "NumDmg"), &ok);
        const char *cards = Mtg_Param(ab, "ValidCards"), *players = Mtg_Param(ab, "ValidPlayers");
        MtgTarget t;
        if (cards)
            for (i = 0; i < d->objCount; i++)
                if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, cards, ctx->source, ctx->controller)) {
                    t.kind = MTG_TGT_OBJ;
                    t.id = (uint16_t)i;
                    Mtg_DealDamage(d, ctx->source, t, dmg, 0);
                }
        if (players)
            for (i = 0; i < MTG_PLAYERS; i++)
                if (Mtg_ValidPlayer(d, i, players, ctx->controller)) {
                    t.kind = MTG_TGT_PLAYER;
                    t.id = (uint16_t)i;
                    Mtg_DealDamage(d, ctx->source, t, dmg, 0);
                }
        break;
    }
    case MTG_API_DRAW: {
        const char *nc = Mtg_Param(ab, "NumCards");
        int n = nc ? Mtg_Amount(d, ctx, card, nc, &ok) : 1;
        Targets(d, ctx, ab, DEF_PLAYER, &who);
        for (i = 0; i < who.n; i++)
            if (who.t[i].kind == MTG_TGT_PLAYER)
                Mtg_Draw(d, who.t[i].id, n);
        break;
    }
    case MTG_API_DESTROY:
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (OnBattlefield(d, &who.t[i]))
                Mtg_Destroy(d, who.t[i].id);
        break;
    case MTG_API_DESTROY_ALL: {
        const char *cards = Mtg_Param(ab, "ValidCards");
        uint8_t doomed[MTG_MAX_OBJS];   // (chosen first, destroyed together)
        for (i = 0; i < d->objCount; i++)
            doomed[i] = d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, cards, ctx->source, ctx->controller);
        for (i = 0; i < d->objCount; i++)
            if (doomed[i])
                Mtg_Destroy(d, i);
        break;
    }
    case MTG_API_GAIN_LIFE:
    case MTG_API_LOSE_LIFE: {
        int n = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "LifeAmount"), &ok);
        Targets(d, ctx, ab, DEF_PLAYER, &who);
        for (i = 0; i < who.n; i++)
            if (who.t[i].kind == MTG_TGT_PLAYER) {
                if (ab->api == MTG_API_GAIN_LIFE)
                    Mtg_GainLife(d, who.t[i].id, n);
                else {
                    d->players[who.t[i].id].life -= n;
                    Mtg_Event(d, MTG_EV_LIFE, who.t[i].id, -n, d->players[who.t[i].id].life);
                }
            }
        break;
    }
    case MTG_API_PUMP:
    case MTG_API_PUMP_ALL: {
        int att = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "NumAtt"), &ok);
        int def = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "NumDef"), &ok);
        uint32_t kw = KeywordList(Mtg_Param(ab, "KW"), &ok);
        if (ab->api == MTG_API_PUMP_ALL) {
            const char *cards = Mtg_Param(ab, "ValidCards");
            for (i = 0; i < d->objCount; i++)
                if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, cards, ctx->source, ctx->controller))
                    Pump(d, i, att, def, kw);
            break;
        }
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (OnBattlefield(d, &who.t[i]))
                Pump(d, who.t[i].id, att, def, kw);
        break;
    }
    case MTG_API_CHANGE_ZONE: {
        const char *dest = Mtg_Param(ab, "Destination");
        int zone = strcmp(dest, "Exile") == 0 ? MTG_ZONE_EXILE : strcmp(dest, "Hand") == 0 ? MTG_ZONE_HAND
                 : strcmp(dest, "Library") == 0 ? MTG_ZONE_LIBRARY : MTG_ZONE_GRAVEYARD;
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (OnBattlefield(d, &who.t[i]))
                Mtg_MoveTo(d, who.t[i].id, zone);
        break;
    }
    case MTG_API_MANA: {
        const char *prod = Mtg_Param(ab, "Produced"), *amt = Mtg_Param(ab, "Amount");
        int n = amt ? Mtg_Amount(d, ctx, card, amt, &ok) : 1;
        int mask = Mtg_ProducedColors(prod), c;
        for (c = 0; c < MTG_MANA_KINDS; c++)
            if (mask & (1 << c)) {
                d->players[ctx->controller].pool[c] += (uint8_t)n;
                break;
            }
        break;
    }
    case MTG_API_DISCARD: {   // Mode$ Random
        const char *nc = Mtg_Param(ab, "NumCards");
        int n = nc ? Mtg_Amount(d, ctx, card, nc, &ok) : 1;
        Targets(d, ctx, ab, DEF_PLAYER, &who);
        for (i = 0; i < who.n; i++) {
            int p = who.t[i].id, k;
            if (who.t[i].kind != MTG_TGT_PLAYER)
                continue;
            for (k = 0; k < n; k++) {
                uint16_t hand[MTG_MAX_OBJS];
                int h = 0, j;
                for (j = 0; j < d->objCount; j++)
                    if (d->objs[j].zone == MTG_ZONE_HAND && d->objs[j].owner == p)
                        hand[h++] = (uint16_t)j;
                if (!h)
                    break;
                Mtg_MoveTo(d, hand[Mtg_Random(d, (uint32_t)h)], MTG_ZONE_GRAVEYARD);
            }
        }
        break;
    }
    case MTG_API_TAP:
    case MTG_API_UNTAP:
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (OnBattlefield(d, &who.t[i]))
                Mtg_Tap(d, who.t[i].id, ab->api == MTG_API_TAP);
        break;
    case MTG_API_COUNTER:
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (who.t[i].kind == MTG_TGT_STACK)
                CounterSpell(d, who.t[i].id);
        break;
    }
}

void Mtg_ResolveAbility(MtgDuel *d, MtgResolveCtx *ctx, int ability)
{
    int guard = 0;
    while (ability != 0xFFFF && guard++ < 16 && !d->over) {
        const MtgAbility *ab = &gMtgAbilities[ability];
        Resolve(d, ctx, ab);
        ability = ab->sub;
    }
}

// ---- what a translation understands (else the card stays unsupported)
static const char *const sCommon[] = {
    "SP", "AB", "DB", "Cost", "ValidTgts", "TgtPrompt", "SubAbility", "SpellDescription", "StackDescription",
    "IsCurse", "PrecostDesc", "CostDesc", "TargetType", "Defined",
};
static const char *const sApiParams[MTG_API_COUNT][5] = {
    [MTG_API_DEAL_DAMAGE] = {"NumDmg"},
    [MTG_API_DAMAGE_ALL] = {"NumDmg", "ValidCards", "ValidPlayers", "ValidDescription"},
    [MTG_API_DRAW] = {"NumCards"},
    [MTG_API_DESTROY] = {"NoRegen"},
    [MTG_API_DESTROY_ALL] = {"ValidCards", "NoRegen"},
    [MTG_API_GAIN_LIFE] = {"LifeAmount"},
    [MTG_API_LOSE_LIFE] = {"LifeAmount"},
    [MTG_API_PUMP] = {"NumAtt", "NumDef", "KW"},
    [MTG_API_PUMP_ALL] = {"NumAtt", "NumDef", "KW", "ValidCards"},
    [MTG_API_CHANGE_ZONE] = {"Origin", "Destination"},
    [MTG_API_MANA] = {"Produced", "Amount"},
    [MTG_API_DISCARD] = {"NumCards", "Mode"},
};

static int InList(const char *key, const char *const *list, int n)
{
    int i;
    for (i = 0; i < n && list[i]; i++)
        if (strcmp(key, list[i]) == 0)
            return 1;
    return 0;
}

static int AmountKnown(const MtgAbility *ab, const char *key, int required)
{
    const char *v = Mtg_Param(ab, key);
    int ok;
    if (!v)
        return !required;
    Mtg_Amount(NULL, NULL, ab->card, v, &ok);
    return ok;
}

int Mtg_ApiParamsKnown(const MtgAbility *ab, const char **why)
{
    int i, ok;
    const char *v;
    *why = gMtgApiNames[ab->api];   // (an unhandled value of a known parameter: the API is named)
    for (i = 0; i < ab->paramCount; i++) {
        const char *key = gMtgParams[ab->paramFirst + i].key;
        if (strncmp(key, "AI", 2) == 0)
            continue;   // (Forge's own AI hints: read by the AI's translation, never by the rules)
        if (!InList(key, sCommon, (int)(sizeof(sCommon) / sizeof(sCommon[0]))) && !InList(key, sApiParams[ab->api], 5)) {
            *why = key;
            return 0;
        }
    }
    v = Mtg_Param(ab, "Defined");
    if (v && !InList(v, sDefinedKnown, (int)(sizeof(sDefinedKnown) / sizeof(sDefinedKnown[0])))) {
        *why = "Defined";
        return 0;
    }
    if (ab->kind == MTG_AB_SUB && ab->tgt) {
        *why = "a second target";
        return 0;
    }
    switch (ab->api) {
    case MTG_API_DEAL_DAMAGE:
    case MTG_API_DAMAGE_ALL:
        if (!AmountKnown(ab, "NumDmg", 1))
            return 0;
        if (ab->api == MTG_API_DAMAGE_ALL) {
            const char *c = Mtg_Param(ab, "ValidCards"), *p = Mtg_Param(ab, "ValidPlayers");
            if ((c && !Mtg_ValidSpecKnown(c)) || (p && !Mtg_ValidSpecKnown(p)) || (!c && !p))
                return 0;
        }
        return 1;
    case MTG_API_DRAW:
    case MTG_API_DISCARD:
        if (!AmountKnown(ab, "NumCards", 0))
            return 0;
        if (ab->api == MTG_API_DISCARD && !((v = Mtg_Param(ab, "Mode")) && strcmp(v, "Random") == 0))
            return 0;
        return 1;
    case MTG_API_DESTROY_ALL:
        return (v = Mtg_Param(ab, "ValidCards")) && Mtg_ValidSpecKnown(v);
    case MTG_API_GAIN_LIFE:
    case MTG_API_LOSE_LIFE:
        return AmountKnown(ab, "LifeAmount", 1);
    case MTG_API_PUMP:
    case MTG_API_PUMP_ALL:
        KeywordList(Mtg_Param(ab, "KW"), &ok);
        if (!ok || !AmountKnown(ab, "NumAtt", 0) || !AmountKnown(ab, "NumDef", 0))
            return 0;
        if (ab->api == MTG_API_PUMP_ALL)
            return (v = Mtg_Param(ab, "ValidCards")) && Mtg_ValidSpecKnown(v);
        return 1;
    case MTG_API_CHANGE_ZONE: {
        const char *o = Mtg_Param(ab, "Origin"), *dz = Mtg_Param(ab, "Destination");
        if (!o || strcmp(o, "Battlefield") != 0 || !dz)
            return 0;
        if (strcmp(dz, "Exile") && strcmp(dz, "Hand") && strcmp(dz, "Graveyard") && strcmp(dz, "Library"))
            return 0;
        return ab->tgt || Mtg_Param(ab, "Defined");
    }
    case MTG_API_MANA:
        if (Mtg_Param(ab, "SubAbility") || !AmountKnown(ab, "Amount", 0))
            return 0;
        v = Mtg_Param(ab, "Produced");
        if (!v || Mtg_ProducedColors(v) <= 0)
            return 0;
        if (ab->kind != MTG_AB_ACTIVATED && strchr(v, ' '))
            return 0;   // (a spell choosing its color: not yet)
        return 1;
    case MTG_API_COUNTER:
        return ab->tgtStack;
    }
    return 1;
}
