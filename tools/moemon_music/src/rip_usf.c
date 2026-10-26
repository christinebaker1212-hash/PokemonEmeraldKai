// Moemon Music Studio: USF (Nintendo 64) rips, played by lazyusf2 (third_party/lazyusf2).
//
// The N64 mixes its music on the RSP from a command list each audio frame. The capture hook in the
// emulated microcode (rsp_hle/alist.c) hears each voice's commands: the ADPCM codebook it loads, the
// decode (whose state address names the voice; its init flag starts a note), the resample (pitch) and
// the envelope mixer (volume and pan). A sample is found from its codebook: the sound driver keeps a
// wavetable (libultra's ALWaveTable, EAD's sample struct) that points at the book, and that gives
// where the sample is in the ROM, how long it is and its loop. It is decoded from the ROM.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stddef.h>
#include "rip.h"
#include "psflib.h"
#include "usf/usf.h"
#include "rsp_hle/hle_internal.h"
#include "rsp_hle/alist.h"
#include "ai/ai_controller.h"

extern const psf_file_callbacks rip_psf_files;

#define USF_VOICES 48

typedef struct {
    uint32_t state;          // its ADPCM decoder state address (0 = slot free)
    int seenTask;            // last task it was in
    int keyon;               // notes started since the last snapshot
    uint32_t book;           // codebook address
    uint32_t load;           // the sample data it read first (DRAM)
    uint32_t pitch;          // Q16.16 at the output rate
    float l, r;              // volume
    int sample;              // RipSample (-1 = not found yet)
    int twoBit;
} UVoice;

typedef struct {             // a voice as one audio task left it
    int16_t sample;
    uint8_t active, keyon;
    float rate, l, r;
} TaskVoice;

typedef struct {
    long play;               // native sample where its sound starts playing (-1 = not heard yet)
    int nsave;
    uint32_t saveLo[8], saveHi[8];
    TaskVoice v[USF_VOICES];
} Task;

typedef struct {
    Rip *rip;
    void *state;
    Task *tasks;
    int ntasks, capTasks;
    long aiTotal;            // samples the audio interface has started playing
    int taskKeyon[USF_VOICES];
    int compare, fifo;
    int task;
    uint32_t book, load;
    int cur;                 // the voice the last decode was for
    UVoice v[USF_VOICES];
    int debug;
    uint32_t cacheBook[512];     // codebook -> sample, so the RAM is searched once per instrument
    int cacheSample[512];
    int ncache;
} Usf;

static Usf *g_usf;

static uint32_t dram32(struct hle_t *hle, uint32_t a) { return *(uint32_t *)(hle->dram + (a & 0x7FFFFC)); }
static uint8_t dram8(struct hle_t *hle, uint32_t a) { return hle->dram[(a & 0x7FFFFF) ^ 3]; }
static const uint8_t *g_rom;
static size_t g_romSize;
static uint8_t rom8(void *s, uint32_t o) { (void)s; uint32_t x = o ^ 3; return x < g_romSize ? g_rom[x] : 0; }

// VADPCM: 9-byte frames (scale / predictor, 16 4-bit samples), predicted from a codebook of
// `order` x 8 coefficients per predictor.
static int16_t *vadpcm_decode(const uint8_t *data, int frames, const int16_t *book, int order, int npred, int *outLen) {
    int16_t *out = malloc(sizeof(int16_t) * (size_t)frames * 16 + 16);
    if (!out) return NULL;
    int16_t prev[16] = {0};   // the last 8 outputs (order 2 uses the last two)
    int n = 0;
    for (int f = 0; f < frames; f++) {
        const uint8_t *fr = data + f * 9;
        int scale = 1 << (fr[0] >> 4), pred = fr[0] & 15;
        if (pred >= npred) pred = 0;
        const int16_t *cb = book + pred * order * 8;
        for (int half = 0; half < 2; half++) {
            int ins[8];
            for (int i = 0; i < 8; i++) {
                int b = fr[1 + half * 4 + i / 2];
                int nib = (i & 1) ? (b & 15) : (b >> 4);
                if (nib >= 8) nib -= 16;
                ins[i] = nib * scale;
            }
            int16_t o8[8];
            for (int i = 0; i < 8; i++) {
                int64_t acc = 0;
                // the previous outputs, then the earlier inputs of this group
                for (int k = 0; k < order; k++) acc += (int64_t)cb[k * 8 + i] * prev[8 - order + k];
                for (int j = 0; j < i; j++) acc += (int64_t)cb[(order - 1) * 8 + i - 1 - j] * ins[j];
                acc += (int64_t)ins[i] << 11;
                int v = (int)(acc >> 11);
                o8[i] = (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v);
            }
            for (int i = 0; i < 8; i++) { prev[i] = o8[i]; out[n++] = o8[i]; }
        }
    }
    *outLen = n;
    return out;
}

