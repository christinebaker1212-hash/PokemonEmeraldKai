// Moemon Kai PC: display settings and the SDL output pipeline.
//
// Owns the window, renderer and textures. The 240x160 game frame is drawn to
// the window with one of three scaling modes:
//   PIXEL PERFECT  largest whole-number scale that fits, centered, black bars
//   SHARP FIT      as large as possible keeping 3:2
//   STRETCH        fills the whole window
// Whenever the output size is not an exact multiple of 240x160 the frame is
// "sharp-bilinear" scaled: it is first enlarged by a whole number (nearest
// neighbour) into a render target slightly larger than the output, then that
// is drawn to the output rect with linear filtering. Every game pixel ends up
// the same size, only its edges are blended over at most one screen pixel, so
// there is no shimmering when things scroll and no visible blur.
//
// Filters (scanlines / LCD grid) are an overlay texture made per whole-number
// scale and drawn on top of the enlarged frame, so they line up exactly with
// game pixels. The game framebuffer itself is never touched.
//
// Settings are stored in moemon_display.ini next to the exe
// ($MOEMON_DISPLAY_CONFIG overrides the path).
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/display.h"
#include "tcg/integration.h"
#include "platform/fight.h"

#ifdef PLATFORM_SDL2

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL2/SDL.h>

#include "platform/display_sdl.h"
#include "platform/app_icon.h"   // Moemon Co-op: the Poke Ball window icon

#define GBA_W 240
#define GBA_H 160
#define POS_UNSET INT_MIN
#define MAX_PRESCALE 16
#define CONFIG_NAME "moemon_display.ini"
// Ignore window geometry events this long after a fullscreen switch, so the
// fullscreen size is never remembered as the window size.
#define TOGGLE_GRACE_MS 750

struct DisplayConfig
{
    int fullscreen;
    int windowScale;
    int scaleMode;
    int filter;
    int vsync;
    int winW, winH;   // windowed size, restored when leaving fullscreen
    int winX, winY;   // windowed position, POS_UNSET = centered
    int maximized;    // the window was maximized
};

static struct DisplayConfig sCfg = {
    0, 3, DISPLAY_SCALE_SHARP_FIT, DISPLAY_FILTER_OFF, 1, 0, 0, POS_UNSET, POS_UNSET, 0
};
static char sConfigPath[1024] = CONFIG_NAME;
static int sPersist = 1;         // 0 in test mode without $MOEMON_DISPLAY_CONFIG
static int sHadConfigFile;

static SDL_Window *sWindow;
static SDL_Renderer *sRenderer;
static SDL_Texture *sGameTex;    // 240x160 game frame, nearest
static SDL_Texture *sScaleTex;   // 240*kx x 160*ky render target, linear
static SDL_Texture *sOverlayTex; // filter pattern, same size as the enlarged frame
static int sScaleKx, sScaleKy, sScaleSrcW, sScaleSrcH;
static int sOverlayKx, sOverlayKy, sOverlayFilter = -1, sOverlaySrcW, sOverlaySrcH;
static int sTargetsSupported;
static int sMaxTexW, sMaxTexH;
static const void *sLastFrame;
static int sDirty = 1;
static int sFilterShown;
static int sFsWasMaximized;
static Uint32 sToggleTicks;

// Moemon Co-op spectating panel (Display_SetPanel)
#define PANEL_PAD 6       // around the cards, in game pixels (times the group's biggest scale)
#define PANEL_GAP 6       // between cards
static int sPanelCount;   // cards in the main window (not the pop-outs)
static int sPanelFit = 1; // the biggest scale a card in the window gets now (each gets min(its own, this))
static int sPanelOwnK;    // the game's whole-number scale the window keeps beside the panel
static int sPanelSavedW, sPanelSavedH, sPanelHadWindow;   // the window before the panel resized it
static SDL_Texture *sPanelFrameTex[DISPLAY_PANEL_MAX], *sPanelLabelTex[DISPLAY_PANEL_MAX];
static int sPanelHasFrame[DISPLAY_PANEL_MAX];
static unsigned sPanelAccent[DISPLAY_PANEL_MAX];
static int sPanelSide[DISPLAY_PANEL_MAX], sPanelReq[DISPLAY_PANEL_MAX], sPanelId[DISPLAY_PANEL_MAX];

// Pop-outs: a friend's screen in a window of its own (its own renderer and textures)
struct Popout
{
    int used, id, scale, shown, dirty, hasFrame;
    unsigned accent;
    Uint32 windowId;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *frame, *label;
};
static struct Popout sPopouts[DISPLAY_PANEL_MAX];
static int sClosedPopouts[8], sClosedCount;
static int sGivenCount, sGivenWhere[DISPLAY_PANEL_MAX], sGivenId[DISPLAY_PANEL_MAX];   // card i: window spot, or -1 = pop-out
static void DestroyPopout(struct Popout *p);
static void DestroyAllPopouts(void);

static const char *const sScaleNames[DISPLAY_SCALE_COUNT] = {"pixel_perfect", "sharp_fit", "stretch"};
static const char *const sFilterNames[DISPLAY_FILTER_COUNT] = {"off", "scanlines", "lcd_grid"};

static int Clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static int CeilDiv(int a, int b)
{
    return (a + b - 1) / b;
}

// ---------------------------------------------------------------------------
// Config file
// ---------------------------------------------------------------------------

static char *Trim(char *s)
{
    char *end;
    while (*s == ' ' || *s == '\t')
        s++;
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
        *--end = 0;
    return s;
}

static int ParseBool(const char *v)
{
    return strcmp(v, "1") == 0 || SDL_strcasecmp(v, "on") == 0 || SDL_strcasecmp(v, "true") == 0
        || SDL_strcasecmp(v, "yes") == 0;
}

static int ParseEnum(const char *v, const char *const *names, int count, int def)
{
    int i;
    for (i = 0; i < count; i++)
        if (SDL_strcasecmp(v, names[i]) == 0)
            return i;
    if (v[0] >= '0' && v[0] <= '9')
        return Clamp(atoi(v), 0, count - 1);
    return def;
}

static int ParsePos(const char *v)
{
    if ((v[0] >= '0' && v[0] <= '9') || v[0] == '-')
        return atoi(v);
    return POS_UNSET;
}

static void LoadConfig(void)
{
    char line[256];
    FILE *f = fopen(sConfigPath, "r");
    if (f == NULL)
        return;
    sHadConfigFile = 1;
    while (fgets(line, sizeof(line), f))
    {
        char *key, *val, *eq;
        key = Trim(line);
        if (key[0] == ';' || key[0] == '#' || key[0] == 0)
            continue;
        eq = strchr(key, '=');
        if (eq == NULL)
            continue;
        *eq = 0;
        key = Trim(key);
        val = Trim(eq + 1);
        if (strcmp(key, "fullscreen") == 0)            sCfg.fullscreen = ParseBool(val);
        else if (strcmp(key, "window_scale") == 0)     sCfg.windowScale = Clamp(atoi(val), 1, DISPLAY_MAX_WINDOW_SCALE);
        else if (strcmp(key, "scaling") == 0)          sCfg.scaleMode = ParseEnum(val, sScaleNames, DISPLAY_SCALE_COUNT, sCfg.scaleMode);
        else if (strcmp(key, "filter") == 0)           sCfg.filter = ParseEnum(val, sFilterNames, DISPLAY_FILTER_COUNT, sCfg.filter);
        else if (strcmp(key, "vsync") == 0)            sCfg.vsync = ParseBool(val);
        else if (strcmp(key, "window_width") == 0)     sCfg.winW = atoi(val);
        else if (strcmp(key, "window_height") == 0)    sCfg.winH = atoi(val);
        else if (strcmp(key, "window_x") == 0)         sCfg.winX = ParsePos(val);
        else if (strcmp(key, "window_y") == 0)         sCfg.winY = ParsePos(val);
        else if (strcmp(key, "window_maximized") == 0) sCfg.maximized = ParseBool(val);
    }
    fclose(f);
}

static void WriteConfig(void)
{
    FILE *f;
    char xs[16], ys[16];
    if (!sPersist)
        return;
    f = fopen(sConfigPath, "w");
    if (f == NULL)
    {
        printf("[display] could not write %s\n", sConfigPath);
        return;
    }
    if (sCfg.winX == POS_UNSET || sCfg.winY == POS_UNSET)
    {
        snprintf(xs, sizeof(xs), "center");
        snprintf(ys, sizeof(ys), "center");
    }
    else
    {
        snprintf(xs, sizeof(xs), "%d", sCfg.winX);
        snprintf(ys, sizeof(ys), "%d", sCfg.winY);
    }
    fprintf(f,
        "; Moemon Emerald Kai display settings.\n"
        "; Easiest to change in game: START > DISPLAY. F11 or Alt+Enter toggles fullscreen.\n"
        "; fullscreen = 0 or 1 (borderless fullscreen on the current monitor)\n"
        "fullscreen = %d\n"
        "; window_scale = 1 to %d (window is 240x160 times this)\n"
        "window_scale = %d\n"
        "; scaling = pixel_perfect, sharp_fit or stretch\n"
        "scaling = %s\n"
        "; filter = off, scanlines or lcd_grid\n"
        "filter = %s\n"
        "vsync = %d\n"
        "; last window size and position (delete these lines to reset)\n"
        "window_width = %d\n"
        "window_height = %d\n"
        "window_x = %s\n"
        "window_y = %s\n"
        "window_maximized = %d\n",
        sCfg.fullscreen, DISPLAY_MAX_WINDOW_SCALE, sCfg.windowScale,
        sScaleNames[sCfg.scaleMode], sFilterNames[sCfg.filter], sCfg.vsync,
        sCfg.winW, sCfg.winH, xs, ys, sCfg.maximized);
    fclose(f);
}

