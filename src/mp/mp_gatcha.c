// Moemon Co-op: the MoeGatcha. Shared achievements (and a few other rewards)
// give each player pulls, and everyone gets a free 10-pull every week.
//
// A full-screen slot reel: item icons, Moemon and money cards race past, then
// slowly click down onto the prize. Seven rarities (STANDARD banner):
//   N 52%  U 28%  R 13.5%  SR 5%  SSR 1.2%  UR 0.25%  EX 0.05%
// Legendary and mythical Moemon only come as UR, their shiny forms as EX.
// Pity: an SR or better at least every 40 pulls, an SSR or better every 100,
// a UR or better every 200 (EX is pure luck). A 10-pull always has an R+.
// A UR/EX is an event: the box climbs through every rarity, the screen goes
// dark, the Moemon's silhouette rises to legendary battle music, then the
// reveal (its cry, rainbow, stars) and friends online hear about it.
// Event Moemon carry the moves of real Generation 3 event Pokémon.
// Banners (L/R): STANDARD, PICKUP (a featured event Moemon each week),
// TRAINING (candies, vitamins) and POKé BALLS, each with its own pity count.
// SUPER/ULTRA RAREs: a "?" box that looks RARE, maybe a stop one short, a
// charge-up with color changes, then a burst with rays, a stamp and confetti.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "mp.h"
#include "bg.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "item.h"
#include "item_icon.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "random.h"
#include "scanline_effect.h"
#include "script.h"
#include "battle_anim.h"
#include "sound.h"
#include "m4a.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "battle_pyramid.h"
#include "fieldmap.h"
#include "new_game.h"
#include "save.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/region_map_sections.h"
#include "platform_text.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
u8 MP_GiveMonKeepOT(struct Pokemon *mon);
void MP_FieldClearVBlankHBlankCallbacks(void);
void MP_OpenMenuAtGatcha(void);

#define PULLS_KEY     "gatcha:pulls"
#define PITY_SR_KEY   "gatcha:pity"     // pulls since the last SR+
#define PITY_SSR_KEY  "gatcha:pityssr"  // ... SSR+
#define PITY_UR_KEY   "gatcha:pity7ur"  // ... UR+ (new key: the old 5-rarity count doesn't carry over)
#define FREE_WEEK_KEY "gatcha:freewk"   // week number + 1 of the last free 10-pull given
#define FREE10_KEY    "gatcha:free10"   // free 10-pulls in stock (STANDARD only, one a week)
#define FREE1_KEY     "gatcha:free1b%d" // free single pulls in stock, per banner (one a day each)
#define FREE1_DAY_KEY "gatcha:free1day" // the day (YYYYMMDD) the last daily free pulls were given
#define TICKETS_KEY   "gatcha:tickets"  // MOEGATCHA TICKETS: one pull on any banner

#define SR_PITY  40
#define SSR_PITY 100
#define UR_PITY  200

// Small helpers (no libm)
static s32 FloorF(float v)
{
    s32 i = (s32)v;
    return (v < i) ? i - 1 : i;
}

static s32 CeilF(float v)
{
    s32 i = (s32)v;
    return (v > i) ? i + 1 : i;
}

enum { R_N, R_U, R_R, R_SR, R_SSR, R_UR, R_EX, R_COUNT };
// Old names, still used for the first tiers
#define R_COMMON   R_N
#define R_UNCOMMON R_U
#define R_RARE     R_R

static const char *const sRarityNames[R_COUNT] = {"NORMAL", "UNCOMMON", "RARE", "SUPER RARE", "SUPER SUPER RARE", "ULTRA RARE", "EX"};
static const char *const sRarityShort[R_COUNT] = {"N", "U", "R", "SR", "SSR", "UR", "EX"};

// ---------------------------------------------------------------------------
// Prizes

struct EventMon
{
    const char *title;
    u16 species;
    u8 level;
    u16 moves[MAX_MON_MOVES];
    const char *otName;    // NULL = you (hatched from an egg)
    u32 otId;
    bool8 fateful;         // Mew/Deoxys obey only when met in a fateful encounter
};

enum {
    EV_JIRACHI, EV_MEW, EV_CELEBI, EV_DEOXYS, EV_ZIGZAGOON, EV_PICHU, EV_SKITTY, EV_SWABLU, EV_LATIAS, EV_LATIOS, EV_LUGIA, EV_HOOH,
    // PICKUP (OT GACHA)
    EV_G_MEW, EV_G_MEWTWO, EV_G_CELEBI, EV_G_JIRACHI, EV_G_DRAGONITE, EV_G_TYRANITAR, EV_G_SALAMENCE, EV_G_METAGROSS,
    EV_G_DITTO, EV_G_EEVEE, EV_G_FEEBAS, EV_G_DEOXYS, EV_G_HITMONLEE, EV_G_HITMONCHAN, EV_G_MOLTRES, EV_G_ZAPDOS,
    EV_G_ARTICUNO, EV_G_BULBASAUR, EV_G_CHARMANDER, EV_G_SQUIRTLE, EV_G_CHIKORITA, EV_G_TOTODILE, EV_G_CYNDAQUIL,
    EV_G_ENTEI, EV_G_RAIKOU, EV_G_SUICUNE, EV_G_LUGIA, EV_G_HOOH, EV_G_LATIOS, EV_G_LATIAS,
    // STANDARD, taking turns day by day: Generation 1, 2 and 3 events and gifts
    EV_SURF_PIKACHU, EV_FLY_PIKACHU, EV_PSYDUCK, EV_ODD_PICHU, EV_ODD_CLEFFA, EV_ODD_IGGLYBUFF, EV_ODD_SMOOCHUM,
    EV_ODD_MAGBY, EV_ODD_ELEKID, EV_ODD_TYROGUE, EV_DRATINI, EV_SHUCKIE, EV_TOGEPI, EV_ESPEON, EV_UMBREON,
    EV_GYARADOS, EV_OLD_MEW, EV_GS_CELEBI, EV_CHANNEL_JIRACHI, EV_FARAWAY_MEW, EV_BIRTH_DEOXYS, EV_XD_LUGIA,
    EV_RANDOM,         // a random Moemon (not legendary), picked at the pull: def->qty is the species
    EV_RANDOM_LEGEND,   // a random legendary, picked at the pull
};

// PICKUP Moemon: OT GACHA, ID No. 23195 (secret ID 40613)
#define GACHA_OT_ID ((40613u << 16) | 23195)
#define NO_MOVES {MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE}   // the moves it knows at that level

