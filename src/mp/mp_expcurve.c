// Moemon Co-op: the EXP CURVE (the user's design and decisions, 2026-11-14), on with LEVEL SCALING (OPTIONS > GAME >
// LEVEL SCALING > EXP CURVE: OFF / GENTLE / STRONG, the room host's online). Staying in one place doesn't grind a team
// far past it, and nobody is punished for it:
//
// - ANALYZE: the area's highest level WITHOUT level scaling: its wild tables' (grass, the water once SURF can be used
//   there; never fishing, ROCK SMASH or UNOVA's dark grass: gated, often far higher, ROUTE 1's is Lv32-35) and the
//   trainers met there (their own teams' strongest, noted as each battle is built: MP_ExpCurve_NoteTrainer). Every
//   region's own tables: gWildMonHeaders (HOENN, KANTO, JOHTO, SINNOH), UNOVA's by zone and season (u3d_wild.c).
// - The FRONTIER: the highest area the player has reached in the region (gRegionState.expFrontier, per region), never
//   lower than the last LEAGUE badge's level (MP_LevelScale_BadgeCap - 3): going back to an early route to catch
//   something costs nothing. The CEILING is the higher of the area and the frontier.
// - Each POKeMON on its own level: below the ceiling +5% a level under it (+100% at most: "a boosted amount"); at it,
//   the game's own EXP; above it x0.85 a level over (GENTLE) or x0.75 (STRONG), never under 10%. Applied after every
//   other bonus (LUCKY EGG, trades, buddy, TOGETHER, PASS POWERS, catch EXP), so stacking can't get round it.
// - CATCH-UP (replaces the badges' catch-up while this is on): a player whose team (BASE SCALING: its top three's
//   average, MP_BaseScaling) is behind a friend's gets +5% a level, +30% at most, toward the friend's level but never
//   past the ceiling: it helps those behind reach the area, never chase a friend who ground past it.
// - Off where the region's LEAGUE is beaten (ORRE too, post-game), in the facilities and wherever scaling is off.
#include <stdio.h>
#include "global.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "constants/metatile_behaviors.h"
#include "constants/moves.h"
#include "mp.h"
#include "mp_region.h"
#include "u3d.h"

#define BOOST_PER_LEVEL 5
#define BOOST_MAX       100
#define FLOOR_PERCENT   10
#define CATCHUP_PER     5
#define CATCHUP_MAX     30

bool8 MP_PartyCanUseFieldMove(u16 move);   // (mp_region.c)

static u8 sAreaWild;            // the map's wild tables' highest (ANALYZE), worked out as it loads
static u8 sAreaTrainers;        // the highest trainer team met on this map since it loaded
static u8 sAreaGroup = 0xFF, sAreaNum = 0xFF;

bool8 MP_ExpCurve_Active(void)
{
    if (!MP_LevelScaling() || MP_LevelScaleRule(MP_SCALE_EXP) == 0 || MP_Region_ClearedHere())
        return FALSE;
    if (InBattlePike() || InBattlePyramid_())
        return FALSE;
    return MP_Region_Current() < REGION_COUNT;
}

static u8 MaxOf(const struct WildPokemonInfo *info, u8 count)
{
    u8 i, best = 0;
    if (info == NULL || info->wildPokemon == NULL)
        return 0;
    for (i = 0; i < count; i++)
        if (info->wildPokemon[i].species != SPECIES_NONE)
        {
            u8 hi = info->wildPokemon[i].maxLevel > info->wildPokemon[i].minLevel ? info->wildPokemon[i].maxLevel
                                                                                  : info->wildPokemon[i].minLevel;
            if (hi > best)
                best = hi;
        }
    return best;
}

