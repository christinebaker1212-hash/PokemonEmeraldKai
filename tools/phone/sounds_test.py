#!/usr/bin/env python3
"""The app's sounds: the pack the game exports (sounds.bin) holds every sound the app names, and they're sound.

    python3 tools/phone/sounds_test.py [sounds.bin]   (default: android/app/src/main/assets/game/sounds.bin)

Checks: every "se_..." / "mus_..." / "scene:mus_..." the app's Java names, every SE_ in the shared attack table
(src/tcg/tcg_flow.c TcgFlow_AttackSound), a cry for every species, songs with a loop inside them and a crossfade tail,
nothing silent or clipped flat, ADPCM data the size its frames need.
"""
import glob
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sounds_pack  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'android/app/src/main/assets/game/sounds.bin')
    b, sounds = sounds_pack.read(path)
    names = {s['name']: s for s in sounds}
    bad = []
    # what the app names
    wanted = set()
    for f in glob.glob(os.path.join(ROOT, 'android/app/src/main/java/com/moemon/companion/*.java')):
        for m in re.finditer(r'"!?((?:[a-z]+:)?(?:se|mus)_[a-z0-9_]+)"', open(f).read()):
            wanted.add(m.group(1))
    # the attack sounds the duel tables play by number
    flow = open(os.path.join(ROOT, 'src/tcg/tcg_flow.c')).read()
    attack = set(re.findall(r'\bSE_[A-Z0-9_]+', flow[flow.index('TcgFlow_AttackSound(const char'):]))
    for a in attack:
        wanted.add(a.lower())
    for w in sorted(wanted):
        if w not in names:
            bad.append('the app names %s: not in the pack' % w)
    # a cry for every species
    species = open(os.path.join(ROOT, 'include/constants/species.h')).read()
    egg = int(re.search(r'#define SPECIES_EGG (\d+)', species).group(1))
    cries = [s for s in sounds if s['kind'] == 1]
    missing = [n for n in range(1, egg) if 'cry_%d' % n not in names]
    if missing:
        bad.append('no cry for %d species (first %s)' % (len(missing), missing[:5]))
    for s in sounds:
        need = 4 * s['ch'] + (s['frames'] * s['ch'] + 1) // 2
        if s['size'] != need:
            bad.append('%s: %d bytes for %d frames' % (s['name'], s['size'], s['frames']))
        if s['kind'] == 3:
            if not (0 <= s['loop'] < s['loop_end'] < s['frames']):
                bad.append('%s: loop %d..%d of %d' % (s['name'], s['loop'], s['loop_end'], s['frames']))
            elif s['frames'] - s['loop_end'] < s['rate'] // 50:
                bad.append('%s: no crossfade tail past the loop' % s['name'])
    # sound, not silence (a sample of each kind; every song)
    check = [s for s in sounds if s['kind'] != 1] + cries[::50]
    for s in check:
        pcm = sounds_pack.decode(b, s)
        peak = max(abs(x) for x in pcm) if pcm else 0
        if peak < 300:
            bad.append('%s: silent (peak %d)' % (s['name'], peak))
        clipped = sum(1 for x in pcm if abs(x) >= 32767)
        if clipped > len(pcm) // 200:
            bad.append('%s: %d samples clipped' % (s['name'], clipped))
    print('sounds: %d in the pack (%d cries), %d named by the app, %d KB' % (len(sounds), len(cries), len(wanted),
                                                                           os.path.getsize(path) // 1024))
    for x in bad:
        print('FAIL ' + x)
    print('PASS' if not bad else 'FAILED')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
