# J1 findings: JOHTO (New Bark to Azalea, Routes 26-33 / 46, Ruins of Alph, Union Cave, Ilex, Dark Cave)

Compared against pret/pokecrystal `maps/*.asm` (the local clone has only `maps/`; Crystal's `data/` and `engine/`
are not in it, so NPC-trade and phone facts below come from Crystal's well-known data and are marked so).
Paths: "Now" = /home/user/PokemonEmeraldKai/..., "Original" = /tmp/claude-0/frlg/pokecrystal/maps/...
Helpers used (throwaway): tmp_J1/cmp.py (text pairing per map), tmp_J1/items.py (item balls / hidden items).

Ordered roughly by importance. Things docs/REGIONS_TODO.md marks done (HM givers, TM givers, MR. POKéMON's EXP. SHARE,
starter table, rival naming, POKéGEAR, BANK OF MOM, APRICORNs / KURT's balls, plates gone, foreign legendaries hidden)
were checked and are not repeated unless still broken.

---

### J1-1: NEW BARK's first visit plants Quetzal's BERRIES into JOHTO's patches 90-99 and the GARDEN farm's plots 100-109
- Maps: Johto_NewBarkTown (also every JOHTO BERRY patch 90-99, and the MOéMON GARDEN farm)
- Category: A (and E)
- Original: Crystal plants nothing (fruit trees are fixed objects, NewBarkTown.asm has no tree set-up).
- Now: data/maps/Johto_NewBarkTown/scripts.inc:56-78 (`Johto_NewBarkTown_EventScript_83E1426`, called from the
  ON_TRANSITION script) runs `setberrytree 0x5A..0x6D` once (guard FLAG_QZ_0x11D0, which nothing else sets; it is not in
  src/data/qz_region_init.h). Tree ids 0x5A-0x63 = 90-99 are now JOHTO's own BERRY patches (src/mp/mp_apricorn.c
  sBerryPatches, planted once by MP_JohtoBerries_Init), and 0x64-0x6D = 100-109 are the GARDEN farm's plots
  (src/mp/mp_garden_map.h GARDEN_FARM_TREE_FIRST 100; offline GetBerryTreeInfo returns gSaveBlock1Ptr->berryTrees[100+]
  directly). ScrCmd_setberrytree (src/scrcmd.c:2009) has no guard. So the first time a player enters NEW BARK (a JOHTO new
  game, or the first trip there from another region), ten GARDEN plots are overwritten with Quetzal's freshly planted
  BERRIES (berry 1-10, stage 1), and the JOHTO patches 90-99 planted by MP_JohtoBerries_Init (if the player reached
  another JOHTO map first, e.g. a new game starts in the bedroom) are reset to sprouts.
- Restore: delete the whole `setberrytree` block (and its call) from NEW BARK's transition script (fix_fruit_trees.py
  removed Quetzal's tree objects but not this set-up). Optionally set FLAG_QZ_0x11D0 in region init for safety.
- Notes: REGIONS_TODO "Quetzal's berry system out" is marked [x]; this leftover was missed. Saves that already ran it
  keep the damage (farm plots replanted). Confidence high (code path traced; not run in-game).

### J1-2: The CHARCOAL KILN apprentice never gives CHARCOAL any more (flag shared with the HM01 gift)
- Maps: Johto_CharcoalKiln, Johto_IlexForest
- Category: A
- Original: IlexForest.asm:352 the CHARCOAL master gives HM01 CUT; CharcoalKiln.asm:50 the apprentice then gives
  CHARCOAL ("I'm sorry--I forgot to thank you. This is CHARCOAL that I made.").
- Now: Kai's HM fix gives HM01 in Ilex and sets FLAG_QZ_0x1030 (data/maps/Johto_IlexForest/scripts.inc:213-217). The
  kiln apprentice's CHARCOAL (Quetzal's "my boss left your gift with me" fallback) is skipped when FLAG_QZ_0x1030 is set
  (data/maps/Johto_CharcoalKiln/scripts.inc:52-58). Since the master hands HM01 over at the end of the FARFETCH'D
  scene, the CHARCOAL is never given. (If a player somehow talks to the apprentice first, the reverse happens: the
  apprentice sets 0x1030 and Ilex's `goto_if_set FLAG_QZ_0x1030` skips HM01.)
- Restore: give the apprentice's CHARCOAL its own flag and Crystal's words (CharcoalKilnApprenticeText2), after the
  FARFETCH'D is back (FLAG_QZ_0x1008); keep 0x1030 for HM01 only. Also add the kiln's FARFETCH'D object ("Kwaa!").
- Notes: CHARCOAL is still found in a RUINS item room, so not unobtainable. Confidence high.

### J1-3: JESSIE & JAMES (Quetzal's postgame visitors) give a Pokémon from a variable nothing sets
- Maps: Johto_NewBarkTown, Johto_CherrygroveCity, Johto_VioletCity, Johto_AzaleaTown (objects with
  `Quetzal_EventScript_83C2D48`, FLAG_QZ_0x1490); shared script data/scripts/qz/johto_shared.inc:787-832
- Category: A + C
- Original: Crystal has no JESSIE / JAMES / MEOWTH / WOBBUFFET.
- Now: after JOHTO's LEAGUE, qz_specials.c TravelingEvent shows them in one town a day; after the battle
  (TRAINER_QZ_913) the script does `copyvar VAR_0x8000, VAR_QZ_0x5073` / `qz_givemon_ex VAR_0x8000, 50, ...`
  (johto_shared.inc:803-804). VAR_QZ_0x5073 is only ever set to 0 (johto_shared.inc:821) and nothing in src/ sets it
  (Quetzal's own special picked the species; Kai's TravelingEvent does not), so ScriptGiveMon is called with species 0.
  Also typo "Wobuffet:" in their motto (data/scripts/qz/shared.inc:519).
- Restore: vanilla: remove the four objects from each town and the travelling-event call (`special
  QzJohtoTravelingEventCheck` in ON_TRANSITION / Quetzal_EventScript_83C2D44). If kept: set the reward species
  (PIKACHU, as the text says) before the givemon.
- Notes: brief lists "Jessie & James" as an unwanted Quetzal addition. Confidence high on the variable (grep of data/,
  src/, include/); medium on the in-game symptom (an empty-species Pokémon given).

### J1-4: ELM's S.S. TICKET words describe Quetzal's travel ("New region in the START menu", "already won the League")
- Maps: Johto_ElmsLab
- Category: B
- Original: ElmsLab.asm:1182-1205 ElmGiveTicketText1/2: "...See? It's an S.S.TICKET. Now you can catch POKéMON in
  KANTO. The ship departs from OLIVINE CITY... Give my regards to PROF.OAK in KANTO!"
- Now: data/maps/Johto_ElmsLab/scripts.inc:964-974: "It's an S.S. Ticket!" / "Keep it to sail from OLIVINE to regions
  where you have already won the League. To begin another adventure, use New region in the START menu." Kai has no
  "New region" START option; its boats go to every other region once the current region's LEAGUE is won
  (src/qz_specials.c:1100 QzGetReachableRegionsMask, first boat lands at the port: MP_Region_FirstBoatIn).
- Restore: Crystal's two texts (OAK / KANTO wording; optionally "other regions" since Kai's boat also goes to HOENN /
  SINNOH). Crystal delivers it by a phone call first; NEW BARK's on-frame "A message from PROF. ELM arrived" (see J1-27)
  could become ELM's POKéGEAR call.
