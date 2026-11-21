#!/bin/sh
# Moemon VMU pet: every emulator test (make test). Each test/scripts/<name>.txt script runs on two units of the image named
# by its first line's "# image: NAME" (default build/linktest.bin); OUT in a script is its result folder.
cd "$(dirname "$0")/.." || exit 1
fail=0
for s in test/scripts/*.txt; do
    name=$(basename "$s" .txt)
    out=build/test/$name
    rm -rf "$out"; mkdir -p "$out"
    img=$(sed -n 's/^# image: *//p' "$s" | head -1)
    img=${img:-build/linktest.bin}
    if [ ! -f "$img" ]; then echo "SKIP $name (no $img)"; continue; fi
    sed "s#OUT#$out#g" "$s" > "$out/script.txt"
    if build/vmutest "$img" "$img" "$out/script.txt" "$out" > "$out/log.txt" 2>&1; then
        echo "PASS $name"
        for i in "$out"/icon_*.bin; do
            [ -f "$i" ] || continue
            if python3 tools/icon_check.py "$i" "${i%.bin}.png" > "${i%.bin}.txt"; then
                echo "PASS $name $(basename "$i" .bin): $(cat "${i%.bin}.txt")"
            else
                echo "FAIL $name $(basename "$i" .bin): $(cat "${i%.bin}.txt")"; fail=1
            fi
        done
    else
        echo "FAIL $name"; grep -E "expected|unknown|can't" "$out/log.txt"; fail=1
    fi
done
[ -f build/test/linktest_send/b.bin ] && python3 tools/find_linktest.py build/test/linktest_send/b.bin | grep -q "SENT 1, GOT 2" \
    && echo "PASS linktest_save" || { echo "FAIL linktest_save"; fail=1; }
exit $fail
