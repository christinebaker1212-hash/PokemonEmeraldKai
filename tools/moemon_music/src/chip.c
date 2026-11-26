// Moemon Music Studio: CHIPTUNE (chip.h says what and how). Three steps:
//   1. capture: the song through the game's engine, every voice read each frame (ChipEv)
//   2. arrange: each frame's notes onto the chip's four channels (ChipOut), at the chip's own frame rate
//   3. write: the channels as the chip's register writes, a stream the driver (gen_chip_drivers.py) plays,
//      in an NSF (NES) or GBS (Game Boy) file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "chip.h"
#include "chip_drivers.h"
#include "rip.h"
#include "engine.h"

#define SRC_FPS ((double)ENG_RATE / ENG_FRAME_SAMPLES)     // the game's frames (59.98 Hz)
#define GB_FPS (4194304.0 / 70224.0)                         // the Game Boy's vertical blank (59.73 Hz)
#define NES_CPU 1789772.72727
#define NSF_SPEED_US 16671                                   // the NSF plays at the game's frame rate
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define BLK 64                                               // sample analysis block (samples)
#define MAGIC "MMSCHIP1"                                     // a file this studio wrote (chip_file_info)

// the engine's (engine.c) instrument types and GB register offsets
#define TYPE_FIX 0x08
#define TYPE_RHY 0x80
enum {
    E_NR11 = 0x02, E_NR13 = 0x04, E_NR14 = 0x05, E_NR21 = 0x08, E_NR23 = 0x0C, E_NR24 = 0x0D,
    E_NR30 = 0x10, E_NR32 = 0x13, E_NR33 = 0x14, E_NR34 = 0x15, E_NR43 = 0x1C, E_NR51 = 0x21, E_NR52 = 0x24, E_WAVE = 0x30,
};

const char *chip_ext(int target) { return target == CHIP_GB ? ".gbs" : ".nsf"; }
const char *chip_name(int target) { return target == CHIP_GB ? "Game Boy" : "NES"; }

void chip_opts_default(ChipOpts *o, int target)
{
    memset(o, 0, sizeof *o);
    o->target = target;
    o->arpFrames = 0;
}

// ------------------------------------------------------------------ 1. capture

// an instrument's sound, analysed once
enum { TIM_SAMPLE, TIM_SQUARE, TIM_WAVE, TIM_NOISE };
typedef struct {
    int kind;
    const void *key;          // the EWave, or the CGB wave's 16 bytes; squares / noise: NULL (kind + duty)
    int duty;                 // the square duty that sounds closest (0 12.5%, 1 25%, 2 50%)
    int hasDuty;              // worked out (a sample's is, the first time it's heard at a known pitch)
    uint8_t wave[32];         // as the Game Boy's wave channel (4 bits a step)
    int hasWave;
    int nblk;
    float *rms, *zcr;         // per BLK samples: RMS (int8 units), zero crossings per sample
    double *cum;              // the samples' squares summed up to each point (a span's RMS)
    int len;
} Timbre;

typedef struct {
    int16_t track;            // the engine's track (a part of the song), -1 = not known
    int16_t timbre;
    uint8_t drum;
    uint32_t note;            // the note (EChan.noteId); 0 = none
    float semis;              // MIDI key with bends and vibrato (drums: unused)
    float amp;                // loudness as heard (the engine's own scale)
    float pan;                // -1 left .. 1 right
    float noiseHz;            // drums: the noise clock that sounds most like it now
} ChipEv;

typedef struct {
    int nframes, loopFrame;   // capture frames (game frames); loopFrame -1 = none
    int *first;               // [nframes + 1]: each frame's events start here
    ChipEv *ev;
    int nev, capEv;
    Timbre *tim;
    int ntim, capTim;
    int ntracks;
} Cap;

static void cap_free(Cap *c)
{
    for (int i = 0; i < c->ntim; i++) { free(c->tim[i].rms); free(c->tim[i].zcr); free(c->tim[i].cum); }
    free(c->tim);
    free(c->ev);
    free(c->first);
    memset(c, 0, sizeof *c);
}

static int timbre_of(Cap *c, int kind, const void *key, int duty)
{
    for (int i = 0; i < c->ntim; i++)
        if (c->tim[i].kind == kind && c->tim[i].key == key && ((kind != TIM_SQUARE && kind != TIM_NOISE) || c->tim[i].duty == duty)) return i;
    if (c->ntim == c->capTim) {
        int n = c->capTim ? c->capTim * 2 : 32;
        Timbre *t = realloc(c->tim, sizeof(Timbre) * (size_t)n);
        if (!t) return -1;
        c->tim = t;
        c->capTim = n;
    }
    Timbre *t = &c->tim[c->ntim];
    memset(t, 0, sizeof *t);
    t->kind = kind;
    t->key = key;
    t->duty = duty < 0 ? 2 : duty;
    if (kind == TIM_SQUARE) {
        t->hasDuty = 1;
        // the square as 32 wave steps (the Game Boy's wave channel playing it)
        static const int k_high[4] = {4, 8, 16, 24};
        for (int k = 0; k < 32; k++) t->wave[k] = k < k_high[duty & 3] ? 15 : 0;
        t->hasWave = 1;
    } else if (kind == TIM_WAVE && key) {
        const uint8_t *w = key;
        for (int k = 0; k < 16; k++) { t->wave[k * 2] = w[k] >> 4; t->wave[k * 2 + 1] = w[k] & 15; }
        t->hasWave = 1;
        t->duty = 2;
        t->hasDuty = 1;
    } else if (kind == TIM_SAMPLE && key) {
        const EWave *w = key;
        int n = (int)w->size;
        t->nblk = n / BLK + 1;
        t->rms = calloc((size_t)t->nblk, sizeof(float));
        t->zcr = calloc((size_t)t->nblk, sizeof(float));
        t->cum = malloc(sizeof(double) * ((size_t)n + 1));
        t->len = n;
        if (t->cum) {
            t->cum[0] = 0;
            for (int i = 0; i < n; i++) t->cum[i + 1] = t->cum[i] + (double)w->data[i] * w->data[i];
        }
        if (t->rms && t->zcr) {
            for (int b = 0; b < t->nblk; b++) {
                int a = b * BLK, e = a + BLK > n ? n : a + BLK;
                double sq = 0;
                int cross = 0, last = 0;
                for (int i = a; i < e; i++) {
                    int v = w->data[i];
                    sq += (double)v * v;
                    // crossings with a little hysteresis, so a quiet hiss doesn't count as a high one
                    int sgn = v > 2 ? 1 : v < -2 ? -1 : 0;
                    if (sgn && last && sgn != last) cross++;
                    if (sgn) last = sgn;
                }
                t->rms[b] = e > a ? (float)sqrt(sq / (e - a)) : 0;
                t->zcr[b] = e > a ? (float)cross / (e - a) : 0;
            }
        }
    }
    return c->ntim++;
}

// A sample's tone at a known period P (samples): which square duty it's closest to, and its shape as 32 steps.
static void analyse_tone(Timbre *t, const EWave *w, double P)
{
    t->hasDuty = 1;
    int n = (int)w->size;
    if (P < 4 || !w->data || n < 64) { t->duty = 2; return; }
    int start = w->loops && (int)w->loopStart < n - (int)(4 * P) ? (int)w->loopStart : n / 4;
    int nper = (int)((n - start) / P);
    if (nper > 8) nper = 8;
    if (nper < 2) { t->duty = 2; return; }
    int W = (int)(nper * P);
    // harmonics 1-8 (Goertzel-style single bins over whole periods, Hann window)
    double h[9] = {0};
    for (int k = 1; k <= 8; k++) {
        double re = 0, im = 0, wk = 2 * M_PI * k / P;
        for (int i = 0; i < W; i++) {
            double win = 0.5 - 0.5 * cos(2 * M_PI * i / W);
            double v = w->data[start + i] * win;
            re += v * cos(wk * i);
            im -= v * sin(wk * i);
        }
        h[k] = sqrt(re * re + im * im);
    }
    if (h[1] <= 0) { t->duty = 2; return; }
    static const double k_duty[3] = {0.125, 0.25, 0.5};
    double best = 1e30;
    for (int d = 0; d < 3; d++) {
        double dist = 0, s1 = sin(M_PI * k_duty[d]);
        for (int k = 2; k <= 8; k++) {
            double D = fabs(sin(M_PI * k * k_duty[d])) / (k * s1);
            double H = h[k] / h[1];
            if (H > 1.5) H = 1.5;
            double e = log(H + 0.03) - log(D + 0.03);
            dist += e * e / k;     // the low harmonics matter most
        }
        if (dist < best) { best = dist; t->duty = d; }
    }
    // its shape: the periods averaged, if they repeat (a steady tone); else a soft triangle
    float cyc[32], prev[32];
    double corrSum = 0;
    int corrN = 0;
    memset(cyc, 0, sizeof cyc);
    for (int m = 0; m < nper; m++) {
        float one[32];
        for (int j = 0; j < 32; j++) {
            double x = start + m * P + j * P / 32.0;
            int i0 = (int)x;
            double fr = x - i0;
            int i1 = i0 + 1 < n ? i0 + 1 : i0;
            one[j] = (float)(w->data[i0] * (1 - fr) + w->data[i1] * fr);
            cyc[j] += one[j];
        }
        if (m) {
            double ab = 0, aa = 0, bb = 0;
            for (int j = 0; j < 32; j++) { ab += one[j] * prev[j]; aa += one[j] * one[j]; bb += prev[j] * prev[j]; }
            if (aa > 0 && bb > 0) { corrSum += ab / sqrt(aa * bb); corrN++; }
        }
        memcpy(prev, one, sizeof one);
    }
    float lo = 1e9f, hi = -1e9f;
    for (int j = 0; j < 32; j++) { if (cyc[j] < lo) lo = cyc[j]; if (cyc[j] > hi) hi = cyc[j]; }
    if (corrN && corrSum / corrN > 0.8 && hi > lo) {
        for (int j = 0; j < 32; j++) t->wave[j] = (uint8_t)lrintf((cyc[j] - lo) / (hi - lo) * 15.0f);
    } else {
        for (int j = 0; j < 32; j++) t->wave[j] = (uint8_t)(j < 16 ? j : 31 - j);   // 0..15..0
    }
    t->hasWave = 1;
}

