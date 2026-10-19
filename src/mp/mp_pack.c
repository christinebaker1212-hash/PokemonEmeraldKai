// Moemon Co-op: trainer sprite packs. See mp_pack.h for the atlas layout.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO_WRAPPERS
#define STBI_NO_SIMD
#include "third_party/stb_image.h"

#include "global.h"
#include "mp.h"
#include "mp_pack.h"
#include "mp_gfx_externs.h"
#include "data.h"
#include "event_object_movement.h"
#include "sprite.h"
#include "constants/event_objects.h"
#include "constants/trainers.h"
#include "constants/rgb.h"
#include "gba/syscall.h"

const u16 *MP_GetObjectEventPaletteData(u16 tag);
extern const struct ObjectEventGraphicsInfo *const gObjectEventGraphicsInfoPointers[];

const struct MpPackSlotInfo gMpPackSlots[PACK_SLOT_COUNT] = {
    [PACK_WALK]       = {"WALK",       0,   0, 16, 32,  9, PACK_PAL_OVERWORLD, 0},
    [PACK_RUN]        = {"RUN",        144, 0, 16, 32,  9, PACK_PAL_OVERWORLD, 0},
    [PACK_DECORATE]   = {"DECORATE",   288, 0, 16, 32,  1, PACK_PAL_OVERWORLD, 0},
    [PACK_MACH_BIKE]  = {"MACH BIKE",  0,  32, 32, 32,  9, PACK_PAL_OVERWORLD, 0},
    [PACK_DIVE]       = {"DIVE",       288, 32, 32, 32,  9, PACK_PAL_DIVE, 0},
    [PACK_ACRO_BIKE]  = {"ACRO BIKE",  0,  64, 32, 32, 27, PACK_PAL_OVERWORLD, 18},
    [PACK_WATERING]   = {"WATERING",   288, 96, 32, 32,  9, PACK_PAL_OVERWORLD, 0},
    [PACK_SURF]       = {"SURF/FLY",   0, 128, 32, 32, 12, PACK_PAL_OVERWORLD, 0},
    [PACK_FIELD_MOVE] = {"USE MOVE",   384, 128, 32, 32, 5, PACK_PAL_OVERWORLD, 0},
    [PACK_FISHING]    = {"FISHING",    0, 160, 32, 32, 12, PACK_PAL_OVERWORLD, 0},
    [PACK_FRONT]      = {"FRONT",      0, 192, 64, 64,  1, PACK_PAL_FRONT, 0},
    [PACK_BACK]       = {"BACK",       64, 192, 64, 64, 4, PACK_PAL_BACK, 0},
};

// Blob: u32 magic, u8 version, u8 base, u16 gfxId, u16 slotMask, u16 pad,
//       u16 pal[PACK_PAL_COUNT][16], then each present slot's tiles in slot order.
#define HEADER_SIZE (12 + PACK_PAL_COUNT * 32)

u32 MP_Pack_FrameBytes(u8 slot)
{
    return gMpPackSlots[slot].w * gMpPackSlots[slot].h / 2;
}

u32 MP_Pack_SlotBytes(u8 slot)
{
    return MP_Pack_FrameBytes(slot) * gMpPackSlots[slot].frames;
}

static void FramePos(u8 slot, int frame, int *x, int *y)
{
    const struct MpPackSlotInfo *s = &gMpPackSlots[slot];
    int col = frame, row = 0;
    if (s->wrapAfter)
    {
        col = frame % s->wrapAfter;
        row = frame / s->wrapAfter;
    }
    *x = s->x + col * s->w;
    *y = s->y + row * s->h;
}

bool8 MP_Pack_Parse(const u8 *blob, u32 size, struct MpPack *out)
{
    u32 pos = HEADER_SIZE, magic;
    u8 i;

    memset(out, 0, sizeof(*out));
    if (blob == NULL || size < HEADER_SIZE)
        return FALSE;
    magic = blob[0] | (blob[1] << 8) | (blob[2] << 16) | ((u32)blob[3] << 24);
    if (magic != PACK_MAGIC || blob[4] != 1)
        return FALSE;
    out->base = blob[5] < MP_BASE_COUNT ? blob[5] : MP_BASE_DEFAULT;
    out->gfxId = blob[6] | (blob[7] << 8);
    out->slotMask = blob[8] | (blob[9] << 8);
    memcpy(out->pal, blob + 12, sizeof(out->pal));
    for (i = 0; i < PACK_SLOT_COUNT; i++)
    {
        if (!(out->slotMask & (1 << i)))
            continue;
        if (pos + MP_Pack_SlotBytes(i) > size)
            return FALSE;
        out->slot[i] = blob + pos;
        pos += MP_Pack_SlotBytes(i);
    }
    return pos == size;
}

