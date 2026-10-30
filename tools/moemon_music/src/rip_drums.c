// Moemon Music Studio: Game Boy drums as the game's drums (see rip_drums.h).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rip_drums.h"
#include "engine.h"
#include "sf.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NFFT 512
#define HOP 256
#define NMEL 40
#define WIN_FRAMES RIP_DRUM_FRAMES
#define WIN_SAMPLES (WIN_FRAMES * ENG_FRAME_SAMPLES)
#define NSPEC ((WIN_SAMPLES - NFFT) / HOP + 1)

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

// mel bands (60 Hz - 16 kHz) of each NFFT bin
static float sFb[NMEL][NFFT / 2 + 1];
static void init_fb(void) {
    static int done;
    if (done) return;
    done = 1;
    double lo = 2595 * log10(1 + 60 / 700.0), hi = 2595 * log10(1 + 16000 / 700.0), pts[NMEL + 2];
    for (int i = 0; i < NMEL + 2; i++) pts[i] = 700 * (pow(10, (lo + (hi - lo) * i / (NMEL + 1)) / 2595) - 1);
    for (int b = 0; b < NMEL; b++)
        for (int k = 0; k <= NFFT / 2; k++) {
            double f = k * (double)ENG_RATE / NFFT, a = pts[b], c = pts[b + 1], d = pts[b + 2];
            double w = f < a || f > d ? 0 : f <= c ? (f - a) / (c - a) : (d - f) / (d - c);
            sFb[b][k] = (float)w;
        }
}

// energy spectrogram (linear) of the first `samples` samples, frames of NFFT every HOP
static int melspec(const float *x, int samples, float out[NSPEC][NMEL]) {
    init_fb();
    static float re[NFFT], im[NFFT];
    int nf = 0;
    for (int s = 0; s + NFFT <= samples && nf < NSPEC; s += HOP, nf++) {
        for (int i = 0; i < NFFT; i++) {
            re[i] = x[s + i] * (0.5f - 0.5f * (float)cos(2 * M_PI * i / (NFFT - 1)));
            im[i] = 0;
        }
        fft(re, im, NFFT);
        for (int b = 0; b < NMEL; b++) {
            double e = 0;
            for (int k = 0; k <= NFFT / 2; k++) if (sFb[b][k] > 0) e += sFb[b][k] * (re[k] * re[k] + im[k] * im[k]);
            out[nf][b] = (float)e;
        }
    }
    return nf;
}

// a hit as the Game Boy makes it, at the engine's level
static void synth_hit(const RipHit *h, float noiseFull, float *out) {
    unsigned lfsr = 0x7FFF;
    double acc = 0;
    float last = 1;
    memset(out, 0, sizeof(float) * WIN_SAMPLES);
    int frames = h->len < WIN_FRAMES ? h->len : WIN_FRAMES;
    for (int f = 0; f < frames; f++) {
        double step = h->hz[f] / ENG_RATE;
        for (int i = 0; i < ENG_FRAME_SAMPLES; i++) {
            acc += step;
            double sum = 0;
            int cnt = 0;
            while (acc >= 1) {
                acc -= 1;
                unsigned b = (lfsr ^ (lfsr >> 1)) & 1;
                lfsr = (lfsr >> 1) | (b << 14);
                if (h->shortMode) lfsr = (lfsr & ~0x40u) | (b << 6);
                last = (~lfsr & 1) ? 1.0f : -1.0f;
                sum += last;
                if (++cnt > 64) { acc = 0; break; }
            }
            out[f * ENG_FRAME_SAMPLES + i] = (cnt ? (float)(sum / cnt) : last) * h->vol[f] * noiseFull;
        }
    }
}

