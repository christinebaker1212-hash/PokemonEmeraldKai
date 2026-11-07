#!/usr/bin/env python3
"""The phone's EGG INCUBATOR against a headless game (docs/PHONE_LINK.md "The EGG INCUBATOR"):

  - three EGGS from the party go into the incubator: each leaves the save, the game saves, the snapshot's "incubator"
    lists them; a POKeMON that isn't an EGG is refused ("notegg")
  - the game started again: all three still in the incubator (custody survives a restart)
  - home to a box without room for both: refused ("full"); to a box with room: the one that walked 400 steps hatched
    (a PICHU EGG's 10 cycles of 32 steps), the one that walked 96 came back an EGG with 3 cycles fewer, both in that
    box; 200,000 steps count as no more than 10,000 a day; the third stays in the incubator
  - started again: no copies (the two that came home stay home), one still in the incubator; FORGET PHONES brings it
    home as it left (to the PC)

    python3 tools/phone/incubator_test.py      (after make linux; exit 0 = passed)
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import walk_test as W   # (the walk test's game, link and helpers)


def fail(msg):
    print('INCUBATOR TEST FAILED: ' + msg)
    g = W.GAME[0]
    if g is not None:
        g.kill()
    sys.exit(1)


def eggs_in(snap):
    return [(i, m) for i, m in enumerate(snap['party']) if m.get('e') == 1]


def everyone(snap):
    out = list(snap['party'])
    for b in snap['boxes']:
        out += [m for m in b['m'] if m]
    return out


def main():
    subprocess.check_call(['make', '-s', '-C', os.path.join(W.ROOT, 'tools/phone')])
    work = tempfile.mkdtemp(prefix='inctest')
    state = os.path.join(work, 'phone.state')
    fill = ['mp boxput 2 %d 25 5' % k for k in range(29)]   # (box 2: one place left)
    game = W.start(work, ['mp giveegg 172', 'mp giveegg 172', 'mp giveegg 172'] + fill
                   + ['mp phone pair', 'wait 60', 'mp save'], True)
    try:
        line = W.wait_line(game, '[test] phone: save')
        f = line.split()
        out = subprocess.run([W.CLI, '127.0.0.1', str(W.PORT), state, 'pair', f[f.index('save') + 1],
                              f[f.index('secret') + 1], 'HATCHER'], capture_output=True, text=True, timeout=30)
        if '"paired"' not in out.stdout:
            fail('pairing: ' + out.stdout + out.stderr)
        link = W.Link(state)
        snap = link.snap()
        if len(eggs_in(snap)) != 3:
            fail('three EGGS in the party to start (%s)' % [m.get('s') for m in snap['party']])
        lead = snap['party'][0]
        if link.order(op='incubate', at={'party': 0}, p=lead['p'], o=lead['o'])['why'] != 'notegg':
            fail('a POKeMON that isn\'t an EGG should be refused')
        ids = []
        for k in range(3):
            i, egg = eggs_in(snap)[0]
            d = link.order(op='incubate', at={'party': i}, p=egg['p'], o=egg['o'])
            if not d['ok'] or len(d.get('incubator', {}).get('eggs', [])) != k + 1:
                fail('incubate %d: %s' % (k, d))
            ids.append(d['incubator']['eggs'][-1]['id'])
            snap = link.snap()
        if eggs_in(snap) or len(snap.get('incubator', {}).get('eggs', [])) != 3:
            fail('the EGGS should be out of the save and in the snapshot\'s incubator')
        print('incubator: %d EGGS in, %d steps a cycle, max %d' % (len(ids), snap['incubator']['cycle'],
                                                                     snap['incubator']['max']))
        link.close()
    finally:
        game.kill()
        game.wait()

    game = W.start(work, [], False)
    try:
        W.wait_line(game, 'phone: listening')
        link = W.Link(state)
        snap = link.wait_free()
        inc = snap.get('incubator', {}).get('eggs', [])
        if [e['id'] for e in inc] != ids:
            fail('after a restart all three should still be in the incubator (%s)' % inc)
        cycles = inc[0]['mon']['f']
        d = link.order(op='hatchhome', box=1, eggs=[[ids[0], 400], [ids[1], 96]])
        if d['ok'] or d['why'] != 'full':
            fail('a box with one place for two EGGS should be refused (%s)' % d)
        d = link.order(op='hatchhome', box=2, eggs=[[ids[0], 400], [ids[1], 96]])
        if not d['ok']:
            fail('home: %s' % d)
        print('home: %s' % d['hatched'])
        h = {r[0]: r for r in d['hatched']}
        if h[ids[0]][2] != 1 or h[ids[1]][2] != 0 or h[ids[1]][3] != cycles - 3:
            fail('400 steps hatch a %d-cycle EGG, 96 take 3 cycles (%s)' % (cycles, d['hatched']))
        snap = link.snap()
        box = [m for m in snap['boxes'][2]['m'] if m]
        if len(box) != 2 or sorted(m.get('e', 0) for m in box) != [0, 1]:
            fail('box 3 should hold the hatched one and the EGG (%s)' % box)
        hatched = [m for m in box if not m.get('e')][0]
        if hatched['s'] != 172 or hatched['l'] != 5:
            fail('the hatched one: a Lv 5 PICHU (%s)' % hatched)
        if [e['id'] for e in snap['incubator']['eggs']] != [ids[2]]:
            fail('the third should stay in the incubator')
        d = link.order(op='hatchhome', box=2, eggs=[[ids[2], 200000]])
        if not d['ok'] or d['hatched'][0][4] > 10000 * 2:
            fail('200,000 steps on the day it went in count as at most a day or two of 10,000 (%s)' % d)
        if link.order(op='hatchhome', box=2, eggs=[[ids[2], 5]])['why'] != 'noegg':
            fail('the same EGG twice')
        before = len(everyone(link.snap()))
        snap = link.snap()
        egg = [m for m in snap['boxes'][2]['m'] if m and m.get('e')][0]
        slot = snap['boxes'][2]['m'].index(egg)
        if not link.order(op='incubate', at={'box': 2, 'slot': slot}, p=egg['p'], o=egg['o'])['ok']:
            fail('an EGG from a box into the incubator')
        link.close()
    finally:
        game.kill()
        game.wait()

    game = W.start(work, [], False)
    try:
        W.wait_line(game, 'phone: listening')
        link = W.Link(state)
        snap = link.wait_free()
        if len(everyone(snap)) != before - 1 or len(snap['incubator']['eggs']) != 1:
            fail('after a restart: everyone home once, one EGG in the incubator (%d / %d)'
                 % (len(everyone(snap)), before))
        link.close()
    finally:
        game.kill()
        game.wait()
    game = W.start(work, ['mp phone forget', 'wait 60', 'mp phone'], False)
    try:
        W.wait_line(game, 'incubator\'s EGGS came home as they left (0 still')
    finally:
        game.kill()
        game.wait()
    if os.path.exists(os.path.join(work, 'a.sav.incubator')):
        fail('the incubator file should be gone once it\'s empty')
    print('INCUBATOR TEST PASSED: in and out of the save, custody across restarts, a box with room, hatching by '
          'steps, the day\'s cap, FORGET')


if __name__ == '__main__':
    main()
