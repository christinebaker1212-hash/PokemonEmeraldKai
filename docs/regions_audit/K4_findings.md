# K4 findings: SAFFRON / SILPH CO., FUCHSIA / SAFARI ZONE, CINNABAR / MANSION, SEAFOAM, ROUTES 19-21 / 23, VICTORY ROAD, INDIGO PLATEAU / LEAGUE

Method: every map in K4_maps.txt was compared with pret/pokefirered (local clone, plus pokefirered's
`data/text/*.inc`, `src/data/ingame_trades.h`, `src/data/trainer_parties.h`, `src/field_control_avatar.c`,
`src/field_specials.c` fetched from github master, since the clone has only data/maps and data/scripts):
all `.string` text (fuzzy-matched against the map's FireRed text and then all of FireRed's text), the
key script commands (gifts, items, trainers, flags, specials, warps, marts), and map.json objects / signs /
hidden items / triggers / warps by position. Item balls (55) match FireRed's; trainers match FireRed's by name on
every map. Fame Checker journals, the PC 2F link-room warps and the Mystery Gift men are gone on purpose
(docs/REGIONS_TODO.md) and are not reported. Paths below are relative to /home/user/PokemonEmeraldKai unless
they start with "FR:" (= /tmp/claude-0/frlg/pokefirered/) or "FR-gh:" (pokefirered master on github).

---

### K4-1: Boulders can't fall through holes, so SEAFOAM's currents can never be stopped (and VICTORY ROAD's 3F->2F boulder never drops)
- Maps: Kanto_SeafoamIslands_1F, _B1F, _B2F, _B3F, _B4F (and Kanto_Route20, which resets the puzzle); Kanto_VictoryRoad_3F / _2F (Route 23 resets them)
- Category: A
- Original: FR's boulder push ends with `HandleBoulderFallThroughHole` (FR-gh: src/field_control_avatar.c:1066): a boulder pushed onto a hole (MB_FALL_WARP) is removed and the flag stored in its template's trainerType (GetBoulderRevealFlagByLocalIdAndMap) is cleared, so the boulder appears on the floor below. SEAFOAM B3F / B4F stop their currents once their two boulders have dropped in (FR: data/maps/SeafoamIslands_B3F/scripts.inc, _B4F/scripts.inc; ROUTE 20 resets the drop flags, FR: data/maps/Route20/scripts.inc:5-25).
- Now: src/field_player_avatar.c:824-845 `TryPushBoulder` refuses any target tile with `MetatileBehavior_IsNonAnimDoor`, and KANTO's holes are exactly that behavior (Kanto_SeafoamIslands secondary metatile 646 = 0x60 MB_NON_ANIMATED_DOOR: the hole warps at SEAFOAM 1F (21,8) / (30,8) etc.). Nothing ports HandleBoulderFallThroughHole (only the switch half: field_control_avatar.c:1041 TryRunBoulderSwitchScript). The data for it is there: the boulders' map.json `trainer_type` holds the reveal flags (SeafoamIslands_1F (22,12) = 8637 = FLAG_QZ_0x21BD = FLAG_HIDE_SEAFOAM_B1F_BOULDER_1, ... B3F (6,17) = 8647, VictoryRoad_3F (33,18) = 8664). Route 20 (data/maps/Kanto_Route20/scripts.inc:5-31) hides the lower boulders on every entry, so B3F (data/maps/Kanto_SeafoamIslands_B3F/scripts.inc OnFrame: the current sweeps the player to B4F) and B4F (Kanto_SeafoamIslands_B4F/scripts.inc 8412DE8: pushed back) never see both boulders.
- Restore: port FireRed's fall: let a boulder be pushed onto a KANTO hole metatile (and only that: Emerald's other NonAnimDoor tiles stay blocked), then remove the object, play SE_FALL and FlagClear(template trainerType) as FR does. Then test the SEAFOAM puzzle end to end (B3F current stops, B4F current stops, ARTICUNO reached) and VICTORY ROAD 3F (33,18) -> 2F (33,19).
- Notes: not covered by any test (qz_strength only checks the switch). I'm confident boulders can't enter the holes; that this makes ARTICUNO / the east-west crossing of SEAFOAM unreachable is my reading of the current scripts, verify in game. Confidence: high (mechanism), medium (how much is blocked).

