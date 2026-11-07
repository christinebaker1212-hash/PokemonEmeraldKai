#!/usr/bin/env python3
"""ORRE's people: each Colosseum character model as a LOOK > CREATE recipe (its field sprite) and a battle picture
from Pokemon Showdown's trainer sprites (play.pokemonshowdown.com/sprites/trainers), the closest look to the model
(the user's choice, 2026-11-07; the pictures shown at DS size, 80x80, as drawn).

PEOPLE below picks each model's styles by eye (body, top, bottom, field hairstyle, hat, bag); the colours come from
the model itself (its hair, skin, top and bottom sampled from a render), the nearest the creator has for that style,
unless an entry names them.

  python3 tools/orre/people.py build [UNPACKED]   src/data/orre_people.h + graphics/orre/people_pics.bin
  python3 tools/orre/people.py sheet OUT.png      models beside their looks (needs the game built: mp createddump)

The Showdown sprites are fetched into ~/.cache/moemon/showdown_trainers (with index.html for the artists' credits).
"""
import os
import re
import struct
import subprocess
import sys
import urllib.request

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
SD_URL = 'https://play.pokemonshowdown.com/sprites/trainers/'
SD_DIR = os.path.expanduser('~/.cache/moemon/showdown_trainers')
UNPACKED = os.path.expanduser('~/.cache/moemon/orre/unpacked')

# Styles (the creator's lists, include/mp.h MP_CR_*): tops 0 T-SHIRT 1 TANK TOP 2 V-NECK 3 COLLARED 4 SHIRT COMBO
# 5 HOODIE 6 OPEN JACKET 7 VEST 8 SCARF TOP 9 TURTLENECK 10 TRENCH COAT 11 CLASSY 12 FORMAL 13 SCHOOL UNIFORM
# 14 JUMPSUIT 15 ALT JUMPSUIT 16 BEACH 17 LORD SUIT; bottoms 0 JEANS 1 SHORTS 2 CAPRI 3 PANTS AND BOOTS 4 CLASSY
# 5 FORMAL 6 LONG SKIRT 7 PENCIL SKIRT 8 RIBBON SKIRT 9 SCHOOL 10 JUMPSUIT 11 JUMPSUIT ALT 12 BEACH 13 LORD SUIT;
# hats (style + 1 in a recipe) 0 BEANIE 1 YOUNGSTER HAT 2 BERET 3 FEDORA 4 STRAW HAT 5 SUN HAT 6 FEATHER HAT 7 BONNET
# 8 CROWN 9 FLOWER 10 RIBBON 11 HAIR BAND 12 HEADBAND 13 GLASSES 14 CAT EARS 15 DEVIL 16 MINING; field hairstyles
# 1-144 (mp_creator_ow.h sOw3Hair; 20 / 94 bald, 28 a bald crown, 22 thinning, 30 AFRO, 60 side tufts).
B, G = 0, 1
H = None   # no hat

