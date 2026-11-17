"""CRYSTAL's EXTENDED party icons (OPTIONS > DISPLAY > MENU POKéMON: EXTENDED): Crystal Clear's own menu POKéMON, one
for each of the 251 and its EGG, in their own colours.

Crystal Clear (ShockSlayer's Crystal hack, 2.5.11) keeps them in a form that reading the ROM alone doesn't find, so they
are taken from the game as it runs, in PyBoy (pip install pyboy): a new game played to the bedroom (the inputs below),
then for six POKéMON at a time the party written into WRAM (wPartyCount $DCD7, wPartySpecies, the party's structs as
pret's Crystal lays them), START > POKéMON opened and each slot's four sprites read for 90 frames: the pixels and
which are see-through from the tiles in VRAM, the colours from the screen. Every icon has two frames.

    python3 tools/ui_styles/cc_icons.py ROM.gbc [OUT.png]     (default: data/crystal_clear_icons.png)

The picture (committed; the ROM never is) is 32 x 16 a POKéMON: frame 1 then frame 2, national numbers 1-251, then
the EGG. gb_build.py reads it with frames().
"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SHEET = os.path.join(HERE, 'data', 'crystal_clear_icons.png')
COUNT = 252   # 1-251 and the EGG

# The new game, from switching on to standing in the bedroom with the starter (Crystal Clear 2.5.11, NORMAL mode,
# NEW BARK TOWN, the default looks and starter, named A): button presses (held 4 frames, 12 apart) and waits
BOOT = """wait 600; a 1; wait 400; start 1; wait 200; a 1; wait 200; start 1; wait 300; start 1; wait 200; a 1; wait 200;
a 1; wait 200; a 1; wait 120; a 1; wait 120; a 1; wait 120; a 1; wait 120; a 1; wait 90; a 1; wait 90; a 1; wait 90;
a 1; wait 90; a 6; wait 60; a 6; wait 60; a 6; wait 60; a 6; wait 60; start 1; wait 60; a 1; wait 90; a 3; wait 90;
a 3; wait 90; b 1; wait 30; right 1; wait 30; right 1; wait 30; right 1; wait 30; right 1; wait 30; a 1; wait 30;
a 1; wait 60; a 1; wait 120; a 3; wait 120; a 1; wait 20; start 1; wait 20; a 1; wait 120; a 4; wait 90; a 4; wait 90;
a 4; wait 90; a 8; wait 90; a 8; wait 90; a 8; wait 90; a 8; wait 120; a 8; wait 120; a 8; wait 120; b 8; wait 120;
b 8; wait 120; a 1; wait 60; a 4; wait 90; a 8; wait 120; a 8; wait 120; a 10; wait 120; a 10; wait 120; a 10;
wait 120; a 10; wait 200; a 30; wait 120; a 30; wait 120; a 30; wait 120; a 30; wait 200; a 100; wait 120; a 100;
wait 120; a 300; wait 120; a 300; wait 120"""

PARTY_COUNT, PARTY_SPECIES, PARTY_MONS, MON_SIZE = 0xDCD7, 0xDCD8, 0xDCDF, 48
OT_NAMES = PARTY_MONS + 6 * MON_SIZE
NICKNAMES = OT_NAMES + 6 * 11
EGG = 0xFD


def _press(pb, button, n=1):
    for _ in range(n):
        pb.button(button, 4)
        pb.tick(12, False)


def _run(pb, script):
    for c in [c.strip() for c in script.replace('\n', ' ').split(';') if c.strip()]:
        p = c.split()
        if p[0] == 'wait':
            pb.tick(int(p[1]), False)
        else:
            _press(pb, p[0], int(p[1]) if len(p) > 1 else 1)


def _party(pb, species):
    """the party: SPECIES, each a copy of the starter's struct with its species"""
    mon = bytes(pb.memory[PARTY_MONS:PARTY_MONS + MON_SIZE])
    nick = bytes(pb.memory[NICKNAMES:NICKNAMES + 11])
    ot = bytes(pb.memory[OT_NAMES:OT_NAMES + 11])
    pb.memory[PARTY_COUNT] = len(species)
    for i, s in enumerate(species):
        pb.memory[PARTY_SPECIES + i] = s
        for k in range(MON_SIZE):
            pb.memory[PARTY_MONS + i * MON_SIZE + k] = mon[k]
        if s != EGG:
            pb.memory[PARTY_MONS + i * MON_SIZE] = s
        for k in range(11):
            pb.memory[NICKNAMES + i * 11 + k] = nick[k]
            pb.memory[OT_NAMES + i * 11 + k] = ot[k]
    pb.memory[PARTY_SPECIES + len(species)] = 0xFF


