/*
 * SDL event pump and input state (original module 0x46B210-0x46B79F).
 *
 * Key and joystick button changes reach Input_OnKey / Input_OnPadButton
 * through g_keyHandler / g_padHandler (installed by Events_Init), which keep
 * g_inputState and queue the changes in g_keyQueue for menus and key
 * configuration (Input_CodePending, Input_Translate*).
 */
#include <SDL.h>

#include "mugen.h"

/* 0x0046B210: handles every pending SDL event. */
void Events_Poll(void)
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_ACTIVEEVENT:
            if (ev.active.state & SDL_APPINPUTFOCUS) {
                int gain = ev.active.gain;
                if (gain) {
                    g_keyModState = SDL_GetModState();
                }
                SDL_ShowCursor((g_videoSdlFlags & SDL_FULLSCREEN) && gain ? SDL_DISABLE : SDL_ENABLE);
                if (g_onActivate != NULL) {
                    g_onActivate(gain);
                }
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            Keyboard_HandleKey(ev.key.keysym.sym, ev.type == SDL_KEYDOWN);
            if (g_keyEventHandler != NULL) {
                /* The original passes the dword at the unicode field, padding included. */
                unsigned int unicode;
                memcpy(&unicode, &ev.key.keysym.unicode, sizeof(unicode));
                g_keyEventHandler(unicode, ev.key.keysym.sym, ev.type == SDL_KEYDOWN);
            }
            break;
        case SDL_JOYAXISMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            Pads_HandleEvent(&ev);
            break;
        case SDL_QUIT:
            g_quitRequested = 1;
            break;
        case SDL_VIDEORESIZE:
            Video_OnResize(ev.resize.w, ev.resize.h);
            break;
        }
    }
}

/* Queues the code of an input that went from released to held (or back). */
static void Input_Changed(int code, int old_state)
{
    if ((g_inputState[code] == 0) != (old_state == 0)) {
        if (g_inputState[code] == 0) {
            code |= INPUT_RELEASE;
        }
        Input_QueueCode(code);
    }
}

/* 0x0046B350: g_keyHandler. Keys remapped to 0 are ignored. */
void Input_OnKey(int key, int pressed)
{
    int code = g_keyMap != NULL ? g_keyMap[key] : key;
    int old;

    if (code == 0) {
        return;
    }
    old = (signed char)g_inputState[code];
    if (pressed) {
        g_inputState[code] |= INPUT_STATE_KEY;
    } else {
        g_inputState[code] &= ~INPUT_STATE_KEY;
    }
    Input_Changed(code, old);
}

/* 0x0046B3D0: g_padHandler. */
void Input_OnPadButton(int button, int pressed)
{
    int code = button + INPUT_PAD_BASE;
    int old;

    if (g_keyMap != NULL) {
        code = g_keyMap[code];
    }
    old = (signed char)g_inputState[code];
    if (pressed) {
        g_inputState[code] |= INPUT_STATE_PAD;
    } else {
        g_inputState[code] &= ~INPUT_STATE_PAD;
    }
    Input_Changed(code, old);
}

/*
 * 0x0046B450: creates the key queue and installs the input handlers. Fails
 * (0) when a handler is already installed, after installing the key handler
 * if only the pad handler was there.
 */
int Events_Init(void)
{
    if (g_keyQueue.data == NULL && !IntArray_Init(&g_keyQueue, INPUT_QUEUE_SIZE)) {
        return 0;
    }
    if (g_keyHandler != NULL) {
        return 0;
    }
    g_keyHandler = Input_OnKey;
    if (g_padHandler != NULL) {
        return 0;
    }
    g_padHandler = Input_OnPadButton;
    memset(g_inputState, 0, sizeof(g_inputState));
    g_keyQueue.write = 0;
    g_keyQueue.read = 0;
    Input_ResetState();
    return 1;
}

/* 0x0046B500: removes the handlers and frees the key map and the key queue. */
void Events_Deinit(void)
{
    int *map = g_keyMap;

    g_keyHandler = NULL;
    g_padHandler = NULL;
    if (map != NULL) {
        g_keyMap = NULL;
        free(map);
    }
    if (g_keyQueue.data != NULL) {
        free(g_keyQueue.data);
    }
    g_keyQueue.data = NULL;
    g_keyQueue.read = 0;
    g_keyQueue.write = 0;
    g_keyQueue.capacity = 0;
}

/* 0x0046B560: rebuilds g_inputState from the held keys and buttons (focus regained, remapping). */
void Input_ResetState(void)
{
    int i;

    memset(g_inputState, 0, sizeof(g_inputState));
    if (g_keyMap == NULL) {
        for (i = 1; i < INPUT_KEY_COUNT; i++) {
            g_inputState[i] |= g_keyState[i] != 0;
        }
        for (i = 0; i < INPUT_PAD_COUNT; i++) {
            g_inputState[INPUT_PAD_BASE + i] |= g_padButtonState[i] != 0 ? INPUT_STATE_PAD : 0;
        }
        return;
    }
    for (i = 1; i < INPUT_KEY_COUNT; i++) {
        g_inputState[g_keyMap[i]] |= g_keyState[i] != 0;
    }
    for (i = 0; i < INPUT_PAD_COUNT; i++) {
        g_inputState[g_keyMap[INPUT_PAD_BASE + i]] |= g_padButtonState[i] != 0 ? INPUT_STATE_PAD : 0;
    }
}

/*
 * 0x0046B670: appends an input code to g_keyQueue (only while the keyboard
 * or the pads are active); when full, the oldest code is dropped.
 */
void Input_QueueCode(int code)
{
    if (!g_padsActive && !g_keyboardActive) {
        return;
    }
    if (code == 0) {
        return;
    }
    g_keyQueue.data[g_keyQueue.write] = code;
    g_keyQueue.write++;
    if (g_keyQueue.write >= g_keyQueue.capacity) {
        g_keyQueue.write = 0;
    }
    if (g_keyQueue.write == g_keyQueue.read) {
        g_keyQueue.read++;
        if (g_keyQueue.read >= g_keyQueue.capacity) {
            g_keyQueue.read = 0;
        }
    }
}

/* 0x0046B6E0: 1 if g_keyQueue holds a code. */
int Input_CodePending(void)
{
    if (!g_padsActive && !g_keyboardActive) {
        return 0;
    }
    return g_keyQueue.read != g_keyQueue.write;
}

/*
 * 0x0046B710: keyboard code with release bit 0x200 to an input code:
 * INPUT_NONE for no key, INPUT_RELEASE for a release.
 */
int Input_TranslateKey(int key)
{
    int code = key & ~0x200;

    if (code == 0x200 || code == 0) {   /* the first test can never hold */
        return INPUT_NONE;
    }
    if (key & 0x200) {
        return code | INPUT_RELEASE;
    }
    return code;
}

/* 0x0046B740: joystick button with release bit 0x100 (JOY_NONE = none) to an input code. */
int Input_TranslateJoy(int button)
{
    int code = button & ~0x100;

    if (code == JOY_NONE) {
        return INPUT_NONE;
    }
    if (code != 0) {
        code += INPUT_PAD_BASE;
        if (button & 0x100) {
            code |= INPUT_RELEASE;
        }
    }
    return code;
}

/* 0x0046B770: an empty array with room for capacity ints; 0 when out of memory. */
int IntArray_Init(IntArray *a, int capacity)
{
    a->data = NULL;
    a->read = 0;
    a->write = 0;
    a->capacity = 0;
    a->data = calloc(1, capacity * sizeof(int));
    if (a->data == NULL) {
        return 0;
    }
    a->capacity = capacity;
    return 1;
}
