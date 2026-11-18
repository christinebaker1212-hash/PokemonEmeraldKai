// Moemon Co-op: controller button icons in text.
// The game's button characters ({A_BUTTON}, {START_BUTTON}, the D-Pad...,
// CHAR_KEYPAD_ICON in text) draw the buttons of the controller used last:
// Nintendo, PlayStation or Xbox (graphics/moemon/buttons/buttons.png, built by
// tools/moemon_buttons/build.py). With the keyboard they stay the GBA buttons.
// Colors come from the window's own palette: the body takes its dark text
// color, the button colors (PlayStation symbols, Xbox letters) the closest one.
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "palette.h"
#include "gpu_regs.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "platform/gamepad.h"
#include "mp_buttons_data.h"
#include "mp_uistyle.h"

#define ICON_CELL_H 12   // the GBA icons' height (what the text engine is told)
extern u8 gMpIconFont;   // the font of the text being printed (text.c)

// Icons sit centered on the capital letters (and their shadow): rows 3/4..11/12
// of the line in every font, so the middle is at 7.5 (7 for the short font)
static int IconTop(int h)
{
    int twiceMiddle = (gMpIconFont >= FONT_SHORT && gMpIconFont <= FONT_SHORT_COPY_3) ? 14 : 15;
    return (twiceMiddle - h + 1) / 2;
}

enum { FACE_S, FACE_E, FACE_W, FACE_N };

static const u8 sFace[4][4] = {   // [family][south, east, west, north]
    [GAMEPAD_ICONS_NINTENDO]    = {BTN_N_B, BTN_N_A, BTN_N_Y, BTN_N_X},
    [GAMEPAD_ICONS_PLAYSTATION] = {BTN_P_CROSS, BTN_P_CIRCLE, BTN_P_SQUARE, BTN_P_TRIANGLE},
    [GAMEPAD_ICONS_XBOX]        = {BTN_X_A, BTN_X_B, BTN_X_X, BTN_X_Y},
};

// The icon for a button character, -1 = draw the GBA one
static int IconFor(u8 id)
{
    int f = Gamepad_IconFamily();
    bool8 nin = Gamepad_EffectiveNintendoLayout();
    if (f == GAMEPAD_ICONS_KEYBOARD)
    {
        // Controller-only buttons have no key of their own: show the Xbox one
        if (id >= CHAR_MP_LT && id <= CHAR_MP_LS && id != CHAR_MP_WEST && id != CHAR_MP_NORTH)
            f = GAMEPAD_ICONS_XBOX;
        else
            return -1;
    }
    switch (id)
    {
    case CHAR_A_BUTTON:   return sFace[f][nin ? FACE_E : FACE_S];
    case CHAR_B_BUTTON:   return sFace[f][nin ? FACE_S : FACE_E];
    case CHAR_MP_WEST:    return sFace[f][FACE_W];
    case CHAR_MP_NORTH:   return sFace[f][FACE_N];
    case CHAR_L_BUTTON:   return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_L : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_L1 : BTN_X_LB;
    case CHAR_R_BUTTON:   return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_R : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_R1 : BTN_X_RB;
    case CHAR_MP_LT:      return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_ZL : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_L2 : BTN_X_LT;
    case CHAR_MP_RT:      return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_ZR : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_R2 : BTN_X_RT;
    case CHAR_MP_LS:      return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_STICK : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_L3 : BTN_X_LS;
    case CHAR_MP_RS:      return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_STICK : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_R3 : BTN_X_RS;
    case CHAR_START_BUTTON:  return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_PLUS : BTN_X_MENU;    // PlayStation OPTIONS: three lines too
    case CHAR_SELECT_BUTTON: return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_MINUS : BTN_X_VIEW;
    case CHAR_DPAD_UP:    return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_UP : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_UP : BTN_X_UP;
    case CHAR_DPAD_DOWN:  return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_DOWN : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_DOWN : BTN_X_DOWN;
    case CHAR_DPAD_LEFT:  return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_LEFT : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_LEFT : BTN_X_LEFT;
    case CHAR_DPAD_RIGHT: return f == GAMEPAD_ICONS_NINTENDO ? BTN_N_RIGHT : f == GAMEPAD_ICONS_PLAYSTATION ? BTN_P_RIGHT : BTN_X_RIGHT;
    case CHAR_DPAD_NONE:  return BTN_P_DOWN;   // drawn without the highlight: a plain D-Pad
    }
    return -1;
}

