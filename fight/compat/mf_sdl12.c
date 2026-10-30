/*
 * Moemon Fight: the part of SDL 1.2 the engine uses, inside the game (whose SDL is SDL2).
 *
 * Surfaces live in memory: the video mode is a plain surface the host copies out when it is
 * shown (MfHost_Present), blits convert between any of the formats the engine creates
 * (RGB565 screens, 32-bit copies, 8-bit palettes). Events are the keys the host pushes for the
 * players' buttons; there are no joysticks, windows or OpenGL. mf_rename.h gives every
 * function here an MF12_ name.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "mf_compat.h"

static SDL_Surface *sVideo;
static char sError[256] = "";

/* ------------------------------------------------------------------ pixel formats */

static void Format_SetMask(Uint32 mask, Uint8 *shift, Uint8 *loss)
{
    int s = 0, bits = 0;

    if (mask == 0) {
        *shift = 0;
        *loss = 8;
        return;
    }
    while (!(mask & 1)) {
        mask >>= 1;
        s++;
    }
    while (mask & 1) {
        mask >>= 1;
        bits++;
    }
    *shift = (Uint8)s;
    *loss = (Uint8)(bits >= 8 ? 0 : 8 - bits);
}

static SDL_PixelFormat *Format_New(int depth, Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask)
{
    SDL_PixelFormat *f = calloc(1, sizeof(SDL_PixelFormat));

    if (f == NULL) {
        return NULL;
    }
    f->BitsPerPixel = (Uint8)depth;
    f->BytesPerPixel = (Uint8)((depth + 7) / 8);
    f->alpha = 0xFF;
    if (depth == 8) {
        f->palette = calloc(1, sizeof(SDL_Palette));
        if (f->palette != NULL) {
            f->palette->ncolors = 256;
            f->palette->colors = calloc(256, sizeof(SDL_Color));
        }
        f->Rloss = f->Gloss = f->Bloss = f->Aloss = 8;
        return f;
    }
    if (rmask == 0 && gmask == 0 && bmask == 0) {
        /* SDL 1.2's defaults */
        if (depth == 16) {
            rmask = 0xF800;
            gmask = 0x07E0;
            bmask = 0x001F;
        } else if (depth == 15) {
            rmask = 0x7C00;
            gmask = 0x03E0;
            bmask = 0x001F;
        } else {
            rmask = 0x00FF0000;
            gmask = 0x0000FF00;
            bmask = 0x000000FF;
        }
    }
    f->Rmask = rmask;
    f->Gmask = gmask;
    f->Bmask = bmask;
    f->Amask = amask;
    Format_SetMask(rmask, &f->Rshift, &f->Rloss);
    Format_SetMask(gmask, &f->Gshift, &f->Gloss);
    Format_SetMask(bmask, &f->Bshift, &f->Bloss);
    Format_SetMask(amask, &f->Ashift, &f->Aloss);
    return f;
}

static void Format_Free(SDL_PixelFormat *f)
{
    if (f == NULL) {
        return;
    }
    if (f->palette != NULL) {
        free(f->palette->colors);
        free(f->palette);
    }
    free(f);
}

