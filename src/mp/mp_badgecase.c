// Moemon Co-op: the BADGE CASE (SELECT on the TRAINER CARD): every region's eight badges in a case modeled on
// Platinum's, drawn in a 480x320 picture of its own like a VS fight's (display.c), so the badges' artwork shows at
// twice the GBA's detail (graphics/moemon/badges/badges.bin: tools/moemon_badges/build_badges.py). The game waits
// while it's open (sdl2.c ScreenStep). L / R: the region; A: a badge chimes (higher the cleaner it is) and comes up
// close, where A chimes it again and holding A while circling the stick polishes it (every 30 degrees of the turn a
// rub, Platinum's rubs a point: mp_tcard.c MP_TCard_Rub; the D-pad's directions one after another or the mouse
// circling the badge do too), the controller rumbling with it. Badges dull day by day (OPTIONS > GAME > BADGE
// FADING) and show it: dirty, filthy, or 2 / 4 sparkles when well polished; a clean one spins up close.
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "sound.h"
#include "constants/songs.h"
#include "data.h"
#include "m4a.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_tcard.h"
#include "../tcg/tcg_gfx.h"

#define BC_W 480
#define BC_H 320
#define NUM_BADGES_ALL (REGION_COUNT * TCARD_BADGES_PER_REGION)

extern const u8 gMpBadgeData[];
bool8 Platform_PoketchMouse(s32 *x, s32 *y, bool8 *down);

static const char *const sRegionNames[REGION_COUNT] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
// the tabs left to right: the regions in the order they're travelled
static const u8 sTabRegion[REGION_COUNT] = {REGION_HOENN, REGION_KANTO, REGION_JOHTO, REGION_SINNOH};

struct BadgeInfo
{
    const char *name, *leader, *town;
};

static const struct BadgeInfo sBadges[REGION_COUNT][TCARD_BADGES_PER_REGION] = {
    [REGION_HOENN] = {
        {"STONE BADGE", "ROXANNE", "RUSTBORO CITY"},
        {"KNUCKLE BADGE", "BRAWLY", "DEWFORD TOWN"},
        {"DYNAMO BADGE", "WATTSON", "MAUVILLE CITY"},
        {"HEAT BADGE", "FLANNERY", "LAVARIDGE TOWN"},
        {"BALANCE BADGE", "NORMAN", "PETALBURG CITY"},
        {"FEATHER BADGE", "WINONA", "FORTREE CITY"},
        {"MIND BADGE", "TATE & LIZA", "MOSSDEEP CITY"},
        {"RAIN BADGE", "JUAN", "SOOTOPOLIS CITY"},
    },
    [REGION_JOHTO] = {
        {"ZEPHYR BADGE", "FALKNER", "VIOLET CITY"},
        {"HIVE BADGE", "BUGSY", "AZALEA TOWN"},
        {"PLAIN BADGE", "WHITNEY", "GOLDENROD CITY"},
        {"FOG BADGE", "MORTY", "ECRUTEAK CITY"},
        {"STORM BADGE", "CHUCK", "CIANWOOD CITY"},
        {"MINERAL BADGE", "JASMINE", "OLIVINE CITY"},
        {"GLACIER BADGE", "PRYCE", "MAHOGANY TOWN"},
        {"RISING BADGE", "CLAIR", "BLACKTHORN CITY"},
    },
    [REGION_KANTO] = {
        {"BOULDER BADGE", "BROCK", "PEWTER CITY"},
        {"CASCADE BADGE", "MISTY", "CERULEAN CITY"},
        {"THUNDER BADGE", "LT. SURGE", "VERMILION CITY"},
        {"RAINBOW BADGE", "ERIKA", "CELADON CITY"},
        {"SOUL BADGE", "KOGA", "FUCHSIA CITY"},
        {"MARSH BADGE", "SABRINA", "SAFFRON CITY"},
        {"VOLCANO BADGE", "BLAINE", "CINNABAR ISLAND"},
        {"EARTH BADGE", "GIOVANNI", "VIRIDIAN CITY"},
    },
    [REGION_SINNOH] = {
        {"COAL BADGE", "ROARK", "OREBURGH CITY"},
        {"FOREST BADGE", "GARDENIA", "ETERNA CITY"},
        {"COBBLE BADGE", "MAYLENE", "VEILSTONE CITY"},
        {"FEN BADGE", "CRASHER WAKE", "PASTORIA CITY"},
        {"RELIC BADGE", "FANTINA", "HEARTHOME CITY"},
        {"MINE BADGE", "BYRON", "CANALAVE CITY"},
        {"ICICLE BADGE", "CANDICE", "SNOWPOINT CITY"},
        {"BEACON BADGE", "VOLKNER", "SUNYSHORE CITY"},
    },
};

void Gamepad_Sticks(float *lx, float *ly, float *rx, float *ry);
void Gamepad_Rumble(int strength, int ms);

// the case's colors (Platinum's case_platinum palette)
#define C_CASE      0xf0e8a8
#define C_CASE_DARK 0xc8bc78
#define C_CASE_EDGE 0x988c50
#define C_TRAY      0x707078
#define C_TRAY_DARK 0x585860
#define C_SLOT      0x4c4c54
#define C_SLOT_RIM  0x8c8c94
#define C_PANEL     0xf8f0d0
#define C_INK       0x383838
#define C_INK_SOFT  0x787060

