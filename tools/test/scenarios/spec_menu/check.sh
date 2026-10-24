#!/bin/bash
# spec_menu: the choices were saved (BOB's by his name), and every picture ALICE made of BOB's screen was the one he sent
d=$1
grep -q "^spectate_panel = 1" "$d/a.ini" || { echo "a.ini: spectate_panel not 1"; exit 1; }
grep -q "^spectate_friend = above 2 0 1 BOB$" "$d/a.ini" || { echo "a.ini: no spectate_friend line for BOB above 2x"; exit 1; }
python3 tools/test/check_spectrace.py 0 "$d"
