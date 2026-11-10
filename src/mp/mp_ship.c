// Moemon Kai: the first voyage to a region (the user, 2026-11-10: "like the boarding of the S.S. TIDAL to the BATTLE
// FRONTIER, a small area without any battles that you can go around and talk to others who are excited about seeing
// the region you are going to"). The ship is three maps (data/maps/VoyageShip_*: the S.S. TIDAL's corridor and
// cabins, the S.S. ANNE's deck); qz_specials.c TravelNow boards it in place of the port when the trip is the player's
// first to that region. Ten passengers talk about where the ship is bound; once half have been talked to the ship's
// announcement says land is in sight (SHIP_NEAR), a rest on the cabin's bed brings the ship in (SHIP_DOCKED: the user,
// "just like Emerald's Battle Frontier trip, so the player can speak to every NPC"), and the sailor by the gangway
// lets the player off at the region's port, where the contact waits (mp_region.c MP_Region_FirstBoatIn).
// Also the family away from home: only the MOM of the region the save began in is MOM; the others' are the player's
// HOST MOM, HOENN's NORMAN their HOST DAD (MP_HostFamily_Filter, on the message box's text in those houses).
// All the state is in gRegionState (the save's ".region" side file): no save layout change.
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "item.h"
#include "mp.h"
#include "mp_region.h"
#include "overworld.h"
#include "script.h"
#include "script_menu.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/vars.h"

void MPNet_Log(const char *fmt, ...);
void MP_Voyage_Begin(bool8 eastbound, bool8 faded);
void MP_Voyage_Look(bool8 eastbound, bool8 coast);
void MP_Region_FirstBoatIn(u8 region);
void MP_Ship_SendTalked(u16 mask);
void MP_Ship_NoteArrival(u8 region);

#define SHIP_PASSENGERS 10
#define SHIP_GREETED    0x8000   // (shipTalked: the gangway's sailor has welcomed the player aboard)
#define SHIP_MASK       ((1 << SHIP_PASSENGERS) - 1)

enum { P_OLD_SALT, P_KID, P_HOMECOMER, P_TOURIST, P_TRAINER, P_GRANNY, P_HIM, P_HER, P_LOOKOUT, P_RESEARCHER };

static const u8 sRegionNames[REGION_COUNT][8] = {_("HOENN"), _("JOHTO"), _("KANTO"), _("SINNOH")};
static const u8 sPortNames[REGION_COUNT][16] = {_("SLATEPORT CITY"), _("OLIVINE CITY"), _("VERMILION CITY"), _("CANALAVE CITY")};
static const u8 sTownNames[REGION_COUNT][16] = {_("LITTLEROOT TOWN"), _("NEW BARK TOWN"), _("PALLET TOWN"), _("TWINLEAF TOWN")};
static const u16 sShowSpecies[REGION_COUNT] = {SPECIES_SKITTY, SPECIES_MAREEP, SPECIES_GROWLITHE, SPECIES_SHINX};
static const u16 sKeepsakes[REGION_COUNT] = {ITEM_LAVA_COOKIE, ITEM_RAGE_CANDY_BAR, ITEM_PEWTER_CRUNCHIES, ITEM_OLD_GATEAU};
static const u16 sTownSongs[REGION_COUNT] = {MUS_LITTLEROOT, MUS_JOHTO_NEW_BARK, MUS_RG_PALLET, MUS_SINNOH_TWINLEAF};

// What the passengers say, by where the ship is bound. Written without line breaks: MP_Text_Wrap fits them to the box
// ({STR_VAR_1} the region, {STR_VAR_2} its port, {STR_VAR_3} the first town)
static const u8 sH0[] = _("SLATEPORT's harbor is where we'll tie up. A shipyard, a market, and the sea breeze in between.\pHOENN is more water than land, you'll see. Make friends with a good swimmer.");
static const u8 sH1[] = _("HOENN has a volcano AND a town up in the trees AND a city floating on the sea!\pI'm going to see all of them. All of them!");
static const u8 sH2[] = _("I'm from FALLARBOR. Ash drifts down from MT. CHIMNEY like snow there.\pThree years away... I even miss sweeping it off the roof.");
static const u8 sH3[] = _("They say everyone in HOENN keeps a secret base inside a tree. Every single person.\p...That can't be right. Can it?");
static const u8 sH4[] = _("Ever seen one of these? You won't find it where we sailed from.\pIt's a SKITTY, HOENN born and raised!");
static const u8 sH5[] = _("Take this, dear. A LAVA COOKIE from LAVARIDGE TOWN. My sister bakes them.\pA taste of HOENN before you even land.");
static const u8 sH6[] = _("We're going to soak in LAVARIDGE's hot springs for a whole week. Nothing else. Just soaking.");
static const u8 sH7[] = _("He says a week of hot springs. I say we also watch the CONTESTS in LILYCOVE.\pWe'll see who wins that one.");
static const u8 sH9[] = _("PROF. BIRCH does his research out in the field.\pIf his lab in LITTLEROOT TOWN is empty, look for him in the tall grass. He's usually running.");

static const u8 sJ0[] = _("OLIVINE's lighthouse will guide us in. An AMPHAROS lights it, you know.\pIt's been bringing ships home for longer than I've sailed.");
static const u8 sJ1[] = _("JOHTO has a tower that burned down and a tower that's still standing, and BOTH have legends!\pI want to hear every one!");
static const u8 sJ2[] = _("I'm from ECRUTEAK. When the leaves turn red around the BELL TOWER... ah. Nothing like it.\pI've been away far too long.");
static const u8 sJ3[] = _("I heard JOHTO's POKé BALLS grow on trees. You just pick them.\p...They're only MADE from what grows on trees? That's nearly the same thing!");
static const u8 sJ4[] = _("Ever seen one of these? You won't find it where we sailed from.\pIt's a MAREEP, straight from JOHTO's pastures!");
static const u8 sJ5[] = _("Take this, dear. A RAGECANDYBAR from MAHOGANY TOWN. They've sold them there since I was a girl.\pA taste of JOHTO before you even land.");
static const u8 sJ6[] = _("We'll walk the NATIONAL PARK first. I hear it's lovely this time of year.");
static const u8 sJ7[] = _("And after the park, the RUINS OF ALPH. I love a good puzzle.\pHe doesn't know yet.");
static const u8 sJ9[] = _("PROF. ELM studies how POKéMON hatch from EGGS.\pHis lab is in NEW BARK TOWN, the town where the winds of a new beginning blow. A good place to start.");

