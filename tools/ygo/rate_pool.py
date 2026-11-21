#!/usr/bin/env python3
"""The generic duelists' decks rated (the user, 2026-11-21: "the cream rises based on the rules"; docs/YGO_DECISIONS.md
"NPC decks from YGOPRODeck"). From ~/.cache/moemon/npc_pool.json (tools/ygo/build_npc_pool.py):

  1. a prior for each deck: its cards' power from their text (tools/ygo/card_power.json: the Main Deck's average,
     its best cards counting more, its Extra Deck a little), and a tournament placing as an anchor (a win in a big
     event most: YGOPRODeck's tournament decks);
  2. Swiss rounds of AI-against-AI duels on EDOPro's engine (tools/ygo/rate_pool_worker.c, a worker a core): each
     round the decks in rating order, each paired with a near one it hasn't met, a duel each, the first player
     alternating; a running Elo between rounds steers the pairings;
  3. at the end a Bradley-Terry fit of every result at once, held toward each deck's prior (so a deck with few duels
     stays near what its cards say), as Elo with the average deck at 1500.

Every result goes to ~/.cache/moemon/npc_pool_results.txt as it comes (a rerun picks up from there: the rounds already
played are not played again). Writes ~/.cache/moemon/npc_pool_rated.json (the pool with "rating", "prior", "games",
"wins") and docs/audit_ygo_npc_pool.txt.

  tools/ygo/rate_pool.py [ROUNDS] [JOBS]
"""
import json, math, os, queue, random, select, subprocess, sys, threading, time
import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CACHE = os.path.expanduser('~/.cache/moemon')
POOL = os.path.join(CACHE, 'npc_pool.json')
RESULTS = os.path.join(CACHE, 'npc_pool_results.txt')
OUT = os.path.join(CACHE, 'npc_pool_rated.json')
TMP = os.environ.get('OUT', '/tmp/ygo-tests')
DUEL_TIMEOUT = 60   # seconds

PLACING = [('winner', 1), ('1st', 1), ('runner', 2), ('2nd', 2), ('top 4', 4), ('top 8', 8), ('top 16', 16),
           ('top 32', 32), ('top 64', 64), ('top 128', 128), ('top 256', 256)]


def anchor(e):
    """a tournament placing as Elo: a win among many players most (log of how many it beat)"""
    p = (e.get('placing') or '').lower()
    if not p:
        return 0.0
    rank = next((r for k, r in PLACING if k in p), 64)
    players = max(int(e.get('players') or 0), rank * 2, 8)
    beaten = max(1.0, players / rank)
    return 40.0 * math.log2(beaten)


def prior(e, power, ids_to_pw):
    vals = sorted((power.get(ids_to_pw.get(x), 1500) for x in e['main']), reverse=True)
    if not vals:
        return 1500.0
    top = vals[:12]
    avg = sum(vals) / len(vals)
    ex = [power.get(ids_to_pw.get(x), 1500) for x in e['extra']]
    exavg = sum(ex) / len(ex) if ex else 1450
    return 0.45 * avg + 0.40 * (sum(top) / len(top)) + 0.15 * exavg


