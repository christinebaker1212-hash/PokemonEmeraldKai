// Moemon Music Studio: turning what a sound chip did into a MIDI and instruments.
//
// Notes go on tracks by instrument: a chip hands its voices to whatever instrument needs one next, so a
// track per chip voice would change instrument all the time. Each instrument gets as many tracks as it
// has notes at once, and every track keeps one instrument (each note still has its own bends and volume). A tick is one game frame (24 ticks
// a beat at 150 BPM is what MP2K plays as one tick a frame), so the song keeps the original's timing to
// the frame. Volume is the voice's own level each frame (envelope included) as expression (CC11), with
// the note's loudest moment as its velocity, and the instruments get a flat envelope: the original's
// envelope is in the expression. Pitch is the key closest to the note plus bends for the rest
// (vibrato, slides, detuning), with a bend range per note just wide enough for it (the game keeps 7 bits
// of bend, so a narrow range keeps a note in tune). The N64's sequencer ticks on its own audio frames,
// which puts notes up to a frame off the beat; when such a song has a steady grid, note starts move onto
// it (steady_rhythm).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "rip_song.h"
#include "inst.h"
#include "engine.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// The game's mixer steps through a sample as if it put out 60 x 701 = 42060 samples a second; it puts
// out 42048 (ENG_RATE). Every sample plays 0.03% slow (half a cent): a song's own samples get their
// rate raised by that, so they play at the original's pitch and speed.
#define MIX_FIX ((60.0 * ENG_FRAME_SAMPLES) / (double)ENG_RATE)

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
    int voice, f0, f1, sample;     // f0 / f1: the frames the chip played it (its data is read from there)
    int key, vel;
    float peak;
    int shift;                     // frames it moves to sit on the song's beat grid (steady_rhythm)
    int track;                     // the MIDI track it goes on
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

// a voice's pitch in a frame (semitones), -1000 when it isn't playing `sample` at a known pitch
static float semis_at(const Rip *r, int nv, int v, int f, int sample) {
    if (f < 0 || f >= r->nframes) return -1000;
    const RipVF *x = &r->vf[(size_t)f * nv + v];
    if (x->sample != sample || x->rate <= 0) return -1000;
    return semis_of(r, x);
}

