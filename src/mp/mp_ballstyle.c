// Moemon Co-op: a Moemon's POKé BALL look. Each Moemon keeps a "ball style"
// in the 16 bits of struct BoxPokemon that Emerald never uses ("unknown",
// MON_DATA_ENCRYPT_SEPARATOR): it's saved, and copied with the Moemon by
// trades, the PC and the co-op link.
//   bits 0-3   color (basic POKé BALLS only): 0 = classic red
//   bits 4-11  decoration (POKé and PREMIER BALLS only): 0 = none
// The color recolors the red top of the ball (battle sprite, summary ball and
// the item icon used as a preview). Decorations are drawn on top of the ball
// in the same places; there are none until their art is added to sDecorations.
#include "global.h"
#include "mp.h"
#include "decompress.h"
#include "item.h"
#include "item_icon.h"
#include "palette.h"
#include "pokeball.h"
#include "pokemon.h"
#include "sprite.h"
#include "constants/items.h"
#include "constants/rgb.h"

#define TAG_MP_BALL_COLOR 0x56A0   // + color: recolored POKé BALL battle palettes

// Five shades per color, dark to light (the classic red uses the game's own)
static const u16 sRamps[MP_BALL_COLORS][5] = {
    [MP_BALL_RED]    = {0},
    [MP_BALL_BLUE]   = {RGB(4, 6, 17),   RGB(5, 9, 22),   RGB(7, 13, 28),  RGB(14, 20, 31), RGB(23, 27, 31)},
    [MP_BALL_GREEN]  = {RGB(3, 12, 5),   RGB(4, 17, 6),   RGB(7, 23, 9),   RGB(15, 28, 14), RGB(25, 31, 21)},
    [MP_BALL_YELLOW] = {RGB(18, 15, 0),  RGB(25, 20, 0),  RGB(30, 25, 3),  RGB(31, 29, 11), RGB(31, 31, 23)},
    [MP_BALL_PINK]   = {RGB(20, 6, 13),  RGB(26, 9, 17),  RGB(31, 14, 22), RGB(31, 21, 26), RGB(31, 27, 29)},
    [MP_BALL_PURPLE] = {RGB(10, 4, 15),  RGB(14, 6, 20),  RGB(19, 10, 26), RGB(24, 17, 30), RGB(28, 25, 31)},
    [MP_BALL_ORANGE] = {RGB(20, 9, 0),   RGB(26, 13, 0),  RGB(31, 17, 3),  RGB(31, 23, 11), RGB(31, 28, 21)},
    [MP_BALL_CYAN]   = {RGB(0, 13, 15),  RGB(0, 18, 20),  RGB(3, 24, 26),  RGB(14, 28, 30), RGB(25, 31, 31)},
    [MP_BALL_BLACK]  = {RGB(2, 2, 3),    RGB(4, 4, 5),    RGB(7, 7, 8),    RGB(12, 12, 13), RGB(19, 19, 20)},
    [MP_BALL_SILVER] = {RGB(12, 13, 14), RGB(17, 18, 19), RGB(22, 23, 24), RGB(26, 27, 28), RGB(30, 30, 31)},
    [MP_BALL_GOLD]   = {RGB(16, 11, 1),  RGB(22, 16, 2),  RGB(27, 21, 5),  RGB(31, 26, 11), RGB(31, 30, 21)},
};

static const u8 sColorNames[MP_BALL_COLORS][8] = {
    [MP_BALL_RED]    = _("RED"),
    [MP_BALL_BLUE]   = _("BLUE"),
    [MP_BALL_GREEN]  = _("GREEN"),
    [MP_BALL_YELLOW] = _("YELLOW"),
    [MP_BALL_PINK]   = _("PINK"),
    [MP_BALL_PURPLE] = _("PURPLE"),
    [MP_BALL_ORANGE] = _("ORANGE"),
    [MP_BALL_CYAN]   = _("CYAN"),
    [MP_BALL_BLACK]  = _("BLACK"),
    [MP_BALL_SILVER] = _("SILVER"),
    [MP_BALL_GOLD]   = _("GOLD"),
};

