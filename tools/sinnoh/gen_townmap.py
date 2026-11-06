#!/usr/bin/env python3
"""SINNOH's TOWN MAP and FLY (src/region_map.c's SINNOH pages, src/sinnoh/s3d_townmap.c), from pret/pokeplatinum's data:

- a map section a place name: each kept place's Platinum location label (its map header's mapLabelTextID), the names
  written from the label's own words (ROUTE 201, MT. CORONET...); MAPSEC_SINNOH_FIRST (0x140) + n, by name
- where each is: Platinum's town map grid is its world matrix's cells (res/town_map/town_map_data.json blocks: the area
  or landmark at each cell); the game's map grid is 28x15 cells, so SINNOH is two pages: NORTH (matrix rows 6-20) and
  SOUTH (rows 14-28), L / R between them. A place not on Platinum's grid (caves, buildings) sits on the cell of the
  door that leads to it from outside
- FLY: the places with a heal location (src/data/heal_locations.json), each unlocked by its own flag (0x3D40 + n, set
  on arriving: s3d_townmap.c Sinnoh_TownMapArrive)

    python3 -I tools/sinnoh/gen_townmap.py POKEPLATINUM_DIR     (gen_maps.py takes each map's section from here)

Writes include/constants/region_map_sections_sinnoh.h and src/data/region_map/sinnoh_region_map.h. The picture is
Platinum's own town map art, built into sinnoh/platinum_gfx.pak by build_gfx.py (picture() below), never committed.
"""
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from places import events  # noqa: E402

FIRST = 0x140            # past JOHTO's (0x100-0x139)
PAGES = [6, 14]          # each page's top matrix row (15 rows a page)
COLS, ROWS = 28, 15
FLY_FLAGS = 0x3D40       # (free: the legendaries' flags are 0x3D00-0x3D0B and 0x3D80-0x3D8B)
LABEL = 'LocationNames_Text_'
NAMES = {   # names whose words aren't the label's (the label run together, or past 16 letters)
    'MtCoronet': 'MT. CORONET', 'PokemonLeague': 'POKéMON LEAGUE', 'PokemonDayCare': 'POKéMON DAY CARE',
    'PokemonMansion': 'POKéMON MANSION', 'TeamGalacticEternaBuilding': 'GALACTIC BLDG.', 'GalacticHQ': 'GALACTIC HQ',
    'JubilifeTV': 'JUBILIFE TV', 'PoketchCo': 'POKéTCH CO.', 'RotomsRoom': 'ROTOM\'S ROOM',
    'TrainersSchool': 'TRAINERS\' SCHOOL', 'Cafe': 'CAFé',
}
KEYS = {'eternia_city': 'EternaCity', 'mount_coronet': 'MtCoronet'}   # (Platinum's town map keys that differ)


def name(label):
    if label in NAMES:
        return NAMES[label]
    words = re.sub(r'(?<=[a-z])(?=[A-Z0-9])|(?<=[0-9])(?=[A-Z])', ' ', label).upper()
    assert len(words) <= 16, words
    return words


def const(label):
    return 'MAPSEC_SINNOH_' + re.sub(r'(?<=[a-z0-9])(?=[A-Z])|(?<=[a-z])(?=[0-9])', '_', label).upper()


def label_of(place):
    return place['fields'].get('mapLabelTextID', '')[len(LABEL):]


def key_label(key, labels):
    """A town map key (route_205_north, eternia_city...) as a location label, or None"""
    if key is None:
        return None
    if key in KEYS:
        return KEYS[key]
    k = re.sub(r'_(north|south)$', '', key).replace('_', '').lower()
    for l in labels:
        if l.lower() == k:
            return l
    return None


