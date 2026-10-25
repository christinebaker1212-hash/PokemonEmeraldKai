#!/usr/bin/env python3
"""Cap the Quetzal rip to a generation: no Pokémon, move or item from a later one.

    python3 tools/quetzal_rip/gencap.py [--max-gen 4] [--out quetzal_rip/gen4]

Reads the rip (quetzal_rip/data, scripts, maps) and never changes it: Quetzal's own, modern data stays there for
later. Writes OUT/:
  data/trainers.json, data/wild_encounters.json, data/gifts.json   same format as quetzal_rip/data
  overlay/<path>      every rip file (scripts.inc, map.json, shared .inc) that changed, at its rip path: copy the
                      overlay over quetzal_rip/ to get the capped game
  changes.json        every change, with the reason
  REPORT.md           a summary for people

How a Pokémon from a later generation is replaced, in order:
  1. its form: a form of an older Pokémon (Alolan Marowak, Mega Mewtwo, Galarian Ponyta) becomes that Pokémon;
  2. its family: later evolutions or relatives of an older Pokémon (Sylveon -> Eevee, Annihilape -> Primeape,
     Kleavor -> Scyther, Great Tusk -> Donphan; FAMILY below);
  3. the reference games, for the same place: Crystal for Johto, FireRed / Red (and Crystal's Kanto) for Kanto.
     Wild slots take a Pokémon from the same map and the same kind of encounter there; trainers take one from
     the same trainer (matched by name and class), then from trainers of the same class; trades take one from
     the trade at the same house. Among those, the most types in common, then the closest base stat total;
  4. anything up to the cap with a type in common and the closest base stat total, not legendary unless the
     original was, and able to exist at that level (no Lv 10 Garchomp).
Trainers' Pokémon then evolve as far as their level allows, and their moves are checked against the new
species' Gen 4 learnset (pokeplatinum: level up to its level, TM, tutor, egg, and its pre-evolutions'), the gaps
filled with its latest level-up moves. Static legendaries from later generations are scrapped (their entries
marked, their overworld objects hidden with FLAG_QZ_GEN_CAPPED, which the port keeps set), and so are later
forms that duplicate a kept legendary (Galarian birds). Starter choices keep only the generations allowed.
Items: later items leave marts; elsewhere they become an older item (ITEM_SUBST); items Kai has stay (Kai's
systems take precedence), and so do HGSS-era story items (KEEP_ITEMS). Quetzal's own items (IV Up, Gender Pill...)
are dropped and listed.

Data: tools/moemon_gen4/gen4_data.json (pokeplatinum, Gen 1-4), tools/quetzal_rip/gen_refs.json (Red, Crystal,
FireRed), quetzal_rip/data/names.json (Quetzal's species / moves / items). Deterministic: same input, same output.
"""
import argparse
import copy
import glob
import json
import os
import re
import shutil

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
RIP = os.path.join(REPO, 'quetzal_rip')

GEN_SPECIES = [151, 251, 386, 493, 649, 721, 809, 898]     # last national dex number of gens 1-8
GEN_MOVES = [165, 251, 354, 467, 559, 621, 742, 826]


def key(s):
    s = re.sub(r'^(MAP_JOHTO_|MAP_KANTO_|MAP_|SPECIES_|MOVE_|ITEM_|TRAINER_CLASS_)', '', s or '')
    s = s.replace('♀', 'F').replace('♂', 'M').replace('é', 'e').replace('É', 'E').replace('{PKMN}', 'PKMN')
    return re.sub(r'[^a-z0-9]', '', s.lower())


def gen_of(n, table):
    for g, last in enumerate(table, 1):
        if n <= last:
            return g
    return 9


# Later Pokémon that belong to an older family (step 2). Values are national names (keys).
FAMILY = {
    'sylveon': 'eevee', 'perrserker': 'persian', 'sirfetchd': 'farfetchd', 'mrrime': 'mrmime', 'cursola': 'corsola',
    'obstagoon': 'linoone', 'annihilape': 'primeape', 'farigiraf': 'girafarig', 'dudunsparce': 'dunsparce',
    'clodsire': 'quagsire', 'wyrdeer': 'stantler', 'kleavor': 'scizor', 'ursaluna': 'ursaring', 'sneasler': 'weavile',
    'overqwil': 'qwilfish', 'kingambit': 'absol', 'greattusk': 'donphan', 'irontreads': 'donphan',
    'screamtail': 'jigglypuff', 'fluttermane': 'mismagius', 'sandyshocks': 'magneton', 'ironbundle': 'delibird',
    'ironhands': 'hariyama', 'ironthorns': 'tyranitar', 'roaringmoon': 'salamence', 'ironvaliant': 'gallade',
    'walkingwake': 'suicune', 'ragingbolt': 'raikou', 'gougingfire': 'entei', 'brutebonnet': 'breloom',
    'slitherwing': 'ninjask', 'ironmoth': 'ninjask', 'ironjugulis': 'salamence', 'irontreads2': 'donphan',
    'wochien': 'celebi', 'chienpao': 'weavile', 'tinglu': 'donphan', 'chiyu': 'houndoom', 'meowscarada': 'meowth',
    'mimejr2': 'mimejr', 'basculegion': 'gyarados', 'enamorus': 'clefable', 'flittle': 'natu', 'espathra': 'xatu',
}
# Forms that existed in Gen 4: kept as the base species, with a note (the port decides how to show the form).
GEN4_FORMS = {'arceus', 'rotom', 'deoxys', 'wormadam', 'giratina', 'shaymin', 'unown', 'castform', 'burmy',
              'shellos', 'gastrodon', 'cherrim', 'pichu'}
