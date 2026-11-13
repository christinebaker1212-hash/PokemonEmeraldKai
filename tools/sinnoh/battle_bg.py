#!/usr/bin/env python3
"""SINNOH's battle backgrounds: Platinum's own (the background picture each map header names, its time of day, and
the two platforms the ground under the player gives), fitted to Kai's battle screen as Emerald's own backgrounds
are: a 4bpp background of at most 511 tiles and three 16-colour palettes (BG3, src/battle_bg.c), from pret/pokeplatinum
(fetch_decomp.sh PARTS 25; never committed): sinnoh/platinum_battle.pak, read by src/sinnoh/s3d_battlebg.c.

    python3 -I tools/sinnoh/battle_bg.py OUT.pak [DECOMP_DIR] [--png DIR]

Platinum (src/battle/battle_main.c, battle_system.c, terrain.c, src/field_battle_data_transfer.c):
  - the background: pl_batt_bg.narc, tiles member 3 + background (8 bits a pixel), the tilemap member 2 (512 x 256, two
    screen blocks, shared by all), the palette member 172 + background * 3 + the time (BattleSystem_GetBackgroundTimeOffset:
    the six outdoor backgrounds 0 morning / day, 1 twilight, 2 night / late night; the rest 0); the map header's
    battleBG, BACKGROUND_WATER while surfing (SetBackgroundAndTerrain)
  - the platforms (Terrain_LoadResources): res/graphics/battle/terrain/<name>/enemy.png (128 x 64, centred at 192, 88)
    and player.png (256 x 32, centred at 64, 136), palettes by the same time offset; which one: CalcTerrain, the tile's
    behaviour (ice, tall grass, sand, snow, mud, cave floor, water) or the background's own (sTerrainForBackground)
Kai's screen puts the foe's platform at 112-240 x 48-80: Platinum's (a 126 x 32 ellipse round 192, 88) lands there
moved by (-16, -24), the background with it; the player's platform by (0, -24), round Kai's 64.

The file is "S3DB" u32 version, u32 the size unpacked, then zlib:
  u16 maps (src/sinnoh/s3d_maps.h's order), each: u8 blocks wide, high; per block (row by row) u8 background, u8 the
      blocks's terrain (0xFF: one per tile follows, 1024 bytes; 0xFE: none, the background's own)
  u8 backgrounds (Platinum's 23): u8 its own terrain (sTerrainForBackground), u8 outdoor (3 palettes by the time)
  u16 scenes, each: u8 background, u8 terrain, u16 tiles, tiles x 32 bytes (4bpp), the map 64 x 16 u16 (tile | flips
      << 10 | palette 0-2 << 12: the scene, then the background going on), u8 palette sets (1 or 3: day, twilight,
      night), 48 u16 colours each (BGR555)
"""
import importlib.util
import os
import re
import struct
import sys
import zlib

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from tv_data import narc_members  # noqa: E402
import build_journal as bj  # noqa: E402
import build_contest as bc  # noqa: E402
from places import header_table  # noqa: E402
from source import open_source, NONE  # noqa: E402
from platinum import Land  # noqa: E402
_spec = importlib.util.spec_from_file_location('unova_battle_bg', os.path.join(os.path.dirname(HERE), 'unova',
                                                                               'battle_bg.py'))
ubb = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(ubb)   # (the quantizer to Kai's format, shared as a tool: never the regions' engines)

VERSION = 1
NARC = 'res/prebuilt/battle/graphic/pl_batt_bg.narc'
TERRAIN_DIR = 'res/graphics/battle/terrain'
OUT_W, OUT_H = 256, 128
SLIDE_SPLIT = 80
ENEMY_AT, PLAYER_AT = (192, 88), (64, 136)      # (battle_display.c: the platforms' places, Platinum's screen)
BG_SHIFT, ENEMY_SHIFT, PLAYER_SHIFT = (-16, -24), (-16, -24), (0, -24)
OUTDOOR = 6                                     # (BACKGROUND_PLAIN .. SNOW: palettes by the time)


def order(root, name):
    return [ln.strip() for ln in open(os.path.join(root, 'generated', name)) if ln.strip()]


def rgb(c):
    return ((c & 31), (c >> 5) & 31, (c >> 10) & 31)


