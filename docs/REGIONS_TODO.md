# The user's list for the regions (2026-11-02)

To do once SINNOH is finished, pushed, built and released, without stopping to ask: decide and note the decision
here ("Decisions" below). Priority when sources disagree: Kai > FireRed (KANTO) / Crystal + HGSS (JOHTO) > Quetzal.
Quetzal's fan additions (GIOVANNI's quest and the like) aren't wanted. Status: [ ] to do, [~] begun, [x] done.

**2026-11-04, the user's audit decisions (docs/REGIONS_AUDIT.md) override this file where they differ:** JOHTO is
strictly Crystal (HGSS no longer ranks above Quetzal: Quetzal's HGSS-style events go); legendaries and mythicals that
belong to the original games' regions stay (up to SINNOH), Quetzal's placements go, and Gen 5+ legendaries leave the
overworld (MOEGATCHA pulls only). The audit's findings: docs/regions_audit/.

## KANTO
- [ ] FireRed's decomp settles any behavior question (Kai > FireRed > Quetzal)
- [x] KANTO's legendaries only in KANTO (exclusives: qz_specials.c MP_LegendForeign / MP_HideForeignLegends; test
      legend_exclusive)
- [x] FireRed's RUNNING SHOES aide after BROCK: back (FireRed's scene and words; a line of his own if the player has
      them already; hide flag 0x2224, done 0x2223; test kanto_shoes_aide)
- [x] Doorways too wide: one tile wide (Emerald's door table, with its two-tile-wide doors, was drawing over KANTO's
      door metatiles; FireRed's own doors now: mp_frlgdoor.c, test kanto_doors)
- [x] Door opening / closing animations: FireRed's doors (its door table, pictures and drawing: src/mp/mp_frlgdoor.c,
      tools/quetzal_port/gen_frlg_doors.py; test kanto_doors)
- [x] Going down some ladders puts the player on top of the ladder, not in front of it (an up ladder in the wall
      steps the player off, south: field_screen_effect.c MpLadderStepOff, KANTO and JOHTO; test kanto_ladder)
- [x] MEGA PUNCH / MEGA KICK people outside MT. MOON: FireRed's two move tutors on Kai's tutor flow, FireRed's words
      (Quetzal's said "move training is handled from your team menu"; flags 0x21DF / 0x21DC; test kanto_tutors)
- [x] CERULEAN: TRIPLE TRIAD first; people who offer a Yu-Gi-Oh! starter deck say so in their own way (the eight ways
      card players talk: kid, stylish, tough, sea, refined, brainy, team, staff; a gift or a loan: ygo_ui.c
      ResolveNoDeck). (Only Yu-Gi-Oh! duelists hand out a deck when challenging; POKéMON card players don't.)
- [x] The shopping area (RARE CANDY, Z / MEGA items whatever GENERATION): Quetzal's market stalls in VERMILION (not
      VIRIDIAN: VIRIDIAN has only FireRed's mart), all nine hidden (flag 0x2225, set as VERMILION loads; their empty canopies are map tiles, left; test kanto_no_market)
- [x] S.S. ANNE: FireRed's ship (its 128 x 64 picture as a map picture, mp_mappic.c), its departure (glides off, horns:
      qz_specials.c QzDoSSAnneDepartureCutscene), gone for good once it sails (Quetzal's dock brought it back each
      visit); FireRed's wake and smoke puffs not drawn; test kanto_ss_anne
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
- [x] POKéGEAR back as Crystal / HGSS have it (display, functions, calls), no manual clock setting; START menu in
      JOHTO, a registrable key item elsewhere once JOHTO's ELITE FOUR is beaten (src/mp/mp_pokegear.c: CLOCK, MAP (the
      TOWN MAP), PHONE (MOM with her savings question, PROF. ELM; Crystal's words), RADIO (OAK's POKéMON TALK in
      Crystal's words with the region's wild POKéMON and places, the POKé FLUTE); MOM gives it as in Crystal; item 693;
      test johto_pokegear. 1.68.0: every Crystal station with Crystal's own songs, built from pret/pokecrystal by
      tools/kaisong/crystal_radio.py: POKéDEX SHOW, POKéMON MUSIC, LUCKY CHANNEL, BUENA's PASSWORD, UNOWN, EVOLUTION
      radio, TEAM ROCKET's takeover, PLACES AND PEOPLE, LET'S ALL SING, POKé FLUTE; tests johto_radio*)
