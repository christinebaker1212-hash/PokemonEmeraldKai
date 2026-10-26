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

# Rips aren't in the repository: MMS_RIPS=folder runs these on every rip in it (and MMS_BIOS=folder with a
# PlayStation 2 BIOS for PSF / PSF2). Each must convert, sound within 4.5 dB of its emulator band by
# band, export a .kaisong the reader accepts and come back the same from its project.
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
        "$B" --save "$f" "$T/rip.mmsproj"
        "$B" --export "$T/rip.mmsproj" "$T/rip.kaisong"
        python3 test/read_kaisong.py "$T/rip.kaisong" > /dev/null
        "$B" --render "$T/rip.mmsproj" "$T/rip_proj.wav" 30
        cmp -s "$T/rip_kai.wav" "$T/rip_proj.wav"
    done
fi
echo "all passed"
