// Moemon Fight: the fight engine's picture, buttons and sound on the game's SDL2 side
// (include/mf_engine.h; the engine itself is in fight/). While a fight runs its 480x320 picture is
// the game picture (Fight_Canvas, like the card screens), the fight's sound replaces the game's
// (Fight_MixAudio) and friends spectating see a 240x160 copy (Fight_Frame).
#include <math.h>
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

// Bicubic (Catmull-Rom) scaling of the fight's picture, separable, with the taps and weights of each output
// column and row worked out once per size (weights in 1/256: 32-bit sums, a shift at the end)
#define BICUBIC_MAX_W (MF_SCREEN_W * 2)
#define BICUBIC_MAX_H (MF_SCREEN_H * 2)
#define BICUBIC_SHIFT 8
#define BICUBIC_ONE (1 << BICUBIC_SHIFT)

struct BicubicTaps
{
    s16 at[4];
    s16 weight[4];
};

static SDL_Texture *sBicubic;
static SDL_Renderer *sBicubicOwner;
static int sBicubicW, sBicubicH;
static unsigned int sBicubicSerial;
static struct BicubicTaps sTapsX[BICUBIC_MAX_W], sTapsY[BICUBIC_MAX_H];
static s32 sPlanes[3][MF_SCREEN_W * MF_SCREEN_H];   // the picture as 8-bit R, G, B
static s32 sRows[3][BICUBIC_MAX_W * MF_SCREEN_H];    // after the horizontal pass (x BICUBIC_ONE)
static u8 sOut[3][BICUBIC_MAX_W * BICUBIC_MAX_H];    // after the vertical pass

static double CatmullRom(double t)
{
    t = t < 0 ? -t : t;
    if (t < 1.0)
        return 1.5 * t * t * t - 2.5 * t * t + 1.0;
    if (t < 2.0)
        return -0.5 * t * t * t + 2.5 * t * t - 4.0 * t + 2.0;
    return 0.0;
}

static void BicubicTaps(struct BicubicTaps *taps, int out, int in)
{
    int i, k;

    for (i = 0; i < out; i++)
    {
        double center = (i + 0.5) * in / out - 0.5;
        int base = (int)floor(center);
        int sum = 0;
        for (k = 0; k < 4; k++)
        {
            int at = base - 1 + k;
            taps[i].at[k] = (s16)(at < 0 ? 0 : at >= in ? in - 1 : at);
            taps[i].weight[k] = (s16)lround(CatmullRom(center - (base - 1 + k)) * BICUBIC_ONE);
            sum += taps[i].weight[k];
        }
        taps[i].weight[1] += BICUBIC_ONE - sum;   // the weights add up exactly
    }
}

