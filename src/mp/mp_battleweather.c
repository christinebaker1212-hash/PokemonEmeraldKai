// Moemon Co-op: weather you can see the whole battle long (task 197). The rain never stops falling, hail keeps coming
// down, the sandstorm rages across the field, and in strong sunlight the Pokémon look brighter, so the game no
// longer replays its weather animation and "Rain continues to fall." line every turn (battle_scripts_1.s).
//
// Drawn over the finished frame like the other co-op layers (sdl2.c), from the game's own art: Rain Dance's drops
// (ANIM_TAG_RAIN_DROPS, falling as AnimRainDrop does), Hail's hailstones and the ice crystals they shatter into
// (AnimTask_Hail), Sandstorm's scrolling sand background and the sand crescents flying across
// (AnimTask_LoadSandstormBackground, AnimFlyingSandCrescent). Sunlight brightens the battlers' sprite palettes in the
// renderer (gMpObjPalBright, gba_easy_draw.c). Only the battle scene is covered (above the message box), the art
// fades with the screen (MP_FadeFit), and it uses its own random numbers so it never touches the battle's.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_main.h"
#include "graphics.h"
#include "main.h"
#include "palette.h"
#include "constants/rgb.h"
#include "sprite.h"
#include "constants/battle_anim.h"
#include "mp.h"

#define SCENE_H       112        // the message box starts here
#define MAX_DROPS     48
#define MAX_STONES    24
#define MAX_SAND      8
#define SAND_ALPHA    5          // the sandstorm's strength over the scene (Sandstorm's own anim peaks at 7/16)
// The tints, as the game's own weather animations blend the scene (General_Rain: the background and the Pokémon
// 4/16 toward black; Move_HAIL: the background 6/16 toward black; Move_SUNNY_DAY: both 6/16 toward white), a little
// lighter since they stay on the whole battle
#define RAIN_DIM      4
#define HAIL_DIM      4
#define SUN_BG        2
#define SUN_MONS      4
#define WARM_WHITE    RGB(31, 31, 26)

u16 (*gMpScenePixel)(u16 color, int x, int y, bool8 sprite);   // the renderer's hook (gba_easy_draw.c)
u8 gMpSceneLayers;
u8 gMpSceneObjPal[64];

struct Art
{
    u8 *tiles;                   // 4bpp, 1D-mapped
    u16 pal[16];
    bool8 loaded;
};

struct Drop  { bool8 on; s16 x, y; u8 t; };
struct Stone { bool8 on; s16 x, y, tx, ty; u8 t, size; bool8 hit; };
struct Sand  { bool8 on; s32 x256, y256; s16 vx, vy; };

static struct
{
    bool8 init;
    struct Art rain, hail, crystal, dirt;
    u8 *sandBg;                  // the Sandstorm background as 256x256 color indices
    u16 sandPal[16];
    struct Drop drops[MAX_DROPS];
    struct Stone stones[MAX_STONES];
    struct Sand sand[MAX_SAND];
    u32 rng;
    u16 frame;
    s16 sandX, sandY;
    u16 lastWeather;
    // what ScenePixel does this frame
    u8 tintBg, tintMons;
    u16 tintColor;
    bool8 sandOn;
    u16 sandShaded[16];
} sW;

static u16 Rand(void)
{
    sW.rng = sW.rng * 1103515245 + 12345;
    return (sW.rng >> 16) & 0x7FFF;
}

static u32 LzSize(const u32 *lz)
{
    return lz[0] >> 8;   // the LZ77 header's uncompressed size
}

static void LoadArt(struct Art *a, u16 tag)
{
    const struct CompressedSpriteSheet *sheet = &gBattleAnimPicTable[tag - ANIM_SPRITES_START];
    const struct CompressedSpritePalette *pal = &gBattleAnimPaletteTable[tag - ANIM_SPRITES_START];
    u16 buf[16];

    if (a->loaded)
        return;
    // (plain malloc, not the game's heap: battles need all of it)
    a->tiles = malloc(LzSize(sheet->data) > sheet->size ? LzSize(sheet->data) : sheet->size);
    if (a->tiles == NULL)
        return;
    LZ77UnCompWram(sheet->data, a->tiles);
    LZ77UnCompWram(pal->data, buf);
    memcpy(a->pal, buf, sizeof(a->pal));
    a->loaded = TRUE;
}

