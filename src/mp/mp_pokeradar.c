// Moemon Kai: the POKé RADAR (ITEM_POKE_RADAR, a key item from SANDGEM's LAB after SINNOH's LEAGUE; it works in every
// region's tall grass). As Platinum's: used in tall grass (not on a bike or in the water) it makes up to four patches of
// grass shake, one at each distance 1-4 from the player; a step into one is a wild battle. Beaten or caught, new
// patches shake at once, and a step into one usually brings the same POKéMON again: a chain, which grows the odds of a
// patch that sparkles (a shiny POKéMON) up to 1 in 200 at a chain of 40. The chain ends when another POKéMON comes,
// the player runs or loses, leaves the map, gets on a bike, or walks 12 steps without stepping into a patch; a new use
// waits for the battery (50 steps). Its POKéMON: a grass slot's, Platinum's radar ones in slots 4, 5, 10 and 11 in
// SINNOH (s3d_wild.c Sinnoh_RadarSlot). The POKéTCH's TRAINER COUNTER shows the chain and the best three
// (MP_Poketch_RadarChain).
#include <stdlib.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "battle.h"
#include "battle_setup.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "random.h"
#include "sound.h"
#include "string_util.h"
#include "palette.h"
#include "main.h"
#include "pokemon.h"
#include "script.h"
#include "constants/battle.h"
#include "constants/field_effects.h"
#include "constants/game_stat.h"
#include "constants/songs.h"
#include "constants/species.h"

#define PATCHES       4
#define BATTERY_STEPS 50
#define WANDER_STEPS  12
#define MAX_CHAIN     999

u16 MP_RadarWildMon(u16 chain, bool8 shiny);               // (wild_encounter.c)
void MP_Poketch_RadarChain(u16 species, u16 chain, bool8 ended);   // (src/sinnoh/poketch.c)
u16 MP_LandSlotSpecies(u8 index);                           // (wild_encounter.c)

static struct
{
    bool8 shown;          // patches shaking
    bool8 inBattle;       // a patch's battle is on
    bool8 refresh;        // new patches when the field is back (after a battle of the chain)
    u8 group, num;        // where
    s16 x[PATCHES], y[PATCHES];
    u8 kind[PATCHES];     // 0 none, 1 shaking, 2 sparkling (shiny)
    bool8 same[PATCHES];  // brings the chain's POKéMON
    u16 chain, species;
    u8 wander;            // steps since the patches came
    u8 battlePatch;
    u32 charged;          // the step count when the battery was used
    bool8 used;           // (the battery has been used this session)
    u32 timer;
    s32 debugShiny;       // (tests: `mp radar shiny`: the next patches all sparkle)
} sR;

static void Stop(bool8 endChain)
{
    if (endChain && sR.chain > 0)
    {
        MP_Poketch_RadarChain(sR.species, sR.chain, TRUE);
        MPNet_Log("radar: chain of %d %d ended", sR.chain, sR.species);
        sR.chain = 0;
        sR.species = SPECIES_NONE;
    }
    sR.shown = FALSE;
    sR.refresh = FALSE;
}

u16 MP_PokeRadar_Chain(u16 *species)
{
    if (species != NULL)
        *species = sR.species;
    return sR.chain;
}

static bool8 OnBikeOrWater(void)
{
    return TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE | PLAYER_AVATAR_FLAG_SURFING
                                 | PLAYER_AVATAR_FLAG_UNDERWATER);
}

static bool8 GrassAt(s16 x, s16 y)
{
    return MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x, y)) && !MapGridGetCollisionAt(x, y);
}

static u32 BatteryLeft(void)
{
    u32 steps = GetGameStat(GAME_STAT_STEPS) - sR.charged;
    return !sR.used || steps >= BATTERY_STEPS ? 0 : BATTERY_STEPS - steps;
}

// Why it can't be used now (NULL: it can)
const u8 *MP_PokeRadar_CantUse(void)
{
    static const u8 sBike[] = _("The POKé RADAR can't be used\nwhile riding.$");
    static const u8 sGrass[] = _("The POKé RADAR is used in tall\ngrass, where MOéMON hide.$");
    static const u8 sBattery1[] = _("The battery is charging. It needs\n");
    static const u8 sBattery2[] = _(" more steps.$");
    static u8 sBattery[80];
    s16 x, y;
    if (OnBikeOrWater())
        return sBike;
    PlayerGetDestCoords(&x, &y);
    if (!MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x, y)) || MP_LandSlotSpecies(0) == SPECIES_NONE)
        return sGrass;
    if (sR.chain == 0 && BatteryLeft() > 0)
    {
        u8 *s = StringCopy(sBattery, sBattery1);
        s = ConvertIntToDecimalStringN(s, BatteryLeft(), STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(s, sBattery2);
        return sBattery;
    }
    return NULL;
}