static Uint32 Pixel_Get(const SDL_Surface *s, int x, int y)
{
    const Uint8 *p = (const Uint8 *)s->pixels + y * s->pitch + x * s->format->BytesPerPixel;

    switch (s->format->BytesPerPixel) {
    case 1:
        return *p;
    case 2:
        return *(const Uint16 *)p;
    case 3:
        return (Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16;
    default:
        return *(const Uint32 *)p;
    }
}

static void Pixel_Put(SDL_Surface *s, int x, int y, Uint32 v)
{
    Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * s->format->BytesPerPixel;

    switch (s->format->BytesPerPixel) {
    case 1:
        *p = (Uint8)v;
        break;
    case 2:
        *(Uint16 *)p = (Uint16)v;
        break;
    case 3:
        p[0] = (Uint8)v;
        p[1] = (Uint8)(v >> 8);
        p[2] = (Uint8)(v >> 16);
        break;
    default:
        *(Uint32 *)p = v;
        break;
    }
}

/* A channel of a pixel, expanded to 8 bits. */
static Uint8 Channel_Get(Uint32 v, Uint32 mask, Uint8 shift, Uint8 loss)
{
    Uint32 c = (v & mask) >> shift;

    if (loss >= 8) {
        return 0;
    }
    c <<= loss;
    return (Uint8)(c | (c >> (8 - loss)));
}

static void Pixel_ToRgba(const SDL_PixelFormat *f, Uint32 v, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
    if (f->BitsPerPixel == 8) {
        const SDL_Color *c = f->palette != NULL ? &f->palette->colors[v & 0xFF] : NULL;
        *r = c ? c->r : (Uint8)v;
        *g = c ? c->g : (Uint8)v;
        *b = c ? c->b : (Uint8)v;
        *a = 0xFF;
        return;
    }
    *r = Channel_Get(v, f->Rmask, f->Rshift, f->Rloss);
    *g = Channel_Get(v, f->Gmask, f->Gshift, f->Gloss);
    *b = Channel_Get(v, f->Bmask, f->Bshift, f->Bloss);
    *a = f->Amask != 0 ? Channel_Get(v, f->Amask, f->Ashift, f->Aloss) : 0xFF;
}

static Uint32 Pixel_FromRgba(const SDL_PixelFormat *f, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if (f->BitsPerPixel == 8) {
        /* nearest palette colour */
        int i, best = 0, best_d = 0x7FFFFFFF;
        if (f->palette == NULL) {
            return r;
        }
        for (i = 0; i < f->palette->ncolors; i++) {
            const SDL_Color *c = &f->palette->colors[i];
            int d = (c->r - r) * (c->r - r) + (c->g - g) * (c->g - g) + (c->b - b) * (c->b - b);
            if (d < best_d) {
                best_d = d;
                best = i;
            }
        }
        return (Uint32)best;
    }
    return ((Uint32)(r >> f->Rloss) << f->Rshift) | ((Uint32)(g >> f->Gloss) << f->Gshift) |
           ((Uint32)(b >> f->Bloss) << f->Bshift) | ((Uint32)(a >> f->Aloss) << f->Ashift & f->Amask);
}

Uint32 SDL_MapRGB(const SDL_PixelFormat *const format, const Uint8 r, const Uint8 g, const Uint8 b)
{
    if (format == NULL) {
        return 0;
    }
    return Pixel_FromRgba(format, r, g, b, 0xFF);
}

/* ------------------------------------------------------------------ surfaces */

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask,
                                  Uint32 Bmask, Uint32 Amask)
{
    SDL_Surface *s;
    int pitch;

    if (width < 0 || height < 0 || (depth != 8 && depth != 15 && depth != 16 && depth != 24 && depth != 32)) {
        snprintf(sError, sizeof(sError), "Unsupported surface %ix%ix%i", width, height, depth);
        return NULL;
    }
    s = calloc(1, sizeof(SDL_Surface));
    if (s == NULL) {
        return NULL;
    }
    s->format = Format_New(depth, Rmask, Gmask, Bmask, Amask);
    if (s->format == NULL) {
        free(s);
        return NULL;
    }
    pitch = (width * s->format->BytesPerPixel + 3) & ~3;
    s->flags = flags & (SDL_SRCALPHA | SDL_SRCCOLORKEY);
    if (Amask != 0) {
        s->flags |= SDL_SRCALPHA;
    }
    s->w = width;
    s->h = height;
    s->pitch = (Uint16)pitch;
    s->pixels = calloc(1, (size_t)pitch * (height > 0 ? height : 1));
    if (s->pixels == NULL) {
        Format_Free(s->format);
        free(s);
        return NULL;
    }
    s->clip_rect.x = 0;
    s->clip_rect.y = 0;
    s->clip_rect.w = (Uint16)width;
    s->clip_rect.h = (Uint16)height;
    s->refcount = 1;
    return s;
}

void SDL_FreeSurface(SDL_Surface *surface)
{
    if (surface == NULL || surface == sVideo) {
        return;
    }
    if (--surface->refcount > 0) {
        return;
    }
    if (!(surface->flags & SDL_PREALLOC)) {
        free(surface->pixels);
    }
    Format_Free(surface->format);
    free(surface);
}

