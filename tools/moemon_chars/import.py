#!/usr/bin/env python3
"""Moemon Co-op: imports the player characters from Moemon's other branch (the user's sprite zip).

Usage: import.py ZIP_DIR
  ZIP_DIR  the extracted zip: "Player Characters/NNN - Name/", "Trainer Front Pics/front_NNN.png",
           "Trainer Back Pics/back_NNN_Kframes.png" and extraction_report.json

For each character (000 Brendan and 001 May are the game's own, pixel for pixel, so they are skipped):
walking + running frames (16x32), Mach Bike and Acro Bike frames (32x32, one entry per picture-table
entry, as the game's own sheets), diving frames (32x32, the same), the field palette (walking, running
and both bikes share it) and the diving palette. Surfing, field moves, fishing, watering and decorating
are built in game (mp_hybrid.c: Brendan's / May's body with the character's head).

Trainer pictures: front_000..092 are Emerald's own (same order as gTrainerFrontPicTable), and the ones
equal to a FireRed/LeafGreen picture the game already has (graphics/moemon/ext) are skipped; the rest
are new. Back pictures: the ones equal to Emerald's are skipped; 5-frame ones keep frames 0, 1, 3, 4
(the game's order: standing, throw..., after the throw, like Red's and Leaf's).

Writes graphics/moemon/chars/*.bin and src/mp/mp_chars_gfx.h.
"""
import json
import os
import re
import struct
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
OUT = os.path.join(ROOT, 'graphics', 'moemon', 'chars')

# index: (menu name, gender, generic, old ini names that now mean this character)
# generic: a class or a group's member (LOOK > GENERICS) rather than a named character
M, F = 'MALE', 'FEMALE'
CHARS = {
    2: ('ETHAN', M), 3: ('RED', M, False, ['red']), 4: ('ASH', M), 5: ('GLORIA', F), 6: ('DAWN', F),
    7: ('GREEN', F), 8: ('MARNIE', F), 9: ('PROF. OAK', M, False, ['frlg_prof._oak']),
    10: ('GIOVANNI', M, False, ['frlg_giovanni']), 11: ('SELENE', F), 12: ('LEAF', F, False, ['leaf']),
    13: ('ELIO', M), 14: ('LUCAS', M), 15: ('REI', M), 16: ('VICTOR', M), 17: ('STEVEN', M, False, ['steven']),
    18: ('ROCKET GRUNT M', M, True, ['frlg_rocket_grunt_m']), 19: ('MAGMA GRUNT F', F, True, ['magma_grunt_f']),
    20: ('HILBERT', M), 21: ('CALEM', M), 22: ('BLUE', M, False, ['frlg_blue']), 23: ('KRIS', F),
    24: ('CYNTHIA', F), 25: ('DIANTHA', F), 26: ('NURSE JOY', F), 27: ('GEETA', F), 28: ('LANCE', M),
    29: ('LYRA', F), 30: ('N', M), 31: ('ROSA', F), 32: ('NATE', M), 33: ('SERENA', F), 34: ('SILVER', M),
    35: ('ADAMAN', M), 36: ('PROF. BIRCH', M, False, ['prof._birch']), 37: ('BROCK', M), 38: ('COLRESS', M),
    39: ('CYRUS', M), 40: ('JANINE', F), 41: ('KOGA', M), 42: ('SABRINA', F), 43: ('WALLY', M, False, ['wally']),
    44: ('AGATHA', F), 45: ('ALDER', M), 46: ('MAXIE', M, False, ['maxie']), 47: ('AKARI', F),
    48: ('GHETSIS', M), 49: ('MISTY', F), 50: ('HAU', M), 51: ('IRIS', F), 52: ('FLORIAN', M), 53: ('LILLIE', F),
    54: ('GLADION', M), 55: ('HUGH', M), 56: ('BEDE', M), 57: ('JULIANA', F), 58: ('KLARA', F),
    59: ('LUSAMINE', F), 60: ('HILDA', F), 61: ('TIERNO', M), 62: ('LYSANDRE', M), 63: ('JAMES', M),
    64: ('JESSIE', F), 65: ('PENNY', F), 66: ('IONO', F), 67: ('LEON', M), 68: ('NEMONA', F),
    69: ('WALLACE', M, False, ['wallace']), 70: ('KUKUI', M), 71: ('HELENA', F), 72: ('ACEROLA', F),
    73: ('ALAIN', M), 74: ('ALLISTER', M), 75: ('ARCHIE', M, False, ['archie']), 76: ('ARVEN', M), 77: ('AZ', M),
    78: ('BEA', F), 79: ('BIANCA', F), 80: ('BONNIE', F), 81: ('CHLOE', F), 82: ('CLAIR', F), 83: ('CLAVELL', M),
    84: ('COURTNEY', F), 85: ('DRAYDEN', M), 86: ('ELESA', F), 87: ('GUZMA', M), 88: ('IRIDA', F),
    89: ('JACQ', M), 90: ('KIAWE', M), 91: ('KORRINA', F), 92: ('LANA', F), 93: ('LIKO', F),
    94: ('LUCY', F, False, ['lucy']), 95: ('MALLOW', F), 96: ('MARS', F), 97: ('MORTY', M), 98: ('NESSA', F),
    99: ('PIERS', M), 100: ('RAINBOW GRUNT M', M, True), 101: ('RAINBOW GRUNT F', F, True),
    102: ('ROCKET GRUNT F', F, True, ['frlg_rocket_grunt_f']), 103: ('ROXIE', F), 104: ('SADA', F),
    105: ('SHADOW TRIAD', M, True), 106: ('SHELLY', F), 107: ('SONIA', F), 108: ('SOPHOCLES', M),
    109: ('TURO', M), 110: ('WHITNEY', F), 111: ('ZINNIA', F), 112: ('AREZU', F),
}