def data(root, places):
    """{'labels': [...], 'cells': {(x, z): label}, 'door': {label: (x, z)}} (places: gen_maps.places())"""
    labels = sorted({label_of(p['place']) for p in places if label_of(p['place'])})
    blocks = json.load(open(os.path.join(root, 'res/town_map/town_map_data.json')))['blocks']
    cells = {}
    for b in blocks:
        l = key_label(b.get('landmark'), labels) or key_label(b.get('area'), labels)
        if l:
            cells[(b['x'], b['z'])] = l
    # every other place: the cell of the door leading to it from outside (through other buildings too)
    by_header = {p['header']: p for p in places}
    door = {}
    outdoor = [p for p in places if p['outdoor']]
    frontier = []
    for p in outdoor:
        for w in events(root, p['place'])['warp_events']:
            frontier.append((w['dest_header_id'], (w['x'] // 32, w['z'] // 32)))
    seen = set()
    while frontier:
        nxt = []
        for h, cell in frontier:
            if h in seen or h not in by_header or by_header[h]['outdoor']:
                continue
            seen.add(h)
            l = label_of(by_header[h]['place'])
            door.setdefault(l, cell)
            for w in events(root, by_header[h]['place'])['warp_events']:
                nxt.append((w['dest_header_id'], cell))
        frontier = nxt
    for p in outdoor:   # an outdoor place's own cells (a city without a block of its own)
        l = label_of(p['place'])
        door.setdefault(l, (p['place']['cells'][0][0], p['place']['cells'][0][1]))
    return {'labels': labels, 'cells': cells, 'door': door}


def page_of(z):
    """The page that shows matrix row z best (rows 14-20 are on both: the nearer middle)"""
    return min(range(len(PAGES)), key=lambda k: abs(z - (PAGES[k] + ROWS // 2)) if PAGES[k] <= z < PAGES[k] + ROWS else 99)


def mapsec_of(d, place):
    l = label_of(place)
    return const(l) if l in d['labels'] else 'MAPSEC_NONE'


def write(root, places):
    d = data(root, places)
    labels = d['labels']
    head = '// Generated by tools/sinnoh/gen_townmap.py. Do not edit.\n'
    with open('include/constants/region_map_sections_sinnoh.h', 'w') as f:
        f.write(head + '#ifndef GUARD_CONSTANTS_REGION_MAP_SECTIONS_SINNOH_H\n#define GUARD_CONSTANTS_REGION_MAP_SECTIONS_SINNOH_H\n'
                '// SINNOH\'s map sections, Platinum\'s place names (MP_METGAME_SINNOH keeps n - MAPSEC_SINNOH_FIRST as a '
                'Pokemon\'s met location)\n#define MAPSEC_SINNOH_FIRST 0x%X\n' % FIRST)
        for i, l in enumerate(labels):
            f.write('#define %-40s (MAPSEC_SINNOH_FIRST + %d)\n' % (const(l), i))
        f.write('#define MAPSEC_SINNOH_END (MAPSEC_SINNOH_FIRST + %d)\n#endif\n' % len(labels))
    out = [head, '// SINNOH\'s TOWN MAP pages (region_map.c QZMAP_SINNOH_NORTH / _SOUTH) and FLY spots\n']
    out += ['static const u8 sSinnohMapSecName%d[] = _("%s");\n' % (i, name(l)) for i, l in enumerate(labels)]
    out.append('static const u8 *const gSinnohMapSectionNames[] = {\n')
    out += ['    [%s - MAPSEC_SINNOH_FIRST] = sSinnohMapSecName%d,\n' % (const(l), i) for i, l in enumerate(labels)]
    out.append('};\n\nstatic const mapsec_u16_t sSinnohMapLayouts[%d][MAP_HEIGHT][MAP_WIDTH] = {\n' % len(PAGES))
    boxes = {}   # (label, page): [x0, y0, x1, y1]
    for k, z0 in enumerate(PAGES):
        out.append('    {\n')
        for gy in range(ROWS):
            row = []
            for gx in range(COLS):
                l = d['cells'].get((gx + 1, gy + z0))
                row.append(const(l) if l else 'MAPSEC_NONE')
                if l:
                    b = boxes.setdefault((l, k), [gx, gy, gx, gy])
                    b[:] = [min(b[0], gx), min(b[1], gy), max(b[2], gx), max(b[3], gy)]
            out.append('        {' + ', '.join(row) + '},\n')
        out.append('    },\n')
    out.append('};\n\n')
    # places: each section's box on each page it shows on, the page it belongs to first (QzPlace finds that one)
    rows = []
    for l in labels:
        zs = [z for (x, z), m in d['cells'].items() if m == l]
        if not zs and l in d['door']:
            x, z = d['door'][l]
            if 1 <= x <= COLS:
                for k, z0 in enumerate(PAGES):
                    if z0 <= z < z0 + ROWS:
                        boxes[(l, k)] = [x - 1, z - z0, x - 1, z - z0]
                zs = [z]
        if not zs:
            continue
        home = page_of(sorted(zs)[len(zs) // 2])
        for k in [home] + [k for k in range(len(PAGES)) if k != home]:
            if (l, k) in boxes:
                b = boxes[(l, k)]
                rows.append('    {%s, QZMAP_SINNOH_NORTH + %d, %d, %d, %d, %d},\n'
                            % (const(l), k, b[0], b[1], b[2] - b[0] + 1, b[3] - b[1] + 1))
    out.append('static const struct QzMapPlace sSinnohMapPlaces[] = {\n' + ''.join(rows) + '};\n\n')
    # FLY: the heal locations' places
    heals = json.load(open('src/data/heal_locations.json'))['heal_locations']
    map_label = {p['id']: label_of(p['place']) for p in places}
    fly, done = [], set()
    for h in heals:
        l = map_label.get(h['map'])
        if not h['id'].startswith('HEAL_LOCATION_SINNOH_') or not l or l in done:
            continue
        done.add(l)
        page = next((k for k in range(len(PAGES)) if (l, k) in boxes), None)
        if page is None:
            continue
        zs = [z for (x, z), m in d['cells'].items() if m == l] or [d['door'][l][1]]
        page = page_of(sorted(zs)[len(zs) // 2])
        fly.append('    {%s, QZMAP_SINNOH_NORTH + %d, %s, 0x%X},\n' % (const(l), page, h['id'], FLY_FLAGS + len(fly)))
    out.append('static const struct QzFlyDest sSinnohFlyDests[] = {\n' + ''.join(fly) + '};\n')
    open('src/data/region_map/sinnoh_region_map.h', 'w').write(''.join(out))
    return d


# ------------------------------------------------------------------------------------------- the picture (build_gfx)
TEX_W, TEX_H = 480, 320   # a page in the full view's screen space at twice the resolution (sharper when zoomed)


def picture(root, page):
    """Page PAGE of the TOWN MAP as RGB555 rows (TEX_W x TEX_H): Platinum's top screen map (its tiles, the sea and land
    layer and the roads layer) scaled by 8/7 (its 7-pixel grid onto the game's 8-pixel one), smoothly"""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import png8
    D = os.path.join(root, 'res/graphics/town_map/')
    tiles = png8.Image(D + 'top_screen_map_tiles.png')
    pal = [tuple(int(v) for v in l.split()) for l in open(D + 'top_screen_map.pal').read().split('\n')[3:] if l.strip()]
    W, H = 256, 192
    img = [[pal[0]] * W for _ in range(H)]
    tw = tiles.w // 8
    for f in ('top_screen_region_bg_tilemap.NSCR', 'top_screen_region_map_tilemap.NSCR'):
        b = open(D + f, 'rb').read()
        mw, mh = struct.unpack('<HH', b[0x18:0x1C])
        e = struct.unpack('<%dH' % (mw // 8 * mh // 8), b[0x24:0x24 + mw // 8 * mh // 8 * 2])
        for ty in range(mh // 8):
            for tx in range(mw // 8):
                v = e[ty * (mw // 8) + tx]
                t, hf, vf, p = v & 0x3FF, v >> 10 & 1, v >> 11 & 1, v >> 12
                for y in range(8):
                    for x in range(8):
                        # (the decomp's gray sheets keep each pixel's color number as 15 - its gray level)
                        c = 15 - tiles.px((t % tw) * 8 + (7 - x if hf else x), (t // tw) * 8 + (7 - y if vf else y))
                        if c:
                            img[ty * 8 + y][tx * 8 + x] = pal[p * 16 + c]
    X0, X1, Y0, Y1 = 21, 235, 1, 167   # the map inside Platinum's frame
    z0 = PAGES[page]

    def ds(sx, sy):   # full view screen point -> Platinum's top screen
        return sx * 7 / 8 + 21.5, (sy + 8 * z0 - 16) * 7 / 8 - 37.5

    out = []
    for Y in range(TEX_H):
        row = []
        for X in range(TEX_W):
            # the 4 points of this half pixel (a box filter over the 8/7 scale), each bilinear
            r = g = bl = 0.0
            for oy in (0.25, 0.75):
                for ox in (0.25, 0.75):
                    fx, fy = ds((X + ox) / 2, (Y + oy) / 2)
                    fx = min(max(fx - 0.5, X0), X1)
                    fy = min(max(fy - 0.5, Y0), Y1)
                    ix, iy = int(fx), int(fy)
                    ax, ay = fx - ix, fy - iy
                    for cx, cy, w in ((ix, iy, (1 - ax) * (1 - ay)), (ix + 1, iy, ax * (1 - ay)),
                                      (ix, iy + 1, (1 - ax) * ay), (ix + 1, iy + 1, ax * ay)):
                        c = img[min(cy, Y1)][min(cx, X1)]
                        r += c[0] * w
                        g += c[1] * w
                        bl += c[2] * w
            row.append((round(r / 4) >> 3) | (round(g / 4) >> 3) << 5 | (round(bl / 4) >> 3) << 10)
        out.append(row)
    return out



# ------------------------------------------------------------------------------- the FLY spots' blocks (build_gfx)
BLOCK_SHAPES = ['1x1_SQUARE', 'VERTICAL', '2x2_SQUARE', 'TOP_LEFT_ANGLE', 'TOP_RIGHT_ANGLE', 'HORIZONTAL',
                'SMALL_RECTANGLE']


def blocks(root, page):
    """Platinum's FLY spots' blocks on page PAGE (its applications/town_map/fly_locations.c sFlyLocations, the art
    fly_destination_blocks.png): [(fly spot index in sSinnohFlyDests or 0xFF, hide while locked, x, y, w, h, locked
    RGB555 rows, unlocked rows)] in the page's texels, 0 = clear, else 0x8000 | color. Locked in Platinum's palette 5
    (gray), unlocked in 6 (towns, blue) or 7 (cities, red); the VICTORY ROAD block only once unlocked (the LEAGUE's
    spot here), PAL PARK's never (closed)"""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import png8
    from gen_frontier import decomp_file
    src = decomp_file(root, 'src/applications/town_map/fly_locations.c')
    img = png8.Image(os.path.join(root, 'res/graphics/town_map/fly_destination_blocks.png'))
    with open(os.path.join(root, 'res/graphics/town_map/fly_destination_blocks.png'), 'rb') as f:
        raw = f.read()
    # (the PNG's own palette: png8 keeps indices; PLTE read here)
    i = raw.index(b'PLTE')
    n = struct.unpack('>I', raw[i - 4:i])[0]
    pal = [tuple(raw[i + 4 + 3 * k:i + 7 + 3 * k]) for k in range(n // 3)]
    order = re.findall(r'\{(MAPSEC_SINNOH_\w+),', open('src/data/region_map/sinnoh_region_map.h').read().split(
        'sSinnohFlyDests[]')[1])
    z0 = PAGES[page]

    def ds(sx, sy):
        return sx * 7 / 8 + 21.5, (sy + 8 * z0 - 16) * 7 / 8 - 37.5

    def rgb(c):
        return 0x8000 | (c[0] >> 3) | (c[1] >> 3) << 5 | (c[2] >> 3) << 10

    out = []
    for m in re.finditer(r'\.mapHeader = MAP_HEADER_(\w+),.*?\.blockShape = FLY_LOCATION_SHAPE_(\w+),\s*'
                         r'\.palette = FLY_LOCATION_PALETTE_(\w+),\s*\.specialFlyLocationID = (\w+),\s*'
                         r'\.spriteX = ([^,]+),\s*\.spriteY = ([^,]+),', src, re.S):
        header, shape, palname, special = m.group(1), BLOCK_SHAPES.index(m.group(2)), m.group(3), m.group(4)
        px = eval(m.group(5).replace('TOWN_MAP_GRID_SPACING', '7')) + 25 - 8
        py = eval(m.group(6).replace('TOWN_MAP_GRID_SPACING', '7')) - 34 - 8
        if special == 'FLY_LOCATION_PAL_PARK':
            continue
        sec = 'MAPSEC_SINNOH_' + header
        idx = order.index(sec) if sec in order else 0xFF
        hide = 1 if special == 'FLY_LOCATION_VICTORY_ROAD' else 0
        unl = 1 if palname == 'BLUE' else 2
        # the sprite's box in the page's texels (the inverse of ds, at twice the resolution)
        x0 = int(((px - 21.5) * 8 / 7) * 2) - 1
        y0 = int(((py + 37.5) * 8 / 7 - 8 * z0 + 16) * 2) - 1
        x1 = int(((px + 16 - 21.5) * 8 / 7) * 2) + 2
        y1 = int(((py + 16 + 37.5) * 8 / 7 - 8 * z0 + 16) * 2) + 2
        if x1 <= 0 or y1 <= 0 or x0 >= TEX_W or y0 >= TEX_H:
            continue
        x0, y0, x1, y1 = max(x0, 0), max(y0, 0), min(x1, TEX_W), min(y1, TEX_H)
        locked, unlocked = [], []
        for Y in range(y0, y1):
            lr, ur = [], []
            for X in range(x0, x1):
                fx, fy = ds((X + 0.5) / 2, (Y + 0.5) / 2)
                sx, sy = int(fx) - px, int(fy) - py
                c = img.px(sx, shape * 16 + sy) if 0 <= sx < 16 and 0 <= sy < 16 else 0
                lr.append(rgb(pal[c]) if c else 0)
                ur.append(rgb(pal[unl * 16 + c]) if c else 0)
            locked.append(lr)
            unlocked.append(ur)
        out.append((idx, hide, x0, y0, x1 - x0, y1 - y0, locked, unlocked))
    return out


if __name__ == '__main__':
    from gen_maps import places as all_places
    from source import open_source
    root = sys.argv[1]
    d = write(root, all_places(open_source(root), root))
    print('%d SINNOH map sections, %d on the TOWN MAP' % (len(d['labels']), len(set(d['cells'].values()))))
