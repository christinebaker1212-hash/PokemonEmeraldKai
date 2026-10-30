/*
 * Sprite files: SFF v1.01 (PCX images), SFF v2.x and single PCX images
 * (original module 0x467510-0x469A45).
 *
 * File offsets in SFF headers count from the start of the file; the loaders
 * convert them relative to the stream position after the 0x200-byte header,
 * so a sprite file can sit inside a larger stream. The stream position is
 * restored before returning.
 *
 * Sprites, palettes and their containers are created by the sprite module
 * (Sprite_Init, Pal_Init, SpriteSet_*); compressed SFF v2 formats are
 * decoded there too.
 */
#include "mugen.h"

#define SFF_LOG_INFO    7
#define SFF_LOG_ERROR   3

/* Offset relative to the start of the stream for a file offset of an SFF. */
static long Sff_StreamPos(unsigned int file_offset, long start)
{
    return (long)(file_offset + start - SFF_HEADER_SIZE);
}

/* ---- headers ---- */

/* 0x00469600: rest of a v1.01 header, after the version. */
int Sff1_ReadHeader(Sff1Header *hdr, Stream *s)
{
    if (Stream_ReadU32(s, (unsigned int *)&hdr->group_count) == -1 ||
        Stream_ReadU32(s, (unsigned int *)&hdr->image_count) == -1 ||
        Stream_ReadU32(s, &hdr->first_offset) == -1 ||
        Stream_ReadU32(s, &hdr->subheader_size) == -1 ||
        Stream_ReadU8(s, &hdr->palette_type) == -1 ||
        Stream_ReadU8(s, &hdr->reserved[0]) == -1 ||
        Stream_ReadU8(s, &hdr->reserved[1]) == -1 ||
        Stream_ReadU8(s, &hdr->reserved[2]) == -1) {
        return 0;
    }
    return Stream_Read(hdr->comments, sizeof(hdr->comments), 1, s) >= 1;
}

/* 0x004696C0: rest of a v2.x header, after the version. */
int Sff2_ReadHeader(Sff2Header *hdr, Stream *s)
{
    if (Stream_ReadU32(s, &hdr->reserved1) == -1 ||
        Stream_ReadU32(s, &hdr->reserved2) == -1 ||
        Stream_ReadU32(s, &hdr->compat_version) == -1 ||
        Stream_ReadU32(s, &hdr->reserved3) == -1 ||
        Stream_ReadU32(s, &hdr->reserved4) == -1 ||
        Stream_ReadU32(s, &hdr->sprite_offset) == -1 ||
        Stream_ReadU32(s, &hdr->sprite_count) == -1 ||
        Stream_ReadU32(s, &hdr->palette_offset) == -1 ||
        Stream_ReadU32(s, &hdr->palette_count) == -1 ||
        Stream_ReadU32(s, &hdr->ldata_offset) == -1 ||
        Stream_ReadU32(s, &hdr->ldata_size) == -1 ||
        Stream_ReadU32(s, &hdr->tdata_offset) == -1 ||
        Stream_ReadU32(s, &hdr->tdata_size) == -1 ||
        Stream_ReadU32(s, &hdr->reserved5) == -1 ||
        Stream_ReadU32(s, &hdr->reserved6) == -1) {
        return 0;
    }
    return Stream_Read(hdr->unused, sizeof(hdr->unused), 1, s) >= 1;
}

/*
 * 0x00469550: reads and checks the file header. The version is stored in
 * *version (if not NULL) as soon as it is read, even when it is not
 * supported (1.01 to 2.01).
 */
int Sff_ReadHeader(Stream *s, SffHeader *hdr, unsigned int *version)
{
    memset(hdr, 0, sizeof(*hdr));
    if (Stream_Read(hdr->v1.signature, sizeof(hdr->v1.signature), 1, s) < 1) {
        return 0;
    }
    /* The signature may lack its terminator: strcmp then runs into the zeroed version. */
    if (strcmp(hdr->v1.signature, "ElecbyteSpr") != 0) {
        return 0;
    }
    if (Stream_ReadU32(s, &hdr->v1.version) == -1) {
        return 0;
    }
    if (version != NULL) {
        *version = hdr->v1.version;
    }
    if (hdr->v1.version < SFF_VERSION_101 || hdr->v1.version > SFF_VERSION_201) {
        return 0;
    }
    if (hdr->v1.version == SFF_VERSION_101) {
        return Sff1_ReadHeader(&hdr->v1, s);
    }
    return Sff2_ReadHeader(&hdr->v2, s);
}

