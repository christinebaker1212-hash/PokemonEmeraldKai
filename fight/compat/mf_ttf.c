/*
 * Moemon Fight: SDL_ttf (TrueType fonts: only the debug console and packs that ask for them) and
 * SDL_gfx's zoomSurface (scaled fonts). TrueType fonts never open, so the engine falls back to its
 * bitmap fonts; zoomSurface scales by nearest pixel, flipping for negative zooms.
 */
#include <stdlib.h>
#include <math.h>

#include <SDL.h>
#include <SDL_ttf.h>

int TTF_Init(void)
{
    return 0;
}

void TTF_Quit(void)
{
}

TTF_Font *TTF_OpenFont(const char *file, int ptsize)
{
    return NULL;
}

void TTF_CloseFont(TTF_Font *font)
{
}

int TTF_FontAscent(const TTF_Font *font)
{
    return 0;
}

int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h)
{
    if (w != NULL) {
        *w = 0;
    }
    if (h != NULL) {
        *h = 0;
    }
    return -1;
}

SDL_Surface *TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg)
{
    return NULL;
}

SDL_Surface *TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg)
{
    return NULL;
}

int TTF_GetFontKerningSize(TTF_Font *font, int prev_index, int index)
{
    return 0;
}

SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth)
{
    SDL_Surface *dst;
    int w, h, x, y, bpp;

    if (src == NULL || zoomx == 0.0 || zoomy == 0.0) {
        return NULL;
    }
    w = (int)floor(fabs(src->w * zoomx) + 0.5);
    h = (int)floor(fabs(src->h * zoomy) + 0.5);
    if (w < 1) {
        w = 1;
    }
    if (h < 1) {
        h = 1;
    }
    dst = SDL_CreateRGBSurface(src->flags, w, h, src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask,
                               src->format->Bmask, src->format->Amask);
    if (dst == NULL) {
        return NULL;
    }
    if (src->format->palette != NULL && dst->format->palette != NULL) {
        int i;
        for (i = 0; i < src->format->palette->ncolors && i < dst->format->palette->ncolors; i++) {
            dst->format->palette->colors[i] = src->format->palette->colors[i];
        }
    }
    dst->format->colorkey = src->format->colorkey;
    dst->format->alpha = src->format->alpha;
    bpp = src->format->BytesPerPixel;
    for (y = 0; y < h; y++) {
        int sy = (int)(y * src->h / h);
        if (zoomy < 0.0) {
            sy = src->h - 1 - sy;
        }
        for (x = 0; x < w; x++) {
            int sx = (int)(x * src->w / w);
            const Uint8 *s;
            Uint8 *d;
            int k;
            if (zoomx < 0.0) {
                sx = src->w - 1 - sx;
            }
            s = (const Uint8 *)src->pixels + sy * src->pitch + sx * bpp;
            d = (Uint8 *)dst->pixels + y * dst->pitch + x * bpp;
            for (k = 0; k < bpp; k++) {
                d[k] = s[k];
            }
        }
    }
    return dst;
}
