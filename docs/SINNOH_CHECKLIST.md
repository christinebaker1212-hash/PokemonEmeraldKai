# SINNOH: the working checklist

The list for measuring SINNOH's progress toward "1:1 with Platinum". `[x]` = done and checked by a test (named after
it), `[~]` = converted or written but not yet played through by a test, `[ ]` = to do. Closed items are at the end.
Rules behind it: CLAUDE.md's SINNOH paragraph and docs/SINNOH_HANDOFF.md.

## Numbers to watch

| What | How to see it | Now |
|---|---|---|
| Platinum's script entries that convert | `python3 -I tools/sinnoh/gen_scripts.py --report ~/.cache/moemon/pokeplatinum` (first line) | 2180 of 2511 |
| Commands still missing | the same report, the list under it (most used first) | 60 kinds |
| Platinum's text slots and page breaks | messages show whole (\\r is a new box, six string slots) | done |
| Platinum songs the game lacks | the same report, "songs the game lacks" | 5 |
| SINNOH tests passing | `tools/test/regress.sh` with the `sinnoh_*`, `region_*` scenarios | all pass |

## 1. Platinum's story, in order

Each segment: Platinum's own map scripts, people, triggers, words and battles, played by a test.

- [x] TWINLEAF: the TV, BARRY, mom (sinnoh_platinum_opening)
- [x] ROUTE 201 and LAKE VERITY: ROWAN, the briefcase, the starter, BARRY's first battle, the Running Shoes
      (sinnoh_platinum_route201, sinnoh_platinum_walk)
