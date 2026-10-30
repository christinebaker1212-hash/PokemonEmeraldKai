/*
 * Graphics core (original module 0x46B7A0-0x46C26F): fades, the screen
 * buffer, render targets and the clip rectangle, presenting the screen
 * buffer, and the selection of the renderer function tables.
 *
 * The render target is a sprite of a sprite set (an SDL surface, or an
 * OpenGL "vbuf" with the OpenGL renderer), or the video surface itself when
 * g_renderTarget is NULL. g_target* and g_clip* describe it to the drawing
 * code.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL.h>
#include <GL/gl.h>

#include "mugen.h"

#define GL_FRAMEBUFFER_EXT 0x8D40

/* Sprite slot `index` of a sprite set, as the original computes it (no bounds check). */
static Sprite *Gfx_SpriteAt(const SpriteSet *set, int index)
{
    const KeyedArray *a = set->sprites;

    return (Sprite *)(a->data + a->elem_size * index);
}

/* ---- fades ---- */

/*
 * 0x0046B7A0: updates the progress of a fade (and the global PalFX of a
 * palette fade); advance counts the tick. A finished fade is reset.
 */
void Fade_Update(Fade *fade, int advance)
{
    float progress;

    if (fade->type == FADE_NONE) {
        return;
    }
    if (fade->ticks >= fade->time || fade->time == 0) {
        fade->bar_size = 0;
        fade->progress = 0.0f;
        fade->type = FADE_NONE;
        fade->unused_04 = 0x100;
        fade->ticks = 0;
        fade->time = 0;
        fade->out = 0;
        return;
    }
    progress = (float)((double)fade->ticks / (double)fade->time);
    if (!fade->out) {
        progress = (float)(1.0 - progress);
    }
    fade->progress = progress;
    if (fade->type == FADE_PALETTE) {
        int mul = (int)((1.0 - progress) * 256.0);
        PalFx_ScaleGlobalMul(mul, mul, mul);
    }
    if (advance) {
        fade->ticks++;
    }
}

/*
 * 0x0046B830: draws a FADE_BARS fade: black bars from the top and bottom
 * edges, followed by 12 bands of bar_size / 12 pixels that get lighter.
 */
void Fade_DrawBars(const Fade *fade)
{
    int bar = (int)((double)(g_targetHeight / 2 + fade->bar_size) * fade->progress - (double)fade->bar_size);
    int step;
    int alpha;

    g_gfxFillRect(0, 0, g_targetWidth, bar, 0);
    g_gfxFillRect(0, g_targetHeight - bar, g_targetWidth, g_targetHeight, 0);
    step = fade->bar_size / 12;
    for (alpha = 0; alpha < 12 * 0x100; alpha += 0x100) {
        int next = bar + step;
        g_gfxBlendRect(0, bar, g_targetWidth, next - 1, 0, 0x100, alpha / 12);
        g_gfxBlendRect(0, g_targetHeight - next, g_targetWidth, g_targetHeight - bar - 1, 0, 0x100, alpha / 12);
        bar = next;
    }
}

/* ---- screen buffer and render targets ---- */

/*
 * 0x0046B930: a sprite set holding one width x height sprite to render to:
 * an SDL surface in the video surface's format, or an OpenGL vbuf. Quirk:
 * the set leaks when a KeyedArray cannot be created.
 */