// Movesets of real Generation 3 event Pokémon
static const struct EventMon sEventMons[] = {
    [EV_JIRACHI]   = {"WISHMKR JIRACHI", SPECIES_JIRACHI, 5, {MOVE_WISH, MOVE_CONFUSION, MOVE_REST, MOVE_NONE}, "WISHMKR", 20043, TRUE},
    [EV_MEW]       = {"AURA MEW", SPECIES_MEW, 10, {MOVE_POUND, MOVE_NONE, MOVE_NONE, MOVE_NONE}, "AURA", 20078, TRUE},
    [EV_CELEBI]    = {"AGETO CELEBI", SPECIES_CELEBI, 10, {MOVE_CONFUSION, MOVE_RECOVER, MOVE_HEAL_BELL, MOVE_SAFEGUARD}, "AGETO", 31121, TRUE},
    [EV_DEOXYS]    = {"SPACE C DEOXYS", SPECIES_DEOXYS, 30, {MOVE_TAUNT, MOVE_PURSUIT, MOVE_PSYCHIC, MOVE_SUPERPOWER}, "SPACE C", 0, TRUE},
    [EV_ZIGZAGOON] = {"BOX ZIGZAGOON", SPECIES_ZIGZAGOON, 5, {MOVE_TACKLE, MOVE_GROWL, MOVE_TAIL_WHIP, MOVE_EXTREME_SPEED}, NULL, 0, FALSE},
    [EV_PICHU]     = {"BOX PICHU", SPECIES_PICHU, 5, {MOVE_THUNDER_SHOCK, MOVE_CHARM, MOVE_SURF, MOVE_NONE}, NULL, 0, FALSE},
    [EV_SKITTY]    = {"BOX SKITTY", SPECIES_SKITTY, 5, {MOVE_GROWL, MOVE_TACKLE, MOVE_TAIL_WHIP, MOVE_PAY_DAY}, NULL, 0, FALSE},
    [EV_SWABLU]    = {"BOX SWABLU", SPECIES_SWABLU, 5, {MOVE_PECK, MOVE_GROWL, MOVE_FALSE_SWIPE, MOVE_NONE}, NULL, 0, FALSE},
    // EON TICKET and MYSTIC TICKET encounters
    [EV_LATIAS]    = {"SOUTHERN LATIAS", SPECIES_LATIAS, 50, {MOVE_MIST_BALL, MOVE_PSYCHIC, MOVE_RECOVER, MOVE_CHARM}, "GACHA", GACHA_OT_ID, FALSE},
    [EV_LATIOS]    = {"SOUTHERN LATIOS", SPECIES_LATIOS, 50, {MOVE_LUSTER_PURGE, MOVE_PSYCHIC, MOVE_RECOVER, MOVE_DRAGON_DANCE}, "GACHA", GACHA_OT_ID, FALSE},
    [EV_LUGIA]     = {"NAVEL ROCK LUGIA", SPECIES_LUGIA, 70, {MOVE_RECOVER, MOVE_HYDRO_PUMP, MOVE_RAIN_DANCE, MOVE_SWIFT}, "GACHA", GACHA_OT_ID, TRUE},
    [EV_HOOH]      = {"NAVEL ROCK HO-OH", SPECIES_HO_OH, 70, {MOVE_RECOVER, MOVE_FIRE_BLAST, MOVE_SUNNY_DAY, MOVE_SWIFT}, "GACHA", GACHA_OT_ID, TRUE},
    // PICKUP. Where a real Generation 3 event existed, its level and moves
    // (and the fateful encounter): AURA Mew, AGETO Celebi, WISHMKR Jirachi,
    // SPACE C Deoxys, the 10th Anniversary Lv70 legends (their Lv70 moves)
    // and the EON TICKET Latios/Latias. The rest are like an in-game gift.
    // Title NULL: the species name.
    [EV_G_MEW]        = {NULL, SPECIES_MEW, 10, {MOVE_POUND, MOVE_NONE, MOVE_NONE, MOVE_NONE}, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_MEWTWO]     = {NULL, SPECIES_MEWTWO, 70, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_CELEBI]     = {NULL, SPECIES_CELEBI, 10, {MOVE_CONFUSION, MOVE_RECOVER, MOVE_HEAL_BELL, MOVE_SAFEGUARD}, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_JIRACHI]    = {NULL, SPECIES_JIRACHI, 5, {MOVE_WISH, MOVE_CONFUSION, MOVE_REST, MOVE_NONE}, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_DRAGONITE]  = {NULL, SPECIES_DRAGONITE, 55, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_TYRANITAR]  = {NULL, SPECIES_TYRANITAR, 55, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_SALAMENCE]  = {NULL, SPECIES_SALAMENCE, 50, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_METAGROSS]  = {NULL, SPECIES_METAGROSS, 45, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_DITTO]      = {NULL, SPECIES_DITTO, 25, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_EEVEE]      = {NULL, SPECIES_EEVEE, 25, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_FEEBAS]     = {NULL, SPECIES_FEEBAS, 20, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_DEOXYS]     = {NULL, SPECIES_DEOXYS, 30, {MOVE_TAUNT, MOVE_PURSUIT, MOVE_PSYCHIC, MOVE_SUPERPOWER}, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_HITMONLEE]  = {NULL, SPECIES_HITMONLEE, 25, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_HITMONCHAN] = {NULL, SPECIES_HITMONCHAN, 25, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_MOLTRES]    = {NULL, SPECIES_MOLTRES, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_ZAPDOS]     = {NULL, SPECIES_ZAPDOS, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_ARTICUNO]   = {NULL, SPECIES_ARTICUNO, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_BULBASAUR]  = {NULL, SPECIES_BULBASAUR, 5, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_CHARMANDER] = {NULL, SPECIES_CHARMANDER, 5, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_SQUIRTLE]   = {NULL, SPECIES_SQUIRTLE, 5, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_CHIKORITA]  = {NULL, SPECIES_CHIKORITA, 5, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_TOTODILE]   = {NULL, SPECIES_TOTODILE, 5, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_CYNDAQUIL]  = {NULL, SPECIES_CYNDAQUIL, 5, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_ENTEI]      = {NULL, SPECIES_ENTEI, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_RAIKOU]     = {NULL, SPECIES_RAIKOU, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_SUICUNE]    = {NULL, SPECIES_SUICUNE, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_LUGIA]      = {NULL, SPECIES_LUGIA, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_HOOH]       = {NULL, SPECIES_HO_OH, 70, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
    [EV_G_LATIOS]     = {NULL, SPECIES_LATIOS, 50, {MOVE_LUSTER_PURGE, MOVE_PSYCHIC, MOVE_RECOVER, MOVE_DRAGON_DANCE}, "GACHA", GACHA_OT_ID, FALSE},
    [EV_G_LATIAS]     = {NULL, SPECIES_LATIAS, 50, {MOVE_MIST_BALL, MOVE_PSYCHIC, MOVE_RECOVER, MOVE_CHARM}, "GACHA", GACHA_OT_ID, FALSE},
    // STANDARD, a few each day. Real events keep their level and moves: the
    // Stadium Surfing Pikachu, the Snap Station Amnesia Psyduck, Crystal's ODD
    // EGG babies (all with DIZZY PUNCH), its Dragon's Den Dratini (EXTREMESPEED),
    // the GS BALL Celebi and Lake of Rage's RED GYARADOS (always shiny), Kirk's Shuckie, the
    // CHANNEL Jirachi. Gifts and catches from the games (Elm's Togepi,
    // Colosseum's Espeon and Umbreon, Faraway Island's Mew, Birth Island's
    // Deoxys, XD's Lugia) know their moves at that level.
    [EV_SURF_PIKACHU]  = {"SURFING PIKACHU", SPECIES_PIKACHU, 5, {MOVE_THUNDER_SHOCK, MOVE_GROWL, MOVE_SURF, MOVE_NONE}, NULL, 0, FALSE},
    [EV_FLY_PIKACHU]   = {"FLYING PIKACHU", SPECIES_PIKACHU, 5, {MOVE_THUNDER_SHOCK, MOVE_GROWL, MOVE_FLY, MOVE_NONE}, NULL, 0, FALSE},
    [EV_PSYDUCK]       = {"AMNESIA PSYDUCK", SPECIES_PSYDUCK, 15, {MOVE_SCRATCH, MOVE_AMNESIA, MOVE_NONE, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_PICHU]     = {"ODD EGG PICHU", SPECIES_PICHU, 5, {MOVE_THUNDER_SHOCK, MOVE_CHARM, MOVE_DIZZY_PUNCH, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_CLEFFA]    = {"ODD EGG CLEFFA", SPECIES_CLEFFA, 5, {MOVE_POUND, MOVE_CHARM, MOVE_DIZZY_PUNCH, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_IGGLYBUFF] = {"ODD EGG IGGLYBUFF", SPECIES_IGGLYBUFF, 5, {MOVE_SING, MOVE_CHARM, MOVE_DIZZY_PUNCH, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_SMOOCHUM]  = {"ODD EGG SMOOCHUM", SPECIES_SMOOCHUM, 5, {MOVE_POUND, MOVE_LICK, MOVE_DIZZY_PUNCH, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_MAGBY]     = {"ODD EGG MAGBY", SPECIES_MAGBY, 5, {MOVE_EMBER, MOVE_DIZZY_PUNCH, MOVE_NONE, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_ELEKID]    = {"ODD EGG ELEKID", SPECIES_ELEKID, 5, {MOVE_QUICK_ATTACK, MOVE_LEER, MOVE_DIZZY_PUNCH, MOVE_NONE}, NULL, 0, FALSE},
    [EV_ODD_TYROGUE]   = {"ODD EGG TYROGUE", SPECIES_TYROGUE, 5, {MOVE_TACKLE, MOVE_DIZZY_PUNCH, MOVE_NONE, MOVE_NONE}, NULL, 0, FALSE},
    [EV_DRATINI]       = {"DRAGON'S DEN DRATINI", SPECIES_DRATINI, 15, {MOVE_WRAP, MOVE_LEER, MOVE_THUNDER_WAVE, MOVE_EXTREME_SPEED}, NULL, 0, FALSE},
    [EV_SHUCKIE]       = {"SHUCKIE", SPECIES_SHUCKLE, 15, NO_MOVES, "KIRK", 518, FALSE},
    [EV_TOGEPI]        = {"ELM'S TOGEPI", SPECIES_TOGEPI, 5, NO_MOVES, NULL, 0, FALSE},
    [EV_ESPEON]        = {"COLOSSEUM ESPEON", SPECIES_ESPEON, 25, NO_MOVES, NULL, 0, FALSE},
    [EV_UMBREON]       = {"COLOSSEUM UMBREON", SPECIES_UMBREON, 26, NO_MOVES, NULL, 0, FALSE},
    [EV_GYARADOS]      = {"RED GYARADOS", SPECIES_GYARADOS, 30, NO_MOVES, NULL, 0, FALSE},
    [EV_OLD_MEW]       = {"CLASSIC MEW", SPECIES_MEW, 5, {MOVE_POUND, MOVE_NONE, MOVE_NONE, MOVE_NONE}, "GF", 22796, TRUE},
    [EV_GS_CELEBI]     = {"GS BALL CELEBI", SPECIES_CELEBI, 30, NO_MOVES, NULL, 0, FALSE},
    [EV_CHANNEL_JIRACHI] = {"CHANNEL JIRACHI", SPECIES_JIRACHI, 5, {MOVE_WISH, MOVE_CONFUSION, MOVE_REST, MOVE_NONE}, "CHANNEL", 40122, TRUE},
    [EV_FARAWAY_MEW]   = {"FARAWAY MEW", SPECIES_MEW, 30, NO_MOVES, NULL, 0, TRUE},
    [EV_BIRTH_DEOXYS]  = {"BIRTH ISLAND DEOXYS", SPECIES_DEOXYS, 30, NO_MOVES, NULL, 0, TRUE},
    [EV_XD_LUGIA]      = {"XD LUGIA", SPECIES_LUGIA, 50, NO_MOVES, NULL, 0, FALSE},
    [EV_RANDOM]       = {NULL, SPECIES_NONE, 20, NO_MOVES, "GACHA", GACHA_OT_ID, FALSE},
    [EV_RANDOM_LEGEND] = {NULL, SPECIES_NONE, 50, NO_MOVES, "GACHA", GACHA_OT_ID, TRUE},
};

enum { K_ITEM, K_MONEY, K_MON, K_SET, K_FX };   // K_SET: several items at once (sSets); K_FX: a ball effect piece

struct PrizeDef
{
    u8 kind;
    u16 id;      // item, or EV_* for K_MON
    u32 qty;     // item count, or money
    u8 weight;
    bool8 shiny;
};

#define ITEM(i, q, w) {K_ITEM, i, q, w, FALSE}
#define MONEY(m, w)   {K_MONEY, ITEM_AMULET_COIN, m, w, FALSE}
#define MON(e, w)     {K_MON, e, 1, w, FALSE}
#define SHINY(e, w)   {K_MON, e, 1, w, TRUE}
#define SET(n, q, w)  {K_SET, n, q, w, FALSE}
// A POKé BALL color or ball effect piece, one of the MOEGATCHA ones (mp_ballunlock.c), picked at
// the pull (qty: its number + 1; 0 = not picked yet). One the player has already: a TICKET instead
#define FX(w)         {K_FX, ITEM_STAR_PIECE, 0, w, FALSE}

static bool8 IsRandomEv(u16 ev)
{
    return ev == EV_RANDOM || ev == EV_RANDOM_LEGEND;
}

// The Moemon a prize gives (a random one keeps its species in qty; 0 = not picked yet)
static u16 DefSpecies(const struct PrizeDef *def)
{
    return IsRandomEv(def->id) ? def->qty : sEventMons[def->id].species;
}

static const u16 sLegendDex[] = {144, 145, 146, 150, 151, 243, 244, 245, 249, 250, 251, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386};

static bool8 IsLegendSpecies(u16 species)
{
    u16 n = SpeciesToNationalPokedexNum(species), i;
    for (i = 0; i < ARRAY_COUNT(sLegendDex); i++)
        if (sLegendDex[i] == n)
            return TRUE;
    return FALSE;
}

static u16 RandomSpecies(bool8 legend)
{
    if (legend)
        return NationalPokedexNumToSpecies(sLegendDex[Random() % ARRAY_COUNT(sLegendDex)]);
    for (;;)
    {
        u16 species = NationalPokedexNumToSpecies(1 + Random() % 386);
        if (species != SPECIES_NONE && !IsLegendSpecies(species) && species != SPECIES_UNOWN)
            return species;
    }
}


enum { SET_VITAMINS, SET_BALLS, SET_APRICORN };
static const u16 sSets[][6] = {
    [SET_VITAMINS] = {ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CALCIUM, ITEM_ZINC, ITEM_CARBOS},
    [SET_BALLS]    = {ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_NEST_BALL, ITEM_REPEAT_BALL, ITEM_TIMER_BALL, ITEM_LUXURY_BALL},
    // The special balls (only from the BALL banner): the six with catch bonuses from the old days
    [SET_APRICORN] = {ITEM_LOVE_BALL, ITEM_LEVEL_BALL, ITEM_LURE_BALL, ITEM_MOON_BALL, ITEM_HEAVY_BALL, ITEM_FAST_BALL},
};
static const char *const sSetNames[] = {[SET_VITAMINS] = "VITAMIN SET", [SET_BALLS] = "BALL SET", [SET_APRICORN] = "APRICORN SET"};

static const struct PrizeDef sCommon[] = {
    ITEM(ITEM_POKE_BALL, 10, 10), ITEM(ITEM_GREAT_BALL, 5, 10), ITEM(ITEM_SUPER_POTION, 5, 10),
    ITEM(ITEM_HYPER_POTION, 3, 8), ITEM(ITEM_REVIVE, 2, 8), ITEM(ITEM_ETHER, 2, 6), ITEM(ITEM_FULL_HEAL, 3, 6),
    ITEM(ITEM_MAX_REPEL, 3, 6), MONEY(500, 10), MONEY(1000, 8),
    // Berries: the everyday ones in fives
    ITEM(ITEM_ORAN_BERRY, 5, 3), ITEM(ITEM_PECHA_BERRY, 5, 3), ITEM(ITEM_CHERI_BERRY, 5, 3),
    ITEM(ITEM_RAWST_BERRY, 5, 3), ITEM(ITEM_ASPEAR_BERRY, 5, 3), ITEM(ITEM_CHESTO_BERRY, 5, 3),
    // Local specialties from other regions (heal all status, like the LAVA COOKIE)
    ITEM(ITEM_OLD_GATEAU, 2, 2), ITEM(ITEM_CASTELIACONE, 2, 2), ITEM(ITEM_LUMIOSE_GALETTE, 2, 2),
    ITEM(ITEM_SHALOUR_SABLE, 2, 2), ITEM(ITEM_BIG_MALASADA, 2, 2), ITEM(ITEM_PEWTER_CRUNCHIES, 2, 2),
    ITEM(ITEM_RAGE_CANDY_BAR, 2, 2),
};
static const struct PrizeDef sUncommon[] = {
    ITEM(ITEM_ULTRA_BALL, 5, 10), ITEM(ITEM_RARE_CANDY, 1, 12), ITEM(ITEM_FULL_RESTORE, 2, 8),
    ITEM(ITEM_MAX_REVIVE, 1, 8), ITEM(ITEM_NUGGET, 1, 8), ITEM(ITEM_ELIXIR, 2, 6), ITEM(ITEM_TIMER_BALL, 5, 6),
    ITEM(ITEM_PROTEIN, 1, 4), ITEM(ITEM_IRON, 1, 4), ITEM(ITEM_CALCIUM, 1, 4), ITEM(ITEM_ZINC, 1, 4),
    ITEM(ITEM_CARBOS, 1, 4), ITEM(ITEM_HP_UP, 1, 4), MONEY(2000, 10),
    // Berries: LEPPA / PERSIM, POKeBLOCK ones and the friendship ones
    ITEM(ITEM_LEPPA_BERRY, 3, 3), ITEM(ITEM_PERSIM_BERRY, 3, 3), ITEM(ITEM_RAZZ_BERRY, 5, 2), ITEM(ITEM_BLUK_BERRY, 5, 2),
    ITEM(ITEM_NANAB_BERRY, 5, 2), ITEM(ITEM_WEPEAR_BERRY, 5, 2), ITEM(ITEM_PINAP_BERRY, 5, 2),
    ITEM(ITEM_POMEG_BERRY, 3, 2), ITEM(ITEM_KELPSY_BERRY, 3, 2), ITEM(ITEM_QUALOT_BERRY, 3, 2),
    ITEM(ITEM_HONDEW_BERRY, 3, 2), ITEM(ITEM_GREPA_BERRY, 3, 2), ITEM(ITEM_TAMATO_BERRY, 3, 2),
};
// STANDARD's R, SR, UR and EX pools change every day (SetupStandard): four
// event Moemon from sDailyMons among the R items (shiny at SR) and six
// legends from sDailyLegends at UR (shiny at EX)
#define DAILY_MONS    4
#define DAILY_LEGENDS 6
static const struct PrizeDef sRareItems[] = {
    ITEM(ITEM_RARE_CANDY, 2, 12), ITEM(ITEM_PP_UP, 2, 8), ITEM(ITEM_MAX_ELIXIR, 2, 6),
    ITEM(ITEM_FIRE_STONE, 1, 3), ITEM(ITEM_WATER_STONE, 1, 3), ITEM(ITEM_THUNDER_STONE, 1, 3),
    ITEM(ITEM_LEAF_STONE, 1, 3), ITEM(ITEM_MOON_STONE, 1, 3), ITEM(ITEM_LEFTOVERS, 1, 4),
    ITEM(ITEM_LUCKY_EGG, 1, 4), ITEM(ITEM_KINGS_ROCK, 1, 3), ITEM(ITEM_CHOICE_BAND, 1, 3),
    ITEM(ITEM_SCOPE_LENS, 1, 3), ITEM(ITEM_SHELL_BELL, 1, 3), ITEM(ITEM_PROTEIN, 3, 2), ITEM(ITEM_CARBOS, 3, 2),
    ITEM(ITEM_EVIOLITE, 1, 3), ITEM(ITEM_QUICK_POWDER, 1, 2),
    MONEY(5000, 8),
    // Berries: SITRUS, LUM and the rare POKeBLOCK ones
    ITEM(ITEM_SITRUS_BERRY, 3, 4), ITEM(ITEM_LUM_BERRY, 2, 4), ITEM(ITEM_CORNN_BERRY, 2, 1), ITEM(ITEM_MAGOST_BERRY, 2, 1),
    ITEM(ITEM_RABUTA_BERRY, 2, 1), ITEM(ITEM_NOMEL_BERRY, 2, 1), ITEM(ITEM_SPELON_BERRY, 2, 1), ITEM(ITEM_PAMTRE_BERRY, 2, 1),
    ITEM(ITEM_WATMEL_BERRY, 2, 1), ITEM(ITEM_DURIN_BERRY, 2, 1), ITEM(ITEM_BELUE_BERRY, 2, 1),
};
static const struct PrizeDef sSuperItems[] = {
    ITEM(ITEM_PP_MAX, 2, 10), ITEM(ITEM_RARE_CANDY, 4, 12), MONEY(10000, 10), ITEM(ITEM_LEFTOVERS, 1, 6),
    ITEM(ITEM_LUCKY_EGG, 1, 6), SHINY(EV_GYARADOS, 2),   // the RED GYARADOS is always there, and always shiny
    FX(4),   // a ball effect piece (more of them on the POKe BALLS banner)
    // Berries: the pinch ones from Mirage Island
    ITEM(ITEM_LIECHI_BERRY, 1, 2), ITEM(ITEM_GANLON_BERRY, 1, 2), ITEM(ITEM_SALAC_BERRY, 1, 2),
    ITEM(ITEM_PETAYA_BERRY, 1, 2), ITEM(ITEM_APICOT_BERRY, 1, 2),
};
static struct PrizeDef sRare[ARRAY_COUNT(sRareItems) + DAILY_MONS];
static struct PrizeDef sSuper[ARRAY_COUNT(sSuperItems) + DAILY_MONS];
static const struct PrizeDef sSSuper[] = {
    ITEM(ITEM_MASTER_BALL, 1, 5), ITEM(ITEM_RARE_CANDY, 8, 10), MONEY(20000, 10), ITEM(ITEM_PP_MAX, 5, 8),
    SET(SET_VITAMINS, 2, 8),
    // Berries: LANSAT, STARF and the e-Reader Berries (found nowhere else)
    ITEM(ITEM_LANSAT_BERRY, 1, 3), ITEM(ITEM_STARF_BERRY, 1, 3),
    ITEM(ITEM_PUMKIN_BERRY, 2, 2), ITEM(ITEM_DRASH_BERRY, 2, 2), ITEM(ITEM_EGGANT_BERRY, 2, 2), ITEM(ITEM_STRIB_BERRY, 2, 2),
    ITEM(ITEM_CHILAN_BERRY, 2, 2), ITEM(ITEM_NUTPEA_BERRY, 2, 2), ITEM(ITEM_GINEMA_BERRY, 2, 2), ITEM(ITEM_KUO_BERRY, 2, 2),
    ITEM(ITEM_YAGO_BERRY, 2, 2), ITEM(ITEM_TOUGA_BERRY, 2, 2), ITEM(ITEM_NINIKU_BERRY, 2, 2), ITEM(ITEM_TOPO_BERRY, 2, 2),
};
// Legendary and mythical Moemon: UR only, shiny ones EX only
static struct PrizeDef sUltra[DAILY_LEGENDS];
static struct PrizeDef sEx[DAILY_LEGENDS];

// Four a day, in turn (the two PICHU and the two PIKACHU never on the same day)
static const u8 sDailyMons[] = {
    EV_ODD_PICHU, EV_ZIGZAGOON, EV_DRATINI, EV_ODD_CLEFFA, EV_SURF_PIKACHU, EV_SKITTY, EV_TOGEPI, EV_ODD_SMOOCHUM,
    EV_ESPEON, EV_PICHU, EV_ODD_MAGBY, EV_PSYDUCK, EV_SHUCKIE, EV_ODD_IGGLYBUFF, EV_FLY_PIKACHU, EV_SWABLU,
    EV_UMBREON, EV_ODD_ELEKID, EV_ODD_TYROGUE,
};
// Six legends a day, one week round (never the same species twice in a day)
static const u8 sDailyLegends[7][DAILY_LEGENDS] = {
    {EV_MEW, EV_CELEBI, EV_JIRACHI, EV_DEOXYS, EV_LATIAS, EV_LUGIA},
    {EV_OLD_MEW, EV_GS_CELEBI, EV_CHANNEL_JIRACHI, EV_BIRTH_DEOXYS, EV_LATIOS, EV_HOOH},
    {EV_FARAWAY_MEW, EV_CELEBI, EV_JIRACHI, EV_XD_LUGIA, EV_LATIAS, EV_HOOH},
    {EV_MEW, EV_GS_CELEBI, EV_CHANNEL_JIRACHI, EV_DEOXYS, EV_LATIOS, EV_LUGIA},
    {EV_OLD_MEW, EV_CELEBI, EV_BIRTH_DEOXYS, EV_XD_LUGIA, EV_LATIAS, EV_LATIOS},
    {EV_FARAWAY_MEW, EV_GS_CELEBI, EV_JIRACHI, EV_DEOXYS, EV_HOOH, EV_LUGIA},
    {EV_MEW, EV_CHANNEL_JIRACHI, EV_BIRTH_DEOXYS, EV_XD_LUGIA, EV_HOOH, EV_LATIAS},
};
static const u8 sLegendWeight[] = {[EV_LUGIA] = 7, [EV_HOOH] = 7, [EV_XD_LUGIA] = 7};   // the rest 10

static s32 sStandardDay = -1;   // test builds: mp gatchaday N

static s32 StandardDay(void)
{
    s32 d = sStandardDay >= 0 ? sStandardDay : MP_DayNumber(MP_Today());
    return d < 0 ? 0 : d;
}

static const u8 *TodaysMons(u8 *out)
{
    s32 day = StandardDay();
    u8 i;
    for (i = 0; i < DAILY_MONS; i++)
        out[i] = sDailyMons[(day * DAILY_MONS + i) % ARRAY_COUNT(sDailyMons)];
    return out;
}

static const u8 *TodaysLegends(void)
{
    return sDailyLegends[StandardDay() % 7];
}

static u8 LegendWeight(u8 ev)
{
    return ev < ARRAY_COUNT(sLegendWeight) && sLegendWeight[ev] ? sLegendWeight[ev] : 10;
}

// Today's STANDARD prizes
static void SetupStandard(void)
{
    u8 mons[DAILY_MONS], i, n;
    const u8 *legends = TodaysLegends();
    TodaysMons(mons);
    memcpy(sRare, sRareItems, sizeof(sRareItems));
    memcpy(sSuper, sSuperItems, sizeof(sSuperItems));
    for (i = 0, n = ARRAY_COUNT(sRareItems); i < DAILY_MONS; i++)
        sRare[n + i] = (struct PrizeDef)MON(mons[i], 4);
    for (i = 0, n = ARRAY_COUNT(sSuperItems); i < DAILY_MONS; i++)
        sSuper[n + i] = (struct PrizeDef)SHINY(mons[i], 3);
    for (i = 0; i < DAILY_LEGENDS; i++)
    {
        sUltra[i] = (struct PrizeDef)MON(legends[i], LegendWeight(legends[i]));
        sEx[i] = (struct PrizeDef)SHINY(legends[i], LegendWeight(legends[i]));
    }
}

void MP_Gatcha_SetDay(s32 day)
{
    sStandardDay = day;
}

// TRAINING banner: build your Moemon up
static const struct PrizeDef sTrainCommon[] = {
    ITEM(ITEM_RARE_CANDY, 1, 14), ITEM(ITEM_HP_UP, 1, 6), ITEM(ITEM_PROTEIN, 1, 6), ITEM(ITEM_IRON, 1, 6),
    ITEM(ITEM_CALCIUM, 1, 6), ITEM(ITEM_ZINC, 1, 6), ITEM(ITEM_CARBOS, 1, 6),
};
static const struct PrizeDef sTrainUncommon[] = {
    ITEM(ITEM_RARE_CANDY, 1, 14), ITEM(ITEM_HP_UP, 2, 5), ITEM(ITEM_PROTEIN, 2, 5), ITEM(ITEM_IRON, 2, 5),
    ITEM(ITEM_CALCIUM, 2, 5), ITEM(ITEM_ZINC, 2, 5), ITEM(ITEM_CARBOS, 2, 5), ITEM(ITEM_PP_UP, 1, 8),
};
static const struct PrizeDef sTrainRare[] = {
    ITEM(ITEM_RARE_CANDY, 2, 14), ITEM(ITEM_HP_UP, 5, 3), ITEM(ITEM_PROTEIN, 5, 3), ITEM(ITEM_IRON, 5, 3),
    ITEM(ITEM_CALCIUM, 5, 3), ITEM(ITEM_ZINC, 5, 3), ITEM(ITEM_CARBOS, 5, 3), ITEM(ITEM_PP_UP, 3, 8),
    ITEM(ITEM_LUCKY_EGG, 1, 5), ITEM(ITEM_MACHO_BRACE, 1, 5), ITEM(ITEM_EXP_SHARE, 1, 5),
};
static const struct PrizeDef sTrainSuper[] = {
    ITEM(ITEM_RARE_CANDY, 4, 14), ITEM(ITEM_PP_MAX, 3, 10), SET(SET_VITAMINS, 1, 12),
};
static const struct PrizeDef sTrainSSuper[] = {
    ITEM(ITEM_RARE_CANDY, 6, 12), SET(SET_VITAMINS, 2, 10), ITEM(ITEM_PP_MAX, 5, 8),
};
static const struct PrizeDef sTrainUltra[] = {
    ITEM(ITEM_RARE_CANDY, 10, 10), SET(SET_VITAMINS, 2, 10), ITEM(ITEM_PP_MAX, 10, 6),
};
static const struct PrizeDef sTrainEx[] = {
    ITEM(ITEM_RARE_CANDY, 15, 10), SET(SET_VITAMINS, 3, 10), ITEM(ITEM_PP_MAX, 25, 6),
};

// BALL banner: special and hard-to-find POKé BALLS
static const struct PrizeDef sBallCommon[] = {
    ITEM(ITEM_GREAT_BALL, 5, 10), ITEM(ITEM_NET_BALL, 3, 8), ITEM(ITEM_DIVE_BALL, 3, 8), ITEM(ITEM_NEST_BALL, 3, 8),
    ITEM(ITEM_REPEAT_BALL, 3, 8), ITEM(ITEM_TIMER_BALL, 3, 8), ITEM(ITEM_PREMIER_BALL, 3, 8), ITEM(ITEM_LUXURY_BALL, 1, 6),
    // Special balls with no catch bonus sit with the PREMIER / LUXURY BALLS
    ITEM(ITEM_HEAL_BALL, 2, 6), ITEM(ITEM_FRIEND_BALL, 1, 6),
    // Red's and Crystal's own POKé BALLS: the old rules, the old look
    ITEM(ITEM_GENESIS_BALL, 3, 6), ITEM(ITEM_EPOCH_BALL, 3, 6),
};
static const struct PrizeDef sBallUncommon[] = {
    ITEM(ITEM_ULTRA_BALL, 5, 10), ITEM(ITEM_NET_BALL, 5, 6), ITEM(ITEM_DIVE_BALL, 5, 6), ITEM(ITEM_NEST_BALL, 5, 6),
    ITEM(ITEM_REPEAT_BALL, 5, 6), ITEM(ITEM_TIMER_BALL, 5, 6), ITEM(ITEM_LUXURY_BALL, 3, 8), ITEM(ITEM_PREMIER_BALL, 10, 6),
    // The ones with a bonus in one situation, like NET / DIVE BALLS
    ITEM(ITEM_LOVE_BALL, 2, 4), ITEM(ITEM_LEVEL_BALL, 2, 4), ITEM(ITEM_LURE_BALL, 2, 4), ITEM(ITEM_MOON_BALL, 2, 4),
    ITEM(ITEM_HEAVY_BALL, 2, 4), ITEM(ITEM_FAST_BALL, 2, 4), ITEM(ITEM_HEAL_BALL, 4, 4), ITEM(ITEM_FRIEND_BALL, 3, 4),
    ITEM(ITEM_GENESIS_BALL, 5, 4), ITEM(ITEM_EPOCH_BALL, 5, 4),
};
static const struct PrizeDef sBallRare[] = {
    ITEM(ITEM_ULTRA_BALL, 15, 10), ITEM(ITEM_LUXURY_BALL, 8, 8), ITEM(ITEM_NET_BALL, 10, 5), ITEM(ITEM_DIVE_BALL, 10, 5),
    ITEM(ITEM_NEST_BALL, 10, 5), ITEM(ITEM_REPEAT_BALL, 10, 5), ITEM(ITEM_TIMER_BALL, 10, 5), SET(SET_BALLS, 3, 8),
    // The strong all-rounders, like the TIMER BALL
    ITEM(ITEM_QUICK_BALL, 3, 5), ITEM(ITEM_DUSK_BALL, 3, 5), ITEM(ITEM_DREAM_BALL, 3, 5), SET(SET_APRICORN, 1, 6),
    FX(12),   // POKe BALL colors and ball effect pieces
};
static const struct PrizeDef sBallSuper[] = {
    SET(SET_BALLS, 5, 10), ITEM(ITEM_ULTRA_BALL, 30, 8), ITEM(ITEM_LUXURY_BALL, 20, 6),
    SET(SET_APRICORN, 3, 8), ITEM(ITEM_QUICK_BALL, 10, 6), ITEM(ITEM_DUSK_BALL, 10, 6), ITEM(ITEM_DREAM_BALL, 10, 6),
    FX(12),
};
static const struct PrizeDef sBallSSuper[] = {
    SET(SET_BALLS, 10, 10), ITEM(ITEM_ULTRA_BALL, 60, 6), ITEM(ITEM_LUXURY_BALL, 40, 4),
    SET(SET_APRICORN, 6, 8), ITEM(ITEM_QUICK_BALL, 25, 5), ITEM(ITEM_DUSK_BALL, 25, 5),
    FX(10),
};
static const struct PrizeDef sBallUltra[] = {
    ITEM(ITEM_MASTER_BALL, 1, 10), SET(SET_BALLS, 20, 6), SET(SET_APRICORN, 12, 6),
};
static const struct PrizeDef sBallEx[] = {
    ITEM(ITEM_MASTER_BALL, 2, 10), SET(SET_BALLS, 40, 6), SET(SET_APRICORN, 25, 6),
};

// PICKUP banner: a new one every week. Every UR is one of its Moemon (picked
// at random each time), every EX one of them shiny
struct Pickup { u8 mons[3]; u8 count; };
static const struct Pickup sPickups[] = {
    {{EV_G_MEW, EV_G_MEWTWO}, 2},
    {{EV_G_CELEBI}, 1},
    {{EV_G_JIRACHI}, 1},
    {{EV_G_DRAGONITE}, 1},
    {{EV_G_TYRANITAR}, 1},
    {{EV_G_SALAMENCE}, 1},
    {{EV_G_METAGROSS}, 1},
    {{EV_G_DITTO}, 1},
    {{EV_G_EEVEE}, 1},
    {{EV_G_FEEBAS}, 1},
    {{EV_G_DEOXYS}, 1},
    {{EV_G_HITMONLEE, EV_G_HITMONCHAN}, 2},
    {{EV_G_MOLTRES, EV_G_ZAPDOS, EV_G_ARTICUNO}, 3},
    {{EV_G_BULBASAUR, EV_G_CHARMANDER, EV_G_SQUIRTLE}, 3},
    {{EV_G_CHIKORITA, EV_G_TOTODILE, EV_G_CYNDAQUIL}, 3},
    {{EV_G_ENTEI, EV_G_RAIKOU, EV_G_SUICUNE}, 3},
    {{EV_G_LUGIA, EV_G_HOOH}, 2},
    {{EV_G_LATIOS, EV_G_LATIAS}, 2},
    {{EV_RANDOM}, 1},   // a random SHINY Moemon (EX: a random shiny legendary)
};
static struct PrizeDef sPickupUltra[3];
static struct PrizeDef sPickupEx[3];
static u8 sPickupCount;
static const struct PrizeDef sPickupRare[] = {
    MON(EV_ZIGZAGOON, 6), MON(EV_PICHU, 6), MON(EV_SKITTY, 6), MON(EV_SWABLU, 6),
    ITEM(ITEM_RARE_CANDY, 2, 12), ITEM(ITEM_PP_UP, 2, 8), MONEY(5000, 8),
};

enum { BANNER_STANDARD, BANNER_PICKUP, BANNER_TRAINING, BANNER_BALLS, BANNER_COUNT };

struct Pool { const struct PrizeDef *defs; u8 count; };

struct BannerDef
{
    const char *name;
    const char *tagline;
    struct Pool pools[R_COUNT];
    u16 per10k[R_COUNT];     // chance of each rarity, out of 10000
    u16 accent;              // color of the stripes and panels
};

static struct BannerDef sBanners[BANNER_COUNT] = {
    [BANNER_STANDARD] = {
        "STANDARD", "Event Moemon, items and money!",
        {{sCommon, ARRAY_COUNT(sCommon)}, {sUncommon, ARRAY_COUNT(sUncommon)}, {sRare, ARRAY_COUNT(sRare)},
         {sSuper, ARRAY_COUNT(sSuper)}, {sSSuper, ARRAY_COUNT(sSSuper)}, {sUltra, ARRAY_COUNT(sUltra)}, {sEx, ARRAY_COUNT(sEx)}},
        {5200, 2800, 1350, 500, 120, 25, 5}, 0,
    },
    [BANNER_PICKUP] = {
        "PICKUP", NULL,   // tagline names the featured Moemon
        {{sCommon, ARRAY_COUNT(sCommon)}, {sUncommon, ARRAY_COUNT(sUncommon)}, {sPickupRare, ARRAY_COUNT(sPickupRare)},
         {sSuper, ARRAY_COUNT(sSuper)}, {sSSuper, ARRAY_COUNT(sSSuper)}, {sPickupUltra, 1}, {sPickupEx, 1}},   // counts set by SetupPickup
        {5150, 2800, 1350, 500, 150, 45, 5}, RGB(31, 10, 20),
    },
    [BANNER_TRAINING] = {
        "TRAINING", "RARE CANDIES and vitamins!",
        {{sTrainCommon, ARRAY_COUNT(sTrainCommon)}, {sTrainUncommon, ARRAY_COUNT(sTrainUncommon)}, {sTrainRare, ARRAY_COUNT(sTrainRare)},
         {sTrainSuper, ARRAY_COUNT(sTrainSuper)}, {sTrainSSuper, ARRAY_COUNT(sTrainSSuper)}, {sTrainUltra, ARRAY_COUNT(sTrainUltra)}, {sTrainEx, ARRAY_COUNT(sTrainEx)}},
        {5200, 2800, 1350, 500, 120, 25, 5}, RGB(31, 17, 2),
    },
    [BANNER_BALLS] = {
        "POKE BALLS", "Rare BALLS + ball colors and effects!",
        {{sBallCommon, ARRAY_COUNT(sBallCommon)}, {sBallUncommon, ARRAY_COUNT(sBallUncommon)}, {sBallRare, ARRAY_COUNT(sBallRare)},
         {sBallSuper, ARRAY_COUNT(sBallSuper)}, {sBallSSuper, ARRAY_COUNT(sBallSSuper)}, {sBallUltra, ARRAY_COUNT(sBallUltra)}, {sBallEx, ARRAY_COUNT(sBallEx)}},
        {5200, 2800, 1350, 500, 120, 25, 5}, RGB(6, 18, 31),
    },
};

static u8 sBanner;   // the banner shown (kept between visits)

static s8 sPickupOverride = -1;   // test builds: mp pickup N

void MP_Gatcha_SetPickup(s8 n)
{
    sPickupOverride = n;
}

// A PICKUP vote that passed this week (friends online choose; see the vote below)
#define VOTE_WEEK_KEY "gatcha:votewk"     // week number + 1 of the vote
#define VOTE_PICK_KEY "gatcha:votepick"   // the PICKUP it chose

static s8 VotedPickup(void)
{
    s32 pick = MP_SaveData_Get(VOTE_PICK_KEY);
    if (MP_SaveData_Get(VOTE_WEEK_KEY) != MP_WeekNumber() + 1 || pick < 0 || pick >= (s32)ARRAY_COUNT(sPickups))
        return -1;
    return pick;
}

static u8 FriendsOnline(void)
{
    u8 i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present)
            n++;
    return n;
}


static u8 ThisWeeksPickupIndex(void)
{
    s8 voted = VotedPickup();
    if (sPickupOverride >= 0 && sPickupOverride < (s8)ARRAY_COUNT(sPickups))
        return sPickupOverride;
    if (voted >= 0)
        return voted;
    return MP_WeekNumber() % ARRAY_COUNT(sPickups);
}

static const struct Pickup *ThisWeeksPickup(void)
{
    return &sPickups[ThisWeeksPickupIndex()];
}

// "MEW/MEWTWO", or "a random SHINY Moemon"
static void PickupLabel(u8 index, char *out, int size)
{
    const struct Pickup *pu = &sPickups[index];
    int n = 0;
    u8 i;
    out[0] = 0;
    if (pu->mons[0] == EV_RANDOM)
    {
        snprintf(out, size, "a random SHINY Moemon");
        return;
    }
    for (i = 0; i < pu->count && n < size; i++)
    {
        u8 name[POKEMON_NAME_LENGTH + 1];
        char a[16];
        GetSpeciesName(name, sEventMons[pu->mons[i]].species);
        MP_GameToAscii(a, name, POKEMON_NAME_LENGTH);
        n += snprintf(out + n, size - n, "%s%s", i ? "/" : "", a);
    }
}

static bool8 PickupIsRandom(void)
{
    return ThisWeeksPickup()->mons[0] == EV_RANDOM;
}

// This week's PICKUP: every UR is one of its Moemon, every EX one of them shiny
static void SetupPickup(void)
{
    const struct Pickup *pu = ThisWeeksPickup();
    u8 i;
    sPickupCount = pu->count;
    for (i = 0; i < pu->count; i++)
    {
        sPickupUltra[i] = (struct PrizeDef)MON(pu->mons[i], 1);
        sPickupEx[i] = (struct PrizeDef)SHINY(pu->mons[i], 1);
    }
    if (pu->mons[0] == EV_RANDOM)
    {
        sPickupUltra[0] = (struct PrizeDef)SHINY(EV_RANDOM, 1);
        sPickupEx[0] = (struct PrizeDef)SHINY(EV_RANDOM_LEGEND, 1);
    }
    sBanners[BANNER_PICKUP].pools[R_UR].count = pu->count;
    sBanners[BANNER_PICKUP].pools[R_EX].count = pu->count;
}

static const struct Pool *BannerPool(u8 rarity)
{
    return &sBanners[sBanner].pools[rarity];
}

struct Result
{
    const struct PrizeDef *def;
    u8 rarity;
    u32 amount;        // money actually given
    char note[48];     // "It joined your party!" ...
};

s32 MP_Gatcha_Pulls(void)
{
    return MP_SaveData_Get(PULLS_KEY);
}

void MP_Gatcha_AddPulls(s32 n)
{
    MP_SaveData_Add(PULLS_KEY, n);
}

// Free pulls pile up: a free 10-pull on STANDARD each week and a free pull on every
// banner each day you play (MP_Gatcha_GrantFree, from MP_Events_Update and on opening)
static s32 Free10Stock(void)
{
    return MP_SaveData_Get(FREE10_KEY);
}

static s32 Free1Stock(u8 banner)
{
    char key[24];
    snprintf(key, sizeof(key), FREE1_KEY, banner);
    return MP_SaveData_Get(key);
}

static void AddFree1(u8 banner, s32 n)
{
    char key[24];
    snprintf(key, sizeof(key), FREE1_KEY, banner);
    MP_SaveData_Add(key, n);
}

s32 MP_Gatcha_Tickets(void)
{
    return MP_SaveData_Get(TICKETS_KEY);
}

void MP_Gatcha_AddTickets(s32 n)
{
    MP_SaveData_Add(TICKETS_KEY, n);
}

// Free pulls everyone has waiting (the CO-OP menu shows them)
s32 MP_Gatcha_FreeWaiting(void)
{
    s32 n = Free10Stock() * 10;
    u8 b;
    for (b = 0; b < BANNER_COUNT; b++)
        n += Free1Stock(b);
    return n;
}

// Special (data/scripts/obtain_item.inc, after giveitem): does this item, given by
// someone, come with a MOEGATCHA TICKET? Key items, HMs, the gym leaders' TMs and the
// EXP. SHARE do, once each per save. The item: the last additem; VAR_RESULT = TRUE
// when a ticket was given.
void MP_GatchaTicketForItem(void)
{
    static const u16 sStoryItems[] = {
        ITEM_TM39, ITEM_TM08, ITEM_TM34, ITEM_TM50, ITEM_TM42, ITEM_TM40, ITEM_TM04, ITEM_TM03,   // the 8 gym leaders
        ITEM_EXP_SHARE,                                                                          // Devon's president
    };
    extern u16 gMpLastScriptItem;
    u16 item = gMpLastScriptItem;   // the giveitem's additem
    char key[24];
    bool8 important = GetItemPocket(item) == POCKET_KEY_ITEMS || (item >= ITEM_HM01 && item <= ITEM_HM08);
    u8 i;
    gSpecialVar_Result = FALSE;
    for (i = 0; i < ARRAY_COUNT(sStoryItems) && !important; i++)
        important = item == sStoryItems[i];
    if (!important)
        return;
    snprintf(key, sizeof(key), "ticketitem:%d", item);
    if (MP_SaveData_Get(key))
        return;
    MP_SaveData_Set(key, 1);
    MP_Gatcha_AddTickets(1);
    MPNet_Log("gatcha: ticket for item %d (%d tickets)", item, (int)MP_Gatcha_Tickets());
    gSpecialVar_Result = TRUE;
}

// MOEGATCHA TICKETS a save would have had if tickets had existed when it got its story
// items (saves from before them): once per save (BACKFILL_KEY), one per important item it
// shows it got and wasn't rewarded for yet ("ticketitem:ID", shared with the live hook).
// Proof: the event flag set when the item is handed over (items taken back later, like the
// DEVON GOODS, or used up, like the gym leaders' TMs) or the item in the bag or PC.
#define BACKFILL_KEY "gatcha:ticketbackfill"
static bool8 GrantItemTicket(u16 item)
{
    char key[24];
    snprintf(key, sizeof(key), "ticketitem:%d", item);
    if (MP_SaveData_Get(key))
        return FALSE;
    MP_SaveData_Set(key, 1);
    return TRUE;
}

static bool8 HasItemAnywhere(u16 item)
{
    return CheckBagHasItem(item, 1) || CheckPCHasItem(item, 1);
}

void MP_Gatcha_BackfillTickets(void)
{
    static const struct { u16 item; u16 flag; } sByFlag[] = {
        {ITEM_HM01, FLAG_RECEIVED_HM_CUT},      {ITEM_HM02, FLAG_RECEIVED_HM_FLY},
        {ITEM_HM03, FLAG_RECEIVED_HM_SURF},     {ITEM_HM04, FLAG_RECEIVED_HM_STRENGTH},
        {ITEM_HM05, FLAG_RECEIVED_HM_FLASH},    {ITEM_HM06, FLAG_RECEIVED_HM_ROCK_SMASH},
        {ITEM_HM07, FLAG_RECEIVED_HM_WATERFALL}, {ITEM_HM08, FLAG_RECEIVED_HM_DIVE},
        {ITEM_TM39, FLAG_RECEIVED_TM_ROCK_TOMB}, {ITEM_TM08, FLAG_RECEIVED_TM_BULK_UP},
        {ITEM_TM34, FLAG_RECEIVED_TM_SHOCK_WAVE}, {ITEM_TM50, FLAG_RECEIVED_TM_OVERHEAT},
        {ITEM_TM42, FLAG_RECEIVED_TM_FACADE},   {ITEM_TM40, FLAG_RECEIVED_TM_AERIAL_ACE},
        {ITEM_TM04, FLAG_RECEIVED_TM_CALM_MIND}, {ITEM_TM03, FLAG_RECEIVED_TM_WATER_PULSE},
        {ITEM_EXP_SHARE, FLAG_RECEIVED_EXP_SHARE},
        {ITEM_DEVON_GOODS, FLAG_RECOVERED_DEVON_GOODS}, {ITEM_LETTER, FLAG_RECEIVED_POKENAV},
        {ITEM_METEORITE, FLAG_RECEIVED_METEORITE}, {ITEM_SCANNER, FLAG_EXCHANGED_SCANNER},
        {ITEM_ROOT_FOSSIL, FLAG_CHOSE_ROOT_FOSSIL}, {ITEM_CLAW_FOSSIL, FLAG_CHOSE_CLAW_FOSSIL},
        {ITEM_COIN_CASE, FLAG_RECEIVED_COIN_CASE}, {ITEM_CONTEST_PASS, FLAG_RECEIVED_CONTEST_PASS},
        {ITEM_DEVON_SCOPE, FLAG_RECEIVED_DEVON_SCOPE}, {ITEM_GO_GOGGLES, FLAG_RECEIVED_GO_GOGGLES},
        {ITEM_OLD_ROD, FLAG_RECEIVED_OLD_ROD},  {ITEM_GOOD_ROD, FLAG_RECEIVED_GOOD_ROD},
        {ITEM_SUPER_ROD, FLAG_RECEIVED_SUPER_ROD}, {ITEM_POKEBLOCK_CASE, FLAG_RECEIVED_POKEBLOCK_CASE},
        {ITEM_SS_TICKET, FLAG_RECEIVED_SS_TICKET}, {ITEM_WAILMER_PAIL, FLAG_RECEIVED_WAILMER_PAIL},
        {ITEM_POWDER_JAR, FLAG_RECEIVED_POWDER_JAR},
        {ITEM_MYSTIC_TICKET, FLAG_RECEIVED_MYSTIC_TICKET}, {ITEM_AURORA_TICKET, FLAG_RECEIVED_AURORA_TICKET},
        {ITEM_OLD_SEA_MAP, FLAG_RECEIVED_OLD_SEA_MAP},
    };
    // key items found lying around (item balls) or not from the story never came with a ticket
    static const u16 sNotGiven[] = {
        ITEM_ROOM_1_KEY, ITEM_ROOM_2_KEY, ITEM_ROOM_4_KEY, ITEM_ROOM_6_KEY, ITEM_STORAGE_KEY, ITEM_BATTLE_FLAG,
    };
    s32 n = 0;
    u16 item;
    u8 i;

    if (MP_SaveData_Get(BACKFILL_KEY))
        return;
    MP_SaveData_Set(BACKFILL_KEY, 1);
    for (i = 0; i < ARRAY_COUNT(sByFlag); i++)
    {
        if ((FlagGet(sByFlag[i].flag) || HasItemAnywhere(sByFlag[i].item)) && GrantItemTicket(sByFlag[i].item))
            n++;
    }
    // a bike: one ticket for the one the player has (or got, by the flag), unless either was rewarded already
    if (FlagGet(FLAG_RECEIVED_BIKE) || HasItemAnywhere(ITEM_MACH_BIKE) || HasItemAnywhere(ITEM_ACRO_BIKE))
    {
        char machKey[24], acroKey[24];
        snprintf(machKey, sizeof(machKey), "ticketitem:%d", ITEM_MACH_BIKE);
        snprintf(acroKey, sizeof(acroKey), "ticketitem:%d", ITEM_ACRO_BIKE);
        if (!MP_SaveData_Get(machKey) && !MP_SaveData_Get(acroKey)
         && GrantItemTicket(HasItemAnywhere(ITEM_ACRO_BIKE) ? ITEM_ACRO_BIKE : ITEM_MACH_BIKE))
            n++;
    }
    // any other key item the player is carrying (SOOT SACK, BASEMENT KEY, MAGMA EMBLEM, ORBS...)
    for (item = ITEM_NONE + 1; item < ITEMS_COUNT; item++)
    {
        bool8 skip = FALSE;
        if (GetItemPocket(item) != POCKET_KEY_ITEMS || item == ITEM_MACH_BIKE || item == ITEM_ACRO_BIKE)
            continue;
        for (i = 0; i < ARRAY_COUNT(sNotGiven) && !skip; i++)
            skip = item == sNotGiven[i];
        if (!skip && HasItemAnywhere(item) && GrantItemTicket(item))
            n++;
    }
    if (n > 0)
    {
        char msg[96];
        u8 text[100];
        MP_Gatcha_AddTickets(n);
        // a note at the top of the screen, not a message box: it never gets in the way of anything
        snprintf(msg, sizeof(msg), "+%d MOEGATCHA TICKET%s", (int)n, n == 1 ? "" : "S");
        MP_AsciiToGame(text, msg, 40);
        {
            u8 nl[] = {CHAR_NEWLINE, EOS};
            StringAppend(text, nl);
        }
        MP_AsciiToGame(text + StringLength(text), "for your story items!", 40);
        MP_Toast(text);
    }
    MPNet_Log("gatcha: ticket backfill: %d for this save's story items (%d tickets)", (int)n, (int)MP_Gatcha_Tickets());
}

// Any OT but MoeGatcha's acts as the player's own: traded Moemon obey (they keep the EXP boost)
bool8 MP_OtActsAsOwn(u32 otId)
{
    return otId != GACHA_OT_ID;
}

// A Battle Tent's prize was handed over (three wins in a row): 3 MOEGATCHA TICKETS
void MP_BattleTentWon(void)
{
    MP_Gatcha_AddTickets(3);
    MPNet_Log("gatcha: battle tent won: 3 tickets (%d tickets)", (int)MP_Gatcha_Tickets());
}

// Special: the Game Corner prize clerk sold a ticket (the coins are already taken)
void MP_GatchaBuyTicket(void)
{
    MP_Gatcha_AddTickets(1);
    MPNet_Log("gatcha: ticket bought (%d tickets)", (int)MP_Gatcha_Tickets());
}

void MP_Gatcha_GrantFree(void)
{
    static s32 sCheckedDay;
    s32 today = MP_Today();
    MP_Gatcha_BackfillTickets();
    if (today == sCheckedDay)
        return;
    sCheckedDay = today;
    // Only a day (week) later than the last one given: the PC's clock set back and forth again gave them every time
    if (MP_WeekNumber() + 1 > MP_SaveData_Get(FREE_WEEK_KEY))
    {
        MP_SaveData_Set(FREE_WEEK_KEY, MP_WeekNumber() + 1);
        MP_SaveData_Add(FREE10_KEY, 1);
        MPNet_Log("gatcha: free 10-pull for the week (%d in stock)", (int)Free10Stock());
    }
    if (today > MP_SaveData_Get(FREE1_DAY_KEY))
    {
        u8 b;
        MP_SaveData_Set(FREE1_DAY_KEY, today);
        for (b = 0; b < BANNER_COUNT; b++)
            AddFree1(b, 1);
        MPNet_Log("gatcha: daily free pull on every banner (STANDARD %d in stock)", (int)Free1Stock(BANNER_STANDARD));
    }
}

static const struct PrizeDef *PickFromPool(u8 rarity)
{
    const struct Pool *pool = BannerPool(rarity);
    u16 total = 0, i, pick;
    for (i = 0; i < pool->count; i++)
        total += pool->defs[i].weight;
    if (total == 0)
        return &pool->defs[0];
    pick = Random() % total;
    for (i = 0; i < pool->count; i++)
    {
        if (pick < pool->defs[i].weight)
            return &pool->defs[i];
        pick -= pool->defs[i].weight;
    }
    return &pool->defs[0];
}

// Each banner keeps its own pity counts (STANDARD keeps the old SR key)
static void PityKeys(char *sr, char *ssr, char *ur)
{
    if (sBanner == BANNER_STANDARD)
    {
        strcpy(sr, PITY_SR_KEY);
        strcpy(ssr, PITY_SSR_KEY);
        strcpy(ur, PITY_UR_KEY);
    }
    else
    {
        sprintf(sr, "%s%d", PITY_SR_KEY, sBanner);
        sprintf(ssr, "%s%d", PITY_SSR_KEY, sBanner);
        sprintf(ur, "%s%d", PITY_UR_KEY, sBanner);
    }
}

static s8 sRig = -1;   // force the next pull's rarity (mp gatcharig N, or the "open" panel)
static const struct PrizeDef *sRigDef;   // ... and its prize (NULL: any)
static struct PrizeDef sRandDefs[10];    // random Moemon picked for this pull

void MP_Gatcha_Rig(s8 rarity)
{
    sRig = rarity;
}

// Test harness: the next pull's prize is a ball effect piece (MOEGATCHA prize number n; -1: picked at random)
void MP_Gatcha_RigFx(s16 n)
{
    static struct PrizeDef sFxRig = FX(1);
    sFxRig.qty = n >= 0 ? n + 1 : 0;
    sRigDef = &sFxRig;
}

static u8 RollRarity(bool8 rarePlus)
{
    char srKey[24], ssrKey[24], urKey[24];
    const u16 *odds = sBanners[sBanner].per10k;
    s32 sinceSR, sinceSSR, sinceUR;
    u16 r = Random() % 10000, acc = 0;
    s8 i;
    u8 rarity = R_N;

    PityKeys(srKey, ssrKey, urKey);
    sinceSR = MP_SaveData_Get(srKey);
    sinceSSR = MP_SaveData_Get(ssrKey);
    sinceUR = MP_SaveData_Get(urKey);
    // From the top: EX, UR, SSR...
    for (i = R_COUNT - 1; i > R_N; i--)
    {
        acc += odds[i];
        if (r < acc)
        {
            rarity = i;
            break;
        }
    }
    if (sinceUR + 1 >= UR_PITY && rarity < R_UR)
        rarity = R_UR;
    else if (sinceSSR + 1 >= SSR_PITY && rarity < R_SSR)
        rarity = R_SSR;
    else if (sinceSR + 1 >= SR_PITY && rarity < R_SR)
        rarity = R_SR;
    if (rarePlus && rarity < R_R)
        rarity = R_R;
    if (sRig >= 0 && sRig < R_COUNT)
    {
        rarity = sRig;
        sRig = -1;
    }
    MP_SaveData_Set(srKey, rarity >= R_SR ? 0 : sinceSR + 1);
    MP_SaveData_Set(ssrKey, rarity >= R_SSR ? 0 : sinceSSR + 1);
    MP_SaveData_Set(urKey, rarity >= R_UR ? 0 : sinceUR + 1);
    return rarity;
}

// "MASTER BALL x3", "$50000", "SHINY AURA MEW" (game text)
static void PrizeName(const struct PrizeDef *def, u32 amount, u8 *dst)
{
    char buf[40];
    if (def->kind == K_MONEY)
    {
        snprintf(buf, sizeof(buf), "$%u", (unsigned)amount);
        MP_AsciiToGame(dst, buf, 30);
    }
    else if (def->kind == K_MON)
    {
        const char *title = sEventMons[def->id].title;
        char species[16];
        if (!title)
        {
            u8 name[POKEMON_NAME_LENGTH + 1];
            u16 sp = DefSpecies(def);
            if (sp == SPECIES_NONE)
                snprintf(species, sizeof(species), "%s", def->id == EV_RANDOM_LEGEND ? "LEGEND?" : "MOEMON?");
            else
            {
                GetSpeciesName(name, sp);
                MP_GameToAscii(species, name, POKEMON_NAME_LENGTH);
            }
            title = species;
        }
        snprintf(buf, sizeof(buf), "%s%s", def->shiny ? "SHINY " : "", title);
        MP_AsciiToGame(dst, buf, 30);
    }
    else if (def->kind == K_SET)
    {
        snprintf(buf, sizeof(buf), "%s x%u", sSetNames[def->id], (unsigned)def->qty);
        MP_AsciiToGame(dst, buf, 30);
    }
    else if (def->kind == K_FX)
    {
        u8 kind, index;
        if (def->qty && MP_BallUnlock_Prize(def->qty - 1, &kind, &index))
            MP_BallUnlock_PieceName(kind, index, buf, sizeof(buf));
        else
            snprintf(buf, sizeof(buf), "BALL EFFECT?");
        MP_AsciiToGame(dst, buf, 30);
    }
    else
    {
        CopyItemName(def->id, dst);
        if (def->qty > 1)
        {
            snprintf(buf, sizeof(buf), " x%u", (unsigned)def->qty);
            MP_AsciiToGame(dst + StringLength(dst), buf, 8);
        }
    }
}

static u32 PersonalityForShiny(u32 otId)
{
    u16 lo = Random();
    u16 hi = (otId >> 16) ^ (otId & 0xFFFF) ^ lo;
    return ((u32)hi << 16) | lo;
}

static void GiveEventMon(const struct PrizeDef *def, struct Result *res)
{
    const struct EventMon *e = &sEventMons[def->id];
    u16 species = DefSpecies(def);
    struct Pokemon *mon = Alloc(sizeof(struct Pokemon));
    u32 otId = e->otName ? e->otId : (gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
              | (gSaveBlock2Ptr->playerTrainerId[2] << 16) | ((u32)gSaveBlock2Ptr->playerTrainerId[3] << 24));
    u8 i, result;

    if (def->shiny)
        CreateMon(mon, species, e->level, USE_RANDOM_IVS, TRUE, PersonalityForShiny(otId), OT_ID_PRESET, otId);
    else
        CreateMon(mon, species, e->level, USE_RANDOM_IVS, FALSE, 0, e->otName ? OT_ID_PRESET : OT_ID_PLAYER_ID, e->otId);
    if (e->moves[0] != MOVE_NONE)   // else: the moves it knows at that level
        for (i = 0; i < MAX_MON_MOVES; i++)
            SetMonMoveSlot(mon, e->moves[i], i);
    if (e->otName)
    {
        u8 ot[PLAYER_NAME_LENGTH + 1];
        u8 gender = MALE;
        MP_AsciiToGame(ot, e->otName, PLAYER_NAME_LENGTH);
        SetMonData(mon, MON_DATA_OT_NAME, ot);
        SetMonData(mon, MON_DATA_OT_GENDER, &gender);
    }
    if (e->fateful)
    {
        u8 one = TRUE, loc = METLOC_FATEFUL_ENCOUNTER;
        SetMonData(mon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, &one);
        SetMonData(mon, MON_DATA_MET_LOCATION, &loc);
    }
    {
        char ot[PLAYER_NAME_LENGTH + 2];
        u8 otName[PLAYER_NAME_LENGTH + 1];
        GetMonData(mon, MON_DATA_OT_NAME, otName);
        otName[PLAYER_NAME_LENGTH] = EOS;
        MP_GameToAscii(ot, otName, PLAYER_NAME_LENGTH);
        MPNet_Log("gatcha: event mon %d: species %d Lv%d moves %d %d %d %d OT %s %u fateful %d met %d shiny %d", def->id,
                  (int)GetMonData(mon, MON_DATA_SPECIES), (int)GetMonData(mon, MON_DATA_LEVEL),
                  (int)GetMonData(mon, MON_DATA_MOVE1), (int)GetMonData(mon, MON_DATA_MOVE2), (int)GetMonData(mon, MON_DATA_MOVE3),
                  (int)GetMonData(mon, MON_DATA_MOVE4), ot, (unsigned)GetMonData(mon, MON_DATA_OT_ID),
                  (int)GetMonData(mon, MON_DATA_MODERN_FATEFUL_ENCOUNTER), (int)GetMonData(mon, MON_DATA_MET_LOCATION),
                  (int)IsMonShiny(mon));
    }
    result = e->otName ? MP_GiveMonKeepOT(mon) : GiveMonToPlayer(mon);
    if (result == MON_CANT_GIVE)
    {
        AddMoney(&gSaveBlock1Ptr->money, 10000);
        snprintf(res->note, sizeof(res->note), "No room for it: $10000 instead.");
    }
    else
    {
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
        snprintf(res->note, sizeof(res->note), result == MON_GIVEN_TO_PARTY ? "It joined your party!" : "It was sent to your PC.");
        MP_Achieve_Unlock("eventmon");
    }
    Free(mon);
}

static void GivePrize(struct Result *res)
{
    const struct PrizeDef *def = res->def;
    res->note[0] = 0;
    res->amount = def->qty;
    switch (def->kind)
    {
    case K_MONEY:
    {
        // A bit more with friends online
        u8 n = MP_ConnectedCount();
        res->amount = def->qty * (100 + 10 * (n > 1 ? n - 1 : 0)) / 100;
        AddMoney(&gSaveBlock1Ptr->money, res->amount);
        if (n > 1)
            snprintf(res->note, sizeof(res->note), "+%d%% for friends online!", 10 * (n - 1));
        break;
    }
    case K_MON:
        GiveEventMon(def, res);
        break;
    case K_FX:
    {
        u8 kind, index;
        if (def->qty && MP_BallUnlock_Prize(def->qty - 1, &kind, &index) && MP_BallUnlock_Give(kind, index))
            snprintf(res->note, sizeof(res->note), "New! On the summary's POK~ BALL page.");
        else
        {
            MP_Gatcha_AddTickets(1);
            snprintf(res->note, sizeof(res->note), "Had it: a MOEGATCHA TICKET instead!");
            MPNet_Log("gatcha: ball piece %d already had: a ticket", (int)def->qty - 1);
        }
        break;
    }
    case K_SET:
    {
        u8 i, missed = 0;
        for (i = 0; i < ARRAY_COUNT(sSets[0]); i++)
            if (!AddBagItem(sSets[def->id][i], def->qty))
                missed++;
        if (missed)
        {
            AddMoney(&gSaveBlock1Ptr->money, 1000 * missed);
            snprintf(res->note, sizeof(res->note), "Some didn't fit: $%d instead.", 1000 * missed);
        }
        else
            snprintf(res->note, sizeof(res->note), "x%u of each of the six!", (unsigned)def->qty);
        break;
    }
    default:
        if (!AddBagItem(def->id, def->qty))
        {
            AddMoney(&gSaveBlock1Ptr->money, 3000);
            snprintf(res->note, sizeof(res->note), "Your BAG was full: $3000 instead.");
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Screen layout

#define TEXT_PAL     15
#define BG3_PAL      9
#define CELL_PAL0    1          // 1-7: one per rarity
#define CELL_TILE    1          // 16 tiles: a 32x32 rounded box
#define BG3_TILE     1          // stripes, band, gold lines

#define CELL_W       32
#define REEL_CX      120        // the selector
#define REEL_TOP     40         // y of the reel cells
#define REEL_CELLS   128        // ring buffer of cells
#define REEL_LEFT    0          // the reel runs edge to edge
#define REEL_RIGHT   DISPLAY_WIDTH
#define SPIN_CELLS   44         // how far a pull travels
#define SPIN_FRAMES  330

#define TAG_ICON     0x7200     // + slot: item icons
#define ITEM_ICON_OFFSET 4      // centers an item picture in its cell
#define TAG_FRAME    0x7240
#define TAG_ARROW    0x7241
#define TAG_CONFETTI 0x7242

enum { WIN_TITLE, WIN_INFO, WIN_NAME, WIN_MENU, WIN_HELP, WIN_COUNT };

static const struct WindowTemplate sWindows[] = {
    [WIN_TITLE] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 0, .width = 28, .height = 2, .paletteNum = TEXT_PAL, .baseBlock = 1},
    [WIN_INFO]  = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 2, .width = 28, .height = 2, .paletteNum = TEXT_PAL, .baseBlock = 1 + 56},
    [WIN_NAME]  = {.bg = 0, .tilemapLeft = 3, .tilemapTop = 9, .width = 24, .height = 4, .paletteNum = TEXT_PAL, .baseBlock = 1 + 112},
    [WIN_MENU]  = {.bg = 0, .tilemapLeft = 3, .tilemapTop = 13, .width = 24, .height = 5, .paletteNum = TEXT_PAL, .baseBlock = 1 + 208},
    [WIN_HELP]  = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 18, .width = 30, .height = 2, .paletteNum = TEXT_PAL, .baseBlock = 1 + 328},
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgs[] = {
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 1, .mapBaseIndex = 28, .screenSize = 1, .paletteMode = 0, .priority = 2, .baseTile = 0},
    {.bg = 3, .charBaseIndex = 2, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

// Text palette: 1 white, 2 shadow, 3 grey, 4-10 the rarities, 11 panel, 12 gold, 13 dim, 14 banner
static const u16 sTextPal[16] = {
    0, RGB(31, 31, 31), RGB(3, 2, 8), RGB(21, 21, 25),
    RGB(23, 23, 25), RGB(12, 30, 12), RGB(12, 22, 31), RGB(28, 14, 31), RGB(31, 15, 8), RGB(31, 28, 6), RGB(16, 31, 31),
    RGB(4, 3, 11), RGB(31, 25, 6), RGB(13, 13, 19), 0, 0,
};
enum { TC_WHITE = 1, TC_SHADOW, TC_GREY, TC_RARITY, TC_PANEL = 11, TC_GOLD, TC_DIM, TC_BANNER };

// Cells: 1 border, 2 fill, 3 top shine, 4 bottom shade
static const u16 sCellPals[R_COUNT][5] = {
    {0, RGB(25, 25, 27), RGB(11, 11, 14), RGB(15, 15, 18), RGB(7, 7, 9)},
    {0, RGB(14, 31, 14), RGB(3, 13, 5), RGB(5, 17, 7), RGB(1, 8, 2)},
    {0, RGB(15, 25, 31), RGB(3, 7, 19), RGB(5, 10, 24), RGB(1, 4, 12)},
    {0, RGB(29, 17, 31), RGB(11, 2, 17), RGB(15, 4, 22), RGB(6, 1, 10)},
    {0, RGB(31, 20, 12), RGB(20, 4, 2), RGB(26, 9, 4), RGB(10, 2, 0)},
    {0, RGB(31, 30, 14), RGB(21, 12, 0), RGB(27, 18, 2), RGB(13, 7, 0)},
    {0, RGB(31, 31, 31), RGB(5, 12, 22), RGB(18, 26, 31), RGB(3, 5, 13)},   // EX: shimmers (AnimateEx)
};
static const u16 sRarityFlash[R_COUNT] = {RGB(24, 24, 26), RGB(14, 31, 14), RGB(14, 24, 31), RGB(29, 16, 31), RGB(31, 14, 6), RGB(31, 29, 10), RGB(20, 31, 31)};

// Background: 1-2 stripes, 3 band, 4 gold line

static u16 sSpritePalThemed[16] = {
    0, RGB(31, 27, 8), RGB(31, 31, 31), RGB(31, 12, 18), RGB(10, 26, 31), RGB(12, 30, 12), RGB(28, 14, 31), RGB(31, 20, 4),
    0, 0, 0, 0, 0, 0, 0, 0,
};

enum { GS_FADE_IN, GS_MENU, GS_SPIN, GS_NUDGE, GS_HYPE, GS_EVENT, GS_REVEAL, GS_GRID, GS_EXIT };
// GS_EXIT + 1: GS_FORCE (small tweaks)
#define GS_VOTE_PICK  (GS_EXIT + 2)   // "reroll this week's PICKUP?" before asking
#define GS_VOTE_ASKED (GS_EXIT + 3)   // a friend asks while this screen is open

#define NO_CELL (-0x7FFFFFFF)   // a live icon slot that shows nothing (cells can be negative)

struct LiveIcon
{
    s32 cell;          // NO_CELL: none
    u8 spriteId;
    bool8 isMon;
    bool8 mystery;     // shows "?" (the big prize before it's revealed)
};

struct ReelCell
{
    const struct PrizeDef *def;
    u8 rarity;
    u8 shown;          // the box color shown: a big prize starts out looking RARE
};

struct GatchaState
{
    u8 state;
    u8 cursor;
    u8 menuRows[6];   // MENU_ROWS
    u8 menuCount;
    u16 timer;
    float pos, spinFrom, spinTo;
    s32 lastCenter;
    u16 lastTickFrame;
    s32 genUpTo;
    struct ReelCell cells[REEL_CELLS];
    struct LiveIcon live[16];
    struct Result results[10];
    u8 resultCount;
    u8 best;
    u8 gridIcons[10];
    bool8 gridIconMon[10];
    u8 frameSprite, arrowTop, arrowBottom;
    u16 bgScroll;
    u8 flashes;
    // The big reveal (SUPER RARE and up)
    s32 prizeCell;
    bool8 mystery;      // the prize box shows "?" until the burst
    bool8 nudge;        // the reel stops one short, then clicks onto it
    u16 hypeTimer;
    u8 raySprite;
    u16 rayRot, rayScale;
    u8 stampSprites[3];
    u16 stampTimer;
    s16 shake;          // amplitude in 1/16 px
    u16 pickupPic;      // the featured Moemon on the PICKUP banner (0xFFFF: none)
    u8 pickupShown;     // which of this week's PICKUP Moemon stands there
    u8 tagPage;         // STANDARD: the tagline, or today's featured Moemon
    u16 tagTimer;
    u16 pickupTimer;
    u8 forceRow;        // the "open" panel: 0 rarity, 1 prize
    u8 forceRarity;
    s16 forcePrize;     // -1: any
    char typed[8];      // the last letters typed
    // A legendary UR/EX: the silhouette, then the showcase
    u16 legendPic;      // 0xFFFF: none
    u8 legendSlot;      // its sprite palette slot
    u16 legendPal[16];  // its real colors (the silhouette is drawn over them)
    u16 eventTimer;
    bool8 showcase;     // the big picture is up (A goes on to the usual reveal)
};

static EWRAM_DATA struct GatchaState *sG = NULL;
// The reel's BG scroll is set in VBlank, together with the sprites (OAM), so
// the boxes and the prizes in them always move on the same frame
static u16 sBg1Hofs;
static EWRAM_DATA u16 *sBg1Map = NULL;
static EWRAM_DATA u16 *sBg3Map = NULL;

enum { MENU_FREE10, MENU_FREE1, MENU_TICKET, MENU_ONE, MENU_TEN, MENU_BACK, MENU_ROWS };

// ---------------------------------------------------------------------------
// Generated graphics

static void PutPixel(u8 *tiles, int tilesPerRow, int x, int y, u8 c)
{
    u8 *t = tiles + ((y / 8) * tilesPerRow + (x / 8)) * 32;
    int px = x % 8, py = y % 8;
    u8 *b = &t[py * 4 + px / 2];
    if (px & 1)
        *b = (*b & 0x0F) | (c << 4);
    else
        *b = (*b & 0xF0) | c;
}

// Background: diagonal stripes, the dark band behind the reel, gold lines
// (loaded again after the full-screen rays borrowed their VRAM)
static void LoadBg3Tiles(void)
{
    static u8 sBgTiles[32 * 4];
    u8 *buf = sBgTiles;
    int x, y;
    memset(buf, 0, sizeof(sBgTiles));
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
        {
            PutPixel(buf, 1, x, y, ((x + y) / 4) % 2 ? 1 : 2);                       // tile 0
            PutPixel(buf + 32, 1, x, y, 3);                                          // tile 1
            PutPixel(buf + 64, 1, x, y, y <= 1 ? 4 : 3);      // tile 2: gold line, band below it
            PutPixel(buf + 96, 1, x, y, y >= 6 ? 4 : 3);      // tile 3: band, gold line below it
        }
    LoadBgTiles(3, buf, 32 * 4, BG3_TILE);
}

static void LoadGeneratedGfx(void)
{
    // The tile copies are queued for VBlank, so the buffers must stay around
    static u8 sCellTiles[32 * 16], sBgTiles[1];
    u8 *buf = sCellTiles;
    int x, y;

    // The reel cell: a rounded box with a shine on top
    for (y = 0; y < 32; y++)
        for (x = 0; x < 32; x++)
        {
            int ex = x < 16 ? x - 2 : 29 - x, ey = y < 16 ? y - 2 : 29 - y;
            u8 c;
            if (ex < 0 || ey < 0 || ex + ey < 2)
                c = 0;
            else if (ex == 0 || ey == 0 || ex + ey == 2)
                c = 1;
            else if (y <= 6)
                c = 3;
            else if (y >= 25)
                c = 4;
            else
                c = 2;
            PutPixel(buf, 4, x, y, c);
        }
    LoadBgTiles(1, buf, 32 * 16, CELL_TILE);

    (void)sBgTiles;
    LoadBg3Tiles();

    // Sprites: the selector frame, arrows, confetti
    {
        static u8 frame[32 * 16], arrow[32 * 4], confetti[32 * 4];
        struct SpriteSheet sheet;
        struct SpritePalette pal = {sSpritePalThemed, TAG_FRAME};
        memset(frame, 0, sizeof(frame));
        for (y = 0; y < 32; y++)
            for (x = 0; x < 32; x++)
            {
                bool8 edge = x < 2 || x > 29 || y < 2 || y > 29;
                bool8 corner = (x < 9 || x > 22) && (y < 9 || y > 22);
                if (edge && corner)
                    PutPixel(frame, 4, x, y, 1);
            }
        memset(arrow, 0, sizeof(arrow));
        for (y = 0; y < 7; y++)
            for (x = 7 - y; x <= 8 + y; x++)
                PutPixel(arrow, 2, x, y + 4, y == 0 || x == 7 - y || x == 8 + y ? 2 : 1);   // pointing up
        memset(confetti, 0, sizeof(confetti));
        for (y = 0; y < 8; y++)
            for (x = 0; x < 8; x++)
            {
                if (x >= 2 && x <= 5 && y >= 1 && y <= 6) PutPixel(confetti, 1, x, y, 3);                     // ribbon
                if ((x == 3 || x == 4) || (y == 3 || y == 4)) { if (abs(x - 3) + abs(y - 3) < 4) PutPixel(confetti + 32, 1, x, y, 1); } // star
                if (x >= 2 && x <= 5 && y >= 2 && y <= 5) PutPixel(confetti + 64, 1, x, y, 4);                // square
                if (x + y >= 5 && x + y <= 8 && x >= 1 && y >= 1 && x <= 6 && y <= 6) PutPixel(confetti + 96, 1, x, y, 6); // diamond
            }
        sheet = (struct SpriteSheet){frame, sizeof(frame), TAG_FRAME};
        LoadSpriteSheet(&sheet);
        sheet = (struct SpriteSheet){arrow, sizeof(arrow), TAG_ARROW};
        LoadSpriteSheet(&sheet);
        sheet = (struct SpriteSheet){confetti, sizeof(confetti), TAG_CONFETTI};
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&pal);
    }
}

static void SpriteCB_Arrow(struct Sprite *sprite)
{
    sprite->data[1]++;
    sprite->y2 = (sprite->data[1] / 8) % 2 ? sprite->data[0] : 0;
}

static void SpriteCB_Confetti(struct Sprite *sprite)
{
    // data: 0-1 x/y speed (1/16 px), 2-3 sub-pixel, 4 life
    sprite->data[2] += sprite->data[0];
    sprite->data[3] += sprite->data[1];
    sprite->data[1] += 3;   // gravity
    sprite->x2 = sprite->data[2] / 16;
    sprite->y2 = sprite->data[3] / 16;
    if (++sprite->data[4] > 150 || sprite->y + sprite->y2 > 170)
        DestroySprite(sprite);
}

static const union AnimCmd sAnim_Confetti0[] = {ANIMCMD_FRAME(0, 8), ANIMCMD_END};
static const union AnimCmd sAnim_Confetti1[] = {ANIMCMD_FRAME(1, 8), ANIMCMD_END};
static const union AnimCmd sAnim_Confetti2[] = {ANIMCMD_FRAME(2, 8), ANIMCMD_END};
static const union AnimCmd sAnim_Confetti3[] = {ANIMCMD_FRAME(3, 8), ANIMCMD_END};
static const union AnimCmd *const sAnims_Confetti[] = {sAnim_Confetti0, sAnim_Confetti1, sAnim_Confetti2, sAnim_Confetti3};

static void SpawnConfetti(u8 count)
{
    struct SpriteTemplate t = {
        .tileTag = TAG_CONFETTI, .paletteTag = TAG_FRAME, .oam = &gOamData_AffineOff_ObjNormal_8x8,
        .anims = sAnims_Confetti, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCB_Confetti,
    };
    u8 i;
    for (i = 0; i < count; i++)
    {
        u8 id = CreateSprite(&t, REEL_CX + (Random() % 40) - 20, REEL_TOP + 16, 0);
        if (id == MAX_SPRITES)
            return;
        gSprites[id].data[0] = (s16)(Random() % 96) - 48;
        gSprites[id].data[1] = -(s16)(40 + Random() % 40);
        gSprites[id].oam.priority = 0;
        gSprites[id].coordOffsetEnabled = TRUE;
        StartSpriteAnim(&gSprites[id], i % 4);
    }
}

static u16 MixColor(u16 a, u16 b, int f);
static void LoadThemedPalettes(void);
static void DrawMenu(void);
static void ShowReveal(void);

// ---------------------------------------------------------------------------
// The big reveal: light rays, a stamp, sparkles, shaking, rainbow colors

#define TAG_RAYS   0x7243
#define TAG_STAMP  0x7244
#define RAY_COL_IN  8      // TAG_FRAME palette: ray colors
#define RAY_COL_OUT 9

static u8 sRayTiles[64 * 64 / 2];
static u8 sStampTiles[3 * 64 * 32 / 2];
static u16 sStampPal[16] = {0, RGB(31, 24, 4), RGB(12, 6, 0), RGB(3, 2, 6), RGB(31, 31, 18)};
static s8 sShakeX, sShakeY;
static u16 sBg3Hofs;

static const struct OamData sOamRays = {
    .affineMode = ST_OAM_AFFINE_DOUBLE, .shape = SPRITE_SHAPE(64x64), .size = SPRITE_SIZE(64x64), .priority = 3,
};
static const struct OamData sOamStamp = {
    .affineMode = ST_OAM_AFFINE_DOUBLE, .shape = SPRITE_SHAPE(64x32), .size = SPRITE_SIZE(64x32), .priority = 0,
};

// Bright rainbow color, h = 0..191
static u16 Hue(u16 h)
{
    u8 f = h % 32;
    switch ((h / 32) % 6)
    {
    case 0:  return RGB(31, f, 0);
    case 1:  return RGB(31 - f, 31, 0);
    case 2:  return RGB(0, 31, f);
    case 3:  return RGB(0, 31 - f, 31);
    case 4:  return RGB(f, 0, 31);
    default: return RGB(31, 0, 31 - f);
    }
}

// One color of a loaded sprite palette
static void SetSprCol(u16 tag, u8 idx, u16 color)
{
    u8 slot = IndexOfSpritePaletteTag(tag);
    if (slot == 0xFF)
        return;
    gPlttBufferUnfaded[OBJ_PLTT_ID(slot) + idx] = color;
    gPlttBufferFaded[OBJ_PLTT_ID(slot) + idx] = color;
}

// A 64x64 burst of 16 light rays
static void BuildRayTiles(void)
{
    int x, y;
    memset(sRayTiles, 0, sizeof(sRayTiles));
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            float dx = x - 31.5f, dy = y - 31.5f, re = dx, im = dy, r2 = dx * dx + dy * dy;
            int k;
            u8 c = 0;
            // (dx + i dy)^8: its sign gives 16 wedges around the middle
            for (k = 0; k < 3; k++)
            {
                float nr = re * re - im * im, ni = 2 * re * im;
                re = nr;
                im = ni;
            }
            // Thin beams: sin(8a) > 0.45, a bit wider near the middle
            {
                float r4 = r2 * r2, r8 = r4 * r4;
                if (r2 < 31 * 31 && im > (r2 < 12 * 12 ? 0.1f : 0.45f) * r8)
                    c = r2 < 14 * 14 ? RAY_COL_IN : RAY_COL_OUT;
            }
            if (r2 < 6 * 6)
                c = RAY_COL_IN;
            if (c)
                PutPixel(sRayTiles, 8, x, y, c);
        }
}

static void SetDarken(u8 level)
{
    if (level)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG3 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDY, level);
    }
    else
    {
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
    }
}

static void SetRayColors(u16 inner, u16 outer)
{
    SetSprCol(TAG_FRAME, RAY_COL_IN, inner);
    SetSprCol(TAG_FRAME, RAY_COL_OUT, outer);
}

static void RayColorsFor(u8 rarity)
{
    u16 c = sRarityFlash[rarity];
    SetRayColors(MixColor(c, RGB(31, 31, 31), 170), c);
}

static void CreateRays(void)
{
    struct SpriteSheet sheet = {sRayTiles, sizeof(sRayTiles), TAG_RAYS};
    struct SpriteTemplate t = {
        .tileTag = TAG_RAYS, .paletteTag = TAG_FRAME, .oam = &sOamRays,
        .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy,
    };
    if (sG->raySprite < MAX_SPRITES)
        return;
    if (GetSpriteTileStartByTag(TAG_RAYS) == 0xFFFF && LoadSpriteSheet(&sheet) == 0)
        return;
    sG->raySprite = CreateSprite(&t, REEL_CX, REEL_TOP + 16, 10);
    if (sG->raySprite == MAX_SPRITES)
        return;
    gSprites[sG->raySprite].affineAnimPaused = TRUE;
    gSprites[sG->raySprite].coordOffsetEnabled = TRUE;
    sG->rayRot = 0;
    sG->rayScale = 0x60;
}

static void UpdateRays(u16 speed, u16 targetScale)
{
    if (sG->raySprite >= MAX_SPRITES)
        return;
    sG->rayRot += speed;
    if (sG->rayScale < targetScale)
        sG->rayScale += (targetScale - sG->rayScale + 7) / 8;
    SetOamMatrixRotationScaling(gSprites[sG->raySprite].oam.matrixNum, sG->rayScale, sG->rayScale, sG->rayRot);
}

// ---------------------------------------------------------------------------
// Full-screen light rays behind a legendary (the showcase): an affine
// background (mode 1, BG2) with a 128x128 burst drawn 2.5x big, so the beams
// reach every corner while it spins. It borrows BG3's tile block (BG3 can't
// show in mode 1 anyway) and gives it back afterwards.

#define BIGRAY_CHARBASE 2
#define BIGRAY_MAPBASE  26
#define BIGRAY_COL_A    (BG_PLTT_ID(10) + 1)   // the two beam colors (8bpp indices)
#define BIGRAY_COL_B    (BG_PLTT_ID(10) + 2)
#define BIGRAY_COL_CORE (BG_PLTT_ID(10) + 3)

static bool8 sBigRays;
static u16 sBigRayRot, sBigRayScale;
static s16 sBigRayCx, sBigRayCy;

static void SetBgColor(u16 index, u16 color)
{
    gPlttBufferUnfaded[index] = color;
    gPlttBufferFaded[index] = color;
}

static void StartBigRays(s16 cx, s16 cy)
{
    static u8 tiles[16 * 16 * 64];
    static u8 map[16 * 16];
    int x, y;
    if (sBigRays)
        return;
    memset(tiles, 0, sizeof(tiles));
    for (y = 0; y < 128; y++)
        for (x = 0; x < 128; x++)
        {
            float dx = x - 63.5f, dy = y - 63.5f, re = dx, im = dy, r2 = dx * dx + dy * dy, r8;
            int k;
            u8 c = 0;
            // (dx + i dy)^8: sin(8a) * r^8. Beams where it's well above 0 (one
            // color) or well below (the other): 16 beams, two colors in turn
            for (k = 0; k < 3; k++)
            {
                float nr = re * re - im * im, ni = 2 * re * im;
                re = nr;
                im = ni;
            }
            r8 = r2 * r2 * r2 * r2;
            if (im > 0.30f * r8)
                c = 1;
            else if (im < -0.30f * r8)
                c = 2;
            if (r2 < 7 * 7)
                c = 3;
            if (c)
                tiles[((y / 8) * 16 + x / 8) * 64 + (y % 8) * 8 + (x % 8)] = BG_PLTT_ID(10) + c;
        }
    for (x = 0; x < 256; x++)
        map[x] = x;
    LoadBgTiles(3, tiles, sizeof(tiles), 0);   // BG3's tile block: BIGRAY_CHARBASE
    memcpy((void *)(BG_VRAM + BIGRAY_MAPBASE * 0x800), map, sizeof(map));
    SetGpuReg(REG_OFFSET_BG2CNT, BGCNT_PRIORITY(3) | BGCNT_CHARBASE(BIGRAY_CHARBASE) | BGCNT_SCREENBASE(BIGRAY_MAPBASE) | BGCNT_AFF128x128);
    sBigRayRot = 0;
    sBigRayScale = 0x100;
    sBigRayCx = cx;
    sBigRayCy = cy;
    sBigRays = TRUE;
    SetGpuReg(REG_OFFSET_DISPCNT, (GetGpuReg(REG_OFFSET_DISPCNT) & ~7) | DISPCNT_MODE_1 | DISPCNT_BG2_ON);
}

static void StopBigRays(void)
{
    if (!sBigRays)
        return;
    sBigRays = FALSE;
    SetGpuReg(REG_OFFSET_DISPCNT, (GetGpuReg(REG_OFFSET_DISPCNT) & ~(7 | DISPCNT_BG2_ON)) | DISPCNT_MODE_0);
    LoadBg3Tiles();
    SetBgColor(0, 0);
}

// Spins the beams and cycles their colors (h: rainbow position)
static void UpdateBigRays(u16 speed, u16 h)
{
    s32 pa, pb, pc, pd, scale, cx, cy, refX, refY;
    u8 a;
    if (!sBigRays)
        return;
    sBigRayRot += speed;
    if (sBigRayScale < 0x280)
        sBigRayScale += (0x280 - sBigRayScale + 7) / 8;   // bursts out, then 2.5x
    scale = sBigRayScale;
    a = sBigRayRot >> 8;
    // Screen -> texture: rotate the other way and shrink by the scale
    pa = Cos(a, 256) * 256 / scale;
    pb = Sin(a, 256) * 256 / scale;
    pc = -Sin(a, 256) * 256 / scale;
    pd = Cos(a, 256) * 256 / scale;
    cx = sBigRayCx + sShakeX;
    cy = sBigRayCy + sShakeY;
    // The texture's middle (64,64) lands on (cx,cy)
    refX = 64 * 256 - (pa * cx + pb * cy);
    refY = 64 * 256 - (pc * cx + pd * cy);
    SetGpuReg(REG_OFFSET_BG2PA, (u16)pa);
    SetGpuReg(REG_OFFSET_BG2PB, (u16)pb);
    SetGpuReg(REG_OFFSET_BG2PC, (u16)pc);
    SetGpuReg(REG_OFFSET_BG2PD, (u16)pd);
    SetGpuReg(REG_OFFSET_BG2X_L, (u16)refX);
    SetGpuReg(REG_OFFSET_BG2X_H, (u16)(refX >> 16) & 0x0FFF);
    SetGpuReg(REG_OFFSET_BG2Y_L, (u16)refY);
    SetGpuReg(REG_OFFSET_BG2Y_H, (u16)(refY >> 16) & 0x0FFF);
    SetBgColor(BIGRAY_COL_A, MixColor(Hue(h), RGB(31, 31, 31), 110));
    SetBgColor(BIGRAY_COL_B, MixColor(Hue(h + 96), RGB(31, 31, 31), 110));
    SetBgColor(BIGRAY_COL_CORE, RGB(31, 31, 31));
    SetBgColor(0, MixColor(RGB(2, 1, 6), Hue(h + 48), 60));   // between the beams
}

static void DestroyRays(void)
{
    if (sG->raySprite < MAX_SPRITES)
    {
        FreeSpriteOamMatrix(&gSprites[sG->raySprite]);
        DestroySprite(&gSprites[sG->raySprite]);
    }
    sG->raySprite = MAX_SPRITES;
    FreeSpriteTilesByTag(TAG_RAYS);
}

// Sparkles: little stars that float up out of the prize box
static void SpriteCB_Sparkle(struct Sprite *sprite)
{
    // data: 0-1 speed (1/16 px), 2-3 sub-pixel, 4 age, 5 life
    sprite->data[2] += sprite->data[0];
    sprite->data[3] += sprite->data[1];
    sprite->x2 = sprite->data[2] / 16;
    sprite->y2 = sprite->data[3] / 16;
    sprite->invisible = sprite->data[4] > sprite->data[5] - 12 && (sprite->data[4] & 2);
    if (++sprite->data[4] > sprite->data[5])
        DestroySprite(sprite);
}

static void SpawnSparkles(u8 count, bool8 burst)
{
    struct SpriteTemplate t = {
        .tileTag = TAG_CONFETTI, .paletteTag = TAG_FRAME, .oam = &gOamData_AffineOff_ObjNormal_8x8,
        .anims = sAnims_Confetti, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCB_Sparkle,
    };
    u8 i;
    for (i = 0; i < count; i++)
    {
        u8 id = CreateSprite(&t, REEL_CX + (Random() % 28) - 14, REEL_TOP + 16 + (Random() % 20) - 10, 1);
        if (id == MAX_SPRITES)
            return;
        if (burst)
        {
            // Out in all directions
            u8 a = Random();
            gSprites[id].data[0] = Cos(a, 40 + Random() % 30);
            gSprites[id].data[1] = Sin(a, 40 + Random() % 30);
            gSprites[id].data[5] = 30 + Random() % 15;
        }
        else
        {
            gSprites[id].data[0] = (s16)(Random() % 16) - 8;
            gSprites[id].data[1] = -(s16)(14 + Random() % 20);
            gSprites[id].data[5] = 40 + Random() % 20;
        }
        gSprites[id].oam.priority = 0;
        gSprites[id].coordOffsetEnabled = TRUE;
        StartSpriteAnim(&gSprites[id], 1);   // the star
    }
}

// Stars raining down the whole screen (ULTRA RARE)
static void SpawnFallingStar(void)
{
    struct SpriteTemplate t = {
        .tileTag = TAG_CONFETTI, .paletteTag = TAG_FRAME, .oam = &gOamData_AffineOff_ObjNormal_8x8,
        .anims = sAnims_Confetti, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCB_Sparkle,
    };
    u8 id = CreateSprite(&t, Random() % DISPLAY_WIDTH, -4, 1);
    if (id == MAX_SPRITES)
        return;
    gSprites[id].data[0] = -(s16)(4 + Random() % 8);
    gSprites[id].data[1] = 28 + Random() % 20;
    gSprites[id].data[5] = 100;
    gSprites[id].oam.priority = 0;
    StartSpriteAnim(&gSprites[id], 1);
}

// The stamp: "SUPER RARE!!" in big letters (double size, outlined), as three
// 64x32 sprites that zoom in and land with a bounce
static void BuildStampTiles(const char *text)
{
    static const struct WindowTemplate sTmp = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 16, .height = 2, .paletteNum = TEXT_PAL, .baseBlock = 0x300};
    static u8 px[32][192];
    u8 buf[32], colors[3] = {0, 1, 2};
    u8 win = AddWindow(&sTmp);
    const u8 *src;
    int w, pad, x, y, k;

    memset(px, 0, sizeof(px));
    memset(sStampTiles, 0, sizeof(sStampTiles));
    if (win == WINDOW_NONE)
        return;
    FillWindowPixelBuffer(win, PIXEL_FILL(0));
    MP_AsciiToGame(buf, text, 30);
    w = GetStringWidth(FONT_NORMAL, buf, 0);
    if (w > 94)
        w = 94;
    AddTextPrinterParameterized3(win, FONT_NORMAL, 0, 0, colors, TEXT_SKIP_DRAW, buf);
    src = gWindows[win].tileData;
    pad = (192 - w * 2) / 2;
    for (y = 0; y < 32; y++)
        for (x = 0; x < 192; x++)
        {
            int sx = (x - pad) / 2, sy = y / 2;
            u8 c;
            if (x < pad || sx >= w)
                continue;
            c = (src[((sy / 8) * 16 + sx / 8) * 32 + (sy % 8) * 4 + (sx % 8) / 2] >> ((sx & 1) * 4)) & 15;
            if (c == 1)
                c = (y < 15) ? 4 : 1;   // lighter top half
            px[y][x] = c;
        }
    RemoveWindow(win);
    // Dark outline around the letters
    for (y = 0; y < 32; y++)
        for (x = 0; x < 192; x++)
        {
            int dx, dy;
            if (px[y][x])
                continue;
            for (dy = -1; dy <= 1; dy++)
                for (dx = -1; dx <= 1; dx++)
                {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < 192 && ny >= 0 && ny < 32 && px[ny][nx] && px[ny][nx] != 5)
                        px[y][x] = 5;   // marked, made 3 below (so it doesn't spread)
                }
        }
    for (k = 0; k < 3; k++)
        for (y = 0; y < 32; y++)
            for (x = 0; x < 64; x++)
            {
                u8 c = px[y][k * 64 + x];
                if (c == 5)
                    c = 3;
                if (c)
                    PutPixel(sStampTiles + k * 1024, 8, x, y, c);
            }
}

static void CreateStamp(u8 rarity)
{
    struct SpriteSheet sheet = {sStampTiles, sizeof(sStampTiles), TAG_STAMP};
    struct SpritePalette pal = {sStampPal, TAG_STAMP};
    struct SpriteTemplate t = {
        .tileTag = TAG_STAMP, .paletteTag = TAG_STAMP, .oam = &sOamStamp,
        .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy,
    };
    u8 k;
    bool8 legend = rarity >= R_UR && sG->results[sG->best].def->kind == K_MON && IsLegendSpecies(DefSpecies(sG->results[sG->best].def));
    switch (rarity)
    {
    case R_EX:
        BuildStampTiles("EX!!!!");
        sStampPal[1] = RGB(16, 31, 31);
        sStampPal[4] = RGB(31, 31, 31);
        sStampPal[2] = RGB(2, 8, 16);
        break;
    case R_UR:
        BuildStampTiles(legend ? "LEGENDARY!!!" : "ULTRA RARE!!!");
        sStampPal[1] = RGB(31, 22, 2);
        sStampPal[4] = RGB(31, 31, 16);
        sStampPal[2] = RGB(14, 6, 0);
        break;
    case R_SSR:
        BuildStampTiles("SSR!!!");
        sStampPal[1] = RGB(31, 11, 4);
        sStampPal[4] = RGB(31, 25, 14);
        sStampPal[2] = RGB(14, 3, 0);
        break;
    default:
        BuildStampTiles("SUPER RARE!!");
        sStampPal[1] = RGB(25, 8, 31);
        sStampPal[4] = RGB(31, 22, 31);
        sStampPal[2] = RGB(9, 1, 14);
        break;
    }
    FreeSpriteTilesByTag(TAG_STAMP);
    FreeSpritePaletteByTag(TAG_STAMP);
    if (LoadSpriteSheet(&sheet) == 0 || LoadSpritePalette(&pal) == 0xFF)
        return;
    for (k = 0; k < 3; k++)
    {
        u8 id = CreateSprite(&t, REEL_CX, 22, 0);
        sG->stampSprites[k] = id;
        if (id == MAX_SPRITES)
            continue;
        gSprites[id].sheetTileStart += k * 32;
        gSprites[id].oam.tileNum += k * 32;
        gSprites[id].affineAnimPaused = TRUE;
        gSprites[id].coordOffsetEnabled = TRUE;
    }
    sG->stampTimer = 0;
}

// Zooms in from 2.5x, overshoots a little and settles
static void UpdateStamp(void)
{
    static const u16 sScales[] = {0x280, 0x240, 0x200, 0x1C0, 0x180, 0x150, 0x128, 0x108, 0xF0, 0xE4, 0xE8, 0xF4, 0x100};
    u16 t = sG->stampTimer++, scale;
    u8 k;
    scale = t < ARRAY_COUNT(sScales) ? sScales[t] : 0x100 + Sin((t * 6) & 0xFF, 8);   // then it breathes
    for (k = 0; k < 3; k++)
    {
        u8 id = sG->stampSprites[k];
        if (id >= MAX_SPRITES)
            continue;
        SetOamMatrixRotationScaling(gSprites[id].oam.matrixNum, scale, scale, 0);
        gSprites[id].x = REEL_CX + ((k - 1) * 64 * scale) / 0x100;
    }
}

static void DestroyStamp(void)
{
    u8 k;
    for (k = 0; k < 3; k++)
    {
        if (sG->stampSprites[k] < MAX_SPRITES)
        {
            FreeSpriteOamMatrix(&gSprites[sG->stampSprites[k]]);
            DestroySprite(&gSprites[sG->stampSprites[k]]);
        }
        sG->stampSprites[k] = MAX_SPRITES;
    }
    FreeSpriteTilesByTag(TAG_STAMP);
    FreeSpritePaletteByTag(TAG_STAMP);
}

// The prize box glows brighter and dimmer
static void PulseCell(u8 rarity, u16 t)
{
    u16 pal[5];
    u8 i;
    int f = 80 + Sin((t * 12) & 0xFF, 70);
    for (i = 0; i < 5; i++)
        pal[i] = i ? MixColor(sCellPals[rarity][i], RGB(31, 31, 31), f) : 0;
    LoadPalette(pal, BG_PLTT_ID(CELL_PAL0 + rarity), sizeof(pal));
}

static void Bg3Rainbow(u16 h)
{
    u16 bg3[5];
    bg3[0] = 0;
    bg3[1] = MixColor(0, Hue(h), 120);
    bg3[2] = MixColor(0, Hue(h + 16), 160);
    bg3[3] = MixColor(0, Hue(h + 96), 50);
    bg3[4] = Hue(h + 48);
    LoadPalette(bg3, BG_PLTT_ID(BG3_PAL), sizeof(bg3));
}

// ---------------------------------------------------------------------------
// Reel

static void DrawCellTiles(u16 *map, int tileX, int tileY, u8 rarity, bool8 wrap)
{
    int x, y;
    for (y = 0; y < 4; y++)
        for (x = 0; x < 4; x++)
        {
            int tx = tileX + x, ty = tileY + y;
            int idx;
            if (wrap)
                tx &= 63;
            // 64x32 map: two 32x32 screens side by side
            idx = (tx >= 32 ? 32 * 32 : 0) + ty * 32 + (tx & 31);
            map[idx] = (CELL_TILE + y * 4 + x) | ((CELL_PAL0 + rarity) << 12);
        }
}

static void ClearBg1(void)
{
    memset(sBg1Map, 0, 64 * 32 * 2);
}

static struct ReelCell *Cell(s32 i)
{
    return &sG->cells[((i % REEL_CELLS) + REEL_CELLS) % REEL_CELLS];
}

// Teasers: lots of flashy prizes going by, never the same picture twice in
// view: a cell's icon differs from every other within TEASER_GAP cells (more
// than the screen holds) whenever the banner has enough different prizes
#define TEASER_GAP 8

static s32 sFixedCells[3];   // cells already decided ahead of the ones being filled (the prize and its neighbours)
static u8 sFixedCount;

// What a cell's icon shows: an item, or a Moemon (a random one, still a "?")
static u16 IconKey(const struct PrizeDef *def)
{
    if (def->kind == K_MON)
        return 0x8000 | DefSpecies(def);
    if (def->kind == K_FX)
        return 0x7FFF;
    return def->kind == K_SET ? sSets[def->id][0] : def->id;   // MONEY: the AMULET COIN
}

// Cells up to hi are filled in already, and so are the fixed ones
static bool8 CellFilled(s32 j, s32 hi)
{
    u8 k;
    if (Cell(j)->def == NULL)
        return FALSE;
    if (j <= hi)
        return TRUE;
    for (k = 0; k < sFixedCount; k++)
        if (sFixedCells[k] == j)
            return TRUE;
    return FALSE;
}

// How close the nearest cell with the same icon is (TEASER_GAP + 1: none that close)
static s32 RepeatDistance(s32 n, s32 hi, u16 key)
{
    s32 d;
    for (d = 1; d <= TEASER_GAP; d++)
        if ((CellFilled(n - d, hi) && IconKey(Cell(n - d)->def) == key)
         || (CellFilled(n + d, hi) && IconKey(Cell(n + d)->def) == key))
            return d;
    return TEASER_GAP + 1;
}

static u8 TeaserRarity(void)
{
    u16 r = Random() % 1000;
    // Flashy, but a legendary going by is still something to see
    return r < 6 ? R_EX : r < 30 ? R_UR : r < 90 ? R_SSR : r < 240 ? R_SR : r < 480 ? R_R : r < 730 ? R_U : R_N;
}

// A prize for cell n of this rarity (0xFF: a teaser's random one) whose icon
// isn't close by; the best of a few tries when the pools are small
static const struct PrizeDef *PickVaried(s32 n, s32 hi, u8 rarity, u8 *rarityOut)
{
    const struct PrizeDef *best = NULL;
    s32 bestDist = -1;
    u8 bestRarity = 0, try;
    for (try = 0; try < 24; try++)
    {
        u8 r = rarity != 0xFF ? rarity : TeaserRarity();
        const struct PrizeDef *def = PickFromPool(r);
        s32 dist = RepeatDistance(n, hi, IconKey(def));
        if (dist > bestDist)
        {
            best = def;
            bestDist = dist;
            bestRarity = r;
            if (dist > TEASER_GAP)
                break;
        }
    }
    if (rarityOut)
        *rarityOut = bestRarity;
    return best;
}

static void MakeTeaserAt(s32 n, s32 hi)
{
    struct ReelCell *c = Cell(n);
    u8 r;
    c->def = PickVaried(n, hi, 0xFF, &r);
    c->rarity = c->shown = r;
}

static void GenerateAhead(s32 upTo)
{
    while (sG->genUpTo < upTo)
    {
        MakeTeaserAt(sG->genUpTo + 1, sG->genUpTo);
        sG->genUpTo++;
    }
}

static s32 CenterCell(void)
{
    return FloorF((sG->pos + CELL_W / 2) / CELL_W);
}

#define TAG_ITEM_PAL 0x7300   // + item id: icons of the same item share one palette

static u16 sIconItem[16 + 10];   // the item each icon slot shows (reel slots, then the grid)

static void FreeItemPalIfUnused(u16 item)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sIconItem); i++)
        if (sIconItem[i] == item)
            return;
    FreeSpritePaletteByTag(TAG_ITEM_PAL + item);
}