// The driver's wavetable for a codebook: a word pointing at the book (KSEG0), in either
// libultra's ALWaveTable {base, len, type, flags, loop*, book*} or EAD's {size bits, addr, loop*, book*}.
static int find_sample_uncached(Usf *u, struct hle_t *hle, UVoice *v);

static int find_sample(Usf *u, struct hle_t *hle, UVoice *v) {
    for (int i = 0; i < u->ncache; i++) if (u->cacheBook[i] == v->book) return u->cacheSample[i];
    int s = find_sample_uncached(u, hle, v);
    if (u->ncache < 512) { u->cacheBook[u->ncache] = v->book; u->cacheSample[u->ncache++] = s; }
    return s;
}

static int find_sample_uncached(Usf *u, struct hle_t *hle, UVoice *v) {
    // the codebook load names the coefficients; the book struct {order, npredictors, coefficients} is 8 bytes before
    uint32_t sb = (v->book - 8) & 0x7FFFFF;
    uint32_t want = 0x80000000u | sb;
    void *s = u->state;
    g_rom = usf_tap_rom(s, &g_romSize);
    uint32_t ram = 0x400000;
    for (uint32_t a = 16; a + 4 <= ram; a += 4) {
        if (dram32(hle, a) != want) continue;
        int16_t book[256];
        int order = 0, npred = 0;
        uint32_t base = 0, len = 0, loopPtr = 0, inRam = 0;
        // libultra: book ptr at +16 of ALWaveTable; the book is {order, npredictors, coefficients}
        uint32_t lbase = dram32(hle, a - 16), llen = dram32(hle, a - 12), lloop = dram32(hle, a - 4);
        uint32_t bo = dram32(hle, sb), bn = dram32(hle, sb + 4);
        // EAD: {u32 bits (size in the low 24), sampleAddr, loop*, book*} with book ptr at +12
        uint32_t ebits = dram32(hle, a - 12), eaddr = dram32(hle, a - 8), eloop = dram32(hle, a - 4);
        if (bo >= 1 && bo <= 8 && bn >= 1 && bn <= 16 && llen > 0 && llen < 0x1000000 && (lloop == 0 || (lloop >> 24) == 0x80)) {
            order = (int)bo; npred = (int)bn; base = lbase; len = llen; loopPtr = lloop;
            for (int i = 0; i < order * npred * 8 && i < 256; i++) book[i] = (int16_t)(dram32(hle, sb + 8 + (i & ~1) * 2) >> ((i & 1) ? 0 : 16));
        } else if (bo >= 1 && bo <= 8 && bn >= 1 && bn <= 16 && (ebits & 0xFFFFFF) > 0 && (eloop >> 24) == 0x80) {
            order = (int)bo; npred = (int)bn; base = eaddr; len = ebits & 0xFFFFFF; loopPtr = eloop;
            inRam = (eaddr >> 24) == 0x80;
            for (int i = 0; i < order * npred * 8 && i < 256; i++) book[i] = (int16_t)(dram32(hle, sb + 8 + (i & ~1) * 2) >> ((i & 1) ? 0 : 16));
        } else continue;
        if ((base >> 24) == 0x80) inRam = 1;
        int frames = (int)(len / 9);
        if (frames <= 0 || frames > 200000) continue;
        uint8_t *data = malloc((size_t)frames * 9);
        if (!data) return -1;
        for (int i = 0; i < frames * 9; i++)
            data[i] = inRam ? dram8(hle, (base & 0x7FFFFF) + i) : rom8(s, (base & 0x0FFFFFFF) + i);
        int n = 0;
        int16_t *pcm = vadpcm_decode(data, frames, book, order, npred, &n);
        uint32_t h = (uint32_t)rip_hash(data, frames * 9 < 4096 ? frames * 9 : 4096) ^ len;
        free(data);
        if (!pcm) return -1;
        RipSample rs;
        memset(&rs, 0, sizeof rs);
        rs.kind = RIPS_PCM;
        rs.pcm = pcm;
        rs.len = n;
        rs.rate = usf_tap_sample_rate(s) > 0 ? usf_tap_sample_rate(s) : 32000;
        rs.id = base;
        rs.hash = h;
        if (loopPtr && (loopPtr >> 24) == 0x80) {
            uint32_t ls = dram32(hle, loopPtr), le = dram32(hle, loopPtr + 4), cnt = dram32(hle, loopPtr + 8);
            if (cnt && le > ls && le <= (uint32_t)n) { rs.loops = 1; rs.loopStart = (int)ls; rs.len = (int)le; }
        }
        int idx = rip_add_sample(u->rip, &rs);
        free(pcm);
        if (u->debug) fprintf(stderr, "usf: book %06x -> %s wavetable at %06x base %08x len %u loop %d@%d -> s%d\n", v->book,
                              base == lbase ? "libultra" : "EAD", a, base, len, rs.loops, rs.loopStart, idx);
        return idx;
    }
    if (u->debug) fprintf(stderr, "usf: no wavetable for book %06x\n", v->book);
    return -1;
}

