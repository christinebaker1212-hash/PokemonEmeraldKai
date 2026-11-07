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
    {"Sacrifice", MTG_API_SACRIFICE}, {"PutCounter", MTG_API_PUT_COUNTER}, {"Attach", MTG_API_ATTACH},
    {"Regenerate", MTG_API_REGENERATE}, {"Token", MTG_API_TOKEN},
    {"Charm", MTG_API_CHARM}, {"ChangeZoneAll", MTG_API_CHANGE_ZONE_ALL}, {"Animate", MTG_API_ANIMATE},
    {"DelayedTrigger", MTG_API_DELAYED_TRIGGER}, {"PreventDamage", MTG_API_PREVENT_DAMAGE},
    {"GainControl", MTG_API_GAIN_CONTROL},
};

const char *const gMtgApiNames[MTG_API_COUNT] = {
    "", "DealDamage", "DamageAll", "Draw", "Destroy", "DestroyAll", "GainLife", "LoseLife", "Pump", "PumpAll",
    "ChangeZone", "Mana", "Discard", "Tap", "Untap", "Counter", "Sacrifice", "PutCounter", "Attach", "Regenerate", "Token", "Charm",
    "ChangeZoneAll", "Animate", "DelayedTrigger", "PreventDamage", "GainControl",
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
    if (strcmp(sv, "TriggerCount$DamageAmount") == 0)   // (a DamageDone trigger's)
        return d ? ctx->e->trigAmount : 0;
    if (strncmp(sv, "Number$", 7) == 0) {
        n = strtol(sv + 7, &end, 10);
        if (*end) {
            *ok = 0;
            return 0;
        }
        return (int)n;
    }
    if (strcmp(sv, "Sacrificed$CardPower") == 0)
        return d ? ctx->e->sacPower : 0;
    if (strcmp(sv, "Targeted$CardManaCost") == 0) {
        if (!d)
            return 0;
        return ctx->e->target.kind == MTG_TGT_OBJ || ctx->e->target.kind == MTG_TGT_STACK
            ? Mtg_ManaCostTotal(&Mtg_Card(d->objs[ctx->e->target.id].def)->cost) : 0;
    }
    // (two players: "the opponent with the most / fewest" is the opponent) the Valid spec read from their side
    if (strncmp(sv, "PlayerCountOpponents$HighestValid ", 34) == 0 || strncmp(sv, "PlayerCountOpponents$LowestValid ", 33) == 0) {
        const char *spec = strchr(sv, ' ') + 1;
        int i, n = 0, opp;
        if (!Mtg_ValidSpecKnown(spec)) {
            *ok = 0;
            return 0;
        }
        if (!d)
            return 0;
        opp = ctx->controller ^ 1;
        for (i = 0; i < d->objCount; i++)
            n += d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, spec, ctx->source, opp);
        return n;
    }
    if (strcmp(sv, "PlayerCountOpponents$LowestCardsInHand") == 0 || strcmp(sv, "PlayerCountOpponents$HighestCardsInHand") == 0) {
        int i, n = 0;
        if (!d)
            return 0;
        for (i = 0; i < d->objCount; i++)
            n += d->objs[i].zone == MTG_ZONE_HAND && d->objs[i].owner == (ctx->controller ^ 1);
        return n;
    }
    if (strcmp(sv, "Count$YourLifeTotal") == 0)
        return d ? d->players[ctx->controller].life : 0;
    if (strcmp(sv, "Count$Domain") == 0) {   // basic land types among its controller's lands
        int i;
        uint8_t types = 0;
        if (!d)
            return 0;
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && d->objs[i].controller == ctx->controller)
                types |= Mtg_Card(d->objs[i].def)->intrinsicMana;
        return __builtin_popcount(types & 0x1f);
    }
    if (strcmp(sv, "TargetedPlayer$CardsInHand") == 0) {
        int i, n = 0;
        if (!d)
            return 0;
        if (ctx->e->target.kind != MTG_TGT_PLAYER)
            return 0;
        for (i = 0; i < d->objCount; i++)
            n += d->objs[i].zone == MTG_ZONE_HAND && d->objs[i].owner == ctx->e->target.id;
        return n;
    }
    if (strncmp(sv, "Count$Valid ", 12) == 0 || strncmp(sv, "Count$ValidGraveyard ", 21) == 0
        || strncmp(sv, "Count$ValidHand ", 16) == 0) {
        // "Count$Valid Forest.YouCtrl", "/Plus.1", "/Minus.2", "/Times.2", "/HalfUp", "/HalfDown" after it;
        // ValidGraveyard: in graveyards, ValidHand: in hands
        int i, count = 0, grave = sv[11] == 'G', hand = sv[11] == 'H', mod = 0, arg = 0;
        const char *spec = sv + (grave ? 21 : hand ? 16 : 12), *slash = strchr(spec, '/');
        char buf[96];
        if (slash) {
            const char *m = slash + 1;
            mod = strncmp(m, "Plus.", 5) == 0 ? 1 : strncmp(m, "Minus.", 6) == 0 ? 2 : strncmp(m, "Times.", 6) == 0 ? 3
                : strcmp(m, "HalfUp") == 0 ? 4 : strcmp(m, "HalfDown") == 0 ? 5 : 0;
            if (!mod || (size_t)(slash - spec) >= sizeof(buf)) {
                *ok = 0;
                return 0;
            }
            if (mod <= 3) {
                arg = atoi(strchr(m, '.') + 1);
                if (strspn(strchr(m, '.') + 1, "0123456789") != strlen(strchr(m, '.') + 1)) {
                    *ok = 0;
                    return 0;
                }
            }
            memcpy(buf, spec, (size_t)(slash - spec));
            buf[slash - spec] = 0;
            spec = buf;
        }
        if (!Mtg_ValidSpecKnown(spec)) {
            *ok = 0;
            return 0;
        }
        if (!d)
            return 0;
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].zone == (grave ? MTG_ZONE_GRAVEYARD : hand ? MTG_ZONE_HAND : MTG_ZONE_BATTLEFIELD)
                && Mtg_ValidObj(d, i, spec, ctx->source, ctx->controller))
                count++;
        return mod == 1 ? count + arg : mod == 2 ? count - arg : mod == 3 ? count * arg : mod == 4 ? (count + 1) / 2
             : mod == 5 ? count / 2 : count;
    }
    *ok = 0;
    return 0;
}

// ---- who or what an effect acts on (AbilityUtils.getDefinedCards / getDefinedPlayers)
#define MTG_MAX_DEFINED 4
typedef struct { int n; MtgTarget t[MTG_MAX_DEFINED]; } Defined;

