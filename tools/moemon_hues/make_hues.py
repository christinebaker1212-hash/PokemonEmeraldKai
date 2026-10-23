#!/usr/bin/env python3
"""Makes src/data/mp_hues.h for the Stadium hues (src/mp/mp_hue.c).

Per species:
  - sMpHueMin / sMpHueMax: Pokemon Stadium 2's hue limits (stadium_hues.json, from the bluemoonfalls Stadium hue
    page, by national dex number). Gen 3 species weren't in Stadium: their limits are the median of the Gen 1-2
    species with the same first type and body color (else the same first type, else all of them).
  - sMpHueSkin: the palette slots of Kai's front picture that are skin (kept when the rest of the palette turns).
    skin.py finds them (light skin tones, then darker ones that mostly border those); skin_overrides.json has the
    ones checked by eye where the rule got hair or clothes of a skin color (or missed a shade). The same slots are
    skin in the shiny palette and on the back picture (same picture layout, same palette).

usage: make_hues.py [REPO]   (writes REPO/src/data/mp_hues.h)
"""
import json, os, re, sys, statistics
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import gba, skin

REPO = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, '..', '..')
G = os.path.join(REPO, 'graphics', 'moemon')


def species_consts():
    out = {}
    for m in re.finditer(r'#define SPECIES_(\w+)\s+(\d+)\b', open(os.path.join(REPO, 'include/constants/species.h')).read()):
        out[m.group(1)] = int(m.group(2))
    return out


def table(path, macro):
    return dict(re.findall(macro + r'\((\w+),\s*(\w+)\)', open(os.path.join(REPO, path)).read()))


def species_info():
    """type1 and bodyColor per SPECIES_ name"""
    t = open(os.path.join(REPO, 'src/data/pokemon/species_info.h')).read()
    out = {}
    for m in re.finditer(r'\[SPECIES_(\w+)\]\s*=\s*\{(.*?)\n    \}', t, re.S):
        body = m.group(2)
        ty = re.search(r'\.types\s*=\s*\{\s*TYPE_(\w+)', body)
        col = re.search(r'\.bodyColor\s*=\s*BODY_COLOR_(\w+)', body)
        if ty and col:
            out[m.group(1)] = (ty.group(1), col.group(1))
    return out


def stadium_key(name):
    if name.startswith('UNOWN'):
        return 'unown'
    return name.lower().replace('_', '-')


def main():
    consts = species_consts()
    num = consts['EGG']   # NUM_SPECIES (the Unown letters past it only pick pictures; the Pokemon is SPECIES_UNOWN)
    front = table('src/data/pokemon_graphics/front_pic_table.h', 'SPECIES_SPRITE')
    pals = table('src/data/pokemon_graphics/palette_table.h', 'SPECIES_PAL')
    info = species_info()
    stadium = json.load(open(os.path.join(HERE, 'stadium_hues.json')))
    over = json.load(open(os.path.join(HERE, 'skin_overrides.json')))

    lim = {}
    for name, v in consts.items():
        k = stadium_key(name)
        if k in stadium and v < num:
            lim[name] = (stadium[k]['min'], stadium[k]['max'])
    # Gen 3: same first type + body color among the Stadium species
    by_both, by_type = {}, {}
    for name, (mn, mx) in lim.items():
        if name in info:
            ty, col = info[name]
            by_both.setdefault((ty, col), []).append((mn, mx))
            by_type.setdefault(ty, []).append((mn, mx))
    everything = list(lim.values())

    def median(pairs):
        return (int(round(statistics.median(p[0] for p in pairs))), int(round(statistics.median(p[1] for p in pairs))))

    derived = {}
    for name, v in consts.items():
        if v >= num or name in lim or name in ('NONE', 'EGG') or name.startswith('OLD_UNOWN') or name not in info:
            continue
        ty, col = info[name]
        pairs = by_both.get((ty, col)) or by_type.get(ty) or everything
        derived[name] = median(pairs)

    skins = {}
    for name, v in consts.items():
        if v >= num or name in ('NONE', 'EGG') or name.startswith('OLD_UNOWN') or name not in front or name not in pals:
            continue
        pic = front[name][len('gMonFrontPic_'):]
        pal = pals[name][len('gMonPalette_'):]
        pf = os.path.join(G, 'gMonFrontPic_%s.bin' % pic)
        pp = os.path.join(G, 'gMonPalette_%s.bin' % pal)
        if not (os.path.exists(pf) and os.path.exists(pp)):
            continue
        if pic in over:
            s = set(over[pic])
        else:
            s = skin.classify(gba.pal(open(pp, 'rb').read()), gba.pic(open(pf, 'rb').read()))
        skins[name] = sum(1 << k for k in s)

    rows = sorted(((v, n) for n, v in consts.items() if v < num), key=lambda x: x[0])
    seen = set()
    lines = ['// Made by tools/moemon_hues/make_hues.py. Do not edit.',
             '// Stadium hue limits (degrees each way) and the skin slots of Kai\'s pictures, by species.', '',
             'static const u8 sMpHueMin[NUM_SPECIES] = {']
    def emit(arr, fmt):
        out = []
        for v, n in rows:
            if v in seen_rows:
                continue
            seen_rows.add(v)
            if n in arr:
                out.append('    [SPECIES_%s] = %s,' % (n, fmt(arr[n])))
        return out
    allim = dict(lim); allim.update(derived)
    seen_rows = set(); lines += emit(allim, lambda p: p[0]); lines += ['};', '', 'static const u8 sMpHueMax[NUM_SPECIES] = {']
    seen_rows = set(); lines += emit(allim, lambda p: p[1]); lines += ['};', '', 'static const u16 sMpHueSkin[NUM_SPECIES] = {']
    seen_rows = set(); lines += emit(skins, lambda m: '0x%04X' % m); lines += ['};', '']
    open(os.path.join(REPO, 'src/data/mp_hues.h'), 'w').write('\n'.join(lines))
    print('stadium %d, derived %d, skin %d' % (len(lim), len(derived), len(skins)))
    for n in sorted(derived, key=lambda n: consts[n]):
        print('  derived %-12s %s/%s -> %s' % (n, info[n][0], info[n][1], derived[n]))


if __name__ == '__main__':
    main()