static void DestroyItemIcon(u8 spriteId, u8 slot)
{
    u16 item = sIconItem[slot];
    DestroySprite(&gSprites[spriteId]);
    FreeSpriteTilesByTag(TAG_ICON + slot);
    sIconItem[slot] = 0;
    if (item)
        FreeItemPalIfUnused(item);
}

static void DestroyLive(struct LiveIcon *l, u8 slot)
{
    if (l->cell == NO_CELL)
        return;
    if (l->spriteId < MAX_SPRITES)
    {
        if (l->isMon)
            FreeAndDestroyMonIconSprite(&gSprites[l->spriteId]);
        else
            DestroyItemIcon(l->spriteId, slot);
    }
    l->cell = NO_CELL;
}

static bool8 SeenSpecies(u16 species)
{
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN);
}

// An all-dark sprite palette for Moemon not seen yet
#define TAG_SHADOW_PAL 0x7246
static u8 ShadowPalSlot(void)
{
    u8 slot = IndexOfSpritePaletteTag(TAG_SHADOW_PAL);
    if (slot == 0xFF)
    {
        u16 pal[16];
        u8 i;
        struct SpritePalette sp = {pal, TAG_SHADOW_PAL};
        pal[0] = 0;
        for (i = 1; i < 16; i++)
            pal[i] = RGB(3, 3, 6);
        slot = LoadSpritePalette(&sp);
    }
    return slot;
}

