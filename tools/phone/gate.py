#!/usr/bin/env python3
"""The phone link's Phase 1 gate (docs/COMPANION_APP.md "Phased roadmap"): 1,000 scripted edits, no POKeMON lost or
copied. A headless game (MOEMON_PHONE_PORT) with three boxes filled pairs with tools/phone/phonecli, then gets random
orders: box <-> box, party <-> PC, party <-> party, give and take items, and stale ones (a POKeMON that moved since,
a place that changed) that must be refused. Every answer is checked against the client's own model, a full snapshot
is compared every 100 edits, and at the end every POKeMON (personality, Trainer ID, species) is there exactly once.

    python3 tools/phone/gate.py [EDITS] [SEED]      (run from the repository after make linux; exit 0 = passed)
"""
import json
import os
import random
import subprocess
import sys
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EDITS = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
SEED = int(sys.argv[2]) if len(sys.argv) > 2 else 1
PORT = 47700 + os.getpid() % 200
ITEMS = [139, 142, 183, 200, 215]   # ORAN BERRY, SITRUS BERRY, QUICK CLAW, LEFTOVERS, CHARCOAL


def fail(msg):
    print('GATE FAILED: ' + msg)
    sys.exit(1)


def start_game(work):
    script = os.path.join(work, 'a.txt')
    lines = ['pressuntil A field 5000', 'wait 30']
    species = [25, 1, 4, 7, 16, 19, 41, 52, 54, 63, 66, 74, 92, 129, 133, 147]
    for box in (1, 2, 5):
        for pos in range(0, 30, 2 if box == 5 else 1):
            lines.append('mp boxput %d %d %d %d' % (box, pos, species[(box * 7 + pos) % len(species)], 5 + pos))
    for item in ITEMS:
        lines.append('mp additem %d 20' % item)
    lines += ['mp phone pair', 'wait 2000000', 'quit']
    open(script, 'w').write('\n'.join(lines) + '\n')
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_TEST=script, MOEMON_SHOTDIR=work,
               MOEMON_DEBUG_QUICKSTART='1', MOEMON_SKIP_COOP_SETUP='1', MOEMON_COOP_CONFIG=os.path.join(work, 'a.ini'),
               MOEMON_CONTROLS_CONFIG=os.path.join(work, 'a_controls.ini'), MOEMON_SAVE=os.path.join(work, 'a.sav'),
               MOEMON_PHONE_PORT=str(PORT), MOEMON_DEBUG_PARTY='25:20,1:20,4:20')
    return subprocess.Popen([os.environ.get('BIN', os.path.join(ROOT, 'pokeemerald64'))], cwd=ROOT, env=env, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, bufsize=1)


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
    def __init__(self, cli, state):
        self.p = subprocess.Popen([cli, '127.0.0.1', str(PORT), state, 'hello'], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        welcome = json.loads(self.p.stdout.readline())
        if welcome.get('t') != 'welcome':
            fail('no welcome: %s' % welcome)

    def send(self, obj):
        self.p.stdin.write(json.dumps(obj, separators=(',', ':')) + '\n')
        self.p.stdin.flush()

    def recv(self, want):
        while True:
            line = self.p.stdout.readline()
            if not line:
                fail('the link closed')
            msg = json.loads(line)
            if msg.get('t') in want:
                return msg


def ident(mon):
    return (mon['p'], mon['o'], mon['s']) if mon else None


class Model:
    """The client's picture of the party and boxes, kept from the snapshot and the answers' changed places"""
    def __init__(self, snap):
        self.party = list(snap['party'])
        self.boxes = [list(b['m']) for b in snap['boxes']]

    def at(self, loc):
        if 'party' in loc:
            return self.party[loc['party']] if loc['party'] < len(self.party) else 0
        return self.boxes[loc['box']][loc['slot']]

    def apply(self, changed):
        for c in changed:
            if 'party' in c:
                self.party = list(c['party'])
            else:
                self.boxes[c['box']][c['slot']] = c['mon']

    def all(self):
        out = [ident(m) for m in self.party if m]
        for b in self.boxes:
            out += [ident(m) for m in b if m]
        return sorted(out)


def main():
    random.seed(SEED)
    work = tempfile.mkdtemp(prefix='phonegate')
    subprocess.check_call(['make', '-s', '-C', os.path.join(ROOT, 'tools/phone')])
    cli = os.path.join(ROOT, 'tools/phone/phonecli')
    game = start_game(work)
    try:
        line = wait_line(game, '[test] phone: save')
        f = line.split()
        save, secret = f[f.index('save') + 1], f[f.index('secret') + 1]
        state = os.path.join(work, 'phone.state')
        out = subprocess.run([cli, '127.0.0.1', str(PORT), state, 'pair', save, secret, 'GATE'], capture_output=True,
                             text=True, timeout=30)
        if out.returncode != 0 or '"paired"' not in out.stdout:
            fail('pairing: %s %s' % (out.stdout, out.stderr))
        link = Link(cli, state)
        link.send({'t': 'snap'})
        snap = link.recv(['snap'])
        model = Model(snap)
        start = model.all()
        if len(start) != len(set(start)):
            fail('the same POKeMON twice before any edit')
        print('gate: %d POKeMON, %d party, place %s' % (len(start), len(model.party), snap['place']))
        if not snap['place']['pc']:
            fail('the test map should allow the party <-> PC')
        done = refused = expected_refusals = 0
        for n in range(1, EDITS + 1):
            places = [{'box': b, 'slot': s} for b in (0, 1, 4, 7) for s in range(30)]
            filled = [l for l in places if model.at(l)] + [{'party': i} for i in range(len(model.party))]
            kind = random.random()
            stale = random.random() < 0.1
            order = {'t': 'order', 'id': n}
            if kind < 0.75:
                frm = random.choice(filled)
                if random.random() < 0.3:
                    to = {'party': random.randrange(6)}
                else:
                    to = random.choice(places)
                mon, target = model.at(frm), model.at(to)
                order.update(op='move', **{'from': frm, 'to': to}, p=mon['p'], o=mon['o'])
                if target:
                    order.update(tp=target['p'], to2=target['o'])
                if stale:
                    order['p'] ^= 0x5A5A
                    expected_refusals += 1
            else:
                at = random.choice(filled)
                mon = model.at(at)
                give = random.random() < 0.6
                order.update(op='give' if give else 'take', at=at, p=mon['p'], o=mon['o'])
                if give:
                    order['item'] = random.choice(ITEMS)
                if stale:
                    order['o'] ^= 1
                    expected_refusals += 1
            link.send(order)
            ans = link.recv(['done'])
            while ans.get('id') != n:
                ans = link.recv(['done'])
            if ans['ok']:
                if stale:
                    fail('edit %d: a stale order was carried out: %s' % (n, order))
                done += 1
                model.apply(ans['changed'])
            else:
                refused += 1
                if stale and ans['why'] not in ('gone', 'changed', 'same'):
                    fail('edit %d: a stale order refused for "%s"' % (n, ans['why']))
            if n % 100 == 0 or n == EDITS:
                link.send({'t': 'snap'})
                snap = link.recv(['snap'])
                full = Model(snap)
                if full.party != model.party or full.boxes != model.boxes:
                    fail('edit %d: the snapshot and the answers disagree' % n)
                now = full.all()
                if now != start:
                    lost = set(start) - set(now)
                    extra = [x for x in now if now.count(x) > 1] or list(set(now) - set(start))
                    fail('edit %d: POKeMON lost %s, copied or new %s' % (n, sorted(lost)[:5], extra[:5]))
                print('gate: %d edits, %d done, %d refused (%d stale): every POKeMON there once' % (n, done, refused,
                                                                                                    expected_refusals))
        link.send({'t': 'ping'})
        st = link.recv(['state'])
        print('gate: the game carried out %d orders' % st['applied'])
        if st['applied'] != done:
            fail('the game counts %d orders done, the client %d' % (st['applied'], done))
        print('GATE PASSED: %d edits, no POKeMON lost or copied' % EDITS)
    finally:
        game.kill()


if __name__ == '__main__':
    main()
