/*
 * PNG reading and writing (original module 0x477140-0x477ABF): screenshots
 * of SDL surfaces, and the SFF v2 sprite formats 10-12 (PNG data after the
 * 4-byte decoded size), encoded into a GrowBuf and decoded from memory.
 *
 * libpng 1.2.38 is linked statically into mugen.exe (third_party/libpng-1.2.38).
 */
#include "mugen.h"

/* 0x00477140: libpng warning callback. */
void Png_OnWarning(png_structp png, png_const_charp msg)
{
    MatchLog_Printf(5, "libpng: warning: %s\n", msg);
}

/* 0x00477160: libpng error callback; libpng then longjmps to the caller's setjmp. */
void Png_OnError(png_structp png, png_const_charp msg)
{
    MatchLog_Printf(3, "libpng: error: %s\n", msg);
}

/*
 * 0x00477180: writes surface as an 8-bit PNG (palette, RGB or RGBA) to fp and
 * closes it. Returns 0, or -1 on errors (fp stays open if libpng could not
 * be set up).
 */
int Png_WriteSurface(SDL_Surface *surface, FILE *fp)
{
    png_color palette[256];
    png_structp png;
    png_infop info;
    png_bytepp rows;
    int color_type;
    int i;

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, Png_OnError, Png_OnWarning);
    if (png == NULL) {
        MatchLog_Printf(3, "libpng: png_create_write_struct error\n");
        return -1;
    }
    info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_write_struct(&png, NULL);
        MatchLog_Printf(3, "libpng: png_create_info_struct error\n");
        return -1;
    }
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return -1;
    }
    png_init_io(png, fp);
    if (surface->format->palette != NULL) {
        color_type = PNG_COLOR_TYPE_PALETTE;
    } else if (surface->format->Amask != 0) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    } else {
        color_type = PNG_COLOR_TYPE_RGB;
    }
    png_set_IHDR(png, info, surface->w, surface->h, 8, color_type, 0, 0, 0);
    if (surface->format->palette != NULL) {
        SDL_Palette *pal = surface->format->palette;
        for (i = 0; i < pal->ncolors; i++) {
            palette[i].red = pal->colors[i].r;
            palette[i].green = pal->colors[i].g;
            palette[i].blue = pal->colors[i].b;
        }
        png_set_PLTE(png, info, palette, pal->ncolors);
    }
    png_write_info(png, info);
    png_set_packing(png);
    rows = malloc(surface->h * sizeof(png_bytep));
    for (i = 0; i < surface->h; i++) {
        rows[i] = (png_bytep)surface->pixels + surface->pitch * i;
    }
    png_write_image(png, rows);
    png_write_end(png, info);
    free(rows);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return 0;
}

/* 0x00477380: libpng read callback, reading from g_pngStream. */
void Png_ReadData(png_structp png, png_bytep data, png_size_t length)
{
    Stream_Read(data, length, 1, g_pngStream);
}

/* Moves the write position of g_pngBuffer by n, unless that leaves the buffer. */
static void Png_Advance(int n)
{
    int pos = g_pngBufferPos + n;

    if (pos >= 0 && pos < g_pngBuffer->size) {
        g_pngBufferPos = pos;
    }
}

/* 0x004773A0: libpng write callback, appending to g_pngBuffer (dropped if it cannot grow). */
void Png_WriteData(png_structp png, png_bytep data, png_size_t length)
{
    int need = g_pngBufferPos + (int)length + 1;

    if (need > g_pngBuffer->size && !GrowBuf_Resize(g_pngBuffer, need)) {
        return;
    }
    memcpy(g_pngBuffer->data + g_pngBufferPos, data, length);
    Png_Advance((int)length);
}

/* libpng flush callback (the original points at a shared empty function, 0x478040). */
static void PNGAPI Png_Flush(png_structp png)
{
}

/*
 * 0x00477400: encodes width x height pixels of bpp bits (<= 8: palette,
 * 32: RGBA, else RGB; 8 bits per channel) into g_pngBuffer at
 * g_pngBufferPos. Returns where the PNG data starts and its size, or NULL
 * on errors, after freeing g_pngOwnedBuffer.
 *
 * The palette is always 256 black entries: the palette source is a local
 * that is never set (the copy code for it is kept).
 */
