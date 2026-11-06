#!/bin/bash
# Moemon Maker M2c's trainers (src/maker/maker_trainers.c, maker/maker_trainers_ui.cpp): the editor gives
# TRAINER_CALVIN_1 a party of one MAGIKARP (MoemonMaker/trainers.txt, with the game's own trainer's hash) and opens the
# TRAINERS window; a game then battles CALVIN through an event of the user's own and meets the MAGIKARP.
# usage: tools/test/maker_trainers.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_trainers_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base"
fail=0
printf 'AllowSmallTweaks = 1\n' > "$T/on.ini"
cat > "$T/edit.txt" <<'S'
pressuntil START mainmenu 3000
wait 40
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press DOWN 2
wait 6
press A 2
wait 60
maker size 1280 800
maker open MAP_LITTLEROOT_TOWN
maker trainer TRAINER_CALVIN_1 SPECIES_MAGIKARP 3
maker trainers TRAINER_CALVIN_1
maker damage SPECIES_MEWTWO 80 SPECIES_MAGIKARP 3 MOVE_PSYCHIC
maker damage SPECIES_PIKACHU 50 SPECIES_GEODUDE 50 MOVE_THUNDERBOLT
maker damage SPECIES_PIKACHU 50 SPECIES_GYARADOS 50 MOVE_THUNDERBOLT
wait 6
makershot trainers
maker close
wait 60
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/edit.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 150 "$BIN" > "$T/edit.log" 2>&1)
want() { grep -q -- "$2" "$1" 2>/dev/null || { echo "FAIL: no '$2' in $(basename "$1")"; fail=1; }; }
want "$T/mm/trainers.txt" "^trainer TRAINER_CALVIN_1 was=0x"
want "$T/mm/trainers.txt" "^  mon SPECIES_MAGIKARP 3 "
cat > "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" <<'S'
map MAP_LITTLEROOT_TOWN 20 20
add person 1 elevation=3 look=OBJ_EVENT_GFX_YOUNGSTER movement=MOVEMENT_TYPE_FACE_DOWN script=event:CALVIN x=10 y=12
event CALVIN on=added
  page trigger=talk
    battle TRAINER_CALVIN_1 "Let's battle!" "You won!"
    text "Good battle."
S
printf 'pressuntil A field 5000\nwait 30\nmash A 4 20\nmp face up\nwait 10\npressuntil A action 1500\nmp foeparty\nquit\n' > "$T/play.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 150 "$BIN" > "$T/play.log" 2>&1)
want "$T/play.log" "\[maker\] 1 edited trainer"
want "$T/play.log" "foeparty: 129:"
want "$T/edit.log" "damage SPECIES_PIKACHU -> SPECIES_GEODUDE with MOVE_THUNDERBOLT: 0-0 of [0-9]* HP, x0% STAB"
want "$T/edit.log" "damage SPECIES_PIKACHU -> SPECIES_GYARADOS with MOVE_THUNDERBOLT: [0-9]*-[0-9]* of [0-9]* HP, x400% STAB"
want "$T/edit.log" "damage SPECIES_MEWTWO -> SPECIES_MAGIKARP with MOVE_PSYCHIC: "
# the battle bench: from the editor straight into a battle with the edited CALVIN (a test game, the testing team)
{
    printf 'pressuntil START mainmenu 3000\nwait 40\n'
    for i in 1 2 3 4 5; do printf 'press DOWN 2\nwait 6\n'; done
    printf 'press A 2\nwait 60\nmaker size 1280 800\nmaker open MAP_LITTLEROOT_TOWN\nwait 4\n'
    printf 'maker bench TRAINER_CALVIN_1 1\nwaitfor field 2000\nwait 30\npressuntil A action 1500\nmp foeparty\nshot bench\nquit\n'
} > "$T/bench.txt"
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/bench.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$T/mm" timeout 200 "$BIN" > "$T/bench.log" 2>&1)
want "$T/bench.log" "\[maker\] battle bench: TRAINER_CALVIN_1"
want "$T/bench.log" "foeparty: 129:"
[ $fail = 0 ] && echo "maker_trainers: OK"
exit $fail
