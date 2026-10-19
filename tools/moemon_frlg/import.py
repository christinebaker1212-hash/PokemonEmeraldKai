#!/usr/bin/env python3
"""Moemon Co-op: imports extra player looks into graphics/moemon/ext.

  * FireRed/LeafGreen overworld sprites that walk in all four directions
  * FireRed/LeafGreen trainer pictures and back pictures
  * FireRed/LeafGreen text window frames
  * Pokemon Showdown trainer sprites (play.pokemonshowdown.com/sprites/trainers)

Usage: import.py FRLG_DIR SHOWDOWN_DIR
  FRLG_DIR      a pret/pokefirered checkout (graphics/, src/data/)
  SHOWDOWN_DIR  png/*.png from the Showdown trainers folder and view.html
                (the "?view=sprites" page, for the artist credits)

Nothing that Emerald already has is imported twice (pixel compared), and
Showdown sprites that look the same as another one are skipped.
Writes graphics/moemon/ext/*.bin and src/mp/mp_ext_gfx.h.
"""
import glob
import html
import os
import re
import sys

import numpy as np
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
OUT = os.path.join(ROOT, 'graphics', 'moemon', 'ext')


# ---------------------------------------------------------------------------
# Helpers

def read_jasc(path):
    lines = open(path).read().split('\n')
    n = int(lines[2])
    cols = []
    for line in lines[3:3 + n]:
        r, g, b = (int(x) for x in line.split()[:3])
        cols.append((r, g, b))
    while len(cols) < 16:
        cols.append((0, 0, 0))
    return cols[:16]


def gba_pal(cols):
    out = bytearray()
    for r, g, b in cols:
        r, g, b = int(r), int(g), int(b)
        v = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
        out += bytes((v & 0xFF, v >> 8))
    return bytes(out)


