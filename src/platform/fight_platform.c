// Moemon Fight: the fight engine's picture, buttons and sound on the game's SDL2 side
// (include/mf_engine.h; the engine itself is in fight/). While a fight runs its 480x320 picture is
// the game picture (Fight_Canvas, like the card screens), the fight's sound replaces the game's
// (Fight_MixAudio) and friends spectating see a 240x160 copy (Fight_Frame).
#include "platform/crt.h"
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
_Static_assert(FIGHT_CANVAS_W == MF_SCREEN_W && FIGHT_CANVAS_H == MF_SCREEN_H, "display.c's size of the fight picture");

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

// (the effect itself: src/platform/crt_effect.c, shared with the DANCE machine)
static const unsigned short *ApplyCrt(const unsigned short *src)
{
    Crt_Apply(src, sCrt, MF_SCREEN_W, MF_SCREEN_H, sCrtPhase == 1 ? CRT_PHASE_ON : CRT_PHASE_OFF,
              (double)sCrtTime / sCrtLen, CRT_FMT_RGB565);
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

// Tests (fightshot NAME): the picture Fight_Canvas shows now, as a 480x320 24-bit BMP. 0 when saved
int Fight_SaveCanvasBMP(const char *path)
{
    const unsigned short *frame;
    unsigned int version;
    SDL_Surface *surf;
    int x, y, rc;

    if (!Fight_Showing() || (frame = Fight_Picture(&version)) == NULL)
        return -1;
    surf = SDL_CreateRGBSurfaceWithFormat(0, MF_SCREEN_W, MF_SCREEN_H, 24, SDL_PIXELFORMAT_RGB24);
    if (surf == NULL)
        return -1;
    for (y = 0; y < MF_SCREEN_H; y++)
    {
        unsigned char *row = (unsigned char *)surf->pixels + y * surf->pitch;
        for (x = 0; x < MF_SCREEN_W; x++)
        {
            unsigned int c = frame[y * MF_SCREEN_W + x];
            // RGB565 to 8 bits a component as SDL converts it for the texture (bits repeated)
            row[x * 3] = (unsigned char)((c >> 11) << 3 | (c >> 13));
            row[x * 3 + 1] = (unsigned char)((c >> 5 & 63) << 2 | (c >> 9 & 3));
            row[x * 3 + 2] = (unsigned char)((c & 31) << 3 | (c >> 2 & 7));
        }
    }
    rc = SDL_SaveBMP(surf, path);
    SDL_FreeSurface(surf);
    return rc;
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

unsigned int Fight_Ms(void)
{
    return SDL_GetTicks();
}

// ---------------------------------------------------------------------------------------------------------------
// The pack against its ingestion record (tools/fight/ingest.py writes mugen/processed_assets.idx: size, modification
// time and path of every file the characters, stages and shared files use). At start, on a thread of its own so the
// game doesn't wait: files changed or gone since, and character / stage .defs the record doesn't know (a pack added
// by hand), go to mugen/ingestion_log.txt with what to run. Nothing is changed: ingestion itself needs the tools.

#include <sys/stat.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <strings.h>
#endif

#define INGEST_MAX 4096

static char sIngestRoot[512];
static char (*sIngestKnown)[160];
static int sIngestKnownCount;

static int Ingest_Known(const char *rel)
{
    int i;
    for (i = 0; i < sIngestKnownCount; i++)
        if (strcmp(sIngestKnown[i], rel) == 0)
            return 1;
    return 0;
}

// The .defs one level down (chars/NAME/*.def) or in the folder itself (stages/*.def) the record lacks
static void Ingest_NewDefs(const char *sub, int depth, FILE *out, int *count)
{
    char dir[700];
    snprintf(dir, sizeof(dir), "%s%s", sIngestRoot, sub);
#ifdef _WIN32
    {
        char pat[720];
        WIN32_FIND_DATAA fd;
        HANDLE h;
        snprintf(pat, sizeof(pat), "%s/*", dir);
        h = FindFirstFileA(pat, &fd);
        if (h == INVALID_HANDLE_VALUE)
            return;
        do
        {
            char rel[400];
            const char *n = fd.cFileName;
            size_t len = strlen(n);
            if (n[0] == '.')
                continue;
            snprintf(rel, sizeof(rel), "%s/%s", sub, n);
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && depth > 0)
                Ingest_NewDefs(rel, depth - 1, out, count);
            else if (len > 4 && _stricmp(n + len - 4, ".def") == 0 && !Ingest_Known(rel) && (*count)++ < 20)
                fprintf(out, "    new: %s\n", rel);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#else
    {
        DIR *d = opendir(dir);
        struct dirent *e;
        if (d == NULL)
            return;
        while ((e = readdir(d)) != NULL)
        {
            char rel[400], full[1200];
            struct stat st;
            size_t len = strlen(e->d_name);
            if (e->d_name[0] == '.')
                continue;
            snprintf(rel, sizeof(rel), "%s/%s", sub, e->d_name);
            snprintf(full, sizeof(full), "%s%s", sIngestRoot, rel);
            if (stat(full, &st) != 0)
                continue;
            if (S_ISDIR(st.st_mode) && depth > 0)
                Ingest_NewDefs(rel, depth - 1, out, count);
            else if (len > 4 && strcasecmp(e->d_name + len - 4, ".def") == 0 && !Ingest_Known(rel) && (*count)++ < 20)
                fprintf(out, "    new: %s\n", rel);
        }
        closedir(d);
    }
#endif
}

static int SDLCALL Ingest_Thread(void *unused)
{
    char path[700], line[700], report[4096];
    FILE *idx, *mem;
    int changed = 0, missing = 0, fresh = 0;
    (void)unused;
    snprintf(path, sizeof(path), "%sprocessed_assets.idx", sIngestRoot);
    sIngestKnown = calloc(INGEST_MAX, sizeof(*sIngestKnown));
    if (sIngestKnown == NULL)
        return 0;
    mem = tmpfile();
    if (mem == NULL)
    {
        free(sIngestKnown);
        return 0;
    }
    idx = fopen(path, "r");
    if (idx == NULL)
    {
        fprintf(mem, "    no ingestion record (processed_assets.idx)\n");
        changed = 1;
    }
    else
    {
        while (fgets(line, sizeof(line), idx))
        {
            long long size, mtime;
            char rel[512], full[1200];
            struct stat st;
            line[strcspn(line, "\r\n")] = 0;
            if (sscanf(line, "%lld %lld %511[^\n]", &size, &mtime, rel) != 3)
                continue;
            if (sIngestKnownCount < INGEST_MAX)
                snprintf(sIngestKnown[sIngestKnownCount++], sizeof(sIngestKnown[0]), "%s", rel);
            snprintf(full, sizeof(full), "%s%s", sIngestRoot, rel);
            if (stat(full, &st) != 0)
            {
                if (missing++ < 20)
                    fprintf(mem, "    gone: %s\n", rel);
            }
            else if ((long long)st.st_size != size)
            {
                // sizes only: unzipping an update sets every file's time (ingestion compares contents)
                if (changed++ < 20)
                    fprintf(mem, "    changed: %s\n", rel);
            }
        }
        fclose(idx);
        Ingest_NewDefs("chars", 1, mem, &fresh);
        Ingest_NewDefs("stages", 0, mem, &fresh);
    }
    if (changed || missing || fresh)
    {
        size_t n;
        FILE *log;
        time_t now = time(NULL);
        char when[32];
        strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", localtime(&now));
        rewind(mem);
        n = fread(report, 1, sizeof(report) - 1, mem);
        report[n] = 0;
        snprintf(path, sizeof(path), "%singestion_log.txt", sIngestRoot);
        log = fopen(path, "a");
        if (log != NULL)
        {
            fprintf(log, "%s  game start: %d changed, %d gone, %d new since the last ingestion (run "
                         "tools/fight/ingest.py)\n%s", when, changed, missing, fresh, report);
            fclose(log);
        }
        printf("[fight] the pack changed since its last ingestion: %d changed, %d gone, %d new (mugen/ingestion_log.txt)\n",
               changed, missing, fresh);
        fflush(stdout);
    }
    fclose(mem);
    free(sIngestKnown);
    sIngestKnown = NULL;
    return 0;
}

void Fight_CheckIngestion(void)
{
    const char *env = getenv("MOEMON_MUGEN_DIR");
    size_t n;
    SDL_Thread *t;
    snprintf(sIngestRoot, sizeof(sIngestRoot), "%s", env != NULL && env[0] ? env : "mugen");
    n = strlen(sIngestRoot);
    if (n && sIngestRoot[n - 1] != '/' && sIngestRoot[n - 1] != '\\' && n + 1 < sizeof(sIngestRoot))
        strcat(sIngestRoot, "/");
    t = SDL_CreateThread(Ingest_Thread, "fight-ingest", NULL);
    if (t != NULL)
        SDL_DetachThread(t);
}
