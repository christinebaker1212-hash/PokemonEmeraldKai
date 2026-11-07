// Moemon Kai: MOSSDEEP's CARD CLUB, the people. The play room below (MOSSDEEP_CITY_GAME_CORNER_B1F) changes every
// half hour of the game's clock (the user, 2026-11-07): who comes to play (the four regulars of the room and the
// visiting guests below), which tables they take and which chairs, and the deck each guest brings (any deck the game
// has that plays: the starters, the regulars' decks, the structure decks: Mtg_PreconPlays). The same half hour gives
// the same room, so friends online see the same people (the room host's clock). The club's talkers (two upstairs, two on the
// play room's carpet) say something new each half hour too. Scripts: data/scripts/mtg_shop.inc.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "rtc.h"
#include "string_util.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/maps.h"
#include "mp.h"
#include "mtg/integration.h"

// ---- who comes to play
typedef struct {
    const char *name;
    u16 gfx;
    const char *hello;                 // %s: the deck they brought ("the SLIGH deck")
    const char *win, *lose;
} MtgGuest;

static const MtgGuest sGuests[] = {
    {"DAPHNE", OBJ_EVENT_GFX_LASS, "I just got %s and I can't stop shuffling it! Duel me, please, please, please!",
     "That was the best game of my whole life!", "Wow... you have to show me how you built that!"},
    {"ROLAND", OBJ_EVENT_GFX_GENTLEMAN, "I played %s at a tournament long ago. Shall we see if it still has teeth?",
     "Experience still counts for something, my friend.", "Marvelous! The game is in good hands."},
    {"KIT", OBJ_EVENT_GFX_SCHOOL_KID_M, "Did you know %s can win on turn four? Maybe! Let's find out right now!",
     "Turn four! Well, almost turn four!", "Huh. I'd better reread the rules."},
    {"MORGANA", OBJ_EVENT_GFX_HEX_MANIAC, "The cards whispered to me... they say %s will crush you. Shall we test it?",
     "The cards never lie.", "The cards... lied?!"},
    {"FELIX", OBJ_EVENT_GFX_PSYCHIC_M, "I already know what you'll play. I'm bringing %s anyway. Sit down!",
     "Just as I foresaw.", "I did NOT see that coming."},
    {"JUNE", OBJ_EVENT_GFX_PICNICKER, "I borrowed %s from my big brother! Don't tell him, okay? Let's play!",
     "Yay! Big brother's deck is the best!", "Oh no, he's going to know I lost with it!"},
    {"BRAM", OBJ_EVENT_GFX_CAMPER, "Lands, creatures, spells! %s has all of them! What are we waiting for?",
     "Woo! What a game!", "You got me! Good game, good game!"},
    {"SORA", OBJ_EVENT_GFX_NINJA_BOY, "Swift as a shadow, silent as a Swamp! %s strikes from the dark! Duel!",
     "The shadows win again!", "Defeated... I must train harder in the dark."},
    {"VOSS", OBJ_EVENT_GFX_SCIENTIST_1, "I've run the numbers on %s. Ninety-one percent chance of victory. Sit.",
     "As calculated.", "Fascinating. I must recalculate everything."},
    {"PENNY", OBJ_EVENT_GFX_GIRL_3, "MAGIC is the best game ever made, and %s is the best deck ever made! Probably!",
     "I told you! Best deck ever!", "Okay, maybe the second best deck ever..."},
    {"HUGO", OBJ_EVENT_GFX_BLACK_BELT, "Hoo-hah! %s hits harder than my karate chop! Duel me!",
     "Hoo-hah! Victory!", "Ugh! You hit harder than me!"},
    {"ESTELLE", OBJ_EVENT_GFX_EXPERT_F, "I've played since the very first set. Let's see how you do against %s.",
     "Come back when you've opened a few more boosters.", "Splendid. You play like a veteran."},
};
#define GUESTS ((int)(sizeof(sGuests) / sizeof(sGuests[0])))
#define FIRST_REGULAR 2                // gMtgDuelists' room regulars: RHEA, MARLOW, TESS, NOLAN (AUGUST and PIP: upstairs)
#define REGULARS (MTG_DUELISTS - FIRST_REGULAR)
static const u16 sRegularGfx[REGULARS] = {OBJ_EVENT_GFX_BEAUTY, OBJ_EVENT_GFX_MANIAC, OBJ_EVENT_GFX_WOMAN_2,
                                          OBJ_EVENT_GFX_RICH_BOY};

