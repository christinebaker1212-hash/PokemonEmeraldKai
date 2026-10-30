/*
 * Fonts (original module 0x46C3E0-0x46E22F): loading FNT v1 files (a PCX
 * image cut into glyph sprites by the "[Map]" section), FNT v2 definitions
 * (bitmap glyphs from an SFF, or a TrueType font through SDL_ttf), and the
 * text drawing (alignment, line and word wrapping, tabs, colour scaling
 * through PalFX) and measuring functions.
 */
#include <SDL_ttf.h>

#include "mugen.h"

#define FONT_LOG_ERROR      3
#define FONT_PRINTF_SIZE    0x1000      /* Font_Printf buffer, not terminated on overflow */
#define FONTS_MAX           10          /* font0 .. font9 of Fonts_Load */

/* SDL_gfx (no header in the SDK). */
extern SDL_Surface *__cdecl zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth);

/* The sprite in slot index of a keyed array of sprites (no check, as in the original). */
static Sprite *Font_SpriteAt(const KeyedArray *sprites, int index)
{
    return (Sprite *)(sprites->data + sprites->elem_size * index);
}

/* isspace() for the characters 1..127 only, as the original tests them. */
static int Font_IsSpace(int c)
{
    return (unsigned int)(c - 1) <= 0x7E && isspace(c);
}

/* Draw parameters of Font_DrawText and Font_DrawTextScaled: every field set, no blending. */
static void Font_DrawParams(DrawParams *dp, float x, float y, float scale_x, float scale_y)
{
    memset(dp, 0, sizeof(DrawParams));
    dp->width_ratio = 1.0f;
    dp->width_anchor = 0.5f;
    dp->x = x;
    dp->y = y;
    dp->scale_x = scale_x;
    dp->scale_y = scale_y;
    dp->scale2_x = 1.0f;
    dp->scale2_y = 1.0f;
    dp->top_stretch = 1.0f;
    dp->bottom_stretch = 1.0f;
    dp->masked = 1;
    dp->alpha_src = 1.0f;
    dp->flags = 0xFFFF;
}

/*
 * 0x0046C3E0: draws len characters of text (-1: up to the end of the line)
 * with a bitmap font, from (dp->x, dp->y) plus the font offset; the glyphs
 * hang above y (the baseline). Nothing is drawn when the start lies right of
 * or below the clip rectangle. With FONT_BANK_PALETTE, bank selects palette
 * (0, bank), or (0, 0) when it does not exist; with FONT_BANK_SPRITE it
 * selects the glyph group. r, g, b scale the colours through PalFX (255 and
 * up to 256 count as 1.0). TrueType fonts go to Font_DrawLineTTF.
 *
 * Quirks kept: the palette is passed to Sprite_DrawWithPalette as the slot
 * number bank (not the slot found); a tab advances by four tab widths
 * without snapping to a tab stop; '\r' still counts against len.
 */
void Font_DrawLine(const DrawParams *dp, Font *font, const char *text, int len, int bank, int r, int g, int b)
{
    float scale_x = dp->scale_x;
    float scale_y = dp->scale_y;
    float x0 = dp->x;
    float y0 = dp->y;
    int group = 0;
    DrawParams glyph = *dp;
    PalFx fx;
    PalFx *saved;
    float x, y;
    int pal_index;
    char c;

    if (font == NULL) {
        return;
    }
    if (font->type == FONT_TYPE_TRUETYPE) {
        Font_DrawLineTTF(text, dp, len, font, r, g, b);
        return;
    }
    x0 = (float)((double)font->offset_x * scale_x + x0);
    x = x0;
    y = (float)((double)(font->offset_y - font->size_h) * scale_y + 1.0 + y0);
    if ((double)g_clipX2 <= x) {
        return;
    }
    if ((double)g_clipY2 <= y) {
        return;
    }
    if (font->banktype == FONT_BANK_PALETTE) {
        if (font->sprites->palettes != NULL) {
            pal_index = KeyedArray_Find(font->sprites->palettes, 0, bank);
            if (pal_index != -1) {
                goto have_palette;
            }
        }
        bank = 0;
        if (font->sprites->palettes != NULL) {
            pal_index = KeyedArray_Find(font->sprites->palettes, 0, 0);
            goto have_palette;
        }
    } else {
        group = bank;
    }
    pal_index = -1;
have_palette:
    if (r == 0xFF) {
        r = 0x100;
    }
    if (g == 0xFF) {
        g = 0x100;
    }
    if (b == 0xFF) {
        b = 0x100;
    }
    saved = g_palFxCur;
    g_palFxCur = PalFx_ScaleMul(PalFx_Init(&fx, saved), r, g, b);
    c = *text++;
    while (c != '\0' && c != '\n' && len != 0) {
        if (c == '\t') {
            float tab = (float)(font->spacing_x + font->size_w);
            x = (float)(((((double)x - x0) / tab) / scale_x * 0.25 + 1.0) * 4.0 * tab * scale_x + x0);
        } else if (c != '\r') {
            if (c == ' ') {
                x = (float)((double)(font->spacing_x + font->size_w) * scale_x + x);
            } else {
                int index = KeyedArray_Find(font->sprites->sprites, group, c);
                float advance = (float)font->size_w;

                if (index != -1) {
                    glyph.x = x;
                    glyph.y = y;
                    if (pal_index != -1) {
                        Sprite_DrawWithPalette(font->sprites, index, bank, &glyph);
                    } else {
                        Sprite_Draw(font->sprites, index, &glyph);
                    }
                    advance = (float)Font_SpriteAt(font->sprites->sprites, index)->w;
                }
                x = (float)(((double)font->spacing_x + advance) * scale_x + x);
            }
        }
        if (len != -1) {
            len--;
        }
        c = *text++;
    }
    g_palFxCur = saved;
}