// D-Pad up/down and left/right on a controller: two icons side by side
// (Nintendo), or one D-Pad with both arms lit (the others, see Overlay)
static int SecondIcon(u8 id)
{
    if (Gamepad_IconFamily() != GAMEPAD_ICONS_NINTENDO)
        return -1;
    if (id == CHAR_DPAD_UPDOWN)
        return IconFor(CHAR_DPAD_DOWN);
    if (id == CHAR_DPAD_LEFTRIGHT)
        return IconFor(CHAR_DPAD_RIGHT);
    return -1;
}

static int Overlay(u8 id)
{
    int f = Gamepad_IconFamily();
    if (f != GAMEPAD_ICONS_PLAYSTATION && f != GAMEPAD_ICONS_XBOX)
        return -1;
    return id == CHAR_DPAD_UPDOWN ? BTN_P_DOWN : id == CHAR_DPAD_LEFTRIGHT ? BTN_P_RIGHT : -1;
}

static int FirstIcon(u8 id)
{
    if (Overlay(id) >= 0)
        return id == CHAR_DPAD_UPDOWN ? BTN_P_UP : BTN_P_LEFT;
    if (Gamepad_IconFamily() != GAMEPAD_ICONS_KEYBOARD)
    {
        if (id == CHAR_DPAD_UPDOWN)
            return IconFor(CHAR_DPAD_UP);
        if (id == CHAR_DPAD_LEFTRIGHT)
            return IconFor(CHAR_DPAD_LEFT);
    }
    return IconFor(id);
}

// The GBA icon a keyboard player sees for the controller's west / north buttons
static u8 KeyboardStandIn(u8 id)
{
    return id == CHAR_MP_WEST ? CHAR_SELECT_BUTTON : id == CHAR_MP_NORTH ? CHAR_START_BUTTON : id;
}

u8 MP_ButtonIcon_Width(u8 id)
{
    int icon = FirstIcon(id), second = SecondIcon(id);
    if (icon < 0)
        return (id == CHAR_MP_WEST || id == CHAR_MP_NORTH) ? GetKeypadIconWidth(KeyboardStandIn(id)) : 0;
    return sBtnIcons[icon].w + 1 + (second >= 0 ? sBtnIcons[second].w : 0);
}

u8 MP_ButtonIcon_Height(u8 id)
{
    int icon = FirstIcon(id);
    if (icon < 0)
        return (id == CHAR_MP_WEST || id == CHAR_MP_NORTH) ? GetKeypadIconHeight(KeyboardStandIn(id)) : 0;
    return ICON_CELL_H;
}

static int Dist(u16 a, u16 b)
{
    int dr = (a & 31) - (b & 31), dg = ((a >> 5) & 31) - ((b >> 5) & 31), db = ((a >> 10) & 31) - ((b >> 10) & 31);
    return dr * dr * 3 + dg * dg * 4 + db * db * 2;
}

static int Luma(u16 c)
{
    return (c & 31) * 3 + ((c >> 5) & 31) * 6 + ((c >> 10) & 31);
}

// Palette index for each color class in this window's palette. The body takes the
// window's darkest color; a button color with nothing close
// in the palette (a green A on a purple screen) becomes its lightest color instead.
static void MapColors(u8 windowId, u8 *map)
{
    const u16 *pal = &gPlttBufferUnfaded[BG_PLTT_ID(GetWindowAttribute(windowId, WINDOW_PALETTE_NUM))];
    int c, i, darkest = 2, lightest = 1;
    memset(map, 0, 10);
    for (i = 1; i < 16; i++)
    {
        if (Luma(pal[i]) < Luma(pal[darkest]))
            darkest = i;
        if (Luma(pal[i]) > Luma(pal[lightest]))
            lightest = i;
    }
    for (c = 1; c < 10; c++)
    {
        int best = 2, bestD = 0x7FFFFFFF;
        if (c <= 2 && Luma(pal[1]) + 60 < Luma(pal[2]))
        {
            // a dark window (OPTIONS > DISPLAY > THEME DARK: a dark fill, light text): the key's face in the text's
            // light color, its letter in the fill's dark, as a light key reads on a dark box
            map[c] = c == 1 ? 2 : 1;
            continue;
        }
        if (c == 1)
        {
            // the body: the darkest color (index 2 is the dark text color in most windows,
            // but not all: the summary screen keeps red there)
            map[c] = (Dist(pal[2], 0) < Dist(pal[2], 0x7FFF) && Luma(pal[2]) <= Luma(pal[darkest]) + 40) ? 2 : darkest;
            continue;
        }
        for (i = 1; i < 16; i++)
        {
            int d = Dist(pal[i], sBtnClassColor[c]);
            if (d < bestD)
            {
                bestD = d;
                best = i;
            }
        }
        if (c >= 3 && bestD > 1500)
            best = lightest;
        map[c] = best;
    }
}