class Workers:
    """a worker process a core; one that dies on a duel (an engine crash) is started again, that duel counted as
    unplayed (result -1) and its decks noted (crashes)"""
    def __init__(self, pool_txt, jobs):
        self.pool_txt = pool_txt
        self.todo = queue.Queue()
        self.done = queue.Queue()
        self.crashes = {}
        for _ in range(jobs):
            threading.Thread(target=self.run, daemon=True).start()

    def spawn(self):
        exe = os.path.join(TMP, 'rate_pool_worker')
        p = subprocess.Popen([exe, self.pool_txt], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.DEVNULL, text=True, bufsize=1, errors='replace')
        p.buf = b''
        return p

    @staticmethod
    def readline(p, deadline):
        """a line from the worker by its raw pipe (its own buffer: select can't see what Python's reader holds)"""
        fd = p.stdout.fileno()
        while b'\n' not in p.buf:
            left = deadline - time.time()
            if left <= 0 or not select.select([fd], [], [], left)[0]:
                return None
            chunk = os.read(fd, 65536)
            if not chunk:
                return None
            p.buf += chunk
        line, p.buf = p.buf.split(b'\n', 1)
        return line.decode('utf-8', 'replace')

    def run(self):
        p = self.spawn()
        while True:
            job = self.todo.get()
            if job is None:
                try:
                    p.stdin.close()
                    p.wait(timeout=10)
                except Exception:
                    p.kill()
                return
            a, b, seed, first = job
            got = None
            try:
                p.stdin.write('%d %d %d %d\n' % (a, b, seed, first))
                p.stdin.flush()
                deadline = time.time() + DUEL_TIMEOUT
                while True:
                    line = self.readline(p, deadline)
                    if line is None:
                        break   # (it died, or a duel that never ends: the worker goes, started again)
                    f = line.split()
                    if len(f) == 4 and f[0] == str(a) and f[1] == str(b) and f[2].lstrip('-').isdigit():
                        got = (a, b, int(f[2]), int(f[3]))
                        break
            except (BrokenPipeError, OSError):
                pass
            if got is None:
                for x in (a, b):
                    self.crashes[x] = self.crashes.get(x, 0) + 1
                try:
                    p.kill()
                except Exception:
                    pass
                p = self.spawn()
                got = (a, b, -1, 0)
            self.done.put(got)

    def play(self, pairs):
        for job in pairs:
            self.todo.put(job)
        return [self.done.get() for _ in pairs]

    def close(self):
        for _ in range(64):
            self.todo.put(None)


def bt_fit(n, results, pri, strength=0.002, iters=300):
    """Bradley-Terry on Elo's scale by gradient ascent, each deck held toward its prior (Gaussian, 1/strength)"""
    r = pri.copy()
    if not results:
        return r
    a = np.array([x[0] for x in results])
    b = np.array([x[1] for x in results])
    s = np.array([1.0 if x[2] == 0 else 0.0 if x[2] == 1 else 0.5 for x in results])
    k = math.log(10) / 400
    lr = 30.0
    for it in range(iters):
        pa = 1 / (1 + np.exp(-k * (r[a] - r[b])))
        g = np.zeros(n)
        np.add.at(g, a, s - pa)
        np.add.at(g, b, pa - s)
        g -= strength * (r - pri)
        cnt = np.bincount(np.concatenate([a, b]), minlength=n) + 1
        r += lr * g / np.sqrt(cnt)
    return r