- [x] JOHTO's legendaries only in JOHTO (FireRed's and Emerald's NAVEL ROCK empty: LUGIA and HO-OH are JOHTO's)
- [x] Quetzal's berry system out: 1x1 berry patches where berries were; APRICORN trees rewritten from Crystal
      (behavior, words), JOHTO's berries (src/mp/mp_apricorn.c, tools/quetzal_port/fix_fruit_trees.py: Crystal's
      APRICORN trees (AZALEA, ROUTE 37's and 42's three each), one a day, its words; KURT makes the BALLS in a day
      (choose the APRICORN and how many); items 686-692 with PokeAPI's icons (tools/moemon_items/gen_apricorns.py);
      the other 17 trees are BERRY patches on JOHTO's own tree numbers (the GARDEN farm's 100-109 were shared),
      starting with Crystal's BERRIES in their Gen 3 names; test johto_apricorn)
- [x] WHITNEY's MILTANK: perfect IVs (her gym team; tools/quetzal_port/gen_trainers.py IV_OVERRIDES)
- [x] Quetzal's auto walk / run: its words gone (MOM in NEW BARK TOWN)
- [x] Door animations (Quetzal's own door table and frames from its ROM: tools/quetzal_rip/rip_doors.py,
      src/data/qz_doors.h; sliding doors slide, swinging ones swing; test johto_doors); [x] water animates (JOHTO's primaries 2-6 carry FireRed's water tiles: the same animation over
      them; primary 1's water is drawn differently and stays still)
- [x] The rival named as in Crystal (online: the host names him, said plainly), his starter the one beating the player's
      (his starter: ELM's ball he takes and every team's starter line, VAR_JOHTO_STARTER 0x52FB, qz_specials.c
      MP_JohtoRivalSpecies, mp_region.c MP_Region_TrainerData; his name: the officer at ELM's opens Crystal's naming
      screen with his picture (NAMING_SCREEN_RIVAL; empty: SILVER), kept in JOHTO's vars 0x52FC-0x52FF, {RIVAL} in
      JOHTO's words and his battles' name; in a friend's room the host's name (MPR_RULES, protocol 73) and no naming
      screen; tests johto_rival_starter, johto_rival_name, johto_rival_name_online)
- [x] No tree-cutting prompt in a new game without the HM and badge (Quetzal's check said yes to everything)
- [x] MOM's money saving (Crystal) and MY ROOM decorating on the Secret Base logic ([x] the BANK OF MOM: her offer as the
      player leaves, a quarter of JOHTO / KANTO winnings, GET / SAVE / CHANGE at home with Crystal's digit entry and words:
      src/mp/mp_mombank.c, test johto_mom_bank; [x] MY ROOM: JOHTO's room decorated on the bedroom's logic (its PC is the
      bedroom PC with DECORATION; dolls and cushions on its plain floor; its own list in JOHTO's vars 0x52D0-0x52DB, a
      decoration in one room counting as in use in the other: mp_region.c MP_RoomDecorations; test johto_my_room))

## HOENN
- [x] HOENN's legendaries only in HOENN (KANTO's BIRTH ISLAND and Quetzal's CINNABAR chamber empty)
- [x] MOENAV: START menu in HOENN (only there now), a registrable key item elsewhere once HOENN's ELITE FOUR is beaten
      (ITEM_MOENAV 685, given on the next map load: mp_region.c MP_Region_GiveDevices; test moenav_key)

## SINNOH
- [x] Plates and SINNOH's legendaries only in SINNOH (Quetzal's plate shops at CINNABAR's lab and the RUINS OF ALPH
      gone: FireRed's / Crystal's scientists back)
