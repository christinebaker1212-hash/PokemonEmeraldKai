# J3 findings: Mahogany, Lake of Rage, Rocket base, Routes 42-45/47/48, Mt. Mortar, Ice Path, Blackthorn, Victory Road, Indigo Plateau / Elite Four, Mt. Silver, Tohjo Falls, Cliff caves, Safari Zone, Route 23

Paths: current game files are under /home/user/PokemonEmeraldKai/ (data/maps/Johto_<Map>/scripts.inc unless said
otherwise); Crystal is /tmp/claude-0/frlg/pokecrystal/maps/<Map>.asm. Only maps/ exists in the local pokecrystal clone
(no data/ or engine/), so Crystal's trade table, mart lists and trainer parties are quoted from pret/pokecrystal as I know
it (marked "from memory" where it matters).

---------------------------------------------------------------------------------------------------------------------

### J3-1: Every JOHTO gym leader win writes raw Quetzal trainer ids with settrainerflag: out-of-bounds writes past sSpecialFlags
- Maps: Johto_MahoganyGym, Johto_BlackthornGym1F (and the six other JOHTO gyms through the same shared script)
- Category: A (Quetzal feature broken; memory corruption)
- Original: Crystal marks a gym's trainers beaten with `setevent EVENT_BEAT_SKIER_ROXANNE...` after the leader
  (MahoganyGym.asm:34-38, BlackthornGym1F.asm Clair script).
- Now: Johto_MahoganyGym/scripts.inc:83-84 and Johto_BlackthornGym1F/scripts.inc:124-125 do `setvar VAR_0x8008, 7|8` +
  `call Quetzal_EventScript_83601F0` (data/scripts/qz/johto_shared.inc:264-326), which runs `settrainerflag 0x115E`,
  `0x12BD`... These are Quetzal's trainer ids (0x115E = 4446 = RONALD) never translated to Kai's TRAINER_QZ_* numbers
  (TRAINER_QZ_4446 is 984, include/constants/qz_port.h:1983). ScrCmd_settrainerflag (src/scrcmd.c:1939) passes 4446 to
  SetTrainerFlag -> TRAINER_FLAG(4446) = 0x3780 + (4446-1664) = 0x425E (include/constants/flags.h:1359-1361) ->
  GetFlagPointer returns &sSpecialFlags[(0x425E-0x4000)/8] (src/event_data.c:242) on a 16-byte array
  (src/event_data.c:9,32). All 28 ids in the script (4105..4938) land 33-137 bytes past the array: every JOHTO gym win
  sets stray bits in whatever EWRAM statics follow sSpecialFlags.
