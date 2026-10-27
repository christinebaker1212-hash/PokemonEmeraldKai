#!/usr/bin/env python3
"""The GAME CENTER: its pictures and its floors' layouts.

The building and its four floors are the pictures in graphics/moemon/gamecenter/*.png, drawn in full color over the
map by src/mp/mp_mappic.c (not cut into the GBA's tiles and palettes). The maps underneath only carry what the game
needs to walk around: collision, the exits, the stairs and the counters, from the grids below, on the Lilycove
department store's tilesets (Building + Shop) so each cell has the right behavior. What the maps' tiles look like
never shows.

Writes graphics/moemon/gamecenter/<name>.bin (RGB555 little-endian, bit 15 = drawn) and
data/layouts/GameCenter_<floor>/{map,border}.bin.
usage: python3 tools/moemon_gamecenter/gen_gamecenter.py
"""
import os, struct
from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
GFX = os.path.join(ROOT, 'graphics/moemon/gamecenter')

# Cells: '.' floor, '#' blocked, 'W' an exit (walk down off the mat), 'S' stairs (a step onto them; the tile below
# must be floor: arriving there, the player steps down), 'C' a counter (talk across it)
FLOORS = {
    '1F': [
        '#############',
        '#############',
        '###S......###',
        '###.......###',
        '####......###',
        '...........##',
        '#..........##',
        '#............',
        '...#######...',
        '#..#######..#',
        '#..#######..#',
        '...###C###...',
        '#...........#',
        '#...........#',
        '..W.......W..',
        '#############',
        '#############',
    ],
    '2F': [
        '#############',
        '#############',
        '##S.......S##',
        '##.........##',
        '###.......###',
        '###.......###',
        '##C.........#',
        '##C.........#',
        '###....####..',
        '#C#....####..',
        '.............',
        '.###...##.##.',
        '.......##.##.',
        '.###...##.##.',
        '.......##.##.',
        '#############',
        '#############',
    ],
    '3F': [
        '#############',
        '#############',
        '##S.......S##',
        '##.........##',
        '###.......###',
        '..........###',
        '..........###',
        '...##........',
        '...##...##C##',
        '...##...#####',
        '...##...#####',
        '...##...#####',
        '...##........',
        '#...........#',
        '#...........#',
        '#############',
        '#############',
    ],
    'Rooftop': [
        '##################',
        '##################',
        '##################',
        '#........#########',
        '#..###.........###',
        '#.#####........S##',
        '#.#####.........##',
        '#..............###',
        '##################',
        '##################',
        '##################',
        '##################',
        '##################',
        '##################',
        '##################',
        '##################',
    ],
}
PICTURES = {'1F': '1f', '2F': '2f', '3F': '3f', 'Rooftop': 'rooftop'}

# What is drawn over the sprites (<name>_over.bin), as the GBA's furniture does with its top layer:
# - anything standing on the floor (a blocked cell that isn't wall or black) whose top rises into the walkable cell
#   north of it: going up each column from that cell's bottom edge, the pixels until the floor shows (someone
#   standing behind a display case, a chair or a counter has their feet behind it);
# - OVER: a counter's back edge in the row of whoever stands behind it, (x0, y0, x1, y1) in pixels, and a part of
#   the floor nearby whose colors are left out (the floor in that row stays under the sprites).
# A cell's floor: the colors of its own top half (rugs and mats too; what rises from below only reaches a cell's
# bottom), and the room's warm floor colors (the tiles' grout and shading, from the open floor; furniture's whites
# and greys are left out). NO_OVER: cells left alone (a rug's edge above the rooftop's railing).
OVER = {
    '1F': [((48, 160, 160, 176), (72, 142, 136, 156))],    # the reception ring's front, behind the receptionist
    '2F': [((0, 128, 48, 144), (2, 85, 30, 125))],         # the CARD SHOP counter, behind its owner
    '3F': [((128, 112, 208, 128), (100, 96, 124, 110))],   # the COIN counter's back, behind whoever talks to her
}

