# J2 findings: JOHTO (Goldenrod, Routes 34-41, National Park, Ecruteak, Olivine, Cianwood, Whirl Islands)

Compared against pret/pokecrystal (`/tmp/claude-0/frlg/pokecrystal/maps/*.asm`; the clone has maps only, no data/ or
engine/). Read-only audit, nothing in the repo was changed. Already-decided items from docs/REGIONS_TODO.md are not
reported again (HM givers, leader TMs, JOHTO's other TM givers, Crystal's TM item balls staying Quetzal's items, RADIO
TOWER kept with the RAINBOW WING and Emerald's LOTTERY CORNER, GOLDENROD's prize Pokémon, BILL's EEVEE, Gens 5-9
legendaries staying, apricorns, POKéGEAR / radio, the rival's name), except where the current files show a new problem.

---

### J2-1: Gym victory script sets trainer flags by Quetzal's raw ids: wrong flags and an out-of-bounds RAM write
- Maps: Johto_GoldenrodGym, Johto_EcruteakGym, Johto_OlivineGym, Johto_CianwoodGym (and the other four JOHTO gyms,
  outside J2); Johto_RadioTower5F; shared script in data/scripts/qz/johto_shared.inc
- Category: A / E
- Original: Crystal marks the gym's trainers beaten through its own events (e.g. GoldenrodGym.asm after the WHITNEY
  battle sets EVENT_BEAT_* for each gym trainer).
