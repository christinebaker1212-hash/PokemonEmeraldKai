// Moemon Kai: OPTIONS > DISPLAY > GB OPTIONS > COLORS (RED / CRYSTAL): the picture in the colours the Game Boy could
// show.
//
// RED: DMG (the first Game Boy's greens), POCKET (its greys), LIGHT (its backlight), COLOR (Red on a Game Boy Color:
// the colours its boot ROM gives Red, the background in reds and sprites in greens, a battle's POKeMON in the
// background's as Red draws them) or GBA (the same on a Game Boy Advance's screen); CRYSTAL: COLOR (the Game Boy
// Color's rules: four colours a palette, so four an 8x8 tile and a sprite, at most eight palettes for the background
// and eight for sprites, a battle's POKeMON in Crystal's own colours for it) or GBA. KAI turns it off.
//
// How: just before the frame is drawn (sdl2.c VDraw) every 16-colour palette in palette RAM is put into four shades
// (by its colours' brightness, from the palette as loaded, gPlttBufferUnfaded, so a fade steps through the shades as
// the Game Boy's did) and each shade given the mode's colour; RAM is put back once it's drawn. What isn't drawn from
// palette RAM (pictures in full colour, the windows UI STYLE draws over the frame) is put into the mode's colours by
// brightness (mono, RED's COLOR) or, in CRYSTAL, four colours to an 8x8 tile, after everything is drawn
// (MP_GbPal_Composite). It's over DISPLAY > COLOR (display.c gMpGbDisplayColor: none, or the GBA's screen for GBA)
// but not its filters.
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "main.h"
#include "palette.h"
#include "pokemon.h"
#include "sprite.h"
#include "mp_uistyle.h"
#include "constants/rgb.h"
#include <string.h>

#include "mp_gbpal_data.h"

enum { KIND_OFF, KIND_MONO, KIND_RED_GBC, KIND_CRYSTAL };

extern int gMpGbDisplayColor;   // display.c: -1 DISPLAY > COLOR as chosen, else the colour shown (0 DEFAULT, 1 GBA)

static const u16 sMono[3][4] = {
    {RGB(19, 23, 1), RGB(17, 21, 1), RGB(6, 12, 6), RGB(1, 7, 1)},       // DMG
    {RGB(24, 25, 20), RGB(17, 18, 13), RGB(9, 10, 7), RGB(3, 3, 3)},     // POCKET
    {RGB(0, 22, 16), RGB(0, 19, 14), RGB(0, 13, 9), RGB(0, 9, 7)},       // LIGHT
};
// The Game Boy Color's boot ROM palettes for POKeMON RED: the background (and OBJ1), OBJ0
static const u16 sRedGbcBg[4] = {RGB(31, 31, 31), RGB(31, 16, 16), RGB(18, 7, 7), RGB(0, 0, 0)};
static const u16 sRedGbcObj[4] = {RGB(31, 31, 31), RGB(15, 31, 6), RGB(0, 16, 0), RGB(0, 0, 0)};

#define NUM_PALS (16 + OBJ_PAL_SLOTS)
static u16 sSaved[PLTT_SIZE / 2];
static bool8 sSwapped;
static u8 sKind, sMonoIdx;
static bool8 sGba;
static u16 sLut[0x8000];            // mono / RED's COLOR: any colour to the mode's (by brightness)
static u8 sLutFor = 0xFF;
static u8 sAllowed[0x8000 / 8];     // CRYSTAL: the colours palette RAM showed this frame
static u16 sAllowedList[NUM_PALS * 4];
static u16 sAllowedCount;

// GB OPTIONS > COLORS: RED 0 DMG, 1 POCKET, 2 LIGHT, 3 COLOR, 4 GBA, 5 KAI; CRYSTAL 0 COLOR, 1 GBA, 2 KAI
u8 MP_GbPal_Setting(void)
{
    u8 i = gMpConfig.uiStyle == UISTYLE_CRYSTAL ? 1 : 0;
    return (gMpConfig.gbOptions[i] >> MP_GB_PAL_SHIFT) & MP_GB_PAL_MASK;
}

