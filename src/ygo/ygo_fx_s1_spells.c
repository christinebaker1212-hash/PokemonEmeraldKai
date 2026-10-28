/* Yu-Gi-Oh! effects, stage 1 (the Starter Decks, Legend of Blue Eyes, Metal Raiders): Spell Cards. */
#include <string.h>
#include "ygo_fx.h"

/* ---- burn and heal */
static void Op_Red(struct YgoCtx *c) { Ygo_Recover(S, ME, 500); }
static void Op_Remedy(struct YgoCtx *c) { Ygo_Recover(S, ME, 600); }
static void Op_DianKeto(struct YgoCtx *c) { Ygo_Recover(S, ME, 1000); }
static void Op_Sparks(struct YgoCtx *c) { Ygo_Damage(S, OPP, 200, 0); }
static void Op_Hinotama(struct YgoCtx *c) { Ygo_Damage(S, OPP, 500, 0); }
static void Op_FinalFlame(struct YgoCtx *c) { Ygo_Damage(S, OPP, 600, 0); }
static void Op_Ookazi(struct YgoCtx *c) { Ygo_Damage(S, OPP, 800, 0); }
static void Op_TremendousFire(struct YgoCtx *c)
{
    Ygo_Damage(S, OPP, 1000, 0);
    Ygo_Damage(S, ME, 500, 0);
}

/* ---- mass destruction */
static int Can_DarkHole(struct YgoCtx *c) { return Ygo_MonsterCount(S, 0) + Ygo_MonsterCount(S, 1) > 0; }
static void Op_DarkHole(struct YgoCtx *c) { Fx_DestroyAll(S, 2, YL_MZONE, NULL, YGO_REASON_EFFECT); }
static int Can_Raigeki(struct YgoCtx *c) { return Ygo_MonsterCount(S, OPP) > 0; }
static void Op_Raigeki(struct YgoCtx *c) { Fx_DestroyAll(S, OPP, YL_MZONE, NULL, YGO_REASON_EFFECT); }

static int NotSelf(const struct YgoState *s, unsigned o);
static uint8_t sSelf = NO_OBJ;
static int NotSelf(const struct YgoState *s, unsigned o) { return o != sSelf && F_SpellTrap(s, o); }
static int Can_HeavyStorm(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS];
    sSelf = c->obj;
    return Fx_Collect(S, 2, YL_SZONE, NotSelf, list) > 0;
}
static void Op_HeavyStorm(struct YgoCtx *c)
{
    sSelf = c->obj;
    Fx_DestroyAll(S, 2, YL_SZONE, NotSelf, YGO_REASON_EFFECT);
}
static void Op_Trunade(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS];
    unsigned n, i;
    sSelf = c->obj;
    n = Fx_Collect(S, 2, YL_SZONE, NotSelf, list);
    for (i = 0; i < n; i++)
        Ygo_ToHand(S, list[i], YGO_REASON_EFFECT);
}

static int Equipped(const struct YgoState *s, unsigned o) { return F_FaceUpMonster(s, o) && Fx_EquipCount(s, o) > 0; }
static int Can_EternalRest(struct YgoCtx *c) { return Ygo_Exists(S, 2, YGO_LOC_MZONE, Equipped); }
static void Op_EternalRest(struct YgoCtx *c) { Fx_DestroyAll(S, 2, YL_MZONE, Equipped, YGO_REASON_EFFECT); }

/* Fissure: the opponent's face-up monster with the lowest ATK (your pick if tied) */
static int Can_Fissure(struct YgoCtx *c) { return Ygo_Exists(S, OPP, YGO_LOC_MZONE, F_FaceUpMonster); }
static void Op_Fissure(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], low[YGO_ZONES], pick;
    unsigned n = Fx_Collect(S, OPP, YL_MZONE, F_FaceUpMonster, list), i, k = 0;
    int best = 1 << 30;
    for (i = 0; i < n; i++)
        if (YgoDuel_Atk(S, list[i]) < best)
            best = YgoDuel_Atk(S, list[i]);
    for (i = 0; i < n; i++)
        if (YgoDuel_Atk(S, list[i]) == best)
            low[k++] = list[i];
    if (Ygo_AskCards(S, ME, YGO_P_DESTROY, CARD, low, k, 1, 1, &pick))
        Ygo_Destroy(S, pick, YGO_REASON_EFFECT);
}

/* ---- targeting one card */
static int Can_OppMonster(struct YgoCtx *c) { return Fx_HasTarget(S, ME, OPP, YL_MZONE, F_Monster); }
static int Can_AnyMonster(struct YgoCtx *c) { return Fx_HasTarget(S, ME, 2, YL_MZONE, F_Monster); }
static int Can_AnyFaceUp(struct YgoCtx *c) { return Fx_HasTarget(S, ME, 2, YL_MZONE, F_FaceUpMonster); }
static void Tg_FaceUp(struct YgoCtx *c) { Fx_Target(c, 2, YL_MZONE, F_FaceUpMonster, 1, YGO_P_TARGET); }

