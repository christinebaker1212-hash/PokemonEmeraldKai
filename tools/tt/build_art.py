#!/usr/bin/env python3
"""Packs the Triple Triad pictures (tools/tt/metadata/, from tools/tt/fetch_cards.py) into graphics/tt/cards.bin:
each card's face at full size (104x128), at the board's size (TT_BOARD_W x TT_BOARD_H, scaled down here with
Lanczos so it stays sharp) and its 40x40 icon; then the extras: the card back at both sizes, the card's backing (the
grey plate that shows the owner's color) at both sizes, the star and the four type icons.

File: "TTCA", u32 highest id, u32 extras; per id from 1: 3 x (u32 offset, u32 size); per extra (u32, u32); PNGs."""
import io, json, os, struct
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
META = os.path.join(HERE, 'metadata')
BOARD = (40, 49)          # src/tt/tt_gfx.h TT_BOARD_W / TT_BOARD_H
# The frame texture's pieces (CardTripleTriad_hr1: twice the card's size)
BACKING = (5, 2, 203, 254)
STAR = (2, 258, 28, 285)
TYPES = [(79, 258, 117, 298), (124, 258, 160, 298), (168, 258, 204, 298), (34, 258, 73, 298)]   # PRIMAL, SCION, SOCIETY, GARLEAN

def card_back():
    """The card back: XIV's own is only a placeholder, so a back of our own, drawn at four times the size and scaled
    down: a deep blue card in the card frame's shape, a gold border, a lattice and three interlocked rings."""
    import math
    from PIL import ImageDraw
    S = 4
    W, H = 104 * S, 128 * S
    frame = Image.open(os.path.join(META, 'frame.png')).convert('RGBA').crop(BACKING).resize((W, H), Image.LANCZOS)
    mask = frame.split()[3]
    im = Image.new('RGBA', (W, H))
    d = ImageDraw.Draw(im)
    for y in range(H):   # a gradient, lighter at the top
        t = y / H
        d.line([(0, y), (W, y)], fill=(int(28 + 20 * (1 - t)), int(40 + 30 * (1 - t)), int(92 + 50 * (1 - t)), 255))
    step = 14 * S   # the lattice
    for k in range(-H, W + H, step):
        d.line([(k, 0), (k + H, H)], fill=(70, 96, 170, 255), width=S)
        d.line([(k, H), (k + H, 0)], fill=(70, 96, 170, 255), width=S)
    m = 7 * S   # the gold borders
    d.rounded_rectangle([m, m, W - m, H - m], radius=6 * S, outline=(216, 176, 96, 255), width=3 * S)
    d.rounded_rectangle([m + 5 * S, m + 5 * S, W - m - 5 * S, H - m - 5 * S], radius=4 * S,
                        outline=(150, 112, 50, 255), width=S)
    cx, cy, r = W / 2, H / 2, 26 * S   # three interlocked rings in the middle
    d.ellipse([cx - r - 6 * S, cy - r - 6 * S, cx + r + 6 * S, cy + r + 6 * S], fill=(20, 28, 70, 255),
              outline=(216, 176, 96, 255), width=2 * S)
    for k in range(3):
        a = math.radians(90 + k * 120)
        rx, ry, rr = cx + math.cos(a) * r * 0.36, cy - math.sin(a) * r * 0.36, r * 0.52
        d.ellipse([rx - rr, ry - rr, rx + rr, ry + rr], outline=(232, 196, 104, 255), width=3 * S)
    d.ellipse([cx - 3 * S, cy - 3 * S, cx + 3 * S, cy + 3 * S], fill=(255, 236, 170, 255))
    im.putalpha(mask)
    return im.resize((104, 128), Image.LANCZOS)

def png(im):
    b = io.BytesIO()
    im.save(b, 'PNG', optimize=True)
    return b.getvalue()

def main():
    cards = json.load(open(os.path.join(HERE, 'cards.json')))
    top = max(c['id'] for c in cards)
    frame = Image.open(os.path.join(META, 'frame.png')).convert('RGBA')
    back = card_back()
    backing = frame.crop(BACKING).resize((104, 128), Image.LANCZOS)
    extras = [back, back.resize(BOARD, Image.LANCZOS), backing, backing.resize(BOARD, Image.LANCZOS),
              frame.crop(STAR)] + [frame.crop(t) for t in TYPES]
    blobs, index = [], {}
    for c in cards:
        big = Image.open(os.path.join(META, 'big_%d.png' % c['id'])).convert('RGBA')
        small = Image.open(os.path.join(META, 'small_%d.png' % c['id'])).convert('RGBA')
        index[c['id']] = (png(big), png(big.resize(BOARD, Image.LANCZOS)), png(small))
    head = 12 + top * 24 + len(extras) * 8
    out = bytearray(b'TTCA' + struct.pack('<II', top, len(extras)))
    data = bytearray()
    for i in range(1, top + 1):
        for k in range(3):
            if i in index:
                out += struct.pack('<II', head + len(data), len(index[i][k]))
                data += index[i][k]
            else:
                out += struct.pack('<II', 0, 0)
    for e in extras:
        b = png(e)
        out += struct.pack('<II', head + len(data), len(b))
        data += b
    assert len(out) == head
    path = os.path.join(ROOT, 'graphics', 'tt', 'cards.bin')
    open(path, 'wb').write(out + data)
    print(path, len(out) + len(data), 'bytes')

if __name__ == '__main__':
    main()
