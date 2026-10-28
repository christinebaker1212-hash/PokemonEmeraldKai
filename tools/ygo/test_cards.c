/* Yu-Gi-Oh! card scenarios (tools/ygo/test.sh): set up the cards, answer as a player would, check what happened. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ygo/ygo_internal.h"

static const char *sTest = "";
#define CHECK(x)                                                                         \
    do                                                                                   \
    {                                                                                    \
        if (!(x))                                                                        \
        {                                                                                \
            fprintf(stderr, "FAIL [%s] %s:%d: %s\n", sTest, __FILE__, __LINE__, #x);     \
            Dump();                                                                      \
            exit(1);                                                                     \
        }                                                                                \
    } while (0)

static struct YgoDuel d;
#define ST (d.s)

static void Dump(void)
{
    unsigned i;
    fprintf(stderr, "  query type %u player %u prompt %u card %u items:", d.query.type, d.query.player, d.query.prompt,
            d.query.card);
    for (i = 0; i < d.query.count; i++)
    {
        const struct YgoChoice *c = &d.query.items[i];
        fprintf(stderr, " [%u kind %u %s v%u]", i, c->kind,
                c->obj < ST.objCount ? Ygo_Card(ST.objs[c->obj].card)->name : "-", c->value);
    }
    fprintf(stderr, "\n  LP %d / %d, turn %u, phase %u, error '%s'\n", ST.players[0].lp, ST.players[1].lp, ST.turn,
            ST.phase, d.error);
}

/* ---- setting up: every scenario starts at player 0's Main Phase 1 on turn 3 (battles allowed) */
static void Start(const char *name, const uint16_t *cards0, const uint16_t *cards1, const uint16_t *extra0,
                  const uint16_t *extra1)
{
    uint16_t a[YGO_DECK_MAX], b[YGO_DECK_MAX];
    unsigned i, n;
    sTest = name;
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    /* the scenario's cards first, then plain fillers (Mystical Elf: 800/2000) */
    for (n = 0; cards0 && cards0[n]; n++)
        a[n] = cards0[n];
    for (; n < 40; n++)
        a[n] = YGO_CARD_MYSTICAL_ELF;
    for (n = 0; cards1 && cards1[n]; n++)
        b[n] = cards1[n];
    for (; n < 40; n++)
        b[n] = YGO_CARD_MYSTICAL_ELF;
    if (!YgoDuel_Init(&d, a, extra0, b, extra1, 7, 0))
    {
        fprintf(stderr, "init: %s\n", d.error);
        exit(1);
    }
    /* empty both hands back into the decks: scenarios give what they need */
    for (i = 0; i < 2; i++)
        while (ST.players[i].handCount)
        {
            unsigned o = ST.players[i].hand[--ST.players[i].handCount];
            ST.players[i].deck[ST.players[i].deckCount++] = o;
            ST.objs[o].location = YGO_LOC_DECK;
        }
    ST.turn = 3;
}

static void Detach(unsigned o)
{
    struct YgoObj *b = &ST.objs[o];
    struct YgoPlayer *p = &ST.players[b->owner];
    uint8_t *piles[5] = {p->deck, p->hand, p->grave, p->banished, p->extra};
    uint8_t *counts[5] = {&p->deckCount, &p->handCount, &p->graveCount, &p->banishedCount, &p->extraCount};
    unsigned k, i;
    for (k = 0; k < 5; k++)
        for (i = 0; i < *counts[k]; i++)
            if (piles[k][i] == o)
            {
                memmove(piles[k] + i, piles[k] + i + 1, *counts[k] - i - 1);
                (*counts[k])--;
                return;
            }
}

/* An object of player's with that card still in the deck */
static unsigned Find(unsigned player, unsigned card)
{
    unsigned i;
    for (i = 0; i < ST.players[player].deckCount; i++)
        if (ST.objs[ST.players[player].deck[i]].card == card)
            return ST.players[player].deck[i];
    for (i = 0; i < ST.players[player].extraCount; i++)
        if (ST.objs[ST.players[player].extra[i]].card == card)
            return ST.players[player].extra[i];
    fprintf(stderr, "[%s] no %s left for player %u\n", sTest, Ygo_Card(card)->name, player);
    exit(1);
}