/*
 * 0x0046C6F0: Font_DrawLine for TrueType fonts. The text is cut at the
 * first newline, rendered by SDL_ttf (solid, or blended when the font's
 * blend or g_fontTtfRender says so) with the colour r, g, b (clamped to 255)
 * and drawn with its top at the font ascent above y: with the OpenGL
 * renderers as a temporary 32-bit sprite (unscaled position), otherwise
 * blitted onto the render target surface, zoomed when scaled.
 *
 * Quirks kept: the copy of the text leaks when SDL_ttf fails; the sprite set
 * leaks when its tables cannot be created.
 */
void Font_DrawLineTTF(const char *text, const DrawParams *dp, int len, Font *font, int r, int g, int b)
{
    float scale_x = dp->scale_x;
    float scale_y = dp->scale_y;
    float x = dp->x;
    float y = dp->y;
    char *dup = NULL;
    DrawParams sprite_dp = *dp;
    const char *nl = strchr(text, '\n');
    SDL_Surface *surface;
    SDL_Color color;
    SDL_Rect dst;
    int blended;

    if (nl != NULL && ((int)(nl - text) < len || len == -1)) {
        len = (int)(nl - text);
    }
    if (len == 0) {
        return;
    }
    if (len > 0) {
        dup = malloc(len + 1);
        if (dup == NULL) {
            return;
        }
        strncpy(dup, text, len);
        dup[len] = '\0';
        text = dup;
    }
    if (r > 0xFF) {
        r = 0xFF;
    }
    if (g > 0xFF) {
        g = 0xFF;
    }
    if (b > 0xFF) {
        b = 0xFF;
    }
    color.r = (Uint8)r;
    color.g = (Uint8)g;
    color.b = (Uint8)b;
    color.unused = 0;
    blended = font->blend;
    if (g_fontTtfRender == FONT_TTF_SOLID) {
        blended = 0;
    } else if (g_fontTtfRender == FONT_TTF_BLENDED) {
        blended = 1;
    }
    dst.x = (Sint16)(int)((double)font->offset_x * scale_x + x);
    dst.y = (Sint16)(int)((double)(font->offset_y - TTF_FontAscent(font->ttf)) * scale_y + y);
    if (blended) {
        surface = TTF_RenderUTF8_Blended(font->ttf, text, color);
    } else {
        surface = TTF_RenderUTF8_Solid(font->ttf, text, color);
    }
    if (surface == NULL) {
        return;
    }
    if (g_renderMode == RENDER_MODE_GL || g_renderMode == RENDER_MODE_GL_ALT) {
        SDL_Surface *rgba = Surface_ToRGBA32(surface);

        if (rgba != NULL) {
            int bpp = rgba->format->BitsPerPixel;
            int w = rgba->w;
            int h = rgba->h;
            void *pixels = rgba->pixels;
            int size = ((bpp + 7) >> 3) * w * h;

            if (bpp > 8 && bpp >= 24) {
                SpriteSet *set = calloc(1, sizeof(SpriteSet));

                if (set == NULL) {
                    g_outOfMemory = 1;
                    SDL_FreeSurface(rgba);
                    goto done;
                }
                set->sprites = KeyedArray_Create(1, sizeof(Sprite));
                if (set->sprites != NULL) {
                    set->palettes = KeyedArray_Create(1, sizeof(Palette));
                    if (set->palettes != NULL) {
                        Sprite spr;

                        if (Sprite_Init(0, &spr, SPRITE_FORMAT_RAW, pixels, size, w, h, bpp, 0, 0)) {
                            SpriteSet_AddSprite(set, &spr, 0, 0);
                            sprite_dp.x = (float)dst.x;
                            sprite_dp.y = (float)dst.y;
                            Sprite_Draw(set, 0, &sprite_dp);
                        }
                        SpriteSet_Free(set);
                    }
                }
            }
            SDL_FreeSurface(rgba);
        }
    } else if (scale_x == 1.0f && scale_y == 1.0f) {
        SDL_UpperBlit(surface, NULL, SpriteSet_GetSurface(g_renderTarget, g_renderTargetSprite), &dst);
    } else {
        SDL_Surface *zoomed = zoomSurface(surface, scale_x, scale_y, 0);

        if (zoomed != NULL) {
            SDL_UpperBlit(zoomed, NULL, SpriteSet_GetSurface(g_renderTarget, g_renderTargetSprite), &dst);
            SDL_FreeSurface(zoomed);
        }
    }
done:
    SDL_FreeSurface(surface);
    if (dup != NULL) {
        free(dup);
    }
}