- Now: every leader script does `setvar VAR_0x8008, n` + `call Quetzal_EventScript_83601F0`
  (GoldenrodGym scripts.inc:88, EcruteakGym:66, OlivineGym:197, CianwoodGym:54). That script
  (johto_shared.inc:264-326) runs `settrainerflag 0x11F6`, `0x1081`... (28 calls) with Quetzal's *raw trainer numbers*
  (0x11F6 = 4598, which Kai calls TRAINER_QZ_4598 = 1044). ScrCmd_settrainerflag passes 4598 to TRAINER_FLAG(), which
  treats any id >= 1664 as a SINNOH trainer: 0x3780 + 4598 - 1664 = 0x42F6. FlagSet(0x42F6) lands in
  sSpecialFlags[(0x42F6-0x4000)/8 = 94], an array of 16 bytes (src/event_data.c:32, :242). Every gym win writes
  1-5 bits up to ~120 bytes past the end of sSpecialFlags in EWRAM. RadioTower5F scripts.inc:362 `cleartrainerflag 0x1362`
  does the same (only reachable in GIOVANNI's story, which old saves can still finish). Olivine's case 6 is empty, so
  Quetzal's two added Olivine gym people are never marked.
- Restore: delete the `call Quetzal_EventScript_83601F0` lines (Kai's mp_coop.c MP_Coop_GymLeaderBeaten, called from
  battle_setup.c:1664, already flags every trainer on a JOHTO gym map), or translate each number to its
  `TRAINER_QZ_<n>` constant. Same for RadioTower5F's cleartrainerflag (TRAINER_QZ_4962). Worth a grep of
  kanto_shared.inc (44 settrainerflag/cleartrainerflag lines there, K-areas) and NewBarkTown / PlayersHouse1F (J1).
- Notes: not in REGIONS_TODO. Memory corruption, so high priority. Confidence high (traced through TRAINER_FLAG,
  GetFlagPointer, SPECIAL_FLAGS_SIZE).

### J2-2: BURNED TOWER B1F calls Emerald's InitRoamer: a Lv 40 LATIOS starts roaming HOENN; JOHTO's beasts don't roam
- Maps: Johto_BurnedTowerB1F (also Johto_EcruteakCity, Johto_OlivineCity for the beasts)
- Category: A / E (and D for the roaming beasts)
- Original: BurnedTowerB1F.asm:83-84 `setevent EVENT_RELEASED_THE_BEASTS` + `special InitRoamMons`: RAIKOU and ENTEI
  roam JOHTO (SUICUNE is chased: Cianwood, Route 42, Tin Tower 1F).
- Now: Johto_BurnedTowerB1F/scripts.inc:108-109 `setvar VAR_0x8004, 1` + `special InitRoamer`. Kai's InitRoamer is
  Emerald's (src/roamer.c:108-113): it clears the save's roamer and creates a Lv 40 LATIOS (0x8004 = 1) roaming HOENN.
  A player who already caught or is chasing HOENN's LATIAS / LATIOS gets their roamer reset and a new LATIOS.
  Meanwhile ENTEI (Johto_EcruteakCity obj 28) and RAIKOU (Johto_OlivineCity) are static overworld encounters shown
  before the League (qz_specials.c PreLeagueLegend).
- Restore: remove `special InitRoamer` (and its setvar). For the vanilla experience, add JOHTO roamers for RAIKOU /
  ENTEI after this scene (Kai's roamer code is HOENN-only; needs a JOHTO roamer table) and hide the static ENTEI /
  RAIKOU objects; otherwise keep the statics as Kai's stand-in.
- Notes: not in REGIONS_TODO. Confidence high for the bug, medium for how far the roamer rework should go.

### J2-3: No DAY-CARE in JOHTO; people say it runs "from any PC", which Kai doesn't do
- Maps: Johto_DayCare, Johto_Route34; shared.inc Quetzal_EventScript_83F08A1 (:208) / 83F0976 (:216), texts
  Quetzal_Text_83F0C34 / 83F09A4 (:532-540)
- Category: B (and C, D)
- Original: DayCare.asm (the DAY-CARE lady inside takes Pokémon, the man inside gives the ODD EGG), Route34.asm:787-789
  (the DAY-CARE man outside with the two guests: DayCareManScript_Outside, DayCareMon1/2Script), the egg flow.
- Now: the man in the house (obj 1) says "The DAY CARE is now managed through the PC. We sell special INCENSES here"
  and opens an incense mart; a Route 34 woman (obj 12) says "The DAY-CARE service is now available from any PC."
  Kai has no PC day care, so JOHTO has no day care at all. The ODD EGG comes from a new "DAY-CARE couple's son"
  (Johto_DayCare/scripts.inc:24-60) with a new text. The book gags are Quetzal's.
- Restore: Crystal's DAY-CARE lady inside and man outside (with the two guests on Route 34) driven by Kai's own DAY
  CARE specials (Emerald's Route 117 scripts: ShowDaycareLevelMenu etc.), Crystal's ODD EGG from the man inside with
  his lines; drop the PC lines and the incense mart (or keep the incenses elsewhere).
- Notes: not in REGIONS_TODO. Kai's DAY CARE must then share one save slot between regions (Emerald's struct): check
  co-op day care eggs. Confidence high.

### J2-4: GOLDENROD FLOWER SHOP girl hands out Gen 8 items (TEACUPs, SCROLLs, armor, WHIPPED DREAM) instead of BERRIES
- Maps: Johto_GoldenrodFlowerShop
- Category: A / E (the NPC itself C)
- Original: Crystal's flower shop has the teacher (SQUIRTBOTTLE) and FLORIA; no gift girl. (The girl and her lines are
  Emerald's PRETTY PETAL shop girl, who gives a random BERRY each day.)
- Now: scripts.inc:16-17 `random 8` / `addvar VAR_RESULT, 0x202` gives item 514-521 = UNREMARKABLE TEACUP,
  MASTERPIECE TEACUP, METAL ALLOY, SCROLL OF DARKNESS, SCROLL OF WATERS, AUSPICIOUS ARMOR, MALICIOUS ARMOR, WHIPPED
  DREAM (include/constants/items.h:612-619): Quetzal's own item number was never translated. FLAG_QZ_0x149C is also
  never cleared, so it isn't daily. Her second line talks about a BERRY-mixing machine.
- Restore: remove the girl (vanilla), or if kept, `random 8` + `addvar VAR_RESULT, ITEM_CHERI_BERRY` (Emerald's form)
  and a daily reset. The two Quetzal berry marts (obj 3, 5: resist berries and 20 berries, scripts.inc:94, :105) are
  also C; Crystal sells no berries here.
- Notes: not in REGIONS_TODO. Confidence high.

### J2-5: WHITNEY's quiz at the RADIO TOWER gates the GOLDENROD GYM; two answers are wrong in Kai; Crystal's RADIO CARD quiz is gone
- Maps: Johto_GoldenrodCity, Johto_RadioTower1F, Johto_GoldenrodGym
- Category: C (gate, quiz) / B + A (answers) / D (RADIO CARD quiz)
- Original: WHITNEY is in her gym from the start (GoldenrodGym.asm). RadioTower1F.asm:114-178: the RADIO CARD woman's
  five questions (morning-only Pokémon, BERRY at a MART, HM01 FLASH, FALKNER, CHARMANDER on the slots) give the RADIO
  CARD.
- Now: Johto_GoldenrodCity/events.inc:26 obj 19 stands at the gym door ("WHITNEY is not here right now. She might be
  at the RADIO TOWER") until FLAG_QZ_0x10A2, set only when the player passes Quetzal's quiz (RadioTower1F
  scripts.inc:63-140, prize METRONOME). The quiz's expected answers: Q3 "Does catching in HOENN register it as caught
  in JOHTO too?" = NO, but Kai's POKéDEX is one for all regions (yes); Q5 "Do you need to give KURT apricorns to buy
  his POKé BALLS?" = NO, but Kai restored Crystal's APRICORNS (yes). A player answering truthfully fails and the gym
  stays shut; Q4 "rename from the party menu" is only roughly true (Kai renames from the summary page).
- Restore: remove obj 19 / set FLAG_QZ_0x10A2 at JOHTO's setup, remove WHITNEY from 1F; put back Crystal's RADIO CARD
  woman with Crystal's five questions and words (the card can be flavor, or the POKéGEAR radio can wait for it as in
  Crystal).
- Notes: not in REGIONS_TODO (the RADIO TOWER item there is about the director). Confidence high.

### J2-6: MAXIE (TIN TOWER roof) and ARCHIE (WHIRL ISLANDS) stand there from the first visit, and their ORBs lead nowhere
- Maps: Johto_TinTowerRoof, Johto_WhirlIslandLugiaChamber, Johto_EmbeddedTower
- Category: A / C
- Original: none (Quetzal postgame: Lv 70 MAXIE / ARCHIE, RED / BLUE ORB, then GROUDON / KYOGRE / RAYQUAZA in the
  EMBEDDED TOWER).
- Now: TinTowerRoof/events.inc obj 9 (FLAG_QZ_0x1314) and WhirlIslandLugiaChamber/events.inc obj 9 (FLAG_QZ_0x1315).
  Quetzal's special 704 hid them until its postgame (docs/quetzal/SPECIALS.md:177, "extra flags 0x1313-0x1315");
  Kai's RefreshStatics (qz_specials.c:492-512) only handles its static table, nothing sets 0x1314 / 0x1315 and the
  region init doesn't list them, so both stand there pre-League ("live up to your title as Champion!") with Lv 70 teams,
  rematchable forever. The ORBs they give unlock the EMBEDDED TOWER's GROUDON / KYOGRE / RAYQUAZA, which Kai hides as
  HOENN's (MP_LegendForeign), and REGIONS_TODO already moved the ORBs to CINNABAR's chamber; the tower's own object
  (obj 4) only says "...".
- Restore: hide both for good (set 0x1314 / 0x1315 in JOHTO's init, or remove the objects); the EMBEDDED TOWER (HGSS)
  can stay as an empty place or be closed.
- Notes: confidence high on the flags (grep finds no setter); medium that they really show (worth one in-game look).

### J2-7: The HAPPINESS RATER is replaced by Quetzal's rules text, which is false in Kai
- Maps: Johto_GoldenrodHappinessRater
- Category: B
- Original: GoldenrodHappinessRater.asm: the teacher rates the lead Pokémon's friendship (GetFirstPokemonHappiness,
  six answers from "It looks really happy!" to "It looks mean").
- Now: scripts.inc:30 "RETURN and FRUSTRATION always have their maximum power: 102... MOéMON no longer evolve through
  friendship... RIOLU evolves into LUCARIO at level 28". Kai keeps friendship evolutions (src/data/pokemon/evolution.h:110
  PICHU EVO_FRIENDSHIP...) and Emerald's friendship-based RETURN.
- Restore: Crystal's rater and its six lines on Emerald's friendship score (Kai already has VERDANTURF's rater special).
- Notes: confidence high.

### J2-8: The NAME RATER doesn't rate or rename; his shelves talk about RADIANT Pokémon
- Maps: Johto_GoldenrodNameRater (also GoldenrodCity's lass line, GoldenrodDeptStore5F's man)
- Category: B / D
- Original: GoldenrodNameRater.asm: rates and changes nicknames (special NameRater).
- Now: scripts.inc:37 "You can change any nickname from the party menu." (Kai renames from the summary page, SELECT);
  shelf signs :25 "Have you heard of RADIANT MOéMON? ...palette filters" (no such thing in Kai) and a sign naming the
  rival.
- Restore: Crystal's Name Rater on Emerald's own Name Rater specials (SLATEPORT's script), Crystal's shelf / radio
  signs. GoldenrodDeptStore5F's "You can't rename a MOéMON you get in a trade" (Crystal's line) is then right again.
- Notes: confidence high.

### J2-9: The BIKE SHOP explains MACH / ACRO modes on L; JOHTO's BICYCLE has one speed in Kai
- Maps: Johto_GoldenrodBikeShop
- Category: A / B (D for the phone number)
- Original: GoldenrodBikeShop.asm: the clerk lends the BICYCLE ("My BICYCLES are first-rate! You can ride them
  anywhere."), then asks for the player's number (BIKE SHOP phone contact); signs "It's a shiny new BICYCLE!" and
  "JUST RELEASED! FIRST-RATE COMPACT BICYCLES!".
- Now: scripts.inc:58 and :78 "Press L to switch modes: MACH for speed, ACRO for hops and tricks"; src/data/items.h
  ITEM_BICYCLE: "rides as FireRed's, one speed" (secondaryId MACH_BIKE). An extra Quetzal clerk (obj 2) repeats it.
- Restore: Crystal's clerk lines and signs, drop obj 2. The BIKE SHOP phone call is left out of Kai's phone on
  purpose (COOP_NOTES "JOHTO's phone, Crystal's"), so only mention it.
- Notes: confidence high.

### J2-10: MAGNET TRAIN station people say the train has broken down for good, even after Kai's train runs
- Maps: Johto_GoldenrodMagnetTrainStation
- Category: A / E (B for the PASS)
- Original: GoldenrodMagnetTrainStation.asm: the officer asks for the PASS ("We'll soon depart for SAFFRON"), the
  gentleman "It really brings JOHTO much closer to KANTO", "The train hasn't come in..." before the power plant.
- Now: scripts.inc:44-62. The clerk runs Kai's EventScript_MagnetTrain (League + S.S. TICKET) and only says "broken
  down" when it's closed (fine), but the PRESIDENT ("service has been suspended indefinitely"), the boy and the seer
  always say it is broken down.
- Restore: keep Kai's rule (B: Kai's region travel), give the three people Crystal's lines (or a "broken down" /
  "running" pair on MP_MagnetTrainOpen).
- Notes: confidence high.

### J2-11: Ferry signs and clerk say "both regions' Leagues"; Kai needs only the current region's League
- Maps: Johto_OlivineCityPier, Johto_OlivinePort; shared.inc Quetzal_Text_840FB2D (:561), Quetzal_Text_83E90EF (:527,
  VICTORY ROAD gate, J3)
- Category: A / E (B: Crystal's S.S. AQUA)
- Original: OlivinePort.asm / OlivinePortPassage.asm: the S.S. AQUA to VERMILION on Mondays and Fridays with the S.S.
  TICKET, after the HALL OF FAME.
- Now: OlivineCityPier/scripts.inc:23 "From OLIVINE you can visit VERMILION and LILYCOVE any day. You need an S.S.
  Ticket and must have won both regions' Leagues"; the port clerk's refusal (shared.inc:561) "must have won both
  Leagues". Kai's QzGetReachableRegionsMask (qz_specials.c) wants only the current region's League + the S.S. TICKET,
  and lists every region (SINNOH too).
- Restore: keep Kai's ferries (Kai's region travel), reword the two texts to Kai's rule; Crystal's port people
  ("We're departing soon", Monday / Friday) don't fit Kai's ferry and can stay out.
- Notes: confidence high.

### J2-12: Former leaders say "come to the FIGHTING DOJO" for rematches; there are none there
- Maps: Johto_GoldenrodDeptStore6F (WHITNEY), Johto_EcruteakCityNorth (MORTY), Johto_OlivineCafe (JASMINE)
- Category: A / C
- Original: none (Crystal: no leader rematches; HGSS rematches by phone in their gyms).
- Now: 6F scripts.inc:104, EcruteakCityNorth:29, OlivineCafe:56. They appear after JOHTO's League (qz_specials.c
  ~1876 shows FLAG_QZ_0x1017/1019/101C/101E). Kanto_SaffronCity_Dojo has only FireRed's karate trainers, no JOHTO
  leaders.
- Restore: remove the three postgame leaders (or keep them with a line that promises nothing).
- Notes: confidence high.

### J2-13: The legendary story follows Quetzal's HGSS-style KIMONO GIRL chain, not Crystal's
- Maps: Johto_EcruteakCity, Johto_DanceTheater, Johto_EcruteakTinTowerEntrance, Johto_TinTowerRoof,
  Johto_WhirlIslandLugiaChamber, Johto_CianwoodOldWomansHouse, Johto_CianwoodLugiaSpeechHouse, Johto_RadioTower5F
- Category: C
- Original: Crystal: the five KIMONO GIRLS are ordinary DANCE THEATER trainers and the old man gives HM03 SURF for
  beating them (DanceTheater.asm); the RADIO director gives the CLEAR BELL; the WISE TRIO test the player; SUICUNE
  at TIN TOWER 1F (Lv 40); HO-OH needs the RAINBOW WING from TIN TOWER 1F's sage after the ELITE FOUR
  (TinTower1F.asm:190-204), Lv 60 (TinTowerRoof.asm:34); LUGIA needs the SILVER WING from PEWTER's old man
  (WhirlIslandLugiaChamber.asm:13, :34, Lv 60).
- Now: a ROCKET grunt harasses a KIMONO GIRL (DanceTheater scripts.inc:59), the old man gives SURF for that; MORTY's gym
  waits for it (EcruteakCity:298); the rival loses to the KIMONO GIRLS (EcruteakCity:20); after ELM's call the five
  girls battle in a row with Lv 48 Eeveelutions and give the CLEAR BELL (+ RAINBOW WING) (DanceTheater:385); TIN TOWER's
  sage wants the CLEAR BELL; the girls dance for HO-OH (Lv 50, johto_shared.inc:1749) before the League, then hand
  the TIDAL BELL for LUGIA (Lv 50, WhirlIslandLugiaChamber:165), also pre-League. CianwoodLugiaSpeechHouse:40 still
  says the SILVER WING is needed (no longer true); CianwoodOldWomansHouse (HGSS-flavored bells talk) sits where
  Crystal has the POKé SEER.
- Restore: the user's call. REGIONS_TODO kept the director's RAINBOW WING "because the TIN TOWER's events wait on the
  WING", and HGSS ranks with Crystal, so the chain itself may stay; the pure-Quetzal parts (ROCKET at the theater and
  the MORTY gate on it, the rival vs KIMONO GIRLS scene, Lv 48 Eeveelution teams, both legendaries pre-League) would
  go for vanilla: KIMONO GIRLS as Crystal's trainers with HM03 for all five, HO-OH / LUGIA after the League at Lv 60
  with the WINGS.