static struct
{
    bool8 active;
    bool8 draining;                     // closed: the game waits until the buttons are let go (B mustn't close the card)
    u8 tab, cursor;
    bool8 closeup;
    u32 age;
    u16 prevKeys;
    s8 rubDir;                          // the last rub's direction (-1 / 1), 0 at the start
    s32 mouseX, mouseY;
    s8 mouseDirX, mouseDirY;
    bool8 mouseDown;
    u8 glint[NUM_BADGES_ALL];           // frames of the light passing over a badge that just shone up
    float angle, turned;                // polishing: the stick's (or mouse's) last angle, how far it turned since a rub
    bool8 angleOk;
    s8 lastDpad;                        // the D-pad's last direction while polishing (0-3, -1 none)
    u16 spin;                           // up close: frames of the badge's spin (0 none)
    u16 idle;                           // up close: frames since it last spun
    SDL_Renderer *owner;
    SDL_Texture *canvas;
    SDL_Texture *small[NUM_BADGES_ALL], *big[NUM_BADGES_ALL];
    SDL_Texture *grimeSmall[NUM_BADGES_ALL], *grimeBig[NUM_BADGES_ALL];
    SDL_Texture *shape[NUM_BADGES_ALL];   // a badge's shape in white (not won, on the TRAINER CARD's dark panels)
    SDL_Texture *slot, *well;           // the hollows the badges sit in (the tray's, the close-up's)
} B;

static const struct MpCardView *sView;
static void MP_BadgeCase_FrameDrawn(void);   // whose case: NULL = the player's own (live records, polishing)

static u8 Region(void) { return sTabRegion[B.tab]; }
static bool8 Has(u8 region, u8 badge)
{
    return sView ? MP_CardView_HasBadge(sView, region, badge) : MP_TCard_HasBadge(region, badge);
}
static u8 PolishOf(const struct MpCardView *v, u8 region, u8 badge)
{
    return v ? v->polish[region * TCARD_BADGES_PER_REGION + badge] : MP_TCard_Badge(region, badge)->polish;
}
static u8 Index(u8 region, u8 badge) { return region * TCARD_BADGES_PER_REGION + badge; }

// ----------------------------------------------------------------------------------------------------- textures

static void DestroyTextures(void)
{
    u8 i;
    for (i = 0; i < NUM_BADGES_ALL; i++)
    {
        if (B.small[i]) SDL_DestroyTexture(B.small[i]);
        if (B.big[i]) SDL_DestroyTexture(B.big[i]);
        if (B.grimeSmall[i]) SDL_DestroyTexture(B.grimeSmall[i]);
        if (B.grimeBig[i]) SDL_DestroyTexture(B.grimeBig[i]);
        if (B.shape[i]) SDL_DestroyTexture(B.shape[i]);
        B.small[i] = B.big[i] = B.grimeSmall[i] = B.grimeBig[i] = B.shape[i] = NULL;
    }
    if (B.slot) SDL_DestroyTexture(B.slot);
    if (B.well) SDL_DestroyTexture(B.well);
    B.slot = B.well = NULL;
    if (B.canvas)
        SDL_DestroyTexture(B.canvas);
    B.canvas = NULL;
}

static SDL_Texture *MakeTexture(SDL_Renderer *r, const u32 *px, int size)
{
    SDL_Texture *t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, size, size);
    if (!t)
        return NULL;
    SDL_UpdateTexture(t, NULL, px, size * 4);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(t, SDL_ScaleModeLinear);
    return t;
}

