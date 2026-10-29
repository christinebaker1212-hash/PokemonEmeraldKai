// Moemon Music Studio: GSF (Game Boy Advance) rips, played by VBA-M's core from in_xsf
// (third_party/vbam_gsf). Games on the game's own sound engine (MP2K) are read straight from the
// ROM by rip_mp2k.c; the emulator gives the original sound.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cstdint>
#include "gba/Globals.h"
#include "gba/GBA.h"
#include "gba/Sound.h"
#include "common/SoundDriver.h"
extern "C" {
#include "rip.h"
#include "psflib.h"
extern const psf_file_callbacks rip_psf_files;
}

namespace {
std::vector<uint8_t> g_rom;
unsigned g_entry;
int g_first;
std::vector<uint8_t> g_buf;
uint32_t g_blen, g_bfil, g_bcur;
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
        if (n > 0) { memcpy(&g_buf[g_bfil], w, (size_t)n); g_bfil += (uint32_t)n; }
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

int gsf_render(RipEmu *e, int16_t *buf, int n) {
    (void)e;
    unsigned want = (unsigned)n * 4, got = 0;
    uint8_t *out = (uint8_t *)buf;
    while (got < want) {
        if (g_bcur >= g_bfil) { g_bcur = g_bfil = 0; CPULoop(250000); continue; }
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
    if (!e->rip->mp2kHeader && g_frame % 15 == 0 && g_frame <= 600) find_mp2k_player(e->rip);
}

void gsf_close(RipEmu *e) {
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
    soundSetEnable(0x30F);
    CPUInit();
    CPUReset();
    snprintf(r->console, sizeof r->console, "Game Boy Advance");
    e->ctx = &g_rom;
    e->rip = r;
    e->rate = 44100;
    e->nvoices = 0;
    e->render = gsf_render;
    e->snapshot = gsf_snapshot;
    e->close = gsf_close;
    return 1;
}