static int Can_RemoveTrap(struct YgoCtx *c) { return Fx_HasTarget(S, ME, 2, YL_SZONE, F_FaceUpTrap); }
static void Tg_RemoveTrap(struct YgoCtx *c) { Fx_Target(c, 2, YL_SZONE, F_FaceUpTrap, 1, YGO_P_DESTROY); }
static void Op_DestroyTarget(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, NULL) && Ygo_OnField(S, TARGET(0)))
        Ygo_Destroy(S, TARGET(0), YGO_REASON_EFFECT);
}

static int SpellOrSet(const struct YgoState *s, unsigned o)
{
    return F_SpellTrap(s, o) && (!s->objs[o].faceUp || F_Spell(s, o));
}
static int Can_DeSpell(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Targets(S, ME, 2, YL_SZONE, SpellOrSet, list), i;
    for (i = 0; i < n; i++)
        if (list[i] != c->obj)
            return 1;
    return 0;
}
static void Tg_DeSpell(struct YgoCtx *c) { Fx_Target(c, 2, YL_SZONE, SpellOrSet, 1, YGO_P_DESTROY); }
static void Op_DeSpell(struct YgoCtx *c)
{
    unsigned t = TARGET(0);
    if (!Fx_TargetOk(c, 0, F_SpellTrap))
        return;
    if (!S->objs[t].faceUp)
        Ygo_Reveal(S, t);
    if (F_Spell(S, t))
        Ygo_Destroy(S, t, YGO_REASON_EFFECT);
}

static int Can_SpellTrapNotSelf(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Targets(S, ME, 2, YL_SZONE, F_SpellTrap, list), i;
    for (i = 0; i < n; i++)
        if (list[i] != c->obj)
            return 1;
    return 0;
}
static void Tg_SpellTrap(struct YgoCtx *c) { Fx_Target(c, 2, YL_SZONE, F_SpellTrap, 1, YGO_P_DESTROY); }

static int Can_StopDefense(struct YgoCtx *c) { return Fx_HasTarget(S, ME, OPP, YL_MZONE, F_DefensePos); }
static void Tg_StopDefense(struct YgoCtx *c) { Fx_Target(c, OPP, YL_MZONE, F_DefensePos, 1, YGO_P_TARGET); }
static void Op_StopDefense(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_DefensePos))
        Ygo_ChangePosition(S, TARGET(0), 0);
}

static int Can_BlockAttack(struct YgoCtx *c) { return Fx_HasTarget(S, ME, OPP, YL_MZONE, F_AttackPos); }
static void Tg_BlockAttack(struct YgoCtx *c) { Fx_Target(c, OPP, YL_MZONE, F_AttackPos, 1, YGO_P_TARGET); }
static void Op_BlockAttack(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_AttackPos))
        Ygo_ChangePosition(S, TARGET(0), 1);
}

static void Tg_OppMonster(struct YgoCtx *c) { Fx_Target(c, OPP, YL_MZONE, F_Monster, 1, YGO_P_TARGET); }
static void Op_ChangeOfHeart(struct YgoCtx *c)
{
    unsigned t = TARGET(0);
    if (!Fx_TargetOk(c, 0, F_Monster) || S->objs[t].controller == ME || !Ygo_FreeZones(S, ME, 0))
        return;
    Ygo_TakeControl(S, t, ME);
    Ygo_AddLasting(S, YL_RETURN_CONTROL, YGO_SCOPE_OBJ, ME, t, 0, 0);
}

static void Op_RushRecklessly(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_FaceUpMonster))
        Ygo_AddStat(S, TARGET(0), 700, 0, 1);
}
static void Op_ReliableGuardian(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_FaceUpMonster))
        Ygo_AddStat(S, TARGET(0), 0, 700, 1);
}