// ---- the tables of the play room (the user's own tables, 2026-11-07: four tables, chairs left and right of each):
// two players face each other across a table, at its top chairs or its bottom ones
typedef struct { u8 x, y; } Spot;
static const Spot sPairs[][2] = {      // [left chair (faces right), right chair (faces left)]
    {{0, 3}, {3, 3}}, {{0, 4}, {3, 4}},           // the top left table
    {{8, 3}, {11, 3}}, {{8, 4}, {11, 4}},         // the top right table
    {{0, 6}, {3, 6}},                             // the bottom left table
    {{8, 6}, {11, 6}},                            // the bottom right table
};
static const u8 sTableOfPair[] = {0, 0, 1, 1, 2, 3};
// The ones standing stay on the carpet in the middle (the user, 2026-11-07: "Make sure NPC concentration is on the
// carpet area, not near the tables"), in its inner columns 5-6 so the tiles in front of the chairs (columns 4 and 7)
// and the aisle stay free: someone watching a game, at the carpet's side nearest that table, and someone chatting in
// the middle row
static const Spot sWatch[4] = {{5, 4}, {6, 4}, {5, 6}, {6, 6}};   // by table: top left, top right, bottom left, right
#define SEATS 6                        // the play room's objects 1-6 (local ids), the one watching (7), the chatter (8)
#define WATCHER 7
#define CHATTER 8

static u32 sForcedSlot;                // "mp mtg club N" (the test harness): slot N + 1

static u32 Rng(u32 *s)
{
    *s ^= *s << 13;
    *s ^= *s >> 17;
    *s ^= *s << 5;
    return *s;
}

static u16 NowSlot(void)               // the game clock's half hour
{
    if (sForcedSlot)
        return (u16)(sForcedSlot - 1);
    RtcCalcLocalTime();
    return (u16)(gLocalTime.days * 48 + gLocalTime.hours * 2 + gLocalTime.minutes / 30);
}

typedef struct {
    u8 who[SEATS];                     // a duelist id each: gMtgDuelists' index, or MTG_DUELISTS + a guest
    Spot at[SEATS];
    u8 facing[SEATS];                  // MOVEMENT_TYPE_FACE_*
    Spot watch, chat;
    u8 watchFacing, chatFacing;
} Lineup;

static void MakeLineup(u16 slot, Lineup *l)
{
    u32 s = 0x9E3779B9u ^ (slot * 2654435761u), i, k;
    int pool[REGULARS + GUESTS], n = 0, regulars, tables[4] = {0, 1, 2, 3}, pairs[3], people[SEATS], order[SEATS];
    if (!s)
        s = 1;
    for (i = 0; i < 4; i++)
        Rng(&s);
    // two to four of the regulars, then guests
    regulars = 2 + (int)(Rng(&s) % 3);
    for (i = 0; i < REGULARS; i++)
        pool[n++] = FIRST_REGULAR + (int)i;
    for (i = n - 1; i > 0; i--) {
        int j = (int)(Rng(&s) % (i + 1)), t = pool[i];
        pool[i] = pool[j], pool[j] = t;
    }
    for (i = 0; i < (u32)regulars; i++)
        people[i] = pool[i];
    n = 0;
    for (i = 0; i < GUESTS; i++)
        pool[n++] = MTG_DUELISTS + (int)i;
    for (i = n - 1; i > 0; i--) {
        int j = (int)(Rng(&s) % (i + 1)), t = pool[i];
        pool[i] = pool[j], pool[j] = t;
    }
    for (k = 0; k < (u32)(SEATS - regulars); k++)
        people[regulars + k] = pool[k];
    // three tables of the four, each at its top or bottom chairs
    for (i = 3; i > 0; i--) {
        int j = (int)(Rng(&s) % (i + 1)), t = tables[i];
        tables[i] = tables[j], tables[j] = t;
    }
    for (i = 0; i < 3; i++) {
        int t = tables[i], first = -1, count = 0;
        for (k = 0; k < sizeof(sTableOfPair); k++)
            if (sTableOfPair[k] == t) {
                if (first < 0)
                    first = (int)k;
                count++;
            }
        pairs[i] = first + (int)(Rng(&s) % (u32)count);
    }
    for (i = 0; i < SEATS; i++)
        order[i] = (int)i;
    for (i = SEATS - 1; i > 0; i--) {
        int j = (int)(Rng(&s) % (i + 1)), t = order[i];
        order[i] = order[j], order[j] = t;
    }
    for (i = 0; i < SEATS; i++) {
        int seat = order[i], pair = pairs[seat / 2], side = seat % 2;
        l->who[i] = (u8)people[i];
        l->at[i] = sPairs[pair][side];
        l->facing[i] = side ? MOVEMENT_TYPE_FACE_LEFT : MOVEMENT_TYPE_FACE_RIGHT;
    }
    // the one watching: on the carpet's side nearest one of the games, facing it; the chatter beside them
    {
        int t = tables[Rng(&s) % 3];
        l->watch = sWatch[t];
        l->watchFacing = t % 2 ? MOVEMENT_TYPE_FACE_RIGHT : MOVEMENT_TYPE_FACE_LEFT;
        l->chat.x = (u8)(l->watch.x == 5 ? 6 : 5);
        l->chat.y = 5;
        l->chatFacing = Rng(&s) & 1 ? MOVEMENT_TYPE_FACE_DOWN : (l->chat.x == 5 ? MOVEMENT_TYPE_FACE_LEFT
                                                                               : MOVEMENT_TYPE_FACE_RIGHT);
    }
}