static void Choose(void)
{
    bool8 MP_TitleScreenShown(void);
    bool8 MP_IntroShown(void);
    u8 v = MP_GbPal_Setting();
    sKind = KIND_OFF;
    sGba = FALSE;
    if (MP_TitleScreenShown() || MP_IntroShown())
        return;   // (the title screen and the intro before it stay as they are: the user, 2026-11-18)
    if (MP_UiStyle_Active() == UISTYLE_RED)
    {
        if (v <= 2)
            sKind = KIND_MONO, sMonoIdx = v;
        else if (v <= 4)
            sKind = KIND_RED_GBC, sGba = v == 4;
    }
    else if (MP_UiStyle_Active() == UISTYLE_CRYSTAL && v <= 1)
        sKind = KIND_CRYSTAL, sGba = v == 1;
}

bool8 MP_GbPal_Active(void)
{
    Choose();
    return sKind != KIND_OFF;
}

// Brightness 0-31 (as the eye sees it)
static s32 Lum(u16 c)
{
    return ((c & 31) * 77 + ((c >> 5) & 31) * 151 + ((c >> 10) & 31) * 28) >> 8;
}

// ... and as a Game Boy shade: a strong colour darker than its brightness (Game Boy art draws an HP bar's fill or a
// fire's orange in its darker shades, not near white)
static s32 GbLum(u16 c)
{
    s32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    return (Lum(c) + min(r, min(g, b))) / 2;
}

static s32 Dist(u16 a, u16 b)
{
    s32 dr = (a & 31) - (b & 31), dg = ((a >> 5) & 31) - ((b >> 5) & 31), db = ((a >> 10) & 31) - ((b >> 10) & 31);
    return dr * dr * 2 + dg * dg * 4 + db * db;
}

// A palette's colours in four shades (0 lightest .. 3 darkest): by brightness, half as it is and half stretched over
// the palette's own range, so a sprite keeps its light and dark parts and a dark place stays dark
static void Shades(const u16 *pal, u8 first, u8 *out)
{
    s32 lo = 31, hi = 0, i, l, v;
    for (i = first; i < 16; i++)
    {
        l = GbLum(pal[i] & 0x7FFF);
        lo = min(lo, l), hi = max(hi, l);
    }
    for (i = 0; i < 16; i++)
    {
        l = GbLum(pal[i] & 0x7FFF);
        v = l * 256 / 31;
        if (hi - lo >= 4)
            v = (v + (l - lo) * 256 / (hi - lo)) / 2;
        out[i] = v >= 176 ? 0 : v >= 112 ? 1 : v >= 48 ? 2 : 3;
    }
}

// The fade on a colour (the palette RAM's colour against the palette as loaded), as the Game Boy's: steps of shade
static u8 Faded(u8 shade, u16 cur, u16 unf)
{
    s32 lc, lu, t;
    if ((cur & 0x7FFF) == (unf & 0x7FFF))
        return shade;
    lc = Lum(cur & 0x7FFF), lu = Lum(unf & 0x7FFF);
    if (lc < lu && lu > 0)        // toward black
    {
        t = (lu - lc) * 256 / lu;
        return shade + ((3 - shade) * t + 128) / 256;
    }
    if (lc > lu && lu < 31)       // toward white
    {
        t = (lc - lu) * 256 / (31 - lu);
        return shade - (shade * t + 128) / 256;
    }
    return shade;
}

// In a battle: the sprite palette of each POKeMON's picture and its species / shininess (Crystal's own colours; in
// RED's COLOR the background's, as Red draws them on the background)
static u16 sMonPalSpecies[OBJ_PAL_SLOTS];
static bool8 sMonPalShiny[OBJ_PAL_SLOTS];
static void FindBattleMons(void)
{
    u32 b;
    memset(sMonPalSpecies, 0, sizeof(sMonPalSpecies));
    if (!gMain.inBattle || gBattleSpritesDataPtr == NULL)
        return;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        u8 id = gBattlerSpriteIds[b];
        u16 species;
        if (id >= MAX_SPRITES || !gSprites[id].inUse || gSprites[id].oam.paletteNum >= OBJ_PAL_SLOTS)
            continue;
        species = gBattleSpritesDataPtr->battlerData[b].transformSpecies;
        if (species == SPECIES_NONE)
            species = gBattleMons[b].species;
        sMonPalSpecies[gSprites[id].oam.paletteNum] = species ? species : 0xFFFF;
        sMonPalShiny[gSprites[id].oam.paletteNum] = IsShinyOtIdPersonality(gBattleMons[b].otId, gBattleMons[b].personality);
    }
}

