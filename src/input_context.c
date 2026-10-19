// Moemon Kai PC: tells the controller code (src/platform/gamepad.c) what the
// game is showing, and runs the extra actions its context buttons trigger.
#include "global.h"
#include "main.h"
#include "mp.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "platform/gamepad.h"
#include "constants/songs.h"

bool8 NamingScreen_IsActive(void);

int Platform_InputContext(void)
{
    if (NamingScreen_IsActive())
        return INPUT_CONTEXT_NAMING;
    if (gMain.inBattle)
        return INPUT_CONTEXT_BATTLE;
    if (MP_IsInOverworld())
        return MP_IsLocalFree() ? INPUT_CONTEXT_OVERWORLD : INPUT_CONTEXT_TEXT;
    return INPUT_CONTEXT_OTHER;
}

void Platform_ContextAction(int action)
{
    if (!MP_IsLocalFree())
        return;
    switch (action)
    {
    case INPUT_ACTION_COOP_MENU:
        PlaySE(SE_WIN_OPEN);
        MP_OpenMenu();
        break;
    case INPUT_ACTION_EMOTE_UP:
    case INPUT_ACTION_EMOTE_RIGHT:
    case INPUT_ACTION_EMOTE_DOWN:
    case INPUT_ACTION_EMOTE_LEFT:
        MP_Emote_FromStick(action - INPUT_ACTION_EMOTE_UP);
        break;
    }
}
