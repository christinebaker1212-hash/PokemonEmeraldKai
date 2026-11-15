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
- [x] The gym models' texture animations (BTA / BTP / BMA, 2026-11-15, seen running: scenarios unova_burgh_gym,
      unova_elesa_gym, unova_gym_lights): BURGH's switch glows, plates, the doors' effect model and the honey cocoon
      (which also brought back Black's scripted honey trainers), ELESA's cart gates and lane levers, CLAY's lift lamps
      and elevator, BRYCEN's switch lights, DRAYDEN's buttons, SKYLA's cannons' colours. Baked per frame into the
      pack (v7) by tools/unova/gimmicks.py from nitro.py's Btp0 / Bta0 / Bma0; u3d.c U3D_ExpTex. A few of Black's
      BTA tracks name materials their models don't have (ov042, ov045's track 0, ov046's extra model), so they change
      nothing here (not yet compared with Black running)
- [x] SKYLA's flight camera (2026-11-15, scenario unova_skyla_cannon): each flight's gCannonsCameraInfos (pitch, yaw,
      move / follow / return frames; gen_gyms.py sCannonCamera) turn the view before the shot (u3d.c U3D_CameraTurn),
      the eye held while the view follows the flying player after the landing frame's bounce (U3D_CameraLook), then
      back (U3D_CameraMoveBack); the landing sound at Black's frame (gCannonsLandSeFramesBeforeEnd)
- [x] **Building animations** (2026-11-15, scenario unova_doors): every animated build model (pack v8: a prop's
      kind, Black's animation kind, slots a group, groups and model type; field_buildmodel.c's anim sets, one a model,
      u3d.c BuildingStep): kind 1 looping (windmills, lamps, signs), kind 3 the season's slot, kind 2 played by doors
      (an automatic door, entry kind 4, opened / held / shut for all of its model; a house's, entry kind 1, on its own
      node: walked at, it opens first, and it opens, lets the player out and shuts on arriving) and by the scripts'
      0x124-0x132 (nodes made, played, waited on and deleted; entries played in place; the door by the player):
      ROUTE 10's badge gates swing open. Not drawn: SwapBuildModelId / SetBuildModelIndex (a model swapped for another)
- [x] Camera moves from Black's archive (0x146: archive 0xA3, tools/unova/gen_cameras.py -> u3d_cameras.h, with their
      channels: u3d.c U3D_CameraMoveMasked): ROUTE 10's gates turn the view to the gate
- [x] CHARGESTONE CAVE's stones (ov051 field_gimmick_d06.c, 2026-11-15): each floor's floating stones (Black's
      effect models from the effects archive 0x4B as pack gimmicks 151-153: the stone and its float, its shadow, the
      panels' glow) on their tiles at their own level; walked into, a stone slides straight on to the electric panel
      it meets (none that way: it won't budge; one resting at a panel stays, its float stopped), the panels flash
      when walked up to, where they rest kept in the save (u3d_vm.c D06*, u3d.c U3D_StoneStep before the map's own
      collision); scenario unova_chargestone (`mp u3dstones`)
- [x] The GEAR STATION's platform trains (ov052 / ov021 fldeff_btrain.c, 2026-11-15): the subway's script commands
      18 / 34 bring a line's train (Black's eight models and their four door animations, pack gimmick 52) to the
      platform, 19 plays its doors, 20 hides it (u3d_subway.c); the carriage (zone 75) sways as Black's does, until
      command 350 stops it (U3D_Subway_SwayFrame, u3d.c U3D_ViewBob); scenario unova_subway_train
      (`mp u3dsubwaytool CMD OP1 OP2`)
- [x] ROUTE 4's traffic (ov058 field_gimmick_lane_trains.c, 2026-11-15; zones 326 / 157): the trucks and cars on the
      lanes drive by as Black's (setup 58 in tools/unova/gimmicks.py, u3d_vm.c LanesLoad / LanesFrame); scenario
      unova_lane_traffic
- [x] SKYARROW's and TUBELINE's traffic and its sound (ov053 / ov054 sound objects, 2026-11-15; zones 249 / 254):
      SKYARROW's cars both ways (a lead and its trail) up and down the ramps and along the road, turned with the
      slope, and the ship below; TUBELINE's trains each way: along Black's own paths (tools/unova/gen_soundobjs.py ->
      u3d_soundobjs.h), a random wait at the start between runs (pack gimmicks 53 / 54, u3d_vm.c SoundObjs*,
      U3D_ExpPlaceRot). Black's 3D sound: the songs' tracks that Black silences on arrival (archive 0xB9) kept as
      tracks of their own by the studio (MMS_RIP_PARTS -> INFO parts=), each brought in by its vehicle's distance from
      the camera and panned to its side; SKYARROW's high part comes in as the camera rises, and the rush of wind as the
      view drops at the bridge's end (SE_M_GUST for Black's SEQ_SE_FLD_173); scenario unova_bridge_traffic
