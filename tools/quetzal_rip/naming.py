"""Names for the Kanto / Johto maps.

Kanto: Quetzal's groups 36-39 are pokefirered's map list (minus the link rooms) in the same
order, with 8 unused FRLG maps dropped and 3 maps added at the end; aligned by map section.
Johto: the maps were redrawn, so they are matched to pokecrystal's maps by their dialogue
(word overlap), inside the same town / route when possible. Unmatched maps get
<Section>_<Role> names (role from the map's scripts: Pokemon Center, Mart, Gym...).
"""
import difflib
import json
import math
import os
import re
from collections import Counter

from scripts import SPECIALS


def camel(s):
    s = (s or 'Unknown').replace('POKéMON', 'POKEMON').replace('é', 'e').replace('É', 'E')
    s = s.replace('’', '').replace("'", '').replace('.', ' ')
    parts = re.split(r'[^A-Za-z0-9]+', s)
    return ''.join(p[:1].upper() + p[1:].lower() for p in parts if p)


def _norm_sec(s):
    s = (s or '').upper().replace('MAPSEC_', '').replace('LANDMARK_', '').replace('_', ' ')
    s = s.replace('POKéMON', 'POKEMON').replace('É', 'E').replace('é', 'E')
    return re.sub(r'[^A-Z0-9]', '', s)


# ---------------------------------------------------------------- Kanto -------
def kanto_names(maps, frlg_dir):
    mg = json.load(open(os.path.join(frlg_dir, 'data/maps/map_groups.json')))
    fr = []
    for g in mg['group_order'][1:]:
        for n in mg[g]:
            j = json.load(open(os.path.join(frlg_dir, 'data/maps', n, 'map.json')))
            fr.append((n, j['region_map_section']))
    ks = [m for m in maps if m['region'] == 'kanto']
    a = [_norm_sec(m['mapsec_name']) for m in ks]
    b = [_norm_sec(x[1]) for x in fr]
    # FRLG section names differ a little from the in-game names
    alias = {'SSANNE': 'SSANNE', 'POKEMONMANSION': 'POKEMONMANSION'}
    a = [alias.get(x, x) for x in a]
    out = {}
    dropped = []
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag in ('equal', 'replace') and i2 - i1 == j2 - j1:
            for k in range(i2 - i1):
                out[(ks[i1 + k]['group'], ks[i1 + k]['num'])] = fr[j1 + k][0]
        else:
            for k in range(j1, j2):
                dropped.append(fr[k][0])
            for k in range(i1, i2):
                m = ks[k]
                out[(m['group'], m['num'])] = None   # new map, named later
    return out, dropped


# ---------------------------------------------------------------- Johto -------
WORD = re.compile(r"[a-z][a-z']{2,}")
STOP = set('the and you your for that this with are was have has not but all can what just get its out who now'
           ' there here they them then from will one when been were his her him she our like know see some'
           ' don\'t i\'m it\'s that\'s you\'re let\'s'.split())


def words(s):
    s = s.lower().replace('{player}', ' ').replace('{rival}', ' ').replace('\\n', ' ').replace('\\l', ' ').replace('\\p', ' ')
    s = s.replace('é', 'e').replace('’', "'")
    return [w for w in WORD.findall(s) if w not in STOP]


def crystal_maps(crystal_dir):
    src = open(os.path.join(crystal_dir, 'data/maps/maps.asm')).read()
    landmark = {m.group(1): m.group(2) for m in re.finditer(r'^\s*map (\w+), \w+, \w+, (\w+),', src, re.M)}
    texts = {}
    for name in landmark:
        p = os.path.join(crystal_dir, 'maps', name + '.asm')
        if not os.path.exists(p):
            continue
        t = []
        for m in re.finditer(r'^\s*(?:text|line|cont|para|next)\s+"([^"]*)"', open(p).read(), re.M):
            t.append(m.group(1))
        texts[name] = ' '.join(t)
    return landmark, texts


def johto_names(maps, map_texts, crystal_dir):
    landmark, ctexts = crystal_maps(crystal_dir)
    docs = {k: Counter(words(v)) for k, v in ctexts.items()}
    qdocs = {}
    for m in maps:
        if m['region'] != 'johto':
            continue
        qdocs[(m['group'], m['num'])] = Counter(words(map_texts.get((m['group'], m['num']), '')))
    df = Counter()
    for c in list(docs.values()) + list(qdocs.values()):
        for w in c:
            df[w] += 1
    total = len(docs) + len(qdocs)

    def vec(c):
        v = {w: (1 + math.log(n)) * math.log(total / df[w]) for w, n in c.items()}
        norm = math.sqrt(sum(x * x for x in v.values())) or 1
        return {w: x / norm for w, x in v.items()}

    cv = {k: vec(c) for k, c in docs.items()}
    qv = {k: vec(c) for k, c in qdocs.items()}
    secs = {(m['group'], m['num']): _norm_sec(m['mapsec_name']) for m in maps if m['region'] == 'johto'}
    pairs = []
    for qk, q in qv.items():
        for ck, c in cv.items():
            s = sum(x * c.get(w, 0) for w, x in q.items())
            if s <= 0:
                continue
            if _norm_sec(landmark.get(ck)) == secs[qk]:
                s *= 1.25
            pairs.append((s, qk, ck))
    pairs.sort(reverse=True)
    out = {}
    used = set()
    for s, qk, ck in pairs:
        if s < 0.30 or qk in out or ck in used:
            continue
        out[qk] = (ck, round(s, 2))
        used.add(ck)
    return out


# ---------------------------------------------------------------- roles -------
def role_of(m, dis):
    """A short role for an unnamed map, from what its scripts do."""
    sp = set()
    marts = 0
    gym = m['battle_type'] == 1
    for a, owners in dis.owners.items():
        if (m['group'], m['num']) not in owners or dis.kind.get(a) != 'script':
            continue
        for pc in dis.blocks[a][0]:
            name, args = dis.insn[pc][:2]
            if name == 'special':
                sp.add(SPECIALS[args[0][1]] if args[0][1] < len(SPECIALS) else '')
            if name.startswith('pokemart'):
                marts += 1
    if 'HealPlayerParty' in sp and m['map_type'] == 8:
        return 'PokemonCenter_1F'
    if gym:
        return 'Gym'
    if marts and m['map_type'] == 8:
        return 'Mart'
    if m['map_type'] == 8:
        return 'House'
    if m['map_type'] == 4:
        return 'Cave'
    return ''


def overrides():
    """(group, num) -> folder name, from the hand-checked name files next to this script:
    johto_map_names.json (warp topology + dialogue vs pokecrystal) and kanto_frlg_flags.json
    (FRLG alignment fixes and names for Quetzal's 3 new Kanto maps)."""
    here = os.path.dirname(os.path.abspath(__file__))
    out = {}
    p = os.path.join(here, 'johto_map_names.json')
    if os.path.exists(p):
        j = json.load(open(p))
        for cur, gn in j.get('_group_num', {}).items():
            if cur in j and not cur.startswith('_'):
                g, n = (int(x) for x in gn.split('.'))
                out[(g, n)] = 'Johto_' + j[cur]
    p = os.path.join(here, 'kanto_frlg_flags.json')
    if os.path.exists(p):
        j = json.load(open(p))
        for k, v in j.get('map_name_corrections', {}).items():
            m = re.search(r'\((\d+)\.(\d+)\)', k)
            if m:
                out[(int(m.group(1)), int(m.group(2)))] = 'Kanto_' + v
        for k, v in j.get('map_names', {}).items():
            out[('name', k)] = 'Kanto_' + v
    return out
