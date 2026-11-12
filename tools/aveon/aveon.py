#!/usr/bin/env python3
"""AveonTrainer's overworld sprites (deviantart.com/aveontrainer/gallery/67900303/overworld: Gen 4 and 5 characters
and trainer classes drawn in FRLG's style) as SINNOH's and UNOVA's people in OPTIONS > DISPLAY > SPRITE SCALING >
SINNOH / UNOVA PEOPLE: MIXED (the user, 2026-11-12): where one of these fits a person, it's theirs; else a true Gen 3
equivalent; else the region's own sprite shrunk. Never the player's own sprite. Credit: dist_extras/AVEON_CREDITS.txt.

The sheets (graphics/aveon/NAME.png, as published: 128x192, RPG Maker's 4 x 4 of 32x48 frames (a cyclist's 48x64),
rows facing down, left, right, up, each pixel drawn 2x2) become the game's nine 32x32 frames (down, up, left
standing, then two steps each), 16x24 a frame at their real size, the standing feet on the packs' FEET_ROW, at most 15 colours.

    python3 tools/aveon/aveon.py fetch DIR     (the gallery's sheets by its RSS feed into DIR, to pick new ones)
    python3 tools/aveon/aveon.py build         (graphics/aveon/frames.bin from the sheets: after changing a map)
    python3 tools/aveon/aveon.py sheet OUT.png (every mapped sprite as the game will show it)
The pack builders (tools/sinnoh/build_gfx.py, tools/unova/build_gfx.py) call art(): the frames and palette of a
Platinum sprite name or a Black object code, or None.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
SHEETS = os.path.join(REPO, 'graphics', 'aveon')
FEET_ROW = 30   # (as tools/sinnoh/build_gfx.py: the standing feet's row in a 32x32 frame)

# Platinum's sprite names (include/constants/sinnoh_gfx.h) -> the sheet
SINNOH = {
    'AARON': 'aaron', 'ARGENTA': 'argenta', 'BARRY': 'barry', 'BERTHA': 'bertha', 'BYRON': 'byron', 'CAITLIN': 'caitlin',
    'CHARON': 'charon', 'CHERYL': 'cheryl', 'CLOWN': 'clown', 'COLLECTOR': 'collector', 'CRASHER_WAKE': 'crasher_wake',
    'CYNTHIA': 'cynthia', 'CYRUS': 'cyrus', 'DAHLIA': 'dahlia', 'DARACH': 'darach', 'FANTINA': 'fantina', 'FLINT': 'flint',
    'GRUNT_F': 'galactic_grunt_f', 'GRUNT_M': 'galactic_grunt_m', 'GUITARIST': 'guitarist', 'JASMINE': 'jasmine',
    'JUPITER': 'jupiter', 'LOOKER': 'looker', 'LUCIAN': 'lucian', 'MAID': 'sinnoh_maid', 'MARLEY': 'marley', 'MARS': 'mars',
    'MIRA': 'mira', 'PALMER': 'palmer', 'POLICEMAN': 'policeman', 'PROF_ROWAN': 'prof_rowan', 'RILEY': 'riley',
    'RUIN_MANIAC': 'ruin_maniac', 'SATURN': 'saturn', 'SOCIALITE': 'socialite', 'THORTON': 'thorton', 'VOLKNER': 'volkner',
    'WAITER': 'waiter', 'WAITRESS': 'waitress', 'SKIER_F': 'skier_f', 'SKIER_M': 'skier_m', 'CYCLIST_M': 'cyclist_m',
    'ACE_TRAINER_SNOW_F': 'snow_ace_trainer_f', 'ACE_TRAINER_SNOW_M': 'snow_ace_trainer_m', 'POKEFAN_M': 'pokefan_m',
    'POKEFAN_F': 'pokefan_f', 'EXPERT_M': 'sinnoh_veteran', 'SCIENTIST_M': 'scientist_m', 'BEAUTY': 'beauty',
    'LADY': 'lady', 'PARASOL_LADY': 'parasol_lady', 'HIKER': 'hiker', 'LASS': 'lass', 'TWIN': 'twin', 'RICH_BOY': 'rich_boy',
    'BATTLE_GIRL': 'battle_girl', 'SWIMMER_F': 'swimmer_f', 'SWIMMER_M': 'swimmer_m', 'POKEMON_BREEDER_F': 'breeder_f',
    'POKEMON_BREEDER_M': 'breeder_m', 'ACE_TRAINER_M': 'ace_trainer_m', 'ACE_TRAINER_F': 'ace_trainer_f',
    'FISHERMAN': 'fisherman', 'POKECENTER_NURSE': 'nurse', 'BLACK_BELT': 'black_belt', 'SCHOOL_KID_F': 'school_girl',
    'JOGGER': 'jogger', 'WORKER': 'worker',
}
# Black's object codes (unova/black_gfx.pak) -> the sheet: its story's people, and its trainers' classes (the class
# of the trainers who use the code: tools/unova/events.py people with trainer scripts)
UNOVA = {
    7: 'cheren', 134: 'bianca', 105: 'prof_juniper', 97: 'alder', 92: 'iris', 100: 'ghetsis', 86: 'lenora', 88: 'elesa',
    103: 'brycen', 93: 'shauntal', 94: 'grimsly', 96: 'caitlin', 102: 'anthea', 193: 'concordia', 215: 'cynthia',
    106: 'looker',
    30: 'ace_trainer_m', 31: 'ace_trainer_f', 46: 'black_belt', 48: 'backpacker_m', 49: 'backpacker_f', 51: 'nurse',
    50: 'doctor', 73: 'scientist_m', 74: 'scientist_f', 98: 'plasma_grunt_m', 99: 'plasma_grunt_f', 60: 'smasher',
    72: 'policeman', 42: 'waiter', 43: 'waitress', 163: 'waiter', 164: 'waiter', 165: 'waiter', 45: 'socialite',
    17: 'lady', 36: 'ranger_m', 37: 'ranger_f', 34: 'breeder_m', 35: 'breeder_f', 24: 'pokefan_m', 26: 'pokefan_f',
    32: 'veteran_m', 33: 'veteran_f', 38: 'cyclist_m', 54: 'clown', 153: 'preschooler_m', 154: 'preschooler_f',
    28: 'painter_m', 15: 'lass', 16: 'school_girl', 9: 'twin', 13: 'rich_boy', 84: 'parasol_lady', 41: 'swimmer_f',
    40: 'swimmer_m', 63: 'fisherman', 64: 'hiker', 67: 'worker', 155: 'worker',
}
# the sheets: graphics/aveon/NAME.png <- the gallery's title (for fetch, and the credits)
SOURCES = {
    'aaron': 'Aaron', 'argenta': 'Argenta', 'barry': 'Barry', 'bertha': 'Bertha', 'byron': 'Byron', 'caitlin': 'Caitlin',
    'charon': 'Charon', 'cheryl': 'Cheryl', 'clown': 'Clown', 'collector': 'Collector', 'crasher_wake': 'Crasher Wake',
    'cynthia': 'Cynthia', 'cyrus': 'Cyrus', 'dahlia': 'Dahlia', 'darach': 'Darach', 'fantina': 'Fantina', 'flint': 'Flint',
    'galactic_grunt_f': 'Galactic Grunt Female', 'galactic_grunt_m': 'Galactic Grunt Male', 'guitarist': 'Guitarist',
    'jasmine': 'Jasmine', 'jupiter': 'Jupiter', 'looker': 'Looker', 'lucian': 'Lucian', 'sinnoh_maid': 'Sinnoh Maid',
    'marley': 'Marley', 'mars': 'Mars', 'mira': 'Mira', 'palmer': 'Palmer', 'policeman': 'Policeman',
    'prof_rowan': 'Pokemon Professor Rowan', 'riley': 'Riley', 'ruin_maniac': 'Ruin Maniac#230', 'saturn': 'Saturn',
    'socialite': 'Socialite', 'thorton': 'Thorton', 'volkner': 'Volkner', 'waiter': 'Waiter', 'waitress': 'Waitress',
    'skier_f': 'Skier (F)', 'skier_m': 'Skier (M)', 'cyclist_m': 'Male Cyclist', 'snow_ace_trainer_f': 'Snow Ace Trainer F',
    'snow_ace_trainer_m': 'Snow Ace Trainer M', 'pokefan_m': 'Poke Fan (M)', 'pokefan_f': 'Poke Fan (F)',
    'sinnoh_veteran': 'Sinnoh Veteran', 'scientist_m': 'Scientist#158', 'scientist_f': 'Unova Scientist (F)',
    'beauty': 'Beauty#164', 'lady': 'Lady#173', 'parasol_lady': 'Parasol Lady#177', 'hiker': 'Hiker', 'lass': 'Lass#218',
    'twin': 'Twin', 'rich_boy': 'Rich Boy#165', 'battle_girl': 'Battle Girl', 'swimmer_f': 'Swimmer Female',
    'swimmer_m': 'Kalos Swimmer (M)', 'breeder_f': 'Female Breeder', 'breeder_m': 'Male Breeder',
    'ace_trainer_m': 'Ace Trainer Male#145', 'ace_trainer_f': 'Female Ace Trainer', 'fisherman': 'Fisher',
    'nurse': 'Nurse', 'black_belt': 'Black Belt', 'school_girl': 'School Girl', 'jogger': 'Pokemon Jogger Male',
    'worker': 'Kalos Worker', 'cheren': 'Cheren', 'bianca': 'Bianca', 'prof_juniper': 'Pokemon Professor Juniper',
    'alder': 'Alder', 'iris': 'Iris', 'ghetsis': 'Ghetsis', 'lenora': 'Lenora', 'elesa': 'Elesa', 'brycen': 'Brycen',
    'shauntal': 'Shauntal', 'grimsly': 'Grimsly', 'anthea': 'Anthea', 'concordia': 'Concordia',
    'backpacker_m': 'Male Backpacker', 'backpacker_f': 'Female Backpacker', 'doctor': 'Doctor',
    'plasma_grunt_m': 'Plasma Grunt Male', 'plasma_grunt_f': 'Plasma Grunt Female', 'smasher': 'Pokemon Trainer Smasher',
    'ranger_m': 'Ranger Male', 'ranger_f': 'Ranger Female', 'veteran_m': 'Kalos Veteran (M)',
    'veteran_f': 'Kalos Veteran (F)', 'clown': 'Clown', 'preschooler_m': 'Preschooler_M', 'preschooler_f': 'Preschooler F',
    'painter_m': 'Kalos Painter (M)',
}
# the game's nine frames from the sheet's (row, column): down, up, left standing; down, up, left stepping twice
FRAMES = [(0, 0), (3, 0), (1, 0), (0, 1), (0, 3), (3, 1), (3, 3), (1, 1), (1, 3)]


def _load(name):
    from PIL import Image
    path = os.path.join(SHEETS, name + '.png')
    if not os.path.exists(path):
        return None
    im = Image.open(path).convert('RGBA')
    w, h = im.size[0] // 8 * 8, im.size[1] // 8 * 8   # (a stray column on some: dropped)
    if w < 64 or h < 96 or w // 8 * 2 > 64 or h // 8 * 2 > 64:
        return None
    im = im.crop((0, 0, w, h))
    return im.resize((w // 2, h // 2), Image.NEAREST)   # (each pixel drawn 2x2: the real size)


DATA = os.path.join(SHEETS, 'frames.bin')
_data = None


def art(region, key):
    """(palette as 16 BGR555 u16, index 0 transparent; nine frames of 32 rows of 32 indices), or None: from
    graphics/aveon/frames.bin (made by `build` from the sheets: the release's runner has no Pillow)"""
    global _data
    import json, zlib
    name = (SINNOH if region == 'sinnoh' else UNOVA).get(key)
    if name is None:
        return None
    if _data is None:
        _data = json.loads(zlib.decompress(open(DATA, 'rb').read())) if os.path.exists(DATA) else {}
    got = _data.get(name)
    if got is None:
        return None
    pal, frames = got['pal'], bytes.fromhex(got['frames'])
    return pal, [[list(frames[f * 1024 + y * 32:f * 1024 + y * 32 + 32]) for y in range(32)] for f in range(9)]


def build(_=None):
    """graphics/aveon/frames.bin: every mapped sheet converted (needs Pillow; committed)"""
    import json, zlib
    out = {}
    for name in sorted(set(SINNOH.values()) | set(UNOVA.values())):
        got = convert(name)
        if got is None:
            print('aveon: %s not converted' % name)
            continue
        pal, frames = got
        out[name] = {'pal': pal, 'frames': bytes(v for rows in frames for r in rows for v in r).hex()}
    open(DATA, 'wb').write(zlib.compress(json.dumps(out, separators=(',', ':')).encode(), 9))
    print('aveon: %d sprites, %d bytes' % (len(out), os.path.getsize(DATA)))


def convert(name):
    """a sheet as the game's nine frames and its palette (Pillow)"""
    im = _load(name)
    if im is None:
        return None
    from PIL import Image
    cells = []
    cw, ch = im.size[0] // 4, im.size[1] // 4   # (16x24 for people; a bike's 24x32)
    for r, c in FRAMES:
        cell = im.crop((c * cw, r * ch, c * cw + cw, r * ch + ch))
        frame = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
        frame.paste(cell, ((32 - cw) // 2, 32 - ch), cell)
        cells.append(frame)
    # at most 15 colours, the same for every frame
    strip = Image.new('RGBA', (32 * 9, 32), (0, 0, 0, 0))
    for k, f in enumerate(cells):
        strip.paste(f, (32 * k, 0))
    opaque = [p[:3] for p in list(strip.getdata()) if p[3] >= 128]
    colours = sorted(set(opaque))
    if len(colours) > 15:
        q = Image.new('RGB', (len(opaque), 1))
        q.putdata(opaque)
        q = q.quantize(15, method=Image.Quantize.MEDIANCUT)
        qp = q.getpalette()[:45]
        colours = [tuple(qp[3 * k:3 * k + 3]) for k in range(15)]
    def nearest(rgb):
        return 1 + min(range(len(colours)), key=lambda k: sum((a - b) ** 2 for a, b in zip(rgb, colours[k])))
    lut = {}
    frames = []
    for f in cells:
        px = f.load()
        rows = [[0] * 32 for _ in range(32)]
        for y in range(32):
            for x in range(32):
                p = px[x, y]
                if p[3] >= 128:
                    rows[y][x] = lut.setdefault(p[:3], nearest(p[:3]))
        frames.append(rows)
    # the standing feet on FEET_ROW
    low = max((y for rows in frames[:3] for y, r in enumerate(rows) if any(r)), default=FEET_ROW)
    dy = FEET_ROW - low
    if dy:
        frames = [[[0] * 32 for _ in range(dy)] + rows[:32 - dy] if dy > 0 else rows[-dy:] + [[0] * 32 for _ in range(-dy)]
                  for rows in frames]
    pal = [0] + [((r >> 3) | (g >> 3) << 5 | (b >> 3) << 10) for r, g, b in colours] + [0] * (15 - len(colours))
    return pal[:16], frames


def fetch(out):
    """the gallery's sheets (all of them, numbered, with items.json) into OUT: to choose new ones by eye"""
    import json, re, urllib.request, xml.etree.ElementTree as ET
    os.makedirs(out, exist_ok=True)
    items = []
    for off in range(0, 2000, 60):
        url = 'https://backend.deviantart.com/rss.xml?type=deviation&q=gallery%3Aaveontrainer%2F67900303&offset=' + str(off)
        root = ET.fromstring(urllib.request.urlopen(url, timeout=30).read())
        got = list(root.iter('item'))
        if not got:
            break
        for it in got:
            c = it.find('{http://search.yahoo.com/mrss/}content')
            items.append({'title': it.findtext('title'), 'link': it.findtext('link'), 'url': c.get('url') if c is not None else None})
    for i, it in enumerate(items):
        if it['url']:
            open(os.path.join(out, '%d.png' % i), 'wb').write(urllib.request.urlopen(it['url'], timeout=30).read())
    json.dump(items, open(os.path.join(out, 'items.json'), 'w'), indent=1)


def sheet(out):
    from PIL import Image, ImageDraw
    keys = [('sinnoh', k) for k in SINNOH] + [('unova', k) for k in UNOVA]
    im = Image.new('RGB', (12 * 72, ((len(keys) + 11) // 12) * 80), (60, 60, 80))
    dr = ImageDraw.Draw(im)
    for n, (region, key) in enumerate(keys):
        a = art(region, key)
        x, y = (n % 12) * 72, (n // 12) * 80
        dr.text((x + 2, y + 2), str(key)[:11], fill=(255, 255, 0))
        if a is None:
            continue
        pal, frames = a
        f = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
        for yy in range(32):
            for xx in range(32):
                v = frames[0][yy][xx]
                if v:
                    c = pal[v]
                    f.putpixel((xx, yy), ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 255))
        f = f.resize((64, 64), Image.NEAREST)
        im.paste(f, (x + 4, y + 14), f)
    im.save(out)


if __name__ == '__main__':
    {'fetch': fetch, 'sheet': sheet, 'build': build}[sys.argv[1]](sys.argv[2] if len(sys.argv) > 2 else None)