- [x] SANDGEM: the lab, the POKéDEX, the walk with DAWN / LUCAS; ROUTE 202's catching tutorial (sinnoh_platinum_tutorial)
- [x] JUBILIFE: the POKéTCH, the tag battle with DAWN / LUCAS against GALACTIC (sinnoh_platinum_tag)
- [x] OREBURGH: ROARK's gym in Platinum's words (sinnoh_platinum_gym)
- [~] FLOAROMA, the VALLEY WINDWORKS (MARS), ROUTE 205
- [x] ETERNA FOREST with CHERYL: partner wild battles, her own back picture (sinnoh_platinum_partner)
- [~] ETERNA: the GALACTIC building, GARDENIA (her gym's clock puzzle: section 3)
- [~] CYCLING ROAD, HEARTHOME: FANTINA, AMITY SQUARE, the CONTEST HALL (section 5)
- [~] SOLACEON, VEILSTONE: MAYLENE, the GALACTIC warehouse with DAWN / LUCAS
- [~] PASTORIA: CRASHER WAKE, the GREAT MARSH tram (MoveGreatMarshTram)
- [~] CELESTIC, CANALAVE: BYRON, IRON ISLAND with RILEY (partner)
- [~] The three lakes, SNOWPOINT: CANDICE, LAKE ACUITY
- [~] GALACTIC HQ, MT. CORONET, SPEAR PILLAR (the tag battle with BARRY)
- [ ] The DISTORTION WORLD, 1:1 (section 3)
- [~] SUNYSHORE: VOLKNER (the gym's buttons: PressSunyshoreGymButton), VICTORY ROAD
- [~] The POKéMON LEAGUE: the ELITE FOUR, CYNTHIA, the platform lift (TriggerPlatformLift), the HALL OF FAME in
      Platinum's own scripts (now the written CHAMPION's)
- [ ] After the HALL OF FAME: the FIGHT AREA, STARK MOUNTAIN with BUCK, the SURVIVAL / RESORT AREAs, the VILLA, the
      NATIONAL DEX, the SNOWPOINT temple
- [ ] One test from TWINLEAF to the HALL OF FAME

## 2. Kai's systems in SINNOH (kept, Platinum's commands mapped onto them)

- [x] Partner battles (StartTagBattle -> Kai's partner battle) (sinnoh_platinum_tag)
- [x] Two-trainer battles (MP_StartTwoTrainerBattle)
- [x] Partner wild battles (sinnoh_platinum_partner)
- [x] The early rival battle, the catching tutorial on Emerald's WALLY tutorial
- [x] Walking partners with Kai's follower behind them
- [x] The camera object (AddFreeCamera), the starter briefcase, the POKéTCH as Kai's item (sinnoh_poketch)
- [x] Marts, nurses, gyms and their written puzzles, the frontier (sinnoh_marts, sinnoh_heal, sinnoh_gym_*,
      sinnoh_tower, sinnoh_factory, sinnoh_hall, sinnoh_castle, sinnoh_arcade)
- [x] Platinum's "choose a POKéMON" (SelectMoveTutorPokemon / GetSelectedPartySlot) on the game's party menu; its save
      prompt (Common_SaveGame) the game's; scripts go on after a warp (SINNOH_WARP_CONTINUE)
- [ ] Platinum's commands still unmapped onto Kai's systems ([x] PokeMart*WithGreeting and Common_CallPokecenterNurse:
      the game's shop and nurse, Platinum's greeting; 2346 of 2511 entries convert), PlayHMCutIn,
      EndSafariGame ([x] the party, BAG, ribbon and trade commands, the move deleter, fossils, the PRIZE EXCHANGE, the
      coin clerk, the HONEY seller, the SHARD tutors: s3d_party.c, COOP_NOTES "Platinum's party, BAG and trade commands"; [x] StartRankingsMachine: JUBILIFE TV's rankings, s3d_rankings.c; [x] the BATTLE TOWER: CallBattleTowerFunction, its frontier scenes, the BATTLE SALON, the STATS
      JUDGE: docs/SINNOH_TOWER_PORT.md)

## 3. Built for 1:1 (Platinum has it, Kai doesn't)

- [ ] The DISTORTION WORLD: walking on walls and ceilings, the turning rooms, boulders, the lift, CYRUS and
      GIRATINA (overlay009, SetPartyGiratinaForm, StartGiratinaOriginBattle)
- [x] ETERNA's gym clock (AdvanceEternaGymClock on s3d_gyms.c's Sinnoh_EternaClock) (sinnoh_gym_eterna)
- [x] PASTORIA's and SUNYSHORE's gym buttons in Platinum's commands (PressPastoriaGymButton / PressSunyshoreGymButton on
      s3d_gyms.c's Sinnoh_PastoriaWater / Sinnoh_SunyshoreTurn) (sinnoh_gym_pastoria, sinnoh_gym_sunyshore)
- [x] HEARTHOME's and VEILSTONE's gyms in Platinum's commands (1.113.0: InitPersistedMapFeaturesForVeilstoneGym /
      ForHearthomeGym run as their maps load, s3d_gyms.c; so the gyms' map scripts are Platinum's own too)
      (sinnoh_gym_veilstone, sinnoh_gym_hearthome)
- [x] Platform lifts (TriggerPlatformLift: IRON ISLAND's three, the LEAGUE's six; src/sinnoh/s3d_lift.c) (sinnoh_lift,
      sinnoh_lift_iron)
- [x] The elevators in Platinum's own scripts: the attendants' words, GetFloorsAbove, the "Current Floor" window,
      SetSpecialLocation, PlayElevatorAnimation; the VISTA LIGHTHOUSE's ride by itself (sinnoh_elevator)
- [x] BERRY soil patches: Platinum's 118, its first BERRIES ripe, the game's own BERRY trees (128+ in the region
      sidecar: src/mp/mp_berrytrees.c) (sinnoh_berry); the TV's BERRY LOOKOUT reads them (sinnoh_studio: mp tvnow)
- [x] The roamers: MESPRIT, CRESSELIA and PROF. OAK's birds in Platinum's own scripts (ActivateRoamingPokemon,
      DrawPokemonPreview, FlickerObject), their moves, encounters, HP kept, the HALL OF FAME's reset, the TV's ROAMER
      NEWS FLASH (src/sinnoh/s3d_roamer.c) (sinnoh_roamer)
