// Moemon Kai: triggered abilities, Forge's TriggerHandler.java and the Trigger<Mode>.java classes translated so far
// (TriggerChangesZone, TriggerPhase, TriggerSpellCast, TriggerDamageDone, TriggerAttacks, TriggerAttackerBlocked,
// TriggerAttackerBlockedByCreature, TriggerBlocks, TriggerAttackerUnblocked). A T: line is read into a trigger whose
// Execute$ SVar is an MTG_AB_TRIGGERED ability; what happens in a duel (mtg_game.c) is checked against every trigger
// of every card in its zone, and the ones that match wait in d->pending until a player would get priority, when they
// go on the stack (the active player's first, 101.4). A mode, parameter or value not written here makes the card
// unsupported, never played wrong.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtg_internal.h"

MtgTrigger *gMtgTriggers;
static int sTrigCount, sTrigCap;

const char *const gMtgTriggerModes[MTG_TRIG_COUNT] = {
    "", "ChangesZone", "Phase", "SpellCast", "DamageDone", "Attacks", "AttackerBlocked", "AttackerBlockedByCreature",
    "Blocks", "AttackerUnblocked", "Cycled", "Taps", "LandPlayed",
};

// the parameters each mode reads (besides the common ones)
static const char *const sModeKeys[MTG_TRIG_COUNT][4] = {
    [MTG_TRIG_CHANGES_ZONE] = {"Origin", "Destination", "ValidCard"},
    [MTG_TRIG_PHASE] = {"Phase", "ValidPlayer"},
    [MTG_TRIG_SPELL_CAST] = {"ValidCard", "ValidActivatingPlayer"},
    [MTG_TRIG_CYCLED] = {"ValidCard"},
    [MTG_TRIG_TAPS] = {"ValidCard"},
    [MTG_TRIG_LAND_PLAYED] = {"ValidCard"},
    [MTG_TRIG_DAMAGE_DONE] = {"ValidSource", "ValidTarget", "CombatDamage"},
    [MTG_TRIG_ATTACKS] = {"ValidCard"},
    [MTG_TRIG_ATTACKER_BLOCKED] = {"ValidCard", "ValidBlocker"},
    [MTG_TRIG_ATTACKER_BLOCKED_BY] = {"ValidCard", "ValidBlocker"},
    [MTG_TRIG_BLOCKS] = {"ValidCard", "ValidBlocked"},
    [MTG_TRIG_ATTACKER_UNBLOCKED] = {"ValidCard"},
};
static const char *const sCommonKeys[] = {
    "Mode", "Execute", "TriggerDescription", "TriggerZones", "OptionalDecider", "Secondary",
    "IsPresent", "PresentCompare", "PresentZone", "CheckSVar", "SVarCompare",
};
// the parameters that are Valid specs (mtg_valid.c)
static const char *const sSpecKeys[] = {
    "ValidCard", "ValidPlayer", "ValidActivatingPlayer", "ValidSource", "ValidTarget", "ValidBlocker", "ValidBlocked",
    "IsPresent",
};

static const struct { const char *name; uint8_t step; } sPhases[] = {
    {"Upkeep", MTG_STEP_UPKEEP}, {"Draw", MTG_STEP_DRAW}, {"Main1", MTG_STEP_MAIN1},
    {"BeginCombat", MTG_STEP_COMBAT_BEGIN}, {"EndCombat", MTG_STEP_COMBAT_END}, {"Main2", MTG_STEP_MAIN2},
    {"End of Turn", MTG_STEP_END},
};
static const struct { const char *name; uint8_t zone; } sZones[] = {
    {"Battlefield", MTG_ZONE_BATTLEFIELD}, {"Graveyard", MTG_ZONE_GRAVEYARD}, {"Hand", MTG_ZONE_HAND},
    {"Library", MTG_ZONE_LIBRARY}, {"Exile", MTG_ZONE_EXILE}, {"Stack", MTG_ZONE_STACK},
};
#define ZONE_BIT(z) (1u << (z))

const char *Mtg_TriggerParam(const MtgTrigger *t, const char *key)
{
    int i;
    for (i = 0; i < t->paramCount; i++)
        if (strcmp(gMtgParams[t->paramFirst + i].key, key) == 0)
            return gMtgParams[t->paramFirst + i].value;
    return NULL;
}

