#!/bin/bash
# Two real clients play a whole online card duel through the duel screen:
# the board's own moves, animations and acknowledgement waits on both sides.
# The winner gets a booster pack, the loser loses nothing.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/online-board}
DELAY=${1:-0}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then make linux -j"$(nproc)"; fi
for who in a b; do
    {
        printf 'pressuntil A field 5000\nwaitfor online 6000\nwait 120\nmp cards starter %s\nmp cards fast\nbarrier cards-ready 2\n' \
            "$([ $who = a ] && echo 0 || echo 1)"
        if [ $who = a ]; then printf 'mp cards ask 1 0\npressuntil A cards 1500\nwait 30\npress A 2\nwait 300\n'
        else printf 'wait 90\npressuntil A cards 1500\nwait 120\npress A 2\nwait 210\n'; fi
        printf 'mp cards auto\nrendershot board-start-%s\n' $who
        for i in $(seq 1 40); do printf 'wait 600\nmp cards netstate\n'; done
        printf 'rendershot board-end-%s\nmp cards state\nquit\n' $who
    } > "$OUT/$who.txt"
done
BIN="$REPO/pokeemerald64" ENV_A='MOEMON_NO_UPDATE=1' ENV_B='MOEMON_NO_UPDATE=1' \
    FPS=60 TMO=600 tools/test/run2.sh "$OUT" "$DELAY" "$((DELAY>0?80:0))" > "$OUT/run.log" 2>&1
python3 - "$OUT" <<'PY'
import re,sys
from pathlib import Path
p=Path(sys.argv[1]);results=[];after={}
for who in ['a','b']:
    text=(p/(who+'.log')).read_text()
    assert 'TIMEOUT' not in text and 'Segmentation' not in text,who
    states=re.findall(r'\[tcg-net\] state 4 seq (\d+) phase 3 hash ([0-9a-f]+) winner ([01])',text)
    assert states, f'{who}: duel did not finish'
    results.append(states[-1])
    assert 'show event 20' in text, f'{who}: the board never showed the result'
    after[who]=re.findall(r'\[tcg\] screen \d+ .* wins (\d+) .* packs (\d{4}) ',text)[-1]
assert results[0]==results[1],results
assert int(results[0][0])>20, 'too few synchronized moves'
# seat 0 is the asker (ALICE)
winner='a' if results[0][2]=='0' else 'b'
loser='b' if winner=='a' else 'a'
assert after[winner][0]=='1' and sum(map(int,after[winner][1]))==1, after
assert after[loser][0]=='0' and after[loser][1]=='0000', after
print('PASS online duel played through both duel screens: matching sequence/hash/winner:',results[0],
      '- one booster pack for the winner only')
PY
