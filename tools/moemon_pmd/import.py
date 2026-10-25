#!/usr/bin/env python3
"""Moemon Co-op: PMD Collab overworld sprites for followers and Garden Pokémon (mp_pmd.c).

usage: import.py SPRITECOLLAB_DIR [OUT_DIR]
  SPRITECOLLAB_DIR: a checkout of https://github.com/PMDCollab/SpriteCollab (sprite/0001-0386, tracker.json,
  credit_names.txt). OUT_DIR defaults to this repo: graphics/moemon/pmd/pmd.bin, src/data/mp_pmd.h,
  dist_extras/PMD_CREDITS.txt.

Per Pokémon (national dex 1-386; normal, and female where the collab has a female sheet) one chunk, deflated
(raw, no zlib header: the game inflates it with stb_image's zlib):
  u32 'PMD2' | u8 bpp (4, or 8 for the few with more than 15 colors), u8 colors (palette entries, 0 = transparent),
  u8 shadowSize, u8 animCount | u16 palette[colors] | u16 shiny[colors] | padded to 4 | u32 animOffset[animCount]
  anim: u8 frames, u8 dirs (8, or 1 when the sheet has one row), u16 0 | u8 durations[frames] (60ths of a second),
        padded to 4 | u32 frameOffset[dirs * frames] (row = direction: down, down-right, right, up-right, up, up-left,
        left, down-left)
  frame: s16 dx, s16 dy (the picture's top-left from the ground point, the frame's center), u16 w, u16 h,
         then h rows of (w + 1) / 2 bytes, 4 bits a pixel (low nibble first; 8 bpp: w bytes), 0 = transparent
Offsets are from the chunk's start. The shiny palette goes with the same pixels when the normal and shiny sheets put
at most 15 different (normal, shiny) color pairs on the same pixels; otherwise (more pairs, or shapes that differ) the
shiny sheet is a chunk of its own: variant 2 (shiny), 3 (female shiny), whose own palette is its colors.
"""
import os
import struct
import sys
import zlib
import json
import xml.etree.ElementTree as ET
import numpy as np
from PIL import Image

# The order is the game's PMD_ANIM_* (include/mp_pmd.h)
ANIMS = ['Walk', 'Idle', 'Sleep', 'Eat', 'Hop', 'Rotate', 'Attack', 'Shoot', 'Strike', 'LookUp', 'Charge', 'Shock',
         'Trip', 'Tumble', 'Hurt', 'Cringe', 'Nod', 'Pose', 'Sit', 'DeepBreath', 'Laying', 'Float', 'LeapForth', 'Pull',
         'Wake', 'EventSleep', 'Swing', 'Double', 'QuickStrike']
MAX_DEX = 386


def bgr555(rgb):
    r, g, b = rgb
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def read_animdata(d):
    p = os.path.join(d, 'AnimData.xml')
    if not os.path.exists(p):
        return None, {}
    root = ET.parse(p).getroot()
    shadow = int(root.findtext('ShadowSize', '1'))
    anims = {}
    for a in root.find('Anims'):
        name = a.findtext('Name')
        if a.find('CopyOf') is not None:
            anims[name] = ('copy', a.findtext('CopyOf'))
        else:
            anims[name] = (int(a.findtext('FrameWidth')), int(a.findtext('FrameHeight')),
                           [int(x.text) for x in a.find('Durations')])
    return shadow, anims


def resolve(anims, name, depth=0):
    a = anims.get(name)
    if a is None or depth > 8:
        return None, None
    if a[0] == 'copy':
        return resolve(anims, a[1], depth + 1)
    return name, a


class Sheet:
    """One Pokémon's sheets (a folder of <Anim>-Anim.png and AnimData.xml)."""

    def __init__(self, d):
        self.d = d
        self.shadow, self.anims = read_animdata(d)
        self.images = {}

    def ok(self):
        return self.shadow is not None and resolve(self.anims, 'Walk')[0] is not None and \
            os.path.exists(os.path.join(self.d, resolve(self.anims, 'Walk')[0] + '-Anim.png'))

    def image(self, name):
        if name not in self.images:
            p = os.path.join(self.d, name + '-Anim.png')
            self.images[name] = Image.open(p).convert('RGBA') if os.path.exists(p) else None
        return self.images[name]

    def colors(self):
        cols = set()
        for n in ANIMS:
            real, a = resolve(self.anims, n)
            if real is None or self.image(real) is None:
                continue
            for _, (r, g, b, al) in self.image(real).getcolors(1 << 22):
                if al:
                    cols.add((r, g, b))
        return cols


def rgb_keys(arr):
    return (arr[..., 0].astype(np.int64) << 16) | (arr[..., 1].astype(np.int64) << 8) | arr[..., 2]


def shiny_image(normal, shiny, real):
    """The shiny sheet's picture for an animation (the normal one when the shiny sheet hasn't got it)."""
    si = shiny.image(real) if shiny is not None else None
    return si if si is not None else normal.image(real)


