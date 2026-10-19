// Moemon Kai PC: game controllers through SDL's GameController API.
//
// Every connected controller drives the game (hot-plugging works). Mapping:
//
//   D-Pad / left stick   D-Pad
//   confirm / cancel     A / B. Nintendo-style pads use the buttons labeled
//                        A and B (right / bottom); Xbox, PlayStation and other
//                        pads use bottom = A, right = B. START > SETTINGS can
//                        force either layout.
//   Start / Options / +  START        Back / Share / -   SELECT
//   LB / RB              L / R
//
// The other buttons depend on what's on screen (Platform_InputContext):
//   Overworld   left stick pushed all the way = run (with Running Shoes),
//               west button (X / Square / Y) = toggle auto-run,
//               north button (Y / Triangle / X) = CO-OP menu,
//               right stick = emotes and quick phrases.
//   Text, menus and battles   LT (hold) = repeat A, to get through text fast.
//               In battles and menus the west button is B and the north
//               button is SELECT.
//   Naming screen   west = backspace (B), north = switch letters (SELECT).
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "platform/gamepad.h"

#define MAX_PADS 4
#define STICK_DEAD   0.45f   // walking
#define STICK_RUN    0.88f   // pushed all the way: run
#define RSTICK_EMOTE 0.75f
#define TRIGGER_ON   0.5f
#define TURBO_PERIOD 6       // frames per A press while LT is held

static SDL_GameController *sPads[MAX_PADS];
static int sLayout = GAMEPAD_LAYOUT_AUTO;
static int sAnalogRun = 1;
static int sRumble = 1;
static int sAutoRun;
static int sInited;
static u32 sPrevButtons;     // bit per SDL button, all pads merged
static int sPrevEmote = -1;
static int sTurbo;
// Controller selector: -1 = every controller drives the game
static SDL_JoystickID sSelected = -1;
static char sSelectedGuid[64];      // remembered between sessions
static int sListening;              // waiting for a button press to pick a controller
static int sSuppress;               // ignore the picking press until it's released
static u32 sPadPrev[MAX_PADS];      // per-controller buttons, for the selector

static SDL_GameController *SelectedPad(void);

static const char *ConfigPath(void)
{
    const char *p = getenv("MOEMON_CONTROLS_CONFIG");
    return p ? p : "moemon_controls.ini";
}

static void LoadSettings(void)
{
    FILE *f = fopen(ConfigPath(), "r");
    char line[128];
    if (!f)
        return;
    while (fgets(line, sizeof(line), f))
    {
        char key[64], val[64];
        if (line[0] == ';' || sscanf(line, " %63[^= ] = %63s", key, val) != 2)
            continue;
        if (strcmp(key, "layout") == 0)
            sLayout = strcmp(val, "standard") == 0 ? GAMEPAD_LAYOUT_STANDARD
                    : strcmp(val, "nintendo") == 0 ? GAMEPAD_LAYOUT_NINTENDO : GAMEPAD_LAYOUT_AUTO;
        else if (strcmp(key, "analog_run") == 0)
            sAnalogRun = atoi(val) != 0;
        else if (strcmp(key, "rumble") == 0)
            sRumble = atoi(val) != 0;
        else if (strcmp(key, "controller") == 0 && strcmp(val, "all") != 0)
            snprintf(sSelectedGuid, sizeof(sSelectedGuid), "%s", val);
    }
    fclose(f);
}

void Gamepad_SaveSettings(void)
{
    static const char *const names[] = {"auto", "standard", "nintendo"};
    FILE *f = fopen(ConfigPath(), "w");
    if (!f)
        return;
    fprintf(f, "; Moemon Emerald Kai controller settings (START > SETTINGS)\n"
               "; layout = auto, standard (bottom button = A) or nintendo (right button = A)\n"
               "layout = %s\nanalog_run = %d\nrumble = %d\n"
               "; controller = all, or the id of the one picked in START > SETTINGS\n"
               "controller = %s\n", names[sLayout], sAnalogRun, sRumble, sSelectedGuid[0] ? sSelectedGuid : "all");
    fclose(f);
}

