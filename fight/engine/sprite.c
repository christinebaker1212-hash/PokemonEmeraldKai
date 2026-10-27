/*
 * Sprites and sprite sets (original module 0x4650A0-0x4661DE): creating
 * sprites from raw, compressed or surface data (uploaded as OpenGL textures
 * with the GL renderers), the (group, number) containers of sprites and
 * palettes that SFF files are loaded into, and the sprite drawing entry
 * points, which dispatch to the OpenGL renderer or to the software one.
 */
#include "mugen.h"

#define SPRITE_LOG_ERROR    1
#define SPRITE_LOG_WARNING  5

/* The sprite in slot index of a set. */
static Sprite *Sprite_At(const SpriteSet *set, int index)
{
    KeyedArray *sprites = set->sprites;

    return (Sprite *)(sprites->data + sprites->elem_size * index);
}

static Palette *Palette_At(const KeyedArray *palettes, int index)
{
    return (Palette *)(palettes->data + palettes->elem_size * index);
}

/* Video memory of an OpenGL texture object (GlTex_*): a byte count at offset 0x18. */
static unsigned int GlTex_Size(const void *tex)
{
    return *(const unsigned int *)((const unsigned char *)tex + 0x18);
}

/*
 * Downscale factor that makes a side of size pixels fit max_size: the next
 * power of two of size, divided by max_size.
 */
static int Sprite_TextureStep(int size, int max_size)
{
    unsigned int v = (unsigned int)size - 1;

    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    if (v != 0) {
        v++;
    }
    return (int)(v / (unsigned int)max_size);
}

/*
 * 0x004650A0: sets up spr from w x h pixels of bpp bits in data (size bytes,
 * stored in format), with x, y as axis. Flags (SPRITE_INIT_*):
 * - DECODE: compressed formats 2..12 are decoded into a new buffer (their
 *   data starts with the decoded size); the sprite becomes raw;
 * - with an OpenGL renderer, raw sprites go into a texture unless
 *   NO_TEXTURE is set; sprites larger than the maximum texture size are
 *   downscaled first and scale_x/scale_y keep the factors;
 * - otherwise the sprite keeps data (taking ownership with OWN) or a copy of
 *   it (COPY), and gets a row offset table.
 * With OWN the data is freed when it is not kept (also on most errors).
 *
 * Quirks kept: formats 5 to 9 have no decoder (the call crashes), a failed
 * COPY allocation leaks owned data, and sprites with no rows (h == 0) fail
 * in Sprite_InitRows after it allocated the row table.
 */