static void begin_task(Usf *u) {
    if (u->ntasks == u->capTasks) {
        int cap = u->capTasks ? u->capTasks * 2 : 1024;
        Task *nt = realloc(u->tasks, sizeof(Task) * (size_t)cap);
        if (!nt) return;
        u->tasks = nt;
        u->capTasks = cap;
    }
    Task *t = &u->tasks[u->ntasks++];
    memset(t, 0, sizeof *t);
    t->play = -1;
    memset(u->taskKeyon, 0, sizeof u->taskKeyon);
}

static void end_task(Usf *u) {
    if (!u->ntasks) return;
    Task *t = &u->tasks[u->ntasks - 1];
    double rate = usf_tap_sample_rate(u->state) > 0 ? usf_tap_sample_rate(u->state) : 32000;
    for (int i = 0; i < USF_VOICES; i++) {
        UVoice *v = &u->v[i];
        TaskVoice *x = &t->v[i];
        x->sample = (int16_t)v->sample;
        x->active = v->state && v->seenTask == u->task && v->sample >= 0;
        x->keyon = u->taskKeyon[i] > 0;
        x->rate = (float)(v->pitch / 65536.0 * rate);
        x->l = v->l;
        x->r = v->r;
    }
}

static void ai_played(uint32_t address, uint32_t length) {
    Usf *u = g_usf;
    if (!u) return;
    uint32_t a = address & 0xFFFFFF;
    // the newest task that saved its sound there
    for (int i = u->ntasks - 1; i >= 0 && i >= u->ntasks - 64; i--) {
        Task *t = &u->tasks[i];
        if (t->play >= 0) continue;
        for (int k = 0; k < t->nsave; k++)
            if (a >= t->saveLo[k] && a < t->saveHi[k]) { t->play = u->aiTotal; i = -1; break; }
    }
    u->aiTotal += length / 4;
}