static const u8 sK0[] = _("VERMILION is our port. The city of sunsets, they call it.\pEvery great ship on these seas has docked there one time or another.");
static const u8 sK1[] = _("KANTO is where it all started! The first POKéDEX! The first GYMS!\pI'm going to stand right where the CHAMPIONS stood!");
static const u8 sK2[] = _("I'm from LAVENDER TOWN. People think it's gloomy, but to me it's the calmest town in the world.\pI can't wait to hear nothing at all.");
static const u8 sK3[] = _("I heard there's a road in KANTO only for bicycles, and it's downhill both ways.\p...Only one way? Well. That's less exciting.");
static const u8 sK4[] = _("Ever seen one of these? You won't find it where we sailed from.\pIt's a GROWLITHE, KANTO's finest. Loyal as they come!");
static const u8 sK5[] = _("Take this, dear. PEWTER CRUNCHIES, from PEWTER CITY. Hard as the rocks there, and just as beloved.\pA taste of KANTO before you even land.");
static const u8 sK6[] = _("We're going to shop at CELADON's DEPT. STORE until we drop.");
static const u8 sK7[] = _("I'll be dropping first. At the GAME CORNER next door.\pDon't tell her.");
static const u8 sK9[] = _("PROF. OAK is THE authority on POKéMON. The man himself!\pHis lab is in PALLET TOWN. I've read every paper he ever wrote. Twice.");

static const u8 sS0[] = _("CANALAVE is where we dock. A city cut in two by a canal, with a drawbridge that lifts for ships like ours.\pMind the cold up there.");
static const u8 sS1[] = _("SINNOH has a mountain so big it cuts the whole land in half! And it SNOWS!\pI've never even seen snow!");
static const u8 sS2[] = _("I'm from SNOWPOINT. Folks ask how we stand the cold.\pTruth is, once you've seen the sun rise over fresh snow, anywhere else looks a little plain.");
static const u8 sS3[] = _("They say in SINNOH there's a whole world under your feet, and people dig for treasure down there all day.\p...That one's TRUE? Huh.");
static const u8 sS4[] = _("Ever seen one of these? You won't find it where we sailed from.\pIt's a SHINX! Its fur sparks when it's happy. It's happy a lot.");
static const u8 sS5[] = _("Take this, dear. An OLD GATEAU, baked to a secret recipe from the woods near ETERNA CITY.\pA taste of SINNOH before you even land.");
static const u8 sS6[] = _("We're going to watch a SUPER CONTEST in HEARTHOME CITY. Front row, if I can manage it.");
static const u8 sS7[] = _("And a walk in AMITY SQUARE after.\pI packed our POKéMON's best ribbon for it.");
static const u8 sS9[] = _("PROF. ROWAN studies how POKéMON evolve. A stern man, they say, but kinder than he looks.\pHis lab is in SANDGEM TOWN, one town over from TWINLEAF.");

static const u8 *const sLines[REGION_COUNT][SHIP_PASSENGERS] = {
    {sH0, sH1, sH2, sH3, sH4, sH5, sH6, sH7, NULL, sH9},
    {sJ0, sJ1, sJ2, sJ3, sJ4, sJ5, sJ6, sJ7, NULL, sJ9},
    {sK0, sK1, sK2, sK3, sK4, sK5, sK6, sK7, NULL, sK9},
    {sS0, sS1, sS2, sS3, sS4, sS5, sS6, sS7, NULL, sS9},
};
static const u8 sGrannyAfter[] = _("Did you like it, dear? There's plenty more where we're going.\pYou'll find {STR_VAR_1} is good to those who visit.");
static const u8 sLookoutSea[] = _("Nothing but blue from here to the horizon. We've a good way to go yet.\pHave you met the other passengers? Half the joy of a crossing is the company.");
static const u8 sLookoutNear[] = _("See that line on the horizon? That's {STR_VAR_1}.\pA rest in your cabin, and we'll be alongside before you know it.");
static const u8 sLookoutDocked[] = _("{STR_VAR_2}! Smell that? That's land, friend. Welcome to {STR_VAR_1}.");
static const u8 sGreeting[] = _("Welcome aboard, {PLAYER}! Your first voyage to {STR_VAR_1}, is it?\pIt's a long crossing, so you have a cabin: CABIN 2, right there.\pHave a walk around and meet the others. Everyone aboard is bound for {STR_VAR_1}, same as you. The stairs at the end lead up to the deck.");
static const u8 sExitSea[] = _("We're still out at sea. {STR_VAR_2} is a long way off yet.\pWhy not meet the other passengers? They've plenty to say about {STR_VAR_1}.");
static const u8 sExitNear[] = _("{STR_VAR_1} is in sight! We'll make port at {STR_VAR_2} soon.\pHave a rest on the bed in CABIN 2. I'll have the gangway down by the time you're up.");
static const u8 sExitDocked[] = _("We've arrived in {STR_VAR_2}!\pLooks like someone on the pier is waiting for you. Off you go, and enjoy {STR_VAR_1}!");
static const u8 sAnnounceNear[] = _("Ding-dong!\pAttention, all passengers. {STR_VAR_1} has been sighted off the bow.\pWe will make port at {STR_VAR_2} shortly. Passengers may rest in their cabins until we dock.");
static const u8 sAnnounceDocked[] = _("Ding-dong!\pWe have arrived at {STR_VAR_2}. Welcome to {STR_VAR_1}!\pPlease mind your step on the gangway.");
static const u8 sBedExcited[] = _("The bed looks soft... but you're far too excited about {STR_VAR_1} to sleep.\pMaybe the other passengers are, too.");
static const u8 sBedRest[] = _("{STR_VAR_1} is in sight. Rest here until the ship makes port?");
static const u8 sBedDocked[] = _("The ship has docked at {STR_VAR_2}. No time for a nap now!");
static const u8 sPorthole[] = _("The sea rolls by outside the porthole. Somewhere past it is {STR_VAR_1}.");
static const u8 sPortholeNear[] = _("Through the porthole, a thin line of land sits on the horizon. {STR_VAR_1}!");
static const u8 sPortholeDocked[] = _("Through the porthole: the piers of {STR_VAR_2}.");
static const u8 sSpyglass[] = _("Want a look through my spyglass?");
static const u8 sFirstVisit[] = _("It's your first voyage to {STR_VAR_1}!\pThe crossing is a long one, so there's a cabin for you aboard.\pA guide will meet you at {STR_VAR_2} and take you to {STR_VAR_3}, where a host family has a room ready for your stay.");