/*
 * 0x0046CA80: draws text with the draw parameters dp. Without wrapping
 * (g_fontWrapMode) the text goes to Font_DrawLines as it is; otherwise it is
 * cut into lines that fit between dp->x and the right clip edge, breaking at
 * characters or at words (the space a line ends with is not drawn).
 * g_fontLineCount receives the number of lines.
 */
void Font_DrawTextEx(const char *text, Font *font, const DrawParams *dp, int align, int bank, int r, int g, int b)
{
    float scale_y = dp->scale_y;
    DrawParams line = *dp;
    int word;

    g_fontLineCount = 0;
    if (g_fontWrapMode == FONT_WRAP_NONE) {
        Font_DrawLines(text, -1, font, dp, align, bank, r, g, b);
        g_fontLineCount++;
        return;
    }
    word = g_fontWrapMode == FONT_WRAP_WORD;
    while (*text != '\0') {
        int max_width = (int)(((double)g_clipX2 - dp->x) / dp->scale_x);
        int n = Font_WrapLength(text, font, max_width, word);
        int drop;

        if (n == 0) {
            n = 1;
        }
        drop = g_fontWrapMode == FONT_WRAP_WORD && Font_IsSpace(text[n - 1]) ? -1 : 0;
        Font_DrawLines(text, n + drop, font, &line, align, bank, r, g, b);
        g_fontLineCount++;
        text += n;
        line.y = (float)((double)(font->size_h + font->spacing_y) * scale_y + line.y);
    }
}

/*
 * 0x0046CBC0: draws len characters of text (-1: all), one line per '\n',
 * each aligned on dp->x (FONT_ALIGN_*: centred and right-aligned lines are
 * measured first). Stops at the first line whose glyphs would start below
 * the clip rectangle; nothing is drawn with a zero or negative scale.
 */
void Font_DrawLines(const char *text, int len, Font *font, const DrawParams *dp, int align, int bank, int r, int g,
                    int b)
{
    float width = 0.0f;
    float scale_x = dp->scale_x;
    float scale_y = dp->scale_y;
    int measure = align != FONT_ALIGN_LEFT;
    DrawParams line = *dp;
    int pos = 0;
    float y;
    char c;

    if (font == NULL || scale_x <= 0.0f || scale_y <= 0.0f) {
        return;
    }
    y = dp->y;
    if (*text == '\0') {
        return;
    }
    do {
        int start;
        int count = 0;

        if (len == 0) {
            return;
        }
        if ((double)g_clipY2 <= (double)y - (double)font->size_h * scale_y + 1.0) {
            return;
        }
        start = pos;
        if (measure) {
            int line_len = 0;
            const char *p = text + pos;
            char *dup = Font_DupLine(len, p, &line_len);
            const char *str;

            if (dup != NULL) {
                count = line_len;
                str = dup;
            } else {
                str = p;
                count = (int)strlen(p);
                if (len > 0 && count > len) {
                    count = len;
                }
            }
            width = (float)Font_TextWidth(str, font);
            if (dup != NULL) {
                free(dup);
            }
            width = scale_x * width;
            pos += count;
        } else {
            c = text[pos];
            while (c != '\0' && c != '\n' && (count < len || len <= 0)) {
                c = text[pos + 1];
                pos++;
                count++;
            }
        }
        c = text[pos];
        if (start < pos) {
            if (align == FONT_ALIGN_CENTER) {
                line.x = (float)(dp->x - width * 0.5);
            } else if (align == FONT_ALIGN_RIGHT) {
                line.x = (float)((double)dp->x - width + 1.0);
            } else {
                line.x = dp->x;
            }
            line.y = y;
            Font_DrawLine(&line, font, text + start, count, bank, r, g, b);
            len -= count;
        }
        pos++;
        y = (float)((double)y + (double)(font->spacing_y + font->size_h) * scale_y);
    } while (c != '\0');
}

/* 0x0046CE40: draws text at (x, y), unscaled (see Font_DrawTextEx). */
void Font_DrawText(const char *text, Font *font, int x, int y, int align, int bank, int r, int g, int b)
{
    DrawParams dp;

    Font_DrawParams(&dp, (float)x, (float)y, 1.0f, 1.0f);
    Font_DrawTextEx(text, font, &dp, align, bank, r, g, b);
}

/* 0x0046CED0: draws text at (x, y) with a scale (see Font_DrawTextEx). */
void Font_DrawTextScaled(const char *text, Font *font, float x, float y, float scale_x, float scale_y, int align,
                         int bank, int r, int g, int b)
{
    DrawParams dp;

    Font_DrawParams(&dp, x, y, scale_x, scale_y);
    Font_DrawTextEx(text, font, &dp, align, bank, r, g, b);
}

/* 0x0046CF90: Font_DrawText of a formatted string (at most FONT_PRINTF_SIZE characters). */
void Font_Printf(Font *font, int x, int y, int align, int bank, int r, int g, int b, const char *fmt, ...)
{
    char text[FONT_PRINTF_SIZE];
    va_list ap;

    va_start(ap, fmt);
    _vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);
    Font_DrawText(text, font, x, y, align, bank, r, g, b);
}