static void LoadSandBackground(void)
{
    u8 *tiles;
    u16 *map, pal[16];
    int tx, ty, x, y;

    u32 nTiles;
    if (sW.sandBg)
        return;
    nTiles = LzSize(gBattleAnimBgImage_Sandstorm) / 32;
    tiles = malloc(nTiles * 32 + 32);
    map = malloc(LzSize(gBattleAnimBgTilemap_Sandstorm) > 0x800 ? LzSize(gBattleAnimBgTilemap_Sandstorm) : 0x800);
    sW.sandBg = malloc(256 * 256);
    if (tiles == NULL || map == NULL || sW.sandBg == NULL)
    {
        free(tiles);
        free(map);
        free(sW.sandBg);
        sW.sandBg = NULL;
        return;
    }
    LZ77UnCompWram(gBattleAnimBgImage_Sandstorm, tiles);
    LZ77UnCompWram(gBattleAnimBgTilemap_Sandstorm, map);
    LZ77UnCompWram(gBattleAnimSpritePal_FlyingDirt, pal);
    memcpy(sW.sandPal, pal, sizeof(pal));
    for (ty = 0; ty < 32; ty++)
        for (tx = 0; tx < 32; tx++)
        {
            u16 e = map[ty * 32 + tx];
            u16 tile = e & 0x3FF;
            bool8 hf = (e >> 10) & 1, vf = (e >> 11) & 1;
            for (y = 0; y < 8; y++)
                for (x = 0; x < 8; x++)
                {
                    int sx = hf ? 7 - x : x, sy = vf ? 7 - y : y;
                    u8 b = tile < nTiles ? tiles[tile * 32 + sy * 4 + sx / 2] : 0;
                    sW.sandBg[(ty * 8 + y) * 256 + tx * 8 + x] = (sx & 1) ? (b >> 4) : (b & 0xF);
                }
        }
    free(tiles);
    free(map);
}

static void InitIfNeeded(void)
{
    if (sW.init)
        return;
    sW.init = TRUE;
    sW.rng = 0x5EA7;
}

// ---------------------------------------------------------------------------
// Drawing into the frame (BGR555, bit 15 = something drawn there)

static struct MpFadeFit sFit;

static u16 Shade(u16 c)
{
    return MP_FadeFit_Apply(&sFit, c & 0x7FFF) | 0x8000;
}

static u16 Mix(u16 under, u16 over, u8 alpha)   // alpha in sixteenths of `over`
{
    s32 r = (under & 0x1F) + ((((over & 0x1F) - (under & 0x1F)) * alpha) >> 4);
    s32 g = ((under >> 5) & 0x1F) + (((((over >> 5) & 0x1F) - ((under >> 5) & 0x1F)) * alpha) >> 4);
    s32 b = ((under >> 10) & 0x1F) + (((((over >> 10) & 0x1F) - ((under >> 10) & 0x1F)) * alpha) >> 4);
    return r | (g << 5) | (b << 10) | 0x8000;
}

// One frame of a 1D-mapped 4bpp sprite (w, h in pixels; firstTile its first tile), top-left at x, y; half size
// draws every other pixel (Hail's smaller stones)
static void Blit(u16 *frame, const struct Art *a, u16 firstTile, int w, int h, int x, int y, bool8 half, u8 alpha)
{
    int px, py, tw = w / 8, step = half ? 2 : 1;
    for (py = 0; py < h; py += step)
    {
        int sy = y + py / step;
        if (sy < 0 || sy >= SCENE_H)
            continue;
        for (px = 0; px < w; px += step)
        {
            int sx = x + px / step;
            u8 b, c;
            if (sx < 0 || sx >= DISPLAY_WIDTH)
                continue;
            b = a->tiles[(firstTile + (py / 8) * tw + px / 8) * 32 + (py % 8) * 4 + (px % 8) / 2];
            c = (px & 1) ? (b >> 4) : (b & 0xF);
            if (c == 0)
                continue;
            if (alpha >= 16)
                frame[sy * DISPLAY_WIDTH + sx] = Shade(a->pal[c]);
            else
                frame[sy * DISPLAY_WIDTH + sx] = Mix(frame[sy * DISPLAY_WIDTH + sx], Shade(a->pal[c]), alpha);
        }
    }
}

// ---------------------------------------------------------------------------
// Rain (AnimRainDrop: 7 frames of 16x32, 2-2-2-6-2-2-2 frames long; it falls 1 right and 4 down a frame for 13
// frames, then splashes)

static const u8 sDropFrameAt[18] = {0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5, 6, 6};

