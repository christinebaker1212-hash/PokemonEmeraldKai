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

// The MOéMON GARDEN and the GAME CENTER's floors are HOENN's maps but every region's: in one, the player is in the
// region they came from (no trip, no PC swap), and its way out leads back to where they stepped in
bool8 MP_Region_IsSharedMap(u8 group, u8 num)
{
    static const u16 sShared[] = {MAP_MOEMON_GARDEN, MAP_RUSTBORO_CITY_GAME_CENTER_1F, MAP_RUSTBORO_CITY_GAME_CENTER_2F,
                                  MAP_RUSTBORO_CITY_GAME_CENTER_3F, MAP_RUSTBORO_CITY_GAME_CENTER_ROOFTOP};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sShared); i++)
        if (MAP_GROUP(sShared[i]) == group && MAP_NUM(sShared[i]) == num)
            return TRUE;
    return FALSE;
}

u8 MP_Region_Current(void)
{
    u8 group = gSaveBlock1Ptr->location.mapGroup;
    if (MP_Region_IsSharedMap(group, gSaveBlock1Ptr->location.mapNum) && gRegionState.current < REGION_COUNT)
        return gRegionState.current;
    return MP_Region_OfGroup(group);
}

void MP_Region_NoteWarp(u8 destGroup, u8 destNum)
{
    const struct WarpData *here = &gSaveBlock1Ptr->location;
    if (!MP_Region_IsSharedMap(destGroup, destNum) || MP_Region_IsSharedMap(here->mapGroup, here->mapNum))
        return;
    gRegionState.originGroup = here->mapGroup;
    gRegionState.originNum = here->mapNum;
    gRegionState.originX = gSaveBlock1Ptr->pos.x;
    gRegionState.originY = gSaveBlock1Ptr->pos.y;
    gRegionState.originValid = TRUE;
}

// JOHTO's / KANTO's GAME CENTER: 2F and 3F, reached by the GAME CORNERS' attendants. The elevator's other floor
// there (VAR_RESULT 2 or 3; 0 in HOENN, whose tower keeps its four floors)
u16 MP_Region_GameCenterOtherFloor(void)
{
    u8 group = gSaveBlock1Ptr->location.mapGroup, num = gSaveBlock1Ptr->location.mapNum;
    if (MP_Region_Current() == REGION_HOENN || group != MAP_GROUP(MAP_RUSTBORO_CITY_GAME_CENTER_2F))
        return gSpecialVar_Result = 0;
    if (num == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_2F))
        return gSpecialVar_Result = 3;
    if (num == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_3F))
        return gSpecialVar_Result = 2;
    return gSpecialVar_Result = 0;
}

// A warp that doesn't lead anywhere in JOHTO's / KANTO's GAME CENTER: 3F's stairs up to the ROOFTOP
bool8 MP_Region_WarpClosed(u8 destGroup, u8 destNum)
{
    return destGroup == MAP_GROUP(MAP_RUSTBORO_CITY_GAME_CENTER_ROOFTOP) && destNum == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_ROOFTOP)
        && MP_Region_Current() != REGION_HOENN;
}

// After SetupWarp set where a warp leads (the GARDEN's own redirect too): out of a shared map in JOHTO / KANTO, back
// to where the player stepped in (HOENN keeps the maps' own exits: RUSTBORO's doors, the CENTERs' escalators)
void MP_Region_SharedExit(u8 destGroup, u8 destNum)
{
    const struct WarpData *here = &gSaveBlock1Ptr->location;
    if (!MP_Region_IsSharedMap(here->mapGroup, here->mapNum) || MP_Region_Current() == REGION_HOENN
     || !gRegionState.originValid)
        return;
    // (JOHTO's / KANTO's GAME CENTER is its 2F and 3F only: the stairs down to 1F lead out)
    if (MP_Region_IsSharedMap(destGroup, destNum)
     && !(destGroup == MAP_GROUP(MAP_RUSTBORO_CITY_GAME_CENTER_1F) && destNum == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_1F)))
        return;
    SetWarpDestination(gRegionState.originGroup, gRegionState.originNum, WARP_ID_NONE, gRegionState.originX,
                       gRegionState.originY);
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

// The region picked on PROF. BIRCH's question (before the new game starts)
u8 MP_Region_StartChoice(void)
{
    return sStartRegion;
}

