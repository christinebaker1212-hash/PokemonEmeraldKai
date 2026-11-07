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
    o->attachedTo = MTG_NO_OBJ;
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
static void ChoosePick(int obj);
static int HandCount(int player);

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

// Auras (Enchant:Creature): cast onto a creature, what they give it, Forge's AttachAILogic, falling off
static void TestAuras(void)
{
    int bears, strength, pacifism, angel, bolt, fire, ai;
    printf("auras: attach, pump, pacifism, the AI's side, falling off\n");
    Table();
    Lands(0, "Plains", 3);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    angel = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    strength = Put(0, "Holy Strength", MTG_ZONE_HAND);
    pacifism = Put(0, "Pacifism", MTG_ZONE_HAND);
    CHECK(Find(MTG_ACT_CAST, strength, MTG_TGT_OBJ, bears) >= 0, "holy strength can target the bears");
    Do(MTG_ACT_CAST, strength, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Zone(strength) == MTG_ZONE_BATTLEFIELD && sD.objs[strength].attachedTo == bears, "holy strength on the bears");
    CHECK(Mtg_Power(&sD, bears) == 3 && Mtg_Toughness(&sD, bears) == 4, "bears 3/4 (%d/%d)", Mtg_Power(&sD, bears),
          Mtg_Toughness(&sD, bears));
    CHECK(Mtg_Power(&sD, angel) == 4, "not the angel");
    Do(MTG_ACT_CAST, pacifism, MTG_TGT_OBJ, angel);
    PassBoth();
    CHECK(sD.objs[pacifism].attachedTo == angel, "pacifism on the angel");
    CHECK(!Mtg_CanBlock(&sD, angel, bears), "the angel can't block");
    Lands(0, "Mountain", 2);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Zone(bears) == MTG_ZONE_BATTLEFIELD, "a bolt doesn't kill the 3/4 bears");
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Zone(bears) == MTG_ZONE_GRAVEYARD, "a second one does");
    CHECK(Zone(strength) == MTG_ZONE_GRAVEYARD, "the bears gone: holy strength goes to the graveyard");
    CHECK(Zone(pacifism) == MTG_ZONE_BATTLEFIELD, "pacifism stays");

    Table();   // Rancor: back to its owner's hand when it goes to the graveyard
    Lands(0, "Forest", 1);
    Lands(0, "Mountain", 1);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    fire = Put(0, "Rancor", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, fire, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Mtg_Power(&sD, bears) == 4 && (Mtg_Keywords(&sD, bears) & MTG_KW_BIT(MTG_KW_TRAMPLE)), "rancor: 4/2 trample");
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, bears);
    PassBoth();
    PassBoth();
    CHECK(Zone(bears) == MTG_ZONE_GRAVEYARD && Zone(fire) == MTG_ZONE_HAND, "the bears gone: rancor back in hand (%d)", Zone(fire));

    Table();   // Firebreathing: its ability pumps the creature it enchants
    Lands(0, "Mountain", 3);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    fire = Put(0, "Firebreathing", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, fire, MTG_TGT_OBJ, bears);
    PassBoth();
    Do(MTG_ACT_ACTIVATE, fire, -1, 0);
    PassBoth();
    CHECK(Mtg_Power(&sD, bears) == 3, "firebreathing: 3/2 (%d)", Mtg_Power(&sD, bears));

    Table();   // the AI: its pump on its own creature, its curse on the other side's
    ai = 0;
    {
        MtgAction act;
        memset(&act, 0, sizeof(act));
        Lands(0, "Plains", 2);
        bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        angel = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD);
        pacifism = Put(0, "Pacifism", MTG_ZONE_HAND);
        CHECK(Find(MTG_ACT_CAST, pacifism, MTG_TGT_OBJ, angel) >= 0 && Find(MTG_ACT_CAST, pacifism, MTG_TGT_OBJ, bears) >= 0,
              "pacifism can go on either");
        {
            MtgAnswer ans;
            const MtgQuery *q;
            memset(&ans, 0, sizeof(ans));
            Mtg_RefreshQuery(&sD);
            q = Mtg_Query(&sD);
            MtgAi_Answer(&sD, &ans);
            act = q->actions[ans.action];
            ai = act.kind == MTG_ACT_CAST && act.obj == pacifism && act.target.kind == MTG_TGT_OBJ && act.target.id == angel;
        }
        CHECK(ai, "the AI puts pacifism on the angel");
    }
}

// costs beyond mana: sacrifice (picked), discard (picked or at random), pay life, return itself
static void TestCosts(void)
{
    int raze, mountain, land, bomb, elves, bears, book, seizure, a, b, disc, att;
    const MtgQuery *q;
    printf("costs: sacrifice, discard, random discard, life, return to hand\n");
    Table();   // Raze: sacrifice a land as you cast it (the player picks which)
    mountain = Put(0, "Mountain", MTG_ZONE_BATTLEFIELD);
    land = Put(0, "Forest", MTG_ZONE_BATTLEFIELD);
    raze = Put(0, "Raze", MTG_ZONE_HAND);
    a = Put(1, "Plains", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, raze, MTG_TGT_OBJ, a);
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->forCost == 1, "raze asks what to sacrifice (%d %d)", q->kind, q->forCost);
    ChoosePick(land);
    CHECK(Zone(land) == MTG_ZONE_GRAVEYARD && Zone(mountain) == MTG_ZONE_BATTLEFIELD, "the forest sacrificed");
    PassBoth();
    CHECK(Zone(a) == MTG_ZONE_GRAVEYARD, "the plains destroyed");

    Table();   // no land to sacrifice once the mountain pays: Raze can't be cast with only one land
    Put(0, "Mountain", MTG_ZONE_BATTLEFIELD);
    raze = Put(0, "Raze", MTG_ZONE_HAND);
    a = Put(1, "Plains", MTG_ZONE_BATTLEFIELD);
    CHECK(Find(MTG_ACT_CAST, raze, MTG_TGT_OBJ, a) >= 0, "one mountain: it pays and is sacrificed (tapped lands sacrifice)");

    Table();   // Goblin Bombardment: sacrifice a creature, 1 damage
    bomb = Put(0, "Goblin Bombardment", MTG_ZONE_BATTLEFIELD);
    elves = Put(0, "Llanowar Elves", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_ACTIVATE, bomb, MTG_TGT_PLAYER, 1);
    ChoosePick(elves);
    PassBoth();
    CHECK(Zone(elves) == MTG_ZONE_GRAVEYARD && Zone(bears) == MTG_ZONE_BATTLEFIELD && sD.players[1].life == 19,
          "elves sacrificed, 1 damage (%d)", sD.players[1].life);

    Table();   // Book of Rass: 2 and 2 life, draw
    Lands(0, "Plains", 2);
    book = Put(0, "Book of Rass", MTG_ZONE_BATTLEFIELD);
    Put(0, "Plains", MTG_ZONE_LIBRARY);
    Do(MTG_ACT_ACTIVATE, book, -1, 0);
    CHECK(sD.players[0].life == 18, "2 life paid (%d)", sD.players[0].life);
    PassBoth();
    CHECK(HandCount(0) == 1, "a card drawn (%d)", HandCount(0));

    Table();   // Sonic Seizure: discard a card at random as you cast it
    Lands(0, "Mountain", 1);
    seizure = Put(0, "Sonic Seizure", MTG_ZONE_HAND);
    CHECK(Find(MTG_ACT_CAST, seizure, MTG_TGT_PLAYER, 1) < 0, "no other card in hand: it can't be cast");
    a = Put(0, "Grizzly Bears", MTG_ZONE_HAND);
    b = Put(0, "Serra Angel", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, seizure, MTG_TGT_PLAYER, 1);
    CHECK(Mtg_Query(&sD)->kind == MTG_Q_PRIORITY, "no pick for a random discard");
    CHECK((Zone(a) == MTG_ZONE_GRAVEYARD) + (Zone(b) == MTG_ZONE_GRAVEYARD) == 1, "one card discarded at random");
    PassBoth();
    CHECK(sD.players[1].life == 17, "3 damage (%d)", sD.players[1].life);

    Table();   // Mental Discipline: discard a card of the player's choice
    Lands(0, "Island", 2);
    disc = Put(0, "Mental Discipline", MTG_ZONE_BATTLEFIELD);
    a = Put(0, "Grizzly Bears", MTG_ZONE_HAND);
    b = Put(0, "Serra Angel", MTG_ZONE_HAND);
    Put(0, "Island", MTG_ZONE_LIBRARY);
    Do(MTG_ACT_ACTIVATE, disc, -1, 0);
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->forCost == 2 && q->candidateCount == 2, "which card to discard");
    ChoosePick(b);
    CHECK(Zone(b) == MTG_ZONE_GRAVEYARD && Zone(a) == MTG_ZONE_HAND, "the angel discarded");
    PassBoth();
    CHECK(HandCount(0) == 2, "and a card drawn (%d)", HandCount(0));

    Table();   // Attunement: return it to its owner's hand: draw three, discard four
    att = Put(0, "Attunement", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_ACTIVATE, att, -1, 0);
    CHECK(Zone(att) == MTG_ZONE_HAND, "attunement back in hand");

    Table();   // the AI: a 1-damage ping isn't worth its best creature
    {
        MtgAnswer ans;
        bomb = Put(0, "Goblin Bombardment", MTG_ZONE_BATTLEFIELD);
        Put(0, "Serra Angel", MTG_ZONE_BATTLEFIELD);
        Mtg_RefreshQuery(&sD);
        memset(&ans, 0, sizeof(ans));
        MtgAi_Answer(&sD, &ans);
        CHECK(Mtg_Query(&sD)->actions[ans.action].kind == MTG_ACT_PASS, "the AI keeps its angel");
    }
}

