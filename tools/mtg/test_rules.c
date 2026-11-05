// Moemon Kai: the Magic engine's tests (tools/mtg/test.sh): the real pack (graphics/mtg/forge.pak), set-up tables
// for the rules each translated part covers, and whole duels between the stand-in AIs (they end, and the same seed
// plays the same duel, also from a copy made with memcpy).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtg_internal.h"
#include "mtg/collection.h"

static int sFails, sChecks;
#define CHECK(cond, ...) do { sChecks++; if (!(cond)) { sFails++; printf("  FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static void *ReadFile(const char *path, uint32_t *size)
{
    FILE *f = fopen(path, "rb");
    void *p;
    long n;
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    p = malloc((size_t)n);
    if (fread(p, 1, (size_t)n, f) != (size_t)n)
        n = 0;
    fclose(f);
    *size = (uint32_t)n;
    return p;
}

static int Card(const char *name)
{
    int id = Mtg_CardByName(name);
    if (id < 0) {
        printf("  no card %s\n", name);
        exit(1);
    }
    return id;
}

// ---- set-up tables: a duel with 30 basic lands each, stopped at a step, cards put in place by hand
static MtgDuel sD;

static int Put(int player, const char *name, int zone)
{
    MtgObj *o = &sD.objs[sD.objCount];
    memset(o, 0, sizeof(*o));
    o->def = (uint16_t)Card(name);
    o->owner = o->controller = (uint8_t)player;
    o->zone = (uint8_t)zone;
    o->blocking = MTG_NO_OBJ;
    o->seq = ++sD.seq;
    return sD.objCount++;
}

static void Answer(const MtgAnswer *a)
{
    if (!Mtg_Answer(&sD, a)) {
        printf("  answer refused\n");
        exit(1);
    }
}

static void Pass(void)
{
    MtgAnswer a;
    memset(&a, 0, sizeof(a));
    Answer(&a);
}

// pass (and declare nothing) until `player` has priority in `step` with an empty stack
static void GoTo(int player, int step)
{
    int guard = 0;
    while (guard++ < 400) {
        const MtgQuery *q = Mtg_Query(&sD);
        MtgAnswer a;
        memset(&a, 0, sizeof(a));
        memset(a.block, -1, sizeof(a.block));
        if (q->kind == MTG_Q_PRIORITY && sD.step == step && q->player == player && sD.stackCount == 0)
            return;
        if (q->kind == MTG_Q_GAME_OVER)
            break;
        Answer(&a);
    }
    printf("  never reached step %d for player %d\n", step, player);
    exit(1);
}

static void Table(void)
{
    static MtgDeck lands[2];
    int i;
    lands[0].count = lands[1].count = 30;
    for (i = 0; i < 30; i++) {
        lands[0].cards[i] = (uint16_t)Card("Plains");
        lands[1].cards[i] = (uint16_t)Card("Plains");
    }
    Mtg_DuelInit(&sD, &lands[0], &lands[1], 12345);
    // the starting player is player 0 in every table: their first main phase, hands emptied
    while (sD.startingPlayer != 0) {
        static uint32_t seed = 1;
        Mtg_DuelInit(&sD, &lands[0], &lands[1], ++seed);
    }
    for (i = 0; i < sD.objCount; i++)
        if (sD.objs[i].zone == MTG_ZONE_HAND)
            sD.objs[i].zone = MTG_ZONE_EXILE;
    GoTo(0, MTG_STEP_MAIN1);
}

static int Find(int kind, int obj, int tgtKind, int tgtId)
{
    const MtgQuery *q;
    int i;
    Mtg_RefreshQuery(&sD);
    q = Mtg_Query(&sD);
    for (i = 0; i < q->actionCount; i++) {
        const MtgAction *a = &q->actions[i];
        if (a->kind == kind && a->obj == obj && (tgtKind < 0 || (a->target.kind == tgtKind && a->target.id == tgtId)))
            return i;
    }
    return -1;
}

