#!/usr/bin/env python3
"""Makes the fetched cries (tools/moemon_cries/fetch_cries.py) into GBA cries beside the game's own:

  - silence trimmed off both ends (a few milliseconds kept), a short fade at the end
  - band-limited as the GBA's cries are: a high-pass at 80 Hz, a steep low-pass at 4.4 kHz, resampled to 10512 Hz
    (Ruby, Sapphire and Emerald's cry rate; ffmpeg's resampler)
  - as loud as the Gen 3 cries: the loud part's RMS matched to theirs (the median of every Gen 3 cry), peaks
    rounded off softly instead of clipped
  - 8-bit, no dither (the build then packs them the GBA's compressed way: audio_rules.mk, wav2agb -c)

    python3 tools/moemon_cries/gba_cries.py CACHE_DIR [NAME...]

Writes sound/direct_sound_samples/cries/<name>.wav for every CACHE_DIR/<name>.ogg (or only the names given).
"""
import glob
import os
import subprocess
import sys
import wave

import numpy as np

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
CRIES = os.path.join(ROOT, 'sound', 'direct_sound_samples', 'cries')
RATE = 10512
WORK = 32768


def decode(path):
    raw = subprocess.run(['ffmpeg', '-v', 'error', '-i', path, '-ac', '1', '-ar', str(WORK), '-f', 'f32le', '-'],
                         check=True, capture_output=True).stdout
    return np.frombuffer(raw, dtype='<f4').astype(np.float64)


def gba_band(x):
    """The GBA's band: 80 Hz - 4.4 kHz (two low-passes for a steep edge), then 10512 Hz"""
    raw = subprocess.run(['ffmpeg', '-v', 'error', '-f', 'f32le', '-ar', str(WORK), '-ac', '1', '-i', '-',
                          '-af', 'highpass=f=80,lowpass=f=4400,lowpass=f=4400,lowpass=f=4400',
                          '-ar', str(RATE), '-f', 'f32le', '-'],
                         input=x.astype('<f4').tobytes(), check=True, capture_output=True).stdout
    return np.frombuffer(raw, dtype='<f4').astype(np.float64)


def loud_rms(x, rate):
    """The RMS of the loud part: 10 ms frames within 30 dB of the loudest"""
    n = max(1, rate // 100)
    frames = x[:len(x) // n * n].reshape(-1, n) if len(x) >= n else x.reshape(1, -1)
    r = np.sqrt((frames ** 2).mean(axis=1))
    if not r.size or r.max() <= 0:
        return 0.0
    loud = r[r > r.max() * 10 ** (-30 / 20)]
    return float(np.sqrt((loud ** 2).mean()))


def read_wav(path):
    w = wave.open(path)
    d = np.frombuffer(w.readframes(w.getnframes()), dtype=np.uint8).astype(np.float64)
    return (d - 128) / 128, w.getframerate()


def trim(x, rate):
    a = np.abs(x)
    if not a.size or a.max() <= 0:
        return x
    on = np.nonzero(a > a.max() * 10 ** (-42 / 20))[0]
    pad = rate * 4 // 1000
    x = x[max(0, on[0] - pad):on[-1] + pad + 1].copy()
    fade = min(len(x), rate * 8 // 1000)
    x[len(x) - fade:] *= np.linspace(1, 0, fade)
    return x


def gen3_target(skip):
    """The Gen 3 cries' loudness: the median loud-part RMS of the 10512 Hz cries (not the ones made here)"""
    vals = []
    for p in glob.glob(os.path.join(CRIES, '*.wav')):
        if os.path.basename(p)[:-4] in skip:
            continue
        try:
            x, rate = read_wav(p)
        except Exception:
            continue
        if rate == RATE and len(x) > rate // 10:
            vals.append(loud_rms(x, rate))
    return float(np.median(vals)), len(vals)


def soft(x, knee=0.85):
    """Peaks above the knee rounded off toward 1"""
    y = x.copy()
    over = np.abs(y) > knee
    y[over] = np.sign(y[over]) * (knee + (1 - knee) * np.tanh((np.abs(y[over]) - knee) / (1 - knee)))
    return y


def write_wav(path, x):
    q = np.clip(np.round(x * 127) + 128, 1, 255).astype(np.uint8)
    w = wave.open(path, 'wb')
    w.setnchannels(1)
    w.setsampwidth(1)
    w.setframerate(RATE)
    w.writeframes(q.tobytes())
    w.close()


def main():
    cache = sys.argv[1]
    names = sys.argv[2:] or sorted(os.path.basename(p)[:-4] for p in glob.glob(os.path.join(cache, '*.ogg')))
    target, count = gen3_target(set(os.path.basename(p)[:-4] for p in glob.glob(os.path.join(cache, '*.ogg'))))
    print('Gen 3 loudness: %.3f (%d cries)' % (target, count))
    for name in names:
        x = trim(decode(os.path.join(cache, name + '.ogg')), WORK)
        y = gba_band(x)
        r = loud_rms(y, RATE)
        if r > 0:
            y = y * (target / r)
        write_wav(os.path.join(CRIES, name + '.wav'), soft(y))
    print('%d cries written' % len(names))


if __name__ == '__main__':
    main()
