#!/bin/bash
# Moemon Kai: PC > VM2 against a pretend VM2 (vmu_pet/manager/test/fake_vm2.py) holding the VMU pet with a grown
# VENUSAUR: the VM2 row shows, the VM2 is read, the box screen opens on its box, VENUSAUR is withdrawn, and the VM2's
# SAVE page then holds nothing, with the VM2 mark. Needs the game built (make linux) and vmu_pet built (make -C vmu_pet).
. "$(dirname "$0")/../common.sh"
W=${OUT:-/tmp/moemon_tests}/kai_vm2
rm -rf "$W"; mkdir -p "$W/sd"
python3 "$TOOLS/vm2/make_pet_image.py" "$REPO/vmu_pet/build/pet.bin" "$W/internal.bin" 2 2 50
python3 "$REPO/vmu_pet/manager/test/fake_vm2.py" "$W/sd" "$W/internal.bin" "$W/port" &
PID=$!
trap 'kill $PID 2>/dev/null' EXIT
for i in $(seq 20); do [ -s "$W/port" ] && break; sleep 0.2; done
cat > "$W/a.txt" <<'SCRIPT'
pressuntil A field 5000
wait 60
mp regionclear 0
mp pcmenu
pressuntil A multichoice 2000
wait 20
shot pc_menu
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
shot storage_menu
press A 3
wait 200
shot vm2_box
press RIGHT 3
wait 10
press RIGHT 3
wait 10
press DOWN 3
wait 10
shot vm2_slot
press A 3
wait 30
press A 3
wait 120
shot withdrawn
press B 3
wait 60
press DOWN 3
wait 10
press A 3
wait 120
shot storage_again
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
TMO=200 "$TOOLS/run1.sh" "$W" MOEMON_VM2_PORT="$(cat "$W/port")" MOEMON_DEBUG_PARTY=258:10
grep -E "vm2:|party" "$W/log.txt" | head -20
python3 - "$W/internal.bin" <<'PY'
import sys
d = open(sys.argv[1], 'rb').read()
page = d[0xBB80:0xBC00]
ok = page[:12] == b'MOEMONVMUPET' and page[16 + 1] == 0 and page[0x78:0x7c] == b'VM2M'
print('PASS kai_vm2: VENUSAUR came in, the VM2 holds nothing and is marked' if ok else
      'FAIL kai_vm2: stage %d mark %r' % (page[17], page[0x78:0x80]))
sys.exit(0 if ok else 1)
PY
