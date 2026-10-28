/* Yu-Gi-Oh! effects, stage 1: Effect Monsters. */
#include "ygo_fx.h"

/* ---- small helpers */
static const struct YgoCard *CardOf(const struct YgoState *s, unsigned o) { return Ygo_ObjCard(s, o); }
static unsigned CountOnField(const struct YgoState *s, unsigned player, unsigned card)
{
    uint8_t list[YGO_ZONES];
    unsigned n = Ygo_List(s, player, YGO_LOC_MZONE, list), i, k = 0;
    for (i = 0; i < n; i++)
        k += s->objs[list[i]].faceUp && s->objs[list[i]].card == card;
    return k;
}
static int OnFieldSelf(struct YgoCtx *c) { return Ygo_FaceUpMonster(S, c->obj); }

/* ---- direct attackers */
static int Rule_Direct(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_DIRECT_ATTACK && a == src;
}
/* ---- passive marker: the effect is the game's own rule (Exodia) */
static int Rule_None(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b) { return 0; }

/* ---- FLIP: destroy / bounce */
static int Can_AnyMonster(struct YgoCtx *c) { return Fx_HasTarget(S, ME, 2, YL_MZONE, F_Monster); }
static void Tg_ManEater(struct YgoCtx *c)
{
    /* any monster, itself included */
    uint8_t list[YGO_OBJS], pick;
    unsigned n = Fx_Targets(S, ME, 2, YL_MZONE, F_Monster, list);
    if (Ygo_AskCards(S, ME, YGO_P_DESTROY, CARD, list, n, 1, 1, &pick))
    {
        c->link->targetMoves[0] = S->objs[pick].moves;
        c->link->targets[0] = pick;
        c->link->targetCount = 1;
    }
}
static void Op_DestroyTarget(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_Monster))
        Ygo_Destroy(S, TARGET(0), YGO_REASON_EFFECT);
}
static void Tg_Bounce(struct YgoCtx *c) { Fx_Target(c, 2, YL_MZONE, F_Monster, 1, YGO_P_TARGET); }
static void Op_BounceTargets(struct YgoCtx *c)
{
    unsigned i;
    for (i = 0; i < c->link->targetCount; i++)
        if (Fx_TargetOk(c, i, F_Monster))
            Ygo_ToHand(S, TARGET(i), YGO_REASON_EFFECT);
}
/* Penguin Soldier: up to 2 */
static void Tg_Penguin(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS], picks[2];
    unsigned n = Fx_Targets(S, ME, 2, YL_MZONE, F_Monster, list), k, i;
    k = Ygo_AskCards(S, ME, YGO_P_TARGET, CARD, list, n, 1, 2, picks);
    for (i = 0; i < k; i++)
    {
        c->link->targetMoves[i] = S->objs[picks[i]].moves;
        c->link->targets[i] = picks[i];
    }
    c->link->targetCount = k;
}

/* Reaper of the Cards / Trap Master (Traps), Armed Ninja (Spells): a Set card is revealed and destroyed if it fits */
static int CanSTTarget(struct YgoCtx *c, int traps)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Targets(S, ME, 2, YL_SZONE, F_SpellTrap, list), i;
    for (i = 0; i < n; i++)
        if (!S->objs[list[i]].faceUp || (traps ? F_Trap(S, list[i]) : F_Spell(S, list[i])))
            return 1;
    return 0;
}
static int FaceDownOrTrap(const struct YgoState *s, unsigned o) { return F_SpellTrap(s, o) && (!s->objs[o].faceUp || F_Trap(s, o)); }
static int FaceDownOrSpell(const struct YgoState *s, unsigned o) { return F_SpellTrap(s, o) && (!s->objs[o].faceUp || F_Spell(s, o)); }
static int Can_TrapKiller(struct YgoCtx *c) { return CanSTTarget(c, 1); }
static int Can_SpellKiller(struct YgoCtx *c) { return CanSTTarget(c, 0); }
static void Tg_TrapKiller(struct YgoCtx *c) { Fx_Target(c, 2, YL_SZONE, FaceDownOrTrap, 1, YGO_P_DESTROY); }
static void Tg_SpellKiller(struct YgoCtx *c) { Fx_Target(c, 2, YL_SZONE, FaceDownOrSpell, 1, YGO_P_DESTROY); }
static void Op_TrapKiller(struct YgoCtx *c)
{
    unsigned t = TARGET(0);
    if (!Fx_TargetOk(c, 0, F_SpellTrap))
        return;
    if (!S->objs[t].faceUp)
        Ygo_Reveal(S, t);
    if (F_Trap(S, t))
        Ygo_Destroy(S, t, YGO_REASON_EFFECT);
}
static void Op_SpellKiller(struct YgoCtx *c)
{
    unsigned t = TARGET(0);
    if (!Fx_TargetOk(c, 0, F_SpellTrap))
        return;
    if (!S->objs[t].faceUp)
        Ygo_Reveal(S, t);
    if (F_Spell(S, t))
        Ygo_Destroy(S, t, YGO_REASON_EFFECT);
}

/* ---- FLIP: Graveyard to hand */
static int GraveTrap(const struct YgoState *s, unsigned o) { return s->objs[o].location == YGO_LOC_GRAVE && F_Trap(s, o); }
static int GraveSpell(const struct YgoState *s, unsigned o) { return s->objs[o].location == YGO_LOC_GRAVE && F_Spell(s, o); }
static int Can_MaskOfDarkness(struct YgoCtx *c) { return Fx_HasTarget(S, ME, ME, YL_GRAVE, GraveTrap); }
static int Can_MagicianOfFaith(struct YgoCtx *c) { return Fx_HasTarget(S, ME, ME, YL_GRAVE, GraveSpell); }
static void Tg_GraveTrap(struct YgoCtx *c) { Fx_Target(c, ME, YL_GRAVE, GraveTrap, 1, YGO_P_ADD_TO_HAND); }
static void Tg_GraveSpell(struct YgoCtx *c) { Fx_Target(c, ME, YL_GRAVE, GraveSpell, 1, YGO_P_ADD_TO_HAND); }
static int InGrave(const struct YgoState *s, unsigned o) { return s->objs[o].location == YGO_LOC_GRAVE; }
static void Op_GraveToHand(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, InGrave))
        Ygo_ToHand(S, TARGET(0), YGO_REASON_EFFECT);
}

/* ---- The Stern Mystic: everyone sees the face-down cards */
static void Op_SternMystic(struct YgoCtx *c)
{
    unsigned o;
    for (o = 0; o < S->objCount; o++)
        if (Ygo_OnField(S, o) && !S->objs[o].faceUp)
            Ygo_Reveal(S, o);
}

