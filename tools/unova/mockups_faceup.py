#!/usr/bin/env python3
"""Mock-ups for the user (2026-11-15) of the last two of Black's screens UNOVA lacks, at the GBA's 240x160 with the
game's own font and window frame (tools/unova/mockups.py's helpers):

- the face-up (ov082 fld_faceup.c, FLD_FACEUP 0x1D3-0x1D5: archive 0xBB, a/1/8/7): N's portrait (char 7, screen 11)
  over one of three backgrounds (chars 4-6, screens 8-10, palettes 0-2: NIMBASA's Ferris wheel, his castle's throne
  room, a white light), the DS's 256x192 cropped to the GBA's screen, never scaled; the words in the game's own
  message box;
- the crowd's chatter (ov055 field_gimmick_crowd.c CROWD_OpenChatterSlot): Black's short lines (bank 0x9F) in small
  8x2-tile windows at its sixteen fixed places (gCrowdChatterSlotX / Y), each open for 16 of Black's frames, over
  CASTELIA's street.

    python3 tools/unova/mockups_faceup.py CROWD_SHOT OUT_DIR

CROWD_SHOT: a 480x320 screenshot of CASTELIA's crowds (scenario unova_crowd). Writes faceup_nimbasa.png,
faceup_castle.png, faceup_light.png, crowd_chatter.png at 2x and sheet.png.
"""
import os
import struct
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, 'tools', 'sinnoh'))
import mockups as mk  # noqa: E402
import narc  # noqa: E402
import gimmicks  # noqa: E402
import text as btext  # noqa: E402
from build_journal import section_of  # noqa: E402

W, H = mk.W, mk.H


def _tiles8(ncgr):
    c = section_of(ncgr, b'CHAR')
    size = struct.unpack_from('<I', c, 24)[0]
    raw = c[0x20:0x20 + size]
    return [raw[t * 64:t * 64 + 64] for t in range(len(raw) // 64)]


def _pal(nclr):
    p = section_of(nclr, b'PLTT')
    size, off = struct.unpack_from('<II', p, 16)
    raw = p[8 + off:8 + off + size]
    return [struct.unpack_from('<H', raw, 2 * i)[0] for i in range(len(raw) // 2)]


def _layer(ci, si, pi):
    """an 8bpp BG layer of the face-up archive as an RGBA picture (colour 0 clear)"""
    a = narc.arc('a/1/8/7')
    ts, pal = _tiles8(a[ci]), _pal(a[pi])
    s = section_of(a[si], b'SCRN')
    w, h = struct.unpack_from('<HH', s, 8)
    ents = struct.unpack_from('<%dH' % (struct.unpack_from('<I', s, 16)[0] // 2), s, 20)
    im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    px = im.load()
    for k, e in enumerate(ents):
        t, hf, vf = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1
        if t >= len(ts):
            continue
        for i, v in enumerate(ts[t]):
            if not v:
                continue
            x, y = i % 8, i // 8
            x, y = 7 - x if hf else x, 7 - y if vf else y
            c = pal[v] if v < len(pal) else 0
            px[(k % (w // 8)) * 8 + x, (k // (w // 8)) * 8 + y] = ((c & 31) << 3, (c >> 5 & 31) << 3,
                                                                   (c >> 10 & 31) << 3, 255)
    return im


def message_box(im, lines):
    """the field's message box: the standard frame along the bottom, two lines"""
    mk.window(im, 0, H - 48, 28, 4)
    for k, line in enumerate(lines[:2]):
        mk.text(im, 16, H - 40 + k * 16, line)


def mock_faceup(bg, words):
    """N's face-up over background BG (0-2): the DS picture's middle 240 x 160, his portrait set on the bottom as
    Black sets it, the message box over him"""
    back = _layer(4 + bg, 8 + bg, bg)
    n = _layer(7, 11, 3)
    pic = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
    pic.alpha_composite(back)
    pic.alpha_composite(n.crop((0, 0, 256, 192)))
    im = pic.crop((8, 16, 8 + W, 16 + H)).convert('RGB')
    message_box(im, words)
    return im


def mock_chatter(street, lines):
    """the crowd's chatter: Black's 8x2-tile windows at its slot places (DS tiles, the GBA's 30x20 taken from them,
    the screen's middle kept), three open"""
    im = street.copy()
    img = gimmicks.Image(55)
    xs = [img.d[0x021F45E0 - img.ram + i] for i in range(16)]
    ys = [img.d[0x021F45D0 - img.ram + i] for i in range(16)]
    for slot, line in zip((1, 3, 6), lines):
        x = min(max(xs[slot] * 8 - 8, 0), W - 80)
        y = min(max(ys[slot] * 8 - 16, 0), H - 32)
        mk.window(im, x, y, 8, 2)
        mk.text(im, x + 8 + 2, y + 8, line, font=mk.s2.SMALL if hasattr(mk.s2, 'SMALL') else mk.s2.NORMAL)
    return im


def main():
    shot, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    street = Image.open(shot).convert('RGB').resize((W, H), Image.NEAREST)
    chat = btext.bank(narc.arc('a/0/0/3')[0x9F])
    def lines(bank, msg):   # (the first two lines of one of Black's messages, its codes left out)
        import re
        t = re.sub(r'\{[^}]*\}', '', btext.bank(narc.arc('a/0/0/3')[bank])[msg])
        return [x for x in t.split('\n') if x][:2]
    shots = [('faceup_nimbasa', mock_faceup(0, lines(69, 8))),   # (NIMBASA, zone 64's N)
             ('faceup_castle', mock_faceup(1, lines(301, 1))),   # (N's CASTLE, zone 278)
             ('faceup_light', mock_faceup(2, lines(301, 8))),
             ('crowd_chatter', mock_chatter(street, [chat[29], chat[31], chat[34]]))]
    for name, im in shots:
        im.resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(out, name + '.png'))
    sheet = Image.new('RGB', (2 * (W * 2 + 8) + 8, 2 * (H * 2 + 8) + 8), (32, 32, 40))
    for i, (name, im) in enumerate(shots):
        sheet.paste(im.resize((W * 2, H * 2), Image.NEAREST), (8 + (i % 2) * (W * 2 + 8), 8 + (i // 2) * (H * 2 + 8)))
    sheet.save(os.path.join(out, 'sheet.png'))
    print(out)


if __name__ == '__main__':
    main()
