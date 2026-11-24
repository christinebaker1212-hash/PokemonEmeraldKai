// Moemon Co-op: the hourly weather (the user, 2026-11-24): "randomized weather effects to outdoor areas across all 5
// regions, WITHOUT messing with pre-existing scripted weather ... Every hour, weather will change ... Rain and sun have
// coverage that moves across regions ... changing one route doesn't mean that the rain will stop falling immediately,
// but going to a completely different part of the map will ... Weather will be the same in both [server and solo
// games] regardless ... Brighter sun cannot happen during the night".
//
// The sky over each region is a field of clouds and clear spells that drifts with a wind (a new one each day) over the
// region's map grid: HOENN, KANTO and JOHTO by their maps' connections (mp_compass.c), SINNOH and UNOVA by Platinum's
// and Black's matrices. Where the rain field is high it rains (a thunderstorm at its heart; hail where it's cold, and
// everywhere in UNOVA's winter), where the sun's is high the sun is bright (only by day: at night it's simply clear),
// and a desert without a sandstorm of its own very rarely gets one. Everything comes from the hour (UTC, the
// server's clock online: MPMSG_S_WELCOME's time) and a fixed seed, so every game sees the same sky at the same hour,
// alone or together.
//
// Only a map whose own weather is plain (its header's NONE / SUNNY / SUNNY_CLOUDS, UNOVA's day with nothing on Black's
// calendar) is touched, and only one without weather triggers on its ground (Route 111's desert, Route 113's ash);
// a script's setweather always wins (the hour leaves a weather it didn't set alone). In battle the weather is the
// field's as always (rain, sun from the bright sun, sandstorm), and this hail is hail.
//
// The wild POKéMON that like the weather (MP_Weather_Likes: by Ability, then type) come out of a table's rare slots
// more often in it (MP_Weather_WildIndex); the app's POKéDEX says which ("Likes the sun.").
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_weather.h"
#include "s3d.h"
#include "u3d.h"
#include "rtc.h"
#include "event_data.h"
#include "script.h"
#include "overworld.h"
#include "field_weather.h"
#include "pokemon.h"
#include "constants/weather.h"
#include "constants/map_types.h"
#include "constants/abilities.h"
#include "constants/pokemon.h"
#include "constants/wild_encounter.h"
#include "wild_encounter.h"

u8 U3D_FieldWeather(void);

#define SEED 0x5EA50A17u
#define CELL 176      // the clouds' size, in tiles (a few routes across)
#define TCELL 6       // ... and how many hours they take to change shape
#define RAIN_AT 41000 // the fields' levels (0-65535) for rain, a storm, bright sun, a sandstorm
#define STORM_AT 50500
#define SUN_AT 40000
#define SAND_AT 52000

extern s64 gMpTestClockShift;
static s64 sServerDelta;
static bool8 sServerKnown;
static s32 sDebugHours;
static s8 sDebugForce = -1;

static u8 sApplied = 0xFF;        // the weather the hour gave this map (0xFF: none of ours here)
static u32 sAppliedHour;
static u16 sAppliedMap;

// ---------------------------------------------------------------------------------------------------- the clock
void MP_Weather_ServerTime(s64 serverUnix)
{
    sServerDelta = serverUnix - (s64)time(NULL);
    sServerKnown = TRUE;
}

static s64 Now(void)
{
    s64 t = (s64)time(NULL) + gMpTestClockShift + (s64)sDebugHours * 3600;
    if (sServerKnown && MP_IsConnected())
        t += sServerDelta;
    return t;
}

u32 MP_Weather_Hour(void)
{
    return (u32)(Now() / 3600);
}

// ---------------------------------------------------------------------------------------------------- the sky
static u32 Hash(u32 a, u32 b, u32 c, u32 d)
{
    u32 h = SEED ^ a * 0x9E3779B1u;
    h = (h ^ b * 0x85EBCA77u) * 0xC2B2AE3Du;
    h = (h ^ c * 0x27D4EB2Fu) * 0x165667B1u;
    h = (h ^ d * 0x9E3779B1u);
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    h *= 0x297A2D39u;
    h ^= h >> 15;
    return h;
}

