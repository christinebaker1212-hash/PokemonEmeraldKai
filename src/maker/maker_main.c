// Moemon Maker: opening and closing the editor (docs/MOEMON_MAKER.md). The editor's window and UI are C++ in maker/
// (maker/maker_ui.cpp); while it's open the game's own window is hidden and the game waits on a black screen.
#include "global.h"
#include "main.h"
#include "main_menu.h"
#include "gpu_regs.h"
#include "palette.h"
#include "sound.h"
#include "m4a.h"
#include "maker.h"
#include "maker_api.h"

void Display_SetMainWindowShown(int shown);

static bool8 sOpen;

// The game while the editor is open: nothing happens, the screen stays black
static void CB2_MakerWaiting(void)
{
    void Maker_TilesFrame(void);
    Maker_TilesFrame();   // the editor's map's tile animations, frame by frame (maker_tiles.c)
}

void Maker_Open(void)
{
    if (!MakerUi_Open())
    {
        SetMainCallback2(CB2_InitMainMenu);   // (no window could be made: back to the menu)
        return;
    }
    sOpen = TRUE;
    m4aMPlayAllStop();
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetMainCallback2(CB2_MakerWaiting);
    Display_SetMainWindowShown(FALSE);
}

// The editor's window closed (maker_ui.cpp)
void MakerApi_Close(void)
{
    if (!sOpen)
        return;
    sOpen = FALSE;
    Display_SetMainWindowShown(TRUE);
    SetMainCallback2(CB2_InitMainMenu);
}

bool8 Maker_IsOpen(void)
{
    return sOpen;
}
