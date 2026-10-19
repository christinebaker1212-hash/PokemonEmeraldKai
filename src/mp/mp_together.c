// Moemon Co-op: playing side by side.
//
// TOGETHER: stay within about a screen of a friend and the TOGETHER meter
// fills (faster with more friends close by); wander off and it drains.
//   Lv1 (30%): EXP +3%   Lv2 (70%): EXP +6%, shiny odds x2
//   Lv3 (full): EXP +10%, shiny odds x2 (x3 with two or more friends close)
//   +2% EXP per extra friend close by. It never goes past +14%.
//
// CATCH-UP: a player with fewer badges than a friend online gets +10% EXP per
// badge behind (up to +30%), so new or casual friends catch up.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "constants/flags.h"

#define METER_MAX        1000
#define NEAR_X           8
#define NEAR_Y           6
#define FILL_PER_SEC     6     // one friend close: about three minutes to fill
#define FILL_PER_EXTRA   3
#define DRAIN_PER_SEC    10
#define TOGETHER_CAP     14    // EXP %
#define BUDDY_TOTAL_CAP  25    // buddy bonus + TOGETHER
#define CATCHUP_PER      10
#define CATCHUP_CAP      30

static u16 sMeter;
static u8 sLevel, sNear;
static u16 sTick;

u8 MP_Badges(void)
{
    u8 i, n = 0;
    for (i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            n++;
    return n;
}

static u8 CountNear(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 rx, ry;
        if (i == gMpLocalId || !gMpPlayers[i].present || !MP_GetRemoteMapPos(i, &rx, &ry))
            continue;
        if (abs(rx - me->currentCoords.x) <= NEAR_X && abs(ry - me->currentCoords.y) <= NEAR_Y)
            n++;
    }
    return n;
}

static u8 LevelFor(u16 meter)
{
    return meter >= METER_MAX ? 3 : meter >= 700 ? 2 : meter >= 300 ? 1 : 0;
}

void MP_Together_Update(void)
{
    u8 level;
    if (!MP_IsConnected() || gMpLocalId < 0)
    {
        sMeter = 0;
        sLevel = sNear = 0;
        return;
    }
    // Battles don't count against you
    if (MP_IsInOverworld() && gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT)
        sNear = CountNear();
    if (++sTick < 60)
        return;
    sTick = 0;
    if (!MP_IsInOverworld())
        return;
    if (sNear > 0)
        sMeter = min(METER_MAX, sMeter + FILL_PER_SEC + FILL_PER_EXTRA * (sNear - 1));
    else
        sMeter = sMeter > DRAIN_PER_SEC ? sMeter - DRAIN_PER_SEC : 0;
    level = LevelFor(sMeter);
    if (level > sLevel)
    {
        char msg[80];
        snprintf(msg, sizeof(msg), "TOGETHER Lv%d!\nEXP +%d%% while you stay close.", level, MP_Together_ExpPercent());
        sLevel = level;
        MP_Events_QueueToast(msg);
    }
    else
        sLevel = level;
}

u8 MP_Together_Level(void)
{
    return sLevel;
}

u8 MP_Together_Meter(void)
{
    return sMeter / 10;
}

u16 MP_Together_ExpPercent(void)
{
    static const u8 sBase[] = {0, 3, 6, 10};
    u16 pct;
    if (sLevel == 0)
        return 0;
    pct = sBase[sLevel] + (sNear > 1 ? 2 * (sNear - 1) : 0);
    return min(pct, TOGETHER_CAP);
}

// Extra tries at a shiny wild Pokemon
u8 MP_Together_ShinyRerolls(void)
{
    if (sLevel >= 3 && sNear >= 2)
        return 2;
    return sLevel >= 2 ? 1 : 0;
}

u16 MP_CatchUpPercent(void)
{
    u8 i, best = 0, mine = MP_Badges();
    if (!MP_IsConnected() || gMpLocalId < 0)
        return 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState && gMpPlayers[i].state.badges > best)
            best = gMpPlayers[i].state.badges;
    if (best <= mine)
        return 0;
    return min((best - mine) * CATCHUP_PER, CATCHUP_CAP);
}

// All the EXP bonuses together
u16 MP_ExpBonusPercent(void)
{
    return min(MP_BuddyBonusPercent() + MP_Together_ExpPercent(), BUDDY_TOTAL_CAP) + MP_CatchUpPercent();
}
