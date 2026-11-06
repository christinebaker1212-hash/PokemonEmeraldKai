// Moemon Kai: one-time legendaries come back 24 hours after a KO (the user, 2026-11-05: "One-time legendaries
// (Birth Island, Faraday, etc.): return 24 h after a KO, retroactively").
//
// A legendary that was beaten (not caught) has its map's "done" flag set, so its own scripts never show it again.
// This keeps the time of each KO in "<save>.legends" (lines "FLAG UNIXTIME", the PC's own clock) and, on entering
// a map (before its transition scripts), clears the done flag of every KO more than a day old: the map's scripts
// then show the legendary as the first time.
//   HOENN: Emerald's islands and caves (sHoenn: their DEFEATED flags; the ones caught have their own CAUGHT flag, or
//          set the same flag, then the POKéDEX tells).
//   JOHTO / KANTO: every legendary of Quetzal's static encounters (qz_specials.c AfterStatic calls MP_Legend_Beaten
//          with the entry's done flag; JOHTO's RAIKOU and ENTEI roam: mp_johto_roamer.c).
//   SINNOH keeps Platinum's own rule.
// Retroactive: a done flag with no time kept, whose POKéMON the player never caught, is a KO from before this: it
// comes back on the next map change.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "pokedex.h"
#include "pokemon.h"
#include "mp.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/species.h"
#include "aw_file.h"   // (sidecars written in the background: src/platform/aw_file.c)

extern char gPlatformSavePath[512];

#define LEGEND_RETURN_SECONDS (24 * 60 * 60)
#define MAX_KOS 128

struct HoennLegend
{
    u16 species, species2, map, defeated, caught;   // (caught 0: the same flag either way, the POKéDEX tells)
};

static const struct HoennLegend sHoenn[] = {
    {SPECIES_DEOXYS, 0, MAP_BIRTH_ISLAND_EXTERIOR, FLAG_DEFEATED_DEOXYS, FLAG_BATTLED_DEOXYS},
    {SPECIES_MEW, 0, MAP_FARAWAY_ISLAND_INTERIOR, FLAG_DEFEATED_MEW, FLAG_CAUGHT_MEW},
    {SPECIES_LUGIA, 0, MAP_NAVEL_ROCK_BOTTOM, FLAG_DEFEATED_LUGIA, FLAG_CAUGHT_LUGIA},
    {SPECIES_HO_OH, 0, MAP_NAVEL_ROCK_TOP, FLAG_DEFEATED_HO_OH, FLAG_CAUGHT_HO_OH},
    {SPECIES_LATIAS, SPECIES_LATIOS, MAP_SOUTHERN_ISLAND_INTERIOR, FLAG_DEFEATED_LATIAS_OR_LATIOS, FLAG_CAUGHT_LATIAS_OR_LATIOS},
    {SPECIES_REGIROCK, 0, MAP_DESERT_RUINS, FLAG_DEFEATED_REGIROCK, 0},
    {SPECIES_REGICE, 0, MAP_ISLAND_CAVE, FLAG_DEFEATED_REGICE, 0},
    {SPECIES_REGISTEEL, 0, MAP_ANCIENT_TOMB, FLAG_DEFEATED_REGISTEEL, 0},
    {SPECIES_KYOGRE, 0, MAP_MARINE_CAVE_END, FLAG_DEFEATED_KYOGRE, 0},
    {SPECIES_GROUDON, 0, MAP_TERRA_CAVE_END, FLAG_DEFEATED_GROUDON, 0},
    {SPECIES_RAYQUAZA, 0, MAP_SKY_PILLAR_TOP, FLAG_DEFEATED_RAYQUAZA, 0},
};