/* ---- Graveyards */
static int OppGraveMonster(const struct YgoState *s, unsigned o) { return F_MonsterCard(s, o); }
static int Can_Gravedigger(struct YgoCtx *c) { return Fx_HasTarget(S, ME, OPP, YL_GRAVE, OppGraveMonster); }
static void Tg_Gravedigger(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS], picks[2];
    unsigned n = Fx_Targets(S, ME, OPP, YL_GRAVE, OppGraveMonster, list), k, i;
    k = Ygo_AskCards(S, ME, YGO_P_BANISH, CARD, list, n, 1, 2, picks);
    for (i = 0; i < k; i++)
    {
        c->link->targetMoves[c->link->targetCount] = S->objs[picks[i]].moves;
        c->link->targets[c->link->targetCount++] = picks[i];
    }
}
static int InGrave(const struct YgoState *s, unsigned o) { return s->objs[o].location == YGO_LOC_GRAVE; }
static void Op_BanishTargets(struct YgoCtx *c)
{
    unsigned i;
    for (i = 0; i < c->link->targetCount; i++)
        if (Fx_TargetOk(c, i, InGrave))
            Ygo_Banish(S, TARGET(i), YGO_REASON_EFFECT);
}
static int Can_SoulRelease(struct YgoCtx *c) { return Fx_HasTarget(S, ME, 2, YL_GRAVE, NULL); }
static void Tg_SoulRelease(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS], picks[5];
    unsigned n = Fx_Targets(S, ME, 2, YL_GRAVE, NULL, list), k, i;
    k = Ygo_AskCards(S, ME, YGO_P_BANISH, CARD, list, n, 1, 5, picks);
    for (i = 0; i < k; i++)
    {
        c->link->targetMoves[c->link->targetCount] = S->objs[picks[i]].moves;
        c->link->targets[c->link->targetCount++] = picks[i];
    }
}

/* Monster Reborn: either Graveyard */
static int Revivable(const struct YgoState *s, unsigned o)
{
    return F_MonsterCard(s, o) && (!Ygo_NeedsProper(s->objs[o].card) || s->objs[o].properly);
}
static int Can_Reborn(struct YgoCtx *c)
{
    return Ygo_FreeZones(S, ME, 0) && !Ygo_Rule(S, YR_NO_SUMMON, ME, 0) &&
           Fx_HasTarget(S, ME, 2, YL_GRAVE, Revivable);
}
static void Tg_Reborn(struct YgoCtx *c) { Fx_Target(c, 2, YL_GRAVE, Revivable, 1, YGO_P_SUMMON); }
static void Op_Reborn(struct YgoCtx *c)
{
    unsigned t = TARGET(0), pos;
    if (!Fx_TargetOk(c, 0, InGrave))
        return;
    pos = Ygo_AskPosition(S, ME, S->objs[t].card, 3);
    Ygo_SpecialSummon(S, t, ME, pos, YGO_SUMMON_SPECIAL);
}

/* ---- drawing and hands */
static int Can_PotOfGreed(struct YgoCtx *c) { return S->players[ME].deckCount >= 2 && !Ygo_Rule(S, YR_NO_DRAW, ME, 0); }
static void Op_PotOfGreed(struct YgoCtx *c) { Ygo_Draw(S, ME, 2); }
static int Can_Charity(struct YgoCtx *c) { return S->players[ME].deckCount >= 3 && !Ygo_Rule(S, YR_NO_DRAW, ME, 0); }
static void Op_Charity(struct YgoCtx *c)
{
    Ygo_Draw(S, ME, 3);
    Fx_Discard(S, ME, 2, 0, YGO_REASON_EFFECT);
}
static int Can_CardDestruction(struct YgoCtx *c)
{
    return S->players[ME].handCount > (S->objs[c->obj].location == YGO_LOC_HAND ? 1u : 0u) ||
           S->players[OPP].handCount > 0;
}
static void Op_CardDestruction(struct YgoCtx *c)
{
    unsigned p, n[2], i;
    for (p = 0; p < 2; p++)
    {
        uint8_t hand[YGO_HAND_MAX];
        unsigned k = Ygo_List(S, ME ^ p, YGO_LOC_HAND, hand);
        n[ME ^ p] = k;
        for (i = 0; i < k; i++)
            Ygo_Discard(S, hand[i], YGO_REASON_EFFECT);
    }
    Ygo_Draw(S, ME, n[ME]);
    Ygo_Draw(S, OPP, n[OPP]);
}
static int Can_OppHand(struct YgoCtx *c) { return S->players[OPP].handCount > 0; }
static void Op_Spy(struct YgoCtx *c)
{
    unsigned n = S->players[OPP].handCount;
    if (n)
        Ygo_Look(S, ME, S->players[OPP].hand[Ygo_Rand(S, n)]);
}
static int Can_Telescope(struct YgoCtx *c) { return S->players[OPP].deckCount > 0; }
static void Op_Telescope(struct YgoCtx *c)
{
    unsigned n = S->players[OPP].deckCount, i;
    for (i = 0; i < 5 && i < n; i++)
        Ygo_Look(S, ME, S->players[OPP].deck[n - 1 - i]);
}
static int MonsterInHand(const struct YgoState *s, unsigned o) { return F_MonsterCard(s, o); }
static int Can_Coffin(struct YgoCtx *c) { return Ygo_Exists(S, ME, YGO_LOC_HAND, MonsterInHand); }
static void Op_Coffin(struct YgoCtx *c)
{
    uint8_t list[YGO_HAND_MAX], picks[3];
    unsigned n = Fx_Collect(S, ME, YL_HAND, MonsterInHand, list), k, i;
    k = Ygo_AskCards(S, ME, YGO_P_DISCARD, CARD, list, n, 1, 3, picks);
    for (i = 0; i < k; i++)
        Ygo_Discard(S, picks[i], YGO_REASON_EFFECT);
}

