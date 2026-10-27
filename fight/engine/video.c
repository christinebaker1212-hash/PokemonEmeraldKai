/*
 * SDL video mode, window and draw parameter helpers (original module
 * 0x4627E0-0x4635EF).
 *
 * The three Coord helpers at the start do not touch any video state; they
 * may be a separate unit that the linker placed between the palette module
 * and this one.
 *
 * Video_SetMode keeps two sizes: the game resolution (g_videoGame*) and the
 * video mode (g_videoWidth/Height), which only differ with the OpenGL
 * renderer, where the game is scaled to the window.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "mugen.h"

/* ---- coordinates ---- */

/*
 * The original keeps unit ratios on the x87 stack, where their exponent range
 * is wider than a double's; they are recomputed in each expression here
 * instead of being stored in a double variable.
 */

/* 0x004627E0: the value in units of new_unit: x, y and z scaled by unit / new_unit. */
Coord *Coord_Rescale(Coord *out, double x, double y, double z, double unit, int type, int pad, double new_unit)
{
    Coord c;

    c.x = unit / new_unit * x;
    c.y = unit / new_unit * y;
    c.z = unit / new_unit * z;
    c.unit = new_unit;
    c.type = type;
    c.pad_24 = pad;
    *out = c;
    return out;
}

/* 0x00462820: a + b, with b first converted to the unit of a. The result keeps b's type. */
Coord *Coord_Add(Coord *out, double ax, double ay, double az, double aunit, int atype, int apad, double bx,
                 double by, double bz, double bunit, int btype, int bpad)
{
    Coord c;

    c.x = bunit / aunit * bx;
    c.y = by * (bunit / aunit);
    c.z = bunit / aunit * bz;
    c.unit = aunit;
    c.type = btype;
    c.pad_24 = bpad;
    *out = c;
    out->x = out->x + ax;
    out->y = ay + out->y;
    out->z = az + out->z;
    return out;
}

/*
 * 0x004628A0: a 2D value with unit 0. The original leaves pad_24 as stack
 * garbage; it is zeroed here.
 */
