// Moemon Kai: card players on the map. With the CARD CASE registered to a
// button (SELECT / L / R / the extra button), that button in front of someone
// asks them for a card duel (FF8's Triple Triad: most people play): they answer
// in their own words and, if they play, the challenge window opens. Nobody in
// front: the CARD CASE opens. Also the CARD CLUB member in the POKeMON MARTS
// (the case, the first deck, the teaching duel and the counter).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "malloc.h"
#include "map_name_popup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "mp.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "rtc.h"
#include "script.h"
#include "constants/maps.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/trainer_types.h"
#include "gba/syscall.h"
#include "constants/trainers.h"
#include "tcg/integration.h"
#include "tcg/npc.h"
#include "tcg/card_ids.h"
#include "tcg/scale.h"
#include "ygo/integration.h"
#include "tt/integration.h"
#include "tt/npc.h"
#include "ygo/npc.h"
#include "menu.h"
#include "window.h"
#include "script_menu.h"
extern const u8 EventScript_CardClub[], EventScript_YgoClub[];

// ---------------------------------------------------------------------------
// Who they are

// How people talk about cards (by their look), and which kind of deck they play
enum { CAT_KID, CAT_STYLISH, CAT_TOUGH, CAT_SEA, CAT_REFINED, CAT_BRAINY, CAT_TEAM, CAT_STAFF, CAT_COUNT, CAT_NONE = 0xFF };
enum { DECK_FIRE, DECK_GRASS, DECK_LIGHTNING, DECK_WATER, DECK_FIGHTING, DECK_PSYCHIC, DECK_COLORLESS, DECK_MIXED };

static const struct
{
    u8 gfx, cat, deck;
    const char *label;   // townsfolk's name on the card screens
} sPeople[] = {
    {OBJ_EVENT_GFX_NINJA_BOY, CAT_KID, DECK_PSYCHIC, "NINJA BOY"},
    {OBJ_EVENT_GFX_TWIN, CAT_KID, DECK_GRASS, "TWIN"},
    {OBJ_EVENT_GFX_BOY_1, CAT_KID, DECK_MIXED, "BOY"},
    {OBJ_EVENT_GFX_GIRL_1, CAT_KID, DECK_MIXED, "GIRL"},
    {OBJ_EVENT_GFX_BOY_2, CAT_KID, DECK_MIXED, "BOY"},
    {OBJ_EVENT_GFX_GIRL_2, CAT_KID, DECK_MIXED, "GIRL"},
    {OBJ_EVENT_GFX_LITTLE_BOY, CAT_KID, DECK_MIXED, "LITTLE BOY"},
    {OBJ_EVENT_GFX_LITTLE_GIRL, CAT_KID, DECK_GRASS, "LITTLE GIRL"},
    {OBJ_EVENT_GFX_BOY_3, CAT_KID, DECK_MIXED, "BOY"},
    {OBJ_EVENT_GFX_GIRL_3, CAT_KID, DECK_MIXED, "GIRL"},
    {OBJ_EVENT_GFX_LITTLE_BOY_3, CAT_KID, DECK_MIXED, "LITTLE BOY"},
    {OBJ_EVENT_GFX_QUINTY_PLUMP, CAT_KID, DECK_COLORLESS, "BIG KID"},
    {OBJ_EVENT_GFX_YOUNGSTER, CAT_KID, DECK_COLORLESS, "YOUNGSTER"},
    {OBJ_EVENT_GFX_BUG_CATCHER, CAT_KID, DECK_GRASS, "BUG CATCHER"},
    {OBJ_EVENT_GFX_SCHOOL_KID_M, CAT_KID, DECK_MIXED, "SCHOOL KID"},
    {OBJ_EVENT_GFX_TUBER_F, CAT_SEA, DECK_WATER, "TUBER"},
    {OBJ_EVENT_GFX_TUBER_M, CAT_SEA, DECK_WATER, "TUBER"},
    {OBJ_EVENT_GFX_TUBER_M_SWIMMING, CAT_SEA, DECK_WATER, "TUBER"},
    {OBJ_EVENT_GFX_GAMEBOY_KID, CAT_BRAINY, DECK_LIGHTNING, "GAME KID"},
    {OBJ_EVENT_GFX_RICH_BOY, CAT_REFINED, DECK_COLORLESS, "RICH BOY"},
    {OBJ_EVENT_GFX_WOMAN_1, CAT_STYLISH, DECK_MIXED, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_2, CAT_STYLISH, DECK_MIXED, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_3, CAT_STYLISH, DECK_MIXED, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_4, CAT_STYLISH, DECK_MIXED, "WOMAN"},
    {OBJ_EVENT_GFX_WOMAN_5, CAT_STYLISH, DECK_MIXED, "WOMAN"},
    {OBJ_EVENT_GFX_POKEFAN_F, CAT_STYLISH, DECK_COLORLESS, "MOeMON FAN"},
    {OBJ_EVENT_GFX_PICNICKER, CAT_STYLISH, DECK_GRASS, "PICNICKER"},
    {OBJ_EVENT_GFX_LASS, CAT_STYLISH, DECK_GRASS, "LASS"},
    {OBJ_EVENT_GFX_BEAUTY, CAT_STYLISH, DECK_PSYCHIC, "BEAUTY"},
    {OBJ_EVENT_GFX_FAT_MAN, CAT_TOUGH, DECK_COLORLESS, "MAN"},
    {OBJ_EVENT_GFX_MAN_1, CAT_TOUGH, DECK_MIXED, "MAN"},
    {OBJ_EVENT_GFX_MAN_2, CAT_TOUGH, DECK_MIXED, "MAN"},
    {OBJ_EVENT_GFX_MAN_3, CAT_TOUGH, DECK_FIGHTING, "MAN"},
    {OBJ_EVENT_GFX_MAN_4, CAT_TOUGH, DECK_MIXED, "MAN"},
    {OBJ_EVENT_GFX_MAN_5, CAT_TOUGH, DECK_FIRE, "MAN"},
    {OBJ_EVENT_GFX_COOK, CAT_TOUGH, DECK_FIRE, "COOK"},
    {OBJ_EVENT_GFX_CAMPER, CAT_TOUGH, DECK_GRASS, "CAMPER"},
    {OBJ_EVENT_GFX_HIKER, CAT_TOUGH, DECK_FIGHTING, "HIKER"},
    {OBJ_EVENT_GFX_BLACK_BELT, CAT_TOUGH, DECK_FIGHTING, "BLACK BELT"},
    {OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M, CAT_TOUGH, DECK_FIGHTING, "TRIATHLETE"},
    {OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F, CAT_TOUGH, DECK_FIGHTING, "TRIATHLETE"},
    {OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M, CAT_TOUGH, DECK_LIGHTNING, "TRIATHLETE"},
    {OBJ_EVENT_GFX_CYCLING_TRIATHLETE_F, CAT_TOUGH, DECK_LIGHTNING, "TRIATHLETE"},
    {OBJ_EVENT_GFX_SWIMMER_M, CAT_SEA, DECK_WATER, "SWIMMER"},
    {OBJ_EVENT_GFX_SWIMMER_F, CAT_SEA, DECK_WATER, "SWIMMER"},
    {OBJ_EVENT_GFX_SAILOR, CAT_SEA, DECK_WATER, "SAILOR"},
    {OBJ_EVENT_GFX_FISHERMAN, CAT_SEA, DECK_WATER, "FISHERMAN"},
    {OBJ_EVENT_GFX_GENTLEMAN, CAT_REFINED, DECK_COLORLESS, "GENTLEMAN"},
    {OBJ_EVENT_GFX_EXPERT_M, CAT_REFINED, DECK_FIGHTING, "EXPERT"},
    {OBJ_EVENT_GFX_EXPERT_F, CAT_REFINED, DECK_PSYCHIC, "EXPERT"},
    {OBJ_EVENT_GFX_POKEFAN_M, CAT_REFINED, DECK_COLORLESS, "MOeMON FAN"},
    {OBJ_EVENT_GFX_OLD_MAN, CAT_REFINED, DECK_MIXED, "OLD MAN"},
    {OBJ_EVENT_GFX_OLD_WOMAN, CAT_REFINED, DECK_PSYCHIC, "OLD WOMAN"},
    {OBJ_EVENT_GFX_HOT_SPRINGS_OLD_WOMAN, CAT_REFINED, DECK_PSYCHIC, "OLD WOMAN"},
    {OBJ_EVENT_GFX_BARD, CAT_REFINED, DECK_MIXED, "BARD"},
    {OBJ_EVENT_GFX_ARTIST, CAT_REFINED, DECK_MIXED, "ARTIST"},
    {OBJ_EVENT_GFX_PSYCHIC_M, CAT_BRAINY, DECK_PSYCHIC, "PSYCHIC"},
    {OBJ_EVENT_GFX_HEX_MANIAC, CAT_BRAINY, DECK_PSYCHIC, "HEX MANIAC"},
    {OBJ_EVENT_GFX_MANIAC, CAT_BRAINY, DECK_LIGHTNING, "MANIAC"},
    {OBJ_EVENT_GFX_SCIENTIST_1, CAT_BRAINY, DECK_LIGHTNING, "SCIENTIST"},
    {OBJ_EVENT_GFX_SCIENTIST_2, CAT_BRAINY, DECK_LIGHTNING, "SCIENTIST"},
    {OBJ_EVENT_GFX_DEVON_EMPLOYEE, CAT_BRAINY, DECK_LIGHTNING, "DEVON WORKER"},
    {OBJ_EVENT_GFX_REPORTER_M, CAT_BRAINY, DECK_MIXED, "REPORTER"},
    {OBJ_EVENT_GFX_REPORTER_F, CAT_BRAINY, DECK_MIXED, "REPORTER"},
    {OBJ_EVENT_GFX_CAMERAMAN, CAT_BRAINY, DECK_MIXED, "CAMERAMAN"},
    {OBJ_EVENT_GFX_AQUA_MEMBER_M, CAT_TEAM, DECK_WATER, "TEAM AQUA GRUNT"},
    {OBJ_EVENT_GFX_AQUA_MEMBER_F, CAT_TEAM, DECK_WATER, "TEAM AQUA GRUNT"},
    {OBJ_EVENT_GFX_MAGMA_MEMBER_M, CAT_TEAM, DECK_FIRE, "TEAM MAGMA GRUNT"},
    {OBJ_EVENT_GFX_MAGMA_MEMBER_F, CAT_TEAM, DECK_FIRE, "TEAM MAGMA GRUNT"},
    {OBJ_EVENT_GFX_NURSE, CAT_STAFF, DECK_MIXED, "NURSE"},
    {OBJ_EVENT_GFX_UNION_ROOM_NURSE, CAT_STAFF, DECK_MIXED, "NURSE"},
    {OBJ_EVENT_GFX_MART_EMPLOYEE, CAT_STAFF, DECK_MIXED, "CLERK"},
    {OBJ_EVENT_GFX_LINK_RECEPTIONIST, CAT_STAFF, DECK_MIXED, "RECEPTIONIST"},
    {OBJ_EVENT_GFX_CONTEST_JUDGE, CAT_STAFF, DECK_MIXED, "JUDGE"},
    {OBJ_EVENT_GFX_TEALA, CAT_STAFF, DECK_MIXED, "TEALA"},
    {OBJ_EVENT_GFX_ROOFTOP_SALE_WOMAN, CAT_STAFF, DECK_MIXED, "CLERK"},
    {OBJ_EVENT_GFX_MYSTERY_GIFT_MAN, CAT_STAFF, DECK_MIXED, "RECEPTIONIST"},
};

