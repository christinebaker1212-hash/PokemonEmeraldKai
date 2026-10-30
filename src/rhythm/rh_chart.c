// Moemon Rhythm: StepMania's simfiles. A song's header (title, music, BPMs, stops, offset) and its dance-single
// charts' places in the file are read when the song list is made; one chart's notes when it is played.
// .ssc files (StepMania 5) keep their charts in #NOTEDATA blocks with #STEPSTYPE / #DIFFICULTY / #METER / #NOTES
// tags; a chart's own BPMs aren't read (the song's are used).
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rhythm/chart.h"

uint32_t Rh_Hash(const void *data, uint32_t len, uint32_t h)
{
    const uint8_t *p = data;
    uint32_t i;
    if (h == 0)
        h = 2166136261u;
    for (i = 0; i < len; i++)
        h = (h ^ p[i]) * 16777619u;
    return h;
}

static char *ReadAll(const char *path, uint32_t *len)
{
    FILE *f = fopen(path, "rb");
    char *buf;
    long n;
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0 || n > 16 * 1024 * 1024 || !(buf = malloc(n + 1)))
    {
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, n, f) != (size_t)n)
    {
        fclose(f);
        free(buf);
        return NULL;
    }
    fclose(f);
    buf[n] = 0;
    *len = (uint32_t)n;
    return buf;
}

// A tag's value: from after the ':' to the ';' (comments "//" to the line's end are skipped). Returns where the
// value starts and its length; *next is past the ';'
static const char *NextTag(const char *p, const char *end, char *name, int nameMax, uint32_t *valueLen,
                           const char **next)
{
    const char *v;
    int n = 0;
    while (p < end)
    {
        if (p[0] == '/' && p + 1 < end && p[1] == '/')
        {
            while (p < end && *p != '\n')
                p++;
            continue;
        }
        if (*p == '#')
            break;
        p++;
    }
    if (p >= end)
        return NULL;
    p++;
    while (p < end && *p != ':' && *p != ';' && n < nameMax - 1)
        name[n++] = (char)toupper((unsigned char)*p++);
    name[n] = 0;
    while (p < end && *p != ':' && *p != ';')
        p++;
    if (p >= end || *p == ';')
    {
        *valueLen = 0;
        *next = p < end ? p + 1 : end;
        return p;
    }
    v = ++p;
    while (p < end && *p != ';')
    {
        // a '#' at a line's start without the ';' before it: the tag ended there (a broken file)
        if (*p == '#' && p > v && (p[-1] == '\n' || p[-1] == '\r'))
            break;
        p++;
    }
    *valueLen = (uint32_t)(p - v);
    *next = p < end && *p == ';' ? p + 1 : p;
    return v;
}

static void CopyText(char *dst, const char *src, uint32_t len)
{
    uint32_t i, o = 0;
    while (len > 0 && isspace((unsigned char)*src))
        src++, len--;
    while (len > 0 && isspace((unsigned char)src[len - 1]))
        len--;
    for (i = 0; i < len && o < RH_TEXT - 1; i++)
        dst[o++] = src[i];
    dst[o] = 0;
}

static double Num(const char *s, uint32_t len)
{
    char buf[64];
    if (len >= sizeof(buf))
        len = sizeof(buf) - 1;
    memcpy(buf, s, len);
    buf[len] = 0;
    return atof(buf);
}

static int CompareBpm(const void *a, const void *b)
{
    double x = ((const struct RhBpm *)a)->beat, y = ((const struct RhBpm *)b)->beat;
    return x < y ? -1 : x > y;
}

static int CompareStop(const void *a, const void *b)
{
    double x = ((const struct RhStop *)a)->beat, y = ((const struct RhStop *)b)->beat;
    return x < y ? -1 : x > y;
}

// "beat=value,beat=value"
static int ReadPairs(const char *v, uint32_t len, double (*out)[2], int max)
{
    const char *end = v + len;
    int n = 0;
    while (v < end && n < max)
    {
        const char *eq = v, *comma;
        while (eq < end && *eq != '=' && *eq != ',')
            eq++;
        comma = eq;
        while (comma < end && *comma != ',')
            comma++;
        if (eq < end && *eq == '=')
        {
            out[n][0] = Num(v, (uint32_t)(eq - v));
            out[n][1] = Num(eq + 1, (uint32_t)(comma - eq - 1));
            n++;
        }
        v = comma + 1;
    }
    return n;
}