// a smooth noise in 0-1 (value noise on an 8-pixel grid, two octaves), the same every time
static u32 Hash(u32 x, u32 y, u32 seed)
{
    u32 h = x * 374761393u + y * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
static float Noise(float x, float y, u32 seed)
{
    int x0 = (int)x, y0 = (int)y;
    float fx = x - x0, fy = y - y0;
    float a = (Hash(x0, y0, seed) & 1023) / 1023.0f, b = (Hash(x0 + 1, y0, seed) & 1023) / 1023.0f;
    float c = (Hash(x0, y0 + 1, seed) & 1023) / 1023.0f, d = (Hash(x0 + 1, y0 + 1, seed) & 1023) / 1023.0f;
    fx = fx * fx * (3 - 2 * fx);
    fy = fy * fy * (3 - 2 * fy);
    return (a + (b - a) * fx) + ((c + (d - c) * fx) - (a + (b - a) * fx)) * fy;
}

// The grime over a badge: brown smudges and specks where the badge is (scaled with the badge: the same look at both
// sizes), drawn over it more strongly the dirtier it is
static SDL_Texture *MakeGrime(SDL_Renderer *r, const u32 *badge, int size, u32 seed)
{
    static u32 px[128 * 128];
    int x, y;
    float k = 64.0f / size;   // (the noise in the small size's pixels)
    for (y = 0; y < size; y++)
        for (x = 0; x < size; x++)
        {
            u32 a = badge[y * size + x] >> 24;
            float n = 0.65f * Noise(x * k / 7.0f, y * k / 7.0f, seed) + 0.35f * Noise(x * k / 2.5f, y * k / 2.5f, seed + 7);
            float speck = (Hash((u32)(x * k), (u32)(y * k), seed + 99) % 97) == 0 ? 0.9f : 0;
            float g = (n - 0.38f) * 2.2f;
            u32 ga, shade;
            if (g < 0) g = 0;
            if (g > 1) g = 1;
            if (speck > g) g = speck;
            ga = (u32)(a * g * 0.85f);
            shade = speck > 0 ? 0x2a2014 : 0x5a4a30;
            px[y * size + x] = (ga << 24) | shade;
        }
    return MakeTexture(r, px, size);
}

// A round hollow pressed into the velvet: smooth-edged, shaded darker at the top (its own shadow) and lit along
// the bottom rim
static SDL_Texture *MakeHollow(SDL_Renderer *r, int size, u32 base)
{
    static u32 px[160 * 160];
    float c = (size - 1) / 2.0f, rad = size / 2.0f - 1;
    int x, y;
    for (y = 0; y < size; y++)
        for (x = 0; x < size; x++)
        {
            float dx = x - c, dy = y - c, d = SDL_sqrtf(dx * dx + dy * dy);
            float a = rad + 0.5f - d, t = (dy / rad + 1) / 2, k;
            u32 rr, gg, bb, al;
            if (a <= 0)
            {
                px[y * size + x] = 0;
                continue;
            }
            if (a > 1)
                a = 1;
            k = 0.62f + 0.38f * t;                    // darker at the top
            if (d > rad - 3 && dy > 0)                 // the lit lower rim
                k += 0.45f * (dy / rad) * (1 - (rad - d) / 3);
            if (d > rad - 2 && dy < 0)                 // the dark upper lip
                k -= 0.2f * (-dy / rad);
            rr = (u32)G_MinF(255, ((base >> 16) & 255) * k);
            gg = (u32)G_MinF(255, ((base >> 8) & 255) * k);
            bb = (u32)G_MinF(255, (base & 255) * k);
            al = (u32)(255 * a);
            px[y * size + x] = (al << 24) | (rr << 16) | (gg << 8) | bb;
        }
    return MakeTexture(r, px, size);
}

static void MakeTextures(SDL_Renderer *r)
{
    u16 caseSize, bigSize;
    const u8 *p = gMpBadgeData;
    u8 i;
    B.slot = MakeHollow(r, 76, 0x5c5c66);
    B.well = MakeHollow(r, 152, 0x70707a);
    if (memcmp(p, "MBDG", 4) != 0)
        return;
    caseSize = p[8] | (p[9] << 8);
    bigSize = p[10] | (p[11] << 8);
    p += 12;
    for (i = 0; i < NUM_BADGES_ALL; i++)
    {
        const u32 *s = (const u32 *)p, *b = (const u32 *)(p + caseSize * caseSize * 4);
        B.small[i] = MakeTexture(r, s, caseSize);
        if (caseSize <= 128)
        {
            static u32 white[128 * 128];
            int k;
            for (k = 0; k < caseSize * caseSize; k++)
                white[k] = (s[k] & 0xFF000000) | 0xFFFFFF;
            B.shape[i] = MakeTexture(r, white, caseSize);
        }
        B.big[i] = MakeTexture(r, b, bigSize);
        if (caseSize <= 128 && bigSize <= 128)
        {
            B.grimeSmall[i] = MakeGrime(r, s, caseSize, 11 + i);
            B.grimeBig[i] = MakeGrime(r, b, bigSize, 11 + i);
        }
        p += (caseSize * caseSize + bigSize * bigSize) * 4;
    }
}

// ----------------------------------------------------------------------------------------------------- drawing

static void Sparkle(float cx, float cy, float size, unsigned alpha)
{
    // a four-pointed star of light, drawn at full detail
    int i;
    SDL_Renderer *r = G_R();
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    for (i = 0; i <= (int)size; i++)
    {
        unsigned a = alpha * (size - i + 1) / (size + 1);
        SDL_SetRenderDrawColor(r, 255, 255, 230, a);
        SDL_RenderDrawPoint(r, (int)cx + i, (int)cy);
        SDL_RenderDrawPoint(r, (int)cx - i, (int)cy);
        SDL_RenderDrawPoint(r, (int)cx, (int)cy + i);
        SDL_RenderDrawPoint(r, (int)cx, (int)cy - i);
        if (i <= size / 3)
        {
            SDL_RenderDrawPoint(r, (int)cx + i, (int)cy + i);
            SDL_RenderDrawPoint(r, (int)cx - i, (int)cy - i);
            SDL_RenderDrawPoint(r, (int)cx + i, (int)cy - i);
            SDL_RenderDrawPoint(r, (int)cx - i, (int)cy + i);
        }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
}

// A badge in the case (hi-res pixels: x, y its top left, size its side): won and as clean as its polish says, or
// the dark shape it leaves in the case when not won yet
static u8 sHollowShade;   // a badge not won: drawn as its shape in black (the case), or in white (dark card panels)
void MP_BadgeCase_HollowShade(u8 shade) { sHollowShade = shade; }

static void DrawBadgeOf(const struct MpCardView *v, u8 region, u8 badge, int x, int y, int size, bool8 big)
{
    SDL_Renderer *r = G_R();
    u8 i = Index(region, badge);
    SDL_Texture *t = big ? B.big[i] : B.small[i];
    SDL_Texture *grime = big ? B.grimeBig[i] : B.grimeSmall[i];
    SDL_Rect dst = {x, y, size, size};
    u8 level;
    if (!t)
        return;
    if (big && B.spin && B.closeup)
    {
        // spinning: turned edge-on and round again (the width as a cosine), the light catching it face-on
        float a = (float)B.spin / 48.0f * 6.2831853f, k = cosf(a);
        int w = (int)(size * (k < 0 ? -k : k));
        if (w < 2)
            w = 2;
        dst.x = x + (size - w) / 2;
        dst.w = w;
    }
    if (v ? !MP_CardView_HasBadge(v, region, badge) : !MP_TCard_HasBadge(region, badge))
    {
        if (sHollowShade && !big && B.shape[i])
        {
            SDL_SetTextureAlphaMod(B.shape[i], 40);
            SDL_RenderCopy(r, B.shape[i], NULL, &dst);
            SDL_SetTextureAlphaMod(B.shape[i], 255);
            return;
        }
        SDL_SetTextureColorMod(t, 0, 0, 0);
        SDL_SetTextureAlphaMod(t, 70);
        SDL_RenderCopy(r, t, NULL, &dst);
        SDL_SetTextureColorMod(t, 255, 255, 255);
        SDL_SetTextureAlphaMod(t, 255);
        return;
    }
    level = MP_TCard_PolishLevel(PolishOf(v, region, badge));
    SDL_RenderCopy(r, t, NULL, &dst);
    if (level <= TCARD_POLISH_LEVEL_DIRTY)
    {
        // dull: the badge's own colors turned toward a tarnished brown, then the grime
        u8 dull = level == TCARD_POLISH_LEVEL_FILTHY ? 140 : 80;
        SDL_SetTextureColorMod(t, 120, 104, 80);
        SDL_SetTextureAlphaMod(t, dull);
        SDL_RenderCopy(r, t, NULL, &dst);
        SDL_SetTextureColorMod(t, 255, 255, 255);
        SDL_SetTextureAlphaMod(t, 255);
        if (grime)
        {
            SDL_SetTextureAlphaMod(grime, level == TCARD_POLISH_LEVEL_FILTHY ? 255 : 130);
            SDL_RenderCopy(r, grime, NULL, &dst);
        }
    }
    else if (level >= TCARD_POLISH_LEVEL_2_SPARKLES)
    {
        // well polished: a soft added shine, and Platinum's 2 or 4 sparkles twinkling at its edge
        static const s8 sSpots[4][2] = {{-38, -34}, {36, 26}, {30, -36}, {-34, 32}};
        u8 n = level == TCARD_POLISH_LEVEL_4_SPARKLES ? 4 : 2, k;
        float s = size / 64.0f;
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_ADD);
        SDL_SetTextureAlphaMod(t, level == TCARD_POLISH_LEVEL_4_SPARKLES ? 46 : 26);
        SDL_RenderCopy(r, t, NULL, &dst);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(t, 255);
        for (k = 0; k < n; k++)
        {
            u32 phase = (B.age * 3 + k * 37 + i * 11) % 90;
            float tw = phase < 45 ? phase / 45.0f : (90 - phase) / 45.0f;
            Sparkle(x + size / 2 + sSpots[k][0] * s * 0.62f, y + size / 2 + sSpots[k][1] * s * 0.62f, (3 + 5 * tw) * s,
                    (unsigned)(90 + 165 * tw));
        }
    }
    if (B.glint[i])
    {
        // the light passing over a badge that just shone up
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_ADD);
        SDL_SetTextureAlphaMod(t, B.glint[i] * 4);
        SDL_RenderCopy(r, t, NULL, &dst);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(t, 255);
    }
}

