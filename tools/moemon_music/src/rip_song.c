// Moemon Music Studio: turning what a sound chip did into a MIDI and instruments.
//
// Every chip voice becomes a MIDI track (MP2K tracks play any number of notes, but a chip voice plays
// one at a time, so each note gets its own pitch bends and volume). A tick is one game frame (24 ticks
// a beat at 150 BPM is what MP2K plays as one tick a frame), so the song keeps the original's timing to
// the frame. Volume is the voice's own level each frame (envelope included) as expression (CC11), with
// the note's loudest moment as its velocity, and the instruments get a flat envelope: the original's
// envelope is in the expression. Pitch is the key closest to the note plus bends for the rest
// (vibrato, slides, detuning), with a bend range per track wide enough for its slides.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "rip_song.h"
#include "inst.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ------------------------------------------------------------------ MIDI writing
typedef struct { uint8_t *b; size_t n, cap; } MBuf;

static void mput(MBuf *m, const void *d, size_t n) {
    if (m->n + n > m->cap) {
        size_t cap = (m->n + n) * 2 + 1024;
        uint8_t *nb = realloc(m->b, cap);
        if (!nb) return;
        m->b = nb;
        m->cap = cap;
    }
    memcpy(m->b + m->n, d, n);
    m->n += n;
}
static void m8(MBuf *m, int v) { uint8_t x = (uint8_t)v; mput(m, &x, 1); }
static void mvar(MBuf *m, uint32_t v) {
    uint8_t t[5];
    int n = 0;
    t[n++] = v & 0x7F;
    while (v >>= 7) t[n++] = 0x80 | (v & 0x7F);
    while (n) m8(m, t[--n]);
}
typedef struct { MBuf b; uint32_t last; } MTrk;
static void mev(MTrk *t, uint32_t tick, int status, int a, int b) {
    if (tick < t->last) tick = t->last;
    mvar(&t->b, tick - t->last);
    t->last = tick;
    m8(&t->b, status);
    m8(&t->b, a);
    if ((status & 0xF0) != 0xC0 && (status & 0xF0) != 0xD0) m8(&t->b, b);
}
static void mmeta(MTrk *t, uint32_t tick, int type, const void *d, int len) {
    if (tick < t->last) tick = t->last;
    mvar(&t->b, tick - t->last);
    t->last = tick;
    m8(&t->b, 0xFF);
    m8(&t->b, type);
    mvar(&t->b, (uint32_t)len);
    mput(&t->b, d, (size_t)len);
}

// ------------------------------------------------------------------ analysis
float rip_detect_root(const int16_t *pcm, int len, int rate, int loopStart, int loops, int *pitched) {
    *pitched = 0;
    if (!pcm || len < 16) return 60;
    enum { N = 4096 };
    static float buf[N];
    int n = 0;
    if (loops && len - loopStart >= 8) {
        int ll = len - loopStart;
        for (n = 0; n < N; n++) buf[n] = pcm[loopStart + n % ll];
    } else {
        int start = len / 10, take = len - start;
        if (take > N) take = N;
        for (n = 0; n < take; n++) buf[n] = pcm[start + n];
    }
    if (n < 64) return 60;
    double mean = 0;
    for (int i = 0; i < n; i++) mean += buf[i];
    mean /= n;
    for (int i = 0; i < n; i++) buf[i] -= (float)mean;
    int tmin = rate / 2500, tmax = rate / 30;
    if (tmin < 2) tmin = 2;
    if (tmax > n / 2) tmax = n / 2;
    if (tmax <= tmin + 2) return 60;
    int span = n - tmax;
    static float r[N];
    float best = 0;
    for (int t = tmin; t <= tmax; t++) {
        double s = 0, e1 = 0, e2 = 0;
        for (int i = 0; i < span; i++) {
            s += (double)buf[i] * buf[i + t];
            e1 += (double)buf[i] * buf[i];
            e2 += (double)buf[i + t] * buf[i + t];
        }
        r[t] = (e1 > 0 && e2 > 0) ? (float)(s / sqrt(e1 * e2)) : 0;
        if (r[t] > best) best = r[t];
    }
    if (best < 0.6f) return 60;
    // the first strong peak (the period, not a multiple of it)
    int pick = -1;
    for (int t = tmin + 1; t < tmax; t++)
        if (r[t] >= 0.9f * best && r[t] >= r[t - 1] && r[t] >= r[t + 1]) { pick = t; break; }
    if (pick < 0) return 60;
    double a = r[pick - 1], b = r[pick], c = r[pick + 1];
    double den = a - 2 * b + c, off = den != 0 ? 0.5 * (a - c) / den : 0;
    if (off > 0.5) off = 0.5;
    if (off < -0.5) off = -0.5;
    double period = pick + off;
    double f0 = rate / period;
    *pitched = 1;
    return (float)(69 + 12 * log2(f0 / 440.0));
}

