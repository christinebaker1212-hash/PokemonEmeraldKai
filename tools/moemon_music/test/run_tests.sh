#!/bin/bash
# Moemon Music Studio tests: make test (or run from anywhere with the built binary).
set -e
cd "$(dirname "$0")/.."
B=${1:-build/linux/moemon_music_studio}
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

echo "== converter vs tools/mid2agb on every game MIDI"
python3 test/check_mid2agb.py "$B"

echo "== a General MIDI song: project round trip, .kaisong, decomp sources"
python3 test/make_test_midi.py "$T/gm.mid"
"$B" --save "$T/gm.mid" "$T/gm.mmsproj" mus_route104
"$B" --dump "$T/gm.mid" > "$T/a.txt"
"$B" --dump "$T/gm.mmsproj" > "$T/b.txt"
cmp "$T/a.txt" "$T/b.txt"
"$B" --export "$T/gm.mmsproj" "$T/gm.kaisong"
python3 test/read_kaisong.py "$T/gm.kaisong" > /dev/null
python3 test/read_kaisong.py "$T/gm.kaisong" --bytecode > "$T/c.txt"
cmp "$T/a.txt" "$T/c.txt"
"$B" --export ../../sound/songs/midi/mus_vs_trainer.mid "$T/vs.kaisong"
python3 test/read_kaisong.py "$T/vs.kaisong" > /dev/null
"$B" --decomp ../../sound/songs/midi/mus_vs_trainer.mid "$T"
printf '\t.include "asm/macros/m4a.inc"\n\t.include "asm/macros/music_voice.inc"\n\t.include "asm/macros/bit_width.inc"\n\t.section .rodata\n\t.include "%s"\n' "$T/mus_vs_trainer_kai.inc" > "$T/vg.s"
(cd ../.. && as --64 --defsym VER_64BIT=1 --defsym MODERN=1 --defsym PORTABLE=1 -I sound -I . -o "$T/song.o" "$T/mus_vs_trainer.s" \
          && as --64 --defsym VER_64BIT=1 --defsym MODERN=1 --defsym PORTABLE=1 -I sound -I . -o "$T/vg.o" "$T/vg.s")
"$B" --render "$T/gm.mmsproj" "$T/gm.wav" 5

echo "== GBA accurate on and off"
python3 test/make_test_midi.py "$T/many.mid" 9          # 14 tracks
"$B" --save "$T/many.mid" "$T/many.mmsproj" mus_route104
"$B" --export "$T/many.mmsproj" "$T/many14.kaisong"            # GBA accurate is off by default: as is
python3 test/read_kaisong.py "$T/many14.kaisong" | grep -q "^version 1, 14 tracks"
python3 test/read_kaisong.py "$T/many14.kaisong" | grep -q "voices: 64"
sed 's/^gba_accurate=0/gba_accurate=1/' "$T/many.mmsproj" > "$T/many_gba.mmsproj"
"$B" --export "$T/many_gba.mmsproj" "$T/many10.kaisong"
python3 test/read_kaisong.py "$T/many10.kaisong" | grep -q "^version 1, 10 tracks"
"$B" --raw "$T/many.mmsproj" "$T/many.f32" 600
test -s "$T/gm.wav"

echo "== Defaults (what every opened song starts with) and Octave"
printf 'octave=1\nvolume=90\nrelease=200\n' > "$T/def.ini"
MMS_DEFAULTS="$T/def.ini" "$B" --save "$T/gm.mid" "$T/gm_def.mmsproj" mus_route104
grep -q '^volume=90$' "$T/gm_def.mmsproj"
grep -q '^octave0=1$' "$T/gm_def.mmsproj"
MMS_DEFAULTS="$T/def.ini" "$B" --dump "$T/gm.mmsproj" | cmp - "$T/a.txt"       # projects keep their own settings
mkdir -p "$T/d0" "$T/d1"
"$B" --decomp "$T/gm.mmsproj" "$T/d0"
"$B" --decomp "$T/gm_def.mmsproj" "$T/d1"
# every note an octave up, but the drums
python3 - "$T/d0/mus_route104.s" "$T/d1/mus_route104.s" <<'PY'
import re, sys
a, b = (open(p).read().split('\n') for p in sys.argv[1:3])
assert len(a) == len(b), 'the song changed shape'
up = same = 0
for x, y in zip(a, b):
    mx, my = re.search(r'\bN\d+\s*,\s*(\d+)', x), re.search(r'\bN\d+\s*,\s*(\d+)', y)
    if not mx: continue
    d = int(my.group(1)) - int(mx.group(1))
    assert d in (0, 12), 'a note moved %d' % d
    up += d == 12; same += d == 0