Coord *Coord_Make(Coord *out, double x, double y)
{
    Coord c;

    c.x = x;
    c.y = y;
    c.z = 0.0;
    c.unit = 0.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/* ---- video mode ---- */

/* 0x004628E0: allocations made before the video mode is set; 0 sets LIBERR_GRAPHICS. */
int Video_PreInit(void)
{
    g_videoScratch = malloc(0x1000);
    if (g_videoScratch == NULL || !SinTable_Init()) {
        g_libraryError = LIBERR_GRAPHICS;
        return 0;
    }
    return PalFx_InitGlobal() != 0;
}

/*
 * 0x00462920: always returns 1. g_unusedSurface1, g_unusedSurface2 and g_sinTable are
 * freed but not cleared.
 */
int Video_Deinit(void)
{
    if (g_renderMode == RENDER_OPENGL || g_renderMode == RENDER_OPENGL_SCREEN) {
        Gl_Deinit();
    }
    if (g_unusedSurface1 != NULL) {
        SDL_FreeSurface(g_unusedSurface1);
    }
    if (g_unusedSurface2 != NULL) {
        SDL_FreeSurface(g_unusedSurface2);
    }
    if (g_videoScratch != NULL) {
        free(g_videoScratch);
    }
    g_videoScratch = NULL;
    if (g_sinTable != NULL) {
        free(g_sinTable);
    }
    return 1;
}

/* Bytes per pixel of a video depth. */
static int Video_BytesPerPixel(int depth)
{
    switch (depth) {
    case 8:
        return 1;
    case 24:
        return 3;
    case 32:
        return 4;
    default:
        return 2;
    }
}

/*
 * 0x00462990: sets the video mode; flags are VIDEO_*. Only the OpenGL
 * renderer uses width x height: the others open a window of the game size.
 * The System renderer retries without hardware surfaces; with OpenGL a
 * failed SDL_SetVideoMode is not noticed here. Returns 0 on failure
 * (LIBERR_VIDEO_MODE when no mode could be set).
 */
int Video_SetMode(int game_width, int game_height, int width, int height, int depth, unsigned int flags)
{
    Uint32 sdl_flags;
    SDL_Surface *surface;
    int bytes;

    if (g_renderMode != RENDER_OPENGL) {
        width = game_width;
        height = game_height;
    }
    if (g_unusedSurface1 != NULL) {
        SDL_FreeSurface(g_unusedSurface1);
    }
    if (g_unusedSurface2 != NULL) {
        SDL_FreeSurface(g_unusedSurface2);
    }
    g_unusedSurface2 = NULL;
    g_unusedSurface1 = NULL;
    g_screenSurface = NULL;
    g_unused504230 = 0;
    g_unused504234 = 0;
    if (g_renderMode == RENDER_OPENGL || g_renderMode == RENDER_OPENGL_SCREEN) {
        sdl_flags = SDL_OPENGL;
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, (flags & VIDEO_HWBLIT) ? 1 : 0);
        if ((flags & VIDEO_RESIZABLE) && !(flags & VIDEO_FULLSCREEN)) {
            sdl_flags = SDL_OPENGL | SDL_RESIZABLE;
        }
        sdl_flags |= SDL_HWSURFACE | VIDEO_SDL_HWACCEL;
        if (g_vretrace) {
            SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
        }
    } else {
        sdl_flags = SDL_HWSURFACE;
    }
    if (flags & VIDEO_FULLSCREEN) {
        sdl_flags |= SDL_FULLSCREEN;
    }
    if (flags & VIDEO_HWBLIT) {
        sdl_flags |= SDL_DOUBLEBUF;
    }
    if (!(flags & VIDEO_FULLSCREEN)) {
        SDL_putenv("SDL_VIDEO_WINDOW_POS=center");
        SDL_putenv("SDL_VIDEO_CENTERED=1");
    }
    if (flags & VIDEO_DIRECTX) {
        SDL_putenv("SDL_VIDEODRIVER=directx");
    }
    surface = SDL_SetVideoMode(width, height, depth, sdl_flags);
    g_videoSurface = surface;
    if (surface == NULL && g_renderMode != RENDER_OPENGL && g_renderMode != RENDER_OPENGL_SCREEN) {
        sdl_flags &= ~(SDL_HWSURFACE | VIDEO_SDL_HWACCEL);
        surface = SDL_SetVideoMode(width, height, depth, sdl_flags);
        g_videoSurface = surface;
        if (surface == NULL) {
            g_libraryError = LIBERR_VIDEO_MODE;
            return 0;
        }
    }
    g_videoGameWidth = game_width;
    g_videoGameHeight = game_height;
    g_videoGamePixels = game_width * game_height;
    g_videoFlags = flags;
    g_videoSdlFlags = sdl_flags;
    g_videoWidth = width;
    g_videoHeight = height;
    g_videoDepth = depth;
    bytes = Video_BytesPerPixel(depth);
    g_videoBytesPerPixel = bytes;
    g_targetWidth = width;
    g_targetHeight = height;
    g_targetBpp = depth;
    g_targetBytesPerPixel = bytes;
    g_screenSurface = surface;
    g_unused50424C = 0;
    SDL_ShowCursor((sdl_flags & SDL_FULLSCREEN) ? SDL_DISABLE : SDL_ENABLE);
    if (g_renderMode == RENDER_OPENGL_SCREEN) {
        if (!Gl_Init(0)) {
            return 0;
        }
    } else if (g_renderMode == RENDER_OPENGL) {
        if (!Gl_Init(1)) {
            return 0;
        }
    }
    Gfx_SetRenderTarget(NULL, 0);
    return 1;
}

/*
 * 0x00462C50: SDL_VIDEORESIZE. The System renderer takes the new size as the
 * game size (the screen buffer keeps its own); OpenGLScreen resets the
 * projection and clears the window to red (sic: glClearColor(1, 0, 0, 0)).
 */
void Video_OnResize(int width, int height)
{
    MatchLog_Printf(9, "Video resize event %ix%i\n", width, height);
    switch (g_renderMode) {
    case RENDER_SYSTEM: {
        SDL_Surface *surface = SDL_SetVideoMode(width, height, g_videoDepth, g_videoSdlFlags);
        g_videoGameWidth = width;
        g_videoSurface = surface;
        g_screenSurface = surface;
        g_videoGameHeight = height;
        g_videoGamePixels = width * height;
        break;
    }
    case RENDER_OPENGL_SCREEN:
        g_videoSurface = SDL_SetVideoMode(width, height, g_videoDepth, g_videoSdlFlags);
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        gluOrtho2D(0.0, (double)width, (double)height, 0.0);
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        g_glScreenResized = 1;
        g_screenSurface = g_videoSurface;
        break;
    case RENDER_OPENGL:
        g_videoWidth = width;
        g_videoHeight = height;
        break;
    }
}