- Restore: delete the `setvar VAR_0x8008, n` / `call Quetzal_EventScript_83601F0` pair from all eight JOHTO gyms (Kai's
  MP_Coop_GymLeaderBeaten, src/battle_setup.c:1663, already marks a non-HOENN gym's trainers beaten), or translate the
  ids to TRAINER_QZ_<id> constants. Optionally add a bounds check to GetFlagPointer for 0x4000+ ids.
- Notes: not in REGIONS_TODO. Affects Violet/Azalea/Goldenrod/Ecruteak/Cianwood/Olivine too (other auditors' maps; one
  fix in the shared script covers all). Confidence: high.

### J3-2: ICE PATH's SWINUB can never be fed: the script compares the berry with Quetzal's item number 518
- Maps: Johto_IcePathB2FBlackthornSide
- Category: A (and C: the whole event is Quetzal's)
- Original: none in Crystal (no SWINUB / MAMOSWINE event; Ice Path B2F has only items).
- Now: scripts.inc:61 `goto_if_ne VAR_ITEM_ID, 518, ...SWINUB doesn't like it!`; ITEM_ASPEAR_BERRY is 137 here and 518 is
  ITEM_SCROLL_OF_WATERS (include/constants/items.h:177, 616). Any berry chosen fails, a blizzard pushes the player back.
  The "evolution" also calls raw ROM addresses (`callnative 0x80DF0BD` / `0x80DF0D9`, scripts.inc:70-72) that
  ScrCmd_callnative skips (src/scrcmd.c:173-177). If it ran it would give a Lv45 MAMOSWINE battle (a PILOSWINE stand-in
  below GEN 4: src/script_pokemon_util.c:144) with perfect IVs and a NEVER-MELT ICE (Quetzal: ICIUM Z).
- Restore: vanilla = remove the SWINUB object (local id 1), the trigger at 9,22, the sign at 22,21 and object 3
  (Crystal has nothing here). If kept instead: compare with ITEM_ASPEAR_BERRY and drop the callnatives.
- Notes: Category C as a whole (JOHTO.md section 8). Confidence: high.

### J3-3: LAKE OF RAGE's RED GYARADOS is not shiny
- Maps: Johto_LakeOfRage
- Category: A / D
- Original: LakeOfRage.asm:86-90 `loadwildmon GYARADOS, 30` + `loadvar VAR_BATTLETYPE, BATTLETYPE_FORCESHINY`.
- Now: scripts.inc:89 `qz_setstaticwildmon SPECIES_GYARADOS, 0x1E` -> src/qz_port.c:169 ScrCmd_qz_setstaticwildmon ->
  CreateScriptedWildMon (src/script_pokemon_util.c:139), random IVs/personality, nothing forces shininess. Quetzal's
  command tagged the mon as its shiny static; that part was lost in the port. The red object leads into a normal-colored
  GYARADOS.
- Restore: make the encounter shiny (e.g. a flag that CreateScriptedWildMon/CreateMon honours to pick a shiny
  personality, the way Emerald's own forced-shiny events do), keep Lv30 and the RED SCALE.
- Notes: Confidence: high.

### J3-4: LANCE says he hands over WHIRLPOOL in the ROCKET HIDEOUT but gives nothing
- Maps: Johto_TeamRocketBaseB2F
- Category: A
- Original: TeamRocketBaseB2F.asm:286-292 `verbosegiveitem HM_WHIRLPOOL` then "That's WHIRLPOOL...".
- Now: scripts.inc ~317-321 shows Text_83EA59D "Oh, yes. You should take this. I found it here..." and Text_83EA67E
  "That's WHIRLPOOL. Teach it to a MOéMON to get across wild water..." with no item given in between.
- Restore: once A4 adds a WHIRLPOOL HM (REGIONS_TODO "Decisions": whirlpools open with the GLACIERBADGE plus a POKéMON with
  WHIRLPOOL or SURF "until A4 adds an HM for it"), give it here as Crystal does; until then reword or cut the two lines.
- Notes: depends on the A4 HM decision. Confidence: high.

### J3-5: MAHOGANY's old man says the MAGIKARP are back only while the player is inside the hideout
- Maps: Johto_MahoganyTown
- Category: A (wrong condition)
- Original: MahoganyTown.asm:90-100 `checkevent EVENT_CLEARED_ROCKET_HIDEOUT` -> "MAGIKARP have returned to LAKE OF RAGE".
- Now: scripts.inc:177 `goto_if_eq VAR_QZ_0x500C, 6`; 6 is set on entering the hideout's B2F (Johto_TeamRocketBaseB2F
  trigger at 9,24, setvar 6) and replaced by 7 on B3F; the hideout is cleared at 14 (TeamRocketBaseB2F setvar 0xE).
  Before and after the hideout he says the "GYARADOS rampage" line.
- Restore: `goto_if_ge VAR_QZ_0x500C, 14`.
- Notes: confidence high.

### J3-6: MT. SILVER's "bag" starts a repeatable COSMOG battle with no gating
- Maps: Johto_SilverCaveItemRoom
- Category: A (and C)
- Original: Crystal has no item room event (SilverCaveItemRoom1/2 hold items only).
- Now: object 2 (gfx QZ_97, flag 0) runs Johto_SilverCaveItemRoom_EventScript_840EA8C (scripts.inc:13): "..." then
  QzJohtoStaticEncounterSetup -> TalkedStatic (src/qz_specials.c:310-326) matches any static on the map when the object
  has no flag, so it picks the first entry, COSMOG Lv20 (src/data/qz_static_encounters.h:64). SetUpStatic does not check
  the done flag, GENERATION or the League (RefreshStatics is skipped: the bag has no flag to hide), and the bag is never
  removed, so a Gen 7 COSMOG can be fought/caught again and again at any GENERATION before or after the League.
- Restore: remove object 2 (and the COSMOG entry, a Quetzal addition); Crystal has nothing here.
- Notes: Confidence: high on the mechanism.

### J3-7: JESSIE & JAMES' reward is an empty POKéMON (VAR_QZ_0x5073 is never set)
- Maps: Johto_MahoganyTown, Johto_BlackthornCity, Johto_IndigoPlateau, Johto_SafariZoneGate (objects with
  Quetzal_EventScript_83C2D48; also other towns)
- Category: A (and C: the whole encounter is Quetzal's)
- Original: none (Crystal has no Jessie/James).
- Now: data/scripts/qz/johto_shared.inc:787-833: after TRAINER_QZ_913 it copies VAR_QZ_0x5073 into the species and
  `qz_givemon_ex VAR_0x8000, 0x32...`; nothing in data/ or src/ sets 0x5073 (Quetzal's code did), so species 0 is given:
  GiveMonToPlayer (src/pokemon.c:4634-4647) copies a species-0 mon into the first empty slot, raises gPlayerPartyCount
  and the script prints "{PLAYER} received Pikachu!" (shared.inc:199-205). The trio is shown one town a day after the
  League (src/qz_specials.c:1044-1066).
- Restore: vanilla = never show them (keep FLAG_QZ_0x1490 set: make QzJohtoTravelingEventCheck return FALSE and set the
  flag). If kept: VarSet 0x5073 = SPECIES_PIKACHU before the gift.
- Notes: Category C pattern noted in JOHTO.md section 0. Confidence: high.

### J3-8: PRYCE at LAKE OF RAGE invites the player to battles at the FIGHTING DOJO that don't exist
- Maps: Johto_LakeOfRage (object 15, flag FLAG_QZ_0x101E); Johto_DragonsDenB1F (CLAIR, object 21, flag 0x1020)
- Category: A / C
- Original: Crystal has no postgame Pryce at the lake nor Clair in the den with this offer.
- Now: Johto_LakeOfRage/scripts.inc:314 "If you wish to battle again, come to the FIGHTING DOJO." Kai's own code shows
  him after the JOHTO League (src/qz_specials.c QzGiovanniStoryUpdateFlags, sLeaders with FLAG_QZ_0x101E), but
  Kanto_SaffronCity_Dojo has only FireRed's karate people (events.inc objects 1-8): no PRYCE, CLAIR, MORTY... CLAIR's
  copy (DragonsDenB1F scripts.inc Text_83CBEDC) stays hidden forever (0x1020 set at start, never cleared).
- Restore: hide both (drop FLAG_QZ_0x101E from sLeaders; delete DragonsDenB1F object 21), or build the Dojo battles.
- Notes: MORTY / WHITNEY / JASMINE (same sLeaders list) are other auditors' maps but share the problem. Confidence: high.

### J3-9: CLAIR promises gym leader rematches that the game doesn't have
- Maps: Johto_DragonShrine
- Category: A
- Original: Crystal's Clair: "...When you reach the POKéMON LEAGUE, don't you dare lose!..." (DragonsDenB1F.asm Clair
  text; no rematch line).
- Now: scripts.inc:435-440 "Once you become CHAMPION, you can challenge the GYM LEADERS again." Rematches were Quetzal's
  QZ_SPECIAL_670; here a beaten leader's TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC (e.g. MahoganyGym scripts.inc
  Johto_MahoganyGym_EventScript_83DE70F, BlackthornGym1F 83C7453) checks the trainer flag and only plays the after text.
- Restore: drop that sentence (Crystal's text otherwise).
- Notes: confidence high.

### J3-10: INDIGO PLATEAU's postgame SILVER: one battle, "challenge me anytime"; Crystal's weekly rival battles missing
- Maps: Johto_IndigoPlateauPokecenter1F, Johto_DragonsDenB1F
- Category: A / C / D
- Original: IndigoPlateauPokecenter1F.asm:44-140: after the rival is beaten at MT. MOON (Kanto) he waits at the plateau on
  Mondays and Wednesdays, once a day (ENGINE_INDIGO_PLATEAU_RIVAL_FIGHT), team by the player's starter.
  DragonsDenB1F.asm:26-38: he also trains in DRAGON'S DEN on Tuesdays and Thursdays (DragonsDenB1FRivalScript). Dragon
  Shrine elder's "A boy close to your age..." text (DragonShrine.asm .BeatRivalInMtMoon).
- Now: object 7 (flag 0x10DA, shown once FLAG_QZ_0x11B1 = JOHTO champion, scripts.inc:30-37) is a plain
  `trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4325` (Lv70 team, src/data) with Quetzal's words; its flag makes it a
  one-time battle although he says "I'll be here. Challenge me anytime." (scripts.inc:152). No rival in the den.
- Restore: Crystal's weekday appearances and texts with a rematch-capable battle (EventScript like TRAINER_BATTLE_REMATCH
  or clearing the flag daily); add the den appearance; levels per Crystal's RIVAL2 team.
- Notes: depends on Kanto's MT. MOON rival battle existing in Kai's KANTO (Crystal's prerequisite). Confidence: high.

### J3-11: BAOBA's "SAFARI LEVEL" quest can never start
- Maps: Johto_SafariZoneEntrance
- Category: A (C as a whole)
- Original: none (HGSS Safari Zone has Baoba's block challenges, not this).
- Now: object 1 script (scripts.inc ~200-350) branches on VAR_QZ_0x502F == 1..5; only values 2-5 are set, by those same
  branches; nothing sets 1, so the BEACH/BRUSH/MOUNTAIN/CAVE requests and their ¥200,000-400,000 rewards are dead. The
  text asks for MOéMON "from the BEACH area of the FUCHSIA SAFARI ZONE" while qz_checkpartymoninlist checks Hoenn
  species lists (src/qz_port.c:214-221).
- Restore: delete the dead branches and texts (vanilla has no such quest).
- Notes: confidence high.

### J3-12: MAHOGANY's mart: Crystal's Rocket-front shop and its people are gone; the shop list is chosen by badges
- Maps: Johto_MahoganyMart1F
- Category: C / D / E
- Original: MahoganyMart1F.asm:32-60: the PHARMACIST sells MART_MAHOGANY_1 (TINYMUSHROOM, SLOWPOKETAIL, POKé BALL,
  POTION: from memory) before Lance comes; afterwards "Arrgh… You found the secret stairway…"; a BLACK BELT ("Heheh! The
  experiment worked like a charm...", "Urrgh… That guy's dragon #MON are tough…"); after Clair (clearevent
  EVENT_MAHOGANY_MART_OWNERS in BlackthornGym1F.asm) GRANNY sells MART_MAHOGANY_2.
- Now: scripts.inc:98-115: one clerk says "The store is... closed right now." (no shop) until VAR_QZ_0x500C >= 14,
  "ROCKETS have hijacked my basement!" at 5, and from 14 a mart. src/shop.c sMpBaseLists picks Crystal's Granny list
  only from 7 badges, else the Rocket list (TINY MUSHROOM, POKé BALL, POTION): a player who clears the hideout with
  fewer than 7 badges buys the Rockets' stock from the old woman, while the Rocket list is unreachable before. Quetzal's
  regional treats (PEWTER CRUNCHIES, LAVA COOKIE, OLD GATEAU, CASTELIACONE, LUMIOSE GALETTE, SHALOUR SABLE, BIG
  MALASADA) are added by GENERATION. LANCE's "The stairs are right here." became "Excellent work finding the stairs,
  DRAGONITE!" (scripts.inc Text_83DF5E7).
- Restore: Crystal's PHARMACIST (Rocket list before Lance, his "Arrgh" line after) and BLACK BELT objects (hidden once the
  hideout is cleared), GRANNY with MART_MAHOGANY_2 after CLAIR; drop the treats list; Crystal's stairs line.
- Notes: Pecharunt object 5 is J3-23. Confidence: medium-high (Crystal mart contents from memory).

### J3-13: The VICTORY ROAD gate also demands the LUGIA event before the LEAGUE
- Maps: Johto_VictoryRoadGate (and Johto_NewBarkTown's ROUTE 27 crossing, another auditor's map)
- Category: C (major)
- Original: VictoryRoadGate.asm:22-38: only `readvar VAR_BADGES` >= 8.
- Now: scripts.inc:54 `goto_if_lt VAR_QZ_0x501A, 8` -> "Sorry kid, this way is closed until someone deals with that
  LEGENDARY MOéMON..."; 501A becomes 8 only in Johto_WhirlIslandLugiaChamber (scripts.inc:187). The player must do
  Quetzal's Kimono Girl chain (ELM's message in BLACKTHORN, DANCE THEATER, HO-OH with the CLEAR BELL, LUGIA with the TIDAL
  BELL) before the LEAGUE. NewBarkTown scripts.inc:41 has the same check.
- Restore: drop the 501A check (badges only). Moving HO-OH / LUGIA back to postgame (Rainbow / Silver Wing) is a wider
  JOHTO decision (Tin Tower, Whirl Islands auditors).
- Notes: JOHTO.md section 12 lists it; REGIONS_TODO doesn't. Confidence: high.

### J3-14: VICTORY ROAD gate's east door walks the whole party into KANTO, skipping Kai's customs
- Maps: Johto_VictoryRoadGate
- Category: B
- Original: Crystal's gate leads west to ROUTE 22 (VictoryRoadGate.asm warps 17,7 / 18,7); its right BLACK BELT ("Off to
  the #MON LEAGUE, are you?...") stands there until EVENT_FOUGHT_SNORLAX (Kanto side).
- Now: warp to MAP_KANTO_ROUTE22_NORTH_ENTRANCE (events.inc:18), guarded by object 3 "Only the champions of both
  MOéMON LEAGUES may cross" (QzVictoryRoadGateCheck). Crossing changes region on map load (src/mp/mp_region.c:2069-2070
  MP_Region_Travel) with the full party, while Kai's boats keep one POKéMON and send the rest to the GLOBAL PC
  (src/qz_specials.c TravelNow). Crystal's "Off to the LEAGUE" line is gone.
- Restore: decide whether walking between regions is allowed; if yes, apply the same customs on this door; if no,
  close it (Kai's boats / MAGNET TRAIN cover travel). Restore Crystal's black belt line for the gate person.
- Notes: Kai first. Confidence: medium (design question).

### J3-15: MT. SILVER and RED open right after the JOHTO LEAGUE; RED stands outdoors, gives a LIGHT BALL and can be re-fought
- Maps: Johto_VictoryRoadGate, Johto_SilverCaveOutside..Summit, Johto_SilverCavePokecenter1F
- Category: C
- Original: OaksLab.asm:43 EVENT_OPENED_MT_SILVER after 16 badges; the VictoryRoadGate left BLACK BELT ("This way leads to
  MT.SILVER...") blocks ROUTE 28 until then; RED in SilverCaveRoom3 (SilverCaveRoom3.asm:9-34), Lv73-81 (PIKACHU 81,
  ESPEON 73, SNORLAX 75, VENUSAUR 77, CHARIZARD 77, BLASTOISE 77, from memory), no prize, disappears, credits.
  SilverCavePokecenter1F's GRANNY: "Trainers who seek power climb MT. SILVER despite its many dangers…".
- Now: the black belt hides as soon as FLAG_QZ_0x11B1 (JOHTO champion) is set (VictoryRoadGate scripts.inc:9-12). RED on
  the summit (Quetzal_EventScript_83E05E8, johto_shared.inc:1207-1255): Lv100 RAICHU (Quetzal's cap PIKACHU capped to the
  wrong species), SNORLAX, CHARIZARD, VENUSAUR, BLASTOISE, LAPRAS; gives a LIGHT BALL (Quetzal: PIKASHUNIUM Z); with both
  flags set talking to him starts the battle again. A Pewter museum researcher replaces the granny. Mt. Silver is
  Quetzal's 9-map complex (Room1-3, Fire Room, Item Room, Mountainside, Snowy Slope, Summit) instead of Crystal's.
- Restore: black belt until 16 badges (JOHTO + KANTO); RED in Room 3 or keep the summit (layout), Crystal's team and
  levels, no item, one battle then credits (as Crystal); Crystal's granny line.
- Notes: rival/RED parties are part of J3-24. Confidence: high.

### J3-16: DRAGON SHRINE's DRATINI is Lv35 with an EVERSTONE; Crystal's Lv15 EXTREMESPEED reward is gone
- Maps: Johto_DragonShrine
- Category: C / D
- Original: DragonShrine.asm:216-218 `givepoke DRATINI, 15` + `special GiveDratini` (EXTREMESPEED when every quiz answer
  was right, EVENT_ANSWERED_DRAGON_MASTER_QUIZ_WRONG).
- Now: scripts.inc Johto_DragonShrine_EventScript_83CD025 (no wrong answers): `qz_givestorymon 4` = DRATINI Lv35 holding
  an EVERSTONE (src/qz_port.c:98); 83CCFE5 (wrong answers): `qz_givemon_ex SPECIES_DRATINI, 0x23` (Lv35).
- Restore: Lv15 both ways, EXTREMESPEED (replacing a move) for a perfect quiz, no held item.
- Notes: REGIONS_TODO "Events" lists JOHTO gift fixes (EEVEE, TYROGUE) but not this one. Confidence: high.

### J3-17: EMY's trade is reversed and gives a perfect DRAGONAIR from "QUETZAL"; her after line names SLIGGOO
- Maps: Johto_BlackthornEmysHouse
- Category: C / E
- Original: Crystal NPC_TRADE_EMY (data/events/npc_trades.asm, from memory): she wants a DRAGONAIR and gives a DODRIO
  nicknamed DORIS. BlackthornEmysHouse.asm:8-14 `trade NPC_TRADE_EMY`.
- Now: trade 7 in src/data/qz_ingame_trades.h:39-43: gives DRAGONAIR (Quetzal's Hisuian SLIGGOO capped) with 31 IVs
  everywhere, nickname "DRAGONAIR", OT "QUETZAL", for a DODRIO; scripts.inc Text_83C816E "Hi, how's my SLIGGOO doing?"
- Restore: Crystal's trade (receive DODRIO "DORIS", give DRAGONAIR, Crystal's OT EMY); Crystal's NPC trade words.
- Notes: 8 of the 17 JOHTO/KANTO trades carry OT "QUETZAL" (src/data/qz_ingame_trades.h): a pattern for all trade owners
  (tools/quetzal_port/gen_trades.py). Confidence: medium (trade direction from memory).

### J3-18: WESLEY and SANTOS are there every day and quiz the player for their gift
- Maps: Johto_LakeOfRage (WESLEY), Johto_BlackthornCity (SANTOS)
- Category: C
- Original: LakeOfRage.asm:34-41 / BlackthornCity.asm Santos callback: they appear only on Wednesday / Saturday, introduce
  themselves ("Seeing as how it's Wednesday today, I'm WESLEY of Wednesday.") and give BLACKBELT / SPELL TAG.
- Now: always present; "Can you guess my favorite day?" / "Take a guess." multichoice (MULTI_QZ_0x4A); a wrong day gets
  "Not exactly..." / "SANTOS: No." (LakeOfRage scripts.inc ~Johto_LakeOfRage_EventScript_83DDBDC; BlackthornCity
  scripts.inc 83C6A1F).
- Restore: weekday callbacks (show/hide by RTC weekday as Kai already does elsewhere), Crystal's meet / not-today texts,
  no quiz.
- Notes: the other weekday siblings belong to other auditors (same pattern). Confidence: high.

### J3-19: LAKE OF RAGE's FISHING GURU buys MAGIKARP instead of rating their length
- Maps: Johto_LakeOfRageMagikarpHouse, Johto_LakeOfRage (sign)
- Category: C / D
- Original: LakeOfRageMagikarpHouse.asm:9-60: after the hideout he asks for a big MAGIKARP, measures it
  (CheckMagikarpLength) and gives an ELIXER for a record; the lake's sign shows the record (LakeOfRage.asm
  MagikarpHouseSignScript).
- Now: from VAR_QZ_0x500C >= 14 he pays ¥2,000 per MAGIKARP, ¥100,000 for a Lv100 one (scripts.inc:8-190); the sign just
  says "FISHING GURU'S HOUSE".
- Restore: Crystal's length contest (Emerald has a size-record mechanic: SEASHORE HOUSE / Fallarbor style
  CompareSeedotSize-like specials could be reused), ELIXER prize, record on the sign.