// The Game Boy decks each kind of player picks from, by badges: 0-1, 2-3, 4-6, 7-8
static const char *const sPools[DECK_MIXED][4][3] = {
    [DECK_FIRE] = {{"CharmanderAndFriends", "LightningAndFire"}, {"Flamethrower", "Anger"},
                   {"FireCharge", "HeatedBattle"}, {"LegendaryMoltres", "LegendaryRonald"}},
    [DECK_GRASS] = {{"BulbasaurAndFriends", "Etcetera"}, {"FlowerGarden", "LovelyNidoran", "Poison"},
                    {"FlowerPower", "WondersofScience", "NapTime"}, {"InvincibleRonald", "Kaleidoscope"}},
    [DECK_LIGHTNING] = {{"LightningAndFire", "BoomBoomSelfdestruct"}, {"Pikachu", "PowerGenerator"},
                        {"ZappingSelfdestruct", "PowerGenerator"}, {"LegendaryZapdos", "ZappingSelfdestruct"}},
    [DECK_WATER] = {{"SquirtleAndFriends", "WaterAndFighting"}, {"WaterfrontPokemon", "SoundoftheWaves", "LonelyFriends"},
                    {"GoGoRainDance", "Imakuni"}, {"LegendaryArticuno", "LegendaryDragonite"}},
    [DECK_FIGHTING] = {{"WaterAndFighting", "SamsPractice", "BlisteringPokemon"}, {"HardPokemon", "LovetoBattle", "Excavation"},
                       {"MusclesforBrains", "RockCrusher"}, {"FirstStrike", "HeatedBattle"}},
    [DECK_PSYCHIC] = {{"NapTime", "SquirtleAndFriends"}, {"Ghost", "Imakuni"},
                      {"StrangePsyshock", "Ghost"}, {"StrangePower", "PowerfulRonald"}},
    [DECK_COLORLESS] = {{"LonelyFriends", "Anger"}, {"FlyinPokemon", "Kaleidoscope"},
                        {"ImRonald", "PowerfulRonald"}, {"LegendaryDragonite", "LegendaryRonald"}},
};

// Special trainers: who they look like, when they'll play, how their table looks and sounds
enum { REQ_NONE, REQ_BADGE, REQ_TRAINER, REQ_CHAMPION };
struct Special
{
    u8 gfx, gfx2, req;
    u16 reqArg;
    const char *name, *title;
    u8 mat, matType;
    u16 music;
    const u8 *accept, *refuse;
};
static const u8 sText_RoxanneYes[] = _("Oh! You play cards? I have studied\nthe rules thoroughly. Let us begin!");
static const u8 sText_RoxanneNo[] = _("First, prove yourself in my GYM.\pOnce you hold the STONE BADGE,\nI'll gladly duel you with cards.");
static const u8 sText_BrawlyYes[] = _("Cards, huh? I'll ride this wave of\nfighting spirit all the way! Bring it!");
static const u8 sText_BrawlyNo[] = _("Show me your guts in my GYM first!\nThen we'll throw down with cards!");
static const u8 sText_WattsonYes[] = _("Wahahahah! A card duel? Fine by me!\nMy deck is fully charged, I tell you!");
static const u8 sText_WattsonNo[] = _("Wahahah! Earn my DYNAMO BADGE first,\nthen we'll have ourselves a duel!");
static const u8 sText_FlanneryYes[] = _("A card duel? I-I'm not nervous!\nMy deck burns hot! Let's go!");
static const u8 sText_FlanneryNo[] = _("Beat me in my GYM first!\nThen I'll show you my cards!");
static const u8 sText_NormanYes[] = _("So you play cards, too, {PLAYER}.\nShow me how much you've grown.");
static const u8 sText_NormanNo[] = _("Earn the BALANCE BADGE first, {PLAYER}.\nThen we'll sit down for a duel.");
static const u8 sText_WinonaYes[] = _("My cards soar like my bird MOéMON.\nLet us duel gracefully.");
static const u8 sText_WinonaNo[] = _("Earn the FEATHER BADGE first,\nand then we shall play.");
static const u8 sText_TateLizaYes[] = _("TATE: Cards? LIZA: Cards!\nTATE & LIZA: We'll duel you together!");
static const u8 sText_TateLizaNo[] = _("TATE: First win the MIND BADGE...\nLIZA: ...and then we'll play cards!");
static const u8 sText_JuanYes[] = _("Ah, the elegance of a card duel...\nAllow me to show you my art.");
static const u8 sText_JuanNo[] = _("Earn the RAIN BADGE first, and then\nI shall duel you with cards.");
static const u8 sText_SidneyYes[] = _("Cards, huh? Heh, I like your style.\nLet's have some fun!");
static const u8 sText_PhoebeYes[] = _("Cards? My ghostly friends love them.\nLet's play!");
static const u8 sText_GlaciaYes[] = _("A card duel? Let me see if your deck\nis as cold as mine.");
static const u8 sText_DrakeYes[] = _("Cards! Do you have the will to win\nwith your deck, too? Show me!");
static const u8 sText_EliteNo[] = _("Defeat me in battle first.\nThen we may talk about cards.");
static const u8 sText_WallaceYes[] = _("A card duel? How beautiful!\nI accept, with pleasure.");
static const u8 sText_WallaceNo[] = _("Become the CHAMPION first,\nand then we'll play.");
static const u8 sText_StevenYes[] = _("Oh, you play cards? I collect rare\nstones, and rare cards. Let's duel!");
static const u8 sText_RivalYes[] = _("Hey! You've got a CARD CASE too?\nLet's see whose deck is better!");
static const u8 sText_WallyYes[] = _("You play cards too? I've been\npracticing! Please, duel with me!");
static const u8 sText_MaxieYes[] = _("Cards... a game of strategy.\nVery well, I'll humor you.");
static const u8 sText_ArchieYes[] = _("Bwahahaha! A card duel? Let's see\nyou weather this storm!");
static const u8 sText_AnabelYes[] = _("A card duel... I'd like to see\nhow you think.");
static const u8 sText_TuckerYes[] = _("Oh, a card duel! The crowd will\nlove this! Let's go!");
static const u8 sText_SpenserYes[] = _("Heh, cards? You've got guts, kid.\nCome on, then!");
static const u8 sText_GretaYes[] = _("Cards? All right, let's get\nfired up!");
static const u8 sText_NolandYes[] = _("A card duel? Sure, let's see\nwhat you know!");
static const u8 sText_LucyYes[] = _("...Cards. Fine.");

