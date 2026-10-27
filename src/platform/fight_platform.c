// Moemon Fight: the fight engine's picture, buttons and sound on the game's SDL2 side
// (include/mf_engine.h; the engine itself is in fight/). While a fight runs its 480x320 picture is
// the game picture (Fight_Canvas, like the card screens), the fight's sound replaces the game's
// (Fight_MixAudio) and friends spectating see a 240x160 copy (Fight_Frame).
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "mf_engine.h"
#include "platform/fight.h"

static SDL_Texture *sCanvas;
static SDL_Renderer *sCanvasOwner;
static u16 sGbaFrame[DISPLAY_WIDTH * DISPLAY_HEIGHT];

// The fight buttons: the arcade layout (docs/FIGHT_DECISIONS.md #21: pad X Y RB = x y z, A B RT = a b c, Start;
// keys U I O / J K L, Enter), each with its key and its pad button changed on OPTIONS > CONTROLS > FIGHT BUTTONS and
// kept in moemon_controls.ini (gamepad.c). The directions stay the arrows / WASD, the D-pad and the left stick.
#define PAD_LT 100      // the triggers, as pad "buttons"
#define PAD_RT 101

static const unsigned short sBindBits[FIGHT_BIND_COUNT] = {
    MF_BTN_X, MF_BTN_Y, MF_BTN_Z, MF_BTN_A, MF_BTN_B, MF_BTN_C, MF_BTN_START,
};
static const char *const sBindIds[FIGHT_BIND_COUNT] = {"x", "y", "z", "a", "b", "c", "start"};
static const int sKeyDefaults[FIGHT_BIND_COUNT] = {
    SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_RETURN,
};
static const int sPadDefaults[FIGHT_BIND_COUNT] = {
    SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, PAD_RT, SDL_CONTROLLER_BUTTON_START,
};
static int sKeys[FIGHT_BIND_COUNT] = {
    SDL_SCANCODE_U, SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_RETURN,
};
static int sPads[FIGHT_BIND_COUNT] = {
    SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, PAD_RT, SDL_CONTROLLER_BUTTON_START,
};

static SDL_GameController *PadAt(int i)
{
    if (!SDL_IsGameController(i))
        return NULL;
    return SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(i));
}

