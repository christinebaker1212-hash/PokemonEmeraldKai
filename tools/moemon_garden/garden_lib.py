"""Shared pieces for the MOeMON GARDEN generator (gen_garden.py).

The Garden is built only from the original game's own pieces, the way its own maps use them:
  - General (the primary tileset) for grass, trees, paths, fences, rocks, tall grass and puddles;
  - features copied whole from the maps that have them (Route 117's flower beds, Lavaridge's hot spring, Route 120's
    marsh, Route 114's boulders, ...), with each map's own collision and elevation bits;
  - tree clusters and sand paths drawn by rules read from the original maps (autotile).
Secondary metatiles from several tilesets end up in one secondary tileset (gTileset_MoemonGarden): their tiles are
copied and their colours packed into the seven secondary palette slots (6-12).
"""
import json, os, struct
from collections import Counter, defaultdict
from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
TS_DIR = os.path.join(ROOT, 'data/tilesets')
NUM_PRIMARY = 0x200      # metatiles (and tiles) of the primary tileset
SECONDARY_SLOTS = list(range(6, 13))

_layouts = None
def layouts():
    global _layouts
    if _layouts is None:
        _layouts = {l['name'].replace('_Layout', ''): l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'name' in l}
    return _layouts

def ts_dir_name(symbol):
    """gTileset_PokemonCenter -> pokemon_center (the folder under data/tilesets)."""
    name = symbol.replace('gTileset_', '')
    out = ''
    for i, ch in enumerate(name):
        if ch.isupper() and i and not name[i - 1].isupper():
            out += '_'
        out += ch.lower()
    return {'ever_grande': 'ever_grande'}.get(out, out)