LEGENDARY = {144, 145, 146, 150, 151, 243, 244, 245, 249, 250, 251, 377, 378, 379, 380, 381, 382, 383, 384, 385,
             386, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493}
LEGENDARY |= set(range(494, 495)) | set(range(638, 650)) | set(range(716, 722)) | set(range(772, 774)) \
    | set(range(785, 810)) | set(range(888, 899))

KEEP_ITEMS = {'sportball', 'gsball', 'tidalbell', 'clearbell', 'squirtbottle', 'radio', 'silverwing', 'rainbowwing',
              'pass', 'redscale', 'mysteryegg', 'redapricorn', 'blueapricorn', 'yellowapricorn', 'greenapricorn',
              'pinkapricorn', 'whiteapricorn', 'blackapricorn'}
ITEM_ALIASES = {'xdefense': 'xdefend', 'xspatk': 'xspecial', 'leek': 'stick', 'dowsingmachine': 'itemfinder',
                'keytoroom1': 'room1key', 'keytoroom2': 'room2key', 'keytoroom4': 'room4key',
                'keytoroom6': 'room6key', 'devonparts': 'devongoods', 'parlyzheal': 'paralyzeheal',
                'thunderstone': 'thunderstone', 'kingsrock': 'kingsrock', 'blackglasses': 'blackglasses'}
QUETZAL_ITEMS = {'ivup', 'ivmax', 'genderpill', 'maxcandy'}
TYPE_ITEMS = {'normal': 'silkscarf', 'fire': 'charcoal', 'water': 'mysticwater', 'electric': 'magnet',
              'grass': 'miracleseed', 'ice': 'nevermeltice', 'fighting': 'blackbelt', 'poison': 'poisonbarb',
              'ground': 'softsand', 'flying': 'sharpbeak', 'psychic': 'twistedspoon', 'bug': 'silverpowder',
              'rock': 'hardstone', 'ghost': 'spelltag', 'dragon': 'dragonfang', 'dark': 'blackglasses',
              'steel': 'metalcoat', 'fairy': 'silkscarf'}
ITEM_SUBST = {
    'assaultvest': 'leftovers', 'rockyhelmet': 'leftovers', 'heavydutyboots': 'leftovers', 'covertcloak': 'leftovers',
    'clearamulet': 'leftovers', 'abilityshield': 'leftovers', 'safetygoggles': 'leftovers', 'airballoon': 'leftovers',
    'protectivepads': 'leftovers', 'utilityumbrella': 'leftovers', 'ejectbutton': 'leftovers', 'ejectpack': 'leftovers',
    'redcard': 'leftovers', 'weaknesspolicy': 'lifeorb', 'blunderpolicy': 'widelens', 'loadeddice': 'kingsrock',
    'throatspray': 'lifeorb', 'punchingglove': 'lifeorb', 'boosterenergy': 'choiceband', 'mirrorherb': 'whiteherb',
    'roomservice': 'laggingtail', 'terrainextender': 'lightclay', 'adrenalineorb': 'sitrusberry',
    'floatstone': 'laggingtail', 'ringtarget': 'laggingtail', 'bindingband': 'gripclaw', 'absorbbulb': 'sitrusberry',
    'cellbattery': 'sitrusberry', 'luminousmoss': 'sitrusberry', 'snowball': 'sitrusberry',
    'electricseed': 'sitrusberry', 'psychicseed': 'sitrusberry', 'mistyseed': 'sitrusberry', 'grassyseed': 'sitrusberry',
    'roseliberry': 'sitrusberry', 'keeberry': 'sitrusberry', 'marangaberry': 'sitrusberry',
    'healthfeather': 'hpup', 'musclefeather': 'protein', 'resistfeather': 'iron', 'geniusfeather': 'calcium',
    'cleverfeather': 'zinc', 'swiftfeather': 'carbos', 'healthmochi': 'hpup', 'musclemochi': 'protein',
    'resistmochi': 'iron', 'geniusmochi': 'calcium', 'clevermochi': 'zinc', 'swiftmochi': 'carbos',
    'expcandyxs': 'rarecandy', 'expcandys': 'rarecandy', 'expcandym': 'rarecandy', 'expcandyl': 'rarecandy',
    'expcandyxl': 'rarecandy', 'gigacandy': 'rarecandy', 'bottlecap': 'ppmax', 'goldbottlecap': 'ppmax',
    'abilitycapsule': 'ppup', 'abilitypatch': 'ppmax', 'bignugget': 'nugget', 'pearlstring': 'bigpearl',
    'cometshard': 'starpiece', 'balmmushroom': 'bigmushroom', 'maxmushrooms': 'bigmushroom', 'prettyfeather': 'pearl',
    'remedy': 'potion', 'fineremedy': 'superpotion', 'superbremedy': 'hyperpotion', 'beastball': 'ultraball',
    'radiantball': 'ultraball', 'lure': 'repel', 'superlure': 'superrepel', 'maxlure': 'maxrepel',
    'sweetheart': 'lavacookie', 'maxhoney': 'honey', 'icestone': 'nevermeltice', 'coverfossil': 'domefossil',
    'plumefossil': 'oldamber', 'jawfossil': 'helixfossil', 'sailfossil': 'domefossil', 'prismscale': 'dragonscale',
    'whippeddream': 'lavacookie', 'sachet': 'lavacookie', 'shinycharm': 'nugget', 'ovalcharm': 'nugget',
    'pixieplate': 'silkscarf',
}
ITEM_DEFAULT_HELD = 'sitrusberry'
ITEM_DEFAULT = 'nugget'
FLAG_CAPPED = 'FLAG_QZ_GEN_CAPPED'
TRAINER_CLASSES = {
    'crystal': {'leader': ['falkner', 'whitney', 'bugsy', 'morty', 'pryce', 'jasmine', 'chuck', 'clair', 'brock',
                           'misty', 'ltsurge', 'erika', 'janine', 'sabrina', 'blaine', 'blue'],
                'elitefour': ['will', 'koga', 'bruno', 'karen'], 'champion': ['champion', 'red'],
                'rival': ['rival1', 'rival2'], 'teamrocket': ['gruntm', 'gruntf'], 'rocketadmin': ['executivem', 'executivef'],
                'cooltrainer': ['cooltrainerm', 'cooltrainerf'], 'fisherman': ['fisher'], 'pokefan': ['pokefanm', 'pokefanf'],
                'schoolkid': ['schoolboy'], 'hexmaniac': ['medium'], 'policeman': ['officer'],
                'mysteryman': ['mysticalman'], 'pkmntrainer': ['pkmntrainer', 'red'], 'dragontamer': ['cooltrainerm', 'cooltrainerf'],
                'expert': ['blackbelt'], 'parasollady': ['beauty'], 'youngcouple': ['twins'], 'aqualeader': ['executivem'],
                'magmaleader': ['executivem'], 'collector': ['pokemaniac']},
    'firered': {'rival': ['rivalearly', 'rivallate'], 'champion': ['champion', 'rivallate'],
                'hexmaniac': ['channeler'], 'aqualeader': ['rocketboss'], 'magmaleader': ['rocketboss'],
                'triathlete': ['swimmerm', 'swimmerf'], 'pkmntrainer': ['pkmntrainer2', 'pkmntrainer3']},
    'red': {'teamrocket': ['rocket'], 'leader': ['brock', 'misty', 'ltsurge', 'erika', 'koga', 'sabrina', 'blaine',
                                                 'giovanni'], 'elitefour': ['lorelei', 'bruno', 'agatha', 'lance'],
            'rival': ['rival1', 'rival2', 'rival3'], 'champion': ['rival3'], 'hexmaniac': ['channeler'],
            'cooltrainer': ['cooltrainerm', 'cooltrainerf'], 'swimmerm': ['swimmer'], 'swimmerf': ['beauty'],
            'fisherman': ['fisher'], 'blackbelt': ['blackbelt'], 'pokemaniac': ['pokemaniac']},
}
TRADE_MAPS = {'goldenroddeptstore6f': 'goldenroddeptstore5f'}     # Quetzal moved Mike's trade up a floor
WILD_KINDS = {'land_mons': 'land', 'water_mons': 'water', 'fishing_mons': 'fish', 'rock_smash_mons': 'rock',
              'headbutt_mons': 'headbutt'}