double RhSong_TimeAt(const struct RhSong *song, double beat)
{
    // the BPM changes and stops before the beat, in order (a step on a stop's beat comes before the stop)
    double t = 0, b = 0, bpm = song->bpms[0].bpm > 0 ? song->bpms[0].bpm : 0.001;
    int i = 1, s = 0;
    for (;;)
    {
        double nb = i < song->bpmCount ? song->bpms[i].beat : 1e18;
        double ns = s < song->stopCount ? song->stops[s].beat : 1e18;
        if ((nb < ns ? nb : ns) >= beat)
            break;
        if (ns <= nb)
        {
            t += (ns - b) * 60.0 / bpm + song->stops[s].seconds;
            b = ns;
            s++;
        }
        else
        {
            t += (nb - b) * 60.0 / bpm;
            b = nb;
            bpm = song->bpms[i].bpm > 0 ? song->bpms[i].bpm : 0.001;
            i++;
        }
    }
    return t + (beat - b) * 60.0 / bpm;
}

static void SetTimes(struct RhSong *song)
{
    int i;
    if (song->bpmCount == 0)
    {
        free(song->bpms);
        song->bpms = calloc(1, sizeof(*song->bpms));
        song->bpms[0].bpm = 120;
        song->bpmCount = 1;
    }
    qsort(song->bpms, song->bpmCount, sizeof(*song->bpms), CompareBpm);
    qsort(song->stops, song->stopCount, sizeof(*song->stops), CompareStop);
    song->bpms[0].beat = 0;   // the first BPM covers everything before it
    for (i = 0; i < song->bpmCount; i++)
        song->bpms[i].time = RhSong_TimeAt(song, song->bpms[i].beat);
    for (i = 0; i < song->stopCount; i++)
        song->stops[i].time = RhSong_TimeAt(song, song->stops[i].beat);
}

double RhSong_BpmAt(const struct RhSong *song, double beat)
{
    int i = song->bpmCount - 1;
    while (i > 0 && song->bpms[i].beat > beat)
        i--;
    return song->bpms[i].bpm;
}