static int add_ev(Cap *c, const ChipEv *x)
{
    if (c->nev == c->capEv) {
        int n = c->capEv ? c->capEv * 2 : 4096;
        ChipEv *e = realloc(c->ev, sizeof(ChipEv) * (size_t)n);
        if (!e) return 0;
        c->ev = e;
        c->capEv = n;
    }
    c->ev[c->nev++] = *x;
    return 1;
}

static float hz_to_semis(double hz) { return hz > 0 ? (float)(69.0 + 12.0 * log2(hz / 440.0)) : 0; }
static double semis_to_hz(double s) { return 440.0 * pow(2.0, (s - 69.0) / 12.0); }

static int capture(Studio *s, Cap *c, char *msg, int msglen)
{
    memset(c, 0, sizeof *c);
    if (!s->haveConv || s->song.ntracks <= 0) { snprintf(msg, msglen, "there's no song open"); return 0; }
    int total = s->totalFrames > 0 ? s->totalFrames : 60 * 60 * 3;
    int loop = s->loopStartFrame >= 0 && s->loopStartFrame < total ? s->loopStartFrame : -1;
    // without a loop, the last notes ring out
    int nf = loop >= 0 ? total : total + (int)(SRC_FPS * 2);
    c->nframes = nf;
    c->loopFrame = loop;
    c->ntracks = s->song.ntracks;
    c->first = malloc(sizeof(int) * (size_t)(nf + 1));
    Engine *e = malloc(sizeof(Engine));
    if (!c->first || !e) { free(e); snprintf(msg, msglen, "out of memory"); return 0; }
    // every track and voice heard (nothing cut, as when GBA accurate is off), 8-bit samples (their position is read)
    ESong song = s->song;
    song.trackLimit = ENG_MAX_TRACKS;
    song.dsVoices = ENG_DS_MAX;
    song.hifi = 0;
    song.nativeRate = 0;
    eng_init(e);
    eng_start(e, &song);
    static float buf[ENG_FRAME_SAMPLES * 2];
    int16_t lastTrack[ENG_DS_MAX + 4];
    for (int i = 0; i < ENG_DS_MAX + 4; i++) lastTrack[i] = -1;
    for (int f = 0; f < nf; f++) {
        eng_frame(e, buf);
        c->first[f] = c->nev;
        // sample channels
        for (int i = 0; i < e->numChans; i++) {
            EChan *ch = &e->chans[i];
            if (!ch->status || !ch->wav || !ch->wav->data) continue;
            ETrack *t = ch->track;
            if (t) lastTrack[i] = (int16_t)(t - e->tracks);
            ChipEv x;
            memset(&x, 0, sizeof x);
            x.track = lastTrack[i];
            x.note = ch->noteId;
            int ti = timbre_of(c, TIM_SAMPLE, ch->wav, -1);
            if (ti < 0) continue;
            Timbre *tm = &c->tim[ti];
            x.timbre = (int16_t)ti;
            int blk = ch->cur / BLK;
            if (blk < 0) blk = 0;
            if (blk >= tm->nblk) blk = tm->nblk - 1;
            // the RMS of what this frame played (from where it was a frame ago to where it is), so it follows the
            // sound's own loudness and not its waveform
            double rate0 = (ch->type & TYPE_FIX) ? ENG_RATE : (double)ch->freq;
            int span = (int)(rate0 / SRC_FPS) + 1, hiEnd = ch->cur, loEnd;
            if (span < BLK) span = BLK;
            if (hiEnd > tm->len) hiEnd = tm->len;
            if (hiEnd < span) hiEnd = span < tm->len ? span : tm->len;
            loEnd = hiEnd - span < 0 ? 0 : hiEnd - span;
            float rms = tm->cum && hiEnd > loEnd ? (float)sqrt((tm->cum[hiEnd] - tm->cum[loEnd]) / (hiEnd - loEnd)) : tm->rms ? tm->rms[blk] : 64;
            int v = ch->envelopeVol * (ENG_MASTER_VOL + 1) / 16;
            float l = rms * (ch->leftVol * v / 256) / 32768.0f, r = rms * (ch->rightVol * v / 256) / 32768.0f;
            x.amp = l > r ? l : r;
            x.pan = l + r > 0 ? (r - l) / (l + r) : 0;
            if (x.amp <= 0) continue;
            double rate = (ch->type & TYPE_FIX) ? ENG_RATE : (double)ch->freq;
            x.drum = t ? (t->instrument.type & TYPE_RHY) != 0 : 0;
            if (x.drum) {
                // a noise clock that changes level as often as the drum does here (an LFSR changes on half its clocks)
                float z = tm->zcr ? tm->zcr[blk] : 0.1f;
                x.noiseHz = (float)(2.0 * z * rate);
                if (x.noiseHz < 50) x.noiseHz = 50;
            } else {
                double key = ch->key + (t ? t->keyShiftCalculated + t->pitchCalculated / 256.0 : 0);
                x.semis = (float)key;
                if (!tm->hasDuty && !(ch->type & TYPE_FIX)) analyse_tone(tm, ch->wav, rate / semis_to_hz(key));
            }
            if (!add_ev(c, &x)) { free(e); snprintf(msg, msglen, "out of memory"); return 0; }
        }
        // the Game Boy channels, read from the engine's own registers
        const uint8_t *io = e->io;
        if (io[E_NR52] & 0x80) {
            for (int k = 0; k < 4; k++) {
                EChan *ch = &e->cgb[k];
                if (!(ch->status & 0xC7) || !(io[E_NR52] & (1 << k))) continue;
                ETrack *t = ch->track;
                if (t) lastTrack[ENG_DS_MAX + k] = (int16_t)(t - e->tracks);
                ChipEv x;
                memset(&x, 0, sizeof x);
                x.track = lastTrack[ENG_DS_MAX + k];
                x.note = ch->noteId;
                int L = io[E_NR51] >> (4 + k) & 1, R = io[E_NR51] >> k & 1;
                x.pan = (float)(R - L);
                if (k < 2) {
                    if (!e->gb.DAC[k] || !e->gb.Vol[k]) continue;
                    int fr = (k == 0 ? (io[E_NR13] | io[E_NR14] << 8) : (io[E_NR23] | io[E_NR24] << 8)) & 0x7FF;
                    x.semis = hz_to_semis(131072.0 / (2048 - fr));
                    x.amp = e->gb.Vol[k] / 15.0f * 0.25f;
                    x.timbre = (int16_t)timbre_of(c, TIM_SQUARE, NULL, (k == 0 ? io[E_NR11] : io[E_NR21]) >> 6);
                } else if (k == 2) {
                    if (!(io[E_NR30] & 0x80) || !e->gb.Vol[2]) continue;
                    int fr = (io[E_NR33] | io[E_NR34] << 8) & 0x7FF;
                    x.semis = hz_to_semis(65536.0 / (2048 - fr));
                    x.amp = e->gb.Vol[2] / 4.0f * 0.25f * 0.6f;   // (a wave's RMS is below a square's)
                    x.timbre = (int16_t)timbre_of(c, TIM_WAVE, (const void *)ch->wavePointer, -1);
                } else {
                    if (!e->gb.DAC[3] || !e->gb.Vol[3]) continue;
                    int nr43 = io[E_NR43], rr = nr43 & 7, sh = nr43 >> 4;
                    x.noiseHz = (float)(524288.0 / (rr ? rr : 0.5) / (double)(2 << sh));
                    x.amp = e->gb.Vol[3] / 15.0f * 0.25f;
                    x.drum = 1;
                    x.timbre = (int16_t)timbre_of(c, TIM_NOISE, NULL, nr43 >> 3 & 1);
                }
                if (x.timbre < 0 || !add_ev(c, &x)) { free(e); snprintf(msg, msglen, "out of memory"); return 0; }
            }
        }
    }
    c->first[nf] = c->nev;
    free(e);
    return 1;
}

// ------------------------------------------------------------------ 2. arrange
// Note by note, not frame by frame. What each part of the song is, from how its notes behave (never from which
// instrument plays it, so any song works the same way):
//   - an ECHO plays notes another part (or the same part) played a steady few frames before, quieter;
//   - a DOUBLING plays the same notes at the same moment as a stronger part (in unison or octaves): one note;
//   - the BASS is the lowest steady part; the MELODY, in each stretch of the song, the part that moves most from note
//     to note (an ostinato repeating one note, a pad of held notes or a chord isn't one);
//   - the rest accompany: harmonies (moving parts sounding with the melody), ostinatos, pads; echoes last.
// The channels then take whole notes: a note keeps its channel to its end, and only a new note that matters more
// takes a channel over. Square 2 prefers the note that harmonizes with the melody (a 3rd or 6th from it before a
// 5th or 4th, a unison or octave of it least). Each part keeps one octave (moved only if it can't be played where it
// is), so a melody's shape and a sweep's path stay as they were.

// one channel in one frame
typedef struct {
    uint8_t on;
    uint8_t newNote;          // starts a note (retrigger)
    uint8_t vol;              // 0-15
    uint8_t duty;             // squares: 0-2
    uint8_t pan;              // Game Boy: bit 0 left, bit 1 right
    int16_t wave;             // Game Boy wave channel: the timbre whose shape it plays
    uint8_t shortNoise;
    float hz;                 // tone: its frequency; noise: its clock
    uint32_t note;
} ChOut;

typedef struct {
    int nframes, loopFrame;   // chip frames
    ChOut (*ch)[4];           // [frame][square 1, square 2, triangle / wave, noise]
    // what happened, for the message
    int parts, drumParts, echoes, doublings, notesHeard, notesTotal, moved, bass;
} Arr;

enum { ROLE_NONE, ROLE_BASS, ROLE_HARMONY, ROLE_OSTINATO, ROLE_PAD };

typedef struct {
    uint32_t id;
    int part, f0, f1;         // capture frames [f0, f1): while it's loud enough to hear
    float key0, keyMin, keyMax;
    float ampPeak, ampSum;
    int16_t timbre;
    uint8_t drum, echo;
    int dupOf;                // the note (index) it doubles, -1 none
    int shift;                // octaves it's moved by (12s), decided the first time it's placed
    uint8_t shiftSet, heard;
    uint8_t lost;             // another note took its channel: it doesn't come back (no flicker between two notes)
} Note;

