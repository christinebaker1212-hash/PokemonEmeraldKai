#!/usr/bin/env python3
"""The app's art and tables from the game itself (src/mp/mp_export.c, MOEMON_EXPORT_APP), packed for the APK into
android/app/src/main/assets/game/: sheets of the POKeMON's icons (first frame) and front pictures, the BAG's icons,
the PC wallpapers, the window frames and the font, compressed; data.json; charmap.json (the game's letters, for the
app's own words, from charmap.txt).

    python3 android/build_assets.py [EXPORT_DIR]   (no EXPORT_DIR: runs ./pokeemerald64 to make one; make linux first)
"""
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'android/app/src/main/assets/game')


def read_png(path):
    """RGBA8 PNGs as the game writes them (any filter)"""
    data = open(path, 'rb').read()
    pos, idat, w, h = 8, b'', 0, 0
    while pos < len(data):
        n, kind = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + n]
        if kind == b'IHDR':
            w, h, depth, color = struct.unpack('>IIBB', body[:10])
            assert depth == 8 and color == 6, path
        elif kind == b'IDAT':
            idat += body
        pos += 12 + n
    raw, stride, out, prev = zlib.decompress(idat), w * 4, bytearray(), bytearray(w * 4)
    for y in range(h):
        f, line = raw[y * (stride + 1)], bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = line[i - 4] if i >= 4 else 0
            b = prev[i]
            c = prev[i - 4] if i >= 4 else 0
            if f == 1:
                line[i] = (line[i] + a) & 255
            elif f == 2:
                line[i] = (line[i] + b) & 255
            elif f == 3:
                line[i] = (line[i] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        out += line
        prev = line
    return w, h, out


def write_png(path, w, h, rgba):
    rows = b''.join(b'\x00' + bytes(rgba[y * w * 4:(y + 1) * w * 4]) for y in range(h))

    def chunk(kind, body):
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', zlib.crc32(kind + body))
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(rows, 9)) + chunk(b'IEND', b''))


def sheet(cells, cw, ch, cols):
    """cells: {index: (w, h, rgba)} cropped to cw x ch from the top left, in a grid of cols by index"""
    count = max(cells) + 1 if cells else 1
    rows = (count + cols - 1) // cols
    W, H = cw * cols, ch * rows
    out = bytearray(W * H * 4)
    for i, (w, h, px) in cells.items():
        x0, y0 = (i % cols) * cw, (i // cols) * ch
        for y in range(min(ch, h)):
            src = px[y * w * 4:y * w * 4 + min(cw, w) * 4]
            o = ((y0 + y) * W + x0) * 4
            out[o:o + len(src)] = src
    return W, H, out


def load_dir(d):
    cells = {}
    if os.path.isdir(d):
        for name in os.listdir(d):
            m = re.match(r'(\d+)\.png$', name)
            if m:
                cells[int(m.group(1))] = read_png(os.path.join(d, name))
    return cells


def charmap():
    out = {}
    for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
        m = re.match(r"^'(\\?.)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line)
        if m:
            ch = m.group(1)[-1]   # ('\'' is the apostrophe)
            if ch not in out or ch == "'":
                out[ch] = int(m.group(2), 16)
    return out


def main():
    exp = sys.argv[1] if len(sys.argv) > 1 else None
    if exp is None:
        exp = tempfile.mkdtemp(prefix='moemon_export')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_EXPORT_APP=exp,
                   MOEMON_SAVE=os.path.join(exp, 'x.sav'), MOEMON_SKIP_COOP_SETUP='1',
                   MOEMON_COOP_CONFIG=os.path.join(exp, 'x.ini'))
        subprocess.check_call([os.path.join(ROOT, 'pokeemerald64')], cwd=ROOT, env=env, stdout=subprocess.DEVNULL)
    os.makedirs(OUT, exist_ok=True)
    data = json.load(open(os.path.join(exp, 'data.json')))
    data['charmap'] = charmap()
    data['sheets'] = {'icons': [32, 32, 32], 'front': [64, 64, 16, 256], 'items': [24, 24, 32], 'wallpapers': [160, 144, 4],
                      'frames': [24, 24, 32]}
    json.dump(data, open(os.path.join(OUT, 'data.json'), 'w'), separators=(',', ':'))
    write_png(os.path.join(OUT, 'icons.png'), *sheet(load_dir(os.path.join(exp, 'icons')), 32, 32, 32))
    front = load_dir(os.path.join(exp, 'front'))
    for k in range(0, max(front) + 1, 256):
        part = {i - k: v for i, v in front.items() if k <= i < k + 256}
        if part:
            write_png(os.path.join(OUT, 'front_%d.png' % (k // 256)), *sheet(part, 64, 64, 16))
    write_png(os.path.join(OUT, 'items.png'), *sheet(load_dir(os.path.join(exp, 'items')), 24, 24, 32))
    write_png(os.path.join(OUT, 'wallpapers.png'), *sheet(load_dir(os.path.join(exp, 'wallpapers')), 160, 144, 4))
    write_png(os.path.join(OUT, 'frames.png'), *sheet(load_dir(os.path.join(exp, 'frames')), 24, 24, 32))
    write_png(os.path.join(OUT, 'font.png'), *read_png(os.path.join(exp, 'font.png')))
    write_png(os.path.join(OUT, 'types.png'), *read_png(os.path.join(exp, 'types.png')))
    for name in ('dex_info.png', 'dex_list.png', 'footprints.png', 'ribbons.png', 'trainers.png'):
        write_png(os.path.join(OUT, name), *read_png(os.path.join(exp, name)))
    # the town maps (the live map): KANTO's and the Sevii Islands' pages are narrower than the full view, and black
    # past their right edge: see-through there, so the device's own background shows
    page = 0
    while os.path.exists(os.path.join(exp, 'townmap_%d.png' % page)):
        w, h, px = read_png(os.path.join(exp, 'townmap_%d.png' % page))
        if 2 <= page <= 5:
            px = bytearray(px)
            for i in range(0, len(px), 4):
                if px[i] == 0 and px[i + 1] == 0 and px[i + 2] == 0:
                    px[i + 3] = 0
        write_png(os.path.join(OUT, 'townmap_%d.png' % page), w, h, bytes(px))
        page += 1
    total = sum(os.path.getsize(os.path.join(OUT, f)) for f in os.listdir(OUT))
    print('app assets: %s (%d KB)' % (OUT, total // 1024))


if __name__ == '__main__':
    main()