// ---------------------------------------------------------------------------
// Window geometry helpers
// ---------------------------------------------------------------------------

static int WindowDisplayIndex(void)
{
    int d = sWindow ? SDL_GetWindowDisplayIndex(sWindow) : 0;
    return d < 0 ? 0 : d;
}

static void GetUsableBounds(int display, SDL_Rect *r)
{
    const char *test = getenv("MOEMON_TEST_MONITOR");   // tests: pretend the monitor is WxH
    if (test && sscanf(test, "%dx%d", &r->w, &r->h) == 2)
    {
        r->x = r->y = 0;
        return;
    }
    if (SDL_GetDisplayUsableBounds(display, r) != 0 && SDL_GetDisplayBounds(display, r) != 0)
    {
        r->x = r->y = 0;
        r->w = 1280;
        r->h = 720;
    }
}

// Window decoration sizes (title bar etc.), or a safe guess.
static void GetBorders(int *top, int *left, int *bottom, int *right)
{
    int t = 0, l = 0, b = 0, r = 0;
    if (sWindow && !(SDL_GetWindowFlags(sWindow) & SDL_WINDOW_FULLSCREEN)
     && SDL_GetWindowBordersSize(sWindow, &t, &l, &b, &r) == 0 && t > 0)
    {
        *top = t, *left = l, *bottom = b, *right = r;
        return;
    }
    *top = 40, *left = 8, *bottom = 8, *right = 8;
}

static int MaxScaleOnDisplay(int display)
{
    SDL_Rect ub;
    int t, l, b, r, s;
    GetUsableBounds(display, &ub);
    GetBorders(&t, &l, &b, &r);
    s = (ub.w - l - r) / GBA_W;
    if ((ub.h - t - b) / GBA_H < s)
        s = (ub.h - t - b) / GBA_H;
    return Clamp(s, 1, DISPLAY_MAX_WINDOW_SCALE);
}

// Keep a w x h window fully inside the usable area of its display.
static void ClampWindowPos(int display, int *x, int *y, int w, int h)
{
    SDL_Rect ub;
    int t, l, b, r;
    GetUsableBounds(display, &ub);
    GetBorders(&t, &l, &b, &r);
    if (*x + w + r > ub.x + ub.w)
        *x = ub.x + ub.w - w - r;
    if (*y + h + b > ub.y + ub.h)
        *y = ub.y + ub.h - h - b;
    if (*x < ub.x + l)
        *x = ub.x + l;
    if (*y < ub.y + t)
        *y = ub.y + t;
}

static int DisplayContainingPoint(int x, int y)
{
    int i, n = SDL_GetNumVideoDisplays();
    for (i = 0; i < n; i++)
    {
        SDL_Rect b;
        if (SDL_GetDisplayBounds(i, &b) == 0 && x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h)
            return i;
    }
    return -1;
}

// Make the saved windowed size/position sane for the current monitors.
static void ValidateWindowedGeometry(void)
{
    int display = 0, maxS;
    SDL_Rect ub;

    if (sCfg.winX != POS_UNSET && sCfg.winY != POS_UNSET)
    {
        display = DisplayContainingPoint(sCfg.winX + sCfg.winW / 2, sCfg.winY + 8);
        if (display < 0)
        {
            display = 0;
            sCfg.winX = sCfg.winY = POS_UNSET;
        }
    }
    maxS = MaxScaleOnDisplay(display);
    if (!sHadConfigFile)
    {
        // First run: a comfortable size, about 3/4 of the desktop height.
        GetUsableBounds(display, &ub);
        sCfg.windowScale = Clamp(ub.h * 3 / 4 / GBA_H, 1, maxS);
    }
    sCfg.windowScale = Clamp(sCfg.windowScale, 1, maxS);
    GetUsableBounds(display, &ub);
    if (sCfg.winW < GBA_W || sCfg.winH < GBA_H || sCfg.winW > ub.w || sCfg.winH > ub.h)
    {
        sCfg.winW = GBA_W * sCfg.windowScale;
        sCfg.winH = GBA_H * sCfg.windowScale;
    }
    if (sCfg.winX != POS_UNSET)
        ClampWindowPos(display, &sCfg.winX, &sCfg.winY, sCfg.winW, sCfg.winH);
}

static void SnapshotWindowed(int force)
{
    Uint32 f;
    if (!sWindow || (!force && SDL_GetTicks() - sToggleTicks < TOGGLE_GRACE_MS))
        return;
    f = SDL_GetWindowFlags(sWindow);
    if (f & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN))
        return;
    sCfg.maximized = (f & SDL_WINDOW_MAXIMIZED) != 0;
    if (sCfg.maximized)
        return;
    // With the spectating panel up the window is the panel's size: remember the game's own
    if (sPanelCount == 0)
        SDL_GetWindowSize(sWindow, &sCfg.winW, &sCfg.winH);
    SDL_GetWindowPosition(sWindow, &sCfg.winX, &sCfg.winY);
}

// ---------------------------------------------------------------------------
// Renderer and textures
// ---------------------------------------------------------------------------

static SDL_Texture *CreateTex(Uint32 format, int access, int w, int h, int linear)
{
    SDL_Texture *t;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, linear ? "1" : "0");
    t = SDL_CreateTexture(sRenderer, format, access, w, h);
#if SDL_VERSION_ATLEAST(2, 0, 12)
    if (t)
        SDL_SetTextureScaleMode(t, linear ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
#endif
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    return t;
}

static void DestroyTextures(void)
{
    int i;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
    {
        if (sPanelFrameTex[i])
            SDL_DestroyTexture(sPanelFrameTex[i]);
        if (sPanelLabelTex[i])
            SDL_DestroyTexture(sPanelLabelTex[i]);
        sPanelFrameTex[i] = sPanelLabelTex[i] = NULL;
        sPanelHasFrame[i] = 0;
    }
    if (sGameTex)
        SDL_DestroyTexture(sGameTex);
    if (sScaleTex)
        SDL_DestroyTexture(sScaleTex);
    if (sOverlayTex)
        SDL_DestroyTexture(sOverlayTex);
    sGameTex = sScaleTex = sOverlayTex = NULL;
    sScaleKx = sScaleKy = 0;
    sOverlayKx = sOverlayKy = 0;
    sOverlayFilter = -1;
}

static int CreateGameTexture(void)
{
    sGameTex = CreateTex(SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, GBA_W, GBA_H, 0);
    if (sGameTex == NULL)
    {
        printf("[display] could not create texture: %s\n", SDL_GetError());
        return -1;
    }
    // The frame is always opaque, whatever the alpha bit says.
    SDL_SetTextureBlendMode(sGameTex, SDL_BLENDMODE_NONE);
    if (sLastFrame)
        SDL_UpdateTexture(sGameTex, NULL, sLastFrame, GBA_W * 2);
    return 0;
}

static int CreateRenderer(void)
{
    SDL_RendererInfo info;
    Uint32 vs = sCfg.vsync ? SDL_RENDERER_PRESENTVSYNC : 0;

    sRenderer = SDL_CreateRenderer(sWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | vs);
    if (sRenderer == NULL)
        sRenderer = SDL_CreateRenderer(sWindow, -1, SDL_RENDERER_ACCELERATED | vs);
    if (sRenderer == NULL)
        sRenderer = SDL_CreateRenderer(sWindow, -1, vs);
    if (sRenderer == NULL)
        sRenderer = SDL_CreateRenderer(sWindow, -1, SDL_RENDERER_SOFTWARE);
    if (sRenderer == NULL)
    {
        printf("[display] could not create renderer: %s\n", SDL_GetError());
        return -1;
    }
    sMaxTexW = sMaxTexH = 0;
    if (SDL_GetRendererInfo(sRenderer, &info) == 0)
    {
        sMaxTexW = info.max_texture_width;
        sMaxTexH = info.max_texture_height;
        printf("[display] renderer: %s%s\n", info.name,
               (info.flags & SDL_RENDERER_PRESENTVSYNC) ? " (vsync)" : "");
    }
    sTargetsSupported = SDL_RenderTargetSupported(sRenderer);
    return CreateGameTexture();
}

static void RecreateRenderer(void)
{
    Tcg_DestroyTextures();
    Fight_DestroyTextures();
    DestroyTextures();
    if (sRenderer)
        SDL_DestroyRenderer(sRenderer);
    sRenderer = NULL;
    CreateRenderer();
    sDirty = 1;
}