static void OpenPad(int deviceIndex)
{
    int i;
    SDL_GameController *pad;
    if (!SDL_IsGameController(deviceIndex))
        return;
    pad = SDL_GameControllerOpen(deviceIndex);
    if (!pad)
        return;
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i] == pad)
            return; // already open (SDL reuses the handle)
    }
    for (i = 0; i < MAX_PADS; i++)
    {
        if (!sPads[i])
        {
            char guid[64];
            sPads[i] = pad;
            sPadPrev[i] = 0;
            SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(pad)), guid, sizeof(guid));
            // The controller picked last time is back
            if (sSelectedGuid[0] && sSelected < 0 && strcmp(guid, sSelectedGuid) == 0)
                sSelected = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad));
            printf("[pad] connected: %s\n", SDL_GameControllerName(pad));
            return;
        }
    }
    SDL_GameControllerClose(pad);
}

static void ClosePad(SDL_JoystickID id)
{
    int i;
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(sPads[i])) == id)
        {
            printf("[pad] disconnected: %s\n", SDL_GameControllerName(sPads[i]));
            if (sSelected == id)
                sSelected = -1; // back to every controller until it returns
            SDL_GameControllerClose(sPads[i]);
            sPads[i] = NULL;
        }
    }
}

void Gamepad_Init(void)
{
    int i;
    // SDL_CONTROLLER_BUTTON_A is always the bottom button; the layout is ours to pick
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("[pad] no controller support: %s\n", SDL_GetError());
        return;
    }
    // Extra controller mappings (community gamecontrollerdb.txt), if present
    if (SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt") > 0)
        printf("[pad] loaded gamecontrollerdb.txt\n");
    LoadSettings();
    for (i = 0; i < SDL_NumJoysticks(); i++)
        OpenPad(i);
    sInited = 1;
}

// Called from the SDL event loop; TRUE when the event was ours
int Gamepad_HandleEvent(const SDL_Event *e)
{
    if (!sInited)
        return 0;
    switch (e->type)
    {
    case SDL_CONTROLLERDEVICEADDED:
        OpenPad(e->cdevice.which);
        return 1;
    case SDL_CONTROLLERDEVICEREMOVED:
        ClosePad(e->cdevice.which);
        return 1;
    }
    return 0;
}

static int PadIsNintendo(SDL_GameController *pad)
{
#if SDL_VERSION_ATLEAST(2, 0, 12)
    switch (SDL_GameControllerGetType(pad))
    {
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
#if SDL_VERSION_ATLEAST(2, 24, 0)
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
#endif
        return 1;
    default:
        break;
    }
#endif
    {
        const char *name = SDL_GameControllerName(pad);
        // Nintendo-labeled third-party pads
        if (name && (strstr(name, "Nintendo") || strstr(name, "Switch") || strstr(name, "SNES") || strstr(name, "GameCube")))
            return 1;
    }
    return 0;
}

static int UseNintendoLayout(SDL_GameController *pad)
{
    if (sLayout == GAMEPAD_LAYOUT_STANDARD)
        return 0;
    if (sLayout == GAMEPAD_LAYOUT_NINTENDO)
        return 1;
    return PadIsNintendo(pad);
}

int Gamepad_EffectiveNintendoLayout(void)
{
    int i;
    if (SelectedPad())
        return UseNintendoLayout(SelectedPad());
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i])
            return UseNintendoLayout(sPads[i]);
    }
    return sLayout == GAMEPAD_LAYOUT_NINTENDO;
}

int Gamepad_IsConnected(void)
{
    int i;
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i])
            return 1;
    }
    return 0;
}

const char *Gamepad_Name(void)
{
    int i;
    if (SelectedPad())
        return Gamepad_SelectedName();
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i])
        {
            const char *n = SDL_GameControllerName(sPads[i]);
            return n ? n : "Controller";
        }
    }
    return "";
}

int Gamepad_GetLayout(void) { return sLayout; }
void Gamepad_SetLayout(int layout)
{
    sLayout = (layout % GAMEPAD_LAYOUT_COUNT + GAMEPAD_LAYOUT_COUNT) % GAMEPAD_LAYOUT_COUNT;
    sSuppress = 1; // the held button may now mean something else: wait for release
}
int Gamepad_GetAnalogRun(void) { return sAnalogRun; }
void Gamepad_SetAnalogRun(int on) { sAnalogRun = on != 0; }
int Gamepad_GetRumble(void) { return sRumble; }
void Gamepad_SetRumble(int on) { sRumble = on != 0; }

