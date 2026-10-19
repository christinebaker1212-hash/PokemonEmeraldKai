// Moemon Co-op: trainer sprite packs (custom sprites for every animation).
//
// A pack is one PNG ("sprite sheet atlas", 576x256) with a fixed layout, made
// with the Sprite Editor (dist_extras/Sprite Editor.html) or any paint program.
// Every slot is optional: an empty (fully transparent) slot falls back to the
// pack's base character, then to the default Brendan/May sprites.
//
// Atlas layout (x, y, frame w x h, frames):
//   WALK        0,   0  16x32  9    RUN        144,  0  16x32  9
//   DECORATE  288,   0  16x32  1
//   MACH BIKE   0,  32  32x32  9    DIVE       288, 32  32x32  9
//   ACRO BIKE   0,  64  32x32 18 (frames 0-17), then 0, 96 (frames 18-26)
//   WATERING  288,  96  32x32  9
//   SURF        0, 128  32x32 12    FIELD MOVE 384,128  32x32  5
//   FISHING     0, 160  32x32 12
//   FRONT       0, 192  64x64  1    BACK        64,192  64x64  4
// The overworld slots share one 15-color palette; DIVE, FRONT and BACK each
// have their own.
// A PNG text chunk "MoemonPack" = "base=<sprite name>" names the base character.
#ifndef GUARD_MP_PACK_H
#define GUARD_MP_PACK_H

#include "global.h"

enum {
    PACK_WALK,
    PACK_RUN,
    PACK_DECORATE,
    PACK_MACH_BIKE,
    PACK_DIVE,
    PACK_ACRO_BIKE,
    PACK_WATERING,
    PACK_SURF,
    PACK_FIELD_MOVE,
    PACK_FISHING,
    PACK_FRONT,
    PACK_BACK,
    PACK_SLOT_COUNT
};

enum {
    PACK_PAL_OVERWORLD,
    PACK_PAL_FRONT,
    PACK_PAL_BACK,
    PACK_PAL_DIVE,      // diving uses its own (underwater) colors
    PACK_PAL_COUNT
};

#define PACK_ATLAS_W 576
#define PACK_ATLAS_H 256
#define PACK_MAGIC   0x314B504D  // "MPK1"

struct MpPackSlotInfo
{
    const char *name;
    u16 x, y;
    u8 w, h;          // frame size in pixels
    u8 frames;
    u8 palette;       // PACK_PAL_*
    u8 wrapAfter;     // frames per row (0 = all on one row); next rows start at x again
};

extern const struct MpPackSlotInfo gMpPackSlots[PACK_SLOT_COUNT];

// Parsed view of a pack blob (pointers into the blob)
struct MpPack
{
    u16 slotMask;
    u8 base;            // MP_BASE_*
    u16 gfxId;          // when base == MP_BASE_NPC
    u16 pal[PACK_PAL_COUNT][16];
    const u8 *slot[PACK_SLOT_COUNT];   // 4bpp tiles, frame after frame, or NULL
};

u32 MP_Pack_FrameBytes(u8 slot);
u32 MP_Pack_SlotBytes(u8 slot);
bool8 MP_Pack_Parse(const u8 *blob, u32 size, struct MpPack *out);

// Loads a pack PNG (or an old 144x32 / 288x32 walking sheet) into a new blob
// (malloc'd). Returns NULL and fills err on failure.
u8 *MP_Pack_LoadFile(const char *path, u32 *sizeOut, char *err, int errLen);

// Writes the given character (plus pack, may be NULL) as an editable pack PNG.
struct MpAppearance;
bool8 MP_Pack_Export(const char *path, const struct MpAppearance *app, const struct MpPack *pack);

// Writes "data" as a valid LZ77 stream made only of literals (the game's
// decompressors accept it). dst needs 4 + size + size / 8 + 1 bytes.
u32 MP_Pack_WriteLz77Literal(u8 *dst, const void *data, u32 size);

#endif // GUARD_MP_PACK_H