assert up > 20 and same > 5, (up, same)    # melodic notes up, drum notes kept
PY

echo "== Defaults: Hi-Fi for rips (a tiny Game Boy song made here)"
python3 test/make_test_gbs.py "$T/tiny.gbs"
"$B" --save "$T/tiny.gbs" "$T/tiny.mmsproj"
grep -q '^hifi=1$' "$T/tiny.mmsproj"                       # on unless the defaults say otherwise
printf 'hifi=0\n' > "$T/hifi.ini"
MMS_DEFAULTS="$T/hifi.ini" "$B" --save "$T/tiny.gbs" "$T/tiny_lofi.mmsproj"
grep -q '^hifi=0$' "$T/tiny_lofi.mmsproj"

echo "== Nintendo DS sound archive: a sequence played by the SDAT player, its pitches, intro and loop (an SDAT made here)"
python3 test/make_test_sdat.py "$T/t.sdat"
"$B" --original "$T/t.sdat#SEQ_TEST" "$T/sdat_orig.wav"
python3 - "$T/sdat_orig.wav" <<'PY'
import sys, wave, struct
w = wave.open(sys.argv[1]); sr = w.getframerate(); n = w.getnframes()
d = struct.unpack('<%dh' % (n * 2), w.readframes(n)); a = [(d[i] + d[i + 1]) / 2 for i in range(0, len(d), 2)]
assert abs(n / sr - 4.5) < 0.05, 'length %.2f s (intro 0.5 + the loop twice)' % (n / sr)
def freq(t0, t1):
    x = a[int(t0 * sr):int(t1 * sr)]
    return sum(1 for i in range(1, len(x)) if x[i - 1] < 0 <= x[i]) / (t1 - t0), max(abs(v) for v in x)
for (t0, t1), want in (((0.1, 0.4), 0), ((0.6, 0.9), 261.6), ((1.1, 1.4), 329.6), ((1.6, 1.9), 392.0), ((2.1, 2.4), 0),
                       ((2.6, 2.9), 261.6)):
    f, peak = freq(t0, t1)
    assert (peak == 0) if want == 0 else abs(f - want) < 8, '%.1f-%.1f s: %.1f Hz, peak %d (want %s)' % (t0, t1, f, peak, want)
PY
"$B" --export "$T/t.sdat#SEQ_TEST" "$T/sdat_song.kaisong" >/dev/null
python3 test/read_kaisong.py "$T/sdat_song.kaisong" >/dev/null

echo "== Sega Genesis: FM, DAC, PSG tone and noise at the emulator's levels; DAC hits that start without a seek (a VGM made here)"
python3 test/make_test_vgm.py "$T/parts.vgm"
"$B" --original "$T/parts.vgm" "$T/parts_orig.wav"
"$B" --render "$T/parts.vgm" "$T/parts_conv.wav" 7
python3 - "$T/parts_orig.wav" "$T/parts_conv.wav" <<'PY'
import sys, wave, struct, math
def load(f):
    w = wave.open(f); sr = w.getframerate(); n = w.getnframes(); ch = w.getnchannels()
    d = struct.unpack('<%dh' % (n * ch), w.readframes(n))
    return [(d[i] + d[i + 1]) / 2 for i in range(0, len(d), 2)] if ch == 2 else list(d), sr
def rms(a, sr, t0, t1):
    x = a[int(t0 * sr):int(t1 * sr)]
    return math.sqrt(sum(v * v for v in x) / max(1, len(x)))
def parts(f):
    a, sr = load(f)
    lv = [rms(a, sr, t0, t1) for t0, t1 in ((0.2, 0.8), (1.2, 1.8), (2.2, 2.8), (3.2, 3.8), (4.0, 6.0))]
    hop = int(sr * 0.005); e = [sum(abs(v) for v in a[i:i + hop]) / hop for i in range(int(3.9 * sr), int(6.1 * sr), hop)]
    hits = sum(1 for i in range(1, len(e)) if e[i] > 150 and e[i - 1] <= 150)
    return lv, hits
