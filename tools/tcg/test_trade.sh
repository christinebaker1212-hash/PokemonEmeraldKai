#!/bin/bash
# Two real clients trade a spare card: ALICE asks, BOB says YES, both offer, both confirm.
set -euo pipefail
cd "$(dirname "$0")/../.."
REPO=$PWD
OUT=${OUT:-$REPO/dev/test-results/card-trade}
mkdir -p "$OUT"
if [ -z "${NOBUILD:-}" ]; then make linux -j"$(nproc)"; fi
cat > "$OUT/a.txt" <<'SCRIPT'
pressuntil A field 5000
waitfor online 6000
wait 120
mp cards starter 0
mp cards own 50 2
barrier ready 2
mp cards ask 1 1
pressuntil A cards 1500
wait 30
press A 2
wait 120
rendershot trade-offers-a
press START 2
wait 120
rendershot trade-done-a
mp cards state
press A 2
wait 60
quit
SCRIPT
cat > "$OUT/b.txt" <<'SCRIPT'
pressuntil A field 5000
waitfor online 6000
wait 120
mp cards starter 1
mp cards own 67 3
barrier ready 2
wait 90
pressuntil A cards 1500
wait 60
press A 2
wait 60
press START 2
wait 120
rendershot trade-done-b
mp cards state
press A 2
wait 60
quit
SCRIPT
BIN="$REPO/pokeemerald64" ENV_A='MOEMON_NO_UPDATE=1' ENV_B='MOEMON_NO_UPDATE=1' \
    TMO=150 tools/test/run2.sh "$OUT" > "$OUT/run.log" 2>&1
grep -q 'asked player 1 for a card trade' "$OUT/a.log"
grep -q 'traded card 50 for 67' "$OUT/a.log"
grep -q 'traded card 67 for 50' "$OUT/b.log"
echo 'PASS a card trade between two players: each got the other card'