// CRYSTAL: four colours for a palette from its own, by how much of the screen each colour covers (WEIGHT): the four
// groups of nearest colours (k-means from the palette's light-to-dark quarters), each drawn in the palette's own colour
// nearest the group's middle, light to dark; SHADE says each colour's
static void FourOf(const u16 *pal, const u32 *weight, u8 first, u16 *four, u8 *shade)
{
    s32 cen[4][3], i, k, it, n = 0, best, d;
    u8 order[16];
    u16 c;
    for (i = first; i < 16; i++)
        order[n++] = i;
    for (i = 1; i < n; i++)   // light to dark
        for (k = i; k > 0 && Lum(pal[order[k]] & 0x7FFF) > Lum(pal[order[k - 1]] & 0x7FFF); k--)
        {
            u8 t = order[k];
            order[k] = order[k - 1], order[k - 1] = t;
        }
    for (k = 0; k < 4; k++)
    {
        c = pal[order[min(n - 1, (2 * k + 1) * n / 8)]] & 0x7FFF;
        cen[k][0] = (c & 31) << 8, cen[k][1] = ((c >> 5) & 31) << 8, cen[k][2] = ((c >> 10) & 31) << 8;
    }
    for (it = 0; it < 6; it++)
    {
        s64 sum[4][3] = {0}, wsum[4] = {0};
        for (i = first; i < 16; i++)
        {
            s32 r, g, b, w = weight[i] + 1;
            c = pal[i] & 0x7FFF, r = (c & 31) << 8, g = ((c >> 5) & 31) << 8, b = ((c >> 10) & 31) << 8;
            best = 0x7FFFFFFF;
            for (k = 0; k < 4; k++)
            {
                s32 dr = (r - cen[k][0]) >> 4, dg = (g - cen[k][1]) >> 4, db = (b - cen[k][2]) >> 4;
                d = dr * dr * 2 + dg * dg * 4 + db * db;
                if (d < best)
                    best = d, shade[i] = k;
            }
            k = shade[i];
            sum[k][0] += (s64)r * w, sum[k][1] += (s64)g * w, sum[k][2] += (s64)b * w, wsum[k] += w;
        }
        for (k = 0; k < 4; k++)
            if (wsum[k])
                cen[k][0] = sum[k][0] / wsum[k], cen[k][1] = sum[k][1] / wsum[k], cen[k][2] = sum[k][2] / wsum[k];
    }
    // each group's colour: the palette's own nearest its middle
    for (k = 0; k < 4; k++)
    {
        u16 mid = RGB(cen[k][0] >> 8, cen[k][1] >> 8, cen[k][2] >> 8);
        four[k] = mid, best = 0x7FFFFFFF;
        for (i = first; i < 16; i++)
            if (shade[i] == k && Dist(pal[i] & 0x7FFF, mid) < best)
                best = Dist(pal[i] & 0x7FFF, mid), four[k] = pal[i] & 0x7FFF;
    }
    // light to dark, the shades following (a fade steps through them)
    {
        u8 rank[4] = {0, 1, 2, 3}, inv[4], j;
        for (i = 1; i < 4; i++)
            for (j = i; j > 0 && Lum(four[rank[j]]) > Lum(four[rank[j - 1]]); j--)
            {
                u8 t = rank[j];
                rank[j] = rank[j - 1], rank[j - 1] = t;
            }
        {
            u16 f[4];
            for (i = 0; i < 4; i++)
                f[i] = four[rank[i]], inv[rank[i]] = i;
            memcpy(four, f, sizeof(f));
        }
        for (i = 0; i < 16; i++)
            shade[i] = i < first ? 0 : inv[shade[i] & 3];
    }
}