SDL_Texture *Fight_CanvasBicubic(SDL_Renderer *r, int w, int h)
{
    const unsigned short *frame;
    unsigned int serial;
    void *pixels;
    int pitch, x, y, c;

    if (!MfEngine_IsActive() || w <= 0 || h <= 0)
        return NULL;
    // at most twice the picture: the renderer's linear filter takes it the rest of the way
    if (w > BICUBIC_MAX_W || h > BICUBIC_MAX_H)
    {
        w = BICUBIC_MAX_W;
        h = BICUBIC_MAX_H;
    }
    if (sBicubic && (sBicubicOwner != r || sBicubicW != w || sBicubicH != h))
    {
        if (sBicubicOwner == r)
            SDL_DestroyTexture(sBicubic);
        sBicubic = NULL;
    }
    if (!sBicubic)
    {
        sBicubic = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (!sBicubic)
            return NULL;
        sBicubicOwner = r;
        sBicubicW = w;
        sBicubicH = h;
        sBicubicSerial = MfEngine_FrameSerial() - 1;
        SDL_SetTextureBlendMode(sBicubic, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(sBicubic, SDL_ScaleModeLinear);
        BicubicTaps(sTapsX, w, MF_SCREEN_W);
        BicubicTaps(sTapsY, h, MF_SCREEN_H);
    }
    frame = MfEngine_Frame();
    serial = MfEngine_FrameSerial();
    if (!frame || serial == sBicubicSerial)
        return sBicubic;   // nothing new (or still loading: what it showed)
    sBicubicSerial = serial;

    for (x = 0; x < MF_SCREEN_W * MF_SCREEN_H; x++)
    {
        unsigned short p = frame[x];
        sPlanes[0][x] = ((p >> 11) << 3) | (p >> 13);
        sPlanes[1][x] = (((p >> 5) & 63) << 2) | ((p >> 9) & 3);
        sPlanes[2][x] = ((p & 31) << 3) | ((p >> 2) & 7);
    }
    for (c = 0; c < 3; c++)
    {
        for (y = 0; y < MF_SCREEN_H; y++)
        {
            const s32 *in = sPlanes[c] + y * MF_SCREEN_W;
            s32 *out = sRows[c] + y * w;
            int fastFrom = w, fastTo = w;
            if (w == MF_SCREEN_W * 2)
            {
                // exactly twice as wide (any window of 960 or more): every even column has the same weights on
                // pixels i-2..i+1, every odd one on i-1..i+2, so no lookups away from the edges (5x quicker)
                const s16 *we = sTapsX[4].weight, *wo = sTapsX[5].weight;
                s32 e0 = we[0], e1 = we[1], e2 = we[2], e3 = we[3], o0 = wo[0], o1 = wo[1], o2 = wo[2], o3 = wo[3];
                int i;
                fastFrom = 4;
                fastTo = (MF_SCREEN_W - 2) * 2;
                for (i = 2; i < MF_SCREEN_W - 2; i++)
                {
                    out[i * 2] = in[i - 2] * e0 + in[i - 1] * e1 + in[i] * e2 + in[i + 1] * e3;
                    out[i * 2 + 1] = in[i - 1] * o0 + in[i] * o1 + in[i + 1] * o2 + in[i + 2] * o3;
                }
            }
            for (x = 0; x < w; x++)
            {
                const struct BicubicTaps *t;
                if (x == fastFrom)
                    x = fastTo;
                t = &sTapsX[x];
                out[x] = in[t->at[0]] * t->weight[0] + in[t->at[1]] * t->weight[1]
                       + in[t->at[2]] * t->weight[2] + in[t->at[3]] * t->weight[3];
            }
        }
    }
    // the vertical pass a channel at a time (contiguous rows: the compiler vectorizes it), then packed
    for (c = 0; c < 3; c++)
    {
        for (y = 0; y < h; y++)
        {
            const struct BicubicTaps *t = &sTapsY[y];
            const s32 *r0 = sRows[c] + t->at[0] * w, *r1 = sRows[c] + t->at[1] * w;
            const s32 *r2 = sRows[c] + t->at[2] * w, *r3 = sRows[c] + t->at[3] * w;
            s32 w0 = t->weight[0], w1 = t->weight[1], w2 = t->weight[2], w3 = t->weight[3];
            u8 *out = sOut[c] + y * w;
            for (x = 0; x < w; x++)
            {
                s32 v = (r0[x] * w0 + r1[x] * w1 + r2[x] * w2 + r3[x] * w3 + (1 << (2 * BICUBIC_SHIFT - 1)))
                      >> (2 * BICUBIC_SHIFT);
                out[x] = (u8)(v < 0 ? 0 : v > 255 ? 255 : v);
            }
        }
    }
    if (SDL_LockTexture(sBicubic, NULL, &pixels, &pitch) != 0)
        return sBicubic;
    for (y = 0; y < h; y++)
    {
        u32 *out = (u32 *)((u8 *)pixels + y * pitch);
        const u8 *rr = sOut[0] + y * w, *gg = sOut[1] + y * w, *bb = sOut[2] + y * w;
        for (x = 0; x < w; x++)
            out[x] = 0xFF000000u | ((u32)rr[x] << 16) | ((u32)gg[x] << 8) | (u32)bb[x];
    }
    SDL_UnlockTexture(sBicubic);
    return sBicubic;
}

void Fight_DestroyTextures(void)
{
    if (sBicubic)
        SDL_DestroyTexture(sBicubic);
    sBicubic = NULL;
    sBicubicOwner = NULL;
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