unsigned char *Png_EncodeToBuffer(const void *pixels, int width, int height, int bpp, int *size_out)
{
    png_color palette[256];
    int row_bytes = (bpp + 7) / 8 * width;
    unsigned char *start = g_pngBuffer->data + g_pngBufferPos;
    int start_pos = g_pngBufferPos;
    int ncolors = 0;
    const unsigned char *colors = NULL;
    const unsigned char *row;
    png_structp png;
    png_infop info;
    png_bytepp rows;
    int color_type;
    int i;

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, Png_OnError, Png_OnWarning);
    if (png == NULL) {
        MatchLog_Printf(3, "libpng: png_create_write_struct error\n");
        goto fail;
    }
    info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_write_struct(&png, NULL);
        MatchLog_Printf(3, "libpng: png_create_info_struct error\n");
        goto fail;
    }
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        goto fail;
    }
    png_set_write_fn(png, png_get_io_ptr(png), Png_WriteData, Png_Flush);
    if (bpp <= 8) {
        color_type = PNG_COLOR_TYPE_PALETTE;
    } else if (bpp == 32) {
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    } else {
        color_type = PNG_COLOR_TYPE_RGB;
    }
    png_set_IHDR(png, info, width, height, 8, color_type, 0, 0, 0);
    if (color_type & PNG_COLOR_MASK_PALETTE) {
        if (ncolors > 256) {
            ncolors = 256;
        }
        if (ncolors > 0 && colors != NULL) {
            for (i = 0; i < ncolors; i++) {
                palette[i].red = colors[4 * i];
                palette[i].green = colors[4 * i + 1];
                palette[i].blue = colors[4 * i + 2];
            }
        } else {
            ncolors = 256;
            for (i = 0; i < 256; i++) {
                palette[i].red = 0;
                palette[i].green = 0;
                palette[i].blue = 0;
            }
        }
        png_set_PLTE(png, info, palette, ncolors);
    }
    png_write_info(png, info);
    png_set_packing(png);
    rows = malloc(height * sizeof(png_bytep));
    row = pixels;
    for (i = 0; i < height; i++) {
        rows[i] = (png_bytep)row;
        row += row_bytes;
    }
    png_write_image(png, rows);
    png_write_end(png, info);
    free(rows);
    png_destroy_write_struct(&png, &info);
    if (size_out != NULL) {
        *size_out = g_pngBufferPos - start_pos;
    }
    return start;

fail:
    if (g_pngOwnedBuffer != NULL) {
        GrowBuf *buf = g_pngOwnedBuffer;
        if (buf->data != NULL) {
            free(buf->data);
            buf->data = NULL;
        }
        free(buf);
    }
    g_pngOwnedBuffer = NULL;
    return NULL;
}

/*
 * 0x00477680: decodes the PNG at the stream position into dest (rows of
 * width * ceil(bits per pixel / 8) bytes). Only palette, RGB and RGBA images
 * of up to 8 bits per channel are decoded; others are skipped (still 0).
 * Returns -1 if the stream holds no PNG or libpng could not be set up.
 *
 * No setjmp here: a libpng error longjmps into png_create_read_struct's
 * finished frame, as in the original.
 */