// regeneration: a shield, used by destruction (lethal damage too), gone at end of turn, NoRegen ignores it
static void TestRegenerate(void)
{
    int skel, bolt, terror, wrath, a;
    printf("regenerate: shields, lethal damage, destroy, can't be regenerated\n");
    Table();
    Lands(0, "Swamp", 3);
    Lands(0, "Mountain", 1);
    skel = Put(0, "Drudge Skeletons", MTG_ZONE_BATTLEFIELD);
    CHECK(Find(MTG_ACT_ACTIVATE, skel, -1, 0) >= 0, "drudge skeletons can regenerate");
    Do(MTG_ACT_ACTIVATE, skel, -1, 0);
    PassBoth();
    CHECK(sD.objs[skel].regen == 1, "a shield (%d)", sD.objs[skel].regen);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, skel);
    PassBoth();
    CHECK(Zone(skel) == MTG_ZONE_BATTLEFIELD && sD.objs[skel].tapped && sD.objs[skel].damage == 0 && sD.objs[skel].regen == 0,
          "bolted: it regenerates (tapped, healed, the shield used)");
    terror = Put(0, "Terror", MTG_ZONE_HAND);
    (void)terror;

    Table();   // Wrath of God: can't be regenerated
    Lands(0, "Swamp", 1);
    Lands(0, "Plains", 4);
    skel = Put(0, "Drudge Skeletons", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_ACTIVATE, skel, -1, 0);
    PassBoth();
    wrath = Put(0, "Wrath of God", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, wrath, -1, 0);
    PassBoth();
    CHECK(Zone(skel) == MTG_ZONE_GRAVEYARD, "wrath: no regeneration");

    Table();   // the shield wears off at end of turn
    Lands(0, "Swamp", 1);
    skel = Put(0, "Drudge Skeletons", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_ACTIVATE, skel, -1, 0);
    PassBoth();
    CHECK(sD.objs[skel].regen == 1, "a shield this turn (%d)", sD.objs[skel].regen);
    GoTo(1, MTG_STEP_UPKEEP);
    CHECK(sD.objs[skel].regen == 0, "next turn: no shield (%d, turn %d)", sD.objs[skel].regen, sD.turn);

    Table();   // the AI regenerates only when its creature would die
    {
        MtgAnswer ans;
        Lands(1, "Swamp", 1);
        Lands(0, "Mountain", 1);
        skel = Put(1, "Drudge Skeletons", MTG_ZONE_BATTLEFIELD);
        a = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
        Do(MTG_ACT_CAST, a, MTG_TGT_OBJ, skel);
        Pass();
        memset(&ans, 0, sizeof(ans));
        MtgAi_Answer(&sD, &ans);
        CHECK(Mtg_Query(&sD)->player == 1 && Mtg_Query(&sD)->actions[ans.action].kind == MTG_ACT_ACTIVATE,
              "the bolt on the stack: the AI regenerates");
    }
}

