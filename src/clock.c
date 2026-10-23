#include <string.h>
#include "global.h"
#include "mp.h"
#include "berry.h"
#include "dewford_trend.h"
#include "event_data.h"
#include "field_specials.h"
#include "field_weather.h"
#include "main.h"
#include "lottery_corner.h"
#include "overworld.h"
#include "rtc.h"
#include "time_events.h"
#include "tv.h"
#include "wallclock.h"

static void UpdatePerDay(struct Time *localTime);
static void UpdatePerMinute(struct Time *localTime);
static void AddSecondsToTime(struct Time *t, s64 secs);

static void InitTimeBasedEvents(void)
{
    FlagSet(FLAG_SYS_CLOCK_SET);
    // Moemon Co-op: the game's time is the PC's clock (online the host's), so friends share
    // it: no offset from the time picked on the bedroom clock (which starts at the PC's time)
    memset(&gSaveBlock2Ptr->localTimeOffset, 0, sizeof(gSaveBlock2Ptr->localTimeOffset));
    RtcCalcLocalTime();
    gSaveBlock2Ptr->lastBerryTreeUpdate = gLocalTime;
    AddSecondsToTime(&gSaveBlock2Ptr->lastBerryTreeUpdate, -MP_ClockShift());   // Moemon Co-op: berries count on our own clock
    VarSet(VAR_DAYS, gLocalTime.days);
}

// Moemon Co-op: until the clock fix, the game's clock was stuck at day 0, 00:00. A save
// from then would see years pass at once (every Berry tree withering, thousands of daily
// updates): its clock becomes the PC's and Berry trees and days carry on from now.
#define CLOCK_REAL_DAYS 2000   // days since 2000: any real date is past this, the stuck clock never was
static void FixStuckClockSave(void)
{
    struct Time *off = &gSaveBlock2Ptr->localTimeOffset;
    u16 days;
    if (off->days == 0 && off->hours == 0 && off->minutes == 0 && off->seconds == 0
     && gSaveBlock2Ptr->lastBerryTreeUpdate.days >= CLOCK_REAL_DAYS)
        return;   // already on the PC's clock
    memset(off, 0, sizeof(*off));
    RtcCalcLocalTime();
    if (gLocalTime.days < CLOCK_REAL_DAYS)
        return;   // the clock can't be read: leave things as they are
    gSaveBlock2Ptr->lastBerryTreeUpdate = gLocalTime;
    AddSecondsToTime(&gSaveBlock2Ptr->lastBerryTreeUpdate, -MP_ClockShift());
    days = VarGet(VAR_DAYS);
    if (days < CLOCK_REAL_DAYS || days > gLocalTime.days)
        VarSet(VAR_DAYS, gLocalTime.days - 1);   // one new day for the daily events, not thousands
    MPNet_Log("clock: the game clock is the PC's now; Berry trees grow from here (day %d)", gLocalTime.days);
}

void DoTimeBasedEvents(void)
{
    // Moemon Co-op: SKIP THE INTRO marks the bedroom clock as set without setting it, which
    // left time-based events (Berry growth, daily events) off. Turn them on from now.
    if (!FlagGet(FLAG_SYS_CLOCK_SET) && FlagGet(FLAG_SET_WALL_CLOCK))
    {
        InitTimeBasedEvents();
        MPNet_Log("clock: time-based events were off (skipped intro): on from now");
    }
    if (FlagGet(FLAG_SYS_CLOCK_SET))
        FixStuckClockSave();
    if (FlagGet(FLAG_SYS_CLOCK_SET) && !InPokemonCenter())
    {
        RtcCalcLocalTime();
        UpdatePerDay(&gLocalTime);
        UpdatePerMinute(&gLocalTime);
    }
}

static void UpdatePerDay(struct Time *localTime)
{
    u16 *days = GetVarPointer(VAR_DAYS);
    u16 daysSince;

    if (*days != localTime->days && *days <= localTime->days)
    {
        daysSince = localTime->days - *days;
        ClearDailyFlags();
        UpdateDewfordTrendPerDay(daysSince);
        UpdateTVShowsPerDay(daysSince);
        UpdateWeatherPerDay(daysSince);
        UpdatePartyPokerusTime(daysSince);
        UpdateMirageRnd(daysSince);
        UpdateBirchState(daysSince);
        UpdateFrontierManiac(daysSince);
        UpdateFrontierGambler(daysSince);
        SetShoalItemFlag(daysSince);
        SetRandomLotteryNumber(daysSince);
        *days = localTime->days;
    }
}

// Moemon Co-op: a time moved by some seconds (they can be negative)
static void AddSecondsToTime(struct Time *t, s64 secs)
{
    s64 total = (s64)t->days * 86400 + t->hours * 3600 + t->minutes * 60 + t->seconds + secs;
    if (total < 0)
        total = 0;
    t->days = total / 86400;
    t->hours = (total / 3600) % 24;
    t->minutes = (total / 60) % 60;
    t->seconds = total % 60;
}

static void UpdatePerMinute(struct Time *localTime)
{
    struct Time difference;
    struct Time ownTime = *localTime;
    int minutes;

    // Moemon Co-op: online the clock is the room host's (mp_main.c), which can be hours
    // off ours. Berry trees grow by the time that really passed, so they count on our own
    // clock: joining or leaving a host never grows (or stops) them by the difference.
    AddSecondsToTime(&ownTime, -MP_ClockShift());
    CalcTimeDifference(&difference, &gSaveBlock2Ptr->lastBerryTreeUpdate, &ownTime);
    minutes = 24 * 60 * difference.days + 60 * difference.hours + difference.minutes;
    if (minutes != 0)
    {
        if (minutes >= 0)
        {
            BerryTreeTimeUpdate(minutes);
            gSaveBlock2Ptr->lastBerryTreeUpdate = ownTime;
        }
    }
}

static void ReturnFromStartWallClock(void)
{
    InitTimeBasedEvents();
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void StartWallClock(void)
{
    SetMainCallback2(CB2_StartWallClock);
    gMain.savedCallback = ReturnFromStartWallClock;
}