/* 0x00469810 */
int Sff1_ReadSubHeader(Sff1SubHeader *sub, Stream *s)
{
    if (Stream_ReadU32(s, &sub->next_offset) == -1 ||
        Stream_ReadU32(s, (unsigned int *)&sub->size) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&sub->x) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&sub->y) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&sub->group) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&sub->number) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&sub->link) == -1 ||
        Stream_ReadU8(s, &sub->same_palette) == -1) {
        return 0;
    }
    return Stream_Read(sub->comment, sizeof(sub->comment), 1, s) >= 1;
}

/* 0x004698C0 */
int Sff2_ReadSpriteNode(Sff2SpriteNode *node, Stream *s)
{
    if (Stream_ReadU16(s, &node->group) == -1 ||
        Stream_ReadU16(s, &node->number) == -1 ||
        Stream_ReadU16(s, &node->w) == -1 ||
        Stream_ReadU16(s, &node->h) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&node->x) == -1 ||
        Stream_ReadU16(s, (unsigned short *)&node->y) == -1 ||
        Stream_ReadU16(s, &node->link) == -1 ||
        Stream_ReadU8(s, &node->format) == -1 ||
        Stream_ReadU8(s, &node->depth) == -1 ||
        Stream_ReadU32(s, &node->data_offset) == -1 ||
        Stream_ReadU32(s, &node->data_size) == -1 ||
        Stream_ReadU16(s, &node->palette) == -1) {
        return 0;
    }
    return Stream_ReadU16(s, &node->flags) != -1;
}

/* 0x004699D0 */
int Sff2_ReadPaletteNode(Sff2PaletteNode *node, Stream *s)
{
    if (Stream_ReadU16(s, &node->group) == -1 ||
        Stream_ReadU16(s, &node->number) == -1 ||
        Stream_ReadU16(s, &node->ncolors) == -1 ||
        Stream_ReadU16(s, &node->link) == -1 ||
        Stream_ReadU32(s, &node->data_offset) == -1) {
        return 0;
    }
    return Stream_ReadU32(s, &node->data_size) != -1;
}

/* ---- PCX ---- */

/* End-of-file test of the palette marker scan; other stream kinds count as ended. */
static int Pcx_StreamEnded(Stream *s)
{
    if (s->type == STREAM_FILE) {
        return feof(s->fp);
    }
    if ((unsigned int)(s->type - STREAM_MEMORY) < 2) {
        return s->eof;
    }
    return 1;
}

/*
 * 0x00468C60: decodes the PCX image at the stream position into a new buffer
 * of *size bytes: *w x *h pixels of 8 bits, or 24 bits stored B, G, R (the
 * planes are written from the last byte of each pixel down). With pal, the
 * 256-color palette at the end of 8-bit images (or the 16-color header
 * palette when it is missing) is set up in pal, color 0 transparent. Flags
 * keeps the palette out of textures. The stream position is restored on
 * success and for unsupported depths; NULL on errors.
 *
 * Quirks kept: header fields keep their previous (uninitialised) value when
 * a read fails, images with an empty size become 1x1 and are not decoded,
 * and the bits == 4 palette case is unreachable.
 */
