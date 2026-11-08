#!/usr/bin/env python3
"""The story scenes' results (tools/orre/gen_story_tests.py's scenarios): each step's flag, set or not, with what its
log shows last (the scene's last message, natives not converted on the way).

    python3 -I tools/orre/check_scenes.py RESULTS_DIR     (regress.sh's OUT: orre_scenes_NN/a.log in it)
"""
import glob
import os
import re
import sys


def main():
    ok = bad = 0
    for log in sorted(glob.glob(os.path.join(sys.argv[1], 'orre_scenes_*', 'a.log'))):
        text = open(log, errors='replace').read()
        for step in text.split('[test] orre story jump ')[1:]:
            flag = int(step.split()[0], 16)
            m = re.search(r'\[test\] orreget 0x([0-9A-F]+) (\d+)', step)
            if not m:
                continue
            if m.group(2) != '0':
                ok += 1
                continue
            bad += 1
            msgs = re.findall(r'orre vm: message (\d+)', step)
            nat = sorted(set(re.findall(r'native (\d+) not converted', step)))
            warp = re.search(r'warp: (\S+ \S+)', step)
            script = re.search(r'orrescript 100 (\d+)', step)
            print('flag %4d  %s  %s  last message %s  %s' % (
                flag, warp.group(1) if warp else '?', 'func ' + script.group(1) if script else 'main',
                msgs[-1] if msgs else '-', 'natives ' + ','.join(nat) if nat else ''))
    print('%d set, %d not' % (ok, bad))


if __name__ == '__main__':
    main()
