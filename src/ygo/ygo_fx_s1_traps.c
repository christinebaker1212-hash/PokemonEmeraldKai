/* Yu-Gi-Oh! effects, stage 1: Trap Cards. */
#include "ygo_fx.h"

/* ---- the moments Traps answer */
static int OppAttacks(struct YgoCtx *c)
{
    return S->window == YW_ATTACK && S->attacker && S->objs[S->attacker - 1].controller == OPP;
}
static int OppSummoned(struct YgoCtx *c)
{
    unsigned o = S->windowObj;
    return S->window == YW_SUMMONED && o < S->objCount && S->objs[o].location == YGO_LOC_MZONE &&
           S->objs[o].controller == OPP;
}
static int SummonWindow(struct YgoCtx *c)
{
    return S->window == YW_SUMMONING && S->chainCount == 0 && S->windowObj < S->objCount &&
           S->objs[S->windowObj].location == YGO_LOC_MZONE;
}
/* The chain link just below this one (as it's being activated: the top) is a Spell (frame) card's activation */
static int TopIsActivation(struct YgoCtx *c, unsigned frame)
{
    const struct YgoLink *l;
    const struct YgoCard *card;
    if (!S->chainCount)
        return 0;
    l = &S->chain[S->chainCount - 1];
    card = Ygo_Card(l->card);
    return l->activation && card && card->frame == frame && !l->negated;
}
/* Remember which link is answered (at activation our own link is the top) */
static void Tg_Below(struct YgoCtx *c) { c->link->value = S->chainCount >= 2 ? S->chainCount - 2 : -1; }
static void Op_NegateDestroy(struct YgoCtx *c)
{
    if (c->link->value >= 0)
        Ygo_NegateLink(S, c->link->value, 1);
}

/* ---- Trap Hole: when the opponent Normal / Flip Summons a monster with 1000+ ATK */
static int Can_TrapHole(struct YgoCtx *c)
{
    unsigned o = S->windowObj;
    return OppSummoned(c) && S->objs[o].summonType <= YGO_SUMMON_FLIP && S->objs[o].faceUp &&
           YgoDuel_Atk(S, o) >= 1000 && Ygo_CanTarget(S, o, ME);
}
static void Tg_Summoned(struct YgoCtx *c)
{
    c->link->targetMoves[0] = S->objs[S->windowObj].moves;
    c->link->targets[0] = S->windowObj;
    c->link->targetCount = 1;
}
static void Op_DestroyTarget(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_Monster))
        Ygo_Destroy(S, TARGET(0), YGO_REASON_EFFECT);
}

/* ---- Mirror Force: destroy all the opponent's Attack Position monsters */
static void Op_MirrorForce(struct YgoCtx *c) { Fx_DestroyAll(S, OPP, YL_MZONE, F_AttackPos, YGO_REASON_EFFECT); }

/* ---- Enchanted Javelin: gain LP equal to the attacker's ATK */
static void Tg_Attacker(struct YgoCtx *c)
{
    unsigned a = S->attacker - 1;
    c->link->targetMoves[0] = S->objs[a].moves;
    c->link->targets[0] = a;
    c->link->targetCount = 1;
}
static void Op_Javelin(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_FaceUpMonster))
        Ygo_Recover(S, ME, YgoDuel_Atk(S, TARGET(0)));
}

/* ---- Two-Pronged Attack: 2 of yours and 1 of theirs */
static int Can_TwoPronged(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES];
    return Fx_Targets(S, ME, ME, YL_MZONE, F_Monster, list) >= 2 && Fx_HasTarget(S, ME, OPP, YL_MZONE, F_Monster);
}
static void Tg_TwoPronged(struct YgoCtx *c)
{
    Fx_Target(c, ME, YL_MZONE, F_Monster, 2, YGO_P_DESTROY);
    Fx_Target(c, OPP, YL_MZONE, F_Monster, 1, YGO_P_DESTROY);
}
static void Op_TwoPronged(struct YgoCtx *c)
{
    unsigned i;
    for (i = 0; i < c->link->targetCount; i++)
        if (Fx_TargetOk(c, i, F_Monster))
            Ygo_Destroy(S, TARGET(i), YGO_REASON_EFFECT);
}

/* ---- stats until the end of the turn */
static int Can_FaceUp(struct YgoCtx *c) { return Fx_HasTarget(S, ME, 2, YL_MZONE, F_FaceUpMonster); }
static void Tg_FaceUp(struct YgoCtx *c) { Fx_Target(c, 2, YL_MZONE, F_FaceUpMonster, 1, YGO_P_TARGET); }
static void Op_Reinforcements(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_FaceUpMonster))
        Ygo_AddStat(S, TARGET(0), 500, 0, 1);
}
static void Op_CastleWalls(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_FaceUpMonster))
        Ygo_AddStat(S, TARGET(0), 0, 500, 1);
}

