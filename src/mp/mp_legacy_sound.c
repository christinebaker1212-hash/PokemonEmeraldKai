// Moemon Co-op: plays the Game Boy sound effects of the GENESIS / EPOCH BALLS.
//
// The sounds are Pokémon Red's and Crystal's own, rendered offline from their
// sound engines and a Game Boy APU model (tools/moemon_legacy/sfx_render.py ->
// mp_legacy_sfx.h, mono 16-bit at the port's 42048 Hz). They're mixed into the
// game's output each frame, after the m4a and CGB channels (music_player.c
// m4aSoundVSync), with Crystal's left / right panning when the sound option is
// STEREO.
#include "global.h"
#include "mp.h"

#if defined(__has_include)
#if __has_include("mp_legacy_sfx.h")
#include "mp_legacy_sfx.h"
#define HAVE_LEGACY_SFX 1
#endif
#endif

#define VOICES 4

static struct {
    s16 id;      // -1: free
    u32 pos;
    u8 pan;      // 0 centre, 1 left, 2 right
} sVoices[VOICES] = {{-1}, {-1}, {-1}, {-1}};

void MP_LegacySfx_Play(u8 id, u8 pan)
{
#ifdef HAVE_LEGACY_SFX
    u8 i, slot = 0;
    if (id >= LEGACY_SFX_COUNT)
        return;
    // a Game Boy channel plays one thing: a sound takes its channels from whatever was on them
    // (the same sound again restarts it)
    for (i = 0; i < VOICES; i++)
        if (sVoices[i].id >= 0 && (sLegacySfx[sVoices[i].id].channels & sLegacySfx[id].channels))
            sVoices[i].id = -1;
    for (i = 0; i < VOICES; i++)
        if (sVoices[i].id < 0)
            break;
    slot = i < VOICES ? i : 0;
    sVoices[slot].id = id;
    sVoices[slot].pos = 0;
    sVoices[slot].pan = pan;
#else
    (void)id;
    (void)pan;
#endif
}

bool8 MP_LegacySfx_Playing(void)
{
    u8 i;
    for (i = 0; i < VOICES; i++)
        if (sVoices[i].id >= 0)
            return TRUE;
    return FALSE;
}

void MP_LegacySfx_StopAll(void)
{
    u8 i;
    for (i = 0; i < VOICES; i++)
        sVoices[i].id = -1;
}

// buffer: interleaved stereo floats, `count` floats (2 per sample frame)
void MP_LegacySfx_Mix(float *buffer, s32 count)
{
#ifdef HAVE_LEGACY_SFX
    u8 i;
    bool8 stereo = gSaveBlock2Ptr != NULL && gSaveBlock2Ptr->optionsSound;
    for (i = 0; i < VOICES; i++)
    {
        s32 n;
        const s16 *data;
        u32 len;
        float l, r;
        if (sVoices[i].id < 0)
            continue;
        data = sLegacySfx[sVoices[i].id].data;
        len = sLegacySfx[sVoices[i].id].length;
        l = r = 1.0f;
        if (stereo && sVoices[i].pan == 1)
            r = 0.0f, l = 1.4f;
        else if (stereo && sVoices[i].pan == 2)
            l = 0.0f, r = 1.4f;
        for (n = 0; n + 1 < count && sVoices[i].pos < len; n += 2)
        {
            float v = data[sVoices[i].pos++] / 32768.0f;
            buffer[n] += v * l;
            buffer[n + 1] += v * r;
        }
        if (sVoices[i].pos >= len)
            sVoices[i].id = -1;
    }
#else
    (void)buffer;
    (void)count;
#endif
}