/*
 * 0x0040B9D0 (stored with the command line code): Alt+Enter toggles
 * [Video] FullScreen. Only the OpenGL renderer switches the window, and only
 * when the video mode was not set full screen; the key is then consumed.
 */
void Video_HandleFullscreenKey(int *key)
{
    GameVars *g;
    int mode;

    if (!g_keyState[SDLK_LALT] && !g_keyState[SDLK_RALT]) {
        return;
    }
    if (*key != SDLK_RETURN) {
        return;
    }
    g = g_game;
    g->fullscreen = g->fullscreen == 0;
    Log_Printf("Fullscreen toggle %i\n", g->fullscreen);
    if (g_renderMode != RENDER_OPENGL || (g_videoFlags & VIDEO_FULLSCREEN)) {
        return;
    }
    if (!Video_SetFullscreen(g_game->fullscreen)) {
        return;
    }
    mode = g_renderMode;
    g = g_game;
    *key = 0;
    if (mode == RENDER_OPENGL || mode == RENDER_OPENGL_SCREEN) {
        g_glScreenResized = 1;
        if (g_bgmInitialized) {
            g_bgmBackgroundMode = (g_videoFlags & VIDEO_FULLSCREEN) ? 0 : g->bgm_background_mode;
        }
    }
    g->frame_skip_count = 0;
    g->skip_draw = 0;
    Timer_ResetFrame();
}

/*
 * 0x00462D80: switches the window between borderless full screen on its
 * monitor and the saved window placement. Returns 0 when SDL has no window.
 */
int Video_SetFullscreen(int fullscreen)
{
    SDL_SysWMinfo info;
    HWND hwnd;
    HWND after;
    int x, y, w, h;

    memset(&info, 0, sizeof(info));     /* the version is left at 0.0.0 */
    if (!SDL_GetWMInfo(&info) || info.window == NULL) {
        return 0;
    }
    hwnd = info.window;
    if (g_videoFullscreen == fullscreen) {
        return 1;
    }
    if (fullscreen) {
        HMONITOR monitor;
        GetClientRect(hwnd, &g_savedClientRect);
        GetWindowRect(hwnd, &g_savedWindowRect);
        g_savedWindowStyle = GetWindowLongA(hwnd, GWL_STYLE);
        g_savedWindowExStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
        y = 0;
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);
        monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        x = 0;
        if (monitor != NULL) {
            MONITORINFO mi;
            memset(&mi, 0, sizeof(mi));
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfoA(monitor, &mi)) {
                x = mi.rcMonitor.left;
                y = mi.rcMonitor.top;
                w = mi.rcMonitor.right - x;
                h = mi.rcMonitor.bottom - y;
            }
        }
        SetWindowLongA(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_SYSMENU);
        after = HWND_TOP;
    } else {
        x = g_savedWindowRect.left;
        w = g_savedWindowRect.right - x;
        y = g_savedWindowRect.top;
        h = g_savedWindowRect.bottom - y;
        SetWindowLongA(hwnd, GWL_STYLE, g_savedWindowStyle);
        SetWindowLongA(hwnd, GWL_EXSTYLE, g_savedWindowExStyle);
        after = HWND_NOTOPMOST;
    }
    SetWindowPos(hwnd, after, x, y, w, h, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    g_videoFullscreen = fullscreen;
    return 1;
}

/* Sprite slot `index` of a sprite set, as the original computes it (no bounds check). */
static Sprite *Video_SpriteAt(const SpriteSet *set, int index)
{
    const KeyedArray *a = set->sprites;

    return (Sprite *)(a->data + a->elem_size * index);
}

/*
 * 0x00462F10: colour value for the current render target: 0xBBGGRR with the
 * OpenGL renderers, else SDL_MapRGB with the target surface's format. Quirk:
 * g_renderTarget must be set, and a target that is not an SDL surface makes
 * the original read the format of a NULL surface.
 */