/* ---- Big Eye: look at up to 5 from the top of your Deck, put them back in any order */
static void Op_BigEye(struct YgoCtx *c)
{
    struct YgoPlayer *p = &S->players[ME];
    uint8_t top[5], order[5];
    unsigned n = p->deckCount < 5 ? p->deckCount : 5, i, k;
    for (i = 0; i < n; i++)
    {
        top[i] = p->deck[p->deckCount - 1 - i];
        Ygo_Look(S, ME, top[i]);
    }
    /* the order: first pick goes on top */
    for (k = 0; k < n; k++)
    {
        uint8_t left[5], m = 0, pick;
        for (i = 0; i < n; i++)
        {
            unsigned j;
            int used = 0;
            for (j = 0; j < k; j++)
                used |= order[j] == top[i];
            if (!used)
                left[m++] = top[i];
        }
        if (m == 1)
            pick = left[0];
        else
            Ygo_AskCards(S, ME, YGO_P_ORDER, CARD, left, m, 1, 1, &pick);
        order[k] = pick;
    }
    for (k = 0; k < n; k++)
        p->deck[p->deckCount - 1 - k] = order[k];
}

/* ---- Hiro's Shadow Scout: the opponent draws 3; Spells among them are discarded */
static void Op_HiroScout(struct YgoCtx *c)
{
    struct YgoPlayer *p = &S->players[OPP];
    unsigned before = p->handCount, i;
    Ygo_Draw(S, OPP, 3);
    for (i = before; i < p->handCount;)
    {
        unsigned o = p->hand[i];
        Ygo_Reveal(S, o);
        if (F_Spell(S, o))
            Ygo_Discard(S, o, YGO_REASON_EFFECT);
        else
            i++;
    }
}

/* ---- Princess of Tsurugi: 500 for each of the opponent's Spells and Traps */
static void Op_Tsurugi(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES + 1];
    Ygo_Damage(S, OPP, 500 * Ygo_List(S, OPP, YGO_LOC_SZONE, list), 0);
}

/* ---- The Immortal of Thunder */
static void Op_Gain3000(struct YgoCtx *c) { Ygo_Recover(S, ME, 3000); }
static void Op_Lose5000(struct YgoCtx *c) { Ygo_Damage(S, ME, 5000, 0); }

/* ---- Castle of Dark Illusions: Zombies +200, +200 more at each of your next 4 Standby Phases */
static void Op_CastleFlip(struct YgoCtx *c)
{
    S->objs[c->obj].data |= 1;
    S->objs[c->obj].counters = 0;
}
static int Stat_Castle(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    const struct YgoCard *m = CardOf(s, target);
    if (!(s->objs[src].data & 1) || which == YS_LEVEL || !m || m->race != YGO_ZOMBIE || !s->objs[target].faceUp)
        return value;
    return value + 200 * (1 + s->objs[src].counters);
}
static int Can_CastleGrow(struct YgoCtx *c) { return (S->objs[c->obj].data & 1) && S->objs[c->obj].counters < 4; }
static void Op_CastleGrow(struct YgoCtx *c)
{
    if (Ygo_FaceUpMonster(S, c->obj))
        S->objs[c->obj].counters++;
}

/* ---- Pumpking: +100 while Castle of Dark Illusions is on the field, +100 at 4 of your Standby Phases */
static int Stat_Pumpking(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (src != target || which == YS_LEVEL || !Fx_OnFieldFaceUp(s, 2, YGO_CARD_CASTLE_OF_DARK_ILLUSIONS))
        return value;
    return value + 100 * (1 + s->objs[src].counters);
}
static int Can_PumpkingGrow(struct YgoCtx *c) { return S->objs[c->obj].counters < 3; }
static void Op_PumpkingGrow(struct YgoCtx *c)
{
    if (!Fx_OnFieldFaceUp(S, 2, YGO_CARD_CASTLE_OF_DARK_ILLUSIONS))
        S->objs[c->obj].counters = 0;
    else
        S->objs[c->obj].counters++;
}

/* ---- Wall of Illusion: the monster that attacked it goes back to the hand after damage calculation */
static int Can_WallOfIllusion(struct YgoCtx *c)
{
    return c->eventObj != NO_OBJ && S->attackTarget == c->obj + 1u && S->objs[c->eventObj].location == YGO_LOC_MZONE;
}
static void Op_WallOfIllusion(struct YgoCtx *c)
{
    if (c->eventObj != NO_OBJ && S->objs[c->eventObj].location == YGO_LOC_MZONE)
        Ygo_ToHand(S, c->eventObj, YGO_REASON_EFFECT);
}

/* ---- The Wicked Worm Beast: back to the hand during your End Phase */
static void Op_ToHandSelf(struct YgoCtx *c)
{
    if (Ygo_FaceUpMonster(S, c->obj))
        Ygo_ToHand(S, c->obj, YGO_REASON_EFFECT);
}

/* ---- Lord of D.: Dragons on the field can't be targeted by card effects */
static int Rule_LordOfD(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    const struct YgoCard *m = a < s->objCount ? CardOf(s, a) : NULL;
    return rule == YR_UNTARGETABLE && m && m->race == YGO_DRAGON && s->objs[a].location == YGO_LOC_MZONE;
}

/* ---- Mysterious Puppeteer: 500 LP each time a monster is Normal or Flip Summoned */
static void Op_Gain500(struct YgoCtx *c) { Ygo_Recover(S, ME, 500); }

/* ---- monsters that can't be Normal Summoned, and their own Summons */
static int Rule_NoNormal(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_CANNOT_NORMAL_SUMMON && a == s->objs[src].card;
}
/* Gate Guardian: Tribute Sanga of the Thunder, Kazejin and Suijin */
static int GateFind(const struct YgoState *s, unsigned player, unsigned card, uint8_t *out)
{
    uint8_t list[YGO_ZONES];
    unsigned n = Ygo_List(s, player, YGO_LOC_MZONE, list), i;
    for (i = 0; i < n; i++)
        if (s->objs[list[i]].card == card)
        {
            *out = list[i];
            return 1;
        }
    return 0;
}
static int Can_GateGuardian(struct YgoCtx *c)
{
    uint8_t x;
    return GateFind(S, ME, YGO_CARD_SANGA_OF_THE_THUNDER, &x) && GateFind(S, ME, YGO_CARD_KAZEJIN, &x) &&
           GateFind(S, ME, YGO_CARD_SUIJIN, &x);
}
static void Cost_GateGuardian(struct YgoCtx *c)
{
    static const uint16_t parts[3] = {YGO_CARD_SANGA_OF_THE_THUNDER, YGO_CARD_KAZEJIN, YGO_CARD_SUIJIN};
    unsigned i;
    for (i = 0; i < 3; i++)
    {
        uint8_t o;
        if (GateFind(S, ME, parts[i], &o))
            Ygo_SendToGrave(S, o, YGO_REASON_COST | YGO_REASON_TRIBUTE);
    }
}
/* Larvae Moth / Great Moth: Tribute a Petit Moth equipped with Cocoon of Evolution for 2 / 4 of your turns */
static int CocoonedMoth(const struct YgoState *s, unsigned player, unsigned turns, uint8_t *out)
{
    unsigned o;
    for (o = 0; o < s->objCount; o++)
        if (s->objs[o].card == YGO_CARD_COCOON_OF_EVOLUTION && Ygo_OnField(s, o) && s->objs[o].equipTo &&
            s->objs[o].counters >= turns)
        {
            unsigned m = s->objs[o].equipTo - 1;
            if (s->objs[m].card == YGO_CARD_PETIT_MOTH && s->objs[m].controller == player && Ygo_FaceUpMonster(s, m))
            {
                *out = m;
                return 1;
            }
        }
    return 0;
}
static int Can_LarvaeMoth(struct YgoCtx *c) { uint8_t x; return CocoonedMoth(S, ME, 2, &x); }
static int Can_GreatMoth(struct YgoCtx *c) { uint8_t x; return CocoonedMoth(S, ME, 4, &x); }
static void Cost_LarvaeMoth(struct YgoCtx *c)
{
    uint8_t m;
    if (CocoonedMoth(S, ME, 2, &m))
        Ygo_SendToGrave(S, m, YGO_REASON_COST | YGO_REASON_TRIBUTE);
}
static void Cost_GreatMoth(struct YgoCtx *c)
{
    uint8_t m;
    if (CocoonedMoth(S, ME, 4, &m))
        Ygo_SendToGrave(S, m, YGO_REASON_COST | YGO_REASON_TRIBUTE);
}

