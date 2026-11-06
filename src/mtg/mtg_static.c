// Moemon Kai: static abilities (StaticAbility.java, StaticAbilityContinuous.java, StaticAbilityCantBlock / CantAttack /
// CantBlockBy). "S:Mode$ Continuous"
// that changes the power, toughness and keywords of the permanents it affects ("Affected$ Creature.Merfolk+Other |
// AddPower$ 1 | AddToughness$ 1 | AddKeyword$ Islandwalk"): lords, anthems, Crusade, Bad Moon, Absolute Law... The
// layers as the rules have them for these: a SetPower / SetToughness (7b, the newest wins), then every AddPower /
// AddToughness and the until-end-of-turn pumps (7c), then the counters (7d). Its source works from the battlefield and
// affects permanents on the battlefield. AffectedDefined$ Self: only its own card (an Affected$ beside it is then a
// condition on that card: "as long as it's untapped"). IsPresent$ (with PresentCompare$ GE / EQ / LE n, GE1 by default):
// only while the battlefield has that many of those. CantBlock / CantAttack (ValidCard$) and CantBlockBy (ValidAttacker$
// can't be blocked by ValidBlocker$) forbid what they say. Anything else a static ability says keeps the card out for now.
#include <stdlib.h>
#include <string.h>
#include "mtg_internal.h"

MtgStatic *gMtgStatics;
static int sStaticCount, sStaticCap;
static int sDepth;   // (an Affected$ that looks at power or keywords sees them without the statics: no loops)

static const char *ParamOf(int first, int count, const char *key)
{
    int i;
    for (i = 0; i < count; i++)
        if (strcmp(gMtgParams[first + i].key, key) == 0)
            return gMtgParams[first + i].value;
    return NULL;
}

static int Integer(const char *s, int *out)
{
    char *end;
    long v;
    if (!s || !*s)
        return 0;
    v = strtol(s, &end, 10);
    if (*end)
        return 0;
    *out = (int)v;
    return 1;
}

// "Black..." starts with "black" (any case)
static int SameWord(const char *s, const char *word)
{
    for (; *word; s++, word++)
        if ((*s | 0x20) != *word)
            return 0;
    return 1;
}

// AddPower$ X (or -X): its SVar a count the static can take at any time (Count$Valid ..., Number$, Count$CardPower)
static int CountedAmount(int cardId, const char *s)
{
    const char *sv;
    int ok;
    if (s[0] == '-' || s[0] == '+')
        s++;
    if (!(sv = Mtg_Svar(cardId, s)))
        return 0;
    if (strncmp(sv, "Count$Valid ", 12) != 0 && strncmp(sv, "Number$", 7) != 0 && strcmp(sv, "Count$CardPower") != 0
        && strcmp(sv, "Count$CardToughness") != 0)
        return 0;
    Mtg_Amount(NULL, NULL, cardId, s, &ok);
    return ok;
}

// "Flying & First Strike", "Islandwalk", "Protection from red": into the static; 0 if one isn't known
static int Keywords(MtgStatic *st, const char *text)
{
    static const char *const sWalk[5] = {"Plainswalk", "Islandwalk", "Swampwalk", "Mountainwalk", "Forestwalk"};
    static const char *const sBasic[5] = {"Plains", "Island", "Swamp", "Mountain", "Forest"};
    static const char *const sProt[5] = {"white", "blue", "black", "red", "green"};
    char buf[160], *p = buf, *k;
    if (strlen(text) >= sizeof(buf))
        return 0;
    strcpy(buf, text);
    while ((k = Mtg_Token(&p, '&')) != NULL) {
        int kw, i, found = 0;
        while (*k == ' ')
            k++;
        for (i = (int)strlen(k); i > 0 && k[i - 1] == ' '; i--)
            k[i - 1] = 0;
        if ((kw = Mtg_KeywordByName(k)) >= 0) {
            st->addKeywords |= MTG_KW_BIT(kw);
            continue;
        }
        for (i = 0; i < 5; i++) {
            if (strcmp(k, sWalk[i]) == 0 || (strncmp(k, "Landwalk:", 9) == 0 && strcmp(k + 9, sBasic[i]) == 0))
                st->addLandwalk |= MTG_COLOR_BIT(i), found = 1;
            if (strncmp(k, "Protection from ", 16) == 0 && strcmp(k + 16, sProt[i]) == 0)
                st->addProtection |= MTG_COLOR_BIT(i), found = 1;
            // the Wards' "Protection:Card.Black:black:Card.CardUID_HostCardUID" (the Aura itself stays on)
            if (strncmp(k, "Protection:Card.", 16) == 0 && SameWord(k + 16, sProt[i])
                && k[16 + strlen(sProt[i])] == ':')
                st->addProtection |= MTG_COLOR_BIT(i), found = 1;
        }
        if (!found)
            return 0;
    }
    return 1;
}