static const struct Special sSpecials[TCG_SPECIAL_COUNT] = {
    [TCG_SPECIAL_ROXANNE] = {OBJ_EVENT_GFX_ROXANNE, 0, REQ_BADGE, 0, "ROXANNE", "LEADER", 7, 4, MUS_VS_GYM_LEADER, sText_RoxanneYes, sText_RoxanneNo},
    [TCG_SPECIAL_BRAWLY] = {OBJ_EVENT_GFX_BRAWLY, 0, REQ_BADGE, 1, "BRAWLY", "LEADER", 7, 4, MUS_VS_GYM_LEADER, sText_BrawlyYes, sText_BrawlyNo},
    [TCG_SPECIAL_WATTSON] = {OBJ_EVENT_GFX_WATTSON, 0, REQ_BADGE, 2, "WATTSON", "LEADER", 7, 2, MUS_VS_GYM_LEADER, sText_WattsonYes, sText_WattsonNo},
    [TCG_SPECIAL_FLANNERY] = {OBJ_EVENT_GFX_FLANNERY, 0, REQ_BADGE, 3, "FLANNERY", "LEADER", 7, 0, MUS_VS_GYM_LEADER, sText_FlanneryYes, sText_FlanneryNo},
    [TCG_SPECIAL_NORMAN] = {OBJ_EVENT_GFX_NORMAN, 0, REQ_BADGE, 4, "NORMAN", "LEADER", 7, 6, MUS_VS_GYM_LEADER, sText_NormanYes, sText_NormanNo},
    [TCG_SPECIAL_WINONA] = {OBJ_EVENT_GFX_WINONA, 0, REQ_BADGE, 5, "WINONA", "LEADER", 7, 6, MUS_VS_GYM_LEADER, sText_WinonaYes, sText_WinonaNo},
    [TCG_SPECIAL_TATE_LIZA] = {OBJ_EVENT_GFX_TATE, OBJ_EVENT_GFX_LIZA, REQ_BADGE, 6, "TATE & LIZA", "LEADERS", 7, 5, MUS_VS_GYM_LEADER, sText_TateLizaYes, sText_TateLizaNo},
    [TCG_SPECIAL_JUAN] = {OBJ_EVENT_GFX_JUAN, 0, REQ_BADGE, 7, "JUAN", "LEADER", 7, 3, MUS_VS_GYM_LEADER, sText_JuanYes, sText_JuanNo},
    [TCG_SPECIAL_SIDNEY] = {OBJ_EVENT_GFX_SIDNEY, 0, REQ_TRAINER, TRAINER_SIDNEY, "SIDNEY", "ELITE FOUR", 8, 0, MUS_VS_ELITE_FOUR, sText_SidneyYes, sText_EliteNo},
    [TCG_SPECIAL_PHOEBE] = {OBJ_EVENT_GFX_PHOEBE, 0, REQ_TRAINER, TRAINER_PHOEBE, "PHOEBE", "ELITE FOUR", 8, 0, MUS_VS_ELITE_FOUR, sText_PhoebeYes, sText_EliteNo},
    [TCG_SPECIAL_GLACIA] = {OBJ_EVENT_GFX_GLACIA, 0, REQ_TRAINER, TRAINER_GLACIA, "GLACIA", "ELITE FOUR", 8, 0, MUS_VS_ELITE_FOUR, sText_GlaciaYes, sText_EliteNo},
    [TCG_SPECIAL_DRAKE] = {OBJ_EVENT_GFX_DRAKE, 0, REQ_TRAINER, TRAINER_DRAKE, "DRAKE", "ELITE FOUR", 8, 0, MUS_VS_ELITE_FOUR, sText_DrakeYes, sText_EliteNo},
    [TCG_SPECIAL_WALLACE] = {OBJ_EVENT_GFX_WALLACE, 0, REQ_CHAMPION, 0, "WALLACE", "CHAMPION", 8, 0, MUS_VS_CHAMPION, sText_WallaceYes, sText_WallaceNo},
    [TCG_SPECIAL_STEVEN] = {OBJ_EVENT_GFX_STEVEN, 0, REQ_NONE, 0, "STEVEN", "", 8, 0, MUS_VS_CHAMPION, sText_StevenYes, NULL},
    [TCG_SPECIAL_RIVAL] = {OBJ_EVENT_GFX_RIVAL_MAY_NORMAL, OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL, REQ_NONE, 0, NULL, "", 0, 0, MUS_VS_RIVAL, sText_RivalYes, NULL},
    [TCG_SPECIAL_WALLY] = {OBJ_EVENT_GFX_WALLY, 0, REQ_NONE, 0, "WALLY", "", 0, 0, MUS_VS_RIVAL, sText_WallyYes, NULL},
    [TCG_SPECIAL_MAXIE] = {OBJ_EVENT_GFX_MAXIE, 0, REQ_NONE, 0, "MAXIE", "MAGMA LEADER", 10, 0, MUS_VS_AQUA_MAGMA_LEADER, sText_MaxieYes, NULL},
    [TCG_SPECIAL_ARCHIE] = {OBJ_EVENT_GFX_ARCHIE, 0, REQ_NONE, 0, "ARCHIE", "AQUA LEADER", 9, 0, MUS_VS_AQUA_MAGMA_LEADER, sText_ArchieYes, NULL},
    [TCG_SPECIAL_ANABEL] = {OBJ_EVENT_GFX_ANABEL, 0, REQ_NONE, 0, "ANABEL", "SALON MAIDEN", 11, 0, MUS_VS_FRONTIER_BRAIN, sText_AnabelYes, NULL},
    [TCG_SPECIAL_TUCKER] = {OBJ_EVENT_GFX_TUCKER, 0, REQ_NONE, 0, "TUCKER", "DOME ACE", 11, 0, MUS_VS_FRONTIER_BRAIN, sText_TuckerYes, NULL},
    [TCG_SPECIAL_SPENSER] = {OBJ_EVENT_GFX_SPENSER, 0, REQ_NONE, 0, "SPENSER", "PALACE MAVEN", 11, 0, MUS_VS_FRONTIER_BRAIN, sText_SpenserYes, NULL},
    [TCG_SPECIAL_GRETA] = {OBJ_EVENT_GFX_GRETA, 0, REQ_NONE, 0, "GRETA", "ARENA TYCOON", 11, 0, MUS_VS_FRONTIER_BRAIN, sText_GretaYes, NULL},
    [TCG_SPECIAL_NOLAND] = {OBJ_EVENT_GFX_NOLAND, 0, REQ_NONE, 0, "NOLAND", "FACTORY HEAD", 11, 0, MUS_VS_FRONTIER_BRAIN, sText_NolandYes, NULL},
    [TCG_SPECIAL_LUCY] = {OBJ_EVENT_GFX_LUCY, 0, REQ_NONE, 0, "LUCY", "PIKE QUEEN", 11, 0, MUS_VS_FRONTIER_BRAIN, sText_LucyYes, NULL},
};

unsigned Tcg_SpecialPromo(unsigned special)
{
    return special < TCG_SPECIAL_COUNT ? gTcgSpecialPromos[special] : 0;
}

// ---------------------------------------------------------------------------
// What they say: 38 lines by the way they talk, for card players and people
// who don't play (and trainers who want a MOeMON battle first)

