#!/bin/bash
# Each game's own emotes are the other's "friend's emotes", on the same battler and icon, never one of its own battlers
d="$1"
own() { grep -o "life battle: emote [0-9]* on battler [0-9]*" "$d/$1.log" | sed 's/.*emote \([0-9]*\) on battler \([0-9]*\)/\1 \2/' | sort -u; }
friend() { grep -o "a friend's emote [0-9]* on battler [0-9]*" "$d/$1.log" | sed 's/.*emote \([0-9]*\) on battler \([0-9]*\)/\1 \2/' | sort -u; }
ok=1
for x in a b; do y=$([ $x = a ] && echo b || echo a)
    [ -n "$(own $x)" ] || { echo "$x: no emote of its own"; ok=0; }
    [ "$(own $x)" = "$(friend $y)" ] || { echo "$x's emotes [$(own $x)] != what $y saw [$(friend $y)]"; ok=0; }
    for b in $(own $x | cut -d' ' -f2); do
        friend $x | cut -d' ' -f2 | grep -qx "$b" && { echo "$x: a friend's emote on its own battler $b"; ok=0; }
    done
done
[ $ok = 1 ] && echo "emotes: each on its own MOEMON on both games"
[ $ok = 1 ]