static void Do(int kind, int obj, int tgtKind, int tgtId)
{
    MtgAnswer a;
    int i = Find(kind, obj, tgtKind, tgtId);
    if (i < 0) {
        printf("  no such action (kind %d, %s)\n", kind, Mtg_Card(sD.objs[obj].def)->name);
        sFails++;
        return;
    }
    memset(&a, 0, sizeof(a));
    a.action = (uint16_t)i;
    Answer(&a);
}

static void PassBoth(void) { Pass(); Pass(); }   // both pass: the top of the stack resolves

static int Zone(int obj) { return sD.objs[obj].zone; }

static void Lands(int player, const char *name, int n)
{
    while (n-- > 0)
        Put(player, name, MTG_ZONE_BATTLEFIELD);
}

// ---- the tables
static void TestBolt(void)
{
    int bolt, bears;
    printf("bolt\n");
    Table();
    Lands(0, "Mountain", 1);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, bears);
    CHECK(Zone(bolt) == MTG_ZONE_STACK, "bolt on the stack");
    PassBoth();
    CHECK(Zone(bears) == MTG_ZONE_GRAVEYARD, "bears died (zone %d)", Zone(bears));
    CHECK(Zone(bolt) == MTG_ZONE_GRAVEYARD, "bolt in the graveyard");

    Table();
    Lands(0, "Mountain", 1);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_PLAYER, 1);
    PassBoth();
    CHECK(sD.players[1].life == 17, "bolt to the face: %d", sD.players[1].life);
}

static void TestResponse(void)
{
    int bolt, bears, growth;
    printf("giant growth in response\n");
    Table();
    Lands(0, "Mountain", 1);
    Lands(1, "Forest", 1);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    growth = Put(1, "Giant Growth", MTG_ZONE_HAND);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, bears);
    Pass();   // player 0 passes: player 1 may answer
    CHECK(Mtg_Query(&sD)->player == 1, "player 1 has priority");
    Do(MTG_ACT_CAST, growth, MTG_TGT_OBJ, bears);
    PassBoth();   // growth resolves
    CHECK(Mtg_Toughness(&sD, bears) == 5, "bears 5 tough: %d", Mtg_Toughness(&sD, bears));
    PassBoth();   // bolt resolves
    CHECK(Zone(bears) == MTG_ZONE_BATTLEFIELD && sD.objs[bears].damage == 3, "bears live with 3 damage");
    GoTo(1, MTG_STEP_UPKEEP);
    CHECK(Mtg_Toughness(&sD, bears) == 2 && sD.objs[bears].damage == 0, "growth and damage gone next turn");
}

static void TestPlowshares(void)
{
    int swords, angel;
    printf("swords to plowshares\n");
    Table();
    Lands(0, "Plains", 1);
    swords = Put(0, "Swords to Plowshares", MTG_ZONE_HAND);
    angel = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, swords, MTG_TGT_OBJ, angel);
    PassBoth();
    CHECK(Zone(angel) == MTG_ZONE_EXILE, "angel exiled");
    CHECK(sD.players[1].life == 24, "its controller gained its power: %d", sD.players[1].life);
}

static void Attack(uint32_t mask)
{
    MtgAnswer a;
    GoTo(sD.active, MTG_STEP_COMBAT_BEGIN);
    Pass();
    Pass();
    CHECK(Mtg_Query(&sD)->kind == MTG_Q_ATTACKERS, "asked for attackers");
    memset(&a, 0, sizeof(a));
    a.attack = mask;
    Answer(&a);
}

static void Block(int blocker, int attacker)
{
    const MtgQuery *q = Mtg_Query(&sD);
    MtgAnswer a;
    int i, j;
    while (q->kind == MTG_Q_PRIORITY && sD.step == MTG_STEP_COMBAT_ATTACKERS)
        Pass();   // (both players get priority once the attackers are declared)
    memset(&a, 0, sizeof(a));
    memset(a.block, -1, sizeof(a.block));
    CHECK(q->kind == MTG_Q_BLOCKERS, "asked for blockers (%d)", q->kind);
    for (i = 0; i < q->candidateCount; i++)
        if (q->candidates[i] == blocker)
            for (j = 0; j < q->attackerCount; j++)
                if (q->attackers[j] == attacker)
                    a.block[i] = (int8_t)j;
    Answer(&a);
}

