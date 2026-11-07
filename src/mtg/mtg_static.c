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
#include <stdio.h>
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
    if (strncmp(sv, "Count$Valid", 11) != 0 && strncmp(sv, "Number$", 7) != 0 && strcmp(sv, "Count$CardPower") != 0
        && strcmp(sv, "Count$CardToughness") != 0 && strcmp(sv, "Count$Domain") != 0)
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
MtgAltCost *gMtgAltCosts;
static int sAltCount, sAltCap;

// "S:Mode$ AlternativeCost | ValidSA$ Spell.Self | EffectZone$ All | Cost$ ... [| IsPresent$ ...] | Description$ ...":
// the spell may be cast paying that instead of its mana cost (StaticAbilityAlternativeCost). Costs without mana only.
static void ParseAlt(MtgCardDef *c, int cardId, char *text)
{
    static const char *const sKnown[] = {"Mode", "ValidSA", "EffectZone", "Cost", "IsPresent", "Description", "ValidCard",
                                         "ValidPlayer", "CheckSVar", "SVarCompare", "CheckSecondSVar", "SecondSVarCompare"};
    MtgAltCost alt;
    int first, count, i;
    const char *s, *desc;
    memset(&alt, 0, sizeof(alt));
    count = Mtg_ParseParams(text, &first);
    for (i = 0; i < count; i++) {
        unsigned k;
        for (k = 0; k < sizeof(sKnown) / sizeof(sKnown[0]); k++)
            if (strcmp(gMtgParams[first + i].key, sKnown[k]) == 0)
                break;
        if (k == sizeof(sKnown) / sizeof(sKnown[0]))
            goto unknown;
    }
    // ValidSA$ Spell.Self, or Spell with ValidCard$ Card.Self (the Legates' form), ValidPlayer$ You
    if ((s = ParamOf(first, count, "ValidSA")) && strcmp(s, "Spell") == 0) {
        const char *vc = ParamOf(first, count, "ValidCard");
        if (!vc || strcmp(vc, "Card.Self") != 0)
            goto unknown;
        s = "Spell.Self";
    } else if (ParamOf(first, count, "ValidCard"))
        goto unknown;
    if ((desc = ParamOf(first, count, "ValidPlayer")) && strcmp(desc, "You") != 0)
        goto unknown;
    for (i = 0; i < 2; i++) {   // CheckSVar$ / CheckSecondSVar$ (each GE1 unless its compare says)
        alt.check[i] = ParamOf(first, count, i ? "CheckSecondSVar" : "CheckSVar");
        alt.compare[i] = ParamOf(first, count, i ? "SecondSVarCompare" : "SVarCompare");
        if ((alt.check[i] && !Mtg_CheckSvarKnown(cardId, alt.check[i], alt.compare[i])) || (!alt.check[i] && alt.compare[i]))
            goto unknown;
    }
    if (!s || strcmp(s, "Spell.Self") != 0 || c->altCost
        || !(s = ParamOf(first, count, "EffectZone")) || strcmp(s, "All") != 0
        || !(s = ParamOf(first, count, "Cost")) || !Mtg_ParseCost(s, &alt.cost)
        || Mtg_ManaCostTotal(&alt.cost.mana) || alt.cost.mana.x || alt.cost.tap || alt.cost.sacSelf || alt.cost.exileSelf
        || alt.cost.returnSelf || alt.cost.discardSelf || alt.cost.discardRandom)
        goto unknown;
    if ((alt.present = ParamOf(first, count, "IsPresent")) && !Mtg_ValidSpecKnown(alt.present))
        goto unknown;
    // the menu's words: what the Description says the player may do ("you may sacrifice a Mountain rather than ...")
    desc = ParamOf(first, count, "Description");
    {
        const char *from = desc ? strstr(desc, "may ") : NULL, *to = from ? strstr(from, " rather than") : NULL;
        size_t n = 0;
        if (from && to && to > from + 4) {
            from += 4;
            for (; from < to && n + 1 < sizeof(alt.label); from++)
                alt.label[n++] = (*from >= 'a' && *from <= 'z') ? (char)(*from - 32) : *from;
        }
        alt.label[n] = 0;
        if (!n && desc && strstr(desc, "without paying its mana cost"))
            snprintf(alt.label, sizeof(alt.label), "FOR FREE");   // (the Legates, Massacre)
        else if (!n)
            snprintf(alt.label, sizeof(alt.label), "PAY ITS OTHER COST");
    }
    if (sAltCount + 1 > sAltCap) {
        sAltCap = sAltCap ? sAltCap * 2 : 64;
        gMtgAltCosts = realloc(gMtgAltCosts, (size_t)sAltCap * sizeof(MtgAltCost));
    }
    gMtgAltCosts[sAltCount++] = alt;
    c->altCost = (uint16_t)sAltCount;
    return;
unknown:
    Mtg_Unsupported(c, "static ability");
}

void Mtg_ParseStatic(MtgCardDef *c, int cardId, char *text)
{
    char *mode = strstr(text, "Mode$"), *comma, *end;
    if (mode && strncmp(mode + 5 + strspn(mode + 5, " "), "AlternativeCost", 15) == 0) {
        ParseAlt(c, cardId, text);
        return;
    }
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
        ParseOne(c, cardId, a);
        Mtg_ParseStatic(c, cardId, b);   // (the rest: more modes split again)
        return;
    }
    ParseOne(c, cardId, text);
}

