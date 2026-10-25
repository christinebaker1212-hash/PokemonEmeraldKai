#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "studio.h"
#include "inst.h"

void studio_init(Studio *s)
{
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

int studio_rebuild(Studio *s)
{
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
    o.slotGain = s->p.gmFixes && s->p.balance ? s->gain : NULL;
    if (s->p.gmFixes) {
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
