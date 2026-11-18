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
import families

ATTACKS = {}
POWERS = {}
TRAINERS = {}
# rules: TcgCard flags (import_sets.py) the engine plays; DARK_ENERGY / METAL_ENERGY: Darkness / Metal costs payable
RULES = {'OWNER', 'DARK', 'LIGHT', 'DARK_ENERGY', 'METAL_ENERGY', 'SPECIAL', 'SUPPORTER', 'STADIUM', 'TOOL', 'RSM', 'BABY', 'SHINING', 'EX',
         'GAME_CORNER'}
# special Energy whose rules the engine plays (Tcg_EnergyRule, src/tcg/tcg_collection.c)
for _label, _rule in (('neo1-19', 'METAL'), ('neo1-104', 'DARKNESS'), ('neo1-105', 'RECYCLE'), ('ecard1-158', 'DARKNESS'),
                      ('ecard1-159', 'METAL_LATE'), ('ecard2-142', 'DARKNESS'), ('ecard2-143', 'METAL'),
                      ('ecard2-144', 'RAINBOW'), ('ecard2-145', 'BOOST')):
    TRAINERS[_label + ':t'] = 'ENERGY_' + _rule


# Trainers with a handler in src/tcg/tcg_rtrainers.inc (their first printings; a reprint with the same name and rules
# plays as its first printing, a Game Boy games' Trainer's reprint as theirs: generate_catalog.py's canon)
RTRAINERS = '''gym1-15 gym1-16 gym1-17 gym1-18 gym1-104 gym1-105 gym1-106 gym1-108 gym1-109 gym1-110 gym1-111 gym1-113
gym1-114 gym1-115 gym1-118 gym1-121 gym1-122 gym1-123 gym1-125 gym1-126 gym2-17 gym2-19 gym2-20 gym2-103 gym2-105
gym2-109 gym2-113 gym2-118 gym2-120 gym2-123 gym2-124 neo1-83 neo1-85 neo1-86 neo1-88 neo1-93 neo1-94 neo1-96 neo1-97
neo1-99 neo1-101 neo1-103 neo2-73 neo2-74 neo2-75 neo3-60 neo3-64 neo4-92 neo4-96 neo4-97 neo4-98 neo4-101 neo4-103
neo4-104 neo4-105 ecard1-137 ecard1-138 ecard1-139 ecard1-140 ecard1-141 ecard1-142 ecard1-145 ecard1-146 ecard1-147
ecard1-148 ecard1-149 ecard1-150 ecard2-120 ecard2-125 ecard2-126 ecard2-130 ecard2-131 ecard2-133 ecard2-134 ecard2-136
ecard2-137 ecard2-141 ecard3-120 ecard3-121 ecard3-122 ecard3-123 ecard3-124 ecard3-125 ecard3-127 ecard3-128 ecard3-132
ecard3-138 np-26 np-27 np-36'''.split()


def _trainers():
    import json
    from pathlib import Path
    root = Path(__file__).resolve().parents[3]
    gb = {c['name'] for c in json.loads((root / 'tools/tcg/catalog.json').read_text())['cards'] if c['kind'] == 'trainer'}
    first = {}
    for c in json.loads((root / 'tools/tcg/regions/cards.json').read_text())['cards']:
        if c['kind'] != 'trainer':
            continue
        head = first.setdefault((c['name'], c['text']), c['src'])
        if c['name'] in gb or head in RTRAINERS:
            TRAINERS[c['effect']] = 'C'


_trainers()


def _powers():
    import json
    from pathlib import Path
    import powers
    root = Path(__file__).resolve().parents[3]
    for c in json.loads((root / 'tools/tcg/regions/cards.json').read_text())['cards']:
        d = powers.match(c)
        if d:
            POWERS[c['ability']['effect']] = d['id']


_powers()


def table():
    """(label, kind, amount, after, extra[, script]) for each attack (a script: ('SCRIPT', script))"""
    out = []
    for name, v in ATTACKS.items():
        if v[0] == 'SCRIPT':
            out.append((name, 'SCRIPT', 0, 2, 0, v[1]))
            continue
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


# ---------------------------------------------------------------------------------------------- learned wordings
# Today's cards' attacks teach the wordings the engine already plays: a template is the text with the card's own name,
# its numbers and type words as slots; the row's amount / extra come from the slot that holds the same value (or are
# constants). A regional text with the same template gets the same kind with its own numbers.
TYPE_WORDS = ['Fire', 'Grass', 'Lightning', 'Water', 'Fighting', 'Psychic', 'Colorless', 'Darkness', 'Metal']


