#!/usr/bin/env python3
# Pack math: won packs to finish one set (18 C, 16 U, 11 holo + 5 plain rares) under the old rules,
# WotC's and the 1.32.0 ones (holo 1 in 3, a new holo after 20 dry packs, CARD POINTS as tcg_collection.c).
# usage: python3 tools/tcg/pack_math.py
import random, sys
# One set like COLOSSEUM: 18 C, 16 U, 16 R (11 holo in real life, 5 plain).
C, U, H, R = 18, 16, 11, 5
KEEP = 4
CARDS = [('c',i) for i in range(C)] + [('u',i) for i in range(U)] + [('h',i) for i in range(H)] + [('r',i) for i in range(R)]
def run(m, seed):
    rnd = random.Random(seed)
    own = {}; st = dict(cp=0, drought=0, earned=0, bought=0)
    def open_pack():
        pulls = [('c', rnd.randrange(C)) for _ in range(m['nc'])] + [('u', rnd.randrange(U)) for _ in range(m['nu'])]
        if m['holo'] is None:
            k = rnd.randrange(H + R); pulls.append(('h', k) if k < H else ('r', k - H))
        else:
            miss = [i for i in range(H) if not own.get(('h', i))]
            st['drought'] += 1
            if m.get('pity') and miss and st['drought'] >= m['pity']:
                pulls.append(('h', rnd.choice(miss)))
            elif rnd.random() < m['holo']:
                pulls.append(('h', rnd.randrange(H)))
            else:
                pulls.append(('r', rnd.randrange(R)))
        for k in pulls:
            if k[0] == 'h' and not own.get(k): st['drought'] = 0
            own[k] = own.get(k, 0) + 1
    def settle():
        if not m['cp']: return
        for k in list(own):
            if own[k] > KEEP: st['cp'] += (own[k] - KEEP) * m['val'][k[0]]; own[k] = KEEP
        for k in sorted((k for k in CARDS if not own.get(k)), key=lambda k: m['cost'][k[0]] or 1e9):
            if m['cost'][k[0]] and st['cp'] >= m['cost'][k[0]]: st['cp'] -= m['cost'][k[0]]; own[k] = 1
    def missing_rares(): return [k for k in CARDS if k[0] in 'hr' and not own.get(k)]
    def done(): return all(own.get(k) for k in CARDS)
    last_at = None
    while not done():
        st['earned'] += 1; open_pack(); settle()
        while m['cp'] and m['packcost'] and st['cp'] >= m['packcost'] and not done():
            st['cp'] -= m['packcost']; st['bought'] += 1; open_pack(); settle()
        if last_at is None and len(missing_rares()) <= 1: last_at = st['earned']
    return st['earned'], st['bought'], st['earned'] - (last_at or st['earned'])
def report(name, m, n=2000):
    r = [run(m, s) for s in range(n)]
    e = sorted(x[0] for x in r); b = sorted(x[1] for x in r); l = sorted(x[2] for x in r)
    print("%-40s won packs: median %4d 90%% %4d (+%3d bought) | last rare: median %3d 90%% %3d" %
          (name, e[n//2], e[n*9//10], b[n//2], l[n//2], l[n*9//10]))
now = dict(nc=5, nu=3, holo=None, cp=True, val={'c':1,'u':3,'h':10,'r':10}, cost={'c':5,'u':15,'h':50,'r':50}, packcost=12)
report("NOW", now)
report("NOW, no CARD POINTS", dict(now, cp=False))
report("WotC (holo 1/3, no CP)", dict(nc=5, nu=3, holo=1/3, cp=False, packcost=0))
for pity, val, cost, pc in [
        (0,  {'c':1,'u':2,'h':20,'r':6}, {'c':5,'u':15,'h':0,'r':80}, 40),
        (20, {'c':1,'u':2,'h':20,'r':6}, {'c':5,'u':15,'h':0,'r':80}, 40),
        (20, {'c':1,'u':2,'h':15,'r':5}, {'c':5,'u':15,'h':0,'r':80}, 60),
        (25, {'c':1,'u':2,'h':15,'r':5}, {'c':5,'u':15,'h':0,'r':80}, 60),
        (25, {'c':0,'u':1,'h':10,'r':4}, {'c':5,'u':15,'h':0,'r':80}, 60),
        (30, {'c':0,'u':1,'h':10,'r':4}, {'c':5,'u':15,'h':0,'r':80}, 60)]:
    report("pity %2d holo val %2d pack %2d C%d U%d" % (pity, val['h'], pc, val['c'], val['u']),
           dict(nc=5, nu=3, holo=1/3, pity=pity, cp=True, val=val, cost=cost, packcost=pc))