(o, ho), (c, hc) = parts(sys.argv[1]), parts(sys.argv[2])
assert min(c) > 0, ('a part is silent', c)
for k, name in enumerate(('DAC', 'PSG tone', 'PSG noise', 'DAC hits'), 1):
    ro, rc = o[k] / o[0], c[k] / c[0]
    assert abs(20 * math.log10(rc / ro)) < 2.5, (name, 'against FM: %.2f, the emulator %.2f' % (rc, ro))
assert hc == ho, ('DAC hits', hc, ho)
PY

echo "== GB > GBA: an arpeggio becomes its chord, a buzz of retriggers one note, duty switches one instrument (a Game Boy song made here)"
python3 test/make_test_gbs_arp.py "$T/arp.gbs"
"$B" --save "$T/arp.gbs" "$T/arp_gb.mmsproj"
printf 'gb_to_gba=1\n' > "$T/gba.ini"
MMS_DEFAULTS="$T/gba.ini" "$B" --save "$T/arp.gbs" "$T/arp_gba.mmsproj"
python3 - "$T/arp_gb.mmsproj" "$T/arp_gba.mmsproj" <<'PY'
import sys, base64, struct
def notes(p):
    t = open(p, 'rb').read().decode('latin1'); i = t.index('\nmidi=') + 6
    d = base64.b64decode(t[i:].split('\nend')[0].replace('\n', ''))
    def vlq(i):
        v = 0
        while True:
            b = d[i]; i += 1; v = v << 7 | b & 127
            if b < 128: return v, i
    i = 14; out = []
    while i < len(d):
        ln = struct.unpack('>I', d[i + 4:i + 8])[0]; j = i + 8; e = j + ln; i = e; st = 0; keys = []
        while j < e:
            _, j = vlq(j); b = d[j]
            if b == 0xFF: n, k = vlq(j + 2); j = k + n; continue
            if b & 0x80: st = b; j += 1
            if st & 0xF0 in (0xC0, 0xD0): j += 1; continue
            a, c = d[j], d[j + 1]; j += 2
            if st & 0xF0 == 0x90 and c: keys.append(a)
        out.append(keys)
    return [k for k in out if k]
gb, gba = notes(sys.argv[1]), notes(sys.argv[2])
assert max(len(k) for k in gb) > 1000, 'the GB voices keep every retrigger'
assert len(gba) == 4, gba and [len(k) for k in gba]          # the buzz, and the arpeggio's three chord notes
chord = sorted(set(k for t in gba[1:] for k in t))
assert chord == [60, 64, 67], chord
assert len(gba[0]) < 400, len(gba[0])
PY

# Rips aren't in the repository: MMS_RIPS=folder runs these on every rip in it (and MMS_BIOS=folder with a
# PlayStation 2 BIOS for PSF / PSF2). Each must convert, sound within 4.5 dB of its emulator band by
# band, export a .kaisong the reader accepts and come back the same from its project.
echo "song table: exports by place, and the SONG_TABLE.md written next to them is the release's page (tools/kaisong/gen.py)"
mkdir -p "$T/music"
"$B" --export ../../sound/songs/midi/mus_route101.mid "$T/music"
"$B" --export ../../sound/songs/midi/mus_petalburg.mid "$T/music" mus_vs_gym_leader@rustboro_city_gym
test -f "$T/music/mus_route101.kaisong" && test -f "$T/music/mus_vs_gym_leader@rustboro_city_gym.kaisong"
grep -q "place=rustboro_city_gym" "$T/music/mus_vs_gym_leader@rustboro_city_gym.kaisong"
python3 ../kaisong/gen.py --music "$T/music" --md "$T/py_table.md"
cmp "$T/music/SONG_TABLE.md" "$T/py_table.md"
grep -q '| `mus_vs_gym_leader@rustboro_city_gym.kaisong` | mus_vs_gym_leader | Rustboro City Gym |' "$T/music/SONG_TABLE.md"

