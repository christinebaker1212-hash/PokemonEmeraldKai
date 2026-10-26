// Moemon Music Studio: the closest General MIDI instrument of the game for each original instrument
// of a rip.
//
// What is compared:
//  - timbre: the strength of the first 12 harmonics of the sample's sustained part (its loop, or the
//    middle of a one-shot), in dB under the strongest, against the same of the game's GM instruments
//    (their samples at key 60 = middle C);
//  - envelope: how much of a note is left half a second after its loudest moment in the song (struck
//    and plucked sounds die away, bowed, blown and organ sounds hold);
//  - register: bass instruments for low parts, and the other way round.
// Game Boy channels (Convert GB > GBA) are compared the same way, from the sound they make: a square at
// its duty or the wave channel's 32 steps, built from their harmonics; a noise channel becomes a drum kit
// with the GM drum each of its noise settings sounds like (brightness from the noise generator at that
// clock, length from the song's notes).
// A sample with no pitch is a drum: it gets the game GM kit's sound for the closest GM drum (kick, snare,
// hi-hat, cymbal, tom...) by its brightness and length. Anything too far from every GM instrument (a
// choir of real voices, sound effects) keeps its original sample.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rip_gm.h"
#include "inst.h"
#include "engine.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NH 12            // harmonics compared
#define FFT_N 8192

static void fft(float *re, float *im, int n) {
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { float t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double a = -2 * M_PI / len;
        float wr = (float)cos(a), wi = (float)sin(a);
        for (int i = 0; i < n; i += len) {
            float cr = 1, ci = 0;
            for (int k = 0; k < len / 2; k++) {
                float ur = re[i + k], ui = im[i + k];
                float vr = re[i + k + len / 2] * cr - im[i + k + len / 2] * ci;
                float vi = re[i + k + len / 2] * ci + im[i + k + len / 2] * cr;
                re[i + k] = ur + vr; im[i + k] = ui + vi;
                re[i + k + len / 2] = ur - vr; im[i + k + len / 2] = ui - vi;
                float nr = cr * wr - ci * wi;
                ci = cr * wi + ci * wr;
                cr = nr;
            }
        }
    }
}

typedef struct {
    float h[NH];         // harmonics, dB under the strongest (floor -50)
    float centroid;      // spectral centroid in Hz (as played at middle C for the game's)
    float noisiness;     // 0 = clean harmonics, 1 = noise between them
    int ok;
} Feat;

// get(i): the sample's i-th value (any length; loops repeat), at `rate`, fundamental f0.
typedef float (*GetFn)(const void *ctx, long i);

static void features(GetFn get, const void *ctx, long start, double rate, double f0, Feat *out) {
    static float re[FFT_N], im[FFT_N];
    memset(out, 0, sizeof *out);
    for (int i = 0; i < FFT_N; i++) {
        float w = 0.5f - 0.5f * (float)cos(2 * M_PI * i / (FFT_N - 1));
        re[i] = get(ctx, start + i) * w;
        im[i] = 0;
    }
    fft(re, im, FFT_N);
    int nb = FFT_N / 2;
    static float mag[FFT_N / 2];
    double tot = 0, wsum = 0;
    for (int i = 1; i < nb; i++) {
        mag[i] = sqrtf(re[i] * re[i] + im[i] * im[i]);
        double hz = i * rate / FFT_N;
        tot += mag[i] * hz;
        wsum += mag[i];
    }
    if (wsum <= 1e-9) return;
    out->centroid = (float)(tot / wsum);
    float best = 0;
    double harm = 0;
    for (int k = 0; k < NH; k++) {
        double fh = f0 * (k + 1);
        int c = (int)(fh * FFT_N / rate + 0.5);
        int sp = (int)(fh * 0.03 * FFT_N / rate) + 2;
        float m = 0;
        for (int i = c - sp; i <= c + sp; i++) if (i > 0 && i < nb && mag[i] > m) m = mag[i];
        out->h[k] = m;
        harm += m;
        if (m > best) best = m;
    }
    double all = 0;
    for (int i = 1; i < nb && i * rate / FFT_N < f0 * (NH + 0.5); i++) all += mag[i];
    out->noisiness = all > 0 ? (float)(1 - harm * 8 / all) : 1;
    if (out->noisiness < 0) out->noisiness = 0;
    for (int k = 0; k < NH; k++) {
        float db = best > 0 && out->h[k] > 0 ? 20 * log10f(out->h[k] / best) : -50;
        out->h[k] = db < -50 ? -50 : db;
    }
    out->ok = 1;
}