- [x] Palettes inverted when the time-of-day lighting changes on a running map (checked: on ROUTE 201 the light switched
      noon -> dusk -> night -> noon in place (`mp s3dhour`) gives Platinum's colors each time and noon's again, nothing
      inverted)
- [x] Falling through CYCLING ROAD's tunnels (ROUTE 206 40.26: the lanes are Platinum's bridge tiles, 0x71, over the
      valley floor between them at rows 72-78; the pack keeps them as behavior 0x1D (build_pack.py S3D_BRIDGE) and, up on
      the deck, a step off its side onto ground far below is blocked as in Platinum: s3d_gyms.c S3D_Gyms_StepBlocked;
      test sinnoh_cycling_road. Tiles with more than three ground surfaces keep the floor and the two highest)
- [x] BERRY patches can't be planted (fixed: test sinnoh_berry picks and plants FLOAROMA's patch)
- [x] Followers cut in half in 3D (checked: a follower in ROUTE 201's tall grass and on its paths stands
      whole, every direction)
- [ ] Seams where the map ends and the out-of-bounds loop begins
- [x] Water animates (Platinum's animated map textures: test sinnoh_water)
- [x] POKéTCH a registrable key item outside SINNOH (on / off, the right stick as usual) once SINNOH's ELITE FOUR is
      beaten (MP_Region_DeviceHere; tests poketch_locked, poketch_regions)

## All added regions
- [~] Events: FireRed for KANTO, then HGSS for gaps
      (KANTO, map by map against pret/pokefirered: every POKéMON gift, static encounter and trainer matches; the
      gifts that differed: ROUTE 15's AIDE gives FireRed's EXP. SHARE (Quetzal: LUCKY EGG,
      tools/quetzal_port/fix_kanto_gifts.py), FIVE ISLAND's MEMORIAL PILLAR FireRed's TM42 (Quetzal: REAPER CLOTH,
      fix_kanto_tms.py). FireRed's own key items the game hasn't got stay out: FAME CHECKER, TEACHY TV, POWDER JAR,
      the TOWN MAP (the POKéNAV / POKéGEAR maps). JOHTO against pret/pokecrystal, to do: BILL's family gives Crystal's
      EEVEE (Quetzal: ROTOM) [x], MT. MORTAR's KARATE KING his TYROGUE [x] (fix_johto_gifts.py, test johto_gifts);
      GIOVANNI's quest (Quetzal's) never begins [x] (it was ELM's ARCEUS scene's way in; test story_giovanni); the
      RADIO TOWER kept as it is [x]: the DIRECTOR's RAINBOW WING is Gold's / HeartGold's (Crystal's CLEAR BELL would
      undo the TIN TOWER's events, which wait on the WING), and the lucky number's prizes are Emerald's own LOTTERY
      CORNER (Kai first); MR. POKéMON's EXP. SHARE for the RED SCALE [x] (Quetzal: DRAGON
      SCALE))
- [x] A non-HOENN gym leader beaten: the gym's unbattled trainers count as beaten (mp_coop.c MP_Coop_GymLeaderBeaten;
      test sinnoh_gym)
- [x] BADGES count every region's badges (the continue screen and the SAVE window: MP_Region_TotalBadges)
- [x] A4: TMs / HMs from leaders, trainers and events as the original games give them (new TMs / HMs where HOENN's
      list lacks them)
      ([x] KANTO's leaders: FireRed's TM39, 03, 34, 19, 06, 04, 38, 26 with FireRed's words, for Quetzal's held items
      (still obtainable elsewhere: item audit), tools/quetzal_port/fix_leader_tms.py, test kanto_leader_tm; [x] JOHTO's
      leaders: HeartGold / SoulSilver's TM51, 89, 45, 30, 01, 23, 07, 59 with HGSS's words (Crystal's TMs MUD-SLAP, FURY
      CUTTER... aren't TMs in this game; HGSS's all are); [x] KANTO's other TMs as FireRed's: its 28 TM balls (Quetzal's
      balls in the same places), the CERULEAN grunt's TM28, ROUTE 12's TM27, MR. PSYCHIC's TM29 and the CELADON roof's
      TM16 / 20 / 33 with FireRed's words: tools/quetzal_port/fix_kanto_tms.py, test kanto_other_tms; [x] JOHTO's other
      TM givers as Crystal's where the move is a TM here (HIDDEN POWER, SUNNY DAY, SANDSTORM, STEEL WING, ROAR, SLUDGE
      BOMB) with Crystal's words: tools/quetzal_port/fix_johto_tms.py, test johto_other_tms. Crystal's TM item balls
      stay Quetzal's items: JOHTO's maps are Quetzal's redraws and their balls don't stand where Crystal's did)