// a drum of the game's kits as the engine plays it (mono), its note let go after `frames`
static void render_drum(const SfVoice *v, int frames, float *out) {
    static Engine e;
    static EInst inst;
    static float buf[ENG_FRAME_SAMPLES * 2];
    eng_init(&e);
    inst_make(v, &inst);
    eng_preview_on(&e, &inst, v->key, 127);
    for (int f = 0; f < WIN_FRAMES; f++) {
        if (f == frames) eng_preview_off(&e);
        eng_frame(&e, buf);
        for (int i = 0; i < ENG_FRAME_SAMPLES; i++) out[f * ENG_FRAME_SAMPLES + i] = (buf[i * 2] + buf[i * 2 + 1]) * 0.5f;
    }
    eng_preview_off(&e);
}

// the game's drum sounds: every sample-playing voice of every drum kit, once each
// what a drum is (from its sample's name in the game's sound set)
enum { ROLE_ANY, ROLE_KICK, ROLE_SNARE, ROLE_CHAT, ROLE_OHAT, ROLE_CRASH, ROLE_TOM, ROLE_OTHER };
static const char *const ROLE_NAMES[] = {"any", "kick", "snare", "closed hi-hat", "open hi-hat", "cymbal", "tom", "other"};

static int role_of_name(const char *n) {
    if (strstr(n, "kick")) return ROLE_KICK;
    if (strstr(n, "snare")) return ROLE_SNARE;
    if (strstr(n, "close_hihat") || strstr(n, "closed_hihat")) return ROLE_CHAT;
    if (strstr(n, "open_hihat") || strstr(n, "hihat")) return ROLE_OHAT;
    if (strstr(n, "crash") || strstr(n, "ride") || strstr(n, "cymbal")) return ROLE_CRASH;
    if (strstr(n, "_tom")) return ROLE_TOM;
    return ROLE_OTHER;
}

typedef struct { SfVoice v; int kitKey, role; } Cand;
static Cand *sCand;
static int sNCand = -1;

static void find_candidates(void) {
    sNCand = 0;
    int kits[SF_NUM_GROUPS] = {0};
    for (int i = 0; i < SF_NUM_PRESETS; i++) {
        const SfVoice *pv = &sf_presets[i].voice;
        if ((pv->type & 0x80) && pv->sub >= 0 && pv->sub < SF_NUM_GROUPS) kits[pv->sub] = 1;
    }
    sCand = malloc(sizeof(Cand) * SF_NUM_GROUPS * 128);
    if (!sCand) return;
    for (int g = 0; g < SF_NUM_GROUPS; g++) {
        if (!kits[g]) continue;
        for (int k = 0; k < 128; k++) {
            const SfVoice *v = &sf_groups[g][k];
            if ((v->type & 0xC7) || v->ref < 0 || v->ref >= SF_NUM_SAMPLES) continue;
            int dup = 0;
            for (int j = 0; j < sNCand && !dup; j++)
                if (sCand[j].v.ref == v->ref && sCand[j].v.key == v->key && sCand[j].v.type == v->type &&
                    sCand[j].v.decay == v->decay && sCand[j].v.sustain == v->sustain && sCand[j].v.release == v->release) dup = 1;
            if (dup) continue;
            sCand[sNCand].v = *v;
            sCand[sNCand].kitKey = k;
            sCand[sNCand].role = role_of_name(sf_samples[v->ref].name);
            sNCand++;
        }
    }
}

static int sig_eq(const RipHit *a, const RipHit *b, int frames) {
    for (int f = 0; f < frames; f++) {
        if (lroundf(12 * log2f(a->hz[f] > 1 ? a->hz[f] : 1)) != lroundf(12 * log2f(b->hz[f] > 1 ? b->hz[f] : 1))) return 0;
        if (lroundf(a->vol[f] * 15) != lroundf(b->vol[f] * 15)) return 0;
    }
    return a->shortMode == b->shortMode;
}