void Gamepad_Rumble(int strength, int ms)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
    int i;
    Uint16 s;
    if (!sRumble || strength <= 0)
        return;
    s = (Uint16)(strength > 100 ? 0xFFFF : strength * 0xFFFF / 100);
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i] && (!SelectedPad() || sPads[i] == SelectedPad()))
            SDL_GameControllerRumble(sPads[i], s, s, ms);
    }
#endif
}

static SDL_JoystickID PadId(SDL_GameController *pad)
{
    return SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad));
}

static void SelectPad(SDL_GameController *pad)
{
    if (pad)
    {
        sSelected = PadId(pad);
        SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(pad)), sSelectedGuid, sizeof(sSelectedGuid));
    }
    else
    {
        sSelected = -1;
        sSelectedGuid[0] = 0;
    }
}

// The pad that drives the game, NULL = every pad
static SDL_GameController *SelectedPad(void)
{
    int i;
    if (sSelected < 0)
        return NULL;
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i] && PadId(sPads[i]) == sSelected)
            return sPads[i];
    }
    return NULL;
}

void Gamepad_StartSelect(void) { sListening = 1; }
void Gamepad_CancelSelect(void) { sListening = 0; }
int Gamepad_IsSelecting(void) { return sListening; }

int Gamepad_Count(void)
{
    int i, n = 0;
    for (i = 0; i < MAX_PADS; i++)
        n += sPads[i] != NULL;
    return n;
}

// "ALL CONTROLLERS" or the picked one's name
const char *Gamepad_SelectedName(void)
{
    SDL_GameController *pad = SelectedPad();
    const char *n;
    if (!pad)
        return "ALL CONTROLLERS";
    n = SDL_GameControllerName(pad);
    return n ? n : "Controller";
}

// Left/Right on the CONTROLLER row: all -> pad 1 -> pad 2 ... -> all
void Gamepad_CycleSelected(int dir)
{
    SDL_GameController *list[MAX_PADS + 1];
    int n = 0, cur = 0, i;
    list[n++] = NULL;
    for (i = 0; i < MAX_PADS; i++)
    {
        if (sPads[i])
        {
            if (PadId(sPads[i]) == sSelected)
                cur = n;
            list[n++] = sPads[i];
        }
    }
    cur = ((cur + dir) % n + n) % n;
    SelectPad(list[cur]);
}

// While selecting: the first controller with a newly pressed button wins
static void UpdateSelector(void)
{
    int i;
    for (i = 0; i < MAX_PADS; i++)
    {
        u32 now = 0;
        int b;
        if (!sPads[i])
            continue;
        for (b = 0; b < SDL_CONTROLLER_BUTTON_MAX && b < 32; b++)
        {
            if (SDL_GameControllerGetButton(sPads[i], b))
                now |= 1u << b;
        }
        if (sListening && (now & ~sPadPrev[i]))
        {
            SelectPad(sPads[i]);
            sListening = 0;
            sSuppress = 1;
            Gamepad_Rumble(60, 250);
            printf("[pad] using only: %s\n", SDL_GameControllerName(sPads[i]));
        }
        sPadPrev[i] = now;
    }
}

static float Axis(SDL_GameController *pad, SDL_GameControllerAxis a)
{
    return SDL_GameControllerGetAxis(pad, a) / 32767.0f;
}

#define BTN(b) (1u << (b))