static u16 RoomSlot(void)              // the half hour the room was set up for (kept while the player is there)
{
    return VarGet(VAR_TEMP_2);
}

// ---- the decks a guest may bring: any deck of the game that plays
typedef struct { const char *const *cards; char label[48]; } DeckChoice;

static int Decks(DeckChoice *out, int cap)
{
    int n = 0, i;
    for (i = 0; i < MTG_STARTERS && n < cap; i++) {
        out[n].cards = gMtgStarters[i].cards;
        snprintf(out[n++].label, sizeof(out[0].label), "the %s deck", gMtgStarters[i].name);
    }
    for (i = 0; i < MTG_DUELISTS && n < cap; i++) {
        out[n].cards = gMtgDuelists[i].cards;
        snprintf(out[n++].label, sizeof(out[0].label), "a copy of %s's deck", gMtgDuelists[i].name);
    }
    for (i = 0; i < MTG_PRECONS && n < cap; i++)
        if (Mtg_PreconPlays(i)) {
            out[n].cards = gMtgPrecons[i].cards;
            snprintf(out[n++].label, sizeof(out[0].label), "the %s deck", gMtgPrecons[i].name);
        }
    return n;
}

const MtgDuelist *Mtg_Opponent(unsigned id)
{
    static MtgDuelist guest;
    static char hello[200];
    static DeckChoice decks[MTG_STARTERS + MTG_DUELISTS + MTG_PRECONS];
    const MtgGuest *g;
    int n;
    u32 s;
    if (id < MTG_DUELISTS)
        return &gMtgDuelists[id];
    if (id >= (unsigned)(MTG_DUELISTS + GUESTS))
        id = MTG_DUELISTS;
    g = &sGuests[id - MTG_DUELISTS];
    Mtg_EnsureDb();
    n = Decks(decks, (int)(sizeof(decks) / sizeof(decks[0])));
    s = (RoomSlot() * 2654435761u) ^ (id * 40503u) ^ 0x5bd1e995u;
    Rng(&s), Rng(&s);
    s = Rng(&s) % (u32)n;
    snprintf(hello, sizeof(hello), g->hello, decks[s].label);
    guest.name = g->name;
    guest.cards = decks[s].cards;
    guest.hello = hello;
    guest.win = g->win;
    guest.lose = g->lose;
    guest.level = 1;
    return &guest;
}

// ---- the map scripts' specials
static void Lookup(Lineup *l)
{
    MakeLineup(RoomSlot(), l);
}