static unsigned ToHand(unsigned player, unsigned card)
{
    unsigned o = Find(player, card);
    Detach(o);
    ST.players[player].hand[ST.players[player].handCount++] = o;
    ST.objs[o].location = YGO_LOC_HAND;
    return o;
}
static unsigned ToGrave(unsigned player, unsigned card)
{
    unsigned o = Find(player, card);
    Detach(o);
    ST.players[player].grave[ST.players[player].graveCount++] = o;
    ST.objs[o].location = YGO_LOC_GRAVE;
    ST.objs[o].faceUp = 1;
    return o;
}
/* pos: 0 face-up ATK, 1 face-up DEF, 2 face-down DEF */
static unsigned ToField(unsigned player, unsigned card, unsigned pos)
{
    unsigned o = Find(player, card), z;
    Detach(o);
    for (z = 0; ST.players[player].monsters[z]; z++)
        ;
    ST.players[player].monsters[z] = o + 1;
    ST.objs[o].location = YGO_LOC_MZONE;
    ST.objs[o].seq = z;
    ST.objs[o].controller = player;
    ST.objs[o].faceUp = pos != 2;
    ST.objs[o].attackPos = pos == 0;
    ST.objs[o].enteredTurn = 1;
    ST.objs[o].turnFlags = 0;
    return o;
}
/* A Spell/Trap set face down on an earlier turn */
static unsigned ToSet(unsigned player, unsigned card)
{
    unsigned o = Find(player, card), z;
    Detach(o);
    for (z = 0; ST.players[player].spells[z]; z++)
        ;
    ST.players[player].spells[z] = o + 1;
    ST.objs[o].location = YGO_LOC_SZONE;
    ST.objs[o].seq = z;
    ST.objs[o].controller = player;
    ST.objs[o].faceUp = 0;
    ST.objs[o].enteredTurn = 1;
    return o;
}
static void Go(void)
{
    if (!YgoDuel_Refresh(&d))
    {
        fprintf(stderr, "[%s] refresh: %s\n", sTest, d.error);
        exit(1);
    }
}

/* ---- answering */
static void Answer(unsigned count, const uint8_t *picks)
{
    struct YgoAnswer a;
    memset(&a, 0, sizeof(a));
    a.count = count;
    if (count)
        memcpy(a.picks, picks, count);
    if (!YgoDuel_Answer(&d, &a))
    {
        fprintf(stderr, "[%s] refused: %s\n", sTest, d.error);
        Dump();
        exit(1);
    }
}
/* Main / Battle Phase action kind with card (0: any card / no card) */
static void Do(unsigned kind, unsigned card)
{
    unsigned i;
    for (i = 0; i < d.query.count; i++)
        if (d.query.items[i].kind == kind &&
            (!card || (d.query.items[i].obj < ST.objCount && ST.objs[d.query.items[i].obj].card == card)))
        {
            uint8_t p = i;
            Answer(1, &p);
            return;
        }
    fprintf(stderr, "[%s] no action %u for %s\n", sTest, kind, card ? Ygo_Card(card)->name : "-");
    Dump();
    exit(1);
}
static int Can(unsigned kind, unsigned card)
{
    unsigned i;
    for (i = 0; i < d.query.count; i++)
        if (d.query.items[i].kind == kind &&
            (!card || (d.query.items[i].obj < ST.objCount && ST.objs[d.query.items[i].obj].card == card)))
            return 1;
    return 0;
}
/* Pick the items showing these objects (Q_CARDS / Q_CHAIN) */
static void PickObj(unsigned o)
{
    unsigned i;
    for (i = 0; i < d.query.count; i++)
        if (d.query.items[i].obj == o)
        {
            uint8_t p = i;
            Answer(1, &p);
            return;
        }
    fprintf(stderr, "[%s] can't pick object %u\n", sTest, o);
    Dump();
    exit(1);
}
static void PickCard(unsigned card)
{
    unsigned i;
    for (i = 0; i < d.query.count; i++)
        if (d.query.items[i].obj < ST.objCount && ST.objs[d.query.items[i].obj].card == card)
        {
            uint8_t p = i;
            Answer(1, &p);
            return;
        }
    fprintf(stderr, "[%s] can't pick %s\n", sTest, Ygo_Card(card)->name);
    Dump();
    exit(1);
}
static void PickValue(unsigned v)
{
    unsigned i;
    for (i = 0; i < d.query.count; i++)
        if (d.query.items[i].value == v)
        {
            uint8_t p = i;
            Answer(1, &p);
            return;
        }
    fprintf(stderr, "[%s] no value %u\n", sTest, v);
    Dump();
    exit(1);
}
static void Pass(void) { Answer(0, NULL); }
/* Answer everything that isn't a Main/Battle Phase question with the quiet choice (pass, no, the first items) until
 * player gets a question of type */
