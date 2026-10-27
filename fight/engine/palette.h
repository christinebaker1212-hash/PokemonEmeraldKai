#ifndef MUGEN_PALETTE_H
#define MUGEN_PALETTE_H

/* Pal_Init flags. */
#define PAL_COPY        0x01    /* copy the colours into a new buffer owned by the palette */
#define PAL_TAKE        0x02    /* the palette takes ownership of the caller's buffer */
#define PAL_NO_TEXTURE  0x08    /* keep the colours in memory even with the OpenGL renderer */

/* g_renderMode values that keep palettes as OpenGL textures. */
#define RENDER_MODE_GL      2
#define RENDER_MODE_GL_ALT  3

/*
 * Palette of up to 256 RGBA colours. With the OpenGL renderer the colours
 * live in a 1D texture; otherwise they stay in memory and are converted to
 * RGB565 for the software renderer.
 */
typedef struct Palette {
    int count;                  /* 0x000 number of colours */
    int owns;                   /* 0x004 data is released by Pal_Free */
    int is_texture;             /* 0x008 data is a GL texture (GlTex_*) */
    void *data;                 /* 0x00C count RGBA colours, or the GL texture */
    unsigned short rgb565[256]; /* 0x010 colours for the software renderer */
} Palette;
MUGEN_ASSERT_OFFSET(Palette, data, 0x00C);
MUGEN_ASSERT_OFFSET(Palette, rgb565, 0x010);
MUGEN_ASSERT_SIZE(Palette, 0x210);

/*
 * PalFX parameters, fixed point with 256 = 1.0. Colours are desaturated by
 * color, inverted (invertall), offset by add and scaled by mul, in that order.
 */
typedef struct PalFx {
    int color;                  /* 0x00 saturation: 0 = grey, 256 = full colour */
    int add[3];                 /* 0x04 r, g, b offsets */
    int mul[3];                 /* 0x10 r, g, b factors */
    int unused_1c;              /* 0x1C cleared on reset, never read */
    int invertall;              /* 0x20 */
    unsigned int serial;        /* 0x24 g_palFxSerial when last changed */
} PalFx;
MUGEN_ASSERT_OFFSET(PalFx, mul, 0x10);
MUGEN_ASSERT_OFFSET(PalFx, invertall, 0x20);
MUGEN_ASSERT_SIZE(PalFx, 0x28);

/* Palette remapping (RemapPal): colours of src_group,src_index come from dst_group,dst_index. */
typedef struct PalRemapEntry {
    int src_group;              /* 0x00 -1 matches any palette */
    int src_index;              /* 0x04 */
    int dst_group;              /* 0x08 -1 disables the entry */
    int dst_index;              /* 0x0C */
} PalRemapEntry;
MUGEN_ASSERT_SIZE(PalRemapEntry, 0x10);

#define PAL_REMAP_MAX 8

typedef struct PalRemap {
    int count;                                  /* 0x00 */
    PalRemapEntry entries[PAL_REMAP_MAX];       /* 0x04 */
} PalRemap;
MUGEN_ASSERT_SIZE(PalRemap, 0x84);

/*
 * A PalFX definition being applied: AllPalFX / BGPalFX (PalGroups_Apply) and the
 * player's own (Player.palfx_def: PalFX controller, HitDef palfx).
 */
typedef struct PalFxDef {
    int time;                   /* 0x00 ticks since it started (sinadd phase) */
    int active;                 /* 0x04 */
    int color;                  /* 0x08 256 = full colour */
    int add[3];                 /* 0x0C */
    int mul[3];                 /* 0x18 256 = 1.0 */
    int sinadd[3];              /* 0x24 */
    int invertall;              /* 0x30 */
    int period;                 /* 0x34 sinadd period, 0 = none */
    int reset;                  /* 0x38 reset the PalFx first */
} PalFxDef;
MUGEN_ASSERT_SIZE(PalFxDef, 0x3C);

#endif /* MUGEN_PALETTE_H */