/* ---- Waboku: no battle damage this turn, your monsters can't be destroyed by battle */
static void Op_Waboku(struct YgoCtx *c)
{
    Ygo_AddLasting(S, YR_NO_BATTLE_DAMAGE, YGO_SCOPE_PLAYER, ME, NO_OBJ, 0, 0);
    Ygo_AddLasting(S, YR_INDESTRUCTIBLE_BATTLE, YGO_SCOPE_MONSTERS, ME, NO_OBJ, 0, 0);
}

/* ---- Reverse Trap: additions and subtractions to ATK/DEF reversed until the End Phase */
static void Op_ReverseTrap(struct YgoCtx *c) { Ygo_AddLasting(S, YR_REVERSE_STATS, YGO_SCOPE_MONSTERS, 2, NO_OBJ, 0, 0); }

/* ---- Just Desserts: 500 for each monster the opponent controls */
static int Can_JustDesserts(struct YgoCtx *c) { return Ygo_MonsterCount(S, OPP) > 0; }
static void Op_JustDesserts(struct YgoCtx *c) { Ygo_Damage(S, OPP, 500 * Ygo_MonsterCount(S, OPP), 0); }

/* ---- Ultimate Offering: during your Main Phase or the opponent's Battle Phase, pay 500 LP: one more Normal
 * Summon / Set right away */
static int SummonableNow(const struct YgoState *s, unsigned o)
{
    unsigned p = s->objs[o].owner;
    return Ygo_CanNormalSummonNow(s, p, o, 0) || Ygo_CanNormalSummonNow(s, p, o, 1);
}
static int Can_Offering(struct YgoCtx *c)
{
    int mine = S->current == ME && (S->phase == YGO_PHASE_MAIN1 || S->phase == YGO_PHASE_MAIN2) && !S->chainCount;
    int theirs = S->current != ME && S->phase == YGO_PHASE_BATTLE && !S->damageStep;
    return (mine || theirs) && Fx_CanPay(S, ME, 500) && Ygo_Exists(S, ME, YGO_LOC_HAND, SummonableNow);
}
static void Cost_Pay500(struct YgoCtx *c) { Fx_Pay(S, ME, 500); }
static void Op_Offering(struct YgoCtx *c)
{
    uint8_t list[YGO_HAND_MAX], pick;
    unsigned n = Fx_Collect(S, ME, YL_HAND, SummonableNow, list);
    uint8_t options[2], k = 0;
    if (!n || !Ygo_AskCards(S, ME, YGO_P_SUMMON, CARD, list, n, 1, 1, &pick))
        return;
    if (Ygo_CanNormalSummonNow(S, ME, pick, 0))
        options[k++] = 0;
    if (Ygo_CanNormalSummonNow(S, ME, pick, 1))
        options[k++] = 1;
    Ygo_ExtraNormalSummon(S, ME, pick, Ygo_AskOption(S, ME, S->objs[pick].card, options, k));
}

/* ---- Robbin' Goblin: each time a monster you control inflicts battle damage, the opponent discards 1 at random */
static int Can_RobbinGoblin(struct YgoCtx *c)
{
    return c->eventObj != NO_OBJ && S->objs[c->eventObj].controller == ME && S->players[OPP].handCount > 0;
}
static void Op_RandomDiscard(struct YgoCtx *c) { Fx_Discard(S, OPP, 1, 1, YGO_REASON_EFFECT); }

/* ---- Dragon Capture Jar: face-up Dragons go to Defense Position and stay there */
static int FaceUpDragonAtk(const struct YgoState *s, unsigned o)
{
    const struct YgoCard *c = Ygo_ObjCard(s, o);
    return F_AttackPos(s, o) && c && c->race == YGO_DRAGON;
}
static void Op_CaptureJar(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES * 2];
    unsigned n = Fx_Collect(S, 2, YL_MZONE, FaceUpDragonAtk, list), i;
    if (!Ygo_OnField(S, c->obj) || !S->objs[c->obj].faceUp)
        return;
    for (i = 0; i < n; i++)
        Ygo_ChangePosition(S, list[i], 1);
}
static int Can_CaptureJarAgain(struct YgoCtx *c)
{
    return c->eventObj != NO_OBJ && FaceUpDragonAtk(S, c->eventObj);
}
static int Rule_CaptureJar(const struct YgoState *s, unsigned src, unsigned rule, unsigned a, unsigned b)
{
    const struct YgoCard *card = a < s->objCount ? Ygo_ObjCard(s, a) : NULL;
    return rule == YR_CANNOT_CHANGE_POSITION && card && card->race == YGO_DRAGON;
}

