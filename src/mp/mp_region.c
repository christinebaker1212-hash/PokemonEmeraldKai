// Moemon Kai: HOENN, JOHTO and KANTO (docs/REGIONS_PLAN.md).
// - Which region a map is in (by its map group: Kai's groups are Hoenn, Johto 34-35, Kanto 36-39).
// - The flags and vars past the save block's: Johto / Kanto content uses flags 0x1000-0x3FFF and vars 0x5000-0x52FF
//   (tools/quetzal_port/import.py). They are kept in "<save>.region", written with the save and only read back
//   when it was written with this very save (like the bag's extra slots, mp_bag.c).
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "event_data.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_savefix.h"
#include "save.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "strings.h"
#include "constants/map_groups.h"
#include "constants/heal_locations.h"
#include "overworld.h"
#include "constants/flags.h"
#include "constants/qz_port.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/region_map_sections.h"

extern char gPlatformSavePath[512];

#define REGION_MAGIC 0x4E474552   // "REGN"
#define REGION_FORMAT 3   // 2: + struct RegionState, 3: + chunks (other regions' PCs, the GLOBAL PC, levels)

static u8 sFlags[REGION_FLAG_BYTES];
static u16 sVars[REGION_VARS_COUNT];
struct RegionState gRegionState;   // per-region progress (mp_region.h)

// Each region's own PC boxes: the save's are the region the player is in; the others wait here (sAwayValid: bit per
// region). The GLOBAL PC is one more set of boxes, shared by every region.
static struct PokemonStorage sAwayPC[REGION_COUNT];
static u8 sAwayValid;
static struct PokemonStorage sGlobalPC;
static bool8 sGlobalValid;

// Levels per region: a Pokémon's EXP in each region it has been in (it is Lv 5 the first time in a new one). The
// Pokémon's own EXP is the one of region "cur".
struct RegionLevel
{
    u32 pers, otid;
    u32 exp[REGION_COUNT];
    u8 has;   // bit per region: exp[] holds its EXP there
    u8 cur;
    u16 filler;
};
#define MAX_REGION_LEVELS 8192
static struct RegionLevel sLevels[MAX_REGION_LEVELS];
static u32 sLevelCount;

#define CHUNK(a, b, c, d) ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

u8 MP_Region_OfGroup(u8 group)
{
    if (group == MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) || group == MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) + 1)
        return REGION_JOHTO;
    if (group >= MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) + 2 && group <= MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) + 5)
        return REGION_KANTO;
    return REGION_HOENN;
}

u8 MP_Region_Current(void)
{
    return MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup);
}

u8 *MP_Region_FlagPointer(u16 id)
{
    if (id < REGION_FLAGS_START || id >= REGION_FLAGS_END)
        return NULL;
    return &sFlags[(id - REGION_FLAGS_START) / 8];
}

u16 *MP_Region_VarPointer(u16 id)
{
    if (id < REGION_VARS_START || id >= REGION_VARS_START + REGION_VARS_COUNT)
        return NULL;
    return &sVars[id - REGION_VARS_START];
}

void MP_Region_NewGame(void)
{
    memset(sFlags, 0, sizeof(sFlags));
    memset(sVars, 0, sizeof(sVars));
    memset(&gRegionState, 0, sizeof(gRegionState));
    sAwayValid = 0;
    sGlobalValid = FALSE;
    sLevelCount = 0;
}

// The region a new game starts in (PROF. BIRCH asks, main_menu.c), applied by MP_Region_StartNewGame
static u8 sStartRegion;

void MP_Region_SetStartRegion(u8 region)
{
    sStartRegion = region < REGION_COUNT ? region : REGION_HOENN;
}

// The house JOHTO's / KANTO's story starts in (NEW BARK, PALLET): a new game there, or the first boat trip (the
// player's room while they stay); sets the warp
void MP_Region_GoToStartHouse(u8 region)
{
    if (region == REGION_JOHTO)
    {
        SetWarpDestination(MAP_GROUP(MAP_JOHTO_PLAYERS_HOUSE2F), MAP_NUM(MAP_JOHTO_PLAYERS_HOUSE2F), WARP_ID_NONE, 4, 4);
        SetLastHealLocationWarp(HEAL_LOCATION_JOHTO_NEW_BARK_TOWN);
    }
    else if (region == REGION_KANTO)
    {
        SetWarpDestination(MAP_GROUP(MAP_KANTO_PALLET_TOWN_PLAYERS_HOUSE_2F), MAP_NUM(MAP_KANTO_PALLET_TOWN_PLAYERS_HOUSE_2F), WARP_ID_NONE, 6, 6);
        SetLastHealLocationWarp(HEAL_LOCATION_KANTO_PALLET_TOWN);
    }
}