def load(p):
    with open(p, encoding='utf-8') as fh:
        return json.load(fh)


class Capper:
    def __init__(self, max_gen):
        self.max_gen = max_gen
        self.changes = []
        self.events = set()     # (file, symbol, kind): events built on content past the cap
        g4 = load(os.path.join(REPO, 'tools', 'moemon_gen4', 'gen4_data.json'))
        self.refs = load(os.path.join(HERE, 'gen_refs.json'))
        names = load(os.path.join(RIP, 'data', 'names.json'))
        # Quetzal species
        self.qsp = {}           # const -> dict
        first = {}
        for i, c, n, types, bst in names['species']:
            if i == 0:
                continue
            d = {'id': i, 'const': c, 'name': n, 'types': [t.lower() for t in types], 'bst': bst}
            if i <= GEN_SPECIES[-1]:
                first.setdefault(key(n), i)
            self.qsp[c] = d
        self.by_id = {d['id']: d for d in self.qsp.values()}
        for d in self.qsp.values():
            i = d['id']
            base = i if i <= GEN_SPECIES[-1] else first.get(key(d['name']))
            d['base'] = base
            d['form'] = base is not None and base != i
            d['gen'] = gen_of(base, GEN_SPECIES) if base else 9
        # Platinum species by key (Gen 1-4 truth)
        self.plat = {key(s['const']): s for s in g4['species']}
        self.plat_by_nat = {s['national']: s for s in g4['species']}
        self.prevo = {}
        for s in g4['species']:
            for e in s['evolutions']:
                self.prevo.setdefault(key(e[-1]), (key(s['const']), e))
        # Quetzal const for a national number (first const with that id)
        self.qconst_by_nat = {d['id']: d['const'] for d in self.qsp.values() if d['id'] <= GEN_SPECIES[-1]}
        # moves
        self.qmove = {c: (i, n) for i, c, n in names['moves']}
        self.qmove_by_key = {}
        for i, c, n in names['moves']:
            self.qmove_by_key.setdefault(key(n), c)
            self.qmove_by_key.setdefault(key(c), c)
        self.pmove = {key(m['name']): m for m in g4['moves']}
        self.pmove_alias = {'highjumpkick': 'hijumpkick', 'feintattack': 'faintattack', 'smellingsalts': 'smellingsalt',
                            'visegrip': 'vicegrip', 'selfdestruct': 'selfdestruct', 'thunderpunch': 'thunderpunch',
                            'solarbeam': 'solarbeam', 'softboiled': 'softboiled', 'doubleslap': 'doubleslap'}
        # items
        self.qitem = {c: (i, n) for i, c, n in names['items']}
        self.qitem_by_key = {}
        for i, c, n in names['items']:
            if n:
                self.qitem_by_key.setdefault(key(n), c)
                self.qitem_by_key.setdefault(key(c), c)
        self.pitems = {key(x['name']) for x in g4['items'] if x.get('name')}
        self.kai_items = {key(m) for m in re.findall(r'#define ITEM_(\w+)', open(os.path.join(REPO, 'include', 'constants', 'items.h')).read())}

    # ---------------------------------------------------------------- classification
    def note(self, where, what, old, new, why):
        self.changes.append({'where': where, 'what': what, 'from': old, 'to': new, 'why': why})

    def sp_ok(self, const):
        d = self.qsp.get(const)
        return d is None or (d['gen'] <= self.max_gen and not d['form'])

    def plat_of(self, k):
        return self.plat.get(k)

    def info(self, k):
        """types / bst / national for a species key, Gen 4 data first."""
        p = self.plat.get(k)
        if p:
            return {'types': [t[5:].lower() for t in p['types']], 'bst': p['bst'], 'national': p['national']}
        c = self.qconst_by_key(k)
        d = self.qsp.get(c)
        if d:
            return {'types': d['types'], 'bst': d['bst'], 'national': d['base'] or 9999}
        return None

    def qconst_by_key(self, k):
        if not hasattr(self, '_qk'):
            self._qk = {}
            for d in sorted(self.qsp.values(), key=lambda x: x['id']):
                self._qk.setdefault(key(d['name']), d['const'])
                self._qk.setdefault(key(d['const']), d['const'])
        return self._qk.get(k)

    def const_for(self, k):
        p = self.plat.get(k)
        if p:
            return self.qconst_by_nat[p['national']]
        return self.qconst_by_key(k)

    def allowed_key(self, k):
        i = self.info(k)
        return i is not None and gen_of(i['national'], GEN_SPECIES) <= self.max_gen

    def min_level(self, k, depth=0):
        """lowest level a Pokémon can have in the wild or on a trainer, from its Gen 4 evolution chain."""
        if depth > 4 or k not in self.prevo:
            return 1
        pk, e = self.prevo[k]
        base = self.min_level(pk, depth + 1)
        if e[0].startswith('EVO_LEVEL') and len(e) == 3 and isinstance(e[1], int):
            return max(base, e[1])
        return max(base, 20 if 'HAPPINESS' in e[0] else 30)

    def score(self, cand, orig_types, orig_bst):
        i = self.info(cand)
        common = len(set(i['types']) & set(orig_types))
        return (common, -abs(i['bst'] - orig_bst))

    def pick(self, pool, orig, avoid, level=None, need_type=False, legendary_ok=False):
        d = self.qsp[orig]
        best = None
        for k in pool:
            if not self.allowed_key(k) or k in avoid:
                continue
            i = self.info(k)
            if not legendary_ok and i['national'] in LEGENDARY:
                continue
            if level is not None and self.min_level(k) > level + 5:
                continue
            s = self.score(k, d['types'], d['bst'])
            if need_type and s[0] == 0:
                continue
            if best is None or s > best[0]:
                best = (s, k)
        return best[1] if best else None

    def everything(self):
        return [key(s['const']) for s in self.plat.values()
                if gen_of(s['national'], GEN_SPECIES) <= self.max_gen]

    def cap_species(self, const, where, pools=(), avoid=(), level=None, memo=None):
        """const -> capped const. pools: list of (name, [keys]) tried in order (step 3)."""
        if const not in self.qsp or self.sp_ok(const):
            return const
        if memo is not None and const in memo:
            return memo[const]
        d = self.qsp[const]
        new, why = None, None
        if d['base'] and d['gen'] <= self.max_gen:
            new = self.qconst_by_nat[d['base']]
            why = 'form of %s' % self.by_id[d['base']]['name'] + (' (a Gen 4 form)' if key(d['name']) in GEN4_FORMS else '')
        if new is None:
            fam = FAMILY.get(key(d['name']))
            if fam and self.allowed_key(fam) and fam not in avoid:
                new, why = self.const_for(fam), 'same family'
        legendary = (d['base'] or 0) in LEGENDARY
        if new is None:
            for name, pool in pools:
                same_place = name == 'trainer' or name.startswith('reference: same map') or name.startswith('reference: trade')
                k = self.pick(pool, const, set(avoid), level, need_type=not same_place, legendary_ok=legendary)
                if k:
                    new, why = self.const_for(k), name
                    break
        if new is None:
            k = (self.pick(self.everything(), const, set(avoid), level, need_type=True, legendary_ok=legendary)
                 or self.pick(self.everything(), const, set(avoid), level, legendary_ok=legendary)
                 or self.pick(self.everything(), const, set(), level, legendary_ok=True))
            new, why = self.const_for(k), 'closest type and stats'
        self.note(where, 'species', const, new, why)
        if memo is not None:
            memo[const] = new
        return new

    # ---------------------------------------------------------------- moves
    def move_gen(self, const):
        i = self.qmove.get(const, (0,))[0]
        return gen_of(i, GEN_MOVES) if i else 0

    def learnable(self, k, level):
        out = set()
        seen = 0
        while k and seen < 5:
            p = self.plat.get(k)
            if not p:
                break
            ls = p['learnset']
            out |= {key(m) for lv, m in ls['level'] if level is None or lv <= level}
            out |= {key(self.pmove_name(t)) for t in ls['tm']}
            out |= {key(m) for m in ls['tutor']} | {key(m) for m in ls['egg']}
            k = self.prevo.get(k, (None,))[0]
            seen += 1
        return out

    def pmove_name(self, tm):
        return self._tm.get(tm, '') if hasattr(self, '_tm') else ''

    def cap_moves(self, moves, species_const, level, where):
        k = self.species_key(species_const)
        p = self.plat.get(k)
        legal = self.learnable(k, level) if p else None
        out = []
        for m in moves:
            if m == 'MOVE_NONE':
                continue
            mk = key(self.qmove.get(m, (0, m))[1])
            mk = self.pmove_alias.get(mk, mk)
            if self.move_gen(m) > self.max_gen:
                self.note(where, 'move', m, None, 'Gen %d move' % self.move_gen(m))
                continue
            if legal is not None and mk not in legal:
                self.note(where, 'move', m, None, '%s cannot learn it in Gen 4' % species_const)
                continue
            out.append(m)
        if p and len(out) < len([m for m in moves if m != 'MOVE_NONE']):
            want = len([m for m in moves if m != 'MOVE_NONE'])
            for lv, m in reversed(p['learnset']['level']):
                if lv > level or len(out) >= want:
                    continue
                c = self.qmove_by_key.get(key(self.pmove[key(m[5:])]['name'] if key(m[5:]) in self.pmove else m))
                if c is None:
                    c = self.qmove_by_key.get(self.pmove_alias.get(key(m), key(m)))
                if c and c not in out:
                    out.append(c)
                    self.note(where, 'move', None, c, 'level-up move of %s' % species_const)
        while len(out) < 4:
            out.append('MOVE_NONE')
        return out[:4]

    def species_key(self, const):
        d = self.qsp.get(const)
        return key(self.by_id[d['base']]['name']) if d and d['base'] else key(const)

    def evolve(self, const, level, where):
        """as far as the level allows (level evolutions; stones / trade / friendship from Lv 32 / 36 / 25)."""
        k = self.species_key(const)
        for _ in range(3):
            p = self.plat.get(k)
            if not p or not p['evolutions']:
                break
            nxt = None
            for e in p['evolutions']:
                tk = key(e[-1])
                if not self.allowed_key(tk):
                    continue
                if e[0] == 'EVO_LEVEL' and e[1] <= level:
                    nxt = tk
                elif e[0] in ('EVO_LEVEL_ATK_GT_DEF', 'EVO_LEVEL_ATK_LT_DEF', 'EVO_LEVEL_ATK_EQ_DEF',
                              'EVO_LEVEL_PID_LOW', 'EVO_LEVEL_FEMALE', 'EVO_LEVEL_NINJASK') and e[1] <= level:
                    nxt = tk
                elif e[0].startswith('EVO_USE_ITEM') and level >= 32:
                    nxt = tk
                elif e[0].startswith('EVO_TRADE') and level >= 36:
                    nxt = tk
                elif 'HAPPINESS' in e[0] and level >= 25:
                    nxt = tk
                if nxt:
                    break
            if not nxt:
                break
            k = nxt
        new = self.const_for(k)
        if new != const and new:
            self.note(where, 'species', const, new, 'evolved for Lv %d' % level)
            return new
        return const

    # ---------------------------------------------------------------- items
    def item_ok(self, const):
        if const in (None, 'ITEM_NONE') or const not in self.qitem:
            return True
        n = self.qitem[const][1]
        k = key(n) if n else key(const)
        k = ITEM_ALIASES.get(k, k)
        if not n:
            return True
        return k in self.pitems or k in self.kai_items or k in KEEP_ITEMS

    def cap_item(self, const, where, held=False):
        if self.item_ok(const):
            n = self.qitem.get(const, (0, ''))[1]
            k = key(n) if n else ''
            if k in ITEM_ALIASES and ITEM_ALIASES[k] != k:
                new = 'ITEM_' + self.kai_const(ITEM_ALIASES[k])
                self.note(where, 'item', const, new, "Kai's name")
                return new
            return const
        k = key(self.qitem[const][1])
        if k in QUETZAL_ITEMS:
            why = "Quetzal's own item"
        else:
            why = 'later item'
        sub = ITEM_SUBST.get(k)
        if sub is None:
            m = re.match(r'(normal|fire|water|electric|grass|ice|fighting|poison|ground|flying|psychic|bug|rock|ghost|'
                         r'dragon|dark|steel|fairy)(gem|memory|teraShard|terashard)', k)
            mz = re.match(r'(normal|fir|water|electr|grass|ic|fighti|poison|ground|flyi|psych|bug|rock|ghost|dragon|'
                          r'dark|steel|fair)i?umz$', k)
            if m:
                sub = TYPE_ITEMS[m.group(1)]
            elif mz:
                t = {'fir': 'fire', 'electr': 'electric', 'ic': 'ice', 'fighti': 'fighting', 'flyi': 'flying',
                     'psych': 'psychic', 'fair': 'fairy'}.get(mz.group(1), mz.group(1))
                sub = TYPE_ITEMS.get(t)
        if sub is None:
            sub = ITEM_DEFAULT_HELD if held else ITEM_DEFAULT
        new = self.qitem_by_key.get(sub) or 'ITEM_' + self.kai_const(sub)
        self.note(where, 'item', const, new, why)
        return new

    def kai_const(self, k):
        if not hasattr(self, '_kai'):
            self._kai = {key(m): m for m in re.findall(r'#define ITEM_(\w+)', open(os.path.join(REPO, 'include', 'constants', 'items.h')).read())}
        return self._kai.get(k, k.upper())

    # ---------------------------------------------------------------- reference pools
    def wild_pool(self, region, mapname, kind):
        games = ['crystal'] if region == 'johto' else ['firered', 'red', 'crystal']
        mk = key(mapname)
        pools = []
        here = []
        for g in games:
            for k in self.refs['wild'][g].get(mk, {}).get(kind, []):
                if k not in here:
                    here.append(k)
        if kind == 'rock' and region == 'johto':
            here += [k for k in self.refs['wild']['crystal']['_any']['rock'] if k not in here]
        if here:
            pools.append(('reference: same map', here))
        allk = []
        for g in games:
            for m, kinds in self.refs['wild'][g].items():
                for k in kinds.get(kind, []):
                    if k not in allk:
                        allk.append(k)
        pools.append(('reference: same region and encounter kind', allk))
        return pools

    def trainer_refs(self, t):
        region = t['region']
        games = ['crystal'] if region == 'johto' else ['firered', 'red']
        ck = key(t['class_name'])
        nk = key(t['name'])
        lvl = max((m['lvl'] for m in t['party']), default=0)
        match, class_pool = None, []
        for g in games:
            classes = set(TRAINER_CLASSES[g].get(ck, [])) | {ck}
            same_class = [r for r in self.refs['trainers'][g] if key(r['class']) in classes]
            for r in same_class:
                for lv, s in r['party']:
                    if s not in class_pool:
                        class_pool.append(s)
            if match is None and nk:
                named = [r for r in self.refs['trainers'][g] if key(r['name']) == nk and r['party']]
                named.sort(key=lambda r: (key(r['class']) not in classes,
                                          abs(max(lv for lv, s in r['party']) - lvl)))
                if named:
                    match = (g, named[0])
        pools = []
        if match:
            pools.append(('trainer', [s for lv, s in match[1]['party']]))
        if class_pool:
            pools.append(('reference: trainers of the same class', class_pool))
        return pools, match

    def trade_pool(self, used_by):
        out = []
        for u in used_by:
            mk = key(u.split('/')[1].split('_', 1)[1]) if '/' in u else key(u)
            mk = TRADE_MAPS.get(mk, mk)
            for g in ('crystal', 'firered'):
                for a, b in self.refs['trades'][g].get(mk, []):
                    for k in (b, a):
                        if k not in out:
                            out.append(k)
        return [('reference: trade in the same house', out)] if out else []