- Notes: confidence high.

### J1-5: EARL's ACADEMY teaches Quetzal's rules; Crystal's lessons are gone
- Maps: Johto_EarlsPokemonAcademy
- Category: B + D
- Original: EarlsPokemonAcademy.asm:14 (EARL's "How to win / raise POKéMON" lessons with YES/NO), :63 the blackboard
  (status conditions: POISON, PARALYSIS, SLEEP, BURN, FREEZE menu), :124 the kid's notebook (4 pages), the two Game Boy
  kids ("I traded my best POKéMON...", "...holding something!"), the note-taking youngster ("A POKéMON holding a BERRY
  will heal itself...").
- Now: data/maps/Johto_EarlsPokemonAcademy/scripts.inc:72-150: EXP. SHARE on/off balance lecture, LARVITAR EXP talk,
  "MACHOKE and GRAVELER evolve at LVL 38, KADABRA and HAUNTER at LVL 42" (Kai: trade evolutions by LINKING CORD,
  COOP_NOTES "Gens 5-9"), "Notable POKéMON ... can be seen in the overworld", "BERRIES in KANTO reduce stats / the
  DOJO in KANTO", "opponents now use teams with better type coverage", blackboard "HMs, once unlocked, don't need to be
  taught" (Kai: a party Pokémon must know the move or hold the HM, REGIONS_TODO Decisions). Only EARL's intro line is
  Crystal's.
- Restore: Crystal's academy scripts and words (EARL's two lessons, the status blackboard with its menu, the notebook,
  the three students); drop the Quetzal blackboards.
- Notes: confidence high.

### J1-6: KURT's first gift is a FAST BALL with "I make BALLS from BERRIES ... batches of 5"
- Maps: Johto_KurtsHouse
- Category: B + C
- Original: KurtsHouse.asm:38-73: after the SLOWPOKE WELL KURT gives a LURE BALL; KurtsHouseKurtBallsFromApricornsText
  "I make BALLS from APRICORNS. Collect them from trees and bring 'em to me. I'll make BALLS out of them."
- Now: data/maps/Johto_KurtsHouse/scripts.inc:96 gives ITEM_FAST_BALL, then Text_83C5F6D (:349-353) "KURT: I make
  BALLS from BERRIES. Collect them from trees and bring 'em to me in batches of 5." Kai's KURT now takes APRICORNS one
  by one (mp_apricorn.c), so the line contradicts the game. Also "Did that sound come from ILEX FOREST????" (:391).
- Restore: LURE BALL and Crystal's APRICORN line (Kai's own KurtBallsFromApricorns text already exists in the file);
  also Crystal's granddaughter lines (KurtsGranddaughterHelpText / FunText) and the house SLOWPOKE.
- Notes: the GS BALL part is J1-17. Confidence high.

### J1-7: AZALEA GYM's words describe Quetzal's CUT-as-BUG move and a different team
- Maps: Johto_AzaleaGym (+ shared Quetzal_Text_8383BB5)
- Category: B + E
- Original: AzaleaGym.asm: BUGSY "Let me demonstrate what I've learned", FURY CUTTER speech (TM49), the guide on
  FLYING / FIRE moves; Bugsy's METAPOD 14, KAKUNA 14, SCYTHER 16.
- Now: data/maps/Johto_AzaleaGym/scripts.inc:723 "did you know that the move CUT is actually a BUG-type move?",
  :815 "His SCYTHER's CUT always seems to crit!", :826 "His PINECO makes short work of ROCK-type", :835-839 guide
  "BUGSY's bagworm POKéMON ... a new version of CUT that is BUG-type". In Kai CUT is NORMAL, and BUGSY's team
  (src/data/trainer_parties_qz.h sParty_Qz4692, no GEN 3 override) is BUTTERFREE, BUTTERFREE, SCIZOR: no SCYTHER,
  PINECO or WORMADAM. The TWINS' "only one POKéMON" line is Emerald's "GINA:" (johto_shared.inc:2762); these twins are
  AMY & MAY.
