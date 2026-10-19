// Moemon Co-op: controller button icons in text.
// The game's button characters ({A_BUTTON}, {START_BUTTON}, the D-Pad...,
// CHAR_KEYPAD_ICON in text) draw the buttons of the controller used last:
// Nintendo, PlayStation or Xbox (graphics/moemon/buttons/buttons.png, built by
// tools/moemon_buttons/build.py). With the keyboard they stay the GBA buttons.
// Colors come from the window's own palette: the body takes its dark text
// color, the button colors (PlayStation symbols, Xbox letters) the closest one.
#include <string.h>
#include "global.h"
#include "palette.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "platform/gamepad.h"
#include "mp_buttons_data.h"

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

// Palette index for each color class in this window's palette
static void MapColors(u8 windowId, u8 *map)
{
    const u16 *pal = &gPlttBufferUnfaded[BG_PLTT_ID(GetWindowAttribute(windowId, WINDOW_PALETTE_NUM))];
    int c, i;
    memset(map, 0, 10);
    for (c = 1; c < 10; c++)
    {
        int best = 2, bestD = 0x7FFFFFFF;
        if (c == 1 && Dist(pal[2], 0) < Dist(pal[2], 0x7FFF))
        {
            map[c] = 2;   // the body: the dark text color, like the letters next to it
            continue;
        }
        for (i = 2; i < 16; i++)
        {
            int d = Dist(pal[i], sBtnClassColor[c]);
            if (d < bestD)
            {
                bestD = d;
                best = i;
            }
        }
        map[c] = best;
    }
}

static void DrawIcon(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay);

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

// overlay: a same-sized icon whose lit pixels are added (a D-Pad with two arms lit)
static void DrawIcon(u8 windowId, int icon, u16 x, u16 y, bool8 plain, int overlay)
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
    BlitBitmapRectToWindow(windowId, sBuf, 0, 0, 16, 16, x, y + IconTop(h), w, h);
}