typedef struct { const int16_t *pcm; int len, loopStart, loops; } P16;
static float get16(const void *c, long i) {
    const P16 *p = c;
    if (i < p->len) return i >= 0 ? p->pcm[i] / 32768.0f : 0;
    if (!p->loops || p->len <= p->loopStart) return 0;
    return p->pcm[p->loopStart + (i - p->len) % (p->len - p->loopStart)] / 32768.0f;
}
typedef struct { const int8_t *d; int len, loopStart, loops; } P8;
static float get8(const void *c, long i) {
    const P8 *p = c;
    if (i < p->len) return i >= 0 ? p->d[i] / 128.0f : 0;
    if (!p->loops || p->len <= p->loopStart) return 0;
    return p->d[p->loopStart + (i - p->len) % (p->len - p->loopStart)] / 128.0f;
}

// a sound made of harmonics (k = 1..n at f0)
typedef struct { float a[64]; int n; double f0, rate; } Additive;
static float get_add(const void *c, long i) {
    const Additive *p = c;
    double t = (double)i / p->rate, v = 0;
    for (int k = 1; k <= p->n; k++) if (p->a[k - 1] != 0) v += p->a[k - 1] * sin(2 * M_PI * k * p->f0 * t);
    return (float)v;
}
typedef struct { const float *b; long n; } PF;
static float getf(const void *c, long i) { const PF *p = c; return i >= 0 && i < p->n ? p->b[i] : 0; }

#define GB_RATE 42048.0
#define GB_F0 261.63

// a Game Boy square (duty 0-3 = 12.5 / 25 / 50 / 75%), band-limited
static void gb_square(int duty, Additive *o) {
    static const double d[4] = {0.125, 0.25, 0.5, 0.75};
    memset(o, 0, sizeof *o);
    o->f0 = GB_F0; o->rate = GB_RATE; o->n = 64;
    for (int k = 1; k <= 64; k++) o->a[k - 1] = (float)fabs(2 * sin(M_PI * k * d[duty & 3]) / (M_PI * k));
}
// the wave channel's 32 4-bit steps, each held for 1/32 of the cycle
static void gb_wave(const uint8_t *w, Additive *o) {
    memset(o, 0, sizeof *o);
    o->f0 = GB_F0; o->rate = GB_RATE; o->n = 64;
    for (int k = 1; k <= 64; k++) {
        double re = 0, im = 0;
        for (int j = 0; j < 32; j++) {
            double x = ((w[j] & 15) - 7.5) / 7.5;
            re += x * cos(2 * M_PI * k * j / 32);
            im -= x * sin(2 * M_PI * k * j / 32);
        }
        double hold = k % 32 ? sin(M_PI * k / 32.0) / (M_PI * k / 32.0) : 0;
        o->a[k - 1] = (float)(2 * sqrt(re * re + im * im) / 32 * fabs(hold));
    }
}
// The noise generator (15-bit, or 7-bit "short") clocked at hz, heard at GB_RATE (each output sample the
// average of the bits clocked out during it).
static void gb_noise(double hz, int shortMode, float *out, long n) {
    unsigned lfsr = 0x7FFF;
    double acc = 0, step = hz / GB_RATE;
    float last = 1;
    for (long i = 0; i < n; i++) {
        acc += step;
        double sum = 0;
        int cnt = 0;
        while (acc >= 1) {
            acc -= 1;
            unsigned b = (lfsr ^ (lfsr >> 1)) & 1;
            lfsr = (lfsr >> 1) | (b << 14);
            if (shortMode) lfsr = (lfsr & ~0x40u) | (b << 6);
            last = (~lfsr & 1) ? 1.0f : -1.0f;
            sum += last;
            if (++cnt > 64) { acc = 0; break; }
        }
        out[i] = cnt ? (float)(sum / cnt) : last;
    }
}
// the clock of the noise setting MP2K plays at a key (gNoiseTable, the inverse of rip_song.c's noise_key)
static double noise_hz(int key) {
    int i = key - 21;
    if (i < 0) i = 0;
    if (i > 59) i = 59;
    int e = sf_gNoiseTable[i], sh = e >> 4, rr = e & 7;
    return 524288.0 / (rr ? rr : 0.5) / (double)(2 << sh);
}

