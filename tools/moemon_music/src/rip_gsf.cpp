// Moemon Music Studio: GSF (Game Boy Advance) rips, played by VBA-M's core from in_xsf
// (third_party/vbam_gsf). Games on the game's own sound engine (MP2K) are read straight from the
// ROM by rip_mp2k.c; the emulator gives the original sound.
//
// Games with their own driver: Konami's (Yu-Gi-Oh! Worldwide Edition and others from KCEJ) is found by its
// mixer, which it copies to IWRAM: six sample channels of 16 bytes {position, bytes left, 4.12 step, fraction,
// sample number (bit 15: the second table), flags (0x80 playing, 0x40 looping), volume 0-15}, mixed at
// timer 0's rate into both FIFOs (three channels each, both FIFOs on both sides: mono), the samples
// {base step, length, loop start or -1, signed 8-bit data} in two tables. Its Game Boy channels are the
// hardware's (read from the APU like a GBS). The rip is then RIP_GBA: notes and instruments like any console.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cstdint>
#include <cmath>
#include "gba/Globals.h"
#include "gba/GBA.h"
#include "gba/Sound.h"
#include "common/SoundDriver.h"
#include "apu/Gb_Apu.h"
extern "C" {
#include "rip.h"
#include "engine.h"
#include "psflib.h"
extern const psf_file_callbacks rip_psf_files;
}
const Gb_Apu *soundTapApu();
int gsfTimer0Reload();

