#!/usr/bin/env python3
"""The card guide: every card of the three card games and where it comes from, as one HTML page for players.

usage: tools/card_guide/gen_guide.py DATA_DIR OUT.html   (tools/card_guide/build.sh makes DATA_DIR's JSON from the
       game's own tables: dump_tcg.c, dump_ygo.c, dump_tt.c)

What isn't in a table the dumps can read is written here, with where in the game it comes from:
- the regular card players' decks and which people play them (src/tcg/tcg_field.c sPeople / sPools),
- the special trainers: who, where, what unlocks them (tcg_field.c / ygo_field.c / tt_field.c sSpecials),
- the set-completion and GYM LEADER prizes (src/tcg/tcg_ui.c sSetPromos, ClaimPrize),
- the Yu-Gi-Oh! special duelists' signature cards (ygo_field.c Signature(), printed by `mp ygo deck THEME 0`),
- the Triple Triad regions and sellers (src/tt/tt_field.c RegionOfMapsec, the maps' EventScript_TtSeller).
"""
import html
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DATA, OUT = Path(sys.argv[1]), Path(sys.argv[2])
tcg = json.load(open(DATA / 'tcg.json'))
ygo = json.load(open(DATA / 'ygo.json'))
tt = json.load(open(DATA / 'tt.json'))
VERSION = re.search(r'#define MOEMON_VERSION "([^"]+)"', (ROOT / 'include/mp_version.h').read_text()).group(1)


def e(s):
    return html.escape(str(s), quote=True)


# ---------------------------------------------------------------------------------------------------------------
# POKeMON cards
# ---------------------------------------------------------------------------------------------------------------
PACKS = ['COLOSSEUM', 'EVOLUTION', 'MYSTERY', 'LABORATORY', 'BEGINNING POKéMON', 'LEGENDARY POWER',
         'ISLAND OF FOSSIL', 'PSYCHIC BATTLE', 'SKY-FLYING POKéMON', 'WE ARE TEAM ROCKET', "TEAM ROCKET'S AMBITION"]
TYPES = ['Fire', 'Grass', 'Lightning', 'Water', 'Fighting', 'Psychic', 'Colorless']
# (tcg_field.c sPools: the Game Boy decks each kind of player picks from, by badges 0-1, 2-3, 4-6, 7-8)
POOLS = {
    0: [['CharmanderAndFriends', 'LightningAndFire'], ['Flamethrower', 'Anger'], ['FireCharge', 'HeatedBattle'],
        ['LegendaryMoltres', 'LegendaryRonald']],
    1: [['BulbasaurAndFriends', 'Etcetera'], ['FlowerGarden', 'LovelyNidoran', 'Poison'],
        ['FlowerPower', 'WondersofScience', 'NapTime'], ['InvincibleRonald', 'Kaleidoscope']],
    2: [['LightningAndFire', 'BoomBoomSelfdestruct'], ['Pikachu', 'PowerGenerator'],
        ['ZappingSelfdestruct', 'PowerGenerator'], ['LegendaryZapdos', 'ZappingSelfdestruct']],
    3: [['SquirtleAndFriends', 'WaterAndFighting'], ['WaterfrontPokemon', 'SoundoftheWaves', 'LonelyFriends'],
        ['GoGoRainDance', 'Imakuni'], ['LegendaryArticuno', 'LegendaryDragonite']],
    4: [['WaterAndFighting', 'SamsPractice', 'BlisteringPokemon'], ['HardPokemon', 'LovetoBattle', 'Excavation'],
        ['MusclesforBrains', 'RockCrusher'], ['FirstStrike', 'HeatedBattle']],
    5: [['NapTime', 'SquirtleAndFriends'], ['Ghost', 'Imakuni'], ['StrangePsyshock', 'Ghost'],
        ['StrangePower', 'PowerfulRonald']],
    6: [['LonelyFriends', 'Anger'], ['FlyinPokemon', 'Kaleidoscope'], ['ImRonald', 'PowerfulRonald'],
        ['LegendaryDragonite', 'LegendaryRonald']],
}
# (tcg_field.c sPeople: who plays which kind; the many "mixed" people play any of them)
PEOPLE = {
    0: 'COOK, MAN, TEAM MAGMA GRUNT',
    1: 'TWIN, LITTLE GIRL, BUG CATCHER, PICNICKER, LASS, CAMPER',
    2: 'GAME KID, MANIAC, SCIENTIST, DEVON WORKER, cycling TRIATHLETE',
    3: 'TUBER, SWIMMER, SAILOR, FISHERMAN, TEAM AQUA GRUNT',
    4: 'HIKER, BLACK BELT, running TRIATHLETE, EXPERT (man), MAN',
    5: 'NINJA BOY, BEAUTY, PSYCHIC, HEX MANIAC, OLD WOMAN, EXPERT (woman)',
    6: 'BIG KID, YOUNGSTER, RICH BOY, MOéMON FAN, GENTLEMAN',
}
STARTERS = {'CharmanderAndFriends': 'Ember Friends', 'SquirtleAndFriends': 'Tidal Friends',
            'BulbasaurAndFriends': 'Garden Friends'}
