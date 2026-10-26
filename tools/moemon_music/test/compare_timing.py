# Timing of a conversion against the original. Both are resampled to 16 kHz; onsets are spectral flux
# (rises in each frequency band, 5 ms hops); for each window the lag that lines them up best is found.
# Steady timing = a steady lag.   compare_timing.py ORIG.wav KAI.wav SECONDS [WINDOW]
import sys, wave, numpy as np
def flux(p, secs):
    w = wave.open(p); r = w.getframerate()
    a = np.frombuffer(w.readframes(int(secs * r)), dtype='<i2').astype(np.float32).reshape(-1, 2).mean(1)
    t = np.arange(int(len(a) * 16000 / r)) * (r / 16000.0)
    a = np.interp(t, np.arange(len(a)), a)
    hop, n = 80, 512
    frames = (len(a) - n) // hop
    idx = np.arange(n)[None, :] + hop * np.arange(frames)[:, None]
    S = np.log(np.abs(np.fft.rfft(a[idx] * np.hanning(n), axis=1)) + 1e-3)
    f = np.maximum(np.diff(S, axis=0, prepend=S[:1]), 0).sum(1)
    f -= np.convolve(f, np.ones(41) / 41, 'same')      # take out the slow part
    return np.maximum(f, 0)
secs = float(sys.argv[3]); win = float(sys.argv[4]) if len(sys.argv) > 4 else 3.0
A = flux(sys.argv[1], secs); B = flux(sys.argv[2], secs)
n = min(len(A), len(B)); W = int(win * 200); M = 40
lags = []
typical = np.median([A[s:s + W].sum() for s in range(M, n - W - M, W)] or [0])
for s in range(M, n - W - M, W):
    a = A[s:s + W]
    if a.sum() < 0.2 * typical: continue      # the original has ended / faded out here
    c = [float(np.dot(a, B[s + L:s + L + W])) for L in range(-M, M + 1)]
    lags.append((int(np.argmax(c)) - M) * 5)
lags = np.array(lags)
print('lag ms per window:', ' '.join(str(int(x)) for x in lags))
print('median %d ms, spread (mean |lag - median|) %.1f ms' % (np.median(lags), np.abs(lags - np.median(lags)).mean()))
