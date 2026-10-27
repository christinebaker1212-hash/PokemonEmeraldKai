#!/bin/bash
# fight_buttons: the fight buttons were saved with the controls
d=$1
for want in "fight_pad_x = leftshoulder" "fight_pad_z = a" "fight_pad_a = rightshoulder" "fight_pad_b = b" "fight_key_x = U" "fight_key_start = Return"; do
    grep -q "^$want$" "$d/a_controls.ini" || { echo "a_controls.ini: no '$want'"; cat "$d/a_controls.ini"; exit 1; }
done
echo ok
