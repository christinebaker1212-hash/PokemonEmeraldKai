/*
 * Palettes and PalFX (original module 0x4618C0-0x4627D3): palette objects,
 * palette remapping tables, the global and per-object PalFX state and the
 * conversion of RGBA colours to the RGB565 screen format.
 *
 * g_palFx is the global PalFX (pauses, AllPalFX...); g_palFxCur points to the
 * PalFX of the object being drawn, if any. Their combination is kept in
 * g_palFxApplied, which only uses the first 0x24 bytes of a PalFx: its serial
 * would overlap g_palFx.color.
 */
#include "mugen.h"

/* Packs clamped 8-bit components as RGB565, exactly as the original. */
static unsigned short pal_pack565(int r, int g, int b)
{
    return (unsigned short)((((r & 0xFFF8) << 5 | g) & 0xFFFC) << 3 | b >> 3);
}

static int pal_clamp(int v)
{
    if (v < 0) {
        return 0;
    }
    if (v > 0xFF) {
        return 0xFF;
    }
    return v;
}

/* 0x004618C0: fills pal with count colours; returns 0 on failure. */
int Pal_Init(Palette *pal, int count, void *colors, int flags)
{
    unsigned char padded[0x400];
    int gl = 0;
    int take;
    int ok;
    void *tex;

    if (g_renderMode == RENDER_MODE_GL || g_renderMode == RENDER_MODE_GL_ALT) {
        gl = 1;
    }
    take = (unsigned int)flags >> 1 & 1;
    memset(pal, 0, sizeof(Palette));
    pal->count = count;
    if (colors == NULL || count == 0) {
        return 1;
    }
    if (!(flags & PAL_NO_TEXTURE) && gl) {
        pal->is_texture = 1;
        tex = GlTex_Create();
        pal->data = tex;
        if (tex == NULL) {
            return 0;
        }
        pal->owns = 1;
        if (count == 0x100) {
            ok = GlTex_Upload1D(tex, 0x100, colors);
        } else if (pal->count < 0x100) {
            memcpy(padded, colors, count * 4);
            memset(padded + count * 4, 0, (0x100 - count) * 4);
            ok = GlTex_Upload1D(tex, 0x100, padded);
        } else {
            /* Quirk: more than 256 colours counts as success without any upload. */
            ok = 1;
        }
        if (!ok) {
            GlTex_Free(pal->data);
            pal->data = NULL;
            return 0;
        }
        if (take) {
            free(colors);
        }
    } else {
        pal->is_texture = 0;
        if (flags & PAL_COPY) {
            void *copy = calloc(1, count * 4);
            pal->data = copy;
            if (copy == NULL) {
                return 0;
            }
            memcpy(copy, colors, count * 4);
            pal->owns = 1;
        } else {
            pal->data = colors;
            pal->owns = take;
        }
    }
    if (!gl && !pal->is_texture) {
        Pal_ToRGB565(pal->data, pal->count - 1, pal->rgb565);
    }
    return 1;
}

/* 0x00461AB0 */
void Pal_Free(Palette *pal)
{
    if (pal->owns) {
        if (pal->is_texture == 1 && pal->data != NULL) {
            /* A texture that is still in use is not released (nor freed). */
            if (GlTex_Delete(pal->data)) {
                free(pal->data);
            }
        } else if (pal->data != NULL) {
            free(pal->data);
        }
    }
    pal->owns = 0;
    pal->data = NULL;
}

/*
 * 0x00461B00: copies src (without its serial) into fx, or resets fx when src
 * is NULL, and gives fx a new serial. Returns fx.
 *
 * As in the original, fx->color is cleared before copying, so fx == src
 * loses its color.
 */
PalFx *PalFx_Init(PalFx *fx, const PalFx *src)
{
    fx->color = 0;
    if (src != NULL) {
        int *d = (int *)fx;
        const int *s = (const int *)src;
        int i;
        for (i = 0; i < 9; i++) {
            d[i] = s[i];
        }
        fx->serial = g_palFxSerial++;
        return fx;
    }
    fx->add[0] = 0;
    fx->add[1] = 0;
    fx->add[2] = 0;
    fx->unused_1c = 0;
    fx->invertall = 0;
    fx->mul[2] = 0x100;
    fx->mul[1] = 0x100;
    fx->mul[0] = 0x100;
    fx->color = 0x100;
    fx->serial = g_palFxSerial++;
    return fx;
}

