// Moemon Music Studio: a Nintendo DS sound archive (SDAT) played directly. No emulator and no 2SF needed: the SDAT's
// own sequence (SSEQ), instrument bank (SBNK) and wave archives (SWAR) run on a sequencer written after the DS's
// sound library (its 192 Hz update, tempo / 240 ticks, ADSR in decibels, channels 8-13 square and 14-15 noise). Its
// channels are reported like a 2SF rip's (rip_2sf.cpp), so rip_song.c makes the same instruments and MIDI from them.
//
// The path names the sequence after a '#': "sound_data.sdat#SEQ_PL_TOWN02" (its name in the archive's SYMB block)
// or "sound_data.sdat#1218" (its number); a plain "sound_data.sdat" plays the track number the window or the command
// line passes (the GBS track's place), else the archive's first sequence.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "rip.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DS_CLOCK 33513982.0
#define SEQ_HZ (DS_CLOCK / (64.0 * 2728.0))   // the sequencer's updates a second (191.95)
#define OUT_RATE 32728
#define NCHAN 16
#define NTRACK 16
#define AMPL_MIN (-92544)                      // -723 << 7: silence
#define TIMER_CLOCK 16756991.0

typedef struct {
    int type;              // 1 PCM, 2 PSG square, 3 noise
    int wav, arc;          // PCM: the wave and its archive slot; PSG: duty
    int base;              // base key
    int a, d, s, r, pan;
} SdInst;

typedef struct {           // a decoded SWAV
    int16_t *pcm;
    int len, loopStart, loops, rate;
    int ripIndex;          // its RipSample
} SdWave;

typedef struct {
    const uint8_t *d;
    int size;
    SdWave *w;
    int n;
} SdArc;

typedef struct SdTrack {
    int on, pos, wait;
    int stack[3], loopCount[3], loopPos[3], sp;
    int prog, vol, expr, pan, transpose, bend, bendRange, prio;
    int noteWait, tie, porta, portaKey, portaTime, sweep;
    int a, d, s, r;       // 255: the instrument's
    int modDepth, modSpeed, modType, modRange, modDelay;
    int cmp;
    int lastChan;          // tie mode: the channel its note plays on
    int loopJumps;         // backward jumps taken (the loop)
} SdTrack;

typedef struct {
    int active, track, prio;
    int kind;              // 1 PCM, 2 PSG, 3 noise
    SdWave *wave;
    int duty;
    int key, base, vel, len;   // len: ticks left (-1: until released)
    int state;             // 0 attack, 1 decay, 2 sustain, 3 release
    int ampl, attack, decay, sustain, release;
    int instPan;
    double pos;            // sample position
    uint16_t lfsr;
    int keyon;
    int modCounter, modDelay;
    double sweepFrom; int sweepLen, sweepAt;
    // what the last update set
    double rate;           // Hz the sample / square steps play at
    float l, r;
    int ripSample;
} SdChan;

typedef struct {
    Rip *rip;
    uint8_t *file; long fileLen;
    const uint8_t *seq; int seqLen;    // sequence data
    const uint8_t *bank; int bankLen;
    SdArc arc[4];
    int seqVol;
    SdTrack t[NTRACK];
    SdChan c[NCHAN];
    int tempo, tempoCounter, masterVol;
    int16_t var[32];
    uint32_t rnd;
    int ended;
    double frameAcc;       // output samples until the next sequencer update
    int sqSample[8], noiseSample;
    int dry;               // timing only (the length pass)
    long ticks;
} Sd;

static uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static int rd16(const uint8_t *p) { return p[0] | p[1] << 8; }

// ---- decibels (the DS's tables, in tenths of a decibel) ----
static int db_square(int v)   // a 0-127 volume as the DS hears it (squared): 127 = 0, 0 = -72.3 dB
{
    if (v <= 0) return -723;
    if (v > 127) v = 127;
    int d = (int)lrint(400.0 * log10(v / 127.0));
    return d < -723 ? -723 : d;
}
static int calc_attack(int a)
{
    static const uint8_t t[19] = {0x00, 0x01, 0x05, 0x0E, 0x1A, 0x26, 0x33, 0x3F, 0x49, 0x54,
                                  0x5C, 0x64, 0x6D, 0x74, 0x7B, 0x7F, 0x84, 0x89, 0x8F};
    if (a < 0) a = 0;
    if (a > 127) a = 127;
    return a < 109 ? 255 - a : t[127 - a];
}
static int calc_fall(int v)
{
    if (v >= 127) return 0xFFFF;
    if (v == 126) return 0x3C00;
    if (v < 50) return v * 2 + 1;
    return 0x1E00 / (126 - v);
}