- Notes: Confidence: high.

### J3-20: ROUTE 43 toll: ¥500 and free for the broke; charged only before LANCE
- Maps: Johto_Route43Gate
- Category: C
- Original: Route43Gate.asm:1 ROUTE43GATE_TOLL EQU 1000, takemoney 1000 (all you have if less), while
  EVENT_CLEARED_ROCKET_HIDEOUT is unset; Rockets block from both sides.
- Now: scripts.inc:11-15 asks only while VAR_QZ_0x500C is 1-5 (not 0, not during the hideout 6-13), ¥500
  (`checkmoney 0x1F4`), and with ¥0 "You're totally broke!... Okay, you can pass..." (Text_83D207A, NEW).
- Restore: ¥1000 from 0 to 13, Crystal's words, no broke line.
- Notes: Confidence: high.

### J3-21: MOVE DELETER missing in BLACKTHORN ("MOVE BROS' HOUSE" is empty)
- Maps: Johto_BlackthornCity, Johto_MoveDeletersHouse, Johto_BlackthornPokecenter1F
- Category: D / C / B
- Original: MoveDeletersHouse.asm (34 lines): the MOVE DELETER (special MoveDeletion); sign "MOVE DELETER'S HOUSE";
  BlackthornCity cooltrainer "Are you going to make your #MON forget some moves?".