SDL_bool SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect)
{
    int x0, y0, x1, y1;

    if (surface == NULL) {
        return SDL_FALSE;
    }
    if (rect == NULL) {
        surface->clip_rect.x = 0;
        surface->clip_rect.y = 0;
        surface->clip_rect.w = (Uint16)surface->w;
        surface->clip_rect.h = (Uint16)surface->h;
        return SDL_TRUE;
    }
    x0 = rect->x < 0 ? 0 : rect->x;
    y0 = rect->y < 0 ? 0 : rect->y;
    x1 = rect->x + rect->w;
    y1 = rect->y + rect->h;
    if (x1 > surface->w) {
        x1 = surface->w;
    }
    if (y1 > surface->h) {
        y1 = surface->h;
    }
    if (x1 <= x0 || y1 <= y0) {
        surface->clip_rect.x = 0;
        surface->clip_rect.y = 0;
        surface->clip_rect.w = 0;
        surface->clip_rect.h = 0;
        return SDL_FALSE;
    }
    surface->clip_rect.x = (Sint16)x0;
    surface->clip_rect.y = (Sint16)y0;
    surface->clip_rect.w = (Uint16)(x1 - x0);
    surface->clip_rect.h = (Uint16)(y1 - y0);
    return SDL_TRUE;
}

int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
    if (surface == NULL) {
        return -1;
    }
    if (flag & SDL_SRCALPHA) {
        surface->flags |= SDL_SRCALPHA;
        surface->format->alpha = alpha;
    } else {
        surface->flags &= ~SDL_SRCALPHA;
        surface->format->alpha = 0xFF;
    }
    return 0;
}

int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    int x0, y0, x1, y1, x, y;

    if (dst == NULL) {
        return -1;
    }
    x0 = dst->clip_rect.x;
    y0 = dst->clip_rect.y;
    x1 = x0 + dst->clip_rect.w;
    y1 = y0 + dst->clip_rect.h;
    if (dstrect != NULL) {
        if (dstrect->x > x0) {
            x0 = dstrect->x;
        }
        if (dstrect->y > y0) {
            y0 = dstrect->y;
        }
        if (dstrect->x + dstrect->w < x1) {
            x1 = dstrect->x + dstrect->w;
        }
        if (dstrect->y + dstrect->h < y1) {
            y1 = dstrect->y + dstrect->h;
        }
    }
    for (y = y0; y < y1; y++) {
        for (x = x0; x < x1; x++) {
            Pixel_Put(dst, x, y, color);
        }
    }
    if (dstrect != NULL) {
        dstrect->x = (Sint16)x0;
        dstrect->y = (Sint16)y0;
        dstrect->w = (Uint16)(x1 > x0 ? x1 - x0 : 0);
        dstrect->h = (Uint16)(y1 > y0 ? y1 - y0 : 0);
    }
    return 0;
}

static int Format_Same(const SDL_PixelFormat *a, const SDL_PixelFormat *b)
{
    return a->BitsPerPixel == b->BitsPerPixel && a->Rmask == b->Rmask && a->Gmask == b->Gmask &&
           a->Bmask == b->Bmask && a->Amask == b->Amask && a->BitsPerPixel != 8;
}

/* SDL_BlitSurface: srcrect NULL for the whole source; only dstrect's x and y are read, and it gets the
 * rectangle drawn. Colour keys and source alpha (per pixel or per surface) as SDL 1.2 blends them. */