// Odds of a sparkling patch at chain C (Platinum's): round(65535 / (8200 - 200 C)) in 65536, C up to 40
static bool8 ShinyPatch(void)
{
    u32 c = sR.chain > 40 ? 40 : sR.chain;
    if (sR.debugShiny)
        return TRUE;
    return (Random() & 0xFFFF) < (65535 + (8200 - 200 * c) / 2) / (8200 - 200 * c);
}

// New patches around the player: one at each distance 1-4 (a random tile of that ring that's tall grass)
static void NewPatches(void)
{
    s16 px, py;
    u32 r, k, n = 0;
    PlayerGetDestCoords(&px, &py);
    for (r = 1; r <= PATCHES; r++)
    {
        u32 i = r - 1;
        sR.kind[i] = 0;
        for (k = 0; k < 16; k++)
        {
            u32 side = Random() % 4, t = Random() % (2 * r + 1);
            s16 x = px + (side == 0 ? -(s32)r : side == 1 ? (s32)r : (s32)t - (s32)r);
            s16 y = py + (side == 2 ? -(s32)r : side == 3 ? (s32)r : (s32)t - (s32)r);
            if (GrassAt(x, y))
            {
                sR.x[i] = x, sR.y[i] = y;
                sR.kind[i] = ShinyPatch() ? 2 : 1;
                // (the farther the patch, the likelier the chain's POKéMON: Platinum's rings)
                sR.same[i] = sR.chain > 0 && (Random() % 100) < 58 + 10 * r;
                n++;
                break;
            }
        }
    }
    sR.debugShiny = FALSE;
    sR.group = gSaveBlock1Ptr->location.mapGroup;
    sR.num = gSaveBlock1Ptr->location.mapNum;
    sR.shown = n > 0;
    sR.wander = 0;
    sR.timer = 0;
    MPNet_Log("radar: %d patches (chain %d)%s%s", n, sR.chain, sR.kind[0] == 2 || sR.kind[1] == 2 || sR.kind[2] == 2
              || sR.kind[3] == 2 ? " sparkling" : "", n ? "" : ": nothing shook");
    if (n == 0)
        Stop(TRUE);
}

void MP_PokeRadar_Start(void)
{
    if (sR.chain == 0)
    {
        sR.charged = GetGameStat(GAME_STAT_STEPS);
        sR.used = TRUE;
    }
    PlaySE(SE_M_DETECT);
    NewPatches();
}

// A step: 0 nothing to do with the radar, 1 into a patch (its battle starts), 2 the patches still shake (no other
// encounter in the grass)
u8 MP_PokeRadar_Step(void)
{
    s16 x, y;
    u32 i;
    if (!sR.shown)
        return 0;
    PlayerGetDestCoords(&x, &y);
    for (i = 0; i < PATCHES; i++)
    {
        if (sR.kind[i] == 0 || sR.x[i] != x || sR.y[i] != y)
            continue;
        {
            u16 want = sR.chain > 0 && sR.same[i] ? sR.species : SPECIES_NONE;
            u16 got = MP_RadarWildMon(want, sR.kind[i] == 2);
            if (got == SPECIES_NONE)
                break;
            sR.battlePatch = i;
            sR.shown = FALSE;
            sR.inBattle = TRUE;
            if (sR.chain > 0 && got != sR.species)
                Stop(TRUE);   // (another POKéMON: the chain is over, this one may start a new one)
            MPNet_Log("radar: patch %d (%s): species %d shiny %d", i + 1, sR.kind[i] == 2 ? "sparkling" : "shaking", got,
                      IsMonShiny(&gEnemyParty[0]));
            BattleSetup_StartWildBattle();
            return 1;
        }
    }
    if (++sR.wander >= WANDER_STEPS)
    {
        Stop(TRUE);
        return 0;
    }
    return 2;
}

// The battle's end (battle_setup.c CB2_EndWildBattle): beaten or caught, the chain grows and new patches come
void MP_PokeRadar_BattleEnd(u8 outcome)
{
    u16 species;
    if (!sR.inBattle)
        return;
    sR.inBattle = FALSE;
    species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
    if ((outcome == B_OUTCOME_WON || outcome == B_OUTCOME_CAUGHT) && (sR.chain == 0 || species == sR.species))
    {
        sR.species = species;
        if (sR.chain < MAX_CHAIN)
            sR.chain++;
        MP_Poketch_RadarChain(sR.species, sR.chain, FALSE);
        sR.refresh = TRUE;
        MPNet_Log("radar: chain %d of %d", sR.chain, species);
    }
    else
        Stop(TRUE);
}

