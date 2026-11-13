// Moemon Co-op: OPTIONS > DISPLAY > THEME: DEFAULT or DARK (the user, 2026-11-13: "Allow the Dark theme to be used
// everywhere where the default menu would draw, including the default start menu, messages, battles", and "put in
// greater effort to make sure it fits across all scenes and all border types").
// The game's windows all take their colors from a few palettes: the standard menu text palette (std_menu.pal: the
// window's fill, its text and shadow), the window frames (OPTIONS > FRAME: Emerald's 20, FireRed's 9), the field's
// message box, the text window palettes, the OPTIONS screen's text, and in battle the text box, the action / move
// windows' text palette and the PP colors. DARK swaps each for a dark one designed for it as it's loaded (palette.c
// LoadPalette / LoadCompressedPalette call MP_Theme_Map*), so every screen that uses them follows without its own
// code: the START menu, messages, yes / no boxes, shops, the options, battles' menus, the co-op screens.
// The dark palettes are made by tools/moemon_darktheme/gen_dark.py (src/data/mp_dark_palettes.h): a night-slate fill,
// warm white text over a shadow darker than the fill, colored text in bright tones over deep shadows of their hue,
// and every frame color by color: its outline kept, its own colors at a dusk tone, the light rims that met the white
// fill turned to the slate a step lighter than the dark one (tinted ones keeping their tint), so each frame still
// looks like itself. gen_dark.py --preview draws all 29 frames light and dark side by side.
#include <string.h>
#include "global.h"
#include "mp.h"
#include "palette.h"
#include "menu.h"
#include "text_window.h"
#include "graphics.h"
#include "../data/mp_dark_palettes.h"

extern const u16 gPPTextPalette[];

_Static_assert(sizeof(sDarkFrames) / sizeof(sDarkFrames[0]) == WINDOW_FRAMES_COUNT, "gen_dark.py: one dark palette a frame");

bool8 MP_Theme_Dark(void)
{
    return gMpConfig.menuTheme == MP_MENU_THEME_DARK;
}

// The palette to load in SRC's place (SRC itself unless DARK is on and it's one of the windows' palettes)
const void *MP_Theme_Map(const void *src, u16 size)
{
    int i;
    if (!MP_Theme_Dark() || size != PLTT_SIZE_4BPP || src == NULL)
        return src;
    if (src == gStandardMenuPalette)
        return sDarkStdMenu[0];
    if (src == gMessageBox_Pal || src == GetTextWindowPalette(0))
        return sDarkMessageBox[0];
    for (i = 1; i <= 4; i++)
        if (src == GetTextWindowPalette(i))
            return sDarkTextPals[i - 1];
    for (i = 0; i < WINDOW_FRAMES_COUNT; i++)
        if (src == GetWindowFrameTilesPal(i)->pal)
            return sDarkFrames[i];
    return src;
}

// The OPTIONS screen's own text palette
const void *MP_Theme_MapText(const void *src)
{
    return MP_Theme_Dark() ? sDarkOptionText[0] : src;
}

// Compressed palettes: battles' text box (two palettes) and the action / move windows' text
void MP_Theme_MapCompressed(const u32 *src, u16 *decompressed, u16 size)
{
    if (!MP_Theme_Dark())
        return;
    if (src == gBattleWindowTextPalette)
        memcpy(decompressed, sDarkBattleText[0], min(size, PLTT_SIZE_4BPP));
    else if (src == gBattleTextboxPalette && size >= 2 * PLTT_SIZE_4BPP)
        memcpy(decompressed, sDarkBattleBox, 2 * PLTT_SIZE_4BPP);
}

// The move list's PP number colors (battle_message.c: picked one pair at a time from gPPTextPalette)
u16 MP_Theme_TextColor(u16 c)
{
    int i;
    if (!MP_Theme_Dark())
        return c;
    for (i = 0; i < 8; i++)
        if (gPPTextPalette[i] == c)
            return sDarkPP[0][i];
    return c;
}

// The BAG's three text colors (item_menu.c): the list's text light over a dark shadow on the dark list
const u16 *MP_Theme_BagText(const u16 *src)
{
    return MP_Theme_Dark() ? sDarkBagText : src;
}

// A copy of the standard menu palette as it's drawn (the co-op screens build their colors on it)
void MP_Theme_StdPalette(u16 *dst)
{
    memcpy(dst, MP_Theme_Map(gStandardMenuPalette, PLTT_SIZE_4BPP), PLTT_SIZE_4BPP);
}