static void tap(struct hle_t *hle, int what, uint32_t a, int32_t b, int32_t c, int32_t d) {
    Usf *u = g_usf;
    if (!u) return;
    switch (what) {
    case ALIST_TAP_TASK:
        if (u->task > 0) end_task(u);
        u->task++;
        u->cur = -1;
        begin_task(u);
        break;
    case ALIST_TAP_SAVE:
        if (u->ntasks > 0) {
            Task *t = &u->tasks[u->ntasks - 1];
            uint32_t lo = a & 0xFFFFFF, hi = lo + (uint32_t)b;
            int merged = 0;
            for (int i = 0; i < t->nsave; i++)
                if (lo <= t->saveHi[i] && hi >= t->saveLo[i]) {
                    if (lo < t->saveLo[i]) t->saveLo[i] = lo;
                    if (hi > t->saveHi[i]) t->saveHi[i] = hi;
                    merged = 1;
                    break;
                }
            if (!merged && t->nsave < 8) { t->saveLo[t->nsave] = lo; t->saveHi[t->nsave] = hi; t->nsave++; }
        }
        break;
    case ALIST_TAP_BOOK: u->book = a & 0xFFFFFF; break;
    case ALIST_TAP_LOAD: u->load = a & 0xFFFFFF; break;
    case ALIST_TAP_ADPCM: {
        uint32_t st = a & 0xFFFFFF;
        int k = -1, freeSlot = -1;
        for (int i = 0; i < USF_VOICES; i++) {
            if (u->v[i].state == st) { k = i; break; }
            if (freeSlot < 0 && (!u->v[i].state || u->v[i].seenTask < u->task - 600)) freeSlot = i;
        }
        if (k < 0) {
            if (freeSlot < 0) { u->cur = -1; break; }
            k = freeSlot;
            memset(&u->v[k], 0, sizeof u->v[k]);
            u->v[k].state = st;
            u->v[k].sample = -1;
        }
        UVoice *v = &u->v[k];
        u->cur = k;
        int first = v->seenTask != u->task;
        v->seenTask = u->task;
        if (b && first) {          // a note starts
            v->keyon++;
            u->taskKeyon[k]++;
            v->book = u->book;
            v->load = u->load;
            v->twoBit = d;
            v->sample = find_sample(u, hle, v);
        } else if (v->book != u->book && first) {
            v->book = u->book;
            v->sample = find_sample(u, hle, v);
        }
        break;
    }
    case ALIST_TAP_RESAMPLE:
        if (u->cur >= 0) u->v[u->cur].pitch = (uint32_t)b;
        break;
    case ALIST_TAP_ENVMIX:
        if (u->cur >= 0) {
            int dry = (int16_t)((uint32_t)d >> 16);
            u->v[u->cur].l = (float)abs((int16_t)b) / 32768.0f * (float)abs(dry) / 32768.0f;
            u->v[u->cur].r = (float)abs((int16_t)c) / 32768.0f * (float)abs(dry) / 32768.0f;
        }
        break;
    case ALIST_TAP_ENVMIX_NEAD:
        if (u->cur >= 0) {
            u->v[u->cur].l = (uint16_t)b / 65536.0f;
            u->v[u->cur].r = (uint16_t)c / 65536.0f;
        }
        break;
    }
}

static int usf_load(void *ctx, const uint8_t *exe, size_t en, const uint8_t *res, size_t rn) {
    Usf *u = ctx;
    if (exe && en) return -1;
    return usf_upload_section(u->state, res, rn);
}

static int usf_info(void *ctx, const char *name, const char *value) {
    Usf *u = ctx;
    if (!strcasecmp(name, "_enablecompare") && *value) u->compare = 1;
    else if (!strcasecmp(name, "_enablefifofull") && *value) u->fifo = 1;
    else rip_psf_tag(u->rip, name, value);
    return 0;
}

static int usf_render_cb(RipEmu *e, int16_t *buf, int n) {
    Usf *u = e->ctx;
    const char *err = usf_render_resampled(u->state, buf, (size_t)n, e->rate);
    if (err) memset(buf, 0, (size_t)n * 4);
    return 0;
}

