# UNOVA: where it isn't yet 1:1 with Black

The user's rule (2026-11-02 for SINNOH, 2026-11-11 for UNOVA): UNOVA is 1:1 with Pokemon Black. Closed only: online-only
features (GTS, Wi-Fi) and what the DS itself needs (a second cartridge, the DS's wireless), as for Platinum. Everything
else that differs is a gap to correct, not a choice. But **anything that has an Emerald or Kai equivalent is not
reworked** (the user, 2026-11-13): Black's scripts open the game's own (its DAY CARE, screens, rules) and only what
neither has is ported from Black. This list was made 2026-11-13 from an audit of Black's script
commands the VM never plays or plays as nothing (src/unova/u3d_vm.c), the handoff's "not yet" notes and the decisions'
systems. Tick an entry only once it plays as Black's and has been seen running.

## 1. Drawn things missing

- [x] **Gym machinery** (Black's field expansion objects, ov021 fld_exp_obj.c, each gimmick's G3dUtilSetup: models,
      textures and their joint animations; 2026-11-13, seen running in each gym): BURGH's honey doors and plates
      (ov038), ELESA's coasters, carts and rails, ridden on Black's paths (ov039), NACRENE's shelf (ov040), SKYLA's
      cannons and the flight (ov041), STRIATON's switches and curtains (ov042), CLAY's lifts, obstacle and elevator,
      ridden at Black's speeds (ov043 / ov044), BRYCEN's switch pieces (ov045), DRAYDEN's dragons and levers (ov046).
      tools/unova/gimmicks.py (pack v5), u3d.c U3D_Exp*, u3d_vm.c's drivers
- [ ] The gym models' texture animations (BTA / BTP / BMA: the switches' and plates' glows, the coasters' gates and
      lights, DRAYDEN's buttons, the cannons' colours): the models are drawn at their first texture frame
- [ ] SKYLA's flight camera (gCannonsCameraInfos: Black turns the view's pitch and yaw for each flight; here the view
      follows the flying player at the field's own angle)
- [ ] **Building animations**: doors opening and closing (SCREENEFFECTS 0x124-0x132, build-model nodes), machines'
      and lamps' BCA0 / BTP0 / BMA0 (field_buildmodel.c), the POKeMON LEAGUE's statues and stairs (0x12B-0x12E)
- [ ] The GEAR STATION's trains on the platforms
- [ ] 3D cut-ins (RunFld3dCiMain 0x1A1: DESERT RESORT, NACRENE), the ELITE FOUR rooms' entrance flights (FkDemo
      0x1A3 / 0x1A4), camera moves from Black's archive (0x146, ROUTE 10), camera shakes (0x148: ABYSSAL RUINS,
      CHARGESTONE, DRAGONSPIRAL, N's CASTLE)
- [ ] People flown along Black's paths (MmdlIca 0x15B-0x15E: the roamer's fly-by, ABUNDANT SHRINE, ROUTE 7), a person
      rising / circling (0x225 / 0x226, CASTELIA), dropping in (0x247, RELIC CASTLE), event POKeMON's moves
      (0x221-0x228: CASTELIA, MOOR OF ICIRRUS, LOSTLORN FOREST)
