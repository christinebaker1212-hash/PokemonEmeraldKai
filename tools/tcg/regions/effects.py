"""What the regional sets' texts do (docs/TCG_REGIONS.md; labels from import_sets.py: "<set>-<number>:a<k>" an
attack, ":p" its power, ":t" a Trainer's / special Energy's rules).

ATTACKS: label -> (kind, amount, extra) or (kind, amount, extra, after), as gb2_effects.py: an FX_* kind of
src/tcg/tcg_effects.c and its numbers. POWERS: label -> TCG_POWER_* name (src/tcg/tcg_powers.c). TRAINERS: label ->
the C handler's name (src/tcg/tcg_trainers.c). RULES: the card kinds whose rules the engine plays (a card needing
another one isn't finished). generate_catalog.py marks a card TCG_CF_READY when everything on it is here;
generate_effects.py adds ATTACKS to tcg_effect_table.inc. scope.py reports what's left.
"""
import re
from gb2_effects import SINGLE_PHASE

ATTACKS = {}
POWERS = {}
TRAINERS = {}
# rules: TcgCard flags (import_sets.py) the engine plays; DARK_ENERGY / METAL_ENERGY: Darkness / Metal costs payable
RULES = {'OWNER', 'DARK', 'LIGHT'}


def table():
    """(label, kind, amount, after, extra) for each attack"""
    out = []
    for name, v in ATTACKS.items():
        kind, amount, extra = v[:3]
        after = v[3] if len(v) > 3 else SINGLE_PHASE.get(kind, 2)
        out.append((name, kind, amount, after, extra))
    return out


def ready(card):
    """Does every rule, attack and power on it play?"""
    if any(f not in RULES for f in card['flags'] if f not in ('DARK', 'LIGHT')):
        return False
    if card['kind'] != 'pokemon':
        return card['effect'] in TRAINERS
    for a in card['attacks']:
        if a['text'] and a['effect'] not in ATTACKS:
            return False
        if a['cost'].get('Darkness') and 'DARK_ENERGY' not in RULES:
            return False
        if a['cost'].get('Metal') and 'METAL_ENERGY' not in RULES:
            return False
    if card.get('ability') and card['ability']['effect'] not in POWERS:
        return False
    return True
