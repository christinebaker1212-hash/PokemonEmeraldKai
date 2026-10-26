// Moemon Music Studio: GameCube streamed music (Pokemon Colosseum / XD): .ish + .isd ("I_SF" header,
// interleaved DSP-ADPCM data) and standard Nintendo .dsp files (one channel, 0x60-byte header).
//
// These are recordings, not notes: there is no sound chip to listen to. The song made from one is the
// recording itself, a sample per stereo side at the GBA's rate, looping where the game loops it.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rip.h"

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static int16_t be16s(const uint8_t *p) { return (int16_t)(p[0] << 8 | p[1]); }

static uint8_t *slurp(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *b = n > 0 ? malloc((size_t)n) : NULL;
    if (b && fread(b, 1, (size_t)n, f) != (size_t)n) { free(b); b = NULL; }
    fclose(f);
    *len = b ? (size_t)n : 0;
    return b;
}

// DSP-ADPCM: 8-byte frames, a header byte (predictor << 4 | scale) and 14 4-bit samples.
static void dsp_decode(const uint8_t *data, size_t bytes, const int16_t coef[16], int16_t *out, long nsamples, int stride) {
    int h1 = 0, h2 = 0;
    long n = 0;
    for (size_t f = 0; f + 8 <= bytes && n < nsamples; f += 8) {
        int ps = data[f], pred = (ps >> 4) & 7, scale = 1 << (ps & 15);
        int c1 = coef[pred * 2], c2 = coef[pred * 2 + 1];
        for (int i = 0; i < 14 && n < nsamples; i++) {
            int nib = (data[f + 1 + i / 2] >> ((i & 1) ? 0 : 4)) & 15;
            if (nib >= 8) nib -= 16;
            int s = ((nib * scale) << 11) + 1024 + c1 * h1 + c2 * h2;
            s >>= 11;
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            h2 = h1;
            h1 = s;
            out[n++ * stride] = (int16_t)s;
        }
    }
}

typedef struct {
    int16_t *pcm;            // interleaved stereo, the whole stream
    long frames, pos;
    long loopStart, loopEnd; // -1: no loop
} Gcn;

static int gcn_render(RipEmu *e, int16_t *buf, int n) {
    Gcn *g = e->ctx;
    for (int i = 0; i < n; i++) {
        if (g->loopStart >= 0 && g->pos >= g->loopEnd) g->pos = g->loopStart;
        if (g->pos < g->frames) {
            buf[i * 2] = g->pcm[g->pos * 2];
            buf[i * 2 + 1] = g->pcm[g->pos * 2 + 1];
            g->pos++;
        } else buf[i * 2] = buf[i * 2 + 1] = 0;
    }
    return 0;
}
static void gcn_snapshot(RipEmu *e, RipVF *out) { (void)e; (void)out; }
static void gcn_close(RipEmu *e) {
    Gcn *g = e->ctx;
    if (g) { free(g->pcm); free(g); }
    e->ctx = NULL;
}