# (tcg_field.c sSpecials, in gTcgSpecialDecks' order: what unlocks them and where they are)
TCG_SPECIALS = [
    ('ROXANNE', 'Gym Leader', 'Rustboro City Gym', 'after you win the STONE BADGE'),
    ('BRAWLY', 'Gym Leader', 'Dewford Town Gym', 'after you win the KNUCKLE BADGE'),
    ('WATTSON', 'Gym Leader', 'Mauville City Gym', 'after you win the DYNAMO BADGE'),
    ('FLANNERY', 'Gym Leader', 'Lavaridge Town Gym', 'after you win the HEAT BADGE'),
    ('NORMAN', 'Gym Leader', 'Petalburg City Gym', 'after you win the BALANCE BADGE'),
    ('WINONA', 'Gym Leader', 'Fortree City Gym', 'after you win the FEATHER BADGE'),
    ('TATE & LIZA', 'Gym Leaders', 'Mossdeep City Gym', 'after you win the MIND BADGE'),
    ('JUAN', 'Gym Leader', 'Sootopolis City Gym', 'after you win the RAIN BADGE'),
    ('SIDNEY', 'Elite Four', 'his room at the POKéMON LEAGUE', 'after you beat him in a Pokémon battle'),
    ('PHOEBE', 'Elite Four', 'her room at the POKéMON LEAGUE', 'after you beat her in a Pokémon battle'),
    ('GLACIA', 'Elite Four', 'her room at the POKéMON LEAGUE', 'after you beat her in a Pokémon battle'),
    ('DRAKE', 'Elite Four', 'his room at the POKéMON LEAGUE', 'after you beat him in a Pokémon battle'),
    ('WALLACE', 'Champion', 'wherever you meet him', 'after you become Champion'),
    ('STEVEN', '', 'wherever you meet him (Granite Cave, his house in Mossdeep, Meteor Falls)', 'any time'),
    ('Your rival (MAY / BRENDAN)', '', 'their house in Littleroot Town, or wherever you meet them', 'any time'),
    ('WALLY', '', 'wherever you meet him (Mauville City, Victory Road)', 'any time'),
    ('MAXIE', 'Magma Leader', 'wherever you meet him', 'any time'),
    ('ARCHIE', 'Aqua Leader', 'wherever you meet him', 'any time'),
    ('ANABEL', 'Salon Maiden', 'outside the Battle Tower, Battle Frontier', 'after winning the Tower silver symbol; about half the Brains are out each day'),
    ('TUCKER', 'Dome Ace', 'outside the Battle Dome, Battle Frontier', 'after winning the Dome silver symbol; about half the Brains are out each day'),
    ('SPENSER', 'Palace Maven', 'outside the Battle Palace, Battle Frontier', 'after winning the Palace silver symbol; about half the Brains are out each day'),
    ('GRETA', 'Arena Tycoon', 'outside the Battle Arena, Battle Frontier', 'after winning the Arena silver symbol; about half the Brains are out each day'),
    ('NOLAND', 'Factory Head', 'outside the Battle Factory, Battle Frontier', 'after winning the Factory silver symbol; about half the Brains are out each day'),
    ('LUCY', 'Pike Queen', 'outside the Battle Pike, Battle Frontier', 'after winning the Pike silver symbol; about half the Brains are out each day'),
]
# JOHTO's and KANTO's special trainers, each in one of HOENN's slots (tcg_field.c sQzSpecials, read from the source):
# slot -> [(region, name, title, when)]
SPECIAL_SLOTS = ['ROXANNE', 'BRAWLY', 'WATTSON', 'FLANNERY', 'NORMAN', 'WINONA', 'TATE_LIZA', 'JUAN', 'SIDNEY', 'PHOEBE',
                 'GLACIA', 'DRAKE', 'WALLACE', 'STEVEN', 'RIVAL', 'WALLY']
QZ_WHEN = {'FLAG': 'after you win their BADGE', 'TRAINER': 'after you beat them in a Pokémon battle',
           'CLEARED': 'after you become Champion', 'NONE': 'any time'}
QZ_SPECIALS = {}
_src = open(Path(__file__).resolve().parent.parent.parent / 'src/tcg/tcg_field.c', encoding='utf-8').read()
_src = _src[_src.index('sQzSpecials[] = {'):]
_src = _src[:_src.index('};')]
_region = 'JOHTO'
for _line in _src.split('\n'):
    if '// KANTO' in _line:
        _region = 'KANTO'
    _m = re.search(r'TCG_SPECIAL_(\w+), REQ_(\w+), \w+, "([^"]*)", "([^"]*)"', _line)
    if _m:
        QZ_SPECIALS.setdefault(SPECIAL_SLOTS.index(_m.group(1)), []).append(
            (_region, _m.group(3), _m.group(4).title(), QZ_WHEN[_m.group(2)]))
# (tcg_ui.c sSetPromos: 75% of a booster's cards; GYM_SWEEP_BIT: every Gym Leader's deck beaten)
SET_PROMOS = [64, 118, 145, 200, 319, 421, 315, 384, 418, 401, 441]
GYM_PROMO = 102
# (tcg_ui.c sMilestonePromos: card milestones claimed under PRIZES, counted from what the save already holds)
MILESTONE_PROMOS = [
    (309, 'Win a POKéMON card duel'), (317, 'Win 10 POKéMON card duels'), (336, 'Win 25 POKéMON card duels'),
    (337, 'Win 50 POKéMON card duels'), (346, 'Win 100 POKéMON card duels'), (436, 'Open 10 booster packs'),
    (430, 'Open 50 booster packs'), (422, 'Open 100 booster packs'), (351, "Beat 5 special trainers' decks"),
    (373, "Beat 10 special trainers' decks"), (364, "Beat 15 special trainers' decks"),
    (286, 'Beat all four ELITE FOUR decks'), (407, 'Win a duel without giving up a Prize (CLEAN SWEEP)'),
    (415, 'Win a duel with a deck of one type (PURE ENERGY)'), (413, 'Own every card of a booster set'),
    (420, 'Own every card of 4 booster sets'), (381, 'Own every card of all 11 booster sets'),
]
# (ygo_field.c: a Yu-Gi-Oh! win over an Elite Four member hands over their POKeMON promo too)
YGO_E4 = {'SIDNEY', 'PHOEBE', 'GLACIA', 'DRAKE'}