static const u8 sText_Kid1[] = _("Whoa, a CARD CASE! I carry my deck\neverywhere, just like my MOéMON!\pLet's duel!");
static const u8 sText_Kid2[] = _("I've been trading cards all week.\nMy deck's the best around here!\pYou're on!");
static const u8 sText_Kid3[] = _("My big brother taught me how to play.\nI'm not going to lose to you!");
static const u8 sText_KidNo1[] = _("Cards? I spent my allowance on\nPOKé BALLS... Maybe next time!");
static const u8 sText_KidNo2[] = _("I don't have any cards yet...\nI'm saving up for a booster pack!");
static const u8 sText_Stylish1[] = _("Oh, how cute! You play cards too?\nI picked my deck for its pictures!");
static const u8 sText_Stylish2[] = _("A card duel? Sounds lovely!\nI won't go easy on you, though.");
static const u8 sText_Stylish3[] = _("I only keep the prettiest cards.\nLet me show you!");
static const u8 sText_StylishNo1[] = _("Cards? Oh, I'd rather not.\nThey always get bent in my bag!");
static const u8 sText_StylishNo2[] = _("I'm afraid I don't play cards.\nBut good luck to you!");
static const u8 sText_Tough1[] = _("Hahaha! A card duel? My deck is as\nsolid as the mountains!");
static const u8 sText_Tough2[] = _("Training isn't only for the body!\nA duel sharpens the mind, too! Hah!");
static const u8 sText_Tough3[] = _("I always keep my deck in my pack.\nCome on, let's go!");
static const u8 sText_ToughNo1[] = _("Cards? My hands are too big for\nthose little things! Hahaha!");
static const u8 sText_ToughNo2[] = _("I don't play cards. I'd rather be\ntraining out here!");
static const u8 sText_Sea1[] = _("I keep my cards in a waterproof case!\nLet's duel!");
static const u8 sText_Sea2[] = _("Ahoy! A card duel? A sailor never\nbacks down from a challenge!");
static const u8 sText_Sea3[] = _("The fish aren't biting anyway.\nA card duel it is!");
static const u8 sText_SeaNo1[] = _("My cards got soaked last summer...\nI don't play anymore.");
static const u8 sText_SeaNo2[] = _("Cards? Out here? They'd just blow\naway in the sea breeze! Sorry!");
static const u8 sText_Refined1[] = _("Ah, a fellow collector! My deck has\nbeen in my family for years.");
static const u8 sText_Refined2[] = _("A duel? Splendid! Let us play with\nthe utmost elegance.");
static const u8 sText_Refined3[] = _("In my day, we played this game for\nhours on end. Let me show you!");
static const u8 sText_RefinedNo1[] = _("I'm afraid my cards are locked away\nin a display case. Not today.");
static const u8 sText_RefinedNo2[] = _("Card games? Oh my, I never learned.\nYou young people are so clever.");
static const u8 sText_Brainy1[] = _("I've worked out every probability\nin my deck. Let's put it to the test!");
static const u8 sText_Brainy2[] = _("I sensed that you would challenge\nme... Very well.");
static const u8 sText_Brainy3[] = _("Card duels? I've read every rule!\nLet's go!");
static const u8 sText_BrainyNo1[] = _("I'm in the middle of my research.\nNo time for cards, sorry.");
static const u8 sText_BrainyNo2[] = _("Cards... I foresaw this.\nBut no, not today.");
static const u8 sText_Team1[] = _("Hah! TEAM members play cards too!\nI'll crush you at this as well!");
static const u8 sText_Team2[] = _("Our boss says cards build strategy.\nGet ready!");
static const u8 sText_Team3[] = _("I was getting bored standing guard.\nLet's duel!");
static const u8 sText_TeamNo1[] = _("Cards? We're busy with our plans!\nGet lost!");
static const u8 sText_TeamNo2[] = _("No cards on duty, kid.\nBeat it!");
static const u8 sText_Staff1[] = _("I'm sorry, I'm on duty right now.\nI can't play cards at work.");
static const u8 sText_Staff2[] = _("Thank you, but I don't play cards.\nPlease come again!");
static const u8 sText_Staff3[] = _("I'm afraid card duels aren't allowed\nat the counter.");
static const u8 sText_BattleFirst[] = _("Cards? First, a MOéMON battle!\nBeat me, and then we'll talk!");
static const u8 sText_BattleFirstPolite[] = _("A card duel? Only after a proper\nMOéMON battle, if you please.");

static const u8 *const sAccept[CAT_COUNT][3] = {
    [CAT_KID] = {sText_Kid1, sText_Kid2, sText_Kid3},
    [CAT_STYLISH] = {sText_Stylish1, sText_Stylish2, sText_Stylish3},
    [CAT_TOUGH] = {sText_Tough1, sText_Tough2, sText_Tough3},
    [CAT_SEA] = {sText_Sea1, sText_Sea2, sText_Sea3},
    [CAT_REFINED] = {sText_Refined1, sText_Refined2, sText_Refined3},
    [CAT_BRAINY] = {sText_Brainy1, sText_Brainy2, sText_Brainy3},
    [CAT_TEAM] = {sText_Team1, sText_Team2, sText_Team3},
};
static const u8 *const sDecline[CAT_COUNT][3] = {
    [CAT_KID] = {sText_KidNo1, sText_KidNo2},
    [CAT_STYLISH] = {sText_StylishNo1, sText_StylishNo2},
    [CAT_TOUGH] = {sText_ToughNo1, sText_ToughNo2},
    [CAT_SEA] = {sText_SeaNo1, sText_SeaNo2},
    [CAT_REFINED] = {sText_RefinedNo1, sText_RefinedNo2},
    [CAT_BRAINY] = {sText_BrainyNo1, sText_BrainyNo2},
    [CAT_TEAM] = {sText_TeamNo1, sText_TeamNo2},
    [CAT_STAFF] = {sText_Staff1, sText_Staff2, sText_Staff3},
};