double RhSong_BeatAt(const struct RhSong *song, double time)
{
    // (the song list is short: a search on the time each beat starts)
    double lo = -64, hi = song->lastBeat + 256;
    int i;
    if (time < RhSong_TimeAt(song, 0))
        return (time - RhSong_TimeAt(song, 0)) * song->bpms[0].bpm / 60.0;
    // in a stop the beat stays
    for (i = 0; i < song->stopCount; i++)
        if (time >= song->stops[i].time && time < song->stops[i].time + song->stops[i].seconds)
            return song->stops[i].beat;
    for (i = 0; i < 60; i++)
    {
        double mid = (lo + hi) / 2;
        if (RhSong_TimeAt(song, mid) <= time)
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}

static int DiffOf(const char *name, int meter)
{
    char low[32];
    int i;
    for (i = 0; name[i] && i < 31; i++)
        low[i] = (char)tolower((unsigned char)name[i]);
    low[i] = 0;
    if (!strcmp(low, "beginner"))
        return RH_BEGINNER;
    if (!strcmp(low, "easy") || !strcmp(low, "basic") || !strcmp(low, "light"))
        return RH_BASIC;
    if (!strcmp(low, "medium") || !strcmp(low, "another") || !strcmp(low, "trick") || !strcmp(low, "standard")
        || !strcmp(low, "difficult"))
        return RH_DIFFICULT;
    if (!strcmp(low, "hard") || !strcmp(low, "ssr") || !strcmp(low, "maniac") || !strcmp(low, "heavy")
        || !strcmp(low, "expert"))
        return RH_EXPERT;
    if (!strcmp(low, "challenge") || !strcmp(low, "smaniac") || !strcmp(low, "oni"))
        return RH_CHALLENGE;
    (void)meter;
    return -1;   // edits aren't on the machine
}

// One chart's notes: counts and the hash
static void Measure(const char *notes, uint32_t len, struct RhChartInfo *info)
{
    uint32_t i;
    int steps = 0, holds = 0, col = 0, rowHas = 0;
    info->hash = 0;
    for (i = 0; i < len; i++)
    {
        char c = notes[i];
        if (c == '/' && i + 1 < len && notes[i + 1] == '/')
        {
            while (i < len && notes[i] != '\n')
                i++;
            col = 0;
            continue;
        }
        if (c == '\n' || c == ',')
        {
            if (col == RH_COLS && rowHas)
                steps++;
            col = 0;
            rowHas = 0;
            if (c == ',')
                info->hash = Rh_Hash(",", 1, info->hash);
            continue;
        }
        if (isspace((unsigned char)c))
            continue;
        if (col < RH_COLS)
        {
            if (c == '1' || c == '2' || c == '4')
                rowHas = 1;
            if (c == '2' || c == '4')
                holds++;
        }
        col++;
        info->hash = Rh_Hash(&c, 1, info->hash);
    }
    if (col == RH_COLS && rowHas)
        steps++;
    info->steps = (uint16_t)(steps > 65535 ? 65535 : steps);
    info->holds = (uint16_t)(holds > 65535 ? 65535 : holds);
}

// "dance-single: credit: Difficulty: meter: radar: notes" (.sm's #NOTES)
static void ReadSmNotes(struct RhSong *song, const char *v, uint32_t len, uint32_t at)
{
    const char *field[6];
    uint32_t flen[6];
    const char *p = v, *end = v + len;
    int i, diff;
    char type[32], name[32];
    for (i = 0; i < 5; i++)
    {
        const char *c = p;
        while (c < end && *c != ':')
            c++;
        if (c >= end)
            return;
        field[i] = p;
        flen[i] = (uint32_t)(c - p);
        p = c + 1;
    }
    field[5] = p;
    flen[5] = (uint32_t)(end - p);
    CopyText(type, field[0], flen[0] < 31 ? flen[0] : 31);
    if (strcmp(type, "dance-single"))
        return;
    CopyText(name, field[2], flen[2] < 31 ? flen[2] : 31);
    diff = DiffOf(name, 0);
    if (diff < 0 || song->charts[diff].present)
        return;
    song->charts[diff].present = 1;
    song->charts[diff].meter = (uint8_t)Num(field[3], flen[3]);
    {
        double r[RH_RADARS] = {0};
        const char *q = field[4], *qe = field[4] + flen[4];
        for (i = 0; i < RH_RADARS && q < qe; i++)
        {
            const char *c = q;
            while (c < qe && *c != ',')
                c++;
            r[i] = Num(q, (uint32_t)(c - q));
            q = c + 1;
        }
        for (i = 0; i < RH_RADARS; i++)
            song->charts[diff].radar[i] = (float)r[i];
    }
    song->charts[diff].notesAt = at + (uint32_t)(field[5] - v);
    song->charts[diff].notesLen = flen[5];
    Measure(field[5], flen[5], &song->charts[diff]);
}

// The measures' lines of RH_COLS steps (comments and blank lines skipped): calls each line with its beat
static void ForEachLine(const char *notes, uint32_t len, void (*fn)(void *ctx, double beat, int32_t row,
                        const char *line), void *ctx)
{
    const char *p = notes, *end = notes + len;
    int measure = 0;
    while (p < end)
    {
        char lines[192][RH_COLS];
        int n = 0, c = 0, i;
        const char *e = p;
        while (e < end && *e != ',')
            e++;
        while (p < e)
        {
            if (*p == '/' && p + 1 < e && p[1] == '/')
            {
                while (p < e && *p != '\n')
                    p++;
                continue;
            }
            if (*p == '\n' || *p == '\r')
            {
                if (c >= RH_COLS && n < 192)
                    n++;
                c = 0;
                p++;
                continue;
            }
            if (!isspace((unsigned char)*p))
            {
                if (c < RH_COLS && n < 192)
                    lines[n][c] = *p;
                c++;
            }
            p++;
        }
        if (c >= RH_COLS && n < 192)
            n++;
        for (i = 0; i < n; i++)
        {
            int32_t row = measure * 4 * RH_ROWS_PER_BEAT + (int32_t)(4 * RH_ROWS_PER_BEAT * (int64_t)i / n);
            fn(ctx, row / (double)RH_ROWS_PER_BEAT, row, lines[i]);
        }
        measure++;
        p = e + 1;
    }
}

static void LastBeatLine(void *ctx, double beat, int32_t row, const char *line)
{
    struct RhSong *song = ctx;
    int i;
    (void)row;
    for (i = 0; i < RH_COLS; i++)
        if (line[i] != '0' && beat > song->lastBeat)
            song->lastBeat = beat;
}

static void LastBeat(struct RhSong *song, const char *notes, uint32_t len)
{
    ForEachLine(notes, len, LastBeatLine, song);
}

int RhSong_Load(struct RhSong *song, const char *path, const char *group)
{
    uint32_t len, vlen;
    char *text = ReadAll(path, &len);
    const char *p, *end, *v, *next;
    char tag[32];
    double pairs[512][2];
    int i, n, any = 0;
    int ssc = strlen(path) > 4 && !strcmp(path + strlen(path) - 4, ".ssc");
    // .ssc chart blocks
    char stepsType[32] = "", diffName[32] = "";
    int meter = 0;
    double radar[RH_RADARS] = {0};

    memset(song, 0, sizeof(*song));
    if (!text)
        return 0;
    snprintf(song->file, sizeof(song->file), "%s", path);
    snprintf(song->dir, sizeof(song->dir), "%s", path);
    {
        char *slash = strrchr(song->dir, '/'), *bs = strrchr(song->dir, '\\');
        if (bs > slash)
            slash = bs;
        if (slash)
            *slash = 0;
    }
    snprintf(song->group, sizeof(song->group), "%s", group ? group : "");
    song->sampleLength = 12;
    p = text;
    end = text + len;
    while ((v = NextTag(p, end, tag, sizeof(tag), &vlen, &next)) != NULL)
    {
        if (!strcmp(tag, "TITLE"))
            CopyText(song->title, v, vlen);
        else if (!strcmp(tag, "SUBTITLE"))
            CopyText(song->subtitle, v, vlen);
        else if (!strcmp(tag, "ARTIST"))
            CopyText(song->artist, v, vlen);
        else if (!strcmp(tag, "TITLETRANSLIT"))
            CopyText(song->titleTranslit, v, vlen);
        else if (!strcmp(tag, "MUSIC"))
            CopyText(song->music, v, vlen);
        else if (!strcmp(tag, "BANNER"))
            CopyText(song->banner, v, vlen);
        else if (!strcmp(tag, "BACKGROUND"))
            CopyText(song->background, v, vlen);
        else if (!strcmp(tag, "OFFSET"))
            song->offset = Num(v, vlen);
        else if (!strcmp(tag, "SAMPLESTART"))
            song->sampleStart = Num(v, vlen);
        else if (!strcmp(tag, "SAMPLELENGTH"))
            song->sampleLength = Num(v, vlen) > 0 ? Num(v, vlen) : 12;
        else if (!strcmp(tag, "DISPLAYBPM"))
        {
            const char *colon = memchr(v, ':', vlen);
            if (vlen > 0 && memchr(v, '*', vlen))
                song->displayBpmRandom = 1;
            song->displayBpmLo = Num(v, colon ? (uint32_t)(colon - v) : vlen);
            song->displayBpmHi = colon ? Num(colon + 1, vlen - (uint32_t)(colon + 1 - v)) : song->displayBpmLo;
        }
        else if (!strcmp(tag, "BPMS") && song->bpmCount == 0)
        {
            n = ReadPairs(v, vlen, pairs, 512);
            song->bpms = calloc(n > 0 ? n : 1, sizeof(*song->bpms));
            for (i = 0; i < n; i++)
            {
                song->bpms[i].beat = pairs[i][0];
                song->bpms[i].bpm = pairs[i][1];
            }
            song->bpmCount = n;
        }
        else if ((!strcmp(tag, "STOPS") || !strcmp(tag, "FREEZES")) && song->stopCount == 0)
        {
            n = ReadPairs(v, vlen, pairs, 512);
            song->stops = calloc(n > 0 ? n : 1, sizeof(*song->stops));
            for (i = 0; i < n; i++)
            {
                song->stops[i].beat = pairs[i][0];
                song->stops[i].seconds = pairs[i][1];
            }
            song->stopCount = n;
        }
        else if (!strcmp(tag, "NOTES") && !ssc)
        {
            ReadSmNotes(song, v, vlen, (uint32_t)(v - text));
        }
        else if (ssc && !strcmp(tag, "NOTEDATA"))
        {
            stepsType[0] = diffName[0] = 0;
            meter = 0;
        }
        else if (ssc && !strcmp(tag, "STEPSTYPE"))
            CopyText(stepsType, v, vlen < 31 ? vlen : 31);
        else if (ssc && !strcmp(tag, "DIFFICULTY"))
            CopyText(diffName, v, vlen < 31 ? vlen : 31);
        else if (ssc && !strcmp(tag, "METER"))
            meter = (int)Num(v, vlen);
        else if (ssc && !strcmp(tag, "RADARVALUES"))
        {
            const char *q = v, *qe = v + vlen;
            for (i = 0; i < RH_RADARS && q < qe; i++)
            {
                const char *c = q;
                while (c < qe && *c != ',')
                    c++;
                radar[i] = Num(q, (uint32_t)(c - q));
                q = c + 1;
            }
        }
        else if (ssc && (!strcmp(tag, "NOTES") || !strcmp(tag, "NOTES2")) && !strcmp(stepsType, "dance-single"))
        {
            int diff = DiffOf(diffName, meter);
            if (diff >= 0 && !song->charts[diff].present)
            {
                song->charts[diff].present = 1;
                song->charts[diff].meter = (uint8_t)meter;
                for (i = 0; i < RH_RADARS; i++)
                    song->charts[diff].radar[i] = (float)radar[i];
                song->charts[diff].notesAt = (uint32_t)(v - text);
                song->charts[diff].notesLen = vlen;
                Measure(v, vlen, &song->charts[diff]);
            }
        }
        p = next;
    }
    for (i = 0; i < RH_DIFFS; i++)
    {
        if (!song->charts[i].present)
            continue;
        if (song->charts[i].steps == 0)
        {
            song->charts[i].present = 0;
            continue;
        }
        any = 1;
        LastBeat(song, text + song->charts[i].notesAt, song->charts[i].notesLen);
    }
    free(text);
    if (!any || !song->title[0])
    {
        RhSong_Free(song);
        return 0;
    }
    SetTimes(song);
    if (song->displayBpmLo <= 0)
    {
        double lo = 1e9, hi = 0;
        for (i = 0; i < song->bpmCount; i++)
        {
            // (a BPM that only lasts a moment isn't shown, as on the arcade's display)
            double until = i + 1 < song->bpmCount ? song->bpms[i + 1].beat : song->lastBeat;
            if (until - song->bpms[i].beat < 2 && song->bpmCount > 1)
                continue;
            if (song->bpms[i].bpm < lo)
                lo = song->bpms[i].bpm;
            if (song->bpms[i].bpm > hi)
                hi = song->bpms[i].bpm;
        }
        song->displayBpmLo = lo < 1e9 ? lo : song->bpms[0].bpm;
        song->displayBpmHi = hi > 0 ? hi : song->bpms[0].bpm;
    }
    return 1;
}

void RhSong_Free(struct RhSong *song)
{
    free(song->bpms);
    free(song->stops);
    song->bpms = NULL;
    song->stops = NULL;
    song->bpmCount = song->stopCount = 0;
}

static int CompareNote(const void *a, const void *b)
{
    const struct RhNote *x = a, *y = b;
    if (x->row != y->row)
        return x->row < y->row ? -1 : 1;
    return (int)x->col - (int)y->col;
}

static uint8_t QuantOf(int row)
{
    int r = row % RH_ROWS_PER_BEAT;
    if (r == 0)
        return 0;                       // 4th
    if (r % (RH_ROWS_PER_BEAT / 2) == 0)
        return 1;                       // 8th
    if (r % (RH_ROWS_PER_BEAT / 3) == 0)
        return 2;                       // 12th
    if (r % (RH_ROWS_PER_BEAT / 4) == 0)
        return 3;                       // 16th
    if (r % (RH_ROWS_PER_BEAT / 6) == 0)
        return 4;                       // 24th
    if (r % (RH_ROWS_PER_BEAT / 8) == 0)
        return 5;                       // 32nd
    if (r % (RH_ROWS_PER_BEAT / 12) == 0)
        return 6;                       // 48th
    return 7;                           // 64th and finer
}

struct LoadCtx
{
    struct RhChart *chart;
    int cap;
    int32_t holdFrom[RH_COLS];
};

static void LoadLine(void *ctx, double beat, int32_t row, const char *line)
{
    struct LoadCtx *l = ctx;
    struct RhChart *chart = l->chart;
    int col;
    (void)beat;
    for (col = 0; col < RH_COLS; col++)
    {
        char ch = line[col];
        struct RhNote *note;
        if (ch == '3' && l->holdFrom[col] >= 0)
        {
            chart->notes[l->holdFrom[col]].endRow = row;
            l->holdFrom[col] = -1;
            continue;
        }
        if (ch != '1' && ch != '2' && ch != '4' && ch != 'M')
            continue;
        if (chart->count == l->cap)
        {
            l->cap *= 2;
            chart->notes = realloc(chart->notes, l->cap * sizeof(*chart->notes));
        }
        note = &chart->notes[chart->count];
        memset(note, 0, sizeof(*note));
        note->row = row;
        note->col = (uint8_t)col;
        note->type = ch == 'M' ? RH_NOTE_MINE : ch == '1' ? RH_NOTE_TAP : RH_NOTE_HOLD;   // rolls play as freezes
        note->quant = QuantOf(row);
        note->endRow = row;
        if (note->type == RH_NOTE_HOLD)
        {
            if (l->holdFrom[col] >= 0)   // a freeze started before the last one ended: that one was a step
                chart->notes[l->holdFrom[col]].type = RH_NOTE_TAP;
            l->holdFrom[col] = chart->count;
        }
        chart->count++;
    }
}

int RhChart_Load(const struct RhSong *song, int diff, struct RhChart *chart)
{
    uint32_t len;
    char *text;
    struct LoadCtx l;
    int i;

    memset(chart, 0, sizeof(*chart));
    if (diff < 0 || diff >= RH_DIFFS || !song->charts[diff].present)
        return 0;
    text = ReadAll(song->file, &len);
    if (!text || song->charts[diff].notesAt + song->charts[diff].notesLen > len)
    {
        free(text);
        return 0;
    }
    l.chart = chart;
    l.cap = 256;
    chart->notes = malloc(l.cap * sizeof(*chart->notes));
    for (i = 0; i < RH_COLS; i++)
        l.holdFrom[i] = -1;
    ForEachLine(text + song->charts[diff].notesAt, song->charts[diff].notesLen, LoadLine, &l);
    free(text);
    // a freeze arrow with no end is a step
    for (i = 0; i < RH_COLS; i++)
        if (l.holdFrom[i] >= 0)
            chart->notes[l.holdFrom[i]].type = RH_NOTE_TAP;
    // (already by row; the sort keeps it so for anything odd, by column within a row)
    qsort(chart->notes, chart->count, sizeof(*chart->notes), CompareNote);
    {
        int32_t lastRow = -1;
        for (i = 0; i < chart->count; i++)
        {
            struct RhNote *note = &chart->notes[i];
            note->beat = note->row / (double)RH_ROWS_PER_BEAT;
            note->time = RhSong_TimeAt(song, note->beat);
            note->endBeat = note->endRow / (double)RH_ROWS_PER_BEAT;
            note->endTime = RhSong_TimeAt(song, note->endBeat);
            if (note->type == RH_NOTE_HOLD && note->endRow <= note->row)
                note->type = RH_NOTE_TAP;
            if (note->type == RH_NOTE_HOLD)
                chart->holds++;
            if (note->type != RH_NOTE_MINE && note->row != lastRow)
            {
                chart->rows++;
                lastRow = note->row;
            }
            if (note->endTime > chart->lastTime)
                chart->lastTime = note->endTime;
        }
    }
    return chart->count > 0;
}

void RhChart_Free(struct RhChart *chart)
{
    free(chart->notes);
    memset(chart, 0, sizeof(*chart));
}
