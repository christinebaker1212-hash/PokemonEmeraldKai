// Moemon Co-op: SKIP THE INTRO (offered by PROF. BIRCH in the new game's speech,
// main_menu.c). The player picks and names a starter there; NewGameInitData
// then sets the story up as it is right after the Route 103 rival battle, the
// POKéDEX from BIRCH, the POKé BALLS from the rival and the RUNNING SHOES from
// MOM. The game still starts in the moving truck, which drops the player off
// in front of their house (FLAG_MP_SKIP_TRUCK: InsideOfTruck / LittlerootTown
// scripts). The starter reads as met at Lv. 5 on ROUTE 101, as in the intro.
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "constants/opponents.h"
#include "constants/trainers.h"

static u16 sSpecies;
static u8 sNickname[POKEMON_NAME_LENGTH + 1];
static bool8 sHasNickname;

// From the speech: the starter (SPECIES_NONE = play the intro) and its nickname (NULL = none)
void MP_QuickStart_Set(u16 species, const u8 *nickname)
{
    sSpecies = species;
    sHasNickname = nickname != NULL;
    if (nickname)
        StringCopyN(sNickname, nickname, POKEMON_NAME_LENGTH + 1);
}

bool8 MP_QuickStart_Pending(void)
{
    return sSpecies != SPECIES_NONE;
}

u16 GetStarterPokemon(u16 chosenStarterId);
void SetUnlockedPokedexFlags(void);

// EXP the starter earns on the way: the wild ZIGZAGOON (Lv. 2) on Route 101 and
// the rival's Lv. 5 POKéMON on Route 103 (trainer battles give 1.5x)
static u32 IntroExp(u16 rivalSpecies)
{
    u32 wild = gSpeciesInfo[SPECIES_ZIGZAGOON].expYield * 2 / 7;
    u32 rival = gSpeciesInfo[rivalSpecies].expYield * 5 / 7;
    return wild + rival + rival / 2;
}