static void DrawBadge(u8 region, u8 badge, int x, int y, int size, bool8 big)
{
    DrawBadgeOf(sView, region, badge, x, y, size, big);
}

static void SlotCenter(u8 badge, int *cx, int *cy)
{
    *cx = 36 + (badge % 4) * 56;
    *cy = 55 + (badge / 4) * 36;
}

static const char *PolishWord(u8 level)
{
    static const char *const sWords[] = {"FILTHY", "DIRTY", "CLEAN", "SHINING", "SPARKLING!"};
    return sWords[level];
}

static void FormatDate(char *out, size_t n, u32 when)
{
    time_t t = (time_t)when;
    struct tm *tm = localtime(&t);
    if (tm == NULL)
        snprintf(out, n, "?");
    else
        snprintf(out, n, "%04d/%02d/%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
}

// the panel under the case: about the badge at the cursor
static void DrawPanel(void)
{
    u8 region = Region(), badge = B.cursor;
    const struct BadgeInfo *info = &sBadges[region][badge];
    char line[160], date[24], who[64];
    G_RoundFill(4, 115, 232, 43, C_CASE_EDGE, 255);
    G_RoundFill(5, 116, 230, 41, C_PANEL, 255);
    G_Text(10, 117, C_INK, G_NO_SHADOW, info->name, 0, TF_NORMAL);
    snprintf(line, sizeof(line), "%s, %s", info->leader, info->town);
    G_Text(10, 130, C_INK_SOFT, G_NO_SHADOW, line, 0, TF_SMALL);
    if (!Has(region, badge))
    {
        G_TextAlign(230, 117, TA_RIGHT, C_INK_SOFT, G_NO_SHADOW, "NOT WON YET", TF_SMALL);
        G_Text(10, 141, C_INK_SOFT, G_NO_SHADOW, sView ? "Not in their case yet." : "Win it at the gym, and it takes its place in the case.", 220, TF_SMALL);
        return;
    }
    if (sView)
    {
        // a friend's case: how it shines (their records stay theirs)
        u8 level = MP_TCard_PolishLevel(PolishOf(sView, region, badge));
        G_TextAlign(230, 117, TA_RIGHT, level >= TCARD_POLISH_LEVEL_2_SPARKLES ? 0xc07818 : level <= TCARD_POLISH_LEVEL_DIRTY ? 0x806040 : C_INK_SOFT,
                    G_NO_SHADOW, PolishWord(level), TF_SMALL);
        snprintf(line, sizeof(line), "In %s's case.", sView->name);
        G_Text(10, 140, C_INK, G_NO_SHADOW, line, 222, TF_SMALL);
        return;
    }
    {
        const struct MpTCardBadge *rec = MP_TCard_Badge(region, badge);
        u8 level = MP_TCard_PolishLevel(rec->polish);
        G_TextAlign(230, 117, TA_RIGHT, level >= TCARD_POLISH_LEVEL_2_SPARKLES ? 0xc07818 : level <= TCARD_POLISH_LEVEL_DIRTY ? 0x806040 : C_INK_SOFT,
                    G_NO_SHADOW, PolishWord(level), TF_SMALL);
        if (rec->when == 0)
            snprintf(line, sizeof(line), "Won before records began.");
        else
        {
            FormatDate(date, sizeof(date), rec->when);
            if (rec->leadSpecies && rec->leadSpecies < NUM_SPECIES)
            {
                char mon[16];
                MP_GameToAscii(mon, gSpeciesNames[rec->leadSpecies], sizeof(mon));
                snprintf(line, sizeof(line), "Won %s with %s Lv%d in the lead.", date, mon, rec->leadLevel);
            }
            else
                snprintf(line, sizeof(line), "Won %s.", date);
        }
        G_Text(10, 140, C_INK, G_NO_SHADOW, line, 222, TF_SMALL);
        if (rec->partnerCount)
        {
            u8 k;
            who[0] = 0;
            for (k = 0; k < rec->partnerCount; k++)
            {
                char name[MP_NAME_LEN + 4];
                MP_GameToAscii(name, rec->partners[k], sizeof(name));
                if (k)
                    strncat(who, k == rec->partnerCount - 1 ? " and " : ", ", sizeof(who) - strlen(who) - 1);
                strncat(who, name, sizeof(who) - strlen(who) - 1);
            }
            snprintf(line, sizeof(line), "A team win with %s!", who);
            G_Text(10, 149, 0xb06018, G_NO_SHADOW, line, 222, TF_SMALL);
        }
    }
}

static void DrawTabs(void)
{
    u8 k;
    for (k = 0; k < REGION_COUNT; k++)
    {
        int x = 22 + k * 50;
        bool8 on = k == B.tab;
        u8 b, y = on ? 1 : 3;
        G_RoundFill(x, y, 46, on ? 18 : 16, on ? C_CASE : C_CASE_DARK, 255);
        G_TextAlign(x + 23, y + 1, TA_CENTER, on ? C_INK : C_INK_SOFT, G_NO_SHADOW, sRegionNames[sTabRegion[k]], TF_SMALL);
        // a pip a badge, lit for those won
        for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
            G_Fill(x + 8 + b * 4, y + 12, 3, 2, Has(sTabRegion[k], b) ? 0xd09820 : on ? 0xc8bc90 : 0xa89c60);
    }
    // L / R
    G_RoundFill(4, 5, 13, 12, C_CASE_EDGE, 255);
    G_TextAlign(10, 6, TA_CENTER, G_WHITE, G_NO_SHADOW, "L", TF_SMALL);
    G_RoundFill(223, 5, 13, 12, C_CASE_EDGE, 255);
    G_TextAlign(229, 6, TA_CENTER, G_WHITE, G_NO_SHADOW, "R", TF_SMALL);
}

static void DrawCase(void)
{
    SDL_Renderer *r = G_R();
    u8 b;
    // (in the GBA's 240x160 pixels, each two of the picture's)
    SDL_RenderSetScale(r, 2, 2);
    G_Fill(0, 0, 240, 160, C_CASE_DARK);
    G_RoundFill(2, 18, 236, 96, C_CASE_EDGE, 255);
    G_RoundFill(3, 19, 234, 94, C_CASE, 255);
    G_RoundFill(9, 22, 222, 89, C_TRAY_DARK, 255);
    G_RoundFill(10, 23, 220, 87, C_TRAY, 255);
    G_TextAlign(120, 23, TA_CENTER, 0xd8d8d8, G_NO_SHADOW, "LEAGUE BADGES", TF_SMALL);
    DrawTabs();
    // the hollows and the badges at full detail
    SDL_RenderSetScale(r, 1, 1);
    for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
    {
        int cx, cy;
        SDL_Rect hollow;
        SlotCenter(b, &cx, &cy);
        hollow.x = cx * 2 - 38, hollow.y = cy * 2 - 38, hollow.w = hollow.h = 76;
        if (B.slot)
            SDL_RenderCopy(r, B.slot, NULL, &hollow);
        DrawBadge(Region(), b, cx * 2 - 32, cy * 2 - 32, 64, FALSE);
    }
    SDL_RenderSetScale(r, 2, 2);
    if (!B.closeup)
    {
        int cx, cy;
        SlotCenter(B.cursor, &cx, &cy);
        G_Focus(cx - 20, cy - 20, 40, 40, 0xf8d038, B.age);
    }
    DrawPanel();
}

static void DrawCloseup(void)
{
    SDL_Renderer *r = G_R();
    u8 region = Region(), badge = B.cursor;
    u8 polish = PolishOf(sView, region, badge);
    static const u8 sMarks[] = {TCARD_POLISH_DIRTY, TCARD_POLISH_NORMAL, TCARD_POLISH_2_SPARKLES, TCARD_POLISH_4_SPARKLES};
    int barX = 64, barW = 112, k, fill;
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 16, 14, 8, 170);
    SDL_RenderFillRect(r, NULL);
    SDL_RenderSetScale(r, 2, 2);
    G_RoundFill(46, 3, 148, 109, C_CASE_EDGE, 255);
    G_RoundFill(47, 4, 146, 107, C_CASE, 255);
    // the polish so far, and where it shines up (Platinum's levels)
    fill = barW * polish / TCARD_POLISH_MAX;
    G_RoundFill(barX - 1, 101, barW + 2, 8, C_CASE_EDGE, 255);
    G_Fill(barX, 102, barW, 6, C_SLOT);
    G_Fill(barX, 102, fill, 6, MP_TCard_PolishLevel(polish) >= TCARD_POLISH_LEVEL_2_SPARKLES ? 0xf0c030 : 0x90b8d8);
    for (k = 0; k < 4; k++)
        G_Fill(barX + barW * sMarks[k] / TCARD_POLISH_MAX, 101, 1, 8, C_CASE_EDGE);
    G_TextAlign(120, 89, TA_CENTER, C_INK_SOFT, G_NO_SHADOW, sView ? "Shined by its owner" : "Hold A and circle the stick to polish", TF_SMALL);
    SDL_RenderSetScale(r, 1, 1);
    if (B.well)
    {
        SDL_Rect w = {240 - 76, 88 - 76, 152, 152};
        SDL_RenderCopy(r, B.well, NULL, &w);
    }
    DrawBadge(region, badge, 240 - 64, 88 - 64, 128, TRUE);
    SDL_RenderSetScale(r, 2, 2);
    DrawPanel();
}

