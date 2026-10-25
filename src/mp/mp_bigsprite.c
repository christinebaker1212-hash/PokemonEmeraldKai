// Moemon PC: sprites of any size (include/mp_bigsprite.h). A big sprite's OAM entry keeps its place in the sorted
// list (so the depth order with the player, NPCs and grass is the game's own); its picture and exact position go
// in a side list the renderer reads. The list is built with the OAM buffer and swapped in when LoadOam copies the
// buffer to OAM, so the two always match (a frame whose OAM load is skipped keeps the old list too).
#include <string.h>
#include "global.h"
#include "main.h"
#include "sprite.h"
#include "mp_bigsprite.h"

bool8 MP_IsReflectionSprite(struct Sprite *sprite);

static struct MpBigOam sPending[OAM_ENTRY_COUNT], sLive[OAM_ENTRY_COUNT];
static u16 sPendingCount, sLiveCount;

void MP_BigSprite_BeginOam(void)
{
    sPendingCount = 0;
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
    gx = sprite->x + sprite->x2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetX : 0);
    gy = sprite->y + sprite->y2 + (sprite->coordOffsetEnabled ? gSpriteCoordOffsetY : 0);
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
}

const struct MpBigOam *MP_BigSprite_Oam(u16 index)
{
    return index < sLiveCount ? &sLive[index] : NULL;
}
