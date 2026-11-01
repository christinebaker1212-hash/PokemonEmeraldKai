// Moemon Music Studio: SPC (Super Nintendo) rips, played by Game_Music_Emu's SNES sound emulation
// (third_party/gme: the SPC700 and the S-DSP). The S-DSP plays 8 voices of BRR samples from the sound RAM:
// each voice's sample (the directory entry its SRCN names: start and loop address), pitch (0x1000 = the
// sample at 32 kHz), envelope and volume are read every game frame, and each key-on as it happens. The
// samples are decoded from the sound RAM exactly as the DSP decodes them (BRR: 9-byte blocks of 16
// samples, a shift and one of four prediction filters).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "Spc_Emu.h"
extern "C" {
#include "rip.h"
}

namespace {

enum { SPC_RATE = 32000, SPC_VOICES = 8 };

struct Spc {
    Rip *rip;
    Spc_Emu emu;
    int kon[SPC_VOICES];
};

Spc *g_spc;

void tap_kon(Spc_Dsp *, int voice) {
    if (g_spc && voice >= 0 && voice < SPC_VOICES) g_spc->kon[voice]++;
}

int spc_render(RipEmu *e, int16_t *buf, int n) {
    Spc *s = (Spc *)e->ctx;
    while (n > 0) {
        int k = n > 2048 ? 2048 : n;
        if (s->emu.play(k * 2, buf)) memset(buf, 0, (size_t)k * 4);
        buf += k * 2;
        n -= k;
    }
    return 0;
}

// One BRR block into 16 samples, as the S-DSP decodes it (Spc_Dsp.cpp decode_brr): p[0] / p[1] are the
// last two outputs, kept between blocks.
void brr_block(const uint8_t *blk, int *p, int16_t *out) {
    // 0: >>1  1: <<0  2: <<1 ... 12: <<11  13-15: >>4 <<11
    static const unsigned char shifts[32] = {13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 16, 16, 16,
                                             0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11};
    int header = blk[0], scale = header >> 4, filter = header & 0x0C;
    int rs = shifts[scale], ls = shifts[scale + 16];
    for (int i = 0; i < 16; i++) {
        int nib = (blk[1 + i / 2] >> ((i & 1) ? 0 : 4)) & 15;
        int s = int16_t(uint16_t((int16_t)(nib << 12) >> rs) << ls);
        int p1 = p[0], p2 = p[1] >> 1;
        if (filter >= 8) {
            s += p1;
            s -= p2;
            if (filter == 8) { s += p2 >> 4; s += (p1 * -3) >> 6; }
            else { s += (p1 * -13) >> 7; s += (p2 * 3) >> 4; }
        } else if (filter) {
            s += p1 >> 1;
            s += (-p1) >> 5;
        }
        if ((int16_t)s != s) s = (s >> 31) ^ 0x7FFF;   // clamp to 16 bits
        s = (int16_t)(s * 2);
        p[1] = p[0];
        p[0] = s;
        out[i] = (int16_t)s;
    }
}

// A sample from the sound RAM: from its start to the block with the end flag; looping (the end block's
// loop flag) back to the loop address, which is usually inside it, or else decoded after it.
int decode_sample(const uint8_t *ram, int start, int loop, std::vector<int16_t> &pcm, int *loopStart) {
    int p[2] = {0, 0};
    int16_t blk[16];
    int addr = start, n = 0, loops = 0;
    std::vector<int> blockAddr;
    *loopStart = 0;
    for (;;) {
        if (addr + 9 > 0x10000 || n > 0x10000 / 9) break;
        brr_block(ram + addr, p, blk);
        blockAddr.push_back(addr);
        pcm.insert(pcm.end(), blk, blk + 16);
        n++;
        int hdr = ram[addr];
        addr += 9;
        if (hdr & 1) { loops = (hdr & 2) != 0; break; }
    }
    if (!loops) return 0;
    for (size_t i = 0; i < blockAddr.size(); i++)
        if (blockAddr[i] == loop) { *loopStart = (int)i * 16; return 1; }
    // the loop is elsewhere: decode it after the sample, once
    *loopStart = (int)pcm.size();
    addr = loop;
    for (int k = 0; k < 0x10000 / 9; k++) {
        if (addr + 9 > 0x10000) break;
        brr_block(ram + addr, p, blk);
        pcm.insert(pcm.end(), blk, blk + 16);
        int hdr = ram[addr];
        addr += 9;
        if (hdr & 1) break;
    }
    return 1;
}

void spc_snapshot(RipEmu *e, RipVF *out) {
    Spc *s = (Spc *)e->ctx;
    Spc_Dsp const &dsp = s->emu.tap_apu().tap_dsp();
    const uint8_t *regs = dsp.tap_regs(), *ram = dsp.tap_ram();
    int dir = regs[0x5D] << 8, non = regs[0x3D], eon = regs[0x4D];
    for (int v = 0; v < SPC_VOICES; v++) {
        RipVF *x = &out[v];
        memset(x, 0, sizeof *x);
        x->sample = -1;
        x->keyon = s->kon[v] > 0;
        s->kon[v] = 0;
        const uint8_t *vr = regs + v * 0x10;
        int env = dsp.tap_env(v);
        if (env <= 0 && !x->keyon) continue;
        int srcn = vr[4], pitch = (vr[2] | vr[3] << 8) & 0x3FFF;
        RipSample rs;
        memset(&rs, 0, sizeof rs);
        if (non >> v & 1) {
            // noise: the generator's clock (FLG's low bits) is the voice's pitch
            static const int k_noiseRate[32] = {0, 16, 21, 25, 31, 42, 50, 63, 83, 100, 125, 167, 200, 250, 333, 400,
                                                500, 667, 800, 1000, 1300, 1600, 2000, 2700, 3200, 4000, 5300, 6400,
                                                8000, 10700, 16000, 32000};
            rs.kind = RIPS_NOISE;
            rs.id = 0x300;
            x->rate = (float)k_noiseRate[regs[0x6C] & 31];
        } else {
            int entry = dir + srcn * 4;
            int start = ram[entry & 0xFFFF] | ram[(entry + 1) & 0xFFFF] << 8;
            int loop = ram[(entry + 2) & 0xFFFF] | ram[(entry + 3) & 0xFFFF] << 8;
            std::vector<int16_t> pcm;
            int ls = 0;
            int loops = decode_sample(ram, start, loop, pcm, &ls);
            if (pcm.empty()) continue;
            rs.kind = RIPS_PCM;
            rs.pcm = pcm.data();
            rs.len = (int)pcm.size();
            rs.rate = SPC_RATE;
            rs.loops = loops;
            rs.loopStart = ls;
            rs.id = (uint32_t)start | (uint32_t)loop << 16;
            x->sample = (int16_t)rip_add_sample(s->rip, &rs);
            x->rate = pitch * (float)SPC_RATE / 4096.0f;
        }
        if (rs.kind == RIPS_NOISE) x->sample = (int16_t)rip_add_sample(s->rip, &rs);
        // the envelope (11 bits) times each side's volume (signed 8 bits: a negative one is the same loudness)
        float lev = env / 2047.0f;
        x->l = lev * abs((int8_t)vr[0]) / 128.0f;
        x->r = lev * abs((int8_t)vr[1]) / 128.0f;
        if (eon >> v & 1) x->flags |= RIPV_REVERB;
    }
    // the echo: how much of the song goes to it, as the reverb amount
    int evol = abs((int8_t)regs[0x2C]) + abs((int8_t)regs[0x3C]), efb = abs((int8_t)regs[0x0D]);
    float rv = eon ? (evol / 256.0f) * (0.5f + efb / 256.0f) : 0;
    if (rv > s->rip->reverb) s->rip->reverb = rv > 1 ? 1 : rv;
}

void spc_close(RipEmu *e) {
    Spc *s = (Spc *)e->ctx;
    Spc_Dsp::tap_kon = 0;
    if (g_spc == s) g_spc = 0;
    delete s;
    e->ctx = 0;
}

} // namespace

