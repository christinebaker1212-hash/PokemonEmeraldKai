#!/usr/bin/env python3
"""The card games on the phone, the game's side (src/mp/mp_phone_cards.c; docs/PHONE_LINK.md "TRIPLE TRIAD on the
phone"), against a headless game through the phone link:
  - the snapshot's "tt": the collection, the decks, the packs, today's prizes
  - "ttdeck": a deck saved and put in use; one with a card the save doesn't own, or two of a card, refused
  - "ttwin": three duel prizes a day (the fourth refused), one puzzle prize a day, each a pack in the game
  - "ttopen": a pack opened by the game's own pack math: its cards in the answer and in the collection

    python3 tools/phone/cards_test.py      (after make linux; exit 0 = passed)
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import walk_test as W


def fail(msg):
    print('CARDS TEST FAILED: ' + msg)
    if W.GAME[0] is not None:
        W.GAME[0].kill()
    sys.exit(1)


def owned_total(tt):
    return sum(c for _, c in tt['owned'])


def main():
    subprocess.check_call(['make', '-s', '-C', os.path.join(W.ROOT, 'tools/phone')])
    work = tempfile.mkdtemp(prefix='cardstest')
    state = os.path.join(work, 'phone.state')
    W.PORT = 47800 + os.getpid() % 90
    game = W.start(work, ['mp tt starter', 'mp phone pair', 'wait 60', 'mp save'], True)
    try:
        line = W.wait_line(game, '[test] phone: save')
        f = line.split()
        out = subprocess.run([W.CLI, '127.0.0.1', str(W.PORT), state, 'pair', f[f.index('save') + 1],
                              f[f.index('secret') + 1], 'CARDS'], capture_output=True, text=True, timeout=30)
        if '"paired"' not in out.stdout:
            fail('pairing: ' + out.stdout + out.stderr)
        link = W.Link(state)
        link.wait_free()
        tt = link.snap().get('tt')
        if not tt or tt['on'] != 1 or 0 in tt['decks'][0]['c']:
            fail('no collection in the snapshot: %s' % tt)
        print('snapshot: %d cards, deck "%s" %s, region %d, today %s' % (owned_total(tt), tt['decks'][0]['n'],
                                                                       tt['decks'][0]['c'], tt['region'], tt['today']))
        deck = tt['decks'][0]['c']
        d = link.order(op='ttdeck', deck=1, cards=list(reversed(deck)), name='phone deck', active=1)
        if not d['ok']:
            fail('a deck: %s' % d)
        tt = link.snap()['tt']
        if tt['decks'][1]['c'] != list(reversed(deck)) or tt['decks'][1]['n'] != 'PHONE DECK' or tt['active'] != 1:
            fail('the deck as saved: %s, active %d' % (tt['decks'][1], tt['active']))
        unowned = next(i for i in range(1, 400) if i not in [c for c, _ in tt['owned']])
        d = link.order(op='ttdeck', deck=2, cards=[deck[0], deck[1], deck[2], deck[3], unowned])
        if d['ok'] or d.get('why') != 'notowned':
            fail('a deck with a card not owned: %s' % d)
        d = link.order(op='ttdeck', deck=2, cards=[deck[0], deck[0], deck[2], deck[3], deck[4]])
        if d['ok'] or d.get('why') != 'deckrules':
            fail('a deck with two of a card: %s' % d)
        print('decks: saved and in use; a card not owned and a double refused')
        packs0 = list(tt['packs'])
        for i in range(3):
            d = link.order(op='ttwin', kind='duel', harder=1 if i == 2 else 0)
            if not d['ok'] or 'prize' not in d:
                fail('a duel prize: %s' % d)
        d = link.order(op='ttwin', kind='duel')
        if d['ok'] or d.get('why') != 'today':
            fail('a fourth duel prize today: %s' % d)
        d = link.order(op='ttwin', kind='puzzle')
        if not d['ok'] or d.get('prize') != 1:
            fail('the puzzle prize: %s' % d)
        if link.order(op='ttwin', kind='puzzle').get('why') != 'today':
            fail('a second puzzle prize')
        tt = link.snap()['tt']
        if tt['packs'][0] != packs0[0] + 2 or tt['packs'][1] != packs0[1] + 2 or tt['today']['packs'] != 3:
            fail('the packs: %s -> %s, today %s' % (packs0, tt['packs'], tt['today']))
        print('prizes: 3 duel packs and 1 puzzle pack today, more refused')
        before = owned_total(tt)
        d = link.order(op='ttopen', pack=0)
        if not d['ok'] or len(d.get('cards', [])) != 3:
            fail('opening a pack: %s' % d)
        tt = link.snap()['tt']
        if owned_total(tt) != before + 3 or tt['packs'][0] != packs0[0] + 1:
            fail('after the pack: %d cards (was %d), packs %s' % (owned_total(tt), before, tt['packs']))
        print('pack: %s' % d['cards'])
        link.close()
        print('CARDS TEST PASSED: the collection, decks, today\'s prizes and packs through the phone link')
    finally:
        game.kill()


if __name__ == '__main__':
    main()
