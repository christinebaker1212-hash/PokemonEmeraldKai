// Moemon Co-op: the MoeGatcha. Shared achievements (and a few other rewards)
// give each player pulls, and everyone gets a free 10-pull every week.
//
// A full-screen slot reel: item icons, Moemon and money cards race past, then
// slowly click down onto the prize. Five rarities:
//   COMMON 55%  UNCOMMON 28%  RARE 12%  SUPER RARE 4.5%  ULTRA RARE 0.5%
// Pity: a SUPER RARE or better at least every 40 pulls, an ULTRA RARE at least
// every 120. A 10-pull always has at least one RARE or better.
// Event Moemon carry the moves of real Generation 3 event Pokémon.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "bg.h"
#include "decompress.h"
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
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "battle_pyramid.h"
#include "fieldmap.h"
#include "new_game.h"
#include "save.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/region_map_sections.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
u8 MP_GiveMonKeepOT(struct Pokemon *mon);
void MP_FieldClearVBlankHBlankCallbacks(void);
void MP_OpenMenuAtGatcha(void);
void MPNet_Log(const char *fmt, ...);

#define PULLS_KEY     "gatcha:pulls"
#define PITY_SR_KEY   "gatcha:pity"
#define PITY_UR_KEY   "gatcha:pityur"
#define FREE_WEEK_KEY "gatcha:freewk"   // week number + 1 of the last free 10-pull

#define SR_PITY 40
#define UR_PITY 120

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

enum { R_COMMON, R_UNCOMMON, R_RARE, R_SUPER, R_ULTRA, R_COUNT };

static const char *const sRarityNames[R_COUNT] = {"COMMON", "UNCOMMON", "RARE", "SUPER RARE", "ULTRA RARE"};

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

enum { EV_JIRACHI, EV_MEW, EV_CELEBI, EV_DEOXYS, EV_ZIGZAGOON, EV_PICHU, EV_SKITTY, EV_SWABLU, EV_LATIAS, EV_LATIOS, EV_LUGIA, EV_HOOH };

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
    [EV_LATIAS]    = {"SOUTHERN LATIAS", SPECIES_LATIAS, 50, {MOVE_MIST_BALL, MOVE_PSYCHIC, MOVE_RECOVER, MOVE_CHARM}, NULL, 0, FALSE},
    [EV_LATIOS]    = {"SOUTHERN LATIOS", SPECIES_LATIOS, 50, {MOVE_LUSTER_PURGE, MOVE_PSYCHIC, MOVE_RECOVER, MOVE_DRAGON_DANCE}, NULL, 0, FALSE},
    [EV_LUGIA]     = {"NAVEL ROCK LUGIA", SPECIES_LUGIA, 70, {MOVE_RECOVER, MOVE_HYDRO_PUMP, MOVE_RAIN_DANCE, MOVE_SWIFT}, NULL, 0, TRUE},
    [EV_HOOH]      = {"NAVEL ROCK HO-OH", SPECIES_HO_OH, 70, {MOVE_RECOVER, MOVE_FIRE_BLAST, MOVE_SUNNY_DAY, MOVE_SWIFT}, NULL, 0, TRUE},
};

enum { K_ITEM, K_MONEY, K_MON, K_SET };   // K_SET: several items at once (sSets)

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

enum { SET_VITAMINS, SET_BALLS };
static const u16 sSets[][6] = {
    [SET_VITAMINS] = {ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CALCIUM, ITEM_ZINC, ITEM_CARBOS},
    [SET_BALLS]    = {ITEM_NET_BALL, ITEM_DIVE_BALL, ITEM_NEST_BALL, ITEM_REPEAT_BALL, ITEM_TIMER_BALL, ITEM_LUXURY_BALL},
};
static const char *const sSetNames[] = {[SET_VITAMINS] = "VITAMIN SET", [SET_BALLS] = "BALL SET"};

