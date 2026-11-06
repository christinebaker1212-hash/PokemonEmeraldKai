// Moemon Kai: mana: what a permanent's {T} adds (basic land types' intrinsic abilities, AB$ Mana), and paying a cost
// from the floating pool and untapped sources: ManaPool.java and the auto-payment of ComputerUtilMana.java. A
// backtracking search gives each colored symbol a source that can make it, so a dual land is never spent on the
// wrong one; the generic part takes the least flexible sources left. Same state, same choice: deterministic.
#include <string.h>
#include "mtg_internal.h"

#define MTG_MAX_SOURCES 40

typedef struct {
    uint16_t obj;
    uint8_t colors;     // MTG_COLOR_BIT of W..C
    uint8_t amount;
} Source;

int Mtg_ProducedColors(const char *produced)
{
    static const char sym[] = "WUBRGC";
    int mask = 0;
    const char *p;
    if (!produced)
        return 0;
    if (strcmp(produced, "Any") == 0)
        return 0x1F;
    if (strncmp(produced, "Combo ", 6) == 0)
        produced += 6;
    for (p = produced; *p; p++) {
        const char *s = strchr(sym, *p);
        if (*p == ' ')
            continue;
        if (!s || (p[1] && p[1] != ' '))
            return -1;
        mask |= 1 << (s - sym);
    }
    return mask;
}

// a usable "{T}: Add ..." (only the tap cost, nothing else)
int Mtg_ManaAbilityColors(const MtgDuel *d, int obj, int *amount)
{
    const MtgObj *o = &d->objs[obj];
    const MtgCardDef *c = Mtg_Card(o->def);
    int i, mask = 0;
    *amount = 1;
    if (o->zone != MTG_ZONE_BATTLEFIELD || o->tapped)
        return 0;
    if ((c->types & MTG_T_CREATURE) && o->sick && !(Mtg_Keywords(d, obj) & MTG_KW_BIT(MTG_KW_HASTE)))
        return 0;
    uint16_t abil[16];
    int abilCount = Mtg_ObjAbilities(d, obj, abil, 16);   // (Citanul Hierophants' creatures too)
    mask = c->intrinsicMana;
    for (i = 0; i < abilCount; i++) {
        const MtgAbility *ab = &gMtgAbilities[abil[i]];
        const char *amt;
        int m;
        if (ab->kind != MTG_AB_ACTIVATED || ab->api != MTG_API_MANA || !ab->cost.tap || ab->cost.sacSelf
            || Mtg_ManaCostTotal(&ab->cost.mana) || ab->cost.mana.x)
            continue;
        m = Mtg_ProducedColors(Mtg_Param(ab, "Produced"));
        if (m <= 0)
            continue;
        amt = Mtg_Param(ab, "Amount");
        if (amt)
            *amount = amt[0] >= '1' && amt[0] <= '9' ? amt[0] - '0' : 1;
        mask |= m;
    }
    return mask;
}

static int Sources(const MtgDuel *d, int player, int exclude, Source *out)
{
    int i, n = 0;
    for (i = 0; i < d->objCount && n < MTG_MAX_SOURCES; i++) {
        int amount, mask;
        if (i == exclude || d->objs[i].controller != player)
            continue;
        mask = Mtg_ManaAbilityColors(d, i, &amount);
        if (mask) {
            out[n].obj = (uint16_t)i;
            out[n].colors = (uint8_t)mask;
            out[n].amount = (uint8_t)amount;
            n++;
        }
    }
    return n;
}

static int Bits(int m)
{
    int n = 0;
    for (; m; m &= m - 1)
        n++;
    return n;
}

typedef struct {
    int need[MTG_MANA_KINDS];   // colored symbols still to pay after the pool
    int generic;
    int n;
    Source src[MTG_MAX_SOURCES];
    int8_t chosen[MTG_MAX_SOURCES];   // the color a used source makes (-1 unused)
    uint8_t left[MTG_MAX_SOURCES];    // its units not yet given to a symbol
} Solver;

