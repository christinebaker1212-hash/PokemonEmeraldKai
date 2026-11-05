#!/usr/bin/env python3
"""Drop script blocks and texts nothing refers to from a map's scripts.inc (after an audit case cut a branch).

    python3 tools/quetzal_port/drop_dead_labels.py MAP [MAP ...]

A block goes only when no file under data/ or src/ names its label and it isn't reached by falling through from the
block above (that one ends with end / return / goto / step_end / a text's last line). Repeats until nothing changes.
"""
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TERM = re.compile(r'^\t(end|return|goto \S+|step_end|releaseall\n?|\.string ".*\$")\s*$')


def refs(label):
    out = subprocess.run(['grep', '-rlw', '--include=*.inc', '--include=*.json', '--include=*.c', '--include=*.h',
                          '--include=*.s', label, 'data', 'src'], cwd=REPO, capture_output=True, text=True).stdout
    return out.split()


def prune(mapname):
    path = os.path.join(REPO, 'data/maps', mapname, 'scripts.inc')
    dropped = 0
    while True:
        lines = open(path).read().split('\n')
        starts = [i for i, l in enumerate(lines) if re.match(r'^\w+::?$', l)]
        gone = None
        for k, i in enumerate(starts):
            label = lines[i].rstrip(':')
            if label.endswith('_MapScripts'):
                continue
            # fall-through: the last code line above this label
            j = i - 1
            while j >= 0 and (lines[j].strip() == '' or lines[j].startswith('@')):
                j -= 1
            if j >= 0 and not TERM.match(lines[j]) and not re.match(r'^\w+::?$', lines[j]):
                continue
            uses = [f for f in refs(label)]
            text = open(path).read()
            if len(re.findall(r'\b%s\b' % re.escape(label), text)) > 1 or any(not f.endswith(mapname + '/scripts.inc') for f in uses):
                continue
            end = starts[k + 1] if k + 1 < len(starts) else len(lines)
            # keep the comment lines that belong to the next block
            while end - 1 > i and lines[end - 1].startswith('@'):
                end -= 1
            gone = (i, end)
            break
        if not gone:
            break
        del lines[gone[0]:gone[1]]
        open(path, 'w').write(re.sub(r'\n{3,}', '\n\n', '\n'.join(lines)))
        dropped += 1
    print('%s: %d blocks dropped' % (mapname, dropped))


if __name__ == '__main__':
    for m in sys.argv[1:]:
        prune(m)