// S: line text -> a static of card c; a part it doesn't translate makes the card unsupported
static void ParseOne(MtgCardDef *c, int cardId, char *text);

// The split texts of "Mode$ A,B" lines: the statics point into them, so they live as long as the pool
static char **sKept;
static int sKeptCount;

static char *Kept(size_t len)
{
    char *t = malloc(len);
    sKept = realloc(sKept, (size_t)(sKeptCount + 1) * sizeof(char *));
    sKept[sKeptCount++] = t;
    return t;
}

// S: line text -> statics of card c ("Mode$ CantAttack,CantBlock": one of each); a part it doesn't translate makes the
// card unsupported
void Mtg_ParseStatic(MtgCardDef *c, int cardId, char *text)
{
    char *mode = strstr(text, "Mode$"), *comma, *end;
    if (mode && (end = strchr(mode, '|')) != NULL && (comma = strchr(mode, ',')) != NULL && comma < end) {
        size_t len = strlen(text);
        char *a = Kept(len + 1), *b = Kept(len + 1);
        size_t pre = (size_t)(mode + 5 - text);
        // the first mode: text up to "Mode$", the first name, the rest from the '|'; the second likewise
        memcpy(a, text, pre);
        a[pre] = 0;
        strncat(a, mode + 5, (size_t)(comma - mode - 5));
        strcat(a, " ");
        strcat(a, end);
        memcpy(b, text, pre);
        b[pre] = 0;
        strncat(b, comma + 1, (size_t)(end - comma - 1));
        strcat(b, end);
        if (strchr(b + pre, ',') && strchr(b + pre, ',') < strchr(b + pre, '|')) {
            Mtg_Unsupported(c, "static ability");
            return;
        }
        ParseOne(c, cardId, a);
        ParseOne(c, cardId, b);
        return;
    }
    ParseOne(c, cardId, text);
}