unsigned int Video_MapRGB(int r, int g, int b)
{
    const Sprite *spr;
    SDL_Surface *surface;

    if (g_renderMode == RENDER_MODE_GL || g_renderMode == RENDER_MODE_GL_ALT) {
        return ((unsigned int)b << 8 | (unsigned int)g) << 8 | (unsigned int)r;
    }
    spr = Video_SpriteAt(g_renderTarget, g_renderTargetSprite);
    if (spr != NULL && spr->format == SPRITE_FORMAT_SURFACE) {
        surface = spr->pixels;
    } else {
        surface = NULL;
    }
    return SDL_MapRGB(surface->format, (Uint8)r, (Uint8)g, (Uint8)b);
}

/* ---- draw parameters ---- */

/* 0x00462F70: neutral parameters with every DRAW_* flag set. */
DrawParams *DrawParams_Init(DrawParams *dp)
{
    memset(dp, 0, sizeof(*dp));
    dp->scale2_y = 1.0f;
    dp->scale2_x = 1.0f;
    dp->flags = 0xFFFF;
    dp->scale_y = 1.0f;
    dp->masked = 1;
    dp->scale_x = 1.0f;
    dp->alpha_src = 1.0f;
    dp->bottom_stretch = 1.0f;
    dp->top_stretch = 1.0f;
    dp->width_ratio = 1.0f;
    dp->width_anchor = 0.5f;
    return dp;
}

/* -1 for a set flip bit, 1 otherwise. */
static int DrawParams_FlipSign(unsigned int flip, unsigned int bit)
{
    return (flip & bit) ? -1 : 1;
}

/* 0x00462FB0: applies parent's scales and flips to the offset of dp. */
DrawParams *DrawParams_ScalePos(const DrawParams *parent, DrawParams *dp)
{
    unsigned int flags = parent->flags;

    if (flags & DRAW_SCALE) {
        dp->x = parent->scale_x * dp->x;
        dp->y = parent->scale_y * dp->y;
    }
    if (flags & DRAW_SCALE2) {
        dp->x = parent->scale2_x * dp->x;
        dp->y = parent->scale2_y * dp->y;
    }
    if (flags & DRAW_FLIP) {
        unsigned int flip = parent->flip;
        int sx = DrawParams_FlipSign(flip, DRAW_FLIP_H);
        int sy = DrawParams_FlipSign(flip, DRAW_FLIP_V);
        dp->x = (float)sx * dp->x;
        dp->y = (float)sy * dp->y;
    }
    if (parent->flags & DRAW_FLIP2) {
        unsigned int flip = parent->flip2;
        int sx = DrawParams_FlipSign(flip, DRAW_FLIP_H);
        int sy = DrawParams_FlipSign(flip, DRAW_FLIP_V);
        dp->x = (float)sx * dp->x;
        dp->y = (float)sy * dp->y;
    }
    return dp;
}

/*
 * 0x00463070: combines parent into dp: offsets and angles add, flips xor,
 * scales multiply, blending is replaced. A field dp does not have yet starts
 * from its neutral value; quirk: top_stretch / bottom_stretch start at 1 and add up.
 */