/* 0x00461B60: 0 when g_palFxCur points to the module's static PalFx. */
int PalFx_CurIsExternal(void)
{
    if (g_palFxCur == &g_palFxStatic && g_palFxCur != NULL) {
        return 0;
    }
    return 1;
}

/* Copies the PalFX parameters (everything but the serial). */
static void PalFx_CopyValues(PalFx *dst, const PalFx *src)
{
    int *d = (int *)dst;
    const int *s = (const int *)src;
    int i;

    for (i = 0; i < 9; i++) {
        d[i] = s[i];
    }
}

/* 0x00461B80: g_palFx combined with *g_palFxCur, in g_palFxApplied. */
PalFx *PalFx_GetApplied(void)
{
    if (g_palFxCur != NULL) {
        PalFx_Combine(&g_palFxApplied, &g_palFx, g_palFxCur);
        return &g_palFxApplied;
    }
    PalFx_CopyValues(&g_palFxApplied, &g_palFx);
    return &g_palFxApplied;
}

/* 0x00461BC0: refreshes the RGB565 colours of a software palette with the applied PalFX. */
void Pal_Convert(Palette *pal)
{
    if (pal->is_texture != 0) {
        return;
    }
    PalFx_GetApplied();
    PalFx_ToRGB565(&g_palFxApplied, pal->count - 1, pal->is_texture == 0 ? pal->data : NULL, pal->rgb565);
    g_palConvertCount++;
}

/*
 * 0x00461C30: drops identity and disabled entries.
 *
 * As in the original, the index still advances after a removal, so the entry
 * moved into the removed slot is not checked in this pass.
 */
void PalRemap_Compact(PalRemap *r)
{
    int i;

    for (i = 0; i < r->count; i++) {
        PalRemapEntry *e = &r->entries[i];
        if ((e->src_group == e->dst_group && e->src_index == e->dst_index) || e->dst_group == -1) {
            int j;
            for (j = i; j < r->count - 1; j++) {
                r->entries[j] = r->entries[j + 1];
            }
            r->count--;
            if (r->count < PAL_REMAP_MAX) {
                PalRemapEntry *last = &r->entries[r->count];
                last->src_group = 0;
                last->src_index = 0;
                last->dst_group = 0;
                last->dst_index = 0;
            }
        }
    }
}

/* 0x00461CB0: adds or replaces the mapping for e's source; 0 when the table is full. */
int PalRemap_Set(PalRemap *r, const PalRemapEntry *e)
{
    int count = r->count;
    int i;

    for (i = 0; i < count; i++) {
        PalRemapEntry *cur = &r->entries[i];
        if (cur->src_group == e->src_group && cur->src_index == e->src_index) {
            cur->src_group = e->src_group;
            cur->src_index = e->src_index;
            cur->dst_group = e->dst_group;
            cur->dst_index = e->dst_index;
            PalRemap_Compact(r);
            return 1;
        }
    }
    if (count < PAL_REMAP_MAX) {
        PalRemapEntry *cur = &r->entries[count];
        cur->src_group = e->src_group;
        cur->src_index = e->src_index;
        cur->dst_group = e->dst_group;
        cur->dst_index = e->dst_index;
        r->count++;
        PalRemap_Compact(r);
        return 1;
    }
    return 0;
}

/* 0x00461D40: 0 when g_palRemapCur points to the module's static table. */
int PalRemap_CurIsExternal(void)
{
    if (g_palRemapCur == &g_palRemapStatic && g_palRemapCur != NULL) {
        return 0;
    }
    return 1;
}

