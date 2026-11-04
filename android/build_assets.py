#!/usr/bin/env python3
"""The app's art and tables from the game itself (src/mp/mp_export.c, MOEMON_EXPORT_APP), packed for the APK into
android/app/src/main/assets/game/: sheets of the POKeMON's icons (first frame) and front pictures, the BAG's icons,
the PC wallpapers, the window frames and the font, compressed; data.json; charmap.json (the game's letters, for the
app's own words, from charmap.txt); sounds.bin (the game's sound effects, cries and songs, played by its own engine); fxclips.bin (the ball effects,
played by the game's own editor: src/mp/mp_fxclip.c).

    python3 android/build_assets.py [EXPORT_DIR]   (no EXPORT_DIR: runs ./pokeemerald64 to make one; make linux first)
"""
import json
import os
import re
import shutil
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


def tt_art():
    """TRIPLE TRIAD's card faces for the phone: the game's own pack (graphics/tt/cards.bin, "TTCA") with only the large
    faces (104x128) and the extras (backs, plates, stars, types, digits): the phone draws every size from those"""
    src = open(os.path.join(ROOT, 'graphics/tt/cards.bin'), 'rb').read()
    top, extras = struct.unpack('<II', src[4:12])
    head = 12 + top * 24 + extras * 8
    out = bytearray(head)
    out[0:12] = src[0:12]
    def put(at, off, size):
        nonlocal out
        if size:
            out[at:at + 8] = struct.pack('<II', len(out), size)
            out += src[off:off + size]
    for i in range(top):
        off, size = struct.unpack('<II', src[12 + i * 24:12 + i * 24 + 8])
        put(12 + i * 24, off, size)
    for k in range(extras):
        at = 12 + top * 24 + k * 8
        off, size = struct.unpack('<II', src[at:at + 8])
        put(at, off, size)
    open(os.path.join(OUT, 'tt_cards.bin'), 'wb').write(bytes(out))


def webp_pack(src_name, out_name, quality):
    """A card game's printed pictures (the game's pack: a count, then offset, size and the illustration's x, y, w, h per
    card, then the back) re-encoded as WebP in the same layout (CardPack.java reads it); made once while the game's
    pack is older"""
    import io
    from PIL import Image
    out_path = os.path.join(OUT, out_name)
    src_path = os.path.join(ROOT, src_name)
    if os.path.exists(out_path) and os.path.getmtime(out_path) >= os.path.getmtime(src_path):
        return   # (made already: the WebP encoding takes a while)
    src = open(src_path, 'rb').read()
    count = struct.unpack('<I', src[4:8])[0]
    head = 8 + 16 * (count + 1)
    out = bytearray(src[0:head])
    for i in range(count + 1):
        off, size, x, y, w, h = struct.unpack('<IIHHHH', src[8 + 16 * i:8 + 16 * i + 16])
        if size:
            im = Image.open(io.BytesIO(src[off:off + size])).convert('RGB')
            b = io.BytesIO()
            im.save(b, 'WEBP', quality=quality, method=4)
            data = b.getvalue()
            out[8 + 16 * i:8 + 16 * i + 16] = struct.pack('<IIHHHH', len(out), len(data), x, y, w, h)
            out += data
    open(out_path, 'wb').write(bytes(out))


def tcg_art():
    """The POKéMON cards for the phone: the game's printed cards (graphics/tcg/printed.bin, "TCGP") as WebP: 443
    pictures in about 13 MB instead of 19 (the phone shows them at most a little larger than they are)"""
    webp_pack('graphics/tcg/printed.bin', 'tcg_cards.bin', 78)


def ygo_art():
    """YU-GI-OH! for the phone: the printed cards (graphics/ygo/printed.bin, "YGOP") as WebP (about 19 MB instead of
    28), and EDOPro's pack (graphics/ygo/edo.pak: every card's database row and script) as it is"""
    webp_pack('graphics/ygo/printed.bin', 'ygo_cards.bin', 75)
    shutil.copyfile(os.path.join(ROOT, 'graphics/ygo/edo.pak'), os.path.join(OUT, 'ygo_edo.pak'))


