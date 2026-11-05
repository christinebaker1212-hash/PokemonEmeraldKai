// Moemon Kai: Forge's "Valid" text (ValidTgts$, ValidCards$...): CardProperty.java / PlayerProperty.java and
// CardType's type tests. "Creature.nonBlack+YouCtrl,Player": alternatives by ',', a type, then '.' and properties
// joined by '+'. Only what is written here is known; anything else makes a card unsupported (Mtg_ValidSpecKnown).
#include <stdlib.h>
#include <string.h>
#include "mtg_internal.h"

static const char *const sColorProps[5] = {"White", "Blue", "Black", "Red", "Green"};

uint8_t Mtg_ObjColors(const MtgDuel *d, int obj)
{
    return Mtg_Card(d->objs[obj].def)->colors;
}

static int TypeBit(const char *t, size_t n)
{
    static const struct { const char *name; int bit; } sT[] = {
        {"Land", MTG_T_LAND}, {"Creature", MTG_T_CREATURE}, {"Artifact", MTG_T_ARTIFACT},
        {"Enchantment", MTG_T_ENCHANTMENT}, {"Instant", MTG_T_INSTANT}, {"Sorcery", MTG_T_SORCERY},
        {"Planeswalker", MTG_T_PLANESWALKER},
        {"Permanent", MTG_T_LAND | MTG_T_CREATURE | MTG_T_ARTIFACT | MTG_T_ENCHANTMENT | MTG_T_PLANESWALKER},
        {"Card", 0xFF},
    };
    unsigned i;
    for (i = 0; i < sizeof(sT) / sizeof(sT[0]); i++)
        if (strlen(sT[i].name) == n && strncmp(t, sT[i].name, n) == 0)
            return sT[i].bit;
    return 0;
}

static int HasSubtype(const MtgCardDef *c, const char *t, size_t n)
{
    const char *s = c->subtypes;
    while (*s) {
        const char *e = strchr(s, ' ');
        size_t len = e ? (size_t)(e - s) : strlen(s);
        if (len == n && strncmp(s, t, n) == 0)
            return 1;
        if (!e)
            break;
        s = e + 1;
    }
    return 0;
}

static int StartsWith(const char *p, size_t n, const char *word)
{
    size_t w = strlen(word);
    return n >= w && strncmp(p, word, w) == 0;
}

static int Number(const char *p, size_t n, int *out)
{
    size_t i;
    if (n == 0)
        return 0;
    for (i = 0; i < n; i++)
        if (p[i] < '0' || p[i] > '9')
            return 0;
    *out = atoi(p);
    return 1;
}

// one card property ("nonBlack", "YouCtrl", "powerLE2"...); *known = 0 if it isn't translated
static int CardProp(const MtgDuel *d, int obj, const char *p, size_t n, int src, int controller, int *known)
{
    const MtgObj *o = d ? &d->objs[obj] : NULL;
    const MtgCardDef *c = d ? Mtg_Card(o->def) : NULL;
    int i, bit, neg = 0, num;
    if (StartsWith(p, n, "non")) {
        neg = 1;
        p += 3;
        n -= 3;
    }
    for (i = 0; i < 5; i++)
        if (strlen(sColorProps[i]) == n && strncmp(p, sColorProps[i], n) == 0)
            return !d || (((Mtg_ObjColors(d, obj) & MTG_COLOR_BIT(i)) != 0) ^ neg);
    if (n == 9 && strncmp(p, "Colorless", 9) == 0)
        return !d || ((Mtg_ObjColors(d, obj) == 0) ^ neg);
    if ((bit = TypeBit(p, n)) != 0 && bit != 0xFF)
        return !d || (((c->types & bit) != 0) ^ neg);
    if (n == 5 && strncmp(p, "Basic", 5) == 0)
        return !d || (((c->supertypes & MTG_ST_BASIC) != 0) ^ neg);
    if (n == 9 && strncmp(p, "Legendary", 9) == 0)
        return !d || (((c->supertypes & MTG_ST_LEGENDARY) != 0) ^ neg);
    if (neg) {   // a subtype ("nonWall")
        if (p[0] >= 'A' && p[0] <= 'Z')
            return !d || !HasSubtype(c, p, n);
        *known = 0;
        return 0;
    }
    if (n == 7 && strncmp(p, "YouCtrl", 7) == 0)
        return !d || o->controller == controller;
    if (n == 7 && strncmp(p, "OppCtrl", 7) == 0)
        return !d || o->controller != controller;
    if (n == 6 && strncmp(p, "YouOwn", 6) == 0)
        return !d || o->owner == controller;
    if (n == 5 && strncmp(p, "Other", 5) == 0)
        return !d || obj != src;
    if (n == 4 && strncmp(p, "Self", 4) == 0)   // (the card the ability or trigger belongs to)
        return !d || obj == src;
    if (n == 6 && strncmp(p, "tapped", 6) == 0)
        return !d || o->tapped;
    if (n == 8 && strncmp(p, "untapped", 8) == 0)
        return !d || !o->tapped;
    if (n == 9 && strncmp(p, "attacking", 9) == 0)
        return !d || o->attacking;
    if (n == 8 && strncmp(p, "blocking", 8) == 0)
        return !d || o->blocking != MTG_NO_OBJ;
    if (StartsWith(p, n, "without") || StartsWith(p, n, "with")) {
        int without = StartsWith(p, n, "without");
        size_t skip = without ? 7 : 4;
        char kw[32];
        int k;
        if (n - skip >= sizeof(kw)) {
            *known = 0;
            return 0;
        }
        memcpy(kw, p + skip, n - skip);
        kw[n - skip] = 0;
        if (strcmp(kw, "FirstStrike") == 0)
            strcpy(kw, "First Strike");
        k = Mtg_KeywordByName(kw);
        if (k < 0) {
            *known = 0;
            return 0;
        }
        return !d || (((Mtg_Keywords(d, obj) & MTG_KW_BIT(k)) != 0) ^ without);
    }
    {
        static const char *const sStats[3] = {"power", "toughness", "cmc"};
        for (i = 0; i < 3; i++) {
            size_t l = strlen(sStats[i]);
            if (n > l + 2 && strncmp(p, sStats[i], l) == 0 && Number(p + l + 2, n - l - 2, &num)) {
                const char *op = p + l;
                int v;
                if (!(op[0] == 'L' || op[0] == 'G' || op[0] == 'E') || !(op[1] == 'E' || op[1] == 'T' || op[1] == 'Q')) {
                    *known = 0;
                    return 0;
                }
                if (!d)
                    return 1;
                v = i == 0 ? Mtg_Power(d, obj) : i == 1 ? Mtg_Toughness(d, obj) : Mtg_ManaCostTotal(&c->cost);
                if (op[0] == 'L')
                    return op[1] == 'E' ? v <= num : v < num;
                if (op[0] == 'G')
                    return op[1] == 'E' ? v >= num : v > num;
                return v == num;
            }
        }
    }
    if (p[0] >= 'A' && p[0] <= 'Z' && n < 24) {   // a subtype ("Creature.Goblin")
        for (i = 0; i < (int)n; i++)
            if (!((p[i] >= 'a' && p[i] <= 'z') || (p[i] >= 'A' && p[i] <= 'Z')))
                break;
        if (i == (int)n)
            return !d || HasSubtype(c, p, n);
    }
    *known = 0;
    return 0;
}

