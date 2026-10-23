#!/bin/bash
# spec_menu: the choices were saved, and every picture ALICE made of BOB's screen was the one he sent
d=$1
grep -q "^spectate_panel = 1" "$d/a.ini" || { echo "a.ini: spectate_panel not 1"; exit 1; }
grep -q "^spectate_size = 2" "$d/a.ini" || { echo "a.ini: spectate_size not 2"; exit 1; }
python3 tools/test/check_spectrace.py 0 "$d"