extern "C" int rip_open_spc(RipEmu *e, Rip *r, char *err, int errlen) {
    Spc *s = new Spc();
    memset(s->kon, 0, sizeof s->kon);
    s->rip = r;
    s->emu.set_sample_rate(SPC_RATE);    // the DSP's own rate: no resampling
    const char *e1 = s->emu.load_file(r->path);
    if (e1) { snprintf(err, errlen, "couldn't open %s: %s", r->path, e1); delete s; return 0; }
    track_info_t info;
    if (!s->emu.track_info(&info, 0)) {
        if (info.song[0]) snprintf(r->title, sizeof r->title, "%s", info.song);
        if (info.game[0]) snprintf(r->game, sizeof r->game, "%s", info.game);
        if (info.author[0]) snprintf(r->artist, sizeof r->artist, "%s", info.author);
        if (info.copyright[0]) snprintf(r->copyright, sizeof r->copyright, "%s", info.copyright);
        if (info.dumper[0]) snprintf(r->ripper, sizeof r->ripper, "%s", info.dumper);
        if (info.length > 0) r->length = info.length / 1000.0;
    }
    if (r->length <= 0) r->length = 0;
    g_spc = s;
    Spc_Dsp::tap_kon = tap_kon;
    s->emu.ignore_silence(true);
    if (s->emu.start_track(0)) { snprintf(err, errlen, "couldn't start %s", r->path); g_spc = 0; Spc_Dsp::tap_kon = 0; delete s; return 0; }
    snprintf(r->console, sizeof r->console, "Super Nintendo");
    r->nativeRate = SPC_RATE;
    e->ctx = s;
    e->rip = r;
    e->rate = SPC_RATE;
    e->nvoices = SPC_VOICES;
    e->render = spc_render;
    e->snapshot = spc_snapshot;
    e->close = spc_close;
    return 1;
}
