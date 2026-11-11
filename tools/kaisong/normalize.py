#!/usr/bin/env python3
"""
Every custom song (music/*.kaisong) played equally loud, at build time, in a few seconds.

The game measures each song in its own sound engine, rendered as fast as it runs (MOEMON_MEASURE_MUSIC,
src/mp/mp_export_sound.c: one pass of the song's loop, at most 60 s, about a quarter of a second a minute of music): its
loudness (RMS of 400 ms blocks with EBU R 128's two gates, as the VS cabinet's music cap measures) and its peak. Each
file gets ONE gain, the difference to the target: the dynamics inside a song stay as they are (no compression, no
limiting). The target is the median loudness of the game's own songs these files replace, so custom music sits with
the rest of the game's sound. A gain never takes a song's peak over -1 dBFS; a song raised is measured again at its
gain (a note already at full volume can't go louder) and corrected once.

Measurements are kept by each file's content (tools/kaisong/loudness_cache.json, committed): only new or changed
songs are measured, so a build with nothing new takes well under a second and a new song about one. The gains go
to music/kaisong_gains.txt ("<file>\\t<dB>"), which the game reads (src/mp/mp_kaisong.c ReadGainTable; it wins over a
file's INFO gain=) and the release ships beside the songs.

  normalize.py [--bin ./pokeemerald64] [--music music] [--jobs N] [--fresh] [--check]
      --fresh   measure everything again (after a change to the sound engine)
      --check   only report: exit 1 if a file has no measurement in the cache (nothing is run or written)
The release runs it after the Linux build; run it after adding a song and commit the cache and the gains.
"""
import argparse, hashlib, json, os, re, statistics, subprocess, sys, tempfile, time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CACHE = os.path.join(ROOT, 'tools', 'kaisong', 'loudness_cache.json')
PEAK_CEILING = -1.0   # dBFS
GAIN_LIMIT = 12.0     # dB either way (the game's range)


def song_numbers():
    nums = {}
    for line in open(os.path.join(ROOT, 'include', 'constants', 'songs.h')):
        m = re.match(r'#define\s+(MUS_\w+)\s+(\d+)', line)
        if m:
            nums[m.group(1).lower()] = int(m.group(2))
    return nums


def measure(binary, items, jobs):
    """{item: (loudness, peak)} for items [(name, gain dB)], run in `jobs` games side by side."""
    if not items:
        return {}
    tmp = tempfile.mkdtemp(prefix='kaisong_measure')
    chunks = [items[i::jobs] for i in range(jobs) if items[i::jobs]]
    procs = []
    for n, chunk in enumerate(chunks):
        lst, out = os.path.join(tmp, 'list%d.txt' % n), os.path.join(tmp, 'out%d.tsv' % n)
        with open(lst, 'w') as f:
            f.writelines('%s %.1f\n' % (name, g) for name, g in chunk)
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_MEASURE_MUSIC=out,
                   MOEMON_MEASURE_LIST=lst, MOEMON_SAVE=os.path.join(tmp, 'x%d.sav' % n),
                   MOEMON_SKIP_COOP_SETUP='1', MOEMON_COOP_CONFIG=os.path.join(tmp, 'x%d.ini' % n),
                   MOEMON_MUSIC_DIR=os.path.abspath(args.music))
        env.pop('MOEMON_TEST', None)
        procs.append((subprocess.Popen([binary], cwd=ROOT, env=env, stdout=subprocess.DEVNULL,
                                       stderr=subprocess.DEVNULL), out))
    got = {}
    for p, out in procs:
        p.wait(timeout=1800)
        for line in open(out) if os.path.exists(out) else []:
            f = line.rstrip('\n').split('\t')
            if len(f) >= 4 and f[2] != 'failed':
                got[(f[0], round(float(f[1]), 1))] = (float(f[2]), float(f[3]))
    return got