typedef struct {
    int notes, frames;
    float keyMed, ampMean, durMed;
    int moves, repeats, poly;
    int harmonyOf;            // the part it moves with in the same rhythm on other notes, below it (its harmony), -1
    int role;
    int echoOf, dupOf;        // the part it echoes / doubles (-1)
    int shift;                // octaves the whole part moves (12s)
} Part;

static int cmp_float(const void *a, const void *b)
{
    float x = *(const float *)a, y = *(const float *)b;
    return x < y ? -1 : x > y;
}

static const Note *g_sortNotes;
static int cmp_note_start(const void *a, const void *b)
{
    const Note *x = &g_sortNotes[*(const int *)a], *y = &g_sortNotes[*(const int *)b];
    return x->f0 != y->f0 ? x->f0 - y->f0 : x->key0 < y->key0 ? -1 : x->key0 > y->key0;
}

// an echo or a doubling is the same note: as long as the other (within a factor of 2.5)
static int same_len(const Note *a, const Note *b)
{
    float x = (float)(a->f1 - a->f0), y = (float)(b->f1 - b->f0);
    return x <= y * 2.5f + 2 && y <= x * 2.5f + 2;
}

static int key_rel_ok(float a, float b)
{
    int d = (int)lroundf(a - b);
    return d == 0 || d == 12 || d == -12 || d == 24 || d == -24;
}

#define LEAD_WINDOW 240        // capture frames (4 s): the melody is chosen for each stretch this long
#define ECHO_MAX 60            // the longest echo delay looked for (frames)

