// Moemon Music Studio: VGM / VGZ (Sega Genesis / Mega Drive) rips. The original sound is Game_Music_Emu's
// VGM player (third_party/gme: Nuked-OPN2's YM2612 and the SN76489). A VGM is the sound chips' register
// writes with their timing, so the notes are read from it directly, frame by frame:
//  - the YM2612's 6 FM channels: key-ons (register 0x28), frequency (0xA0-0xA6), and the patch (the 4
//    operators' registers, algorithm and feedback). Each patch becomes an instrument: the patch played on
//    its own by Nuked-OPN2 at middle C, looped over whole periods once its envelope has settled. The
//    carriers' total levels are the note's volume, and a key-off fades it at the carriers' release rate.
//  - the DAC (channel 6 playing samples): each sample started from the VGM's PCM data bank (seek, then
//    writes) is a drum hit, its sound cut from the bank at the rate it's written.
//  - the SN76489: 3 square waves (50% duty) and the noise, as the GBA's own Game Boy channels.
// The VGM's own loop (its header's loop offset) is the song's loop.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include "Vgm_Emu.h"
#include "Ym2612_Emu.h"
#include "zlib/zlib.h"
extern "C" {
#include "rip.h"
#include "engine.h"
}

namespace {

enum { VGM_RATE = 44100, FM_VOICES = 6, DAC_VOICE = 6, PSG_VOICE = 7, VGM_VOICES = 11 };
const double REF_FREQ_BLOCK = 4, REF_FNUM = 644;      // middle C (261.7 Hz at the Genesis's clock)
const int LOOP_PERIODS = 20;                          // 20 periods of middle C: 4071 samples (to 0.02 of one)

struct FmChan {
    int keyMask;          // operators keyed on
    int keyons;           // since the last snapshot
    double release;       // 1 while keyed; after a key-off, fades by the carriers' release rate
    double releaseStep;   // per game frame
    int sample;           // the patch's instrument
    uint32_t patchHash;
};

struct DacHit { long start; int len; double rate; };

struct Vgm {
    Rip *rip;
    Vgm_Emu emu;
    std::vector<uint8_t> data;
    // the parse
    size_t pos, dataStart, end, loopPos;
    long time;            // VGM samples (44100 Hz)
    double fmClock, psgClock, fmRate;
    uint8_t ym[2][256];
    int a4latch[2][8];
    FmChan fm[FM_VOICES];
    std::map<uint32_t, int> patchSample;
    // DAC
    std::vector<uint8_t> bank;
    long bankPos;
    int dacEnabled, dacKeyons, dacSample;
    double dacRate;
    long dacUntil;        // VGM time the current hit ends
    std::map<uint64_t, int> dacSamples;
    // PSG
    int psgLatch, psgTone[4], psgVol[4], psgKeyons[4], psgStereo;
    double lastFrameTime;
    int only;
};

uint32_t rd32(const std::vector<uint8_t> &d, size_t o) { return o + 4 <= d.size() ? d[o] | d[o + 1] << 8 | d[o + 2] << 16 | (uint32_t)d[o + 3] << 24 : 0; }

int vgm_render(RipEmu *e, int16_t *buf, int n) {
    Vgm *v = (Vgm *)e->ctx;
    while (n > 0) {
        int k = n > 2048 ? 2048 : n;
        if (v->emu.play(k * 2, buf)) memset(buf, 0, (size_t)k * 4);
        buf += k * 2;
        n -= k;
    }
    return 0;
}

// the operators that are heard (carriers) of each algorithm, as slot bits (op1 op2 op3 op4 = 1 2 4 8)
const int k_carriers[8] = {8, 8, 8, 8, 2 | 8, 2 | 4 | 8, 2 | 4 | 8, 15};
// register offset of operator n (op1, op2, op3, op4) in a channel's block: slots are ordered 1, 3, 2, 4
const int k_opOfs[4] = {0, 8, 4, 12};

// YM2612 envelope: how long a rate (0-63) takes over the 96 dB range, in seconds
double eg_time96(int rate) {
    if (rate <= 0) return 1e9;
    if (rate >= 62) return 0.002;
    int k = rate >> 2, m = rate & 3;
    double eg = 7670453.0 / 144 / 3;                  // envelope clock
    double perUpdate = (4.0 + m) / 4.0;
    double cycles = k <= 11 ? 1024.0 / perUpdate * pow(2.0, 11 - k) : 1024.0 / (perUpdate * pow(2.0, k - 11));
    return cycles / eg;
}

// a patch's instrument: the patch alone at middle C, its envelope settled, looped on whole periods
int patch_sample(Vgm *v, int port, int ch) {
    const uint8_t *r = v->ym[port];
    int alg = r[0xB0 + ch] & 7;
    int carriers = k_carriers[alg];
    // the carriers' levels relative to the loudest (the loudest's level is the note's volume)
    int minTl = 127;
    for (int op = 0; op < 4; op++) if (carriers >> op & 1) { int tl = r[0x40 + k_opOfs[op] + ch] & 0x7F; if (tl < minTl) minTl = tl; }
    uint8_t key[32];
    int n = 0;
    key[n++] = r[0xB0 + ch];
    for (int op = 0; op < 4; op++) {
        int o = k_opOfs[op] + ch;
        int tl = r[0x40 + o] & 0x7F;
        if (carriers >> op & 1) tl -= minTl;
        key[n++] = r[0x30 + o]; key[n++] = (uint8_t)tl; key[n++] = r[0x50 + o]; key[n++] = r[0x60 + o];
        key[n++] = r[0x70 + o]; key[n++] = r[0x80 + o]; key[n++] = r[0x90 + o];
    }
    uint32_t h = rip_hash(key, n);
    auto it = v->patchSample.find(h);
    if (it != v->patchSample.end()) return it->second;

    Ym2612_Emu ym;
    double fs = v->fmClock / 144;
    ym.set_rate(fs, v->fmClock);
    ym.reset();
    ym.write0(0x22, 0);                               // no LFO
    ym.write0(0x27, 0);
    ym.write0(0x2B, 0);
    ym.write0(0xB0, key[0]);
    ym.write0(0xB4, 0xC0);
    for (int op = 0; op < 4; op++) {
        int o = k_opOfs[op];
        const uint8_t *k = key + 1 + op * 7;
        ym.write0(0x30 + o, k[0]); ym.write0(0x40 + o, k[1]); ym.write0(0x50 + o, k[2]); ym.write0(0x60 + o, k[3]);
        ym.write0(0x70 + o, k[4]); ym.write0(0x80 + o, k[5]); ym.write0(0x90 + o, k[6]);
    }
    int fnum = (int)REF_FNUM, block = (int)REF_FREQ_BLOCK;
    ym.write0(0xA4, block << 3 | fnum >> 8);
    ym.write0(0xA0, fnum & 0xFF);
    ym.write0(0x28, 0xF0);                            // all four operators on, channel 1
    double ref = fnum * fs * pow(2.0, block - 1) / 1048576.0;
    double period = fs / ref;
    int loopLen = (int)lround(period * LOOP_PERIODS);
    int total = (int)fs;                              // one second
    std::vector<int16_t> st((size_t)total * 2, 0), pcm((size_t)total);
    ym.run(total, st.data());
    for (int i = 0; i < total; i++) pcm[i] = (int16_t)((st[i * 2] + st[i * 2 + 1]) / 2);
    RipSample rs;
    memset(&rs, 0, sizeof rs);
    rs.kind = RIPS_PCM;
    rs.pcm = pcm.data();
    rs.len = total;
    rs.rate = (int)lround(fs);
    rs.loops = 1;
    rs.loopStart = total - loopLen;
    rs.id = 0x4000000u | (h & 0xFFFFFF);
    rs.hash = h;
    int idx = rip_add_sample(v->rip, &rs);
    v->patchSample[h] = idx;
    return idx;
}

// the frequency (Hz) a channel's registers name
double fm_freq(Vgm *v, int port, int ch) {
    const uint8_t *r = v->ym[port];
    int fnum = (r[0xA4 + ch] & 7) << 8 | r[0xA0 + ch], block = (r[0xA4 + ch] >> 3) & 7;
    return fnum * (v->fmClock / 144) * pow(2.0, block - 1) / 1048576.0;
}

void fm_write(Vgm *v, int port, int addr, int data) {
    v->ym[port][addr] = (uint8_t)data;
    if (port == 0 && addr == 0x28) {
        int c = data & 7;
        if ((c & 3) == 3) return;
        int ch = (c & 3) + (c & 4 ? 3 : 0);
        int mask = data >> 4;
        FmChan *f = &v->fm[ch];
        if (mask && !f->keyMask) {
            f->keyons++;
            f->release = 1;
            f->sample = patch_sample(v, ch / 3, ch % 3);
        } else if (!mask && f->keyMask) {
            // released: fade at the carriers' release rate (the slowest is heard last)
            const uint8_t *r = v->ym[ch / 3];
            int alg = r[0xB0 + ch % 3] & 7, slow = 63;
            for (int op = 0; op < 4; op++) if (k_carriers[alg] >> op & 1) {
                int rr = r[0x80 + k_opOfs[op] + ch % 3] & 15, rate = rr * 4 + 2;
                if (rate < slow) slow = rate;
            }
            double t = eg_time96(slow), fps = (double)ENG_RATE / ENG_FRAME_SAMPLES;
            f->releaseStep = pow(10.0, -96.0 / 20.0 / (t * fps));   // per frame, linear in dB
        }
        f->keyMask = mask;
    }
    if (port == 0 && addr == 0x2B) v->dacEnabled = data >> 7;
}

// the DAC hit starting at the bank position: how many bytes are written before the next seek (or 2 s) and
// how fast, read ahead in the command stream
DacHit dac_hit(Vgm *v) {
    DacHit h;
    h.start = v->bankPos;
    h.len = 0;
    h.rate = 0;
    size_t p = v->pos;
    long t = 0, first = -1, last = 0;
    while (p < v->end && t < 2 * VGM_RATE) {
        int c = v->data[p];
        if (c >= 0x80 && c <= 0x8F) {
            if (first < 0) first = t;
            last = t;
            h.len++;
            t += c & 15;
            p++;
        } else if (c >= 0x70 && c <= 0x7F) { t += (c & 15) + 1; p++; }
        else if (c == 0x61) { t += v->data[p + 1] | v->data[p + 2] << 8; p += 3; }
        else if (c == 0x62) { t += 735; p++; }
        else if (c == 0x63) { t += 882; p++; }
        else if (c == 0xE0 || c == 0x66 || c == 0x67) break;
        else if (c == 0x4F || c == 0x50) p += 2;
        else if (c >= 0x51 && c <= 0x5F) p += 3;
        else if (c >= 0x30 && c <= 0x3F) p += 2;
        else if (c >= 0x40 && c <= 0x4E) p += 3;
        else if (c >= 0xA0 && c <= 0xBF) p += 3;
        else if (c >= 0xC0 && c <= 0xDF) p += 4;
        else if (c >= 0xE1) p += 5;
        else if (c >= 0x90 && c <= 0x95) p += c == 0x92 ? 6 : c == 0x93 ? 11 : c == 0x94 ? 2 : 5;
        else p++;
    }
    if (h.len > 1 && last > first) h.rate = (double)VGM_RATE * (h.len - 1) / (last - first);
    else h.rate = 8000;
    // trailing silence (the driver writing the middle value) isn't the sound
    while (h.len > 0 && h.start + h.len - 1 < (long)v->bank.size() && abs(v->bank[h.start + h.len - 1] - 0x80) <= 1) h.len--;
    return h;
}

void dac_start(Vgm *v) {
    DacHit h = dac_hit(v);
    if (h.len < 16 || h.start + h.len > (long)v->bank.size()) return;
    uint64_t key = (uint64_t)h.start << 32 | (uint32_t)h.len;
    int idx;
    auto it = v->dacSamples.find(key);
    if (it != v->dacSamples.end()) idx = it->second;
    else {
        std::vector<int16_t> pcm((size_t)h.len);
        for (int i = 0; i < h.len; i++) pcm[i] = (int16_t)((v->bank[h.start + i] - 0x80) << 8);
        RipSample rs;
        memset(&rs, 0, sizeof rs);
        rs.kind = RIPS_PCM;
        rs.pcm = pcm.data();
        rs.len = h.len;
        rs.rate = (int)lround(h.rate);
        rs.id = 0x8000000u | (uint32_t)h.start;
        idx = rip_add_sample(v->rip, &rs);
        v->dacSamples[key] = idx;
    }
    v->dacSample = idx;
    v->dacRate = v->rip->samples[idx].rate;
    v->dacKeyons++;
    v->dacUntil = v->time + (long)(h.len * (double)VGM_RATE / h.rate);
}

void psg_write(Vgm *v, int d) {
    if (d & 0x80) {
        v->psgLatch = (d >> 4) & 7;
        int ch = v->psgLatch >> 1;
        if (v->psgLatch & 1) {
            int was = v->psgVol[ch];
            v->psgVol[ch] = d & 15;
            if (was == 15 && v->psgVol[ch] != 15) v->psgKeyons[ch]++;
        } else if (ch == 3) {
            v->psgTone[3] = d & 7;
            v->psgKeyons[3]++;                        // the noise restarts on a write
        } else v->psgTone[ch] = (v->psgTone[ch] & 0x3F0) | (d & 15);
    } else {
        int ch = v->psgLatch >> 1;
        if (v->psgLatch & 1) v->psgVol[ch] = d & 15;
        else if (ch < 3) v->psgTone[ch] = (v->psgTone[ch] & 15) | (d & 0x3F) << 4;
    }
}

// runs the commands up to VGM time `until`
void advance(Vgm *v, long until) {
    while (v->pos < v->end && v->time < until) {
        int c = v->data[v->pos];
        const uint8_t *d = &v->data[v->pos];
        if (c == 0x66) {                              // the end: on from the loop, as the player does
            if (v->loopPos) { v->pos = v->loopPos; continue; }
            v->pos = v->end;
            break;
        }
        if (c >= 0x70 && c <= 0x7F) { v->time += (c & 15) + 1; v->pos++; continue; }
        if (c >= 0x80 && c <= 0x8F) {
            if (v->bankPos < (long)v->bank.size()) v->bankPos++;
            v->time += c & 15;
            v->pos++;
            continue;
        }
        switch (c) {
        case 0x61: v->time += d[1] | d[2] << 8; v->pos += 3; break;
        case 0x62: v->time += 735; v->pos++; break;
        case 0x63: v->time += 882; v->pos++; break;
        case 0x4F: v->psgStereo = d[1]; v->pos += 2; break;
        case 0x50: psg_write(v, d[1]); v->pos += 2; break;
        case 0x52: fm_write(v, 0, d[1], d[2]); v->pos += 3; break;
        case 0x53: fm_write(v, 1, d[1], d[2]); v->pos += 3; break;
        case 0x67: {
            int type = d[2];
            uint32_t len = (d[3] | d[4] << 8 | d[5] << 16 | (uint32_t)d[6] << 24) & 0x7FFFFFFF;
            if (type == 0 && v->pos + 7 + len <= v->end) v->bank.insert(v->bank.end(), d + 7, d + 7 + len);
            v->pos += 7 + len;
            break;
        }
        case 0xE0:
            v->bankPos = d[1] | d[2] << 8 | d[3] << 16 | (long)d[4] << 24;
            v->pos += 5;
            if (v->dacEnabled) dac_start(v);
            break;
        default:
            if (c >= 0x30 && c <= 0x3F) v->pos += 2;
            else if ((c >= 0x40 && c <= 0x4E) || (c >= 0x51 && c <= 0x5F) || (c >= 0xA0 && c <= 0xBF)) v->pos += 3;
            else if (c >= 0xC0 && c <= 0xDF) v->pos += 4;
            else if (c >= 0x90 && c <= 0x95) v->pos += c == 0x92 ? 6 : c == 0x93 ? 11 : c == 0x94 ? 2 : 5;
            else if (c >= 0xE1) v->pos += 5;
            else v->pos++;
        }
    }
}

void vgm_snapshot(RipEmu *e, RipVF *out) {
    Vgm *v = (Vgm *)e->ctx;
    double fps = (double)ENG_RATE / ENG_FRAME_SAMPLES;
    v->lastFrameTime += VGM_RATE / fps;
    advance(v, (long)v->lastFrameTime);
    for (int i = 0; i < VGM_VOICES; i++) { memset(&out[i], 0, sizeof out[i]); out[i].sample = -1; }
    // FM
    for (int ch = 0; ch < FM_VOICES && (v->only & 1); ch++) {
        FmChan *f = &v->fm[ch];
        RipVF *x = &out[ch];
        x->keyon = f->keyons > 0;
        f->keyons = 0;
        if (ch == 5 && v->dacEnabled) { f->release = 0; continue; }
        if (!f->keyMask) { f->release *= f->releaseStep; if (f->release < 0.002) f->release = 0; }
        if (f->release <= 0 || f->sample < 0) continue;
        const uint8_t *r = v->ym[ch / 3];
        int c = ch % 3, alg = r[0xB0 + c] & 7, minTl = 127;
        for (int op = 0; op < 4; op++) if (k_carriers[alg] >> op & 1) { int tl = r[0x40 + k_opOfs[op] + c] & 0x7F; if (tl < minTl) minTl = tl; }
        double gain = pow(10.0, -0.75 * minTl / 20.0) * f->release;
        double ref = REF_FNUM * (v->fmClock / 144) * pow(2.0, REF_FREQ_BLOCK - 1) / 1048576.0;
        x->sample = (int16_t)f->sample;
        x->rate = (float)(v->rip->samples[f->sample].rate * fm_freq(v, ch / 3, c) / ref);
        int pan = r[0xB4 + c] >> 6;
        x->l = (float)((pan & 2) ? gain : 0);
        x->r = (float)((pan & 1) ? gain : 0);
    }
    // DAC
    if (v->only & 2) {
        RipVF *x = &out[DAC_VOICE];
        x->keyon = v->dacKeyons > 0;
        v->dacKeyons = 0;
        if (v->dacEnabled && v->dacSample >= 0 && (v->time < v->dacUntil || x->keyon)) {
            x->sample = (int16_t)v->dacSample;
            x->rate = (float)v->dacRate;
            int pan = v->ym[1][0xB6] >> 6;
            x->l = (pan & 2) ? 0.6f : 0;
            x->r = (pan & 1) ? 0.6f : 0;
        }
    }
    // PSG: squares and noise
    for (int ch = 0; ch < 4 && (v->only & 4); ch++) {
        RipVF *x = &out[PSG_VOICE + ch];
        x->keyon = v->psgKeyons[ch] > 0;
        v->psgKeyons[ch] = 0;
        if (v->psgVol[ch] >= 15) continue;
        RipSample s;
        memset(&s, 0, sizeof s);
        if (ch < 3) {
            int n = v->psgTone[ch];
            if (n <= 1) continue;
            s.kind = RIPS_SQUARE;
            s.param = 2;                              // 50%
            s.id = 0x500;
            x->rate = (float)(v->psgClock / (32.0 * n));
        } else {
            int nc = v->psgTone[3];
            s.kind = RIPS_NOISE;
            s.param = (nc & 4) ? 0 : 1;               // white / periodic
            s.id = 0x600 + (uint32_t)s.param;
            int rate = nc & 3;
            x->rate = (float)(rate < 3 ? v->psgClock / (16.0 * (16 << rate)) : v->psgClock / (32.0 * (v->psgTone[2] > 0 ? v->psgTone[2] : 1)));
        }
        x->sample = (int16_t)rip_add_sample(v->rip, &s);
        // (2 dB a step; against the FM channels and the DAC as the Genesis mixes them: measured, each heard alone)
        float g = (float)pow(10.0, -2.0 * v->psgVol[ch] / 20.0) * 0.145f;
        int st = v->psgStereo;
        x->l = (st >> (4 + ch) & 1) ? g : 0;
        x->r = (st >> ch & 1) ? g : 0;
    }
}

void vgm_close(RipEmu *e) {
    Vgm *v = (Vgm *)e->ctx;
    delete v;
    e->ctx = 0;
}

bool gunzip(const std::vector<uint8_t> &in, std::vector<uint8_t> &out) {
    z_stream z;
    memset(&z, 0, sizeof z);
    if (inflateInit2(&z, 16 + MAX_WBITS) != Z_OK) return false;
    z.next_in = (Bytef *)in.data();
    z.avail_in = (uInt)in.size();
    uint8_t buf[65536];
    int ret;
    do {
        z.next_out = buf;
        z.avail_out = sizeof buf;
        ret = inflate(&z, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) { inflateEnd(&z); return false; }
        out.insert(out.end(), buf, buf + (sizeof buf - z.avail_out));
    } while (ret != Z_STREAM_END);
    inflateEnd(&z);
    return true;
}

} // namespace

