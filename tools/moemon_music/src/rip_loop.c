// Moemon Music Studio: a rip's loop, found in the original sound.
//
// An emulated song loops by playing the same thing again: after the loop start, the sound one period later
// is the same sound (the same notes, at the same moments; only reverb tails and sample phases differ a
// little). So the loop is read from the recording itself: each game frame's spectrum (24 bands), then the
// period at which the latest part of the song repeats best (any notes-based guesses are tried too), the
// earliest frame from which the song keeps repeating at that period, and the period to the sample by
// matching the waveform one period apart (a loop a frame too long or short is heard every time it loops).
// A song counts as looping only when it repeats clearly: well under half the spectral distance of the
// song compared with itself at other delays, over most of a period.
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rip.h"
#include "rip_loop.h"
#include "engine.h"

#define NB 24                // spectrum bands
#define FFTN 1024
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
                float t = cr * wr - ci * wi; ci = cr * wi + ci * wr; cr = t;
            }
        }
    }
}

// each frame's log band energies (n frames of NB)
static float *features(const Rip *r, double spf, int n) {
    float *S = calloc((size_t)n * NB, sizeof(float));
    float re[FFTN], im[FFTN], win[FFTN];
    if (!S) return NULL;
    for (int i = 0; i < FFTN; i++) win[i] = 0.5f - 0.5f * (float)cos(2 * M_PI * i / (FFTN - 1));
    int edges[NB + 1];
    double lo = 60, hi = r->rate / 2.0 * 0.9;
    if (hi > 16000) hi = 16000;
    for (int b = 0; b <= NB; b++) {
        double fq = lo * pow(hi / lo, (double)b / NB);
        edges[b] = (int)(fq * FFTN / r->rate);
        if (b && edges[b] <= edges[b - 1]) edges[b] = edges[b - 1] + 1;
        if (edges[b] > FFTN / 2) edges[b] = FFTN / 2;
    }
    for (int f = 0; f < n; f++) {
        long c = (long)(f * spf);
        for (int i = 0; i < FFTN; i++) {
            long k = c + i - FFTN / 2;
            float v = (k >= 0 && k < r->pcmFrames) ? (r->pcm[k * 2] + r->pcm[k * 2 + 1]) * 0.5f / 32768.0f : 0;
            re[i] = v * win[i];
            im[i] = 0;
        }
        fft(re, im, FFTN);
        for (int b = 0; b < NB; b++) {
            double e = 0;
            for (int k = edges[b]; k < edges[b + 1]; k++) e += re[k] * re[k] + im[k] * im[k];
            S[f * NB + b] = (float)log1p(e * 1000.0);
        }
    }
    return S;
}

static float dist(const float *S, int a, int b) {
    const float *x = S + a * NB, *y = S + b * NB;
    float d = 0;
    for (int i = 0; i < NB; i++) d += fabsf(x[i] - y[i]);
    return d / NB;
}

static int cmp_f(const void *a, const void *b) {
    float x = *(const float *)a, y = *(const float *)b;
    return (x > y) - (x < y);
}

// how well the latest part of the song (up to 30 s with a repeat) matches itself L frames later
static float lag_distance(const float *S, int n, int L, int step) {
    int W = L < 1800 ? L : 1800;
    int t0 = n - L - W;
    if (t0 < 0) t0 = 0;
    float sum = 0;
    int cnt = 0;
    for (int t = t0; t + L < n; t += step) { sum += dist(S, t, t + L); cnt++; }
    return cnt ? sum / cnt : 1e9f;
}

// the period to the sample: the waveform one period apart, matched at a few places in the repeat
static double refine_period(const Rip *r, double spf, int startF, int endF, int L) {
    int nPts = 6, win = 4096, reach = (int)(spf * 1.5);
    double sumLag = 0;
    int got = 0;
    for (int k = 0; k < nPts; k++) {
        long t = (long)((startF + (endF - L - startF) * (k + 0.5) / nPts) * spf);
        long base = (long)(L * spf);
        if (t < 0 || t + base + reach + win >= r->pcmFrames) continue;
        double best = -1e30;
        int bestD = 0;
        for (int d = -reach; d <= reach; d++) {
            double c = 0, ea = 0, eb = 0;
            for (int i = 0; i < win; i += 2) {
                double a = r->pcm[(t + i) * 2] + r->pcm[(t + i) * 2 + 1];
                double b = r->pcm[(t + base + d + i) * 2] + r->pcm[(t + base + d + i) * 2 + 1];
                c += a * b; ea += a * a; eb += b * b;
            }
            if (ea < 1e3 || eb < 1e3) { best = -1e30; break; }
            c /= sqrt(ea * eb);
            if (c > best) { best = c; bestD = d; }
        }
        if (best > 0.5) { sumLag += base + bestD; got++; }
    }
    return got ? sumLag / got / spf : L;
}