static int arrange(const Cap *c, int target, int arpFrames, Arr *a, char *msg, int msglen)
{
    memset(a, 0, sizeof *a);
    double fps = target == CHIP_GB ? GB_FPS : SRC_FPS;
    int nf = (int)floor(c->nframes * fps / SRC_FPS);
    if (nf < 1) nf = 1;
    a->nframes = nf;
    a->loopFrame = c->loopFrame >= 0 ? (int)lround(c->loopFrame * fps / SRC_FPS) : -1;
    if (a->loopFrame >= nf) a->loopFrame = -1;
    a->ch = calloc((size_t)nf, sizeof *a->ch);
    if (!a->ch) { snprintf(msg, msglen, "out of memory"); return 0; }

    // the loudness a channel's full volume stands for: the loud end of what single notes reach
    float aref = 0.25f;
    {
        float *amps = malloc(sizeof(float) * (size_t)(c->nev + 1));
        if (!amps) { snprintf(msg, msglen, "out of memory"); return 0; }
        int na = 0;
        for (int i = 0; i < c->nev; i++) if (c->ev[i].amp > 0) amps[na++] = c->ev[i].amp;
        if (na) { qsort(amps, (size_t)na, sizeof(float), cmp_float); aref = amps[(int)(na * 0.97)]; }
        free(amps);
        if (aref <= 0) aref = 0.25f;
    }
    float quiet = aref * 0.035f;       // below this a note isn't worth a channel (it would be volume 0)

    // ---- the notes
    uint32_t maxId = 0;
    for (int i = 0; i < c->nev; i++) if (c->ev[i].note > maxId) maxId = c->ev[i].note;
    int *idx = malloc(sizeof(int) * (size_t)(maxId + 1));
    Note *nt = calloc((size_t)maxId + 1, sizeof(Note));
    int nn = 0, np = c->ntracks + 1;   // (the last part: notes whose part isn't known)
    Part *pt = calloc((size_t)np, sizeof(Part));
    if (!idx || !nt || !pt) { free(idx); free(nt); free(pt); snprintf(msg, msglen, "out of memory"); return 0; }
    for (uint32_t i = 0; i <= maxId; i++) idx[i] = -1;
    for (int f = 0; f < c->nframes; f++)
        for (int i = c->first[f]; i < c->first[f + 1]; i++) {
            const ChipEv *x = &c->ev[i];
            if (!x->note || x->amp < quiet) continue;
            int k = idx[x->note];
            if (k < 0) {
                k = idx[x->note] = nn++;
                Note *n = &nt[k];
                n->id = x->note;
                n->part = x->track >= 0 && x->track < np - 1 ? x->track : np - 1;
                n->f0 = f;
                n->key0 = n->keyMin = n->keyMax = x->semis;
                n->timbre = x->timbre;
                n->drum = x->drum;
                n->dupOf = -1;
            }
            Note *n = &nt[k];
            n->f1 = f + 1;
            if (x->amp > n->ampPeak) n->ampPeak = x->amp;
            n->ampSum += x->amp;
            if (!x->drum) {
                if (x->semis < n->keyMin) n->keyMin = x->semis;
                if (x->semis > n->keyMax) n->keyMax = x->semis;
            }
        }
    a->notesTotal = nn;
    // the notes in order of their start, and each part's
    int *order = malloc(sizeof(int) * (size_t)(nn + 1));
    int *byStart = calloc((size_t)c->nframes + 1, sizeof(int));   // first note (in order) starting at or after a frame
    if (!order || !byStart) { free(idx); free(nt); free(pt); free(order); free(byStart); snprintf(msg, msglen, "out of memory"); return 0; }
    for (int i = 0; i < nn; i++) order[i] = i;
    g_sortNotes = nt;
    qsort(order, (size_t)nn, sizeof(int), cmp_note_start);
    {
        int j = 0;
        for (int f = 0; f <= c->nframes; f++) {
            while (j < nn && nt[order[j]].f0 < f) j++;
            byStart[f] = j;
        }
    }
#define NOTES_FROM(f) byStart[(f) < 0 ? 0 : (f) > c->nframes ? c->nframes : (f)]

    // ---- echoes and doublings between parts: for each pair, the delay at which most of one part's notes are the
    // other's (same key, or octaves of it), within a frame
    for (int i = 0; i < np; i++) { pt[i].echoOf = pt[i].dupOf = pt[i].harmonyOf = -1; }
    for (int i = 0; i < nn; i++) if (!nt[i].drum) { pt[nt[i].part].notes++; pt[nt[i].part].ampMean += nt[i].ampSum / (nt[i].f1 - nt[i].f0); }
    for (int p = 0; p < np; p++) if (pt[p].notes) pt[p].ampMean /= pt[p].notes;
    {
        int *hist = malloc(sizeof(int) * (ECHO_MAX + 1));
        for (int b = 0; b < np && hist; b++) {
            if (pt[b].notes < 6) continue;
            int bestA = -1, bestD = 0, bestM = 0;
            for (int ap = 0; ap < np; ap++) {
                if (ap == b || pt[ap].notes < 4) continue;
                memset(hist, 0, sizeof(int) * (ECHO_MAX + 1));
                for (int q = 0; q < nn; q++) {
                    const Note *n = &nt[order[q]];
                    if (n->part != b || n->drum) continue;
                    // the A notes that start up to ECHO_MAX frames before it (a delay counted once per B note)
                    uint8_t got[ECHO_MAX + 1];
                    memset(got, 0, sizeof got);
                    for (int r = NOTES_FROM(n->f0 - ECHO_MAX - 1); r < nn && nt[order[r]].f0 <= n->f0 + 1; r++) {
                        const Note *m = &nt[order[r]];
                        if (m->part != ap || m->drum || !key_rel_ok(n->key0, m->key0) || !same_len(n, m)) continue;
                        int d = n->f0 - m->f0;
                        if (d < -1 || d > ECHO_MAX) continue;
                        if (d < 0) d = 0;
                        got[d] = 1;
                    }
                    for (int d = 0; d <= ECHO_MAX; d++) if (got[d] || (d > 0 && got[d - 1]) || (d < ECHO_MAX && got[d + 1])) hist[d]++;
                }
                for (int d = 0; d <= ECHO_MAX; d++)
                    if (hist[d] > bestM) { bestM = hist[d]; bestD = d; bestA = ap; }
            }
            if (bestA < 0 || bestM < pt[b].notes * 0.55) continue;
            if (bestD <= 2) {
                // a doubling: the part with fewer notes (or the quieter) doubles the other
                if (pt[bestA].dupOf == b) continue;
                if (pt[b].notes > pt[bestA].notes || (pt[b].notes == pt[bestA].notes && pt[b].ampMean > pt[bestA].ampMean)) continue;
                pt[b].dupOf = bestA;
            } else if (pt[b].ampMean <= pt[bestA].ampMean * 1.1f) {
                pt[b].echoOf = bestA;
                // (its own delay, kept in dupOf's place for the per-note marking below)
                pt[b].shift = bestD;
            }
        }
        free(hist);
    }
    // the notes that are an echo or a doubling of another note
    for (int q = 0; q < nn; q++) {
        Note *n = &nt[order[q]];
        if (n->drum) continue;
        Part *p = &pt[n->part];
        if (p->echoOf >= 0) {
            int d = p->shift;
            for (int r = NOTES_FROM(n->f0 - d - 2); r < nn && nt[order[r]].f0 <= n->f0 - d + 2; r++) {
                const Note *m = &nt[order[r]];
                if (m->part == p->echoOf && key_rel_ok(n->key0, m->key0) && same_len(n, m)) { n->echo = 1; break; }
            }
        }
        if (p->dupOf >= 0) {
            for (int r = NOTES_FROM(n->f0 - 2); r < nn && nt[order[r]].f0 <= n->f0 + 2; r++) {
                const Note *m = &nt[order[r]];
                if (m->part == p->dupOf && key_rel_ok(n->key0, m->key0) && same_len(n, m)) { n->dupOf = order[r]; break; }
            }
        }
    }
    for (int i = 0; i < np; i++) pt[i].shift = 0;
    // an echo inside one part (a rip's lane can hold both): notes repeating a louder note of their own part a steady
    // number of frames later
    {
        int hist[ECHO_MAX + 1];
        for (int p = 0; p < np; p++) {
            if (pt[p].notes < 12 || pt[p].echoOf >= 0) continue;
            memset(hist, 0, sizeof hist);
            for (int q = 0; q < nn; q++) {
                const Note *n = &nt[order[q]];
                if (n->part != p || n->drum) continue;
                for (int r = NOTES_FROM(n->f0 - ECHO_MAX); r < q; r++) {
                    const Note *m = &nt[order[r]];
                    int d = n->f0 - m->f0;
                    if (m->part == p && d >= 3 && d <= ECHO_MAX && fabsf(m->key0 - n->key0) < 0.5f && n->ampPeak < m->ampPeak * 0.8f) hist[d]++;
                }
            }
            int bd = 0;
            for (int d = 3; d <= ECHO_MAX; d++) if (hist[d] + (d > 3 ? hist[d - 1] : 0) > hist[bd] + (bd > 3 ? hist[bd - 1] : 0)) bd = d;
            if (!bd || hist[bd] + hist[bd - 1] < pt[p].notes * 0.3f) continue;
            for (int q = 0; q < nn; q++) {
                Note *n = &nt[order[q]];
                if (n->part != p || n->drum) continue;
                for (int r = NOTES_FROM(n->f0 - bd - 1); r < q; r++) {
                    const Note *m = &nt[order[r]];
                    int d = n->f0 - m->f0;
                    if (m->part == p && d >= bd - 1 && d <= bd + 1 && fabsf(m->key0 - n->key0) < 0.5f && n->ampPeak < m->ampPeak * 0.8f) { n->echo = 1; break; }
                }
            }
        }
    }
    for (int i = 0; i < nn; i++) { if (nt[i].echo) a->echoes++; if (nt[i].dupOf >= 0) a->doublings++; }

    // ---- what each part is (from its own notes: not the echoes or doublings)
    {
        float *keys = malloc(sizeof(float) * (size_t)(nn + 1)), *durs = malloc(sizeof(float) * (size_t)(nn + 1));
        int *lastF = malloc(sizeof(int) * (size_t)np);
        float *lastK = malloc(sizeof(float) * (size_t)np);
        for (int p = 0; keys && durs && lastF && lastK && p < np; p++) {
            Part *P = &pt[p];
            int k = 0, active = 0, maxPoly = 0;
            lastF[p] = -100;
            lastK[p] = -1;
            for (int q = 0; q < nn; q++) {
                const Note *n = &nt[order[q]];
                if (n->part != p || n->drum || n->echo || n->dupOf >= 0) continue;
                keys[k] = n->key0;
                durs[k] = (float)(n->f1 - n->f0);
                k++;
                if (n->f0 > lastF[p] + 1) {
                    if (lastK[p] >= 0) { if (fabsf(n->key0 - lastK[p]) > 0.5f) P->moves++; else P->repeats++; }
                    lastK[p] = n->key0;
                }
                lastF[p] = n->f0;
                int poly = 0;
                for (int r = NOTES_FROM(n->f0 - 600); r < nn && nt[order[r]].f0 <= n->f0; r++) {
                    const Note *m = &nt[order[r]];
                    if (m->part == p && m->f1 > n->f0 && !m->echo && m->dupOf < 0) poly++;
                }
                if (poly > maxPoly) maxPoly = poly;
                active += n->f1 - n->f0;
            }
            P->notes = k;
            P->frames = active;
            P->poly = maxPoly;
            if (k) {
                qsort(keys, (size_t)k, sizeof(float), cmp_float);
                qsort(durs, (size_t)k, sizeof(float), cmp_float);
                P->keyMed = keys[k / 2];
                P->durMed = durs[k / 2];
                a->parts++;
            }
        }
        free(keys); free(durs); free(lastF); free(lastK);
    }
    {
        // the bass: the lowest steady part below G3 (and any other part as low, when it's silent)
        int bass = -1;
        float best = 0;
        for (int p = 0; p < np; p++) {
            Part *P = &pt[p];
            if (P->notes < 4 || P->keyMed >= 55) continue;
            float sc = (float)P->frames * (60 - P->keyMed);
            if (sc > best) { best = sc; bass = p; }
        }
        for (int p = 0; p < np; p++) {
            Part *P = &pt[p];
            if (!P->notes) continue;
            if (p == bass || (bass >= 0 && P->keyMed < 50 && P->keyMed <= pt[bass].keyMed + 5)) P->role = ROLE_BASS;
            else if (P->durMed > SRC_FPS * 0.9f) P->role = ROLE_PAD;
            else if (P->repeats > P->moves * 2) P->role = ROLE_OSTINATO;
            else P->role = ROLE_HARMONY;
        }
        a->bass = bass >= 0;
        // a song without a bass: the lowest part plays on the triangle / wave
        if (bass < 0) {
            int low = -1;
            for (int p = 0; p < np; p++) if (pt[p].notes >= 4 && (low < 0 || pt[p].keyMed < pt[low].keyMed)) low = p;
            if (low >= 0) pt[low].role = ROLE_BASS;
        }
        for (int i = 0; i < np; i++) if (pt[i].echoOf >= 0 || pt[i].dupOf >= 0) { if (pt[i].role == ROLE_BASS && pt[i].dupOf >= 0 && pt[pt[i].dupOf].role != ROLE_BASS) pt[i].role = ROLE_HARMONY; }
    }
    for (int i = 0; i < nn; i++) if (nt[i].drum && !pt[nt[i].part].notes) { a->drumParts = 1; break; }
    // two parts moving in the same rhythm on other notes: a melody harmonized (in 3rds...) or stabs; the lower one is
    // the upper one's harmony
    {
        int *co = calloc((size_t)np * np, sizeof(int));
        for (int q = 0; co && q < nn; q++) {
            const Note *n = &nt[order[q]];
            if (n->drum || n->echo || n->dupOf >= 0) continue;
            int role = pt[n->part].role;
            if (role == ROLE_BASS || role == ROLE_OSTINATO) continue;
            uint8_t seen[256] = {0};
            for (int r = NOTES_FROM(n->f0 - 1); r < nn && nt[order[r]].f0 <= n->f0 + 1; r++) {
                const Note *m = &nt[order[r]];
                if (m->part == n->part || m->drum || m->echo || m->dupOf >= 0 || m->part > 255 || seen[m->part]) continue;
                if (((int)lroundf(fabsf(m->key0 - n->key0))) % 12 == 0) continue;
                seen[m->part] = 1;
                co[n->part * np + m->part]++;
            }
        }
        for (int p = 0; co && p < np; p++) {
            if (pt[p].notes < 6 || pt[p].role == ROLE_BASS || pt[p].role == ROLE_OSTINATO) continue;
            int best = -1;
            float bf = 0;
            for (int q = 0; q < np; q++) {
                if (q == p || pt[q].notes < 6) continue;
                float f1 = (float)co[p * np + q] / pt[p].notes, f2 = (float)co[q * np + p] / pt[q].notes;
                if (f1 >= 0.45f && f2 >= 0.45f && f1 > bf) { bf = f1; best = q; }
            }
            if (best >= 0 && pt[p].keyMed < pt[best].keyMed) pt[p].harmonyOf = best;

        }
        free(co);
    }

    // ---- the melody of each stretch: the part that moves most from note to note, as loud, high and one note at a time
    int nwin = c->nframes / LEAD_WINDOW + 1;
    int *leadOf = malloc(sizeof(int) * (size_t)nwin);
    float *wsc = calloc((size_t)np * 4, sizeof(float));   // per part: moves, notes, amp, frames in the stretch
    float *scores = calloc((size_t)np * nwin, sizeof(float));
    if (!leadOf || !wsc || !scores) { free(idx); free(nt); free(pt); free(order); free(byStart); free(leadOf); free(wsc); free(scores); snprintf(msg, msglen, "out of memory"); return 0; }
    {
        // each stretch: how much of a melody each part is there. It moves from note to note over many pitches (an
        // ostinato or a stab repeats a few), it's loud and high and one note at a time; the lower of two parts moving
        // together is the other's harmony
        float *lastK = malloc(sizeof(float) * (size_t)np);
        uint8_t *keysUsed = malloc((size_t)np * 128);
        uint8_t *iois = malloc((size_t)np * 64);     // the gaps between its notes' starts (2-frame steps) it uses
        int *lastStart = malloc(sizeof(int) * (size_t)np);
        for (int w = 0; lastK && keysUsed && iois && lastStart && w < nwin; w++) {
            memset(wsc, 0, sizeof(float) * (size_t)np * 4);
            memset(keysUsed, 0, (size_t)np * 128);
            memset(iois, 0, (size_t)np * 64);
            for (int p = 0; p < np; p++) { lastK[p] = -1; lastStart[p] = -1; }
            for (int r = NOTES_FROM(w * LEAD_WINDOW); r < nn && nt[order[r]].f0 < (w + 1) * LEAD_WINDOW; r++) {
                const Note *n = &nt[order[r]];
                if (n->drum || n->echo || n->dupOf >= 0 || pt[n->part].role == ROLE_BASS) continue;
                float *W = &wsc[n->part * 4];
                if (lastK[n->part] >= 0 && fabsf(n->key0 - lastK[n->part]) > 0.5f) W[0] += 1;
                lastK[n->part] = n->key0;
                int kk = (int)lroundf(n->key0);
                if (kk >= 0 && kk < 128) keysUsed[n->part * 128 + kk] = 1;
                if (lastStart[n->part] >= 0 && n->f0 > lastStart[n->part] + 1) {
                    int g = (n->f0 - lastStart[n->part] + 1) / 2;
                    iois[n->part * 64 + (g < 63 ? g : 63)] = 1;
                }
                if (n->f0 > lastStart[n->part] + 1) lastStart[n->part] = n->f0;
                W[1] += 1;
                W[2] += n->ampSum;
                W[3] += (float)(n->f1 - n->f0);
            }
            for (int p = 0; p < np; p++) {
                float *W = &wsc[p * 4];
                const Part *P = &pt[p];
                if (W[1] < 2 || P->role == ROLE_BASS) continue;
                int distinct = 0, rhythms = 0;
                for (int kk = 0; kk < 128; kk++) distinct += keysUsed[p * 128 + kk];
                for (int g = 0; g < 64; g++) rhythms += iois[p * 64 + g];
                // a melody's rhythm varies; a figure keeps one (a broken chord in steady 8ths)
                float varied = 0.35f + 0.65f * (rhythms >= 4 ? 1 : rhythms <= 1 ? 0 : (rhythms - 1) / 3.0f);
                float loud = sqrtf(W[2] / (W[3] > 0 ? W[3] : 1) / aref);
                float moving = W[0] / (LEAD_WINDOW / (float)SRC_FPS) + 0.15f;
                float high = 1 + (P->keyMed - 60) / 30.0f;
                if (high < 0.4f) high = 0.4f;
                float presence = W[3] / LEAD_WINDOW;
                if (presence > 1) presence = 1;
                float poly = P->poly > 1 ? sqrtf((float)P->poly) : 1;
                float rep = P->moves + P->repeats ? (float)P->repeats / (P->moves + P->repeats) : 0;
                float sc = loud * moving * sqrtf((float)distinct) * varied * high * sqrtf(presence) / poly * (1 - 0.7f * rep);
                if (P->role == ROLE_OSTINATO) sc *= 0.3f;
                if (P->harmonyOf >= 0) sc *= 0.3f;
                scores[(size_t)w * np + p] = sc;
            }
        }
        // the choice weighs the stretches either side too, and stays with the part that has it unless another is
        // clearly more of a melody
        int cur = -1;
        for (int w = 0; w < nwin; w++) {
            int best = -1;
            float bestSc = 0, curSc = 0;
            for (int p = 0; p < np; p++) {
                float sc = scores[(size_t)w * np + p] * 2 + (w > 0 ? scores[(size_t)(w - 1) * np + p] : 0) + (w + 1 < nwin ? scores[(size_t)(w + 1) * np + p] : 0);
                if (sc > bestSc) { bestSc = sc; best = p; }
                if (p == cur) curSc = sc;
            }
            if (cur < 0 || curSc <= 0 || bestSc > curSc * 1.6f) cur = best;
            leadOf[w] = cur;
        }
        free(lastK);
        free(keysUsed);
        free(iois);
        free(lastStart);
    }
    free(wsc);
    // each stretch's strongest melody score, for square 2's "how much of a melody is it"
    float *wmax = calloc((size_t)nwin, sizeof(float));
    for (int w = 0; wmax && w < nwin; w++)
        for (int p = 0; p < np; p++) if (scores[(size_t)w * np + p] > wmax[w]) wmax[w] = scores[(size_t)w * np + p];

    // ---- each part's octave: moved as a whole when its notes don't fit the channel its role plays on
    float sqLo = target == CHIP_GB ? 36 : 33, sqHi = 108;     // Game Boy squares reach down to 64 Hz, NES 55 Hz
    float triLo = target == CHIP_GB ? 24 : 22, triHi = 100;
    for (int p = 0; p < np; p++) {
        Part *P = &pt[p];
        if (!P->notes) continue;
        float lo = P->role == ROLE_BASS ? triLo : sqLo, hi = P->role == ROLE_BASS ? triHi : sqHi;
        // its low end (the 10th percentile of its notes' lowest points)
        float *ks = malloc(sizeof(float) * (size_t)P->notes);
        int k = 0;
        for (int i = 0; i < nn && ks; i++) if (nt[i].part == p && !nt[i].drum && k < P->notes) ks[k++] = nt[i].keyMin;
        if (ks && k) {
            qsort(ks, (size_t)k, sizeof(float), cmp_float);
            float low = ks[k / 10], top = ks[k - 1];
            while (low + 12 * P->shift < lo - 0.5f) P->shift++;
            while (top + 12 * P->shift > hi + 0.5f && low + 12 * (P->shift - 1) >= lo - 0.5f) P->shift--;
        }
        free(ks);
    }

    // ---- the channels, frame by frame, a note at a time
    int curN[4] = {-1, -1, -1, -1};         // the note each channel plays (index), -1 none
    int prevOn[4] = {-1, -1, -1, -1};       // ... and played the frame before
    long arp = 0;
    int *act = malloc(sizeof(int) * 1024);
    float *actAmp = malloc(sizeof(float) * 1024), *actKey = malloc(sizeof(float) * 1024);
    float *actPan = malloc(sizeof(float) * 1024);
    float *smooth = calloc((size_t)nn + 1, sizeof(float));
    int ok = act && actAmp && actKey && actPan && smooth;
    if (!ok) snprintf(msg, msglen, "out of memory");
    for (int k = 0; ok && k < nf; k++) {
        int f = (int)lround(k * SRC_FPS / fps);
        if (f >= c->nframes) f = c->nframes - 1;
        int lead = leadOf[f / LEAD_WINDOW];
        ChOut *out = a->ch[k];
        // the notes sounding (as loud as is worth a channel)
        int na = 0;
        for (int i = c->first[f]; i < c->first[f + 1] && na < 1024; i++) {
            const ChipEv *x = &c->ev[i];
            if (!x->note || x->amp < quiet) continue;
            int ni = idx[x->note];
            if (ni < 0) continue;
            int dup = 0;
            for (int j = 0; j < na; j++) if (act[j] == ni) { dup = 1; if (x->amp > actAmp[j]) actAmp[j] = x->amp; }
            if (dup) continue;
            act[na] = ni;
            actAmp[na] = x->amp;
            actKey[na] = x->semis;
            actPan[na] = x->pan;
            na++;
        }
        int on[4] = {-1, -1, -1, -1};        // the active-list index each channel plays this frame
        // keep each channel's note while it sounds
        for (int q = 0; q < 4; q++)
            for (int j = 0; j < na; j++) if (curN[q] >= 0 && act[j] == curN[q]) on[q] = j;
        // the noise: the drums (the newest hit, else the loudest; a hit keeps it against one as loud)
        {
            int best = -1;
            float bs = 0;
            for (int j = 0; j < na; j++) {
                const Note *n = &nt[act[j]];
                if (!n->drum) continue;
                int age = f - n->f0;
                float sc = actAmp[j] * (age == 0 ? 8.0f : age == 1 ? 4.0f : age == 2 ? 2.0f : 1.0f) * (j == on[3] ? 1.25f : 1);
                if (sc > bs) { bs = sc; best = j; }
            }
            on[3] = best;
        }
        // the triangle / wave: the bass
        if (on[2] < 0 || nt[act[on[2]]].f0 == f) {
            int best = -1;
            for (int j = 0; j < na; j++) {
                const Note *n = &nt[act[j]];
                if (n->drum || pt[n->part].role != ROLE_BASS || n->echo) continue;
                if (n->dupOf >= 0 && n->dupOf != act[j]) {
                    int doubled = 0;
                    for (int q = 0; q < na; q++) if (act[q] == n->dupOf) doubled = 1;
                    if (doubled) continue;
                }
                if (best < 0 || actKey[j] < actKey[best] || (n->f0 == f && nt[act[best]].f0 != f)) best = j;
            }
            if (best >= 0) on[2] = best;
        }
        // square 1: the melody (its top note; a new melody note takes over at once), else what it was playing,
        // else the note that matters most
        {
            int top = -1;
            for (int j = 0; j < na; j++) {
                const Note *n = &nt[act[j]];
                if (n->drum || n->part != lead || n->echo || j == on[2]) continue;
                if (top < 0 || (n->f0 >= nt[act[top]].f0 && (n->f0 > nt[act[top]].f0 || actKey[j] > actKey[top]))) top = j;
            }
            // the melody as another part slides it: a note that starts with the melody's (or ends on it) and sweeps
            // is the melody's note, played with its slide
            if (top >= 0) {
                const Note *t = &nt[act[top]];
                for (int j = 0; j < na; j++) {
                    const Note *n = &nt[act[j]];
                    if (j == top || n->drum || n->lost || j == on[2] || n->keyMax - n->keyMin <= 1.5f) continue;
                    if (abs(n->f0 - t->f0) > 3) continue;
                    float d0 = fmodf(fabsf(n->key0 - t->key0), 12), d1 = fmodf(fabsf(n->keyMax - t->key0), 12), d2 = fmodf(fabsf(n->keyMin - t->key0), 12);
                    if (d0 < 0.6f || d0 > 11.4f || d1 < 0.6f || d1 > 11.4f || d2 < 0.6f || d2 > 11.4f) { top = j; break; }
                }
            }
            if (top >= 0 && (on[0] < 0 || (nt[act[on[0]]].part != lead && !(nt[act[on[0]]].keyMax - nt[act[on[0]]].keyMin > 1.5f)) || nt[act[top]].f0 == f)) {
                if (on[0] >= 0 && on[0] != top) nt[act[on[0]]].lost = 1;
                on[0] = top;
            }
        }
        // the rest: square 2 (and square 1 when the melody rests), by how much each note matters
        for (int pass = 0; pass < 2; pass++) {
            int q = pass == 0 ? 1 : 0;
            if (q == 0 && on[0] >= 0) continue;
            float melodyKey = on[0] >= 0 ? actKey[on[0]] : -100;
            int best = -1;
            float bs = -1e9f, held = -1e9f;
            for (int j = 0; j < na; j++) {
                const Note *n = &nt[act[j]];
                if (n->drum || (j == on[0] && q != 0) || (j == on[1] && q != 1) || j == on[2] || n->lost) continue;
                if (pt[n->part].role == ROLE_BASS && !n->echo) continue;          // (the bass plays on its own channel)
                // a doubling of a note that's playing is that note
                if (n->dupOf >= 0) {
                    int playing = 0;
                    for (int qq = 0; qq < 3; qq++) if (on[qq] >= 0 && act[on[qq]] == n->dupOf) playing = 1;
                    if (playing) continue;
                }
                const Part *P = &pt[n->part];
                float sc = P->role == ROLE_HARMONY ? 3 : P->role == ROLE_PAD ? 2 : P->role == ROLE_OSTINATO ? 2 : 1;
                if (n->part == lead) sc += 1.5f;                                   // the melody's own chord notes
                if (lead >= 0 && P->harmonyOf == lead) sc += 2;                    // the melody's harmony part
                if (on[0] >= 0 && abs(n->f0 - nt[act[on[0]]].f0) <= 1) sc += 1.5f;  // moving with the melody's note
                if (wmax && wmax[f / LEAD_WINDOW] > 0)                             // a melody of its own
                    sc += 2.5f * scores[(size_t)(f / LEAD_WINDOW) * np + n->part] / wmax[f / LEAD_WINDOW];
                if (n->echo) sc -= 3;
                if (n->dupOf >= 0) sc -= 1;
                if (melodyKey > -100) {
                    int iv = ((int)lroundf(fabsf(actKey[j] - melodyKey))) % 12;
                    if (iv == 3 || iv == 4 || iv == 8 || iv == 9) sc += 2;         // a 3rd or a 6th
                    else if (iv == 5 || iv == 7) sc += 1;                          // a 4th or a 5th
                    else if (iv == 0) sc -= 2.5f;                                  // the melody's own note again
                }
                for (int qq = 0; qq < 3; qq++)
                    if (on[qq] >= 0 && qq != q && ((int)lroundf(fabsf(actKey[j] - actKey[on[qq]]))) % 12 == 0) sc -= 1.5f;
                sc += sqrtf(actAmp[j] / aref);
                if (n->f0 == f) sc += 1.5f;                                        // a note starting now
                if (n->keyMax - n->keyMin > 1.5f) sc += 3.5f;                      // a sweep (its path is the point)
                if (j == on[q]) sc += 2.5f;                                        // the note the channel is playing
                if (j == on[q]) held = sc;
                if (sc > bs) { bs = sc; best = j; }
            }
            // the channel's note keeps it unless a note starting now clearly matters more
            if (best >= 0 && (on[q] < 0 || best == on[q] || (nt[act[best]].f0 == f && bs > held + 1.0f))) {
                if (on[q] >= 0 && on[q] != best) nt[act[on[q]]].lost = 1;
                on[q] = best;
            }
            // arpeggio (Taking turns): a chord in one part goes round its notes on square 2
            if (q == 1 && arpFrames > 0 && on[1] >= 0) {
                int part = nt[act[on[1]]].part, ch[16], nc = 0;
                for (int j = 0; j < na && nc < 16; j++)
                    if (!nt[act[j]].drum && nt[act[j]].part == part && j != on[0] && j != on[2] && !nt[act[j]].echo) ch[nc++] = j;
                if (nc >= 2) { on[1] = ch[(arp / arpFrames) % nc]; arp++; }
            }
        }
        // out
        for (int q = 0; q < 4; q++) {
            ChOut *o = &out[q];
            if (on[q] < 0) { curN[q] = -1; prevOn[q] = -1; continue; }
            int j = on[q];
            Note *n = &nt[act[j]];
            n->heard = 1;
            int vol = (int)lroundf(15.0f * actAmp[j] / aref);
            // the volume a little smoothed within a note (a sample's own waveform makes its level flutter)
            float sm = n->f0 == f || smooth[act[j]] <= 0 ? (float)vol : smooth[act[j]] * 0.5f + vol * 0.5f;
            smooth[act[j]] = sm;
            vol = (int)lroundf(sm);
            o->on = 1;
            o->vol = (uint8_t)(vol < 1 ? 1 : vol > 15 ? 15 : vol);
            o->note = n->id;
            o->newNote = act[j] != prevOn[q];
            o->pan = (uint8_t)(actPan[j] < -0.4f ? 1 : actPan[j] > 0.4f ? 2 : 3);
            const Timbre *tm = n->timbre >= 0 && n->timbre < c->ntim ? &c->tim[n->timbre] : NULL;
            if (q == 3) {
                for (int i = c->first[f]; i < c->first[f + 1]; i++) if (c->ev[i].note == n->id) { o->hz = c->ev[i].noiseHz; break; }
                o->shortNoise = (uint8_t)(tm && tm->kind == TIM_NOISE ? tm->duty & 1 : 0);
            } else {
                // its octave: the part's, and the note's own only if it still doesn't fit (decided once, for the whole note)
                if (!n->shiftSet) {
                    float lo = q == 2 ? triLo : sqLo, hi = q == 2 ? triHi : sqHi;
                    int sh = pt[n->part].shift;
                    while (n->keyMin + 12 * sh < lo - 0.5f) sh++;
                    while (n->keyMax + 12 * sh > hi + 0.5f) sh--;
                    if (sh) a->moved++;
                    n->shift = sh;
                    n->shiftSet = 1;
                }
                o->hz = (float)semis_to_hz(actKey[j] + 12 * n->shift);
                o->duty = (uint8_t)(tm && tm->hasDuty ? tm->duty : 2);
                o->wave = n->timbre;
            }
            curN[q] = act[j];
            prevOn[q] = act[j];
        }
    }
    // a note started again right after the same note on the same channel: a frame's gap before it, so it's heard
    // as a new note (the triangle has no attack of its own)
    for (int k = 1; ok && k < nf; k++)
        for (int q = 0; q < 3; q++) {
            ChOut *o = &a->ch[k][q], *p = &a->ch[k - 1][q];
            if (!o->on || !o->newNote || !p->on || fabsf(o->hz - p->hz) > o->hz * 0.01f) continue;
            if (k >= 3 && a->ch[k - 2][q].on && a->ch[k - 2][q].note == p->note && a->ch[k - 3][q].on && a->ch[k - 3][q].note == p->note) p->on = 0;
        }
    // the parts' own notes (an echo or a doubling of a note that played isn't a note missed)
    a->notesTotal = 0;
    for (int i = 0; i < nn; i++)
        if (!nt[i].echo && nt[i].dupOf < 0) { a->notesTotal++; if (nt[i].heard) a->notesHeard++; }
    if (getenv("MMS_CHIP_DEBUG") && ok) {
        static const char *const k_role[] = {"-", "bass", "harmony", "ostinato", "pad"};
        for (int p = 0; p < np; p++)
            if (pt[p].notes || pt[p].echoOf >= 0)
                fprintf(stderr, "part %d: %s, key %.1f, %d notes, %d moves %d repeats, harmony of %d, poly %d, dur %.0f%s%s, octave %+d\n", p, k_role[pt[p].role],
                        pt[p].keyMed, pt[p].notes, pt[p].moves, pt[p].repeats, pt[p].harmonyOf, pt[p].poly, pt[p].durMed,
                        pt[p].echoOf >= 0 ? " (an echo)" : "", pt[p].dupOf >= 0 ? " (a doubling)" : "", pt[p].shift);
        fprintf(stderr, "echo notes %d, doubled notes %d; melody by stretch:", a->echoes, a->doublings);
        for (int w = 0; w < nwin; w++) fprintf(stderr, " %d", leadOf[w]);
        fprintf(stderr, "\n");
    }
#undef NOTES_FROM
    free(act); free(actAmp); free(actKey); free(actPan); free(smooth);
    free(scores); free(wmax);
    free(idx); free(nt); free(pt); free(order); free(byStart); free(leadOf);
    return ok;
}