static const char *const sDefinedKnown[] = {
    "You", "Opponent", "Player", "Self", "Targeted", "ParentTarget", "TargetedController", "TargetedPlayer",
    "Enchanted", "DelayTriggerRemembered", "DelayTriggerRememberedLKI",
};

// what set a triggered ability off (SpellAbility.getTriggeringObject: "Triggered" + the AbilityKey's name, its
// "LKICopy" the same here, "Controller" / "Owner" its player), by trigger mode
enum { TW_OBJ, TW_OBJ2, TW_PLAYER, TW_TARGET };
static const struct { const char *name; uint8_t mode, what; } sTriggered[] = {
    {"Card", MTG_TRIG_CHANGES_ZONE, TW_OBJ}, {"NewCard", MTG_TRIG_CHANGES_ZONE, TW_OBJ},
    {"Player", MTG_TRIG_PHASE, TW_PLAYER},
    {"Card", MTG_TRIG_SPELL_CAST, TW_OBJ}, {"Activator", MTG_TRIG_SPELL_CAST, TW_PLAYER},
    {"Card", MTG_TRIG_CYCLED, TW_OBJ},
    {"Source", MTG_TRIG_DAMAGE_DONE, TW_OBJ}, {"Target", MTG_TRIG_DAMAGE_DONE, TW_TARGET},
    {"Attacker", MTG_TRIG_ATTACKS, TW_OBJ}, {"Defender", MTG_TRIG_ATTACKS, TW_PLAYER},
    {"DefendingPlayer", MTG_TRIG_ATTACKS, TW_PLAYER},
    {"Attacker", MTG_TRIG_ATTACKER_BLOCKED, TW_OBJ}, {"Defender", MTG_TRIG_ATTACKER_BLOCKED, TW_PLAYER},
    {"DefendingPlayer", MTG_TRIG_ATTACKER_BLOCKED, TW_PLAYER},
    {"Attacker", MTG_TRIG_ATTACKER_BLOCKED_BY, TW_OBJ}, {"Blocker", MTG_TRIG_ATTACKER_BLOCKED_BY, TW_OBJ2},
    {"Blocker", MTG_TRIG_BLOCKS, TW_OBJ}, {"Attackers", MTG_TRIG_BLOCKS, TW_OBJ2},
    {"Attacker", MTG_TRIG_ATTACKER_UNBLOCKED, TW_OBJ}, {"Defender", MTG_TRIG_ATTACKER_UNBLOCKED, TW_PLAYER},
    {"DefendingPlayer", MTG_TRIG_ATTACKER_UNBLOCKED, TW_PLAYER},
};
enum { TS_NONE, TS_CONTROLLER, TS_OWNER };

// "TriggeredCardLKICopy" -> what, suffix; mode 0: any mode's
static int TriggeredName(int mode, const char *name, int *what, int *suffix)
{
    char buf[48];
    size_t n;
    unsigned i;
    if (strncmp(name, "Triggered", 9) != 0 || strlen(name + 9) >= sizeof(buf))
        return 0;
    strcpy(buf, name + 9);
    n = strlen(buf);
    *suffix = TS_NONE;
    if (n > 10 && strcmp(buf + n - 10, "Controller") == 0)
        buf[n -= 10] = 0, *suffix = TS_CONTROLLER;
    else if (n > 5 && strcmp(buf + n - 5, "Owner") == 0)
        buf[n -= 5] = 0, *suffix = TS_OWNER;
    if (n > 7 && strcmp(buf + n - 7, "LKICopy") == 0)
        buf[n -= 7] = 0;
    for (i = 0; i < sizeof(sTriggered) / sizeof(sTriggered[0]); i++)
        if ((mode == 0 || sTriggered[i].mode == mode) && strcmp(buf, sTriggered[i].name) == 0) {
            *what = sTriggered[i].what;
            return 1;
        }
    return 0;
}

int Mtg_TriggeredDefinedKnown(int mode, const char *name)
{
    int what, suffix;
    return TriggeredName(mode, name, &what, &suffix);
}

static void Add(Defined *out, int kind, int id)
{
    if (out->n < MTG_MAX_DEFINED) {
        out->t[out->n].kind = (uint8_t)kind;
        out->t[out->n].id = (uint16_t)id;
        out->n++;
    }
}

enum { DEF_PLAYER, DEF_CARD };   // what the effect acts on when nothing is targeted or Defined$

static void Triggered(const MtgDuel *d, const MtgResolveCtx *ctx, const char *def, Defined *out)
{
    const MtgStackEntry *e = ctx->e;
    int what, suffix, obj = MTG_NO_OBJ, player = 0xFF;
    if (e->trigger == 0xFFFF || !TriggeredName(gMtgTriggers[e->trigger].mode, def, &what, &suffix))
        return;
    if (what == TW_OBJ)
        obj = e->trigObj;
    else if (what == TW_OBJ2)
        obj = e->trigObj2;
    else if (what == TW_PLAYER)
        player = e->trigPlayer;
    else if (e->trigPlayer < MTG_PLAYERS)
        player = e->trigPlayer;
    else
        obj = e->trigObj2;
    if (suffix != TS_NONE && obj != MTG_NO_OBJ) {   // (its controller as it happened: the last known information)
        player = suffix == TS_OWNER ? d->objs[obj].owner : what == TW_OBJ ? e->trigCtrl : d->objs[obj].controller;
        obj = MTG_NO_OBJ;
    }
    if (obj != MTG_NO_OBJ)
        Add(out, MTG_TGT_OBJ, obj);
    else if (player < MTG_PLAYERS)
        Add(out, MTG_TGT_PLAYER, player);
}

static void DefinedBy(const MtgDuel *d, const MtgResolveCtx *ctx, const char *def, Defined *out);

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
    DefinedBy(d, ctx, def, out);
}