- Notes: large change touching VAR_QZ_0x5004/5006/5007/501A chains; confidence high on the differences.

### J2-14: The WISE TRIO, SUICUNE at TIN TOWER 1F and the sage's RAINBOW WING are missing
- Maps: Johto_WiseTriosRoom (empty), Johto_TinTower1F, Johto_EcruteakTinTowerEntrance
- Category: D
- Original: WiseTriosRoom.asm (six sages, the trio battles SAGE GAKU / MASA / KOJI once the CLEAR BELL is shown),
  TinTower1F.asm:15, :116-121 (SUICUNE battle Lv 40, EUSINE's lines after), :190-204 (RAINBOW WING after the
  ELITE FOUR), EcruteakTinTowerEntrance.asm (the sage blocks without the FOGBADGE).
- Now: WiseTriosRoom has no people; TIN TOWER 1F has three talking sages; the entrance sage wants the CLEAR BELL.
  SUICUNE is a static overworld encounter elsewhere (J3's LAKE OF RAGE).
- Restore: depends on J2-13; at least the WISE TRIO and their battles, SUICUNE here instead of the static.
- Notes: confidence high.

### J2-15: ECRUTEAK's ITEMFINDER gift is missing
- Maps: Johto_EcruteakItemfinderHouse
- Category: D
- Original: EcruteakItemfinderHouse.asm: "Ah. You're on an adventure... what's an adventure without treasure
  hunting?" gives the ITEMFINDER and explains it.
- Now: scripts.inc:5 the man only says "ECRUTEAK is a very old city..."; the books are Crystal's.
- Restore: Crystal's gift with Kai's ITEM_ITEMFINDER (items.h:313), a once-flag, Crystal's words.
- Notes: docs/quetzal/JOHTO.md lists it as dropped; not in REGIONS_TODO. Confidence high.

### J2-16: BUENA's PASSWORD show can't be played (no BLUE CARD, no points, no prize desk)
- Maps: Johto_RadioTower2F
- Category: D (A: the radio announces a password nobody takes)
- Original: RadioTower2F.asm: BUENA gives the BLUE CARD, asks for the day's password after 6 PM, one point a day,
  the receptionist trades points for prizes, BUENA's phone number at 30 points.
- Now: scripts.inc:170 BUENA only says "Tune in to my PASSWORD SHOW!" (and "HELP" during the takeover). Kai's POKéGEAR
  station 10.5 (src/mp/mp_pokegear.c:713-718) broadcasts "Today's password! ...HELP! ...I'm in GOLDENROD's RADIO
  TOWER!" every night.
- Restore: the BLUE CARD, the password check (Crystal's password list, one a day), the prize desk with Crystal's
  prizes; the radio would read the day's password.
- Notes: COOP_NOTES says BUENA isn't on Kai's phone; this is the show itself. Confidence high.

### J2-17: GAME CORNER's TM prize counter is missing
- Maps: Johto_GoldenrodGameCorner
- Category: D (C for the line changes)
- Original: GoldenrodGameCorner.asm:1-3, :59-157: TM25 THUNDER, TM14 BLIZZARD, TM38 FIRE BLAST for 5500 coins each;
  card flip machines; the BLIZZARD gentleman.
- Now: obj 2 (16,2) only says "All MOéMON prizes are at the counter to the right" (scripts.inc:432). Kai has all three
  TMs (Gen 3 numbers match). The gentleman says ICE BEAM (:480; Crystal: BLIZZARD). CARD FLIP is Emerald's roulette
  (keep: Kai's engine).
- Restore: Crystal's TM vendor on obj 2 with Crystal's prices and menu (fix_prize_room.py style).
- Notes: REGIONS_TODO only covered the Pokémon prizes. Confidence high.

### J2-18: DEPT. STORE 5F's Sunday RETURN / FRUSTRATION lady is missing, though a shopper still talks about her
- Maps: Johto_GoldenrodDeptStore5F
- Category: D / A
- Original: GoldenrodDeptStore5F.asm (CheckIfSundayCallback, ReceptionistScript): on Sundays she checks the lead's
  friendship and gives TM27 RETURN or TM21 FRUSTRATION once a week.
- Now: scripts.inc:80 "During the day, a lady comes to check out MOéMON. She even gives away TMs!" and no lady.
- Restore: the receptionist with Crystal's lines (Kai has TM27 / TM21 and friendship).
- Notes: Crystal's 5F TM shop (punch TMs, HEADBUTT, ROCK SMASH) is already excluded by Kai's shop rules
  (COOP_NOTES "Shops (1.61.5)"). Confidence high.

### J2-19: In-game trades: thank-you lines name Quetzal's PANCHAM / HELIOPTILE; nicknames and OTs aren't Crystal's
- Maps: Johto_GoldenrodDeptStore6F, Johto_OlivineTimsHouse
- Category: E (C)
- Original: Crystal NPC_TRADE_MIKE (GoldenrodDeptStore5F.asm:127, MACHOP "MUSCLE" for ABRA, on 5F) and NPC_TRADE_TIM
  (OlivineTimsHouse.asm:12, VOLTORB "VOLTY" for KRABBY).
- Now: the Gen 4 cap turned Quetzal's trades into MACHOP / VOLTORB (src/data/qz_ingame_trades.h [5], [6]: nickname
  "MACHOP" / "VOLTORB", OT "QUETZAL"), but the after-trade lines say "Hi, how's my PANCHAM doing?"
  (6F scripts.inc:124) and "how's my HELIOPTILE doing?" (TimsHouse:71). MIKE stands on 6F.
- Restore: Crystal's nicknames and OT names (MUSCLE / MIKE, VOLTY / TIM) and Crystal's after-trade lines with the
  species (or STR_VAR); MIKE on 5F if the store floors are redone.
- Notes: confidence high.

### J2-20: GOLDENROD DEPT. STORE floors and directory are Quetzal's
- Maps: Johto_GoldenrodDeptStore1F-6F, B1F, Roof
- Category: C
- Original: 1F service counter, 2F TRAINER'S MARKET, 3F BATTLE COLLECTION, 4F MEDICINE BOX, 5F TM CORNER, 6F TRANQUIL
  SQUARE (vending), roof with binoculars, POKé DOLL machine and the bargain sale; Crystal's shoppers (2F "gear can
  store ten numbers", 3F X SPEED / X SPECIAL nerds, 4F IRON bug catcher, 6F lass and nerd).
- Now: directory (1F scripts.inc:76) "2F ITEMS & BATTLE TOOLS, 3F TRAINING & MINTS, 4F EVOLUTION & FORMS, 5F MEGA, Z &
  TERA"; floor signs to match; 4F's man denies trade evolutions ("they all evolve through level up"; Kai has both
  trade and Lv 40); B1F's MACHOKE boulder gag and a RARE CANDY ball (Crystal: BURN HEAL); roof missing binoculars and
  the doll machine. The clerks already sell Crystal's lists plus Kai's extras (shop.c sMpBaseLists).
- Restore: Crystal's directory and floor signs, its shoppers' lines; keep Kai's shop contents (Kai's choice).
- Notes: low priority. Confidence high.