SpriteSet *Screen_Create(int depth, int width, int height)
{
    SpriteSet *set;
    Sprite spr;
    int i;

    set = calloc(1, sizeof(SpriteSet));
    if (set == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    set->sprites = KeyedArray_Create(1, sizeof(Sprite));
    if (set->sprites == NULL) {
        return NULL;
    }
    set->palettes = KeyedArray_Create(1, sizeof(Palette));
    if (set->palettes == NULL) {
        return NULL;
    }
    for (i = 0; i < 1; i++) {
        if (g_renderMode == RENDER_OPENGL) {
            void *vbuf = GlVbuf_Alloc(depth, width, height);
            if (!Sprite_Init(SPRITE_INIT_OWN, &spr, SPRITE_FORMAT_TEXTURE, vbuf, 0x1C, width, height, depth, 0, 0)) {
                SpriteSet_Free(set);
                return NULL;
            }
        } else {
            const SDL_PixelFormat *fmt = g_videoSurface->format;
            SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, depth, fmt->Rmask, fmt->Gmask, fmt->Bmask,
                                                        fmt->Amask);
            if (surface == NULL) {
                SpriteSet_Free(set);
                return NULL;
            }
            SDL_SetClipRect(surface, NULL);
            if (!Sprite_Init(SPRITE_INIT_OWN, &spr, SPRITE_FORMAT_SURFACE, surface, sizeof(SDL_Surface), width, height,
                             depth, 0, 0)) {
                SpriteSet_Free(set);
                return NULL;
            }
        }
        if (!SpriteSet_AddSprite(set, &spr, 0, i)) {
            SpriteSet_Free(set);
            return NULL;
        }
    }
    return set;
}

/*
 * 0x0046BA90: renders to sprite `index` of target, or to the screen when
 * target is NULL, and resets the clip rectangle. Returns 0 (possibly with
 * the target globals already changed) when the sprite cannot be a target.
 */
int Gfx_SetRenderTarget(SpriteSet *target, int index)
{
    int changed = !(target == g_renderTarget && index == g_renderTargetSprite);
    Sprite *spr;

    if (target == NULL) {
        g_targetWidth = g_videoWidth;
        g_targetHeight = g_videoHeight;
        g_renderTarget = NULL;
        g_renderTargetSprite = 0;
        g_targetBpp = g_videoDepth;
        g_targetPixels = 0;
        g_targetBytesPerPixel = g_videoBytesPerPixel;
        g_clipRowSkip = 0;
        if (g_renderMode == RENDER_OPENGL && changed) {
            g_glewBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            g_glBoundFramebuffer = 0;
            glViewport(0, 0, g_videoWidth, g_videoHeight);
            Gl_SetupView();
        }
        Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);
        return 1;
    }
    spr = Gfx_SpriteAt(target, index);
    if (spr == NULL || spr->format < 0 || (spr->format > SPRITE_FORMAT_SURFACE && spr->format != SPRITE_FORMAT_TEXTURE)) {
        return 0;
    }
    g_renderTarget = target;
    g_targetWidth = spr->w;
    g_targetHeight = spr->h;
    g_targetPixels = spr->w * spr->h;
    g_renderTargetSprite = index;
    g_targetBpp = spr->bpp;
    g_targetBytesPerPixel = (spr->bpp + 7) >> 3;
    g_clipRowSkip = 0;
    if (spr->format == SPRITE_FORMAT_TEXTURE && (spr->pixels == NULL || !GlVbuf_Bind(spr->pixels))) {
        return 0;
    }
    Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);
    return 1;
}

/* 0x0046BC10: the SDL surface of a sprite, or NULL if it is not one. */
SDL_Surface *SpriteSet_GetSurface(const SpriteSet *set, int sprite)
{
    const Sprite *spr = Gfx_SpriteAt(set, sprite);

    if (spr != NULL && spr->format == SPRITE_FORMAT_SURFACE) {
        return spr->pixels;
    }
    return NULL;
}

/*
 * 0x0046BC30: the screen as an SDL surface (screenshots): a new 32-bit copy
 * of the captured screen with the OpenGL renderers, else the render target's
 * own surface (NULL for none).
 */
SDL_Surface *Gfx_GetScreenSurface(void)
{
    if (g_renderMode == RENDER_MODE_GL || g_renderMode == RENDER_MODE_GL_ALT) {
        void *pixels = g_gfxCapture();
        SDL_Surface *surface;
        if (pixels == NULL) {
            return NULL;
        }
        surface = SDL_CreateRGBSurface(0, g_targetWidth, g_targetHeight, 32, 0x000000FF, 0x0000FF00, 0x00FF0000,
                                       0xFF000000);
        if (surface == NULL) {
            free(pixels);
            return NULL;
        }
        memcpy(surface->pixels, pixels, g_targetWidth * g_targetHeight * 4);
        free(pixels);
        return surface;
    }
    if (g_renderTarget != NULL) {
        const Sprite *spr = Gfx_SpriteAt(g_renderTarget, g_renderTargetSprite);
        if (spr != NULL && spr->format == SPRITE_FORMAT_SURFACE) {
            return spr->pixels;
        }
    }
    return NULL;
}

