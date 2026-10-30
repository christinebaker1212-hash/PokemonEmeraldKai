#!/usr/bin/env python3
"""fight_packs: a copy of the fight pack (links to the real files) with one part changed.
   mkpack.py DIR char: SSF2X Ken's folder has one more file (his hash changes: greyed out for both players)
   mkpack.py DIR core: data/common1.cns has one more line (the engine's data differs: no fight)"""
import os, shutil, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
SRC = os.path.join(REPO, 'mugen')


def mirror(src, dst, changed):
    """dst links to everything in src, except the path changed, which becomes a real folder (recursively)"""
    os.makedirs(dst, exist_ok=True)
    for name in os.listdir(src):
        s, d = os.path.join(src, name), os.path.join(dst, name)
        if changed and (changed == name or changed.startswith(name + '/')):
            if os.path.isdir(s):
                mirror(s, d, changed[len(name) + 1:])
            else:
                shutil.copyfile(s, d)
        else:
            os.symlink(s, d)


def main():
    out, mode = sys.argv[1], sys.argv[2]
    shutil.rmtree(out, ignore_errors=True)
    if mode == 'char':
        mirror(SRC, out, 'chars/ssf2x_ken/x')
        with open(os.path.join(out, 'chars', 'ssf2x_ken', 'readme_extra.txt'), 'w') as f:
            f.write('a file the other player does not have\n')
    else:
        mirror(SRC, out, 'data/common1.cns')
        with open(os.path.join(out, 'data', 'common1.cns'), 'a') as f:
            f.write('\n; changed\n')


main()
