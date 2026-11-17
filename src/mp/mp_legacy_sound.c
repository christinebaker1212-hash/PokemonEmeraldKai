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

// GB OPTIONS > MOVES (mp_gb_moves.c): a move's sound from the pack (u32 samples, then 4-bit IMA ADPCM at 21024 Hz, each
// sample played twice at the port's 42048). One at a time: a sound starting takes the Game Boy's sound channels from
// the one playing, as the games' own did.
static struct {
    const u8 *data;
    u32 count, pos;
    s32 pred, index;
    s16 cur;
    u8 half;
} sMoveVoice;

void MP_LegacySfx_PlayMove(const u8 *sound)
{
    sMoveVoice.count = sound[0] | (sound[1] << 8) | (sound[2] << 16) | ((u32)sound[3] << 24);
    sMoveVoice.data = sound + 4;
    sMoveVoice.pos = 0;
    sMoveVoice.pred = 0;
    sMoveVoice.index = 0;
    sMoveVoice.half = 0;
}

static s16 MoveVoiceNext(void)
{
    static const s16 sSteps[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107,
        118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876,
        963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086,
        29794, 32767};
    static const s8 sIndex[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
    u8 code;
    s32 step, delta;
    if (sMoveVoice.half)
    {
        sMoveVoice.half = 0;
        return sMoveVoice.cur;
    }
    code = (sMoveVoice.data[sMoveVoice.pos >> 1] >> ((sMoveVoice.pos & 1) * 4)) & 15;
    step = sSteps[sMoveVoice.index];
    delta = step >> 3;
    if (code & 4) delta += step;
    if (code & 2) delta += step >> 1;
    if (code & 1) delta += step >> 2;
    sMoveVoice.pred += (code & 8) ? -delta : delta;
    if (sMoveVoice.pred > 32767) sMoveVoice.pred = 32767;
    if (sMoveVoice.pred < -32768) sMoveVoice.pred = -32768;
    sMoveVoice.index += sIndex[code & 7];
    if (sMoveVoice.index < 0) sMoveVoice.index = 0;
    if (sMoveVoice.index > 88) sMoveVoice.index = 88;
    sMoveVoice.pos++;
    sMoveVoice.half = 1;
    sMoveVoice.cur = sMoveVoice.pred;
    return sMoveVoice.cur;
}

// buffer: interleaved stereo floats, `count` floats (2 per sample frame)
void MP_LegacySfx_Mix(float *buffer, s32 count)
{
    if (sMoveVoice.data != NULL)
    {
        s32 n;
        for (n = 0; n + 1 < count && (sMoveVoice.pos < sMoveVoice.count || sMoveVoice.half); n += 2)
        {
            float v = MoveVoiceNext() / 32768.0f * 0.9f;
            buffer[n] += v;
            buffer[n + 1] += v;
        }
        if (sMoveVoice.pos >= sMoveVoice.count && !sMoveVoice.half)
            sMoveVoice.data = NULL;
    }
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
}

// UI STYLE: RED's / CRYSTAL's party SWITCH sound (SFX_SWAP / SFX_SWITCH_POKEMON)
void MP_LegacySfx_PlaySwap(bool8 crystal)
{
#ifdef HAVE_LEGACY_SFX
    MP_LegacySfx_Play(crystal ? LEGACY_SFX_GSC_SWITCH_POKEMON : LEGACY_SFX_RED_SWAP, 0);
#else
    (void)crystal;
#endif
}