static void Until(unsigned player, unsigned type)
{
    unsigned guard = 0;
    while (!(d.query.player == player && d.query.type == type) && d.query.type != YGO_Q_NONE && guard++ < 200)
    {
        if (d.query.type == YGO_Q_CHAIN)
            Pass();
        else if (d.query.type == YGO_Q_YESNO)
            PickValue(0);
        else if (d.query.type == YGO_Q_IDLE)
            Do(YGO_ACT_END_TURN, 0);
        else if (d.query.type == YGO_Q_BATTLE)
            Do(YGO_ACT_TO_MAIN2, 0);
        else
        {
            uint8_t p[8] = {0, 1, 2, 3, 4, 5, 6, 7};
            Answer(d.query.min ? d.query.min : 1, p);
        }
    }
    if (d.query.player != player || d.query.type != type)
    {
        fprintf(stderr, "[%s] never got question %u for player %u\n", sTest, type, player);
        Dump();
        exit(1);
    }
}
static int Where(unsigned o) { return ST.objs[o].location; }

/* ---- scenarios */
static void T_Polymerization(void)
{
    static const uint16_t c0[] = {YGO_CARD_POLYMERIZATION, YGO_CARD_FLAME_MANIPULATOR,
                                  YGO_CARD_MASAKI_THE_LEGENDARY_SWORDSMAN, 0};
    static const uint16_t e0[] = {YGO_CARD_FLAME_SWORDSMAN, 0};
    unsigned fs, m1, m2;
    Start("Polymerization", c0, NULL, e0, NULL);
    ToHand(0, YGO_CARD_POLYMERIZATION);
    m1 = ToHand(0, YGO_CARD_FLAME_MANIPULATOR);
    m2 = ToField(0, YGO_CARD_MASAKI_THE_LEGENDARY_SWORDSMAN, 0);
    fs = Find(0, YGO_CARD_FLAME_SWORDSMAN);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_POLYMERIZATION);
    CHECK(d.query.type == YGO_Q_POSITION);
    PickValue(0);
    CHECK(Where(fs) == YGO_LOC_MZONE && ST.objs[fs].attackPos && ST.objs[fs].properly);
    CHECK(Where(m1) == YGO_LOC_GRAVE && Where(m2) == YGO_LOC_GRAVE);
    CHECK(YgoDuel_Atk(&ST, fs) == 1800);
    CHECK(ST.players[0].graveCount == 3);   /* both materials and Polymerization */
}

static void T_MirrorForce(void)
{
    static const uint16_t c1[] = {YGO_CARD_MIRROR_FORCE, 0};
    static const uint16_t c0[] = {YGO_CARD_BLUE_EYES_WHITE_DRAGON, YGO_CARD_SUMMONED_SKULL, 0};
    unsigned bewd, skull, mf;
    Start("Mirror Force", c0, c1, NULL, NULL);
    bewd = ToField(0, YGO_CARD_BLUE_EYES_WHITE_DRAGON, 0);
    skull = ToField(0, YGO_CARD_SUMMONED_SKULL, 0);
    mf = ToSet(1, YGO_CARD_MIRROR_FORCE);
    Go();
    Do(YGO_ACT_TO_BATTLE, 0);
    Until(0, YGO_Q_BATTLE);
    Do(YGO_ACT_ATTACK, YGO_CARD_BLUE_EYES_WHITE_DRAGON);
    Until(1, YGO_Q_CHAIN);
    PickObj(mf);
    Until(0, YGO_Q_BATTLE);
    CHECK(Where(bewd) == YGO_LOC_GRAVE && Where(skull) == YGO_LOC_GRAVE && Where(mf) == YGO_LOC_GRAVE);
    CHECK(ST.players[1].lp == 8000);
}

static void T_TrapHole(void)
{
    static const uint16_t c0[] = {YGO_CARD_GEMINI_ELF, YGO_CARD_KURIBOH, 0};
    static const uint16_t c1[] = {YGO_CARD_TRAP_HOLE, 0};
    unsigned elf, kuri, th;
    Start("Trap Hole", c0, c1, NULL, NULL);
    elf = ToHand(0, YGO_CARD_GEMINI_ELF);
    kuri = ToHand(0, YGO_CARD_KURIBOH);
    th = ToSet(1, YGO_CARD_TRAP_HOLE);
    Go();
    /* Kuriboh (300 ATK) doesn't set it off: Gemini Elf (1900) does */
    Do(YGO_ACT_SUMMON, YGO_CARD_GEMINI_ELF);
    Until(1, YGO_Q_CHAIN);
    PickObj(th);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(elf) == YGO_LOC_GRAVE && Where(th) == YGO_LOC_GRAVE);
    (void)kuri;
}