/* ---- Cocoon of Evolution: equips itself from the hand to your Petit Moth; that Petit Moth's original ATK/DEF
 * become Cocoon's; counts your turns */
static int PetitMoth(const struct YgoState *s, unsigned o)
{
    return F_FaceUpMonster(s, o) && s->objs[o].card == YGO_CARD_PETIT_MOTH;
}
static int Can_Cocoon(struct YgoCtx *c)
{
    return Ygo_FreeZones(S, ME, 1) && Fx_HasTarget(S, ME, ME, YL_MZONE, PetitMoth);
}
static void Tg_Cocoon(struct YgoCtx *c) { Fx_Target(c, ME, YL_MZONE, PetitMoth, 1, YGO_P_EQUIP); }
void Fx_PlaceAsEquip(struct YgoState *s, unsigned o, unsigned player, unsigned monster);
static void Op_Cocoon(struct YgoCtx *c)
{
    if (!Fx_TargetOk(c, 0, PetitMoth) || S->objs[c->obj].location != YGO_LOC_HAND)
        return;
    Fx_PlaceAsEquip(S, c->obj, ME, TARGET(0));
    S->objs[c->obj].counters = 0;
}
static int Stat_Cocoon(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    const struct YgoCard *moth = CardOf(s, target), *cocoon = CardOf(s, src);
    if (s->objs[src].location != YGO_LOC_SZONE || s->objs[src].equipTo != target + 1 || which == YS_LEVEL)
        return value;
    return which == YS_ATK ? value - moth->atk + cocoon->atk : value - moth->def + cocoon->def;
}
static int Can_CocoonCount(struct YgoCtx *c)
{
    return S->objs[c->obj].location == YGO_LOC_SZONE && S->objs[c->obj].equipTo;
}
static void Op_CocoonCount(struct YgoCtx *c)
{
    if (Fx_EquippedTo(S, c->obj) != NO_OBJ)
        S->objs[c->obj].counters++;
}

/* ---- battle damage */
static void Op_OppDiscardRandom(struct YgoCtx *c) { Fx_Discard(S, OPP, 1, 1, YGO_REASON_EFFECT); }
static int Can_OppHasHand(struct YgoCtx *c) { return S->players[OPP].handCount > 0; }
static void Op_Draw1(struct YgoCtx *c) { Ygo_Draw(S, ME, 1); }
static void Op_OppDraw2(struct YgoCtx *c) { Ygo_Draw(S, OPP, 2); }

/* ---- Sanga of the Thunder, Kazejin, Suijin: the attacker's ATK is 0 during damage calculation (once while face up) */
static int Can_Sanga(struct YgoCtx *c)
{
    return S->damageStep && S->current != ME && S->attackTarget == c->obj + 1u && S->attacker &&
           Ygo_CanTarget(S, S->attacker - 1, ME);
}
static void Tg_Attacker(struct YgoCtx *c)
{
    unsigned a = S->attacker - 1;
    c->link->targetMoves[0] = S->objs[a].moves;
    c->link->targets[0] = a;
    c->link->targetCount = 1;
}
static void Op_Sanga(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_FaceUpMonster))
        Ygo_AddLasting(S, YR_ATK_ZERO, YGO_SCOPE_OBJ, ME, TARGET(0), YGO_UNTIL_BATTLE, 0);
}

/* ---- Kuriboh: discard it; no battle damage from that battle */
static int Can_Kuriboh(struct YgoCtx *c)
{
    return S->damageStep && S->current != ME && S->attacker && S->objs[S->attacker - 1].controller == OPP;
}
static void Cost_DiscardSelf(struct YgoCtx *c) { Ygo_Discard(S, c->obj, YGO_REASON_COST); }
static void Op_Kuriboh(struct YgoCtx *c)
{
    Ygo_AddLasting(S, YR_NO_BATTLE_DAMAGE, YGO_SCOPE_PLAYER, ME, NO_OBJ, YGO_UNTIL_BATTLE, 0);
}

/* ---- Steel Scorpion: a non-Machine monster that attacks it is destroyed at the End Phase of your opponent's 2nd
 * turn after the attack */
static int Can_SteelScorpion(struct YgoCtx *c)
{
    const struct YgoCard *a;
    if (c->eventObj == NO_OBJ || S->attackTarget != c->obj + 1u)
        return 0;
    a = CardOf(S, c->eventObj);
    return a && a->race != YGO_MACHINE;
}
static void Op_SteelScorpion(struct YgoCtx *c)
{
    unsigned a = c->eventObj;
    if (a == NO_OBJ || S->objs[a].location != YGO_LOC_MZONE)
        return;
    Ygo_AddLasting(S, YL_DESTROY_AT, YGO_SCOPE_OBJ, ME, a, 4, S->objs[a].moves);
}
/* Electric Lizard: a non-Zombie monster that attacks it can't attack on its next turn */
static int Can_ElectricLizard(struct YgoCtx *c)
{
    const struct YgoCard *a;
    if (c->eventObj == NO_OBJ || S->attackTarget != c->obj + 1u)
        return 0;
    a = CardOf(S, c->eventObj);
    return a && a->race != YGO_ZOMBIE;
}
static void Op_ElectricLizard(struct YgoCtx *c)
{
    if (c->eventObj != NO_OBJ && S->objs[c->eventObj].location == YGO_LOC_MZONE)
        Ygo_AddLasting(S, YR_CANNOT_ATTACK, YGO_SCOPE_OBJ, ME, c->eventObj, 2, 0);
}

/* ---- Blast Juggler: during your Standby Phase, Tribute it: destroy 2 face-up monsters with 1000 or less ATK */
static int Weak(const struct YgoState *s, unsigned o) { return F_FaceUpMonster(s, o) && YgoDuel_Atk(s, o) <= 1000; }
static int Can_BlastJuggler(struct YgoCtx *c)
{
    uint8_t list[YGO_OBJS];
    unsigned n = Fx_Targets(S, ME, 2, YL_MZONE, Weak, list), i, k = 0;
    for (i = 0; i < n; i++)
        k += list[i] != c->obj;
    return S->current == ME && S->phase == YGO_PHASE_STANDBY && k >= 2;
}
static void Cost_TributeSelf(struct YgoCtx *c) { Ygo_SendToGrave(S, c->obj, YGO_REASON_COST | YGO_REASON_TRIBUTE); }
static void Tg_BlastJuggler(struct YgoCtx *c) { Fx_Target(c, 2, YL_MZONE, Weak, 2, YGO_P_DESTROY); }
static void Op_DestroyAllTargets(struct YgoCtx *c)
{
    unsigned i;
    for (i = 0; i < c->link->targetCount; i++)
        if (Fx_TargetOk(c, i, F_Monster))
            Ygo_Destroy(S, TARGET(i), YGO_REASON_EFFECT);
}