static u32 Hash(u32 x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static u8 Badges(void)
{
    u8 i, n = 0;
    for (i = 0; i < NUM_BADGES; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) != 0;
    return n;
}

unsigned Tcg_Today(void)
{
    RtcCalcLocalTime();
    return (unsigned)gLocalTime.days;
}

static const struct TcgDeckDefinition *DeckNamed(const char *name)
{
    unsigned i;
    for (i = 0; i < gTcgDeckDefinitionCount; i++)
        if (!strcmp(gTcgDeckDefinitions[i].name, name))
            return &gTcgDeckDefinitions[i];
    return NULL;
}

// ---------------------------------------------------------------------------
// CARD SCALING: who brings which deck (include/tcg/scale.h)

// A touch weaker than the player for ordinary card players (about 55% for the player), a real test from the special
// trainers (about 45%), counting how they play; badges cap the ordinary ones (a strong deck early on still meets the
// early decks, and then they play more carefully)
#define SCALE_FAVOR 35
#define SCALE_BOSS_FAVOR (-35)
static const s16 sBadgeCap[NUM_BADGES + 1] = {1460, 1490, 1530, 1570, 1610, 1650, 1690, 1730, 2000};

static int PlayerRating(const uint16_t *mine, int *deckRating)
{
    struct TcgCollection *c = Tcg_GetCollection();
    char reason[64];
    if (!mine)
        mine = c->decks[c->activeDeck].cards;
    // (no legal deck yet: as strong as a starter)
    *deckRating = Tcg_DeckValidate(mine, NULL, reason, sizeof(reason)) ? TcgScale_RateDeck(mine)
                                                                         : gTcgDeckRating[DeckNamed("CharmanderAndFriends") - gTcgDeckDefinitions];
    return *deckRating + c->skill;
}

// The candidate closest to the target; among those nearly as close, the person's own pick
struct Candidate
{
    const uint16_t *deck;
    int rating;
};
static const struct Candidate *Closest(const struct Candidate *list, unsigned count, int target, u32 pick)
{
    unsigned i, near = 0, best = 0xFFFF;
    const struct Candidate *chosen = NULL;
    for (i = 0; i < count; i++)
    {
        unsigned d = (unsigned)abs(list[i].rating - target);
        if (d < best)
            best = d;
    }
    for (i = 0; i < count; i++)
        near += (unsigned)abs(list[i].rating - target) <= best + 30;
    pick %= near ? near : 1;
    for (i = 0; i < count; i++)
        if ((unsigned)abs(list[i].rating - target) <= best + 30 && !pick--)
            chosen = &list[i];
    return chosen;
}

void Tcg_FitOpponent(struct TcgOpponent *o, const uint16_t *mine)
{
    struct Candidate list[32];
    const struct Candidate *chosen;
    unsigned n = 0, i, t, k;
    int you, deckRating, target;
    if (o->pool == TCG_SCALE_POOL_NONE)
        return;
    you = PlayerRating(mine, &deckRating) + TcgScale_LevelRating(2);   /* (the player as the SKILLED level) */
    o->deckRating = (s16)deckRating;
    // how they play: their best for the special trainers, by the badges for the rest (some more loosely)
    o->level = o->pool == TCG_SCALE_POOL_SPECIAL ? 3 : Badges() >= 4 ? 2 : 1;
    if (o->pool != TCG_SCALE_POOL_SPECIAL && (o->pick >> 17) % 4 == 0)
        o->level--;
again:
    if (o->pool == TCG_SCALE_POOL_SPECIAL)
    {
        list[n].deck = gTcgSpecialDecks[o->special].cards;
        list[n++].rating = gTcgSpecialRating[o->special];
        // (off: their own deck, as it always was)
        for (i = 0; gMpConfig.cardScaling && i < gTcgSpecialVariantCount && n < ARRAY_COUNT(list); i++)
            if (gTcgSpecialVariantOwner[i] == o->special)
            {
                list[n].deck = gTcgSpecialVariants[i].cards;
                list[n++].rating = gTcgVariantRating[i];
            }
        target = you - SCALE_BOSS_FAVOR - TcgScale_LevelRating(o->level);
    }
    else if (gMpConfig.cardScaling)
    {
        // Every deck of their kind, any badge tier, up to what the badges allow
        int cap = sBadgeCap[Badges()];
        for (t = 0; t < 4; t++)
            for (k = 0; k < 3; k++)
            {
                const struct TcgDeckDefinition *def = sPools[o->pool][t][k] ? DeckNamed(sPools[o->pool][t][k]) : NULL;
                bool8 seen = FALSE;
                if (!def)
                    continue;
                for (i = 0; i < n; i++)
                    seen |= list[i].deck == def->cards;
                if (!seen && n < ARRAY_COUNT(list))
                {
                    list[n].deck = def->cards;
                    list[n++].rating = gTcgDeckRating[def - gTcgDeckDefinitions];
                }
            }
        target = you - SCALE_FAVOR - TcgScale_LevelRating(o->level);
        if (target > cap)
        {
            // the decks they may bring this early fall well short: they play more carefully instead (SKILLED at most)
            if (target - cap > 50 && o->level < 2)
            {
                o->level++;
                n = 0;
                goto again;
            }
            target = cap;
        }
    }
    else
    {
        // By badges (CARD SCALING off): their tier's decks
        u8 tier = Badges();
        tier = tier >= 7 ? 3 : tier >= 4 ? 2 : tier >= 2 ? 1 : 0;
        for (k = 0; k < 3; k++)
        {
            const struct TcgDeckDefinition *def = sPools[o->pool][tier][k] ? DeckNamed(sPools[o->pool][tier][k]) : NULL;
            if (def)
            {
                list[n].deck = def->cards;
                list[n++].rating = gTcgDeckRating[def - gTcgDeckDefinitions];
            }
        }
        target = 0;
    }
    if (!n)
    {
        const struct TcgDeckDefinition *def = DeckNamed("CharmanderAndFriends");
        list[n].deck = def->cards;
        list[n++].rating = gTcgDeckRating[def - gTcgDeckDefinitions];
    }
    if (o->pool == TCG_SCALE_POOL_SPECIAL && !gMpConfig.cardScaling)
        chosen = &list[0];
    else if (gMpConfig.cardScaling)
        chosen = Closest(list, n, target, o->pick);
    else
        chosen = &list[(o->pick >> 12) % n];
    o->deck = chosen->deck;
    o->rating = (s16)chosen->rating;
    printf("[tcg] scaling %s: you %d (deck %d, skill %d), target %d, %s %d of %u decks, level %u\n",
           gMpConfig.cardScaling ? "on" : "off", you, deckRating, Tcg_GetCollection()->skill, target, o->name,
           o->rating, n, o->level);
}

// Which mat a kind of deck is played on
static u8 MatFor(u8 deck)
{
    switch (deck)
    {
    case DECK_GRASS:     return 2;   // TCG_MAT_FOREST
    case DECK_WATER:     return 1;   // BEACH
    case DECK_FIGHTING:  return 3;   // CAVE
    case DECK_FIRE:      return 3;
    case DECK_LIGHTNING: return 4;   // CITY
    case DECK_PSYCHIC:   return 6;   // NIGHT
    case DECK_COLORLESS: return 5;   // SKY
    }
    return 0;
}

// The NPC's sprite as it is on screen (OBJ VRAM and its palette): their portrait
static void CapturePortrait(struct TcgOpponent *o, struct ObjectEvent *npc)
{
    static const u8 sDims[3][4][2] = {
        {{8, 8}, {16, 16}, {32, 32}, {64, 64}},   // square
        {{16, 8}, {32, 8}, {32, 16}, {64, 32}},   // wide
        {{8, 16}, {8, 32}, {16, 32}, {32, 64}},   // tall
    };
    struct Sprite *sprite;
    int w, h, x, y;
    const u8 *tiles;
    const u16 *pal;
    o->portraitW = o->portraitH = 0;
    if (npc->spriteId >= MAX_SPRITES)
        return;
    sprite = &gSprites[npc->spriteId];
    if (sprite->oam.shape > 2)
        return;
    w = sDims[sprite->oam.shape][sprite->oam.size][0];
    h = sDims[sprite->oam.shape][sprite->oam.size][1];
    if (w > 32 || h > 32)
        return;
    tiles = (const u8 *)OBJ_VRAM0 + sprite->oam.tileNum * TILE_SIZE_4BPP;
    pal = &gPlttBufferUnfaded[OBJ_PLTT_ID(sprite->oam.paletteNum)];
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            int tile = (y / 8) * (w / 8) + x / 8, px = (y % 8) * 8 + x % 8;
            u8 c = tiles[tile * TILE_SIZE_4BPP + px / 2];
            u16 rgb;
            c = (px & 1) ? c >> 4 : c & 15;
            if (!c)
            {
                o->portrait[y * w + x] = 0;
                continue;
            }
            rgb = pal[c];
            o->portrait[y * w + x] = 0xFF000000u | (u32)((rgb & 31) * 255 / 31) << 16
                                   | (u32)(((rgb >> 5) & 31) * 255 / 31) << 8 | (u32)(((rgb >> 10) & 31) * 255 / 31);
        }
    o->portraitW = w;
    o->portraitH = h;
}

// The challenge screens show the opponent's trainer battle picture, cut to the window. Townsfolk without one
// get the battle picture of the trainer they look most like.
#define PIC(gfx, pic) [OBJ_EVENT_GFX_##gfx] = TRAINER_PIC_##pic + 1
static const u8 sPicOfGfx[] = {
    PIC(BRENDAN_NORMAL, BRENDAN), PIC(BRENDAN_MACH_BIKE, BRENDAN), PIC(BRENDAN_ACRO_BIKE, BRENDAN),
    PIC(BRENDAN_SURFING, BRENDAN), PIC(BRENDAN_FIELD_MOVE, BRENDAN), PIC(RIVAL_BRENDAN_NORMAL, BRENDAN),
    PIC(RIVAL_BRENDAN_MACH_BIKE, BRENDAN), PIC(RIVAL_BRENDAN_ACRO_BIKE, BRENDAN), PIC(LINK_BRENDAN, BRENDAN),
    PIC(MAY_NORMAL, MAY), PIC(MAY_MACH_BIKE, MAY), PIC(MAY_ACRO_BIKE, MAY), PIC(MAY_SURFING, MAY),
    PIC(MAY_FIELD_MOVE, MAY), PIC(RIVAL_MAY_NORMAL, MAY), PIC(RIVAL_MAY_MACH_BIKE, MAY),
    PIC(RIVAL_MAY_ACRO_BIKE, MAY), PIC(LINK_MAY, MAY), PIC(LINK_RS_BRENDAN, RS_BRENDAN), PIC(LINK_RS_MAY, RS_MAY),
    PIC(NINJA_BOY, NINJA_BOY), PIC(TWIN, TWINS), PIC(RICH_BOY, RICH_BOY), PIC(POKEFAN_F, POKEFAN_F),
    PIC(POKEFAN_M, POKEFAN_M), PIC(EXPERT_M, EXPERT_M), PIC(EXPERT_F, EXPERT_F), PIC(CAMPER, CAMPER),
    PIC(PICNICKER, PICNICKER), PIC(YOUNGSTER, YOUNGSTER), PIC(BUG_CATCHER, BUG_CATCHER), PIC(PSYCHIC_M, PSYCHIC_M),
    PIC(SCHOOL_KID_M, SCHOOL_KID_M), PIC(MANIAC, POKEMANIAC), PIC(HEX_MANIAC, HEX_MANIAC), PIC(SWIMMER_M, SWIMMER_M),
    PIC(SWIMMER_F, SWIMMER_F), PIC(BLACK_BELT, BLACK_BELT), PIC(BEAUTY, BEAUTY), PIC(LASS, LASS),
    PIC(GENTLEMAN, GENTLEMAN), PIC(SAILOR, SAILOR), PIC(FISHERMAN, FISHERMAN),
    PIC(RUNNING_TRIATHLETE_M, RUNNING_TRIATHLETE_M), PIC(RUNNING_TRIATHLETE_F, RUNNING_TRIATHLETE_F),
    PIC(CYCLING_TRIATHLETE_M, CYCLING_TRIATHLETE_M), PIC(CYCLING_TRIATHLETE_F, CYCLING_TRIATHLETE_F),
    PIC(TUBER_F, TUBER_F), PIC(TUBER_M, TUBER_M), PIC(TUBER_M_SWIMMING, TUBER_M), PIC(HIKER, HIKER),
    PIC(AQUA_MEMBER_M, AQUA_GRUNT_M), PIC(AQUA_MEMBER_F, AQUA_GRUNT_F), PIC(MAGMA_MEMBER_M, MAGMA_GRUNT_M),
    PIC(MAGMA_MEMBER_F, MAGMA_GRUNT_F), PIC(ARCHIE, AQUA_LEADER_ARCHIE), PIC(MAXIE, MAGMA_LEADER_MAXIE),
    PIC(SIDNEY, ELITE_FOUR_SIDNEY), PIC(PHOEBE, ELITE_FOUR_PHOEBE), PIC(GLACIA, ELITE_FOUR_GLACIA),
    PIC(DRAKE, ELITE_FOUR_DRAKE), PIC(ROXANNE, LEADER_ROXANNE), PIC(BRAWLY, LEADER_BRAWLY),
    PIC(WATTSON, LEADER_WATTSON), PIC(FLANNERY, LEADER_FLANNERY), PIC(NORMAN, LEADER_NORMAN),
    PIC(WINONA, LEADER_WINONA), PIC(LIZA, LEADER_TATE_AND_LIZA), PIC(TATE, LEADER_TATE_AND_LIZA),
    PIC(JUAN, LEADER_JUAN), PIC(WALLACE, CHAMPION_WALLACE), PIC(STEVEN, STEVEN), PIC(WALLY, WALLY),
    PIC(ANABEL, SALON_MAIDEN_ANABEL), PIC(TUCKER, DOME_ACE_TUCKER), PIC(GRETA, ARENA_TYCOON_GRETA),
    PIC(SPENSER, PALACE_MAVEN_SPENSER), PIC(NOLAND, FACTORY_HEAD_NOLAND), PIC(LUCY, PIKE_QUEEN_LUCY),
    PIC(BRANDON, PYRAMID_KING_BRANDON), PIC(RED, RED), PIC(LEAF, LEAF),
    // the closest look-alikes
    PIC(BOY_1, YOUNGSTER), PIC(BOY_2, SCHOOL_KID_M), PIC(BOY_3, CAMPER), PIC(GIRL_1, LASS), PIC(GIRL_2, PICNICKER),
    PIC(GIRL_3, SCHOOL_KID_F), PIC(LITTLE_BOY, TUBER_M), PIC(LITTLE_BOY_3, TUBER_M), PIC(LITTLE_GIRL, TUBER_F),
    PIC(GAMEBOY_KID, YOUNGSTER), PIC(QUINTY_PLUMP, POKEMANIAC), PIC(FAT_MAN, POKEFAN_M), PIC(WOMAN_1, LADY),
    PIC(WOMAN_2, COOLTRAINER_F), PIC(WOMAN_3, AROMA_LADY), PIC(WOMAN_4, PARASOL_LADY), PIC(WOMAN_5, POKEMON_BREEDER_F),
    PIC(MAN_1, COOLTRAINER_M), PIC(MAN_2, GUITARIST), PIC(MAN_3, POKEMON_BREEDER_M), PIC(MAN_4, GENTLEMAN),
    PIC(MAN_5, BIRD_KEEPER), PIC(OLD_MAN, EXPERT_M), PIC(OLD_WOMAN, EXPERT_F), PIC(HOT_SPRINGS_OLD_WOMAN, EXPERT_F),
    PIC(COOK, GENTLEMAN), PIC(BARD, GUITARIST), PIC(ARTIST, GENTLEMAN), PIC(REPORTER_M, INTERVIEWER),
    PIC(REPORTER_F, INTERVIEWER), PIC(CAMERAMAN, INTERVIEWER), PIC(SCIENTIST_1, COLLECTOR), PIC(SCIENTIST_2, COLLECTOR),
    PIC(DEVON_EMPLOYEE, COOLTRAINER_M), PIC(PROF_BIRCH, GENTLEMAN), PIC(MOM, POKEFAN_F), PIC(SCOTT, GENTLEMAN),
    PIC(MYSTERY_GIFT_MAN, GENTLEMAN), PIC(CONTEST_JUDGE, GENTLEMAN), PIC(TEALA, LADY), PIC(ROOFTOP_SALE_WOMAN, LADY),
    PIC(NURSE, LADY), PIC(UNION_ROOM_NURSE, LADY), PIC(MART_EMPLOYEE, COOLTRAINER_M),
    PIC(LINK_RECEPTIONIST, LADY),
};
#undef PIC

