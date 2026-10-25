// Moemon Music Studio: 2SF (Nintendo DS) rips, played by vio2sf's DeSmuME core
// (third_party/desmume_2sf). The capture hook in SPU.cpp says when a channel starts; the channel's
// registers (sample, format, timer, volume, pan) say the rest.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cstdint>
#include "desmume/NDSSystem.h"
#include "desmume/SPU.h"
extern "C" {
#include "rip.h"
#include "psflib.h"
extern const psf_file_callbacks rip_psf_files;
}

extern void (*spu_tap_keyon)(int chan);
extern int spu_tap_decode(int ch, short **out, int *len, int *loopStart, int *loops);
extern unsigned char spu_tap_read8(unsigned int addr);

volatile bool execute = false;

namespace {

std::vector<uint8_t> g_sbuf;
unsigned g_filled, g_used;
int SInit(int n) { g_sbuf.assign((size_t)n * 2 + 3, 0); g_filled = g_used = 0; return 0; }
void SDe() {}
void SMute() {}
void SUn() {}
void SVol(int) {}
uint32_t SSpace() { return (uint32_t)(g_sbuf.size() - 3) >> 2; }
void SUpd(int16_t *b, uint32_t n) {
    uint32_t nb = n << 2;
    if (nb > g_sbuf.size() - 3) nb = (uint32_t)g_sbuf.size() - 3;
    memcpy(&g_sbuf[0], b, nb);
    g_filled = nb;
    g_used = 0;
}
SoundInterface_struct SIF = {1, "Moemon Music Studio", SInit, SDe, SUpd, SSpace, SMute, SUn, SVol, nullptr, nullptr, nullptr};

struct Nds {
    Rip *rip;
    std::vector<uint8_t> rom;
    int frames, syncType;
    int keyon[16];
    int sample[16];
};
Nds *g_nds;

int ld(void *c, const uint8_t *e, size_t en, const uint8_t *, size_t) {
    Nds *n = (Nds *)c;
    if (en < 8) return 0;
    uint32_t off = e[0] | e[1] << 8 | e[2] << 16 | (uint32_t)e[3] << 24;
    uint32_t sz = e[4] | e[5] << 8 | e[6] << 16 | (uint32_t)e[7] << 24;
    if (sz > en - 8) sz = (uint32_t)(en - 8);
    uint32_t fin = 1;
    while (fin < off + sz) fin <<= 1;
    if (n->rom.size() < (size_t)fin + 10) n->rom.resize((size_t)fin + 10, 0);
    memcpy(&n->rom[off], e + 8, sz);
    return 0;
}

int inf(void *c, const char *name, const char *v) {
    Nds *n = (Nds *)c;
    if (!strcmp(name, "_frames")) n->frames = atoi(v);
    else if (!strcmp(name, "_2sf_sync_type")) n->syncType = atoi(v);
    else rip_psf_tag(n->rip, name, v);
    return 0;
}

void tap_keyon(int ch) {
    if (!g_nds) return;
    g_nds->keyon[ch]++;
    const channel_struct &c = SPU_core->channels[ch];
    RipSample s;
    memset(&s, 0, sizeof s);
    if (c.format == 3) {
        if (ch >= 14) { s.kind = RIPS_NOISE; s.param = 0; s.id = 0x300; }
        else {
            // a PSG square: duty (n+1)/8, made as an 8-step sample so any channel can play it
            static int16_t sq[8][8];
            int duty = c.waveduty & 7;
            for (int i = 0; i < 8; i++) sq[duty][i] = (int16_t)(i >= 7 - duty ? 0x7FFF : -0x7FFF);
            if (duty == 7) for (int i = 0; i < 8; i++) sq[duty][i] = -0x7FFF;
            s.kind = RIPS_PCM;
            s.pcm = sq[duty];
            s.len = 8;
            s.loops = 1;
            s.loopStart = 0;
            s.rate = 0;    // filled per frame from the timer
            s.id = 0x400 + (uint32_t)duty;
            s.param = 0x80 | duty;
        }
        int idx = rip_add_sample(g_nds->rip, &s);
        g_nds->sample[ch] = idx;
        return;
    }
    // id: where and what the sample is (hash of its first bytes and its size)
    uint8_t head[64];
    for (int i = 0; i < 64; i++) head[i] = spu_tap_read8(c.addr + i);
    uint32_t h = rip_hash(head, 64) ^ (c.length * 2654435761u) ^ (c.loopstart * 40503u) ^ (uint32_t)c.format;
    int f = rip_find_sample(g_nds->rip, c.addr, h);
    if (f >= 0) { g_nds->sample[ch] = f; return; }
    short *pcm = nullptr;
    int len = 0, loopStart = 0, loops = 0;
    if (!spu_tap_decode(ch, &pcm, &len, &loopStart, &loops) || len <= 0) { free(pcm); g_nds->sample[ch] = -1; return; }
    s.kind = RIPS_PCM;
    s.pcm = pcm;
    s.len = len;
    s.loopStart = loopStart;
    s.loops = loops;
    s.rate = 16756991 / (65536 - (c.timer ? c.timer : 0xFC00));   // the rate it starts at (for reference)
    s.id = c.addr;
    s.hash = h;
    g_nds->sample[ch] = rip_add_sample(g_nds->rip, &s);
    free(pcm);
}

int nds_render(RipEmu *e, int16_t *buf, int n) {
    (void)e;
    unsigned want = (unsigned)n * 4, got = 0;
    uint8_t *out = (uint8_t *)buf;
    while (got < want) {
        if (g_used < g_filled) {
            unsigned k = g_filled - g_used;
            if (k > want - got) k = want - got;
            memcpy(out + got, &g_sbuf[g_used], k);
            g_used += k;
            got += k;
            continue;
        }
        NDS_exec<false>();
        SPU_Emulate_user();
    }
    return 0;
}

void nds_snapshot(RipEmu *e, RipVF *out) {
    Nds *n = (Nds *)e->ctx;
    float mv = SPU_core->regs.mastervol / 127.0f;
    for (int ch = 0; ch < 16; ch++) {
        RipVF *x = &out[ch];
        memset(x, 0, sizeof *x);
        x->sample = -1;
        const channel_struct &c = SPU_core->channels[ch];
        x->keyon = n->keyon[ch] > 0;
        n->keyon[ch] = 0;
        if (c.status != CHANSTAT_PLAY && !x->keyon) continue;
        if (n->sample[ch] < 0) continue;
        x->sample = (int16_t)n->sample[ch];
        double rate = 16756991.0 / (65536 - c.timer);
        RipSample *s = &n->rip->samples[n->sample[ch]];
        if (s->kind == RIPS_NOISE) x->rate = (float)rate;
        else x->rate = (float)rate;
        if (s->rate == 0 && s->kind == RIPS_PCM) s->rate = 16756991 / 1024;   // the PSG squares' reference rate
        static const int k_shift[4] = {0, 1, 2, 4};
        float v = c.vol / 127.0f / (float)(1 << k_shift[c.volumeDiv & 3]) * mv;
        if (c.status != CHANSTAT_PLAY) v = 0;
        float pan = c.pan / 127.0f;
        x->l = v * (1 - pan);
        x->r = v * pan;
    }
}

void nds_close(RipEmu *e) {
    Nds *n = (Nds *)e->ctx;
    spu_tap_keyon = nullptr;
    MMU_unsetRom();
    NDS_DeInit();
    if (g_nds == n) g_nds = nullptr;
    delete n;
    e->ctx = nullptr;
}

} // namespace