/* Tribute to the Doomed: discard 1, then target 1 monster; destroy it */
static int Can_TributeDoomed(struct YgoCtx *c)
{
    return S->players[ME].handCount > (S->objs[c->obj].location == YGO_LOC_HAND ? 1u : 0u) && Can_AnyMonster(c);
}
static void Cost_DiscardOne(struct YgoCtx *c) { Fx_Discard(S, ME, 1, 0, YGO_REASON_COST); }
static void Tg_AnyMonster(struct YgoCtx *c) { Fx_Target(c, 2, YL_MZONE, F_Monster, 1, YGO_P_DESTROY); }

/* ---- summoning */
static int Can_Polymerization(struct YgoCtx *c) { return Fx_CanFusion(S, ME, c->obj); }
static void Op_Polymerization(struct YgoCtx *c) { Ygo_FusionSummon(c); }

static int Can_BlackIllusion(struct YgoCtx *c) { return Fx_CanRitual(S, ME, YGO_CARD_RELINQUISHED, c->obj); }
static void Op_BlackIllusion(struct YgoCtx *c) { Ygo_RitualSummon(c, YGO_CARD_RELINQUISHED); }

static int Low1500Deck(const struct YgoState *s, unsigned o)
{
    return F_MonsterCard(s, o) && YgoDuel_Atk(s, o) <= 1500 && Ygo_Card(s->objs[o].card)->frame <= YGO_EFFECT;
}
static int Can_LastWill(struct YgoCtx *c)
{
    return S->players[ME].monsterToGraveTurn == S->turn && S->turn && Ygo_FreeZones(S, ME, 0) &&
           Ygo_Exists(S, ME, YGO_LOC_DECK, Low1500Deck);
}
static void Op_LastWill(struct YgoCtx *c)
{
    uint8_t list[YGO_DECK_MAX], pick;
    unsigned n = Fx_Collect(S, ME, YL_DECK, Low1500Deck, list), pos;
    if (!n || !Ygo_AskCards(S, ME, YGO_P_SUMMON, CARD, list, n, 1, 1, &pick))
        return;
    pos = Ygo_AskPosition(S, ME, S->objs[pick].card, 3);
    Ygo_SpecialSummon(S, pick, ME, pos, YGO_SUMMON_SPECIAL);
    Ygo_ShuffleDeck(S, ME);
}

static int Dragon(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    return c && c->race == YGO_DRAGON && c->frame <= YGO_RITUAL;
}
static int Can_Flute(struct YgoCtx *c)
{
    return Fx_OnFieldFaceUp(S, 2, YGO_CARD_LORD_OF_D) && Ygo_FreeZones(S, ME, 0) &&
           Ygo_Exists(S, ME, YGO_LOC_HAND, Dragon) && !Ygo_Rule(S, YR_NO_SUMMON, ME, 0);
}
static int SummonableDragon(const struct YgoState *s, unsigned o)
{
    return Dragon(s, o) && Ygo_CanSpecialSummon(s, o, s->objs[o].owner);
}
static void Op_Flute(struct YgoCtx *c)
{
    uint8_t list[YGO_HAND_MAX], picks[2];
    unsigned n, k, i, max;
    if (!Fx_OnFieldFaceUp(S, 2, YGO_CARD_LORD_OF_D))
        return;
    n = Fx_Collect(S, ME, YL_HAND, SummonableDragon, list);
    max = Ygo_FreeZones(S, ME, 0) < 2 ? Ygo_FreeZones(S, ME, 0) : 2;
    k = Ygo_AskCards(S, ME, YGO_P_SUMMON, CARD, list, n, 1, max, picks);
    for (i = 0; i < k; i++)
        Ygo_SpecialSummon(S, picks[i], ME, Ygo_AskPosition(S, ME, S->objs[picks[i]].card, 3), YGO_SUMMON_SPECIAL);
}

