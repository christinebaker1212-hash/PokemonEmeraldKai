#!/usr/bin/env python3
"""Pack graphics/tcg/original/NNN.png (the Game Boy card illustrations) into
graphics/tcg/original.bin, built into the game (src/tcg/tcg_art_data.c).

Format: "TCGA", u32 count, then count x (u32 offset, u32 size) little-endian,
offsets from the start of the file; an entry of size 0 has no picture.
Card N is entry N - 1."""
from pathlib import Path
import struct
ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / 'graphics/tcg/original'
COUNT = 228
blobs = [(SRC / f'{n:03d}.png').read_bytes() if (SRC / f'{n:03d}.png').exists() else b'' for n in range(1, COUNT + 1)]
head = 8 + 8 * COUNT
out = bytearray(b'TCGA' + struct.pack('<I', COUNT))
offset = head
for blob in blobs:
    out += struct.pack('<II', offset if blob else 0, len(blob))
    offset += len(blob)
for blob in blobs:
    out += blob
(ROOT / 'graphics/tcg/original.bin').write_bytes(out)
print(f'graphics/tcg/original.bin: {sum(1 for b in blobs if b)} pictures, {len(out)} bytes')