def main():
    rounds = int(sys.argv[1]) if len(sys.argv) > 1 else 12
    jobs = int(sys.argv[2]) if len(sys.argv) > 2 else os.cpu_count() or 4
    pool = json.load(open(POOL))
    catalog = json.load(open(os.path.join(ROOT, 'tools/ygo/catalog.json')))['cards']
    ids_to_pw = {c['id']: c['password'] for c in catalog}
    power = {int(k): v for k, v in json.load(open(os.path.join(ROOT, 'tools/ygo/card_power.json'))).items()}
    n = len(pool)
    pri = np.array([prior(e, power, ids_to_pw) for e in pool])
    # (the card prior on the Elo scale: spread like a field of decks, then the placings on top)
    pri = 1500 + (pri - pri.mean()) / (pri.std() + 1e-9) * 150 + np.array([anchor(e) for e in pool])
    pool_txt = os.path.join(TMP, 'pool.txt')
    with open(pool_txt, 'w') as f:
        for e in pool:
            f.write(' '.join(map(str, e['main'])) + ' | ' + ' '.join(map(str, e['extra'])) + '\n')
    key = [e['num'] for e in pool]
    index = {k: i for i, k in enumerate(key)}
    results, played_rounds = [], 0
    crashes = {}
    if os.path.exists(RESULTS):   # (a result is kept by the decks' numbers: still good after the pool changes)
        for line in open(RESULTS):
            f = line.split()
            if len(f) >= 4 and f[0] == 'round':
                played_rounds = max(played_rounds, int(f[1]))
            elif len(f) >= 4 and int(f[2]) < 0:
                for x in (int(f[0]), int(f[1])):
                    crashes[x] = crashes.get(x, 0) + 1
            elif len(f) >= 4 and int(f[0]) in index and int(f[1]) in index:
                results.append((index[int(f[0])], index[int(f[1])], int(f[2]), int(f[3])))
    elo = bt_fit(n, results, pri.copy(), iters=100) if results else pri.copy()
    met = set((min(a, b), max(a, b)) for a, b, _, _ in results)
    log = open(RESULTS, 'a')
    w = Workers(pool_txt, jobs)
    rng = random.Random(1234 + played_rounds)
    try:
        for rd in range(played_rounds + 1, rounds + 1):
            t0 = time.time()
            order = sorted(range(n), key=lambda i: elo[i] + rng.gauss(0, 25))
            used, pairs = set(), []
            for pos, i in enumerate(order):
                if i in used:
                    continue
                for j in order[pos + 1:pos + 8]:
                    if j not in used and (min(i, j), max(i, j)) not in met:
                        used.update((i, j))
                        pairs.append((i, j, rng.randrange(1 << 30), rd % 2))
                        break
            out = w.play(pairs)
            K = max(16.0, 64.0 / math.sqrt(rd))
            for a, b, res, turns in out:
                met.add((min(a, b), max(a, b)))
                if res < 0:   # (the engine crashed on it: not a result)
                    log.write('%d %d -1 0\n' % (key[a], key[b]))
                    for x in (key[a], key[b]):
                        crashes[x] = crashes.get(x, 0) + 1
                    continue
                results.append((a, b, res, turns))
                log.write('%d %d %d %d\n' % (key[a], key[b], res, turns))
                ea = 1 / (1 + 10 ** ((elo[b] - elo[a]) / 400))
                sa = 1.0 if res == 0 else 0.0 if res == 1 else 0.5
                elo[a] += K * (sa - ea)
                elo[b] -= K * (sa - ea)
            log.write('round %d %d %.0f\n' % (rd, len(out), time.time() - t0))
            log.flush()
            print('round %d: %d duels in %.0f s' % (rd, len(out), time.time() - t0), flush=True)
    finally:
        w.close()
    final = bt_fit(n, results, pri)
    final = 1500 + (final - final.mean())
    games = np.bincount(np.array([x[0] for x in results] + [x[1] for x in results], dtype=int), minlength=n) if results else np.zeros(n)
    wins = np.zeros(n)
    for a, b, res, _ in results:
        wins[a] += res == 0
        wins[b] += res == 1
    for i, e in enumerate(pool):
        e['rating'] = int(round(final[i]))
        e['prior'] = int(round(pri[i]))
        e['games'] = int(games[i])
        e['wins'] = int(wins[i])
        e['crashes'] = crashes.get(e['num'], 0)   # (the engine fell over in its duels: 2+ and it is left out)
    json.dump(pool, open(OUT, 'w'))
    corr = float(np.corrcoef(pri, final)[0, 1]) if n > 2 else 0
    order = sorted(range(n), key=lambda i: -final[i])
    rep = ['Yu-Gi-Oh! generic duelists\' decks rated (tools/ygo/rate_pool.py)', '',
           '%d decks, %d duels over %d rounds; the text prior and the duels agree with r = %.3f' % (n, len(results), rounds, corr),
           '', 'strongest 50:']
    rep += ['  %5d  (prior %4d, %2d/%2d)  %s  [%s, %s]' % (final[i], pri[i], wins[i], games[i], pool[i]['name'],
                                                         pool[i]['format'], pool[i]['era'][:4]) for i in order[:50]]
    rep += ['', 'weakest 30:']
    rep += ['  %5d  (prior %4d, %2d/%2d)  %s  [%s, %s]' % (final[i], pri[i], wins[i], games[i], pool[i]['name'],
                                                         pool[i]['format'], pool[i]['era'][:4]) for i in order[-30:]]
    open(os.path.join(ROOT, 'docs/audit_ygo_npc_pool.txt'), 'w').write('\n'.join(rep) + '\n')
    print('\n'.join(rep[:4]))


if __name__ == '__main__':
    main()
