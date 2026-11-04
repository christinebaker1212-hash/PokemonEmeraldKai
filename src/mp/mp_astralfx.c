// Moemon Co-op: the ASTRAL BALL's looks, drawn over the finished frame (sdl2.c VDraw) like the other effects
// layers: no sprite or palette is used and the ball's art is never changed.
//  - A prismatic sheen (MP_AstralFx_Prism, right after the game's frame is drawn): the ball's own pixels, found
//    by its palette's colors inside its box, tinted by a rainbow that slides across it and breathes, with a
//    slower second shimmer of lighter and darker patches drifting the other way. The outline and shine stay.
//  - Silver sparkles (MP_AstralFx_Composite): four-pointed stars twinkling around the ball, a trail while it
//    moves.
// Both show on an ASTRAL BALL that hasn't caught anything yet: its item icon (the BAG, shops, the MoeGatcha) and
// the thrown ball (battle_anim_throw.c MP_AstralFx_Thrown). When it clicks shut on a catch (MP_AstralFx_Caught)
// the sheen fades to the ball's own grey and every sparkle is pulled into it; after that it's a plain ball
// (the summary's, a send-out). Its own random numbers: the game's are never touched.
#include <string.h>
#include "global.h"
#include "main.h"
#include "mp.h"
#include "palette.h"
#include "sprite.h"
#include "constants/items.h"
#include "constants/rgb.h"

#define MAX_ICONS 4
#define MAX_BALLS 4
#define MAX_SPARKS 64
#define SPARK_LIFE 26
#define FADE_FRAMES 36   // the catch: the sheen fades and the sparkles fly in over this many frames
#define FX 16            // sparkle positions in 1/16 pixels

struct AstralIcon { u8 spriteId; u32 sum; };   // an item icon sprite and its tiles' checksum
struct AstralBall { u8 spriteId; bool8 on; u8 fade; s16 x, y; };   // a thrown ball; fade counts up after the click
struct AstralSpark { s32 x, y; u8 age, size; bool8 on, inward; };

static struct AstralIcon sIcons[MAX_ICONS];
static struct AstralBall sBalls[MAX_BALLS];
static struct AstralSpark sSparks[MAX_SPARKS];
static u8 sIconCount;
// The icons' last positions (sIcons order)
static s16 sIconX[MAX_ICONS] = {-1000, -1000, -1000, -1000}, sIconY[MAX_ICONS];
static u8 sLastBright = 16;   // the balls' fade, kept for the sparkles left after a ball is gone
static u16 sTime;
static u32 sRng = 0x5EED1234;
static s16 sSuckX, sSuckY;    // where sparkles fly to after a click
static u8 sSuckTimer;

static u32 Rand(void)
{
    sRng = sRng * 1103515245 + 12345;
    return sRng >> 16;
}