static void T_Solemn(void)
{
    static const uint16_t c0[] = {YGO_CARD_GEMINI_ELF, 0};
    static const uint16_t c1[] = {YGO_CARD_SOLEMN_JUDGMENT, 0};
    unsigned elf, sj;
    Start("Solemn Judgment", c0, c1, NULL, NULL);
    elf = ToHand(0, YGO_CARD_GEMINI_ELF);
    sj = ToSet(1, YGO_CARD_SOLEMN_JUDGMENT);
    Go();
    Do(YGO_ACT_SUMMON, YGO_CARD_GEMINI_ELF);
    CHECK(d.query.type == YGO_Q_CHAIN && d.query.player == 1);
    PickObj(sj);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(elf) == YGO_LOC_GRAVE && ST.players[1].lp == 4000);
    /* the Normal Summon was used */
    CHECK(!Can(YGO_ACT_SUMMON, 0));
}

static void T_MagicJammer(void)
{
    static const uint16_t c0[] = {YGO_CARD_POT_OF_GREED, 0};
    static const uint16_t c1[] = {YGO_CARD_MAGIC_JAMMER, YGO_CARD_KURIBOH, 0};
    unsigned pot, mj;
    Start("Magic Jammer", c0, c1, NULL, NULL);
    pot = ToHand(0, YGO_CARD_POT_OF_GREED);
    mj = ToSet(1, YGO_CARD_MAGIC_JAMMER);
    ToHand(1, YGO_CARD_KURIBOH);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_POT_OF_GREED);
    CHECK(d.query.type == YGO_Q_CHAIN && d.query.player == 1);
    PickObj(mj);
    /* Kuriboh is the one card to discard: asked automatically */
    Until(0, YGO_Q_IDLE);
    CHECK(Where(pot) == YGO_LOC_GRAVE && ST.players[0].handCount == 0 && ST.players[1].handCount == 0);
}

static void T_MonsterReborn(void)
{
    static const uint16_t c0[] = {YGO_CARD_MONSTER_REBORN, 0};
    static const uint16_t c1[] = {YGO_CARD_BLUE_EYES_WHITE_DRAGON, 0};
    unsigned bewd;
    Start("Monster Reborn", c0, c1, NULL, NULL);
    ToHand(0, YGO_CARD_MONSTER_REBORN);
    bewd = ToGrave(1, YGO_CARD_BLUE_EYES_WHITE_DRAGON);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_MONSTER_REBORN);
    Until(0, YGO_Q_POSITION);
    PickValue(0);
    CHECK(Where(bewd) == YGO_LOC_MZONE && ST.objs[bewd].controller == 0);
}

static void T_ChangeOfHeart(void)
{
    static const uint16_t c0[] = {YGO_CARD_CHANGE_OF_HEART, 0};
    static const uint16_t c1[] = {YGO_CARD_SUMMONED_SKULL, 0};
    unsigned skull;
    Start("Change of Heart", c0, c1, NULL, NULL);
    ToHand(0, YGO_CARD_CHANGE_OF_HEART);
    skull = ToField(1, YGO_CARD_SUMMONED_SKULL, 0);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_CHANGE_OF_HEART);
    Until(0, YGO_Q_IDLE);
    CHECK(ST.objs[skull].controller == 0);
    Do(YGO_ACT_END_TURN, 0);
    Until(1, YGO_Q_IDLE);
    CHECK(ST.objs[skull].controller == 1 && Where(skull) == YGO_LOC_MZONE);
}

