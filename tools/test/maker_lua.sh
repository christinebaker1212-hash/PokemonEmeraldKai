#!/bin/bash
# Moemon Maker M5's Lua hooks (src/maker/maker_lua.cpp, maker_luagame.c): MoemonMaker/lua/test.lua has a function an
# event calls (`lua GiveCandy 2`: gives two RARE CANDIES, answers how many there are, `if result 2` branches on it),
# OnMapEnter (logs the map) and OnStep (counts steps into VAR_TEMP_1; the third starts the map's event STEPPED), and a
# loop that never ends (stopped by the step budget, the game carries on). The sandbox has no print / io / os.
# usage: tools/test/maker_lua.sh [BIN]
set -u
BIN=$(realpath "${1:-$(dirname "$0")/../../pokeemerald64}")
T=/tmp/maker_lua_test
rm -rf "$T"; mkdir -p "$T/shots" "$T/mm/base" "$T/mm/lua"
fail=0
cat > "$T/mm/lua/test.lua" <<'S'
local steps = 0
function GiveCandy(n)
  game.giveitem("ITEM_RARE_CANDY", n)
  return game.item("ITEM_RARE_CANDY")
end
function OnMapEnter(map)
  game.log("entered " .. map .. (print == nil and io == nil and os == nil and " sandboxed" or " open"))
end
function OnStep(x, y, map)
  steps = steps + 1
  game.setvar("VAR_TEMP_1", steps)
  if steps == 3 then return "STEPPED" end
end
function Forever()
  while true do end
end
S
cat > "$T/mm/base/MAP_LITTLEROOT_TOWN.txt" <<'S'
map MAP_LITTLEROOT_TOWN 20 20
add person 1 elevation=3 look=OBJ_EVENT_GFX_BOY_1 movement=MOVEMENT_TYPE_FACE_DOWN script=event:CANDY x=10 y=12
event CANDY on=added
  page trigger=talk
    lua GiveCandy 2
    if result 2
      log 1
    end
    lua Forever
    log 3
event STEPPED on=map
  page trigger=talk
    log 2
S
cat > "$T/play.txt" <<'S'
pressuntil A field 5000
wait 20
mp face up
wait 10
pressuntillog A 900 maker event: line 3 ran
mash B 6 10
wait 30
press DOWN 20
wait 20
press DOWN 20
wait 20
press DOWN 20
wait 60
mash B 6 10
wait 20
mp checkitem 68
quit
S
(cd "$T" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$T/play.txt" \
    MOEMON_SHOTDIR="$T/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$T/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_DEBUG_WARP=0,9,10,13 MOEMON_MAKER_DIR="$T/mm" MOEMON_SAVE="$T/play.sav" timeout 200 "$BIN" > "$T/play.log" 2>&1)
grep "maker lua\|maker event: line" "$T/play.log" | head
want() { grep -q -- "$1" "$T/play.log" || { echo "FAIL: no '$1' in play.log"; fail=1; }; }
want "\[maker\] lua: 1 script"
want "maker lua: entered MAP_LITTLEROOT_TOWN sandboxed"
want "maker event: line 1 ran"
want "maker lua: .*ran too long"
want "maker event: line 3 ran"
want "maker event: line 2 ran"
want "\[test\] item 68: 2"
# battles: OnDamageCalc makes the player's hits do 1 damage, OnEndTurn logs each turn (a wild ZIGZAGOON, SCRATCH)
B="$T/battle"; mkdir -p "$B/mm/lua" "$B/shots"
cat > "$B/mm/lua/battle.lua" <<'S'
function OnDamageCalc(attacker, target, move, damage, side)
  if side == 0 then
    game.log("hit " .. attacker .. " " .. move .. " was " .. damage)
    return 1
  end
end
function OnEndTurn(turn)
  game.log("turn " .. turn)
end
S
cat > "$B/play.txt" <<'S'
pressuntil A field 5000
mp teach 0 10
mp wild 288:5
pressuntil A action 3000
wait 10
press A 2
wait 20
press A 2
pressuntillog A 3000 maker lua: turn 1
wait 30
quit
S
(cd "$B" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$B/play.txt" \
    MOEMON_SHOTDIR="$B/shots" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$B/play.ini" MOEMON_DEBUG_QUICKSTART=1 \
    MOEMON_MAKER_DIR="$B/mm" MOEMON_SAVE="$B/play.sav" timeout 200 "$BIN" > "$B/play.log" 2>&1)
grep "maker lua" "$B/play.log" | head -4
grep -q "maker lua: hit SPECIES_[A-Z]* MOVE_SCRATCH was [0-9]*" "$B/play.log" || { echo "FAIL: OnDamageCalc didn't see the player's SCRATCH"; fail=1; }
grep -q "maker lua: turn 1" "$B/play.log" || { echo "FAIL: OnEndTurn didn't run"; fail=1; }

# the editor: Tools > Lua scripts makes a new script, checks a broken text and a fixed one, saves it
E="$T/editor"; mkdir -p "$E/mm"
printf 'AllowSmallTweaks = 1\n' > "$E/on.ini"
cat > "$E/play.txt" <<'S'
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
maker lua new hooks
wait 5
maker lua text function OnStep(x, y\n  return nil\nend
maker lua check
maker lua text function OnStep(x, y, map)\n  game.setvar("VAR_TEMP_1", x)\nend
maker lua check
maker lua save
wait 10
makershot lua
wait 5
quit
S
(cd "$E" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_NO_UPDATE=1 MOEMON_TEST="$E/play.txt" \
    MOEMON_SHOTDIR="$E" MOEMON_SKIP_COOP_SETUP=1 MOEMON_COOP_CONFIG="$E/on.ini" MOEMON_MAKER_UNLOCK=1 \
    MOEMON_MAKER_DIR="$E/mm" timeout 200 "$BIN" > "$E/play.log" 2>&1)
grep -q "lua check: .*expected" "$E/play.log" || { echo "FAIL: the broken script wasn't caught"; fail=1; }
grep -q "lua check: $" "$E/play.log" || { echo "FAIL: the fixed script wasn't read fine"; fail=1; }
grep -q 'game.setvar("VAR_TEMP_1", x)' "$E/mm/lua/hooks.lua" 2>/dev/null || { echo "FAIL: the script wasn't saved"; fail=1; }
grep -q "\[maker\] shot .*lua.bmp ok" "$E/play.log" || { echo "FAIL: no picture"; fail=1; }
[ $fail = 0 ] && echo "maker_lua: OK"
exit $fail
