#!/usr/bin/env python3
"""The ME page on the phone, the game's side (src/mp/mp_phone_kai.c; docs/PHONE_LINK.md "The ME page"), against a
headless game through the phone link:
  - the snapshot's "kai": achievements done of all, MOEGATCHA pulls, tickets and free ones, the CANDY JAR, play time
  - {"t":"ach"}: every achievement in its category with its words, progress and reward
  - {"t":"gatcha"}: the banners and their free pulls
  - "mgift": MYSTERY GIFT, twelve a day, the same code twice refused
  - "pull": ten pulls (rigged: an SR among them) with the prizes in the answer and the pulls gone; with none left,
    refused ("nopulls")
  - "candy": with the jar, 25 FIRE candies -> 2 RARE CANDIES (5 left); fewer than 10 refused
  - "rename": a POKéMON renamed (its nickname in the snapshot); a name with a control code refused

    python3 tools/phone/kai_test.py      (after make linux; exit 0 = passed)
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import walk_test as W

RARE_CANDY, CANDY_JAR, FIRE = 68, 398, 10


def fail(msg):
    print('KAI TEST FAILED: ' + msg)
    if W.GAME[0] is not None:
        W.GAME[0].kill()
    sys.exit(1)


def bag_count(snap, item):
    return sum(c for pocket in snap['bag'] for i, c in pocket if i == item)


def main():
    subprocess.check_call(['make', '-s', '-C', os.path.join(W.ROOT, 'tools/phone')])
    work = tempfile.mkdtemp(prefix='kaitest')
    state = os.path.join(work, 'phone.state')
    W.PORT = 47600 + os.getpid() % 90
    game = W.start(work, ['mp gatcharig 3 10', 'mp additem %d 1' % CANDY_JAR, 'mp candy 10 25', 'mp phone pair',
                          'wait 60', 'mp save'], True)
    try:
        line = W.wait_line(game, '[test] phone: save')
        f = line.split()
        out = subprocess.run([W.CLI, '127.0.0.1', str(W.PORT), state, 'pair', f[f.index('save') + 1],
                              f[f.index('secret') + 1], 'KAI'], capture_output=True, text=True, timeout=30)
        if '"paired"' not in out.stdout:
            fail('pairing: ' + out.stdout + out.stderr)
        link = W.Link(state)
        link.wait_free()
        snap = link.snap()
        kai = snap.get('kai')
        if not kai or len(kai['ach']) != 2 or kai['ach'][1] < 10 or kai['pulls'] < 10 or len(kai['candy']) != 19:
            fail('the snapshot\'s "kai": %s' % kai)
        print('snapshot: achievements %d/%d, %d pulls, %d tickets, %d free, jar %d, %d FIRE candies' % (
            kai['ach'][0], kai['ach'][1], kai['pulls'], kai['tickets'], kai['free'], kai['jar'], kai['candy'][FIRE]))

        link.send({'t': 'ach'})
        ach = link.recv(['ach'])
        if len(ach['list']) != kai['ach'][1] or not ach['cats'] or sum(a[5] for a in ach['list']) != kai['ach'][0]:
            fail('the achievements: %d of %d, done %d' % (len(ach['list']), kai['ach'][1], sum(a[5] for a in ach['list'])))
        if any(not a[1] or not a[4].startswith('+') for a in ach['list']):
            fail('an achievement without a name or reward: %s' % next(a for a in ach['list'] if not a[1] or not a[4].startswith('+')))
        print('ach: %d in %d categories, e.g. %s' % (len(ach['list']), len(ach['cats']), ach['list'][0]))

        link.send({'t': 'gatcha'})
        g = link.recv(['gatcha'])
        if len(g['banners']) != 4 or g['banners'][0][0] != 'STANDARD':
            fail('the banners: %s' % g)
        print('gatcha: %s' % [b[0] for b in g['banners']])

        pulls = kai['pulls']
        d = link.order(op='pull', banner=0, how=4)
        if not d['ok'] or len(d.get('prizes', [])) != 10 or max(p['r'] for p in d['prizes']) < 3:
            fail('ten pulls: %s' % d)
        snap = link.snap()
        # (pulling can finish achievements, whose rewards are pulls)
        link.send({'t': 'ach'})
        after = link.recv(['ach'])
        earned = sum(int(a[4].split()[0][1:]) for a, b in zip(after['list'], ach['list']) if a[5] and not b[5])
        if snap['kai']['pulls'] != pulls - 10 + earned:
            fail('the pulls after: %d, not %d - 10 + %d' % (snap['kai']['pulls'], pulls, earned))
        print('pull: %s' % ', '.join('%s %s' % ('N U R SR SSR UR EX'.split()[p['r']], p['n']) for p in d['prizes']))
        for _ in range(60):
            if snap['kai']['pulls'] < 1:
                break
            link.order(op='pull', banner=0, how=4 if snap['kai']['pulls'] >= 10 else 3)
            snap = link.snap()
        d = link.order(op='pull', banner=0, how=3)
        if d['ok'] or d.get('why') != 'nopulls':
            fail('a pull with none left: %s' % d)

        print('pull: none left refused')

        if not snap['kai']['jar'] or snap['kai']['candy'][FIRE] != 25:
            fail('the jar and candies: %s' % snap['kai'])
        rare = bag_count(snap, RARE_CANDY)
        d = link.order(op='candy', type=FIRE)
        if not d['ok'] or d.get('made') != 2:
            fail('25 FIRE candies: %s' % d)
        snap = link.snap()
        if snap['kai']['candy'][FIRE] != 5 or bag_count(snap, RARE_CANDY) != rare + 2:
            fail('after: %d FIRE candies, %d RARE CANDY' % (snap['kai']['candy'][FIRE], bag_count(snap, RARE_CANDY)))
        d = link.order(op='candy', type=FIRE)
        if d['ok'] or d.get('why') != 'fewcandy':
            fail('5 candies: %s' % d)
        print('candy: 25 -> 2 RARE CANDY, 5 left; fewer than 10 refused')

        mon = snap['party'][0]
        d = link.order(op='rename', at={'party': 0}, p=mon['p'], o=mon['o'], name='C2C3C2C3')
        if not d['ok']:
            fail('a rename: %s' % d)
        snap = link.snap()
        if snap['party'][0]['n'] != 'C2C3C2C3':
            fail('the name after: %s' % snap['party'][0]['n'])
        d = link.order(op='rename', at={'party': 0}, p=mon['p'], o=mon['o'], name='C2FC')
        if d['ok'] or d.get('why') != 'badname':
            fail('a name with a control code: %s' % d)
        print('rename: kept; a control code refused')

        # MYSTERY GIFT: twelve scans a day, each code once a day, the gift in the bag and the answer
        if snap['kai'].get('gift') != 12:
            fail('gifts left at the start: %s' % snap['kai'].get('gift'))
        d = link.order(op='mgift', code='4006381333931')
        if not d['ok'] or d.get('left') != 11 or not d.get('item') or d.get('tier') not in range(5):
            fail('a gift: %s' % d)
        snap = link.snap()
        if bag_count(snap, d['item']) < d['qty'] or snap['kai']['gift'] != 11:
            fail('the gift after: %d of item %d, %s left' % (bag_count(snap, d['item']), d['item'], snap['kai']['gift']))
        again = link.order(op='mgift', code='4006381333931')
        if again['ok'] or again.get('why') != 'giftsame':
            fail('the same code again: %s' % again)
        tiers = [0] * 5
        tiers[d['tier']] += 1
        for k in range(11):
            g = link.order(op='mgift', code='https://example.com/%d' % k)
            if not g['ok']:
                fail('gift %d: %s' % (k + 2, g))
            tiers[g['tier']] += 1
        d = link.order(op='mgift', code='0123456789012')
        if d['ok'] or d.get('why') != 'giftdone':
            fail('a thirteenth gift: %s' % d)
        print('mystery gift: 12 a day (tiers %s), the same code refused, the 13th refused' % tiers)
    finally:
        game.kill()
    print('KAI TEST PASSED')


if __name__ == '__main__':
    main()
