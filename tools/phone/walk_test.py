#!/usr/bin/env python3
"""The phone's walk and training against a headless game (docs/PHONE_LINK.md "Phase 3"): the roadmap's Phase 3 gate,
"a week of real walks: steps, caps and homecomings right", run on the game's own rules.

  - a box POKeMON goes for a walk: it leaves the save, the game saves, the snapshot shows the walk; the game started
    again still has it out (custody survives a restart)
  - it comes home after a "week" (the walk's start moved back 7 days in "<save>.walk"): steps capped at 10,000 a day,
    finds only from the place's pool and no more than the steps earn, catches only from the place's wild table at its
    levels and 3 a day, EXP to the badge cap; it's back, exactly once, with what it earned; a second homecoming is refused
  - training: STAT in a box, an hour later (moved back in "<save>.train") 12 EVs, taken into the party it ends
  - a pet and a BERRY from the phone; FORGET PHONES brings a walking POKeMON home as it left

    python3 tools/phone/walk_test.py      (after make linux; exit 0 = passed)
"""
import json
import os
import struct
import subprocess
import sys
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PORT = 47900 + os.getpid() % 90
CLI = os.path.join(ROOT, 'tools/phone/phonecli')


GAME = [None]


def fail(msg):
    print('WALK TEST FAILED: ' + msg)
    g = GAME[0]
    if g is not None and os.environ.get('VERBOSE'):
        import select
        g.kill()
        tail = []
        while select.select([g.stdout], [], [], 1)[0]:
            l = g.stdout.readline()
            if not l:
                break
            tail = (tail + [l.rstrip()])[-40:]
        print('\n'.join(tail))
    sys.exit(1)


def start(work, lines, fresh):
    script = os.path.join(work, 'run%d.txt' % time.time_ns())
    open(script, 'w').write('\n'.join(['pressuntil A field 5000', 'wait 30'] + lines + ['wait 2000000', 'quit']) + '\n')
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_TEST=script, MOEMON_SHOTDIR=work,
               MOEMON_SKIP_COOP_SETUP='1', MOEMON_COOP_CONFIG=os.path.join(work, 'a.ini'),
               MOEMON_CONTROLS_CONFIG=os.path.join(work, 'c.ini'), MOEMON_SAVE=os.path.join(work, 'a.sav'),
               MOEMON_PHONE_PORT=str(PORT), MOEMON_NO_UPDATE='1')
    if fresh:
        env.update(MOEMON_DEBUG_QUICKSTART='1', MOEMON_DEBUG_PARTY='25:20,1:20,4:20', MOEMON_DEBUG_WARP='0,16,10,10')
    GAME[0] = subprocess.Popen([os.path.join(ROOT, 'pokeemerald64')], cwd=ROOT, env=env, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, text=True, bufsize=1)
    return GAME[0]


def wait_line(proc, needle, timeout=120):
    end = time.time() + timeout
    while time.time() < end:
        line = proc.stdout.readline()
        if not line:
            fail('the game ended before "%s"' % needle)
        if needle in line:
            return line
    fail('no "%s" in time' % needle)