static const struct PrizeDef sCommon[] = {
    ITEM(ITEM_POKE_BALL, 10, 10), ITEM(ITEM_GREAT_BALL, 5, 10), ITEM(ITEM_SUPER_POTION, 5, 10),
    ITEM(ITEM_HYPER_POTION, 3, 8), ITEM(ITEM_REVIVE, 2, 8), ITEM(ITEM_ETHER, 2, 6), ITEM(ITEM_FULL_HEAL, 3, 6),
    ITEM(ITEM_MAX_REPEL, 3, 6), MONEY(2000, 10), MONEY(3000, 8),
};
static const struct PrizeDef sUncommon[] = {
    ITEM(ITEM_ULTRA_BALL, 5, 10), ITEM(ITEM_RARE_CANDY, 1, 12), ITEM(ITEM_FULL_RESTORE, 2, 8),
    ITEM(ITEM_MAX_REVIVE, 1, 8), ITEM(ITEM_NUGGET, 1, 8), ITEM(ITEM_ELIXIR, 2, 6), ITEM(ITEM_TIMER_BALL, 5, 6),
    ITEM(ITEM_PROTEIN, 1, 4), ITEM(ITEM_IRON, 1, 4), ITEM(ITEM_CALCIUM, 1, 4), ITEM(ITEM_ZINC, 1, 4),
    ITEM(ITEM_CARBOS, 1, 4), ITEM(ITEM_HP_UP, 1, 4), MONEY(6000, 10),
};
static const struct PrizeDef sRare[] = {
    ITEM(ITEM_RARE_CANDY, 3, 12), ITEM(ITEM_PP_UP, 2, 8), ITEM(ITEM_MAX_ELIXIR, 2, 6),
    ITEM(ITEM_FIRE_STONE, 1, 3), ITEM(ITEM_WATER_STONE, 1, 3), ITEM(ITEM_THUNDER_STONE, 1, 3),
    ITEM(ITEM_LEAF_STONE, 1, 3), ITEM(ITEM_MOON_STONE, 1, 3), ITEM(ITEM_LEFTOVERS, 1, 4),
    ITEM(ITEM_LUCKY_EGG, 1, 4), ITEM(ITEM_KINGS_ROCK, 1, 3), ITEM(ITEM_CHOICE_BAND, 1, 3),
    ITEM(ITEM_SCOPE_LENS, 1, 3), ITEM(ITEM_SHELL_BELL, 1, 3), ITEM(ITEM_PROTEIN, 3, 2), ITEM(ITEM_CARBOS, 3, 2),
    MON(EV_ZIGZAGOON, 4), MON(EV_PICHU, 4), MON(EV_SKITTY, 4), MON(EV_SWABLU, 4), MONEY(15000, 8),
};
static const struct PrizeDef sSuper[] = {
    ITEM(ITEM_MASTER_BALL, 1, 12), ITEM(ITEM_PP_MAX, 2, 10), ITEM(ITEM_RARE_CANDY, 10, 12), MONEY(50000, 10),
    MON(EV_MEW, 8), MON(EV_CELEBI, 8),
    SHINY(EV_ZIGZAGOON, 3), SHINY(EV_PICHU, 3), SHINY(EV_SKITTY, 3), SHINY(EV_SWABLU, 3),
};
static const struct PrizeDef sUltra[] = {
    MON(EV_JIRACHI, 10), MON(EV_DEOXYS, 10), SHINY(EV_MEW, 4), SHINY(EV_CELEBI, 4),
    ITEM(ITEM_MASTER_BALL, 3, 6), MONEY(200000, 6),
};

// TRAINING banner: build your Moemon up
static const struct PrizeDef sTrainCommon[] = {
    ITEM(ITEM_RARE_CANDY, 1, 14), ITEM(ITEM_HP_UP, 1, 6), ITEM(ITEM_PROTEIN, 1, 6), ITEM(ITEM_IRON, 1, 6),
    ITEM(ITEM_CALCIUM, 1, 6), ITEM(ITEM_ZINC, 1, 6), ITEM(ITEM_CARBOS, 1, 6),
};
static const struct PrizeDef sTrainUncommon[] = {
    ITEM(ITEM_RARE_CANDY, 2, 14), ITEM(ITEM_HP_UP, 2, 5), ITEM(ITEM_PROTEIN, 2, 5), ITEM(ITEM_IRON, 2, 5),
    ITEM(ITEM_CALCIUM, 2, 5), ITEM(ITEM_ZINC, 2, 5), ITEM(ITEM_CARBOS, 2, 5), ITEM(ITEM_PP_UP, 1, 8),
};
static const struct PrizeDef sTrainRare[] = {
    ITEM(ITEM_RARE_CANDY, 5, 14), ITEM(ITEM_HP_UP, 5, 3), ITEM(ITEM_PROTEIN, 5, 3), ITEM(ITEM_IRON, 5, 3),
    ITEM(ITEM_CALCIUM, 5, 3), ITEM(ITEM_ZINC, 5, 3), ITEM(ITEM_CARBOS, 5, 3), ITEM(ITEM_PP_UP, 3, 8),
    ITEM(ITEM_LUCKY_EGG, 1, 5), ITEM(ITEM_MACHO_BRACE, 1, 5), ITEM(ITEM_EXP_SHARE, 1, 5),
};
static const struct PrizeDef sTrainSuper[] = {
    ITEM(ITEM_RARE_CANDY, 15, 14), ITEM(ITEM_PP_MAX, 3, 10), SET(SET_VITAMINS, 3, 12),
};
static const struct PrizeDef sTrainUltra[] = {
    ITEM(ITEM_RARE_CANDY, 50, 10), SET(SET_VITAMINS, 10, 10), ITEM(ITEM_PP_MAX, 10, 6),
};