// ------------------------------------------------------------------------------------------------ fitting text
// Breaks the paragraphs of TEXT (between its \p's) that are marked, or all of them, into lines the message box holds:
// words kept whole, the first break of a paragraph a new line, the rest scrolling on
#define BOX_WIDTH 208
static u32 LineWidth(const u8 *from, const u8 *to)
{
    u8 buf[160];
    u32 n = to - from;
    if (n >= sizeof(buf))
        n = sizeof(buf) - 1;
    memcpy(buf, from, n);
    buf[n] = EOS;
    return GetStringWidth(FONT_NORMAL, buf, 0);
}

static u8 *NextChar(u8 *s)
{
    if (*s == EXT_CTRL_CODE_BEGIN)
        return s + 1 + GetExtCtrlCodeLength(s[1]);
    return s + 1;
}

static void WrapParagraph(u8 *start, u8 *end)
{
    u8 *s, *lineStart = start, *lastSpace = NULL;
    bool8 first = TRUE;
    for (s = start; s < end; s = NextChar(s))
        if (*s == CHAR_NEWLINE || *s == CHAR_PROMPT_SCROLL)
            *s = CHAR_SPACE;
    for (s = start; s < end; s = NextChar(s))
    {
        if (*s == CHAR_SPACE)
            lastSpace = s;
        if (LineWidth(lineStart, NextChar(s)) > BOX_WIDTH && lastSpace != NULL && lastSpace > lineStart)
        {
            *lastSpace = first ? CHAR_NEWLINE : CHAR_PROMPT_SCROLL;
            first = FALSE;
            lineStart = lastSpace + 1;
            lastSpace = NULL;
        }
    }
}

static bool8 ParagraphOverflows(u8 *start, u8 *end)
{
    u8 *s, *lineStart = start;
    for (s = start; s <= end; s = NextChar(s))
    {
        if (s == end || *s == CHAR_NEWLINE || *s == CHAR_PROMPT_SCROLL)
        {
            if (LineWidth(lineStart, s) > BOX_WIDTH)
                return TRUE;
            lineStart = s + 1;
        }
        if (s == end)
            break;
    }
    return FALSE;
}

// (all: every paragraph; else only those with a line too long for the box)
void MP_Text_Wrap(u8 *text, bool8 all)
{
    u8 *s = text, *start = text;
    for (;; s = NextChar(s))
    {
        if (*s == EOS || *s == CHAR_PROMPT_CLEAR)
        {
            if (all || ParagraphOverflows(start, s))
                WrapParagraph(start, s);
            if (*s == EOS)
                break;
            start = s + 1;
        }
    }
}

// ------------------------------------------------------------------------------------------------ the ship
bool8 MP_Ship_Aboard(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_VOYAGE_SHIP_CORRIDOR);
}

static u8 Dest(void)
{
    return gRegionState.shipTo >= 1 && gRegionState.shipTo <= REGION_COUNT ? gRegionState.shipTo - 1 : MP_Region_Current() % REGION_COUNT;
}

static void BufferNames(void)
{
    u8 to = Dest();
    StringCopy(gStringVar1, sRegionNames[to]);
    StringCopy(gStringVar2, sPortNames[to]);
    StringCopy(gStringVar3, sTownNames[to]);
}

static void Say(const u8 *text)
{
    BufferNames();
    StringExpandPlaceholders(gStringVar4, text);
    MP_Text_Wrap(gStringVar4, TRUE);
}

static u32 TalkedCount(void)
{
    u32 n = 0, i;
    for (i = 0; i < SHIP_PASSENGERS; i++)
        n += (gRegionState.shipTalked >> i) & 1;
    return n;
}

void MP_Ship_Board(u8 from, u8 to)
{
    gRegionState.shipTo = to + 1;
    gRegionState.shipFrom = from + 1;
    gRegionState.shipState = SHIP_AT_SEA;
    gRegionState.shipTalked = 0;
    SetWarpDestination(MAP_GROUP(MAP_VOYAGE_SHIP_CORRIDOR), MAP_NUM(MAP_VOYAGE_SHIP_CORRIDOR), WARP_ID_NONE, 1, 10);
    MPNet_Log("ship: aboard for region %d (the first voyage there)", to);
}

// The customs' words before a first voyage (mp_region.inc EventScript_RegionCustoms: VAR_0x8004 the region)
void MP_Ship_FirstVisitText(u8 to)
{
    StringCopy(gStringVar1, sRegionNames[to]);
    StringCopy(gStringVar2, sPortNames[to]);
    StringCopy(gStringVar3, sTownNames[to]);
    StringExpandPlaceholders(gStringVar4, sFirstVisit);
    MP_Text_Wrap(gStringVar4, TRUE);
}

// Half the passengers talked to: land comes in sight. TRUE when it just did (the announcement plays)
static bool8 CheckNear(void)
{
    if (gRegionState.shipState != SHIP_AT_SEA || TalkedCount() < SHIP_PASSENGERS / 2)
        return FALSE;
    gRegionState.shipState = SHIP_NEAR;
    MPNet_Log("ship: %d passengers talked to, land in sight", (int)TalkedCount());
    return TRUE;
}

void MP_Ship_FriendTalked(u16 mask)
{
    if (!MP_Ship_Aboard() || gRegionState.shipTo == 0)
        return;
    gRegionState.shipTalked |= mask & SHIP_MASK;
}

