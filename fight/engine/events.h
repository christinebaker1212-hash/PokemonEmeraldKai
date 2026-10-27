#ifndef MUGEN_EVENTS_H
#define MUGEN_EVENTS_H

/*
 * SDL event pump and the input state/queue it feeds (events.c).
 *
 * Input codes: SDL key codes 0..0x1FF and joystick buttons 0x200 + n; a
 * release is queued with INPUT_RELEASE set. g_keyMap (when present) remaps
 * codes to built-in keys before they reach g_inputState.
 */

#define INPUT_KEY_COUNT     0x200   /* SDL key codes */
#define INPUT_PAD_COUNT     0x80    /* joystick buttons */
#define INPUT_PAD_BASE      INPUT_KEY_COUNT
#define INPUT_CODE_COUNT    (INPUT_KEY_COUNT + INPUT_PAD_COUNT + 1)    /* g_inputState entries */
#define INPUT_NONE          0x280   /* translated "no key" */
#define INPUT_RELEASE       0x1000  /* flag of a queued release */

/* g_inputState bits */
#define INPUT_STATE_KEY     0x01
#define INPUT_STATE_PAD     0x02

/* Key queue size set by Events_Init. */
#define INPUT_QUEUE_SIZE    0x100

/* Receives key (code, pressed) and joystick button (button, pressed) changes (g_keyHandler, g_padHandler). */
typedef void (__cdecl *InputHandlerFn)(int code, int pressed);

/* Receives every SDL key event (g_keyEventHandler): unicode (low 16 bits), key symbol, pressed. */
typedef void (__cdecl *KeyEventFn)(unsigned int unicode, int sym, int pressed);

/*
 * Keyboard and joystick code queues (g_keyboardQueue, g_padQueue): ring
 * buffers of key codes (release: KEYBOARD_RELEASE) and joystick buttons
 * (release: PAD_RELEASE); when full, the oldest code is dropped.
 */
#define INPUT_RING_SIZE     0x100
#define KEYBOARD_RELEASE    0x200
#define PAD_RELEASE         0x100

#define KEY_PAUSE_TOGGLE    0x13    /* SDLK_PAUSE: each press toggles g_pauseToggled */

/* Joysticks opened by Pads_Init. */
#define PAD_MAX             4
#define PAD_BUTTONS         0x20    /* button codes per joystick: pad * PAD_BUTTONS + 1.. */

/*
 * An open joystick (g_pads). Its buttons are numbered from 1: two per axis
 * (negative, positive; axes 0 and 1 swapped), four for the hats (up, down,
 * left, right; all hats share them) and then the joystick buttons.
 */
typedef struct Pad {
    struct _SDL_Joystick *joystick; /* 0x00 SDL_Joystick */
    int button_count;           /* 0x04 at most PAD_BUTTONS */
    int pad_08;                 /* 0x08 never accessed by the game */
    int hat_base;               /* 0x0C 2 * axes */
    int button_base;            /* 0x10 hat_base + 4 * hats */
} Pad;
MUGEN_ASSERT_SIZE(Pad, 0x14);

#endif /* MUGEN_EVENTS_H */
