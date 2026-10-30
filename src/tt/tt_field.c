// Moemon Kai: Triple Triad players on the map. Who plays it (src/tcg/tcg_field.c asks here first): the worldly and
// the well-mannered, people who travel and people who've seen a bit of the world, students between lessons; some of
// them play another card game as well and ask which. Their hands come from their region's cards (tt_hands.c),
// stronger with the player's badges. The special players with their own five-star cards; the sellers in the
// POKeMON CENTERS (and the GAME CENTER) and the CARD QUEEN, who travels HOENN and stirs the rules up wherever she is.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "item.h"
#include "mp.h"
#include "script.h"
#include "string_util.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/trainer_types.h"
#include "data.h"
#include "tcg/integration.h"
#include "tt/integration.h"
#include "tt/npc.h"
#include "tt/duel.h"

extern const u8 EventScript_TtSeller[], EventScript_TtQueen[];

// ---------------------------------------------------------------------------
// Where the player is

static unsigned sLastRegion = TT_REGION_PETALBURG;
static unsigned RegionOfMapsec(unsigned mapsec)
{
    switch (mapsec)
    {
    case MAPSEC_LITTLEROOT_TOWN: case MAPSEC_OLDALE_TOWN: case MAPSEC_PETALBURG_CITY: case MAPSEC_ROUTE_101:
    case MAPSEC_ROUTE_102: case MAPSEC_ROUTE_103: case MAPSEC_ROUTE_104:
        return TT_REGION_PETALBURG;
    case MAPSEC_RUSTBORO_CITY: case MAPSEC_PETALBURG_WOODS: case MAPSEC_ROUTE_115: case MAPSEC_ROUTE_116:
    case MAPSEC_RUSTURF_TUNNEL:
        return TT_REGION_RUSTBORO;
    case MAPSEC_DEWFORD_TOWN: case MAPSEC_SLATEPORT_CITY: case MAPSEC_ROUTE_105: case MAPSEC_ROUTE_106:
    case MAPSEC_ROUTE_107: case MAPSEC_ROUTE_108: case MAPSEC_ROUTE_109: case MAPSEC_ROUTE_110:
    case MAPSEC_GRANITE_CAVE: case MAPSEC_ABANDONED_SHIP:
        return TT_REGION_SLATEPORT;
    case MAPSEC_MAUVILLE_CITY: case MAPSEC_VERDANTURF_TOWN: case MAPSEC_ROUTE_111: case MAPSEC_ROUTE_117:
    case MAPSEC_ROUTE_118: case MAPSEC_NEW_MAUVILLE:
        return TT_REGION_MAUVILLE;
    case MAPSEC_LAVARIDGE_TOWN: case MAPSEC_FALLARBOR_TOWN: case MAPSEC_ROUTE_112: case MAPSEC_ROUTE_113:
    case MAPSEC_ROUTE_114: case MAPSEC_MT_CHIMNEY: case MAPSEC_FIERY_PATH: case MAPSEC_FIERY_PATH2:
    case MAPSEC_JAGGED_PASS: case MAPSEC_JAGGED_PASS2: case MAPSEC_METEOR_FALLS: case MAPSEC_METEOR_FALLS2:
    case MAPSEC_DESERT_RUINS:
        return TT_REGION_LAVARIDGE;
    case MAPSEC_FORTREE_CITY: case MAPSEC_LILYCOVE_CITY: case MAPSEC_ROUTE_119: case MAPSEC_ROUTE_120:
    case MAPSEC_ROUTE_121: case MAPSEC_ROUTE_122: case MAPSEC_ROUTE_123: case MAPSEC_MT_PYRE: case MAPSEC_SAFARI_ZONE:
    case MAPSEC_AQUA_HIDEOUT_OLD: case MAPSEC_SCORCHED_SLAB: case MAPSEC_ANCIENT_TOMB:
        return TT_REGION_LILYCOVE;
    case MAPSEC_MOSSDEEP_CITY: case MAPSEC_SOOTOPOLIS_CITY: case MAPSEC_PACIFIDLOG_TOWN: case MAPSEC_ROUTE_124:
    case MAPSEC_ROUTE_125: case MAPSEC_ROUTE_126: case MAPSEC_ROUTE_127: case MAPSEC_ROUTE_128: case MAPSEC_ROUTE_129:
    case MAPSEC_ROUTE_130: case MAPSEC_ROUTE_131: case MAPSEC_ROUTE_132: case MAPSEC_ROUTE_133: case MAPSEC_ROUTE_134:
    case MAPSEC_UNDERWATER_124: case MAPSEC_UNDERWATER_126: case MAPSEC_UNDERWATER_127: case MAPSEC_UNDERWATER_128:
    case MAPSEC_UNDERWATER_SOOTOPOLIS: case MAPSEC_SHOAL_CAVE: case MAPSEC_SEAFLOOR_CAVERN:
    case MAPSEC_UNDERWATER_SEAFLOOR_CAVERN: case MAPSEC_CAVE_OF_ORIGIN: case MAPSEC_SKY_PILLAR:
    case MAPSEC_SEALED_CHAMBER: case MAPSEC_UNDERWATER_SEALED_CHAMBER: case MAPSEC_MIRAGE_ISLAND: case MAPSEC_ISLAND_CAVE:
        return TT_REGION_SOOTOPOLIS;
    case MAPSEC_EVER_GRANDE_CITY: case MAPSEC_VICTORY_ROAD: case MAPSEC_BATTLE_FRONTIER: case MAPSEC_SOUTHERN_ISLAND:
        return TT_REGION_EVER_GRANDE;
    default:
        return TT_REGION_NONE;
    }
}
unsigned Tt_CurrentRegion(void)
{
    unsigned r = RegionOfMapsec(gMapHeader.regionMapSectionId);
    if (r < TT_REGIONS)
        sLastRegion = r;
    return sLastRegion;   // (a secret base, a building of no town: the region last seen)
}