static void ParseOne(MtgCardDef *c, int cardId, char *text)
{
    static const char *const sKnown[] = {"Mode", "Affected", "AddPower", "AddToughness", "AddKeyword", "SetPower",
                                         "SetToughness", "Description", "AffectedZone", "EffectZone", "AffectedDefined",
                                         "IsPresent", "PresentCompare", "ValidCard", "ValidAttacker", "ValidBlocker", "CharacteristicDefining",
                                         "Activator", "Type", "Amount", "Cost", "Condition", "Secondary", "UnlessDefender",
                                         "ValidCreature", "ValidTarget", "ValidSA", "ValidSource", "Caster", "AddAbility", "AddTrigger", "AddSVar", "GainControl",
                                         "CheckSVar", "SVarCompare", "AddHiddenKeyword"};
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
            : strcmp(s, "RaiseCost") == 0 ? MTG_STATIC_RAISE_COST : strcmp(s, "ReduceCost") == 0 ? MTG_STATIC_REDUCE_COST
            : strcmp(s, "MustAttack") == 0 ? MTG_STATIC_MUST_ATTACK : strcmp(s, "CantBeCast") == 0 ? MTG_STATIC_CANT_BE_CAST
            : strcmp(s, "CantTarget") == 0 ? MTG_STATIC_CANT_TARGET : strcmp(s, "CantPlayLand") == 0 ? MTG_STATIC_CANT_PLAY_LAND
            : strcmp(s, "CantBeActivated") == 0 ? MTG_STATIC_CANT_BE_ACTIVATED
            : strcmp(s, "CantRegenerate") == 0 ? MTG_STATIC_CANT_REGENERATE : 0xFF;
    if ((s = ParamOf(first, count, "AddHiddenKeyword"))) {   // "CARDNAME can't block." / "can't attack.": those modes
        if (st.mode != MTG_STATIC_CONTINUOUS || ParamOf(first, count, "AddPower") || ParamOf(first, count, "AddToughness")
            || ParamOf(first, count, "AddKeyword") || ParamOf(first, count, "SetPower") || !ParamOf(first, count, "Affected"))
            goto unknown;
        if (strncmp(s, "CARDNAME can't block.", 21) == 0)
            st.mode = MTG_STATIC_CANT_BLOCK;
        else if (strncmp(s, "CARDNAME can't attack.", 22) == 0)
            st.mode = MTG_STATIC_CANT_ATTACK;
        else
            goto unknown;
        st.hiddenKw = 1;
    }
    // the newer modes' own keys (StaticAbilityMustAttack, CantBeCast, CantTarget, CantPlayLand), nowhere else
    if ((ParamOf(first, count, "ValidCreature") && st.mode != MTG_STATIC_MUST_ATTACK)
        || ((ParamOf(first, count, "ValidTarget") || ParamOf(first, count, "ValidSource")) && st.mode != MTG_STATIC_CANT_TARGET)
        || (ParamOf(first, count, "ValidSA") && st.mode != MTG_STATIC_CANT_TARGET && st.mode != MTG_STATIC_CANT_BE_ACTIVATED
            && st.mode != MTG_STATIC_CANT_ATTACK && st.mode != MTG_STATIC_CANT_BLOCK)
        || (ParamOf(first, count, "Caster") && st.mode != MTG_STATIC_CANT_BE_CAST))
        goto unknown;
    if (st.mode == MTG_STATIC_CANT_TARGET) {
        if ((s = ParamOf(first, count, "ValidSA")) && strcmp(s, "Spell") != 0)
            goto unknown;
        st.spellsOnly = s != NULL;
        if ((st.source = ParamOf(first, count, "ValidSource")) && !Mtg_ValidSpecKnown(st.source))
            goto unknown;
        if (!ParamOf(first, count, "ValidTarget") || ParamOf(first, count, "AffectedZone"))
            goto unknown;
    }
    // CantBeActivated (StaticAbilityCantBeCast's activated side): ValidCard$ whose abilities; ValidSA$ Activated (all),
    // Activated.hasTapCost, Activated.Cycling; Activator$ Player (anyone) only
    if (st.mode == MTG_STATIC_CANT_BE_ACTIVATED) {
        s = ParamOf(first, count, "ValidSA");
        st.saKind = !s || strcmp(s, "Activated") == 0 || strcmp(s, "Activated.!ManaAbility") == 0 ? 0
                  : strcmp(s, "Activated.hasTapCost") == 0 ? 1 : strcmp(s, "Activated.Cycling") == 0 ? 2 : 0xFF;
        if (st.saKind == 0xFF)
            goto unknown;
        if ((s = ParamOf(first, count, "Activator"))) {   // (an Effect's: its remembered player only)
            if (strcmp(s, "Player.IsRemembered") == 0)
                st.activator = 3;
            else if (strcmp(s, "Player") != 0)
                goto unknown;
        }
    }
    if (st.mode == MTG_STATIC_CANT_BE_CAST) {   // Caster$ Player (anyone, as none) / You / Opponent: the static's activator slot
        s = ParamOf(first, count, "Caster");
        st.activator = !s || strcmp(s, "Player") == 0 ? 0 : strcmp(s, "You") == 0 ? 1 : strcmp(s, "Opponent") == 0 ? 2
                     : strcmp(s, "Player.IsRemembered") == 0 ? 3 : 0xFF;
        if (st.activator == 0xFF)
            goto unknown;
    }
    if ((s = ParamOf(first, count, "CharacteristicDefining"))) {   // */*: on the card itself, wherever it is
        if (strcmp(s, "True") != 0 || st.mode != MTG_STATIC_CONTINUOUS || ParamOf(first, count, "Affected")
            || ParamOf(first, count, "AffectedDefined") || !c->starPT)
            goto unknown;
        st.self = 1;
        if ((s = ParamOf(first, count, "EffectZone")) && strcmp(s, "All") != 0)
            goto unknown;
    } else if ((s = ParamOf(first, count, "EffectZone")) && strcmp(s, "All") == 0
               && (st.mode == MTG_STATIC_REDUCE_COST || st.mode == MTG_STATIC_RAISE_COST)
               && (s = ParamOf(first, count, "ValidCard")) && strcmp(s, "Card.Self") == 0) {
        st.selfCost = 1;   // (the Avatars: "CARDNAME costs {6} less to cast" from the hand)
    } else if ((s = ParamOf(first, count, "EffectZone")) && strcmp(s, "Battlefield") != 0) {
        if (strcmp(s, "Graveyard") != 0 || st.mode != MTG_STATIC_CONTINUOUS || ParamOf(first, count, "AffectedDefined"))
            goto unknown;
        st.fromGrave = 1;   // (Anger, Wonder...: while the card is in its owner's graveyard)
    }
    if (st.mode == 0xFF || ((s = ParamOf(first, count, "AffectedZone")) && strcmp(s, "Battlefield") != 0))
        goto unknown;
    if ((s = ParamOf(first, count, "AffectedDefined"))) {
        int ench = strcmp(s, "Enchanted") == 0 || strcmp(s, "AttachedBy Self") == 0;   // (the same, for an Aura)
        if (st.mode != MTG_STATIC_CONTINUOUS || (strcmp(s, "Self") != 0 && !ench) || (ench && !c->enchant))
            goto unknown;
        st.self = s[0] == 'S';
        st.enchanted = (uint8_t)ench;
    }
    st.affected = ParamOf(first, count, st.mode == MTG_STATIC_CONTINUOUS || st.hiddenKw ? "Affected"
                                      : st.mode == MTG_STATIC_CANT_BLOCK_BY ? "ValidAttacker"
                                      : st.mode == MTG_STATIC_MUST_ATTACK ? "ValidCreature"
                                      : st.mode == MTG_STATIC_CANT_TARGET ? "ValidTarget" : "ValidCard");
    if (st.mode == MTG_STATIC_CANT_BE_ACTIVATED && !st.affected)
        st.affected = "Card";   // (Stabilizer: every card's Cycling)
    if (st.mode == MTG_STATIC_CANT_PLAY_LAND && !st.affected)
        st.affected = "Land";   // (Territorial Dispute: every land)
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
    } else if ((ParamOf(first, count, "Activator") && st.mode != MTG_STATIC_CANT_BE_CAST && st.mode != MTG_STATIC_CANT_BE_ACTIVATED)
               || ParamOf(first, count, "Type")
               || ParamOf(first, count, "Amount") || ParamOf(first, count, "Cost"))
        goto unknown;
    if (st.mode == MTG_STATIC_CANT_BE_CAST && ParamOf(first, count, "Activator"))
        goto unknown;
    if ((st.present = ParamOf(first, count, "IsPresent")) && !Mtg_ValidSpecKnown(st.present))
        goto unknown;
    // Condition$ (StaticAbility.java checkConditions): Threshold (seven or more cards in its controller's graveyard),
    // PlayerTurn / NotPlayerTurn (its controller's turn or not). Secondary$ True only keeps it out of the card's text.
    if ((s = ParamOf(first, count, "Condition"))) {
        st.condition = strcmp(s, "Threshold") == 0 ? 1 : strcmp(s, "PlayerTurn") == 0 ? 2 : strcmp(s, "NotPlayerTurn") == 0 ? 3 : 0xFF;
        if (st.condition == 0xFF)
            goto unknown;
    }
    if ((s = ParamOf(first, count, "Secondary")) && strcmp(s, "True") != 0)
        goto unknown;
    // UnlessDefender$ (StaticAbilityCantAttackBlock: "can't attack unless defending player ..."): controls<Valid>, the
    // Valid spec negated by a leading "!", or hasFewer<Creatures|Lands>InPlayThanYou
    if ((s = ParamOf(first, count, "UnlessDefender"))) {
        const char *x = s[0] == '!' ? s + 1 : s;
        if (st.mode != MTG_STATIC_CANT_ATTACK)
            goto unknown;
        if (strncmp(x, "controls", 8) == 0) {
            if (!Mtg_ValidSpecKnown(x + 8))
                goto unknown;
        } else if (strcmp(x, "hasFewerCreaturesInPlayThanYou") != 0 && strcmp(x, "hasFewerLandsInPlayThanYou") != 0)
            goto unknown;
        st.unlessDefender = s;
    }
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
    if ((st.check = ParamOf(first, count, "CheckSVar"))) {   // (StaticAbility.checkConditions' CheckSVar)
        st.compare = ParamOf(first, count, "SVarCompare");
        if (!Mtg_CheckSvarKnown(cardId, st.check, st.compare))
            goto unknown;
    } else if (ParamOf(first, count, "SVarCompare"))
        goto unknown;
    if ((s = ParamOf(first, count, "GainControl"))) {   // (Control Magic: "You control enchanted creature")
        if (strcmp(s, "You") != 0 || st.mode != MTG_STATIC_CONTINUOUS || !st.enchanted)
            goto unknown;
        st.gainControl = 1;
    }
    // AddAbility$ "A & B": SVars of its card, each an activated ability ("AB$ ...") of whatever it affects; CARDNAME and
    // Self there are that permanent (the ability's source), its SVars the granting card's (as Forge's AddSVar)
    if ((s = ParamOf(first, count, "AddAbility"))) {
        char names[96], *p = names, *k;
        if (st.mode != MTG_STATIC_CONTINUOUS || strlen(s) >= sizeof(names))
            goto unknown;
        strcpy(names, s);
        while ((k = Mtg_Token(&p, '&')) != NULL) {
            const char *sv;
            char *copy;
            int i;
            while (*k == ' ')
                k++;
            for (i = (int)strlen(k); i > 0 && k[i - 1] == ' '; i--)
                k[i - 1] = 0;
            if (!(sv = Mtg_Svar(cardId, k)) || strncmp(sv, "AB$", 3) != 0 || st.grantCount >= 3)
                goto unknown;
            copy = Kept(strlen(sv) + 1);
            strcpy(copy, sv);
            st.grant[st.grantCount++] = (uint16_t)Mtg_ParseAbilityText(c, cardId, copy, MTG_AB_ACTIVATED);
        }
    }
    // AddTrigger$ "A & B": SVars of its card, each a trigger ("Mode$ ...") of whatever it affects
    if ((s = ParamOf(first, count, "AddTrigger"))) {
        char names[96], *p = names, *k;
        if (st.mode != MTG_STATIC_CONTINUOUS || strlen(s) >= sizeof(names))
            goto unknown;
        strcpy(names, s);
        while ((k = Mtg_Token(&p, '&')) != NULL) {
            const char *sv;
            char *copy;
            int i, t;
            while (*k == ' ')
                k++;
            for (i = (int)strlen(k); i > 0 && k[i - 1] == ' '; i--)
                k[i - 1] = 0;
            if (!(sv = Mtg_Svar(cardId, k)) || strncmp(sv, "Mode$", 5) != 0 || st.grantTrigCount >= 2)
                goto unknown;
            copy = Kept(strlen(sv) + 1);
            strcpy(copy, sv);
            if ((t = Mtg_ParseGrantedTrigger(c, cardId, copy)) < 0)
                return;   // (it said why)
            st.grantTrig[st.grantTrigCount++] = (uint16_t)t;
        }
    }
    // AddSVar$: Forge copies those SVars onto what it affects; here the granted abilities read their own card's
    if ((s = ParamOf(first, count, "AddSVar"))) {
        char names[96], *p = names, *k;
        if (!st.grantCount && !st.grantTrigCount)
            goto unknown;
        if (strlen(s) >= sizeof(names))
            goto unknown;
        strcpy(names, s);
        while ((k = Mtg_Token(&p, '&')) != NULL) {
            int i;
            while (*k == ' ')
                k++;
            for (i = (int)strlen(k); i > 0 && k[i - 1] == ' '; i--)
                k[i - 1] = 0;
            if (!Mtg_Svar(cardId, k))
                goto unknown;
        }
    }
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