// ---- the archive ----
static const uint8_t *sdat_file(Sd *s, int id, int *size)
{
    uint32_t fat = rd32(s->file + 0x20);
    if (fat + 12 > (uint32_t)s->fileLen) return NULL;
    uint32_t n = rd32(s->file + fat + 8);
    if (id < 0 || (uint32_t)id >= n) return NULL;
    const uint8_t *e = s->file + fat + 12 + 16 * id;
    uint32_t off = rd32(e), sz = rd32(e + 4);
    if (off + sz > (uint32_t)s->fileLen) return NULL;
    // (an archive written by some tools points a few bytes short of each file: its own tag says where it starts)
    for (uint32_t k = 0; k <= 0x40 && off + k + 4 <= (uint32_t)s->fileLen; k += 4) {
        const uint8_t *p = s->file + off + k;
        if (p[0] == 'S' && (!memcmp(p, "SSEQ", 4) || !memcmp(p, "SBNK", 4) || !memcmp(p, "SWAR", 4) || !memcmp(p, "SSAR", 4)
                            || !memcmp(p, "STRM", 4))) {
            off += k;
            break;
        }
    }
    if (off + sz > (uint32_t)s->fileLen) sz = (uint32_t)s->fileLen - off;
    *size = (int)sz;
    return s->file + off;
}
// INFO record list `rec` entry `i` (NULL when empty)
static const uint8_t *info_entry(Sd *s, int rec, int i)
{
    uint32_t info = rd32(s->file + 0x18);
    uint32_t list = info + rd32(s->file + info + 8 + rec * 4);
    uint32_t n = rd32(s->file + list);
    if (i < 0 || (uint32_t)i >= n) return NULL;
    uint32_t o = rd32(s->file + list + 4 + 4 * i);
    return o ? s->file + info + o : NULL;
}
static int info_count(Sd *s, int rec)
{
    uint32_t info = rd32(s->file + 0x18);
    return (int)rd32(s->file + info + rd32(s->file + info + 8 + rec * 4));
}
static const char *symb_name(Sd *s, int rec, int i)
{
    uint32_t symb = rd32(s->file + 0x10);
    if (!symb || !rd32(s->file + 0x14)) return NULL;
    uint32_t list = symb + rd32(s->file + symb + 8 + rec * 4);
    uint32_t n = rd32(s->file + list);
    if (i < 0 || (uint32_t)i >= n) return NULL;
    uint32_t o = rd32(s->file + list + 4 + 4 * i);
    return o ? (const char *)s->file + symb + o : NULL;
}