static void DrawIcon(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay);
static void DrawIconAt(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay);

// Vanilla scenes that blit a button picture (summary screen A/B, the bag's SELECT tag):
// the controller's icon centered in that box. FALSE = keyboard, draw the game's own picture.
bool8 MP_ButtonIcon_DrawInBox(u8 windowId, u8 id, u16 x, u16 y, u16 w, u16 h)
{
    int icon = FirstIcon(id), second = SecondIcon(id);
    int iw, ih;
    if (Gamepad_IconFamily() == GAMEPAD_ICONS_KEYBOARD || icon < 0)
        return FALSE;
    iw = sBtnIcons[icon].w + (second >= 0 ? sBtnIcons[second].w : 0);
    ih = sBtnIcons[icon].h;
    x += (w > iw) ? (w - iw) / 2 : 0;
    y += (h > ih) ? (h - ih + 1) / 2 : 0;
    DrawIconAt(windowId, icon, x, y, id == CHAR_DPAD_NONE, Overlay(id));
    if (second >= 0)
        DrawIconAt(windowId, second, x + sBtnIcons[icon].w, y, FALSE, -1);
    return TRUE;
}

// Into an 8bpp index buffer, for button pictures on sprites (easy chat's START / SELECT
// tags): body in bodyIdx, button colors from pal when close, else contrastIdx. Returns the width (0 = keyboard).
u8 MP_ButtonIcon_ToPixels(u8 id, u8 *buf, int stride, int x, int y, const u16 *pal, u8 bodyIdx, u8 contrastIdx)
{
    int icon = FirstIcon(id), w, h, i, j, c;
    u8 map[10];
    if (Gamepad_IconFamily() == GAMEPAD_ICONS_KEYBOARD || icon < 0)
        return 0;
    w = sBtnIcons[icon].w;
    h = sBtnIcons[icon].h;
    map[0] = 0;
    map[1] = bodyIdx;
    for (c = 2; c < 10; c++)
    {
        int best = contrastIdx, bestD = 1500;
        for (i = 1; i < 16; i++)
        {
            int d = Dist(pal[i], sBtnClassColor[c]);
            if (d < bestD)
            {
                bestD = d;
                best = i;
            }
        }
        map[c] = (c == 2 && best == contrastIdx) ? contrastIdx : best;
    }
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
        {
            u8 v = sBtnPixels[sBtnIcons[icon].offset + j * w + i];
            if (v)
                buf[(y + j) * stride + x + i] = map[v];
        }
    return w;
}

u8 MP_ButtonIcon_Draw(u8 windowId, u8 id, u16 x, u16 y)
{
    int icon = FirstIcon(id), second = SecondIcon(id);
    if (icon < 0)
        return (id == CHAR_MP_WEST || id == CHAR_MP_NORTH) ? DrawKeypadIcon(windowId, KeyboardStandIn(id), x, y) : 0;
    DrawIcon(windowId, icon, x, y, id == CHAR_DPAD_NONE, Overlay(id));
    if (second >= 0)
        DrawIcon(windowId, second, x + sBtnIcons[icon].w, y, FALSE, -1);
    return MP_ButtonIcon_Width(id);
}

// On a text line: centered on the capitals
static void DrawIcon(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay)
{
    DrawIconAt(windowId, icon, x, y + IconTop(sBtnIcons[icon].h), plain, overlay);
}