/*
 * 0x0046D030: loads a font file: FNT v1, or a FNT v2 definition, whose
 * files are found next to it. height > 0 sets the size of TrueType fonts.
 */
Font *Font_Load(const char *path, int height)
{
    char dir[0x204];
    Stream *s = Stream_OpenFile(path, "rb");
    Font *font;

    if (s == NULL) {
        return NULL;
    }
    if (path[0] == '\0') {
        dir[0] = '\0';
    } else {
        int n = (int)strlen(path) - 1;

        while (n >= 0 && path[n] != '\\' && path[n] != '/') {
            n--;
        }
        n++;
        if (n > 0) {
            strncpy(dir, path, n);
        }
        dir[n] = '\0';
    }
    font = Font_LoadStream(s, dir, height);
    Stream_Close(s);
    return font;
}

/*
 * 0x0046D100: loads a font from s: FNT v1 when it starts with an
 * "ElecbyteFnt" header (other versions give NULL), else a FNT v2
 * definition. The stream is left where it was.
 */
Font *Font_LoadStream(Stream *s, const char *dir, int height)
{
    long start = Stream_Tell(s);
    Font *font = NULL;
    FntHeader hdr;

    memset(&hdr, 0, sizeof(hdr));
    Stream_Read(&hdr, sizeof(hdr), 1, s);
    if (strcmp(hdr.signature, "ElecbyteFnt") == 0) {
        if (hdr.version == FNT_VERSION_1) {
            font = Font_LoadFnt1(s, &hdr);
        }
    } else {
        Stream_Seek(s, start, SEEK_SET);
        font = Font_LoadDef(s, dir, height);
    }
    Stream_Seek(s, start, SEEK_SET);
    return font;
}

/*
 * Locals of Font_LoadFnt1 in the layout of the original stack frame (from
 * esp + 0x10 on). "[Map]" lines index map.x and map.w with their character
 * unchecked: characters 128..255 (and negative ones, from bytes >= 0x80) as
 * well as "0x" codes >= 0x80 reach the neighbouring locals. FntFrame_Slot
 * reproduces those accesses inside the frame.
 */
typedef struct FntFrame {
    int unused_00[4];           /* 0x000 */
    Font *font;                 /* 0x010 */
    long start;                 /* 0x014 stream position after the header */
    int x;                      /* 0x018 column of the next glyph */
    int nscan;                  /* 0x01C numbers read from the last "[Map]" line */
    unsigned int code;          /* 0x020 character of the line */
    Stream *s;                  /* 0x024 */
    FntMap map;                 /* 0x028 */
    int unused_42c;             /* 0x42C */
    Palette src_pal;            /* 0x430 palette of the image */
    Palette pal;                /* 0x640 */
    int unused_850;             /* 0x850 */
} FntFrame;
MUGEN_ASSERT_OFFSET(FntFrame, map, 0x028);
MUGEN_ASSERT_OFFSET(FntFrame, src_pal, 0x430);
MUGEN_ASSERT_SIZE(FntFrame, 0x854);

#define FNT_FRAME_X     0x02C   /* offset of map.x */
#define FNT_FRAME_W     0x22C   /* offset of map.w */

/*
 * Element index of the frame array at offset base, with the original's 32-bit
 * address arithmetic. Outside the frame the original wrote to unused stack
 * below it, or to the security cookie and saved registers above it (which
 * ends the process); such writes are dropped here and such reads give 0.
 */
static int *FntFrame_Slot(FntFrame *f, int base, unsigned int index)
{
    int offset = (int)((unsigned int)base + index * 4u);

    if (offset < 0 || offset > (int)sizeof(FntFrame) - 4) {
        return NULL;
    }
    return (int *)((char *)f + offset);
}

static void FntFrame_Set(FntFrame *f, int base, unsigned int index, int value)
{
    int *slot = FntFrame_Slot(f, base, index);

    if (slot != NULL) {
        *slot = value;
    }
}

static int FntFrame_Get(FntFrame *f, int base, unsigned int index)
{
    int *slot = FntFrame_Slot(f, base, index);

    return slot != NULL ? *slot : 0;
}

/*
 * 0x0046D260: loads a FNT v1 font from s, positioned after hdr. The PCX
 * image becomes sprite (0, 0) of the font's set; "[Def]" gives the metrics
 * and "colors", "[Map]" the column and width of each character (one line per
 * character: "c x w", "0xNN x w", or just "c" for a glyph as wide as the
 * font, the columns following each other). With more than one colour the
 * image palette is split into palettes (0, n) of the next power of two of
 * colours, taken from its end, and the image pixels are rebased to them.
 * The glyphs are then cut out by Font_SplitGlyphs.
 *
 * Quirks kept: map characters outside 0..127 (see FntFrame); the locals the
 * original leaves uninitialised (nscan before the first line, the unused
 * frame slots and the image palette copy before it is made) start at 0 here,
 * where the original has stack garbage; the parsed text leaks on errors after "[Def]" was found, and
 * the palettes added so far leak when one cannot be added.
 */
