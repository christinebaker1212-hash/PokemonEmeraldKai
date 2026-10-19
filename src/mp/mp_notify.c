// Moemon Co-op: short notices shown in the field (sprite packs, etc.) and
// files dropped onto the game window.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "map_name_popup.h"
#include "main.h"
#include "field_message_box.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/songs.h"

static EWRAM_DATA u8 sNoticeText[200];
static bool8 sNoticePending;

static void Task_Notice(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (data[0])
    {
    case 0:
        if (!MP_IsLocalFree())
            break;
        LockPlayerFieldControls();
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sNoticeText);
        data[0]++;
        break;
    case 1:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            data[0]++;
        }
        break;
    case 2:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
}

// Shows the text in the field as soon as the player is free to move
void MP_ShowNotice(const char *ascii)
{
    MP_AsciiToGame(sNoticeText, ascii, sizeof(sNoticeText) - 1);
    sNoticePending = TRUE;
    printf("[mp] %s\n", ascii);
}

void MP_Notice_Update(void)
{
    if (sNoticePending && MP_IsInOverworld() && !FuncIsActiveTask(Task_Notice))
    {
        sNoticePending = FALSE;
        CreateTask(Task_Notice, 80);
    }
}

// A file was dropped onto the window
void MP_OnFileDropped(const char *path)
{
    char err[200], msg[260];
    if (MP_Avatar_InstallPack(path, err, sizeof(err)))
    {
        MP_SaveConfig();
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        PlaySE(SE_SUCCESS);
        snprintf(msg, sizeof(msg), "Sprite sheet loaded!\nYou're now wearing %.20s.",
                 MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app)) + 2);
    }
    else
    {
        PlaySE(SE_FAILURE);
        snprintf(msg, sizeof(msg), "That file isn't a sprite sheet from\nthe Sprite Editor.");
        printf("[mp] %s\n", err);
    }
    MP_ShowNotice(msg);
}

// Test harness condition "notice": a notice box is up
bool8 MP_Notice_Active(void)
{
    return FuncIsActiveTask(Task_Notice);
}
