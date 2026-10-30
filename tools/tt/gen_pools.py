#!/usr/bin/env python3
"""Where each Triple Triad card can be won and bought in HOENN: writes src/tt/tt_pools.c.

- Region pools: every card of one to four stars belongs to one region's players (whose hands rotate through their
  region's cards day by day, so every card turns up). Themes pull cards to a region (the sea's to SLATEPORT, fire to
  LAVARIDGE...); the rest are spread to even the regions out, lower stars to the regions met earlier.
- Special players (include/tt/npc.h TT_SPECIAL_*): every five-star card belongs to one of them (one in each hand,
  rotating), plus a four-star card or two of their own.
- Sellers: each region's seller stocks a curated few of its region's cards (three stars at most), never more than
  15% of all the cards.
Checked here: every card is in a pool or a special's list, and no seller passes the limit."""
import json, os

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
REGIONS = ['PETALBURG', 'RUSTBORO', 'SLATEPORT', 'MAUVILLE', 'LAVARIDGE', 'LILYCOVE', 'SOOTOPOLIS', 'EVER_GRANDE']
# The stars each region's players carry (the regions in the order a journey meets them)
STARS = {0: (1, 2), 1: (1, 3), 2: (1, 3), 3: (1, 4), 4: (2, 4), 5: (2, 4), 6: (3, 4), 7: (3, 4)}
THEMES = {
    0: ['Dodo', 'Chocobo', 'Moogle', 'Coeurl', 'Sabotender', 'Mandragora', 'Tonberry', 'Bomb', 'Ahriman', 'Morbol',
        'Gaelicat', 'Spriggan', 'Pudding', 'Fat Chocobo', 'Apkallu', 'Namazu', 'Puk'],
    1: ['Titan', 'Golem', 'Stone', 'Iron', 'Rock', 'Gigas', 'Dullahan', 'Gnath', 'Dwarf', 'Mammet', 'Scholar',
        'Sharlayan', 'Archon', 'Urianger', 'Thancred', "Y'shtola", 'Lalafell'],
    2: ['Kraken', 'Sahagin', 'Kojin', 'Siren', 'Sea', 'Pirate', 'Limsa', 'Lominsan', 'Kraken', 'Octopus',
        'Ocean', 'Kuribu', 'Mist', 'Coral', 'Tide', 'Ondo', 'Hancock'],
    3: ['Ramuh', 'Magitek', 'Alexander', 'Cid', 'Machin', 'Brute Justice', 'Gold', 'Saucer', 'Gobbie', 'Goblin',
        'Engine', 'Mecha', 'Weapon', 'Thunder', 'Storm'],
    4: ['Ifrit', 'Salamander', 'Fire', 'Flame', 'Lava', 'Phoenix', 'Amalj', 'Sand', 'Ruby', 'Ash', 'Volcan',
        'Ul\'dah', 'Desert', 'Bomb', 'Rubicante', 'Asura', 'Ravana'],
    5: ['Garuda', 'Cockatrice', 'Bird', 'Ixal', 'Sylph', 'Opo-opo', 'Wind', 'Gryphon', 'Wing', 'Vidofnir', 'Moogle',
        'Gridania', 'Forest', 'Tree', 'Barbariccia', 'Emerald'],
    6: ['Shiva', 'Leviathan', 'Lakshmi', 'Ice', 'Frost', 'Bismarck', 'Sky', 'Cloud', 'Water', 'Wave',
        'Sapphire', 'Cagnazzo', 'Mermaid', 'Sea Serpent', 'Calofisteri', 'Tsukuyomi'],
    7: ['Bahamut', 'Dragon', 'Wyrm', 'Tiamat', 'Odin', 'Diamond', 'Ultima', 'Zodiark', 'Hydaelyn', 'Elidibus',
        'Lahabrea', 'Ascian', 'Emet', 'Hythlodaeus', 'Necron', 'Promathia', 'Shinryu'],
}
SPECIALS = ['STEVEN', 'RIVAL', 'WALLY', 'WALLACE', 'JUAN', 'WINONA', 'NORMAN', 'ROXANNE', 'TATE_LIZA', 'MR_STONE',
            'MR_BRINEY', 'SCOTT', 'BIRCH', 'MOM', 'QUEEN']
SIGNATURES = {
    'STEVEN': ['Omega', 'Ultima, the High Seraph', 'Midgardsormr', 'Shinryu (Vana\'diel)', 'Diamond Weapon'],
    'RIVAL': ['Cloud Strife', 'Squall Leonhart', 'Tidus', 'Lightning', 'Noctis Lucis Caelum'],
    'WALLY': ['Onion Knight', 'Firion', 'Bartz Klauser', 'Zidane Tribal', 'Vaan'],
    'WALLACE': ['Sophia', 'Hydaelyn', 'Venat', 'Queen Eternal', 'Sapphire Weapon'],
    'JUAN': ['Archbishop Thordan VII', 'Zurvan', 'Hades', 'Athena', 'Leviathan'],
    'WINONA': ['Hraesvelgr', 'Nidhogg', 'Phoenix', 'Bahamut', 'Emerald Weapon'],
    'NORMAN': ['Raubahn Aldynn', 'Raubahn & Pipin', 'Gosetsu', 'Warrior of Light', 'Ardbert'],
    'ROXANNE': ['Shantotto', 'Nanamo Ul Namo', 'Kan-E-Senna', 'Dawntrail Krile', 'Louisoix Leveilleur'],
    'TATE_LIZA': ['Stormblood Alphinaud & Alisaie', '9S', '2B', '2P', 'Hisui & Kurenai', 'Alphinaud & Alisaie'],
    'MR_STONE': ['Gaius van Baelsar', 'Regula van Hydrus', 'Varis yae Galvus', 'Zenos yae Galvus', 'Zenos Galvus',
                 'Cid Garlond'],
    'MR_BRINEY': ['Merlwyb Bloefhiswyn', 'Yojimbo & Daigoro', 'Hien', 'Wuk Lamat', 'Hancock'],
    'SCOTT': ['Cecil Harvey', 'Golbez', 'Zeromus', 'Clive Rosfield', 'Zoraal Ja', 'Ruby Weapon'],
    'BIRCH': ['Archaeotania', 'Yiazmat', 'Lindwurm', 'Howling Blade', 'Therion'],
    'MOM': ['Hildibrand & Nashu Mhakaracca', 'Sphene', 'Eden\'s Promise', 'Senor Sabotender'],
    'QUEEN': ['Shadowbringers Warrior of Light', 'Cloud of Darkness', 'Zodiark', 'Endsinger', 'Hephaistos',
              'Shadow Lord'],
}
SELLER_LIMIT = 0.15