// special (the corridor's ON_TRANSITION): 1 while the gangway's sailor hasn't welcomed the player yet
u16 MP_ShipGreetDue(void)
{
    return gSpecialVar_Result = gRegionState.shipTo != 0 && !(gRegionState.shipTalked & SHIP_GREETED);
}

u16 MP_ShipGreet(void)
{
    gRegionState.shipTalked |= SHIP_GREETED;
    Say(sGreeting);
    return 0;
}

// special: VAR_0x8004 = the passenger. Their words in gStringVar4; RESULT 1: land just came in sight (the script
// plays the announcement after them), VAR_0x8005: the trainer's POKéMON / the keepsake still to give
u16 MP_ShipTalk(void)
{
    u8 who = gSpecialVar_0x8004, to = Dest();
    const u8 *line;
    bool8 near;
    gSpecialVar_0x8005 = 0;
    if (who >= SHIP_PASSENGERS)
        who = 0;
    if (!(gRegionState.shipTalked & (1 << who)))
    {
        gRegionState.shipTalked |= 1 << who;
        MP_Ship_SendTalked(1 << who);   // (co-op: friends aboard hear of it, mp_main.c)
        MPNet_Log("ship: passenger %d talked to (%d of %d)", who, (int)TalkedCount(), SHIP_PASSENGERS);
    }
    near = gRegionState.shipTo != 0 && CheckNear();   // (first: the lookout sees the land this talk brought in sight)
    if (who == P_LOOKOUT)
        line = gRegionState.shipState == SHIP_AT_SEA ? sLookoutSea : gRegionState.shipState == SHIP_NEAR ? sLookoutNear : sLookoutDocked;
    else if (who == P_GRANNY && (gRegionState.shipGift & (1 << to)))
        line = sGrannyAfter;
    else
        line = sLines[to][who];
    if (who == P_TRAINER)
        gSpecialVar_0x8005 = sShowSpecies[to];
    if (who == P_GRANNY && !(gRegionState.shipGift & (1 << to)))
        gSpecialVar_0x8005 = sKeepsakes[to];
    Say(line);
    return gSpecialVar_Result = near;
}

// special: a friend's talks may have brought land in sight while the player stood about: RESULT as MP_ShipTalk's
u16 MP_ShipCheckNear(void)
{
    return gSpecialVar_Result = gRegionState.shipTo != 0 && CheckNear();
}

u16 MP_ShipShowMon(void)
{
    ScriptMenu_ShowPokemonPic(gSpecialVar_0x8005, 10, 3);
    PlayCry_Normal(gSpecialVar_0x8005, 0);
    return 0;
}

u16 MP_ShipGiftDone(void)
{
    gRegionState.shipGift |= 1 << Dest();
    return 0;
}

u16 MP_ShipAnnounce(void)
{
    Say(gRegionState.shipState == SHIP_DOCKED ? sAnnounceDocked : sAnnounceNear);
    return 0;
}

// special: the cabin's bed. RESULT 0: too excited yet, 1: may rest (the script asks), 2: docked already
u16 MP_ShipBed(void)
{
    if (gRegionState.shipTo == 0 || gRegionState.shipState == SHIP_DOCKED)
    {
        Say(sBedDocked);
        return gSpecialVar_Result = 2;
    }
    if (gRegionState.shipState == SHIP_AT_SEA && !CheckNear())
    {
        Say(sBedExcited);
        return gSpecialVar_Result = 0;
    }
    Say(sBedRest);
    return gSpecialVar_Result = 1;
}

// special: rested: the ship is in. The first town's song plays from here to the pier
u16 MP_ShipDock(void)
{
    gRegionState.shipState = SHIP_DOCKED;
    Overworld_SetSavedMusic(sTownSongs[Dest()]);
    Overworld_ChangeMusicTo(sTownSongs[Dest()]);
    MPNet_Log("ship: docked at region %d's port", Dest());
    return 0;
}

// special: the gangway's sailor. RESULT 1: docked (the script lets the player off: MP_ShipLeave)
u16 MP_ShipExit(void)
{
    Say(gRegionState.shipTo == 0 || gRegionState.shipState == SHIP_DOCKED ? sExitDocked
        : gRegionState.shipState == SHIP_NEAR ? sExitNear : sExitSea);
    return gSpecialVar_Result = gRegionState.shipTo == 0 || gRegionState.shipState == SHIP_DOCKED;
}

// special (waitstate): down the gangway to the region's port, where the contact waits
u16 MP_ShipLeave(void)
{
    u8 to = Dest();
    Overworld_ClearSavedMusic();
    gRegionState.shipTo = 0;
    gRegionState.shipState = SHIP_AT_SEA;
    gRegionState.shipTalked = 0;
    gRegionState.stamps |= 1 << to;
    gRegionState.pad[0] = 1;   // (off the ship: the pier's sailor takes the POKéMON that came along ahead to the lab)
    MP_Region_FirstBoatIn(to);
    MP_Ship_NoteArrival(to);
    MPNet_Log("ship: off at region %d's port", to);
    DoWarp();
    ResetInitialPlayerAvatarState();
    return 0;
}

u16 MP_ShipPorthole(void)
{
    Say(gRegionState.shipState == SHIP_AT_SEA ? sPorthole : gRegionState.shipState == SHIP_NEAR ? sPortholeNear : sPortholeDocked);
    return 0;
}

u16 MP_ShipSpyglassAsk(void)
{
    Say(sSpyglass);
    return gSpecialVar_Result = gRegionState.shipState != SHIP_DOCKED;
}

