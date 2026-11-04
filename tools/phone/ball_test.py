#!/usr/bin/env python3
"""The POKé BALL page on the phone, the game's side (src/mp/mp_phone_ball.c; docs/PHONE_LINK.md "The POKé BALL
page"), against a headless game through the phone link:
  - the snapshot's "fxu" (the pieces the save has: the first ones of each kind, not the earned ones) and "fxp"
  - "ballfx": an effect of earned pieces kept (the POKéMON's "fx" / "fxs"); one with a piece not earned refused
    ("locked")
  - "ballstyle": a color and decoration on a POKé BALL; a color not earned refused; another kind of ball costs one from
    the BAG (and is refused with none)
  - "fxpreset": MY PRESET 2 saved, in the snapshot's "fxp"; "fxparty": every POKéMON in the party

    python3 tools/phone/ball_test.py      (after make linux; exit 0 = passed)
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import walk_test as W


def fail(msg):
    print('BALL TEST FAILED: ' + msg)
    if W.GAME[0] is not None:
        W.GAME[0].kill()
    sys.exit(1)


def pack(form, part, color, recall):
    base = (form & 0xF) | ((part & 0xF) << 4) | ((color & 0xF) << 8) | ((recall & 0x7) << 12)
    ext = ((form >> 4) & 1) | (((part >> 4) & 1) << 1) | (((color >> 4) & 3) << 2) | (((recall >> 3) & 7) << 4)
    return base | (ext << 16)


def has(hexs, i):
    return i // 4 < len(hexs) and (int(hexs[i // 4], 16) >> (i % 4)) & 1


def main():
    subprocess.check_call(['make', '-s', '-C', os.path.join(W.ROOT, 'tools/phone')])
    work = tempfile.mkdtemp(prefix='balltest')
    state = os.path.join(work, 'phone.state')
    W.PORT = 47700 + os.getpid() % 90
    game = W.start(work, ['mp phone pair', 'wait 60', 'mp save'], True)
    try:
        line = W.wait_line(game, '[test] phone: save')
        f = line.split()
        out = subprocess.run([W.CLI, '127.0.0.1', str(W.PORT), state, 'pair', f[f.index('save') + 1],
                              f[f.index('secret') + 1], 'BALL'], capture_output=True, text=True, timeout=30)
        if '"paired"' not in out.stdout:
            fail('pairing: ' + out.stdout + out.stderr)
        link = W.Link(state)
        link.wait_free()
        snap = link.snap()
        fxu, fxp = snap.get('fxu'), snap.get('fxp')
        if not fxu or len(fxu) != 4 or not fxp or len(fxp) != 6:
            fail('no "fxu" / "fxp" in the snapshot: %s %s' % (fxu, fxp))
        if not has(fxu[1], 11) or has(fxu[1], 12) or not has(fxu[0], 10) or has(fxu[0], 11):
            fail('the first pieces earned, the others not: %s' % fxu)
        mon = snap['party'][0]
        at = {'party': 0}
        who = dict(at=at, p=mon['p'], o=mon['o'])
        print('snapshot: fxu %s, MY PRESETs %s, %s in ball %d' % (fxu, fxp, mon['s'], mon['ball']))

        # an effect of earned pieces: HAUNTED FADE, WISPS + EYES, PURPLE, SMOKE RETREAT, SPOOKY
        d = link.order(op='ballfx', fx=pack(5, 9, 5, 4), fxs=7, **who)
        if not d['ok']:
            fail('an effect: %s' % d)
        mon = link.snap()['party'][0]
        if mon.get('fx') != pack(5, 9, 5, 4) or mon.get('fxs') != 7:
            fail('the effect as kept: %s' % mon)
        # SAND SCULPT (20) isn't earned: refused
        d = link.order(op='ballfx', fx=pack(20, 9, 5, 4), fxs=7, **who)
        if d['ok'] or d.get('why') != 'locked':
            fail('a piece not earned: %s' % d)
        print('ballfx: kept; a piece not earned refused')

        # a POKé BALL's color and decoration; a color not earned (CRIMSON, 11) refused
        d = link.order(op='ballstyle', item=4, color=5, deco=1, **who)
        if not d['ok']:
            fail('a color and decoration: %s' % d)
        mon = link.snap()['party'][0]
        style = mon.get('bst', 0)
        if style & 0xF != 5 or (style >> 4) & 0x7F != 1 or mon['ball'] != 4:
            fail('the style as kept: %s' % mon)
        d = link.order(op='ballstyle', item=4, color=11, deco=0, **who)
        if d['ok'] or d.get('why') != 'locked':
            fail('a color not earned: %s' % d)
        # another kind of ball: none in the BAG, refused
        d = link.order(op='ballstyle', item=2, color=0, deco=0, **who)
        if d['ok'] or d.get('why') != 'noitem':
            fail('a ball not in the BAG: %s' % d)
        print('ballstyle: PURPLE with a STAR; a color not earned and a ball not in the BAG refused')

        # MY PRESET 2, then the whole party
        d = link.order(op='fxpreset', slot=1, fx=pack(1, 2, 3, 1), fxs=5)
        if not d['ok']:
            fail('MY PRESET 2: %s' % d)
        snap = link.snap()
        if snap['fxp'][1] != [pack(1, 2, 3, 1), 5]:
            fail('MY PRESET 2 as kept: %s' % snap['fxp'])
        d = link.order(op='fxparty', fx=pack(1, 2, 3, 1), fxs=5)
        if not d['ok']:
            fail('the party: %s' % d)
        snap = link.snap()
        if any(m.get('fx') != pack(1, 2, 3, 1) for m in snap['party'] if not m.get('e')):
            fail('not every POKéMON in the party: %s' % [m.get('fx') for m in snap['party']])
        print('fxpreset: MY PRESET 2 kept; fxparty: %d POKéMON' % len(snap['party']))
    finally:
        game.kill()
    print('BALL TEST PASSED')


if __name__ == '__main__':
    main()