def region_of_map(m):
    return 'johto' if m.startswith('MAP_JOHTO') or m.startswith('Johto') else 'kanto'


def cap_wild(c, wild):
    w = copy.deepcopy(wild)
    for grp in w['wild_encounter_groups']:
        for e in grp['encounters']:
            region = region_of_map(e['map'])
            for field, kind in WILD_KINDS.items():
                if field not in e:
                    continue
                mons = e[field]['mons']
                memo = {}
                present = {c.species_key(m['species']) for m in mons if c.sp_ok(m['species'])}
                pools = c.wild_pool(region, e['map'], kind)
                for i, m in enumerate(mons):
                    where = '%s %s slot %d' % (e['map'], field, i)
                    new = c.cap_species(m['species'], where, pools, avoid=present, level=m['max_level'], memo=memo)
                    if new != m['species']:
                        present.add(c.species_key(new))
                        m['species'] = new
    return w


def cap_trainers(c, data):
    d = copy.deepcopy(data)
    for t in d['trainers']:
        if t['region'] not in ('johto', 'kanto'):
            continue
        where0 = '%s %s %s' % (t['const'], t['class_name'], t['name'])
        pools, match = c.trainer_refs(t)
        if match:
            t['gencap_reference'] = '%s: %s %s' % (match[0], match[1]['class'], match[1]['name'])
        t['items'] = [c.cap_item(i, where0 + ' bag') for i in t['items']]
        party_keys = {c.species_key(m['species']) for m in t['party'] if c.sp_ok(m['species'])}
        for i, m in enumerate(t['party']):
            where = '%s #%d' % (where0, i + 1)
            sp = m['species']
            new = c.cap_species(sp, where, pools, avoid=party_keys, level=m['lvl'])
            if new != sp:
                new = c.evolve(new, m['lvl'], where)
                party_keys.add(c.species_key(new))
                m['species'] = new
            if 'held_item' in m:
                m['held_item'] = c.cap_item(m['held_item'], where, held=True)
            if 'moves' in m:
                if new != sp or any(c.move_gen(x) > c.max_gen for x in m['moves']):
                    m['moves'] = c.cap_moves(m['moves'], m['species'], m['lvl'], where)
    return d