// special (waitstate): the sea from the deck, the ship under way; the coast on the horizon once it's in sight
u16 MP_ShipSpyglass(void)
{
    const struct WarpData *here = &gSaveBlock1Ptr->location;
    SetWarpDestination(here->mapGroup, here->mapNum, WARP_ID_NONE, gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
    MP_Voyage_Look(gRegionState.shipTo > gRegionState.shipFrom, gRegionState.shipState == SHIP_NEAR);
    return 0;
}

// ------------------------------------------------------------------------------------------------ home and hosts
// The region the save began in. Saves from before 1.186.0 don't have it: HOENN when its opening was played (a boat
// never ran it), else the one region begun, else the first of those begun
u8 MP_Region_Home(void)
{
    u8 i;
    if (gRegionState.home >= 1 && gRegionState.home <= REGION_COUNT)
        return gRegionState.home - 1;
    if (VarGet(VAR_LITTLEROOT_INTRO_STATE) != 0 || !(gRegionState.started & ~1))
        gRegionState.home = REGION_HOENN + 1;
    else
        for (i = 1; i < REGION_COUNT; i++)
            if (gRegionState.started & (1 << i))
            {
                gRegionState.home = i + 1;
                break;
            }
    if (gRegionState.home == 0)
        gRegionState.home = REGION_HOENN + 1;
    MPNet_Log("region: the save began in region %d", gRegionState.home - 1);
    return gRegionState.home - 1;
}

// The maps where the region's MOM (HOENN: and NORMAN, in his GYM) speak to the player as family. 2: a map where
// others speak of their own family too (LITTLEROOT: the rival's DAD is PROF. BIRCH), so only MOM's own lines count
static u8 FamilyMap(void)
{
    static const u16 sMaps[] = {
        MAP_PETALBURG_CITY_GYM,
        MAP_JOHTO_PLAYERS_HOUSE1F, MAP_JOHTO_PLAYERS_HOUSE2F,
        MAP_KANTO_PALLET_TOWN_PLAYERS_HOUSE_1F, MAP_KANTO_PALLET_TOWN_PLAYERS_HOUSE_2F,
        MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE1F, MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE2F,
    };
    u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    if (gSaveBlock2Ptr->playerGender == MALE ? (here == MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F || here == MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F)
                                             : (here == MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F || here == MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F))
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(sMaps); i++)
        if (sMaps[i] == here)
            return TRUE;
    return here == MAP_LITTLEROOT_TOWN ? 2 : FALSE;
}

static bool8 IsLetter(u8 c)
{
    return (c >= CHAR_A && c <= CHAR_Z) || (c >= CHAR_a && c <= CHAR_z);
}

// MOM / Mom / mom and DAD / Dad / dad as whole words get HOST / Host / host before them; paragraphs that no longer
// fit the box are broken again. The first time the region's MOM speaks, she welcomes the player to her home first
void MP_HostFamily_Filter(u8 *text)
{
    static const u8 sWords[][4] = {_("MOM"), _("Mom"), _("mom"), _("DAD"), _("Dad"), _("dad")};
    static const u8 sHosts[][6] = {_("HOST "), _("Host "), _("host ")};
    static const u8 sWelcome[] = _("HOST MOM: Oh! You must be {PLAYER}, all the way from {STR_VAR_2}!\pWelcome to {STR_VAR_1}. I'm your HOST MOM for as long as you stay, so this is your home, too.\pYour room is upstairs. Make yourself comfortable!\p");
    static const u8 sMomLabel[] = _("MOM:");
    u8 region, *s, kind;
    bool8 changed = FALSE, momSpeaks = FALSE;
    u32 w, len = StringLength(text);
    if (gSaveBlock1Ptr == NULL || (kind = FamilyMap()) == 0)
        return;
    if (kind == 2 && memcmp(text, sMomLabel, 4) != 0)
        return;
    region = MP_Region_Current();
    if (region >= REGION_COUNT || region == MP_Region_Home())
        return;
    for (s = text; *s != EOS; s = NextChar(s))
    {
        if (s != text && IsLetter(s[-1]))
            continue;
        for (w = 0; w < ARRAY_COUNT(sWords); w++)
        {
            const u8 *host = sHosts[w % 3];
            u32 hl = StringLength(host);
            if (memcmp(s, sWords[w], 3) != 0 || IsLetter(s[3]) || len + hl >= 990)
                continue;
            if (s - text >= (s32)hl && memcmp(s - hl, host, hl) == 0)
                continue;   // (said already)
            memmove(s + hl, s, len - (s - text) + 1);
            memcpy(s, host, hl);
            len += hl;
            if (w < 3 && s == text)
                momSpeaks = TRUE;
            s += hl + 2;
            changed = TRUE;
            break;
        }
    }
    // (SINNOH's words are Platinum's, which name no speaker: there the first words in the house's 1F are her greeting
    // as the player comes down, TwinleafTownPlayerHouse1F's OnFrame scene)
    if (region == REGION_SINNOH && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE1F)
     && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE1F))
        momSpeaks = TRUE;
    if (changed)
        MP_Text_Wrap(text, FALSE);
    if (momSpeaks && !(gRegionState.hostMet & (1 << region)))
    {
        static u8 sBuf[1000];
        u32 wl;
        u8 keep1[32], keep2[32];
        gRegionState.hostMet |= 1 << region;
        memcpy(keep1, gStringVar1, sizeof(keep1)), memcpy(keep2, gStringVar2, sizeof(keep2));   // (the script's own)
        StringCopy(gStringVar1, sRegionNames[region]);
        StringCopy(gStringVar2, sRegionNames[MP_Region_Home()]);
        StringExpandPlaceholders(sBuf, region == REGION_SINNOH ? sWelcome + 10 : sWelcome);   // (SINNOH: no "HOST MOM: ")
        memcpy(gStringVar1, keep1, sizeof(keep1)), memcpy(gStringVar2, keep2, sizeof(keep2));
        MP_Text_Wrap(sBuf, TRUE);
        wl = StringLength(sBuf);
        if (wl + len < 990)
        {
            memmove(text + wl, text, len + 1);
            memcpy(text, sBuf, wl);
        }
        MPNet_Log("host: region %d's HOST MOM welcomes the player", region);
    }
}


// ------------------------------------------------------------------------------------------------ with friends
// Friends aboard the same ship count each other's talks: ours go out as they happen
void MP_Ship_SendTalked(u16 mask)
{
    u8 msg[3] = {MPR_SHIP, mask & 0xFF, mask >> 8};
    if (MP_IsConnected())
        MP_SendRelay(0xFF, msg, sizeof(msg));
}

