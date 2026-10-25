#ifndef GUARD_MP_BIGSPRITE_H
#define GUARD_MP_BIGSPRITE_H

// Moemon PC: sprites of any size (mp_bigsprite.c). A sprite with mpBig set draws one picture, as big as it is,
// in its own place among the other sprites (priority, subpriority, the order the game sorts them in), so it
// stands behind the player or in tall grass like any sprite. The picture's colors are the sprite's palette slot
// (fades, weather and hues apply as usual); 8-bit pictures take colors 16+ from extraShown.

struct MpBigSprite
{
    const u8 *frame;        // s16 dx, s16 dy (top-left from the sprite's x / y), u16 w, u16 h, then the pixels
    u8 bpp;                 // 4 (rows of (w + 1) / 2 bytes, low nibble first) or 8 (rows of w bytes)
    u8 colorCount;          // palette entries (0 = transparent)
    s16 lift;               // pixels the picture is drawn lower than its frame says (a PMD hop's rise, cut down)
    u16 extraShown[240];    // 8 bpp: colors 16-255 as shown now (faded like the palette slot)
};

struct MpBigOam
{
    const struct MpBigSprite *big;
    const u8 *pixels;
    s16 left, top;          // on screen (the whole picture, after flips)
    u16 w, h;
    s16 gx, gy;             // affine: the point on screen the picture turns around (a reflection: the sprite's own point,
    s16 px, py;             // its feet; anything else: the picture's middle), and that point in the picture
    s16 reach;              // affine: how far from (gx, gy) the turned picture can reach
    u8 bpp;
    bool8 hflip, vflip;
};

struct Sprite;
void MP_BigSprite_BeginOam(void);                                 // AddSpritesToOamBuffer: a new list
bool8 MP_BigSprite_AddOam(struct Sprite *sprite, u16 *oamIndex);  // ... a big sprite's entry
void MP_BigSprite_CommitOam(void);                                // LoadOam: the list goes with the OAM
const struct MpBigOam *MP_BigSprite_Oam(u16 index);               // the renderer (gba_easy_draw.c)

#define MP_BIG_OAM_SHAPE 3   // OAM shape 3 is "prohibited" on the GBA: a big sprite's entry, tileNum = its index

#endif // GUARD_MP_BIGSPRITE_H
