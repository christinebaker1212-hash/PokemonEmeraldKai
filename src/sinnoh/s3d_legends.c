#include <string.h>
// Moemon Kai: SINNOH's legendaries standing on the map (tools/sinnoh/legends.py, src/data/sinnoh_legends.h): on entering
// their map (special Sinnoh_LegendsRefresh) one is shown unless it was caught or beaten, or its time hasn't come:
// UXIE, AZELF, MESPRIT and GIRATINA from the 7th BADGE (when Platinum's story gives them), the others once SINNOH's
// LEAGUE is beaten
#include <stdlib.h>
#include "global.h"
#include "event_data.h"
#include "mp_region.h"
#include "constants/species.h"
#include "constants/map_groups.h"
#include "fieldmap.h"
#include "rtc.h"
#include "pokemon.h"
#include "../data/sinnoh_legends.h"
#include "../data/sinnoh_story_people.h"
#include "s3d.h"
#include "pokemon_icon.h"
#include "sound.h"
#include "mp.h"
#include "constants/songs.h"

#define FRIDAYS 0xFE   // (tools/sinnoh/legends.py)

// The PC's calendar (the room host's online): day 1 is 1 January 2000, a Saturday
static bool8 IsFriday(void)
{
    if (getenv("MOEMON_TEST_FRIDAY") != NULL)   // (tests)
        return atoi(getenv("MOEMON_TEST_FRIDAY")) != 0;
    return (RtcGetLocalDayCount() + 5) % 7 == 5;
}

u16 Sinnoh_FridayMet(void)
{
    VarSet(VAR_SINNOH_FRIDAY_DAY, RtcGetLocalDayCount() + 1);
    return 0;
}

u16 Sinnoh_LegendsRefresh(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSinnohLegends); i++)
    {
        bool8 shown;
        if (sSinnohLegends[i].group != gSaveBlock1Ptr->location.mapGroup || sSinnohLegends[i].num != gSaveBlock1Ptr->location.mapNum)
            continue;
        if (sSinnohLegends[i].badges == FRIDAYS)
        {
            // (DRIFLOON: met once a Friday. Its battle sets its "done" flag and the day (Sinnoh_FridayMet); a later
            // day clears them)
            if (FlagGet(sSinnohLegends[i].doneFlag) && VarGet(VAR_SINNOH_FRIDAY_DAY) != RtcGetLocalDayCount() + 1)
            {
                FlagClear(sSinnohLegends[i].doneFlag);
                VarSet(VAR_SINNOH_FRIDAY_DAY, 0);
            }
        }
        shown = !FlagGet(sSinnohLegends[i].doneFlag)
             && (sSinnohLegends[i].species == SPECIES_NONE || MP_Gen_Allowed(sSinnohLegends[i].species))
             && (sSinnohLegends[i].badges == 0xFF ? MP_Region_Cleared(REGION_SINNOH)
               : sSinnohLegends[i].badges == FRIDAYS ? IsFriday() && MP_Region_BadgeCount(REGION_SINNOH) >= 1
                                                      : MP_Region_BadgeCount(REGION_SINNOH) >= sSinnohLegends[i].badges);
        if (shown)
            FlagClear(sSinnohLegends[i].objFlag);
        else
            FlagSet(sSinnohLegends[i].objFlag);
    }
    return 0;
}

// (s3d_roamer.c) MESPRIT's or CRESSELIA's object done with (roaming) or back (Platinum's HALL OF FAME)
void Sinnoh_LegendDone(u16 species, bool8 done)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSinnohLegends); i++)
        if (sSinnohLegends[i].species == species)
        {
            if (done)
                FlagSet(sSinnohLegends[i].doneFlag);
            else
                FlagClear(sSinnohLegends[i].doneFlag);
        }
}

// SINNOH's story people (tools/sinnoh/story_people.py SHOW): each hide flag set or cleared by where the story is, as
// a SINNOH map loads (overworld.c, before the map's own on-transition script and before its people are placed)
void Sinnoh_StoryRefresh(void)
{
    u32 i;
    u8 badges, hour;
    bool8 night;
    if (!S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup))
        return;
    if (FlagGet(FLAG_SINNOH_PLATINUM_STORY))
        return;   // (Platinum's own scripts show and hide the story's people: tools/sinnoh/gen_scripts.py)
    badges = MP_Region_Cleared(REGION_SINNOH) ? 9 : MP_Region_BadgeCount(REGION_SINNOH);
    RtcCalcLocalTime();
    hour = gLocalTime.hours;
    night = hour >= 20 || hour < 4;   // (Platinum's night)
    for (i = 0; i < ARRAY_COUNT(sSinnohStoryPeople); i++)
    {
        const struct SinnohStoryPerson *p = &sSinnohStoryPeople[i];
        bool8 shown = badges >= p->from && badges < p->before
                   && (p->need == 0 || FlagGet(p->need)) && (p->unless == 0 || !FlagGet(p->unless))
                   && (p->time == 0 || (p->time == 2) == night);
        if (shown)
            FlagClear(p->flag);
        else
            FlagSet(p->flag);
    }
}