SoundInterface_struct *SNDCoreList[] = {&SIF, &SNDDummy, nullptr};

extern "C" int rip_open_2sf(RipEmu *e, Rip *r, char *err, int errlen) {
    Nds *n = new Nds();
    n->rip = r;
    n->frames = -1;
    for (int i = 0; i < 16; i++) n->sample[i] = -1;
    if (psf_load(r->path, &rip_psf_files, 0x24, ld, n, inf, n, 0, nullptr, nullptr) < 0 || n->rom.empty()) {
        snprintf(err, errlen, "couldn't read %s (or its _lib file)", r->path);
        delete n;
        return 0;
    }
    if (NDS_Init()) { snprintf(err, errlen, "the DS emulator didn't start"); delete n; return 0; }
    SPU_ChangeSoundCore(1, (int)(DESMUME_SAMPLE_RATE / 59.837));
    MMU_unsetRom();
    NDS_SetROM(&n->rom[0], (u32)n->rom.size() - 1);
    gameInfo.loadData((char *)&n->rom[0], (u32)n->rom.size() - 1);
    CommonSettings.use_jit = false;
    NDS_Reset();
    execute = true;
    for (int i = 0; i < n->frames; i++) NDS_exec<false>();
    CommonSettings.rigorous_timing = true;
    CommonSettings.spu_advanced = true;
    CommonSettings.advanced_timing = true;
    g_nds = n;
    spu_tap_keyon = tap_keyon;
    snprintf(r->console, sizeof r->console, "Nintendo DS");
    e->ctx = n;
    e->rip = r;
    e->rate = 32728;            // the DS mixes at 32728.5 Hz (the core's own rate)
    e->nvoices = 16;
    e->render = nds_render;
    e->snapshot = nds_snapshot;
    e->close = nds_close;
    return 1;
}