// BALL banner: special and hard-to-find POKé BALLS
static const struct PrizeDef sBallCommon[] = {
    ITEM(ITEM_GREAT_BALL, 5, 10), ITEM(ITEM_NET_BALL, 3, 8), ITEM(ITEM_DIVE_BALL, 3, 8), ITEM(ITEM_NEST_BALL, 3, 8),
    ITEM(ITEM_REPEAT_BALL, 3, 8), ITEM(ITEM_TIMER_BALL, 3, 8), ITEM(ITEM_PREMIER_BALL, 3, 8), ITEM(ITEM_LUXURY_BALL, 1, 6),
};
static const struct PrizeDef sBallUncommon[] = {
    ITEM(ITEM_ULTRA_BALL, 5, 10), ITEM(ITEM_NET_BALL, 5, 6), ITEM(ITEM_DIVE_BALL, 5, 6), ITEM(ITEM_NEST_BALL, 5, 6),
    ITEM(ITEM_REPEAT_BALL, 5, 6), ITEM(ITEM_TIMER_BALL, 5, 6), ITEM(ITEM_LUXURY_BALL, 3, 8), ITEM(ITEM_PREMIER_BALL, 10, 6),
};
static const struct PrizeDef sBallRare[] = {
    ITEM(ITEM_ULTRA_BALL, 15, 10), ITEM(ITEM_LUXURY_BALL, 8, 8), ITEM(ITEM_NET_BALL, 10, 5), ITEM(ITEM_DIVE_BALL, 10, 5),
    ITEM(ITEM_NEST_BALL, 10, 5), ITEM(ITEM_REPEAT_BALL, 10, 5), ITEM(ITEM_TIMER_BALL, 10, 5), SET(SET_BALLS, 3, 8),
};
static const struct PrizeDef sBallSuper[] = {
    ITEM(ITEM_MASTER_BALL, 1, 14), SET(SET_BALLS, 10, 8), ITEM(ITEM_LUXURY_BALL, 30, 6),
};
static const struct PrizeDef sBallUltra[] = {
    ITEM(ITEM_MASTER_BALL, 3, 10), ITEM(ITEM_MASTER_BALL, 5, 4), SET(SET_BALLS, 30, 6),
};

// PICKUP banner: one featured event Moemon a week, filled in by SetupPickup()
static const u8 sPickupOrder[] = {EV_JIRACHI, EV_LATIAS, EV_MEW, EV_LUGIA, EV_CELEBI, EV_LATIOS, EV_HOOH};
static struct PrizeDef sPickupUltra[2];
static struct PrizeDef sPickupSuper[ARRAY_COUNT(sPickupOrder) - 1 + 4];
static const struct PrizeDef sPickupRare[] = {
    MON(EV_ZIGZAGOON, 6), MON(EV_PICHU, 6), MON(EV_SKITTY, 6), MON(EV_SWABLU, 6),
    ITEM(ITEM_RARE_CANDY, 3, 12), ITEM(ITEM_PP_UP, 2, 8), MONEY(15000, 8),
};

enum { BANNER_STANDARD, BANNER_PICKUP, BANNER_TRAINING, BANNER_BALLS, BANNER_COUNT };

struct Pool { const struct PrizeDef *defs; u8 count; };

struct BannerDef
{
    const char *name;
    const char *tagline;
    struct Pool pools[R_COUNT];
    u16 perMille[R_COUNT];   // chance of each rarity, out of 1000
    u16 accent;              // color of the stripes and panels
};

static struct BannerDef sBanners[BANNER_COUNT] = {
    [BANNER_STANDARD] = {
        "STANDARD", "Event Moemon, items and money!",
        {{sCommon, ARRAY_COUNT(sCommon)}, {sUncommon, ARRAY_COUNT(sUncommon)}, {sRare, ARRAY_COUNT(sRare)},
         {sSuper, ARRAY_COUNT(sSuper)}, {sUltra, ARRAY_COUNT(sUltra)}},
        {550, 280, 120, 45, 5}, 0,
    },
    [BANNER_PICKUP] = {
        "PICKUP", NULL,   // tagline names the featured Moemon
        {{sCommon, ARRAY_COUNT(sCommon)}, {sUncommon, ARRAY_COUNT(sUncommon)}, {sPickupRare, ARRAY_COUNT(sPickupRare)},
         {sPickupSuper, ARRAY_COUNT(sPickupSuper)}, {sPickupUltra, ARRAY_COUNT(sPickupUltra)}},
        {520, 280, 140, 50, 10}, RGB(31, 10, 20),
    },
    [BANNER_TRAINING] = {
        "TRAINING", "RARE CANDIES and vitamins!",
        {{sTrainCommon, ARRAY_COUNT(sTrainCommon)}, {sTrainUncommon, ARRAY_COUNT(sTrainUncommon)}, {sTrainRare, ARRAY_COUNT(sTrainRare)},
         {sTrainSuper, ARRAY_COUNT(sTrainSuper)}, {sTrainUltra, ARRAY_COUNT(sTrainUltra)}},
        {550, 280, 120, 45, 5}, RGB(31, 17, 2),
    },
    [BANNER_BALLS] = {
        "POKE BALLS", "Rare and special POK~ BALLS!",
        {{sBallCommon, ARRAY_COUNT(sBallCommon)}, {sBallUncommon, ARRAY_COUNT(sBallUncommon)}, {sBallRare, ARRAY_COUNT(sBallRare)},
         {sBallSuper, ARRAY_COUNT(sBallSuper)}, {sBallUltra, ARRAY_COUNT(sBallUltra)}},
        {550, 280, 120, 45, 5}, RGB(6, 18, 31),
    },
};