static u8 Badges(void)
{
    u8 i, n = 0;
    for (i = 0; i < NUM_BADGES; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) != 0;
    return n;
}
static unsigned TierOfBadges(void)
{
    u8 b = Badges();
    return b >= 7 ? 3 : b >= 4 ? 2 : b >= 2 ? 1 : 0;
}

// ---------------------------------------------------------------------------
// How they talk: the kinds of Triple Triad player

enum { TALK_GENTRY, TALK_TRAVELER, TALK_STUDENT, TALK_ROMANTIC, TALK_SCHOLAR, TALK_COLLECTOR, TALK_COUNT };
static const u8 sText_Gentry1[] = _("TRIPLE TRIAD? Splendid. Nothing\nsettles the mind like a civil game.");
static const u8 sText_Gentry2[] = _("A game of cards, here? How civilized.\nDo mind the local rules, won't you?");
static const u8 sText_Gentry3[] = _("I keep a hand of my finest cards for\njust such an occasion. Shall we?");
static const u8 sText_GentryNo1[] = _("Not today, I'm afraid. My best cards\nare being reframed.");
static const u8 sText_GentryNo2[] = _("I only play after afternoon tea.\nDo come by another time.");
static const u8 sText_Traveler1[] = _("TRIPLE TRIAD! Every port plays it its\nown way. Let's see how you play it!");
static const u8 sText_Traveler2[] = _("I picked up some rules on the road.\nCareful, they spread like gossip!");
static const u8 sText_Traveler3[] = _("A card game's the best way to meet\npeople when you're on the move!");
static const u8 sText_TravelerNo1[] = _("My cards are packed away for the trip.\nCatch me at the next stop!");
static const u8 sText_TravelerNo2[] = _("Sorry, I'm in a hurry!\nNext time, for sure!");
static const u8 sText_Student1[] = _("TRIPLE TRIAD! Everyone at school\nplays between lessons. I'm the best!");
static const u8 sText_Student2[] = _("I traded my lunch for a rare card.\nWorth it! Want to see it?");
static const u8 sText_Student3[] = _("Our teacher says it's good for math.\nPLUS is all about sums, after all!");
static const u8 sText_StudentNo1[] = _("My teacher took my cards away...\nI'll get them back after class.");
static const u8 sText_StudentNo2[] = _("I'm supposed to be studying.\nDon't tell anyone I play!");
static const u8 sText_Romantic1[] = _("We met over a game of TRIPLE TRIAD!\nIt's our favorite thing to play.");
static const u8 sText_Romantic2[] = _("A game of cards with you? How lovely.\nI'll even let you go first. Maybe.");
static const u8 sText_Romantic3[] = _("My cards are as pretty as the sunset.\nShall I show you?");
static const u8 sText_RomanticNo1[] = _("Not now, dear.\nWe're having a moment.");
static const u8 sText_RomanticNo2[] = _("Oh, I'd love to, but my parasol\nkeeps my cards out of the sun. Later!");
static const u8 sText_Scholar1[] = _("TRIPLE TRIAD is a solved game, you\nknow. Well... almost. Let's test it!");
static const u8 sText_Scholar2[] = _("I've been charting how rules spread\nfrom town to town. Fascinating!");
static const u8 sText_Scholar3[] = _("Every number on a card is a promise.\nLet's see whose promises hold!");
static const u8 sText_ScholarNo1[] = _("I'm in the middle of an experiment.\nCome back when it's done!");
static const u8 sText_ScholarNo2[] = _("My notes say I lose on days like\ntoday. Better not.");
static const u8 sText_Collector1[] = _("You play TRIPLE TRIAD? Then you know\nthe rush of a five-star card!");
static const u8 sText_Collector2[] = _("I'd trade anything for a rare card.\nWell, almost. Play me for one!");
static const u8 sText_Collector3[] = _("A true collector plays for keeps.\nLet's go!");
static const u8 sText_CollectorNo1[] = _("My cards stay in their sleeves today.\nSorry!");
static const u8 sText_CollectorNo2[] = _("I'm sorting my collection by stars.\nIt'll take all day!");
static const u8 *const sAccept[TALK_COUNT][3] = {
    [TALK_GENTRY] = {sText_Gentry1, sText_Gentry2, sText_Gentry3},
    [TALK_TRAVELER] = {sText_Traveler1, sText_Traveler2, sText_Traveler3},
    [TALK_STUDENT] = {sText_Student1, sText_Student2, sText_Student3},
    [TALK_ROMANTIC] = {sText_Romantic1, sText_Romantic2, sText_Romantic3},
    [TALK_SCHOLAR] = {sText_Scholar1, sText_Scholar2, sText_Scholar3},
    [TALK_COLLECTOR] = {sText_Collector1, sText_Collector2, sText_Collector3},
};
static const u8 *const sDecline[TALK_COUNT][2] = {
    [TALK_GENTRY] = {sText_GentryNo1, sText_GentryNo2},
    [TALK_TRAVELER] = {sText_TravelerNo1, sText_TravelerNo2},
    [TALK_STUDENT] = {sText_StudentNo1, sText_StudentNo2},
    [TALK_ROMANTIC] = {sText_RomanticNo1, sText_RomanticNo2},
    [TALK_SCHOLAR] = {sText_ScholarNo1, sText_ScholarNo2},
    [TALK_COLLECTOR] = {sText_CollectorNo1, sText_CollectorNo2},
};
// Players of more than one game ask which, each kind in its own way (games: 1 Pokemon cards, 2 Yu-Gi-Oh!, 4 Triple
// Triad)
static const u8 sText_WhichGentryPk[] = _("MOéMON cards for nostalgia, or a\nround of TRIPLE TRIAD? Your choice.");
static const u8 sText_WhichTravelerPk[] = _("I play MOéMON cards with the kids\nand TRIPLE TRIAD in every port. Which?");
static const u8 sText_WhichStudentPk[] = _("MOéMON cards at recess, TRIPLE TRIAD\nafter class! Which one?");
static const u8 sText_WhichRomanticPk[] = _("We play MOéMON cards on dates and\nTRIPLE TRIAD at home. Pick one!");
static const u8 sText_WhichScholarPk[] = _("I'm studying two card games at once:\nMOéMON cards, or TRIPLE TRIAD?");
static const u8 sText_WhichCollectorYg[] = _("Yu-Gi-Oh! or TRIPLE TRIAD? I collect\nthe rarest of both. Choose!");
static const u8 sText_WhichScholarYg[] = _("Yu-Gi-Oh! for the combos, TRIPLE TRIAD\nfor the math. What will it be?");
static const u8 sText_WhichAll[] = _("A CARD CASE! MOéMON cards, Yu-Gi-Oh!\nor TRIPLE TRIAD? I play them all!");
static const u8 sText_WhichPk[] = _("MOéMON cards or TRIPLE TRIAD?\nI've got decks for both!");
static const u8 sText_WhichYg[] = _("Yu-Gi-Oh! or TRIPLE TRIAD?\nI'm up for either!");
static u8 sTalk;   // the kind last asked (for Tt_WhichLine)

