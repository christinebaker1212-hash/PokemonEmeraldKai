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

#define MAX_ENTRIES 256
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
            // Full: drop the oldest entry (the file is in the order keys were added)
            memmove(&sEntries[0], &sEntries[1], sizeof(sEntries[0]) * (MAX_ENTRIES - 1));
            i = sCount = MAX_ENTRIES - 1;
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

// Today's date as YYYYMMDD (the PC's local time), for daily things
#include <time.h>
s32 MP_Today(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (getenv("MOEMON_DEBUG_DATE"))
        return (s32)atoi(getenv("MOEMON_DEBUG_DATE"));
    return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
}

// Days since 2000-01-01 for a YYYYMMDD date
s32 MP_DayNumber(s32 yyyymmdd)
{
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_year = yyyymmdd / 10000 - 1900;
    t.tm_mon = (yyyymmdd / 100) % 100 - 1;
    t.tm_mday = yyyymmdd % 100;
    t.tm_hour = 12;
    return (s32)((mktime(&t) - 946728000) / 86400);
}

// Week number (weeks start on Monday), the same for everyone in the week
s32 MP_WeekNumber(void)
{
    // 2000-01-03 was a Monday (day 2)
    return (MP_DayNumber(MP_Today()) - 2) / 7;
}