cards = {c['id']: c for c in tcg['cards']}
decks = {d['name']: d for d in tcg['decks']}
RARITY = ['Common', 'Uncommon', 'Rare', 'Promo']


def tname(c):
    n = c['name']
    if c['kind'] == 0 and c['level']:   # (TCG_POKEMON: the level tells same-named cards apart)
        n += ' Lv%d' % c['level']
    return n


def gb2_kind(d):
    count = [0] * 7
    for k in d['cards']:
        c = cards.get(k)
        if not c:
            continue
        if c['kind'] == 2 and k <= 6:   # (basic Energy)
            count[c['type']] += 2
        elif c['kind'] == 0 and c['type'] <= 6:
            count[c['type']] += 1
    best = 0
    for i in range(1, 7):
        if count[i] > count[best]:
            best = i
    return best


kind_cards = {k: set() for k in range(7)}
for kind, tiers in POOLS.items():
    for tier in tiers:
        for name in tier:
            kind_cards[kind].update(decks[name]['cards'])
for d in tcg['decks']:
    if d['name'].startswith('GB2 '):
        kind_cards[gb2_kind(d)].update(d['cards'])
special_of = {}
for i, sp in enumerate(tcg['special']):
    for k in set(sp['cards']):
        special_of.setdefault(k, []).append(TCG_SPECIALS[i][0])
struct_of, starter_of = {}, {}
for s in tcg['structure']:
    for k in set(s['cards']):
        struct_of.setdefault(k, []).append(s['name'])
for name, label in STARTERS.items():
    for k in set(decks[name]['cards']):
        starter_of.setdefault(k, []).append(label)

promo_src = {}
for i, sp in enumerate(tcg['special']):
    if sp['promo']:
        who, title, where, when = TCG_SPECIALS[i]
        promo_src.setdefault(sp['promo'], []).append(
            'Beat %s%s in a POKéMON card duel for the first time (%s; they duel %s).' %
            (who, ' (%s)' % title if title else '', where, when))
        if who in YGO_E4:
            promo_src[sp['promo']].append(
                'Or beat %s in a YU-GI-OH! duel for the first time: they hand over this card too.' % who)
        for region, qwho, qtitle, qwhen in QZ_SPECIALS.get(i, []):
            promo_src[sp['promo']].append(
                'In %s: beat %s%s in a POKéMON card duel for the first time (they duel %s; same deck).' %
                (region, qwho, ' (%s)' % qtitle if qtitle else '', qwhen))
for i, p in enumerate(SET_PROMOS):
    promo_src.setdefault(p, []).append(
        'Own 75%% of the %s booster set\'s cards, then claim it under PRIZES at a CARD CLUB counter or the '
        'GAME CENTER 2F CARD SHOP.' % PACKS[i])
for p, what in MILESTONE_PROMOS:
    promo_src.setdefault(p, []).append(
        '%s, then claim it under PRIZES at a CARD CLUB counter or the GAME CENTER 2F CARD SHOP.' % what)
promo_src.setdefault(GYM_PROMO, []).append(
    'Beat all eight GYM LEADERS in POKéMON card duels, then claim it under PRIZES at a CARD CLUB counter or the '
    'GAME CENTER 2F CARD SHOP.')


def tcg_row(c):
    k = c['id']
    rarity = 'Basic Energy' if k <= 6 else 'Promo' if c['set'] == 4 else ('Holo ' if c['holo'] else '') + RARITY[min(c['rarity'], 3)]
    packs = ', '.join(PACKS[p] for p in c['packs'])
    shops = []
    if k <= 6:
        shops.append('free, as many as you like')
    if c['cp']:
        shops.append('CARD CLUB: %d CP' % c['cp'])
    if c['money']:
        shops.append('GAME CENTER 2F: ¥%d' % c['money'])
    ok_daily = c['set'] != 4 and not c['holo'] and k > 6
    people = []
    if ok_daily:
        for kind in range(7):
            if k in kind_cards[kind]:
                people.append(TYPES[kind])
    duel = ('%s-deck duelists' % ' / '.join(people)) if people else ''
    sp = ', '.join(special_of.get(k, [])) if ok_daily else ''
    decks_ = []
    if k in starter_of:
        decks_.append('Starter: ' + ', '.join(starter_of[k]))
    if k in struct_of:
        decks_.append('GAME CENTER deck: ' + ', '.join(struct_of[k]))
    special = ' '.join(promo_src.get(k, []))
    if c['set'] == 4 and not special:
        special = 'No way to get this card in this version of the game.'
    return [tname(c), TYPES[c['type']] if c['kind'] == 0 else ('Trainer' if c['kind'] == 1 else 'Energy'), rarity,
            packs, '; '.join(shops), duel, sp, '; '.join(decks_), special]


tcg_rows = [tcg_row(c) for c in tcg['cards']]

