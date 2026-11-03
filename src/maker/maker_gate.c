// Moemon Maker: the way in (docs/MOEMON_MAKER.md "The way in"). The main menu's MOéMON MAKER row appears only when
// AllowSmallTweaks = 1 (moemon_coop.ini) and "maker" has been typed on the keyboard since the game started (a typed
// word like "speed": src/platform/text_input.c CheckTypedWords). Never remembered: typed again after each start.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "maker.h"

bool8 MP_SmallTweaksAllowed(void);

static bool8 sUnlocked;
static bool8 sChime;          // the chime still to play (on the next game frame)
static bool8 sMenuRefresh;    // the main menu, if open, still to show the new row

// "maker" typed (text_input.c): only with AllowSmallTweaks = 1
void Maker_Typed(void)
{
    printf("[maker] typed (small tweaks %d)\n", MP_SmallTweaksAllowed());
    if (!MP_SmallTweaksAllowed() || sUnlocked)
        return;
    sUnlocked = TRUE;
    sChime = TRUE;
    sMenuRefresh = TRUE;
}

bool8 Maker_Unlocked(void)
{
    return sUnlocked && MP_SmallTweaksAllowed();
}

// The main menu asks each frame whether to draw itself again with the row
bool8 Maker_TakeMenuRefresh(void)
{
    bool8 r = sMenuRefresh;
    sMenuRefresh = FALSE;
    return r;
}

// Every game frame (mp_main.c MP_FrameUpdate)
void Maker_FrameUpdate(void)
{
    if (sChime)
    {
        sChime = FALSE;
        PlaySE(SE_SUCCESS);
    }
}

// Tests: MOEMON_MAKER_UNLOCK=1 unlocks as if typed (still only with AllowSmallTweaks = 1)
void Maker_InitFromEnv(void)
{
    const char *e = getenv("MOEMON_MAKER_UNLOCK");
    if (e && e[0] == '1' && MP_SmallTweaksAllowed())
        sUnlocked = TRUE;
}