int SDL_UpperBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
    int sx, sy, w, h, dx, dy, x, y;
    int keyed, alpha;

    if (src == NULL || dst == NULL) {
        snprintf(sError, sizeof(sError), "SDL_UpperBlit: passed a NULL surface");
        return -1;
    }
    if (srcrect != NULL) {
        sx = srcrect->x;
        sy = srcrect->y;
        w = srcrect->w;
        h = srcrect->h;
    } else {
        sx = 0;
        sy = 0;
        w = src->w;
        h = src->h;
    }
    dx = dstrect != NULL ? dstrect->x : 0;
    dy = dstrect != NULL ? dstrect->y : 0;
    /* clip to the source */
    if (sx < 0) {
        w += sx;
        dx -= sx;
        sx = 0;
    }
    if (sy < 0) {
        h += sy;
        dy -= sy;
        sy = 0;
    }
    if (sx + w > src->w) {
        w = src->w - sx;
    }
    if (sy + h > src->h) {
        h = src->h - sy;
    }
    /* clip to the destination's clip rectangle */
    if (dx < dst->clip_rect.x) {
        int d = dst->clip_rect.x - dx;
        w -= d;
        sx += d;
        dx += d;
    }
    if (dy < dst->clip_rect.y) {
        int d = dst->clip_rect.y - dy;
        h -= d;
        sy += d;
        dy += d;
    }
    if (dx + w > dst->clip_rect.x + dst->clip_rect.w) {
        w = dst->clip_rect.x + dst->clip_rect.w - dx;
    }
    if (dy + h > dst->clip_rect.y + dst->clip_rect.h) {
        h = dst->clip_rect.y + dst->clip_rect.h - dy;
    }
    if (dstrect != NULL) {
        dstrect->x = (Sint16)dx;
        dstrect->y = (Sint16)dy;
        dstrect->w = (Uint16)(w > 0 ? w : 0);
        dstrect->h = (Uint16)(h > 0 ? h : 0);
    }
    if (w <= 0 || h <= 0) {
        return 0;
    }
    keyed = (src->flags & SDL_SRCCOLORKEY) != 0;
    alpha = (src->flags & SDL_SRCALPHA) != 0 && (src->format->Amask != 0 || src->format->alpha != 0xFF);
    if (!keyed && !alpha && Format_Same(src->format, dst->format)) {
        int bytes = w * src->format->BytesPerPixel;
        for (y = 0; y < h; y++) {
            memmove((Uint8 *)dst->pixels + (dy + y) * dst->pitch + dx * dst->format->BytesPerPixel,
                    (const Uint8 *)src->pixels + (sy + y) * src->pitch + sx * src->format->BytesPerPixel, bytes);
        }
        return 0;
    }
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            Uint32 v = Pixel_Get(src, sx + x, sy + y);
            Uint8 r, g, b, a;
            if (keyed && v == src->format->colorkey) {
                continue;
            }
            Pixel_ToRgba(src->format, v, &r, &g, &b, &a);
            if (alpha) {
                unsigned int k = src->format->Amask != 0 ? a : src->format->alpha;
                Uint8 dr, dg, db, da;
                if (k == 0) {
                    continue;
                }
                if (k < 0xFF) {
                    Pixel_ToRgba(dst->format, Pixel_Get(dst, dx + x, dy + y), &dr, &dg, &db, &da);
                    r = (Uint8)(dr + ((r - dr) * (int)k >> 8));
                    g = (Uint8)(dg + ((g - dg) * (int)k >> 8));
                    b = (Uint8)(db + ((b - db) * (int)k >> 8));
                }
                a = 0xFF;
            }
            Pixel_Put(dst, dx + x, dy + y, Pixel_FromRgba(dst->format, r, g, b, a));
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ the video mode */

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    SDL_Surface *s;

    if (bpp != 16 && bpp != 32) {
        bpp = 16;
    }
    if (sVideo != NULL && sVideo->w == width && sVideo->h == height && sVideo->format->BitsPerPixel == bpp) {
        return sVideo;
    }
    s = SDL_CreateRGBSurface(0, width, height, bpp, 0, 0, 0, 0);
    if (s == NULL) {
        return NULL;
    }
    if (sVideo != NULL) {
        SDL_Surface *old = sVideo;
        sVideo = NULL;
        SDL_FreeSurface(old);
    }
    s->flags |= flags & (SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    sVideo = s;
    return s;
}

void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    if (screen != NULL && screen == sVideo && x == 0 && y == 0 && w == 0 && h == 0) {
        MfHost_Present(screen);
    }
}

int SDL_Flip(SDL_Surface *screen)
{
    if (screen != NULL && screen == sVideo) {
        MfHost_Present(screen);
    }
    return 0;
}