class Link:
    def __init__(self, state):
        self.p = subprocess.Popen([CLI, '127.0.0.1', str(PORT), state, 'hello'], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        if json.loads(self.p.stdout.readline()).get('t') != 'welcome':
            fail('no welcome')
        self.id = 0

    def send(self, obj):
        self.p.stdin.write(json.dumps(obj, separators=(',', ':')) + '\n')
        self.p.stdin.flush()

    def recv(self, want):
        import select
        end = time.time() + 60
        while True:
            if not select.select([self.p.stdout], [], [], max(0, end - time.time()))[0]:
                fail('no %s in time' % want)
            line = self.p.stdout.readline()
            if not line:
                fail('the link closed')
            m = json.loads(line)
            if os.environ.get('VERBOSE'):
                print('<<', line[:200].strip(), flush=True)
            if m.get('t') in want:
                return m

    def snap(self):
        self.send({'t': 'snap'})
        return self.recv(['snap'])

    def wait_free(self):
        # (the game on the field, free: a restarted game links on its title screen)
        end = time.time() + 90
        while time.time() < end:
            snap = self.snap()
            if snap['place'].get('safe') == 1:
                return snap
            time.sleep(0.5)
        fail('the game never got free')

    def order(self, **o):
        self.id += 1
        o.update(t='order', id=self.id)
        self.send(o)
        return self.recv(['done'])

    def close(self):
        self.p.kill()


def mons(snap):
    out = [(m['p'], m['o']) for m in snap['party']]
    for b in snap['boxes']:
        out += [(m['p'], m['o']) for m in b['m'] if m]
    return out


def main():
    subprocess.check_call(['make', '-s', '-C', os.path.join(ROOT, 'tools/phone')])
    work = tempfile.mkdtemp(prefix='walktest')
    state = os.path.join(work, 'phone.state')
    game = start(work, ['mp boxput 1 0 25 5', 'mp boxput 1 1 1 30', 'mp boxput 1 2 4 10', 'mp phone pair', 'wait 60',
                        'mp save'], True)
    try:
        line = wait_line(game, '[test] phone: save')
        f = line.split()
        out = subprocess.run([CLI, '127.0.0.1', str(PORT), state, 'pair', f[f.index('save') + 1],
                              f[f.index('secret') + 1], 'WALKER'], capture_output=True, text=True, timeout=30)
        if '"paired"' not in out.stdout:
            fail('pairing: ' + out.stdout + out.stderr)
        link = Link(state)
        snap = link.snap()
        walker = snap['boxes'][0]['m'][0]
        before = mons(snap)
        d = link.order(op='walk', at={'box': 0, 'slot': 0}, p=walker['p'], o=walker['o'])
        if not d['ok'] or 'walk' not in d:
            fail('the walk: %s' % d)
        walk = d['walk']
        print('walk: %s went, %d wild, %d finds, cap %d' % (walker['s'], len(walk['wild']), len(walk['finds']), walk['cap']))
        snap = link.snap()
        if (walker['p'], walker['o']) in mons(snap) or snap.get('walk', {}).get('id') != walk['id']:
            fail('the walker should be out of the save and in the snapshot\'s walk')
        # a second walk at once: refused (one at a time)
        other = snap['boxes'][0]['m'][1]
        if link.order(op='walk', at={'box': 0, 'slot': 1}, p=other['p'], o=other['o'])['why'] != 'walking':
            fail('a second walker')
        link.close()
    finally:
        game.kill()
        game.wait()

    # the game started again: still out; then a week later it comes home
    wpath = os.path.join(work, 'a.sav.walk')
    raw = bytearray(open(wpath, 'rb').read())
    sent = struct.unpack_from('<I', raw, 12)[0]
    struct.pack_into('<I', raw, 12, sent - 7 * 86400)
    open(wpath, 'wb').write(raw)
    game = start(work, [], False)
    try:
        wait_line(game, 'phone: listening')
        link = Link(state)
        snap = link.wait_free()
        if snap.get('walk', {}).get('id') != walk['id'] or (walker['p'], walker['o']) in mons(snap):
            fail('after a restart the walk should still be out')
        wild = snap['walk']['wild']
        finds = snap['walk']['finds']
        if not wild:
            fail('no wild POKeMON where it started')
        sp, lo, hi = wild[0]
        caught = [[sp, lo, 4]] * 25 + [[9999, 5, 4], [sp, hi + 50, 4]]
        d = link.order(op='home', walk=walk['id'], steps=200000, finds=[finds[0]] * 40 + [9999], caught=caught)
        if not d['ok']:
            fail('home: %s' % d)
        home = d['home']
        print('home: %s' % home)
        import datetime
        days = (datetime.date.today() - datetime.date.fromtimestamp(sent - 7 * 86400)).days + 1
        if home['steps'] != days * 10000:
            fail('steps should be capped at %d days x 10,000 (%d)' % (days, home['steps']))
        if len(home['finds']) != days * 10000 // 2000 or any(i != finds[0] for i in home['finds']):
            fail('finds: %d steps earn %d from the pool (%s)' % (days * 10000, days * 5, home['finds']))
        if len(home['caught']) != min(days * 3, days * 10000 // 3000, 25):
            fail('catches: 3 a day for %d days (%d)' % (days, len(home['caught'])))
        snap = link.snap()
        after = mons(snap)
        if after.count((walker['p'], walker['o'])) != 1 or 'walk' in snap:
            fail('the walker should be back exactly once')
        if len(after) != len(before) + len(home['caught']):
            fail('everyone else, plus the catches (%d vs %d)' % (len(after), len(before)))
        back = [m for m in snap['party'] + [x for b in snap['boxes'] for x in b['m'] if x] if m['p'] == walker['p']][0]
        if back['l'] > walk['cap'] or back['l'] <= walker['l']:
            fail('EXP to the badge cap: level %d -> %d (cap %d)' % (walker['l'], back['l'], walk['cap']))
        if link.order(op='home', walk=walk['id'], steps=10)['why'] != 'nowalk':
            fail('a second homecoming')
        # training: STAT in a box; an hour later 12 EVs
        trainee = snap['boxes'][0]['m'][2]
        d = link.order(op='train', at={'box': 0, 'slot': 2}, p=trainee['p'], o=trainee['o'], k=0, s=1)
        if not d['ok']:
            fail('train: %s' % d)
        link.close()
    finally:
        game.kill()
        game.wait()

    tpath = os.path.join(work, 'a.sav.train')
    raw = bytearray(open(tpath, 'rb').read())
    last = struct.unpack_from('<I', raw, 8 + 16)[0]
    struct.pack_into('<I', raw, 8 + 16, last - 3600)
    open(tpath, 'wb').write(raw)
    game = start(work, ['mp additem 139 2'], False)
    try:
        wait_line(game, 'phone: listening')
        link = Link(state)
        snap = link.wait_free()
        if len(snap['train']['list']) != 1:
            fail('the training should still be on')
        d = link.order(op='collect')
        gains = d.get('gains')
        if not gains or gains[0][3] != 12:
            fail('an hour of STAT training is 12 EVs (%s)' % gains)
        # taken into the party, it ends
        snap = link.snap()
        d = link.order(op='move', **{'from': {'box': 0, 'slot': 2}, 'to': {'party': len(snap['party'])}, 'p': trainee['p'],
                                     'o': trainee['o'], 'tp': 0, 'to2': 0})
        if not d['ok']:
            fail('move to the party: %s' % d)
        time.sleep(3)
        if link.snap()['train']['list']:
            fail('in the party its training should end')
        # a pet, then an ORAN BERRY
        d = link.order(op='kind', at={'party': 0}, p=snap['party'][0]['p'], o=snap['party'][0]['o'], verb=0)
        if not d['ok'] or not d.get('line'):
            fail('a pet: %s' % d)
        d = link.order(op='kind', at={'party': 0}, p=snap['party'][0]['p'], o=snap['party'][0]['o'], verb=1, item=139)
        if not d['ok']:
            fail('a BERRY: %s' % d)
        # FORGET PHONES: a walking POKeMON comes home as it left
        snap = link.snap()
        w2 = snap['boxes'][0]['m'][1]
        if not link.order(op='walk', at={'box': 0, 'slot': 1}, p=w2['p'], o=w2['o'])['ok']:
            fail('the second walk')
        link.close()
        game.stdin = None
    finally:
        game.kill()
        game.wait()
    game = start(work, ['mp phone forget', 'wait 60', 'mp phone'], False)
    try:
        wait_line(game, 'came home as it left')
    finally:
        game.kill()
        game.wait()
    if os.path.exists(wpath):
        fail('the walk file should be gone once it came home')
    print('WALK TEST PASSED: custody across a restart, a week\'s caps, homecoming once, training, kindness, FORGET')


if __name__ == '__main__':
    main()