static int has_ext(const char *p, const char *ext) {
    size_t a = strlen(p), b = strlen(ext);
    if (a < b) return 0;
    for (size_t i = 0; i < b; i++) {
        char c = p[a - b + i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (c != ext[i]) return 0;
    }
    return 1;
}

int rip_open_gcn(RipEmu *e, Rip *r, char *err, int errlen) {
    char head[1100], data[1100];
    snprintf(head, sizeof head, "%s", r->path);
    snprintf(data, sizeof data, "%s", r->path);
    size_t hl = strlen(head);
    // .ish + .isd: whichever was opened, find the other
    if (has_ext(head, ".isd")) head[hl - 1] = head[hl - 1] == 'D' ? 'H' : 'h';
    if (has_ext(data, ".ish")) data[hl - 1] = data[hl - 1] == 'H' ? 'D' : 'd';
    size_t hlen = 0, dlen = 0;
    uint8_t *h = slurp(head, &hlen);
    if (!h || hlen < 0x60) { free(h); snprintf(err, errlen, "can't read %s", head); return 0; }
    Gcn *g = calloc(1, sizeof *g);
    if (!g) { free(h); return 0; }
    int channels, rate;
    long nsamples, loopStart = -1, loopEnd = -1;
    if (!memcmp(h, "I_SF", 4)) {
        // "I_SF": rate 0x08, samples 0x0C, channels 0x14, interleave 0x18, loop 0x1C/0x20/0x24 (bytes), coefs 0x40 + 0x40 * ch
        uint8_t *d = has_ext(r->path, ".dsp") ? NULL : slurp(data, &dlen);
        if (!d) {
            snprintf(err, errlen, "%s is only a header: its sound data (a .isd file of the same name) isn't there", head);
            free(h); free(g);
            return 0;
        }
        rate = (int)be32(h + 0x08);
        nsamples = (long)be32(h + 0x0C);
        channels = (int)be32(h + 0x14);
        uint32_t il = be32(h + 0x18);
        if (channels < 1 || channels > 2 || rate < 4000 || rate > 96000 || !il || hlen < 0x40 + 0x40u * channels) {
            snprintf(err, errlen, "%s: an I_SF header this studio doesn't understand", head);
            free(h); free(d); free(g);
            return 0;
        }
        if (be32(h + 0x1C)) {
            loopStart = (long)((uint64_t)be32(h + 0x20) * 14 / 8 / channels);
            loopEnd = (long)((uint64_t)be32(h + 0x24) * 14 / 8 / channels);
        }
        long perCh = (long)(dlen / channels / 8 * 14);
        if (nsamples > perCh) nsamples = perCh;
        g->pcm = calloc((size_t)nsamples * 2 + 2, sizeof(int16_t));
        // de-interleave each channel's blocks, then decode it
        size_t chBytes = dlen / channels;
        uint8_t *cb = malloc(chBytes);
        for (int c = 0; c < channels && g->pcm && cb; c++) {
            size_t o = 0;
            for (size_t blk = (size_t)c * il; blk < dlen && o < chBytes; blk += il * channels) {
                size_t take = il;
                if (blk + take > dlen) take = dlen - blk;
                if (o + take > chBytes) take = chBytes - o;
                memcpy(cb + o, d + blk, take);
                o += take;
            }
            int16_t coef[16];
            for (int i = 0; i < 16; i++) coef[i] = be16s(h + 0x40 + 0x40 * c + i * 2);
            dsp_decode(cb, o, coef, g->pcm + c, nsamples, 2);
        }
        free(cb);
        free(d);
    } else if (has_ext(r->path, ".dsp")) {
        // Nintendo .dsp: samples 0x00, nibbles 0x04, rate 0x08, loop 0x0C, loop start / end 0x10 / 0x14 (nibbles), coefs 0x1C
        nsamples = (long)be32(h);
        rate = (int)be32(h + 0x08);
        channels = 1;
        if (rate < 4000 || rate > 96000 || hlen <= 0x60) {
            snprintf(err, errlen, "%s: not a DSP-ADPCM file this studio understands", head);
            free(h); free(g);
            return 0;
        }
        if (h[0x0C] || h[0x0D]) {
            loopStart = (long)(be32(h + 0x10) / 16 * 14 + (be32(h + 0x10) % 16 >= 2 ? be32(h + 0x10) % 16 - 2 : 0));
            loopEnd = (long)(be32(h + 0x14) / 16 * 14 + (be32(h + 0x14) % 16 >= 2 ? be32(h + 0x14) % 16 - 2 : 0)) + 1;
        }
        long have = (long)((hlen - 0x60) / 8 * 14);
        if (nsamples > have) nsamples = have;
        int16_t coef[16];
        for (int i = 0; i < 16; i++) coef[i] = be16s(h + 0x1C + i * 2);
        g->pcm = calloc((size_t)nsamples * 2 + 2, sizeof(int16_t));
        if (g->pcm) {
            dsp_decode(h + 0x60, hlen - 0x60, coef, g->pcm, nsamples, 2);
            for (long i = 0; i < nsamples; i++) g->pcm[i * 2 + 1] = g->pcm[i * 2];
        }
    } else {
        snprintf(err, errlen, "%s isn't GameCube music this studio knows", head);
        free(h); free(g);
        return 0;
    }
    free(h);
    if (!g->pcm || nsamples <= 0) { free(g->pcm); free(g); snprintf(err, errlen, "no sound in %s", r->path); return 0; }
    if (channels == 1) for (long i = 0; i < nsamples; i++) g->pcm[i * 2 + 1] = g->pcm[i * 2];
    g->frames = nsamples;
    if (loopStart >= 0 && (loopEnd <= loopStart || loopEnd > nsamples)) loopEnd = nsamples;
    g->loopStart = loopStart;
    g->loopEnd = loopEnd;
    // the original player hears the intro, the loop twice and a fade
    if (r->length <= 0) {
        if (loopStart >= 0) { r->length = (loopEnd + (loopEnd - loopStart)) / (double)rate; r->fade = 8; }
        else r->length = nsamples / (double)rate;
    }
    // the stream itself, for the song made from it
    r->streamPcm = malloc(sizeof(int16_t) * 2 * (size_t)nsamples);
    if (r->streamPcm) memcpy(r->streamPcm, g->pcm, sizeof(int16_t) * 2 * (size_t)nsamples);
    r->streamFrames = nsamples;
    r->streamRate = rate;
    r->streamLoopStart = loopStart;
    r->streamLoopEnd = loopEnd;
    r->streamStereo = channels == 2;
    snprintf(r->console, sizeof r->console, "GameCube");
    e->ctx = g;
    e->rip = r;
    e->rate = rate;
    e->nvoices = 0;
    e->render = gcn_render;
    e->snapshot = gcn_snapshot;
    e->close = gcn_close;
    return 1;
}
