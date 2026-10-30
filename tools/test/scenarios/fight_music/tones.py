#!/usr/bin/env python3
# Moemon Fight's music test (fight_music). "tones.py pack DIR": a copy of mugen/ (links) whose cabinet only has the
# SSF2X Dhalsim stage, playing a test song of tones instead of its own: second k of the song is a tone of 300 + 40k
# Hz. "tones.py RESULTS": reads the game's sound (audio.raw, stereo float32 at 42048 Hz) and checks the song plays at
# its own speed: during the fight each tone lasts a second (it once played 8 times too fast, at the right pitch), and
# that the song and the fighters' voices were measured (the song is kept under the voices).
import math, os, struct, sys, wave

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", ".."))


def make_pack(dst):
    os.makedirs(dst, exist_ok=True)
    src = os.path.join(REPO, 'mugen')
    for name in os.listdir(src):
        if name in ('cabinets.ini', 'stages', 'sound'):
            continue
        link = os.path.join(dst, name)
        if not os.path.lexists(link):
            os.symlink(os.path.join(src, name), link)
    for sub in ('stages', 'sound'):
        os.makedirs(os.path.join(dst, sub), exist_ok=True)
        for name in os.listdir(os.path.join(src, sub)):
            link = os.path.join(dst, sub, name)
            if name != 'ssf2x-dhalsim.def' and not os.path.lexists(link):
                os.symlink(os.path.join(src, sub, name), link)
    with open(os.path.join(src, 'cabinets.ini'), encoding='latin-1') as f:
        ini = f.read().replace('\nstages =', '\nstages = ssf2x-dhalsim', 1)
    with open(os.path.join(dst, 'cabinets.ini'), 'w', encoding='latin-1') as f:
        f.write(ini)
    with open(os.path.join(src, 'stages', 'ssf2x-dhalsim.def'), 'rb') as f:
        stage = f.read()
    lines = [b'BGMusic = sound/tones.wav' if l.strip().lower().startswith(b'bgmusic') else l for l in stage.split(b'\n')]
    with open(os.path.join(dst, 'stages', 'ssf2x-dhalsim.def'), 'wb') as f:
        f.write(b'\n'.join(lines))
    rate = 22050
    w = wave.open(os.path.join(dst, 'sound', 'tones.wav'), 'wb')
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(rate)
    frames = bytearray()
    for k in range(60):
        f = 300 + 40 * k
        for i in range(rate):
            v = int(16000 * math.sin(2 * math.pi * f * i / rate)) if i < rate * 8 // 10 else 0
            frames += struct.pack('<h', v)
    w.writeframes(bytes(frames))
    w.close()


def tones(path, rate=42048):
    import numpy as np
    x = np.fromfile(path, dtype=np.float32).reshape(-1, 2).mean(1)
    hop = rate // 4
    f = np.fft.rfftfreq(hop, 1 / rate)
    band = (f > 250) & (f < 3000)
    out = []
    for i in range(0, len(x) - hop, hop):
        sp = np.abs(np.fft.rfft(x[i:i + hop] * np.hanning(hop)))
        out.append(round((f[band][np.argmax(sp[band])] - 300) / 40) if sp[band].max() > 0.5 else None)
    return out


def check(results):
    seq = tones(os.path.join(results, 'audio.raw'))
    # the longest run where the tone number goes up by one a second (4 readings each), a stray reading allowed
    best = run = 0
    for i in range(4, len(seq)):
        a, b = seq[i - 4], seq[i]
        run = run + 1 if a is not None and b is not None and b == a + 1 else 0
        best = max(best, run)
    print('quarter seconds of the song at its own speed:', best)
    if best < 40:
        print('the song does not play at its own speed:', seq[:200])
        sys.exit(1)
    # the stage's song is measured and kept under the fighters' voices (mf_mixer.c)
    import re
    log = open(os.path.join(os.path.dirname(results.rstrip('/')), 'fight_music_pack', 'mugen.log'), 'rb').read().decode('latin-1')
    song = re.search(r'BGM sound/tones\.wav[^\n]*\nBGM level (-?[\d.]+) dBFS', log)
    voice = re.search(r"Fighters' voices (-?[\d.]+) dBFS", log)
    if not song or not voice:
        print('no song or voice level in mugen.log')
        sys.exit(1)
    print('song %s dBFS, voices %s dBFS' % (song.group(1), voice.group(1)))
    if not (-14 < float(song.group(1)) < -6 and -30 < float(voice.group(1)) < -3):
        print('levels out of range')
        sys.exit(1)
    print('ok')


if __name__ == '__main__':
    if sys.argv[1] == 'pack':
        make_pack(sys.argv[2])
    else:
        check(sys.argv[1])