// Legendary and mythical POKéMON, generations 1-9
static const u16 sLegendSpecies[] = {
    SPECIES_ARTICUNO, SPECIES_ZAPDOS, SPECIES_MOLTRES, SPECIES_MEWTWO, SPECIES_MEW,
    SPECIES_RAIKOU, SPECIES_ENTEI, SPECIES_SUICUNE, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI,
    SPECIES_REGIROCK, SPECIES_REGICE, SPECIES_REGISTEEL, SPECIES_LATIAS, SPECIES_LATIOS, SPECIES_KYOGRE,
    SPECIES_GROUDON, SPECIES_RAYQUAZA, SPECIES_JIRACHI, SPECIES_DEOXYS,
    SPECIES_UXIE, SPECIES_MESPRIT, SPECIES_AZELF, SPECIES_DIALGA, SPECIES_PALKIA, SPECIES_HEATRAN, SPECIES_REGIGIGAS,
    SPECIES_GIRATINA, SPECIES_CRESSELIA, SPECIES_PHIONE, SPECIES_MANAPHY, SPECIES_DARKRAI, SPECIES_SHAYMIN, SPECIES_ARCEUS,
    SPECIES_VICTINI, SPECIES_COBALION, SPECIES_TERRAKION, SPECIES_VIRIZION, SPECIES_TORNADUS, SPECIES_THUNDURUS,
    SPECIES_RESHIRAM, SPECIES_ZEKROM, SPECIES_LANDORUS, SPECIES_KYUREM, SPECIES_KELDEO, SPECIES_MELOETTA, SPECIES_GENESECT,
    SPECIES_XERNEAS, SPECIES_YVELTAL, SPECIES_ZYGARDE, SPECIES_DIANCIE, SPECIES_HOOPA, SPECIES_VOLCANION,
    SPECIES_TYPE_NULL, SPECIES_SILVALLY, SPECIES_TAPU_KOKO, SPECIES_TAPU_LELE, SPECIES_TAPU_BULU, SPECIES_TAPU_FINI,
    SPECIES_COSMOG, SPECIES_COSMOEM, SPECIES_SOLGALEO, SPECIES_LUNALA, SPECIES_NIHILEGO, SPECIES_BUZZWOLE,
    SPECIES_PHEROMOSA, SPECIES_XURKITREE, SPECIES_CELESTEELA, SPECIES_KARTANA, SPECIES_GUZZLORD, SPECIES_NECROZMA,
    SPECIES_MAGEARNA, SPECIES_MARSHADOW, SPECIES_POIPOLE, SPECIES_NAGANADEL, SPECIES_STAKATAKA, SPECIES_BLACEPHALON,
    SPECIES_ZERAORA, SPECIES_MELTAN, SPECIES_MELMETAL, SPECIES_ZACIAN, SPECIES_ZAMAZENTA, SPECIES_ETERNATUS,
    SPECIES_KUBFU, SPECIES_URSHIFU, SPECIES_ZARUDE, SPECIES_REGIELEKI, SPECIES_REGIDRAGO, SPECIES_GLASTRIER,
    SPECIES_SPECTRIER, SPECIES_CALYREX, SPECIES_ENAMORUS, SPECIES_WO_CHIEN, SPECIES_CHIEN_PAO, SPECIES_TING_LU,
    SPECIES_CHI_YU, SPECIES_KORAIDON, SPECIES_MIRAIDON, SPECIES_WALKING_WAKE, SPECIES_IRON_LEAVES, SPECIES_OKIDOGI,
    SPECIES_MUNKIDORI, SPECIES_FEZANDIPITI, SPECIES_OGERPON, SPECIES_TERAPAGOS, SPECIES_PECHARUNT,
};

static struct { u16 flag; s64 at; } sKo[MAX_KOS];
static u32 sKoCount;
static char sLoadedFor[512];

bool8 MP_IsLegendSpecies(u16 species)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sLegendSpecies); i++)
        if (sLegendSpecies[i] == species)
            return TRUE;
    return FALSE;
}

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.legends", gPlatformSavePath);
}

// (again whenever the save file changed: SAVE FILE switching)
static void Load(void)
{
    char path[600], line[64];
    FILE *f;
    if (strcmp(sLoadedFor, gPlatformSavePath) == 0)
        return;
    snprintf(sLoadedFor, sizeof(sLoadedFor), "%s", gPlatformSavePath);
    sKoCount = 0;
    Path(path, sizeof(path));
    f = fopen(path, "r");
    if (f == NULL)
        return;
    while (fgets(line, sizeof(line), f) && sKoCount < MAX_KOS)
    {
        unsigned flag;
        long long at;
        if (sscanf(line, "%x %lld", &flag, &at) == 2)
        {
            sKo[sKoCount].flag = flag;
            sKo[sKoCount].at = at;
            sKoCount++;
        }
    }
    fclose(f);
}