### J2-21: The WEEKDAY siblings are there every day with a guess-the-day game
- Maps: Johto_Route36 (ARTHUR), Johto_Route37 (SUNNY), Johto_Route40 (MONICA); same on Route29 / 32 / LAKE OF RAGE /
  BLACKTHORN (J1 / J3)
- Category: C
- Original: Route36.asm ArthurScript etc.: each appears only on his or her weekday ("Today's not Thursday. How
  disappointing.") and gives the item once with Crystal's lines (HARD STONE, MAGNET, SHARP BEAK).
- Now: always present; "If you can guess my favorite day, I'll give you a reward" + a seven-day menu
  (Route36:54, Route37:44, Route40:63), "Not quite!" on a wrong guess.
- Restore: show each only on their day (RTC day of week) and use Crystal's greeting / not-today lines; drop the menu.
- Notes: DayOfWeekSiblingsHouse's book already lists the days (Crystal's). Confidence high.

### J2-22: Bug-Catching Contest uses Quetzal's rules, and its rules text promises RADIANT scoring
- Maps: Johto_Route35NationalParkGate, Johto_NationalParkBugContest, Johto_NationalPark
- Category: C (B for "radiant"; text)
- Original: Route35NationalParkGate.asm / Route36NationalParkGate.asm: Tuesday, Thursday, Saturday; 20 PARK BALLS, 20
  minutes; keep one catch (swap); other contestants (DON, ED, NICK, BENNY, CINDY, JOSH...) visible in the park and in
  the gate afterwards; SUN STONE / EVERSTONE / SITRUS (GOLD BERRY) / consolation BERRY.
