// Moemon Kai PC: SDL side of the display module, used only by src/platform/sdl2.c.
#ifndef GUARD_PLATFORM_DISPLAY_SDL_H
#define GUARD_PLATFORM_DISPLAY_SDL_H

#include <SDL2/SDL.h>

// Load moemon_display.ini (or $MOEMON_DISPLAY_CONFIG) and set SDL hints.
// Call before SDL_Init.
void Display_PreInit(void);
// Create the window and renderer from the loaded settings. Returns 0 on success.
int Display_Init(const char *title);
void Display_Shutdown(void);
// Returns 1 if the event was consumed (F11, Alt+Enter).
int Display_HandleEvent(const SDL_Event *event);
// pixels: 240x160 ABGR1555. The pointer must stay valid (it is re-uploaded
// if the renderer has to be recreated).
void Display_UploadFrame(const void *pixels);
int Display_NeedsPresent(void);
int Display_GetRefreshRate(void);
void Display_Present(void);
// Test harness helpers
int Display_SaveOutputBMP(const char *path);
void Display_TestSetWindowSize(int w, int h);
void Display_TestCommand(const char *key, int value);

#endif // GUARD_PLATFORM_DISPLAY_SDL_H
