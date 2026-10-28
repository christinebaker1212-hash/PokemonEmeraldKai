#!/usr/bin/env python3
"""Packs the Triple Triad pictures (tools/tt/metadata/, from tools/tt/fetch_cards.py) into graphics/tt/cards.bin:
each card's face at full size (104x128), at the board's size (TT_BOARD_W x TT_BOARD_H, scaled down here with
Lanczos so it stays sharp) and its 40x40 icon; then the extras: the card back at both sizes, the card's backing (the
grey plate that shows the owner's color) at both sizes, the star, the four type icons and the large numbers (0-9 and FF8's A).

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
    """The card back: the user's picture (graphics/tt/back.png, docs/user_images/46), at the card's 104x128."""
    return Image.open(os.path.join(ROOT, 'graphics', 'tt', 'back.png')).convert('RGBA').resize((104, 128), Image.LANCZOS)

DIGIT_W, DIGIT_H = 10, 14
def big_digits():
    """The large numbers 0-9 (graphics/tt/numbers.png, docs/user_images/47: grey on a blue key) and an A drawn in
    their style: an atlas of 11 cells of 10x14, transparent around them."""
    src = Image.open(os.path.join(ROOT, 'graphics', 'tt', 'numbers.png')).convert('RGB')
    key = src.getpixel((0, 0))
    out = Image.new('RGBA', (DIGIT_W * 11, DIGIT_H))
    x, k = 0, 0
    while x < src.width and k < 10:   # each digit: a run of columns that aren't all the key
        if all(src.getpixel((x, y)) == key for y in range(src.height)):
            x += 1
            continue
        x0 = x
        while x < src.width and not all(src.getpixel((x, y)) == key for y in range(src.height)):
            x += 1
        top = min(y for y in range(src.height) if any(src.getpixel((i, y)) != key for i in range(x0, x)))
        for i in range(x0, x):
            for y in range(top, min(top + DIGIT_H, src.height)):
                c = src.getpixel((i, y))
                if c != key:
                    out.putpixel((k * DIGIT_W + (DIGIT_W - (x - x0)) // 2 + i - x0, y - top), c + (255,))
        k += 1
    assert k == 10, k
    # The A: Final Fantasy VIII's own, from its menu sheet (graphics/tt/ff8_menu.png, docs/user_images/48)
    sheet = Image.open(os.path.join(ROOT, 'graphics', 'tt', 'ff8_menu.png')).convert('RGB')
    for y in range(DIGIT_H):
        for x in range(DIGIT_W):
            c = sheet.getpixel((178 + x, 84 + y))
            if c != key:
                out.putpixel((10 * DIGIT_W + x, y), c + (255,))
    return out

def mid_digits():
    """The large numbers at 7x10 for the mid-sized cards (a pack's cards, the spoils): each cell shrunk alone"""
    big = big_digits()
    out = Image.new('RGBA', (7 * 11, 10))
    for k in range(11):
        out.paste(big.crop((k * DIGIT_W, 0, (k + 1) * DIGIT_W, DIGIT_H)).resize((7, 10), Image.LANCZOS), (k * 7, 0))
    return out

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
              frame.crop(STAR)] + [frame.crop(t) for t in TYPES] + [big_digits(), mid_digits()]
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