// a note's pitch in a frame, a one-frame blip (away by a semitone or more and back) taken as the note's
// pitch around it; -1000 = none
static float note_semis(const Rip *r, int nv, int v, int f, int sample, int f0, int f1) {
    float p = semis_at(r, nv, v, f, sample);
    if (p < -999) return p;
    float a = f > f0 ? semis_at(r, nv, v, f - 1, sample) : -1000, b = f + 1 < f1 ? semis_at(r, nv, v, f + 1, sample) : -1000;
    if (a > -999 && b > -999 && fabsf(p - a) >= 0.75f && fabsf(p - b) >= 0.75f && fabsf(a - b) < 0.3f) return (a + b) * 0.5f;
    return p;
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
    if (maxP < 180) { free(ev); return 0; }
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
            // and a real loop: at least 3 seconds with a dozen notes in it (a jingle's last bars can look
            // like a short repeat)
            if (p < 180) continue;
            int inside = 0;
            for (int i = 0; i < nn; i++) if (ev[i].f >= start && ev[i].f < start + p) inside++;
            if (inside < 12) continue;
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

// ------------------------------------------------------------------ steady rhythm
static double grid_conc(const int *on, int n, double u) {
    double c = 0, sn = 0;
    for (int i = 0; i < n; i++) { double a = 2 * M_PI * on[i] / u; c += cos(a); sn += sin(a); }
    return sqrt(c * c + sn * sn) / n;
}

static int cmp_int(const void *a, const void *b) { int x = *(const int *)a, y = *(const int *)b; return (x > y) - (x < y); }

// The song's finest steady grid (a sixteenth or so: at least 4.5 frames), its phase followed through the
// song; every note start within 1.25 frames of it moves onto it (rounded to a frame). Notes between its
// points go to finer levels the song really uses, the coarsest that fits: triplets of the beat, then
// halves down to a sixty-fourth note (each with a closer tolerance). Songs without a grid
// most notes sit on (rubato, tempo changes) stay as they are. Returns the notes moved.
static int steady_rhythm(Note *notes, int nn) {
    int *on = malloc(sizeof(int) * (size_t)(nn > 0 ? nn : 1));
    if (!on) return 0;
    for (int i = 0; i < nn; i++) on[i] = notes[i].f0;
    qsort(on, nn, sizeof(int), cmp_int);
    int n = 0;
    for (int i = 0; i < nn; i++) if (!n || on[i] != on[n - 1]) on[n++] = on[i];
    if (n < 32) { free(on); return 0; }
    // the grid: the strongest period, then the finest one nearly as strong
    double bestC = 0, bestU = 0;
    int nu = 0;
    // searched on the first 1500 onsets, in steps fine enough that the grid can't slip a quarter of itself
    // over them; then refined on all of them
    int ns0 = n < 1500 ? n : 1500;
    double span = on[ns0 - 1] - on[0] + 1;
    static double cu[200000];
    for (double u = 4.5; u <= 24.0 && nu < 200000; u += u * u / (4 * span), nu++) {
        cu[nu] = u;
    }
    double *cc = malloc(sizeof(double) * (size_t)(nu > 0 ? nu : 1));
    if (!cc) { free(on); return 0; }
    for (int k = 0; k < nu; k++) {
        cc[k] = grid_conc(on, ns0, cu[k]);
        if (cc[k] > bestC) { bestC = cc[k]; bestU = cu[k]; }
    }
    for (int k = 1; k + 1 < nu; k++)
        if (cc[k] >= 0.85 * bestC && cc[k] >= cc[k - 1] && cc[k] >= cc[k + 1]) { bestU = cu[k]; break; }
    free(cc);
    double u = bestU, c0 = 0;
    double du = bestU * bestU / (4 * span), spanAll = on[n - 1] - on[0] + 1;
    for (double x = bestU - du; x <= bestU + du; x += bestU * bestU / (40 * spanAll)) {
        double c = grid_conc(on, n, x);
        if (c > c0) { c0 = c; u = x; }
    }
    // the phase around each onset (a 10-second window), and how far the onset is from the grid
    // where each onset is, in grid steps from the grid's local phase (followed through a 10-second window)
    double *pos = malloc(sizeof(double) * (size_t)n), *res = malloc(sizeof(double) * (size_t)n);
    if (!pos || !res) { free(pos); free(res); free(on); return 0; }
    int lo = 0, hi = 0, near = 0;
    double sc = 0, ss = 0, tolU = u * 0.3 < 1.25 ? u * 0.3 : 1.25;
    for (int i = 0; i < n; i++) {
        while (hi < n && on[hi] <= on[i] + 300) { double a = 2 * M_PI * on[hi] / u; sc += cos(a); ss += sin(a); hi++; }
        while (on[lo] < on[i] - 300) { double a = 2 * M_PI * on[lo] / u; sc -= cos(a); ss -= sin(a); lo++; }
        pos[i] = on[i] / u - atan2(ss, sc) / (2 * M_PI);
        res[i] = 1e9;
        double r = (pos[i] - floor(pos[i] + 0.5)) * u;
        if (fabs(r) <= tolU) { res[i] = r; near++; }
    }
    int moved = 0;
    if (near >= n * 0.55) {
        // the beat: the grid doubled until it's a quarter note's length at 100-200 BPM (18-36 frames), starting
        // where onsets crowd most
        int m = 0;
        while (u * (1 << m) < 18 && m < 4) m++;
        double Q = u * (1 << m);
        int per = 1 << m, cnt[16] = {0}, bo = 0;
        for (int i = 0; i < n; i++) if (res[i] < 1e8) cnt[(((long)floor(pos[i] + 0.5)) % per + per) % per]++;
        for (int k = 1; k < per; k++) if (cnt[k] > cnt[bo]) bo = k;
        // finer levels, the coarser first: triplets of the beat (to a sixteenth triplet), halves of the grid
        // (to a sixty-fourth note). Each only if far more onsets sit on it than would by chance.
        typedef struct { double g; int trip; double tol; } Level;
        Level lv[16];
        int nl = 0;
        double steps[16];
        int nst = 0;
        for (int t = 3; t <= 12; t *= 2) steps[nst++] = -t;                // triplets: Q / t (to a sixteenth triplet)
        for (double g = u / 2; g >= Q / 16 * 0.99; g /= 2) steps[nst++] = g;
        // in size order (biggest first)
        for (int i = 0; i < nst; i++)
            for (int j = i + 1; j < nst; j++) {
                double gi = steps[i] < 0 ? Q / -steps[i] : steps[i], gj = steps[j] < 0 ? Q / -steps[j] : steps[j];
                if (gj > gi) { double tmp = steps[i]; steps[i] = steps[j]; steps[j] = tmp; }
            }
        for (int i = 0; i < nst && nl < 16; i++) {
            Level L;
            L.trip = steps[i] < 0;
            L.g = L.trip ? Q / -steps[i] : steps[i];
            L.tol = L.g * (L.trip ? 0.25 : 0.3);
            if (L.tol > 1.25) L.tol = 1.25;
            lv[nl++] = L;
        }
        int used[16] = {0};
        if (getenv("MMS_RHYTHM_DEBUG")) {
            int h[40] = {0};
            for (int i = 0; i < n; i++) if (res[i] > 1e8) {
                double r = (pos[i] - floor(pos[i] + 0.5)) * u;    // frames from the nearest grid point
                int b = (int)floor((r + u / 2) / u * 40);
                if (b >= 0 && b < 40) h[b]++;
            }
            fprintf(stderr, "rhythm: leftover onsets across one grid step:");
            for (int b = 0; b < 40; b++) fprintf(stderr, " %d", h[b]);
            fprintf(stderr, "\n");
        }
        for (int l = 0; l < nl; l++) {
            // the finest level (a sixty-fourth) takes every onset left, to its nearest point; the others only
            // when onsets really crowd their points (more in the inner half of the tolerance than the outer)
            int last = l == nl - 1 && !lv[l].trip;
            int inner = 0, outer = 0;
            for (int i = 0; i < n; i++) {
                if (res[i] < 1e8) continue;
                double x = lv[l].trip ? (pos[i] - bo) * u / lv[l].g : pos[i] * u / lv[l].g;
                double r = fabs(x - floor(x + 0.5)) * lv[l].g;
                if (r <= lv[l].tol * 0.5) inner++;
                else if (r <= lv[l].tol) outer++;
            }
            int hits = inner + outer;
            if (last) lv[l].tol = lv[l].g * 0.5 + 1e-6;
            else if (hits < 8 || inner < 1.3 * outer) continue;
            used[l] = hits;
            for (int i = 0; i < n; i++) {
                if (res[i] < 1e8) continue;
                double x = lv[l].trip ? (pos[i] - bo) * u / lv[l].g : pos[i] * u / lv[l].g;
                double r = (x - floor(x + 0.5)) * lv[l].g;
                if (fabs(r) <= lv[l].tol) res[i] = r;
            }
        }
        for (int k = 0; k < nn; k++) {
            int a = 0, b = n - 1;
            while (a < b) { int mm = (a + b) / 2; if (on[mm] < notes[k].f0) a = mm + 1; else b = mm; }
            if (res[a] > 1e8) continue;
            notes[k].shift = (int)lround(notes[k].f0 - res[a]) - notes[k].f0;
            if (notes[k].f0 + notes[k].shift < 0) notes[k].shift = -notes[k].f0;
            if (notes[k].shift) moved++;
        }
        if (getenv("MMS_RHYTHM_DEBUG")) {
            fprintf(stderr, "rhythm: beat %.3f frames (%.1f BPM), finer levels:", Q, 3600 / Q);
            for (int l = 0; l < nl; l++)
                fprintf(stderr, " %s%.0f=%d", lv[l].trip ? "3x" : "1/", lv[l].trip ? 4 * Q / lv[l].g / 3 * 2 : 4 * Q / lv[l].g, used[l]);
            fprintf(stderr, "\n");
        }
    }
    if (getenv("MMS_RHYTHM_DEBUG")) {
        int on1 = 0;
        for (int i = 0; i < n; i++) on1 += res[i] < 1e8;
        fprintf(stderr, "rhythm: grid %.4f frames, strength %.2f, %d of %d onsets on it, %d on a grid in all, %d notes moved\n",
                u, c0, near, n, on1, moved);
    }
    free(pos);
    free(res);
    free(on);
    return moved;
}

static int note_cmp(const void *a, const void *b) {
    const Note *x = a, *y = b;
    int fx = x->f0 + x->shift, fy = y->f0 + y->shift;
    if (fx != fy) return fx - fy;
    return x->voice - y->voice;
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
    free(s->keyDur);
    s->keyDur = NULL;
    free(s->mid);
    s->mid = NULL;
    s->midLen = 0;
}

// ------------------------------------------------------------------ streamed music (GameCube)
// The recording itself: per stereo side an intro sample and a loop sample, played by one note each.
// The loop is stretched by a hair (well under 0.1%) so it lasts a whole number of game frames: then the
// song can loop forever, jumping back on a frame like any other song, without the jump drifting.
static int add_stream_sample(RipSong *out, const char *name, const int16_t *pcm, long n, int stride, int rate,
                             double speed, int loops, float *peakOut) {
    int16_t *mono = malloc(sizeof(int16_t) * (size_t)(n > 0 ? n : 1));
    if (!mono) return -1;
    for (long i = 0; i < n; i++) mono[i] = pcm[i * stride];
    double to = RIP_GBA_RATE;
    if (to > rate) to = rate;
    int16_t *rs = NULL;
    int rloop = 0;
    int rl = rip_resample(mono, (int)n, rate, 0, 0, to, &rs, &rloop);
    free(mono);
    if (!rs || rl <= 0) { free(rs); return -1; }
    int pk = 1;
    for (int k = 0; k < rl; k++) if (abs(rs[k]) > pk) pk = abs(rs[k]);
    int8_t *d8 = malloc((size_t)rl + 1);
    if (!d8) { free(rs); return -1; }
    for (int k = 0; k < rl; k++) {
        int q = (int)lround(rs[k] * 127.0 / pk);
        d8[k] = (int8_t)(q > 127 ? 127 : q < -128 ? -128 : q);
    }
    free(rs);
    CustomSample cs;
    memset(&cs, 0, sizeof cs);
    snprintf(cs.name, sizeof cs.name, "%s", name);
    cs.data = d8;
    cs.size = (uint32_t)rl;
    cs.loops = loops;
    cs.loopStart = 0;
    cs.freq = (uint32_t)llround(to * speed * 1024.0 * MIX_FIX);   // key 60 plays it at its own pitch (sped up by `speed`)
    int ref = custom_list_add(&out->custom, &out->ncustom, &cs);
    free(d8);
    *peakOut = pk / 32768.0f;
    return ref;
}

static int stream_build(Rip *r, RipSong *out, float gain, char *err, int errlen) {
    double fps = (double)ENG_RATE / ENG_FRAME_SAMPLES;
    long n = r->streamFrames, ls = r->streamLoopStart, le = r->streamLoopEnd;
    int loops = ls >= 0 && le > ls;
    double speed = 1;
    long introFrames = 0, loopFrames = 0, padFrames = 0;
    if (loops) {
        double body = (double)(le - ls) / r->streamRate * fps;          // frames, not whole
        loopFrames = lround(body);
        if (loopFrames < 1) loopFrames = 1;
        speed = body / loopFrames;
        double intro = (double)ls / r->streamRate * fps / speed;
        introFrames = (long)ceil(intro - 1e-9);
        padFrames = 0;
        (void)intro;
    } else {
        introFrames = (long)ceil((double)n / r->streamRate * fps);
    }
    int sides = r->streamStereo ? 2 : 1;
    int introRef[2] = {-1, -1}, loopRef[2] = {-1, -1};
    float peak = 0, pk;
    for (int c = 0; c < sides; c++) {
        char nm[40];
        long introLen = loops ? ls : n;
        if (introLen > 0) {
            // the intro ends exactly where the loop starts: a little silence first makes up the part of a frame
            double introExact = (double)introLen / r->streamRate * fps / speed;
            long pad = (long)llround((introFrames - introExact) / fps * r->streamRate * speed);
            if (pad < 0) pad = 0;
            int16_t *tmp = calloc((size_t)(introLen + pad) * 2, sizeof(int16_t));
            if (!tmp) { snprintf(err, errlen, "out of memory"); return 0; }
            for (long i = 0; i < introLen; i++) tmp[(pad + i) * 2] = r->streamPcm[i * 2 + c];
            snprintf(nm, sizeof nm, "%s_%s", out->prefix, loops ? (c ? "intro_r" : "intro_l") : (c ? "r" : "l"));
            if (sides == 1) snprintf(nm, sizeof nm, "%s_%s", out->prefix, loops ? "intro" : "song");
            introRef[c] = add_stream_sample(out, nm, tmp, introLen + pad, 2, r->streamRate, speed, 0, &pk);
            free(tmp);
            if (pk > peak) peak = pk;
        }
        if (loops) {
            snprintf(nm, sizeof nm, "%s_%s", out->prefix, sides == 1 ? "loop" : c ? "loop_r" : "loop_l");
            loopRef[c] = add_stream_sample(out, nm, r->streamPcm + ls * 2 + c, le - ls, 2, r->streamRate, speed, 1, &pk);
            if (pk > peak) peak = pk;
        }
    }
    (void)padFrames;
    // voices: 0/1 intro (left/right), 2/3 loop
    for (int c = 0; c < sides; c++) {
        int refs[2] = {introRef[c], loopRef[c]};
        for (int k = 0; k < 2; k++) {
            if (refs[k] < 0) continue;
            int slot = k * 2 + c;
            SfVoice v;
            memset(&v, 0, sizeof v);
            v.type = 0; v.key = 60; v.sub = v.ks = -1; v.ref = (int16_t)refs[k];
            v.attack = 255; v.decay = 0; v.sustain = 255; v.release = 0;
            if (sides == 2) v.panSweep = (uint8_t)(0x80 | (c ? 127 : 0));   // the side it belongs to
            out->orig[slot] = v;
            out->slotSample[slot] = 0;
            out->nslots = slot + 1;
        }
    }
    // the MIDI: tempo, loop markers, a track per side
    int vel = (int)lroundf(127.0f * gain);
    if (vel < 1) vel = 1;
    if (vel > 127) vel = 127;
    int nTracks = 1 + sides;
    MTrk *tr = calloc((size_t)nTracks, sizeof(MTrk));
    uint8_t tempo[3] = {0x06, 0x1A, 0x80};
    mmeta(&tr[0], 0, 0x51, tempo, 3);
    mmeta(&tr[0], 0, 0x03, r->title, (int)strlen(r->title));
    for (int c = 0; c < sides; c++) {
        MTrk *t = &tr[1 + c];
        char nm[16];
        snprintf(nm, sizeof nm, sides == 2 ? (c ? "Right" : "Left") : "Recording");
        mmeta(t, 0, 0x03, nm, (int)strlen(nm));
        mev(t, 0, 0xB0 | c, 7, 127);
        mev(t, 0, 0xB0 | c, 10, sides == 2 ? (c ? 127 : 0) : 64);
        if (introRef[c] >= 0 && introFrames > 0) {
            mev(t, 0, 0xC0 | c, c, 0);
            mev(t, 0, 0x90 | c, 60, vel);
            mev(t, (uint32_t)introFrames, 0x80 | c, 60, 0);
        }
        if (loops && loopRef[c] >= 0) {
            mev(t, (uint32_t)introFrames, 0xC0 | c, 2 + c, 0);
            mev(t, (uint32_t)introFrames, 0x90 | c, 60, vel);
            mev(t, (uint32_t)(introFrames + loopFrames), 0x80 | c, 60, 0);
        }
    }
    out->hasLoop = loops;
    if (loops) { out->loopStart = (uint32_t)introFrames; out->loopEnd = (uint32_t)(introFrames + loopFrames); }
    MBuf mf = {0};
    mput(&mf, "MThd", 4);
    uint8_t hd[10] = {0, 0, 0, 6, 0, 1, 0, (uint8_t)nTracks, 0, 24};
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
    out->voicesUsed = sides;
    (void)peak;
    return 1;
}

int rip_song_build(Rip *r, RipSong *out, float gain, char *err, int errlen) {
    memset(out, 0, sizeof *out);
    if (r->streamPcm) {
        for (int i = 0; i < 128; i++) { out->slotSample[i] = -1; out->gm[i] = -1; out->orig[i] = VOICE_PLACEHOLDER; }
        out->reverb = -1;
        make_prefix(r, out->prefix, sizeof out->prefix);
        return stream_build(r, out, gain, err, errlen);
    }
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

    if (getenv("MMS_ONSETS_DEBUG")) {      // tests: when each note starts (voice, frame)
        FILE *df = fopen(getenv("MMS_ONSETS_DEBUG"), "w");
        for (int i = 0; df && i < nn; i++) fprintf(df, "%d %d\n", notes[i].voice, notes[i].f0);
        if (df) fclose(df);
    }

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
            if (x->sample == n->sample && x->rate > 0) tmp[k++] = note_semis(r, nv, n->voice, f, n->sample, n->f0, n->f1);
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

    // noise channels: how long the notes at each noise setting (key) last, for drum matching
    out->keyDur = malloc(sizeof(float) * 128 * (size_t)(nslots > 0 ? nslots : 1));
    if (out->keyDur) {
        for (int i = 0; i < 128 * (nslots > 0 ? nslots : 1); i++) out->keyDur[i] = -1;
        float *d = malloc(sizeof(float) * (size_t)nn);
        for (int si = 0; si < nslots && d; si++) {
            if (r->samples[out->slotSample[si]].kind != RIPS_NOISE) continue;
            for (int k = 0; k < 128; k++) {
                int c = 0;
                for (int i = 0; i < nn; i++)
                    if (notes[i].sample == out->slotSample[si] && notes[i].key == k) d[c++] = (notes[i].f1 - notes[i].f0) / 60.0f;
                if (!c) continue;
                qsort(d, c, sizeof(float), cmp_float);
                out->keyDur[si * 128 + k] = d[c / 2];
            }
        }
        free(d);
    }

    // on the beat, then in time order (as they will be played)
    // Only the N64's: its music runs on audio frames of its own rate, placed on game frames afterwards (up to
    // a frame off). Game Boy, DS and PlayStation drivers tick on the console's frames, so their notes already
    // sit where the original plays them (their own uneven tempo rounding included, which is the song's).
    if (r->kind == RIP_USF && !getenv("MMS_NO_RHYTHM")) steady_rhythm(notes, nn);
    qsort(notes, nn, sizeof(Note), note_cmp);

    // the loop (on the moved notes)
    int ls = 0, le = 0;
    {
        Note *moved = malloc(sizeof(Note) * (size_t)nn);
        if (moved) {
            for (int i = 0; i < nn; i++) { moved[i] = notes[i]; moved[i].f0 += notes[i].shift; moved[i].f1 += notes[i].shift; }
            out->hasLoop = find_loop(moved, nn, nf + 2, &ls, &le);
            free(moved);
        }
    }
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
            cs.freq = (uint32_t)llround(to * 1024.0 * pow(2.0, (60.0 - s->rootKey) / 12.0) * MIX_FIX);
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
    // tracks: each instrument gets lanes, a note going on the first of its instrument's lanes that's free
    int ntr = 0;
    {
        int *laneEnd = malloc(sizeof(int) * (size_t)nn), *laneSlot = malloc(sizeof(int) * (size_t)nn);
        for (int i = 0; i < nn && laneEnd && laneSlot; i++) {
            int slot = notes[i].sample < ns ? slotOf[notes[i].sample] : 127;
            int f0 = notes[i].f0 + notes[i].shift, t = -1;
            for (int k = 0; k < ntr; k++) if (laneSlot[k] == slot && laneEnd[k] <= f0) { t = k; break; }
            if (t < 0) { t = ntr++; laneSlot[t] = slot; }
            laneEnd[t] = notes[i].f1 + notes[i].shift;
            notes[i].track = t;
        }
        free(laneEnd);
        free(laneSlot);
    }
    if (ntr > nv) { MTrk *nt = realloc(tr, sizeof(MTrk) * (size_t)(ntr + 1)); if (nt) { tr = nt; memset(tr + 1, 0, sizeof(MTrk) * (size_t)ntr); } }
    int chanOf = 0;
    for (int lane = 0; lane < ntr; lane++) {
        int first = -1, nOfSlot = 0;
        for (int i = 0; i < nn; i++) if (notes[i].track == lane) { first = i; break; }
        if (first < 0) continue;
        int slot = notes[first].sample < ns ? slotOf[notes[first].sample] : 127;
        if (slot < 0) slot = 127;
        for (int k = 0; k < lane; k++) {
            for (int i = 0; i < nn; i++) if (notes[i].track == k) { if ((notes[i].sample < ns ? slotOf[notes[i].sample] : 127) == slot) nOfSlot++; break; }
        }
        out->voicesUsed++;
        MTrk *t = &tr[nTracks++];
        int ch = chanOf++ % 15;
        if (ch >= 9) ch++;                       // channel 10 is General MIDI's drums
        char name[40];
        if (nOfSlot) snprintf(name, sizeof name, "Instrument %d (%d)", slot + 1, nOfSlot + 1);
        else snprintf(name, sizeof name, "Instrument %d", slot + 1);
        mmeta(t, 0, 0x03, name, (int)strlen(name));
        mev(t, 0, 0xB0 | ch, 7, 127);
        mev(t, 0, 0xC0 | ch, slot, 0);
        int lastExpr = -1, lastPan = -1, lastBend = -1, lastTune = -1, range = -1;
        for (int i = first; i < nn; i++) {
            Note *n = &notes[i];
            if (n->track != lane) continue;
            int v = n->voice, sh = n->shift;
            // the bend range this note needs (the game's bends go to 63/64 of it)
            float maxDev = 0;
            for (int f = n->f0; f < n->f1; f++) {
                float p = note_semis(r, nv, v, f, n->sample, n->f0, n->f1);
                if (p < -999) continue;
                float d = fabsf(p - n->key);
                if (d > maxDev) maxDev = d;
            }
            if (getenv("MMS_BEND_DEBUG") && maxDev > 3) {
                static int shown;
                if (shown++ < 6) {
                    fprintf(stderr, "lane %d note f%d key %d:", lane, n->f0, n->key);
                    for (int f = n->f0; f < n->f1 && f < n->f0 + 40; f++) {
                        const RipVF *x = &r->vf[(size_t)f * nv + v];
                        fprintf(stderr, " %.1f", x->sample == n->sample && x->rate > 0 ? semis_of(r, x) : -1.0f);
                    }
                    fprintf(stderr, "\n");
                }
            }
            int nr = (int)ceilf(maxDev * 64.0f / 63.0f + 0.02f);   // (fine tune does the rest exactly)
            if (nr < 1) nr = 1;
            if (nr > 48) nr = 48;
            if (nr != range) {
                uint32_t tk = (uint32_t)(n->f0 + sh);
                mev(t, tk, 0xB0 | ch, 101, 0);
                mev(t, tk, 0xB0 | ch, 100, 0);
                mev(t, tk, 0xB0 | ch, 6, nr);
                mev(t, tk, 0xB0 | ch, 38, 0);
                range = nr;
                lastBend = -1;                   // the same bend value means another pitch now
            }
            float velA = n->vel / 127.0f * aref / gain;
            lastBend = lastTune = -1;            // each note sends its pitch at its start (a loop can jump to it)
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
                // the pitch in 64ths of a semitone: the bend's 7 bits (steps of range / 64) get close, the
                // track's fine tune (TUNE, CC 24: 1/64 semitone, +-1 semitone) makes up the rest
                int bend = lastBend < 0 ? 8192 : lastBend, tune = lastTune < 0 ? 64 : lastTune;
                float pf = note_semis(r, nv, v, f, n->sample, n->f0, n->f1);
                if (pf > -999) {
                    int units = (int)lroundf((pf - n->key) * 64.0f);
                    int bv = (int)lroundf((float)units / range);
                    if (bv > 63) bv = 63;
                    if (bv < -64) bv = -64;
                    int tv = units - bv * range;
                    if (tv > 63) tv = 63;
                    if (tv < -64) tv = -64;
                    bend = (bv + 64) << 7;
                    tune = tv + 64;
                }
                uint32_t tick = (uint32_t)(f + sh);
                if (expr != lastExpr) { mev(t, tick, 0xB0 | ch, 11, expr); lastExpr = expr; }
                if (pan != lastPan) { mev(t, tick, 0xB0 | ch, 10, pan); lastPan = pan; }
                if (tune != lastTune) { mev(t, tick, 0xB0 | ch, 24, tune); lastTune = tune; }
                if (bend != lastBend) { mev(t, tick, 0xE0 | ch, bend & 0x7F, bend >> 7); lastBend = bend; }
                if (f == n->f0) mev(t, tick, 0x90 | ch, n->key, n->vel);
            }
            mev(t, (uint32_t)(n->f1 + sh), 0x80 | ch, n->key, 0);
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