- [x] ROUTE 10's bridge (ov047 field_gimmick_bridge.c, 2026-11-15; zone 362): on its planks (Black's no-dash ground,
      terrain 0x33: phenomenon kind 9 in the pack) the player sways up and down, 8 degrees of a sine a Black frame to
      a height picked each swing (8, 8, 12 or 14 units), the view staying put (u3d_vm.c BridgeFrame, U3D_PlayerBob);
      scenario unova_bridge_sway (debug u3dkind K)
- [x] The LEAGUE's hub and the descent to the CHAMPION (ov049 / ov050, script command 0x1BB: ov010 namein_setup.c
      RunFallEvent / RunLandEvent; 2026-11-15, zones 137 / 138): the hub's four lamps lit by the ELITE FOUR's flags and
      its centre alight with all four; the centre statue's platform sinks with the player into its pit after Black's
      camera move (tools/unova/gen_league.py -> u3d_league.h: archive 0x9D's paths), the screen goes black, and in the
      CHAMPION's room the platform comes down from above with its glow, lands with a thud and a shake (u3d_vm.c
      League*, U3D_PlayerDepthTest); scenario unova_league_descent
- [x] The special POKeMON's scenes (ov059 sp_poke_gimmick.c, script commands 0x221-0x224 and 0x23E; 2026-11-15, zones
      47 / 53 / 346): a distributed POKeMON in the party (CELEBI, MELOETTA, KELDEO...) sent out and back in Black's
      ball models along their arcs, played and waited on as Black's, and in CASTELIA ZORUA's puff of smoke over the
      lost boy, the swap at its frame 15 (pack gimmicks 59 / 159, u3d_vm.c SpPoke*); scenario unova_special_poke
- [x] LIBERTY GARDEN's ship (ov062 field_gimmick_62.c, 2026-11-15; zone 235): the LIBERTY ship moored at the pier
      with its water moving (pack gimmick 62); after script command 0x23C it's gone the next time the island loads,
      until the player has been elsewhere (Black's gimmick work 0x34, which another place's gimmick takes back);
      scenario unova_liberty_ship
- [ ] Black's other field gimmicks (arc 0x65's zone table, ov021 gFieldGimmickOverlayIds): the ENTRALINK's gauge and
      markers (ov060 / ov061)
- [x] Camera shakes (0x148: ABYSSAL RUINS, CHARGESTONE, DRAGONSPIRAL, N's CASTLE; 2026-11-15, scenario
      unova_camera_shake): ShakeTask's swings, their decay and the script's wait (u3d.c U3D_CameraShake)
- [x] The ELITE FOUR rooms' entrance scenes (ov056 field_fk_demo.c, script command 0x1A2 with the room's slot;
      2026-11-15, zones 140-143): Black's set for each room with its hero card (the player's gender's frames) and its
      joint animation (pack gimmicks 171-174), the view along Black's camera path (tools/unova/gen_fkdemo.py ->
      u3d_fkdemo.h, U3D_CameraPlace), its sounds at their cues (Black's own numbered ones as the game's nearest), slots
      3 and 4 playing the room's building, then the player on the room's end tile (u3d_vm.c Fk*); scenario
      unova_e4_entrance. (0x1A3 sets Black's camera setting for the room: our view keeps Emerald's framing there, as
      everywhere off the rail maps.)
- [ ] The ELITE FOUR rooms' own sounds (0x1A4 StartFkDemoSubProc: a building's sounds by its animation's frames, two
      ambient loops until work 0x4001 is set)