SDL_Texture *MP_BadgeCase_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!B.active)
        return NULL;
    G_Begin(r);
    if (B.owner != r)
    {
        // (a new renderer: the old one's textures went with it)
        memset(B.small, 0, sizeof(B.small));
        memset(B.big, 0, sizeof(B.big));
        memset(B.grimeSmall, 0, sizeof(B.grimeSmall));
        memset(B.grimeBig, 0, sizeof(B.grimeBig));
        memset(B.shape, 0, sizeof(B.shape));
        B.slot = B.well = NULL;
        B.canvas = NULL;
        B.owner = r;
    }
    if (!B.canvas)
    {
        B.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BC_W, BC_H);
        if (!B.canvas)
            return NULL;
        SDL_SetTextureBlendMode(B.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(B.canvas, SDL_ScaleModeLinear);
        if (!B.small[0])
            MakeTextures(r);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, B.canvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    DrawCase();
    if (B.closeup)
        DrawCloseup();
    MP_BadgeCase_FrameDrawn();
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return B.canvas;
}

// ----------------------------------------------------------------------------------------------------- input

bool8 MP_BadgeCase_IsActive(void)
{
    return B.active;
}

void MP_BadgeCase_OpenView(const struct MpCardView *view, u8 region)
{
    u8 k;
    sView = view;
    B.active = TRUE;
    B.closeup = FALSE;
    B.cursor = 0;
    B.tab = 0;
    for (k = 0; k < REGION_COUNT; k++)
        if (sTabRegion[k] == region)
            B.tab = k;
    B.prevKeys = 0xFFFF;   // (the button that opened it doesn't count as pressed in here)
    B.rubDir = 0;
    memset(B.glint, 0, sizeof(B.glint));
    PlaySE(SE_PC_LOGIN);
    MPNet_Log("card: BADGE CASE opened on %s%s", sRegionNames[region], view ? " (a friend's)" : "");
}

