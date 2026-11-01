"""The PÓKETCH's art for sinnoh/platinum_gfx.pak (build_gfx.py appends it): Platinum's own app screens and sprites from
pret/pokeplatinum's res/graphics/poketch (never committed), as LCD shades the game colors at draw time
(src/sinnoh/poketch.c; the COLOR CHANGER's colors).

Section "PKCH": u16 screens, each name[16] + 192x160 shade bytes (the app area of Platinum's 256x192 screen, from
(16, 16)); u16 sheets, each name[16] + u16 cells, each s16 x, s16 y (its top-left from the sprite's point), u16 w, h,
w*h shade bytes. Shades: 0 the lightest .. 3 the darkest, 255 nothing (a sprite's clear pixels).
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import png8  # noqa: E402

SHADE = {4: 0, 15: 1, 8: 2, 1: 3}   # (Platinum's LCD colors in its 16-color palettes, lightest first)
# app screens: (name, tilemap, tiles); the others are the tilemap's name with _bg_tiles
SCREENS = ['alarm_clock', 'analog_watch', 'backlight_toggle', 'berry_searcher', 'calculator', 'calendar', 'coin_toss',
           'color_changer', 'counter', 'daycare_checker', 'digital_watch', 'dowsing_machine', 'kitchen_timer',
           'link_searcher', 'marking_map', 'matchup_checker', 'memo_pad', 'move_tester', 'pedometer', 'roulette',
           'stopwatch', 'trainer_counter']
TILES = {'analog_watch': 'watch_bg_tiles', 'digital_watch': 'watch_bg_tiles', 'berry_searcher': 'map_bg_tiles',
         'marking_map': 'map_bg_tiles'}
SHEETS = ['alarm_clock', 'analog_watch', 'backlight_toggle', 'calendar', 'coin_toss', 'color_changer', 'counter',
          'daycare_checker', 'digits', 'dowsing_machine', 'friendship_checker', 'kitchen_timer', 'link_searcher', 'map',
          'matchup_checker', 'memo_pad', 'move_tester', 'party_status', 'pedometer', 'roulette', 'trainer_counter']
OBJ = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
       (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}


def shade(v, clear):
    if v == 0:
        return 255 if clear else 0
    return SHADE.get(v, 0)


def screen(D, name, tiles):
    t = png8.Image(D + tiles + '.png')
    b = open(D + name + '.NSCR', 'rb').read()
    mw, mh = struct.unpack('<HH', b[0x18:0x1C])
    mw, mh = mw // 8, mh // 8
    e = struct.unpack('<%dH' % (mw * mh), b[0x24:0x24 + mw * mh * 2])
    tw, count = t.w // 8, (t.w // 8) * (t.h // 8)
    out = bytearray(192 * 160)
    for y in range(160):
        for x in range(192):
            sx, sy = x + 16, y + 16
            v = e[(sy // 8) * mw + sx // 8]
            n, hf, vf = v & 0x3FF, v >> 10 & 1, v >> 11 & 1
            if n >= count:
                continue
            px, py = sx % 8, sy % 8
            c = t.px((n % tw) * 8 + (7 - px if hf else px), (n // tw) * 8 + (7 - py if vf else py))
            out[y * 192 + x] = shade(c, False)
    return bytes(out)


def cells(D, name):
    """[(x, y, w, h, shade bytes)] of each cell of the sheet (its OAM pieces drawn together)"""
    t = png8.Image(D + name + '.png')
    tw, count = t.w // 8, (t.w // 8) * (t.h // 8)
    cj = json.load(open(D + name + '_cell.json'))
    unit = 1 << cj.get('mappingType', 0)
    out = []
    for c in cj['cells']:
        pieces = []
        for o in c['OAM']:
            a0, a1, a2 = o['Attr0'], o['Attr1'], o['Attr2']
            w, h = OBJ[(a0['Shape'], a1['Size'])]
            x, y = a1['XCoordinate'], a0['YCoordinate']
            x = x - 512 if x >= 256 else x
            y = y - 256 if y >= 128 else y
            flips = 0 if a0.get('Rotation') else a1.get('RotationScaling', 0)
            pieces.append((x, y, w, h, a2['CharName'] * unit, flips & 8, flips & 16))
        if not pieces:
            out.append((0, 0, 1, 1, b'\xff'))
            continue
        x0, y0 = min(p[0] for p in pieces), min(p[1] for p in pieces)
        x1, y1 = max(p[0] + p[2] for p in pieces), max(p[1] + p[3] for p in pieces)
        W, H = x1 - x0, y1 - y0
        px = bytearray([255]) * (W * H)
        for x, y, w, h, tile, hf, vf in pieces:
            for oy in range(h):
                for ox in range(w):
                    sx, sy = (w - 1 - ox if hf else ox), (h - 1 - oy if vf else oy)
                    n = tile + (sy // 8) * (w // 8) + sx // 8
                    if n >= count:
                        continue
                    v = t.px((n % tw) * 8 + sx % 8, (n // tw) * 8 + sy % 8)
                    if v:
                        px[(y - y0 + oy) * W + (x - x0 + ox)] = shade(v, True)
        out.append((x0, y0, W, H, bytes(px)))
    return out


def big_digits(D):
    """The DIGITAL WATCH's digits: its digits screen (10 digits, 32x72 each) as 10 cells"""
    b = open(D + 'digital_watch_digits.NSCR', 'rb').read()
    mw, mh = struct.unpack('<HH', b[0x18:0x1C])
    mw, mh = mw // 8, mh // 8
    e = struct.unpack('<%dH' % (mw * mh), b[0x24:0x24 + mw * mh * 2])
    out = []
    for d in range(10):
        px = bytearray(32 * 72)
        for y in range(72):
            for x in range(32):
                tx, ty = (d * 32 + x) // 8, y // 8
                # (a screen past 256 pixels is kept in 32-tile-wide blocks; tile 2 is a lit block, tile 1 a clear one)
                v = e[ty * 32 + tx] if tx < 32 else e[32 * mh + ty * (mw - 32) + tx - 32]
                px[y * 32 + x] = 2 if v & 0x3FF == 2 else 255
        out.append((0, 0, 32, 72, bytes(px)))
    return out


