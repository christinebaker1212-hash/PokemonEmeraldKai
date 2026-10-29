// Moemon Music Studio: GBS (Game Boy) rips, played by Game_Music_Emu (third_party/gme_gbs). The GBA
// has the Game Boy's four sound channels, so these become the game's own Game Boy voices: squares
// with their duty, the wave channel with the song's waves, and noise.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "Gbs_Emu.h"
extern "C" {
#include "rip.h"
}

namespace {

struct Gbs {
    Rip *rip;
    Gbs_Emu emu;
    int triggers[4];
};

Gbs *g_gbs;
const Gb_Apu *g_apu;

void tap_trigger(Gb_Apu *apu, int osc) {
    g_apu = apu;
    if (g_gbs && osc >= 0 && osc < 4) g_gbs->triggers[osc]++;
}

int gbs_render(RipEmu *e, int16_t *buf, int n) {
    Gbs *g = (Gbs *)e->ctx;
    while (n > 0) {
        int k = n > 2048 ? 2048 : n;
        if (g->emu.play(k * 2, buf)) memset(buf, 0, (size_t)k * 4);
        buf += k * 2;
        n -= k;
    }
    return 0;
}

void gbs_snapshot(RipEmu *e, RipVF *out) {
    Gbs *g = (Gbs *)e->ctx;
    for (int i = 0; i < 4; i++) { memset(&out[i], 0, sizeof out[i]); out[i].sample = -1; }
    if (!g_apu) return;
    const uint8_t *regs = g_apu->tap_regs();
    int nr50 = regs[0x14], nr51 = regs[0x15];
    float ml = (((nr50 >> 4) & 7) + 1) / 8.0f, mr = ((nr50 & 7) + 1) / 8.0f;
    for (int i = 0; i < 4; i++) {
        const Gb_Osc *o = g_apu->tap_osc(i);
        RipVF *x = &out[i];
        x->keyon = g->triggers[i] > 0;
        g->triggers[i] = 0;
        int playing = o->enabled && o->volume && (!(o->regs[4] & Gb_Osc::len_enabled_mask) || o->length);
        if (i == 2 && !(regs[0x0A] & 0x80)) playing = 0;
        if (!playing && !x->keyon) continue;
        RipSample s;
        memset(&s, 0, sizeof s);
        int fr = o->frequency();
        float vol;
        if (i < 2) {
            s.kind = RIPS_SQUARE;
            s.param = (o->regs[1] >> 6) | (i == 0 ? 0x10 : 0x20);
            s.id = 0x100 + (uint32_t)s.param;
            x->rate = 131072.0f / (2048 - fr);
            vol = o->volume / 15.0f;
        } else if (i == 2) {
            s.kind = RIPS_WAVE;
            memcpy(s.wave, g_apu->tap_wave(), 32);
            s.id = 0x200;
            x->rate = 65536.0f / (2048 - fr);
            static const float k_wv[4] = {0, 1, 0.5f, 0.25f};
            vol = k_wv[o->volume & 3];
        } else {
            s.kind = RIPS_NOISE;
            int nr43 = regs[0x12];
            s.param = (nr43 >> 3) & 1;
            s.id = 0x300 + (uint32_t)s.param;
            int r = nr43 & 7, sh = nr43 >> 4;
            x->rate = 524288.0f / (r ? r : 0.5f) / (float)(2 << sh);
            vol = o->volume / 15.0f;
        }
        if (!playing) vol = 0;
        x->sample = (int16_t)rip_add_sample(g->rip, &s);
        x->l = (nr51 >> (4 + i) & 1) ? vol * ml : 0;
        x->r = (nr51 >> i & 1) ? vol * mr : 0;
    }
}

void gbs_close(RipEmu *e) {
    Gbs *g = (Gbs *)e->ctx;
    Gb_Apu::tap_trigger = 0;
    g_apu = 0;
    if (g_gbs == g) g_gbs = 0;
    delete g;
    e->ctx = 0;
}

} // namespace