void *Pcx_Decode(Palette *pal, int *w_out, Stream *s, int *h_out, int *bpp_out, size_t *size_out, int flags)
{
    unsigned char colors[256 * 4];
    unsigned short value;
    unsigned int dpi;
    unsigned int pal_flags = PAL_COPY;
    int empty = 0;
    int xmin, ymin, w, h, bits, bytes_pp, size, i;
    unsigned char *pixels;
    long start;

    memset(colors, 0, sizeof(colors));
    *bpp_out = 0;
    *h_out = 0;
    *w_out = 0;
    *size_out = 0;
    if (flags) {
        pal_flags = PAL_COPY | PAL_NO_TEXTURE;
    }
    start = Stream_Tell(s);

    /* Manufacturer, version, encoding; then bits per pixel (per plane). */
    Stream_GetByte(s);
    Stream_GetByte(s);
    Stream_GetByte(s);
    if (Stream_GetByte(s) != 8) {
        return NULL;
    }
    Stream_ReadU16(s, &value);
    xmin = (short)value;
    Stream_ReadU16(s, &value);
    ymin = (short)value;
    Stream_ReadU16(s, &value);
    w = (short)value - xmin + 1;
    Stream_ReadU16(s, &value);
    h = (short)value - ymin + 1;
    Stream_ReadU32(s, &dpi);
    for (i = 0; i < 16; i++) {
        colors[i * 4 + 0] = (unsigned char)Stream_GetByte(s);
        colors[i * 4 + 1] = (unsigned char)Stream_GetByte(s);
        colors[i * 4 + 2] = (unsigned char)Stream_GetByte(s);
        colors[i * 4 + 3] = 0xFF;
    }
    Stream_GetByte(s);
    bits = Stream_GetByte(s) * 8;
    if (bits != 8 && bits != 24) {
        Stream_Seek(s, start, SEEK_SET);
        return NULL;
    }
    bytes_pp = (bits - 1) / 8 + 1;
    Stream_ReadU16(s, &value);  /* bytes per line of one plane */
    size = bytes_pp * h * w;
    *size_out = size;
    if (size <= 0) {
        *size_out = bytes_pp;
        w = 1;
        h = 1;
        empty = 1;
    }
    for (i = 0; i < 60; i++) {
        Stream_GetByte(s);
    }
    pixels = calloc(1, *size_out);
    if (pixels == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }

    /* RLE: 0xC0 | count before a repeated byte. Bytes past the width are dropped. */
    if (!empty && h > 0) {
        int bpl = (short)value;
        int line_bytes = bpl * bits / 8;
        int stride = bytes_pp * w;
        unsigned char *row = pixels;
        int rows = h;

        do {
            int x = 0, px = 0, plane = 2;
            while (x < line_bytes) {
                int c = Stream_GetByte(s);
                int run = 1;
                if ((c & 0xC0) == 0xC0) {
                    run = c & 0x3F;
                    c = Stream_GetByte(s);
                }
                if (bits == 8) {
                    while (run != 0) {
                        run--;
                        if (x < w) {
                            row[x] = (unsigned char)c;
                        }
                        x++;
                    }
                } else {
                    while (run != 0) {
                        run--;
                        if (px < w) {
                            row[px * 3 + plane] = (unsigned char)c;
                        }
                        x++;
                        if (x == bpl) {
                            px = 0;
                            plane = 1;
                        } else if (x == bpl * 2) {
                            px = 0;
                            plane = 0;
                        } else {
                            px++;
                        }
                    }
                }
            }
            row += stride;
        } while (--rows != 0);
    }

    if (bits == 8) {
        if (pal != NULL) {
            /* The 256-color palette follows a 12 byte at the end of the image. */
            for (;;) {
                if (Pcx_StreamEnded(s)) {
                    goto init_palette;
                }
                if (Stream_GetByte(s) == 12) {
                    break;
                }
            }
            for (i = 0; i < 256; i++) {
                colors[i * 4 + 0] = (unsigned char)Stream_GetByte(s);
                colors[i * 4 + 1] = (unsigned char)Stream_GetByte(s);
                colors[i * 4 + 2] = (unsigned char)Stream_GetByte(s);
                colors[i * 4 + 3] = 0xFF;
            }
            colors[3] = 0;
        init_palette:
            if (!Pal_Init(pal, 256, colors, pal_flags)) {
                return NULL;
            }
        }
    } else if (bits == 4 && pal != NULL) {
        if (!Pal_Init(pal, 16, colors, pal_flags)) {
            return NULL;
        }
    }
    *bpp_out = bits;
    *w_out = w;
    *h_out = h;
    Stream_Seek(s, start, SEEK_SET);
    return pixels;
}

/*
 * 0x00469410: a sprite set holding one PCX image as sprite (0, 0), with
 * palette (0, 0) for 8-bit images. Flags keeps the image and the palette out
 * of textures.
 */
