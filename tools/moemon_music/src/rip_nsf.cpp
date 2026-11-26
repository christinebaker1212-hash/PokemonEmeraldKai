// Moemon Music Studio: NSF / NSFe (NES) rips, played by Game_Music_Emu (third_party/gme). The NES's
// 2A03 has two squares with the Game Boy's four duties, a triangle, noise and the DMC (1-bit delta
// samples); the squares become the GBA's Game Boy squares, the triangle its wave channel playing a
// triangle (the same 32 steps of 4 bits), the noise its noise, and DMC samples are samples. The
// cartridges' extra sound chips are read from their registers too: VRC6 (two pulses of 16 duties and a
// sawtooth), Namco 163 (up to eight wavetables), Sunsoft 5B (three squares), the Famicom Disk System
// (a 64-step wavetable), MMC5 (two more 2A03 squares) and VRC7 (six FM voices, as a pure tone per patch).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include "Nsf_Emu.h"
#include "Nsfe_Emu.h"
extern "C" {
#include "rip.h"
}

namespace {

const double CPU = 1789772.72727;
const int k_noisePeriods[16] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};
const int k_dmcPeriods[16] = {428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54};

enum { CH_VRC6 = 1, CH_VRC7 = 2, CH_FDS = 4, CH_MMC5 = 8, CH_N163 = 16, CH_FME7 = 32 };

struct Nsf {
    Rip *rip;
    Nsf_Emu *emu;
    int chips;
    // the voices: 0-4 the 2A03, then each chip's from these indices (-1 = not on the cartridge)
    int vVrc6, vN163, vFme7, vFds, vMmc5, vVrc7;
    int nvoices;
    // key-ons since the last snapshot
    int trig[64];
    int dmcWas, vrc6On[3], vrc7Key[6];
    // the extra chips' registers, as the music wrote them
    uint8_t vrc6[3][3];
    uint8_t n163[128];
    int n163Addr;
    uint8_t fme7[16];
    int fme7Latch;
    uint8_t fds[0x53];
    uint8_t vrc7[0x40];
    int vrc7Latch;
};

Nsf *g_nsf;

void tap_write(Nsf_Emu *, unsigned addr, int data) {
    Nsf *n = g_nsf;
    if (!n) return;
    data &= 0xFF;
    // the 2A03: a write to a channel's last register restarts its note
    if (addr == 0x4003) n->trig[0]++;
    else if (addr == 0x4007) n->trig[1]++;
    else if (addr == 0x400B) n->trig[2]++;
    else if (addr == 0x400F) n->trig[3]++;
    if ((n->chips & CH_FDS) && addr >= 0x4040 && addr < 0x4040 + 0x53) {
        // the wave can only be written while $4089 bit 7 is set
        if (addr < 0x4080 && !(n->fds[0x49] & 0x80)) return;
        n->fds[addr - 0x4040] = (uint8_t)data;
        if (addr == 0x4083 && !(data & 0x80) && n->vFds >= 0) n->trig[n->vFds]++;
        return;
    }
    if ((n->chips & CH_MMC5) && n->vMmc5 >= 0) {
        if (addr == 0x5003) n->trig[n->vMmc5]++;
        else if (addr == 0x5007) n->trig[n->vMmc5 + 1]++;
    }
    if ((n->chips & CH_N163) && addr == 0x4800) {
        n->n163[n->n163Addr & 0x7F] = (uint8_t)data;
        if (n->n163Addr & 0x80) n->n163Addr = 0x80 | ((n->n163Addr + 1) & 0x7F);
        return;
    }
    if ((n->chips & CH_N163) && (addr & 0xF800) == 0xF800) { n->n163Addr = data; return; }
    if ((n->chips & CH_FME7) && addr >= 0xC000) {
        if ((addr & 0xE000) == 0xC000) n->fme7Latch = data & 15;
        else if ((addr & 0xE000) == 0xE000) n->fme7[n->fme7Latch & 15] = (uint8_t)data;
        return;
    }
    if ((n->chips & CH_VRC7) && (addr == 0x9010 || addr == 0x9030)) {
        if (addr == 0x9010) n->vrc7Latch = data & 0x3F;
        else n->vrc7[n->vrc7Latch & 0x3F] = (uint8_t)data;
        return;
    }
    if ((n->chips & CH_VRC6) && addr >= 0x9000 && addr < 0xC000) {
        int osc = (int)(addr - 0x9000) >> 12, reg = (int)(addr & 0xFFF);
        if (reg < 3 && osc < 3) n->vrc6[osc][reg] = (uint8_t)data;
    }
}