static void UpdateRain(u16 *frame, bool8 heavy)
{
    int i, spawns = heavy ? 2 : 1;
    for (i = 0; i < MAX_DROPS && spawns > 0; i++)
        if (!sW.drops[i].on)
        {
            sW.drops[i].on = TRUE;
            sW.drops[i].x = Rand() % (DISPLAY_WIDTH + 16) - 16;
            sW.drops[i].y = Rand() % (SCENE_H - 24) - 40;
            sW.drops[i].t = 0;
            spawns--;
        }
    for (i = 0; i < MAX_DROPS; i++)
    {
        struct Drop *d = &sW.drops[i];
        if (!d->on)
            continue;
        if (d->t < 13)
        {
            d->x += 1;
            d->y += 4;
        }
        Blit(frame, &sW.rain, sDropFrameAt[d->t] * 8, 16, 32, d->x - 8, d->y - 16, FALSE, 16);
        if (++d->t >= ARRAY_COUNT(sDropFrameAt))
            d->on = FALSE;
    }
}

// ---------------------------------------------------------------------------
// Hail (AnimHailBegin: 4 right and 8 down a frame to its spot, then the large ice crystal for a moment)

static void UpdateHail(u16 *frame)
{
    int i;
    if (sW.frame % 3 == 0)
        for (i = 0; i < MAX_STONES; i++)
            if (!sW.stones[i].on)
            {
                struct Stone *s = &sW.stones[i];
                s->on = TRUE;
                s->hit = FALSE;
                s->t = 0;
                s->size = Rand() % 3;               // the three sizes Hail uses (1, 15/16, 7/8: shown as 1 or 1/2)
                s->tx = Rand() % DISPLAY_WIDTH;
                s->ty = 24 + Rand() % (SCENE_H - 32);
                s->x = s->tx - (s->ty + 8) / 2;
                s->y = -8;
                break;
            }
    for (i = 0; i < MAX_STONES; i++)
    {
        struct Stone *s = &sW.stones[i];
        if (!s->on)
            continue;
        if (!s->hit)
        {
            s->x += 4;
            s->y += 8;
            if (s->x >= s->tx || s->y >= s->ty)
                s->hit = TRUE;
            Blit(frame, &sW.hail, 0, 16, 16, s->x - 8, s->y - 8, s->size == 2, 16);
        }
        else
        {
            // the large ice crystal (frame 4 of ICE_CRYSTALS, 8x16) where it lands, for 12 frames
            Blit(frame, &sW.crystal, 4, 8, 16, s->tx - 4, s->ty - 8, s->size == 2, 12);
            if (++s->t >= 12)
                s->on = FALSE;
        }
    }
}

// ---------------------------------------------------------------------------
// Sandstorm (the background scrolling 6 left and 1 up a frame, blended in; 64x16 sand crescents flying across at
// Sandstorm's speeds)

static void UpdateSandstorm(u16 *frame)
{
    int i;

    // the background itself is blended into the scene by ScenePixel (under the healthboxes); here it scrolls 6 left
    // and 1 up a frame, as in Sandstorm
    if (sW.sandBg)
    {
        for (i = 0; i < 16; i++)
            sW.sandShaded[i] = Shade(sW.sandPal[i]);
        sW.sandX -= 6;
        sW.sandY -= 1;
    }
    if (sW.frame % 10 == 0)
        for (i = 0; i < MAX_SAND; i++)
            if (!sW.sand[i].on)
            {
                static const s16 sSpeeds[] = {2304, 2048, 2560, 1984, 2816};
                sW.sand[i].on = TRUE;
                sW.sand[i].x256 = -64 * 256;
                sW.sand[i].y256 = (Rand() % 90) * 256;
                sW.sand[i].vx = sSpeeds[Rand() % ARRAY_COUNT(sSpeeds)];
                sW.sand[i].vy = 96;
                break;
            }
    for (i = 0; i < MAX_SAND; i++)
    {
        struct Sand *s = &sW.sand[i];
        if (!s->on)
            continue;
        s->x256 += s->vx;
        s->y256 += s->vy;
        if (s->x256 / 256 > DISPLAY_WIDTH + 32)
        {
            s->on = FALSE;
            continue;
        }
        // two 32x16 halves side by side (sFlyingSandSubsprites: tiles 0 and 8)
        Blit(frame, &sW.dirt, 0, 32, 16, s->x256 / 256 - 32, s->y256 / 256 - 8, FALSE, 16);
        Blit(frame, &sW.dirt, 8, 32, 16, s->x256 / 256, s->y256 / 256 - 8, FALSE, 16);
    }
}

// ---------------------------------------------------------------------------

static u16 Tint(u16 c, u16 to, u8 n)
{
    s32 r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    r += (((s32)(to & 0x1F) - r) * n) >> 4;
    g += (((s32)((to >> 5) & 0x1F) - g) * n) >> 4;
    b += (((s32)((to >> 10) & 0x1F) - b) * n) >> 4;
    return (c & 0x8000) | r | (g << 5) | (b << 10);
}

