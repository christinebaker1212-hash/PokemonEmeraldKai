#ifndef GUARD_MP_LIFE_SOUND_H
#define GUARD_MP_LIFE_SOUND_H

// Moemon Co-op: MOéMON GARDEN's quiet sounds (src/mp/mp_life_sound.c)
enum
{
    LIFE_SFX_BLOOP_UP,      // a Pokémon's battle emote: happy, proud, fond
    LIFE_SFX_BLOOP_DOWN,    // ... worried, sad
    LIFE_SFX_CHIME,         // a rare battle line
    LIFE_SFX_COUNT
};

void MP_LifeSfx_Play(u8 id);
bool8 MP_LifeSfx_Playing(void);
void MP_LifeSfx_Mix(float *buffer, s32 count);

#endif // GUARD_MP_LIFE_SOUND_H