// ---- waves ----
static int decode_swav(const uint8_t *p, int avail, SdWave *w)
{
    if (avail < 12) return 0;
    // (u8 format, u8 loops, u16 rate, u16 timer, u16 loop start in words, u32 words after it; the samples at 12)
    int fmt = p[0], loop = p[1];
    int timer = rd16(p + 4), loopOff = rd16(p + 6);
    uint32_t nonLoop = rd32(p + 8);
    int words = loopOff + (int)nonLoop, bytes = words * 4;
    const uint8_t *d = p + 12;
    if (bytes > avail - 12) bytes = avail - 12;
    if (bytes <= 0) return 0;
    w->rate = timer ? (int)lrint(TIMER_CLOCK / timer) : rd16(p + 2);
    w->loops = loop != 0;
    if (fmt == 0) {          // PCM8
        w->len = bytes;
        w->loopStart = loopOff * 4;
        w->pcm = malloc(sizeof(int16_t) * w->len);
        for (int i = 0; i < w->len; i++) w->pcm[i] = (int16_t)((int8_t)d[i] * 256);
    } else if (fmt == 1) {   // PCM16
        w->len = bytes / 2;
        w->loopStart = loopOff * 2;
        w->pcm = malloc(sizeof(int16_t) * w->len);
        for (int i = 0; i < w->len; i++) w->pcm[i] = (int16_t)rd16(d + i * 2);
    } else {                 // IMA-ADPCM: the first word is the starting state
        static const int16_t step[89] = {7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
            50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
            449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499,
            2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635,
            13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};
        static const int8_t idxt[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
        if (bytes < 4) return 0;
        int pred = (int16_t)rd16(d), idx = d[2] > 88 ? 88 : d[2];
        w->len = (bytes - 4) * 2;
        w->loopStart = loopOff > 0 ? (loopOff * 4 - 4) * 2 : 0;
        w->pcm = malloc(sizeof(int16_t) * (w->len > 0 ? w->len : 1));
        for (int i = 0; i < w->len; i++) {
            int nib = (d[4 + i / 2] >> ((i & 1) * 4)) & 15, st = step[idx];
            int diff = st >> 3;
            if (nib & 1) diff += st >> 2;
            if (nib & 2) diff += st >> 1;
            if (nib & 4) diff += st;
            pred += (nib & 8) ? -diff : diff;
            if (pred > 0x7FFF) pred = 0x7FFF;
            if (pred < -0x7FFF) pred = -0x7FFF;
            idx += idxt[nib & 7];
            if (idx < 0) idx = 0;
            if (idx > 88) idx = 88;
            w->pcm[i] = (int16_t)pred;
        }
    }
    if (w->loopStart >= w->len) { w->loopStart = 0; w->loops = 0; }
    w->ripIndex = -1;
    return w->len > 0;
}

static SdWave *get_wave(Sd *s, int arc, int wav)
{
    if (arc < 0 || arc > 3 || !s->arc[arc].d) return NULL;
    SdArc *a = &s->arc[arc];
    if (wav < 0 || wav >= a->n) return NULL;
    SdWave *w = &a->w[wav];
    if (!w->pcm) {
        uint32_t off = rd32(a->d + 0x3C + 4 * wav);
        if (off >= (uint32_t)a->size) return NULL;
        if (!decode_swav(a->d + off, a->size - (int)off, w)) return NULL;
    }
    if (w->ripIndex < 0 && !s->dry) {
        RipSample rs;
        memset(&rs, 0, sizeof rs);
        rs.kind = RIPS_PCM;
        rs.pcm = w->pcm;
        rs.len = w->len;
        rs.loopStart = w->loopStart;
        rs.loops = w->loops;
        rs.rate = w->rate;
        rs.id = 0x10000u * (uint32_t)(arc + 1) + (uint32_t)wav;
        w->ripIndex = rip_add_sample(s->rip, &rs);
    }
    return w;
}

// The instrument for a program and key (NULL: none)
static int get_inst(Sd *s, int prog, int key, SdInst *out)
{
    if (!s->bank || s->bankLen < 0x3C) return 0;
    int n = (int)rd32(s->bank + 0x38);
    if (prog < 0 || prog >= n) return 0;
    const uint8_t *rec = s->bank + 0x3C + 4 * prog;
    int type = rec[0], off = rd16(rec + 1);
    if (!type || off <= 0 || off >= s->bankLen) return 0;
    const uint8_t *p = s->bank + off;
    if (type == 16) {        // a drum set: one instrument a key
        int lo = p[0], hi = p[1];
        if (key < lo || key > hi) return 0;
        p += 2 + 12 * (key - lo);
        type = p[0];
        p += 2;
    } else if (type == 17) { // key split: up to 8 regions
        int r = 0;
        while (r < 8 && p[r] && key > p[r]) r++;
        if (r >= 8 || !p[r]) return 0;
        p += 8 + 12 * r;
        type = p[0];
        p += 2;
    }
    if (type < 1 || type > 3) return 0;
    out->type = type;
    out->wav = rd16(p);
    out->arc = rd16(p + 2);
    out->base = p[4];
    out->a = p[5]; out->d = p[6]; out->s = p[7]; out->r = p[8];
    out->pan = p[9];
    return 1;
}

// The PSG squares (8-step samples, as rip_2sf.cpp makes them) and the noise, added the first time one plays
static int square_sample(Sd *s, int duty)
{
    static int16_t sq[8][8];
    if (s->sqSample[duty] >= 0) return s->sqSample[duty];
    for (int i = 0; i < 8; i++) sq[duty][i] = (int16_t)(duty == 7 ? -0x7FFF : i >= 7 - duty ? 0x7FFF : -0x7FFF);
    RipSample rs;
    memset(&rs, 0, sizeof rs);
    rs.kind = RIPS_PCM; rs.pcm = sq[duty]; rs.len = 8; rs.loops = 1; rs.rate = (int)(TIMER_CLOCK / 1024);
    rs.id = 0x400 + (uint32_t)duty; rs.param = 0x80 | duty;
    return s->sqSample[duty] = rip_add_sample(s->rip, &rs);
}
static int noise_sample(Sd *s)
{
    if (s->noiseSample >= 0) return s->noiseSample;
    RipSample rs;
    memset(&rs, 0, sizeof rs);
    rs.kind = RIPS_NOISE; rs.param = 0; rs.id = 0x300;
    return s->noiseSample = rip_add_sample(s->rip, &rs);
}

// ---- channels ----
static int alloc_chan(Sd *s, int kind, int prio)
{
    static const int pcmOrder[16] = {4, 5, 6, 7, 2, 0, 3, 1, 8, 9, 10, 11, 14, 12, 15, 13};
    static const int psgOrder[6] = {8, 9, 10, 11, 12, 13};
    static const int noiseOrder[2] = {14, 15};
    const int *order = kind == 1 ? pcmOrder : kind == 2 ? psgOrder : noiseOrder;
    int n = kind == 1 ? 16 : kind == 2 ? 6 : 2;
    int best = -1, bestScore = 0x7FFFFFFF;
    for (int i = 0; i < n; i++) {
        SdChan *c = &s->c[order[i]];
        int score = !c->active ? -0x40000000 : c->prio * 0x20000 + (c->state == 3 ? 0 : 0x10000) + (c->ampl >> 3);
        if (c->active && c->prio > prio) continue;
        if (score < bestScore) { bestScore = score; best = order[i]; }
    }
    return best;
}

static void note_on(Sd *s, SdTrack *t, int ti, int key, int vel, int len)
{
    SdInst in;
    if (s->dry) return;
    int k = key + t->transpose;
    if (k < 0) k = 0;
    if (k > 127) k = 127;
    if (!get_inst(s, t->prog, k, &in)) return;
    SdChan *c;
    if (t->tie && t->lastChan >= 0 && s->c[t->lastChan].active && s->c[t->lastChan].track == ti) {
        c = &s->c[t->lastChan];   // tie: the same channel goes on to the new key
        c->sweepFrom = c->key + 0.0;
        c->key = k;
        c->vel = vel;
        c->len = len > 0 ? len : -1;
        return;
    }
    int ch = alloc_chan(s, in.type, t->prio);
    if (ch < 0) return;
    c = &s->c[ch];
    memset(c, 0, sizeof *c);
    c->active = 1;
    c->track = ti;
    c->prio = t->prio;
    c->kind = in.type;
    if (in.type == 1) {
        c->wave = get_wave(s, in.arc, in.wav);
        if (!c->wave) { c->active = 0; return; }
        c->ripSample = c->wave->ripIndex;
    } else if (in.type == 2) {
        c->duty = in.wav & 7;
        c->ripSample = square_sample(s, c->duty);
    } else {
        c->lfsr = 0x7FFF;
        c->ripSample = noise_sample(s);
    }
    c->key = k;
    c->base = in.base;
    c->vel = vel;
    c->len = len > 0 ? len : -1;
    c->instPan = in.pan - 64;
    c->attack = calc_attack(t->a != 255 ? t->a : in.a);
    c->decay = calc_fall(t->d != 255 ? t->d : in.d);
    c->sustain = db_square(t->s != 255 ? t->s : in.s) << 7;
    c->release = calc_fall(t->r != 255 ? t->r : in.r);
    c->ampl = AMPL_MIN;
    c->state = 0;
    c->keyon = 1;
    c->modDelay = t->modDelay;
    c->sweepLen = 0;
    if (t->porta && t->portaKey >= 0 && t->portaKey != k) {
        c->sweepFrom = t->portaKey;
        int diff = abs(k - t->portaKey);
        c->sweepLen = t->portaTime ? (t->portaTime * t->portaTime * diff) >> 11 : (len > 0 ? len : 1);
        if (c->sweepLen < 1) c->sweepLen = 1;
    }
    if (t->sweep) { c->sweepFrom = k + t->sweep / 64.0; c->sweepLen = len > 0 ? len : 1; }
    t->portaKey = k;
    t->lastChan = ch;
}

static void release_track(Sd *s, int ti)
{
    for (int i = 0; i < NCHAN; i++)
        if (s->c[i].active && s->c[i].track == ti) s->c[i].state = 3;
}

// one update of every channel (the envelope, its pitch and volume from its track now)
static void update_chans(Sd *s)
{
    for (int i = 0; i < NCHAN; i++) {
        SdChan *c = &s->c[i];
        if (!c->active) continue;
        SdTrack *t = &s->t[c->track];
        switch (c->state) {
        case 0:
            c->ampl = (c->attack * c->ampl) / 256;
            if (c->ampl == 0 || c->attack == 0) { c->ampl = 0; c->state = 1; }
            break;
        case 1:
            c->ampl -= c->decay;
            if (c->ampl <= c->sustain) { c->ampl = c->sustain; c->state = 2; }
            break;
        case 2:
            break;
        case 3:
            c->ampl -= c->release;
            if (c->ampl <= AMPL_MIN) { c->active = 0; continue; }
            break;
        }
        // modulation
        double lfo = 0;
        if (t->modDepth && t->modRange) {
            if (c->modDelay > 0) c->modDelay--;
            else {
                c->modCounter = (c->modCounter + (t->modSpeed << 6)) & 0x7FFF;
                lfo = sin(c->modCounter / 32768.0 * 2 * M_PI) * (t->modDepth / 127.0) * t->modRange;
            }
        }
        double key = c->key;
        if (c->sweepLen > 0 && c->sweepAt < c->sweepLen) {
            key = c->sweepFrom + (c->key - c->sweepFrom) * c->sweepAt / (double)c->sweepLen;
        }
        double semis = key - c->base + t->bend * t->bendRange / 128.0 + (t->modType == 0 ? lfo : 0);
        double mul = pow(2.0, semis / 12.0);
        if (c->kind == 1) c->rate = c->wave->rate * mul;
        else c->rate = TIMER_CLOCK / 8006.0 * mul;   // the squares' and the noise's steps: 8006 = C4 at the base key
        int db = db_square(c->vel) + db_square(t->vol) + db_square(t->expr) + db_square(s->masterVol) +
                 db_square(s->seqVol) + (c->ampl >> 7);
        if (t->modType == 1) db += (int)lrint(lfo * 60);
        if (db < -723) db = -723;
        if (db > 0) db = 0;
        float v = db <= -723 ? 0.0f : (float)pow(10.0, db / 200.0);
        int pan = t->pan + c->instPan + (t->modType == 2 ? (int)lrint(lfo * 64) : 0);
        if (pan < 0) pan = 0;
        if (pan > 127) pan = 127;
        c->l = v * (127 - pan) / 127.0f;
        c->r = v * pan / 127.0f;
    }
}

// ---- the sequencer ----
static int rd_var(Sd *s, SdTrack *t)   // variable-length number
{
    int v = 0;
    for (int i = 0; i < 4 && t->pos < s->seqLen; i++) {
        int b = s->seq[t->pos++];
        v = (v << 7) | (b & 0x7F);
        if (!(b & 0x80)) break;
    }
    return v;
}
static int rd_u8(Sd *s, SdTrack *t) { return t->pos < s->seqLen ? s->seq[t->pos++] : 0; }
static int rd_u16(Sd *s, SdTrack *t) { int a = rd_u8(s, t); return a | rd_u8(s, t) << 8; }
static int rd_u24(Sd *s, SdTrack *t) { int a = rd_u16(s, t); return a | rd_u8(s, t) << 16; }
static uint32_t rnd(Sd *s) { s->rnd = s->rnd * 1664525u + 1013904223u; return s->rnd >> 16; }

// A command's last argument: as written (`kind`: 1 a byte, 2 a 16-bit number, 3 signed 16-bit, 4 variable-length),
// or after the random prefix (A0: a range, two signed 16-bit numbers) or the variable prefix (A1: a variable's number)
static int last_arg(Sd *s, SdTrack *t, int useRand, int useVar, int kind)
{
    if (useRand) {
        int lo = (int16_t)rd_u16(s, t);
        int hi = (int16_t)rd_u16(s, t);
        return hi > lo ? lo + (int)(rnd(s) % (unsigned)(hi - lo + 1)) : lo;
    }
    if (useVar) return s->var[rd_u8(s, t) & 31];
    switch (kind) {
    case 1: return rd_u8(s, t);
    case 2: return rd_u16(s, t);
    case 3: return (int16_t)rd_u16(s, t);
    default: return rd_var(s, t);
    }
}

static void init_track(SdTrack *t, int pos)
{
    memset(t, 0, sizeof *t);
    t->on = 1;
    t->pos = pos;
    t->vol = 127; t->expr = 127; t->pan = 64; t->bendRange = 2; t->prio = 64;
    t->noteWait = 1; t->portaKey = -1;
    t->a = t->d = t->s = t->r = 255;
    t->modSpeed = 16; t->modRange = 1;
    t->lastChan = -1;
}

// One tick of a track: its commands until it waits.
static void track_tick(Sd *s, int ti)
{
    SdTrack *t = &s->t[ti];
    if (!t->on) return;
    for (int i = 0; i < NCHAN; i++) {   // its notes' lengths
        SdChan *c = &s->c[i];
        if (c->active && c->track == ti) {
            if (c->len > 0 && --c->len == 0) c->state = 3;
            if (c->sweepLen > 0 && c->sweepAt < c->sweepLen) c->sweepAt++;
        }
    }
    if (t->wait > 0 && --t->wait > 0) return;
    int guard = 0;
    while (t->on && t->wait == 0 && guard++ < 10000) {
        if (t->pos >= s->seqLen) { t->on = 0; break; }
        int cmd = rd_u8(s, t);
        int cond = 1, useVar = 0, useRand = 0;
        if (cmd == 0xA2) { cond = t->cmp; cmd = rd_u8(s, t); }
        if (cmd == 0xA0) { useRand = 1; cmd = rd_u8(s, t); }
        else if (cmd == 0xA1) { useVar = 1; cmd = rd_u8(s, t); }
        if (cmd < 0x80) {
            int vel = rd_u8(s, t);
            int len = last_arg(s, t, useRand, useVar, 4);
            if (!cond) continue;
            note_on(s, t, ti, cmd, vel, len);
            if (t->noteWait) t->wait = len;
            continue;
        }
        switch (cmd) {
        case 0x80: { int v = last_arg(s, t, useRand, useVar, 4); if (cond) t->wait = v; break; }
        case 0x81: { int v = last_arg(s, t, useRand, useVar, 4); if (cond) t->prog = v; break; }
        case 0x93: { int n = rd_u8(s, t), off = rd_u24(s, t); if (cond && n > 0 && n < NTRACK) init_track(&s->t[n], off); break; }
        case 0x94: { int off = rd_u24(s, t); if (cond) { if (off <= t->pos) t->loopJumps++; if (getenv("MMS_SDAT_DEBUG")) fprintf(stderr, "sdat: track %d jumps %d -> %d (tick %ld)\n", ti, t->pos, off, s->ticks); t->pos = off; } break; }
        case 0x95: { int off = rd_u24(s, t); if (cond && t->sp < 3) { t->stack[t->sp++] = t->pos; t->pos = off; } break; }
        case 0xFD: if (cond && t->sp > 0) t->pos = t->stack[--t->sp]; break;
        case 0xD4: { int n = rd_u8(s, t); if (cond && t->sp < 3) { t->loopCount[t->sp] = n; t->stack[t->sp++] = t->pos; } break; }
        case 0xFC:
            if (cond && t->sp > 0) {
                int i = t->sp - 1;
                if (t->loopCount[i] == 0 || --t->loopCount[i] > 0) { t->pos = t->stack[i]; if (t->loopCount[i] == 0) t->loopJumps++; }
                else t->sp--;
            }
            break;
        case 0xFE: rd_u16(s, t); break;
        case 0xFF: if (cond) { t->on = 0; } break;
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: {
            int vi = rd_u8(s, t) & 31;
            int v = last_arg(s, t, useRand, useVar, 3);
            if (!cond) break;
            int16_t *x = &s->var[vi];
            switch (cmd) {
            case 0xB0: *x = (int16_t)v; break;
            case 0xB1: *x = (int16_t)(*x + v); break;
            case 0xB2: *x = (int16_t)(*x - v); break;
            case 0xB3: *x = (int16_t)(*x * v); break;
            case 0xB4: if (v) *x = (int16_t)(*x / v); break;
            case 0xB5: *x = (int16_t)(v >= 0 ? *x << v : *x >> -v); break;
            case 0xB6: *x = (int16_t)(v ? (int)(rnd(s) % (unsigned)(abs(v) + 1)) * (v < 0 ? -1 : 1) : 0); break;
            case 0xB8: t->cmp = *x == v; break;
            case 0xB9: t->cmp = *x >= v; break;
            case 0xBA: t->cmp = *x > v; break;
            case 0xBB: t->cmp = *x <= v; break;
            case 0xBC: t->cmp = *x < v; break;
            case 0xBD: t->cmp = *x != v; break;
            }
            break;
        }
        case 0xE0: { int v = last_arg(s, t, useRand, useVar, 3); if (cond) t->modDelay = v; break; }
        case 0xE1: { int v = last_arg(s, t, useRand, useVar, 2); if (cond) s->tempo = v; break; }
        case 0xE3: { int v = last_arg(s, t, useRand, useVar, 3); if (cond) t->sweep = v; break; }
        default:
            if (cmd >= 0xC0 && cmd <= 0xDF) {
                int v = last_arg(s, t, useRand, useVar, 1);
                if (!cond) break;
                switch (cmd) {
                case 0xC0: t->pan = v; break;
                case 0xC1: t->vol = v; break;
                case 0xC2: s->masterVol = v; break;
                case 0xC3: t->transpose = (int8_t)v; break;
                case 0xC4: t->bend = (int8_t)v; break;
                case 0xC5: t->bendRange = v; break;
                case 0xC6: t->prio = v; break;
                case 0xC7: t->noteWait = v != 0; break;
                case 0xC8: t->tie = v != 0; if (!t->tie) release_track(s, ti), t->lastChan = -1; break;
                case 0xC9: t->portaKey = v + t->transpose; t->porta = 1; break;
                case 0xCA: t->modDepth = v; break;
                case 0xCB: t->modSpeed = v; break;
                case 0xCC: t->modType = v; break;
                case 0xCD: t->modRange = v; break;
                case 0xCE: t->porta = v != 0; break;
                case 0xCF: t->portaTime = v; break;
                case 0xD0: t->a = v; break;
                case 0xD1: t->d = v; break;
                case 0xD2: t->s = v; break;
                case 0xD3: t->r = v; break;
                case 0xD5: t->expr = v; break;
                default: break;   // D6 print and the rest
                }
            } else {
                t->on = 0;   // a command this player doesn't know: the track stops rather than reading garbage
            }
            break;
        }
    }
}

static void seq_update(Sd *s)
{
    s->tempoCounter += s->tempo;
    while (s->tempoCounter >= 240) {
        s->tempoCounter -= 240;
        for (int i = 0; i < NTRACK; i++) track_tick(s, i);
        s->ticks++;
    }
    int any = 0;
    for (int i = 0; i < NTRACK; i++) any |= s->t[i].on;
    if (!any) s->ended = 1;
    if (!s->dry) update_chans(s);
}

static void seq_start(Sd *s)
{
    memset(s->t, 0, sizeof s->t);
    memset(s->c, 0, sizeof s->c);
    init_track(&s->t[0], 0);
    s->tempo = 120;
    s->tempoCounter = 240;   // the first tick at once
    s->masterVol = 127;
    memset(s->var, 0, sizeof s->var);
    s->rnd = 0x12345678;
    s->ended = 0;
    s->ticks = 0;
}

// ---- RipEmu ----
static int sd_render(RipEmu *e, int16_t *buf, int n)
{
    Sd *s = e->ctx;
    double perFrame = OUT_RATE / SEQ_HZ;
    for (int i = 0; i < n; i++) {
        if (s->frameAcc <= 0) { seq_update(s); s->frameAcc += perFrame; }
        s->frameAcc -= 1;
        float L = 0, R = 0;
        for (int ch = 0; ch < NCHAN; ch++) {
            SdChan *c = &s->c[ch];
            if (!c->active) continue;
            float x;
            if (c->kind == 1) {
                SdWave *w = c->wave;
                int p = (int)c->pos;
                if (p >= w->len) {
                    if (!w->loops) { c->active = 0; continue; }
                    c->pos = w->loopStart + fmod(c->pos - w->loopStart, (double)(w->len - w->loopStart));
                    p = (int)c->pos;
                }
                int q = p + 1;
                if (q >= w->len) q = w->loops ? w->loopStart : p;
                double f = c->pos - p;
                x = (float)((w->pcm[p] * (1 - f) + w->pcm[q] * f) / 32768.0);
                c->pos += c->rate / OUT_RATE;
            } else if (c->kind == 2) {
                int step = (int)c->pos & 7;
                x = c->duty == 7 ? -1.0f : (step >= 7 - c->duty ? 1.0f : -1.0f);
                c->pos += c->rate / OUT_RATE;
                if (c->pos >= 8) c->pos -= 8 * floor(c->pos / 8);
            } else {
                double before = c->pos;
                c->pos += c->rate / OUT_RATE;
                for (int k = (int)before; k < (int)c->pos; k++) {
                    int bit = c->lfsr & 1;
                    c->lfsr >>= 1;
                    if (bit) c->lfsr ^= 0x6000;
                }
                if (c->pos > 1e9) c->pos = 0;
                x = (c->lfsr & 1) ? -1.0f : 1.0f;
            }
            L += x * c->l;
            R += x * c->r;
        }
        L *= 0.35f; R *= 0.35f;
        int l = (int)lrintf(L * 32767), r = (int)lrintf(R * 32767);
        buf[i * 2] = (int16_t)(l > 32767 ? 32767 : l < -32768 ? -32768 : l);
        buf[i * 2 + 1] = (int16_t)(r > 32767 ? 32767 : r < -32768 ? -32768 : r);
    }
    return 0;
}

static void sd_snapshot(RipEmu *e, RipVF *out)
{
    Sd *s = e->ctx;
    for (int ch = 0; ch < NCHAN; ch++) {
        SdChan *c = &s->c[ch];
        RipVF *x = &out[ch];
        memset(x, 0, sizeof *x);
        x->sample = -1;
        x->keyon = (uint8_t)c->keyon;
        c->keyon = 0;
        if (!c->active || c->ripSample < 0) continue;
        x->sample = (int16_t)c->ripSample;
        x->rate = (float)c->rate;
        x->l = c->l;
        x->r = c->r;
    }
}

static void sd_close(RipEmu *e)
{
    Sd *s = e->ctx;
    if (!s) return;
    for (int a = 0; a < 4; a++) {
        for (int i = 0; i < s->arc[a].n; i++) free(s->arc[a].w[i].pcm);
        free(s->arc[a].w);
    }
    free(s->file);
    free(s);
    e->ctx = NULL;
}

// The sequence's length: its intro and the loop played twice (track 0 jumping back the second time), or to its end
static double measure(Sd *s, int *loops)
{
    s->dry = 1;
    seq_start(s);
    long frames = 0, max = (long)(SEQ_HZ * (RIP_MAX_SECONDS - 30));
    while (!s->ended && frames < max) {
        seq_update(s);
        frames++;
        if (s->t[0].loopJumps >= 2) break;
    }
    *loops = s->t[0].loopJumps >= 2;
    s->dry = 0;
    seq_start(s);
    return frames / SEQ_HZ;
}

int rip_open_sdat(RipEmu *e, Rip *r, int track, char *err, int errlen)
{
    char path[1024], want[256] = {0};
    snprintf(path, sizeof path, "%s", r->path);
    char *hash = strrchr(path, '#');
    if (hash) { snprintf(want, sizeof want, "%s", hash + 1); *hash = 0; }
    FILE *f = fopen(path, "rb");
    if (!f) { snprintf(err, errlen, "couldn't open %s", path); return 0; }
    Sd *s = calloc(1, sizeof *s);
    fseek(f, 0, SEEK_END);
    s->fileLen = ftell(f);
    fseek(f, 0, SEEK_SET);
    s->file = malloc((size_t)s->fileLen + 16);
    if (!s->file || fread(s->file, 1, (size_t)s->fileLen, f) != (size_t)s->fileLen || s->fileLen < 0x40 || memcmp(s->file, "SDAT", 4)) {
        fclose(f);
        snprintf(err, errlen, "%s isn't a DS sound archive (SDAT)", path);
        free(s->file); free(s);
        return 0;
    }
    fclose(f);
    s->rip = r;
    // which sequence
    int nseq = info_count(s, 0), seq = -1;
    if (want[0]) {
        char *end;
        long v = strtol(want, &end, 10);
        if (!*end) seq = (int)v;
        else for (int i = 0; i < nseq && seq < 0; i++) {
            const char *nm = symb_name(s, 0, i);
            if (nm && !strcmp(nm, want)) seq = i;
        }
    } else if (track >= 0) seq = track;
    else for (int i = 0; i < nseq && seq < 0; i++) if (info_entry(s, 0, i)) seq = i;
    const uint8_t *si = info_entry(s, 0, seq);
    if (!si) { snprintf(err, errlen, "%s has no sequence %s", path, want[0] ? want : "there"); sd_close(&(RipEmu){.ctx = s}); return 0; }
    int fileId = rd16(si), bankId = rd16(si + 4);
    s->seqVol = si[6];
    int sz;
    const uint8_t *sseq = sdat_file(s, fileId, &sz);
    if (!sseq || sz < 0x1C || memcmp(sseq, "SSEQ", 4)) { snprintf(err, errlen, "sequence %d's file is missing", seq); sd_close(&(RipEmu){.ctx = s}); return 0; }
    uint32_t dataOff = rd32(sseq + 0x18);
    if (dataOff >= (uint32_t)sz) dataOff = 0x1C;
    s->seq = sseq + dataOff;
    s->seqLen = sz - (int)dataOff;
    const uint8_t *bi = info_entry(s, 2, bankId);
    if (bi) {
        s->bank = sdat_file(s, rd16(bi), &s->bankLen);
        for (int a = 0; a < 4; a++) {
            int wa = rd16(bi + 4 + 2 * a);
            const uint8_t *wi = wa != 0xFFFF ? info_entry(s, 3, wa) : NULL;
            if (!wi) continue;
            int asz;
            const uint8_t *ad = sdat_file(s, rd16(wi), &asz);
            if (!ad || asz < 0x3C || memcmp(ad, "SWAR", 4)) continue;
            s->arc[a].d = ad;
            s->arc[a].size = asz;
            s->arc[a].n = (int)rd32(ad + 0x38);
            if (s->arc[a].n < 0 || s->arc[a].n > 4096) s->arc[a].n = 0;
            s->arc[a].w = calloc((size_t)(s->arc[a].n ? s->arc[a].n : 1), sizeof(SdWave));
        }
    }
    if (!s->bank) { snprintf(err, errlen, "sequence %d's instrument bank is missing", seq); sd_close(&(RipEmu){.ctx = s}); return 0; }
    for (int d = 0; d < 8; d++) s->sqSample[d] = -1;
    s->noiseSample = -1;
    const char *nm = symb_name(s, 0, seq);
    snprintf(r->title, sizeof r->title, "%s", nm ? nm : "");
    if (!r->title[0]) snprintf(r->title, sizeof r->title, "sequence %d", seq);
    snprintf(r->console, sizeof r->console, "Nintendo DS");
    int looped = 0;
    double len = measure(s, &looped);
    r->length = len + (looped ? 0 : 1.5);   // a song that ends: a moment for its last notes to fade
    r->fade = 0;
    e->ctx = s;
    e->rip = r;
    e->rate = OUT_RATE;
    e->nvoices = NCHAN;
    e->render = sd_render;
    e->snapshot = sd_snapshot;
    e->close = sd_close;
    return 1;
}
