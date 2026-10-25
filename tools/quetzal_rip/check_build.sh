#!/usr/bin/env bash
# Assemble everything in quetzal_rip/ with Kai's own tools (mapjson, preproc, cpp, as) and,
# given the ROM, compare every labelled script / text / movement / mart block byte for byte.
#   tools/quetzal_rip/check_build.sh [QUETZAL.gba]
#   QZ_RIP=dir tools/quetzal_rip/check_build.sh       (assemble another rip tree; no byte check)
# Writes into $TMPDIR (or /tmp); in the repo it only builds Kai's tools and `make generated` headers.
set -e -o pipefail
cd "$(dirname "$0")/../.."
ROM="$1"
R="${QZ_RIP:-quetzal_rip}"   # another rip tree: e.g. a copy of quetzal_rip with quetzal_rip/gen4/overlay over it
T="${TMPDIR:-/tmp}/quetzal_check"
rm -rf "$T"; mkdir -p "$T/kai" "$T/qz/maps" "$T/inc/constants"
for t in preproc mapjson jsonproc; do make -s -C tools/$t >/dev/null; done
make -s generated >/dev/null    # Kai's own generated headers (map_groups.h, heal_locations.h...; gitignored)
M=tools/mapjson/mapjson
$M layouts emerald $R/layouts/layouts.json "$T/qz" "$T/qz"
$M groups emerald $R/maps/map_groups.json "$T/qz" "$T/qz"
n=0
for d in $R/maps/*/; do
  b=$(basename "$d"); mkdir -p "$T/qz/maps/$b"
  $M map emerald "$d/map.json" $R/layouts/layouts.json "$T/qz/maps/$b" >/dev/null
  n=$((n+1))
done
echo "mapjson: $n map.json files converted"
# Quetzal's groups are 34-39; mapjson numbers them from 0
python3 -c "import re,sys; s=open(sys.argv[1]).read(); s=re.sub(r'\((\d+) << 8\)', lambda m: '(%d << 8)' % (int(m.group(1)) + 34), s); s=s.replace('GUARD_CONSTANTS_MAP_GROUPS_H', 'GUARD_QZ_MAP_GROUPS_H'); s=re.sub(r'#define MAP_GROUPS_COUNT.*', '', s); open(sys.argv[2], 'w').write(s)" "$T/qz/map_groups.h" "$T/inc/qz_map_groups.h"
{
  sed -n '1,61p' data/event_scripts.s
  echo '#include "constants/event_bg.h"'
  echo '#include "constants/trainer_types.h"'
  echo '#include "qz_map_groups.h"'
  echo '#include "quetzal_constants.h"'
  [ -n "$ROM" ] && echo '#include "quetzal_ids.h"'   # Quetzal's numbers, for the byte check
  echo "	.include \"$R/include/quetzal_macros.inc\""
  for f in $R/scripts/*.inc $R/maps/*/scripts.inc; do echo "	.include \"$f\""; done
  for d in $R/maps/*/; do echo "	.include \"$T/qz/maps/$(basename "$d")/events.inc\""; done
} > "$T/test.s"
tools/preproc/preproc "$T/test.s" charmap.txt \
  | cpp -I "$T/inc" -I include -I $R/include - \
  | tools/preproc/preproc -ie "$T/test.s" charmap.txt > "$T/test.pp.s"
as --32 --defsym VER_64BIT=0 --defsym MODERN=0 --defsym PORTABLE=1 --defsym UBFIX=1 -o "$T/test.o" "$T/test.pp.s"
echo "assembled: $T/test.o"
nm -u "$T/test.o" | awk '{print $2}' | sed -E 's/_[0-9A-F]+$//' > "$T/undef.txt"
echo "undefined symbols (to map onto Kai): $(nm -u "$T/test.o" | wc -l)"
nm -u "$T/test.o" | awk '{print $2}' | sed -E 's/^([A-Z]+_?[A-Z]*)_.*/\1/' | sort | uniq -c | sort -rn | head -20
if [ -n "$ROM" ]; then python3 tools/quetzal_rip/verify_bytes.py "$ROM" "$T/test.o"; fi