// ---------------------------------------------------------------------------
// Color reduction (median cut on 15-bit colors)

struct ColorCount
{
    u16 color;   // RGB555
    u32 count;
};

struct Box
{
    int start, end;  // range in the color array
};

static int Channel(u16 c, int ch)
{
    return (c >> (ch * 5)) & 0x1F;
}

static int CompareChannel;
static int CmpColors(const void *a, const void *b)
{
    return Channel(((const struct ColorCount *)a)->color, CompareChannel)
         - Channel(((const struct ColorCount *)b)->color, CompareChannel);
}

// Reduces colors[0..n) to at most `max` representative colors in out[]
static int MedianCut(struct ColorCount *colors, int n, u16 *out, int max)
{
    struct Box boxes[16];
    int nBoxes = 1, i;

    if (n <= max)
    {
        for (i = 0; i < n; i++)
            out[i] = colors[i].color;
        return n;
    }
    boxes[0].start = 0;
    boxes[0].end = n;
    while (nBoxes < max)
    {
        int best = -1, bestRange = -1, bestCh = 0, b;
        for (b = 0; b < nBoxes; b++)
        {
            int ch;
            if (boxes[b].end - boxes[b].start < 2)
                continue;
            for (ch = 0; ch < 3; ch++)
            {
                int lo = 31, hi = 0, k;
                for (k = boxes[b].start; k < boxes[b].end; k++)
                {
                    int v = Channel(colors[k].color, ch);
                    if (v < lo) lo = v;
                    if (v > hi) hi = v;
                }
                if (hi - lo > bestRange)
                {
                    bestRange = hi - lo;
                    best = b;
                    bestCh = ch;
                }
            }
        }
        if (best < 0)
            break;
        CompareChannel = bestCh;
        qsort(colors + boxes[best].start, boxes[best].end - boxes[best].start, sizeof(colors[0]), CmpColors);
        {
            // split at the weighted median
            u32 total = 0, acc = 0;
            int k, split;
            for (k = boxes[best].start; k < boxes[best].end; k++)
                total += colors[k].count;
            split = boxes[best].start + 1;
            for (k = boxes[best].start; k < boxes[best].end - 1; k++)
            {
                acc += colors[k].count;
                if (acc * 2 >= total)
                {
                    split = k + 1;
                    break;
                }
            }
            boxes[nBoxes].start = split;
            boxes[nBoxes].end = boxes[best].end;
            boxes[best].end = split;
            nBoxes++;
        }
    }
    for (i = 0; i < nBoxes; i++)
    {
        u32 r = 0, g = 0, b = 0, w = 0;
        int k;
        for (k = boxes[i].start; k < boxes[i].end; k++)
        {
            r += Channel(colors[k].color, 0) * colors[k].count;
            g += Channel(colors[k].color, 1) * colors[k].count;
            b += Channel(colors[k].color, 2) * colors[k].count;
            w += colors[k].count;
        }
        if (w == 0)
            w = 1;
        out[i] = RGB((r + w / 2) / w, (g + w / 2) / w, (b + w / 2) / w);
    }
    return nBoxes;
}

static int Nearest(const u16 *pal, int n, u16 c)
{
    int i, best = 1, bestD = 1 << 30;
    for (i = 1; i < n; i++)
    {
        int dr = Channel(pal[i], 0) - Channel(c, 0);
        int dg = Channel(pal[i], 1) - Channel(c, 1);
        int db = Channel(pal[i], 2) - Channel(c, 2);
        int d = dr * dr * 3 + dg * dg * 4 + db * db * 2;
        if (d < bestD)
        {
            bestD = d;
            best = i;
        }
    }
    return best;
}

static u16 To555(const u8 *p)
{
    return RGB(p[0] >> 3, p[1] >> 3, p[2] >> 3);
}

// ---------------------------------------------------------------------------
// Loading