static const u8 *NpcScript(const struct ObjectEvent *npc)
{
    u8 i;
    if (!gMapHeader.events)
        return NULL;
    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
        if (gMapHeader.events->objectEvents[i].localId == npc->localId)
            return gMapHeader.events->objectEvents[i].script;
    return NULL;
}

// A battle picture (TRAINER_PIC_*, the co-op slots too) as 64x64 ARGB
static bool8 DecodeTrainerPic(struct TcgOpponent *o, u16 pic)
{
    static u8 tiles[TRAINER_PIC_SIZE];
    u16 pal[16];
    int x, y;
    if (pic > TRAINER_PIC_MP_OPPONENT || !gTrainerFrontPicTable[pic].data || !gTrainerFrontPicPaletteTable[pic].data)
        return FALSE;
    LZ77UnCompWram(gTrainerFrontPicTable[pic].data, tiles);
    LZ77UnCompWram(gTrainerFrontPicPaletteTable[pic].data, pal);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            u8 c = tiles[((y / 8) * 8 + x / 8) * TILE_SIZE_4BPP + (y % 8) * 4 + (x % 8) / 2];
            u16 rgb;
            c = (x & 1) ? c >> 4 : c & 15;
            rgb = pal[c];
            o->portrait[y * 64 + x] = !c ? 0 : 0xFF000000u | (u32)((rgb & 31) * 255 / 31) << 16
                                    | (u32)(((rgb >> 5) & 31) * 255 / 31) << 8 | (u32)(((rgb >> 10) & 31) * 255 / 31);
        }
    o->portraitW = o->portraitH = 64;
    return TRUE;
}

// The challenge screen's picture of someone on the map: their battle picture, else their sprite
static void CaptureChallengePic(struct TcgOpponent *o, struct ObjectEvent *npc)
{
    const u8 *script = NpcScript(npc);
    u16 pic = 0xFFFF;
    if (npc->trainerType != TRAINER_TYPE_NONE && script && script[0] == 0x5C)   // trainerbattle
    {
        u16 id = script[2] | script[3] << 8;
        if (id < TRAINERS_COUNT)
            pic = gTrainers[id].trainerPic;
    }
    if (pic == 0xFFFF && npc->graphicsId < ARRAY_COUNT(sPicOfGfx) && sPicOfGfx[npc->graphicsId])
        pic = sPicOfGfx[npc->graphicsId] - 1;
    printf("[tcg] challenge picture: graphics %u, trainer picture %d\n", npc->graphicsId, pic == 0xFFFF ? -1 : pic);
    if (pic != 0xFFFF && DecodeTrainerPic(o, pic))
        return;
    CapturePortrait(o, npc);
}

// The portrait of someone on the map, for the other card games' screens (Triple Triad's CARD QUEEN)
void Tcg_PortraitOfObject(unsigned objectId, uint32_t *pixels, uint8_t *w, uint8_t *h)
{
    static struct TcgOpponent sFace;
    struct TcgOpponent *o = &sFace;
    *w = *h = 0;
    if (objectId >= OBJECT_EVENTS_COUNT || !gObjectEvents[objectId].active)
        return;
    CapturePortrait(o, &gObjectEvents[objectId]);
    *w = o->portraitW;
    *h = o->portraitH;
    memcpy(pixels, o->portrait, sizeof(o->portrait));
}

// A friend's portrait: their look's battle picture, else their avatar on the map if it's on screen
void Tcg_PortraitOfPlayer(unsigned id, struct TcgOpponent *o)
{
    u8 objectId = MP_GetRemoteObjectEventId(id);
    u16 pic = MP_Avatar_RemoteFrontPic((u8)id);
    o->portraitW = o->portraitH = 0;
    if (pic != 0xFFFF && DecodeTrainerPic(o, pic))
        return;
    if (objectId < OBJECT_EVENTS_COUNT && gObjectEvents[objectId].active)
        CapturePortrait(o, &gObjectEvents[objectId]);
}

// What happens when you ask them; NULL when they're not someone to ask
enum { ASK_DUEL, ASK_NO, ASK_YGO, ASK_WHICH, ASK_TT };
enum { GAME_PKMN = 1, GAME_YGO = 2, GAME_TT = 4 };
static struct TcgOpponent sOpponent;
static struct TtOpponent sTt;     // the same person's Triple Triad side
static u8 sGames;                 // ASK_WHICH: the games they offer (GAME_*)
static struct YgoOpponent sYgo;   // the same person's Yu-Gi-Oh! side
static const u8 *sYgoLine;        // and what they say when they play both
static const u8 sText_Which[] = _("Want a card duel? MOéMON cards or\nYu-Gi-Oh!? I'm up for either!");
// The GAME CENTER's CARD SHOP (RUSTBORO CITY 2F): the people at its table always play
static bool8 InGameCenterCardShop(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_RUSTBORO_CITY_GAME_CENTER_2F)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_2F);
}