// ------------------------------------------------------------------ the game's GM instruments
typedef struct {
    int prog;            // GM program
    int preset;
    SfVoice v;           // the DirectSound voice that plays middle C
    Feat f;
    float sustain;       // 0 dies away, 1 holds
    float rms;           // of its sample's sustained part
} GmCand;

static GmCand sCand[128];
static int sNumCand = -1;

static const SfVoice *voice_at_key(const SfVoice *v, int key) {
    if (v->type & 0x40) {
        const SfVoice *g = group_voices(v->sub);
        const uint8_t *t = keysplit_table(v->ks);
        if (!g || !t) return NULL;
        return &g[t[key]];
    }
    return v;
}

static void build_candidates(void) {
    sNumCand = 0;
    for (int prog = 0; prog < 128; prog++) {
        const SfVoice *pv = &sf_presets[sf_gm_preset[prog]].voice;
        const SfVoice *v = voice_at_key(pv, 60);
        if (!v || (v->type & 0xC7) || v->ref < 0 || v->ref >= SF_NUM_SAMPLES) continue;   // samples only
        const SfSample *s = &sf_samples[v->ref];
        P8 p = {(const int8_t *)(sf_pcm + s->offset), (int)s->size, (int)s->loopStart, (s->flags & 0xC0000000u) != 0};
        double rate = s->freq / 1024.0;     // what middle C plays it at
        if (rate < 1000 || p.len < 256) continue;
        GmCand *c = &sCand[sNumCand];
        memset(c, 0, sizeof *c);
        c->prog = prog;
        c->preset = sf_gm_preset[prog];
        c->v = *v;
        long start = p.loops ? p.loopStart : p.len / 4;
        features(get8, &p, start, rate, 261.63, &c->f);
        if (!c->f.ok) continue;
        // envelope: a looping sample with a high sustain holds; a short decay / low sustain dies away
        float sus = v->sustain / 255.0f;
        if (!p.loops) sus *= 0.3f;
        if (v->decay < 200 && v->sustain < 128) sus *= 0.5f;
        c->sustain = sus;
        double sum = 0;
        for (int i = 0; i < 4096; i++) { float x = get8(&p, start + i); sum += x * x; }
        c->rms = (float)sqrt(sum / 4096);
        sNumCand++;
    }
}


// Where each GM instrument usually plays (MIDI keys): a part far outside it isn't that instrument.
static void gm_range(int prog, int *lo, int *hi) {
    static const unsigned char fam[16][2] = {
        {21, 108}, {60, 108}, {36, 96}, {40, 86}, {28, 60}, {36, 100}, {28, 100}, {40, 86},
        {50, 96}, {60, 108}, {36, 96}, {36, 96}, {36, 96}, {48, 96}, {36, 84}, {24, 96}};
    *lo = fam[prog / 8][0];
    *hi = fam[prog / 8][1];
    switch (prog) {
    case 40: *lo = 55; *hi = 103; break;      // violin
    case 41: *lo = 48; *hi = 91; break;       // viola
    case 42: *lo = 36; *hi = 76; break;       // cello
    case 43: *lo = 28; *hi = 60; break;       // contrabass
    case 56: *lo = 54; *hi = 86; break;       // trumpet
    case 57: *lo = 40; *hi = 74; break;       // trombone
    case 58: *lo = 28; *hi = 58; break;       // tuba
    case 60: *lo = 41; *hi = 77; break;       // horn
    case 70: *lo = 34; *hi = 72; break;       // bassoon
    case 71: *lo = 50; *hi = 91; break;       // clarinet
    case 72: *lo = 74; *hi = 108; break;      // piccolo
    case 73: *lo = 60; *hi = 96; break;       // flute
    }
}

