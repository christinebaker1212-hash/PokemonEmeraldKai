#!/usr/bin/env python3
"""Which items can be had at each GENERATION (3-9): an item counts when some source names it.

    python3 tools/moemon_gen59/item_audit.py [--out docs/audit_items.txt]

Sources: every map's and shared script (marts, gifts, item balls, prizes, hidden items in map.json), the shops' extra
lists (shop.c) and JOHTO / KANTO's badge marts, MoeGatcha (mp_gatcha.c), PICKUP and the other src/ tables that hand
items out (SOURCE_FILES), and wild POKéMON's held items (species_info*.h: the species must be obtainable at that
GENERATION, from obtain_audit.py). An item's generation is mp_region.c MP_Gen_OfItem's; 0 (Kai's own, story items)
and the gimmick items (MEGA STONES, Z-CRYSTALS: the gimmick shop) count from GEN 3. Unused slots ("????????") and the
items no player can use (EXEMPT) are left out. Exits 1 if anything is missing.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import obtain_audit as oa   # noqa: E402

REPO = oa.REPO
SOURCE_FILES = ['src/shop.c', 'src/data/qz_badge_marts.h', 'src/mp/mp_gatcha.c', 'src/battle_script_commands.c',
                'src/mp/mp_candy.c', 'src/mp/mp_tmrefund.c', 'src/mp/mp_achieve.c', 'src/mp/mp_daily.c',
                'src/qz_bugcontest.c', 'src/qz_specials.c', 'src/mp/mp_farm.c', 'src/data/battle_frontier/battle_frontier_exchange_corner.h',
                'src/frontier_util.c', 'src/battle_pyramid.c', 'src/lottery_corner.c', 'src/mp/mp_life.c', 'src/mp/mp_life_field.c',
                'src/data/wild_qz_held.h']
EXEMPT = {   # never meant to be had (removed from the games, or a game's machinery)
    'ITEM_NONE', 'ITEM_ENIGMA_BERRY',
    # FireRed's key items: the game has its own (the BAG's pockets, the MATCH CALL, JOHTO / KANTO's PARCEL...)
    'ITEM_OAKS_PARCEL', 'ITEM_TOWN_MAP', 'ITEM_VS_SEEKER', 'ITEM_FAME_CHECKER', 'ITEM_TM_CASE', 'ITEM_BERRY_POUCH',
    'ITEM_TEACHY_TV',
}


def items():
    out = {}
    for m in re.finditer(r'#define (ITEM_\w+) (\d+)\b', oa.rd('include/constants/items.h')):
        out.setdefault(m.group(1), int(m.group(2)))
    # ITEM_TM_<MOVE> / ITEM_HM_<MOVE>: tms_hms.h's order from ITEM_TM01 / ITEM_HM01
    t = oa.rd('include/constants/tms_hms.h')
    for kind in ('TM', 'HM'):
        body = t.split('#define FOREACH_%s(F)' % kind)[1].split('\n\n')[0]
        for k, mv in enumerate(re.findall(r'F\((\w+)\)', body)):
            out['ITEM_%s_%s' % (kind, mv)] = out['ITEM_%s01' % kind] + k
    return out


def names():
    """items with a real name (unused slots are '????????')"""
    ok = set()
    for f in ('items.h', 'items_gen4.h', 'items_gen4_berries.h', 'items_qz.h', 'items_gen59.h'):
        p = os.path.join(REPO, 'src/data', f)
        if not os.path.exists(p):
            continue
        for m in re.finditer(r'\[(ITEM_\w+)\]\s*=\s*\{\s*\.name\s*=\s*_\("([^"]*)"\)', open(p).read()):
            if '????' not in m.group(2):
                ok.add(m.group(1))
    return ok


def item_gen(ids):
    """MP_Gen_OfItem"""
    src = oa.rd('src/mp/mp_region.c')
    later = [int(x) for x in re.search(r'sLater\[\] = \{[^\n]*\n\s*([\d, ]+)\}', src).group(1).replace(' ', '').split(',') if x]
    g59 = [int(x) for x in re.search(r'sGen59\[\] = \{[^\n]*\n(.*?)\};', src, re.S).group(1).replace('\n', '').replace(' ', '').split(',') if x]
    g = {}
    for n, i in ids.items():
        if i <= ids['ITEM_TOPO_BERRY']:
            g[n] = 3
        elif ids['ITEM_QUICK_POWDER'] <= i <= ids['ITEM_RAGE_CANDY_BAR']:
            g[n] = later[i - ids['ITEM_QUICK_POWDER']]
        elif n in ('ITEM_CANDY_JAR', 'ITEM_CARD_CASE'):
            g[n] = 0
        elif n == 'ITEM_LINKING_CORD':
            g[n] = 3
        elif ids['ITEM_CARD_CASE'] < i <= ids['ITEM_ROWAP_BERRY']:
            g[n] = 4
        elif ids['ITEM_ICE_STONE'] <= i < ids['ITEM_ICE_STONE'] + len(g59):
            g[n] = g59[i - ids['ITEM_ICE_STONE']]
        else:
            g[n] = 0
    return g


def sources():
    """item -> set of where (without held items)"""
    found = {}

    def scan(text, where):
        # (a script line that only asks about an item, or takes one, gives nothing)
        text = '\n'.join(l for l in text.split('\n') if not re.search(
            r'\b(checkitem\w*|removeitem|takeitem|bufferitemname\w*|checkpcitem|goto_if|call_if|compare|qz_checkitem\w*)\b', l))
        for t in set(re.findall(r'\bITEM_[A-Z0-9_]+\b', text)):
            found.setdefault(ALIAS.get(t, t), set()).add(where)
    for root in ('data/maps', 'data/scripts'):
        for dp, _, fs in os.walk(os.path.join(REPO, root)):
            for f in fs:
                if f.endswith(('.inc', '.json')):
                    scan(open(os.path.join(dp, f), encoding='utf-8', errors='replace').read(),
                         'script ' + os.path.basename(dp if root == 'data/maps' else f))
    for f in SOURCE_FILES:
        p = os.path.join(REPO, f)
        if os.path.exists(p):
            scan(open(p, encoding='utf-8', errors='replace').read(), f)
    return found


def held():
    """species -> held items"""
    out = {}
    for f in ('species_info.h', 'species_info_gen4.h', 'species_info_gen59.h'):
        for m in re.finditer(r'\[SPECIES_(\w+)\]\s*=\s*\{(.*?)\n\s*\},', oa.rd('src/data/pokemon/' + f), re.S):
            its = re.findall(r'\.item(?:Common|Rare)\s*=\s*(ITEM_\w+)', m.group(2))
            out[m.group(1)] = [i for i in its if i != 'ITEM_NONE']
    return out


ALIAS = {}   # ITEM_TM06 -> ITEM_TM_TOXIC (items.h's names)


def main():
    out = sys.argv[sys.argv.index('--out') + 1] if '--out' in sys.argv else None
    ids, named = items(), names()
    by_id = {}
    for n in named:
        if n in ids:
            by_id[ids[n]] = n
    for n, i in ids.items():
        if i in by_id and by_id[i] != n:
            ALIAS[n] = by_id[i]
    named = {n for n in named if n in ids}
    gen = item_gen(ids)
    src = sources()
    hold = held()
    lines, bad = [], False
    for g in range(3, 10):
        mons = oa.obtainable(g)
        have = {i for i in src}
        for s in mons:
            have.update(hold.get(s, ()))
        want = sorted((i for i in named if i not in EXEMPT and gen.get(i, 0) <= g), key=lambda i: ids[i])
        miss = [i for i in want if i not in have]
        bad |= bool(miss)
        lines.append('GEN %d: %d of %d items can be had, %d missing' % (g, len(want) - len(miss), len(want), len(miss)))
        lines += ['    %3d %s (Gen %d)' % (ids[i], i, gen.get(i, 0)) for i in miss]
    text = '\n'.join(lines) + '\n'
    if out:
        open(os.path.join(REPO, out), 'w').write(text)
    print(text, end='')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
