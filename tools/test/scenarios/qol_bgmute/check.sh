#!/bin/bash
# qol_bgmute: the option was saved
grep -q "^mute_in_background = 1" "$1/a.ini" || { echo "a.ini lacks mute_in_background = 1"; exit 1; }