### K4-2: Quetzal's PRIMAL CHAMBER still runs: MAXIE / ARCHIE at Lv 70, then GROUDON / KYOGRE appear and can be battled in KANTO
- Maps: Kanto_CinnabarIsland_PrimalChamber (reached by Quetzal's warp in Kanto_PokemonMansion_B1F events.inc:18, (31,2))
- Category: A (and C)
- Original: no such map or warp in FireRed (PokemonMansion_B1F has no warp at (31,2)).
- Now: MAXIE (TRAINER_QZ_8935) and ARCHIE (8936), six Lv 70 POKéMON each, are always there (flags FLAG_QZ_0x2E3D / 0x2E3E are never set). After the win the script does `clearflag FLAG_QZ_0x23A9` + `addobject 3` (GROUDON) / `clearflag FLAG_QZ_0x2E3A` + `addobject 4` (KYOGRE) (data/maps/Kanto_CinnabarIsland_PrimalChamber/scripts.inc:26-27 and 78-79). MP_HideForeignLegends only runs after a map's transition scripts, and SetUpStatic (src/qz_specials.c:328) has no foreign-legend check, so the legendary stands there and Quetzal_EventScript_840FBBD starts the battle. Both trainers can be rematched forever (8422527 / 842277F). docs/REGIONS_TODO.md "HOENN ... Quetzal's CINNABAR chamber empty" is marked [x] but isn't true for this path.
- Restore: remove the chamber's way in (the Mansion B1F warp at (31,2)) and hide MAXIE / ARCHIE (or delete their objects); if the RED / BLUE ORB gift (fix_prize_room.py decision) should stay reachable, give it without the HOENN legendaries (or drop it: FireRed has neither ORB). At minimum make QzKanto/Johto static setup refuse HiddenForeignLegend entries.
- Notes: the user decided the ORBs replace TM02/TM03 here, so ask before removing the map entirely. Confidence high.

### K4-3: Jessie & James' traveling battle (Quetzal) still appears after the LEAGUE, and its PIKACHU reward gives species 0
- Maps: Kanto_CinnabarIsland, Kanto_FuchsiaCity, Kanto_SaffronCity, Kanto_IndigoPlateau_Exterior (4 objects each, Quetzal_EventScript_83C303A; also every other KANTO town)
- Category: C (and A)
- Original: nothing in FireRed.
- Now: src/qz_specials.c:1044-1066 TravelingEvent shows them in one KANTO town a day once KANTO's LEAGUE is beaten; data/scripts/qz/kanto_shared.inc:156-197 plays the anime motto, battles TRAINER_QZ_913 and then `qz_givemon_ex VAR_0x8000 (= VAR_QZ_0x514A), 50`; nothing in data/ or src/ ever sets VAR_QZ_0x514A (only its reset to 0 at line 190), so the gift is species 0 (text data/scripts/qz/shared.inc:501-525, "{PLAYER} received Pikachu!").
- Restore: hide them for good (QzKantoTravelingEventCheck_ always FALSE + FlagSet 0x2EAB), as was done for GIOVANNI's quest; or delete the four objects per town.
- Notes: the same applies to JOHTO (QzJohtoTravelingEventCheck_, flag 0x1490): another area's maps. Confidence high.

### K4-4: FUCHSIA's MOVE DELETER only says "delete moves on your summary screen", which Kai can't do
- Maps: Kanto_FuchsiaCity_House3
- Category: B (text promises a mechanic the game lacks; FireRed function missing)
- Original: FR: data/maps/FuchsiaCity_House3/scripts.inc:4-80, the MOVE DELETER (ChoosePartyMon, SelectMoveDeleterMove, MoveDeleterForgetMove...).
- Now: data/maps/Kanto_FuchsiaCity_House3/scripts.inc:5-12: "You can delete moves on your MOéMON's summary screen. You don't need my help anymore!" src/pokemon_summary_screen.c has no move deleting (only the HM "can't forget" window).
- Restore: FireRed's script and words; Emerald's own specials are in the game (LILYCOVE's MOVE DELETER, data/maps/LilycoveCity_MoveDeletersHouse/scripts.inc), so it is a straight copy.
- Notes: docs/quetzal/KANTO.md section 1 lists it; not in REGIONS_TODO. LAVENDER's NAME RATER (another area) has the same pattern (though Kai's summary SELECT rename does exist). Confidence high.

### K4-5: The SAFARI GAME is gone: no fee, no SAFARI BALLS, no step limit; signs and people rewritten to say so
- Maps: Kanto_FuchsiaCity_SafariZone_Entrance, Kanto_SafariZone_Center, _East, _North, _North_RestHouse (and every Safari area's wild battles)
- Category: D (and C for the rewritten lines)
- Original: FR: data/maps/FuchsiaCity_SafariZone_Entrance/scripts.inc: the three entry triggers (82-...), ¥500 (removemoney 500, l.116), 30 SAFARI BALLS, EnterSafariMode (l.130) / ExitSafariMode, the info attendant (l.194) with ExplainSafariZone / "Sorry, you're a regular here!"; signs "PRESS START TO CHECK REMAINING TIME" (Center), "THE REMAINING TIME DECLINES ONLY WHILE YOU WALK" (East), "WIN A FREE HM FOR FINDING THE SECRET HOUSE" (North); NORTH REST HOUSE "...you'll run out of time if you try for all of them at once."
- Now: data/maps/Kanto_FuchsiaCity_SafariZone_Entrance/scripts.inc: only the attendant, "Battles work normally, so bring your own POKé BALLS. There is no fee or time limit." / "Then you know the way. Have fun!"; the entry triggers are gone (map.json coord_events empty). Rewritten signs: Kanto_SafariZone_Center/scripts.inc:34, _East:52, _North:68; Kanto_SafariZone_North_RestHouse/scripts.inc:30.
- Restore: FireRed's entrance script and words on Emerald's own SAFARI ZONE engine (EnterSafariMode / ExitSafariMode, safari_zone.c, already used by HOENN's), FireRed's 600 steps / 30 balls, and FireRed's sign / rest-house lines (the North sign's "free HM" is true again now that the SECRET HOUSE gives HM03).
- Notes: co-op needs a thought (HOENN's Safari Zone already has to cope). Confidence high.

### K4-6: The POKéMON TRAINER FAN CLUB is Quetzal's "KANTO EXPLORERS' CLUB" of hint people, some describing things Kai doesn't have
- Maps: Kanto_SaffronCity_PokemonTrainerFanClub, Kanto_SaffronCity (its sign)
- Category: C (and B / D)
- Original: FR: data/maps/SaffronCity_PokemonTrainerFanClub/scripts.inc: the fan club (first-meeting scene, eight members who are fans of the player or of a trainer, losing fans with play time: Script_GetNumFansOfPlayerInTrainerFanClub, Script_IsFanClubMemberFanOfPlayer, Script_BufferFanClubTrainerName, Script_TryLoseFansFromPlayTime; 38 lines of text); sign "POKéMON TRAINER FAN CLUB / Many TRAINERS have scribbled their names on this sign." (FR: data/maps/SaffronCity/text.inc:102).
- Now: data/maps/Kanto_SaffronCity_PokemonTrainerFanClub/scripts.inc: 11 hint lines. Wrong for Kai: "search for a species with DexNav ... Your DexNav bonus carries over between regions" (Kai has no DexNav: nothing in src/include); PRIMEAPE evolves into ANNIHILAPE "if it levels up while knowing DYNAMIC PUNCH" (buffermovename MOVE_DYNAMIC_PUNCH) but Kai's is EVO_LEVEL 35 (src/data/pokemon/evolution_gen59.h:217); LORELEI's dolls "start growing after you beat the KANTO LEAGUE twice, the last after twenty wins" but Kai's QzUpdateLoreleiDollCollection (src/qz_specials.c:854-863) adds one per 25 HALL OF FAME entries; the regional-dex rule ("you must obtain or evolve it here ... Taking it out of the PC doesn't count!") is Quetzal's. Sign data/maps/Kanto_SaffronCity/scripts.inc:267 "KANTO EXPLORERS' CLUB".
- Restore: FireRed's fan club (scripts and words) with a port of its fan-count logic (FR src/trainer_fan_club.c; a small save record in the KANTO region sidecar), and FireRed's sign. If the fan logic isn't wanted now, at least FireRed's people and their pre-game-clear lines instead of the hints.
- Notes: the CHANSEY Dance / TANOBY hints point at Quetzal events Kai still has (MELOETTA), so dropping them loses a hint. Confidence high.

### K4-7: SILPH CO. 1F receptionist opens an empty shop after GIOVANNI is beaten
- Maps: Kanto_SilphCo_1F
- Category: A (and C)
- Original: FR: data/maps/SilphCo_1F/scripts.inc:9 "Welcome. The PRESIDENT is in the boardroom on 11F." only.
- Now: data/maps/Kanto_SilphCo_1F/scripts.inc:10-21 (mart at l.16): with VAR_QZ_0x512D (VAR_MAP_SCENE_SILPH_CO_11F) = 1 she adds "We also sell supplies for your research." and `pokemart Kanto_SilphCo_1F_Mart_8411364`, whose list is only ITEM_NONE (Quetzal's goods were removed).
- Restore: FireRed's receptionist (msgbox only); delete the mart and the extra line.
- Notes: confidence high.

### K4-8: Gym leaders' words around FireRed's TMs still describe Quetzal's held items
- Maps: Kanto_SaffronCity_Gym, Kanto_CinnabarIsland_Gym (Kanto_FuchsiaCity_Gym only the receive line)
- Category: E
- Original: SABRINA: after TM04 "TM04 is CALM MIND..." and afterwards SaffronCity_Gym_Text_SabrinaPostBattle "Psychic power isn't something that only a few people have..." (FR text.inc:18); badge line ends "Wait, please take this TM with you." (l.24). BLAINE: after TM38 "FIRE BLAST is the ultimate fire technique. Don't waste it on WATER POKéMON." and afterwards BlainePostBattle "TM38 contains FIRE BLAST..." (FR: CinnabarIsland_Gym/text.inc:16, 31). All three: "{PLAYER} received TM.. from <LEADER>." (giveitem_msg).
- Now: SAFFRON post-battle line is the CHOICE SPECS text "It boosts the holder's Special Attack, but locks it into one move until you switch MOéMON." (data/maps/Kanto_SaffronCity_Gym/scripts.inc:11-15 -> Text_844B736 l.157); badge line "Please take this gift, too." (l.144). CINNABAR post-battle is the LIFE ORB text "That item will make your attacks even more powerful! ... All that power comes at a price!" (Kanto_CinnabarIsland_Gym/scripts.inc:55-59 -> Text_844965A l.784) and the line after the TM is "TM38 contains FIRE BLAST..." (l.768) instead of "FIRE BLAST is the ultimate...". KOGA's are right. All three use plain `giveitem` ("received TM..").
- Restore: SAFFRON 844B736 -> SabrinaPostBattle, 844B655's last line -> "Wait, please take this TM with you."; CINNABAR 844965A -> "TM38 contains FIRE BLAST..." and 844959A -> "FIRE BLAST is the ultimate fire technique..."; optionally FireRed's "received TM.. from <LEADER>" messages.
- Notes: tools/quetzal_port/fix_leader_tms.py swapped the items but not these texts; worth checking the other five KANTO gyms (other areas) for the same leftovers. Confidence high.

### K4-9: CINNABAR LAB's three in-game trades are Quetzal's (PORYGON2, SCIZOR, KINGDRA, all 31 IVs)
- Maps: Kanto_CinnabarIsland_PokemonLab_Lounge, Kanto_CinnabarIsland_PokemonLab_ExperimentRoom
- Category: C
- Original: FR-gh: src/data/ingame_trades.h: ELECTRODE "ESPHERE" for RAICHU (CLIFTON), TANGELA "TANGENY" for VENONAT (NORMA), SEEL "SEELOR" for PONYTA (GARETT) (FR: data/maps/CinnabarIsland_PokemonLab_Lounge/scripts.inc:11, 45; _ExperimentRoom/scripts.inc:11).
- Now: data/maps/Kanto_CinnabarIsland_PokemonLab_Lounge/scripts.inc:10, 47 (trades 0xD, 0xE) and _ExperimentRoom/scripts.inc:5 (0xF) -> src/data/qz_ingame_trades.h [13] PORYGON2 for RAICHU, [14] SCIZOR for HERACROSS, [15] KINGDRA for RAPIDASH (OT names kept, perfect IVs).
- Restore: FireRed's three entries in tools/quetzal_port/gen_trades.py (species, nicknames, requested species, FR's IVs / personality).
- Notes: COOP_NOTES "JOHTO and KANTO" only describes the table; no decision to keep it. The other KANTO trades (ROUTE 2, 11, 18, CERULEAN, VERMILION...) are other areas but the same table. Confidence high.

### K4-10: Fossil revival is Quetzal's: any region's fossil (and Galar half fossils), revived at once, Lv 20
- Maps: Kanto_CinnabarIsland_PokemonLab_ExperimentRoom (Kanto_CinnabarIsland_PokemonLab_Entrance's ready-scene var too)
- Category: C
- Original: FR: data/maps/CinnabarIsland_PokemonLab_ExperimentRoom/scripts.inc: HELIX / DOME FOSSIL and OLD AMBER only; "Oh! That is <fossil>... My Resurrection Machine will make that POKéMON live again!", "I take a little time! You go for walk a little while!", come back after leaving the building ("You late. Where were you? Your fossil is back to life! It was <mon> like I think!"); givemon Lv 5 (l.201-223).
- Now: Kanto_..._ExperimentRoom/scripts.inc:39-170: QzBagHasFossil / QzChooseItemFromBagForFossil / QzIsSelectedItemFossilHalf / QzAreFossilHalvesCompatible, instant "Your fossil is back to life!"; src/qz_specials.c:607-655: ROOT / CLAW / ARMOR / SKULL FOSSIL too, ScriptGiveMon at Lv 20 in KANTO.
- Restore: FireRed's three-fossil script, its words and its wait (VAR_MAP_SCENE_..._REVIVE_STATE already exists as VAR_QZ_0x510C / 0x510D: the entrance sets 2 as in FR), Lv 5; HOENN's / SINNOH's fossils stay with their own regions' revivers.
- Notes: level scaling may lift the Lv 5 anyway. Confidence high.

### K4-11: ELITE FOUR rematches: the rematch words come right after the first win, FireRed's stronger rematch teams are missing
- Maps: Kanto_PokemonLeague_LoreleisRoom, _BrunosRoom, _AgathasRoom, _LancesRoom, _ChampionsRoom (Kanto_IndigoPlateau_PokemonCenter_1F)
- Category: D
- Original: each room picks Intro / RematchIntro and the trainer TRAINER_ELITE_FOUR_<X> / <X>_2 (CHAMPION_FIRST_* / CHAMPION_REMATCH_*) by FLAG_SYS_CAN_LINK_WITH_RS, set when the SEVII ISLANDS' network machine is fixed (FR: data/maps/PokemonLeague_LoreleisRoom/scripts.inc:60-80, ChampionsRoom/scripts.inc:16-27, 124-144); the INDIGO door guard closes the LEAGUE after the NATIONAL DEX until then ("LORELEI is absent").
- Now: the rooms test FLAG_QZ_0x2108 (FLAG_SYS_GAME_CLEAR, set at the first HALL OF FAME) for the words (e.g. Kanto_PokemonLeague_LoreleisRoom/scripts.inc:70-71, ChampionsRoom:41-42) and always battle the first teams (TRAINER_QZ_8602-8605, 8630). Quetzal's own "rematch" teams 8200-8204 (mostly legendaries) are unused. The INDIGO guard uses GAME_CLEAR / FLAG_QZ_0x220A (Kanto_IndigoPlateau_PokemonCenter_1F/scripts.inc:5-30). Also, ONE ISLAND's network machine runs QzClearSeagallopRestrictions (src/qz_specials.c:285-290, called at data/maps/Kanto_OneIsland_PokemonCenter_1F/scripts.inc:251), which clears FLAG_QZ_0x2108 again, so after that the rooms go back to the first-time words.
- Restore: test FLAG_QZ_0x220A (CAN_LINK_WITH_RS) as FireRed does and battle FireRed's rematch parties (FR-gh: src/data/trainer_parties.h sParty_EliteFour*_2, sParty_Champion*Rematch*), e.g. in Quetzal's unused 8200-8204 slots (BLUE's three versions as sQzBlueBattles does); stop QzClearSeagallopRestrictions from clearing 0x2108 (One Island is another area: check what FireRed's SetPostgameFlags really does first).
- Notes: low risk; level scaling still applies on top. Confidence high (conditions), medium (the 0x2108 clearing's intent).

### K4-12: Gym leaders', ELITE FOUR's and route trainers' teams are Quetzal's, not FireRed's
- Maps: Kanto_FuchsiaCity_Gym, Kanto_SaffronCity_Gym, Kanto_CinnabarIsland_Gym, the four ELITE FOUR rooms, and every trainer on Routes 19-21, SILPH CO., the MANSION, VICTORY ROAD
- Category: C
- Original: FR-gh: src/data/trainer_parties.h, e.g. KOGA Koffing 37 / Muk 39 / Koffing 37 / Weezing 43; SABRINA Kadabra 38 / Mr. Mime 37 / Venomoth 38 / Alakazam 43; BLAINE Growlithe 42 / Ponyta 40 / Rapidash 42 / Arcanine 47; LORELEI Dewgong, Cloyster, Slowbro, Jynx, Lapras (five); ROUTE 19 swimmer RICHARD Tentacool 30 / Shellder 30.
- Now: the GEN 3 copy (quetzal_rip/gen2/data/trainers.json, MP_Region_TrainerData) keeps Quetzal's choices with held items: KOGA Weezing / Muk / Crobat / Weezing; SABRINA Girafarig / Mr. Mime / Alakazam / Alakazam; BLAINE Ninetales / Rapidash / Arcanine / Rapidash; LORELEI six including ARTICUNO 53; BRUNO three STEELIX; AGATHA five GENGAR + GYARADOS; LANCE four DRAGONITE; RICHARD Tentacruel x2 (GEN 4+ use Quetzal's Gen 4-9 teams: Drapion, Rotom-Heat, Gallade...).
- Restore: FireRed's parties for every KANTO trainer at GEN 3 (as tools/moemon_starters/frlg_rival.py already does for BLUE's 24 battles), leaving the GEN 5+ pools for the modern setting if the user wants them.
- Notes: a big, region-wide change; ask the user (REGIONS_TODO only says "trainer matches", i.e. the trainers are where FireRed has them). Confidence high.

### K4-13: SAFFRON's COPYCAT (the MIMIC tutor) still says "Move training is handled directly from your MOéMON team menu."
- Maps: Kanto_SaffronCity_CopycatsHouse_2F
- Category: B
- Original: FR: data/maps/SaffronCity_CopycatsHouse_2F/scripts.inc:20-31 + data/scripts/move_tutors.inc:339-370: with a POKé DOLL she teaches MIMIC once and takes the doll ("Oh wow! A POKé DOLL! For me? Thank you! ...").
- Now: data/maps/Kanto_SaffronCity_CopycatsHouse_2F/scripts.inc:23-31 and 58-60: with a POKé DOLL, Quetzal_Text_8471CE4 (data/scripts/qz/kanto_shared.inc:2370) "Move training is handled directly from your MOéMON team menu." — Kai has no such menu.
- Restore: FireRed's MIMIC tutor on Kai's flow like the other restored tutors (TUTOR_MOVE_MIMIC 7 exists, include/constants/party_menu.h:22; flag 0x2221 is already reserved in fix_move_tutors.py FLAGS) plus `removeitem ITEM_POKE_DOLL`; the doll is on sale in every badge mart.
- Notes: REGIONS_TODO "Decisions" lists the COPYCAT as a Quetzal stand-in kept for now; the stand-in line itself is wrong for Kai either way. Confidence high.

### K4-14: Every KANTO POKéMON CENTER 2F has three people explaining Quetzal's "MPLAYER" multiplayer
- Maps: Kanto_CinnabarIsland_PokemonCenter_2F, Kanto_FuchsiaCity_PokemonCenter_2F, Kanto_IndigoPlateau_PokemonCenter_2F, Kanto_SaffronCity_PokemonCenter_2F (and the 15 other KANTO PC 2Fs: shared script)
- Category: B
- Original: FireRed's PC 2F has the UNION ROOM / TRADE CENTER / colosseum attendants and the MYSTERY GIFT man (link features).
- Now: data/scripts/qz/kanto_shared.inc:11-62 -> texts 2064-2100: "Start multiplayer from the START menu! Choose MPLAYER ... Your game and cheat modes must match ... In Nuzlocke, your trade clause must allow trading ... enable COOP battles ... in OPTIONS". Kai has no MPLAYER entry, cheat modes or Nuzlocke; its co-op is the CO-OP menu / HOST A GAME.
- Restore: FireRed's attendants can't work (no link rooms); either remove the three people or give them lines about Kai's own CO-OP (the user's call: Kai's systems come first).
- Notes: confidence high.

### K4-15: KANTO's marts sell Quetzal's badge lists (MASTER BALL, MAX REVIVE, every Gen 4 ball...) instead of FireRed's per-town stock
- Maps: Kanto_CinnabarIsland_Mart, Kanto_FuchsiaCity_Mart, Kanto_SaffronCity_Mart, Kanto_IndigoPlateau_PokemonCenter_1F
- Category: C (touches Kai's own marts)
- Original: FR: CINNABAR Ultra Ball, Great Ball, Hyper Potion, Revive, Full Heal, Escape Rope, Max Repel; FUCHSIA Ultra Ball, Great Ball, Super Potion, Revive, Full Heal, Max Repel; SAFFRON Great Ball, Hyper Potion, Revive, Full Heal, Escape Rope, Max Repel; INDIGO Ultra Ball, Great Ball, Full Restore, Max Potion, Revive, Full Heal, Max Repel (each map's scripts.inc `*_Items`).
- Now: `pokemart 0x0` in all four (e.g. data/maps/Kanto_SaffronCity_Mart/scripts.inc:22) -> src/data/qz_badge_marts.h by badge count; with 8 badges that is 36 items including MASTER BALL, MAX REVIVE, MAX ELIXIR, FULL RESTORE, DUSK / QUICK / TIMER / HEAL BALLs and POKé DOLL.
- Restore: FireRed's list per mart (and FireRed's for the other KANTO marts in other areas); keep Kai's GENERATION item filtering on top.
- Notes: the brief lists "Kai's own marts" as a Kai system: check whether HOENN's marts also go by badges in Kai before changing. Confidence high.

### K4-16: Quetzal's overworld legendaries in this area (most kept by decision), and dead ARCEUS trial objects
- Maps: Kanto_SaffronCity (TAPU LELE), Kanto_SaffronCity_Dojo (KUBFU), Kanto_SilphCo_3F (MAGEARNA), Kanto_SeafoamIslands_1F (NIHILEGO), _B2F (KYUREM), Kanto_PokemonMansion_3F (GUZZLORD), _B1F (TYPE: NULL), Kanto_SafariZone_East (BUZZWOLE), _North (XERNEAS), Kanto_Route20 (THUNDURUS + an ARCEUS trial), Kanto_Route21_North (LATIAS), _South (LATIOS), Kanto_Route23 (IRON BOULDER + an ARCEUS trial), Kanto_VictoryRoad_2F (VICTINI), _3F (RESHIRAM, ZEKROM)
- Category: C
- Original: none of these stand in FireRed's maps (objdiff: all are extra objects with Quetzal_EventScript_840FBBD / 840FDF6).
- Now: shown after KANTO's LEAGUE at their GENERATION (src/qz_specials.c RefreshStatics, src/data/qz_static_encounters.h). LATIAS / LATIOS (HOENN) are hidden for good; the ROUTE 20 / 23 ARCEUS trials (SINNOH's ARCEUS) are hidden for good too (MP_LegendForeign), so their objects are dead.
- Restore: keep, per docs/REGIONS_TODO.md Decisions ("Gens 5-9 legendaries have no home region here and stay"); optional clean-up: delete the dead LATIAS / LATIOS / ARCEUS objects.
- Notes: listed so the user sees the full set; GROUDON / KYOGRE are K4-2. Confidence high.

### K4-17: The rival is always "BLUE": FireRed's {RIVAL} lines are hard-coded
- Maps: Kanto_FuchsiaCity_Gym, Kanto_SaffronCity_Gym, Kanto_CinnabarIsland_Gym (statues), Kanto_SilphCo_7F, Kanto_VictoryRoad_2F, Kanto_PokemonLeague_LancesRoom, _ChampionsRoom
- Category: D (FireRed lets the player name the rival)
- Original: {RIVAL} in all of them (e.g. FR: data/maps/SilphCo_7F/text.inc RivalWhatKeptYou, VictoryRoad_2F NelsonPostBattle, gym statues "WINNING TRAINERS: {RIVAL}").
- Now: "BLUE" written in: Kanto_FuchsiaCity_Gym/scripts.inc:261,267; Kanto_SaffronCity_Gym:273,279; Kanto_CinnabarIsland_Gym:897,903; Kanto_SilphCo_7F:404,407; Kanto_VictoryRoad_2F:199; Kanto_PokemonLeague_LancesRoom:209; _ChampionsRoom:189,207,256,262. KANTO has no rival name (src/string_util.c:472 ExpandPlaceholder_RivalName only knows JOHTO's).
- Restore: give KANTO the rival naming JOHTO got (OAK's lab: another area) and put {RIVAL} back in these lines; the battles' trainer name the same way.
- Notes: docs/quetzal/KANTO.md quirk "The rival's name is hard-coded as BLUE everywhere". Confidence high.

### K4-18: SILPH CO. 5F hidden PP UP is a NUGGET
- Maps: Kanto_SilphCo_5F
- Category: C
- Original: FR: data/maps/SilphCo_5F/map.json hidden item (32,9) ITEM_PP_UP.
- Now: data/maps/Kanto_SilphCo_5F/map.json:321 (32,9) ITEM_NUGGET.
- Restore: ITEM_PP_UP.
- Notes: the only hidden-item difference in K4. Confidence high.

### K4-19: CINNABAR GYM's last quiz question is reworded
- Maps: Kanto_CinnabarIsland_Gym
- Category: E (C)
- Original: "TM28 contains TOMBSTONY?" (FR: data/maps/CinnabarIsland_Gym/text.inc:180; answer NO).
- Now: "The move TOMBSTONY exists?" (data/maps/Kanto_CinnabarIsland_Gym/scripts.inc:933), Quetzal's no-TM wording; TMs are back (TM28 DIG from the CERULEAN grunt), so FireRed's question works again.
- Restore: FireRed's line.
- Notes: confidence high.

### K4-20: A few trainers' lines rewritten by Quetzal
- Maps: Kanto_Route19, Kanto_Route21_North, Kanto_SaffronCity_Gym
- Category: C
- Original: ROUTE 19 CONNIE post-battle "If I were to ride a POKéMON on the sea, I'd like a LAPRAS. LAPRAS is so big, I bet it would keep me dry on water." (FR: Route19/text.inc:110); ROUTE 21 WADE defeat "Heheh, MAGIKARP just don't make the grade, do they?" / post-battle "I seem to only catch MAGIKARP!" (FR: Route21_North/text.inc:16, 20); SAFFRON GYM JOHAN "PSYCHIC POKéMON fear only ghosts and bugs!" (FR: SaffronCity_Gym/text.inc:64).
- Now: "When I get tired, I ride LAPRAS. It's so big that I can cross the sea without getting wet." (data/maps/Kanto_Route19/scripts.inc:274); "A big haul isn't everything!" / "I keep catching different fish!" (Kanto_Route21_North/scripts.inc:85, 88); "BUG, GHOST, and DARK moves can threaten PSYCHIC MOéMON." (Kanto_SaffronCity_Gym/scripts.inc:184).
- Restore: FireRed's lines (WADE's only fits if his team is FireRed's MAGIKARP: see K4-12).
- Notes: everything else on Routes 19-21 / 23 (including the rematch lines) is FireRed's. Confidence high.

### K4-21: SEAFOAM B4F's current doesn't take the player off their POKéMON
- Maps: Kanto_SeafoamIslands_B4F
- Category: A (minor)
- Original: FR special SeafoamIslandsB4F_CurrentDumpsPlayerOnLand (FR-gh: src/field_specials.c:1603-1610) runs CreateStopSurfingTask(DIR_NORTH): the swept player hops onto land.
- Now: data/maps/Kanto_SeafoamIslands_B4F/scripts.inc:87 (8412DE8) calls QzSeafoamCurrentEffect, a stub returning 0 (src/qz_specials.c:1084); the player stays surfing at the shore.
- Restore: QzSeafoamCurrentEffect = the FireRed body (Emerald has the stop-surfing task).
- Notes: low impact (Emerald dismounts when walking onto land); matters once K4-1 makes the puzzle playable. Confidence medium.

### K4-22: Dead Quetzal leftovers (no effect in game, clean-up)
- Maps: Kanto_CinnabarIsland_PokemonLab_Entrance, Kanto_FuchsiaCity_WardensHouse, Kanto_SafariZone_SecretHouse
- Category: E
- Original: n/a.
- Now: CINNABAR LAB entrance keeps the unreachable plate shop / "ARCEUS trials Completed: n/17" script and its 16-PLATE mart after FireRed's scientist (data/maps/Kanto_CinnabarIsland_PokemonLab_Entrance/scripts.inc:24-105); unused strings "{PLAYER} received BLACK AUGURITE from the WARDEN." (Kanto_FuchsiaCity_WardensHouse/scripts.inc:114) and "{PLAYER} received a PRISM SCALE from the attendant!" (Kanto_SafariZone_SecretHouse/scripts.inc:38).
- Restore: delete them (keeps text searches and future audits honest).
- Notes: confidence high.

---

## Coverage

| Map | Result |
|---|---|
| Kanto_CinnabarIsland | K4-3 (otherwise FireRed's: BILL's SEVII scenes, gym lock) |
| Kanto_CinnabarIsland_Gym | K4-8, K4-12, K4-17, K4-19 |
| Kanto_CinnabarIsland_Mart | K4-15 |
| Kanto_CinnabarIsland_PokemonCenter_1F | only trivial differences (Fame Checker journal removed by decision) |
| Kanto_CinnabarIsland_PokemonCenter_2F | K4-14 |
| Kanto_CinnabarIsland_PokemonLab_Entrance | K4-22 (FireRed's scientist already back) |
| Kanto_CinnabarIsland_PokemonLab_ExperimentRoom | K4-9, K4-10 |
| Kanto_CinnabarIsland_PokemonLab_Lounge | K4-9 |
| Kanto_CinnabarIsland_PokemonLab_ResearchRoom | same as the original (METRONOME tutor restored) |
| Kanto_CinnabarIsland_PrimalChamber | K4-2 |
| Kanto_FuchsiaCity | K4-3 (SUBSTITUTE tutor restored) |
| Kanto_FuchsiaCity_Gym | K4-8 (receive line only), K4-12, K4-17 |
| Kanto_FuchsiaCity_House1 | same as the original |
| Kanto_FuchsiaCity_House2 | same as the original (GOOD ROD) |
| Kanto_FuchsiaCity_House3 | K4-4 |
| Kanto_FuchsiaCity_Mart | K4-15 |
| Kanto_FuchsiaCity_PokemonCenter_1F | same as the original |
| Kanto_FuchsiaCity_PokemonCenter_2F | K4-14 |
| Kanto_FuchsiaCity_SafariZone_Entrance | K4-5 |
| Kanto_FuchsiaCity_SafariZone_Office | same as the original |
| Kanto_FuchsiaCity_WardensHouse | K4-22 (HM04 restored; only a dead string) |
| Kanto_IndigoPlateau_Exterior | K4-3 (credits scene FireRed's) |
| Kanto_IndigoPlateau_PokemonCenter_1F | K4-11, K4-15 |
| Kanto_IndigoPlateau_PokemonCenter_2F | K4-14 |
| Kanto_PokemonLeague_AgathasRoom | K4-11, K4-12 |
| Kanto_PokemonLeague_BrunosRoom | K4-11, K4-12 |
| Kanto_PokemonLeague_ChampionsRoom | K4-11, K4-17 (BLUE's teams are FireRed's: sQzBlueBattles) |
| Kanto_PokemonLeague_HallOfFame | only trivial differences (Quetzal re-sets FLAG_DEFEATED_CHAMP after FireRed's reset; nothing reads it) |
| Kanto_PokemonLeague_LancesRoom | K4-11, K4-12, K4-17 |
| Kanto_PokemonLeague_LoreleisRoom | K4-11, K4-12 |
| Kanto_PokemonMansion_1F | same as the original |
| Kanto_PokemonMansion_2F | same as the original |
| Kanto_PokemonMansion_3F | K4-16 |
| Kanto_PokemonMansion_B1F | K4-2 (the chamber warp), K4-16 |
| Kanto_Route19 | K4-12, K4-20 |
| Kanto_Route20 | K4-1 (resets the SEAFOAM puzzle), K4-12, K4-16 |
| Kanto_Route21_North | K4-12, K4-16, K4-20 |
| Kanto_Route21_South | K4-12, K4-16 |
| Kanto_Route23 | K4-16 (badge guards FireRed's) |
| Kanto_SafariZone_Center | K4-5 |
| Kanto_SafariZone_Center_RestHouse | same as the original |
| Kanto_SafariZone_East | K4-5, K4-16 |
| Kanto_SafariZone_East_RestHouse | same as the original |
| Kanto_SafariZone_North | K4-5, K4-16 |
| Kanto_SafariZone_North_RestHouse | K4-5 |
| Kanto_SafariZone_SecretHouse | K4-22 (HM03 restored; only a dead string) |
| Kanto_SafariZone_West | same as the original |
| Kanto_SafariZone_West_RestHouse | same as the original |
| Kanto_SaffronCity | K4-3, K4-6 (sign), K4-16 (MAGNET TRAIN attendant is Kai's) |
| Kanto_SaffronCity_Connection | same as the original |
| Kanto_SaffronCity_CopycatsHouse_1F | same as the original |
| Kanto_SaffronCity_CopycatsHouse_2F | K4-13 |
| Kanto_SaffronCity_Dojo | K4-16 (HITMON gift FireRed's) |
| Kanto_SaffronCity_Gym | K4-8, K4-12, K4-17, K4-20 |
| Kanto_SaffronCity_House | same as the original |
| Kanto_SaffronCity_Mart | K4-15 |
| Kanto_SaffronCity_MrPsychicsHouse | same as the original (TM29 restored) |
| Kanto_SaffronCity_PokemonCenter_1F | only trivial differences (Kai's TRIPLE TRIAD seller / CARD QUEEN; journal removed) |
| Kanto_SaffronCity_PokemonCenter_2F | K4-14 |
| Kanto_SaffronCity_PokemonTrainerFanClub | K4-6 |
| Kanto_SeafoamIslands_1F | K4-1, K4-16 |
| Kanto_SeafoamIslands_B1F | K4-1 |
| Kanto_SeafoamIslands_B2F | K4-1, K4-16 |
| Kanto_SeafoamIslands_B3F | K4-1 |
| Kanto_SeafoamIslands_B4F | K4-1, K4-21 (ARTICUNO Lv 50 as FireRed) |
| Kanto_SilphCo_10F | same as the original |
| Kanto_SilphCo_11F | same as the original (MASTER BALL, GIOVANNI) |
| Kanto_SilphCo_1F | K4-7 |
| Kanto_SilphCo_2F | same as the original (THUNDER WAVE tutor restored) |
| Kanto_SilphCo_3F | K4-16 |
| Kanto_SilphCo_4F | same as the original |
| Kanto_SilphCo_5F | K4-18 |
| Kanto_SilphCo_6F | same as the original |
| Kanto_SilphCo_7F | K4-17 (LAPRAS gift FireRed's) |
| Kanto_SilphCo_8F | same as the original |
| Kanto_SilphCo_9F | same as the original |
| Kanto_SilphCo_Elevator | only trivial differences (Quetzal's floor menu on Emerald's elevator window; not run in game) |
| Kanto_VictoryRoad_1F | same as the original |
| Kanto_VictoryRoad_2F | K4-1 (the 3F boulder can't drop here), K4-16, K4-17 (DOUBLE-EDGE tutor restored) |
| Kanto_VictoryRoad_3F | K4-1, K4-16 |

Shared scripts touched: data/scripts/qz/kanto_shared.inc (K4-3 Jessie & James, K4-13 the tutor stand-in text, K4-14 PC 2F texts, K4-16 the static encounter scripts), data/scripts/qz/shared.inc (K4-3 texts; the NUZLOCKE gift check Quetzal_EventScript_8361A4D is dead: QzIsNuzlockeActiveInRegion always FALSE).
