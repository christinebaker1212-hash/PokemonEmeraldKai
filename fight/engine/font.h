#ifndef MUGEN_FONT_H
#define MUGEN_FONT_H

/*
 * Fonts (font.c): FNT v1 files ("ElecbyteFnt": a PCX image and a text
 * section mapping characters to columns of it), FNT v2 definitions
 * ("[FNT v2]" .def files naming an SFF of glyph sprites or a TrueType file)
 * and the text drawing and measuring functions.
 */
struct _TTF_Font;           /* SDL_ttf */

/* Font.type */
#define FONT_TYPE_BITMAP    0
#define FONT_TYPE_TRUETYPE  1

/* Font.banktype: what the bank argument of the drawing functions selects. */
#define FONT_BANK_PALETTE   0       /* palette (0, bank) of the glyph sprites */
#define FONT_BANK_SPRITE    1       /* glyph sprites (bank, character) */

/* Text alignment (align argument of the drawing functions). */
#define FONT_ALIGN_CENTER   0
#define FONT_ALIGN_LEFT     1
#define FONT_ALIGN_RIGHT    (-1)

/* g_fontWrapMode */
#define FONT_WRAP_NONE      0
#define FONT_WRAP_CHAR      1       /* break lines at the right clip edge */
#define FONT_WRAP_WORD      2       /* break lines between words */

/* g_fontTtfRender: TrueType rendering, else Font.blend decides. */
#define FONT_TTF_SOLID      1
#define FONT_TTF_BLENDED    2

typedef struct Font {
    int type;                   /* 0x00 FONT_TYPE_* */
    int banktype;               /* 0x04 FONT_BANK_* */
    int offset_x;               /* 0x08 drawing offset ("offset"); FNT v1 negates y */
    int offset_y;               /* 0x0C */
    int size_w;                 /* 0x10 "size": width (spaces, missing glyphs), height (lines) */
    int size_h;                 /* 0x14 TrueType: point size */
    int variable;               /* 0x18 1, 0 for a FNT v1 "type = fixed" font; not used */
    int spacing_x;              /* 0x1C "spacing": added after each glyph, between lines */
    int spacing_y;              /* 0x20 */
    int blend;                  /* 0x24 FNT v2 "blend": TrueType text is rendered blended */
    SpriteSet *sprites;         /* 0x28 bitmap font glyphs, or NULL */
    struct _TTF_Font *ttf;      /* 0x2C TrueType font, or NULL */
    char name[0x40];            /* 0x30 FNT v2 "name", not terminated when 64 characters long */
} Font;
MUGEN_ASSERT_OFFSET(Font, sprites, 0x28);
MUGEN_ASSERT_OFFSET(Font, ttf, 0x2C);
MUGEN_ASSERT_SIZE(Font, 0x70);

/* ---- FNT v1 file format ---- */

#define FNT_VERSION_1       0x100

typedef struct FntHeader {
    char signature[12];         /* 0x00 "ElecbyteFnt" */
    int version;                /* 0x0C FNT_VERSION_1 */
    int pcx_offset;             /* 0x10 image, from the start of the file */
    int pcx_size;               /* 0x14 */
    int text_offset;            /* 0x18 "[Def]" and "[Map]" text */
    int text_size;              /* 0x1C */
    char comment[0x20];         /* 0x20 */
} FntHeader;
MUGEN_ASSERT_OFFSET(FntHeader, text_offset, 0x18);
MUGEN_ASSERT_SIZE(FntHeader, 0x40);

/* Glyph columns of a FNT v1 image, by character ("[Map]" lines: char x width). */
#define FNT_MAP_SIZE        128

typedef struct FntMap {
    int colors;                 /* 0x000 "colors" of [Def] */
    int x[FNT_MAP_SIZE];        /* 0x004 */
    int w[FNT_MAP_SIZE];        /* 0x204 */
} FntMap;
MUGEN_ASSERT_SIZE(FntMap, 0x404);

#endif /* MUGEN_FONT_H */