// ANALYZE: the map's wild POKeMON at their own levels
static u8 AnalyzeWild(void)
{
    u8 group = gSaveBlock1Ptr->location.mapGroup, num = gSaveBlock1Ptr->location.mapNum, best = 0, area, lv;
    // the water once SURF can be used here: its badge (the region's own; UNOVA needs none) and a POKeMON with SURF
    bool8 surf = MP_Region_HasFieldMoveBadge(4) && MP_PartyCanUseFieldMove(MOVE_SURF);
    u16 i;
    if (U3D_IsUnovaGroup(group))
    {
        static const u16 sKinds[] = {MB_TALL_GRASS, MB_POND_WATER};   // (dark grass left out: early routes keep late levels there)
        const struct WildPokemonInfo *info;
        for (i = 0; i < ARRAY_COUNT(sKinds); i++)
        {
            if (sKinds[i] == MB_POND_WATER && !surf)
                continue;
            if (U3D_WildInfo(sKinds[i], &info, &area) && (lv = MaxOf(info, area == 1 ? 5 : 12)) > best)
                best = lv;
        }
        return best;
    }
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        if (gWildMonHeaders[i].mapGroup == group && gWildMonHeaders[i].mapNum == num)
        {
            best = MaxOf(gWildMonHeaders[i].landMonsInfo, 12);
            if (surf && (lv = MaxOf(gWildMonHeaders[i].waterMonsInfo, 5)) > best)
                best = lv;
            break;
        }
    return best;
}

static u8 BadgeFloor(void)
{
    u8 b = MP_Region_ScaleBadges(), cap;
    if (b == 0)
        return 0;
    cap = MP_LevelScale_BadgeCap(b - 1);
    return cap > 3 ? cap - 3 : cap;
}

static void Refresh(void)
{
    u8 r = MP_Region_Current();
    if (sAreaGroup != gSaveBlock1Ptr->location.mapGroup || sAreaNum != gSaveBlock1Ptr->location.mapNum)
    {
        sAreaGroup = gSaveBlock1Ptr->location.mapGroup;
        sAreaNum = gSaveBlock1Ptr->location.mapNum;
        sAreaWild = AnalyzeWild();
        sAreaTrainers = 0;
    }
    if (r < REGION_COUNT && !MP_Region_ClearedHere())
    {
        u8 area = sAreaWild > sAreaTrainers ? sAreaWild : sAreaTrainers;
        if (area > gRegionState.expFrontier[r])
            gRegionState.expFrontier[r] = area;
    }
}

// mp_region.c MP_Region_GiveDevices (every map load): ANALYZE the new map, the frontier raised
void MP_ExpCurve_MapLoaded(void)
{
    Refresh();
}

// mp_levelscale.c MP_LevelScale_LogTrainer: a trainer's team as it's built, its strongest at its own level (badge cap
// at most: a rematch or a boss's margin never lifts the frontier past what the badges allow)
void MP_ExpCurve_NoteTrainer(u8 partyMax)
{
    Refresh();
    if (partyMax > sAreaTrainers)
        sAreaTrainers = partyMax;
    Refresh();
}

// The ceiling never passes the level the region's badges allow (LEVEL SCALING's own BADGE CAPS: 18 with none ... 65
// with eight): an area's tables reach far higher than the story there (KANTO's water is Lv34-36 from ROUTE 4 on,
// CERULEAN CAVE Lv54-62 beside CERULEAN), and a frontier raised by them gave a player who could SURF there, or who
// stepped into the cave, the whole +100% at Lv30 by NUGGET BRIDGE while friends there were cut to a tenth (the EXP
// audit, 2026-11-17)
static u8 Top(void)
{
    return MP_LevelScale_BadgeCap(MP_Region_ScaleBadges());
}

u8 MP_ExpCurve_Ceiling(void)
{
    u8 r = MP_Region_Current(), c, f, top;
    Refresh();
    c = sAreaWild > sAreaTrainers ? sAreaWild : sAreaTrainers;
    if (r < REGION_COUNT && gRegionState.expFrontier[r] > c)
        c = gRegionState.expFrontier[r];
    f = BadgeFloor();
    c = c > f ? c : f;
    top = Top();
    return c < top ? c : top;
}