DrawParams *DrawParams_Combine(const DrawParams *parent, DrawParams *dp)
{
    if (parent->flags == 0) {
        return dp;
    }
    if (parent->flags & DRAW_POS) {
        if (!(dp->flags & DRAW_POS)) {
            dp->x = 0.0f;
            dp->y = 0.0f;
        }
        dp->x = parent->x + dp->x;
        dp->flags |= DRAW_POS;
        dp->y = parent->y + dp->y;
    }
    if (parent->flags & DRAW_FLIP) {
        if (!(dp->flags & DRAW_FLIP)) {
            dp->flip = 0;
        }
        dp->flip ^= parent->flip;
        dp->flags |= DRAW_FLIP;
    }
    if (parent->flags & DRAW_FLIP2) {
        if (!(dp->flags & DRAW_FLIP2)) {
            dp->flip2 = 0;
        }
        dp->flip2 ^= parent->flip2;
        dp->flags |= DRAW_FLIP2;
    }
    if (parent->flags & DRAW_SCALE) {
        if (!(dp->flags & DRAW_SCALE)) {
            dp->scale_y = 1.0f;
            dp->scale_x = 1.0f;
        }
        dp->scale_x = parent->scale_x * dp->scale_x;
        dp->flags |= DRAW_SCALE;
        dp->scale_y = parent->scale_y * dp->scale_y;
    }
    if (parent->flags & DRAW_SCALE2) {
        if (!(dp->flags & DRAW_SCALE2)) {
            dp->scale2_y = 1.0f;
            dp->scale2_x = 1.0f;
        }
        dp->scale2_x = parent->scale2_x * dp->scale2_x;
        dp->flags |= DRAW_SCALE2;
        dp->scale2_y = parent->scale2_y * dp->scale2_y;
    }
    if (parent->flags & DRAW_ANGLE) {
        if (!(dp->flags & DRAW_ANGLE)) {
            dp->angle = 0.0f;
            dp->angle_y = 0.0f;
            dp->angle_z = 0.0f;
        }
        dp->angle = parent->angle + dp->angle;
        dp->angle_y = parent->angle_y + dp->angle_y;
        dp->flags |= DRAW_ANGLE;
        dp->angle_z = parent->angle_z + dp->angle_z;
    }
    if (parent->flags & DRAW_SHEAR_X) {
        if (!(dp->flags & DRAW_SHEAR_X)) {
            dp->shear_x = 0.0f;
        }
        dp->flags |= DRAW_SHEAR_X;
        dp->shear_x = parent->shear_x + dp->shear_x;
    }
    if (parent->flags & DRAW_SHEAR_Y) {
        if (!(dp->flags & DRAW_SHEAR_Y)) {
            dp->shear_y = 0.0f;
        }
        dp->flags |= DRAW_SHEAR_Y;
        dp->shear_y = parent->shear_y + dp->shear_y;
    }
    if (parent->flags & DRAW_BLEND) {
        dp->blend = parent->blend;
        dp->alpha_src = parent->alpha_src;
        dp->flags |= DRAW_BLEND;
        dp->alpha_dst = parent->alpha_dst;
    }
    if (parent->flags & DRAW_STRETCH) {
        if (!(dp->flags & DRAW_STRETCH)) {
            dp->bottom_offset = 0.0f;
            dp->top_offset = 0.0f;
            dp->bottom_stretch = 1.0f;
            dp->top_stretch = 1.0f;
        }
        dp->top_offset = parent->top_offset + dp->top_offset;
        dp->bottom_offset = parent->bottom_offset + dp->bottom_offset;
        dp->top_stretch = parent->top_stretch + dp->top_stretch;
        dp->flags |= DRAW_STRETCH;
        dp->bottom_stretch = parent->bottom_stretch + dp->bottom_stretch;
    }
    if (parent->flags & DRAW_UNUSED_400) {
        dp->flags |= DRAW_UNUSED_400;
    }
    if (parent->flags & DRAW_UNUSED_1000) {
        dp->flags |= DRAW_UNUSED_1000;
    }
    return dp;
}

/*
 * 0x00463260: turns negative scales and the flips into a rotation (for the
 * software renderer): every mirror changes the angle and toggles the
 * vertical flag of flip2; flip ends up clear (apart from bits other than H
 * and V) and the angle in [0, 360).
 */
void DrawParams_FlipToAngle(DrawParams *dp)
{
    int odd = 0;

    if (dp->scale_x < 0.0f) {
        odd = 1;
        dp->scale_x = (float)(dp->scale_x * -1.0);
        dp->angle = (float)(360.0 - dp->angle);
    }
    if (dp->scale_y < 0.0f) {
        odd = !odd;
        dp->scale_y = (float)(dp->scale_y * -1.0);
        dp->angle = (float)(180.0 - dp->angle);
    }
    if (dp->scale2_x < 0.0f) {
        odd = !odd;
        dp->scale2_x = (float)(dp->scale2_x * -1.0);
    }
    if (dp->scale2_y < 0.0f) {
        odd = !odd;
        dp->scale2_y = (float)(dp->scale2_y * -1.0);
        dp->angle = (float)(dp->angle + 180.0);
    }
    if (dp->flip & DRAW_FLIP_H) {
        dp->flip &= ~DRAW_FLIP_H;
        odd = !odd;
        dp->angle = (float)(360.0 - dp->angle);
    }
    if (dp->flip & DRAW_FLIP_V) {
        odd = !odd;
        dp->angle = (float)(180.0 - dp->angle);
        dp->flip &= ~DRAW_FLIP_V;
    }
    if (dp->flip2 & DRAW_FLIP_V) {
        odd = !odd;
        dp->angle = (float)(180.0 - dp->angle);
        dp->flip2 &= ~DRAW_FLIP_V;
    }
    if (dp->angle < 0.0f) {
        int turns = (int)(dp->angle / -360.0);
        dp->angle = (float)((double)(360 - turns * -360) + dp->angle);
    }
    if (dp->angle >= 360.0f) {
        int turns = (int)(dp->angle / 360.0);
        dp->angle = (float)(dp->angle - (double)(turns * 360));
    }
    if (odd) {
        dp->flip2 ^= DRAW_FLIP_H;
    }
    dp->flags |= DRAW_FLIP2 | DRAW_ANGLE;
}