static void T_Swords(void)
{
    static const uint16_t c0[] = {YGO_CARD_SWORDS_OF_REVEALING_LIGHT, 0};
    static const uint16_t c1[] = {YGO_CARD_SUMMONED_SKULL, 0};
    unsigned sw, turn;
    Start("Swords of Revealing Light", c0, c1, NULL, NULL);
    sw = ToHand(0, YGO_CARD_SWORDS_OF_REVEALING_LIGHT);
    ToField(1, YGO_CARD_SUMMONED_SKULL, 2);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_SWORDS_OF_REVEALING_LIGHT);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(sw) == YGO_LOC_SZONE);
    for (turn = 0; turn < 3; turn++)
    {
        Do(YGO_ACT_END_TURN, 0);
        Until(1, YGO_Q_IDLE);
        /* the opponent can't attack: no Battle Phase attacks offered */
        if (Can(YGO_ACT_TO_BATTLE, 0))
        {
            Do(YGO_ACT_TO_BATTLE, 0);
            Until(1, YGO_Q_BATTLE);
            CHECK(!Can(YGO_ACT_ATTACK, 0));
            Do(YGO_ACT_TO_MAIN2, 0);
            Until(1, YGO_Q_IDLE);
        }
        Do(YGO_ACT_END_TURN, 0);
        Until(0, YGO_Q_IDLE);
        CHECK(Where(sw) == (turn < 2 ? YGO_LOC_SZONE : YGO_LOC_GRAVE));
    }
}

static void T_Sangan(void)
{
    static const uint16_t c0[] = {YGO_CARD_SANGAN, YGO_CARD_DARK_HOLE, YGO_CARD_KURIBOH, 0};
    unsigned kuri;
    Start("Sangan", c0, NULL, NULL, NULL);
    ToField(0, YGO_CARD_SANGAN, 0);
    ToHand(0, YGO_CARD_DARK_HOLE);
    kuri = Find(0, YGO_CARD_KURIBOH);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_DARK_HOLE);
    Until(0, YGO_Q_CARDS);
    CHECK(d.query.prompt == YGO_P_ADD_TO_HAND);
    PickObj(kuri);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(kuri) == YGO_LOC_HAND);
}

static void T_GateGuardian(void)
{
    static const uint16_t c0[] = {YGO_CARD_GATE_GUARDIAN, YGO_CARD_SANGA_OF_THE_THUNDER, YGO_CARD_KAZEJIN,
                                  YGO_CARD_SUIJIN, 0};
    unsigned gg;
    Start("Gate Guardian", c0, NULL, NULL, NULL);
    gg = ToHand(0, YGO_CARD_GATE_GUARDIAN);
    ToField(0, YGO_CARD_SANGA_OF_THE_THUNDER, 0);
    ToField(0, YGO_CARD_KAZEJIN, 0);
    ToField(0, YGO_CARD_SUIJIN, 0);
    Go();
    CHECK(!Can(YGO_ACT_SUMMON, YGO_CARD_GATE_GUARDIAN));
    Do(YGO_ACT_SPECIAL, YGO_CARD_GATE_GUARDIAN);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(gg) == YGO_LOC_MZONE && Ygo_MonsterCount(&ST, 0) == 1 && YgoDuel_Atk(&ST, gg) == 3750);
}

static void T_Toon(void)
{
    static const uint16_t c0[] = {YGO_CARD_TOON_WORLD, YGO_CARD_TOON_MERMAID, 0};
    unsigned tm;
    Start("Toons", c0, NULL, NULL, NULL);
    ToHand(0, YGO_CARD_TOON_WORLD);
    tm = ToHand(0, YGO_CARD_TOON_MERMAID);
    Go();
    CHECK(!Can(YGO_ACT_SPECIAL, YGO_CARD_TOON_MERMAID) && !Can(YGO_ACT_SUMMON, YGO_CARD_TOON_MERMAID));
    Do(YGO_ACT_ACTIVATE, YGO_CARD_TOON_WORLD);
    Until(0, YGO_Q_IDLE);
    CHECK(ST.players[0].lp == 7000);
    Do(YGO_ACT_SPECIAL, YGO_CARD_TOON_MERMAID);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(tm) == YGO_LOC_MZONE);
    Do(YGO_ACT_TO_BATTLE, 0);
    Until(0, YGO_Q_BATTLE);
    CHECK(!Can(YGO_ACT_ATTACK, YGO_CARD_TOON_MERMAID));
}

static void T_Kuriboh(void)
{
    static const uint16_t c0[] = {YGO_CARD_SUMMONED_SKULL, 0};
    static const uint16_t c1[] = {YGO_CARD_KURIBOH, 0};
    unsigned kuri;
    Start("Kuriboh", c0, c1, NULL, NULL);
    ToField(0, YGO_CARD_SUMMONED_SKULL, 0);
    kuri = ToHand(1, YGO_CARD_KURIBOH);
    Go();
    Do(YGO_ACT_TO_BATTLE, 0);
    Until(0, YGO_Q_BATTLE);
    Do(YGO_ACT_ATTACK, YGO_CARD_SUMMONED_SKULL);
    Until(1, YGO_Q_CHAIN);
    PickObj(kuri);
    Until(0, YGO_Q_BATTLE);
    CHECK(ST.players[1].lp == 8000 && Where(kuri) == YGO_LOC_GRAVE);
}