#            model             body top bot fhair hat  picture                  colours named here (else sampled)
PEOPLE = [
    ('ken_b1',          B, 10, 3, 104, H,  'volkner',               dict(hair=10)),
    ('hizuki_b1',       G, 6, 1, 95, H,    'rosa',                  dict(hair=3, bottom=4)),
    ('sport_m_b1',      B, 0, 1, 12, 12,   'triathleterunnerm-gen3', {}),
    ('sport_f_b1',      G, 1, 1, 16, 11,   'triathleterunnerf-gen3', {}),
    ('traner_m_b1',     B, 6, 1, 1, H,     'acetrainer-gen4',       {}),
    ('traner_m_b2',     B, 5, 1, 5, H,     'acetrainer',            {}),
    ('traner_f_b1',     G, 15, 1, 27, H,   'acetrainerf',           {}),
    ('obo_b1',          B, 12, 5, 9, H,    'richboy-gen6',          {}),
    ('obo_b2',          B, 0, 1, 5, H,     'schoolboy',             {}),
    ('obo_b3',          B, 3, 0, 2, H,     'schoolkid',             {}),
    ('obo_b4',          B, 6, 0, 5, H,     'youngster-gen4',        {}),
    ('ojo_b1',          G, 11, 8, 32, H,   'lady-gen3',             {}),
    ('ojo_b2',          G, 3, 6, 27, H,    'lass-gen4',             {}),
    ('ojo_b3',          G, 11, 6, 37, H,   'lady-gen4',             {}),
    ('ojisan_b1',       B, 6, 0, 9, H,     'officeworker',          {}),
    ('ojisan_b_b1',     B, 14, 10, 22, H,  'rancher',               {}),
    ('obasan_b1',       G, 12, 7, 41, 1,   'madame-gen4',           {}),
    ('jiji_a_b1',       B, 9, 4, 28, H,    'mrfuji-gen3',           dict(hair=10)),
    ('jiji_b_b1',       B, 11, 4, 20, H,   'gentleman-gen3',        {}),
    ('jiji_b_b2',       B, 10, 0, 28, 3,   'kurt',                  dict(hair=10)),
    ('baba_a_b1',       G, 9, 2, 37, H,    'madame',                dict(hair=10)),
    ('baba_b_b1',       G, 11, 6, 37, 7,   'veteranf',              dict(hair=10)),
    ('hunter_m_b1',     B, 7, 3, 5, 3,     'backpacker',            {}),
    ('hunter_m_b2',     B, 6, 3, 1, 13,    'pokemonranger-gen4',    {}),
    ('hunter_m_b3',     B, 7, 3, 52, H,    'punkguy',               {}),
    ('hunter_f_b1',     G, 1, 0, 71, H,    'pokemonrangerf-gen4',   {}),
    ('hunter_f_b2',     G, 3, 2, 27, H,    'backpackerf',           {}),
    ('hunter_f_b3',     G, 1, 0, 41, H,    'pokemonrangerf',        {}),
    ('mania_b1',        B, 10, 0, 5, 1,    'supernerd',             {}),
    ('rider_m_b1',      B, 6, 0, 31, H,    'guitarist-gen4',        dict(hair=5)),
    ('rider_m_b2',      B, 10, 3, 52, H,   'cyclist',               {}),
    ('rider_f_b1',      G, 14, 10, 111, 5, 'cowgirl',               {}),
    ('warugaki_b1',     B, 0, 1, 5, 1,     'rollerskater',          {}),
    ('warugaki_b2',     B, 0, 1, 5, 1,     'youngster-gen4',        {}),
    ('warugaki_b3',     B, 0, 1, 5, 1,     'schoolkid',             {}),
    ('ippan_m_b1',      B, 5, 0, 9, H,     'tourist',               {}),
    ('ippan_f_b1',      G, 3, 7, 27, H,    'beauty-gen4dp',         {}),
    ('cook_b1',         B, 0, 10, 52, 12,  'cook',                  {}),
    ('sensei_b1',       G, 10, 2, 32, H,   'teacher',               {}),
    ('kenkyuin_b1',     B, 3, 4, 9, 13,    'scientist',             {}),
    ('agent_m_b1',      B, 14, 10, 2, 0,   'flaregrunt',            dict(hatColor=1)),
    ('agent_m_b2',      B, 14, 10, 2, 0,   'flaregrunt',            dict(hatColor=1)),
    ('agent_m_b3',      B, 14, 10, 2, 0,   'flaregrunt',            dict(hatColor=1)),
    ('agent_m_b4',      B, 14, 10, 2, 0,   'flaregrunt',            dict(hatColor=1)),
    ('agent_m_b5',      B, 14, 10, 2, 0,   'flaregrunt',            dict(hatColor=1)),
    ('agent_m_b6',      B, 14, 10, 2, 0,   'flaregrunt',            dict(hatColor=1)),
    ('agent_f_b1',      G, 14, 10, 32, 0,  'flaregruntf',           {}),
    ('boss555_b1',      B, 14, 10, 30, H,  'dancer',                dict(hair=4)),
    ('battleyama_b1',   B, 4, 1, 52, H,    'linebacker',            dict(hair=4)),
    ('boss777_b1',      G, 13, 6, 111, 8,  'lusamine',              dict(hair=2)),
    ('boss999_b1',      B, 10, 3, 104, H,  'colress',               dict(hair=10)),
    ('boss_s_b1',       B, 7, 3, 52, H,    'bruno',                 dict(hair=4)),
    ('sityo_b1',        B, 10, 4, 60, H,   'gentleman-gen3',        dict(hair=10)),
    ('sityo_b2',        B, 10, 4, 60, H,   'gentleman-gen3',        dict(hair=10)),
    ('niku_m_b1',       B, 1, 1, 52, H,    'bodybuilder-gen9',      {}),
    ('niku_m_b2',       B, 1, 5, 20, H,    'blackbelt-gen4',        {}),
    ('niku_f_b1',       G, 1, 1, 32, H,    'bodybuilderf-gen9',     {}),
    ('pc_b1',           G, 12, 7, 37, H,   'nurse',                 {}),
    ('thief_m_b1',      B, 5, 10, 31, H,   'punkguy',               dict(hair=4)),
    ('thief_m_b2',      B, 14, 10, 2, 13,  'punkguy-gen7',          {}),
    ('thief_f_b1',      G, 1, 0, 27, 1,    'punkgirl',              {}),
    ('thief_f_b2',      G, 1, 2, 111, 1,   'punkgirl-gen7',         {}),
    ('worker_m_b1',     B, 10, 10, 5, 16,  'worker-gen7',           {}),
    ('geinin_b1',       G, 12, 8, 111, 3,  'idol',                  {}),
    ('sheriff_b1',      B, 12, 5, 22, H,   'policeman-gen4',        dict(hair=10)),
    ('sheriff_as_b1',   B, 12, 5, 5, 1,    'policeman-gen7',        {}),
    ('boy_b1',          B, 0, 1, 5, H,     'youngster-gen4',        {}),
    ('girl_b1',         G, 3, 8, 72, H,    'schoolkidf-gen4',       {}),
    ('uranai_b1',       G, 11, 6, 72, 13,  'psychicf',              dict(hair=5)),
    ('grandpa_b1',      B, 10, 4, 22, H,   'sage',                  dict(hair=10)),
    ('grandpa_b2',      B, 10, 4, 22, H,   'sage',                  dict(hair=10)),
    ('grandpa_b3',      B, 10, 4, 22, H,   'sage',                  dict(hair=10)),
    ('grandma_b1',      G, 9, 6, 37, H,    'madame-gen6',           dict(hair=10)),
    ('info_b1',         G, 12, 7, 32, H,   'officeworkerf',         {}),
    ('shopboy_b1',      B, 14, 5, 2, H,    'clerk',                 {}),
    ('syotyou_b1',      B, 11, 4, 60, H,   'veteran-gen6',          dict(hair=10)),
    ('kuro_b1',         B, 11, 4, 9, H,    'butler',                {}),
    ('caster_a_b1',     G, 12, 7, 32, H,   'reporter',              {}),
    ('fusya_b1',        B, 10, 3, 20, 13,  'engineer-gen3',         {}),
    ('snach_zako_b1',   B, 7, 3, 20, H,    'roughneck',             {}),
    ('snach_zako_b2',   B, 7, 3, 20, H,    'roughneck',             {}),
    ('snach_zako_b3',   B, 7, 3, 20, H,    'roughneck',             {}),
    ('ginzaru_b1',      B, 0, 5, 5, H,     'blackbelt-gen4',        {}),
    ('gym_a_b1',        G, 10, 0, 27, H,   'scientistf',            {}),
    ('gym_a_b2',        B, 10, 0, 2, H,    'acetrainer-gen4',       {}),
    ('gym_a_b3',        G, 15, 0, 32, H,   'pokemonbreederf-gen4',  {}),
    ('gym_a_b4',        B, 10, 0, 1, H,    'acetrainer-gen6',       {}),
    ('gym_b_b1',        B, 1, 0, 5, H,     'blackbelt-gen4',        {}),
    ('gym_b_b2',        B, 0, 0, 5, H,     'hiker-gen4',            {}),
    ('gym_b_b3',        B, 1, 0, 5, 1,     'hiker',                 {}),
    ('gym_c_b1',        G, 11, 8, 71, H,   'lass-gen4',             {}),
    ('gym_c_b2',        G, 12, 8, 37, H,   'lass-gen6',             {}),
]