const uint8_t *Tt_WhichLine(unsigned games, uint32_t hash)
{
    (void)hash;
    if ((games & 7) == 7)
        return sText_WhichAll;
    if (games & 1)
        switch (sTalk)
        {
        case TALK_GENTRY: return sText_WhichGentryPk;
        case TALK_TRAVELER: return sText_WhichTravelerPk;
        case TALK_STUDENT: return sText_WhichStudentPk;
        case TALK_ROMANTIC: return sText_WhichRomanticPk;
        case TALK_SCHOLAR: return sText_WhichScholarPk;
        default: return sText_WhichPk;
        }
    if (games & 2)
        return sTalk == TALK_SCHOLAR ? sText_WhichScholarYg : sTalk == TALK_COLLECTOR ? sText_WhichCollectorYg
                                                                                      : sText_WhichYg;
    return NULL;
}

// ---------------------------------------------------------------------------
// Special players

enum { REQ_NONE, REQ_BADGE, REQ_CHAMPION };
struct Special
{
    u16 gfx, gfx2;
    u16 mapGroup, mapNum;   // (people who look like others: where they are; 0xFFFF anywhere)
    u8 req, reqArg, play;
    const char *name, *title;
    u16 music;
    const u8 *accept, *refuse;
};
static const u8 sText_StevenTT[] = _("TRIPLE TRIAD too? You're a true\ncollector. Which game shall it be?");
static const u8 sText_WallaceYes[] = _("A game of TRIPLE TRIAD, played with\ngrace? I accept!");
static const u8 sText_WallaceNo[] = _("Become the CHAMPION first,\nand then we'll play.");
static const u8 sText_JuanYes[] = _("TRIPLE TRIAD... the cards turn like\nthe tide. Shall we dance?");
static const u8 sText_JuanNo[] = _("Earn the RAIN BADGE first, and then\nwe shall play.");
static const u8 sText_WinonaYes[] = _("My cards fly as high as my bird\nMOéMON. Let us play!");
static const u8 sText_WinonaNo[] = _("Earn the FEATHER BADGE first,\nand then we shall play.");
static const u8 sText_NormanYes[] = _("TRIPLE TRIAD, {PLAYER}? I played it\nbefore you were born. Show me.");
static const u8 sText_NormanNo[] = _("Earn the BALANCE BADGE first,\n{PLAYER}. Then we'll play.");
static const u8 sText_RoxanneYes[] = _("TRIPLE TRIAD is part of my lessons!\nLet's see what you've learned.");
static const u8 sText_RoxanneNo[] = _("First, prove yourself in my GYM.\nThen we can play cards.");
static const u8 sText_TateLizaYes[] = _("TATE: We read each other's cards...\nLIZA: ...and we read yours, too!");
static const u8 sText_TateLizaNo[] = _("TATE: First win the MIND BADGE...\nLIZA: ...then we'll play!");
static const u8 sText_StoneYes[] = _("Ah, TRIPLE TRIAD! DEVON once made\ncard tables, you know. Let's play!");
static const u8 sText_BrineyYes[] = _("Ahoy! I've played TRIPLE TRIAD in\nevery port on the sea. Shall we?");
static const u8 sText_ScottYes[] = _("I scout talent at card tables too.\nShow me what you've got!");
static const u8 sText_BirchYes[] = _("TRIPLE TRIAD! Field research needs\na break now and then. Let's play!");
static const u8 sText_MomYes[] = _("Oh, you play TRIPLE TRIAD now? Your\nfather and I used to play every night!");
static const struct Special sSpecials[TT_SPECIAL_COUNT] = {
    [TT_SPECIAL_STEVEN] = {OBJ_EVENT_GFX_STEVEN, 0, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ALSO, "STEVEN", "", MUS_VS_CHAMPION, sText_StevenTT, NULL},
    [TT_SPECIAL_RIVAL] = {OBJ_EVENT_GFX_RIVAL_MAY_NORMAL, OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ALSO, NULL, "", MUS_VS_RIVAL, NULL, NULL},
    [TT_SPECIAL_WALLY] = {OBJ_EVENT_GFX_WALLY, 0, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ALSO, "WALLY", "", MUS_VS_RIVAL, NULL, NULL},
    [TT_SPECIAL_WALLACE] = {OBJ_EVENT_GFX_WALLACE, 0, 0xFFFF, 0, REQ_CHAMPION, 0, TT_PLAYS_ALSO, "WALLACE", "CHAMPION", MUS_VS_CHAMPION, sText_WallaceYes, sText_WallaceNo},
    [TT_SPECIAL_JUAN] = {OBJ_EVENT_GFX_JUAN, 0, 0xFFFF, 0, REQ_BADGE, 7, TT_PLAYS_ALSO, "JUAN", "LEADER", MUS_VS_GYM_LEADER, sText_JuanYes, sText_JuanNo},
    [TT_SPECIAL_WINONA] = {OBJ_EVENT_GFX_WINONA, 0, 0xFFFF, 0, REQ_BADGE, 5, TT_PLAYS_ALSO, "WINONA", "LEADER", MUS_VS_GYM_LEADER, sText_WinonaYes, sText_WinonaNo},
    [TT_SPECIAL_NORMAN] = {OBJ_EVENT_GFX_NORMAN, 0, 0xFFFF, 0, REQ_BADGE, 4, TT_PLAYS_ALSO, "NORMAN", "LEADER", MUS_VS_GYM_LEADER, sText_NormanYes, sText_NormanNo},
    [TT_SPECIAL_ROXANNE] = {OBJ_EVENT_GFX_ROXANNE, 0, 0xFFFF, 0, REQ_BADGE, 0, TT_PLAYS_ALSO, "ROXANNE", "LEADER", MUS_VS_GYM_LEADER, sText_RoxanneYes, sText_RoxanneNo},
    [TT_SPECIAL_TATE_LIZA] = {OBJ_EVENT_GFX_TATE, OBJ_EVENT_GFX_LIZA, 0xFFFF, 0, REQ_BADGE, 6, TT_PLAYS_ALSO, "TATE & LIZA", "LEADERS", MUS_VS_GYM_LEADER, sText_TateLizaYes, sText_TateLizaNo},
    [TT_SPECIAL_MR_STONE] = {OBJ_EVENT_GFX_GENTLEMAN, 0, MAP_GROUP(MAP_RUSTBORO_CITY_DEVON_CORP_3F), MAP_NUM(MAP_RUSTBORO_CITY_DEVON_CORP_3F), REQ_NONE, 0, TT_PLAYS_ONLY, "MR. STONE", "DEVON PRESIDENT", MUS_RG_VS_TRAINER, sText_StoneYes, NULL},
    [TT_SPECIAL_MR_BRINEY] = {OBJ_EVENT_GFX_OLD_MAN, 0, MAP_GROUP(MAP_ROUTE104_MR_BRINEYS_HOUSE), MAP_NUM(MAP_ROUTE104_MR_BRINEYS_HOUSE), REQ_NONE, 0, TT_PLAYS_ONLY, "MR. BRINEY", "SAILOR", MUS_RG_VS_TRAINER, sText_BrineyYes, NULL},
    [TT_SPECIAL_SCOTT] = {OBJ_EVENT_GFX_SCOTT, 0, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ONLY, "SCOTT", "", MUS_RG_VS_TRAINER, sText_ScottYes, NULL},
    [TT_SPECIAL_BIRCH] = {OBJ_EVENT_GFX_PROF_BIRCH, 0, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ONLY, "PROF. BIRCH", "", MUS_RG_VS_TRAINER, sText_BirchYes, NULL},
    [TT_SPECIAL_MOM] = {OBJ_EVENT_GFX_MOM, 0, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ONLY, "MOM", "", MUS_RG_VS_TRAINER, sText_MomYes, NULL},
    [TT_SPECIAL_QUEEN] = {0xFFFF, 0, 0xFFFF, 0, REQ_NONE, 0, TT_PLAYS_ONLY, "CARD QUEEN", "", MUS_VS_ELITE_FOUR, NULL, NULL},
};

