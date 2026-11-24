#!/bin/bash
# sky_coop: both players' hourly weather lines (hour, place, sky) are the same. usage: check_sky_same.sh RESULT_DIR
a=$(grep -o "hour [0-9]*: region [0-9]* at [-0-9,]* flags [0-9]*: [A-Z]*" "$1/a.log" | head -1)
b=$(grep -o "hour [0-9]*: region [0-9]* at [-0-9,]* flags [0-9]*: [A-Z]*" "$1/b.log" | head -1)
echo "a: $a"; echo "b: $b"
[ -n "$a" ] && [ "$a" = "$b" ]