// End of NewGameInitData: JOHTO starts in NEW BARK's house, KANTO in PALLET's (their own opening scripts run there)
void MP_Region_StartNewGame(void)
{
    const char *env = getenv("MOEMON_DEBUG_REGION");
    if (env != NULL)
        sStartRegion = atoi(env) % REGION_COUNT;
    gRegionState.current = sStartRegion;
    gRegionState.started = 1 << sStartRegion;
    if (sStartRegion != REGION_HOENN)
    {
        MP_Region_GoToStartHouse(sStartRegion);
        WarpIntoMap();
    }
    sStartRegion = REGION_HOENN;
}

// The ports' customs: TRUE when the trip to VAR_0x8004 is the player's first there (gStringVar4: what happens)
u16 MP_RegionFirstVisit(void)
{
    static const u8 sJohto[] = _("It's your first voyage to JOHTO!\pA guide will meet you at OLIVINE and\ntake you to NEW BARK TOWN, home of\lPROF. ELM's POKéMON LAB.\pYou'll have a room there while you\nstay in JOHTO.$");
    static const u8 sKanto[] = _("It's your first voyage to KANTO!\pA guide will meet you at VERMILION\nand take you to PALLET TOWN, home of\lPROF. OAK's POKéMON LAB.\pYou'll have a room there while you\nstay in KANTO.$");
    u8 to = gSpecialVar_0x8004;
    if (to == REGION_HOENN || to >= REGION_COUNT || (gRegionState.started & (1 << to)))
        return gSpecialVar_Result = FALSE;
    StringCopy(gStringVar4, to == REGION_JOHTO ? sJohto : sKanto);
    return gSpecialVar_Result = TRUE;
}

// What ties the side file to the save it was written with
static u32 Fingerprint(void)
{
    u32 sum = gSaveCounter * 2654435761u;
    sum = sum * 31 + (gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8
                      | gSaveBlock2Ptr->playerTrainerId[2] << 16 | (u32)gSaveBlock2Ptr->playerTrainerId[3] << 24);
    return sum;
}

static u8 sSaveFormat;   // the loaded save's format (mp_savefix.h): its region side file's SAVF chunk

void MP_Region_WriteSidecar(void)
{
    char path[600], tmp[610];
    FILE *f;
    u32 header[4];
    if (gSaveBlock2Ptr == NULL)
        return;
    snprintf(path, sizeof(path), "%s.region", gPlatformSavePath);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return;
    header[0] = REGION_MAGIC;
    header[1] = REGION_FORMAT;
    header[2] = Fingerprint();
    header[3] = sizeof(sFlags) + sizeof(sVars) + sizeof(gRegionState);
    fwrite(header, sizeof(header), 1, f);
    fwrite(sFlags, sizeof(sFlags), 1, f);
    fwrite(sVars, sizeof(sVars), 1, f);
    fwrite(&gRegionState, sizeof(gRegionState), 1, f);
    // chunks: {tag, length, data}
    {
        u32 c[2];
        u8 r;
        for (r = 0; r < REGION_COUNT; r++)
        {
            if (!(sAwayValid & (1 << r)))
                continue;
            c[0] = CHUNK('P', 'C', '0' + r, ' ');
            c[1] = sizeof(sAwayPC[r]);
            fwrite(c, sizeof(c), 1, f);
            fwrite(&sAwayPC[r], sizeof(sAwayPC[r]), 1, f);
        }
        if (sGlobalValid)
        {
            c[0] = CHUNK('G', 'L', 'P', 'C');
            c[1] = sizeof(sGlobalPC);
            fwrite(c, sizeof(c), 1, f);
            fwrite(&sGlobalPC, sizeof(sGlobalPC), 1, f);
        }
        c[0] = CHUNK('L', 'V', 'L', 'S');
        c[1] = sLevelCount * sizeof(sLevels[0]);
        fwrite(c, sizeof(c), 1, f);
        fwrite(sLevels, sizeof(sLevels[0]), sLevelCount, f);
        {
            u32 format = MP_SAVE_FORMAT;   // the save's own layout version (mp_savefix.h)
            c[0] = CHUNK('S', 'A', 'V', 'F');
            c[1] = sizeof(format);
            fwrite(c, sizeof(c), 1, f);
            fwrite(&format, sizeof(format), 1, f);
        }
    }
    fclose(f);
    remove(path);
    rename(tmp, path);
}