// The GREAT MARSH (tools/sinnoh/gen_maps.py marsh_merge): the player stays on the 6th area's map for the whole marsh,
// so the wild Pokemon are the area's under the player's feet (Platinum's blocks: areas 1-2 / 3-4 / 5-6 from the
// top, west then east; the border's blocks count as the nearest area)
static bool8 InGreatMarsh(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    return map >= MAP_SINNOH_GREAT_MARSH1 && map <= MAP_SINNOH_GREAT_MARSH6;
}

u16 Sinnoh_InGreatMarsh(void)
{
    return InGreatMarsh();
}

// The map whose wild Pokemon are met where the player stands (FALSE: not in the GREAT MARSH)
bool8 Sinnoh_GreatMarshWildMap(u8 *group, u8 *num)
{
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 bx, bz;
    u16 map;
    if (!InGreatMarsh())
        return FALSE;
    bx = (player->currentCoords.x - MAP_OFFSET) / 32;
    bz = (player->currentCoords.y - MAP_OFFSET) / 32;
    bx = bx <= 1 ? 0 : 1;
    bz = bz <= 1 ? 0 : bz >= 3 ? 2 : 1;
    map = MAP_SINNOH_GREAT_MARSH1 + bz * 2 + bx;
    *group = map >> 8;
    *num = map & 0xFF;
    return TRUE;
}

// Platinum's daily Pokemon of the GREAT MARSH (its SAFARI GAME only): each area has one a day in its 5 percent slots
// (6 and 7), one of 32 picks; after SINNOH's LEAGUE (Platinum: the NATIONAL POKeDEX) the second list
static const u16 sMarshDaily[2][32] = {
    {
        SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_CARNIVINE,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_WOOPER,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_WOOPER,
        SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_SKORUPI, SPECIES_CARNIVINE,
        SPECIES_YANMA, SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER,
        SPECIES_WOOPER, SPECIES_TROPIUS, SPECIES_WOOPER, SPECIES_WOOPER,
        SPECIES_SKORUPI, SPECIES_CARNIVINE, SPECIES_CROAGUNK, SPECIES_TANGELA,
    },
    {
        SPECIES_TOXICROAK, SPECIES_KECLEON, SPECIES_KECLEON, SPECIES_CARNIVINE,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_DRAPION,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_KANGASKHAN,
        SPECIES_PARAS, SPECIES_DRAPION, SPECIES_EXEGGCUTE, SPECIES_EXEGGCUTE,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_SKORUPI, SPECIES_CARNIVINE,
        SPECIES_YANMA, SPECIES_SHROOMISH, SPECIES_PARAS, SPECIES_KANGASKHAN,
        SPECIES_GULPIN, SPECIES_TROPIUS, SPECIES_GULPIN, SPECIES_SHROOMISH,
        SPECIES_SKORUPI, SPECIES_CARNIVINE, SPECIES_CROAGUNK, SPECIES_TANGELA,
    },
};

// Today's pick for area AREA (0-5): the day and the player's Trainer ID, so each save has its own days
u16 Sinnoh_GreatMarshDailySpecies(u8 area)
{
    u32 seed = RtcGetLocalDayCount() * 2654435761u ^ (gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8
             | gSaveBlock2Ptr->playerTrainerId[2] << 16 | (u32)gSaveBlock2Ptr->playerTrainerId[3] << 24);
    seed ^= seed >> 15;
    seed *= 0x2C1B3C6Du;
    seed ^= seed >> 12;
    return sMarshDaily[MP_Region_Cleared(REGION_SINNOH) ? 1 : 0][(seed >> (5 * area)) & 31];
}

// The SAFARI GAME's land Pokemon for slot INDEX (SPECIES: the table's): today's pick in the 5 percent slots
u16 Sinnoh_GreatMarshDaily(u16 species, u8 index)
{
    u8 group, num;
    u16 daily;
    if ((index != 6 && index != 7) || !FlagGet(FLAG_SYS_SAFARI_MODE) || !Sinnoh_GreatMarshWildMap(&group, &num))
        return species;
    daily = Sinnoh_GreatMarshDailySpecies(num - MAP_NUM(MAP_SINNOH_GREAT_MARSH1));
    return MP_Gen_Allowed(daily) ? daily : species;
}

