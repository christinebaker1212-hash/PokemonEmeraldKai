#!/bin/bash
# Two real game processes through the normal relay; optional latency each way.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/online}
DELAY=${1:-0}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then ./dev/build.sh linux 1; fi
python3 - "$OUT" "$DELAY" <<'PY'
import sys
from pathlib import Path
p=Path(sys.argv[1]);lag=int(sys.argv[2])
for who in ['a','b']:
    s='pressuntil A field 5000\nwaitfor online 6000\nwait 120\nmp cards starter 0\nmp cards online\nbarrier cards-ready 2\n'
    s+= 'mp cards invite 1\nwait 360\n' if who=='a' else 'wait 180\npress A 2\nwait 180\n'
    s+='rendershot online-start-'+who+'\n'
    s+=('mp cards netstep\nwait 3\n'*(600 if lag else 1800))
    if lag:
        # Finish by concession if normal play is still running. Exercise legal
        # human actions under latency without waiting for a complete AI match.
        s+=('mp cards netconcede\nmp cards netstep\nwait 30\n'*24)
    s+='rendershot online-result-'+who+'\nwait 180\nmp cards netstep\nquit\n'
    (p/(who+'.txt')).write_text(s)
PY
BIN="$REPO/pokeemerald-dev64" ENV_A='MOEMON_NO_UPDATE=1' ENV_B='MOEMON_NO_UPDATE=1' \
    FPS=60 TMO=120 tools/test/run2.sh "$OUT" "$DELAY" "$((DELAY>0?80:0))" > "$OUT/run.log" 2>&1
python3 - "$OUT" <<'PY'
import re,sys
from pathlib import Path
p=Path(sys.argv[1]);results=[]
for who in ['a','b']:
    text=(p/(who+'.log')).read_text()
    assert 'TIMEOUT' not in text and 'Segmentation' not in text,who
    states=re.findall(r'\[tcg-net\] state 4 seq (\d+) phase 3 hash ([0-9a-f]+) winner ([01])',text)
    assert states, f'{who}: duel did not finish'
    results.append(states[-1])
    assert not list(p.glob(who+'.sav*')), 'dev game wrote progress'
assert results[0]==results[1],results
assert int(results[0][0])>5, 'did not exercise synchronized card actions'
print('PASS two real clients: matching final sequence/hash/winner and no progress writes:',results[0])
PY
