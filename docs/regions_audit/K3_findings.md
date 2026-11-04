# K3 findings: the Sevii Islands, Mt. Ember, Navel Rock, Birth Island, Trainer Tower (vs FireRed)

Method: every map's scripts.inc text and commands diffed against pokefirered's scripts.inc + text.inc (plus
pokefirered data/scripts/*.inc for the people whose scripts live there, and pokefirered's data/text/trainers.inc for
the VS Seeker rematch lines, fetched from GitHub because the local clone lacks data/text); map.json objects, item balls,
hidden items, signs and triggers compared by position. Helper scripts: scratchpad/audit/tmp_K3/cmp.py, ev.py.

Notes on the maps themselves:
- The Seven Island misnames are already fixed in the game: Kanto_SevenIsland_House_Room1 / _House_Room2 / _Mart /
  _PokemonCenter_1F / _PokemonCenter_2F match FireRed's maps of the same names (layouts, objects, scripts); FireRed's
  SevenIsland_UnusedHouse has no counterpart (it is unused in FireRed too).
- Every map in K3_maps.txt has a FireRed source; none of them is a map Quetzal added.
- Trainers: every map has FireRed's number of trainers, and all their intro / defeat / post-battle lines match. The
  rematch intros (TRAINER_BATTLE_REMATCH) are FireRed's own VS Seeker rematch lines (pokefirered data/text/trainers.inc),
  not Quetzal's.
- Not reported: Quetzal's QzReturnZero / QzQuestLogNop leftovers (harmless), the dead NUZLOCKE gift check (always
  false), braille strings (the 6 leading bytes are the format header Emerald's braillemessage skips).

---

### K3-1: Delivering the SAPPHIRE clears KANTO's GAME CLEAR and CHAMPION flags
- Maps: Kanto_OneIsland_PokemonCenter_1F (and everything gated on the flag: the SEAGALLOP to ISLANDS 4-7, Kanto_MtEmber_Exterior, Kanto_TwoIsland, the ELITE FOUR rooms)
- Category: A
- Original: pokefirered data/maps/OneIsland_PokemonCenter_1F/scripts.inc:275 `special SetPostgameFlags` (sets the postgame / link flags; nothing is cleared), then :280 `special InitRoamer`.
- Now: data/maps/Kanto_OneIsland_PokemonCenter_1F/scripts.inc:251 `special QzClearSeagallopRestrictions`, which is src/qz_specials.c:285-290 `FlagClear(FLAG_QZ_0x2108); FlagClear(FLAG_QZ_0x2109);`. In this port 0x2108 is KANTO's FLAG_SYS_GAME_CLEAR and 0x2109 FLAG_DEFEATED_CHAMP (set by Kanto_PokemonLeague_HallOfFame / ChampionsRoom; kanto_frlg_flags.json). After CELIO gets the SAPPHIRE:
  - the ferry refuses ISLANDS 4-7 ("To visit ISLANDS 4, 5, 6, and 7, you must win the KANTO LEAGUE.": data/scripts/qz/kanto_shared.inc:1492-1495) until the League is beaten again;
  - MT. EMBER's RUBY PATH entrance closes again (Kanto_MtEmber_Exterior/scripts.inc:22) and its grunts go back to their pre-League lines (:29, :70);
  - TWO ISLAND's shop says "Come back after winning KANTO's League" (Kanto_TwoIsland/scripts.inc:91);
  - CELIO falls back to "I'm modifying my Network Machine" (Kanto_OneIsland_PokemonCenter_1F/scripts.inc:169 -> 8421040) instead of FireRed's post-SAPPHIRE line;
  - the ELITE FOUR use their first-run teams again (their rooms' `call_if_set FLAG_QZ_0x2108`), DUNSPARCE TUNNEL's layout reverts, OAK's lab / RIVAL's house postgame lines revert.
- Restore: make the special do nothing (the script already sets FLAG_SYS_CAN_LINK_WITH_RS 0x220A, the CERULEAN CAVE guard and LORELEI flags itself, as SetPostgameFlags would); never clear 0x2108 / 0x2109.
- Notes: Quetzal's C did clear "0x2108/0x2109" (docs/quetzal/SPECIALS.md row 750) but its flag ids there can't be the script flags these became; the port copied it literally. Not in REGIONS_TODO. Confidence high. Saves that already passed this point need 0x2108 / 0x2109 set again on load if KANTO's League is recorded as cleared (gRegionState.cleared).

### K3-2: KANTO's BIRTH ISLAND rock puzzle soft-locks and shares HOENN's Deoxys flag
- Maps: Kanto_BirthIsland_Exterior
- Category: A
- Original: pokefirered BirthIsland_Exterior: `special DoDeoxysTriangleInteraction` on FireRed's own map; its own VAR_DEOXYS_INTERACTION_NUM / step counter, reset on entering.
- Now: data/maps/Kanto_BirthIsland_Exterior/scripts.inc:61 `special DoDeoxysRockInteraction` = Emerald's (src/field_specials.c:3369-3492), which is hard-wired to HOENN's MAP_BIRTH_ISLAND_EXTERIOR (26.58):
  - ChangeDeoxysRockLevel starts FLDEFF_MOVE_DEOXYS_ROCK with HOENN's map number/group (field_specials.c:3457-3458); on KANTO's map (37.53) FldEff_MoveDeoxysRock finds no object and returns without removing itself from the active list (src/field_effect.c:3889-3910), so WaitForDeoxysRockMovement never re-enables the script: the first touch of the triangle that should move it freezes the player;
  - the step counter only counts on HOENN's island (IncrementBirthIslandRockStepCount, :3485), so the "shortest path" rule never applies here;
  - the progress lives in Emerald's VAR_DEOXYS_ROCK_LEVEL / FLAG_DEOXYS_ROCK_COMPLETE, which KANTO's transition script doesn't reset (it resets Quetzal's VAR_QZ_0x5101/5102, :15-16): with FLAG_DEOXYS_ROCK_COMPLETE already set by HOENN's island (it stays set once HOENN's DEOXYS was battled) KANTO's triangle answers COMPLETE and DEOXYS never comes; solving KANTO's sets HOENN's flag.
- Restore: give the Kanto map its own path: the map number/group of the current map in ChangeDeoxysRockLevel / the step counter when on MAP_KANTO_BIRTH_ISLAND_EXTERIOR, and FireRed's own var/flag (VAR_QZ_0x5101/0x5102, FLAG_QZ_0x220B FLAG_SYS_DEOXYS_AWAKENED) instead of Emerald's, reset on entering as FireRed does. A test that solves the puzzle on 37.53.
- Notes: Not in REGIONS_TODO (only exclusivity was handled). Confidence high for the freeze reading of the code; not run in game.

### K3-3: "The VOLBEAT flew away!" when KANTO's DEOXYS is beaten or the player runs
- Maps: Kanto_BirthIsland_Exterior
- Category: E
- Original: pokefirered: `setvar VAR_0x8004, SPECIES_DEOXYS` before "The {STR_VAR_1} flew away!".
- Now: data/maps/Kanto_BirthIsland_Exterior/scripts.inc:113 and :117 `setvar VAR_0x8004, 0x182` (Quetzal's national number 386); in Kai 386 is SPECIES_VOLBEAT (SPECIES_DEOXYS is 410). NAVEL ROCK's 0xF9 / 0xFA (LUGIA / HO-OH) and MT. EMBER's 0x92 (MOLTRES) happen to be right.
- Restore: `setvar VAR_0x8004, SPECIES_DEOXYS` at both lines (and SPECIES_LUGIA / SPECIES_HO_OH / SPECIES_MOLTRES by name for safety).
- Notes: Confidence high.

### K3-4: JESSIE & JAMES's roaming battle: a Quetzal addition, and its reward gives nothing
- Maps: Kanto_OneIsland, Kanto_TwoIsland, Kanto_ThreeIsland, Kanto_FourIsland, Kanto_FiveIsland, Kanto_SixIsland, Kanto_SevenIsland (4 objects each, flag FLAG_QZ_0x2EAB); data/scripts/qz/kanto_shared.inc:80-202 (also Kanto's mainland towns and, through johto_shared.inc:788, JOHTO's)
- Category: A (and C)
- Original: FireRed has no such event.
- Now: after KANTO's League, one town a day shows them (src/qz_specials.c:1037-1066 TravelingEvent); a Lv. 50 battle (TRAINER_QZ_913), then `copyvar VAR_0x8000, VAR_QZ_0x514A` + `qz_givemon_ex VAR_0x8000, 0x32` (kanto_shared.inc:172-173) and "{PLAYER} received Pikachu!" (shared.inc:201, :524). Nothing sets VAR_QZ_0x514A (only cleared, :190; JOHTO's VAR_QZ_0x5073 likewise): Quetzal's C picked the PIKACHU form there; Kai's TravelingEvent doesn't. So givemon gets SPECIES_NONE, the player gets no POKéMON while the text says PIKACHU.
- Restore: remove the event (the 28 objects in KANTO's towns and the map-load `special QzKantoTravelingEventCheck` / Quetzal_EventScript_83C3036 calls); vanilla has nothing there.
- Notes: if the user wants to keep it, the reward must set the species first. Not in REGIONS_TODO. Confidence high.

### K3-5: TANOBY's seven chambers each stand six overlapping REGI objects; only one chamber works
- Maps: Kanto_SevenIsland_TanobyRuins_MoneanChamber, _LiptooChamber, _WeepthChamber, _DilfordChamber, _ScufibChamber, _RixyChamber, _ViapoisChamber
- Category: A (and C)
- Original: FireRed's chambers have no objects (only the UNOWN wild tables and the darkness until the TANOBY KEY puzzle is solved).
- Now: each chamber's map.json has 6 objects on one tile (REGIROCK, REGICE, REGISTEEL, REGIELEKI, REGIDRAGO, REGIGIGAS; flags FLAG_QZ_0x2340-0x234F, all chambers sharing the same flags). src/data/qz_static_encounters.h lists them only for MONEAN (37.24), so RefreshStatics / MP_HideForeignLegends never touch the flags on the other six maps: they show whatever MONEAN's last visit left (all six objects before MONEAN is ever entered; at GEN 8+ REGIELEKI and REGIDRAGO once it has been), and talking to them gives "..." (TalkedStatic finds no entry: kanto_shared.inc:370-373).
- Restore: remove the six objects from all seven chambers (vanilla). (Keeping REGIELEKI / REGIDRAGO would need one per chamber with its own flag and table entry.)
- Notes: REGIROCK / REGICE / REGISTEEL / REGIGIGAS are hidden in MONEAN as foreign legendaries, but not in the other six. Confidence high.

### K3-6: The three GALARIAN birds stand at every GENERATION and only say "..."
- Maps: Kanto_FiveIsland_MemorialPillar (GALARIAN MOLTRES, 12,43), Kanto_FiveIsland_ResortGorgeous (GALARIAN ARTICUNO, 67,15), Kanto_SevenIsland_SevaultCanyon_Entrance (GALARIAN ZAPDOS, 14,11)
- Category: A (and C)
- Original: none of them in FireRed.
- Now: their objects still carry FLAG_QZ_GEN_CAPPED (0x2CFF) from the Gen 4 cap; tools/quetzal_port/gen_later_objects.py didn't give them their own flag, nothing sets 0x2CFF, and they aren't in src/data/qz_later_objects.h. So they show at every GENERATION, and since the static table's entries use 0x2361 / 0x2364 / 0x2367, TalkedStatic doesn't match the object: "...".
- Restore: remove the three objects (vanilla).
- Notes: medium confidence on what the player sees (the gfx id the cap gave them, OBJ_EVENT_GFX_QZ_3526-3528, wasn't checked in game).

### K3-7: Quetzal's overworld legendaries across the Sevii Islands and MT. EMBER
- Maps: (still shown after KANTO's League, GENERATION permitting) TWO ISLAND PECHARUNT, BLACEPHALON; CAPE BRINK WALKING WAKE; THREE ISLAND OKIDOGI; THREE ISLAND PORT OGERPON; BOND BRIDGE FEZANDIPITI; BERRY FOREST MUNKIDORI, CALYREX; KINDLE ROAD's EMBER SPA VOLCANION; TREASURE BEACH MANAPHY; ICEFALL CAVE CHIEN-PAO (Entrance), GLASTRIER (B1F); MT. EMBER RUBY PATH B3F CHI-YU, B5F DIANCIE; MEADOW COBALION; WATER LABYRINTH KELDEO; LOST CAVE ROOM1 SPECTRIER; ROCKET WAREHOUSE GENESECT; RESORT GORGEOUS PHEROMOSA; WATER PATH LANDORUS; RUIN VALLEY RAGING BOLT, WO-CHIEN; PATTERN BUSH ZARUDE; DOTTED HOLE SAPPHIRE ROOM TERAPAGOS; SEVAULT CANYON GOUGING FIRE, TING-LU; TANOBY RUINS STAKATAKA (plus K3-5, K3-6, K3-19). Hidden already (foreign legendaries): MT. EMBER ENTEI, SUMMIT PATH 3F HEATRAN, CAPE BRINK UXIE, PATTERN BUSH CELEBI, the 6 ARCEUS trials (FOUR ISLAND, KINDLE ROAD, MEMORIAL PILLAR, SEVAULT CANYON + ENTRANCE, ALTERING CAVE).
- Category: C
- Original: FireRed has only MOLTRES (MT. EMBER), LUGIA / HO-OH (NAVEL ROCK), DEOXYS (BIRTH ISLAND) and the roaming beast in this area.
- Now: Quetzal_EventScript_840FBBD objects (kanto_shared.inc:212-352) shown by src/qz_specials.c RefreshStatics once the League is beaten.
- Restore: remove these objects for a vanilla Sevii.
- Notes: docs/REGIONS_TODO.md "Decisions" says "Gens 5-9 legendaries have no home region here and stay" and MANAPHY keeps its places; so this is the user's call (keep, or hide them all). The ARCEUS trial objects are dead weight either way (ARCEUS is SINNOH's, hidden for good): they could go. Confidence high.

### K3-8: SELPHY's daily request never resets, and FireRed's wish scene is gone
- Maps: Kanto_FiveIsland_ResortGorgeous_House
- Category: A (and C)
- Original: pokefirered FiveIsland_ResortGorgeous_House/scripts.inc:6-60: "Oh? Do I know you from somewhere? ... I am exquisitely bored. Please, hear my wish." / "I wish to see a POKéMON. A darling {STR_VAR_1}." ; SEBASTIAN's reward; "I wish you the best of luck."; a new wish after each visit (VAR_RESORT_GORGEOUS_REQUESTED_MON 0xFFFF / SPECIES_NONE).
- Now: data/maps/Kanto_FiveIsland_ResortGorgeous_House/scripts.inc: Quetzal's "I'll reward you for showing me a MOéMON. One reward per day!", cancel / re-roll, "Register more captures in Kanto". After a reward it sets FLAG_QZ_0x2EB2 ("You've received today's reward! Come back tomorrow"), but 0x2EB2 is a region flag outside Emerald's daily range and nothing clears it (no reference in src/), so SELPHY gives exactly one reward ever and then says "come back tomorrow" forever.
- Restore: FireRed's script and lines (the rewards in QzSampleResortGorgeousMonAndReward are already FireRed's six); drop the daily flag.
- Notes: also the `callnative 0x80E5AED` lines there are skipped ROM addresses (harmless). Confidence high.

### K3-9: FOUR ISLAND's DAY CARE is an incense shop, and "the DAY-CARE is available from any PC"
- Maps: Kanto_FourIsland_PokemonDayCare, Kanto_FourIsland (the old man at 16,13); shared data/scripts/qz/shared.inc:208-224, :532-540 (also used by Kanto_Route5_PokemonDayCare and JOHTO's DAY CARE / ROUTE 34)
- Category: B (and D)
- Original: pokefirered FourIsland_PokemonDayCare/scripts.inc:4- (the DAY CARE lady: leave / take back two POKéMON, level fee) and FourIsland/scripts.inc:70- (the DAY CARE man: eggs, "I'm the DAY-CARE MAN...").
- Now: the lady says "The DAY CARE is now managed through the PC. We sell special INCENSES here for breeding." and sells incenses (Quetzal_EventScript_83F0976); the man says "The DAY-CARE service is now available from any PC." (Quetzal_EventScript_83F08A1). Kai's PC has no DAY CARE: the claim is false and FOUR ISLAND's DAY CARE can't be used.
- Restore: FireRed's two scripts on Emerald's day care specials (GetDaycareState, ChooseSendDaycareMon, StoreSelectedPokemonInDaycare, TakePokemonFromDaycare, GiveEggFromDaycare...: the same specials FireRed calls).
- Notes: it would be the save's one DAY CARE (the same pair as ROUTE 117's) seen from two places, unless Kai wants a second slot; co-op eggs (MP_Daycare_TryCoopEgg) go through the same code. Confidence high.

### K3-10: The TRAINER TOWER can't be entered: no door, a blocker, and the tower maps unreachable
- Maps: Kanto_SevenIsland_TrainerTower, Kanto_TrainerTower_Lobby, _1F-_8F, _Roof, _Elevator
- Category: D
- Original: pokefirered SevenIsland_TrainerTower/map.json warp (58,7) -> TRAINER_TOWER_LOBBY; the TIME ATTACK challenge (data/scripts/trainer_tower.inc), the OWNER on the roof, the Time Board.
- Now: Kanto_SevenIsland_TrainerTower has no warps at all and Quetzal's object at 58,8 says "Under construction..." (scripts.inc:6-9). The Lobby's exit warp points at a warp 0 the exterior doesn't have; Kai's own stand-in (kanto_shared.inc:1967-1990, "the TRAINER TOWER is closed for now") is therefore never seen; floors' trainers are Quetzal_EventScript_8414471.. stubs.
- Restore: either FireRed's TRAINER TOWER (a big port: trainer_tower.c, its e-Reader-free default floors), or Emerald's TRAINER HILL logic on these maps; at least FireRed's door warp so the lobby (heal, mart, Kai's "closed" desk) is reachable and Quetzal's construction blocker removed.
- Notes: COOP_NOTES "region_closed_tower" made it closed on purpose; restoring is a feature decision. Confidence high.

### K3-11: The POKéMON CENTER 2F attendants describe Quetzal's MPLAYER, cheat modes and NUZLOCKE
- Maps: Kanto_OneIsland/TwoIsland/ThreeIsland/FourIsland/FiveIsland/SixIsland/SevenIsland_PokemonCenter_2F (and every KANTO PC 2F); data/scripts/qz/kanto_shared.inc:30-78, texts :2064-2100
- Category: B
- Original: FireRed's Common_EventScript_UnionRoomAttendant / WirelessClubAttendant / DirectCornerAttendant (and the MYSTERY GIFT delivery man).
- Now: "Start multiplayer from the START menu! Choose MPLAYER.", "Your game and cheat modes must match... In Nuzlocke, your trade clause must allow trading.", "Check its settings in OPTIONS: you can enable COOP battles and adjust their experience." None of this exists in Kai (Kai's co-op is the CO-OP menu).
- Restore: the same attendants HOENN's PC 2Fs use in Kai (Common_EventScript_UnionRoomAttendant etc., e.g. data/maps/OldaleTown_PokemonCenter_2F/events.inc:8-10); the MYSTERY GIFT man can stay out (as everywhere in Kai).
- Notes: one shared change fixes every KANTO PC 2F. Confidence high.

### K3-12: TWO ISLAND's MOVE MANIAC and CAPE BRINK's old man say moves are trained "from your MOéMON team menu"
- Maps: Kanto_TwoIsland_House, Kanto_TwoIsland_CapeBrink_House; text data/scripts/qz/kanto_shared.inc:2370 (Quetzal_Text_8471CE4)
- Category: B (and D)
- Original: pokefirered TwoIsland_House/scripts.inc (MOVE MANIAC: two TINYMUSHROOMS or a BIG MUSHROOM -> `special ChooseMonForMoveRelearner` :51 / TeachMoveRelearnerMove); data/scripts/move_tutors.inc:402 TwoIsland_CapeBrink_House_EventScript_StarterTutor (FRENZY PLANT / BLAST BURN / HYDRO CANNON for a fully evolved starter with max friendship).
- Now: both just say "Move training is handled directly from your MOéMON team menu." Kai's party menu has no move training, so the line is false.
- Restore: the MOVE MANIAC on Emerald's relearner specials (Kai has them: FallarborTown_MoveRelearnersHouse), mushrooms as payment, FireRed's words; the starter tutor on Kai's tutor flow (the three moves exist in Kai's move list).
- Notes: docs/REGIONS_TODO.md "Decisions" lists both as "still Quetzal's stand-ins"; the wrong text is the part to fix at once even if the systems wait. Confidence high.

### K3-13: WATER LABYRINTH's TOGEPI egg is a hatched Lv. 5 TOGEPI; the friendship check and DAISY lines are gone
- Maps: Kanto_FiveIsland_WaterLabyrinth
- Category: C
- Original: pokefirered FiveIsland_WaterLabyrinth/scripts.inc:12 GetLeadMonFriendship check ("Let me take a look... You treat your POKéMON right"), :33 `giveegg SPECIES_TOGEPI`, "received an EGG from the man!", the no-room retry, and later "That's a cute TOGEPI/TOGETIC... I think it was DAISY."
- Now: data/maps/Kanto_FiveIsland_WaterLabyrinth/scripts.inc:13-15 "This little MOéMON wants to see the world." `qz_givemon_ex SPECIES_TOGEPI, 5`, "You received a special MOéMON!".
- Restore: FireRed's script and lines (giveegg, friendship gate, FLAG_NO_ROOM_FOR_TOGEPI_EGG equivalent, the DAISY follow-up through PlayerPartyContainsSpeciesWithPlayerID / QzPartyHasTogepiLineOrTogekiss).
- Notes: confidence high.

### K3-14: TWO ISLAND's shop sells AURORA / MYSTIC TICKETs instead of FireRed's growing shop
- Maps: Kanto_TwoIsland
- Category: C
- Original: pokefirered TwoIsland/scripts.inc:11-175: a shop that opens with GREAT BALL / FRESH WATER and grows after LOSTELLE's rescue, the League and the HOENN link (four lists, four lines: "We've only opened up recently", "the people of THREE ISLAND have changed their attitudes", "I've started bringing in items from distant lands").
- Now: data/maps/Kanto_TwoIsland/scripts.inc:90-101, :163-166: only after the League and the RAINBOW PASS, "We sell tickets for special voyages from VERMILION... Tickets are reusable." (AURORA TICKET, MYSTIC TICKET); before that "I'm preparing new voyages."
- Restore: FireRed's shop chain (its flags 0x222A-0x222D are already set by the script). Keep the two tickets available somewhere (they are the only way to FireRed's NAVEL ROCK / BIRTH ISLAND events, which the user wants kept), e.g. as a fifth list or another clerk.
- Notes: the tickets also open Emerald's own NAVEL ROCK / FARAWAY-style events if Kai's LILYCOVE ferry reads the same items (check). Confidence high.

### K3-15: BILL's METEORITE became a "rock sample"
- Maps: Kanto_OneIsland_PokemonCenter_1F, Kanto_TwoIsland_JoyfulGameCorner
- Category: C
- Original: pokefirered OneIsland_PokemonCenter_1F: "So, can I get you to deliver this METEORITE to him?", "{PLAYER} accepted the METEORITE from BILL." (additem ITEM_METEORITE); TwoIsland_JoyfulGameCorner: "{PLAYER} handed the METEORITE to LOSTELLE's daddy." (removeitem).
- Now: Kanto_OneIsland_PokemonCenter_1F/scripts.inc (Text_8451552, Text_8451686 "{PLAYER} agrees to deliver BILL's rock sample."), no item; Kanto_TwoIsland_JoyfulGameCorner Text_8452FB4 "delivered the sample".
- Restore: FireRed's words; the item only if it can't collide with HOENN's METEORITE (Emerald's key item, the same ITEM_METEORITE); words alone are safe.
- Notes: confidence high.

### K3-16: The JOYFUL GAME CORNER's games are gone (a rest area now)
- Maps: Kanto_TwoIsland_JoyfulGameCorner
- Category: D
- Original: pokefirered TwoIsland_JoyfulGameCorner + data/scripts/cable_club.inc:1275-1290: POKéMON JUMP and DODRIO BERRY PICKING with friends, their record boards; before LOSTELLE "We're not running any games today", after "I'll get the GAME CORNER running right away. You have to come visit with a friend."
- Now: "You can rest here and share stories with other TRAINERS.", "There is much to discover on these islands. You can rest here first.", "Now we can take a little rest."
- Restore: at least FireRed's lines; the games would need POKéMON JUMP / DODRIO BERRY PICKING (both in pokeemerald's source) on Kai's co-op link, like BERRY BLENDER / BERRY CRUSH (mp_minigame.c).
- Notes: confidence high for the difference; the port is a feature decision.

### K3-17: SIX ISLAND's HERACROSS size contest became "show one, get a gift"
- Maps: Kanto_SixIsland_WaterPath_House1
- Category: C (and D)
- Original: pokefirered SixIsland_WaterPath_House1: measures the HERACROSS (GetHeracrossSizeRecordInfo / CompareHeracrossSize), NEST BALL for a new record, the record chart on the wall.
- Now: "HERACROSS is my favorite MOéMON! Bring one in your party..." (QzPartyHasSpecies), one NEST BALL; the chart is "A drawing of HERACROSS."
- Restore: FireRed's contest on Emerald's size-record code (CompareSeedotSize / CompareLotadSize are the same mechanism; one var for the record).
- Notes: needs a save var for the record (Kai's free vars / region sidecar). Confidence high.

### K3-18: SEVEN ISLAND's old man hands out BERRIES; FireRed's man and his box are gone
- Maps: Kanto_SevenIsland_House_Room1, Kanto_SevenIsland_House_Room2
- Category: C
- Original: pokefirered SevenIsland_House_Room1: "I may not look like much now, but when I was younger... my only source of enjoyment is reminiscing about the past while watching battles." and the box "There is a slight breeze blowing around the box."; with an e-Reader TRAINER the hidden basement battle (Room2).
- Now: "These Berries were with me through many battles..." gives a LANSAT and a STARF BERRY (flags 0x2EB0 / 0x2EB1); the box "holds keepsakes from past travels and battles."; Room2 has no TRAINER object.
- Restore: FireRed's two lines and no gift; the e-Reader battle stays out (no e-Reader cards in Kai).
- Notes: confidence high.

### K3-19: MELOETTA joins the CHANSEY Dance
- Maps: Kanto_SevenIsland_SevaultCanyon_House
- Category: C
- Original: pokefirered SevenIsland_SevaultCanyon_House: the dance and the heal only (DID_CHANSEY_DANCE).
- Now: scripts.inc:31-34, :84-104 + src/qz_specials.c:1584 QzSevaultHouseEventEligibility: with a CHANSEY holding a LUCKY PUNCH and knowing SING in the lead after the League, "Oh, it's MELOETTA!... MELOETTA performs Relic Song!" and MELOETTA stays to be battled.
- Restore: remove the MELOETTA branch and object (vanilla).
- Notes: a Gen 5 legendary: under REGIONS_TODO's decision such legendaries stay; the user's call (see K3-7). Confidence high.

### K3-20: PP UP / PP MAX replaced by NUGGETs
- Maps: Kanto_FiveIsland_Meadow (ball 3,22), Kanto_SixIsland_OutcastIsland (ball 11,16), Kanto_ThreeIsland (hidden 5,13), Kanto_TwoIsland_CapeBrink (hidden PP MAX 16,28)
- Category: C
- Original: FireRed's PP UP at the first three, PP MAX at CAPE BRINK.
- Now: NUGGET at all four (map.json / item scripts).
- Restore: FireRed's items.
- Notes: the same swap elsewhere in KANTO (for the other auditors): CELADON hidden 55,20; CERULEAN CAVE 2F ball 9,18; ROCKET HIDEOUT B1F hidden 16,17; ROUTE 13 hidden 23,14; ROUTE 17 hidden 4,68; SILPH CO. 5F hidden 32,9 (FireRed's PP UP spots). Kai has PP UP / PP MAX. Confidence high.

### K3-21: FOUR ISLAND's STICKER MAN is gone
- Maps: Kanto_FourIsland_House2
- Category: D
- Original: pokefirered data/scripts/trainer_card.inc:65-200 FourIsland_House2_EventScript_StickerMan: brag about HALL OF FAME entries, hatched EGGS or link wins for a TRAINER CARD sticker.
- Now: "I collect stickers from places I've visited. Each one reminds me of a trip..." only.
- Restore: FireRed's man, his stickers shown on Kai's TRAINER CARD (mp_tcard.c / mp_cardscreen.c), or at least his words.
- Notes: low priority; Kai's own card may supersede stickers. Confidence high.

### K3-22: ICEFALL CAVE 1F's thin ice never cracks
- Maps: Kanto_FourIsland_IcefallCave_1F
- Category: D
- Original: pokefirered FourIsland_IcefallCave_1F: `setstepcallback STEP_CB_ICE` (thin ice cracks, a second step falls through to B1F) and `special SetIcefallCaveCrackedIceMetatiles` on load.
- Now: scripts.inc:12 `setstepcallback 8`: Emerald has 8 callbacks (0-7), ActivatePerStepCallback turns 8 into STEP_CB_DUMMY (src/field_tasks.c:208); QzSetIcefallCaveCrackedIceMetatiles is a stub (src/qz_specials.c:1088-1089, its comment promises FireRed's behaviour). The fall trigger (VAR_TEMP_1 == 1) can never fire.
- Restore: FireRed's ice callback (like SootopolisGymIcePerStepCallback, on FireRed's ice metatiles) as step callback 8, and the cracked-tile restore.
- Notes: not blocking (the ladders reach B1F). Confidence high.

### K3-23: The first SEVII trip doesn't disable the PC
- Maps: Kanto_OneIsland_PokemonCenter_1F, Kanto_ThreeIsland_Port
- Category: D
- Original: pokefirered OneIsland_PokemonCenter_1F/scripts.inc:116 sets FLAG_SYS_PC_STORAGE_DISABLED (data/scripts/pc.inc:4: "The usual PC services aren't available…") until THREE ISLAND's port clears it.
- Now: the scripts set / clear FLAG_QZ_0x2211 (Kanto_OneIsland_PokemonCenter_1F/scripts.inc:119, Kanto_ThreeIsland_Port/scripts.inc:6) but no code reads it.
- Restore: keep (Kai's shared PC / co-op boxes argue against locking the PC); or make the PC check 0x2211 in KANTO.
- Notes: low; mention for completeness. Confidence high.

### K3-24: No roaming beast after the SAPPHIRE
- Maps: Kanto_OneIsland_PokemonCenter_1F
- Category: D
- Original: pokefirered OneIsland_PokemonCenter_1F/scripts.inc:280 `special InitRoamer` (RAIKOU / ENTEI / SUICUNE by starter roams KANTO).
- Now: absent.
- Restore: keep: the beasts are JOHTO's legendaries in Kai (REGIONS_TODO "Legendaries" decision); FireRed's own roamer isn't one of the "original island events" the user kept.
- Notes: decision for the user; confidence high.

### K3-25: The SEAGALLOP uses Quetzal's destination picker and gates
- Maps: Kanto_OneIsland_Harbor, _TwoIsland_Harbor, _ThreeIsland_Harbor, _FourIsland_Harbor, _FiveIsland_Harbor, _SixIsland_Harbor, _SevenIsland_Harbor (and VERMILION); data/scripts/qz/kanto_shared.inc:1483-1540, src/qz_specials.c:905-960
- Category: C
- Original: pokefirered data/scripts/seagallop.inc: "Ahoy, there! Where do you want to sail?" with fixed multichoices; VERMILION is offered only once VAR_MAP_SCENE_CINNABAR_ISLAND >= 4 (the first SEVII trip has to end with BILL), ISLANDS 4-7 once the RAINBOW PASS is held.
- Now: "Choose a destination (up/down). A: Sail B: Cancel" scroll list (QzShowSeagallopDestinationMenu); VERMILION is always listed, so the player can leave during the first trip; ISLANDS 4-7 also need GAME CLEAR ("To visit ISLANDS 4, 5, 6, and 7, you must win the KANTO LEAGUE.", which K3-1 turns into a real block).
- Restore: FireRed's prompt line; hide VERMILION until VAR_QZ (CINNABAR scene) >= 4 as FireRed; drop the GAME CLEAR check (the RAINBOW PASS flag 0x2214 already implies it). The scroll list itself can stay (FireRed's has two pages there too).
- Notes: medium priority. Confidence high.

### K3-26: The rival is "BLUE" in the islands' lines
- Maps: Kanto_FourIsland (scripts.inc:157), Kanto_SixIsland_PokemonCenter_1F (:99); data/scripts/qz/kanto_shared.inc:2139 (Quetzal_Text_843E142, used by ROUTE 22 / OAK's lab)
- Category: E
- Original: pokefirered FourIsland/text.inc:20 and SixIsland_PokemonCenter_1F/text.inc:17: "{RIVAL}: Hey...".
- Now: "BLUE: Hey!" (26 such lines in KANTO's maps).
- Restore: {RIVAL} with KANTO's rival name (as JOHTO's {RIVAL} was done), or keep BLUE if KANTO's rival is fixed as BLUE by design.
- Notes: KANTO has no rival naming yet (REGIONS_TODO only did JOHTO's); one system-wide decision. Confidence high.

### K3-27: MOLTRES's cry line
- Maps: Kanto_MtEmber_Summit
- Category: C
- Original: pokefirered MtEmber_Summit/scripts.inc:32 `message Text_Gyaoo` ("Gyaoo!").
- Now: scripts.inc:67 "Moltres!".
- Restore: "Gyaoo!".
- Notes: trivial; the same for ZAPDOS / ARTICUNO elsewhere (POWER PLANT, SEAFOAM). Confidence high.

### K3-28: FIVE / SEVEN ISLAND's POKéMON JOURNALs are missing
- Maps: Kanto_FiveIsland_PokemonCenter_1F, Kanto_SevenIsland_PokemonCenter_1F
- Category: D
- Original: pokefirered map.json objects at (2,1)/(3,1), shown after the HALL OF FAME (FLAG_HIDE_POSTGAME_GOSSIPERS cleared, data/scripts/hall_of_fame.inc:2): data/scripts/fame_checker.inc:154 (LORELEI feature), :189 (AGATHA feature).
- Now: no objects (FOUR ISLAND's and RESORT GORGEOUS's journals are there).
- Restore: the two journal objects with FireRed's PokemonJournal_Text_SpecialFeatureLorelei / ...Agatha words (no Fame Checker call).
- Notes: low; confidence high.

### K3-29: SIX ISLAND mart's woman wants REPELS, not MAIL
- Maps: Kanto_SixIsland_Mart
- Category: C
- Original: pokefirered SixIsland_Mart: "I should buy some MAIL and write him a letter…"
- Now: "I should buy some REPELS before exploring more of the island." (Text_845480B)
- Restore: FireRed's line (Kai has MAIL).
- Notes: trivial; confidence high.

### K3-30: RUIN VALLEY's braille door asks "Use CUT on the door?"
- Maps: Kanto_SixIsland_RuinValley
- Category: C
- Original: pokefirered: read the braille ("CUT"), then use CUT from the party menu facing the door (FLAG_USED_CUT_ON_RUIN_VALLEY_BRAILLE).
- Now: scripts.inc:37-50: after the braille, with the badge (QzHasBadgeForFieldMove) "The inscription says CUT. Use CUT on the door?".
- Restore: keep: Emerald's CUT field effect has no FireRed door hook; the prompt does the same job with Kai's badge/move rules. (Optionally drop "The inscription says CUT." so the player still reads the braille.)
- Notes: low.

### K3-31: "Your BERRY POUCH is full."
- Maps: Kanto_ThreeIsland_BerryForest
- Category: E
- Original: FireRed's line (FireRed has a BERRY POUCH).
- Now: scripts.inc:128, the same words; Kai has the BAG's BERRIES pocket.
- Restore: "Your BAG's BERRIES pocket is full." or Kai's usual bag-full line.
- Notes: trivial; confidence high.

### K3-32: kanto_shared.inc: the other Quetzal-only shared pieces
- Maps: data/scripts/qz/kanto_shared.inc (callers outside K3 except as noted)
- Category: C
- Original / Now / Restore, item by item:
  - OAK's AIDES (:2187-2200, Quetzal_Text_8455BAC / 8455C04 / 8455C4B; ROUTE 2, 10, 11, 15, 16): "In KANTO, you have registered N species as caught" is Quetzal's regional wording; FireRed's aide.inc lines ("You have caught only N kinds of POKéMON!") count the same KANTO dex (QzGetPokedexCount 0x8004 = 0). Restore FireRed's words.
  - The SEAGALLOP texts (:2115-2122, :2296-2310): see K3-25.
  - JESSIE & JAMES (:80-202): K3-4. Overworld legendaries / ARCEUS trials (:212-418): K3-5, K3-6, K3-7.
  - TRAINER TOWER stubs (:420-478, :1883-1990): K3-10.
  - PC 2F attendants (:1-78): K3-11. "Move training ..." (:2370): K3-12.
  - The CUT / ROCK SMASH / STRENGTH object scripts (:1636-1880) are FireRed's words on Kai's badge rules (REGIONS_TODO Decisions): fine. The gym / gate / PC / trade helpers are FireRed's: fine.
- Notes: confidence high for the wording; low priority.

---

## Summary by category
- A (broken): K3-1, K3-2, K3-4, K3-5, K3-6, K3-8 (6)
- B (clashes with Kai): K3-9, K3-11, K3-12 (3)
- C (unwanted Quetzal change): K3-7, K3-13, K3-14, K3-15, K3-17, K3-18, K3-19, K3-20, K3-25, K3-27, K3-29, K3-30, K3-32 (13)
- D (FireRed missing): K3-10, K3-16, K3-21, K3-22, K3-23, K3-24, K3-28 (7)
- E (plain bugs): K3-3, K3-26, K3-31 (3)

## Coverage
- Kanto_BirthIsland_Exterior: K3-2, K3-3
- Kanto_BirthIsland_Harbor: only trivial differences (Quetzal's return menu, same choices)
- Kanto_FiveIsland: K3-4
- Kanto_FiveIsland_Harbor: K3-25
- Kanto_FiveIsland_House1: same as the original
- Kanto_FiveIsland_House2: same as the original
- Kanto_FiveIsland_LostCave_Entrance: same as the original
- Kanto_FiveIsland_LostCave_Room1: K3-7
- Kanto_FiveIsland_LostCave_Room10: same as the original (SELPHY's rescue)
- Kanto_FiveIsland_LostCave_Room11: same as the original
- Kanto_FiveIsland_LostCave_Room12: same as the original
- Kanto_FiveIsland_LostCave_Room13: same as the original
- Kanto_FiveIsland_LostCave_Room14: same as the original
- Kanto_FiveIsland_LostCave_Room2: same as the original
- Kanto_FiveIsland_LostCave_Room3: same as the original
- Kanto_FiveIsland_LostCave_Room4: same as the original
- Kanto_FiveIsland_LostCave_Room5: same as the original
- Kanto_FiveIsland_LostCave_Room6: same as the original
- Kanto_FiveIsland_LostCave_Room7: same as the original
- Kanto_FiveIsland_LostCave_Room8: same as the original
- Kanto_FiveIsland_LostCave_Room9: same as the original
- Kanto_FiveIsland_Meadow: K3-7, K3-20
- Kanto_FiveIsland_MemorialPillar: K3-6, K3-7 (TM42 already FireRed's)
- Kanto_FiveIsland_PokemonCenter_1F: K3-28
- Kanto_FiveIsland_PokemonCenter_2F: K3-11
- Kanto_FiveIsland_ResortGorgeous: K3-6, K3-7
- Kanto_FiveIsland_ResortGorgeous_House: K3-8
- Kanto_FiveIsland_RocketWarehouse: K3-7
- Kanto_FiveIsland_WaterLabyrinth: K3-7, K3-13
- Kanto_FourIsland: K3-4, K3-7, K3-9, K3-26
- Kanto_FourIsland_Harbor: K3-25
- Kanto_FourIsland_House1: only trivial differences (BODY SLAM tutor already FireRed's)
- Kanto_FourIsland_House2: K3-21
- Kanto_FourIsland_IcefallCave_1F: K3-22 (HM07 already FireRed's)
- Kanto_FourIsland_IcefallCave_B1F: K3-7
- Kanto_FourIsland_IcefallCave_Back: same as the original
- Kanto_FourIsland_IcefallCave_Entrance: K3-7
- Kanto_FourIsland_LoreleisHouse: same as the original
- Kanto_FourIsland_Mart: only trivial differences (FireRed's list via shop.c's base lists)
- Kanto_FourIsland_PokemonCenter_1F: same as the original
- Kanto_FourIsland_PokemonCenter_2F: K3-11
- Kanto_FourIsland_PokemonDayCare: K3-9
- Kanto_MtEmber_Exterior: K3-1, K3-7 (EXPLOSION tutor already FireRed's)
- Kanto_MtEmber_RubyPath_1F: same as the original
- Kanto_MtEmber_RubyPath_B1F: same as the original
- Kanto_MtEmber_RubyPath_B1F_Stairs: same as the original
- Kanto_MtEmber_RubyPath_B2F: same as the original
- Kanto_MtEmber_RubyPath_B2F_Stairs: same as the original
- Kanto_MtEmber_RubyPath_B3F: K3-7
- Kanto_MtEmber_RubyPath_B4F: same as the original
- Kanto_MtEmber_RubyPath_B5F: K3-7
- Kanto_MtEmber_Summit: K3-27
- Kanto_MtEmber_SummitPath_1F: same as the original
- Kanto_MtEmber_SummitPath_2F: same as the original
- Kanto_MtEmber_SummitPath_3F: K3-7 (HEATRAN, hidden)
- Kanto_NavelRock_1F: same as the original
- Kanto_NavelRock_B1F: same as the original
- Kanto_NavelRock_Base: only trivial differences
- Kanto_NavelRock_BasePath_B10F: same as the original
- Kanto_NavelRock_BasePath_B11F: same as the original
- Kanto_NavelRock_BasePath_B1F: same as the original
- Kanto_NavelRock_BasePath_B2F: same as the original
- Kanto_NavelRock_BasePath_B3F: same as the original
- Kanto_NavelRock_BasePath_B4F: same as the original
- Kanto_NavelRock_BasePath_B5F: same as the original
- Kanto_NavelRock_BasePath_B6F: same as the original
- Kanto_NavelRock_BasePath_B7F: same as the original
- Kanto_NavelRock_BasePath_B8F: same as the original
- Kanto_NavelRock_BasePath_B9F: same as the original
- Kanto_NavelRock_Exterior: same as the original
- Kanto_NavelRock_Fork: same as the original
- Kanto_NavelRock_Harbor: only trivial differences
- Kanto_NavelRock_Summit: only trivial differences
- Kanto_NavelRock_SummitPath_2F: same as the original
- Kanto_NavelRock_SummitPath_3F: same as the original
- Kanto_NavelRock_SummitPath_4F: same as the original
- Kanto_NavelRock_SummitPath_5F: same as the original
- Kanto_OneIsland: K3-4
- Kanto_OneIsland_Harbor: K3-25
- Kanto_OneIsland_House1: same as the original
- Kanto_OneIsland_House2: same as the original
- Kanto_OneIsland_KindleRoad: K3-7 (ARCEUS trial, hidden)
- Kanto_OneIsland_KindleRoad_EmberSpa: K3-7 (HM06 already FireRed's)
- Kanto_OneIsland_PokemonCenter_1F: K3-1, K3-15, K3-23, K3-24
- Kanto_OneIsland_PokemonCenter_2F: K3-11
- Kanto_OneIsland_TreasureBeach: K3-7
- Kanto_SevenIsland: K3-4 (SWORDS DANCE tutor already FireRed's)
- Kanto_SevenIsland_Harbor: K3-25
- Kanto_SevenIsland_House_Room1: K3-18
- Kanto_SevenIsland_House_Room2: K3-18
- Kanto_SevenIsland_Mart: only trivial differences
- Kanto_SevenIsland_PokemonCenter_1F: K3-28
- Kanto_SevenIsland_PokemonCenter_2F: K3-11
- Kanto_SevenIsland_SevaultCanyon: K3-7
- Kanto_SevenIsland_SevaultCanyon_Entrance: K3-6, K3-7
- Kanto_SevenIsland_SevaultCanyon_House: K3-19
- Kanto_SevenIsland_SevaultCanyon_TanobyKey: same as the original
- Kanto_SevenIsland_TanobyRuins: K3-7
- Kanto_SevenIsland_TanobyRuins_DilfordChamber: K3-5
- Kanto_SevenIsland_TanobyRuins_LiptooChamber: K3-5
- Kanto_SevenIsland_TanobyRuins_MoneanChamber: K3-5
- Kanto_SevenIsland_TanobyRuins_RixyChamber: K3-5
- Kanto_SevenIsland_TanobyRuins_ScufibChamber: K3-5
- Kanto_SevenIsland_TanobyRuins_ViapoisChamber: K3-5
- Kanto_SevenIsland_TanobyRuins_WeepthChamber: K3-5
- Kanto_SevenIsland_TrainerTower: K3-10
- Kanto_SixIsland: K3-4
- Kanto_SixIsland_AlteringCave: K3-7 (ARCEUS, hidden)
- Kanto_SixIsland_DottedHole_1F: same as the original
- Kanto_SixIsland_DottedHole_B1F: same as the original
- Kanto_SixIsland_DottedHole_B2F: same as the original
- Kanto_SixIsland_DottedHole_B3F: same as the original
- Kanto_SixIsland_DottedHole_B4F: same as the original
- Kanto_SixIsland_DottedHole_SapphireRoom: K3-7
- Kanto_SixIsland_GreenPath: same as the original
- Kanto_SixIsland_Harbor: K3-25
- Kanto_SixIsland_House: same as the original
- Kanto_SixIsland_Mart: K3-29
- Kanto_SixIsland_OutcastIsland: K3-20
- Kanto_SixIsland_PatternBush: K3-7
- Kanto_SixIsland_PokemonCenter_1F: K3-26
- Kanto_SixIsland_PokemonCenter_2F: K3-11
- Kanto_SixIsland_RuinValley: K3-7, K3-30
- Kanto_SixIsland_WaterPath: K3-7
- Kanto_SixIsland_WaterPath_House1: K3-17
- Kanto_SixIsland_WaterPath_House2: same as the original
- Kanto_ThreeIsland: K3-4, K3-7, K3-20
- Kanto_ThreeIsland_BerryForest: K3-7, K3-31
- Kanto_ThreeIsland_BondBridge: K3-7
- Kanto_ThreeIsland_DunsparceTunnel: only trivial differences (GAME CLEAR for FireRed's NATIONAL DEX check; see K3-1)
- Kanto_ThreeIsland_Harbor: K3-25
- Kanto_ThreeIsland_House1: same as the original
- Kanto_ThreeIsland_House2: same as the original
- Kanto_ThreeIsland_House3: same as the original
- Kanto_ThreeIsland_House4: same as the original
- Kanto_ThreeIsland_House5: same as the original
- Kanto_ThreeIsland_Mart: only trivial differences
- Kanto_ThreeIsland_PokemonCenter_1F: same as the original
- Kanto_ThreeIsland_PokemonCenter_2F: K3-11
- Kanto_ThreeIsland_Port: K3-7
- Kanto_TrainerTower_1F: K3-10
- Kanto_TrainerTower_2F: K3-10
- Kanto_TrainerTower_3F: K3-10
- Kanto_TrainerTower_4F: K3-10
- Kanto_TrainerTower_5F: K3-10
- Kanto_TrainerTower_6F: K3-10
- Kanto_TrainerTower_7F: K3-10
- Kanto_TrainerTower_8F: K3-10
- Kanto_TrainerTower_Elevator: K3-10
- Kanto_TrainerTower_Lobby: K3-10
- Kanto_TrainerTower_Roof: K3-10
- Kanto_TwoIsland: K3-1, K3-4, K3-7, K3-14
- Kanto_TwoIsland_CapeBrink: K3-7, K3-20
- Kanto_TwoIsland_CapeBrink_House: K3-12
- Kanto_TwoIsland_Harbor: K3-25
- Kanto_TwoIsland_House: K3-12
- Kanto_TwoIsland_JoyfulGameCorner: K3-15, K3-16
- Kanto_TwoIsland_PokemonCenter_1F: same as the original
- Kanto_TwoIsland_PokemonCenter_2F: K3-11
- data/scripts/qz/kanto_shared.inc: K3-4, K3-5, K3-6, K3-7, K3-10, K3-11, K3-12, K3-25, K3-26, K3-32