/* ---- Counter Traps */
/* Solemn Judgment: a Summon (as it happens) or a Spell/Trap activation; pay half your LP */
static int Can_Solemn(struct YgoCtx *c)
{
    return SummonWindow(c) || TopIsActivation(c, YGO_SPELL) || TopIsActivation(c, YGO_TRAP);
}
static void Cost_Solemn(struct YgoCtx *c)
{
    Fx_Pay(S, ME, S->players[ME].lp / 2);
}
static void Tg_Solemn(struct YgoCtx *c)
{
    /* -1: the Summon; else the link answered */
    c->link->value = S->chainCount >= 2 ? S->chainCount - 2 : -1;
    if (c->link->value < 0)
        Tg_Summoned(c);
}
static void Op_Solemn(struct YgoCtx *c)
{
    if (c->link->value < 0)
    {
        if (Fx_TargetOk(c, 0, F_Monster))
            Ygo_NegateSummon(S, TARGET(0));
    }
    else
        Ygo_NegateLink(S, c->link->value, 1);
}
/* Magic Jammer: a Spell's activation; discard 1 */
static int Can_MagicJammer(struct YgoCtx *c)
{
    return TopIsActivation(c, YGO_SPELL) && S->players[ME].handCount > 0;
}
static void Cost_DiscardOne(struct YgoCtx *c) { Fx_Discard(S, ME, 1, 0, YGO_REASON_COST); }
/* Seven Tools of the Bandit: a Trap's activation; pay 1000 LP */
static int Can_SevenTools(struct YgoCtx *c) { return TopIsActivation(c, YGO_TRAP) && Fx_CanPay(S, ME, 1000); }
static void Cost_Pay1000(struct YgoCtx *c) { Fx_Pay(S, ME, 1000); }
/* Horn of Heaven: a Summon; Tribute 1 monster */
static int Can_HornOfHeaven(struct YgoCtx *c)
{
    unsigned i;
    if (!SummonWindow(c))
        return 0;
    for (i = 0; i < YGO_ZONES; i++)
        if (S->players[ME].monsters[i] && S->players[ME].monsters[i] - 1u != S->windowObj)
            return 1;
    return 0;
}
static void Cost_HornOfHeaven(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES], picks[YGO_ZONES], pick;
    unsigned n = Ygo_List(S, ME, YGO_LOC_MZONE, list), i, k = 0;
    for (i = 0; i < n; i++)
        if (list[i] != S->windowObj)
            picks[k++] = list[i];
    if (Ygo_AskCards(S, ME, YGO_P_TRIBUTE, CARD, picks, k, 1, 1, &pick))
        Ygo_SendToGrave(S, pick, YGO_REASON_COST | YGO_REASON_TRIBUTE);
}
static void Op_NegateSummon(struct YgoCtx *c)
{
    if (Fx_TargetOk(c, 0, F_Monster))
        Ygo_NegateSummon(S, TARGET(0));
}

/* Fake Trap: the opponent activates something that would destroy your Traps: this card is destroyed instead */
static int DestroysTraps(unsigned card)
{
    return card == YGO_CARD_HEAVY_STORM || card == YGO_CARD_MYSTICAL_SPACE_TYPHOON ||
           card == YGO_CARD_HARPIE_S_FEATHER_DUSTER || card == YGO_CARD_REMOVE_TRAP ||
           card == YGO_CARD_TRAP_MASTER || card == YGO_CARD_REAPER_OF_THE_CARDS;
}
static int Can_FakeTrap(struct YgoCtx *c)
{
    const struct YgoLink *l;
    if (!S->chainCount)
        return 0;
    l = &S->chain[S->chainCount - 1];
    return l->player == OPP && DestroysTraps(l->card) && !l->negated;
}
static int OtherTrap(const struct YgoState *s, unsigned o)
{
    return F_SpellTrap(s, o) && (F_Trap(s, o) || !s->objs[o].faceUp);
}
static void Op_FakeTrap(struct YgoCtx *c)
{
    uint8_t list[YGO_ZONES + 1];
    unsigned n = Fx_Collect(S, ME, YL_SZONE, OtherTrap, list), i;
    for (i = 0; i < n; i++)
        if (list[i] != c->obj)
            Ygo_AddLasting(S, YR_INDESTRUCTIBLE_EFFECT, YGO_SCOPE_OBJ, ME, list[i], YGO_UNTIL_CHAIN, 0);
    if (Ygo_OnField(S, c->obj))
        Ygo_Destroy(S, c->obj, YGO_REASON_EFFECT);
}