int SDL_ShowCursor(int toggle)
{
    return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon)
{
}

int SDL_GetWMInfo(SDL_SysWMinfo *info)
{
    return 0;   /* no window of its own */
}

int SDL_GL_SetAttribute(SDL_GLattr attr, int value)
{
    return -1;
}

void SDL_GL_SwapBuffers(void)
{
}

/* ------------------------------------------------------------------ events and keys */

#define MF_EVENT_QUEUE 64

static SDL_Event sEvents[MF_EVENT_QUEUE];
static int sEventRead, sEventWrite;
static Uint8 sKeyState[SDLK_LAST];

void MfSdl_PushKey(int sym, int down)
{
    SDL_Event *ev;
    int next = (sEventWrite + 1) % MF_EVENT_QUEUE;

    if (sym <= 0 || sym >= SDLK_LAST || next == sEventRead) {
        return;
    }
    sKeyState[sym] = (Uint8)(down != 0);
    ev = &sEvents[sEventWrite];
    memset(ev, 0, sizeof(*ev));
    ev->type = (Uint8)(down ? SDL_KEYDOWN : SDL_KEYUP);
    ev->key.type = ev->type;
    ev->key.state = (Uint8)(down ? SDL_PRESSED : SDL_RELEASED);
    ev->key.keysym.sym = (SDLKey)sym;
    sEventWrite = next;
}

void MfSdl_ClearEvents(void)
{
    sEventRead = sEventWrite = 0;
    memset(sKeyState, 0, sizeof(sKeyState));
}

int SDL_PollEvent(SDL_Event *event)
{
    if (sEventRead == sEventWrite) {
        return 0;
    }
    if (event != NULL) {
        *event = sEvents[sEventRead];
    }
    sEventRead = (sEventRead + 1) % MF_EVENT_QUEUE;
    return 1;
}

Uint8 *SDL_GetKeyState(int *numkeys)
{
    if (numkeys != NULL) {
        *numkeys = SDLK_LAST;
    }
    return sKeyState;
}

SDLMod SDL_GetModState(void)
{
    return KMOD_NONE;
}

int SDL_EnableKeyRepeat(int delay, int interval)
{
    return 0;
}

int SDL_EnableUNICODE(int enable)
{
    return 0;
}

char *SDL_GetKeyName(SDLKey key)
{
    static char name[16];

    if (key >= 32 && key < 127) {
        name[0] = (char)key;
        name[1] = '\0';
    } else {
        snprintf(name, sizeof(name), "key %d", (int)key);
    }
    return name;
}

/* ------------------------------------------------------------------ joysticks: none */

int SDL_NumJoysticks(void)
{
    return 0;
}

SDL_Joystick *SDL_JoystickOpen(int device_index)
{
    return NULL;
}

int SDL_JoystickOpened(int device_index)
{
    return 0;
}

int SDL_JoystickNumAxes(SDL_Joystick *joystick)
{
    return 0;
}

int SDL_JoystickNumButtons(SDL_Joystick *joystick)
{
    return 0;
}

int SDL_JoystickNumHats(SDL_Joystick *joystick)
{
    return 0;
}

void SDL_JoystickClose(SDL_Joystick *joystick)
{
}

int SDL_JoystickEventState(int state)
{
    return 0;
}

/* ------------------------------------------------------------------ RWops */

static int SDLCALL Mem_Seek(SDL_RWops *ctx, int offset, int whence)
{
    Uint8 *pos;

    switch (whence) {
    case RW_SEEK_SET:
        pos = ctx->hidden.mem.base + offset;
        break;
    case RW_SEEK_CUR:
        pos = ctx->hidden.mem.here + offset;
        break;
    case RW_SEEK_END:
        pos = ctx->hidden.mem.stop + offset;
        break;
    default:
        return -1;
    }
    if (pos < ctx->hidden.mem.base) {
        pos = ctx->hidden.mem.base;
    }
    if (pos > ctx->hidden.mem.stop) {
        pos = ctx->hidden.mem.stop;
    }
    ctx->hidden.mem.here = pos;
    return (int)(pos - ctx->hidden.mem.base);
}