SpriteSet *Pcx_Load(Stream *s, int flags)
{
    Palette pal;
    Sprite spr;
    int w, h, bpp;
    size_t size;
    void *pixels;
    SpriteSet *set = calloc(1, sizeof(SpriteSet));

    if (set == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    /* The set leaks when these fail. */
    set->sprites = KeyedArray_Create(1, sizeof(Sprite));
    if (set->sprites == NULL) {
        return NULL;
    }
    set->palettes = KeyedArray_Create(1, sizeof(Palette));
    if (set->palettes == NULL) {
        return NULL;
    }
    pixels = Pcx_Decode(&pal, &w, s, &h, &bpp, &size, flags);
    if (pixels == NULL) {
        SpriteSet_Free(set);
        return NULL;
    }
    if (bpp <= 8) {
        SpriteSet_AddPalette(set, &pal, 0, 0);
    }
    if (!Sprite_Init(flags ? SPRITE_INIT_OWN | SPRITE_INIT_DECODE | SPRITE_INIT_NO_TEXTURE
                           : SPRITE_INIT_OWN | SPRITE_INIT_DECODE,
                     &spr, SPRITE_FORMAT_RAW, pixels, size, w, h, bpp, 0, 0)) {
        SpriteSet_Free(set);
        return NULL;
    }
    if (bpp <= 8) {
        SpriteSet_LinkPalette(set, &spr, 0, 0);
    }
    SpriteSet_AddSprite(set, &spr, 0, 0);
    return set;
}

/* ---- SFF v1.01 ---- */

/*
 * 0x004677D0: every sprite of a v1.01 file. Palettes are numbered (0, n) in
 * load order. Linked sprites copy the linked sprite by slot index without a
 * range check.
 */
SpriteSet *Sff1_Load(const Sff1Header *hdr, Stream *s)
{
    Sff1SubHeader sub;
    Palette pal;
    Sprite spr;
    int w, h, bpp;
    size_t size;
    int pal_count = 0;
    int last_pal = -1;
    long start = Stream_Tell(s);
    int image_count = hdr->image_count;
    unsigned int offset;
    int i;
    SpriteSet *set = calloc(1, sizeof(SpriteSet));

    if (set == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    /* The set leaks when these fail. */
    if (image_count > 0) {
        set->sprites = KeyedArray_Create(image_count, sizeof(Sprite));
        if (set->sprites == NULL) {
            return NULL;
        }
    }
    set->palettes = KeyedArray_Create(1, sizeof(Palette));
    if (set->palettes == NULL) {
        return NULL;
    }
    offset = hdr->first_offset;
    for (i = 0; i < hdr->image_count; i++) {
        Stream_Seek(s, Sff_StreamPos(offset, start), SEEK_SET);
        if (!Sff1_ReadSubHeader(&sub, s)) {
            goto fail;
        }
        if (sub.size > 0) {
            Palette *new_pal = NULL;
            void *pixels;
            if (last_pal == -1 || !sub.same_palette) {
                new_pal = &pal;
            }
            pixels = Pcx_Decode(new_pal, &w, s, &h, &bpp, &size, 0);
            if (pixels == NULL) {
                goto fail;
            }
            if (bpp > 8) {
                new_pal = NULL;
            } else if (new_pal != NULL && !SpriteSet_AddPalette(set, new_pal, 0, pal_count)) {
                goto fail;
            }
            if (!Sprite_Init(SPRITE_INIT_OWN | SPRITE_INIT_DECODE, &spr, SPRITE_FORMAT_RAW, pixels, size,
                             w, h, bpp, sub.x, sub.y)) {
                goto fail;
            }
            if (new_pal != NULL) {
                SpriteSet_LinkPalette(set, &spr, 0, pal_count);
                last_pal = pal_count++;
            } else if (sub.same_palette) {
                SpriteSet_LinkPalette(set, &spr, 0, last_pal);
            }
        } else {
            KeyedArray *sprites = set->sprites;
            spr = *(const Sprite *)(sprites->data + sub.link * sprites->elem_size);
            spr.owns_rows = 0;
            spr.owns_pixels = 0;
            spr.x = sub.x;
            spr.y = sub.y;
        }
        SpriteSet_AddSprite(set, &spr, sub.group, sub.number);
        offset = sub.next_offset;
    }
    Stream_Seek(s, start, SEEK_SET);
    return set;

fail:
    SpriteSet_Free(set);
    return NULL;
}

/*
 * 0x00467C70: only the sprites in select (and the palettes they need) of a
 * v1.01 file. Stops once as many sprites as select pairs were loaded; linked
 * sprites are skipped.
 */
SpriteSet *Sff1_LoadSelected(Stream *s, const Sff1Header *hdr, const int *select)
{
    Sff1SubHeader sub;
    Palette pal;
    Sprite spr;
    int w, h, bpp;
    size_t size;
    int pal_count = 0;
    int last_pal = -1;
    int loaded = 0;
    int wanted_count = 0;
    const int *p;
    long start;
    unsigned int offset;
    int i;
    SpriteSet *set;

    for (p = select; p[0] != SFF_SELECT_END; p += 2) {
        wanted_count++;
    }
    start = Stream_Tell(s);
    set = calloc(1, sizeof(SpriteSet));
    if (set == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    /* The set leaks when these fail. */
    if (wanted_count > 0) {
        set->sprites = KeyedArray_Create(wanted_count, sizeof(Sprite));
        if (set->sprites == NULL) {
            return NULL;
        }
    }
    set->palettes = KeyedArray_Create(1, sizeof(Palette));
    if (set->palettes == NULL) {
        return NULL;
    }
    offset = hdr->first_offset;
    if (hdr->image_count > 0) {
        i = 0;
        for (;;) {
            int wanted = 0;

            Stream_Seek(s, Sff_StreamPos(offset, start), SEEK_SET);
            if (!Sff1_ReadSubHeader(&sub, s)) {
                goto fail;
            }
            /* Selected and not loaded yet. */
            for (p = select; p[0] != SFF_SELECT_END; p += 2) {
                if (sub.group == p[0] && sub.number == p[1]) {
                    KeyedArray *sprites = set->sprites;
                    int index = KeyedArray_Find(sprites, p[0], p[1]);
                    if (index == -1 || sprites->data + index * sprites->elem_size == NULL) {
                        wanted = 1;
                        break;
                    }
                }
            }
            if (sub.size > 0) {
                Palette *new_pal = NULL;
                if (last_pal == -1 || !sub.same_palette) {
                    new_pal = &pal;
                }
                if (wanted) {
                    void *pixels = Pcx_Decode(new_pal, &w, s, &h, &bpp, &size, 0);
                    if (pixels == NULL) {
                        goto fail;
                    }
                    if (bpp > 8) {
                        new_pal = NULL;
                    } else if (new_pal != NULL && !SpriteSet_AddPalette(set, new_pal, 0, pal_count)) {
                        goto fail;
                    }
                    if (!Sprite_Init(SPRITE_INIT_OWN | SPRITE_INIT_DECODE, &spr, SPRITE_FORMAT_RAW, pixels, size,
                                     w, h, bpp, sub.x, sub.y)) {
                        goto fail;
                    }
                    if (new_pal != NULL) {
                        SpriteSet_LinkPalette(set, &spr, 0, pal_count);
                        last_pal = pal_count++;
                    } else if (sub.same_palette) {
                        SpriteSet_LinkPalette(set, &spr, 0, last_pal);
                    }
                    SpriteSet_AddSprite(set, &spr, sub.group, sub.number);
                    loaded++;
                }
            }
            if (loaded >= wanted_count) {
                break;
            }
            offset = sub.next_offset;
            if (++i >= hdr->image_count) {
                break;
            }
        }
    }
    Stream_Seek(s, start, SEEK_SET);
    return set;

fail:
    SpriteSet_Free(set);
    return NULL;
}

/* ---- SFF v2.x ---- */

/*
 * Links spr to the palette of node index pal_index: the palette that went
 * into slot pal_index of the palette table (palettes fill it in file order).
 */
static void Sff2_LinkPalette(SpriteSet *set, Sprite *spr, const Sff2SpriteNode *node, const Sff2Header *hdr)
{
    if (node->depth <= 8 && node->palette < hdr->palette_count) {
        const KeyedArraySlot *slot = &set->palettes->slots[node->palette];
        SpriteSet_LinkPalette(set, spr, slot->key1, slot->key2);
    }
}

/*
 * 0x004680F0: sprites of a v2.x file: all of them when select is empty, else
 * the selected ones (palettes are always loaded). With everything selected
 * the ldata block is read at once into SpriteSet.data and sprites point
 * into it; otherwise only the data of selected sprites is read into it.
 * Sprites in the tdata block get their own decoded copy.
 *
 * Quirks kept: the tdata offset is used as a stream position without the
 * header adjustment, the tdata buffer leaks when Sprite_Init fails, and the
 * alpha of v2.00 palettes is patched in the file data (color 0 transparent).
 */
SpriteSet *Sff2_Load(Stream *s, const Sff2Header *hdr, const int *select)
{
    Sff2SpriteNode *sprite_nodes = NULL;
    Sff2PaletteNode *pal_nodes = NULL;
    unsigned int data_pos = 0;
    int load_all = select[0] == SFF_SELECT_END;
    int v200 = hdr->version == SFF_VERSION_200;
    long start = Stream_Tell(s);
    unsigned char *data;
    size_t data_size;
    unsigned int i;
    long pos;
    SpriteSet *set;
    Palette pal;
    Sprite spr;

    set = SpriteSet_Create(hdr->sprite_count, hdr->palette_count);
    if (set == NULL) {
        return NULL;
    }
    if (hdr->palette_count != 0) {
        pal_nodes = calloc(hdr->palette_count, sizeof(Sff2PaletteNode));
        if (pal_nodes == NULL) {
            g_outOfMemory = 1;
            SpriteSet_Free(set);
            return NULL;
        }
        pos = Sff_StreamPos(hdr->palette_offset, start);
        if (pos != Stream_Tell(s)) {
            Stream_Seek(s, pos, SEEK_SET);
        }
        for (i = 0; i < hdr->palette_count; i++) {
            if (!Sff2_ReadPaletteNode(&pal_nodes[i], s)) {
                free(pal_nodes);
                SpriteSet_Free(set);
                return NULL;
            }
        }
    }
    if (hdr->sprite_count != 0) {
        sprite_nodes = calloc(hdr->sprite_count, sizeof(Sff2SpriteNode));
        if (sprite_nodes == NULL) {
            g_outOfMemory = 1;
            if (pal_nodes != NULL) {
                free(pal_nodes);
            }
            SpriteSet_Free(set);
            return NULL;
        }
        pos = Sff_StreamPos(hdr->sprite_offset, start);
        if (pos != Stream_Tell(s)) {
            Stream_Seek(s, pos, SEEK_SET);
        }
        for (i = 0; i < hdr->sprite_count; i++) {
            if (!Sff2_ReadSpriteNode(&sprite_nodes[i], s)) {
                free(sprite_nodes);
                if (pal_nodes != NULL) {
                    free(pal_nodes);
                }
                SpriteSet_Free(set);
                return NULL;
            }
        }
    }

    /* Size of the data block: the whole ldata, or the palettes and selected ldata sprites. */
    if (load_all) {
        data_size = hdr->ldata_size;
    } else {
        data_size = 0;
        for (i = 0; i < hdr->palette_count; i++) {
            data_size += pal_nodes[i].data_size;
        }
        for (i = 0; i < hdr->sprite_count; i++) {
            const Sff2SpriteNode *node = &sprite_nodes[i];
            const int *p;
            for (p = select; p[0] != SFF_SELECT_END; p += 2) {
                if (node->group == p[0] && node->number == p[1]) {
                    if (!(node->flags & SFF2_SPRITE_TDATA)) {
                        data_size += node->data_size;
                    }
                    break;
                }
            }
        }
    }
    data = malloc(data_size);
    set->data = data;
    if (data == NULL) {
        goto fail_nodes;
    }
    if (load_all) {
        pos = Sff_StreamPos(hdr->ldata_offset, start);
        if (pos != Stream_Tell(s)) {
            Stream_Seek(s, pos, SEEK_SET);
        }
        if (Stream_Read(data, hdr->ldata_size, 1, s) < 1) {
            goto fail_nodes;
        }
    }

    for (i = 0; i < hdr->palette_count; i++) {
        const Sff2PaletteNode *node = &pal_nodes[i];
        if (node->data_size != 0) {
            unsigned int at;
            if (!load_all) {
                Stream_Seek(s, Sff_StreamPos(hdr->ldata_offset + node->data_offset, start), SEEK_SET);
                if (Stream_Read(data + data_pos, node->data_size, 1, s) < 1) {
                    goto fail_nodes;
                }
            } else {
                data_pos = node->data_offset;
            }
            at = data_pos;
            if (v200) {
                int n = node->ncolors;
                int k;
                if (n > 0) {
                    data[at + 3] = 0;
                }
                for (k = 1; k < n; k++) {
                    data[at + k * 4 + 3] = 0xFF;
                }
            }
            if (!Pal_Init(&pal, node->ncolors, data + at, 0)) {
                goto fail_nodes;
            }
            if (!load_all) {
                data_pos = at + node->data_size;
            }
        } else {
            KeyedArray *palettes = set->palettes;
            const Palette *src = (const Palette *)(palettes->data + node->link * palettes->elem_size);
            if (src == NULL) {
                goto fail_nodes;
            }
            pal = *src;
            pal.owns = 0;
        }
        if (!SpriteSet_AddPalette(set, &pal, node->group, node->number)) {
            goto fail_nodes;
        }
    }
    if (pal_nodes != NULL) {
        free(pal_nodes);
    }

    for (i = 0; i < hdr->sprite_count; i++) {
        const Sff2SpriteNode *node = &sprite_nodes[i];
        if (!load_all) {
            const int *p;
            for (p = select;; p += 2) {
                if (p[0] == SFF_SELECT_END) {
                    goto next_sprite;
                }
                if (node->group == p[0] && node->number == p[1]) {
                    break;
                }
            }
        }
        if (node->data_size != 0) {
            if (node->flags & SFF2_SPRITE_TDATA) {
                void *buf = malloc(node->data_size);
                if (buf == NULL) {
                    goto fail_sprites;
                }
                Stream_Seek(s, (long)(node->data_offset + hdr->tdata_offset), SEEK_SET);
                if (Stream_Read(buf, node->data_size, 1, s) < 1) {
                    free(buf);
                    goto fail_sprites;
                }
                if (!Sprite_Init(SPRITE_INIT_COPY | SPRITE_INIT_DECODE, &spr, node->format, buf, node->data_size,
                                 (short)node->w, (short)node->h, node->depth, node->x, node->y)) {
                    free(NULL);
                    SpriteSet_Free(set);
                    free(sprite_nodes);
                    return NULL;
                }
                free(buf);
            } else {
                unsigned int flags = (g_gfxFlags & 1) ? SPRITE_INIT_DECODE : 0;
                unsigned int at;
                if (!load_all) {
                    Stream_Seek(s, Sff_StreamPos(node->data_offset + hdr->ldata_offset, start), SEEK_SET);
                    if (Stream_Read(data + data_pos, node->data_size, 1, s) < 1) {
                        goto fail_sprites;
                    }
                    at = data_pos;
                } else {
                    at = node->data_offset;
                    data_pos = at;
                }
                if (!Sprite_Init(flags, &spr, node->format, data + at, node->data_size,
                                 (short)node->w, (short)node->h, node->depth, node->x, node->y)) {
                    goto fail_sprites;
                }
                if (!load_all) {
                    data_pos = at + node->data_size;
                }
            }
            Sff2_LinkPalette(set, &spr, node, hdr);
        } else {
            KeyedArray *sprites = set->sprites;
            const Sprite *src = (const Sprite *)(sprites->data + node->link * sprites->elem_size);
            if (src == NULL) {
                goto fail_sprites;
            }
            spr = *src;
            spr.owns_rows = 0;
            spr.owns_pixels = 0;
            spr.x = node->x;
            spr.y = node->y;
            Sff2_LinkPalette(set, &spr, node, hdr);
        }
        if (!SpriteSet_AddSprite(set, &spr, node->group, node->number)) {
            goto fail_sprites;
        }
    next_sprite:;
    }
    if (sprite_nodes != NULL) {
        free(sprite_nodes);
    }
    Stream_Seek(s, start, SEEK_SET);
    return set;

fail_nodes:
    if (sprite_nodes != NULL) {
        free(sprite_nodes);
    }
    if (pal_nodes != NULL) {
        free(pal_nodes);
    }
    SpriteSet_Free(set);
    return NULL;

fail_sprites:
    free(sprite_nodes);
    SpriteSet_Free(set);
    return NULL;
}

/* ---- entry points ---- */

/*
 * 0x00467680: every sprite of the SFF file at the stream position (the
 * position is restored).
 */
SpriteSet *Sff_LoadStream(Stream *s)
{
    SffHeader hdr;
    unsigned int version = 0;
    SpriteSet *set = NULL;
    long start = Stream_Tell(s);

    if (Sff_ReadHeader(s, &hdr, &version)) {
        MatchLog_Printf(SFF_LOG_INFO, "SFF version %08X (compatver %08X)\n", version, hdr.v2.compat_version);
        if (version == SFF_VERSION_101) {
            set = Sff1_Load(&hdr.v1, s);
        } else if (hdr.v2.compat_version > SFF_VERSION_201) {
            MatchLog_Printf(SFF_LOG_ERROR, "SFF version not supported: %08X (compatver %08X)\n", version,
                            hdr.v2.compat_version);
        } else {
            int select_all = SFF_SELECT_END;
            set = Sff2_Load(s, &hdr.v2, &select_all);
        }
        if (set != NULL) {
            set->version = hdr.v1.version;
        }
    }
    Stream_Seek(s, start, SEEK_SET);
    return set;
}

/*
 * 0x00467B30: the sprites in select ((group, number) pairs ended by -1) of
 * the SFF file at the stream position (the position is restored).
 */
SpriteSet *Sff_LoadSelected(Stream *s, const int *select)
{
    SffHeader hdr;
    unsigned int version = 0;
    SpriteSet *set = NULL;
    long start = Stream_Tell(s);

    if (Sff_ReadHeader(s, &hdr, &version)) {
        if (version == SFF_VERSION_101) {
            set = Sff1_LoadSelected(s, &hdr.v1, select);
        } else if (hdr.v2.compat_version > SFF_VERSION_201) {
            MatchLog_Printf(SFF_LOG_ERROR, "SFF version not supported: %08X (compatver %08X)\n", version,
                            hdr.v2.compat_version);
        } else {
            set = Sff2_Load(s, &hdr.v2, select);
        }
        if (set != NULL) {
            set->version = hdr.v1.version;
        }
    }
    Stream_Seek(s, start, SEEK_SET);
    return set;
}

/*
 * 0x00467510: loads a sprite file (.sff, or a .pcx image) and logs its
 * estimated memory use. With BufferedRead the file is read into memory first.
 */
SpriteSet *Sff_Load(const char *path)
{
    unsigned int sys_bytes = 0;
    unsigned int tex_bytes = 0;
    SpriteSet *set;
    Stream *s;

    if (!g_bufferedRead) {
        s = Stream_OpenFile(path, "rb");
        if (s == NULL) {
            return NULL;
        }
    } else {
        FILE *fp = fopen(path, "rb");
        long pos, size;
        void *buf;

        if (fp == NULL) {
            return NULL;
        }
        pos = ftell(fp);
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, pos, SEEK_SET);
        buf = malloc(size);
        if (buf == NULL) {
            fclose(fp);
            return NULL;
        }
        if (fread(buf, size, 1, fp) < 1) {
            fclose(fp);
            free(buf);
            return NULL;
        }
        fclose(fp);
        s = Stream_OpenMemory(buf, size);
        if (s == NULL) {
            free(buf);
            return NULL;
        }
        s->owns_buf = 1;
    }
    Stream_Seek(s, 0, SEEK_SET);
    if (File_GetType(path) == FILETYPE_PCX) {
        set = Pcx_Load(s, 0);
    } else {
        set = Sff_LoadStream(s);
    }
    Stream_Close(s);
    if (set == NULL) {
        return NULL;
    }
    SpriteSet_MemUse(set, &sys_bytes, &tex_bytes);
    MatchLog_Printf(SFF_LOG_INFO, "SFF %s estimated mem use: %uk sys %uk tex\n", path, sys_bytes >> 10,
                    tex_bytes >> 10);
    return set;
}
