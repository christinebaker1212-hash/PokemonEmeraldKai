#!/usr/bin/env python3
"""
How loud each .kaisong in music/ plays in the game, against the game's own songs (the one it replaces, and the
game's songs as a whole).

The game plays each song from a clean sound state (the test commands `mp playsong N` / `mp playkai FILE`, then
`mp musiclock 1` so the overworld doesn't change the music) with its sound sent to a file (MOEMON_AUDIO_DUMP), one headless game per
song, and the loudness is measured over 120 s of it (most songs loop within that): RMS over 400 ms blocks with the two
gates of EBU R 128 (-70 dBFS absolute, 10 dB under the gated mean), on the mixed stereo signal, no weighting filter.
It is the same measure as the VS cabinet's music cap (fight/compat/mf_mixer.c MfMix_GatedLevel), in dB.

  loudness.py [--bin pokeemerald64] [--music music] [--jobs N] [--seconds 120] [--reference N] [--write] [--passes N]
      --reference N   also measure N of the game's own background songs spread over the list (default 40)
      --seconds S     how long each song is measured (default 120)
      --write         put each file's gain in its INFO chunk (gain=<dB>): the game plays it that much louder or
                      quieter (src/mp/mp_kaisong.c), so it matches the song it replaces (or the game's median for a
                      song the game doesn't have). The measure includes the gain already there, so running it again
                      only corrects what is still missing (a voice at full volume can't go louder)
      --passes N      with --write: measure and correct N times (the release build runs 2)
Files holding the same music (the GAME CENTER's floors) get one gain, the mean of what their songs want, so the music
doesn't jump when the game plays it on from one to the other.
The release workflow runs it on the files it ships (.github/workflows/release.yml), so a new song needs nothing done.
Prints a table; exit 0.
"""
import argparse
import hashlib
import concurrent.futures as cf
import math
import os
import re
import struct
import subprocess
import sys
import tempfile

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
RATE = 42048
START = 300                 # the frame the song starts at (the quick start is in the field by then)


def song_ids():
    """{name: id} from include/constants/songs.h"""
    out = {}
    for line in open(os.path.join(REPO, 'include', 'constants', 'songs.h')):
        m = re.match(r'#define\s+(MUS_\w+|SE_\w+)\s+(\d+)', line)
        if m:
            out[m.group(1).lower()] = int(m.group(2))
    return out


def gated_db(samples):
    """samples: list of mono floats"""
    block = int(RATE * 0.4)
    ms = []
    for i in range(0, len(samples) - block + 1, block):
        s = 0.0
        for v in samples[i:i + block]:
            s += v * v
        ms.append(s / block)
    ms = [m for m in ms if m > 1e-7]
    if not ms:
        return -99.0
    mean = sum(ms) / len(ms)
    ms = [m for m in ms if m > mean * 0.1]
    return 10 * math.log10(sum(ms) / len(ms))


def measure(bin_, music, cmd, seconds):
    """Loudness in dB of what the game plays after the test command cmd ("playsong 12" / "playkai F")."""
    with tempfile.TemporaryDirectory() as d:
        script = os.path.join(d, 's.txt')
        frames = int(seconds * 60)
        with open(script, 'w') as f:
            f.write('wait %d\nmp %s\nmp musiclock 1\nwait %d\nquit\n' % (START, cmd, frames + 89))
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', MOEMON_TEST=script, MOEMON_NO_UPDATE='1',
                   MOEMON_SHOTDIR=d, MOEMON_DEBUG_QUICKSTART='1', MOEMON_SAVE=os.path.join(d, 'a.sav'),
                   MOEMON_COOP_CONFIG=os.path.join(d, 'a.ini'), MOEMON_AUDIO_DUMP=os.path.join(d, 'a.raw'),
                   MOEMON_MUSIC_DIR=music)
        out = subprocess.run([bin_], cwd=d, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             timeout=600).stdout.decode('latin-1')
        if 'FAILED' in out:
            return None
        raw = open(os.path.join(d, 'a.raw'), 'rb').read()
    # the audio from the command's frame on (700.8 stereo float frames a game frame), skipping its first second
    start = int((START + 60) * RATE / 60) * 8
    data = raw[start:start + int(seconds * RATE) * 8]
    n = len(data) // 4
    vals = struct.unpack('<%df' % n, data[:n * 4])
    mono = [(vals[i] + vals[i + 1]) * 0.5 for i in range(0, n - 1, 2)]
    return gated_db(mono)


def info_chunk(path):
    data = open(path, 'rb').read()
    count = struct.unpack_from('<I', data, 16)[0]
    p = struct.unpack_from('<H', data, 10)[0]
    for _ in range(count):
        cid = data[p:p + 4]
        size = struct.unpack_from('<I', data, p + 4)[0]
        if cid == b'INFO':
            return data, p, size
        p += 8 + size + (-size % 4)
    return data, None, 0


def info_of(path):
    data, p, size = info_chunk(path)
    out = {}
    if p is not None:
        for line in data[p + 8:p + 8 + size].decode('utf-8', 'replace').split('\n'):
            if '=' in line:
                k, v = line.split('=', 1)
                out[k.strip()] = v.strip()
    return out


