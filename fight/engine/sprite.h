#ifndef MUGEN_SPRITE_H
#define MUGEN_SPRITE_H

/*
 * Sprites, the sprite sets they are loaded into (SFF files, see sff.h) and
 * the parameters of the sprite drawing entry points.
 */

/* Sprite.format: SFF v2 compression, or one of these. */
#define SPRITE_FORMAT_RAW       0
#define SPRITE_FORMAT_SURFACE   1       /* SDL_Surface */
#define SPRITE_FORMAT_TEXTURE   100     /* OpenGL texture */

/*
 * Compressed formats that Sprite_Init decodes with SPRITE_INIT_DECODE (SFF v2:
 * 2 RLE8, 3 RLE5, 4 LZ5, 10-12 PNG; 5-9 have no decoder in g_spriteDecoders).
 */
#define SPRITE_FORMAT_DECODE_MIN    2
#define SPRITE_FORMAT_DECODE_MAX    12

/* Sprite_Init flags. */
#define SPRITE_INIT_COPY        0x01    /* copy the pixel data */
#define SPRITE_INIT_OWN         0x02    /* take ownership of the pixel data */
#define SPRITE_INIT_DECODE      0x04    /* decode compressed formats (2..12) at load */
#define SPRITE_INIT_NO_TEXTURE  0x08    /* never upload to a texture */

/* Largest texture side when the renderer is not RENDER_MODE_GL (g_glMaxTextureSize otherwise). */
#define SPRITE_MAX_TEXTURE_SIZE 0x100000

/* Bits per pixel of the target that Sprite_DrawSoft draws to. */
#define SPRITE_SOFT_BPP         16

typedef struct Sprite {
    int w;                  /* 0x00 */
    int h;                  /* 0x04 */
    int x;                  /* 0x08 axis */
    int y;                  /* 0x0C */
    int bpp;                /* 0x10 */
    int ncolors;            /* 0x14 1 << bpp for paletted sprites */
    int owns_pixels;        /* 0x18 pixels is freed with the sprite */
    int format;             /* 0x1C SPRITE_FORMAT_* or SFF v2 compression */
    int pixels_size;        /* 0x20 bytes of pixels (4 for a texture) */
    void *pixels;           /* 0x24 pixel data, SDL_Surface or texture */
    int owns_rows;          /* 0x28 rows is freed with the sprite */
    int *rows;              /* 0x2C offset of each row in pixels */
    int pal_index;          /* 0x30 slot in SpriteSet.palettes, -1 for none */
    float scale_x;          /* 0x34 >1 when the texture was downscaled */
    float scale_y;          /* 0x38 */
} Sprite;
MUGEN_ASSERT_OFFSET(Sprite, ncolors, 0x14);
MUGEN_ASSERT_OFFSET(Sprite, pal_index, 0x30);
MUGEN_ASSERT_SIZE(Sprite, 0x3C);

/* A loaded sprite file. */
typedef struct SpriteSet {
    KeyedArray *sprites;            /* 0x00 Sprite by (group, number) */
    void *data;                     /* 0x04 SFF v2: sprite and palette data block */
    KeyedArray *palettes;           /* 0x08 Palette by (group, number) */
    struct SpriteSet *shared;       /* 0x0C sprites borrowed from this set */
    int refs;                       /* 0x10 the set is only freed at 0 */
    unsigned int version;           /* 0x14 SFF version */
} SpriteSet;
MUGEN_ASSERT_SIZE(SpriteSet, 0x18);

/* Decoder of a compressed sprite format (g_spriteDecoders[format]). */
typedef int (__cdecl *SpriteDecodeFn)(const void *data, size_t size, void *out);

#endif /* MUGEN_SPRITE_H */
