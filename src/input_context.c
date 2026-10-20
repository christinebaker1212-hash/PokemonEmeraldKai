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
    if (MP_QuickPickOpen() || MP_BattleChat_IsOpen())
        return INPUT_CONTEXT_QUICKPICK;
    if (gMain.inBattle)
        return INPUT_CONTEXT_BATTLE;
    if (MP_IsInOverworld())
        return MP_IsLocalFree() ? INPUT_CONTEXT_OVERWORLD : INPUT_CONTEXT_TEXT;
    return INPUT_CONTEXT_OTHER;
}

void Platform_ContextAction(int action)
{
    // Moemon Co-op: in a battle LT / RT (Q / E) open the battle emote pick
    if (gMain.inBattle)
    {
        if (action == INPUT_ACTION_QUICK_EMOTES || action == INPUT_ACTION_QUICK_PHRASES)
            MP_BattleChat_Open(action == INPUT_ACTION_QUICK_EMOTES ? MP_QUICK_PAGE_EMOTES : MP_QUICK_PAGE_PHRASES,
                               action == INPUT_ACTION_QUICK_EMOTES ? 0 : 1);
        return;
    }
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
    case INPUT_ACTION_EMOTE_MENU:
        MP_OpenQuickPick(MP_QUICK_PAGE_EMOTES, -1);
        break;
    case INPUT_ACTION_QUICK_EMOTES:
        MP_OpenQuickPick(MP_QUICK_PAGE_EMOTES, 0);
        break;
    case INPUT_ACTION_QUICK_PHRASES:
        MP_OpenQuickPick(MP_QUICK_PAGE_PHRASES, 1);
        break;
    case INPUT_ACTION_PING:
        MP_Ping_Place();
        break;
    case INPUT_ACTION_ITEM_X:
        gMpItemXPressed = 20;   // used by the field input (field_control_avatar.c)
        break;
    default:
        if (action >= INPUT_ACTION_EMOTE_KEY1 && action < INPUT_ACTION_EMOTE_KEY1 + INPUT_ACTION_EMOTE_KEY_COUNT)
            MP_Emote_Hotkey(action - INPUT_ACTION_EMOTE_KEY1);
        break;
    }
}