/* 0x00461D60: palette to use for group,index (unchanged when nothing matches). */
void PalRemap_Lookup(const PalRemap *r, int group, int index, int *out_group, int *out_index)
{
    int i;

    for (i = 0; i < r->count; i++) {
        const PalRemapEntry *e = &r->entries[i];
        if ((e->src_group == -1 || (e->src_group == group && e->src_index == index)) && e->dst_group != -1) {
            *out_index = e->dst_index;
            *out_group = e->dst_group;
            return;
        }
    }
    *out_index = index;
    *out_group = group;
}

/*
 * 0x00461DB0: out = a followed by b (unused_1c and serial are left alone).
 * Returns out->invertall.
 */
int PalFx_Combine(PalFx *out, const PalFx *a, const PalFx *b)
{
    out->color = a->color * b->color >> 8;
    out->add[0] = a->add[0] + b->add[0];
    out->add[1] = a->add[1] + b->add[1];
    out->add[2] = a->add[2] + b->add[2];
    out->mul[0] = a->mul[0] * b->mul[0] >> 8;
    out->mul[1] = a->mul[1] * b->mul[1] >> 8;
    out->mul[2] = a->mul[2] * b->mul[2] >> 8;
    if (a->invertall == 0 && b->invertall == 0) {
        out->invertall = 0;
        return 0;
    }
    out->invertall = 1;
    return 1;
}

/* 0x00461E20: scales fx's mul by r,g,b (256 = 1.0). Returns fx. */
PalFx *PalFx_ScaleMul(PalFx *fx, int r, int g, int b)
{
    if (r != 0x100 || g != 0x100 || b != 0x100) {
        int mul_r = fx->mul[0] * r;
        fx->mul[1] = fx->mul[1] * g >> 8;
        fx->mul[2] = fx->mul[2] * b >> 8;
        fx->serial = g_palFxSerial;
        fx->mul[0] = mul_r >> 8;
        g_palFxSerial++;
    }
    return fx;
}

/* Resets the global PalFX (body of PalFx_InitGlobal and PalFx_ResetGlobal). */
static void PalFx_ClearGlobal(void)
{
    g_palFx.color = 0;
    g_palFx.mul[0] = 0;
    g_palFx.mul[1] = 0;
    g_palFx.mul[2] = 0;
    g_palFx.add[0] = 0;
    g_palFx.add[1] = 0;
    g_palFx.add[2] = 0;
    g_palFx.unused_1c = 0;
    g_palFx.invertall = 0;
    g_palFx.mul[2] = 0x100;
    g_palFx.mul[1] = 0x100;
    g_palFx.mul[0] = 0x100;
    g_palFx.color = 0x100;
    g_palFx.serial = g_palFxSerial++;
}

/* 0x00461E70 */
int PalFx_InitGlobal(void)
{
    PalFx_ClearGlobal();
    return 1;
}

/* 0x00461EE0 */
void PalFx_ResetGlobal(void)
{
    PalFx_ClearGlobal();
}

/* 0x00461F40: scales the global PalFX's mul by r,g,b (256 = 1.0). */
void PalFx_ScaleGlobalMul(int r, int g, int b)
{
    if (r != 0x100 || g != 0x100 || b != 0x100) {
        int mul_r = g_palFx.mul[0] * r;
        int mul_g = g_palFx.mul[1] * g;
        int mul_b = g_palFx.mul[2] * b;
        g_palFx.mul[1] = mul_g >> 8;
        g_palFx.serial = g_palFxSerial;
        g_palFx.mul[0] = mul_r >> 8;
        g_palFx.mul[2] = mul_b >> 8;
        g_palFxSerial++;
    }
}

/* 0x00461FA0: applies the global PalFX to an RGB565 colour. */
unsigned int PalFx_Apply565(unsigned int color)
{
    unsigned char rgba[4];
    unsigned short out;

    rgba[0] = (unsigned char)(color >> 8 & 0xF8);
    rgba[1] = (unsigned char)(color >> 3 & 0xFC);
    rgba[2] = (unsigned char)(color << 3);
    rgba[3] = 0;
    PalFx_ToRGB565(&g_palFx, 0, rgba, &out);
    return out;
}

