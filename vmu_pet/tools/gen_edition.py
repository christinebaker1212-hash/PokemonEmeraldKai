#!/usr/bin/env python3
"""Moemon VMU pet: an edition's data (its MOeMON, names, evolutions, sprites, Eggs: eggs.py) as waterbear source.

usage: gen_edition.py kanto OUT.inc [PREVIEW.png]

From Kai's own tables (include/constants/species.h, src/data/text/species_names.h, src/data/pokemon/species_info.h,
src/data/pokemon/evolution.h) and its PMD Collab sprites (pmd_read.py / onebit.py). KANTO = FireRed's POKeDEX, the
national 1-151. In the edition a MOeMON is its index (0..n-1):
  ed_count                      how many
  ed_hatch_count, ed_hatch      the hatchable ones (the lowest of their line, not UNDISCOVERED, not DITTO)
  ed_species  (2 bytes each)    Kai's species number
  ed_evo_to   (1 each)          what it becomes ($ff = nothing), ed_evo_lv (1 each) at which level
  ed_evo_kind (1 each)          0 by level, 1 a stone / trade / friendship (care: at ed_evo_lv), 2 EEVEE (care picks)
  ed_egg      (1 each)          its Egg (eggs.py): bits 0-3 the design (kind * 2 + phase), bit 4 mirrored
  ed_type1, ed_type2 (1 each)   its types (eggs.TYPES' order), type_chart: 19 x 19 (attacker's row), 0 no effect,
                                1 not very effective, 2 normal, 3 super effective
  ed_egg_rgb  (6 each)          its Egg's colours for the file icon: outline, shell, pattern (ARGB4444, little-endian)
  ed_name     (2 each)          -> a 0-terminated name
  ed_sprite   (2 each)          -> its sprite: a row count h, then h rows of 3 bytes (24 pixels, bit 7 left), the
                                   sprite centred and standing on the bottom row
  egg_pat_N, egg_pats           the 16 Egg designs for the LCD: a sprite like ed_sprite's (20 rows, outline and pattern)
  icon_egg_ol, icon_egg_in      the Egg on the colour icon: 28 rows of 4 bytes (32 pixels), its outline and inside
  icon_egg_pat_N, icon_egg_pats each design's pattern on the icon (28 rows of 4 bytes)
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


# The type chart (Gen 6 on, FAIRY included): attacker -> {defender: multiplier}; MYSTERY (Kai's ???) is neutral
CHART = {
    'NORMAL': {'ROCK': .5, 'GHOST': 0, 'STEEL': .5},
    'FIGHTING': {'NORMAL': 2, 'FLYING': .5, 'POISON': .5, 'ROCK': 2, 'BUG': .5, 'GHOST': 0, 'STEEL': 2,
                 'PSYCHIC': .5, 'ICE': 2, 'DARK': 2, 'FAIRY': .5},
    'FLYING': {'FIGHTING': 2, 'ROCK': .5, 'BUG': 2, 'STEEL': .5, 'GRASS': 2, 'ELECTRIC': .5},
    'POISON': {'POISON': .5, 'GROUND': .5, 'ROCK': .5, 'GHOST': .5, 'STEEL': 0, 'GRASS': 2, 'FAIRY': 2},
    'GROUND': {'FLYING': 0, 'POISON': 2, 'ROCK': 2, 'BUG': .5, 'STEEL': 2, 'FIRE': 2, 'GRASS': .5, 'ELECTRIC': 2},
    'ROCK': {'FIGHTING': .5, 'FLYING': 2, 'GROUND': .5, 'BUG': 2, 'STEEL': .5, 'FIRE': 2, 'ICE': 2},
    'BUG': {'FIGHTING': .5, 'FLYING': .5, 'POISON': .5, 'GHOST': .5, 'STEEL': .5, 'FIRE': .5, 'GRASS': 2,
            'PSYCHIC': 2, 'DARK': 2, 'FAIRY': .5},
    'GHOST': {'NORMAL': 0, 'GHOST': 2, 'PSYCHIC': 2, 'DARK': .5},
    'STEEL': {'ROCK': 2, 'STEEL': .5, 'FIRE': .5, 'WATER': .5, 'ELECTRIC': .5, 'ICE': 2, 'FAIRY': 2},
    'FIRE': {'ROCK': .5, 'BUG': 2, 'STEEL': 2, 'FIRE': .5, 'WATER': .5, 'GRASS': 2, 'ICE': 2, 'DRAGON': .5},
    'WATER': {'GROUND': 2, 'ROCK': 2, 'FIRE': 2, 'WATER': .5, 'GRASS': .5, 'DRAGON': .5},
    'GRASS': {'FLYING': .5, 'POISON': .5, 'GROUND': 2, 'ROCK': 2, 'BUG': .5, 'STEEL': .5, 'FIRE': .5, 'WATER': 2,
              'GRASS': .5, 'DRAGON': .5},
    'ELECTRIC': {'FLYING': 2, 'GROUND': 0, 'WATER': 2, 'GRASS': .5, 'ELECTRIC': .5, 'DRAGON': .5},
    'PSYCHIC': {'FIGHTING': 2, 'POISON': 2, 'STEEL': .5, 'PSYCHIC': .5, 'DARK': 0},
    'ICE': {'FLYING': 2, 'GROUND': 2, 'STEEL': .5, 'FIRE': .5, 'WATER': .5, 'GRASS': 2, 'ICE': .5, 'DRAGON': 2},
    'DRAGON': {'STEEL': .5, 'DRAGON': 2, 'FAIRY': 0},
    'DARK': {'FIGHTING': .5, 'GHOST': 2, 'PSYCHIC': 2, 'DARK': .5, 'FAIRY': .5},
    'FAIRY': {'FIGHTING': 2, 'POISON': .5, 'STEEL': .5, 'FIRE': .5, 'DRAGON': 2, 'DARK': 2},
}


def type_code(a, d):
    m = CHART.get(a, {}).get(d, 1)
    return {0: 0, .5: 1, 1: 2, 2: 3}[m]


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
    import eggs
    designs = [eggs.design(d, info[d]['types']) for d in dex]
    for label, vals in (('ed_evo_to', to), ('ed_evo_lv', lv), ('ed_evo_kind', kind),
                        ('ed_egg', [k * 2 + p + (16 if m else 0) for k, p, m in designs])):
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
    for label, n in (('ed_type1', 0), ('ed_type2', 1)):
        L.append(label + ':')
        vals = [eggs.TYPES.index(info[d]['types'][n]) if info[d]['types'][n] in eggs.TYPES else 0 for d in dex]
        for i in range(0, len(vals), 16):
            L.append('  .byte ' + ', '.join(str(v) for v in vals[i:i + 16]))
    L.append('type_chart:')
    for a in eggs.TYPES:
        L.append('  .byte ' + ', '.join(str(type_code(a, d)) for d in eggs.TYPES))
    L.append('ed_egg_rgb:')
    for d in dex:
        L.append('  .word ' + ', '.join('$%04x' % eggs.to4444(c)[0] for c in eggs.palette(*eggs.body_colours(d))))

    def bits(rows, left, width):
        out = []
        for r in rows:
            v = 0
            for x, on in enumerate(r):
                if on:
                    v |= 1 << (width - 1 - (left + x))
            out.append(v)
        return out
    egg_lcd = []
    for n in range(16):
        ol, sh, pt = eggs.masks(eggs.LCD_W, eggs.LCD_H, n // 2, n % 2)
        rows = bits([[o or q for o, q in zip(a, b)] for a, b in zip(ol, pt)], 2, 24)
        egg_lcd.append(rows)
        L.append('egg_pat_%d:' % n)
        L.append('  .byte %d' % len(rows))
        for y in range(0, len(rows), 5):
            L.append('  .byte ' + ', '.join('$%02x, $%02x, $%02x' % ((v >> 16) & 255, (v >> 8) & 255, v & 255) for v in rows[y:y + 5]))
    L.append('egg_pats:')
    L.append('  .word ' + ', '.join('egg_pat_%d' % n for n in range(16)))

    def rows32(label, rows):
        L.append(label + ':')
        for y in range(0, len(rows), 4):
            L.append('  .byte ' + ', '.join('$%02x, $%02x, $%02x, $%02x' % tuple((v >> s) & 255 for s in (24, 16, 8, 0)) for v in rows[y:y + 4]))
    ol, sh, pt = eggs.masks(eggs.ICON_W, eggs.ICON_H, 0, 0)
    rows32('icon_egg_ol', bits(ol, 2, 32))
    rows32('icon_egg_in', bits(sh, 2, 32))
    for n in range(16):
        ol, sh, pt = eggs.masks(eggs.ICON_W, eggs.ICON_H, n // 2, n % 2)
        rows32('icon_egg_pat_%d' % n, bits(pt, 2, 32))
    L.append('icon_egg_pats:')
    L.append('  .word ' + ', '.join('icon_egg_pat_%d' % n for n in range(16)))
    open(outp, 'w').write('\n'.join(L) + '\n')
    if preview:
        from PIL import Image
        cols = 16
        img = Image.new('L', (cols * 26, ((len(dex) + len(egg_lcd) + cols - 1) // cols) * 26), 200)
        px = img.load()
        for i, rows in enumerate(sheet + egg_lcd):
            ox, oy = (i % cols) * 26 + 1, (i // cols) * 26 + 1 + (24 - len(rows))
            for y, v in enumerate(rows):
                for x in range(24):
                    if (v >> (23 - x)) & 1:
                        px[ox + x, oy + y] = 30
        img.resize((img.width * 3, img.height * 3), Image.NEAREST).save(preview)
    print('%s: %d MOeMON, %d hatchable' % (ed, len(dex), len(hatch)))


if __name__ == '__main__':
    main()