/* ---- Battleguards */
static int Stat_Swamp(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (src != target || which != YS_ATK)
        return value;
    return value + 500 * CountOnField(s, s->objs[src].controller, YGO_CARD_LAVA_BATTLEGUARD);
}
static int Stat_Lava(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (src != target || which != YS_ATK)
        return value;
    return value + 500 * CountOnField(s, s->objs[src].controller, YGO_CARD_SWAMP_BATTLEGUARD);
}

/* ---- Time Wizard: call a coin; right: destroy all the opponent's monsters; wrong: yours, and take half their ATK */
static int Can_OppMonster(struct YgoCtx *c) { return Ygo_MonsterCount(S, OPP) > 0 || Ygo_MonsterCount(S, ME) > 0; }
static void Op_TimeWizard(struct YgoCtx *c)
{
    static const uint8_t calls[2] = {1, 0};   /* heads, tails */
    unsigned call = Ygo_AskOption(S, ME, YGO_P_COIN_CALL, CARD, calls, 2), heads = Ygo_Coin(S, ME);
    if (call == heads)
        Fx_DestroyAll(S, OPP, YL_MZONE, NULL, YGO_REASON_EFFECT);
    else
    {
        uint8_t list[YGO_ZONES];
        unsigned n = Ygo_List(S, ME, YGO_LOC_MZONE, list), i;
        int total = 0;
        for (i = 0; i < n; i++)
        {
            int atk = S->objs[list[i]].faceUp ? YgoDuel_Atk(S, list[i]) : 0;
            if (Ygo_Destroy(S, list[i], YGO_REASON_EFFECT))
                total += atk;
        }
        Ygo_Damage(S, ME, total / 2, 0);
    }
}

/* ---- Dragon Piper: destroy face-up Dragon Capture Jars; if any, face-up Dragons to Attack Position */
static int CaptureJar(const struct YgoState *s, unsigned o) { return F_FaceUp(s, o) && s->objs[o].card == YGO_CARD_DRAGON_CAPTURE_JAR; }
static int FaceUpDragon(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *m = CardOf(s, o);
    return F_FaceUpMonster(s, o) && m->race == YGO_DRAGON;
}
static void Op_DragonPiper(struct YgoCtx *c)
{
    if (Fx_DestroyAll(S, 2, YL_SZONE, CaptureJar, YGO_REASON_EFFECT))
    {
        uint8_t list[YGO_ZONES * 2];
        unsigned n = Fx_Collect(S, 2, YL_MZONE, FaceUpDragon, list), i;
        for (i = 0; i < n; i++)
            if (!S->objs[list[i]].attackPos)
                Ygo_ChangePosition(S, list[i], 0);
    }
}

/* ---- Sangan / Witch of the Black Forest: sent from the field to the GY -> search; once per turn by name, and the
 * card found can't be activated this turn */
static int LowAtk(const struct YgoState *s, unsigned o) { return F_MonsterCard(s, o) && YgoDuel_Atk(s, o) <= 1500; }
static int LowDef(const struct YgoState *s, unsigned o) { return F_MonsterCard(s, o) && YgoDuel_Def(s, o) <= 1500; }
static int Can_NameOnce(struct YgoCtx *c) { return !Ygo_LastingValue(S, YL_NAME_USED, ME, CARD); }
static void Searcher(struct YgoCtx *c, int (*filter)(const struct YgoState *, unsigned))
{
    unsigned found;
    Ygo_AddLasting(S, YL_NAME_USED, YGO_SCOPE_PLAYER, ME, NO_OBJ, 0, CARD);
    found = Fx_Search(S, ME, filter, CARD);
    if (found != NO_OBJ)
        Ygo_AddLasting(S, YL_NO_ACTIVATE_NAME, YGO_SCOPE_PLAYER, ME, NO_OBJ, 0, S->objs[found].card);
}
static void Op_Sangan(struct YgoCtx *c) { Searcher(c, LowAtk); }
static void Op_Witch(struct YgoCtx *c) { Searcher(c, LowDef); }

/* ---- Catapult Turtle / Cannon Soldier: Tribute 1 monster */
static int Can_TributeAny(struct YgoCtx *c) { return Ygo_MonsterCount(S, ME) > 0; }
static void Cost_Tribute(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], pick;
    unsigned n = Ygo_List(S, ME, YGO_LOC_MZONE, list);
    if (Ygo_AskCards(S, ME, YGO_P_TRIBUTE, CARD, list, n, 1, 1, &pick))
    {
        c->link->value = S->objs[pick].faceUp ? YgoDuel_Atk(S, pick) : 0;
        Ygo_SendToGrave(S, pick, YGO_REASON_COST | YGO_REASON_TRIBUTE);
    }
}
static void Op_Catapult(struct YgoCtx *c) { Ygo_Damage(S, OPP, c->link->value / 2, 0); }
static void Op_Cannon(struct YgoCtx *c) { Ygo_Damage(S, OPP, 500, 0); }

/* ---- Clowns and Tainted Wisdom: position changes */
static int Can_OppTarget(struct YgoCtx *c) { return Fx_HasTarget(S, ME, OPP, YL_MZONE, F_Monster); }
static void Tg_Opp(struct YgoCtx *c) { Fx_Target(c, OPP, YL_MZONE, F_Monster, 1, YGO_P_TARGET); }
static void Op_Shuffle(struct YgoCtx *c) { Ygo_ShuffleDeck(S, ME); }

/* ---- Yado Karu: any number of cards from your hand to the bottom of your Deck */
static int Can_HandCards(struct YgoCtx *c) { return S->players[ME].handCount > 0; }
static void Op_YadoKaru(struct YgoCtx *c)
{
    uint8_t hand[YGO_HAND_MAX], picks[YGO_HAND_MAX];
    unsigned n = Ygo_List(S, ME, YGO_LOC_HAND, hand), k, i;
    k = Ygo_AskCards(S, ME, YGO_P_ORDER, CARD, hand, n, 0, n, picks);
    for (i = 0; i < k; i++)
        Ygo_ToDeck(S, picks[i], 0);
}

/* ---- The Little Swordsman of Aile: Tribute 1 other monster, +700 ATK this turn */
static int Can_Aile(struct YgoCtx *c) { return Ygo_MonsterCount(S, ME) > 1; }
static void Cost_TributeOther(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], other[YGO_ZONES], pick;
    unsigned n = Ygo_List(S, ME, YGO_LOC_MZONE, list), i, k = 0;
    for (i = 0; i < n; i++)
        if (list[i] != c->obj)
            other[k++] = list[i];
    if (Ygo_AskCards(S, ME, YGO_P_TRIBUTE, CARD, other, k, 1, 1, &pick))
        Ygo_SendToGrave(S, pick, YGO_REASON_COST | YGO_REASON_TRIBUTE);
}
static void Op_Aile(struct YgoCtx *c)
{
    if (Ygo_FaceUpMonster(S, c->obj))
        Ygo_AddStat(S, c->obj, 700, 0, 1);
}

