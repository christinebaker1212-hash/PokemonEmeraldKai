# SINNOH's BATTLE TOWER: the full port (in progress)

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

## Steps
1. [x] Data: tools/sinnoh/gen_frontier.py -> src/data/sinnoh_frontier.h (sets, trainers: class, picture, set ids);
       names and lines in the text pack (build_text.py: a bank after the trainers').
2. [ ] Engine: src/sinnoh/s3d_tower.c (the rules above, Lv 50 copies of the party, records in "<save>.region").
       Done: Sinnoh_Frontier_FillParty (Platinum's party building), the battle ids TRAINER_SINNOH_FRONTIER_A / _B
       (include/sinnoh_tower.h) answered by battle_tower.c / frontier_util.c / s3d_gfx.c (name, class, picture,
       Platinum's battle picture, the three lines); new classes TOWER TYCOON ... WAITER (gen_trainers.py NEW).
3. [ ] gen_scripts.py: InitBattleTower, CallBattleTowerFunction (BT_FUNC_*), SetBattleTowerNull, the rest.
4. [ ] The frontier scripts' converter (battle rooms, corridors, multi room).
5. [ ] The BATTLE SALON's partners (multi battles), PALMER.
6. [ ] The records machine (OpenBattleTowerRecordsApp).
7. [ ] Tests; remove data/maps/Sinnoh_BattleTower*'s own scripts.
