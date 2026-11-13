# UNOVA: where it isn't yet 1:1 with Black

The user's rule (2026-11-02 for SINNOH, 2026-11-11 for UNOVA): UNOVA is 1:1 with Pokemon Black. Closed only: online-only
features (GTS, Wi-Fi) and what the DS itself needs (a second cartridge, the DS's wireless), as for Platinum. Everything
else that differs is a gap to correct, not a choice. But **anything that has an Emerald or Kai equivalent is not
reworked** (the user, 2026-11-13): Black's scripts open the game's own (its DAY CARE, screens, rules) and only what
neither has is ported from Black. This list was made 2026-11-13 from an audit of Black's script
commands the VM never plays or plays as nothing (src/unova/u3d_vm.c), the handoff's "not yet" notes and the decisions'
systems. Tick an entry only once it plays as Black's and has been seen running.

## 1. Drawn things missing

- [ ] **Gym machinery** (Black's field expansion objects, ov021 fld_exp_obj.c, each gimmick's G3dUtilSetup: models,
      textures and their animations): BURGH's honey doors (ov038), ELESA's coasters, carts and rails (ov039), CLAY's
      lifts and the lobby's elevator (ov043 / ov044), SKYLA's cannons and the flight (ov041), BRYCEN's switches and
      spinning pieces (ov045), DRAYDEN's dragons, levers and buttons (ov046), STRIATON's and NACRENE's (below)
- [ ] **Building animations**: doors opening and closing (SCREENEFFECTS 0x124-0x132, build-model nodes), machines'
      and lamps' BCA0 / BTP0 / BMA0 (field_buildmodel.c), the POKeMON LEAGUE's statues and stairs (0x12B-0x12E)
- [ ] The GEAR STATION's trains on the platforms; the ROYAL UNOVA as a boat
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

- [ ] STRIATON's gym switches (BUTTON_STATES 0x187-0x189, Ov037SetValueB 0x186)
- [ ] NACRENE's gym books (FLAG_ANIME 0x185, Ov037SetValueA 0x182)
- [ ] CASTELIA's (Ov037ReadEightVars 0x18F) and NIMBASA's (Ov037InitFromVars 0x17F) kept states as Black's

## 3. Script commands not played (story and towns)

- [ ] Word buffers: group / grouped / byte / signed / map entry names, an easy-chat word, the DAY CARE POKeMON's
      nickname (0x051, 0x05A, 0x05B, 0x05E-0x063): DRIFTVEIL, GEAR STATION, UNITY TOWER, CASTELIA, the centers
- [ ] People: move codes (0x066, 0x073), event flags (0x06A), params (0x079), searches (0x06F, 0x070), the player's rail
      place (0x07C), hops and jumps (0x0C5, 0x0C6), the player turned or offset (0x075, 0x076, 0x25B)
- [ ] A battle with a lent POKeMON (StartTrainerBattleWithMon 0x094: DRIFTVEIL, NACRENE)
- [ ] Gifts with set details (AddMonToPartyDetailed 0x10E: CASTELIA), friendship (0x0FD), forms (0x0FF, 0x117, ROTOM's
      move 0x11C), counts and records (0x108, 0x112, 0x114, 0x11B, 0x11D, 0x11F, 0x120, 0x242)
- [ ] Items: is a TM (0x0BA), party use (0x0BD), effect encounter item (0x0BC), table lookups (0x20E, 0x20F, 0x233,
      0x249, 0x24A), random pocket (0x238), daily gifts (0x234, 0x235), a TM's type (0x236)
- [ ] Places: fixed-zone warp (0x0C3), special location (0x0DB), a sign moved (0x1EA), field fade-in (0x1B4)
- [ ] POKeDEX completion and rating (0x1CE, 0x1CF: JUNIPER's / the professors' ratings)
- [ ] Trainer records (0x23A, 0x23B), the trainer card's events (0x24E-0x250: NUVEMA), names (0x218 / 0x219)
- [ ] Sound: wait for a song to stop (0x0A0), stop a sound (0x0A7), volumes (0x253 / 0x254), CHATOT's voice (0x24D)
- [ ] Black's wait / mode screens (0x151 CallModeScreen: ANVILLE, BATTLE SUBWAY, CASTELIA, GEAR STATION, NUVEMA)

## 4. Systems

- [ ] ROUTE 3's DAY CARE (0x0EB-0x0F8, 0x251) and EGG hatching (0x106): Black's scripts on the game's own DAY CARE
      and hatching (Kai's Gen 9 breeding), not reworked
- [ ] The MOVE RELEARNER (0x1E8 / 0x1E9), move deleter / setter (0x109, 0x10B), the move tutors (0x1E4-0x1E7): the
      game's own screens
- [ ] POKeMON MUSICALS (0x05D, 0x167-0x16E: the dress-up, the stage, the results, props, fans; four on stage with
      co-op friends)
- [ ] The ROYAL UNOVA (PLEASURE_BOAT 0x1A5-0x1AC: its passengers, the sunset, the time aboard)
- [ ] The ENTRALINK (INTRUDE / EVENT_COMM 0x16F-0x177: Black's forest, missions; co-op friends for the wireless),
      PASS POWERS (0x1F1 / 0x1F2, shareable), WHITE FOREST / BLACK CITY's people (0x1C5 / 0x1C6)
- [ ] The C-GEAR as UNOVA's device, the XTRANSCEIVER's calls (friends' video calls)
- [ ] Dark grass: two POKeMON at once (Black's double wild battles); shaking grass, dust clouds, rippling water
- [ ] POKeMON forms outside battle (ROTOM's appliances 0x117 / 0x11C, DEERLING / SAWSBUCK's seasons, form numbers
      0x0FF): Kai keeps forms its own way (battle recomputes); not reworked, unless the user says otherwise
- [ ] TRIPLE and ROTATION battles as Black's trainers have them (Kai's Triple Battles)
- [ ] To ask the user: Black's catch rules in UNOVA and Black's battle backgrounds were decided for UNOVA
      (UNOVA_DECISIONS.md) but the game has its own of both
- [ ] NIMBASA's people tables (NETWORK NPC tables 0x1F3-0x1F8, 0x25C)
- [ ] UNITY TOWER, the GEAR STATION / CASTELIA name and greeting boards (0x1D8-0x1E3, 0x1ED, 0x20C) as far as they
      don't need Wi-Fi

## 5. Closed (as Platinum's)

Online-only or the DS's own: the GTS and Wi-Fi club (0x150, 0x15F-0x166), Wi-Fi research surveys (0x210-0x21F),
mystery gift downloads (0x1D6 / 0x1D7), the POKe TRANSFER LAB's cartridge transfer (PALPARK 0x1C0 / 0x1C1), the
subscreen modes (one screen here), the wireless session commands (Kai's co-op stands in for them), the game manual
(0x244). Check each against Platinum's precedent before closing.