static u8 CreateIcon(const struct PrizeDef *def, u8 slot, s16 x, s16 y, bool8 *isMon, bool8 mystery)
{
    u8 id;
    if (def->kind == K_MON && DefSpecies(def) == SPECIES_NONE)
        mystery = TRUE;   // a random Moemon: nobody knows yet
    if (def->kind == K_MON && !mystery)
    {
        u16 species = DefSpecies(def);
        LoadMonIconPalette(species);   // only the palettes in use: item icons need theirs too
        if (IndexOfSpritePaletteTag(gMonIconPaletteTable[gMonIconPaletteIndices[species]].tag) == 0xFF)
            return MAX_SPRITES;
        id = CreateMonIcon(species, SpriteCB_MonIcon, x, y - 4, 1, 0, FALSE);   // FALSE: Kai has no DEOXYS form icons
        *isMon = TRUE;
        // Not seen yet: only its shadow, so the design isn't spoiled
        if (id < MAX_SPRITES && !SeenSpecies(species))
        {
            u8 shadow = ShadowPalSlot();
            if (shadow != 0xFF)
                gSprites[id].oam.paletteNum = shadow;
        }
    }
    else
    {
        *isMon = FALSE;
        u16 item = mystery ? ITEM_NONE : def->kind == K_SET ? sSets[def->id][0] : def->id;   // ITEM_NONE: "?"; a set shows its first item
        id = AddItemIconSprite(TAG_ICON + slot, TAG_ITEM_PAL + item, item);
        if (id < MAX_SPRITES && IndexOfSpritePaletteTag(TAG_ITEM_PAL + item) == 0xFF)
        {
            // No palette slot left: an empty box looks better than garbage
            DestroySprite(&gSprites[id]);
            FreeSpriteTilesByTag(TAG_ICON + slot);
            id = MAX_SPRITES;
        }
        if (id < MAX_SPRITES)
        {
            sIconItem[slot] = item;
            // The 24x24 picture sits in the top-left of its 32x32 sprite
            gSprites[id].x = x;
            gSprites[id].y = y;
            gSprites[id].x2 = ITEM_ICON_OFFSET;
            gSprites[id].y2 = ITEM_ICON_OFFSET;
        }
    }
    if (id < MAX_SPRITES)
    {
        gSprites[id].oam.priority = 1;
        gSprites[id].subpriority = 2;
        gSprites[id].coordOffsetEnabled = TRUE;   // shakes with the screen
    }
    return id;
}