Font *Font_LoadFnt1(Stream *s, const FntHeader *hdr)
{
    FntFrame f;
    Font *font;
    Ini *ini;
    const char *line;
    const char *type;
    long base;

    memset(&f, 0, sizeof(f));
    f.s = s;
    f.start = Stream_Tell(s);
    base = f.start - (long)sizeof(FntHeader);
    font = calloc(1, sizeof(Font));
    f.font = font;
    if (font == NULL) {
        return NULL;
    }
    memset(&f.map, 0, sizeof(f.map));
    Stream_Seek(s, hdr->pcx_offset + base, SEEK_SET);
    font->sprites = Pcx_Load(s, 1);
    if (font->sprites == NULL) {
        if (font->ttf != NULL) {
            TTF_CloseFont(font->ttf);
        }
        free(font);
        return NULL;
    }
    font->type = FONT_TYPE_BITMAP;
    font->variable = 1;
    Stream_Seek(s, hdr->text_offset + base, SEEK_SET);
    ini = Ini_Parse(s, INI_MODE_READ);
    if (ini == NULL || Ini_SelectGroup(ini, "def") == -1) {
        goto fail;
    }
    Ini_Scanf(ini, "size", "%i,%i", &font->size_w, &font->size_h);
    Ini_Scanf(ini, "spacing", "%i,%i", &font->spacing_x, &font->spacing_y);
    Ini_GetInt(ini, "colors", &f.map.colors);
    Ini_Scanf(ini, "offset", "%i,%i", &font->offset_x, &font->offset_y);
    font->offset_y = -font->offset_y;
    type = Ini_GetValue(ini, "type");
    if (type != NULL && (type[0] == 'f' || type[0] == 'F')) {
        font->variable = 0;
    }
    if (Ini_SelectGroup(ini, "map") == -1) {
        goto fail;
    }
    for (line = Ini_FirstLine(ini); line != NULL; line = Ini_NextLine(ini)) {
        unsigned int code;
        int i = 2;

        f.code = (unsigned int)(int)(signed char)line[0];
        code = f.code;
        if (line[1] == '\0') {
            f.nscan = 0;
            goto default_x;
        }
        if (line[0] == '0' && line[1] == 'x') {
            sscanf(line + 2, "%x", &f.code);
            while (line[i] != '\0' && line[i] != ' ') {
                i++;
            }
            while (line[i] != '\0' && line[i] == ' ') {
                i++;
            }
            code = f.code;
        }
        if (code <= 0x7F) {
            f.nscan = sscanf(line + i, "%i %i", &f.map.x[code], &f.map.w[code]);
            code = f.code;
        }
        if (f.nscan < 1) {
default_x:
            FntFrame_Set(&f, FNT_FRAME_X, code, f.x);
        }
        if (f.nscan < 2) {
            FntFrame_Set(&f, FNT_FRAME_W, code, font->size_w);
        }
        f.x += FntFrame_Get(&f, FNT_FRAME_W, code);
    }
    f.map.w[' '] = font->size_w;
    if (font->size_w == 0 || font->size_h == 0 || f.map.colors == 0) {
        goto fail;
    }
    Ini_Free(ini);

    if (f.map.colors > 1) {
        unsigned int n = (unsigned int)(f.map.colors * 2 - 2);
        SpriteSet *set = font->sprites;
        Sprite *image;
        int bit, offset, number = 0;
        int i;

        /* Highest power of two in 2 * colors - 2 (the next one from colors). */
        for (bit = 31; bit != 0; bit--) {
            if ((int)n >> bit != 0) {
                n = 1u << bit;
                break;
            }
        }
        f.src_pal = *(Palette *)set->palettes->data;
        KeyedArray_Remove(0, set->palettes);
        for (offset = 0x100 - (int)n; offset >= 0; offset -= (int)n) {
            unsigned char *colors = (unsigned char *)(f.src_pal.is_texture ? NULL : f.src_pal.data) + offset * 4;

            colors[3] = 0;
            Pal_Init(&f.pal, (int)n, colors, PAL_COPY);
            if (!SpriteSet_AddPalette(f.font->sprites, &f.pal, 0, number++)) {
                Font_Free(f.font);
                return NULL;
            }
        }
        Pal_Free(&f.src_pal);
        image = Font_SpriteAt(f.font->sprites->sprites, 0);
        image->ncolors = (int)n;
        for (bit = 0; bit < 32; bit++) {
            if ((1u << bit) >= n) {
                break;
            }
        }
        image->bpp = bit;
        for (i = 0; i < image->pixels_size; i++) {
            unsigned char *p = (unsigned char *)image->pixels + i;

            if (*p != 0) {
                *p = (unsigned char)(*p + (unsigned char)n);
                if ((int)*p >= (int)n) {
                    *p = 0;
                }
            }
        }
        font = f.font;
    }
    if (!Font_SplitGlyphs(font, &f.map)) {
        if (font->sprites != NULL) {
            SpriteSet_Free(font->sprites);
        }
        if (font->ttf != NULL) {
            TTF_CloseFont(font->ttf);
        }
        free(font);
        return NULL;
    }
    Stream_Seek(f.s, f.start, SEEK_SET);
    return font;

fail:
    Font_Free(font);
    return NULL;
}