// Timbre: the harmonics in four groups (1, 2-3, 4-6, 7-12), compared as levels, plus how bright.
static float shape_dist(const Feat *a, const Feat *b) {
    static const int g0[4] = {0, 1, 3, 6}, g1[4] = {1, 3, 6, 12};
    float d = 0;
    for (int g = 0; g < 4; g++) {
        float x = 0, y = 0;
        for (int k = g0[g]; k < g1[g]; k++) { x += a->h[k]; y += b->h[k]; }
        x /= g1[g] - g0[g];
        y /= g1[g] - g0[g];
        d += fabsf(x - y) * (g == 0 ? 1.5f : 1.0f);
    }
    return d / 4.5f;
}

static float cand_dist(const Feat *f, float sus, float key, const GmCand *g) {
    float d = shape_dist(f, &g->f);
    if (sus >= 0) d += 12 * fabsf(sus - g->sustain);
    d += 6 * fabsf(f->noisiness - g->f.noisiness);
    if (key > 0) {
        int lo, hi;
        gm_range(g->prog, &lo, &hi);
        float out = key < lo ? lo - key : key > hi ? key - hi : 0;
        d += out > 12 ? 8 : out * 0.66f;
    }
    return d;
}

// ------------------------------------------------------------------ drums
typedef struct { int key; float centroid; float len; } DrumRef;
// rough brightness (Hz) and length (s) of the GM drums the game's kit has
static const DrumRef k_drums[] = {
    {36, 150, 0.30f},   // kick
    {38, 1800, 0.25f},  // snare
    {40, 2500, 0.25f},  // snare 2
    {39, 2200, 0.30f},  // clap
    {42, 7000, 0.08f},  // closed hi-hat
    {46, 6500, 0.45f},  // open hi-hat
    {49, 5500, 1.50f},  // crash
    {51, 5000, 1.20f},  // ride
    {41, 250, 0.45f},   // low tom
    {45, 400, 0.40f},   // mid tom
    {48, 600, 0.35f},   // high tom
    {56, 1200, 0.20f},  // cowbell
    {37, 1500, 0.06f},  // side stick
    {69, 6000, 0.15f},  // cabasa
    {70, 5000, 0.12f},  // maracas
};

static int drum_key_for(float centroid, float lenSec) {
    int best = 38;
    float bd = 1e30f;
    for (size_t i = 0; i < sizeof k_drums / sizeof k_drums[0]; i++) {
        float d = fabsf(log2f((centroid + 50) / (k_drums[i].centroid + 50))) * 2 +
                  fabsf(log2f((lenSec + 0.03f) / (k_drums[i].len + 0.03f)));
        if (d < bd) { bd = d; best = k_drums[i].key; }
    }
    return best;
}

// ------------------------------------------------------------------ Game Boy channels
// How loud a note is in the engine over its first `frames` game frames (the song's typical note length):
// a plucked or struck sound counts as loud as it is while the song's notes last.
static float note_level(const SfVoice *v, int key, int frames) {
    static Engine e;
    static EInst inst;
    static float buf[ENG_FRAME_SAMPLES * 2];
    eng_init(&e);
    inst_make(v, &inst);
    eng_preview_on(&e, &inst, key, 127);
    double sum = 0;
    long n = 0;
    for (int f = 0; f < frames; f++) {
        eng_frame(&e, buf);
        for (int i = 0; i < ENG_FRAME_SAMPLES * 2; i++) sum += (double)buf[i] * buf[i];
        n += ENG_FRAME_SAMPLES * 2;
    }
    eng_preview_off(&e);
    return n ? (float)sqrt(sum / n) : 0;
}
static int note_frames(float sec) {
    int f = (int)lroundf(sec * 60);
    return f < 3 ? 3 : f > 30 ? 30 : f;
}
static float loud_ratio(const SfVoice *orig, const SfVoice *gm, int key, int frames) {
    if (key < 24 || key > 108) key = 60;
    float a = note_level(orig, key, frames), b = note_level(gm, key, frames);
    return a > 0 && b > 0 ? a / b : 1;
}

