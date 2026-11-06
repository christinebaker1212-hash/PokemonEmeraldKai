#!/usr/bin/env python3
"""Contact sheets of MODERN battle animations (the audit's eyes: docs/MODERN_ANIMS.md).

Plays the given moves in a wild battle on MODERN (ALICE's side attacking), a shot every STEP frames, and lays each
move out as one row of frames; a sheet of up to 4 moves per PNG.

    python3 tools/moemon_modernanim/sheets.py OUTDIR MOVE [MOVE...] [--step 6] [--frames 120]
"""
import os
import re
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]


def main():
    args = [a for a in sys.argv[1:]]
    step = int(args[args.index('--step') + 1]) if '--step' in args else 6
    frames = int(args[args.index('--frames') + 1]) if '--frames' in args else 120
    args = [a for a in args if a != '--big']
    for opt in ('--step', '--frames'):
        if opt in args:
            i = args.index(opt)
            del args[i:i + 2]
    out = Path(args[0]).resolve()
    names = {}
    for line in (ROOT / 'include/constants/moves.h').read_text().splitlines():
        m = re.match(r'#define MOVE_(\w+)\s+(\d+)\b', line)
        if m:
            names[m.group(1)] = int(m.group(2))
    moves = [int(a) if a.isdigit() else names[a] for a in args[1:]]
    out.mkdir(parents=True, exist_ok=True)
    script = ['pressuntil A field 5000', 'mp wild 288:3 45 45 45 45', 'pressuntil A action 3000', 'wait 30']
    for m in moves:
        script += ['mp animall %d %d 0 1' % (m, m), 'rendershotburst m%d %d %d' % (m, frames, step),
                   'waitlog 3000 animall done', 'wait 20']
    script.append('quit')
    (out / 'a.txt').write_text('\n'.join(script) + '\n')
    for f in out.glob('a.sav*'):
        f.unlink()
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_TEST=str(out / 'a.txt'),
               MOEMON_SHOTDIR=str(out), MOEMON_DEBUG_QUICKSTART='1', MOEMON_SKIP_COOP_SETUP='1',
               MOEMON_COOP_CONFIG=str(out / 'a.ini'), MOEMON_CONTROLS_CONFIG=str(out / 'a_controls.ini'),
               MOEMON_SAVE=str(out / 'a.sav'), MOEMON_DEBUG_PARTY='283:40', MOEMON_BATTLE_ANIMS='modern',
               MOEMON_MODERN_AUDIT='1')
    with open(out / 'log.txt', 'w') as log:
        subprocess.run(['timeout', '900', str(ROOT / 'pokeemerald64')], env=env, stdout=log, stderr=subprocess.STDOUT)
    rev = {v: k for k, v in names.items()}
    cols = frames // step
    tw, th = (240, 160) if '--big' in sys.argv else (160, 107)
    for start in range(0, len(moves), 4):
        group = moves[start:start + 4]
        sheet = Image.new('RGB', (cols * tw, len(group) * (th + 12)), (20, 20, 28))
        d = ImageDraw.Draw(sheet)
        for row, m in enumerate(group):
            shots = sorted(list(out.glob('m%d_*.bmp' % m)) + list(out.glob('m%d_*.png' % m)), key=lambda p: int(re.search(r'_(\d+)\.(png|bmp)$', p.name).group(1)))
            d.text((2, row * (th + 12)), '%d %s' % (m, rev.get(m, '?')), fill=(255, 255, 255))
            for i, p in enumerate(shots[:cols]):
                im = Image.open(p).convert('RGB').resize((tw, th))
                sheet.paste(im, (i * tw, row * (th + 12) + 12))
        sheet.save(out / ('sheet_%02d.png' % (start // 4)))
    for p in list(out.glob('m*_*.png')) + list(out.glob('m*_*.bmp')):
        p.unlink()
    print('sheets in', out)


if __name__ == '__main__':
    main()