- [ ] The LIGHT STONE's arcs and glow (0x23F, 0x255-0x258: N's CASTLE)
- [ ] Face-up portraits (FLD_FACEUP 0x1D3-0x1D5: N's CASTLE, NIMBASA)
- [ ] The gates' electric news boards (GIMMICK_OBJ_ELBOARD 0x17E)
- [ ] The dragons' levels in DRAYDEN's gym (walkways at their own heights, so the poses force Black's route)

## 2. Gyms

- [x] STRIATON's gym switches (BUTTON_STATES 0x187-0x189, Ov037SetValueB 0x186) and NACRENE's gym books (FLAG_ANIME
      0x185, Ov037SetValueA 0x182): the way through is Black's own works and people (walked in the story); the
      commands only move models, so their states are kept (sWork.gym[6], gym[7], sButtonModels) for the models of
      section 1 to show
- [x] CASTELIA's (Ov037ReadEightVars 0x18F) and NIMBASA's (Ov037InitFromVars 0x17F) kept states as Black's: the
      plate doors' switches already live in works 0x4000-0x4007, the lanes in Black's flags 0x74 / 0x75

## 3. Script commands not played (story and towns)

Checked against every script file on 2026-11-14 (a scan of the commands each file uses that the VM has no case for):
the word buffers, people's move codes / flags / searches, lent-POKeMON battles, detailed gifts, friendship, counts and
records, item lookups, the POKeDEX ratings, trainer records and the sounds listed here before are all played now.

- [x] Items (1.199.6): party use (0x0BD: CASTELIA's ZORUA girl's POKe BALL check), a random TM from the BAG (0x238),
      the HIDDEN POWER type reader (0x236, the battle's own formula), the dust cloud's item (0x0BC: none yet)
- [x] Places (1.199.6): check spots moved by a zone's scripts (0x1EA: NACRENE's gym's shelves), the special location
      set here (0x0DB), the elevators' floor menus and rides (0x1CB-0x1CD: CASTELIA's buildings; scenario
      unova_elevator), the player's form model (0x24C: never)
- [x] The POKeDEX diplomas (0x151 modes 0 / 1: the game's own DIPLOMA screen naming UNOVA, or National; scenario
      unova_diploma)
- [x] A POKeMON's picture until a key (0x227: the game's picture box), a person swapped for another where they stood
      (0x23E): CASTELIA's ZORUA event, which needs a distributed CELEBI (closed: online only)
- [ ] The BATTLE SUBWAY's map (0x151 mode 2: Black's picture, a new screen: mock-up first)
- [ ] The player set on a rail (0x07C: zone 66)
- [x] The battle phrases asked for (0x1ED kinds 0-3, zone 66): the game's own EASY CHAT on the save's introduction /
      won / lost phrases ("No. 1" asked, not kept: Kai has none); scenario unova_phrases
- [ ] Black's own words (0x1ED kinds 4 / 5, 0x20C): the PEP QUIZ (zone 117) needs Black's word list; MR. LOCK's
      password (zone 50) is an event's (closed)
- [x] Counting POKeMON from other trainers (0x239: the party and every box, EGGS left out, 50 at most), NPC
      greetings by the clock and the party (0x23D with Black's record / message tables), the birthday lines (0x20D:
      Black's other days', no DS birthday here), the barred POKeMON named (0x1B9), the GeoNet country (0x246: none);
      scenario unova_greetings
- [ ] The time effect and trainer-card records of zone 389 (0x24E-0x250), the subprocess of zones 32-34 / 254 (0x134)
- Kai's own: SURF / WATERFALL's hops and jumps (0x0C5 / 0x0C6, Black's field-move scripts: Kai's field moves); closed
  as the DS's or online: birthdays (0x1D6 / 0x1D7 / 0x20D), the research surveys (0x210-0x21F), Wi-Fi lists, GTS,
  the TRIAL HOUSE downloads, PAL PARK (0x1C0), the fixed-zone link-room warp (0x0C3)

## 4. Systems

- [x] ROUTE 3's DAY CARE (0x0EB-0x0F8, 0x251) and EGG hatching (0x106): Black's scripts on the game's own DAY CARE
      and hatching (Kai's Gen 9 breeding), not reworked (2026-11-13: deposit, withdraw, the EGG man; an EGG hatched
      on a UNOVA route by the game's own step counter and scene)
- [x] The MOVE RELEARNER (0x1E8 / 0x1E9), move deleter / setter (0x109, 0x10B), the move tutors (0x1E4-0x1E7): the
      game's own screens (2026-11-13: MISTRALTON's deleter and relearner, a HEART SCALE taken; DRIFTVEIL's PLEDGE
      tutor taught FIRE PLEDGE through the summary's forget list; Black's tutor bits in src/unova/u3d_tables.h)
- [x] POKeMON MUSICALS (1.199.8, the user's approved mock-ups: src/unova/u3d_musical.c): Black's 100 props, four shows,
      their NPC performers and the fans' tables from the ROM (tools/unova/build_musical.py -> unova/black_musical.pak);
      Black's MUSICAL_* commands; the show as Black's event (the theater's scripts 3 / 4 / 5 and 0x28CC between the
      DRESS UP, STAGE and RESULTS screens), Black's scoring and fans' props; friends' latest dress-ups on stage
      (relay MPR_MUSICAL); the practice dressing room. Scenario unova_musical
- [x] The ROYAL UNOVA (PLEASURE_BOAT 0x1A5-0x1AC, SYMBOL_MAP 0x1A9 / 0x1AB: 1.199.6): the day's passengers dealt to
      the cabins as ov036's DAILY_LAYOUT_Generate deals them (the weekday's trainers, two bonus people, the rest), the
      trip's clock (30 ticks a second, a chime each 1350, home at 5400 after four), a talk's time aboard, the cabins'
      battles; the deck's view of the sea (ov203's 3D scene) isn't drawn: it leaves the boat sailing on (u3d_vm.c
      BoatCreate / BoatFrame; scenario unova_boat). Every warp, inside a zone too, runs its zone-change script, as Black