/* ---- Jirai Gumo: on its attack, call a coin; wrong: lose half your LP */
static void Op_JiraiGumo(struct YgoCtx *c)
{
    static const uint8_t calls[2] = {1, 0};
    unsigned call = Ygo_AskOption(S, ME, YGO_P_COIN_CALL, CARD, calls, 2);
    if (call != Ygo_Coin(S, ME))
        Ygo_Damage(S, ME, S->players[ME].lp / 2, 0);
}

/* ---- ATK by counts */
static int Stat_ShadowGhoul(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    uint8_t list[YGO_OBJS];
    unsigned n, i, k = 0;
    if (src != target || which != YS_ATK)
        return value;
    n = Ygo_List(s, s->objs[src].controller, YGO_LOC_GRAVE, list);
    for (i = 0; i < n; i++)
        k += F_MonsterCard(s, list[i]);
    return value + 100 * k;
}
static int Stat_MukaMuka(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (src != target || which == YS_LEVEL)
        return value;
    return value + 300 * s->players[s->objs[src].controller].handCount;
}
static int Stat_MahaVailo(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    if (src != target || which != YS_ATK)
        return value;
    return value + 500 * Fx_EquipCount(s, src);
}
/* Insect Soldiers of the Sky: +1000 during the Damage Step only, attacking a WIND monster */
static int Stat_InsectSoldiers(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    unsigned t = s->attackTarget ? s->attackTarget - 1u : NO_OBJ;
    if (src != target || which != YS_ATK || !s->damageStep || s->attacker != src + 1u || t == NO_OBJ)
        return value;
    return CardOf(s, t)->attribute == YGO_WIND ? value + 1000 : value;
}
/* Attribute auras: ATK up for one attribute, down for another */
static int Aura(const struct YgoState *s, unsigned target, int which, int value, unsigned up, unsigned down)
{
    const struct YgoCard *m = CardOf(s, target);
    if (which != YS_ATK || !m || !s->objs[target].faceUp)
        return value;
    if (m->attribute == up)
        return value + 500;
    if (m->attribute == down)
        return value - 400;
    return value;
}
static int Stat_Hoshiningen(const struct YgoState *s, unsigned src, unsigned t, int w, int v) { return Aura(s, t, w, v, YGO_LIGHT, YGO_DARK); }
static int Stat_StarBoy(const struct YgoState *s, unsigned src, unsigned t, int w, int v) { return Aura(s, t, w, v, YGO_WATER, YGO_FIRE); }
static int Stat_MilusRadiant(const struct YgoState *s, unsigned src, unsigned t, int w, int v) { return Aura(s, t, w, v, YGO_EARTH, YGO_WIND); }
static int Stat_LittleChimera(const struct YgoState *s, unsigned src, unsigned t, int w, int v) { return Aura(s, t, w, v, YGO_FIRE, YGO_WATER); }
static int Stat_Bladefly(const struct YgoState *s, unsigned src, unsigned t, int w, int v) { return Aura(s, t, w, v, YGO_WIND, YGO_EARTH); }
static int Stat_WitchsApprentice(const struct YgoState *s, unsigned src, unsigned t, int w, int v) { return Aura(s, t, w, v, YGO_DARK, YGO_LIGHT); }

/* ---- Thunder Dragon: discard it; add up to 2 Thunder Dragons from your Deck */
static int ThunderDragon(const struct YgoState *s, unsigned o) { return s->objs[o].card == YGO_CARD_THUNDER_DRAGON; }
static int Can_ThunderDragon(struct YgoCtx *c) { return Ygo_Exists(S, ME, YGO_LOC_DECK, ThunderDragon); }
static void Op_ThunderDragon(struct YgoCtx *c)
{
    uint8_t list[YGO_DECK_MAX], picks[2];
    unsigned n = Fx_Collect(S, ME, YL_DECK, ThunderDragon, list), k, i;
    k = Ygo_AskCards(S, ME, YGO_P_ADD_TO_HAND, CARD, list, n, 1, 2, picks);
    for (i = 0; i < k; i++)
    {
        Ygo_ToHand(S, picks[i], YGO_REASON_EFFECT);
        Ygo_Reveal(S, picks[i]);
    }
    Ygo_ShuffleDeck(S, ME);
}

/* ---- Dark Elf: 1000 LP to attack. Toons: 500 LP. */
static int Rule_DarkElf(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_ATTACK_COST && a == src ? 1000 : 0;
}

/* ---- Mushroom Man #2: its controller loses 300 at their Standby Phase; at their End Phase they can pay 500 LP to
 * give it to the opponent */
static void Op_Lose300(struct YgoCtx *c) { Ygo_Damage(S, ME, 300, 0); }
static int Can_MushroomShift(struct YgoCtx *c) { return Fx_CanPay(S, ME, 500) && Ygo_FreeZones(S, OPP, 0); }
static void Op_MushroomShift(struct YgoCtx *c)
{
    if (!Ygo_FaceUpMonster(S, c->obj) || !Fx_CanPay(S, ME, 500))
        return;
    Fx_Pay(S, ME, 500);
    Ygo_TakeControl(S, c->obj, OPP);
}

/* ---- The Unhappy Maiden: sent to the GY by battle -> the Battle Phase ends */
static void Op_EndBattle(struct YgoCtx *c) { S->endBattle = 1; }

/* ---- Barrel Dragon: target 1 opponent's monster; 3 coins, 2 heads destroy it */
static void Op_BarrelDragon(struct YgoCtx *c)
{
    unsigned heads = Ygo_Coin(S, ME) + Ygo_Coin(S, ME) + Ygo_Coin(S, ME);
    if (heads >= 2 && Fx_TargetOk(c, 0, F_Monster))
        Ygo_Destroy(S, TARGET(0), YGO_REASON_EFFECT);
}

/* ---- Gearfried the Iron Knight: Equip Cards on it are destroyed */
static int Rule_Gearfried(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_NO_EQUIP && a == src;
}

/* ---- Karate Man: double its original ATK once per turn; destroyed in that End Phase */
static void Op_KarateMan(struct YgoCtx *c)
{
    if (!Ygo_FaceUpMonster(S, c->obj))
        return;
    Ygo_AddStat(S, c->obj, CardOf(S, c->obj)->atk, 0, 1);
    S->objs[c->obj].data |= 2;
}
static int Can_KarateEnd(struct YgoCtx *c) { return (S->objs[c->obj].data & 2) != 0; }
static void Op_DestroySelf(struct YgoCtx *c)
{
    if (Ygo_OnField(S, c->obj))
        Ygo_Destroy(S, c->obj, YGO_REASON_EFFECT);
}