static void T_ManEaterBug(void)
{
    static const uint16_t c0[] = {YGO_CARD_SUMMONED_SKULL, 0};
    static const uint16_t c1[] = {YGO_CARD_MAN_EATER_BUG, 0};
    unsigned skull, bug;
    Start("Man-Eater Bug", c0, c1, NULL, NULL);
    skull = ToField(0, YGO_CARD_SUMMONED_SKULL, 0);
    bug = ToField(1, YGO_CARD_MAN_EATER_BUG, 2);
    Go();
    Do(YGO_ACT_TO_BATTLE, 0);
    Until(0, YGO_Q_BATTLE);
    Do(YGO_ACT_ATTACK, YGO_CARD_SUMMONED_SKULL);
    Until(1, YGO_Q_CARDS);
    PickObj(skull);
    Until(0, YGO_Q_BATTLE);
    CHECK(Where(skull) == YGO_LOC_GRAVE && Where(bug) == YGO_LOC_GRAVE);
}

static void T_Relinquished(void)
{
    static const uint16_t c0[] = {YGO_CARD_BLACK_ILLUSION_RITUAL, YGO_CARD_RELINQUISHED, YGO_CARD_KURIBOH, 0};
    static const uint16_t c1[] = {YGO_CARD_BLUE_EYES_WHITE_DRAGON, 0};
    unsigned rel, bewd;
    Start("Relinquished", c0, c1, NULL, NULL);
    ToHand(0, YGO_CARD_BLACK_ILLUSION_RITUAL);
    rel = ToHand(0, YGO_CARD_RELINQUISHED);
    ToHand(0, YGO_CARD_KURIBOH);
    bewd = ToField(1, YGO_CARD_BLUE_EYES_WHITE_DRAGON, 0);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_BLACK_ILLUSION_RITUAL);
    Until(0, YGO_Q_POSITION);
    PickValue(0);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(rel) == YGO_LOC_MZONE && ST.objs[rel].summonType == YGO_SUMMON_RITUAL);
    Do(YGO_ACT_ACTIVATE, YGO_CARD_RELINQUISHED);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(bewd) == YGO_LOC_SZONE && YgoDuel_Atk(&ST, rel) == 3000 && YgoDuel_Def(&ST, rel) == 2500);
}

static void T_Moth(void)
{
    static const uint16_t c0[] = {YGO_CARD_PETIT_MOTH, YGO_CARD_COCOON_OF_EVOLUTION, YGO_CARD_LARVAE_MOTH, 0};
    unsigned moth, cocoon, larvae;
    Start("Larvae Moth", c0, NULL, NULL, NULL);
    moth = ToField(0, YGO_CARD_PETIT_MOTH, 0);
    cocoon = ToHand(0, YGO_CARD_COCOON_OF_EVOLUTION);
    larvae = ToHand(0, YGO_CARD_LARVAE_MOTH);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_COCOON_OF_EVOLUTION);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(cocoon) == YGO_LOC_SZONE && YgoDuel_Def(&ST, moth) == 2000 && YgoDuel_Atk(&ST, moth) == 0);
    CHECK(!Can(YGO_ACT_SPECIAL, YGO_CARD_LARVAE_MOTH));
    /* two of your turns later */
    Do(YGO_ACT_END_TURN, 0);
    Until(0, YGO_Q_IDLE);
    Do(YGO_ACT_END_TURN, 0);
    Until(0, YGO_Q_IDLE);
    CHECK(ST.objs[cocoon].counters == 2);
    Do(YGO_ACT_SPECIAL, YGO_CARD_LARVAE_MOTH);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(larvae) == YGO_LOC_MZONE && Where(moth) == YGO_LOC_GRAVE && Where(cocoon) == YGO_LOC_GRAVE);
}

