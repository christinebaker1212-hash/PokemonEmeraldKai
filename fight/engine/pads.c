/*
 * Joysticks (original module 0x4745F0-0x474CCF): up to four SDL joysticks
 * whose axes, hats and buttons all become buttons (see Pad), the state of
 * those buttons (g_padButtonState, by pad * PAD_BUTTONS + button) and the
 * joystick code queue that menus read (g_padQueue). Button changes reach
 * the input engine through g_padHandler (Input_OnPadButton).
 *
 * Also here: Input_FindKey, the menu lookup of an input code in the
 * players' key maps.
 */
#include <SDL.h>

#include "mugen.h"

/* Axis values beyond this press the axis' button. */
#define PAD_AXIS_THRESHOLD 16000

/*
 * 0x004745F0: opens the joysticks (at most PAD_MAX). Returns the button
 * count of the first opened joystick that has buttons, 0 when there is
 * none. With no joystick at all nothing else changes (g_padsActive keeps
 * its value).
 */
int Pads_Init(void)
{
    int found = 0;
    int i;

    g_padCount = SDL_NumJoysticks();
    if (g_padCount == 0) {
        return 0;
    }
    if (g_padCount > PAD_MAX) {
        g_padCount = PAD_MAX;
    }
    memset(g_pads, 0, sizeof(g_pads));
    for (i = 0; i < g_padCount; i++) {
        SDL_Joystick *joy = SDL_JoystickOpen(i);
        Pad *pad = &g_pads[i];
        int count;

        if (joy == NULL) {
            continue;
        }
        pad->joystick = joy;
        pad->hat_base = SDL_JoystickNumAxes(joy) * 2;
        pad->button_base = pad->hat_base + SDL_JoystickNumHats(joy) * 4;
        count = SDL_JoystickNumButtons(joy) + pad->button_base;
        pad->button_count = count;
        if (count > PAD_BUTTONS) {
            pad->button_count = PAD_BUTTONS;
        }
        if (found == 0) {
            found = pad->button_count;
        }
    }
    /* One byte more than the INPUT_PAD_COUNT buttons. */
    memset(g_padButtonState, 0, INPUT_PAD_COUNT + 1);
    g_padsActive = found != 0;
    if (g_padsActive) {
        SDL_JoystickEventState(SDL_ENABLE);
    }
    g_padsChanged = 0;
    return found;
}

/* 0x004746D0: closes the joysticks and removes the joystick handler. */
void Input_DeinitPads(void)
{
    int i;

    if (g_padsActive == 0) {
        return;
    }
    g_padHandler = NULL;
    memset(g_padButtonState, 0, INPUT_PAD_COUNT + 1);
    g_padQueueWrite = 0;
    g_padQueueRead = 0;
    SDL_JoystickEventState(SDL_IGNORE);
    for (i = 0; i < g_padCount; i++) {
        if (SDL_JoystickOpened(i) && g_pads[i].joystick != NULL) {
            SDL_JoystickClose(g_pads[i].joystick);
        }
    }
    g_padsActive = 0;
    g_padCount = 0;
}

/* 0x00474750: reports a button change (code = pad * PAD_BUTTONS + button). */
void Pads_SetButton(int code, int pressed)
{
    if (g_padHandler != NULL) {
        g_padHandler(code, pressed);
    }
    Pads_QueueCode(pressed ? code : code | PAD_RELEASE);
    g_padButtonState[code] = (unsigned char)pressed;
    g_padsChanged = 1;
}

/* A button whose state is a flag value (hat directions) or 0. */
static void pad_set_flag(int code, int value)
{
    if ((signed char)g_padButtonState[code] == value) {
        return;
    }
    Pads_SetButton(code, value);
}

/*
 * 0x00474790: handles an SDL joystick event.
 *
 * Quirks kept: every hat of a joystick drives the same four buttons, whose
 * state stores the SDL_HAT_* bit itself (2, 4 or 8, not 1) and which is
 * what the handler gets as "pressed"; axis and hat buttons are not checked
 * against PAD_BUTTONS, so high axis numbers write past g_padButtonState
 * (axis buttons of one pad may overlap the next pad's).
 */
