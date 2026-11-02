# The user's list for the regions (2026-11-02)

To do once SINNOH is finished, pushed, built and released, without stopping to ask: decide and note the decision
here ("Decisions" below). Priority when sources disagree: Kai > FireRed (KANTO) / Crystal + HGSS (JOHTO) > Quetzal.
Quetzal's fan additions (GIOVANNI's quest and the like) aren't wanted. Status: [ ] to do, [~] begun, [x] done.

## KANTO
- [ ] FireRed's decomp settles any behavior question (Kai > FireRed > Quetzal)
- [ ] KANTO's legendaries only in KANTO (exclusives)
- [x] FireRed's RUNNING SHOES aide after BROCK: back (FireRed's scene and words; a line of his own if the player has
      them already; hide flag 0x2224, done 0x2223; test kanto_shoes_aide)
- [ ] Doorways too wide: one tile wide
- [x] Door opening / closing animations: FireRed's doors (its door table, pictures and drawing: src/mp/mp_frlgdoor.c,
      tools/quetzal_port/gen_frlg_doors.py; test kanto_doors)
- [ ] Going down some ladders puts the player on top of the ladder, not in front of it
- [x] MEGA PUNCH / MEGA KICK people outside MT. MOON: FireRed's two move tutors on Kai's tutor flow, FireRed's words
      (Quetzal's said "move training is handled from your team menu"; flags 0x21DF / 0x21DC; test kanto_tutors)
- [ ] CERULEAN: TRIPLE TRIAD first; people who offer a Yu-Gi-Oh! / POKéMON starter deck get words by their personality
- [x] The shopping area (RARE CANDY, Z / MEGA items whatever GENERATION): Quetzal's market stalls in VERMILION (not
      VIRIDIAN: VIRIDIAN has only FireRed's mart), all nine hidden (flag 0x2225, set as VERMILION loads; their empty canopies are map tiles, left; test kanto_no_market)
- [ ] S.S. ANNE: its sprite, FireRed's departure scene, can't be boarded once gone
- [x] The CUT man gives HM01 (not Quetzal's flag): the S.S. ANNE CAPTAIN, FireRed's (test hm_once); KANTO's other HM
      givers too, FireRed's words (HM02 ROUTE 16, HM03 SAFARI ZONE, HM04 the WARDEN, HM05 OAK's AIDE, HM06 EMBER SPA,
      HM07 ICEFALL CAVE's ball: tools/quetzal_port/fix_hm_givers.py; test hm_kanto); every region: an HM never given
      twice (item.c MP_PlayerHasHm; HOENN's and KANTO's givers say so: EventScript_GiveHmOnce / FindHmOnce)
- [x] JOHTO's HM givers give the HM they talk about (Crystal's: HM01 ILEX's CHARCOAL master, HM02 CIANWOOD, HM03 the
      DANCE THEATER, HM04 OLIVINE CAFE, HM05 SPROUT TOWER, HM07 ICE PATH's ball; HGSS's HM06 ROCK SMASH on ROUTE 36)
- [x] Water animates (FireRed's General tileset animations: water, its edges, flowers; src/tileset_anims.c
      TilesetAnim_QzGeneral, tools/quetzal_port/gen_tile_anims.py; test qz_water_anim)
- [x] The CUT tree east of CERULEAN (to ROUTE 9 / ROCK TUNNEL) can be cut (test kanto_cut_tree)

## JOHTO
- [ ] POKéGEAR back as Crystal / HGSS have it (display, functions, calls), no manual clock setting; START menu in
      JOHTO, a registrable key item elsewhere once JOHTO's ELITE FOUR is beaten
- [ ] JOHTO's legendaries only in JOHTO
- [ ] Quetzal's berry system out: 1x1 berry patches where berries were; APRICORN trees rewritten from Crystal
      (behavior, words), JOHTO's berries
- [x] WHITNEY's MILTANK: perfect IVs (her gym team; tools/quetzal_port/gen_trainers.py IV_OVERRIDES)
- [x] Quetzal's auto walk / run: its words gone (MOM in NEW BARK TOWN)
- [ ] Door animations; [x] water animates (JOHTO's primaries 2-6 carry FireRed's water tiles: the same animation over
      them; primary 1's water is drawn differently and stays still)
- [ ] The rival named as in Crystal (online: the host names him, said plainly), his starter the one beating the player's
- [x] No tree-cutting prompt in a new game without the HM and badge (Quetzal's check said yes to everything)
- [ ] MOM's money saving (Crystal) and MY ROOM decorating on the Secret Base logic

## HOENN
- [ ] HOENN's legendaries only in HOENN
- [ ] MOENAV: START menu in HOENN (done), a registrable key item elsewhere once HOENN's ELITE FOUR is beaten

## SINNOH
- [ ] Plates and SINNOH's legendaries only in SINNOH
- [ ] Palettes inverted when the time-of-day lighting changes on a running map (maybe fixed)
- [ ] Falling through CYCLING ROAD's tunnels (maybe fixed)
- [ ] BERRY patches can't be planted (maybe fixed)
- [ ] Followers cut in half in 3D (maybe fixed)
- [ ] Seams where the map ends and the out-of-bounds loop begins
- [ ] Water animates
- [ ] POKéTCH a registrable key item outside SINNOH (on / off, the right stick as usual) once SINNOH's ELITE FOUR is
      beaten

## All added regions
- [ ] Events: FireRed for KANTO, then HGSS for gaps
- [x] A non-HOENN gym leader beaten: the gym's unbattled trainers count as beaten (mp_coop.c MP_Coop_GymLeaderBeaten;
      test sinnoh_gym)
- [x] BADGES count every region's badges (the continue screen and the SAVE window: MP_Region_TotalBadges)
- [ ] A4: TMs / HMs from leaders, trainers and events as the original games give them (new TMs / HMs where HOENN's
      list lacks them)
- [ ] SINNOH's time of day in KANTO, JOHTO and HOENN: MORNING / DAY / NIGHT encounters
- [ ] Encounter audit: no strong Pokémon early, places keep their identity (DIGLETT'S CAVE), no DITTO-like utility early;
      each region's own Pokémon mainly there, the other regions' Pokémon spread where they fit; never remove one already
      found somewhere; FILL never sends OFF GENERATION, OFF REGION very sparingly

## General
- [ ] Shadows fade with screen transitions
- [ ] No extra shadow under a follower's reflection
- [ ] A follower behind the player drawn over the player when facing / moving down
- [x] OPTIONS: RETURN TO MAIN MENU (GAME tab, from the field: the save prompt, declined: sure?, then the save read again
      and the title screen; the PC build's soft reset quits, so it's main_menu.c MP_ReturnToTitleNow; test options_title)

## Decisions
(each assumption made while working through the list, with why)
- FireRed "clone" objects (an object copying a neighbor map's one at the map's edge: trainer_type 65535, the target's
  local id in elevation, its map in the range) aren't supported by Emerald's engine: the 9 in KANTO's maps now carry
  their original's script, flag and elevation (cutting either side counts for both: the same flag).
- Field moves check the region's own badge: FireRed's per HM in KANTO, Crystal's in JOHTO (ROCK SMASH, a TM in
  Crystal: the ZEPHYRBADGE; DIVE, which neither has: the last badge), Platinum's in SINNOH (mp_region.c
  MP_Region_FieldMoveBadge). Quetzal's tree / rock / boulder / whirlpool scripts now need that badge and a party Pokémon
  that knows the move or holds its HM (or a friend's), as Kai's own scripts.
- SINNOH released before it's 100%: the parts still open need art or songs that don't exist yet (contest stage art,
  Platinum songs the game lacks) or are large (the DISTORTION WORLD, the full story test); a release with what's done and
  the KANTO / JOHTO fixes is more use to players than waiting. SINNOH's remaining list stays in docs/SINNOH_CHECKLIST.md.
- An HM a player already has (BAG, PC or held) is never handed over again by a script (scrcmd.c additem: a held HM
  going back to the BAG is C's own AddBagItem, untouched). HOENN's and KANTO's givers
  run EventScript_GiveHmOnce, which says "Oh! You're already carrying <HM>..." instead; SINNOH's givers keep Platinum's
  words (no original lines there) and simply add nothing. The S.S. ANNE CAPTAIN's METAL COAT (Quetzal's) is gone.
- JOHTO's whirlpools: Kai has no WHIRLPOOL HM (only level-up learners), so one opens for the GLACIERBADGE and a Pokémon
  with WHIRLPOOL or SURF until A4 adds an HM for it (qz_specials.c).
- ROUTE 4's MEGA PUNCH / MEGA KICK people are move tutors in FireRed (not TM givers), so they teach once, as FireRed's
  and Kai's tutors do. KANTO's other FireRed tutors are back the same way (DOUBLE-EDGE, THUNDER WAVE, ROCK SLIDE,
  EXPLOSION, DREAM EATER, SOFTBOILED, SUBSTITUTE, SWORDS DANCE, SEISMIC TOSS, METRONOME, BODY SLAM:
  tools/quetzal_port/fix_move_tutors.py). Still Quetzal's stand-ins: CAPE BRINK's starter tutor and TWO ISLAND's move
  maniac (FireRed systems of their own) and SAFFRON's COPYCAT (MIMIC, needs her POKé DOLL trade).
- JOHTO's doors keep still: its tilesets are Quetzal's own, their door metatiles aren't FireRed's numbers.
