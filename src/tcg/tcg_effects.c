#include "mp_version.h"
#if MOEMON_DEV_BUILD || defined(TCG_TEST)
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <stdio.h>
#include <string.h>
enum
{
    FX_PARALYZE_COIN,
    FX_POISON,
    FX_POISON_COIN,
    FX_CONFUSE_COIN,
    FX_SLEEP,
    FX_SLEEP_COIN,
    FX_DRAIN,
    FX_BARRIER_COIN,
    FX_BONUS_COIN,
    FX_RAGE,
    FX_MEDITATE,
    FX_PSYWAVE,
    FX_PSYCHIC,
    FX_KARATE_CHOP,
    FX_SUPER_FANG,
    FX_MULTI_COIN,
    FX_DO_THE_WAVE,
    FX_WATER_EXTRA
};
struct Effect
{
    const char *name;
    unsigned kind, amount, after;
};
static const struct Effect sEffects[] = {
#include "tcg_effect_table.inc"
};
static const struct Effect *Effect(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof(sEffects) / sizeof(sEffects[0]); i++)
        if (!strcmp(name, sEffects[i].name))
            return &sEffects[i];
    return NULL;
}
int TcgDuel_EffectSupported(const char *name) { return !strcmp(name, "NONE") || Effect(name) != NULL; }
int TcgDuel_AttackEffect(struct TcgDuel *g, const struct TcgAttack *a, const struct TcgAction *action, unsigned *damage,
                         int after)
{
    const struct Effect *e = Effect(a->effect);
    struct TcgFieldCard *self = &g->players[g->current].field[0], *foe = &g->players[g->current ^ 1].field[0];
    unsigned n = 0, i;
    (void)action;
    if (!strcmp(a->effect, "NONE"))
        return TCG_OK;
    if (!e)
        return TCG_UNSUPPORTED;
    if (e->after != (unsigned)after)
        return TCG_OK;
    switch (e->kind)
    {
    case FX_PARALYZE_COIN:
        if (Tcg_RandomBelow(&g->rng, 2))
            foe->condition = TCG_PARALYZED;
        break;
    case FX_POISON:
        foe->poison = e->amount;
        break;
    case FX_POISON_COIN:
        if (Tcg_RandomBelow(&g->rng, 2))
            foe->poison = e->amount;
        break;
    case FX_CONFUSE_COIN:
        if (Tcg_RandomBelow(&g->rng, 2))
            foe->condition = TCG_CONFUSED;
        break;
    case FX_SLEEP:
        foe->condition = TCG_ASLEEP;
        break;
    case FX_SLEEP_COIN:
        if (Tcg_RandomBelow(&g->rng, 2))
            foe->condition = TCG_ASLEEP;
        break;
    case FX_DRAIN:
        if (*damage)
            self->damage = self->damage > e->amount ? self->damage - e->amount : 0;
        break;
    case FX_BARRIER_COIN:
        if (Tcg_RandomBelow(&g->rng, 2))
            self->barrier = 1;
        break;
    case FX_BONUS_COIN:
        if (Tcg_RandomBelow(&g->rng, 2))
            *damage += e->amount;
        break;
    case FX_RAGE:
        *damage += self->damage;
        break;
    case FX_MEDITATE:
        *damage += foe->damage;
        break;
    case FX_PSYWAVE:
        *damage = 10 * TcgDuel_Energy(g, g->current ^ 1, 0, TCG_NONE);
        break;
    case FX_PSYCHIC:
        *damage += 10 * TcgDuel_Energy(g, g->current ^ 1, 0, TCG_NONE);
        break;
    case FX_KARATE_CHOP:
        *damage = *damage > self->damage ? *damage - self->damage : 0;
        break;
    case FX_SUPER_FANG:
    {
        const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
        unsigned left = c->hp > foe->damage ? c->hp - foe->damage : 0;
        *damage = ((left + 19) / 20) * 10;
        break;
    }
    case FX_MULTI_COIN:
        for (i = 0; i < e->amount; i++)
            n += Tcg_RandomBelow(&g->rng, 2);
        *damage *= n;
        break;
    case FX_DO_THE_WAVE:
        for (i = 1; i < TCG_FIELD_SIZE; i++)
            if (g->players[g->current].field[i].stackSize)
                *damage += 10;
        break;
    case FX_WATER_EXTRA:
        n = TcgDuel_Energy(g, g->current, 0, TCG_WATER);
        if (n > e->amount)
        {
            n -= e->amount;
            *damage += 10 * (n > 2 ? 2 : n);
        }
        break;
    default:
        return TCG_UNSUPPORTED;
    }
    /* Fossil and Doll are immune to special conditions. */
    if (TcgDuel_Field(g, g->current ^ 1, 0)->kind == TCG_TRAINER)
    {
        foe->condition = TCG_HEALTHY;
        foe->poison = 0;
    }
    return TCG_OK;
}
/* Powers and card-specific trainer choices are ported separately. */
int TcgDuel_Power(struct TcgDuel *g, const struct TcgAction *a)
{
    (void)a;
    snprintf(g->error, sizeof(g->error), "This Pokemon Power has not been ported yet.");
    return TCG_UNSUPPORTED;
}
#endif