// The OBJ shapes' sizes (shape, size) -> width, height
static const u8 sDims[3][4][2] = {
    {{8, 8}, {16, 16}, {32, 32}, {64, 64}},
    {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
    {{8, 16}, {8, 32}, {16, 32}, {32, 64}},
};

static u32 TileSum(u8 spriteId)
{
    const u8 *tiles = (const u8 *)OBJ_VRAM0 + gSprites[spriteId].oam.tileNum * TILE_SIZE_4BPP;
    u32 sum = 0, i;
    for (i = 0; i < 0x200; i++)   // the 32x32 icon's 16 tiles
        sum = sum * 31 + tiles[i];
    return sum;
}

// item_icon.c: an icon sprite was just made for this item (its tiles are in VRAM now)
void MP_AstralFx_Icon(u8 spriteId, u16 itemId)
{
    u8 i;
    if (spriteId >= MAX_SPRITES || itemId != ITEM_ASTRAL_BALL)
        return;
    for (i = 0; i < sIconCount; i++)
        if (sIcons[i].spriteId == spriteId)
            break;
    if (i == sIconCount)
    {
        if (sIconCount == MAX_ICONS)   // (the oldest goes)
        {
            memmove(&sIcons[0], &sIcons[1], sizeof(sIcons[0]) * (MAX_ICONS - 1));
            i = MAX_ICONS - 1;
        }
        else
            sIconCount++;
    }
    sIcons[i].spriteId = spriteId;
    sIcons[i].sum = TileSum(spriteId);
}

// An icon that's a Moemon's own ball (the summary's POKé BALL page): no sheen, no sparkles
void MP_AstralFx_Forget(u8 spriteId)
{
    u8 i;
    for (i = 0; i < sIconCount; i++)
        if (sIcons[i].spriteId == spriteId)
        {
            memmove(&sIcons[i], &sIcons[i + 1], sizeof(sIcons[0]) * (sIconCount - i - 1));
            memmove(&sIconX[i], &sIconX[i + 1], sizeof(sIconX[0]) * (sIconCount - i - 1));
            memmove(&sIconY[i], &sIconY[i + 1], sizeof(sIconY[0]) * (sIconCount - i - 1));
            sIconX[--sIconCount] = -1000;
            return;
        }
}

// battle_anim_throw.c: an ASTRAL BALL was just thrown (this sprite)
void MP_AstralFx_Thrown(u8 spriteId)
{
    u8 i, slot = 0;
    if (spriteId >= MAX_SPRITES)
        return;
    for (i = 0; i < MAX_BALLS; i++)
        if (!sBalls[i].on || sBalls[i].spriteId == spriteId)
        {
            slot = i;
            break;
        }
    sBalls[slot].spriteId = spriteId;
    sBalls[slot].on = TRUE;
    sBalls[slot].fade = 0;
    sBalls[slot].x = sBalls[slot].y = -1000;
}

static struct AstralBall *BallOf(u8 spriteId)
{
    u8 i;
    for (i = 0; i < MAX_BALLS; i++)
        if (sBalls[i].on && sBalls[i].spriteId == spriteId)
            return &sBalls[i];
    return NULL;
}

// Where a sprite is on the screen; FALSE if it isn't shown
static bool8 SpriteBox(u8 id, s16 *x, s16 *y, s16 *w, s16 *h)
{
    struct Sprite *s = &gSprites[id];
    if (!s->inUse || s->invisible || s->oam.shape > 2)
        return FALSE;
    *w = sDims[s->oam.shape][s->oam.size][0];
    *h = sDims[s->oam.shape][s->oam.size][1];
    if ((s->oam.affineMode & ST_OAM_AFFINE_DOUBLE_MASK) == ST_OAM_AFFINE_DOUBLE_MASK)
        *w *= 2, *h *= 2;
    *x = s->oam.x;
    *y = s->oam.y;
    if (*x >= 256)
        *x -= 512;
    if (*y + *h > 256)
        *y -= 256;
    return *x + *w > 0 && *x < DISPLAY_WIDTH && *y + *h > 0 && *y < DISPLAY_HEIGHT;
}

// How far the sprite's palette is faded (0 = gone .. 16 = as it is), so the sparkles fade with it
static u8 Brightness(u8 id)
{
    u16 base = OBJ_PLTT_ID(gSprites[id].oam.paletteNum), i;
    u32 a = 0, b = 0;
    for (i = 1; i < 16; i++)
    {
        u16 u = gPlttBufferUnfaded[base + i], f = gPlttBufferFaded[base + i];
        a += (u & 31) + ((u >> 5) & 31) + ((u >> 10) & 31);
        b += (f & 31) + ((f >> 5) & 31) + ((f >> 10) & 31);
    }
    if (a == 0)
        return 16;
    return b >= a ? 16 : (u8)(b * 16 / a);
}

// battle_anim_throw.c: the ball clicked shut on a catch. Its sheen fades to its own grey and the sparkles
// around it (and a ring of new ones) fly into it.
void MP_AstralFx_Caught(u8 spriteId)
{
    struct AstralBall *ball = BallOf(spriteId);
    s16 x, y, w, h;
    u8 i, k;
    if (ball == NULL || ball->fade)
        return;
    ball->fade = 1;
    if (!SpriteBox(spriteId, &x, &y, &w, &h))
        return;
    sSuckX = x + w / 2;
    sSuckY = y + h / 2;
    sSuckTimer = FADE_FRAMES;
    for (i = 0; i < MAX_SPARKS; i++)
        if (sSparks[i].on)
            sSparks[i].inward = TRUE, sSparks[i].age = SPARK_LIFE / 2;   // at their brightest as they go
    // a ring of twelve more, so the pull reads
    for (k = 0; k < 12; k++)
    {
        static const s8 sRing[12][2] = {{18, 0}, {16, 9}, {9, 16}, {0, 18}, {-9, 16}, {-16, 9},
                                        {-18, 0}, {-16, -9}, {-9, -16}, {0, -18}, {9, -16}, {16, -9}};
        for (i = 0; i < MAX_SPARKS; i++)
            if (!sSparks[i].on)
                break;
        if (i == MAX_SPARKS)
            break;
        sSparks[i].x = (sSuckX + sRing[k][0] + (s16)(Rand() % 5) - 2) * FX;
        sSparks[i].y = (sSuckY + sRing[k][1] + (s16)(Rand() % 5) - 2) * FX;
        sSparks[i].age = (k * 3) % (SPARK_LIFE / 2);
        sSparks[i].size = 2 + Rand() % 2;
        sSparks[i].on = TRUE;
        sSparks[i].inward = TRUE;
    }
}

static void Spawn(s16 cx, s16 cy, u8 rMin, u8 rMax)
{
    u8 i;
    s16 dx, dy;
    for (i = 0; i < MAX_SPARKS; i++)
        if (!sSparks[i].on)
            break;
    if (i == MAX_SPARKS)
        return;
    // a point in the ring around the ball
    do
    {
        dx = (s16)(Rand() % (rMax * 2 + 1)) - rMax;
        dy = (s16)(Rand() % (rMax * 2 + 1)) - rMax;
    } while (dx * dx + dy * dy < rMin * rMin || dx * dx + dy * dy > rMax * rMax);
    sSparks[i].x = (cx + dx) * FX;
    sSparks[i].y = (cy + dy) * FX;
    sSparks[i].age = 0;
    sSparks[i].size = 2 + Rand() % 2;   // arms of 2 or 3 pixels at the brightest
    sSparks[i].on = TRUE;
    sSparks[i].inward = FALSE;
}

static u16 Mix(u16 dst, u16 src, u8 a)   // a: 0..16
{
    u16 r = ((dst & 31) * (16 - a) + (src & 31) * a) / 16;
    u16 g = (((dst >> 5) & 31) * (16 - a) + ((src >> 5) & 31) * a) / 16;
    u16 b = (((dst >> 10) & 31) * (16 - a) + ((src >> 10) & 31) * a) / 16;
    return r | (g << 5) | (b << 10);
}

// ---------------------------------------------------------------------------
// The prismatic sheen

// hue 0..191 (six sixths of 32) -> a full-strength color, 0..31 a channel
static void HueRgb(u16 hue, u8 *r, u8 *g, u8 *b)
{
    u8 f = hue % 32, seg = hue / 32;
    u8 up = f, down = 31 - f;
    switch (seg)
    {
    case 0: *r = 31; *g = up; *b = 0; break;
    case 1: *r = down; *g = 31; *b = 0; break;
    case 2: *r = 0; *g = 31; *b = up; break;
    case 3: *r = 0; *g = down; *b = 31; break;
    case 4: *r = up; *g = 0; *b = 31; break;
    default: *r = 31; *g = 0; *b = down; break;
    }
}

// A triangle wave: v of any size -> -16..16 over a period of 64
static s8 Tri(s32 v)
{
    s32 p = v & 63;
    return (s8)(p < 32 ? p - 16 : 47 - p);
}

static u8 Clamp31(s32 v)
{
    return v < 0 ? 0 : (v > 31 ? 31 : v);
}

// strength: 0..16 (16 = the full sheen; a caught ball fades it out)
static void PrismSprite(u16 *frame, u8 id, s16 bx, s16 by, s16 bw, s16 bh, u8 strength)
{
    const u16 *pal = (const u16 *)(PLTT + 0x200) + gSprites[id].oam.paletteNum * 16;
    u16 colors[15];
    u8 i, wave;
    s16 x, y;
    u16 t = sTime;

    for (i = 0; i < 15; i++)
        colors[i] = pal[i + 1] & 0x7FFF;
    // the rainbow's strength breathes: 9..15 of 16 over about a second and a half
    wave = (t / 3) % 32;
    wave = wave < 16 ? wave : 31 - wave;
    for (y = by < 0 ? 0 : by; y < by + bh && y < DISPLAY_HEIGHT; y++)
        for (x = bx < 0 ? 0 : bx; x < bx + bw && x < DISPLAY_WIDTH; x++)
        {
            u16 *dst = &frame[y * DISPLAY_WIDTH + x], c = *dst & 0x7FFF;
            u8 r, g, b, hr, hg, hb, lum, a;
            s16 hue, shim;
            for (i = 0; i < 15; i++)
                if (colors[i] == c)
                    break;
            if (i == 15)
                continue;   // not the ball's pixel
            r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
            lum = (r * 3 + g * 6 + b) / 10;
            if (lum < 6)
                continue;   // the outline
            // bands running down-left to up-right, sliding along; a slower second wave bends them
            hue = (x - y) * 7 + t * 2 + Tri(t / 2 + y * 4) + 16;
            hue %= 192;
            if (hue < 0)
                hue += 192;
            HueRgb(hue, &hr, &hg, &hb);
            // the rainbow at this pixel's brightness, a little pastel so it reads as light, not paint
            hr = Clamp31((lum * (12 + hr)) / 28);
            hg = Clamp31((lum * (12 + hg)) / 28);
            hb = Clamp31((lum * (12 + hb)) / 28);
            a = 9 + wave * 6 / 15;
            if (lum > 27)
                a /= 2;     // the shine stays nearly white
            else if (lum < 10)
                a = a * (lum - 5) / 5;   // fading into the outline
            a = a * strength / 16;
            c = Mix(c, RGB(hr, hg, hb), a);
            // the second shimmer: lighter and darker patches where two slow waves meet, drifting the other way
            shim = Tri((x + y) * 5 - t) + Tri(x * 3 - y * 7 + t * 2 / 3 + 20);   // -32..32
            shim = shim * (lum < 10 ? lum - 5 : 5) / 5 * strength / 16;          // (not on the outline)
            if (shim != 0)
            {
                s16 k = 16 + shim / 5;   // x(10..22)/16
                c = RGB(Clamp31((c & 31) * k / 16), Clamp31(((c >> 5) & 31) * k / 16), Clamp31(((c >> 10) & 31) * k / 16));
            }
            *dst = c | (*dst & 0x8000);
        }
}

// Is this battle-sheet sprite a thrown ASTRAL BALL still showing off? (its sheen's strength, 0: plain)
static u8 BallStrength(u8 id)
{
    struct AstralBall *ball = BallOf(id);
    if (ball == NULL)
        return 0;   // the summary's ball, a send-out: a plain ball
    if (ball->fade == 0)
        return 16;
    return ball->fade >= FADE_FRAMES ? 0 : (u8)(16 * (FADE_FRAMES - ball->fade) / FADE_FRAMES);
}

void MP_AstralFx_Prism(u16 *frame)
{
    u8 i;
    s16 x, y, w, h;
    u16 sheet = MP_BallStyle_SheetStart(MP_SPECIAL_ASTRAL);

    if ((sIconCount == 0 && sheet == 0xFFFF) || !MP_OBJ_LAYER_SHOWN())
        return;
    sTime++;
    for (i = 0; i < sIconCount; i++)
    {
        u8 id = sIcons[i].spriteId;
        if (gSprites[id].inUse && TileSum(id) == sIcons[i].sum && SpriteBox(id, &x, &y, &w, &h))
            PrismSprite(frame, id, x, y, 24, 24, 16);
    }
    if (sheet != 0xFFFF)
        for (i = 0; i < MAX_SPRITES; i++)
            if (gSprites[i].inUse && gSprites[i].sheetTileStart == sheet && BallStrength(i)
             && SpriteBox(i, &x, &y, &w, &h))
                PrismSprite(frame, i, x, y, w, h, BallStrength(i));
}

// ---------------------------------------------------------------------------
// The sparkles

static void Plot(u16 *frame, s16 x, s16 y, u16 color, u8 a, s16 maxY)
{
    u16 dst;
    if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= maxY || a == 0)
        return;
    dst = frame[y * DISPLAY_WIDTH + x];
    // on a bright background (the BAG's white square) white can't be seen: a darker, bluer silver there
    if ((dst & 31) + ((dst >> 5) & 31) + ((dst >> 10) & 31) > 66)
        color = RGB((color & 31) * 9 / 16, ((color >> 5) & 31) * 10 / 16, ((color >> 10) & 31) * 13 / 16);
    frame[y * DISPLAY_WIDTH + x] = Mix(dst, color, a) | 0x8000;
}