static u8 sBanner;   // the banner shown (kept between visits)

static u8 PickupFeatured(void)
{
    return sPickupOrder[MP_WeekNumber() % ARRAY_COUNT(sPickupOrder)];
}

// This week's PICKUP: the featured one as the ULTRA RARE (1 in 4 shiny), the
// other headliners as SUPER RAREs
static void SetupPickup(void)
{
    u8 feat = PickupFeatured(), i, n = 0;
    sPickupUltra[0] = (struct PrizeDef)MON(feat, 75);
    sPickupUltra[1] = (struct PrizeDef)SHINY(feat, 25);
    for (i = 0; i < ARRAY_COUNT(sPickupOrder); i++)
        if (sPickupOrder[i] != feat)
            sPickupSuper[n++] = (struct PrizeDef)MON(sPickupOrder[i], 6);
    sPickupSuper[n++] = (struct PrizeDef)SHINY(EV_ZIGZAGOON, 3);
    sPickupSuper[n++] = (struct PrizeDef)SHINY(EV_PICHU, 3);
    sPickupSuper[n++] = (struct PrizeDef)SHINY(EV_SKITTY, 3);
    sPickupSuper[n++] = (struct PrizeDef)SHINY(EV_SWABLU, 3);
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

static bool8 FreeTenAvailable(void)
{
    return MP_SaveData_Get(FREE_WEEK_KEY) != MP_WeekNumber() + 1;
}

static const struct PrizeDef *PickFromPool(u8 rarity)
{
    const struct Pool *pool = BannerPool(rarity);
    u16 total = 0, i, pick;
    for (i = 0; i < pool->count; i++)
        total += pool->defs[i].weight;
    pick = Random() % total;
    for (i = 0; i < pool->count; i++)
    {
        if (pick < pool->defs[i].weight)
            return &pool->defs[i];
        pick -= pool->defs[i].weight;
    }
    return &pool->defs[0];
}

// Each banner keeps its own pity count (STANDARD keeps the old keys)
static void PityKeys(char *sr, char *ur)
{
    if (sBanner == BANNER_STANDARD)
    {
        strcpy(sr, PITY_SR_KEY);
        strcpy(ur, PITY_UR_KEY);
    }
    else
    {
        sprintf(sr, "%s%d", PITY_SR_KEY, sBanner);
        sprintf(ur, "%s%d", PITY_UR_KEY, sBanner);
    }
}

static u8 RollRarity(bool8 rarePlus)
{
    char srKey[24], urKey[24];
    const u16 *pm = sBanners[sBanner].perMille;
    s32 sinceSuper, sinceUltra;
    u16 r = Random() % 1000;
    u8 rarity;

    PityKeys(srKey, urKey);
    sinceSuper = MP_SaveData_Get(srKey);
    sinceUltra = MP_SaveData_Get(urKey);
    rarity = r < pm[R_ULTRA] ? R_ULTRA
           : r < pm[R_ULTRA] + pm[R_SUPER] ? R_SUPER
           : r < pm[R_ULTRA] + pm[R_SUPER] + pm[R_RARE] ? R_RARE
           : r < pm[R_ULTRA] + pm[R_SUPER] + pm[R_RARE] + pm[R_UNCOMMON] ? R_UNCOMMON : R_COMMON;
    if (sinceUltra + 1 >= UR_PITY)
        rarity = R_ULTRA;
    else if (sinceSuper + 1 >= SR_PITY && rarity < R_SUPER)
        rarity = R_SUPER;
    if (rarePlus && rarity < R_RARE)
        rarity = R_RARE;
    MP_SaveData_Set(srKey, rarity >= R_SUPER ? 0 : sinceSuper + 1);
    MP_SaveData_Set(urKey, rarity == R_ULTRA ? 0 : sinceUltra + 1);
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
        snprintf(buf, sizeof(buf), "%s%s", def->shiny ? "SHINY " : "", sEventMons[def->id].title);
        MP_AsciiToGame(dst, buf, 30);
    }
    else if (def->kind == K_SET)
    {
        snprintf(buf, sizeof(buf), "%s x%u", sSetNames[def->id], (unsigned)def->qty);
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
    struct Pokemon *mon = Alloc(sizeof(struct Pokemon));
    u32 otId = e->otName ? e->otId : (gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
              | (gSaveBlock2Ptr->playerTrainerId[2] << 16) | ((u32)gSaveBlock2Ptr->playerTrainerId[3] << 24));
    u8 i, result;

    if (def->shiny)
        CreateMon(mon, e->species, e->level, USE_RANDOM_IVS, TRUE, PersonalityForShiny(otId), OT_ID_PRESET, otId);
    else
        CreateMon(mon, e->species, e->level, USE_RANDOM_IVS, FALSE, 0, e->otName ? OT_ID_PRESET : OT_ID_PLAYER_ID, e->otId);
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
    result = e->otName ? MP_GiveMonKeepOT(mon) : GiveMonToPlayer(mon);
    if (result == MON_CANT_GIVE)
    {
        AddMoney(&gSaveBlock1Ptr->money, 10000);
        snprintf(res->note, sizeof(res->note), "No room for it: $10000 instead.");
    }
    else
    {
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(e->species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(e->species), FLAG_SET_CAUGHT);
        snprintf(res->note, sizeof(res->note), result == MON_GIVEN_TO_PARTY ? "It joined your party!" : "It was sent to your PC.");
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
#define BG3_PAL      6
#define CELL_PAL0    1          // 1-5: one per rarity
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

// Text palette: 1 white, 2 shadow, 3 grey, 4-8 the rarities, 9 panel, 10 gold, 11 dim
static const u16 sTextPal[16] = {
    0, RGB(31, 31, 31), RGB(3, 2, 8), RGB(21, 21, 25),
    RGB(23, 23, 25), RGB(12, 30, 12), RGB(12, 22, 31), RGB(28, 14, 31), RGB(31, 28, 6),
    RGB(4, 3, 11), RGB(31, 25, 6), RGB(13, 13, 19), 0, 0, 0, 0,
};
enum { TC_WHITE = 1, TC_SHADOW, TC_GREY, TC_RARITY, TC_PANEL = 9, TC_GOLD, TC_DIM };

// Cells: 1 border, 2 fill, 3 top shine, 4 bottom shade
static const u16 sCellPals[R_COUNT][5] = {
    {0, RGB(25, 25, 27), RGB(11, 11, 14), RGB(15, 15, 18), RGB(7, 7, 9)},
    {0, RGB(14, 31, 14), RGB(3, 13, 5), RGB(5, 17, 7), RGB(1, 8, 2)},
    {0, RGB(15, 25, 31), RGB(3, 7, 19), RGB(5, 10, 24), RGB(1, 4, 12)},
    {0, RGB(29, 17, 31), RGB(11, 2, 17), RGB(15, 4, 22), RGB(6, 1, 10)},
    {0, RGB(31, 30, 14), RGB(21, 12, 0), RGB(27, 18, 2), RGB(13, 7, 0)},
};
static const u16 sRarityFlash[R_COUNT] = {RGB(24, 24, 26), RGB(14, 31, 14), RGB(14, 24, 31), RGB(29, 16, 31), RGB(31, 29, 10)};

// Background: 1-2 stripes, 3 band, 4 gold line

static u16 sSpritePalThemed[16] = {
    0, RGB(31, 27, 8), RGB(31, 31, 31), RGB(31, 12, 18), RGB(10, 26, 31), RGB(12, 30, 12), RGB(28, 14, 31), RGB(31, 20, 4),
    0, 0, 0, 0, 0, 0, 0, 0,
};

enum { GS_FADE_IN, GS_MENU, GS_SPIN, GS_REVEAL, GS_GRID, GS_EXIT };

#define NO_CELL (-0x7FFFFFFF)   // a live icon slot that shows nothing (cells can be negative)

struct LiveIcon
{
    s32 cell;          // NO_CELL: none
    u8 spriteId;
    bool8 isMon;
};

struct ReelCell
{
    const struct PrizeDef *def;
    u8 rarity;
};

struct GatchaState
{
    u8 state;
    u8 cursor;
    u8 menuRows[4];
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
};

static EWRAM_DATA struct GatchaState *sG = NULL;
// The reel's BG scroll is set in VBlank, together with the sprites (OAM), so
// the boxes and the prizes in them always move on the same frame
static u16 sBg1Hofs;
static EWRAM_DATA u16 *sBg1Map = NULL;
static EWRAM_DATA u16 *sBg3Map = NULL;

enum { MENU_FREE, MENU_ONE, MENU_TEN, MENU_BACK };

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

static void LoadGeneratedGfx(void)
{
    // The tile copies are queued for VBlank, so the buffers must stay around
    static u8 sCellTiles[32 * 16], sBgTiles[32 * 4];
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

    // Background: diagonal stripes, the dark band behind the reel, gold lines
    buf = sBgTiles;
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
        StartSpriteAnim(&gSprites[id], i % 4);
    }
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

// Teasers: lots of flashy prizes going by
static void MakeTeaser(struct ReelCell *c)
{
    u16 r = Random() % 100;
    c->rarity = r < 14 ? R_ULTRA : r < 34 ? R_SUPER : r < 56 ? R_RARE : r < 78 ? R_UNCOMMON : R_COMMON;
    c->def = PickFromPool(c->rarity);
}

static void GenerateAhead(s32 upTo)
{
    while (sG->genUpTo < upTo)
    {
        sG->genUpTo++;
        MakeTeaser(Cell(sG->genUpTo));
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

static u8 CreateIcon(const struct PrizeDef *def, u8 slot, s16 x, s16 y, bool8 *isMon)
{
    u8 id;
    if (def->kind == K_MON)
    {
        const struct EventMon *e = &sEventMons[def->id];
        LoadMonIconPalette(e->species);   // only the palettes in use: item icons need theirs too
        if (IndexOfSpritePaletteTag(gMonIconPaletteTable[gMonIconPaletteIndices[e->species]].tag) == 0xFF)
            return MAX_SPRITES;
        id = CreateMonIcon(e->species, SpriteCB_MonIcon, x, y - 4, 1, 0, FALSE);   // FALSE: Kai has no DEOXYS form icons
        *isMon = TRUE;
    }
    else
    {
        *isMon = FALSE;
        u16 item = def->kind == K_SET ? sSets[def->id][0] : def->id;   // a set shows its first item
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
        DrawCellTiles(sBg1Map, i * 4, REEL_TOP / 8, Cell(i)->rarity, TRUE);
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
        slot = ((i % 16) + 16) % 16;
        l = &sG->live[slot];
        if (l->cell != i)
        {
            DestroyLive(l, slot);
            if (Cell(i)->def == NULL)   // never filled in: give it a prize to show
                MakeTeaser(Cell(i));
            l->spriteId = CreateIcon(Cell(i)->def, slot, x, REEL_TOP + 16, &l->isMon);
            l->cell = i;
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
    char line[64];
    s32 sinceSuper = MP_SaveData_Get(PITY_SR_KEY), sinceUltra = MP_SaveData_Get(PITY_UR_KEY);

    FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(0));
    PrintAscii(WIN_TITLE, FONT_NORMAL, 2, 1, 0, TC_GOLD, "MOEGATCHA");
    snprintf(line, sizeof(line), "PULLS: %d", (int)MP_Gatcha_Pulls());
    {
        u8 buf[32];
        MP_AsciiToGame(buf, line, 30);
        Print(WIN_TITLE, FONT_NORMAL, 28 * 8 - 4 - GetStringWidth(FONT_NORMAL, buf, 0), 1, 0, TC_WHITE, buf);
    }
    CopyWindowToVram(WIN_TITLE, COPYWIN_FULL);

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
    // Pity: how many pulls at most until the next SUPER RARE+ / ULTRA RARE
    snprintf(line, sizeof(line), "SUPER RARE+ within %d", (int)(SR_PITY - sinceSuper));
    PrintAscii(WIN_INFO, FONT_SMALL, 2, 1, 0, TC_RARITY + R_SUPER, line);
    snprintf(line, sizeof(line), "ULTRA RARE within %d", (int)(UR_PITY - sinceUltra));
    {
        u8 buf[40];
        MP_AsciiToGame(buf, line, 38);
        Print(WIN_INFO, FONT_SMALL, 28 * 8 - 2 - GetStringWidth(FONT_SMALL, buf, 0), 1, 0, TC_RARITY + R_ULTRA, buf);
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
        buf[len - 1] = EOS;
    Print(WIN_HELP, font, 4, 2, TC_PANEL, TC_GREY, buf);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
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
    if (FreeTenAvailable())
        sG->menuRows[sG->menuCount++] = MENU_FREE;
    sG->menuRows[sG->menuCount++] = MENU_ONE;
    sG->menuRows[sG->menuCount++] = MENU_TEN;
    sG->menuRows[sG->menuCount++] = MENU_BACK;
    if (sG->cursor >= sG->menuCount)
        sG->cursor = 0;
}

static void DrawMenu(void)
{
    static const char *const sLabels[] = {
        [MENU_FREE] = "FREE x10!",
        [MENU_ONE]  = "PULL x1",
        [MENU_TEN]  = "PULL x10",
        [MENU_BACK] = "BACK",
    };
    static const char *const sInfo[] = {
        [MENU_FREE] = "Free this week! One RARE+ for sure.",
        [MENU_ONE]  = "Uses 1 pull.",
        [MENU_TEN]  = "Uses 10 pulls. One RARE+ guaranteed!",
        [MENU_BACK] = "Back to the CO-OP menu.",
    };
    u8 i;
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    // Two columns of two
    for (i = 0; i < sG->menuCount; i++)
    {
        bool8 sel = i == sG->cursor;
        u8 row = sG->menuRows[i];
        int x = (i % 2) * 96 + 8, y = (i / 2) * 12 + 2;
        u8 fg = sel ? TC_GOLD : (row == MENU_FREE ? TC_RARITY + R_SUPER : TC_WHITE);
        if ((row == MENU_ONE && MP_Gatcha_Pulls() < 1) || (row == MENU_TEN && MP_Gatcha_Pulls() < 10))
            fg = sel ? TC_GOLD : TC_DIM;
        PrintAscii(WIN_MENU, FONT_NORMAL, x, y - 1, TC_PANEL, fg, sel ? ">" : "");
        PrintAscii(WIN_MENU, FONT_NORMAL, x + 10, y - 1, TC_PANEL, fg, sLabels[row]);
    }
    PrintAscii(WIN_MENU, FONT_SMALL, 8, 28, TC_PANEL, TC_GREY, sInfo[sG->menuRows[sG->cursor]]);
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp("ODDS  C 55%  U 28%  R 12%  SR 4.5%  UR 0.5%");
    FillWindowPixelBuffer(WIN_NAME, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_NAME, FONT_SMALL, 2, TC_PANEL, TC_GREY, "Event Moemon, items and money!");
    PrintCenteredAscii(WIN_NAME, FONT_NORMAL, 14, TC_PANEL, TC_WHITE, MP_Gatcha_Pulls() > 0 || FreeTenAvailable() ? "Feeling lucky?" : "Earn pulls with team achievements!");
    CopyWindowToVram(WIN_NAME, COPYWIN_FULL);
}

static void SetReelClip(bool8 on)
{
    if (on)
    {
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(REEL_LEFT, REEL_RIGHT));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(0, DISPLAY_HEIGHT));
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG3);
    }
    else
    {
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ);
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
}

static void StartSpin(u8 count)
{
    u8 i, best = 0;
    bool8 gotRare = FALSE;
    s32 center = CenterCell(), prizeCell;

    sG->resultCount = count;
    for (i = 0; i < count; i++)
    {
        struct Result *r = &sG->results[i];
        bool8 lastChance = (count == 10 && i == count - 1 && !gotRare);
        r->rarity = RollRarity(lastChance);
        r->def = PickFromPool(r->rarity);
        GivePrize(r);
        if (r->rarity >= R_RARE)
            gotRare = TRUE;
        if (r->rarity > sG->results[best].rarity)
            best = i;
    }
    sG->best = best;
    MPNet_Log("gatcha: %d pull(s), best rarity %d", count, sG->results[best].rarity);
    SaveAfterPull();

    // New cells past the visible part of the reel, and the prize where it stops
    prizeCell = center + SPIN_CELLS;
    for (i = 5; i <= SPIN_CELLS + 8; i++)   // up to genUpTo below: those count as filled in
        MakeTeaser(Cell(center + i));
    Cell(prizeCell)->def = sG->results[best].def;
    Cell(prizeCell)->rarity = sG->results[best].rarity;
    // So close! The big one sits right next to it
    if (sG->results[best].rarity < R_ULTRA)
    {
        Cell(prizeCell + 1)->rarity = R_ULTRA;
        Cell(prizeCell + 1)->def = PickFromPool(R_ULTRA);
        Cell(prizeCell - 1)->rarity = R_SUPER;
        Cell(prizeCell - 1)->def = PickFromPool(R_SUPER);
    }
    sG->genUpTo = prizeCell + 8;
    sG->spinFrom = sG->pos;
    sG->spinTo = prizeCell * CELL_W;
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

static void ShowReveal(void)
{
    struct Result *r = &sG->results[sG->best];
    static const u16 sFanfares[R_COUNT] = {0, MUS_LEVEL_UP, MUS_OBTAIN_ITEM, MUS_SLOTS_WIN, MUS_SLOTS_JACKPOT};
    static const u8 sConfetti[R_COUNT] = {0, 4, 10, 24, 48};

    SetReelClip(FALSE);
    DrawNameLine(r->def, r->rarity, r->amount, TRUE);
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(TC_PANEL));
    PrintCenteredAscii(WIN_MENU, FONT_SMALL, 6, TC_PANEL, TC_GREY, r->note[0] ? r->note : (r->def->kind == K_MONEY ? "Ka-ching!" : "It's yours!"));
    if (sG->resultCount == 10)
        PrintCenteredAscii(WIN_MENU, FONT_SMALL, 20, TC_PANEL, TC_WHITE, "Your best pull of the ten!");
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    DrawHelp(sG->resultCount == 10 ? "A: SEE ALL TEN" : "A: OK");
    if (sFanfares[r->rarity])
        PlayFanfare(sFanfares[r->rarity]);
    else
        PlaySE(SE_SUCCESS);
    BeginNormalPaletteFade(PALETTES_ALL, 0, r->rarity >= R_SUPER ? 16 : 10, 0, sRarityFlash[r->rarity]);
    sG->flashes = r->rarity == R_ULTRA ? 3 : r->rarity == R_SUPER ? 1 : 0;
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
    DrawHelp(r->note[0] ? r->note : "D-PAD: LOOK AT EACH PRIZE      A/B: DONE");
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
        sG->gridIcons[i] = CreateIcon(sG->results[i].def, 16 + i, x, y, &sG->gridIconMon[i]);
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
    BuildMenu();
    DrawMenu();
    DrawHeader();
    SetReelClip(TRUE);
}

// ---------------------------------------------------------------------------
// Scene

static void VBlankCB_Gatcha(void)
{
    SetGpuReg(REG_OFFSET_BG1HOFS, sBg1Hofs);
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

static void FreeScene(void)
{
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

static void Task_Gatcha(u8 taskId)
{
    sG->bgScroll++;
    // Sideways only: the diagonal stripes still slide, the dark band stays put
    SetGpuReg(REG_OFFSET_BG3HOFS, sG->bgScroll / 2);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);

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
        if (JOY_NEW(DPAD_ANY))
        {
            u8 c = sG->cursor;
            if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
                c ^= 1;
            else
                c ^= 2;
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
            if (row == MENU_FREE)
            {
                MP_SaveData_Set(FREE_WEEK_KEY, MP_WeekNumber() + 1);
                PlaySE(SE_SHOP);
                StartSpin(10);
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
                DrawHelp(row == MENU_TEN ? "Not enough pulls for x10. Team achievements give more!" : "No pulls left. Team achievements give more!");
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
            DrawNameLine(Cell(center)->def, Cell(center)->rarity, Cell(center)->def->qty, FALSE);
        }
        if (sG->timer >= SPIN_FRAMES + 20)
            ShowReveal();
        break;
    }
    case GS_REVEAL:
        sG->timer++;
        if (sG->flashes && !gPaletteFade.active && sG->timer > 20)
        {
            sG->flashes--;
            BeginNormalPaletteFade(PALETTES_ALL, 0, 14, 0, sRarityFlash[sG->results[sG->best].rarity]);
            SpawnConfetti(16);
        }
        if (sG->timer > 45 && !gPaletteFade.active && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (sG->resultCount == 10)
                ShowGrid();
            else
                BackToMenu();
        }
        break;
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
    accent = theme[MPT_BORDER];
    mx = max(accent & 31, max((accent >> 5) & 31, (accent >> 10) & 31));
    mn = min(accent & 31, min((accent >> 5) & 31, (accent >> 10) & 31));
    gold = (mx - mn >= 6) ? MixColor(accent, white, 150) : RGB(31, 25, 6);
    memcpy(text, sTextPal, sizeof(text));
    text[TC_SHADOW] = MixColor(0, accent, 18);
    text[TC_GREY] = MixColor(RGB(21, 21, 23), accent, 40);
    text[TC_PANEL] = MixColor(0, accent, 58);
    text[TC_GOLD] = gold;
    text[TC_DIM] = MixColor(text[TC_PANEL], text[TC_GREY], 128);
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
    sG->arrowTop = CreateSprite(&arrowT, REEL_CX, REEL_TOP - 3, 0);
    gSprites[sG->arrowTop].vFlip = TRUE;
    gSprites[sG->arrowTop].data[0] = 1;   // bobs 1px: stays between the gold line and the boxes
    sG->arrowBottom = CreateSprite(&arrowT, REEL_CX, REEL_TOP + 38, 0);
    gSprites[sG->arrowBottom].invisible = TRUE;   // the name panel sits right under the reel

    BuildMenu();
    DrawHeader();
    DrawMenu();
    UpdateReel();
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
        DestroyTask(taskId);
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitGatcha);
    }
}

void MP_Gatcha_Open(void)
{
    LockPlayerFieldControls();
    FadeScreen(FADE_TO_BLACK, 0);
    CreateTask(Task_EnterGatcha, 80);
}
