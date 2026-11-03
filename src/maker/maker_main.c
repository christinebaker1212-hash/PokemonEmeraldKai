// Moemon Maker: opening and closing the editor (docs/MOEMON_MAKER.md). The editor's window and UI are C++ in maker/.
#include "global.h"
#include "main.h"
#include "main_menu.h"
#include "maker.h"

static bool8 sOpen;

void Maker_Open(void)
{
    sOpen = FALSE;   // (the editor window comes with the editor shell)
    SetMainCallback2(CB2_InitMainMenu);
}

bool8 Maker_IsOpen(void)
{
    return sOpen;
}
