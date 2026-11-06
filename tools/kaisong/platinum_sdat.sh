#!/bin/bash
# Platinum's own sound data for the songs the 2SF rips lack (tools/kaisong/sinnoh_songs.py SONGS_SDAT): pret/pokeplatinum
# at SINNOH's pinned commit (tools/sinnoh/fetch_decomp.sh), only its sound sources (the songs' MIDIs, the instrument
# banks as text, the wave archives as WAVs) and its own converter, nitrosfx, which builds them into a DS sound archive
# with just those songs, their banks and waves (their sequence numbers as Platinum's). Moemon Music Studio plays it
# ("OUT.sdat#SEQ_TANKOU": src/rip_sdat.c). Nothing from it is committed; no ROM is needed.
# usage: tools/kaisong/platinum_sdat.sh OUT.sdat
set -e
OUT="${1:?usage: platinum_sdat.sh OUT.sdat}"
COMMIT=6eeb1b04c6433761f3049811b7e1383619c59743   # (SINNOH's: tools/sinnoh/fetch_decomp.sh)
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SEQS="SEQ_TANKOU SEQ_KUSAGASA SEQ_OPENING SEQ_PL_TOWN02"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"
git init -q src
cd src
git remote add origin https://github.com/pret/pokeplatinum.git
git sparse-checkout set --no-cone /tools/nitrosfx/ /res/sound/pl_sound_data.json /res/sound/BANK/ /res/sound/WAVARC/ \
    $(for s in $SEQS; do echo "/res/sound/SEQ/$s.mid"; done)
for i in 1 2 3 4; do
    git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && break
    sleep $((2 ** i))
done
git checkout -q FETCH_HEAD
cd "$WORK"
gcc -O2 -o nitrosfx src/tools/nitrosfx/*.c -lm
mkdir -p dir
# the songs' entries only (the rest emptied, so every number stays Platinum's) and the files they need
python3 -I - "$SEQS" <<'EOF'
import json, sys
j = json.load(open('src/res/sound/pl_sound_data.json'))
seqs = set(sys.argv[1].split())
banks, waves = set(), set()
for i, s in enumerate(j['seqInfo']):
    if s.get('name') in seqs:
        banks.add(s['bank'])
    else:
        j['seqInfo'][i] = {'name': ''}
for i, b in enumerate(j['bankInfo']):
    if b.get('name') in banks:
        waves.update(w for w in b['waves'] if w)
        b['fileName'] = b['name'] + '.sbnk'
    else:
        j['bankInfo'][i] = {'name': ''}
for i, w in enumerate(j['wavarcInfo']):
    if w.get('name') in waves:
        w['fileName'] = w['name'] + '.swar'
    else:
        j['wavarcInfo'][i] = {'name': ''}
j['groupInfo'] = []
json.dump(j, open('mini.json', 'w'), indent=1)
open('banks.txt', 'w').write('\n'.join(sorted(banks)) + '\n')
open('waves.txt', 'w').write('\n'.join(sorted(waves)) + '\n')
EOF
for s in $SEQS; do ./nitrosfx "src/res/sound/SEQ/$s.mid" "dir/$s.sseq"; done
while read -r b; do ./nitrosfx "src/res/sound/BANK/$b.txt" "dir/$b.sbnk"; done < banks.txt
while read -r w; do ./nitrosfx "src/res/sound/WAVARC/$w" "dir/$w.swar"; done < waves.txt
mkdir -p "$(dirname "$OUT")"
./nitrosfx dir "$OUT" mini.json -symb
echo "Platinum's sound data: $OUT ($(stat -c %s "$OUT") bytes; $SEQS)"