// Each overworld frame: the patches rustle (sparkling ones glitter too); the chain ends off the map or on a bike
void MP_PokeRadar_Update(void)
{
    u32 i;
    if (!sR.shown && !sR.refresh)
        return;
    if (sR.group != gSaveBlock1Ptr->location.mapGroup || sR.num != gSaveBlock1Ptr->location.mapNum || OnBikeOrWater())
    {
        Stop(TRUE);
        return;
    }
    if (sR.refresh)
    {
        if (gMain.callback2 != CB2_Overworld || ArePlayerFieldControlsLocked() || gPaletteFade.active)
            return;
        sR.refresh = FALSE;
        PlaySE(SE_M_DETECT);
        NewPatches();
        return;
    }
    sR.timer++;
    for (i = 0; i < PATCHES; i++)
    {
        u32 phase = (sR.timer + i * 11) % (sR.kind[i] == 2 ? 24 : 40);
        if (sR.kind[i] == 0 || phase != 0)
            continue;
        gFieldEffectArguments[0] = sR.x[i];
        gFieldEffectArguments[1] = sR.y[i];
        gFieldEffectArguments[2] = gObjectEvents[gPlayerAvatar.objectEventId].previousElevation;
        gFieldEffectArguments[3] = 2;
        gFieldEffectArguments[4] = gObjectEvents[gPlayerAvatar.objectEventId].localId << 8
                                 | gObjectEvents[gPlayerAvatar.objectEventId].mapNum;
        gFieldEffectArguments[5] = gObjectEvents[gPlayerAvatar.objectEventId].mapGroup;
        gFieldEffectArguments[6] = (u8)gSaveBlock1Ptr->location.mapNum << 8 | (u8)gSaveBlock1Ptr->location.mapGroup;
        gFieldEffectArguments[7] = FALSE;
        FieldEffectStart(FLDEFF_TALL_GRASS);
        if (sR.kind[i] == 2)
        {
            gFieldEffectArguments[0] = sR.x[i] - MAP_OFFSET;
            gFieldEffectArguments[1] = sR.y[i] - MAP_OFFSET;
            gFieldEffectArguments[2] = 2;
            FieldEffectStart(FLDEFF_SPARKLE);
        }
    }
}

// (tests: `mp radar ...`: "shiny" the next patches sparkle; "where" each patch; "charge" a full battery)
void MP_PokeRadar_Debug(const char *arg)
{
    u32 i;
    if (strcmp(arg, "shiny") == 0)
        sR.debugShiny = TRUE;
    else if (strcmp(arg, "charge") == 0)
        sR.used = FALSE;
    else if (strncmp(arg, "near ", 5) == 0)
    {
        // near D: the nearest patch moves next to the player (D 0 up, 1 down, 2 left, 3 right), the chain's POKéMON
        s16 x, y, d = atoi(arg + 5);
        PlayerGetDestCoords(&x, &y);
        sR.x[0] = x + (d == 2 ? -1 : d == 3 ? 1 : 0);
        sR.y[0] = y + (d == 0 ? -1 : d == 1 ? 1 : 0);
        if (sR.kind[0] == 0)
            sR.kind[0] = 1;
        sR.same[0] = TRUE;
        sR.shown = TRUE;
    }
    else if (strcmp(arg, "find") == 0)
    {
        // the nearest tall grass with tall grass all around it (map tiles, as `mp warp` takes them)
        s16 x, y, r, dx, dy;
        PlayerGetDestCoords(&x, &y);
        for (r = 0; r < 60; r++)
            for (dy = -r; dy <= r; dy++)
                for (dx = -r; dx <= r; dx++)
                {
                    s16 k, ok = TRUE;
                    for (k = 0; k < 25 && ok; k++)
                        ok = GrassAt(x + dx + k % 5 - 2, y + dy + k / 5 - 2);
                    if (ok)
                    {
                        printf("[test] radar: grass at %d,%d\n", x + dx - MAP_OFFSET, y + dy - MAP_OFFSET);
                        return;
                    }
                }
        printf("[test] radar: no grass\n");
    }
    else
    {
        s16 x, y;
        PlayerGetDestCoords(&x, &y);
        printf("[test] radar: shown %d chain %d species %d player %d,%d", sR.shown, sR.chain, sR.species, x, y);
        for (i = 0; i < PATCHES; i++)
            if (sR.kind[i])
                printf(" patch %d,%d", sR.x[i] - x, sR.y[i] - y);
        printf("\n");
    }
}