def cap_gifts(c, gifts):
    g = copy.deepcopy(gifts)
    before = len(g['starter_choices'])
    g['starter_choices'] = [s for s in g['starter_choices'] if c.sp_ok(s)]
    c.note('starter choices', 'species', '%d choices' % before, '%d choices' % len(g['starter_choices']),
           'only Gen %d and older' % c.max_gen if c.max_gen > 1 else 'Gen 1 only')
    g['starter_choices_hoenn'] = [s for s in g['starter_choices_hoenn'] if c.sp_ok(s)]
    kept = {}
    for s in g['static_encounters']:
        d = c.qsp.get(s['species'])
        if d and d['gen'] <= c.max_gen and not d['form']:
            kept.setdefault(s['region'], set()).add(d['base'])
    for s in g['static_encounters']:
        d = c.qsp.get(s['species'])
        where = 'static %s %d (%s)' % (s['region'], s['index'], s['map'])
        if not d or c.sp_ok(s['species']):
            continue
        if d['gen'] > c.max_gen:
            s['scrapped'] = 'Gen %d' % d['gen']
        elif key(d['name']) in GEN4_FORMS:
            s['species'] = c.qconst_by_nat[d['base']]
            s['form_of'] = d['const']
            c.note(where, 'species', d['const'], s['species'], 'a Gen 4 form, kept as its species')
            continue
        elif d['base'] in kept.get(s['region'], set()):
            s['scrapped'] = 'later form of %s, which has its own encounter' % c.by_id[d['base']]['name']
        else:
            s['species'] = c.qconst_by_nat[d['base']]
            c.note(where, 'species', d['const'], s['species'], 'form of %s' % c.by_id[d['base']]['name'])
            continue
        c.note(where, 'static encounter', d['const'], None, 'scrapped: ' + s['scrapped'])
    for t in g['in_game_trades']:
        where = 'trade %d (%s)' % (t['index'], ', '.join(u.split('/')[1] for u in t['used_by']) or 'unused')
        pools = c.trade_pool(t['used_by'])
        sp = c.cap_species(t['species'], where + ' offered', pools)
        rq = c.cap_species(t['requested_species'], where + ' wanted', pools, avoid={c.species_key(sp)})
        if sp != t['species']:
            if t['nickname'].upper() == c.qsp[t['species']]['name'].upper().replace('’', "'"):
                t['nickname'] = c.qsp[sp]['name'].upper()
        t['requested_species'], t['species'] = rq, sp
        t['held_item'] = c.cap_item(t['held_item'], where, held=True)
    return g