/* 0x00461FF0: dst = src * mul (one RGBA colour, components capped at 255). */
void PalFx_ColorMul(const unsigned char *src, unsigned char *dst, int mul_r, int mul_g, int mul_b)
{
    int v;

    if (mul_r == 0x100 && mul_g == 0x100 && mul_b == 0x100) {
        if (src != dst) {
            *(unsigned int *)dst = *(const unsigned int *)src;
        }
        return;
    }
    /* Only the upper bound is checked: a negative factor wraps the byte. */
    v = src[0] * mul_r >> 8;
    if (v > 0xFF) {
        v = 0xFF;
    }
    dst[0] = (unsigned char)v;
    v = src[1] * mul_g >> 8;
    if (v > 0xFF) {
        v = 0xFF;
    }
    dst[1] = (unsigned char)v;
    v = src[2] * mul_b >> 8;
    if (v > 0xFF) {
        v = 0xFF;
    }
    dst[2] = (unsigned char)v;
    dst[3] = src[3];
}

/* 0x00462080: dst = src + add (one RGBA colour, clamped). */
void PalFx_ColorAdd(const unsigned char *src, unsigned char *dst, int add_r, int add_g, int add_b)
{
    if (add_r == 0 && add_g == 0 && add_b == 0) {
        if (src != dst) {
            *(unsigned int *)dst = *(const unsigned int *)src;
        }
        return;
    }
    dst[0] = (unsigned char)pal_clamp(src[0] + add_r);
    dst[1] = (unsigned char)pal_clamp(src[1] + add_g);
    dst[2] = (unsigned char)pal_clamp(src[2] + add_b);
    dst[3] = src[3];
}

/* 0x00462110: blends one RGBA colour with its grey level (amount 0 = grey, 256 = unchanged). */
void PalFx_ColorDesaturate(const unsigned char *src, unsigned char *dst, int amount)
{
    if (amount == 0) {
        unsigned char a = src[3];
        unsigned char grey = (unsigned char)((src[0] * 5 + src[1] * 7 + src[2] * 4) >> 4);
        dst[2] = grey;
        dst[1] = grey;
        dst[0] = grey;
        dst[3] = a;
        return;
    }
    if ((unsigned int)(amount - 1) < 0xFF) {
        int r = src[0];
        int g = src[1];
        int b = src[2];
        int grey = ((r * 5 + g * 7 + b * 4) >> 4) * (0x100 - amount);
        unsigned char a;
        dst[1] = (unsigned char)((g * amount + grey) >> 8);
        a = src[3];
        dst[0] = (unsigned char)((r * amount + grey) >> 8);
        dst[2] = (unsigned char)((b * amount + grey) >> 8);
        dst[3] = a;
        return;
    }
    if (src != dst) {
        *(unsigned int *)dst = *(const unsigned int *)src;
    }
}

/* 0x004621B0: inverts the colour components of one RGBA colour. */
void PalFx_ColorInvert(const unsigned char *src, unsigned char *dst)
{
    dst[0] = (unsigned char)~src[0];
    dst[1] = (unsigned char)~src[1];
    dst[2] = (unsigned char)~src[2];
    dst[3] = src[3];
}

/* 0x004621F0: applies the global PalFX to one RGBA colour. Not called by the game. */
void PalFx_ApplyColor(const unsigned char *src, unsigned char *dst)
{
    PalFx_ColorDesaturate(src, dst, g_palFx.color);
    if (g_palFx.invertall != 0) {
        PalFx_ColorInvert(dst, dst);
    }
    PalFx_ColorAdd(dst, dst, g_palFx.add[0], g_palFx.add[1], g_palFx.add[2]);
    PalFx_ColorMul(dst, dst, g_palFx.mul[0], g_palFx.mul[1], g_palFx.mul[2]);
}

/* End of file for Pal_ReadAct: unknown stream types count as ended. */
static int pal_stream_eof(Stream *s)
{
    if (s->type == STREAM_FILE) {
        return feof(s->fp);
    }
    if ((unsigned int)(s->type - STREAM_MEMORY) <= 1) {
        return s->eof;
    }
    return 1;
}

/*
 * 0x00462260: reads a 768-byte .act palette into 256 RGBA colours. MUGEN
 * stores them in reverse order; colour 0 becomes transparent. Fails when the
 * stream ends before the last colour.
 */
