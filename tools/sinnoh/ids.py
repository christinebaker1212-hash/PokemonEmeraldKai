"""SINNOH's numbers for Platinum's flags and variables, shared by tools/sinnoh/gen_maps.py (the people's hide flags, the coord
events' variables) and gen_scripts.py (the script pack): every flag / variable name Platinum's scripts and events use,
in name order, from REGION_SINNOH_FLAGS + 8 (after the 8 badges, include/mp_region.h; 0x3328) and from SINNOH_VARS_START.
Only the numbers reach the repository's files; the names stay in the decomp.

Names the game shares with Platinum's scripts as special variables (VAR_RESULT, VAR_0x8000-0x800F...) keep the game's
own numbers.
"""
import json
import os
import re

FLAGS_START = 0x3328          # REGION_SINNOH_FLAGS (0x3320) + the 8 badges (JOHTO / KANTO trainers' flags end at 0x3319)
FLAGS_END = 0x3770            # then the gym leaders' TMs given (tools/sinnoh/gen_words.py), the trainers' from 0x3780
VARS_START = 0x5180           # past Quetzal's region vars (up to 0x514A)
VARS_END = 0x52F0             # (then the game's own SINNOH vars: VAR_SINNOH_ARRIVAL 0x52F0, include/mp_region.h)
SPECIAL_VAR = re.compile(r'^VAR_(0x8[0-9A-Fa-f]{3}|RESULT|FACING|SPECIAL_\w+)$')

_cache = {}


def _names(root):
    if root in _cache:
        return _cache[root]
    flags, vars_ = set(), set()
    sdir = os.path.join(root, 'res/field/scripts')
    for f in os.listdir(sdir):
        if f.endswith('.s'):
            text = open(os.path.join(sdir, f), encoding='utf-8', errors='replace').read()
            flags |= set(re.findall(r'\bFLAG_[A-Z0-9_]+\b', text))
            vars_ |= set(re.findall(r'\bVAR_[A-Za-z0-9_]+\b', text))
    edir = os.path.join(root, 'res/field/events')
    for f in os.listdir(edir):
        if f.endswith('.json'):
            d = json.load(open(os.path.join(edir, f)))
            for o in d.get('object_events', []):
                if isinstance(o.get('hidden_flag'), str) and o['hidden_flag'].startswith('FLAG_'):
                    flags.add(o['hidden_flag'])
            for c in d.get('coord_events', []):
                if isinstance(c.get('var'), str) and c['var'].startswith('VAR_'):
                    vars_.add(c['var'])
    vars_ = {v for v in vars_ if not SPECIAL_VAR.match(v)}
    flags, vars_ = sorted(flags), sorted(vars_)
    assert FLAGS_START + len(flags) <= FLAGS_END, 'too many SINNOH flags'
    assert VARS_START + len(vars_) <= VARS_END, 'too many SINNOH vars'
    _cache[root] = ({n: FLAGS_START + i for i, n in enumerate(flags)}, {n: VARS_START + i for i, n in enumerate(vars_)})
    return _cache[root]


def flag(root, name):
    """The number of a Platinum flag name (None: not one of the scripts' flags)"""
    return _names(root)[0].get(name)


def var(root, name):
    """The number of a Platinum variable name, or the game's own name for a special variable (None: unknown)"""
    if SPECIAL_VAR.match(name):
        return name
    return _names(root)[1].get(name)


def counts(root):
    f, v = _names(root)
    return len(f), len(v)