# Hair colours by eye (0 black 1 brown 2 blonde 3 ginger 4 red 5 pink 6 purple 7 blue 8 cyan 9 green 10 white)
# and the tanned ones (skin MEDIUM; everyone else LIGHT): a render's top rows are often a hat
HAIR = {
    'ken_b1': 10, 'hizuki_b1': 3, 'sport_m_b1': 1, 'sport_f_b1': 0, 'traner_m_b1': 1, 'traner_m_b2': 10,
    'traner_f_b1': 1, 'obo_b1': 2, 'obo_b2': 1, 'obo_b3': 1, 'obo_b4': 1, 'ojo_b1': 2,
    'ojo_b2': 4, 'ojo_b3': 0, 'ojisan_b1': 0, 'ojisan_b_b1': 10, 'obasan_b1': 4, 'jiji_a_b1': 10,
    'jiji_b_b1': 10, 'jiji_b_b2': 10, 'baba_a_b1': 10, 'baba_b_b1': 10, 'hunter_m_b1': 1, 'hunter_m_b2': 0,
    'hunter_m_b3': 3, 'hunter_f_b1': 8, 'hunter_f_b2': 6, 'hunter_f_b3': 4, 'mania_b1': 1, 'rider_m_b1': 5,
    'rider_m_b2': 2, 'rider_f_b1': 2, 'warugaki_b1': 2, 'warugaki_b2': 1, 'warugaki_b3': 1, 'ippan_m_b1': 0,
    'ippan_f_b1': 1, 'cook_b1': 4, 'sensei_b1': 6, 'kenkyuin_b1': 1, 'agent_m_b1': 0, 'agent_m_b2': 0,
    'agent_m_b3': 0, 'agent_m_b4': 0, 'agent_m_b5': 0, 'agent_m_b6': 0, 'agent_f_b1': 6, 'boss555_b1': 4,
    'battleyama_b1': 4, 'boss777_b1': 2, 'boss999_b1': 10, 'boss_s_b1': 4, 'sityo_b1': 10, 'sityo_b2': 10,
    'niku_m_b1': 3, 'niku_m_b2': 0, 'niku_f_b1': 6, 'pc_b1': 0, 'thief_m_b1': 4, 'thief_m_b2': 9,
    'thief_f_b1': 1, 'thief_f_b2': 2, 'worker_m_b1': 3, 'geinin_b1': 2, 'sheriff_b1': 10, 'sheriff_as_b1': 0,
    'boy_b1': 1, 'girl_b1': 1, 'uranai_b1': 5, 'grandpa_b1': 10, 'grandpa_b2': 10, 'grandpa_b3': 10,
    'grandma_b1': 10, 'info_b1': 2, 'shopboy_b1': 1, 'syotyou_b1': 10, 'kuro_b1': 1, 'caster_a_b1': 6,
    'fusya_b1': 10, 'snach_zako_b1': 0, 'snach_zako_b2': 0, 'snach_zako_b3': 0, 'ginzaru_b1': 1, 'gym_a_b1': 6,
    'gym_a_b2': 4, 'gym_a_b3': 1, 'gym_a_b4': 7, 'gym_b_b1': 1, 'gym_b_b2': 2, 'gym_b_b3': 0,
    'gym_c_b1': 4, 'gym_c_b2': 1,
}
TAN = {'battleyama_b1', 'boss_s_b1', 'gym_b_b1', 'gym_b_b2', 'gym_b_b3', 'niku_m_b1'}