// an Effect's StaticAbilities$ line: parsed as the card's own would be, then kept apart from the card's statics
int Mtg_ParseEffectStatic(MtgCardDef *c, int cardId, char *text)
{
    int before = c->staticCount;
    ParseOne(c, cardId, text);
    if (c->staticCount == before)
        return -1;
    return c->statics[--c->staticCount];
}

// R: lines the engine plays as statics: "enters the battlefield tapped" (Event$ Moved, ReplaceWith$ an SVar that taps
// it: ETBTapped), "doesn't untap during its controller's untap step" (Event$ Untap, Layer$ CantHappen) and "prevent all
// damage that would be dealt ..." (Event$ DamageDone, Prevent$ True: PreventDamage)
static int sReplForEffect;   // (parsing an Effect's ReplacementEffects$: ActiveZones$ Command is its own)

// a spec with its Emblem alternatives left out (there are no emblems here), or NULL when it's only those
static const char *NoEmblems(const char *spec)
{
    char *out = malloc(strlen(spec) + 1), *o = out;
    const char *p = spec;
    while (*p) {
        const char *comma = strchr(p, ',');
        size_t n = comma ? (size_t)(comma - p) : strlen(p);
        if (strncmp(p, "Emblem", 6) != 0) {
            if (o != out)
                *o++ = ',';
            memcpy(o, p, n);
            o += n;
        }
        if (!comma)
            break;
        p = comma + 1;
    }
    *o = 0;
    if (o == out) {
        free(out);
        return NULL;
    }
    return out;
}