- Now: src/qz_bugcontest.c (Kai's code on Quetzal's rules): any day, 8 minutes, 30 SAFARI BALLS, five catches all kept,
  scored by level / IVs / catch rate / shiny (qz_bugcontest.c:97-101). Gate text (scripts.inc:229) says "Level, IVs,
  rarity, catch rate, shiny and radiant all affect score" (no radiant in Kai), "EXP will be reverted", "Make five
  spaces in your PC boxes".
- Restore: user's call (Kai already invested in it). For vanilla: Crystal's days, ball count, time, keep-one rule,
  contestants and words. Minimum: drop "radiant" from the text.
- Notes: confidence high.

### J2-23: GOLDENROD GYM guide talks about FAIRY types; gym statues read "TOWN" / "CERTIFIED TRAINERS"
- Maps: Johto_GoldenrodGym (statue wording also Ecruteak / Olivine / Cianwood gyms)
- Category: C / E
- Original: GoldenrodGym.asm GuideText "This GYM is home to normal-type #MON trainers. I recommend you use
  fighting-type #MON."; statues via GymStatue1Script / GymStatue2Script ("<CITY> #MON GYM / LEADER: <NAME>",
  "WINNING TRAINERS: <PLAYER>").
- Now: scripts.inc:260 "Some of the MOéMON in this gym have the newly discovered FAIRY type... Check ROUTE 34 if you
  need a FIGHTING-type or POISON-type" (no FAIRY below GENERATION 6); :154 "GOLDENROD TOWN MOéMON GYM / WHITNEY'S
  CERTIFIED TRAINERS" (FireRed's wording, and "TOWN" is wrong); the other gyms say "<X>'S CERTIFIED TRAINERS" too.
- Restore: Crystal's guide line and statue texts.
- Notes: low. Confidence high.

### J2-24: Crystal's GOLDENROD move tutor, city sign and RADIO CARD lady are missing
- Maps: Johto_GoldenrodCity
- Category: D
- Original: GoldenrodCity.asm:39-170: after the ELITE FOUR, on Wednesdays and Saturdays, a tutor teaches
  FLAMETHROWER / THUNDERBOLT / ICE BEAM for 4000 coins then goes into the GAME CORNER; GoldenrodCitySign "GOLDENROD
  CITY / The Festive City of Opulent Charm"; the cooltrainer about the RADIO CARD.
- Now: none of these; the sign spot at 50,22 says "It's empty." (johto_shared.inc Quetzal_EventScript_836086B). New
  Quetzal lines: the gym-door man (J2-5), the GAME CORNER day / night joke, "Whew! This town is so big".
- Restore: the tutor on Kai's tutor flow (as KANTO's, fix_move_tutors.py), the city sign; the RADIO CARD line goes
  with J2-5.
- Notes: confidence high.

### J2-25: GOLDENROD UNDERGROUND: haircut brothers and the bargain merchant missing; Quetzal shops and a KIMONO GIRL scene added
- Maps: Johto_GoldenrodUnderground, Johto_GoldenrodUndergroundEntrances
- Category: D / C
- Original: GoldenrodUnderground.asm: the older / younger HAIRCUT BROTHERS (friendship, on set weekdays), the
  bargain merchant (Monday mornings: NUGGET, PEARL...), the herb granny (weekends); coin case ball.
- Now: herb shop every day (obj 2), Quetzal's glass FLUTE shop (:261), a METEORITE shop, an empty clerk (obj 3; Kai
  sells it nothing below GEN 4); KUNI the KIMONO GIRL scene at VAR_QZ_0x500A 7 (:28); a trainer still says "Your
  MOéMON will like you more if you give them haircuts".
