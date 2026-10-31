#!/usr/bin/env python3
"""MOéMON's words in the game's text, as HOENN's scripts have them: POKéMON -> MOéMON, POKéDEX -> MOéDEX, POKéNAV ->
MOéNAV and every other POKé word -> MOé (POKéMANIAC, POKéRUS...), but POKé BALL stays (the item keeps its name; MOé
FLUTE and MOé DOLL are the items' names). Mixed case (Pokémon, Poké) goes the same way, in capitals as the game
writes it.

    python3 -I tools/moemon_text/moe_words.py FILE...      # rewrites the .string lines of these script files
"""
import re
import sys

_WORD = re.compile(r"(POKé|Poké)(?! ?BALL)(?! ?Ball)([A-Za-zé]*)")


def moe(text):
    """The rule on a piece of text (not on labels or code)"""
    def sub(m):
        rest = m.group(2)
        return 'MOé' + rest.upper().replace('É', 'é')
    return _WORD.sub(sub, text)


def rewrite(path):
    lines = open(path, encoding='utf-8').read().split('\n')
    changed = 0
    for i, line in enumerate(lines):
        s = line.lstrip()
        if s.startswith('.string') and ('POKé' in line or 'Poké' in line):
            new = moe(line)
            if new != line:
                lines[i] = new
                changed += 1
    if changed:
        open(path, 'w', encoding='utf-8').write('\n'.join(lines))
    return changed


def main():
    total = 0
    for path in sys.argv[1:]:
        total += rewrite(path)
    print('%d lines changed in %d files' % (total, len(sys.argv) - 1))


if __name__ == '__main__':
    main()