// End of NewGameInitData: JOHTO starts in NEW BARK's house, KANTO in PALLET's (their own opening scripts run there)
#include "../data/qz_region_init.h"

// JOHTO / KANTO set up as Quetzal does on a region's first arrival (its new game, or the first trip there): the people
// and objects that start hidden, story variables; then the guard flag, so only once. Every arrival sets one more flag.
static void MP_Region_Arrive(u8 region)
{
    u32 i;
    if (region == REGION_HOENN || region > ARRAY_COUNT(sQzRegionInit))
        return;
    if (!FlagGet(sQzRegionInit[region - 1].guard))
    {
        for (i = 0; i < sQzRegionInit[region - 1].nFlags; i++)
            FlagSet(sQzRegionInit[region - 1].flags[i]);
        for (i = 0; i < sQzRegionInit[region - 1].nVars; i++)
            VarSet(sQzRegionInit[region - 1].vars[i][0], sQzRegionInit[region - 1].vars[i][1]);
        FlagSet(sQzRegionInit[region - 1].guard);
        MPNet_Log("region: %s set up (%d flags)", region == 1 ? "JOHTO" : "KANTO", sQzRegionInit[region - 1].nFlags);
    }
    FlagSet(sQzRegionInit[region - 1].arrival);
}

void MP_Region_StartNewGame(void)
{
    const char *env = getenv("MOEMON_DEBUG_REGION");
    if (env != NULL)
        sStartRegion = atoi(env) % REGION_COUNT;
    gRegionState.current = sStartRegion;
    gRegionState.started = 1 << sStartRegion;
    MP_Region_Arrive(sStartRegion);
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

// Saves from before 1.61.3 kept Quetzal's system flags in the region flags (0x2A05 POKeMON, 0x2A06 POKeDEX,
// 0x2A08 STRENGTH, 0x2A0A RUNNING SHOES): they're Emerald's own now (constants/qz_port.h)
static void MoveQuetzalSystemFlags(void)
{
    static const u16 sMoves[][2] = {{0x2A05, FLAG_SYS_POKEMON_GET}, {0x2A06, FLAG_SYS_POKEDEX_GET},
                                    {0x2A08, FLAG_SYS_USE_STRENGTH}, {0x2A0A, FLAG_SYS_B_DASH}};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMoves); i++)
    {
        if (FlagGet(sMoves[i][0]))
        {
            if (sMoves[i][1] != FLAG_SYS_USE_STRENGTH)
                FlagSet(sMoves[i][1]);
            FlagClear(sMoves[i][0]);
        }
    }
}

// HOENN's POKeDEX comes with the NATIONAL DEX (1.61.4: before, only after HOENN's League); saves that got BIRCH's
// POKeDEX without it get it on loading (it shows the GENERATION's POKeMON: MP_Gen_DexCount). JOHTO / KANTO give theirs
// in their own scripts.
static void GiveNationalDex(void)
{
    if (FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH) && !IsNationalPokedexEnabled())
    {
        EnableNationalPokedex();
        MPNet_Log("region: the NATIONAL DEX for this save's POKeDEX");
    }
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
            // three Johto / Kanto trainers had Emerald's special numbers 1022-1024 in 1.60 (gen_trainers.py): their
            // "beaten" flags move to their new numbers
            static const u16 sMoved[][2] = {{1022, TRAINER_QZ_4563}, {1023, TRAINER_QZ_4564}, {1024, TRAINER_QZ_4566}};
            int k;
            MPNet_Log("region: loaded%s", i ? " (from a backup)" : "");
            for (k = 0; k < (int)ARRAY_COUNT(sMoved); k++)
                if (FlagGet(TRAINER_FLAG(sMoved[k][0])))
                {
                    FlagSet(TRAINER_FLAG(sMoved[k][1]));
                    FlagClear(TRAINER_FLAG(sMoved[k][0]));
                }
            MoveQuetzalSystemFlags();
            MP_SaveFix_Migrate(sSaveFormat);
            GiveNationalDex();
            return;
        }
    }
    MP_Region_NewGame();
    MPNet_Log("region: no side file for this save (Johto / Kanto progress starts empty)");
    MP_SaveFix_Migrate(0);   // 1.59.x and before
    GiveNationalDex();
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