int nsf_render(RipEmu *e, int16_t *buf, int n) {
    Nsf *g = (Nsf *)e->ctx;
    while (n > 0) {
        int k = n > 2048 ? 2048 : n;
        if (g->emu->play(k * 2, buf)) memset(buf, 0, (size_t)k * 4);
        buf += k * 2;
        n -= k;
    }
    return 0;
}

// A single-cycle waveform as a sample: the cycle repeated (so the studio hears a held tone), at the rate that
// makes it middle C (key 60), its root known.
int add_cycle(Rip *r, const float *cyc, int len, uint32_t id) {
    RipSample s;
    memset(&s, 0, sizeof s);
    s.kind = RIPS_PCM;
    int reps = (512 + len - 1) / len;
    std::vector<int16_t> pcm((size_t)len * reps);
    for (int i = 0; i < len * reps; i++) {
        float v = cyc[i % len];
        pcm[i] = (int16_t)lrintf((v < -1 ? -1 : v > 1 ? 1 : v) * 16000.0f);
    }
    s.pcm = pcm.data();
    s.len = len * reps;
    s.rate = (int)lrint(len * 261.6255653);
    s.loops = 1;
    s.loopStart = 0;
    s.id = id;
    s.rootKnown = 1;
    return rip_add_sample(r, &s);
}

void set_vf(RipVF *x, int sample, float rate, float vol, int keyon) {
    x->sample = (int16_t)sample;
    x->rate = rate;
    x->l = x->r = vol;
    x->keyon = (uint8_t)(keyon != 0);
}

// a 2A03 square (also the MMC5's): -1 when silent
void square(Nsf *g, const Nes_Square &o, int enabled, int voice, RipVF *x, int second) {
    int period = o.period();
    int vol = o.volume();
    int sweepOut = !(o.regs[1] & Nes_Square::negate_flag) && period + (period >> (o.regs[1] & Nes_Square::shift_mask)) >= 0x800 &&
                   (o.regs[1] & 0x80);
    int keyon = g->trig[voice] > 0;
    g->trig[voice] = 0;
    if (!enabled || !vol || period < 8 || sweepOut) {
        if (keyon) x->keyon = 1;
        return;
    }
    RipSample s;
    memset(&s, 0, sizeof s);
    s.kind = RIPS_SQUARE;
    s.param = (o.regs[0] >> 6) | (second ? 0x20 : 0x10);    // the duties are the Game Boy's: 12.5, 25, 50, 75%
    s.id = 0x100 + (uint32_t)s.param;
    set_vf(x, rip_add_sample(g->rip, &s), (float)(CPU / (16.0 * (period + 1))), vol / 15.0f, keyon);
}