- Now: Johto_MoveDeletersHouse has no objects or scripts; sign "MOVE BROS' HOUSE" (BlackthornCity scripts.inc
  Text_83C6F07); cooltrainer "...remember some moves?". The PC's "I got the MOVE DELETER to make it forget an HM move"
  points to nobody. Kai's MOVE RELEARNER is in the CO-OP menu (src/mp/mp_menu.c:1134-1138); there is no deleter here.
- Restore: put a MOVE DELETER in the house using Emerald's own deleter script (LilycoveCity_MoveDeletersHouse), Crystal's
  sign and line.
- Notes: Kai's HM rules (HMs can be forgotten?) may make the PC line moot; keep Crystal's words anyway. Confidence: high.

### J3-22: BLACKTHORN's gym and DRAGON'S DEN gating are Quetzal's
- Maps: Johto_BlackthornCity, Johto_BlackthornGym1F
- Category: C / D / A (dead branches)
- Original: BlackthornCity.asm:33-55: the SUPER NERD blocks the gym ("I am sorry. CLAIR, our GYM LEADER, entered the
  DRAGON'S DEN behind the GYM...") until EVENT_CLEARED_RADIO_TOWER; BLACK BELT's "weird radio" line before the RADIO
  TOWER is freed; the DRAGON'S DEN GRAMPS refuses entry until CLAIR is beaten (BlackthornGramps1Script /
  EVENT_BLACKTHORN_CITY_GRAMPS_BLOCKS_DRAGONS_DEN). Gym guide: "CLAIR uses the mythical and sacred dragon-type #MON..."
- Now: the nerd blocks the door until CHUCK, JASMINE and PRYCE's badges (FLAG_QZ_0x11B6-11B8; "she only battles trainers
  who already have 7 badges", scripts.inc:5-14, Text_83C6AEA). The GRAMPS's refusing branch and trigger need
  VAR_QZ_0x5010 == 1, which nothing sets (only 2, 4, 5), so the DEN is open from the start and the gramps always says
  "You may enter."; Quetzal_EventScript_83CC300 (johto_shared.inc:1004) waits for 5010 == 3, also never set. Only the
  post-RADIO TOWER black belt line exists. Gym guide rewritten: "team consisting of dragons from the sea... DRAGON-type or
  FAIRY-type moves" (BlackthornGym1F Text_83C7CEA): FAIRY doesn't exist below GENERATION 6.
- Restore: Crystal's gates (RADIO TOWER cleared for the gym; gramps blocks until CLAIR is beaten: set 5010 = 1 at the
  right time or use the badge/flag directly), the black belt's two lines, Crystal's guide text.
- Notes: confidence high.

