#!/usr/bin/env python3
"""Can every JOHTO / KANTO story trigger fire? For each trigger (a map's coord events, its frame scripts
map_script_2 VAR, VALUE) the value it waits for must be one the game can reach: set by some script (setvar), by
code (qz_specials.c / mp_region.c VarSet with a constant, the region's first-arrival table), reached by counting
(addvar from a value that is reachable), or the variable's start value (0). Prints the triggers whose value nothing
reaches; exits 1 if there are any.

    python3 tools/quetzal_port/story_vars.py [-v]
"""
import json
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def rd(p):
    return open(os.path.join(REPO, p), encoding='utf-8', errors='replace').read()


def num(v):
    try:
        return int(v, 0)
    except ValueError:
        return None


# Triggers that wait for a value no script sets, and why that's right (checked against Quetzal's ROM)
KNOWN = {
    99: 'pressed by a boulder pushed onto it (FireRed\'s switches: field_control_avatar.c TryRunBoulderSwitchScript)',
    ('VAR_QZ_0x5076', None): 'GIOVANNI\'s past (not ported: src/qz_specials_stub.c)',
    ('VAR_QZ_0x5077', None): 'BILL\'s time travel (not ported: src/qz_specials_stub.c)',
    ('VAR_QZ_0x5015', 1): 'GOLDENROD\'s elevator arrival, set by Quetzal\'s own elevator code (the port\'s panel opens the doors instead)',
    ('VAR_QZ_0x5008', 2): 'set nowhere in Quetzal either (ROUTE 32\'s leftover step triggers)',
    ('VAR_QZ_0x5032', 2): 'set nowhere in Quetzal either (JOHTO\'s SAFARI ZONE is free: no Safari game to leave)',
    ('VAR_QZ_0x5113', 1): 'set nowhere in Quetzal either (INDIGO PLATEAU)',
    ('VAR_QZ_0x5100', 8): 'set nowhere in Quetzal either (TWO ISLAND\'s GAME CORNER)',
}


def known(v, want):
    return KNOWN.get(want) if want == 99 else KNOWN.get((v, want)) or KNOWN.get((v, None))


def main():
    verbose = '-v' in sys.argv
    groups = json.load(open(os.path.join(REPO, 'data/maps/map_groups.json')))
    names = [n for g in groups['group_order'][34:40] for n in groups[g]]
    # every script text (maps, shared Quetzal scripts, Kai's own)
    texts = []
    for root in ('data/maps', 'data/scripts'):
        for dp, _, fs in os.walk(os.path.join(REPO, root)):
            texts += [open(os.path.join(dp, f), errors='replace').read() for f in fs if f.endswith('.inc')]
    alltext = '\n'.join(texts)
    sets, adds = {}, {}
    for v, val in re.findall(r'^\s*setvar (VAR_\w+), (\w+)', alltext, re.M):
        if num(val) is not None:
            sets.setdefault(v, set()).add(num(val))
    for v, val in re.findall(r'^\s*addvar (VAR_\w+), (\w+)', alltext, re.M):
        adds.setdefault(v, set()).add(num(val) or 1)
    for v, val in re.findall(r'^\s*copyvar (VAR_\w+), (VAR_\w+)', alltext, re.M):
        sets.setdefault(v, set()).add('copy')
    code = rd('src/qz_specials.c') + rd('src/mp/mp_region.c') + rd('src/data/qz_region_init.h')
    for v, val in re.findall(r'VarSet\((VAR_\w+),\s*(\w+)\)', code):
        if num(val) is not None:
            sets.setdefault(v, set()).add(num(val))
    for v, val in re.findall(r'\{(VAR_\w+|0x[0-9A-Fa-f]+),\s*(\d+)\}', rd('src/data/qz_region_init.h')):
        vv = v if v.startswith('VAR_') else 'VAR_QZ_0x%X' % int(v, 16)
        sets.setdefault(vv, set()).add(int(val))

    def reachable(v, want):
        s = sets.get(v, set())
        if want == 0 or want in s or 'copy' in s:
            return True
        # counting up from a reachable value
        for step in adds.get(v, ()):
            base = want - step
            seen = 0
            while base >= 0 and seen < 64:
                if base == 0 or base in s:
                    return True
                base -= step
                seen += 1
        return False

    triggers, bad = 0, []
    for name in names:
        m = json.load(open(os.path.join(REPO, 'data/maps', name, 'map.json')))
        for c in m.get('coord_events') or []:
            v = c.get('var', '0')
            if c.get('type', 'trigger') != 'trigger' or v in ('0', 0):
                continue
            want = num(str(c.get('var_value', '0')))
            triggers += 1
            if want is not None and not reachable(v, want) and not known(v, want):
                bad.append('%s: step trigger (%d, %d) %s waits for %s = %d' % (name, c['x'], c['y'], c['script'], v, want))
        p = os.path.join(REPO, 'data/maps', name, 'scripts.inc')
        if os.path.exists(p):
            for v, val, script in re.findall(r'map_script_2 (\w+), (\w+), (\w+)', open(p).read()):
                want = num(val)
                triggers += 1
                if want is not None and not reachable(v, want) and not known(v, want):
                    bad.append('%s: frame script %s waits for %s = %d' % (name, script, v, want))
    print('%d triggers, %d wait for a value nothing sets' % (triggers, len(bad)))
    for b in bad:
        print('  ' + b)
    if verbose:
        for v in sorted(sets):
            print(v, sorted(x for x in sets[v] if x != 'copy'), 'add', sorted(adds.get(v, ())))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