/* Smallest power of two >= n, computed as the original does (0 for n <= 1). */
static int Gfx_NextPow2(int n)
{
    unsigned int x = (unsigned int)n - 1;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x == 0 ? 0 : (int)(x + 1);
}

/* OpenGLScreen: uploads the RGB565 screen buffer to g_glScreenTex and draws it to the window. */
static void Gfx_PresentGlScreen(const SDL_Surface *surface)
{
    unsigned int *rgba = malloc(surface->h * surface->w * 4);
    int tex_w = Gfx_NextPow2(surface->w);
    int tex_h = Gfx_NextPow2(surface->h);
    const unsigned short *src;
    unsigned int *dst;
    int n;

    if (g_glScreenTexReady && g_glScreenTex.w >= tex_w && g_glScreenTex.h >= tex_h && !g_glScreenResized) {
        /* the texture is big enough */
    } else {
        if (g_glScreenTexReady) {
            if (g_glScreenTexCur != NULL) {
                g_glScreenTexCur->refs--;
            }
            GlTex_Delete(&g_glScreenTex);
            g_glScreenTexReady = 0;
            g_glScreenResized = 0;
        }
        GlTex_Init(&g_glScreenTex);
        GlTex_Alloc(&g_glScreenTex, tex_w, tex_h);
        g_glScreenTex.refs++;
        g_glScreenTexCur = &g_glScreenTex;
        g_glScreenTexReady = 1;
    }
    src = surface->pixels;
    dst = rgba;
    for (n = surface->h * surface->w; n != 0; n--) {
        unsigned int px = *src++;
        /* RGB565 -> R, G, B, 0xFF bytes */
        *dst++ = (((px | 0xFFFFFFE0u) << 14 | (px & 0x7E0)) << 5) | ((px >> 8) & 0xF8);
    }
    GlScreen_Upload(surface->h, surface->w, rgba);
    GlScreen_Draw(g_screenSurface->w, g_screenSurface->h);
    free(rgba);
}

/*
 * 0x0046BCF0: g_gfxPresent of the System and OpenGLScreen renderers: shows
 * the screen buffer (sprite `sprite` of set) at (x, y), then flips the video
 * surface when double buffered. w and h are only used by the OpenGL
 * renderer's version.
 */