### J3-23: Overworld legendaries in J3's maps (Quetzal's 99-object list)
- Maps: LakeOfRage, MahoganyMart1F, TeamRocketBaseB2F/B3F, Route44/45/47/48, MountMortar1FOutside/1FInside/B1F,
  IcePathB1F/B2FBlackthornSide/B3F, DragonsDenB1F, DragonShrine, VictoryRoad1F/2F, IndigoPlateau, CliffCave,
  SafariZone_Desert/Forest/Peak, SilverCaveFireRoom/ItemRoom/Mountainside/SnowySlope/Room3, TohjoFalls
- Category: C
- Original: none of these in Crystal (SUICUNE is met at TIN TOWER after the chase; the beasts roam).
- Now (src/data/qz_static_encounters.h, gating src/qz_specials.c:427-512):
  - shown before the LEAGUE: SUICUNE Lv30 at LAKE OF RAGE (PreLeagueLegend).
  - hidden (another region's, MP_HideForeignLegends): MESPRIT (Lake), AZELF (Den), ARTICUNO (Ice Path B2F, Snowy Slope),
    ZAPDOS-G (Route 45), MEWTWO (Rocket B3F), CRESSELIA (Safari Peak), MOLTRES + HEATRAN (Fire Room), DEOXYS (Room 3),
    DIALGA + PALKIA (Tohjo).
  - after the LEAGUE, by GENERATION: PECHARUNT, XURKITREE, TYPE: NULL, VIRIZION, IRON LEAVES, OKIDOGI, TORNADUS,
    FEZANDIPITI, LANDORUS, COBALION, IRON CROWN, TING-LU, KELDEO, VOLCANION, KUBFU, CHIEN-PAO, GLASTRIER, KYUREM, RESHIRAM,
    ZEKROM, TERRAKION, IRON BOULDER, STAKATAKA, ZACIAN, ZAMAZENTA, ETERNATUS, DIANCIE, PHEROMOSA, BUZZWOLE, COSMOG +
    NECROZMA, CALYREX, JIRACHI.
- Restore: per the user's decisions (REGIONS_TODO "Decisions": Gens 5-9 legendaries "have no home region here and stay";
  JIRACHI keeps its places) keep those; question for the user: SUICUNE at the lake before the League (Crystal has it at
  TIN TOWER), and KUBFU, which needs the SCROLLS nobody gives any more (the KARATE KING's TYROGUE replaced them).
- Notes: COSMOG's repeatable copy is J3-6. Confidence: high.

### J3-24: JOHTO's gym leaders, ELITE FOUR, CHAMPION, RED and SILVER use Quetzal's teams
- Maps: MahoganyGym, BlackthornGym1F, WillsRoom, KogasRoom, BrunosRoom, KarensRoom, LancesRoom, SilverCaveSummit,
  VictoryRoad3F, IndigoPlateauPokecenter1F (and every route trainer)
- Category: C
- Original (Crystal parties, from memory): PRYCE SEEL 27, DEWGONG 29, PILOSWINE 31; CLAIR DRAGONAIR 37 x3, KINGDRA 40;
  WILL XATU 40, JYNX 41, EXEGGUTOR 41, SLOWBRO 41, XATU 42; KOGA ARIADOS 40, VENOMOTH 41, FORRETRESS 43, MUK 42, CROBAT
  44; BRUNO HITMONTOP 42, HITMONLEE 42, HITMONCHAN 42, ONIX 43, MACHAMP 46; KAREN UMBREON 42, VILEPLUME 42, GENGAR 45,
  MURKROW 44, HOUNDOOM 47; LANCE GYARADOS 44, DRAGONITE 47 x2, AERODACTYL 46, CHARIZARD 46, DRAGONITE 50.
- Now (src/data/trainers_qz.h / trainer_parties_qz.h, Gen-4 capped): PRYCE NINETALES, SANDSLASH, DELIBIRD, FROSLASS 37,
  MAMOSWINE 38; CLAIR DRAGONAIR 42, STEELIX 43, ENTEI 43, GARCHOMP 44, KINGDRA 45; WILL 48-50 with METAGROSS; KOGA 50-52
  with NINJASK, SLOWKING; BRUNO 52-54 with HARIYAMA, BLAZIKEN; KAREN 53-55 with DRAPION, SALAMENCE, HONCHKROW, DARKRAI;
  LANCE GYARADOS 54, CHARIZARD 55, ENTEI 54, AERODACTYL 55, SALAMENCE 54, DRAGONITE 56; RED Lv100 (J3-15); SILVER at
  VICTORY ROAD Lv45-48 (WEAVILE, MAGNEZONE...). The Gen-4 cap turned Quetzal's GOUGING FIRE into ENTEI on CLAIR and LANCE,
  and KAREN keeps SINNOH's DARKRAI, against the spirit of "legendaries only in their home region". The gym guide hints
  were rewritten to match (J3-25).
- Restore: Crystal's parties and levels (tools/quetzal_port/gen_trainers.py overrides, like IV_OVERRIDES), at least
  no legendaries on gym leaders / ELITE FOUR; Kai's LEVEL SCALING can still raise them.
- Notes: a JOHTO-wide question; REGIONS_TODO covers KANTO's trainers ("trainer matches") but not JOHTO's.
  Confidence: high for "differs", medium for Crystal's exact levels.

### J3-25: MAHOGANY gym guide rewritten for Quetzal's PRYCE
- Maps: Johto_MahoganyGym
- Category: C
- Original: MahoganyGym.asm MahoganyGymGuideText: "...He's said to be good at freezing opponents with ice-type moves. That
  means you should melt him with your burning ambition!"
- Now: Text_83DEC22 "He's got a solid lineup of MOéMON to cover whatever you bring his way. Fire is a good place to
  start, but watch out for the rain!"
- Restore: Crystal's text. Confidence: high.

### J3-26: "Welcome back" rematch intros for the ELITE FOUR, LANCE and the HALL OF FAME
- Maps: Johto_WillsRoom, KogasRoom, BrunosRoom, KarensRoom, LancesRoom, HallOfFame, BlackthornGym1F
- Category: C
- Original: Crystal says the same intro each run (WillsRoom.asm etc.); LANCE / MARY / OAK lines; HALL OF FAME text once.
- Now: with FLAG_QZ_0x11CA (champion) set: WILL "Ah, welcome back. I have foreseen this encounter...", KOGA "You return,
  brave challenger...", BRUNO, KAREN, LANCE "Welcome back! You conquered the JOHTO LEAGUE once...", OAK "Conquering the
  JOHTO LEAGUE again...", MARY "Come on! You can't blow off the interview AGAIN!", HALL OF FAME "Once again..."; CLAIR
  "You did it, huh? You are now the JOHTO CHAMP." Same teams (QzUseAltGymLeaderTeam returns FALSE).
- Restore: always Crystal's first-run words (drop the 11CA branches). Low priority.
- Notes: Confidence: high.

### J3-27: HALL OF FAME doesn't send PROF. ELM's S.S. TICKET call
- Maps: Johto_HallOfFame
- Category: D
- Original: HallOfFame.asm:43 `specialphonecall SPECIALCALL_SSTICKET` (ELM calls to come for the S.S. TICKET).
- Now: Johto_HallOfFame scripts.inc 83E7EB3 sets flags, no call; src/mp/mp_pokegear.c has no S.S. TICKET call.
- Restore: queue ELM's Crystal call on the POKéGEAR (mp_pokegear.c / mp_qzphone.c) after the first JOHTO HALL OF FAME.
- Notes: ELM's lab belongs to another auditor (where the ticket is given). Confidence: high.

### J3-28: LAKE OF RAGE's LANCE scene and ROUTE 42's EUSINE scene are rewritten
- Maps: Johto_LakeOfRage, Johto_Route42
- Category: C
- Original: LakeOfRage.asm LanceIntroText: "Did you come here because of the rumors? You're <PLAYER>? I'm LANCE, a trainer
  like you... could you help me investigate?"; he teleports away. Route42.asm:21-30: SUICUNE just flees (after the
  Cianwood sighting), no EUSINE.