static void ParseOne(MtgCardDef *c, int cardId, char *text)
{
    static const char *const sKnown[] = {"Mode", "Affected", "AddPower", "AddToughness", "AddKeyword", "SetPower",
                                         "SetToughness", "Description", "AffectedZone", "EffectZone", "AffectedDefined",
                                         "IsPresent", "PresentCompare", "ValidCard", "ValidAttacker", "ValidBlocker", "CharacteristicDefining",
                                         "Activator", "Type", "Amount", "Cost"};
    MtgStatic st;
    int first, count, i, v;
    const char *s;
    memset(&st, 0, sizeof(st));
    st.card = (uint16_t)cardId;
    st.setPower = st.setToughness = MTG_STATIC_NO_SET;
    count = Mtg_ParseParams(text, &first);
    for (i = 0; i < count; i++) {
        unsigned k;
        for (k = 0; k < sizeof(sKnown) / sizeof(sKnown[0]); k++)
            if (strcmp(gMtgParams[first + i].key, sKnown[k]) == 0)
                break;
        if (k == sizeof(sKnown) / sizeof(sKnown[0])) {
            Mtg_Unsupported(c, "static ability");
            return;
        }
    }
    s = ParamOf(first, count, "Mode");
    st.mode = !s ? 0xFF : strcmp(s, "Continuous") == 0 ? MTG_STATIC_CONTINUOUS : strcmp(s, "CantBlock") == 0 ? MTG_STATIC_CANT_BLOCK
            : strcmp(s, "CantAttack") == 0 ? MTG_STATIC_CANT_ATTACK : strcmp(s, "CantBlockBy") == 0 ? MTG_STATIC_CANT_BLOCK_BY
            : strcmp(s, "RaiseCost") == 0 ? MTG_STATIC_RAISE_COST : strcmp(s, "ReduceCost") == 0 ? MTG_STATIC_REDUCE_COST : 0xFF;
    if ((s = ParamOf(first, count, "CharacteristicDefining"))) {   // */*: on the card itself, wherever it is
        if (strcmp(s, "True") != 0 || st.mode != MTG_STATIC_CONTINUOUS || ParamOf(first, count, "Affected")
            || ParamOf(first, count, "AffectedDefined") || !c->starPT)
            goto unknown;
        st.self = 1;
        if ((s = ParamOf(first, count, "EffectZone")) && strcmp(s, "All") != 0)
            goto unknown;
    } else if ((s = ParamOf(first, count, "EffectZone")) && strcmp(s, "Battlefield") != 0)
        goto unknown;
    if (st.mode == 0xFF || ((s = ParamOf(first, count, "AffectedZone")) && strcmp(s, "Battlefield") != 0))
        goto unknown;
    if ((s = ParamOf(first, count, "AffectedDefined"))) {
        if (st.mode != MTG_STATIC_CONTINUOUS || (strcmp(s, "Self") != 0 && strcmp(s, "Enchanted") != 0)
            || (strcmp(s, "Enchanted") == 0 && !c->enchant))
            goto unknown;
        st.self = s[0] == 'S';
        st.enchanted = s[0] == 'E';
    }
    st.affected = ParamOf(first, count, st.mode == MTG_STATIC_CONTINUOUS ? "Affected"
                                      : st.mode == MTG_STATIC_CANT_BLOCK_BY ? "ValidAttacker" : "ValidCard");
    if (st.mode == MTG_STATIC_CANT_BLOCK_BY && (st.blocker = ParamOf(first, count, "ValidBlocker")) != NULL
        && !Mtg_ValidSpecKnown(st.blocker))
        goto unknown;   // (none: it can't be blocked at all)
    if ((!st.affected && !st.self && !st.enchanted) || (st.affected && !Mtg_ValidSpecKnown(st.affected)))
        goto unknown;
    if (st.mode == MTG_STATIC_RAISE_COST || st.mode == MTG_STATIC_REDUCE_COST) {
        // spells only; "Amount$ n" more or less generic mana, or "Cost$ W" (raise) one more colored symbol
        static const char sPips[] = "WUBRG";
        if (!(s = ParamOf(first, count, "Type")) || strcmp(s, "Spell") != 0)
            goto unknown;
        s = ParamOf(first, count, "Activator");
        st.activator = !s || strcmp(s, "Player") == 0 ? 0 : strcmp(s, "You") == 0 ? 1 : strcmp(s, "Opponent") == 0 ? 2 : 0xFF;
        if (st.activator == 0xFF)
            goto unknown;
        if ((s = ParamOf(first, count, "Amount"))) {
            if (!Integer(s, &v) || v < 0 || v > 20)
                goto unknown;
            st.costGeneric = (uint8_t)v;
        } else if ((s = ParamOf(first, count, "Cost"))) {
            if (Integer(s, &v) && v >= 0 && v <= 20)
                st.costGeneric = (uint8_t)v;
            else if (st.mode == MTG_STATIC_RAISE_COST && strlen(s) == 1 && strchr(sPips, s[0]))
                st.costPip = (uint8_t)(strchr(sPips, s[0]) - sPips + 1);
            else
                goto unknown;
        } else
            goto unknown;
    } else if (ParamOf(first, count, "Activator") || ParamOf(first, count, "Type") || ParamOf(first, count, "Amount")
               || ParamOf(first, count, "Cost"))
        goto unknown;
    if ((st.present = ParamOf(first, count, "IsPresent")) && !Mtg_ValidSpecKnown(st.present))
        goto unknown;
    st.presentNum = 1;
    if ((s = ParamOf(first, count, "PresentCompare"))) {
        if (!st.present || strlen(s) < 3 || !Integer(s + 2, &v) || v < 0 || v > 255)
            goto unknown;
        st.presentOp = strncmp(s, "GE", 2) == 0 ? 0 : strncmp(s, "EQ", 2) == 0 ? 1 : strncmp(s, "LE", 2) == 0 ? 2 : 0xFF;
        if (st.presentOp == 0xFF)
            goto unknown;
        st.presentNum = (uint8_t)v;
    }
    if ((s = ParamOf(first, count, "AddPower")) && !(Integer(s, &v) && (st.addPower = (int8_t)v, 1))) {
        if (!CountedAmount(cardId, s))
            goto unknown;
        st.powerX = s;
    }
    if ((s = ParamOf(first, count, "AddToughness")) && !(Integer(s, &v) && (st.addToughness = (int8_t)v, 1))) {
        if (!CountedAmount(cardId, s))
            goto unknown;
        st.toughX = s;
    }
    if ((s = ParamOf(first, count, "SetPower")) && !(Integer(s, &v) && (st.setPower = (int16_t)v, 1))) {
        if (!CountedAmount(cardId, s))
            goto unknown;
        st.setPowerX = s;
    }
    if ((s = ParamOf(first, count, "SetToughness")) && !(Integer(s, &v) && (st.setToughness = (int16_t)v, 1))) {
        if (!CountedAmount(cardId, s))
            goto unknown;
        st.setToughX = s;
    }
    if ((s = ParamOf(first, count, "AddKeyword")) && !Keywords(&st, s))
        goto unknown;
    if (c->staticCount >= sizeof(c->statics) / sizeof(c->statics[0]))
        goto unknown;
    if (sStaticCount + 1 > sStaticCap) {
        sStaticCap = sStaticCap ? sStaticCap * 2 : 256;
        gMtgStatics = realloc(gMtgStatics, (size_t)sStaticCap * sizeof(MtgStatic));
    }
    gMtgStatics[sStaticCount] = st;
    c->statics[c->staticCount++] = (uint16_t)sStaticCount++;
    return;
unknown:
    Mtg_Unsupported(c, "static ability");
}