static double sinc(double x) { return fabs(x) < 1e-9 ? 1.0 : sin(M_PI * x) / (M_PI * x); }

int rip_resample(const int16_t *in, int len, int fromRate, int loopStart, int loops, double toRate,
                 int16_t **out, int *outLoopStart) {
    if (len <= 0) { *out = NULL; *outLoopStart = 0; return 0; }
    double ratio = toRate / fromRate;
    int ll = len - loopStart;
    int outLen, outLoop;
    if (loops && ll > 0) {
        int outLL = (int)lround(ll * ratio);
        if (outLL < 1) outLL = 1;
        ratio = (double)outLL / ll;               // a whole number of samples per loop
        outLoop = (int)lround(loopStart * ratio);
        outLen = outLoop + outLL;
    } else {
        outLen = (int)ceil(len * ratio);
        outLoop = 0;
    }
    int16_t *o = malloc(sizeof(int16_t) * (outLen > 0 ? outLen : 1));
    if (!o) { *out = NULL; return 0; }
    double cutoff = ratio < 1 ? ratio : 1;        // of the source Nyquist
    const int taps = 16;
    double step = 1.0 / ratio;
    for (int j = 0; j < outLen; j++) {
        double t;
        if (loops && ll > 0 && j >= outLoop) t = loopStart + (j - outLoop) * ((double)ll / (outLen - outLoop));
        else t = j * step;
        int c = (int)floor(t);
        double acc = 0, wsum = 0;
        int reach = (int)ceil(taps / cutoff);
        for (int k = c - reach + 1; k <= c + reach; k++) {
            double x = (t - k) * cutoff;
            if (fabs(x) >= taps) continue;
            double w = sinc(x) * (0.42 + 0.5 * cos(M_PI * x / taps) + 0.08 * cos(2 * M_PI * x / taps));
            int idx = k;
            double v;
            if (idx < 0) v = 0;
            else if (idx >= len) v = (loops && ll > 0) ? in[loopStart + (idx - len) % ll] : 0;
            else v = in[idx];
            acc += v * w;
            wsum += w;
        }
        double v = wsum > 0.5 ? acc / wsum : acc;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        o[j] = (int16_t)lround(v);
    }
    *out = o;
    *outLoopStart = outLoop;
    return outLen;
}

// ------------------------------------------------------------------ notes
typedef struct {
    int voice, f0, f1, sample;
    int key, vel;
    float peak;
} Note;

#define SILENT 1e-6f

static float amp_of(const Rip *r, const RipVF *x) {
    if (x->sample < 0) return 0;
    const RipSample *s = &r->samples[x->sample];
    if (s->kind == RIPS_PCM) return (x->l + x->r) * 0.5f * s->peak;
    // a Game Boy channel plays at its envelope's level on each side it's on (panning doesn't split it)
    return (x->l > x->r ? x->l : x->r) * 0.5f;
}

// The key whose MP2K noise setting (gNoiseTable: clock shift and divider) is closest to a noise
// frequency (Game Boy formula: 524288 / r / 2^(s+1), r = 0 counts as 0.5).
static int noise_key(float hz) {
    int best = 0;
    float bd = 1e30f;
    for (int i = 0; i < 60; i++) {
        int e = sf_gNoiseTable[i], sh = e >> 4, rr = e & 7;
        float f = 524288.0f / (rr ? rr : 0.5f) / (float)(2 << sh);
        float d = fabsf(log2f(f / hz));
        if (d < bd) { bd = d; best = i; }
    }
    return 21 + best;
}