// A region's League beaten (GameClear): every Pokémon that has been in that region takes the higher of its levels, in
// every region it has been in (the Lv 5 start only lasts until the new region's League; a region it hasn't been to
// still starts it at Lv 5). The user's rule (1.61.4).
static void LeagueLevelsIn(struct BoxPokemon *b, struct Pokemon *mon)
{
    struct RegionLevel *e;
    u32 exp;
    if (!GetBoxMonData(b, MON_DATA_SANITY_HAS_SPECIES) || GetBoxMonData(b, MON_DATA_IS_EGG))
        return;
    e = FindLevels(GetBoxMonData(b, MON_DATA_PERSONALITY), GetBoxMonData(b, MON_DATA_OT_ID));
    if (e == NULL || !(e->has & (1 << e->cur)))
        return;
    exp = e->exp[e->cur];
    if (GetBoxMonData(b, MON_DATA_EXP) == exp)
        return;
    SetBoxMonData(b, MON_DATA_EXP, &exp);
    if (mon != NULL)
    {
        u16 hp;
        CalculateMonStats(mon);
        hp = GetMonData(mon, MON_DATA_MAX_HP);
        SetMonData(mon, MON_DATA_HP, &hp);
    }
}

void MP_Region_LeagueLevels(u8 region)
{
    struct PokemonStorage *own = MP_PersonalStorage();
    u32 i, b, p, raised = 0;
    u8 r;
    if (region >= REGION_COUNT)
        return;
    MP_Region_FitAll();   // (records the party's and boxes' EXP here first)
    for (i = 0; i < sLevelCount; i++)
    {
        struct RegionLevel *e = &sLevels[i];
        u32 best = 0;
        if (!(e->has & (1 << region)))
            continue;
        for (r = 0; r < REGION_COUNT; r++)
            if ((e->has & (1 << r)) && e->exp[r] > best)
                best = e->exp[r];
        for (r = 0; r < REGION_COUNT; r++)
            if ((e->has & (1 << r)) && e->exp[r] != best)
            {
                e->exp[r] = best;
                raised++;
            }
    }
    for (i = 0; i < PARTY_SIZE; i++)
        LeagueLevelsIn(&gPlayerParty[i].box, &gPlayerParty[i]);
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (p = 0; p < IN_BOX_COUNT; p++)
        {
            LeagueLevelsIn(&own->boxes[b][p], NULL);
            if (sGlobalValid)
                LeagueLevelsIn(&sGlobalPC.boxes[b][p], NULL);
            for (r = 0; r < REGION_COUNT; r++)
                if (sAwayValid & (1 << r))
                    LeagueLevelsIn(&sAwayPC[r].boxes[b][p], NULL);
        }
    MPNet_Log("region: League of region %d beaten: %u level(s) raised to the Pokemon's best", region, raised);
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
    MP_Region_Arrive(to);
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
#include "item.h"
#include "main.h"
#include "data.h"
#include "battle.h"
#include "constants/abilities.h"
#include "../data/wild_qz_gen2.h"
#include "../data/trainer_parties_qz_gen2.h"
#include "../data/wild_qz_modern.h"
#include "../data/trainer_parties_qz_modern.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

u8 gMpGenSetting = 0;   // tests (mp gen N): overrides OPTIONS > GAME > GENERATION / the room's

u8 MP_Gen_Cap(u8 region)
{
    u8 g = gMpGenSetting ? gMpGenSetting : MP_Generation();
    if (g < 3)
        g = 3;
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

// The generation a move comes from (Kai's ids are the national ones to Gen 4, then pokeemerald-expansion's)
u8 MP_Gen_OfMove(u16 move)
{
    static const u16 sLast[] = {165, 251, 354, 467, 559, 621, 689, 778};
    u8 g;
    for (g = 0; g < ARRAY_COUNT(sLast); g++)
        if (move <= sLast[g])
            return g + 1;
    return ARRAY_COUNT(sLast) + 1;
}

u8 MP_Gen_OfAbility(u16 ability)
{
    static const u16 sLast[] = {0, 0, ABILITY_AIR_LOCK, ABILITY_BAD_DREAMS, ABILITY_AROMA_VEIL - 1, ABILITY_STAMINA - 1,
                                ABILITY_INTREPID_SWORD - 1, ABILITY_LINGERING_AROMA - 1};
    u8 g;
    for (g = 2; g < ARRAY_COUNT(sLast); g++)
        if (ability <= sLast[g])
            return g + 1;
    return ARRAY_COUNT(sLast) + 1;
}

// The generation moves and abilities follow: the region's cap, down to Gen 3 (the default)
// What a POKéMON, move, item or ability may be (SEALED past it): the GENERATION, never below Gen 3. JOHTO's / KANTO's
// Gen 2 cap at GENERATION 3 (MP_Gen_Cap) is for the POKéMON the region shows (wild, trainers, evolutions) only: every
// POKéMON has an ability and the items are Gen 3's, so a Gen 2 limit there would seal every team
static u8 UseCap(void)
{
    u8 cap = MP_Gen_Cap(MP_Region_Current());
    return cap < 3 ? 3 : cap;
}

bool8 MP_Gen_MoveAllowed(u16 move)
{
    return MP_Gen_OfMove(move) <= UseCap();
}

// FAIRY came in Gen 6: before it a Pokémon or move takes its older type; from it the older Pokémon that became FAIRY
// take it (Kai's own data has them as Gen 3 had them)
static const struct {u16 species; u8 type1, type2;} sFairyFromGen6[] = {
    {SPECIES_CLEFAIRY, TYPE_FAIRY, TYPE_FAIRY}, {SPECIES_CLEFABLE, TYPE_FAIRY, TYPE_FAIRY}, {SPECIES_CLEFFA, TYPE_FAIRY, TYPE_FAIRY},
    {SPECIES_JIGGLYPUFF, TYPE_NORMAL, TYPE_FAIRY}, {SPECIES_WIGGLYTUFF, TYPE_NORMAL, TYPE_FAIRY}, {SPECIES_IGGLYBUFF, TYPE_NORMAL, TYPE_FAIRY},
    {SPECIES_MR_MIME, TYPE_PSYCHIC, TYPE_FAIRY}, {SPECIES_MIME_JR, TYPE_PSYCHIC, TYPE_FAIRY},
    {SPECIES_TOGEPI, TYPE_FAIRY, TYPE_FAIRY}, {SPECIES_TOGETIC, TYPE_FAIRY, TYPE_FLYING}, {SPECIES_TOGEKISS, TYPE_FAIRY, TYPE_FLYING},
    {SPECIES_SNUBBULL, TYPE_FAIRY, TYPE_FAIRY}, {SPECIES_GRANBULL, TYPE_FAIRY, TYPE_FAIRY},
    {SPECIES_MARILL, TYPE_WATER, TYPE_FAIRY}, {SPECIES_AZUMARILL, TYPE_WATER, TYPE_FAIRY}, {SPECIES_AZURILL, TYPE_NORMAL, TYPE_FAIRY},
    {SPECIES_MAWILE, TYPE_STEEL, TYPE_FAIRY},
    {SPECIES_RALTS, TYPE_PSYCHIC, TYPE_FAIRY}, {SPECIES_KIRLIA, TYPE_PSYCHIC, TYPE_FAIRY}, {SPECIES_GARDEVOIR, TYPE_PSYCHIC, TYPE_FAIRY},
};

u8 MP_Gen_SpeciesType(u16 species, u8 which)
{
    u8 type = gSpeciesInfo[species].types[which & 1], other = gSpeciesInfo[species].types[!(which & 1)];
    u32 i;
    if (MP_Gen_Cap(MP_Region_Current()) >= 6)
    {
        for (i = 0; i < ARRAY_COUNT(sFairyFromGen6); i++)
            if (sFairyFromGen6[i].species == species)
                return (which & 1) ? sFairyFromGen6[i].type2 : sFairyFromGen6[i].type1;
        return type;
    }
    if (type == TYPE_FAIRY)
        return other != TYPE_FAIRY ? other : TYPE_NORMAL;
    return type;
}

u8 MP_Gen_MoveType(u8 type)
{
    if (type == TYPE_FAIRY && MP_Gen_Cap(MP_Region_Current()) < 6)
        return TYPE_NORMAL;
    return type;
}

// The generation an item comes from (0: Kai's own and the story items JOHTO / KANTO need: always there)
u8 MP_Gen_OfItem(u16 item)
{
    static const u8 sLater[] = {   // ITEM_QUICK_POWDER .. ITEM_RAGE_CANDY_BAR
        4, 5, 4, 5, 6, 6, 7, 7, 2};
    static const u8 sGen59[] = {   // ITEM_ICE_STONE .. ITEM_LEADERS_CREST
        7, 8, 8, 9, 8, 8, 9, 9, 9, 8, 8, 9, 9, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5, 6, 7, 7, 7, 7, 9, 7, 7, 7, 7,
        6, 5, 5, 6, 8, 6, 9, 9, 9};
    if (item <= ITEM_TOPO_BERRY)
        return 3;
    if (item >= ITEM_QUICK_POWDER && item <= ITEM_RAGE_CANDY_BAR)
        return sLater[item - ITEM_QUICK_POWDER];
    if (item == ITEM_CANDY_JAR || item == ITEM_CARD_CASE)
        return 0;
    if (item == ITEM_LINKING_CORD)
        return 3;   // (it does the trade evolutions of every generation: pokemon.c)
    if (item >= ITEM_CARD_CASE + 1 && item <= ITEM_ROWAP_BERRY)
        return 4;
    if (item >= ITEM_ICE_STONE && item < ITEM_ICE_STONE + ARRAY_COUNT(sGen59))
        return sGen59[item - ITEM_ICE_STONE];
    return 0;
}

bool8 MP_Gen_ItemAllowed(u16 item)
{
    return MP_Gen_OfItem(item) <= UseCap();
}

// SEALED: a Pokémon of a species, with a move or with an ability past the GENERATION stays yours but can't battle
// (like an EGG), until GENERATION is raised again
bool8 MP_Gen_SealedSpecies(u16 species)
{
    return species != SPECIES_NONE && species < NUM_SPECIES && MP_Gen_OfSpecies(species) > UseCap();
}

bool8 MP_Gen_SealedMon(struct Pokemon *mon)
{
    u32 i;
    u16 species;
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return FALSE;
    if (MP_Gen_SealedSpecies(species))
        return TRUE;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u16 move = GetMonData(mon, MON_DATA_MOVE1 + i);
        if (move != MOVE_NONE && !MP_Gen_MoveAllowed(move))
            return TRUE;
    }
    return MP_Gen_OfAbility(GetMonAbility(mon)) > UseCap();
}

// Held items past the GENERATION go back to the bag (party and PC; one that doesn't fit stays held). Called when the
// generation may have changed: a map loads, OPTIONS, the room's rules
static u8 ReturnItem(struct BoxPokemon *b)
{
    u16 item;
    if (!GetBoxMonData(b, MON_DATA_SPECIES) || GetBoxMonData(b, MON_DATA_IS_EGG))
        return 0;
    item = GetBoxMonData(b, MON_DATA_HELD_ITEM);
    if (item == ITEM_NONE || MP_Gen_ItemAllowed(item) || !AddBagItem(item, 1))
        return 0;
    item = ITEM_NONE;
    SetBoxMonData(b, MON_DATA_HELD_ITEM, &item);
    return 1;
}

void MP_Gen_ReturnHeldItems(void)
{
    u8 cap = MP_Gen_Cap(MP_Region_Current());
    u32 i, j, n = 0;
    if (gSaveBlock1Ptr == NULL || gMain.inBattle)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
        n += ReturnItem(&gPlayerParty[i].box);
    if (gPokemonStoragePtr != NULL)
        for (i = 0; i < TOTAL_BOXES_COUNT; i++)
            for (j = 0; j < IN_BOX_COUNT; j++)
                n += ReturnItem(&gPokemonStoragePtr->boxes[i][j]);
    if (n)
        MPNet_Log("generation %d: %u held item(s) back to the bag", cap, n);
}

// A battler's types and ability as the generation has them (past the cap: no ability)
void MP_Gen_FitBattler(u8 battler)
{
    gBattleMons[battler].types[0] = MP_Gen_SpeciesType(gBattleMons[battler].species, 0);
    gBattleMons[battler].types[1] = MP_Gen_SpeciesType(gBattleMons[battler].species, 1);
    if (MP_Gen_OfAbility(gBattleMons[battler].ability) > UseCap())
        gBattleMons[battler].ability = ABILITY_NONE;
}

// The NATIONAL DEX's size at the GENERATION (pokedex.c): 386 at GEN 3 ... 1025 at 9, never below Gen 3's
u16 MP_Gen_DexCount(void)
{
    static const u16 sLast[] = {151, 251, 386, 493, 649, 721, 809, 905, 1025};
    u8 c = UseCap();
    return sLast[min(c, ARRAY_COUNT(sLast)) - 1];
}

// A Pokémon of a generation the region the player is in shows
bool8 MP_Gen_Allowed(u16 species)
{
    return MP_Gen_OfSpecies(species) <= MP_Gen_Cap(MP_Region_Current());
}

#include "../data/qz_later_objects.h"

// JOHTO's / KANTO's overworld POKéMON of a later generation (legendaries standing on the map, MAGEARNA...) stay out
// of sight while the GENERATION doesn't allow them (event_object_movement.c, with the object's own flag)
bool8 MP_Gen_ObjectHidden(u8 localId)
{
    u32 i;
    u8 group = gSaveBlock1Ptr->location.mapGroup, num = gSaveBlock1Ptr->location.mapNum;
    if (MP_Region_OfGroup(group) == REGION_HOENN)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sQzLaterObjects); i++)
        if (sQzLaterObjects[i].group == group && sQzLaterObjects[i].num == num && sQzLaterObjects[i].localId == localId)
            return !MP_Gen_Allowed(sQzLaterObjects[i].species);
    return FALSE;
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

