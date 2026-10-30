// Moemon Rhythm: one player's song, judged the way Beware's DDR EXTREME build of StepMania 3.9 does it
// (docs/RHYTHM_DECISIONS.md #2-#3):
// - a step goes to the nearest arrow in its column that isn't judged yet, and is judged by how far off it was
//   (MARVELOUS 16.7 ms, PERFECT 33.3, GREAT 83, GOOD 120, BOO 158.3; further: the step does nothing);
// - a row (a jump is one) is judged when all its arrows are, by the last one stepped on (a missed one: MISS);
// - the combo counts arrows (a jump is 2) while rows are GREAT or better;
// - a freeze arrow is OK unless let go of for 250 ms; two that start together are judged as one;
// - score: feet x 10,000,000 over the rows and freezes, each worth more than the last (p * (B / S) * n), and what's
//   left over when the last one is PERFECT; grade by dance points; the life bar as LifeMeterBar.cpp.
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "rhythm/play.h"

static const float sLifeDelta[RH_J_COUNT] = {0, -0.080f, -0.040f, 0.000f, 0.004f, 0.008f, 0.008f};
static const int sGradeWeight[RH_J_COUNT] = {0, -8, -4, 0, 1, 2, 2};
#define LIFE_OK       0.008f
#define LIFE_NG      -0.080f
#define LIFE_MINE    -0.160f
#define LIFE_HOT_MISS -0.10f
#define DP_OK 6
#define REGEN_AFTER_MISS 5
#define REGEN_AFTER_FAIL 10
#define REGEN_MAX 10

const char *RhPlay_JudgeName(int judge)
{
    static const char *const names[RH_J_COUNT] = {"", "MISS", "BOO", "GOOD", "GREAT", "PERFECT", "MARVELOUS"};
    return judge >= 0 && judge < RH_J_COUNT ? names[judge] : "";
}

static void Event(struct RhPlay *p, uint8_t kind, uint8_t judge, uint8_t col, int note, float offset)
{
    struct RhEvent *e;
    if (p->eventCount >= RH_EVENTS)
    {
        memmove(p->events, p->events + 1, (RH_EVENTS - 1) * sizeof(*p->events));
        p->eventCount--;
    }
    e = &p->events[p->eventCount++];
    e->kind = kind;
    e->judge = judge;
    e->col = col;
    e->note = note;
    e->offset = offset;
}

static int CompareNote(const void *a, const void *b)
{
    const struct RhNote *x = a, *y = b;
    if (x->row != y->row)
        return x->row < y->row ? -1 : 1;
    return (int)x->col - (int)y->col;
}

int RhPlay_Begin(struct RhPlay *p, const struct RhSong *song, const struct RhChart *chart, int meter,
                 const struct RhNoteOptions *opt, int showMarvelous)
{
    static const uint8_t turns[5][RH_COLS] = {
        {0, 1, 2, 3},   // off
        {3, 2, 1, 0},   // mirror
        {1, 3, 0, 2},   // left: the pad turned a quarter to the left (LEFT is now DOWN...)
        {2, 0, 3, 1},   // right
        {0, 1, 2, 3},   // shuffle (below)
    };
    uint8_t map[RH_COLS];
    int i, n = 0, lastRow = -1, lastHoldRow = -1, N;
    struct RhNoteOptions none = {0};

    memset(p, 0, sizeof(*p));
    if (!opt)
        opt = &none;
    p->song = song;
    p->showMarvelous = (uint8_t)showMarvelous;
    memcpy(map, turns[opt->turn < 5 ? opt->turn : 0], RH_COLS);
    if (opt->turn == RH_TURN_SHUFFLE)
    {
        uint32_t r = opt->shuffleSeed | 1;
        do
        {
            for (i = RH_COLS - 1; i > 0; i--)
            {
                int j;
                uint8_t t;
                r = r * 1103515245u + 12345u;
                j = (int)((r >> 16) % (uint32_t)(i + 1));
                t = map[i];
                map[i] = map[j];
                map[j] = t;
            }
        } while (map[0] == 0 && map[1] == 1 && map[2] == 2 && map[3] == 3);
    }
    p->notes = malloc((chart->count ? chart->count : 1) * sizeof(*p->notes));
    for (i = 0; i < chart->count; i++)
    {
        struct RhNote note = chart->notes[i];
        if (opt->little && note.row % RH_ROWS_PER_BEAT != 0)
            continue;
        if (opt->noFreeze && note.type == RH_NOTE_HOLD)
        {
            note.type = RH_NOTE_TAP;
            note.endRow = note.row;
            note.endBeat = note.beat;
            note.endTime = note.time;
        }
        note.col = map[note.col & 3];
        p->notes[n++] = note;
    }
    p->count = n;
    qsort(p->notes, n, sizeof(*p->notes), CompareNote);
    p->judge = calloc(n + 1, 1);
    p->hold = calloc(n + 1, 1);
    p->holdLife = calloc(n + 1, sizeof(float));
    p->offset = calloc(n + 1, sizeof(float));
    for (i = 0; i < n; i++)
    {
        const struct RhNote *note = &p->notes[i];
        if (note->type == RH_NOTE_MINE)
            continue;
        if (note->row != lastRow)
        {
            p->rowsTotal++;
            lastRow = note->row;
        }
        if (note->type == RH_NOTE_HOLD)
        {
            p->holdsTotal++;
            p->holdLife[i] = 1;
            if (note->row != lastHoldRow)
            {
                p->holdGroups++;
                lastHoldRow = note->row;
            }
        }
    }
    p->meter = meter;
    if (meter < 1)
        meter = 1;
    if (meter > 10)
        meter = 10;
    p->maxPoints = meter * 10000000;
    if (opt->little)
        p->maxPoints >>= 1;
    p->pointBonus = p->maxPoints;
    N = p->rowsTotal + p->holdGroups;
    p->scoreEventsTotal = N;
    p->dpPossible = p->rowsTotal * sGradeWeight[RH_J_MARVELOUS] + p->holdGroups * DP_OK;
    p->life = 0.5f;
    return n > 0;
}