void nsf_snapshot(RipEmu *e, RipVF *out) {
    Nsf *g = (Nsf *)e->ctx;
    Rip *r = g->rip;
    for (int i = 0; i < g->nvoices; i++) { memset(&out[i], 0, sizeof out[i]); out[i].sample = -1; }
    Nes_Apu *apu = g->emu->apu_();
    int en = apu->tap_enables();
    square(g, apu->tap_square(0), en & 1, 0, &out[0], 0);
    square(g, apu->tap_square(1), en & 2, 1, &out[1], 1);
    {   // the triangle: a wave of 32 steps, at its full level (it has no volume)
        const Nes_Triangle &t = apu->tap_triangle();
        int keyon = g->trig[2] > 0;
        g->trig[2] = 0;
        int period = t.period();
        if ((en & 4) && t.length_counter && t.linear_counter && period >= 2) {
            RipSample s;
            memset(&s, 0, sizeof s);
            s.kind = RIPS_WAVE;
            for (int k = 0; k < 32; k++) s.wave[k] = (uint8_t)(k < 16 ? 15 - k : k - 16);
            s.id = 0x200;
            set_vf(&out[2], rip_add_sample(r, &s), (float)(CPU / (32.0 * (period + 1))), 1.0f, keyon);
        } else if (keyon) out[2].keyon = 1;
    }
    {
        const Nes_Noise &o = apu->tap_noise();
        int keyon = g->trig[3] > 0;
        g->trig[3] = 0;
        int vol = o.volume();
        if ((en & 8) && vol) {
            RipSample s;
            memset(&s, 0, sizeof s);
            s.kind = RIPS_NOISE;
            s.param = (o.regs[2] >> 7) & 1;
            s.id = 0x300 + (uint32_t)s.param;
            set_vf(&out[3], rip_add_sample(r, &s), (float)(CPU / k_noisePeriods[o.regs[2] & 15]), vol / 15.0f, keyon);
        } else if (keyon) out[3].keyon = 1;
    }
    {   // the DMC: its sample decoded from memory as the chip would (from the middle), at the rate it plays at
        const Nes_Dmc &d = apu->tap_dmc();
        int on = (en & 0x10) != 0;
        int keyon = on && !g->dmcWas;
        g->dmcWas = on;
        if (on) {
            int bytes = d.regs[3] * 16 + 1, start = 0x4000 + d.regs[2] * 0x40;
            uint32_t id = 0x10000u | (uint32_t)d.regs[2] << 8 | d.regs[3];
            int idx = -1;
            for (int i = 0; i < r->nsamples; i++) if (r->samples[i].id == id) { idx = i; break; }
            float hz = (float)(CPU / k_dmcPeriods[d.regs[0] & 15]);
            if (idx < 0) {
                std::vector<int16_t> pcm((size_t)bytes * 8);
                int dac = 64;
                for (int b = 0; b < bytes; b++) {
                    int v = apu->tap_read(0x8000u + ((start + b) & 0x7FFF));
                    for (int k = 0; k < 8; k++) {
                        if (v >> k & 1) { if (dac <= 125) dac += 2; }
                        else if (dac >= 2) dac -= 2;
                        pcm[b * 8 + k] = (int16_t)((dac - 64) * 256);
                    }
                }
                RipSample s;
                memset(&s, 0, sizeof s);
                s.kind = RIPS_PCM;
                s.pcm = pcm.data();
                s.len = bytes * 8;
                s.rate = (int)lrintf(hz);
                s.loops = (d.regs[0] & 0x40) != 0;
                s.id = id;
                idx = rip_add_sample(r, &s);
            }
            if (idx >= 0) set_vf(&out[4], idx, hz, 1.0f, keyon);
        }
    }
    if (g->vVrc6 >= 0) {
        for (int i = 0; i < 3; i++) {
            RipVF *x = &out[g->vVrc6 + i];
            const uint8_t *rg = g->vrc6[i];
            int on = (rg[2] & 0x80) != 0;
            int period = (rg[2] & 15) << 8 | rg[1];
            int keyon = on && !g->vrc6On[i];
            g->vrc6On[i] = on;
            if (!on) continue;
            float cyc[16];
            if (i < 2) {
                int vol = rg[0] & 15, duty = (rg[0] >> 4) & 7, digital = rg[0] >> 7;
                if (!vol) continue;
                for (int k = 0; k < 16; k++) cyc[k] = digital || k <= duty ? 1.0f : -1.0f;
                int idx = add_cycle(r, cyc, 16, 0x500u + (uint32_t)(digital ? 8 : duty));
                set_vf(x, idx, (float)(CPU / (16.0 * (period + 1))), vol / 15.0f, keyon);
            } else {
                int rate = rg[0] & 0x3F;
                if (!rate) continue;
                for (int k = 0; k < 14; k++) cyc[k] = (k / 2) / 3.0f - 1.0f;   // 7 steps, each held 2 clocks
                int idx = add_cycle(r, cyc, 14, 0x510);
                set_vf(x, idx, (float)(CPU / (14.0 * (period + 1))), (rate > 42 ? 42 : rate) / 42.0f, keyon);
            }
        }
    }
    if (g->vN163 >= 0) {
        int nch = ((g->n163[0x7F] >> 4) & 7) + 1;
        for (int c = 0; c < 8; c++) {
            if (c < 8 - nch) continue;   // the active channels are the last ones
            const uint8_t *rg = &g->n163[0x40 + c * 8];
            int vol = rg[7] & 15;
            int f18 = rg[0] | rg[2] << 8 | (rg[4] & 3) << 16;
            int len = 256 - (rg[4] & 0xFC), addr = rg[6];
            if (!vol || !f18 || len <= 0) continue;
            std::vector<float> cyc((size_t)len);
            for (int k = 0; k < len; k++) {
                int a = (addr + k) & 0xFF, b = g->n163[a >> 1];
                int nib = a & 1 ? b >> 4 : b & 15;
                cyc[k] = (nib - 7.5f) / 7.5f;
            }
            int idx = add_cycle(r, cyc.data(), len, 0x600u + (uint32_t)c);
            double hz = CPU * f18 / (15.0 * 65536.0 * nch * len);
            set_vf(&out[g->vN163 + (c - (8 - nch))], idx, (float)hz, vol / 15.0f, 0);
        }
    }
    if (g->vFme7 >= 0) {
        for (int c = 0; c < 3; c++) {
            int tone = !(g->fme7[7] >> c & 1);
            int vr = g->fme7[8 + c];
            int vol = vr & 0x10 ? 15 : vr & 15;
            int period = g->fme7[c * 2] | (g->fme7[c * 2 + 1] & 15) << 8;
            if (!tone || !vol || period < 1) continue;
            RipSample s;
            memset(&s, 0, sizeof s);
            s.kind = RIPS_SQUARE;
            s.param = 2 | (c == 1 ? 0x20 : 0x10);
            s.id = 0x100 + (uint32_t)s.param;
            // its volume steps are 3 dB apart
            set_vf(&out[g->vFme7 + c], rip_add_sample(r, &s), (float)(CPU / (32.0 * period)), powf(2.0f, (vol - 15) / 2.0f), 0);
        }
    }
    if (g->vFds >= 0) {
        RipVF *x = &out[g->vFds];
        int keyon = g->trig[g->vFds] > 0;
        g->trig[g->vFds] = 0;
        int f = g->fds[0x42] | (g->fds[0x43] & 15) << 8;
        int halt = g->fds[0x43] & 0x80;
        int gain = g->fds[0x40] & 0x3F;
        static const float k_master[4] = {1.0f, 2.0f / 3, 0.5f, 0.4f};
        if (!halt && f && gain) {
            float cyc[64];
            for (int k = 0; k < 64; k++) cyc[k] = ((g->fds[k] & 0x3F) - 31.5f) / 31.5f;
            int idx = add_cycle(r, cyc, 64, 0x700);
            set_vf(x, idx, (float)(CPU * f / (65536.0 * 64.0)), (gain > 32 ? 32 : gain) / 32.0f * k_master[g->fds[0x49] & 3], keyon);
        }
    }
    if (g->vMmc5 >= 0) {
        Nes_Apu *m = g->emu->tap_mmc5();
        if (m) {
            int me = m->tap_enables();
            square(g, m->tap_square(0), me & 1, g->vMmc5, &out[g->vMmc5], 0);
            square(g, m->tap_square(1), me & 2, g->vMmc5 + 1, &out[g->vMmc5 + 1], 1);
        }
    }
    if (g->vVrc7 >= 0) {
        for (int c = 0; c < 6; c++) {
            int fnum = g->vrc7[0x10 + c] | (g->vrc7[0x20 + c] & 1) << 8;
            int block = (g->vrc7[0x20 + c] >> 1) & 7;
            int key = (g->vrc7[0x20 + c] >> 4) & 1;
            int patch = g->vrc7[0x30 + c] >> 4, att = g->vrc7[0x30 + c] & 15;
            int keyon = key && !g->vrc7Key[c];
            g->vrc7Key[c] = key;
            if (!key || !fnum) continue;
            float cyc[32];
            for (int k = 0; k < 32; k++) cyc[k] = sinf(k * 6.2831853f / 32);
            int idx = add_cycle(r, cyc, 32, 0x800u + (uint32_t)patch);
            double hz = 49715.9 * fnum / (double)(1 << (19 - block));
            set_vf(&out[g->vVrc7 + c], idx, (float)hz, powf(10.0f, -3.0f * att / 20.0f), keyon);
        }
    }
    (void)r;
}