// ------------------------------------------------------------------------------------------------ the arrival
// Off the ship in a new region: the POKéMON who came along remembers the crossing (its JOURNAL), and the TRAINER
// CARD gets the region's stamp (gRegionState.stamps: mp_cardscreen.c draws them)
void MP_Ship_NoteArrival(u8 region)
{
    void MP_Life_Voyage(u8 region);
    MP_Life_Voyage(region);
}

u8 MP_Ship_Stamps(void)
{
    return gRegionState.stamps;
}

// ------------------------------------------------------------------------------------------------ the arrival's scenes
// Off the ship (the user, 2026-11-10: "all of the arrivals need more of a scene to them"): at the pier a sailor comes
// up to the player, welcomes them, takes the POKéMON that sailed with them ahead to the professor's lab ("every game
// expects you to not have a Pokemon when you begin": it comes back with the region's first, MP_Ship_Update below) and
// leads them off; outside the house in the first town he shows them the door and takes his leave; they go in, and the
// region's own opening starts from their room. data/scripts/mp_region.inc EventScript_RegionArrival*.
#define GUIDE_LOCALID 0x7D
#define ARRIVAL_TOWN  10   // VAR_REGION_ARRIVAL: 10 + region, in the first town with the sailor
#include "constants/event_objects.h"
#include "constants/qz_port.h"
#include "constants/sinnoh_gfx.h"
#include "constants/event_object_movement.h"
#include "constants/flags.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "fieldmap.h"
#include "field_player_avatar.h"

void MP_Region_FitMon(struct Pokemon *mon, u8 region);
struct PokemonStorage *MP_Region_GlobalPC(void);
bool8 MP_Region_DepositGlobal(const struct BoxPokemon *mon);
u16 MP_Region_GlobalFree(void);
void MP_Hoenn_SettleIn(bool8 male);
bool8 MP_IsInOverworld(void);
bool8 MP_IsLocalFree(void);

static const u8 sProfNames[REGION_COUNT][12] = {_("PROF. BIRCH"), _("PROF. ELM"), _("PROF. OAK"), _("PROF. ROWAN")};   // (HOENN, JOHTO, KANTO, SINNOH)
// where the sailor comes to at the pier (beside where the ship lets the player off: sRegionPorts / MP_Region_FirstBoatIn)
static const struct { u16 gfx; s8 x, y; } sPierGuide[REGION_COUNT] = {
    {OBJ_EVENT_GFX_SAILOR, 12, 11}, {OBJ_EVENT_GFX_QZ_49, 8, 13}, {OBJ_EVENT_GFX_QZ_16193, 24, 29}, {OBJ_EVENT_GFX_SINNOH_SAILOR, 8, 20},
};
// the first town: the house's door (the player stands under it, the sailor to their right), and the room upstairs
static const struct { u16 town; s8 doorX, doorY; u16 room; s8 roomX, roomY; } sHomes[REGION_COUNT + 1] = {
    {MAP_LITTLEROOT_TOWN, 5, 8, MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F, 4, 4},
    {MAP_JOHTO_NEW_BARK_TOWN, 20, 11, MAP_JOHTO_PLAYERS_HOUSE2F, 4, 4},
    {MAP_KANTO_PALLET_TOWN, 6, 7, MAP_KANTO_PALLET_TOWN_PLAYERS_HOUSE_2F, 6, 6},
    {MAP_SINNOH_TWINLEAF_TOWN, 20, 21, MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE2F, 8, 5},
    {MAP_LITTLEROOT_TOWN, 14, 8, MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F, 4, 4},   // (HOENN, a girl's house)
};

static const u8 sArrWelcome[] = _("Ahoy there! You're {PLAYER}, the TRAINER from overseas, aren't you?\pWelcome to {STR_VAR_1}! {STR_VAR_3} asked me to meet your ship.");
static const u8 sArrPartner[] = _("First things first. A newcomer's POKéMON gets a checkup at the professor's lab before it travels here. Harbor rules!\pI'll see that {STR_VAR_2} gets there safe and sound.\pYou'll have it back the day you set out as a TRAINER in {STR_VAR_1}.");
static const u8 sArrEntrusted[] = _("{PLAYER} entrusted {STR_VAR_2} to the sailor.");
static const u8 sArrFollowHoenn[] = _("Now then. A family in LITTLEROOT TOWN has a room ready for you.\pYour HOST MOM can't wait to meet you, and your HOST DAD runs a GYM!\pIt's a fair way, so stay close. Follow me!");
static const u8 sArrFollow[] = _("Now then. A family in {STR_VAR_2} has a room ready for you. Your HOST MOM can't wait to meet you.\pIt's a fair way, so stay close. Follow me!");
static const u8 sArrFollowSinnoh[] = _("Now then. A family in TWINLEAF TOWN has a room ready for you. Your HOST MOM is expecting you.\pTWINLEAF is down the coast, so hop in my little boat. I'll have you there before the tide turns!");
static const u8 sArrTown[] = _("Here we are: {STR_VAR_1}!\pThis is the house. Your room is upstairs, and your HOST MOM has been looking forward to this all week.\p{STR_VAR_3}'s lab is {STR_VAR_2}. Rest first, though. You've come a long way.\pGood luck out there, {PLAYER}!");
static const u8 sLabHoenn[] = _("just down the road"), sLabJohto[] = _("just up the road"), sLabKanto[] = _("right over there"),
                sLabSinnoh[] = _("in SANDGEM TOWN, up ROUTE 201");
static const u8 *const sLabWhere[REGION_COUNT] = {sLabHoenn, sLabJohto, sLabKanto, sLabSinnoh};

