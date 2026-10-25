"""Draw a ripped map from the ROM (layout + tilesets) to a PNG, to check the rip by eye.

Quetzal splits tile / metatile ids at the primary tileset's size: 512 for Emerald primaries,
640 for the FRLG-style primaries Kanto and Johto use.
"""
from PIL import Image

from gfx import lz77, gba_colors
from tilesets import parse_tileset


class TilesetCache:
    def __init__(self, rom):
        self.rom = rom
        self.cache = {}

    def get(self, a):
        if a not in self.cache:
            t = parse_tileset(self.rom, a)
            tiles = lz77(self.rom.d, t['tiles'])[0] if t['compressed'] else self.rom.d[t['tiles']:t['tiles'] + 0x8000]
            t['tile_data'] = tiles
            t['pals'] = [gba_colors(self.rom.d, t['palettes'] + 32 * i) for i in range(16)]
            self.cache[a] = t
        return self.cache[a]


def split_of(prim):
    return 640 if len(prim['tile_data']) // 32 > 512 else 512


def render_layout(rom, L, cache, scale=1):
    prim = cache.get(L['primary'])
    sec = cache.get(L['secondary'])
    split = split_of(prim)
    w, h = L['width'], L['height']
    img = Image.new('RGB', (w * 16, h * 16))
    px = img.load()
    pals = prim['pals'][:7] + sec['pals'][7:] if split == 640 else prim['pals'][:6] + sec['pals'][6:]
    for my in range(h):
        for mx in range(w):
            v = rom.u16(L['blockdata'] + 2 * (my * w + mx))
            mid = v & 0x3FF
            ts = prim if mid < split else sec
            mo = ts['metatiles'] + 16 * (mid if mid < split else mid - split)
            for layer in range(2):
                for q in range(4):
                    e = rom.u16(mo + 8 * layer + 2 * q)
                    tid, hf, vf, pal = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
                    tdat = prim['tile_data'] if tid < split else sec['tile_data']
                    tb = (tid if tid < split else tid - split) * 32
                    if tb + 32 > len(tdat):
                        continue
                    colors = pals[pal]
                    ox, oy = mx * 16 + (q & 1) * 8, my * 16 + (q >> 1) * 8
                    for y in range(8):
                        for x in range(8):
                            b = tdat[tb + y * 4 + x // 2]
                            c = (b >> 4) if x & 1 else (b & 15)
                            if c == 0 and layer == 1:
                                continue
                            if c == 0 and layer == 0:
                                c = 0
                            sx = 7 - x if hf else x
                            sy = 7 - y if vf else y
                            px[ox + sx, oy + sy] = colors[c]
    if scale != 1:
        img = img.resize((img.width * scale, img.height * scale), Image.NEAREST)
    return img