- Restore: Crystal's BUGSY intro, guide and twins' words (TM89 U-TURN speech can stay: HGSS's TM, decided). Twins'
  one-POKéMON line without a name (or "AMY:").
- Notes: confidence high.

### J1-8: The ILEX FOREST "HEADBUTT" men talk about Quetzal's summary-screen move management; Crystal's HEADBUTT teacher is gone
- Maps: Johto_IlexForest (objects 6 and 18 -> Quetzal_EventScript_83DC26C, johto_shared.inc:1173)
- Category: B + D
- Original: IlexForest.asm:371-378: the rocker shakes trees, gives TM02 HEADBUTT ("Rattle trees with HEADBUTT.
  Sometimes, sleeping POKéMON fall out.").
- Now: both say "Moves can be managed from the POKEMON summary screen." (johto_shared.inc:2830; also "POKEMON" not
  MOéMON). Kai has no such summary-screen feature. Kai's HEADBUTT trees need a party Pokémon that knows HEADBUTT
  (COOP_NOTES "HEADBUTT and ROCK SMASH"), and HEADBUTT is not a TM here (fix_johto_tms.py), so it can't be learned there.
- Restore: one man with Crystal's words teaching HEADBUTT on Kai's move-tutor flow (as KANTO's FireRed tutors,
  fix_move_tutors.py); remove the second copy.
- Notes: confidence high.

### J1-9: TUSCANY and FRIEDA play Quetzal's "guess my favourite day" instead of appearing on their day
- Maps: Johto_Route29 (TUSCANY), Johto_Route32 (FRIEDA)
- Category: C
- Original: Route29.asm:167-197 TUSCANY is there on Tuesdays only (object EVENT_ROUTE_29_TUSCANY_OF_TUESDAY;
  "Today is not Tuesday. That is unfortunate…"), gives PINK BOW; Route32.asm:452-458 FRIEDA on Fridays, POISON BARB.
- Now: data/maps/Johto_Route29/scripts.inc:82 / Johto_Route32/scripts.inc:172 `multichoice MULTI_QZ_0x4A` (pick a
  weekday; wrong answer "Not quite!" / "FRIEDA: Not that one!"); MeetTuscany / MeetFrieda rewritten ("I'm ... of the
  WEEKDAY FAMILY ... If you can guess my favorite day"). Gifts: SILK SCARF (the Gen 3 PINK BOW, fine), POISON BARB.
- Restore: Crystal's weekday check (`gettime` / the RTC weekday; Kai's clock is the PC's) and Crystal's words; keep
  SILK SCARF for PINK BOW. The other five siblings (Routes 36-43, Blackthorn) are other areas: same pattern likely.
- Notes: confidence high.

### J1-10: NEW BARK's Route 27 crossing is blocked until Quetzal's LUGIA / Kimono Girl chain is done
- Maps: Johto_NewBarkTown (object 1, coord events 24,11-25,15), Johto_ElmsLab (VAR_QZ_0x5004 8-10 texts)
- Category: C
- Original: Crystal has no guard: ROUTE 27 is reached from NEW BARK by SURF whenever the player has it; only VICTORY ROAD's
  gate checks the 8 badges.
- Now: data/maps/Johto_NewBarkTown/scripts.inc:179-232 a guard ("Wait! That crossing leads to the POKéMON LEAGUE. You
  still have things to do in JOHTO") until VAR_QZ_0x501A >= 8, which only Johto_WhirlIslandLugiaChamber sets (after the
  Kimono Girls, TIN TOWER and LUGIA). ELM's lab carries the chain: Johto_ElmsLab/scripts.inc VAR 0x5004 7-10 (after the
  MASTER BALL: "KIMONO GIRLS are waiting ... DANCE THEATER", "gone to ECRUTEAK's BELL TOWER", "WHIRL ISLANDS ... You
  still need to meet LUGIA", "...face the LEAGUE now").
- Restore: remove the guard and its triggers (and ELM's chain lines once the Kimono chain is decided in J2/J3); ELM's
  later lines as Crystal's (ElmsLab.asm).
- Notes: depends on the J2/J3 decision about the HGSS-style Kimono / LUGIA-before-LEAGUE chain (docs/quetzal/JOHTO.md
  section 12). Confidence high on the gate.

### J1-11: ROUTE 32's guard demands Elder LI; Crystal only wants the ZEPHYRBADGE
- Maps: Johto_Route32
- Category: C
- Original: Route32.asm:51 the cooltrainer stops the player until ENGINE_ZEPHYRBADGE (and sends them to the aide for the
  EGG), then gives the MIRACLE SEED.
- Now: data/maps/Johto_Route32/scripts.inc:263-264 `goto_if_unset FLAG_QZ_0x1082 / FLAG_QZ_0x1004` -> Text_83EEEC4
  (:402) "Before going on, you must defeat the elder, LI. He awaits you at the top of SPROUT TOWER." before the badge
  check.
- Restore: drop the two SPROUT TOWER checks; keep Crystal's badge / EGG / MIRACLE SEED logic (already there).
- Notes: medium priority. Confidence high.

### J1-12: JOHTO has no hidden items at all
- Maps (this area): AzaleaTown (FULL HEAL), DarkCaveVioletEntrance (ELIXER), IlexForest (ETHER, SUPER POTION, FULL
  HEAL), Route28 (RARE CANDY), Route30 (POTION), Route32 (GREAT BALL, SUPER POTION), VioletCity (HYPER POTION)
- Category: D
- Original: `hiddenitem` bg_events in those maps (e.g. VioletCity.asm:302, Route30.asm bg_event 14,9).
- Now: no `bg_hidden_item_event` in any data/maps/Johto_*/events.inc (KANTO has them in 79 maps); quetzal_rip's JOHTO
  maps have none either: Quetzal dropped them.
- Restore: add Crystal's hidden items at matching spots of Quetzal's redrawn maps (positions need choosing per map).
- Notes: region-wide (61 Crystal maps have hidden items); the other JOHTO areas should list theirs. Confidence high.

### J1-13: Item balls are Quetzal's, not Crystal's (non-TM ones too)
- Maps / Now -> Original (tmp_J1/items_out.txt):
  - DarkCaveVioletEntrance: POTION, REVIVE, MAX REVIVE, ELIXIR -> POTION, FULL HEAL, HYPER POTION, DIRE HIT
  - DarkCaveBlackthornEntrance: BLACK FLUTE, STAR PIECE -> REVIVE, TM SNORE
  - IlexForest: SUN STONE, FULL HEAL, ETHER, SUPER POTION -> REVIVE, X ATTACK, ANTIDOTE, ETHER
  - Route26South: none -> MAX ELIXER
  - Route27: RARE CANDY, MOON STONE, SHINY STONE, REVIVE -> TM SOLARBEAM, RARE CANDY
  - Route30: none -> ANTIDOTE;  Route31: POKé BALL, ANTIDOTE -> POTION, POKé BALL
  - Route32: SUPER POTION, GREAT BALL, POTION, GREAT BALL -> GREAT BALL, REPEL
  - Route46: REVIVE -> X SPEED
  - RuinsOfAlphChambers (item rooms): LUM, CHARCOAL, MAX REVIVE, SACRED ASH, SITRUS, PECHA, ORAN, LUM, MOON / WATER
    STONE, STAR PIECE, STARDUST, HEAL POWDER x2, ENERGY ROOT / POWDER -> GOLD BERRY x2, MYSTERYBERRY x2, REVIVAL HERB,
    CHARCOAL, BERRY, PSNCUREBERRY, MYSTIC WATER, STARDUST, STAR PIECE, MOON STONE, HEAL POWDER x2, ENERGY ROOT /
    ENERGYPOWDER
  - SlowpokeWellB1F: SUPER POTION x2, GREAT BALL, FULL HEAL -> SUPER POTION;  SlowpokeWellB2F: KING'S ROCK -> TM RAIN DANCE
  - SproutTower2F: X DEFEND -> X ACCURACY;  UnionCaveB1F: NUGGET, X DEFEND -> TM SWIFT, X DEFEND;
    UnionCaveB2F: ETHER, MAX REVIVE, HYPER POTION -> ELIXER, HYPER POTION
  - VioletCity: PECHA BERRY, HYPER POTION, RARE CANDY -> PP UP, RARE CANDY (+ hidden HYPER POTION)
  - RuinsOfAlphInnerChamber: HELIX / DOME FOSSIL, OLD AMBER, GS BALL -> none (see J1-16, J1-17)
  - Same as Crystal: Route29, Route31's POKé BALL, SproutTower1F / 3F, UnionCave1F.
- Category: C
- Original: each map's `itemball` objects.
- Now: data/maps/<map>/scripts.inc STD_FIND_ITEM scripts.
- Restore: Crystal's items in Quetzal's ball positions (non-TM; the TM balls stay Quetzal's per fix_johto_tms.py, a
  decision already made). Check tools/moemon_gen59/item_audit.py afterwards (MAX REVIVE, SACRED ASH, SHINY / SUN STONE,
  BLACK FLUTE... may need another source).
- Notes: low-medium priority. Confidence high.

### J1-14: Rival battles in CHERRYGROVE and AZALEA can't be lost without whiting out; his "loss" line is gone
- Maps: Johto_CherrygroveCity, Johto_AzaleaTown
- Category: D
- Original: CherrygroveCity.asm:120-163 / AzaleaTown.asm:67-85 `winlosstext RivalCherrygroveWinText,
  RivalCherrygroveLossText` and the story goes on either way ("Humph. That was a waste of time." / "…Humph! I knew you
  were lying.").
- Now: data/maps/Johto_CherrygroveCity/scripts.inc:205 and Johto_AzaleaTown/scripts.inc:189 use
  TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT with only the defeat text: a loss is a normal white-out, the after-battle scene
  never plays for a loser.
- Restore: KANTO already has FireRed's no-whiteout early-rival battle (COOP_NOTES 1.61.6, qz_trainerbattle 13 /
  EventScript_QzEarlyRivalBattle with a "rival wins" text); use it for these two with Crystal's loss lines.
- Notes: confidence medium-high (the white-out is Emerald's normal trainer-battle path).

### J1-15: Kyle's trade gives SPHEAL for BELLSPROUT (Crystal: ONIX)
- Maps: Johto_VioletKylesHouse
- Category: C
- Original: VioletKylesHouse.asm `trade NPC_TRADE_KYLE` (Crystal's data/events/npc_trades.asm, not in the local clone:
  KYLE gives ONIX "ROCKY" for a BELLSPROUT).
- Now: src/data/qz_ingame_trades.h entry [4]: SPHEAL "SPHEAL", OT QUETZAL, for BELLSPROUT; text "Hi, how's my SPHEAL
  doing?" (data/maps/Johto_VioletKylesHouse/scripts.inc).
- Restore: ONIX, nickname ROCKY, OT KYLE in tools/quetzal_port/gen_trades.py's output; Crystal's trade dialogue.
- Notes: the other J1-relevant trade entries were not checked against Crystal (outside the area). Confidence
  medium-high (from memory of Crystal's table).

### J1-16: RUINS OF ALPH: DUDUNSPARCE tunnel and ARCEUS chamber (Quetzal); its battle is now an UNOWN that keeps coming back
- Maps: Johto_RuinsOfAlphOutside (object 18, warp 34,10), Johto_RuinsOfAlphArceusChamber
- Category: A + C
- Original: none of it exists in Crystal.
- Now: after JOHTO's LEAGUE (FLAG_QZ_0x11CA) the DUDUNSPARCE object (gfx DUDUNSPARCE, "drilling through the wall")
  starts `special QzSetupUnownStaticEncounter` (data/maps/Johto_RuinsOfAlphOutside/scripts.inc:87) which Kai implements
  as UNOWN Lv 40 (src/qz_specials.c:518-525; Quetzal's special 707 was species 0x544 Lv 70 with an item,
  docs/quetzal/SPECIALS.md:180). Afterwards QzClearStaticFlagAfterWildBattle *clears* FLAG_QZ_0x1313, the object's own
  hide flag (qz_specials.c:400-405), so it is back on the next visit and can be fought forever. The tunnel leads to
  Johto_RuinsOfAlphArceusChamber: ARCEUS (hidden as SINNOH's) and the GIOVANNI-story item (QzGiovanniStoryItemAvailable;
  the story never starts any more).
- Restore: remove the DUDUNSPARCE object, the warp at (34,10) and the ARCEUS chamber map from use.
- Notes: if kept instead: set (not clear) 0x1313 after the battle and use DUDUNSPARCE. Confidence high.

### J1-17: The GS BALL lies in the RUINS before the LEAGUE; CELEBI can be called again and again
- Maps: Johto_RuinsOfAlphInnerChamber (object 7), Johto_RuinsOfAlphChambers (:205 clears its flag after the HO-OH
  puzzle), Johto_KurtsHouse, Johto_AzaleaTown, Johto_IlexForest
- Category: C
- Original: Crystal (Virtual Console): GoldenrodPokecenter1F.asm SCENE_GOLDENRODPOKECENTER1F_GS_BALL gives the GS BALL
  after the HALL OF FAME; KURT keeps it a day, gives it back in AZALEA; ILEX's shrine calls CELEBI once.
- Now: GS BALL is an item ball in the INNER CHAMBER once the HO-OH panel puzzle is solved; KURT examines it at once and
  hands it back outside (no day), and Ilex's shrine offers "The GS BALL is still in the shrine. Call CELEBI again?"
  (Johto_IlexForest/scripts.inc 83DC2C4) after a run / loss.
- Restore: GS BALL from GOLDENROD's POKéMON CENTER after JOHTO's HoF (J2's map), KURT's day wait (Crystal's
  KurtsHouseKurtImCheckingItNowText / AhHaISeeText), one CELEBI encounter.
- Notes: CELEBI is JOHTO's legendary, so the event itself stays. Confidence high.

### J1-18: RUINS RESEARCH CENTER is Quetzal's fossil regenerator; Crystal's UNOWN research is missing
- Maps: Johto_RuinsOfAlphResearchCenter, Johto_RuinsOfAlphInnerChamber (fossil balls), Johto_RuinsOfAlphOutside
- Category: C + D
- Original: RuinsOfAlphOutside.asm:54 the scientist who sees the first UNOWN and takes the player to the lab;
  RuinsOfAlphResearchCenter.asm:76.. the POKéDEX UNOWN upgrade, scientists' UNOWN lines, the computer ("Exploration Year
  10" / UNOWN count), the UNOWN printer, PROF.SILKTREE's photo, academic books.
- Now: a "FOSSIL REGENERATOR" (all generations, combining fossils) and fossil balls in the INNER CHAMBER (HELIX, DOME,
  OLD AMBER); the plate shop is already gone (REGIONS_TODO).
- Restore: Crystal's scientists and signs (the UNOWN POKéDEX mode itself isn't in Kai; its words can go or be adapted);
  remove the regenerator and the INNER CHAMBER fossils (fossils have their own sources in HOENN / KANTO).
- Notes: check item_audit / obtain_audit (fossil POKéMON sources) before removing. Confidence high.

### J1-19: SLOWPOKE WELL B2F: Quetzal's SLOWKING / DAMP ROCK instead of Crystal's KING'S ROCK man; B1F's SLOWPOKE with MAIL missing
- Maps: Johto_SlowpokeWellB2F, Johto_SlowpokeWellB1F
- Category: C + D
- Original: SlowpokeWellB2F.asm:17 the man waiting for SLOWPOKE to evolve gives KING'S ROCK (and a TM RAIN DANCE ball);
  SlowpokeWellB1F.asm:96 a tailless SLOWPOKE carrying MAIL ("Be good and look after the house with Grandpa and
  SLOWPOKE. Love, Dad"), another tailless SLOWPOKE.
- Now: data/maps/Johto_SlowpokeWellB2F/scripts.inc: a SLOWKING NPC dances, makes it rain and gives DAMP ROCK (:41);
  KING'S ROCK is a plain item ball (:7). No SLOWPOKE objects on B1F.
- Restore: Crystal's KING'S ROCK man and words; SLOWPOKE objects with the MAIL on B1F.
- Notes: confidence high.

### J1-20: ROUTE 29's catching demonstration is a text plug for the ACADEMY; its night line can never show
- Maps: Johto_Route29
- Category: D + A
- Original: Route29.asm: the dude catches a wild RATTATA in a scripted catching tutorial ("That's how you do it...");
  Route29.asm:148-165 the COOLTRAINER says "waiting for night" by day and "waiting for morning" at night.
- Now: data/maps/Johto_Route29/scripts.inc 83ECC5B: "If you weaken them first ... go to the ACADEMY in VIOLET CITY!"
  (no demo). :140 `goto_if_set FLAG_QZ_0x1015` (nothing in data/ or src/ sets 0x1015) so he always says Quetzal's
  "waiting for MOéMON that appear only later in the day".
- Restore: Emerald's WALLY catching-tutorial battle (it exists in the engine) with Crystal's words; the time check with
  `gettime` (Kai has MORNING / DAY / NIGHT: mp_region.c MP_TimeLandSlot hours) and Crystal's two lines.
- Notes: confidence high.

### J1-21: Kimono Girls ZUKI (VIOLET) and NAOKO (ILEX) scenes (HGSS-style arc)
- Maps: Johto_VioletCity (object 11, Text_83FEF92-83FF076), Johto_IlexForest (object 17, 83DC433), triggered from
  Johto_VioletPokecenter1F (83FFDFA clears FLAG_QZ_0x108B)
- Category: C
- Original: not in Crystal (HGSS has ZUKI and NAOKO in these places).
- Now: ZUKI watches the EGG ("Me oh my... That EGG must be...!") and NAOKO is escorted out of ILEX.
- Restore: remove both with the rest of the Kimono / LUGIA chain, or keep as HGSS (priority Crystal + HGSS) if J2/J3
  keep the chain; decide together with J1-10.
- Notes: confidence high on presence.

### J1-22: Trainer teams are Quetzal's (gym leaders included), and the VIOLET gym guide describes another team
- Maps: all trainer maps; examples Johto_VioletGym, Johto_AzaleaGym
- Category: C (+E)
- Original: Crystal parties (data/trainers/parties.asm, not in the clone): FALKNER PIDGEY 7, PIDGEOTTO 9; BUGSY METAPOD
  14, KAKUNA 14, SCYTHER 16. VioletGym.asm:253-272 guide: "The grass-type is weak against the flying-type."
- Now: FALKNER at GEN 3 = PIDGEY 8, SKARMORY 11 (src/data/trainer_parties_qz_gen2.h sParty_Qz4115); BUGSY = BUTTERFREE
  15, BUTTERFREE 15, SCIZOR 16 (trainer_parties_qz.h). Guide (data/maps/Johto_VioletGym/scripts.inc:179) "FALKNER's ace is
  a bulky owl that loves to eat BERRIES ... watch out for MUD-SLAP", which matches neither.
- Restore: Crystal's parties at GEN 3 (a table like KANTO's sQzBlueBattles / FireRed teams), Crystal's guide words.
- Notes: large job; Kai's level scaling sits on top either way. Confidence high on the data.

### J1-23: Quetzal "viewpoint" camera pans fire on every step onto their tiles
- Maps: Johto_Route29 (coord 4,9, 83ECBA0), Johto_UnionCave1F (coord 33,32, 83FDB22)
- Category: C
- Original: none.
- Now: camera pans 4 tiles, "Press A or B to stop viewing.", pans back and steps the player off; VAR_TEMP_0 is never set,
  so it repeats on every visit to the tile.
- Restore: remove both coord events.
- Notes: low. Confidence high.

### J1-24: GIOVANNI's alternate-past remnants (dead since 1.68.1) still sit in these maps
- Maps: Johto_NewBarkTown (8403935-8403A3E, SILVER fight TRAINER_QZ_4988), Johto_PlayersHouse1F (84019F4.., ROCKET /
  EEVEE, TRAINER_QZ_4987, object 3-4), Johto_CherrygroveCity (objects 10-11), Johto_AzaleaTown (objects 14-15),
  Johto_ElmsLab (ARCEUS egg 8401D20, object 8), Johto_RuinsOfAlphArceusChamber
- Category: C
- Original: none.
- Now: unreachable (QzGiovanniStoryCanStart FALSE) except for saves already in it (COOP_NOTES "it never begins").
- Restore: keep for old saves (the user's 1.68.1 choice) or delete once no save needs it. Listed for completeness.
- Notes: no action needed now. Confidence high.

### J1-25: Overworld legendaries of Gens 5-9 in these maps (kept by decision)
- Maps: AzaleaTown TAPU BULU; ElmsLab POIPOLE; IlexForest XERNEAS, ZARUDE; Route29 OGERPON; Route33 ZERAORA (and its
  thunderstorm); RuinsOfAlphChambers REGIELEKI, REGIDRAGO; RuinsOfAlphOutside KORAIDON; RuinsOfAlphResearchCenter
  GENESECT; SproutTower1F-3F MARSHADOW, SPECTRIER, KARTANA; UnionCaveB1F TERAPAGOS; UnionCaveB2F ZYGARDE. (Hidden as
  foreign: GIRATINA, MEW, LATIOS, LATIAS, REGIROCK / ICE / STEEL, REGIGIGAS, UXIE, ARCEUS, DARKRAI.)
- Category: C
- Original: none in Crystal.
- Now: shown after JOHTO's LEAGUE (qz_specials.c RefreshStatics).
- Restore: keep (REGIONS_TODO Decisions: "Gens 5-9 legendaries have no home region here and stay"). Listed only.
- Notes: confidence high.

### J1-26: NEW BARK / home: Crystal's people and lines missing, Quetzal's scenes added
- Maps: Johto_NewBarkTown, Johto_PlayersHouse1F, Johto_PlayersHouse2F, Johto_PlayersNeighborsHouse
- Category: D (+C)
- Original: NewBarkTown.asm: the FISHER ("Yo, PLAYER! I hear PROF.ELM discovered some new POKéMON."), the TEACHER's
  "Wow, your POKéGEAR is impressive! Did your mom get it for you?" (before the starter) and "Call your mom on your
  POKéGEAR..." (after the EGG quest), her stop scene ("Wait, PLAYER!" / "What do you think you're doing?", walking the
  player back). PlayersHouse1F.asm: MOM's "So, what was PROF. ELM's errand? ... you should be proud that people rely on
  you." (after the starter), MOM standing at the stove / table / sink by time of day, the neighbour's
  morning / day / evening greeting, STOVE / SINK / FRIDGE / TV signs (:399-402); her POKéGEAR scene ends with the PHONE
  instructions. PlayersHouse2F.asm and PlayersNeighborsHouse.asm: the RADIO (OAK's POKéMON TALK opening).
- Now: NEW BARK has no fisher (object 1 is the ROUTE 27 guard), the teacher's first line is "Don't forget to visit your
  mom from time to time.", her stop scene is one line and one step; she also stops the player at the west exit after the
  EGG quest until MOM is talked to (coord events at 0,12-14 with VAR_QZ_0x5003 == 4; Crystal only says it when talked
  to). Johto_PlayersHouse1F has no bg events at all; MOM says "PROF.ELM is waiting for you" until the EGG is returned;
  her later default is Emerald's "You look a little tired... rest" heal. The bedroom has Emerald's "The clock is
  stopped… Better set it and start it!" scene with StartWallClock (2F coord 9,3 / 10,3; Kai's clock is the PC's) and
  Quetzal's bed (heal, and the disabled DARKRAI dream), no radio.
- Restore: Crystal's FISHER, teacher lines and stop scene; MOM's errand line; the four kitchen signs; the radios (Kai's
  POKéGEAR already has OAK's show: mp_pokegear.c); drop the wall-clock scene; keep MOM's heal only if wanted (HGSS's MOM
  heals too).
- Notes: low-medium. Confidence high.

### J1-27: ELM's phone calls arrive as on-screen "A message from PROF. ELM arrived" texts
- Maps: Johto_Route30 (theft call, coord 34-36,5-6), Johto_VioletCity (EGG call, Text_83FEEFF), Johto_NewBarkTown (HoF
  call, 83E139C)
- Category: C
- Original: ELM rings the POKéGEAR (Crystal's phone scripts: "It's terrible!", "We discovered something about the EGG",
  the S.S. TICKET call).
- Now: on-frame message boxes. Kai now has a POKéGEAR with ELM in the phone book (mp_pokegear.c, mp_qzphone.c).
- Restore: play them as incoming POKéGEAR calls from ELM (Crystal's words), if mp_qzphone can ring a scripted call.
- Notes: low. Confidence high on the difference.

### J1-28: ROUTE 27 SANDSTORM house gives TM37 without Crystal's friendship check
- Maps: Johto_Route27SandstormHouse
- Category: D
- Original: Route27SandstormHouse.asm:9-37: "Are your POKéMON loyal enough...? Let me see…", the lead's happiness must
  be 150+, else "If it doesn't come to trust you some more, it could be tough going."
- Now: data/maps/Johto_Route27SandstormHouse/scripts.inc: always "Ah! Your POKéMON trusts you very much." and TM37.
- Restore: check the lead's friendship (Emerald's GetLeadMonFriendshipScore special) and Crystal's two lines.
- Notes: confidence high.

### J1-29: UNION CAVE B2F's Friday LAPRAS is missing
- Maps: Johto_UnionCaveB2F
- Category: D
- Original: UnionCaveB2F.asm:16-34, 158: a LAPRAS (Lv 20) surfs there on Fridays, once a week.
- Now: no object; LAPRAS is only in the water table (docs/quetzal/JOHTO.md section 4).
- Restore: Crystal's weekly LAPRAS static (Friday by the PC clock).
- Notes: medium. Confidence high.

### J1-30: CHERRYGROVE: the rival's "That's my TRAINER CARD! You saw my name..." line and the guide's MAP gift words
- Maps: Johto_CherrygroveCity
- Category: C + D
- Original: CherrygroveCity.asm: the rival only says "My name's ???..." (the name comes at ELM's lab); the guide gent
  gives the MAP CARD ("PLAYER's POKéGEAR now has a MAP!", "POKéGEAR becomes more useful as you add CARDS"); the teacher
  asks "Did you talk to the old man by the POKéMON CENTER? He'll put a MAP of JOHTO on your POKéGEAR." until then; a
  GUIDE GENT'S HOUSE sign. Crystal's guide is talked to; Quetzal's stops the player on entering (coord 57,9-11).
- Now: data/maps/Johto_CherrygroveCity/scripts.inc:481 Text_83C9475 (TRAINER CARD) after the battle (Kai now names him at
  ELM's: REGIONS_TODO); guide ends "If you're ever lost, check your REGION MAP!"; teacher always says the have-map line.
- Restore: drop the TRAINER CARD line; Crystal's guide words (Kai's POKéGEAR MAP is always there, so the "now has a
  MAP" line can just be said), the house sign.
- Notes: low. Confidence high.

### J1-31: Marts' and Centers' talkers explain Quetzal's badge marts / other Quetzal lines
- Maps: Johto_CherrygroveMart, Johto_AzaleaMart, Johto_CherrygrovePokecenter1F, Johto_VioletPokecenter1F
- Category: B (low) / C
- Original: CherrygroveMart.asm "They're fresh out of POKé BALLS!" -> "POKé BALLS are in stock!" (with the POKéDEX);
  AzaleaMart.asm "There's no GREAT BALL here. POKé BALLS will have to do. I wish KURT would make me some of his custom
  BALLS."; CherrygrovePokecenter1F.asm teacher on the COMMUNICATION CENTER upstairs; VioletPokecenter1F.asm youngster.
- Now: "You can buy POKé BALLS even before earning your first Badge." / "...more items become available to buy when you
  earn GYM BADGES!" (Quetzal's badge marts; Kai sells Crystal's lists: src/shop.c:1481-1486, CHERRYGROVE's balls with the
  POKéDEX: :1601); "They say a watched pot never boils..."; "While my MOéMON are recovering I love to read a good book."
  Also the VIOLET aide's "make room in your party" while the script checks the current PC box (QzIsCurrentPcBoxFull,
  data/maps/Johto_VioletPokecenter1F/scripts.inc), refusing the TOGEPI EGG when that box is full even with party room;
  Crystal checks the party (VioletCityElmsAideFullPartyText).
- Restore: Crystal's lines (they match Kai's Crystal mart lists); the egg check on the party only with Crystal's words.
  The COMMUNICATION CENTER line may stay changed (Kai's upstairs is the GARDEN).
- Notes: low. Confidence high.

### J1-32: Dead NUZLOCKE branches in gift scripts
- Maps: Johto_VioletPokecenter1F (Text_83FFE11), Johto_Route32, Johto_ElmsLab (QzIsNuzlockeOption2D4Bit6),
  Johto_RuinsOfAlphResearchCenter; shared Quetzal_EventScript_8361A4D / Quetzal_Text_8361A68 (shared.inc:125, 487)
- Category: B
- Original: none.
- Now: QzIsNuzlockeActiveInRegion / QzIsNuzlockeOption2D4Bit6 always return FALSE (src/qz_specials.c:62-63), so the
  "NUZLOCKE challenge" texts never show.
- Restore: remove the branches and texts (harmless now).
- Notes: low. Confidence high.

### J1-33: SPROUT TOWER's ELDER LI gives HM05 with Quetzal's words
- Maps: Johto_SproutTower3F
- Category: D (low)
- Original: SproutTower3F.asm SageLiTakeThisFlashText "...Take this FLASH HM." and SageLiFlashExplanationText ("FLASH
  illuminates even the darkest of all places. But to use it out of battle, you need the BADGE from VIOLET's GYM.").
- Now: data/maps/Johto_SproutTower3F/scripts.inc 83FA8B7: "You and your MOéMON have passed the tower's test." / "Trust
  in that bond to face the challenges ahead." around EventScript_GiveHmOnce.
- Restore: Crystal's two texts (KANTO's HM givers already use FireRed's).
- Notes: confidence high.

### J1-34: Small missing people, signs and lines (Crystal has them, the game doesn't)
- Category: D (low each)
- Johto_ElmsLab: window ("The window's open..." / "He broke in through here!"), the four TRAVEL TIP books, trash can,
  ELM's PC ("OBSERVATIONS ON POKéMON EVOLUTION"), "I thought the EGG hatched. Where is the POKéMON?"; Quetzal's extra
  aide ("There are only three of us" / "save your game regularly", ElmsLab.asm has two). Typo "In VIOLET CITY!"
  (scripts.inc:724, capital I mid-sentence).
- Johto_ElmsHouse: ELM's research-paper PC (bg 0,1 shows a bookshelf line).
- Johto_MrPokemonsHouse: "It's a big computer. Hmm. It's broken."
- Johto_Route32RuinsOfAlphGate: the OFFICER ("RUINS OF ALPH A Look-and-Touch Tourist Site") and POKéFAN M.
- Johto_VioletCity: VIOLET CITY / SPROUT TOWER / EARL'S ACADEMY signs (VioletCity.asm:296-299); the LASS's and SUPER
  NERD's lines rewritten (GHOST vs ONIX tip; ACADEMY plug instead of "If you beat the GYM LEADER here...").
- Johto_Route31VioletGate: COOLTRAINER F "I came too far out. I'd better phone home!" (now a PIKACHU look-alike line).
- Johto_Route31: KENYA's sleepy man: "This MAIL isn't for me.", "...It doesn't have any MAIL.", "What? You don't want
  anything?", "If I take that POKéMON from you, what are you going to use in battle?" (EVIOLITE for TM50 stays: decided).
- Johto_Route33: ROUTE 33 sign; the LASS's line edited ("exploring for MARILL").
- Johto_AzaleaTown: the SLOWPOKE and WOOSTER objects; Johto_CharcoalKiln: FARFETCH'D; Johto_VioletNicknameSpeechHouse:
  STRAWBERRY the PIDGEY; Johto_Route28SteelWingHouse: the celebrity's FEAROW; Johto_KurtsHouse: the SLOWPOKE.
- Johto_RuinsOfAlphChambers: Crystal's chamber scientists' lines (cryptic patterns, tremor, "another huge hole"), the
  wall-pattern and hole texts (Quetzal's own scientist lines and HGSS-style lead-POKéMON sensors instead).
- Johto_UnionCave1F: "UNION CAVE" sign is unused in Crystal too (ignore).
- Johto_Route26: "MOéMON are in-valuable" (typo, scripts.inc:27).
- Original: the named Crystal maps. Now: absent / as quoted. Restore: add them with Crystal's words.
- Notes: confidence high.

---

## Coverage
- Johto_AzaleaGym: J1-7, J1-22
- Johto_AzaleaMart: J1-31
- Johto_AzaleaPokecenter1F: only trivial differences ("30 per BOX": Kai's real box size)
- Johto_AzaleaTown: J1-3, J1-12, J1-14, J1-17, J1-24, J1-25, J1-34
- Johto_CharcoalKiln: J1-2, J1-34
- Johto_CherrygroveCity: J1-3, J1-14, J1-24, J1-30
- Johto_CherrygroveEvolutionSpeechHouse: same as the original
- Johto_CherrygroveGymSpeechHouse: same as the original
- Johto_CherrygroveMart: J1-31
- Johto_CherrygrovePokecenter1F: J1-31
- Johto_DarkCaveBlackthornEntrance: J1-13, J1-25 (DARKRAI dream object, disabled)
- Johto_DarkCaveVioletEntrance: J1-12, J1-13, J1-25
- Johto_EarlsPokemonAcademy: J1-5
- Johto_ElmsHouse: J1-34
- Johto_ElmsLab: J1-4, J1-10, J1-24, J1-25, J1-32, J1-34
- Johto_GuideGentsHouse: same as the original
- Johto_IlexForest: J1-2, J1-8, J1-12, J1-13, J1-17, J1-21, J1-25
- Johto_IlexForestAzaleaGate: only trivial differences
- Johto_IlexForest_Copy: unreferenced empty duplicate (no events, nothing warps to it); nothing to compare
- Johto_KurtsHouse: J1-6, J1-17, J1-34
- Johto_MrPokemonsHouse: J1-34 (EXP. SHARE trade already Crystal's)
- Johto_NewBarkTown: J1-1, J1-3, J1-10, J1-24, J1-26, J1-27
- Johto_PlayersHouse1F: J1-24, J1-26
- Johto_PlayersHouse2F: J1-26
- Johto_PlayersNeighborsHouse: J1-26
- Johto_Route26: J1-34 (typo); otherwise only trivial differences
- Johto_Route26HealHouse: same as the original
- Johto_Route26South: J1-13, J1-25
- Johto_Route27: J1-13, J1-25 (LATIAS hidden as HOENN's); otherwise only trivial differences
- Johto_Route27SandstormHouse: J1-28
- Johto_Route28: J1-12
- Johto_Route28SteelWingHouse: J1-34 (TM47 already Crystal's)
- Johto_Route29: J1-9, J1-20, J1-23, J1-25
- Johto_Route29Route46Gate: same as the original
- Johto_Route30: J1-12, J1-13, J1-27
- Johto_Route30BerryHouse: same as the original (already Crystal's words)
- Johto_Route31: J1-13, J1-34
- Johto_Route31VioletGate: J1-34
- Johto_Route32: J1-9, J1-11, J1-12, J1-13, J1-32
- Johto_Route32Pokecenter1F: only trivial differences
- Johto_Route32RuinsOfAlphGate: J1-34
- Johto_Route33: J1-25, J1-34
- Johto_Route46: J1-13
- Johto_RuinsOfAlphAerodactylWordRoom: only trivial differences (empty room with warps, as Crystal's)
- Johto_RuinsOfAlphArceusChamber: J1-16, J1-24
- Johto_RuinsOfAlphChambers: J1-13, J1-17, J1-25, J1-34
- Johto_RuinsOfAlphHoOhWordRoom: only trivial differences
- Johto_RuinsOfAlphInnerChamber: J1-13, J1-17, J1-18, J1-25
- Johto_RuinsOfAlphKabutoWordRoom: only trivial differences
- Johto_RuinsOfAlphOmanyteWordRoom: only trivial differences
- Johto_RuinsOfAlphOutside: J1-16, J1-18, J1-25
- Johto_RuinsOfAlphResearchCenter: J1-18, J1-25, J1-32
- Johto_SlowpokeWellB1F: J1-13, J1-19
- Johto_SlowpokeWellB2F: J1-13, J1-19
- Johto_SproutTower1F: J1-25
- Johto_SproutTower2F: J1-13, J1-25
- Johto_SproutTower3F: J1-25, J1-33
- Johto_UnionCave1F: J1-23
- Johto_UnionCaveB1F: J1-13, J1-25
- Johto_UnionCaveB2F: J1-13, J1-25, J1-29
- Johto_VioletCity: J1-3, J1-12, J1-13, J1-21, J1-27, J1-34
- Johto_VioletGym: J1-22 (TM51 ROOST is HGSS's: decided)
- Johto_VioletKylesHouse: J1-15
- Johto_VioletMart: only trivial differences
- Johto_VioletNicknameSpeechHouse: J1-34
- Johto_VioletPokecenter1F: J1-31, J1-32

Shared scripts called from these maps (data/scripts/qz/johto_shared.inc, shared.inc): J1-3 (Quetzal_EventScript_83C2D48,
VAR_QZ_0x5073; "Wobuffet" typo), J1-7 (Quetzal_Text_8383BB5 "GINA:"), J1-8 (Quetzal_Text_83DC904), J1-32
(Quetzal_EventScript_8361A4D). The CUT / ROCK SMASH / STRENGTH, heal, bookshelf and nurse helpers are fine.