void Gfx_Present(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    SDL_Surface *surface = NULL;

    if (g_gfxPrePresent != NULL) {
        SpriteSet *target = g_renderTarget;
        int target_sprite = g_renderTargetSprite;
        g_gfxPrePresent(set, sprite);
        Gfx_SetRenderTarget(target, target_sprite);
    }
    if (g_renderMode != RENDER_MODE_GL && g_renderMode != RENDER_MODE_GL_ALT) {
        const Sprite *spr = Gfx_SpriteAt(set, sprite);
        if (spr == NULL || spr->format != SPRITE_FORMAT_SURFACE) {
            return;
        }
        surface = spr->pixels;
    }
    if (g_renderMode == RENDER_OPENGL_SCREEN) {
        Gfx_PresentGlScreen(surface);
    } else if (g_renderMode == RENDER_SYSTEM) {
        SDL_Rect dst;
        dst.x = (Sint16)(int)x;
        dst.y = (Sint16)(int)y;
        dst.w = 0;
        dst.h = 0;
        SDL_UpperBlit(surface, NULL, g_screenSurface, &dst);
        SDL_UpdateRect(g_screenSurface, 0, 0, 0, 0);
    }
    if (g_videoFlags & VIDEO_HWBLIT) {
        if (g_videoSdlFlags & SDL_DOUBLEBUF) {
            if (g_renderMode == RENDER_OPENGL || g_renderMode == RENDER_OPENGL_SCREEN) {
                SDL_GL_SwapBuffers();
            } else {
                SDL_Flip(g_videoSurface);
            }
        }
        if (g_renderMode == RENDER_OPENGL) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClearDepth(1.0);
            glDisable(GL_SCISSOR_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }
}

/*
 * 0x0046BFC0: sets the clip rectangle (x2/y2 exclusive) of the render
 * target, ordered and clamped to it, and passes it to the target's SDL
 * surface or to OpenGL.
 */
void Gfx_SetClipRect(int x1, int y1, int x2, int y2)
{
    const Sprite *spr = NULL;
    int t;

    if (g_renderTarget != NULL) {
        spr = Gfx_SpriteAt(g_renderTarget, g_renderTargetSprite);
    }
    if (x1 > x2) {
        t = x1;
        x1 = x2;
        x2 = t;
    }
    if (y1 > y2) {
        t = y1;
        y1 = y2;
        y2 = t;
    }
    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x1 > g_targetWidth - 1) {
        x1 = g_targetWidth - 1;
    }
    if (y1 > g_targetHeight - 1) {
        y1 = g_targetHeight - 1;
    }
    if (x2 < x1) {
        x2 = x1;
    }
    if (y2 < y1) {
        y2 = y1;
    }
    if (x2 > g_targetWidth) {
        x2 = g_targetWidth;
    }
    if (y2 > g_targetHeight) {
        y2 = g_targetHeight;
    }
    g_clipX2 = x2;
    g_clipY2 = y2;
    g_clipX1 = x1;
    g_clipY1 = y1;
    g_clipW = x2 - x1;
    g_clipH = y2 - y1;
    g_clipRowSkip = g_targetWidth - (x2 - x1);
    if (spr != NULL && spr->format == SPRITE_FORMAT_SURFACE) {
        SDL_Rect rect;
        rect.x = (Sint16)x1;
        rect.y = (Sint16)y1;
        rect.w = (Uint16)(x2 - x1);
        rect.h = (Uint16)(y2 - y1);
        SDL_SetClipRect(spr->pixels, &rect);
    }
    if (g_renderMode == RENDER_OPENGL) {
        Gl_ApplyClipRect();
    }
}

/* v * scale rounded to the nearest integer (halves up), with the original's float steps. */
static int Gfx_ScaleRound(float v, float scale)
{
    float scaled = v * scale;
    float rounded = (float)floor(scaled + 0.5);

    return (int)rounded;
}

/* 0x0046C0C0: Gfx_SetClipRect with coordinates scaled by (sx, sy) and rounded. */
void Gfx_SetClipRectScaled(float x1, float y1, float x2, float y2, float sx, float sy)
{
    int iy2 = Gfx_ScaleRound(y2, sy);
    int ix2 = Gfx_ScaleRound(x2, sx);
    int iy1 = Gfx_ScaleRound(y1, sy);
    int ix1 = Gfx_ScaleRound(x1, sx);

    Gfx_SetClipRect(ix1, iy1, ix2, iy2);
}

/* 0x0046C190: points the renderer entry points at those of renderer `mode` (g_renderMode). */
void Gfx_SelectRenderer(int mode)
{
    g_gfxClear = g_gfxClearFns[mode];
    g_gfxPutPixel = g_gfxPutPixelFns[mode];
    g_gfxAddPixel = g_gfxAddPixelFns[mode];
    g_gfxInvertPixel = g_gfxInvertPixelFns[mode];
    g_gfxDrawLine = g_gfxDrawLineFns[mode];
    g_gfxDrawLineAdd = g_gfxDrawLineAddFns[mode];
    g_gfxDrawRect = g_gfxDrawRectFns[mode];
    g_gfxDrawRectAdd = g_gfxDrawRectAddFns[mode];
    g_gfxDrawRectBlend = g_gfxDrawRectBlendFns[mode];
    g_gfxDrawRectSub = g_gfxDrawRectSubFns[mode];
    g_gfxFillRect = g_gfxFillRectFns[mode];
    g_gfxFillRectColor = g_gfxFillRectColorFns[mode];
    g_gfxBlendRect = g_gfxBlendRectFns[mode];
    g_gfxFillRectSub = g_gfxFillRectSubFns[mode];
    g_gfxCapture = g_gfxCaptureFns[mode];
    g_gfxPresent = g_gfxPresentFns[mode];
}