static int Harpie(const struct YgoState *s, unsigned o)
{
    return s->objs[o].card == YGO_CARD_HARPIE_LADY || s->objs[o].card == YGO_CARD_HARPIE_LADY_SISTERS;
}
static int HarpieFaceUp(const struct YgoState *s, unsigned o)
{
    return F_FaceUpMonster(s, o) && s->objs[o].card == YGO_CARD_HARPIE_LADY;
}
static int Can_Egotist(struct YgoCtx *c)
{
    return Ygo_Exists(S, 2, YGO_LOC_MZONE, HarpieFaceUp) && Ygo_FreeZones(S, ME, 0) &&
           !Ygo_Rule(S, YR_NO_SUMMON, ME, 0) &&
           (Ygo_Exists(S, ME, YGO_LOC_HAND, Harpie) || Ygo_Exists(S, ME, YGO_LOC_DECK, Harpie));
}
static void Op_Egotist(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS], pick;
    unsigned n = Fx_Collect(S, ME, YL_HAND | YL_DECK, Harpie, list), fromDeck;
    if (!Ygo_Exists(S, 2, YGO_LOC_MZONE, HarpieFaceUp) || !n)
        return;
    Ygo_AskCards(S, ME, YGO_P_SUMMON, CARD, list, n, 1, 1, &pick);
    fromDeck = S->objs[pick].location == YGO_LOC_DECK;
    if (Ygo_SpecialSummon(S, pick, ME, Ygo_AskPosition(S, ME, S->objs[pick].card, 3), YGO_SUMMON_SPECIAL))
        S->objs[pick].properly = 1;
    if (fromDeck)
        Ygo_ShuffleDeck(S, ME);
}

/* ---- Soul Exchange: Tribute that opponent's monster this turn; no Battle Phase */
static int Can_SoulExchange(struct YgoCtx *c) { return S->phase == YGO_PHASE_MAIN1 && Can_OppMonster(c); }
static void Op_SoulExchange(struct YgoCtx *c)
{
    Ygo_AddLasting(S, YR_NO_BATTLE_PHASE, YGO_SCOPE_PLAYER, ME, NO_OBJ, 0, 0);
    if (Fx_TargetOk(c, 0, F_Monster))
        Ygo_AddLasting(S, YR_EXTRA_TRIBUTE, YGO_SCOPE_OBJ, ME, TARGET(0), 0, 0);
}

/* ---- Shield & Sword: original ATK and DEF switched until the end of this turn (the monsters there now) */
static int Can_ShieldSword(struct YgoCtx *c) { return Ygo_Exists(S, 2, YGO_LOC_MZONE, F_FaceUpMonster); }
static void Op_ShieldSword(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES * 2];
    unsigned n = Fx_Collect(S, 2, YL_MZONE, F_FaceUpMonster, list), i;
    for (i = 0; i < n; i++)
        Ygo_AddLasting(S, YR_SWAP_STATS, YGO_SCOPE_OBJ, ME, list[i], 0, 0);
}

/* ---- Share the Pain: Tribute 1; the opponent Tributes 1 */
static int Can_SharePain(struct YgoCtx *c) { return Ygo_MonsterCount(S, ME) && Ygo_MonsterCount(S, OPP); }
static void Cost_TributeOne(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], pick;
    unsigned n = Ygo_List(S, ME, YGO_LOC_MZONE, list);
    if (Ygo_AskCards(S, ME, YGO_P_TRIBUTE, CARD, list, n, 1, 1, &pick))
    {
        c->link->value = YgoDuel_Atk(S, pick);
        Ygo_SendToGrave(S, pick, YGO_REASON_COST | YGO_REASON_TRIBUTE);
    }
}
static void Op_SharePain(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], pick;
    unsigned n = Ygo_List(S, OPP, YGO_LOC_MZONE, list);
    if (Ygo_AskCards(S, OPP, YGO_P_TRIBUTE, CARD, list, n, 1, 1, &pick))
        Ygo_SendToGrave(S, pick, YGO_REASON_EFFECT | YGO_REASON_TRIBUTE);
}

/* ---- Swords of Revealing Light: stays for 3 of the opponent's turns */
static void Op_Swords(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES];
    unsigned n = Fx_Collect(S, OPP, YL_MZONE, F_FaceDownMonster, list), i;
    for (i = 0; i < n; i++)
        Ygo_ChangePosition(S, list[i], 1);
    S->objs[c->obj].counters = 0;
}
static int Rule_Swords(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_NO_ATTACKS && a == (s->objs[src].controller ^ 1u);
}
static int Can_SwordsCount(struct YgoCtx *c) { return S->current != ME && Ygo_OnField(S, c->obj) && S->objs[c->obj].faceUp; }
static void Op_SwordsCount(struct YgoCtx *c)
{
    if (!Ygo_OnField(S, c->obj))
        return;
    if (++S->objs[c->obj].counters >= 3)
        Ygo_Destroy(S, c->obj, YGO_REASON_RULE);
}