void MP_BadgeCase_Open(u8 region)
{
    MP_BadgeCase_OpenView(NULL, region);
}

static void Close(void)
{
    if (!sView)
        MP_TCard_SaveNow();
    B.active = FALSE;
    B.draining = TRUE;
    B.closeup = FALSE;
    PlaySE(SE_PC_OFF);
}

static void Pick(u8 badge)
{
    B.cursor = badge;   // (no sound: a badge chimes only when its button is pressed)
}

// The badge's chime: higher the cleaner it is (an octave down filthy, an octave up shining: 1/256 semitones)
static void Chime(void)
{
    s32 pitch = ((s32)PolishOf(sView, Region(), B.cursor) - 100) * 12 * 256 / 100;
    if (pitch > 12 * 256)
        pitch = 12 * 256;
    PlaySE(SE_NOTE_G);
    m4aMPlayPitchControl(&gMPlayInfo_SE1, TRACKS_ALL, (s16)pitch);
    m4aMPlayPitchControl(&gMPlayInfo_SE2, TRACKS_ALL, (s16)pitch);
}

// One rub (a stretch of a polishing circle): a point of polish when enough have added up (Platinum's), the controller
// rumbling (more on a dirtier badge) and a pulse, the light and a spin when it shines up
static void Rub(void)
{
    u8 region = Region(), badge = B.cursor;
    u8 polish;
    if (sView || !MP_TCard_HasBadge(region, badge))
        return;
    polish = MP_TCard_Badge(region, badge)->polish;
    Gamepad_Rumble(polish < TCARD_POLISH_DIRTY ? 45 : polish < TCARD_POLISH_NORMAL ? 30 : 18, 70);
    if (MP_TCard_Rub(region, badge))
    {
        u8 level = MP_TCard_PolishLevel(MP_TCard_Badge(region, badge)->polish);
        B.glint[Index(region, badge)] = 40;
        if (level >= TCARD_POLISH_LEVEL_NORMAL)
            B.spin = 48;
        Gamepad_Rumble(90, 220);
        PlaySE(level >= TCARD_POLISH_LEVEL_2_SPARKLES ? SE_SHINY : SE_SUCCESS);
        MPNet_Log("card: badge %d of %s polished up to %d", badge, sRegionNames[region], MP_TCard_Badge(region, badge)->polish);
    }
    else if ((B.age & 7) == 0)
        PlaySE(SE_BALL_TRAY_ENTER);
}