static int PadHeld(SDL_GameController *pad, int button)
{
    if (button == PAD_LT || button == PAD_RT)
        return SDL_GameControllerGetAxis(pad, button == PAD_LT ? SDL_CONTROLLER_AXIS_TRIGGERLEFT : SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000;
    return SDL_GameControllerGetButton(pad, button);
}

// Keys and pad buttons that can't be fight buttons: the directions, and what cancels choosing one
static int KeyFree(int key)
{
    return key != SDL_SCANCODE_UP && key != SDL_SCANCODE_DOWN && key != SDL_SCANCODE_LEFT && key != SDL_SCANCODE_RIGHT
        && key != SDL_SCANCODE_W && key != SDL_SCANCODE_A && key != SDL_SCANCODE_S && key != SDL_SCANCODE_D
        && key != SDL_SCANCODE_ESCAPE && key != SDL_SCANCODE_F11 && key < SDL_NUM_SCANCODES;
}

static int PadFree(int button)
{
    return button == PAD_LT || button == PAD_RT
        || (button >= 0 && button < SDL_CONTROLLER_BUTTON_MAX && button != SDL_CONTROLLER_BUTTON_BACK
            && button != SDL_CONTROLLER_BUTTON_GUIDE && (button < SDL_CONTROLLER_BUTTON_DPAD_UP || button > SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
}

static int sListen;             // the fight button being chosen + 1
static int sListenTime;
static Uint8 sKeyWas[SDL_NUM_SCANCODES];
static unsigned int sPadWas[8];  // per controller: bit b = button b, bits 30 / 31 the triggers

static unsigned int PadMask(SDL_GameController *pad)
{
    unsigned int m = 0;
    int b;
    for (b = 0; b < SDL_CONTROLLER_BUTTON_MAX && b < 30; b++)
        if (SDL_GameControllerGetButton(pad, b))
            m |= 1u << b;
    if (PadHeld(pad, PAD_LT))
        m |= 1u << 30;
    if (PadHeld(pad, PAD_RT))
        m |= 1u << 31;
    return m;
}

static void Remember(void)
{
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    int i;
    memcpy(sKeyWas, k, sizeof(sKeyWas));
    for (i = 0; i < 8; i++)
    {
        SDL_GameController *pad = i < SDL_NumJoysticks() ? PadAt(i) : NULL;
        sPadWas[i] = pad != NULL ? PadMask(pad) : 0;
    }
}

int Fight_BindListening(void)
{
    return sListen;
}

void Fight_BindListen(int which)
{
    sListen = which >= 0 && which < FIGHT_BIND_COUNT ? which + 1 : 0;
    sListenTime = 0;
    Remember();
}

// Taken from another fight button: that one gets this one's old key / button
static void Bind(int *table, int which, int value)
{
    int i;
    for (i = 0; i < FIGHT_BIND_COUNT; i++)
        if (i != which && table[i] == value)
            table[i] = table[which];
    table[which] = value;
}

int Fight_BindPoll(void)
{
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    int which = sListen - 1, i, b;

    if (!sListen)
        return 0;
    if (k[SDL_SCANCODE_ESCAPE] || ++sListenTime > 60 * 8)
    {
        sListen = 0;
        return -1;
    }
    for (i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        if (k[i] && !sKeyWas[i] && KeyFree(i))
        {
            Bind(sKeys, which, i);
            sListen = 0;
            return 1;
        }
    }
    for (i = 0; i < SDL_NumJoysticks() && i < 8; i++)
    {
        SDL_GameController *pad = PadAt(i);
        unsigned int now, pressed;
        if (pad == NULL)
            continue;
        now = PadMask(pad);
        pressed = now & ~sPadWas[i];
        if (pressed & (1u << SDL_CONTROLLER_BUTTON_BACK))
        {
            sListen = 0;
            return -1;
        }
        for (b = 0; b < 32; b++)
        {
            int button = b == 30 ? PAD_LT : b == 31 ? PAD_RT : b;
            if ((pressed & (1u << b)) && PadFree(button))
            {
                Bind(sPads, which, button);
                sListen = 0;
                return 1;
            }
        }
    }
    Remember();
    return 0;
}

void Fight_BindDefaults(void)
{
    memcpy(sKeys, sKeyDefaults, sizeof(sKeys));
    memcpy(sPads, sPadDefaults, sizeof(sPads));
}

int Fight_BindIsDefault(void)
{
    return memcmp(sKeys, sKeyDefaults, sizeof(sKeys)) == 0 && memcmp(sPads, sPadDefaults, sizeof(sPads)) == 0;
}

const char *Fight_BindKeyName(int which)
{
    static char names[FIGHT_BIND_COUNT][24];
    const char *n = sKeys[which] == SDL_SCANCODE_RETURN ? "Enter" : SDL_GetScancodeName((SDL_Scancode)sKeys[which]);
    char *out = names[which];
    int i;
    for (i = 0; n[i] && i < 23; i++)
        out[i] = n[i] >= 'a' && n[i] <= 'z' ? n[i] - 32 : n[i];
    out[i] = 0;
    return i ? out : "-";
}

const char *Fight_BindPadName(int which)
{
    static const char *const names[] = {
        [SDL_CONTROLLER_BUTTON_A] = "A", [SDL_CONTROLLER_BUTTON_B] = "B", [SDL_CONTROLLER_BUTTON_X] = "X",
        [SDL_CONTROLLER_BUTTON_Y] = "Y", [SDL_CONTROLLER_BUTTON_START] = "START",
        [SDL_CONTROLLER_BUTTON_LEFTSTICK] = "LS", [SDL_CONTROLLER_BUTTON_RIGHTSTICK] = "RS",
        [SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = "LB", [SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = "RB",
    };
    int b = sPads[which];
    if (b == PAD_LT)
        return "LT";
    if (b == PAD_RT)
        return "RT";
    return b >= 0 && b < (int)(sizeof(names) / sizeof(names[0])) && names[b] ? names[b] : "PAD";
}

// moemon_controls.ini: fight_key_x = U (spaces as _), fight_pad_x = x / rightshoulder / righttrigger
int Fight_BindSetting(const char *key, const char *val)
{
    int i, j;
    for (i = 0; i < FIGHT_BIND_COUNT; i++)
    {
        char want[32];
        snprintf(want, sizeof(want), "fight_key_%s", sBindIds[i]);
        if (strcmp(key, want) == 0)
        {
            char name[64];
            SDL_Scancode sc;
            for (j = 0; val[j] && j < 63; j++)
                name[j] = val[j] == '_' ? ' ' : val[j];
            name[j] = 0;
            sc = SDL_GetScancodeFromName(name);
            if (sc != SDL_SCANCODE_UNKNOWN && KeyFree(sc))
                sKeys[i] = sc;
            return 1;
        }
        snprintf(want, sizeof(want), "fight_pad_%s", sBindIds[i]);
        if (strcmp(key, want) == 0)
        {
            SDL_GameControllerButton b = SDL_GameControllerGetButtonFromString(val);
            if (strcmp(val, "lefttrigger") == 0)
                sPads[i] = PAD_LT;
            else if (strcmp(val, "righttrigger") == 0)
                sPads[i] = PAD_RT;
            else if (b != SDL_CONTROLLER_BUTTON_INVALID && PadFree(b))
                sPads[i] = b;
            return 1;
        }
    }
    return 0;
}

void Fight_BindWrite(void *file)
{
    FILE *f = file;
    int i, j;
    fprintf(f, "; the fight buttons (OPTIONS > CONTROLS > FIGHT BUTTONS): x y z punches, a b c kicks\n");
    for (i = 0; i < FIGHT_BIND_COUNT; i++)
    {
        char name[64];
        const char *pad = sPads[i] == PAD_LT ? "lefttrigger" : sPads[i] == PAD_RT ? "righttrigger"
                        : SDL_GameControllerGetStringForButton((SDL_GameControllerButton)sPads[i]);
        snprintf(name, sizeof(name), "%s", SDL_GetScancodeName((SDL_Scancode)sKeys[i]));
        for (j = 0; name[j]; j++)
            if (name[j] == ' ')
                name[j] = '_';
        fprintf(f, "fight_key_%s = %s\nfight_pad_%s = %s\n", sBindIds[i], name, sBindIds[i], pad ? pad : "x");
    }
}

unsigned short Fight_PollButtons(void)
{
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    u16 b = 0;
    int i, j;

    if (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W])
        b |= MF_BTN_UP;
    if (k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S])
        b |= MF_BTN_DOWN;
    if (k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A])
        b |= MF_BTN_LEFT;
    if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D])
        b |= MF_BTN_RIGHT;
    for (j = 0; j < FIGHT_BIND_COUNT; j++)
        if (k[sKeys[j]])
            b |= sBindBits[j];

    for (i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_GameController *pad;
        float x, y;
        if ((pad = PadAt(i)) == NULL)
            continue;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP))
            b |= MF_BTN_UP;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
            b |= MF_BTN_DOWN;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
            b |= MF_BTN_LEFT;
        if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
            b |= MF_BTN_RIGHT;
        for (j = 0; j < FIGHT_BIND_COUNT; j++)
            if (PadHeld(pad, sPads[j]))
                b |= sBindBits[j];
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


