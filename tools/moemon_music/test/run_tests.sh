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
test -s "$T/gm.wav"
echo "all passed"