int rip_loop_find(const Rip *r, const int *candidates, int ncand, int *outStart, double *outPeriod, int debug) {
    if (!r->pcm || r->pcmFrames <= 0 || r->rate <= 0) return 0;
    double fps = (double)ENG_RATE / ENG_FRAME_SAMPLES, spf = r->rate / fps;
    // the song up to its tagged length (the fade after it is no loop)
    double secs = r->length > 0 ? r->length : r->pcmFrames / (double)r->rate;
    int n = (int)(secs * fps);
    if (n > (int)(r->pcmFrames / spf)) n = (int)(r->pcmFrames / spf);
    int minL = (int)(8 * fps);
    if (n < 2 * minL) return 0;
    float *S = features(r, spf, n);
    if (!S) return 0;

    // the song against itself at delays in general: what "different" looks like
    int maxL = n - (int)(3 * fps);
    float *dl = malloc(sizeof(float) * (size_t)(maxL + 1));
    if (!dl) { free(S); return 0; }
    for (int L = 0; L <= maxL; L++) dl[L] = 1e9f;
    for (int L = minL; L <= maxL; L += 2) dl[L] = lag_distance(S, n, L, 4);
    float *tmp = malloc(sizeof(float) * (size_t)(maxL + 1));
    int nt = 0;
    for (int L = minL; L <= maxL; L += 2) tmp[nt++] = dl[L];
    qsort(tmp, nt, sizeof(float), cmp_f);
    float typical = nt ? tmp[nt / 2] : 1;
    free(tmp);

    // candidates: the clearest dips of the delay curve, and the notes' guesses
    int cand[48], nc = 0;
    for (int i = 0; i < ncand && nc < 16; i++) if (candidates[i] >= minL && candidates[i] <= maxL) cand[nc++] = candidates[i];
    for (int pass = 0; pass < 12 && nc < 48; pass++) {
        int bl = -1;
        for (int L = minL; L <= maxL; L += 2) {
            if (dl[L] > typical * 0.75f) continue;
            int near = 0;
            for (int c = 0; c < nc; c++) if (abs(cand[c] - L) < (int)fps) near = 1;
            if (!near && (bl < 0 || dl[L] < dl[bl])) bl = L;
        }
        if (bl < 0) break;
        cand[nc++] = bl;
    }

    int found = 0, bestStart = 0, bestL = 0;
    float bestScore = 0;
    for (int c = 0; c < nc; c++) {
        // the delay to the frame
        int L = cand[c], bL = L;
        float bd = 1e9f;
        for (int x = L - 3; x <= L + 3; x++) {
            if (x < minL || x > maxL) continue;
            float d = lag_distance(S, n, x, 1);
            if (d < bd) { bd = d; bL = x; }
        }
        L = bL;
        // frame by frame from the start: repeats where the distance is near the repeat's own
        int m = n - L;
        float *d = malloc(sizeof(float) * (size_t)m);
        if (!d) continue;
        for (int t = 0; t < m; t++) d[t] = dist(S, t, t + L);
        float *sorted = malloc(sizeof(float) * (size_t)m);
        memcpy(sorted, d, sizeof(float) * (size_t)m);
        qsort(sorted, m, sizeof(float), cmp_f);
        float rep = sorted[m / 4];                 // a repeating frame's distance (the quieter quarter)
        free(sorted);
        float thr = rep + (typical - rep) * 0.35f;
        // the earliest frame from which (nearly) every frame repeats, read from the end back
        int start = m, bad = 0, run = 0;
        for (int t = m - 1; t >= 0; t--) {
            int ok = d[t] < thr;
            run++;
            if (!ok) bad++;
            if (ok) start = t;
            // stop once the last 3 seconds are mostly not repeating
            if (!ok) {
                int w = (int)(3 * fps), miss = 0, len = 0;
                for (int k = t; k < m && k < t + w; k++, len++) if (d[k] >= thr) miss++;
                if (len >= w / 2 && miss > len / 2) break;
            }
        }
        // the repeat has to cover most of a period, and be clearly closer than the song in general
        float sum = 0;
        int cnt = 0;
        for (int t = start; t < m; t++) { sum += d[t]; cnt++; }
        float mean = cnt ? sum / cnt : 1e9f;
        float score = typical / (mean + 1e-6f);
        if (debug)
            fprintf(stderr, "loop: try %.2f s: repeats from %.2f s over %.2f s (%.0f%% of a period), distance %.3f (in general %.3f)\n",
                    L / fps, start / fps, cnt / fps, 100.0 * cnt / L, mean, typical);
        free(d);
        if (cnt < L * 0.85 || score < 2.0f) continue;
        // shorter periods that are sub-loops of a longer one fail the coverage; among the rest, the clearest
        // (a period twice as long repeats as well: keep the shorter one when it's about as clear)
        if (!found || score > bestScore * 1.15f || (score > bestScore * 0.87f && L < bestL * 0.9f)) {
            found = 1; bestScore = score; bestL = L; bestStart = start;
        }
    }
    free(dl);
    free(S);
    if (!found) return 0;
    *outStart = bestStart;
    *outPeriod = refine_period(r, spf, bestStart, n, bestL);
    if (debug) fprintf(stderr, "loop: from %.2f s, period %.4f frames (%.3f s)\n", bestStart / fps, *outPeriod, *outPeriod / fps);
    return 1;
}