// Reads every controller. Returns GBA keys (*fastForward is always 0 now).
u16 Gamepad_Poll(int *fastForward)
{
    u16 keys = 0;
    u32 buttons = 0;           // merged SDL buttons (positional)
    u32 pressed;
    float lx = 0, ly = 0, rx = 0, ry = 0, lt = 0, rt = 0;
    int nintendo = 0, any = 0, i;
    int context;
    int run = 0;

    *fastForward = 0;
    if (!sInited)
        return 0;
    UpdateSelector();
    if (sListening)
    {
        sPrevButtons = 0xFFFFFFFF; // no "new" presses when it ends
        return 0;
    }
    for (i = 0; i < MAX_PADS; i++)
    {
        SDL_GameController *pad = sPads[i];
        if (SelectedPad() && pad != SelectedPad())
            continue;
        int b;
        float v;
        if (!pad)
            continue;
        if (!any)
            nintendo = UseNintendoLayout(pad);
        any = 1;
        for (b = 0; b < SDL_CONTROLLER_BUTTON_MAX && b < 32; b++)
        {
            if (SDL_GameControllerGetButton(pad, b))
                buttons |= BTN(b);
        }
        v = Axis(pad, SDL_CONTROLLER_AXIS_LEFTX);  if (v * v > lx * lx) lx = v;
        v = Axis(pad, SDL_CONTROLLER_AXIS_LEFTY);  if (v * v > ly * ly) ly = v;
        v = Axis(pad, SDL_CONTROLLER_AXIS_RIGHTX); if (v * v > rx * rx) rx = v;
        v = Axis(pad, SDL_CONTROLLER_AXIS_RIGHTY); if (v * v > ry * ry) ry = v;
        v = Axis(pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT);  if (v > lt) lt = v;
        v = Axis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT); if (v > rt) rt = v;
    }
    if (!any)
    {
        sPrevButtons = 0;
        return 0;
    }
    if (sSuppress)
    {
        if (buttons)
            return 0;
        sSuppress = 0;
    }
    pressed = buttons & ~sPrevButtons;
    sPrevButtons = buttons;
    context = Platform_InputContext();

    // Confirm / cancel
    {
        u32 south = BTN(SDL_CONTROLLER_BUTTON_A), east = BTN(SDL_CONTROLLER_BUTTON_B);
        if (buttons & (nintendo ? east : south)) keys |= A_BUTTON;
        if (buttons & (nintendo ? south : east)) keys |= B_BUTTON;
    }
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_START))         keys |= START_BUTTON;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_BACK))          keys |= SELECT_BUTTON;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_LEFTSHOULDER))  keys |= L_BUTTON;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) keys |= R_BUTTON;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_DPAD_UP))       keys |= DPAD_UP;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_DPAD_DOWN))     keys |= DPAD_DOWN;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_DPAD_LEFT))     keys |= DPAD_LEFT;
    if (buttons & BTN(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))    keys |= DPAD_RIGHT;

    // Left stick: the strongest direction, so diagonals don't jitter
    if (lx * lx > ly * ly)
    {
        if (lx < -STICK_DEAD) keys |= DPAD_LEFT;
        if (lx >  STICK_DEAD) keys |= DPAD_RIGHT;
    }
    else
    {
        if (ly < -STICK_DEAD) keys |= DPAD_UP;
        if (ly >  STICK_DEAD) keys |= DPAD_DOWN;
    }

    (void)rt; // RT used to be fast forward; there's no fast forward any more

    {
        u32 west = BTN(nintendo ? SDL_CONTROLLER_BUTTON_Y : SDL_CONTROLLER_BUTTON_X);
        u32 north = BTN(nintendo ? SDL_CONTROLLER_BUTTON_X : SDL_CONTROLLER_BUTTON_Y);

        switch (context)
        {
        case INPUT_CONTEXT_OVERWORLD:
            if (pressed & west)
            {
                sAutoRun = !sAutoRun;
                Gamepad_Rumble(15, 60);
            }
            if (pressed & north)
                Platform_ContextAction(INPUT_ACTION_COOP_MENU);
            if (pressed & BTN(SDL_CONTROLLER_BUTTON_RIGHTSTICK))
                Platform_ContextAction(INPUT_ACTION_EMOTE_MENU);
            if (sAnalogRun && lx * lx + ly * ly > STICK_RUN * STICK_RUN)
                run = 1;
            if (sAutoRun && (keys & DPAD_ANY))
                run = 1;
            if (run)
                keys |= B_BUTTON;
            // Right stick flick = emote
            {
                int emote = -1;
                if (rx * rx + ry * ry > RSTICK_EMOTE * RSTICK_EMOTE)
                {
                    if (rx * rx > ry * ry)
                        emote = rx > 0 ? INPUT_ACTION_EMOTE_RIGHT : INPUT_ACTION_EMOTE_LEFT;
                    else
                        emote = ry > 0 ? INPUT_ACTION_EMOTE_DOWN : INPUT_ACTION_EMOTE_UP;
                }
                if (emote >= 0 && emote != sPrevEmote)
                    Platform_ContextAction(emote);
                sPrevEmote = emote;
            }
            break;
        case INPUT_CONTEXT_NAMING:
            if (buttons & west)  keys |= B_BUTTON;       // backspace
            if (buttons & north) keys |= SELECT_BUTTON;  // switch letters
            break;
        default:
            if (buttons & west)  keys |= B_BUTTON;
            if (buttons & north) keys |= SELECT_BUTTON;
            // LT held: tap A over and over to get through text
            if (lt > TRIGGER_ON)
            {
                if (++sTurbo >= TURBO_PERIOD)
                    sTurbo = 0;
                if (sTurbo < TURBO_PERIOD / 2)
                    keys |= A_BUTTON;
            }
            else
            {
                sTurbo = 0;
            }
            break;
        }
        if (context != INPUT_CONTEXT_OVERWORLD)
            sPrevEmote = -1;
    }
    return keys;
}