SCRIPT_SPECIES = re.compile(r'\bSPECIES_[A-Z0-9_]+')
SCRIPT_ITEM = re.compile(r'\bITEM_[A-Z0-9_]+')
SCRIPT_MOVE = re.compile(r'\bMOVE_[A-Z0-9_]+')


QUEST_ITEM_CMDS = {'checkitem', 'removeitem', 'checkitemtype', 'checkpcitem'}


def cap_files(c, statics_scrapped):
    """scripts and map.json: species, items, moves; hide objects of scrapped statics / later Pokémon."""
    import json as _j
    ow = load(os.path.join(HERE, 'pokemon_ow_gfx.json'))['gid_to_species']
    scrapped_species = {sp for sp, m in statics_scrapped}
    changed = {}
    for path in sorted(glob.glob(os.path.join(RIP, '**', '*.inc'), recursive=True)):
        rel = os.path.relpath(path, RIP)
        region = 'johto' if 'johto' in rel.lower() else 'kanto' if 'kanto' in rel.lower() else 'shared'
        lines = open(path, encoding='utf-8').read().split('\n')
        out, dirty = [], False
        for n, line in enumerate(lines):
            where = '%s:%d' % (rel, n + 1)
            new = line
            cmd = line.split()[0] if line.split() else ''
            for sp in SCRIPT_SPECIES.findall(line):
                if not c.sp_ok(sp):
                    d = c.qsp.get(sp)
                    if d and (d['gen'] > c.max_gen and (d['base'] or 0) in LEGENDARY or sp in scrapped_species):
                        c.events.add((rel, sp, 'species'))
                    pools = [('reference: same region', c.wild_pool(region if region != 'shared' else 'kanto', '', 'land')[-1][1])]
                    new = re.sub(r'\b%s\b' % sp, c.cap_species(sp, where, pools), new)
            for it in SCRIPT_ITEM.findall(line):
                n_ = key(c.qitem.get(it, (0, ''))[1])
                if c.item_ok(it) and n_ not in ITEM_ALIASES:
                    continue
                if line.strip().startswith('.2byte') and not c.item_ok(it):
                    c.note(where, 'item', it, None, 'later item, out of the mart')
                    new = None
                    break
                if not c.item_ok(it) and n_ not in ITEM_SUBST and n_ not in QUETZAL_ITEMS and not re.search(
                        r'(gem|memory|terashard|iumz)$', n_) and cmd in QUEST_ITEM_CMDS:
                    # a later quest item the event checks for: the event goes with it
                    c.events.add((rel, it, 'item'))
                    c.note(where, 'item', it, it, 'later quest item, left for the event to be scrapped')
                    continue
                new = re.sub(r'\b%s\b' % it, c.cap_item(it, where), new)
            if new is not None:
                for mv in SCRIPT_MOVE.findall(line):
                    if c.move_gen(mv) > c.max_gen:
                        c.note(where, 'move', mv, None, 'Gen %d move (text only: buffermovename)' % c.move_gen(mv))
            if new != line:
                dirty = True
            if new is not None:
                out.append(new)
        if dirty:
            changed[rel] = '\n'.join(out)
    for path in sorted(glob.glob(os.path.join(RIP, 'maps', '*', 'map.json'))):
        rel = os.path.relpath(path, RIP)
        m = _j.load(open(path, encoding='utf-8'))
        dirty = False
        mapname = os.path.basename(os.path.dirname(path))
        for i, o in enumerate(m.get('object_events', [])):
            g = re.match(r'OBJ_EVENT_GFX_QZ_(\d+)$', o.get('graphics_id', ''))
            if not g or g.group(1) not in ow:
                continue
            nat, name, shiny = ow[g.group(1)]
            d = c.by_id.get(nat)
            if not d or c.sp_ok(d['const']):
                continue
            where = '%s object %d' % (rel, i + 1)
            if d['gen'] > c.max_gen or (d['const'], mapname) in statics_scrapped:
                o['flag'] = FLAG_CAPPED
                c.note(where, 'object', d['const'], None, 'hidden: %s' % ('later Pokémon' if d['gen'] > c.max_gen else 'scrapped encounter'))
            else:
                base = c.qconst_by_nat[d['base']]
                gid = c.by_id[d['base']]['id'] + 256 + (5000 if shiny else 0)
                o['graphics_id'] = 'OBJ_EVENT_GFX_QZ_%d' % gid
                c.note(where, 'object', d['const'], base, 'form of %s (overworld sprite)' % c.by_id[d['base']]['name'])
            dirty = True
        for b in m.get('bg_events', []):
            if 'item' in b and not c.item_ok(b['item']):
                b['item'] = c.cap_item(b['item'], '%s hidden item' % rel)
                dirty = True
        if dirty:
            changed[rel] = _j.dumps(m, indent=2, ensure_ascii=False) + '\n'
    return changed


