# CHIPTUNE: how close a chip version sounds to the original (what tuned the arranger): harmony (chroma per 100 ms),
# rhythm (onset envelope), dynamics (loudness contour), tone balance (energy per band). chip_compare.py ORIG.wav CHIP.wav [SECONDS]
import sys, wave, numpy as np
def load(p, secs):
    w = wave.open(p); r = w.getframerate(); n = min(w.getnframes(), int(secs * r)); ch = w.getnchannels()
    a = np.frombuffer(w.readframes(n), dtype='<i2').astype(np.float32).reshape(-1, ch).mean(1) / 32768
    if r != 32000:
        t = np.arange(int(len(a) * 32000 / r)) * r / 32000
        a = np.interp(t, np.arange(len(a)), a)
    return a
secs = float(sys.argv[3]) if len(sys.argv) > 3 else 55
A, B = load(sys.argv[1], secs), load(sys.argv[2], secs)
n = min(len(A), len(B)); A, B = A[:n], B[:n]
N, H = 4096, 1600                                      # 128 ms windows, 50 ms hop
f = np.fft.rfftfreq(N, 1 / 32000)
win = np.hanning(N)
key = 69 + 12 * np.log2(np.maximum(f, 1) / 440)
pc = np.round(key).astype(int) % 12
tonal = (f > 55) & (f < 4200)
edges = np.geomspace(40, 12000, 13)
def frames(x):
    S = np.array([np.abs(np.fft.rfft(x[i:i + N] * win)) for i in range(0, len(x) - N, H)])
    chroma = np.zeros((len(S), 12))
    for k in range(12): chroma[:, k] = (S[:, tonal & (pc == k)] ** 2).sum(1)
    bands = np.array([(S[:, (f >= edges[b]) & (f < edges[b + 1])] ** 2).sum(1) for b in range(12)]).T
    flux = np.maximum(np.diff(np.log(S + 1e-6), axis=0), 0).sum(1)
    return S, chroma, bands, flux
SA, CA, BA, FA = frames(A); SB, CB, BB, FB = frames(B)
m = min(len(CA), len(CB))
CA, CB, BA, BB = CA[:m], CB[:m], BA[:m], BB[:m]
loud = BA.sum(1) > BA.sum(1).max() * 1e-3
def cos(x, y): return (x * y).sum(1) / (np.linalg.norm(x, axis=1) * np.linalg.norm(y, axis=1) + 1e-12)
ch = cos(np.sqrt(CA), np.sqrt(CB))[loud]
# the original's chord tones (pitch classes above 30% of its strongest) the chip leaves out, per frame
na = CA / (CA.max(1, keepdims=True) + 1e-12); nb = CB / (CB.max(1, keepdims=True) + 1e-12)
present = na > 0.3; missing = (present & (nb < 0.1)).sum(1)[loud]; extra = ((nb > 0.3) & (na < 0.1)).sum(1)[loud]
fa, fb = FA[:m - 1], FB[:m - 1]
rh = np.corrcoef(fa, fb)[0, 1]
ba = 10 * np.log10(BA[loud].mean(0) + 1e-12); bb = 10 * np.log10(BB[loud].mean(0) + 1e-12)
d = (bb - ba); d -= np.median(d)
print('harmony (chroma similarity)  %.3f   (1 = the same notes sounding)' % ch.mean())
print('  chord tones of the original missing, per moment: %.2f   notes the original hasn\'t: %.2f' % (missing.mean(), extra.mean()))
print('rhythm (onset correlation)   %.3f' % rh)
ea = np.log(BA.sum(1) + 1e-9); eb = np.log(BB.sum(1) + 1e-9)
print('dynamics (loudness contour correlation) %.3f' % np.corrcoef(ea[loud], eb[loud])[0, 1])
print('tone balance, chip minus original per band (dB):')
print('  ' + '  '.join('%5.0f' % e for e in edges[:-1]))
print('  ' + '  '.join('%+5.1f' % x for x in d))
# rhythm per register, and the bass's notes on their own
def bandflux(S, lo, hi):
    sel = (f >= lo) & (f < hi)
    return np.maximum(np.diff(np.log(S[:, sel] + 1e-6), axis=0), 0).sum(1)
for lo, hi, name in ((40, 200, 'bass'), (200, 1500, 'middle'), (1500, 12000, 'high')):
    a, b = bandflux(SA[:m], lo, hi), bandflux(SB[:m], lo, hi)
    print('rhythm in the %-6s %.3f' % (name, np.corrcoef(a, b)[0, 1]))
low = (f > 30) & (f < 180)
def lowpc(S):
    c = np.zeros((len(S), 12))
    for k in range(12): c[:, k] = (S[:, low & (pc == k)] ** 2).sum(1)
    return c
la, lb = lowpc(SA[:m]), lowpc(SB[:m])
print('bass notes (chroma under 180 Hz) %.3f' % cos(np.sqrt(la), np.sqrt(lb))[loud].mean())