/* ---- Scapegoat: 4 Sheep Tokens; no other Summons this turn */
static int Can_Scapegoat(struct YgoCtx *c)
{
    return Ygo_FreeZones(S, ME, 0) >= 4 && S->players[ME].summonedTurn != S->turn &&
           !Ygo_Rule(S, YR_NO_SUMMON, ME, 0);
}
static void Op_Scapegoat(struct YgoCtx *c)
{
    unsigned i;
    if (Ygo_FreeZones(S, ME, 0) < 4)
        return;
    for (i = 0; i < 4; i++)
        Ygo_MakeToken(S, YGO_CARD_SHEEP_TOKEN, ME, 1);
    Ygo_AddLasting(S, YR_NO_SUMMON, YGO_SCOPE_PLAYER, ME, NO_OBJ, 0, 0);
}

/* ---- Toon World: activated by paying 1000 LP */
static int Can_ToonWorld(struct YgoCtx *c) { return Fx_CanPay(S, ME, 1000); }
static void Cost_ToonWorld(struct YgoCtx *c) { Fx_Pay(S, ME, 1000); }

/* ---- Equip Spells with more to them */
/* Sword of Deep-Seated: sent to the GY -> on top of the Deck */
static void Op_DeepSeated(struct YgoCtx *c)
{
    if (S->objs[c->obj].location == YGO_LOC_GRAVE)
        Ygo_ToDeck(S, c->obj, 1);
}
/* Malevolent Nuzzler: sent from the field to the GY -> you can pay 500 LP; top of the Deck */
static int Can_Nuzzler(struct YgoCtx *c) { return Fx_CanPay(S, ME, 500); }
static void Op_Nuzzler(struct YgoCtx *c)
{
    if (S->objs[c->obj].location != YGO_LOC_GRAVE || !Fx_CanPay(S, ME, 500))
        return;
    Fx_Pay(S, ME, 500);
    Ygo_ToDeck(S, c->obj, 1);
}
/* Black Pendant: sent from the field to the GY -> 500 damage */
static void Op_BlackPendant(struct YgoCtx *c) { Ygo_Damage(S, OPP, 500, 0); }
/* Stim-Pack: -200 at each of your Standby Phases (counted on the card) */
static int Stat_StimPack(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (s->objs[src].equipTo != target + 1 || which != YS_ATK)
        return value;
    return value + 700 - 200 * s->objs[src].counters;
}
static int Can_Equipped(struct YgoCtx *c) { return Fx_EquippedTo(S, c->obj) != NO_OBJ; }
static void Op_Count(struct YgoCtx *c)
{
    if (Fx_EquippedTo(S, c->obj) != NO_OBJ)
        S->objs[c->obj].counters++;
}
/* Germ Infection: -300 at each of the equipped monster's controller's Standby Phases */
static int Stat_Germ(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (s->objs[src].equipTo != target + 1 || which != YS_ATK)
        return value;
    return value - 300 * s->objs[src].counters;
}
static int Can_GermCount(struct YgoCtx *c)
{
    unsigned m = Fx_EquippedTo(S, c->obj);
    return m != NO_OBJ && S->objs[m].controller == S->current;
}
/* Paralyzing Potion: it can't attack */
static int Rule_Paralyzing(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_CANNOT_ATTACK && s->objs[src].equipTo == a + 1;
}
/* Ring of Magnetism: the opponent must attack it, if they attack */
static int Rule_Magnetism(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_ATTACK_ONLY && s->objs[src].equipTo == a + 1;
}