// Panning: the engine plays a Game Boy channel panned to one side at the same level on that side (the
// other is off), but a sample panned to one side twice as loud there (half on each side in the middle).
// The level match is made in the middle; this brings it to how the song pans the slot (energy-weighted).
static float pan_fix(const Rip *r, const RipSong *song, int slot) {
    int si = song->slotSample[slot];
    double num = 0, den = 0;
    const double ds0 = 128.0 * 128 + 127.0 * 127, cgb0 = 2.0;
    for (long k = 0; k < (long)r->nframes * r->nvoices; k++) {
        const RipVF *x = &r->vf[k];
        if (x->sample != si) continue;
        float a = x->l > x->r ? x->l : x->r;
        if (a <= 1e-6f) continue;
        int pan = (int)lroundf(64.0f + 63.0f * (x->r - x->l) / (x->r + x->l));
        int y = 2 * (pan - 64);
        double rr = y + 128, ll = 127 - y;
        double ds = rr * rr + ll * ll;
        double cgb = (rr / 2 >= ll || ll / 2 >= rr) ? 1.0 : 2.0;
        num += (double)a * a * (ds / ds0) / (cgb / cgb0);
        den += (double)a * a;
    }
    return den > 0 && num > 0 ? (float)sqrt(den / num) : 1.0f;
}

// a square or the wave channel: the GM instrument closest to how it sounds, how it's played and where
static void gb_tone(const Rip *r, const RipSample *rs, const RipSong *song, int slot, RipGm *out) {
    Additive a;
    if (rs->kind == RIPS_SQUARE) gb_square(rs->param & 3, &a);
    else gb_wave(rs->wave, &a);
    Feat f;
    features(get_add, &a, 0, a.rate, a.f0, &f);
    if (!f.ok) return;
    float key = song->slotKey[slot];
    if (key > 0 && rs->kind == RIPS_WAVE) key -= 12;     // the wave channel's notes are written an octave up
    // the wave channel has no volume envelope (every note holds until it stops), so how it holds says
    // nothing; where it plays says more (it's usually the bass)
    float sus = rs->kind == RIPS_WAVE ? -1 : song->slotSustain[slot];
    int best = -1;
    float bd = 1e30f;
    for (int c = 0; c < sNumCand; c++) {
        float d = cand_dist(&f, sus, key, &sCand[c]);
        if (key > 0) {
            int lo, hi;
            gm_range(sCand[c].prog, &lo, &hi);
            d += 0.08f * fabsf(key - (lo + hi) * 0.5f);   // the middle of the instrument's range fits best
        }
        if (getenv("MMS_GM_DEBUG") && d < 9) fprintf(stderr, "   %5.1f %3d %s\n", d, sCand[c].prog, sf_gm_names[sCand[c].prog]);
        if (d < bd) { bd = d; best = c; }
    }
    if (best < 0) return;
    SfVoice gv = sCand[best].v;
    gv.attack = 255; gv.decay = 0; gv.sustain = 255; gv.release = 0;   // the envelope is in the expression
    out->voice[slot] = gv;
    out->prog[slot] = sCand[best].prog;
    out->dist[slot] = bd;
    out->gain[slot] = loud_ratio(&song->orig[slot], &gv, (int)lroundf(song->slotKey[slot]), note_frames(song->slotDur[slot])) *
                      pan_fix(r, song, slot);
    if (getenv("MMS_GM_DEBUG"))
        fprintf(stderr, "slot %d %s key %.0f sus %.2f -> %d %s (%.1f) gain %.2f\n", slot, rs->kind == RIPS_SQUARE ? "square" : "wave",
                key, song->slotSustain[slot], sCand[best].prog, sf_gm_names[sCand[best].prog], bd, out->gain[slot]);
}