/* Gryphon Wing: the opponent activates Harpie's Feather Duster: negate it, destroy their Spells and Traps */
static int Can_GryphonWing(struct YgoCtx *c)
{
    const struct YgoLink *l;
    if (!S->chainCount)
        return 0;
    l = &S->chain[S->chainCount - 1];
    return l->player == OPP && l->card == YGO_CARD_HARPIE_S_FEATHER_DUSTER && !l->negated;
}
static void Op_GryphonWing(struct YgoCtx *c)
{
    if (c->link->value < 0)
        return;
    Ygo_NegateLink(S, c->link->value, 0);
    Fx_DestroyAll(S, OPP, YL_SZONE, NULL, YGO_REASON_EFFECT);
}

const struct YgoEffect gYgoFxS1Traps[] = {
    ACT(YGO_CARD_TRAP_HOLE, YF_TARGETS, Can_TrapHole, NULL, Tg_Summoned, Op_DestroyTarget),
    ACT(YGO_CARD_MIRROR_FORCE, 0, OppAttacks, NULL, NULL, Op_MirrorForce),
    ACT(YGO_CARD_ENCHANTED_JAVELIN, 0, OppAttacks, NULL, Tg_Attacker, Op_Javelin),
    ACT(YGO_CARD_TWO_PRONGED_ATTACK, YF_TARGETS, Can_TwoPronged, NULL, Tg_TwoPronged, Op_TwoPronged),
    ACT(YGO_CARD_REINFORCEMENTS, YF_TARGETS | YF_DAMAGE_STEP, Can_FaceUp, NULL, Tg_FaceUp, Op_Reinforcements),
    ACT(YGO_CARD_CASTLE_WALLS, YF_TARGETS | YF_DAMAGE_STEP, Can_FaceUp, NULL, Tg_FaceUp, Op_CastleWalls),
    ACT(YGO_CARD_WABOKU, 0, NULL, NULL, NULL, Op_Waboku),
    ACT(YGO_CARD_REVERSE_TRAP, YF_DAMAGE_STEP, NULL, NULL, NULL, Op_ReverseTrap),
    ACT(YGO_CARD_JUST_DESSERTS, 0, Can_JustDesserts, NULL, NULL, Op_JustDesserts),
    ACT(YGO_CARD_ULTIMATE_OFFERING, 0, NULL, NULL, NULL, NULL),
    FX(YGO_CARD_ULTIMATE_OFFERING, YE_QUICK, 2, 0, 0, Can_Offering, Cost_Pay500, NULL, Op_Offering),
    ACT(YGO_CARD_ROBBIN_GOBLIN, 0, NULL, NULL, NULL, NULL),
    TRIG(YGO_CARD_ROBBIN_GOBLIN, 0, YEV_ANY_BATTLE_DAMAGE, Can_RobbinGoblin, NULL, Op_RandomDiscard),
    ACT_HOOKS(YGO_CARD_DRAGON_CAPTURE_JAR, 0, NULL, NULL, NULL, Op_CaptureJar, NULL, Rule_CaptureJar),
    TRIG(YGO_CARD_DRAGON_CAPTURE_JAR, 0, YEV_ANY_SUMMONED, Can_CaptureJarAgain, NULL, Op_CaptureJar),
    ACT(YGO_CARD_SOLEMN_JUDGMENT, YF_SUMMON_NEGATE | YF_DAMAGE_STEP, Can_Solemn, Cost_Solemn, Tg_Solemn, Op_Solemn),
    ACT(YGO_CARD_MAGIC_JAMMER, YF_DAMAGE_STEP, Can_MagicJammer, Cost_DiscardOne, Tg_Below, Op_NegateDestroy),
    ACT(YGO_CARD_SEVEN_TOOLS_OF_THE_BANDIT, YF_DAMAGE_STEP, Can_SevenTools, Cost_Pay1000, Tg_Below, Op_NegateDestroy),
    ACT(YGO_CARD_HORN_OF_HEAVEN, YF_SUMMON_NEGATE, Can_HornOfHeaven, Cost_HornOfHeaven, Tg_Summoned, Op_NegateSummon),
    ACT(YGO_CARD_FAKE_TRAP, 0, Can_FakeTrap, NULL, NULL, Op_FakeTrap),
    ACT(YGO_CARD_GRYPHON_WING, 0, Can_GryphonWing, NULL, Tg_Below, Op_GryphonWing),
};
const unsigned gYgoFxS1TrapsCount = sizeof(gYgoFxS1Traps) / sizeof(gYgoFxS1Traps[0]);