- Now: "This man here informed me of some rumors. So we came to investigate…" / "could you help us..." (talks about the
  old man, object 4), walks off with DRAGONITE; ROUTE 42 trigger at 36,15 plays SUICUNE plus three NEW EUSINE lines
  (Text_83F4BEF/83F4C58/83F4D2D) the first time the player passes, with no story condition. LAKE OF RAGE has rain /
  thunderstorm weather (setweather 5) and its trainers stand there before the hideout is cleared (Crystal: they appear
  after, EVENT_LAKE_OF_RAGE_CIVILIANS).
- Restore: Crystal's LANCE lines; SUICUNE without EUSINE and only after the Cianwood sighting; trainers after the
  hideout; no weather.
- Notes: Confidence: high.

### J3-29: TOHJO FALLS gives a SUPER ROD and a HEART SCALE
- Maps: Johto_TohjoFalls, Johto_TohjoFallsGiovannisRoom
- Category: C
- Original: TohjoFalls.asm:10 `itemball MOON_STONE`, no NPC. Crystal's SUPER ROD is ROUTE 12's FISHING GURU in KANTO.
- Now: a FISHER (object 4) gives the SUPER ROD ("You've made it to TOHJO FALLS..."), item ball HEART SCALE; a side room
  (TohjoFallsGiovannisRoom) holds GIOVANNI only for saves already in his story (src/qz_specials.c GiovanniStoryBegun) and
  a dead radio sign.
- Restore: MOON STONE ball; SUPER ROD from KANTO ROUTE 12's guru (check the KANTO side gives it); the room can stay as an
  empty dead end or be walled off.
- Notes: GIOVANNI quest already closed (REGIONS_TODO "Events" [x]). Confidence: high.

### J3-30: SAFARI ZONE (Quetzal's) - free, QUICK BALLs, held-item shops, BERRY BLENDERs
- Maps: Johto_SafariZoneGate, SafariZoneGatePokecenter1F, SafariZoneEntrance, SafariZone_EntrancePath, _Plains, _Forest,
  _Desert, _Beach, _Peak, _Savannah, Johto_Route48, Johto_CliffEdgeCave, Johto_CliffCave, Johto_Route47
- Category: C (A/E parts: J3-11, J3-31)
- Original: no Safari Zone, Routes 47/48 or Cliff caves in Crystal (HGSS has them: Safari game with SAFARI BALLs, time
  limit, block placement; Baoba).
- Now: reachable from CIANWOOD (CliffEdgeCave) -> ROUTE 47 -> ROUTE 48 -> SAFARI ZONE GATE (a small town with a POKéMON
  CENTER and four held-item clerks: type boosters, weather rocks, classic and "modern" items), the ENTRANCE gives 10 QUICK
  BALLs (VAR_QZ_0x500F chain from OLIVINE), three EMERALD BERRY BLENDER spots (DoBerryBlending), free catching with no
  limit, six areas holding PHEROMOSA, BUZZWOLE, CRESSELIA (hidden). Routes 47/48 use HGSS trainers' words.
- Restore: Kai > Crystal + HGSS > Quetzal: keep the HGSS maps and trainers; for "vanilla" drop the four held-item
  clerks, the QUICK BALL gift and the free rules or add HGSS's Safari game; the blenders are harmless Kai (HOENN contest)
  extras. User's call.
- Notes: confidence high on contents.

### J3-31: Small text bugs in J3 maps and shared scripts
- Maps / files: data/scripts/qz/johto_shared.inc:2761 (Quetzal_Text_8383BB5), Johto_DragonsDenB1F (LEA & PIA),
  Johto_Route47 (THOM & KAE, DUFF & EDA), Johto_Route45, Johto_SafariZoneGate, Johto_Route47