static bool8 LoadFrom(const char *path)
{
    FILE *f = fopen(path, "rb");
    u32 header[4];
    bool8 ok = FALSE;
    if (!f)
        return FALSE;
    memset(&gRegionState, 0, sizeof(gRegionState));
    sSaveFormat = 1;   // a side file without the format: 1.60.0
    sAwayValid = 0;
    sGlobalValid = FALSE;
    sLevelCount = 0;
    if (fread(header, sizeof(header), 1, f) == 1 && header[0] == REGION_MAGIC && header[2] == Fingerprint()
     && header[3] >= sizeof(sFlags) + sizeof(sVars))
    {
        ok = fread(sFlags, sizeof(sFlags), 1, f) == 1 && fread(sVars, sizeof(sVars), 1, f) == 1;
        if (ok && header[3] > sizeof(sFlags) + sizeof(sVars))   // format 2+: the region state (a later build's longer one: what fits)
        {
            u32 n = header[3] - sizeof(sFlags) - sizeof(sVars);
            fread(&gRegionState, n < sizeof(gRegionState) ? n : sizeof(gRegionState), 1, f);
            if (n > sizeof(gRegionState))
                fseek(f, n - sizeof(gRegionState), SEEK_CUR);
        }
        // format 3+: chunks (an unknown one is skipped)
        while (ok)
        {
            u32 c[2];
            long next;
            if (fread(c, sizeof(c), 1, f) != 1)
                break;
            next = ftell(f) + c[1];
            if (c[0] >= CHUNK('P', 'C', '0', ' ') && c[0] < CHUNK('P', 'C', '0' + REGION_COUNT, ' ')
             && (c[0] & 0xFF00FFFF) == CHUNK('P', 'C', 0, ' ') && c[1] == sizeof(struct PokemonStorage))
            {
                u8 r = ((c[0] >> 16) & 0xFF) - '0';
                if (fread(&sAwayPC[r], sizeof(sAwayPC[r]), 1, f) == 1)
                    sAwayValid |= 1 << r;
            }
            else if (c[0] == CHUNK('G', 'L', 'P', 'C') && c[1] == sizeof(sGlobalPC))
            {
                sGlobalValid = fread(&sGlobalPC, sizeof(sGlobalPC), 1, f) == 1;
            }
            else if (c[0] == CHUNK('S', 'A', 'V', 'F') && c[1] == sizeof(u32))
            {
                u32 format = 0;
                if (fread(&format, sizeof(format), 1, f) == 1)
                    sSaveFormat = format > 255 ? 255 : format;
            }
            else if (c[0] == CHUNK('L', 'V', 'L', 'S') && c[1] % sizeof(sLevels[0]) == 0)
            {
                u32 n = c[1] / sizeof(sLevels[0]);
                if (n > MAX_REGION_LEVELS)
                    n = MAX_REGION_LEVELS;
                sLevelCount = fread(sLevels, sizeof(sLevels[0]), n, f);
            }
            fseek(f, next, SEEK_SET);
        }
    }
    fclose(f);
    return ok;
}

void MP_Region_LoadFromSave(void)
{
    char path[600];
    int i;
    for (i = 0; i <= 3; i++)
    {
        if (i == 0)
            snprintf(path, sizeof(path), "%s.region", gPlatformSavePath);
        else
            snprintf(path, sizeof(path), "%s.bak%d.region", gPlatformSavePath, i);
        if (LoadFrom(path))
        {
            MPNet_Log("region: loaded%s", i ? " (from a backup)" : "");
            MP_SaveFix_Migrate(sSaveFormat);
            return;
        }
    }
    MP_Region_NewGame();
    MPNet_Log("region: no side file for this save (Johto / Kanto progress starts empty)");
    MP_SaveFix_Migrate(0);   // 1.59.x and before
}

