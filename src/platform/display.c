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
static int sScaleKx, sScaleKy;
static int sOverlayKx, sOverlayKy, sOverlayFilter = -1;
static int sTargetsSupported;
static int sMaxTexW, sMaxTexH;
static const void *sLastFrame;
static int sDirty = 1;
static int sFilterShown;
static int sFsWasMaximized;
static Uint32 sToggleTicks;

// Moemon Co-op spectating panel (Display_SetPanel)
#define PANEL_PAD 6       // around the cards, in game pixels (times the panel's scale)
#define PANEL_GAP 6       // between cards
static int sPanelCount, sPanelReqScale, sPanelScale = 1;
static int sPanelOwnK;    // the game's whole-number scale the window keeps beside the panel
static int sPanelSavedW, sPanelSavedH, sPanelHadWindow;   // the window before the panel resized it
static SDL_Texture *sPanelFrameTex[DISPLAY_PANEL_MAX], *sPanelLabelTex[DISPLAY_PANEL_MAX];
static int sPanelHasFrame[DISPLAY_PANEL_MAX];
static unsigned sPanelAccent[DISPLAY_PANEL_MAX];

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
    DestroyTextures();
    if (sRenderer)
        SDL_DestroyRenderer(sRenderer);
    sRenderer = NULL;
    CreateRenderer();
    sDirty = 1;
}

