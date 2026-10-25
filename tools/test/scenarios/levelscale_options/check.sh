#!/bin/bash
# levelscale_options: the page's settings are saved with the rest of the ini, and HIGH put the wild one at 40-43
d=$1
grep -q "^level_scaling_rules = 3 2 1 1 0$" "$d/a.ini" || { echo "a.ini lacks level_scaling_rules = 3 2 1 1 0"; exit 1; }
grep -q "level scale: wild .* -> Lv4[0-3] (team 46" "$d/a.log" || { echo "the wild one isn't Lv40-43"; exit 1; }
echo ok