static s32 FloorDiv(s32 a, s32 b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

// smooth value noise over (x, y, t), 0-65535
static u32 Smooth(u32 f, u32 one)   // 3f^2 - 2f^3, in 0..65536
{
    u64 x = (u64)f * 65536 / one;
    return (u32)((x * x * (3 * 65536 - 2 * x)) >> 32);
}
static u32 Noise(u32 layer, s32 x, s32 y, s32 t, s32 cell, s32 tcell)
{
    s32 cx = FloorDiv(x, cell), cy = FloorDiv(y, cell), ct = FloorDiv(t, tcell);
    u32 fx = Smooth((u32)(x - cx * cell), (u32)cell), fy = Smooth((u32)(y - cy * cell), (u32)cell);
    u32 ft = Smooth((u32)(t - ct * tcell), (u32)tcell);
    u64 v[2];
    s32 k;
    for (k = 0; k < 2; k++)
    {
        u32 a = Hash(layer, cx, cy, ct + k) & 0xFFFF, b = Hash(layer, cx + 1, cy, ct + k) & 0xFFFF;
        u32 c = Hash(layer, cx, cy + 1, ct + k) & 0xFFFF, d = Hash(layer, cx + 1, cy + 1, ct + k) & 0xFFFF;
        u64 top = ((u64)a * (65536 - fx) + (u64)b * fx) >> 16, bot = ((u64)c * (65536 - fx) + (u64)d * fx) >> 16;
        v[k] = (top * (65536 - fy) + bot * fy) >> 16;
    }
    return (u32)((v[0] * (65536 - ft) + v[1] * ft) >> 16);
}

// where the wind has carried the sky by this hour: a wind a day (one of eight ways, 16-40 tiles an hour), added up
// over the days since the last 64-day mark
static void Drift(u8 region, u32 hour, s32 *ox, s32 *oy)
{
    static const s8 sWay[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
    u32 day = hour / 24, d;
    s32 x = 0, y = 0;
    for (d = day & ~63u; d <= day; d++)
    {
        u32 h = Hash(region, d, 77, 0), way = h % 8, speed = 16 + (h >> 8) % 25;
        u32 hours = d < day ? 24 : hour % 24;
        s32 step = (s32)(speed * hours);
        if (sWay[way][0] && sWay[way][1])
            step = step * 7 / 10;   // (a diagonal: as fast)
        x += sWay[way][0] * step, y += sWay[way][1] * step;
    }
    *ox = x, *oy = y;
}

static u32 Field(u32 layer, u8 region, s32 x, s32 y, u32 hour)
{
    s32 ox, oy, t = (s32)(hour % 100000);
    Drift(region, hour, &ox, &oy);
    x -= ox, y -= oy;
    // (two sizes of cloud: the big fronts and the showers in them)
    return (Noise(layer * 8 + region, x, y, t, CELL, TCELL) * 2 + Noise(layer * 8 + region + 64, x, y, t, CELL / 2, TCELL / 2)) / 3;
}

static bool8 IsNight(void)
{
    RtcCalcLocalTime();
    return gLocalTime.hours < 6 || gLocalTime.hours >= 20;
}

// The sky at a place of a region at an hour (MP_WX_*): the fields alone
u8 MP_Weather_Sky(u8 region, s32 x, s32 y, u32 hour, u8 flags)
{
    u32 rain = Field(1, region, x, y, hour), sun = Field(2, region, x, y, hour);
    if ((flags & MP_WXF_DESERT) && Field(3, region, x, y, hour) >= SAND_AT && rain < RAIN_AT)
        return MP_WX_SAND;
    if (rain >= RAIN_AT)
        return (flags & MP_WXF_COLD) ? MP_WX_HAIL : rain >= STORM_AT ? MP_WX_STORM : MP_WX_RAIN;
    if (sun >= SUN_AT)
        return (flags & MP_WXF_NIGHT) ? MP_WX_CLEAR : MP_WX_SUN;
    return MP_WX_CLEAR;
}

// ---------------------------------------------------------------------------------------------------- the places
static const u16 sCold[] = {
    MAP_JOHTO_ROUTE28, MAP_JOHTO_ROUTE43, MAP_JOHTO_ROUTE44, MAP_JOHTO_ROUTE45, MAP_JOHTO_MAHOGANY_TOWN,
    MAP_JOHTO_BLACKTHORN_CITY, MAP_JOHTO_LAKE_OF_RAGE, MAP_JOHTO_LAKE_OF_RAGE_ALT, MAP_JOHTO_SILVER_CAVE_OUTSIDE,
    MAP_KANTO_ROUTE20, MAP_SINNOH_ROUTE211_EAST, MAP_SINNOH_ROUTE211_WEST, MAP_SINNOH_ROUTE210_NORTH,
};

static bool8 InList(const u16 *list, u32 n, u8 group, u8 num)
{
    u32 i;
    for (i = 0; i < n; i++)
        if (list[i] == ((group << 8) | num))
            return TRUE;
    return FALSE;
}

// UNOVA's deserts (Black's zones: DESERT RESORT, ROUTE 4), on the days Black's calendar gives them no sandstorm
static bool8 UnovaDesert(u16 zone) { return zone == 157 || zone == 326; }

// Where a map lies on its region's sky, and what it's like; FALSE when the hour leaves it alone
static bool8 Place(u8 group, u8 num, u8 base, u8 *region, s32 *x, s32 *y, u8 *flags)
{
    const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(group, num);
    u16 zone = 0xFFFF;
    u32 i;
    if (h == NULL || (base != WEATHER_NONE && base != WEATHER_SUNNY && base != WEATHER_SUNNY_CLOUDS))
        return FALSE;
    if (h->mapType != MAP_TYPE_ROUTE && h->mapType != MAP_TYPE_TOWN && h->mapType != MAP_TYPE_CITY &&
        h->mapType != MAP_TYPE_OCEAN_ROUTE)
        return FALSE;
    if (MP_Region_IsSharedMap(group, num))
        return FALSE;
    // (weather triggers on its ground: the map's weather is scripted)
    for (i = 0; h->events != NULL && i < h->events->coordEventCount; i++)
        if (h->events->coordEvents[i].script == NULL)
            return FALSE;
    *region = MP_Region_OfGroup(group);
    *flags = 0;
    if (S3D_IsSinnohGroup(group))
    {
        if (!S3D_MapWorldTile(group, num, x, y))
            return FALSE;
    }
    else if (U3D_IsUnovaGroup(group))
    {
        if (!U3D_MapWorldTile(group, num, x, y, &zone))
            return FALSE;
        if (U3D_Season() == 3)
            *flags |= MP_WXF_COLD;   // (UNOVA's winter)
        if (UnovaDesert(zone))
            *flags |= MP_WXF_DESERT;
    }
    else if (*region <= REGION_KANTO)
    {
        if (!MP_Compass_MapWorldTile(group, num, x, y))
            return FALSE;
    }
    else
        return FALSE;   // (ORRE and the rest: their own skies)
    if (InList(sCold, ARRAY_COUNT(sCold), group, num))
        *flags |= MP_WXF_COLD;
    return TRUE;
}

static u8 FieldWeatherOf(u8 wx, u8 base)
{
    switch (wx)
    {
    case MP_WX_RAIN:  return WEATHER_RAIN;
    case MP_WX_STORM: return WEATHER_RAIN_THUNDERSTORM;
    case MP_WX_SUN:   return WEATHER_DROUGHT;
    case MP_WX_HAIL:  return WEATHER_SNOW;
    case MP_WX_SAND:  return WEATHER_SANDSTORM;
    default:          return base == WEATHER_NONE ? WEATHER_NONE : WEATHER_SUNNY;
    }
}

// The sky over a map now (MP_WX_*), 0xFF when the hour leaves the map alone
u8 MP_Weather_MapSky(u8 group, u8 num, u8 base, u32 hour)
{
    u8 region, flags;
    s32 x, y;
    // (the test harness: plain maps keep their weather, so a scenario doesn't depend on its hour, unless MOEMON_SKY)
    if (getenv("MOEMON_TEST") && !getenv("MOEMON_SKY") && sDebugForce < 0 && !sDebugHours)
        return 0xFF;
    if (!Place(group, num, base, &region, &x, &y, &flags))
        return 0xFF;
    if (IsNight())
        flags |= MP_WXF_NIGHT;
    if (sDebugForce >= 0)
        return (u8)sDebugForce;
    return MP_Weather_Sky(region, x, y, hour, flags);
}

// The current map's weather as it's loaded (field_weather_effect.c SetSavedWeatherFromCurrMapHeader): its own, or the
// hour's
u8 MP_Weather_ForCurrentMap(u8 base)
{
    u8 group = gSaveBlock1Ptr->location.mapGroup, num = gSaveBlock1Ptr->location.mapNum;
    u32 hour = MP_Weather_Hour();
    u8 sky = MP_Weather_MapSky(group, num, base, hour);
    if (sky == 0xFF)
    {
        sApplied = 0xFF;
        return base;
    }
    sApplied = FieldWeatherOf(sky, base);
    sAppliedHour = hour;
    sAppliedMap = (group << 8) | num;
    return sApplied;
}

// The field weather now is the hour's (not a script's, not the map's own)
bool8 MP_Weather_Ours(void)
{
    return sApplied != 0xFF && gSaveBlock1Ptr != NULL &&
           sAppliedMap == ((gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum) &&
           gSaveBlock1Ptr->weather == sApplied;
}

// The bright sun is the hour's: gentler than the DROUGHT of GROUDON (field_weather.c DroughtStateRun)
bool8 MP_Weather_GentleSun(void)
{
    return MP_Weather_Ours() && sApplied == WEATHER_DROUGHT;
}

// Each field frame (overworld.c OverworldBasic): the hour turning changes the sky over the map, slowly, unless a
// script is running or changed the weather itself; the sun going down clears a bright sky
void MP_Weather_Update(void)
{
    static u8 sTick;
    u32 hour;
    u8 sky, want, base;
    u16 here;
    if (++sTick & 31 || gSaveBlock1Ptr == NULL || ScriptContext_IsEnabled())
        return;
    here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    base = gMapHeader.weather;
    if (U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) && U3D_FieldWeather() != 0xFF)
        base = U3D_FieldWeather();
    if (sAppliedMap != here)
    {
        // (a game just loaded: the map's weather as the save had it is taken for the hour's when it's a weather the
        // hour could have given it, and the hour takes it from there)
        u8 w = gSaveBlock1Ptr->weather;
        sAppliedMap = here;
        sApplied = 0xFF;
        if (MP_Weather_MapSky(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, base, 0) != 0xFF &&
            (w == base || w == WEATHER_SUNNY || w == WEATHER_RAIN || w == WEATHER_RAIN_THUNDERSTORM ||
             w == WEATHER_DROUGHT || w == WEATHER_SNOW || w == WEATHER_SANDSTORM))
            sApplied = w, sAppliedHour = 0;
    }
    if (!MP_Weather_Ours())
        return;
    hour = MP_Weather_Hour();
    sky = MP_Weather_MapSky(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, base, hour);
    if (sky == 0xFF)
        return;
    want = FieldWeatherOf(sky, base);
    if (want == sApplied)
    {
        sAppliedHour = hour;
        return;
    }
    sApplied = want;
    sAppliedHour = hour;
    SetSavedWeather(want);
    SetNextWeather(want);
}

// ---------------------------------------------------------------------------------------------------- wild POKéMON
// What weather a POKéMON likes (MP_WX_*, MP_WX_CLEAR none): its Abilities first (CHLOROPHYLL, SWIFT SWIM...), then its
// types (sun: FIRE, GRASS; rain: WATER, ELECTRIC, BUG; hail: ICE; sandstorm: ROCK, GROUND, STEEL)
u8 MP_Weather_Likes(u16 species)
{
    const struct SpeciesInfo *si;
    u32 i;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return MP_WX_CLEAR;
    si = &gSpeciesInfo[species];
    for (i = 0; i < 2; i++)
        switch (si->abilities[i])
        {
        case ABILITY_CHLOROPHYLL: case ABILITY_DROUGHT: case ABILITY_SOLAR_POWER: case ABILITY_LEAF_GUARD:
        case ABILITY_FLOWER_GIFT:
            return MP_WX_SUN;
        case ABILITY_SWIFT_SWIM: case ABILITY_RAIN_DISH: case ABILITY_DRIZZLE: case ABILITY_HYDRATION:
        case ABILITY_DRY_SKIN:
            return MP_WX_RAIN;
        case ABILITY_ICE_BODY: case ABILITY_SNOW_CLOAK: case ABILITY_SNOW_WARNING:
            return MP_WX_HAIL;
        case ABILITY_SAND_VEIL: case ABILITY_SAND_STREAM:
            return MP_WX_SAND;
        }
    for (i = 0; i < 2; i++)
        switch (si->types[i])
        {
        case TYPE_FIRE: case TYPE_GRASS:                 return MP_WX_SUN;
        case TYPE_WATER: case TYPE_ELECTRIC: case TYPE_BUG: return MP_WX_RAIN;
        case TYPE_ICE:                                   return MP_WX_HAIL;
        case TYPE_ROCK: case TYPE_GROUND: case TYPE_STEEL: return MP_WX_SAND;
        }
    return MP_WX_CLEAR;
}

// The field weather as a liking (any weather: a map's own rain counts too)
static u8 FieldSky(void)
{
    switch (GetCurrentWeather())
    {
    case WEATHER_RAIN: case WEATHER_RAIN_THUNDERSTORM: case WEATHER_DOWNPOUR: return MP_WX_RAIN;
    case WEATHER_DROUGHT:                                                  return MP_WX_SUN;
    case WEATHER_SNOW:                                                     return MP_WX_HAIL;
    case WEATHER_SANDSTORM:                                                return MP_WX_SAND;
    default:                                                               return MP_WX_CLEAR;
    }
}

// A wild POKéMON's slot (wild_encounter.c, after the table's own roll): in a weather some of a table's rare ones
// (5% or less) like, one encounter in five is one of them instead (their own odds between them)
u8 MP_Weather_WildIndex(const struct WildPokemon *mons, const u8 *rates, u8 count, u8 index, u32 roll)
{
    u8 sky = FieldSky(), i;
    u32 total = 0, r;
    if (sky == MP_WX_CLEAR || mons == NULL || roll % 5)
        return index;
    for (i = 0; i < count; i++)
        if (rates[i] <= 5 && MP_Weather_Likes(mons[i].species) == sky)
            total += rates[i];
    if (!total)
        return index;
    r = (roll / 5) % total;
    for (i = 0; i < count; i++)
        if (rates[i] <= 5 && MP_Weather_Likes(mons[i].species) == sky)
        {
            if (r < rates[i])
                return i;
            r -= rates[i];
        }
    return index;
}

// ---------------------------------------------------------------------------------------------------- tests
// The region's sky as letters, one a 48-tile square (. clear, r rain, T storm, S sun, h hail), around (x, y)
static void PrintRegion(u8 region, s32 x, s32 y, u32 hour)
{
    static const char sCh[] = ".rTShs";
    s32 i, j;
    for (j = -8; j <= 8; j++)
    {
        char line[40];
        for (i = -16; i <= 16; i++)
            line[i + 16] = (i == 0 && j == 0) ? '@' : sCh[MP_Weather_Sky(region, x + i * 48, y + j * 48, hour, 0)];
        line[33] = 0;
        printf("[weather]   %s\n", line);
    }
}

// mp sky [hours N | force W | off | region]: the sky here now (and the region's around it)
void MP_Weather_Debug(const char *arg)
{
    static const char *const sName[] = {"CLEAR", "RAIN", "STORM", "SUN", "HAIL", "SAND"};
    u8 group = gSaveBlock1Ptr->location.mapGroup, num = gSaveBlock1Ptr->location.mapNum, region, flags;
    s32 x, y;
    u32 hour;
    if (arg && !strncmp(arg, "hours ", 6))
        sDebugHours = atoi(arg + 6);
    else if (arg && !strncmp(arg, "force ", 6))
        sDebugForce = (s8)atoi(arg + 6);
    else if (arg && !strcmp(arg, "off"))
        sDebugForce = -1;
    hour = MP_Weather_Hour();
    if (!Place(group, num, gMapHeader.weather, &region, &x, &y, &flags))
    {
        printf("[weather] hour %u: this map keeps its own weather (%u, field %u)\n", hour, gMapHeader.weather,
               GetCurrentWeather());
        return;
    }
    printf("[weather] hour %u: region %u at %d,%d flags %u: %s (field %u, ours %d)\n", hour, region, x, y, flags,
           sName[MP_Weather_Sky(region, x, y, hour, flags | (IsNight() ? MP_WXF_NIGHT : 0))], GetCurrentWeather(),
           MP_Weather_Ours());
    if (arg && !strcmp(arg, "region"))
    {
        u32 k, n[MP_WX_COUNT] = {0};
        PrintRegion(region, x, y, hour);
        for (k = 0; k < 24 * 60; k++)   // (60 days here: how often each)
            n[MP_Weather_Sky(region, x, y, hour + k, 0)]++;
        printf("[weather] over 60 days here: clear %u rain %u storm %u sun %u (of %u hours)\n", n[0], n[1], n[2], n[3],
               24 * 60);
        {
            // (how often a place 60 tiles away (the next route) has the same sky, and one 600 away; and how often the
            // sky here stays the same from one hour to the next)
            u32 near = 0, far = 0, same = 0;
            for (k = 0; k < 24 * 60; k++)
            {
                u8 here = MP_Weather_Sky(region, x, y, hour + k, 0);
                near += here == MP_Weather_Sky(region, x + 60, y, hour + k, 0);
                far += here == MP_Weather_Sky(region, x + 600, y + 300, hour + k, 0);
                same += here == MP_Weather_Sky(region, x, y, hour + k + 1, 0);
            }
            printf("[weather] next route alike %u%%, far away alike %u%%, an hour later alike %u%%\n", near * 100 / 1440,
                   far * 100 / 1440, same * 100 / 1440);
        }
    }
}