static int IsPlayerAlt(const char *p, size_t n)
{
    return (n >= 6 && strncmp(p, "Player", 6) == 0 && (n == 6 || p[6] == '.')) || (n == 3 && strncmp(p, "You", 3) == 0)
        || (n == 8 && strncmp(p, "Opponent", 8) == 0);
}

// ValidTarget$ into its cards' and its players' parts (each NULL when it has none)
static int SplitTarget(const char *spec, const char **cards, const char **players)
{
    char *c = malloc(strlen(spec) + 1), *pl = malloc(strlen(spec) + 1), *co = c, *po = pl;
    const char *p = spec;
    while (*p) {
        const char *comma = strchr(p, ',');
        size_t n = comma ? (size_t)(comma - p) : strlen(p);
        char **o = IsPlayerAlt(p, n) ? &po : &co;
        char *base = o == &po ? pl : c;
        if (*o != base)
            *(*o)++ = ',';
        memcpy(*o, p, n);
        *o += n;
        if (!comma)
            break;
        p = comma + 1;
    }
    *co = *po = 0;
    *cards = co != c ? c : (free(c), (const char *)NULL);
    *players = po != pl ? pl : (free(pl), (const char *)NULL);
    if (*cards && !Mtg_ValidSpecKnown(*cards))
        return 0;
    return !*players || Mtg_ValidPlayerSpecKnown(*players);
}