# ---------------------------------------------------------------------------
# The creator's tables, read from the game's sources


def _creator():
    d = open(os.path.join(ROOT, 'src/mp/mp_creator_data.h')).read()
    styles = [(n, int(a), int(c)) for n, a, c in
              re.findall(r'\{"([^"]+)", (\d+), (\d+)\},', d.split('sCrStyles[]')[1].split('};')[0])]
    cats = [[tuple(map(int, p)) for p in re.findall(r'\{(\d+), (\d+)\}', row)]
            for row in re.findall(r'\{(\{\d+, \d+\}(?:, \{\d+, \d+\})*)\}', d.split('sCrCatStyles')[1].split('};')[0])]
    o = open(os.path.join(ROOT, 'src/mp/mp_creator_ow.h')).read()
    tone = [int(x, 16) for x in re.findall(r'0x([0-9a-f]+)', o.split('sOw3Tone[')[1].split('};')[0])]
    tone = [tone[i * 2] for i in range(len(tone) // 2)]
    skins = [[int(x, 16) for x in re.findall(r'0x([0-9a-f]+)', s)]
             for s in re.findall(r'\{(0x[^}]+)\}', o.split('sOw3Skin[4][4] =')[1].split(';')[0])]
    return styles, cats, tone, skins


def rgb555(c):
    return np.array([(c & 31) << 3, ((c >> 5) & 31) << 3, ((c >> 10) & 31) << 3], float)


def nearest(rgb, options):
    """Index of the option colour nearest RGB (hue and lightness both weigh)."""
    def lab(c):
        c = np.asarray(c, float) / 255
        y = c @ [0.3, 0.59, 0.11]
        return np.array([y * 2.0, (c[0] - y) * 1.2, (c[2] - y) * 1.2])
    d = [np.sum((lab(rgb) - lab(o)) ** 2) for o in options]
    return int(np.argmin(d))


# ---------------------------------------------------------------------------
# Colours from the model


def model_path(stem):
    for d in sorted(os.listdir(UNPACKED)):
        p = os.path.join(UNPACKED, d, stem + '.dat')
        if os.path.exists(p):
            return p
    return None


def render(stem):
    sys.path.insert(0, HERE)
    import hsd_anim
    import people_sprites
    f, root, an = hsd_anim.model(model_path(stem))
    ms = hsd_anim.load_posed(f, root, an[1] if len(an) > 1 else {}, 0)
    pos = np.concatenate([m.pos for m in ms])
    lo, hi = pos.min(0), pos.max(0)
    ppu = 300 / max(hi[1] - lo[1], 1e-3)
    big, zb, ib = people_sprites.render_rgba(ms, f.images, 200, 320, (100 - (lo[0] + hi[0]) / 2 * ppu, 310 + lo[1] * ppu),
                                             ppu, 5, 180)
    return big


def sample(big):
    """Median colours of the hair, the skin, the top and the bottom. The head is the top sixth: its skin-coloured
    pixels (red over green over blue, warm) are the skin, the rest of its top rows the hair; the top and the bottom
    from the middle columns of the chest and the legs."""
    a = big[..., 3] > 0
    rows = np.where(a.any(1))[0]
    t, b = rows.min(), rows.max()
    h = b - t + 1
    cols = np.where(a.any(0))[0]
    cx = (cols.min() + cols.max()) // 2
    w = max(4, (cols.max() - cols.min()) // 5)
    rgb = big[..., :3].astype(float)
    r, g, bl = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    skinlike = a & (r > g) & (g > bl) & (r - bl > 40) & (r > 120) & (r - g < 90)
    head = np.zeros_like(a)
    head[t:t + h // 6] = True

    def med(mask, default=(128, 128, 128)):
        px = rgb[mask]
        return np.median(px, 0) if len(px) else np.array(default, float)

    def box(f0, f1, wide=1.0):
        m = np.zeros_like(a)
        m[t + int(h * f0):t + max(int(h * f1), int(h * f0) + 1), cx - int(w * wide):cx + int(w * wide) + 1] = True
        return m & a
    hairrows = np.zeros_like(a)
    hairrows[t:t + max(2, int(h * 0.07))] = True
    hair = hairrows & a & ~skinlike
    return dict(hair=med(hair if hair.sum() > 4 else hairrows & a), skin=med(head & skinlike, (232, 184, 152)),
                top=med(box(0.28, 0.42)), bottom=med(box(0.62, 0.78, 0.6)))


def recipe(entry, colours=None):
    stem, girl, top, bot, fhair, hat, pic, named = entry
    styles, cats, tone, skins = _creator()

    def opts(cat, style):
        a, n = cats[girl][cat]
        _, first, count = styles[a + style]
        return [rgb555(tone[first + k]) for k in range(count)]
    c = colours or {}
    named = dict(named)
    named.setdefault('hair', HAIR.get(stem, 0))
    named.setdefault('skin', 2 if stem in TAN else 1)
    pick = {}
    for key, cat, style in (('hair', 0, 0), ('top', 1, top), ('bottom', 2, bot)):
        pick[key] = named[key] if key in named else (nearest(c[key], opts(cat, style)) if key in c else 0)
    skin = named.get('skin', int(np.argmin([abs(np.dot(c['skin'], [0.3, 0.59, 0.11]) - np.dot(rgb555(s[1]), [0.3, 0.59, 0.11]))
                                            for s in skins])) if 'skin' in c else 1)
    hairstyle = {B: 0, G: 0}[girl]   # the battle-picture hairstyle (the LOOK's own picture isn't used for ORRE)
    vals = [1, girl, skin, hairstyle, pick['hair'], top, pick['top'], bot, pick['bottom'],
            0 if hat is None else hat + 1, named.get('hatColor', 0), 0, 0, fhair]
    return ''.join('%02x' % v for v in vals)


# ---------------------------------------------------------------------------
# Showdown pictures


def _get(url, path):
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0 (moemon-build)'})
    with urllib.request.urlopen(req, timeout=60) as r:
        open(path, 'wb').write(r.read())


def fetch(stem):
    os.makedirs(SD_DIR, exist_ok=True)
    p = os.path.join(SD_DIR, stem + '.png')
    if not os.path.exists(p):
        _get(SD_URL + stem + '.png', p)
    return p


def credits():
    p = os.path.join(SD_DIR, 'index.html')
    if not os.path.exists(p):
        _get(SD_URL, p)
    src = open(p, encoding='utf-8', errors='replace').read()
    out = {}
    for fig in re.findall(r'<figure.*?</figure>', src, re.S):
        m = re.search(r'src="[^"]*?([^"/]+)\.png"', fig)
        cap = re.search(r'by ([^<]+)', fig)
        if m:
            out[m.group(1)] = cap.group(1).strip() if cap else ''
    return out


def picture(stem):
    """80x80 as drawn: (bpp, colours as RGB555 with 0 = transparent, pixel indices)."""
    im = np.array(Image.open(fetch(stem)).convert('RGBA'))
    if im.shape[:2] != (80, 80):
        raise SystemExit('%s: %dx%d, not 80x80' % (stem, im.shape[1], im.shape[0]))
    a = im[..., 3] >= 128
    c = ((im[..., 0].astype(int) >> 3) | ((im[..., 1].astype(int) >> 3) << 5) | ((im[..., 2].astype(int) >> 3) << 10))
    used, counts = np.unique(c[a], return_counts=True)
    order = used[np.argsort(-counts)]
    if len(order) > 255:
        raise SystemExit('%s: %d colours' % (stem, len(order)))
    lut = {v: i + 1 for i, v in enumerate(order)}
    idx = np.zeros((80, 80), np.uint8)
    for y, x in zip(*np.where(a)):
        idx[y, x] = lut[c[y, x]]
    return (4 if len(order) <= 15 else 8), [0] + [int(v) for v in order], idx


def build(unpacked):
    global UNPACKED
    UNPACKED = unpacked
    cr = credits()
    pics = []
    for e in PEOPLE:
        if e[6] not in pics:
            pics.append(e[6])
    blob = bytearray(b'ORPP' + struct.pack('<H', len(pics)))
    for name in pics:
        bpp, pal, idx = picture(name)
        blob += name.encode()[:23].ljust(24, b'\0') + struct.pack('<BB', bpp, len(pal))
        blob += struct.pack('<%dH' % len(pal), *pal)
        # feet where the game's 64x64 pictures have theirs; that puts the top 8 rows above the screen, so a picture
        # drawn closer to its top than that comes down until its head shows (its feet still on the platform)
        top = int(np.argmax(idx.any(1)))
        blob += struct.pack('<hhHH', -40, -48 + max(0, 8 - top), 80, 80)
        if bpp == 4:
            for row in idx:
                blob += bytes((row[x] & 15) | (row[x + 1] & 15) << 4 for x in range(0, 80, 2))
        else:
            blob += idx.tobytes()
    os.makedirs(os.path.join(ROOT, 'graphics/orre'), exist_ok=True)
    open(os.path.join(ROOT, 'graphics/orre/people_pics.bin'), 'wb').write(blob)
    lines = ['// ORRE\'s people: each Colosseum character model\'s LOOK recipe (field sprite) and battle picture.',
             '// Generated by tools/orre/people.py from its PEOPLE table. Do not edit.', '',
             '#define ORRE_PEOPLE_PIC_COUNT %d' % len(pics), '#define ORRE_PEOPLE_COUNT %d' % len(PEOPLE), '',
             'static const struct OrrePerson sOrrePeople[ORRE_PEOPLE_COUNT] = {']
    for e in PEOPLE:
        rec = recipe(e, sample(render(e[0])))
        lines.append('    {"%s", "%s", %d},   // %s' % (e[0], rec, pics.index(e[6]), e[6]))
    lines += ['};', '', '// The pictures (Showdown\'s names)', 'static const char *const sOrrePicName[ORRE_PEOPLE_PIC_COUNT] = {']
    lines += ['    "%s",' % p for p in pics]
    lines += ['};', '', '// The pictures\' artists (Showdown\'s credits; empty: the games\' own sprites)',
              'static const char *const sOrrePicArtist[ORRE_PEOPLE_PIC_COUNT] = {']
    lines += ['    "%s",   // %s' % (cr.get(p, ''), p) for p in pics]
    lines += ['};', '']
    open(os.path.join(ROOT, 'src/data/orre_people.h'), 'w').write('\n'.join(lines))
    print('%d people, %d pictures, %d bytes' % (len(PEOPLE), len(pics), len(blob)))


def sheet(out):
    """Each model's render, its LOOK field sprite (down, side, up) and its battle picture, for review."""
    from PIL import ImageDraw
    recs = {}
    for line in open(os.path.join(ROOT, 'src/data/orre_people.h')):
        m = re.match(r'\s+\{"(\w+)", "([0-9a-f]+)", (\d+)\},\s+// (\S+)', line)
        if m:
            recs[m.group(1)] = (m.group(2), m.group(4))
    tmp = os.path.expanduser('~/.cache/moemon/orre_people_dump')
    os.makedirs(tmp, exist_ok=True)
    scr = os.path.join(tmp, 'script.txt')
    with open(scr, 'w') as s:
        s.write('wait 90\n')
        for stem, (rec, _) in recs.items():
            os.makedirs(os.path.join(tmp, rec), exist_ok=True)
            s.write('mp createddump %s %s\nwait 1\n' % (rec, os.path.join(tmp, rec)))
        s.write('quit\n')
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_TEST=scr,
               MOEMON_DEBUG_QUICKSTART='1', MOEMON_SAVE=os.path.join(tmp, 't.sav'),
               MOEMON_COOP_CONFIG=os.path.join(tmp, 'coop.ini'))
    subprocess.run([os.path.join(ROOT, 'pokeemerald64')], cwd=ROOT, env=env, stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL, timeout=300)
    W, Hh = 400, 112
    cols = 4
    im = Image.new('RGBA', (cols * W, ((len(recs) + cols - 1) // cols) * Hh), (112, 160, 112, 255))
    d = ImageDraw.Draw(im)
    for k, (stem, (rec, pic)) in enumerate(recs.items()):
        x, y = (k % cols) * W, (k // cols) * Hh
        r = Image.fromarray(render(stem).astype(np.uint8)).resize((60, 96))
        im.alpha_composite(r, (x + 2, y + 2))
        f = Image.open(os.path.join(tmp, rec, 'field.png'))
        for n, fi in enumerate((0, 2, 1)):
            im.alpha_composite(f.crop((fi * 32 + 4, 4, fi * 32 + 28, 32)).resize((72, 84), Image.NEAREST), (x + 64 + n * 76, y + 12))
        p = Image.open(fetch(pic)).convert('RGBA')
        im.alpha_composite(p.resize((80, 80), Image.NEAREST), (x + 300, y + 10))
        d.text((x + 64, y + 2), stem, fill=(0, 0, 0))
        d.text((x + 64, y + 96), pic, fill=(0, 0, 0))
    im.save(out)


if __name__ == '__main__':
    if len(sys.argv) >= 2 and sys.argv[1] == 'build':
        build(sys.argv[2] if len(sys.argv) > 2 else UNPACKED)
    elif len(sys.argv) == 3 and sys.argv[1] == 'sheet':
        sheet(sys.argv[2])
    else:
        print(__doc__)