static void TestCombat(void)
{
    int angel, bears, knight, wurm, goblin, sick;
    printf("combat: flying, vigilance, first strike, trample, haste\n");
    Table();
    angel = Put(0, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Attack(1);
    CHECK(Mtg_Query(&sD)->kind != MTG_Q_BLOCKERS, "bears can't block a flier");
    GoTo(0, MTG_STEP_COMBAT_END);
    CHECK(sD.players[1].life == 16, "angel hit: %d", sD.players[1].life);
    CHECK(!sD.objs[angel].tapped, "vigilance: untapped");

    Table();
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    knight = Put(1, "White Knight", MTG_ZONE_BATTLEFIELD);
    Attack(1);
    Block(knight, bears);
    GoTo(0, MTG_STEP_COMBAT_END);
    CHECK(Zone(bears) == MTG_ZONE_GRAVEYARD && Zone(knight) == MTG_ZONE_BATTLEFIELD, "first strike wins the trade");

    Table();
    wurm = Put(0, "Rootbreaker Wurm", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Attack(1);
    Block(bears, wurm);
    GoTo(0, MTG_STEP_COMBAT_END);
    CHECK(sD.players[1].life == 16 && Zone(bears) == MTG_ZONE_GRAVEYARD, "trample: 4 through, %d", sD.players[1].life);

    Table();
    goblin = Put(0, "Raging Goblin", MTG_ZONE_BATTLEFIELD);
    sick = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    sD.objs[goblin].sick = sD.objs[sick].sick = 1;
    GoTo(0, MTG_STEP_COMBAT_BEGIN);
    Pass();
    Pass();
    {
        const MtgQuery *q = Mtg_Query(&sD);
        CHECK(q->kind == MTG_Q_ATTACKERS && q->candidateCount == 1 && q->candidates[0] == goblin,
              "only the haste goblin may attack");
    }
}

static void TestMana(void)
{
    int elves, bears, ritual, wail, i, hand;
    printf("mana: elves, dark ritual, specter's wail\n");
    Table();
    Lands(0, "Forest", 1);
    elves = Put(0, "Llanowar Elves", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_HAND);
    sD.objs[elves].sick = 1;
    CHECK(Find(MTG_ACT_CAST, bears, -1, 0) < 0, "sick elves make no mana");
    sD.objs[elves].sick = 0;
    Do(MTG_ACT_CAST, bears, -1, 0);
    CHECK(sD.objs[elves].tapped, "elves tapped for the bears");
    PassBoth();
    CHECK(Zone(bears) == MTG_ZONE_BATTLEFIELD, "bears in play");

    Table();
    Lands(0, "Swamp", 1);
    ritual = Put(0, "Dark Ritual", MTG_ZONE_HAND);
    wail = Put(0, "Specter's Wail", MTG_ZONE_HAND);
    for (i = 0; i < 3; i++)
        Put(1, "Plains", MTG_ZONE_HAND);
    CHECK(Find(MTG_ACT_CAST, wail, -1, 0) < 0, "the wail isn't castable from one swamp");
    Do(MTG_ACT_CAST, ritual, -1, 0);
    PassBoth();
    CHECK(sD.players[0].pool[MTG_B] == 3, "three black floating: %d", sD.players[0].pool[MTG_B]);
    Do(MTG_ACT_CAST, wail, MTG_TGT_PLAYER, 1);
    PassBoth();
    for (i = 0, hand = 0; i < sD.objCount; i++)
        hand += sD.objs[i].zone == MTG_ZONE_HAND && sD.objs[i].owner == 1;
    CHECK(hand == 2, "the wail took one at random: %d left", hand);
    CHECK(sD.players[0].pool[MTG_B] == 1, "one black still floating: %d", sD.players[0].pool[MTG_B]);
    GoTo(0, MTG_STEP_COMBAT_BEGIN);
    CHECK(sD.players[0].pool[MTG_B] == 0, "floating mana empties when the step ends");
}

static void TestSpells(void)
{
    int wrath, a, b, wurm, counter, sorc, stream;
    printf("wrath, counterspell, prodigal sorcerer, stream of life\n");
    Table();
    Lands(0, "Plains", 4);
    wrath = Put(0, "Wrath of God", MTG_ZONE_HAND);
    a = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    b = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, wrath, -1, 0);
    PassBoth();
    CHECK(Zone(a) == MTG_ZONE_GRAVEYARD && Zone(b) == MTG_ZONE_GRAVEYARD, "every creature destroyed");

    Table();
    Lands(0, "Forest", 6);
    Lands(1, "Island", 2);
    wurm = Put(0, "Craw Wurm", MTG_ZONE_HAND);
    counter = Put(1, "Counterspell", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, wurm, -1, 0);
    Pass();
    Do(MTG_ACT_CAST, counter, MTG_TGT_STACK, wurm);
    PassBoth();
    CHECK(Zone(wurm) == MTG_ZONE_GRAVEYARD && sD.stackCount == 0, "wurm countered");

    Table();
    sorc = Put(0, "Prodigal Sorcerer", MTG_ZONE_BATTLEFIELD);
    sD.objs[sorc].sick = 1;
    CHECK(Find(MTG_ACT_ACTIVATE, sorc, -1, 0) < 0, "a sick sorcerer can't tap");
    sD.objs[sorc].sick = 0;
    Do(MTG_ACT_ACTIVATE, sorc, MTG_TGT_PLAYER, 1);
    PassBoth();
    CHECK(sD.players[1].life == 19 && sD.objs[sorc].tapped, "ping: %d", sD.players[1].life);

    Table();
    Lands(0, "Forest", 5);
    stream = Put(0, "Stream of Life", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, stream, MTG_TGT_PLAYER, 0);
    PassBoth();
    CHECK(sD.players[0].life == 24, "X = 4: %d", sD.players[0].life);
}

// ---- whole duels between the stand-in AIs

// a choice while something resolves (MTG_Q_CHOOSE): card = 1 / 0, or the candidate holding obj
static void Choose(int card)
{
    MtgAnswer a;
    memset(&a, 0, sizeof(a));
    a.card = (uint8_t)card;
    Answer(&a);
}

static void ChoosePick(int obj)
{
    const MtgQuery *q = Mtg_Query(&sD);
    int i;
    for (i = 0; i < q->candidateCount && q->candidates[i] != obj; i++)
        ;
    CHECK(i < q->candidateCount, "%s can be picked", Mtg_Card(sD.objs[obj].def)->name);
    Choose(i);
}

static int HandCount(int player)
{
    int i, n = 0;
    for (i = 0; i < sD.objCount; i++)
        n += sD.objs[i].zone == MTG_ZONE_HAND && sD.objs[i].owner == player;
    return n;
}

static void TestTriggers(void)
{
    const MtgQuery *q;
    MtgAnswer a;
    int nek, bears, angel, digger, dead, warden, specter, leak, wurm, braids, land, i;
    printf("triggers: enters, dies, upkeep, damage, unless, sacrifice\n");

    Table();   // Nekrataal: two legal targets, its controller chooses
    Lands(0, "Swamp", 4);
    nek = Put(0, "Nekrataal", MTG_ZONE_HAND);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    angel = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    Put(1, "Dark Ritual", MTG_ZONE_EXILE);
    Do(MTG_ACT_CAST, nek, -1, 0);
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_TRIGGER_TARGET && q->player == 0 && q->actionCount == 2, "nekrataal asks for a target (%d, %d)",
          q->kind, q->actionCount);
    memset(&a, 0, sizeof(a));
    for (i = 0; i < q->actionCount; i++)
        if (q->actions[i].target.id == angel)
            a.action = (uint16_t)i;
    Answer(&a);
    CHECK(sD.stackCount == 1 && sD.stack[0].trigger != 0xFFFF, "the trigger is on the stack");
    PassBoth();
    CHECK(Zone(angel) == MTG_ZONE_GRAVEYARD && Zone(bears) == MTG_ZONE_BATTLEFIELD, "the angel destroyed");

    Table();   // Gravedigger: one target (chosen by itself), "you may" asked as it resolves
    Lands(0, "Swamp", 4);
    digger = Put(0, "Gravedigger", MTG_ZONE_HAND);
    dead = Put(0, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
    Put(1, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
    Do(MTG_ACT_CAST, digger, -1, 0);
    PassBoth();
    CHECK(sD.stackCount == 1 && sD.stack[0].target.kind == MTG_TGT_OBJ && sD.stack[0].target.id == dead,
          "gravedigger targets the only creature card in its graveyard");
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_YESNO && q->player == 0, "you may: asked (%d)", q->kind);
    Choose(1);
    CHECK(Zone(dead) == MTG_ZONE_HAND, "the bears back in hand");

    Table();   // Soul Warden: another creature enters
    Lands(0, "Forest", 2);
    warden = Put(0, "Soul Warden", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bears, -1, 0);
    PassBoth();
    PassBoth();
    CHECK(sD.players[0].life == 21, "soul warden: %d", sD.players[0].life);
    (void)warden;

    Table();   // Hypnotic Specter's combat damage: a discard at random; Abyssal Specter's: of the player's choice
    specter = Put(0, "Hypnotic Specter", MTG_ZONE_BATTLEFIELD);
    Put(1, "Grizzly Bears", MTG_ZONE_HAND);
    Put(1, "Giant Growth", MTG_ZONE_HAND);
    Attack(1);
    GoTo(0, MTG_STEP_COMBAT_END);
    CHECK(sD.players[1].life == 18 && HandCount(1) == 1, "specter hit: %d life, %d cards", sD.players[1].life, HandCount(1));
    (void)specter;

    Table();
    specter = Put(0, "Abyssal Specter", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_HAND);
    wurm = Put(1, "Craw Wurm", MTG_ZONE_HAND);
    Attack(1);
    for (i = 0; i < 40 && Mtg_Query(&sD)->kind != MTG_Q_CHOOSE; i++)
        Pass();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_PICK && q->player == 1 && q->candidateCount == 2,
          "the damaged player picks the discard");
    ChoosePick(wurm);
    CHECK(Zone(wurm) == MTG_ZONE_GRAVEYARD && Zone(bears) == MTG_ZONE_HAND, "the wurm discarded");

    Table();   // Mana Leak: paid, then not payable
    Lands(0, "Forest", 9);
    Lands(1, "Island", 2);
    wurm = Put(0, "Craw Wurm", MTG_ZONE_HAND);
    leak = Put(1, "Mana Leak", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, wurm, -1, 0);
    Pass();
    Do(MTG_ACT_CAST, leak, MTG_TGT_STACK, wurm);
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_PAY && q->player == 0 && q->pay.generic == 3,
          "the wurm's controller may pay 3");
    Choose(1);
    PassBoth();
    CHECK(Zone(wurm) == MTG_ZONE_BATTLEFIELD, "paid: the wurm resolves");

    Table();
    Lands(0, "Forest", 6);
    Lands(1, "Island", 2);
    wurm = Put(0, "Craw Wurm", MTG_ZONE_HAND);
    leak = Put(1, "Mana Leak", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, wurm, -1, 0);
    Pass();
    Do(MTG_ACT_CAST, leak, MTG_TGT_STACK, wurm);
    PassBoth();
    CHECK(Zone(wurm) == MTG_ZONE_GRAVEYARD, "no mana left: countered without asking");

    Table();   // Braids: each upkeep, that player sacrifices an artifact, creature or land of their choice
    braids = Put(0, "Braids, Cabal Minion", MTG_ZONE_BATTLEFIELD);
    land = Put(1, "Island", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    for (i = 0; i < 60 && Mtg_Query(&sD)->kind != MTG_Q_CHOOSE; i++)
        Pass();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->player == 1 && sD.step == MTG_STEP_UPKEEP && sD.active == 1,
          "the opponent sacrifices in their upkeep");
    ChoosePick(land);
    CHECK(Zone(land) == MTG_ZONE_GRAVEYARD && Zone(bears) == MTG_ZONE_BATTLEFIELD, "the island sacrificed");
    (void)braids;
}