int Pal_ReadAct(Stream *s, unsigned char *out)
{
    int i;

    for (i = 0; i < 0x100; i++) {
        unsigned char *c = out + (0xFF - i) * 4;
        c[0] = (unsigned char)Stream_GetByte(s);
        c[1] = (unsigned char)Stream_GetByte(s);
        c[2] = (unsigned char)Stream_GetByte(s);
        c[3] = 0xFF;
        if (pal_stream_eof(s) && i != 0xFF) {
            return 0;
        }
    }
    out[3] = 0;
    return 1;
}

/* 0x00462380: converts colours 0..last to RGB565 without PalFX. */
void Pal_ToRGB565(const unsigned char *rgba, int last, unsigned short *out)
{
    unsigned int n;

    for (n = (unsigned int)last + 1; n != 0; n--) {
        *out++ = pal_pack565(rgba[0], rgba[1], rgba[2]);
        rgba += 4;
    }
}

/*
 * 0x004623E0: converts colours 0..last to RGB565 through fx.
 *
 * Quirks kept from the original: a negative color takes the partial
 * desaturation path, and mul is skipped only when color is exactly 256.
 */
void PalFx_ToRGB565(const PalFx *fx, int last, const unsigned char *rgba, unsigned short *out)
{
    int add_r = fx->add[0];
    int add_g = fx->add[1];
    int add_b = fx->add[2];
    int mul_r = fx->mul[0];
    int mul_g = fx->mul[1];
    int mul_b = fx->mul[2];
    int color = fx->color;
    int inv = fx->invertall != 0 ? 0xFF : 0;
    int bytes = (int)((unsigned int)last * 4 + 4);
    int i;

    if (mul_r == 0x100 && mul_g == 0x100 && mul_b == 0x100 && color == 0x100) {
        for (i = 0; i < bytes; i += 4) {
            int r = pal_clamp((rgba[i] ^ inv) + add_r);
            int g = pal_clamp((rgba[i + 1] ^ inv) + add_g);
            int b = pal_clamp((rgba[i + 2] ^ inv) + add_b);
            *out++ = pal_pack565(r, g, b);
        }
    } else if (color == 0) {
        if (bytes > 0) {
            unsigned int n = ((unsigned int)(bytes - 1) >> 2) + 1;
            const unsigned char *c = rgba;
            for (; n != 0; n--, c += 4) {
                int grey = ((c[0] * 5 + c[1] * 7 + c[2] * 4) >> 4) ^ inv;
                int r = pal_clamp((grey + add_r) * mul_r >> 8);
                int g = pal_clamp((grey + add_g) * mul_g >> 8);
                int b = pal_clamp((grey + add_b) * mul_b >> 8);
                *out++ = pal_pack565(r, g, b);
            }
        }
    } else if (color < 0x100) {
        if (bytes > 0) {
            unsigned int n = ((unsigned int)(bytes - 1) >> 2) + 1;
            const unsigned char *c = rgba;
            for (; n != 0; n--, c += 4) {
                int grey = ((c[0] * 5 + c[1] * 7 + c[2] * 4) >> 4) * (0x100 - color);
                int r = pal_clamp(((((c[0] * color + grey) >> 8) ^ inv) + add_r) * mul_r >> 8);
                int g = pal_clamp(((((c[1] * color + grey) >> 8) ^ inv) + add_g) * mul_g >> 8);
                int b = pal_clamp(((((c[2] * color + grey) >> 8) ^ inv) + add_b) * mul_b >> 8);
                *out++ = pal_pack565(r, g, b);
            }
        }
    } else {
        for (i = 0; i < bytes; i += 4) {
            int r = pal_clamp(((rgba[i] ^ inv) + add_r) * mul_r >> 8);
            int g = pal_clamp(((rgba[i + 1] ^ inv) + add_g) * mul_g >> 8);
            int b = pal_clamp(((rgba[i + 2] ^ inv) + add_b) * mul_b >> 8);
            *out++ = pal_pack565(r, g, b);
        }
    }
}
