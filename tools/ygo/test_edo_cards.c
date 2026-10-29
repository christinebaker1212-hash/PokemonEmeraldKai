/* Yu-Gi-Oh! card scenarios on EDOPro's engine (tools/ygo/test.sh): known hands (the Decks unshuffled), a few turns
 * played by name, and the table checked after: Summons, Tributes, an attack, Dark Hole, Monster Reborn, a Fusion, a
 * Ritual (a sum of Levels), Mirror Force in a chain, Change of Heart, Scapegoat's tokens, a coin toss, a declared card
 * name. Each runs through the game's own questions (YgoQuery) and answers, so the bridge is what's tested. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ygo/ygo_internal.h"
#include "../../src/ygo/ygo_edo.h"

extern int gYgoEdoNoShuffle;
extern unsigned gYgoEdoRebuilds;
static const char *sScenario = "";
#define CHECK(x)                                                                          \
    do                                                                                    \
    {                                                                                     \
        if (!(x))                                                                         \
        {                                                                                 \
            fprintf(stderr, "FAIL %s:%d [%s]: %s\n", __FILE__, __LINE__, sScenario, #x); \
            exit(1);                                                                      \
        }                                                                                 \
    } while (0)

static struct YgoDuel D;

static unsigned Id(const char *name)
{
    unsigned i;
    for (i = 1; i <= YGO_CARD_COUNT; i++)
        if (!strcmp(Ygo_Card(i)->name, name))
            return i;
    fprintf(stderr, "no card %s\n", name);
    exit(1);
}
/* A Deck: filler, then the cards named (the last named is drawn first) */
static void Deck(uint16_t *out, const char *const *top, const char *filler)
{
    unsigned n = 0, k = 0, i;
    while (top[k])
        k++;
    memset(out, 0, sizeof(uint16_t) * YGO_DECK_MAX);
    while (n < 40 - k)
        out[n++] = (uint16_t)Id(filler);
    for (i = k; i-- > 0;)
        out[n++] = (uint16_t)Id(top[i]);
}
static void Start(const char *const *hand0, const char *const *hand1, const char *fusion0)
{
    uint16_t m0[YGO_DECK_MAX], m1[YGO_DECK_MAX], x0[YGO_FUSION_MAX], x1[YGO_FUSION_MAX];
    memset(x0, 0, sizeof(x0));
    memset(x1, 0, sizeof(x1));
    if (fusion0)
        x0[0] = (uint16_t)Id(fusion0);
    Deck(m0, hand0, "Giant Soldier of Stone");
    Deck(m1, hand1, "Mystical Elf");
    gYgoEdoNoShuffle = 1;
    CHECK(YgoDuel_Init(&D, m0, x0, m1, x1, 42, 0));
    CHECK(YgoDuel_Conserved(&D));
}
static void Answer(const struct YgoAnswer *a)
{
    if (!YgoDuel_Answer(&D, a))
    {
        fprintf(stderr, "[%s] refused: %s (question %u prompt %u)\n", sScenario, D.error, D.query.type, D.query.prompt);
        CHECK(0);
    }
    CHECK(YgoDuel_Conserved(&D));
}
static void Pick(unsigned item)
{
    struct YgoAnswer a;
    memset(&a, 0, sizeof(a));
    a.count = 1;
    a.picks[0] = (uint8_t)item;
    Answer(&a);
}
static void Pass(void)
{
    struct YgoAnswer a;
    memset(&a, 0, sizeof(a));
    Answer(&a);
}
static const char *ItemName(unsigned i) { return Ygo_Card(D.query.items[i].card) ? Ygo_Card(D.query.items[i].card)->name : ""; }
/* Answers what's asked with nothing much (pass, no, the first choices) until player gets a question of type */
static void Until(unsigned player, unsigned type)
{
    unsigned guard = 0;
    while (!(D.query.player == player && D.query.type == type))
    {
        struct YgoAnswer a;
        if (getenv("TRACE"))
            printf("  until: q%u p%u prompt %u count %u min %u max %u first %s\n", D.query.type, D.query.player,
                   D.query.prompt, D.query.count, D.query.min, D.query.max, D.query.count ? ItemName(0) : "-");
        CHECK(D.query.type != YGO_Q_NONE && guard++ < 200);
        if (D.query.type == YGO_Q_CHAIN && !D.query.min)
            Pass();
        else if (D.query.type == YGO_Q_IDLE || D.query.type == YGO_Q_BATTLE)
        {
            /* the turn moves on */
            unsigned i, pick = D.query.count - 1;
            for (i = 0; i < D.query.count; i++)
                if (D.query.items[i].kind == YGO_ACT_END_TURN)
                    pick = i;
            Pick(pick);
        }
        else
        {
            YgoDuel_AiAnswer(&D, &a);
            Answer(&a);
        }
    }
}
/* The Main Phase (or Battle Phase) action of kind on the card named */
static void Act(unsigned kind, const char *name)
{
    unsigned i;
    for (i = 0; i < D.query.count; i++)
        if (D.query.items[i].kind == kind && (!name || !strcmp(ItemName(i), name)))
        {
            Pick(i);
            return;
        }
    fprintf(stderr, "[%s] no action %u on %s; offered:", sScenario, kind, name ? name : "-");
    for (i = 0; i < D.query.count; i++)
        fprintf(stderr, " [%u %s]", D.query.items[i].kind, ItemName(i));
    fprintf(stderr, "\n");
    CHECK(0);
}
/* Pick the cards named in the question now (CARDS / SUM) */
static void Choose(const char *const *names)
{
    struct YgoAnswer a;
    unsigned k, i;
    memset(&a, 0, sizeof(a));
    for (k = 0; names[k]; k++)
    {
        for (i = 0; i < D.query.count; i++)
        {
            unsigned j, taken = 0;
            for (j = 0; j < a.count; j++)
                taken |= a.picks[j] == i;
            if (!taken && !strcmp(ItemName(i), names[k]))
                break;
        }
        if (i == D.query.count)
        {
            fprintf(stderr, "[%s] no %s to pick (question %u)\n", sScenario, names[k], D.query.type);
            CHECK(0);
        }
        a.picks[a.count++] = (uint8_t)i;
    }
    Answer(&a);
}
static unsigned Monsters(unsigned p)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_ZONES; i++)
        n += D.s.players[p].monsters[i] != 0;
    return n;
}
static int HasMonster(unsigned p, const char *name)
{
    unsigned i;
    for (i = 0; i < YGO_ZONES; i++)
        if (D.s.players[p].monsters[i] && !strcmp(Ygo_Card(D.s.objs[D.s.players[p].monsters[i] - 1].card)->name, name))
            return 1;
    return 0;
}
static int InGrave(unsigned p, const char *name)
{
    unsigned i;
    for (i = 0; i < D.s.players[p].graveCount; i++)
        if (!strcmp(Ygo_Card(D.s.objs[D.s.players[p].grave[i]].card)->name, name))
            return 1;
    return 0;
}
static int SawEvent(unsigned type)
{
    uint32_t seq;
    for (seq = D.s.eventSeq; seq > 0 && D.s.eventSeq - seq < YGO_EVENT_RING; seq--)
        if (YgoDuel_GetEvent(&D.s, seq - 1)->type == type)
            return 1;
    return 0;
}
#define LIST(...) ((const char *const[]){__VA_ARGS__, NULL})