def load_layout(name):
    """(grid of raw u16, width, height, primary folder, secondary folder)"""
    l = layouts()[name]
    d = open(os.path.join(ROOT, l['blockdata_filepath']), 'rb').read()
    v = struct.unpack('<%dH' % (len(d) // 2), d)
    w = l['width']
    return [list(v[r * w:(r + 1) * w]) for r in range(len(v) // w)], w, len(v) // w, \
        ts_dir_name(l['primary_tileset']), ts_dir_name(l['secondary_tileset'])

def pal(path):
    lines = open(path).read().replace('\r', '').split('\n')[3:19]
    return [tuple(int(v) for v in l.split()[:3]) for l in lines if l.strip()]

_ts = {}
def tileset(folder, primary):
    key = (folder, primary)
    if key not in _ts:
        d = os.path.join(TS_DIR, 'primary' if primary else 'secondary', folder)
        img = Image.open(os.path.join(d, 'tiles.png'))
        px = img.load()
        w, h = img.size
        tiles = [[[px[tx * 8 + x, ty * 8 + y] & 15 for x in range(8)] for y in range(8)]
                 for ty in range(h // 8) for tx in range(w // 8)]
        mt = open(os.path.join(d, 'metatiles.bin'), 'rb').read()
        metas = [list(struct.unpack('<8H', mt[i:i + 16])) for i in range(0, len(mt), 16)]
        at = open(os.path.join(d, 'metatile_attributes.bin'), 'rb').read()
        attrs = list(struct.unpack('<%dH' % (len(at) // 2), at))
        pals = [pal(os.path.join(d, 'palettes', '%02d.pal' % i)) + [(0, 0, 0)] * 16 for i in range(16)]
        _ts[key] = dict(tiles=tiles, metas=metas, attrs=attrs, pals=pals)
    return _ts[key]

# ---------------------------------------------------------------------------
# Cells: a map cell is (bits, ref) where bits are the collision/elevation bits (0xFC00 of the u16) and ref is either
# an int (a General metatile, < 0x200) or (secondary folder, primary folder, metatile id) or ('fixed', id) for a
# metatile kept at its own id in the Garden tileset (the escalator).

def cell_from_raw(raw, primary, secondary):
    mid = raw & 0x3FF
    if mid < NUM_PRIMARY:
        assert primary == 'general', 'a primary metatile from %s' % primary
        return (raw & 0xFC00, mid)
    return (raw & 0xFC00, (secondary, primary, mid))

def chunk(layout, x0, y0, w, h):
    """A rectangle of an original map as cells (rows of (bits, ref))."""
    g, W, H, prim, sec = load_layout(layout)
    return [[cell_from_raw(g[y][x], prim, sec) for x in range(x0, x0 + w)] for y in range(y0, y0 + h)]

# ---------------------------------------------------------------------------
# Pictures of metatiles, and made-up metatiles (CUSTOM): pieces of the original art changed to fit (a crystal or a
# rock lifted off its cave floor onto grass, a rim whose cliff behind it becomes grass)

def tile_image(primary, secondary, e):
    """An 8x8 tile of a metatile entry as RGB (None = transparent), flips applied."""
    t, p = e & 0x3FF, e >> 12
    if t < 512:
        src = tileset(primary, True)['tiles']
        img = src[t] if t < len(src) else [[0] * 8] * 8
    else:
        src = tileset(secondary, False)['tiles']
        img = src[t - 512] if t - 512 < len(src) else [[0] * 8] * 8
    colors = tileset(primary, True)['pals'][p] if p < 6 else tileset(secondary, False)['pals'][p]
    rows = [[(colors[c] if c else None) for c in row] for row in img]
    if e & 0x400:
        rows = [row[::-1] for row in rows]
    if e & 0x800:
        rows = rows[::-1]
    return rows

def meta_image(ref, layer=None):
    """16x16 RGB of a metatile (ref: General id or (secondary, primary, id)); layer 0/1 alone, or both."""
    if isinstance(ref, tuple) and ref[0] == 'general_as_ref':
        ref = ref[1]
    if isinstance(ref, int):
        prim, sec, m = 'general', None, tileset('general', True)['metas'][ref]
    else:
        sec, prim, mid = ref
        m = tileset(sec, False)['metas'][mid - NUM_PRIMARY] if mid >= NUM_PRIMARY else tileset(prim, True)['metas'][mid]
    out = [[None] * 16 for _ in range(16)]
    for l in ((0, 1) if layer is None else (layer,)):
        for k in range(4):
            img = tile_image(prim, sec, m[l * 4 + k])
            for y in range(8):
                for x in range(8):
                    if img[y][x] is not None:
                        out[(k // 2) * 8 + y][(k % 2) * 8 + x] = img[y][x]
    return out

def split4(img16):
    return [[row[(k % 2) * 8:(k % 2) * 8 + 8] for row in img16[(k // 2) * 8:(k // 2) * 8 + 8]] for k in range(4)]

CUSTOM = {}    # name -> (entries: 8 x ('keep', u16) | ('img', 8x8 RGB/None), attribute)

def custom(name, bottom, top, attr):
    """bottom/top: 4 entries each, ('keep', General entry) or an 8x8 picture."""
    ents = []
    for e in list(bottom) + list(top):
        ents.append(e if isinstance(e, tuple) else ('img', e))
    CUSTOM[name] = (ents, attr)
    return ('custom', name)

# ---------------------------------------------------------------------------
# The Garden tileset: every secondary ref used by the map, packed

def build_tileset(refs, fixed_ids, out_dir):
    """refs: secondary refs (folder, primary folder, id) or ('custom', name) used by the map; fixed_ids: {ref: id}
    kept at their own ids (the escalator). Returns ({ref: Garden metatile id}, stats). Writes metatiles.bin,
    metatile_attributes.bin, palettes/ and tiles.png."""
    refs = sorted(set(refs) | set(fixed_ids), key=lambda r: tuple(str(v) for v in r))
    ids = dict(fixed_ids)
    taken = set(ids.values())
    nxt = NUM_PRIMARY
    for r in refs:
        if r in ids:
            continue
        while nxt in taken:
            nxt += 1
        ids[r] = nxt
        taken.add(nxt)
        nxt += 1
    count = max(taken) - NUM_PRIMARY + 1
    uses = {}       # key -> 8x8 RGB/None
    meta_src = {}
    attrs = {}
    for r in refs:
        entries = []
        if r[0] == 'custom':
            ents, attrs[r] = CUSTOM[r[1]]
            for e in ents:
                if e[0] == 'keep':
                    entries.append(e)
                else:
                    key = ('img', tuple(tuple(row) for row in e[1]))
                    uses[key] = e[1]
                    entries.append(('copy', key, 0))
        else:
            sec, prim, mid = r
            m = tileset(sec, False)['metas'][mid - NUM_PRIMARY]
            attrs[r] = tileset(sec, False)['attrs'][mid - NUM_PRIMARY]
            for e in m:
                t, p = e & 0x3FF, e >> 12
                if t < 512 and p < 6 and prim == 'general':
                    entries.append(('keep', e))    # General's own tile and palette
                    continue
                key = ((prim if t < 512 else sec), t, p)
                uses[key] = tile_image(prim, sec, e & 0xF3FF)
                entries.append(('copy', key, e & 0x0C00))
        meta_src[r] = entries
    # colours: each tile's set of colours into the 7 free palette slots (greedy, biggest sets first)
    csets = {k: frozenset(c for row in img for c in row if c is not None) for k, img in uses.items()}
    order = sorted(uses, key=lambda k: (-len(csets[k]), str(k)))
    slots, slot_of = [], {}
    for k in order:
        cs = csets[k]
        best = None
        for i, s in enumerate(slots):
            add = len(cs - s)
            if len(s) + add <= 15 and (best is None or add < best[1]):
                best = (i, add)
        if best is None:
            if len(slots) >= len(SECONDARY_SLOTS):
                raise SystemExit('colours: no room for %s (%d colours; slots %s)' % (k, len(cs), [len(s) for s in slots]))
            slots.append(set())
            best = (len(slots) - 1, 0)
        slots[best[0]] |= cs
        slot_of[k] = best[0]
    palettes = [[(0, 0, 0)] + sorted(s) for s in slots]
    out_tiles = [[[0] * 8 for _ in range(8)]]
    tile_index = {}
    def place(k):
        si = slot_of[k]
        p = palettes[si]
        img = tuple(tuple(p.index(c) if c is not None else 0 for c in row) for row in uses[k])
        if (si, img) not in tile_index:
            tile_index[(si, img)] = len(out_tiles)
            out_tiles.append([list(row) for row in img])
        return tile_index[(si, img)], SECONDARY_SLOTS[si]
    out_metas = [[0] * 8 for _ in range(count)]
    out_attrs = [0] * count
    for r, entries in meta_src.items():
        row = []
        for ent in entries:
            if ent[0] == 'keep':
                row.append(ent[1])
            else:
                ti, slot = place(ent[1])
                row.append((512 + ti) | ent[2] | (slot << 12))
        out_metas[ids[r] - NUM_PRIMARY] = row
        out_attrs[ids[r] - NUM_PRIMARY] = attrs[r]
    assert len(out_tiles) <= 512, 'too many tiles: %d' % len(out_tiles)
    os.makedirs(os.path.join(out_dir, 'palettes'), exist_ok=True)
    with open(os.path.join(out_dir, 'metatiles.bin'), 'wb') as f:
        for m in out_metas:
            f.write(struct.pack('<8H', *m))
    with open(os.path.join(out_dir, 'metatile_attributes.bin'), 'wb') as f:
        f.write(struct.pack('<%dH' % len(out_attrs), *out_attrs))
    allpals = [[(0, 0, 0)] * 16 for _ in range(16)]
    for i, p in enumerate(palettes):
        allpals[SECONDARY_SLOTS[i]] = p + [(0, 0, 0)] * (16 - len(p))
    for i, p in enumerate(allpals):
        with open(os.path.join(out_dir, 'palettes/%02d.pal' % i), 'w') as f:
            f.write('JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in p))
    rows = (len(out_tiles) + 15) // 16
    im = Image.new('P', (128, rows * 8))
    flat = []
    for c in allpals[SECONDARY_SLOTS[0]]:
        flat += list(c)
    im.putpalette(flat + [0] * (768 - len(flat)))
    px = im.load()
    for i, t in enumerate(out_tiles):
        for y in range(8):
            for x in range(8):
                px[(i % 16) * 8 + x, (i // 16) * 8 + y] = t[y][x]
    im.save(os.path.join(out_dir, 'tiles.png'))
    return ids, dict(metatiles=count, tiles=len(out_tiles), slots=[len(s) for s in slots])

# ---------------------------------------------------------------------------
# Autotile rules read from the original maps: for a class of metatiles, which one stands at each 8-neighbourhood
# shape (a neighbour in the class or not), with its bits

def learn(cls_ids, layout_names, primary='general'):
    rules = defaultdict(Counter)
    for name in layout_names:
        g, W, H, prim, sec = load_layout(name)
        if prim != primary:
            continue
        def inc(x, y):
            if 0 <= x < W and 0 <= y < H:
                return (g[y][x] & 0x3FF) in cls_ids
            return True
        for y in range(H):
            for x in range(W):
                if (g[y][x] & 0x3FF) in cls_ids:
                    key = tuple(inc(x + dx, y + dy) for dy in (-1, 0, 1) for dx in (-1, 0, 1) if (dx, dy) != (0, 0))
                    rules[key][g[y][x]] += 1
    return rules

def pick(rules, key):
    """The most common metatile (with bits) for a neighbourhood; the closest learnt shape when it was never seen."""
    if key in rules:
        return rules[key].most_common(1)[0][0]
    best, bd = None, 99
    for k, c in rules.items():
        d = sum(a != b for a, b in zip(k, key)) * 4 - min(sum(c.values()), 3)
        # the 4 direct neighbours matter most
        d += sum(2 for i in (1, 3, 4, 6) if k[i] != key[i])
        if d < bd:
            best, bd = c.most_common(1)[0][0], d
    return best
