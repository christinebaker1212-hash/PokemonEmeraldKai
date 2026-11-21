#!/usr/bin/env python3
"""Moemon VMU pet: an edition's data (its MOeMON, names, evolutions, sprites, Eggs) as waterbear source.

usage: gen_edition.py kanto OUT.inc [PREVIEW.png]

From Kai's own tables (include/constants/species.h, src/data/text/species_names.h, src/data/pokemon/species_info.h,
src/data/pokemon/evolution.h) and its PMD Collab sprites (pmd_read.py / onebit.py). KANTO = FireRed's POKeDEX, the
national 1-151. In the edition a MOeMON is its index (0..n-1):
  ed_count                      how many
  ed_hatch_count, ed_hatch      the hatchable ones (the lowest of their line, not UNDISCOVERED, not DITTO)
  ed_species  (2 bytes each)    Kai's species number
  ed_evo_to   (1 each)          what it becomes ($ff = nothing), ed_evo_lv (1 each) at which level
  ed_evo_kind (1 each)          0 by level, 1 a stone / trade / friendship (care: at ed_evo_lv), 2 EEVEE (care picks)
  ed_egg      (1 each)          its Egg's pattern (0-5)
  ed_name     (2 each)          -> a 0-terminated name
  ed_sprite   (2 each)          -> its sprite: a row count h, then h rows of 3 bytes (24 pixels, bit 7 left), the
                                   sprite centred and standing on the bottom row
  egg_pat_N                     the Eggs: 18 rows of 3 bytes each (16x18 in the middle), N = 0-5
Non-level evolutions (the user: "care decides"): stones and friendship at level 30, trades at 36; EEVEE at 25 into
VAPOREON (no care mistakes), FLAREON (trained more than fed) or JOLTEON (the rest).
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, '..', '..')
sys.path.insert(0, HERE)

EDITIONS = {'kanto': list(range(1, 152))}
CARE_LEVEL = {'EVO_ITEM': 30, 'EVO_FRIENDSHIP': 30, 'EVO_FRIENDSHIP_DAY': 30, 'EVO_FRIENDSHIP_NIGHT': 30,
              'EVO_TRADE': 36, 'EVO_TRADE_ITEM': 36}
# An Egg's pattern by the MOeMON's first type (the user's style guide, docs/user_images/96.png: a pattern on each)
PATTERN = {'FIRE': 0, 'DRAGON': 0, 'WATER': 1, 'ICE': 1, 'GRASS': 2, 'BUG': 2, 'ELECTRIC': 3, 'FLYING': 3,
           'POISON': 4, 'GHOST': 4, 'PSYCHIC': 4, 'DARK': 4, 'FAIRY': 5, 'NORMAL': 5,
           'ROCK': 6, 'GROUND': 6, 'FIGHTING': 6, 'STEEL': 6}


def read(p):
    return open(os.path.join(REPO, p), encoding='utf-8', errors='replace').read()


def species_numbers():
    nums = {}
    for m in re.finditer(r'#define SPECIES_(\w+)\s+(\d+)\b', read('include/constants/species.h')):
        nums.setdefault(m.group(1), int(m.group(2)))
    return nums


def names(nums):
    out = {}
    for m in re.finditer(r'\[SPECIES_(\w+)\]\s*=\s*_\("([^"]*)"\)', read('src/data/text/species_names.h')):
        if m.group(1) in nums:
            out[nums[m.group(1)]] = m.group(2)
    return out


def infos(nums):
    txt = read('src/data/pokemon/species_info.h')
    out = {}
    for m in re.finditer(r'\[SPECIES_(\w+)\]\s*=\s*\{((?:(?!\[SPECIES_).)*?)\n\s*\},', txt, re.S):
        if m.group(1) not in nums:
            continue
        body = m.group(2)
        types = re.search(r'\.types\s*=\s*\{\s*TYPE_(\w+)\s*,\s*TYPE_(\w+)', body)
        eggs = re.search(r'\.eggGroups\s*=\s*\{\s*EGG_GROUP_(\w+)\s*,\s*EGG_GROUP_(\w+)', body)
        out[nums[m.group(1)]] = {'types': types.groups() if types else ('NORMAL', 'NORMAL'),
                                 'eggs': eggs.groups() if eggs else ('UNDISCOVERED', 'UNDISCOVERED')}
    return out


def evolutions(nums):
    out = {}
    for chunk in read('src/data/pokemon/evolution.h').split('[SPECIES_')[1:]:
        m = re.match(r'(\w+)\]\s*=', chunk)
        if not m or m.group(1) not in nums:
            continue
        evs = re.findall(r'\{\s*(EVO_\w+)\s*,\s*(\w+)\s*,\s*SPECIES_(\w+)\s*\}', chunk)
        out[nums[m.group(1)]] = [(k, a, nums.get(t)) for k, a, t in evs if t in nums]
    return out


def egg_patterns():
    """Seven 16x18 1-bit Eggs: an outline (the egg's own edge) and a pattern inside, after the user's style guide."""
    W, H = 16, 18
    cx, cy = 7.5, 9.6

    def inside(x, y):
        dx, dy = (x - cx) / 7.6, (y - cy) / (8.9 if y > cy else 8.2)
        return dx * dx + dy * dy <= 1.0

    shape = [[inside(x, y) for x in range(W)] for y in range(H)]

    def edge(x, y):
        if not shape[y][x]:
            return False
        for ax, ay in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if not (0 <= ax < W and 0 <= ay < H) or not shape[ay][ax]:
                return True
        return False

    pats = {
        0: lambda x, y: (y >= 11 and ((x + (y - 11) // 2) % 5 < 2 or y >= 15)),           # flames from below
        1: lambda x, y: (y in (7, 12) and True) or (y in (6, 11) and x % 4 == 1) or (y in (8, 13) and x % 4 == 3),  # waves
        2: lambda x, y: ((x - 4) ** 2 + (y - 6) ** 2 <= 3) or ((x - 11) ** 2 + (y - 12) ** 2 <= 4) or ((x - 5) ** 2 + (y - 13) ** 2 <= 2),  # leaves / spots
        3: lambda x, y: (y - 7) in (0, 1) and x % 6 < 3 or (y - 9) in (0, 1) and (x + 3) % 6 < 3,  # zigzag
        4: lambda x, y: abs(((x - 8) ** 2 + (y - 10) ** 2) ** 0.5 - 3.2) < 0.7 and not (x > 8 and y < 10),  # a swirl
        5: lambda x, y: ((x - 5) ** 2 + (y - 7) ** 2 <= 1.5) or ((x - 10) ** 2 + (y - 11) ** 2 <= 2) or ((x - 6) ** 2 + (y - 14) ** 2 <= 1),  # spots
        6: lambda x, y: y in (8, 9, 13),                                                   # bands
    }
    out = []
    for n in range(7):
        rows = []
        for y in range(H):
            row = 0
            for x in range(W):
                on = edge(x, y) or (shape[y][x] and pats[n](x, y))
                if on:
                    row |= 1 << (23 - (x + 4))  # centred in 24
            rows.append(row)
        out.append(rows)
    return out


def main():
    ed, outp = sys.argv[1], sys.argv[2]
    preview = sys.argv[3] if len(sys.argv) > 3 else None
    dex = EDITIONS[ed]
    nums = species_numbers()
    nm, info, evo = names(nums), infos(nums), evolutions(nums)
    index = {d: i for i, d in enumerate(dex)}
    targets = {t for d in dex for k, a, t in evo.get(d, []) if t in index}
    noegg = lambda g: 'UNDISCOVERED' in g or 'NO_EGGS' in g or g == 'DITTO'  # Kai: EGG_GROUP_NO_EGGS_DISCOVERED
    hatch = [d for d in dex if d not in targets and not any(noegg(g) for g in info[d]['eggs'])]
    from onebit import onebit  # noqa: E402

    L = [';; Generated by vmu_pet/tools/gen_edition.py %s from Kai\'s tables and PMD sprites. Do not edit.' % ed,
         'ED_COUNT = %d' % len(dex), 'ED_HATCH_COUNT = %d' % len(hatch), 'ed_hatch:']
    for i in range(0, len(hatch), 16):
        L.append('  .byte ' + ', '.join(str(index[d]) for d in hatch[i:i + 16]))
    to, lv, kind = [], [], []
    for d in dex:
        ev = [(k, a, t) for k, a, t in evo.get(d, []) if t in index]
        if not ev:
            to.append(255); lv.append(0); kind.append(0)
        elif d == 133:  # EEVEE
            to.append(index[135]); lv.append(25); kind.append(2)
        else:
            k, a, t = ev[0]
            to.append(index[t])
            if k == 'EVO_LEVEL' or k.startswith('EVO_LEVEL'):
                lv.append(int(a) if a.isdigit() else 30); kind.append(0)
            else:
                lv.append(CARE_LEVEL.get(k, 30)); kind.append(1)
    for label, vals in (('ed_evo_to', to), ('ed_evo_lv', lv), ('ed_evo_kind', kind),
                        ('ed_egg', [PATTERN.get(info[d]['types'][0], 5) for d in dex])):
        L.append(label + ':')
        for i in range(0, len(vals), 16):
            L.append('  .byte ' + ', '.join(str(v) for v in vals[i:i + 16]))
    L.append('ed_species:')
    for i in range(0, len(dex), 8):
        L.append('  .word ' + ', '.join(str(d) for d in dex[i:i + 8]))
    L.append('ed_name:')
    for i in range(0, len(dex), 8):
        L.append('  .word ' + ', '.join('nm_%d' % d for d in dex[i:i + 8]))
    L.append('ed_sprite:')
    for i in range(0, len(dex), 8):
        L.append('  .word ' + ', '.join('sp_%d' % d for d in dex[i:i + 8]))
    for d in dex:
        n = re.sub(r'[^A-Z0-9 .\-]', '', nm.get(d, '?').upper().replace('♀', ' F').replace('♂', ' M'))[:11]
        L.append('nm_%d:' % d)
        L.append('  .byte %s, 0' % ', '.join(str(ord(c)) for c in n))
    sheet = []
    for d in dex:
        on = onebit(d, 24)
        h, w = on.shape
        left = (24 - w) // 2
        rows = []
        for y in range(h):
            v = 0
            for x in range(w):
                if on[y, x]:
                    v |= 1 << (23 - (left + x))
            rows.append(v)
        sheet.append(rows)
        L.append('sp_%d:' % d)
        L.append('  .byte %d' % h)
        for y in range(0, h, 4):
            L.append('  .byte ' + ', '.join('$%02x, $%02x, $%02x' % ((v >> 16) & 255, (v >> 8) & 255, v & 255) for v in rows[y:y + 4]))
    eggs = egg_patterns()
    for n, rows in enumerate(eggs):
        L.append('egg_pat_%d:' % n)
        for y in range(0, len(rows), 6):
            L.append('  .byte ' + ', '.join('$%02x, $%02x, $%02x' % ((v >> 16) & 255, (v >> 8) & 255, v & 255) for v in rows[y:y + 6]))
    L.append('egg_pats:')
    L.append('  .word ' + ', '.join('egg_pat_%d' % n for n in range(len(eggs))))
    open(outp, 'w').write('\n'.join(L) + '\n')
    if preview:
        from PIL import Image
        cols = 16
        img = Image.new('L', (cols * 26, ((len(dex) + len(eggs) + cols - 1) // cols) * 26), 200)
        px = img.load()
        for i, rows in enumerate(sheet + [[0] * (24 - len(e)) + e for e in eggs]):
            ox, oy = (i % cols) * 26 + 1, (i // cols) * 26 + 1 + (24 - len(rows))
            for y, v in enumerate(rows):
                for x in range(24):
                    if (v >> (23 - x)) & 1:
                        px[ox + x, oy + y] = 30
        img.resize((img.width * 3, img.height * 3), Image.NEAREST).save(preview)
    print('%s: %d MOeMON, %d hatchable' % (ed, len(dex), len(hatch)))


if __name__ == '__main__':
    main()
