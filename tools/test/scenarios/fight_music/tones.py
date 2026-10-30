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
    # the longest run of seconds where the tone number goes up by one each second: each second is the tone most of
    # its four readings agree on (the fighters' voices, louder than the music, can take a reading), tried at the
    # four ways of lining seconds up with readings
    best = 0
    for phase in range(4):
        secs = []
        for i in range(phase, len(seq) - 3, 4):
            q = [v for v in seq[i:i + 4] if v is not None]
            top = max(set(q), key=q.count) if q else None
            secs.append(top if top is not None and q.count(top) >= 2 else None)
        run = 0
        for a, b in zip(secs, secs[1:]):
            run = run + 1 if a is not None and b is not None and b == a + 1 else 0
            best = max(best, run * 4)
    print('quarter seconds of the song at its own speed:', best)
    if best < 40:
        print('the song does not play at its own speed:', seq[:200])
        sys.exit(1)
    # the stage's song is measured and kept under the fighters' voices (mf_mixer.c)
    import re
    log = open(os.path.join(os.path.dirname(results.rstrip('/')), 'fight_music_pack', 'mugen.log'), 'rb').read().decode('latin-1')
    m = re.search(r"Fighters' voices (-?[\d.]+) dBFS, the stage's song (-?[\d.]+) dBFS", log)
    if not m:
        print("no Fighters' voices line in mugen.log")
        sys.exit(1)
    voice, song = float(m.group(1)), float(m.group(2))
    print('song %.1f dBFS, voices %.1f dBFS' % (song, voice))
    if not (-14 < song < -6 and -30 < voice < -3):
        print('levels out of range')
        sys.exit(1)
    print('ok')


if __name__ == '__main__':
    if sys.argv[1] == 'pack':
        make_pack(sys.argv[2])
    else:
        check(sys.argv[1])