- [x] Prizes named by Quetzal's own numbers (a species / item put in a variable first kept Quetzal's number, another
      POKéMON or item here): CELADON's GAME CORNER prize room is FireRed's (POKéMON at FireRed's levels, TM13 / 23 / 24 /
      30 / 35, items, FireRed's prices and menus), GOLDENROD's prize clerk Crystal's (ABRA, CUBONE, WOBBUFFET); B leaves
      the menus; the vending machines sell drinks again (they sold MAX ETHERs); CINNABAR's chamber gives the RED / BLUE
      ORB (it gave TM02 / TM03): tools/quetzal_port/fix_prize_room.py, tests prize_room_kanto, prize_room_johto
- [x] SINNOH's time of day in KANTO, JOHTO and HOENN: MORNING / DAY / NIGHT encounters (grass slots 2-3 by Platinum's
      hours: mp_region.c MP_TimeLandSlot, src/data/wild_time.h from tools/quetzal_port/gen_wild_time.py; tests time_wild_*)
- [x] Encounter audit: no strong Pokémon early, places keep their identity (DIGLETT'S CAVE), no DITTO-like utility early;
      each region's own Pokémon mainly there, the other regions' Pokémon spread where they fit; never remove one already
      found somewhere; FILL never sends OFF GENERATION, OFF REGION very sparingly
      ([x] FILL: TEAM PADDING's POKéMON are the region's own (its wild tables and their evolutions), another region's only
      when none fits or one time in ten, never newer than the trainer's own team: mp_coop.c PickFillerSpecies; tests
      coop_fill_gen3, tri_padded. [x] DIGLETT'S CAVE is FireRed's (DIGLETT, DUGTRIO) at every GENERATION, no DITTO on
      KANTO's ROUTE 1 / 5: tools/quetzal_port/gen_wild.py OVERRIDES, applied by gen_gen2.py / gen_modern.py too; test
      kanto_digletts_cave. [x] Quetzal's DARKRAI dreams (JOHTO's bed, the S.S. ANNE's cabin) don't start: DARKRAI is
      SINNOH's; a save in one wakes; tests qz_dream*. [x] no strong POKéMON early: in a table up to level 20 a base stat
      total of 480+ in a common slot (4% or more) is the original game's POKéMON there (ILEX FOREST's PINSIR, SCYTHER,
      HERACROSS; ROCK TUNNEL B1F's KANGASKHAN, DRUDDIGON, DURALUDON, DRAMPA; MT. MOON B2F's SCYTHER at GEN 1-2; the
      RUINS OF ALPH's SIGILYPH; ROUTE 5's COMFEY), Quetzal's 1% rare finds kept; every POKéMON still obtainable at
      every GENERATION; test early_wild_strong)

## General
- [x] Shadows fade with screen transitions (gba_easy_draw.c ShadowKeepForFrame: the share of the backgrounds still shown)
- [x] No extra shadow under a follower's reflection (mp_followfx.c clears the copy's mpCastShadow; mp_shadow.c skips
      reflections; a shadow knows its owner, so two owners never share one: test follow_ground)
- [x] A follower behind the player drawn over the player when facing / moving down (raised above tall grass, it
      gives way to its Trainer: sprite->mpUnder, gba_easy_draw.c; test follow_behind)