/* ---- Sonic Bird: add a Ritual Spell from the Deck */
static int RitualSpell(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = CardOf(s, o);
    return c && c->frame == YGO_SPELL && c->property == YGO_PROP_RITUAL;
}
static int Can_SonicBird(struct YgoCtx *c) { return Ygo_Exists(S, ME, YGO_LOC_DECK, RitualSpell); }
static void Op_SonicBird(struct YgoCtx *c) { Fx_Search(S, ME, RitualSpell, CARD); }

/* ---- Jigen Bakudan: once flipped, Tribute it in your Standby Phase: destroy your monsters; half their ATK to the
 * opponent */
static void Op_JigenFlip(struct YgoCtx *c) { S->objs[c->obj].data |= 4; }
static int Can_Jigen(struct YgoCtx *c) { return (S->objs[c->obj].data & 4) && Ygo_FaceUpMonster(S, c->obj); }
static void Op_Jigen(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES];
    unsigned n, i;
    int total = 0;
    if (!Ygo_FaceUpMonster(S, c->obj))
        return;
    Ygo_SendToGrave(S, c->obj, YGO_REASON_TRIBUTE | YGO_REASON_EFFECT);
    n = Ygo_List(S, ME, YGO_LOC_MZONE, list);
    for (i = 0; i < n; i++)
    {
        int atk = S->objs[list[i]].faceUp ? YgoDuel_Atk(S, list[i]) : 0;
        if (Ygo_Destroy(S, list[i], YGO_REASON_EFFECT))
            total += atk;
    }
    Ygo_Damage(S, OPP, total / 2, 0);
}

/* ---- Relinquished: once per turn, equip 1 of the opponent's monsters; its ATK/DEF become that monster's; that
 * monster is destroyed instead of it in battle; battle damage you take from its battles hits the opponent too */
void Fx_PlaceAsEquip(struct YgoState *s, unsigned o, unsigned player, unsigned monster);
static unsigned Absorbed(const struct YgoState *s, unsigned relinquished)
{
    unsigned o;
    for (o = 0; o < s->objCount; o++)
        if (s->objs[o].equipTo == relinquished + 1 && s->objs[o].location == YGO_LOC_SZONE && F_MonsterCard(s, o) &&
            (s->objs[o].data & 8))
            return o;
    return NO_OBJ;
}
static int Can_Relinquished(struct YgoCtx *c)
{
    return Absorbed(S, c->obj) == NO_OBJ && Ygo_FreeZones(S, ME, 1) && Fx_HasTarget(S, ME, OPP, YL_MZONE, F_Monster);
}
static void Op_Relinquished(struct YgoCtx *c)
{
    unsigned t = TARGET(0);
    if (!Fx_TargetOk(c, 0, F_Monster) || !Ygo_FaceUpMonster(S, c->obj) || Absorbed(S, c->obj) != NO_OBJ)
        return;
    Fx_PlaceAsEquip(S, t, ME, c->obj);
    S->objs[t].data |= 8;
}
static int Stat_Relinquished(const struct YgoState *s, unsigned src, unsigned target, int which, int value)
{
    unsigned eq;
    const struct YgoCard *m;
    if (src != target || which == YS_LEVEL)
        return value;
    eq = Absorbed(s, src);
    if (eq == NO_OBJ)
        return value;
    m = CardOf(s, eq);
    return which == YS_ATK ? value - CardOf(s, src)->atk + (m->atk == YGO_STAT_UNKNOWN ? 0 : m->atk)
                           : value - CardOf(s, src)->def + (m->def == YGO_STAT_UNKNOWN ? 0 : m->def);
}
static int Rule_Relinquished(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    unsigned eq;
    if (a != src)
        return 0;
    eq = Absorbed(s, src);
    if (rule == YR_BATTLE_SUBSTITUTE)
        return eq == NO_OBJ ? 0 : (int)eq + 1;
    if (rule == YR_MIRROR_DAMAGE)
        return eq != NO_OBJ;
    return 0;
}

/* ---- Toons: Special Summoned from the hand while you control Toon World (Tributes: 0, 1 or 2); can't attack the
 * turn they're Summoned; 500 LP to attack; attack directly unless the opponent controls a Toon; destroyed with Toon
 * World */
static int ToonCount(unsigned card)
{
    return card == YGO_CARD_BLUE_EYES_TOON_DRAGON || card == YGO_CARD_MANGA_RYU_RAN ? 2
           : card == YGO_CARD_TOON_SUMMONED_SKULL                               ? 1
                                                                                 : 0;
}
static int Can_Toon(struct YgoCtx *c)
{
    unsigned need = ToonCount(CARD);
    if (!Fx_OnFieldFaceUp(S, ME, YGO_CARD_TOON_WORLD))
        return 0;
    return need ? Ygo_MonsterCount(S, ME) >= need : Ygo_FreeZones(S, ME, 0) > 0;
}
static void Cost_Toon(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], picks[2];
    unsigned need = ToonCount(CARD), n = Ygo_List(S, ME, YGO_LOC_MZONE, list), k, i;
    if (!need)
        return;
    k = Ygo_AskCards(S, ME, YGO_P_TRIBUTE, CARD, list, n, need, need, picks);
    for (i = 0; i < k; i++)
        Ygo_SendToGrave(S, picks[i], YGO_REASON_COST | YGO_REASON_TRIBUTE);
}
static int IsToon(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = CardOf(s, o);
    return F_FaceUpMonster(s, o) && c && (c->abilities & YGO_ABILITY_TOON);
}
static int Rule_Toon(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    if (a != src)
        return 0;
    switch (rule)
    {
    case YR_CANNOT_NORMAL_SUMMON:
        return 0;
    case YR_CANNOT_ATTACK:
        return s->objs[src].enteredTurn == s->turn && s->objs[src].summonType == YGO_SUMMON_SPECIAL;
    case YR_ATTACK_COST:
        return 500;
    case YR_DIRECT_ATTACK:
        return !Ygo_Exists(s, s->objs[src].controller ^ 1, YGO_LOC_MZONE, IsToon);
    }
    return 0;
}
static int Rule_ToonNoNormal(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_CANNOT_NORMAL_SUMMON && a == s->objs[src].card;
}
static int Can_ToonWorldGone(struct YgoCtx *c)
{
    return c->eventObj != NO_OBJ && S->objs[c->eventObj].card == YGO_CARD_TOON_WORLD &&
           S->objs[c->eventObj].controller == ME;
}

/* ---- Sheep Tokens can't be Tributed for a Tribute Summon */
static int Rule_Sheep(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    return rule == YR_NOT_TRIBUTE && a == src;
}

#define DIRECT(card) PASSIVE(card, NULL, Rule_Direct)
#define NO_NORMAL(card) PASSIVE(card, NULL, Rule_NoNormal)
#define SUMMON_RULE(card, can, cost) FX(card, YE_SUMMON_RULE, 0, 0, 0, can, cost, NULL, NULL)
#define TOON(card)                                                   \
    {card, YE_CONTINUOUS, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, Rule_Toon, 0}, \
        PASSIVE(card, NULL, Rule_ToonNoNormal), SUMMON_RULE(card, Can_Toon, Cost_Toon), \
        TRIG(card, 0, YEV_ANY_DESTROYED, Can_ToonWorldGone, NULL, Op_DestroySelf)