const struct YgoEffect gYgoFxS1Spells[] = {
    /* burn / heal */
    ACT(YGO_CARD_RED_MEDICINE, 0, NULL, NULL, NULL, Op_Red),
    ACT(YGO_CARD_GOBLIN_S_SECRET_REMEDY, 0, NULL, NULL, NULL, Op_Remedy),
    ACT(YGO_CARD_DIAN_KETO_THE_CURE_MASTER, 0, NULL, NULL, NULL, Op_DianKeto),
    ACT(YGO_CARD_SPARKS, 0, NULL, NULL, NULL, Op_Sparks),
    ACT(YGO_CARD_HINOTAMA, 0, NULL, NULL, NULL, Op_Hinotama),
    ACT(YGO_CARD_FINAL_FLAME, 0, NULL, NULL, NULL, Op_FinalFlame),
    ACT(YGO_CARD_OOKAZI, 0, NULL, NULL, NULL, Op_Ookazi),
    ACT(YGO_CARD_TREMENDOUS_FIRE, 0, NULL, NULL, NULL, Op_TremendousFire),
    /* destruction */
    ACT(YGO_CARD_DARK_HOLE, 0, Can_DarkHole, NULL, NULL, Op_DarkHole),
    ACT(YGO_CARD_RAIGEKI, 0, Can_Raigeki, NULL, NULL, Op_Raigeki),
    ACT(YGO_CARD_HEAVY_STORM, 0, Can_HeavyStorm, NULL, NULL, Op_HeavyStorm),
    ACT(YGO_CARD_GIANT_TRUNADE, 0, Can_HeavyStorm, NULL, NULL, Op_Trunade),
    ACT(YGO_CARD_ETERNAL_REST, 0, Can_EternalRest, NULL, NULL, Op_EternalRest),
    ACT(YGO_CARD_FISSURE, 0, Can_Fissure, NULL, NULL, Op_Fissure),
    ACT(YGO_CARD_REMOVE_TRAP, YF_TARGETS, Can_RemoveTrap, NULL, Tg_RemoveTrap, Op_DestroyTarget),
    ACT(YGO_CARD_DE_SPELL, YF_TARGETS, Can_DeSpell, NULL, Tg_DeSpell, Op_DeSpell),
    ACT(YGO_CARD_MYSTICAL_SPACE_TYPHOON, YF_TARGETS, Can_SpellTrapNotSelf, NULL, Tg_SpellTrap, Op_DestroyTarget),
    ACT(YGO_CARD_TRIBUTE_TO_THE_DOOMED, YF_TARGETS, Can_TributeDoomed, Cost_DiscardOne, Tg_AnyMonster,
        Op_DestroyTarget),
    /* positions, control, stats */
    ACT(YGO_CARD_STOP_DEFENSE, YF_TARGETS, Can_StopDefense, NULL, Tg_StopDefense, Op_StopDefense),
    ACT(YGO_CARD_BLOCK_ATTACK, YF_TARGETS, Can_BlockAttack, NULL, Tg_BlockAttack, Op_BlockAttack),
    ACT(YGO_CARD_CHANGE_OF_HEART, YF_TARGETS, Can_OppMonster, NULL, Tg_OppMonster, Op_ChangeOfHeart),
    ACT(YGO_CARD_RUSH_RECKLESSLY, YF_TARGETS | YF_DAMAGE_STEP, Can_AnyFaceUp, NULL, Tg_FaceUp, Op_RushRecklessly),
    ACT(YGO_CARD_THE_RELIABLE_GUARDIAN, YF_TARGETS | YF_DAMAGE_STEP, Can_AnyFaceUp, NULL, Tg_FaceUp,
        Op_ReliableGuardian),
    ACT(YGO_CARD_SHIELD_SWORD, 0, Can_ShieldSword, NULL, NULL, Op_ShieldSword),
    ACT(YGO_CARD_SOUL_EXCHANGE, YF_TARGETS, Can_SoulExchange, NULL, Tg_OppMonster, Op_SoulExchange),
    /* Graveyards */
    ACT(YGO_CARD_GRAVEDIGGER_GHOUL, YF_TARGETS, Can_Gravedigger, NULL, Tg_Gravedigger, Op_BanishTargets),
    ACT(YGO_CARD_SOUL_RELEASE, YF_TARGETS, Can_SoulRelease, NULL, Tg_SoulRelease, Op_BanishTargets),
    ACT(YGO_CARD_MONSTER_REBORN, YF_TARGETS, Can_Reborn, NULL, Tg_Reborn, Op_Reborn),
    /* cards */
    ACT(YGO_CARD_POT_OF_GREED, 0, Can_PotOfGreed, NULL, NULL, Op_PotOfGreed),
    ACT(YGO_CARD_GRACEFUL_CHARITY, 0, Can_Charity, NULL, NULL, Op_Charity),
    ACT(YGO_CARD_CARD_DESTRUCTION, 0, Can_CardDestruction, NULL, NULL, Op_CardDestruction),
    ACT(YGO_CARD_THE_INEXPERIENCED_SPY, 0, Can_OppHand, NULL, NULL, Op_Spy),
    ACT(YGO_CARD_ANCIENT_TELESCOPE, 0, Can_Telescope, NULL, NULL, Op_Telescope),
    ACT(YGO_CARD_THE_CHEERFUL_COFFIN, 0, Can_Coffin, NULL, NULL, Op_Coffin),
    ACT(YGO_CARD_SHARE_THE_PAIN, 0, Can_SharePain, Cost_TributeOne, NULL, Op_SharePain),
    /* summoning */
    ACT(YGO_CARD_POLYMERIZATION, 0, Can_Polymerization, NULL, NULL, Op_Polymerization),
    ACT(YGO_CARD_BLACK_ILLUSION_RITUAL, 0, Can_BlackIllusion, NULL, NULL, Op_BlackIllusion),
    ACT(YGO_CARD_LAST_WILL, 0, Can_LastWill, NULL, NULL, Op_LastWill),
    ACT(YGO_CARD_THE_FLUTE_OF_SUMMONING_DRAGON, 0, Can_Flute, NULL, NULL, Op_Flute),
    ACT(YGO_CARD_ELEGANT_EGOTIST, 0, Can_Egotist, NULL, NULL, Op_Egotist),
    ACT(YGO_CARD_SCAPEGOAT, 0, Can_Scapegoat, NULL, NULL, Op_Scapegoat),
    /* staying on the field */
    ACT_HOOKS(YGO_CARD_SWORDS_OF_REVEALING_LIGHT, YF_STAYS, NULL, NULL, NULL, Op_Swords, NULL, Rule_Swords),
    TRIG(YGO_CARD_SWORDS_OF_REVEALING_LIGHT, 0, YEV_END_ANY, Can_SwordsCount, NULL, Op_SwordsCount),
    ACT_HOOKS(YGO_CARD_TOON_WORLD, 0, Can_ToonWorld, Cost_ToonWorld, NULL, NULL, NULL, NULL),
    /* Equip Spells */
    EQUIP(YGO_CARD_LEGENDARY_SWORD),
    EQUIP(YGO_CARD_BEAST_FANGS),
    EQUIP(YGO_CARD_VIOLET_CRYSTAL),
    EQUIP(YGO_CARD_BOOK_OF_SECRET_ARTS),
    EQUIP(YGO_CARD_POWER_OF_KAISHIN),
    EQUIP(YGO_CARD_DARK_ENERGY),
    EQUIP(YGO_CARD_LASER_CANNON_ARMOR),
    EQUIP(YGO_CARD_VILE_GERMS),
    EQUIP(YGO_CARD_SILVER_BOW_AND_ARROW),
    EQUIP(YGO_CARD_DRAGON_TREASURE),
    EQUIP(YGO_CARD_ELECTRO_WHIP),
    EQUIP(YGO_CARD_MYSTICAL_MOON),
    EQUIP(YGO_CARD_MACHINE_CONVERSION_FACTORY),
    EQUIP(YGO_CARD_RAISE_BODY_HEAT),
    EQUIP(YGO_CARD_FOLLOW_WIND),
    EQUIP(YGO_CARD_SWORD_OF_DARK_DESTRUCTION),
    EQUIP(YGO_CARD_INVIGORATION),
    EQUIP(YGO_CARD_SWORD_OF_DEEP_SEATED),
    TRIG(YGO_CARD_SWORD_OF_DEEP_SEATED, 0, YEV_TO_GRAVE, NULL, NULL, Op_DeepSeated),
    EQUIP(YGO_CARD_MALEVOLENT_NUZZLER),
    TRIG(YGO_CARD_MALEVOLENT_NUZZLER, YF_OPTIONAL | YF_WHEN, YEV_FROM_FIELD_TO_GRAVE, Can_Nuzzler, NULL, Op_Nuzzler),
    EQUIP(YGO_CARD_BLACK_PENDANT),
    TRIG(YGO_CARD_BLACK_PENDANT, 0, YEV_FROM_FIELD_TO_GRAVE, NULL, NULL, Op_BlackPendant),
    ACT_HOOKS(YGO_CARD_STIM_PACK, YF_TARGETS, Equip_Can, NULL, Equip_Target, Equip_Op, Stat_StimPack, NULL),
    TRIG(YGO_CARD_STIM_PACK, 0, YEV_STANDBY_OWN, Can_Equipped, NULL, Op_Count),
    ACT_HOOKS(YGO_CARD_GERM_INFECTION, YF_TARGETS, Equip_Can, NULL, Equip_Target, Equip_Op, Stat_Germ, NULL),
    TRIG(YGO_CARD_GERM_INFECTION, 0, YEV_STANDBY_ANY, Can_GermCount, NULL, Op_Count),
    ACT_HOOKS(YGO_CARD_PARALYZING_POTION, YF_TARGETS, Equip_Can, NULL, Equip_Target, Equip_Op, NULL,
              Rule_Paralyzing),
    ACT_HOOKS(YGO_CARD_RING_OF_MAGNETISM, YF_TARGETS, Equip_Can, NULL, Equip_Target, Equip_Op, Equip_Stat,
              Rule_Magnetism),
    /* Field Spells */
    FIELD(YGO_CARD_FOREST),
    FIELD(YGO_CARD_WASTELAND),
    FIELD(YGO_CARD_MOUNTAIN),
    FIELD(YGO_CARD_SOGEN),
    FIELD(YGO_CARD_UMI),
    FIELD(YGO_CARD_YAMI),
};
const unsigned gYgoFxS1SpellsCount = sizeof(gYgoFxS1Spells) / sizeof(gYgoFxS1Spells[0]);