static void UpdateReel(void)
{
    // One extra cell on the right, where new ones come in: icons are made while
    // still hidden past the edge, so a new one never shows up half loaded
    s32 first = FloorF((sG->pos - (REEL_CX - REEL_LEFT) - CELL_W / 2) / CELL_W);
    s32 last = CeilF((sG->pos + (REEL_RIGHT - REEL_CX) + CELL_W / 2) / CELL_W) + 1;
    s32 i;
    u8 slot;

    GenerateAhead(last + 2);
    // BG1: cell boxes. Screen x of cell i's left edge = REEL_CX - 16 + i*32 - pos
    for (i = first - 1; i <= last + 1; i++)
        DrawCellTiles(sBg1Map, i * 4, REEL_TOP / 8, Cell(i)->shown, TRUE);
    CopyBgTilemapBufferToVram(1);
    sBg1Hofs = (FloorF(sG->pos) - (REEL_CX - CELL_W / 2)) & 511;
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);

    // Sprites
    for (slot = 0; slot < 16; slot++)
    {
        struct LiveIcon *l = &sG->live[slot];
        if (l->cell != NO_CELL && (l->cell < first || l->cell > last || ((l->cell % 16) + 16) % 16 != slot))
            DestroyLive(l, slot);
    }
    for (i = first; i <= last; i++)
    {
        struct LiveIcon *l;
        s16 x = REEL_CX + i * CELL_W - FloorF(sG->pos);
        bool8 mystery = sG->mystery && i == sG->prizeCell;
        slot = ((i % 16) + 16) % 16;
        l = &sG->live[slot];
        if (l->cell != i || l->mystery != mystery)
        {
            DestroyLive(l, slot);
            if (Cell(i)->def == NULL)   // never filled in: give it a prize to show
                MakeTeaserAt(i, sG->genUpTo);
            l->spriteId = CreateIcon(Cell(i)->def, slot, x, REEL_TOP + 16, &l->isMon, mystery);
            l->cell = i;
            l->mystery = mystery;
        }
        if (l->spriteId < MAX_SPRITES)
            gSprites[l->spriteId].x = x;
    }
}

static void DestroyAllLive(void)
{
    u8 slot;
    for (slot = 0; slot < 16; slot++)
        DestroyLive(&sG->live[slot], slot);
}

// ---------------------------------------------------------------------------
// Text

static void Print(u8 win, u8 font, int x, int y, u8 bg, u8 fg, const u8 *str)
{
    u8 colors[3] = {bg, fg, TC_SHADOW};
    AddTextPrinterParameterized3(win, font, x, y, colors, TEXT_SKIP_DRAW, str);
}

static void PrintAscii(u8 win, u8 font, int x, int y, u8 bg, u8 fg, const char *s)
{
    u8 buf[96];
    MP_AsciiToGame(buf, s, 90);
    Print(win, font, x, y, bg, fg, buf);
}

static void PrintCentered(u8 win, u8 font, int y, u8 bg, u8 fg, const u8 *str)
{
    int w = sWindows[win].width * 8;
    int x = (w - GetStringWidth(font, str, 0)) / 2;
    Print(win, font, x < 0 ? 0 : x, y, bg, fg, str);
}

static void PrintCenteredAscii(u8 win, u8 font, int y, u8 bg, u8 fg, const char *s)
{
    u8 buf[96];
    MP_AsciiToGame(buf, s, 90);
    PrintCentered(win, font, y, bg, fg, buf);
}

static void DrawHeader(void)
{
    char line[64], srKey[24], ssrKey[24], urKey[24];
    s32 sinceSR, sinceSSR, sinceUR;
    PityKeys(srKey, ssrKey, urKey);
    sinceSR = MP_SaveData_Get(srKey);
    sinceSSR = MP_SaveData_Get(ssrKey);
    sinceUR = MP_SaveData_Get(urKey);

    FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(0));
    PrintAscii(WIN_TITLE, FONT_NORMAL, 2, 1, 0, TC_GOLD, "MOEGATCHA");
    if (MP_Gatcha_Tickets() > 0)
        snprintf(line, sizeof(line), "PULLS %d  TICKETS %d", (int)MP_Gatcha_Pulls(), (int)MP_Gatcha_Tickets());
    else
        snprintf(line, sizeof(line), "PULLS: %d", (int)MP_Gatcha_Pulls());
    {
        u8 buf[40];
        MP_AsciiToGame(buf, line, 38);
        Print(WIN_TITLE, FONT_NORMAL, 28 * 8 - 4 - GetStringWidth(FONT_NORMAL, buf, 0), 1, 0, TC_WHITE, buf);
    }
    CopyWindowToVram(WIN_TITLE, COPYWIN_FULL);

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
    // Pity: how many pulls at most until the next SR+ / SSR+ / UR+
    {
        u8 buf[40];
        snprintf(line, sizeof(line), "SR+ in %d", (int)(SR_PITY - sinceSR));
        PrintAscii(WIN_INFO, FONT_SMALL, 2, 1, 0, TC_RARITY + R_SR, line);
        snprintf(line, sizeof(line), "SSR+ in %d", (int)(SSR_PITY - sinceSSR));
        MP_AsciiToGame(buf, line, 38);
        Print(WIN_INFO, FONT_SMALL, (28 * 8 - GetStringWidth(FONT_SMALL, buf, 0)) / 2, 1, 0, TC_RARITY + R_SSR, buf);
        snprintf(line, sizeof(line), "UR+ in %d", (int)(UR_PITY - sinceUR));
        MP_AsciiToGame(buf, line, 38);
        Print(WIN_INFO, FONT_SMALL, 28 * 8 - 2 - GetStringWidth(FONT_SMALL, buf, 0), 1, 0, TC_RARITY + R_UR, buf);
    }
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static void DrawHelp(const char *s)
{
    u8 buf[160];
    u8 font = FONT_SMALL;
    int len, maxW = 30 * 8 - 8;
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(TC_PANEL));
    MP_AsciiToGame(buf, s, 150);
    if (GetStringWidth(font, buf, 0) > maxW)
        font = FONT_SMALL_NARROW;
    for (len = StringLength(buf); len > 1 && GetStringWidth(font, buf, 0) > maxW; len--)
    {
        buf[len - 1] = EOS;
        if (len >= 2 && buf[len - 2] == CHAR_KEYPAD_ICON)   // not half a button icon
            buf[--len - 1] = EOS;
    }
    Print(WIN_HELP, font, 4, 2, TC_PANEL, TC_GREY, buf);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

// The "?" box: only its (fake) rarity shows
static void DrawMysteryLine(u8 shown)
{
    FillWindowPixelBuffer(WIN_NAME, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_NAME, FONT_SMALL, 2, TC_PANEL, TC_RARITY + shown, sRarityNames[shown]);
    PrintCenteredAscii(WIN_NAME, FONT_NORMAL, 14, TC_PANEL, TC_RARITY + shown, "? ? ?");
    CopyWindowToVram(WIN_NAME, COPYWIN_FULL);
}

static void DrawNameLine(const struct PrizeDef *def, u8 rarity, u32 amount, bool8 big)
{
    u8 name[48];
    FillWindowPixelBuffer(WIN_NAME, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_NAME, FONT_SMALL, 2, TC_PANEL, TC_RARITY + rarity, sRarityNames[rarity]);
    PrizeName(def, amount, name);
    PrintCentered(WIN_NAME, FONT_NORMAL, 14, TC_PANEL, big ? TC_RARITY + rarity : TC_WHITE, name);
    CopyWindowToVram(WIN_NAME, COPYWIN_FULL);
}

static void BuildMenu(void)
{
    sG->menuCount = 0;
    if (sBanner == BANNER_STANDARD && Free10Stock() > 0)
        sG->menuRows[sG->menuCount++] = MENU_FREE10;
    if (Free1Stock(sBanner) > 0)
        sG->menuRows[sG->menuCount++] = MENU_FREE1;
    if (MP_Gatcha_Tickets() > 0)
        sG->menuRows[sG->menuCount++] = MENU_TICKET;
    sG->menuRows[sG->menuCount++] = MENU_ONE;
    sG->menuRows[sG->menuCount++] = MENU_TEN;
    sG->menuRows[sG->menuCount++] = MENU_BACK;
    if (sG->cursor >= sG->menuCount)
        sG->cursor = 0;
}

// The banner's name between L and R, and what it's about
static void DrawBannerLine(void)
{
    char line[48];
    int width = sBanner == BANNER_PICKUP ? 156 : 24 * 8;   // the featured Moemon stands on the right
    u8 buf[48];
    FillWindowPixelBuffer(WIN_NAME, PIXEL_FILL(TC_PANEL));
    PrintAscii(WIN_NAME, FONT_SMALL, 4, 2, TC_PANEL, TC_GREY, "< {L}");
    MP_AsciiToGame(buf, "{R} >", 20);
    if (sBanner != BANNER_PICKUP)
        Print(WIN_NAME, FONT_SMALL, 24 * 8 - 4 - GetStringWidth(FONT_SMALL, buf, 0), 2, TC_PANEL, TC_GREY, buf);
    else
    {
        Print(WIN_NAME, FONT_SMALL, width - GetStringWidth(FONT_SMALL, buf, 0), 2, TC_PANEL, TC_GREY, buf);
    }
    MP_AsciiToGame(buf, sBanners[sBanner].name, 40);
    Print(WIN_NAME, FONT_SMALL, (width - GetStringWidth(FONT_SMALL, buf, 0)) / 2, 2, TC_PANEL, TC_BANNER, buf);
    if (sBanner == BANNER_PICKUP)
    {
        const struct Pickup *pu = ThisWeeksPickup();
        u8 i;
        if (PickupIsRandom())
            snprintf(line, sizeof(line), "UR: a random SHINY Moemon!");
        else
        {
            int n = snprintf(line, sizeof(line), "UR:");
            for (i = 0; i < pu->count; i++)
            {
                u8 name[POKEMON_NAME_LENGTH + 1];
                char a[16];
                GetSpeciesName(name, sEventMons[pu->mons[i]].species);
                MP_GameToAscii(a, name, POKEMON_NAME_LENGTH);
                n += snprintf(line + n, sizeof(line) - n, "%s%s", i ? "/" : " ", a);
            }
        }
    }
    else if (sBanner == BANNER_STANDARD && sG->tagPage % 5 != 0)
    {
        // Taking turns with the tagline: today's legends, then today's event Moemon
        u8 page = sG->tagPage % 5, mons[DAILY_MONS], i, from, count;
        const u8 *evs;
        int n;
        if (page <= 2)
        {
            evs = TodaysLegends();
            from = (page - 1) * 3;
            count = 3;
            n = snprintf(line, sizeof(line), "TODAY'S UR:");
        }
        else
        {
            evs = TodaysMons(mons);
            from = (page - 3) * 2;
            count = 2;
            n = snprintf(line, sizeof(line), "TODAY'S R:");
        }
        for (i = from; i < from + count; i++)
        {
            u8 name[POKEMON_NAME_LENGTH + 1];
            char a[16];
            GetSpeciesName(name, sEventMons[evs[i]].species);
            MP_GameToAscii(a, name, POKEMON_NAME_LENGTH);
            n += snprintf(line + n, sizeof(line) - n, "%s%s", i == from ? " " : "/", a);
        }
    }
    else
        snprintf(line, sizeof(line), "%s", sBanners[sBanner].tagline);
    MP_AsciiToGame(buf, line, 44);
    {
        // the widest font that fits (three featured names need the narrow one)
        u8 font = FONT_NORMAL;
        s32 w;
        if (GetStringWidth(font, buf, 0) > width - 8)
            font = FONT_SMALL;
        if (GetStringWidth(font, buf, 0) > width - 4)
            font = FONT_SMALL_NARROW;
        w = GetStringWidth(font, buf, 0);
        Print(WIN_NAME, font, w < width ? (width - w) / 2 : 0, font == FONT_NORMAL ? 14 : 16, TC_PANEL, TC_WHITE, buf);
    }
    CopyWindowToVram(WIN_NAME, COPYWIN_FULL);
}