// how loud the engine plays a Game Boy noise channel at full volume (RMS)
static float noise_full(int shortMode) {
    static Engine e;
    static EInst inst;
    static float buf[ENG_FRAME_SAMPLES * 2];
    SfVoice v = VOICE_PLACEHOLDER;
    v.type = 4; v.key = 60; v.ref = (int16_t)(shortMode & 1); v.sub = v.ks = -1;
    v.attack = 0; v.decay = 0; v.sustain = 15; v.release = 0;
    eng_init(&e);
    inst_make(&v, &inst);
    eng_preview_on(&e, &inst, 60, 127);
    double sum = 0;
    long cnt = 0;
    for (int f = 0; f < 12; f++) {
        eng_frame(&e, buf);
        if (f < 2) continue;
        for (int i = 0; i < ENG_FRAME_SAMPLES; i++) { double m = (buf[i * 2] + buf[i * 2 + 1]) * 0.5; sum += m * m; cnt++; }
    }
    eng_preview_off(&e);
    return cnt ? (float)sqrt(sum / cnt) : 0.2f;
}

int rip_drums_match(const RipHit *hits, int n, CustomGroup *kit, RipDrums *out) {
    memset(out, 0, sizeof *out);
    float noiseFull = n > 0 ? noise_full(hits[0].shortMode) : 0.2f;
    if (n <= 0) return 0;
    out->groupOf = malloc(sizeof(int) * (size_t)n);
    if (!out->groupOf) return 0;
    // groups: hits starting the same way (their first three frames); shorter hits join the group they begin like
    int sig = 3;
    for (int i = 0; i < n; i++) {
        out->groupOf[i] = -1;
        if (hits[i].len < sig) continue;
        int g = -1;
        for (int k = 0; k < out->ngroups && g < 0; k++) if (sig_eq(&hits[i], &hits[out->proto[k]], sig)) g = k;
        if (g < 0 && out->ngroups < 64) { g = out->ngroups++; out->proto[g] = i; }
        out->groupOf[i] = g;
        if (g >= 0 && hits[i].len > hits[out->proto[g]].len) out->proto[g] = i;    // the longest is heard
    }
    for (int i = 0; i < n; i++) {
        if (out->groupOf[i] >= 0) continue;
        int fr = hits[i].len < 1 ? 1 : hits[i].len, g = -1, best = 0;
        for (int k = 0; k < out->ngroups; k++) {
            if (!sig_eq(&hits[i], &hits[out->proto[k]], fr)) continue;
            int cnt = 0;
            for (int j = 0; j < n; j++) cnt += out->groupOf[j] == k;
            if (cnt > best) { best = cnt; g = k; }
        }
        if (g < 0 && out->ngroups < 64) { g = out->ngroups++; out->proto[g] = i; }
        out->groupOf[i] = g < 0 ? 0 : g;
    }
    if (sNCand < 0) find_candidates();
    if (!sNCand) return 0;
    // each candidate's spectrogram is made per group (its note is let go where the group's hit ends)
    float *a = malloc(sizeof(float) * WIN_SAMPLES), *b = malloc(sizeof(float) * WIN_SAMPLES);
    static float SA[NSPEC][NMEL], SB[NSPEC][NMEL];
    for (int i = 0; i < 128; i++) kit->v[i] = VOICE_PLACEHOLDER;
    for (int g = 0; g < out->ngroups && a && b; g++) {
        const RipHit *h = &hits[out->proto[g]];
        int frames = h->len < WIN_FRAMES ? h->len : WIN_FRAMES;
        if (frames < 1) frames = 1;
        out->protoVol[g] = h->vol[0] > 0 ? h->vol[0] : 1;
        synth_hit(h, noiseFull, a);
        int samples = frames * ENG_FRAME_SAMPLES + NFFT;            // its sound, and the cut-off after it
        if (samples > WIN_SAMPLES) samples = WIN_SAMPLES;
        int na = melspec(a, samples, SA);
        double ea = 0, peak = 0;
        for (int t = 0; t < na; t++) for (int k = 0; k < NMEL; k++) { ea += SA[t][k]; if (SA[t][k] > peak) peak = SA[t][k]; }
        double floorDb = 10 * log10(peak + 1e-20) - 50;
        // its part in the kit, as an arranger hears a Game Boy drum: the noise's rate (a low rumble, a hiss, a
        // fizz) and how long it rings (from how fast its volume falls)
        double hz = h->hz[0];
        if (h->len > 1 && h->hz[1] > 0) hz = sqrt(h->hz[0] * h->hz[1]);
        // how long it sounds: its longest hit (cut by the next hit, or faded out), the whole length heard
        double ring = h->len;
        int role;
        if (hz < 8000) role = ring <= 10 ? ROLE_KICK : ROLE_TOM;
        else if (hz < 60000) role = ring <= 40 ? ROLE_SNARE : ROLE_CRASH;
        else role = ring <= 8 ? ROLE_CHAT : ring <= 45 ? ROLE_OHAT : ROLE_CRASH;   // (ring: frames)
        int haveRole = 0;
        for (int c = 0; c < sNCand; c++) if (sCand[c].role == role) haveRole = 1;
        int best = -1;
        double bd = 1e30, bg = 1;
        for (int c = 0; c < sNCand; c++) {
            if (haveRole && sCand[c].role != role) continue;
            render_drum(&sCand[c].v, frames, b);
            int nb = melspec(b, samples, SB);
            double eb = 0;
            for (int t = 0; t < nb; t++) for (int k = 0; k < NMEL; k++) eb += SB[t][k];
            if (eb <= 0) continue;
            double gdb = 10 * log10(ea / eb), d = 0;
            int cnt = 0;
            for (int t = 0; t < na && t < nb; t++)
                for (int k = 0; k < NMEL; k++) {
                    double x = 10 * log10(SA[t][k] + 1e-20), y = 10 * log10(SB[t][k] + 1e-20) + gdb;
                    if (x < floorDb && y < floorDb) continue;
                    if (x < floorDb) x = floorDb;
                    if (y < floorDb) y = floorDb;
                    d += fabs(x - y);
                    cnt++;
                }
            if (!cnt) continue;
            d /= cnt;
            if (d < bd) { bd = d; best = c; bg = pow(10, gdb / 20); }
        }
        if (best < 0) continue;
        out->key[g] = 36 + g < 128 ? 36 + g : 127;
        out->drum[g] = sCand[best].kitKey;
        out->gain[g] = (float)bg;
        out->dist[g] = (float)bd;
        kit->v[out->key[g]] = sCand[best].v;
        // a hi-hat stops when the next hit comes (it's choked); the others ring out their whole sample
        if (role != ROLE_CHAT && role != ROLE_OHAT) {
            const SfVoice *bv = &sCand[best].v;
            const SfSample *ss = &sf_samples[bv->ref];
            double rate = ss->freq / 1024.0 * ((bv->type & 0x08) ? 1.0 : pow(2.0, (bv->key - 60) / 12.0));
            double sec = (ss->flags & 0xC0000000u) ? 1.5 : ss->size / (rate > 0 ? rate : 13379);
            out->hold[g] = (int)ceil(sec * 60);
        }
        // its level: as loud as the Game Boy hit over the 150 ms the ear takes a drum's loudness from, the drum
        // played as the song will (held its ring, or let go with the hit)
        {
            int lf = 9, play = out->hold[g] > frames ? out->hold[g] : frames;
            render_drum(&sCand[best].v, play, b);
            double e1 = 0, e2 = 0;
            for (int i = 0; i < lf * ENG_FRAME_SAMPLES; i++) { e1 += (double)a[i] * a[i]; e2 += (double)b[i] * b[i]; }
            if (e1 > 0 && e2 > 0) out->gain[g] = (float)sqrt(e1 / e2);
        }
        int cntHits = 0;
        for (int j = 0; j < n; j++) cntHits += out->groupOf[j] == g;
        if (getenv("MMS_GM_DEBUG"))
            fprintf(stderr, "drum group %d: %d hits, %d frames, %.0f Hz vol %.2f rings %.0f frames: %s -> %s (dist %.1f dB, gain %.2f)\n", g,
                    cntHits, frames, hz, h->vol[0], ring, ROLE_NAMES[role], sf_samples[sCand[best].v.ref].name, bd, bg);
        (void)sCand[best].kitKey;
    }
    free(a);
    free(b);
    return out->ngroups;
}

void rip_drums_free(RipDrums *d) {
    free(d->groupOf);
    d->groupOf = NULL;
}