// Every button drawn into a window, so the finished frame can show it in its own colours (MP_ButtonIcon_Composite):
// a window's palette only gives it the nearest of the window's colours (a light grey A on a dark window, a pale one on
// the summary's green), and the user, 2026-11-18: "Button icons should always show the correct colors and never change
// palettes". The window keeps the palette's version underneath (what anything reading the window sees).
#define SHOWN_MAX 48
static struct Shown {
    u8 windowId, w, h, plain;
    s16 icon, overlay;
    u16 x, y;
    u8 map[10];
} sShown[SHOWN_MAX];
static u8 sShownNext;

static u8 IconClass(int icon, int overlay, bool8 plain, int i, int j, int w)
{
    u8 c = sBtnPixels[sBtnIcons[icon].offset + j * w + i];
    if (overlay >= 0 && sBtnPixels[sBtnIcons[overlay].offset + j * w + i] > c)
        c = sBtnPixels[sBtnIcons[overlay].offset + j * w + i];
    if (plain && c == 2)
        c = 1;
    return c;
}

static void Remember(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay, const u8 *map)
{
    struct Shown *s = &sShown[sShownNext];
    u8 k;
    for (k = 0; k < SHOWN_MAX; k++)   // (the same spot drawn again: that entry)
        if (sShown[k].w && sShown[k].windowId == windowId && sShown[k].x == x && sShown[k].y == y)
        {
            s = &sShown[k];
            break;
        }
    if (k == SHOWN_MAX)
        sShownNext = (sShownNext + 1) % SHOWN_MAX;
    s->windowId = windowId, s->icon = icon, s->overlay = overlay, s->plain = plain;
    s->x = x, s->y = y, s->w = sBtnIcons[icon].w, s->h = sBtnIcons[icon].h;
    memcpy(s->map, map, sizeof(s->map));
    if (getenv("MOEMON_BUTTONS_TRACE"))
        printf("[btn] window %u icon %d at %u,%u (%ux%u) map %u %u %u\n", windowId, icon, x, y, s->w, s->h, map[1], map[2], map[3]);
}

// The window's pixel (x, y): its palette index
static u8 WindowPixel(u8 windowId, int x, int y)
{
    const struct Window *win = &gWindows[windowId];
    const u8 *t = win->tileData;
    int width = win->window.width;
    u8 b;
    if (t == NULL || x < 0 || y < 0 || x >= width * 8 || y >= win->window.height * 8)
        return 0xFF;
    b = t[((y / 8) * width + x / 8) * TILE_SIZE_4BPP + (y % 8) * 4 + (x % 8) / 2];
    return (x & 1) ? b >> 4 : b & 15;
}

// Is the window's own tile on its background's map at window pixel (x, y) right now? (another window can sit in the same
// place, shown while this one isn't: a summary page's header under another page's)
static bool8 WindowShownAt(const struct Window *win, int x, int y)
{
    int bg = win->window.bg, size, mx, my, block;
    u16 cnt = GetGpuReg(REG_OFFSET_BG0CNT + bg * 2), e;
    size = cnt >> 14;
    mx = (win->window.tilemapLeft + x / 8) & ((size & 1) ? 63 : 31);
    my = (win->window.tilemapTop + y / 8) & ((size & 2) ? 63 : 31);
    block = ((cnt >> 8) & 31) + (mx >> 5) + ((my >> 5) * ((size & 1) ? 2 : 1));
    e = ((const u16 *)(VRAM + block * 0x800))[(my & 31) * 32 + (mx & 31)];
    return (e & 0x3FF) == (u16)(win->window.baseBlock + (y / 8) * win->window.width + x / 8)
        && (e >> 12) == win->window.paletteNum;
}

static u16 Shade(u16 trueColor, u16 unfaded, u16 faded)
{
    // (a fade on the screen: the icon's colour moved as far as the window's own colour was)
    s32 out = 0, sh;
    if (unfaded == faded)
        return trueColor;
    for (sh = 0; sh <= 10; sh += 5)
    {
        s32 v = ((trueColor >> sh) & 31) + (((faded >> sh) & 31) - ((unfaded >> sh) & 31));
        out |= (v < 0 ? 0 : v > 31 ? 31 : v) << sh;
    }
    return out;
}