static void DrawSpark(u16 *frame, const struct AstralSpark *s, u8 bright, s16 maxY)
{
    // grows to its size and back; the core white, the arms silver fading out
    u8 half = SPARK_LIFE / 2, t = s->age < half ? s->age : SPARK_LIFE - s->age, len, d;
    s16 x = s->x / FX, y = s->y / FX;
    u8 a = bright;
    len = (t * (s->size + 1)) / half;
    if (len > s->size)
        len = s->size;
    Plot(frame, x, y, RGB(31, 31, 31), a * (t < 3 ? t + 1 : 4) / 4, maxY);
    for (d = 1; d <= len; d++)
    {
        u8 arm = a * (len + 1 - d) / (len + 1);
        Plot(frame, x - d, y, RGB(25, 27, 31), arm, maxY);
        Plot(frame, x + d, y, RGB(25, 27, 31), arm, maxY);
        Plot(frame, x, y - d, RGB(25, 27, 31), arm, maxY);
        Plot(frame, x, y + d, RGB(25, 27, 31), arm, maxY);
    }
    if (len >= 3)   // the brightest moment: a faint glint on the diagonals too
    {
        Plot(frame, x - 1, y - 1, RGB(22, 24, 30), a / 3, maxY);
        Plot(frame, x + 1, y - 1, RGB(22, 24, 30), a / 3, maxY);
        Plot(frame, x - 1, y + 1, RGB(22, 24, 30), a / 3, maxY);
        Plot(frame, x + 1, y + 1, RGB(22, 24, 30), a / 3, maxY);
    }
}