def color_pairs(normal, shiny):
    """Every (normal color, shiny color) the two sheets put on the same pixel; None if their shapes differ."""
    pairs = set()
    for n in ANIMS:
        real, a = resolve(normal.anims, n)
        if real is None or normal.image(real) is None:
            continue
        ni, si = normal.image(real), shiny_image(normal, shiny, real)
        if si.size != ni.size:
            return None
        na, sa = np.asarray(ni), np.asarray(si)
        if np.any((na[..., 3] != 0) != (sa[..., 3] != 0)):
            return None
        on = na[..., 3] != 0
        pairs |= set(np.unique((rgb_keys(na)[on] << 24) | rgb_keys(sa)[on]).tolist())
    return sorted(pairs)


def unkey(k):
    return ((k >> 16) & 255, (k >> 8) & 255, k & 255)


def build_chunk(sheet, shiny_sheet, warn):
    """The chunk, and whether the shiny sheet needs a chunk of its own (its colors don't fit beside the normal ones,
    or its shapes differ)."""
    pairs = color_pairs(sheet, shiny_sheet) if shiny_sheet is not None and shiny_sheet.ok() else None
    separate = shiny_sheet is not None and shiny_sheet.ok() and (pairs is None or len(pairs) > 15)
    if pairs is not None and len(pairs) <= 15:
        keys = pairs                                  # a pixel's key: normal color << 24 | shiny color
        pal = [0] + [bgr555(unkey(k >> 24)) for k in keys]
        shiny = [0] + [bgr555(unkey(k & 0xFFFFFF)) for k in keys]
        paired = True
    else:
        cols = sorted(sheet.colors())
        if len(cols) > 255:
            raise SystemExit('%s: %d colors' % (sheet.d, len(cols)))
        keys = [(r << 16) | (g << 8) | b for r, g, b in cols]
        pal = [0] + [bgr555(c) for c in cols]
        shiny = list(pal)
        paired = False
    bpp = 4 if len(pal) <= 16 else 8   # more than 15 colors (a few shiny sheets): a byte a pixel, all of them kept
    key_list = np.array(keys, dtype=np.int64)   # sorted

    body = bytearray()
    anim_offsets = [0] * len(ANIMS)
    pal_bytes = 4 * len(pal)
    head_size = 8 + pal_bytes + (-pal_bytes % 4) + 4 * len(ANIMS)
    records = {}   # real anim name -> offset (CopyOf shares one record)

    def here():
        return head_size + len(body)

    for ai, n in enumerate(ANIMS):
        real, a = resolve(sheet.anims, n)
        if real is None:
            continue
        if real in records:
            anim_offsets[ai] = records[real]
            continue
        im = sheet.image(real)
        if im is None:
            continue
        shiny_im = shiny_image(sheet, shiny_sheet, real) if paired else None
        fw, fh, durs = a
        cols_n = im.width // fw
        rows = im.height // fh
        frames = min(len(durs), cols_n)
        dirs = 8 if rows >= 8 else 1
        if frames == 0:
            continue
        frame_blobs = []
        for r in range(dirs):
            for c in range(frames):
                fr = im.crop((c * fw, r * fh, c * fw + fw, r * fh + fh))
                bb = fr.getbbox()
                if not bb:
                    frame_blobs.append(struct.pack('<hhHH', 0, 0, 0, 0))
                    continue
                pic = fr.crop(bb)
                w, h = pic.size
                a = np.asarray(pic).astype(np.int64)
                keys = rgb_keys(a)
                if paired:
                    sp = shiny_im.crop((c * fw + bb[0], r * fh + bb[1], c * fw + bb[2], r * fh + bb[3]))
                    keys = (keys << 24) | rgb_keys(np.asarray(sp))
                idx = np.zeros((h, w + (w & 1)), dtype=np.uint8)
                pos = np.searchsorted(key_list, keys)
                pos = np.clip(pos, 0, len(key_list) - 1)
                ok = (a[:, :, 3] != 0)
                if np.any(ok & (key_list[pos] != keys)):
                    raise SystemExit('%s: a color outside the palette' % sheet.d)
                idx[:, :w] = np.where(ok, pos + 1, 0)
                if bpp == 4:
                    data = (idx[:, 0::2] | (idx[:, 1::2] << 4)).astype(np.uint8).tobytes()
                else:
                    data = idx[:, :w].astype(np.uint8).tobytes()
                frame_blobs.append(struct.pack('<hhHH', bb[0] - fw // 2, bb[1] - fh // 2, w, h) + bytes(data))
        start = here()
        records[real] = start
        anim_offsets[ai] = start
        rec = bytearray(struct.pack('<BBH', frames, dirs, 0))
        rec += bytes(min(255, max(1, d)) for d in durs[:frames])
        while len(rec) % 4:
            rec.append(0)
        table_pos = len(rec)
        rec += bytes(4 * len(frame_blobs))
        offs = []
        for fb in frame_blobs:
            offs.append(start + len(rec))
            rec += fb
            while len(rec) % 2:
                rec.append(0)
        for i, o in enumerate(offs):
            struct.pack_into('<I', rec, table_pos + 4 * i, o)
        while len(rec) % 4:
            rec.append(0)
        body += rec
    head = struct.pack('<4sBBBB', b'PMD2', bpp, len(pal), min(255, max(0, sheet.shadow)), len(ANIMS))
    head += struct.pack('<%dH' % len(pal), *pal) + struct.pack('<%dH' % len(shiny), *shiny)
    while len(head) % 4:
        head += b'\0'
    head += struct.pack('<%dI' % len(ANIMS), *anim_offsets)
    assert len(head) == head_size
    return bytes(head + body), separate


def deflate(raw):
    c = zlib.compressobj(9, zlib.DEFLATED, -15)
    return c.compress(raw) + c.flush()


def credits_for(repo, dex_dirs, names):
    """Every species' sprite artists, from the collab's credits.txt files."""
    lines = ['PMD Collab overworld sprites (https://sprites.pmdcollab.org, github.com/PMDCollab/SpriteCollab)',
             'Used for followers and Garden Pokemon under CC BY-NC 4.0 (non-commercial, with credit).',
             'Sprite artists for each Pokemon:', '']
    for dex, name, dirs in dex_dirs:
        authors = []
        for d in dirs:
            p = os.path.join(d, 'credits.txt')
            if not os.path.exists(p):
                continue
            for row in open(p, encoding='utf-8'):
                parts = row.rstrip('\n').split('\t')
                if len(parts) >= 2:
                    who = names.get(parts[1]) or parts[1].strip('<@!>')
                    if who and who not in authors:
                        authors.append(who)
        lines.append('#%03d %s: %s' % (dex, name, ', '.join(authors) if authors else 'unknown'))
    return '\n'.join(lines) + '\n'


def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    repo = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
    tracker = json.load(open(os.path.join(repo, 'tracker.json'), encoding='utf-8'))
    names = {}
    cn = os.path.join(repo, 'credit_names.txt')
    if os.path.exists(cn):
        for row in open(cn, encoding='utf-8'):
            parts = row.rstrip('\n').split('\t')
            if len(parts) >= 2 and parts[0] != 'Name':
                names[parts[1]] = parts[0]
    warnings = []
    blob = bytearray()
    index = []
    credit_rows = []
    for dex in range(1, MAX_DEX + 1):
        base = os.path.join(repo, 'sprite', '%04d' % dex)
        name = tracker.get('%04d' % dex, {}).get('name', '?')
        sheet = Sheet(base)
        if not sheet.ok():
            warnings.append('no sprite: #%d %s' % (dex, name))
            continue
        used = [base]
        variants = [(0, sheet, Sheet(os.path.join(base, '0000', '0001')))]
        female = Sheet(os.path.join(base, '0000', '0000', '0002'))
        if female.ok():
            variants.append((1, female, Sheet(os.path.join(base, '0000', '0001', '0002'))))
            used.append(female.d)
        for variant, s, sh in variants:
            chunk, separate = build_chunk(s, sh, lambda m: warnings.append('#%d %s: %s' % (dex, name, m)))
            todo = [(variant, chunk)]
            if separate:   # variant 2 / 3: the shiny (female shiny) sheet on its own
                todo.append((variant + 2, build_chunk(sh, None, None)[0]))
            for v, ch in todo:
                packed = deflate(ch)
                index.append((dex, v, len(blob), len(packed), len(ch)))
                blob += packed
                while len(blob) % 4:
                    blob.append(0)
        credit_rows.append((dex, name, used))
    os.makedirs(os.path.join(out, 'graphics', 'moemon', 'pmd'), exist_ok=True)
    with open(os.path.join(out, 'graphics', 'moemon', 'pmd', 'pmd.bin'), 'wb') as f:
        f.write(blob)
    with open(os.path.join(out, 'src', 'data', 'mp_pmd.h'), 'w') as f:
        f.write('// Generated by tools/moemon_pmd/import.py from the PMD Collab sprites. Do not edit.\n')
        f.write('// dex = national dex number, variant 0 = normal, 1 = female, 2 = shiny, 3 = female shiny (when the\n')
        f.write('// shiny colors need their own pixels); offset / size = the deflated chunk in\n')
        f.write('// graphics/moemon/pmd/pmd.bin, raw = its size inflated.\n')
        f.write('static const struct MpPmdIndex sMpPmdIndex[] = {\n')
        for e in index:
            f.write('    {%d, %d, %d, %d, %d},\n' % e)
        f.write('};\n')
    with open(os.path.join(out, 'dist_extras', 'PMD_CREDITS.txt'), 'w', encoding='utf-8') as f:
        f.write(credits_for(repo, credit_rows, names))
    print('%d chunks, %.1f MB deflated, %.1f MB inflated' % (len(index), len(blob) / 1e6, sum(e[4] for e in index) / 1e6))
    for w in warnings:
        print('warning:', w)


if __name__ == '__main__':
    main()