# Pictures nobody's sheet points at (HeartGold/SoulSilver characters)
FRONT_NAMES = {
    151: 'JESSIE & JAMES', 202: 'ARCHER', 203: 'ARIANA', 205: 'BRUNO', 206: 'KAREN', 207: 'WILL',
    208: 'EUSINE', 209: 'HARLEQUIN', 211: 'KIMONO GIRL', 212: 'BUGSY', 213: 'CHUCK', 214: 'FALKNER',
    215: 'JASMINE', 216: 'PRYCE', 217: 'ELDER', 218: 'PETREL', 219: 'OFFICER', 220: 'PROTON',
    221: 'SAGE', 284: 'BILL',
}

# Mach / Acro Bike sheets that show a ridden Pokemon or a vehicle rather than a bicycle (LOOK > BIKE
# offers RIDE / BIKE for these). Found by comparing each sheet with Brendan's / May's bicycle (shape
# overlap below 0.78), then checked by eye: these drawn bicycles overlap less and are bicycles.
BICYCLE_MACH = {9, 35, 54, 60, 62, 66, 70, 87, 99}
BICYCLE_ACRO = {9, 35, 49, 50, 60, 63, 66, 69, 70, 86, 87, 99}
RIDE_THRESHOLD = 0.78

# Acro Bike frames 11-12, 15-16, 19-20 are the front-wheel hop / wheelie (S, N, W). Most sheets from the
# middle of the pack on left them empty, and these hold Brendan's in the character's colors: they take the
# back-wheel ones (9-10, 13-14, 17-18) instead, as do sheets with any of the six empty.
FRONT_WHEEL = [11, 12, 15, 16, 19, 20]
FRONT_WHEEL_PLACEHOLDERS = {34: [15, 16, 19, 20], 35: FRONT_WHEEL, 36: FRONT_WHEEL, 39: FRONT_WHEEL,
                            43: FRONT_WHEEL, 45: FRONT_WHEEL}

FRAMES = {'walking': (16, 9), 'running': (16, 9), 'mach_bike': (32, 9), 'acro_bike': (32, 27),
          'underwater_diving': (32, 9)}
FIVE_FRAME_ORDER = [0, 1, 3, 4]


def rgb5(px):
    r, g, b, a = px
    return None if a == 0 else (r >> 3, g >> 3, b >> 3)


def load_rgba(path):
    im = Image.open(path)
    if im.mode == 'P':
        pal = im.getpalette()
        idx = list(im.getdata())
        rgba = Image.new('RGBA', im.size)
        rgba.putdata([(0, 0, 0, 0) if v == 0 else (pal[3 * v], pal[3 * v + 1], pal[3 * v + 2], 255) for v in idx])
        return rgba
    return im.convert('RGBA')


