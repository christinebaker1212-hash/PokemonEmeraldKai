#ifndef MUGEN_GFX_H
#define MUGEN_GFX_H

/*
 * Graphics core (gfx.c): render targets, the clip rectangle, the screen
 * buffer, presenting it, fades and the renderer function tables that
 * Gfx_SelectRenderer fills.
 */

/* Renderer-specific "clear the current target" entry point (see g_gfxClear). */
typedef void (__cdecl *GfxClearFn)(unsigned int color);

/* Renderer entry point whose parameters are not known yet (call through a cast). */
typedef void (__cdecl *GfxFn)(void);

/* Fills the rectangle (x1, y1)-(x2, y2) with color (g_gfxFillRect). */
typedef void (__cdecl *GfxRectFn)(int x1, int y1, int x2, int y2, unsigned int color);

/* Blends color over the rectangle with source/destination alphas out of 256 (g_gfxBlendRect). */
typedef void (__cdecl *GfxBlendRectFn)(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src,
                                       int alpha_dst);

/* Returns the screen as malloc'ed 32-bit RGBA pixels (g_gfxCapture, OpenGL renderers). */
typedef void *(__cdecl *GfxCaptureFn)(void);

/* Shows sprite `sprite` of set (the screen buffer) at (x, y), scaled to w x h by OpenGL (g_gfxPresent). */
typedef void (__cdecl *GfxPresentFn)(SpriteSet *set, int sprite, float x, float y, float w, float h);

/* Called by Gfx_Present with the screen buffer before it is shown (g_gfxPrePresent). */
typedef void (__cdecl *GfxOverlayFn)(SpriteSet *set, int sprite);

/* glBindFramebufferEXT, through GLEW's function pointer. */
typedef void (__stdcall *GlBindFramebufferFn)(unsigned int target, unsigned int framebuffer);

/* Renderer indices of the function tables (g_renderMode, see game.h and palette.h). */
#define GFX_RENDERER_COUNT 4

/* An OpenGL texture (GlTex_* functions, renderer module glrender.c); also the render target "vbuf". */
typedef struct GlTex {
    unsigned int id;        /* 0x00 GL texture name */
    int w;                  /* 0x04 allocated size */
    int h;                  /* 0x08 (0 for 1D textures) */
    int unused_0c;          /* 0x0C cleared, never read */
    unsigned int depth;     /* 0x10 depth renderbuffer of a vbuf (GlTex_AllocDepth) */
    int refs;               /* 0x14 */
    unsigned int size;      /* 0x18 bytes of video memory (g_glTexMemory) */
} GlTex;
MUGEN_ASSERT_OFFSET(GlTex, refs, 0x14);
MUGEN_ASSERT_SIZE(GlTex, 0x1C);

/* Fade.type */
#define FADE_NONE       0
#define FADE_PALETTE    1       /* darkens through the global PalFX */
#define FADE_BARS       2       /* black bars closing from the top and bottom (Fade_DrawBars) */

/* A screen fade (GameVars + 0x11EE4), advanced by Fade_Update. */
typedef struct Fade {
    int type;               /* 0x00 FADE_*, 0 when idle */
    int unused_04;          /* 0x04 set to 256 when the fade ends, never read */
    int out;                /* 0x08 1: fade out (progress runs 0 -> 1), 0: fade in (1 -> 0) */
    int time;               /* 0x0C duration in ticks */
    int bar_size;           /* 0x10 height of the soft edge of the bars (FADE_BARS) */
    int ticks;              /* 0x14 ticks elapsed */
    float progress;         /* 0x18 0..1 */
} Fade;
MUGEN_ASSERT_OFFSET(Fade, ticks, 0x14);
MUGEN_ASSERT_SIZE(Fade, 0x1C);

#endif /* MUGEN_GFX_H */