static bool8 SlotHasPixels(const u8 *rgba, int w, u8 slot)
{
    int f, x, y, fx, fy;
    for (f = 0; f < gMpPackSlots[slot].frames; f++)
    {
        FramePos(slot, f, &fx, &fy);
        for (y = 0; y < gMpPackSlots[slot].h; y++)
            for (x = 0; x < gMpPackSlots[slot].w; x++)
                if (rgba[((fy + y) * w + fx + x) * 4 + 3] >= 128)
                    return TRUE;
    }
    return FALSE;
}

// Builds the palette for one group of slots
static void BuildPalette(const u8 *rgba, int w, u16 slotMask, u8 group, u16 *pal)
{
    struct ColorCount *colors = malloc(sizeof(struct ColorCount) * 4096);
    int n = 0, i, cap = 4096;
    u8 slot;

    memset(pal, 0, 32);
    pal[0] = RGB(14, 24, 20); // transparent (never shown)
    for (slot = 0; slot < PACK_SLOT_COUNT; slot++)
    {
        int f, x, y, fx, fy;
        if (gMpPackSlots[slot].palette != group || !(slotMask & (1 << slot)))
            continue;
        for (f = 0; f < gMpPackSlots[slot].frames; f++)
        {
            FramePos(slot, f, &fx, &fy);
            for (y = 0; y < gMpPackSlots[slot].h; y++)
                for (x = 0; x < gMpPackSlots[slot].w; x++)
                {
                    const u8 *p = rgba + ((fy + y) * w + fx + x) * 4;
                    u16 c;
                    if (p[3] < 128)
                        continue;
                    c = To555(p);
                    for (i = 0; i < n; i++)
                        if (colors[i].color == c)
                            break;
                    if (i == n)
                    {
                        if (n == cap)
                        {
                            cap *= 2;
                            colors = realloc(colors, sizeof(struct ColorCount) * cap);
                        }
                        colors[n].color = c;
                        colors[n].count = 0;
                        n++;
                    }
                    colors[i].count++;
                }
        }
    }
    MedianCut(colors, n, pal + 1, 15);
    free(colors);
}

static void PackFrame(const u8 *rgba, int w, int fx, int fy, int fw, int fh, const u16 *pal, int palCount, u8 *tiles)
{
    int tx, ty, x, y;
    int tilesW = fw / 8;
    memset(tiles, 0, fw * fh / 2);
    for (ty = 0; ty < fh / 8; ty++)
        for (tx = 0; tx < tilesW; tx++)
        {
            u8 *tile = tiles + (ty * tilesW + tx) * 32;
            for (y = 0; y < 8; y++)
                for (x = 0; x < 8; x++)
                {
                    const u8 *p = rgba + ((fy + ty * 8 + y) * w + fx + tx * 8 + x) * 4;
                    int idx = (p[3] < 128) ? 0 : Nearest(pal, palCount, To555(p));
                    int k = y * 8 + x;
                    tile[k / 2] |= (k & 1) ? (idx << 4) : idx;
                }
        }
}

// Finds the "MoemonPack" text chunk ("base=hiker")
static void ReadPackBase(const u8 *png, long size, u8 *base, u16 *gfxId)
{
    long pos = 8;
    while (pos + 12 <= size)
    {
        u32 len = ((u32)png[pos] << 24) | (png[pos + 1] << 16) | (png[pos + 2] << 8) | png[pos + 3];
        const u8 *type = png + pos + 4;
        if (pos + 12 + (long)len > size)
            break;
        if (memcmp(type, "tEXt", 4) == 0 && len > 11 && memcmp(png + pos + 8, "MoemonPack", 10) == 0 && png[pos + 18] == 0)
        {
            char text[96];
            int n = (int)len - 11;
            if (n > 95)
                n = 95;
            memcpy(text, png + pos + 19, n);
            text[n] = 0;
            if (strncmp(text, "base=", 5) == 0)
            {
                struct MpAppearance app;
                memset(&app, 0, sizeof(app));
                if (MP_Avatar_ParseSpriteName(text + 5, &app))
                {
                    *base = app.base;
                    *gfxId = app.gfxId;
                }
            }
        }
        if (memcmp(type, "IEND", 4) == 0)
            break;
        pos += 12 + len;
    }
}