// ------------------------------------------------------------------ 3. write: the stream

typedef struct {
    uint8_t *d;
    int n, cap;
    int loopAt;               // stream offset of the loop point, -1 = none
    // the registers as the driver has left them
    uint8_t shadow[0x40];
    uint8_t known[0x40];
    // this frame's writes (they go in after the wait that ends the frame before)
    uint8_t fb[1024];
    int fn;
} Stream;

static void put(Stream *s, uint8_t b)
{
    if (s->n == s->cap) {
        int c = s->cap ? s->cap * 2 : 65536;
        uint8_t *d = realloc(s->d, (size_t)c);
        if (!d) return;
        s->d = d;
        s->cap = c;
    }
    s->d[s->n++] = b;
}

// a register write; `force` writes it even when it already holds the value (it does something: a restart)
static void wr(Stream *s, int reg, int val, int force)
{
    val &= 0xFF;
    if (!force && s->known[reg] && s->shadow[reg] == val) return;
    if (s->fn + 2 > (int)sizeof s->fb) return;
    s->fb[s->fn++] = (uint8_t)reg;
    s->fb[s->fn++] = (uint8_t)val;
    s->shadow[reg] = (uint8_t)val;
    s->known[reg] = 1;
}

// after each frame: the frame's end, frames without writes folded into its wait
static void end_frames(Stream *s, int idle)
{
    while (idle >= 0) {
        int w = idle > 0x7D ? 0x7D : idle;
        put(s, (uint8_t)(0x81 + w));
        idle -= w + 1;
    }
}