// sdl2.c VDraw, over the finished frame: each button still in its window, in its own colours, on the pixels that show
// it (where the window's colour for that pixel is what the frame has: not under anything drawn over the window)
void MP_ButtonIcon_Composite(u16 *frame)
{
    u8 k;
    {
        char *getenv(const char *name);
        static s8 sOff = -1;
        if (sOff < 0)
            sOff = getenv("MOEMON_BUTTONS_PALETTE") != NULL;   // (tests: the window's own palette version, as before)
        if (sOff)
            return;
    }
    for (k = 0; k < SHOWN_MAX; k++)
    {
        struct Shown *s = &sShown[k];
        const struct Window *win;
        int i, j, sx, sy, bg, pal;
        bool8 alive = TRUE;
        if (!s->w)
            continue;
        win = &gWindows[s->windowId];
        if (s->windowId >= WINDOWS_MAX || win->tileData == NULL)
        {
            s->w = 0;
            continue;
        }
        // still there? (the text erased or written over: forgotten)
        for (j = 0; j < s->h && alive; j++)
            for (i = 0; i < s->w && alive; i++)
            {
                u8 c = IconClass(s->icon, s->overlay, s->plain, i, j, s->w);
                if (c && WindowPixel(s->windowId, s->x + i, s->y + j) != s->map[c])
                    alive = FALSE;
            }
        if (!alive)
        {
            s->w = 0;
            continue;
        }
        bg = win->window.bg;
        pal = win->window.paletteNum;
        if (!(GetGpuReg(REG_OFFSET_DISPCNT) & (DISPCNT_BG0_ON << bg)))
            continue;
        sx = win->window.tilemapLeft * 8 + s->x - (GetGpuReg(REG_OFFSET_BG0HOFS + bg * 4) & 0x1FF);
        sy = win->window.tilemapTop * 8 + s->y - (GetGpuReg(REG_OFFSET_BG0VOFS + bg * 4) & 0x1FF);
        for (j = 0; j < s->h; j++)
            for (i = 0; i < s->w; i++)
            {
                u8 c = IconClass(s->icon, s->overlay, s->plain, i, j, s->w);
                int px = sx + i, py = sy + j;
                u16 shown, unf;
                if (!c || px < 0 || py < 0 || px >= DISPLAY_WIDTH || py >= DISPLAY_HEIGHT
                 || !WindowShownAt(win, s->x + i, s->y + j))
                    continue;
                shown = gPlttBufferFaded[BG_PLTT_ID(pal) + s->map[c]] & 0x7FFF;
                unf = gPlttBufferUnfaded[BG_PLTT_ID(pal) + s->map[c]] & 0x7FFF;
                if ((frame[py * DISPLAY_WIDTH + px] & 0x7FFF) != shown)
                    continue;   // (something over the window here, or the window not there)
                frame[py * DISPLAY_WIDTH + px] = (frame[py * DISPLAY_WIDTH + px] & 0x8000)
                                               | Shade(sBtnClassColor[c] & 0x7FFF, unf, shown);
            }
    }
}

// overlay: a same-sized icon whose lit pixels are added (a D-Pad with two arms lit)
static void DrawIconAt(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay)
{
    static u8 sBuf[2 * 2 * TILE_SIZE_4BPP];   // 16x16, tile order
    u8 map[10];
    int w, h, i, j;
    const u8 *src;

    w = sBtnIcons[icon].w;
    h = sBtnIcons[icon].h;
    src = &sBtnPixels[sBtnIcons[icon].offset];
    MapColors(windowId, map);
    if (plain)
        map[2] = map[1];   // no highlighted direction
    memset(sBuf, 0, sizeof(sBuf));
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            u8 c = src[j * w + i];
            if (overlay >= 0 && sBtnPixels[sBtnIcons[overlay].offset + j * w + i] > c)
                c = sBtnPixels[sBtnIcons[overlay].offset + j * w + i];
            u8 v = map[c];
            u8 *b = &sBuf[((j / 8) * 2 + i / 8) * TILE_SIZE_4BPP + (j % 8) * 4 + (i % 8) / 2];
            *b |= (i & 1) ? v << 4 : v;
        }
    }
    gMpUiTextBlit++;   // (a button in the words: the UI STYLE writes it as letters)
    BlitBitmapRectToWindow(windowId, sBuf, 0, 0, 16, 16, x, y, w, h);
    gMpUiTextBlit--;
    Remember(windowId, icon, x, y, plain, overlay, map);
}