// ---------------------------------------------------------------------------------------------------- progress
static const u16 sBadgeFlags[REGION_COUNT] = {FLAG_BADGE01_GET, FLAG_QZ_0x11B2, FLAG_QZ_0x2100};

u16 MP_Region_BadgeFlag(u8 region, u8 badge)
{
    if (region >= REGION_COUNT || badge >= 8)
        return 0;
    return sBadgeFlags[region] + badge;
}

u8 MP_Region_BadgeCount(u8 region)
{
    u8 i, n = 0;
    for (i = 0; i < 8; i++)
        n += FlagGet(MP_Region_BadgeFlag(region, i)) != 0;
    return n;
}

bool8 MP_Region_Cleared(u8 region)
{
    if (region == REGION_HOENN && FlagGet(FLAG_SYS_GAME_CLEAR))
        return TRUE;
    return region < REGION_COUNT && (gRegionState.cleared & (1 << region));
}

void MP_Region_SetCleared(u8 region)
{
    if (region < REGION_COUNT)
        gRegionState.cleared |= 1 << region;
}

bool8 MP_Region_AnyCleared(void)
{
    u8 r;
    for (r = 0; r < REGION_COUNT; r++)
        if (MP_Region_Cleared(r))
            return TRUE;
    return FALSE;
}

// ---------------------------------------------------------------------------------------------------- PCs
static void ResetStorage(struct PokemonStorage *st, const u8 *name)
{
    u8 i;
    memset(st, 0, sizeof(*st));
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        u8 *dest = StringCopy(st->boxNames[i], name);
        ConvertIntToDecimalStringN(dest, i + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        st->boxWallpapers[i] = i % 12;   // the 12 default wallpapers (FOREST to SAVANNA: pokemon_storage_system.c)
    }
}

struct PokemonStorage *MP_Region_GlobalPC(void)
{
    static const u8 sGlobal[] = _("GLOBAL");   // + 1-14: BOX_NAME_LENGTH (8) letters at most
    if (!sGlobalValid)
    {
        ResetStorage(&sGlobalPC, sGlobal);
        sGlobalValid = TRUE;
    }
    return &sGlobalPC;
}

// Puts a Pokémon in the GLOBAL PC's first free place (FALSE: it's full)
bool8 MP_Region_DepositGlobal(const struct BoxPokemon *mon)
{
    struct PokemonStorage *st = MP_Region_GlobalPC();
    u8 b, p;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (p = 0; p < IN_BOX_COUNT; p++)
            if (!GetBoxMonData((struct BoxPokemon *)&st->boxes[b][p], MON_DATA_SANITY_HAS_SPECIES))
            {
                st->boxes[b][p] = *mon;
                return TRUE;
            }
    return FALSE;
}

u16 MP_Region_GlobalFree(void)
{
    struct PokemonStorage *st = MP_Region_GlobalPC();
    u16 n = 0;
    u8 b, p;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (p = 0; p < IN_BOX_COUNT; p++)
            n += !GetBoxMonData(&st->boxes[b][p], MON_DATA_SANITY_HAS_SPECIES);
    return n;
}

// ---------------------------------------------------------------------------------------------------- levels
static struct RegionLevel *FindLevels(u32 pers, u32 otid)
{
    u32 i;
    for (i = 0; i < sLevelCount; i++)
        if (sLevels[i].pers == pers && sLevels[i].otid == otid)
            return &sLevels[i];
    return NULL;
}