static const char *Reason(const char *key)   // "T:BecomesTarget", kept once each (docs/MTG_SCOPE.md counts them)
{
    static char *sReasons[256];
    static int sCount;
    char buf[64];
    int i;
    snprintf(buf, sizeof(buf), "T:%s", key);
    for (i = 0; i < sCount; i++)
        if (strcmp(sReasons[i], buf) == 0)
            return sReasons[i];
    if (sCount == 256)
        return "trigger";
    sReasons[sCount] = malloc(strlen(buf) + 1);
    strcpy(sReasons[sCount], buf);
    return sReasons[sCount++];
}

static int InList(const char *key, const char *const *list, int n)
{
    int i;
    for (i = 0; i < n && list[i]; i++)
        if (strcmp(key, list[i]) == 0)
            return 1;
    return 0;
}

// "Battlefield,Graveyard" -> zone bits (0: one it doesn't know); "Any" -> every zone
static unsigned Zones(const char *s)
{
    unsigned bits = 0, i;
    char buf[64], *tok, *save;
    if (strcmp(s, "Any") == 0)
        return 0xFE;
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    for (save = buf, tok = Mtg_Token(&save, ','); tok; tok = Mtg_Token(&save, ',')) {
        unsigned found = 0;
        for (i = 0; i < sizeof(sZones) / sizeof(sZones[0]); i++)
            if (strcmp(tok, sZones[i].name) == 0)
                found = ZONE_BIT(sZones[i].zone);
        if (!found)
            return 0;
        bits |= found;
    }
    return bits;
}

static unsigned Steps(const char *s)
{
    unsigned bits = 0, i;
    char buf[64], *tok, *save;
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    for (save = buf, tok = Mtg_Token(&save, ','); tok; tok = Mtg_Token(&save, ',')) {
        unsigned found = 0;
        for (i = 0; i < sizeof(sPhases) / sizeof(sPhases[0]); i++)
            if (strcmp(tok, sPhases[i].name) == 0)
                found = 1u << sPhases[i].step;
        if (!found)
            return 0;
        bits |= found;
    }
    return bits;
}

// "GE1", "EQ0" (Expressions.compare)
static int Compare(const char *s, int v, int *ok)
{
    int n;
    *ok = 1;
    if (!s)
        s = "GE1";
    if (strlen(s) < 3 || s[2] < '0' || s[2] > '9') {
        *ok = 0;
        return 0;
    }
    n = atoi(s + 2);
    if (strncmp(s, "EQ", 2) == 0) return v == n;
    if (strncmp(s, "NE", 2) == 0) return v != n;
    if (strncmp(s, "GE", 2) == 0) return v >= n;
    if (strncmp(s, "GT", 2) == 0) return v > n;
    if (strncmp(s, "LE", 2) == 0) return v <= n;
    if (strncmp(s, "LT", 2) == 0) return v < n;
    *ok = 0;
    return 0;
}

// the Defined$ / UnlessPayer$ values of an ability chain that name what set it off must name something this mode has
static int ChainKnown(int ability, int mode, const char **why)
{
    int guard = 0;
    while (ability != 0xFFFF && guard++ < 16) {
        const MtgAbility *ab = &gMtgAbilities[ability];
        const char *v;
        if ((v = Mtg_Param(ab, "Defined")) && strncmp(v, "Triggered", 9) == 0 && !Mtg_TriggeredDefinedKnown(mode, v)) {
            *why = v;
            return 0;
        }
        if ((v = Mtg_Param(ab, "UnlessPayer")) && strncmp(v, "Triggered", 9) == 0
            && !Mtg_TriggeredDefinedKnown(mode, v)) {
            *why = v;
            return 0;
        }
        ability = ab->sub;
    }
    return 1;
}

