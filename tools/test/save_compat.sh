#!/bin/bash
# Saves from every released layout load as the same game (include/mp_savefix.h):
#   today's / 1.59's layout without a region side file (format 0), and 1.60.0's on Linux and on Windows (format 1:
#   tools/test/save_variants.py writes them from today's save, the side file without its SAVF chunk).
# Each one is loaded, its summary (mp savesum: SaveBlock1's bytes, badges, money, object events, templates) must
# match the original's, and the game must continue into the overworld without a crash.
# usage: tools/test/save_compat.sh [SAVE]   (SAVE: a save to use instead of one made here; no side file)
#   BIN=pokeemerald64.exe runs the Windows build under Wine (SDL2.dll next to it).
. "$(dirname "$0")/common.sh"
OUT=${OUT:-/tmp/save_compat}; rm -rf "$OUT"; mkdir -p "$OUT"
cd "$REPO" || exit 1
W=""; RUNNER=()
case "$BIN" in *.exe)
    command -v wine >/dev/null || { echo "wine not installed"; exit 99; }
    export WINEDEBUG=-all WINEPREFIX="${WINEPREFIX:-$OUT/wineprefix}"; W="Z:"; RUNNER=(wine) ;;
esac
run() {   # run DIR SAVE SCRIPT [ENV...]
    local d=$1 sav=$2 script=$3; shift 3
    mkdir -p "$d"; printf '%b' "$script" > "$d/a.txt"
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$W$d/a.txt" MOEMON_SHOTDIR="$W$d" MOEMON_SKIP_COOP_SETUP=1 \
        MOEMON_COOP_CONFIG="$W$d/a.ini" MOEMON_CONTROLS_CONFIG="$W$d/c.ini" MOEMON_SAVE="$W$sav" "$@" \
        timeout -k 10 300 "${RUNNER[@]}" "$BIN" > "$d/log.txt" 2>&1
}
sum() { grep -m1 "^\[test\] savesum:" "$1/log.txt" | sed 's/^\[test\] savesum: //'; }
fail=0
if [ -n "$1" ]; then
    cp "$1" "$OUT/base.sav"
else
    # a save of today's: a few badges, money, a party, a walk, saved
    run "$OUT/make" "$OUT/base.sav" "pressuntil A field 5000\nmp setflag 0x867\nmp setflag 0x868\nmp setflag 0x869\nmp money 12345\npress DOWN 20\npress RIGHT 30\nwait 30\nmp save\nwait 60\nquit\n" \
        MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_PARTY=280:20,277:15
    rm -f "$OUT/base.sav.region"
fi
# the reference: today's build reading it (format 0: no side file), then saving it (a side file with SAVF)
run "$OUT/ref" "$OUT/ref.sav" "wait 200\nmp savesum\nquit\n"   # (a placeholder run so ref.sav exists)
cp "$OUT/base.sav" "$OUT/ref.sav"; rm -f "$OUT/ref.sav.region"
run "$OUT/ref" "$OUT/ref.sav" "wait 200\nmp savesum\nmp save\nwait 60\nquit\n"
REF0=$(sum "$OUT/ref")
[ -n "$REF0" ] || { echo "FAIL: no reference summary"; exit 1; }
grep -q "save: format 0 -> 2" "$OUT/ref/log.txt" || { echo "FAIL: format 0 not migrated"; fail=1; }
# today's save as today's build wrote it (its side file has the format): read again, the same game
run "$OUT/ref2" "$OUT/ref.sav" "wait 200\nmp savesum\nquit\n"
REF=$(sum "$OUT/ref2"); echo "reference: $REF"
if [ "$REF" = "$REF0" ]; then echo "ok   format 0 (no side file)"; else echo "FAIL format 0: $REF0"; fail=1; fi
grep -q "save: format" "$OUT/ref2/log.txt" && { echo "FAIL: today's save migrated again"; fail=1; }
# 1.60.0's side file: today's without the SAVF chunk
python3 - "$OUT/ref.sav.region" "$OUT/v160.region" <<'PY'
import struct, sys
d = open(sys.argv[1], 'rb').read()
magic, fmt, fp, n = struct.unpack_from('<4I', d, 0)
out, o = bytearray(d[:16 + n]), 16 + n
while o + 8 <= len(d):
    tag, ln = struct.unpack_from('<2I', d, o)
    if tag != struct.unpack('<I', b'SAVF')[0]:
        out += d[o:o + 8 + ln]
    o += 8 + ln
open(sys.argv[2], 'wb').write(out)
PY
for kind in linux160 win160; do
    python3 tools/test/save_variants.py "$OUT/ref.sav" "$OUT/$kind.sav" $kind || fail=1
    cp "$OUT/v160.region" "$OUT/$kind.sav.region"
    run "$OUT/$kind" "$OUT/$kind.sav" "wait 200\nmp savesum\npressuntil A field 6000\nwait 120\nmp badges\nshot cont\nquit\n"
    S=$(sum "$OUT/$kind")
    if [ "$S" = "$REF" ]; then echo "ok   $kind"; else echo "FAIL $kind: $S"; fail=1; fi
    grep -q "save: format 1 -> 2" "$OUT/$kind/log.txt" || { echo "FAIL $kind: not migrated from format 1"; fail=1; }
    grep -q "\[crash\]\|crash report" "$OUT/$kind/log.txt" && { echo "FAIL $kind: crashed"; fail=1; }
done
exit $fail
