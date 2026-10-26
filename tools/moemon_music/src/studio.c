#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "studio.h"
#include "inst.h"
#include <math.h>

char g_biosDir[1024];

void studio_init(Studio *s)
{
    if (!g_biosDir[0] && getenv("MMS_BIOS")) snprintf(g_biosDir, sizeof(g_biosDir), "%s", getenv("MMS_BIOS"));
    memset(s, 0, sizeof(*s));
    project_init(&s->p);
    s->loopStartFrame = -1;
}

void studio_refresh_voices(Studio *s)
{
    for (int i = 0; i < 128; i++) inst_make(&s->p.voice[i], &s->voices[i]);
}

void studio_measure(Studio *s)
{
    static Engine e;
    eng_init(&e);
    eng_start(&e, &s->song);
    int frames = 0, limit = 60 * 60 * 20, started = -1;
    s->peakVoices = 0;
    uint32_t endClock = s->c.hasLoop ? s->c.loopEndClock : 0xFFFFFFFFu;
    while (e.playing && frames < limit && e.clock < endClock) {
        if (started < 0 && s->c.hasLoop && e.clock >= s->c.loopStartClock) started = frames;
        eng_skip(&e, 1);
        int b = eng_ds_busy(&e);
        if (b > s->peakVoices) s->peakVoices = b;
        frames++;
    }
    s->totalFrames = frames;
    s->loopStartFrame = s->c.hasLoop ? (started < 0 ? 0 : started) : -1;
}

#include "rip.h"

void studio_rip_levels(Studio *s, int frames, double *kai, double *orig)
{
    static Engine e;
    static float buf[ENG_FRAME_SAMPLES * 2];
    *kai = *orig = 0;
    const Rip *r = s->p.rip;
    if (!r || frames <= 0) return;
    eng_init(&e);
    eng_start(&e, &s->song);
    double sum = 0;
    for (int f = 0; f < frames; f++) {
        eng_frame(&e, buf);
        for (int i = 0; i < ENG_FRAME_SAMPLES * 2; i++) sum += (double)buf[i] * buf[i];
    }
    *kai = sqrt(sum / ((double)frames * ENG_FRAME_SAMPLES * 2));
    long n = (long)((double)frames * ENG_FRAME_SAMPLES / ENG_RATE * r->rate);
    if (n > r->pcmFrames) n = r->pcmFrames;
    sum = 0;
    for (long i = 0; i < n * 2; i++) { double v = r->pcm[i] / 32768.0; sum += v * v; }
    *orig = n > 0 ? sqrt(sum / (n * 2.0)) : 0;
}

float studio_match_rip(Studio *s)
{
    if (!s->p.rip || s->p.ripKind == RIP_GSF) return s->p.ripGain;   // GBA songs: the same engine, the same volume
    int anyPcm = 0;
    for (int i = 0; i < s->p.rip->nsamples; i++) if (s->p.rip->samples[i].kind == RIPS_PCM) anyPcm = 1;
    if (!anyPcm) return s->p.ripGain;     // Game Boy channels play at the hardware's own volume
    int frames = s->totalFrames > 0 ? s->totalFrames : 60 * 60;
    if (frames > 60 * 60) frames = 60 * 60;
    for (int pass = 0; pass < 2; pass++) {
        double kai, orig;
        studio_rip_levels(s, frames, &kai, &orig);
        if (kai <= 1e-7 || orig <= 1e-7) break;
        float g = s->p.ripGain * (float)(orig / kai);
        if (g > 64) g = 64;
        if (fabsf(g / s->p.ripGain - 1) < 0.01f) break;
        char err[256];
        if (!project_rip_regain(&s->p, g, err, sizeof(err))) break;
        if (!studio_rebuild(s)) break;
    }
    return s->p.ripGain;
}

int studio_rebuild(Studio *s)
{
    project_bind(&s->p);
    if (s->haveConv) conv_free(&s->c);
    s->haveConv = 0;
    memset(&s->song, 0, sizeof(s->song));
    ConvOpts o;
    project_conv_opts(&s->p, &o);
    if (!conv_build(&s->c, &s->p.midi, &o, s->msg, sizeof(s->msg)))
        return 0;
    s->haveConv = 1;
    project_default_voices(&s->p, &s->c, 0);
    // with the instruments chosen: drum notes move to sounds the kits have, and each instrument's
    // volume is evened out, so convert once more
    for (int i = 0; i < 128; i++) s->gain[i] = 1.0f;
    if (s->p.gmFixes && s->p.balance)
        for (int i = 0; i < 128; i++)
            if (s->c.slotUsed[i]) s->gain[i] = voice_gm_gain(&s->p.voice[i]);
    // rips: a General MIDI instrument picked for an original one is brought to the original's loudness
    int ripGains = 0;
    if (s->p.isRip && s->p.gmFixes)
        for (int i = 0; i < 128; i++)
            if (s->c.slotUsed[i] && s->p.ripGm[i].type != 0xFF && voice_same(&s->p.voice[i], &s->p.ripGm[i])) {
                s->gain[i] = s->p.ripGmGain[i];
                ripGains = 1;
            }
    o.slotGain = (s->p.gmFixes && s->p.balance) || ripGains ? s->gain : NULL;
    if (s->p.gmFixes || ripGains) {
        conv_free(&s->c);
        if (!conv_build(&s->c, &s->p.midi, &o, s->msg, sizeof(s->msg))) { s->haveConv = 0; return 0; }
    }
    studio_refresh_voices(s);
    s->song.ntracks = s->c.ntracks;
    for (int i = 0; i < s->c.ntracks; i++) {
        s->song.data[i] = s->c.data[i];
        s->song.len[i] = s->c.len[i];
    }
    s->song.trackLimit = s->p.moreTracks ? ENG_MAX_TRACKS : ENG_GAME_TRACKS;
    s->song.dsVoices = s->p.voices;
    s->song.priority = (uint8_t)s->p.priority;
    s->song.reverb = project_reverb_byte(&s->p);
    s->song.voices = s->voices;
    studio_measure(s);
    s->msg[0] = 0;
    return 1;
}