u8 *MP_Pack_LoadFile(const char *path, u32 *sizeOut, char *err, int errLen)
{
    FILE *f = fopen(path, "rb");
    long fileSize;
    u8 *file, *rgba, *blob, *atlas = NULL;
    int w, h, comp, i;
    u8 base = MP_BASE_DEFAULT, slot;
    u16 gfxId = 0, mask = 0;
    u16 pal[PACK_PAL_COUNT][16];
    u32 size;

    if (!f)
    {
        snprintf(err, errLen, "can't open %s", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fileSize <= 0 || fileSize > 8 * 1024 * 1024)
    {
        fclose(f);
        snprintf(err, errLen, "%s is not a sprite file", path);
        return NULL;
    }
    file = malloc(fileSize);
    if (fread(file, 1, fileSize, f) != (size_t)fileSize)
    {
        fclose(f);
        free(file);
        snprintf(err, errLen, "can't read %s", path);
        return NULL;
    }
    fclose(f);
    rgba = stbi_load_from_memory(file, (int)fileSize, &w, &h, &comp, 4);
    if (!rgba)
    {
        free(file);
        snprintf(err, errLen, "%s is not a PNG image", path);
        return NULL;
    }
    ReadPackBase(file, fileSize, &base, &gfxId);
    free(file);

    // Images without any transparency: the top-left color is the background
    {
        bool8 hasAlpha = FALSE;
        for (i = 0; i < w * h; i++)
            if (rgba[i * 4 + 3] < 255)
                hasAlpha = TRUE;
        if (!hasAlpha)
        {
            u8 key[3] = {rgba[0], rgba[1], rgba[2]};
            for (i = 0; i < w * h; i++)
                if (memcmp(rgba + i * 4, key, 3) == 0)
                    rgba[i * 4 + 3] = 0;
        }
    }

    if (w == PACK_ATLAS_W && h == PACK_ATLAS_H)
    {
        atlas = rgba;
    }
    else if (h == 32 && (w == 144 || w == 288))
    {
        // Old walking (+ running) sheet: place it into an empty atlas
        int y;
        atlas = calloc(PACK_ATLAS_W * PACK_ATLAS_H, 4);
        for (y = 0; y < 32; y++)
            memcpy(atlas + y * PACK_ATLAS_W * 4, rgba + y * w * 4, w * 4);
        stbi_image_free(rgba);
        rgba = NULL;
        base = MP_BASE_DEFAULT;
    }
    else
    {
        stbi_image_free(rgba);
        snprintf(err, errLen, "sprite sheet must be %dx%d (made with the Sprite Editor), got %dx%d", PACK_ATLAS_W, PACK_ATLAS_H, w, h);
        return NULL;
    }

    for (slot = 0; slot < PACK_SLOT_COUNT; slot++)
        if (SlotHasPixels(atlas, PACK_ATLAS_W, slot))
            mask |= 1 << slot;
    if (mask == 0)
    {
        if (rgba) stbi_image_free(rgba); else free(atlas);
        snprintf(err, errLen, "the sprite sheet is empty");
        return NULL;
    }
    for (i = 0; i < PACK_PAL_COUNT; i++)
        BuildPalette(atlas, PACK_ATLAS_W, mask, i, pal[i]);

    size = HEADER_SIZE;
    for (slot = 0; slot < PACK_SLOT_COUNT; slot++)
        if (mask & (1 << slot))
            size += MP_Pack_SlotBytes(slot);
    blob = calloc(1, size);
    blob[0] = PACK_MAGIC & 0xFF; blob[1] = (PACK_MAGIC >> 8) & 0xFF;
    blob[2] = (PACK_MAGIC >> 16) & 0xFF; blob[3] = PACK_MAGIC >> 24;
    blob[4] = 1;
    blob[5] = base;
    blob[6] = gfxId & 0xFF; blob[7] = gfxId >> 8;
    blob[8] = mask & 0xFF; blob[9] = mask >> 8;
    memcpy(blob + 12, pal, sizeof(pal));
    {
        u32 pos = HEADER_SIZE;
        for (slot = 0; slot < PACK_SLOT_COUNT; slot++)
        {
            int fr;
            if (!(mask & (1 << slot)))
                continue;
            for (fr = 0; fr < gMpPackSlots[slot].frames; fr++)
            {
                int fx, fy;
                FramePos(slot, fr, &fx, &fy);
                PackFrame(atlas, PACK_ATLAS_W, fx, fy, gMpPackSlots[slot].w, gMpPackSlots[slot].h,
                          pal[gMpPackSlots[slot].palette], 16, blob + pos);
                pos += MP_Pack_FrameBytes(slot);
            }
        }
    }
    if (rgba) stbi_image_free(rgba); else free(atlas);
    *sizeOut = size;
    return blob;
}

// ---------------------------------------------------------------------------
// Exporting an editable PNG

static void PutFrame(u8 *atlas, u8 slot, int frame, const u8 *tiles, const u16 *pal)
{
    int fx, fy, tx, ty, x, y;
    int fw = gMpPackSlots[slot].w, fh = gMpPackSlots[slot].h, tilesW = fw / 8;
    FramePos(slot, frame, &fx, &fy);
    for (ty = 0; ty < fh / 8; ty++)
        for (tx = 0; tx < tilesW; tx++)
        {
            const u8 *tile = tiles + (ty * tilesW + tx) * 32;
            for (y = 0; y < 8; y++)
                for (x = 0; x < 8; x++)
                {
                    int k = y * 8 + x;
                    u8 idx = (k & 1) ? (tile[k / 2] >> 4) : (tile[k / 2] & 0xF);
                    u8 *p = atlas + ((fy + ty * 8 + y) * PACK_ATLAS_W + fx + tx * 8 + x) * 4;
                    if (idx == 0)
                        continue;
                    p[0] = Channel(pal[idx], 0) * 255 / 31;
                    p[1] = Channel(pal[idx], 1) * 255 / 31;
                    p[2] = Channel(pal[idx], 2) * 255 / 31;
                    p[3] = 255;
                }
        }
}

static void PutInfoFrames(u8 *atlas, u8 slot, const struct ObjectEventGraphicsInfo *info, int firstImage)
{
    const u16 *pal = MP_Avatar_OriginalPalette(info->paletteTag);
    int i;
    if (!pal || info->width != gMpPackSlots[slot].w || info->height != gMpPackSlots[slot].h)
        return;
    for (i = 0; i < gMpPackSlots[slot].frames; i++)
        PutFrame(atlas, slot, i, info->images[firstImage + i].data, pal);
}

static u32 sCrcTable[256];

static u32 Crc32(u32 crc, const u8 *p, u32 n)
{
    u32 i;
    if (sCrcTable[1] == 0)
    {
        u32 k, c;
        for (k = 0; k < 256; k++)
        {
            c = k;
            for (i = 0; i < 8; i++)
                c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            sCrcTable[k] = c;
        }
    }
    crc = ~crc;
    for (i = 0; i < n; i++)
        crc = sCrcTable[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

static void PutBE32(u8 *p, u32 v)
{
    p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v;
}

static void WriteChunk(FILE *f, const char *type, const u8 *data, u32 len)
{
    u8 hdr[8];
    u32 crc;
    PutBE32(hdr, len);
    memcpy(hdr + 4, type, 4);
    fwrite(hdr, 1, 8, f);
    if (len)
        fwrite(data, 1, len, f);
    crc = Crc32(0, hdr + 4, 4);
    crc = Crc32(crc, data, len);
    PutBE32(hdr, crc);
    fwrite(hdr, 1, 4, f);
}

// Minimal PNG writer: RGBA, zlib "stored" blocks (no compression library needed)
static bool8 WritePng(const char *path, const u8 *rgba, int w, int h, const char *text)
{
    FILE *f = fopen(path, "wb");
    static const u8 sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    u8 ihdr[13];
    u32 rawLen = (u32)(w * 4 + 1) * h, zLen, a = 1, b = 0, pos = 0, i;
    u8 *raw, *z;
    int y;

    if (!f)
        return FALSE;
    raw = malloc(rawLen);
    for (y = 0; y < h; y++)
    {
        raw[y * (w * 4 + 1)] = 0;
        memcpy(raw + y * (w * 4 + 1) + 1, rgba + y * w * 4, w * 4);
    }
    zLen = 2 + rawLen + 5 * (rawLen / 65535 + 1) + 4;
    z = malloc(zLen);
    z[pos++] = 0x78;
    z[pos++] = 0x01;
    for (i = 0; i < rawLen; )
    {
        u32 n = rawLen - i > 65535 ? 65535 : rawLen - i;
        z[pos++] = (i + n == rawLen) ? 1 : 0;
        z[pos++] = n & 0xFF;
        z[pos++] = n >> 8;
        z[pos++] = ~n & 0xFF;
        z[pos++] = (~n >> 8) & 0xFF;
        memcpy(z + pos, raw + i, n);
        pos += n;
        i += n;
    }
    for (i = 0; i < rawLen; i++)
    {
        a = (a + raw[i]) % 65521;
        b = (b + a) % 65521;
    }
    PutBE32(z + pos, (b << 16) | a);
    pos += 4;

    fwrite(sig, 1, 8, f);
    PutBE32(ihdr, w);
    PutBE32(ihdr + 4, h);
    ihdr[8] = 8;  // bit depth
    ihdr[9] = 6;  // RGBA
    ihdr[10] = ihdr[11] = ihdr[12] = 0;
    WriteChunk(f, "IHDR", ihdr, 13);
    if (text)
    {
        u8 t[128];
        int n = snprintf((char *)t, sizeof(t), "MoemonPack");
        t[n] = 0;
        n += 1 + snprintf((char *)t + n + 1, sizeof(t) - n - 1, "%s", text);
        WriteChunk(f, "tEXt", t, n);
    }
    WriteChunk(f, "IDAT", z, pos);
    WriteChunk(f, "IEND", NULL, 0);
    fclose(f);
    free(raw);
    free(z);
    return TRUE;
}

bool8 MP_WritePng(const char *path, const u8 *rgba, int w, int h, const char *text)
{
    return WritePng(path, rgba, w, h, text);
}

bool8 MP_Pack_Export(const char *path, const struct MpAppearance *appIn, const struct MpPack *pack)
{
    struct MpAppearance app = *appIn;
    u8 base = app.base;
    u16 gfxId = app.gfxId;
    u8 gender = app.gender;
    u8 *atlas = calloc(PACK_ATLAS_W * PACK_ATLAS_H, 4);
    static u8 sPicBuf[64 * 64 / 2 * 4];
    u16 pal[16];
    char text[64];
    bool8 ok;
    u8 slot;
    bool8 female = (gender == FEMALE);

    if (base == MP_BASE_DEFAULT || base == MP_BASE_CUSTOM)
        base = female ? MP_BASE_MAY : MP_BASE_BRENDAN;

    // The character's own overworld sprites. Only Brendan/May have all the
    // action sheets; other characters fall back to them in game, so those
    // slots are left empty here (empty = use the default).
    if (base == MP_BASE_BRENDAN || base == MP_BASE_MAY)
    {
        bool8 m = (base == MP_BASE_MAY);
        PutInfoFrames(atlas, PACK_WALK, m ? &gObjectEventGraphicsInfo_MayNormal : &gObjectEventGraphicsInfo_BrendanNormal, 0);
        PutInfoFrames(atlas, PACK_RUN, m ? &gObjectEventGraphicsInfo_MayNormal : &gObjectEventGraphicsInfo_BrendanNormal, 9);
        PutInfoFrames(atlas, PACK_DECORATE, m ? &gObjectEventGraphicsInfo_MayDecorating : &gObjectEventGraphicsInfo_BrendanDecorating, 0);
        PutInfoFrames(atlas, PACK_MACH_BIKE, m ? &gObjectEventGraphicsInfo_MayMachBike : &gObjectEventGraphicsInfo_BrendanMachBike, 0);
        PutInfoFrames(atlas, PACK_DIVE, m ? &gObjectEventGraphicsInfo_MayUnderwater : &gObjectEventGraphicsInfo_BrendanUnderwater, 0);
        PutInfoFrames(atlas, PACK_ACRO_BIKE, m ? &gObjectEventGraphicsInfo_MayAcroBike : &gObjectEventGraphicsInfo_BrendanAcroBike, 0);
        PutInfoFrames(atlas, PACK_WATERING, m ? &gObjectEventGraphicsInfo_MayWatering : &gObjectEventGraphicsInfo_BrendanWatering, 0);
        PutInfoFrames(atlas, PACK_SURF, m ? &gObjectEventGraphicsInfo_MaySurfing : &gObjectEventGraphicsInfo_BrendanSurfing, 0);
        PutInfoFrames(atlas, PACK_FIELD_MOVE, m ? &gObjectEventGraphicsInfo_MayFieldMove : &gObjectEventGraphicsInfo_BrendanFieldMove, 0);
        PutInfoFrames(atlas, PACK_FISHING, m ? &gObjectEventGraphicsInfo_MayFishing : &gObjectEventGraphicsInfo_BrendanFishing, 0);
    }
    else if (base == MP_BASE_RS_BRENDAN || base == MP_BASE_RS_MAY)
    {
        PutInfoFrames(atlas, PACK_WALK, base == MP_BASE_RS_MAY ? &gObjectEventGraphicsInfo_RubySapphireMay : &gObjectEventGraphicsInfo_RubySapphireBrendan, 0);
    }
    else if (base == MP_BASE_NPC && MP_Avatar_NpcInfo(gfxId))
    {
        const struct ObjectEventGraphicsInfo *npc = MP_Avatar_NpcInfo(gfxId);
        if (npc->width == 16 && npc->height == 32)
            PutInfoFrames(atlas, PACK_WALK, npc, 0);
    }
    else if (base == MP_BASE_CREATED && app.recipe.made)
    {
        // A CUSTOM trainer (LOOK > CREATE): its 32x32 field frames. Walking and
        // running are 16 wide in the sheet, so they get the middle 16 columns.
        const struct MpCreatedGfx *g = MP_Created_Gfx(MP_CR_CACHE_LOCAL, &app.recipe);
        static u8 sNarrow[16 * 32 / 2];
        int f, r;
        for (f = 0; f < 18; f++)
        {
            for (r = 0; r < 4; r++)   // 4 rows of 8x8 tiles: keep tile columns 1 and 2
                memcpy(sNarrow + r * 64, g->ow[f] + r * 128 + 32, 64);
            PutFrame(atlas, f < 9 ? PACK_WALK : PACK_RUN, f % 9, sNarrow, g->owPal);
        }
        for (f = 0; f < 9; f++)
            PutFrame(atlas, PACK_MACH_BIKE, f, g->ow[18 + f], g->owPal);
    }

    // The trainer pictures picked in LOOK (or the ones that match the sprite)
    app.base = base;
    app.hasCustom = FALSE;
    if (MP_Avatar_GetFrontTiles(&app, sPicBuf, pal))
        PutFrame(atlas, PACK_FRONT, 0, sPicBuf, pal);
    if (MP_Avatar_GetBackTiles(&app, sPicBuf, pal))
    {
        int i;
        for (i = 0; i < 4; i++)
            PutFrame(atlas, PACK_BACK, i, sPicBuf + i * 2048, pal);
    }

    // The player's own edits on top
    if (pack)
    {
        for (slot = 0; slot < PACK_SLOT_COUNT; slot++)
        {
            int fr, fx, fy, y;
            if (!pack->slot[slot])
                continue;
            for (fr = 0; fr < gMpPackSlots[slot].frames; fr++)
            {
                FramePos(slot, fr, &fx, &fy);
                for (y = 0; y < gMpPackSlots[slot].h; y++)
                    memset(atlas + ((fy + y) * PACK_ATLAS_W + fx) * 4, 0, gMpPackSlots[slot].w * 4);
                PutFrame(atlas, slot, fr, pack->slot[slot] + fr * MP_Pack_FrameBytes(slot), pack->pal[gMpPackSlots[slot].palette]);
            }
        }
    }

    {
        char name[40];
        struct MpAppearance named = app;
        if (named.base == MP_BASE_CREATED)   // the sheet stands on its own: Brendan/May fill in the rest
            named.base = female ? MP_BASE_MAY : MP_BASE_BRENDAN;
        MP_Avatar_SpriteConfigName(&named, name, sizeof(name));
        snprintf(text, sizeof(text), "base=%s", name);
    }
    ok = WritePng(path, atlas, PACK_ATLAS_W, PACK_ATLAS_H, text);
    free(atlas);
    return ok;
}

u32 MP_Pack_WriteLz77Literal(u8 *dst, const void *data, u32 size)
{
    const u8 *src = data;
    u32 pos = 4, i;
    dst[0] = 0x10;
    dst[1] = size & 0xFF;
    dst[2] = (size >> 8) & 0xFF;
    dst[3] = (size >> 16) & 0xFF;
    for (i = 0; i < size; i++)
    {
        if ((i & 7) == 0)
            dst[pos++] = 0; // 8 literals follow
        dst[pos++] = src[i];
    }
    return pos;
}
