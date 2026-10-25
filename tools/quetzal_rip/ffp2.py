"""Decoder for Moemon Quetzal's "FFP2" overworld frame blobs (Pokemon overworld sprites).

The game's frame upload (ROM 0x08008200, check 0x08007E5C) treats an image pointer as
an index into one blob that holds up to 6 frames (the images array points at
blob + i * frame_size; the frame index is (ptr - blob) / frame_size).

Blob layout (little endian):
  +0x00 u32  0x32504646 "FFP2"
  +0x04 u32  0xCDAFB9B9 (second magic)
  +0x08 u32  blob size in bytes (0x28 < x <= 0x3028)
  +0x0C u16  frame size: 0x200 (32x32) or 0x800 (64x64), 4bpp
  +0x0E u16  0x0602
  +0x10 u32[6] per-frame offsets from the blob start; 0 = no frame
        bit 31 clear: raw 4bpp frame (frame_size bytes)   (copied by 0x08007F18)
        bit 31 set:   tile delta against frame i-1        (applied by 0x08007F48)
Delta record at blob + (offset & 0x7FFFFFFF):
  +0 u16 n (number of runs), +2 u16 (total tiles, unused by the game)
  +4 (u8 first_tile, u8 tile_count) * n
  then, 4-aligned at (2*n + 7) & ~3, the new tiles packed back to back (32 bytes each).
The game uploads frame i-1 raw and then overwrites the listed tiles (only odd frames are deltas).
Each Pokemon sprite has 6 frames (down, down step, side, side step, up, up step).
"""
import struct

MAGIC0, MAGIC1, TAG = 0x32504646, 0xCDAFB9B9, 0x0602


def is_ffp2(data, offset):
    if offset + 0x28 > len(data):
        return False
    m0, m1, csize, fsize, tag = struct.unpack_from('<IIIHH', data, offset)
    return (m0 == MAGIC0 and m1 == MAGIC1 and tag == TAG and fsize in (0x200, 0x800)
            and 0x28 <= csize <= 0x3028)


def blob_info(data, offset):
    """(frame_size, [6 raw offset words]) of the blob at offset."""
    fsize = struct.unpack_from('<H', data, offset + 0xC)[0]
    return fsize, list(struct.unpack_from('<6I', data, offset + 0x10))


def decode_frame(data, blob, index):
    """Frame `index` (0-5) of the blob starting at `blob`."""
    fsize, offs = blob_info(data, blob)
    ent = offs[index]
    pos = blob + (ent & 0x7FFFFFFF)
    if not ent & 0x80000000:
        return bytes(data[pos:pos + fsize])
    out = bytearray(decode_frame(data, blob, index - 1))  # base: the previous frame
    n = struct.unpack_from('<H', data, pos)[0]
    src = pos + ((2 * n + 7) & ~3)
    for i in range(n):
        tile, count = data[pos + 4 + 2 * i], data[pos + 5 + 2 * i]
        out[tile * 32:(tile + count) * 32] = data[src:src + count * 32]
        src += count * 32
    return bytes(out)


def decode(data, offset, index=0):
    """Raw 4bpp frame `index` of the blob at file offset `offset`.

    For a sprite's images array (6 SpriteFrameImage entries), offset is images[0].data
    and index is (images[i].data - images[0].data) // size, as the game computes it
    (0x08007EC4). Do not look up the blob from images[i].data alone: blobs are packed
    tightly, so blob + i*size can be the start of the next sprite's blob."""
    if not is_ffp2(data, offset):
        raise ValueError('no FFP2 blob at 0x%X' % offset)
    return decode_frame(data, offset, index)


def decode_images(data, images, count=6):
    """All frames of an images array at file offset `images` (pointers are ROM addresses)."""
    ptrs = [struct.unpack_from('<IH', data, images + 8 * i) for i in range(count)]
    base = ptrs[0][0]
    return [decode(data, base - 0x08000000, (p - base) // size) for p, size in ptrs]
