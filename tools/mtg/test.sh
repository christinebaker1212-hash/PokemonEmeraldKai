#!/bin/bash
# The Magic engine's tests, built natively with the sanitizers: the card pack read whole, set-up tables for the
# translated rules, and whole duels between the stand-in AIs (they end; the same seed plays the same duel).
set -e
cd "$(dirname "$0")/../.."
OUT=${OUT:-/tmp/mtg-tests}
mkdir -p "$OUT"
# the rules and the collection only: plain C (the screens, the table and the game hooks need the game itself)
SRC="src/mtg/mtg_script.c src/mtg/mtg_valid.c src/mtg/mtg_mana.c src/mtg/mtg_game.c src/mtg/mtg_effects.c src/mtg/mtg_trigger.c src/mtg/mtg_ai.c src/mtg/mtg_sets.c src/mtg/mtg_sets_data.c
     src/mtg/mtg_collection.c src/mtg/mtg_decks.c src/mtg/mtg_text.c"
gcc -std=gnu99 -O1 -g -Wall -Wextra -Wno-unused-parameter -fsanitize=address,undefined -fno-sanitize-recover=undefined \
    -iquote include -iquote src/mtg tools/mtg/test_rules.c $SRC -o "$OUT/test_rules"
"$OUT/test_rules" graphics/mtg/forge.pak "$@"
# the printed art (graphics/mtg/printed.bin, tools/mtg/build_printed.py): every card a duel can use has its picture
# (only banned cards may go without: no printing of theirs is in Scryfall's Premodern listing)
gcc -std=gnu99 -O1 -iquote include -iquote src/mtg tools/mtg/scope_dump.c src/mtg/mtg_script.c src/mtg/mtg_valid.c \
    src/mtg/mtg_mana.c src/mtg/mtg_game.c src/mtg/mtg_effects.c src/mtg/mtg_trigger.c src/mtg/mtg_ai.c -o "$OUT/scope_dump"
"$OUT/scope_dump" graphics/mtg/forge.pak > "$OUT/scope.txt"
python3 - "$OUT/scope.txt" <<'PY'
import json, struct, sys
b = open('graphics/mtg/printed.bin', 'rb').read()
assert b[:4] == b'MTGP', 'printed.bin: not one of ours'
n = struct.unpack_from('<I', b, 4)[0]
names = set()
for i in range(n):
    no = struct.unpack_from('<I', b, 16 + i * 24)[0]
    names.add(b[no:b.index(b'\0', no)].decode())
banned = {c['name'] for c in json.load(open('tools/mtg/pool.json')) if c.get('banned')}
play = {l.split('\t')[0] for l in open(sys.argv[1]) if l.rstrip('\n').endswith('\t')}
missing = sorted(play - names - banned)
print('printed art: %d cards, %d playable without art%s' % (n, len(missing), (': ' + ', '.join(missing[:10])) if missing else ''))
sys.exit(1 if missing else 0)
PY