# ---------------------------------------------------------------------------------------------------------------
# YU-GI-OH! cards
# ---------------------------------------------------------------------------------------------------------------
FRAMES = ['Normal Monster', 'Effect Monster', 'Fusion Monster', 'Ritual Monster', 'Spell', 'Trap', 'Token']
YRARITY = ['Common', 'Short Print', 'Super Short Print', 'Rare', 'Super Rare', 'Ultra Rare', 'Secret Rare']
# (ygo_field.c sDuelists: their theme's Signature(), as `mp ygo deck THEME 0` prints it)
YGO_SPECIALS = [
    ('SIDNEY', 'Elite Four', 'his room at the POKéMON LEAGUE', 'after you beat him in a Pokémon battle', 'Raviel, Lord of Phantasms'),
    ('PHOEBE', 'Elite Four', 'her room at the POKéMON LEAGUE', 'after you beat her in a Pokémon battle', 'Berserk Dragon'),
    ('GLACIA', 'Elite Four', 'her room at the POKéMON LEAGUE', 'after you beat her in a Pokémon battle', 'Suijin'),
    ('DRAKE', 'Elite Four', 'his room at the POKéMON LEAGUE', 'after you beat him in a Pokémon battle', 'Horus the Black Flame Dragon LV8'),
    ('MAXIE', 'Magma Leader', 'wherever you meet him', 'any time', 'Hino-Kagu-Tsuchi'),
    ('ARCHIE', 'Aqua Leader', 'wherever you meet him', 'any time', 'Water Dragon'),
    ('ANABEL', 'Salon Maiden', 'outside the Battle Tower, Battle Frontier', 'after winning the Tower silver symbol; about half the Brains are out each day', 'Silent Magician LV8'),
    ('TUCKER', 'Dome Ace', 'outside the Battle Dome, Battle Frontier', 'after winning the Dome silver symbol; about half the Brains are out each day', 'Gate Guardian'),
    ('SPENSER', 'Palace Maven', 'outside the Battle Palace, Battle Frontier', 'after winning the Palace silver symbol; about half the Brains are out each day', 'Fairy King Truesdale'),
    ('GRETA', 'Arena Tycoon', 'outside the Battle Arena, Battle Frontier', 'after winning the Arena silver symbol; about half the Brains are out each day', 'Mystical Knight of Jackal'),
    ('NOLAND', 'Factory Head', 'outside the Battle Factory, Battle Frontier', 'after winning the Factory silver symbol; about half the Brains are out each day', 'Perfect Machine King'),
    ('LUCY', 'Pike Queen', 'outside the Battle Pike, Battle Frontier', 'after winning the Pike silver symbol; about half the Brains are out each day', 'Gogiga Gagagigo'),
    ('STEVEN', '', 'wherever you meet him (Granite Cave, his house in Mossdeep, Meteor Falls)', 'any time', 'Valkyrion the Magna Warrior'),
    ('Your rival (MAY / BRENDAN)', '', 'their house in Littleroot Town, or wherever you meet them', 'any time', 'Theinen the Great Sphinx'),
    ('WALLY', '', 'wherever you meet him (Mauville City, Victory Road)', 'any time', 'Guardian Angel Joan'),
]
ycards = {c['id']: c for c in ygo['cards']}
yname = {c['name']: c['id'] for c in ygo['cards']}
signature_of = {}
for who, title, where, when, card in YGO_SPECIALS:
    signature_of.setdefault(yname[card], []).append((who, title, where, when))
boosters_of, ydecks_of = {}, {}
for s in ygo['sets']:
    for card, rarity, copies in s['cards']:
        if s['starter']:
            ydecks_of.setdefault(card, []).append(('Starter Deck: ' if s['starter'] == 1 else 'Structure Deck: ') + s['name'].split(': ', 1)[-1])
        else:
            boosters_of.setdefault(card, []).append(s['code'])
# (ygo_npc_decks.c gYgoNpcDecks: the Game Boy Advance games' duelists' decks that card players carry)
npc_src = (ROOT / 'src/ygo/ygo_npc_decks.c').read_text()
arrays = {m.group(1): [int(x) for x in m.group(2).split(',') if x.strip()]
          for m in re.finditer(r'static const uint16_t (sDeck\d+)\[\] = \{([^}]*)\}', npc_src)}
played_by = {}
for m in re.finditer(r'\{"((?:[^"\\]|\\.)*)", "(\w+)", "[^"]*", \d+, \d+, (sDeck\d+)\}', npc_src):
    who = m.group(1).replace('\\"', '"')
    for k in set(arrays[m.group(3)]):
        lst = played_by.setdefault(k, [])
        if who not in lst:
            lst.append(who)
set_names = {s['code']: s['name'] for s in ygo['sets']}


def ygo_row(c):
    k = c['id']
    shops = []
    if c['cp']:
        shops.append('CARD CLUB: %d CP' % c['cp'])
    if c['money']:
        shops.append('GAME CENTER 2F: ¥%d' % c['money'])
    pb = played_by.get(k, [])
    duel = ', '.join(pb[:4]) + (' and %d more' % (len(pb) - 4) if len(pb) > 4 else '')
    sig = ' '.join('First win over %s%s (%s; they duel %s).' % (w, ' (%s)' % t if t else '', wh, wn)
                   for w, t, wh, wn in signature_of.get(k, []))
    return [c['name'], FRAMES[c['frame']], YRARITY[c['rarity']], ', '.join(boosters_of.get(k, [])),
            '; '.join(shops), duel, '; '.join(ydecks_of.get(k, [])), sig]


ygo_rows = [ygo_row(c) for c in ygo['cards'] if c['frame'] != 6]