static void SpriteCB_PickupPic(struct Sprite *sprite)
{
    sprite->data[0]++;
    sprite->y2 = Sin((sprite->data[0] * 3) & 0xFF, 3);
}

#define TAG_PICKUP_PAL 0x7247

static void DestroyPickupPic(void)
{
    if (sG->pickupPic != 0xFFFF)
    {
        FreeAndDestroyMonPicSprite(sG->pickupPic);
        FreeSpritePaletteByTag(TAG_PICKUP_PAL);
        sG->pickupPic = 0xFFFF;
    }
}

// This week's PICKUP Moemon, standing next to the banner (taking turns when
// there are several). One not seen yet is only a shadow.
static void ShowPickupPic(void)
{
    DestroyPickupPic();
    if (sBanner == BANNER_PICKUP && !PickupIsRandom())
    {
        const struct Pickup *pu = ThisWeeksPickup();
        u16 species = sEventMons[pu->mons[sG->pickupShown % pu->count]].species;
        u8 slot = AllocSpritePalette(TAG_PICKUP_PAL);
        u16 id;
        if (slot == 0xFF)
            return;
        // otId 0 with personality 0 would be shiny
        id = CreateMonPicSprite_HandleDeoxys(species, 0, 0xFFFF0000, TRUE, 222, 104, slot, TAG_NONE);
        if (id < MAX_SPRITES)
        {
            gSprites[id].oam.priority = 0;
            gSprites[id].oam.paletteNum = slot;
            gSprites[id].callback = SpriteCB_PickupPic;
            gSprites[id].coordOffsetEnabled = TRUE;
            sG->pickupPic = id;
            if (!SeenSpecies(species))
            {
                u8 i;
                for (i = 1; i < 16; i++)
                    gPlttBufferUnfaded[OBJ_PLTT_ID(slot) + i] = gPlttBufferFaded[OBJ_PLTT_ID(slot) + i] = RGB(3, 3, 6);
            }
        }
        else
            FreeSpritePaletteByTag(TAG_PICKUP_PAL);
    }
}

// L/R: another banner. The reel fills up with that banner's prizes
static void SwitchBanner(s8 dir)
{
    s32 c = CenterCell(), i;
    sBanner = (sBanner + BANNER_COUNT + dir) % BANNER_COUNT;
    PlaySE(SE_DEX_PAGE);
    LoadThemedPalettes();
    for (i = c - 8; i <= sG->genUpTo; i++)
        MakeTeaserAt(i, i - 1);
    DestroyAllLive();
    UpdateReel();
    ShowPickupPic();
    BeginNormalPaletteFade(PALETTES_BG, 0, 8, 0, sBanners[sBanner].accent ? sBanners[sBanner].accent : RGB(31, 31, 31));
    DrawHeader();
    BuildMenu();   // free pulls differ per banner
    DrawMenu();
}

// The help line under the menu: the odds (on PICKUP with friends online,
// taking turns with how to start a vote)
static void DrawOddsLine(void)
{
    u8 i;
    if (sBanner == BANNER_PICKUP && (sG->tagPage & 1) && gMpLocalId >= 0 && FriendsOnline() > 0 && VotedPickup() < 0)
    {
        DrawHelp("{SELECT} Vote to reroll this week's PICKUP");
        return;
    }
    {
        // ODDS%  N 52  U 28  R 13.5  SR 5  SSR 1.2  UR .25  EX .05
        char odds[96];
        int n = snprintf(odds, sizeof(odds), "ODDS%%");
        for (i = 0; i < R_COUNT; i++)
        {
            u16 v = sBanners[sBanner].per10k[i];   // 1/100 of a percent
            if (v >= 100 && v % 100 == 0)
                n += snprintf(odds + n, sizeof(odds) - n, " %s %d", sRarityShort[i], v / 100);
            else if (v >= 100 && v % 10 == 0)
                n += snprintf(odds + n, sizeof(odds) - n, " %s %d.%d", sRarityShort[i], v / 100, (v / 10) % 10);
            else if (v >= 100)
                n += snprintf(odds + n, sizeof(odds) - n, " %s %d.%02d", sRarityShort[i], v / 100, v % 100);
            else
                n += snprintf(odds + n, sizeof(odds) - n, " %s .%02d", sRarityShort[i], v);
        }
        DrawHelp(odds);
    }
}

static void DrawMenu(void)
{
    static const char *const sLabels[] = {
        [MENU_FREE10] = "FREE x10",
        [MENU_FREE1]  = "FREE x1",
        [MENU_TICKET] = "TICKET",
        [MENU_ONE]    = "PULL x1",
        [MENU_TEN]    = "PULL x10",
        [MENU_BACK]   = "BACK",
    };
    char info[80];
    u8 i, row = sG->menuRows[sG->cursor];
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    // Three columns of two
    for (i = 0; i < sG->menuCount; i++)
    {
        bool8 sel = i == sG->cursor;
        u8 r = sG->menuRows[i];
        int x = (i % 3) * 64 + 2, y = (i / 3) * 12 + 2;
        u8 fg = sel ? TC_GOLD : (r <= MENU_TICKET ? TC_RARITY + R_SR : TC_WHITE);
        if ((r == MENU_ONE && MP_Gatcha_Pulls() < 1) || (r == MENU_TEN && MP_Gatcha_Pulls() < 10))
            fg = sel ? TC_GOLD : TC_DIM;
        PrintAscii(WIN_MENU, FONT_NORMAL, x, y - 1, TC_PANEL, fg, sel ? ">" : "");
        PrintAscii(WIN_MENU, FONT_NORMAL, x + 9, y - 1, TC_PANEL, fg, sLabels[r]);
    }
    switch (row)
    {
    case MENU_FREE10: snprintf(info, sizeof(info), "Free 10-pull, one a week. Waiting: %d", (int)Free10Stock()); break;
    case MENU_FREE1:  snprintf(info, sizeof(info), "Free pull, one a day per banner. Waiting: %d", (int)Free1Stock(sBanner)); break;
    case MENU_TICKET: snprintf(info, sizeof(info), "TICKET: 1 pull on any banner. You have %d.", (int)MP_Gatcha_Tickets()); break;
    case MENU_ONE:    snprintf(info, sizeof(info), "Uses 1 pull."); break;
    case MENU_TEN:    snprintf(info, sizeof(info), "Uses 10 pulls. One RARE+ guaranteed!"); break;
    default:          snprintf(info, sizeof(info), "Back to the CO-OP menu."); break;
    }
    PrintAscii(WIN_MENU, FONT_SMALL, 8, 28, TC_PANEL, TC_GREY, info);
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawOddsLine();
    DrawBannerLine();
    CopyWindowToVram(WIN_NAME, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------
// Small tweaks (AllowSmallTweaks = 1 in moemon_coop.ini): type "gimme" on this
// screen for 999 pulls, "open" to choose the next pull

#define GS_FORCE GS_EXIT + 1

static void DrawForce(void)
{
    const struct Pool *pool = BannerPool(sG->forceRarity);
    char line[64];
    u8 name[48], buf[64];
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    PrintAscii(WIN_MENU, FONT_SMALL, 4, 1, TC_PANEL, TC_GREY, "NEXT PULL");
    snprintf(line, sizeof(line), "%c RARITY  < %s >", sG->forceRow == 0 ? '>' : ' ', sRarityNames[sG->forceRarity]);
    PrintAscii(WIN_MENU, FONT_NORMAL, 4, 10, TC_PANEL, sG->forceRow == 0 ? TC_GOLD : TC_RARITY + sG->forceRarity, line);
    if (sG->forcePrize < 0 || sG->forcePrize >= pool->count)
        MP_AsciiToGame(name, "ANY", 10);
    else
        PrizeName(&pool->defs[sG->forcePrize], pool->defs[sG->forcePrize].qty, name);
    snprintf(line, sizeof(line), "%c PRIZE  < ", sG->forceRow == 1 ? '>' : ' ');
    MP_AsciiToGame(buf, line, 30);
    StringAppend(buf, name);
    MP_AsciiToGame(buf + StringLength(buf), " >", 4);
    Print(WIN_MENU, GetStringWidth(FONT_NORMAL, buf, 0) > 24 * 8 - 8 ? FONT_SMALL : FONT_NORMAL, 4, 24, TC_PANEL, sG->forceRow == 1 ? TC_GOLD : TC_WHITE, buf);
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp("{DPAD} CHOOSE   {A} SET   {B} CANCEL");
}

static void UpdateForce(void)
{
    const struct Pool *pool = BannerPool(sG->forceRarity);
    if (JOY_NEW(DPAD_UP | DPAD_DOWN))
    {
        sG->forceRow ^= 1;
        PlaySE(SE_SELECT);
        DrawForce();
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        s8 d = JOY_NEW(DPAD_RIGHT) ? 1 : -1;
        if (sG->forceRow == 0)
        {
            sG->forceRarity = (sG->forceRarity + R_COUNT + d) % R_COUNT;
            sG->forcePrize = -1;
        }
        else
        {
            // -1 (ANY), 0 .. count-1
            sG->forcePrize += d;
            if (sG->forcePrize < -1)
                sG->forcePrize = pool->count - 1;
            if (sG->forcePrize >= pool->count)
                sG->forcePrize = -1;
        }
        PlaySE(SE_SELECT);
        DrawForce();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        sRig = sG->forceRarity;
        sRigDef = sG->forcePrize >= 0 && sG->forcePrize < pool->count ? &pool->defs[sG->forcePrize] : NULL;
        PlaySE(SE_SHOP);
        sG->state = GS_MENU;
        DrawMenu();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sG->state = GS_MENU;
        DrawMenu();
    }
}

// Reads the letters typed since the last frame (only while the menu is up)
static void CheckTypedCodes(void)
{
    int c;
    while ((c = Platform_PopLetter()) != 0)
    {
        u8 n = strlen(sG->typed);
        if (n == sizeof(sG->typed) - 1)
        {
            memmove(sG->typed, sG->typed + 1, n);
            n--;
        }
        sG->typed[n] = c;
        sG->typed[n + 1] = 0;
        if (!gMpConfig.smallTweaks)
            continue;
        n++;
        if (n >= 5 && memcmp(sG->typed + n - 5, "gimme", 5) == 0)
        {
            MP_SaveData_Set(PULLS_KEY, 999);
            PlaySE(SE_SHOP);
            sG->typed[0] = 0;
            DrawHeader();
            DrawMenu();
        }
        else if (n >= 4 && memcmp(sG->typed + n - 4, "open", 4) == 0)
        {
            PlaySE(SE_SELECT);
            sG->typed[0] = 0;
            sG->forceRow = 0;
            sG->forceRarity = R_UR;
            sG->forcePrize = -1;
            sG->state = GS_FORCE;
            DrawForce();
            return;
        }
    }
}

static void SetReelClip(bool8 on)
{
    if (on)
    {
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(REEL_LEFT, REEL_RIGHT));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(0, DISPLAY_HEIGHT));
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG3 | WINOUT_WIN01_CLR);
    }
    else
    {
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
    }
}

// ---------------------------------------------------------------------------
// Pulling

// The prizes are already in the bag/party and the pulls are spent: save the
// game right away so quitting without saving can't lose them
static void SaveAfterPull(void)
{
    u8 status;
    SaveMapView();
    PausePyramidChallenge();
    if (gDifferentSaveFile == TRUE)
    {
        status = TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
        gDifferentSaveFile = FALSE;
    }
    else
        status = TrySavingData(SAVE_NORMAL);
    MPNet_Log("gatcha: saved after the pull (status %d)", status);
    MP_Autosave_Saved();
}

// The game's random numbers start the same on every launch and move on once a frame, so the same buttons on the same
// frames (a macro, a save copied back) pulled the same prizes again, and a frame later others: the PC's timer in now
static void StirRandom(void)
{
    u64 t = SDL_GetPerformanceCounter();
    gRngValue ^= (u32)t ^ (u32)(t >> 32) ^ (u32)time(NULL) * 2654435761u;
    Random();
}

static bool8 IsLegend(const struct Result *r)
{
    return r->rarity >= R_UR && r->def->kind == K_MON;
}

// Friends online hear about a UR/EX
static void TellFriends(const struct Result *r)
{
    char line[80], me[16];
    u8 name[48];
    char prize[48];
    MP_GameToAscii(me, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    PrizeName(r->def, r->amount, name);
    MP_GameToAscii(prize, name, sizeof(prize) - 1);
    snprintf(line, sizeof(line), "%s pulled %s %s!", me, r->rarity == R_EX ? "an EX:" : "a UR:", prize);
    MP_Minigame_SendNews(line);
}

static void StartSpin(u8 count)
{
    u8 i, best = 0;
    bool8 gotRare = FALSE;
    s32 center = CenterCell(), prizeCell;

    sG->resultCount = count;
    StirRandom();
    for (i = 0; i < count; i++)
    {
        struct Result *r = &sG->results[i];
        bool8 lastChance = (count == 10 && i == count - 1 && !gotRare);
        r->rarity = RollRarity(lastChance);
        r->def = PickFromPool(r->rarity);
        if (i == 0 && sRigDef)
            r->def = sRigDef;
        sRigDef = NULL;
        if (r->def->kind == K_MON && IsRandomEv(r->def->id))
        {
            // A random Moemon: which one is decided now
            sRandDefs[i] = *r->def;
            sRandDefs[i].qty = RandomSpecies(r->def->id == EV_RANDOM_LEGEND);
            r->def = &sRandDefs[i];
        }
        else if (r->def->kind == K_FX && r->def->qty == 0)
        {
            // which ball effect piece: decided now (any of them: one already had becomes a TICKET)
            sRandDefs[i] = *r->def;
            sRandDefs[i].qty = MP_BallUnlock_PrizeCount() ? 1 + Random() % MP_BallUnlock_PrizeCount() : 0;
            r->def = &sRandDefs[i];
        }
        GivePrize(r);
        if (r->rarity >= R_R)
            gotRare = TRUE;
        if (r->rarity > sG->results[best].rarity)
            best = i;
    }
    sG->best = best;
    MPNet_Log("gatcha: %d pull(s), best rarity %d", count, sG->results[best].rarity);
    {
        char list[96];
        int at = 0;
        for (i = 0; i < count; i++)
            at += snprintf(list + at, sizeof(list) - at, " %s:%d", sRarityShort[sG->results[i].rarity],
                           (int)(sG->results[i].def - BannerPool(sG->results[i].rarity)->defs));
        MPNet_Log("gatcha: results%s", list);
    }
    // achievements: pulls made, and the rarest prize
    MP_Achieve_Count("pull", count);
    if (sG->results[best].rarity >= R_SR)
        MP_Achieve_Unlock("sr");
    if (sG->results[best].rarity >= R_SSR)
        MP_Achieve_Unlock("ssr");
    if (sG->results[best].rarity >= R_UR)
        MP_Achieve_Unlock("ur");
    SaveAfterPull();

    // The prize where the reel stops, and the cells next to it
    prizeCell = center + SPIN_CELLS;
    Cell(prizeCell)->def = sG->results[best].def;
    Cell(prizeCell)->rarity = Cell(prizeCell)->shown = sG->results[best].rarity;
    sFixedCount = 0;
    sFixedCells[sFixedCount++] = prizeCell;
    // So close! A legendary sits right next to it
    if (sG->results[best].rarity < R_UR)
    {
        Cell(prizeCell + 1)->def = PickVaried(prizeCell + 1, center, R_UR, NULL);
        Cell(prizeCell + 1)->rarity = Cell(prizeCell + 1)->shown = R_UR;
        sFixedCells[sFixedCount++] = prizeCell + 1;
        Cell(prizeCell - 1)->def = PickVaried(prizeCell - 1, center, R_SSR, NULL);
        Cell(prizeCell - 1)->rarity = Cell(prizeCell - 1)->shown = R_SSR;
        sFixedCells[sFixedCount++] = prizeCell - 1;
    }
    // SR and up: a "?" box that looks RARE... and doesn't stay that way
    sG->prizeCell = prizeCell;
    sG->mystery = sG->results[best].rarity >= R_SR;
    sG->nudge = FALSE;
    if (sG->mystery)
    {
        Cell(prizeCell)->shown = R_R;
        sG->nudge = sG->results[best].rarity >= R_SSR || (Random() & 1);
        if (sG->nudge)
        {
            // It seems to stop on a dud first
            Cell(prizeCell - 1)->def = PickVaried(prizeCell - 1, center, R_N, NULL);
            Cell(prizeCell - 1)->rarity = Cell(prizeCell - 1)->shown = R_N;
            if (sFixedCount < ARRAY_COUNT(sFixedCells) && sFixedCells[sFixedCount - 1] != prizeCell - 1)
                sFixedCells[sFixedCount++] = prizeCell - 1;
        }
    }
    // New teasers past the visible part of the reel (cells up to center + 6
    // may already have their icons, so they stay as they are)
    for (i = 7; i <= SPIN_CELLS + 8; i++)
    {
        u8 k;
        bool8 fixed = FALSE;
        for (k = 0; k < sFixedCount; k++)
            if (sFixedCells[k] == center + i)
                fixed = TRUE;
        if (!fixed)
            MakeTeaserAt(center + i, center + i - 1);
    }
    sFixedCount = 0;
    if (sG->pickupPic != 0xFFFF)
        gSprites[sG->pickupPic].invisible = TRUE;
    sG->genUpTo = prizeCell + 8;
    sG->spinFrom = sG->pos;
    sG->spinTo = (prizeCell - (sG->nudge ? 1 : 0)) * CELL_W;
    sG->timer = 0;
    sG->lastCenter = center;
    sG->state = GS_SPIN;
    SetReelClip(TRUE);
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_MENU, FONT_NORMAL, 12, TC_PANEL, TC_GREY, count == 10 ? "10-PULL!" : "Here it comes...");
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp("");
    DrawHeader();
}

static void StartHype(void)
{
    sG->state = GS_HYPE;
    sG->hypeTimer = 0;
    FadeOutBGM(4);
    CreateRays();
    RayColorsFor(R_R);
    PlaySE(SE_M_CHARGE);
}

// The "?" box turns into the next rarity. Each step is bigger than the last
static void Upgrade(u8 rarity)
{
    static const u16 sSounds[R_COUNT] = {0, 0, 0, SE_SHINY, SE_M_MORNING_SUN, SE_M_COSMIC_POWER, SE_M_SACRED_FIRE};
    static const u8 sShake[R_COUNT] = {0, 0, 0, 24, 34, 44, 56};
    LoadPalette(sCellPals[Cell(sG->prizeCell)->shown], BG_PLTT_ID(CELL_PAL0 + Cell(sG->prizeCell)->shown), sizeof(sCellPals[0]));
    Cell(sG->prizeCell)->shown = rarity;
    PlaySE(sSounds[rarity]);
    BeginNormalPaletteFade(PALETTES_ALL, 0, rarity >= R_UR ? 16 : 13, 0, sRarityFlash[rarity]);
    RayColorsFor(rarity);
    SpawnSparkles(8 + 4 * (rarity - R_SR), TRUE);
    DrawMysteryLine(rarity);
    sG->shake += sShake[rarity];
}

// When the box changes color: SR, SSR, UR. (EX comes later, with a crack.)
#define HYPE_SR   34
#define HYPE_SSR  76
#define HYPE_UR   118
#define HYPE_EX   170

static u16 HypeEnd(u8 best)
{
    switch (best)
    {
    case R_SR:  return 68;
    case R_SSR: return 112;
    case R_UR:  return 156;
    default:    return 206;
    }
}

static void StartEvent(void);

// Before the burst: the screen darkens, the box glows and climbs through the
// rarities, light rays spin up, sparkles rise, everything shakes harder
static void UpdateHype(void)
{
    struct Result *r = &sG->results[sG->best];
    u8 best = r->rarity;
    u16 t = ++sG->hypeTimer, end = HypeEnd(best);
    UpdateReel();
    SetDarken(t / 2 < 11 ? t / 2 : 11);
    PulseCell(Cell(sG->prizeCell)->shown, t);
    UpdateRays(0x180 + t * 10, 0x80 + (t * 3 < 0x100 ? t * 3 : 0x100));
    if (t % (best >= R_UR ? 2 : 3) == 0)
        SpawnSparkles(1, FALSE);
    if (sG->shake < 16 + 8 * (best - R_SR))
        sG->shake++;
    if (t == HYPE_SR)
        Upgrade(R_SR);
    if (best >= R_SSR && t == HYPE_SSR)
        Upgrade(R_SSR);
    if (best >= R_UR && t == HYPE_UR)
        Upgrade(R_UR);
    if (best >= R_UR && t > HYPE_UR)
        SetRayColors(MixColor(Hue(t * 6), RGB(31, 31, 31), 150), Hue(t * 6 + 96));
    // A legendary: the whole screen goes dark instead of bursting
    if (IsLegend(r) && t == HypeEnd(R_UR) - 6)
    {
        StartEvent();
        return;
    }
    if (best == R_EX)
    {
        // It seems to have settled on gold... then it cracks
        if (t == HYPE_EX - 24)
            PlaySE(SE_M_DETECT);
        if (t >= HYPE_EX - 12 && t < HYPE_EX)
            BeginNormalPaletteFade(PALETTES_ALL, 0, (t & 2) ? 16 : 0, 0, (t & 2) ? RGB_WHITE : RGB_BLACK);
        if (t == HYPE_EX)
        {
            PlaySE(SE_M_EXPLOSION);
            Upgrade(R_EX);
        }
        if (t > HYPE_EX)
            Bg3Rainbow(t * 8);
    }
    if (t == end - 10)
        PlaySE(SE_M_HYPER_BEAM);
    if (t >= end)
        ShowReveal();
}

// ---------------------------------------------------------------------------
// A legendary: darkness, a heartbeat, the silhouette rises to battle music

#define TAG_LEGEND_PAL 0x7245

static void HideReelSprites(bool8 hide)
{
    u8 slot;
    for (slot = 0; slot < 16; slot++)
        if (sG->live[slot].cell != NO_CELL && sG->live[slot].spriteId < MAX_SPRITES)
            gSprites[sG->live[slot].spriteId].invisible = hide;
    gSprites[sG->frameSprite].invisible = hide;
    gSprites[sG->arrowTop].invisible = hide;
}

static void DestroyLegendPic(void)
{
    if (sG->legendPic != 0xFFFF)
    {
        FreeAndDestroyMonPicSprite(sG->legendPic);
        FreeSpritePaletteByTag(TAG_LEGEND_PAL);
    }
    sG->legendPic = 0xFFFF;
}

static void SpriteCB_Legend(struct Sprite *sprite)
{
    // data: 0 age, 1 start y offset (rises from it)
    s16 t = ++sprite->data[0];
    s16 rise = sprite->data[1] - t;
    sprite->y2 = (rise > 0 ? rise : 0) + Sin((t * 2) & 0xFF, 3);
}

// The silhouette: the real colors are kept, every color is drawn dark
static void SetLegendColors(u8 glow, u16 glowColor)
{
    u8 i;
    u16 base = OBJ_PLTT_ID(sG->legendSlot);
    for (i = 1; i < 16; i++)
    {
        u16 c = MixColor(RGB(1, 0, 3), glowColor, glow);
        gPlttBufferUnfaded[base + i] = c;
        gPlttBufferFaded[base + i] = c;
    }
}

static void RestoreLegendColors(void)
{
    u16 base = OBJ_PLTT_ID(sG->legendSlot);
    memcpy(&gPlttBufferUnfaded[base], sG->legendPal, sizeof(sG->legendPal));
    memcpy(&gPlttBufferFaded[base], sG->legendPal, sizeof(sG->legendPal));
}