// The game kit's drums as they sound: brightness of the sample as the kit plays it, and how long it lasts
// (a one-shot's length; a looping one held or dying away by its envelope).
typedef struct { int key; float centroid, len; } KitDrum;
static KitDrum sKit[16];
static int sNumKit = -1;

static void build_kit(const SfVoice *kit) {
    static const int keys[] = {36, 38, 40, 39, 42, 46, 44, 45, 49, 51, 57, 52, 37};
    sNumKit = 0;
    const SfVoice *g = group_voices(kit->sub);
    if (!g) return;
    for (size_t i = 0; i < sizeof keys / sizeof keys[0] && sNumKit < 16; i++) {
        int k = keys[i];
        if (drum_resolve(kit, k) != k) continue;       // the kit has nothing of its own there
        const SfVoice *v = &g[k];
        if (v->type & 0xC7 || v->ref < 0 || v->ref >= SF_NUM_SAMPLES) continue;
        const SfSample *ss = &sf_samples[v->ref];
        int fixed = v->type & 0x08;
        double rate = ss->freq / 1024.0 * pow(2.0, ((fixed ? v->key : k) - 60) / 12.0);
        P8 q = {(const int8_t *)(sf_pcm + ss->offset), (int)ss->size, (int)ss->loopStart, (ss->flags & 0xC0000000u) != 0};
        Feat f;
        features(get8, &q, 0, rate, 100, &f);
        float len = (float)(ss->size / rate);
        if (q.loops) {
            // held at its sustain, or dying away by its decay (to a hundredth)
            if (v->sustain > 16) len = 1.5f;
            else len = v->decay > 0 && v->decay < 255 ? (float)(log(0.01) / log(v->decay / 256.0) / 60.0) : 1.5f;
        }
        for (int j = 0; j < sNumKit; j++) if (sKit[j].centroid == f.centroid && sKit[j].len == len) len = -1;   // the same sound again
        if (len < 0) continue;
        sKit[sNumKit].key = k;
        sKit[sNumKit].centroid = f.centroid;
        sKit[sNumKit].len = len;
        sNumKit++;
    }
}

static int kit_drum_for(float centroid, float lenSec) {
    int best = 38;
    float bd = 1e30f;
    for (int i = 0; i < sNumKit; i++) {
        float d = fabsf(log2f((centroid + 50) / (sKit[i].centroid + 50))) * 2 +
                  fabsf(log2f((lenSec + 0.03f) / (sKit[i].len + 0.03f)));
        if (d < bd) { bd = d; best = sKit[i].key; }
    }
    return best;
}