// ATTACH: one of the card's own (T: lines); else a static's AddTrigger$ (the trigger only). -1: unsupported
static int ParseTrig(MtgCardDef *c, int cardId, char *text, int attach)
{
    MtgTrigger *t;
    int first, n, idx, i, mode = 0;
    const char *v, *why;
    n = Mtg_ParseParams(text, &first);
    if (sTrigCount + 1 > sTrigCap) {
        sTrigCap = sTrigCap ? sTrigCap * 2 : 1024;
        gMtgTriggers = realloc(gMtgTriggers, (size_t)sTrigCap * sizeof(MtgTrigger));
    }
    idx = sTrigCount++;
    t = &gMtgTriggers[idx];
    memset(t, 0, sizeof(*t));
    t->card = (uint16_t)cardId;
    t->execute = 0xFFFF;
    t->paramFirst = (uint16_t)first;
    t->paramCount = (uint16_t)n;
    if (attach) {
        if (c->triggerCount >= sizeof(c->triggers) / sizeof(c->triggers[0])) {
            Mtg_Unsupported(c, "too many triggers");
            return -1;
        }
        c->triggers[c->triggerCount++] = (uint16_t)idx;
    }
    v = Mtg_TriggerParam(t, "Mode");
    for (i = 1; v && i < MTG_TRIG_COUNT; i++)
        if (strcmp(v, gMtgTriggerModes[i]) == 0)
            mode = i;
    if (!mode) {
        Mtg_Unsupported(c, Reason(v ? v : "?"));
        return -1;
    }
    t->mode = (uint8_t)mode;
    for (i = 0; i < n; i++) {
        const char *key = gMtgParams[first + i].key, *val = gMtgParams[first + i].value;
        if (!InList(key, sCommonKeys, (int)(sizeof(sCommonKeys) / sizeof(sCommonKeys[0])))
            && !InList(key, sModeKeys[mode], 4)) {
            Mtg_Unsupported(c, Reason(key));
            return -1;
        }
        if (InList(key, sSpecKeys, (int)(sizeof(sSpecKeys) / sizeof(sSpecKeys[0]))) && !Mtg_ValidSpecKnown(val)) {
            Mtg_Unsupported(c, Reason(key));
            return -1;
        }
    }
    // where the card must be: TriggerZones$, else TriggerChangesZone.correctZones' (leaving the battlefield: from
    // there; its own arrival: where it arrives), else the battlefield
    if ((v = Mtg_TriggerParam(t, "TriggerZones")) != NULL) {
        t->zones = (uint8_t)Zones(v);
        if (!t->zones || strcmp(v, "Any") == 0) {
            Mtg_Unsupported(c, Reason("TriggerZones"));
            return -1;
        }
    } else if (mode == MTG_TRIG_CHANGES_ZONE && (v = Mtg_TriggerParam(t, "ValidCard")) && strstr(v, "Self")
               && (!Mtg_TriggerParam(t, "Origin") || strcmp(Mtg_TriggerParam(t, "Origin"), "Any") == 0)) {
        const char *dz = Mtg_TriggerParam(t, "Destination");
        t->zones = (uint8_t)(dz && strcmp(dz, "Any") ? Zones(dz) : 0);
        if (!t->zones) {
            Mtg_Unsupported(c, Reason("Destination"));
            return -1;
        }
    } else if (mode == MTG_TRIG_CYCLED && (v = Mtg_TriggerParam(t, "ValidCard")) && strstr(v, "Self"))
        t->zones = (uint8_t)ZONE_BIT(MTG_ZONE_GRAVEYARD);   // ("when you cycle CARDNAME": it's in the graveyard by then)
    else
        t->zones = (uint8_t)ZONE_BIT(MTG_ZONE_BATTLEFIELD);
    if (mode == MTG_TRIG_CHANGES_ZONE) {
        if (!(v = Mtg_TriggerParam(t, "Destination")) || !Zones(v)
            || ((v = Mtg_TriggerParam(t, "Origin")) && !Zones(v))) {
            Mtg_Unsupported(c, Reason("Origin"));
            return -1;
        }
    }
    if (mode == MTG_TRIG_PHASE && (!(v = Mtg_TriggerParam(t, "Phase")) || !Steps(v))) {
        Mtg_Unsupported(c, Reason("Phase"));
        return -1;
    }
    if ((v = Mtg_TriggerParam(t, "CombatDamage")) && strcmp(v, "True") && strcmp(v, "False")) {
        Mtg_Unsupported(c, Reason("CombatDamage"));
        return -1;
    }
    if ((v = Mtg_TriggerParam(t, "OptionalDecider")) != NULL) {   // who says "may": its controller, or another
        t->optional = strcmp(v, "You") == 0 ? 1 : strcmp(v, "TriggeredPlayer") == 0 ? 4
                    : strcmp(v, "TriggeredCardController") == 0 || strcmp(v, "TriggeredSourceController") == 0 ? 3 : 0;
        if (!t->optional) {
            Mtg_Unsupported(c, Reason("OptionalDecider"));
            return -1;
        }
    }
    if ((v = Mtg_TriggerParam(t, "CheckSVar")) != NULL) {   // (an "if" on an amount counted as it triggers and resolves)
        if (!Mtg_CheckSvarKnown(cardId, v, Mtg_TriggerParam(t, "SVarCompare"))) {
            Mtg_Unsupported(c, Reason("CheckSVar"));
            return -1;
        }
    } else if (Mtg_TriggerParam(t, "SVarCompare")) {
        Mtg_Unsupported(c, Reason("SVarCompare"));
        return -1;
    }
    if (Mtg_TriggerParam(t, "IsPresent")) {
        int ok;
        Compare(Mtg_TriggerParam(t, "PresentCompare"), 0, &ok);
        v = Mtg_TriggerParam(t, "PresentZone");
        if (!ok || (v && (!Zones(v) || strcmp(v, "Any") == 0))) {
            Mtg_Unsupported(c, Reason("IsPresent"));
            return -1;
        }
    } else if (Mtg_TriggerParam(t, "PresentCompare") || Mtg_TriggerParam(t, "PresentZone")) {
        Mtg_Unsupported(c, Reason("PresentCompare"));
        return -1;
    }
    // what it does: an SVar "DB$ ..." (an ability of its own kind: it may target, as a spell does)
    v = Mtg_TriggerParam(t, "Execute");
    v = v ? Mtg_Svar(cardId, v) : NULL;
    if (!v || strncmp(v, "DB$", 3) != 0) {
        Mtg_Unsupported(c, Reason("Execute"));
        return -1;
    }
    {
        size_t len = strlen(v) + 1;
        char *copy = malloc(len);
        int ab;
        memcpy(copy, v, len);
        ab = Mtg_ParseAbilityText(c, cardId, copy, MTG_AB_TRIGGERED);
        gMtgTriggers[idx].execute = (uint16_t)ab;
        if (!ChainKnown(ab, mode, &why)) {
            Mtg_Unsupported(c, why);
            return -1;
        }
    }
    return idx;
}