class Platinum:
    def __init__(self, root):
        self.root = root
        self.m = narc_members(os.path.join(root, NARC))
        self.backgrounds = order(root, 'battle_backgrounds.txt')[:-1]
        self.terrains = order(root, 'battle_terrains.txt')[:-1]
        text = open(os.path.join(root, 'src/battle/terrain.c')).read()
        self.player_src = self._table(text, 'sTerrainSpriteSource_PlayerSide', r'terrain_(\w+)_player_NCGR_lz')
        self.enemy_src = self._table(text, 'sTerrainSpriteSource_EnemySide', r'terrain_(\w+)_enemy_NCGR_lz')
        self.pal_src = {}
        body = text[text.index('sTerrainPaletteSource'):]
        for t, a, b, c in re.findall(r'\[(TERRAIN_\w+)\]\s*=\s*\{\s*(\w+),\s*(\w+),\s*(\w+)\s*\}', body):
            self.pal_src[t] = [self._pal_file(x) for x in (a, b, c)]
        text = open(os.path.join(root, 'src/field_battle_data_transfer.c')).read()
        self.bg_terrain = dict(re.findall(r'\[(BACKGROUND_\w+)\] = (TERRAIN_\w+)', text))

    @staticmethod
    def _table(text, name, pat):
        body = text[text.index(name):]
        body = body[:body.index('};')]
        return {t: n for t, n in re.findall(r'\[(TERRAIN_\w+)\]\s*=\s*' + pat, body)}

    def _pal_file(self, sym):
        m = re.match(r'terrain_(\w+?)_(day|evening|night|all)_NCLR(?:_\d)?$', sym)
        return os.path.join(self.root, TERRAIN_DIR, m.group(1), m.group(2) + '.pal')

    def member(self, i):
        d = self.m[i]
        return bc.lz10(d) if d[0] == 0x10 else d

    def background(self, bg, time):
        """Platinum's background BG at the time (0-2): 512 x 256 RGB (0-31)"""
        s = bj.section_of(self.member(2), b'SCRN')
        w, h = struct.unpack_from('<HH', s, 8)
        size = struct.unpack_from('<I', s, 16)[0]
        ents = struct.unpack_from('<%dH' % (size // 2), s, 20)
        c = bj.section_of(self.member(3 + bg), b'CHAR')
        csize = struct.unpack_from('<I', c, 24)[0]
        raw = np.frombuffer(c[0x20:0x20 + csize], np.uint8).reshape(-1, 8, 8)
        pal = np.array([rgb(x) for x in bj.palettes(self.m[172 + bg * 3 + (time if bg < OUTDOOR else 0)])], np.float32)
        img = np.zeros((h, w, 3), np.float32)
        for ty in range(h // 8):
            for tx in range(w // 8):
                e = ents[(tx // 32) * 1024 + ty * 32 + tx % 32]
                n = e & 0x3FF
                if n >= len(raw):
                    continue
                t = raw[n]
                if e & 0x400:
                    t = t[:, ::-1]
                if e & 0x800:
                    t = t[::-1, :]
                img[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8] = pal[t]
        return img

    def platform(self, terrain, side, time):
        """(indices, palette RGB 0-31) of a platform"""
        from PIL import Image
        key = self.terrains[terrain]
        name = (self.enemy_src if side else self.player_src)[key]
        idx = np.array(Image.open(os.path.join(self.root, TERRAIN_DIR, name, ('enemy' if side else 'player') + '.png')))
        lines = open(self.pal_src[key][time]).read().split('\n')[3:]
        pal = np.array([[int(v) * 31 // 255 for v in ln.split()[:3]] for ln in lines if ln.strip()][:16], np.float32)
        return idx, pal

    def scene(self, bg, terrain, time):
        """The scene as Kai's background shows it: 512 x 128, the platforms in the first half"""
        b = self.background(bg, time if bg < OUTDOOR else 0)
        b = np.roll(b, (BG_SHIFT[1], BG_SHIFT[0]), axis=(0, 1))[:OUT_H]
        img = b.copy()
        for side, at, shift in ((1, ENEMY_AT, ENEMY_SHIFT), (0, PLAYER_AT, PLAYER_SHIFT)):
            idx, pal = self.platform(terrain, side, time if bg < OUTDOOR else 0)
            h, w = idx.shape
            x0, y0 = at[0] - w // 2 + shift[0], at[1] - h // 2 + shift[1]
            for y in range(h):
                for x in range(w):
                    if idx[y, x] and 0 <= y0 + y < OUT_H and 0 <= x0 + x < OUT_W:
                        img[y0 + y, x0 + x] = pal[idx[y, x]]
        return img


def behaviours(root):
    """Platinum's tile behaviour numbers by name, and the surfable ones"""
    text = open(os.path.join(root, 'include/constants/field/map_tile_behaviors.h')).read()
    body = text[text.index('enum TileBehavior'):]
    body = body[body.index('{') + 1:body.index('};')]
    num, n = {}, 0
    for ln in body.split(','):
        ln = ln.strip()
        if not ln:
            continue
        m = re.match(r'(TILE_BEHAVIOR_\w+)(?:\s*=\s*(\w+))?', ln)
        if m.group(2):
            n = int(m.group(2), 0)
        num[m.group(1)] = n
        n += 1
    flags = open(os.path.join(root, 'src/map_tile_behavior.c')).read()
    surf = {num[b] for b, f in re.findall(r'\[(TILE_BEHAVIOR_\w+)\]\s*=\s*(TILE_BEHAVIOR_FLAG_\w+)', flags)
            if 'SURFABLE' in f and b in num}
    return num, surf


def tile_terrain(b, num, surf, terrains):
    """CalcTerrain's tile part: the terrain a behaviour gives, None for the background's own"""
    t = None
    if b == num['TILE_BEHAVIOR_ICE']:
        t = 'TERRAIN_ICE'
    elif b in (num['TILE_BEHAVIOR_TALL_GRASS'], num['TILE_BEHAVIOR_VERY_TALL_GRASS']):
        t = 'TERRAIN_GRASS'
    elif b == num['TILE_BEHAVIOR_SAND']:
        t = 'TERRAIN_SAND'
    elif b in (num['TILE_BEHAVIOR_SNOW_DEEP'], num['TILE_BEHAVIOR_SNOW_DEEPER'], num['TILE_BEHAVIOR_SNOW_DEEPEST'],
               num['TILE_BEHAVIOR_SNOW_SHALLOW']):
        t = 'TERRAIN_SNOW'
    elif b in (num['TILE_BEHAVIOR_MUD'], num['TILE_BEHAVIOR_MUD_DEEP'], num['TILE_BEHAVIOR_MUD_WITH_GRASS'],
               num['TILE_BEHAVIOR_MUD_DEEP_WITH_GRASS']):
        t = 'TERRAIN_GREAT_MARSH'
    elif b == num['TILE_BEHAVIOR_CAVE_FLOOR']:
        t = 'TERRAIN_CAVE'
    elif b in surf:
        t = 'TERRAIN_WATER'
    return terrains.index(t) if t else None


def sinnoh_maps():
    """src/sinnoh/s3d_maps.h's maps: (MAP_SINNOH_x, matrix, cx, cz, bw, bh)"""
    text = open(os.path.join(HERE, '..', '..', 'src', 'sinnoh', 's3d_maps.h')).read()
    return [(m, int(a), int(b), int(c), int(d), int(e)) for m, a, b, c, d, e in
            re.findall(r'\{(MAP_SINNOH_\w+), (\d+), (\d+), (\d+), (\d+), (\d+)\}', text)]


def ident(header):
    return 'MAP_SINNOH_' + re.sub(r'_(\d)', r'\1', header[len('MAP_HEADER_'):])


def build(root, png_dir=None):
    pt = Platinum(root)
    src = open_source(root)
    table = header_table(root)
    by_id = {ident(h): h for h in table}
    num, surf = behaviours(root)
    bgs, ters = pt.backgrounds, pt.terrains
    used = set()
    maps_out = bytearray()
    smaps = sinnoh_maps()
    maps_out += struct.pack('<H', len(smaps))
    for mid, mat, cx, cz, bw, bh in smaps:
        header = by_id.get(mid)
        m = src.matrix(mat)
        maps_out += struct.pack('<BB', bw, bh)
        for z in range(cz, cz + bh):
            for x in range(cx, cx + bw):
                lid, h, _ = m.cell(x, z) if x < m.width and z < m.height else (NONE, None, 0)
                h = h if h and h in table else header
                bgname = table.get(h, {}).get('battleBG', 'BACKGROUND_PLAIN')
                bg = bgs.index(bgname) if bgname in bgs else 0
                tts = [None] * 1024
                if lid != NONE:
                    land = Land(src.land(lid))
                    tts = [tile_terrain(a & 0xFF, num, surf, ters) for a in land.attrs]
                own = ters.index(pt.bg_terrain[bgs[bg]])
                for t in set(tts):
                    used.add((bg, own if t is None else t))
                used.add((bg, own))
                used.add((bgs.index('BACKGROUND_WATER'), ters.index('TERRAIN_WATER')))   # (surfing)
                maps_out += struct.pack('<B', bg)
                if len(set(tts)) == 1:
                    maps_out += bytes([0xFE if tts[0] is None else tts[0]])
                else:
                    maps_out += bytes([0xFF]) + bytes(0xFE if t is None else t for t in tts)
    out = bytearray(maps_out)
    out += struct.pack('<B', len(bgs))
    for b in bgs:
        out += struct.pack('<BB', ters.index(pt.bg_terrain[b]), 1 if bgs.index(b) < OUTDOOR else 0)
    # (the special battles' own: GIRATINA's in the DISTORTION WORLD, the facilities' and the League's set with them)
    used.add((bgs.index('BACKGROUND_DISTORTION_WORLD'), ters.index('TERRAIN_GIRATINA')))
    for b in bgs:
        used.add((bgs.index(b), ters.index(pt.bg_terrain[b])))
    scenes = sorted(used)
    out += struct.pack('<H', len(scenes))
    for n, (bg, ter) in enumerate(scenes):
        times = 3 if bg < OUTDOOR else 1
        imgs = []
        for t in range(times):
            sc = pt.scene(bg, ter, t)
            # (the second half: the background going on, as Kai's own backgrounds keep it free of platforms; Platinum's
            # pictures are 512 wide and go round, so every edge the intro's slide crosses is its own)
            rest = np.roll(pt.background(bg, t if bg < OUTDOOR else 0), (BG_SHIFT[1], BG_SHIFT[0]), axis=(0, 1))
            imgs.append(np.concatenate([sc[:, :OUT_W], rest[:OUT_H, OUT_W:2 * OUT_W]], axis=1))
        tiles, tmap, pal, alt = quantize_times(imgs)
        out += struct.pack('<BBH', bg, ter, len(tiles))
        for t in tiles:
            for row in t:
                out += bytes((int(row[x]) | (int(row[x + 1]) << 4)) for x in range(0, 8, 2))
        out += struct.pack('<%dH' % len(tmap), *tmap)
        out += struct.pack('<B', times)
        for p in [pal] + alt:
            out += struct.pack('<48H', *[int(r) | (int(g) << 5) | (int(b) << 10) for r, g, b in p])
        print('scene %d: %s %s, %d tiles, %d palette sets' % (n, bgs[bg], ters[ter], len(tiles), times), flush=True)
        if png_dir:
            from PIL import Image
            os.makedirs(png_dir, exist_ok=True)
            for k, p in enumerate([pal] + alt):
                Image.fromarray(ubb.picture(tiles, tmap, p)).save(
                    os.path.join(png_dir, 'scene_%02d_%s_%s_%d.png' % (n, bgs[bg][11:].lower(), ters[ter][8:].lower(), k)))
    return b'S3DB' + struct.pack('<II', VERSION, len(out)) + zlib.compress(bytes(out), 9)


def quantize_times(imgs):
    """Kai's tiles and palettes of the day's picture, and the twilight's and night's palettes for the same tiles: each
    colour the mean of the pixels it draws at that time"""
    tiles, tmap, pal = ubb.quantize(imgs[0])
    alt = []
    if len(imgs) > 1:
        th, tw = OUT_H // 8, len(tmap) // (OUT_H // 8)
        slot = np.zeros((OUT_H, tw * 8), np.int64)
        for i, e in enumerate(tmap):
            t = tiles[e & 0x3FF]
            if e & 0x400:
                t = t[:, ::-1]
            if e & 0x800:
                t = t[::-1, :]
            y, x = (i // tw) * 8, (i % tw) * 8
            slot[y:y + 8, x:x + 8] = ((e >> 12) & 3) * 16 + t
        for img in imgs[1:]:
            p = pal.copy()
            for k in range(48):
                m = slot == k
                if m.any():
                    p[k] = np.clip(np.rint(img[m].mean(0)), 0, 31)
            alt.append(p)
    return tiles, tmap, pal, alt


def main():
    args = [a for a in sys.argv[1:]]
    png = None
    if '--png' in args:
        png = args[args.index('--png') + 1]
        del args[args.index('--png'):args.index('--png') + 2]
    out = args[0]
    root = args[1] if len(args) > 1 else os.path.expanduser('~/.cache/moemon/pokeplatinum')
    data = build(root, png)
    open(out, 'wb').write(data)
    print('%s: %d bytes' % (out, len(data)))


if __name__ == '__main__':
    main()