// Trainer classes who play Triple Triad: only it, or it besides their other game
static const struct
{
    u8 trainerClass, play, talk;
} sClasses[] = {
    {TRAINER_CLASS_GENTLEMAN, TT_PLAYS_ONLY, TALK_GENTRY},
    {TRAINER_CLASS_LADY, TT_PLAYS_ONLY, TALK_GENTRY},
    {TRAINER_CLASS_PARASOL_LADY, TT_PLAYS_ONLY, TALK_ROMANTIC},
    {TRAINER_CLASS_YOUNG_COUPLE, TT_PLAYS_ONLY, TALK_ROMANTIC},
    {TRAINER_CLASS_OLD_COUPLE, TT_PLAYS_ONLY, TALK_ROMANTIC},
    {TRAINER_CLASS_BIRD_KEEPER, TT_PLAYS_ONLY, TALK_TRAVELER},
    {TRAINER_CLASS_DRAGON_TAMER, TT_PLAYS_ONLY, TALK_COLLECTOR},
    {TRAINER_CLASS_PKMN_RANGER, TT_PLAYS_ONLY, TALK_TRAVELER},
    {TRAINER_CLASS_PKMN_BREEDER, TT_PLAYS_ONLY, TALK_SCHOLAR},
    {TRAINER_CLASS_INTERVIEWER, TT_PLAYS_ONLY, TALK_SCHOLAR},
    {TRAINER_CLASS_SAILOR, TT_PLAYS_ALSO, TALK_TRAVELER},
    {TRAINER_CLASS_TRIATHLETE, TT_PLAYS_ALSO, TALK_TRAVELER},
    {TRAINER_CLASS_SCHOOL_KID, TT_PLAYS_ALSO, TALK_STUDENT},
    {TRAINER_CLASS_RICH_BOY, TT_PLAYS_ALSO, TALK_GENTRY},
    {TRAINER_CLASS_BEAUTY, TT_PLAYS_ALSO, TALK_ROMANTIC},
    {TRAINER_CLASS_POKEFAN, TT_PLAYS_ALSO, TALK_GENTRY},
    {TRAINER_CLASS_WINSTRATE, TT_PLAYS_ALSO, TALK_ROMANTIC},
    {TRAINER_CLASS_COLLECTOR, TT_PLAYS_ALSO, TALK_COLLECTOR},
    {TRAINER_CLASS_COOLTRAINER, TT_PLAYS_ALSO, TALK_COLLECTOR},
    {TRAINER_CLASS_COOLTRAINER_2, TT_PLAYS_ALSO, TALK_COLLECTOR},
    {TRAINER_CLASS_RUIN_MANIAC, TT_PLAYS_ALSO, TALK_SCHOLAR},
    {TRAINER_CLASS_EXPERT, TT_PLAYS_ALSO, TALK_SCHOLAR},
};
// Townsfolk who play it, by how they look
static const struct
{
    u16 gfx;
    u8 play, talk;
    const char *label;
} sTownsfolk[] = {
    {OBJ_EVENT_GFX_GENTLEMAN, TT_PLAYS_ONLY, TALK_GENTRY, "GENTLEMAN"},
    {OBJ_EVENT_GFX_BARD, TT_PLAYS_ONLY, TALK_TRAVELER, "BARD"},
    {OBJ_EVENT_GFX_SAILOR, TT_PLAYS_ALSO, TALK_TRAVELER, "SAILOR"},
    {OBJ_EVENT_GFX_SCHOOL_KID_M, TT_PLAYS_ALSO, TALK_STUDENT, "SCHOOL KID"},
    {OBJ_EVENT_GFX_GAMEBOY_KID, TT_PLAYS_ALSO, TALK_STUDENT, "GAME KID"},
    {OBJ_EVENT_GFX_RICH_BOY, TT_PLAYS_ALSO, TALK_GENTRY, "RICH BOY"},
    {OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M, TT_PLAYS_ALSO, TALK_TRAVELER, "TRIATHLETE"},
    {OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F, TT_PLAYS_ALSO, TALK_TRAVELER, "TRIATHLETE"},
    {OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M, TT_PLAYS_ALSO, TALK_TRAVELER, "TRIATHLETE"},
    {OBJ_EVENT_GFX_CYCLING_TRIATHLETE_F, TT_PLAYS_ALSO, TALK_TRAVELER, "TRIATHLETE"},
    {OBJ_EVENT_GFX_BEAUTY, TT_PLAYS_ALSO, TALK_ROMANTIC, "BEAUTY"},
    {OBJ_EVENT_GFX_WOMAN_4, TT_PLAYS_ALSO, TALK_ROMANTIC, "LADY"},
    {OBJ_EVENT_GFX_POKEFAN_M, TT_PLAYS_ALSO, TALK_GENTRY, "MOeMON FAN"},
    {OBJ_EVENT_GFX_POKEFAN_F, TT_PLAYS_ALSO, TALK_GENTRY, "MOeMON FAN"},
    {OBJ_EVENT_GFX_ARTIST, TT_PLAYS_ALSO, TALK_ROMANTIC, "ARTIST"},
    {OBJ_EVENT_GFX_SCIENTIST_1, TT_PLAYS_ALSO, TALK_SCHOLAR, "SCIENTIST"},
    {OBJ_EVENT_GFX_SCIENTIST_2, TT_PLAYS_ALSO, TALK_SCHOLAR, "SCIENTIST"},
    {OBJ_EVENT_GFX_DEVON_EMPLOYEE, TT_PLAYS_ALSO, TALK_SCHOLAR, "DEVON WORKER"},
    {OBJ_EVENT_GFX_REPORTER_M, TT_PLAYS_ALSO, TALK_SCHOLAR, "REPORTER"},
    {OBJ_EVENT_GFX_REPORTER_F, TT_PLAYS_ALSO, TALK_SCHOLAR, "REPORTER"},
    {OBJ_EVENT_GFX_EXPERT_M, TT_PLAYS_ALSO, TALK_SCHOLAR, "EXPERT"},
    {OBJ_EVENT_GFX_EXPERT_F, TT_PLAYS_ALSO, TALK_SCHOLAR, "EXPERT"},
    {OBJ_EVENT_GFX_MANIAC, TT_PLAYS_ALSO, TALK_COLLECTOR, "MANIAC"},
};
static const u8 sText_BattleFirst[] = _("A game of cards? After a proper\nMOéMON battle, if you please.");