int Sprite_Init(unsigned int flags, Sprite *spr, int format, void *data, size_t size, int w, int h, int bpp,
                int x, int y)
{
    int gl = g_renderMode == RENDER_MODE_GL || g_renderMode == RENDER_MODE_GL_ALT;
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    int owns = (flags & SPRITE_INIT_OWN) != 0;
    int copy = flags & SPRITE_INIT_COPY;
    int texture = 0;

    memset(spr, 0, sizeof(Sprite));
    spr->scale_y = 1.0f;
    spr->scale_x = 1.0f;
    spr->pal_index = -1;
    spr->format = format;
    if ((flags & SPRITE_INIT_DECODE) && format >= SPRITE_FORMAT_DECODE_MIN && format <= SPRITE_FORMAT_DECODE_MAX) {
        size_t out_size = *(const unsigned int *)data;
        void *out = malloc(out_size);

        if (out == NULL) {
            if (owns) {
                free(data);
            }
            return 0;
        }
        g_spriteDecoders[format](data, size, out);
        if (owns) {
            free(data);
        }
        data = out;
        size = out_size;
        owns = 1;
        copy = 0;
        format = SPRITE_FORMAT_RAW;
    }
    spr->format = format;

    if (!(flags & SPRITE_INIT_NO_TEXTURE) && gl && format == SPRITE_FORMAT_RAW) {
        void *upload = data;
        void *scaled = NULL;
        int step_x = 1;
        int step_y = 1;
        int max_size;
        void *tex;
        int ok;

        texture = 1;
        max_size = g_renderMode == RENDER_MODE_GL ? g_glMaxTextureSize : SPRITE_MAX_TEXTURE_SIZE;
        if (w > max_size || h > max_size) {
            MatchLog_Printf(SPRITE_LOG_WARNING, "Sprite dimensions %ix%i exceed max texture size %ix%i.\n", w, h,
                            max_size, max_size);
            if (w > max_size) {
                step_x = Sprite_TextureStep(w, max_size);
            }
            if (h > max_size) {
                step_y = Sprite_TextureStep(h, max_size);
            }
            if (step_x < 1) {
                step_x = 1;
            }
            if (step_y < 1) {
                step_y = 1;
            }
            scaled = Sprite_Downscale(data, w, h, step_x, step_y, bpp);
            scale_x = (float)step_x;
            scale_y = (float)step_y;
            if (scaled == NULL) {
                MatchLog_Printf(SPRITE_LOG_ERROR, "Out of memory.\n");
                if (owns) {
                    free(data);
                }
                return 0;
            }
            upload = scaled;
        }
        spr->format = SPRITE_FORMAT_TEXTURE;
        tex = GlTex_Create();
        if (tex == NULL) {
            if (owns) {
                free(data);
            }
            if (scaled != NULL) {
                free(scaled);
            }
            return 0;
        }
        ok = GlTex_Upload2D(h / step_y, bpp, tex, upload, w / step_x);
        if (scaled != NULL) {
            free(scaled);
        }
        if (!ok) {
            if (owns) {
                free(data);
            }
            GlTex_Free(tex);
            return 0;
        }
        spr->pixels = tex;
        spr->owns_pixels = 1;
        spr->pixels_size = 4;
        if (owns) {
            free(data);
        }
    } else if (copy) {
        spr->pixels = calloc(1, size);
        if (spr->pixels == NULL) {
            return 0;
        }
        memcpy(spr->pixels, data, size);
        spr->owns_pixels = 1;
        spr->pixels_size = size;
    } else {
        spr->pixels = data;
        spr->owns_pixels = owns;
        spr->pixels_size = size;
    }

    spr->h = h;
    spr->w = w;
    spr->bpp = bpp;
    if (bpp <= 8) {
        spr->ncolors = 1 << (bpp & 31);
    }
    spr->x = x;
    spr->scale_x = spr->scale_x * scale_x;
    spr->y = y;
    spr->scale_y = spr->scale_y * scale_y;
    if (!texture && !Sprite_InitRows(spr)) {
        return 0;
    }
    return 1;
}

/*
 * 0x004654A0: releases what spr owns (pixels by format, row table) and
 * clears it. The pixels are dropped from the decoded sprite cache first.
 */
void Sprite_Free(Sprite *spr)
{
    if (spr == NULL) {
        return;
    }
    if (spr->pixels != NULL) {
        SpriteCache_Remove(spr->pixels);
    }
    if (spr->owns_pixels && spr->pixels != NULL) {
        if (spr->format == SPRITE_FORMAT_SURFACE) {
            SDL_FreeSurface(spr->pixels);
        } else if (spr->format == SPRITE_FORMAT_TEXTURE) {
            GlTex_Free(spr->pixels);
        } else {
            free(spr->pixels);
        }
    }
    if (spr->owns_rows && spr->rows != NULL) {
        free(spr->rows);
    }
    memset(spr, 0, sizeof(Sprite));
}

/*
 * 0x00465510: builds the table of row offsets in the pixel data. Returns the
 * number of rows, 0 when the table exists or cannot be allocated.
 */
int Sprite_InitRows(Sprite *spr)
{
    int offset = 0;
    int stride;
    int *rows;
    int i;

    if (spr->rows != NULL) {
        return 0;
    }
    rows = calloc(4, spr->h);
    if (rows == NULL) {
        return 0;
    }
    stride = ((spr->bpp + 7) >> 3) * spr->w;
    spr->owns_rows = 1;
    for (i = 0; i < spr->h; i++) {
        rows[i] = offset;
        offset += stride;
    }
    spr->rows = rows;
    return spr->h;
}