static const u8 *AskCards(struct ObjectEvent *npc, const u8 *script, u8 *answer, u32 key, u32 h);
// Who plays what: Triple Triad first (src/tt/tt_field.c: theirs only, or besides another game), then the others
static const u8 *Ask(struct ObjectEvent *npc, const u8 *script, u8 *answer)
{
    u32 key = (u32)gSaveBlock1Ptr->location.mapGroup << 24 | (u32)gSaveBlock1Ptr->location.mapNum << 16
            | (u32)npc->localId << 8 | 1;
    u32 h = Hash(key);
    const u8 *ttLine, *line;
    int ttRefused, staff = script == EventScript_CardClub || script == EventScript_YgoClub, ttOk;
    int tt = Tt_FieldPick(npc, script, h, key, staff, &sTt, &ttLine, &ttRefused);
    if (tt == TT_PLAYS_ONLY)
    {
        *answer = ttRefused ? ASK_NO : ASK_TT;
        return ttLine;
    }
    ttOk = tt == TT_PLAYS_ALSO && !ttRefused;
    line = AskCards(npc, script, answer, key, h);
    if (!ttOk)
        return line ? line : (tt && ttRefused) ? (*answer = ASK_NO, ttLine) : NULL;
    // They play Triple Triad too
    switch (line ? *answer : ASK_NO)
    {
    case ASK_DUEL: sGames = GAME_PKMN | GAME_TT; break;
    case ASK_YGO: sGames = GAME_YGO | GAME_TT; break;
    case ASK_WHICH: sGames = sGames | GAME_TT; break;
    default:
        *answer = ASK_TT;   // (the other game said no: Triple Triad it is)
        return ttLine ? ttLine : line;
    }
    *answer = ASK_WHICH;
    // a special player's own words; anyone else asks the way their kind does
    if (sTt.special != TT_SPECIAL_NONE && ttLine)
        return ttLine;
    {
        const u8 *which = Tt_WhichLine(sGames, h);
        return which ? which : line;
    }
}

static const u8 *AskCards(struct ObjectEvent *npc, const u8 *script, u8 *answer, u32 key, u32 h)
{
    u8 i, cat = CAT_NONE, deck = DECK_MIXED;
    const char *label = NULL;
    struct TcgOpponent *o = &sOpponent;

    memset(o, 0, sizeof(*o) - sizeof(o->portrait));
    // Yu-Gi-Oh! duelists (src/ygo/ygo_field.c): theirs only, or both games
    {
        const u8 *line;
        int refused, staff = script == EventScript_CardClub || script == EventScript_YgoClub;
        int play = Ygo_FieldPick(npc, script, h, key, staff, &sYgo, &line, &refused);
        if (play == YGO_PLAYS_YGO)
        {
            *answer = refused ? ASK_NO : ASK_YGO;
            return line;
        }
        if (play == YGO_PLAYS_BOTH)
        {
            if (staff)
            {
                // the Pokemon side of the CARD CLUB staff: a deck of any kind
                snprintf(o->name, sizeof(o->name), "CARD CLUB");
                o->special = TCG_SPECIAL_NONE;
                o->pool = (h >> 4) % DECK_MIXED;
                o->pick = h;
                Tcg_FitOpponent(o, NULL);
                o->key = key;
                o->prizes = 6;
                o->mat = 0;
                o->music = MUS_VS_TRAINER;
                *answer = ASK_WHICH;
                sGames = GAME_PKMN | GAME_YGO;
                return sText_Which;
            }
            sYgoLine = line;
        }
        else
            sYgoLine = NULL;
    }
    // Someone with a deck of their own
    for (i = 0; i < TCG_SPECIAL_COUNT; i++)
    {
        const struct Special *s = &sSpecials[i];
        bool8 ok = TRUE;
        if (npc->graphicsId != s->gfx && (!s->gfx2 || npc->graphicsId != s->gfx2))
            continue;
        if (s->req == REQ_BADGE)
            ok = FlagGet(FLAG_BADGE01_GET + s->reqArg);
        else if (s->req == REQ_TRAINER)
            ok = FlagGet(TRAINER_FLAGS_START + s->reqArg);
        else if (s->req == REQ_CHAMPION)
            ok = FlagGet(FLAG_IS_CHAMPION);
        if (!ok)
        {
            *answer = ASK_NO;
            return s->refuse;
        }
        snprintf(o->name, sizeof(o->name), "%s", s->name ? s->name
                 : npc->graphicsId == OBJ_EVENT_GFX_RIVAL_MAY_NORMAL ? "MAY" : "BRENDAN");
        snprintf(o->title, sizeof(o->title), "%s", s->title);
        o->deck = gTcgSpecialDecks[i].cards;
        o->key = 0xFF000000u | i;
        o->special = i;
        o->mat = s->mat;
        o->matType = s->matType;
        o->music = s->music;
        o->promo = gTcgSpecialPromos[i];
        o->pack = i % TCG_PACK_TYPES;
        o->pool = TCG_SCALE_POOL_SPECIAL;
        o->pick = h;
        o->prizes = 6;   // (the full game: LEADERS and the like)
        Tcg_FitOpponent(o, NULL);
        *answer = sYgoLine ? ASK_WHICH : ASK_DUEL;
        sGames = sYgoLine ? GAME_PKMN | GAME_YGO : GAME_PKMN;
        return sYgoLine ? sYgoLine : s->accept;
    }
    o->special = TCG_SPECIAL_NONE;
    for (i = 0; i < ARRAY_COUNT(sPeople); i++)
        if (sPeople[i].gfx == npc->graphicsId)
        {
            cat = sPeople[i].cat;
            deck = sPeople[i].deck;
            label = sPeople[i].label;
            break;
        }
    if (cat == CAT_NONE)
        return NULL;
    if (cat == CAT_STAFF)
    {
        *answer = ASK_NO;
        return sDecline[CAT_STAFF][h % 3];
    }
    if (npc->trainerType != TRAINER_TYPE_NONE && script && script[0] == 0x5C)   // trainerbattle: a real trainer
    {
        u16 id = script[2] | script[3] << 8;
        if (!FlagGet(TRAINER_FLAGS_START + id))
        {
            *answer = ASK_NO;
            return cat == CAT_REFINED || cat == CAT_STYLISH ? sText_BattleFirstPolite : sText_BattleFirst;
        }
        if (id < TRAINERS_COUNT)
        {
            char cls[16], who[16];
            MP_GameToAscii(cls, gTrainerClassNames[gTrainers[id].trainerClass], sizeof(cls));
            MP_GameToAscii(who, gTrainers[id].trainerName, sizeof(who));
            snprintf(o->name, sizeof(o->name), "%s %s", cls, who);
        }
    }
    else if (h % 100 >= 65 && !InGameCenterCardShop())   // townsfolk: most of them play (the CARD SHOP's all do)
    {
        *answer = ASK_NO;
        return sDecline[cat][(h >> 8) % 2];
    }
    if (!o->name[0])
        snprintf(o->name, sizeof(o->name), "%s", label);
    if (deck == DECK_MIXED)
        deck = (h >> 4) % DECK_MIXED;
    o->pool = deck;
    o->pick = h;
    Tcg_FitOpponent(o, NULL);
    o->key = key;
    // How many Prize cards they like: 6 and 3 are what most play, a few play 4 or 2
    {
        u32 r = (h >> 12) % 100;
        o->prizes = r < 42 ? 6 : r < 84 ? 3 : r < 92 ? 4 : 2;
    }
    o->mat = cat == CAT_TEAM ? (deck == DECK_WATER ? 9 : 10) : MatFor(deck);
    o->music = cat == CAT_TEAM ? MUS_VS_AQUA_MAGMA : MUS_VS_TRAINER;
    o->pack = deck == DECK_FIRE || deck == DECK_FIGHTING ? 0 : deck == DECK_GRASS || deck == DECK_WATER ? 1
            : deck == DECK_PSYCHIC ? 2 : 3;
    *answer = ASK_DUEL;
    return sAccept[cat][(h >> 20) % 3];
}

// ---------------------------------------------------------------------------
// The conversation

#define tState   data[0]
#define tTimer   data[1]
#define tObject  data[2]
#define tAnswer  data[3]
#define tWindow  data[4]
#define tChoice  data[5]