static void GiveStarter(u16 rivalSpecies)
{
    struct Pokemon mon;
    u16 dex = SpeciesToNationalPokedexNum(sSpecies);
    u8 growth = gSpeciesInfo[sSpecies].growthRate, level = 5;
    u32 exp = gExperienceTables[growth][5] + IntroExp(rivalSpecies);
    while (level < MAX_LEVEL && gExperienceTables[growth][level + 1] <= exp)
        level++;
    // Made at the level it would have reached, so it knows the moves it would have learned
    CreateMon(&mon, sSpecies, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    SetMonData(&mon, MON_DATA_EXP, &exp);
    {
        u8 metLocation = MAPSEC_ROUTE_101, metLevel = 5;   // where PROF. BIRCH's bag is
        SetMonData(&mon, MON_DATA_MET_LOCATION, &metLocation);
        SetMonData(&mon, MON_DATA_MET_LEVEL, &metLevel);
    }
    if (sHasNickname)
        SetMonData(&mon, MON_DATA_NICKNAME, sNickname);
    CalculateMonStats(&mon);
    GiveMonToPlayer(&mon);
    GetSetPokedexFlag(dex, FLAG_SET_SEEN);
    GetSetPokedexFlag(dex, FLAG_SET_CAUGHT);
}

// Called at the end of NewGameInitData. Everything the intro's scripts change
// on the way (InsideOfTruck, LittlerootTown and both houses, players_house.inc,
// Route101, ProfessorBirchsLab, Route103, OldaleTown), in the end state.
void MP_QuickStart_Apply(void)
{
    static const u16 sRivalTrainer[2][3] = {   // [player gender][starter]: the Route 103 battle
        {TRAINER_MAY_ROUTE_103_TREECKO, TRAINER_MAY_ROUTE_103_TORCHIC, TRAINER_MAY_ROUTE_103_MUDKIP},
        {TRAINER_BRENDAN_ROUTE_103_TREECKO, TRAINER_BRENDAN_ROUTE_103_TORCHIC, TRAINER_BRENDAN_ROUTE_103_MUDKIP},
    };
    static const u16 sRivalSpecies[3] = {SPECIES_TORCHIC, SPECIES_MUDKIP, SPECIES_TREECKO};   // has the edge on yours
    bool8 male = gSaveBlock2Ptr->playerGender == MALE;
    u16 starter = 0, i;

    if (!MP_QuickStart_Pending())
        return;
    for (i = 0; i < 3; i++)
        if (GetStarterPokemon(i) == sSpecies)
            starter = i;
    VarSet(VAR_STARTER_MON, starter);
    GiveStarter(sRivalSpecies[starter]);
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_ZIGZAGOON), FLAG_SET_SEEN);
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(sRivalSpecies[starter]), FLAG_SET_SEEN);

    // The moving truck (it still brings you: hidden after you step off), the house, the clock and the TV
    FlagSet(male ? FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK : FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK);
    FlagSet(FLAG_MP_SKIP_TRUCK);
    FlagSet(male ? FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM : FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM);
    FlagSet(male ? FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM : FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM);
    FlagSet(male ? FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING : FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE);
    FlagClear(FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN);
    FlagClear(FLAG_HIDE_MAP_NAME_POPUP);
    FlagSet(FLAG_SET_WALL_CLOCK);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_1);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_2);
    FlagSet(FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_BEDROOM_MOM);
    FlagSet(FLAG_SYS_TV_HOME);
    VarSet(VAR_LITTLEROOT_INTRO_STATE, 7);
    VarSet(male ? VAR_LITTLEROOT_HOUSES_STATE_BRENDAN : VAR_LITTLEROOT_HOUSES_STATE_MAY, 2);

    // Meeting the rival next door
    FlagSet(FLAG_MET_RIVAL_MOM);
    FlagSet(male ? FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_BEDROOM : FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM);
    FlagSet(FLAG_RIVAL_LEFT_FOR_ROUTE103);

    // Route 101: PROF. BIRCH saved, the starter from his bag
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_RESCUED_BIRCH);
    FlagSet(FLAG_HIDE_ROUTE_101_ZIGZAGOON);
    FlagSet(FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE);
    FlagSet(FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG);
    FlagClear(FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH);
    FlagClear(FLAG_HIDE_ROUTE_101_BOY);
    VarSet(VAR_ROUTE101_STATE, 3);

    // Route 103: the rival battle, then OLDALE on the way back
    FlagSet(FLAG_HIDE_ROUTE_103_RIVAL);
    FlagSet(FLAG_DEFEATED_RIVAL_ROUTE103);
    FlagSet(TRAINER_FLAGS_START + sRivalTrainer[male ? 0 : 1][starter]);
    FlagClear(FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_RIVAL);
    FlagSet(FLAG_HIDE_OLDALE_TOWN_RIVAL);
    VarSet(VAR_OLDALE_RIVAL_STATE, 2);
    VarSet(VAR_OLDALE_TOWN_STATE, 1);
    FlagSet(FLAG_VISITED_LITTLEROOT_TOWN);
    FlagSet(FLAG_VISITED_OLDALE_TOWN);
    AddMoney(&gSaveBlock1Ptr->money, 4 * 5 * 15);   // the rival's prize (class RIVAL, Lv. 5)

    // Back at the lab: the POKéDEX and 5 POKé BALLS
    FlagSet(FLAG_SYS_POKEDEX_GET);
    SetUnlockedPokedexFlags();
    FlagSet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH);
    VarSet(VAR_CABLE_CLUB_TUTORIAL_STATE, 1);
    AddBagItem(ITEM_POKE_BALL, 5);
    VarSet(VAR_BIRCH_LAB_STATE, 5);
    FlagSet(FLAG_ADVENTURE_STARTED);
    VarSet(VAR_LITTLEROOT_RIVAL_STATE, 4);

    // MOM's RUNNING SHOES outside the lab
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    FlagSet(FLAG_SYS_B_DASH);
    VarSet(VAR_LITTLEROOT_TOWN_STATE, 4);

    // Home is the place to wake up; the game starts in the truck (NewGameInitData already warped there)
    SetLastHealLocationWarp(male ? HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F : HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F);
    sSpecies = SPECIES_NONE;
}