def main():
    global args
    ap = argparse.ArgumentParser()
    ap.add_argument('--bin', default=os.path.join(ROOT, 'pokeemerald64'))
    ap.add_argument('--music', default=os.path.join(ROOT, 'music'))
    ap.add_argument('--jobs', type=int, default=max(1, os.cpu_count() or 1))
    ap.add_argument('--fresh', action='store_true')
    ap.add_argument('--check', action='store_true')
    args = ap.parse_args()
    t0 = time.time()
    cache = {'files': {}, 'songs': {}}
    if os.path.exists(CACHE) and not args.fresh:
        cache = json.load(open(CACHE))
    files = sorted(f for f in os.listdir(args.music) if f.endswith('.kaisong'))
    key = {f: hashlib.sha1(open(os.path.join(args.music, f), 'rb').read()).hexdigest() for f in files}
    nums = song_numbers()
    refs = sorted({nums[f.split('@')[0].split('.')[0]] for f in files if f.split('@')[0].split('.')[0] in nums})

    def cached(f, g):
        return cache['files'].get(key[f], {}).get('%.1f' % g)

    want = [(f, 0.0) for f in files if cached(f, 0.0) is None]
    want += [('song:%d' % n, 0.0) for n in refs if str(n) not in cache['songs']]
    if args.check:
        print('%d songs not measured' % len(want))
        return 1 if want else 0
    if want and not os.path.exists(args.bin):
        print('normalize.py: no game binary (%s) to measure %d songs; gains kept as they are' % (args.bin, len(want)))
        return 0

    def store(got):
        for (name, g), (loud, peak) in got.items():
            if name.startswith('song:'):
                cache['songs'][name[5:]] = [loud, peak]
            elif name in key:
                cache['files'].setdefault(key[name], {})['%.1f' % g] = [loud, peak]

    store(measure(args.bin, want, args.jobs))
    levels = [cache['songs'][str(n)][0] for n in refs if str(n) in cache['songs']]
    target = statistics.median(levels) if levels else -18.0

    def gain_for(loud, peak, base=0.0):
        g = base + target - loud
        g = min(g, base + PEAK_CEILING - peak)
        return round(max(-GAIN_LIMIT, min(GAIN_LIMIT, g)), 1)

    gains, again = {}, []
    for f in files:
        m = cached(f, 0.0)
        if m is None:
            continue
        g = gain_for(*m)
        gains[f] = g
        if g > 0.3 and cached(f, g) is None:
            again.append((f, g))
    store(measure(args.bin, again, args.jobs))
    for f, g in list(gains.items()):
        m = cached(f, g)
        if g > 0.3 and m is not None and m[0] < target - 0.3:
            gains[f] = gain_for(m[0], m[1], g)   # (raised less than asked: notes at full volume; once more)
    # Colosseum's songs (ORRE's sound: musyx/orre_musyx.c), measured the same way and brought to the same target: a
    # gain each into music/orre_gains.txt (src/orre/orre_audio.c reads it). The whole of ORRE's mix takes the gain of
    # the song playing, so its sound effects keep Colosseum's own balance with its music; "fx" is the gain while no
    # song plays: the songs' median. Needs ORRE's pack (orre/orre_data.pak, from the disc); without it the gains kept
    # are shipped as they are. Kept by the pack's own content
    orre_new = 0
    pack = os.path.join(ROOT, 'orre', 'orre_data.pak')
    orre = cache.get('orre') or {}
    if os.path.exists(pack) and os.path.exists(args.bin):
        h = hashlib.sha1(open(pack, 'rb').read()).hexdigest()
        if orre.get('pack') != h or args.fresh:
            got = measure(args.bin, [('orre:all', 0.0)], 1)
            songs = {name[5:]: [loud, peak] for (name, _g), (loud, peak) in got.items() if name.startswith('orre:')}
            if songs:
                orre = {'pack': h, 'songs': songs}
                orre_new = len(songs)
    if orre.get('songs'):
        cache['orre'] = orre
        # (6 dB either way at most: a track made quiet, the wind over the desert, stays a quiet one)
        og = {int(k): max(-6.0, min(6.0, gain_for(*v))) for k, v in orre['songs'].items()}
        with open(os.path.join(args.music, 'orre_gains.txt'), 'w') as f:
            f.write('# Colosseum\'s songs in ORRE: each sound id\'s gain in dB, so all play %.1f dBFS loud; fx: while no song '
                    'plays (tools/kaisong/normalize.py; generated)\n' % target)
            f.write('fx\t%.1f\n' % statistics.median(og.values()))
            for k in sorted(og):
                f.write('%d\t%.1f\n' % (k, og[k]))
    os.makedirs(os.path.dirname(CACHE), exist_ok=True)
    keep = set(key.values())
    cache['files'] = {k: v for k, v in cache['files'].items() if k in keep}
    cache['target'] = round(target, 2)
    with open(CACHE, 'w') as f:
        json.dump(cache, f, indent=0, sort_keys=True)
        f.write('\n')
    with open(os.path.join(args.music, 'kaisong_gains.txt'), 'w') as f:
        f.write('# Every music/ file\'s gain in dB, so all play %.1f dBFS loud (tools/kaisong/normalize.py; generated)\n'
                % target)
        for name in files:
            if name in gains:
                f.write('%s\t%.1f\n' % (name, gains[name]))
    # (said so it can't be read as a failure: "0 measured now" is every song already measured, its gain shipped)
    if orre_new:
        print('normalize.py: ORRE: %d of Colosseum\'s songs measured' % orre_new)
    if not want:
        print('normalize.py: OK, all %d songs already measured (nothing new or changed): gains written, target %.1f dB, %.1f s'
              % (len(gains), target, time.time() - t0))
    else:
        print('normalize.py: OK, %d songs, target %.1f dB: %d new or changed measured now (%d again at their gain), %.1f s'
              % (len(gains), target, len(want), len(again), time.time() - t0))
    return 0


if __name__ == '__main__':
    sys.exit(main())
