// Moemon Kai: UNOVA's wild POKéMON (docs/UNOVA_HANDOFF.md): Black's own encounter tables (a/1/2/6 in UNOVA's
// scripts pack, by the zone's encounter id; a season's row where the zone has four), read as the player steps and fed
// to the game's own encounter step (wild_encounter.c StandardWildEncounter's UNOVA line): grass and dark grass from
// their twelve slots (Black's slot odds are the game's: 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1), surfing from its
// five (60, 30, 5, 4, 1), each at its own rate. Species are Black's national numbers. Its own code.
#include "global.h"
#include "metatile_behavior.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "random.h"
#include "rtc.h"
#include "u3d.h"
#include "constants/species.h"

const u8 *U3D_Vm_WildRow(u16 zone, u8 season);

#define GRASS 8
#define DARK_GRASS (GRASS + 12 * 4)
#define SURF (8 + 36 * 4)
#define GRASS_SPECIAL (GRASS + 24 * 4)   // (Black's EncountData: ground[2], water[1], fishing[0] and [1])
#define SURF_SPECIAL (SURF + 5 * 4)
#define FISHING (SURF + 10 * 4)
#define FISHING_SPECIAL (FISHING + 5 * 4)

static struct WildPokemon sMons[12];
static struct WildPokemonInfo sInfo;

// White's own in Black's slots (the user, 2026-11-12: White's exclusives caught in UNOVA too): where White has its
// counterpart, half the time (national numbers: COTTONEE / PETILIL, GOTHITA / SOLOSIS, VULLABY / RUFFLET lines)
static u16 WhiteCounterpart(u16 national)
{
    static const u16 sPairs[][2] = {{546, 548}, {547, 549}, {574, 577}, {575, 578}, {576, 579}, {629, 627}, {630, 628}};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPairs); i++)
        if (sPairs[i][0] == national)
            return (Random() & 1) ? sPairs[i][1] : national;
    return national;
}

static bool8 Fill(const u8 *row, u32 at, u32 count, u8 rate)
{
    u32 i;
    if (rate == 0)
        return FALSE;
    for (i = 0; i < count; i++)
    {
        const u8 *s = row + at + 4 * i;
        u16 national = WhiteCounterpart((s[0] | s[1] << 8) & 0x7FF);
        sMons[i].species = national ? NationalPokedexNumToSpecies(national) : SPECIES_NONE;
        sMons[i].minLevel = s[2] ? s[2] : 1;
        sMons[i].maxLevel = s[3] >= sMons[i].minLevel ? s[3] : sMons[i].minLevel;
    }
    sInfo.encounterRate = rate;
    sInfo.wildPokemon = sMons;
    return TRUE;
}

// Black's mass outbreaks (ov021 field_encount.c FIELD_ENCOUNT_GetSwarmEntry / TrySwarmSlot, its sSwarmTable): once
// the HALL OF FAME is entered (flag 0x960) one of these 17 routes has a swarm each day (Black rolls it with the day;
// here the day and the player's ID pick it, the same all day), 400 in 1000 of its grass's POKeMON, Lv 15-55
static const u16 sSwarms[17][2] = {{317, 83}, {319, 360}, {321, 313}, {326, 449}, {329, 235}, {331, 311}, {337, 161},
                                   {345, 453}, {348, 228}, {355, 236}, {365, 285}, {368, 84}, {370, 353}, {374, 193},
                                   {378, 56}, {383, 204}, {387, 102}};

static const u16 *SwarmToday(void)
{
    bool8 U3D_Vm_Flag(u16 id);
    u32 day;
    if (!U3D_Vm_Flag(0x960))
        return NULL;
    RtcCalcLocalTime();
    day = (u32)gLocalTime.days + (gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8);
    return sSwarms[(day * 2654435761u >> 16) % ARRAY_COUNT(sSwarms)];
}

// (the gates' boards: GIMMICK_OBJ_ELBOARD_AddLastPlaceMessage) today's outbreak's zone, 0xFFFF none
u16 U3D_SwarmZone(void)
{
    const u16 *s = SwarmToday();
    return s ? s[0] : 0xFFFF;
}

static bool8 SwarmFill(u8 rate)
{
    const u16 *s = SwarmToday();
    u32 i;
    if (s == NULL || s[0] != U3D_CurrentZone() || Random() % 1000 > 400)
        return FALSE;
    for (i = 0; i < 12; i++)
        sMons[i].species = NationalPokedexNumToSpecies(s[1]), sMons[i].minLevel = 15, sMons[i].maxLevel = 55;
    sInfo.encounterRate = rate;
    sInfo.wildPokemon = sMons;
    return TRUE;
}

// The table underfoot (behavior: the tile's), its area (0 land, 1 water: wild_encounter.c WILD_AREA_*); FALSE none
bool8 U3D_WildInfo(u16 behavior, const struct WildPokemonInfo **info, u8 *area)
{
    const u8 *row = U3D_Vm_WildRow(U3D_CurrentZone(), U3D_Season());
    if (row == NULL)
        return FALSE;
    if (MetatileBehavior_IsLongGrass(behavior))   // (Black's dark grass: build_pack.py)
    {
        if (!Fill(row, DARK_GRASS, 12, row[1]))
            return FALSE;
        *area = 0;
    }
    else if (MetatileBehavior_IsLandWildEncounter(behavior))
    {
        if (row[0] && SwarmFill(row[0]))   // (Black's swarm: its normal grass only, encountType 0)
            ;
        else if (!Fill(row, GRASS, 12, row[0]))
            return FALSE;
        *area = 0;
    }
    else if (MetatileBehavior_IsWaterWildEncounter(behavior))
    {
        if (!Fill(row, SURF, 5, row[3]))
            return FALSE;
        *area = 1;
    }
    else
        return FALSE;
    *info = &sInfo;
    return TRUE;
}

// Black's other tables (field_encount.c's types): 2 rustling grass / a dust cloud / a bird's shadow (twelve slots), 4
// rippling water (five), 5 fishing, 6 fishing in rippling water (five each; Black's fishing odds 40, 40, 15, 4, 1:
// wild_encounter.c); FALSE when the zone has none (rate 0)
bool8 U3D_WildSpecial(u8 type, const struct WildPokemonInfo **info)
{
    static const u8 sAt[7] = {GRASS, DARK_GRASS, GRASS_SPECIAL, SURF, SURF_SPECIAL, FISHING, FISHING_SPECIAL};
    static const u8 sCount[7] = {12, 12, 12, 5, 5, 5, 5};
    const u8 *row = U3D_Vm_WildRow(U3D_CurrentZone(), U3D_Season());
    if (row == NULL || type >= 7 || !Fill(row, sAt[type], sCount[type], row[type]))
        return FALSE;
    *info = &sInfo;
    return TRUE;
}

// The zone's phenomena setting (Black's EncountData.effectKind, 0-2: ov021's table at 0x021D8E70): the steps before a
// roll and its odds in percent; FALSE when the zone has no encounter table
bool8 U3D_PhenomSetting(u8 *kind, u8 *steps, u8 *percent)
{
    static const u8 sSettings[3][2] = {{20, 10}, {20, 10}, {5, 15}};
    const u8 *row = U3D_Vm_WildRow(U3D_CurrentZone(), U3D_Season());
    u8 k;
    if (row == NULL)
        return FALSE;
    k = row[7] & 0x7F;
    if (k > 2)
        k = 0;
    *kind = k;
    *steps = sSettings[k][0];
    *percent = sSettings[k][1];
    return TRUE;
}