namespace {
std::vector<uint8_t> g_rom;
unsigned g_entry;
int g_first;
std::vector<uint8_t> g_buf;
uint32_t g_blen, g_bfil, g_bcur;
uint64_t g_written;          // bytes the emulator has made since the start (where its time is)
Rip *g_rip;

class Drv : public SoundDriver {
public:
    bool init(long r) override { g_blen = (uint32_t)((r / 10) << 2); g_buf.assign(g_blen, 0); g_bfil = g_bcur = 0; return true; }
    void pause() override {}
    void reset() override {}
    void resume() override {}
    void setThrottle(unsigned short) override {}
    void write(uint16_t *w, int n) override {
        if ((uint32_t)n > g_blen - g_bfil) n = (int)(g_blen - g_bfil);
        if (n > 0) { memcpy(&g_buf[g_bfil], w, (size_t)n); g_bfil += (uint32_t)n; g_written += (uint32_t)n; }
    }
};

int ld(void *, const uint8_t *e, size_t en, const uint8_t *, size_t) {
    if (en < 12) return 0;
    uint32_t ent, off, sz;
    memcpy(&ent, e, 4);
    memcpy(&off, e + 4, 4);
    memcpy(&sz, e + 8, 4);
    off &= 0x1FFFFFF;
    if (sz > en - 12) sz = (uint32_t)(en - 12);
    if (g_first) { g_entry = ent; g_first = 0; }
    uint32_t fin = 1;
    while (fin < off + sz) fin <<= 1;
    if (g_rom.size() < (size_t)off + sz) g_rom.resize((size_t)fin + 10, 0);
    memcpy(&g_rom[off], e + 12, sz);
    return 0;
}

int inf(void *c, const char *name, const char *v) { rip_psf_tag((Rip *)c, name, v); return 0; }


// ---- Konami's driver -------------------------------------------------------------------------
const int K_PCM = 6, K_NV = K_PCM + 4;
const int K_CHUNK = 4096;            // cycles the CPU runs between looks (a quarter of a millisecond)
// Levels against the Game Boy squares, as the studio's engine plays them: what each kind of channel alone needed
// to match the same channel alone in the original (median of 11 songs; the samples' 1.5x less, the wave's 0.9x)
const float K_LEVEL_PCM = 0.67f, K_LEVEL_WAVE = 0.6f;
struct KVoice { int16_t sample; uint8_t keyon; float rate, l, r; };
struct KRec { long t; KVoice v[K_NV]; };
struct Konami {
    bool on;
    uint32_t chan, tab[2];
    uint32_t rem[K_PCM];
    int16_t idx[K_PCM];
    uint8_t active[K_PCM];
    std::vector<int> cache[2];       // table entry -> RipSample (-1 = not made yet, -2 = not a sample)
    int psgTrig[4];
    std::vector<KRec> recs;
    KVoice last[K_NV];
    bool haveLast;
} g_k;

uint32_t rom32(uint32_t a) {
    a &= 0x1FFFFFF;
    return a + 4 <= g_rom.size() ? (uint32_t)(g_rom[a] | g_rom[a + 1] << 8 | g_rom[a + 2] << 16 | (uint32_t)g_rom[a + 3] << 24) : 0;
}
uint32_t iw32(uint32_t a) { a &= 0x7FFC; return internalRAM[a] | internalRAM[a + 1] << 8 | internalRAM[a + 2] << 16 | (uint32_t)internalRAM[a + 3] << 24; }
uint16_t iw16(uint32_t a) { a &= 0x7FFE; return (uint16_t)(internalRAM[a] | internalRAM[a + 1] << 8); }

// The mixer's first instructions (stmdb sp!,{lr}; cmp r2,#0x800; beq; mov r0,#0; ldrsb r1,[r5,r0]!; mul r1,r8,r1;
// asr r1,r1,#4; subs r4,r4,#1); the two sample tables follow it at +0xD4, and the literal pool before it holds
// the channels, the mix state and buffer, DMA1, and the mixer's IWRAM address twice.
const uint8_t k_mixer[32] = {0x00, 0x40, 0x2D, 0xE9, 0x02, 0x0B, 0x52, 0xE3, 0x22, 0x00, 0x00, 0x0A, 0x00, 0x00, 0xA0, 0xE3,
                              0xD0, 0x10, 0xB5, 0xE1, 0x98, 0x01, 0x01, 0xE0, 0x41, 0x12, 0xA0, 0xE1, 0x01, 0x40, 0x54, 0xE2};

bool konami_find() {
    g_k.on = false;
    for (size_t a = 0x40; a + 0x100 < g_rom.size(); a += 4) {
        if (memcmp(&g_rom[a], k_mixer, sizeof k_mixer)) continue;
        uint32_t t0 = rom32((uint32_t)a + 0xD4), t1 = rom32((uint32_t)a + 0xD8);
        if ((t0 >> 24) != 8 || (t1 >> 24) != 8) continue;
        for (uint32_t b = (uint32_t)a - 0x18; b + 0x14 <= a && b >= 0x40; b -= 4) {
            uint32_t ch = rom32(b), mix = rom32(b + 0x10);
            if ((ch >> 24) == 3 && (mix >> 24) == 3 && rom32(b + 0x14) == mix) {
                g_k.on = true;
                g_k.chan = ch;
                g_k.tab[0] = t0;
                g_k.tab[1] = t1;
                break;
            }
            if (b < (uint32_t)a - 0x40) break;
        }
        if (g_k.on) break;
    }
    g_k.cache[0].assign(1024, -1);
    g_k.cache[1].assign(1024, -1);
    memset(g_k.rem, 0, sizeof g_k.rem);
    memset(g_k.active, 0, sizeof g_k.active);
    memset(g_k.psgTrig, 0, sizeof g_k.psgTrig);
    g_k.recs.clear();
    g_k.haveLast = false;
    return g_k.on;
}

double konami_mix_rate() {
    int rl = gsfTimer0Reload();
    return rl > 0 && rl < 0xFFFF ? 16777216.0 / (0x10000 - rl) : 13379.0;
}

int konami_sample(int t, int i) {
    if (i < 0 || i >= (int)g_k.cache[t].size()) return -1;
    if (g_k.cache[t][i] != -1) return g_k.cache[t][i] < 0 ? -1 : g_k.cache[t][i];
    g_k.cache[t][i] = -2;
    uint32_t h = rom32(g_k.tab[t] + 4 * (uint32_t)i);
    if ((h >> 24) != 8) return -1;
    uint32_t base = rom32(h), len = rom32(h + 4), loop = rom32(h + 8);
    uint32_t off = (h & 0x1FFFFFF) + 12;
    if (!len || len > 0x400000 || off + len > g_rom.size() || !base || base > 0x10000) return -1;
    RipSample s;
    memset(&s, 0, sizeof s);
    s.kind = RIPS_PCM;
    s.pcm = (int16_t *)malloc(sizeof(int16_t) * len);
    if (!s.pcm) return -1;
    for (uint32_t k = 0; k < len; k++) s.pcm[k] = (int16_t)((int8_t)g_rom[off + k] * 256);
    s.len = (int)len;
    s.rate = (int)lround(base / 4096.0 * konami_mix_rate());
    if (loop != 0xFFFFFFFFu && loop < len) { s.loops = 1; s.loopStart = (int)loop; }
    s.id = h;
    s.hash = rip_hash(&g_rom[off], len < 4096 ? (int)len : 4096) ^ len;
    int idx = rip_add_sample(g_rip, &s);
    free(s.pcm);
    g_k.cache[t][i] = idx;
    return idx;
}

void konami_trigger(Gb_Apu *, int osc) { if (osc >= 0 && osc < 4) g_k.psgTrig[osc]++; }

// What every voice plays now; a record when anything changed (time: the sample of the output it's heard at).
void konami_look() {
    KVoice v[K_NV];
    memset(v, 0, sizeof v);
    double rate = konami_mix_rate();
    int cnt = ioMem[0x82] | ioMem[0x83] << 8;
    for (int c = 0; c < K_PCM; c++) {
        uint32_t b = g_k.chan + 16 * (uint32_t)c;
        uint32_t rem = iw32(b + 4);
        int step = iw16(b + 8);
        int16_t idx = (int16_t)iw16(b + 12);
        int fl = internalRAM[(b + 14) & 0x7FFF], vol = internalRAM[(b + 15) & 0x7FFF];
        KVoice *x = &v[c];
        x->sample = -1;
        int act = (fl & 0x80) != 0;
        if (act) {
            int t = idx < 0, i = idx & 0x7FFF;
            x->sample = (int16_t)konami_sample(t, i);
            bool kon = !g_k.active[c] || idx != g_k.idx[c];
            if (!kon && rem > g_k.rem[c]) {
                // more left than before: a new note, or the loop went round (then no more than the loop is left)
                uint32_t h = rom32(g_k.tab[t] + 4 * (uint32_t)i), len = rom32(h + 4), loop = rom32(h + 8);
                kon = loop == 0xFFFFFFFFu || loop >= len || rem > len - loop;
            }
            x->keyon = kon;
            x->rate = (float)(step / 4096.0 * rate);
            float dma = (cnt >> (c < 3 ? 2 : 3) & 1) ? 1.0f : 0.5f;
            x->l = x->r = vol ? (vol + 1) / 16.0f * dma * K_LEVEL_PCM : 0.0f;
        }
        g_k.active[c] = (uint8_t)act;
        g_k.idx[c] = idx;
        g_k.rem[c] = rem;
    }
    // the Game Boy channels
    const Gb_Apu *apu = soundTapApu();
    if (apu) {
        const uint8_t *regs = apu->tap_regs();
        int nr50 = regs[0x14], nr51 = regs[0x15];
        static const float k_ratio[4] = {0.25f, 0.5f, 1.0f, 0.0f};
        float ratio = k_ratio[cnt & 3];
        float ml = (((nr50 >> 4) & 7) + 1) / 8.0f * ratio, mr = ((nr50 & 7) + 1) / 8.0f * ratio;
        for (int i = 0; i < 4; i++) {
            const Gb_Osc *o = apu->tap_osc(i);
            KVoice *x = &v[K_PCM + i];
            x->sample = -1;
            x->keyon = g_k.psgTrig[i] > 0;
            g_k.psgTrig[i] = 0;
            int env = i == 2 ? 0 : ((const Gb_Env *)o)->volume;
            int playing = o->enabled && (i == 2 ? (o->regs[0] & 0x80) && (o->regs[2] & 0xE0) : env > 0) &&
                          (!(o->regs[4] & 0x40) || o->length_ctr);
            if (!(regs[0x16] & 0x80)) playing = 0;   // NR52: the sound circuits off
            if (!playing && !x->keyon) continue;
            RipSample s;
            memset(&s, 0, sizeof s);
            int fr = (o->regs[4] & 7) * 0x100 + o->regs[3];
            float vol;
            if (i < 2) {
                s.kind = RIPS_SQUARE;
                s.param = (o->regs[1] >> 6) | (i == 0 ? 0x10 : 0x20);
                s.id = 0x100 + (uint32_t)s.param;
                x->rate = 131072.0f / (2048 - fr);
                vol = env / 15.0f;
            } else if (i == 2) {
                s.kind = RIPS_WAVE;
                const uint8_t *w = apu->tap_wave(), *w2 = apu->tap_wave_other();
                x->rate = 65536.0f / (2048 - fr);
                if (apu->tap_wave_64() && memcmp(w, w2, 16)) {
                    // both banks as one 64-step wave (the one heard first): an octave down, two steps made one
                    uint8_t n64[64];
                    for (int k = 0; k < 16; k++) { n64[k * 2] = w[k] >> 4; n64[k * 2 + 1] = w[k] & 15; }
                    for (int k = 0; k < 16; k++) { n64[32 + k * 2] = w2[k] >> 4; n64[33 + k * 2] = w2[k] & 15; }
                    for (int k = 0; k < 32; k++) s.wave[k] = (uint8_t)((n64[k * 2] + n64[k * 2 + 1] + 1) / 2);
                    x->rate *= 0.5f;
                } else {
                    // (64 steps of two equal banks are the bank twice: the same wave at this rate)
                    for (int k = 0; k < 16; k++) { s.wave[k * 2] = w[k] >> 4; s.wave[k * 2 + 1] = w[k] & 15; }
                }
                s.id = 0x200;
                int code = o->regs[2] >> 5 & 7;
                static const float k_wv[8] = {0, 1, 0.5f, 0.25f, 0.75f, 0.75f, 0.75f, 0.75f};
                vol = k_wv[code] * K_LEVEL_WAVE;
            } else {
                s.kind = RIPS_NOISE;
                int nr43 = o->regs[3];
                s.param = (nr43 >> 3) & 1;
                s.id = 0x300 + (uint32_t)s.param;
                int r = nr43 & 7, sh = nr43 >> 4;
                x->rate = 524288.0f / (r ? r : 0.5f) / (float)(2 << sh);
                vol = env / 15.0f;
            }
            if (!playing) vol = 0;
            x->sample = (int16_t)rip_add_sample(g_rip, &s);
            x->l = (nr51 >> (4 + i) & 1) ? vol * ml : 0;
            x->r = (nr51 >> i & 1) ? vol * mr : 0;
        }
    }
    bool same = g_k.haveLast;
    for (int i = 0; i < K_NV && same; i++)
        same = !v[i].keyon && v[i].sample == g_k.last[i].sample && v[i].rate == g_k.last[i].rate && v[i].l == g_k.last[i].l &&
               v[i].r == g_k.last[i].r;
    if (same) return;
    KRec rec;
    rec.t = (long)(g_written / 4) + (long)((SOUND_CLOCK_TICKS - soundTicks) * 44100.0 / 16777216.0);
    memcpy(rec.v, v, sizeof v);
    g_k.recs.push_back(rec);
    memcpy(g_k.last, v, sizeof v);
    g_k.haveLast = true;
}

// Each game frame: the voices as recorded nearest its start, and the notes started since the last. The sample
// channels are heard a mixing buffer after the driver sets them (the mixer works ahead of the FIFO).
void konami_finalize(RipEmu *e) {
    Rip *r = e->rip;
    double fps = (double)ENG_RATE / ENG_FRAME_SAMPLES;
    // the driver's DMA buffers: 400 samples a half (0x320 bytes for both halves), played while the other is mixed
    long delay = getenv("MMS_GSF_DELAY") ? atol(getenv("MMS_GSF_DELAY")) : (long)lround(400.0 / konami_mix_rate() * e->rate);
    for (int group = 0; group < 2; group++) {
        int v0 = group ? K_PCM : 0, v1 = group ? K_NV : K_PCM;
        long d = group ? 0 : delay;
        KVoice cur[K_NV];
        for (int i = 0; i < K_NV; i++) { memset(&cur[i], 0, sizeof cur[i]); cur[i].sample = -1; }
        size_t ri = 0;
        for (int f = 0; f < r->nframes; f++) {
            // a change goes on the frame whose start is nearest to it
            long tMid = (long)llround(((double)f + 0.5) * e->rate / fps);
            uint8_t kon[K_NV] = {0};
            while (ri < g_k.recs.size() && g_k.recs[ri].t + d < tMid) {
                for (int i = v0; i < v1; i++) {
                    cur[i] = g_k.recs[ri].v[i];
                    if (cur[i].keyon) kon[i] = 1;
                }
                ri++;
            }
            for (int i = v0; i < v1; i++) {
                if (getenv("MMS_GSF_VOICES") && !(strtol(getenv("MMS_GSF_VOICES"), NULL, 0) >> i & 1)) cur[i].sample = -1;   // tests
                RipVF *x = &r->vf[(size_t)f * r->nvoices + i];
                x->sample = cur[i].sample;
                x->keyon = kon[i] && cur[i].sample >= 0;
                x->flags = 0;
                x->rate = cur[i].rate;
                x->l = cur[i].l;
                x->r = cur[i].r;
            }
        }
    }
    g_k.recs.clear();
    g_k.recs.shrink_to_fit();
}

int gsf_render(RipEmu *e, int16_t *buf, int n) {
    (void)e;
    unsigned want = (unsigned)n * 4, got = 0;
    uint8_t *out = (uint8_t *)buf;
    while (got < want) {
        if (g_bcur >= g_bfil) {
            g_bcur = g_bfil = 0;
            if (!g_k.on) { CPULoop(250000); continue; }
            for (int k = 0; k < 64 && !g_bfil; k++) { CPULoop(K_CHUNK); konami_look(); }
            continue;
        }
        unsigned k = g_bfil - g_bcur;
        if (k > want - got) k = want - got;
        memcpy(out + got, &g_buf[g_bcur], k);
        g_bcur += k;
        got += k;
    }
    return 0;
}

int g_frame;

uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

// MP2K's MusicPlayerInfo has ident 'Smsh' (ID_NUMBER) at 0x34 and the song header at 0. The player
// with the most tracks playing a song from the ROM is the music.
void find_mp2k_player(Rip *r) {
    const uint8_t *areas[2] = {workRAM, internalRAM};
    const uint32_t sizes[2] = {0x40000, 0x8000};
    uint32_t best = 0;
    int bestTracks = 0;
    for (int a = 0; a < 2; a++) {
        const uint8_t *m = areas[a];
        for (uint32_t o = 0x34; o + 4 <= sizes[a]; o += 4) {
            if (rd32(m + o) != 0x68736D53u) continue;
            uint32_t base = o - 0x34;
            uint32_t header = rd32(m + base), status = rd32(m + base + 4);
            int tracks = m[base + 8];
            if ((header >> 24) != 8 && (header >> 24) != 9) continue;
            if (!(status & 0xFFFF) || tracks <= 0) continue;
            if (tracks > bestTracks) { bestTracks = tracks; best = header; }
        }
    }
    if (best) r->mp2kHeader = best;
}

void gsf_snapshot(RipEmu *e, RipVF *) {
    g_frame++;
    // look a few times in the first seconds (the song may start after a short wait)
    if (g_k.on) return;   // konami_finalize() fills the frames
    if (!e->rip->mp2kHeader && g_frame % 15 == 0 && g_frame <= 600) find_mp2k_player(e->rip);
}

void gsf_close(RipEmu *e) {
    Gb_Apu::tap_trigger = nullptr;
    g_k.recs.clear();
    // keep the ROM for reading the song
    if (e->rip && e->rip->mp2kHeader && !e->rip->rom && !g_rom.empty()) {
        e->rip->rom = (uint8_t *)malloc(g_rom.size());
        if (e->rip->rom) { memcpy(e->rip->rom, &g_rom[0], g_rom.size()); e->rip->romLen = g_rom.size(); }
    }
    soundShutdown();
    g_rom.clear();
    g_rip = nullptr;
    e->ctx = nullptr;
}
} // namespace