if [ -n "$MMS_RIPS" ]; then
    echo "== rips in $MMS_RIPS"
    find "$MMS_RIPS" -maxdepth 2 \( -iname '*.minipsf' -o -iname '*.psf' -o -iname '*.minipsf2' -o -iname '*.miniusf' \
        -o -iname '*.m3u' -o -iname '*.mini2sf' -o -iname '*.minigsf' -o -iname '*.ish' \) ! -name '!*' | sort | awk -F/ '!seen[$(NF-1)]++' | while read -r f; do
        "$B" --render "$f" "$T/rip_kai.wav" 30
        "$B" --original "$f" "$T/rip_orig.wav"
        d=$(python3 test/compare_audio.py "$T/rip_orig.wav" "$T/rip_kai.wav" 30 10 | grep overall | awk '{print $2}')
        echo "  $(basename "$f"): $d dB"
        python3 -c "import sys; sys.exit(0 if float('$d') <= 4.5 else 1)"
        case "$f" in *.miniusf|*.usf)   # N64: steady timing (notes placed by when their audio buffer plays)
            sp=$(python3 test/compare_timing.py "$T/rip_orig.wav" "$T/rip_kai.wav" 20 | tail -1 | sed 's/.*spread (mean |lag - median|) \([0-9.]*\).*/\1/')
            echo "    timing spread $sp ms"
            python3 -c "import sys; sys.exit(0 if float('$sp') <= 10 else 1)" ;;
        esac
        case "$f" in *.mini2sf|*.2sf)   # DS: the sound keeps time with the channels (no drift)
            "$B" --render "$f" "$T/rip_kai60.wav" 60
            dr=$(python3 test/compare_timing.py "$T/rip_orig.wav" "$T/rip_kai60.wav" 60 | grep '^lag ms' | python3 -c "
import sys, statistics as st
v = [float(x) for x in sys.stdin.read().split(':')[1].split()]
print('%.0f' % abs(st.median(v[-6:]) - st.median(v[2:8])))")
            echo "    drift over a minute $dr ms"
            python3 -c "import sys; sys.exit(0 if float('$dr') <= 10 else 1)" ;;
        esac
        case "$f" in *.m3u|*.gbs)   # Game Boy: notes where the chip plays them (the emulator read as it's heard)
            sp=$(python3 test/compare_timing.py "$T/rip_orig.wav" "$T/rip_kai.wav" 30 | tail -1 | sed 's/.*spread (mean |lag - median|) \([0-9.]*\).*/\1/')
            echo "    timing spread $sp ms"
            python3 -c "import sys; sys.exit(0 if float('$sp') <= 10 else 1)" ;;
        esac
        case "$f" in *.m3u|*.gbs)   # Game Boy: Convert GB > GBA (other instruments, near the original's sound and level)
            printf 'gb_to_gba=1\n' > "$T/gb.ini"
            MMS_DEFAULTS="$T/gb.ini" "$B" --render "$f" "$T/rip_gba.wav" 30
            d=$(python3 test/compare_audio.py "$T/rip_orig.wav" "$T/rip_gba.wav" 30 10 | grep overall | awk '{print $2}')
            l=$(python3 -c "
import wave, numpy as np, sys
def rms(p):
    w = wave.open(p); a = np.frombuffer(w.readframes(min(w.getnframes(), 30 * w.getframerate())), dtype='<i2') / 32768.0
    return 20 * np.log10(np.sqrt((a ** 2).mean()) + 1e-9)
print('%.1f' % (rms('$T/rip_gba.wav') - rms('$T/rip_orig.wav')))")
            echo "    GB > GBA: $d dB, level $l dB"
            python3 -c "import sys; sys.exit(0 if float('$d') <= 6 and abs(float('$l')) <= 4 else 1)"
            MMS_DEFAULTS="$T/gb.ini" "$B" --save "$f" "$T/rip_gba.mmsproj"
            grep -q '^rip_use_gm=1$' "$T/rip_gba.mmsproj"
            "$B" --render "$T/rip_gba.mmsproj" "$T/rip_gba2.wav" 30
            cmp -s "$T/rip_gba.wav" "$T/rip_gba2.wav" ;;
        esac
        "$B" --save "$f" "$T/rip.mmsproj"
        "$B" --export "$T/rip.mmsproj" "$T/rip.kaisong"
        python3 test/read_kaisong.py "$T/rip.kaisong" > /dev/null
        "$B" --render "$T/rip.mmsproj" "$T/rip_proj.wav" 30
        cmp -s "$T/rip_kai.wav" "$T/rip_proj.wav"
    done
fi
echo "all passed"