// The whole-number enlargement of the picture (srcW x srcH: the GBA's 240x160, or a VS fight's 480x320)
static int EnsureScaleTex(int kx, int ky, int srcW, int srcH)
{
    if (sScaleTex && sScaleKx == kx && sScaleKy == ky && sScaleSrcW == srcW && sScaleSrcH == srcH)
        return 1;
    if (sScaleTex)
        SDL_DestroyTexture(sScaleTex);
    sScaleTex = CreateTex(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, srcW * kx, srcH * ky, 1);
    sScaleKx = sScaleKy = 0;
    if (sScaleTex == NULL)
        return 0;
    SDL_SetTextureBlendMode(sScaleTex, SDL_BLENDMODE_NONE);
    sScaleKx = kx;
    sScaleKy = ky;
    sScaleSrcW = srcW;
    sScaleSrcH = srcH;
    return 1;
}

// Darkness of one screen row (or column) inside a game pixel that is k screen
// pixels tall. Light: only the last row, plus a softer one at big scales.
static Uint8 ScanlineAlpha(int pos, int k)
{
    if (k < 2)
        return 0;
    if (pos == k - 1)
        return 72;
    if (k >= 6 && pos == k - 2)
        return 32;
    return 0;
}

static Uint8 GridAlpha(int pos, int k)
{
    if (k < 3)
        return 0;
    if (pos == k - 1)
        return 60;
    if (k >= 8 && pos == k - 2)
        return 24;
    return 0;
}

// The filter's overlay (scanlines or the LCD grid) over each of the picture's srcW x srcH pixels
static int EnsureOverlay(int kx, int ky, int srcW, int srcH)
{
    int w = srcW * kx, h = srcH * ky, x, y;
    Uint32 *pixels;
    Uint8 colA[MAX_PRESCALE + 1];

    if (sOverlayTex && sOverlayKx == kx && sOverlayKy == ky && sOverlayFilter == sCfg.filter && sOverlaySrcW == srcW &&
        sOverlaySrcH == srcH)
        return 1;
    if (sOverlayTex)
        SDL_DestroyTexture(sOverlayTex);
    sOverlayTex = NULL;
    sOverlayFilter = -1;
    if (kx > MAX_PRESCALE || ky > MAX_PRESCALE)
        return 0;
    pixels = malloc((size_t)w * h * sizeof(Uint32));
    if (pixels == NULL)
        return 0;
    for (x = 0; x < kx; x++)
        colA[x] = (sCfg.filter == DISPLAY_FILTER_LCD_GRID) ? GridAlpha(x, kx) : 0;
    for (y = 0; y < h; y++)
    {
        Uint8 rowA = (sCfg.filter == DISPLAY_FILTER_LCD_GRID) ? GridAlpha(y % ky, ky) : ScanlineAlpha(y % ky, ky);
        Uint32 *row = pixels + (size_t)y * w;
        for (x = 0; x < w; x++)
        {
            Uint8 a = colA[x % kx];
            if (rowA > a)
                a = rowA;
            row[x] = (Uint32)a << 24; // black with alpha (ARGB8888)
        }
    }
    sOverlayTex = CreateTex(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h, 0);
    if (sOverlayTex)
    {
        SDL_UpdateTexture(sOverlayTex, NULL, pixels, w * (int)sizeof(Uint32));
        SDL_SetTextureBlendMode(sOverlayTex, SDL_BLENDMODE_BLEND);
        sOverlayKx = kx;
        sOverlayKy = ky;
        sOverlayFilter = sCfg.filter;
        sOverlaySrcW = srcW;
        sOverlaySrcH = srcH;
    }
    free(pixels);
    return sOverlayTex != NULL;
}

// Where the game image goes in an outW x outH output.
static void ComputeDstRect(int outW, int outH, SDL_Rect *dst)
{
    int mode = sCfg.scaleMode;

    if (mode == DISPLAY_SCALE_PIXEL_PERFECT)
    {
        int k = outW / GBA_W;
        if (outH / GBA_H < k)
            k = outH / GBA_H;
        if (k >= 1)
        {
            dst->w = GBA_W * k;
            dst->h = GBA_H * k;
            dst->x = (outW - dst->w) / 2;
            dst->y = (outH - dst->h) / 2;
            return;
        }
        mode = DISPLAY_SCALE_SHARP_FIT; // window smaller than 1x
    }
    if (mode == DISPLAY_SCALE_STRETCH)
    {
        dst->x = dst->y = 0;
        dst->w = outW;
        dst->h = outH;
        return;
    }
    if ((long)outW * GBA_H >= (long)outH * GBA_W)
    {
        dst->h = outH;
        dst->w = (outH * GBA_W + GBA_H / 2) / GBA_H;
    }
    else
    {
        dst->w = outW;
        dst->h = (outW * GBA_H + GBA_W / 2) / GBA_W;
    }
    dst->x = (outW - dst->w) / 2;
    dst->y = (outH - dst->h) / 2;
}

// ---------------------------------------------------------------------------
// Spectating panel
// ---------------------------------------------------------------------------

// Where the cards in the window go at fit f: rows above / below the game (the whole width), columns left / right of
// it (between the rows). Each card is the screen and its label inside a one-game-pixel outline, at min(its scale, f).
struct PanelLayout
{
    int scale[DISPLAY_PANEL_MAX];
    int w[DISPLAY_SIDE_COUNT], h[DISPLAY_SIDE_COUNT];   // each group's box (0 x 0: nobody there)
    int maxScale[DISPLAY_SIDE_COUNT];
};

static int CardW(int s) { return GBA_W * s + 2 * s; }
static int CardH(int s) { return GBA_H * s + DISPLAY_PANEL_LABEL_H * s + 2 * s; }

static void PanelLayout(int f, struct PanelLayout *L)
{
    int i, side;
    memset(L, 0, sizeof(*L));
    for (i = 0; i < sPanelCount; i++)
    {
        L->scale[i] = sPanelReq[i] < f ? sPanelReq[i] : f;
        if (L->scale[i] > L->maxScale[sPanelSide[i]])
            L->maxScale[sPanelSide[i]] = L->scale[i];
    }
    for (side = 0; side < DISPLAY_SIDE_COUNT; side++)
    {
        int n = 0, along = 0, across = 0, m = L->maxScale[side];
        int row = side == DISPLAY_SIDE_BELOW || side == DISPLAY_SIDE_ABOVE;
        for (i = 0; i < sPanelCount; i++)
        {
            int cw, ch;
            if (sPanelSide[i] != side)
                continue;
            cw = CardW(L->scale[i]);
            ch = CardH(L->scale[i]);
            along += row ? cw : ch;
            if ((row ? ch : cw) > across)
                across = row ? ch : cw;
            n++;
        }
        if (n == 0)
            continue;
        along += 2 * PANEL_PAD * m + (n - 1) * PANEL_GAP * m;
        across += 2 * PANEL_PAD * m;
        L->w[side] = row ? along : across;
        L->h[side] = row ? across : along;
    }
}

static int PanelMaxReq(void)
{
    int i, m = 1;
    for (i = 0; i < sPanelCount; i++)
        if (sPanelReq[i] > m)
            m = sPanelReq[i];
    return m;
}

// Does the layout fit an outW x outH output with room for the game at 1x?
static int PanelFits(const struct PanelLayout *L, int outW, int outH)
{
    int midH = outH - L->h[DISPLAY_SIDE_ABOVE] - L->h[DISPLAY_SIDE_BELOW];
    return L->w[DISPLAY_SIDE_ABOVE] <= outW && L->w[DISPLAY_SIDE_BELOW] <= outW
        && outW - L->w[DISPLAY_SIDE_LEFT] - L->w[DISPLAY_SIDE_RIGHT] >= GBA_W
        && midH >= GBA_H && midH >= L->h[DISPLAY_SIDE_LEFT] && midH >= L->h[DISPLAY_SIDE_RIGHT];
}

// The biggest fit whose layout fits the output (1 when nothing does)
static int PanelFitScale(int outW, int outH)
{
    int f;
    struct PanelLayout L;
    for (f = PanelMaxReq(); f > 1; f--)
    {
        PanelLayout(f, &L);
        if (PanelFits(&L, outW, outH))
            break;
    }
    return f;
}

// The window's size for fit f with the game at whole scale k
static void PanelWindowSize(int f, int k, int *w, int *h)
{
    struct PanelLayout L;
    int midW, midH;
    PanelLayout(f, &L);
    midW = L.w[DISPLAY_SIDE_LEFT] + GBA_W * k + L.w[DISPLAY_SIDE_RIGHT];
    midH = GBA_H * k;
    if (L.h[DISPLAY_SIDE_LEFT] > midH)
        midH = L.h[DISPLAY_SIDE_LEFT];
    if (L.h[DISPLAY_SIDE_RIGHT] > midH)
        midH = L.h[DISPLAY_SIDE_RIGHT];
    *w = midW;
    if (L.w[DISPLAY_SIDE_ABOVE] > *w)
        *w = L.w[DISPLAY_SIDE_ABOVE];
    if (L.w[DISPLAY_SIDE_BELOW] > *w)
        *w = L.w[DISPLAY_SIDE_BELOW];
    *h = L.h[DISPLAY_SIDE_ABOVE] + midH + L.h[DISPLAY_SIDE_BELOW];
}

