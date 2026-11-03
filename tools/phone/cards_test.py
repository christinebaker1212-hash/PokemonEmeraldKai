#!/usr/bin/env python3
"""The card games on the phone, the game's side (src/mp/mp_phone_cards.c; docs/PHONE_LINK.md "TRIPLE TRIAD on the
phone"), against a headless game through the phone link:
  - the snapshot's "tt": the collection, the decks, the packs, today's prizes
  - "ttdeck": a deck saved and put in use; one with a card the save doesn't own, or two of a card, refused
  - "ttwin": three duel prizes a day (the fourth refused), one puzzle prize a day, each a pack in the game
  - "ttopen": a pack opened by the game's own pack math: its cards in the answer and in the collection
  - the snapshot's "tcg" (the POKéMON card game): the starter deck, the collection, today's prizes
  - "tcgdeck": a deck saved, renamed and put in use; a card not owned, or five of a name, refused; an empty slot can't
    be put in use, the deck in use can't be emptied
  - "tcgduel": a loss (no prize) and three wins a day each a booster (the fourth win none), CARD SCALING's skill moving
  - "tcgopen": a booster opened by the game's pack math: ten cards in the answer and in the collection

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
    game = W.start(work, ['mp tt starter', 'mp cards starter 0', 'mp phone pair', 'wait 60', 'mp save'], True)
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

        # the POKéMON card game
        tcg = link.snap().get('tcg')
        if not tcg or tcg['on'] != 1 or 0 in tcg['decks'][0]['c'] or len(tcg['packs']) != 11:
            fail('no POKéMON card collection in the snapshot: %s' % (tcg and {k: tcg[k] for k in tcg if k != 'owned'}))
        starter = tcg['decks'][0]['c']
        print('tcg snapshot: %d cards, deck "%s" (ace %d), skill %d, today %s' % (owned_total(tcg), tcg['decks'][0]['n'],
                                                                         tcg['decks'][0]['ace'], tcg['skill'], tcg['today']))
        d = link.order(op='tcgdeck', deck=2, active=1)
        if d['ok'] or d.get('why') != 'empty':
            fail('an empty deck slot put in use: %s' % d)
        swapped = list(reversed(starter))
        d = link.order(op='tcgdeck', deck=2, cards=swapped, name='phone deck', active=1)
        if not d['ok']:
            fail('a POKéMON deck: %s' % d)
        tcg = link.snap()['tcg']
        if tcg['decks'][2]['c'] != swapped or tcg['decks'][2]['n'] != 'PHONE DECK' or tcg['active'] != 2:
            fail('the deck as saved: %s, active %d' % (tcg['decks'][2]['n'], tcg['active']))
        have = {c: n for c, n in tcg['owned']}
        unowned = next(i for i in range(8, 228) if have.get(i, 0) == 0)
        d = link.order(op='tcgdeck', deck=3, cards=starter[:59] + [unowned])
        if d['ok'] or d.get('why') != 'notowned':
            fail('a POKéMON deck with a card not owned: %s' % d)
        five = next(i for i in starter if i > 7)
        d = link.order(op='tcgdeck', deck=3, cards=[five] * 5 + starter[5:])
        if d['ok'] or d.get('why') != 'deckrules':
            fail('a POKéMON deck with five of a card: %s' % d)
        d = link.order(op='tcgdeck', deck=2, cards=[])
        if d['ok'] or d.get('why') != 'activedeck':
            fail('emptying the deck in use: %s' % d)
        if not link.order(op='tcgdeck', deck=0, active=1)['ok'] or not link.order(op='tcgdeck', deck=2, cards=[])['ok']:
            fail('back to the starter, the phone deck emptied')
        if 0 not in link.snap()['tcg']['decks'][2]['c']:
            fail('the emptied deck')
        print('tcg decks: saved, renamed, in use; not owned / five of a name / empty / in use refused')
        packs0, skill0 = list(tcg['packs']), tcg['skill']
        d = link.order(op='tcgduel', won=0, deck=1350, opp=1400, level=1)
        if not d['ok'] or d.get('prize') != -1:
            fail('a lost duel: %s' % d)
        for i in range(3):
            d = link.order(op='tcgduel', won=1, deck=1350, opp=1400, level=1)
            if not d['ok'] or not 0 <= d.get('prize', -1) < 11 or d.get('left') != 2 - i:
                fail('a won duel: %s' % d)
        d = link.order(op='tcgduel', won=1, deck=1350, opp=1400, level=1)
        if not d['ok'] or d.get('prize') != -1 or d.get('left') != 0:
            fail('a fourth win today: %s' % d)
        tcg = link.snap()['tcg']
        if sum(tcg['packs']) != sum(packs0) + 3 or tcg['today']['packs'] != 3 or tcg['skill'] == skill0:
            fail('the boosters: %s -> %s, today %s, skill %d -> %d' % (packs0, tcg['packs'], tcg['today'], skill0,
                                                                     tcg['skill']))
        print('tcg prizes: a loss none, 3 wins a booster each, the 4th none; skill %d -> %d' % (skill0, tcg['skill']))
        pack = next(i for i, n in enumerate(tcg['packs']) if n)
        before, unopened = owned_total(tcg), tcg['packs'][pack]
        d = link.order(op='tcgopen', pack=pack)
        if not d['ok'] or len(d.get('cards', [])) != 10:
            fail('opening a booster: %s' % d)
        tcg = link.snap()['tcg']
        if owned_total(tcg) != before + 10 or tcg['packs'][pack] != unopened - 1:
            fail('after the booster: %d cards (was %d), %d left' % (owned_total(tcg), before, tcg['packs'][pack]))
        empty = next((i for i, n in enumerate(tcg['packs']) if n == 0), None)
        if empty is not None and link.order(op='tcgopen', pack=empty).get('why') != 'nopack':
            fail('a booster that isn\'t there')
        print('tcg booster: %s' % d['cards'])
        link.close()
        print('CARDS TEST PASSED: both games\' collections, decks, today\'s prizes and packs through the phone link')
    finally:
        game.kill()


if __name__ == '__main__':
    main()