// The REGIS' ruins and the OLD CHATEAU's ROTOM (data/maps/Sinnoh_*Ruins, Sinnoh_OldChateauBackMiddleWestRoom)
u16 Sinnoh_LeagueDone(void)
{
    return MP_Region_Cleared(REGION_SINNOH);
}

u16 Sinnoh_PartyHasRegigigas(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_REGIGIGAS)
            return TRUE;
    return FALSE;
}

// Platinum's ActivateRegiRuinsDot (overlay005): the dot under the player sets its bit of the ruins' state; all seven:
// RUINS_STATE_ACTIVATED_ALL_DOTS (260). Its tables of the dots, Platinum's tiles of each room
void Sinnoh_RegiDot(u16 var, u16 type)
{
    static const s8 sIron[7][2] = {{4, 7}, {5, 5}, {5, 9}, {7, 7}, {9, 5}, {9, 9}, {10, 7}};
    static const s8 sIceberg[7][2] = {{3, 7}, {5, 7}, {7, 5}, {7, 7}, {7, 9}, {9, 7}, {11, 7}};
    static const s8 sRockPeak[7][2] = {{5, 5}, {5, 7}, {5, 9}, {7, 7}, {9, 5}, {9, 7}, {9, 9}};
    const s8 (*dots)[2] = type == 588 ? sIron : type == 590 ? sIceberg : sRockPeak;
    s32 cx = 0, cz = 0, x, z, i;
    u16 state = VarGet(var);
    S3D_MapCells(&cx, &cz);
    x = gSaveBlock1Ptr->pos.x + cx * 32;
    z = gSaveBlock1Ptr->pos.y + cz * 32;
    for (i = 0; i < 7; i++)
        if (x == dots[i][0] && z == dots[i][1])
        {
            PlaySE(SE_M_CHARM);
            state |= 1 << i;
            break;
        }
    if (state == 0x7F)
        state = 260;
    VarSet(var, state);
    MPNet_Log("sinnoh: regi dot %d,%d: state %u", (int)x, (int)z, state);
}

u16 Sinnoh_UnownFormsSeen(void);

// The UNOWN letters seen (Platinum's POKéDEX keeps them: GetUnownFormsSeenCount, the RUIN MANIAC's cave on ROUTE
// 214 grows with them); "<save>.region" chunk UNOF (mp_region.c)
static u32 sUnownSeen;

void Sinnoh_UnownSeen(u32 personality)
{
    u16 letter = GetUnownLetterByPersonality(personality);
    if (letter < 28 && !((sUnownSeen >> letter) & 1))
    {
        sUnownSeen |= 1u << letter;
        MPNet_Log("sinnoh: UNOWN %u seen (%u letters)", letter, Sinnoh_UnownFormsSeen());
    }
}

// (bit 31: the POKeDEX's form viewer, ROUTE 218's gate scientist's upgrade: Platinum's TurnOnPokedexFormDetection)
#define FORM_VIEWER_ON (1u << 31)
#define UNOWN_LETTERS  ((1u << 28) - 1)

u16 Sinnoh_UnownFormsSeen(void)
{
    u16 n = 0;
    u32 m;
    for (m = sUnownSeen & UNOWN_LETTERS; m; m &= m - 1)
        n++;
    return n;
}

u32 Sinnoh_UnownLetters(void)
{
    return sUnownSeen & UNOWN_LETTERS;
}

bool8 Sinnoh_FormViewerOn(void)
{
    return (sUnownSeen & FORM_VIEWER_ON) != 0;
}

void Sinnoh_FormViewerEnable(void)
{
    sUnownSeen |= FORM_VIEWER_ON;
    MPNet_Log("sinnoh: the POKeDEX's form viewer is on");
}

const void *Sinnoh_UnownData(u32 *size)
{
    *size = sizeof(sUnownSeen);
    return &sUnownSeen;
}

void Sinnoh_UnownLoad(const void *data, u32 size)
{
    sUnownSeen = 0;
    if (size == sizeof(sUnownSeen))
        memcpy(&sUnownSeen, data, size);
}

void Sinnoh_UnownReset(void)
{
    sUnownSeen = 0;
}

u16 Sinnoh_IsNight(void)   // (SINNOH's night: 20:00 to 4:00, as its night music)
{
    RtcCalcLocalTime();
    return gLocalTime.hours >= 20 || gLocalTime.hours < 4;
}

// BARRY's team in the story's battles (tools/sinnoh/legends.py STORY): Platinum's three versions (its CHIMCHAR,
// PIPLUP or TURTWIG line), by the player's Trainer ID
u16 Sinnoh_RivalVersion(void)
{
    return (gSaveBlock2Ptr->playerTrainerId[0] + gSaveBlock2Ptr->playerTrainerId[1]) % 3;
}