# ---------------------------------------------------------------------------------------------------------------
# TRIPLE TRIAD cards
# ---------------------------------------------------------------------------------------------------------------
REGIONS = [('PETALBURG', 'Littleroot, Oldale, Petalburg, Routes 101-104', 'Petalburg City POKéMON CENTER'),
           ('RUSTBORO', 'Rustboro, Petalburg Woods, Routes 115-116, Rusturf Tunnel', 'GAME CENTER 2F (Rustboro City)'),
           ('SLATEPORT', 'Dewford, Slateport, Routes 105-110, Granite Cave, Abandoned Ship', 'Slateport City POKéMON CENTER'),
           ('MAUVILLE', 'Mauville, Verdanturf, Routes 111, 117, 118, New Mauville', 'Mauville City POKéMON CENTER'),
           ('LAVARIDGE', 'Lavaridge, Fallarbor, Routes 112-114, Mt. Chimney, Jagged Pass, Meteor Falls, Desert Ruins', 'Fallarbor Town POKéMON CENTER'),
           ('LILYCOVE', 'Fortree, Lilycove, Routes 119-123, Mt. Pyre, Safari Zone', 'Lilycove City POKéMON CENTER'),
           ('SOOTOPOLIS', 'Mossdeep, Sootopolis, Pacifidlog, Routes 124-134 and the sea', 'Mossdeep City POKéMON CENTER'),
           ('EVER GRANDE', 'Ever Grande City, Victory Road, the POKéMON LEAGUE', 'Ever Grande City POKéMON CENTER')]
# (tt_field.c sSpecials, in gTtSpecialCards' order)
TT_SPECIALS = [
    ('STEVEN', 'wherever you meet him (Granite Cave, his house in Mossdeep, Meteor Falls)', 'any time'),
    ('Your rival (MAY / BRENDAN)', 'their house in Littleroot Town, or wherever you meet them', 'any time'),
    ('WALLY', 'wherever you meet him (Mauville City, Victory Road)', 'any time'),
    ('WALLACE', 'wherever you meet him', 'after you become Champion'),
    ('JUAN', 'Sootopolis City Gym', 'after you win the RAIN BADGE'),
    ('WINONA', 'Fortree City Gym', 'after you win the FEATHER BADGE'),
    ('NORMAN', 'Petalburg City Gym', 'after you win the BALANCE BADGE'),
    ('ROXANNE', 'Rustboro City Gym', 'after you win the STONE BADGE'),
    ('TATE & LIZA', 'Mossdeep City Gym', 'after you win the MIND BADGE'),
    ('MR. STONE', 'DEVON CORPORATION 3F, Rustboro City', 'any time'),
    ('MR. BRINEY', 'his cottage on Route 104', 'any time'),
    ('SCOTT', 'wherever you meet him', 'any time'),
    ('PROF. BIRCH', 'his lab in Littleroot Town, or wherever you meet him', 'any time'),
    ('MOM', 'your house in Littleroot Town', 'any time'),
    ('CARD QUEEN', 'a POKéMON CENTER: she moves to another town every few days', 'any time'),
]
tcards = {c['id']: c for c in tt['cards']}
pool_regions, shelf_regions, tt_special_of = {}, {}, {}
for r, pool in enumerate(tt['pools']):
    for k in pool:
        pool_regions.setdefault(k, []).append(r)
for r, shelf in enumerate(tt['shelves']):
    for k in shelf:
        shelf_regions.setdefault(k, []).append(r)
for i, own in enumerate(tt['specials']):
    for k in own:
        tt_special_of.setdefault(k, []).append(i)
TT_PACK = {1: 'BRONZE (¥600)', 2: 'SILVER (¥1600)', 3: 'GOLD (¥4000)'}
TT_PRICE = {1: 400, 2: 1000, 3: 2600}


def tt_row(c):
    k, stars = c['id'], c['stars']
    sold = '; '.join('%s: ¥%d' % (REGIONS[r][2], TT_PRICE[stars]) for r in shelf_regions.get(k, []))
    regions = pool_regions.get(k, [])
    if stars <= 3:
        players = ', '.join(REGIONS[r][0].title() for r in regions) or ''
    else:
        players = ', '.join(REGIONS[r][0].title() for r in regions) + ' (with 4 or more badges)' if regions else ''
    sp = []
    for i in tt_special_of.get(k, []):
        who, where, when = TT_SPECIALS[i]
        sp.append('%s (%s; plays %s)%s' % (who, where, when, ': one of their 5★ cards each day' if stars == 5 else ''))
    return [c['name'], '★' * stars, TT_PACK.get(stars, ''), sold, players, '; '.join(sp)]


tt_rows = [tt_row(c) for c in sorted(tt['cards'], key=lambda c: (c['stars'], c['order']))]

# ---------------------------------------------------------------------------------------------------------------
# The page
# ---------------------------------------------------------------------------------------------------------------
def table(tid, head, rows, highlight=None):
    out = ['<div class="scroll"><table id="%s"><thead><tr>' % tid]
    out += ['<th>%s</th>' % e(h) for h in head]
    out.append('</tr></thead><tbody>')
    for row in rows:
        cls = ' class="hl"' if highlight and highlight(row) else ''
        out.append('<tr%s>' % cls + ''.join('<td>%s</td>' % e(v) for v in row) + '</tr>')
    out.append('</tbody></table></div>')
    return ''.join(out)


def search(tid, what):
    return ('<label class="search">Search %s <input type="search" data-table="%s" placeholder="Card name, pack, '
            'person..." autocomplete="off"></label><p class="count" id="%s-count"></p>' % (e(what), tid, tid))


# The special ways, game by game
tcg_promos = []
for c in tcg['cards']:
    if c['set'] == 4:
        how = promo_src.get(c['id'])
        tcg_promos.append([tname(c), TYPES[c['type']] if c['kind'] == 0 else 'Trainer',
                           ' '.join(how) if how else 'No way to get this card in this version of the game.'])