int mapgsf(uint8_t *d, int l, int &s) {
    if ((size_t)l > g_rom.size()) l = (int)g_rom.size();
    if (l) memcpy(d, &g_rom[0], (size_t)l);
    s = l;
    return l;
}
SoundDriver *systemSoundInit() { return new Drv(); }

// The loaded ROM (for rip_mp2k.c): valid until the rip closes.
extern "C" const uint8_t *rip_gsf_rom(size_t *len) { *len = g_rom.size(); return g_rom.empty() ? nullptr : &g_rom[0]; }
// The GBA's work RAM (256 KB at 0x02000000) and internal RAM (32 KB at 0x03000000).
extern "C" const uint8_t *rip_gsf_ewram(void) { return workRAM; }
extern "C" const uint8_t *rip_gsf_iwram(void) { return internalRAM; }

extern "C" int rip_open_gsf(RipEmu *e, Rip *r, char *err, int errlen) {
    g_rom.clear();
    g_first = 1;
    g_entry = 0;
    if (psf_load(r->path, &rip_psf_files, 0x22, ld, nullptr, inf, r, 0, nullptr, nullptr) < 0 || g_rom.empty()) {
        snprintf(err, errlen, "couldn't read %s (or its _lib file)", r->path);
        return 0;
    }
    g_rip = r;
    g_frame = 0;
    cpuIsMultiBoot = (g_entry >> 24) == 2;
    CPULoadRom();
    soundSetSampleRate(44100);
    soundInit();
    soundReset();
    soundSetEnable(getenv("MMS_GSF_CHANNELS") ? (int)strtol(getenv("MMS_GSF_CHANNELS"), NULL, 0) : 0x30F);   // tests: hear some channels
    CPUInit();
    CPUReset();
    snprintf(r->console, sizeof r->console, "Game Boy Advance");
    g_written = 0;
    if (konami_find()) {
        // its own driver: the voices are read as it plays (RIP_GBA), the sample channels at the mixer's rate
        r->kind = RIP_GBA;
        e->nvoices = K_NV;
        e->finalize = konami_finalize;
        Gb_Apu::tap_trigger = konami_trigger;
        snprintf(r->console, sizeof r->console, "Game Boy Advance (Konami)");
    }
    e->ctx = &g_rom;
    e->rip = r;
    e->rate = 44100;
    if (!g_k.on) e->nvoices = 0;
    e->render = gsf_render;
    e->snapshot = gsf_snapshot;
    e->close = gsf_close;
    return 1;
}
