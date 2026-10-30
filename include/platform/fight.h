// Moemon Fight: the fight engine on the platform side (src/platform/fight_platform.c)
#ifndef GUARD_PLATFORM_FIGHT_H
#define GUARD_PLATFORM_FIGHT_H

struct SDL_Renderer;
struct SDL_Texture;

// The local player's MF_BTN_* buttons (keyboard and every controller)
unsigned short Fight_PollButtons(void);
// The fight buttons' keys and pad buttons (OPTIONS > CONTROLS > FIGHT BUTTONS; saved by gamepad.c)
enum { FIGHT_BIND_X, FIGHT_BIND_Y, FIGHT_BIND_Z, FIGHT_BIND_A, FIGHT_BIND_B, FIGHT_BIND_C, FIGHT_BIND_START, FIGHT_BIND_COUNT };
void Fight_BindListen(int which);           // the next key or pad button pressed becomes this one's
int Fight_BindListening(void);              // the fight button being chosen + 1, else 0
int Fight_BindPoll(void);                   // every frame while choosing: 1 chosen, -1 cancelled (Esc / Back / 8 s), 0
void Fight_BindDefaults(void);
int Fight_BindIsDefault(void);
const char *Fight_BindKeyName(int which);
const char *Fight_BindPadName(int which);
int Fight_BindSetting(const char *key, const char *val);   // a moemon_controls.ini line: TRUE when it was one of these
void Fight_BindWrite(void *file);                         // a FILE *
// The fight's 480x320 picture while one runs, else NULL: display.c shows it as the game picture, scaled and
// filtered as the game's 240x160 is
#define FIGHT_CANVAS_W 480
#define FIGHT_CANVAS_H 320
struct SDL_Texture *Fight_Canvas(struct SDL_Renderer *r);
int Fight_SaveCanvasBMP(const char *path);  // tests: that picture as a BMP (0 saved)
void Fight_DestroyTextures(void);
// The fight as a 240x160 GBA frame, else NULL (spectating)
const unsigned short *Fight_Frame(void);
// Replaces the game's sound with the fight's while one runs
void Fight_MixAudio(float *stereo, int frames, int rate);
// The game's messages over the fight (text: "" none; badSignal: the bad connection icon)
void Fight_SetOverlay(const char *text, int badSignal);
// The fight is the game picture (else the game's own, fading to black for it)
void Fight_Show(int on);
// The CRT effect: phase 0 none, 1 turning on, 2 turning off; time of length frames
void Fight_Crt(int phase, int time, int length);
// Keeps showing the last picture once the engine has stopped (while the CRT turns off)
void Fight_Hold(int on);
// Milliseconds since the game started (the online fight's timing)
unsigned int Fight_Ms(void);

#endif // GUARD_PLATFORM_FIGHT_H