static const u8 sBackHoenn[] = _("PROF. BIRCH: Oh, {PLAYER}! I nearly forgot. The harbor sent this POKé BALL up from SLATEPORT.\pIt's the partner that sailed here with you. Every newcomer's POKéMON gets a checkup at my lab, and yours is in fine shape!");
static const u8 sBackHoenn2[] = _("PROF. BIRCH: A partner from home and a new one from HOENN. You're off to a great start!");
static const u8 sBackJohto[] = _("ELM: Oh! Before I forget, {PLAYER}! This came up from OLIVINE's port for you.\pIt's the POKéMON you sailed here with. I checked it over myself. Healthy, and eager to see you!");
static const u8 sBackJohto2[] = _("ELM: I'd love to hear how a POKéMON from far away takes to JOHTO. Do tell me sometime!");
static const u8 sBackKanto[] = _("OAK: Ah, one more thing, {PLAYER}! The sailor from VERMILION left this in my care.\pYour partner from overseas! I gave it a looking-over, as I do for every visiting TRAINER. A splendid POKéMON!");
static const u8 sBackKanto2[] = _("OAK: Travel with both of them, and you'll learn twice as much about KANTO!");
static const u8 sBackSinnoh[] = _("Hm. One more matter. The sailor from CANALAVE brought this to my lab.\pIt is the POKéMON that crossed the sea with you. I examined it, as I do for every newcomer to SINNOH. It is well.");
static const u8 sBackSinnoh2[] = _("It waited for you patiently. See that you do not keep it waiting again.");
static const u8 sBackTogether[] = _("{PLAYER} and {STR_VAR_2} are together again!");
static const u8 *const sBack[REGION_COUNT][2] = {{sBackHoenn, sBackHoenn2}, {sBackJohto, sBackJohto2}, {sBackKanto, sBackKanto2}, {sBackSinnoh, sBackSinnoh2}};
// the labs, and the professor there (local id)
static const struct { u16 map; u8 prof; } sLabs[REGION_COUNT] = {
    {MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 2}, {MAP_JOHTO_ELMS_LAB, 2}, {MAP_KANTO_PALLET_TOWN_PROFESSOR_OAKS_LAB, 4},
    {MAP_SINNOH_SANDGEM_TOWN_POKEMON_RESEARCH_LAB, 1},
};

static u8 ArrivalRegion(void)
{
    u8 r = MP_Region_Current();
    return r < REGION_COUNT ? r : 0;
}

static u8 HomeIndex(u8 region)
{
    return region == REGION_HOENN && gSaveBlock2Ptr->playerGender != MALE ? REGION_COUNT : region;
}

static void PartnerName(u8 *dst)
{
    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_HAS_SPECIES))
    {
        GetMonData(&gPlayerParty[0], MON_DATA_NICKNAME, dst);
        StringGet_Nickname(dst);
    }
    else
        dst[0] = EOS;
}

// special: VAR_0x8004 = 0 at the pier, 1 in the town: the sailor appears there
u16 MP_ArrivalGuideSpawn(void)
{
    u8 r = ArrivalRegion(), id;
    s16 x, y;
    if (gSpecialVar_0x8004 == 0)
        x = sPierGuide[r].x, y = sPierGuide[r].y;
    else
        x = sHomes[HomeIndex(r)].doorX + 1, y = sHomes[HomeIndex(r)].doorY + 1;
    id = SpawnSpecialObjectEventParameterized(sPierGuide[r].gfx, MOVEMENT_TYPE_FACE_LEFT, GUIDE_LOCALID, x + MAP_OFFSET, y + MAP_OFFSET, 3);
    if (id < OBJECT_EVENTS_COUNT)
        gObjectEvents[id].currentElevation = gObjectEvents[id].previousElevation = gObjectEvents[gPlayerAvatar.objectEventId].currentElevation;
    return 0;
}

u16 MP_ArrivalGuideRemove(void)
{
    RemoveObjectEventByLocalIdAndMap(GUIDE_LOCALID, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    return 0;
}

// special: VAR_0x8004 = the sailor's words: 0 welcome, 1 the partner's checkup, 2 entrusted, 3 follow me, 4 in the town
u16 MP_ArrivalText(void)
{
    u8 r = ArrivalRegion();
    const u8 *t;
    StringCopy(gStringVar1, sRegionNames[r]);
    StringCopy(gStringVar3, sProfNames[r]);
    switch (gSpecialVar_0x8004)
    {
    default: t = sArrWelcome; break;
    case 1: t = sArrPartner; PartnerName(gStringVar2); break;
    case 2: t = sArrEntrusted; PartnerName(gStringVar2); break;
    case 3:
        t = r == REGION_HOENN ? sArrFollowHoenn : r == REGION_SINNOH ? sArrFollowSinnoh : sArrFollow;
        StringCopy(gStringVar2, sTownNames[r]);
        break;
    case 4:
        t = sArrTown;
        StringCopy(gStringVar1, sTownNames[r]);
        StringCopy(gStringVar2, sLabWhere[r]);
        break;
    }
    StringExpandPlaceholders(gStringVar4, t);
    MP_Text_Wrap(gStringVar4, TRUE);
    return 0;
}

// special: RESULT 1 when the sailor takes the player's POKéMON ahead (off the ship, with the one that sailed with them)
u16 MP_ArrivalCanHold(void)
{
    struct Pokemon *mon = &gPlayerParty[0];
    // (HOENN with PROF. BIRCH's rescue already behind the save, as SKIP THE INTRO leaves it: no first POKéMON is
    // coming there, so the one that sailed along stays with the player)
    if (ArrivalRegion() == REGION_HOENN && FlagGet(FLAG_RESCUED_BIRCH))
        return gSpecialVar_Result = 0;
    return gSpecialVar_Result = gRegionState.pad[0] && gRegionState.heldFor == 0 && CalculatePlayerPartyCount() == 1
        && GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(mon, MON_DATA_IS_EGG) && MP_Region_GlobalFree() >= 1;
}

// special: the POKéMON that sailed with the player goes ahead to the professor's lab (it waits in the GLOBAL PC).
// RESULT 0: nothing to keep (no POKéMON, or no room: it stays with the player)
u16 MP_ArrivalHold(void)
{
    u8 r = ArrivalRegion();
    struct Pokemon *mon = &gPlayerParty[0];
    if (gRegionState.heldFor != 0 || !GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES) || GetMonData(mon, MON_DATA_IS_EGG)
     || CalculatePlayerPartyCount() != 1 || !MP_Region_DepositGlobal(&mon->box))
        return gSpecialVar_Result = 0;
    gRegionState.heldFor = r + 1;
    gRegionState.heldPid = GetMonData(mon, MON_DATA_PERSONALITY);
    gRegionState.heldOt = GetMonData(mon, MON_DATA_OT_ID);
    MPNet_Log("ship: species %d goes ahead to region %d's lab", (int)GetMonData(mon, MON_DATA_SPECIES), r);
    ZeroMonData(mon);
    CalculatePlayerPartyCount();
    return gSpecialVar_Result = 1;
}