static bool8 RegionUsesModern(void)
{
    return MP_Region_Current() != REGION_HOENN && MP_Gen_Cap(MP_Region_Current()) >= 5;
}

static const struct WildPokemonInfo *HoennModernWild(const struct WildPokemonInfo *info);

const struct WildPokemonInfo *MP_Region_WildInfo(const struct WildPokemonInfo *info)
{
    u32 i;
    if (info != NULL && MP_Region_Current() == REGION_HOENN && MP_Gen_Cap(REGION_HOENN) >= 4)
        return HoennModernWild(info);
    if (info != NULL && RegionUsesModern())
    {
        // Quetzal's own Pokémon where their generation is allowed, the Gen 4 cap's elsewhere
        static struct WildPokemon sMons[12];
        static struct WildPokemonInfo sInfo;
        for (i = 0; i < ARRAY_COUNT(sQzModernWild); i++)
        {
            if (sQzModernWild[i].gen4 == info)
            {
                u8 j, n = min(sQzModernWild[i].count, ARRAY_COUNT(sMons));
                for (j = 0; j < n; j++)
                {
                    sMons[j] = info->wildPokemon[j];
                    if (MP_Gen_Allowed(sQzModernWild[i].species[j]))
                        sMons[j].species = sQzModernWild[i].species[j];
                }
                sInfo.encounterRate = info->encounterRate;
                sInfo.wildPokemon = sMons;
                return &sInfo;
            }
        }
        return info;
    }
    if (info == NULL || !RegionUsesGen2())
        return info;
    for (i = 0; i < ARRAY_COUNT(sQzGen2Wild); i++)
        if (sQzGen2Wild[i].gen4 == info)
            return sQzGen2Wild[i].gen2;
    return info;
}