static void usf_snapshot(RipEmu *e, RipVF *out) { (void)e; (void)out; }   // finalize() does it

// The tasks on the game's frames: each frame gets the voices as the last task heard by its end left
// them, and the notes started by the tasks heard during it.
static void usf_finalize(RipEmu *e) {
    Usf *u = e->ctx;
    Rip *r = e->rip;
    if (u->task > 0) end_task(u);
    double rate = usf_tap_sample_rate(u->state) > 0 ? usf_tap_sample_rate(u->state) : 32000;
    double fps = 42048.0 / 701.0;
    // tasks never matched to a buffer: halfway between their neighbours
    long last = 0;
    for (int i = 0; i < u->ntasks; i++) {
        if (u->tasks[i].play >= 0) { last = u->tasks[i].play; continue; }
        long next = -1;
        for (int j = i + 1; j < u->ntasks && j < i + 64; j++) if (u->tasks[j].play >= 0) { next = u->tasks[j].play; break; }
        u->tasks[i].play = next >= 0 ? (last + next) / 2 : last;
    }
    int ti = 0;
    const Task *cur = NULL;
    for (int f = 0; f < r->nframes; f++) {
        long end = (long)((f + 1) * rate / fps);
        uint8_t kon[USF_VOICES] = {0};
        while (ti < u->ntasks && u->tasks[ti].play < end) {
            cur = &u->tasks[ti];
            for (int i = 0; i < USF_VOICES; i++) if (cur->v[i].keyon) kon[i] = 1;
            ti++;
        }
        RipVF *row = r->vf + (size_t)f * r->nvoices;
        for (int i = 0; i < USF_VOICES; i++) {
            RipVF *x = &row[i];
            memset(x, 0, sizeof *x);
            x->sample = -1;
            if (!cur) continue;
            const TaskVoice *tv = &cur->v[i];
            x->keyon = kon[i];
            if (!tv->active || tv->sample < 0) continue;
            x->sample = tv->sample;
            x->rate = tv->rate;
            x->l = tv->l;
            x->r = tv->r;
        }
    }
}

static void usf_close(RipEmu *e) {
    Usf *u = e->ctx;
    if (!u) return;
    alist_tap = NULL;
    ai_tap = NULL;
    if (g_usf == u) g_usf = NULL;
    free(u->tasks);
    usf_shutdown(u->state);
    free(u->state);
    free(u);
    e->ctx = NULL;
}

int rip_open_usf(RipEmu *e, Rip *r, char *err, int errlen) {
    Usf *u = calloc(1, sizeof *u);
    if (!u) return 0;
    u->rip = r;
    u->debug = getenv("MMS_USF_DEBUG") != NULL;
    u->state = malloc(usf_get_state_size());
    if (!u->state) { free(u); return 0; }
    usf_clear(u->state);
    if (psf_load(r->path, &rip_psf_files, 0x21, usf_load, u, usf_info, u, 0, NULL, NULL) < 0) {
        snprintf(err, errlen, "couldn't read %s (or its _lib file)", r->path);
        usf_shutdown(u->state);
        free(u->state);
        free(u);
        return 0;
    }
    usf_set_compare(u->state, u->compare);
    usf_set_fifo_full(u->state, u->fifo);
    usf_set_hle_audio(u->state, 1);     // the capture listens to the emulated audio microcode
    for (int i = 0; i < USF_VOICES; i++) u->v[i].sample = -1;
    g_usf = u;
    alist_tap = tap;
    ai_tap = ai_played;
    snprintf(r->console, sizeof r->console, "Nintendo 64");
    e->ctx = u;
    e->rip = r;
    e->rate = 44100;
    e->nvoices = USF_VOICES;
    e->render = usf_render_cb;
    e->snapshot = usf_snapshot;
    e->close = usf_close;
    e->finalize = usf_finalize;
    return 1;
}
