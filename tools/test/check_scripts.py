#!/usr/bin/env python3
"""Script arguments the game looks up in its own tables must be in range (JOHTO / KANTO's scripts come from Moemon
Quetzal, whose numbers can differ): every `bufferstdstring VAR, N` has an entry N in gStdStrings.

    python3 tools/test/check_scripts.py
"""
import glob
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def main():
    menu = open(os.path.join(REPO, 'src/data/script_menu.h')).read()
    table = menu[menu.index('const u8 *const gStdStrings[] ='):]
    table = table[:table.index('};')]
    consts = dict(re.findall(r'#define (STDSTRING_\w+)\s+(\d+)', open(os.path.join(REPO, 'include/constants/script_menu.h')).read()))
    have = set()
    for k in re.findall(r'\[(\w+)\]\s*=', table):
        have.add(int(consts[k]) if k in consts else int(k, 0))
    bad = 0
    files = glob.glob(os.path.join(REPO, 'data/maps/*/scripts.inc')) + glob.glob(os.path.join(REPO, 'data/scripts/**/*.inc'), recursive=True)
    for f in files:
        for n, line in enumerate(open(f, encoding='utf-8', errors='replace'), 1):
            m = re.match(r'\s*bufferstdstring\s+\w+,\s*(\w+)', line)
            if m:
                v = m.group(1)
                if v.startswith('VAR_') or v.startswith('0x8'):
                    continue   # (a variable: checked in the game, ScrCmd_bufferstdstring)
                idx = int(consts[v]) if v in consts else int(v, 0)
                if idx not in have:
                    print('%s:%d: bufferstdstring %s has no gStdStrings entry' % (os.path.relpath(f, REPO), n, v))
                    bad += 1
    print('scripts: %s' % ('ok' if not bad else '%d out of range' % bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