// Quetzal's own team where each Pokémon's generation is allowed, the Gen 4 cap's elsewhere; moves past the cap are
// the Gen 4 cap's in the same slot
static const struct Trainer *ModernTeam(u16 trainerNum)
{
    static struct Trainer sCopy;
    static struct TrainerMonItemCustomMoves sCustom[PARTY_SIZE];
    static struct TrainerMonItemDefaultMoves sDefault[PARTY_SIZE];
    const struct Trainer *t = &gTrainers[trainerNum];
    bool8 custom = (t->partyFlags & F_TRAINER_PARTY_CUSTOM_MOVESET) != 0;
    u32 i, j, k;

    for (i = 0; i < ARRAY_COUNT(sQzModernParties); i++)
    {
        const struct QzModernMon *m;
        if (sQzModernParties[i].trainerId != trainerNum)
            continue;
        m = sQzModernParties[i].mons;
        for (j = 0; j < t->partySize && j < PARTY_SIZE; j++)
        {
            u16 species, item = ITEM_NONE, moves[MAX_MON_MOVES] = {0};
            u8 lvl, iv;
            switch (t->partyFlags)
            {
            case 0:
                species = t->party.NoItemDefaultMoves[j].species, lvl = t->party.NoItemDefaultMoves[j].lvl, iv = t->party.NoItemDefaultMoves[j].iv;
                break;
            case F_TRAINER_PARTY_CUSTOM_MOVESET:
                species = t->party.NoItemCustomMoves[j].species, lvl = t->party.NoItemCustomMoves[j].lvl, iv = t->party.NoItemCustomMoves[j].iv;
                memcpy(moves, t->party.NoItemCustomMoves[j].moves, sizeof(moves));
                break;
            case F_TRAINER_PARTY_HELD_ITEM:
                species = t->party.ItemDefaultMoves[j].species, lvl = t->party.ItemDefaultMoves[j].lvl, iv = t->party.ItemDefaultMoves[j].iv;
                item = t->party.ItemDefaultMoves[j].heldItem;
                break;
            default:
                species = t->party.ItemCustomMoves[j].species, lvl = t->party.ItemCustomMoves[j].lvl, iv = t->party.ItemCustomMoves[j].iv;
                item = t->party.ItemCustomMoves[j].heldItem;
                memcpy(moves, t->party.ItemCustomMoves[j].moves, sizeof(moves));
                break;
            }
            if (j < sQzModernParties[i].count && MP_Gen_Allowed(m[j].species))
            {
                bool8 any = FALSE;
                if (m[j].species != species)
                    item = m[j].heldItem;
                species = m[j].species;
                lvl = m[j].lvl;
                for (k = 0; k < MAX_MON_MOVES; k++)
                {
                    if (m[j].moves[k] != MOVE_NONE && MP_Gen_MoveAllowed(m[j].moves[k]))
                        moves[k] = m[j].moves[k];
                    any |= moves[k] != MOVE_NONE;
                }
                if (custom && !any)
                    custom = FALSE;   // (none of its moves fits: the level-up ones)
            }
            sCustom[j].iv = sDefault[j].iv = iv;
            sCustom[j].lvl = sDefault[j].lvl = lvl;
            sCustom[j].species = sDefault[j].species = species;
            sCustom[j].heldItem = sDefault[j].heldItem = item;
            memcpy(sCustom[j].moves, moves, sizeof(moves));
        }
        sCopy = *t;
        if (custom)
        {
            sCopy.partyFlags = F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM;
            sCopy.party.ItemCustomMoves = sCustom;
        }
        else
        {
            sCopy.partyFlags = F_TRAINER_PARTY_HELD_ITEM;
            sCopy.party.ItemDefaultMoves = sDefault;
        }
        return &sCopy;
    }
    return t;
}