void Pads_HandleEvent(const SDL_Event *ev)
{
    if (g_padsActive == 0) {
        return;
    }
    if (ev->type == SDL_JOYHATMOTION) {
        int base = ev->jhat.which * PAD_BUTTONS + g_pads[ev->jhat.which].hat_base + 1;

        pad_set_flag(base, ev->jhat.value & SDL_HAT_UP);
        pad_set_flag(base + 1, ev->jhat.value & SDL_HAT_DOWN);
        pad_set_flag(base + 2, ev->jhat.value & SDL_HAT_LEFT);
        pad_set_flag(base + 3, ev->jhat.value & SDL_HAT_RIGHT);
    }
    if (ev->type == SDL_JOYAXISMOTION) {
        int axis = ev->jaxis.axis;
        int pad = ev->jaxis.which;
        int dir, first;

        /* The first two axes are swapped: vertical, then horizontal. */
        if (axis == 0) {
            axis = 1;
        } else if (axis == 1) {
            axis = 0;
        }
        if (ev->jaxis.value < -PAD_AXIS_THRESHOLD || ev->jaxis.value > PAD_AXIS_THRESHOLD) {
            dir = ev->jaxis.value > 0 ? 1 : -1;
        } else {
            dir = 0;
        }
        first = axis * 2 + 1;
        /* Release the button of the other direction (both when centred). */
        if (dir >= 0 && g_padButtonState[pad * PAD_BUTTONS + first] != 0) {
            Pads_SetButton(pad * PAD_BUTTONS + first, 0);
        }
        if (dir <= 0 && g_padButtonState[pad * PAD_BUTTONS + first + 1] != 0) {
            Pads_SetButton(pad * PAD_BUTTONS + first + 1, 0);
        }
        if (dir != 0) {
            int code = pad * PAD_BUTTONS + first + (dir > 0);

            if (g_padButtonState[code] != 1) {
                Pads_SetButton(code, 1);
            }
        }
        return;
    }
    if (ev->type == SDL_JOYBUTTONDOWN || ev->type == SDL_JOYBUTTONUP) {
        int button = g_pads[ev->jbutton.which].button_base + 1 + ev->jbutton.button;
        int pressed = ev->type == SDL_JOYBUTTONDOWN;
        int code;

        if (button >= PAD_BUTTONS) {
            return;
        }
        code = ev->jbutton.which * PAD_BUTTONS + button;
        if ((signed char)g_padButtonState[code] != pressed) {
            Pads_SetButton(code, pressed);
        }
    }
}

/* 0x00474A60: appends a code to g_padQueue (only while the pads are active), dropping the oldest when full. */
void Pads_QueueCode(int code)
{
    if (g_padsActive == 0) {
        return;
    }
    g_padQueue[g_padQueueWrite] = code;
    g_padQueueWrite++;
    if (g_padQueueWrite >= INPUT_RING_SIZE) {
        g_padQueueWrite = 0;
    }
    if (g_padQueueWrite == g_padQueueRead) {
        g_padQueueRead++;
        if (g_padQueueRead >= INPUT_RING_SIZE) {
            g_padQueueRead = 0;
        }
    }
}

/*
 * 0x00474AC0: takes the oldest code of g_padQueue (0 when the pads are not
 * active). Quirk: with an empty queue it loops forever; callers check
 * Pads_CodePending first.
 */
int Pads_PopCode(void)
{
    int code;

    if (g_padsActive == 0) {
        return 0;
    }
    while (g_padQueueRead == g_padQueueWrite) {
        /* The original spins here (jmp $). */
    }
    code = g_padQueue[g_padQueueRead];
    g_padQueueRead++;
    if (g_padQueueRead >= INPUT_RING_SIZE) {
        g_padQueueRead = 0;
    }
    return code;
}

/* 0x00474B10: 1 if g_padQueue holds a code. */
int Pads_CodePending(void)
{
    return g_padsActive != 0 && g_padQueueRead != g_padQueueWrite;
}

/*
 * 0x00474B30: looks code (an input code, see Input_TranslateKey) up in
 * count key maps of KEY_COUNT keyboard codes. Returns the bit of the first
 * matching key (1 << (key - 1)) in the low half and the map index in the
 * high half, or 0.
 */
long long Input_FindKey(int *const *maps, int count, int code)
{
    int m;

    for (m = 0; m < count; m++) {
        int keys[KEY_COUNT];
        int i;

        memcpy(keys, maps[m], sizeof(keys));
        for (i = 0; i < KEY_COUNT; i++) {
            /* Input_TranslateKey, inlined. */
            int k = keys[i] & ~KEYBOARD_RELEASE;
            if (k == KEY_NONE || k == 0) {
                k = INPUT_NONE;
            } else if (keys[i] & KEYBOARD_RELEASE) {
                k |= INPUT_RELEASE;
            }
            keys[i] = k;
        }
        for (i = 1; i < KEY_COUNT; i++) {
            if (code == keys[i]) {
                return ((long long)m << 32) | (unsigned int)(1 << (i - 1));
            }
        }
    }
    return 0;
}
