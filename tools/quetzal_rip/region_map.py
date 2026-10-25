#!/usr/bin/env python3
"""Johto and Kanto region maps, fly destinations and heal locations from Moemon Quetzal.

    python3 tools/quetzal_rip/region_map.py QUETZAL.gba [OUTDIR]     (default quetzal_rip/region_map)

How Quetzal does it (all addresses ROM; found from the region map code around 0x08194C00-0x08199000):
- LoadRegionMapGfx (0x08195244, also 0x08195170) picks tiles / tilemap / palette by region:
    Hoenn  0x09225FC0 / 0x09226D24 / 0x09225F80  (stock)
    Johto  0x092270B0 / 0x09227B4C / 0x09227070
    Kanto  tiles 0x0922B010[sub] / tilemap 0x0922B020[sub] / palette 0x09227EC8, where sub = the byte at
           0x030012C5 (0 Kanto, 1-3 the Sevii Island groups, FRLG style; 0x08195134 cycles it).
  Same format as Emerald's region map: an 8bpp affine background (LZ77 tiles, LZ77 64x64 one-byte tilemap),
  palette loaded at BG palette 7 so pixel values are 112+ (Kai's graphics/pokenav/region_map/map.png keeps them).
  Johto's / Kanto's palettes are 32 colors (Emerald's 48).
- Section grid (which map section each 8x8 cell of the map is): u8[15][28] mapsec ids, 0xD5 = none
  (0x08194D54): Hoenn 0x0922FE3C, Johto 0x0922FFE0, Kanto 0x0922B030 + sub * 420.
- Section entries {x, y, w, h, names**}: 0x09232A00, Johto 0x09233C94 (0x08194D34).
- Kanto sections -> sub map: u8 table 0x0922B6C0[mapsec] (0x08194CB4; sections 0xC8 / 0xD2 / 0xD3 alias
  0xBB / 0xB7 / 0xAE first, 0x08194C8C).
- Fly destinations (SetFlyDestination 0x08194F3C): Johto table 0x092347AC (14 entries), Kanto 0x0923470C
  (20), each {u16 mapsec, u16 heal location, u16 unlock flag, u16 pad}; the flag must be set (0x08194EC8 /
  0x08194DE0). Johto also tests VAR 0x5008 == 3 for Violet (mapsec 2) and VAR 0x500D == 1 for Olivine (mapsec 6)
  (0x08194E84); Kanto's Pallet (mapsec 0x58) is always allowed. Hoenn uses the stock table 0x09234674.
- Heal locations (GetHealLocation 0x08194C0C): 0x09225C60, 8-byte entries {s8 group, s8 num, s16 x, s16 y,
  pad}, ids 1-57: 0x17-0x25 Johto, 0x26-0x39 Kanto. No per-heal-location whiteout / Pokémon Center table was
  found (Emerald's sWhiteoutRespawnHealCenterMapIdxs).
"""
import json
import os
import sys

from qrom import Rom, REPO, decode, MAPSEC_MAIN, MAPSEC_JOHTO
from gfx import lz77, gba_colors, write_jasc

try:
    from PIL import Image
except ImportError:
    Image = None

GFX = {'johto': (0x12270B0, 0x1227B4C, 0x1227070)}
KANTO_TILES, KANTO_TILEMAPS, KANTO_PAL = 0x122B010, 0x122B020, 0x1227EC8
GRID_JOHTO, GRID_KANTO, GRID_SIZE = 0x122FFE0, 0x122B030, 15 * 28
KANTO_SUBMAP = 0x122B6C0
FLY = {'johto': (0x12347AC, 14), 'kanto': (0x123470C, 20)}
HEAL = 0x1225C60
HEAL_IDS = {'johto': range(0x17, 0x26), 'kanto': range(0x26, 0x3A)}
SUBMAP_NAMES = ['Kanto', 'Sevii 1-3', 'Sevii 4-5', 'Sevii 6-7']
PAL_BASE = 112          # BG palette 7


def mapsec_name(rom, region, sec):
    t = (MAPSEC_JOHTO if region == 'johto' else MAPSEC_MAIN) + 8 * sec
    p = rom.ptr(t + 4)
    q = rom.ptr(p) if p is not None else None
    return decode(rom, q, 40)[0] if q is not None else None


