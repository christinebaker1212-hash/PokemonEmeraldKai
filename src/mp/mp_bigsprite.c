// Moemon PC: sprites of any size (include/mp_bigsprite.h). A big sprite's OAM entry keeps its place in the sorted
// list (so the depth order with the player, NPCs and grass is the game's own); its picture and exact position go
// in a side list the renderer reads. The list is built with the OAM buffer and swapped in when LoadOam copies the
// buffer to OAM, so the two always match (a frame whose OAM load is skipped keeps the old list too).
#include <string.h>
#include "global.h"
extern s16 gMpPlaceDx[], gMpPlaceDy[];   // sprite.c
#include "o3d.h"
#include "main.h"
#include "sprite.h"
#include "mp_bigsprite.h"

bool8 MP_IsReflectionSprite(struct Sprite *sprite);
bool8 MP_SpriteFeetRow(const struct Sprite *sp, s16 *row);   // mp_shadow.c

static struct MpBigOam sPending[OAM_ENTRY_COUNT], sLive[OAM_ENTRY_COUNT];
static u16 sPendingCount, sLiveCount;
// Any sprite's OAM entries can stop at a screen line (sprite->mpClipOn: a Pokémon in water, half in the ground)
#define NO_CLIP 0x7FFF
static s16 sPendingClip[OAM_ENTRY_COUNT], sLiveClip[OAM_ENTRY_COUNT];
static bool8 sPendingAnyClip, sLiveAnyClip;
// ... and be drawn smaller (sprite->mpScale), around the bottom middle of all of them
struct ScaleNote { u16 scale; s16 ax, ay; };
static struct ScaleNote sPendingScale[OAM_ENTRY_COUNT], sLiveScale[OAM_ENTRY_COUNT];
static bool8 sPendingAnyScale, sLiveAnyScale;

void MP_BigSprite_BeginOam(void)
{
    sPendingCount = 0;
    if (sPendingAnyScale)
    {
        memset(sPendingScale, 0, sizeof(sPendingScale));
        sPendingAnyScale = FALSE;
    }
    if (sPendingAnyClip)
    {
        u16 i;
        for (i = 0; i < OAM_ENTRY_COUNT; i++)
            sPendingClip[i] = NO_CLIP;
        sPendingAnyClip = FALSE;
    }
}

void MP_BigSprite_NoteClip(struct Sprite *sprite, u16 first, u16 end)
{
    u16 i;
    if (!sprite->mpClipOn)
        return;
    if (!sPendingAnyClip)
    {
        for (i = 0; i < OAM_ENTRY_COUNT; i++)
            sPendingClip[i] = NO_CLIP;
        sPendingAnyClip = TRUE;
    }
    for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
        sPendingClip[i] = sprite->mpClipY;
}

// Any sprite's OAM entries can be drawn smaller (sprite->mpScale), around the bottom middle of all of them

static void EntryRect(u16 i, s32 *x0, s32 *y0, s32 *x1, s32 *y1)
{
    static const u8 sSizes[3][4][2] = {{{8, 8}, {16, 16}, {32, 32}, {64, 64}}, {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
                                       {{8, 16}, {8, 32}, {16, 32}, {32, 64}}};
    const struct OamData *oam = &gMain.oamBuffer[i];
    s32 x, y;
    if (oam->shape == MP_BIG_OAM_SHAPE)
    {
        const struct MpBigOam *e = &sPending[oam->tileNum];
        *x0 = e->left, *y0 = e->top, *x1 = e->left + e->w, *y1 = e->top + e->h;
        return;
    }
    x = oam->x >= DISPLAY_WIDTH ? (s32)oam->x - 512 : oam->x;
    y = oam->y >= DISPLAY_HEIGHT ? (s32)oam->y - 256 : oam->y;
    *x0 = x, *y0 = y;
    *x1 = x + sSizes[oam->shape % 3][oam->size][0];
    *y1 = y + sSizes[oam->shape % 3][oam->size][1];
}

void MP_BigSprite_NoteScale(struct Sprite *sprite, u16 first, u16 end)
{
    s32 x0 = 0x7FFF, y0 = 0x7FFF, x1 = -0x7FFF, y1 = -0x7FFF, a, b, c, d;
    s16 feet;
    u16 i, scale = sprite->mpScale, o3d = O3D_SpriteScale(sprite - gSprites);
    // (Moemon Kai: under a shot of ORRE's, as big as the shot sees it where it stands: src/orre/o3d.c)
    if (o3d)
        scale = scale ? (u16)((u32)scale * o3d / 256) : o3d;
    if (scale == 0 || scale == 256 || (o3d == 0 && scale > 256) || first >= end)
        return;
    if (!sPendingAnyScale)
    {
        memset(sPendingScale, 0, sizeof(sPendingScale));
        sPendingAnyScale = TRUE;
    }
    if (sprite->mpBig && sprite->mpBig->footed)
    {
        // (1.63.11) toward its own point, the same in every frame: a walk's legs or a hop no longer move what it
        // shrinks toward; a reflection toward the line it mirrors at
        s32 gx = sprite->x + sprite->x2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetX : 0) + sprite->mpBig->shift;
        s32 gy = sprite->y + sprite->y2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetY : 0);
        if (sprite->coordOffsetEnabled)
            gx += gMpPlaceDx[sprite - gSprites], gy += gMpPlaceDy[sprite - gSprites];
        x0 = x1 = gx;
        y1 = MP_IsReflectionSprite(sprite) ? gy + sprite->mpBig->lift : gy + sprite->mpBig->footY;
    }
    else if (sprite->mpCastShadow && sprite->mpBig == NULL && !MP_IsReflectionSprite(sprite)
             && MP_SpriteFeetRow(sprite, &feet))
    {
        // (1.63.13) a follower's or Garden POKeMON's own sprite: toward its feet and its point (mp_follower.c puts its
        // body's middle there), the same in every frame, as a PMD picture
        x0 = x1 = sprite->x + sprite->x2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetX : 0);
        y1 = sprite->y + sprite->y2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetY : 0) + feet;
        if (sprite->coordOffsetEnabled)
            x0 = x1 = x0 + gMpPlaceDx[sprite - gSprites], y1 += gMpPlaceDy[sprite - gSprites];
    }
    else
    {
        for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
        {
            EntryRect(i, &a, &b, &c, &d);
            if (a < x0) x0 = a;
            if (b < y0) y0 = b;
            if (c > x1) x1 = c;
            if (d > y1) y1 = d;
        }
    }
    for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
    {
        sPendingScale[i].scale = scale;
        sPendingScale[i].ax = (x0 + x1) / 2;
        sPendingScale[i].ay = y1;
    }
}

