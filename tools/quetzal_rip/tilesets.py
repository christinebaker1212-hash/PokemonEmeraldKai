"""Rip the tilesets the Kanto / Johto layouts use, in decomp layout:
tilesets/<primary|secondary>/<name>/{tiles.png, palettes/NN.pal, metatiles.bin, metatile_attributes.bin}.
"""
import glob
import json
import os
from collections import Counter

from gfx import lz77, gba_colors, write_jasc, tiles_png


def parse_tileset(rom, t):
    return {'addr': t, 'compressed': rom.d[t], 'secondary': rom.d[t + 1], 'tiles': rom.ptr(t + 4),
            'palettes': rom.ptr(t + 8), 'metatiles': rom.ptr(t + 12), 'attributes': rom.ptr(t + 16),
            'callback': rom.u32(t + 20)}


def all_tilesets(rom, maps):
    ts = {}
    for m in maps:
        L = m['layout']
        for k in ('primary', 'secondary'):
            if L[k] is not None and L[k] not in ts:
                ts[L[k]] = parse_tileset(rom, L[k])
    return ts


def metatile_counts(rom, ts):
    """Metatile counts from the gaps between consecutive metatile / attribute blocks."""
    mts = sorted({t['metatiles'] for t in ts.values() if t['metatiles'] is not None})
    ats = sorted({t['attributes'] for t in ts.values() if t['attributes'] is not None})
    out = {}
    for a, t in ts.items():
        m, at = t['metatiles'], t['attributes']
        nm = na = None
        i = mts.index(m)
        if i + 1 < len(mts):
            nm = (mts[i + 1] - m) // 16
        if at is not None:
            j = ats.index(at)
            if j + 1 < len(ats):
                na = (ats[j + 1] - at) // 2
        if at is not None and at > m:
            nm = min(nm, (at - m) // 16) if nm else (at - m) // 16
        cands = [x for x in (nm, na) if x]
        default = 640 if not t['secondary'] else 384
        n = min(cands) if cands else default
        out[a] = min(n, 1024)
    return out


def frlg_index(frlg_dir):
    idx = {}
    if not frlg_dir or not os.path.isdir(frlg_dir):
        return idx
    for p in glob.glob(os.path.join(frlg_dir, 'data/tilesets/*/*/metatiles.bin')):
        kind, name = p.split(os.sep)[-3:-1]
        idx[open(p, 'rb').read()] = '%s/%s' % (kind, name)
    return idx


def rip_tilesets(rom, maps, targets, out, frlg_dir, map_name_of):
    ts = all_tilesets(rom, targets)
    counts = metatile_counts(rom, all_tilesets(rom, maps))
    frlg = frlg_index(frlg_dir)
    users = {}
    for m in targets:
        for k in ('primary', 'secondary'):
            users.setdefault(m['layout'][k], []).append(map_name_of(m))
    hoenn_users = Counter()
    for m in maps:
        if m['region'] == 'hoenn':
            for k in ('primary', 'secondary'):
                hoenn_users[m['layout'][k]] += 1
    names = {}
    used = set()
    jp = 0
    rows = []
    for a in sorted(ts):
        t = ts[a]
        n = counts[a]
        mt = rom.d[t['metatiles']:t['metatiles'] + 16 * n]
        src = None
        for size in (n,):
            src = frlg.get(mt)
        if src is None:   # some FRLG sets were trimmed or grown: compare the common part
            for blob, nm in frlg.items():
                k = min(len(blob), len(mt))
                if k >= 160 and blob[:k] == mt[:k]:
                    src = nm + ' (first %d metatiles)' % (k // 16)
                    break
        regions = sorted({m['region'] for m in targets if a in (m['layout']['primary'], m['layout']['secondary'])})
        if hoenn_users[a]:
            name = 'Emerald_%X' % (a + 0x08000000)
            note = 'also used by %d Hoenn maps: a stock Emerald tileset' % hoenn_users[a]
        elif src:
            name = 'Kanto_' + ''.join(w.capitalize() for w in src.split()[0].split('/')[1].split('_'))
            note = 'pokefirered data/tilesets/' + src
        elif not t['secondary']:
            jp += 1
            name = '%s_Primary%d' % ('Johto' if 'johto' in regions else 'Kanto', jp)
            note = ''
        else:
            name = users[a][0]
            note = ''
        while name in used:
            name += '_2'
        used.add(name)
        names[a] = name
        kind = 'secondary' if t['secondary'] else 'primary'
        d = os.path.join(out, 'tilesets', kind, name)
        os.makedirs(os.path.join(d, 'palettes'), exist_ok=True)
        if t['compressed']:
            tiles, _ = lz77(rom.d, t['tiles'])
        else:
            tiles = rom.d[t['tiles']:t['tiles'] + 32 * (640 if kind == 'primary' else 384)]
        pals = [gba_colors(rom.d, t['palettes'] + 32 * i) for i in range(16)]
        for i, p in enumerate(pals):
            write_jasc(os.path.join(d, 'palettes', '%02d.pal' % i), p)
        # preview palette: the one the metatiles use most
        pc = Counter()
        for i in range(0, len(mt), 2):
            v = mt[i] | mt[i + 1] << 8
            pc[v >> 12] += 1
        best = pc.most_common(1)[0][0] if pc else 0
        tiles_png(os.path.join(d, 'tiles.png'), tiles, pals[best])
        open(os.path.join(d, 'metatiles.bin'), 'wb').write(mt)
        if t['attributes'] is not None:
            open(os.path.join(d, 'metatile_attributes.bin'), 'wb').write(rom.d[t['attributes']:t['attributes'] + 2 * n])
        rows.append({'name': 'gTileset_' + name, 'quetzal_addr': '0x%X' % (a + 0x08000000), 'kind': kind,
                     'compressed': bool(t['compressed']), 'tiles': len(tiles) // 32, 'metatiles': n,
                     'png_palette': best, 'anim_callback': '0x%X' % t['callback'] if t['callback'] else None,
                     'regions': regions, 'used_by': users[a][:12] + (['...%d more' % (len(users[a]) - 12)] if len(users[a]) > 12 else []),
                     'source': note})
    with open(os.path.join(out, 'tilesets', 'tilesets.json'), 'w') as fh:
        json.dump(rows, fh, indent=1)
        fh.write('\n')
    return names