// R: lines the engine plays as statics: "enters the battlefield tapped" (Event$ Moved, ReplaceWith$ an SVar that taps
// it: ETBTapped) and "doesn't untap during its controller's untap step" (Event$ Untap, Layer$ CantHappen)
void Mtg_ParseReplacement(MtgCardDef *c, int cardId, char *text)
{
    static const char *const sKnown[] = {"Event", "ValidCard", "Destination", "ReplaceWith", "ReplacementResult",
                                         "Description", "ActiveZones", "Layer", "ValidStepTurnToController"};
    MtgStatic st;
    int first, count, i;
    const char *s, *ev;
    memset(&st, 0, sizeof(st));
    st.card = (uint16_t)cardId;
    st.setPower = st.setToughness = MTG_STATIC_NO_SET;
    count = Mtg_ParseParams(text, &first);
    for (i = 0; i < count; i++) {
        unsigned k;
        for (k = 0; k < sizeof(sKnown) / sizeof(sKnown[0]); k++)
            if (strcmp(gMtgParams[first + i].key, sKnown[k]) == 0)
                break;
        if (k == sizeof(sKnown) / sizeof(sKnown[0]))
            goto unknown;
    }
    ev = ParamOf(first, count, "Event");
    st.affected = ParamOf(first, count, "ValidCard");
    if (!ev || !st.affected || !Mtg_ValidSpecKnown(st.affected))
        goto unknown;
    if ((s = ParamOf(first, count, "ActiveZones")) && strcmp(s, "Battlefield") != 0)
        goto unknown;
    if (strcmp(st.affected, "Card.Self") != 0 && !ParamOf(first, count, "ActiveZones"))
        goto unknown;   // (another card's: only from the battlefield)
    if (strcmp(ev, "Moved") == 0) {
        const char *sv;
        if (!(s = ParamOf(first, count, "Destination")) || strcmp(s, "Battlefield") != 0
            || !(s = ParamOf(first, count, "ReplaceWith")) || !(sv = Mtg_Svar(cardId, s)) || ParamOf(first, count, "Layer")
            || ParamOf(first, count, "ValidStepTurnToController"))
            goto unknown;
        // "DB$ Tap | Defined$ Self | ETB$ True" (Defined$ ReplacedCard for another card's)
        if (strcmp(sv, "DB$ Tap | Defined$ Self | ETB$ True") != 0 && strcmp(sv, "DB$ Tap | ETB$ True | Defined$ ReplacedCard") != 0
            && strcmp(sv, "DB$ Tap | Defined$ ReplacedCard | ETB$ True") != 0)
            goto unknown;
        st.mode = MTG_STATIC_ETB_TAPPED;
    } else if (strcmp(ev, "Untap") == 0) {
        if (!(s = ParamOf(first, count, "Layer")) || strcmp(s, "CantHappen") != 0
            || !(s = ParamOf(first, count, "ValidStepTurnToController")) || strcmp(s, "You") != 0
            || ParamOf(first, count, "ReplaceWith") || ParamOf(first, count, "Destination"))
            goto unknown;
        st.mode = MTG_STATIC_NO_UNTAP;
    } else
        goto unknown;
    if (c->staticCount >= sizeof(c->statics) / sizeof(c->statics[0]))
        goto unknown;
    if (sStaticCount + 1 > sStaticCap) {
        sStaticCap = sStaticCap ? sStaticCap * 2 : 256;
        gMtgStatics = realloc(gMtgStatics, (size_t)sStaticCap * sizeof(MtgStatic));
    }
    gMtgStatics[sStaticCount] = st;
    c->statics[c->staticCount++] = (uint16_t)sStaticCount++;
    return;
unknown:
    Mtg_Unsupported(c, "replacement effect");
}