static void CreateLegendPic(s16 x, s16 y)
{
    struct Result *r = &sG->results[sG->best];
    u16 species = DefSpecies(r->def);
    // otId 0: personality 0 makes it shiny, 0xFFFF0000 doesn't
    u32 personality = r->def->shiny ? 0 : 0xFFFF0000;
    u8 slot;
    DestroyLegendPic();
    slot = AllocSpritePalette(TAG_LEGEND_PAL);
    if (slot == 0xFF)
        return;
    sG->legendSlot = slot;
    sG->legendPic = CreateMonPicSprite_HandleDeoxys(species, 0, personality, TRUE, x, y, slot, TAG_NONE);
    if (sG->legendPic >= MAX_SPRITES)
    {
        sG->legendPic = 0xFFFF;
        FreeSpritePaletteByTag(TAG_LEGEND_PAL);
        return;
    }
    memcpy(sG->legendPal, &gPlttBufferUnfaded[OBJ_PLTT_ID(slot)], sizeof(sG->legendPal));
    gSprites[sG->legendPic].oam.priority = 0;
    gSprites[sG->legendPic].oam.paletteNum = slot;
    gSprites[sG->legendPic].coordOffsetEnabled = TRUE;
    gSprites[sG->legendPic].callback = SpriteCB_Legend;
}

static void StartEvent(void)
{
    sG->state = GS_EVENT;
    sG->eventTimer = 0;
    DestroyRays();
    sG->shake = 0;
    m4aMPlayStop(&gMPlayInfo_BGM);
    PlaySE(SE_M_DETECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 16, RGB_BLACK);   // instant black
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG3 | BLDCNT_EFFECT_DARKEN);
    SetGpuReg(REG_OFFSET_BLDY, 16);
    HideReelSprites(TRUE);
}

#define EVENT_RISE   70
#define EVENT_GLOW   160
#define EVENT_END_UR 196
#define EVENT_END_EX 236

static void UpdateEvent(void)
{
    struct Result *r = &sG->results[sG->best];
    u16 e = ++sG->eventTimer;
    bool8 ex = r->rarity == R_EX;
    if (e == 8)
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);   // sprites come back (all dark anyway)
    if (e == 30 || e == 46)
        PlaySE(SE_M_BELLY_DRUM);   // a heartbeat
    if (e == EVENT_RISE)
    {
        CreateLegendPic(REEL_CX, 92);
        if (sG->legendPic != 0xFFFF)
        {
            gSprites[sG->legendPic].data[1] = 48;
            SetLegendColors(0, 0);
        }
        PlayBGM(ex ? MUS_RG_HALL_OF_FAME : MUS_HALL_OF_FAME);   // happy and triumphant
        PlaySE(SE_M_EARTHQUAKE);
        sG->shake = 30;
    }
    if (e > EVENT_RISE && sG->legendPic != 0xFFFF)
    {
        // A glow around the dark shape, stronger and stronger
        u8 glow = (e - EVENT_RISE) * 3 / 4;
        if (glow > 130)
            glow = 130;
        glow = glow / 2 + (glow / 2) * (Sin((e * 8) & 0xFF, 64) + 64) / 128;
        SetLegendColors(glow, ex ? Hue(e * 6) : sRarityFlash[R_UR]);
        if (e % 6 == 0)
            SpawnSparkles(1, FALSE);
    }
    if (e > EVENT_RISE && e < EVENT_RISE + 50 && sG->shake < 20)
        sG->shake = 20;   // the ground rumbles while it rises
    if (e == EVENT_GLOW)
        PlaySE(ex ? SE_M_SACRED_FIRE : SE_M_COSMIC_POWER);
    if (ex && e > EVENT_END_UR - 20 && e < EVENT_END_EX - 10)
    {
        // EX: the screen can't hold it together
        if (e % 4 == 0)
            BeginNormalPaletteFade(PALETTES_ALL, 0, (e & 4) ? 16 : 6, 0, (e & 4) ? RGB_WHITE : Hue(e * 12));
        if (e % 10 == 0)
            PlaySE(SE_M_THUNDER_WAVE);
        sG->shake = 60;
    }
    if (e == (ex ? EVENT_END_EX : EVENT_END_UR) - 12)
        PlaySE(SE_M_HYPER_BEAM);
    if (e >= (ex ? EVENT_END_EX : EVENT_END_UR))
        ShowReveal();
}

// After the showcase: the usual reveal panel
static void EndShowcase(void)
{
    struct Result *r = &sG->results[sG->best];
    sG->showcase = FALSE;
    DestroyLegendPic();
    StopBigRays();
    CreateRays();   // the usual beams behind the prize box
    sG->rayScale = 0x200;
    ShowBg(1);
    HideReelSprites(FALSE);
    PutWindowTilemap(WIN_TITLE);
    PutWindowTilemap(WIN_INFO);
    PutWindowTilemap(WIN_NAME);
    PutWindowTilemap(WIN_MENU);
    CopyBgTilemapBufferToVram(0);
    DrawNameLine(r->def, r->rarity, r->amount, TRUE);
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_MENU, FONT_SMALL, 6, TC_PANEL, TC_GREY, r->note[0] ? r->note : "It's yours!");
    if (sG->resultCount == 10)
        PrintCenteredAscii(WIN_MENU, FONT_SMALL, 20, TC_PANEL, TC_WHITE, "Your best pull of the ten!");
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp(sG->resultCount == 10 ? "{A} SEE ALL TEN" : "{A} OK");
}

// Back to normal after a big reveal
static void ClearHype(void)
{
    u8 i;
    if (sG->showcase)
        EndShowcase();
    StopBigRays();
    DestroyLegendPic();
    DestroyRays();
    DestroyStamp();
    sG->shake = 0;
    SetDarken(0);
    LoadThemedPalettes();
    for (i = 0; i < R_COUNT; i++)
        LoadPalette(sCellPals[i], BG_PLTT_ID(CELL_PAL0 + i), sizeof(sCellPals[i]));
    if (sG->hypeTimer || sG->eventTimer)
    {
        sG->hypeTimer = sG->eventTimer = 0;
        PlayBGM(MUS_GAME_CORNER);
    }
}

static void ShowReveal(void)
{
    struct Result *r = &sG->results[sG->best];
    static const u16 sFanfares[R_COUNT] = {0, MUS_LEVEL_UP, MUS_OBTAIN_ITEM, MUS_SLOTS_WIN, MUS_SLOTS_JACKPOT, MUS_OBTAIN_SYMBOL, MUS_OBTAIN_SYMBOL};
    static const u8 sConfetti[R_COUNT] = {0, 4, 10, 20, 30, 44, 60};
    static const u8 sFlashes[R_COUNT] = {0, 0, 0, 3, 4, 6, 10};
    bool8 legend = IsLegend(r);
    u8 i;

    // Friends online hear about every UR/EX of this pull, right as it shows
    for (i = 0; i < sG->resultCount; i++)
        if (sG->results[i].rarity >= R_UR)
            TellFriends(&sG->results[i]);
    SetReelClip(FALSE);
    if (sG->mystery)
    {
        // The burst: the "?" opens, white flash, a big shake and the stamp
        sG->mystery = FALSE;
        Cell(sG->prizeCell)->shown = r->rarity;
        for (i = 0; i < R_COUNT; i++)
            LoadPalette(sCellPals[i], BG_PLTT_ID(CELL_PAL0 + i), sizeof(sCellPals[i]));
        UpdateReel();
        SetDarken(0);
        sG->shake = r->rarity >= R_UR ? 96 : 72;
        if (sG->raySprite >= MAX_SPRITES)
        {
            CreateRays();
            sG->rayScale = 0x180;
        }
        CreateStamp(r->rarity);
        SpawnSparkles(r->rarity >= R_UR ? 30 : 20, TRUE);
        PlaySE(SE_M_EXPLOSION);
    }
    if (legend)
    {
        // The showcase: the legendary in full color, front and center
        sG->showcase = TRUE;
        if (sG->legendPic == 0xFFFF)
            CreateLegendPic(REEL_CX, 92);
        if (sG->legendPic != 0xFFFF)
        {
            RestoreLegendColors();
            gSprites[sG->legendPic].data[1] = 0;
            gSprites[sG->legendPic].invisible = FALSE;
        }
        HideBg(1);
        HideReelSprites(TRUE);
        ClearWindowTilemap(WIN_TITLE);
        ClearWindowTilemap(WIN_INFO);
        ClearWindowTilemap(WIN_NAME);
        ClearWindowTilemap(WIN_MENU);
        CopyBgTilemapBufferToVram(0);
        {
            u8 name[48];
            char a[48], line[96];
            PrizeName(r->def, r->amount, name);
            MP_GameToAscii(a, name, sizeof(a) - 1);
            snprintf(line, sizeof(line), "%s! %s", a, r->note[0] ? r->note : "");
            DrawHelp(line);
        }
        DestroyRays();
        StartBigRays(REEL_CX, 92);
        PlayCry_Normal(DefSpecies(r->def), 0);
    }
    else
    {
        DrawNameLine(r->def, r->rarity, r->amount, TRUE);
        FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
        PrintCenteredAscii(WIN_MENU, FONT_SMALL, 6, TC_PANEL, TC_GREY, r->note[0] ? r->note : (r->def->kind == K_MONEY ? "Ka-ching!" : "It's yours!"));
        if (sG->resultCount == 10)
            PrintCenteredAscii(WIN_MENU, FONT_SMALL, 20, TC_PANEL, TC_WHITE, "Your best pull of the ten!");
        CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
        DrawHelp(sG->resultCount == 10 ? "{A} SEE ALL TEN" : "{A} OK");
    }
    if (sFanfares[r->rarity])
        PlayFanfare(sFanfares[r->rarity]);
    else
        PlaySE(SE_SUCCESS);
    if (r->rarity >= R_SR)
        BeginNormalPaletteFade(PALETTES_ALL, 1, 16, 0, RGB_WHITE);   // blinding
    else
        BeginNormalPaletteFade(PALETTES_ALL, 0, 10, 0, sRarityFlash[r->rarity]);
    sG->flashes = sFlashes[r->rarity];
    SpawnConfetti(sConfetti[r->rarity]);
    sG->state = GS_REVEAL;
    sG->timer = 0;
    DrawHeader();
}

// 10-pull: every prize in a grid (two rows of five)
static void GridCellPos(u8 i, s16 *x, s16 *y)
{
    *x = 24 + (i % 5) * 40 + 16;
    *y = 32 + (i / 5) * 32 + 16;
}

static void DrawGridSelection(void)
{
    struct Result *r = &sG->results[sG->cursor];
    s16 x, y;
    u8 name[48];
    GridCellPos(sG->cursor, &x, &y);
    gSprites[sG->frameSprite].x = x;
    gSprites[sG->frameSprite].y = y;
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_MENU, FONT_SMALL, 6, TC_PANEL, TC_RARITY + r->rarity, sRarityNames[r->rarity]);
    PrizeName(r->def, r->amount, name);
    PrintCentered(WIN_MENU, FONT_NORMAL, 18, TC_PANEL, TC_WHITE, name);
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp(r->note[0] ? r->note : "{DPAD} LOOK AT EACH PRIZE      {A}{B} DONE");
}

static void ShowGrid(void)
{
    u8 i;
    DestroyAllLive();
    FreeMonIconPalettes();   // ten prizes may need ten item palettes
    ClearBg1();
    sBg1Hofs = 0;
    for (i = 0; i < 10; i++)
    {
        s16 x, y;
        GridCellPos(i, &x, &y);
        DrawCellTiles(sBg1Map, (x - 16) / 8, (y - 16) / 8, sG->results[i].rarity, FALSE);
        sG->gridIcons[i] = CreateIcon(sG->results[i].def, 16 + i, x, y, &sG->gridIconMon[i], FALSE);
    }
    CopyBgTilemapBufferToVram(1);
    gSprites[sG->arrowTop].invisible = TRUE;
    gSprites[sG->arrowBottom].invisible = TRUE;
    ClearWindowTilemap(WIN_NAME);
    CopyBgTilemapBufferToVram(0);
    sG->cursor = 0;
    DrawGridSelection();
    sG->state = GS_GRID;
}

static void HideGrid(void)
{
    u8 i;
    for (i = 0; i < 10; i++)
    {
        if (sG->gridIcons[i] >= MAX_SPRITES)
            continue;
        if (sG->gridIconMon[i])
            FreeAndDestroyMonIconSprite(&gSprites[sG->gridIcons[i]]);
        else
            DestroyItemIcon(sG->gridIcons[i], 16 + i);
        sG->gridIcons[i] = MAX_SPRITES;
    }
    ClearBg1();
    PutWindowTilemap(WIN_NAME);
    CopyBgTilemapBufferToVram(0);
    gSprites[sG->arrowTop].invisible = FALSE;
    gSprites[sG->frameSprite].x = REEL_CX;
    gSprites[sG->frameSprite].y = REEL_TOP + 16;
    // Force the reel to redraw everything
    {
        u8 s;
        for (s = 0; s < 16; s++)
            sG->live[s].cell = NO_CELL;
    }
}

static void BackToMenu(void)
{
    sG->state = GS_MENU;
    if (sG->pickupPic != 0xFFFF)
        gSprites[sG->pickupPic].invisible = FALSE;
    BuildMenu();
    DrawMenu();
    DrawHeader();
    SetReelClip(TRUE);
}

// ---------------------------------------------------------------------------
// Scene

static void VBlankCB_Gatcha(void)
{
    SetGpuReg(REG_OFFSET_BG0HOFS, (u16)(-sShakeX) & 511);
    SetGpuReg(REG_OFFSET_BG0VOFS, (u16)(-sShakeY) & 511);
    SetGpuReg(REG_OFFSET_BG1HOFS, (sBg1Hofs - sShakeX) & 511);
    SetGpuReg(REG_OFFSET_BG1VOFS, (u16)(-sShakeY) & 511);
    SetGpuReg(REG_OFFSET_BG3HOFS, (sBg3Hofs - sShakeX) & 511);
    SetGpuReg(REG_OFFSET_BG3VOFS, (u16)(-sShakeY) & 511);
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Gatcha(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

// ---------------------------------------------------------------------------
// PICKUP reroll vote: on the PICKUP banner, SELECT asks everyone online
// whether to reroll this week's PICKUP. Nobody picks the new one: if more
// than half say YES (the asker counts), a random different PICKUP is drawn
// and revealed to everyone, for the rest of the week; pity counts stay. Once a
// vote has passed there are no more that week. Friends who come online later
// get the result from whoever has it.


#define VOTE_FRAMES      (60 * 45)    // the asker waits this long for answers
#define VOTE_ASK_FRAMES  (60 * 40)    // a question not answered by then counts as a NO
#define VOTE_RETRY       (60 * 120)   // after a vote that didn't pass, before asking again

enum { GV_ASK = MPR_GATCHA_FIRST, GV_ANSWER, GV_RESULT, GV_STATE };
enum { VA_NO, VA_YES, VA_BUSY };

static struct
{
    // A vote we started
    bool8 active;
    u8 id;
    u8 asked;      // bit per player asked
    u8 answered;
    u8 yes;        // counting ours
    u8 total;      // everyone online when it started, us too
    u16 timer;
    s32 week;
    u16 retry;     // frames before we may ask again
    // A friend's question to us
    bool8 pending;
    bool8 shown;   // on screen (gatcha screen or the field prompt)
    u8 from;
    u8 askId;
    s32 askWeek;
    u16 askTimer;
    u8 taskId;
    u16 cursor;
    // Friends we've told about this week's vote
    bool8 told[MP_MAX_PLAYERS];
    bool8 changed;   // the PICKUP changed: the open screen catches up
} sVote = {.taskId = TASK_NONE};

static void PutS32(u8 *p, s32 v)
{
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}

static s32 GetS32(const u8 *p)
{
    return (s32)(p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24));
}

static void VoterName(u8 id, char *out)
{
    MP_GameToAscii(out, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
}

// The toast has two short lines: "HITMONLEE/HITMONCHAN" becomes "HITMONLEE & co."
static void ShortPickupLabel(u8 pickup, char *out, int size)
{
    if (sPickups[pickup].mons[0] == EV_RANDOM)
    {
        snprintf(out, size, "random SHINY!");
        return;
    }
    PickupLabel(pickup, out, size);
    if (strlen(out) > 18)
    {
        char *slash = strchr(out, '/');
        if (slash)
            snprintf(slash, size - (slash - out), " & co.");
    }
}

static void ShowVoteNote(const char *ascii)
{
    u8 text[96];
    MP_AsciiToGame(text, ascii, sizeof(text) - 1);
    MP_Toast(text);
}

// A vote's result, here and on every friend's game
static void ApplyVote(u8 pickup, bool8 passed, u8 yes, u8 total, s32 week)
{
    char label[48], line[96];
    ShortPickupLabel(pickup, label, sizeof(label));
    if (!passed)
    {
        snprintf(line, sizeof(line), "No PICKUP reroll\n(%d of %d said YES)", yes, total);
        MPNet_Log("gatcha: vote for pickup %d did not pass (%d/%d)", pickup, yes, total);
        ShowVoteNote(line);
        return;
    }
    if (week != MP_WeekNumber() || VotedPickup() >= 0 || pickup >= ARRAY_COUNT(sPickups))
    {
        MPNet_Log("gatcha: vote for pickup %d passed but not used here (week %d/%d, voted %d)", pickup, (int)week,
                  (int)MP_WeekNumber(), VotedPickup());
        return;
    }
    MP_SaveData_Set(VOTE_WEEK_KEY, week + 1);
    MP_SaveData_Set(VOTE_PICK_KEY, pickup);
    memset(sVote.told, 0, sizeof(sVote.told));
    sVote.changed = TRUE;
    MPNet_Log("gatcha: vote passed, this week's pickup is %d (%d/%d)", pickup, yes, total);
    snprintf(line, sizeof(line), "New PICKUP! (%d/%d YES)\nUR: %s", yes, total, label);
    ShowVoteNote(line);
}

static void FinishVote(bool8 cancelled)
{
    u8 msg[1 + 5 + 4], pickup = 0;
    bool8 passed = !cancelled && sVote.yes * 2 > sVote.total;
    if (passed)
    {
        // The surprise: any PICKUP but this week's
        u8 cur = ThisWeeksPickupIndex();
        pickup = Random() % (ARRAY_COUNT(sPickups) - 1);
        if (pickup >= cur)
            pickup++;
    }
    msg[0] = GV_RESULT;
    msg[1] = sVote.id;
    msg[2] = pickup;
    msg[3] = cancelled ? 2 : passed;
    msg[4] = sVote.yes;
    msg[5] = sVote.total;
    PutS32(msg + 6, sVote.week);
    MP_SendRelay(0xFF, msg, sizeof(msg));
    sVote.active = FALSE;
    if (cancelled)
    {
        MPNet_Log("gatcha: our vote was called off (a friend asked first)");
        return;
    }
    if (!passed)
        sVote.retry = VOTE_RETRY;
    ApplyVote(pickup, passed, sVote.yes, sVote.total, sVote.week);
}

static void StartVote(void)
{
    u8 msg[1 + 2 + 4], i;
    sVote.active = TRUE;
    sVote.id++;
    sVote.asked = sVote.answered = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present)
            sVote.asked |= 1 << i;
    sVote.yes = 1;
    sVote.total = 1 + FriendsOnline();
    sVote.timer = 0;
    sVote.week = MP_WeekNumber();
    msg[0] = GV_ASK;
    msg[1] = sVote.id;
    msg[2] = ThisWeeksPickupIndex();   // the one it would replace
    PutS32(msg + 3, sVote.week);
    MP_SendRelay(0xFF, msg, sizeof(msg));
    MPNet_Log("gatcha: reroll vote started, pickup now %d (%d online)", msg[2], sVote.total);
}

static void SendAnswer(u8 answer)
{
    u8 msg[3] = {GV_ANSWER, sVote.askId, answer};
    MP_SendRelay(sVote.from, msg, sizeof(msg));
    MPNet_Log("gatcha: answered player %d's vote: %s", sVote.from + 1, answer == VA_YES ? "YES" : answer == VA_NO ? "NO" : "BUSY");
    sVote.pending = FALSE;
    sVote.shown = FALSE;
}

// The friend's question in the field
static void CloseVotePrompt(void)
{
    if (sVote.taskId != TASK_NONE)
    {
        if (MpUi_IsOpen())
            MpUi_Close();
        DestroyTask(sVote.taskId);
        sVote.taskId = TASK_NONE;
    }
}

static void DrawVotePrompt(void)
{
    char line[64], name[16];
    VoterName(sVote.from, name);
    MpUi_Begin("PICKUP VOTE");
    snprintf(line, sizeof(line), "%s wants to reroll this week's PICKUP!", name);
    MpUi_TextAscii(0, line);
    MpUi_TextAscii(1, "The new one is a surprise...");
    MpUi_RowAscii(3, sVote.cursor == 0, "YES", NULL);
    MpUi_RowAscii(4, sVote.cursor == 1, "NO", NULL);
    MpUi_End();
    MpUi_HelpAscii("More than half say YES: a new one for everyone.");
}

static void Task_VotePrompt(u8 taskId)
{
    if (!sVote.pending)
    {
        CloseVotePrompt();
        return;
    }
    if (MpUi_MoveCursor(&sVote.cursor, 2))
        DrawVotePrompt();
    else if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        SendAnswer(JOY_NEW(A_BUTTON) && sVote.cursor == 0 ? VA_YES : VA_NO);
        CloseVotePrompt();
    }
}

void MP_Gatcha_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || from == gMpLocalId)
        return;
    switch (data[0])
    {
    case GV_ASK:
    {
        u8 msg[3] = {GV_ANSWER, len >= 2 ? data[1] : 0, VA_BUSY};
        if (len < 7)
            return;
        MPNet_Log("gatcha: player %d asks for a reroll vote", from + 1);
        if (sVote.active)
        {
            // Two at once: the lower player number keeps theirs
            if (gMpLocalId < from)
            {
                MP_SendRelay(from, msg, sizeof(msg));
                return;
            }
            FinishVote(TRUE);
        }
        if (sVote.pending || GetS32(data + 3) != MP_WeekNumber() || VotedPickup() >= 0)
        {
            if (!sVote.pending && VotedPickup() >= 0)
                msg[2] = VA_NO;   // this week's was chosen already
            MP_SendRelay(from, msg, sizeof(msg));
            return;
        }
        sVote.pending = TRUE;
        sVote.shown = FALSE;
        sVote.from = from;
        sVote.askId = data[1];
        sVote.askWeek = GetS32(data + 3);
        sVote.askTimer = 0;
        sVote.cursor = 0;
        break;
    }
    case GV_ANSWER:
        if (len < 3 || !sVote.active || data[1] != sVote.id || !(sVote.asked & (1 << from)) || (sVote.answered & (1 << from)))
            return;
        sVote.answered |= 1 << from;
        if (data[2] == VA_YES)
            sVote.yes++;
        MPNet_Log("gatcha: player %d voted %s (%d yes of %d)", from + 1, data[2] == VA_YES ? "YES" : "NO", sVote.yes, sVote.total);
        break;
    case GV_RESULT:
        if (len < 10)
            return;
        if (sVote.pending && sVote.from == from && sVote.askId == data[1])
        {
            sVote.pending = FALSE;
            sVote.shown = FALSE;
        }
        if (data[3] == 2)
        {
            MPNet_Log("gatcha: player %d's vote was called off", from + 1);
            return;
        }
        ApplyVote(data[2], data[3] == 1, data[4], data[5], GetS32(data + 6));
        break;
    case GV_STATE:
        // A friend's group already picked this week's PICKUP
        if (len >= 6 && VotedPickup() < 0 && GetS32(data + 2) == MP_WeekNumber() && data[1] < ARRAY_COUNT(sPickups))
        {
            char label[48], line[96];
            MP_SaveData_Set(VOTE_WEEK_KEY, MP_WeekNumber() + 1);
            MP_SaveData_Set(VOTE_PICK_KEY, data[1]);
            sVote.changed = TRUE;
            ShortPickupLabel(data[1], label, sizeof(label));
            MPNet_Log("gatcha: player %d says this week's pickup is %d", from + 1, data[1]);
            snprintf(line, sizeof(line), "Friends' new PICKUP!\nUR: %s", label);
            ShowVoteNote(line);
        }
        break;
    }
}

// Every frame while online
void MP_Gatcha_VoteUpdate(void)
{
    u8 i;
    if (sVote.retry)
        sVote.retry--;
    if (sVote.active)
    {
        u8 waiting = 0;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if ((sVote.asked & (1 << i)) && !(sVote.answered & (1 << i)) && gMpPlayers[i].present)
                waiting++;
        // Done once the answer can't change: a YES majority, too many NOs, or time's up
        if (sVote.yes * 2 > sVote.total || (sVote.yes + waiting) * 2 <= sVote.total || waiting == 0 || ++sVote.timer >= VOTE_FRAMES)
            FinishVote(FALSE);
    }
    if (sVote.pending)
    {
        if (!gMpPlayers[sVote.from].present)
        {
            sVote.pending = FALSE;
            CloseVotePrompt();
        }
        else if (++sVote.askTimer >= VOTE_ASK_FRAMES)
        {
            SendAnswer(VA_BUSY);
            CloseVotePrompt();
            if (sG && sG->state == GS_VOTE_ASKED)
                sVote.changed = TRUE;   // the screen goes back to its menu
        }
        else if (!sVote.shown && sG == NULL && sVote.taskId == TASK_NONE && MP_IsLocalFree() && !MpUi_IsOpen())
        {
            sVote.shown = TRUE;
            PlaySE(SE_PIN);
            MpUi_Open();
            sVote.taskId = CreateTask(Task_VotePrompt, 80);
            DrawVotePrompt();
        }
    }
    // Tell friends who come online about this week's vote
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (i == gMpLocalId || !gMpPlayers[i].present || !gMpPlayers[i].hasInfo)
        {
            sVote.told[i] = FALSE;
            continue;
        }
        if (!sVote.told[i] && VotedPickup() >= 0)
        {
            u8 msg[6];
            msg[0] = GV_STATE;
            msg[1] = VotedPickup();
            PutS32(msg + 2, MP_WeekNumber());
            MP_SendRelay(i, msg, sizeof(msg));
            sVote.told[i] = TRUE;
        }
    }
}