void nsf_close(RipEmu *e) {
    Nsf *g = (Nsf *)e->ctx;
    Nsf_Emu::tap_write = 0;
    if (g_nsf == g) g_nsf = 0;
    delete g->emu;
    delete g;
    e->ctx = 0;
}

std::string parse_m3u(const std::string &path, std::vector<std::string> &fields, char *err, int errlen) {
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) { snprintf(err, errlen, "can't read %s", path.c_str()); return ""; }
    char line[4096];
    std::string file;
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '\r' || line[0] == '\n') continue;
        char *sep = strstr(line, "::");
        if (!sep) continue;
        *sep = 0;
        file = line;
        std::string cur;
        for (char *q = sep + 2; *q && *q != '\r' && *q != '\n'; q++) {
            if (*q == '\\' && q[1]) { cur += *++q; continue; }
            if (*q == ',') { fields.push_back(cur); cur.clear(); continue; }
            cur += *q;
        }
        fields.push_back(cur);
        break;
    }
    fclose(f);
    return file;
}

} // namespace

extern "C" int rip_open_nsf(RipEmu *e, Rip *r, int track, char *err, int errlen) {
    std::string path = r->path;
    std::vector<std::string> fields;
    size_t dot = path.find_last_of('.');
    std::string ext = dot == std::string::npos ? "" : path.substr(dot);
    for (auto &c : ext) c = (char)tolower((unsigned char)c);
    if (ext == ".m3u") {
        // a playlist line (NEZplug style): "file.nsf::NSF,track,title,length,loop,fade" (tracks counted from 1)
        std::string file = parse_m3u(path, fields, err, errlen);
        if (file.empty() || fields.size() < 2) { if (!err[0]) snprintf(err, errlen, "%s doesn't name a .nsf file", path.c_str()); return 0; }
        if (track < 0) track = atoi(fields[1].c_str()) - 1;
        size_t slash = path.find_last_of("/\\");
        path = (slash == std::string::npos ? std::string() : path.substr(0, slash + 1)) + file;
        dot = path.find_last_of('.');
        ext = dot == std::string::npos ? "" : path.substr(dot);
        for (auto &c : ext) c = (char)tolower((unsigned char)c);
    }
    Nsf *g = new Nsf();
    memset(g->trig, 0, sizeof g->trig);
    g->rip = r;
    g->emu = ext == ".nsfe" ? new Nsfe_Emu() : new Nsf_Emu();
    Classic_Emu::exact_steps = true;
    g->emu->set_sample_rate(44100);
    const char *e1 = g->emu->load_file(path.c_str());
    if (e1) { snprintf(err, errlen, "couldn't open %s: %s", path.c_str(), e1); delete g->emu; delete g; return 0; }
    if (track < 0) track = 0;
    if (track >= g->emu->track_count()) track = 0;
    g->chips = g->emu->tap_chips();
    int nv = 5;
    g->vVrc6 = g->vN163 = g->vFme7 = g->vFds = g->vMmc5 = g->vVrc7 = -1;
    if (g->chips & CH_VRC6) { g->vVrc6 = nv; nv += 3; }
    if (g->chips & CH_N163) { g->vN163 = nv; nv += 8; }
    if (g->chips & CH_FME7) { g->vFme7 = nv; nv += 3; }
    if (g->chips & CH_FDS) { g->vFds = nv; nv += 1; }
    if (g->chips & CH_MMC5) { g->vMmc5 = nv; nv += 2; }
    if (g->chips & CH_VRC7) { g->vVrc7 = nv; nv += 6; }
    g->nvoices = nv;
    track_info_t info;
    if (!g->emu->track_info(&info, track)) {
        if (info.song[0]) snprintf(r->title, sizeof r->title, "%s", info.song);
        if (info.game[0]) snprintf(r->game, sizeof r->game, "%s", info.game);
        if (info.author[0]) snprintf(r->artist, sizeof r->artist, "%s", info.author);
        if (info.copyright[0]) snprintf(r->copyright, sizeof r->copyright, "%s", info.copyright);
        if (info.length > 0) r->length = info.length / 1000.0;
        else if (info.intro_length >= 0 && info.loop_length > 0) r->length = (info.intro_length + 2 * info.loop_length) / 1000.0;
    }
    if (!r->title[0] || !strcmp(r->title, "<?>")) snprintf(r->title, sizeof r->title, "%s #%d", r->game[0] ? r->game : "NES song", track + 1);
    if (fields.size() > 2 && !fields[2].empty()) snprintf(r->title, sizeof r->title, "%s", fields[2].c_str());
    if (fields.size() > 3 && !fields[3].empty()) r->length = rip_parse_time(fields[3].c_str());
    if (fields.size() > 5 && !fields[5].empty()) r->fade = rip_parse_time(fields[5].c_str());
    r->track = track;
    g_nsf = g;
    Nsf_Emu::tap_write = tap_write;
    g->emu->ignore_silence(true);
    if (g->emu->start_track(track)) { snprintf(err, errlen, "couldn't start song %d", track + 1); g_nsf = 0; Nsf_Emu::tap_write = 0; delete g->emu; delete g; return 0; }
    if (getenv("MMS_NSF_MUTE")) g->emu->mute_voices((int)strtol(getenv("MMS_NSF_MUTE"), NULL, 0));   // tests: hear some channels
    snprintf(r->console, sizeof r->console, "NES");
    e->ctx = g;
    e->rip = r;
    e->rate = 44100;
    e->nvoices = nv;
    e->render = nsf_render;
    e->snapshot = nsf_snapshot;
    e->close = nsf_close;
    return 1;
}