def write_gain(path, gain):
    """Sets gain=<dB> in the file's INFO chunk (the chunk is rewritten; the others stay as they are)."""
    data, p, size = info_chunk(path)
    if p is None:
        return False
    lines = [l for l in data[p + 8:p + 8 + size].decode('utf-8', 'replace').rstrip('\0').split('\n')
             if l and not l.startswith('gain=')]
    lines.append('gain=%.1f' % gain)
    body = ('\n'.join(lines) + '\n').encode('utf-8')
    pad = -len(body) % 4
    old_end = p + 8 + size + (-size % 4)
    new = data[:p] + b'INFO' + struct.pack('<I', len(body)) + body + b'\0' * pad + data[old_end:]
    tmp = path + '.part'
    open(tmp, 'wb').write(new)
    os.replace(tmp, path)
    return True


def music_key(path):
    """The music itself: the header's song fields and every chunk but INFO (files with the same key are one song;
    the game keeps it playing from one to the other: mp_kaisong.c MP_Kaisong_SameMusic)"""
    data = open(path, 'rb').read()
    count = struct.unpack_from('<I', data, 16)[0]
    p = struct.unpack_from('<H', data, 10)[0]
    parts = [data[8:32]]
    for _ in range(count):
        cid = data[p:p + 4]
        size = struct.unpack_from('<I', data, p + 4)[0]
        if cid != b'INFO':
            parts.append(data[p + 8:p + 8 + size])
        p += 8 + size + (-size % 4)
    return hashlib.sha1(b''.join(parts)).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--bin', default=os.path.join(REPO, 'pokeemerald64'))
    ap.add_argument('--music', default=os.path.join(REPO, 'music'))
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 2)
    ap.add_argument('--seconds', type=float, default=120)
    ap.add_argument('--reference', type=int, default=40)
    ap.add_argument('--passes', type=int, default=1)
    ap.add_argument('--write', action='store_true')
    a = ap.parse_args()
    a.music = os.path.abspath(a.music)
    ids = song_ids()
    files = sorted(f for f in os.listdir(a.music) if f.endswith('.kaisong'))
    if not files:
        print('no .kaisong files in %s' % a.music)
        return 0
    # files holding the same music are one group with one gain: measured once (its first file)
    groups = {}
    for f in files:
        groups.setdefault(music_key(os.path.join(a.music, f)), []).append(f)
    groups = sorted(groups.values())
    # an empty music folder for the game's own songs, so nothing is replaced
    empty = tempfile.mkdtemp()
    game = {}
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        jobs = {}
        for f in files:
            song = f.split('@')[0].replace('.kaisong', '')
            if song in ids and song not in jobs:
                jobs[song] = ex.submit(measure, a.bin, empty, 'playsong %d' % ids[song], a.seconds)
        bgm = sorted((v, k) for k, v in ids.items() if k.startswith('mus_') and v > 0)
        step = max(1, len(bgm) // max(1, a.reference)) if a.reference else 0
        for v, k in (bgm[::step][:a.reference] if step else []):
            if k not in jobs:
                jobs[k] = ex.submit(measure, a.bin, empty, 'playsong %d' % v, a.seconds)
        game = {k: j.result() for k, j in jobs.items()}
    levels = sorted(v for v in game.values() if v is not None and v > -60)
    median = levels[len(levels) // 2] if levels else None
    if levels:
        print('the game\'s songs: %d measured, median %.1f dB, middle half %.1f to %.1f dB' % (
            len(levels), median, levels[len(levels) // 4], levels[3 * len(levels) // 4]))

    def target_of(f):
        ref = game.get(f.split('@')[0].replace('.kaisong', ''))
        return ref if ref is not None and ref > -60 else median

    for n in range(max(1, a.passes)):
        with cf.ThreadPoolExecutor(a.jobs) as ex:
            res = list(ex.map(lambda g: measure(a.bin, a.music, 'playkai ' + g[0], a.seconds), groups))
        print('%s%-58s %8s %8s %8s %6s' % ('pass %d: ' % (n + 1) if a.passes > 1 else '', 'file', 'plays',
                                          'replaces', 'gain', 'was'))
        for g, kai in zip(groups, res):
            old = float(info_of(os.path.join(a.music, g[0])).get('gain', '0') or 0)
            targets = [t for t in (target_of(f) for f in g) if t is not None]
            if kai is None or kai < -60 or not targets:
                for f in g:
                    print('%-58s   (does not play)' % f)
                continue
            target = sum(targets) / len(targets)
            # the measure ran with the gain already in the file: correct it by what is still missing (the gain isn't
            # always fully heard: a voice can't go past full volume, so another pass gets closer)
            gain = max(-12.0, min(12.0, round(old + target - kai, 1)))
            for f in g:
                print('%-58s %8.1f %8.1f %+8.1f %+6.1f' % (f, kai, target, gain, old))
                if a.write and abs(gain - float(info_of(os.path.join(a.music, f)).get('gain', '0') or 0)) >= 0.05:
                    write_gain(os.path.join(a.music, f), gain)
        if not a.write:
            break
    return 0


if __name__ == '__main__':
    sys.exit(main())