static float semis_of(const Rip *r, const RipVF *x) {
    const RipSample *s = &r->samples[x->sample];
    if (x->rate <= 0) return s->rootKey;
    if (s->kind == RIPS_PCM) return s->rootKey + 12.0f * log2f(x->rate / (float)s->rate);
    if (s->kind == RIPS_NOISE) return (float)noise_key(x->rate);
    float k = 69.0f + 12.0f * log2f(x->rate / 440.0f);
    // MP2K plays the wave channel an octave below a square on the same key
    if (s->kind == RIPS_WAVE) k += 12.0f;
    return k;
}

static int cmp_float(const void *a, const void *b) {
    float x = *(const float *)a, y = *(const float *)b;
    return x < y ? -1 : x > y;
}

// Loop: the shortest period after which the key-ons repeat (within a frame and a half) to the end.
typedef struct { int f; uint32_t key; } KEv;

static int kev_cmp(const void *a, const void *b) {
    const KEv *x = a, *y = b;
    return x->f != y->f ? x->f - y->f : (x->key > y->key) - (x->key < y->key);
}

static int find_loop(const Note *notes, int nn, int nframes, int *ls, int *le) {
    if (nn < 16) return 0;
    KEv *ev = malloc(sizeof *ev * nn);
    if (!ev) return 0;
    for (int i = 0; i < nn; i++) { ev[i].f = notes[i].f0; ev[i].key = (uint32_t)notes[i].sample << 8 | (uint32_t)notes[i].key; }
    qsort(ev, nn, sizeof *ev, kev_cmp);
    int lastF = ev[nn - 1].f;
    // candidate periods: differences between same-key events, the most common first
    int maxP = lastF / 2 + 1;
    if (maxP < 60) { free(ev); return 0; }
    int *hist = calloc((size_t)maxP + 2, sizeof(int));
    if (!hist) { free(ev); return 0; }
    for (int i = 0; i < nn; i++)
        for (int j = i + 1; j < nn; j++) {
            int d = ev[j].f - ev[i].f;
            if (d > maxP) break;
            if (d >= 60 && ev[j].key == ev[i].key) hist[d]++;
        }
    int found = 0, bestP = 0, bestS = 0;
    // try periods from long to short? the shortest that holds to the end is the loop; collect peaks
    int cand[64], nc = 0;
    for (int pass = 0; pass < 64; pass++) {
        int bp = -1, bv = 0;
        for (int d = 60; d <= maxP; d++) {
            int v = hist[d] + hist[d - 1] + hist[d + 1];
            if (v > bv) {
                int near = 0;
                for (int c = 0; c < nc; c++) if (abs(cand[c] - d) < 3) near = 1;
                if (!near) { bv = v; bp = d; }
            }
        }
        if (bp < 0 || bv < 8) break;
        cand[nc++] = bp;
    }
    for (int c = 0; c < nc; c++) {
        int P = cand[c];
        // refine P (+-2) and find the earliest start that repeats to the end
        for (int dp = -2; dp <= 2; dp++) {
            int p = P + dp;
            if (p < 60) continue;
            // event i "repeats" when an event with the same key is at f+p (+-1)
            // scan backwards from the end: the loop start is where repeats stop holding
            int start = -1;
            int ok = 1;
            int j = nn - 1;
            int cutoff = lastF - p;      // events after this have nothing to repeat into
            for (int i = nn - 1; i >= 0; i--) {
                if (ev[i].f > cutoff) continue;
                int want = ev[i].f + p, hit = 0;
                while (j > 0 && ev[j].f > want + 1) j--;
                for (int k = j; k >= 0 && ev[k].f >= want - 1; k--)
                    if (ev[k].key == ev[i].key) { hit = 1; break; }
                if (!hit) { start = ev[i].f + 1; ok = 0; break; }
            }
            (void)ok;
            if (start < 0) start = 0;
            // the repeating part has to be long enough to be the song, not a fade-out
            if (lastF - start < 2 * p - 2) continue;
            if (!found || p < bestP || (p == bestP && start < bestS)) {
                // prefer the shorter period only if it really is a loop of the music (covers > 1/4 of the song)
                if (p * 4 < lastF && found) continue;
                found = 1; bestP = p; bestS = start;
            }
        }
    }
    free(hist);
    free(ev);
    if (!found) return 0;
    // start the loop at the first key-on at or after bestS (a clean place to jump to)
    *ls = bestS;
    *le = bestS + bestP;
    if (*le > nframes) return 0;
    return 1;
}