def main():
    cards = json.load(open(os.path.join(HERE, 'cards.json')))
    by_name = {c['name']: c for c in cards}
    total = len(cards)
    limit = int(total * SELLER_LIMIT)
    # specials
    special = {s: [] for s in SPECIALS}
    taken = set()
    for s, names in SIGNATURES.items():
        for n in names:
            c = by_name.get(n)
            assert c, n
            assert c['id'] not in taken, n
            special[s].append(c['id'])
            taken.add(c['id'])
    fives = [c for c in cards if c['stars'] == 5 and c['id'] not in taken]
    order = sorted(SPECIALS, key=lambda s: sum(by_name[n]['stars'] == 5 for n in SIGNATURES[s]))
    for i, c in enumerate(fives):   # the five-star cards left: the QUEEN takes most, the rest even out the others
        s = 'QUEEN' if i % 2 == 0 else order[(i // 2) % len(order)]
        special[s].append(c['id'])
        taken.add(c['id'])
    for s in SPECIALS:
        assert sum(by_name_id(cards, i)['stars'] == 5 for i in special[s]) >= 3, s
    # regions: themed first, then evened out
    pools = {r: [] for r in range(8)}
    for c in sorted(cards, key=lambda c: (c['stars'], c['order'])):
        if c['stars'] == 5:
            continue
        lo_hi = [r for r in range(8) if STARS[r][0] <= c['stars'] <= STARS[r][1]]
        themed = [r for r in lo_hi if any(k.lower() in c['name'].lower() for k in THEMES[r])]
        choices = themed or lo_hi
        r = min(choices, key=lambda r: (len([x for x in pools[r] if by_name_id(cards, x)['stars'] == c['stars']]), r))
        pools[r].append(c['id'])
    covered = taken | {i for p in pools.values() for i in p}
    missing = [c['name'] for c in cards if c['id'] not in covered]
    assert not missing, missing
    # sellers: a curated few of their region's cards (three stars at most)
    sellers = {}
    for r in range(8):
        low = [i for i in pools[r] if by_name_id(cards, i)['stars'] <= 3]
        # a balanced shelf: every star, the region's themed cards first
        themed = [i for i in low if any(k.lower() in by_name_id(cards, i)['name'].lower() for k in THEMES[r])]
        rest = [i for i in low if i not in themed]
        shelf = (themed + rest[::2])[:min(limit, 48)]
        sellers[r] = sorted(shelf, key=lambda i: (by_name_id(cards, i)['stars'], by_name_id(cards, i)['order']))
        assert len(sellers[r]) <= limit
    out = ['// Generated by tools/tt/gen_pools.py: where each Triple Triad card is won and sold. Do not edit: rerun it.',
           '#include "tt/npc.h"', '']
    def arr(name, ids):
        out.append('static const uint16_t %s[] = {%s, 0};' % (name, ', '.join(map(str, ids))))
    for r in range(8):
        arr('sPool%d' % r, pools[r])
        arr('sShelf%d' % r, sellers[r])
    for s in SPECIALS:
        arr('sSpecial_%s' % s, special[s])
    out.append('const uint16_t *const gTtRegionPool[TT_REGIONS] = {%s};' % ', '.join('sPool%d' % r for r in range(8)))
    out.append('const uint16_t *const gTtSellerStock[TT_REGIONS] = {%s};' % ', '.join('sShelf%d' % r for r in range(8)))
    out.append('const uint16_t *const gTtSpecialCards[TT_SPECIAL_COUNT] = {%s};' %
               ', '.join('[TT_SPECIAL_%s] = sSpecial_%s' % (s, s) for s in SPECIALS))
    open(os.path.join(ROOT, 'src', 'tt', 'tt_pools.c'), 'w').write('\n'.join(out) + '\n')
    for r in range(8):
        stars = [by_name_id(cards, i)['stars'] for i in pools[r]]
        print('%-12s pool %3d (%s) shelf %2d' % (REGIONS[r], len(pools[r]),
              ' '.join('%d*:%d' % (s, stars.count(s)) for s in range(1, 5)), len(sellers[r])))
    for s in SPECIALS:
        print('%-10s %d cards (%d five-star)' % (s, len(special[s]),
              sum(by_name_id(cards, i)['stars'] == 5 for i in special[s])))
    print('seller limit', limit, 'of', total)

_cache = {}
def by_name_id(cards, i):
    if not _cache:
        _cache.update({c['id']: c for c in cards})
    return _cache[i]

if __name__ == '__main__':
    main()