// A window (not fullscreen or maximized) takes the size the panel needs: the game keeps its scale, or gets
// smaller, then the screens do, until everything fits the monitor
static void FitWindowToPanel(void)
{
    int display, k, f, w = 0, h = 0, x, y, t, l, b, r, minW, minH;
    SDL_Rect ub;
    Uint32 f2;
    if (sWindow == NULL)
        return;
    sDirty = 1;
    f2 = SDL_GetWindowFlags(sWindow);
    if (sPanelCount > 0)
    {
        PanelWindowSize(1, 1, &minW, &minH);
        SDL_SetWindowMinimumSize(sWindow, minW, minH);
    }
    else
        SDL_SetWindowMinimumSize(sWindow, GBA_W, GBA_H);
    if (f2 & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED))
        return;
    display = WindowDisplayIndex();
    GetUsableBounds(display, &ub);
    GetBorders(&t, &l, &b, &r);
    if (sPanelCount == 0)
    {
        // Gone: the window as it was (unless it wasn't a plain window then)
        if (!sPanelHadWindow)
            return;
        w = sPanelSavedW;
        h = sPanelSavedH;
    }
    else
    {
        for (f = PanelMaxReq(); f >= 1; f--)
        {
            for (k = sPanelOwnK; k >= 1; k--)
            {
                PanelWindowSize(f, k, &w, &h);
                if (w <= ub.w - l - r && h <= ub.h - t - b)
                    break;
            }
            if (k >= 1)
                break;
        }
        if (f < 1)
            PanelWindowSize(1, 1, &w, &h);
    }
    if (w < GBA_W || h < GBA_H)
        return;
    SDL_GetWindowPosition(sWindow, &x, &y);
    {
        int cw, ch;
        SDL_GetWindowSize(sWindow, &cw, &ch);
        x += (cw - w) / 2;   // grow / shrink around the top middle
    }
    ClampWindowPos(display, &x, &y, w, h);
    SDL_SetWindowSize(sWindow, w, h);
    SDL_SetWindowPosition(sWindow, x, y);
}