static void T_Waboku(void)
{
    static const uint16_t c0[] = {YGO_CARD_SUMMONED_SKULL, 0};
    static const uint16_t c1[] = {YGO_CARD_WABOKU, YGO_CARD_KURIBOH, 0};
    unsigned kuri, wab;
    Start("Waboku", c0, c1, NULL, NULL);
    ToField(0, YGO_CARD_SUMMONED_SKULL, 0);
    kuri = ToField(1, YGO_CARD_KURIBOH, 0);
    wab = ToSet(1, YGO_CARD_WABOKU);
    Go();
    Do(YGO_ACT_TO_BATTLE, 0);
    Until(0, YGO_Q_BATTLE);
    /* Kuriboh is the only target: not asked */
    Do(YGO_ACT_ATTACK, YGO_CARD_SUMMONED_SKULL);
    Until(1, YGO_Q_CHAIN);
    PickObj(wab);
    Until(0, YGO_Q_BATTLE);
    CHECK(ST.players[1].lp == 8000 && Where(kuri) == YGO_LOC_MZONE);
}

static void T_Scapegoat(void)
{
    static const uint16_t c0[] = {YGO_CARD_SCAPEGOAT, YGO_CARD_KURIBOH, 0};
    Start("Scapegoat", c0, NULL, NULL, NULL);
    ToHand(0, YGO_CARD_SCAPEGOAT);
    ToHand(0, YGO_CARD_KURIBOH);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_SCAPEGOAT);
    Until(0, YGO_Q_IDLE);
    CHECK(Ygo_MonsterCount(&ST, 0) == 4);
    CHECK(!Can(YGO_ACT_SUMMON, YGO_CARD_KURIBOH) && Can(YGO_ACT_SET_MONSTER, YGO_CARD_KURIBOH));
    CHECK(YgoDuel_Conserved(&d));
}

static void T_Exodia(void)
{
    static const uint16_t c0[] = {YGO_CARD_EXODIA_THE_FORBIDDEN_ONE, YGO_CARD_LEFT_ARM_OF_THE_FORBIDDEN_ONE,
                                  YGO_CARD_RIGHT_ARM_OF_THE_FORBIDDEN_ONE, YGO_CARD_LEFT_LEG_OF_THE_FORBIDDEN_ONE,
                                  YGO_CARD_RIGHT_LEG_OF_THE_FORBIDDEN_ONE, YGO_CARD_POT_OF_GREED, 0};
    Start("Exodia", c0, NULL, NULL, NULL);
    ToHand(0, YGO_CARD_EXODIA_THE_FORBIDDEN_ONE);
    ToHand(0, YGO_CARD_LEFT_ARM_OF_THE_FORBIDDEN_ONE);
    ToHand(0, YGO_CARD_RIGHT_ARM_OF_THE_FORBIDDEN_ONE);
    ToHand(0, YGO_CARD_LEFT_LEG_OF_THE_FORBIDDEN_ONE);
    /* the last piece on top of the Deck, Pot of Greed to draw it */
    {
        unsigned leg = Find(0, YGO_CARD_RIGHT_LEG_OF_THE_FORBIDDEN_ONE);
        Detach(leg);
        ST.players[0].deck[ST.players[0].deckCount++] = leg;
    }
    ToHand(0, YGO_CARD_POT_OF_GREED);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_POT_OF_GREED);
    CHECK(ST.over && ST.winner == 0 && ST.winReason == YGO_WIN_EXODIA);
}

static void T_Segoc(void)
{
    /* Sangan and Witch of the Black Forest both go: two triggers, the player orders them */
    static const uint16_t c0[] = {YGO_CARD_SANGAN, YGO_CARD_WITCH_OF_THE_BLACK_FOREST, YGO_CARD_DARK_HOLE,
                                  YGO_CARD_KURIBOH, YGO_CARD_MAN_EATER_BUG, 0};
    unsigned kuri, bug;
    Start("Simultaneous triggers", c0, NULL, NULL, NULL);
    ToField(0, YGO_CARD_SANGAN, 0);
    ToField(0, YGO_CARD_WITCH_OF_THE_BLACK_FOREST, 0);
    ToHand(0, YGO_CARD_DARK_HOLE);
    kuri = Find(0, YGO_CARD_KURIBOH);
    bug = Find(0, YGO_CARD_MAN_EATER_BUG);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_DARK_HOLE);
    Until(0, YGO_Q_CARDS);
    CHECK(d.query.prompt == YGO_P_ORDER);
    PickCard(YGO_CARD_SANGAN);
    /* chain: Witch resolves first (it's on top) */
    Until(0, YGO_Q_CARDS);
    PickObj(bug);
    Until(0, YGO_Q_CARDS);
    PickObj(kuri);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(kuri) == YGO_LOC_HAND && Where(bug) == YGO_LOC_HAND);
}