static void Fill(struct TtOpponent *o, const char *name, const char *title, unsigned region, uint32_t h, uint32_t key)
{
    unsigned tier = TierOfBadges();
    memset(o, 0, sizeof(*o) - sizeof(o->portrait));
    snprintf(o->name, sizeof(o->name), "%s", name);
    snprintf(o->title, sizeof(o->title), "%s", title ? title : "");
    o->region = (uint8_t)region;
    o->hash = h;
    o->tier = (uint8_t)tier;
    o->fit = 1;
    Tt_RegionHand(region, h, tier, Tcg_Today(), o->hand);
    o->level = (uint8_t)(tier >= 2 ? 2 : 1);
    if ((h >> 13) % 4 == 0 && o->level > 0)
        o->level--;   // (some play more loosely)
    o->key = key;
    o->special = TT_SPECIAL_NONE;
    o->music = MUS_RG_VS_TRAINER;
}
static void FillSpecial(struct TtOpponent *o, unsigned i, const char *name)
{
    const struct Special *s = &sSpecials[i];
    memset(o, 0, sizeof(*o) - sizeof(o->portrait));
    snprintf(o->name, sizeof(o->name), "%s", name ? name : s->name);
    snprintf(o->title, sizeof(o->title), "%s", s->title);
    o->region = (uint8_t)Tt_CurrentRegion();
    Tt_SpecialHand(i, Tcg_Today(), o->hand);
    o->fit = 1;
    o->level = 3;
    o->key = 0xFD000000u | i;
    o->special = (uint8_t)i;
    o->music = s->music;
}
void Tt_QueenOpponent(struct TtOpponent *o)
{
    FillSpecial(o, TT_SPECIAL_QUEEN, NULL);
    o->region = Tt_GetCollection()->queenRegion;
}