static void Deck(MtgDeck *d, const char *land, const char *const *cards)
{
    int i;
    d->count = 0;
    for (i = 0; i < 24; i++)
        d->cards[d->count++] = (uint16_t)Card(land);
    for (; *cards; cards++) {
        int id = Card(*cards);
        if (!Mtg_Card(id)->supported)
            printf("  deck card %s unsupported: %s\n", *cards, Mtg_Card(id)->unsupported), sFails++;
        for (i = 0; i < 4; i++)
            d->cards[d->count++] = (uint16_t)id;
    }
}

static const char *const sRed[] = {"Raging Goblin", "Lightning Bolt", "Shock", "Hill Giant", "Fire Elemental",
                                   "Shivan Dragon", "Gray Ogre", "Goblin Hero", "Volcanic Hammer",
                                   "Blaze", NULL};
static const char *const sGreen[] = {"Llanowar Elves", "Grizzly Bears", "Giant Growth", "Craw Wurm", "War Mammoth",
                                     "Rootbreaker Wurm", "Pygmy Razorback", "Spined Wurm", "Stream of Life",
                                     NULL};

static uint32_t PlayOut(MtgDuel *d, int *turns, int *winner)
{
    int guard = 0;
    while (Mtg_Query(d)->kind != MTG_Q_GAME_OVER && guard++ < 200000) {
        MtgAnswer a;
        MtgAi_Answer(d, &a);
        if (!Mtg_Answer(d, &a)) {
            printf("  the AI's answer was refused (query %d)\n", Mtg_Query(d)->kind);
            sFails++;
            break;
        }
    }
    *turns = d->turn;
    *winner = d->winner;
    return d->hash;
}