void RhPlay_End(struct RhPlay *p)
{
    free(p->notes);
    free(p->judge);
    free(p->hold);
    free(p->holdLife);
    free(p->offset);
    p->notes = NULL;
    p->judge = p->hold = NULL;
    p->holdLife = p->offset = NULL;
}

static void AddScore(struct RhPlay *p, int judge)
{
    int pts = 0, N = p->scoreEventsTotal;
    int32_t sum = N * (N + 1) / 2, B = p->maxPoints / 10;
    switch (judge)
    {
    case RH_J_MARVELOUS:
        pts = 10;
        break;
    case RH_J_PERFECT:
        pts = p->showMarvelous ? 9 : 10;
        break;
    case RH_J_GREAT:
        pts = 5;
        break;
    }
    p->scoreEvents++;
    if (sum <= 0)
        return;
    p->score += pts * (B / sum) * p->scoreEvents;
    p->pointBonus -= 10 * (B / sum) * p->scoreEvents;
    if (p->scoreEvents == N && judge >= RH_J_PERFECT)
        p->score += p->pointBonus;
}

static void ChangeLife(struct RhPlay *p, float delta, double t)
{
    if (p->failed)
        return;
    if (delta < 0)
        delta *= 0.5f + 0.5f * p->life;   // merciful drain
    if (delta >= 0)
    {
        p->missCombo = 0;
        if (p->comboToRegain > 0)
            p->comboToRegain--;
        if (p->comboToRegain > 0)
            delta = 0;
    }
    else
    {
        int regain = p->comboToRegain + REGEN_AFTER_MISS;
        p->missCombo++;
        if (regain > REGEN_MAX)
            regain = REGEN_MAX;
        if (regain > p->comboToRegain)
            p->comboToRegain = regain;
    }
    if (p->life + delta <= 0 && p->life > 0)
    {
        int regain = p->comboToRegain + REGEN_AFTER_FAIL;
        if (regain > REGEN_MAX)
            regain = REGEN_MAX;
        if (regain > p->comboToRegain)
            p->comboToRegain = regain;
    }
    p->life += delta;
    if (p->life > 1)
        p->life = 1;
    if (p->life <= 0)
    {
        p->life = 0;
        p->failed = 1;
        p->failTime = t;
        Event(p, RH_EV_FAIL, 0, 0, -1, 0);
    }
}

// A row's arrows are all judged: the row's judgment counts once
static void RowDone(struct RhPlay *p, int first, int last, double t)
{
    int i, judge = RH_J_NONE, taps = 0, lastCol = 0;
    float latest = -1e9f;
    for (i = first; i <= last; i++)
    {
        if (p->notes[i].type == RH_NOTE_MINE)
            continue;
        taps++;
        if (p->judge[i] == RH_J_MISS)
        {
            judge = RH_J_MISS;
            lastCol = p->notes[i].col;
            break;
        }
        // the last stepped on: the latest (least early) offset
        if (-p->offset[i] > latest)
        {
            latest = -p->offset[i];
            judge = p->judge[i];
            lastCol = p->notes[i].col;
        }
    }
    if (taps == 0)
        return;
    p->rowCount[judge]++;
    p->dpActual += sGradeWeight[judge];
    if (judge >= RH_J_GREAT)
    {
        p->combo += taps;
        if (p->combo > p->maxCombo)
            p->maxCombo = p->combo;
    }
    else
    {
        p->combo = 0;
    }
    AddScore(p, judge);
    {
        float delta = sLifeDelta[judge];
        if (p->life >= 1 && judge < RH_J_GOOD)
            delta = LIFE_HOT_MISS;
        ChangeLife(p, delta, t);
    }
    Event(p, RH_EV_ROW, (uint8_t)judge, (uint8_t)lastCol, first, 0);
}