// Gives a Pokémon its EXP for "region" (keeping its EXP for the region it had). TRUE when its EXP changed.
bool8 MP_Region_FitBoxMon(struct BoxPokemon *b, u8 region)
{
    struct RegionLevel *e;
    u32 pers, otid, exp, newExp;
    u16 species;
    if (region >= REGION_COUNT || !GetBoxMonData(b, MON_DATA_SANITY_HAS_SPECIES) || GetBoxMonData(b, MON_DATA_IS_EGG))
        return FALSE;
    pers = GetBoxMonData(b, MON_DATA_PERSONALITY);
    otid = GetBoxMonData(b, MON_DATA_OT_ID);
    exp = GetBoxMonData(b, MON_DATA_EXP);
    e = FindLevels(pers, otid);
    if (e == NULL)
    {
        // first seen: the level it has is the one of the region it's in
        if (sLevelCount >= MAX_REGION_LEVELS)
            return FALSE;
        e = &sLevels[sLevelCount++];
        memset(e, 0, sizeof(*e));
        e->pers = pers;
        e->otid = otid;
        e->cur = region;
    }
    e->exp[e->cur] = exp;
    e->has |= 1 << e->cur;
    if (e->cur == region)
        return FALSE;
    species = GetBoxMonData(b, MON_DATA_SPECIES);
    newExp = (e->has & (1 << region)) ? e->exp[region] : gExperienceTables[gSpeciesInfo[species].growthRate][5];
    e->cur = region;
    if (newExp == exp)
        return FALSE;
    SetBoxMonData(b, MON_DATA_EXP, &newExp);
    return TRUE;
}

void MP_Region_FitMon(struct Pokemon *mon, u8 region)
{
    if (MP_Region_FitBoxMon(&mon->box, region))
    {
        CalculateMonStats(mon);
        if (GetMonData(mon, MON_DATA_HP) > 0)
        {
            u16 hp = GetMonData(mon, MON_DATA_MAX_HP);
            SetMonData(mon, MON_DATA_HP, &hp);
        }
    }
}

// The party and the player's own boxes take the levels of the region the player is in (after the GLOBAL PC: what
// came out of it came with another region's level)
void MP_Region_FitAll(void)
{
    u8 region = MP_Region_Current(), i, b, p;
    struct PokemonStorage *st = MP_PersonalStorage();
    for (i = 0; i < PARTY_SIZE; i++)
        MP_Region_FitMon(&gPlayerParty[i], region);
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (p = 0; p < IN_BOX_COUNT; p++)
            MP_Region_FitBoxMon(&st->boxes[b][p], region);
}

// ---------------------------------------------------------------------------------------------------- travel
// Leaving "from" for "to": the personal PC's boxes change places, the party takes the new region's levels
void MP_Region_Travel(u8 from, u8 to)
{
    static const u8 sBox[] = _("BOX");
    struct PokemonStorage *own = MP_PersonalStorage();
    u8 i;
    if (from >= REGION_COUNT || to >= REGION_COUNT || from == to)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
        MP_Region_FitMon(&gPlayerParty[i], from);   // remembers their levels here
    sAwayPC[from] = *own;
    sAwayValid |= 1 << from;
    if (sAwayValid & (1 << to))
    {
        *own = sAwayPC[to];
        sAwayValid &= ~(1 << to);
    }
    else
    {
        ResetStorage(own, sBox);
    }
    for (i = 0; i < PARTY_SIZE; i++)
        MP_Region_FitMon(&gPlayerParty[i], to);
    gRegionState.current = to;
    gRegionState.started |= 1 << to;
    MP_SendRoomRules();   // a host's room goes with them: friends still in the old region are told and leave
    MPNet_Log("region: travel %d -> %d (PC boxes swapped, party at its levels there)", from, to);
}

// ---------------------------------------------------------------------------------------------------- specials
// The GLOBAL PC links only in a region whose Elite Four the player has beaten
u16 MP_GlobalPcLinked(void)
{
    return gSpecialVar_Result = MP_Region_Cleared(MP_Region_Current());
}

u16 MP_RegionPartyCount(void)
{
    return gSpecialVar_Result = CalculatePlayerPartyCount();
}

// ---------------------------------------------------------------------------------------------------- generations
// Which Pokémon generations a region shows (docs/REGIONS_PLAN.md "Generations per region"): HOENN its own Gen 1-3,
// JOHTO and KANTO Gen 1-2. The GENERATION setting (gMpGenSetting, 3 = the vanilla game) raises them: at 4 and up
// every region shows up to that generation. Johto's / Kanto's wild tables and trainers' teams are the rip's Gen 2 cap
// below Gen 4 (tools/quetzal_port/gen_gen2.py), its Gen 4 cap from there.
#include "wild_encounter.h"
#include "data.h"
#include "../data/wild_qz_gen2.h"
#include "../data/trainer_parties_qz_gen2.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