const struct YgoEffect gYgoFxS1Monsters[] = {
    /* FLIP */
    FLIP(YGO_CARD_MAN_EATER_BUG, 0, Can_AnyMonster, Tg_ManEater, Op_DestroyTarget),
    FLIP(YGO_CARD_HANE_HANE, 0, Can_AnyMonster, Tg_Bounce, Op_BounceTargets),
    FLIP(YGO_CARD_PENGUIN_SOLDIER, YF_OPTIONAL, Can_AnyMonster, Tg_Penguin, Op_BounceTargets),
    FLIP(YGO_CARD_REAPER_OF_THE_CARDS, 0, Can_TrapKiller, Tg_TrapKiller, Op_TrapKiller),
    FLIP(YGO_CARD_TRAP_MASTER, 0, Can_TrapKiller, Tg_TrapKiller, Op_TrapKiller),
    FLIP(YGO_CARD_ARMED_NINJA, 0, Can_SpellKiller, Tg_SpellKiller, Op_SpellKiller),
    FLIP(YGO_CARD_MASK_OF_DARKNESS, 0, Can_MaskOfDarkness, Tg_GraveTrap, Op_GraveToHand),
    FLIP(YGO_CARD_MAGICIAN_OF_FAITH, 0, Can_MagicianOfFaith, Tg_GraveSpell, Op_GraveToHand),
    FLIP(YGO_CARD_THE_STERN_MYSTIC, 0, NULL, NULL, Op_SternMystic),
    FLIP(YGO_CARD_BIG_EYE, 0, NULL, NULL, Op_BigEye),
    FLIP(YGO_CARD_HIRO_S_SHADOW_SCOUT, 0, NULL, NULL, Op_HiroScout),
    FLIP(YGO_CARD_PRINCESS_OF_TSURUGI, 0, NULL, NULL, Op_Tsurugi),
    FLIP(YGO_CARD_THE_IMMORTAL_OF_THUNDER, 0, NULL, NULL, Op_Gain3000),
    TRIG(YGO_CARD_THE_IMMORTAL_OF_THUNDER, 0, YEV_FROM_FIELD_TO_GRAVE, NULL, NULL, Op_Lose5000),
    FLIP(YGO_CARD_CASTLE_OF_DARK_ILLUSIONS, 0, NULL, NULL, Op_CastleFlip),
    {YGO_CARD_CASTLE_OF_DARK_ILLUSIONS, YE_CONTINUOUS, 0, 0, 0, NULL, NULL, NULL, NULL, Stat_Castle, NULL, 0},
    TRIG(YGO_CARD_CASTLE_OF_DARK_ILLUSIONS, 0, YEV_STANDBY_OWN, Can_CastleGrow, NULL, Op_CastleGrow),
    PASSIVE(YGO_CARD_PUMPKING_THE_KING_OF_GHOSTS, Stat_Pumpking, NULL),
    TRIG(YGO_CARD_PUMPKING_THE_KING_OF_GHOSTS, 0, YEV_STANDBY_OWN, Can_PumpkingGrow, NULL, Op_PumpkingGrow),
    FLIP(YGO_CARD_DRAGON_PIPER, 0, NULL, NULL, Op_DragonPiper),
    FLIP(YGO_CARD_JIGEN_BAKUDAN, 0, NULL, NULL, Op_JigenFlip),
    TRIG(YGO_CARD_JIGEN_BAKUDAN, YF_OPTIONAL, YEV_STANDBY_OWN, Can_Jigen, NULL, Op_Jigen),
    /* always on */
    PASSIVE(YGO_CARD_EXODIA_THE_FORBIDDEN_ONE, NULL, Rule_None),
    PASSIVE(YGO_CARD_LORD_OF_D, NULL, Rule_LordOfD),
    PASSIVE(YGO_CARD_SWAMP_BATTLEGUARD, Stat_Swamp, NULL),
    PASSIVE(YGO_CARD_LAVA_BATTLEGUARD, Stat_Lava, NULL),
    PASSIVE(YGO_CARD_SHADOW_GHOUL, Stat_ShadowGhoul, NULL),
    PASSIVE(YGO_CARD_MUKA_MUKA, Stat_MukaMuka, NULL),
    PASSIVE(YGO_CARD_MAHA_VAILO, Stat_MahaVailo, NULL),
    PASSIVE(YGO_CARD_INSECT_SOLDIERS_OF_THE_SKY, Stat_InsectSoldiers, NULL),
    PASSIVE(YGO_CARD_HOSHININGEN, Stat_Hoshiningen, NULL),
    PASSIVE(YGO_CARD_STAR_BOY, Stat_StarBoy, NULL),
    PASSIVE(YGO_CARD_MILUS_RADIANT, Stat_MilusRadiant, NULL),
    PASSIVE(YGO_CARD_LITTLE_CHIMERA, Stat_LittleChimera, NULL),
    PASSIVE(YGO_CARD_BLADEFLY, Stat_Bladefly, NULL),
    PASSIVE(YGO_CARD_WITCH_S_APPRENTICE, Stat_WitchsApprentice, NULL),
    PASSIVE(YGO_CARD_DARK_ELF, NULL, Rule_DarkElf),
    PASSIVE(YGO_CARD_GEARFRIED_THE_IRON_KNIGHT, NULL, Rule_Gearfried),
    PASSIVE(YGO_CARD_SHEEP_TOKEN, NULL, Rule_Sheep),
    DIRECT(YGO_CARD_MYSTIC_LAMP),
    DIRECT(YGO_CARD_LEGHUL),
    DIRECT(YGO_CARD_OOGUCHI),
    DIRECT(YGO_CARD_JINZO_7),
    DIRECT(YGO_CARD_RAINBOW_FLOWER),
    DIRECT(YGO_CARD_QUEEN_S_DOUBLE),
    /* their own Summons */
    NO_NORMAL(YGO_CARD_GATE_GUARDIAN),
    SUMMON_RULE(YGO_CARD_GATE_GUARDIAN, Can_GateGuardian, Cost_GateGuardian),
    NO_NORMAL(YGO_CARD_LARVAE_MOTH),
    SUMMON_RULE(YGO_CARD_LARVAE_MOTH, Can_LarvaeMoth, Cost_LarvaeMoth),
    NO_NORMAL(YGO_CARD_GREAT_MOTH),
    SUMMON_RULE(YGO_CARD_GREAT_MOTH, Can_GreatMoth, Cost_GreatMoth),
    NO_NORMAL(YGO_CARD_HARPIE_LADY_SISTERS),
    FX(YGO_CARD_COCOON_OF_EVOLUTION, YE_IGNITION, 1, YF_FROM_HAND | YF_TARGETS, 0, Can_Cocoon, NULL, Tg_Cocoon,
       Op_Cocoon),
    {YGO_CARD_COCOON_OF_EVOLUTION, YE_CONTINUOUS, 0, 0, 0, NULL, NULL, NULL, NULL, Stat_Cocoon, NULL, 0},
    TRIG(YGO_CARD_COCOON_OF_EVOLUTION, 0, YEV_STANDBY_OWN, Can_CocoonCount, NULL, Op_CocoonCount),
    /* triggers */
    TRIG(YGO_CARD_WALL_OF_ILLUSION, 0, YEV_BATTLED, Can_WallOfIllusion, NULL, Op_WallOfIllusion),
    TRIG(YGO_CARD_THE_WICKED_WORM_BEAST, 0, YEV_END_OWN, OnFieldSelf, NULL, Op_ToHandSelf),
    TRIG(YGO_CARD_MYSTERIOUS_PUPPETEER, 0, YEV_ANY_NORMAL_SUMMONED, NULL, NULL, Op_Gain500),
    TRIG(YGO_CARD_WHITE_MAGICAL_HAT, 0, YEV_BATTLE_DAMAGE, Can_OppHasHand, NULL, Op_OppDiscardRandom),
    TRIG(YGO_CARD_MASKED_SORCERER, 0, YEV_BATTLE_DAMAGE, NULL, NULL, Op_Draw1),
    TRIG(YGO_CARD_THE_BISTRO_BUTCHER, 0, YEV_BATTLE_DAMAGE, NULL, NULL, Op_OppDraw2),
    TRIG(YGO_CARD_STEEL_SCORPION, 0, YEV_BATTLED, Can_SteelScorpion, NULL, Op_SteelScorpion),
    TRIG(YGO_CARD_ELECTRIC_LIZARD, 0, YEV_BATTLED, Can_ElectricLizard, NULL, Op_ElectricLizard),
    TRIG(YGO_CARD_SANGAN, YF_ONCE_PER_TURN, YEV_FROM_FIELD_TO_GRAVE, Can_NameOnce, NULL, Op_Sangan),
    TRIG(YGO_CARD_WITCH_OF_THE_BLACK_FOREST, YF_ONCE_PER_TURN, YEV_FROM_FIELD_TO_GRAVE, Can_NameOnce, NULL, Op_Witch),
    TRIG(YGO_CARD_CRASS_CLOWN, YF_TARGETS, YEV_TO_ATTACK, Can_OppTarget, Tg_Opp, Op_BounceTargets),
    TRIG(YGO_CARD_DREAM_CLOWN, YF_TARGETS, YEV_TO_DEFENSE, Can_OppTarget, Tg_Opp, Op_DestroyTarget),
    TRIG(YGO_CARD_TAINTED_WISDOM, 0, YEV_TO_DEFENSE, NULL, NULL, Op_Shuffle),
    TRIG(YGO_CARD_YADO_KARU, YF_OPTIONAL | YF_WHEN, YEV_TO_DEFENSE, Can_HandCards, NULL, Op_YadoKaru),
    TRIG(YGO_CARD_JIRAI_GUMO, 0, YEV_DECLARED_ATTACK, NULL, NULL, Op_JiraiGumo),
    TRIG(YGO_CARD_MUSHROOM_MAN_2, 0, YEV_STANDBY_OWN, OnFieldSelf, NULL, Op_Lose300),
    TRIG(YGO_CARD_MUSHROOM_MAN_2, YF_OPTIONAL, YEV_END_OWN, Can_MushroomShift, NULL, Op_MushroomShift),
    TRIG(YGO_CARD_THE_UNHAPPY_MAIDEN, 0, YEV_DESTROYED_BATTLE, NULL, NULL, Op_EndBattle),
    TRIG(YGO_CARD_SONIC_BIRD, YF_OPTIONAL | YF_WHEN, YEV_NORMAL_SUMMONED | YEV_FLIP_SUMMONED, Can_SonicBird, NULL,
         Op_SonicBird),
    /* activated */
    FX(YGO_CARD_SANGA_OF_THE_THUNDER, YE_QUICK, 2, YF_DAMAGE_STEP | YF_HARD_ONCE | YF_TARGETS, 0, Can_Sanga, NULL,
       Tg_Attacker, Op_Sanga),
    FX(YGO_CARD_KAZEJIN, YE_QUICK, 2, YF_DAMAGE_STEP | YF_HARD_ONCE | YF_TARGETS, 0, Can_Sanga, NULL, Tg_Attacker,
       Op_Sanga),
    FX(YGO_CARD_SUIJIN, YE_QUICK, 2, YF_DAMAGE_STEP | YF_HARD_ONCE | YF_TARGETS, 0, Can_Sanga, NULL, Tg_Attacker,
       Op_Sanga),
    FX(YGO_CARD_KURIBOH, YE_QUICK, 2, YF_DAMAGE_STEP | YF_FROM_HAND, 0, Can_Kuriboh, Cost_DiscardSelf, NULL,
       Op_Kuriboh),
    FX(YGO_CARD_BLAST_JUGGLER, YE_QUICK, 2, YF_TARGETS | YF_PHASE, 0, Can_BlastJuggler, Cost_TributeSelf, Tg_BlastJuggler,
       Op_DestroyAllTargets),
    FX(YGO_CARD_TIME_WIZARD, YE_IGNITION, 1, YF_ONCE_PER_TURN, 0, Can_OppMonster, NULL, NULL, Op_TimeWizard),
    FX(YGO_CARD_CATAPULT_TURTLE, YE_IGNITION, 1, YF_ONCE_PER_TURN, 0, Can_TributeAny, Cost_Tribute, NULL, Op_Catapult),
    FX(YGO_CARD_CANNON_SOLDIER, YE_IGNITION, 1, 0, 0, Can_TributeAny, Cost_Tribute, NULL, Op_Cannon),
    FX(YGO_CARD_THE_LITTLE_SWORDSMAN_OF_AILE, YE_IGNITION, 1, 0, 0, Can_Aile, Cost_TributeOther, NULL, Op_Aile),
    FX(YGO_CARD_THUNDER_DRAGON, YE_IGNITION, 1, YF_FROM_HAND, 0, Can_ThunderDragon, Cost_DiscardSelf, NULL,
       Op_ThunderDragon),
    FX(YGO_CARD_BARREL_DRAGON, YE_IGNITION, 1, YF_ONCE_PER_TURN | YF_TARGETS, 0, Can_OppTarget, NULL, Tg_Opp,
       Op_BarrelDragon),
    FX(YGO_CARD_KARATE_MAN, YE_IGNITION, 1, YF_ONCE_PER_TURN, 0, NULL, NULL, NULL, Op_KarateMan),
    TRIG(YGO_CARD_KARATE_MAN, 0, YEV_END_OWN, Can_KarateEnd, NULL, Op_DestroySelf),
    FX(YGO_CARD_RELINQUISHED, YE_IGNITION, 1, YF_ONCE_PER_TURN | YF_TARGETS, 0, Can_Relinquished, NULL, Tg_Opp,
       Op_Relinquished),
    {YGO_CARD_RELINQUISHED, YE_CONTINUOUS, 0, 0, 0, NULL, NULL, NULL, NULL, Stat_Relinquished, Rule_Relinquished, 0},
    /* Toons */
    TOON(YGO_CARD_BLUE_EYES_TOON_DRAGON),
    TOON(YGO_CARD_TOON_SUMMONED_SKULL),
    TOON(YGO_CARD_MANGA_RYU_RAN),
    TOON(YGO_CARD_TOON_MERMAID),
};
const unsigned gYgoFxS1MonstersCount = sizeof(gYgoFxS1Monsters) / sizeof(gYgoFxS1Monsters[0]);