// A turn of the polishing circle (radians, either way): a rub every 30 degrees
static void Turned(float d)
{
    if (d > 3.14159265f)
        d -= 6.2831853f;
    if (d < -3.14159265f)
        d += 6.2831853f;
    B.turned += d < 0 ? -d : d;
    while (B.turned >= 0.5235988f)
    {
        B.turned -= 0.5235988f;
        Rub();
    }
}

// Holding A up close: the stick going round (or the D-pad's directions one after another) polishes
static void Polish(u16 keys)
{
    float lx, ly, rx, ry;
    s8 dir = -1;
    Gamepad_Sticks(&lx, &ly, &rx, &ry);
    if (rx * rx + ry * ry > lx * lx + ly * ly)
        lx = rx, ly = ry;
    if (lx * lx + ly * ly > 0.35f)
    {
        float a = atan2f(ly, lx);
        if (B.angleOk)
            Turned(a - B.angle);
        B.angle = a;
        B.angleOk = TRUE;
        B.lastDpad = -1;
        return;
    }
    B.angleOk = FALSE;
    if (keys & DPAD_UP) dir = 0;
    else if (keys & DPAD_RIGHT) dir = 1;
    else if (keys & DPAD_DOWN) dir = 2;
    else if (keys & DPAD_LEFT) dir = 3;
    if (dir >= 0 && B.lastDpad >= 0 && dir != B.lastDpad && ((dir - B.lastDpad + 4) % 4 == 1 || (dir - B.lastDpad + 4) % 4 == 3))
        Turned(1.5707964f);   // (a quarter turn)
    if (dir >= 0)
        B.lastDpad = dir;
}

static void Mouse(void)
{
    s32 x, y;
    bool8 down, moved = Platform_PoketchMouse(&x, &y, &down);
    bool8 clicked = down && !B.mouseDown;
    if (!moved && !clicked)
        return;
    if (B.closeup)
    {
        // the button held, circling the badge: the angle around its center turning polishes
        s32 dx = x - 120, dy = y - 44;
        if (clicked && dx * dx + dy * dy <= 34 * 34)
            Chime();
        if (down && B.mouseDown && dx * dx + dy * dy >= 6 * 6 && dx * dx + dy * dy <= 46 * 46)
        {
            float a = atan2f((float)dy, (float)dx);
            if (B.angleOk)
                Turned(a - B.angle);
            B.angle = a;
            B.angleOk = TRUE;
        }
        else if (!down)
            B.angleOk = FALSE;
        if (clicked && (x < 46 || x >= 194 || y >= 112))
            B.closeup = FALSE;
    }
    else if (clicked)
    {
        u8 b, k;
        for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
        {
            int cx, cy;
            SlotCenter(b, &cx, &cy);
            if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= 19 * 19)
            {
                Pick(b);
                if (Has(Region(), b))
                {
                    Chime();
                    B.closeup = TRUE;
                    B.angleOk = FALSE;
                    B.idle = 0;
                }
            }
        }
        for (k = 0; k < REGION_COUNT; k++)
            if (y < 20 && x >= 22 + k * 50 && x < 68 + k * 50 && k != B.tab)
            {
                B.tab = k;
                PlaySE(SE_SELECT);
            }
    }
    B.mouseX = x;
    B.mouseY = y;
    B.mouseDown = down;
}

