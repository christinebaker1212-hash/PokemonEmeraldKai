# Triple Battles: progress log

Working notes while triples are built (plan: TRIPLES_PLAN.md, decisions: TRIPLES_DECISIONS.md).
Each step ends with a commit; the newest state is at the bottom.

## Order of work
1. Engine capacity: MAX_BATTLERS_COUNT 6 (recorded battles stay 4), exec-flag strides, faint
   markers, infatuation / Leech Seed sources, Palace flags, Mirror Move strides. Singles/doubles unchanged.
2. Local test triple (debug: the player commands all three of 0/2/4 against 1/3/5) to bring up the
   engine, the six-sprite layout, the cards and targeting without the network.
3. Battle rules: targeting resolver (any target, spread 3/4, Helping Hand either ally), partner
   lookups (XOR 2) replaced where they can meet battlers 4/5, faint/replacement, win/loss, AI.
4. Three-client link: NetLink for 3 participants (per-sender queues, readiness, IS_DONE per player).
5. Matchmaking: trainers (original party 3+, host starts doubles or waits), wild (up to 2 friends),
   2-Pokémon pick (co-op team 1-2).
6. Items, balls (target picker over three), catches, EXP per owner, settlement, disconnects.
7. Presentation polish: cards, owner strips, team glance (R), intents for two allies, emotes.
8. Tests (three-client runner), docs, Windows build.

## Log
- Step 1 done: MAX_BATTLERS_COUNT = 6 (B_BATTLER_4/5), SINGLE/DOUBLE/TRIPLE_BATTLERS_COUNT 2/4/6,
  RECORDED_BATTLERS_COUNT 4 (recorded_battle.c ignores battlers 4-5), BATTLER_SLOTS (4, or 6 in a triple) for
  "every battler" loops (anims, CountAliveMonsInBattle), exec flags EXEC_FLAGS_PLAYER_SHIFT = player * 6 and
  EXEC_FLAGS_OUTBOUND_MASK (bits 26-31), gBattlerFaintMarks (was gHitMarker bits 28-31), infatuation =
  STATUS2_INFATUATION bit 16 + gBattlerInfatuatedWith[], Leech Seed source in STATUS3 bits 21-23
  (STATUS3_LEECHSEED_FROM/SOURCE), palaceFlags u16, LAST_TAKEN_MOVE_FROM_STRIDE, BATTLER_SPRITE_PALETTES().
  Doubles still set gBattlersCount = DOUBLE_BATTLERS_COUNT. Full regression passes.
- Step 2 (local bring-up) working: `mp triple S:L S:L S:L` (mp_debug.c) starts a local wild triple
  (battle_setup.c BattleSetup_StartLocalTripleWildBattle, gIsTripleBattle, BATTLE_TYPE_DOUBLE). Positions
  B_POSITION_PLAYER_CENTER (battler 4) / OPPONENT_CENTER (5); coords row 2 in battle_anim_mons.c sBattlerCoords;
  gBattlerSpriteTemplates[4/5]; SetBattlePartyIds checks every earlier battler on the side (IsPartyIdTakenBySide).
  Cards: src/mp/mp_tricards.c drawn over the frame from VDraw (sdl2.c), healthbox objects parked at x=280
  (InitBattlerHealthboxCoords), data from UpdateHealthboxAttribute / MoveBattleBar, palette TAG 0x53C8 (reloaded
  in reshow_battle_screen.c). Frame pixels are ABGR1555 (bit 15 = opaque). BATTLE_PARTNER(4/5) = 0/1.
  Choosing order: battle_main.c PrevChooser/HasNextChooser (left, center, right). gBattleCommunication's shared
  entries moved to 8-11 (ACTIONS_CONFIRMED_COUNT etc.): 0-5 are the battlers' selection states.
  Local triple: intro sends out all three, turn runs, KOs, back to field. Full regression passes.
- Step 3 (rules): any target (player_controller.c HandleInputChooseTargetTriple: left to right over the six,
  skipping absent ones; foes first), Helping Hand picks either ally (MpIsAllyPickMove; Cmd_trysethelpinghand
  uses the chosen ally), spread moves 3/4 with 2+ targets (pokemon.c SpreadDamage, snapshot in
  gMpSpreadTargetCount), every spread target in turn (battle_script_commands.c MpNextSpreadTarget),
  HandleAction_UseMove's target (battle_util.c MpTripleChooseTarget: Follow Me, Lightning Rod, random foe for a
  gone target), replacements (HasNoMonsToSwitch / MP_IsPartyIdOnSide / MP_TripleSwitchRange: each linked player
  owns two slots, the NPC side shares six; every fainted battler chooses at once like a multi battle; a local
  debug triple sends replacements in by itself), AI targets any of three, the third ally for Heal Bell /
  Aromatherapy / status icons, Future Sight, foe EXP slots (gSentPokesToOpponent[battler >> 1]),
  AI_itemType/Flags per trainer slot, "Wild X, Y and Z appeared!". Partner math `^ BIT_FLANK` that can meet
  battlers 4-5 now uses BATTLE_PARTNER (battle_anim.c Cmd_monbg made battler 7 and hid sprite 0), and the center
  foe gets its own shadow sprite (LoadAndCreateEnemyShadowSprites).
- Step 4 (three games): mp_battle.c NetLink roster (MP_Battle_OpenLinkGroup; per-link queues, sync sequences,
  ready mask, every other game's LinkPlayer), battle start 2+2+2 (battle_main.c CB2_HandleStartMultiPartnerBattle:
  AllLinkBlocksReceived, link player i's two in slots 2i-2i+1, gLinkPlayers ids 0/2/4, the NPC is link player 3),
  controllers (battle_controllers.c: battler 2i = link player i, master runs 1/3/5), intro send-outs for the
  center battlers, trainer back pictures left/center/right (MpTripleTrainerX), "BOB sent out X!" lines.
- Step 5 (matchmaking, mp_coop.c seats): wild = the two closest friends in the grass are asked at once (two
  yeses: triple with up to three wild, one: double, none: alone); trainers with 3+ authored Pokémon (single
  battles only) keep waiting for a third after the first friend joins ("{B} start now" = double). ACCEPT and
  WILD_GO carry the seat list. Results, bonuses, catches, refunds and level-ups per seat; pair counters for every
  pair (MP_Events_OnCoopWinGroup). Protocol 22. tools/test/runN.sh runs 2-4 clients.
  Verified: three clients on Route 101, wild triple to the win, EXP per owner, all three back in the field.
