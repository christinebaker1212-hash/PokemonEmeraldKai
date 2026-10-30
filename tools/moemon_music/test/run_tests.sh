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
