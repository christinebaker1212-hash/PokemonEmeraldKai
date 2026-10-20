// Moemon Co-op: small numbers kept next to the save file ("<save>.coop"),
// for things that belong to this save but not to the game itself: daily
// gifts sent, the play-together streak, rewards already claimed...
// One "key = value" per line.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"

extern char gPlatformSavePath[512];

#define MAX_ENTRIES 2048
#define KEEP_DAYS 14   // dated keys ("goal:20261020:0") older than this are dropped
#define KEY_MAX 47

struct Entry
{
    char key[KEY_MAX + 1];
    s32 value;
};

static struct Entry sEntries[MAX_ENTRIES];
static int sCount;
static char sLoadedFor[512];
static bool8 sLoaded;

s32 MP_DayNumber(s32 yyyymmdd);
s32 MP_Today(void);

// The date in a key like "goal:20261020:1" or "played:20261020" (0 = not a dated key)
static s32 KeyDate(const char *key)
{
    const char *p;
    for (p = key; *p; p++)
    {
        if ((p == key || p[-1] == ':') && p[0] == '2' && p[1] == '0')
        {
            int n = 0;
            while (p[n] >= '0' && p[n] <= '9')
                n++;
            if (n == 8 && (p[n] == ':' || p[n] == 0))
                return (s32)strtol(p, NULL, 10);
        }
    }
    return 0;
}

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.coop", gPlatformSavePath);
}

static void Load(void)
{
    FILE *f;
    char path[600], line[128];
    if (sLoaded && strcmp(sLoadedFor, gPlatformSavePath) == 0)
        return;
    sLoaded = TRUE;
    snprintf(sLoadedFor, sizeof(sLoadedFor), "%s", gPlatformSavePath);
    sCount = 0;
    Path(path, sizeof(path));
    f = fopen(path, "r");
    if (!f)
        return;
    while (fgets(line, sizeof(line), f) && sCount < MAX_ENTRIES)
    {
        char *eq = strchr(line, '='), *k = line, *end;
        if (line[0] == ';' || !eq)
            continue;
        *eq = 0;
        while (*k == ' ')
            k++;
        end = eq - 1;
        while (end >= k && (*end == ' ' || *end == '\t'))
            *end-- = 0;
        if (!*k || strlen(k) > KEY_MAX)
            continue;
        // Old dated keys (yesterday's goals, last month's gifts...) are dropped
        if (KeyDate(k) && MP_DayNumber(KeyDate(k)) < MP_DayNumber(MP_Today()) - KEEP_DAYS)
            continue;
        snprintf(sEntries[sCount].key, sizeof(sEntries[sCount].key), "%s", k);
        sEntries[sCount].value = (s32)strtol(eq + 1, NULL, 10);
        sCount++;
    }
    fclose(f);
}

static void Save(void)
{
    FILE *f;
    char path[600];
    int i;
    Path(path, sizeof(path));
    f = fopen(path, "w");
    if (!f)
        return;
    fprintf(f, "; Moemon Co-op data for this save file (gifts, streaks, rewards)\n");
    for (i = 0; i < sCount; i++)
        fprintf(f, "%s = %d\n", sEntries[i].key, (int)sEntries[i].value);
    fclose(f);
}

s32 MP_SaveData_Get(const char *key)
{
    int i;
    Load();
    for (i = 0; i < sCount; i++)
        if (strcmp(sEntries[i].key, key) == 0)
            return sEntries[i].value;
    return 0;
}

void MP_SaveData_Set(const char *key, s32 value)
{
    int i;
    Load();
    for (i = 0; i < sCount; i++)
        if (strcmp(sEntries[i].key, key) == 0)
            break;
    if (i == sCount)
    {
        if (sCount >= MAX_ENTRIES)
        {
            // Full: drop the oldest dated entry. Never an undated one (pulls,
            // pity, streak, achievements): losing those used to wipe pulls and
            // hand out achievement rewards again.
            int j, oldest = -1;
            for (j = 0; j < sCount; j++)
                if (KeyDate(sEntries[j].key) && (oldest < 0 || KeyDate(sEntries[j].key) < KeyDate(sEntries[oldest].key)))
                    oldest = j;
            if (oldest < 0)
                return;   // only permanent keys left: keep them
            memmove(&sEntries[oldest], &sEntries[oldest + 1], sizeof(sEntries[0]) * (sCount - oldest - 1));
            i = sCount = sCount - 1;
        }
        snprintf(sEntries[i].key, sizeof(sEntries[i].key), "%s", key);
        sCount++;
    }
    sEntries[i].value = value;
    Save();
}

void MP_SaveData_Add(const char *key, s32 delta)
{
    MP_SaveData_Set(key, MP_SaveData_Get(key) + delta);
}

// Dates for daily things. UTC, so friends in different countries share
// the same day. MOEMON_DEBUG_DATE=YYYYMMDD overrides it for testing.
#include <time.h>
s32 MP_Today(void)
{
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    if (getenv("MOEMON_DEBUG_DATE"))
        return (s32)atoi(getenv("MOEMON_DEBUG_DATE"));
    return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
}

// Days since 2000-01-01 for a YYYYMMDD date (no time zones involved)
s32 MP_DayNumber(s32 yyyymmdd)
{
    s32 y = yyyymmdd / 10000, m = (yyyymmdd / 100) % 100, d = yyyymmdd % 100;
    s32 era, yoe, doy, doe;
    y -= m <= 2;
    era = (y >= 0 ? y : y - 399) / 400;
    yoe = y - era * 400;
    doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468 - 10957;   // days since 1970, then since 2000
}

// Week number (weeks start on Monday), the same for everyone in the week
s32 MP_WeekNumber(void)
{
    // 2000-01-03 was a Monday (day 2)
    return (MP_DayNumber(MP_Today()) - 2) / 7;
}