- [x] The ENTRALINK (1.199.7, the user's approved mock-ups: src/unova/u3d_entralink.c): Black's ENTREE (bank 69),
      missions with co-op friends (SUPPORT: talk to the friend, who gets its PASS POWER; BATTLE: a C-GEAR IR battle;
      relay MPR_ENTRALINK 15), Black Level and Pass Orbs, RECEIVE PASS POWER, CHECK STATUS; PASS POWERS from
      Black's table (a/0/3/8) with gpower.c's rules, SHARE for the room. Scenarios unova_entralink,
      unova_entralink_mission
- [ ] The ENTRALINK's own forest maps' extras: WHITE FOREST / BLACK CITY's people (0x1C5 / 0x1C6)
- [x] The C-GEAR as UNOVA's device (1.199.7, the approved mock-up: src/unova/u3d_screen.c; START > C-GEAR): IR with
      the friend in front (TRADE, BATTLE), WIRELESS (the CO-OP menu), ENTRALINK; scenarios unova_cgear, unova_cgear_ir
- [ ] The XTRANSCEIVER's calls (friends' video calls)
- [x] Dark grass: two POKeMON at once (Black's double wild battles: 40 in 100 with two able to battle,
      wild_encounter.c U3dStartWild; test: mp u3dwild 3 b, MOEMON_U3D_DARK_DOUBLE=100)
- [x] Shaking grass, dust clouds, rippling water, birds' shadows (Black's ov021 effect_encount.c: after the first
      badge, the zone's steps and odds, a tile round the player of the pack's encounter-effect kinds; battles from
      Black's special tables, a dust cloud's gem / stone and a shadow's wing by Black's script 10133) and UNOVA's
      fishing (Black's table and odds, a sure bite at the ripples): src/unova/u3d_phenom.c; scenario unova_phenomena
- [ ] POKeMON forms outside battle (ROTOM's appliances 0x117 / 0x11C, DEERLING / SAWSBUCK's seasons, form numbers
      0x0FF): Kai keeps forms its own way (battle recomputes); not reworked, unless the user says otherwise
- [x] TRIPLE battles as Kai's Triple Battles (the player commanding three, with three able; u3d_battle.c, the center
      foe sent out by battle_main.c); ROTATION trainers as Doubles: no Rotation Battles in Kai (the user, 2026-11-14).
      Scenario unova_triple
- [x] Black's catch rules (the user, 2026-11-13: "Black's catch rules and battle backgrounds, adapted to Kai
      standards"): ov093's CalcCaptureShakes, critical captures, the dex factor and Black's balls' rates
      (u3d_battle.c U3D_CatchCase, from battle_script_commands.c); Kai's own balls keep their rates (the ASTRAL BALL its sure catch)
- [x] Black's battle backgrounds: each scene (archive 0x98's tables, the models of a/0/1/1) drawn through Black's
      battle camera when the packs are built, fitted to Kai's screen and Kai's format (BG3, 3 palettes, the intro's
      second half Black's field going on past the scene, so the intro's slide is seamless; palettes by bands of rows,
      no 8 x 8 blocks): visual only, Kai's battles unchanged (singles, Doubles, Triples, co-op; PvP keeps
      Kai's building): tools/unova/battle_bg.py, u3d_battlebg.c; the time's light tints the lit rows. Left: the
      fields' own animations (clouds, water: a/0/1/1's animation files) are drawn at their first frame
- [x] NIMBASA's people tables (NETWORK NPC tables 0x1F3-0x1F8, 0x25C: Black's a/2/0/9 in the scripts pack's
      version 11; the BIG STADIUM / SMALL COURT trainers and crowd set by the zones' own scripts); scenario
      unova_stadium
- [ ] UNITY TOWER, the GEAR STATION / CASTELIA name and greeting boards (0x1D8-0x1E3, 0x1ED, 0x20C) as far as they
      don't need Wi-Fi

## 5. Closed (as Platinum's)

Online-only or the DS's own: the GTS and Wi-Fi club (0x150, 0x15F-0x166), Wi-Fi research surveys (0x210-0x21F),
mystery gift downloads (0x1D6 / 0x1D7), the POKe TRANSFER LAB's cartridge transfer (PALPARK 0x1C0 / 0x1C1), the
subscreen modes (one screen here), the wireless session commands (Kai's co-op stands in for them), the game manual
(0x244). Check each against Platinum's precedent before closing.
