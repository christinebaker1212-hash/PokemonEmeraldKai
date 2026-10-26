# How close a conversion is to the original, in detail: fidelity.py ORIG.wav KAI.wav [SECONDS] [--worst N]
# Both are lined up to the sample (the best overall offset), brought to the same overall level, and compared
# on a fine time-frequency grid (2048-point frames every 16 ms at 32 kHz, 64 mel bands, as the ear spaces
# pitch). Reported:
#   lsd        mean difference of the two log-mel spectrograms (dB, frames where the original sounds)
#   bands      that difference per region (low / low-mid / mid / high-mid / high)
#   attack     the difference in the first 50 ms after each onset of the original
#   level      how the loudness moves against the original's over time (dB, 100 ms steps)
#   worst      the moments that differ most (time, dB, the region)
import sys, wave, numpy as np

def load(p, secs, rate=32000):
    w = wave.open(p); r = w.getframerate(); n = w.getnframes() if secs <= 0 else min(w.getnframes(), int(secs * r))
    a = np.frombuffer(w.readframes(n), dtype='<i2').astype(np.float64).reshape(-1, w.getnchannels()).mean(1) / 32768
    t = np.arange(int(len(a) * rate / r)) * (r / rate)
    return np.interp(t, np.arange(len(a)), a)

def mel_fb(n_fft, rate, nb=64, lo=40, hi=15000):
    m = lambda f: 2595 * np.log10(1 + f / 700); im = lambda x: 700 * (10 ** (x / 2595) - 1)
    pts = im(np.linspace(m(lo), m(hi), nb + 2)); f = np.fft.rfftfreq(n_fft, 1 / rate)
    fb = np.zeros((nb, len(f)))
    for i in range(nb):
        a, c, b = pts[i], pts[i + 1], pts[i + 2]
        fb[i] = np.clip(np.minimum((f - a) / (c - a), (b - f) / (b - c)), 0, None)
    return fb, pts[1:-1]

def spec(x, n=2048, hop=512):
    fr = (len(x) - n) // hop
    if fr <= 0: return np.zeros((0, n // 2 + 1))
    idx = np.arange(n)[None, :] + hop * np.arange(fr)[:, None]
    return np.abs(np.fft.rfft(x[idx] * np.hanning(n), axis=1)) ** 2

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    worstN = int(sys.argv[sys.argv.index('--worst') + 1]) if '--worst' in sys.argv else 5
    if '--worst' in sys.argv: args.remove(str(worstN))
    secs = float(args[2]) if len(args) > 2 else 0
    A = load(args[0], secs); B = load(args[1], secs); rate = 32000
    # the offset: envelopes first (fast), then the samples around it
    hop = 160
    ea = np.sqrt(np.convolve(A ** 2, np.ones(hop) / hop, 'same')[::hop]); eb = np.sqrt(np.convolve(B ** 2, np.ones(hop) / hop, 'same')[::hop])
    n = min(len(ea), len(eb)); M = 40
    da = np.diff(np.log(ea[:n] + 1e-4)).clip(0); db = np.diff(np.log(eb[:n] + 1e-4)).clip(0)
    c = [np.dot(da[M:-M], db[M + L:len(db) - M + L]) for L in range(-M, M + 1)]
    lag = (int(np.argmax(c)) - M) * hop
    best, bl = -1, lag
    seg = slice(rate * 5, min(len(A), len(B)) - rate * 5 - 2 * hop) if min(len(A), len(B)) > rate * 12 else slice(2 * hop, min(len(A), len(B)) - 2 * hop)
    a = A[seg]
    for L in range(lag - hop, lag + hop + 1, 4):
        s = seg.start + L
        if s < 0 or s + len(a) > len(B): continue
        v = np.dot(a, B[s:s + len(a)]) / (np.linalg.norm(B[s:s + len(a)]) + 1e-9)
        if v > best: best, bl = v, L
    lag = bl
    if lag > 0: B = B[lag:]
    else: A = A[-lag:]
    n = min(len(A), len(B)); A = A[:n]; B = B[:n]
    g = np.sqrt((A ** 2).mean() / ((B ** 2).mean() + 1e-12)); B = B * g
    fb, centers = mel_fb(2048, rate)
    SA = np.log10(spec(A) @ fb.T + 1e-10) * 10; SB = np.log10(spec(B) @ fb.T + 1e-10) * 10
    loud = SA.max(1) > SA.max() - 45                       # frames where the original sounds
    floor = SA.max() - 70                                  # differences under the original's quiet floor don't count
    D = np.maximum(SA, floor) - np.maximum(SB, floor)
    lsd = np.sqrt((D[loud] ** 2).mean(1)).mean()
    regions = [('low <250', centers < 250), ('lowmid 250-1k', (centers >= 250) & (centers < 1000)), ('mid 1-4k', (centers >= 1000) & (centers < 4000)),
               ('highmid 4-8k', (centers >= 4000) & (centers < 8000)), ('high >8k', centers >= 8000)]
    print('offset %.1f ms (kai %s), level x%.2f' % (abs(lag) / rate * 1000, 'later' if lag > 0 else 'earlier', g))
    print('lsd %.2f dB' % lsd)
    print('bands ' + '  '.join('%s %+.1f/%.1f' % (nm, D[loud][:, m].mean(), np.abs(D[loud][:, m]).mean()) for nm, m in regions) + '   (mean / mean |diff| dB)')
    # attacks: original onsets (spectral flux peaks), the 3 frames after each
    flux = np.maximum(np.diff(SA, axis=0), 0).sum(1)
    thr = np.percentile(flux, 92)
    on = [i + 1 for i in range(1, len(flux) - 1) if flux[i] > thr and flux[i] >= flux[i - 1] and flux[i] >= flux[i + 1]]
    if on:
        att = np.mean([np.sqrt((D[i:i + 3] ** 2).mean()) for i in on if i + 3 < len(D)])
        print('attack %.2f dB over %d onsets' % (att, len(on)))
    # level over time (100 ms)
    w = rate // 10; k = n // w
    la = 10 * np.log10((A[:k * w].reshape(k, w) ** 2).mean(1) + 1e-10); lb = 10 * np.log10((B[:k * w].reshape(k, w) ** 2).mean(1) + 1e-10)
    m = la > la.max() - 40
    dl = la[m] - lb[m]
    print('level over time: mean |diff| %.2f dB, 90%% within %.2f dB' % (np.abs(dl - dl.mean()).mean(), np.percentile(np.abs(dl - dl.mean()), 90)))
    # worst moments (half-second blocks)
    blk = 31
    per = [(np.sqrt((D[i:i + blk][loud[i:i + blk]] ** 2).mean()) if loud[i:i + blk].any() else 0, i) for i in range(0, len(D) - blk, blk)]
    per.sort(reverse=True)
    for v, i in per[:worstN]:
        reg = max(regions, key=lambda r: np.abs(D[i:i + blk][:, r[1]]).mean())
        print('  worst %6.2f s  %.2f dB  most in %s (%+.1f dB)' % (i * 512 / rate, v, reg[0], D[i:i + blk][:, reg[1]].mean()))

main()