/*
 * 0x0046D700: cuts the glyphs of a FNT v1 font out of its image (sprite
 * (0, 0), which must be raw pixels): characters 1..127 with a width become
 * sprites (0, character) linked to palette (0, 0).
 *
 * Quirk kept: with the software renderer, the result of an SDL_MapRGB on the
 * render target surface is not used, and a render target that is not a
 * surface sprite (or none) crashes.
 */
int Font_SplitGlyphs(Font *font, const FntMap *map)
{
    Sprite *image;
    int index;
    int bytes, h;
    int i;

    if (g_renderMode != RENDER_MODE_GL && g_renderMode != RENDER_MODE_GL_ALT) {
        Sprite *target = Font_SpriteAt(g_renderTarget->sprites, g_renderTargetSprite);
        SDL_Surface *surface = target != NULL && target->format == SPRITE_FORMAT_SURFACE ? target->pixels : NULL;

        SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF);
    }
    index = KeyedArray_Find(font->sprites->sprites, 0, 0);
    image = Font_SpriteAt(font->sprites->sprites, index);
    if (image == NULL || image->format != SPRITE_FORMAT_RAW) {
        return 0;
    }
    h = image->h;
    bytes = (image->bpp + 7) >> 3;
    for (i = 1; i < FNT_MAP_SIZE; i++) {
        int w = map->w[i];

        if (w > 0) {
            int size = w * bytes * h;
            void *pixels = calloc(1, size);
            Sprite glyph;

            if (pixels == NULL) {
                g_outOfMemory = 1;
                return 0;
            }
            Font_CopyGlyph(image, pixels, map->x[i], w);
            if (!Sprite_Init(SPRITE_INIT_OWN, &glyph, SPRITE_FORMAT_RAW, pixels, size, w, h, image->bpp, 0, 0)) {
                return 0;
            }
            if (!SpriteSet_AddSprite(font->sprites, &glyph, 0, i)) {
                return 0;
            }
            image = Font_SpriteAt(font->sprites->sprites, index);
            SpriteSet_LinkPalette(font->sprites,
                                  Font_SpriteAt(font->sprites->sprites, KeyedArray_Find(font->sprites->sprites, 0, i)),
                                  0, 0);
        }
    }
    return 1;
}

/*
 * 0x0046D8C0: copies the columns x .. x + w - 1 of every row of image to dst.
 * Quirk kept: no bounds check; columns past the right edge come from the next
 * row, or from beyond the image buffer on the last row (font/f-6x8f.fnt's '|'
 * reads one byte past it).
 */
void Font_CopyGlyph(const Sprite *image, void *dst, int x, int w)
{
    int bytes = (image->bpp + 7) >> 3;
    unsigned char *out = dst;
    int row;

    for (row = 0; row < image->h; row++) {
        memcpy(out, (const unsigned char *)image->pixels + image->rows[row] + bytes * x, bytes * w);
        out += bytes * w;
    }
}

/*
 * 0x0046D910: loads a FNT v2 definition from s: "[FNT v2]" name, "[Def]"
 * type (bitmap / truetype), banktype (palette / sprite), size, spacing,
 * offset, blend and file, looked up in dir (TrueType files also in the
 * Windows fonts folder). height > 0 sets the size of a TrueType font, the
 * width scaled with it. The stream is left where it was.
 *
 * Quirk kept: the parsed text leaks on every error after it was parsed.
 */
