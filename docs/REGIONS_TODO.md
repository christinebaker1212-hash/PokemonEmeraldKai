# The user's list for the regions (2026-11-02)

To do once SINNOH is finished, pushed, built and released, without stopping to ask: decide and note the decision
here ("Decisions" below). Priority when sources disagree: Kai > FireRed (KANTO) / Crystal + HGSS (JOHTO) > Quetzal.
Quetzal's fan additions (GIOVANNI's quest and the like) aren't wanted. Status: [ ] to do, [~] begun, [x] done.

## KANTO
- [ ] FireRed's decomp settles any behavior question (Kai > FireRed > Quetzal)
- [ ] KANTO's legendaries only in KANTO (exclusives)
- [ ] FireRed's RUNNING SHOES aide after BROCK doesn't appear: remove him
- [ ] Doorways too wide: one tile wide
- [ ] Door opening / closing animations
- [ ] Going down some ladders puts the player on top of the ladder, not in front of it
- [ ] MEGA PUNCH / MEGA KICK people outside MT. MOON give their TMs (A4)
- [ ] CERULEAN: TRIPLE TRIAD first; people who offer a Yu-Gi-Oh! / POKéMON starter deck get words by their personality
- [ ] VIRIDIAN's shopping area (RARE CANDY, Z / MEGA items whatever GENERATION): remove (CELADON DEPT. covers it)
- [ ] S.S. ANNE: its sprite, FireRed's departure scene, can't be boarded once gone
- [x] The CUT man gives HM01 (not Quetzal's flag): the S.S. ANNE CAPTAIN, FireRed's (test hm_once); KANTO's other HM
      givers too, FireRed's words (HM02 ROUTE 16, HM03 SAFARI ZONE, HM04 the WARDEN, HM05 OAK's AIDE, HM06 EMBER SPA,
      HM07 ICEFALL CAVE's ball: tools/quetzal_port/fix_hm_givers.py; test hm_kanto); every region: an HM never given
      twice (item.c MP_PlayerHasHm; HOENN's and KANTO's givers say so: EventScript_GiveHmOnce / FindHmOnce)
- [x] JOHTO's HM givers give the HM they talk about (Crystal's: HM01 ILEX's CHARCOAL master, HM02 CIANWOOD, HM03 the
      DANCE THEATER, HM04 OLIVINE CAFE, HM05 SPROUT TOWER, HM07 ICE PATH's ball; HGSS's HM06 ROCK SMASH on ROUTE 36)
- [ ] Water animates
- [x] The CUT tree east of CERULEAN (to ROUTE 9 / ROCK TUNNEL) can be cut (test kanto_cut_tree)

## JOHTO
- [ ] POKéGEAR back as Crystal / HGSS have it (display, functions, calls), no manual clock setting; START menu in
      JOHTO, a registrable key item elsewhere once JOHTO's ELITE FOUR is beaten
- [ ] JOHTO's legendaries only in JOHTO
- [ ] Quetzal's berry system out: 1x1 berry patches where berries were; APRICORN trees rewritten from Crystal
      (behavior, words), JOHTO's berries
- [ ] WHITNEY's MILTANK: perfect IVs
- [ ] Quetzal's auto walk / run: remove its words
- [ ] Door animations; water animates
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
- [ ] A non-HOENN gym leader beaten: the gym's unbattled trainers count as beaten
- [ ] BADGES count every region's badges
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
- [ ] OPTIONS: RETURN TO MAIN MENU (save? no: sure? then soft reset)

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
- An HM a player already has (BAG, PC or held) is never added again (item.c AddBagItem). HOENN's and KANTO's givers
  run EventScript_GiveHmOnce, which says "Oh! You're already carrying <HM>..." instead; SINNOH's givers keep Platinum's
  words (no original lines there) and simply add nothing. The S.S. ANNE CAPTAIN's METAL COAT (Quetzal's) is gone.
- JOHTO's whirlpools: Kai has no WHIRLPOOL HM (only level-up learners), so one opens for the GLACIERBADGE and a Pokémon
  with WHIRLPOOL or SURF until A4 adds an HM for it (qz_specials.c).