def report(c, out, trainers, wild, gifts):
    from collections import Counter
    ch = c.changes
    by = Counter((x['what'], x['why'].split(':')[0] if x['why'] else '') for x in ch)
    sp = [x for x in ch if x['what'] == 'species' and x['from'] and x['to'] and not x['why'].startswith('evolved')]
    lines = ['# Quetzal rip capped to Gen %d' % c.max_gen, '',
             'Made by `tools/quetzal_rip/gencap.py --max-gen %d`. The modern originals stay in `quetzal_rip/`; this '
             'folder holds the capped data and an overlay of the changed rip files.' % c.max_gen, '',
             '## Counts', '', '| change | why | count |', '|---|---|---|']
    for (w, y), n in sorted(by.items(), key=lambda x: (-x[1], x[0])):
        lines.append('| %s | %s | %d |' % (w, y, n))
    lines += ['', '## Static encounters scrapped', '']
    for s in gifts['static_encounters']:
        if s.get('scrapped'):
            lines.append('- %s %s (%s): %s' % (s['region'], s['species'], s['map'], s['scrapped']))
    lines += ['', '## Events built on later content', '',
              'These scripts check for a later quest item or show a later legendary (its encounter is scrapped and '
              'its object hidden). The capped overlay keeps them compiling; the port drops or rewrites the events.', '']
    for rel, sym, kind in sorted(c.events):
        lines.append('- `%s`: %s' % (rel, sym))
    lines += ['', '## Starter choices', '', ', '.join(x[8:] for x in gifts['starter_choices']), '',
              '## Trades', '', '| where | wanted | offered |', '|---|---|---|']
    for t in gifts['in_game_trades']:
        lines.append('| %s | %s | %s |' % (', '.join(u.split('/')[1] for u in t['used_by']) or 'unused',
                                          t['requested_species'][8:], t['species'][8:]))
    lines += ['', '## Species swaps (most common)', '', '| from | to | why | times |', '|---|---|---|---|']
    cnt = Counter((x['from'], x['to'], x['why']) for x in sp)
    for (a, b, y), n in cnt.most_common(150):
        lines.append('| %s | %s | %s | %d |' % (a[8:], b[8:], y, n))
    lines += ['', 'Every change, with where it happened: `changes.json`.', '']
    open(os.path.join(out, 'REPORT.md'), 'w', encoding='utf-8').write('\n'.join(lines))