// The row the note is in: first and last indexes
static void RowOf(const struct RhPlay *p, int i, int *first, int *last)
{
    int a = i, b = i;
    while (a > 0 && p->notes[a - 1].row == p->notes[i].row)
        a--;
    while (b + 1 < p->count && p->notes[b + 1].row == p->notes[i].row)
        b++;
    *first = a;
    *last = b;
}

static void Judged(struct RhPlay *p, int i, double t)
{
    int first, last, k;
    RowOf(p, i, &first, &last);
    for (k = first; k <= last; k++)
        if (p->notes[k].type != RH_NOTE_MINE && p->judge[k] == RH_J_NONE)
            return;
    RowDone(p, first, last, t);
}

void RhPlay_Press(struct RhPlay *p, int col, double t)
{
    int i, best = -1;
    double bestDist = 1e9;
    if (col < 0 || col >= RH_COLS)
        return;
    p->held[col] = 1;
    if (p->failed || p->finished)
        return;
    // the nearest arrow in this column not judged yet (StepMania looks about a beat each way)
    for (i = p->first; i < p->count; i++)
    {
        const struct RhNote *note = &p->notes[i];
        double d;
        if (note->time - t > 1.0)
            break;
        if (note->col != col || p->judge[i] != RH_J_NONE)
            continue;
        d = fabs(note->time - t);
        if (d < bestDist)
        {
            bestDist = d;
            best = i;
        }
    }
    if (best < 0)
        return;
    if (p->notes[best].type == RH_NOTE_MINE)
    {
        if (bestDist <= RH_WINDOW_MINE)
        {
            p->judge[best] = RH_J_MISS;   // (gone)
            p->minesHit++;
            ChangeLife(p, p->life >= 1 ? LIFE_HOT_MISS : LIFE_MINE, t);
            Event(p, RH_EV_MINE, 0, (uint8_t)col, best, 0);
        }
        return;
    }
    {
        int judge;
        float off = (float)(p->notes[best].time - t);
        if (bestDist <= RH_WINDOW_MARVELOUS)
            judge = RH_J_MARVELOUS;
        else if (bestDist <= RH_WINDOW_PERFECT)
            judge = RH_J_PERFECT;
        else if (bestDist <= RH_WINDOW_GREAT)
            judge = RH_J_GREAT;
        else if (bestDist <= RH_WINDOW_GOOD)
            judge = RH_J_GOOD;
        else if (bestDist <= RH_WINDOW_BOO)
            judge = RH_J_BOO;
        else
            return;
        p->judge[best] = (uint8_t)judge;
        p->offset[best] = off;
        p->tapCount[judge]++;
        p->offsetSum += off * 1000.0;
        p->offsetAbsSum += fabs(off) * 1000.0;
        p->offsetN++;
        if (judge < RH_J_MARVELOUS)
        {
            if (off > 0)
                p->fastCount++;
            else
                p->slowCount++;
        }
        if (p->notes[best].type == RH_NOTE_HOLD)
            p->hold[best] = RH_HOLD_ACTIVE;
        Event(p, RH_EV_TAP, (uint8_t)judge, (uint8_t)col, best, off);
        Judged(p, best, t);
    }
}

void RhPlay_Release(struct RhPlay *p, int col, double t)
{
    (void)t;
    if (col >= 0 && col < RH_COLS)
        p->held[col] = 0;
}

// A freeze arrow is decided: freezes that start together count once (OK when all are, NG at the first NG)
static void HoldDone(struct RhPlay *p, int i, int result, double t)
{
    int first, last, k, others = 0;
    RowOf(p, i, &first, &last);
    p->hold[i] = (uint8_t)result;
    for (k = first; k <= last; k++)
    {
        if (k == i || p->notes[k].type != RH_NOTE_HOLD)
            continue;
        others++;
        if (result == RH_HOLD_OK && p->hold[k] != RH_HOLD_OK)
            return;               // the other one isn't done: it decides
        if (result == RH_HOLD_NG && p->hold[k] == RH_HOLD_NG)
            return;               // already counted
    }
    if (result == RH_HOLD_NG)
        for (k = first; k <= last; k++)
            if (p->notes[k].type == RH_NOTE_HOLD)
            {
                p->hold[k] = RH_HOLD_NG;
                p->holdLife[k] = 0;
            }
    (void)others;
    if (result == RH_HOLD_OK)
    {
        p->ok++;
        p->dpActual += DP_OK;
        AddScore(p, RH_J_MARVELOUS);
        ChangeLife(p, LIFE_OK, t);
    }
    else
    {
        p->ng++;
        AddScore(p, RH_J_MISS);
        ChangeLife(p, p->life >= 1 ? LIFE_HOT_MISS : LIFE_NG, t);
    }
    // (an NG on a freeze whose head was missed isn't shown)
    Event(p, RH_EV_HOLD, (uint8_t)result, p->notes[i].col, i, p->judge[i] > RH_J_MISS ? 1.0f : 0.0f);
}