static int EnsureScaleTex(int kx, int ky)
{
    if (sScaleTex && sScaleKx == kx && sScaleKy == ky)
        return 1;
    if (sScaleTex)
        SDL_DestroyTexture(sScaleTex);
    sScaleTex = CreateTex(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, GBA_W * kx, GBA_H * ky, 1);
    sScaleKx = sScaleKy = 0;
    if (sScaleTex == NULL)
        return 0;
    SDL_SetTextureBlendMode(sScaleTex, SDL_BLENDMODE_NONE);
    sScaleKx = kx;
    sScaleKy = ky;
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

static int EnsureOverlay(int kx, int ky)
{
    int w = GBA_W * kx, h = GBA_H * ky, x, y;
    Uint32 *pixels;
    Uint8 colA[MAX_PRESCALE + 1];

    if (sOverlayTex && sOverlayKx == kx && sOverlayKy == ky && sOverlayFilter == sCfg.filter)
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

// The panel's size in screen pixels for n cards at scale s: each card is the screen and its label inside a
// one-game-pixel outline
static void PanelSize(int n, int s, int *w, int *h)
{
    *w = 2 * PANEL_PAD * s + n * (GBA_W * s + 2 * s) + (n - 1) * PANEL_GAP * s;
    *h = 2 * PANEL_PAD * s + 2 * s + GBA_H * s + DISPLAY_PANEL_LABEL_H * s;
}

// The biggest scale up to the one asked whose panel fits an outW x outH output with room for the game at 1x
static int PanelFitScale(int outW, int outH)
{
    int s, w, h;
    for (s = sPanelReqScale; s > 1; s--)
    {
        PanelSize(sPanelCount, s, &w, &h);
        if (w <= outW && outH - h >= GBA_H)
            break;
    }
    return s < 1 ? 1 : s;
}

// The window's size for the panel at scale s with the game at whole scale k
static void PanelWindowSize(int s, int k, int *w, int *h)
{
    int pw, ph;
    PanelSize(sPanelCount, s, &pw, &ph);
    *w = GBA_W * k > pw ? GBA_W * k : pw;
    *h = GBA_H * k + ph;
}

// A window (not fullscreen or maximized) takes the size the panel needs: the game keeps its scale, or gets
// smaller, then the panel does, until both fit the monitor
static void FitWindowToPanel(void)
{
    int display, k, s, w = 0, h = 0, x, y, t, l, b, r, minW, minH;
    SDL_Rect ub;
    Uint32 f;
    if (sWindow == NULL)
        return;
    sDirty = 1;
    f = SDL_GetWindowFlags(sWindow);
    if (sPanelCount > 0)
    {
        PanelWindowSize(1, 1, &minW, &minH);
        SDL_SetWindowMinimumSize(sWindow, minW, minH);
    }
    else
        SDL_SetWindowMinimumSize(sWindow, GBA_W, GBA_H);
    if (f & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED))
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
        for (s = sPanelReqScale; s >= 1; s--)
        {
            for (k = sPanelOwnK; k >= 1; k--)
            {
                PanelWindowSize(s, k, &w, &h);
                if (w <= ub.w - l - r && h <= ub.h - t - b)
                    break;
            }
            if (k >= 1)
                break;
        }
        if (s < 1)
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

static SDL_Texture *PanelTex(SDL_Texture **tex, Uint32 format, int w, int h)
{
    if (*tex == NULL)
    {
        *tex = CreateTex(format, SDL_TEXTUREACCESS_STREAMING, w, h, 0);
        if (*tex)
            SDL_SetTextureBlendMode(*tex, format == SDL_PIXELFORMAT_ARGB8888 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    }
    return *tex;
}

void Display_SetPanel(int count, int scale, const struct DisplayPanelCard *cards)
{
    int i, changed;
    if (count < 0)
        count = 0;
    if (count > DISPLAY_PANEL_MAX)
        count = DISPLAY_PANEL_MAX;
    scale = Clamp(scale, 1, 4);
    if (count > 0 && sPanelCount == 0 && sWindow)
    {
        // Coming up: remember the window as it was, and the game's whole scale in it
        Uint32 f = SDL_GetWindowFlags(sWindow);
        SDL_GetWindowSize(sWindow, &sPanelSavedW, &sPanelSavedH);
        sPanelHadWindow = !(f & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED));
        sPanelOwnK = Clamp(sPanelSavedW / GBA_W < sPanelSavedH / GBA_H ? sPanelSavedW / GBA_W : sPanelSavedH / GBA_H,
                           1, DISPLAY_MAX_WINDOW_SCALE);
    }
    changed = (count != sPanelCount || (count > 0 && scale != sPanelReqScale));
    sPanelReqScale = scale;
    if (changed)
    {
        sPanelCount = count;
        FitWindowToPanel();
    }
    if (sRenderer == NULL)
        return;
    for (i = 0; i < count; i++)
    {
        sPanelAccent[i] = cards[i].accent;
        if (cards[i].frame && (cards[i].frameChanged || !sPanelHasFrame[i]))
        {
            if (PanelTex(&sPanelFrameTex[i], SDL_PIXELFORMAT_ABGR1555, GBA_W, GBA_H))
            {
                SDL_UpdateTexture(sPanelFrameTex[i], NULL, cards[i].frame, GBA_W * 2);
                sPanelHasFrame[i] = 1;
                sDirty = 1;
            }
        }
        else if (!cards[i].frame && sPanelHasFrame[i])
        {
            sPanelHasFrame[i] = 0;
            sDirty = 1;
        }
        if (cards[i].label && cards[i].labelChanged)
        {
            if (PanelTex(&sPanelLabelTex[i], SDL_PIXELFORMAT_ARGB8888, DISPLAY_PANEL_LABEL_W, DISPLAY_PANEL_LABEL_H))
            {
                SDL_UpdateTexture(sPanelLabelTex[i], NULL, cards[i].label, DISPLAY_PANEL_LABEL_W * 4);
                sDirty = 1;
            }
        }
    }
    for (; i < DISPLAY_PANEL_MAX; i++)
        sPanelHasFrame[i] = 0;
}

// The size the panel's cards really have (smaller than asked when that doesn't fit the window)
int Display_GetPanelScale(void)
{
    int w = 0, h = 0;
    if (sPanelCount == 0)
        return 0;
    if (sRenderer && SDL_GetRendererOutputSize(sRenderer, &w, &h) == 0 && w > 0 && h > 0)
        return PanelFitScale(w, h);
    return sPanelScale;
}

static void FillRect(int x, int y, int w, int h, int r, int g, int b, int a)
{
    SDL_Rect rc;
    rc.x = x, rc.y = y, rc.w = w, rc.h = h;
    SDL_SetRenderDrawColor(sRenderer, r, g, b, a);
    SDL_RenderFillRect(sRenderer, &rc);
}

// The panel's background color at row y of h (a soft vertical gradient)
static void PanelBg(int y, int h, int *r, int *g, int *b)
{
    *r = 25 - 11 * y / (h > 1 ? h : 1);
    *g = 28 - 12 * y / (h > 1 ? h : 1);
    *b = 37 - 15 * y / (h > 1 ? h : 1);
}

static void DrawPanel(int outW, int y0, int panelH, int s)
{
    int pw, ph, i, y, x, cardW = GBA_W * s + 2 * s, cardH = GBA_H * s + DISPLAY_PANEL_LABEL_H * s + 2 * s;
    PanelSize(sPanelCount, s, &pw, &ph);
    SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_NONE);
    for (y = 0; y < panelH; y++)
    {
        int r, g, b;
        PanelBg(y, panelH, &r, &g, &b);
        FillRect(0, y0 + y, outW, 1, r, g, b, 255);
    }
    // a hairline on top, lighter, then a dark one
    FillRect(0, y0, outW, 1, 58, 64, 82, 255);
    if (panelH > 4)
        FillRect(0, y0 + 1, outW, 1, 10, 11, 15, 255);
    x = (outW - pw) / 2 + PANEL_PAD * s;
    y = y0 + (panelH - cardH) / 2;
    for (i = 0; i < sPanelCount; i++, x += cardW + PANEL_GAP * s)
    {
        unsigned a = sPanelAccent[i];
        int ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF, c;
        SDL_Rect dst;
        // soft shadow
        SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_BLEND);
        FillRect(x + s, y + 2 * s, cardW, cardH, 0, 0, 0, 120);
        SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_NONE);
        // outline in the player's color, its corners cut (rounded at this size)
        FillRect(x + s, y, cardW - 2 * s, cardH, ar, ag, ab, 255);
        FillRect(x, y + s, cardW, cardH - 2 * s, ar, ag, ab, 255);
        // the screen
        dst.x = x + s;
        dst.y = y + s;
        dst.w = GBA_W * s;
        dst.h = GBA_H * s;
        if (sPanelHasFrame[i] && sPanelFrameTex[i])
            SDL_RenderCopy(sRenderer, sPanelFrameTex[i], NULL, &dst);
        else
            FillRect(dst.x, dst.y, dst.w, dst.h, 11, 13, 18, 255);
        // the label strip: dark, the player's color faintly along its top
        FillRect(x + s, y + s + GBA_H * s, GBA_W * s, DISPLAY_PANEL_LABEL_H * s, 24, 27, 36, 255);
        c = s / 2 > 0 ? s / 2 : 1;
        FillRect(x + s, y + s + GBA_H * s, GBA_W * s, c, ar / 2 + 12, ag / 2 + 13, ab / 2 + 18, 255);
        if (sPanelLabelTex[i])
        {
            dst.y = y + s + GBA_H * s;
            dst.h = DISPLAY_PANEL_LABEL_H * s;
            SDL_RenderCopy(sRenderer, sPanelLabelTex[i], NULL, &dst);
        }
    }
    SDL_SetRenderDrawBlendMode(sRenderer, SDL_BLENDMODE_NONE);
}