def frames(path, fw, n, square=False):
    """n frames fw wide and 32 high (square: fw x fw, fw = the sheet's height: Birch's bikes are 64x64)"""
    im = load_rgba(path)
    w, h = im.size
    if square:
        fw = h
    if w != fw * n or h != (fw if square else 32) or h not in (32, 64):
        sys.exit('%s: %dx%d, expected %d frames of %d' % (path, w, h, n, fw))
    return [[rgb5(p) for p in im.crop((i * fw, 0, i * fw + fw, h)).getdata()] for i in range(n)]


def pics(path, n, fw=64):
    im = load_rgba(path)
    return [[rgb5(p) for p in im.crop((i * fw, 0, i * fw + fw, 64)).getdata()] for i in range(n)]


def make_palette(frame_list, what):
    count = {}
    for fr in frame_list:
        for c in fr:
            if c is not None:
                count[c] = count.get(c, 0) + 1
    if len(count) > 15:
        sys.exit('%s: %d colors (15 fit)' % (what, len(count)))
    # Most used first: stable, and the order the color maps are written in
    cols = sorted(count, key=lambda c: (-count[c], c))
    return cols


def pal_bytes(cols):
    out = bytearray(32)
    for i, (r, g, b) in enumerate(cols):
        struct.pack_into('<H', out, 2 * (i + 1), r | (g << 5) | (b << 10))
    return bytes(out)