- [x] OPTIONS: RETURN TO MAIN MENU (GAME tab, from the field: the save prompt, declined: sure?, then the save read again
      and the title screen; the PC build's soft reset quits, so it's main_menu.c MP_ReturnToTitleNow; test options_title)

## The user's additions (2026-11-02, later)
- [x] Legendaries: FireRed's NAVEL ROCK and BIRTH ISLAND static encounters and Emerald's NAVEL ROCK and FARAWAY ISLAND
      ones stay, whatever region the legendary is from (they were in the original games); exclusivity is for the others
      (qz_specials.c OriginalGameEvent; test legend_exclusive)
- [x] Platinum's remaining songs: the user's Platinum.zip (the BATTLE FACTORY / HALL / CASTLE / ARCADE, the VILLA's
      audio, the five SUPER CONTEST dances, the Frontier Brain's victory: tools/kaisong/sinnoh_songs.py; COOP_NOTES
      "SINNOH's music"; the opening, the Underground's and KUSAGASA's aren't in the rips)
- [x] Time of day looks like Platinum's in every region: its color tinting by the hour (Platinum's outdoor area light
      on flat ground, over noon's, per half hour: tools/sinnoh/build_daylight.py -> sinnoh/platinum_daylight.bin with the
      pack; src/mp/mp_daytint.c tints outdoor maps' layers and world sprites; test day_tint)

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
- KANTO's ladders already match FireRed's warps (every KANTO warp's place and destination checked against
  pokefirered's maps: only the POKéMON CENTER 2Fs' link rooms, Quetzal's own additions and the elevators differ), and
  FireRed itself leaves the player on the ladder; the user saw that as a bug, so an up ladder in a wall (a wall above,
  floor below) now steps the player off to the south. Ladders down (holes in the floor) stay as FireRed's.
- JOHTO's door animations need Quetzal's door frames, which live only in its ROM (tools/quetzal_rip didn't read the
  door table); left open until the rip can read them from the user's ROM.
- Legendaries: each one's home is the region whose games it belongs to (KANTO: the birds, MEWTWO, MEW; JOHTO: the
  beasts, LUGIA, HO-OH, CELEBI; HOENN: the Regis, the Latis, the weather trio, DEOXYS; SINNOH: Platinum's). Outside
  their home Quetzal's static encounters of them are hidden. The original games' own island events stay (the user,
  later the same day): FireRed's NAVEL ROCK and BIRTH ISLAND, Emerald's NAVEL ROCK and FARAWAY ISLAND. JIRACHI, MANAPHY and PHIONE have no encounter in their home region, so they keep
  their places (else they couldn't be caught at all). Gens 5-9 legendaries have no home region here and stay.
  (Overridden 2026-11-04, docs/REGIONS_AUDIT.md case 60, done in 1.111.0: Gen 5+ legendaries leave the overworld,
  MOEGATCHA only; JIRACHI, MANAPHY and PHIONE are HOENN's / SINNOH's and hidden here too; Quetzal's SUICUNE by the LAKE
  OF RAGE goes.)
- Time of day outside SINNOH: Platinum's mechanism (grass slots 2 and 3; morning 4:00-9:59, night 20:00-3:59). JOHTO's
  and KANTO's morning / night POKéMON are Crystal's (the ones its tables show then and not by day, heaviest slot first;
  79 maps; Crystal's DIGLETT'S CAVE, ROUTE 10 NORTH, 21, 26, TIN TOWER 2F, SILVER CAVE's item rooms and the WHIRL
  ISLANDS have no grass table here). At GENERATION 6+ JOHTO's and KANTO's modern pools take the same slots' morning /
  night POKéMON. HOENN has no original times: its own tables' POKéMON that Crystal shows only at night (ODDISH, MARILL...)
  fill slots 2-3 at night, only by day (SANDSHREW, SKARMORY...) in the morning: nothing added, nothing removed (22 maps).
  By day every table is as it was, so no POKéMON found before is lost.