- [x] The GREAT MARSH tram (MoveGreatMarshTram, CheckGreatMarshTramLocation: src/sinnoh/s3d_tram.c) (sinnoh_tram)
- [x] The REGI ruins in Platinum's own scripts (ActivateRegiRuinsDot, the statue's quake ScrCmd_29F, FLAG_GAME_COMPLETED;
      REGIGIGAS's check: any REGIGIGAS, Platinum's being a DS event's) (sinnoh_regi); the OLD CHATEAU's ROTOM (sinnoh_rotom)
- [x] ROTOM's room in Platinum's own scripts: as Platinum's without the ROTOM distribution (a DS event, closed): no
      appliances, the notebooks and PROF. ROWAN (sinnoh_rotom_room)
- [x] UNOWN messages in Platinum's UNOWN font (MessageUnown), the letters seen (GetUnownFormsSeenCount) (sinnoh_unown)
- [x] The VILLA (RESORT AREA) and its furniture: all 44 of Platinum's scripts (the order form, the visitors, each
      piece's own); what's bought stands as Platinum's model (s3d_gyms.c VILLA, its table generated from Platinum's
      villa_furniture.c into src/data/sinnoh_gyms.h, the models in the 3D pack: gymprops.py), blocks its tiles, runs
      its script on A; the order form's requirements on the game's records (s3d_misc.c 20). The music box's song
      (SEQ_PL_TOWN02) isn't in the game yet: the music plays on. Test sinnoh_villa
- [x] The Frontier Brains' and the HALL MATRON's teams (1.114.0: ARGENTA, DARACH, DAHLIA at Platinum's battles with Platinum's sets and words; the FACTORY is Kai's own, with Kai's FACTORY HEAD; PALMER since the TOWER)
- [x] The size contest (ROUTE 222's house, REMORAID; not PASTORIA's): Platinum's script, s3d_misc.c 6-9 on Emerald's size
      code with Platinum's rounding and starting record; test sinnoh_size_contest
- [x] JUBILIFE's TV lottery in Platinum's own script, on the game's own lottery number (s3d_misc.c) (sinnoh_lottery)
- [x] The JOURNAL (1.113.0): Platinum's journal.c rules and journal_printer.c lines in its own words, its notebook
      (f_note_gra.narc, build_journal.py), ten pages, a page a day the game is loaded; places (first visit to a town, out
      of a cave / home / the lab / a named building, a GYM too tough), wild POKéMON, gym leaders, the Elite Four, the
      Champion, trainers, shopping, the PC Box, field moves, item balls, the Underground, the BATTLE FRONTIER, friends'
      battles; opened from the BAG or on CONTINUE after two days (s3d_journal.c; COOP_NOTES "SINNOH's JOURNAL"; test
      sinnoh_journal)
- [x] AMITY SQUARE: Platinum's own scripts with the player's follower as the square's walker (every POKéMON follows in
      this game, so the gates no longer check the party or hand one over: gen_scripts.py KAI_REWRITES); its finds every
      200 steps by Platinum's pools and odds (BERRIES, accessories), its lines, the gift man's BERRY or accessory a
      day (s3d_misc.c 10-16, mp_life_interact.c); test sinnoh_amity
- [x] The VEILSTONE DEPARTMENT STORE's regular (CheckIsDepartmentStoreRegular: 5 buys there, s3d_misc.c)
- [x] Game records (AddToGameRecord: kept by nothing, Platinum showing them only in the rankings rooms); the rankings
      machines (StartRankingsMachine) closed: group / Wi-Fi rankings
- [x] DAWN's and LUCAS's own overworld sprites (Platinum's player_f / player_m sheets, appended to the pack after the
      others: OBJ_EVENT_GFX_SINNOH_PLAYER_F / _M)