static const u8 *sLine;
static const u8 sText_ChoicePokemon[] = _("MOéMON CARDS");
static const u8 sText_ChoiceYgo[] = _("YU-GI-OH!");
static const u8 sText_ChoiceTt[] = _("TRIPLE TRIAD");
static struct MenuAction sGameChoices[3];
static u8 sGameOf[3];   // the menu's rows: GAME_*
static u8 GameChoices(void)
{
    u8 n = 0;
    if (sGames & GAME_PKMN)
        sGameChoices[n].text = sText_ChoicePokemon, sGameOf[n++] = GAME_PKMN;
    if (sGames & GAME_YGO)
        sGameChoices[n].text = sText_ChoiceYgo, sGameOf[n++] = GAME_YGO;
    if (sGames & GAME_TT)
        sGameChoices[n].text = sText_ChoiceTt, sGameOf[n++] = GAME_TT;
    return n;
}
// The row the case was last open on
static u8 LastSideRow(u8 n)
{
    u8 want = Tt_GetCollection()->flags & TT_FLAG_CASE_SIDE ? GAME_TT
            : Ygo_GetCollection()->flags & YGO_FLAG_CASE_SIDE ? GAME_YGO : GAME_PKMN, i;
    for (i = 0; i < n; i++)
        if (sGameOf[i] == want)
            return i;
    return 0;
}
static void OpenTtChallenge(void)
{
    sTt.portraitW = sOpponent.portraitW;
    sTt.portraitH = sOpponent.portraitH;
    memcpy(sTt.portrait, sOpponent.portrait, sizeof(sTt.portrait));
    TtUi_OpenChallenge(&sTt);
}
static void OpenYgoChallenge(void)
{
    sYgo.portraitW = sOpponent.portraitW;
    sYgo.portraitH = sOpponent.portraitH;
    memcpy(sYgo.portrait, sOpponent.portrait, sizeof(sYgo.portrait));
    YgoUi_OpenChallenge(&sYgo);
}
static bool8 sFromScript;   // Tcg_TalkChallenge: the script waits for the talk to end

static void FinishTalk(u8 taskId)
{
    if (sFromScript)
    {
        sFromScript = FALSE;
        ScriptContext_Enable();
    }
    HideFieldMessageBox();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

static void Task_CardChallenge(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    struct ObjectEvent *npc = &gObjectEvents[tObject];
    switch (tState)
    {
    case 0:
        // They turn to you (everyone else stays still)
        UnfreezeObjectEvent(npc);
        ObjectEventFaceOppositeDirection(npc, GetPlayerFacingDirection());
        tState++;
        break;
    case 1:
        if (++tTimer < 6)
            break;
        ObjectEventClearHeldMovementIfFinished(npc);
        FreezeObjectEvent(npc);
        CaptureChallengePic(&sOpponent, npc);
        HideMapNamePopUpWindow();
        MP_ShowFieldMessageFitted(sLine);
        tState++;
        break;
    case 2:
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        PlaySE(SE_SELECT);
        if (tAnswer == ASK_WHICH)
        {
            // which game: a choice beside the message, a line for each game they play
            u8 n = GameChoices();
            tWindow = CreateWindowFromRect(17, 11 - 2 * n, 12, 2 * n);
            SetStandardWindowBorderStyle(tWindow, FALSE);
            PrintMenuTable(tWindow, n, sGameChoices);
            InitMenuInUpperLeftCornerNormal(tWindow, n, LastSideRow(n));
            ScheduleBgCopyTilemapToVram(0);
            tState = 4;
            break;
        }
        if (tAnswer != ASK_DUEL && tAnswer != ASK_YGO && tAnswer != ASK_TT)
        {
            FinishTalk(taskId);
            break;
        }
        HideFieldMessageBox();
        if (tAnswer == ASK_YGO)
            OpenYgoChallenge();
        else if (tAnswer == ASK_TT)
            OpenTtChallenge();
        else
            Tcg_OpenNpcChallenge(&sOpponent);
        tState++;
        break;
    case 4:
        tChoice = Menu_ProcessInputNoWrap();
        if (tChoice == MENU_NOTHING_CHOSEN)
            break;
        ClearStdWindowAndFrameToTransparent(tWindow, TRUE);
        RemoveWindow(tWindow);
        if (tChoice == MENU_B_PRESSED)
        {
            FinishTalk(taskId);
            break;
        }
        HideFieldMessageBox();
        if (sGameOf[tChoice] == GAME_YGO)
            OpenYgoChallenge();
        else if (sGameOf[tChoice] == GAME_TT)
            OpenTtChallenge();
        else
            Tcg_OpenNpcChallenge(&sOpponent);
        tState = 3;
        break;
    case 3:
        // (the card screens run instead of the game until they close)
        if (!Tcg_IsActive())
            FinishTalk(taskId);
        break;
    }
}

int Tcg_FieldChallenge(void)
{
    s16 x, y;
    u8 objectId, answer = ASK_NO, i, taskId;
    const u8 *script = NULL, *line;
    if (Tcg_IsActive())
        return 0;
    if (Tcg_FieldAskFriend())   // a friend in front: an online duel request (tcg_online.c)
        return 1;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    objectId = GetObjectEventIdByPosition(x, y, PlayerGetElevation());
    if (objectId == OBJECT_EVENTS_COUNT || !gMapHeader.events || MP_Coop_IsNpcHeld(&gObjectEvents[objectId]))
        return 0;
    // (the map's own templates: talking helpers would change who was last spoken to)
    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
        if (gMapHeader.events->objectEvents[i].localId == gObjectEvents[objectId].localId)
        {
            script = gMapHeader.events->objectEvents[i].script;
            break;
        }
    line = Ask(&gObjectEvents[objectId], script, &answer);
    if (!line)
        return 0;
    sLine = line;
    printf("[tcg] asked %s: %s\n", answer == ASK_YGO ? sYgo.name : answer == ASK_TT ? sTt.name
                                  : sOpponent.name[0] ? sOpponent.name : "someone",
           answer == ASK_DUEL ? "duel" : answer == ASK_YGO ? "yu-gi-oh" : answer == ASK_TT ? "triple triad"
                               : answer == ASK_WHICH ? "which game" : "no");
    taskId = CreateTask(Task_CardChallenge, 80);
    gTasks[taskId].tObject = objectId;
    gTasks[taskId].tAnswer = answer;
    return 1;
}

#undef tState
#undef tTimer
#undef tObject
#undef tAnswer
#undef tWindow
#undef tChoice

// A script's speaker asked for a duel (the GAME CENTER's card players; the script waits for the talk to end)
void Tcg_TalkChallenge(void)
{
    sFromScript = TRUE;
    MPNet_Log("game center: duel asked");
    if (!Tcg_FieldChallenge())
    {
        sFromScript = FALSE;
        ScriptContext_Enable();
    }
}

// The GAME CENTER CARD SHOP (the script waits for the card screens to close)
void Tcg_GameCenterShop(void)
{
    MPNet_Log("game center: card shop open");
    Tcg_OpenGameCenterShop();
}

// ---------------------------------------------------------------------------
// The CARD CLUB member (OLDALE and MAUVILLE POKeMON MARTS, LILYCOVE DEPARTMENT STORE)

// VAR_RESULT: 0 hasn't got a CARD CASE, 1 has one
void Tcg_ClubState(void)
{
    gSpecialVar_Result = Tcg_GetCollection()->optedIn && CheckBagHasItem(ITEM_CARD_CASE, 1);
}

// After the case: choose the first deck (the card screens; the script waits)
void Tcg_ClubChooseDeck(void)
{
    Tcg_OpenStarterReveal();
}

void Tcg_ClubTeachingDuel(void)
{
    Tcg_GetCollection()->flags |= TCG_FLAG_TUTORIAL;
    Tcg_OpenTeachingDuel();
}

void Tcg_ClubCounter(void)
{
    Tcg_OpenClubCounter();
}

// VAR_RESULT: 1 when every card is collected and the gift is still waiting
void Tcg_ClubGiftReady(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned total, have = Tcg_SetOwned(c, TCG_SET_COUNT, &total);
    gSpecialVar_Result = c->optedIn && have == total && !(c->flags & TCG_FLAG_FULL_GIFT);
}

// The gift for collecting all 228: the promo PIKACHU that both FLIES and SURFS
// (VAR_RESULT: 0 no room, 1 in the party, 2 sent to the PC)
void Tcg_ClubGiveGift(void)
{
    static const u8 sOtName[] = _("PROMO");
    struct Pokemon *mon = AllocZeroed(sizeof(struct Pokemon));
    struct TcgCollection *c = Tcg_GetCollection();
    u16 item = ITEM_LIGHT_BALL;
    u8 ball = ITEM_PREMIER_BALL, result;
    if (!mon)
    {
        gSpecialVar_Result = 0;
        return;
    }
    CreateMon(mon, SPECIES_PIKACHU, 25, 31, FALSE, 0, OT_ID_PRESET, 20228);
    SetMonMoveSlot(mon, MOVE_THUNDERBOLT, 0);
    SetMonMoveSlot(mon, MOVE_FLY, 1);
    SetMonMoveSlot(mon, MOVE_SURF, 2);
    SetMonMoveSlot(mon, MOVE_AGILITY, 3);
    SetMonData(mon, MON_DATA_HELD_ITEM, &item);
    SetMonData(mon, MON_DATA_POKEBALL, &ball);
    SetMonData(mon, MON_DATA_OT_NAME, sOtName);
    result = GiveMonToPlayer(mon);
    Free(mon);
    gSpecialVar_Result = result == MON_GIVEN_TO_PARTY ? 1 : result == MON_GIVEN_TO_PC ? 2 : 0;
    if (gSpecialVar_Result)
    {
        c->flags |= TCG_FLAG_FULL_GIFT;
        Tcg_WriteSidecar();
    }
}