static void DefinedBy(const MtgDuel *d, const MtgResolveCtx *ctx, const char *def, Defined *out)
{
    const MtgTarget *t = &ctx->e->target;
    out->n = 0;
    if (strncmp(def, "Triggered", 9) == 0)
        Triggered(d, ctx, def, out);
    else if (strncmp(def, "DelayTriggerRemembered", 22) == 0) {   // (a delayed trigger's: what it remembered)
        if (ctx->e->trigObj != MTG_NO_OBJ && ctx->e->trigger != 0xFFFF)
            Add(out, MTG_TGT_OBJ, ctx->e->trigObj);
    } else if (strcmp(def, "You") == 0)
        Add(out, MTG_TGT_PLAYER, ctx->controller);
    else if (strcmp(def, "Opponent") == 0)
        Add(out, MTG_TGT_PLAYER, ctx->controller ^ 1);
    else if (strcmp(def, "Player") == 0) {
        Add(out, MTG_TGT_PLAYER, d->active);   // (in turn order, APNAP)
        Add(out, MTG_TGT_PLAYER, d->active ^ 1);
    } else if (strcmp(def, "Self") == 0)
        Add(out, MTG_TGT_OBJ, ctx->source);
    else if (strcmp(def, "Enchanted") == 0) {   // the creature its source (an Aura) is attached to
        int on = ctx->source >= 0 && ctx->source < d->objCount ? d->objs[ctx->source].attachedTo : MTG_NO_OBJ;
        if (on != MTG_NO_OBJ && d->objs[on].zone == MTG_ZONE_BATTLEFIELD)
            Add(out, MTG_TGT_OBJ, on);
    }
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

void Mtg_AddCounters(MtgDuel *d, int obj, int n)
{
    MtgObj *o = &d->objs[obj];
    if (!n)
        return;
    o->counters11 = (int16_t)(o->counters11 + n);
    Mtg_Event(d, MTG_EV_COUNTERS, obj, n, o->counters11);
}

void Mtg_Sacrifice(MtgDuel *d, int obj)
{
    if (d->objs[obj].zone == MTG_ZONE_BATTLEFIELD)
        Mtg_MoveTo(d, obj, MTG_ZONE_GRAVEYARD);
}

static int AskPick(MtgDuel *d, int player, const uint16_t *list, int n)
{
    int i;
    Mtg_AskChoice(d, player, MTG_CHOICE_PICK, NULL);
    for (i = 0; i < n && i < MTG_MAX_COMBAT; i++)
        d->query.candidates[i] = list[i];
    d->query.candidateCount = (uint8_t)(n < MTG_MAX_COMBAT ? n : MTG_MAX_COMBAT);
    return 0;
}

// each player of who chooses `amount` of their cards in `zone` matching spec (all of them when they have no more),
// one MTG_CHOICE_PICK at a time; d->resPicks counts the picks so far (player = resPicks / amount). done(d, obj) is
// what happens to each. Returns 1 when every one is done, 0 while it waits.
static int EachChooses(MtgDuel *d, const MtgResolveCtx *ctx, const Defined *who, int amount, int zone,
                       const char *spec, void (*done)(MtgDuel *, int))
{
    if (amount <= 0)
        return 1;
    for (;;) {
        uint16_t list[MTG_MAX_COMBAT];
        int pi = d->resPicks / amount, left, n = 0, p, i;
        if (pi >= who->n)
            return 1;
        p = who->t[pi].id;
        left = amount - d->resPicks % amount;
        for (i = 0; i < d->objCount && n < MTG_MAX_COMBAT; i++) {
            const MtgObj *o = &d->objs[i];
            if (o->zone != zone || (zone == MTG_ZONE_BATTLEFIELD ? o->controller : o->owner) != p)
                continue;
            if (spec && !Mtg_ValidObj(d, i, spec, ctx->source, ctx->controller))
                continue;
            if (zone == MTG_ZONE_LIBRARY) {   // (a library's copies of one card: one choice)
                int k, dup = 0;
                for (k = 0; k < n && !dup; k++)
                    dup = d->objs[list[k]].def == o->def;
                if (dup)
                    continue;
            }
            list[n++] = (uint16_t)i;
        }
        if (who->t[pi].kind != MTG_TGT_PLAYER || n == 0) {
            d->resPicks = (uint8_t)((pi + 1) * amount);
            continue;
        }
        if (n <= left) {   // (no choice: all of them)
            for (i = 0; i < n; i++)
                done(d, list[i]);
            d->resPicks = (uint8_t)((pi + 1) * amount);
            continue;
        }
        if (d->resChoice < 0)
            return AskPick(d, p, list, n);
        for (i = 0; i < n && list[i] != d->resChoice; i++)
            ;
        if (i < n)
            done(d, list[i]);
        d->resChoice = -1;
        d->resPicks++;
    }
}

static void DiscardOne(MtgDuel *d, int obj) { Mtg_MoveTo(d, obj, MTG_ZONE_GRAVEYARD); }

// Animate's Types$ ("Artifact,Creature,Assembly-Worker": the card types; subtypes are only words) -> MTG_T_* bits
static int AnimTypes(const char *s, int *ok)
{
    static const struct { const char *name; int bit; } sT[] = {
        {"Creature", MTG_T_CREATURE}, {"Artifact", MTG_T_ARTIFACT}, {"Land", MTG_T_LAND}, {"Enchantment", MTG_T_ENCHANTMENT}};
    char buf[96], *p = buf, *t;
    int bits = 0;
    unsigned k;
    *ok = 1;
    if (!s)
        return 0;
    if (strlen(s) >= sizeof(buf)) {
        *ok = 0;
        return 0;
    }
    strcpy(buf, s);
    while ((t = Mtg_Token(&p, ',')) != NULL) {
        int found = 0;
        for (k = 0; k < sizeof(sT) / sizeof(sT[0]); k++)
            if (strcmp(t, sT[k].name) == 0)
                bits |= sT[k].bit, found = 1;
        if (!found && !(t[0] >= 'A' && t[0] <= 'Z'))
            *ok = 0;   // (a creature type: a word, kept by nothing here)
    }
    return bits;
}

// Colors$ "Red" / "Green,White" -> MTG_COLOR_BIT mask (-1: unknown)
static int AnimColors(const char *s)
{
    static const char *const sNames[5] = {"White", "Blue", "Black", "Red", "Green"};
    char buf[64], *p = buf, *t;
    int mask = 0, k;
    if (!s)
        return 0;
    if (strlen(s) >= sizeof(buf))
        return -1;
    strcpy(buf, s);
    while ((t = Mtg_Token(&p, ',')) != NULL) {
        int found = 0;
        for (k = 0; k < 5; k++)
            if (strcmp(t, sNames[k]) == 0)
                mask |= MTG_COLOR_BIT(k), found = 1;
        if (strcmp(t, "Colorless") == 0)
            found = 1;
        if (!found)
            return -1;
    }
    return mask;
}

// "Graveyard,Hand" -> zone bits (0: one it doesn't know)
static unsigned ZoneList(const char *s)
{
    static const struct { const char *name; int zone; } sZones[] = {
        {"Battlefield", MTG_ZONE_BATTLEFIELD}, {"Graveyard", MTG_ZONE_GRAVEYARD}, {"Hand", MTG_ZONE_HAND},
        {"Exile", MTG_ZONE_EXILE}, {"Library", MTG_ZONE_LIBRARY}};
    unsigned bits = 0, k;
    char buf[64], *p = buf, *t;
    if (!s || strlen(s) >= sizeof(buf))
        return 0;
    strcpy(buf, s);
    while ((t = Mtg_Token(&p, ',')) != NULL) {
        unsigned found = 0;
        for (k = 0; k < sizeof(sZones) / sizeof(sZones[0]); k++)
            if (strcmp(t, sZones[k].name) == 0)
                found = 1u << sZones[k].zone;
        if (!found)
            return 0;
        bits |= found;
    }
    return bits;
}

// a search's find: to its destination (onto the battlefield under the searcher's control, tapped if it says)
static int sFoundZone, sFoundTapped, sFoundCtrl;
static void FoundOne(MtgDuel *d, int obj)
{
    if (sFoundZone == MTG_ZONE_BATTLEFIELD)
        d->objs[obj].controller = (uint8_t)sFoundCtrl;
    Mtg_MoveTo(d, obj, sFoundZone);
    if (sFoundZone == MTG_ZONE_BATTLEFIELD && sFoundTapped)
        Mtg_Tap(d, obj, 1);
}

// GainControlEffect's LoseControl$ ("EOT", "LeavesPlay,LoseControl", "Untap,...") as MTG_CTRL_* bits; -1: not known
static int LoseControlBits(const char *s)
{
    char buf[64], *tok, *save;
    int bits = 0;
    if (!s)
        return 0;
    if (strlen(s) >= sizeof(buf))
        return -1;
    strcpy(buf, s);
    for (save = buf, tok = Mtg_Token(&save, ','); tok; tok = Mtg_Token(&save, ',')) {
        while (*tok == ' ')
            tok++;
        if (strcmp(tok, "EOT") == 0)
            bits |= MTG_CTRL_EOT;
        else if (strcmp(tok, "LeavesPlay") == 0)
            bits |= MTG_CTRL_LEAVES;
        else if (strcmp(tok, "LoseControl") == 0)
            bits |= MTG_CTRL_LOSE;
        else if (strcmp(tok, "Untap") == 0)
            bits |= MTG_CTRL_UNTAP;
        else
            return -1;
    }
    return bits;
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
        if (k < 0) {   // (a Pump's own: landwalk, can't block / attack)
            static const char *const sBasic[5] = {"Plains", "Island", "Swamp", "Mountain", "Forest"};
            int b, found = 0;
            for (b = 0; b < 5; b++)
                if (strncmp(tok, "Landwalk:", 9) == 0 && strcmp(tok + 9, sBasic[b]) == 0)
                    mask |= 1u << (MTG_KWX_WALK_SHIFT + b), found = 1;
            if (strcmp(tok, "HIDDEN CARDNAME can't block.") == 0)
                mask |= MTG_KWX_CANT_BLOCK, found = 1;
            else if (strcmp(tok, "HIDDEN CARDNAME can't attack.") == 0)
                mask |= MTG_KWX_CANT_ATTACK, found = 1;
            else if (strcmp(tok, "HIDDEN CARDNAME can't attack or block.") == 0)
                mask |= MTG_KWX_CANT_ATTACK | MTG_KWX_CANT_BLOCK, found = 1;
            if (!found) {
                *ok = 0;
                return 0;
            }
            continue;
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

static int Resolve(MtgDuel *d, MtgResolveCtx *ctx, const MtgAbility *ab)
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
                Mtg_Destroy(d, who.t[i].id, Mtg_Param(ab, "NoRegen") != NULL);
        break;
    case MTG_API_DESTROY_ALL: {
        const char *cards = Mtg_Param(ab, "ValidCards");
        uint8_t doomed[MTG_MAX_OBJS];   // (chosen first, destroyed together)
        for (i = 0; i < d->objCount; i++)
            doomed[i] = d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, cards, ctx->source, ctx->controller);
        for (i = 0; i < d->objCount; i++)
            if (doomed[i])
                Mtg_Destroy(d, i, Mtg_Param(ab, "NoRegen") != NULL);
        break;
    }
    case MTG_API_CHANGE_ZONE_ALL: {   // every ChangeType$ card of Origin$ (a target player's only, with a target) at once
        unsigned from = ZoneList(Mtg_Param(ab, "Origin")), to = ZoneList(Mtg_Param(ab, "Destination"));
        const char *spec = Mtg_Param(ab, "ChangeType"), *sh = Mtg_Param(ab, "Shuffle");
        int zone = 0, only = -1, shuffled[MTG_PLAYERS] = {0, 0};
        uint8_t moving[MTG_MAX_OBJS];
        while (!(to & (1u << zone)))
            zone++;
        if (ab->tgt) {
            Targets(d, ctx, ab, DEF_PLAYER, &who);
            if (!who.n || who.t[0].kind != MTG_TGT_PLAYER)
                break;
            only = who.t[0].id;
        }
        for (i = 0; i < d->objCount; i++) {
            const MtgObj *o = &d->objs[i];
            int whose = o->zone == MTG_ZONE_BATTLEFIELD ? o->controller : o->owner;
            moving[i] = (from & (1u << o->zone)) && (only < 0 || whose == only)
                        && Mtg_ValidObj(d, i, spec, ctx->source, ctx->controller) && i != ctx->source;
        }
        for (i = 0; i < d->objCount; i++)
            if (moving[i]) {
                if (zone == MTG_ZONE_BATTLEFIELD)   // (onto the battlefield under its owner's control)
                    d->objs[i].controller = d->objs[i].owner;
                if (zone == MTG_ZONE_LIBRARY)
                    shuffled[d->objs[i].owner] = 1;
                Mtg_MoveTo(d, i, zone);
            }
        for (i = 0; i < MTG_PLAYERS; i++)
            if (shuffled[i] && sh && strcmp(sh, "True") == 0)
                Mtg_ShuffleLibrary(d, i);
        break;
    }
    case MTG_API_ANIMATE: {   // it becomes ... until end of turn (Duration$ Permanent: for good)
        const char *dur = Mtg_Param(ab, "Duration"), *pw = Mtg_Param(ab, "Power"), *tg = Mtg_Param(ab, "Toughness");
        int types = AnimTypes(Mtg_Param(ab, "Types"), &ok), colors = AnimColors(Mtg_Param(ab, "Colors"));
        uint32_t kw = KeywordList(Mtg_Param(ab, "Keywords"), &ok);
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++) {
            MtgObj *o;
            if (!OnBattlefield(d, &who.t[i]))
                continue;
            o = &d->objs[who.t[i].id];
            o->animUntil = (uint8_t)(dur && strcmp(dur, "Permanent") == 0 ? 2 : 1);
            o->animTypes = (uint8_t)types;
            o->animPower = (int8_t)(pw ? Mtg_Amount(d, ctx, card, pw, &ok) : MTG_ANIM_NO_PT);
            o->animToughness = (int8_t)(tg ? Mtg_Amount(d, ctx, card, tg, &ok) : MTG_ANIM_NO_PT);
            o->animColors = (uint8_t)(colors > 0 ? colors : 0);
            o->animOverwrite = (uint8_t)(Mtg_Param(ab, "OverwriteColors") != NULL || (colors >= 0 && Mtg_Param(ab, "Colors")));
            o->animKeywords = kw & ((1u << MTG_KW_COUNT) - 1);
            Mtg_Event(d, MTG_EV_COUNTERS, who.t[i].id, 0, o->counters11);   // (the table redraws it)
        }
        break;
    }
    case MTG_API_GAIN_CONTROL: {   // GainControlEffect: control changes in the control layer (state-based check)
        const char *nc = Mtg_Param(ab, "NewController"), *un = Mtg_Param(ab, "Untap");
        int bits = LoseControlBits(Mtg_Param(ab, "LoseControl")), p = ctx->controller;
        uint32_t kw = KeywordList(Mtg_Param(ab, "AddKWs"), &ok);
        if (nc && strcmp(nc, "You")) {
            Defined np;
            DefinedBy(d, ctx, nc, &np);
            if (!np.n)
                break;
            p = np.t[0].kind == MTG_TGT_PLAYER ? np.t[0].id : d->objs[np.t[0].id].controller;
        }
        if (ab->tgt && ctx->e->target.kind == MTG_TGT_PLAYER && Mtg_Param(ab, "Defined")) {
            p = ctx->e->target.id;   // ("target opponent gains control of CARDNAME": the target is the new controller)
            DefinedBy(d, ctx, Mtg_Param(ab, "Defined"), &who);
        } else
            Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++) {
            MtgObj *o;
            if (!OnBattlefield(d, &who.t[i]))
                continue;
            o = &d->objs[who.t[i].id];
            o->effCtrl = (uint8_t)(p + 1);
            o->effUntil = (uint8_t)(bits < 0 ? 0 : bits);
            o->effSource = (uint16_t)ctx->source;
            if (un)
                Mtg_Tap(d, who.t[i].id, 0);
            o->tempKeywords |= kw;
        }
        break;
    }
    case MTG_API_PREVENT_DAMAGE: {   // a shield on each: the next Amount$ damage to it this turn is prevented
        int n = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "Amount"), &ok);
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n && n > 0; i++) {
            uint8_t *sh = who.t[i].kind == MTG_TGT_PLAYER ? &d->players[who.t[i].id].prevent
                        : OnBattlefield(d, &who.t[i]) ? &d->objs[who.t[i].id].prevent : NULL;
            if (sh)
                *sh = (uint8_t)(*sh + n > 255 ? 255 : *sh + n);
        }
        break;
    }
    case MTG_API_DELAYED_TRIGGER: {   // DelayedTriggerEffect: set up now, fires once when its step begins
        const char *rem = Mtg_Param(ab, "RememberObjects"), *nt = Mtg_Param(ab, "NextTurn");
        MtgDelayed *dl;
        if (!ab->delayed || d->delayedCount >= MTG_MAX_DELAYED)
            break;
        dl = &d->delayed[d->delayedCount++];
        dl->trigger = (uint16_t)(ab->delayed - 1);
        dl->source = (uint16_t)ctx->source;
        dl->controller = (uint8_t)ctx->controller;
        dl->turn = d->turn;
        dl->nextTurn = (uint8_t)(nt && strcmp(nt, "True") == 0);
        dl->obj = MTG_NO_OBJ;
        if (rem) {
            DefinedBy(d, ctx, rem, &who);
            for (i = 0; i < who.n; i++)
                if (who.t[i].kind == MTG_TGT_OBJ) {
                    dl->obj = who.t[i].id;
                    break;
                }
        }
        break;
    }
    case MTG_API_REGENERATE:   // a shield on each: the next destruction this turn regenerates it instead
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (OnBattlefield(d, &who.t[i]) && d->objs[who.t[i].id].regen < 9) {
                d->objs[who.t[i].id].regen++;
                Mtg_Event(d, MTG_EV_REGENERATE, who.t[i].id, 1, d->objs[who.t[i].id].regen);
            }
        break;
    case MTG_API_TOKEN: {   // TokenEffect: the tokens enter under each owner's control
        const char *own = Mtg_Param(ab, "TokenOwner");
        int def = Mtg_TokenDef(Mtg_Param(ab, "TokenScript")), n = 1, k, p;
        if (Mtg_Param(ab, "TokenAmount"))
            n = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "TokenAmount"), &ok);
        DefinedBy(d, ctx, own ? own : "You", &who);
        for (i = 0; i < who.n && def >= 0; i++) {
            if (who.t[i].kind != MTG_TGT_PLAYER)
                continue;
            p = who.t[i].id;
            for (k = 0; k < n && k < 20 && d->objCount < MTG_MAX_OBJS; k++) {
                int obj = d->objCount++;
                MtgObj *o = &d->objs[obj];
                memset(o, 0, sizeof(*o));
                o->def = (uint16_t)def;
                o->owner = o->controller = (uint8_t)p;
                o->zone = MTG_ZONE_NONE;
                o->blocking = o->attachedTo = MTG_NO_OBJ;
                Mtg_MoveTo(d, obj, MTG_ZONE_BATTLEFIELD);
            }
        }
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
        if (Mtg_Param(ab, "ChangeType")) {   // a search of its controller's library (then a shuffle) or hand
            const char *nc = Mtg_Param(ab, "ChangeNum"), *tp = Mtg_Param(ab, "Tapped");
            int lib = strcmp(Mtg_Param(ab, "Origin"), "Library") == 0, n = nc ? Mtg_Amount(d, ctx, card, nc, &ok) : 1;
            Defined me;
            me.n = 1;
            me.t[0].kind = MTG_TGT_PLAYER;
            me.t[0].id = (uint16_t)ctx->controller;
            sFoundZone = strcmp(dest, "Hand") == 0 ? MTG_ZONE_HAND : strcmp(dest, "Battlefield") == 0 ? MTG_ZONE_BATTLEFIELD
                       : strcmp(dest, "Exile") == 0 ? MTG_ZONE_EXILE : MTG_ZONE_GRAVEYARD;
            sFoundTapped = tp && strcmp(tp, "True") == 0;
            sFoundCtrl = ctx->controller;
            if (!EachChooses(d, ctx, &me, n, lib ? MTG_ZONE_LIBRARY : MTG_ZONE_HAND, Mtg_Param(ab, "ChangeType"), FoundOne))
                return 0;
            if (lib)
                Mtg_ShuffleLibrary(d, ctx->controller);
            break;
        }
        int zone = strcmp(dest, "Exile") == 0 ? MTG_ZONE_EXILE : strcmp(dest, "Hand") == 0 ? MTG_ZONE_HAND
                 : strcmp(dest, "Library") == 0 ? MTG_ZONE_LIBRARY
                 : strcmp(dest, "Battlefield") == 0 ? MTG_ZONE_BATTLEFIELD : MTG_ZONE_GRAVEYARD;
        int from = strcmp(Mtg_Param(ab, "Origin"), "Graveyard") == 0 ? MTG_ZONE_GRAVEYARD : MTG_ZONE_BATTLEFIELD;
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++) {
            int obj = who.t[i].id;
            if (who.t[i].kind != MTG_TGT_OBJ || d->objs[obj].zone != from)
                continue;
            if (zone == MTG_ZONE_BATTLEFIELD)   // (onto the battlefield under its owner's control, or GainControl$)
                d->objs[obj].controller = Mtg_Param(ab, "GainControl") ? (uint8_t)ctx->controller : d->objs[obj].owner;
            Mtg_MoveTo(d, obj, zone);
        }
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
    case MTG_API_DISCARD: {   // Mode$ Random, TgtChoose (the player chooses), Hand
        const char *nc = Mtg_Param(ab, "NumCards"), *mode = Mtg_Param(ab, "Mode");
        int n = nc ? Mtg_Amount(d, ctx, card, nc, &ok) : 1;
        Targets(d, ctx, ab, DEF_PLAYER, &who);
        if (strcmp(mode, "TgtChoose") == 0)
            return EachChooses(d, ctx, &who, n, MTG_ZONE_HAND, Mtg_Param(ab, "DiscardValid"), DiscardOne);
        if (strcmp(mode, "Hand") == 0) {
            for (i = 0; i < who.n; i++) {
                int j;
                if (who.t[i].kind != MTG_TGT_PLAYER)
                    continue;
                for (j = 0; j < d->objCount; j++)
                    if (d->objs[j].zone == MTG_ZONE_HAND && d->objs[j].owner == who.t[i].id)
                        Mtg_MoveTo(d, j, MTG_ZONE_GRAVEYARD);
            }
            break;
        }
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
    case MTG_API_CHARM:    // (never on the stack itself: each mode is cast or activated on its own)
    case MTG_API_ATTACH:   // (the Aura enters attached as the spell finishes: mtg_game.c FinishResolve)
        break;
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
    case MTG_API_SACRIFICE: {   // SacrificeEffect: SacValid$ Self (its source), or each Defined$ player chooses
        const char *valid = Mtg_Param(ab, "SacValid"), *amt = Mtg_Param(ab, "Amount");
        if (!valid || strcmp(valid, "Self") == 0) {
            if (strcmp(Mtg_Param(ab, "Defined") ? Mtg_Param(ab, "Defined") : "Self", "Self") == 0
                && d->objs[ctx->source].controller == ctx->controller)
                Mtg_Sacrifice(d, ctx->source);
            break;
        }
        Targets(d, ctx, ab, DEF_PLAYER, &who);
        return EachChooses(d, ctx, &who, amt ? Mtg_Amount(d, ctx, card, amt, &ok) : 1, MTG_ZONE_BATTLEFIELD, valid,
                           Mtg_Sacrifice);
    }
    case MTG_API_PUT_COUNTER: {   // CountersPutEffect: +1/+1 and -1/-1 counters
        int n = Mtg_Amount(d, ctx, card, Mtg_Param(ab, "CounterNum"), &ok);
        if (strcmp(Mtg_Param(ab, "CounterType"), "M1M1") == 0)
            n = -n;
        Targets(d, ctx, ab, DEF_CARD, &who);
        for (i = 0; i < who.n; i++)
            if (OnBattlefield(d, &who.t[i]))
                Mtg_AddCounters(d, who.t[i].id, n);
        break;
    }
    }
    return 1;
}