extern "C" int rip_open_vgm(RipEmu *e, Rip *r, char *err, int errlen) {
    FILE *f = fopen(r->path, "rb");
    if (!f) { snprintf(err, errlen, "can't read %s", r->path); return 0; }
    std::vector<uint8_t> raw;
    uint8_t buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) raw.insert(raw.end(), buf, buf + n);
    fclose(f);
    Vgm *v = new Vgm();
    v->rip = r;
    if (raw.size() > 2 && raw[0] == 0x1F && raw[1] == 0x8B) {
        if (!gunzip(raw, v->data)) { snprintf(err, errlen, "%s: couldn't decompress", r->path); delete v; return 0; }
    } else v->data.swap(raw);
    std::vector<uint8_t> &d = v->data;
    if (d.size() < 0x40 || memcmp(d.data(), "Vgm ", 4)) { snprintf(err, errlen, "%s isn't a VGM file", r->path); delete v; return 0; }
    uint32_t ver = rd32(d, 8);
    v->psgClock = rd32(d, 0x0C) & 0x3FFFFFFF;
    v->fmClock = ver >= 0x110 ? (rd32(d, 0x2C) & 0x3FFFFFFF) : (rd32(d, 0x10) & 0x3FFFFFFF);
    if (v->fmClock <= 0) v->fmClock = 7670453;
    if (v->psgClock <= 0) v->psgClock = 3579545;
    v->dataStart = ver >= 0x150 && rd32(d, 0x34) ? 0x34 + rd32(d, 0x34) : 0x40;
    v->end = d.size();
    if (rd32(d, 4)) { size_t eof = 4 + rd32(d, 4); if (eof < v->end) v->end = eof; }
    v->pos = v->dataStart;
    v->loopPos = rd32(d, 0x1C) ? 0x1C + rd32(d, 0x1C) : 0;
    if (v->loopPos >= v->end) v->loopPos = 0;
    for (int i = 0; i < FM_VOICES; i++) { v->fm[i].sample = -1; v->fm[i].releaseStep = 0.5; }
    v->dacSample = -1;
    for (int i = 0; i < 4; i++) v->psgVol[i] = 15;
    v->psgStereo = 0xFF;

    v->emu.set_sample_rate(VGM_RATE);
    long size = (long)d.size();
    const char *e1 = v->emu.load_tracks(d.data(), &size, 1);   // (load_mem leaves its track table empty)
    if (e1) { snprintf(err, errlen, "couldn't open %s: %s", r->path, e1); delete v; return 0; }
    track_info_t info;
    if (!v->emu.track_info(&info, 0)) {
        if (info.song[0]) snprintf(r->title, sizeof r->title, "%s", info.song);
        if (info.game[0]) snprintf(r->game, sizeof r->game, "%s", info.game);
        if (info.author[0]) snprintf(r->artist, sizeof r->artist, "%s", info.author);
        if (info.copyright[0]) snprintf(r->copyright, sizeof r->copyright, "%s", info.copyright);
        if (info.dumper[0]) snprintf(r->ripper, sizeof r->ripper, "%s", info.dumper);
    }
    // its length: the whole log, and the loop once more when it has one
    double total = rd32(d, 0x18) / (double)VGM_RATE, loop = rd32(d, 0x20) ? rd32(d, 0x20) / (double)VGM_RATE : 0;
    r->length = total + loop;
    r->fade = loop > 0 ? 8 : 0;
    v->emu.ignore_silence(true);
    v->only = getenv("MMS_VGM_ONLY") ? atoi(getenv("MMS_VGM_ONLY")) : 7;   // tests: 1 FM, 2 DAC, 4 PSG
    if (v->emu.start_track(0)) { snprintf(err, errlen, "couldn't start %s", r->path); delete v; return 0; }
    if (getenv("MMS_VGM_MUTE")) v->emu.mute_voices((int)strtol(getenv("MMS_VGM_MUTE"), NULL, 0));   // tests: FM 1-6, PCM, PSG
    snprintf(r->console, sizeof r->console, "Sega Genesis");
    r->nativeRate = VGM_RATE;
    e->ctx = v;
    e->rip = r;
    e->rate = VGM_RATE;
    e->nvoices = VGM_VOICES;
    e->render = vgm_render;
    e->snapshot = vgm_snapshot;
    e->close = vgm_close;
    return 1;
}
