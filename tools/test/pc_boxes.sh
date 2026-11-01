#!/bin/bash
# 40 PC boxes (1.63.7, src/mp/mp_boxes.c): the save holds boxes 1-14 as it always did, "<save>.boxes" the rest.
#   1. a new game puts POKeMON in boxes 1, 15 and 40, makes box 33 the one in use, saves: "<save>.boxes" written
#   2. continuing: all three back, box 33 in use (BOX33), box 14 untouched
#   3. without "<save>.boxes" (a save from before 1.63.7): boxes 1-14 as saved, 15-40 empty and named BOX15...
#   4. the save back as it was before a later save: its own boxes file from the backups ("<save>.bak1.boxes")
#   5. no backups either: the later boxes file is still read (a POKeMON is never dropped because the save fell back)
# usage: tools/test/pc_boxes.sh
. "$(dirname "$0")/common.sh"
OUT=${PC_BOXES_OUT:-/tmp/pc_boxes}; rm -rf "$OUT"; mkdir -p "$OUT"
cd "$REPO" || exit 1
run() {   # run DIR SAVE SCRIPT [ENV...]
    local d=$1 sav=$2 script=$3; shift 3
    mkdir -p "$d"; printf '%b' "$script" > "$d/a.txt"
    env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$d/a.txt" MOEMON_SHOTDIR="$d" MOEMON_SKIP_COOP_SETUP=1 \
        MOEMON_COOP_CONFIG="$d/a.ini" MOEMON_CONTROLS_CONFIG="$d/c.ini" MOEMON_SAVE="$sav" "$@" \
        timeout -k 10 300 "$BIN" > "$d/log.txt" 2>&1
}
fail=0
want() {   # want DIR TEXT
    grep -qF -- "$2" "$1/log.txt" || { echo "FAIL ($(basename "$1")): no \"$2\""; grep "^\[test\]" "$1/log.txt" | head -20; fail=1; }
}
CHECK="pressuntil A field 6000\nwait 30\nmp boxmons 1\nmp boxmons 14\nmp boxmons 15\nmp boxmons 40\nmp curbox\nquit\n"

run "$OUT/make" "$OUT/a.sav" "pressuntil A field 5000\nmp boxput 1 0 280 5\nmp boxput 15 3 252 7\nmp boxput 40 29 258 9\nmp curbox 33\nmp save\nwait 60\nquit\n" \
    MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_PARTY=255:10
want "$OUT/make" "boxput 40 29: 258"
[ -s "$OUT/a.sav.boxes" ] || { echo "FAIL: no a.sav.boxes"; fail=1; }

run "$OUT/cont" "$OUT/a.sav" "$CHECK"
want "$OUT/cont" "boxmons 1: 0:280 |"
want "$OUT/cont" "boxmons 14: |"
want "$OUT/cont" "boxmons 15: 3:252 |"
want "$OUT/cont" "boxmons 40: 29:258 |"
want "$OUT/cont" "curbox 33 BOX33"

cp "$OUT/a.sav" "$OUT/old.sav"   # (no .boxes: an older save)
run "$OUT/old" "$OUT/old.sav" "$CHECK"
want "$OUT/old" "boxmons 1: 0:280 |"
want "$OUT/old" "boxmons 15: |"
want "$OUT/old" "boxmons 40: |"
want "$OUT/old" "curbox 1 BOX1"

# another save's boxes file (this save moved on): still read
cp "$OUT/a.sav" "$OUT/b.sav"; cp "$OUT/a.sav.boxes" "$OUT/b.sav.boxes"
run "$OUT/move" "$OUT/b.sav" "pressuntil A field 6000\nmp boxput 2 0 300 5\nmp save\nwait 60\nquit\n"
cp "$OUT/a.sav" "$OUT/b.sav"   # the save back as it was, its boxes file a later one
run "$OUT/backup" "$OUT/b.sav" "$CHECK"
want "$OUT/backup" "boxmons 40: 29:258 |"
want "$OUT/backup" "boxes 15-40 loaded (a backup's)"
cp "$OUT/a.sav" "$OUT/b.sav"; rm -f "$OUT"/b.sav.bak*
run "$OUT/other" "$OUT/b.sav" "$CHECK"
want "$OUT/other" "boxmons 40: 29:258 |"
want "$OUT/other" "boxes 15-40 from the newest boxes file"

grep -l "\[crash\]" "$OUT"/*/log.txt && fail=1
[ $fail = 0 ] && echo "pc_boxes: ok"
exit $fail