// Test builds (mp gatchagive EV SHINY): give one event Moemon, as a pull would
void MP_Gatcha_DebugGive(u8 ev, bool8 shiny)
{
    struct PrizeDef def = {K_MON, ev, 1, 1, shiny};
    struct Result res;
    if (ev >= ARRAY_COUNT(sEventMons) || IsRandomEv(ev))
        return;
    memset(&res, 0, sizeof(res));
    GiveEventMon(&def, &res);
    printf("[test] gatchagive %d: %s\n", ev, res.note);
}

// Test builds (mp gatchareel): the icons in view and the closest repeat
void MP_Gatcha_DebugReel(void)
{
    s32 c, i, j, closest = 99;
    if (sG == NULL)
    {
        printf("[test] gatcha reel: not open\n");
        return;
    }
    c = CenterCell();
    printf("[test] gatcha reel banner %d day %d:", sBanner, (int)StandardDay());
    for (i = c - 4; i <= c + 4; i++)
    {
        printf(" %X", Cell(i)->def ? IconKey(Cell(i)->def) : 0);
        for (j = i + 1; j <= c + 4; j++)
            if (Cell(i)->def && Cell(j)->def && IconKey(Cell(i)->def) == IconKey(Cell(j)->def) && j - i < closest)
                closest = j - i;
    }
    printf(" closest repeat %d\n", (int)closest);
}

// The reel shows another banner's (or another week's) prizes
static void RefillReel(void)
{
    s32 c = CenterCell(), i;
    for (i = c - 8; i <= sG->genUpTo; i++)
        MakeTeaserAt(i, i - 1);
    DestroyAllLive();
    UpdateReel();
}

// A line in the menu window, left of the PICKUP Moemon: the widest font that fits
static void PrintFitted(int y, u8 fg, const char *ascii)
{
    u8 buf[64], font = FONT_NORMAL;
    s32 w, room = 184;
    MP_AsciiToGame(buf, ascii, 60);
    if (GetStringWidth(font, buf, 0) > room)
        font = FONT_SMALL;
    if (GetStringWidth(font, buf, 0) > room)
        font = FONT_SMALL_NARROW;
    w = GetStringWidth(font, buf, 0);
    Print(WIN_MENU, font, w < room ? (room - w) / 2 : 0, y, TC_PANEL, fg, buf);
}

static void DrawVotePick(void)
{
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    PrintFitted(1, TC_GOLD, "REROLL THIS WEEK'S PICKUP?");
    PrintFitted(14, TC_WHITE, "Friends vote. The new one is a surprise!");
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp("{A} ASK EVERYONE ONLINE  {B} BACK");
}

static void DrawVoteAsked(void)
{
    char line[64], name[16];
    VoterName(sVote.from, name);
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    snprintf(line, sizeof(line), "%s: reroll this week's PICKUP?", name);
    PrintFitted(1, TC_GOLD, line);
    PrintFitted(14, TC_WHITE, "The new one is a surprise...");
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp("{A} YES  {B} NO   (more than half YES: it changes)");
}

// SELECT on the PICKUP banner
static void TryStartVotePick(void)
{
    const char *why = NULL;
    if (gMpLocalId < 0 || FriendsOnline() == 0)
        why = "A PICKUP reroll vote needs friends online.";
    else if (VotedPickup() >= 0)
        why = "This week's PICKUP was rerolled already.";
    else if (sVote.active || sVote.pending)
        why = "A vote is going on right now.";
    else if (sVote.retry)
        why = "Give it a little while before asking again.";
    if (why)
    {
        PlaySE(SE_FAILURE);
        DrawHelp(why);
        MPNet_Log("gatcha: no vote: %s", why);
        return;
    }
    PlaySE(SE_SELECT);
    sG->state = GS_VOTE_PICK;
    DrawVotePick();
}

static void HandleVotePick(void)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sG->state = GS_MENU;
        DrawMenu();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        sG->state = GS_MENU;
        DrawMenu();
        if (FriendsOnline() == 0 || VotedPickup() >= 0 || sVote.active || sVote.pending)
        {
            PlaySE(SE_FAILURE);
            DrawHelp("The vote can't start right now.");
            return;
        }
        PlaySE(SE_PIN);
        StartVote();
        DrawHelp("Asked everyone online! The result shows in a moment.");
    }
}

// Every frame on the menu: a friend's question, or a new PICKUP to show
static void CheckVoteOnMenu(void)
{
    if (sVote.pending && !sVote.shown)
    {
        sVote.shown = TRUE;
        PlaySE(SE_PIN);
        sG->state = GS_VOTE_ASKED;
        DrawVoteAsked();
        return;
    }
    if (sVote.changed)
    {
        sVote.changed = FALSE;
        SetupPickup();
        if (sBanner == BANNER_PICKUP)
        {
            RefillReel();
            sG->pickupShown = 0;
            ShowPickupPic();
        }
        DrawMenu();
    }
}

static void HandleVoteAsked(void)
{
    if (!sVote.pending)   // the vote ended (or the time ran out)
    {
        sG->state = GS_MENU;
        DrawMenu();
        return;
    }
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        SendAnswer(JOY_NEW(A_BUTTON) ? VA_YES : VA_NO);
        sG->state = GS_MENU;
        DrawMenu();
    }
}

static void FreeScene(void)
{
    sBigRays = FALSE;
    DestroyLegendPic();
    DestroyRays();
    DestroyStamp();
    DestroyPickupPic();
    sShakeX = sShakeY = 0;
    gSpriteCoordOffsetX = gSpriteCoordOffsetY = 0;
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    DestroyAllLive();
    FreeMonIconPalettes();
    FreeAllWindowBuffers();
    TRY_FREE_AND_SET_NULL(sBg1Map);
    TRY_FREE_AND_SET_NULL(sBg3Map);
    TRY_FREE_AND_SET_NULL(sG);
}

static void Task_ReopenCoopMenu(u8 taskId)
{
    if (IsWeatherNotFadingIn() == TRUE)
    {
        DestroyTask(taskId);
        UnlockPlayerFieldControls();
        MP_OpenMenuAtGatcha();
    }
}

static void FieldCB_ReturnFromGatcha(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_ReopenCoopMenu, 80);
}

// EX boxes and EX text shimmer through the rainbow, wherever they are
static void AnimateEx(void)
{
    u16 h = sG->bgScroll * 3, c = Hue(h);
    u16 base = BG_PLTT_ID(CELL_PAL0 + R_EX);
    if (gPaletteFade.active || (sG->state == GS_HYPE && Cell(sG->prizeCell)->shown == R_EX))
        return;
    gPlttBufferUnfaded[base + 1] = gPlttBufferFaded[base + 1] = MixColor(c, RGB(31, 31, 31), 140);
    gPlttBufferUnfaded[base + 2] = gPlttBufferFaded[base + 2] = MixColor(RGB(2, 2, 8), Hue(h + 96), 90);
    gPlttBufferUnfaded[base + 3] = gPlttBufferFaded[base + 3] = MixColor(RGB(2, 2, 8), Hue(h + 96), 150);
    gPlttBufferUnfaded[BG_PLTT_ID(TEXT_PAL) + TC_RARITY + R_EX] = gPlttBufferFaded[BG_PLTT_ID(TEXT_PAL) + TC_RARITY + R_EX] = MixColor(c, RGB(31, 31, 31), 100);
}

static void Task_Gatcha(u8 taskId)
{
    sG->bgScroll++;
    // Sideways only: the diagonal stripes still slide, the dark band stays put
    sBg3Hofs = sG->bgScroll / 2;
    // The screen shakes with the big reveals (sprites now, the backgrounds in VBlank)
    if (sG->shake > 0)
    {
        s16 amp = sG->shake / 16 + 1;
        sShakeX = (s16)(Random() % (2 * amp + 1)) - amp;
        sShakeY = (s16)(Random() % (2 * amp + 1)) - amp;
        if (sG->shake < 12)
            sShakeX = sShakeY = 0;
    }
    else
        sShakeX = sShakeY = 0;
    gSpriteCoordOffsetX = sShakeX;
    gSpriteCoordOffsetY = sShakeY;
    AnimateEx();

    switch (sG->state)
    {
    case GS_FADE_IN:
        UpdateReel();
        if (!gPaletteFade.active)
            sG->state = GS_MENU;
        break;
    case GS_MENU:
        sG->pos += 0.5f;   // idle drift
        UpdateReel();
        CheckTypedCodes();
        if (sG->state != GS_MENU)
            break;
        if (sBanner == BANNER_PICKUP && ThisWeeksPickup()->count > 1 && ++sG->pickupTimer >= 150)
        {
            sG->pickupTimer = 0;
            sG->pickupShown++;
            ShowPickupPic();
        }
        CheckVoteOnMenu();
        if (sG->state != GS_MENU)
            break;
        if ((sBanner == BANNER_STANDARD || sBanner == BANNER_PICKUP) && ++sG->tagTimer >= 150)
        {
            sG->tagTimer = 0;
            sG->tagPage++;
            if (sBanner == BANNER_STANDARD)
                DrawBannerLine();
            else
                DrawOddsLine();
        }
        if (JOY_NEW(L_BUTTON | R_BUTTON))
            SwitchBanner(JOY_NEW(R_BUTTON) ? 1 : -1);
        else if (JOY_NEW(SELECT_BUTTON) && sBanner == BANNER_PICKUP)
            TryStartVotePick();
        else if (JOY_NEW(DPAD_ANY))
        {
            // three columns of two
            u8 c = sG->cursor, col = c % 3, line = c / 3;
            if (JOY_NEW(DPAD_LEFT))
                col = (col + 2) % 3;
            else if (JOY_NEW(DPAD_RIGHT))
                col = (col + 1) % 3;
            else
                line ^= 1;
            c = line * 3 + col;
            if (c >= sG->menuCount)
                c = sG->menuCount - 1;
            if (c != sG->cursor)
            {
                sG->cursor = c;
                PlaySE(SE_SELECT);
                DrawMenu();
            }
        }
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sG->menuRows[sG->cursor] == MENU_BACK))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            FadeOutBGM(4);
            sG->state = GS_EXIT;
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 row = sG->menuRows[sG->cursor];
            if (row == MENU_FREE10 && sBanner == BANNER_STANDARD && Free10Stock() > 0)
            {
                MP_SaveData_Add(FREE10_KEY, -1);
                PlaySE(SE_SHOP);
                StartSpin(10);
            }
            else if (row == MENU_FREE1 && Free1Stock(sBanner) > 0)
            {
                AddFree1(sBanner, -1);
                PlaySE(SE_SHOP);
                StartSpin(1);
            }
            else if (row == MENU_TICKET && MP_Gatcha_Tickets() > 0)
            {
                MP_Gatcha_AddTickets(-1);
                PlaySE(SE_SHOP);
                StartSpin(1);
            }
            else if (row == MENU_ONE && MP_Gatcha_Pulls() >= 1)
            {
                MP_Gatcha_AddPulls(-1);
                PlaySE(SE_SHOP);
                StartSpin(1);
            }
            else if (row == MENU_TEN && MP_Gatcha_Pulls() >= 10)
            {
                MP_Gatcha_AddPulls(-10);
                PlaySE(SE_SHOP);
                StartSpin(10);
            }
            else
            {
                PlaySE(SE_FAILURE);
                DrawHelp(row == MENU_TEN ? "Not enough pulls for x10. Daily goals and achievements give more!" : "No pulls left. Daily goals and achievements give more!");
            }
        }
        break;
    case GS_SPIN:
    {
        float t = (float)++sG->timer / SPIN_FRAMES;
        s32 center;
        if (t > 1)
            t = 1;
        // Fast at first, then it slowly clicks down onto the prize
        sG->pos = sG->spinFrom + (sG->spinTo - sG->spinFrom) * (1 - (1 - t) * (1 - t) * (1 - t) * (0.5f + 0.5f * (1 - t)));
        UpdateReel();
        center = CenterCell();
        if (center != sG->lastCenter)
        {
            sG->lastCenter = center;
            if (sG->timer - sG->lastTickFrame >= 3)
            {
                PlaySE(SE_BALL_TRAY_BALL);
                sG->lastTickFrame = sG->timer;
            }
            if (sG->mystery && center == sG->prizeCell)
                DrawMysteryLine(Cell(center)->shown);
            else
                DrawNameLine(Cell(center)->def, Cell(center)->shown, Cell(center)->def->qty, FALSE);
        }
        if (sG->timer >= SPIN_FRAMES + (sG->nudge ? 0 : 20))
        {
            if (sG->nudge)
            {
                sG->state = GS_NUDGE;
                sG->timer = 0;
            }
            else if (sG->mystery)
                StartHype();
            else
                ShowReveal();
        }
        break;
    }
    case GS_NUDGE:
        // It stopped one short... a heartbeat... and clicks onto the next one
        sG->timer++;
        UpdateReel();
        if (sG->timer == 1 || sG->timer == 16)
            PlaySE(SE_M_DETECT);
        gSprites[sG->arrowTop].x2 = (sG->timer < 36) ? (s16)(Random() % 3) - 1 : 0;
        if (sG->timer >= 36)
        {
            u16 t = sG->timer - 36;
            float f = t >= 12 ? 1.0f : (float)t / 12;
            f = 1 - (1 - f) * (1 - f);
            sG->pos = (sG->prizeCell - 1) * CELL_W + CELL_W * f;
            if (t == 0)
                PlaySE(SE_BALL_TRAY_BALL);
            if (t == 12)
            {
                PlaySE(SE_M_SWORDS_DANCE);
                DrawMysteryLine(Cell(sG->prizeCell)->shown);
                StartHype();
            }
        }
        break;
    case GS_VOTE_PICK:
        sG->pos += 0.5f;
        UpdateReel();
        if (sG->pickupPic != 0xFFFF && ThisWeeksPickup()->count > 1 && ++sG->pickupTimer >= 150)
        {
            sG->pickupTimer = 0;
            sG->pickupShown++;
            ShowPickupPic();
        }
        HandleVotePick();
        break;
    case GS_VOTE_ASKED:
        sG->pos += 0.5f;
        UpdateReel();
        HandleVoteAsked();
        break;
    case GS_FORCE:
        sG->pos += 0.5f;
        UpdateReel();
        UpdateForce();
        break;
    case GS_HYPE:
        UpdateHype();
        break;
    case GS_EVENT:
        UpdateEvent();
        break;
    case GS_REVEAL:
    {
        u8 best = sG->results[sG->best].rarity;
        sG->timer++;
        if (sG->flashes && !gPaletteFade.active && sG->timer > 20)
        {
            sG->flashes--;
            BeginNormalPaletteFade(PALETTES_ALL, 0, 14, 0, best >= R_UR ? Hue(Random() % 192) : sRarityFlash[best]);
            SpawnConfetti(best >= R_SR ? 20 : 16);
            if (best >= R_UR && sG->flashes % 3 == 0)
                PlaySE(SE_M_SWORDS_DANCE);
        }
        if (best >= R_SR)
        {
            UpdateRays(best >= R_UR ? 0x500 : best == R_SSR ? 0x380 : 0x300, best >= R_UR ? 0x280 : 0x200);
            UpdateStamp();
            if (sG->shake > 0)
                sG->shake -= 2;
            if (sG->timer % (best >= R_UR ? 4 : 9) == 0)
                SpawnSparkles(3, TRUE);
            if (best == R_SSR)
                SetSprCol(TAG_STAMP, 4, MixColor(RGB(31, 25, 14), RGB(31, 31, 31), 128 + Sin((sG->timer * 8) & 0xFF, 120)));
            if (best >= R_UR)
            {
                // Rainbow everything, and stars raining down (EX: twice as many, and faster)
                u16 h = sG->timer * (best == R_EX ? 8 : 4);
                UpdateBigRays(best == R_EX ? 0x220 : 0x160, h);
                Bg3Rainbow(h);
                if (best == R_EX)
                {
                    SetSprCol(TAG_STAMP, 1, Hue(h));
                    SetSprCol(TAG_STAMP, 4, MixColor(Hue(h + 96), RGB(31, 31, 31), 170));
                }
                SetRayColors(MixColor(Hue(h + 64), RGB(31, 31, 31), 150), Hue(h + 128));
                if (sG->timer % (best == R_EX ? 2 : 4) == 0)
                    SpawnFallingStar();
            }
        }
        // A works even while the screen flashes (the flashes stop)
        if (sG->timer > 45 && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sG->flashes = 0;
            if (gPaletteFade.active)
            {
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0, RGB_BLACK);
                UpdatePaletteFade();
            }
            if (sG->showcase)
            {
                EndShowcase();
                sG->timer = 30;
                break;
            }
            ClearHype();
            if (sG->resultCount == 10)
                ShowGrid();
            else
                BackToMenu();
        }
        break;
    }
    case GS_GRID:
    {
        u8 c = sG->cursor;
        if (JOY_NEW(DPAD_LEFT)) c = (c % 5 == 0) ? c + 4 : c - 1;
        else if (JOY_NEW(DPAD_RIGHT)) c = (c % 5 == 4) ? c - 4 : c + 1;
        else if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN)) c = (c + 5) % 10;
        if (c != sG->cursor)
        {
            sG->cursor = c;
            PlaySE(SE_SELECT);
            DrawGridSelection();
        }
        else if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            HideGrid();
            sG->cursor = 0;
            BackToMenu();
        }
        break;
    }
    case GS_EXIT:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            FreeScene();
            SetGpuReg(REG_OFFSET_DISPCNT, 0);
            Overworld_ResetMapMusic();
            Overworld_PlaySpecialMapMusic();
            MP_FieldClearVBlankHBlankCallbacks();
            gFieldCallback = FieldCB_ReturnFromGatcha;
            SetMainCallback2(CB2_ReturnToField);
        }
        break;
    }
}

static u16 MixColor(u16 a, u16 b, int f)
{
    int r = (a & 31) + (((int)(b & 31)) - (int)(a & 31)) * f / 256;
    int g = ((a >> 5) & 31) + (((int)((b >> 5) & 31)) - (int)((a >> 5) & 31)) * f / 256;
    int bl = ((a >> 10) & 31) + (((int)((b >> 10) & 31)) - (int)((a >> 10) & 31)) * f / 256;
    return RGB(r, g, bl);
}

// Panels, stripes and highlights in the colors of the player's window frame
// (OPTIONS > FRAME), like the other co-op menus. Rarity colors stay as they are.
static void LoadThemedPalettes(void)
{
    u16 theme[16], text[16], bg3[5];
    u16 accent, white = RGB(31, 31, 31), gold;
    int mx, mn;
    MpTheme_BuildPalette(theme);
    accent = sBanners[sBanner].accent ? sBanners[sBanner].accent : theme[MPT_BORDER];
    mx = max(accent & 31, max((accent >> 5) & 31, (accent >> 10) & 31));
    mn = min(accent & 31, min((accent >> 5) & 31, (accent >> 10) & 31));
    gold = (mx - mn >= 6) ? MixColor(accent, white, 150) : RGB(31, 25, 6);
    memcpy(text, sTextPal, sizeof(text));
    text[TC_SHADOW] = MixColor(0, accent, 18);
    text[TC_GREY] = MixColor(RGB(21, 21, 23), accent, 40);
    text[TC_PANEL] = MixColor(0, accent, 58);
    text[TC_GOLD] = gold;
    text[TC_DIM] = MixColor(text[TC_PANEL], text[TC_GREY], 128);
    text[TC_BANNER] = MixColor(accent, white, 110);
    LoadPalette(text, BG_PLTT_ID(TEXT_PAL), PLTT_SIZE_4BPP);
    LoadPalette(text, BG_PLTT_ID(0), PLTT_SIZEOF(1));
    bg3[0] = 0;
    bg3[1] = MixColor(0, accent, 34);    // stripes
    bg3[2] = MixColor(0, accent, 50);
    bg3[3] = MixColor(0, accent, 16);    // band behind the reel
    bg3[4] = gold;                       // its trim
    LoadPalette(bg3, BG_PLTT_ID(BG3_PAL), sizeof(bg3));
    sSpritePalThemed[1] = gold;              // selector frame and arrow
}

static void CB2_InitGatcha(void)
{
    u8 i;
    struct SpriteTemplate frameT = {
        .tileTag = TAG_FRAME, .paletteTag = TAG_FRAME, .oam = &gOamData_AffineOff_ObjNormal_32x32,
        .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy,
    };
    struct SpriteTemplate arrowT = frameT;
    arrowT.tileTag = TAG_ARROW;
    arrowT.oam = &gOamData_AffineOff_ObjNormal_16x16;
    arrowT.callback = SpriteCB_Arrow;

    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, OAM, OAM_SIZE);
    DmaClear16(3, PLTT, PLTT_SIZE);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));
    for (i = 0; i < 4; i++)
    {
        ChangeBgX(i, 0, BG_COORD_SET);
        ChangeBgY(i, 0, BG_COORD_SET);
    }
    sG = AllocZeroed(sizeof(*sG));
    sG->raySprite = MAX_SPRITES;
    sG->stampSprites[0] = sG->stampSprites[1] = sG->stampSprites[2] = MAX_SPRITES;
    sG->pickupPic = 0xFFFF;
    sG->legendPic = 0xFFFF;
    sShakeX = sShakeY = 0;
    SetupStandard();
    SetupPickup();
    memset(sIconItem, 0, sizeof(sIconItem));
    sBg1Map = AllocZeroed(64 * 32 * 2);
    sBg3Map = AllocZeroed(32 * 32 * 2);
    SetBgTilemapBuffer(1, sBg1Map);
    SetBgTilemapBuffer(3, sBg3Map);
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    SetGpuReg(REG_OFFSET_BLDCNT, 0);

    LoadThemedPalettes();
    for (i = 0; i < R_COUNT; i++)
        LoadPalette(sCellPals[i], BG_PLTT_ID(CELL_PAL0 + i), sizeof(sCellPals[i]));
    LoadGeneratedGfx();
    BuildRayTiles();

    // Background: stripes, with a dark band and gold lines behind the reel
    {
        int x, y;
        for (y = 0; y < 32; y++)
            for (x = 0; x < 32; x++)
            {
                // The band runs from y=32 to 79: the boxes (y=42-70) sit in its middle,
                // with room above them for the selector arrow
                u16 tile = BG3_TILE;                                   // stripes
                if (y == REEL_TOP / 8 - 1)
                    tile = BG3_TILE + 2;                               // gold line, band
                else if (y >= REEL_TOP / 8 && y < REEL_TOP / 8 + 4)
                    tile = BG3_TILE + 1;                               // band
                else if (y == REEL_TOP / 8 + 4)
                    tile = BG3_TILE + 3;                               // band, gold line
                sBg3Map[y * 32 + x] = tile | (BG3_PAL << 12);
            }
        CopyBgTilemapBufferToVram(3);
    }

    for (i = 0; i < WIN_COUNT; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(i == WIN_TITLE || i == WIN_INFO ? 0 : TC_PANEL));
        PutWindowTilemap(i);
        CopyWindowToVram(i, COPYWIN_FULL);
    }
    CopyBgTilemapBufferToVram(0);

    for (i = 0; i < 16; i++)
        sG->live[i].cell = NO_CELL;
    for (i = 0; i < 10; i++)
        sG->gridIcons[i] = MAX_SPRITES;
    sG->genUpTo = -8;
    sG->pos = 0;
    GenerateAhead(12);

    sG->frameSprite = CreateSprite(&frameT, REEL_CX, REEL_TOP + 16, 0);
    gSprites[sG->frameSprite].oam.priority = 1;
    gSprites[sG->frameSprite].coordOffsetEnabled = TRUE;
    sG->arrowTop = CreateSprite(&arrowT, REEL_CX, REEL_TOP - 3, 0);
    gSprites[sG->arrowTop].vFlip = TRUE;
    gSprites[sG->arrowTop].coordOffsetEnabled = TRUE;
    gSprites[sG->arrowTop].data[0] = 1;   // bobs 1px: stays between the gold line and the boxes
    sG->arrowBottom = CreateSprite(&arrowT, REEL_CX, REEL_TOP + 38, 0);
    gSprites[sG->arrowBottom].invisible = TRUE;   // the name panel sits right under the reel

    MP_Gatcha_GrantFree();
    BuildMenu();
    DrawHeader();
    DrawMenu();
    UpdateReel();
    ShowPickupPic();
    SetReelClip(TRUE);

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP | DISPCNT_WIN0_ON);
    ShowBg(0);
    ShowBg(1);
    ShowBg(3);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    PlayBGM(MUS_GAME_CORNER);
    SetVBlankCallback(VBlankCB_Gatcha);
    SetMainCallback2(CB2_Gatcha);
    CreateTask(Task_Gatcha, 0);
}

// ---------------------------------------------------------------------------
// From the field (DAILY menu)

static void Task_EnterGatcha(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();   // (before destroying ourselves: it may destroy the task after us)
        SetMainCallback2(CB2_InitGatcha);
        DestroyTask(taskId);
    }
}

void MP_Gatcha_Open(void)
{
    LockPlayerFieldControls();
    FadeScreen(FADE_TO_BLACK, 0);
    CreateTask(Task_EnterGatcha, 80);
}