def to_4bpp(idx, w, h):
    """idx: 2D numpy array (h, w) of indices 0-15 -> GBA 4bpp tiles, row-major tiles."""
    out = bytearray()
    for ty in range(h // 8):
        for tx in range(w // 8):
            t = idx[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8]
            for y in range(8):
                for x in range(0, 8, 2):
                    out.append((int(t[y, x]) & 0xF) | ((int(t[y, x + 1]) & 0xF) << 4))
    return bytes(out)


def indexed(path):
    im = Image.open(path)
    if im.mode != 'P':
        raise ValueError('%s is not indexed' % path)
    return np.array(im), im.getpalette()[:48]


def rgb_of(idx, cols):
    """RGBA image of an index array, index 0 transparent."""
    h, w = idx.shape
    out = np.zeros((h, w, 4), np.uint8)
    for i in range(1, 16):
        m = idx == i
        if m.any():
            r, g, b = cols[i]
            out[m] = (r & 0xF8, g & 0xF8, b & 0xF8, 255)
    return out


def trim(rgba):
    a = rgba[:, :, 3] > 0
    if not a.any():
        return rgba[:0, :0]
    ys, xs = np.where(a)
    return rgba[ys.min():ys.max() + 1, xs.min():xs.max() + 1]


def same_picture(a, b, tol=0.04):
    """a, b: trimmed RGBA arrays. True when they are the same sprite."""
    if abs(a.shape[0] - b.shape[0]) > 2 or abs(a.shape[1] - b.shape[1]) > 2:
        return False
    h = min(a.shape[0], b.shape[0])
    w = min(a.shape[1], b.shape[1])
    a = a[:h, :w]
    b = b[:h, :w]
    ma = a[:, :, 3] > 0
    mb = b[:, :, 3] > 0
    union = (ma | mb).sum()
    if union == 0:
        return True
    if (ma != mb).sum() / union > tol * 2:
        return False
    both = ma & mb
    diff = np.abs(a[:, :, :3].astype(int) - b[:, :, :3].astype(int)).max(axis=2)
    # Colors differ between games a little; shape and shading matter
    return (diff[both] > 48).sum() / max(1, both.sum()) < 0.25


def _labels(img):
    """Color ids for an RGBA (h, w, 4) or index (h, w) array; 0 = transparent."""
    if img.ndim == 2:
        return img.astype(np.int32)
    flat = img[:, :, 0].astype(np.int32) << 16 | img[:, :, 1].astype(np.int32) << 8 | img[:, :, 2]
    lab = np.zeros(flat.shape, np.int32)
    vis = img[:, :, 3] > 0
    _, inv = np.unique(flat[vis], return_inverse=True)
    lab[vis] = inv.reshape(-1) + 1
    return lab


def _agree(x, y):
    """How consistently colors of x map to colors of y (1.0 = same drawing, any palette)."""
    m = (x > 0) | (y > 0)
    pairs = x[m] * 4096 + y[m]
    vals, counts = np.unique(pairs, return_counts=True)
    best = {}
    for v, c in zip(vals, counts):
        a = v // 4096
        best[a] = max(best.get(a, 0), c)
    return sum(best.values()) / max(1, m.sum())


def same_drawing(a, b, threshold=0.92, shift=2):
    """a, b: trimmed RGBA (or index) arrays. True when they're the same drawing,
    even recolored, moved by a pixel or two, or mirrored."""
    la, lb0 = _labels(a), _labels(b)
    for lb in (lb0, lb0[:, ::-1]):
        if abs(la.shape[0] - lb.shape[0]) > shift * 2 or abs(la.shape[1] - lb.shape[1]) > shift * 2:
            continue
        H = max(la.shape[0], lb.shape[0]) + shift * 2
        W = max(la.shape[1], lb.shape[1]) + shift * 2
        ca = np.zeros((H, W), np.int32)
        ca[shift:shift + la.shape[0], shift:shift + la.shape[1]] = la
        for dy in range(0, shift * 2 + 1):
            for dx in range(0, shift * 2 + 1):
                cb = np.zeros((H, W), np.int32)
                if dy + lb.shape[0] > H or dx + lb.shape[1] > W:
                    continue
                cb[dy:dy + lb.shape[0], dx:dx + lb.shape[1]] = lb
                ma, mb = ca > 0, cb > 0
                if (ma & mb).sum() < 0.9 * (ma | mb).sum():
                    continue
                if min(_agree(ca, cb), _agree(cb, ca)) >= threshold:
                    return True
    return False


def shape_key(rgba):
    """Gray 16x16 thumbnail of a trimmed sprite, for fast duplicate checks."""
    t = trim(rgba)
    if t.size == 0:
        return np.zeros((16, 16))
    im = Image.fromarray(t, 'RGBA')
    bg = Image.new('RGBA', im.size, (255, 255, 255, 255))
    bg.alpha_composite(im)
    g = np.array(bg.convert('L').resize((16, 16), Image.BILINEAR), float)
    return g


def c_name(s):
    return re.sub(r'[^A-Za-z0-9]+', '_', s).strip('_')


def menu_name(s, n=20):
    s = s.upper().replace('_', ' ').replace('-', ' ')
    s = re.sub(r'\s+', ' ', s).strip()
    return s[:n]


# ---------------------------------------------------------------------------
# Emerald pictures (for duplicate checks)

def emerald_fronts():
    pics = []
    for png in sorted(glob.glob(os.path.join(ROOT, 'graphics/trainers/front_pics/*.png'))):
        stem = os.path.basename(png)[:-4]
        idx, p = indexed(png)
        cols = [tuple(p[i * 3:i * 3 + 3]) for i in range(16)]
        pics.append((stem, trim(rgb_of(idx[:64, :64] & 0xF, cols))))
    return pics


def emerald_overworld_frames():
    frames = []
    for png in glob.glob(os.path.join(ROOT, 'graphics/object_events/pics/**/*.png'), recursive=True):
        im = Image.open(png)
        if im.mode != 'P':
            continue
        a = np.array(im)
        h = a.shape[0]
        for w in (16, 32):
            if a.shape[1] >= w:
                frames.append((png, a[:h, :w].copy()))
    return frames


# ---------------------------------------------------------------------------
# FRLG overworld sprites

PAL_TAG_FILES = {
    'OBJ_EVENT_PAL_TAG_NPC_BLUE': 'npc_blue.pal',
    'OBJ_EVENT_PAL_TAG_NPC_PINK': 'npc_pink.pal',
    'OBJ_EVENT_PAL_TAG_NPC_GREEN': 'npc_green.pal',
    'OBJ_EVENT_PAL_TAG_NPC_WHITE': 'npc_white.pal',
    'OBJ_EVENT_PAL_TAG_PLAYER_RED': 'player.pal',
    'OBJ_EVENT_PAL_TAG_PLAYER_GREEN': 'player.pal',
    'OBJ_EVENT_PAL_TAG_RS_QUINTY_PLUMP': 'rs_quinty_plump.pal',
    'OBJ_EVENT_PAL_TAG_METEORITE': 'meteorite.pal',
    'OBJ_EVENT_PAL_TAG_SEAGALLOP': 'seagallop.pal',
    'OBJ_EVENT_PAL_TAG_SS_ANNE': 'ss_anne.pal',
}

# FRLG overworld sprite -> FRLG trainer picture (file stem), and gender
OW_FRONT = {
    'Youngster': ('youngster', 'M'), 'BugCatcher': ('bug_catcher', 'M'), 'Lass': ('lass', 'F'),
    'Camper': ('camper', 'M'), 'Picnicker': ('picnicker', 'F'), 'Hiker': ('hiker', 'M'),
    'Fisher': ('fisherman', 'M'), 'Sailor': ('sailor', 'M'), 'BlackBelt': ('black_belt', 'M'),
    'Beauty': ('beauty', 'F'), 'Gentleman': ('gentleman', 'M'), 'RichBoy': ('rich_boy', 'M'),
    'CooltrainerM': ('cool_trainer_m', 'M'), 'CooltrainerF': ('cool_trainer_f', 'F'),
    'Biker': ('biker', 'M'), 'Channeler': ('channeler', 'F'), 'CrushGirl': ('crush_girl', 'F'),
    'Rocker': ('guitarist', 'M'), 'PokeManiac': ('pokemaniac', 'M'), 'Scientist': ('scientist', 'M'),
    'RocketM': ('rocket_grunt_m', 'M'), 'RocketF': ('rocket_grunt_f', 'F'),
    'SwimmerMLand': ('swimmer_m', 'M'), 'SwimmerFLand': ('swimmer_f', 'F'),
    'TuberMLand': ('tuber_m', 'M'), 'TuberF': ('tuber_f', 'F'),
    'ProfOak': ('professor_oak', 'M'), 'Blue': ('rival_late', 'M'), 'Giovanni': ('leader_giovanni', 'M'),
    'Brock': ('leader_brock', 'M'), 'Misty': ('leader_misty', 'F'), 'LtSurge': ('leader_lt_surge', 'M'),
    'Erika': ('leader_erika', 'F'), 'Koga': ('leader_koga', 'M'), 'Sabrina': ('leader_sabrina', 'F'),
    'Blaine': ('leader_blaine', 'M'), 'Lorelei': ('elite_four_lorelei', 'F'), 'Bruno': ('elite_four_bruno', 'M'),
    'Agatha': ('elite_four_agatha', 'F'), 'Lance': ('elite_four_lance', 'M'),
    'BaldingMan': ('cue_ball', 'M'), 'Boy': (None, 'M'), 'LittleBoy': (None, 'M'), 'LittleGirl': (None, 'F'),
    'Woman1': (None, 'F'), 'Woman2': (None, 'F'), 'Woman3': (None, 'F'), 'Man': (None, 'M'),
    'OldMan1': (None, 'M'), 'OldMan2': (None, 'M'), 'OldWoman': (None, 'F'), 'FatMan': (None, 'M'),
    'Bill': (None, 'M'), 'Celio': (None, 'M'), 'Daisy': (None, 'F'), 'MrFuji': (None, 'M'),
    'Mom': (None, 'F'), 'Nurse': (None, 'F'), 'Clerk': (None, 'M'), 'Policeman': (None, 'M'),
    'WorkerM': (None, 'M'), 'WorkerF': (None, 'F'), 'Chef': (None, 'M'), 'Captain': (None, 'M'),
    'GymGuy': (None, 'M'), 'Cameraman': (None, 'M'), 'GBAKid': (None, 'M'), 'TeachyTVHost': (None, 'M'),
    'TrainerTowerDude': (None, 'M'), 'MGDeliveryman': (None, 'M'), 'UnionRoomReceptionist': (None, 'F'),
    'CableClubReceptionist': (None, 'F'), 'UnusedMaleReceptionist': (None, 'M'), 'UnusedMan': (None, 'M'),
    'UnusedWoman': (None, 'F'), 'SittingBoy': (None, 'M'),
}

OW_NAMES = {
    'PokeManiac': 'POKEMANIAC', 'CooltrainerM': 'COOLTRAINER M', 'CooltrainerF': 'COOLTRAINER F',
    'RocketM': 'ROCKET GRUNT M', 'RocketF': 'ROCKET GRUNT F', 'SwimmerMLand': 'SWIMMER M',
    'SwimmerFLand': 'SWIMMER F', 'TuberMLand': 'TUBER M', 'TuberF': 'TUBER F', 'ProfOak': 'PROF. OAK',
    'LtSurge': 'LT. SURGE', 'MrFuji': 'MR. FUJI', 'GBAKid': 'GBA KID', 'TeachyTVHost': 'TEACHY TV HOST',
    'TrainerTowerDude': 'TOWER GUIDE', 'MGDeliveryman': 'DELIVERYMAN', 'UnionRoomReceptionist': 'UNION CLERK',
    'CableClubReceptionist': 'CABLE CLUB CLERK', 'UnusedMaleReceptionist': 'RECEPTIONIST M',
    'UnusedMan': 'MAN 2', 'UnusedWoman': 'WOMAN 4', 'BaldingMan': 'BALDING MAN', 'FatMan': 'BIG MAN',
    'OldMan1': 'OLD MAN', 'OldMan2': 'OLD MAN 2', 'Woman1': 'WOMAN', 'Woman2': 'WOMAN 2', 'Woman3': 'WOMAN 3',
    'Fisher': 'FISHER', 'WorkerM': 'WORKER M', 'WorkerF': 'WORKER F', 'GymGuy': 'GYM GUIDE',
    'Blue': 'BLUE', 'LittleBoy': 'LITTLE BOY', 'LittleGirl': 'LITTLE GIRL', 'SittingBoy': 'BOY 2',
    'BlackBelt': 'BLACK BELT', 'BugCatcher': 'BUG CATCHER', 'RichBoy': 'RICH BOY', 'CrushGirl': 'CRUSH GIRL',
}


# Bikes and swimming-in-water sprites aren't walking looks; RS Brendan/May are bases already
OW_SKIP = {'RedBike', 'GreenBike', 'RedBikeAlt', 'RSBrendan', 'RSMay', 'SwimmerMWater', 'SwimmerFWater', 'TuberMWater'}


def split_camel(s):
    return re.sub(r'(?<=[a-z])(?=[A-Z0-9])', ' ', s).upper()


def frlg_overworld(frlg, emerald_frames):
    infos_src = open(os.path.join(frlg, 'src/data/object_events/object_event_graphics_info.h')).read()
    pics_src = open(os.path.join(frlg, 'src/data/object_events/object_event_pic_tables.h')).read()
    gfx_src = open(os.path.join(frlg, 'src/data/object_events/object_event_graphics.h')).read()

    gfx = dict(re.findall(r'gObjectEventPic_(\w+)\[\] = INCBIN_U(?:16|32)\("([^"]+)\.4bpp"\)', gfx_src))
    tables = {}
    for name, body in re.findall(r'sPicTable_(\w+)\[\] = \{(.*?)\};', pics_src, re.S):
        tables[name] = re.findall(r'overworld_frame\(gObjectEventPic_(\w+), (\d+), (\d+), (\d+)\)', body)

    out = []
    for name, body in re.findall(r'gObjectEventGraphicsInfo_(\w+) = \{(.*?)\};', infos_src, re.S):
        f = dict(re.findall(r'\.(\w+) = ([^,]+),', body))
        if f.get('anims') != 'sAnimTable_Standard' or name in OW_SKIP:
            continue
        table = tables.get(f['images'].replace('sPicTable_', ''))
        if not table or len(table) < 9:
            continue
        pic, w, h = table[0][0], int(table[0][1]) * 8, int(table[0][2]) * 8
        if any(t[0] != pic for t in table[:9]) or pic not in gfx:
            continue
        frames_idx = [int(t[3]) for t in table[:9]]
        # Four directions (down, up, left; right is mirrored) and walking frames
        if len(set(frames_idx[0:3])) < 3:
            continue
        walk = frames_idx[3:9]
        if any(walk[i] in frames_idx[0:3] for i in range(6)):
            continue
        palf = PAL_TAG_FILES.get(f['paletteTag'])
        if not palf:
            continue
        png = os.path.join(frlg, gfx[pic] + '.png')
        idx, _ = indexed(png)
        per_row = idx.shape[1] // w
        if max(frames_idx) >= per_row * (idx.shape[0] // h):
            continue
        frames = []
        for n in frames_idx:
            fx, fy = (n % per_row) * w, (n // per_row) * h
            frames.append(idx[fy:fy + h, fx:fx + w] & 0xF)
        # Walking frames must really move
        if all((frames[3 + i] == frames[(i // 2)]).all() for i in range(6)):
            continue
        # Already in Emerald?
        dup = None
        for epng, ef in emerald_frames:
            if ef.shape == frames[0].shape and ((ef == frames[0]).all() or same_drawing(ef & 0xF, frames[0], 0.95, 0)):
                dup = epng
                break
        if dup:
            print('  skip %-24s same as %s' % (name, os.path.relpath(dup, ROOT)))
            continue
        cols = read_jasc(os.path.join(frlg, 'graphics/object_events/palettes', palf))
        front, gender = OW_FRONT.get(name, (None, 'M'))
        out.append({
            'name': OW_NAMES.get(name, split_camel(name)),
            'key': name, 'w': w, 'h': h,
            'tiles': b''.join(to_4bpp(fr, w, h) for fr in frames),
            'pal': gba_pal(cols), 'front': front, 'female': gender == 'F',
        })
    return out


# ---------------------------------------------------------------------------
# FRLG trainer pictures and back pictures

FRONT_NAMES = {
    'rival_early': 'BLUE (EARLY)', 'rival_late': 'BLUE', 'champion_rival': 'CHAMPION BLUE',
    'professor_oak': 'PROF. OAK', 'leader_lt_surge': 'LT. SURGE', 'cue_ball': 'CUE BALL',
    'rocket_grunt_m': 'ROCKET GRUNT M', 'rocket_grunt_f': 'ROCKET GRUNT F', 'leaf': 'LEAF (FRLG)',
    'red': 'RED (FRLG)', 'pokemon_ranger_m': 'RANGER M', 'pokemon_ranger_f': 'RANGER F',
}


def front_menu_name(stem, prefix=''):
    if stem in FRONT_NAMES:
        return FRONT_NAMES[stem]
    s = re.sub(r'^(leader|elite_four)_', '', stem)
    s = s.replace('cool_trainer', 'cooltrainer').replace('pokemon_', '')
    s = re.sub(r'_m$', ' M', s)
    s = re.sub(r'_f$', ' F', s)
    return menu_name(prefix + s)


FRLG_FRONT_DUPS = {}


def emerald_pic_ids():
    """Emerald front pic file stem -> TRAINER_PIC_ number."""
    consts = dict(re.findall(r'#define TRAINER_PIC_(\w+)\s+(\d+)', open(os.path.join(ROOT, 'include/constants/trainers.h')).read()))
    tables = open(os.path.join(ROOT, 'src/data/trainer_graphics/front_pic_tables.h')).read()
    gfx = open(os.path.join(ROOT, 'src/data/graphics/trainers.h')).read()
    sym_file = dict(re.findall(r'(gTrainerFrontPic_\w+)\[\] = INC\w+\("graphics/trainers/front_pics/(\w+)\.', gfx))
    out = {}
    for const, sym in re.findall(r'TRAINER_SPRITE\((\w+), (gTrainerFrontPic_\w+)', tables):
        if sym in sym_file and const in consts:
            out[sym_file[sym]] = int(consts[const])
    return out


def frlg_fronts(frlg, known):
    out = []
    for png in sorted(glob.glob(os.path.join(frlg, 'graphics/trainers/front_pics/*.png'))):
        stem = os.path.basename(png)[:-len('_front_pic.png')]
        pal = os.path.join(frlg, 'graphics/trainers/palettes', stem + '.pal')
        if not os.path.exists(pal):
            continue
        idx, _ = indexed(png)
        idx = idx[:64, :64] & 0xF
        cols = read_jasc(pal)
        rgba = rgb_of(idx, cols)
        t = trim(rgba)
        dup = next((k for k, pic in known if same_picture(t, pic) or same_drawing(t, pic)), None)
        if dup:
            print('  skip front %-24s same as %s' % (stem, dup))
            FRLG_FRONT_DUPS[stem] = dup
            continue
        known.append(('frlg ' + stem, t))
        out.append({'stem': stem, 'name': front_menu_name(stem), 'artist': '',
                    'tiles': to_4bpp(idx, 64, 64), 'pal': gba_pal(cols), 'recolor': True,
                    'key': shape_key(rgba)})
    return out


def frlg_backs(frlg):
    out = []
    for stem, name in (('old_man', 'OLD MAN'), ('pokedude', 'POKE DUDE')):
        png = os.path.join(frlg, 'graphics/trainers/back_pics/%s_back_pic.png' % stem)
        pal = os.path.join(frlg, 'graphics/trainers/palettes/%s.pal' % stem)
        if not os.path.exists(pal):
            pal = os.path.join(frlg, 'graphics/trainers/palettes/%s_back_pic.pal' % stem)
        idx, _ = indexed(png)
        idx = idx & 0xF
        frames = idx.shape[0] // 64
        tiles = b''.join(to_4bpp(idx[i * 64:i * 64 + 64, :64], 64, 64) for i in range(4))
        if frames < 4:
            tiles += to_4bpp(idx[0:64, :64], 64, 64) * (4 - frames)
        out.append({'name': name, 'tiles': tiles, 'pal': gba_pal(read_jasc(pal))})
    return out


# ---------------------------------------------------------------------------
# Window frames

def png_rgb_set(path):
    im = Image.open(path).convert('RGBA')
    return np.array(im)


def frlg_frames(frlg):
    em = [png_rgb_set(p) for p in glob.glob(os.path.join(ROOT, 'graphics/text_window/*.png'))]
    out = []
    for i in range(1, 11):
        png = os.path.join(frlg, 'graphics/text_window/type%d.png' % i)
        pal = os.path.join(frlg, 'graphics/text_window/type%d.pal' % i)
        if not os.path.exists(png):
            continue
        im = Image.open(png)
        idx = np.array(im) & 0xF
        if os.path.exists(pal):
            cols = read_jasc(pal)
        else:
            p = im.getpalette()[:48]
            cols = [tuple(p[j * 3:j * 3 + 3]) for j in range(16)]
        rgba = rgb_of(idx, cols)
        if any(e.shape == rgba.shape and (np.abs(e.astype(int) - rgba.astype(int)).max() <= 8) for e in em):
            print('  skip window frame type%d (same as Emerald)' % i)
            continue
        out.append({'name': 'FRLG %d' % i, 'tiles': to_4bpp(idx, 24, 24), 'pal': gba_pal(cols)})
    return out


# ---------------------------------------------------------------------------
# Showdown trainer sprites

def showdown_credits(view_html):
    credits = {}
    src = open(view_html, encoding='utf-8', errors='replace').read()
    for fig in re.findall(r'<figure.*?</figure>', src, re.S):
        m = re.search(r'src="[^"]*?([^"/]+)\.png"', fig)
        if not m:
            continue
        cap = re.search(r'<figcaption>(.*?)</figcaption>', fig, re.S)
        artist = ''
        if cap:
            parts = re.split(r'<br\s*/?>', cap.group(1))
            for p in parts[1:]:
                p = html.unescape(re.sub(r'<[^>]+>', '', p)).strip()
                if p.lower().startswith('by '):
                    artist = p[3:].strip()
        credits[m.group(1)] = artist
    return credits


def quantize15(rgba):
    """Index array (0 = transparent) and 15 colors."""
    a = rgba[:, :, 3] >= 128
    rgb = rgba[:, :, :3].copy()
    rgb[~a] = 0
    # 5-bit colors, like the GBA
    rgb = (rgb & 0xF8)
    cols = {}
    for c in map(tuple, rgb[a]):
        cols[c] = cols.get(c, 0) + 1
    if len(cols) > 15:
        im = Image.fromarray(np.dstack([rgb, np.full(a.shape, 255, np.uint8)]).astype(np.uint8), 'RGBA')
        flat = Image.new('RGB', im.size)
        flat.paste(im.convert('RGB'))
        # Quantize only the visible pixels
        vis = np.array(flat)[a].reshape(-1, 1, 3)
        q = Image.fromarray(vis.astype(np.uint8), 'RGB').quantize(15, method=Image.MEDIANCUT, dither=Image.NONE)
        qpal = q.getpalette()[:45]
        qi = np.array(q).reshape(-1)
        idx = np.zeros(a.shape, np.uint8)
        idx[a] = qi + 1
        palette = [(0, 0, 0)] + [tuple(qpal[i * 3:i * 3 + 3]) for i in range(15)]
        return idx, palette, True
    order = sorted(cols, key=lambda c: -cols[c])
    lut = {c: i + 1 for i, c in enumerate(order)}
    idx = np.zeros(a.shape, np.uint8)
    ys, xs = np.where(a)
    for y, x in zip(ys, xs):
        idx[y, x] = lut[tuple(rgb[y, x])]
    palette = [(0, 0, 0)] + order + [(0, 0, 0)] * (15 - len(order))
    return idx, palette, False


def place64(idx):
    """Centers a trimmed sprite at the bottom of a 64x64 picture (feet on the ground)."""
    h, w = idx.shape
    out = np.zeros((64, 64), np.uint8)
    x0 = (64 - w) // 2
    y0 = 64 - h
    out[y0:y0 + h, x0:x0 + w] = idx
    return out


SD_WORDS = set('''ace trainer couple snow aether employee foundation aqua grunt suit aroma lady artist backers
backpacker ball guy battle girl bird keeper black belt body builder bug catcher maniac cafe master cue crush kin
dragon tamer fairy tale fire breather fighter fisherman free diver furisode gentleman hex interviewers jr kimono
league staff magma office worker old pokemon pokefan poke kid breeder center ranger preschooler preschoolers punk
rail rainbow rocket executive rising star roller skater ruin rich boy school kid sis and bro sr super nerd scuba
security corps sky team triathlete biker runner swimmer young athlete flare galactic plasma skull yell clan member
diamond pearl depot agent double ogre officer parasol street thug sightseer tourist trial guide mr mrs police man
nursery aide medium model musician owner painter pilot postman reporter rollerskater rough neck ruffian sailor
scientist skier smasher striker surfer teacher tuber twins veteran waiter waitress worker ice youngster cyclist
delinquent dancer doctor gambler gamer gardener golfer harlequin hiker hoopster idol infielder janitor jogger
juggler kindler linebacker madame maid pokemaniac psychic rancher shadow triad teammates the royal todd snap
ultra forest karten voy cowgirl cook courier clown clerk channeler collector camper beauty baker bellhop butler
cabbie caretaker engineer expert lass picnicker picknicker coin burglar cameraman champion
policeman roughneck lt surge jessie james'''.split())

# The same art as an Emerald/FRLG picture, rearranged (the checks can't see it)
SD_SKIP = {'acetrainercouple-gen3'}

SD_TAGS = {'gen1': 'G1', 'gen1rb': 'G1 RB', 'gen2': 'G2', 'gen2jp': 'G2 JP', 'gen3': 'G3', 'gen3rs': 'G3 RS',
           'gen3jp': 'G3 JP', 'gen4': 'G4', 'gen4dp': 'DP', 'gen4pt': 'PT', 'gen5bw': 'BW', 'gen5bw2': 'BW2',
           'gen6': 'G6', 'gen6xy': 'XY', 'gen6oras': 'ORAS', 'gen7': 'G7', 'gen8': 'G8', 'gen9': 'G9'}


def split_words(base):
    best = {0: []}
    for i in range(1, len(base) + 1):
        for j in range(max(0, i - 14), i):
            if j in best and base[j:i] in SD_WORDS:
                cand = best[j] + [base[j:i]]
                if i not in best or len(cand) < len(best[i]):
                    best[i] = cand
    return best.get(len(base))


def showdown_name(stem):
    parts = stem.split('-')
    base, tags = parts[0], parts[1:]
    tail = ''
    m = re.match(r'^(.*?)(f|m)(\d?)$', base)
    words = split_words(base)
    if not words and m and split_words(m.group(1)):
        words = split_words(m.group(1)) + [m.group(2) + m.group(3)]
    name = ' '.join(words) if words else base
    for t in tags:
        pre = max((k for k in SD_TAGS if t.startswith(k)), key=len, default=None)
        if pre:
            tail += ' ' + SD_TAGS[pre] + (' ' + t[len(pre):] if t[len(pre):] else '')
        else:
            tail += ' ' + re.sub(r'(\D)(\d)', r'\1 \2', t)
    return (name + tail).upper()[:24]


def showdown(sd_dir, known, known_keys):
    credits = showdown_credits(os.path.join(sd_dir, 'view.html'))
    out = []
    skipped = {'credited': 0, 'size': 0, 'dup': 0}
    keys = list(known_keys)
    for png in sorted(glob.glob(os.path.join(sd_dir, 'png', '*.png'))):
        stem = os.path.basename(png)[:-4]
        if stem.startswith('unknown') or stem.endswith('-flipped') or stem in SD_SKIP:
            continue
        artist = credits.get(stem, '')
        rgba = np.array(Image.open(png).convert('RGBA'))
        rgba[rgba[:, :, 3] < 128] = 0
        t = trim(rgba)
        if t.size == 0:
            continue
        h, w = t.shape[:2]
        if w > 64:
            skipped['size'] += 1
            continue
        if artist:
            # Fan art: used only as drawn (no cropping, color changes or recoloring)
            if h > 64:
                skipped['credited'] += 1
                continue
            idx, palette, changed = quantize15(t)
            if changed:
                skipped['credited'] += 1
                continue
        else:
            if h > 64:
                # Taller than a GBA trainer picture: cropping would cut the legs off
                skipped['size'] += 1
                continue
            idx, palette, changed = quantize15(t)
        rgba64 = rgb_of(place64(idx), palette)
        key = shape_key(rgba64)
        tt = trim(rgba64)
        dup = None
        for i, k in enumerate(keys):
            d = np.abs(k - key).mean()
            if i < len(known) and ((d < 10 and same_picture(tt, known[i][1], 0.08)) or (d < 30 and same_drawing(tt, known[i][1]))):
                dup = known[i][0]
                break
        if dup:
            skipped['dup'] += 1
            print('  skip showdown %-28s looks like %s' % (stem, dup))
            continue
        known.append(('showdown ' + stem, tt))
        keys.append(key)
        out.append({'stem': stem, 'name': showdown_name(stem), 'artist': artist,
                    'tiles': to_4bpp(place64(idx), 64, 64), 'pal': gba_pal(palette),
                    'recolor': not artist})
    print('  showdown: %d imported, skipped %s' % (len(out), skipped))
    return out


# ---------------------------------------------------------------------------

def c_str(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    frlg, sd = sys.argv[1], sys.argv[2]
    os.makedirs(OUT, exist_ok=True)

    print('Emerald pictures...')
    known = [('emerald ' + s, p) for s, p in emerald_fronts()]
    ow = frlg_overworld(frlg, emerald_overworld_frames())
    print('FRLG overworld: %d' % len(ow))
    fronts = frlg_fronts(frlg, known)
    print('FRLG fronts: %d' % len(fronts))
    known_keys = []
    for name, t in known:
        im = np.zeros((64, 64, 4), np.uint8)
        im[64 - t.shape[0]:, (64 - t.shape[1]) // 2:(64 - t.shape[1]) // 2 + t.shape[1]] = t[:64, :64]
        known_keys.append(shape_key(im))
    sdf = showdown(sd, known, known_keys)
    backs = frlg_backs(frlg)
    frames = frlg_frames(frlg)
    print('FRLG window frames: %d' % len(frames))

    all_fronts = fronts + sdf
    stem_index = {f['stem']: i for i, f in enumerate(fronts)}

    with open(os.path.join(OUT, 'ow_tiles.bin'), 'wb') as f:
        for o in ow:
            f.write(o['tiles'])
    with open(os.path.join(OUT, 'ow_pals.bin'), 'wb') as f:
        for o in ow:
            f.write(o['pal'])
    with open(os.path.join(OUT, 'front_tiles.bin'), 'wb') as f:
        for p in all_fronts:
            f.write(p['tiles'])
    with open(os.path.join(OUT, 'front_pals.bin'), 'wb') as f:
        for p in all_fronts:
            f.write(p['pal'])
    with open(os.path.join(OUT, 'back_tiles.bin'), 'wb') as f:
        for b in backs:
            f.write(b['tiles'])
    with open(os.path.join(OUT, 'back_pals.bin'), 'wb') as f:
        for b in backs:
            f.write(b['pal'])
    with open(os.path.join(OUT, 'frame_tiles.bin'), 'wb') as f:
        for b in frames:
            f.write(b['tiles'])
    with open(os.path.join(OUT, 'frame_pals.bin'), 'wb') as f:
        for b in frames:
            f.write(b['pal'])

    h = []
    h.append('// Moemon Co-op: generated by tools/moemon_frlg/import.py, do not edit.')
    h.append('// FireRed/LeafGreen sprites (c) Nintendo / Game Freak, imported from pret/pokefirered.')
    h.append('// Showdown trainer sprites from play.pokemonshowdown.com/sprites/trainers; credited')
    h.append('// fan art is used as drawn and never recolored (see the artist field).')
    h.append('')
    h.append('#define MP_EXT_OW_COUNT %d' % len(ow))
    h.append('#define MP_EXT_FRONT_COUNT %d' % len(all_fronts))
    h.append('#define MP_EXT_FRONT_FRLG_COUNT %d' % len(fronts))
    h.append('#define MP_EXT_BACK_COUNT %d' % len(backs))
    h.append('#define MP_EXT_FRAME_COUNT %d' % len(frames))
    h.append('')
    h.append('static const struct MpExtOverworld sMpExtOverworld[MP_EXT_OW_COUNT] = {')
    off = 0
    em_ids = emerald_pic_ids()
    em_count = int(re.search(r'#define TRAINER_PIC_MP_LOCAL\s+(\d+)', open(os.path.join(ROOT, 'include/constants/trainers.h')).read()).group(1))
    for o in ow:
        # Front choice: an Emerald TRAINER_PIC_ or TRAINER_PIC_MP_LOCAL + ext front
        fi = -1
        if o['front'] in stem_index:
            fi = em_count + stem_index[o['front']]
        elif o['front'] in FRLG_FRONT_DUPS:
            fi = em_ids.get(FRLG_FRONT_DUPS[o['front']].split(' ', 1)[1], -1)
        h.append('    {%s, %d, %d, %d, %d, %s},' % (c_str(o['name']), o['w'], o['h'], off, fi,
                                                'FEMALE' if o['female'] else 'MALE'))
        off += len(o['tiles'])
    h.append('};')
    h.append('')
    h.append('static const struct MpExtFront sMpExtFronts[MP_EXT_FRONT_COUNT] = {')
    for p in all_fronts:
        cfg = ('sd_' if p in sdf else 'frlg_') + p['stem'].lower()
        h.append('    {%s, %s, %s, %d},' % (c_str(p['name']), c_str(cfg), c_str(p['artist']), 1 if p['recolor'] else 0))
    h.append('};')
    h.append('')
    # Emerald's own trainer pictures, named for the menu and the ini file
    em = re.findall(r'#define TRAINER_PIC_(\w+)\s+(\d+)', open(os.path.join(ROOT, 'include/constants/trainers.h')).read())
    em = [n for n, v in sorted(em, key=lambda e: int(e[1])) if not n.startswith('MP_')]
    h.append('static const char *const sMpEmeraldFrontNames[][2] = {')
    for n in em:
        pretty = re.sub(r'^(LEADER|ELITE_FOUR|CHAMPION|AQUA_LEADER|MAGMA_LEADER|SALON_MAIDEN|DOME_ACE|PALACE_MAVEN|'
                        r'ARENA_TYCOON|FACTORY_HEAD|PIKE_QUEEN|PYRAMID_KING)_', '', n)
        pretty = pretty.replace('POKEMON_', '').replace('TATE_AND_LIZA', 'TATE AND LIZA').replace('_', ' ')
        if n in ('RS_BRENDAN', 'RS_MAY'):
            pretty = n.replace('_', ' ')
        h.append('    {%s, %s},' % (c_str(pretty), c_str(n.lower())))
    h.append('};')
    h.append('')
    h.append('static const char *const sMpExtBackNames[MP_EXT_BACK_COUNT] = {')
    for b in backs:
        h.append('    %s,' % c_str(b['name']))
    h.append('};')
    h.append('')
    with open(os.path.join(ROOT, 'src/mp/mp_ext_gfx.h'), 'w') as f:
        f.write('\n'.join(h))

    # Window frames go to text_window.c
    fr = ['// Moemon Kai: FireRed/LeafGreen text window frames (generated by tools/moemon_frlg/import.py)']
    fr.append('#define MP_FRLG_FRAME_COUNT %d' % len(frames))
    fr.append('static const u8 sMpFrlgFrameTiles[] = INCBIN_U8("graphics/moemon/ext/frame_tiles.bin");')
    fr.append('static const u16 sMpFrlgFramePals[] = INCBIN_U16("graphics/moemon/ext/frame_pals.bin");')
    with open(os.path.join(ROOT, 'src/mp/mp_frlg_frames.h'), 'w') as f:
        f.write('\n'.join(fr) + '\n')
    print('done: %d overworld, %d fronts (%d FRLG + %d Showdown), %d backs, %d frames' % (
        len(ow), len(all_fronts), len(fronts), len(sdf), len(backs), len(frames)))


if __name__ == '__main__':
    main()
