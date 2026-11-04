#!/usr/bin/env python3
"""ASTRAL BALL: every catch logged its new personality; the party Moemon has it, is shiny, and kept what shows."""
import re, sys

log = open(sys.argv[1] + '/a.log', errors='replace').read()


def unown(p):
    return ((((p >> 24) & 3) << 6) | (((p >> 16) & 3) << 4) | (((p >> 8) & 3) << 2) | (p & 3)) % 28


astral = re.findall(r'astral: species (\d+) personality ([0-9a-f]{8}) -> ([0-9a-f]{8}) gender (\d+) -> (\d+) ot ([0-9a-f]{8})', log)
mons = {int(m[0]): m for m in re.findall(r'\[test\] mon (\d+): species (\d+) personality ([0-9a-f]{8}) nature (\d+) gender (\d+) shiny (\d+) style ([0-9a-f]{4}) bad (\d+)', log)}
fail = 0
seen = {}
for sp, old, new, g0, g1, ot in astral:
    seen[int(sp)] = (int(old, 16), int(new, 16), int(g0), int(g1), int(ot, 16))
for slot, sp in ((1, 288), (2, 201), (3, 290), (4, 384)):
    if sp not in seen or slot not in mons:
        print('missing: slot %d species %d' % (slot, sp))
        fail = 1
        continue
    old, new, g0, g1, ot = seen[sp]
    _, msp, pers, nature, gender, shiny, style, bad = mons[slot]
    pers = int(pers, 16)
    checks = {
        'species': int(msp) == sp,
        'personality': pers == new,
        'shiny': shiny == '1' and ((ot >> 16) ^ (ot & 0xFFFF) ^ (pers >> 16) ^ (pers & 0xFFFF)) < 8,
        'nature': int(nature) == old % 25,
        'gender': g0 == g1 == int(gender),
        'unown letter': sp != 201 or unown(old) == unown(new),
        'wurmple': ((old >> 16) % 10 <= 4) == ((new >> 16) % 10 <= 4),
        'astral style': int(style, 16) >> 12 == 14,
        'not a bad egg': bad == '0',
    }
    bad_ = [k for k, v in checks.items() if not v]
    print('slot %d species %d: %08x -> %08x %s' % (slot, sp, old, new, 'ok' if not bad_ else 'FAILED ' + ', '.join(bad_)))
    fail |= bool(bad_)
sys.exit(fail)