FLOOR, BLOCK, EXIT, STAIRS, COUNTER = 0x001, 0x000, 0x006, 0x250, 0x232

def cell(c):
    """A map cell: metatile | collision << 10 | elevation << 12"""
    if c == '#':
        return BLOCK | 1 << 10
    mid = {'.': FLOOR, 'W': EXIT, 'S': STAIRS, 'C': COUNTER}[c]
    return mid | (1 << 10 if c == 'C' else 0) | 3 << 12

def picture(name, size=None):
    """The picture as RGB555 with bit 15 = drawn (its transparent color is not), padded with nothing to SIZE."""
    im = Image.open(os.path.join(GFX, name + '.png')).convert('RGBA')
    w, h = size or im.size
    px = im.load()
    out = bytearray()
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y] if x < im.width and y < im.height else (0, 0, 0, 0)
            v = (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10 | (0x8000 if a >= 128 else 0)
            out += struct.pack('<H', v)
    open(os.path.join(GFX, name + '.bin'), 'wb').write(out)
    return w, h

WALL_ROWS = {'1F': 5, '2F': 5, '3F': 5, 'Rooftop': 3}   # the back walls: rows above this are never furniture
NO_OVER = {'Rooftop': {(x, 7) for x in range(1, 15)}}

def warm(c):
    r, g, b = c
    return r > b + 24

def over(name, floor, size):
    """The parts of the picture drawn over the sprites (all else not drawn)"""
    im = Image.open(os.path.join(GFX, name + '.png')).convert('RGB')
    px = im.load()
    w, h = size
    rows = FLOORS[floor]
    mask = [[False] * w for _ in range(h)]
    def walk(cx, cy):
        return 0 <= cy < len(rows) and 0 <= cx < len(rows[0]) and rows[cy][cx] not in '#C'
    def inside(x, y):
        return x < im.width and y < im.height
    room = set()
    for cy in range(len(rows)):
        for cx in range(len(rows[0])):
            if all(walk(cx + dx, cy + dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                room |= {px[x, y] for y in range(cy * 16, cy * 16 + 16) for x in range(cx * 16, cx * 16 + 16)
                         if inside(x, y) and warm(px[x, y])}
    for cy in range(len(rows) - 1):
        for cx in range(len(rows[0])):
            if not walk(cx, cy) or walk(cx, cy + 1) or cy + 1 < WALL_ROWS[floor] or (cx, cy) in NO_OVER.get(floor, ()):
                continue
            # what stands in the cell south of it (not black: the room's edge)
            below = [px[x, y] for y in range((cy + 1) * 16, (cy + 2) * 16) for x in range(cx * 16, cx * 16 + 16)
                     if inside(x, y)]
            if not below or sum(1 for c in below if c == (0, 0, 0)) > len(below) // 2:
                continue
            # (this cell's floor: the colors of its top half)
            floor_colors = room | {px[x, y] for y in range(cy * 16, cy * 16 + 8) for x in range(cx * 16, cx * 16 + 16)
                                   if inside(x, y)}
            for x in range(cx * 16, cx * 16 + 16):
                y = cy * 16 + 15
                while y >= cy * 16 and inside(x, y) and px[x, y] not in floor_colors:
                    mask[y][x] = True
                    y -= 1
    for (x0, y0, x1, y1), (fx0, fy0, fx1, fy1) in OVER.get(floor, []):
        near = {px[x, y] for y in range(fy0, fy1) for x in range(fx0, fx1)}
        for y in range(y0, y1):
            for x in range(x0, x1):
                mask[y][x] = mask[y][x] or px[x, y] not in near
    out = bytearray()
    for y in range(h):
        for x in range(w):
            if mask[y][x]:
                r, g, b = px[x, y]
                out += struct.pack('<H', (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10 | 0x8000)
            else:
                out += b'\0\0'
    open(os.path.join(GFX, name + '_over.bin'), 'wb').write(out)
    return sum(m for row in mask for m in row)

# The GAME CENTER on the region maps (Town Map, MOENAV, FLY): the tower drawn small by hand (its picture shrunk is a
# blur), in its own colors: the roof's screen, the purple tower, window columns, the two banners and doors.
ICON = [
    '.GGGGGGGGG.',
    'GGYYYYYYYGG',
    'GGYYYYYYYGG',
    'GGGGGGGGGGG',
    'LLLLLLLLLLL',
    'PBPRPBPRPBP',
    'POPRPOPRPOP',
    'PBPRPBPRPBP',
    'POPPPOPPPOP',
    'PBPPPBPPPBP',
    'POPPPOPPPOP',
    'PBPDPBPDPBP',
    'PPPDPPPDPPP',
]
ICON_COLORS = {'G': (144, 152, 168), 'Y': (232, 216, 152), 'L': (208, 192, 240), 'P': (152, 120, 200),
               'B': (96, 168, 232), 'O': (232, 192, 80), 'R': (232, 144, 136), 'D': (64, 80, 128)}

def icon():
    """ICON as a 16x16 sprite's 4bpp tiles and palette (src/data/mp_gamecenter_icon.h), with a dark outline"""
    keys = list(ICON_COLORS)
    idx = [[0] * 16 for _ in range(16)]
    ox, oy = (16 - len(ICON[0])) // 2, 1
    for y, row in enumerate(ICON):
        for x, ch in enumerate(row):
            if ch != '.':
                idx[oy + y][ox + x] = 2 + keys.index(ch)
    for y in range(16):
        for x in range(16):
            if not idx[y][x] and any(0 <= x + dx < 16 and 0 <= y + dy < 16 and idx[y + dy][x + dx] > 1
                                     for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                idx[y][x] = 1   # the outline
    colors = [(0, 0, 0), (40, 24, 56)] + [ICON_COLORS[k] for k in keys]
    colors += [(0, 0, 0)] * (16 - len(colors))
    def c555(c):
        return (c[0] >> 3) | (c[1] >> 3) << 5 | (c[2] >> 3) << 10
    words = []
    for ty in range(2):
        for tx in range(2):
            for y in range(8):
                v = 0
                for x in range(8):
                    v |= idx[ty * 8 + y][tx * 8 + x] << (4 * x)
                words.append(v)
    out = ['// Generated by tools/moemon_gamecenter/gen_gamecenter.py: the GAME CENTER on the region maps (region_map.c)',
           'static const u32 sMpGameCenterIconGfx[] = {']
    out += ['    ' + ', '.join('0x%08x' % w for w in words[i:i + 8]) + ',' for i in range(0, len(words), 8)]
    out += ['};', 'static const u16 sMpGameCenterIconPal[] = {', '    ' + ', '.join('0x%04x' % c555(c) for c in colors) + ',', '};']
    open(os.path.join(ROOT, 'src/data/mp_gamecenter_icon.h'), 'w').write('\n'.join(out) + '\n')
    print('icon: %dx%d in 16x16' % (len(ICON[0]), len(ICON)))

def main():
    for floor, rows in FLOORS.items():
        w, h = len(rows[0]), len(rows)
        assert all(len(r) == w for r in rows), floor
        d = os.path.join(ROOT, 'data/layouts/GameCenter_' + floor)
        os.makedirs(d, exist_ok=True)
        open(os.path.join(d, 'map.bin'), 'wb').write(b''.join(struct.pack('<H', cell(c)) for r in rows for c in r))
        open(os.path.join(d, 'border.bin'), 'wb').write(struct.pack('<4H', *([cell('#')] * 4)))
        pw, ph = picture(PICTURES[floor], (w * 16, h * 16))
        n = over(PICTURES[floor], floor, (pw, ph))
        print('GameCenter_%s: %dx%d, picture %dx%d, %d pixels over the sprites' % (floor, w, h, pw, ph, n))
    print('building: %dx%d' % picture('building'))
    icon()

if __name__ == '__main__':
    main()