def tiles(fr, w, h, cols):
    index = {c: i + 1 for i, c in enumerate(cols)}
    out = bytearray(w * h // 2)
    for ty in range(h // 8):
        for tx in range(w // 8):
            base = (ty * (w // 8) + tx) * 32
            for y in range(8):
                for x in range(8):
                    c = fr[(ty * 8 + y) * w + tx * 8 + x]
                    v = 0 if c is None else index[c]
                    out[base + y * 4 + x // 2] |= v << (4 if x & 1 else 0)
    return bytes(out)


def bike_overlap(a, b):
    """Mean shape overlap of the lower halves (the bicycle) of two sheets' frames."""
    tot = 0.0
    for fa, fb in zip(a, b):
        if len(fa) != len(fb):
            return 0.0   # a 64x64 sheet: not a bicycle
        sa = {(i % 32, i // 32) for i, c in enumerate(fa) if c is not None and i // 32 >= 16}
        sb = {(i % 32, i // 32) for i, c in enumerate(fb) if c is not None and i // 32 >= 16}
        tot += len(sa & sb) / max(1, len(sa | sb))
    return tot / len(a)


def cfg_name(name):
    return 'kai_' + re.sub(r'_+', '_', re.sub(r'[^a-z0-9]', '_', name.lower())).strip('_')


def c_str(s):
    return '"%s"' % s.replace('\\', '\\\\').replace('"', '\\"')


def ext_fronts():
    """FireRed/LeafGreen / Showdown pictures the game already has (graphics/moemon/ext), as pixels."""
    t = open(os.path.join(ROOT, 'graphics/moemon/ext/front_tiles.bin'), 'rb').read()
    p = open(os.path.join(ROOT, 'graphics/moemon/ext/front_pals.bin'), 'rb').read()
    out = set()
    for i in range(len(t) // 2048):
        pal = [struct.unpack_from('<H', p, i * 32 + 2 * k)[0] for k in range(16)]
        px = [None] * 4096
        for tile in range(64):
            tx, ty = tile % 8, tile // 8
            for y in range(8):
                for x in range(8):
                    v = t[i * 2048 + tile * 32 + y * 4 + x // 2]
                    v = (v >> 4) if x & 1 else (v & 15)
                    c = pal[v]
                    px[(ty * 8 + y) * 64 + tx * 8 + x] = None if v == 0 else (c & 31, (c >> 5) & 31, (c >> 10) & 31)
        out.add(tuple(px))
    return out


def emerald_backs():
    out = {}
    d = os.path.join(ROOT, 'graphics/trainers/back_pics')
    order = ['brendan', 'may', 'red', 'leaf', 'brendan_rs', 'may_rs', 'wally', 'steven']   # TRAINER_BACK_PIC_*
    for i, n in enumerate(order):
        im = load_rgba(os.path.join(d, n + '.png'))
        first = tuple(rgb5(p) for p in im.crop((0, 0, 64, 64)).getdata())
        out[first] = i
    return out


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    src = sys.argv[1]
    report = json.load(open(os.path.join(src, 'extraction_report.json')))['characters']
    chdir = os.path.join(src, 'Player Characters')
    folders = {int(n[:3]): os.path.join(chdir, n) for n in os.listdir(chdir) if re.match(r'\d{3} - ', n)}
    if sorted(folders) != list(range(113)) or len(report) != 113:
        sys.exit('expected characters 000-112')
    os.makedirs(OUT, exist_ok=True)

    # ---- trainer pictures
    fdir = os.path.join(src, 'Trainer Front Pics')
    known = ext_fronts()
    new_fronts = []        # (pack index, name)
    front_of_pack = {}     # pack index -> ('E', emerald index) or ('N', new index)
    owners = {}
    for k, c in report.items():
        owners.setdefault(c['front_pic'], int(k))
    ft, fp = bytearray(), bytearray()
    for f in sorted(os.listdir(fdir)):
        n = int(f[6:9])
        if n < 93:
            front_of_pack[n] = ('E', n)
            continue
        px = pics(os.path.join(fdir, f), 1)[0]
        if tuple(px) in known:
            continue   # FireRed/LeafGreen: already in the game
        if n in owners:
            name = CHARS[owners[n]][0] if owners[n] in CHARS else report[str(owners[n])]['name'].upper()
        elif n in FRONT_NAMES:
            name = FRONT_NAMES[n]
        else:
            sys.exit('front_%03d: no name' % n)
        cols = make_palette([px], f)
        front_of_pack[n] = ('N', len(new_fronts))
        new_fronts.append((n, name))
        ft += tiles(px, 64, 64, cols)
        fp += pal_bytes(cols)
    bdir = os.path.join(src, 'Trainer Back Pics')
    eback = emerald_backs()
    new_backs = []
    back_of_char = {}
    bt, bp = bytearray(), bytearray()
    for f in sorted(os.listdir(bdir)):
        m = re.match(r'back_(\d{3})_(\d)frames\.png$', f)
        if not m:
            sys.exit('%s: unexpected name' % f)
        n, k = int(m.group(1)), int(m.group(2))
        fr = pics(os.path.join(bdir, f), k)
        if tuple(fr[0]) in eback:
            back_of_char[n] = ('E', eback[tuple(fr[0])])
            continue
        if n not in CHARS:
            sys.exit('%s: no character' % f)
        order = FIVE_FRAME_ORDER if k == 5 else list(range(4))
        if k not in (4, 5):
            sys.exit('%s: %d frames' % (f, k))
        cols = make_palette([fr[i] for i in order], f)
        back_of_char[n] = ('N', len(new_backs))
        new_backs.append((n, CHARS[n][0]))
        for i in order:
            bt += tiles(fr[i], 64, 64, cols)
        bp += pal_bytes(cols)

    # ---- overworld sheets
    ref_mach = frames(os.path.join(folders[0], 'mach_bike.png'), 32, 9)
    ref_acro = frames(os.path.join(folders[0], 'acro_bike.png'), 32, 27)
    ref_mach_f = frames(os.path.join(folders[1], 'mach_bike.png'), 32, 9)
    ref_acro_f = frames(os.path.join(folders[1], 'acro_bike.png'), 32, 27)
    ot, op = bytearray(), bytearray()
    defs = []
    for n in range(2, 113):
        meta = CHARS[n]
        name, gender = meta[0], meta[1]
        generic = len(meta) > 2 and meta[2]
        aliases = meta[3] if len(meta) > 3 else []
        d = folders[n]
        sheets = {k: frames(os.path.join(d, k + '.png'), *FRAMES[k], square=k in ('mach_bike', 'acro_bike'))
                  for k in FRAMES}
        acro = sheets['acro_bike']
        fix = FRONT_WHEEL if any(all(c is None for c in acro[i]) for i in FRONT_WHEEL) else FRONT_WHEEL_PLACEHOLDERS.get(n, [])
        for i in fix:
            acro[i] = acro[i - 2]
        bike = 64 if len(sheets['mach_bike'][0]) == 64 * 64 else 32
        if len(sheets['acro_bike'][0]) != bike * bike:
            sys.exit('%03d: the bikes differ in size' % n)
        offset = len(ot)
        field = make_palette(sheets['walking'] + sheets['running'] + sheets['mach_bike'] + sheets['acro_bike'],
                             '%03d field' % n)
        dive = make_palette(sheets['underwater_diving'], '%03d dive' % n)
        for k in ('walking', 'running', 'mach_bike', 'acro_bike'):
            w = bike if k in ('mach_bike', 'acro_bike') else 16
            for fr in sheets[k]:
                ot += tiles(fr, w, w if w == 64 else 32, field)
        for fr in sheets['underwater_diving']:
            ot += tiles(fr, 32, 32, dive)
        op += pal_bytes(field) + pal_bytes(dive)
        mach = max(bike_overlap(sheets['mach_bike'], ref_mach), bike_overlap(sheets['mach_bike'], ref_mach_f))
        acro = max(bike_overlap(sheets['acro_bike'], ref_acro), bike_overlap(sheets['acro_bike'], ref_acro_f))
        flags = []
        if mach < RIDE_THRESHOLD and n not in BICYCLE_MACH:
            flags.append('MP_CHARF_MACH_RIDE')
        if acro < RIDE_THRESHOLD and n not in BICYCLE_ACRO:
            flags.append('MP_CHARF_ACRO_RIDE')
        if generic:
            flags.append('MP_CHARF_GENERIC')
        fk = front_of_pack.get(report[str(n)]['front_pic'])
        if fk is None:
            sys.exit('%03d: front_%03d is a FireRed/LeafGreen picture' % (n, report[str(n)]['front_pic']))
        front = 'MP_CHAR_FRONT_EMERALD(%d)' % fk[1] if fk[0] == 'E' else 'MP_CHAR_FRONT_NEW(%d)' % fk[1]
        bk = back_of_char[n]
        back = 'MP_CHAR_BACK_EMERALD(%d)' % bk[1] if bk[0] == 'E' else 'MP_CHAR_BACK_NEW(%d)' % bk[1]
        defs.append((name, cfg_name(name), gender, front, back, flags, aliases, offset, bike))

    for fn, data in (('ow_tiles.bin', ot), ('ow_pals.bin', op), ('front_tiles.bin', ft), ('front_pals.bin', fp),
                     ('back_tiles.bin', bt), ('back_pals.bin', bp)):
        open(os.path.join(OUT, fn), 'wb').write(data)

    h = ['// Moemon Co-op: generated by tools/moemon_chars/import.py, do not edit.',
         '// Player characters from Moemon\'s other branch: their overworld sheets (graphics/moemon/chars/ow_tiles.bin:',
         '// per character 9 walking + 9 running 16x32 frames, 9 Mach Bike and 27 Acro Bike frames (32x32, or 64x64',
         '// for bikeSize 64), 9 diving 32x32 frames;',
         '// ow_pals.bin: the field palette, then the diving one), and the trainer pictures the game didn\'t have.',
         '',
         '#define MP_CHAR_COUNT %d' % len(defs),
         '#define MP_CHAR_FRONT_COUNT %d' % len(new_fronts),
         '#define MP_CHAR_BACK_COUNT %d' % len(new_backs),
         '',
         'static const struct MpCharDef sMpChars[MP_CHAR_COUNT] = {']
    for name, cfg, gender, front, back, flags, aliases, offset, bike in defs:
        h.append('    {%s, %s, %s, %s, %s, %s, %d, %d},' % (c_str(name), c_str(cfg), gender, front, back,
                                                            ' | '.join(flags) if flags else '0', offset, bike))
    h += ['};', '', '// Old ini names of looks these characters replace (saves made before them keep working)',
          'static const char *const sMpCharAliases[][2] = {']
    for d in defs:
        cfg, aliases = d[1], d[6]
        for a in aliases:
            h.append('    {%s, %s},' % (c_str(a), c_str(cfg)))
    h += ['};', '', 'static const char *const sMpCharFrontNames[MP_CHAR_FRONT_COUNT][2] = {']
    seen = set()
    for n, name in new_fronts:
        cfg = cfg_name(name)
        if cfg in seen:
            sys.exit('front name %s twice' % cfg)
        seen.add(cfg)
        h.append('    {%s, %s},' % (c_str(name), c_str(cfg)))
    h += ['};', '', 'static const char *const sMpCharBackNames[MP_CHAR_BACK_COUNT][2] = {']
    for n, name in new_backs:
        h.append('    {%s, %s},' % (c_str(name), c_str(cfg_name(name))))
    h.append('};')
    open(os.path.join(ROOT, 'src/mp/mp_chars_gfx.h'), 'w').write('\n'.join(h) + '\n')
    rides = sum(1 for d in defs if any('RIDE' in f for f in d[5]))
    print('%d characters (%d with a ridden Pokemon or vehicle), %d new trainer pictures, %d new back pictures'
          % (len(defs), rides, len(new_fronts), len(new_backs)))


if __name__ == '__main__':
    main()
