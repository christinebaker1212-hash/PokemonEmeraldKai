// Moemon Music Studio: GBA songs on the game's own engine (MP2K), read from the ROM.
//
// The emulator finds the playing song (rip_gsf.cpp: the music player's SongHeader). Its tracks become
// a MIDI the way mid2agb reads one back (VOL, PAN, BEND, BENDR, LFOS, LFODL, MOD, MODT, TUNE, PRIO and
// XCMD as mid2agb's controllers, tempo, loop markers, exact gate times), so converting it gives the
// original commands again. The song's voicegroup becomes the project's instruments: samples, kits and
// key-split tables the game also has are used by name, the rest come along as the song's own.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rip_mp2k.h"
#include "inst.h"

typedef struct { const uint8_t *rom; size_t len; } Rom;

static int in_rom(const Rom *r, uint32_t addr, uint32_t n) {
    if ((addr >> 25) != (0x08000000u >> 25)) return 0;
    uint32_t o = addr & 0x01FFFFFF;
    return o + n <= r->len && o + n >= o;
}
static const uint8_t *at(const Rom *r, uint32_t addr) { return r->rom + (addr & 0x01FFFFFF); }
static uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

// ------------------------------------------------------------------ MIDI building
typedef struct { uint32_t time; int order; uint8_t st, a, b; uint8_t meta; char text[2]; } Ev;
typedef struct { Ev *e; int n, cap; } EvL;

static void add(EvL *l, uint32_t time, int st, int a, int b) {
    if (l->n == l->cap) {
        int cap = l->cap ? l->cap * 2 : 256;
        Ev *ne = realloc(l->e, sizeof(Ev) * cap);
        if (!ne) return;
        l->e = ne;
        l->cap = cap;
    }
    Ev *e = &l->e[l->n];
    memset(e, 0, sizeof *e);
    e->time = time;
    e->order = l->n++;
    e->st = (uint8_t)st;
    e->a = (uint8_t)a;
    e->b = (uint8_t)b;
}

static int ev_cmp(const void *x, const void *y) {
    const Ev *a = x, *b = y;
    if (a->time != b->time) return a->time < b->time ? -1 : 1;
    return a->order - b->order;
}