static int ParseRepl(MtgCardDef *c, int cardId, char *text, MtgStatic *out)
{
    static const char *const sKnown[] = {"Event", "ValidCard", "Origin", "Destination", "ReplaceWith", "ReplacementResult",
                                         "Description", "ActiveZones", "Layer", "ValidStepTurnToController",
                                         "Prevent", "ValidTarget", "ValidSource", "IsCombat", "Secondary", "IsPresent",
                                         "PresentCompare", "CheckSVar", "SVarCompare", "DamageAmount", "PreventionEffect",
                                         "ValidSA", "AddSVar"};
    MtgStatic st;
    int first, count, i;
    const char *s, *ev, *zones;
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
            return 0;
    }
    ev = ParamOf(first, count, "Event");
    zones = ParamOf(first, count, "ActiveZones");
    if (!ev)
        return 0;
    if (zones && strcmp(zones, "Battlefield") != 0 && !(sReplForEffect && strcmp(zones, "Command") == 0))
        return 0;
    if (strcmp(ev, "DamageDone") == 0) {   // ReplacePreventDamage (Prevent$ True: all of it, nothing in its place)
        const char *tgt = ParamOf(first, count, "ValidTarget"), *srcSpec = ParamOf(first, count, "ValidSource");
        const char *rw = ParamOf(first, count, "ReplaceWith"), *sv;
        if (rw) {   // "the next time ...": prevented once, then the Effect exiles itself (an Effect's only)
            static const char sExileSelf[] = "DB$ ChangeZone | Defined$ Self | Origin$ Command | Destination$ Exile";
            static const char sGain[] = "DB$ GainLife | Defined$ You | LifeAmount$ X | SubAbility$ ";
            if (!sReplForEffect || ParamOf(first, count, "Prevent") || !(s = ParamOf(first, count, "PreventionEffect"))
                || strcmp(s, "True") || !(sv = Mtg_Svar(cardId, rw)))
                return 0;
            if (strncmp(sv, sGain, sizeof(sGain) - 1) == 0) {   // (and its controller gains that much life: REVERSE DAMAGE)
                const char *ex = Mtg_Svar(cardId, sv + sizeof(sGain) - 1), *x = Mtg_Svar(cardId, "X");
                if (!ex || strcmp(ex, sExileSelf) || !x || strcmp(x, "ReplaceCount$DamageAmount"))
                    return 0;
                st.gainLife = 1;
            } else if (strcmp(sv, sExileSelf))
                return 0;
            st.oneShot = 1;
        } else if (!(s = ParamOf(first, count, "Prevent")) || strcmp(s, "True") != 0 || ParamOf(first, count, "PreventionEffect"))
            return 0;
        if (ParamOf(first, count, "ValidCard") || ParamOf(first, count, "Destination") || ParamOf(first, count, "Layer")
            || ParamOf(first, count, "ValidStepTurnToController"))
            return 0;
        if (tgt) {
            if (!SplitTarget(tgt, &st.affected, &st.targetPlayer))
                goto fail;
            st.tgtKind = (uint8_t)((st.affected ? 1 : 0) | (st.targetPlayer ? 2 : 0));
        }
        if (srcSpec && (!(st.source = NoEmblems(srcSpec)) || !Mtg_ValidSpecKnown(st.source)))
            goto fail;
        if ((s = ParamOf(first, count, "IsCombat")))
            st.combatOnly = strcmp(s, "True") == 0 ? 1 : 2;
        if ((s = ParamOf(first, count, "DamageAmount"))) {
            if (strncmp(s, "LE", 2) || s[2] < '1' || s[2] > '9' || s[3])
                goto fail;
            st.damageMax = (uint8_t)(s[2] - '0');
        }
        if ((s = ParamOf(first, count, "IsPresent"))) {
            const char *cmp = ParamOf(first, count, "PresentCompare");
            if (!Mtg_ValidSpecKnown(s) || (cmp && (strncmp(cmp, "GE", 2) || cmp[2] < '0' || cmp[2] > '9' || cmp[3])))
                goto fail;
            st.present = s;
            st.presentNum = (uint8_t)(cmp ? cmp[2] - '0' : 1);
        } else if (ParamOf(first, count, "PresentCompare"))
            goto fail;
        if ((s = ParamOf(first, count, "CheckSVar"))) {
            st.check = s;
            st.compare = ParamOf(first, count, "SVarCompare");
            if (!Mtg_CheckSvarKnown(cardId, st.check, st.compare))
                goto fail;
        } else if (ParamOf(first, count, "SVarCompare"))
            goto fail;
        st.mode = MTG_STATIC_PREVENT_DAMAGE;
        *out = st;
        return 1;
    fail:
        free((void *)st.affected);
        free((void *)st.targetPlayer);
        free((void *)st.source);
        return 0;
    }
    if (ParamOf(first, count, "Prevent") || ParamOf(first, count, "ValidTarget")
        || ParamOf(first, count, "ValidSource") || ParamOf(first, count, "IsCombat") || ParamOf(first, count, "DamageAmount"))
        return 0;
    if ((s = ParamOf(first, count, "IsPresent"))) {   // (only while the battlefield has one: Untap's)
        const char *cmp = ParamOf(first, count, "PresentCompare");
        if (!Mtg_ValidSpecKnown(s) || (cmp && (strncmp(cmp, "GE", 2) || cmp[2] < '0' || cmp[2] > '9' || cmp[3])))
            return 0;
        st.present = s;
        st.presentNum = (uint8_t)(cmp ? cmp[2] - '0' : 1);
    } else if (ParamOf(first, count, "PresentCompare"))
        return 0;
    if ((s = ParamOf(first, count, "CheckSVar"))) {
        st.check = s;
        st.compare = ParamOf(first, count, "SVarCompare");
        if (!Mtg_CheckSvarKnown(cardId, st.check, st.compare))
            return 0;
    } else if (ParamOf(first, count, "SVarCompare"))
        return 0;
    if ((st.present || st.check) && strcmp(ev, "Untap") != 0)
        return 0;
    if (strcmp(ev, "Counter") == 0) {   // "can't be countered": the spell itself, or spells of a kind (from the battlefield)
        const char *sa = ParamOf(first, count, "ValidSA"), *vc = ParamOf(first, count, "ValidCard");
        if (sReplForEffect || !(s = ParamOf(first, count, "Layer")) || strcmp(s, "CantHappen") || !sa
            || ParamOf(first, count, "ReplaceWith") || ParamOf(first, count, "Destination")
            || ParamOf(first, count, "ValidStepTurnToController"))
            return 0;
        if (vc && strcmp(vc, "Card.Self") == 0 && strcmp(sa, "Spell") == 0 && !zones) {
            c->cantBeCountered = 1;
            return 2;   // (on the card: no static)
        }
        if (vc || !zones)
            return 0;
        if (strncmp(sa, "Spell.", 6) == 0) {
            st.affected = sa + 6;
            if (!Mtg_ValidSpecKnown(st.affected))
                return 0;
        } else if (strcmp(sa, "Spell") != 0)
            return 0;
        st.mode = MTG_STATIC_CANT_BE_COUNTERED;
        *out = st;
        return 1;
    }
    if (ParamOf(first, count, "ValidSA"))
        return 0;
    st.affected = ParamOf(first, count, "ValidCard");
    if (!st.affected || !Mtg_ValidSpecKnown(st.affected))
        return 0;
    if (strcmp(st.affected, "Card.Self") != 0 && !zones && !sReplForEffect)
        return 0;   // (another card's: only from the battlefield)
    if (strcmp(ev, "Moved") == 0 && (s = ParamOf(first, count, "Origin"))) {
        // "if it would die, put it on top of its owner's library instead" (on the card: Mtg_MoveTo)
        const char *sv, *d2 = ParamOf(first, count, "Destination"), *rw = ParamOf(first, count, "ReplaceWith");
        if (sReplForEffect || strcmp(s, "Battlefield") || !d2 || strcmp(d2, "Graveyard") || strcmp(st.affected, "Card.Self")
            || !rw || !(sv = Mtg_Svar(cardId, rw)) || ParamOf(first, count, "Layer")
            || strcmp(sv, "DB$ ChangeZone | Origin$ Battlefield | Destination$ Library | LibraryPosition$ 0 | Defined$ ReplacedCard"))
            return 0;
        c->diesToLibraryTop = 1;
        return 2;
    }
    if (ParamOf(first, count, "Origin"))
        return 0;
    if (strcmp(ev, "Moved") == 0) {
        if (sReplForEffect)
            return 0;
        const char *sv;
        if (!(s = ParamOf(first, count, "Destination")) || strcmp(s, "Battlefield") != 0
            || !(s = ParamOf(first, count, "ReplaceWith")) || !(sv = Mtg_Svar(cardId, s)) || ParamOf(first, count, "Layer")
            || ParamOf(first, count, "ValidStepTurnToController"))
            return 0;
        // "DB$ Tap | Defined$ Self | ETB$ True" (Defined$ ReplacedCard for another card's)
        if (strcmp(sv, "DB$ Tap | Defined$ Self | ETB$ True") != 0 && strcmp(sv, "DB$ Tap | ETB$ True | Defined$ ReplacedCard") != 0
            && strcmp(sv, "DB$ Tap | Defined$ ReplacedCard | ETB$ True") != 0)
            return 0;
        st.mode = MTG_STATIC_ETB_TAPPED;
    } else if (strcmp(ev, "Untap") == 0) {
        if (!(s = ParamOf(first, count, "Layer")) || strcmp(s, "CantHappen") != 0
            || !(s = ParamOf(first, count, "ValidStepTurnToController"))
            || (strcmp(s, "You") != 0 && !(sReplForEffect && strcmp(s, "Player.Activator") == 0))
            || ParamOf(first, count, "ReplaceWith") || ParamOf(first, count, "Destination"))
            return 0;
        st.mode = MTG_STATIC_NO_UNTAP;
    } else
        return 0;
    (void)c;
    *out = st;
    return 1;
}

