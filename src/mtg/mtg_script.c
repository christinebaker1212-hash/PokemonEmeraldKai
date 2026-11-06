// Moemon Kai: reading Forge's card scripts (graphics/mtg/forge.pak) into card definitions: what Forge's CardRules /
// CardFactory / AbilityFactory do. A line the engine doesn't translate yet makes the card unsupported (with the
// reason, for docs/MTG_SCOPE.md), so a duel never meets a card it would play wrongly.
#include <stdlib.h>
#include <string.h>
#include "mtg_internal.h"

MtgAbility *gMtgAbilities;
MtgParam *gMtgParams;
MtgParam *gMtgSvars;
static int sAbilityCount, sAbilityCap, sParamCount, sParamCap, sSvarCount, sSvarCap;
static MtgCardDef *sCards;
static int sCardCount;
static char *sText;            // the pack's text, copied once and cut into strings in place

int Mtg_DbCount(void) { return sCardCount; }
const MtgCardDef *Mtg_Card(int id) { return (id >= 0 && id < sCardCount) ? &sCards[id] : NULL; }

int Mtg_CardByName(const char *name)
{
    int lo = 0, hi = sCardCount - 1;   // the pack is sorted by name (bytes)
    while (lo <= hi) {
        int mid = (lo + hi) / 2, c = strcmp(sCards[mid].name, name);
        if (c == 0)
            return mid;
        if (c < 0)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

int Mtg_ManaCostTotal(const MtgManaCost *c)
{
    int n = c->generic, i;
    for (i = 0; i < MTG_MANA_KINDS; i++)
        n += c->pip[i];
    return n;
}

static void *Grow(void *p, int *cap, int need, size_t size)
{
    if (need <= *cap)
        return p;
    *cap = *cap ? *cap * 2 : 1024;
    if (*cap < need)
        *cap = need;
    return realloc(p, (size_t)*cap * size);
}

// the next token of *p split by sep (runs of sep skipped; NULL at the end), written here because MinGW's C
// library doesn't have strtok_r everywhere
char *Mtg_Token(char **p, char sep)
{
    char *s = *p, *e;
    while (s && *s == sep)
        s++;
    if (!s || !*s) {
        *p = NULL;
        return NULL;
    }
    e = strchr(s, sep);
    if (e) {
        *e = 0;
        *p = e + 1;
    } else
        *p = NULL;
    return s;
}

static char *Trim(char *s)
{
    char *e;
    while (*s == ' ' || *s == '\t')
        s++;
    e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r'))
        *--e = 0;
    return s;
}

static int ManaSymbol(const char *t)
{
    static const char sym[] = "WUBRGC";
    const char *p;
    if (t[0] && !t[1] && (p = strchr(sym, t[0])) != NULL)
        return (int)(p - sym);
    return -1;
}

// ManaCost.java's text form: "3 W W", "X G", "no cost"
int Mtg_ParseManaCost(const char *s, MtgManaCost *out)
{
    char buf[64], *tok, *save;
    memset(out, 0, sizeof(*out));
    if (!s || strncmp(s, "no cost", 7) == 0)
        return 1;
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    for (save = buf, tok = Mtg_Token(&save, ' '); tok; tok = Mtg_Token(&save, ' ')) {
        int c = ManaSymbol(tok);
        if (c >= 0)
            out->pip[c]++;
        else if (tok[0] >= '0' && tok[0] <= '9' && strspn(tok, "0123456789") == strlen(tok))
            out->generic += atoi(tok);
        else if (strcmp(tok, "X") == 0)
            out->x++;
        else
            return 0;   // hybrid, Phyrexian, snow: not yet
    }
    return 1;
}

static void Unsupported(MtgCardDef *c, const char *why)
{
    if (c->supported) {
        c->supported = 0;
        c->unsupported = why;
    }
}

void Mtg_Unsupported(MtgCardDef *c, const char *why) { Unsupported(c, why); }

// "SP$ DealDamage | ValidTgts$ Any | NumDmg$ 3" -> params (the first is the API: key "SP" / "AB" / "DB")
int Mtg_ParseParams(char *s, int *first)
{
    int n = 0;
    char *part = s;
    *first = sParamCount;
    while (part) {
        char *bar = strchr(part, '|'), *dollar;
        if (bar)
            *bar = 0;
        dollar = strchr(part, '$');
        if (dollar) {
            *dollar = 0;
            gMtgParams = Grow(gMtgParams, &sParamCap, sParamCount + 1, sizeof(MtgParam));
            gMtgParams[sParamCount].key = Trim(part);
            gMtgParams[sParamCount].value = Trim(dollar + 1);
            sParamCount++;
            n++;
        }
        part = bar ? bar + 1 : NULL;
    }
    return n;
}

const char *Mtg_Param(const MtgAbility *ab, const char *key)
{
    int i;
    for (i = 0; i < ab->paramCount; i++)
        if (strcmp(gMtgParams[ab->paramFirst + i].key, key) == 0)
            return gMtgParams[ab->paramFirst + i].value;
    return NULL;
}

const char *Mtg_Svar(int card, const char *name)
{
    const MtgCardDef *c = Mtg_Card(card);
    int i;
    if (!c)
        return NULL;
    for (i = 0; i < c->svarCount; i++)
        if (strcmp(gMtgSvars[c->svarFirst + i].key, name) == 0)
            return gMtgSvars[c->svarFirst + i].value;
    return NULL;
}

// a cost part's spec kept for the pool's life (the cost text is parsed from a copy)
static const char *KeepSpec(const char *s, size_t n)
{
    static char **sKeep;
    static int sKeepCount;
    char *t = malloc(n + 1);
    memcpy(t, s, n);
    t[n] = 0;
    sKeep = realloc(sKeep, (size_t)(sKeepCount + 1) * sizeof(char *));
    sKeep[sKeepCount++] = t;
    return t;
}

// "Sac<2/Land/two lands>" -> n 2, spec "Land" (the words after a second '/' only describe it); 0: not that part
static int CostPart(const char *tok, const char *name, int *n, const char **spec, size_t *specLen)
{
    size_t len = strlen(name);
    const char *p, *slash, *end;
    if (strncmp(tok, name, len) != 0 || tok[len] != '<' || !(end = strchr(tok, '>')))
        return 0;
    p = tok + len + 1;
    if (*p < '0' || *p > '9')
        return -1;   // (X and the like: not yet)
    *n = atoi(p);
    while (*p >= '0' && *p <= '9')
        p++;
    if (*p == '>') {
        *spec = NULL, *specLen = 0;
        return 1;
    }
    if (*p != '/')
        return -1;
    p++;
    slash = strchr(p, '/');
    *spec = p;
    *specLen = (size_t)((slash && slash < end ? slash : end) - p);
    return 1;
}

// Cost.java's text: "T", "2 R", "1 T", "T Sac<1/CARDNAME>", "1 Sac<1/Creature>", "Discard<1/Card>", "PayLife<2>"
static int ParseCost(const char *s, MtgCost *out)
{
    char buf[160], mana[96] = "", *tok, *save, *open = NULL;
    memset(out, 0, sizeof(*out));
    if (!s)
        return 1;
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    for (save = buf, tok = Mtg_Token(&save, ' '); tok; tok = Mtg_Token(&save, ' ')) {
        int n, r;
        const char *spec;
        size_t specLen;
        if (open) {   // (a part's description had spaces: join it up to its '>')
            tok[-1] = ' ';
            if (!strchr(tok, '>'))
                continue;
            tok = open;
            open = NULL;
        } else if (strchr(tok, '<') && !strchr(tok, '>')) {
            open = tok;
            continue;
        }
        if (strcmp(tok, "T") == 0)
            out->tap = 1;
        else if (strcmp(tok, "Sac<1/CARDNAME>") == 0)
            out->sacSelf = 1;
        else if (strcmp(tok, "Exile<1/CARDNAME>") == 0)
            out->exileSelf = 1;
        else if (strcmp(tok, "Return<1/CARDNAME>") == 0)
            out->returnSelf = 1;
        else if ((r = CostPart(tok, "PayLife", &n, &spec, &specLen)) != 0) {
            if (r < 0 || spec || n < 1 || n > 20 || out->payLife)
                return 0;
            out->payLife = (uint8_t)n;
        } else if ((r = CostPart(tok, "Sac", &n, &spec, &specLen)) != 0) {
            if (r < 0 || !spec || n < 1 || n > 3 || out->sacN)
                return 0;
            out->sacN = (uint8_t)n;
            out->sacSpec = KeepSpec(spec, specLen);
            if (!Mtg_ValidSpecKnown(out->sacSpec))
                return 0;
        } else if ((r = CostPart(tok, "Discard", &n, &spec, &specLen)) != 0) {
            if (r < 0 || !spec || n < 1 || n > 3 || out->discardN)
                return 0;
            out->discardN = (uint8_t)n;
            if (specLen == 6 && strncmp(spec, "Random", 6) == 0)
                out->discardRandom = 1, out->discardSpec = "Card";
            else if (specLen == 4 && strncmp(spec, "Hand", 4) == 0)
                return 0;
            else {
                out->discardSpec = KeepSpec(spec, specLen);
                if (!Mtg_ValidSpecKnown(out->discardSpec))
                    return 0;
            }
        } else if (ManaSymbol(tok) >= 0 || strcmp(tok, "X") == 0 || strspn(tok, "0123456789") == strlen(tok)) {
            if (strlen(mana) + strlen(tok) + 2 > sizeof(mana))
                return 0;
            if (mana[0])
                strcat(mana, " ");
            strcat(mana, tok);
        } else
            return 0;
    }
    return Mtg_ParseManaCost(mana, &out->mana);
}

static int ParseAbility(MtgCardDef *c, int cardId, char *text, int kind, int depth);

static int SubAbility(MtgCardDef *c, int cardId, const char *name, int depth)
{
    int i;
    if (depth > 8)
        return -1;
    for (i = 0; i < c->svarCount; i++) {
        MtgParam *sv = &gMtgSvars[c->svarFirst + i];
        if (strcmp(sv->key, name) == 0 && strncmp(sv->value, "DB$", 3) == 0) {
            // cut a copy (an SVar can be the sub-ability of more than one ability)
            size_t len = strlen(sv->value) + 1;
            char *copy = malloc(len);
            memcpy(copy, sv->value, len);
            return ParseAbility(c, cardId, copy, MTG_AB_SUB, depth + 1);
        }
    }
    return -1;
}

static int ParseAbility(MtgCardDef *c, int cardId, char *text, int kind, int depth)
{
    int first, n, idx;
    MtgAbility *ab;
    const char *sub, *v;
    n = Mtg_ParseParams(text, &first);
    gMtgAbilities = Grow(gMtgAbilities, &sAbilityCap, sAbilityCount + 1, sizeof(MtgAbility));
    idx = sAbilityCount++;
    ab = &gMtgAbilities[idx];
    memset(ab, 0, sizeof(*ab));
    ab->kind = kind;
    ab->card = cardId;
    ab->sub = 0xFFFF;
    ab->paramFirst = first;
    ab->paramCount = n;
    if (n == 0) {
        Unsupported(c, "ability without an API");
        return idx;
    }
    ab->api = Mtg_ApiKnown(gMtgParams[first].value);
    if (!ab->api) {
        Unsupported(c, gMtgParams[first].value);
        return idx;
    }
    if (!ParseCost(Mtg_Param(ab, "Cost"), &ab->cost))
        Unsupported(c, "cost");
    else if (ab->api == MTG_API_MANA && (ab->cost.sacN || ab->cost.discardN || ab->cost.payLife || ab->cost.exileSelf
                                         || ab->cost.returnSelf))
        Unsupported(c, "cost");   // (mana abilities are used by paying: one with a cost to pick, not yet)
    if (kind == MTG_AB_ACTIVATED && !Mtg_Param(ab, "Cost"))
        Unsupported(c, "cost");
    if (kind != MTG_AB_SPELL && kind != MTG_AB_ACTIVATED && Mtg_Param(ab, "Cost"))
        Unsupported(c, "optional cost");   // (a trigger's "you may pay ...": not yet)
    ab->tgt = Mtg_Param(ab, "ValidTgts");
    v = Mtg_Param(ab, "TargetType");
    if (v) {
        if (strcmp(v, "Spell") == 0)
            ab->tgtStack = 1;
        else
            Unsupported(c, "TargetType");
    }
    if (ab->tgt && !Mtg_ValidSpecKnown(ab->tgt))
        Unsupported(c, "ValidTgts");
    ab->tgtZone = MTG_ZONE_BATTLEFIELD;   // (ChangeZone targets cards where they come from: a graveyard)
    if (ab->api == MTG_API_CHANGE_ZONE && (v = Mtg_Param(ab, "Origin")) && strcmp(v, "Graveyard") == 0)
        ab->tgtZone = MTG_ZONE_GRAVEYARD;
    {
        const char *why;
        if (!Mtg_ApiParamsKnown(ab, &why))
            Unsupported(c, why);
    }
    sub = Mtg_Param(ab, "SubAbility");
    if (sub) {
        int s = SubAbility(c, cardId, sub, depth);
        if (s < 0)
            Unsupported(c, "SubAbility");
        gMtgAbilities[idx].sub = s < 0 ? 0xFFFF : s;   // (the table may have moved)
    }
    return idx;
}

int Mtg_ParseAbilityText(MtgCardDef *c, int cardId, char *text, int kind)
{
    return ParseAbility(c, cardId, text, kind, 0);
}

static const struct { const char *name; int kw; } sKeywords[] = {
    {"Flying", MTG_KW_FLYING}, {"Reach", MTG_KW_REACH}, {"Haste", MTG_KW_HASTE}, {"Vigilance", MTG_KW_VIGILANCE},
    {"Defender", MTG_KW_DEFENDER}, {"First Strike", MTG_KW_FIRST_STRIKE}, {"Double Strike", MTG_KW_DOUBLE_STRIKE},
    {"Trample", MTG_KW_TRAMPLE}, {"Shroud", MTG_KW_SHROUD}, {"Fear", MTG_KW_FEAR}, {"Flash", MTG_KW_FLASH},
    {"Lifelink", MTG_KW_LIFELINK},
};
static const char *const sColorNames[5] = {"white", "blue", "black", "red", "green"};
static const char *const sBasicTypes[5] = {"Plains", "Island", "Swamp", "Mountain", "Forest"};

int Mtg_KeywordByName(const char *s)
{
    unsigned i;
    for (i = 0; i < sizeof(sKeywords) / sizeof(sKeywords[0]); i++)
        if (strcmp(s, sKeywords[i].name) == 0)
            return sKeywords[i].kw;
    return -1;
}

static void ParseKeyword(MtgCardDef *c, const char *k)
{
    int i, kw = Mtg_KeywordByName(k);
    if (kw >= 0) {
        c->keywords |= MTG_KW_BIT(kw);
        return;
    }
    if (strncmp(k, "Protection from ", 16) == 0)
        for (i = 0; i < 5; i++)
            if (strcmp(k + 16, sColorNames[i]) == 0) {
                c->protection |= MTG_COLOR_BIT(i);
                return;
            }
    if (strcmp(k, "Enchant:Creature") == 0) {
        c->enchant = 1;
        return;
    }
    if (strncmp(k, "Landwalk:", 9) == 0)
        for (i = 0; i < 5; i++)
            if (strcmp(k + 9, sBasicTypes[i]) == 0) {
                c->landwalk |= MTG_COLOR_BIT(i);
                return;
            }
    Unsupported(c, k);
}

static void ParseTypes(MtgCardDef *c, char *s)
{
    static const struct { const char *name; uint8_t bit, super; } sTypes[] = {
        {"Land", MTG_T_LAND, 0}, {"Creature", MTG_T_CREATURE, 0}, {"Artifact", MTG_T_ARTIFACT, 0},
        {"Enchantment", MTG_T_ENCHANTMENT, 0}, {"Instant", MTG_T_INSTANT, 0}, {"Sorcery", MTG_T_SORCERY, 0},
        {"Planeswalker", MTG_T_PLANESWALKER, 0}, {"Tribal", MTG_T_TRIBAL, 0}, {"Kindred", MTG_T_TRIBAL, 0},
        {"Basic", MTG_ST_BASIC, 1}, {"Legendary", MTG_ST_LEGENDARY, 1}, {"Snow", MTG_ST_SNOW, 1},
        {"World", MTG_ST_WORLD, 1},
    };
    char *tok, *save, *subs = s;   // the subtypes are written back into the same buffer
    size_t used = 0;
    for (save = s, tok = Mtg_Token(&save, ' '); tok; tok = Mtg_Token(&save, ' ')) {
        unsigned i;
        int known = 0;
        for (i = 0; i < sizeof(sTypes) / sizeof(sTypes[0]); i++)
            if (strcmp(tok, sTypes[i].name) == 0) {
                if (sTypes[i].super)
                    c->supertypes |= sTypes[i].bit;
                else
                    c->types |= sTypes[i].bit;
                known = 1;
            }
        if (!known) {
            size_t len = strlen(tok);
            for (i = 0; i < 5; i++)
                if (strcmp(tok, sBasicTypes[i]) == 0)
                    c->intrinsicMana |= MTG_COLOR_BIT(i);
            if (used)
                subs[used++] = ' ';
            memmove(subs + used, tok, len);
            used += len;
        }
    }
    subs[used] = 0;
    c->subtypes = subs;
    if (!c->types)
        Unsupported(c, "type");
    if (c->types & MTG_T_PLANESWALKER)
        Unsupported(c, "Planeswalker");
}

static int HasWordI(const char *s, const char *word)   // (case-insensitive: Forge writes "Color:Green")
{
    size_t n = strlen(word);
    for (; *s; s++) {
        size_t i;
        for (i = 0; i < n && s[i] && (s[i] | 32) == (word[i] | 32); i++)
            ;
        if (i == n)
            return 1;
    }
    return 0;
}

#define MTG_SCRIPT_LINES 160

static void ParseCard(int id, char *text)
{
    MtgCardDef *c = &sCards[id];
    char *lines[MTG_SCRIPT_LINES], *abilityLines[8], *triggerLines[8], *line, *next;
    int nLines = 0, nAbil = 0, nTrig = 0, i, k;
    memset(c, 0, sizeof(*c));
    c->supported = 1;
    c->oracle = "";
    c->subtypes = "";
    c->svarFirst = sSvarCount;
    for (line = text; line && *line; line = next) {
        next = strchr(line, '\n');
        if (next)
            *next++ = 0;
        if (strcmp(line, "ALTERNATE") == 0) {   // a split or flip card's other face: not yet
            Unsupported(c, "ALTERNATE");
            break;
        }
        if (line[0] == '#' || !strchr(line, ':'))
            continue;
        if (nLines == MTG_SCRIPT_LINES) {
            Unsupported(c, "too many lines");
            break;
        }
        lines[nLines++] = line;
    }
    // the SVars first: abilities name their sub-abilities by SVar ("SVar:DBGainLife:DB$ GainLife | ...")
    for (k = 0; k < nLines; k++) {
        char *name, *val;
        if (strncmp(lines[k], "SVar:", 5) != 0)
            continue;
        name = lines[k] + 5;
        val = strchr(name, ':');
        lines[k] = NULL;
        if (!val)
            continue;
        *val++ = 0;
        gMtgSvars = Grow(gMtgSvars, &sSvarCap, sSvarCount + 1, sizeof(MtgParam));
        gMtgSvars[sSvarCount].key = name;
        gMtgSvars[sSvarCount].value = Trim(val);
        sSvarCount++;
        c->svarCount++;
    }
    for (k = 0; k < nLines; k++) {
        char *colon, *val;
        if (!lines[k])
            continue;
        line = lines[k];
        colon = strchr(line, ':');
        *colon = 0;
        val = colon + 1;
        if (strcmp(line, "Name") == 0)
            c->name = val;
        else if (strcmp(line, "ManaCost") == 0) {
            if (!Mtg_ParseManaCost(val, &c->cost))
                Unsupported(c, "mana cost");
            for (i = 0; i < 5; i++)
                if (c->cost.pip[i])
                    c->colors |= MTG_COLOR_BIT(i);
        } else if (strcmp(line, "Types") == 0)
            ParseTypes(c, val);
        else if (strcmp(line, "PT") == 0) {
            char *slash = strchr(val, '/');
            if (!slash || strchr(val, '*'))
                Unsupported(c, "* power or toughness");
            else {
                c->power = (int8_t)atoi(val);
                c->toughness = (int8_t)atoi(slash + 1);
            }
        } else if (strcmp(line, "K") == 0)
            ParseKeyword(c, val);
        else if (strcmp(line, "A") == 0) {
            if (nAbil < 8)
                abilityLines[nAbil++] = val;
            else
                Unsupported(c, "too many abilities");
        } else if (strcmp(line, "Oracle") == 0)
            c->oracle = val;
        else if (strcmp(line, "T") == 0) {
            if (nTrig < 8)
                triggerLines[nTrig++] = val;
            else
                Unsupported(c, "too many triggers");
        }
        else if (strcmp(line, "S") == 0)
            Mtg_ParseStatic(c, id, val);
        else if (strcmp(line, "R") == 0)
            Mtg_ParseReplacement(c, id, val);
        else if (strcmp(line, "AlternateMode") == 0)
            Unsupported(c, "AlternateMode");
        else if (strcmp(line, "Color") == 0) {
            c->colors = 0;
            for (i = 0; i < 5; i++)
                if (HasWordI(val, sColorNames[i]))
                    c->colors |= MTG_COLOR_BIT(i);
        } else if (strcmp(line, "AI") != 0 && strcmp(line, "DeckHas") != 0 && strcmp(line, "DeckHints") != 0
                   && strcmp(line, "DeckNeeds") != 0 && strcmp(line, "Text") != 0)
            Unsupported(c, "script line");
    }
    for (i = 0; i < nAbil; i++) {
        int kind = strncmp(abilityLines[i], "SP$", 3) == 0 ? MTG_AB_SPELL
                 : strncmp(abilityLines[i], "AB$", 3) == 0 ? MTG_AB_ACTIVATED : -1;
        if (kind < 0) {
            Unsupported(c, "ability kind");
            continue;
        }
        c->abilities[c->abilityCount++] = (uint16_t)ParseAbility(c, id, abilityLines[i], kind, 0);
    }
    if (c->enchant) {   // an Aura's spell (Forge gives it one: "SP$ Attach | ValidTgts$ Creature")
        static const char sAura[] = "SP$ Attach | ValidTgts$ Creature";
        char *text = malloc(sizeof(sAura));
        int spells = 0;
        memcpy(text, sAura, sizeof(sAura));
        for (i = 0; i < c->abilityCount; i++)
            spells += gMtgAbilities[c->abilities[i]].kind == MTG_AB_SPELL;
        if (spells || !(c->types & MTG_T_ENCHANTMENT) || c->abilityCount >= 8)
            Unsupported(c, "Enchant:Creature");
        else
            c->abilities[c->abilityCount++] = (uint16_t)ParseAbility(c, id, text, MTG_AB_SPELL, 0);
    }
    for (i = 0; i < nTrig; i++)
        Mtg_ParseTrigger(c, id, triggerLines[i]);
    if ((c->types & (MTG_T_INSTANT | MTG_T_SORCERY))) {
        int spells = 0;
        for (i = 0; i < c->abilityCount; i++)
            spells += gMtgAbilities[c->abilities[i]].kind == MTG_AB_SPELL;
        if (spells != 1)
            Unsupported(c, spells ? "modes" : "no spell ability");
    }
    if (!c->name)
        Unsupported(c, "no name");
}

// Every subtype a card of the pool has (from each Types: line, before the cards are parsed: the cards' Valid specs are
// checked against it as they load), sorted
static char **sSubtypes;
static int sSubtypeCount;

static int CmpStr(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static void CollectSubtypes(const char *text, int cardCount, const uint8_t *table)
{
    static const char *const sNot[] = {"Land", "Creature", "Artifact", "Enchantment", "Instant", "Sorcery",
                                       "Planeswalker", "Tribal", "Kindred", "Basic", "Legendary", "Snow", "World"};
    int i, cap = 0, k, n = 0;
    for (i = 0; i < cardCount; i++) {
        uint32_t e[4];
        const char *t, *line, *end;
        memcpy(e, table + 16 * i, 16);
        t = text + e[1];
        line = strstr(t, "Types:");
        while (line && line != t && line[-1] != '\n')
            line = strstr(line + 1, "Types:");
        if (!line)
            continue;
        line += 6;
        end = strchr(line, '\n');
        if (!end)
            end = line + strlen(line);
        while (line < end) {
            const char *w = line;
            size_t len;
            while (line < end && *line != ' ')
                line++;
            len = (size_t)(line - w);
            while (line < end && *line == ' ')
                line++;
            if (!len)
                continue;
            for (k = 0; k < (int)(sizeof(sNot) / sizeof(sNot[0])); k++)
                if (strlen(sNot[k]) == len && strncmp(sNot[k], w, len) == 0)
                    break;
            if (k < (int)(sizeof(sNot) / sizeof(sNot[0])))
                continue;
            if (n == cap)
                cap = cap ? cap * 2 : 1024, sSubtypes = realloc(sSubtypes, cap * sizeof(char *));
            sSubtypes[n] = malloc(len + 1);
            memcpy(sSubtypes[n], w, len);
            sSubtypes[n++][len] = 0;
        }
    }
    qsort(sSubtypes, n, sizeof(char *), CmpStr);
    for (i = k = 0; i < n; i++) {   // (each once)
        if (k && strcmp(sSubtypes[k - 1], sSubtypes[i]) == 0) {
            free(sSubtypes[i]);
            continue;
        }
        sSubtypes[k++] = sSubtypes[i];
    }
    sSubtypeCount = k;
}

int Mtg_IsSubtype(const char *t, size_t n)
{
    int lo = 0, hi = sSubtypeCount - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2, c = strncmp(sSubtypes[mid], t, n);
        if (c == 0 && sSubtypes[mid][n])
            c = 1;
        if (c == 0)
            return 1;
        if (c < 0)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

int Mtg_DbLoad(const void *pak, uint32_t size)
{
    const uint8_t *p = pak;
    uint32_t count, version, i, textOff;
    if (sCards)
        return sCardCount;
    if (size < 12 || memcmp(p, "MTGF", 4) != 0)
        return 0;
    memcpy(&version, p + 4, 4);
    memcpy(&count, p + 8, 4);
    textOff = 12 + 16 * count;
    if (version != 2 || textOff > size)
        return 0;
    sText = malloc(size - textOff + 1);
    memcpy(sText, p + textOff, size - textOff);
    sText[size - textOff] = 0;
    sCards = calloc(count, sizeof(MtgCardDef));
    sCardCount = (int)count;
    CollectSubtypes(sText, (int)count, p + 12);
    for (i = 0; i < count; i++) {
        uint32_t e[4];
        memcpy(e, p + 12 + 16 * i, 16);
        ParseCard((int)i, sText + e[1]);
        sCards[i].name = sText + e[0];   // (the pack's own name, the one it is sorted by)
        sCards[i].rarity = (uint8_t)(e[3] & 3);
        sCards[i].banned = (uint8_t)((e[3] >> 2) & 1);
    }
    return sCardCount;
}
