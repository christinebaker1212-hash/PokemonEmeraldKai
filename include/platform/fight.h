// Moemon Fight: the fight engine on the platform side (src/platform/fight_platform.c)
#ifndef GUARD_PLATFORM_FIGHT_H
#define GUARD_PLATFORM_FIGHT_H

struct SDL_Renderer;
struct SDL_Texture;

// The local player's MF_BTN_* buttons (keyboard and every controller)
unsigned short Fight_PollButtons(void);
// The fight's 480x320 picture while one runs, else NULL
struct SDL_Texture *Fight_Canvas(struct SDL_Renderer *r);
// The fight's picture scaled bicubic towards w x h (at most twice its size: the renderer's linear filter does
// the rest), else NULL (display.c)
struct SDL_Texture *Fight_CanvasBicubic(struct SDL_Renderer *r, int w, int h);
void Fight_DestroyTextures(void);
// The fight as a 240x160 GBA frame, else NULL (spectating)
const unsigned short *Fight_Frame(void);
// Replaces the game's sound with the fight's while one runs
void Fight_MixAudio(float *stereo, int frames, int rate);

#endif // GUARD_PLATFORM_FIGHT_H
