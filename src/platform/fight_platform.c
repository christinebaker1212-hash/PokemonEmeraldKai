// Moemon Fight: the fight engine's picture, buttons and sound on the game's SDL2 side
// (include/mf_engine.h; the engine itself is in fight/). While a fight runs its 480x320 picture is
// the game picture (Fight_Canvas, like the card screens), the fight's sound replaces the game's
// (Fight_MixAudio) and friends spectating see a 240x160 copy (Fight_Frame).
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "mf_engine.h"
#include "platform/fight.h"

static SDL_Texture *sCanvas;
static SDL_Renderer *sCanvasOwner;
static u16 sGbaFrame[DISPLAY_WIDTH * DISPLAY_HEIGHT];

// The arcade layout (docs/FIGHT_DECISIONS.md #21): pad X Y RB = x y z, A B RT = a b c; keys U I O / J K L
unsigned short Fight_PollButtons(void)
{
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    u16 b = 0;
    int i;

    if (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W])
        b |= MF_BTN_UP;
    if (k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S])
        b |= MF_BTN_DOWN;
    if (k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A])
        b |= MF_BTN_LEFT;
    if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D])
        b |= MF_BTN_RIGHT;
    if (k[SDL_SCANCODE_U])
        b |= MF_BTN_X;
    if (k[SDL_SCANCODE_I])
        b |= MF_BTN_Y;
    if (k[SDL_SCANCODE_O])
        b |= MF_BTN_Z;
    if (k[SDL_SCANCODE_J])
        b |= MF_BTN_A;
    if (k[SDL_SCANCODE_K])
        b |= MF_BTN_B;
    if (k[SDL_SCANCODE_L])
        b |= MF_BTN_C;
    if (k[SDL_SCANCODE_RETURN])
        b |= MF_BTN_START;

    for (i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_GameController *pad;
        float x, y;
        if (!SDL_IsGameController(i) || (pad = SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(i))) == NULL)
            continue;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP))
            b |= MF_BTN_UP;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
            b |= MF_BTN_DOWN;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
            b |= MF_BTN_LEFT;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
            b |= MF_BTN_RIGHT;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_X))
            b |= MF_BTN_X;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_Y))
            b |= MF_BTN_Y;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
            b |= MF_BTN_Z;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A))
            b |= MF_BTN_A;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))
            b |= MF_BTN_B;
        if (SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000)
            b |= MF_BTN_C;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START))
            b |= MF_BTN_START;
        // the left stick in eight directions (fighting games need the diagonals)
        x = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
        y = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
        if (x < -0.5f)
            b |= MF_BTN_LEFT;
        if (x > 0.5f)
            b |= MF_BTN_RIGHT;
        if (y < -0.5f)
            b |= MF_BTN_UP;
        if (y > 0.5f)
            b |= MF_BTN_DOWN;
    }
    return b;
}

SDL_Texture *Fight_Canvas(SDL_Renderer *r)
{
    const unsigned short *frame;

    if (!MfEngine_IsActive())
        return NULL;
    if (sCanvas && sCanvasOwner != r)
        sCanvas = NULL;
    if (!sCanvas)
    {
        sCanvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, MF_SCREEN_W, MF_SCREEN_H);
        sCanvasOwner = r;
        if (!sCanvas)
            return NULL;
        SDL_SetTextureBlendMode(sCanvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(sCanvas, SDL_ScaleModeNearest);
    }
    frame = MfEngine_Frame();
    if (frame)
    {
        SDL_UpdateTexture(sCanvas, NULL, frame, MF_SCREEN_W * 2);
    }
    else
    {
        // still loading: black
        static const unsigned short sBlack[MF_SCREEN_W * 8];
        SDL_Rect band = {0, 0, MF_SCREEN_W, 8};
        for (band.y = 0; band.y < MF_SCREEN_H; band.y += 8)
            SDL_UpdateTexture(sCanvas, &band, sBlack, MF_SCREEN_W * 2);
    }
    return sCanvas;
}

void Fight_DestroyTextures(void)
{
    if (sCanvas)
        SDL_DestroyTexture(sCanvas);
    sCanvas = NULL;
    sCanvasOwner = NULL;
}

// The fight as a GBA frame (spectating, screenshots): each 2x2 block averaged into one 15-bit pixel
const unsigned short *Fight_Frame(void)
{
    const unsigned short *f;
    int x, y;

    if (!MfEngine_IsActive() || (f = MfEngine_Frame()) == NULL)
        return NULL;
    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
        const unsigned short *row = f + (y * 2) * MF_SCREEN_W;
        for (x = 0; x < DISPLAY_WIDTH; x++)
        {
            unsigned short p[4] = {row[x * 2], row[x * 2 + 1], row[x * 2 + MF_SCREEN_W], row[x * 2 + 1 + MF_SCREEN_W]};
            int r = 0, g = 0, b = 0, i;
            for (i = 0; i < 4; i++)
            {
                r += p[i] >> 11;
                g += (p[i] >> 6) & 31;
                b += p[i] & 31;
            }
            sGbaFrame[y * DISPLAY_WIDTH + x] = (u16)((r / 4) | ((g / 4) << 5) | ((b / 4) << 10));
        }
    }
    return sGbaFrame;
}

// While a fight runs its sound replaces the game's (frames: stereo float samples at the game's rate)
void Fight_MixAudio(float *stereo, int frames, int rate)
{
    static short sMix[4096 * 2];
    int i;

    if (!MfEngine_IsActive())
        return;
    while (frames > 0)
    {
        int n = frames > 4096 ? 4096 : frames;
        memset(sMix, 0, (size_t)n * 2 * sizeof(short));
        MfEngine_MixAudio(sMix, n, rate);
        for (i = 0; i < n * 2; i++)
            stereo[i] = sMix[i] / 32768.0f;
        stereo += n * 2;
        frames -= n;
    }
}