static int InGameCenter(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_RUSTBORO_CITY_GAME_CENTER_2F)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_2F);
}

int Tt_FieldPick(struct ObjectEvent *npc, const uint8_t *script, uint32_t h, uint32_t key, int clubStaff,
                 struct TtOpponent *o, const uint8_t **line, int *refused)
{
    unsigned i, region = Tt_CurrentRegion();
    *line = NULL;
    *refused = 0;
    sTalk = TALK_COLLECTOR;
    if (clubStaff)
    {
        Fill(o, "CARD CLUB", NULL, region, h, key);
        return TT_PLAYS_ALSO;
    }
    // The sellers play too, and the CARD QUEEN
    if (script == EventScript_TtSeller)
    {
        Fill(o, "TRIAD SELLER", NULL, region, h, key);
        o->level = 2;
        *line = sText_Traveler2;
        return TT_PLAYS_ONLY;
    }
    if (script == EventScript_TtQueen)
    {
        static const u8 sText_Queen[] = _("A game with the CARD QUEEN? I play by\nthe rules of wherever I stand.");
        Tt_QueenOpponent(o);
        *line = sText_Queen;
        return TT_PLAYS_ONLY;
    }
    for (i = 0; i < TT_SPECIAL_COUNT; i++)
    {
        const struct Special *s = &sSpecials[i];
        int ok = 1;
        if (npc->graphicsId != s->gfx && (!s->gfx2 || npc->graphicsId != s->gfx2))
            continue;
        if (s->mapGroup != 0xFFFF && (gSaveBlock1Ptr->location.mapGroup != s->mapGroup
                                      || gSaveBlock1Ptr->location.mapNum != s->mapNum))
            continue;
        if (s->req == REQ_BADGE)
            ok = FlagGet(FLAG_BADGE01_GET + s->reqArg);
        else if (s->req == REQ_CHAMPION)
            ok = FlagGet(FLAG_IS_CHAMPION);
        if (!ok)
        {
            *line = s->refuse;
            *refused = 1;
            return s->play;
        }
        FillSpecial(o, i, s->name ? s->name : npc->graphicsId == OBJ_EVENT_GFX_RIVAL_MAY_NORMAL ? "MAY" : "BRENDAN");
        *line = s->accept;
        return s->play;
    }
    {
        int play = 0;
        const char *label = NULL;
        if (npc->trainerType != TRAINER_TYPE_NONE && script && script[0] == 0x5C)   // trainerbattle: a real trainer
        {
            u16 id = script[2] | script[3] << 8;
            u8 cls = id < TRAINERS_COUNT ? gTrainers[id].trainerClass : 0;
            for (i = 0; i < ARRAY_COUNT(sClasses); i++)
                if (sClasses[i].trainerClass == cls)
                {
                    play = sClasses[i].play;
                    sTalk = sClasses[i].talk;
                }
            if (!play)
                return 0;
            if (!FlagGet(TRAINER_FLAGS_START + id))
            {
                *line = sText_BattleFirst;
                *refused = 1;
                return play;
            }
            {
                char clsName[16], who[16], name[28];
                MP_GameToAscii(clsName, gTrainerClassNames[cls], sizeof(clsName));
                MP_GameToAscii(who, gTrainers[id].trainerName, sizeof(who));
                snprintf(name, sizeof(name), "%s %s", clsName, who);
                Fill(o, name, NULL, region, h, key);
            }
        }
        else
        {
            for (i = 0; i < ARRAY_COUNT(sTownsfolk); i++)
                if (sTownsfolk[i].gfx == npc->graphicsId)
                {
                    play = sTownsfolk[i].play;
                    sTalk = sTownsfolk[i].talk;
                    label = sTownsfolk[i].label;
                }
            if (!play)
                return 0;
            if ((h >> 16) % 100 >= 70 && !InGameCenter())   // most of them play (the GAME CENTER's all do)
            {
                *line = sDecline[sTalk][(h >> 9) % 2];
                *refused = 1;
                return play;
            }
            Fill(o, label, NULL, region, h, key);
        }
        *line = sAccept[sTalk][(h >> 21) % 3];
        return play;
    }
}