// (B1F's ON_TRANSITION) the room for this half hour: who sits where, their looks (OBJ_EVENT_GFX_VAR_0-5)
void Mtg_ClubSetUp(void)
{
    Lineup l;
    int i;
    VarSet(VAR_TEMP_2, NowSlot());
    Lookup(&l);
    for (i = 0; i < SEATS; i++) {
        int who = l.who[i];
        VarSet(VAR_OBJ_GFX_ID_0 + i, who < MTG_DUELISTS ? sRegularGfx[who - FIRST_REGULAR]
                                                       : sGuests[who - MTG_DUELISTS].gfx);
        SetObjEventTemplateCoords((u8)(i + 1), l.at[i].x, l.at[i].y);
        SetObjEventTemplateMovementType((u8)(i + 1), l.facing[i]);
    }
    SetObjEventTemplateCoords(WATCHER, l.watch.x, l.watch.y);
    SetObjEventTemplateMovementType(WATCHER, l.watchFacing);
    SetObjEventTemplateCoords(CHATTER, l.chat.x, l.chat.y);
    SetObjEventTemplateMovementType(CHATTER, l.chatFacing);
    MPNet_Log("mtg: club slot %u: %s %d,%d  %s %d,%d  %s %d,%d  %s %d,%d  %s %d,%d  %s %d,%d  watch %d,%d",
              RoomSlot(), Mtg_Opponent(l.who[0])->name, l.at[0].x, l.at[0].y, Mtg_Opponent(l.who[1])->name, l.at[1].x,
              l.at[1].y, Mtg_Opponent(l.who[2])->name, l.at[2].x, l.at[2].y, Mtg_Opponent(l.who[3])->name, l.at[3].x,
              l.at[3].y, Mtg_Opponent(l.who[4])->name, l.at[4].x, l.at[4].y, Mtg_Opponent(l.who[5])->name, l.at[5].x,
              l.at[5].y, l.watch.x, l.watch.y);
}

// VAR_0x8005 a chair of the play room (0-5): VAR_0x8004 the duelist sitting there
void Mtg_ClubSeat(void)
{
    Lineup l;
    Lookup(&l);
    gSpecialVar_0x8004 = l.who[gSpecialVar_0x8005 < SEATS ? gSpecialVar_0x8005 : 0];
}

// (1F's ON_LOAD) VAR_RESULT: the player came up from the play room (through the door behind the counter)
void Mtg_ClubBackDoor(void)
{
    gSpecialVar_Result = gSaveBlock1Ptr->pos.x == 5 && gSaveBlock1Ptr->pos.y == 1;
}

// ---- the club's talkers: VAR_0x8004 which one (0, 1 upstairs; 2 watching below, 3 chatting): words in gStringVar4
static const char *const sTalk[] = {
    "Did you see that?! Someone just won with LIGHTNING BOLT! Three damage for one red mana. Beautiful!",
    "Five colors, five ways to play! White for armies, blue for tricks, black for power, red for speed, green for giants!",
    "My dream is to open a SERRA ANGEL in a booster. Flying AND vigilance... she's perfect!",
    "COUNTERSPELL! Two blue mana, and your best spell just... doesn't happen. I love it so much!",
    "Any booster could hold a rare. I've opened a hundred and I still get butterflies every time!",
    "The structure decks at the counter are real decks from years ago. World Champions played some of them!",
    "Untap, upkeep, draw... Once you learn how a turn goes, the whole game opens up like a book!",
    "Never attack into an untapped GIANT SPIDER. Trust me. I learned that the hard way. Twice.",
    "Trade your spare cards at the counter for CARD POINTS, buy boosters, open them, trade the spares... It never ends!",
    "Sometimes the best play is to do nothing and keep your mana open. Your opponent gets so nervous!",
    "You start at 20 life. Sounds like a lot... until a giant green creature with trample shows up!",
    "The players in the play room change every half hour! New faces, new decks! Come back later and see who's in!",
    "I built a deck with only one-mana creatures. It's either amazing or terrible. Usually both!",
    "Have you read the cards' little stories at the bottom? Some of them are better than the rules text!",
    "Two colors in one deck is the sweet spot. Three if you're brave. Five if you're... a legend!",
    "I keep a binder just for my rares. I look at it every night before bed. Is that weird? It's not weird.",
};
#define TALK ((u32)(sizeof(sTalk) / sizeof(sTalk[0])))

void Mtg_ClubTalk(void)
{
    u32 slot = gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_MOSSDEEP_CITY_GAME_CORNER_B1F) &&
               gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_MOSSDEEP_CITY_GAME_CORNER_B1F) ? RoomSlot() : NowSlot();
    u32 k = gSpecialVar_0x8004 % 4;
    MPNet_Log("mtg: club talker %u says line %u", k, (slot * 7 + k * 4) % TALK);
    Mtg_WrapToVar4(sTalk[(slot * 7 + k * 4) % TALK]);
}

void Mtg_ClubDebug(const char *command)   // "mp mtg club N": the room of half hour N (the next time it's entered)
{
    unsigned n = 0;
    if (sscanf(command, "%u", &n) == 1)
        sForcedSlot = n + 1;
    else
        sForcedSlot = 0;
    MPNet_Log("mtg: club slot forced %u", n);
}