def read_clip(path):
    """A clip as the game records it (mp_fxclip.c): each frame XORed with the one before, as runs of unchanged and
    changed pixels (ABGR1555)"""
    d = open(path, 'rb').read()
    if d[:4] != b'MFXC':
        raise ValueError(path)
    w, h = struct.unpack_from('<HH', d, 4)
    pos, prev, frames = 8, [0] * (w * h), []
    while pos < len(d):
        n, = struct.unpack_from('<I', d, pos)
        words = struct.unpack_from('<%dH' % (n // 2), d, pos + 4)
        pos += 4 + n
        cur, i, k = prev[:], 0, 0
        while k < len(words):
            same, diff = words[k], words[k + 1]
            k += 2
            i += same
            for _ in range(diff):
                cur[i] ^= words[k]
                k += 1
                i += 1
        frames.append(cur)
        prev = cur
    return w, h, frames


def fx_clips(exp_clips):
    """fxclips.bin for the app's POKé BALL page: "MFXA", u16 count, u16 width, u16 height; per clip u8 kind (0
    SEND-OUT, 1 PARTICLES, 2 RETURN), u8 piece, u16 frames, u32 offset, u32 size; then each clip, zlib: its frames at
    30 a second (every second one), each XORed with the one before, u16 ABGR1555 little-endian"""
    names = sorted(f for f in os.listdir(exp_clips) if f.endswith('.clip'))
    entries, blobs, size = [], [], None
    for name in names:
        kind, piece = (int(v) for v in name[:-5].split('_'))
        w, h, frames = read_clip(os.path.join(exp_clips, name))
        size = (w, h)
        frames = frames[::2]
        while len(frames) > 2 and frames[-1] == frames[-2]:
            frames.pop()   # (the rest at the end: once)
        out, prev = bytearray(), [0] * (w * h)
        for f in frames:
            out += struct.pack('<%dH' % (w * h), *[a ^ b for a, b in zip(f, prev)])
            prev = f
        entries.append((kind, piece, len(frames)))
        blobs.append(zlib.compress(bytes(out), 9))
    head = b'MFXA' + struct.pack('<HHH', len(entries), size[0], size[1])
    offset = len(head) + 12 * len(entries)
    table = b''
    for (kind, piece, n), blob in zip(entries, blobs):
        table += struct.pack('<BBHII', kind, piece, n, offset, len(blob))
        offset += len(blob)
    with open(os.path.join(OUT, 'fxclips.bin'), 'wb') as f:
        f.write(head + table + b''.join(blobs))


def main():
    exp = sys.argv[1] if len(sys.argv) > 1 else None
    if exp is None:
        exp = tempfile.mkdtemp(prefix='moemon_export')
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_EXPORT_APP=exp,
                   MOEMON_SAVE=os.path.join(exp, 'x.sav'), MOEMON_SKIP_COOP_SETUP='1',
                   MOEMON_COOP_CONFIG=os.path.join(exp, 'x.ini'))
        subprocess.check_call([os.path.join(ROOT, 'pokeemerald64')], cwd=ROOT, env=env, stdout=subprocess.DEVNULL)
        # the ball effects: the game's editor plays each piece (unthrottled through the test harness: a script that
        # only waits; the game quits when every piece is recorded)
        if not os.environ.get('MOEMON_EXPORT_NO_FXCLIPS'):
            script = os.path.join(exp, 'fxclips.txt')
            open(script, 'w').write('wait 1000000\n')
            env = dict(env, MOEMON_EXPORT_FXCLIPS=os.path.join(exp, 'clips'), MOEMON_TEST=script, MOEMON_SHOTDIR=exp)
            del env['MOEMON_EXPORT_APP']
            subprocess.check_call([os.path.join(ROOT, 'pokeemerald64')], cwd=ROOT, env=env, stdout=subprocess.DEVNULL)
    os.makedirs(OUT, exist_ok=True)
    data = json.load(open(os.path.join(exp, 'data.json')))
    data['charmap'] = charmap()
    data['sheets'] = {'icons': [32, 32, 32], 'front': [64, 64, 16, 256], 'items': [24, 24, 32], 'wallpapers': [160, 144, 4],
                      'frames': [24, 24, 32]}
    json.dump(data, open(os.path.join(OUT, 'data.json'), 'w'), separators=(',', ':'))
    # both art sets (OPTIONS > CREATURES): Kai's MOéMON, and the original POKéMON as front_p_N (the party icons are the
    # same in both)
    write_png(os.path.join(OUT, 'icons.png'), *sheet(load_dir(os.path.join(exp, 'icons')), 32, 32, 32))
    for suffix in ('', '_p'):
        if not os.path.isdir(os.path.join(exp, 'front' + suffix)):
            continue
        front = load_dir(os.path.join(exp, 'front' + suffix))
        for k in range(0, max(front) + 1, 256):
            part = {i - k: v for i, v in front.items() if k <= i < k + 256}
            if part:
                write_png(os.path.join(OUT, 'front%s_%d.png' % (suffix, k // 256)), *sheet(part, 64, 64, 16))
    write_png(os.path.join(OUT, 'items.png'), *sheet(load_dir(os.path.join(exp, 'items')), 24, 24, 32))
    write_png(os.path.join(OUT, 'wallpapers.png'), *sheet(load_dir(os.path.join(exp, 'wallpapers')), 160, 144, 4))
    write_png(os.path.join(OUT, 'frames.png'), *sheet(load_dir(os.path.join(exp, 'frames')), 24, 24, 32))
    write_png(os.path.join(OUT, 'font.png'), *read_png(os.path.join(exp, 'font.png')))
    write_png(os.path.join(OUT, 'font_small.png'), *read_png(os.path.join(exp, 'font_small.png')))
    write_png(os.path.join(OUT, 'types.png'), *read_png(os.path.join(exp, 'types.png')))
    for name in ('dex_info.png', 'dex_list.png', 'dex_list_p.png', 'footprints.png', 'ribbons.png', 'trainers.png'):
        write_png(os.path.join(OUT, name), *read_png(os.path.join(exp, name)))
    # the game's sounds, recorded from its own sound engine (src/mp/mp_export_sound.c; Sound.java plays them)
    if os.path.exists(os.path.join(exp, 'sounds.bin')):
        shutil.copyfile(os.path.join(exp, 'sounds.bin'), os.path.join(OUT, 'sounds.bin'))
    if os.path.isdir(os.path.join(exp, 'clips')):
        fx_clips(os.path.join(exp, 'clips'))
    tt_art()
    tcg_art()
    ygo_art()
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