u8 gMpGenSetting = 3;

u8 MP_Gen_Cap(u8 region)
{
    u8 g = gMpGenSetting < 3 ? 3 : gMpGenSetting;
    if (region != REGION_HOENN && g <= 3)
        return 2;
    return g;
}

u8 MP_Gen_OfSpecies(u16 species)
{
    static const u16 sLast[] = {151, 251, 386, 493, 649, 721, 809, 905, 1025};
    u16 nat = SpeciesToNationalPokedexNum(species);
    u8 g;
    if (nat == 0)
        return 0;   // (the EGG, none)
    for (g = 0; g < ARRAY_COUNT(sLast); g++)
        if (nat <= sLast[g])
            return g + 1;
    return ARRAY_COUNT(sLast) + 1;
}

// A Pokémon of a generation the region the player is in shows
bool8 MP_Gen_Allowed(u16 species)
{
    return MP_Gen_OfSpecies(species) <= MP_Gen_Cap(MP_Region_Current());
}

// The nearest earlier form the region shows (a MAMOSWINE in a Gen 2 region is a PILOSWINE); the species itself when
// it's allowed or nothing before it is
u16 MP_Gen_StandIn(u16 species)
{
    u16 s = species, depth;
    for (depth = 0; depth < 4 && !MP_Gen_Allowed(s); depth++)
    {
        u16 from = SPECIES_NONE, i;
        u8 j;
        for (i = 1; i < NUM_SPECIES && from == SPECIES_NONE; i++)
            for (j = 0; j < EVOS_PER_MON; j++)
                if (gEvolutionTable[i][j].targetSpecies == s)
                {
                    from = i;
                    break;
                }
        if (from == SPECIES_NONE)
            return species;
        s = from;
    }
    return MP_Gen_Allowed(s) ? s : species;
}

static bool8 RegionUsesGen2(void)
{
    return MP_Region_Current() != REGION_HOENN && MP_Gen_Cap(MP_Region_Current()) < 4;
}

const struct WildPokemonInfo *MP_Region_WildInfo(const struct WildPokemonInfo *info)
{
    u32 i;
    if (info == NULL || !RegionUsesGen2())
        return info;
    for (i = 0; i < ARRAY_COUNT(sQzGen2Wild); i++)
        if (sQzGen2Wild[i].gen4 == info)
            return sQzGen2Wild[i].gen2;
    return info;
}

const struct Trainer *MP_Region_TrainerData(u16 trainerNum)
{
    static struct Trainer sCopy;
    u32 i;
    if (trainerNum < sQzGen2Parties[0].trainerId || !RegionUsesGen2())
        return &gTrainers[trainerNum];
    for (i = 0; i < ARRAY_COUNT(sQzGen2Parties); i++)
    {
        if (sQzGen2Parties[i].trainerId == trainerNum)
        {
            sCopy = gTrainers[trainerNum];
            sCopy.party = sQzGen2Parties[i].party;
            sCopy.partySize = sQzGen2Parties[i].partySize;
            sCopy.partyFlags = sQzGen2Parties[i].partyFlags;
            return &sCopy;
        }
    }
    return &gTrainers[trainerNum];
}

// ---------------------------------------------------------------------------------------------------- met location
void MP_Region_TagMet(struct BoxPokemon *b)
{
    u8 game = MP_METGAME_JOHTO;
    if (gMapHeader.regionMapSectionId >= MAPSEC_JOHTO_FIRST && gMapHeader.regionMapSectionId < MAPSEC_JOHTO_END)
        SetBoxMonData(b, MON_DATA_MET_GAME, &game);
}

// A map of another region was entered without the boat (ROUTE 22's gate, the debug warp): the same change of PC
// boxes and levels as a trip
void MP_Region_OnMapLoaded(void)
{
    u8 cur = MP_Region_Current();
    if (cur != REGION_HOENN)
    {
        void Tt_MapSetup(void);
        Tt_MapSetup();   // the CARD QUEEN shows in JOHTO's / KANTO's POKeMON CENTERs while she's there
    }
    if (cur != gRegionState.current && gRegionState.current < REGION_COUNT)
        MP_Region_Travel(gRegionState.current, cur);
}