// ---------------------------------------------------------------------------------------------------- HOENN by GENERATION
// HOENN has no newer tables of its own: tools/moemon_gen59/gen_hoenn_modern.py gives rarer wild slots and some team
// members a generation 4-9 and a newer POKéMON of the same type and stage; each shows from its generation on
#include "../data/hoenn_modern.h"

static const struct WildPokemonInfo *HoennModernWild(const struct WildPokemonInfo *info)
{
    static struct WildPokemon sMons[12];
    static struct WildPokemonInfo sInfo;
    u8 cap = MP_Gen_Cap(REGION_HOENN);
    bool8 any = FALSE;
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sHoennModernWild); i++)
    {
        const struct HoennModernWild *e = &sHoennModernWild[i];
        if (e->info != info || e->gen > cap || !MP_Gen_Allowed(e->species))
            continue;
        if (!any)
        {
            memcpy(sMons, info->wildPokemon, min(e->count, ARRAY_COUNT(sMons)) * sizeof(sMons[0]));
            any = TRUE;
        }
        if (e->slot < ARRAY_COUNT(sMons))
            sMons[e->slot].species = e->species;   // (the table is in generation order: a later one wins its slot)
    }
    if (!any)
        return info;
    sInfo.encounterRate = info->encounterRate;
    sInfo.wildPokemon = sMons;
    return &sInfo;
}

