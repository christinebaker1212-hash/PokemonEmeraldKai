# SINNOH's BATTLE TOWER: the full port

The user (2026-11-02): Platinum's BATTLE TOWER fully, not words on Emerald's tower. What Kai already has it uses (the
BP, the save's frontier records where they fit); everything Platinum does is Platinum's. Wi-Fi rooms stay closed
(online-only).

## Where Platinum keeps it (pret/pokeplatinum at tools/sinnoh/fetch_decomp.sh's commit; the cache is a blobless clone:
`git -C <cache> cat-file -p HEAD:<path>` for single files, `git sparse-checkout add <dir>` for folders)
- Rules: src/battle_tower.c (trainer ranges per room, PALMER, set building), src/unk_02049D08.c (the BattleTower
  struct's functions: init, opponent list, records, BP, IVs), src/unk_020494DC.c (CallBattleTowerFunction's switch),
  include/constants/battle_tower_functions.h (BT_FUNC_*), generated/battle_tower_modes.txt.
- Data: res/trainers/frontier/data/<class>_<name>.json (315 trainers: name, class, availableSets, messages
  PRE_BATTLE / WIN / DEFEAT; order generated/frontier_trainers.txt), res/trainers/frontier/pokemon/<species>_<n>.json
  (951 sets: species, moves, evFlags, nature, item, form; order frontier_pokemon.order).
- Scripts: res/field/scripts/scripts_battle_tower*.s (lobby, elevator, corridors, salon) and
  res/field/frontier_scripts/frontier_scripts_battle_tower_*.s (the battle rooms: a second script language,
  include/overlay104/frscrcmd_battle_tower.h).
- Words: res/text/battle_tower*.json (banks 29, 30...), menu entries bank 263.

## Platinum's rules (read from the C)
- Rooms of 7 battles; room n's 7th opponent from sBattleTowerBossTrainerRangesPerRoom[n], the others from
  sBattleTowerTrainerRangesPerRoom[n] (n < 7; after: range 7). Singles: battle 21 (room 2, 7th) is PALMER (silver),
  battle 49 (room 6, 7th) PALMER (gold). No trainer twice in a room.
- A trainer's party: random sets from its list, no species twice (also not the partner's), no item twice (after 50
  tries: the reserved items BRIGHTPOWDER, LUM BERRY, LEFTOVERS, QUICK CLAW). IVs by trainer number: <100 4,
  <120 8, <140 12, <160 16, <180 20, <200 24, <220 28, else 31. EVs: 510 / flagged stats, 255 at most each.
  Personality random with the set's nature, never shiny; ability by personality bit 0; friendship 255 (0 with
  FRUSTRATION); level 50.
- The player's POKéMON: 3 (singles), 4 (doubles), 2 + partner's 2 (multi); no species or item twice; the banned list;
  any level, battled at 50.
- BP: BattleTower_GiveBattlePointsReward (unk_02049D08.c:750).

## How it runs here
- The lobby, the elevator and the BATTLE SALON are Platinum's field scripts (gen_scripts.py, as every SINNOH map's).
- The corridor and the battle room are Platinum's frontier scenes (overlay104's own script language,
  res/field/frontier_scripts): gen_scripts.py FrontierConverter turns them into the game's scripts on the maps of the
  same rooms. Each scene's actors are the map's people (FRONTIER_ACTORS: the attendant, the opponent, the player who
  walks in); its pixel positions their tiles (x / 16 - 1, y rounded up / 16 - 1, checked on both rooms); its variables
  the map's VAR_TEMP_*. LaunchBattleFrontierScene and ChangeScript warp to the scene's map and go on in its script there
  (SINNOH_WARP_CONTINUE); ExitFrontier goes back to the field script after the LaunchBattleFrontierScene that started
  the scene (the elevator's: the warp to the lobby). Platinum's field versions of these rooms (left from Diamond and
  Pearl, never run by Platinum) aren't used: FRONTIER_SCENE_MAPS take no field map scripts.
- `sinnoh SINNOH_TOWER, FUNC, ARG, DESTVAR` (src/sinnoh/s3d_scrcmd.c) runs Platinum's BT_FUNC_* (src/sinnoh/s3d_tower.c
  Sinnoh_TowerCall) and the script commands around them (InitBattleTower, the intro line, the battle, CheckWonBattle,
  the party menu, the save: SINNOH_TOWER_* in include/sinnoh_tower.h).
- The records (Platinum's frontier stats and WifiBattleTowerRecord: best and last streaks, rooms cleared, the streak
  going on, the challenge REST saved, game records) are "<save>.region" chunk TOWR; the BATTLE POINTS the game's own.
- The party menu (party_menu.c, Sinnoh_Tower_Choosing): Platinum's banned list, any level (battled as level 50
  copies: the party is the player's again after each battle), the duplicate checks Platinum's script makes after.
- Closed: the Wi-Fi rooms (online), Battle Videos (no recorder: the offers to record are left out, FRONTIER_REWRITES),
  TEALA's trophies (the Underground's secret base goods). REST goes back to the title screen as Platinum's reset does
  (the PC build's soft reset quits), the game continued in the lobby.

## Steps
1. [x] Data: tools/sinnoh/gen_frontier.py -> src/data/sinnoh_frontier.h (sets, trainers: class, picture, set ids, the
       battle room sprite; the rooms' trainer ranges, PALMER's and the partners' numbers); names and lines in the text
       pack (build_text.py: a bank after the trainers').
2. [x] Engine: src/sinnoh/s3d_tower.c (the rules above, Lv 50 copies of the party, records in "<save>.region").
3. [x] gen_scripts.py: InitBattleTower, CallBattleTowerFunction (BT_FUNC_*), the rest (tower_command).
4. [x] The frontier scripts' converter (corridor, battle room; the multi ones with step 5).
5. [x] The BATTLE SALON's partners (multi battles: GetBattleTowerPartnerSpeciesAndMove, BT_FUNC_UNK_56, the multi
       corridor and battle room scenes), the multi attendant (its question about a wireless friend left out).
6. [x] The STATS JUDGE (JudgeStats). The lobby's machines are the Wi-Fi rooms' results and leaders (online: closed;
       the solo and multi results machines are Diamond and Pearl's, unused by Platinum).
7. [x] Tests (sinnoh_tower, sinnoh_tower_palmer, sinnoh_tower_rest, sinnoh_tower_double, sinnoh_tower_multi,
       sinnoh_stats_judge); Kai's own tower scripts removed.
