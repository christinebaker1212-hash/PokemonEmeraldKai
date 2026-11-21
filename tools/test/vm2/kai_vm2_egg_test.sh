#!/bin/bash
# Moemon Kai: PC > VM2 the other way: the VM2 holds nothing; an EGG of CHARMANDER (DITTO's and CHARMANDER's) is
# DEPOSITed in its box; then the VM2's SAVE page holds it (stage 1, KANTO index 3) and the VM2 mark.
. "$(dirname "$0")/../common.sh"
W=${OUT:-/tmp/moemon_tests}/kai_vm2_egg
rm -rf "$W"; mkdir -p "$W/sd"
python3 "$TOOLS/vm2/make_pet_image.py" "$REPO/vmu_pet/build/pet.bin" "$W/internal.bin" 0 0 0
python3 "$REPO/vmu_pet/manager/test/fake_vm2.py" "$W/sd" "$W/internal.bin" "$W/port" &
PID=$!
trap 'kill $PID 2>/dev/null' EXIT
for i in $(seq 20); do [ -s "$W/port" ] && break; sleep 0.2; done
cat > "$W/a.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp life egg 0 1
wait 30
mp regionclear 0
mp pcmenu
pressuntil A multichoice 2000
wait 20
press DOWN 3
wait 6
press DOWN 3
wait 6
press DOWN 3
wait 6
press DOWN 3
wait 6
press A 3
wait 400
shot connected
press A 3
wait 60
press DOWN 3
wait 10
press A 3
wait 200
shot deposit
press DOWN 3
wait 10
press DOWN 3
wait 10
press A 3
wait 30
press A 3
wait 60
shot choose_box
press A 3
wait 120
shot deposited
press B 3
wait 60
press DOWN 3
wait 10
press A 3
wait 120
press B 3
wait 400
shot disconnected
press A 3
wait 60
pressuntil B field 3000
wait 30
mp party
quit
SCRIPT
TMO=200 "$TOOLS/run1.sh" "$W" MOEMON_VM2_PORT="$(cat "$W/port")" MOEMON_DEBUG_PARTY=4:15,132:15
grep -E "vm2:|party" "$W/log.txt" | head -20
python3 - "$W/internal.bin" <<'PY'
import sys
d = open(sys.argv[1], 'rb').read()
page = d[0xBB80:0xBC00]
r = page[16:16 + 39]
ok = r[0] == ord('P') and r[1] == 1 and r[2] == 3 and r[31] == 1 and page[0x78:0x7c] == b'VM2M'
print('PASS kai_vm2_egg: the EGG of CHARMANDER went to the VM2' if ok else 'FAIL kai_vm2_egg: record %s' % r[:4].hex())
sys.exit(0 if ok else 1)
PY
