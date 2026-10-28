#ifdef PORTABLE
#include "mp_version.h"
#endif
// Moemon Rhythm: the DANCE machine's records, kept by the server for everyone on it whatever their room
// (docs/RHYTHM_DECISIONS.md #15). Per chart (its notes' hash and difficulty, so the same chart in another
// folder shares them) the best 10 scores, one per player name. Saved to "rhythm_scores.dat" where the server
// runs (the standalone server, or the game hosting on the LAN).
//
// Wire format (MPMSG_C_STORE / MPMSG_S_STORE, ops MP_ST_RH_*): see mp_net.h.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "mp_net.h"

#define SCORE_FILE "rhythm_scores.dat"
#define SCORE_MAGIC 0x31524853   // "SHR1"

struct ScoreEntry
{
    char name[MP_RH_NAME];
    uint32_t score;
    uint8_t grade, fullCombo;
    uint16_t maxCombo;
    uint32_t when;
};

struct Chart
{
    uint32_t key;
    uint8_t diff, meter, count;
    char title[48];
    struct ScoreEntry top[MP_RH_TOP];
};

static struct Chart *sCharts;
static int sChartCount, sChartCap, sLoaded;

// where the server keeps its files (MOEMON_STORE_DIR, as mp_store.c's)
static void ScorePath(char *out, size_t size, const char *suffix)
{
    const char *dir = getenv("MOEMON_STORE_DIR");
    snprintf(out, size, "%s%s%s%s", dir ? dir : "", dir ? "/" : "", SCORE_FILE, suffix);
}

static void Load(void)
{
    char path[600];
    FILE *f;
    uint32_t magic = 0, n = 0;
    if (sLoaded)
        return;
    sLoaded = 1;
    ScorePath(path, sizeof(path), "");
    if (!(f = fopen(path, "rb")))
        return;
    if (fread(&magic, 4, 1, f) == 1 && magic == SCORE_MAGIC && fread(&n, 4, 1, f) == 1 && n < 1000000)
    {
        sCharts = calloc(n + 16, sizeof(*sCharts));
        sChartCap = (int)n + 16;
        sChartCount = (int)fread(sCharts, sizeof(*sCharts), n, f);
    }
    fclose(f);
}

static void Save(void)
{
    char tmp[600], path[600];
    FILE *f;
    uint32_t magic = SCORE_MAGIC, n = (uint32_t)sChartCount;
    ScorePath(tmp, sizeof(tmp), ".tmp");
    ScorePath(path, sizeof(path), "");
    if (!(f = fopen(tmp, "wb")))
        return;
    fwrite(&magic, 4, 1, f);
    fwrite(&n, 4, 1, f);
    fwrite(sCharts, sizeof(*sCharts), n, f);
    fclose(f);
    remove(path);
    rename(tmp, path);
}

static struct Chart *Find(uint32_t key, int make)
{
    int i;
    for (i = 0; i < sChartCount; i++)
        if (sCharts[i].key == key)
            return &sCharts[i];
    if (!make)
        return NULL;
    if (sChartCount == sChartCap)
    {
        sChartCap = sChartCap ? sChartCap * 2 : 256;
        sCharts = realloc(sCharts, sChartCap * sizeof(*sCharts));
    }
    memset(&sCharts[sChartCount], 0, sizeof(sCharts[0]));
    sCharts[sChartCount].key = key;
    return &sCharts[sChartCount++];
}

static uint32_t Get32(const uint8_t *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
}

static void Put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

// MP_ST_RH_TOP: u32 chart, u8 count, (name[MP_RH_NAME], u32 score, u8 grade, u8 fullCombo, u16 maxCombo) * count
static uint16_t TopMsg(uint32_t key, uint8_t *out)
{
    struct Chart *c = Find(key, 0);
    uint16_t n = 6;
    int i;
    out[0] = MP_ST_RH_TOP;
    Put32(out + 1, key);
    out[5] = c ? c->count : 0;
    for (i = 0; c && i < c->count; i++)
    {
        memcpy(out + n, c->top[i].name, MP_RH_NAME);
        n += MP_RH_NAME;
        Put32(out + n, c->top[i].score);
        out[n + 4] = c->top[i].grade;
        out[n + 5] = c->top[i].fullCombo;
        out[n + 6] = (uint8_t)c->top[i].maxCombo;
        out[n + 7] = (uint8_t)(c->top[i].maxCombo >> 8);
        n += 8;
    }
    return n;
}

static int Submit(const uint8_t *d, uint16_t len)
{
    struct Chart *c;
    struct ScoreEntry e;
    uint32_t key;
    int i, at;
    uint8_t titleLen;
    // u32 chart, u8 diff, u8 meter, u32 score, u8 grade, u8 fullCombo, u16 maxCombo, name[MP_RH_NAME], u8 len, title
    if (len < 1 + 14 + MP_RH_NAME + 1)
        return 0;
    key = Get32(d + 1);
    memset(&e, 0, sizeof(e));
    e.score = Get32(d + 7);
    e.grade = d[11];
    e.fullCombo = d[12];
    e.maxCombo = (uint16_t)(d[13] | d[14] << 8);
    memcpy(e.name, d + 15, MP_RH_NAME);
    e.name[MP_RH_NAME - 1] = 0;
    e.when = (uint32_t)time(NULL);
    if (!e.name[0])
        return 0;
    c = Find(key, 1);
    c->diff = d[5];
    c->meter = d[6];
    titleLen = d[15 + MP_RH_NAME];
    if (titleLen && 16 + MP_RH_NAME + titleLen <= len)
    {
        int n = titleLen < sizeof(c->title) - 1 ? titleLen : (int)sizeof(c->title) - 1;
        memcpy(c->title, d + 16 + MP_RH_NAME, n);
        c->title[n] = 0;
    }
    // one entry per name: only a better score replaces theirs
    for (i = 0; i < c->count; i++)
        if (!strncmp(c->top[i].name, e.name, MP_RH_NAME))
        {
            if (c->top[i].score >= e.score)
                return 0;
            memmove(&c->top[i], &c->top[i + 1], (c->count - i - 1) * sizeof(e));
            c->count--;
            break;
        }
    for (at = 0; at < c->count && c->top[at].score >= e.score; at++)
        ;
    if (at >= MP_RH_TOP)
        return 0;
    if (c->count < MP_RH_TOP)
        c->count++;
    memmove(&c->top[at + 1], &c->top[at], (c->count - at - 1) * sizeof(e));
    c->top[at] = e;
    Save();
    return 1;
}

void MPScores_Handle(const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx)
{
    static uint8_t reply[8 + MP_RH_TOP * (MP_RH_NAME + 8)];
    Load();
    if (len < 5)
        return;
    switch (data[0])
    {
    case MP_ST_RH_SUBMIT:
        // a new record goes to the room (their wheels show it); else just the sender hears the list
        send(ctx, Submit(data, len) ? 1 : 0, reply, TopMsg(Get32(data + 1), reply));
        break;
    case MP_ST_RH_GET:
        send(ctx, 0, reply, TopMsg(Get32(data + 1), reply));
        break;
    }
}