// ---------------------------------------------------------------------------
// The game's own messages over the fight (online: waiting for a friend who went quiet, a bad connection),
// drawn over the engine's last picture (it draws nothing while it waits): a 5x7 font at twice its size, on a
// darkened band; the bad connection as a signal icon in the bottom right corner

static const struct { char c; unsigned char rows[7]; } sFont[] = {
    { '!', { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 } },
    { '\'', { 0x0C, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00 } },
    { '-', { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { '.', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C } },
    { '0', { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E } },
    { '1', { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { '2', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F } },
    { '3', { 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E } },
    { '4', { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 } },
    { '5', { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E } },
    { '6', { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E } },
    { '7', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 } },
    { '8', { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E } },
    { '9', { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C } },
    { ':', { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00 } },
    { '?', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 } },
    { 'A', { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'B', { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E } },
    { 'C', { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E } },
    { 'D', { 0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C } },
    { 'E', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F } },
    { 'F', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 } },
    { 'G', { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F } },
    { 'H', { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'I', { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { 'J', { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C } },
    { 'K', { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 } },
    { 'L', { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F } },
    { 'M', { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 } },
    { 'N', { 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11 } },
    { 'O', { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'P', { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 } },
    { 'Q', { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D } },
    { 'R', { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 } },
    { 'S', { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E } },
    { 'T', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 'U', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'V', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'W', { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A } },
    { 'X', { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 } },
    { 'Y', { 0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04 } },
    { 'Z', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F } },
};

static char sOverlayText[64];
static int sOverlaySignal;
static unsigned int sOverlayVersion;
static unsigned short sComposed[MF_SCREEN_W * MF_SCREEN_H];

void Fight_SetOverlay(const char *text, int badSignal)
{
    char up[sizeof(sOverlayText)];
    size_t i;

    for (i = 0; text != NULL && text[i] != '\0' && i < sizeof(up) - 1; i++)
        up[i] = (text[i] >= 'a' && text[i] <= 'z') ? (char)(text[i] - 32) : text[i];
    up[i] = '\0';
    badSignal = badSignal != 0;
    if (strcmp(up, sOverlayText) != 0 || badSignal != sOverlaySignal)
    {
        strcpy(sOverlayText, up);
        sOverlaySignal = badSignal;
        sOverlayVersion++;
    }
}

static const unsigned char *Glyph(char c)
{
    size_t i;

    for (i = 0; i < sizeof(sFont) / sizeof(sFont[0]); i++)
        if (sFont[i].c == c)
            return sFont[i].rows;
    return NULL;
}

static void Darken(int x0, int y0, int w, int h)
{
    int x, y;

    for (y = y0; y < y0 + h; y++)
    {
        unsigned short *row = sComposed + y * MF_SCREEN_W;
        if (y < 0 || y >= MF_SCREEN_H)
            continue;
        for (x = x0; x < x0 + w; x++)
            if (x >= 0 && x < MF_SCREEN_W)
                row[x] = (row[x] >> 2) & 0x39E7;    // a quarter as bright (RGB565)
    }
}

static void Fill(int x0, int y0, int w, int h, unsigned short color)
{
    int x, y;

    for (y = y0; y < y0 + h; y++)
        for (x = x0; x < x0 + w; x++)
            if (x >= 0 && x < MF_SCREEN_W && y >= 0 && y < MF_SCREEN_H)
                sComposed[y * MF_SCREEN_W + x] = color;
}

static void DrawText(const char *text, int x, int y)
{
    for (; *text != '\0'; text++, x += 12)
    {
        const unsigned char *g = Glyph(*text);
        int r, c;
        if (g == NULL)
            continue;
        for (r = 0; r < 7; r++)
            for (c = 0; c < 5; c++)
                if (g[r] & (0x10 >> c))
                {
                    Fill(x + c * 2 + 2, y + r * 2 + 2, 2, 2, 0x0000);   // shadow
                    Fill(x + c * 2, y + r * 2, 2, 2, 0xFFFF);
                }
    }
}

// ---------------------------------------------------------------------------
// The fight's picture as a CRT turning on (a white line widens, then opens into the picture) and off (the picture
// collapses into a line that shrinks to a point), and the last picture held while it turns off

static int sShow;                   // the fight is the game picture (the game faded to black for it)
static int sCrtPhase, sCrtTime, sCrtLen;
static int sHold;
static unsigned short sHeld[MF_SCREEN_W * MF_SCREEN_H];
static unsigned short sCrt[MF_SCREEN_W * MF_SCREEN_H];

void Fight_Show(int on)
{
    sShow = on;
}

void Fight_Crt(int phase, int time, int length)
{
    sCrtPhase = phase;
    sCrtTime = time;
    sCrtLen = length > 0 ? length : 1;
}

static const unsigned short *Fight_Picture(unsigned int *version);

void Fight_Hold(int on)
{
    if (on && !sHold)
    {
        unsigned int version;
        const unsigned short *p = Fight_Picture(&version);
        if (p != NULL)
            memcpy(sHeld, p, sizeof(sHeld));
        else
            memset(sHeld, 0, sizeof(sHeld));
    }
    sHold = on;
}

// The picture is shown (Fight_Canvas): the engine is running or its last picture is held
static int Fight_Showing(void)
{
    return sShow && (MfEngine_IsActive() || sHold);
}

static unsigned short Brighten(unsigned short c, int amount)   // amount 0..256 towards white
{
    int r = c >> 11, g = (c >> 5) & 63, b = c & 31;
    r += (31 - r) * amount >> 8;
    g += (63 - g) * amount >> 8;
    b += (31 - b) * amount >> 8;
    return (unsigned short)(r << 11 | g << 5 | b);
}

static const unsigned short *ApplyCrt(const unsigned short *src)
{
    double p = (double)sCrtTime / sCrtLen, v, h;
    int cy = MF_SCREEN_H / 2, cx = MF_SCREEN_W / 2, x, y;

    if (p > 1.0)
        p = 1.0;
    if (sCrtPhase == 1)
    {
        // on: the line widens (the first 30%), then the picture opens from it
        h = p < 0.3 ? p / 0.3 : 1.0;
        v = p < 0.3 ? 0.0 : (p - 0.3) / 0.7;
    }
    else
    {
        // off: the picture collapses into the line (60%), the line shrinks to a point
        v = p < 0.6 ? 1.0 - p / 0.6 : 0.0;
        h = p < 0.6 ? 1.0 : 1.0 - (p - 0.6) / 0.4;
    }
    memset(sCrt, 0, sizeof(sCrt));
    if (v * MF_SCREEN_H < 2.0)
    {
        int half = (int)(h * MF_SCREEN_W / 2);
        for (y = cy - 1; y <= cy; y++)
            for (x = cx - half; x < cx + half; x++)
                sCrt[y * MF_SCREEN_W + x] = 0xFFFF;
        return sCrt;
    }
    {
        int half = (int)(v * MF_SCREEN_H / 2), boost = (int)((1.0 - v) * (1.0 - v) * 180);
        for (y = cy - half; y < cy + half; y++)
        {
            int sy = cy + (int)((y - cy) / v);
            const unsigned short *row;
            if (y < 0 || y >= MF_SCREEN_H)
                continue;
            sy = sy < 0 ? 0 : sy >= MF_SCREEN_H ? MF_SCREEN_H - 1 : sy;
            row = src + sy * MF_SCREEN_W;
            for (x = 0; x < MF_SCREEN_W; x++)
                sCrt[y * MF_SCREEN_W + x] = boost > 0 ? Brighten(row[x], boost) : row[x];
        }
    }
    return sCrt;
}

// The picture to show: the engine's (or the one held), with the messages over it and the CRT effect
// (*version changes when any of it does)
static const unsigned short *Fight_Picture(unsigned int *version)
{
    const unsigned short *frame = sHold && !MfEngine_IsActive() ? sHeld : MfEngine_Frame();

    *version = MfEngine_FrameSerial() * 2654435761u ^ sOverlayVersion ^ (unsigned int)sHold << 30
               ^ (unsigned int)(sCrtPhase * 1000 + sCrtTime) * 40503u;
    if (frame == NULL)
    {
        static const unsigned short sBlack[MF_SCREEN_W * MF_SCREEN_H];
        if (sCrtPhase == 0)
            return NULL;
        frame = sBlack;
    }
    if (frame == sHeld || (sOverlayText[0] == '\0' && !sOverlaySignal))
        return sCrtPhase != 0 ? ApplyCrt(frame) : frame;
    memcpy(sComposed, frame, sizeof(sComposed));
    if (sOverlayText[0] != '\0')
    {
        int w = (int)strlen(sOverlayText) * 12;
        int x = (MF_SCREEN_W - w) / 2, y = MF_SCREEN_H / 2 - 8;
        Darken(0, y - 10, MF_SCREEN_W, 36);
        DrawText(sOverlayText, x, y);
    }
    if (sOverlaySignal)
    {
        // four bars, one lit (red): the connection is poor
        int x = MF_SCREEN_W - 8 - 26, y = MF_SCREEN_H - 8 - 20, i;
        Darken(x - 4, y - 4, 34, 28);
        for (i = 0; i < 4; i++)
            Fill(x + i * 7, y + 20 - (i + 1) * 5, 5, (i + 1) * 5, i == 0 ? 0xF800 : 0x52AA);
    }
    return sCrtPhase != 0 ? ApplyCrt(sComposed) : sComposed;
}

SDL_Texture *Fight_Canvas(SDL_Renderer *r)
{
    const unsigned short *frame;
    unsigned int version;

    if (!Fight_Showing())
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
    frame = Fight_Picture(&version);
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

    if (!Fight_Showing() || w <= 0 || h <= 0)
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
        sBicubicSerial = ~0u;
        SDL_SetTextureBlendMode(sBicubic, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(sBicubic, SDL_ScaleModeLinear);
        BicubicTaps(sTapsX, w, MF_SCREEN_W);
        BicubicTaps(sTapsY, h, MF_SCREEN_H);
    }
    frame = Fight_Picture(&serial);
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

unsigned int Fight_Ms(void)
{
    return SDL_GetTicks();
}
