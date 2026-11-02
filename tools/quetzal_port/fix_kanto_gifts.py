#!/usr/bin/env python3
"""KANTO's gifts where FireRed's differ from Quetzal's (the TMs are fix_kanto_tms.py's, the HMs fix_hm_givers.py's).

    python3 tools/quetzal_port/fix_kanto_gifts.py      (run from anywhere; safe to run again)

- ROUTE 15's gate, 2F: PROF. OAK's AIDE gives the EXP. SHARE for 50 kinds of POKéMON, as in FireRed (Quetzal: a
  LUCKY EGG, which stays obtainable: held by wild CHANSEY, the MoeGatcha), with FireRed's words about it.
"""
import os
import re

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MARK = '@ Moemon Kai: FireRed\'s gift (tools/quetzal_port/fix_kanto_gifts.py)'
GIFTS = {   # map: ([(Quetzal's item, FireRed's)], {text label: FireRed's words})
    'Route15_WestEntrance_2F': ([('ITEM_LUCKY_EGG', 'ITEM_EXP_SHARE')], {
        '844FBB9': ['EXP. SHARE is an item to be held\\n', 'by a POKéMON.\\p', 'The POKéMON will receive a share\\n',
                    'of the EXP. Points without having\\l', 'to battle.$'],
    }),
}


def kai_words(s):
    return s.replace('POKéMON', 'MOéMON').replace("'", '’')


def main():
    for name, (swaps, texts) in GIFTS.items():
        path = os.path.join(REPO, 'data/maps/Kanto_%s/scripts.inc' % name)
        s = open(path).read()
        if MARK in s:
            continue
        first = min(s.index(old) for old, _ in swaps)
        line = s.rindex('\n', 0, first) + 1
        s = s[:line] + '\t%s (Quetzal gave %s)\n' % (MARK, ', '.join(o.replace('ITEM_', '') for o, _ in swaps)) + s[line:]
        for old, new in swaps:
            s = s.replace(old, new)
        for label, lines in texts.items():
            full = 'Kanto_%s_Text_%s' % (name, label)
            t = re.search(re.escape(full) + r'::\n((?:\t\.string .*\n)+)', s)
            if not t:
                raise SystemExit('%s: no %s' % (name, full))
            s = s[:t.start(1)] + ''.join('\t.string "%s"\n' % kai_words(l) for l in lines) + s[t.end(1):]
        open(path, 'w').write(s)
        print(name, ', '.join('%s for %s' % (new, old) for old, new in swaps))


if __name__ == '__main__':
    main()