def write_map(rom, out, name, tiles_off, map_off, pal_off, ncolors=32):
    d = os.path.join(out, name)
    os.makedirs(d, exist_ok=True)
    tiles = lz77(rom.d, tiles_off)[0]
    tilemap = lz77(rom.d, map_off)[0]
    colors = gba_colors(rom.d, pal_off, ncolors)
    write_jasc(os.path.join(d, 'map.pal'), colors)
    open(os.path.join(d, 'map.bin'), 'wb').write(tilemap)
    if Image is None:
        return
    pal = [0, 0, 0] * PAL_BASE + [c for rgb in colors for c in rgb]
    pal += [0] * (768 - len(pal))
    n = len(tiles) // 64
    sheet = Image.new('P', (128, ((n + 15) // 16) * 8))
    sheet.putpalette(pal)
    px = sheet.load()
    for t in range(n):
        for y in range(8):
            for x in range(8):
                px[(t % 16) * 8 + x, (t // 16) * 8 + y] = tiles[t * 64 + y * 8 + x]
    sheet.save(os.path.join(d, 'map.png'))          # like Kai's graphics/pokenav/region_map/map.png
    view = Image.new('P', (512, 512))
    view.putpalette(pal)
    vp = view.load()
    for ty in range(64):
        for tx in range(64):
            t = tilemap[ty * 64 + tx]
            for y in range(8):
                for x in range(8):
                    vp[tx * 8 + x, ty * 8 + y] = tiles[t * 64 + y * 8 + x]
    view.convert('RGB').save(os.path.join(d, 'preview.png'))


def grid(rom, region, off):
    rows = [list(rom.d[off + 28 * r: off + 28 * r + 28]) for r in range(15)]
    secs = sorted({s for row in rows for s in row if s != 0xD5})
    return {'rows': rows, 'sections': {s: mapsec_name(rom, region, s) for s in secs}}


def main():
    rom = Rom(sys.argv[1])
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(REPO, 'quetzal_rip', 'region_map')
    os.makedirs(out, exist_ok=True)
    index = {}
    p = os.path.join(REPO, 'quetzal_rip', 'maps', 'index.json')
    if os.path.exists(p):
        index = {(m['group'], m['num']): m['name'] for m in json.load(open(p))['maps']}
    layouts = {}
    write_map(rom, out, 'johto', *GFX['johto'])
    layouts['johto'] = grid(rom, 'johto', GRID_JOHTO)
    for sub in range(4):
        nm = 'kanto' if sub == 0 else 'kanto_sevii%d' % sub
        write_map(rom, out, nm, rom.ptr(KANTO_TILES + 4 * sub), rom.ptr(KANTO_TILEMAPS + 4 * sub), KANTO_PAL)
        layouts[nm] = grid(rom, 'kanto', GRID_KANTO + GRID_SIZE * sub)
        layouts[nm]['submap'] = SUBMAP_NAMES[sub]
    with open(os.path.join(out, 'layouts.json'), 'w', encoding='utf-8') as fh:
        json.dump({'_note': 'u8[15][28] map section ids per region map cell (0xD5 = none), as '
                            'src/data/region_map/region_map_layout.h', 'layouts': layouts},
                  fh, indent=1, ensure_ascii=False)
        fh.write('\n')
    heal = {}
    for region, ids in HEAL_IDS.items():
        heal[region] = []
        for i in ids:
            o = HEAL + 8 * i
            g, n = rom.u8(o), rom.u8(o + 1)
            heal[region].append({'heal_location': i, 'map': index.get((g, n)), 'group': g, 'num': n,
                                 'x': rom.s16(o + 2), 'y': rom.s16(o + 4)})
    frlg = {}
    p = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'kanto_frlg_flags.json')
    if os.path.exists(p):
        frlg = json.load(open(p)).get('flags', {})
    fly = {}
    for region, (base, n) in FLY.items():
        fly[region] = []
        for i in range(n):
            o = base + 8 * i
            sec = rom.u16(o)
            e = {'mapsec': sec, 'name': mapsec_name(rom, region, sec), 'heal_location': rom.u16(o + 2),
                 'unlock_flag': 'FLAG_QZ_0x%X' % rom.u16(o + 4)}
            if region == 'kanto':
                e['frlg_flag'] = frlg.get(e['unlock_flag'])
                e['submap'] = SUBMAP_NAMES[rom.u8(KANTO_SUBMAP + sec)] if sec < 0xDC else None
            fly[region].append(e)
    fly['johto_extra_checks'] = {'2 (Violet City)': 'also checks VAR_QZ_0x5008 == 3 (0x08194E84; which way is not traced)',
                                 '6 (Olivine City)': 'also checks VAR_QZ_0x500D == 1 (0x08194E84; which way is not traced)'}
    fly['kanto_always_open'] = ['0x58 PALLET TOWN']
    with open(os.path.join(out, 'fly_heal.json'), 'w', encoding='utf-8') as fh:
        json.dump({'heal_locations': heal, 'fly_destinations': fly}, fh, indent=1, ensure_ascii=False)
        fh.write('\n')
    print('region maps: johto + 4 kanto, %d heal locations, %d fly spots' % (
        sum(len(v) for v in heal.values()), len(fly['johto']) + len(fly['kanto'])))


if __name__ == '__main__':
    main()