// The palettes the screen shows now (the background's by its tile maps, the sprites' by OAM): CRYSTAL keeps eight of
// each
static bool8 sGbPalTrace;
static u32 sWeight[NUM_PALS][16];   // CRYSTAL: the pixels of each colour the background shows (its tiles')
static u8 sBgsOf[16];               // the background layers each background palette shows on
static void UsedPalettes(u16 *used)
{
    u32 bg, i;
    u16 dispcnt = REG_DISPCNT;
    memset(used, 0, NUM_PALS * sizeof(*used));
    memset(sWeight, 0, sizeof(sWeight));
    memset(sBgsOf, 0, sizeof(sBgsOf));
    for (bg = 0; bg < 4; bg++)
    {
        u16 cnt, hofs, vofs, size;
        u32 tx, ty, sbb;
        if (!(dispcnt & (DISPCNT_BG0_ON << bg)))
            continue;
        cnt = *(u16 *)(REG_ADDR_BG0CNT + bg * 2);
        if ((cnt & 0x80) || ((dispcnt & 7) == 1 && bg == 2) || ((dispcnt & 7) == 2 && bg >= 2))
        {
            for (i = 0; i < 16; i++)
                used[i] += 1, sBgsOf[i] |= 1 << bg;   // (256 colours or turned: every palette)
            continue;
        }
        hofs = *(u16 *)(REG_ADDR_BG0HOFS + bg * 4) & 0x1FF;
        vofs = *(u16 *)(REG_ADDR_BG0VOFS + bg * 4) & 0x1FF;
        sbb = (cnt >> 8) & 31, size = cnt >> 14;
        for (ty = 0; ty < 21; ty++)
            for (tx = 0; tx < 31; tx++)
            {
                u32 mx = ((hofs >> 3) + tx) & ((size & 1) ? 63 : 31), my = ((vofs >> 3) + ty) & ((size & 2) ? 63 : 31);
                u32 block = sbb + (mx >> 5) + ((my >> 5) * ((size & 1) ? 2 : 1));
                u16 e = ((u16 *)(VRAM + block * 0x800))[(my & 31) * 32 + (mx & 31)];
                const u8 *tile = (const u8 *)(VRAM + ((cnt >> 2) & 3) * 0x4000 + (e & 0x3FF) * 32);
                u32 *w = sWeight[e >> 12], k;
                used[e >> 12]++;
                sBgsOf[e >> 12] |= 1 << bg;
                for (k = 0; k < 32; k++)   // (colour 0 is see-through)
                    w[tile[k] & 15]++, w[tile[k] >> 4]++;
                w[0] = 0;
            }
    }
    used[0]++;   // (the backdrop)
    if (dispcnt & DISPCNT_OBJ_ON)
        for (i = 0; i < OAM_ENTRY_COUNT; i++)
        {
            const struct OamData *o = &((const struct OamData *)OAM)[i];
            if (o->affineMode == ST_OAM_AFFINE_ERASE || o->paletteNum >= OBJ_PAL_SLOTS)
                continue;
            if (o->bpp)
            {
                u32 k;
                for (k = 16; k < 32; k++)
                    used[k]++;
            }
            else
                used[16 + o->paletteNum] += 4;   // (a sprite as much as four tiles)
        }
}

// How far palette P's own colours would move drawn with another's four (its tiles taking that palette)
static s32 MergeCost(const u16 *pal, const u16 *four)
{
    s32 i, k, d, best, sum = 0;
    for (i = 1; i < 16; i++)
    {
        best = 0x7FFFFFFF;
        for (k = 0; k < 4; k++)
            if ((d = Dist(pal[i] & 0x7FFF, four[k])) < best)
                best = d;
        sum += best;
    }
    return sum;
}