Font *Font_LoadDef(Stream *s, const char *dir, int height)
{
    char path[0x204];
    long start;
    Font *font;
    Ini *ini;
    const char *value;
    const char *file;

    if (dir == NULL) {
        dir = "";
    }
    start = Stream_Tell(s);
    font = calloc(1, sizeof(Font));
    if (font == NULL) {
        return NULL;
    }
    font->type = FONT_TYPE_BITMAP;
    font->variable = 1;
    ini = Ini_Parse(s, INI_MODE_READ);
    if (ini == NULL) {
        if (font->sprites != NULL) {
            SpriteSet_Free(font->sprites);
        }
        if (font->ttf != NULL) {
            TTF_CloseFont(font->ttf);
        }
        free(font);
        return NULL;
    }
    if (Ini_SelectGroup(ini, "fnt v2") == -1) {
        goto fail;
    }
    value = Ini_GetValue(ini, "name");
    if (value != NULL) {
        strncpy(font->name, value, sizeof(font->name));
    }
    if (Ini_SelectGroup(ini, "def") == -1) {
        goto fail;
    }
    value = Ini_GetValue(ini, "type");
    if (value != NULL) {
        if (_stricmp(value, "truetype") == 0) {
            font->type = FONT_TYPE_TRUETYPE;
        } else if (_stricmp(value, "bitmap") == 0) {
            font->type = FONT_TYPE_BITMAP;
        } else {
            MatchLog_Printf(FONT_LOG_ERROR, "Invalid font type: %s\n", value);
            goto fail;
        }
    }
    value = Ini_GetValue(ini, "banktype");
    if (value != NULL) {
        if (_stricmp(value, "sprite") == 0) {
            font->banktype = FONT_BANK_SPRITE;
        } else if (_stricmp(value, "palette") == 0) {
            font->banktype = FONT_BANK_PALETTE;
        } else {
            MatchLog_Printf(FONT_LOG_ERROR, "Invalid font banktype: %s\n", value);
            Font_Free(font);
            return NULL;
        }
    }
    Ini_Scanf(ini, "size", "%i,%i", &font->size_w, &font->size_h);
    Ini_Scanf(ini, "spacing", "%i,%i", &font->spacing_x, &font->spacing_y);
    Ini_Scanf(ini, "offset", "%i,%i", &font->offset_x, &font->offset_y);
    Ini_Scanf(ini, "blend", "%i", &font->blend);
    if (height > 0 && font->type == FONT_TYPE_TRUETYPE) {
        font->size_w = font->size_w * height / font->size_h;
        if (font->size_w < 1) {
            font->size_w = 1;
        }
        font->size_h = height;
    }
    file = Ini_GetValue(ini, "file");
    if (file == NULL) {
        goto fail;
    }
    Path_Join(file, dir, path);
    if (font->type == FONT_TYPE_BITMAP) {
        font->sprites = Sff_Load(path);
        if (font->sprites == NULL) {
            Font_Free(font);
            return NULL;
        }
    } else if (font->type == FONT_TYPE_TRUETYPE) {
        font->ttf = TTF_OpenFont(path, font->size_h);
        if (font->ttf == NULL && g_systemFontDirPtr != NULL) {
            Path_Join(file, g_systemFontDirPtr, path);
            font->ttf = TTF_OpenFont(path, font->size_h);
        }
        if (font->ttf == NULL) {
            goto fail;
        }
    }
    Ini_Free(ini);
    Stream_Seek(s, start, SEEK_SET);
    return font;

fail:
    Font_Free(font);
    return NULL;
}

/* 0x0046DC60: frees a font (NULL is accepted); returns 1. */
int Font_Free(Font *font)
{
    if (font != NULL) {
        if (font->sprites != NULL) {
            SpriteSet_Free(font->sprites);
        }
        if (font->ttf != NULL) {
            TTF_CloseFont(font->ttf);
        }
        free(font);
    }
    return 1;
}

/*
 * 0x0046DCA0: width of character c: the glyph sprite (0, c) of a bitmap
 * font (the font width for spaces, 0 when missing), or the SDL_ttf width of
 * c alone (left unset when SDL_ttf fails, as in the original).
 */
int Font_GlyphWidth(int c, Font *font)
{
    if (font->type == FONT_TYPE_BITMAP) {
        KeyedArray *sprites;
        int index;

        if (c == ' ') {
            return font->size_w;
        }
        sprites = font->sprites->sprites;
        index = KeyedArray_Find(sprites, 0, c);
        if (index != -1) {
            Sprite *spr = Font_SpriteAt(sprites, index);
            if (spr != NULL) {
                return spr->w;
            }
        }
    } else if (font->type == FONT_TYPE_TRUETYPE) {
        char text[2];
        int w, h;

        text[0] = (char)c;
        text[1] = '\0';
        TTF_SizeUTF8(font->ttf, text, &w, &h);
        return w;
    }
    return 0;
}

/*
 * 0x0046DD60: width of the widest line of s. Bitmap fonts add the spacing
 * after each glyph but the last; TrueType lines are measured with their
 * '\n' and cut at 4095 bytes (a longer last line is measured unterminated,
 * as in the original).
 */
int Font_TextWidth(const char *s, Font *font)
{
    char line[0x1000];
    int spacing = font->spacing_x;
    int max = 0;

    if (font->type == FONT_TYPE_BITMAP) {
        int cur = 0;

        if (*s == '\0') {
            return 0;
        }
        do {
            if (*s == '\n') {
                if (max < cur) {
                    max = cur;
                }
                cur = 0;
            } else {
                cur += Font_GlyphWidth(*s, font) + spacing;
            }
            s++;
        } while (*s != '\0');
        if (max < cur) {
            max = cur;
        }
        if (max > 0) {
            max -= spacing;
        }
        return max;
    }
    if (font->type == FONT_TYPE_TRUETYPE) {
        unsigned int n = 0;

        for (;;) {
            if (n < sizeof(line) - 1) {
                line[n++] = *s;
            }
            if (*s == '\n') {
                line[n] = '\0';
            } else if (*s != '\0') {
                s++;
                continue;
            }
            {
                int w = 0, h;
                TTF_SizeUTF8(font->ttf, line, &w, &h);
                if (max < w) {
                    max = w;
                }
            }
            n = 0;
            if (*s == '\0') {
                break;
            }
            s++;
        }
    }
    return max;
}

/*
 * 0x0046DE60: number of characters of text that fit in max_width pixels on
 * one line (a newline is included and ends the line; tabs advance to the
 * next multiple of four font widths). With word_wrap the line is cut after
 * the last space or '-', unless a single word does not fit. TrueType fonts
 * go to Font_WrapLengthTTF.
 */
