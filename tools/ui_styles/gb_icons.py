"""RED's and CRYSTAL's party icons for every POKéMON (OPTIONS > DISPLAY > MENU POKéMON: ORIGINAL).

The games' own choices for their own POKéMON (pret/pokered data/pokemon/menu_icons.asm: 151 to ten icons; pret/
pokecrystal's: 251 to 38), and the same decisions carried to every later one: a POKéMON whose evolution line holds
one of the game's own takes that relative's icon (the nearest stage); any other takes the icon the game gave most
often to its POKéMON of the same body shape and first type, then of the same shape, then of the same type
(data/species_shapes.csv, from PokeAPI's pokemon_species.csv and pokemon_types.csv).

icons(kind, pret) -> (frames, table): frames a list of (frame A, frame B) 16 x 16 pictures as the game draws
them (shades: 0 ink .. 3 clear), table bytes indexed by the national POKéDEX number (0 the EGG) giving each its icon.
"""
import collections
import csv
import os
import re

from PIL import Image

NATIONAL_MAX = 1025


def _red_assign(pret):
    names = []
    for line in open(os.path.join(pret, 'data', 'pokemon', 'menu_icons.asm')):
        m = re.match(r'\s*nybble\s+(ICON_\w+)', line)
        if m:
            names.append(m.group(1))
    return names[:151]


def _crystal_assign(pret):
    names = []
    for line in open(os.path.join(pret, 'data', 'pokemon', 'menu_icons.asm')):
        m = re.match(r'\s*db\s+(ICON_\w+)', line)
        if m:
            names.append(m.group(1))
    return names[:251]


RED_ICONS = ['ICON_MON', 'ICON_BALL', 'ICON_HELIX', 'ICON_FAIRY', 'ICON_BIRD', 'ICON_WATER', 'ICON_BUG', 'ICON_GRASS',
             'ICON_SNAKE', 'ICON_QUADRUPED']


def _crystal_icons(pret):
    out = []
    for line in open(os.path.join(pret, 'constants', 'icon_constants.asm')):
        m = re.match(r'\s*const\s+(ICON_\w+)', line)
        if m:
            out.append(m.group(1))
    return out


def _shade(v):
    """a Game Boy 2bpp picture's grey as a shade: 0 the darkest .. 3 the lightest (clear on a sprite)"""
    return min(3, (v + 42) // 85)


def _shades(im):
    im = im.convert('L')
    return [[_shade(im.getpixel((x, y))) for x in range(im.width)] for y in range(im.height)]


def _mirror_left(rows8x16):
    """the symmetric writer: the left column, its mirror on the right (pokered WriteSymmetricMonPartySpriteOAM)"""
    return [r[:8] + r[:8][::-1] for r in rows8x16]


def _red_frames(pret):
    g = lambda *p: os.path.join(pret, 'gfx', *p)  # noqa: E731
    out = {}

    def sheet(name, frame):
        s = _shades(Image.open(g('sprites', name + '.png')))
        return [r[:] for r in s[frame * 16:frame * 16 + 16]]

    def half(name, frame):
        s = _shades(Image.open(g('icons', name + '.png')))
        return _mirror_left([r[:8] for r in s[frame * 16:frame * 16 + 16]])

    def shake(rows):
        return [[3] * 16] + rows[:-1]
    # (data/icon_pointers.asm: the first frame from tile 12, the second from tile 0; SEEL the other way round; the
    # ball and the helix shake a pixel down; BUG and GRASS start on their second half-picture, SNAKE and QUADRUPED on
    # their first)
    for icon, name in (('ICON_MON', 'monster'), ('ICON_FAIRY', 'fairy'), ('ICON_BIRD', 'bird')):
        a, b = sheet(name, 3), sheet(name, 0)
        out[icon] = (_mirror_left([r[:8] for r in a]), _mirror_left([r[:8] for r in b]))
    a, b = sheet('seel', 0), sheet('seel', 3)
    out['ICON_WATER'] = (_mirror_left([r[:8] for r in a]), _mirror_left([r[:8] for r in b]))
    ball = _mirror_left([r[:8] for r in _shades(Image.open(g('sprites', 'poke_ball.png')))])
    out['ICON_BALL'] = (ball, shake(ball))
    helix = _shades(Image.open(g('sprites', 'fossil.png')))[:16]
    out['ICON_HELIX'] = (helix, shake(helix))
    for icon, name, first in (('ICON_BUG', 'bug', 1), ('ICON_GRASS', 'plant', 1), ('ICON_SNAKE', 'snake', 0),
                              ('ICON_QUADRUPED', 'quadruped', 0)):
        out[icon] = (half(name, first), half(name, 1 - first))
    return [out[i] for i in RED_ICONS]


def _crystal_frames(pret, icons):
    out = []
    for icon in icons:
        name = icon[len('ICON_'):].lower()
        path = os.path.join(pret, 'gfx', 'icons', name + '.png')
        if not os.path.exists(path):
            out.append(([[3] * 16 for _ in range(16)],) * 2)
            continue
        s = _shades(Image.open(path))
        out.append((s[0:16], s[16:32] if len(s) >= 32 else s[0:16]))
    return out


SHAPES = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data', 'species_shapes.csv')


def _pokeapi(path=SHAPES):
    """data/species_shapes.csv: PokeAPI's pokemon_species.csv (evolution line, parent, body shape) and
    pokemon_types.csv (the first type) for national numbers 1-1025"""
    species = {}
    for r in csv.DictReader(open(path)):
        species[int(r['id'])] = dict(chain=int(r['evolution_chain_id'] or 0),
                                     parent=int(r['evolves_from_species_id'] or 0), shape=int(r['shape_id'] or 0),
                                     type=int(r['type_id'] or 0))
    for n, s in species.items():
        d, p = 0, s['parent']
        while p and d < 5:
            d, p = d + 1, species.get(p, {}).get('parent', 0)
        s['stage'] = d
    return species


def _map(assign, species, default):
    """national number -> icon name for every POKéMON: the game's own, else carried over"""
    own = {n + 1: icon for n, icon in enumerate(assign)}
    out = dict(own)
    by = collections.defaultdict(collections.Counter)
    for n, icon in own.items():
        s = species.get(n)
        if s:
            by[('st', s['shape'], s['type'])][icon] += 1
            by[('s', s['shape'])][icon] += 1
            by[('t', s['type'])][icon] += 1
    for n in range(1, NATIONAL_MAX + 1):
        if n in out:
            continue
        s = species.get(n)
        if s is None:
            out[n] = default
            continue
        kin = [k for k in own if species.get(k, {}).get('chain') == s['chain'] and s['chain']]
        if kin:
            k = min(kin, key=lambda k: (abs(species[k]['stage'] - s['stage']), -species[k]['stage'], k))
            out[n] = own[k]
            continue
        for key in (('st', s['shape'], s['type']), ('s', s['shape']), ('t', s['type'])):
            if by[key]:
                out[n] = by[key].most_common(1)[0][0]
                break
        else:
            out[n] = default
    return out


def icons(kind, pret):
    species = _pokeapi()
    if kind == 'red':
        names = RED_ICONS
        frames = _red_frames(pret)
        mapping = _map(_red_assign(pret), species, 'ICON_MON')
        egg = 'ICON_BALL'
    else:
        names = _crystal_icons(pret)
        frames = _crystal_frames(pret, names)
        mapping = _map(_crystal_assign(pret), species, 'ICON_MONSTER')
        egg = 'ICON_EGG'
    table = bytes([names.index(egg)] + [names.index(mapping[n]) for n in range(1, NATIONAL_MAX + 1)])
    return frames, table, names, mapping