// A POKeMON at LEVEL with CEILING: its EXP in percent (100: the game's own)
u16 MP_ExpCurve_Percent(u8 level, u8 ceiling)
{
    u32 pct = 100, i, keep;
    if (ceiling == 0 || level == ceiling)
        return 100;
    if (level < ceiling)
    {
        u32 up = (ceiling - level) * BOOST_PER_LEVEL;
        return 100 + (up > BOOST_MAX ? BOOST_MAX : up);
    }
    keep = MP_LevelScaleRule(MP_SCALE_EXP) == 2 ? 75 : 85;
    for (i = 0; i < (u32)(level - ceiling) && pct > FLOOR_PERCENT; i++)
        pct = pct * keep / 100;
    return pct < FLOOR_PERCENT ? FLOOR_PERCENT : pct;
}

// SCALED (the user, 2026-11-19: trainer battles still worth fighting, wild ones good EXP): no cut and no boost by the
// ceiling; a beaten POKeMON gives the EXP it would give at the level LEVEL SCALING set it to, 5 levels lower
bool8 MP_ExpCurve_Scaled(void)
{
    return MP_ExpCurve_Active() && MP_LevelScaleRule(MP_SCALE_EXP) == 3;
}

// battle_script_commands.c Cmd_getexp: the level a beaten POKeMON's EXP is worked out from
u8 MP_ExpCurve_FoeLevel(u8 level)
{
    if (!MP_ExpCurve_Scaled())
        return level;
    return level > 5 ? level - 5 : 1;
}

// battle_script_commands.c Cmd_getexp: EXP for a POKeMON at LEVEL (after every other bonus). *BOOSTED: it got more
u32 MP_ExpCurve_Apply(u32 exp, u8 level, u8 ceiling, bool8 *boosted)
{
    u16 pct;
    if (boosted)
        *boosted = FALSE;
    if (exp == 0 || !MP_ExpCurve_Active() || MP_ExpCurve_Scaled())
        return exp;
    pct = MP_ExpCurve_Percent(level, ceiling);
    if (pct == 100)
        return exp;
    if (pct > 100 && boosted)
        *boosted = TRUE;
    exp = exp * pct / 100;
    MPNet_Log("exp curve: Lv%d, ceiling %d: %d%%", level, ceiling, pct);
    return exp ? exp : 1;
}

// mp_together.c MP_CatchUpPercent: CATCH-UP toward a friend's team, never past the ceiling
u16 MP_ExpCurve_CatchUp(void)
{
    u8 i, best = 0, mine = MP_BaseScaling(), target;
    if (!MP_IsConnected() || gMpLocalId < 0)
        return 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState && gMpPlayers[i].state.top3 > best)
            best = gMpPlayers[i].state.top3;
    target = MP_ExpCurve_Ceiling();
    if (best < target)
        target = best;
    if (target <= mine)
        return 0;
    return (target - mine) * CATCHUP_PER > CATCHUP_MAX ? CATCHUP_MAX : (target - mine) * CATCHUP_PER;
}

// Tests: `mp expcurve`: the area, the frontier, the ceiling and what each party POKeMON would get
void MP_ExpCurve_Debug(void)
{
    u8 i, c = MP_ExpCurve_Ceiling(), r = MP_Region_Current();
    printf("[test] expcurve: active %d mode %d region %d badges %d wild %d trainers %d frontier %d floor %d top %d ceiling %d base %d catchup %d\n",
           MP_ExpCurve_Active(), MP_LevelScaleRule(MP_SCALE_EXP), r, MP_Region_ScaleBadges(), sAreaWild, sAreaTrainers,
           r < REGION_COUNT ? gRegionState.expFrontier[r] : 0, BadgeFloor(), Top(), c, MP_BaseScaling(), MP_ExpCurve_CatchUp());
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            printf("[test] expcurve: slot %d Lv%d -> %d%%\n", i, GetMonData(&gPlayerParty[i], MON_DATA_LEVEL),
                   MP_ExpCurve_Scaled() ? 100 : MP_ExpCurve_Percent(GetMonData(&gPlayerParty[i], MON_DATA_LEVEL), c));
}