static SDL_Texture *PanelTex(SDL_Renderer *ren, SDL_Texture **tex, Uint32 format, int w, int h)
{
    if (*tex == NULL)
    {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        *tex = SDL_CreateTexture(ren, format, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (*tex)
            SDL_SetTextureBlendMode(*tex, format == SDL_PIXELFORMAT_ARGB8888 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    }
    return *tex;
}

// A card's frame and label into a pair of textures; TRUE when anything changed
static int UploadCard(SDL_Renderer *ren, SDL_Texture **frameTex, SDL_Texture **labelTex, int *hasFrame,
                      const struct DisplayPanelCard *c)
{
    int changed = 0;
    if (c->frame && (c->frameChanged || !*hasFrame))
    {
        if (PanelTex(ren, frameTex, SDL_PIXELFORMAT_ABGR1555, GBA_W, GBA_H))
        {
            SDL_UpdateTexture(*frameTex, NULL, c->frame, GBA_W * 2);
            *hasFrame = 1;
            changed = 1;
        }
    }
    else if (!c->frame && *hasFrame)
    {
        *hasFrame = 0;
        changed = 1;
    }
    if (c->label && (c->labelChanged || *labelTex == NULL))
    {
        if (PanelTex(ren, labelTex, SDL_PIXELFORMAT_ARGB8888, DISPLAY_PANEL_LABEL_W, DISPLAY_PANEL_LABEL_H))
        {
            SDL_UpdateTexture(*labelTex, NULL, c->label, DISPLAY_PANEL_LABEL_W * 4);
            changed = 1;
        }
    }
    return changed;
}

// ---------------------------------------------------------------------------
// Pop-out windows

static void DestroyPopout(struct Popout *p)
{
    if (p->frame)
        SDL_DestroyTexture(p->frame);
    if (p->label)
        SDL_DestroyTexture(p->label);
    if (p->renderer)
        SDL_DestroyRenderer(p->renderer);
    if (p->window)
        SDL_DestroyWindow(p->window);
    memset(p, 0, sizeof(*p));
}

static void DestroyAllPopouts(void)
{
    int i;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
        if (sPopouts[i].used)
            DestroyPopout(&sPopouts[i]);
}

static struct Popout *FindPopout(int id)
{
    int i;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
        if (sPopouts[i].used && sPopouts[i].id == id)
            return &sPopouts[i];
    return NULL;
}

static struct Popout *OpenPopout(const struct DisplayPanelCard *c, int nth)
{
    int i, x = SDL_WINDOWPOS_CENTERED, y = SDL_WINDOWPOS_CENTERED, s = Clamp(c->scale, 1, 4);
    struct Popout *p = NULL;
    for (i = 0; i < DISPLAY_PANEL_MAX && p == NULL; i++)
        if (!sPopouts[i].used)
            p = &sPopouts[i];
    if (p == NULL)
        return NULL;
    if (sWindow)
    {
        // beside the game's window, on its right (one step down for each)
        int wx, wy, ww, wh;
        SDL_GetWindowPosition(sWindow, &wx, &wy);
        SDL_GetWindowSize(sWindow, &ww, &wh);
        x = wx + ww + 16 + nth * 24;
        y = wy + nth * 24;
        ClampWindowPos(WindowDisplayIndex(), &x, &y, CardW(s), CardH(s));
    }
    p->window = SDL_CreateWindow(c->title ? c->title : "Moemon Co-op", x, y, CardW(s), CardH(s),
                                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (p->window == NULL)
    {
        printf("[display] pop-out window: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_SetWindowMinimumSize(p->window, CardW(1), CardH(1));
    // no vsync here: the game's window keeps the pace
    p->renderer = SDL_CreateRenderer(p->window, -1, SDL_RENDERER_ACCELERATED);
    if (p->renderer == NULL)
        p->renderer = SDL_CreateRenderer(p->window, -1, SDL_RENDERER_SOFTWARE);
    if (p->renderer == NULL)
    {
        printf("[display] pop-out renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(p->window);
        p->window = NULL;
        return NULL;
    }
    p->used = 1;
    p->id = c->id;
    p->scale = s;
    p->dirty = 1;
    p->windowId = SDL_GetWindowID(p->window);
    printf("[display] pop-out window for card %d (%dx)\n", c->id, s);
    return p;
}

static void DrawCard(SDL_Renderer *ren, SDL_Texture *frame, int hasFrame, SDL_Texture *label, unsigned a, int x, int y, int s);

static void DrawPopout(struct Popout *p)
{
    int ow = 0, oh = 0, s, cw, ch;
    if (SDL_GetRendererOutputSize(p->renderer, &ow, &oh) != 0 || ow <= 0 || oh <= 0)
        return;
    SDL_SetRenderDrawColor(p->renderer, 17, 19, 26, 255);
    SDL_RenderClear(p->renderer);
    // the biggest whole size that fits the window (at least 1x)
    for (s = 8; s > 1 && (CardW(s) > ow || CardH(s) > oh); s--)
        ;
    p->shown = s;
    cw = CardW(s);
    ch = CardH(s);
    DrawCard(p->renderer, p->frame, p->hasFrame, p->label, p->accent, (ow - cw) / 2, (oh - ch) / 2, s);
}

static void PresentPopouts(void)
{
    int i;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
        if (sPopouts[i].used && sPopouts[i].dirty)
        {
            DrawPopout(&sPopouts[i]);
            SDL_RenderPresent(sPopouts[i].renderer);
            sPopouts[i].dirty = 0;
        }
}

// The pop-out cards: windows open for the new ones, close for the ones gone or back in the panel
static void SyncPopouts(int count, const struct DisplayPanelCard *cards)
{
    int i, j, nth = 0;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
    {
        int keep = 0;
        if (!sPopouts[i].used)
            continue;
        for (j = 0; j < count; j++)
            if (cards[j].popout && cards[j].id == sPopouts[i].id)
                keep = 1;
        if (!keep)
        {
            printf("[display] pop-out window for card %d closed\n", sPopouts[i].id);
            DestroyPopout(&sPopouts[i]);
        }
    }
    for (j = 0; j < count; j++)
    {
        struct Popout *p;
        if (!cards[j].popout)
            continue;
        p = FindPopout(cards[j].id);
        if (p == NULL)
            p = OpenPopout(&cards[j], nth);
        nth++;
        if (p == NULL)
            continue;
        if (p->scale != Clamp(cards[j].scale, 1, 4))
        {
            // a new size: the window takes it
            p->scale = Clamp(cards[j].scale, 1, 4);
            SDL_SetWindowSize(p->window, CardW(p->scale), CardH(p->scale));
            p->dirty = 1;
        }
        if (p->accent != cards[j].accent)
            p->dirty = 1;
        p->accent = cards[j].accent;
        if (cards[j].title)
            SDL_SetWindowTitle(p->window, cards[j].title);
        if (UploadCard(p->renderer, &p->frame, &p->label, &p->hasFrame, &cards[j]))
            p->dirty = 1;
    }
}

int Display_TakeClosedPopout(void)
{
    int id, i;
    if (sClosedCount == 0)
        return -1;
    id = sClosedPopouts[0];
    for (i = 1; i < sClosedCount; i++)
        sClosedPopouts[i - 1] = sClosedPopouts[i];
    sClosedCount--;
    return id;
}

// A pop-out window's own events; TRUE when the event was one of theirs
static int PopoutEvent(const SDL_Event *e)
{
    int i;
    if (e->type != SDL_WINDOWEVENT)
        return 0;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
    {
        struct Popout *p = &sPopouts[i];
        if (!p->used || e->window.windowID != p->windowId)
            continue;
        if (e->window.event == SDL_WINDOWEVENT_CLOSE)
        {
            // closing it stops showing that friend's screen (mp_spectate.c takes the id)
            if (sClosedCount < (int)(sizeof(sClosedPopouts) / sizeof(sClosedPopouts[0])))
                sClosedPopouts[sClosedCount++] = p->id;
            printf("[display] pop-out window for card %d closed by the player\n", p->id);
            DestroyPopout(p);
        }
        else
            p->dirty = 1;
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------

void Display_SetPanel(int count, const struct DisplayPanelCard *cards)
{
    int i, n = 0, changed;
    if (count < 0)
        count = 0;
    if (count > DISPLAY_PANEL_MAX)
        count = DISPLAY_PANEL_MAX;
    SyncPopouts(count, cards);
    // the cards that stay in the window
    for (i = 0, changed = 0; i < count; i++)
    {
        int side = Clamp(cards[i].side, 0, DISPLAY_SIDE_COUNT - 1), req = Clamp(cards[i].scale, 1, 4);
        if (cards[i].popout)
            continue;
        if (n >= sPanelCount || sPanelSide[n] != side || sPanelReq[n] != req)
            changed = 1;
        if (n < sPanelCount && sPanelId[n] != cards[i].id)
        {
            // someone else in this spot: their own picture, not the last one's
            sPanelHasFrame[n] = 0;
            changed = 1;
        }
        n++;
    }
    if (n != sPanelCount)
        changed = 1;
    if (n > 0 && sPanelCount == 0 && sWindow)
    {
        // Coming up: remember the window as it was, and the game's whole scale in it
        Uint32 f = SDL_GetWindowFlags(sWindow);
        SDL_GetWindowSize(sWindow, &sPanelSavedW, &sPanelSavedH);
        sPanelHadWindow = !(f & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED));
        sPanelOwnK = Clamp(sPanelSavedW / GBA_W < sPanelSavedH / GBA_H ? sPanelSavedW / GBA_W : sPanelSavedH / GBA_H,
                           1, DISPLAY_MAX_WINDOW_SCALE);
    }
    sGivenCount = count;
    for (i = 0, n = 0; i < count; i++)
    {
        sGivenId[i] = cards[i].id;
        sGivenWhere[i] = cards[i].popout ? -1 : n;
        if (cards[i].popout)
            continue;
        sPanelSide[n] = Clamp(cards[i].side, 0, DISPLAY_SIDE_COUNT - 1);
        sPanelReq[n] = Clamp(cards[i].scale, 1, 4);
        sPanelId[n] = cards[i].id;
        n++;
    }
    if (changed)
    {
        sPanelCount = n;
        FitWindowToPanel();
    }
    if (sRenderer == NULL)
        return;
    for (i = 0, n = 0; i < count; i++)
    {
        if (cards[i].popout)
            continue;
        sPanelAccent[n] = cards[i].accent;
        if (UploadCard(sRenderer, &sPanelFrameTex[n], &sPanelLabelTex[n], &sPanelHasFrame[n], &cards[i]))
            sDirty = 1;
        n++;
    }
    for (; n < DISPLAY_PANEL_MAX; n++)
        sPanelHasFrame[n] = 0;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
        if (sPopouts[i].used && sPopouts[i].dirty)
            sDirty = 1;   // Display_Present draws them
}

// The size card i (in the order given to Display_SetPanel) is shown at now: smaller than asked when it doesn't fit
int Display_GetCardScale(int i)
{
    int w = 0, h = 0, n;
    struct PanelLayout L;
    if (i < 0 || i >= sGivenCount)
        return 0;
    if (sGivenWhere[i] < 0)
    {
        // a pop-out: the size its window shows it at
        struct Popout *p = FindPopout(sGivenId[i]);
        return p ? (p->shown ? p->shown : p->scale) : 0;
    }
    n = sGivenWhere[i];
    if (n >= sPanelCount)
        return 0;
    if (sRenderer && SDL_GetRendererOutputSize(sRenderer, &w, &h) == 0 && w > 0 && h > 0)
        PanelLayout(PanelFitScale(w, h), &L);
    else
        PanelLayout(sPanelFit, &L);
    return L.scale[n];
}

static void PanelFillRectOn(SDL_Renderer *ren, int x, int y, int w, int h, int r, int g, int b, int a)
{
    SDL_Rect rc;
    rc.x = x, rc.y = y, rc.w = w, rc.h = h;
    SDL_SetRenderDrawColor(ren, r, g, b, a);
    SDL_RenderFillRect(ren, &rc);
}

static void PanelFillRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
    PanelFillRectOn(sRenderer, x, y, w, h, r, g, b, a);
}

// The panel's background color at row y of h (a soft vertical gradient)
static void PanelBg(int y, int h, int *r, int *g, int *b)
{
    *r = 25 - 11 * y / (h > 1 ? h : 1);
    *g = 28 - 12 * y / (h > 1 ? h : 1);
    *b = 37 - 15 * y / (h > 1 ? h : 1);
}

// One card: a soft shadow, the outline in the player's color (corners cut), the screen, the label strip
static void DrawCard(SDL_Renderer *ren, SDL_Texture *frame, int hasFrame, SDL_Texture *label, unsigned a, int x, int y, int s)
{
    int ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF, c, cardW = CardW(s), cardH = CardH(s);
    SDL_Rect dst;
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    PanelFillRectOn(ren, x + s, y + 2 * s, cardW, cardH, 0, 0, 0, 120);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
    PanelFillRectOn(ren, x + s, y, cardW - 2 * s, cardH, ar, ag, ab, 255);
    PanelFillRectOn(ren, x, y + s, cardW, cardH - 2 * s, ar, ag, ab, 255);
    dst.x = x + s;
    dst.y = y + s;
    dst.w = GBA_W * s;
    dst.h = GBA_H * s;
    if (hasFrame && frame)
        SDL_RenderCopy(ren, frame, NULL, &dst);
    else
        PanelFillRectOn(ren, dst.x, dst.y, dst.w, dst.h, 11, 13, 18, 255);
    PanelFillRectOn(ren, x + s, y + s + GBA_H * s, GBA_W * s, DISPLAY_PANEL_LABEL_H * s, 24, 27, 36, 255);
    c = s / 2 > 0 ? s / 2 : 1;
    PanelFillRectOn(ren, x + s, y + s + GBA_H * s, GBA_W * s, c, ar / 2 + 12, ag / 2 + 13, ab / 2 + 18, 255);
    if (label)
    {
        dst.y = y + s + GBA_H * s;
        dst.h = DISPLAY_PANEL_LABEL_H * s;
        SDL_RenderCopy(ren, label, NULL, &dst);
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
}

// Where each group's box sits in an outW x outH output: rows the whole width at the top / bottom, columns between them
static void GroupRect(const struct PanelLayout *L, int side, int outW, int outH, SDL_Rect *r)
{
    int top = L->h[DISPLAY_SIDE_ABOVE], bottom = L->h[DISPLAY_SIDE_BELOW];
    switch (side)
    {
    case DISPLAY_SIDE_ABOVE: r->x = 0; r->y = 0; r->w = outW; r->h = top; break;
    case DISPLAY_SIDE_BELOW: r->x = 0; r->y = outH - bottom; r->w = outW; r->h = bottom; break;
    case DISPLAY_SIDE_LEFT:  r->x = 0; r->y = top; r->w = L->w[side]; r->h = outH - top - bottom; break;
    default:                 r->x = outW - L->w[side]; r->y = top; r->w = L->w[side]; r->h = outH - top - bottom; break;
    }
}

static SDL_Rect sLastCardRect[DISPLAY_PANEL_MAX];   // where the last frame drew each card (tests)

static void DrawPanel(const struct PanelLayout *L, int outW, int outH)
{
    int side, i, y;
    SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_NONE);
    for (side = 0; side < DISPLAY_SIDE_COUNT; side++)
    {
        SDL_Rect g;
        int row = side == DISPLAY_SIDE_BELOW || side == DISPLAY_SIDE_ABOVE, m = L->maxScale[side], pos, total = 0, n = 0;
        if (L->w[side] == 0)
            continue;
        GroupRect(L, side, outW, outH, &g);
        for (y = 0; y < g.h; y++)
        {
            int r, gg, b;
            PanelBg(y, g.h, &r, &gg, &b);
            PanelFillRect(g.x, g.y + y, g.w, 1, r, gg, b, 255);
        }
        // a hairline on the side facing the game, lighter, then a dark one
        switch (side)
        {
        case DISPLAY_SIDE_BELOW: PanelFillRect(g.x, g.y, g.w, 1, 58, 64, 82, 255);
                                 PanelFillRect(g.x, g.y + 1, g.w, 1, 10, 11, 15, 255); break;
        case DISPLAY_SIDE_ABOVE: PanelFillRect(g.x, g.y + g.h - 1, g.w, 1, 58, 64, 82, 255);
                                 PanelFillRect(g.x, g.y + g.h - 2, g.w, 1, 10, 11, 15, 255); break;
        case DISPLAY_SIDE_LEFT:  PanelFillRect(g.x + g.w - 1, g.y, 1, g.h, 58, 64, 82, 255);
                                 PanelFillRect(g.x + g.w - 2, g.y, 1, g.h, 10, 11, 15, 255); break;
        default:                 PanelFillRect(g.x, g.y, 1, g.h, 58, 64, 82, 255);
                                 PanelFillRect(g.x + 1, g.y, 1, g.h, 10, 11, 15, 255); break;
        }
        // the cards, centered along the group
        for (i = 0; i < sPanelCount; i++)
            if (sPanelSide[i] == side)
            {
                total += row ? CardW(L->scale[i]) : CardH(L->scale[i]);
                n++;
            }
        total += (n - 1) * PANEL_GAP * m;
        pos = (row ? g.x + (g.w - total) / 2 : g.y + (g.h - total) / 2);
        for (i = 0; i < sPanelCount; i++)
        {
            int s = L->scale[i], x, yy;
            if (sPanelSide[i] != side)
                continue;
            if (row)
            {
                x = pos;
                yy = g.y + (g.h - CardH(s)) / 2;
                pos += CardW(s) + PANEL_GAP * m;
            }
            else
            {
                x = g.x + (g.w - CardW(s)) / 2;
                yy = pos;
                pos += CardH(s) + PANEL_GAP * m;
            }
            DrawCard(sRenderer, sPanelFrameTex[i], sPanelHasFrame[i], sPanelLabelTex[i], sPanelAccent[i], x, yy, s);
            sLastCardRect[i].x = x;
            sLastCardRect[i].y = yy;
            sLastCardRect[i].w = CardW(s);
            sLastCardRect[i].h = CardH(s);
        }
    }
    SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_NONE);
}

static SDL_Rect sLastGameRect;   // where the last frame drew the game (tests)
static int sLastPanelH;

static void RenderToBackbuffer(void)
{
    int outW = 0, outH = 0, kx, ky, exact, filterOn, fullW, fullH, gameX = 0, gameY = 0, srcW = GBA_W, srcH = GBA_H;
    SDL_Rect dst;
    SDL_Texture *gameTex;
    int panelH = 0;
    struct PanelLayout L;

    sFilterShown = 0;
    // Moemon Kai: the card screens are the game picture while they're open (src/tcg/tcg_ui.c)
    gameTex = sGameTex ? Tcg_Canvas(sRenderer, sGameTex) : NULL;
    if (gameTex == NULL)
        gameTex = sGameTex;
    SDL_SetRenderTarget(sRenderer, NULL);
    if (sPanelCount > 0)
        SDL_SetRenderDrawColor(sRenderer, 17, 19, 26, 255);   // beside the game: the panel's slate, darker
    else
        SDL_SetRenderDrawColor(sRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sRenderer);
    if (sGameTex == NULL || SDL_GetRendererOutputSize(sRenderer, &outW, &outH) != 0 || outW <= 0 || outH <= 0)
        return;

    // Moemon Co-op: the spectating panel's groups take the sides they're on; the game fits in the middle
    fullW = outW;
    fullH = outH;
    if (sPanelCount > 0)
    {
        sPanelFit = PanelFitScale(outW, outH);
        PanelLayout(sPanelFit, &L);
        gameX = L.w[DISPLAY_SIDE_LEFT];
        gameY = L.h[DISPLAY_SIDE_ABOVE];
        outW -= L.w[DISPLAY_SIDE_LEFT] + L.w[DISPLAY_SIDE_RIGHT];
        outH -= L.h[DISPLAY_SIDE_ABOVE] + L.h[DISPLAY_SIDE_BELOW];
        if (outW < 1)
            outW = 1;
        if (outH < 1)
            outH = 1;
        panelH = L.h[DISPLAY_SIDE_BELOW];
    }
    ComputeDstRect(outW, outH, &dst);
    dst.x += gameX;
    dst.y += gameY;
    // Moemon Fight: a fight is the game picture, at its own 480x320 (src/platform/fight_platform.c), scaled the same
    // way as the game's 240x160 in every mode (whole multiples, sharp-bilinear between them) with the same filters
    {
        SDL_Texture *fight = Fight_Canvas(sRenderer);
        if (fight)
        {
            gameTex = fight;
            srcW = FIGHT_CANVAS_W;
            srcH = FIGHT_CANVAS_H;
        }
    }
    kx = CeilDiv(dst.w, srcW);
    ky = CeilDiv(dst.h, srcH);
    exact = (dst.w == srcW * kx && dst.h == srcH * ky);
    if (!exact)
    {
        kx = Clamp(kx, 1, MAX_PRESCALE);
        ky = Clamp(ky, 1, MAX_PRESCALE);
        while (sMaxTexW > 0 && kx > 1 && srcW * kx > sMaxTexW)
            kx--;
        while (sMaxTexH > 0 && ky > 1 && srcH * ky > sMaxTexH)
            ky--;
    }

    // Filters need a few screen pixels per game pixel to look right.
    if (sCfg.filter == DISPLAY_FILTER_OFF)
        filterOn = 0;
    else if (exact)
        filterOn = (sCfg.filter == DISPLAY_FILTER_SCANLINES) ? (ky >= 2) : (kx >= 3 && ky >= 3);
    else
        filterOn = (dst.w >= srcW * 3 && dst.h >= srcH * 3);
    if (filterOn)
        filterOn = EnsureOverlay(kx, ky, srcW, srcH);

    if (exact)
    {
        SDL_RenderCopy(sRenderer, gameTex, NULL, &dst);
        if (filterOn)
            SDL_RenderCopy(sRenderer, sOverlayTex, NULL, &dst);
    }
    else if (sTargetsSupported && EnsureScaleTex(kx, ky, srcW, srcH) && SDL_SetRenderTarget(sRenderer, sScaleTex) == 0)
    {
        // Sharp-bilinear: whole-number nearest enlarge, then linear to the output rect.
        SDL_RenderCopy(sRenderer, gameTex, NULL, NULL);
        if (filterOn)
            SDL_RenderCopy(sRenderer, sOverlayTex, NULL, NULL);
        SDL_SetRenderTarget(sRenderer, NULL);
        SDL_RenderCopy(sRenderer, sScaleTex, NULL, &dst);
    }
    else
    {
        SDL_RenderCopy(sRenderer, gameTex, NULL, &dst);
        filterOn = 0;
    }
    sFilterShown = filterOn;
    sLastGameRect = dst;
    sLastPanelH = panelH;
    if (sPanelCount > 0)
        DrawPanel(&L, fullW, fullH);   // last: drawing to a render target above may not keep it
}

static void ClearToBlack(void)
{
    SDL_SetRenderDrawColor(sRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sRenderer);
    SDL_RenderPresent(sRenderer);
}

// ---------------------------------------------------------------------------
// Platform API (sdl2.c)
// ---------------------------------------------------------------------------

void Display_PreInit(void)
{
    const char *env = getenv("MOEMON_DISPLAY_CONFIG");

    if (env && env[0])
    {
        snprintf(sConfigPath, sizeof(sConfigPath), "%s", env);
    }
    else
    {
        char *base = SDL_GetBasePath();
        if (base)
        {
            snprintf(sConfigPath, sizeof(sConfigPath), "%s%s", base, CONFIG_NAME);
            SDL_free(base);
        }
        // Automated tests use the defaults and never write the user's file.
        if (getenv("MOEMON_TEST"))
            sPersist = 0;
    }
    if (sPersist)
        LoadConfig();

    // Alt-tabbing out of borderless fullscreen must not minimize the game.
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#ifdef _WIN32
    {
        // Without DPI awareness Windows stretches the window with a blurry
        // filter on 125%/150% scaled desktops.
        SDL_version v;
        SDL_GetVersion(&v);
#ifdef SDL_HINT_WINDOWS_DPI_AWARENESS
        SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif
        if (SDL_VERSIONNUM(v.major, v.minor, v.patch) < SDL_VERSIONNUM(2, 24, 0))
            SetProcessDPIAware();
        timeBeginPeriod(1); // accurate SDL_Delay(1) for frame pacing with vsync off
    }
#endif
}

int Display_Init(const char *title)
{
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    int x, y, display;

    ValidateWindowedGeometry();
    display = (sCfg.winX != POS_UNSET) ? DisplayContainingPoint(sCfg.winX + sCfg.winW / 2, sCfg.winY + 8) : 0;
    if (display < 0)
        display = 0;
    x = (sCfg.winX != POS_UNSET) ? sCfg.winX : (int)SDL_WINDOWPOS_CENTERED_DISPLAY(display);
    y = (sCfg.winY != POS_UNSET) ? sCfg.winY : (int)SDL_WINDOWPOS_CENTERED_DISPLAY(display);

    sWindow = SDL_CreateWindow(title, x, y, sCfg.winW, sCfg.winH,
                               flags | (sCfg.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    if (sWindow == NULL && sCfg.fullscreen)
    {
        sCfg.fullscreen = 0;
        sWindow = SDL_CreateWindow(title, x, y, sCfg.winW, sCfg.winH, flags);
    }
    if (sWindow == NULL)
    {
        printf("[display] could not create window: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowMinimumSize(sWindow, GBA_W, GBA_H);
    {
        SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormatFrom((void *)sAppIcon, APP_ICON_SIZE, APP_ICON_SIZE, 32,
                                                               APP_ICON_SIZE * 4, SDL_PIXELFORMAT_RGBA32);
        if (icon)
        {
            SDL_SetWindowIcon(sWindow, icon);
            SDL_FreeSurface(icon);
        }
    }
    if (CreateRenderer() != 0)
        return -1;

    // Black from the very first frame (no white flash).
    ClearToBlack();
    SDL_ShowWindow(sWindow);
    if (sCfg.fullscreen)
        sFsWasMaximized = sCfg.maximized;
    else if (sCfg.maximized)
        SDL_MaximizeWindow(sWindow);
    ClearToBlack();
    sToggleTicks = SDL_GetTicks();
    sDirty = 1;
    printf("[display] %s, %dx window, %s, filter %s, vsync %s (%s)\n",
           sCfg.fullscreen ? "fullscreen" : "windowed", sCfg.windowScale, sScaleNames[sCfg.scaleMode],
           sFilterNames[sCfg.filter], sCfg.vsync ? "on" : "off", sConfigPath);
    return 0;
}

void Display_Shutdown(void)
{
    Tcg_DestroyTextures();
    Fight_DestroyTextures();
    DestroyAllPopouts();
    DestroyTextures();
    if (sRenderer)
        SDL_DestroyRenderer(sRenderer);
    if (sWindow)
        SDL_DestroyWindow(sWindow);
    sRenderer = NULL;
    sWindow = NULL;
#ifdef _WIN32
    timeEndPeriod(1);
#endif
}

int Display_HandleEvent(const SDL_Event *e)
{
    switch (e->type)
    {
    case SDL_KEYDOWN:
    {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_F11 || ((k == SDLK_RETURN || k == SDLK_KP_ENTER) && (e->key.keysym.mod & KMOD_ALT)))
        {
            if (!e->key.repeat)
            {
                Display_ToggleFullscreen();
                Display_SaveSettings();
            }
            return 1; // Alt+Enter must not press START
        }
        break;
    }
    case SDL_WINDOWEVENT:
        if (PopoutEvent(e))
            return 1;
        sDirty = 1;
        // With pop-outs open SDL only quits when every window is closed: closing the game's window quits
        if (e->window.event == SDL_WINDOWEVENT_CLOSE && sWindow && e->window.windowID == SDL_GetWindowID(sWindow))
        {
            SDL_Event q;
            int i, any = 0;
            for (i = 0; i < DISPLAY_PANEL_MAX; i++)
                any |= sPopouts[i].used;
            if (any)
            {
                memset(&q, 0, sizeof(q));
                q.type = SDL_QUIT;
                SDL_PushEvent(&q);
            }
        }
        switch (e->window.event)
        {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_MOVED:
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
            SnapshotWindowed(0);
            break;
        }
        break;
    case SDL_RENDER_TARGETS_RESET:
        sDirty = 1;
        break;
    case SDL_RENDER_DEVICE_RESET:
        // All textures are lost; make them again (the pop-outs open again with the next cards).
        DestroyAllPopouts();
        DestroyTextures();
        if (sRenderer)
            CreateGameTexture();
        sDirty = 1;
        break;
    }
    return 0;
}

void Display_UploadFrame(const void *pixels)
{
    sLastFrame = pixels;
    if (sGameTex)
        SDL_UpdateTexture(sGameTex, NULL, pixels, GBA_W * 2);
    sDirty = 1;
}

// The monitor's refresh rate in Hz (0 if unknown)
int Display_GetRefreshRate(void)
{
    SDL_DisplayMode mode;
    int display;
    if (sWindow == NULL)
        return 0;
    display = SDL_GetWindowDisplayIndex(sWindow);
    if (display < 0 || SDL_GetCurrentDisplayMode(display, &mode) != 0)
        return 0;
    return mode.refresh_rate;
}

int Display_NeedsPresent(void)
{
    return sDirty;
}

void Display_Present(void)
{
    if (sRenderer == NULL)
        return;
    RenderToBackbuffer();
    SDL_RenderPresent(sRenderer);
    PresentPopouts();
    sDirty = 0;
}

// Test harness: each pop-out window's picture to PREFIX_<card id>.bmp
void Display_SavePopoutBMPs(const char *prefix)
{
    int i;
    for (i = 0; i < DISPLAY_PANEL_MAX; i++)
    {
        struct Popout *p = &sPopouts[i];
        int w = 0, h = 0;
        SDL_Surface *surf;
        char path[1100];
        if (!p->used)
            continue;
        DrawPopout(p);
        SDL_GetRendererOutputSize(p->renderer, &w, &h);
        surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
        snprintf(path, sizeof(path), "%s_%d.bmp", prefix, p->id);
        if (surf)
        {
            int ok = SDL_RenderReadPixels(p->renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surf->pixels, surf->pitch) == 0
                  && SDL_SaveBMP(surf, path) == 0;
            printf("[display] popshot %s %dx%d %s\n", path, w, h, ok ? "ok" : "FAILED");
            SDL_FreeSurface(surf);
        }
        SDL_RenderPresent(p->renderer);
    }
}

int Display_SaveOutputBMP(const char *path)
{
    int w = 0, h = 0, ok = -1;
    SDL_Surface *surf;

    if (sRenderer == NULL)
        return -1;
    RenderToBackbuffer();
    SDL_GetRendererOutputSize(sRenderer, &w, &h);
    surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (surf)
    {
        if (SDL_RenderReadPixels(sRenderer, NULL, SDL_PIXELFORMAT_ARGB8888, surf->pixels, surf->pitch) == 0)
            ok = SDL_SaveBMP(surf, path);
        SDL_FreeSurface(surf);
    }
    SDL_RenderPresent(sRenderer);
    sDirty = 0;
    return ok;
}

void Display_TestSetWindowSize(int w, int h)
{
    if (sWindow)
        SDL_SetWindowSize(sWindow, w, h);
    sDirty = 1;
}

void Display_TestCommand(const char *key, int value)
{
    if (strcmp(key, "fullscreen") == 0)   Display_SetFullscreen(value);
    else if (strcmp(key, "scale") == 0)   Display_SetWindowScale(value);
    else if (strcmp(key, "mode") == 0)    Display_SetScaleMode(value);
    else if (strcmp(key, "filter") == 0)  Display_SetFilter(value);
    else if (strcmp(key, "vsync") == 0)   Display_SetVSync(value);
    else if (strcmp(key, "save") == 0)    Display_SaveSettings();
    else if (strcmp(key, "toggle") == 0)  Display_ToggleFullscreen();
    else if (strcmp(key, "closepopout") == 0)
    {
        // closepopout ID: the player clicks the close button of card ID's pop-out window
        struct Popout *p = FindPopout(value);
        if (p)
        {
            SDL_Event e;
            memset(&e, 0, sizeof(e));
            e.type = SDL_WINDOWEVENT;
            e.window.event = SDL_WINDOWEVENT_CLOSE;
            e.window.windowID = p->windowId;
            SDL_PushEvent(&e);
        }
    }
    else if (strcmp(key, "info") == 0 && sWindow && sRenderer)
    {
        static const char *const sides[DISPLAY_SIDE_COUNT] = {"below", "above", "left", "right"};
        int ww, wh, ow = 0, oh = 0, i, big = 0;
        struct PanelLayout L;
        RenderToBackbuffer();   // the layout as it is now (headless runs only draw for screenshots)
        SDL_GetWindowSize(sWindow, &ww, &wh);
        SDL_GetRendererOutputSize(sRenderer, &ow, &oh);
        PanelLayout(sPanelFit, &L);
        for (i = 0; i < sPanelCount; i++)
            if (L.scale[i] > big)
                big = L.scale[i];
        // "cards Nx": the biggest card in the window; then each card (side, size, where) and each pop-out
        printf("[display] info: window %dx%d output %dx%d game %d,%d %dx%d panel %d cards %dx (strip %d)",
               ww, wh, ow, oh, sLastGameRect.x, sLastGameRect.y, sLastGameRect.w, sLastGameRect.h,
               sPanelCount, big, sLastPanelH);
        for (i = 0; i < sPanelCount; i++)
            printf(" | card %d %s %dx at %d,%d %dx%d", sPanelId[i], sides[sPanelSide[i]], L.scale[i], sLastCardRect[i].x,
                   sLastCardRect[i].y, sLastCardRect[i].w, sLastCardRect[i].h);
        for (i = 0; i < DISPLAY_PANEL_MAX; i++)
            if (sPopouts[i].used)
            {
                int pw = 0, ph = 0;
                SDL_GetWindowSize(sPopouts[i].window, &pw, &ph);
                printf(" | popout %d window %dx%d", sPopouts[i].id, pw, ph);
            }
        printf("\n");
    }
}

// ---------------------------------------------------------------------------
// Settings API (game code)
// ---------------------------------------------------------------------------

int Display_GetFullscreen(void)
{
    return sCfg.fullscreen;
}

void Display_SetFullscreen(int on)
{
    Uint32 f;
    int isFs;

    on = on ? 1 : 0;
    if (sWindow == NULL)
    {
        sCfg.fullscreen = on;
        return;
    }
    f = SDL_GetWindowFlags(sWindow);
    isFs = (f & SDL_WINDOW_FULLSCREEN) != 0;
    if (on && !isFs)
    {
        SnapshotWindowed(1);
        sFsWasMaximized = (f & SDL_WINDOW_MAXIMIZED) != 0;
        if (SDL_SetWindowFullscreen(sWindow, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
        {
            printf("[display] fullscreen failed: %s\n", SDL_GetError());
            on = 0;
        }
    }
    else if (!on && isFs)
    {
        SDL_SetWindowFullscreen(sWindow, 0);
        if (sFsWasMaximized)
        {
            SDL_MaximizeWindow(sWindow);
        }
        else
        {
            SDL_SetWindowSize(sWindow, sCfg.winW, sCfg.winH);
            if (sCfg.winX != POS_UNSET && sCfg.winY != POS_UNSET)
                SDL_SetWindowPosition(sWindow, sCfg.winX, sCfg.winY);
            if (sPanelCount > 0)
            {
                // Moemon Co-op: the spectating panel is up: this window is the one to go back to when it goes, and it
                // grows to fit the panel now
                sPanelSavedW = sCfg.winW;
                sPanelSavedH = sCfg.winH;
                sPanelHadWindow = 1;
                sPanelOwnK = Clamp(sCfg.winW / GBA_W < sCfg.winH / GBA_H ? sCfg.winW / GBA_W : sCfg.winH / GBA_H,
                                   1, DISPLAY_MAX_WINDOW_SCALE);
                FitWindowToPanel();
            }
        }
    }
    sToggleTicks = SDL_GetTicks();
    sCfg.fullscreen = on;
    sDirty = 1;
}

void Display_ToggleFullscreen(void)
{
    Display_SetFullscreen(!sCfg.fullscreen);
}

int Display_GetWindowScale(void)
{
    if (sWindow)
    {
        Uint32 f = SDL_GetWindowFlags(sWindow);
        if (!(f & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_MINIMIZED)))
        {
            // Reflect manual resizing: the biggest whole scale the window holds.
            int w, h, s;
            SDL_GetWindowSize(sWindow, &w, &h);
            if (sPanelCount > 0)
            {
                struct PanelLayout L;
                PanelLayout(sPanelFit, &L);
                w -= L.w[DISPLAY_SIDE_LEFT] + L.w[DISPLAY_SIDE_RIGHT];
                h -= L.h[DISPLAY_SIDE_ABOVE] + L.h[DISPLAY_SIDE_BELOW];
            }
            s = w / GBA_W;
            if (h / GBA_H < s)
                s = h / GBA_H;
            sCfg.windowScale = Clamp(s, 1, DISPLAY_MAX_WINDOW_SCALE);
        }
    }
    return sCfg.windowScale;
}

int Display_GetMaxWindowScale(void)
{
    return MaxScaleOnDisplay(WindowDisplayIndex());
}

void Display_SetWindowScale(int scale)
{
    int display = WindowDisplayIndex();
    int w, h, x, y, cw, ch;
    Uint32 f;

    scale = Clamp(scale, 1, MaxScaleOnDisplay(display));
    sCfg.windowScale = scale;
    w = GBA_W * scale;
    h = GBA_H * scale;
    if (sPanelCount > 0 && sWindow && !(SDL_GetWindowFlags(sWindow) & SDL_WINDOW_FULLSCREEN))
    {
        // The game at this scale with the panel under it (the window without it comes back when it goes)
        sPanelOwnK = scale;
        sPanelSavedW = w;
        sPanelSavedH = h;
        sPanelHadWindow = 1;
        sCfg.winW = w;
        sCfg.winH = h;
        if (SDL_GetWindowFlags(sWindow) & SDL_WINDOW_MAXIMIZED)
            SDL_RestoreWindow(sWindow);
        FitWindowToPanel();
        return;
    }
    if (sWindow == NULL)
    {
        sCfg.winW = w;
        sCfg.winH = h;
        return;
    }
    f = SDL_GetWindowFlags(sWindow);
    if ((f & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED)) || sCfg.winX == POS_UNSET)
    {
        // Base it on the remembered windowed geometry
        cw = sCfg.winW;
        ch = sCfg.winH;
        x = sCfg.winX;
        y = sCfg.winY;
        if (x == POS_UNSET || y == POS_UNSET)
        {
            // Never placed by us: center on the monitor
            SDL_Rect ub;
            GetUsableBounds(display, &ub);
            cw = w;
            ch = h;
            x = ub.x + (ub.w - w) / 2;
            y = ub.y + (ub.h - h) / 2;
        }
    }
    else
    {
        SDL_GetWindowSize(sWindow, &cw, &ch);
        SDL_GetWindowPosition(sWindow, &x, &y);
    }
    // Grow/shrink around the window's center and keep it on screen
    x += (cw - w) / 2;
    y += (ch - h) / 2;
    ClampWindowPos(display, &x, &y, w, h);
    sCfg.winW = w;
    sCfg.winH = h;
    sCfg.winX = x;
    sCfg.winY = y;
    sCfg.maximized = 0;
    sFsWasMaximized = 0;
    sDirty = 1;
    if (f & SDL_WINDOW_FULLSCREEN)
        return; // used when leaving fullscreen
    if (f & SDL_WINDOW_MAXIMIZED)
        SDL_RestoreWindow(sWindow);
    SDL_SetWindowSize(sWindow, w, h);
    SDL_SetWindowPosition(sWindow, x, y);
}

int Display_GetScaleMode(void)
{
    return sCfg.scaleMode;
}

void Display_SetScaleMode(int mode)
{
    sCfg.scaleMode = Clamp(mode, 0, DISPLAY_SCALE_COUNT - 1);
    sDirty = 1;
}

int Display_GetFilter(void)
{
    return sCfg.filter;
}

void Display_SetFilter(int filter)
{
    sCfg.filter = Clamp(filter, 0, DISPLAY_FILTER_COUNT - 1);
    sDirty = 1;
}

int Display_IsFilterShown(void)
{
    return sFilterShown;
}

int Display_GetVSync(void)
{
    return sCfg.vsync;
}

void Display_SetVSync(int on)
{
    sCfg.vsync = on ? 1 : 0;
    if (sRenderer == NULL)
        return;
#if SDL_VERSION_ATLEAST(2, 0, 18)
    if (SDL_RenderSetVSync(sRenderer, sCfg.vsync) == 0)
    {
        sDirty = 1;
        return;
    }
#endif
    RecreateRenderer();
}

void Display_SaveSettings(void)
{
    SnapshotWindowed(0);
    if (sWindow)
        Display_GetWindowScale();
    WriteConfig();
}

#else // !PLATFORM_SDL2: settings only, nothing to apply

static int sFullscreen, sWindowScale = 3, sScaleMode = DISPLAY_SCALE_SHARP_FIT, sFilter, sVSync = 1;

int Display_GetFullscreen(void) { return sFullscreen; }
void Display_SetFullscreen(int on) { sFullscreen = on ? 1 : 0; }
void Display_ToggleFullscreen(void) { sFullscreen = !sFullscreen; }
int Display_GetWindowScale(void) { return sWindowScale; }
int Display_GetMaxWindowScale(void) { return DISPLAY_MAX_WINDOW_SCALE; }
void Display_SetWindowScale(int scale) { sWindowScale = scale; }
int Display_GetScaleMode(void) { return sScaleMode; }
void Display_SetScaleMode(int mode) { sScaleMode = mode; }
int Display_GetFilter(void) { return sFilter; }
void Display_SetFilter(int filter) { sFilter = filter; }
int Display_IsFilterShown(void) { return sFilter != DISPLAY_FILTER_OFF; }
int Display_GetVSync(void) { return sVSync; }
void Display_SetVSync(int on) { sVSync = on ? 1 : 0; }
void Display_SaveSettings(void) { }
void Display_SetPanel(int count, int scale, const struct DisplayPanelCard *cards) { (void)count; (void)scale; (void)cards; }
int Display_GetPanelScale(void) { return 0; }

#endif // PLATFORM_SDL2