static void T_EquipField(void)
{
    static const uint16_t c0[] = {YGO_CARD_LEGENDARY_SWORD, YGO_CARD_CELTIC_GUARDIAN, YGO_CARD_SOGEN, 0};
    unsigned cg;
    Start("Equip and Field Spells", c0, NULL, NULL, NULL);
    ToHand(0, YGO_CARD_LEGENDARY_SWORD);
    ToHand(0, YGO_CARD_SOGEN);
    cg = ToField(0, YGO_CARD_CELTIC_GUARDIAN, 0);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_LEGENDARY_SWORD);
    Until(0, YGO_Q_IDLE);
    CHECK(YgoDuel_Atk(&ST, cg) == 1700 && YgoDuel_Def(&ST, cg) == 1500);
    Do(YGO_ACT_ACTIVATE, YGO_CARD_SOGEN);
    Until(0, YGO_Q_IDLE);
    CHECK(YgoDuel_Atk(&ST, cg) == 1900 && YgoDuel_Def(&ST, cg) == 1700);
}

static void T_Fissure(void)
{
    static const uint16_t c0[] = {YGO_CARD_FISSURE, 0};
    static const uint16_t c1[] = {YGO_CARD_SUMMONED_SKULL, YGO_CARD_KURIBOH, 0};
    unsigned skull, kuri;
    Start("Fissure", c0, c1, NULL, NULL);
    ToHand(0, YGO_CARD_FISSURE);
    skull = ToField(1, YGO_CARD_SUMMONED_SKULL, 0);
    kuri = ToField(1, YGO_CARD_KURIBOH, 0);
    Go();
    Do(YGO_ACT_ACTIVATE, YGO_CARD_FISSURE);
    Until(0, YGO_Q_IDLE);
    CHECK(Where(kuri) == YGO_LOC_GRAVE && Where(skull) == YGO_LOC_MZONE);
}

static void T_Battle(void)
{
    /* attack a face-down DEF monster: no damage, flipped; a stronger DEF hurts the attacker */
    static const uint16_t c0[] = {YGO_CARD_GEMINI_ELF, YGO_CARD_KURIBOH, 0};
    static const uint16_t c1[] = {YGO_CARD_MYSTICAL_ELF, 0};
    unsigned elf, kuri, me;
    Start("Battle", c0, c1, NULL, NULL);
    elf = ToField(0, YGO_CARD_GEMINI_ELF, 0);
    kuri = ToField(0, YGO_CARD_KURIBOH, 0);
    me = ToField(1, YGO_CARD_MYSTICAL_ELF, 2);
    Go();
    Do(YGO_ACT_TO_BATTLE, 0);
    Until(0, YGO_Q_BATTLE);
    Do(YGO_ACT_ATTACK, YGO_CARD_KURIBOH);
    Until(0, YGO_Q_BATTLE);
    /* Kuriboh 300 vs Mystical Elf DEF 2000: 1700 to player 0, both stay */
    CHECK(ST.players[0].lp == 8000 - 1700 && ST.objs[me].faceUp && Where(me) == YGO_LOC_MZONE);
    Do(YGO_ACT_ATTACK, YGO_CARD_GEMINI_ELF);
    Until(0, YGO_Q_BATTLE);
    /* Gemini Elf 1900 vs DEF 2000: 100 more */
    CHECK(ST.players[0].lp == 8000 - 1800);
    (void)elf;
    (void)kuri;
}

int main(void)
{
    static void (*const tests[])(void) = {T_Polymerization, T_MirrorForce, T_TrapHole, T_Solemn, T_MagicJammer,
                                          T_MonsterReborn, T_ChangeOfHeart, T_Swords, T_Sangan, T_GateGuardian,
                                          T_Toon, T_Kuriboh, T_ManEaterBug, T_Relinquished, T_Moth, T_Waboku,
                                          T_Scapegoat, T_Exodia, T_Segoc, T_EquipField, T_Fissure, T_Battle};
    unsigned i;
    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
    {
        tests[i]();
        CHECK(YgoDuel_Conserved(&d));
    }
    printf("PASS %u card scenarios: Fusion and Ritual Summons, chains, summon windows, the Damage Step, SEGOC\n",
           (unsigned)(sizeof(tests) / sizeof(tests[0])));
    return 0;
}