static int SDLCALL Mem_Read(SDL_RWops *ctx, void *ptr, int size, int maxnum)
{
    size_t total, left;

    if (size <= 0 || maxnum <= 0) {
        return 0;
    }
    total = (size_t)size * (size_t)maxnum;
    left = (size_t)(ctx->hidden.mem.stop - ctx->hidden.mem.here);
    if (total > left) {
        total = left - left % (size_t)size;
    }
    memcpy(ptr, ctx->hidden.mem.here, total);
    ctx->hidden.mem.here += total;
    return (int)(total / (size_t)size);
}

static int SDLCALL Mem_Write(SDL_RWops *ctx, const void *ptr, int size, int num)
{
    size_t total, left;

    if (size <= 0 || num <= 0) {
        return 0;
    }
    total = (size_t)size * (size_t)num;
    left = (size_t)(ctx->hidden.mem.stop - ctx->hidden.mem.here);
    if (total > left) {
        total = left - left % (size_t)size;
    }
    memcpy(ctx->hidden.mem.here, ptr, total);
    ctx->hidden.mem.here += total;
    return (int)(total / (size_t)size);
}

static int SDLCALL Rw_Close(SDL_RWops *ctx)
{
    if (ctx != NULL) {
        SDL_FreeRW(ctx);
    }
    return 0;
}

static int SDLCALL Fp_Seek(SDL_RWops *ctx, int offset, int whence)
{
    if (fseek(ctx->hidden.stdio.fp, offset, whence) != 0) {
        return -1;
    }
    return (int)ftell(ctx->hidden.stdio.fp);
}

static int SDLCALL Fp_Read(SDL_RWops *ctx, void *ptr, int size, int maxnum)
{
    return (int)fread(ptr, (size_t)size, (size_t)maxnum, ctx->hidden.stdio.fp);
}

static int SDLCALL Fp_Write(SDL_RWops *ctx, const void *ptr, int size, int num)
{
    return (int)fwrite(ptr, (size_t)size, (size_t)num, ctx->hidden.stdio.fp);
}

static int SDLCALL Fp_Close(SDL_RWops *ctx)
{
    if (ctx != NULL) {
        if (ctx->hidden.stdio.autoclose) {
            fclose(ctx->hidden.stdio.fp);
        }
        SDL_FreeRW(ctx);
    }
    return 0;
}

SDL_RWops *SDL_AllocRW(void)
{
    return calloc(1, sizeof(SDL_RWops));
}

void SDL_FreeRW(SDL_RWops *area)
{
    free(area);
}

SDL_RWops *SDL_RWFromMem(void *mem, int size)
{
    SDL_RWops *rw = SDL_AllocRW();

    if (rw == NULL) {
        return NULL;
    }
    rw->seek = Mem_Seek;
    rw->read = Mem_Read;
    rw->write = Mem_Write;
    rw->close = Rw_Close;
    rw->hidden.mem.base = mem;
    rw->hidden.mem.here = mem;
    rw->hidden.mem.stop = (Uint8 *)mem + size;
    return rw;
}

SDL_RWops *SDL_RWFromFP(FILE *fp, int autoclose)
{
    SDL_RWops *rw = SDL_AllocRW();

    if (rw == NULL) {
        return NULL;
    }
    rw->seek = Fp_Seek;
    rw->read = Fp_Read;
    rw->write = Fp_Write;
    rw->close = Fp_Close;
    rw->hidden.stdio.fp = fp;
    rw->hidden.stdio.autoclose = autoclose;
    return rw;
}

/* ------------------------------------------------------------------ the rest */

int SDL_Init(Uint32 flags)
{
    return 0;
}

void SDL_Quit(void)
{
}

char *SDL_GetError(void)
{
    return sError;
}

int SDL_putenv(const char *variable)
{
    return 0;   /* the engine's SDL hints are not the game's environment */
}

char *SDL_AudioDriverName(char *namebuf, int maxlen)
{
    if (namebuf != NULL && maxlen > 0) {
        snprintf(namebuf, (size_t)maxlen, "moemon");
    }
    return namebuf;
}