static void Store(void)
{
    char path[600];
    AwFile *f;
    u32 i;
    Path(path, sizeof(path));
    if (sKoCount == 0)
    {
        AwFile_Remove(path);
        return;
    }
    f = AwFile_Open(path);
    if (f == NULL)
        return;
    for (i = 0; i < sKoCount; i++)
        AwFile_Printf(f, "%04x %lld\n", sKo[i].flag, (long long)sKo[i].at);
    AwFile_Close(f);
}

static s32 Find(u16 flag)
{
    u32 i;
    for (i = 0; i < sKoCount; i++)
        if (sKo[i].flag == flag)
            return i;
    return -1;
}

static void Forget(u32 i)
{
    sKo[i] = sKo[--sKoCount];
}

// A legendary whose done flag is FLAG was just beaten (not caught): it comes back in a day
void MP_Legend_Beaten(u16 flag)
{
    s32 i;
    Load();
    i = Find(flag);
    if (i < 0)
    {
        if (sKoCount >= MAX_KOS)
            return;
        i = sKoCount++;
        sKo[i].flag = flag;
    }
    sKo[i].at = (s64)time(NULL);
    Store();
    MPNet_Log("legends: flag %04x beaten, back in a day", flag);
}

// After a scripted wild battle (battle_setup.c CB2_EndScriptedWildBattle): one of HOENN's, beaten here
void MP_Legend_BattleEnded(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u16 species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
    u32 i;
    if (gBattleOutcome != B_OUTCOME_WON)
        return;
    for (i = 0; i < ARRAY_COUNT(sHoenn); i++)
        if (sHoenn[i].map == map && (sHoenn[i].species == species || (sHoenn[i].species2 && sHoenn[i].species2 == species)))
            MP_Legend_Beaten(sHoenn[i].defeated);
}

static bool8 Caught(u16 species)
{
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
}

// The done flag back off: TRUE if it was on
static bool8 Return(u16 flag)
{
    if (!FlagGet(flag))
        return FALSE;
    FlagClear(flag);
    MPNet_Log("legends: flag %04x back", flag);
    return TRUE;
}

// JOHTO / KANTO's statics (qz_specials.c): each legendary's done flag and species, for the retroactive pass
u32 MP_Qz_LegendStatics(u16 *flags, u16 *species, u32 max);

// On entering a map (script.c RunOnTransitionMapScript, before the map's own scripts)
void MP_Legend_Tick(void)
{
    static u16 qzFlags[256], qzSpecies[256];
    s64 now = (s64)time(NULL), wait = LEGEND_RETURN_SECONDS;
    u32 i, n;
    bool8 changed = FALSE;
    if (getenv("MOEMON_LEGEND_RETURN"))   // (tests: seconds)
        wait = atoi(getenv("MOEMON_LEGEND_RETURN"));
    if (gSaveBlock1Ptr == NULL)
        return;
    Load();
    // the ones kept: back when a day has gone
    for (i = 0; i < sKoCount;)
    {
        if (now - sKo[i].at >= wait || now < sKo[i].at - 60)   // (a clock turned back: no wait)
        {
            Return(sKo[i].flag);
            Forget(i);
            changed = TRUE;
        }
        else
            i++;
    }
    // retroactive: beaten before the times were kept
    for (i = 0; i < ARRAY_COUNT(sHoenn); i++)
    {
        const struct HoennLegend *e = &sHoenn[i];
        bool8 caught;
        if (!FlagGet(e->defeated) || Find(e->defeated) >= 0)
            continue;
        caught = e->caught ? FlagGet(e->caught) : Caught(e->species);
        if (!caught)
            Return(e->defeated);
    }
    n = MP_Qz_LegendStatics(qzFlags, qzSpecies, ARRAY_COUNT(qzFlags));
    for (i = 0; i < n; i++)
        if (FlagGet(qzFlags[i]) && Find(qzFlags[i]) < 0 && !Caught(qzSpecies[i]))
            Return(qzFlags[i]);
    if (changed)
        Store();
}