u16 MP_BigSprite_OamScale(u16 oamIndex, s16 *ax, s16 *ay)
{
    if (!sLiveAnyScale || oamIndex >= OAM_ENTRY_COUNT || sLiveScale[oamIndex].scale == 0)
        return 0;
    *ax = sLiveScale[oamIndex].ax;
    *ay = sLiveScale[oamIndex].ay;
    return sLiveScale[oamIndex].scale;
}

int MP_BigSprite_OamClipY(u16 oamIndex)
{
    return sLiveAnyClip && oamIndex < OAM_ENTRY_COUNT ? sLiveClip[oamIndex] : NO_CLIP;
}

bool8 MP_BigSprite_AddOam(struct Sprite *sprite, u16 *oamIndex)
{
    const struct MpBigSprite *big = sprite->mpBig;
    const u8 *f = big->frame;
    struct MpBigOam *e;
    struct OamData *oam;
    s16 gx, gy, dx, dy;

    if (*oamIndex >= gOamLimit)
        return TRUE;
    if (f == NULL || sPendingCount >= OAM_ENTRY_COUNT)
        return FALSE;   // nothing to show this frame
    dx = (s16)(f[0] | (f[1] << 8));
    dy = (s16)(f[2] | (f[3] << 8));
    e = &sPending[sPendingCount];
    e->big = big;
    e->w = f[4] | (f[5] << 8);
    e->h = f[6] | (f[7] << 8);
    e->pixels = f + 8;
    e->bpp = big->bpp;
    if (e->w == 0 || e->h == 0)
        return FALSE;
    e->hflip = !(sprite->oam.affineMode & 1) && ((sprite->oam.matrixNum >> 3) & 1);
    e->vflip = !(sprite->oam.affineMode & 1) && ((sprite->oam.matrixNum >> 4) & 1);
    gx = sprite->x + sprite->x2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetX : 0) + big->shift;
    gy = sprite->y + sprite->y2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetY : 0) + big->lift;
    // (on a 3D map, SINNOH's or ORRE's: where the view put the sprite, as its own pictures are. It stayed on the flat
    // map's spot, drifting off its place as the camera turned, its legs under the ground: the user, 2026-11-11)
    if (sprite->coordOffsetEnabled && sprite - gSprites < MAX_SPRITES)
        gx += gMpPlaceDx[sprite - gSprites], gy += gMpPlaceDy[sprite - gSprites];
    e->left = e->hflip ? gx - dx - e->w : gx + dx;   // flips mirror the picture around the sprite's x / y
    e->top = e->vflip ? gy - dy - e->h : gy + dy;
    // affine: a reflection mirrors at the feet (the sprite's own point); anything else (ball effects scaling and
    // turning it) about the picture's middle
    if (MP_IsReflectionSprite(sprite))
    {
        e->gx = gx;
        e->gy = gy;
        e->px = -dx;
        e->py = -dy;
    }
    else
    {
        e->px = e->w / 2;
        e->py = e->h / 2;
        e->gx = gx + dx + e->px;
        e->gy = gy + dy + e->py;
    }
    {
        s16 r = 0, v[4] = {e->px, e->w - e->px, e->py, e->h - e->py};
        u8 i;
        for (i = 0; i < 4; i++)
            if ((v[i] < 0 ? -v[i] : v[i]) > r)
                r = v[i] < 0 ? -v[i] : v[i];
        e->reach = 2 * r + 2;   // (an effect can grow it to about twice its size)
    }

    oam = &gMain.oamBuffer[*oamIndex];
    *oam = sprite->oam;
    oam->shape = MP_BIG_OAM_SHAPE;
    oam->tileNum = sPendingCount;
    oam->x = e->left & 0x1FF;   // (the renderer uses the side list's coordinates)
    oam->y = e->top & 0xFF;
    sPendingCount++;
    (*oamIndex)++;
    return FALSE;
}

void MP_BigSprite_CommitOam(void)
{
    memcpy(sLive, sPending, sPendingCount * sizeof(sLive[0]));
    sLiveCount = sPendingCount;
    sLiveAnyClip = sPendingAnyClip;
    sLiveAnyScale = sPendingAnyScale;
    if (sPendingAnyScale)
        memcpy(sLiveScale, sPendingScale, sizeof(sLiveScale));
    if (sPendingAnyClip)
        memcpy(sLiveClip, sPendingClip, sizeof(sLiveClip));
}

const struct MpBigOam *MP_BigSprite_Oam(u16 index)
{
    return index < sLiveCount ? &sLive[index] : NULL;
}
