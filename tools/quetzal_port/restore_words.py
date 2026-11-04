#!/usr/bin/env python3
"""Put FireRed's / Crystal's own words back over Quetzal's (docs/REGIONS_AUDIT.md), from a list.

    python3 tools/quetzal_port/restore_words.py [LIST]     (default tools/quetzal_port/restore_words.txt)

Each line of the list: `OUR_MAP OUR_LABEL fr|gsc THEIR_MAP THEIR_LABEL` (# starts a comment). OUR_LABEL's .string lines
in data/maps/OUR_MAP/scripts.inc become THEIR_LABEL's (fr: pret/pokefirered data/maps/THEIR_MAP/text.inc or a data/
path, firered_words.py; gsc: pret/pokecrystal maps/THEIR_MAP.asm, crystal_words.py). The label and everything around
it stay. Running it again changes nothing. A MoemonMaker/ edit is never touched (they sit on top at run time).
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import firered_words   # noqa: E402
import crystal_words   # noqa: E402


def body_of(kind, their_map, their_label):
    if kind == 'fr':
        return firered_words.words(their_map, their_label)
    return crystal_words.words(their_map, their_label)


def restore(our_map, our_label, kind, their_map, their_label):
    path = os.path.join(REPO, 'data/maps', our_map, 'scripts.inc')
    text = open(path, encoding='utf-8').read()
    m = re.search(r'^%s::?[ \t]*\n((?:[ \t]*\.string[^\n]*\n)+)' % re.escape(our_label), text, re.M)
    if not m:
        raise SystemExit('%s: no text %s' % (our_map, our_label))
    new = body_of(kind, their_map, their_label)
    if m.group(1) == new:
        return False
    text = text[:m.start(1)] + new + text[m.end(1):]
    open(path, 'w', encoding='utf-8').write(text)
    return True


def main():
    lst = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, 'restore_words.txt')
    changed = 0
    for line in open(lst, encoding='utf-8'):
        line = line.split('#', 1)[0].split()
        if not line:
            continue
        if len(line) != 5 or line[2] not in ('fr', 'gsc'):
            raise SystemExit('bad line: %s' % ' '.join(line))
        if restore(*line):
            changed += 1
            print('restored %s %s' % (line[0], line[1]))
    print('%d texts restored' % changed)


if __name__ == '__main__':
    main()
