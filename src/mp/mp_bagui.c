// Moemon Co-op: the BAG screen's picture (tools/moemon_bag/gen_bag.py: the HGSS-style design fitted to 240x160), in
// full color under the game's layers like the CANDY JAR's (mp_candy.c MP_Candy_Composite). item_menu.c draws the text,
// the BRENDAN / MAY bag and the item's icon on top; this draws the background, the pocket icon row with the pocket
// shown picked out, and the red frame on the list's cursor row (a gray one on an item being moved).
#include "global.h"
#include "main.h"
#include "item_menu.h"
#include "mp.h"
#include "mp_bag_gfx.h"

// The list's rows (item_menu.c: WIN_ITEM_LIST and BAG_LIST_FIRST_Y print each row's text inside its frame)
#define LIST_FRAME_X 105
#define LIST_FRAME_W 132
#define LIST_FRAME_Y 16
#define LIST_FRAME_H 17
#define LIST_ROW_H 16

struct BagUIFrame
{
    s16 x, y, w, h;
    u8 cut;     // the rounded corners: 1 for the pocket icon's frame, 2 for the list row's (the design's)
    u8 color;
};

// Is (x, y) on a 2-pixel frame's line? The two outer rows are cut in at the corners (cut, cut - 1 pixels) and the
// first inner row has a pixel more at each corner, as the design draws them.
static bool8 OnFrame(const struct BagUIFrame *f, s32 x, s32 y)
{
    s32 i = x - f->x, j = y - f->y, jj;
    if (i < 0 || j < 0 || i >= f->w || j >= f->h)
        return FALSE;
    jj = (j < f->h / 2) ? j : f->h - 1 - j;   // (rows from the nearer edge)
    if (jj < 2)
    {
        s32 inset = f->cut - jj;
        return i >= inset && i < f->w - inset;
    }
    if (i < 2 || i >= f->w - 2)
        return TRUE;
    return jj == 2 && (i == 2 || i == f->w - 3);
}

void MP_BagUI_Composite(u16 *frame)
{
    s8 fadeOf[32];
    u16 bd = 0x7FFF;
    int i, x, y, coeff, nFrames = 0;
    u8 view, cellX;
    struct BagUIFrame frames[3];

    if (gMain.callback2 != CB2_BagMenuRun || gBagMenu == NULL)
        return;
    // how far the fade has gone: the backdrop is white (the same math as palette.c BlendPalette)
    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
        if (!(frame[i] & 0x8000))
        {
            bd = frame[i];
            break;
        }
    memset(fadeOf, -1, sizeof(fadeOf));
    for (coeff = 16; coeff >= 0; coeff--)
        fadeOf[31 + ((-31 * coeff) >> 4)] = coeff;
    coeff = fadeOf[bd & 0x1F];
    if (coeff < 0)
        coeff = 0;

    view = gBagPosition.pocket < BAG_VIEWS ? gBagPosition.pocket : 0;
    cellX = BAGUI_CELL_X0 + BAGUI_CELL_PITCH * view;
    frames[nFrames++] = (struct BagUIFrame){cellX - 2, BAGUI_STRIP_Y - 2, BAGUI_CELL_W + 4, 14, 1, BAGUI_C_RED};
    if (gBagMenu->numShownItems[view] != 0 && gBagPosition.cursorPosition[view] < gBagMenu->numShownItems[view])
        frames[nFrames++] = (struct BagUIFrame){LIST_FRAME_X, LIST_FRAME_Y + LIST_ROW_H * gBagPosition.cursorPosition[view],
                                                LIST_FRAME_W, LIST_FRAME_H, 2, BAGUI_C_RED};
    if (gBagMenu->toSwapPos != 0xFF)   // (item_menu.c NOT_SWAPPING) the item being moved, where it was
    {
        s32 row = (s32)gBagMenu->toSwapPos - gBagPosition.scrollPosition[view];
        if (row >= 0 && row < gBagMenu->numShownItems[view] && row != gBagPosition.cursorPosition[view])
            frames[nFrames++] = (struct BagUIFrame){LIST_FRAME_X, LIST_FRAME_Y + LIST_ROW_H * row,
                                                    LIST_FRAME_W, LIST_FRAME_H, 2, BAGUI_C_GRAY};
    }

    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
        u16 *row = &frame[y * DISPLAY_WIDTH];
        for (x = 0; x < DISPLAY_WIDTH; x++)
        {
            u16 c;
            u8 k = sBagUI_Bg[y][x];
            if (row[x] & 0x8000)
                continue;
            // the pocket shown: its icon in the picked colors
            if (x >= cellX && x < cellX + BAGUI_CELL_W && y >= BAGUI_STRIP_Y && y < BAGUI_STRIP_Y + 10)
            {
                u8 v = sBagUI_Icons[view][y - BAGUI_STRIP_Y][x - cellX];
                k = v == 1 ? BAGUI_C_SEL_DARK : v == 2 ? BAGUI_C_SEL_LIGHT : BAGUI_C_WHITE;
            }
            for (i = 0; i < nFrames; i++)
                if (OnFrame(&frames[i], x, y))
                {
                    k = frames[i].color;
                    break;
                }
            c = sBagUI_Colors[k];
            if (coeff)
            {
                s32 r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
                r += (-r * coeff) >> 4;
                g += (-g * coeff) >> 4;
                b += (-b * coeff) >> 4;
                c = r | (g << 5) | (b << 10);
            }
            row[x] = c | 0x8000;
        }
    }
}
