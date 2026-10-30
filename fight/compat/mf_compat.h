/*
 * Moemon Fight: what the compat layer (the engine's SDL 1.2 / SDL_mixer / SDL_ttf) and the host
 * (fight/host/mf_host.c) give each other. Engine thread side; SDL 1.2 types.
 */
#ifndef MF_COMPAT_H
#define MF_COMPAT_H

#include <SDL.h>

/* mf_sdl12.c: key events the engine reads with SDL_PollEvent (engine thread only) */
void MfSdl_PushKey(int sym, int down);
void MfSdl_ClearEvents(void);

/* mf_host.c: the video surface was shown (SDL_UpdateRect / SDL_Flip of the SDL_SetVideoMode surface) */
void MfHost_Present(const SDL_Surface *surface);

/* mf_mixer.c: the mixer's output rate, and its sound added to the game's (audio thread) */
#define MF_MIX_RATE 13379           /* the GBA's DirectSound rate of Emerald's samples */
void MfMix_Render(short *stereo, int frames, int rate);
void MfMix_Reset(void);

/* mf_mixer.c: stage music is kept under the fighters' voices (FIGHT_DECISIONS "Music under the voices").
 * Levels are RMS on a 0..1 scale; 0 = not known (the music then plays as it is). */
#define MF_BGM_UNDER_VOICES_DB 3.0  /* how far the stage music stays under the voices */
double MfMix_ClipLevel(const short *stereo, int frames);    /* a 13379 Hz sound's loudness while it sounds */
double MfMix_GatedLevel(double *ms, int count);           /* a song's from the mean squares of its 0.4 s blocks */
void MfMix_SetVoiceLevel(double level);     /* engine thread: the fighters' voices at the volume they play */
void MfMix_SetMusicLevel(double level);     /* BGM thread: the song just opened, at full volume */
double MfMix_MusicGain(void);               /* what the stage music is multiplied by now */

#endif /* MF_COMPAT_H */