def template(text, name):
    """(template, numbers, types): the text with the card's name as SELF, numbers as #, type words as @"""
    t = text.replace('Pokémon', 'Pokemon').replace('é', 'e').replace('’', "'")
    for n in sorted({name, name.split()[-1], name.replace('Dark ', '').replace('Light ', '')}, key=len, reverse=True):
        if n:
            t = re.sub(r"\b" + re.escape(n) + r"\b", 'SELF', t)
    t = re.sub(r"\b(this Pokemon|this card)\b", 'SELF', t, flags=re.I)
    nums = [int(n) for n in re.findall(r'\d+', t)]
    t = re.sub(r'\d+', '#', t)
    types = re.findall(r'\b(' + '|'.join(TYPE_WORDS) + r')\b', t)
    t = re.sub(r'\b(' + '|'.join(TYPE_WORDS) + r')\b', '@', t)
    t = re.sub(r'\s+', ' ', t.strip().lower())
    return t, nums, [TYPE_WORDS.index(x) for x in types]


def _spec(value, nums, types, typed):
    if typed and value in types and value != 15:
        return ('type', types.index(value))
    if value in nums and value:
        return ('num', nums.index(value))
    return ('const', value)


def learn(catalog_rows):
    """catalog_rows: (card name, attack text, base damage, (kind, amount, after, extra)) of today's cards ->
    {template: spec} (a wording played two ways: the way most of its cards play it, if two thirds of them do)"""
    import collections
    votes = collections.defaultdict(collections.Counter)
    for name, text, damage, (kind, amount, after, extra) in catalog_rows:
        t, nums, types = template(text, name)
        typed = kind in TYPED_EXTRA
        spec = (kind, _spec(amount, nums, [], False), after, _spec(extra, nums, types, typed))
        # (the numbers no parameter reads must match the example's: a constant is a code, not the text's number)
        used = {s[1] for s in (spec[1], spec[3]) if s[0] == 'num'}
        spec = spec + (tuple((i, 'base' if v == damage and damage else v) for i, v in enumerate(nums) if i not in used),)
        votes[t][spec] += 1
    out = {}
    for t, v in votes.items():
        spec, n = v.most_common(1)[0]
        if n * 3 >= sum(v.values()) * 2:
            out[t] = spec
    return out


# kinds whose extra is a type (TCG_FIRE...): learned from the type words
TYPED_EXTRA = {'DISCARD_TYPED', 'TYPE_DOUBLE', 'ENERGY_SEARCH', 'WATER_EXTRA', 'WATER_EXTRA2'}


def apply(spec, text, name, damage):
    kind, amount, after, extra, fixed = spec
    t, nums, types = template(text, name)
    if any(i >= len(nums) or nums[i] != (damage if v == 'base' else v) for i, v in fixed):
        return None

    def val(s):
        k, v = s
        if k == 'const':
            return v
        src = types if k == 'type' else nums
        return src[v] if v < len(src) else None
    a, e = val(amount), val(extra)
    if a is None or e is None:
        return None
    return (kind, a, after, e)


def _today():
    """today's cards' attacks with their rows (src/tcg/tcg_catalog.c, tcg_effect_table.inc: Game Boy rows only)"""
    from pathlib import Path
    root = Path(__file__).resolve().parents[3]
    rows = {}
    for name, kind, amount, after, extra in re.findall(r'\{"(\w+)", FX_(\w+),(\d+),(\d+),(\d+)\}',
                                                     (root / 'src/tcg/tcg_effect_table.inc').read_text()):
        rows[name] = (kind, int(amount), int(after), int(extra))
    src = (root / 'src/tcg/tcg_catalog.c').read_text()
    out = []
    for block in re.split(r'\n\[\d+\] = ', src)[1:]:
        m = re.match(r'\{\.id=(\d+), \.name="([^"]*)"', block)
        if not m or int(m.group(1)) > 442:
            continue
        for desc, eff, dmg in re.findall(r'\.description="((?:[^"\\]|\\.)*)",\.effect="(\w*)",\.animation="[^"]*",\.cost=\{[^}]*\},\.damage=(\d+)', block):
            if eff in rows and desc:
                out.append((m.group(2), desc.replace('\\"', '"'), int(dmg), rows[eff]))
    return out


def autofill(cards):
    """ATTACKS for the regional attacks whose wording today's cards already play (hand-written entries win)"""
    learned = learn(_today())
    n = 0
    for c in cards:
        for a in c.get('attacks', []):
            if a['text'] and a['effect'] not in ATTACKS:
                t = template(a['text'], c['name'])[0]
                if t in learned:
                    row = apply(learned[t], a['text'], c['name'], a['damage'])
                    if row:
                        kind, amount, after, extra = row
                        ATTACKS[a['effect']] = (kind, amount, extra, after)
                        n += 1
                        continue
                fam = families.script(a['text'], c['name'], a['damage'])
                if fam:
                    ATTACKS[a['effect']] = fam
                    n += 1
    return n


# (the generators read ATTACKS after this: today's wordings first, then what's written above wins)
def _load():
    import json
    from pathlib import Path
    cards = json.loads((Path(__file__).resolve().parent / 'cards.json').read_text())['cards']
    autofill(cards)


_load()