- [x] The Underground, for its digging and SPIRITOMB only (the user, 2026-11-02; its wireless sides are CO-OP's):
      the EXPLORER KIT down and up (Platinum's cell mapping), the day's dig spots, Platinum's digging screen and rules
      (s3d_mining.c, art "MINE"), treasures to the BAG, the UNDERGROUND MAN's and ROARK's own scripts, SPIRITOMB's
      counter from co-op friends talked to there, ROUTE 208's ODD KEYSTONE and ROUTE 209's tower in Platinum's scripts,
      the UNDERGROUND TREASURES CORNER on TV (sinnoh_underground, sinnoh_underground_man, sinnoh_spiritomb,
      sinnoh_spiritomb_talk)
- [ ] The Underground's own song (SEQ_TANKOU; OREBURGH MINE's stands in)
- [~] Platinum's songs the game lacks: [x] SEQ_PL_AUDIO (the VILLA's audio system: its bossa nova, mus_sinnoh_villa_audio)
      and SEQ_PL_PIANO (its piano: the music box, mus_sinnoh_villa) from the user's Platinum rip (2026-11-02, which also
      gave the Frontier halls and the SUPER CONTEST category dances); [ ] SEQ_OPENING, SEQ_KUSAGASA (the POKé RADAR
      demo), SEQ_TANKOU: not in the rips the user sent (tracks 1-19 of Platinum's and the Underground's); SEQ_SILENCE_FIELD
      is PAL PARK's (closed)
- [x] Platinum's POKéDEX ratings (Load*DexRating)
- [x] Platinum's key items (items 662-684)

## 4. Region travel at parity with JOHTO and KANTO

- [x] CANALAVE's ferry sailor shown only once SINNOH's ELITE FOUR is beaten and the S.S. TICKET is in the bag, and on the
      first boat in (sinnoh_ferry)
- [x] SINNOH's S.S. TICKET from the CHAMPION's win (a game begun in SINNOH has no other way to get one) (sinnoh_ferry)
- [x] Customs before boarding: one Pokémon comes, the rest go to the GLOBAL PC (EventScript_RegionCustoms)
- [x] The GLOBAL PC: open in SINNOH only once its ELITE FOUR is beaten (MP_GlobalPcLinked)
- [x] A first boat in starts Platinum's story (region_trip_hoenn_sinnoh)
- [x] A test: SINNOH's sailor hidden before the LEAGUE, there after, the boat's list (sinnoh_ferry); to HOENN and back
      (region_trip_hoenn_sinnoh)

## 5. Super Contests

- [x] The FASHION CASE: Platinum's 100 accessories and 18 backdrops kept, given, checked, named in its words
      (src/sinnoh/s3d_fashion.c; AMITY SQUARE's and the hall's gifts convert)