// IsPresent$: does the battlefield have enough of them now (relative to the static's source and its controller)?
static int Present(const MtgDuel *d, const MtgStatic *st, int src)
{
    int i, n = 0;
    if (!st->present)
        return 1;
    for (i = 0; i < d->objCount; i++)
        if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD && Mtg_ValidObj(d, i, st->present, src, d->objs[src].controller))
            n++;
    return st->presentOp == 0 ? n >= st->presentNum : st->presentOp == 1 ? n == st->presentNum : n <= st->presentNum;
}

// Does the static (from SRC) apply to OBJ?
static int Applies(const MtgDuel *d, const MtgStatic *st, int src, int obj)
{
    if (st->self && obj != src)
        return 0;
    if (st->enchanted && obj != d->objs[src].attachedTo)
        return 0;
    if (st->affected && !Mtg_ValidObj(d, obj, st->affected, src, d->objs[src].controller))
        return 0;
    return Present(d, st, src);
}

// What every static on the battlefield does to OBJ now
void Mtg_StaticMods(const MtgDuel *d, int obj, MtgStaticMods *m)
{
    uint32_t setPowerSeq = 0, setToughSeq = 0;
    int s, k;
    memset(m, 0, sizeof(*m));
    m->setPower = m->setToughness = MTG_STATIC_NO_SET;
    if (sDepth || d->objs[obj].zone != MTG_ZONE_BATTLEFIELD)
        return;
    sDepth++;
    for (s = 0; s < d->objCount; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *c;
        if (src->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if (st->mode != MTG_STATIC_CONTINUOUS || !Applies(d, st, s, obj))
                continue;
            m->addPower += st->addPower;
            m->addToughness += st->addToughness;
            if (st->powerX || st->toughX) {   // (counted now, as Forge's layer 7c does each time)
                MtgStackEntry e;
                MtgResolveCtx ctx;
                int ok;
                memset(&e, 0, sizeof(e));
                ctx.e = &e;
                ctx.controller = src->controller;
                ctx.source = s;
                if (st->powerX)
                    m->addPower += Mtg_Amount(d, &ctx, st->card, st->powerX, &ok);
                if (st->toughX)
                    m->addToughness += Mtg_Amount(d, &ctx, st->card, st->toughX, &ok);
            }
            m->keywords |= st->addKeywords;
            m->landwalk |= st->addLandwalk;
            m->protection |= st->addProtection;
            if (st->setPower != MTG_STATIC_NO_SET && src->seq + 1 > setPowerSeq)
                m->setPower = st->setPower, setPowerSeq = src->seq + 1;
            if (st->setToughness != MTG_STATIC_NO_SET && src->seq + 1 > setToughSeq)
                m->setToughness = st->setToughness, setToughSeq = src->seq + 1;
            if (st->setPowerX || st->setToughX) {   // (a */* creature's own: counted now; any later set wins)
                MtgStackEntry e;
                MtgResolveCtx ctx;
                int ok;
                memset(&e, 0, sizeof(e));
                ctx.e = &e;
                ctx.controller = src->controller;
                ctx.source = s;
                if (st->setPowerX && setPowerSeq == 0)
                    m->setPower = Mtg_Amount(d, &ctx, st->card, st->setPowerX, &ok), setPowerSeq = 1;
                if (st->setToughX && setToughSeq == 0)
                    m->setToughness = Mtg_Amount(d, &ctx, st->card, st->setToughX, &ok), setToughSeq = 1;
            }
        }
    }
    sDepth--;
}

