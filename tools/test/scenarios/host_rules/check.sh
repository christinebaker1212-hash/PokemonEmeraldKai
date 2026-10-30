#!/bin/bash
d=$1
grep -q "^level_scaling_rules = 2 3 0 1 1$" "$d/a.ini" || { echo "a.ini lacks level_scaling_rules = 2 3 0 1 1"; exit 1; }
echo ok