// one alternative ("Creature.nonBlack+YouCtrl") against an object or a player
static int Alternative(const MtgDuel *d, int isPlayer, int id, const char *a, size_t n, int src, int controller,
                       int *known)
{
    const char *dot = memchr(a, '.', n);
    size_t tn = dot ? (size_t)(dot - a) : n;
    int bit, ok = 1;
    if ((tn == 3 && strncmp(a, "Any", 3) == 0) && !dot) {
        if (!d)
            return 1;
        return isPlayer || (Mtg_Card(d->objs[id].def)->types & (MTG_T_CREATURE | MTG_T_PLANESWALKER));
    }
    if ((tn == 6 && strncmp(a, "Player", 6) == 0) || (tn == 8 && strncmp(a, "Opponent", 8) == 0)
        || (tn == 3 && strncmp(a, "You", 3) == 0)) {
        if (dot) {   // "Player.Opponent", "Player.You"
            size_t pn = n - tn - 1;
            const char *pp = dot + 1;
            if (!((pn == 8 && strncmp(pp, "Opponent", 8) == 0) || (pn == 3 && strncmp(pp, "You", 3) == 0))) {
                *known = 0;
                return 0;
            }
            if (!d)
                return 1;
            return isPlayer && ((pp[0] == 'O') ? id != controller : id == controller);
        }
        if (!d)
            return 1;
        if (!isPlayer)
            return 0;
        return a[0] == 'P' || (a[0] == 'O' ? id != controller : id == controller);
    }
    bit = TypeBit(a, tn);
    if (!bit) {   // a bare subtype ("Goblin")
        if (!(a[0] >= 'A' && a[0] <= 'Z')) {
            *known = 0;
            return 0;
        }
    }
    if (d && isPlayer)
        return 0;
    if (d) {
        const MtgCardDef *c = Mtg_Card(d->objs[id].def);
        if (bit && bit != 0xFF && !(c->types & bit))
            return 0;
        if (!bit && !HasSubtype(c, a, tn))
            return 0;
    }
    if (dot) {
        const char *p = dot + 1, *end = a + n;
        while (p < end) {
            const char *plus = memchr(p, '+', (size_t)(end - p));
            size_t pn = plus ? (size_t)(plus - p) : (size_t)(end - p);
            if (!CardProp(d, id, p, pn, src, controller, known))
                ok = 0;
            if (!*known)
                return 0;
            p += pn + 1;
        }
    }
    return ok;
}

static int Valid(const MtgDuel *d, int isPlayer, int id, const char *spec, int src, int controller, int *known)
{
    const char *p = spec;
    int any = 0;
    *known = 1;
    while (*p) {
        const char *comma = strchr(p, ',');
        size_t n = comma ? (size_t)(comma - p) : strlen(p);
        while (n && *p == ' ')
            p++, n--;
        while (n && p[n - 1] == ' ')
            n--;
        if (Alternative(d, isPlayer, id, p, n, src, controller, known))
            any = 1;
        if (!*known)
            return 0;
        if (!comma)
            break;
        p = comma + 1;
    }
    return any;
}

int Mtg_ValidObj(const MtgDuel *d, int obj, const char *spec, int sourceObj, int controller)
{
    int known;
    return Valid(d, 0, obj, spec, sourceObj, controller, &known) && known;
}

int Mtg_ValidPlayer(const MtgDuel *d, int player, const char *spec, int controller)
{
    int known;
    return Valid(d, 1, player, spec, MTG_NO_OBJ, controller, &known) && known;
}

int Mtg_ValidSpecKnown(const char *spec)
{
    int known;
    Valid(NULL, 0, 0, spec, 0, 0, &known);
    return known;
}

int Mtg_CanTarget(const MtgDuel *d, int obj, int sourceObj)
{
    const MtgCardDef *c = Mtg_Card(d->objs[obj].def);
    if (Mtg_Keywords(d, obj) & MTG_KW_BIT(MTG_KW_SHROUD))
        return 0;
    if (sourceObj != MTG_NO_OBJ && (c->protection & Mtg_ObjColors(d, sourceObj)))
        return 0;
    return 1;
}