/*
 * 0x00465570: makes spr use palette (group, number) of set, if it exists and
 * has as many colors as spr. set->palettes must exist.
 */
int SpriteSet_LinkPalette(SpriteSet *set, Sprite *spr, int group, int number)
{
    int index = KeyedArray_Find(set->palettes, group, number);

    if (index == -1 || Palette_At(set->palettes, index)->count != spr->ncolors) {
        return 0;
    }
    spr->pal_index = index;
    return 1;
}

/*
 * 0x004655B0: an empty set with room for the given numbers of sprites and
 * palettes (tables are only created for counts > 0). The set leaks when a
 * table cannot be created.
 */
SpriteSet *SpriteSet_Create(int sprite_count, int palette_count)
{
    SpriteSet *set = calloc(1, sizeof(SpriteSet));

    if (set == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    if (sprite_count > 0) {
        set->sprites = KeyedArray_Create(sprite_count, sizeof(Sprite));
        if (set->sprites == NULL) {
            return NULL;
        }
    }
    if (palette_count > 0) {
        set->palettes = KeyedArray_Create(palette_count, sizeof(Palette));
        if (set->palettes == NULL) {
            return NULL;
        }
    }
    return set;
}

/* 0x00465610: adds a copy of spr as (group, number), growing the table when full. */
int SpriteSet_AddSprite(SpriteSet *set, const Sprite *spr, int group, int number)
{
    KeyedArray *list;

    if (set->sprites == NULL) {
        set->sprites = KeyedArray_Create(1, sizeof(Sprite));
        if (set->sprites == NULL) {
            return 0;
        }
    }
    list = set->sprites;
    if (list->count >= list->capacity && !KeyedArray_Grow(list, list->capacity + list->grow_step)) {
        return 0;
    }
    return KeyedArray_Add(list, spr, group, number) != -1;
}

/* 0x00465670: adds a copy of pal as (group, number), growing the table when full. */
int SpriteSet_AddPalette(SpriteSet *set, const Palette *pal, int group, int number)
{
    KeyedArray *list;

    if (set->palettes == NULL) {
        set->palettes = KeyedArray_Create(1, sizeof(Palette));
        if (set->palettes == NULL) {
            return 0;
        }
    }
    list = set->palettes;
    if (list->count >= list->capacity && !KeyedArray_Grow(list, list->capacity + list->grow_step)) {
        return 0;
    }
    return KeyedArray_Add(list, pal, group, number) != -1;
}

/*
 * 0x004656E0: frees the set, its palettes and (unless they are borrowed from
 * another set, whose reference count drops) its sprites. Returns 0 without
 * freeing anything while other sets borrow its sprites.
 */
int SpriteSet_Free(SpriteSet *set)
{
    if (set == NULL) {
        return 1;
    }
    if (set->refs > 0) {
        return 0;
    }
    if (set->palettes != NULL) {
        Palette *pal;
        for (pal = KeyedArray_First(set->palettes); pal != NULL; pal = KeyedArray_Next(set->palettes)) {
            Pal_Free(pal);
        }
        KeyedArray_Free(set->palettes);
    }
    if (set->shared == NULL) {
        if (set->sprites != NULL) {
            Sprite *spr;
            for (spr = KeyedArray_First(set->sprites); spr != NULL; spr = KeyedArray_Next(set->sprites)) {
                Sprite_Free(spr);
            }
            KeyedArray_Free(set->sprites);
        }
        if (set->data != NULL) {
            free(set->data);
        }
    } else {
        set->shared->refs--;
    }
    free(set);
    return 1;
}

/* 0x00465810: sprite (group, number) of set, or NULL. */
Sprite *SpriteSet_FindSprite(SpriteSet *set, int group, int number)
{
    return KeyedArray_Get(set->sprites, group, number);
}

/*
 * 0x00465840: palette in slot index of set, through the palette remapping
 * of the object being drawn (g_palRemapCur): the remapped palette is used
 * when it exists and has as many colors. Indices from count up are
 * rejected even when the slot is used.
 */
Palette *SpriteSet_GetPalette(SpriteSet *set, int index)
{
    KeyedArray *palettes;

    if (index < 0) {
        return NULL;
    }
    palettes = set->palettes;
    if (index >= palettes->count) {
        return NULL;
    }
    if (g_palRemapCur != NULL) {
        Palette *pal = Palette_At(palettes, index);
        Palette *found;
        int group = palettes->slots[index].key1;
        int number = palettes->slots[index].key2;
        int slot;

        PalRemap_Lookup(g_palRemapCur, group, number, &group, &number);
        slot = KeyedArray_Find(palettes, group, number);
        found = slot == -1 ? NULL : Palette_At(palettes, slot);
        if (pal != NULL) {
            if (found == NULL) {
                return Palette_At(set->palettes, index);
            }
            if (pal->count != found->count) {
                found = pal;
            }
        }
        if (found != NULL) {
            return found;
        }
    }
    return Palette_At(set->palettes, index);
}

/* 0x00465920: palette (group, number) of set, or NULL. */
Palette *SpriteSet_FindPalette(SpriteSet *set, int group, int number)
{
    if (set->palettes == NULL) {
        return NULL;
    }
    return KeyedArray_Get(set->palettes, group, number);
}

/* 0x00465950: SpriteSet_GetPalette, with the colors converted for the software renderer. */
Palette *SpriteSet_GetPaletteConverted(SpriteSet *set, int index)
{
    Palette *pal = SpriteSet_GetPalette(set, index);

    if (pal != NULL) {
        Pal_Convert(pal);
    }
    return pal;
}

/*
 * 0x00465970: the pixels of a sprite: the raw data, the pixels of an SDL
 * surface, or the decoded data of a compressed sprite from the sprite cache
 * (NULL when it is not available, and for textures).
 */
void *Sprite_GetPixels(SpriteSet *set, int sprite)
{
    Sprite *spr = Sprite_At(set, sprite);
    void *out = NULL;

    if (spr == NULL) {
        return NULL;
    }
    if (spr->format == SPRITE_FORMAT_RAW) {
        return spr->pixels;
    }
    if (spr->format == SPRITE_FORMAT_SURFACE) {
        return ((SDL_Surface *)spr->pixels)->pixels;
    }
    SpriteCache_Get(spr->pixels, spr->format, spr->pixels_size, &out);
    return out;
}

/* 0x004659D0: draws a sprite with the current renderer. */
void Sprite_Draw(SpriteSet *set, int sprite, const DrawParams *dp)
{
    if (g_renderMode == RENDER_MODE_GL) {
        Sprite_DrawGL(set, sprite, dp);
    } else {
        Sprite_DrawSoft(set, sprite, dp);
    }
}

/*
 * 0x00465A00: draws a sprite with palette slot pal_index of its set instead
 * of its own, when the palette has as many colors.
 */
void Sprite_DrawWithPalette(SpriteSet *set, int sprite, int pal_index, const DrawParams *dp)
{
    Sprite *spr = Sprite_At(set, sprite);
    int saved = spr->pal_index;

    if (spr->ncolors == Palette_At(set->palettes, pal_index)->count) {
        spr->pal_index = pal_index;
    }
    Sprite_Draw(set, sprite, dp);
    spr->pal_index = saved;
}

/*
 * 0x00465A80: software renderer, 16-bit targets only. Paletted sprites go
 * to the software rasterizer; SDL surface sprites are blitted onto the
 * render target surface, and only without scaling, rotation, flipping or
 * blending.
 */
void Sprite_DrawSoft(SpriteSet *set, int sprite, const DrawParams *dp)
{
    const Sprite *spr;
    const Sprite *target;

    if (g_targetBpp != SPRITE_SOFT_BPP) {
        return;
    }
    spr = Sprite_At(set, sprite);
    if (spr->bpp <= 8) {
        Sprite_DrawSoft8(dp, set, sprite);
        return;
    }
    if (spr->format != SPRITE_FORMAT_SURFACE) {
        return;
    }
    target = Sprite_At(g_renderTarget, g_renderTargetSprite);
    if (target == NULL || target->format != SPRITE_FORMAT_SURFACE || target->pixels == NULL) {
        return;
    }
    if (dp->scale2_x == 1.0f && dp->scale2_y == 1.0f && dp->scale_x == 1.0f && dp->scale_y == 1.0f &&
        dp->shear_x == 0.0f && dp->shear_y == 0.0f && dp->angle == 0.0f && dp->flip == 0 && dp->flip2 == 0 &&
        dp->blend == BLEND_NONE) {
        Sprite_BlitSurface(set, sprite, (int)dp->x, (int)dp->y);
    }
}

/*
 * 0x00465B90: SDL_BlitSurface of a surface sprite onto the render target
 * surface at (x, y) (16-bit coordinates). A missing surface on either side
 * is passed as NULL, which SDL rejects.
 */
void Sprite_BlitSurface(SpriteSet *set, int sprite, int x, int y)
{
    SDL_Surface *dst_surface = NULL;
    const Sprite *target = Sprite_At(g_renderTarget, g_renderTargetSprite);
    const Sprite *spr;
    SDL_Rect dst;

    /* w and h stay uninitialised: SDL_UpperBlit only reads x and y of the destination. */
    dst.x = (Sint16)x;
    dst.y = (Sint16)y;
    if (target != NULL && target->format == SPRITE_FORMAT_SURFACE) {
        dst_surface = target->pixels;
    }
    spr = Sprite_At(set, sprite);
    if (spr != NULL && spr->format == SPRITE_FORMAT_SURFACE) {
        SDL_UpperBlit(spr->pixels, NULL, dst_surface, &dst);
    } else {
        SDL_UpperBlit(NULL, NULL, dst_surface, &dst);
    }
}

/* Draw parameters of the Sprite_Draw* helpers: every field set, no blending. */
static void Sprite_SimpleDrawParams(DrawParams *dp, float x, float y, float scale_x, float scale_y)
{
    memset(dp, 0, sizeof(DrawParams));
    dp->width_ratio = 1.0f;
    dp->width_anchor = 0.5f;
    dp->x = x;
    dp->y = y;
    dp->scale_x = scale_x;
    dp->scale_y = scale_y;
    dp->flip = 0;
    dp->scale2_x = 1.0f;
    dp->scale2_y = 1.0f;
    dp->top_stretch = 1.0f;
    dp->bottom_stretch = 1.0f;
    dp->masked = 1;
    dp->alpha_src = 1.0f;
    dp->alpha_dst = 0.0f;
    dp->blend = BLEND_NONE;
    dp->flags = 0xFFFF;
}

/* 0x00465C10: draws a sprite at (x, y) with a scale. */
void Sprite_DrawAt(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y)
{
    DrawParams dp;

    Sprite_SimpleDrawParams(&dp, x, y, scale_x, scale_y);
    Sprite_Draw(set, sprite, &dp);
}

/* 0x00465CB0: Sprite_DrawAt with additive blending. */
void Sprite_DrawAdd(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y)
{
    DrawParams dp;

    Sprite_SimpleDrawParams(&dp, x, y, scale_x, scale_y);
    dp.blend = BLEND_ADD;
    dp.alpha_src = 1.0f;
    dp.alpha_dst = 1.0f;
    Sprite_Draw(set, sprite, &dp);
}

/* 0x00465D60: Sprite_DrawAt with additive blending by alpha_src/alpha_dst (256 = 1.0). */
void Sprite_DrawAlpha(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y, int alpha_src,
                      int alpha_dst)
{
    DrawParams dp;

    Sprite_SimpleDrawParams(&dp, x, y, scale_x, scale_y);
    dp.blend = BLEND_ADD;
    dp.alpha_src = (float)(alpha_src * 0.00390625);
    dp.alpha_dst = (float)(alpha_dst * 0.00390625);
    Sprite_Draw(set, sprite, &dp);
}

/* 0x00465E20: Sprite_DrawAt with subtractive blending. */
void Sprite_DrawSub(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y)
{
    DrawParams dp;

    Sprite_SimpleDrawParams(&dp, x, y, scale_x, scale_y);
    dp.blend = BLEND_SUB;
    dp.alpha_src = 1.0f;
    dp.alpha_dst = 1.0f;
    Sprite_Draw(set, sprite, &dp);
}

/*
 * 0x00465ED0: a set holding w x h pixels of bpp bits as sprite (0, 0), with
 * pal as palette (0, 0). Paletted sprites need pal, others need 24 bits or
 * more; flags go to Sprite_Init. The set leaks when its tables cannot be
 * created.
 */
SpriteSet *SpriteSet_FromPixels(int bpp, void *pixels, int w, int h, Palette *pal, unsigned int flags)
{
    int size = ((bpp + 7) >> 3) * w * h;
    SpriteSet *set;
    Sprite spr;

    if (bpp <= 8) {
        if (pal == NULL) {
            return NULL;
        }
    } else if (bpp < 24) {
        return NULL;
    }
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
    if (pal != NULL) {
        SpriteSet_AddPalette(set, pal, 0, 0);
    }
    if (!Sprite_Init(flags, &spr, SPRITE_FORMAT_RAW, pixels, size, w, h, bpp, 0, 0)) {
        SpriteSet_Free(set);
        return NULL;
    }
    if (bpp <= 8) {
        SpriteSet_LinkPalette(set, &spr, 0, 0);
    }
    SpriteSet_AddSprite(set, &spr, 0, 0);
    return set;
}

/*
 * 0x00465FE0: estimated memory used by the set, in system memory and in
 * textures. Borrowed sprites are not counted.
 */
void SpriteSet_MemUse(SpriteSet *set, unsigned int *sys_bytes, unsigned int *tex_bytes)
{
    unsigned int sys = 0;
    unsigned int tex = 0;

    if (set != NULL) {
        if (set->palettes != NULL) {
            Palette *pal;
            for (pal = KeyedArray_First(set->palettes); pal != NULL; pal = KeyedArray_Next(set->palettes)) {
                sys += sizeof(Palette);
                if (pal->owns) {
                    if (pal->is_texture == 0) {
                        sys += pal->count * 4;
                    } else if (pal->is_texture == 1) {
                        sys += 0x1C;
                        if (pal->data != NULL) {
                            tex += GlTex_Size(pal->data);
                        }
                    }
                }
            }
        }
        if (set->shared == NULL && set->sprites != NULL) {
            Sprite *spr;
            for (spr = KeyedArray_First(set->sprites); spr != NULL; spr = KeyedArray_Next(set->sprites)) {
                sys += sizeof(Sprite);
                if (spr->owns_pixels) {
                    sys += spr->pixels_size;
                    if (spr->format == SPRITE_FORMAT_TEXTURE && spr->pixels != NULL) {
                        tex += GlTex_Size(spr->pixels);
                    }
                }
            }
        }
        sys += sizeof(SpriteSet);
    }
    if (sys_bytes != NULL) {
        *sys_bytes = sys;
    }
    if (tex_bytes != NULL) {
        *tex_bytes = tex;
    }
}

/*
 * 0x00466100: a copy of w x h pixels of bpp bits keeping one pixel out of
 * each step_x x step_y block (the first pixel of the block's last row).
 */
void *Sprite_Downscale(const void *pixels, int w, int h, int step_x, int step_y, int bpp)
{
    int out_w = w / step_x;
    int bytes = (bpp + 7) >> 3;
    int out_h = h / step_y;
    const unsigned char *row;
    unsigned char *out;
    unsigned char *dst;
    int x, y, i;

    if (step_x < 1 || step_y < 1) {
        return NULL;
    }
    out = calloc(1, bytes * out_h * out_w);
    if (out == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    dst = out;
    row = (const unsigned char *)pixels + (step_y - 1) * bytes * w;
    for (y = 0; y < out_h; y++) {
        const unsigned char *src = row;
        for (x = 0; x < out_w; x++) {
            for (i = 0; i < bytes; i++) {
                *dst++ = *src++;
            }
            src += (step_x - 1) * bytes;
        }
        row += bytes * w * step_y;
    }
    return out;
}