void Mtg_ParseTrigger(MtgCardDef *c, int cardId, char *text)
{
    ParseTrig(c, cardId, text, 1);
}

int Mtg_ParseGrantedTrigger(MtgCardDef *c, int cardId, char *text)
{
    return ParseTrig(c, cardId, text, 0);
}


// IsPresent$ (Trigger.requirementsCheck's "present" test): how many such cards are in the zone
static int Present(const MtgDuel *d, const MtgTrigger *t, int host)
{
    const char *spec = Mtg_TriggerParam(t, "IsPresent"), *zv, *sv = Mtg_TriggerParam(t, "CheckSVar");
    unsigned zones;
    int i, count = 0, ok;
    if (sv && !Mtg_CheckSvar(d, t->card, host, d->objs[host].controller, sv, Mtg_TriggerParam(t, "SVarCompare")))
        return 0;   // (CheckSVar$: checked with IsPresent$, as it triggers and again as it resolves)
    if (!spec)
        return 1;
    zv = Mtg_TriggerParam(t, "PresentZone");
    zones = zv ? Zones(zv) : ZONE_BIT(MTG_ZONE_BATTLEFIELD);
    for (i = 0; i < d->objCount; i++)
        if ((zones & ZONE_BIT(d->objs[i].zone)) && Mtg_ValidObj(d, i, spec, host, d->objs[host].controller))
            count++;
    return Compare(Mtg_TriggerParam(t, "PresentCompare"), count, &ok);
}

int Mtg_TriggerStillTrue(const MtgDuel *d, const MtgStackEntry *e)
{
    if (e->trigger == 0xFFFF)
        return 1;
    return Present(d, &gMtgTriggers[e->trigger], e->source);
}