static int Dfs(Solver *s, int color)
{
    int i;
    while (color < MTG_MANA_KINDS && s->need[color] == 0)
        color++;
    if (color == MTG_MANA_KINDS) {
        int units = 0;
        for (i = 0; i < s->n; i++)
            units += s->chosen[i] >= 0 ? s->left[i] : s->src[i].amount;
        return units >= s->generic;
    }
    // a source already making this color first, then new ones, least flexible first (src is sorted that way)
    for (i = 0; i < s->n; i++) {
        if (s->chosen[i] == color && s->left[i]) {
            s->left[i]--;
            s->need[color]--;
            if (Dfs(s, color))
                return 1;
            s->need[color]++;
            s->left[i]++;
            return 0;   // (another source of the same kind would be the same search)
        }
    }
    for (i = 0; i < s->n; i++) {
        if (s->chosen[i] < 0 && (s->src[i].colors & (1 << color))) {
            s->chosen[i] = (int8_t)color;
            s->left[i] = (uint8_t)(s->src[i].amount - 1);
            s->need[color]--;
            if (Dfs(s, color))
                return 1;
            s->need[color]++;
            s->chosen[i] = -1;
        }
    }
    return 0;
}

static int Solve(const MtgDuel *d, int player, const MtgManaCost *cost, int x, int exclude, Solver *s,
                 uint8_t poolLeft[MTG_MANA_KINDS])
{
    int i, j;
    const MtgPlayer *p = &d->players[player];
    memset(s, 0, sizeof(*s));
    memcpy(poolLeft, p->pool, MTG_MANA_KINDS);
    for (i = 0; i < MTG_MANA_KINDS; i++) {
        int use = cost->pip[i] < poolLeft[i] ? cost->pip[i] : poolLeft[i];
        s->need[i] = cost->pip[i] - use;
        poolLeft[i] -= use;
    }
    s->generic = cost->generic + x * cost->x;
    for (i = 0; i < MTG_MANA_KINDS && s->generic; i++) {   // the pool's leftover pays generic
        int use = s->generic < poolLeft[i] ? s->generic : poolLeft[i];
        s->generic -= use;
        poolLeft[i] -= use;
    }
    s->n = Sources(d, player, exclude, s->src);
    for (i = 1; i < s->n; i++)   // least flexible first (insertion sort keeps the object order otherwise)
        for (j = i; j > 0 && Bits(s->src[j - 1].colors) > Bits(s->src[j].colors); j--) {
            Source t = s->src[j];
            s->src[j] = s->src[j - 1];
            s->src[j - 1] = t;
        }
    for (i = 0; i < s->n; i++)
        s->chosen[i] = -1;
    return Dfs(s, 0);
}

int Mtg_CanPay(const MtgDuel *d, int player, const MtgManaCost *cost, int x, int excludeObj)
{
    Solver s;
    uint8_t pool[MTG_MANA_KINDS];
    return Solve(d, player, cost, x, excludeObj, &s, pool);
}

int Mtg_MaxX(const MtgDuel *d, int player, const MtgManaCost *cost, int excludeObj)
{
    int x = 0;
    if (!cost->x)
        return 0;
    while (x < 20 && Mtg_CanPay(d, player, cost, x + 1, excludeObj))
        x++;
    return x;
}

static int FirstColor(int mask)
{
    int c;
    for (c = 0; c < MTG_MANA_KINDS; c++)
        if (mask & (1 << c))
            return c;
    return MTG_C;
}

int Mtg_Pay(MtgDuel *d, int player, const MtgManaCost *cost, int x, int excludeObj)
{
    Solver s;
    uint8_t pool[MTG_MANA_KINDS];
    MtgPlayer *p = &d->players[player];
    int i, generic;
    if (!Solve(d, player, cost, x, excludeObj, &s, pool))
        return 0;
    // the sources the colored symbols chose, then the least flexible others for the generic part
    generic = s.generic;
    for (i = 0; i < s.n; i++)
        if (s.chosen[i] >= 0) {
            int give = s.left[i] < generic ? s.left[i] : generic;
            generic -= give;
        }
    for (i = 0; i < s.n && generic > 0; i++)
        if (s.chosen[i] < 0) {
            s.chosen[i] = (int8_t)FirstColor(s.src[i].colors);
            generic -= s.src[i].amount;
        }
    for (i = 0; i < s.n; i++)
        if (s.chosen[i] >= 0) {
            Mtg_Tap(d, s.src[i].obj, 1);
            p->pool[s.chosen[i]] += s.src[i].amount;
        }
    // spend: colored symbols from their color, the generic part from colorless first, then the fullest color
    for (i = 0; i < MTG_MANA_KINDS; i++)
        p->pool[i] -= cost->pip[i];
    generic = cost->generic + x * cost->x;
    while (generic > 0) {
        int best = MTG_C, c;
        if (!p->pool[MTG_C])
            for (c = 0, best = -1; c < MTG_C; c++)
                if (p->pool[c] && (best < 0 || p->pool[c] > p->pool[best]))
                    best = c;
        if (best < 0)
            return 0;   // (can't happen: Solve said it fits)
        p->pool[best]--;
        generic--;
    }
    return 1;
}