// Decorations, from the labeled art sheets (none yet). Index 0 is "none".
struct MpBallDecoration
{
    const u8 *name;
    const u8 *iconGfx;      // 24x24 4bpp overlay for the item icon (preview), colors 12-15 of its palette
    const u8 *battleGfx;    // 16x16 4bpp overlay for the battle / summary ball
};

static const struct MpBallDecoration sDecorations[] = {
    {NULL, NULL, NULL},
};

static const u8 sText_None[] = _("NONE");

u8 MP_BallStyle_Color(u16 style)
{
    u8 c = style & 0xF;
    return c < MP_BALL_COLORS ? c : MP_BALL_RED;
}

u8 MP_BallStyle_Decoration(u16 style)
{
    u8 d = (style >> 4) & 0xFF;
    return d < ARRAY_COUNT(sDecorations) ? d : 0;
}

u16 MP_BallStyle_Make(u8 color, u8 decoration)
{
    return (color & 0xF) | ((u16)decoration << 4);
}

u16 MP_BallStyle_OfMon(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_ENCRYPT_SEPARATOR) & 0xFFF;
}

u16 MP_BallStyle_OfBoxMon(struct BoxPokemon *mon)
{
    return GetBoxMonData(mon, MON_DATA_ENCRYPT_SEPARATOR) & 0xFFF;
}

const u8 *MP_BallStyle_ColorName(u8 color)
{
    return sColorNames[color < MP_BALL_COLORS ? color : 0];
}

u8 MP_BallStyle_DecorationCount(void)
{
    return ARRAY_COUNT(sDecorations);   // including "none"
}

const u8 *MP_BallStyle_DecorationName(u8 decoration)
{
    if (decoration == 0 || decoration >= ARRAY_COUNT(sDecorations) || !sDecorations[decoration].name)
        return sText_None;
    return sDecorations[decoration].name;
}

bool8 MP_BallStyle_CanColor(u8 ballId)
{
    return ballId == BALL_POKE;
}

bool8 MP_BallStyle_CanDecorate(u8 ballId)
{
    return (ballId == BALL_POKE || ballId == BALL_PREMIER) && ARRAY_COUNT(sDecorations) > 1;
}

// The battle / summary ball: palette indices 1-4 are the red top
static void RecolorBallPalette(u16 *pal, u8 color)
{
    pal[1] = sRamps[color][0];
    pal[2] = sRamps[color][1];
    pal[3] = sRamps[color][2];
    pal[4] = sRamps[color][3];
}

// Gives a ball sprite of a Moemon its color (call right after creating it)
void MP_BallStyle_ApplyToBallSprite(u8 spriteId, u8 ballId, u16 style)
{
    u8 color = MP_BallStyle_Color(style), slot;
    u16 tag;
    if (spriteId >= MAX_SPRITES || !MP_BallStyle_CanColor(ballId) || color == MP_BALL_RED)
        return;
    tag = TAG_MP_BALL_COLOR + color;
    slot = IndexOfSpritePaletteTag(tag);
    if (slot == 0xFF)
    {
        static u16 sPal[16];
        struct SpritePalette pal = {sPal, tag};
        LZDecompressWram(gBallSpritePalettes[BALL_POKE].data, sPal);
        RecolorBallPalette(sPal, color);
        slot = LoadSpritePalette(&pal);
        if (slot == 0xFF)
            return;
    }
    gSprites[spriteId].oam.paletteNum = slot;
}

// The item icon (the preview): palette indices 7-11 are the red top
void MP_BallStyle_ApplyToIconPalette(u8 paletteNum, u8 ballId, u16 style)
{
    static const u8 sIdx[5] = {7, 8, 9, 10, 11};
    static const u8 sShade[5] = {0, 1, 1, 2, 4};
    u8 color = MP_BallStyle_Color(style), i;
    if (!MP_BallStyle_CanColor(ballId) || color == MP_BALL_RED)
        return;
    for (i = 0; i < 5; i++)
    {
        u16 c = sRamps[color][sShade[i]];
        gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum) + sIdx[i]] = c;
        gPlttBufferFaded[OBJ_PLTT_ID(paletteNum) + sIdx[i]] = c;
    }
}