// --- the NES (2A03): $4000 + reg
static int nes_period(double hz, int div, int lo, int hi)
{
    int p = (int)lround(NES_CPU / (div * hz) - 1);
    return p < lo ? lo : p > hi ? hi : p;
}

static const int k_nesNoise[16] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};

static void nes_frame(Stream *s, const ChOut *o, const ChOut *prev, int full)
{
    if (full) {
        memset(s->known, 0, sizeof s->known);
        wr(s, 0x15, 0x0F, 1);
        wr(s, 0x01, 0x08, 1);       // sweeps off (negate set: no muting of low notes)
        wr(s, 0x05, 0x08, 1);
    }
    for (int q = 0; q < 2; q++) {
        int base = q * 4;
        if (!o[q].on) { wr(s, base, 0x30, 0); continue; }
        // constant volume, length counter held
        wr(s, base, (o[q].duty << 6) | 0x30 | o[q].vol, 0);
        int p = nes_period(o[q].hz, 16, 8, 2047);
        // the high bits restart the square's phase (a click), so only for a new note or when they change; a held
        // note that goes just past them (a vibrato) stays at the edge instead
        int restart = full || o[q].newNote || !prev[q].on;
        if (!restart && s->known[base + 3]) {
            int h = s->shadow[base + 3] & 7;
            if (p >> 8 > h && p - (h << 8 | 0xFF) <= 12) p = h << 8 | 0xFF;
            else if (p >> 8 < h && (h << 8) - p <= 12) p = h << 8;
        }
        wr(s, base + 2, p & 0xFF, 0);
        if (restart || !s->known[base + 3] || (s->shadow[base + 3] & 7) != (p >> 8)) wr(s, base + 3, p >> 8 | 0x08, restart);
    }
    if (!o[2].on) wr(s, 0x08, 0x80, 0);
    else {
        int p = nes_period(o[2].hz, 32, 2, 2047);
        int start = full || !prev[2].on;
        if (!start && s->known[0x0B]) {
            int h = s->shadow[0x0B] & 7;
            if (p >> 8 > h && p - (h << 8 | 0xFF) <= 12) p = h << 8 | 0xFF;
            else if (p >> 8 < h && (h << 8) - p <= 12) p = h << 8;
        }
        wr(s, 0x08, 0xFF, 0);
        wr(s, 0x0A, p & 0xFF, 0);
        if (start || !s->known[0x0B] || (s->shadow[0x0B] & 7) != (p >> 8)) wr(s, 0x0B, p >> 8 | 0x08, start);
    }
    if (!o[3].on) wr(s, 0x0C, 0x30, 0);
    else {
        wr(s, 0x0C, 0x30 | o[3].vol, 0);
        int best = 0;
        double bd = 1e9;
        for (int i = 0; i < 16; i++) {
            double d = fabs(log((NES_CPU / k_nesNoise[i]) / o[3].hz));
            if (d < bd) { bd = d; best = i; }
        }
        wr(s, 0x0E, best | (o[3].shortNoise ? 0x80 : 0), 0);
        if (full || o[3].newNote || !prev[3].on) wr(s, 0x0F, 0x08, 1);
    }
}