static void TestDuels(void)
{
    static MtgDeck red, green;
    static MtgDuel copy;
    int seed, wins[3] = {0, 0, 0}, maxTurns = 0;
    printf("stand-in AI duels\n");
    Deck(&red, "Mountain", sRed);
    Deck(&green, "Forest", sGreen);
    for (seed = 1; seed <= 40; seed++) {
        static MtgDuel a, b;
        int ta, wa, tb, wb, i;
        uint32_t ha, hb;
        Mtg_DuelInit(&a, &red, &green, (uint32_t)seed);
        ha = PlayOut(&a, &ta, &wa);
        Mtg_DuelInit(&b, &red, &green, (uint32_t)seed);
        for (i = 0; i < 300 && Mtg_Query(&b)->kind != MTG_Q_GAME_OVER; i++) {   // part way, then a copy goes on
            MtgAnswer ans;
            MtgAi_Answer(&b, &ans);
            Mtg_Answer(&b, &ans);
        }
        memcpy(&copy, &b, sizeof(copy));
        hb = PlayOut(&copy, &tb, &wb);
        CHECK(ha == hb && ta == tb && wa == wb, "seed %d: the copy played another duel", seed);
        CHECK(wa >= 0 && ta < 60, "seed %d: ended in %d turns (winner %d)", seed, ta, wa);
        wins[wa < 0 ? 2 : wa]++;
        if (ta > maxTurns)
            maxTurns = ta;
    }
    printf("  red %d, green %d, draws %d, longest %d turns\n", wins[0], wins[1], wins[2], maxTurns);
}