// The kept POKéMON, back in the party. FALSE: it isn't in the GLOBAL PC any more (taken out there), or no room
static bool8 ReturnPartner(void)
{
    struct PokemonStorage *st = MP_Region_GlobalPC();
    u32 b, p;
    u8 n = CalculatePlayerPartyCount();
    if (n >= PARTY_SIZE)
        return FALSE;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (p = 0; p < IN_BOX_COUNT; p++)
        {
            struct BoxPokemon *box = &st->boxes[b][p];
            if (!GetBoxMonData(box, MON_DATA_SANITY_HAS_SPECIES) || GetBoxMonData(box, MON_DATA_PERSONALITY) != gRegionState.heldPid
             || GetBoxMonData(box, MON_DATA_OT_ID) != gRegionState.heldOt)
                continue;
            BoxMonToMon(box, &gPlayerParty[n]);
            MP_Region_FitMon(&gPlayerParty[n], ArrivalRegion());
            ZeroBoxMonData(box);
            CalculatePlayerPartyCount();
            MPNet_Log("ship: species %d is back with the player", (int)GetMonData(&gPlayerParty[n], MON_DATA_SPECIES));
            return TRUE;
        }
    return FALSE;
}

// special: the professor's words on giving it back (VAR_0x8004: 0 before, 1 together again, 2 after); 1 gives it
u16 MP_ArrivalPartnerBack(void)
{
    u8 r = ArrivalRegion();
    if (gSpecialVar_0x8004 == 1)
    {
        u8 n = CalculatePlayerPartyCount();
        gStringVar2[0] = EOS;
        if (ReturnPartner())
        {
            GetMonData(&gPlayerParty[n], MON_DATA_NICKNAME, gStringVar2);
            StringGet_Nickname(gStringVar2);
        }
        gRegionState.heldFor = 0;
        StringExpandPlaceholders(gStringVar4, sBackTogether);
    }
    else
        StringExpandPlaceholders(gStringVar4, sBack[r][gSpecialVar_0x8004 == 2]);
    MP_Text_Wrap(gStringVar4, TRUE);
    return 0;
}

// special (waitstate): after the pier, to the first town, outside the house (SINNOH: in the sailor's boat)
u16 MP_ArrivalToTown(void)
{
    u8 r = ArrivalRegion(), h = HomeIndex(r);
    VarSet(VAR_REGION_ARRIVAL, ARRIVAL_TOWN + r);
    if (r == REGION_HOENN)
        MP_Hoenn_SettleIn(gSaveBlock2Ptr->playerGender == MALE);   // (the house lived in: the moving-in is a HOENN beginner's)
    if (r == REGION_HOENN)
    {
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);   // (no moving truck: the family has lived here for years)
        FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK);
    }
    SetWarpDestination(sHomes[h].town >> 8, sHomes[h].town & 0xFF, WARP_ID_NONE, sHomes[h].doorX, sHomes[h].doorY + 1);
    if (r == REGION_SINNOH)
        MP_Voyage_Begin(FALSE, FALSE);
    else
    {
        DoWarp();
        ResetInitialPlayerAvatarState();
    }
    return 0;
}

// special (waitstate): in at the door, up to the room: the region's own opening starts there
u16 MP_ArrivalToRoom(void)
{
    u8 r = ArrivalRegion(), h = HomeIndex(r);
    VarSet(VAR_REGION_ARRIVAL, 0);
    gRegionState.pad[0] = 0;
    MPNet_Log("ship: home, to the room in region %d", r);
    SetWarpDestination(sHomes[h].room >> 8, sHomes[h].room & 0xFF, WARP_ID_NONE, sHomes[h].roomX, sHomes[h].roomY);
    DoWarp();
    ResetInitialPlayerAvatarState();
    return 0;
}

// special: the professor turns to the player (the hand-back, in the lab)
u16 MP_ArrivalFaceProf(void)
{
    u8 r = ArrivalRegion(), id;
    if (TryGetObjectEventIdByLocalIdAndMap(sLabs[r].prof, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &id))
        return gSpecialVar_Result = 0;
    gSelectedObjectEvent = id;
    gSpecialVar_LastTalked = sLabs[r].prof;
    return gSpecialVar_Result = 1;
}

// Every frame on the field (mp_main.c): in the first town with the sailor, his scene there; in the professor's lab
// with the region's first POKéMON in hand, the one kept comes back
void MP_Ship_Update(void)
{
    extern const u8 EventScript_RegionArrivalTown[], EventScript_RegionPartnerBack[];
    u16 here, v;
    u8 r;
    if (gSaveBlock1Ptr == NULL || gSaveBlock2Ptr == NULL || !MP_IsInOverworld() || !MP_IsLocalFree()
     || ScriptContext_IsEnabled() || ArePlayerFieldControlsLocked())
        return;
    here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    v = VarGet(VAR_REGION_ARRIVAL);
    r = MP_Region_Current();
    if (r >= REGION_COUNT)
        return;
    if (v >= ARRIVAL_TOWN && v < ARRIVAL_TOWN + REGION_COUNT)
    {
        if (v - ARRIVAL_TOWN == r && here == sHomes[HomeIndex(r)].town)
            ScriptContext_SetupScript(EventScript_RegionArrivalTown);
        return;
    }
    if (gRegionState.heldFor == r + 1 && here == sLabs[r].map && CalculatePlayerPartyCount() >= 1
     && CalculatePlayerPartyCount() < PARTY_SIZE)
        ScriptContext_SetupScript(EventScript_RegionPartnerBack);
}