// --- the Game Boy: $FF10 + reg (wave RAM at 0x20-0x2F)
typedef struct {
    int vol, dir, period;     // the envelope as the hardware runs it
    double ticks;             // 1/64 s steps since the last one
    int cnt;
    int off;                  // frames in a row it has been 2 or more from the volume wanted
} GbEnv;

static void env_step(GbEnv *e)
{
    e->ticks += 64.0 / GB_FPS;
    while (e->ticks >= 1) {
        e->ticks -= 1;
        if (!e->period) continue;
        if (--e->cnt <= 0) {
            e->cnt = e->period;
            if (e->dir && e->vol < 15) e->vol++;
            else if (!e->dir && e->vol > 0) e->vol--;
        }
    }
}

// the envelope (start, direction, speed) that follows the volumes a note wants next most closely
static int env_fit(const uint8_t *want, int n, GbEnv *out)
{
    int best = 0;
    double bestErr = 1e30;
    for (int opt = 0; opt < 15; opt++) {
        GbEnv e = {want[0], opt >= 8, opt == 0 ? 0 : opt < 8 ? opt : opt - 7, 0, 0};
        e.cnt = e.period;
        double err = 0;
        for (int i = 0; i < n; i++) {
            double d = e.vol - want[i];
            err += fabs(d) * (1.0 - i * 0.5 / n);    // the near frames matter more
            env_step(&e);
        }
        if (opt == 0) err -= 0.01;                    // a steady volume when it's as good
        if (err < bestErr) { bestErr = err; best = opt; }
    }
    out->vol = want[0];
    out->dir = best >= 8;
    out->period = best == 0 ? 0 : best < 8 ? best : best - 7;
    out->cnt = out->period;
    out->ticks = 0;
    return (out->vol << 4) | (out->dir << 3) | out->period;
}

static int gb_noise43(double hz, int shortMode)
{
    int best = 0;
    double bd = 1e9;
    for (int sh = 0; sh < 14; sh++)
        for (int r = 0; r < 8; r++) {
            double f = 524288.0 / (r ? r : 0.5) / (double)(2 << sh);
            double d = fabs(log(f / hz));
            if (d < bd) { bd = d; best = sh << 4 | r; }
        }
    return best | (shortMode ? 8 : 0);
}

typedef struct {
    GbEnv env[4];
    int wave;                 // the timbre the wave RAM holds (-1 none)
} GbState;

static void gb_frame(Stream *s, GbState *g, const Arr *a, const Cap *c, int k, int full)
{
    const ChOut *o = a->ch[k], *prev = k > 0 && !full ? a->ch[k - 1] : NULL;
    static const ChOut k_off[4];
    if (!prev) prev = k_off;
    if (full) {
        memset(s->known, 0, sizeof s->known);
        wr(s, 0x16, 0x80, 1);        // NR52: sound on
        wr(s, 0x14, 0x77, 1);        // NR50
        wr(s, 0x00, 0x08, 1);        // NR10: no sweep
        g->wave = -1;
    }
    // squares 1 and 2 (NR11-14, NR21-24), the noise (NR41-44): volume by envelope, set when a note starts
    static const int k_nr2[4] = {0x02, 0x07, -1, 0x11};
    for (int q = 0; q < 4; q++) {
        if (q == 2) continue;
        int nr2 = k_nr2[q];
        if (!o[q].on) {
            if (prev[q].on || full) wr(s, nr2, 0x00, 0);    // the channel's DAC off: silent
            continue;
        }
        // the volumes this note wants from here (as long as the same note stays on the channel)
        uint8_t want[48];
        int n = 0;
        for (int j = k; j < a->nframes && n < 48; j++) {
            const ChOut *x = &a->ch[j][q];
            if (!x->on || x->note != o[q].note || (j > k && x->newNote)) break;
            want[n++] = x->vol;
        }
        if (!n) want[n++] = o[q].vol;
        int start = full || o[q].newNote || !prev[q].on;
        // a held note whose envelope has drifted from the volume it wants starts its envelope again (each start
        // restarts the square's waveform, so only for a clear difference: 5 steps, or 2 for 6 frames)
        int diff = abs(g->env[q].vol - o[q].vol);
        g->env[q].off = diff >= 2 ? g->env[q].off + 1 : 0;
        if (!start && (diff >= 5 || g->env[q].off >= 6)) start = 1;
        if (q < 2) wr(s, q == 0 ? 0x01 : 0x06, o[q].duty << 6, 0);
        int trig = 0;
        if (start) {
            wr(s, nr2, env_fit(want, n, &g->env[q]), 1);
            g->env[q].off = 0;
            trig = 0x80;
        }
        if (q < 2) {
            int x = (int)lround(2048 - 131072.0 / o[q].hz);
            if (x < 0) x = 0;
            if (x > 2047) x = 2047;
            int nr3 = q == 0 ? 0x03 : 0x08;
            wr(s, nr3, x & 0xFF, 0);
            if (trig || !s->known[nr3 + 1] || (s->shadow[nr3 + 1] & 7) != (x >> 8)) wr(s, nr3 + 1, trig | x >> 8, trig);
        } else {
            wr(s, 0x12, gb_noise43(o[3].hz, o[3].shortNoise), 0);
            if (trig) wr(s, 0x13, 0x80, 1);
        }
        env_step(&g->env[q]);
    }
    // the wave channel (NR30-34): its shape from the instrument, its level in four steps
    if (!o[2].on) {
        if (prev[2].on || full) wr(s, 0x0C, 0x00, 0);       // NR32: mute
    } else {
        int start = full || o[2].newNote || !prev[2].on;
        const Timbre *tm = o[2].wave >= 0 && o[2].wave < c->ntim ? &c->tim[o[2].wave] : NULL;
        int w = tm && tm->hasWave ? o[2].wave : -2;
        if (start && w != g->wave) {
            static const uint8_t k_tri[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                              15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
            const uint8_t *shape = w >= 0 ? tm->wave : k_tri;
            wr(s, 0x0A, 0x00, 1);                               // NR30: off while the wave RAM is written
            for (int i = 0; i < 16; i++) wr(s, 0x20 + i, (shape[i * 2] & 15) << 4 | (shape[i * 2 + 1] & 15), 1);
            g->wave = w;
        }
        wr(s, 0x0A, 0x80, 0);
        int lv = o[2].vol >= 11 ? 1 : o[2].vol >= 6 ? 2 : 3;   // 100 / 50 / 25%
        wr(s, 0x0C, lv << 5, 0);
        int x = (int)lround(2048 - 65536.0 / o[2].hz);
        if (x < 0) x = 0;
        if (x > 2047) x = 2047;
        wr(s, 0x0D, x & 0xFF, 0);
        if (start || !s->known[0x0E] || (s->shadow[0x0E] & 7) != (x >> 8)) wr(s, 0x0E, (start ? 0x80 : 0) | x >> 8, start);
    }
    // NR51: each channel to its side(s)
    int nr51 = 0;
    for (int q = 0; q < 4; q++) {
        int p = o[q].on ? o[q].pan : 3;
        if (p & 1) nr51 |= 0x10 << q;
        if (p & 2) nr51 |= 0x01 << q;
    }
    wr(s, 0x15, nr51, 0);
}

static int build_stream(const Arr *a, const Cap *c, int target, Stream *s)
{
    memset(s, 0, sizeof *s);
    s->loopAt = -1;
    GbState g;
    memset(&g, 0, sizeof g);
    g.wave = -1;
    // each frame with writes: its writes, then the wait byte that ends it, counting the frames without writes after
    // it (known only when the next frame with writes comes, or the loop point, or the end)
    int idle = -1;            // -1: no frame waiting for its wait byte
    for (int k = 0; k < a->nframes; k++) {
        int full = k == 0 || k == a->loopFrame;
        if (k == a->loopFrame) {
            if (idle >= 0) end_frames(s, idle);
            idle = -1;
            s->loopAt = s->n;
        }
        s->fn = 0;
        if (target == CHIP_NES) nes_frame(s, a->ch[k], k > 0 && !full ? a->ch[k - 1] : a->ch[k], full);
        else gb_frame(s, &g, a, c, k, full);
        if (s->fn) {
            if (idle >= 0) end_frames(s, idle);
            for (int i = 0; i < s->fn; i++) put(s, s->fb[i]);
            idle = 0;
        } else if (idle >= 0) idle++;
    }
    if (idle >= 0) end_frames(s, idle);
    put(s, a->loopFrame >= 0 ? 0x80 : 0xFF);
    return s->d != NULL;
}

// ------------------------------------------------------------------ the files

static void tag_copy(uint8_t *dst, const char *src)
{
    // 31 characters and a 0; only plain ASCII (players show these as they are)
    int n = 0;
    for (const unsigned char *p = (const unsigned char *)src; *p && n < 31; p++) dst[n++] = *p >= 32 && *p < 127 ? *p : '?';
    dst[n] = 0;
}

// what this studio leaves after the driver: MAGIC, total frames, loop frame (-1 none), frames a second x 1000
static void put_info(uint8_t *p, const Arr *a, double fps)
{
    memcpy(p, MAGIC, 8);
    int32_t v[3] = {a->nframes, a->loopFrame, (int32_t)lround(fps * 1000)};
    for (int i = 0; i < 3; i++) {
        uint32_t u = (uint32_t)v[i];
        p[8 + i * 4] = (uint8_t)u;
        p[9 + i * 4] = (uint8_t)(u >> 8);
        p[10 + i * 4] = (uint8_t)(u >> 16);
        p[11 + i * 4] = (uint8_t)(u >> 24);
    }
}

int chip_file_info(const char *path, double *length, double *loopStart)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    uint8_t buf[4096];
    size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    for (size_t i = 0; i + 20 <= n; i++) {
        if (memcmp(buf + i, MAGIC, 8)) continue;
        int32_t v[3];
        for (int j = 0; j < 3; j++)
            v[j] = (int32_t)(buf[i + 8 + j * 4] | buf[i + 9 + j * 4] << 8 | buf[i + 10 + j * 4] << 16 | (uint32_t)buf[i + 11 + j * 4] << 24);
        double fps = v[2] > 0 ? v[2] / 1000.0 : 60;
        *length = v[0] / fps;
        *loopStart = v[1] >= 0 ? v[1] / fps : -1;
        return 1;
    }
    return 0;
}