void RhPlay_Update(struct RhPlay *p, double t, double dt)
{
    int i;
    if (p->failed || p->finished)
        return;
    for (i = p->first; i < p->count && !p->failed; i++)
    {
        struct RhNote *note = &p->notes[i];
        if (note->time > t + 0.001 && note->type != RH_NOTE_HOLD)
            break;
        if (note->time > t)
            break;
        // misses
        if (p->judge[i] == RH_J_NONE && t - note->time > RH_WINDOW_BOO)
        {
            if (note->type == RH_NOTE_MINE)
            {
                p->judge[i] = RH_J_GREAT;   // (passed)
                continue;
            }
            p->judge[i] = RH_J_MISS;
            p->tapCount[RH_J_MISS]++;
            Event(p, RH_EV_TAP, RH_J_MISS, note->col, i, 0);
            Judged(p, i, t);
        }
        // a mine passing under a held arrow
        if (note->type == RH_NOTE_MINE && p->judge[i] == RH_J_NONE && p->held[note->col] && t >= note->time
            && t - note->time < 0.03)
        {
            p->judge[i] = RH_J_MISS;
            p->minesHit++;
            ChangeLife(p, p->life >= 1 ? LIFE_HOT_MISS : LIFE_MINE, t);
            Event(p, RH_EV_MINE, 0, note->col, i, 0);
        }
        // freezes
        if (note->type == RH_NOTE_HOLD && p->hold[i] != RH_HOLD_OK && p->hold[i] != RH_HOLD_NG && t >= note->time)
        {
            int stepped = p->judge[i] > RH_J_MISS;
            if (t <= note->endTime)
            {
                if (stepped && p->held[note->col])
                    p->holdLife[i] = 1;
                else if (p->judge[i] != RH_J_NONE || t - note->time > RH_WINDOW_BOO)
                {
                    p->holdLife[i] -= (float)(dt / RH_WINDOW_OK);
                    if (p->holdLife[i] < 0)
                        p->holdLife[i] = 0;
                }
            }
            if (p->holdLife[i] <= 0)
                HoldDone(p, i, RH_HOLD_NG, t);
            else if (t >= note->endTime && stepped)
                HoldDone(p, i, RH_HOLD_OK, t);
        }
    }
    // move past what's done
    while (p->first < p->count)
    {
        const struct RhNote *note = &p->notes[p->first];
        if (p->judge[p->first] == RH_J_NONE)
            break;
        if (note->type == RH_NOTE_HOLD && (p->hold[p->first] == RH_HOLD_ACTIVE || p->hold[p->first] == RH_HOLD_NONE))
            break;
        p->first++;
    }
    if (p->first >= p->count)
        p->finished = 1;
}

int RhPlay_Grade(const struct RhPlay *p)
{
    float pct;
    int belowPerfect;
    if (p->failed)
        return RH_GRADE_E;
    belowPerfect = p->rowCount[RH_J_GREAT] + p->rowCount[RH_J_GOOD] + p->rowCount[RH_J_BOO] + p->rowCount[RH_J_MISS]
                 + p->ng + p->minesHit;
    if (belowPerfect == 0 && p->rowCount[RH_J_PERFECT] == 0 && p->rowCount[RH_J_MARVELOUS] > 0)
        return p->showMarvelous ? RH_GRADE_AAAA : RH_GRADE_AAA;
    if (belowPerfect == 0 && p->rowsTotal > 0)
        return RH_GRADE_AAA;
    pct = p->dpPossible > 0 ? (float)p->dpActual / p->dpPossible : 0;
    if (pct >= 0.93f - 0.00001f)
        return RH_GRADE_AA;
    if (pct >= 0.80f - 0.00001f)
        return RH_GRADE_A;
    if (pct >= 0.65f - 0.00001f)
        return RH_GRADE_B;
    if (pct >= 0.45f - 0.00001f)
        return RH_GRADE_C;
    return RH_GRADE_D;
}

int RhPlay_FullCombo(const struct RhPlay *p)
{
    if (p->failed || p->rowsTotal == 0 || p->rowCount[RH_J_GOOD] + p->rowCount[RH_J_BOO] + p->rowCount[RH_J_MISS] > 0)
        return 0;
    if (p->ng || p->minesHit || p->rowCount[RH_J_GREAT])
        return 1;
    return p->rowCount[RH_J_PERFECT] ? 2 : 3;
}
