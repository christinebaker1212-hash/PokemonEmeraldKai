// Moemon Kai: UNOVA's wild POKéMON (docs/UNOVA_HANDOFF.md): Black's own encounter tables (a/1/2/6 in UNOVA's
// scripts pack, by the zone's encounter id; a season's row where the zone has four), read as the player steps and fed
// to the game's own encounter step (wild_encounter.c StandardWildEncounter's UNOVA line): grass and dark grass from
// their twelve slots (Black's slot odds are the game's: 20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1), surfing from its
// five (60, 30, 5, 4, 1), each at its own rate. Species are Black's national numbers. Its own code.
#include "global.h"
#include "metatile_behavior.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "u3d.h"
#include "constants/species.h"

const u8 *U3D_Vm_WildRow(u16 zone, u8 season);

#define GRASS 8
#define DARK_GRASS (GRASS + 12 * 4)
#define SURF (8 + 36 * 4)

static struct WildPokemon sMons[12];
static struct WildPokemonInfo sInfo;

static bool8 Fill(const u8 *row, u32 at, u32 count, u8 rate)
{
    u32 i;
    if (rate == 0)
        return FALSE;
    for (i = 0; i < count; i++)
    {
        const u8 *s = row + at + 4 * i;
        u16 national = (s[0] | s[1] << 8) & 0x7FF;
        sMons[i].species = national ? NationalPokedexNumToSpecies(national) : SPECIES_NONE;
        sMons[i].minLevel = s[2] ? s[2] : 1;
        sMons[i].maxLevel = s[3] >= sMons[i].minLevel ? s[3] : sMons[i].minLevel;
    }
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
        if (!Fill(row, GRASS, 12, row[0]))
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