- Restore: the two brothers and the merchant with Crystal's days and lines; drop the flute / Meteorite / empty clerks
  and KUNI's scene (HGSS-ish, tied to J2-13).
- Notes: confidence high.

### J2-26: ROUTE 34 / its gate: KATE's SOFT SAND is a ball; Trainer Tips say berries regrow "several times a day"; SWEET SCENT teacher gone
- Maps: Johto_Route34, Johto_Route34IlexForestGate
- Category: C / B / D
- Original: Route34.asm:459-465 KATE gives SOFT SAND after her battle; :742 "BERRY trees grow new BERRIES every day";
  Route34IlexForestGate.asm the teacher gives TM12 SWEET SCENT and warns about ILEX during the CELEBI event.
- Now: SOFT SAND item ball at 29,84 (scripts.inc:98), KATE's lines rewritten; tips sign :193 "several times a day"
  (Kai's JOHTO trees are once a day, REGIONS_TODO); the gate woman only has the shrine line.
- Restore: KATE's gift and lines, Crystal's tip text. SWEET SCENT isn't a TM in Kai, so the teacher can only come
  back with words (or Kai's tutor flow).
- Notes: low. Confidence high.

### J2-27: ROUTE 36: FLORIA isn't at the odd tree; SQUIRTBOTTLE needs only the badge; a Hisuian GROWLITHE warning
- Maps: Johto_Route36, Johto_GoldenrodFlowerShop, Johto_Route36RuinsOfAlphGate
- Category: C / D
- Original: Route36.asm FloriaText1 (she waters the tree, goes for her sister's bottle; GoldenrodCity's callback puts
  her there), the teacher gives the SQUIRTBOTTLE once FLORIA met it and the PLAINBADGE is won; the lass "An odd tree is
  blocking the way to GOLDENROD... preventing me from shopping"; the RUINS gate gramps about the strange tree.
- Now: no FLORIA on the route; FlowerShop scripts.inc:31 gives the bottle on the badge alone, with rewritten lines;
  Route36:263 "Be careful at night... looked like GROWLITHE, but it had a scary aura" (a Gen 8 form) in the lass's place.
- Restore: FLORIA by the tree with Crystal's lines and her flag, Crystal's teacher condition and words, the lass's and
  gramps' lines.
- Notes: low. Confidence high.

### J2-28: NATIONAL PARK / ROUTE 35 people rewritten or added
- Maps: Johto_NationalPark, Johto_Route35, Johto_Route35GoldenrodGate, Johto_Route35NationalParkGate
- Category: C
- Original: NationalPark.asm (QUICK CLAW teacher, BEVERLY's MARILL friend, the mail mother, the DEX-printing kid),
  Route35.asm (OFFICER DIRK at night, ARNIE's YANMA), Route35GoldenrodGate.asm ("there are over 250 kinds").
- Now: a healing lady (obj 9), a couple ("With her by my side..."), sticker kids; ROUTE 35's officer lines rewritten;
  the gate fan says "over 400!" (Route35GoldenrodGate:130). Item balls MOON STONE / SOOTHE BELL in the park
  (Crystal: PARLYZ HEAL, TM28 DIG).
- Restore: Crystal's lines; remove the healer. Item balls: REGIONS_TODO only covers TM balls; these are low.
- Notes: low. Confidence high.

### J2-29: MANIA's SHUCKLE is a gift to keep, not a loan
- Maps: Johto_ManiasHouse
- Category: C
- Original: ManiasHouse.asm: he lends SHUCKLE, asks for it back later, and lets you keep it if it's happy.
- Now: scripts.inc:55 "I want my MOéMON to travel with you. You can keep it for good!"
- Restore: Crystal's loan and return lines (happiness check).
- Notes: low. Confidence high.

### J2-30: CIANWOOD's PHOTO STUDIO and POKé SEER are missing; an HGSS-style bells lady in their place
- Maps: Johto_CianwoodCity, Johto_CianwoodOldWomansHouse
- Category: D / C
- Original: CianwoodPhotoStudio.asm (photo souvenir, Game Boy printer), PokeSeersHouse.asm (tells where and when a
  Pokémon was met); city signs for both.
- Now: no such houses or signs; CianwoodOldWomansHouse talks about the CLEAR and TIDAL BELLS; the city has CLIFF EDGE
  CAVE / SAFARI ZONE signs and a "path under construction" man (:361).
- Restore: the SEER (Kai stores met location / level) with Crystal's lines; the photo studio is printer-only (low).
- Notes: low-medium. Confidence high.

### J2-31: MARY gives BRIGHT POWDER (Crystal: PINK BOW)
- Maps: Johto_RadioTower4F
- Category: C
- Original: RadioTower4F.asm DJMaryText_GivePinkBow: PINK BOW (boosts NORMAL moves).
- Now: scripts.inc:76 ITEM_BRIGHT_POWDER, "It's good for dodging the enemy's attacks."
- Restore: SILK SCARF (Gen 3's PINK BOW) with Crystal's line "It's just right for #MON that know normal-type moves."
- Notes: low. Confidence high.

### J2-32: MOOMOO needs five ORAN BERRIES (Crystal: seven), the farm gives LEFTOVERS (Crystal: TM13 SNORE)
- Maps: Johto_Route39Barn, Johto_Route39Farmhouse
- Category: C
- Original: Route39Barn.asm:67-75 (BERRY seven times, healed at 7), Route39Farmhouse.asm:71-85 (TM13 SNORE).
- Now: Barn :27 counts five ORAN BERRIES (VAR_QZ_0x501B 0-4); Farmhouse :17 LEFTOVERS.
- Restore: seven feedings; SNORE isn't a TM in Kai, so LEFTOVERS can stay (or SNORE by tutor).
- Notes: low. Confidence high.

### J2-33: TRAINER HILL / BATTLE TENT signs north of ROUTE 40 lead to an empty room; Crystal's BATTLE TOWER area is gone
- Maps: Johto_TrainerHillOutside, Johto_TrainerHillEntrance, Johto_Route40TrainerHillGate, Johto_Route40,
  Johto_OlivineCity
- Category: A / C / D
- Original: BattleTowerOutside.asm, BattleTower1F.asm, Route40BattleTowerGate.asm (the BATTLE TOWER, its people,
  OLIVINE's "BATTLE TOWER AHEAD" sign).
- Now: TrainerHillOutside:42 signs "TRAINER HILL ENTRANCE / BATTLE TENTS" and three BATTLE TENT signs; the entrance is
  an empty room, the gate empty; OLIVINE's sign missing.
- Restore: either reword the signs to what's there (nothing), or make the entrance a door to Kai's own facility. A
  real BATTLE TOWER would be new work.
- Notes: confidence high.

### J2-34: Overworld legendaries and the JESSIE & JAMES trio in J2's maps
- Maps: Johto_GoldenrodCity (TAPU KOKO; trio obj 27-30), Johto_GoldenrodBikeShop (MELTAN),
  Johto_GoldenrodMagnetTrainStation (MIRAIDON), Johto_GoldenrodUndergroundSwitchRoomEntrances (MAGEARNA),
  Johto_GoldenrodUndergroundWarehouse (GUZZLORD), Johto_EcruteakCity (TAPU LELE, ENTEI, GOUGING FIRE; trio 30-33),
  Johto_EcruteakCityNorth (WO-CHIEN), Johto_BurnedTower1F (MOLTRES, hidden), Johto_BurnedTowerB1F (CHI-YU, YVELTAL),
  Johto_DanceTheater (MELOETTA), Johto_CianwoodCity (TAPU FINI; trio 15-18), Johto_OlivineCity (RAIKOU, RAGING BOLT),
  Johto_OlivineLighthouse (ZAPDOS hidden, VICTINI), Johto_Route35 (MUNKIDORI, BLACEPHALON), Johto_Route41
  (THUNDURUS), Johto_NationalPark (SHAYMIN hidden, ENAMORUS), Johto_TinTower8F (CELESTEELA), Johto_WhirlIslandTunnelEast
  (MANAPHY, NIHILEGO), Johto_EmbeddedTower (KYOGRE / GROUDON / RAYQUAZA hidden); johto_shared.inc
  Quetzal_EventScript_840EA6A (:2242), Quetzal_EventScript_83C2D48 (:787)
- Category: C
- Original: none of these objects; JESSIE & JAMES aren't in Crystal.
- Now: the Gens 5-9 legendaries wait for the League and GENERATION (decided: they stay); ENTEI / RAIKOU see J2-2.
  The trio (TRAINER_QZ_913, then a Lv 50 PIKACHU) visit one town a day after the League (qz_specials.c TravelingEvent).
- Restore: the legendaries per the existing decision (no change); the trio is a Quetzal fan addition: hide it for good
  (keep FLAG_QZ_0x1490 set) unless the user wants it.
- Notes: reported once for the whole pattern; J1 / J3 have the other towns. Confidence high.

### J2-35: Small Quetzal rewrites and additions (pattern)
- Maps: Johto_GoldenrodPokecenter1F (two "the roof is made of gold" people, :52), Johto_EcruteakPokecenter1F (BILL:
  "my PC is running at all POKéMON CENTERS in JOHTO and KANTO", :107; Crystal: the TIME CAPSULE speech), every JOHTO
  POKéMON CENTER's CHANSEY (Quetzal_EventScript_83C9AC2), Johto_OlivineCity (SAFARI ZONE news :188, sailors on
  JASMINE's ranking), Johto_Route39 (BAOBA's intro and HO-OH's shadow, :240), Johto_Route38 ("NO SWIMMING OR FISHING"
  sign; Crystal's B-button evolution tip missing), Johto_OlivineGym (two cheering NPCs at 11,14 / 5,10 on triggers;
  badge line "SPECIAL DEFENSE" :332, Crystal: DEFENSE), Johto_GoldenrodUndergroundWarehouse (grunt "Sorry GIOVANNI!"),
  Johto_BurnedTower1F (FIREBREATHER / BURGLAR added), Johto_OlivineCafe (the "dishes" sailor), Johto_GoldenrodCity
  ("GAME CORNER... night time / day time" man)
- Category: C (E for SPECIAL DEFENSE)
- Original: the matching Crystal people say Crystal's lines (missing_all in tmp_J2 lists each).
- Restore: Crystal's lines where a Crystal person stands; remove the added people (low priority each).
- Notes: low. Confidence high.

### J2-36: Nuzlocke checks and texts left in gift / contest scripts (dead in Kai)
- Maps: Johto_BillsFamilysHouse, Johto_Route35GoldenrodGate, Johto_ManiasHouse, Johto_DayCare,
  Johto_GoldenrodGameCorner, Johto_Route35NationalParkGate, Johto_NationalParkBugContest; shared.inc
  Quetzal_EventScript_8361A4D (:125), johto_shared.inc Quetzal_Text_83C36A1
- Category: B
- Original: none.
- Now: `call Quetzal_EventScript_8361A4D` / `QzIsNuzlockeActiveInRegion` / `QzIsNuzlockeOption2D4Bit6` before gifts;
  the contest's "exhibition run" texts (NationalParkBugContest:189). All return FALSE (qz_specials.c:62-63), so never
  shown.
- Restore: harmless; remove when the scripts are touched anyway.
- Notes: low. Confidence high.

---

## Coverage
- Johto_BillsFamilysHouse: J2-36 (BILL's sister's phone number missing: Kai's phone leaves BILL out on purpose); otherwise same as the original
- Johto_BurnedTower1F: J2-34, J2-35; otherwise only trivial differences (EUSINE / MORTY / rival lines close to Crystal)
- Johto_BurnedTowerB1F: J2-2, J2-34
- Johto_CianwoodCity: J2-30, J2-34, J2-35; HM02 FLY, EUSINE and SUICUNE scene as Crystal
- Johto_CianwoodGym: J2-1, J2-23
- Johto_CianwoodLugiaSpeechHouse: J2-13 (SILVER WING line now untrue)
- Johto_CianwoodOldWomansHouse: J2-13, J2-30
- Johto_CianwoodPharmacy: only trivial differences
- Johto_CianwoodPokecenter1F: J2-35; otherwise only trivial differences
- Johto_DanceTheater: J2-13, J2-34
- Johto_DayCare: J2-3, J2-36
- Johto_DayOfWeekSiblingsHouse: same as the original (see J2-21 for the siblings)
- Johto_EcruteakCity: J2-2, J2-13, J2-34
- Johto_EcruteakCityNorth: J2-12, J2-34
- Johto_EcruteakGym: J2-1, J2-23
- Johto_EcruteakItemfinderHouse: J2-15
- Johto_EcruteakLugiaSpeechHouse: only trivial differences
- Johto_EcruteakMart: only trivial differences
- Johto_EcruteakPokecenter1F: J2-35
- Johto_EcruteakTinTowerEntrance: J2-13, J2-14
- Johto_EmbeddedTower: J2-6, J2-34
- Johto_GoldenrodBikeShop: J2-9, J2-34
- Johto_GoldenrodCity: J2-5, J2-24, J2-34, J2-35
- Johto_GoldenrodCity_Copy: unreferenced empty duplicate (no events, no warps in): nothing to compare
- Johto_GoldenrodDeptStore1F: J2-20
- Johto_GoldenrodDeptStore2F: J2-20
- Johto_GoldenrodDeptStore3F: J2-20
- Johto_GoldenrodDeptStore4F: J2-20
- Johto_GoldenrodDeptStore5F: J2-18, J2-20
- Johto_GoldenrodDeptStore6F: J2-12, J2-19, J2-20
- Johto_GoldenrodDeptStoreB1F: J2-20
- Johto_GoldenrodDeptStoreElevator: Kai's own elevator (COOP_NOTES); only trivial differences
- Johto_GoldenrodDeptStoreRoof: J2-20
- Johto_GoldenrodDeptStoreRoofAlt: alternate layout of the roof (FLAG_QZ_0x1016), no scripts: J2-20
- Johto_GoldenrodFlowerShop: J2-4, J2-27
- Johto_GoldenrodGameCorner: J2-17, J2-36
- Johto_GoldenrodGym: J2-1, J2-5, J2-23
- Johto_GoldenrodHappinessRater: J2-7
- Johto_GoldenrodMagnetTrainStation: J2-10, J2-34
- Johto_GoldenrodNameRater: J2-8
- Johto_GoldenrodPPSpeechHouse: same as the original
- Johto_GoldenrodPokecenter1F: J2-35 (Crystal's other people are mobile / link ones)
- Johto_GoldenrodPokecenter1F_Copy: unreferenced empty duplicate: nothing to compare
- Johto_GoldenrodUnderground: J2-25
- Johto_GoldenrodUndergroundEntrances: only trivial differences
- Johto_GoldenrodUndergroundSwitchRoomEntrances: J2-34; rival battle and switches as Crystal (only trivial differences)
- Johto_GoldenrodUndergroundWarehouse: J2-34, J2-35; DIRECTOR / CARD KEY as Crystal
- Johto_ManiasHouse: J2-29, J2-36
- Johto_NationalPark: J2-28, J2-34
- Johto_NationalParkBugContest: J2-22, J2-36
- Johto_OlivineCafe: J2-12, J2-35
- Johto_OlivineCity: J2-2, J2-33, J2-34, J2-35
- Johto_OlivineCityPier: J2-11
- Johto_OlivineGoodRodHouse: only trivial differences (an added POKéDEX fishing-group tip)
- Johto_OlivineGym: J2-1, J2-23, J2-35
- Johto_OlivineLighthouse: J2-34; SECRET POTION errand and trainers as Crystal
- Johto_OlivineMart: only trivial differences
- Johto_OlivinePokecenter1F: only trivial differences
- Johto_OlivinePort: J2-11
- Johto_OlivinePunishmentSpeechHouse: same as the original
- Johto_OlivineTimsHouse: J2-19
- Johto_RadioTower1F: J2-5
- Johto_RadioTower2F: J2-16
- Johto_RadioTower3F: only trivial differences (TM11 already Crystal's)
- Johto_RadioTower4F: J2-31 (PROTON by name is HGSS's: keep)
- Johto_RadioTower5F: J2-1 (GIOVANNI-story cleartrainerflag); otherwise the decided RAINBOW WING version
- Johto_Route34: J2-3, J2-26
- Johto_Route34IlexForestGate: J2-26
- Johto_Route34IlexForestGate_Copy: unreferenced empty duplicate: nothing to compare
- Johto_Route34_Copy: unreferenced empty duplicate: nothing to compare
- Johto_Route35: J2-28, J2-34
- Johto_Route35GoldenrodGate: J2-28, J2-36
- Johto_Route35NationalParkGate: J2-22, J2-28, J2-36
- Johto_Route36: J2-21, J2-27
- Johto_Route36RuinsOfAlphGate: J2-27
- Johto_Route37: J2-21
- Johto_Route38: J2-35
- Johto_Route38EcruteakGate: same as the original
- Johto_Route39: J2-35
- Johto_Route39Barn: J2-32
- Johto_Route39Farmhouse: J2-32
- Johto_Route40: J2-21, J2-33
- Johto_Route40TrainerHillGate: J2-33
- Johto_Route41: J2-34; otherwise only trivial differences
- Johto_TinTower1F: J2-13, J2-14
- Johto_TinTower2F: only trivial differences (no scripts)
- Johto_TinTower3F: only trivial differences (item balls: decided)
- Johto_TinTower4F: only trivial differences
- Johto_TinTower5F: only trivial differences
- Johto_TinTower6F: only trivial differences
- Johto_TinTower7F: only trivial differences
- Johto_TinTower8F: J2-34
- Johto_TinTower9F: only trivial differences
- Johto_TinTowerRoof: J2-6, J2-13
- Johto_TinTowerRoofAlt: alternate layout of the roof, no scripts: J2-6 / J2-13 apply
- Johto_TrainerHillEntrance: J2-33
- Johto_TrainerHillOutside: J2-33
- Johto_WhirlIslandB1F: only trivial differences
- Johto_WhirlIslandB2F: only trivial differences
- Johto_WhirlIslandCave: only trivial differences (no scripts)
- Johto_WhirlIslandEntrances: only trivial differences
- Johto_WhirlIslandLugiaChamber: J2-6, J2-13
- Johto_WhirlIslandTunnelEast: J2-34
- Johto_WhirlIslandTunnelWest: only trivial differences (no scripts)
- Johto_WiseTriosRoom: J2-14
- Shared (johto_shared.inc / shared.inc) called by these maps: J2-1 (83601F0), J2-3 (83F08A1, 83F0976), J2-11
  (840FB2D, 83E90EF), J2-34 (840EA6A, 83C2D48), J2-36 (8361A4D, 83C36A1); helper outputs in
  /tmp/claude-0/-home-user/a6424581-b487-50fe-8bae-5e1098371be0/scratchpad/audit/tmp_J2/ (new_all.tsv: game lines
  without a Crystal match; missing_all.tsv: Crystal lines without a game match)