// statics that count (Blanchwood Armor: +1/+1 per Forest you control), unblockable, the Wards
static void TestStatics2(void)
{
    int bears, armor, ward, path, b2;
    printf("statics 2: counted amounts, unblockable, wards\n");
    Table();
    Lands(0, "Forest", 3);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    armor = Put(0, "Blanchwood Armor", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, armor, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Mtg_Power(&sD, bears) == 5 && Mtg_Toughness(&sD, bears) == 5, "three forests: 5/5 (%d/%d)", Mtg_Power(&sD, bears),
          Mtg_Toughness(&sD, bears));
    Put(0, "Forest", MTG_ZONE_BATTLEFIELD);
    CHECK(Mtg_Power(&sD, bears) == 6, "a fourth: 6/6 (%d)", Mtg_Power(&sD, bears));

    Table();   // Cephalid Pathmage can't be blocked
    path = Put(0, "Cephalid Pathmage", MTG_ZONE_BATTLEFIELD);
    b2 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    CHECK(!Mtg_CanBlock(&sD, b2, path), "nothing blocks the pathmage");

    Table();   // White Ward on a creature: protection from white, and it stays on
    Lands(0, "Plains", 1);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    ward = Put(0, "White Ward", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, ward, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK((Mtg_Protection(&sD, bears) & MTG_COLOR_BIT(MTG_W)) != 0, "protection from white");
    CHECK(Zone(ward) == MTG_ZONE_BATTLEFIELD, "the ward stays on");

    Table();   // Coastal Tower enters tapped; Kismet: the opponent's things enter tapped
    {
        int tower = Put(0, "Coastal Tower", MTG_ZONE_HAND), elves, angel, meek;
        Do(MTG_ACT_LAND, tower, -1, 0);
        CHECK(Zone(tower) == MTG_ZONE_BATTLEFIELD && sD.objs[tower].tapped, "coastal tower enters tapped");
        Put(1, "Kismet", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Forest", 1);
        elves = Put(0, "Llanowar Elves", MTG_ZONE_HAND);
        Do(MTG_ACT_CAST, elves, -1, 0);
        PassBoth();
        CHECK(Zone(elves) == MTG_ZONE_BATTLEFIELD && sD.objs[elves].tapped, "kismet: the elves enter tapped");

        Table();   // Meekstone: power 3 or more doesn't untap
        meek = Put(1, "Meekstone", MTG_ZONE_BATTLEFIELD);
        angel = Put(0, "Serra Angel", MTG_ZONE_BATTLEFIELD);
        elves = Put(0, "Llanowar Elves", MTG_ZONE_BATTLEFIELD);
        sD.objs[angel].tapped = sD.objs[elves].tapped = 1;
        while (sD.turn < 3)   // (to player 0's next turn, past its untap step)
            GoTo(sD.priority ^ 1, MTG_STEP_UPKEEP);
        CHECK(sD.objs[angel].tapped && !sD.objs[elves].tapped, "meekstone: the angel stays tapped, the elves untap (%d %d, turn %d, meek zone %d, forbids %d)",
              sD.objs[angel].tapped, sD.objs[elves].tapped, sD.turn, Zone(meek), Mtg_StaticForbids(&sD, MTG_STATIC_NO_UNTAP, angel, MTG_NO_OBJ));
        (void)meek;
    }
}

// tokens: made from Forge's token names, under their owner's control, gone once they leave the battlefield
static void TestTokens(void)
{
    int scouts, i, n, tok = -1, after, bears, bolt;
    printf("tokens: goblin scouts, afterlife, a token dies and is gone\n");
    CHECK(Mtg_TokenDef("g_1_1_saproling") >= MTG_TOKEN_BASE, "a saproling token");
    CHECK(Mtg_TokenDef("w_1_1_spirit_flying") >= 0 && (Mtg_Card(Mtg_TokenDef("w_1_1_spirit_flying"))->keywords & MTG_KW_BIT(MTG_KW_FLYING)),
          "a flying spirit");
    CHECK(strcmp(Mtg_Card(Mtg_TokenDef("c_1_1_a_gnome"))->name, "Gnome") == 0
          && (Mtg_Card(Mtg_TokenDef("c_1_1_a_gnome"))->types & MTG_T_ARTIFACT), "an artifact gnome");
    Table();
    Lands(0, "Mountain", 5);
    scouts = Put(0, "Goblin Scouts", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, scouts, -1, 0);
    PassBoth();
    for (i = n = 0; i < sD.objCount; i++)
        if (sD.objs[i].def >= MTG_TOKEN_BASE && sD.objs[i].zone == MTG_ZONE_BATTLEFIELD && sD.objs[i].controller == 0)
            n++, tok = i;
    CHECK(n == 3, "three goblins (%d)", n);
    CHECK(tok >= 0 && Mtg_Power(&sD, tok) == 1 && (Mtg_Landwalk(&sD, tok) & MTG_COLOR_BIT(MTG_R)), "1/1 mountainwalk goblin scouts");
    CHECK(tok >= 0 && strcmp(Mtg_Card(sD.objs[tok].def)->name, "Goblin Scout") == 0, "named Goblin Scout");
    Lands(0, "Mountain", 1);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, bolt, MTG_TGT_OBJ, tok);
    PassBoth();
    CHECK(Zone(tok) == MTG_ZONE_NONE, "a bolted token is gone, not in the graveyard (%d)", Zone(tok));

    Table();   // Afterlife: destroy target creature, its controller gets a 1/1 flying spirit
    Lands(0, "Plains", 3);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    after = Put(0, "Afterlife", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, after, MTG_TGT_OBJ, bears);
    PassBoth();
    for (i = n = 0; i < sD.objCount; i++)
        if (sD.objs[i].def >= MTG_TOKEN_BASE && sD.objs[i].zone == MTG_ZONE_BATTLEFIELD && sD.objs[i].controller == 1)
            n++;
    CHECK(Zone(bears) == MTG_ZONE_GRAVEYARD && n == 1, "the bears destroyed, its controller gets a spirit (%d)", n);
}

// counted amounts: the sacrificed creature's power (Fling), domain (Tribal Flames), cards in graveyards
static void TestAmounts(void)
{
    int fling, angel, flames, b;
    printf("amounts: sacrificed power, domain\n");
    Table();
    Lands(0, "Mountain", 2);
    angel = Put(0, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    fling = Put(0, "Fling", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, fling, MTG_TGT_PLAYER, 1);
    ChoosePick(angel);
    PassBoth();
    CHECK(Zone(angel) == MTG_ZONE_GRAVEYARD && sD.players[1].life == 16, "fling the angel: 4 damage (%d)", sD.players[1].life);

    Table();
    Put(0, "Mountain", MTG_ZONE_BATTLEFIELD);
    Put(0, "Forest", MTG_ZONE_BATTLEFIELD);
    Put(0, "Island", MTG_ZONE_BATTLEFIELD);
    b = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD);
    flames = Put(0, "Tribal Flames", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, flames, MTG_TGT_PLAYER, 1);
    PassBoth();
    CHECK(sD.players[1].life == 17, "three basic land types: 3 damage (%d)", sD.players[1].life);
    (void)b;

    Table();   // a */* creature: Pestilence Rats counts the other Rats
    {
        int r1 = Put(0, "Pestilence Rats", MTG_ZONE_BATTLEFIELD), r2;
        CHECK(Mtg_Power(&sD, r1) == 0 && Mtg_Toughness(&sD, r1) == 3, "a lone rat 0/3 (%d/%d)", Mtg_Power(&sD, r1), Mtg_Toughness(&sD, r1));
        r2 = Put(1, "Pestilence Rats", MTG_ZONE_BATTLEFIELD);
        CHECK(Mtg_Power(&sD, r1) == 1 && Mtg_Toughness(&sD, r1) == 3, "two rats: each 1/3 (%d/%d)", Mtg_Power(&sD, r1),
              Mtg_Toughness(&sD, r1));
        CHECK(strcmp(Mtg_PTText(Mtg_Card(sD.objs[r1].def)), "*/3") == 0, "printed */3");
        (void)r2;
    }

    Table();   // a "choose one": Blue Elemental Blast counters a red spell or destroys a red permanent, each its own action
    {
        int beb, goblin, bolt;
        Lands(1, "Island", 1);
        Lands(0, "Mountain", 1);
        goblin = Put(0, "Raging Goblin", MTG_ZONE_BATTLEFIELD);
        beb = Put(1, "Blue Elemental Blast", MTG_ZONE_HAND);
        bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
        Do(MTG_ACT_CAST, bolt, MTG_TGT_PLAYER, 1);
        Pass();
        CHECK(Find(MTG_ACT_CAST, beb, MTG_TGT_STACK, bolt) >= 0 && Find(MTG_ACT_CAST, beb, MTG_TGT_OBJ, goblin) >= 0,
              "both modes offered: counter the bolt, or destroy the goblin");
        Do(MTG_ACT_CAST, beb, MTG_TGT_OBJ, goblin);
        PassBoth();
        CHECK(Zone(goblin) == MTG_ZONE_GRAVEYARD, "the goblin destroyed");
        PassBoth();
        CHECK(sD.players[1].life == 17, "the bolt resolves (%d)", sD.players[1].life);
    }
}

static int HandCount(int player);
static void Choose(int card);

// Cycling (Forge's keyword: "{cost}, discard this card: draw a card", from the hand, at instant speed)
static void TestCycling(void)
{
    int sandbar, hand0;
    Table();
    CHECK(Mtg_CardByName("Lonely Sandbar") >= 0 && Mtg_Card(Mtg_CardByName("Lonely Sandbar"))->supported,
          "Lonely Sandbar plays");
    Lands(0, "Island", 1);
    sandbar = Put(0, "Lonely Sandbar", MTG_ZONE_HAND);
    hand0 = HandCount(0);
    CHECK(Find(MTG_ACT_ACTIVATE, sandbar, MTG_TGT_NONE, 0) >= 0, "cycling offered from the hand");
    Do(MTG_ACT_ACTIVATE, sandbar, MTG_TGT_NONE, 0);
    CHECK(Zone(sandbar) == MTG_ZONE_GRAVEYARD, "discarded as the cost");
    PassBoth();
    CHECK(HandCount(0) == hand0, "drew a card for it (%d, was %d)", HandCount(0), hand0);
    Table();   // no blue mana: no cycling; on the battlefield it's a land, not a cycler
    sandbar = Put(0, "Lonely Sandbar", MTG_ZONE_HAND);
    CHECK(Find(MTG_ACT_ACTIVATE, sandbar, MTG_TGT_NONE, 0) < 0, "no mana: no cycling");
    Table();   // "when you cycle CARDNAME": Renewed Faith's 2 life
    {
        int faith, life;
        CHECK(Mtg_CardByName("Renewed Faith") >= 0 && Mtg_Card(Mtg_CardByName("Renewed Faith"))->supported,
              "Renewed Faith plays");
        Lands(0, "Plains", 2);
        faith = Put(0, "Renewed Faith", MTG_ZONE_HAND);
        life = sD.players[0].life;
        Do(MTG_ACT_ACTIVATE, faith, MTG_TGT_NONE, 0);
        if (Mtg_Query(&sD)->kind == MTG_Q_CHOOSE)
            Choose(1);   // (you may gain 2 life: yes)
        PassBoth();
        if (Mtg_Query(&sD)->kind == MTG_Q_CHOOSE)
            Choose(1);
        PassBoth();
        CHECK(sD.players[0].life == life + 2, "cycling it gained 2 life (%d, was %d)", sD.players[0].life, life);
    }
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

// static abilities (mtg_static.c): lords, anthems, keywords and protection granted, the layers
static void TestStatics(void)
{
    int lord, lord2, merfolk, bears, goblin, moon, island, law, bolt, black;
    printf("statics: lords, anthems, landwalk, protection, can't block, conditions\n");

    Table();   // Lord of Atlantis: other Merfolk +1/+1 and islandwalk; itself and non-Merfolk unchanged
    lord = Put(0, "Lord of Atlantis", MTG_ZONE_BATTLEFIELD);
    merfolk = Put(0, "Merfolk of the Pearl Trident", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    CHECK(Mtg_Power(&sD, merfolk) == 2 && Mtg_Toughness(&sD, merfolk) == 2, "the lord pumps another merfolk (%d/%d)",
          Mtg_Power(&sD, merfolk), Mtg_Toughness(&sD, merfolk));
    CHECK(Mtg_Power(&sD, lord) == 2 && Mtg_Toughness(&sD, lord) == 2, "not itself");
    CHECK(Mtg_Power(&sD, bears) == 2, "not a bear");
    CHECK(Mtg_CanBlock(&sD, bears, merfolk), "no island: the bears block the merfolk");
    island = Put(1, "Island", MTG_ZONE_BATTLEFIELD);
    CHECK(!Mtg_CanBlock(&sD, bears, merfolk), "an island: islandwalk");
    CHECK(Mtg_CanBlock(&sD, bears, lord), "the lord itself has no islandwalk");
    lord2 = Put(0, "Lord of Atlantis", MTG_ZONE_BATTLEFIELD);
    CHECK(Mtg_Power(&sD, merfolk) == 3 && Mtg_Power(&sD, lord) == 3 && Mtg_Power(&sD, lord2) == 3,
          "two lords pump each other (%d %d %d)", Mtg_Power(&sD, merfolk), Mtg_Power(&sD, lord), Mtg_Power(&sD, lord2));
    Mtg_MoveTo(&sD, lord, MTG_ZONE_GRAVEYARD);
    Mtg_MoveTo(&sD, lord2, MTG_ZONE_GRAVEYARD);
    CHECK(Mtg_Power(&sD, merfolk) == 1, "the lords gone: back to 1/1");
    (void)island;

    Table();   // Bad Moon: black creatures, either side; a pump stacks on it; Goblin King's mountainwalk
    moon = Put(0, "Bad Moon", MTG_ZONE_BATTLEFIELD);
    black = Put(1, "Drudge Skeletons", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    CHECK(Mtg_Power(&sD, black) == 2 && Mtg_Toughness(&sD, black) == 2, "bad moon pumps the opponent's black creature too (%d/%d)",
          Mtg_Power(&sD, black), Mtg_Toughness(&sD, black));
    CHECK(Mtg_Power(&sD, bears) == 2, "not the green bears");
    sD.objs[black].tempPower = 2;
    CHECK(Mtg_Power(&sD, black) == 4, "a pump on top (%d)", Mtg_Power(&sD, black));
    goblin = Put(0, "Raging Goblin", MTG_ZONE_BATTLEFIELD);
    Put(0, "Goblin King", MTG_ZONE_BATTLEFIELD);
    Put(1, "Mountain", MTG_ZONE_BATTLEFIELD);
    CHECK(Mtg_Power(&sD, goblin) == 2 && !Mtg_CanBlock(&sD, bears, goblin), "goblin king: +1/+1 and mountainwalk");
    (void)moon;

    Table();   // Absolute Law: every creature has protection from red; the bolt can't target them
    law = Put(1, "Absolute Law", MTG_ZONE_BATTLEFIELD);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Lands(0, "Mountain", 1);
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    CHECK((Mtg_Protection(&sD, bears) & MTG_COLOR_BIT(MTG_R)) != 0, "protection from red");
    CHECK(!Mtg_CanTarget(&sD, bears, bolt), "the bolt can't target the bears");
    (void)law;

    Table();   // can't block, can't be blocked by, a static on itself only while it's untapped
    {
        int glider = Put(1, "Aesthir Glider", MTG_ZONE_BATTLEFIELD), kithkin = Put(0, "Amrou Kithkin", MTG_ZONE_BATTLEFIELD);
        int angel = Put(1, "Serra Angel", MTG_ZONE_BATTLEFIELD), tortoise;
        bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        CHECK(!Mtg_CanBlock(&sD, glider, kithkin), "aesthir glider can't block");
        CHECK(!Mtg_CanBlock(&sD, angel, kithkin) && Mtg_CanBlock(&sD, bears, kithkin),
              "amrou kithkin: not by power 3 or more (the angel), the bears may");
        Put(0, "Bedlam", MTG_ZONE_BATTLEFIELD);
        CHECK(!Mtg_CanBlock(&sD, bears, kithkin), "bedlam: no creature can block");
        tortoise = Put(0, "Giant Tortoise", MTG_ZONE_BATTLEFIELD);
        CHECK(Mtg_Toughness(&sD, tortoise) == 4, "giant tortoise untapped: 1/4 (%d)", Mtg_Toughness(&sD, tortoise));
        sD.objs[tortoise].tapped = 1;
        CHECK(Mtg_Toughness(&sD, tortoise) == 1, "tapped: 1/1");
    }

    Table();   // RaiseCost / ReduceCost: chill makes red spells cost {2} more; a soldier spell {1} less under daru warchief
    {
        MtgManaCost cost;
        const MtgQuery *q;
        int k, castable = 0, soldier;
        Put(1, "Chill", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Mountain", 1);
        bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
        Mtg_SpellCost(&sD, 0, bolt, &cost);
        CHECK(cost.generic == 2 && cost.pip[MTG_R] == 1, "chill: the bolt costs {2}{R} (%d, %d)", cost.generic, cost.pip[MTG_R]);
        q = Mtg_Query(&sD);
        for (k = 0; k < q->actionCount; k++)
            castable |= q->actions[k].kind == MTG_ACT_CAST && q->actions[k].obj == bolt;
        CHECK(!castable, "one mountain can't cast it any more");
        Put(0, "Daru Warchief", MTG_ZONE_BATTLEFIELD);
        soldier = Put(0, "Standing Troops", MTG_ZONE_HAND);
        Mtg_SpellCost(&sD, 0, soldier, &cost);
        CHECK(cost.generic == 1 && cost.pip[MTG_W] == 1, "daru warchief: standing troops cost {1}{W} (%d)", cost.generic);
    }

    Table();   // Condition$ Threshold (seven cards in the graveyard), UnlessDefender$ (can't attack unless ...), Secondary$
    {
        int rip = Put(0, "Anurid Barkripper", MTG_ZONE_BATTLEFIELD), shark = Put(0, "Hammerhead Shark", MTG_ZONE_BATTLEFIELD);
        int brawl = Put(0, "Branded Brawlers", MTG_ZONE_BATTLEFIELD), rage, k, land;
        CHECK(Mtg_Power(&sD, rip) == 2, "barkripper without threshold: 2 power (%d)", Mtg_Power(&sD, rip));
        for (k = 0; k < 6; k++)
            Put(0, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
        CHECK(Mtg_Power(&sD, rip) == 2, "six cards: still 2");
        Put(1, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
        CHECK(Mtg_Power(&sD, rip) == 2, "the opponent's graveyard doesn't count");
        Put(0, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
        CHECK(Mtg_Power(&sD, rip) == 4 && Mtg_Toughness(&sD, rip) == 4, "threshold: 4/4 (%d/%d)", Mtg_Power(&sD, rip),
              Mtg_Toughness(&sD, rip));
        CHECK(Mtg_StaticForbids(&sD, MTG_STATIC_CANT_ATTACK, shark, -1), "hammerhead shark: no island over there, can't attack");
        Put(1, "Island", MTG_ZONE_BATTLEFIELD);
        CHECK(!Mtg_StaticForbids(&sD, MTG_STATIC_CANT_ATTACK, shark, -1), "an island: it may attack");
        CHECK(Mtg_StaticForbids(&sD, MTG_STATIC_CANT_ATTACK, brawl, -1), "branded brawlers: the defender has an untapped land");
        for (k = 0; k < sD.objCount; k++)
            if (sD.objs[k].zone == MTG_ZONE_BATTLEFIELD && sD.objs[k].controller == 1)
                sD.objs[k].tapped = 1;
        CHECK(!Mtg_StaticForbids(&sD, MTG_STATIC_CANT_ATTACK, brawl, -1), "their lands tapped: it may attack");
        rage = Put(0, "Maniacal Rage", MTG_ZONE_BATTLEFIELD);   // an Aura with Secondary$: +2/+2, can't block
        land = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        sD.objs[rage].attachedTo = (int16_t)land;
        CHECK(Mtg_Power(&sD, land) == 4 && !Mtg_CanBlock(&sD, land, shark), "maniacal rage: 4/4 and can't block (%d)",
              Mtg_Power(&sD, land));
    }
}

// the action casting OBJ for its alternative cost (target any), or -1
static int FindAlt(int obj)
{
    const MtgQuery *q;
    int i;
    Mtg_RefreshQuery(&sD);
    q = Mtg_Query(&sD);
    for (i = 0; i < q->actionCount; i++)
        if (q->actions[i].kind == MTG_ACT_CAST && q->actions[i].obj == obj && q->actions[i].alt)
            return i;
    return -1;
}

// alternative costs (StaticAbilityAlternativeCost) and the cost parts they use: Sac, Return, ExileFromHand
static void TestAltCosts(void)
{
    MtgAnswer a;
    int crash, mountain, bird, gush, isl1, isl2, cave, bolt, bears, k, n0;
    printf("alternative costs: crash, gush, cave-in\n");

    Table();   // CRASH: sacrifice a Mountain instead of {2}{R}; one Mountain can't pay the mana
    mountain = Put(0, "Mountain", MTG_ZONE_BATTLEFIELD);
    crash = Put(0, "Crash", MTG_ZONE_HAND);
    bird = Put(1, "Ornithopter", MTG_ZONE_BATTLEFIELD);
    CHECK(Find(MTG_ACT_CAST, crash, -1, 0) < 0 || sD.query.actions[Find(MTG_ACT_CAST, crash, -1, 0)].alt,
          "no mana cast with one mountain");
    k = FindAlt(crash);
    CHECK(k >= 0, "crash may be cast by sacrificing a mountain");
    if (k >= 0) {
        memset(&a, 0, sizeof(a));
        a.action = (uint16_t)k;
        Answer(&a);
        CHECK(Mtg_Query(&sD)->kind == MTG_Q_CHOOSE && Mtg_Query(&sD)->forCost == 1, "asked what to sacrifice");
        ChoosePick(mountain);
        CHECK(Zone(mountain) == MTG_ZONE_GRAVEYARD && !sD.objs[mountain].tapped, "the mountain sacrificed");
        PassBoth();
        CHECK(Zone(bird) == MTG_ZONE_GRAVEYARD, "the ornithopter destroyed (%d)", Zone(bird));
    }

    Table();   // GUSH: return two Islands instead of {4}{U}; draws two
    isl1 = Put(0, "Island", MTG_ZONE_BATTLEFIELD);
    isl2 = Put(0, "Island", MTG_ZONE_BATTLEFIELD);
    gush = Put(0, "Gush", MTG_ZONE_HAND);
    Put(0, "Grizzly Bears", MTG_ZONE_LIBRARY);
    Put(0, "Grizzly Bears", MTG_ZONE_LIBRARY);
    k = FindAlt(gush);
    CHECK(k >= 0, "gush may be cast by returning two islands");
    if (k >= 0) {
        memset(&a, 0, sizeof(a));
        a.action = (uint16_t)k;
        Answer(&a);
        CHECK(Mtg_Query(&sD)->forCost == 4, "asked what to return (%d)", Mtg_Query(&sD)->forCost);
        ChoosePick(isl1);
        ChoosePick(isl2);
        CHECK(Zone(isl1) == MTG_ZONE_HAND && Zone(isl2) == MTG_ZONE_HAND, "both islands back in the hand");
        n0 = HandCount(0);
        PassBoth();
        CHECK(HandCount(0) == n0 + 2, "drew two (%d -> %d)", n0, HandCount(0));
    }

    Table();   // CAVE-IN: exile a red card from the hand instead of {3}{R}{R}; 2 damage to each creature and player
    cave = Put(0, "Cave-In", MTG_ZONE_HAND);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    CHECK(FindAlt(cave) < 0, "no other red card: no free cast");
    bolt = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
    k = FindAlt(cave);
    CHECK(k >= 0, "a bolt to exile: cave-in may be cast");
    if (k >= 0) {
        memset(&a, 0, sizeof(a));
        a.action = (uint16_t)k;
        Answer(&a);
        CHECK(Mtg_Query(&sD)->forCost == 3, "asked what to exile (%d)", Mtg_Query(&sD)->forCost);
        ChoosePick(bolt);
        CHECK(Zone(bolt) == MTG_ZONE_EXILE, "the bolt exiled");
        PassBoth();
        CHECK(Zone(bears) == MTG_ZONE_GRAVEYARD && sD.players[0].life == 18 && sD.players[1].life == 18,
              "2 to everything (bears %d, life %d / %d)", Zone(bears), sD.players[0].life, sD.players[1].life);
    }
    // MustAttack: BLOODROCK CYCLOPS attacks though the answer left it out; CantTarget: DENSE FOLIAGE keeps creatures
    // from being a spell's target (the player still is)
    Table();
    {
        int cyclops = Put(0, "Bloodrock Cyclops", MTG_ZONE_BATTLEFIELD);
        sD.objs[cyclops].sick = 0;
        Attack(0);
        CHECK(sD.objs[cyclops].attacking, "bloodrock cyclops attacks each combat if able");
    }
    Table();
    {
        int foliage = Put(1, "Dense Foliage", MTG_ZONE_BATTLEFIELD), bolt2;
        bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Mountain", 1);
        bolt2 = Put(0, "Lightning Bolt", MTG_ZONE_HAND);
        CHECK(Find(MTG_ACT_CAST, bolt2, MTG_TGT_OBJ, bears) < 0, "dense foliage: the bolt can't target the bears");
        CHECK(Find(MTG_ACT_CAST, bolt2, MTG_TGT_PLAYER, 1) >= 0, "the opponent still can be");
        (void)foliage;
    }
}

// abilities and triggers a static gives (AddAbility$ / AddTrigger$)
static void TestGranted(void)
{
    int bears, aura, growth, flux, crusade, i;
    printf("granted abilities\n");
    Table();   // ARCANE TEACHINGS: enchanted creature has "{T}: 1 damage to any target"
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    sD.objs[bears].sick = 0;
    CHECK(Find(MTG_ACT_ACTIVATE, bears, MTG_TGT_PLAYER, 1) < 0, "no ability before the Aura");
    aura = Put(0, "Arcane Teachings", MTG_ZONE_BATTLEFIELD);
    sD.objs[aura].attachedTo = (uint16_t)bears;
    CHECK(Mtg_Power(&sD, bears) == 4, "+2/+2 (power %d)", Mtg_Power(&sD, bears));
    CHECK(Find(MTG_ACT_ACTIVATE, bears, MTG_TGT_PLAYER, 1) >= 0, "the bears can ping");
    Do(MTG_ACT_ACTIVATE, bears, MTG_TGT_PLAYER, 1);
    PassBoth();
    CHECK(sD.players[1].life == 19 && sD.objs[bears].tapped, "1 damage, the bears tapped (life %d)", sD.players[1].life);

    Table();   // CITANUL HIEROPHANTS: creatures you control have "{T}: Add {G}"
    Put(0, "Citanul Hierophants", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    growth = Put(0, "Giant Growth", MTG_ZONE_HAND);
    for (i = 0; i < sD.objCount; i++)
        if (sD.objs[i].zone == MTG_ZONE_BATTLEFIELD)
            sD.objs[i].sick = 1;
    CHECK(Find(MTG_ACT_CAST, growth, MTG_TGT_OBJ, bears) < 0, "summoning sick: no mana yet");
    for (i = 0; i < sD.objCount; i++)
        sD.objs[i].sick = 0;
    CHECK(Find(MTG_ACT_CAST, growth, MTG_TGT_OBJ, bears) >= 0, "the creatures make green mana");

    Table();   // AURA FLUX: other enchantments have "at the beginning of your upkeep, sacrifice this unless you pay {2}"
    flux = Put(1, "Aura Flux", MTG_ZONE_BATTLEFIELD);
    crusade = Put(1, "Crusade", MTG_ZONE_BATTLEFIELD);
    GoTo(1, MTG_STEP_DRAW);
    CHECK(Zone(crusade) == MTG_ZONE_GRAVEYARD && Zone(flux) == MTG_ZONE_BATTLEFIELD,
          "no mana: the Crusade sacrificed, the Flux stays (%d %d)", Zone(crusade), Zone(flux));

    Table();   // ASPECT OF WOLF: +X/+Y, half the Forests rounded down / up; EMPYRIAL ARMOR: +1/+1 a card in hand
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    aura = Put(0, "Aspect of Wolf", MTG_ZONE_BATTLEFIELD);
    sD.objs[aura].attachedTo = (uint16_t)bears;
    Lands(0, "Forest", 3);
    CHECK(Mtg_Power(&sD, bears) == 3 && Mtg_Toughness(&sD, bears) == 4, "three Forests: +1/+2 (%d/%d)",
          Mtg_Power(&sD, bears), Mtg_Toughness(&sD, bears));
    aura = Put(0, "Empyrial Armor", MTG_ZONE_BATTLEFIELD);
    sD.objs[aura].attachedTo = (uint16_t)bears;
    Put(0, "Forest", MTG_ZONE_HAND);
    Put(0, "Forest", MTG_ZONE_HAND);
    CHECK(Mtg_Power(&sD, bears) == 5, "two cards in hand: +2/+2 more (%d)", Mtg_Power(&sD, bears));

    Table();   // PSYCHATOG: "Exile two cards from your graveyard: +1/+1 until end of turn"
    {
        int tog = Put(0, "Psychatog", MTG_ZONE_BATTLEFIELD), g1, g2, k, act = -1;
        const MtgQuery *q;
        g1 = Put(0, "Forest", MTG_ZONE_GRAVEYARD);
        g2 = Put(0, "Island", MTG_ZONE_GRAVEYARD);
        q = Mtg_Query(&sD);
        Mtg_RefreshQuery(&sD);
        for (k = 0; k < q->actionCount; k++)
            if (q->actions[k].kind == MTG_ACT_ACTIVATE && q->actions[k].obj == tog
                && gMtgAbilities[q->actions[k].ability].cost.exileGraveN == 2)
                act = k;
        CHECK(act >= 0, "two in the graveyard: the exile ability is there");
        if (act >= 0) {
            MtgAnswer a;
            memset(&a, 0, sizeof(a));
            a.action = (uint16_t)act;
            Answer(&a);
            for (k = 0; k < 2 && Mtg_Query(&sD)->kind == MTG_Q_CHOOSE; k++)
                ChoosePick(k == 0 ? g1 : g2);
            PassBoth();
            CHECK(Zone(g1) == MTG_ZONE_EXILE && Zone(g2) == MTG_ZONE_EXILE && Mtg_Power(&sD, tog) == 2,
                  "both exiled, the tog 2/3 (%d %d, power %d)", Zone(g1), Zone(g2), Mtg_Power(&sD, tog));
        }
    }

    Table();   // ARREST: the enchanted creature's abilities can't be activated; ANGER in the graveyard: haste
    {
        int tim = Put(0, "Prodigal Sorcerer", MTG_ZONE_BATTLEFIELD), arrest;
        sD.objs[tim].sick = 0;
        CHECK(Find(MTG_ACT_ACTIVATE, tim, MTG_TGT_PLAYER, 1) >= 0, "the sorcerer pings");
        arrest = Put(1, "Arrest", MTG_ZONE_BATTLEFIELD);
        sD.objs[arrest].attachedTo = (uint16_t)tim;
        CHECK(Find(MTG_ACT_ACTIVATE, tim, MTG_TGT_PLAYER, 1) < 0, "arrested: no ping");
    }
    Table();
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Put(0, "Anger", MTG_ZONE_GRAVEYARD);
    CHECK(!(Mtg_Keywords(&sD, bears) & MTG_KW_BIT(MTG_KW_HASTE)), "no Mountain: no haste");
    Put(0, "Mountain", MTG_ZONE_BATTLEFIELD);
    CHECK(Mtg_Keywords(&sD, bears) & MTG_KW_BIT(MTG_KW_HASTE), "Anger in the graveyard and a Mountain: haste");

    Table();   // CONTROL MAGIC: cast on their bears, the bears are ours; the Aura gone, they go back
    {
        int magic;
        bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Island", 4);
        magic = Put(0, "Control Magic", MTG_ZONE_HAND);
        Do(MTG_ACT_CAST, magic, MTG_TGT_OBJ, bears);
        PassBoth();
        CHECK(sD.objs[bears].controller == 0 && sD.objs[bears].sick, "the bears are ours (and sick)");
        Mtg_MoveTo(&sD, magic, MTG_ZONE_GRAVEYARD);
        Pass();
        CHECK(sD.objs[bears].controller == 1, "the Aura gone: back to their owner (%d)", sD.objs[bears].controller);
    }

    Table();   // AVATAR OF WOE: {6} less with ten creature cards in graveyards; CHO-ARRIM LEGATE free against a Swamp
    {
        int woe = Put(0, "Avatar of Woe", MTG_ZONE_HAND), legate, k, alt = 0;
        MtgManaCost cost;
        const MtgQuery *q;
        Mtg_SpellCost(&sD, 0, woe, &cost);
        CHECK(cost.generic == 6, "an empty graveyard: full cost (%d)", cost.generic);
        for (k = 0; k < 10; k++)
            Put(k & 1, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
        Mtg_SpellCost(&sD, 0, woe, &cost);
        CHECK(cost.generic == 0 && cost.pip[MTG_B] == 2, "ten creatures there: BB (%d)", cost.generic);
        legate = Put(0, "Cho-Arrim Legate", MTG_ZONE_HAND);
        Put(0, "Plains", MTG_ZONE_BATTLEFIELD);
        Mtg_RefreshQuery(&sD);
        q = Mtg_Query(&sD);
        for (k = 0; k < q->actionCount; k++)
            alt |= q->actions[k].obj == legate && q->actions[k].alt;
        CHECK(!alt, "no Swamp across the table: no free cast");
        Put(1, "Swamp", MTG_ZONE_BATTLEFIELD);
        Mtg_RefreshQuery(&sD);
        for (k = 0; k < q->actionCount; k++)
            alt |= q->actions[k].obj == legate && q->actions[k].alt;
        CHECK(alt, "their Swamp and our Plains: the Legate for free");
    }
}

// spells with several targets (TargetMin$ / TargetMax$): the first with the spell, the rest picked after
static void TestTargets(void)
{
    int swelter, b1, b2, b3, tethers, hope;
    const MtgQuery *q;
    MtgAnswer a;
    printf("several targets\n");
    Table();   // SWELTER: 2 damage to each of two target creatures (one creature: it can't be cast)
    b1 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Lands(0, "Mountain", 4);
    swelter = Put(0, "Swelter", MTG_ZONE_HAND);
    CHECK(Find(MTG_ACT_CAST, swelter, MTG_TGT_OBJ, b1) < 0, "one creature: no Swelter");
    b2 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, swelter, MTG_TGT_OBJ, b1);
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->forCost == 7 && q->candidateCount == 1 && q->candidates[0] == b2,
          "asked for the second target (the other bears)");
    ChoosePick(b2);
    PassBoth();
    CHECK(Zone(b1) == MTG_ZONE_GRAVEYARD && Zone(b2) == MTG_ZONE_GRAVEYARD, "both bears dead (%d %d)", Zone(b1), Zone(b2));

    Table();   // CHOKING TETHERS: tap up to four; stopping after two
    b1 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    b2 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    b3 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Lands(0, "Island", 4);
    tethers = Put(0, "Choking Tethers", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, tethers, MTG_TGT_OBJ, b1);
    ChoosePick(b2);
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->forCost == 7, "a third may be picked");
    memset(&a, 0, sizeof(a));
    a.card = q->candidateCount;   // (done)
    Answer(&a);
    PassBoth();
    CHECK(sD.objs[b1].tapped && sD.objs[b2].tapped && !sD.objs[b3].tapped, "two tapped, the third not");

    Table();   // HOPE AND GLORY: untap two target creatures, each gets +1/+1 (the sub-ability on both)
    b1 = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    b2 = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    sD.objs[b1].tapped = sD.objs[b2].tapped = 1;
    Lands(0, "Plains", 2);
    hope = Put(0, "Hope and Glory", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, hope, MTG_TGT_OBJ, b1);
    ChoosePick(b2);
    PassBoth();
    CHECK(!sD.objs[b1].tapped && !sD.objs[b2].tapped && Mtg_Power(&sD, b1) == 3 && Mtg_Power(&sD, b2) == 3,
          "both untapped and 3/3 (%d %d)", Mtg_Power(&sD, b1), Mtg_Power(&sD, b2));

    Table();   // DREGS OF SORROW (X 4 B): seven Swamps make X = 2: two of the three bears destroyed, two cards drawn
    {
        int dregs, hand, k;
        b1 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        b2 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        b3 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Swamp", 7);
        dregs = Put(0, "Dregs of Sorrow", MTG_ZONE_HAND);
        k = Find(MTG_ACT_CAST, dregs, MTG_TGT_OBJ, b1);
        CHECK(k >= 0 && Mtg_Query(&sD)->actions[k].x == 2, "X = 2");
        hand = HandCount(0);
        Do(MTG_ACT_CAST, dregs, MTG_TGT_OBJ, b1);
        ChoosePick(b2);
        CHECK(Mtg_Query(&sD)->kind == MTG_Q_PRIORITY, "two targets for X = 2: no third asked");
        PassBoth();
        CHECK(Zone(b1) == MTG_ZONE_GRAVEYARD && Zone(b2) == MTG_ZONE_GRAVEYARD && Zone(b3) == MTG_ZONE_BATTLEFIELD,
              "two destroyed, one left");
        CHECK(HandCount(0) == hand - 1 + 2, "two cards drawn (%d -> %d)", hand, HandCount(0));
    }
}

// searches (ChangeZone with ChangeType$: Hidden, the library or the hand)
static void TestSearch(void)
{
    int growth, lands, piper, wurm, i, found = -1;
    printf("searches\n");
    Table();   // RAMPANT GROWTH: a basic land from the library onto the battlefield tapped (the table's libraries: Plains)
    Lands(0, "Forest", 2);
    growth = Put(0, "Rampant Growth", MTG_ZONE_HAND);
    lands = sD.players[0].libraryCount;
    Do(MTG_ACT_CAST, growth, -1, 0);
    PassBoth();
    for (i = 0; i < sD.objCount; i++)
        if (sD.objs[i].zone == MTG_ZONE_BATTLEFIELD && sD.objs[i].controller == 0
            && strcmp(Mtg_Card(sD.objs[i].def)->name, "Plains") == 0)
            found = i;
    CHECK(found >= 0 && sD.objs[found].tapped, "a Plains found, onto the battlefield tapped");
    CHECK(sD.players[0].libraryCount == lands - 1, "one card fewer in the library (%d)", sD.players[0].libraryCount);

    Table();   // ELVISH PIPER: {G}, {T}: a creature card from the hand onto the battlefield (the only one: no question)
    Lands(0, "Forest", 1);
    piper = Put(0, "Elvish Piper", MTG_ZONE_BATTLEFIELD);
    sD.objs[piper].sick = 0;
    wurm = Put(0, "Craw Wurm", MTG_ZONE_HAND);
    Do(MTG_ACT_ACTIVATE, piper, -1, 0);
    PassBoth();
    CHECK(Zone(wurm) == MTG_ZONE_BATTLEFIELD && sD.objs[wurm].controller == 0, "the wurm is on the battlefield");

    Table();   // EVACUATION: every creature back to its owner's hand (lands stay)
    {
        int evac, b1, b2, land;
        b1 = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        b2 = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Island", 5);
        land = Put(1, "Forest", MTG_ZONE_BATTLEFIELD);
        evac = Put(0, "Evacuation", MTG_ZONE_HAND);
        Do(MTG_ACT_CAST, evac, -1, 0);
        PassBoth();
        CHECK(Zone(b1) == MTG_ZONE_HAND && Zone(b2) == MTG_ZONE_HAND && Zone(land) == MTG_ZONE_BATTLEFIELD,
              "both bears in hand, the Forest stays (%d %d %d)", Zone(b1), Zone(b2), Zone(land));
    }
    Table();   // FELDON'S CANE: {T}, exile it: shuffle your graveyard into your library
    {
        int cane = Put(0, "Feldon's Cane", MTG_ZONE_BATTLEFIELD), g1, g2, before = sD.players[0].libraryCount;
        g1 = Put(0, "Grizzly Bears", MTG_ZONE_GRAVEYARD);
        g2 = Put(0, "Lightning Bolt", MTG_ZONE_GRAVEYARD);
        Do(MTG_ACT_ACTIVATE, cane, -1, 0);
        PassBoth();
        CHECK(Zone(g1) == MTG_ZONE_LIBRARY && Zone(g2) == MTG_ZONE_LIBRARY && sD.players[0].libraryCount == before + 2
              && Zone(cane) == MTG_ZONE_EXILE, "the graveyard in the library, the cane exiled");
    }

    Table();   // ADARKAR WASTES: {T}: {C}, or {W}/{U} and 1 damage to you
    {
        int lions = Put(0, "Savannah Lions", MTG_ZONE_HAND), tome;
        Put(0, "Adarkar Wastes", MTG_ZONE_BATTLEFIELD);
        Do(MTG_ACT_CAST, lions, -1, 0);
        CHECK(sD.players[0].life == 19, "white from the Wastes: 1 damage (life %d)", sD.players[0].life);
        PassBoth();
        Table();
        Lands(0, "Adarkar Wastes", 2);
        Put(0, "Ancient Tomb", MTG_ZONE_BATTLEFIELD);
        tome = Put(0, "Jalum Tome", MTG_ZONE_HAND);   // ({3}: the Tomb's two and the Wastes' colorless)
        Do(MTG_ACT_CAST, tome, -1, 0);
        CHECK(sD.players[0].life == 18, "generic: the Tomb's 2 damage, the Wastes' colorless free (life %d)",
              sD.players[0].life);
    }

    Table();   // UNSEEN WALKER: forestwalk until end of turn; JAMURAAN LION: target creature can't block this turn
    {
        int walker = Put(0, "Unseen Walker", MTG_ZONE_BATTLEFIELD), lion = Put(0, "Jamuraan Lion", MTG_ZONE_BATTLEFIELD), wall;
        sD.objs[walker].sick = sD.objs[lion].sick = 0;
        wall = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Forest", 3);
        Lands(0, "Plains", 1);
        Do(MTG_ACT_ACTIVATE, walker, MTG_TGT_OBJ, walker);
        PassBoth();
        CHECK(Mtg_Landwalk(&sD, walker) & MTG_COLOR_BIT(MTG_G), "forestwalk");
        Do(MTG_ACT_ACTIVATE, lion, MTG_TGT_OBJ, wall);
        PassBoth();
        sD.objs[walker].attacking = 1;
        CHECK(!Mtg_CanBlock(&sD, wall, walker), "the bears can't block this turn");
    }

    Table();   // TREETOP VILLAGE: {1}{G}: a 3/3 green Ape with trample until end of turn (still a land)
    {
        int factory = Put(0, "Treetop Village", MTG_ZONE_BATTLEFIELD);
        Lands(0, "Forest", 2);
        CHECK(!(Mtg_ObjTypes(&sD, factory) & MTG_T_CREATURE), "a land, no creature");
        Do(MTG_ACT_ACTIVATE, factory, -1, 0);
        PassBoth();
        CHECK((Mtg_ObjTypes(&sD, factory) & (MTG_T_CREATURE | MTG_T_LAND)) == (MTG_T_CREATURE | MTG_T_LAND)
                  && Mtg_Power(&sD, factory) == 3 && (Mtg_Keywords(&sD, factory) & MTG_KW_BIT(MTG_KW_TRAMPLE))
                  && Mtg_ObjColors(&sD, factory) == MTG_COLOR_BIT(MTG_G),
              "a 3/3 green trampling creature land (power %d)", Mtg_Power(&sD, factory));
        GoTo(1, MTG_STEP_UPKEEP);
        CHECK(!(Mtg_ObjTypes(&sD, factory) & MTG_T_CREATURE), "the next turn: a land again");
    }
}

// UnlessCost$ that isn't only mana (AbilityUtils.handleUnlessCost with a full Cost): returns, discards, damage
static void TestUnless(void)
{
    int atoll, island, horde, card, blister, i;
    const MtgQuery *q;
    static const char *const cards[] = {"Coral Atoll", "Balduvian Horde", "Lava Blister", "Karoo", "Bog Elemental",
                                        "Browbeat", "Masticore", NULL};
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Coral Atoll: sacrificed unless an untapped Island goes back to the hand
    island = Put(0, "Island", MTG_ZONE_BATTLEFIELD);
    atoll = Put(0, "Coral Atoll", MTG_ZONE_HAND);
    Do(MTG_ACT_LAND, atoll, -1, 0);
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_PAY && q->player == 0, "the atoll asks to return an Island");
    Choose(1);
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->forCost == 4 && q->candidateCount == 1 && q->candidates[0] == island,
          "the Island is the one to return");
    ChoosePick(island);
    CHECK(Zone(island) == MTG_ZONE_HAND && Zone(atoll) == MTG_ZONE_BATTLEFIELD, "the Island back, the atoll stays");
    CHECK(Mtg_Query(&sD)->kind == MTG_Q_PRIORITY && sD.stackCount == 0, "and the game goes on");

    Table();   // not paid: sacrificed
    island = Put(0, "Island", MTG_ZONE_BATTLEFIELD);
    atoll = Put(0, "Coral Atoll", MTG_ZONE_HAND);
    Do(MTG_ACT_LAND, atoll, -1, 0);
    PassBoth();
    Choose(0);
    CHECK(Zone(island) == MTG_ZONE_BATTLEFIELD && Zone(atoll) == MTG_ZONE_GRAVEYARD, "not paid: the atoll sacrificed");

    Table();   // no untapped Island: sacrificed without asking
    island = Put(0, "Island", MTG_ZONE_BATTLEFIELD);
    sD.objs[island].tapped = 1;
    atoll = Put(0, "Coral Atoll", MTG_ZONE_HAND);
    Do(MTG_ACT_LAND, atoll, -1, 0);
    PassBoth();
    CHECK(Zone(atoll) == MTG_ZONE_GRAVEYARD && Mtg_Query(&sD)->kind == MTG_Q_PRIORITY, "a tapped Island can't pay");

    Table();   // Balduvian Horde: sacrificed unless a card is discarded at random
    Lands(0, "Mountain", 4);
    horde = Put(0, "Balduvian Horde", MTG_ZONE_HAND);
    card = Put(0, "Grizzly Bears", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, horde, -1, 0);
    PassBoth();
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_PAY, "the horde asks for a discard");
    Choose(1);
    CHECK(Zone(card) == MTG_ZONE_GRAVEYARD && Zone(horde) == MTG_ZONE_BATTLEFIELD, "the bears discarded, the horde stays");

    Table();   // Lava Blister: the land's controller may take 6 damage instead
    Lands(0, "Mountain", 2);
    atoll = Put(1, "Coral Atoll", MTG_ZONE_BATTLEFIELD);
    blister = Put(0, "Lava Blister", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, blister, MTG_TGT_OBJ, atoll);
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_PAY && q->player == 1, "the land's controller is asked");
    Choose(1);
    CHECK(sD.players[1].life == 14 && Zone(atoll) == MTG_ZONE_BATTLEFIELD, "6 damage taken, the land kept");
}

// DelayedTrigger (Mode$ Phase): set up by an effect, fires once when its step begins
static void TestDelayed(void)
{
    int wine, basilisk, wurm, hand, i;
    static const char *const cards[] = {"Blessed Wine", "Thicket Basilisk", "Cinder Wall", "Carrier Pigeons",
                                        "Brood of Cockroaches", "Lifeline", "Wall of Tears", NULL};
    printf("delayed triggers: next upkeep, end of combat\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Blessed Wine: 1 life now, a card at the beginning of the next turn's upkeep
    Lands(0, "Plains", 2);
    wine = Put(0, "Blessed Wine", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, wine, -1, 0);
    PassBoth();
    hand = HandCount(0);
    CHECK(sD.players[0].life == 21 && sD.delayedCount == 1, "1 life, a card to come");
    GoTo(0, MTG_STEP_END);
    CHECK(HandCount(0) == hand, "not this turn");
    GoTo(1, MTG_STEP_DRAW);
    CHECK(HandCount(0) == hand + 1 && sD.delayedCount == 0, "drawn in the next upkeep, once");

    Table();   // Thicket Basilisk: what it blocks is destroyed at end of combat
    wurm = Put(0, "Craw Wurm", MTG_ZONE_BATTLEFIELD);
    basilisk = Put(1, "Thicket Basilisk", MTG_ZONE_BATTLEFIELD);
    Attack(1);
    Block(basilisk, wurm);
    GoTo(0, MTG_STEP_COMBAT_DAMAGE);
    CHECK(Zone(wurm) == MTG_ZONE_BATTLEFIELD, "the wurm lives through the damage");
    GoTo(0, MTG_STEP_MAIN2);
    CHECK(Zone(wurm) == MTG_ZONE_GRAVEYARD && Zone(basilisk) == MTG_ZONE_GRAVEYARD, "destroyed at end of combat");
}

// PreventDamage: "prevent the next n damage that would be dealt to ... this turn" (a shield, used up first)
static void TestPrevent(void)
{
    int clergy, bears, shock, cons, i;
    static const char *const cards[] = {"Clergy en-Vec", "Conservator", "Femeref Healer", "Bandage", "Alabaster Wall",
                                        "Battlefield Medic", NULL};
    printf("prevent damage: shields on creatures and players\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Clergy en-Vec: 1 prevented, so SHOCK leaves the bears at 1 damage
    clergy = Put(0, "Clergy en-Vec", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Lands(0, "Mountain", 1);
    shock = Put(0, "Shock", MTG_ZONE_HAND);
    Do(MTG_ACT_ACTIVATE, clergy, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(sD.objs[bears].prevent == 1, "a 1-damage shield on the bears");
    Do(MTG_ACT_CAST, shock, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Zone(bears) == MTG_ZONE_BATTLEFIELD && sD.objs[bears].damage == 1 && sD.objs[bears].prevent == 0,
          "1 prevented, 1 dealt: the bears live");

    Table();   // Conservator: the next 2 to you
    cons = Put(0, "Conservator", MTG_ZONE_BATTLEFIELD);
    Lands(0, "Mountain", 4);
    shock = Put(0, "Shock", MTG_ZONE_HAND);
    Do(MTG_ACT_ACTIVATE, cons, -1, 0);
    PassBoth();
    CHECK(sD.players[0].prevent == 2, "a 2-damage shield on its player");
    Do(MTG_ACT_CAST, shock, MTG_TGT_PLAYER, 0);
    PassBoth();
    CHECK(sD.players[0].life == 20 && sD.players[0].prevent == 0, "SHOCK to itself prevented");

    Table();   // the shields end with the turn
    clergy = Put(0, "Clergy en-Vec", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_ACTIVATE, clergy, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(sD.objs[bears].prevent == 1, "shield up");
    GoTo(1, MTG_STEP_DRAW);
    CHECK(sD.objs[bears].prevent == 0, "gone at end of turn");
}

// GainControl: Threaten's until end of turn (untapped, haste), Aladdin's for as long as it stays
static void TestControl(void)
{
    int threaten, bears, aladdin, thopter, shock, i;
    static const char *const cards[] = {"Threaten", "Aladdin", "Jinxed Idol", "Helm of Possession", "Sleeper Agent",
                                        "Rubinia Soulsinger", NULL};
    printf("gain control: until end of turn, while its source stays\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Threaten: theirs until end of turn, untapped, with haste
    Lands(0, "Mountain", 3);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    sD.objs[bears].tapped = 1;
    threaten = Put(0, "Threaten", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, threaten, MTG_TGT_OBJ, bears);
    PassBoth();
    Mtg_RefreshQuery(&sD);
    CHECK(sD.objs[bears].controller == 0 && !sD.objs[bears].tapped
          && (Mtg_Keywords(&sD, bears) & MTG_KW_BIT(MTG_KW_HASTE)), "ours, untapped, hasty");
    GoTo(1, MTG_STEP_DRAW);
    CHECK(sD.objs[bears].controller == 1, "back to them after the turn");

    Table();   // Aladdin: their artifact for as long as we control Aladdin
    Lands(0, "Mountain", 4);
    aladdin = Put(0, "Aladdin", MTG_ZONE_BATTLEFIELD);
    thopter = Put(1, "Ornithopter", MTG_ZONE_BATTLEFIELD);
    shock = Put(0, "Shock", MTG_ZONE_HAND);
    Do(MTG_ACT_ACTIVATE, aladdin, MTG_TGT_OBJ, thopter);
    PassBoth();
    Mtg_RefreshQuery(&sD);
    CHECK(sD.objs[thopter].controller == 0, "the Ornithopter is ours");
    Do(MTG_ACT_CAST, shock, MTG_TGT_OBJ, aladdin);
    PassBoth();
    Mtg_RefreshQuery(&sD);
    CHECK(Zone(aladdin) == MTG_ZONE_GRAVEYARD && sD.objs[thopter].controller == 1, "Aladdin gone: it goes back");
}

// counters of every kind: etbCounter, PutCounter of a named kind, SubCounter costs, RemoveCounter, counters_ specs
static void TestCounters(void)
{
    int feeder, arrows, bears, i;
    static const char *const cards[] = {"Spike Feeder", "Serrated Arrows", "Triskelion", "Lotus Blossom", "Iceberg",
                                        "Bounty Hunter", "Time Bomb", NULL};
    printf("counters: enters with, named kinds, removed as a cost\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Spike Feeder: enters with two +1/+1 counters; remove one: gain 2 life
    Lands(0, "Forest", 3);
    feeder = Put(0, "Spike Feeder", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, feeder, -1, 0);
    PassBoth();
    CHECK(Zone(feeder) == MTG_ZONE_BATTLEFIELD && Mtg_Counters(&sD, feeder, MTG_CTR_P1P1) == 2
          && Mtg_Power(&sD, feeder) == 2, "a 2/2 with two counters");
    Do(MTG_ACT_ACTIVATE, feeder, -1, 0);
    PassBoth();
    CHECK(Mtg_Counters(&sD, feeder, MTG_CTR_P1P1) == 1 && sD.players[0].life == 22, "one counter off, 2 life");

    Table();   // Serrated Arrows: three arrowhead counters, each one a -1/-1 counter on a creature
    Lands(0, "Plains", 4);
    arrows = Put(0, "Serrated Arrows", MTG_ZONE_HAND);
    bears = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Do(MTG_ACT_CAST, arrows, -1, 0);
    PassBoth();
    CHECK(Mtg_Counters(&sD, arrows, Mtg_CounterKind("ARROWHEAD")) == 3, "three arrowhead counters");
    Do(MTG_ACT_ACTIVATE, arrows, MTG_TGT_OBJ, bears);
    PassBoth();
    CHECK(Mtg_Counters(&sD, arrows, Mtg_CounterKind("ARROWHEAD")) == 2 && Mtg_Power(&sD, bears) == 1
          && Mtg_Toughness(&sD, bears) == 1, "one spent: the bears are 1/1");
    CHECK(Mtg_ValidObj(&sD, arrows, "Artifact.counters_GE2_ARROWHEAD", arrows, 0)
          && !Mtg_ValidObj(&sD, arrows, "Artifact.counters_GE3_ARROWHEAD", arrows, 0), "counters_ specs count them");
}

// a card put in its player's library at the bottom (the test's own set-up: Put leaves libraries alone)
static int PutLibBottom(int player, const char *name)
{
    int obj = Put(player, name, MTG_ZONE_LIBRARY);
    MtgPlayer *pl = &sD.players[player];
    memmove(&pl->library[1], &pl->library[0], pl->libraryCount * sizeof(pl->library[0]));
    pl->library[0] = (uint16_t)obj;
    pl->libraryCount++;
    return obj;
}

// LibraryPosition$: onto the top, the bottom or n cards down; a tutor's find left on top of the shuffled rest
static void TestLibraryPosition(void)
{
    int ext, crusade, tutor, bears, i, count;
    static const char *const cards[] = {"Aura Extraction", "Enlightened Tutor", "Mystical Tutor", "Vampiric Tutor",
                                        "Worldly Tutor", "Time Ebb", "Repel", "Cromat", NULL};
    printf("library positions: top, bottom, tutors\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Aura Extraction: their enchantment on top of their library
    Lands(0, "Plains", 2);
    crusade = Put(1, "Crusade", MTG_ZONE_BATTLEFIELD);
    ext = Put(0, "Aura Extraction", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, ext, MTG_TGT_OBJ, crusade);
    PassBoth();
    CHECK(Zone(crusade) == MTG_ZONE_LIBRARY && Mtg_TopOfLibrary(&sD, 1) == crusade, "on top of its owner's library");

    Table();   // Enlightened Tutor: the enchantment found, the library shuffled, the find on top
    Lands(0, "Plains", 1);
    crusade = PutLibBottom(0, "Crusade");
    count = sD.players[0].libraryCount;
    tutor = Put(0, "Enlightened Tutor", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, tutor, -1, 0);
    PassBoth();
    CHECK(Mtg_TopOfLibrary(&sD, 0) == crusade && sD.players[0].libraryCount == count, "found and put on top");

    Table();   // the bottom, and two cards down
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    Mtg_PutInLibrary(&sD, bears, -1);
    CHECK(sD.players[0].library[0] == bears && Mtg_TopOfLibrary(&sD, 0) != bears, "-1: the bottom");
    Mtg_MoveTo(&sD, bears, MTG_ZONE_GRAVEYARD);
    Mtg_PutInLibrary(&sD, bears, 2);
    CHECK(sD.players[0].library[sD.players[0].libraryCount - 3] == bears, "2: under two cards");
}

// a card put on top of its player's library (the test's own set-up)
static int PutLibTop(int player, const char *name)
{
    int obj = Put(player, name, MTG_ZONE_LIBRARY);
    MtgPlayer *pl = &sD.players[player];
    pl->library[pl->libraryCount++] = (uint16_t)obj;
    return obj;
}

// DigEffect: the top cards looked at, some taken, the rest to the bottom / the graveyard
static void TestDig(void)
{
    int impulse, bears, mem, i, looked[3], lib, grave, hand;
    const MtgQuery *q;
    static const char *const cards[] = {"Impulse", "Ancestral Memories", "Sleight of Hand", "Prophetic Bolt",
                                        "Goblin Ringleader", "Worldly Counsel", "Call of the Wild", NULL};
    printf("dig: the top cards looked at\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();   // Impulse: one of the top four into the hand, the other three to the bottom
    Lands(0, "Island", 2);
    bears = PutLibTop(0, "Grizzly Bears");
    lib = sD.players[0].libraryCount;
    for (i = 0; i < 3; i++)
        looked[i] = sD.players[0].library[lib - 2 - i];
    impulse = Put(0, "Impulse", MTG_ZONE_HAND);
    Do(MTG_ACT_CAST, impulse, -1, 0);
    PassBoth();
    q = Mtg_Query(&sD);
    CHECK(q->kind == MTG_Q_CHOOSE && q->choice == MTG_CHOICE_PICK && q->candidateCount == 4, "four to choose from");
    ChoosePick(bears);
    CHECK(Zone(bears) == MTG_ZONE_HAND && sD.players[0].libraryCount == lib - 1, "the bears taken");
    for (i = 0; i < 3; i++)
        CHECK(Zone(looked[i]) == MTG_ZONE_LIBRARY && Mtg_TopOfLibrary(&sD, 0) != looked[i], "the rest at the bottom");

    Table();   // Ancestral Memories: two of the top seven, five into the graveyard
    Lands(0, "Island", 5);
    mem = Put(0, "Ancestral Memories", MTG_ZONE_HAND);
    hand = HandCount(0);
    grave = 0;
    for (i = 0; i < sD.objCount; i++)
        grave += sD.objs[i].zone == MTG_ZONE_GRAVEYARD && sD.objs[i].owner == 0;
    Do(MTG_ACT_CAST, mem, -1, 0);
    PassBoth();
    Choose(0);
    Choose(0);
    {
        int g = 0;
        for (i = 0; i < sD.objCount; i++)
            g += sD.objs[i].zone == MTG_ZONE_GRAVEYARD && sD.objs[i].owner == 0;
        CHECK(HandCount(0) == hand - 1 + 2 && g == grave + 5 + 1, "two in hand, five (and the spell) in the graveyard");
    }
}

// an ability's own timing: ActivationLimit$ (PIT IMP twice a turn), SorcerySpeed$ (RECURRING NIGHTMARE), a spell's
// ActivationPhases$ (PANIC only in combat before blockers)
static void TestActivation(void)
{
    int imp, night, bears, dead, panic, foe, i;
    static const char *const cards[] = {"Pit Imp", "Rootwalla", "Recurring Nightmare", "Panic", "Fire Drake",
                                        "Quirion Ranger", "Volrath's Gardens", NULL};
    printf("activation: once a turn, as a sorcery, only in combat\n");
    for (i = 0; cards[i]; i++)
        CHECK(Mtg_CardByName(cards[i]) >= 0 && Mtg_Card(Mtg_CardByName(cards[i]))->supported, "%s plays", cards[i]);

    Table();
    Lands(0, "Swamp", 4);
    imp = Put(0, "Pit Imp", MTG_ZONE_BATTLEFIELD);
    for (i = 0; i < 2; i++) {
        CHECK(Find(MTG_ACT_ACTIVATE, imp, -1, 0) >= 0, "the imp's ability offered (%d)", i + 1);
        Do(MTG_ACT_ACTIVATE, imp, -1, 0);
        PassBoth();
    }
    CHECK(Mtg_Power(&sD, imp) == 2, "the imp pumped twice");
    CHECK(Find(MTG_ACT_ACTIVATE, imp, -1, 0) < 0, "no third time this turn");
    GoTo(1, MTG_STEP_MAIN1);
    GoTo(0, MTG_STEP_MAIN1);
    CHECK(Find(MTG_ACT_ACTIVATE, imp, -1, 0) >= 0, "again next turn");

    Table();
    night = Put(0, "Recurring Nightmare", MTG_ZONE_BATTLEFIELD);
    bears = Put(0, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    dead = Put(0, "Hill Giant", MTG_ZONE_GRAVEYARD);
    CHECK(Find(MTG_ACT_ACTIVATE, night, MTG_TGT_OBJ, dead) >= 0, "the nightmare in a main phase");
    GoTo(0, MTG_STEP_END);
    CHECK(Find(MTG_ACT_ACTIVATE, night, -1, 0) < 0, "not in the end step");
    GoTo(0, MTG_STEP_UPKEEP);   // (the opponent's turn first: player 0 holds priority there in their upkeep)
    CHECK(sD.active == 1 && Find(MTG_ACT_ACTIVATE, night, -1, 0) < 0, "not on the opponent's turn");
    (void)bears;

    Table();
    Lands(0, "Mountain", 1);
    panic = Put(0, "Panic", MTG_ZONE_HAND);
    foe = Put(1, "Grizzly Bears", MTG_ZONE_BATTLEFIELD);
    CHECK(Find(MTG_ACT_CAST, panic, -1, 0) < 0, "PANIC not in the main phase");
    GoTo(0, MTG_STEP_COMBAT_BEGIN);
    CHECK(Find(MTG_ACT_CAST, panic, MTG_TGT_OBJ, foe) >= 0, "PANIC at the beginning of combat");
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
    TestStatics();
    TestAuras();
    TestCosts();
    TestRegenerate();
    TestStatics2();
    TestCycling();
    TestTokens();
    TestAmounts();
    TestAltCosts();
    TestGranted();
    TestTargets();
    TestSearch();
    TestTriggers();
    TestUnless();
    TestDelayed();
    TestPrevent();
    TestControl();
    TestCounters();
    TestLibraryPosition();
    TestDig();
    TestActivation();
    TestDuels();
    printf("%d checks, %d failed\n", sChecks, sFails);
    return sFails != 0;
}
