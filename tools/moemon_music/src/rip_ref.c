// Moemon Music Studio: a reference mix of what a rip's capture says, at full precision: every voice plays
// its original 16-bit sample at its own rate (cubic interpolation), with pitch and volume moving smoothly
// through each frame, mixed in floating point. Measured against the original sound it shows what the
// capture misses (reverb, effects); measured against the game engine's version, what the engine loses.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rip.h"
#include "engine.h"

static void w16(FILE *f, int v) { fputc(v & 255, f); fputc((v >> 8) & 255, f); }
static void w32(FILE *f, uint32_t v) { w16(f, (int)(v & 0xFFFF)); w16(f, (int)(v >> 16)); }

// MMS_REF_LIMIT (tests): what the game engine does that the reference doesn't, one at a time, to measure
// each: "8bit" samples, "steps" (volume and pitch change once a frame), "vol8" (volumes in 256 steps),
// "rate13k" (samples at about 13379 Hz as heard), "linear" (linear interpolation, like the engine)
static int has_limit(const char *w) { const char *e = getenv("MMS_REF_LIMIT"); return e && strstr(e, w); }

static float smp(const RipSample *s, double pos) {
    // the sample at a fractional position (loops wrap), 4-point cubic
    long i = (long)floor(pos);
    double t = pos - i;
    float y[4];
    for (int k = 0; k < 4; k++) {
        long j = i - 1 + k;
        if (j >= s->len) {
            if (!s->loops || s->len <= s->loopStart) { y[k] = 0; continue; }
            j = s->loopStart + (j - s->len) % (s->len - s->loopStart);
        }
        y[k] = j < 0 ? 0 : s->pcm[j] / 32768.0f;
    }
    if (has_limit("8bit")) {           // 8 bits over the sample's own range (the conversion fills it)
        float pk = s->peak > 0 ? s->peak : 1;
        for (int k = 0; k < 4; k++) y[k] = (float)(lroundf(y[k] / pk * 127) / 127.0 * pk);
    }
    if (has_limit("linear")) return (float)(y[1] + (y[2] - y[1]) * t);
    double a = -0.5 * y[0] + 1.5 * y[1] - 1.5 * y[2] + 0.5 * y[3];
    double b = y[0] - 2.5 * y[1] + 2 * y[2] - 0.5 * y[3];
    double c = -0.5 * y[0] + 0.5 * y[2];
    return (float)(((a * t + b) * t + c) * t + y[1]);
}

int rip_render_reference(const Rip *r, const char *path, int seconds) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    int nf = r->nframes, nv = r->nvoices;
    if (seconds > 0 && seconds * 60 < nf) nf = seconds * 60;
    uint32_t n = (uint32_t)nf * ENG_FRAME_SAMPLES;
    fwrite("RIFF", 1, 4, f); w32(f, 36 + n * 4); fwrite("WAVEfmt ", 1, 8, f);
    w32(f, 16); w16(f, 1); w16(f, 2); w32(f, ENG_RATE); w32(f, ENG_RATE * 4); w16(f, 4); w16(f, 16);
    fwrite("data", 1, 4, f); w32(f, n * 4);
    double *pos = calloc((size_t)nv, sizeof(double));
    float *mix = malloc(sizeof(float) * ENG_FRAME_SAMPLES * 2);
    for (int fr = 0; fr < nf; fr++) {
        memset(mix, 0, sizeof(float) * ENG_FRAME_SAMPLES * 2);
        for (int v = 0; v < nv; v++) {
            const RipVF *x = &r->vf[(size_t)fr * nv + v];
            const RipVF *p = fr > 0 ? &r->vf[(size_t)(fr - 1) * nv + v] : NULL;
            if (x->keyon) pos[v] = 0;
            if (x->sample < 0 || x->sample >= r->nsamples) continue;
            const RipSample *s = &r->samples[x->sample];
            if (s->kind != RIPS_PCM || !s->pcm || s->rate <= 0) continue;
            int same = p && p->sample == x->sample && !x->keyon && !has_limit("steps");
            float l0 = same ? p->l : x->l, r0 = same ? p->r : x->r;
            double rate0 = same && p->rate > 0 ? p->rate : x->rate;
            for (int i = 0; i < ENG_FRAME_SAMPLES; i++) {
                double t = (i + 0.5) / ENG_FRAME_SAMPLES;
                double rate = rate0 > 0 && x->rate > 0 ? rate0 * pow(x->rate / rate0, t) : x->rate;
                float y = smp(s, pos[v]);
                float gl = (float)(l0 + (x->l - l0) * t), gr = (float)(r0 + (x->r - r0) * t);
                if (has_limit("vol8")) { gl = lroundf(gl * 255) / 255.0f; gr = lroundf(gr * 255) / 255.0f; }
                mix[i * 2] += y * gl;
                mix[i * 2 + 1] += y * gr;
                pos[v] += rate / ENG_RATE;
            }
        }
        for (int i = 0; i < ENG_FRAME_SAMPLES * 2; i++) {
            float v = mix[i];
            if (v > 1) v = 1;
            if (v < -1) v = -1;
            w16(f, (int)lroundf(v * 32767));
        }
    }
    free(pos);
    free(mix);
    return fclose(f) == 0;
}