tcg_promos.sort(key=lambda r: (r[2].startswith('No way'), r[0]))
missing = [r for r in tcg_promos if r[2].startswith('No way')]
non_promo_special = [r for r, c in zip(tcg_rows, tcg['cards'])
                     if c['set'] != 4 and c['id'] > 6 and not (r[3] or r[4] or r[5])]
holos = [tname(c) for c in tcg['cards'] if c['holo']]

sig_rows = [[w, t or '-', wh, wn, card] for w, t, wh, wn, card in YGO_SPECIALS]
tt5 = []
for i, (who, where, when) in enumerate(TT_SPECIALS):
    own = [tcards[k] for k in tt['specials'][i]]
    tt5.append([who, where, when, ', '.join(c['name'] for c in own if c['stars'] == 5),
                ', '.join(c['name'] for c in own if c['stars'] == 4) or '-'])
fours = sum(1 for c in tt['cards'] if c['stars'] == 4)
fives = sum(1 for c in tt['cards'] if c['stars'] == 5)

css = """
:root{--bg:#f6f4ef;--panel:#fffdf8;--ink:#1f2328;--soft:#5b6270;--line:#ddd6c8;--accent:#b0302a;--accent2:#2a5db0;
--hl:#fff1d6;--warn:#fbe3e1;--chip:#efe9dc;color-scheme:light}
@media (prefers-color-scheme:dark){:root{--bg:#16181d;--panel:#1d2027;--ink:#e6e3dc;--soft:#9aa1ad;--line:#343844;
--accent:#ef6a5f;--accent2:#7aa7f2;--hl:#3a3220;--warn:#41231f;--chip:#2a2e37;color-scheme:dark}}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font:15px/1.55 system-ui,-apple-system,
"Segoe UI",Roboto,sans-serif}
header{padding:28px max(16px,4vw) 18px;border-bottom:3px solid var(--accent);background:var(--panel)}
header h1{margin:0 0 4px;font-size:1.9rem;letter-spacing:.01em;text-wrap:balance}
header p{margin:0;color:var(--soft);max-width:70ch}
nav{position:sticky;top:0;z-index:5;background:var(--panel);border-bottom:1px solid var(--line);
padding:8px max(16px,4vw);display:flex;flex-wrap:wrap;gap:6px 14px;font-size:.9rem}
nav a{color:var(--accent2);text-decoration:none;font-weight:600}nav a:hover{text-decoration:underline}
main{padding:8px max(16px,4vw) 60px;max-width:1500px}
h2{font-size:1.45rem;margin:38px 0 6px;padding-bottom:4px;border-bottom:1px solid var(--line);text-wrap:balance}
h3{font-size:1.1rem;margin:26px 0 6px}
p,li{max-width:80ch}.soft{color:var(--soft)}
.callout{background:var(--panel);border:1px solid var(--line);border-left:4px solid var(--accent2);padding:10px 14px;
border-radius:6px;margin:12px 0;max-width:90ch}
.callout.warn{border-left-color:var(--accent);background:var(--warn)}
.scroll{overflow-x:auto;border:1px solid var(--line);border-radius:6px;background:var(--panel);margin:8px 0 4px}
table{border-collapse:collapse;width:100%;font-size:.86rem}
th{position:sticky;top:0;background:var(--chip);text-align:left;padding:7px 9px;white-space:nowrap;font-size:.75rem;
letter-spacing:.05em;text-transform:uppercase;color:var(--soft)}
td{padding:6px 9px;border-top:1px solid var(--line);vertical-align:top}
td:first-child{font-weight:600;white-space:nowrap}tr.hl td{background:var(--hl)}
.search{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin:14px 0 2px;font-weight:600;font-size:.9rem}
.search input{flex:1 1 260px;max-width:440px;padding:7px 10px;border:1px solid var(--line);border-radius:6px;
background:var(--panel);color:var(--ink);font:inherit}
.count{margin:2px 0 0;color:var(--soft);font-size:.82rem}
.chips{display:flex;flex-wrap:wrap;gap:6px;margin:8px 0}.chips span{background:var(--chip);border-radius:99px;
padding:2px 10px;font-size:.82rem}
details{margin:10px 0}summary{cursor:pointer;font-weight:600;color:var(--accent2)}
dl.legend{display:grid;grid-template-columns:max-content 1fr;gap:2px 14px;font-size:.88rem;max-width:90ch}
dl.legend dt{font-weight:700}dl.legend dd{margin:0}
footer{padding:20px max(16px,4vw);color:var(--soft);font-size:.82rem;border-top:1px solid var(--line)}
@media print{nav,.search,.count{display:none}th{position:static}}
"""
js = """
document.querySelectorAll('input[data-table]').forEach(function(inp){
  var t=document.getElementById(inp.dataset.table),rows=t.tBodies[0].rows,out=document.getElementById(inp.dataset.table+'-count');
  function run(){var q=inp.value.trim().toLowerCase(),n=0;for(var i=0;i<rows.length;i++){var on=!q||rows[i].textContent.toLowerCase().indexOf(q)>=0;
    rows[i].style.display=on?'':'none';if(on)n++;}out.textContent=n+' of '+rows.length+' cards';}
  inp.addEventListener('input',run);run();});
"""

page = []
page.append('<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" '
            'content="width=device-width,initial-scale=1"><title>Moemon Card Guide</title><style>%s</style></head><body>' % css)
page.append('<header><h1>Moemon Emerald Kai Card Guide</h1><p>Where to find every card in the three card games: '
            'POKéMON cards, YU-GI-OH! and TRIPLE TRIAD. The special cards come first, the ones you won\'t get from '
            'ordinary card players, booster packs or the card shops. After them, every card of each game and '
            'every place it turns up. Version %s.</p></header>' % e(VERSION))
