/*
 * Keyboard (original module 0x46E290-0x46E42F): the state of every key
 * (g_keyState), the keyboard code queue that menus read (g_keyboardQueue)
 * and the SDL key event callback of the console.
 *
 * Key changes reach the input engine through g_keyHandler (Input_OnKey).
 * The pause key toggles g_pauseToggled on each press. Shutdown is inlined in
 * System_Deinit.
 */
#include <SDL.h>

#include "mugen.h"

/*
 * 0x0046E290: a key went down (pressed != 0) or up. Key 0 is ignored. The
 * change is queued, releases with KEYBOARD_RELEASE unless g_keyboardNoReleases
 * (then only presses are queued).
 */
void Keyboard_HandleKey(int key, int pressed)
{
    if (key == 0) {
        return;
    }
    g_keyModState = SDL_GetModState();
    if (key == KEY_PAUSE_TOGGLE && pressed) {
        g_pauseToggled ^= 1;
    }
    g_keyState[key] = (unsigned char)pressed;
    if (g_keyboardNoReleases == 0) {
        Keyboard_QueueCode(pressed ? key : key | KEYBOARD_RELEASE);
    } else if (pressed) {
        Keyboard_QueueCode(key);
    }
    if (g_keyHandler != NULL) {
        g_keyHandler(key, pressed);
    }
}

/*
 * 0x0046E2F0: clears the key state, the key handler and the queue. Returns 0
 * when the keyboard is already active. Key repeat is turned off unless
 * g_keyboardKeepRepeat is set.
 */
int Keyboard_Init(void)
{
    if (g_keyboardActive != 0) {
        return 0;
    }
    memset(g_keyState, 0, sizeof(g_keyState));
    g_keyHandler = NULL;
    if (g_keyboardKeepRepeat == 0) {
        g_keyboardKeepRepeat = 0;
        SDL_EnableKeyRepeat(0, 0);
    }
    g_keyboardQueueWrite = 0;
    g_keyboardQueueRead = 0;
    g_keyboardActive = 1;
    return 1;
}

/*
 * 0x0046E350: copies SDL's key state into g_keyState (focus regained). At
 * most 0x143 keys are copied: the "more than 0x200" clamp can never apply.
 */
void Keyboard_SyncState(void)
{
    int count;
    const Uint8 *state = SDL_GetKeyState(&count);
    int i;

    if (count > 0x143) {
        count = 0x143;
    } else if (count > INPUT_KEY_COUNT) {
        count = INPUT_KEY_COUNT;
    }
    for (i = 0; i < count; i++) {
        g_keyState[i] = state[i];
    }
}

/*
 * 0x0046E3B0: installs the callback that receives every key event (the
 * console). Fails (0) when one is installed and fn is not NULL; NULL
 * removes it. SDL's unicode translation is turned on for a callback (and
 * never off again).
 */
int Keyboard_SetEventHandler(KeyEventFn fn)
{
    if (g_keyEventHandler != NULL && fn != NULL) {
        return 0;
    }
    g_keyEventHandler = fn;
    if (fn != NULL) {
        SDL_EnableUNICODE(1);
    }
    return 1;
}

/* 0x0046E3E0: appends a code to g_keyboardQueue, dropping the oldest one when full. */
void Keyboard_QueueCode(int code)
{
    g_keyboardQueue[g_keyboardQueueWrite] = code;
    g_keyboardQueueWrite++;
    if (g_keyboardQueueWrite >= INPUT_RING_SIZE) {
        g_keyboardQueueWrite = 0;
    }
    if (g_keyboardQueueWrite == g_keyboardQueueRead) {
        g_keyboardQueueRead++;
        if (g_keyboardQueueRead >= INPUT_RING_SIZE) {
            g_keyboardQueueRead = 0;
        }
    }
}