typedef struct { uint8_t *b; size_t n, cap; } Buf;
static void bput(Buf *m, const void *d, size_t n) {
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
static void b8(Buf *m, int v) { uint8_t x = (uint8_t)v; bput(m, &x, 1); }
static void bvar(Buf *m, uint32_t v) {
    uint8_t t[5];
    int n = 0;
    t[n++] = v & 0x7F;
    while (v >>= 7) t[n++] = 0x80 | (v & 0x7F);
    while (n) b8(m, t[--n]);
}

// One MP2K track -> events. Stops at FINE or the GOTO that loops it (reporting where it jumps to).
static void decode_track(const Rom *r, uint32_t start, int ch, EvL *out, EvL *tempo, uint32_t *loopFrom, uint32_t *loopTo,
                         uint32_t *endTime) {
    // first time each position was reached (for the loop start)
    typedef struct { uint32_t addr, time; } Seen;
    Seen *seen = NULL;
    int nseen = 0, capseen = 0;
    uint32_t pos = start, time = 0;
    uint32_t stack[3];
    int sp = 0, rep = 0;
    uint8_t running = 0, key = 60, vel = 127, gtp = 0;
    int8_t keysh = 0;
    int lastKeyOn = -1;
    int tieKeys[16], ntie = 0;
    *loopFrom = *loopTo = 0;
    for (int steps = 0; steps < 2000000; steps++) {
        if (!in_rom(r, pos, 1) || time > 60u * 60 * 24 * 60) break;   // a day of clocks: something's wrong
        if (sp == 0) {
            if (nseen == capseen) {
                capseen = capseen ? capseen * 2 : 1024;
                Seen *ns = realloc(seen, sizeof(Seen) * capseen);
                if (!ns) break;
                seen = ns;
            }
            seen[nseen].addr = pos;     // the first match is the first visit
            seen[nseen].time = time;
            nseen++;
        }
        uint8_t op = *at(r, pos);
        uint8_t cmd;
        if (op < 0x80) {
            if (running < 0xBD) break;
            cmd = running;
        } else {
            pos++;
            cmd = op;
            if (op >= 0xBD) running = op;
        }
#define ARG() (in_rom(r, pos, 1) ? *at(r, pos++) : 0)
#define PEEK() (in_rom(r, pos, 1) ? *at(r, pos) : 0x80)
        if (cmd >= 0x80 && cmd <= 0xB0) { time += sf_gClockTable[cmd - 0x80]; continue; }
        if (cmd == 0xB1) break;
        if (cmd == 0xB2) {
            if (!in_rom(r, pos, 4)) break;
            uint32_t to = rd32(at(r, pos));
            *loopFrom = time;
            *loopTo = 0;
            int found = 0;
            for (int i = 0; i < nseen; i++) if (seen[i].addr == to) { *loopTo = seen[i].time; found = 1; break; }
            if (!found) *loopFrom = 0;
            break;
        }
        if (cmd == 0xB3) {
            if (!in_rom(r, pos, 4) || sp >= 3) break;
            stack[sp++] = pos + 4;
            pos = rd32(at(r, pos));
            continue;
        }
        if (cmd == 0xB4) { if (sp) pos = stack[--sp]; continue; }
        if (cmd == 0xB5) {
            int n = ARG();
            if (!in_rom(r, pos, 4)) break;
            uint32_t to = rd32(at(r, pos));
            if (n == 0 || ++rep < n) { if (n == 0) { pos += 4; continue; } pos = to; }
            else { rep = 0; pos += 4; }
            continue;
        }
        if (cmd == 0xB9) {      // MEMACC: the game's special effects; not carried
            int mop = ARG();
            ARG(); ARG();
            if (mop >= 6) pos += 4;
            continue;
        }
        if (cmd == 0xBA) { add(out, time, 0xB0 | ch, 0x21, ARG()); continue; }
        if (cmd == 0xBB) { int t = ARG(); add(tempo, time, 0xFF, 0x51, t); continue; }
        if (cmd == 0xBC) { keysh = (int8_t)ARG(); continue; }
        if (cmd == 0xBD) { add(out, time, 0xC0 | ch, ARG() & 0x7F, 0); continue; }
        if (cmd == 0xBE) { add(out, time, 0xB0 | ch, 7, ARG() & 0x7F); continue; }
        if (cmd == 0xBF) { add(out, time, 0xB0 | ch, 10, ARG() & 0x7F); continue; }
        if (cmd == 0xC0) { int v = ARG() & 0x7F; add(out, time, 0xE0 | ch, 0, v); continue; }
        if (cmd == 0xC1) { add(out, time, 0xB0 | ch, 0x14, ARG() & 0x7F); continue; }
        if (cmd == 0xC2) { add(out, time, 0xB0 | ch, 0x15, ARG() & 0x7F); continue; }
        if (cmd == 0xC3) { add(out, time, 0xB0 | ch, 0x1A, ARG() & 0x7F); continue; }
        if (cmd == 0xC4) { add(out, time, 0xB0 | ch, 0x01, ARG() & 0x7F); continue; }
        if (cmd == 0xC5) { add(out, time, 0xB0 | ch, 0x16, ARG() & 0x7F); continue; }
        if (cmd == 0xC8) { add(out, time, 0xB0 | ch, 0x18, ARG() & 0x7F); continue; }
        if (cmd == 0xCD) {
            int sub = ARG(), v = ARG();
            if (sub == 8 || sub == 9) { add(out, time, 0xB0 | ch, 0x1E, sub); add(out, time, 0xB0 | ch, 0x1D, v & 0x7F); }
            continue;
        }
        if (cmd == 0xCE) {      // EOT [key]
            int k = -1;
            if (PEEK() < 0x80) { key = ARG(); k = (key + keysh) & 0x7F; }
            for (int i = 0; i < ntie; i++) {
                if (k >= 0 && tieKeys[i] != k) continue;
                add(out, time, 0x80 | ch, tieKeys[i], 0);
                tieKeys[i--] = tieKeys[--ntie];
                if (k >= 0) break;
            }
            continue;
        }
        if (cmd >= 0xCF) {      // TIE or a note
            int clocks = cmd == 0xCF ? 0 : sf_gClockTable[cmd - 0xCF];
            gtp = 0;
            if (PEEK() < 0x80) {
                key = ARG();
                if (PEEK() < 0x80) {
                    vel = ARG();
                    if (cmd != 0xCF && PEEK() < 0x80) gtp = ARG();
                }
            }
            int k = (key + keysh) & 0x7F;
            add(out, time, 0x90 | ch, k, vel ? vel : 1);
            lastKeyOn = k;
            if (cmd == 0xCF) { if (ntie < 16) tieKeys[ntie++] = k; }
            else add(out, time + clocks + gtp, 0x80 | ch, k, 0);
            continue;
        }
        // anything else (0xB6-0xB8, 0xC6, 0xC7, 0xC9-0xCC): one argument in m4a's table
        ARG();
#undef ARG
#undef PEEK
    }
    (void)lastKeyOn;
    for (int i = 0; i < ntie; i++) add(out, time, 0x80 | ch, tieKeys[i], 0);
    *endTime = time;
    free(seen);
}

static void write_track(Buf *mf, EvL *l, const char *name) {
    qsort(l->e, l->n, sizeof(Ev), ev_cmp);
    Buf t = {0};
    if (name) {
        bvar(&t, 0); b8(&t, 0xFF); b8(&t, 0x03); bvar(&t, (uint32_t)strlen(name)); bput(&t, name, strlen(name));
    }
    uint32_t last = 0;
    for (int i = 0; i < l->n; i++) {
        Ev *e = &l->e[i];
        bvar(&t, e->time - last);
        last = e->time;
        if (e->st == 0xFF && e->a == 0x51) {
            uint32_t us = (uint32_t)(60000000.0 / (2.0 * (e->b ? e->b : 1)) + 0.5);
            b8(&t, 0xFF); b8(&t, 0x51); b8(&t, 3); b8(&t, us >> 16); b8(&t, us >> 8); b8(&t, us);
        } else if (e->st == 0xFF && e->a == 0x06) {
            b8(&t, 0xFF); b8(&t, 0x06); b8(&t, 1); b8(&t, e->text[0]);
        } else {
            b8(&t, e->st);
            b8(&t, e->a);
            if ((e->st & 0xF0) != 0xC0) b8(&t, e->b);
        }
    }
    bvar(&t, 0); b8(&t, 0xFF); b8(&t, 0x2F); b8(&t, 0);
    bput(mf, "MTrk", 4);
    uint8_t ln[4] = {(uint8_t)(t.n >> 24), (uint8_t)(t.n >> 16), (uint8_t)(t.n >> 8), (uint8_t)t.n};
    bput(mf, ln, 4);
    bput(mf, t.b, t.n);
    free(t.b);
}

// ------------------------------------------------------------------ instruments
typedef struct {
    const Rom *rom;
    const char *prefix;
    CustomSample **custom;
    int *ncustom;
    CustomGroup **groups;
    int *ngroups;
    CustomSplit **splits;
    int *nsplits;
    uint32_t sampleAddr[INST_MAX_CUSTOM];
    int sampleRef[INST_MAX_CUSTOM];
    int nsamp;
    uint32_t groupAddr[64];
    int groupRef[64];
    int ngrp;
    uint32_t splitAddr[64];
    int splitRef[64];
    int nspl;
} Ctx;

static int match_game_sample(const uint8_t *wd, uint32_t size) {
    uint32_t status = wd[2] | wd[3] << 8, freq = rd32(wd + 4), loop = rd32(wd + 8);
    for (int i = 0; i < SF_NUM_SAMPLES; i++) {
        const SfSample *s = &sf_samples[i];
        if (s->size != size || s->freq != freq || s->loopStart != loop) continue;
        if (((s->flags & 0x40000000u) != 0) != ((status & 0x4000) != 0)) continue;
        if (!memcmp(sf_pcm + s->offset, wd + 16, size)) return i;
    }
    return -1;
}

static int sample_ref(Ctx *c, uint32_t addr) {
    for (int i = 0; i < c->nsamp; i++) if (c->sampleAddr[i] == addr) return c->sampleRef[i];
    int ref = -1;
    if (in_rom(c->rom, addr, 16)) {
        const uint8_t *wd = at(c->rom, addr);
        uint32_t size = rd32(wd + 12);
        if (size < 0x1000000 && in_rom(c->rom, addr, 16 + size)) {
            ref = match_game_sample(wd, size);
            if (ref < 0) {
                CustomSample cs;
                memset(&cs, 0, sizeof cs);
                snprintf(cs.name, sizeof cs.name, "%s_s%02d", c->prefix, *c->ncustom);
                cs.data = (int8_t *)(wd + 16);
                cs.size = size;
                cs.freq = rd32(wd + 4);
                cs.loopStart = rd32(wd + 8);
                cs.loops = (wd[2] | wd[3] << 8) & 0x4000 ? 1 : 0;
                ref = custom_list_add(c->custom, c->ncustom, &cs);
            }
        }
    }
    if (c->nsamp < INST_MAX_CUSTOM) { c->sampleAddr[c->nsamp] = addr; c->sampleRef[c->nsamp++] = ref; }
    return ref;
}

static int wave_ref(Ctx *c, uint32_t addr) {
    if (!in_rom(c->rom, addr, 16)) return 0;
    const uint8_t *w = at(c->rom, addr);
    for (int i = 0; i < SF_NUM_WAVES; i++) if (!memcmp(sf_pcm + sf_waves[i].offset, w, 16)) return i;
    for (int i = 0; i < *c->ncustom; i++)
        if ((*c->custom)[i].isWave && !memcmp((*c->custom)[i].data, w, 16)) return INST_CUSTOM + i;
    CustomSample cs;
    memset(&cs, 0, sizeof cs);
    snprintf(cs.name, sizeof cs.name, "%s_w%02d", c->prefix, *c->ncustom);
    cs.data = (int8_t *)w;
    cs.isWave = 1;
    int ref = custom_list_add(c->custom, c->ncustom, &cs);
    return ref < 0 ? 0 : ref;
}

static int read_group(Ctx *c, uint32_t addr);
static int read_split(Ctx *c, uint32_t addr);

// One ToneData (12 bytes) -> a voice. depth 1: inside a kit (no kits in kits).
static SfVoice read_voice(Ctx *c, const uint8_t *td, int depth) {
    SfVoice v;
    memset(&v, 0, sizeof v);
    v.type = td[0];
    v.key = td[1];
    v.length = td[2];
    v.panSweep = td[3];
    v.attack = td[8];
    v.decay = td[9];
    v.sustain = td[10];
    v.release = td[11];
    v.ref = v.sub = v.ks = -1;
    uint32_t ptr = rd32(td + 4);
    if (v.type & 0xC0) {
        if (depth) return VOICE_PLACEHOLDER;
        v.attack = v.decay = v.sustain = v.release = 0;
        v.sub = (int16_t)read_group(c, ptr);
        if (v.type & 0x40) v.ks = (int16_t)read_split(c, rd32(td + 8));
        if (v.sub < 0 || ((v.type & 0x40) && v.ks < 0)) return VOICE_PLACEHOLDER;
        return v;
    }
    switch (v.type & 7) {
    case 0: v.ref = (int16_t)sample_ref(c, ptr); if (v.ref < 0) return VOICE_PLACEHOLDER; break;
    case 3: v.ref = (int16_t)wave_ref(c, ptr); break;
    default: v.ref = (int16_t)(ptr & 0xFF); break;   // square duty / noise period
    }
    return v;
}

static int read_group(Ctx *c, uint32_t addr) {
    for (int i = 0; i < c->ngrp; i++) if (c->groupAddr[i] == addr) return c->groupRef[i];
    if (!in_rom(c->rom, addr, 128 * 12) || c->ngrp >= 64) return -1;
    SfVoice g[128];
    for (int k = 0; k < 128; k++) g[k] = read_voice(c, at(c->rom, addr + k * 12), 1);
    int ref = -1;
    for (int i = 0; i < SF_NUM_GROUPS && ref < 0; i++) {
        int same = 1;
        for (int k = 0; k < 128 && same; k++) if (!voice_same(&sf_groups[i][k], &g[k])) same = 0;
        if (same) ref = i;
    }
    if (ref < 0) {
        CustomGroup *ng = realloc(*c->groups, sizeof(CustomGroup) * (size_t)(*c->ngroups + 1));
        if (ng) {
            *c->groups = ng;
            CustomGroup *d = &ng[*c->ngroups];
            snprintf(d->name, sizeof d->name, "%s_g%02d", c->prefix, *c->ngroups);
            memcpy(d->v, g, sizeof g);
            ref = INST_CUSTOM + (*c->ngroups)++;
        }
    }
    c->groupAddr[c->ngrp] = addr;
    c->groupRef[c->ngrp++] = ref;
    return ref;
}

static int read_split(Ctx *c, uint32_t addr) {
    for (int i = 0; i < c->nspl; i++) if (c->splitAddr[i] == addr) return c->splitRef[i];
    if (!in_rom(c->rom, addr, 128) || c->nspl >= 64) return -1;
    const uint8_t *t = at(c->rom, addr);
    int ref = -1;
    for (int i = 0; i < SF_NUM_KEYSPLITS && ref < 0; i++) if (!memcmp(sf_keysplits[i], t, 128)) ref = i;
    if (ref < 0) {
        CustomSplit *ns = realloc(*c->splits, sizeof(CustomSplit) * (size_t)(*c->nsplits + 1));
        if (ns) {
            *c->splits = ns;
            CustomSplit *d = &ns[*c->nsplits];
            snprintf(d->name, sizeof d->name, "%s_k%02d", c->prefix, *c->nsplits);
            memcpy(d->t, t, 128);
            ref = INST_CUSTOM + (*c->nsplits)++;
        }
    }
    c->splitAddr[c->nspl] = addr;
    c->splitRef[c->nspl++] = ref;
    return ref;
}

// ------------------------------------------------------------------ the song
int rip_mp2k_song(const uint8_t *rom, size_t romLen, uint32_t header, const char *prefix, Mp2kSong *out, char *err, int errlen) {
    memset(out, 0, sizeof *out);
    Rom r = {rom, romLen};
    if (!in_rom(&r, header, 8)) { snprintf(err, errlen, "the song header isn't in the ROM"); return 0; }
    const uint8_t *h = at(&r, header);
    int ntr = h[0];
    if (ntr < 1 || ntr > 16 || !in_rom(&r, header, 8 + 4u * ntr)) { snprintf(err, errlen, "the song header has %d tracks", ntr); return 0; }
    out->priority = h[2];
    out->reverb = h[3] & 0x80 ? (h[3] & 0x7F) : -1;
    uint32_t tone = rd32(h + 4);
    EvL tempo = {0};
    EvL *tracks = calloc((size_t)ntr, sizeof(EvL));
    uint32_t loopFrom = 0, loopTo = 0, end = 0;
    for (int t = 0; t < ntr; t++) {
        uint32_t lf, lt, et;
        decode_track(&r, rd32(h + 8 + 4 * t), t & 15, &tracks[t], &tempo, &lf, &lt, &et);
        if (lf && !loopFrom) { loopFrom = lf; loopTo = lt; }
        if (et > end) end = et;
    }
    // the conductor track: tempo and the loop markers
    EvL cond = tempo;
    if (loopFrom > loopTo) {
        add(&cond, loopTo, 0xFF, 0x06, 0);
        cond.e[cond.n - 1].text[0] = '[';
        add(&cond, loopFrom, 0xFF, 0x06, 0);
        cond.e[cond.n - 1].text[0] = ']';
        out->hasLoop = 1;
    }
    Buf mf = {0};
    bput(&mf, "MThd", 4);
    uint8_t hd[10] = {0, 0, 0, 6, 0, 1, 0, (uint8_t)(ntr + 1), 0, 24};
    bput(&mf, hd, 10);
    write_track(&mf, &cond, NULL);
    for (int t = 0; t < ntr; t++) {
        char nm[16];
        snprintf(nm, sizeof nm, "Track %d", t + 1);
        write_track(&mf, &tracks[t], nm);
        free(tracks[t].e);
    }
    free(tracks);
    free(cond.e);
    out->mid = mf.b;
    out->midLen = mf.n;
    out->ntracks = ntr;

    // the voicegroup
    Ctx *c = calloc(1, sizeof(Ctx));
    if (!c) { snprintf(err, errlen, "out of memory"); return 0; }
    c->rom = &r;
    c->prefix = prefix;
    c->custom = &out->custom;
    c->ncustom = &out->ncustom;
    c->groups = &out->groups;
    c->ngroups = &out->ngroups;
    c->splits = &out->splits;
    c->nsplits = &out->nsplits;
    // the groups' and samples' refs must resolve while reading (voice_same compares refs only)
    for (int s = 0; s < 128; s++) {
        if (!in_rom(&r, tone + s * 12, 12)) { out->voice[s] = VOICE_PLACEHOLDER; continue; }
        out->voice[s] = read_voice(c, at(&r, tone + s * 12), 0);
    }
    free(c);
    return 1;
}

void rip_mp2k_free(Mp2kSong *s) {
    free(s->mid);
    custom_list_free(&s->custom, &s->ncustom);
    free(s->groups);
    free(s->splits);
    memset(s, 0, sizeof *s);
}