static int ObjOk(const MtgDuel *d, const MtgTrigger *t, const char *key, int obj, int host)
{
    const char *spec = Mtg_TriggerParam(t, key);
    if (!spec)
        return 1;
    if (obj == MTG_NO_OBJ)
        return 0;
    return Mtg_ValidObj(d, obj, spec, host, d->objs[host].controller);
}

static int PlayerOk(const MtgDuel *d, const MtgTrigger *t, const char *key, int player, int host)
{
    const char *spec = Mtg_TriggerParam(t, key);
    if (!spec)
        return 1;
    return player < MTG_PLAYERS && Mtg_ValidPlayerFrom(d, player, spec, host, d->objs[host].controller);
}

// Trigger<Mode>.performTest
static int Matches(const MtgDuel *d, const MtgTrigger *t, int host, const MtgTrigEvent *ev)
{
    const char *v;
    int i;
    switch (t->mode) {
    case MTG_TRIG_CHANGES_ZONE:
        if ((v = Mtg_TriggerParam(t, "Origin")) && !(Zones(v) & ZONE_BIT(ev->from)))
            return 0;
        if (!(Zones(Mtg_TriggerParam(t, "Destination")) & ZONE_BIT(ev->to)))
            return 0;
        return ObjOk(d, t, "ValidCard", ev->obj, host);
    case MTG_TRIG_PHASE:
        return (Steps(Mtg_TriggerParam(t, "Phase")) & (1u << ev->step)) && PlayerOk(d, t, "ValidPlayer", ev->player, host);
    case MTG_TRIG_SPELL_CAST:
        return ObjOk(d, t, "ValidCard", ev->obj, host) && PlayerOk(d, t, "ValidActivatingPlayer", ev->player, host);
    case MTG_TRIG_DAMAGE_DONE:
        if ((v = Mtg_TriggerParam(t, "CombatDamage")) && (strcmp(v, "True") == 0) != (ev->combat != 0))
            return 0;
        if (!ObjOk(d, t, "ValidSource", ev->obj, host))
            return 0;
        if ((v = Mtg_TriggerParam(t, "ValidTarget")) != NULL) {
            if (ev->player < MTG_PLAYERS)
                return Mtg_ValidPlayerFrom(d, ev->player, v, host, d->objs[host].controller);
            return ev->obj2 != MTG_NO_OBJ && Mtg_ValidObj(d, ev->obj2, v, host, d->objs[host].controller);
        }
        return 1;
    case MTG_TRIG_ATTACKER_BLOCKED:
        if (!ObjOk(d, t, "ValidCard", ev->obj, host))
            return 0;
        if (!Mtg_TriggerParam(t, "ValidBlocker"))
            return 1;
        for (i = 0; i < d->objCount; i++)   // (any of its blockers)
            if (d->objs[i].blocking == ev->obj && ObjOk(d, t, "ValidBlocker", i, host))
                return 1;
        return 0;
    case MTG_TRIG_ATTACKER_BLOCKED_BY:
        return ObjOk(d, t, "ValidCard", ev->obj, host) && ObjOk(d, t, "ValidBlocker", ev->obj2, host);
    case MTG_TRIG_BLOCKS:
        return ObjOk(d, t, "ValidCard", ev->obj, host) && ObjOk(d, t, "ValidBlocked", ev->obj2, host);
    case MTG_TRIG_ATTACKS:
    case MTG_TRIG_ATTACKER_UNBLOCKED:
    case MTG_TRIG_CYCLED:
    case MTG_TRIG_TAPS:
    case MTG_TRIG_LAND_PLAYED:
        return ObjOk(d, t, "ValidCard", ev->obj, host);
    }
    return 0;
}