// ---------------------------------------------------------------------------
// The sellers and the CARD QUEEN (data/scripts/tt_seller.inc)

static void BufferAscii(u8 *dest, const char *text)
{
    MP_AsciiToGame(dest, text, 64);
}
// VAR_RESULT: 1 the player has TRIPLE TRIAD cards. gStringVar1: this region, gStringVar2: its rules, gStringVar3:
// its trade rule
void Tt_SellerState(void)
{
    struct TtCollection *c = Tt_GetCollection();
    unsigned region = Tt_CurrentRegion(), r;
    char rules[96];
    int n = 0;
    gSpecialVar_Result = c->optedIn;
    rules[0] = 0;
    for (r = 0; r < TT_RULE_COUNT; r++)
        if (c->regionRules[region] >> r & 1)
            n += snprintf(rules + n, sizeof(rules) - (size_t)n, "%s%s", n ? ", " : "", Tt_RuleName(r));
    BufferAscii(gStringVar1, Tt_RegionName(region));
    BufferAscii(gStringVar2, n ? rules : "no special rules");
    BufferAscii(gStringVar3, Tt_TradeName(c->regionTrade[region]));
}
void Tt_SellerHasCase(void) { gSpecialVar_Result = CheckBagHasItem(ITEM_CARD_CASE, 1); }
void Tt_SellerStarter(void) { TtUi_OpenStarter(); }
void Tt_SellerShop(void)
{
    MPNet_Log("tt: seller open");
    TtUi_OpenSeller(Tt_CurrentRegion());
}
// The CARD QUEEN moves on by herself every few days
static void QueenDrift(void)
{
    struct TtCollection *c = Tt_GetCollection();
    unsigned today = Tcg_Today();
    if (Tt_WorldFromHost())
        return;   /* (in a friend's room she goes where the host's HOENN says) */
    if (!c->queenDay)
        c->queenDay = (uint16_t)today;
    if (today >= (unsigned)c->queenDay + 3)
    {
        unsigned to = (c->queenRegion + 1 + Tt_Rand(&c->rng) % (TT_REGIONS - 1)) % TT_REGIONS;
        unsigned rule = Tt_QueenMoves(c, to, today, Tt_Rand(&c->rng));
        printf("[tt] the CARD QUEEN moved to %s%s%s\n", Tt_RegionName(to), rule != 0xFF ? ", bringing " : "",
               rule != 0xFF ? Tt_RuleName(rule) : "");
    }
}
// Map setup (the POKeMON CENTERS with a seller): the CARD QUEEN shows only in her region
void Tt_MapSetup(void)
{
    QueenDrift();
    if (Tt_GetCollection()->queenRegion == Tt_CurrentRegion())
        FlagClear(FLAG_TT_QUEEN_AWAY);
    else
        FlagSet(FLAG_TT_QUEEN_AWAY);
}
void Tt_QueenPlaced(void) {}
// VAR_RESULT: 1 she's still in this region (after her favors: she may have left)
void Tt_QueenHere(void) { gSpecialVar_Result = Tt_GetCollection()->queenRegion == Tt_CurrentRegion(); }
void Tt_QueenTalk(void)
{
    static struct TtOpponent sQueen;
    MPNet_Log("tt: card queen");
    Tt_QueenOpponent(&sQueen);
    Tcg_PortraitOfObject(GetObjectEventIdByLocalIdAndMap((u8)gSpecialVar_LastTalked, gSaveBlock1Ptr->location.mapNum,
                                                         gSaveBlock1Ptr->location.mapGroup),
                         sQueen.portrait, &sQueen.portraitW, &sQueen.portraitH);
    TtUi_OpenQueen(&sQueen);
}
// VAR_RESULT: 1 the first meeting (and remembered)
void Tt_QueenFirstMeeting(void)
{
    struct TtCollection *c = Tt_GetCollection();
    gSpecialVar_Result = !(c->flags & TT_FLAG_QUEEN_MET);
    c->flags |= TT_FLAG_QUEEN_MET;
}
