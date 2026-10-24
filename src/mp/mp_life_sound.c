// Moemon Co-op: MOéMON GARDEN, the Garden's own quiet sounds (docs/POKEMON_GARDEN.md 9).
//
// A Pokémon's battle emote gets a soft "bloop", and a rare battle line a short bell chime. Neither is one
// of the game's sound effects, so they can't be mistaken for anything else (the Trainers' battle emotes
// are silent). They're synthesized here and mixed into the game's output each frame, after the m4a and
// CGB channels and the GENESIS / EPOCH BALLS' sounds (music_player.c m4aSoundVSync).
#include "global.h"
#include "mp.h"
#include "mp_life_sound.h"

#define RATE   42048.0f   // the port's output rate (sdl2.c)
#define VOICES 4
#define TWO_PI 6.2831853f

#define PI 3.14159265f

// No libm in every build (mp_candy.c has its own sqrt too): a parabolic sine with one refinement step
// (error under 0.1%) and exp for x <= 0 as (1 + x/256)^256
static float Sin(float x)
{
    float y;
    while (x > PI)
        x -= TWO_PI;
    while (x < -PI)
        x += TWO_PI;
    y = (4.0f / PI) * x - (4.0f / (PI * PI)) * x * (x < 0 ? -x : x);
    return 0.225f * (y * (y < 0 ? -y : y) - y) + y;
}

static float ExpNeg(float x)
{
    float v = 1.0f + x / 256.0f;
    u8 i;
    if (v <= 0)
        return 0;
    for (i = 0; i < 8; i++)
        v *= v;
    return v;
}

static const float sLength[LIFE_SFX_COUNT] = {
    [LIFE_SFX_BLOOP_UP] = 0.11f,
    [LIFE_SFX_BLOOP_DOWN] = 0.12f,
    [LIFE_SFX_CHIME] = 0.75f,
};

static struct {
    s8 id;          // -1: free
    u32 pos;        // samples played
    float phase;    // the bloop's gliding sine
} sVoices[VOICES] = {{-1}, {-1}, {-1}, {-1}};

void MP_LifeSfx_Play(u8 id)
{
    u8 i, slot = 0;
    if (id >= LIFE_SFX_COUNT)
        return;
    for (i = 0; i < VOICES; i++)
        if (sVoices[i].id < 0)
            break;
    slot = i < VOICES ? i : 0;
    sVoices[slot].id = id;
    sVoices[slot].pos = 0;
    sVoices[slot].phase = 0;
}

bool8 MP_LifeSfx_Playing(void)
{
    u8 i;
    for (i = 0; i < VOICES; i++)
        if (sVoices[i].id >= 0)
            return TRUE;
    return FALSE;
}

// A soft rounded blip that glides up (happy) or down (worried): a sine with a quick attack and a short decay
static float Bloop(float t, float *phase, bool8 up)
{
    float len = up ? sLength[LIFE_SFX_BLOOP_UP] : sLength[LIFE_SFX_BLOOP_DOWN];
    float k = t / len;
    float f = up ? 620.0f + 520.0f * k : 980.0f - 380.0f * k;
    float env = (t < 0.006f ? t / 0.006f : 1.0f) * (1.0f - k) * (1.0f - k);
    *phase += TWO_PI * f / RATE;
    if (*phase > TWO_PI)
        *phase -= TWO_PI;
    return Sin(*phase) * env * 0.085f;
}

// 2 pi f t, kept small (t * f grows to thousands of turns; only the fraction matters)
static float Phase(float f, float t)
{
    float turns = f * t;
    return TWO_PI * (turns - (float)(s32)turns);
}

// One bell: a sine and a faint inharmonic partial, struck and ringing out
static float Bell(float t, float f)
{
    float env;
    if (t < 0)
        return 0;
    env = (t < 0.004f ? t / 0.004f : 1.0f) * ExpNeg(-t * 6.5f);
    return (Sin(Phase(f, t)) + 0.28f * Sin(Phase(f * 2.76f, t))) * env;
}

// The battle line's chime: three bells rising (C6, E6, G6), then the C an octave up, quietly
static float Chime(float t)
{
    float v = Bell(t, 1046.5f) + Bell(t - 0.075f, 1318.5f) + Bell(t - 0.15f, 1568.0f) + 0.8f * Bell(t - 0.225f, 2093.0f);
    return v * 0.075f;
}

// buffer: interleaved stereo floats, `count` floats (2 per sample frame)
void MP_LifeSfx_Mix(float *buffer, s32 count)
{
    u8 i;
    for (i = 0; i < VOICES; i++)
    {
        s32 n;
        u32 len;
        if (sVoices[i].id < 0)
            continue;
        len = (u32)(sLength[sVoices[i].id] * RATE);
        for (n = 0; n + 1 < count && sVoices[i].pos < len; n += 2)
        {
            float t = sVoices[i].pos++ / RATE, v;
            switch (sVoices[i].id)
            {
            case LIFE_SFX_BLOOP_UP: v = Bloop(t, &sVoices[i].phase, TRUE); break;
            case LIFE_SFX_BLOOP_DOWN: v = Bloop(t, &sVoices[i].phase, FALSE); break;
            default: v = Chime(t); break;
            }
            buffer[n] += v;
            buffer[n + 1] += v;
        }
        if (sVoices[i].pos >= len)
            sVoices[i].id = -1;
    }
}
