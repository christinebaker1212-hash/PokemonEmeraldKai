# K1 findings: KANTO (Pallet, Viridian, Pewter, Cerulean, Routes 1-6, 9, 10, 22, 24, 25, Viridian Forest, Mt. Moon, Diglett's Cave, Rock Tunnel, Power Plant, Cerulean Cave, Underground Paths)

Method: every K1 map's dialogue was diffed against pokefirered (map text.inc / scripts.inc, plus FireRed's
data/text/trainers.inc rematch lines read from git); object, sign, hidden item, trigger and warp positions, item balls,
map header flags, gifts, trades, marts and every `special` / shared `Quetzal_*` call were compared, then the differing
scripts were read by hand. Paths: "FR" = /tmp/claude-0/frlg/pokefirered, "now" = /home/user/PokemonEmeraldKai.
Things already marked [x] in docs/REGIONS_TODO.md (leader TMs, HM05, TM28, the RUNNING SHOES aide, the tutors, the
legendary exclusivity...) were checked and are not repeated: they're in place.

Already fine and not reported: every item ball (but one, K1-17) and every hidden item is FireRed's; the Mt. Moon
fossils, Old Amber, Nugget Bridge, Bill's S.S. TICKET, Route 1 Potion, Route 10 aide's EVERSTONE, the Magikarp
salesman, Mewtwo (Lv 70, "Mew!"), the Power Plant Electrodes, the Route 5 / 6 gate guards' TEA, the badge guy's list,
the Route 22 badge gate, gym trainers marked beaten after each leader (Quetzal_EventScript_8456FD2's trainer ids match
each gym), all trainers' intro / defeat / after-battle and rematch lines.

---

### K1-1: FireRed's trainer rematches can never happen (no VS SEEKER; rematch branches unreachable)
- Maps: Kanto_Route3, Kanto_Route4, Kanto_Route6, Kanto_Route9, Kanto_Route10, Kanto_Route24, Kanto_Route25 (45 trainers)
- Category: A (and D)
- Original: FireRed's VS SEEKER rematches: `specialvar VAR_RESULT, ShouldTryRematchBattle` then `trainerbattle_rematch`
  with the RematchIntro lines (FR data/scripts/trainers.inc:1-11 for Route 3's BEN, and every route trainer after
  it; the lines in FR data/text/trainers.inc:16-705).
- Now: the scripts are there with FireRed's rematch lines (e.g. data/maps/Kanto_Route3/scripts.inc:16-24), but
  `ShouldTryRematchBattle` (src/battle_setup.c:2221) only looks in Emerald's gRematchTable, which holds no
  TRAINER_QZ_* id, so it always returns FALSE: every rematch line and second team is dead. There is no VS SEEKER in
  KANTO (item 362 exists in the item table only).
