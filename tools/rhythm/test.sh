#!/bin/bash
# The DANCE machine's simfile reader and rules on every chart (tools/rhythm/README.md). usage: tools/rhythm/test.sh [SONGS_DIR]
cd "$(dirname "$0")/../.." || exit 99
SONGS=${1:-stepmania/Songs}
B=$(mktemp -d)
trap 'rm -rf "$B"' EXIT
gcc -O2 -Wall -iquote include -o "$B/test_charts" tools/rhythm/test_charts.c src/rhythm/rh_chart.c -lm || exit 99
gcc -O2 -Wall -iquote include -o "$B/test_play" tools/rhythm/test_play.c src/rhythm/rh_play.c src/rhythm/rh_chart.c -lm || exit 99
fails=0
"$B/test_charts" "$SONGS" | tail -5 || fails=$((fails+1))
"$B/test_play" "$SONGS" | tail -5 || fails=$((fails+1))
exit $fails
