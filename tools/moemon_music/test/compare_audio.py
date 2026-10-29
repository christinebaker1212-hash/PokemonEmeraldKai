# Compares two WAVs band by band (24 log-spaced bands, 60 Hz-16 kHz), after taking out the overall level
# difference: compare_audio.py A.wav B.wav SECONDS STEP. Prints the mean |difference| per band.
import sys, wave, numpy as np
def load(p, secs):
    w = wave.open(p); r = w.getframerate(); n = min(w.getnframes(), int(secs * r))
    a = np.frombuffer(w.readframes(n), dtype='<i2').astype(np.float32).reshape(-1, 2).mean(1) / 32768
    return a, r
def bands(a, r, t0, t1, nb=24):
    seg = a[int(t0 * r):int(t1 * r)]
    if len(seg) < 1024: return None
    win = 4096; out = np.zeros(nb); edges = np.geomspace(60, min(r / 2, 16000), nb + 1)
    f = np.fft.rfftfreq(win, 1 / r)
    for i in range(0, len(seg) - win, win // 2):
        S = np.abs(np.fft.rfft(seg[i:i + win] * np.hanning(win))) ** 2
        for b in range(nb): out[b] += S[(f >= edges[b]) & (f < edges[b + 1])].sum()
    return 10 * np.log10(out + 1e-12), edges
A, ra = load(sys.argv[1], float(sys.argv[3])); B, rb = load(sys.argv[2], float(sys.argv[3]))
step = float(sys.argv[4]) if len(sys.argv) > 4 else 5
tot = []
# windows where the original has faded out (the end of its length tag) don't count
def level(a, r, t0, t1):
    seg = a[int(t0 * r):int(t1 * r)]
    return float(np.sqrt((seg ** 2).mean())) if len(seg) else 0.0
full = level(A, ra, 0, float(sys.argv[3]))
edges = None
for t in np.arange(0, float(sys.argv[3]) - step + 0.01, step):
    if level(A, ra, t, t + step) < 0.25 * full: continue
    x = bands(A, ra, t, t + step); y = bands(B, rb, t, t + step)
    if x is None or y is None: continue
    edges = x[1]
    d = x[0] - y[0]; d -= np.median(d)
    tot.append(np.abs(d).mean())
    print('%5.0fs  mean |band diff| %.1f dB   %s' % (t, np.abs(d).mean(), ' '.join('%+3.0f' % v for v in d)))
print('overall %.2f dB' % np.mean(tot))
if edges is not None: print('bands (Hz):', ' '.join('%d' % e for e in edges))