// UnlessCost$ (AbilityUtils.handleUnlessCost): a mana cost, or an SVar counting the generic mana
static int UnlessCost(const MtgDuel *d, const MtgResolveCtx *ctx, const MtgAbility *ab, MtgManaCost *out)
{
    const char *v = Mtg_Param(ab, "UnlessCost");
    int ok;
    if (!v)
        return 0;
    if (strcmp(v, "X") != 0 && Mtg_Svar(ab->card, v)) {
        memset(out, 0, sizeof(*out));
        out->generic = (uint8_t)Mtg_Amount(d, ctx, ab->card, v, &ok);
        return ok;
    }
    return Mtg_ParseManaCost(v, out) && !out->x;
}

// one ability of a chain. Its UnlessCost$ first: each payer in turn may pay it, and if one does the ability does
// nothing (its sub-abilities still resolve). resStage: bit 7 settled, bit 6 paid, the rest the payer asked next.
int Mtg_ResolveStep(MtgDuel *d, MtgResolveCtx *ctx, int ability)
{
    const MtgAbility *ab = &gMtgAbilities[ability];
    MtgManaCost cost;
    if (Mtg_Param(ab, "UnlessCost") && !(d->resStage & 0x80)) {
        Defined payers;
        const char *who = Mtg_Param(ab, "UnlessPayer");
        const MtgCost *other = ab->unless ? &gMtgUnlessCosts[ab->unless - 1] : NULL;
        DefinedBy(d, ctx, who ? who : "TargetedController", &payers);
        if (other)
            cost = other->mana;
        else
            UnlessCost(d, ctx, ab, &cost);
        for (;;) {
            int pi = d->resStage & 0x3F, p;
            if (pi >= payers.n) {
                d->resStage = 0x80;
                break;
            }
            p = payers.t[pi].id;
            if (payers.t[pi].kind != MTG_TGT_PLAYER || !Mtg_CanPay(d, p, &cost, 0, MTG_NO_OBJ)
                || (other && !Mtg_CostPayable(d, p, other, ctx->source, MTG_NO_OBJ))) {
                d->resStage++;
                continue;
            }
            if (d->resChoice < 0) {
                Mtg_AskChoice(d, p, MTG_CHOICE_PAY, &cost);
                return 0;
            }
            if (d->resChoice == 1) {
                d->resStage = 0xC0;
                d->resChoice = -1;
                if (other) {   // (its sacrifices, discards, returns... are picked now: the answer goes on from there)
                    if (Mtg_PayUnlessCost(d, p, ability, ctx->source))
                        return 0;
                } else
                    Mtg_Pay(d, p, &cost, 0, MTG_NO_OBJ);
                break;
            }
            d->resChoice = -1;
            d->resStage++;
        }
    }
    if (d->resStage & 0x40)
        return 1;
    return Resolve(d, ctx, ab);
}