- Restore: give FireRed's VS SEEKER (the VERMILION POKéMON CENTER lady, outside K1) and a KANTO rematch table for
  the QZ trainers that `ShouldTryRematchBattle` / `IsTrainerReadyForRematch` read (FireRed's rules: 5 badges...,
  the trainer's rematch party), or route KANTO rematches through Kai's MOENAV / match-call rematch logic.
- Notes: REGIONS_TODO's "FireRed's own key items ... stay out" lists FAME CHECKER, TEACHY TV, POWDER JAR, TOWN MAP,
  not the VS SEEKER: a decision is needed. Needs the rematch parties (Quetzal's trainers.json may have them). High
  confidence that the branches are unreachable.

### K1-2: ROCK TUNNEL isn't dark
- Maps: Kanto_RockTunnel_1F, Kanto_RockTunnel_B1F
- Category: D
- Original: FR data/maps/RockTunnel_1F/map.json:7 and RockTunnel_B1F `"requires_flash": true`: pitch dark until FLASH.
- Now: data/maps/Kanto_RockTunnel_1F/map.json and _B1F/map.json `"requires_flash": false` (Quetzal's rip had it
  off; no KANTO or JOHTO map requires FLASH at all). HM05 is given again (Route 2 aide, "FLASH lights up even the
  darkest of caves") and the BOULDERBADGE text promises FLASH, but there's nothing to light.
- Restore: `"requires_flash": true` in both map.json files (Kai's FLASH with the region's badge then works as in
  HOENN's GRANITE CAVE). Also restore the aide's partner line (K1-14).
- Notes: JOHTO's DARK CAVE probably has the same problem (J area). High confidence.

### K1-3: Jessie & James's reward is a POKéMON with species 0 ("received Pikachu!" but nothing arrives)
- Maps: Kanto_PalletTown, Kanto_ViridianCity, Kanto_PewterCity, Kanto_CeruleanCity (objects at (19-21,7-8),
  (39-41,3-4), (7-9,24-25), (32-33,11-12)); shared data/scripts/qz/kanto_shared.inc:156-202
- Category: A (and C)
- Original: nothing; FireRed has no such event.
- Now: after the LEAGUE, `QzKantoTravelingEventCheck` (src/qz_specials.c:1044-1066) puts the four objects in one town
  a day; the battle (TRAINER_QZ_913) is followed by `copyvar VAR_0x8000, VAR_QZ_0x514A` / `qz_givemon_ex VAR_0x8000,
  50` (kanto_shared.inc:172-173). Nothing in the game ever sets VAR_QZ_0x514A (only `setvar ... 0` at :190), so the
  gift is SPECIES_NONE: ScriptGiveMon (src/script_pokemon_util.c:63) makes an empty POKéMON while
  Quetzal_EventScript_83C2D35 prints "{PLAYER} received Pikachu!".
- Restore: remove the event (it's a Quetzal fan addition: hide the objects for good, drop the ON_LOAD / ON_RESUME
  `Quetzal_EventScript_83C3036` calls). If kept, set the species (Quetzal's Pikachu line) before the givemon.
- Notes: the JOHTO copy (QzJohtoTravelingEventCheck, flag 0x1490) should be checked the same way (J area). Not in
  REGIONS_TODO. High confidence.

### K1-4: ARCEUS still visits the PALLET bedroom although every ARCEUS trial in KANTO is hidden
- Maps: Kanto_PalletTown_PlayersHouse_2F (bed bg events (2,5), (2,6); object 1 at (5,7))
- Category: A (and C)
- Original: FireRed's 2F has the NES, PC and notice only; no bed script.
- Now: data/maps/Kanto_PalletTown_PlayersHouse_2F/scripts.inc:39-70: "Would you like to sleep?" heals, and after the
  LEAGUE with six Lv 100 POKéMON `QzPalletPostgameEventEligibility` (src/qz_specials.c:1616) shows ARCEUS ("Your
  journey is not yet over. We shall meet again.") and sets FLAG_QZ_0x2E13, the key to the 17 ARCEUS trials. But
  ARCEUS is SINNOH's (sLegendHome, src/qz_specials.c:453), so HiddenForeignLegend hides every KANTO trial: the promise
  leads nowhere. The eligibility check doesn't call MP_LegendForeign.
- Restore: remove the ARCEUS visit (and, for vanilla, the sleep-to-heal bed: a Quetzal addition) - or at least make
  QzPalletPostgameEventEligibility return FALSE when MP_LegendForeign(SPECIES_ARCEUS, REGION_KANTO).
- Notes: the trial counter at CINNABAR's lab is in another area. High confidence.

### K1-5: Overworld legendaries / mythicals Quetzal placed in K1 maps (Gen 5-9 ones still appear after the LEAGUE)
- Maps: Kanto_CeruleanCave_1F (ETERNATUS 24,20), Kanto_CeruleanCity (TAPU FINI 5,16), Kanto_DiglettsCave_B1F
  (ZYGARDE 43,40), Kanto_MtMoon_B2F (NECROZMA 8,5), Kanto_PalletTown_ProfessorOaksLab (MELTAN 3,5),
  Kanto_PewterCity_Museum_1F (HOOPA 9,4), Kanto_PewterCity_Museum_2F (JIRACHI 7,4), Kanto_PowerPlant (XURKITREE
  42,25), Kanto_RockTunnel_B1F (TERRAKION 42,18), Kanto_Route10 (YVELTAL 18,71; ZERAORA 15,14), Kanto_Route25
  (ENAMORUS 46,7), Kanto_Route25_SeaCottage (POIPOLE 11,8), Kanto_ViridianForest (ZACIAN 32,6; ZAMAZENTA 37,6).
  Hidden for good as other regions' (decision in place): DIALGA, PALKIA (Mt. Moon 1F), SHAYMIN (Pewter), RAIKOU
  (Route 10), SUICUNE (Route 25), AZELF (Route 22), MESPRIT (Route 6), CRESSELIA (Route 4), RAYQUAZA (K1-6), the
  ARCEUS trials (Routes 2, 5, 6, 9, 22).
- Category: C
- Original: none of these exist in FireRed.
- Now: objects with Quetzal_EventScript_840FBBD (kanto_shared.inc:344); RefreshStatics shows them after KANTO's
  LEAGUE when GENERATION allows.
- Restore: keep per docs/REGIONS_TODO.md Decisions ("Gens 5-9 legendaries have no home region here and stay";
  JIRACHI keeps its place). Listed so the user can confirm, since the request asks for a vanilla experience: for a
  strict FireRed KANTO they would all be hidden (set their object flags permanently).
- Notes: decision already made; low priority unless the user changes it.

### K1-6: CERULEAN CAVE's added RAYQUAZA chamber is now an empty room
- Maps: Kanto_CeruleanCave_2F (warp (4,3)), Kanto_CeruleanCave_RayquazaChamber
- Category: C
- Original: FireRed's CERULEAN CAVE 2F has six warps; no such room.
- Now: data/maps/Kanto_CeruleanCave_2F/map.json has a 7th warp to MAP_KANTO_CERULEAN_CAVE_RAYQUAZA_CHAMBER, whose only
  object, RAYQUAZA, is always hidden (HOENN's legendary). A dead-end Quetzal room.
- Restore: remove the warp (and the map), or leave as harmless scenery.
- Notes: high confidence.

### K1-7: Quetzal's empty "MT. MOON ROUTE 4 cave"
- Maps: Kanto_Route4 (warp (58,2)), Kanto_MtMoon_Route4Cave
- Category: C
- Original: FireRed's ROUTE 4 has three warps; the cave doesn't exist.
- Now: data/maps/Kanto_Route4/map.json 4th warp -> MAP_KANTO_MT_MOON_ROUTE4_CAVE: an empty 21x20 cave, no objects,
  no wild table, one setmetatile (data/maps/Kanto_MtMoon_Route4Cave/scripts.inc).
- Restore: remove the warp and its door metatile (FireRed's ROUTE 4 has a plain cliff there), or keep as harmless.
- Notes: docs/quetzal/KANTO.md calls it unfinished. Medium confidence that the door tile should go (layout change).

### K1-8: Marts sell Quetzal's badge-count lists, not FireRed's town lists
- Maps: Kanto_ViridianCity_Mart, Kanto_PewterCity_Mart, Kanto_CeruleanCity_Mart
- Category: C
- Original: per-town lists: VIRIDIAN POKé BALL, POTION, ANTIDOTE, PARLYZ HEAL (FR data/maps/ViridianCity_Mart/scripts.inc:59-70);
  PEWTER + AWAKENING, BURN HEAL, ESCAPE ROPE, REPEL; CERULEAN + SUPER POTION.
- Now: `pokemart 0x0` (e.g. data/maps/Kanto_ViridianCity_Mart/scripts.inc:50) = src/data/qz_badge_marts.h by badge
  count: PREMIER / HEAL / LUXURY / DUSK / NEST / QUICK / TIMER BALLs, POKé DOLL from the start, MASTER BALL and FULL
  RESTORE at 8 badges.
- Restore: FireRed's own list per mart (each map's own `pokemart` table, as in pokefirered); the badge marts are a
  Quetzal system (COOP_NOTES "Badge marts").
- Notes: also affects every other KANTO / JOHTO mart (other areas). Check against Kai's own mart policy first (B list
  names "Kai's own marts"). High confidence about the difference.

### K1-9: Oak's POKéDEX rating is Quetzal's "registered in KANTO" talk, not FireRed's graded rating
- Maps: Kanto_PalletTown_ProfessorOaksLab
- Category: B / C
- Original: `PokedexRating_EventScript_RateInPerson` (FR PalletTown_ProfessorOaksLab/scripts.inc:773-806, words in
  FR data/text/pokedex_rating.inc): a comment for each step of the KANTO count, "Wroooaaarrr!" at 150.
- Now: data/maps/Kanto_PalletTown_ProfessorOaksLab/scripts.inc:1023-1062 and texts :1739-1760: "Your NATIONAL MOéDEX:
  {STR_VAR_1} species seen in total. In KANTO, you have registered {STR_VAR_2} species as caught", "Keep registering
  species in KANTO! I look forward to a full NATIONAL MOéDEX for this region!", "You've completed the NATIONAL MOéDEX
  with your KANTO catches!". The words describe Quetzal's per-region registration; the game counts NATIONAL caught
  (VAR_0x8004 = 1) and completion is QzIsRegionalDexComplete = KANTO 1-150 caught anywhere
  (src/qz_specials.c:158). Said and done don't match.
- Restore: FireRed's PokedexRating_EventScript_RateInPerson and its graded texts on the KANTO count
  (QzGetPokedexCount with VAR_0x8004 = 0), keeping the completion check.
- Notes: the PC's "PROF. OAK's PC" rating, if used in KANTO, should match. Medium-high confidence.

### K1-10: The NATIONAL DEX comes with the KANTO dex in OAK's lab; FireRed's National Dex scenes are dead code
- Maps: Kanto_PalletTown, Kanto_PalletTown_ProfessorOaksLab
- Category: C (keep)
- Original: OAK gives the KANTO POKéDEX (FR PalletTown_ProfessorOaksLab/scripts.inc:650-660, "received the POKéDEX
  from PROF. OAK") and, after the LEAGUE + ONE ISLAND + 60 caught, meets the player outside the house for the
  NATIONAL DEX (FR PalletTown/scripts.inc:37-78, lab's NationalDexScene :69-121).
- Now: the dex scene says "Take your MOéDEX on your travels" / "received from OAK the NATIONAL MOéDEX!"
  (ProfessorOaksLab scripts.inc:823-832, texts "843E8FB", "843E930", "843F5D6"), and the lab enables the NATIONAL DEX
  on every visit (:27-31), so Pallet's on-frame OAK scene (data/maps/Kanto_PalletTown/scripts.inc:39-41) always ends at
  once and the lab's VAR 7 scene never runs.
- Restore: keep (Kai gives the NATIONAL DEX in every region: COOP_NOTES "hoenn_national_dex"); optionally put back
  FireRed's words "Take these with you." / "received the POKéDEX from PROF. OAK." with the NATIONAL DEX given.
- Notes: low priority.

### K1-11: The KANTO rival can't be named: "BLUE" is written into every line
- Maps: Kanto_PalletTown_ProfessorOaksLab (22 lines), Kanto_PalletTown (sign "BLUE's house", :537),
  Kanto_PalletTown_RivalsHouse (:83, :88), Kanto_CeruleanCity (:379, :393), Kanto_Route22 (:209, :232),
  Kanto_PewterCity_Gym / Kanto_CeruleanCity_Gym / Kanto_ViridianCity_Gym (gym statues "WINNING TRAINERS: BLUE"),
  Kanto_ViridianCity_School (:210), shared Quetzal_Text_843E142 ("BLUE: Yeah! Am I great or what?")
- Category: D / E
- Original: FireRed names the rival in OAK's intro and writes {RIVAL} (e.g. FR PalletTown_ProfessorOaksLab text,
  Route22 text.inc:2).
- Now: no naming; "BLUE" hard-coded (COOP_NOTES: "The rival is always BLUE").
- Restore: as JOHTO got (REGIONS_TODO JOHTO "The rival named as in Crystal"): a naming step for KANTO's rival
  (FireRed's place: OAK's intro / first visit to the lab; online the host's), stored in KANTO vars, and {RIVAL} in
  these texts and the battle name.
- Notes: needs a user decision; touches K2-K4 maps too. Medium priority.

### K1-12: DAISY: Quetzal's "no friendship" line, a TOWN MAP she never gives, no grooming
- Maps: Kanto_PalletTown_RivalsHouse
- Category: B, E, D
- Original: FR PalletTown_RivalsHouse/scripts.inc:21-51: "Grandpa asked you to run an errand? ... Here, this will
  help you." + TOWN MAP; then "Just like people, POKéMON are living things..."; after the LEAGUE her grooming
  (friendship up) and her friendship rating of the lead POKéMON.
- Now: data/maps/Kanto_PalletTown_RivalsHouse/scripts.inc:18-60, texts :80-100: after OAK's dex scene she explains
  "You can use the TOWN MAP to find out where you are..." though no TOWN MAP is given (E), then forever, and after the
  LEAGUE, says "Want to know how your MOéMON evolves? ... It doesn't depend on friendship here." (B: Kai has
  friendship evolutions). No grooming (D).
- Restore: FireRed's lines: "PleaseGiveMonsRest" instead of the TOWN MAP explanation (the TOWN MAP stays out per
  REGIONS_TODO), "ErrandForGrandpa" without the gift or with a line pointing to Kai's map, and FireRed's GroomMon /
  friendship rating after GAME CLEAR (VAR_MASSAGE_COOLDOWN_STEP_COUNTER logic, Emerald's friendship).
- Notes: high confidence on the B/E parts.

### K1-13: VIRIDIAN's old man never shows how to catch a POKéMON
- Maps: Kanto_ViridianCity
- Category: D
- Original: FR ViridianCity/scripts.inc:97-238: after the PARCEL he says "...let me show you!" and runs the
  catching tutorial battle (`special StartOldManTutorialBattle`), then "That was educational", (TEACHY TV), and
  "WeakenMonsFirstToCatch" afterwards.
- Now: data/maps/Kanto_ViridianCity/scripts.inc:104-205 and text :315-323: at VAR 1 he only says Quetzal's merged
  line ("I made it too strong... are you filling your MOéDEX? ... focus on weakening the MOéMON") and sets VAR 2. The
  tutorial lines ShowYouHowToCatchMons / ThatWasEducationalTakeThis are gone.
- Restore: FireRed's DoTutorialBattle without the TEACHY TV gift (decided out), using Emerald's catching tutorial
  (`special StartWallyTutorialBattle`, data/specials.inc:176) with the old man as the catcher, then FireRed's
  "WeakenMonsFirstToCatch".
- Notes: needs a small battle-setup change for the old man's picture. Medium confidence that it's worth it.

### K1-14: Lines Quetzal rewrote because it had no TMs / HMs (now that FireRed's TMs / HMs are back)
- Maps: Kanto_CeruleanCity_House2, Kanto_Route2_EastBuilding, Kanto_CeruleanCity_Gym, Kanto_ViridianCity_Gym
- Category: E (C)
- Original: CERULEAN robbed house: "They stole a TM for teaching POKéMON how to DIG holes! I was going to use it on a
  MANKEY or a SANDSHREW..." / "I decided to teach DIGLETT how to DIG without a TM." (FR CeruleanCity_House2 text);
  ROUTE 2 aide's partner: "Once a POKéMON learns FLASH, you can get through ROCK TUNNEL."; MISTY: "...You can also have
  my favorite TM."; GIOVANNI: "Also, take this TM. Consider it a gift for your POKéMON LEAGUE challenge."
- Now: data/maps/Kanto_CeruleanCity_House2/scripts.inc:28-40 still says "They stole a valuable SUN STONE! ... evolve
  one of my MOéMON" while the grunt now returns TM28 DIG (Kanto_CeruleanCity/scripts.inc:106-115); Kanto_Route2_EastBuilding
  "ROCK TUNNEL is very dark. Prepare carefully"; Kanto_CeruleanCity_Gym/scripts.inc:102 ends "Please take this gift,
  too."; Kanto_ViridianCity_Gym/scripts.inc:155 "Also, take this gift for your LEAGUE challenge." Also orphaned
  receive lines "{PLAYER} received\n{STR_VAR_1}!" (Gym texts 84426E8 / 84413AC / 843FA0F) and Route 2's
  "received a GALARICA CUFF" (844DC63), unused.
- Restore: FireRed's words in all four places; delete the orphaned texts.
- Notes: fix_leader_tms.py / fix_kanto_tms.py changed the items but missed these lines. High confidence.

### K1-15: GIOVANNI stays in his GYM after the battle
- Maps: Kanto_ViridianCity_Gym
- Category: C
- Original: FR ViridianCity_Gym/scripts.inc:4-13: "...Let us meet again someday! Farewell!", fade, `removeobject`
  (FLAG_HIDE_VIRIDIAN_GIOVANNI): he's gone for good.
- Now: data/maps/Kanto_ViridianCity_Gym/scripts.inc:10-14 and :145: "I will keep training here at my GYM.", no
  removeobject.
- Restore: FireRed's line, the fade and `removeobject 8` after the post-battle message.
- Notes: check that nothing in Quetzal's KANTO uses him for a rematch (Quetzal's leader rematch teams 8192-8204 have
  no script). High confidence.

### K1-16: In-game trades are Quetzal's, not FireRed's
- Maps: Kanto_Route2_House (VAR_0x8008 0x10), Kanto_CeruleanCity_House3 (8), Kanto_UndergroundPath_NorthEntrance (9)
- Category: C
- Original: FR src/data/ingame_trades.h: ROUTE 2 "MIMIEN" MR. MIME for ABRA (OT REYLEY); CERULEAN "ZYNX" JYNX for
  POLIWHIRL holding FAB MAIL (OT DONTAE); UNDERGROUND PATH "MS. NIDO" NIDORAN♀ for NIDORAN♂ holding TINY MUSHROOM
  (FireRed; OT SAIGE), FireRed's IVs.
- Now: src/data/qz_ingame_trades.h: [16] "MR. MIME" MR. MIME for DROWZEE; [8] ZYNX for PSYDUCK, no item; [9]
  "PONYTA" PONYTA for NIDORAN♀; IVs mostly 31/20.
- Restore: FireRed's three entries (species, request, nickname, held item, OT, IVs) in the table rows these maps use
  (tools/quetzal_port/gen_trades.py overrides). The trade lines in the shared scripts (Quetzal_Text_8455CFC...) are
  FireRed's already.
- Notes: REGIONS_TODO's event audit covered gifts, statics and trainers, not trades. High confidence.

### K1-17: CERULEAN CAVE 2F's PP UP is a NUGGET
- Maps: Kanto_CeruleanCave_2F (9,18)
- Category: C
- Original: CeruleanCave_2F_EventScript_ItemPPUp (FR data/scripts/item_ball_scripts.inc).
- Now: data/maps/Kanto_CeruleanCave_2F/scripts.inc:10-15 ITEM_NUGGET.
- Restore: ITEM_PP_UP.
- Notes: the only item ball in K1 that differs. High confidence.

### K1-18: CERULEAN's BERRY POWDER man gives a NUGGET while saying "BERRY SWEET"
- Maps: Kanto_CeruleanCity_House5
- Category: E (C)
- Original: FR CeruleanCity_House5/scripts.inc:4-16: the BERRY POWDER man (POWDER JAR, then exchanges powder for
  medicine / vitamins).
- Now: data/maps/Kanto_CeruleanCity_House5/scripts.inc:4-20, texts :33-50: "I study rare sweets made from BERRIES...
  Please, take this BERRY SWEET." "A BERRY SWEET can help a certain MOéMON evolve." but gives ITEM_NUGGET (Quetzal's
  item 0xF0, a Gen 8 item, became a NUGGET at the Gen 4 cap).
- Restore: the POWDER JAR is decided out (REGIONS_TODO), so FireRed's man can't work; either give him FireRed's
  "TakeInterestInAllSortsOfThings"-style idle lines with no gift, or keep a gift whose words name the item given.
- Notes: high confidence on the mismatch.

### K1-19: The BIKE SHOP talks about Quetzal's MACH / ACRO bicycle and has no shop
- Maps: Kanto_CeruleanCity_BikeShop
- Category: B (C)
- Original: FR CeruleanCity_BikeShop/scripts.inc:4-40: "Hi! Welcome to our BIKE SHOP. Have we got just the BIKE for
  you!" with the BICYCLE / ¥1,000,000 menu ("Sorry! You can't afford it!"), the voucher exchange, "How do you like
  your new BICYCLE? ... CYCLING ROAD and even into caves!"; the kid: "These bikes are cool, but they're way
  expensive!"
- Now: data/maps/Kanto_CeruleanCity_BikeShop/scripts.inc:58-100: "Get a BIKE VOUCHER there and exchange it here",
  "This BICYCLE has two modes: MACH and ACRO. ... Press L to switch modes." Kai's BICYCLE is FireRed's one-speed bike
  (src/data/items.h ITEM_BICYCLE: "rides as FireRed's, one speed"; L is a registered-item button): the words are
  false.
- Restore: FireRed's clerk script and words (menu with the BICYCLE, money box, voucher exchange, "How do you like
  your new BICYCLE?"), FireRed's kid line.
- Notes: high confidence.

### K1-20: ROUTE 5's DAY-CARE is Quetzal's incense shop ("The DAY CARE is now managed through the PC")
- Maps: Kanto_Route5_PokemonDayCare; shared data/scripts/qz/shared.inc:216-224 (also used by Kanto_FourIsland and
  Johto_DayCare, other areas)
- Category: B
- Original: FR data/scripts/day_care.inc:1 Route5_PokemonDayCare_EventScript_DaycareMan (FireRed's one-POKéMON
  DAY-CARE: "I run the DAY-CARE service..."), the man at (4,4).
- Now: data/maps/Kanto_Route5_PokemonDayCare/scripts.inc: an object at (2,4) running Quetzal_EventScript_83F0976:
  "The DAY CARE is now managed through the PC. We sell special INCENSES here for breeding." and a mart of nine
  incenses. Kai has no PC DAY CARE: the words describe Quetzal's mechanic.
- Restore: FireRed's DAY-CARE man (one POKéMON, gains EXP, ¥100 + 100/level) on Kai's day-care code, or Kai's own
  DAY CARE here; the incenses can stay sold elsewhere if wanted.
- Notes: the brief lists "DAY-CARE from any PC" as a Quetzal mechanic to report. High confidence.

### K1-21: Gift POKéMON sent to the PC name the wrong BOX
- Maps: callers of Quetzal_EventScript_84590FF: Kanto_Route4_PokemonCenter_1F (MAGIKARP) and four other KANTO maps;
  shared data/scripts/qz/kanto_shared.inc:1358-1363
- Category: A / E
- Original: FireRed's `bufferboxname STR_VAR_1, VAR_PC_BOX_TO_SEND_MON` (the box the game just used).
- Now: `bufferboxname 0, VAR_QZ_0x513F` - FireRed's VAR_PC_BOX_TO_SEND_MON number in Quetzal's numbering, which no
  C code sets (only Emerald's own VAR_PC_BOX_TO_SEND_MON is updated), so "It was placed in BOX "..."" always names
  box 0's name.
- Restore: buffer from Emerald's VAR_PC_BOX_TO_SEND_MON (or `specialvar VAR_RESULT, GetPCBoxToSendMon` as the
  box-full branch already does).
- Notes: medium-high confidence (didn't run it).

### K1-22: PALLET's sign girl never moves to the ROUTE 1 exit (FLAG_OPENED_START_MENU is never set)
- Maps: Kanto_PalletTown
- Category: A
- Original: FR PalletTown/scripts.inc:8-35: once the player has opened the START menu (FLAG_OPENED_START_MENU, set by
  the menu code) she waits by the route exit and shows the TRAINER TIPS sign.
- Now: data/maps/Kanto_PalletTown/scripts.inc:11-40 test FLAG_QZ_0x21E7 (that flag); no C code sets 0x21E7
  (include/constants/qz_port.h:745 only), so that branch never runs; she only does her sign-side lines.
- Restore: set FLAG_QZ_0x21E7 when the START menu opens in KANTO (start_menu.c), as FireRed does.
- Notes: harmless but a FireRed scene lost. Medium confidence.

### K1-23: CERULEAN rival gives a LUCKY EGG with Quetzal's lines
- Maps: Kanto_CeruleanCity
- Category: C
- Original: FR CeruleanCity/scripts.inc:58: "I feel sorry for you... here, I'll give you a little present as a
  favor." + FAME CHECKER, "A chatty gossip like you... That thing's perfect... Smell ya!"
- Now: data/maps/Kanto_CeruleanCity/scripts.inc:59-65: "I picked up an extra LUCKY EGG. You can have it--try to keep
  up!" + ITEM_LUCKY_EGG, "Have a MOéMON hold it to gain more EXP...".
- Restore: the FAME CHECKER is decided out, so either drop the gift and go from the battle to FireRed's "...this time
  I really am gone. Smell ya!" ending, or keep the LUCKY EGG as a deliberate stand-in (user's call).
- Notes: LUCKY EGG this early is a strong item (Quetzal's balance). Medium.

### K1-24: Small Quetzal rewrites of FireRed's words
- Maps / Now / Original:
  - Kanto_CeruleanCity_Gym:152 gym guide: "GRASS and ELECTRIC types can help! But watch out for Abilities and
    secondary types!" (FR: "You can drain all their water with GRASS-type POKéMON. Or, you might use ELECTRIC-type
    POKéMON and zap them!")
  - Kanto_PewterCity_Museum_1F:333-344: AERODACTYL / KABUTOPS displays: "A reconstruction of Aerodactyl's skeleton..."
    (FR: "AERODACTYL Fossil\nA primitive and rare POKéMON.", FR PewterCity_Museum_1F/scripts.inc:169-189).
  - Kanto_PowerPlant:180 ZAPDOS's cry "Zapdos!" (FR Text_Gyaoo "Gyaoo!", FR data/event_scripts.s:1006).
  - Kanto_PewterCity_PokemonCenter_1F object 5 (13,2): "TRAINERS from all sorts of places pass through here..."
    (FR: the MYSTERY EVENT club woman; Mystery Event isn't in the game, so a filler line is reasonable: keep).
  - Kanto_CeruleanCity_House4: "I love reading stories of travels..." (FR: the news man, Mystery Gift news and
    BERRIES; FireRed's idle line "Sigh... Too much time, too little to do..." could stand alone).
  - Kanto_PalletTown_ProfessorOaksLab dex scene: "Use POKé BALLS to catch wild MOéMON." (FR "So, here are some tools
    for catching wild POKéMON." + "received five POKé BALLS": the balls are given silently by
    Quetzal_EventScript_835FF25).
- Category: C
- Restore: FireRed's lines (the CERULEAN gym guide's own ability tip can stay if the user prefers it).
- Notes: low priority, high confidence.

### K1-25: OAK's first-battle advice is missing from the lab battle
- Maps: Kanto_PalletTown_ProfessorOaksLab
- Category: D
- Original: `trainerbattle_earlyrival ..., RIVAL_BATTLE_TUTORIAL` (FR PalletTown_ProfessorOaksLab/scripts.inc:333): the
  tutorial flag makes OAK comment in battle (FireRed's first-battle help).
- Now: `qz_trainerbattle_13 TRAINER_QZ_8518, 3, ...` keeps the flags but src/battle_setup.c (sQzEarlyRivalFlags,
  :125, :1644) only uses bit 1 (heal after).
- Restore: bit 2 -> FireRed's OAK tutorial messages in that battle.
- Notes: low priority; medium confidence on FireRed's exact behaviour.

### K1-26: ROUTE 22's gate north to JOHTO (Quetzal's) - consistent with Gen 2 geography
- Maps: Kanto_Route22_NorthEntrance (officer at (3,2), warp (3,1) -> MAP_JOHTO_VICTORY_ROAD_GATE)
- Category: C (keep)
- Original: FireRed's gate has no north door to another region; GSC / HGSS do link ROUTE 22 to ROUTE 26.
- Now: Quetzal_Text_83E90EF "Only the champions of both POKéMON LEAGUES may cross" (QzVictoryRoadGateCheck:
  both LEAGUEs beaten, src/qz_specials.c:265). Kai's region travel handles walking in (MP_Region_OnMapLoaded).
- Restore: keep (matches HGSS and works with Kai's travel); listed for completeness.
- Notes: none.

---

## Coverage

| Map | Result |
|---|---|
| Kanto_CeruleanCave_1F | K1-5 (ETERNATUS); otherwise same as the original |
| Kanto_CeruleanCave_2F | K1-6, K1-17 |
| Kanto_CeruleanCave_B1F | same as the original (MEWTWO Lv 70, "Mew!", items) |
| Kanto_CeruleanCave_RayquazaChamber | K1-6 (Quetzal's own map) |
| Kanto_CeruleanCity | K1-3, K1-5, K1-11, K1-23; TM28 grunt, policeman, cave guard as FireRed |
| Kanto_CeruleanCity_BikeShop | K1-19 |
| Kanto_CeruleanCity_Gym | K1-11 (statue), K1-14, K1-24 |
| Kanto_CeruleanCity_House1 | same as the original (badge list) |
| Kanto_CeruleanCity_House2 | K1-14 |
| Kanto_CeruleanCity_House3 | K1-16 |
| Kanto_CeruleanCity_House4 | K1-24 |
| Kanto_CeruleanCity_House5 | K1-18 |
| Kanto_CeruleanCity_Mart | K1-8 |
| Kanto_CeruleanCity_PokemonCenter_1F | only trivial differences (FireRed's POKéMON JOURNAL objects gone with the FAME CHECKER; Kai's TRIPLE TRIAD seller / CARD QUEEN added) |
| Kanto_CeruleanCity_PokemonCenter_2F | only trivial differences (Kai's GARDEN replaces the link rooms and MYSTERY GIFT man) |
| Kanto_DiglettsCave_B1F | K1-5 (ZYGARDE) |
| Kanto_DiglettsCave_NorthEntrance | same as the original |
| Kanto_DiglettsCave_SouthEntrance | same as the original |
| Kanto_MtMoon_1F | K1-5 (DIALGA / PALKIA hidden); otherwise same |
| Kanto_MtMoon_B1F | same as the original |
| Kanto_MtMoon_B2F | K1-5 (NECROZMA); fossils, TM46 as FireRed |
| Kanto_MtMoon_Route4Cave | K1-7 (Quetzal's own map) |
| Kanto_PalletTown | K1-3, K1-10, K1-11, K1-22 |
| Kanto_PalletTown_PlayersHouse_1F | same as the original |
| Kanto_PalletTown_PlayersHouse_2F | K1-4 |
| Kanto_PalletTown_ProfessorOaksLab | K1-5 (MELTAN), K1-9, K1-10, K1-11, K1-24, K1-25 (starter table is Kai's, decided) |
| Kanto_PalletTown_RivalsHouse | K1-11, K1-12 |
| Kanto_PewterCity | K1-3, K1-5 (SHAYMIN hidden); RUNNING SHOES aide as decided |
| Kanto_PewterCity_Gym | K1-11 (statue); otherwise same (TM39) |
| Kanto_PewterCity_House1 | same as the original |
| Kanto_PewterCity_House2 | same as the original |
| Kanto_PewterCity_Mart | K1-8 |
| Kanto_PewterCity_Museum_1F | K1-5 (HOOPA), K1-24; SEISMIC TOSS tutor and OLD AMBER as FireRed |
| Kanto_PewterCity_Museum_2F | K1-5 (JIRACHI, kept by decision) |
| Kanto_PewterCity_PokemonCenter_1F | K1-24 (MYSTERY EVENT woman's line) |
| Kanto_PewterCity_PokemonCenter_2F | only trivial differences (Kai's GARDEN) |
| Kanto_PowerPlant | K1-5 (XURKITREE), K1-24 (ZAPDOS cry text) |
| Kanto_RockTunnel_1F | K1-2 |
| Kanto_RockTunnel_B1F | K1-2, K1-5 (TERRAKION); ROCK SLIDE tutor as FireRed |
| Kanto_Route1 | same as the original |
| Kanto_Route10 | K1-1, K1-5 (YVELTAL, ZERAORA; RAIKOU hidden) |
| Kanto_Route10_PokemonCenter_1F | same as the original (EVERSTONE aide) |
| Kanto_Route10_PokemonCenter_2F | only trivial differences (Kai's GARDEN) |
| Kanto_Route2 | K1-5 (ARCEUS trial, hidden) |
| Kanto_Route22 | K1-5 (AZELF / ARCEUS hidden), K1-11 |
| Kanto_Route22_NorthEntrance | K1-26 |
| Kanto_Route24 | K1-1 |
| Kanto_Route25 | K1-1, K1-5 (ENAMORUS; SUICUNE hidden) |
| Kanto_Route25_SeaCottage | K1-5 (POIPOLE); BILL's scene as FireRed |
| Kanto_Route2_EastBuilding | K1-14 |
| Kanto_Route2_House | K1-16 |
| Kanto_Route2_ViridianForest_NorthEntrance | same as the original |
| Kanto_Route2_ViridianForest_SouthEntrance | same as the original |
| Kanto_Route3 | K1-1 |
| Kanto_Route4 | K1-1, K1-5 (CRESSELIA hidden), K1-7; MEGA PUNCH / KICK tutors as FireRed |
| Kanto_Route4_PokemonCenter_1F | K1-21 (MAGIKARP salesman otherwise FireRed's) |
| Kanto_Route4_PokemonCenter_2F | only trivial differences (Kai's GARDEN) |
| Kanto_Route5 | K1-5 (ARCEUS trial, hidden) |
| Kanto_Route5_PokemonDayCare | K1-20 |
| Kanto_Route5_SouthEntrance | same as the original (TEA guard) |
| Kanto_Route6 | K1-1, K1-5 (MESPRIT / ARCEUS hidden) |
| Kanto_Route6_NorthEntrance | same as the original (TEA guard) |
| Kanto_Route9 | K1-1, K1-5 (ARCEUS trial, hidden) |
| Kanto_UndergroundPath_EastEntrance | same as the original |
| Kanto_UndergroundPath_EastWestTunnel | same as the original |
| Kanto_UndergroundPath_NorthEntrance | K1-16 |
| Kanto_UndergroundPath_NorthSouthTunnel | same as the original |
| Kanto_UndergroundPath_SouthEntrance | same as the original |
| Kanto_UndergroundPath_WestEntrance | same as the original |
| Kanto_ViridianCity | K1-3, K1-13; DREAM EATER tutor as FireRed |
| Kanto_ViridianCity_Gym | K1-11 (statue), K1-14, K1-15 |
| Kanto_ViridianCity_House | same as the original |
| Kanto_ViridianCity_Mart | K1-8 (PARCEL scene as FireRed) |
| Kanto_ViridianCity_PokemonCenter_1F | same as the original |
| Kanto_ViridianCity_PokemonCenter_2F | only trivial differences (Kai's GARDEN) |
| Kanto_ViridianCity_School | K1-11 (journal names BLUE) |
| Kanto_ViridianForest | K1-5 (ZACIAN, ZAMAZENTA) |

Shared scripts with findings: data/scripts/qz/kanto_shared.inc (Jessie & James 156-202: K1-3; gift box name
1358-1363: K1-21; overworld legendaries 344-400: K1-5), data/scripts/qz/shared.inc (DAY CARE incense shop 216-224,
532-540: K1-20; Jessie & James text 501-525: K1-3).