// a noise channel: a kit with, at every key (noise setting), the GM drum that setting sounds like
static void noise_kit(const Rip *r, const RipSong *song, int slot, const SfVoice *kit, RipGm *out) {
    (void)r;
    const SfVoice *g = group_voices(kit->sub);
    if (!g) return;
    const RipSample *rs = &r->samples[song->slotSample[slot]];
    CustomGroup *ng = realloc(out->groups, sizeof(CustomGroup) * (size_t)(out->ngroups + 1));
    if (!ng) return;
    out->groups = ng;
    CustomGroup *cg = &out->groups[out->ngroups];
    memset(cg, 0, sizeof *cg);
    snprintf(cg->name, sizeof cg->name, "%s_noise%d", song->prefix, out->ngroups);
    float defLen = song->slotDur[slot] > 0 ? song->slotDur[slot] : 0.2f;
    static float buf[FFT_N];
    float cent[60];
    for (int i = 0; i < 60; i++) {
        gb_noise(noise_hz(21 + i), rs->param & 1, buf, FFT_N);
        PF pf = {buf, FFT_N};
        Feat f;
        features(getf, &pf, 0, GB_RATE, 100, &f);
        cent[i] = f.centroid;
    }
    float lsum = 0, lcache[128];
    int lused = 0;
    int nf = note_frames(defLen);
    for (int k = 0; k < 128; k++) lcache[k] = -1;
    for (int k = 0; k < 128; k++) {
        int i = k < 21 ? 0 : k > 80 ? 59 : k - 21;
        float kd = song->keyDur ? song->keyDur[slot * 128 + k] : -1;
        if (sNumKit < 0) build_kit(kit);
        int dk = drum_resolve(kit, kit_drum_for(cent[i], kd >= 0 ? kd : defLen));
        SfVoice dv = g[dk];
        if (dv.type & 0xC7) { cg->v[k] = VOICE_PLACEHOLDER; continue; }
        dv.type |= 0x08;                                 // the kit's own pitch, whatever the key
        dv.attack = 255; dv.decay = 0; dv.sustain = 255; dv.release = 0;
        cg->v[k] = dv;
        if (kd >= 0) {                                   // a key the song plays: counts for the volume
            if (lcache[dk] < 0) lcache[dk] = note_level(&dv, 60, nf);
            lsum += lcache[dk];
            lused++;
            if (getenv("MMS_GM_DEBUG")) fprintf(stderr, "slot %d noise key %d (%.0f Hz, bright %.0f, %.2f s) -> drum %d\n", slot, k, noise_hz(k), cent[i], kd, dk);
        }
    }
    SfVoice kv = *kit;
    kv.sub = (int16_t)(INST_CUSTOM + out->ngroups);
    out->ngroups++;
    out->voice[slot] = kv;
    out->prog[slot] = 128;
    out->dist[slot] = 0;
    float lo = note_level(&song->orig[slot], (int)lroundf(song->slotKey[slot] > 0 ? song->slotKey[slot] : 60), nf);
    if (lused && lsum > 0 && lo > 0) out->gain[slot] = lo / (lsum / lused) * pan_fix(r, song, slot);
}