// Merge palettes until eight are left (from FIRST to LAST: the background's or the sprites'), each time the one
// whose tiles would change least drawn with another's four colours
static void KeepEight(u16 (*four)[4], const u16 *used, u32 first, u32 last, const u16 *pals)
{
    u8 rep[NUM_PALS];
    u32 i, j, n = 0;
    for (i = first; i < last; i++)
        rep[i] = i, n += used[i] != 0;
    while (n > 8)
    {
        s32 best = 0x7FFFFFFF;
        u32 bi = 0, bj = 0;
        for (i = first; i < last; i++)
            if (used[i] && rep[i] == i)
                for (j = first; j < last; j++)
                    if (used[j] && rep[j] == j && j != i)
                    {
                        // (weighted by how much of the screen the palette that changes covers)
                        s32 ci = MergeCost(pals + j * 16, four[i]) * min(used[j], 600) / 8,
                            cj = MergeCost(pals + i * 16, four[j]) * min(used[i], 600) / 8;
                        if (ci < best)
                            best = ci, bi = i, bj = j;   // (j drawn with i's)
                        if (cj < best)
                            best = cj, bi = j, bj = i;
                    }
        if (best == 0x7FFFFFFF)
            break;
        if (sGbPalTrace)
            MPNet_Log("gbpal merge %u <- %u cost %d (used %u)", bi, bj, best, used[bj]);
        for (i = first; i < last; i++)   // (j's tiles take i's colours)
            if (rep[i] == bj)
                rep[i] = bi;
        n--;
    }
    for (i = first; i < last; i++)
        if (used[i] && rep[i] != i)
            memcpy(four[i], four[rep[i]], sizeof(four[i]));
}

static void AllowOne(u16 c)
{
    c &= 0x7FFF;
    if (!(sAllowed[c >> 3] & (1 << (c & 7))))
    {
        sAllowed[c >> 3] |= 1 << (c & 7);
        if (sAllowedCount < ARRAY_COUNT(sAllowedList))
            sAllowedList[sAllowedCount++] = c;
    }
}

// CRYSTAL's light of the time of day over a battle (WHITE BG ON: mp_uistyle.c TintBattleScene, after palette RAM is
// drawn): each colour palette RAM gives is allowed as that light leaves it too, or every 8x8 cell where a POKeMON meets
// the background would be cut to four colours of its own (blocks)
static bool8 sTintOn;
static s32 sTintR, sTintG, sTintB;
static void Allow(u16 c)
{
    extern u16 (*gMpScenePixel)(u16 color, int x, int y, bool8 sprite);
    AllowOne(c);
    if (gMpScenePixel != NULL)   // (the time of day's light, battle weather's tints: drawn into the layers)
    {
        AllowOne(gMpScenePixel(c, 0, 0, FALSE));
        AllowOne(gMpScenePixel(c, 0, 0, TRUE));
    }
    if (sTintOn)
        AllowOne(((c & 31) * sTintR / 31) | ((((c >> 5) & 31) * sTintG / 31) << 5)
                 | ((((c >> 10) & 31) * sTintB / 31) << 10));
}

static void TintNow(void)
{
    bool8 MP_BattlePic_SceneTint(u16 *tint);
    u16 t;
    sTintOn = gMain.inBattle && MP_BattlePic_SceneTint(&t);
    if (sTintOn)
        sTintR = t & 31, sTintG = (t >> 5) & 31, sTintB = (t >> 10) & 31;
}

// GB OPTIONS > MOVES (mp_gb_moves.c): a colour it draws over the frame, as a sprite's of the Game Boy's own (CRYSTAL's
// palettes: its sprites have their own palettes, not the background cell's)
void MP_GbPal_AllowColor(u16 c)
{
    if (sKind == KIND_CRYSTAL)
        AllowOne(c);
}

// GB OPTIONS > MOVES (mp_gb_moves.c): a Game Boy sprite's shade (OBP 0 / 1, 2 the background's) in the colours shown
u16 MP_GbPal_ObjShade(u8 shade, u8 obp)
{
    static const u16 sGrey[4] = {RGB(31, 31, 31), RGB(21, 21, 21), RGB(10, 10, 10), RGB(0, 0, 0)};
    Choose();
    shade &= 3;
    if (sKind == KIND_MONO)
        return sMono[sMonoIdx][shade];
    if (sKind == KIND_RED_GBC)
        return obp == 0 ? sRedGbcObj[shade] : sRedGbcBg[shade];
    return sGrey[shade];
}