// ------------------------------------------------------------------ the build
static void make_prefix(const Rip *r, char *out, int n) {
    const char *src = r->title[0] ? r->title : "rip";
    int k = 0;
    for (const char *p = src; *p && k < n - 1 && k < 14; p++) {
        unsigned char c = (unsigned char)*p;
        if (isalnum(c)) out[k++] = (char)tolower(c);
        else if (k && out[k - 1] != '_') out[k++] = '_';
    }
    while (k && out[k - 1] == '_') k--;
    if (!k) { strcpy(out, "rip"); return; }
    out[k] = 0;
    if (isdigit((unsigned char)out[0]) && k < n - 2) { memmove(out + 1, out, (size_t)k + 1); out[0] = 'r'; }
}

void rip_song_free(RipSong *s) {
    custom_list_free(&s->custom, &s->ncustom);
    free(s->mid);
    s->mid = NULL;
    s->midLen = 0;
}

int rip_song_build(Rip *r, RipSong *out, float gain, char *err, int errlen) {
    memset(out, 0, sizeof *out);
    for (int i = 0; i < 128; i++) { out->slotSample[i] = -1; out->gm[i] = -1; out->orig[i] = VOICE_PLACEHOLDER; }
    out->reverb = -1;
    make_prefix(r, out->prefix, sizeof out->prefix);
    int nv = r->nvoices, nf = r->nframes;

    // the samples: loudness and pitch
    for (int i = 0; i < r->nsamples; i++) {
        RipSample *s = &r->samples[i];
        if (s->kind != RIPS_PCM || !s->pcm) { s->peak = 0.5f; s->pitched = 1; continue; }
        int pk = 0;
        double sum = 0;
        for (int k = 0; k < s->len; k++) { int a = abs(s->pcm[k]); if (a > pk) pk = a; sum += (double)s->pcm[k] * s->pcm[k]; }
        s->peak = pk / 32768.0f;
        s->rms = (float)sqrt(sum / (s->len ? s->len : 1)) / 32768.0f;
        s->rootKey = rip_detect_root(s->pcm, s->len, s->rate, s->loopStart, s->loops, &s->pitched);
    }

    // notes
    int capN = 1024, nn = 0;
    Note *notes = malloc(sizeof(Note) * capN);
    if (!notes) { snprintf(err, errlen, "out of memory"); return 0; }
    for (int v = 0; v < nv; v++) {
        int in = 0;
        Note cur = {0};
        for (int f = 0; f <= nf; f++) {
            const RipVF *x = f < nf ? &r->vf[(size_t)f * nv + v] : NULL;
            float a = x ? amp_of(r, x) : 0;
            int has = x && x->sample >= 0;
            int start = has && (x->keyon || (!in && a > SILENT) || (in && x->sample != cur.sample));
            int stop = in && (!has || start || a <= SILENT || (cur.peak > 0 && a < cur.peak * 0.0005f));
            if (stop) {
                cur.f1 = f;
                if (cur.peak > SILENT && cur.f1 > cur.f0) {
                    if (nn == capN) {
                        capN *= 2;
                        Note *nb = realloc(notes, sizeof(Note) * capN);
                        if (!nb) { free(notes); snprintf(err, errlen, "out of memory"); return 0; }
                        notes = nb;
                    }
                    notes[nn++] = cur;
                }
                in = 0;
            }
            if (start) {
                memset(&cur, 0, sizeof cur);
                cur.voice = v;
                cur.f0 = f;
                cur.sample = x->sample;
                in = 1;
            }
            if (in && x && a > cur.peak) cur.peak = a;
        }
    }
    if (!nn) { free(notes); snprintf(err, errlen, "no notes: the rip played nothing in %.0f s", nf / 60.0); return 0; }

    // which samples are used, how high they play: shift each sample's key by octaves into MIDI range
    float *lo = malloc(sizeof(float) * r->nsamples), *hi = malloc(sizeof(float) * r->nsamples);
    for (int i = 0; i < r->nsamples; i++) { lo[i] = 1e9f; hi[i] = -1e9f; }
    for (int i = 0; i < nn; i++) {
        Note *n = &notes[i];
        for (int f = n->f0; f < n->f1; f++) {
            const RipVF *x = &r->vf[(size_t)f * nv + n->voice];
            if (x->sample != n->sample) continue;
            float s = semis_of(r, x);
            if (s < lo[n->sample]) lo[n->sample] = s;
            if (s > hi[n->sample]) hi[n->sample] = s;
        }
    }
    for (int i = 0; i < r->nsamples; i++) {
        if (lo[i] > hi[i]) continue;
        RipSample *s = &r->samples[i];
        int shift = 0;
        while (lo[i] + shift < 12 && hi[i] + shift < 103) shift += 12;
        while (hi[i] + shift > 115 && lo[i] + shift > 24) shift -= 12;
        if (s->kind == RIPS_PCM) s->rootKey += (float)shift;
    }
    free(lo);
    free(hi);

    // slots: samples in order of first use
    int slotOf[4096];
    int ns = r->nsamples < 4096 ? r->nsamples : 4096;
    for (int i = 0; i < ns; i++) slotOf[i] = -1;
    int nslots = 0;
    for (int i = 0; i < nn; i++) {
        int s = notes[i].sample;
        if (s < ns && slotOf[s] < 0) {
            if (nslots < 128) { slotOf[s] = nslots; out->slotSample[nslots] = s; nslots++; }
            else slotOf[s] = 127;
        }
    }
    out->nslots = nslots;

    // keys, velocities
    float aref = 0;
    for (int i = 0; i < nn; i++) if (notes[i].peak > aref) aref = notes[i].peak;
    if (aref <= 0) aref = 1;
    // Game Boy channels only: the GBA has the same ones, so a channel at full volume stays at full
    // volume (velocity and expression 127 = the hardware's 15)
    int anyPcm = 0;
    for (int i = 0; i < nn; i++) if (r->samples[notes[i].sample].kind == RIPS_PCM) anyPcm = 1;
    if (!anyPcm) aref = 0.5f;
    out->hardwareLevels = !anyPcm;
    float *tmp = malloc(sizeof(float) * (nf + 1));
    for (int i = 0; i < nn; i++) {
        Note *n = &notes[i];
        int k = 0;
        for (int f = n->f0; f < n->f1; f++) {
            const RipVF *x = &r->vf[(size_t)f * nv + n->voice];
            if (x->sample == n->sample && x->rate > 0) tmp[k++] = semis_of(r, x);
        }
        float med = 60;
        if (k) { qsort(tmp, k, sizeof(float), cmp_float); med = tmp[k / 2]; }
        int key = (int)lroundf(med);
        n->key = key < 0 ? 0 : key > 127 ? 127 : key;
        int vel = (int)ceilf(127.0f * n->peak * gain / aref);
        n->vel = vel < 1 ? 1 : vel > 127 ? 127 : vel;
    }
    free(tmp);

    // how each instrument is played: register, envelope, note length
    for (int si = 0; si < nslots; si++) {
        int sidx = out->slotSample[si];
        int cnt = 0, nlong = 0;
        float *keys = malloc(sizeof(float) * nn), *sus = malloc(sizeof(float) * nn), *dur = malloc(sizeof(float) * nn);
        int pitches = 0, distinct = 0, lastKey = -1;
        for (int i = 0; i < nn; i++) {
            Note *n = &notes[i];
            if (n->sample != sidx) continue;
            keys[cnt] = (float)n->key;
            dur[cnt] = (n->f1 - n->f0) / 60.0f;
            cnt++;
            if (lastKey >= 0 && n->key != lastKey) distinct = 1;
            lastKey = n->key;
            pitches++;
            int fp = n->f0;
            float pk = 0;
            for (int f = n->f0; f < n->f1; f++) {
                float a = amp_of(r, &r->vf[(size_t)f * nv + n->voice]);
                if (a > pk) { pk = a; fp = f; }
            }
            // what's left after half a second: measured over up to 30 frames past the peak, scaled to 30
            int span = n->f1 - 1 - fp;
            if (span > 30) span = 30;
            if (span >= 6 && pk > 0) {
                float a = amp_of(r, &r->vf[(size_t)(fp + span) * nv + n->voice]) / pk;
                if (a < 1e-4f) a = 1e-4f;
                float v = powf(a, 30.0f / span);
                sus[nlong++] = v > 1 ? 1 : v;
            }
        }
        if (cnt) {
            qsort(keys, cnt, sizeof(float), cmp_float);
            qsort(dur, cnt, sizeof(float), cmp_float);
            out->slotKey[si] = keys[cnt / 2];
            out->slotDur[si] = dur[cnt / 2];
        }
        if (nlong) { qsort(sus, nlong, sizeof(float), cmp_float); out->slotSustain[si] = sus[nlong / 2]; }
        else out->slotSustain[si] = -1;
        out->slotNoise[si] = cnt >= 4 && !distinct;
        (void)pitches;
        free(keys); free(sus); free(dur);
    }

    // the loop
    int ls = 0, le = 0;
    out->hasLoop = find_loop(notes, nn, nf, &ls, &le);
    if (out->hasLoop) { out->loopStart = (uint32_t)ls; out->loopEnd = (uint32_t)le; }

    // the custom samples
    for (int si = 0; si < nslots; si++) {
        int sidx = out->slotSample[si];
        RipSample *s = &r->samples[sidx];
        snprintf(s->name, sizeof s->name, "%s_%02d", out->prefix, si);
        SfVoice v;
        memset(&v, 0, sizeof v);
        v.sub = v.ks = -1;
        v.attack = 255; v.decay = 0; v.sustain = 255; v.release = 0;   // the envelope is in the expression
        v.key = 60;
        if (s->kind == RIPS_PCM) {
            // as heard: about the GBA's rate at the rate the sample usually plays at
            double sum = 0, wsum = 0;
            for (int i = 0; i < nn; i++) {
                if (notes[i].sample != sidx) continue;
                const RipVF *x = &r->vf[(size_t)notes[i].f0 * nv + notes[i].voice];
                if (x->rate > 0) { sum += log(x->rate) * (notes[i].f1 - notes[i].f0); wsum += notes[i].f1 - notes[i].f0; }
            }
            double typ = wsum > 0 ? exp(sum / wsum) : s->rate;
            double to = RIP_GBA_RATE * (double)s->rate / typ;
            if (to > s->rate) to = s->rate;
            if (to < 2000) to = 2000;
            int16_t *rs = NULL;
            int rl = 0, rloop = 0;
            if (fabs(to - s->rate) < 1) {
                rs = malloc(sizeof(int16_t) * s->len);
                if (rs) memcpy(rs, s->pcm, sizeof(int16_t) * s->len);
                rl = s->len;
                rloop = s->loopStart;
            } else {
                rl = rip_resample(s->pcm, s->len, s->rate, s->loopStart, s->loops, to, &rs, &rloop);
                // the rate the loop rounding left
                if (s->loops && s->len > s->loopStart && rl > rloop) to = (double)s->rate * (rl - rloop) / (s->len - s->loopStart);
            }
            if (!rs) continue;
            int pk = 1;
            for (int k = 0; k < rl; k++) if (abs(rs[k]) > pk) pk = abs(rs[k]);
            int8_t *d8 = malloc((size_t)rl + 1);
            for (int k = 0; k < rl; k++) {
                int q = (int)lround(rs[k] * 127.0 / pk);
                d8[k] = (int8_t)(q > 127 ? 127 : q < -128 ? -128 : q);
            }
            free(rs);
            // the peak the expression assumed was the original's; the 8-bit sample is at full scale
            CustomSample cs;
            memset(&cs, 0, sizeof cs);
            snprintf(cs.name, sizeof cs.name, "%s", s->name);
            cs.data = d8;
            cs.size = (uint32_t)rl;
            cs.loopStart = (uint32_t)rloop;
            cs.loops = s->loops;
            cs.freq = (uint32_t)llround(to * 1024.0 * pow(2.0, (60.0 - s->rootKey) / 12.0));
            int ref = custom_list_add(&out->custom, &out->ncustom, &cs);
            free(d8);
            if (ref < 0) continue;
            v.type = 0;
            v.ref = (int16_t)ref;
        } else if (s->kind == RIPS_SQUARE) {
            v.type = (s->param >> 4) == 2 ? 2 : 1;      // Game Boy channel 2 -> Square 2, else Square 1
            v.ref = (int16_t)(s->param & 3);
            v.attack = 0; v.decay = 0; v.sustain = 15; v.release = 0;
        } else if (s->kind == RIPS_WAVE) {
            CustomSample cs;
            memset(&cs, 0, sizeof cs);
            snprintf(cs.name, sizeof cs.name, "%s", s->name);
            uint8_t w16[16];
            for (int k = 0; k < 16; k++) w16[k] = (uint8_t)((s->wave[k * 2] & 15) << 4 | (s->wave[k * 2 + 1] & 15));
            cs.data = (int8_t *)w16;
            cs.isWave = 1;
            int ref = custom_list_add(&out->custom, &out->ncustom, &cs);
            v.type = 3;
            v.ref = (int16_t)(ref < 0 ? 0 : ref);
            v.attack = 0; v.decay = 0; v.sustain = 15; v.release = 0;
        } else {
            v.type = 4;
            v.ref = (int16_t)(s->param & 1);
            v.attack = 0; v.decay = 0; v.sustain = 15; v.release = 0;
        }
        out->orig[si] = v;
    }

    // the MIDI: track 0 tempo, then a track per chip voice that plays
    int nTracks = 1;
    MTrk *tr = calloc((size_t)nv + 1, sizeof(MTrk));
    {
        uint8_t tempo[3] = {0x06, 0x1A, 0x80};   // 400000 us a beat = 150 BPM
        mmeta(&tr[0], 0, 0x51, tempo, 3);
        const char *nm = r->title;
        mmeta(&tr[0], 0, 0x03, nm, (int)strlen(nm));
        if (out->hasLoop) {
            mmeta(&tr[0], out->loopStart, 0x06, "[", 1);
            mmeta(&tr[0], out->loopEnd, 0x06, "]", 1);
        }
    }
    int chanOf = 0;
    for (int v = 0; v < nv; v++) {
        int first = -1;
        float maxDev = 0;
        for (int i = 0; i < nn; i++) {
            if (notes[i].voice != v) continue;
            if (first < 0) first = i;
            for (int f = notes[i].f0; f < notes[i].f1; f++) {
                const RipVF *x = &r->vf[(size_t)f * nv + v];
                if (x->sample != notes[i].sample || x->rate <= 0) continue;
                float d = fabsf(semis_of(r, x) - notes[i].key);
                if (d > maxDev) maxDev = d;
            }
        }
        if (first < 0) continue;
        out->voicesUsed++;
        MTrk *t = &tr[nTracks++];
        int ch = chanOf++ % 15;
        if (ch >= 9) ch++;                       // channel 10 is General MIDI's drums
        char name[32];
        snprintf(name, sizeof name, "Voice %d", v + 1);
        mmeta(t, 0, 0x03, name, (int)strlen(name));
        int range = (int)ceilf(maxDev + 0.02f);
        if (range < 2) range = 2;
        if (range > 24) range = 24;
        mev(t, 0, 0xB0 | ch, 101, 0);
        mev(t, 0, 0xB0 | ch, 100, 0);
        mev(t, 0, 0xB0 | ch, 6, range);
        mev(t, 0, 0xB0 | ch, 38, 0);
        mev(t, 0, 0xB0 | ch, 7, 127);
        int prog = -1, lastExpr = -1, lastPan = -1, lastBend = -1;
        for (int i = first; i < nn; i++) {
            Note *n = &notes[i];
            if (n->voice != v) continue;
            int slot = n->sample < ns ? slotOf[n->sample] : 127;
            if (slot < 0) slot = 127;
            if (slot != prog) { mev(t, (uint32_t)n->f0, 0xC0 | ch, slot, 0); prog = slot; }
            float velA = n->vel / 127.0f * aref / gain;
            for (int f = n->f0; f < n->f1; f++) {
                const RipVF *x = &r->vf[(size_t)f * nv + v];
                float a = x->sample == n->sample ? amp_of(r, x) : 0;
                int expr = (int)lroundf(127.0f * a / velA);
                if (expr > 127) expr = 127;
                if (expr < 0) expr = 0;
                int pan = 64;
                if (x->l + x->r > 0) pan = (int)lroundf(64.0f + 63.0f * (x->r - x->l) / (x->r + x->l));
                if (pan < 0) pan = 0;
                if (pan > 127) pan = 127;
                int bend = lastBend < 0 ? 8192 : lastBend;
                if (x->sample == n->sample && x->rate > 0) {
                    float dev = semis_of(r, x) - n->key;
                    bend = 8192 + (int)lroundf(dev / range * 8192.0f);
                    if (bend < 0) bend = 0;
                    if (bend > 16383) bend = 16383;
                }
                uint32_t tick = (uint32_t)f;
                if (expr != lastExpr) { mev(t, tick, 0xB0 | ch, 11, expr); lastExpr = expr; }
                if (abs(pan - (lastPan < 0 ? -99 : lastPan)) >= 1) { mev(t, tick, 0xB0 | ch, 10, pan); lastPan = pan; }
                // 7 bits of bend is what the game keeps; only send a change it will hear
                if (lastBend < 0 || (bend >> 7) != (lastBend >> 7)) { mev(t, tick, 0xE0 | ch, bend & 0x7F, bend >> 7); lastBend = bend; }
                if (f == n->f0) mev(t, tick, 0x90 | ch, n->key, n->vel);
            }
            mev(t, (uint32_t)n->f1, 0x80 | ch, n->key, 0);
        }
    }
    free(notes);

    // the file
    MBuf mf = {0};
    mput(&mf, "MThd", 4);
    uint8_t hd[10] = {0, 0, 0, 6, 0, 1, (uint8_t)(nTracks >> 8), (uint8_t)nTracks, 0, 24};
    mput(&mf, hd, 10);
    for (int i = 0; i < nTracks; i++) {
        MTrk *t = &tr[i];
        mvar(&t->b, 0);
        m8(&t->b, 0xFF); m8(&t->b, 0x2F); m8(&t->b, 0);
        mput(&mf, "MTrk", 4);
        uint8_t ln[4] = {(uint8_t)(t->b.n >> 24), (uint8_t)(t->b.n >> 16), (uint8_t)(t->b.n >> 8), (uint8_t)t->b.n};
        mput(&mf, ln, 4);
        mput(&mf, t->b.b, t->b.n);
        free(t->b.b);
    }
    free(tr);
    out->mid = mf.b;
    out->midLen = mf.n;
    // reverb: a light one when the original sends most of its voices to its reverb
    long rv = 0, all = 0;
    for (size_t k = 0; k < (size_t)nf * nv; k++) if (r->vf[k].sample >= 0 && r->vf[k].l + r->vf[k].r > SILENT) { all++; if (r->vf[k].flags & RIPV_REVERB) rv++; }
    if (all && rv * 2 > all) out->reverb = 50;
    return 1;
}
