#!/usr/bin/env python3
"""MODERN never draws over the battle's UI: the same moves on CLASSIC and on MODERN, a shot every frame, and the text box
(rows 112-159: BG0's box and menus) must be the same pixel for pixel. (The HP boxes are kept the same way, by the
renderer's UI mask, gba_easy_draw.c gMpUiLayer; look at diff.png for them.)

    python3 tools/moemon_modernanim/ui_check.py OUTDIR [MOVE...]
"""
import os
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]


def run(out, mode, moves):
    d = out / mode
    d.mkdir(parents=True, exist_ok=True)
    for f in d.glob('*'):
        f.unlink()
    script = ['pressuntil A field 5000', 'mp wild 288:3 45 45 45 45', 'pressuntil A action 3000', 'wait 30']
    for i, m in enumerate(moves):
        script += ['mp animall %d %d 0 1' % (m, m), 'shotburst m%d 140 1' % i, 'waitlog 3000 animall done', 'wait 10']
    (d / 'a.txt').write_text('\n'.join(script + ['quit']) + '\n')
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_TEST=str(d / 'a.txt'),
               MOEMON_SHOTDIR=str(d), MOEMON_DEBUG_QUICKSTART='1', MOEMON_SKIP_COOP_SETUP='1',
               MOEMON_COOP_CONFIG=str(d / 'a.ini'), MOEMON_CONTROLS_CONFIG=str(d / 'c.ini'), MOEMON_SAVE=str(d / 'a.sav'),
               MOEMON_DEBUG_PARTY='283:40', MOEMON_BATTLE_ANIMS=mode)
    with open(d / 'log.txt', 'w') as log:
        subprocess.run(['timeout', '600', str(ROOT / 'pokeemerald64')], env=env, stdout=log, stderr=subprocess.STDOUT)


def main():
    out = Path(sys.argv[1]).resolve()
    moves = [int(a) for a in sys.argv[2:]] or [5, 89, 63, 57, 85, 14]   # MEGA PUNCH, EARTHQUAKE, HYPER BEAM, SURF...
    run(out, 'classic', moves)
    run(out, 'modern', moves)
    bad = n = 0
    for f in sorted((out / 'modern').glob('m*_*.*')):
        c = out / 'classic' / f.name
        if not c.exists():
            continue
        a, b = Image.open(f).convert('RGB'), Image.open(c).convert('RGB')
        n += 1
        if ImageChops.difference(a.crop((0, 112, 240, 160)), b.crop((0, 112, 240, 160))).getbbox():
            bad += 1
            print('text box differs:', f.name)
    print('%d frames compared, %d with the text box changed' % (n, bad))
    sys.exit(1 if bad or n == 0 else 0)


if __name__ == '__main__':
    main()
