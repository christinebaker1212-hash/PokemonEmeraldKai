#!/bin/bash
# options_tabs: the choices were saved where they belong
d=$1
grep -q "^autosave = 10$" "$d/a.ini" || { echo "a.ini: autosave not 10"; exit 1; }
grep -q "^moenav_calls = battles$" "$d/a.ini" || { echo "a.ini: moenav_calls not battles"; exit 1; }
grep -q "^button_icons = playstation$" "$d/a_controls.ini" || { echo "a_controls.ini: button_icons not playstation"; exit 1; }
grep -q "moenav test: 5 of 5 calls would come" "$d/a.log" || { echo "a.log: before the change all 5 calls should come"; exit 1; }
echo ok
