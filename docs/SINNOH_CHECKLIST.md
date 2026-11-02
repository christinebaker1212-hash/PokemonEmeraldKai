# SINNOH: the working checklist

The list for measuring SINNOH's progress toward "1:1 with Platinum". `[x]` = done and checked by a test (named after
it), `[~]` = converted or written but not yet played through by a test, `[ ]` = to do. Closed items are at the end.
Rules behind it: CLAUDE.md's SINNOH paragraph and docs/SINNOH_HANDOFF.md.

## Numbers to watch

| What | How to see it | Now |
|---|---|---|
| Platinum's script entries that convert | `python3 -I tools/sinnoh/gen_scripts.py --report ~/.cache/moemon/pokeplatinum` (first line) | 2032 of 2511 |
| Commands still missing | the same report, the list under it (most used first) | 60 kinds |
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
- [x] ETERNA FOREST with CHERYL: partner wild battles, her back picture still a stand-in (sinnoh_platinum_partner)
- [~] ETERNA: the GALACTIC building, GARDENIA (her gym's clock puzzle: section 3)
- [~] CYCLING ROAD, HEARTHOME: FANTINA, AMITY SQUARE, the CONTEST HALL (section 5)
- [~] SOLACEON, VEILSTONE: MAYLENE, the GALACTIC warehouse with DAWN / LUCAS
- [~] PASTORIA: CRASHER WAKE, the GREAT MARSH tram (MoveGreatMarshTram), the size contest
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
- [ ] Platinum's commands still unmapped onto Kai's systems: PokeMart*WithGreeting, Common_CallPokecenterNurse,
      SelectMoveTutorPokemon / ShowMoveTutorMoveSelectionMenu, PlayHMCutIn, SelectPokemonToTrade,
      CallBattleTowerFunction / the frontier lobbies, EndSafariGame

## 3. Built for 1:1 (Platinum has it, Kai doesn't)

- [ ] The DISTORTION WORLD: walking on walls and ceilings, the turning rooms, boulders, the lift, CYRUS and
      GIRATINA (overlay009, SetPartyGiratinaForm, StartGiratinaOriginBattle)
- [ ] ETERNA's gym clock (AdvanceEternaGymClock)
- [ ] PASTORIA's and SUNYSHORE's gym buttons in Platinum's commands (PressPastoriaGymButton / PressSunyshoreGymButton)
- [ ] HEARTHOME's and VEILSTONE's gyms in Platinum's commands
- [ ] Platform lifts (TriggerPlatformLift), GetFloorsAbove
- [ ] The GREAT MARSH tram (MoveGreatMarshTram, CheckGreatMarshTramLocation)
- [ ] The REGI ruins' dots (ActivateRegiRuinsDot), REGIGIGAS's check
- [ ] ROTOM's room (GetPartyRotomCountAndFirst)
- [ ] UNOWN messages (MessageUnown, GetUnownFormsSeenCount)
- [ ] The VILLA (Resort Area) and its furniture
- [ ] The Frontier Brains' and the HALL MATRON's teams
- [ ] The PASTORIA size contest
- [ ] JUBILIFE's TV lottery
- [ ] The JOURNAL
- [ ] AMITY SQUARE's walk (ClearAmitySquareStepCount)
- [ ] The VEILSTONE DEPARTMENT STORE's regular (CheckIsDepartmentStoreRegular)
- [ ] Game records (AddToGameRecord), the rankings machines (StartRankingsMachine)
- [ ] DAWN's and LUCAS's own overworld sprites (stand-ins now)
- [ ] Platinum's songs the game lacks: SEQ_OPENING, SEQ_KUSAGASA, SEQ_PL_AUDIO, SEQ_PL_PIANO, SEQ_SILENCE_FIELD
- [x] Platinum's POKéDEX ratings (Load*DexRating)
- [x] Platinum's key items (items 662-684)

## 4. Region travel at parity with JOHTO and KANTO

- [ ] CANALAVE's ferry sailor shown only once SINNOH's ELITE FOUR is beaten and the S.S. TICKET is in the bag
- [ ] SINNOH's S.S. TICKET after its HALL OF FAME (a game begun in SINNOH has no other way to get one)
- [x] Customs before boarding: one Pokémon comes, the rest go to the GLOBAL PC (EventScript_RegionCustoms)
- [x] The GLOBAL PC: open in SINNOH only once its ELITE FOUR is beaten (MP_GlobalPcLinked)
- [x] A first boat in starts Platinum's story (region_trip_hoenn_sinnoh)
- [ ] A test: SINNOH's sailor hidden before the LEAGUE, there after, the boat to HOENN and back

## 5. Super Contests

- [ ] Platinum's Super Contests: the dress-up round, the dance round, the acting round, ranks and HALLs
      (OpenPartyMenuForContest, the CONTEST HALL's scripts)
- [ ] Accessories and backdrops (AddAccessory, BufferAccessoryName, BufferContestBackdropName), the photos
      (ContestPhotoHasData, DressUpPhotoHasData)
- [ ] POKé BALL effects (Kai's mp_ballfx.c) wherever Platinum has Seals: the story's gifts, the menus, the shops
- [ ] Tests for each

## 6. Platinum's TV

- [ ] The TV in every house and CENTER plays Platinum's shows
- [ ] The interviewers (CheckTVInterviewEligible) and the shows they make
- [ ] JUBILIFE TV's floors and their people
- [ ] Tests

## 7. Battles look right

- [ ] Partners' back pictures in tag, partner and double battles: Platinum's own backs for CHERYL, MIRA, RILEY,
      BUCK, MARLEY, BARRY, DAWN and LUCAS, through Kai's partner back slot (96x64; five-frame backs keep frames
      0, 1, 3, 4)
- [x] Trainers' battle pictures and overworld sprites from Platinum (sinnoh_pics)

## 8. Clean-up

- [ ] No written story left: every new game in SINNOH plays Platinum's (no old saves to keep, no migration)
- [ ] The written words go once nothing uses them (words*.py, story_people.py, trainer_words.py)
- [ ] Tests that used the written story moved to Platinum's

## Closed (not built)

- Online-only: the GTS, the Wi-Fi Club and Plaza, global rankings, distribution events
- The DS's own: the PAL PARK and its GBA slot, the second screen (the POKéTCH is Kai's)
- Superseded by Kai: Ball Seals (POKé BALL effects take their place), Kai's TRAINER CARD
- Still undecided by the user: the Underground
