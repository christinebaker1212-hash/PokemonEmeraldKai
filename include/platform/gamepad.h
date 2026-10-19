// Moemon Kai PC: game controllers (src/platform/gamepad.c).
// Any SDL game controller works (Xbox, PlayStation, Switch, 8BitDo, ...) and
// can be plugged in at any time. The button layout follows the controller:
// on Nintendo-style pads A/B are the buttons labeled A/B, on the others the
// bottom button confirms and the right one cancels. Extra buttons do
// different things depending on what the game is showing (see gamepad.c).
#ifndef GUARD_PLATFORM_GAMEPAD_H
#define GUARD_PLATFORM_GAMEPAD_H

enum {
    GAMEPAD_LAYOUT_AUTO,      // follow the controller type
    GAMEPAD_LAYOUT_STANDARD,  // bottom = A, right = B (Xbox, PlayStation)
    GAMEPAD_LAYOUT_NINTENDO,  // right = A, bottom = B (Switch)
    GAMEPAD_LAYOUT_COUNT
};

// What the game is showing, for the context-dependent buttons
enum {
    INPUT_CONTEXT_OTHER,       // menus and anything else
    INPUT_CONTEXT_OVERWORLD,   // walking around, free to move
    INPUT_CONTEXT_TEXT,        // a message or script is running in the overworld
    INPUT_CONTEXT_BATTLE,
    INPUT_CONTEXT_NAMING,      // the keyboard / naming screen
    INPUT_CONTEXT_QUICKPICK,   // the emote / phrase pick around the player is open
};

// Extra actions the platform asks the game to run (Platform_ContextAction)
enum {
    INPUT_ACTION_COOP_MENU,    // overworld: open START > CO-OP
    INPUT_ACTION_EMOTE_UP,     // right stick: emotes / quick phrases
    INPUT_ACTION_EMOTE_RIGHT,
    INPUT_ACTION_EMOTE_DOWN,
    INPUT_ACTION_EMOTE_LEFT,
    INPUT_ACTION_EMOTE_MENU,   // right stick click: START > CO-OP > EMOTES
    INPUT_ACTION_EMOTE_KEY1,   // keyboard 1-8: emotes and your phrases
    INPUT_ACTION_QUICK_EMOTES = INPUT_ACTION_EMOTE_KEY1 + 8,   // LT held: quick emote pick
    INPUT_ACTION_QUICK_PHRASES,                                // RT held: quick phrase pick
    INPUT_ACTION_PING,                                         // left stick click / V: "look here!" on the tile ahead
};
#define INPUT_ACTION_EMOTE_KEY_COUNT 8

// Implemented by the game (src/input_context.c)
int Platform_InputContext(void);
void Platform_ContextAction(int action);

int Gamepad_GetLayout(void);
void Gamepad_SetLayout(int layout);
int Gamepad_GetAnalogRun(void);
void Gamepad_SetAnalogRun(int on);
int Gamepad_GetRumble(void);
void Gamepad_SetRumble(int on);
void Gamepad_SaveSettings(void);
int Gamepad_IsConnected(void);
const char *Gamepad_Name(void);            // first connected controller, "" if none
int Gamepad_EffectiveNintendoLayout(void); // TRUE when A/B are right/bottom right now
void Gamepad_Rumble(int strength, int ms); // strength 0-100, no-op when off/unsupported
// Controller selector: press a button on the controller you want to use
void Gamepad_StartSelect(void);
void Gamepad_CancelSelect(void);
int Gamepad_IsSelecting(void);
void Gamepad_CycleSelected(int dir);       // ALL -> each connected controller -> ALL
const char *Gamepad_SelectedName(void);    // "ALL CONTROLLERS" or a controller name
int Gamepad_Count(void);
int Gamepad_TriggerHeld(int right);        // LT/Q (0) or RT/E (1) held: a controller trigger or the keyboard key
void Gamepad_SetKeyHold(int which, int held);
// Which button icons to show: the last used controller's, or the keyboard's (the GBA buttons)
enum { GAMEPAD_ICONS_KEYBOARD, GAMEPAD_ICONS_NINTENDO, GAMEPAD_ICONS_PLAYSTATION, GAMEPAD_ICONS_XBOX };
int Gamepad_IconFamily(void);
void Gamepad_NoteKeyboard(void);

#endif // GUARD_PLATFORM_GAMEPAD_H