// Every pixel of the battle scene's layers (BG1-3; BG0 has the message box) and of the Pokémon's sprites: the
// sandstorm blended in as Sandstorm's BG1 is, and the weather's tint. The healthboxes aren't touched.
static u16 ScenePixel(u16 c, int x, int y, bool8 sprite)
{
    u8 n = sprite ? sW.tintMons : sW.tintBg;
    if (sW.sandOn && y < SCENE_H)
    {
        u8 k = sW.sandBg[((y - sW.sandY) & 0xFF) * 256 + ((x - sW.sandX) & 0xFF)];
        if (k)
            c = (c & 0x8000) | (Mix(c, sW.sandShaded[k], SAND_ALPHA) & 0x7FFF);
    }
    if (n)
        c = Tint(c, sW.tintColor, n);
    return c;
}

// What the renderer's hook does next frame. Tints are off while the screen fades (one toward white would show on a
// black screen); the sand fades with the scene (MP_FadeFit)
static void SetScene(u16 w)
{
    int b;
    u16 tw = (gPaletteFade.active || gPaletteFade.y != 0) ? 0 : w;

    sW.tintBg = sW.tintMons = 0;
    sW.tintColor = RGB_BLACK;
    if (tw & B_WEATHER_SUN)
        sW.tintMons = SUN_MONS, sW.tintBg = SUN_BG, sW.tintColor = WARM_WHITE;
    else if (tw & B_WEATHER_RAIN)
        sW.tintMons = RAIN_DIM, sW.tintBg = RAIN_DIM;
    else if (tw & B_WEATHER_HAIL)
        sW.tintBg = HAIL_DIM;
    sW.sandOn = (w & B_WEATHER_SANDSTORM) && sW.sandBg != NULL;
    memset(gMpSceneObjPal, 0, sizeof(gMpSceneObjPal));
    if (!sW.sandOn && !sW.tintBg && !sW.tintMons)
    {
        gMpScenePixel = NULL;
        return;
    }
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        u8 id = gBattlerSpriteIds[b];
        if (id < MAX_SPRITES && gSprites[id].inUse)
            gMpSceneObjPal[gSprites[id].oam.paletteNum & 63] = 1;
    }
    gMpSceneLayers = 0xE;
    gMpScenePixel = ScenePixel;
}

static void Clear(void)
{
    memset(sW.drops, 0, sizeof(sW.drops));
    memset(sW.stones, 0, sizeof(sW.stones));
    memset(sW.sand, 0, sizeof(sW.sand));
}

void MP_BattleWeather_Composite(u16 *frame)
{
    u16 w;

    InitIfNeeded();
    if (!gMain.inBattle || gMain.callback2 != BattleMainCB2 || gBattleResources == NULL)
    {
        gMpScenePixel = NULL;
        Clear();
        return;
    }
    w = gBattleWeather;
    if (w != sW.lastWeather)
    {
        printf("[mp] battle weather on screen:%s%s%s%s%s\n", (w & B_WEATHER_RAIN) ? " rain" : "", (w & B_WEATHER_SUN) ? " sun" : "",
               (w & B_WEATHER_SANDSTORM) ? " sandstorm" : "", (w & B_WEATHER_HAIL) ? " hail" : "", (w & B_WEATHER_ANY) ? "" : " none");
        sW.lastWeather = w;
    }
    sW.frame++;
    MP_FadeFit_FromPalette(&sFit, 0);   // fade as the battle scene does
    if (w & B_WEATHER_SANDSTORM)
        LoadSandBackground();
    if (w & B_WEATHER_RAIN)
    {
        LoadArt(&sW.rain, ANIM_TAG_RAIN_DROPS);
        if (sW.rain.loaded)
            UpdateRain(frame, (w & B_WEATHER_RAIN_DOWNPOUR) != 0);
    }
    else
    {
        memset(sW.drops, 0, sizeof(sW.drops));
    }
    if (w & B_WEATHER_HAIL)
    {
        LoadArt(&sW.hail, ANIM_TAG_HAIL);
        LoadArt(&sW.crystal, ANIM_TAG_ICE_CRYSTALS);
        if (sW.hail.loaded && sW.crystal.loaded)
            UpdateHail(frame);
    }
    else
    {
        memset(sW.stones, 0, sizeof(sW.stones));
    }
    if (w & B_WEATHER_SANDSTORM)
    {
        LoadArt(&sW.dirt, ANIM_TAG_FLYING_DIRT);
        if (sW.dirt.loaded)
            UpdateSandstorm(frame);
    }
    else
    {
        memset(sW.sand, 0, sizeof(sW.sand));
    }
    SetScene(w);
}