def dump(obj, path):
    with open(path, 'w', encoding='utf-8') as fh:
        json.dump(obj, fh, indent=1, ensure_ascii=False)
        fh.write('\n')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--max-gen', type=int, default=4)
    ap.add_argument('--out')
    a = ap.parse_args()
    out = a.out or os.path.join(RIP, 'gen%d' % a.max_gen)
    c = Capper(a.max_gen)
    c._tm = {k.replace('TM', 'TM').upper(): v for k, v in
             load(os.path.join(REPO, 'tools', 'moemon_gen4', 'gen4_data.json'))['tm_moves'].items()}
    c._tm = {k: c.pmove[key(v[5:])]['name'] if key(v[5:]) in c.pmove else v for k, v in c._tm.items()}
    trainers = cap_trainers(c, load(os.path.join(RIP, 'data', 'trainers.json')))
    wild = cap_wild(c, load(os.path.join(RIP, 'data', 'wild_encounters.json')))
    gifts = cap_gifts(c, load(os.path.join(RIP, 'data', 'gifts.json')))
    scrapped = {(s['species'], (s['map'] or '').replace('MAP_', '')) for s in gifts['static_encounters'] if s.get('scrapped')}
    files = cap_files(c, scrapped)
    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(os.path.join(out, 'data'))
    dump(trainers, os.path.join(out, 'data', 'trainers.json'))
    dump(wild, os.path.join(out, 'data', 'wild_encounters.json'))
    dump(gifts, os.path.join(out, 'data', 'gifts.json'))
    for rel, text in files.items():
        p = os.path.join(out, 'overlay', rel)
        os.makedirs(os.path.dirname(p), exist_ok=True)
        open(p, 'w', encoding='utf-8').write(text)
    with open(os.path.join(out, 'changes.json'), 'w', encoding='utf-8') as fh:
        fh.write('[\n' + ',\n'.join(json.dumps(x, ensure_ascii=False) for x in c.changes) + '\n]\n')
    report(c, out, trainers, wild, gifts)
    left = []
    for t in trainers['trainers']:
        if t['region'] in ('johto', 'kanto'):
            left += [m['species'] for m in t['party'] if not c.sp_ok(m['species'])]
    for e in wild['wild_encounter_groups'][0]['encounters']:
        for f in WILD_KINDS:
            left += [m['species'] for m in e.get(f, {}).get('mons', []) if not c.sp_ok(m['species'])]
    print('gen %d: %d changes, %d files in the overlay, %d later species left' % (
        a.max_gen, len(c.changes), len(files), len(left)))


if __name__ == '__main__':
    main()