// A frame while the case is open (the game waits). TRUE while it's open.
bool8 MP_BadgeCase_Step(u16 keys)
{
    u16 pressed;
    u8 i;
    if (B.draining)
    {
        if (keys & (A_BUTTON | B_BUTTON | SELECT_BUTTON | START_BUTTON))
            return TRUE;
        B.draining = FALSE;
    }
    if (!B.active)
        return FALSE;
    B.age++;
    for (i = 0; i < NUM_BADGES_ALL; i++)
        if (B.glint[i])
            B.glint[i]--;
    pressed = keys & ~B.prevKeys;
    B.prevKeys = keys;
    Mouse();
    if (!B.active)
        return TRUE;
    if (B.closeup)
    {
        if (B.spin)
            B.spin--;
        else if (++B.idle >= 300 && MP_TCard_PolishLevel(PolishOf(sView, Region(), B.cursor)) >= TCARD_POLISH_LEVEL_NORMAL)
            B.spin = 48, B.idle = 0;   // (a clean badge turns now and then, catching the light)
        if (pressed & B_BUTTON)
        {
            B.closeup = FALSE;
            B.angleOk = FALSE;
        }
        else if (pressed & A_BUTTON)
            Chime();
        else if (keys & A_BUTTON)
            Polish(keys);
        else
            B.angleOk = FALSE, B.lastDpad = -1;
        return TRUE;
    }
    if (pressed & B_BUTTON)
    {
        Close();
        return TRUE;
    }
    if (pressed & A_BUTTON)
    {
        if (Has(Region(), B.cursor))
        {
            Chime();
            B.closeup = TRUE;
            B.angleOk = FALSE;
            B.lastDpad = -1;
            B.idle = 0;
        }
        else
            PlaySE(SE_FAILURE);
    }
    else if (pressed & L_BUTTON)
    {
        B.tab = (B.tab + REGION_COUNT - 1) % REGION_COUNT;
        PlaySE(SE_SELECT);
    }
    else if (pressed & R_BUTTON)
    {
        B.tab = (B.tab + 1) % REGION_COUNT;
        PlaySE(SE_SELECT);
    }
    else if (pressed & DPAD_LEFT)
        Pick(B.cursor % 4 ? B.cursor - 1 : B.cursor + 3);
    else if (pressed & DPAD_RIGHT)
        Pick(B.cursor % 4 != 3 ? B.cursor + 1 : B.cursor - 3);
    else if (pressed & (DPAD_UP | DPAD_DOWN))
        Pick((B.cursor + 4) % 8);
    return TRUE;
}

// For the TRAINER CARD's BADGES & STARS page (mp_cardscreen.c): a badge as the case shows it (the textures made on
// first use; MP_BadgeCase_Tick once a frame moves its sparkles)
void MP_BadgeCase_Tick(void)
{
    if (!B.active)
        B.age++;
}

void MP_BadgeCase_DrawBadgeFor(SDL_Renderer *r, const struct MpCardView *v, u8 region, u8 badge, int x, int y, int size)
{
    if (B.owner != r)
    {
        memset(B.small, 0, sizeof(B.small));
        memset(B.big, 0, sizeof(B.big));
        memset(B.grimeSmall, 0, sizeof(B.grimeSmall));
        memset(B.grimeBig, 0, sizeof(B.grimeBig));
        memset(B.shape, 0, sizeof(B.shape));
        B.slot = B.well = NULL;
        B.canvas = NULL;
        B.owner = r;
    }
    if (!B.small[0])
        MakeTextures(r);
    DrawBadgeOf(v && v->own ? NULL : v, region, badge, x, y, size, size > 64);
}

void MP_BadgeCase_DestroyTextures(void)
{
    DestroyTextures();
    B.owner = NULL;
}

// ----------------------------------------------------------------------------------------------------- spectating
// Friends spectating see these screens as the game's 240x160 picture (as the card games' Tcg_Frame): the 480x320
// canvas read back and halved (each 2x2 pixels averaged)
static u16 sBigFrame[BC_W * BC_H];
static u16 sFrame[240 * 160];
static bool8 sFrameRead;

const u16 *MP_CardScreen_Halve(SDL_Renderer *r, SDL_Texture *canvas, u16 *out)
{
    SDL_Texture *previous = SDL_GetRenderTarget(r);
    int x, y;
    SDL_SetRenderTarget(r, canvas);
    if (SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ABGR1555, sBigFrame, BC_W * 2) != 0)
    {
        SDL_SetRenderTarget(r, previous);
        return NULL;
    }
    SDL_SetRenderTarget(r, previous);
    for (y = 0; y < 160; y++)
        for (x = 0; x < 240; x++)
        {
            const u16 *p = &sBigFrame[(y * 2) * BC_W + x * 2];
            u16 a = p[0], b = p[1], c = p[BC_W], d = p[BC_W + 1];
            u16 rr = ((a & 31) + (b & 31) + (c & 31) + (d & 31) + 2) / 4;
            u16 gg = (((a >> 5) & 31) + ((b >> 5) & 31) + ((c >> 5) & 31) + ((d >> 5) & 31) + 2) / 4;
            u16 bb = (((a >> 10) & 31) + ((b >> 10) & 31) + ((c >> 10) & 31) + ((d >> 10) & 31) + 2) / 4;
            out[y * 240 + x] = rr | (gg << 5) | (bb << 10);
        }
    return out;
}

const u16 *MP_BadgeCase_Frame(void)
{
    if (!B.active || !B.canvas || !B.owner)
        return NULL;
    if (!sFrameRead)
        sFrameRead = MP_CardScreen_Halve(B.owner, B.canvas, sFrame) != NULL;
    return sFrameRead ? sFrame : NULL;
}

static void MP_BadgeCase_FrameDrawn(void)
{
    sFrameRead = FALSE;   // (a new picture: read again when spectators need it)
}