page.append('<nav><a href="#basics">How cards come to you</a><a href="#pkmn-special">POKéMON specials</a>'
            '<a href="#ygo-special">YU-GI-OH! specials</a><a href="#tt-special">TRIPLE TRIAD specials</a>'
            '<a href="#pkmn-all">All POKéMON cards</a><a href="#ygo-all">All YU-GI-OH! cards</a>'
            '<a href="#tt-all">All TRIPLE TRIAD cards</a></nav><main>')

# Basics
page.append('''<h2 id="basics">How cards come to you</h2>
<p>Register the CARD CASE and press its button in front of someone to ask for a game. These are the ordinary ways to
collect cards. The special cards below are the ones they never give.</p>
<h3>POKéMON cards</h3><ul>
<li><b>Booster packs.</b> Every duel you win gives you a pack: ordinary duelists give one of their deck's kind, special
trainers one of their own. Each pack has 10 cards, with a rare in every pack and a holo in about one pack in three.
Packs are the only way to get <b>holo</b> cards (apart from trading with a friend).</li>
<li><b>A card from their deck.</b> The first time each day you beat someone, you pick one card from their deck to keep
(never a promo or a holo).</li>
<li><b>CARD CLUB counters</b> (Oldale and Mauville POKéMON MARTS, the Lilycove DEPARTMENT STORE): trade in spare
cards for CARD POINTS (CP) and buy any common or uncommon card with them.</li>
<li><b>GAME CENTER 2F CARD SHOP</b> (Rustboro City; in JOHTO and KANTO, by the attendant in GOLDENROD's and
CELADON's GAME CORNERS): one copy of every non-holo card of the booster sets for money, the structure and theme decks, and the CARD CLUB counter's services.</li>
<li><b>Starter deck.</b> Your first deck (Ember, Tidal or Garden Friends). Basic Energy is always free.</li></ul>
<h3>YU-GI-OH!</h3><ul>
<li><b>Booster packs</b> from every win, and from the CARD CLUB for CP.</li>
<li><b>A card from their deck</b>, the first time each day you beat someone.</li>
<li><b>The CARD CLUB</b> sells every collectible card for CP, and the GAME CENTER 2F clerk sells one copy of each for
money, plus the Starter and Structure Decks. So no YU-GI-OH! card is out of reach. The special duelists'
<b>signature cards</b> are a free bonus for your first win over them.</li></ul>
<h3>TRIPLE TRIAD</h3><ul>
<li><b>Winning cards.</b> When you win, you take cards from your opponent's hand. The town's trade rule decides how
many: ONE (one card of your choice), DIFF (one per point you won by), DIRECT (the cards you turned) or ALL.</li>
<li><b>Sellers</b> in seven POKéMON CENTERS and the GAME CENTER 2F: BRONZE (1★), SILVER (2★) and GOLD (3★) packs of
5 cards, and a few singles (1★ to 3★) that are different in each town. No one sells 4★ or 5★ cards.</li>
<li><b>4★ cards</b> are carried by ordinary players once you have 4 or more badges (more of them with 7 or more).
<b>5★ cards</b> are carried only by the special players below.</li></ul>''')

# POKeMON specials
page.append('<h2 id="pkmn-special">POKéMON cards: the special ones</h2>')
page.append('<p>%d of the %d POKéMON cards are <b>promo cards</b>. They never come in packs, decks or shops. Each one '
            'is handed to you once, as a reward:</p>' % (len(tcg_promos), len(tcg['cards'])))
page.append('<dl class="legend"><dt>First win</dt><dd>A special trainer gives you their promo card the first time you '
            'beat them in a POKéMON card duel.</dd><dt>75% of a set</dt><dd>Own three quarters of a booster set\'s '
            'cards, then claim the promo at the counter\'s PRIZES tab (25% and 50% give packs and CP).</dd>'
            '<dt>All GYM LEADERS</dt><dd>Beat all eight GYM LEADERS\' decks, then claim it under PRIZES.</dd>'
            '<dt>Card milestones</dt><dd>Duels won, packs opened, special trainers beaten, booster sets completed and '
            'more: claim each promo under PRIZES once you get there (what you did before counts too).</dd></dl>')
if missing:
    page.append('<div class="callout warn"><b>%d promo cards can\'t be obtained yet.</b> These Game Boy 2 promos are in '
                'the card list, but no one hands them out in this version of the game, so you can\'t complete the '
                'POKéMON card list. They\'re marked in the table.</div>' % len(missing))
page.append(table('pkmn-promos', ['Card', 'Type', 'How to get it'], tcg_promos,
                  highlight=lambda r: r[2].startswith('No way')))
page.append('<h3>The special trainers</h3><p>They play the full game with 6 Prize cards. Beating one also counts toward '
            'LEGENDS OF THE TABLE, and their deck becomes a recipe you can build in DECK RECIPES.</p>')
st_rows = []
for i, (who, title, where, when) in enumerate(TCG_SPECIALS):
    p = tcg['special'][i]['promo']
    st_rows.append([who, title or '-', where, when, tname(cards[p]) if p else '-'])
page.append(table('pkmn-trainers', ['Trainer', 'Title', 'Where', 'Duels you', 'First-win promo'], st_rows))
qz_rows = []
for i, (who, title, where, when) in enumerate(TCG_SPECIALS):
    p = tcg['special'][i]['promo']
    for region, qwho, qtitle, qwhen in QZ_SPECIALS.get(i, []):
        qz_rows.append([region, qwho, qtitle or '-', who, qwhen, tname(cards[p]) if p else '-'])