// ---------------------------------------------------------------------------
// Headless tests: a virtual controller (test script commands vpad / vbtn / vaxis)

static SDL_Joystick *sVirtualPads[MAX_PADS];
static SDL_Joystick *sVirtual;   // the one vbtn / vaxis act on (vuse N picks)
static int sVirtualCount;

int Gamepad_TestActive(void)
{
    return sVirtual != NULL;
}

static int ButtonByName(const char *n)
{
    static const char *const names[] = {"south", "east", "west", "north", "back", "guide", "start",
                                        "lstick", "rstick", "lb", "rb", "up", "down", "left", "right"};
    int i;
    for (i = 0; i < (int)(sizeof(names) / sizeof(names[0])); i++)
    {
        if (strcmp(n, names[i]) == 0)
            return i;
    }
    return -1;
}

static int AxisByName(const char *n)
{
    static const char *const names[] = {"lx", "ly", "rx", "ry", "lt", "rt"};
    int i;
    for (i = 0; i < 6; i++)
    {
        if (strcmp(n, names[i]) == 0)
            return i;
    }
    return -1;
}

// vpad NAME (attach; NAME "nintendo" makes a Switch-style pad) / vbtn BUTTON 0|1 / vaxis AXIS -100..100
void Gamepad_TestCommand(const char *cmd, const char *arg, int value)
{
#if SDL_VERSION_ATLEAST(2, 24, 0)
    if (strcmp(cmd, "vuse") == 0)
    {
        int n = atoi(arg);
        if (n >= 0 && n < sVirtualCount)
            sVirtual = sVirtualPads[n];
    }
    else if (strcmp(cmd, "vpad") == 0 && sVirtualCount < MAX_PADS)
    {
        SDL_VirtualJoystickDesc desc;
        int index;
        if (!sInited)
            Gamepad_Init();
        memset(&desc, 0, sizeof(desc));
        desc.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
        desc.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        desc.naxes = SDL_CONTROLLER_AXIS_MAX;
        desc.nbuttons = SDL_CONTROLLER_BUTTON_MAX;
        desc.name = strcmp(arg, "nintendo") == 0 ? "Virtual Nintendo Switch Pad" : "Virtual Xbox Pad";
        index = SDL_JoystickAttachVirtualEx(&desc);
        if (index < 0)
        {
            printf("[pad] virtual pad failed: %s\n", SDL_GetError());
            return;
        }
        sVirtual = SDL_JoystickOpen(index);
        sVirtualPads[sVirtualCount++] = sVirtual;
        OpenPad(index);
        printf("[pad] virtual pad attached (%s), game controller: %d\n", desc.name, SDL_IsGameController(index));
    }
    else if (strcmp(cmd, "vbtn") == 0 && sVirtual)
    {
        int b = ButtonByName(arg);
        if (b >= 0)
            SDL_JoystickSetVirtualButton(sVirtual, b, value ? 1 : 0);
    }
    else if (strcmp(cmd, "vaxis") == 0 && sVirtual)
    {
        int a = AxisByName(arg);
        if (a >= 0)
            SDL_JoystickSetVirtualAxis(sVirtual, a, (Sint16)(value * 32767 / 100));
    }
    SDL_JoystickUpdate();
#endif
}
