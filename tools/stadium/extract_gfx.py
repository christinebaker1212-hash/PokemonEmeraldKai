#!/usr/bin/env python3
"""POKéMON STADIUM's own pictures for the STADIUM's screens (the user: "Graphics for the STADIUM SELECTION will be taken
from Stadium directly"), from the user's ROM (never committed: keep it in ~/.cache/moemon/stadium_dl).

    python3 tools/stadium/extract_gfx.py ROM.z64 graphics/stadium/stadium.bin [PREVIEW_DIR]

The asset packs at pret/pokestadium's yamls/us/rom.yaml offsets (stadium_select_ui 0x5046B0, cup_ball_select_ui
0x5FFC30): PERS-SZP = a header then Yay0 (tools/stadium/yay0.py), the pictures RGBA 5551, big endian:
  - the cups' pictures: four 128 x 96 (two 128 x 192 sheets): POKé CUP (STARMIE / TAUROS), PRIME CUP (ELECTRODE /
    JOLTEON), PETIT CUP (SHELLDER / GEODUDE), PIKA CUP (MR. MIME / RATICATE)
  - the classes' banners: four 256 x 64: POKé BALL, GREAT BALL, ULTRA BALL, MASTER BALL
  - the areas' name signs (area_select_ui 0x4D1330, at 0x6E60, every 0x6200): ten 196 x 64: STADIUM, FREE BATTLE,
    GYM LEADER CASTLE, VS MEWTWO, KIDS CLUB, VICTORY PALACE, POKéMON LAB, GB TOWER, DODUO / DODRIO GB TOWER
  - four 256 x 96 name plates (the packs at 0x45B660, 0x461600, 0x46DD80, 0x49BD40)
  - the pictures the user sent for the cups STADIUM 2 adds (docs/user_images 71, 72), from graphics/stadium/*.png, kept
    at 160 x 120: picture 22 the CHALLENGE CUP, 23 the GYM LEADER CASTLE (a dark stand-in until its file is here)
stadium.bin: "STDM", u16 count, then per picture u16 w, u16 h, w x h u16 pixels (RGBA 5551, little endian)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import yay0  # noqa: E402


def asset(rom, start, end):
    b = rom[start:end]
    assert b[:8] == b'PERS-SZP', hex(start)
    return yay0.persszp(b)


def main():
    rom = open(sys.argv[1], 'rb').read()
    select = asset(rom, 0x5046B0, 0x52ABA0)
    balls = asset(rom, 0x5FFC30, 0x613110)
    pics = []
    for sheet in (0, 81920 + 96):   # (the second sheet starts 48 pixels in: read from 81920 its pictures wrap round)
        for half in range(2):
            off = sheet + half * 128 * 96 * 2
            pics.append((128, 96, select[off:off + 128 * 96 * 2]))
    for i in range(4):
        off = i * 256 * 64 * 2
        pics.append((256, 64, balls[off:off + 256 * 64 * 2]))
    area = asset(rom, 0x4D1330, 0x4EB0C0)
    for i in range(10):
        off = 0x6E60 + i * 0x6200
        pics.append((196, 64, area[off:off + 196 * 64 * 2]))
    for start in (0x45B660, 0x461600, 0x46DD80, 0x49BD40):
        plate = asset(rom, start, start + 0x8000)
        pics.append((256, 96, plate[:256 * 96 * 2]))
    from PIL import Image
    here = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'graphics', 'stadium')
    for name in ('challenge_cup.png', 'gym_leader_castle.png'):
        path = os.path.join(here, name)
        im = Image.open(path).convert('RGB').resize((160, 120), Image.LANCZOS) if os.path.exists(path) \
            else Image.new('RGB', (160, 120), (0, 0, 0))
        px = b''.join(struct.pack('>H', (r >> 3) << 11 | (g >> 3) << 6 | (b >> 3) << 1 | (1 if os.path.exists(path) else 0))
                      for r, g, b in im.getdata())
        pics.append((160, 120, px))
    out = bytearray(b'STDM') + struct.pack('<H', len(pics))
    for w, h, px in pics:
        out += struct.pack('<HH', w, h)
        out += b''.join(struct.pack('<H', struct.unpack('>H', px[i:i + 2])[0]) for i in range(0, len(px), 2))
    os.makedirs(os.path.dirname(sys.argv[2]), exist_ok=True)
    open(sys.argv[2], 'wb').write(out)
    if len(sys.argv) > 3:
        from PIL import Image
        os.makedirs(sys.argv[3], exist_ok=True)
        for n, (w, h, px) in enumerate(pics):
            im = Image.new('RGBA', (w, h))
            im.putdata([(((v >> 11) & 31) * 255 // 31, ((v >> 6) & 31) * 255 // 31, ((v >> 1) & 31) * 255 // 31,
                         255 if v & 1 else 0) for v in struct.unpack('>%dH' % (w * h), px)])
            im.save(os.path.join(sys.argv[3], 'pic%d.png' % n))
    print('%d pictures, %d bytes' % (len(pics), len(out)))


if __name__ == '__main__':
    main()