// ---- what a translation understands (else the card stays unsupported)
static const char *const sCommon[] = {
    "SP", "AB", "DB", "Cost", "ValidTgts", "TgtPrompt", "SubAbility", "SpellDescription", "StackDescription",
    "IsCurse", "PrecostDesc", "CostDesc", "TargetType", "Defined", "UnlessCost", "UnlessPayer", "ActivationZone",
    "TargetMin", "TargetMax",
};
static const char *const sApiParams[MTG_API_COUNT][10] = {
    [MTG_API_DEAL_DAMAGE] = {"NumDmg"},
    [MTG_API_DAMAGE_ALL] = {"NumDmg", "ValidCards", "ValidPlayers", "ValidDescription"},
    [MTG_API_DRAW] = {"NumCards"},
    [MTG_API_DESTROY] = {"NoRegen"},
    [MTG_API_DESTROY_ALL] = {"ValidCards", "NoRegen"},
    [MTG_API_CHANGE_ZONE_ALL] = {"Origin", "Destination", "ChangeType", "Shuffle", "UseAllOriginZones"},
    [MTG_API_ANIMATE] = {"Power", "Toughness", "Types", "Colors", "OverwriteColors", "Keywords", "RemoveCreatureTypes",
                         "Duration"},
    [MTG_API_PREVENT_DAMAGE] = {"Amount"},
    [MTG_API_GAIN_CONTROL] = {"NewController", "LoseControl", "Untap", "AddKWs"},
    [MTG_API_DELAYED_TRIGGER] = {"Mode", "Phase", "ValidPlayer", "Execute", "RememberObjects", "NextTurn",
                                 "TriggerDescription", "Secondary"},
    [MTG_API_GAIN_LIFE] = {"LifeAmount"},
    [MTG_API_LOSE_LIFE] = {"LifeAmount"},
    [MTG_API_PUMP] = {"NumAtt", "NumDef", "KW"},
    [MTG_API_PUMP_ALL] = {"NumAtt", "NumDef", "KW", "ValidCards"},
    [MTG_API_CHANGE_ZONE] = {"Origin", "Destination", "GainControl", "ChangeType", "ChangeNum", "ChangeTypeDesc", "Tapped",
                             "Mandatory", "ShuffleNonMandatory", "Optional"},
    [MTG_API_MANA] = {"Produced", "Amount"},
    [MTG_API_DISCARD] = {"NumCards", "Mode", "DiscardValid"},
    [MTG_API_SACRIFICE] = {"SacValid", "Amount", "SacMessage"},
    [MTG_API_PUT_COUNTER] = {"CounterType", "CounterNum"},
    [MTG_API_TOKEN] = {"TokenScript", "TokenOwner", "TokenAmount"},
    [MTG_API_CHARM] = {"Choices"},
};
static const char *const sPayersKnown[] = {
    "You", "Opponent", "Player", "Targeted", "TargetedController", "TargetedPlayer",
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
        if (!InList(key, sCommon, (int)(sizeof(sCommon) / sizeof(sCommon[0]))) && !InList(key, sApiParams[ab->api], 10)) {
            *why = key;
            return 0;
        }
    }
    if (Mtg_Param(ab, "TargetMin") || Mtg_Param(ab, "TargetMax")) {   // several targets: objects, spells and abilities
        const char *mn = Mtg_Param(ab, "TargetMin"), *mx = Mtg_Param(ab, "TargetMax");
        int lo = mn && mn[0] >= '0' && mn[0] <= '4' && !mn[1] ? mn[0] - '0' : mn && strcmp(mn, "X") == 0 ? 9 : -1;
        int hi = mx && mx[0] >= '1' && mx[0] <= '4' && !mx[1] ? mx[0] - '0' : mx && strcmp(mx, "X") == 0 ? 9 : -1;   // (9: X)
        const char *t = Mtg_Param(ab, "ValidTgts");
        static const uint8_t sApis[] = {MTG_API_PUMP, MTG_API_DESTROY, MTG_API_CHANGE_ZONE, MTG_API_TAP, MTG_API_UNTAP,
                                        MTG_API_DEAL_DAMAGE};
        unsigned k, apiOk = 0;
        for (k = 0; k < sizeof(sApis); k++)
            apiOk |= ab->api == sApis[k];
        if (lo < 0 || hi < 0 || hi < lo || !t || strstr(t, "Player") || strcmp(t, "Any") == 0 || Mtg_Param(ab, "TargetType")
            || (ab->kind != MTG_AB_SPELL && ab->kind != MTG_AB_ACTIVATED) || !apiOk) {
            *why = "TargetMin";
            return 0;
        }
    }
    v = Mtg_Param(ab, "Defined");
    if (v && !InList(v, sDefinedKnown, (int)(sizeof(sDefinedKnown) / sizeof(sDefinedKnown[0])))
        && !(ab->kind != MTG_AB_SPELL && ab->kind != MTG_AB_ACTIVATED && Mtg_TriggeredDefinedKnown(0, v))) {
        *why = "Defined";
        return 0;
    }
    if (Mtg_Param(ab, "UnlessCost")) {
        MtgManaCost cost;
        v = Mtg_Param(ab, "UnlessPayer");
        if (!ab->unless && !UnlessCost(NULL, NULL, ab, &cost)) {
            *why = "UnlessCost";
            return 0;
        }
        if (v && !InList(v, sPayersKnown, (int)(sizeof(sPayersKnown) / sizeof(sPayersKnown[0])))
            && !(ab->kind != MTG_AB_SPELL && ab->kind != MTG_AB_ACTIVATED && Mtg_TriggeredDefinedKnown(0, v))) {
            *why = "UnlessPayer";
            return 0;
        }
    } else if (Mtg_Param(ab, "UnlessPayer")) {
        *why = "UnlessPayer";
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
        if (ab->api == MTG_API_DISCARD) {
            v = Mtg_Param(ab, "Mode");
            if (!v || (strcmp(v, "Random") && strcmp(v, "TgtChoose") && strcmp(v, "Hand")))
                return 0;
            if ((v = Mtg_Param(ab, "DiscardValid")) && (strcmp(Mtg_Param(ab, "Mode"), "TgtChoose") || !Mtg_ValidSpecKnown(v)))
                return 0;
        }
        return 1;
    case MTG_API_DESTROY_ALL:
        return (v = Mtg_Param(ab, "ValidCards")) && Mtg_ValidSpecKnown(v);
    case MTG_API_ANIMATE: {
        const char *dur = Mtg_Param(ab, "Duration"), *rc = Mtg_Param(ab, "RemoveCreatureTypes");
        AnimTypes(Mtg_Param(ab, "Types"), &ok);
        if (!ok || AnimColors(Mtg_Param(ab, "Colors")) < 0 || (dur && strcmp(dur, "Permanent")) || (rc && strcmp(rc, "True")))
            return 0;
        KeywordList(Mtg_Param(ab, "Keywords"), &ok);
        if (!ok || !AmountKnown(ab, "Power", 0) || !AmountKnown(ab, "Toughness", 0))
            return 0;
        if (Mtg_Param(ab, "Power") && !Mtg_Param(ab, "Toughness"))
            return 0;
        return ab->tgt || Mtg_Param(ab, "Defined");
    }
    case MTG_API_PREVENT_DAMAGE:
        return AmountKnown(ab, "Amount", 1) && (ab->tgt || Mtg_Param(ab, "Defined"));
    case MTG_API_GAIN_CONTROL: {
        const char *nc = Mtg_Param(ab, "NewController"), *un = Mtg_Param(ab, "Untap");
        KeywordList(Mtg_Param(ab, "AddKWs"), &ok);
        if (!ok || LoseControlBits(Mtg_Param(ab, "LoseControl")) < 0 || (un && strcmp(un, "True")))
            return 0;
        if (nc && strcmp(nc, "You") && strcmp(nc, "ParentTarget")
            && !(ab->kind != MTG_AB_SPELL && ab->kind != MTG_AB_ACTIVATED && Mtg_TriggeredDefinedKnown(0, nc)))
            return 0;
        return ab->tgt || Mtg_Param(ab, "Defined");
    }
    case MTG_API_DELAYED_TRIGGER: {
        const char *rem = Mtg_Param(ab, "RememberObjects");
        if (!ab->delayed)
            return 0;
        if (rem && strcmp(rem, "Self") && strcmp(rem, "Targeted") && strcmp(rem, "ParentTarget")
            && !(ab->kind != MTG_AB_SPELL && ab->kind != MTG_AB_ACTIVATED && Mtg_TriggeredDefinedKnown(0, rem)))
            return 0;
        return 1;
    }
    case MTG_API_CHANGE_ZONE_ALL: {
        const char *o = Mtg_Param(ab, "Origin"), *dz = Mtg_Param(ab, "Destination");
        if (!o || !dz || !ZoneList(o) || !ZoneList(dz) || (ZoneList(dz) & (ZoneList(dz) - 1)) || (ZoneList(o) & ZoneList(dz))
            || (ZoneList(dz) & (1u << MTG_ZONE_STACK)) || !(v = Mtg_Param(ab, "ChangeType")) || !Mtg_ValidSpecKnown(v))
            return 0;
        if (ab->tgt && (strstr(ab->tgt, "Creature") || strstr(ab->tgt, "Card") || strstr(ab->tgt, "Permanent")))
            return 0;   // (a target player only: "target player's graveyard")
        return !(v = Mtg_Param(ab, "Shuffle")) || strcmp(v, "True") == 0;
    }
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
        int grave = o && strcmp(o, "Graveyard") == 0;
        if ((v = Mtg_Param(ab, "ChangeType"))) {   // a search (Hidden: the library or the hand) for ChangeNum$ cards
            const char *n = Mtg_Param(ab, "ChangeNum");
            if (!o || (strcmp(o, "Library") && strcmp(o, "Hand")) || !dz || ab->tgt || Mtg_Param(ab, "Defined")
                || Mtg_Param(ab, "GainControl") || !Mtg_ValidSpecKnown(v) || (n && !AmountKnown(ab, "ChangeNum", 0)))
                return 0;
            if (strcmp(dz, "Hand") && strcmp(dz, "Battlefield") && strcmp(dz, "Graveyard") && strcmp(dz, "Exile"))
                return 0;
            return strcmp(o, dz) != 0 && (!(v = Mtg_Param(ab, "Tapped")) || strcmp(v, "True") == 0);
        }
        if (Mtg_Param(ab, "ChangeNum") || Mtg_Param(ab, "Tapped") || Mtg_Param(ab, "Mandatory")
            || Mtg_Param(ab, "ShuffleNonMandatory") || Mtg_Param(ab, "Optional"))
            return 0;
        if (!o || (strcmp(o, "Battlefield") != 0 && !grave) || !dz || strcmp(o, dz) == 0)
            return 0;
        if (strcmp(dz, "Exile") && strcmp(dz, "Hand") && strcmp(dz, "Graveyard") && strcmp(dz, "Library")
            && !(grave && strcmp(dz, "Battlefield") == 0))
            return 0;
        if (Mtg_Param(ab, "GainControl") && strcmp(dz, "Battlefield"))
            return 0;
        return ab->tgt || Mtg_Param(ab, "Defined");
    }
    case MTG_API_SACRIFICE:
        if ((v = Mtg_Param(ab, "SacValid")) && strcmp(v, "Self") && !Mtg_ValidSpecKnown(v))
            return 0;
        if ((!v || strcmp(v, "Self") == 0) && (v = Mtg_Param(ab, "Defined")) && strcmp(v, "Self"))
            return 0;   // (Defined$ cards to sacrifice: not yet)
        return AmountKnown(ab, "Amount", 0) && !ab->tgt;
    case MTG_API_PUT_COUNTER:
        v = Mtg_Param(ab, "CounterType");
        return v && (strcmp(v, "P1P1") == 0 || strcmp(v, "M1M1") == 0) && AmountKnown(ab, "CounterNum", 1);
    case MTG_API_MANA:
        if ((Mtg_Param(ab, "SubAbility") && ab->kind != MTG_AB_ACTIVATED) || !AmountKnown(ab, "Amount", 0))
            return 0;
        v = Mtg_Param(ab, "Produced");
        if (!v || Mtg_ProducedColors(v) <= 0)
            return 0;
        if (ab->kind != MTG_AB_ACTIVATED && strchr(v, ' '))
            return 0;   // (a spell choosing its color: not yet)
        return 1;
    case MTG_API_COUNTER:
        return ab->tgtStack;
    case MTG_API_TOKEN:
        v = Mtg_Param(ab, "TokenOwner");
        if (v && !InList(v, sPayersKnown, (int)(sizeof(sPayersKnown) / sizeof(sPayersKnown[0])))
            && !(ab->kind != MTG_AB_SPELL && ab->kind != MTG_AB_ACTIVATED && Mtg_TriggeredDefinedKnown(0, v)))
            return 0;
        return Mtg_TokenDef(Mtg_Param(ab, "TokenScript")) >= 0 && AmountKnown(ab, "TokenAmount", 0);
    }
    return 1;
}
