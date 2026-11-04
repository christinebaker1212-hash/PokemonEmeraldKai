#!/bin/bash
# Moemon Maker's checker and Publish (maker/maker_publish.cpp, src/maker/maker_publish.c) against a stand-in for GitHub's
# API (tools/test/maker_github_mock.py): an added warp to nowhere is an error and Publish refuses; fixed, a first publish
# sends one file as one commit (a "Moemon-Maker:" trailer), a second publish only the changed file; the history lists
# both; rolling back to the first publishes its MoemonMaker/ again and puts its files back in the working copy.
# usage: tools/test/maker_publish.sh [BIN]
set -u
ROOT=$(realpath "$(dirname "$0")/../..")
BIN=$(realpath "${1:-$ROOT/pokeemerald64}")
T=/tmp/maker_publish_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm"
fail=0
PORT=$((20000 + RANDOM % 20000))
python3 "$ROOT/tools/test/maker_github_mock.py" $PORT testtoken > "$T/mock.log" 2>&1 &
MOCK=$!
trap 'kill $MOCK 2>/dev/null' EXIT
sleep 1
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker open MAP_LITTLEROOT_TOWN\nwait 3\nmaker token testtoken\n'
    printf 'maker mode 2\nmaker add warp 9 13\nmaker set dest MAP_ROUTE101:7\nwait 2\nmaker check\nmaker publish refused\nwait 10\n'
    printf 'maker set dest MAP_LITTLEROOT_TOWN:0\nmaker mode 0\nmaker stamp 4\nmaker paint 9 10\nwait 2\nmaker check\n'
    printf 'maker publish first\nwait 120\nmaker paint 10 10\nmaker publish second\nwait 120\n'
    printf 'maker history\nwait 60\nmaker historylist\nmaker rollback 1\nwait 150\nmaker close\nwait 30\nquit\n'
} > "$T/s.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/s.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" MOEMON_GITHUB_API="http://127.0.0.1:$PORT" MOEMON_GITHUB_REPO=owner/game \
    timeout 200 "$BIN" > "$T/run.log" 2>&1)
want() { grep -q -- "$2" "$T/$1" || { echo "FAIL: no '$2' in $1"; fail=1; }; }
want run.log "check error: LITTLEROOT TOWN: the added warp at 9, 13 leads to a warp that doesn't exist"
want run.log "publish: refused, the checker found errors"
want run.log "publish: Published 1 file"
want mock.log "main is now .*: first"
want mock.log "main is now .*: second"
want run.log "history 0: .*second"
want run.log "history 1: .*first"
want run.log "publish: Back to the publish"
want mock.log "main is now .*: Moemon Maker: back to the publish"
# (the working copy is the first publish's: tile 9,10 painted, 10,10 not)
grep -q "^block 9 10 " "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" && ! grep -q "^block 10 10 " "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" \
    || { echo "FAIL: the working copy isn't the first publish's"; fail=1; }
grep -q "Moemon-Maker:" "$T/mock.log" "$T/run.log" 2>/dev/null; true
[ $fail = 0 ] && echo "maker_publish: OK"
exit $fail