extern "C" int rip_open_gbs(RipEmu *e, Rip *r, int track, char *err, int errlen) {
    std::string path = r->path, m3u;
    // a playlist line (NEZplug style): "file.gbs::GBS,track,title,length,loop,fade" (commas in
    // fields are escaped as \,)
    std::vector<std::string> fields;
    size_t dot = path.find_last_of('.');
    if (dot != std::string::npos && (path.substr(dot) == ".m3u" || path.substr(dot) == ".M3U")) {
        m3u = path;
        FILE *f = fopen(m3u.c_str(), "rb");
        if (!f) { snprintf(err, errlen, "can't read %s", m3u.c_str()); return 0; }
        char line[4096];
        std::string gbs;
        while (fgets(line, sizeof line, f)) {
            if (line[0] == '#' || line[0] == '\r' || line[0] == '\n') continue;
            char *sep = strstr(line, "::");
            if (!sep) continue;
            *sep = 0;
            gbs = line;
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
        if (gbs.empty() || fields.size() < 2) { snprintf(err, errlen, "%s doesn't name a .gbs file", m3u.c_str()); return 0; }
        if (track < 0) track = atoi(fields[1].c_str());
        size_t slash = path.find_last_of("/\\");
        path = (slash == std::string::npos ? std::string() : path.substr(0, slash + 1)) + gbs;
    }
    Gbs *g = new Gbs();
    g->rip = r;
    Classic_Emu::exact_steps = true;    // the chip read when it's heard (see gbs_snapshot)
    g->emu.set_sample_rate(44100);
    const char *e1 = g->emu.load_file(path.c_str());
    if (e1) { snprintf(err, errlen, "couldn't open %s: %s", path.c_str(), e1); delete g; return 0; }
    if (track < 0) track = 0;
    if (track >= g->emu.track_count()) track = 0;
    track_info_t info;
    if (!g->emu.track_info(&info, track)) {
        if (info.song[0]) snprintf(r->title, sizeof r->title, "%s", info.song);
        if (info.game[0]) snprintf(r->game, sizeof r->game, "%s", info.game);
        if (info.author[0]) snprintf(r->artist, sizeof r->artist, "%s", info.author);
        if (info.copyright[0]) snprintf(r->copyright, sizeof r->copyright, "%s", info.copyright);
        if (info.length > 0) r->length = info.length / 1000.0;
        else if (info.intro_length >= 0 && info.loop_length > 0) r->length = (info.intro_length + 2 * info.loop_length) / 1000.0;
    }
    if (fields.size() > 2 && !fields[2].empty()) {
        // "Title - Artist - Game - Copyright"
        std::string t = fields[2];
        size_t a = t.find(" - ");
        snprintf(r->title, sizeof r->title, "%s", t.substr(0, a).c_str());
        if (a != std::string::npos) {
            std::string rest = t.substr(a + 3);
            size_t b = rest.find(" - ");
            snprintf(r->artist, sizeof r->artist, "%s", rest.substr(0, b).c_str());
            if (b != std::string::npos) {
                std::string rest2 = rest.substr(b + 3);
                size_t c = rest2.find(" - ");
                snprintf(r->game, sizeof r->game, "%s", rest2.substr(0, c).c_str());
                if (c != std::string::npos) snprintf(r->copyright, sizeof r->copyright, "%s", rest2.substr(c + 3).c_str());
            }
        }
    }
    if (fields.size() > 3 && !fields[3].empty()) r->length = rip_parse_time(fields[3].c_str());
    if (fields.size() > 5 && !fields[5].empty()) r->fade = rip_parse_time(fields[5].c_str());
    r->track = track;
    g_gbs = g;
    Gb_Apu::tap_trigger = tap_trigger;
    g->emu.ignore_silence(true);
    if (g->emu.start_track(track)) { snprintf(err, errlen, "couldn't start track %d", track); g_gbs = 0; delete g; return 0; }
    if (getenv("MMS_GBS_MUTE")) g->emu.mute_voices((int)strtol(getenv("MMS_GBS_MUTE"), NULL, 0));   // tests: hear some channels
    snprintf(r->console, sizeof r->console, "Game Boy");
    e->ctx = g;
    e->rip = r;
    e->rate = 44100;
    e->nvoices = 4;
    e->render = gbs_render;
    e->snapshot = gbs_snapshot;
    e->close = gbs_close;
    return 1;
}