page.append('<p>In JOHTO and KANTO, their own trainers play these decks: each stands in for one of HOENN\'s above and '
            'hands over the same promo card the first time you beat them (once, in whichever region you win it first).</p>')
page.append(table('pkmn-trainers-regions', ['Region', 'Trainer', 'Title', "Plays the deck of", 'Duels you',
                                            'First-win promo'], qz_rows))
page.append('<p class="soft">Each Frontier Brain waits outside their own facility once you hold its silver symbol. About half of the Brains you have beaten are out on any day, and the lineup shifts daily.</p>')
if non_promo_special:
    page.append('<h3>Other cards outside packs, shops and ordinary duelists</h3>')
    page.append(table('pkmn-other', ['Card', 'Type', 'Rarity', 'Packs', 'Shops', 'Duelists', 'Special trainers',
                                     'Decks', 'Special'], non_promo_special))
page.append('<div class="callout"><b>Holo cards</b> (%d of them) come only from booster packs: no shop sells them and no '
            'duelist lets you pick them. If you go a long time without a new holo from one kind of pack, the next '
            'rare from that kind is a holo you don\'t have yet.</div>' % len(holos))

# YGO specials
page.append('<h2 id="ygo-special">YU-GI-OH!: the special ones</h2>')
page.append('<p>Every collectible YU-GI-OH! card (%d of them) can be bought at the CARD CLUB with CARD POINTS. These '
            'cards are also a free bonus: each special duelist hands over their <b>signature card</b> the first time '
            'you beat them in a YU-GI-OH! duel. The four Elite Four members also give their POKéMON promo card along '
            'with it.</p>' % len(ygo_rows))
page.append(table('ygo-signatures', ['Duelist', 'Title', 'Where', 'Duels you', 'Signature card'], sig_rows))
page.append('<p class="soft">The CARD CLUB duelist (Oldale and Mauville POKéMON MARTS, Lilycove DEPARTMENT STORE) teaches '
            'the game and lends or gives a Starter Deck. Your first deck is a Starter Deck of your choice.</p>')

# TT specials
page.append('<h2 id="tt-special">TRIPLE TRIAD: the special ones</h2>')
page.append('<p>No one sells 4★ or 5★ cards. All %d <b>5★ cards</b> are carried by special players. Each of them brings '
            'one of their own 5★ cards each day, a different one from day to day. Beat them and take it with the '
            'town\'s trade rule (ONE lets you choose any card). The %d <b>4★ cards</b> are carried by ordinary players '
            'once you have 4 or more badges, and by the special players. The full list is below.</p>' % (fives, fours))
page.append(table('tt-specials', ['Player', 'Where', 'Plays you', 'Their 5★ cards (one each day)', 'Their own 4★'], tt5))
page.append('<div class="callout">With CARD SCALING on, opponents bring hands that suit yours. A special player keeps '
            'their 5★ card in the hand unless your deck is far too weak for it, so bring your best cards. The CARD '
            'QUEEN travels from town to town every few days and changes the local rules wherever she goes.</div>')
page.append('<h3>Where the regions are</h3>')
page.append(table('tt-regions', ['Region', 'Towns and routes', 'Seller'], [list(r) for r in REGIONS]))

# Full indexes
page.append('<h2 id="pkmn-all">Every POKéMON card</h2><p>Highlighted rows are cards you can\'t get from packs, shops or '
            'ordinary duelists. "Duelists" means ordinary card players with decks of that type (%s). "Special trainers" '
            'are the people whose decks you can pick the card from after a win.</p>'
            % '; '.join('%s: %s' % (TYPES[k], v) for k, v in PEOPLE.items()))
page.append(search('pkmn-all-t', 'POKéMON cards'))
page.append(table('pkmn-all-t', ['Card', 'Type', 'Rarity', 'Booster packs', 'Shops', 'Duelists', 'Special trainers',
                                 'Decks', 'Special'], tcg_rows, highlight=lambda r: bool(r[8])))
page.append('<h2 id="ygo-all">Every YU-GI-OH! card</h2><p>Booster codes: %s. "Played by" lists the duelists (from the '
            'Game Boy Advance games) whose decks hold the card. Ordinary card players carry those decks.</p>'
            % ', '.join('%s %s' % (k, v) for k, v in set_names.items() if not any(s['code'] == k and s['starter'] for s in ygo['sets'])))
page.append(search('ygo-all-t', 'YU-GI-OH! cards'))
page.append(table('ygo-all-t', ['Card', 'Kind', 'Rarity', 'Boosters', 'Shops', 'Played by', 'Decks', 'Signature'],
                  ygo_rows, highlight=lambda r: bool(r[7])))
page.append('<h2 id="tt-all">Every TRIPLE TRIAD card</h2><p>"Players" lists the regions whose ordinary players carry the '
            'card. Packs: BRONZE holds every 1★ card, SILVER every 2★ and GOLD every 3★.</p>')
page.append(search('tt-all-t', 'TRIPLE TRIAD cards'))
page.append(table('tt-all-t', ['Card', 'Stars', 'Pack', 'Sold as a single', 'Players', 'Special players'], tt_rows,
                  highlight=lambda r: len(r[1]) >= 4))
page.append('</main><footer>Generated from the game\'s own card tables for version %s.</footer><script>%s</script>'
            '</body></html>' % (e(VERSION), js))
OUT.write_text(''.join(page))
print('%s: %d POKeMON, %d YU-GI-OH!, %d TRIPLE TRIAD cards; %d promos (%d with no source), %d other special'
      % (OUT, len(tcg_rows), len(ygo_rows), len(tt_rows), len(tcg_promos), len(missing), len(non_promo_special)))