static int AddStatic(MtgCardDef *c, const MtgStatic *st)
{
    if (c->staticCount >= sizeof(c->statics) / sizeof(c->statics[0]))
        return 0;
    if (sStaticCount + 1 > sStaticCap) {
        sStaticCap = sStaticCap ? sStaticCap * 2 : 256;
        gMtgStatics = realloc(gMtgStatics, (size_t)sStaticCap * sizeof(MtgStatic));
    }
    gMtgStatics[sStaticCount] = *st;
    c->statics[c->staticCount++] = (uint16_t)sStaticCount++;
    return 1;
}

void Mtg_ParseReplacement(MtgCardDef *c, int cardId, char *text)
{
    MtgStatic st;
    int r = ParseRepl(c, cardId, text, &st);
    if (!r || (r == 1 && !AddStatic(c, &st)))
        Mtg_Unsupported(c, "replacement effect");
}

// an Effect's ReplacementEffects$ line: kept apart from the card's statics, as Mtg_ParseEffectStatic's
int Mtg_ParseEffectReplacement(MtgCardDef *c, int cardId, char *text)
{
    MtgStatic st;
    int ok;
    sReplForEffect = 1;
    ok = ParseRepl(c, cardId, text, &st);
    sReplForEffect = 0;
    if (ok != 1 || !AddStatic(c, &st)) {
        Mtg_Unsupported(c, "ReplacementEffects");
        return -1;
    }
    return c->statics[--c->staticCount];
}

// Condition$ now
int Mtg_CheckSvar(const MtgDuel *d, int card, int src, int ctrl, const char *name, const char *cmp)
{
    MtgStackEntry e;
    MtgResolveCtx ctx;
    int ok, v, w;
    char *end;
    memset(&e, 0, sizeof(e));
    ctx.e = &e;
    ctx.controller = ctrl;
    ctx.source = src;
    v = Mtg_Amount(d, &ctx, card, name, &ok);
    if (!cmp)
        cmp = "GE1";
    w = (int)strtol(cmp + 2, &end, 10);
    if (end == cmp + 2 || *end)
        w = Mtg_Amount(d, &ctx, card, cmp + 2, &ok);   // ("LEY": another SVar)
    return strncmp(cmp, "GE", 2) == 0 ? v >= w : strncmp(cmp, "GT", 2) == 0 ? v > w : strncmp(cmp, "LE", 2) == 0 ? v <= w
         : strncmp(cmp, "LT", 2) == 0 ? v < w : strncmp(cmp, "EQ", 2) == 0 ? v == w : v != w;
}

int Mtg_CheckSvarKnown(int card, const char *name, const char *cmp)
{
    int ok;
    char *end;
    if (!Mtg_Svar(card, name))
        return 0;
    Mtg_Amount(NULL, NULL, card, name, &ok);
    if (!ok)
        return 0;
    if (!cmp)
        return 1;
    if (strlen(cmp) < 3 || (strncmp(cmp, "GE", 2) && strncmp(cmp, "GT", 2) && strncmp(cmp, "LE", 2) && strncmp(cmp, "LT", 2)
                            && strncmp(cmp, "EQ", 2) && strncmp(cmp, "NE", 2)))
        return 0;
    strtol(cmp + 2, &end, 10);
    if (end != cmp + 2 && !*end)
        return 1;
    if (!Mtg_Svar(card, cmp + 2))
        return 0;
    Mtg_Amount(NULL, NULL, card, cmp + 2, &ok);
    return ok;
}

static int ConditionHolds(const MtgDuel *d, const MtgStatic *st, int src)
{
    int i, n = 0, me = d->objs[src].controller;
    if (st->check && !Mtg_CheckSvar(d, st->card, src, me, st->check, st->compare))
        return 0;
    switch (st->condition) {
    case 1:
        for (i = 0; i < d->objCount; i++)
            n += d->objs[i].zone == MTG_ZONE_GRAVEYARD && d->objs[i].owner == me;
        return n >= 7;
    case 2:
        return d->active == me;
    case 3:
        return d->active != me;
    }
    return 1;
}

// UnlessDefender$: is the defending player (OBJ's controller's opponent) as it says?
static int DefenderIs(const MtgDuel *d, const MtgStatic *st, int src, int obj)
{
    const char *x = st->unlessDefender;
    int neg = x[0] == '!', you = d->objs[obj].controller, def = you ^ 1, i, mine = 0, theirs = 0, has;
    if (neg)
        x++;
    if (strncmp(x, "controls", 8) == 0) {
        has = 0;
        for (i = 0; i < d->objCount && !has; i++)
            has = d->objs[i].zone == MTG_ZONE_BATTLEFIELD && d->objs[i].controller == def
                  && Mtg_ValidObj(d, i, x + 8, src, def);
    } else {
        int creatures = strstr(x, "Creatures") != NULL;
        for (i = 0; i < d->objCount; i++)
            if (d->objs[i].zone == MTG_ZONE_BATTLEFIELD
                && Mtg_ValidObj(d, i, creatures ? "Creature" : "Land", src, d->objs[i].controller)) {
                mine += d->objs[i].controller == you;
                theirs += d->objs[i].controller == def;
            }
        has = theirs < mine;
    }
    return neg ? !has : has;
}