def _tile(pb, t):
    d = bytes(pb.memory[0x8000 + t * 16:0x8000 + t * 16 + 16])
    return [[((d[2 * y] >> (7 - x)) & 1) | (((d[2 * y + 1] >> (7 - x)) & 1) << 1) for x in range(8)] for y in range(8)]


def _grab(pb, ticks):
    """{slot: {frame's pixels (16 x 16, 0 see-through): {pixel: (r, g, b)}}}: each slot's sprites use OBJ palette
    slot + 1"""
    out = {}
    for _ in range(ticks):
        pb.tick(1, True)
        scr = pb.screen.ndarray
        groups = {}
        for i in range(40):
            s = pb.get_sprite(i)
            if s.on_screen and 1 <= s.attr_palette_number <= 6:
                groups.setdefault(s.attr_palette_number - 1, []).append(s)
        for slot, sp in groups.items():
            if len(sp) != 4:
                continue
            x0, y0 = min(s.x for s in sp), min(s.y for s in sp)
            px = [[0] * 16 for _ in range(16)]
            col = {}
            for s in sp:
                t = _tile(pb, s.tile_identifier)
                for y in range(8):
                    for x in range(8):
                        v = t[7 - y if s.attr_y_flip else y][7 - x if s.attr_x_flip else x]
                        X, Y = s.x - x0 + x, s.y - y0 + y
                        if 0 <= X < 16 and 0 <= Y < 16:
                            px[Y][X] = v
                            sx, sy = s.x + x, s.y + y
                            if v and 0 <= sx < 160 and 0 <= sy < 144:
                                col[v] = tuple(int(c) for c in scr[sy, sx, :3])
            key = tuple(map(tuple, px))
            out.setdefault(slot, {}).setdefault(key, {}).update(col)
    return out


def extract(rom, out_png=SHEET):
    from pyboy import PyBoy
    import io
    pb = PyBoy(rom, window='null', sound_emulated=False)
    pb.set_emulation_speed(0)
    _run(pb, BOOT)
    if pb.memory[PARTY_COUNT] != 1 or pb.memory[PARTY_SPECIES] != 1:
        sys.exit('the new game did not reach the bedroom with the starter (another version of Crystal Clear?)')
    start = io.BytesIO()
    pb.save_state(start)
    species = list(range(1, 252)) + [EGG]
    sheet = Image.new('RGBA', (32, 16 * COUNT), (0, 0, 0, 0))
    for i in range(0, len(species), 6):
        batch = species[i:i + 6]
        start.seek(0)
        pb.load_state(start)
        _party(pb, batch)
        _press(pb, 'start')
        pb.tick(30, False)
        _press(pb, 'down')
        _press(pb, 'a')
        pb.tick(90, False)
        got = _grab(pb, 90)
        for slot, s in enumerate(batch):
            frames = list(got.get(slot, {}).items())
            if len(frames) != 2:
                sys.exit('POKéMON %d: %d frames, not 2' % (s, len(frames)))
            row = i + slot
            for f, (px, col) in enumerate(frames):
                for y in range(16):
                    for x in range(16):
                        if px[y][x]:
                            sheet.putpixel((f * 16 + x, row * 16 + y), col[px[y][x]] + (255,))
        print('icons %d-%d' % (batch[0], batch[-1]), flush=True)
    pb.stop(save=False)
    sheet.save(out_png, optimize=True)
    print('%s: %d icons' % (out_png, COUNT))


def frames(path=SHEET):
    """[(frame 1, frame 2)] RGBA 16 x 16, national numbers 1-251 then the EGG; None without the picture"""
    if not os.path.exists(path):
        return None
    im = Image.open(path).convert('RGBA')
    return [(im.crop((0, n * 16, 16, n * 16 + 16)), im.crop((16, n * 16, 32, n * 16 + 16))) for n in range(COUNT)]


if __name__ == '__main__':
    extract(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else SHEET)