- [x] POFFINS as POKéBLOCKS (the POFFIN case checks, GivePoffin)
- [x] The contest's art in the graphics pack: accessories, backdrops, the stages (contest_bg)
- [x] The CONTEST HALL's lobby and stages, the POFFIN HOUSE as maps again (numbers of every other map kept)
- [x] The VISUAL round: dress-up (accessories placed on the POKéMON over a backdrop, Platinum's time and theme),
      then its judging (sinnoh_contest)
- [x] The DANCE round (following the lead dancer's steps to the music) (sinnoh_contest)
- [x] The ACTING round (appeals to the three judges, voltage, the 23 effects, the computer's table) (sinnoh_contest)
- [x] Results, ranks (NORMAL to MASTER), ribbons (the game's), the first-win accessories, the winners' photos
      (ContestPhotoHasData, the lobby's frames), the hall's receptionists, practice
- [x] JUBILIFE TV's dress-up and its photos (ScrCmd_0A6, ShowDressUpPhoto, DressUpPhotoHasData) (sinnoh_studio)
- [x] The POFFIN HOUSE's cooking as the game's BERRY BLENDER (CheckCanCookPoffin, OpenPoffinCooking)
- [x] Platinum's stage art (1.113.0: contest_bg.narc's VISUAL stage and audience, DANCE floor, ACTING stage and results
      bars behind the rounds: build_contest.py stages() "CSTG", s3d_contest_ui.c DrawStagePicture) (sinnoh_contest)
- [x] Contests with friends: the link desk's BECOME LEADER / JOIN GROUP gather friends in the room; every game runs
      the same contest, each one's dress-up, steps and moves over the relay (sinnoh_contest_link: COOP_NOTES "SUPER
      CONTESTS with friends")
- [x] POKé BALL effects (Kai's mp_ballfx.c) wherever Platinum has Seals: each Seal stands for an earned ball effect
      piece; the SUNYSHORE MARKET's Seal shop sells the day's at Platinum's prices (sinnoh_seals), SOLACEON's gifts
      unlock them, the PC's BALL CAPSULES are the summary's POKé BALL page, the TV's SEAL CLUB SHOW tells of a POKéMON
      given an effect in SINNOH
- [ ] Tests for each

## 6. Platinum's TV

- [x] The TV in every house and CENTER plays Platinum's broadcast: its weekly schedule, greetings, segments, farewells,
      commercials (sinnoh_tv)
- [x] TRAINER SIGHTINGS from what the player does in SINNOH: catches, fishing, shopping, eggs, the GREAT MARSH, hidden
      items, the name rater, the PC bulletin, BERRY crops, POKé BALL effects in the SEAL CLUB SHOW's place (sinnoh_tv: a catch)
- [x] Each of the nine programs watched to its end (sinnoh_tv_programs: mp tvprogram N); SINNOH NOW, RECORDS and the
      VARIETY HOUR play Platinum's segments; the reporters' interviews (one word on the game's own easy chat)
- [x] The interviews and records from SINNOH's own systems: contests, the dress-up photo (PHOTO RATING), POFFINS (the
      POFFIN HOUSE's blends), the BATTLE TOWER's records and interviews, the HALL, BP, ribbons, the GAME CORNER, BERRY
      patches (BERRY LOOKOUT), AMITY SQUARE, the VILLA (sinnoh_studio, sinnoh_tower: COOP_NOTES "SINNOH's TV")
- [x] Friends' segments on each other's TV: CO-OP's record mixing carries each game's kept segments; the TV keeps 8
      of friends' and airs them with the friend's name (sinnoh_tv_mix: COOP_NOTES "SINNOH's TV")
- [x] JUBILIFE TV's floors and their people: every script converts; the 3F questionnaire man's two words on the
      game's own easy chat (EASY_CHAT_TYPE_MP_TWO_WORDS, its QUESTIONNAIRE screen); his MYSTERY GIFT phrase is the
      game's own (LINK TOGETHER WITH ALL); no phrase unlocks Platinum's extra PC wallpapers (the game's PC has its
      own) (sinnoh_jubilife_tv)

## 7. Battles look right

- [x] Partners' back pictures in tag, partner and double battles: Platinum's own backs for CHERYL, MIRA, RILEY,
      BUCK, MARLEY and BARRY (sinnoh/platinum_gfx.pak), Kai's LOOKs' for DAWN and LUCAS, in Kai's partner back slot
      (sinnoh_platinum_partner, sinnoh_platinum_tag)
- [x] The catching tutorial's DAWN / LUCAS stay themselves after the BAG (sinnoh_platinum_tutorial)
- [x] Trainers' battle pictures and overworld sprites from Platinum (sinnoh_pics)

## 8. Clean-up

- [x] No written story left: every game in SINNOH plays Platinum's (no old saves to keep, no migration)
- [ ] The written words go once nothing uses them (words*.py, story_people.py, trainer_words.py)
- [x] Tests that used the written story moved to Platinum's (or gone where a Platinum test covers them)

## Closed (not built)

- Online-only: the GTS, the Wi-Fi Club and Plaza, global rankings, distribution events
- The DS's own: the PAL PARK and its GBA slot, the second screen (the POKéTCH is Kai's)
- Superseded by Kai: Ball Seals (POKé BALL effects take their place), Kai's TRAINER CARD
- The Underground's wireless sides (the user, 2026-11-02: CO-OP supersedes them): secret bases, spheres and their
  trades, traps, goods, Capture the Flag; its vendors aren't there, the UNDERGROUND MAN's missions past digging can't
  be finished