- [ ] 3D cut-ins (RunFld3dCiMain 0x1A1: DESERT RESORT, NACRENE)
- [ ] People flown along Black's paths (built; MmdlIca 0x15B-0x15E: archive 0xCB by tools/unova/gen_icas.py -> u3d_icas.h;
      u3d.c U3D_ObjectFly draws the person at the path's point from where the player stood, set down on the tile at
      its end; 2026-11-15, scenario unova_flown_person: the ABUNDANT SHRINE's LANDORUS flies in and out by the logs,
      but the scene's camera keeps it off the test's shots, so not yet seen on screen)
- [x] A person rising / circling (0x225 / 0x226), dropping in (0x247, RELIC CASTLE), spinning away (SpinRise, the
      LEAGUE): u3d_vm.c Motion* on U3D_ObjectFly, as scrcmd_mmdl.c / scrcmd_fldmmdl.c move them (2026-11-15, scenario
      unova_person_motions by `mp u3dmotion`; CASTELIA's rise and circling belong to the distributed MELOETTA's scene)
- [x] The LIGHT STONE's glow and N's thrown ball (0x23F, 0x255-0x258: ov064 field_gimmick_arc_effect.c in the throne
  room, zone 278): pack gimmick 64 (gimmicks.py: gArcEffectDefaultUnitSetup's two models; parts flat at rest, a node
  scaled to nothing, now taken from their first shaped frame), u3d_vm.c ArcGlowStart / ArcFlyStart / ArcShowAt /
  ArcFrame (the glow at the player, its sounds at Black's frames, the white flash at frame 101 held 15 frames; the
  ball flown from N to the dais over the script's frames, arched, then opened there; the peak and end waits). Black's
  fades on screens 4 / 8 (FADE_Start 0xC) are white now. Scenario unova_light_stone (`mp u3darc LOCAL`: the throw
  without N's battle before it).
- [ ] Face-up portraits (FLD_FACEUP 0x1D3-0x1D5: N's CASTLE, NIMBASA)
- [x] The gates' electric news boards (ov048, 2026-11-15; src/unova/u3d_elboard.c, data tools/unova/gen_elboard.py ->
      u3d_elboard.h; pack gimmick 48, the English game's lanes): each gate's record places the board and its TV's
      picture; the lines Black writes (the date, "place: weather" for its four places by Black's weather calendar and
      the roamers' storms, the day's three sayings or the story's news by their flags, the week's notice) drawn in the
      small font into each lane's own texture (u3d.c U3D_ExpOwnTexture), the lamps flickering orange / amber, scrolled
      one after another by the lanes' tracks as GIMMICK_OBJ_ELBOARD_UpdateLine; scenario unova_news_board. Not yet:
      the HALL OF FAME team for a day after the LEAGUE (gate points, 0x960 / 0x97B), the board's camera view
      (0x17E's own command)
- [x] Field weather (2026-11-15): Black's weather calendar (ov010 calender.c: 68 zones, a weather a day of the year)
      and the roamers' storms (TORNADUS's downpour, THUNDURUS's thunderstorm where they roam) as the map's weather on
      every UNOVA map load (field_weather_effect.c SetSavedWeatherFromCurrMapHeader -> u3d_elboard.c
      U3D_FieldWeather: rain, snow (also for Black's hail, heavy snow and diamond dust), sandstorm, mist as fog);
      scenario unova_weather (`mp weather`)
- [x] Mass outbreaks (2026-11-15; u3d_wild.c SwarmFill / U3D_SwarmZone): after the HALL OF FAME (flag 0x960) one of
      Black's 17 swarm routes a day (sSwarmTable: the day and the player's ID pick it), 400 in 1000 of its grass's
      POKeMON at Lv 15-55, and its line first on the gates' boards; `mp u3dswarm`, scenario unova_news_board
- [x] The dragons' levels in DRAYDEN's gym (2026-11-15): its walkways and the dragons' necks are Black's rails, got
      on and off only as field_rail.c lets them (along a line, across within its width, off its ends): each cell's
      ways at its height (tools/unova/gen_gyms.py rail_moves -> u3d_gyms.h sDragonRails), a step any other way none
      (u3d_vm.c U3D_Vm_RailWayBlocked from u3d.c U3D_StepBlocked), so the poses force Black's route; scenario
      unova_dragon_levels (`mp u3dheights`, `mp u3dreach X Y`)

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
- [x] The BATTLE SUBWAY's map (0x151 mode 2, the user's approved mock-up 2026-11-15: Black's a/2/0/6 picture in the
      scripts pack's version 12, shown 1:1, centred, the D-pad scrolling: src/unova/u3d_subwaymap.c; scenario
      unova_subway_map)
- [x] The player set on a rail (0x07C: zone 66, the GEAR STATION; its two places as their tiles by tools/unova/rails.py
      tile_of; scenario unova_rail_place)
- [x] The battle phrases asked for (0x1ED kinds 0-3, zone 66): the game's own EASY CHAT on the save's introduction /
      won / lost phrases ("No. 1" asked, not kept: Kai has none); scenario unova_phrases
- [x] Black's own words (0x1ED kind 4, 2026-11-15): the PEP QUIZ (zone 117) asks for a word as Black's PMS input
      does; here it's typed on the game's naming screen (NAMING_SCREEN_UNOVA_WORD, the host's own "Challenger, please
      answer!" over it) and found in Black's 13 word lists (pms_word.c's banks: POKeMON, moves, types, Abilities and
      Black's own words; case, accents and PK MN alike), its number given to the script's check (u3d_vm.c
      SCREEN_WORD / WordId); scenario unova_pep_quiz. MR. LOCK's password (kind 5, zone 50) is an event's (closed)
- [x] Counting POKeMON from other trainers (0x239: the party and every box, EGGS left out, 50 at most), NPC
      greetings by the clock and the party (0x23D with Black's record / message tables), the birthday lines (0x20D:
      Black's other days', no DS birthday here), the barred POKeMON named (0x1B9), the GeoNet country (0x246: none);
      scenario unova_greetings
- [x] Zone 389's time effect (0x24E-0x250: overlays share addresses, so these are ov065's, not the trainer card's;
      2026-11-15, scenario unova_time_effect): the effect model by day or night (gimmicks 65 / 165 in the pack), its
      pending flag, played with the script waiting, then hidden (u3d_vm.c TimeFx*); in ROUTE 1 (317) and zone 397 too,
      as arc 0x65 has it (gimmick 54 in all three)
- [x] CASTELIA's crowds (0x134 starts ov055's field_gimmick_crowd.c in zones 32-34 / 254; 2026-11-15, scenario
      unova_crowd): Black's setups (tools/unova/gen_crowds.py -> u3d_crowds.h), people spawned on each area's edge at
      the time of day's rate and walked its length, solid ones stepping aside two tiles out, one in ten player-aware
      (stops before the player and can be talked to: its look's script), local ids 0xA0 + slot (u3d_vm.c Crowd*)
- [ ] The crowd's chatter: Black's small windows popping up over the street (bank 0x9F) need a mock-up first
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
- The XTRANSCEIVER's friends' video calls: closed (the user, 2026-11-15: "video calls aren't important")
- [x] Dark grass: two POKeMON at once (Black's double wild battles: 40 in 100 with two able to battle,
      wild_encounter.c U3dStartWild; test: mp u3dwild 3 b, MOEMON_U3D_DARK_DOUBLE=100)
- [x] Shaking grass, dust clouds, rippling water, birds' shadows (Black's ov021 effect_encount.c: after the first
      badge, the zone's steps and odds, a tile round the player of the pack's encounter-effect kinds; battles from
      Black's special tables, a dust cloud's gem / stone and a shadow's wing by Black's script 10133) and UNOVA's
      fishing (Black's table and odds, a sure bite at the ripples): src/unova/u3d_phenom.c; scenario unova_phenomena
- [x] POKeMON forms outside battle (the user, 2026-11-15: every region): GetMonForm 0x0FF, ChangeMonForm 0x117 (the
      meteorite), SetRotomFormMove 0x11C (the appliances), trainers' forms, DEERLING / SAWSBUCK's seasons: src/mp/mp_forms.c,
      COOP_NOTES "POKéMON forms outside battle, in every region"; scenario forms_unova
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
