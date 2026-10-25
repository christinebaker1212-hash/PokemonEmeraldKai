#!/usr/bin/env python3
"""MOeMON GARDEN tileset: data/tilesets/secondary/moemon_garden (gTileset_MoemonGarden).

The Garden is the POKEMON CENTER's second floor (a meadow inside it), so it needs the CENTER's escalator next to
Petalburg's pieces. This is Petalburg's secondary tileset (its metatiles keep their ids 0x200-0x28F, its tiles and
palettes their places) plus, from the POKEMON CENTER's tilesets (Building primary + PokemonCenter secondary):
  - the 2F escalator at its own ids: 0x298/0x299 (top) and 0x2A0-0x2AD (the animation frames that
    fldeff_escalator.c swaps by id: METATILE_PokemonCenter_Escalator2F_*), so riding it works as in a CENTER;
  - the CENTER's floor and its west wall pieces, at 0x2B0 and up (GARDEN_MT_* in gen_map.py).
Their tiles are copied after Petalburg's, recolored into palette slot 7 (free in Petalburg; they use 14 colors).
"""
import os, struct
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..', '..')
TS = os.path.join(ROOT, 'data/tilesets')
OUT = os.path.join(TS, 'secondary/moemon_garden')
PAL_SLOT = 7

def metas(path):
    d = open(path, 'rb').read()
    return [list(struct.unpack('<8H', d[i:i + 16])) for i in range(0, len(d), 16)]

def attrs(path):
    d = open(path, 'rb').read()
    return list(struct.unpack('<%dH' % (len(d) // 2), d))

def pal(path):
    lines = open(path).read().split('\n')[3:19]
    return [tuple(int(v) for v in l.split()[:3]) for l in lines if l.strip()]

def tiles(path):
    im = Image.open(path)
    px = im.load()
    w, h = im.size
    return [[[px[tx * 8 + x, ty * 8 + y] & 15 for x in range(8)] for y in range(8)]
            for ty in range(h // 8) for tx in range(w // 8)]

pb_dir, pc_dir, bd_dir = (os.path.join(TS, p) for p in ('secondary/petalburg', 'secondary/pokemon_center', 'primary/building'))
pb_metas, pb_attrs, pb_tiles = metas(pb_dir + '/metatiles.bin'), attrs(pb_dir + '/metatile_attributes.bin'), tiles(pb_dir + '/tiles.png')
pc_metas, pc_attrs, pc_tiles = metas(pc_dir + '/metatiles.bin'), attrs(pc_dir + '/metatile_attributes.bin'), tiles(pc_dir + '/tiles.png')
bd_tiles = tiles(bd_dir + '/tiles.png')
pb_pals = [pal(pb_dir + '/palettes/%02d.pal' % i) for i in range(16)]
pc_pals = [pal(pc_dir + '/palettes/%02d.pal' % i) for i in range(16)]
bd_pals = [pal(bd_dir + '/palettes/%02d.pal' % i) for i in range(16)]

# Petalburg must leave the slot and the escalator's ids free
assert all((e >> 12) != PAL_SLOT for m in pb_metas for e in m)
assert len(pb_metas) <= 0x98

# what to bring: (our id, the CENTER's id)
IMPORT = [(0x298, 0x298), (0x299, 0x299)] + [(i, i) for i in range(0x2A0, 0x2A6)] + [(i, i) for i in range(0x2A8, 0x2AE)]
IMPORT += [(0x2B0, 0x202),   # floor
           (0x2B1, 0x220),   # west wall, beside the floor
           (0x2B2, 0x228)]   # west wall, its bottom corner

out_tiles = [t for t in pb_tiles]
while len(out_tiles) < 160:
    out_tiles.append([[0] * 8 for _ in range(8)])
new_pal = [pb_pals[PAL_SLOT][0]]   # (index 0: transparent)
tile_map = {}

def color_index(rgb):
    if rgb not in new_pal:
        new_pal.append(rgb)
        assert len(new_pal) <= 16, 'too many colors for one palette'
    return new_pal.index(rgb)

def bring_tile(entry):
    t, p = entry & 0x3FF, entry >> 12
    src = bd_tiles[t] if t < 512 else pc_tiles[t - 512]
    colors = bd_pals[p] if p < 6 else pc_pals[p]
    key = (t, p)
    if key not in tile_map:
        img = [[(color_index(colors[c]) if c else 0) for c in row] for row in src]
        tile_map[key] = len(out_tiles)
        out_tiles.append(img)
    return (512 + tile_map[key]) | (entry & 0x0C00) | (PAL_SLOT << 12)

out_metas = [m for m in pb_metas]
out_attrs = [a for a in pb_attrs]
top = max(i for i, _ in IMPORT) - 0x200 + 1
while len(out_metas) < top:
    out_metas.append([0] * 8)
    out_attrs.append(0)
for ours, theirs in IMPORT:
    out_metas[ours - 0x200] = [bring_tile(e) for e in pc_metas[theirs - 0x200]]
    out_attrs[ours - 0x200] = pc_attrs[theirs - 0x200]

assert len(out_tiles) <= 512

os.makedirs(os.path.join(OUT, 'palettes'), exist_ok=True)
with open(os.path.join(OUT, 'metatiles.bin'), 'wb') as f:
    for m in out_metas:
        f.write(struct.pack('<8H', *m))
with open(os.path.join(OUT, 'metatile_attributes.bin'), 'wb') as f:
    f.write(struct.pack('<%dH' % len(out_attrs), *out_attrs))
pals = [list(p) for p in pb_pals]
pals[PAL_SLOT] = new_pal + [(0, 0, 0)] * (16 - len(new_pal))
for i, p in enumerate(pals):
    with open(os.path.join(OUT, 'palettes/%02d.pal' % i), 'w') as f:
        f.write('JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in p))
# tiles.png: 16 tiles wide, indexed (the pixel values are what counts; the PNG's palette is the new one)
rows = (len(out_tiles) + 15) // 16
im = Image.new('P', (128, rows * 8))
flat = []
for c in pals[PAL_SLOT]:
    flat += list(c)
im.putpalette(flat + [0] * (768 - len(flat)))
px = im.load()
for i, t in enumerate(out_tiles):
    for y in range(8):
        for x in range(8):
            px[(i % 16) * 8 + x, (i // 16) * 8 + y] = t[y][x]
im.save(os.path.join(OUT, 'tiles.png'))
print('moemon_garden: %d metatiles, %d tiles, %d colors in palette %d' % (len(out_metas), len(out_tiles), len(new_pal) - 1, PAL_SLOT))