// The last 4 level-up moves up to LVL the GENERATION allows (as a POKéMON made at that level knows)
static void LevelUpMoves(u16 species, u8 lvl, u16 *moves)
{
    u32 i, n = 0;
    memset(moves, 0, MAX_MON_MOVES * sizeof(u16));
    for (i = 0; gLevelUpLearnsets[species][i] != LEVEL_UP_END; i++)
    {
        u16 move = gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_ID;
        u32 k;
        if ((gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_LV) > LEVEL_UP_LV(lvl))
            break;
        if (!MP_Gen_MoveAllowed(move))
            continue;
        for (k = 0; k < MAX_MON_MOVES && moves[k] != move; k++)
            ;
        if (k < MAX_MON_MOVES)
            continue;
        if (n < MAX_MON_MOVES)
            moves[n++] = move;
        else
        {
            memmove(moves, moves + 1, (MAX_MON_MOVES - 1) * sizeof(u16));
            moves[MAX_MON_MOVES - 1] = move;
        }
    }
}

static const struct Trainer *HoennModernTeam(u16 trainerNum)
{
    static struct Trainer sCopy;
    static struct TrainerMonItemCustomMoves sCustom[PARTY_SIZE];
    const struct Trainer *t = &gTrainers[trainerNum];
    u8 cap = MP_Gen_Cap(REGION_HOENN);
    bool8 any = FALSE, custom = (t->partyFlags & F_TRAINER_PARTY_CUSTOM_MOVESET) != 0;
    u32 i, j;

    for (j = 0; j < t->partySize && j < PARTY_SIZE; j++)
    {
        struct TrainerMonItemCustomMoves *m = &sCustom[j];
        memset(m, 0, sizeof(*m));
        switch (t->partyFlags)
        {
        case 0:
            m->species = t->party.NoItemDefaultMoves[j].species, m->lvl = t->party.NoItemDefaultMoves[j].lvl;
            m->iv = t->party.NoItemDefaultMoves[j].iv;
            break;
        case F_TRAINER_PARTY_CUSTOM_MOVESET:
            m->species = t->party.NoItemCustomMoves[j].species, m->lvl = t->party.NoItemCustomMoves[j].lvl;
            m->iv = t->party.NoItemCustomMoves[j].iv;
            memcpy(m->moves, t->party.NoItemCustomMoves[j].moves, sizeof(m->moves));
            break;
        case F_TRAINER_PARTY_HELD_ITEM:
            m->species = t->party.ItemDefaultMoves[j].species, m->lvl = t->party.ItemDefaultMoves[j].lvl;
            m->iv = t->party.ItemDefaultMoves[j].iv, m->heldItem = t->party.ItemDefaultMoves[j].heldItem;
            break;
        default:
            *m = t->party.ItemCustomMoves[j];
            break;
        }
        if (!custom)
            LevelUpMoves(m->species, m->lvl, m->moves);   // (the copy always names its moves)
        for (i = 0; i < ARRAY_COUNT(sHoennModernTrainers); i++)
        {
            const struct HoennModernMon *e = &sHoennModernTrainers[i];
            if (e->trainerId == trainerNum && e->slot == j && e->gen <= cap && MP_Gen_Allowed(e->species))
            {
                m->species = e->species;
                LevelUpMoves(m->species, m->lvl, m->moves);
                any = TRUE;
            }
        }
    }
    if (!any)
        return t;
    sCopy = *t;
    sCopy.partyFlags = F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM;
    sCopy.party.ItemCustomMoves = sCustom;
    return &sCopy;
}

const struct Trainer *MP_Region_TrainerData(u16 trainerNum)
{
    static struct Trainer sCopy;
    u32 i;
    if (trainerNum < MAX_TRAINERS_COUNT && MP_Region_Current() == REGION_HOENN && MP_Gen_Cap(REGION_HOENN) >= 4)
        return HoennModernTeam(trainerNum);
    if (trainerNum >= sQzGen2Parties[0].trainerId && RegionUsesModern())
        return ModernTeam(trainerNum);
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
    MP_Gen_ReturnHeldItems();   // GENERATION: held items past it go back to the bag
    if (cur == REGION_JOHTO)
    {
        u16 QzGiovanniStoryUpdateFlags(void);
        QzGiovanniStoryUpdateFlags();   // Quetzal refreshes these on every JOHTO warp (its special 693)
    }
    if (cur != REGION_HOENN)
    {
        void Tt_MapSetup(void);
        Tt_MapSetup();   // the CARD QUEEN shows in JOHTO's / KANTO's POKeMON CENTERs while she's there
    }
    if (cur != gRegionState.current && gRegionState.current < REGION_COUNT)
        MP_Region_Travel(gRegionState.current, cur);
}