// RED's modes in a battle: a full-colour background picture (SINNOH's / UNOVA's backgrounds, WHITE BG's plain one)
// drawn in the mode's lightest shade, Red's own battle background (the user, 2026-11-18: DMG was unreadable; "the
// battle background should be the same color as the background in Red")
extern void (*gMpBgPictureLine)(int line, uint16_t *layer, int32_t x0, int32_t y0, int32_t dx, int32_t dy);
static void (*sPicLine)(int line, uint16_t *layer, int32_t x0, int32_t y0, int32_t dx, int32_t dy);
static void LightPicLine(int line, uint16_t *layer, int32_t x0, int32_t y0, int32_t dx, int32_t dy)
{
    const u16 *four = sKind == KIND_MONO ? sMono[sMonoIdx] : sRedGbcBg;
    u8 drawn[240];
    s32 i;
    for (i = 0; i < 240; i++)
        drawn[i] = (layer[i] & 0x8000) != 0;   // (the picture's pixels: where the layer drew, as the picture reads it)
    sPicLine(line, layer, x0, y0, dx, dy);
    for (i = 0; i < 240; i++)
        if (drawn[i])
            layer[i] = (layer[i] & 0x8000) | four[0];
}

static void BattleBackdropLight(void)
{
    if (sKind != KIND_CRYSTAL && gMain.inBattle && gMpBgPictureLine != NULL && gMpBgPictureLine != LightPicLine)
    {
        sPicLine = gMpBgPictureLine;
        gMpBgPictureLine = LightPicLine;
    }
}

// sdl2.c VDraw, before DrawFrame: palette RAM in the Game Boy's colours
void MP_GbPal_PreDraw(void)
{
    static u16 four[NUM_PALS][4];
    static u16 used[NUM_PALS];
    u16 *pltt = (u16 *)PLTT;
    u32 p, i;
    u8 shade[16];

    Choose();
    gMpGbDisplayColor = sKind == KIND_OFF ? -1 : sGba ? 1 : 0;
    if (sKind == KIND_OFF)
        return;
    memcpy(sSaved, pltt, sizeof(sSaved));
    sSwapped = TRUE;
    FindBattleMons();
    if (sKind == KIND_CRYSTAL)
    {
        for (i = 0; i < sAllowedCount; i++)
            sAllowed[sAllowedList[i] >> 3] = 0;
        sAllowedCount = 0;
        TintNow();
    }
    if (sKind == KIND_CRYSTAL || gMain.inBattle)
        UsedPalettes(used);
    BattleBackdropLight();
    for (p = 0; p < NUM_PALS; p++)
    {
        const u16 *unf = gPlttBufferUnfaded + p * 16, *cur = sSaved + p * 16;
        bool8 obj = p >= 16;
        u8 first = (obj || p != 0) ? 1 : 0;   // (a sprite's colour 0 is clear; the background's only shows in palette 0)
        u16 species = obj ? sMonPalSpecies[p - 16] : 0;
        bool8 blank = TRUE;
        for (i = 0; i < 16 && blank; i++)
            blank = unf[i] == 0;
        Shades(blank ? cur : unf, first, shade);
        if (sKind == KIND_MONO)
            memcpy(four[p], sMono[sMonoIdx], sizeof(four[p]));
        else if (sKind == KIND_RED_GBC)
            memcpy(four[p], obj && species == 0 ? sRedGbcObj : sRedGbcBg, sizeof(four[p]));
        else if (species != 0 && species != 0xFFFF && species <= 251)
        {
            u8 sh = sMonPalShiny[p - 16];
            four[p][0] = RGB(31, 31, 31), four[p][3] = RGB(0, 0, 0);
            four[p][1] = sGbCrystalMonPals[species][sh][0], four[p][2] = sGbCrystalMonPals[species][sh][1];
        }
        else
        {
            static const u32 sEven[16] = {0};
            FourOf(blank ? cur : unf, p < 16 ? sWeight[p] : sEven, first, four[p], shade);
        }
        if (sKind != KIND_CRYSTAL && gMain.inBattle && p < 16 && sBgsOf[p] == (1 << 3))
            for (i = 0; i < 16; i++)
                shade[i] = 0;   // (the battle's background: the lightest shade, as Red's own battles are on white)
        if (sKind != KIND_CRYSTAL && gMain.inBattle && p == 0)
            shade[0] = 0;   // (... and the backdrop the lightest)
        for (i = 0; i < 16; i++)
            shade[i] = Faded(shade[i], cur[i], blank ? cur[i] : unf[i]);
        for (i = 0; i < 16; i++)
            pltt[p * 16 + i] = shade[i];   // (the shade for now: the colour once the palettes are merged)
    }
    if (sKind == KIND_CRYSTAL)
    {
        {
            char *getenv(const char *name);
            static u32 n;
            sGbPalTrace = getenv("MOEMON_GBPAL_TRACE") && (++n % 120) == 0;
            if (sGbPalTrace)
                for (p = 0; p < 16; p++)
                    MPNet_Log("gbpal bg %u used %u four %04x %04x %04x %04x", p, used[p], four[p][0], four[p][1], four[p][2], four[p][3]);
        }
        KeepEight(four, used, 0, 16, sSaved);
        KeepEight(four, used, 16, NUM_PALS, sSaved);
    }
    for (p = 0; p < NUM_PALS; p++)
        for (i = 0; i < 16; i++)
        {
            u16 c = four[p][pltt[p * 16 + i] & 3];
            pltt[p * 16 + i] = c;
            if (sKind == KIND_CRYSTAL && used[p] && (p < 16 || i != 0))
                Allow(c);
        }
}