static int write_nsf(const char *path, const Stream *s, const Arr *a, const ChipOpts *o, char *msg, int msglen)
{
    int banks = 1 + (s->n + 4095) / 4096;
    if (banks > 256) { snprintf(msg, msglen, "the song is too long for an NSF (%d KB of chip writes; 1020 KB fit)", s->n / 1024); return 0; }
    size_t size = 0x80 + (size_t)banks * 4096;
    uint8_t *d = calloc(1, size);
    if (!d) { snprintf(msg, msglen, "out of memory"); return 0; }
    memcpy(d, "NESM\x1A", 5);
    d[5] = 1;                         // version
    d[6] = 1;                         // songs
    d[7] = 1;                         // the first
    d[8] = NES_DRV_ORG & 0xFF; d[9] = NES_DRV_ORG >> 8;
    d[10] = NES_DRV_INIT & 0xFF; d[11] = NES_DRV_INIT >> 8;
    d[12] = NES_DRV_PLAY & 0xFF; d[13] = NES_DRV_PLAY >> 8;
    tag_copy(d + 0x0E, o->title);
    tag_copy(d + 0x2E, o->artist);
    tag_copy(d + 0x4E, o->copyright);
    d[0x6E] = NSF_SPEED_US & 0xFF; d[0x6F] = NSF_SPEED_US >> 8;
    d[0x70] = 0; d[0x71] = 1;         // bank 0 at $8000 (the driver), the stream through $9000
    d[0x78] = 0x1D; d[0x79] = 0x4E;   // PAL speed (20000 us; NTSC is this file's)
    d[0x7A] = 0;                      // NTSC
    uint8_t *drv = d + 0x80;
    memcpy(drv, k_nesDriver, sizeof k_nesDriver);
    int la = s->loopAt >= 0 ? s->loopAt : 0;
    drv[NES_LOOP_BANK] = (uint8_t)(1 + la / 4096);
    drv[NES_LOOP_LO] = (uint8_t)((0x9000 + la % 4096) & 0xFF);
    drv[NES_LOOP_HI] = (uint8_t)((0x9000 + la % 4096) >> 8);
    put_info(drv + 0x100, a, SRC_FPS);
    memcpy(d + 0x80 + 4096, s->d, (size_t)s->n);
    FILE *f = fopen(path, "wb");
    int ok = f && fwrite(d, 1, size, f) == size;
    if (f) ok = fclose(f) == 0 && ok;
    free(d);
    if (!ok) snprintf(msg, msglen, "couldn't write %s", path);
    return ok;
}

static int write_gbs(const char *path, const Stream *s, const Arr *a, const ChipOpts *o, char *msg, int msglen)
{
    int banks = 1 + (s->n + 16383) / 16384;        // bank 0: the driver; the stream from bank 1
    if (banks > 32) { snprintf(msg, msglen, "the song is too long for a GBS (%d KB of chip writes; 496 KB fit)", s->n / 1024); return 0; }
    size_t rom = (size_t)banks * 16384;            // the ROM from address 0; the file holds it from $0400
    size_t size = 0x70 + rom - GB_DRV_ORG;
    uint8_t *d = calloc(1, size);
    if (!d) { snprintf(msg, msglen, "out of memory"); return 0; }
    memcpy(d, "GBS", 3);
    d[3] = 1;                          // version
    d[4] = 1;                          // songs
    d[5] = 1;                          // the first
    d[6] = GB_DRV_ORG & 0xFF; d[7] = GB_DRV_ORG >> 8;
    d[8] = GB_DRV_INIT & 0xFF; d[9] = GB_DRV_INIT >> 8;
    d[10] = GB_DRV_PLAY & 0xFF; d[11] = GB_DRV_PLAY >> 8;
    d[12] = 0xFE; d[13] = 0xFF;        // stack at $FFFE
    d[14] = 0; d[15] = 0;              // timer: none (the vertical blank, 59.73 Hz)
    tag_copy(d + 0x10, o->title);
    tag_copy(d + 0x30, o->artist);
    tag_copy(d + 0x50, o->copyright);
    uint8_t *drv = d + 0x70;           // ROM $0400
    memcpy(drv, k_gbDriver, sizeof k_gbDriver);
    int la = s->loopAt >= 0 ? s->loopAt : 0;
    drv[GB_LOOP_BANK] = (uint8_t)(1 + la / 16384);
    drv[GB_LOOP_LO] = (uint8_t)((0x4000 + la % 16384) & 0xFF);
    drv[GB_LOOP_HI] = (uint8_t)((0x4000 + la % 16384) >> 8);
    put_info(drv + 0x100, a, GB_FPS);
    memcpy(d + 0x70 + (0x4000 - GB_DRV_ORG), s->d, (size_t)s->n);
    FILE *f = fopen(path, "wb");
    int ok = f && fwrite(d, 1, size, f) == size;
    if (f) ok = fclose(f) == 0 && ok;
    free(d);
    if (!ok) snprintf(msg, msglen, "couldn't write %s", path);
    return ok;
}

int chip_export(Studio *s, const ChipOpts *o, const char *path, char *msg, int msglen)
{
    msg[0] = 0;
    ChipOpts op = *o;
    if (!op.title[0]) snprintf(op.title, sizeof op.title, "%s", s->p.title[0] ? s->p.title : path_base(s->p.midiName));
    if (!op.artist[0] && s->p.isRip && s->p.rip && s->p.rip->artist[0]) snprintf(op.artist, sizeof op.artist, "%s", s->p.rip->artist);
    if (!op.copyright[0]) snprintf(op.copyright, sizeof op.copyright, "Moemon Music Studio %s", chip_name(op.target));
    Cap cap;
    // a recording (GameCube music is streamed) has no notes to make chiptune from
    if (s->p.isRip && s->p.ripKind == RIP_GCN) { snprintf(msg, msglen, "GameCube music is a recording, not notes: there's nothing to make chiptune from."); return 0; }
    if (!capture(s, &cap, msg, msglen)) { cap_free(&cap); return 0; }
    Arr arr;
    if (!arrange(&cap, op.target, op.arpFrames, &arr, msg, msglen)) { cap_free(&cap); free(arr.ch); return 0; }
    if (getenv("MMS_CHIP_DEBUG")) {   // debugging: the capture and the arrangement, frames A-B
        int f0 = 0, f1 = 60;
        sscanf(getenv("MMS_CHIP_DEBUG"), "%d-%d", &f0, &f1);
        for (int f = f0; f <= f1 && f < cap.nframes; f++) {
            fprintf(stderr, "in %5d:", f);
            for (int i = cap.first[f]; i < cap.first[f + 1]; i++) {
                const ChipEv *x = &cap.ev[i];
                fprintf(stderr, " t%d#%u%s%.1f/%.3f", x->track, x->note, x->drum ? "D" : "@", x->drum ? x->noiseHz : x->semis, x->amp);
            }
            fprintf(stderr, "\n");
        }
        for (int k = f0; k <= f1 && k < arr.nframes; k++) {
            fprintf(stderr, "out %5d:", k);
            for (int q = 0; q < 4; q++) {
                const ChOut *x = &arr.ch[k][q];
                if (!x->on) { fprintf(stderr, "  ---------------------"); continue; }
                fprintf(stderr, "  %c#%-5u %7.1fHz v%-2d d%d", x->newNote ? '*' : ' ', x->note, x->hz, x->vol, x->duty);
            }
            fprintf(stderr, "\n");
        }
    }
    Stream st;
    if (!build_stream(&arr, &cap, op.target, &st)) { cap_free(&cap); free(arr.ch); snprintf(msg, msglen, "out of memory"); return 0; }
    int ok = op.target == CHIP_GB ? write_gbs(path, &st, &arr, &op, msg, msglen) : write_nsf(path, &st, &arr, &op, msg, msglen);
    if (ok) {
        int secs = (int)lround(arr.nframes / (op.target == CHIP_GB ? GB_FPS : SRC_FPS));
        snprintf(msg, msglen, "%s: %d parts%s on the %s's %s, %d:%02d%s. %d%% of the parts' notes heard%s%s%s.",
                 path_base(path), arr.parts, arr.drumParts ? " and drums" : "", chip_name(op.target),
                 op.target == CHIP_GB ? "2 squares, wave and noise" : "2 squares, triangle and noise",
                 secs / 60, secs % 60, arr.loopFrame >= 0 ? " with its loop" : "",
                 arr.notesTotal ? (int)(100L * arr.notesHeard / arr.notesTotal) : 100,
                 arr.echoes ? "; echoes kept for the gaps" : "", arr.doublings ? "; doublings played once" : "",
                 arr.moved ? "; parts too low or high moved by octaves" : "");
    }
    free(st.d);
    free(arr.ch);
    cap_free(&cap);
    return ok;
}