// CantBlock / CantAttack: does a static forbid OBJ? CantBlockBy: may OBJ (attacking) not be blocked by OTHER?
int Mtg_StaticForbids(const MtgDuel *d, int mode, int obj, int other)
{
    int s, k, forbid = 0;
    if (sDepth)
        return 0;
    sDepth++;
    for (s = 0; s < d->objCount && !forbid; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *c;
        if (src->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount && !forbid; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if (st->mode != mode || !Applies(d, st, s, obj))
                continue;
            if (mode == MTG_STATIC_CANT_BLOCK_BY && st->blocker && !Mtg_ValidObj(d, other, st->blocker, s, src->controller))
                continue;
            forbid = 1;
        }
    }
    sDepth--;
    return forbid;
}

// A spell's mana cost as its caster pays it now: Raise / ReduceCost from the battlefield (a reduction takes generic
// mana only, never below none)
void Mtg_SpellCost(const MtgDuel *d, int player, int obj, MtgManaCost *out)
{
    int s, k, raise = 0, reduce = 0;
    *out = Mtg_Card(d->objs[obj].def)->cost;
    if (sDepth)
        return;
    sDepth++;
    for (s = 0; s < d->objCount; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *c;
        if (src->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if ((st->mode != MTG_STATIC_RAISE_COST && st->mode != MTG_STATIC_REDUCE_COST)
                || (st->activator == 1 && player != src->controller) || (st->activator == 2 && player == src->controller)
                || !Mtg_ValidObj(d, obj, st->affected, s, src->controller) || !Present(d, st, s))
                continue;
            if (st->mode == MTG_STATIC_RAISE_COST) {
                raise += st->costGeneric;
                if (st->costPip)
                    out->pip[st->costPip - 1]++;
            } else
                reduce += st->costGeneric;
        }
    }
    sDepth--;
    raise += out->generic;
    out->generic = (uint8_t)(raise > reduce ? raise - reduce : 0);
}