// One ball showing off this frame: twinkles around it, more of them (and a trail) while it moves.
// lastX / lastY: where it was a frame ago (-1000: unknown)
static void Emit(s16 cx, s16 cy, s16 *lastX, s16 *lastY, u8 rMin, u8 rMax, u8 bright)
{
    s32 moved = *lastX == -1000 ? 0 : (cx - *lastX) * (cx - *lastX) + (cy - *lastY) * (cy - *lastY);
    if (bright > 4)
    {
        if (moved > 4)
        {
            Spawn(*lastX, *lastY, 0, 3);   // the trail, where it just was
            if (Rand() % 2 == 0)
                Spawn(cx, cy, rMin, rMax);
        }
        else if (Rand() % 6 == 0)
        {
            Spawn(cx, cy, rMin, rMax);
        }
    }
    *lastX = cx;
    *lastY = cy;
}


void MP_AstralFx_Composite(u16 *frame)
{
    u8 i, maxBright = 0;
    s16 x, y, w, h, maxY = DISPLAY_HEIGHT;
    u16 sheet = MP_BallStyle_SheetStart(MP_SPECIAL_ASTRAL);
    bool8 any = FALSE;

    for (i = 0; i < MAX_SPARKS && !any; i++)
        any = sSparks[i].on;
    // forget thrown balls whose sprite is gone (or now another sprite)
    for (i = 0; i < MAX_BALLS; i++)
        if (sBalls[i].on && (sheet == 0xFFFF || !gSprites[sBalls[i].spriteId].inUse
                             || gSprites[sBalls[i].spriteId].sheetTileStart != sheet))
            sBalls[i].on = FALSE;
    if (!any && sIconCount == 0 && sheet == 0xFFFF)
        return;   // (nothing ASTRAL anywhere: the usual case)
    if (!MP_OBJ_LAYER_SHOWN())
    {
        if (any)
            memset(sSparks, 0, sizeof(sSparks));
        return;
    }

    // the item icons (the ball art is the icon's top-left 24x24)
    for (i = 0; i < sIconCount; )
    {
        u8 id = sIcons[i].spriteId;
        if (!gSprites[id].inUse || TileSum(id) != sIcons[i].sum)
        {
            memmove(&sIcons[i], &sIcons[i + 1], sizeof(sIcons[0]) * (sIconCount - i - 1));
            memmove(&sIconX[i], &sIconX[i + 1], sizeof(sIconX[0]) * (sIconCount - i - 1));
            memmove(&sIconY[i], &sIconY[i + 1], sizeof(sIconY[0]) * (sIconCount - i - 1));
            sIconX[--sIconCount] = -1000;
            continue;
        }
        if (SpriteBox(id, &x, &y, &w, &h))
        {
            u8 b = Brightness(id);
            Emit(x + 12, y + 12, &sIconX[i], &sIconY[i], 7, 14, b);
            if (b > maxBright)
                maxBright = b;
        }
        i++;
    }
    // the thrown balls (their own sheet: the ball's middle at (8, 16) of each 16x32 frame)
    if (gMain.inBattle)
        maxY = 112;   // the battle text box stays clear
    for (i = 0; i < MAX_BALLS; i++)
    {
        struct AstralBall *ball = &sBalls[i];
        if (!ball->on || !SpriteBox(ball->spriteId, &x, &y, &w, &h))
            continue;
        if (ball->fade == 0)
        {
            u8 b = Brightness(ball->spriteId);
            Emit(x + w / 2, y + h / 2, &ball->x, &ball->y, 5, 11, b);
            if (b > maxBright)
                maxBright = b;
        }
        else
        {
            sSuckX = x + w / 2;   // (the pull follows the ball as it settles)
            sSuckY = y + h / 2;
            if (ball->fade < FADE_FRAMES)
                ball->fade++;
            if (maxBright < 12)
                maxBright = 12;   // (the click dims its palette: the flying sparkles stay bright)
        }
    }
    if (maxBright)
        sLastBright = maxBright;

    for (i = 0; i < MAX_SPARKS; i++)
    {
        struct AstralSpark *s = &sSparks[i];
        if (!s->on)
            continue;
        if (s->inward)
        {
            // pulled in, faster and faster, shrinking as it arrives
            s32 dx = sSuckX * FX - s->x, dy = sSuckY * FX - s->y;
            u8 left = sSuckTimer ? sSuckTimer : 1;
            s->x += dx * 4 / (left + 3);
            s->y += dy * 4 / (left + 3);
            if (dx * dx + dy * dy < (2 * FX) * (2 * FX))
            {
                s->on = FALSE;
                continue;
            }
            if (s->age < SPARK_LIFE / 2)
                s->age++;
            DrawSpark(frame, s, sLastBright, maxY);
            continue;
        }
        DrawSpark(frame, s, sLastBright, maxY);
        if (++s->age >= SPARK_LIFE)
            s->on = FALSE;
    }
    if (sSuckTimer)
    {
        if (--sSuckTimer == 0)
            for (i = 0; i < MAX_SPARKS; i++)
                if (sSparks[i].inward)
                    sSparks[i].on = FALSE;   // (all in by now)
    }
}