// ------------------------------------------------------------------ matching
void rip_gm_match(const Rip *r, const RipSong *song, RipGm *out) {
    memset(out, 0, sizeof *out);
    out->cgbNorm = 1;
    for (int s = 0; s < 128; s++) { out->prog[s] = -1; out->voice[s].type = 0xFF; out->gain[s] = 1; }
    if (sNumCand < 0) build_candidates();
    const SfVoice *kit = &sf_presets[SF_GM_DRUM_PRESET].voice;
    for (int slot = 0; slot < 128; slot++) {
        int si = song->slotSample[slot];
        if (si < 0 || si >= r->nsamples) continue;
        const RipSample *rs = &r->samples[si];
        if (rs->kind == RIPS_NOISE) { noise_kit(r, song, slot, kit, out); continue; }
        if (rs->kind == RIPS_SQUARE || rs->kind == RIPS_WAVE) { gb_tone(r, rs, song, slot, out); continue; }
        if (rs->kind != RIPS_PCM || !rs->pcm || rs->len < 64) continue;
        P16 p = {rs->pcm, rs->len, rs->loopStart, rs->loops};
        float origRms;
        {
            long st = rs->loops ? rs->loopStart : rs->len / 4;
            double sum = 0;
            for (int i = 0; i < 4096; i++) { float x = get16(&p, st + i); sum += x * x; }
            origRms = (float)sqrt(sum / 4096) / (rs->peak > 0 ? rs->peak : 1);   // as stored (peak-normalized)
        }
        if (!rs->pitched || (song->slotNoise[slot] && !rs->loops)) {
            // a drum: its brightness over its first 0.1 s and how long it sounds
            Feat f;
            double f0 = 100;
            features(get16, &p, 0, rs->rate, f0, &f);
            float lenSec = (float)rs->len / (float)rs->rate;
            if (rs->loops) lenSec = 2.0f;
            if (song->slotDur[slot] > 0 && song->slotDur[slot] < lenSec) lenSec = song->slotDur[slot];
            int key = drum_key_for(f.centroid, lenSec);
            key = drum_resolve(kit, key);
            const SfVoice *g = group_voices(kit->sub);
            if (!g) continue;
            SfVoice dv = g[key];
            if (dv.type & 0xC7) continue;
            dv.type |= 0x08;                         // keeps the kit's pitch whatever key the note is on
            dv.attack = 255; dv.decay = 0; dv.sustain = 255; dv.release = 0;
            out->voice[slot] = dv;
            out->prog[slot] = 128 + key;
            out->dist[slot] = 0;
            const SfSample *ss = dv.ref >= 0 && dv.ref < SF_NUM_SAMPLES ? &sf_samples[dv.ref] : NULL;
            if (ss) {
                P8 q = {(const int8_t *)(sf_pcm + ss->offset), (int)ss->size, (int)ss->loopStart, (ss->flags & 0xC0000000u) != 0};
                double sum = 0;
                long st = q.loops ? q.loopStart : q.len / 4;
                for (int i = 0; i < 4096; i++) { float x = get8(&q, st + i); sum += x * x; }
                float gr = (float)sqrt(sum / 4096);
                if (gr > 0) out->gain[slot] = origRms / gr;
            }
            continue;
        }
        Feat f;
        double f0 = 440.0 * pow(2.0, (rs->rootKey - 69) / 12.0);
        long start = rs->loops ? rs->loopStart : rs->len / 4;
        features(get16, &p, start, rs->rate, f0, &f);
        if (!f.ok) continue;
        float sus = song->slotSustain[slot];
        float key = song->slotKey[slot];
        int best = -1;
        float bd = 1e30f;
        for (int c = 0; c < sNumCand; c++) {
            float d = cand_dist(&f, sus, key, &sCand[c]);
            if (d < bd) { bd = d; best = c; }
        }
        if (getenv("MMS_GM_DEBUG")) {
            fprintf(stderr, "slot %d root %.1f key %.0f sus %.2f noise %.2f h:", slot, rs->rootKey, key, sus, f.noisiness);
            for (int k = 0; k < NH; k++) fprintf(stderr, " %.0f", f.h[k]);
            fprintf(stderr, "\n");
            // the three closest
            for (int pass = 0; pass < 3; pass++) {
                int bi = -1; float bv = 1e30f;
                for (int c = 0; c < sNumCand; c++) {
                    float d = cand_dist(&f, sus, key, &sCand[c]);
                    if (d < bv && d > (pass ? out->dist[slot] : -1)) { bv = d; bi = c; }
                }
                if (bi >= 0) { fprintf(stderr, "   %5.1f %3d %s (sus %.2f)\n", bv, sCand[bi].prog, sf_gm_names[sCand[bi].prog], sCand[bi].sustain); out->dist[slot] = bv; }
            }
        }
        if (best < 0) continue;
        out->dist[slot] = bd;
        // too far from every GM sound: keep the original sample
        if (bd > RIP_GM_MAX_DIST) continue;
        // the GM instrument's middle-C sample with a flat envelope: the original's envelope is in the expression
        SfVoice gv = sCand[best].v;
        gv.attack = 255; gv.decay = 0; gv.sustain = 255; gv.release = 0;
        out->voice[slot] = gv;
        out->prog[slot] = sCand[best].prog;
        if (sCand[best].rms > 0) out->gain[slot] = origRms / sCand[best].rms;
    }
    for (int s = 0; s < 128; s++) {
        if (out->gain[s] < 0.25f) out->gain[s] = 0.25f;
        if (out->gain[s] > 4) out->gain[s] = 4;
    }
    // Game Boy songs play at the hardware's level, which a GBA instrument can need more than a volume of
    // 127 to reach: everything comes down together instead
    if (r->kind == RIP_GBS) {
        float m = 1;
        for (int s = 0; s < 128; s++) if (out->voice[s].type != 0xFF && out->gain[s] > m) m = out->gain[s];
        out->cgbNorm = 1 / m;
        for (int s = 0; s < 128; s++) if (out->voice[s].type != 0xFF) out->gain[s] /= m;
    }
}