// IsPresent$: does the battlefield have enough of them now (relative to the static's source and its controller)?
// (Condition$ too: both say whether the static is on at all)
static int Present(const MtgDuel *d, const MtgStatic *st, int src)
{
    int i, n = 0;
    if (!ConditionHolds(d, st, src))
        return 0;
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
// one static's share of what OBJ is now (SRC its source, CTRL that source's controller)
static void ModsBy(const MtgDuel *d, const MtgStatic *st, int s, int ctrl, uint32_t srcSeq, MtgStaticMods *m,
                   uint32_t *setPowerSeqP, uint32_t *setToughSeqP)
{
    uint32_t setPowerSeq = *setPowerSeqP, setToughSeq = *setToughSeqP;
    m->addPower += st->addPower;
    m->addToughness += st->addToughness;
    if (st->powerX || st->toughX) {   // (counted now, as Forge's layer 7c does each time)
        MtgStackEntry e;
        MtgResolveCtx ctx;
        int ok;
        memset(&e, 0, sizeof(e));
        ctx.e = &e;
        ctx.controller = ctrl;
        ctx.source = s;
        if (st->powerX)
            m->addPower += Mtg_Amount(d, &ctx, st->card, st->powerX, &ok);
        if (st->toughX)
            m->addToughness += Mtg_Amount(d, &ctx, st->card, st->toughX, &ok);
    }
    m->keywords |= st->addKeywords;
    m->landwalk |= st->addLandwalk;
    m->protection |= st->addProtection;
    if (st->setPower != MTG_STATIC_NO_SET && srcSeq + 1 > setPowerSeq)
        m->setPower = st->setPower, setPowerSeq = srcSeq + 1;
    if (st->setToughness != MTG_STATIC_NO_SET && srcSeq + 1 > setToughSeq)
        m->setToughness = st->setToughness, setToughSeq = srcSeq + 1;
    if (st->setPowerX || st->setToughX) {   // (a */* creature's own: counted now; any later set wins)
        MtgStackEntry e;
        MtgResolveCtx ctx;
        int ok;
        memset(&e, 0, sizeof(e));
        ctx.e = &e;
        ctx.controller = ctrl;
        ctx.source = s;
        if (st->setPowerX && setPowerSeq == 0)
            m->setPower = Mtg_Amount(d, &ctx, st->card, st->setPowerX, &ok), setPowerSeq = 1;
        if (st->setToughX && setToughSeq == 0)
            m->setToughness = Mtg_Amount(d, &ctx, st->card, st->setToughX, &ok), setToughSeq = 1;
    }
    *setPowerSeqP = setPowerSeq;
    *setToughSeqP = setToughSeq;
}

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
        if (src->zone != MTG_ZONE_BATTLEFIELD && src->zone != MTG_ZONE_GRAVEYARD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if ((src->zone == MTG_ZONE_GRAVEYARD) != st->fromGrave)
                continue;   // (EffectZone$ Graveyard: only from there; the rest only from the battlefield)
            if (st->mode != MTG_STATIC_CONTINUOUS || !Applies(d, st, s, obj))
                continue;
            ModsBy(d, st, s, src->controller, src->seq, m, &setPowerSeq, &setToughSeq);
        }
    }
    for (s = 0; s < d->effCount; s++) {   // (the Effects in force: their own statics, their own remembered)
        const MtgEffect *e = &d->effects[s];
        if (!Mtg_EffectAlive(d, e))
            continue;
        gMtgEffCtx = e;
        for (k = 0; k < gMtgAbilities[e->ability].effStaticCount; k++) {
            const MtgStatic *st = &gMtgStatics[gMtgAbilities[e->ability].effStatics[k]];
            if (st->mode == MTG_STATIC_CONTINUOUS && Applies(d, st, e->source, obj))
                ModsBy(d, st, e->source, e->controller, e->sourceSeq, m, &setPowerSeq, &setToughSeq);
        }
        gMtgEffCtx = NULL;
    }
    sDepth--;
}

int Mtg_ObjAbilities(const MtgDuel *d, int obj, uint16_t *out, int max)
{
    const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
    int n = 0, s, k, g;
    for (k = 0; k < c->abilityCount && n < max; k++)
        out[n++] = c->abilities[k];
    if (sDepth || d->objs[obj].zone != MTG_ZONE_BATTLEFIELD)
        return n;
    sDepth++;
    for (s = 0; s < d->objCount && n < max; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *sc;
        if (src->zone != MTG_ZONE_BATTLEFIELD && src->zone != MTG_ZONE_GRAVEYARD)
            continue;
        sc = Mtg_Card(src->def);
        for (k = 0; k < sc->staticCount && n < max; k++) {
            const MtgStatic *st = &gMtgStatics[sc->statics[k]];
            if ((src->zone == MTG_ZONE_GRAVEYARD) != st->fromGrave)
                continue;   // (EffectZone$ Graveyard: only from there; the rest only from the battlefield)
            if (st->mode != MTG_STATIC_CONTINUOUS || !st->grantCount || !Applies(d, st, s, obj))
                continue;
            for (g = 0; g < st->grantCount && n < max; g++)
                out[n++] = st->grant[g];
        }
    }
    sDepth--;
    return n;
}

int Mtg_ObjTriggers(const MtgDuel *d, int obj, uint16_t *out, int max)
{
    const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
    int n = 0, s, k, g;
    for (k = 0; k < c->triggerCount && n < max; k++)
        out[n++] = c->triggers[k];
    if (sDepth || d->objs[obj].zone != MTG_ZONE_BATTLEFIELD)
        return n;
    sDepth++;
    for (s = 0; s < d->objCount && n < max; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *sc;
        if (src->zone != MTG_ZONE_BATTLEFIELD && src->zone != MTG_ZONE_GRAVEYARD)
            continue;
        sc = Mtg_Card(src->def);
        for (k = 0; k < sc->staticCount && n < max; k++) {
            const MtgStatic *st = &gMtgStatics[sc->statics[k]];
            if ((src->zone == MTG_ZONE_GRAVEYARD) != st->fromGrave)
                continue;   // (EffectZone$ Graveyard: only from there; the rest only from the battlefield)
            if (st->mode != MTG_STATIC_CONTINUOUS || !st->grantTrigCount || !Applies(d, st, s, obj))
                continue;
            for (g = 0; g < st->grantTrigCount && n < max; g++)
                out[n++] = st->grantTrig[g];
        }
    }
    sDepth--;
    return n;
}

int Mtg_StaticController(const MtgDuel *d, int obj)
{
    int s, k, who = -1;
    uint32_t seq = 0;
    if (sDepth)
        return -1;
    sDepth++;
    for (s = 0; s < d->objCount; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *c;
        if (src->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if (!st->gainControl || !Applies(d, st, s, obj) || src->seq < seq)
                continue;
            who = src->controller, seq = src->seq;   // (the newest wins: timestamps)
        }
    }
    sDepth--;
    return who;
}