int main(void)
{
    /* A Normal Summon on turn 1, a direct attack on turn 3 */
    sScenario = "summon and attack";
    Start(LIST("Celtic Guardian", "Pot of Greed"), LIST("Waboku"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Celtic Guardian");
    Until(0, YGO_Q_IDLE);
    CHECK(HasMonster(0, "Celtic Guardian") && D.s.turn == 1);
    Act(YGO_ACT_END_TURN, NULL);
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(0, YGO_Q_IDLE);
    CHECK(D.s.turn == 3);
    Act(YGO_ACT_TO_BATTLE, NULL);
    Until(0, YGO_Q_BATTLE);
    Act(YGO_ACT_ATTACK, "Celtic Guardian");
    Until(0, YGO_Q_BATTLE);
    CHECK(D.s.players[1].lp == 8000 - 1400);
    printf("ok   %s\n", sScenario);

    /* A Tribute Summon: Summoned Skull for a monster */
    sScenario = "tribute summon";
    Start(LIST("Celtic Guardian", "Summoned Skull"), LIST("Waboku"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Celtic Guardian");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Summoned Skull");
    CHECK(D.query.type == YGO_Q_CARDS && D.query.prompt == YGO_P_TRIBUTE);
    Choose(LIST("Celtic Guardian"));
    Until(0, YGO_Q_IDLE);
    CHECK(HasMonster(0, "Summoned Skull") && !HasMonster(0, "Celtic Guardian") && InGrave(0, "Celtic Guardian"));
    printf("ok   %s\n", sScenario);

    /* Dark Hole: every monster destroyed */
    sScenario = "dark hole";
    Start(LIST("Celtic Guardian", "Dark Hole"), LIST("Feral Imp"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Celtic Guardian");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Feral Imp");
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(0, YGO_Q_IDLE);
    CHECK(Monsters(0) == 1 && Monsters(1) == 1);
    Act(YGO_ACT_ACTIVATE, "Dark Hole");
    Until(0, YGO_Q_IDLE);
    CHECK(Monsters(0) == 0 && Monsters(1) == 0 && InGrave(1, "Feral Imp") && InGrave(0, "Dark Hole"));
    CHECK(SawEvent(YGO_EV_DESTROY));
    printf("ok   %s\n", sScenario);

    /* Monster Reborn: the opponent's monster back, from their Graveyard */
    sScenario = "monster reborn";
    Start(LIST("Celtic Guardian", "Dark Hole", "Monster Reborn"), LIST("Feral Imp"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Feral Imp");
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Dark Hole");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Monster Reborn");
    CHECK(D.query.type == YGO_Q_CARDS);
    Choose(LIST("Feral Imp"));
    Until(0, YGO_Q_IDLE);
    CHECK(HasMonster(0, "Feral Imp") && !InGrave(1, "Feral Imp"));
    printf("ok   %s\n", sScenario);

    /* Polymerization: Flame Swordsman from Flame Manipulator + Masaki */
    sScenario = "fusion";
    Start(LIST("Polymerization", "Flame Manipulator", "Masaki the Legendary Swordsman"), LIST("Waboku"),
          "Flame Swordsman");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Polymerization");
    {
        unsigned guard = 0;
        /* the Fusion Monster, then the materials (one at a time or together) */
        while (D.query.player == 0 && D.query.type != YGO_Q_IDLE && guard++ < 10)
        {
            struct YgoAnswer a;
            YgoDuel_AiAnswer(&D, &a);
            if (getenv("TRACE"))
                printf("  fusion: q%u prompt %u count %u min %u max %u first %s -> %u picks\n", D.query.type,
                       D.query.prompt, D.query.count, D.query.min, D.query.max, D.query.count ? ItemName(0) : "-", a.count);
            Answer(&a);
        }
    }
    Until(0, YGO_Q_IDLE);
    CHECK(HasMonster(0, "Flame Swordsman"));
    CHECK(InGrave(0, "Flame Manipulator") && InGrave(0, "Masaki the Legendary Swordsman"));
    {
        unsigned i, o = 0;
        for (i = 0; i < YGO_ZONES; i++)
            if (D.s.players[0].monsters[i])
                o = D.s.players[0].monsters[i] - 1;
        CHECK(D.s.objs[o].summonType == YGO_SUMMON_FUSION);
    }
    printf("ok   %s\n", sScenario);

    /* A Ritual Summon: Black Luster Soldier for Levels adding up to 8 */
    sScenario = "ritual";
    Start(LIST("Black Luster Ritual", "Black Luster Soldier", "Summoned Skull", "Celtic Guardian", "Feral Imp"),
          LIST("Waboku"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Black Luster Ritual");
    {
        unsigned guard = 0, sums = 0;
        while (D.query.player == 0 && D.query.type != YGO_Q_IDLE && guard++ < 10)
        {
            struct YgoAnswer a;
            sums += D.query.type == YGO_Q_SUM;
            YgoDuel_AiAnswer(&D, &a);
            Answer(&a);
        }
        CHECK(sums >= 1);
    }
    Until(0, YGO_Q_IDLE);
    CHECK(HasMonster(0, "Black Luster Soldier"));
    printf("ok   %s\n", sScenario);

    /* Mirror Force: chained to an attack, the attackers destroyed */
    sScenario = "mirror force";
    Start(LIST("Celtic Guardian", "Feral Imp"), LIST("Mirror Force"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Celtic Guardian");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_SET_SPELL, "Mirror Force");
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_TO_BATTLE, NULL);
    Until(0, YGO_Q_BATTLE);
    Act(YGO_ACT_ATTACK, "Celtic Guardian");
    {
        unsigned guard = 0, chained = 0;
        while (!(D.query.player == 0 && (D.query.type == YGO_Q_BATTLE || D.query.type == YGO_Q_IDLE)) && guard++ < 20)
        {
            if (D.query.player == 1 && D.query.type == YGO_Q_CHAIN)
            {
                unsigned i;
                for (i = 0; i < D.query.count && strcmp(ItemName(i), "Mirror Force"); i++)
                    ;
                if (i < D.query.count)
                {
                    Pick(i);
                    chained = 1;
                    continue;
                }
                Pass();
            }
            else if (D.query.type == YGO_Q_CHAIN)
                Pass();
            else
            {
                struct YgoAnswer a;
                YgoDuel_AiAnswer(&D, &a);
                Answer(&a);
            }
        }
        CHECK(chained);
    }
    CHECK(Monsters(0) == 0 && InGrave(0, "Celtic Guardian") && InGrave(1, "Mirror Force") && D.s.players[1].lp == 8000);
    printf("ok   %s\n", sScenario);

    /* Change of Heart: their monster on our side */
    sScenario = "change of heart";
    Start(LIST("Change of Heart"), LIST("Feral Imp"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Feral Imp");
    Until(1, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Change of Heart");
    if (D.query.type == YGO_Q_CARDS)
        Choose(LIST("Feral Imp"));
    Until(0, YGO_Q_IDLE);
    CHECK(HasMonster(0, "Feral Imp") && !HasMonster(1, "Feral Imp"));
    printf("ok   %s\n", sScenario);

    /* Scapegoat: four tokens (cards outside the catalog, from EDOPro's database) */
    sScenario = "scapegoat";
    Start(LIST("Scapegoat"), LIST("Waboku"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SET_SPELL, "Scapegoat");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_END_TURN, NULL);
    {
        unsigned guard = 0, done = 0;
        while (!done && guard++ < 40)
        {
            CHECK(D.query.type != YGO_Q_NONE);
            if (D.query.player == 0 && D.query.type == YGO_Q_CHAIN)
            {
                unsigned i;
                for (i = 0; i < D.query.count && strcmp(ItemName(i), "Scapegoat"); i++)
                    ;
                if (i < D.query.count)
                {
                    Pick(i);
                    done = 1;
                    continue;
                }
                Pass();
            }
            else if (D.query.type == YGO_Q_CHAIN)
                Pass();
            else if (D.query.type == YGO_Q_IDLE)
                Act(YGO_ACT_END_TURN, NULL);
            else
            {
                struct YgoAnswer a;
                YgoDuel_AiAnswer(&D, &a);
                Answer(&a);
            }
        }
        CHECK(done);
    }
    {
        unsigned guard = 0;
        while (Monsters(0) < 4 && guard++ < 20)
        {
            struct YgoAnswer a;
            if (D.query.type == YGO_Q_CHAIN)
                Pass();
            else
            {
                YgoDuel_AiAnswer(&D, &a);
                Answer(&a);
            }
        }
    }
    CHECK(Monsters(0) == 4);
    CHECK(Ygo_Card(D.s.objs[D.s.players[0].monsters[0] - 1].card)->frame == YGO_TOKEN);
    CHECK(D.s.objs[D.s.players[0].monsters[0] - 1].card > YGO_CARD_COUNT ||
          !strcmp(Ygo_Card(D.s.objs[D.s.players[0].monsters[0] - 1].card)->name, "Sheep Token"));
    printf("ok   %s\n", sScenario);

    /* Time Wizard: a coin call and the toss */
    sScenario = "coin";
    Start(LIST("Time Wizard"), LIST("Waboku"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_SUMMON, "Time Wizard");
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Time Wizard");
    {
        unsigned guard = 0, called = 0;
        while (D.query.type != YGO_Q_IDLE && guard++ < 10)
        {
            struct YgoAnswer a;
            called |= D.query.prompt == YGO_P_COIN_CALL;
            YgoDuel_AiAnswer(&D, &a);
            Answer(&a);
        }
        CHECK(called);
    }
    CHECK(SawEvent(YGO_EV_COIN));
    printf("ok   %s\n", sScenario);

    /* Prohibition: a card name declared */
    sScenario = "declare";
    Start(LIST("Prohibition"), LIST("Waboku"), NULL);
    Until(0, YGO_Q_IDLE);
    Act(YGO_ACT_ACTIVATE, "Prohibition");
    {
        unsigned guard = 0, named = 0;
        while (D.query.type != YGO_Q_IDLE && guard++ < 10)
        {
            struct YgoAnswer a;
            if (D.query.type == YGO_Q_NAME)
            {
                unsigned k;
                named = 1;
                CHECK(D.nameCount > 100);
                for (k = 0; k < D.nameCount && strcmp(Ygo_Card(D.names[k])->name, "Waboku"); k++)
                    ;
                CHECK(k < D.nameCount);
                memset(&a, 0, sizeof(a));
                a.count = 2;
                a.picks[0] = (uint8_t)k;
                a.picks[1] = (uint8_t)(k >> 8);
            }
            else
                YgoDuel_AiAnswer(&D, &a);
            Answer(&a);
        }
        CHECK(named && SawEvent(YGO_EV_DECLARE));
    }
    printf("ok   %s\n", sScenario);

    YgoDuel_Free(&D);
    CHECK(gYgoEdoRebuilds == 0);
    puts("ygo card scenarios passed");
    return 0;
}