- Category: E
- Now / Restore:
  - The double-battle "only one POKéMON" text says "GINA: Oh? Only one MOéMON? Then, we don't battle with you..."
    (Emerald's twins) for LEA & PIA, THOM & KAE, DUFF & EDA: use a neutral line or each pair's names.
  - Route 45 Camper QUENTIN: "I was tough at the TRAINER HILL…" / "Have you been to the TRAINER HILL?" (Crystal: BATTLE
    TOWER, Route45.asm CamperQuentin texts); JOHTO's Trainer Hill has no facility.
  - Route 47 DUFF & EDA: "as wide and deep\l the ocean" (missing "as").
  - SAFARI ZONE GATE "modern held items" clerk sells only EVIOLITE (Gen 5): an empty shop below GENERATION 5 (src/shop.c
    MP_ShopItems filters by MP_Gen_ItemAllowed); "weather and terrain items" has no terrain items.
  - CLIFF CAVE hiker: "...to a dead end.\p but people can't help..." (lowercase start).
- Notes: Confidence: high.

### J3-32: RAGECANDYBAR seller and ROUTE 43's MAHOGANY gate officer follow the RADIO TOWER instead of the hideout
- Maps: Johto_MahoganyTown, Johto_Route43MahoganyGate
- Category: E
- Original: MahoganyTown.asm RageCandyBarMerchantScript: sold out once EVENT_CLEARED_ROCKET_HIDEOUT;
  Route43MahoganyGate.asm: "Nobody goes up to LAKE OF RAGE these days." after the hideout.
- Now: MahoganyTown scripts.inc:114 sold out only at VAR_QZ_0x500C == 17 (RADIO TOWER), and its step trigger keeps
  forcing the sales pitch for values 1-16 (incl. after the hideout); Route43MahoganyGate checks VAR_QZ_0x500A >= 11.
- Restore: `goto_if_ge VAR_QZ_0x500C, 14` in both places (and the triggers only below 14).
- Notes: Confidence: medium-high.

### J3-33: MT. SILVER ROOM 3's triangular rock (DEOXYS) is a dead prop
- Maps: Johto_SilverCaveRoom3
- Category: A (low)
- Original: none in Crystal.
- Now: the rock (object 1) calls QzDoDeoxysRockInteractionAlt, a stub that only re-enables the script
  (src/qz_specials.c:1086); DEOXYS (HOENN) is hidden by MP_HideForeignLegends anyway.
- Restore: remove the rock and the DEOXYS object (Crystal's Room 3 is RED's room, J3-15).
- Notes: Confidence: high.

### J3-34: Mahogany / Blackthorn / Indigo dead or Quetzal-only bits (low)
- Maps: Johto_IndigoPlateauPokecenter1F, Johto_VictoryRoadGate, Johto_IndigoPlateau, Johto_IcePathB1F, Johto_Route23
- Category: C (trivial)
- Now: INDIGO PC object 8 ("The MOéMON LEAGUE is not taking new challengers", flag 0x1061 set at start, never cleared)
  and object 10 / VICTORY ROAD GATE object 4 (GIOVANNI's past / DARKRAI dream guards, FLAG_QZ_0x1321) never show; extra
  NPC "Congrats on making it here!" (object 9); the ABRA teleport guy stays after the HALL OF FAME (Crystal sets
  EVENT_TELEPORT_GUY there); POKéMON statues read "It's a replica of an ancient MOéMON." (Quetzal_EventScript_83F804F on
  ROUTE 23, INDIGO, DRAGON'S DEN, DRAGON SHRINE); ICE PATH B1F sign "DANGER: SLIPPERY ICE..." (NEW).
- Restore: optional cleanup: delete the never-shown objects, hide the teleport guy after the HALL OF FAME.
- Notes: Confidence: high.

### J3-35: HGSS-flavoured events kept from Quetzal (for the record: HGSS ranks above Quetzal)
- Maps: Johto_IcePath1F (KIMONO GIRL SAYO stuck on the ice, blocks 49,29 until pushed), Johto_TeamRocketBaseB1F-B3F
  (ARIANA + grunt 2-on-1 with LANCE joining as a multi battle; PETREL disguised as GIOVANNI; MURKROW password - also in
  Crystal), Johto_Route47 / Route48 / CliffCave / CliffEdgeCave (HGSS maps and words)
- Category: C
- Original: Crystal has none of SAYO, the ARIANA multi battle or PETREL's disguise.
- Restore: keep (HGSS) unless the user wants Crystal strictly; the multi battle depends on QzBackupPartyForOptionBattle
  (fixed: src/qz_specials.c:68-75).
- Notes: Confidence: high.

---------------------------------------------------------------------------------------------------------------------

## Shared scripts (data/scripts/qz/shared.inc and data/scripts/qz/johto_shared.inc, Quetzal-only parts)

### J3-36: DAY CARE "through the PC" texts are still used
- Files: shared.inc:207-224 (Quetzal_EventScript_83F08A1 "The DAY-CARE service is now available from any PC.";
  Quetzal_EventScript_83F0976 "The DAY CARE is now managed through the PC. We sell special INCENSES here for breeding."
  + INCENSE mart Quetzal_Mart_83F0990)
- Category: B
- Original: Crystal's ROUTE 34 DAY-CARE couple; FireRed's ROUTE 5 / FOUR ISLAND day cares.
- Now: used by Johto_DayCare object 1 (events.inc:8), Johto_Route34 object 12 (events.inc:19), Kanto_FourIsland,
  Kanto_FourIsland_PokemonDayCare, Kanto_Route5_PokemonDayCare. Describes Quetzal's PC day care, not Kai's DAY CARE.
- Restore: Crystal's / FireRed's day-care people on Kai's DAY CARE scripts; drop the INCENSE clerk (or keep as an
  ordinary shop with honest words).
- Notes: the owners of those maps (J1/K auditors) should apply it; listed here as the shared script's owner. Confidence:
  high.

### J3-37: "Moves can be managed from the POKEMON summary screen."
- Files: johto_shared.inc:1173-1178 (Quetzal_EventScript_83DC26C, Quetzal_Text_83DC904)
- Category: B / E
- Now: used by Johto_IlexForest objects 6 and 18 and Johto_OlivineCity object 8. Kai has no summary-screen move manager
  (its MOVE RELEARNER is the CO-OP menu's REMINDER for a HEART SCALE, src/mp/mp_menu.c:1134); "POKEMON" is also the only
  unconverted spelling.
- Restore: those maps' Crystal lines (e.g. ILEX's people / OLIVINE's); remove the script.
- Notes: Confidence: high.

### J3-38: PC transfer and boat messages say the wrong thing in JOHTO / KANTO
- Files: shared.inc:149-178, 458-480 (gift sent to "SOMEONE'S PC" / "LANETTE'S PC": FLAG_QZ_0xAC3 is never set, so always
  "SOMEONE'S PC"); shared.inc:561-566 Quetzal_Text_840FB2D "you need an S.S. Ticket and must have won both Leagues"
- Category: E
- Original: Crystal "...was sent to BILL's PC"; FireRed "BILL's PC" (once met) / "SOMEONE's PC".
- Now: used by johto_shared Quetzal_EventScript_840180E (DRAGON SHRINE's DRATINI, MANIA's SHUCKLE...) and the
  Jessie & James gift; the boat refusal is reached when QzGetReachableRegionsMask is 0, whose rule is this region's
  League + S.S. TICKET (src/qz_specials.c:1100-1106), not both.
- Restore: "BILL's PC" in JOHTO and KANTO (Kai's own wording for HOENN stays); boat text "...an S.S. TICKET and must have
  won this region's LEAGUE."
- Notes: Confidence: high.

### J3-39: Shared-script inventory (what is Quetzal-only and its state)
- Files: data/scripts/qz/shared.inc, data/scripts/qz/johto_shared.inc
- Category: C (summary; individual problems are the findings above)
- Now:
  - Dead (no caller): shared.inc NUZLOCKE gate 8361A4D is still called by 16 gift scripts but QzIsNuzlockeActiveInRegion
    returns FALSE (src/qz_specials.c:62), so its text never shows (harmless); johto_shared 836266F (Quetzal berry tree,
    objects re-pointed to BerryTreeScript / Johto_EventScript_FruitTree), 83FDB17, 8406ED7, the four MAGEARNA texts
    840745D-8407672 (BILL's future arc, unreferenced).
  - GIOVANNI's past (8401B9D-8404B5E, 8401C4F, 83C2DFA texts, the alternate-past townsfolk 8404959): reachable only for a
    save already in that story (src/qz_specials.c GiovanniStoryBegun) - REGIONS_TODO [x].
  - Jessie & James 83C2D44/83C2D48: J3-7.
  - Gym trainer flags 83601F0: J3-1.
  - RED 83E05E8: J3-15. Radio alert 83DF6B4 (MAHOGANY / OLIVINE / CIANWOOD, "A public RADIO alert interrupts the usual
    broadcast: TEAM ROCKET has taken over the GOLDENROD RADIO TOWER!", set when the 7th badge is won at those gyms via
    83DF68E): Quetzal's stand-in for Crystal's RadioTowerRocketsScript + phone call (MahoganyGym.asm
    MahoganyGymActivateRockets); the timing matches Crystal, the delivery doesn't: D/C low, keep or move to a POKéGEAR
    radio interruption.
  - TIN TOWER roof dance + HO-OH (83FB64A-83FBA45: SILVER WING and TIDAL BELL handed by the Kimono Girls) and the VICTORY
    ROAD dependency on it: J3-13 (Tin Tower / Whirl Islands auditors own the scenes).
  - Whirlpool 837F567 ("You cannot cross it yet" without the badge), CUT 837EEDF, ROCK SMASH 837EF35, STRENGTH 837EFAC:
    Kai's region badge + party check (REGIONS_TODO "Decisions"), fine; texts are Emerald-style, not Crystal's (trivial).
  - Vending machine 8306A5D: FireRed/Crystal drinks restored (REGIONS_TODO [x]).
  - Nurse 82E1B64: Emerald's nurse (GOLD CARD, POKéRUS lines) on region flags 0x2A01/0x2A02/0x2A07: fine (Kai's).
  - Statue / bookshelf / clock signs 83F804F, 83FA60F, 8360850, 84017BA: trivial Quetzal words.
  - HALL OF FAME save loop 835F6E4 ("The League could not be recorded... Press A or B to try again", loops until the
    save succeeds): acceptable.
  - Boat menus 840F954-840FABD: Kai's region travel wrapper (mp_region.inc EventScript_RegionPickAll) - J3-38 text only.
- Restore: delete the dead labels when convenient (no gameplay effect).
- Notes: Confidence: high.

---------------------------------------------------------------------------------------------------------------------

## Coverage
- Johto_BlackthornCity: J3-7, J3-18, J3-21, J3-22, J3-24
- Johto_BlackthornDragonSpeechHouse: same as the original
- Johto_BlackthornEmysHouse: J3-17
- Johto_BlackthornGym1F: J3-1, J3-22, J3-24, J3-26
- Johto_BlackthornMart: only trivial differences (Crystal's two NPC lines; shop list per src/shop.c)
- Johto_BlackthornPokecenter1F: J3-21 (MOVE DELETER line); otherwise only trivial differences
- Johto_BrunosRoom: J3-24, J3-26
- Johto_CliffCave: J3-23, J3-30, J3-31
- Johto_CliffEdgeCave: J3-30
- Johto_DragonShrine: J3-9, J3-16, J3-23, J3-38
- Johto_DragonsDen1F: same as the original (warps only)
- Johto_DragonsDenB1F: J3-8, J3-10, J3-23, J3-31
- Johto_HallOfFame: J3-26, J3-27
- Johto_IcePath1F: J3-35 (SAYO, HGSS); HM07 ball is Crystal's (REGIONS_TODO [x])
- Johto_IcePathB1F: J3-23, J3-34
- Johto_IcePathB2FBlackthornSide: J3-2, J3-23
- Johto_IcePathB2FMahoganySide: only trivial differences (item balls)
- Johto_IcePathB3F: J3-23
- Johto_IndigoPlateau: J3-7, J3-23, J3-34
- Johto_IndigoPlateauPokecenter1F: J3-10, J3-34
- Johto_KarensRoom: J3-24, J3-26
- Johto_KogasRoom: J3-24, J3-26
- Johto_LakeOfRage: J3-3, J3-8, J3-18, J3-19, J3-23, J3-28
- Johto_LakeOfRageHiddenPowerHouse: same as the original (TM10 restored; wall-clock sign trivial)
- Johto_LakeOfRageMagikarpHouse: J3-19
- Johto_LakeOfRage_Alt: unused Quetzal layout header, no events, nothing warps to it (only its own map.json references
  MAP_JOHTO_LAKE_OF_RAGE_ALT); harmless
- Johto_LancesRoom: J3-24, J3-26
- Johto_MahoganyGym: J3-1, J3-24, J3-25
- Johto_MahoganyMart1F: J3-12, J3-23
- Johto_MahoganyPokecenter1F: same as the original
- Johto_MahoganyRedGyaradosSpeechHouse: same as the original
- Johto_MahoganyTown: J3-5, J3-7, J3-32
- Johto_MountMortar1FInside: J3-23 (otherwise only trivial differences)
- Johto_MountMortar1FOutside: J3-23
- Johto_MountMortar2FInside: only trivial differences
- Johto_MountMortarB1F: J3-23 (KARATE KING's TYROGUE restored, REGIONS_TODO [x])
- Johto_MoveDeletersHouse: J3-21
- Johto_Route23: J3-34 (Quetzal's Johto copy of FireRed's ROUTE 23 north part; reached from VICTORY ROAD 3F, connects to
  INDIGO PLATEAU)
- Johto_Route42: J3-28 (Tully's phone is Kai's mp_qzphone hook)
- Johto_Route42EcruteakGate: same as the original
- Johto_Route43: only trivial differences
- Johto_Route43Gate: J3-20
- Johto_Route43MahoganyGate: J3-32
- Johto_Route44: J3-23
- Johto_Route45: J3-23, J3-31
- Johto_Route47: J3-23, J3-30, J3-31 (HGSS map, no Crystal source; reached from CLIFF EDGE CAVE)
- Johto_Route48: J3-23, J3-30 (HGSS map; leads to the SAFARI ZONE GATE)
- Johto_SafariZoneEntrance: J3-11, J3-30
- Johto_SafariZoneGate: J3-7, J3-30, J3-31
- Johto_SafariZoneGatePokecenter1F: J3-30 (Quetzal's own POKéMON CENTER, three new NPC lines)
- Johto_SafariZone_Beach: J3-30 (Quetzal area, no events; reached through the area connections)
- Johto_SafariZone_Desert: J3-23, J3-30
- Johto_SafariZone_EntrancePath: J3-30 (path from the ENTRANCE into PLAINS)
- Johto_SafariZone_Forest: J3-23, J3-30
- Johto_SafariZone_Peak: J3-23, J3-30
- Johto_SafariZone_Plains: J3-30
- Johto_SafariZone_Savannah: J3-30
- Johto_SilverCaveFireRoom: J3-15, J3-23 (Quetzal room off ROOM 1, only hidden legendaries)
- Johto_SilverCaveItemRoom: J3-6, J3-23
- Johto_SilverCaveMountainside: J3-15, J3-23
- Johto_SilverCaveOutside: J3-15
- Johto_SilverCavePokecenter1F: J3-15
- Johto_SilverCaveRoom1: J3-15 (item balls only)
- Johto_SilverCaveRoom2: J3-15 (ROCK SMASH rocks only)
- Johto_SilverCaveRoom3: J3-15, J3-23, J3-33
- Johto_SilverCaveSnowySlope: J3-15, J3-23
- Johto_SilverCaveSummit: J3-15, J3-24
- Johto_SilverCaveSummitAlt: unused alternate layout (picked by FLAG_QZ_0x1016, which nothing sets; nothing warps to it);
  its RED object would be J3-15
- Johto_TeamRocketBaseB1F: J3-35 (security cameras and secret switch present)
- Johto_TeamRocketBaseB2F: J3-4, J3-5, J3-23, J3-35
- Johto_TeamRocketBaseB3F: J3-23, J3-35
- Johto_TohjoFalls: J3-23, J3-29
- Johto_TohjoFallsGiovannisRoom: J3-29 (Quetzal room; GIOVANNI only for saves already in his story)
- Johto_VictoryRoad1F: J3-23 (Quetzal's three-floor VICTORY ROAD; items differ, kept per REGIONS_TODO A4 note)
- Johto_VictoryRoad2F: J3-23
- Johto_VictoryRoad3F: J3-24 (SILVER's battle and words are Crystal's)
- Johto_VictoryRoadGate: J3-13, J3-14, J3-15, J3-34
- Johto_WillsRoom: J3-24, J3-26
- data/scripts/qz/shared.inc and johto_shared.inc: J3-1, J3-7, J3-15, J3-31, J3-36, J3-37, J3-38, J3-39
