#!/bin/sh
# Moemon Kai: the game's VM2 link (src/platform/vm2.c) against a pretend VM2 on a pseudo-terminal (Linux).
set -e
cd "$(dirname "$0")/../../.."
W=$(mktemp -d)
trap 'kill $PID 2>/dev/null; rm -rf "$W"' EXIT
python3 -c "import os; open('$W/internal.bin','wb').write(os.urandom(131072))"
mkdir -p "$W/sd"
python3 vmu_pet/manager/test/fake_vm2.py "$W/sd" "$W/internal.bin" "$W/port" &
PID=$!
for i in 1 2 3 4 5 6 7 8 9 10; do [ -s "$W/port" ] && break; sleep 0.2; done
gcc -O1 -Wall -iquote include tools/test/vm2/vm2_test.c src/platform/vm2.c -o "$W/vm2_test"
MOEMON_VM2_PORT=$(cat "$W/port") "$W/vm2_test" "$W/internal.bin"
