#!/usr/bin/env python3
"""shadow_covered's CHECK: the player's own shadow (mp shadows' "shadow player" lines, in the order the scenario
stands): there in the open at (2,5), gone at (1,5) and (0,5) where the tile's top layer covers the feet.

    python3 tools/test/check_shadow_covered.py RESULT_DIR
"""
import os
import re
import sys


def main():
    log = open(os.path.join(sys.argv[1], 'a.log'), errors='replace').read()
    got = [int(v) for v in re.findall(r'shadow player: \d+ sprites used, its shadow (\d+)', log)]
    want = ['a shadow', 'none', 'none']
    if len(got) != 3:
        sys.exit('FAIL: %d "shadow player" lines, not 3' % len(got))
    seen = ['none' if v == 128 else 'a shadow' for v in got]
    if seen != want:
        sys.exit('FAIL: the player had %s (open, behind, behind), not %s' % (seen, want))
    print('shadow covered: in the open a shadow, behind the cover none')


if __name__ == '__main__':
    main()