static void TestDb(void)
{
    static const char *const sMust[] = {"Lightning Bolt", "Grizzly Bears", "Giant Growth", "Serra Angel",
        "Llanowar Elves", "Swords to Plowshares", "Dark Ritual", "Wrath of God", "Craw Wurm",
        "Shivan Dragon", "Prodigal Sorcerer", "Stream of Life", "Forest", "Counterspell", "White Knight",
        "Raging Goblin", "Rootbreaker Wurm", "Specter's Wail", NULL};
    const char *const *n;
    int i, supported = 0;
    printf("card database\n");
    for (i = 0; i < Mtg_DbCount(); i++)
        supported += Mtg_Card(i)->supported;
    printf("  %d cards, %d playable\n", Mtg_DbCount(), supported);
    for (n = sMust; *n; n++) {
        int id = Mtg_CardByName(*n);
        CHECK(id >= 0 && Mtg_Card(id)->supported, "%s playable (%s)", *n,
              id < 0 ? "not in the pool" : Mtg_Card(id)->unsupported ? Mtg_Card(id)->unsupported : "?");
    }
}

// ---- the collection, the decks, boosters, the file
static void TestCollection(void)
{
    static MtgCollection c, back;
    static uint8_t buf[200000];
    uint16_t booster[MTG_PACK_MAX];
    uint8_t rarity[MTG_PACK_MAX];
    MtgSavedDeck d;
    int i, k, n;
    uint32_t size;
    printf("collection, starters, duelists, boosters\n");
    for (i = 0; i < MTG_STARTERS; i++) {
        int missing = Mtg_RecipeDeck(&gMtgStarters[i], &d);
        CHECK(missing == 0 && d.count == 40, "starter %s: %d cards, %d missing", gMtgStarters[i].name, d.count, missing);
        CHECK(Mtg_DeckProblem(NULL, &d) == NULL, "starter %s plays: %s", gMtgStarters[i].name, Mtg_DeckProblem(NULL, &d));
    }
    for (i = 0; i < MTG_DUELISTS; i++) {
        MtgRecipe r = {gMtgDuelists[i].name, "", 0, gMtgDuelists[i].cards};
        int missing = Mtg_RecipeDeck(&r, &d);
        CHECK(missing == 0 && d.count == 40, "duelist %s: %d cards, %d missing", r.name, d.count, missing);
        CHECK(Mtg_DeckProblem(NULL, &d) == NULL, "duelist %s's deck plays: %s", r.name, Mtg_DeckProblem(NULL, &d));
    }
    Mtg_CollectionInit(&c, 99);
    c.owner = 0x12345678;
    Mtg_GiveStarter(&c, 3);
    CHECK(c.optedIn && c.decks[0].count == 40 && !strcmp(c.decks[0].name, "WILDWOOD"), "the starter is deck 0");
    CHECK(Mtg_DeckProblem(&c, &c.decks[0]) == NULL, "the starter is owned: %s", Mtg_DeckProblem(&c, &c.decks[0]));
    CHECK(c.owned[Card("Llanowar Elves")] == 2 && c.owned[Card("Forest")] == 0, "owned: 2 elves, forests free");
    {   // boosters: by set, held, then opened (11 commons, 3 uncommons, a rare in most sets)
        int set = -1, avail = 0, j, rares = 0, dups = 0;
        for (i = 0; i < MTG_SETS; i++)
            if (Mtg_SetAvailable(i)) {
                avail++;
                if (set < 0 && gMtgSets[i].commons == 11)
                    set = i;
            }
        printf("  %d of %d booster sets open\n", avail, MTG_SETS);
        CHECK(avail >= 8 && set >= 0, "enough booster sets play (%d)", avail);
        CHECK(Mtg_OpenPack(&c, set, booster, rarity) == 0, "no booster held: nothing opens");
        CHECK(Mtg_AwardPack(&c, set) && Mtg_AwardPack(&c, set) && c.packs[set] == 2, "two boosters held");
        n = Mtg_OpenPack(&c, set, booster, rarity);
        CHECK(n == 15 && c.packs[set] == 1 && c.opened[set] == 1 && c.packsOpened == 1, "a booster of %d", n);
        for (i = 0; i < n; i++) {
            uint16_t ids[512];
            uint8_t rs[512];
            int m = Mtg_SetCards(set, ids, rs), in = 0;
            for (j = 0; j < m; j++)
                in |= ids[j] == booster[i];
            CHECK(in && Mtg_CanUse(booster[i]) && !Mtg_IsBasicLand(booster[i]) && c.owned[booster[i]] > 0,
                  "booster card %d from the set, usable", i);
            for (j = 0; j < i; j++)
                dups += booster[j] == booster[i];
            rares += rarity[i] == MTG_RARITY_RARE;
        }
        CHECK(rarity[0] == MTG_RARITY_COMMON && rarity[n - 1] == MTG_RARITY_RARE && rares == 1 && !dups,
              "commons first, one rare last, no card twice (%d rares, %d twice)", rares, dups);
        CHECK(Mtg_SetMascot(set) >= 0, "the set's pack has a card on it");
    }
    d = c.decks[0];
    Mtg_DeckAdd(&d, Card("Llanowar Elves"));
    Mtg_DeckAdd(&d, Card("Llanowar Elves"));
    Mtg_DeckAdd(&d, Card("Llanowar Elves"));
    CHECK(Mtg_DeckProblem(&c, &d) != NULL, "five elves (two owned) refused");
    strcpy(c.decks[3].name, "TEST");
    for (k = 0; k < 41; k++)
        Mtg_DeckAdd(&c.decks[3], Card("Island"));
    c.wins = 7, c.beaten = 5, c.activeDeck = 3;
    c.ace[0] = (int16_t)Card("Llanowar Elves");
    c.tray[2] = 0x41;
    size = Mtg_CollectionWrite(&c, buf, sizeof(buf));
    CHECK(size > 0 && Mtg_CollectionRead(&back, buf, size), "written and read back (%u bytes)", size);
    CHECK(back.owner == c.owner && back.wins == 7 && back.beaten == 5 && back.activeDeck == 3 && back.optedIn,
          "the record came back");
    CHECK(!memcmp(back.owned, c.owned, sizeof(c.owned)), "the cards came back");
    for (k = 0; k < MTG_DECKS; k++)
        CHECK(back.decks[k].count == c.decks[k].count && !memcmp(back.decks[k].cards, c.decks[k].cards,
              c.decks[k].count * 2) && !strcmp(back.decks[k].name, c.decks[k].name), "deck %d came back", k);
    CHECK(back.ace[0] == c.ace[0] && back.ace[1] == -1, "the ACE CARDs came back");
    CHECK(back.tray[2] == 0x41 && back.tray[0] == 0, "the tray colors came back");
    CHECK(!memcmp(back.packs, c.packs, sizeof(c.packs)) && !memcmp(back.opened, c.opened, sizeof(c.opened))
          && back.packsOpened == 1, "the boosters came back");
    CHECK(Mtg_DeckAce(&back, 0) == (Mtg_DeckCopies(&back.decks[0], c.ace[0]) ? c.ace[0] : Mtg_AutoAce(back.decks[0].cards,
          back.decks[0].count)), "the ACE CARD stands for its deck");
    {   // a format 1 file (no ACE CARDs at its end) still reads, every deck's ace picked by the game
        uint32_t v4 = 3 + 6;   // (format 4's boosters: one set's)
        buf[4] = 1;
        CHECK(Mtg_CollectionRead(&back, buf, size - v4 - MTG_DECKS * 5) && back.ace[0] == -1
              && back.decks[3].count == 41 && !back.tray[2], "format 1 reads");
        buf[4] = 2;
        CHECK(Mtg_CollectionRead(&back, buf, size - v4 - MTG_DECKS) && back.ace[0] == c.ace[0] && !back.tray[2],
              "format 2 reads");
        buf[4] = 3;
        CHECK(Mtg_CollectionRead(&back, buf, size - v4) && back.tray[2] == 0x41 && !back.packsOpened, "format 3 reads");
        buf[4] = 4;
    }
    CHECK(!Mtg_CollectionRead(&back, buf, 3), "a cut file is refused");
}

int main(int argc, char **argv)
{
    uint32_t size;
    void *pak = ReadFile(argc > 1 ? argv[1] : "graphics/mtg/forge.pak", &size);
    if (!pak || !Mtg_DbLoad(pak, size)) {
        printf("can't read the pack\n");
        return 1;
    }
    free(pak);
    TestDb();
    TestCollection();
    TestBolt();
    TestResponse();
    TestPlowshares();
    TestCombat();
    TestMana();
    TestSpells();
    TestTriggers();
    TestDuels();
    printf("%d checks, %d failed\n", sChecks, sFails);
    return sFails != 0;
}