int Png_ReadStream(Stream *s, void *dest)
{
    unsigned char sig[8];
    png_structp png;
    png_infop info;
    png_infop end_info;
    png_uint_32 width, height;
    int bit_depth, color_type;
    png_bytepp rows;
    int bpp;
    int row_bytes;
    int i;

    if (Stream_Read(sig, 1, 8, s) < 8) {
        return -1;
    }
    if (png_sig_cmp(sig, 0, 8) != 0) {
        return -1;
    }
    /* The error callback is passed as the error pointer too. */
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)Png_OnError, Png_OnError, Png_OnWarning);
    if (png == NULL) {
        return -1;
    }
    info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        return -1;
    }
    end_info = png_create_info_struct(png);
    if (end_info == NULL) {
        png_destroy_read_struct(&png, &info, NULL);
        return -1;
    }
    g_pngStream = s;
    png_set_read_fn(png, png_get_io_ptr(png), Png_ReadData);
    png_set_sig_bytes(png, 8);
    png_read_info(png, info);
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if (bit_depth <= 8) {
        switch (color_type) {
        case PNG_COLOR_TYPE_RGB:
            bpp = bit_depth * 3;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            bpp = bit_depth;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            bpp = bit_depth * 4;
            break;
        default:
            goto done;
        }
        rows = malloc(height * sizeof(png_bytep));
        if (rows != NULL) {
            row_bytes = (bpp + 7) / 8;
            for (i = 0; i < (int)height; i++) {
                rows[i] = (png_bytep)dest + i * row_bytes * (int)width;
            }
            if (bit_depth < 8) {
                png_set_packing(png);
            }
            png_read_update_info(png, info);
            png_read_image(png, rows);
            free(rows);
        }
    }
done:
    g_pngStream = NULL;
    png_destroy_read_struct(&png, &info, &end_info);
    return 0;
}

/*
 * 0x004778B0: encodes an image as SFF v2 PNG sprite data: the decoded size
 * (bytes per pixel * width * height) followed by the PNG. Returns a malloc'd
 * buffer and its size, or NULL.
 */
unsigned char *Png_Encode(const void *pixels, int width, int height, int bpp, int *size_out)
{
    int raw_size = (bpp + 7) / 8 * width * height;
    unsigned char *data;
    GrowBuf *buf;
    int png_size;
    int ok;

    buf = Mem_Calloc(1, sizeof(GrowBuf));
    if (buf != NULL) {
        buf->data = Mem_Calloc(1, 0x8000);
        if (buf->data == NULL) {
            free(buf);
            buf = NULL;
        } else {
            buf->size = 0x8000;
            buf->grow_step = 0x400;
        }
    }
    g_pngOwnedBuffer = buf;
    if (buf == NULL) {
        return NULL;
    }
    buf->grow_step = 0x2000;
    g_pngBuffer = buf;
    g_pngBufferPos = 0;
    if (buf->size >= 5 || GrowBuf_Resize(buf, 5)) {
        *(int *)(g_pngBuffer->data + g_pngBufferPos) = raw_size;
        Png_Advance(PNG_SPRITE_PREFIX);
    }
    ok = Png_EncodeToBuffer(pixels, width, height, bpp, &png_size) != NULL;
    if (ok) {
        *size_out = png_size + PNG_SPRITE_PREFIX;
    }
    data = NULL;
    buf = g_pngOwnedBuffer;
    if (buf != NULL) {
        data = buf->data;
        buf->data = NULL;
        free(buf);
    }
    g_pngOwnedBuffer = NULL;
    g_pngBuffer = NULL;
    return ok ? data : NULL;
}

/* 0x00477A00: sprite format 10 encoder (palette). */
unsigned char *Png_Encode8(const void *pixels, int width, int height, int *size_out)
{
    return Png_Encode(pixels, width, height, 8, size_out);
}

/* 0x00477A20: sprite format 11 encoder (RGB). */
unsigned char *Png_Encode24(const void *pixels, int width, int height, int *size_out)
{
    return Png_Encode(pixels, width, height, 24, size_out);
}

/* 0x00477A40: sprite format 12 encoder (RGBA). */
unsigned char *Png_Encode32(const void *pixels, int width, int height, int *size_out)
{
    return Png_Encode(pixels, width, height, 32, size_out);
}

/*
 * 0x00477A60: sprite formats 10-12 decoder: data is the sprite data (the
 * decoded size, then the PNG), size the PNG size. Returns the Stream_Close
 * result (the decoding result is dropped), 0 if out of memory.
 */
int Png_Decode(const void *data, int size, void *dest)
{
    Stream *s = Stream_OpenMemory((unsigned char *)data + PNG_SPRITE_PREFIX, size);

    if (s == NULL) {
        return 0;
    }
    Png_ReadStream(s, dest);
    return Stream_Close(s);
}