// CantBlock / CantAttack: does a static forbid OBJ? CantBlockBy: may OBJ (attacking) not be blocked by OTHER?
// does this static (from S, controlled by CTRL) forbid it? (Mtg_StaticForbids' test of one)
static int ForbidBy(const MtgDuel *d, const MtgStatic *st, int s, int ctrl, int mode, int obj, int other)
{
    if (st->mode != mode || !Applies(d, st, s, obj))
        return 0;
    if (mode == MTG_STATIC_CANT_BLOCK_BY && st->blocker && !Mtg_ValidObj(d, other, st->blocker, s, ctrl))
        return 0;
    if (st->unlessDefender && DefenderIs(d, st, s, obj))
        return 0;   // (the defending player is as it says: it may attack)
    // CantBeCast: OTHER is the player casting it (Caster$: anyone / its controller / an opponent / remembered)
    if (mode == MTG_STATIC_CANT_BE_CAST && ((st->activator == 1 && other != ctrl)
                                            || (st->activator == 2 && other == ctrl)
                                            || (st->activator == 3 && !Mtg_IsRememberedPlayer(d, other))))
        return 0;
    if (mode == MTG_STATIC_CANT_BE_ACTIVATED && st->activator == 3 && !Mtg_IsRememberedPlayer(d, d->objs[obj].controller))
        return 0;
    // CantBeActivated: OTHER is the ability (its kind: any, with {T}, Cycling)
    if (mode == MTG_STATIC_CANT_BE_ACTIVATED && other != 0xFFFF
        && ((st->saKind == 1 && !gMtgAbilities[other].cost.tap) || (st->saKind == 2 && !gMtgAbilities[other].fromHand)))
        return 0;
    // CantTarget: OTHER is the targeting spell's or ability's source (ValidSA$ Spell: only from a spell, which
    // isn't on the battlefield; ValidSource$: only such sources)
    if (mode == MTG_STATIC_CANT_TARGET && other != MTG_NO_OBJ) {
        if (st->spellsOnly && d->objs[other].zone == MTG_ZONE_BATTLEFIELD)
            return 0;
        if (st->source && !Mtg_ValidObj(d, other, st->source, s, ctrl))
            return 0;
    }
    return 1;
}

int Mtg_StaticForbids(const MtgDuel *d, int mode, int obj, int other)
{
    int s, k, forbid = 0;
    if (sDepth)
        return 0;
    sDepth++;
    for (s = 0; s < d->objCount && !forbid; s++) {
        const MtgObj *src = &d->objs[s];
        const MtgCardDef *c;
        if (src->zone != MTG_ZONE_BATTLEFIELD && src->zone != MTG_ZONE_GRAVEYARD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount && !forbid; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if ((src->zone == MTG_ZONE_GRAVEYARD) != st->fromGrave)
                continue;   // (EffectZone$ Graveyard: only from there; the rest only from the battlefield)
            if (!ForbidBy(d, st, s, src->controller, mode, obj, other))
                continue;
            forbid = 1;
        }
    }
    for (s = 0; s < d->effCount && !forbid; s++) {   // (the Effects in force)
        const MtgEffect *e = &d->effects[s];
        if (!Mtg_EffectAlive(d, e))
            continue;
        gMtgEffCtx = e;
        for (k = 0; k < gMtgAbilities[e->ability].effStaticCount && !forbid; k++)
            forbid = ForbidBy(d, &gMtgStatics[gMtgAbilities[e->ability].effStatics[k]], e->source, e->controller, mode, obj,
                              other);
        gMtgEffCtx = NULL;
    }
    sDepth--;
    return forbid;
}

// one PreventDamage (from S, its controller CTRL) against damage from SRC to TO
static int PreventBy(const MtgDuel *d, const MtgStatic *st, int s, int ctrl, int src, MtgTarget to, int combat, int amount)
{
    if (st->mode != MTG_STATIC_PREVENT_DAMAGE)
        return 0;
    if ((st->combatOnly == 1 && !combat) || (st->combatOnly == 2 && combat) || (st->damageMax && amount > st->damageMax))
        return 0;
    if (to.kind == MTG_TGT_OBJ) {
        if (st->tgtKind == 2 || (st->affected && !Mtg_ValidObj(d, to.id, st->affected, s, ctrl)))
            return 0;
    } else if (to.kind == MTG_TGT_PLAYER) {
        if (st->tgtKind == 1 || (st->targetPlayer && !Mtg_ValidPlayer(d, to.id, st->targetPlayer, ctrl)))
            return 0;
    } else
        return 0;
    if (st->source && (src == MTG_NO_OBJ || !Mtg_ValidObj(d, src, st->source, s, ctrl)))
        return 0;
    return Present(d, st, s);
}

int Mtg_DamagePrevented(MtgDuel *d, int src, MtgTarget to, int combat, int amount)
{
    int s, k, prevent = 0;
    if (sDepth)
        return 0;
    sDepth++;
    for (s = 0; s < d->objCount && !prevent; s++) {
        const MtgObj *o = &d->objs[s];
        const MtgCardDef *c;
        if (o->zone != MTG_ZONE_BATTLEFIELD)
            continue;
        c = Mtg_Card(o->def);
        for (k = 0; k < c->staticCount && !prevent; k++)
            prevent = PreventBy(d, &gMtgStatics[c->statics[k]], s, o->controller, src, to, combat, amount);
    }
    for (s = 0; s < d->effCount && !prevent; s++) {
        MtgEffect *e = &d->effects[s];
        if (!Mtg_EffectAlive(d, e))
            continue;
        gMtgEffCtx = e;
        for (k = 0; k < gMtgAbilities[e->ability].effStaticCount && !prevent; k++) {
            const MtgStatic *st = &gMtgStatics[gMtgAbilities[e->ability].effStatics[k]];
            prevent = PreventBy(d, st, e->source, e->controller, src, to, combat, amount);
            if (prevent && st->gainLife)
                Mtg_GainLife(d, e->controller, amount);   // ("you gain life equal to the damage prevented this way")
            if (prevent && st->oneShot)
                e->used = 1;   // ("the next time": done, the Effect gone at the next check)
        }
        gMtgEffCtx = NULL;
    }
    sDepth--;
    return prevent;
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
        if (src->zone != MTG_ZONE_BATTLEFIELD && src->zone != MTG_ZONE_GRAVEYARD)
            continue;
        c = Mtg_Card(src->def);
        for (k = 0; k < c->staticCount; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if ((src->zone == MTG_ZONE_GRAVEYARD) != st->fromGrave)
                continue;   // (EffectZone$ Graveyard: only from there; the rest only from the battlefield)
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
    {   // its own "costs less to cast" (the Avatars: EffectZone$ All, from the hand)
        const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
        int k;
        for (k = 0; k < c->staticCount; k++) {
            const MtgStatic *st = &gMtgStatics[c->statics[k]];
            if (!st->selfCost || (st->check && !Mtg_CheckSvar(d, st->card, obj, player, st->check, st->compare)))
                continue;
            if (st->mode == MTG_STATIC_RAISE_COST)
                raise += st->costGeneric;
            else
                reduce += st->costGeneric;
        }
    }
    sDepth--;
    raise += out->generic;
    out->generic = (uint8_t)(raise > reduce ? raise - reduce : 0);
}