def section(root):
    D = os.path.join(root, 'res/graphics/poketch/')
    out = bytearray(b'PKCH')
    out += struct.pack('<H', len(SCREENS))
    for s in SCREENS:
        out += s.encode()[:16].ljust(16, b'\0') + screen(D, s, TILES.get(s, s + '_bg_tiles'))
    out += struct.pack('<H', len(SHEETS) + 1)
    for s in SHEETS + ['bigdigits']:
        cs = big_digits(D) if s == 'bigdigits' else cells(D, s)
        out += s.encode()[:16].ljust(16, b'\0') + struct.pack('<H', len(cs))
        for x, y, w, h, px in cs:
            out += struct.pack('<hhHH', x, y, w, h) + px
    return bytes(out)


if __name__ == '__main__':
    # (a look at the art: python3 tools/sinnoh/build_poketch.py POKEPLATINUM_DIR OUT_DIR writes each screen as a PGM)
    root, outdir = sys.argv[1], sys.argv[2]
    D = os.path.join(root, 'res/graphics/poketch/')
    os.makedirs(outdir, exist_ok=True)
    gray = [230, 160, 90, 20]
    for s in SCREENS:
        px = screen(D, s, TILES.get(s, s + '_bg_tiles'))
        open(os.path.join(outdir, s + '.pgm'), 'wb').write(b'P5 192 160 255\n' + bytes(gray[v] for v in px))
    for s in SHEETS:
        cs = cells(D, s)
        print(s, len(cs), [(x, y, w, h) for x, y, w, h, _ in cs][:12])