// ... and after it: palette RAM as the game left it
void MP_GbPal_PostDraw(void)
{
    if (gMpBgPictureLine == LightPicLine)
        gMpBgPictureLine = sPicLine;
    if (sSwapped)
        memcpy((u16 *)PLTT, sSaved, sizeof(sSaved));
    sSwapped = FALSE;
}

static void BuildLut(void)
{
    u8 want = sKind == KIND_MONO ? sMonoIdx : 3;
    const u16 *four = sKind == KIND_MONO ? sMono[sMonoIdx] : sRedGbcBg;
    u32 c, k;
    if (sLutFor == want)
        return;
    for (c = 0; c < 0x8000; c++)
    {
        s32 l = GbLum(c), best = 13;
        sLut[c] = four[l >= 22 ? 0 : l >= 13 ? 1 : l >= 5 ? 2 : 3];
        // (the mode's own colours, and any the renderer took a step from them (a blend), stay as they are)
        for (k = 0; k < 4; k++)
        {
            if (Dist(c, four[k]) < best)
                best = Dist(c, four[k]), sLut[c] = four[k];
            if (sKind == KIND_RED_GBC && Dist(c, sRedGbcObj[k]) < best)
                best = Dist(c, sRedGbcObj[k]), sLut[c] = sRedGbcObj[k];
        }
    }
    sLutFor = want;
}

// CRYSTAL: a colour palette RAM didn't give (a blend, a full-colour picture, an effect drawn over the frame) becomes
// the nearest of the colours it did give this frame, pixel by pixel. Never by 8x8 cells: a cell's four colours of its
// own drew blocks round every sprite, tint and effect (the user, 2026-11-18: "I don't want to see those palette
// blocks anywhere ever again")
static u16 sNear[0x8000];
static u16 sNearGen[0x8000];
static u16 sGen;
static u16 Nearest(u16 c)
{
    u32 i;
    s32 best = 0x7FFFFFFF;
    u16 out = c;
    if (sNearGen[c] == sGen)
        return sNear[c];
    for (i = 0; i < sAllowedCount; i++)
    {
        s32 d = Dist(c, sAllowedList[i]);
        if (d < best)
            best = d, out = sAllowedList[i];
    }
    sNear[c] = out, sNearGen[c] = sGen;
    return out;
}

// sdl2.c VDraw, after everything is drawn over the frame
void MP_GbPal_Composite(u16 *frame)
{
    s32 i;
    if (sKind == KIND_OFF)
        return;
    if (sKind == KIND_CRYSTAL)
    {
        if (sAllowedCount == 0)
            return;
        if (++sGen == 0)
        {
            memset(sNearGen, 0, sizeof(sNearGen));
            sGen = 1;
        }
        for (i = 0; i < 240 * 160; i++)
        {
            u16 c = frame[i] & 0x7FFF;
            if (!(sAllowed[c >> 3] & (1 << (c & 7))))
                frame[i] = Nearest(c) | (frame[i] & 0x8000);
        }
        return;
    }
    BuildLut();
    for (i = 0; i < 240 * 160; i++)
        frame[i] = sLut[frame[i] & 0x7FFF] | (frame[i] & 0x8000);   // (the frame's top bit kept)
}