void Mtg_CheckTriggers(MtgDuel *d, const MtgTrigEvent *ev)
{
    int i, k;
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        const MtgCardDef *c = Mtg_Card(o->def);
        uint16_t trig[12];
        int trigCount;
        if (!c->supported)
            continue;
        trigCount = Mtg_ObjTriggers(d, i, trig, 12);   // (its card's and those statics give it: AddTrigger$)
        for (k = 0; k < trigCount; k++) {
            const MtgTrigger *t = &gMtgTriggers[trig[k]];
            MtgPending *p;
            if (t->mode != ev->mode || !(t->zones & ZONE_BIT(o->zone)) || !Matches(d, t, i, ev) || !Present(d, t, i))
                continue;
            if (d->pendingCount >= MTG_MAX_PENDING)
                return;
            p = &d->pending[d->pendingCount++];
            p->source = (uint16_t)i;
            p->trigger = trig[k];
            p->trigObj = ev->obj;
            p->trigObj2 = ev->obj2;
            p->controller = o->zone == MTG_ZONE_BATTLEFIELD ? o->controller : o->owner;
            p->trigPlayer = ev->player;
            p->trigCtrl = ev->objCtrl;
            p->trigAmount = ev->amount;
        }
    }
}

// DelayedTriggerEffect's trigger: Mode$ Phase at Phase$ for ValidPlayer$, doing Execute$ (an SVar of the card). The
// ability's own params serve as the trigger's (its Mode$ / Phase$ / ValidPlayer$ / Execute$ / TriggerDescription$).
int Mtg_MakeDelayedTrigger(MtgCardDef *c, int cardId, int ability)
{
    const MtgAbility *ab = &gMtgAbilities[ability];
    MtgTrigger *t;
    const char *v, *why;
    int idx, ex;
    size_t len;
    char *copy;
    if (!(v = Mtg_Param(ab, "Mode")) || strcmp(v, "Phase") || !(v = Mtg_Param(ab, "Phase")) || !Steps(v)
        || ((v = Mtg_Param(ab, "ValidPlayer")) && strcmp(v, "Player") && strcmp(v, "You"))
        || !(v = Mtg_Param(ab, "Execute")) || !(v = Mtg_Svar(cardId, v)) || strncmp(v, "DB$", 3) != 0) {
        Mtg_Unsupported(c, "DelayedTrigger");
        return -1;
    }
    len = strlen(v) + 1;
    copy = malloc(len);
    memcpy(copy, v, len);
    ex = Mtg_ParseAbilityText(c, cardId, copy, MTG_AB_TRIGGERED);
    if (!ChainKnown(ex, MTG_TRIG_PHASE, &why)) {
        Mtg_Unsupported(c, why);
        return -1;
    }
    if (sTrigCount + 1 > sTrigCap) {
        sTrigCap = sTrigCap ? sTrigCap * 2 : 1024;
        gMtgTriggers = realloc(gMtgTriggers, (size_t)sTrigCap * sizeof(MtgTrigger));
    }
    idx = sTrigCount++;
    t = &gMtgTriggers[idx];
    memset(t, 0, sizeof(*t));
    ab = &gMtgAbilities[ability];   // (the table may have moved)
    t->mode = MTG_TRIG_PHASE;
    t->card = (uint16_t)cardId;
    t->execute = (uint16_t)ex;
    t->paramFirst = ab->paramFirst;
    t->paramCount = ab->paramCount;
    t->zones = 0;   // (on no card: only its ability sets it up)
    return idx;
}

void Mtg_FireDelayed(MtgDuel *d)
{
    int i = 0;
    while (i < d->delayedCount) {
        MtgDelayed *dl = &d->delayed[i];
        const MtgTrigger *t = &gMtgTriggers[dl->trigger];
        const char *vp = Mtg_TriggerParam(t, "ValidPlayer");
        MtgPending *p;
        if (!(Steps(Mtg_TriggerParam(t, "Phase")) & (1u << d->step)) || (dl->nextTurn && d->turn == dl->turn)
            || (vp && strcmp(vp, "You") == 0 && d->active != dl->controller)) {
            i++;
            continue;
        }
        if (d->pendingCount < MTG_MAX_PENDING) {
            p = &d->pending[d->pendingCount++];
            memset(p, 0, sizeof(*p));
            p->source = dl->source;
            p->trigger = dl->trigger;
            p->trigObj = dl->obj;
            p->trigObj2 = MTG_NO_OBJ;
            p->controller = dl->controller;
            p->trigPlayer = (uint8_t)d->active;
        }
        memmove(dl, dl + 1, (size_t)(d->delayedCount - i - 1) * sizeof(*dl));
        d->delayedCount--;
    }
}