static SDL_Rect sLastGameRect;   // where the last frame drew the game (tests)
static int sLastPanelH;

static void RenderToBackbuffer(void)
{
    int outW = 0, outH = 0, kx, ky, exact, filterOn;
    SDL_Rect dst;
    int panelH = 0;

    sFilterShown = 0;
    SDL_SetRenderTarget(sRenderer, NULL);
    if (sPanelCount > 0)
        SDL_SetRenderDrawColor(sRenderer, 17, 19, 26, 255);   // beside the game: the panel's slate, darker
    else
        SDL_SetRenderDrawColor(sRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sRenderer);
    if (sGameTex == NULL || SDL_GetRendererOutputSize(sRenderer, &outW, &outH) != 0 || outW <= 0 || outH <= 0)
        return;

    // Moemon Co-op: the spectating panel takes the bottom; the game fits above it
    if (sPanelCount > 0)
    {
        int pw;
        sPanelScale = PanelFitScale(outW, outH);
        PanelSize(sPanelCount, sPanelScale, &pw, &panelH);
        if (panelH > outH - 1)
            panelH = outH - 1;
        outH -= panelH;
    }
    ComputeDstRect(outW, outH, &dst);
    kx = CeilDiv(dst.w, GBA_W);
    ky = CeilDiv(dst.h, GBA_H);
    exact = (dst.w == GBA_W * kx && dst.h == GBA_H * ky);
    if (!exact)
    {
        kx = Clamp(kx, 1, MAX_PRESCALE);
        ky = Clamp(ky, 1, MAX_PRESCALE);
        while (sMaxTexW > 0 && kx > 1 && GBA_W * kx > sMaxTexW)
            kx--;
        while (sMaxTexH > 0 && ky > 1 && GBA_H * ky > sMaxTexH)
            ky--;
    }

    // Filters need a few screen pixels per game pixel to look right.
    if (sCfg.filter == DISPLAY_FILTER_OFF)
        filterOn = 0;
    else if (exact)
        filterOn = (sCfg.filter == DISPLAY_FILTER_SCANLINES) ? (ky >= 2) : (kx >= 3 && ky >= 3);
    else
        filterOn = (dst.w >= GBA_W * 3 && dst.h >= GBA_H * 3);
    if (filterOn)
        filterOn = EnsureOverlay(kx, ky);

    if (exact)
    {
        SDL_RenderCopy(sRenderer, sGameTex, NULL, &dst);
        if (filterOn)
            SDL_RenderCopy(sRenderer, sOverlayTex, NULL, &dst);
    }
    else if (sTargetsSupported && EnsureScaleTex(kx, ky) && SDL_SetRenderTarget(sRenderer, sScaleTex) == 0)
    {
        // Sharp-bilinear: whole-number nearest enlarge, then linear to the output rect.
        SDL_RenderCopy(sRenderer, sGameTex, NULL, NULL);
        if (filterOn)
            SDL_RenderCopy(sRenderer, sOverlayTex, NULL, NULL);
        SDL_SetRenderTarget(sRenderer, NULL);
        SDL_RenderCopy(sRenderer, sScaleTex, NULL, &dst);
    }
    else
    {
        SDL_RenderCopy(sRenderer, sGameTex, NULL, &dst);
        filterOn = 0;
    }
    sFilterShown = filterOn;
    sLastGameRect = dst;
    sLastPanelH = panelH;
    if (panelH > 0)
        DrawPanel(outW, outH, panelH, sPanelScale);   // last: drawing to a render target above may not keep it
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
        sDirty = 1;
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
        // All textures are lost; make them again.
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
    sDirty = 0;
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
    else if (strcmp(key, "info") == 0 && sWindow && sRenderer)
    {
        int ww, wh, ow = 0, oh = 0, pw = 0, ph = 0;
        RenderToBackbuffer();   // the layout as it is now (headless runs only draw for screenshots)
        SDL_GetWindowSize(sWindow, &ww, &wh);
        SDL_GetRendererOutputSize(sRenderer, &ow, &oh);
        if (sPanelCount > 0)
            PanelSize(sPanelCount, sPanelScale, &pw, &ph);
        printf("[display] info: window %dx%d output %dx%d game %d,%d %dx%d panel %d cards %dx (%dx%d, strip %d)\n",
               ww, wh, ow, oh, sLastGameRect.x, sLastGameRect.y, sLastGameRect.w, sLastGameRect.h,
               sPanelCount, sPanelCount ? sPanelScale : 0, pw, ph, sLastPanelH);
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
                int pw, ph;
                PanelSize(sPanelCount, sPanelScale, &pw, &ph);
                h -= ph;
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