int Font_WrapLength(const char *text, Font *font, int max_width, int word_wrap)
{
    int spacing = font->spacing_x;
    int n = 0;
    int w = 0;
    int saved;
    char c;

    if (font->type == FONT_TYPE_TRUETYPE) {
        return Font_WrapLengthTTF(font, text, max_width, word_wrap);
    }
    c = *text;
    if (c != '\0') {
        for (;;) {
            if (c == '\n') {
                n++;
                break;
            }
            if (c == '\t') {
                int tab = font->size_w + spacing;
                w = (w / tab / 4 + 1) * tab * 4;
            } else {
                w += Font_GlyphWidth(c, font);
            }
            if (w > max_width) {
                if (!word_wrap) {
                    return n;
                }
                if (Font_IsSpace(*text)) {
                    n++;
                }
                break;
            }
            c = *++text;
            w += spacing;
            n++;
            if (c == '\0') {
                break;
            }
        }
    }
    if (!word_wrap || n <= 0 || w <= max_width) {
        return n;
    }
    if (Font_IsSpace(*text) || *text == '-' || *text == '\0') {
        return n;
    }
    /* Back to the last space or '-' (the glyph widths are looked up and dropped). */
    saved = n;
    text--;
    Font_GlyphWidth(*text, font);
    n--;
    for (;;) {
        if (Font_IsSpace(*text) || *text == '-') {
            return n + 1;
        }
        if (n <= 0) {
            return n != 0 ? n + 1 : saved;
        }
        Font_GlyphWidth(*text, font);
        n--;
        text--;
    }
}

/*
 * 0x0046DFD0: Font_WrapLength for TrueType fonts, measuring the line of
 * text up to each UTF-8 character. A line that fits whole counts one past
 * its end (its '\n', or the terminator of the last line) without word_wrap.
 *
 * Quirk kept: when the last line fits, that count makes the caller step
 * past the terminator of the text (and the original reads the byte after
 * it here).
 */
int Font_WrapLengthTTF(Font *font, const char *text, int max_width, int word_wrap)
{
    const char *nl;
    char *buf = NULL;
    int len = 0;
    int last_break = 0;
    int prev = 0;
    int before = 0;
    int i;

    if (*text == '\0') {
        return 0;
    }
    nl = strchr(text, '\n');
    if (nl != NULL) {
        len = (int)(nl - text);
        buf = malloc(len + 1);
    }
    if (buf == NULL) {
        len = (int)strlen(text);
        buf = malloc(len + 1);
        if (buf == NULL) {
            return 0;
        }
    }
    strncpy(buf, text, len);
    buf[len] = '\0';
    i = 1;
    if (len >= 1) {
        do {
            char ch = buf[i];

            if ((ch & 0xC0) != 0x80) {
                char c;

                before = prev;
                prev = i;
                buf[i] = '\0';
                if (Font_TextWidth(buf, font) > max_width) {
                    i = before;
                    break;
                }
                buf[i] = ch;
                c = buf[before];
                if (Font_IsSpace(c) || c == '-' || i == len) {
                    last_break = i;
                }
            }
            i++;
        } while (i <= len);
    }
    if (last_break == 0) {
        last_break = before;
    }
    if (last_break < 0) {
        last_break = 0;
    }
    free(buf);
    if (word_wrap) {
        i = last_break;
    }
    if (text[i] == '\n') {
        i++;
    }
    return i;
}

/*
 * 0x0046E120: loads fonts font0 .. font9 of the selected group of ini (with
 * "fontN.height"), trying the directories of dirs (NULL-terminated; by
 * default the current one) in order. Missing fonts are NULL.
 */
void Fonts_Load(Ini *ini, Font **fonts, const char *const *dirs)
{
    char key[0x104];
    int i;

    if (dirs == NULL) {
        dirs = g_fontDefaultDirs;
    }
    for (i = 0; i < FONTS_MAX; i++, fonts++) {
        const char *value;
        const char *file;
        int height = 0;

        *fonts = NULL;
        sprintf(key, "%s%i.height", "font", i);
        value = Ini_GetValue(ini, key);
        if (value != NULL) {
            height = atoi(value);
        }
        sprintf(key, "%s%i", "font", i);
        file = Ini_GetValue(ini, key);
        if (file != NULL && *fonts == NULL) {
            const char *const *dir;

            for (dir = dirs; *dir != NULL; dir++) {
                Path_Join(file, *dir, key);
                *fonts = Font_Load(key, height);
                if (*fonts != NULL) {
                    break;
                }
            }
        }
    }
}

/*
 * 0x0046E230: copy of s up to its first '\n' (not included), with its
 * length in *out_len; NULL without a newline or when the line is longer
 * than max (max > 0).
 */
char *Font_DupLine(int max, const char *s, int *out_len)
{
    const char *nl = strchr(s, '\n');
    int n;
    char *line;

    if (nl == NULL) {
        return NULL;
    }
    n = (int)(nl - s);
    if (max > 0 && n > max) {
        return NULL;
    }
    line = malloc(n + 1);
    if (line == NULL) {
        return NULL;
    }
    strncpy(line, s, n);
    line[n] = '\0';
    if (out_len != NULL) {
        *out_len = n;
    }
    return line;
}