/* 0x004633F0: makes every scale positive, moving the signs into flip / flip2. */
void DrawParams_FoldScales(DrawParams *dp)
{
    unsigned int flip = 0;
    unsigned int flip2 = 0;

    if (dp->scale_x < 0.0f) {
        flip = DRAW_FLIP_H;
        dp->scale_x = (float)(dp->scale_x * -1.0);
    }
    if (dp->scale_y < 0.0f) {
        flip ^= DRAW_FLIP_V;
        dp->scale_y = (float)(dp->scale_y * -1.0);
    }
    if (dp->scale2_x < 0.0f) {
        flip2 = DRAW_FLIP_H;
        dp->scale2_x = (float)(dp->scale2_x * -1.0);
    }
    if (dp->scale2_y < 0.0f) {
        dp->flip ^= flip;
        flip2 ^= DRAW_FLIP_V;
        dp->flip2 ^= flip2;
        dp->flags |= DRAW_FLIP | DRAW_FLIP2;
        dp->scale2_y = (float)(dp->scale2_y * -1.0);
        return;
    }
    dp->flip ^= flip;
    dp->flip2 ^= flip2;
    dp->flags |= DRAW_FLIP | DRAW_FLIP2;
}

/*
 * 0x00463470: blending from a Trans_Parse result: none, add, subtract, or
 * add with alphas out of 256. Other modes leave dp alone.
 */
DrawParams *DrawParams_SetBlend(int mode, DrawParams *dp, int alpha_src, int alpha_dst)
{
    switch (mode) {
    case TRANS_NONE:
        dp->flags |= DRAW_BLEND;
        dp->alpha_src = 1.0f;
        dp->blend = BLEND_NONE;
        dp->alpha_dst = 0.0f;
        break;
    case 1:                 /* full additive (not a Trans_Parse result) */
        dp->flags |= DRAW_BLEND;
        dp->alpha_src = 1.0f;
        dp->blend = BLEND_ADD;
        dp->alpha_dst = 1.0f;
        break;
    case TRANS_SUB:
        dp->flags |= DRAW_BLEND;
        dp->alpha_src = 1.0f;
        dp->blend = BLEND_SUB;
        dp->alpha_dst = 1.0f;
        break;
    case TRANS_ADD:
        dp->flags |= DRAW_BLEND;
        dp->blend = BLEND_ADD;
        dp->alpha_src = (float)((double)alpha_src * (1.0 / 256));
        dp->alpha_dst = (float)((1.0 / 256) * (double)alpha_dst);
        break;
    }
    return dp;
}

/* ---- surfaces ---- */

/*
 * 0x00463500: a 32-bit RGBA copy of src. Per-surface alpha is turned off for
 * the copy and restored afterwards (with the RLE flag it had).
 */
SDL_Surface *Surface_ToRGBA32(SDL_Surface *src)
{
    SDL_Surface *dst;
    Uint32 flags;
    Uint8 alpha;
    SDL_Rect rect;

    dst = SDL_CreateRGBSurface(0, src->w, src->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    if (dst == NULL) {
        return NULL;
    }
    flags = src->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
    alpha = src->format->alpha;
    if (flags & SDL_SRCALPHA) {
        SDL_SetAlpha(src, 0, 0);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = (Uint16)src->w;
    rect.h = (Uint16)src->h;
    SDL_UpperBlit(src, &rect, dst, &rect);     /* the same rectangle for both */
    if ((flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
        SDL_SetAlpha(src, flags, alpha);
    }
    return dst;
}

/* 0x004635C0: [Video] VRetrace; only the OpenGL renderers pass it to SDL. */
void Video_SetVRetrace(int vretrace)
{
    if (g_vretrace == vretrace) {
        return;
    }
    if (g_renderMode == RENDER_OPENGL || g_renderMode == RENDER_OPENGL_SCREEN) {
        SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vretrace != 0);
    }
    g_vretrace = vretrace;
}
