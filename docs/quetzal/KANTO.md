# Quetzal's Kanto, compared with FireRed/LeafGreen and Gen 1

Porting notes for the Kanto region of Moemon Quetzal, ripped to `quetzal_rip/`. Each section compares
the rip with pret/pokefirered (FRLG) and, where a Gen 1 fan would notice, with pret/pokered (RBY).

Conventions:
- Map names are the rip's `Kanto_<FRLG map>` folders. Labels are the rip's `<Map>_EventScript_<ROM addr>`.
  Shared scripts are in `quetzal_rip/scripts/kanto_shared.inc` (`Quetzal_*`) and `shared.inc`.
- **(inference)** marks a conclusion drawn from code shape, names or data rather than from a script that
  says so outright.
- Flag/var names below are the FRLG names that `tools/quetzal_rip/kanto_frlg_flags.json` maps the
  `FLAG_QZ_` / `VAR_QZ_` ids to (see "Flag and var mapping" at the end).

Rip quirks to know first:
- `{MUS_RG_VS_TRAINER}` inside `.string` lines is the player-name code (`{PLAYER}`), decoded wrongly by the
  rip's charmap. The rival's name is hard-coded as `BLUE` everywhere. FRLG uses `{RIVAL}`.
- **Map names 38.112-116 are off by one.** Quetzal dropped FRLG's `SevenIsland_UnusedHouse`, not
  `SevenIsland_House_Room1`. So the rip's `Kanto_SevenIsland_House_Room1` is really House_Room1,
  `..._Mart` is really House_Room2, `..._PokemonCenter_1F` is really the Mart, `..._PokemonCenter_2F`
  is really the PC 1F and `..._UnusedHouse` is really the PC 2F. Sizes, layouts, objects and warps all
  prove it. These are the only 4 Kanto maps whose size differs from FRLG's. The fix is listed in
  `kanto_frlg_flags.json` → `map_name_corrections`.
- At the time of writing, stock `special` names in the rip (not `QZ_SPECIAL_n`) were shifted by one
  entry in pokeemerald's specials table. Don't read meaning into a stock special's name (for example
  `ShowDaycareLevelMenu` where FRLG has `GetBattleOutcome`). For QZ specials whose call sites line up with
  FRLG specials, see `qz_specials_frlg_counterpart` in the JSON.
- Item ids in scripts are mostly raw numbers (`setorcopyvar VAR_0x8000, 0x1D0`), in the hack's own item
  table.

---

## 1. How Quetzal's Kanto relates to FRLG

**What is identical:**
- The map set and order: groups 36-39 are FRLG's groups minus the link rooms. All 415 Kanto maps except
  the 3 new ones have an FRLG source, and all but the 4 misnamed Seven Island maps match FRLG's size.
- The tilesets are FRLG's. The one exception is `Kanto_CinnabarIsland_PrimalChamber`, which uses stock Emerald
  cave tilesets (`tilesets.json`).
- The story skeleton is FRLG's, in the same order with the same scenes: Oak's lab, Viridian parcel,
  Brock, Mt. Moon fossils, Bill, S.S. Anne, Rock Tunnel, Pokémon Tower, Celadon/Rocket Hideout, Silph Co.,
  Saffron, Fuchsia, Cinnabar, Viridian, Victory Road, E4, then the Sevii Islands post-game (Bill/Celio,
  Ruby, Sapphire, Lorelei, Rocket Warehouse, Lostelle, Selphy, Tanoby). About 90% of Kanto text is
  verbatim FRLG; only ~300 strings differ.
- The scripts are FRLG's, recompiled with renumbered flags and vars. Hidden items keep FRLG's
  hidden-item ids (183/183 match).
- Static encounters keep their FRLG levels: Snorlax ×2 Lv30, Electrode ×2 Lv34, Hypno Lv30, Mewtwo Lv70,
  Lugia/Ho-Oh Lv70, Deoxys, the Seafoam/Power Plant/Mt. Ember birds. The scripts use Quetzal's legendary
  battle path (`QZ_SPECIAL_755/756`, the FRLG `StartLegendaryBattle` slot).

**What changed in general:**
1. **No TMs or HMs.** No `ITEM_TM*`/`ITEM_HM*` is left in any Kanto script or item ball.
   - "Moves can be managed directly from the POKéMON team menu" and "Field techniques are unlocked by
     earning BADGES" (`Kanto_CeladonCity_Condominiums_RoofRoom`, `Kanto_Route14` Carter).
   - Every TM/HM gift became an evolution item or a held item (table in §2).
   - Every FRLG move tutor was removed: Mega Punch/Kick, Dream Eater, Seismic Toss, Counter, Softboiled,
     Thunder Wave, Mimic, Substitute, Rock Slide, Double-Edge, Explosion, Swords Dance, Body Slam, the Cape
     Brink ultimate moves and the Two Island Move Maniac.
   - The Move Deleter (`Kanto_FuchsiaCity_House3`) and the Name Rater (`Kanto_LavenderTown_House2`) now
     only say that the summary screen and party menu do their job.
2. **FRLG systems removed:**
   - Fame Checker journals (the `PokemonJournal*` objects are gone from every Pokémon Center).
   - Mystery Gift delivery men on every PC 2F.
   - Teachy TV (Viridian old man), the Running Shoes aide in Pewter, VS Seeker (§2 Vermilion).
   - Berry Powder (Cerulean House 1/5), the Berry news man (Cerulean House 4), Daisy's grooming.
   - The Magikarp and Heracross size records (now "show me one, get a gift").
   - Union Room / Wireless / Direct Corner attendants, Trainer Tower ("Under construction...",
     `Kanto_SevenIsland_TrainerTower`), and the Joyful Game Corner games.
   - Quest Log and Fame Checker calls became `QZ_SPECIAL_737`, probably a stub (inference).
3. **Safari Zone rules:** "Battles work normally, so bring your own POKé BALLS. There is no fee or time
   limit." (`Kanto_FuchsiaCity_SafariZone_Entrance`).
4. **Mixed generations everywhere:**
   - All 132 Kanto wild tables were rebuilt with Emerald's 12 land slots and 697 different species from
     Gen 1-9, including regional forms (`RATTATA_2`, `MAROWAK_2`, `TAUROS_2..4`, `PIKACHU_17`...).
   - Example, Route 1: Pidgey, Rattata, Bunnelby, Eevee, Fletchling, Lechonk, Lillipup, Scorbunny, Shinx,
     Sprigatito, Wooloo, Yamper (FRLG: Pidgey, Rattata).
   - Example, Altering Cave: only Smeargle Lv50.
   - Water and fishing tables are rebuilt the same way.
5. **Trainers and teams (`data/trainers.json`):**
   - Kanto trainers are ids 8192+ (0x2000+). A trainer's "defeated" flag is its own id
     (`checktrainerflag 0x22E7` = `TRAINER_QZ_8935`).
   - Story Gym Leaders keep FRLG's party size, levels and roles, with modern species and held items
     (table in §2).
   - Each leader and the E4/Champion also have a second, much stronger 6-Pokémon team full of
     legendaries, Z-crystals and Mega Stones: ids 8192-8204, `used_by_scripts: false`. No Kanto script
     references them, so code must swap them in for rematches (inference).
   - The rival's first variant (Squirtle line: 8518/8524/8618/8621/8624/8627/8630) is reworked with
     items and Mega Blastoise. The other two starter variants stay close to FRLG's.
   - Only the first variant is referenced by scripts, so the starter-based variant is picked in code from
     `VAR_STARTER_MON` (inference).
6. **About 100 legendary and mythical Pokémon stand in the overworld across Kanto and Sevii (§3):**
   - Shared script `Quetzal_EventScript_840FBBD`: `QZ_SPECIAL_758` checks or starts the encounter,
     `QZ_SPECIAL_756` finishes it, and `removeobject VAR_0x8004` removes the Pokémon.
   - Seventeen overworld **Arceus** "trials" use `Quetzal_EventScript_840FDF6`, which sets
     `VAR_0x8001-0x8007 = 6` before `QZ_SPECIAL_540` (probably perfect IVs, inference). Progress is shown at
     the Cinnabar Lab (§2).
7. **Regional Pokédex and multi-region play:**
   - Dex counts and diplomas count "registered … as caught in KANTO" (Oak's rating texts, Celadon game
     designer `QZ_SPECIAL_718`, Saffron Fan Club hint NPCs).
   - "Catches from other regions don't count" and "Taking it out of the PC doesn't count!"
     (`Kanto_SaffronCity_PokemonTrainerFanClub`). A DexNav exists: "Your DexNav bonus carries over between
     regions" (same map; `Kanto_Route17` sign).
   - Oak sets a cross-region dex flag `FLAG_QZ_0xA79` only if it is unset, plus the Kanto copy
     (`FLAG_SYS_POKEDEX_GET` = 0x226A) (`Kanto_PalletTown_ProfessorOaksLab_EventScript_8419C3F`).
8. **How the player gets to Kanto:**
   - Kanto is a full campaign that starts in your own bedroom in Pallet Town: Mom, Oak's lab, starter,
     rival.
   - The only map link to another region is a new warp in `Kanto_Route22_NorthEntrance` to
     `MAP_JOHTO_VICTORY_ROAD_GATE`. An officer blocks it ("Only the champions of both POKéMON LEAGUES may
     cross", `Quetzal_Text_83E90EF`; `QZ_SPECIAL_772` decides, `FLAG_QZ_0x210A` hides him).
   - No non-Kanto script warps into Kanto, so the region is chosen at new game, or reached in code
     (inference; not in these scripts).
9. **Johto and Hoenn tie-ins:**
   - The Johto gate (above).
   - Jessie & James with their Meowth balloon appear in a town at random, in Kanto and Johto alike
     (`Quetzal_EventScript_83C303A`, 4 objects in every town/island; `QZ_SPECIAL_773` decides). They
     battle (`TRAINER_QZ_913`, "Team Rocket's blasting off again!"), then hand over a Lv50 Pikachu-line
     reward (`givemon VAR_QZ_0x514A, 50`, "received Pikachu!").
   - Maxie and Archie under the Pokémon Mansion with Groudon, Kyogre and the Red/Blue Orbs
     (`Kanto_CinnabarIsland_PrimalChamber`).
   - Hoenn's Rayquaza in a new Cerulean Cave room.
10. **Story/state changes:**
    - Beating the league sets `FLAG_DEFEATED_CHAMP` (0x2109) and `FLAG_SYS_GAME_CLEAR` (0x2108) in the
      Hall of Fame (`Kanto_PokemonLeague_HallOfFame`).
    - The Champion rematch check uses GAME_CLEAR where FRLG checks `FLAG_SYS_CAN_LINK_WITH_RS`
      (`Kanto_PokemonLeague_ChampionsRoom`).
    - The Sevii ferry uses a TRI-PASS / RAINBOW PASS destination picker at Vermilion ("Choose a destination
      (up/down)", `Kanto_VermilionCity`). Tickets are reusable ("If you have one, you don't need to buy it
      again", `Kanto_TwoIsland`).

### Differences from Gen 1 that a Gen 1 fan would notice
- FRLG additions still present (not Gen 1): the Sevii Islands, the Ruby/Sapphire quest, Mt. Ember, Lostelle,
  Selphy, Tanoby, Navel Rock, Birth Island, Bill's sister etc. Running Shoes, Teachy TV, VS Seeker, the Fame
  Checker, move tutors and the Trainer Tower are gone, so several FRLG-only extras are removed.
- **Gen 1 legends restored or acknowledged:**
  - Mew is under the truck by the S.S. Anne. Push it with Strength (§2, `Kanto_SSAnne_Exterior`).
  - The S.S. Anne never leaves for good. It returns to its pier and can be revisited, with beds to sleep
    in.
- The rival is always BLUE, the Gen 1 default.
- Gen 1 gym structure (levels, ace levels, gym order) is intact, but the species are modern. Brock still
  leads with a Rock fossil (Lileep) and an Onix, and Erika still ends on Victreebel.
- Gen 1 had no held items, abilities, Mega Evolution or Z-moves. Quetzal Kanto uses all of them.
- No Safari Balls or Safari timer, which existed in both Gen 1 and FRLG.
- Pokémon Tower's ghost is an **Alolan** Marowak (`setwildbattle SPECIES_MAROWAK_2, 30`,
  `Kanto_PokemonTower_6F`).

---

## 2. Location by location (story order)

The "Gen 1" notes only mention what a RBY player would notice.

### Pallet Town
- **Your bed** (`Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B06`):
  - "Would you like to sleep?" heals the party ("You feel rested.").
  - When `QZ_SPECIAL_768` returns 1, **Arceus** appears in the room (object 1, flag `FLAG_QZ_0x2E14`):
    "I have witnessed the bond between you and your Pokémon. Your journey is not yet over. We shall meet
    again."
  - It then sets `FLAG_QZ_0x2E13`, which later unlocks the Arceus trials at the Cinnabar Lab.
  - FRLG's 2F has a PC; Gen 1's has a PC and an SNES.
- **Oak's Lab:**
  - **Starter:** all three balls (objects 5-7, FRLG's Bulbasaur/Squirtle/Charmander balls) run the same
    picker, `QZ_SPECIAL_691` → `QZ_SPECIAL_692` (`..._EventScript_8419AFD`/`8419BB9`). The ball's index
    (0/1/2) goes to `VAR_STARTER_MON` and the species to `VAR_QZ_0x5146`.
  - FRLG's "CHARMANDER/SQUIRTLE is your choice" lines are gone. A nickname prompt follows. The species
    offered come from code, so I can't tell which ones from the scripts (inference: any starter).
  - Oak hands over the **NATIONAL POKéDEX** at once ("Take this NATIONAL POKéDEX with you, too").
  - The Pokédex rating counts Kanto catches only (`Text_8457663`).
  - The first rival battle uses `trainerbattle TRAINER_BATTLE_QZ_13, TRAINER_QZ_8518, 3, …` (a custom
    battle type) against a Lv2 Squirtle / Bulbasaur / Charmander (8518-8520).
  - A **Meltan** stands in the lab (object 11, `Quetzal_EventScript_840FBBD`).
- Rival's House: Daisy's grooming and friendship check are gone. She now explains that the POKéDEX shows
  how each Pokémon evolves: "It doesn't depend on friendship here."
- `Kanto_PalletTown`: the Jessie & James set is placed at (19-21,7-8).

### Route 1 / Viridian City / Route 22 / Route 2 / Viridian Forest
- Viridian: FRLG's Teachy TV lines are cut, but the coffee old man's catching demo is kept.
- Route 22:
  - The early rival line changes from "You collected all the BADGES, too?" to "You probably don't have any
    BADGES, do you?" (`Kanto_Route22_Text_8438B71`).
  - New overworld Azelf, and an Arceus trial.
  - The north gate leads to Johto (§1.8).
- Route 2 East Building (Oak's aide, 10 species): **GALARICA CUFF** instead of HM05 Flash.
- Viridian Forest: **Zacian** and **Zamazenta** at the north end (objects 12/13). The wild table is
  Grookey, Applin, Zorua, Dedenne…

### Pewter City
- **Gym** (`Kanto_PewterCity_Gym`):
  - Brock is `TRAINER_QZ_8606`: Lileep 12, Onix 14 @Eviolite. FRLG: Geodude 12, Onix 14. Gen 1: same as
    FRLG.
  - Reward: **ASSAULT VEST** instead of TM39.
- The Running Shoes aide (FRLG object at 46,20) is removed.
- Shaymin in the overworld.
- Museum:
  - Hoopa (1F) and Jirachi (2F).
  - The fossil displays show a picture (`QZ_SPECIAL_775/776/777`, FRLG `CloseMuseumFossilPic`) with new
    "reconstruction" text.
  - The Seismic Toss tutor is gone.

### Route 3 / Mt. Moon / Route 4
- Route 4 Pokémon Center: the Magikarp salesman remains (`givemon SPECIES_MAGIKARP, 5`).
- Mt. Moon:
  - **Dialga** and **Palkia** on 1F, **Necrozma** on B2F.
  - Wild: Axew, Carbink, Cleffa, Noibat, Tinkatink, Tyrogue…
  - The fossil choice is unchanged in structure (`VAR_MAP_SCENE_MT_MOON_B2F`).
- Route 4: Cresselia in the overworld.
- **New map `Kanto_MtMoon_Route4Cave`:**
  - Entered from Route 4 warp 3 at (58,2), behind the one-way ledges east of Mt. Moon.
  - An empty 21×20 cave: no objects, no wild table, only an exit `setmetatile`
    (`Kanto_MtMoon_Route4Cave_EventScript_8422161`).
  - Unfinished or decorative (inference). Proposed name `MtMoon_Route4Cave`.

### Cerulean City / Routes 24-25 / Bill
- **Rival:** after the battle he gives a **LUCKY EGG** instead of the Fame Checker ("I picked up an extra
  LUCKY EGG").
- **Rocket thief:** he returns a **SUN STONE** instead of TM28 Dig, and the House 2 dialogue changes to
  match.
- **Gym:**
  - Misty is `TRAINER_QZ_8607`: Lanturn 18 @Oran Berry, Starmie 21 @Sitrus Berry. FRLG/Gen 1: Staryu 18,
    Starmie 21.
  - Reward: **LEFTOVERS** (the "restores a little HP" text) instead of TM03.
  - The gym guide's tips mention abilities and dual types.
- Bike Shop:
  - Bikes can't be bought. Its only role is the voucher exchange: "Get a BIKE VOUCHER there and exchange it
    here".
  - Its new text explains the bicycle's MACH and ACRO modes, switched with L.
- House 5 (FRLG's Berry Powder man) now gives a **BERRY SWEET** once.
- House 4 (Berry news man) is replaced by a story-loving NPC.
- Route 25:
  - Overworld **Suicune** and **Enamorus**.
  - Sea Cottage: Poipole. The Eevee-evolution pictures are kept, and so is the Bill teleporter
    (`QZ_SPECIAL_784/785` = FRLG `AnimateTeleporterHousing/Cable`). Bill's "SS Ticket dup" flag is still
    set.
- New Cerulean officer (object at 29,12, `Kanto_CeruleanCity_EventScript_841632B`), and **Tapu Fini**.

### Vermilion City / S.S. Anne / Route 6 / Route 11 / Diglett's Cave
- **New shopping row** (objects 9-16 in `Kanto_VermilionCity`, mart lists `*_Mart_82C...`): ENERGY GURU,
  EFFORT WOMAN, NATURE WOMAN, GIGA COOK, MEGA GURU and Z-MOVE GURU, each saying "I'll go for it and sell you
  what you need!". The construction man and his Machop are removed.
- **Harbor:**
  - Signs and sailor explain the "S.S. ANNE: at the end of the pier" and the island ferry.
  - The ferry needs a TRI-PASS or RAINBOW PASS and has a destination picker (`Text_842FCEF`/`842FD8C`).
  - The S.S. Anne no longer leaves for a year: "The S.S. ANNE returns to this pier. You can visit with your
    TICKET."
- **S.S. Anne:**
  - Captain: "Let me teach you my prized CUT technique. With the CASCADE BADGE…" plus a **METAL COAT**
    (`Kanto_SSAnne_CaptainsOffice`). No HM01.
  - `Kanto_SSAnne_1F_Room6`:
    - The bed asks "Would you like to sleep?" and heals the party.
    - A Darkrai event: "This bed doesn't feel real... The door is still there." (`QZ_SPECIAL_760-763`,
      object 2 = **Darkrai**, `EventScript_8410A6B`).
    - Once you have Cut (`FLAG_GOT_HM01`), the room offers "leave the ship and return to VERMILION?".
  - The departure cutscene (`QZ_SPECIAL_724`, FRLG `DoSSAnneDepartureCutscene`) is kept.
  - Item balls: TM31 (1F Room 2) became another item.
- **Mew under the truck** (`Kanto_SSAnne_Exterior_EventScript_8410248`):
  - Bg events at (55-57,2-3).
  - Prompt: "It's a very heavy truck. Would you like to use Strength?"
  - Checks: `QZ_SPECIAL_764`.
  - The truck moves (`QZ_SPECIAL_766`, `FLAG_QZ_0x2931`) and **Mew** appears (object 2,
    `FLAG_QZ_0x292E`), then is battled through `QZ_SPECIAL_758`.
  - This is unique: Gen 1 only had the rumor, FRLG has nothing.
- **Gym:**
  - Lt. Surge is `TRAINER_QZ_8608`: Rotom-? (`ROTOM_6`) 21, Electivire 18, Raichu 24. FRLG/Gen 1: Voltorb
    21, Pikachu 18, Raichu 24.
  - Reward: **CHOICE SCARF** instead of TM34.
  - The trash-can puzzle is kept (`QZ_SPECIAL_752` = FRLG `SetVermilionTrashCans`).
- Pokémon Center: the VS Seeker giver now gives an **ABILITY PATCH**.
- Fan Club: the Bike Voucher is still there.
- Overworld: Tapu Koko, Celesteela (city), Mesprit (Route 6), Zygarde (Diglett's Cave B1F), plus Arceus
  trials on Routes 6 and 11.

### Route 9 / Rock Tunnel / Route 10 / Power Plant / Lavender Town / Pokémon Tower
- Route 10:
  - Overworld **Raikou**, **Yveltal** and **Zeraora**. Route 2 East's text now says only "ROCK TUNNEL is
    very dark."
  - The Fan Club hint says "return to Route 10 and explore the river. There's a Power Plant…".
- Rock Tunnel B1F: **Terrakion**.
- Power Plant:
  - The Electrode statics are kept. The Zapdos static is kept (the cry text is now "Zapdos!" instead of
    "Gyaoo!").
  - **Xurkitree** added.
- Lavender:
  - The Name Rater is gone (§1).
  - Pokémon Tower: **Marshadow** (3F), **Giratina** (7F) and the Alolan Marowak ghost (6F,
    `QZ_SPECIAL_753` = FRLG `StartMarowakBattle`).
  - New Silph Scope hint: "A spirit blocks the way."

### Celadon City
- **Department Store** reorganised:
  - 2F CATCHING & TRAVEL (Balls, flutes), 3F BATTLE SUPPLIES (held items, gems, X items), 4F BERRIES & CARE
    (Berries, Gracidea, vitamins), 5F FORMS & TREATS (form items, Oricorio nectars, treats).
  - Each clerk asks "Which section would you like?".
  - Roof: FRLG's TM16/20/33 lines are replaced by SWEET APPLE / TART APPLE / SYRUPY APPLE lines, so the
    thirsty girl now gives the apples (inference).
  - 3F Counter tutor removed.
- Condominiums:
  - 3F game designer: diploma only for a full **National Dex registered in Kanto** (`QZ_SPECIAL_718` =
    FRLG `HasAllKantoMons/HasAllMons`).
  - Roof Room: Eevee via `qz_givemon_ex SPECIES_EEVEE, 25`.
- Game Corner:
  - The Pokémon Printer is "out of service".
  - Prize Room: no TMs (`givemon VAR_TEMP_1, 24` for prize Pokémon is kept).
- **Gym:**
  - Erika is `TRAINER_QZ_8609`: Cradily 29, Tsareena 24, Victreebel 29. FRLG/Gen 1: Victreebel 29,
    Tangela 24, Vileplume 29.
  - Reward: **BIG ROOT** (the "draining moves" text) instead of TM19.
- Rocket Hideout:
  - Item balls on B2F replaced: Moon Stone, TM12 and Super Potion moved and changed.
  - **Iron Crown** on B4F.
  - On B1F, a Birch's-bag object runs the overworld-Pokémon check (`Kanto_RocketHideout_B1F_EventScript_840FBDF`).
  - Giovanni's B4F battle is FRLG's.
- **Tapu Bulu** in the city.

### Saffron City / Silph Co.
- Silph Co.:
  - **Magearna** on 3F.
  - The Lapras gift on 7F is kept (`givemon SPECIES_LAPRAS, 25`). Rival on 7F.
  - The Thunder Wave tutor (2F) is removed.
  - "We also sell supplies for your research" (1F).
- Fighting Dojo: the Hitmon choice is kept, as `qz_givemon_ex VAR_TEMP_1, 25`. **Kubfu** stands in the
  dojo.
- Mr. Psychic gives a **MOON STONE** (TM29 gone). The Copycat's Mimic tutor is removed.
- **Gym:**
  - Sabrina is `TRAINER_QZ_8612`: Farigiraf 38, Hatterene 37, Gallade 38, Alakazam 43 @**Alakazite**
    (Mega). FRLG/Gen 1: Kadabra 38, Mr. Mime 37, Venomoth 38, Alakazam 43.
  - Reward: **CHOICE SPECS**. The gym trainer now says BUG, GHOST and DARK beat Psychic, not the Gen 1
    "fear only ghosts and bugs".
- **Pokémon Trainer Fan Club → "KANTO EXPLORERS' CLUB"** (`Kanto_SaffronCity_PokemonTrainerFanClub`):
  - All FRLG fan/battle-count dialogue is replaced by hint NPCs:
    - Kanto dex rules, DexNav, the Route 10 Power Plant.
    - "Some Pokémon even need two compatible pieces" for fossils.
    - Primeape → **Annihilape** (level up knowing a move).
    - The Seven Island boulder cave tied to Tanoby.
    - The Chansey dance (Meloetta).
    - Lorelei's dolls ("starts growing after you beat the Kanto League twice. The last one appears after
      twenty wins").
- **Tapu Lele** in the city.

### Routes 12-18 / Fuchsia City / Safari Zone
- Route 12:
  - Snorlax static kept.
  - Fishing House: "Bring one [MAGIKARP] in your party, and I'll have a gift" (`QZ_SPECIAL_726` = FRLG
    `DoesPlayerPartyContainSpecies`). The size record is gone.
  - North Entrance 2F: the little girl gives a **SHINY STONE** instead of TM27.
- Route 14: Iron Leaves. Carter's TM dialogue is rewritten for "no TMs".
- Route 15: Kartana. West Entrance 2F: the aide's Exp. Share is kept, with new item text.
- Route 16:
  - Snorlax static kept.
  - The house girl gives a **RAZOR FANG** instead of HM02 Fly. Fly comes from the badge.
- Routes 16-18 (Cycling Road):
  - You need a bicycle to enter, but "Once inside, you can get off your bike and explore on foot".
  - The Route 17 sign is about DexNav sneaking instead of slopes.
  - Koraidon and Miraidon on Route 17, Tornadus on Route 18.
- Fuchsia:
  - **Gym:**
    - Koga is `TRAINER_QZ_8610`: Weezing 37, Drapion 39, Crobat 37 @Flyinium Z, Scolipede 43.
      FRLG/Gen 1: Koffing 37, Muk 39, Koffing 37, Weezing 43.
    - Reward: **BLACK SLUDGE** instead of TM06.
  - Warden's House: the Gold Teeth reward is **BLACK AUGURITE** (HM04 Strength gone).
  - Move Deleter gone (§1). The Substitute tutor is removed.
- Safari Zone:
  - Normal battles, no fee, no Safari Balls, no time.
  - Secret House prize: **PRISM SCALE** (HM03 Surf gone).
  - **Buzzwole** (East) and **Xerneas** (North).
  - The wild table mixes Kangaskhan, Tauros, Chansey and Miltank with Gen 3-9 species.

### Seafoam Islands / Cinnabar Island / Route 19-21
- Seafoam:
  - **Nihilego** on 1F and **Kyurem** on B2F.
  - The Articuno static (B4F) and the current/boulder puzzle are kept (`QZ_SPECIAL_749` = FRLG
    `SeafoamIslandsB4F_CurrentDumpsPlayerOnLand`).
  - The Rock Slide tutor is removed.
- Routes 21: Latias (North) and Latios (South).
- **Pokémon Mansion:** Guzzlord (3F), Type: Null (B1F). Its B1F has a new warp to
  `Kanto_CinnabarIsland_PrimalChamber`.
- **New map `Kanto_CinnabarIsland_PrimalChamber`** (proposed name `CinnabarIsland_PrimalChamber`):
  - 52×20, built on Emerald cave tilesets, with Cave of Origin music.
  - **MAXIE** (`TRAINER_QZ_8935`, six Lv70 Pokémon, Mega Camerupt) battles
    (`Kanto_CinnabarIsland_PrimalChamber_EventScript_8422180`). Then a shake, the fall-warp effect and
    **Groudon** appears. He gives the **Red Orb** (item 0x122).
  - **ARCHIE** (`TRAINER_QZ_8936`, Lv70, Mega Sharpedo) does the same with **Kyogre** and the **Blue Orb**
    (0x123) (`…_8422242`).
  - Both can be rematched afterwards.
  - Not in FRLG or Gen 1.
- **Cinnabar Lab:**
  - Entrance: "We also sell plates for your research" (the mart sells the type plates).
  - The **Arceus trials** counter: "ARCEUS trials Completed: n/17", and once all are done "Only the
    challenge of Normal-type ARCEUS remains" (`Kanto_CinnabarIsland_PokemonLab_Entrance`,
    `QZ_SPECIAL_781/829`). It needs `FLAG_QZ_0x2E13` from the Pallet bedroom scene.
  - Experiment Room:
    - Fossil revival is rewritten (`QZ_SPECIAL_561-567`) for many fossils, including two-piece Galar
      fossils (Fan Club hint).
    - The "take a walk" wait is gone: "Your fossil is back to life!".
    - The Metronome tutor is removed.
- **Gym:**
  - Blaine is `TRAINER_QZ_8611`: Ninetales 42 @Heat Rock, Rotom-Heat (`ROTOM_2`) 40, Arcanine 42, Chandelure
    47. FRLG/Gen 1: Growlithe 42, Ponyta 40, Rapidash 42, Arcanine 47.
  - Reward: **LIFE ORB** instead of TM38.
  - Quiz question 6 asks "The move TOMBSTONY exists?" (FRLG: "TM28 contains TOMBSTONY?").

### Viridian Gym / Route 23 / Victory Road / Indigo Plateau
- **Viridian Gym:**
  - Giovanni is `TRAINER_QZ_8542`: Garchomp 45, Gastrodon 42, Nidoking 44, Rhyperior 45, Excadrill 50.
    FRLG/Gen 1: Rhyhorn 45, Dugtrio 42, Nidoqueen 44, Nidoking 45, Rhydon 50.
  - Reward: **CHOICE BAND** instead of TM26.
- Route 23: the badge checks use the same shared scripts (`Quetzal_EventScript_8457E31` and the
  `8457E9B..` badge branches). Iron Boulder and an Arceus trial are here.
- Victory Road: **Victini** (2F), **Reshiram** and **Zekrom** (3F). The Double-Edge tutor is removed.
- **Elite Four / Champion** (story teams; FRLG first-run teams for comparison):

  | Member | Quetzal (story) | FRLG first run |
  |---|---|---|
  | Lorelei 8602 | Alolan Ninetales 52, Arctozolt 51, Galarian Darmanitan 52, Crabominable 54, Articuno 53, Lapras 54 | Dewgong, Cloyster, Slowbro, Jynx, Lapras |
  | Bruno 8603 | Scrafty 51, Sneasler 53, Gallade 53, Lucario 54, Keldeo 55, Machamp 56 | Onix, Hitmonchan, Hitmonlee, Onix, Machamp |
  | Agatha 8604 | Gholdengo 54, Mimikyu 54, Spiritomb 53, Sinistcha 56, Basculegion 55, Gengar 58 @Gengarite | Gengar, Golbat, Haunter, Arbok, Gengar |
  | Lance 8605 | Archaludon 56, Kingdra 54, Tyrantrum 54, Naganadel 58, Haxorus 57, Dragonite 60 | Gyarados, Dragonair ×2, Aerodactyl, Dragonite |
  | BLUE 8630 (Squirtle variant) | Rillaboom 59, Alakazam 57, Arcanine 59 @Firium Z, `EXCADRILL_2` 59, Salamence 61, Blastoise 63 @Blastoisinite | Pidgeot, Alakazam, Rhydon, Arcanine/Gyarados/Exeggutor, starter |

  - Every story team grew to 6 Pokémon while keeping FRLG's levels.
  - The rematch teams (8200-8204) are almost all legendaries, for example Lance with Zygarde, Dialga,
    Palkia, Rayquaza, Kyurem and Garchomp.
  - The credits are started from `Kanto_IndigoPlateau_Exterior` through `QZ_SPECIAL_723` (FRLG `DoCredits`).

### Sevii Islands (post-game)
The FRLG Sevii plot is intact. Changes:

- **One Island:**
  - Bill gives a **TRI-PASS**, then trades it for the **RAINBOW PASS** later.
  - The "METEORITE" is a "rock sample" in the Two Island scenes ("agrees to deliver BILL's rock sample").
  - Kindle Road: Arceus trial. Ember Spa: **Volcanion**; the spa man gives a **PROTECTOR** (HM06 Rock Smash
    gone).
  - Treasure Beach: **Manaphy**.
- **Two Island:**
  - The shop man sells reusable tickets for special voyages, available after the League and the Rainbow
    Pass. FRLG's expanding-shop chain is replaced; its flags are still mapped.
  - Joyful Game Corner: no games, now a rest area with record mixing ("share stories with other
    TRAINERS").
  - The Lostelle rescue is kept.
  - Cape Brink: the ultimate-move tutor is gone; **Walking Wake** and **Uxie** stand there.
  - Pecharunt and Blacephalon in town. The Move Maniac (House) is removed.
- **Three Island:**
  - Bikers are kept.
  - Overworld: Okidogi (town), Fezandipiti (Bond Bridge), Calyrex and Munkidori (Berry Forest; the Hypno
    Lv30 static is kept).
- **Four Island:**
  - Day Care: FRLG's version is kept here.
  - Route 5's Day Care is now a **mart** (`Kanto_Route5_PokemonDayCare_EventScript_846C8C6` →
    `Quetzal_EventScript_83F0976`).
  - Icefall Cave: Chien-Pao (entrance) and **Glastrier** (B1F). Lorelei's scenes are kept.
  - Lorelei's dolls grow with league wins (`QZ_SPECIAL_774` = FRLG `UpdateLoreleiDollCollection`; Fan Club
    hint).
  - "I collect stickers" NPC in House 2.
- **Five Island:**
  - Meadow: **Cobalion**. Memorial Pillar: **Galarian Moltres** (gfx id → species 977, a form id;
    inference) plus an Arceus trial.
  - Lost Cave: **Spectrier**. Rocket Warehouse: **Genesect**.
  - Resort Gorgeous: **Galarian Articuno** (species 975) and **Pheromosa**.
  - **Water Labyrinth:**
    - The Togepi **egg** became a hatched Togepi: "You received a special POKéMON!"
      (`qz_givemon_ex SPECIES_TOGEPI, 5`).
    - The egg man's friendship check is gone.
    - **Keldeo** stands here.
  - **Selphy** (`Kanto_FiveIsland_ResortGorgeous_House`): the one-time "show me X" quest became a **daily
    request**.
    - "I'll reward you for showing me a Pokémon. One reward per day!"
    - Requests can be cancelled, and the reward waits if your bag is full.
    - Requests only use species you have registered in Kanto (`QZ_SPECIAL_748/786`).
- **Six Island:**
  - Ruin Valley:
    - The braille door is now "The inscription says CUT. Use CUT on the door?" (`QZ_SPECIAL_671`).
    - Raging Bolt and Wo-Chien.
  - Water Path: Landorus. House 1 Heracross fan: "Bring one in your party … gift" instead of the size
    record.
  - Pattern Bush: Zarude and **Celebi**. Dotted Hole Sapphire Room: **Terapagos**. The braille texts are
    still there.
  - Altering Cave: Smeargle Lv50 only, plus an Arceus trial (object 1).
- **Seven Island:**
  - Sevault Canyon: Gouging Fire, Ting-Lu, Arceus. Entrance: **Galarian Zapdos** (976) and Arceus.
  - **Sevault Canyon House:** the Chansey Dance can summon **Meloetta**: "Oh, it's MELOETTA! It looks like
    it wants to join our CHANSEY Dance!", "MELOETTA performs Relic Song!"
    (`Kanto_SevenIsland_SevaultCanyon_House_EventScript_8421FFD`, `QZ_SPECIAL_759`).
  - Tanoby Ruins: Stakataka outside.
  - **Tanoby chambers:** every one of the 7 chambers holds 6 overlapping objects at the same tile:
    Regirock, Regice, Registeel, Regieleki, Regidrago and Regigigas. Presumably one is shown, chosen by
    flags/code (inference).
  - The Unown wild tables now mix Psychic, Ghost and Steel ruin Pokémon.
  - Old woman's house (rip "House_Room2", really Room1): she gives a set of **Berries** from her travels.
  - Trainer Tower: "Under construction...". A Machamp object is outside (`EventScript_8366160`).
  - FRLG's link-room attendants on the Seven Island PC 2F (rip "UnusedHouse") are kept as Quetzal objects.

### Post-game dungeons
- **Cerulean Cave:**
  - The guard check is kept. **Eternatus** on 1F.
  - Mewtwo static (B1F, Lv70, `QZ_SPECIAL_755` with species 0x96, level 0x46).
  - New room **`Kanto_CeruleanCave_RayquazaChamber`** (from 2F warp 6 at (4,3), 19×25, shade weather, Sevii cave
    music) holds **Rayquaza**. Proposed name `CeruleanCave_RayquazaChamber`.
- **Navel Rock:** Ho-Oh (Summit) and Lugia (Base) statics kept, Lv70.
- **Birth Island:** the Deoxys triangle puzzle is kept.
- **Altering Cave, Tanoby, Mt. Ember:**
  - Mt. Ember: Moltres static kept ("Moltres!"). Entei (exterior), Chi-Yu (Ruby Path B3F), **Diancie**
    (Ruby Path B5F), **Heatran** (Summit Path 3F).
  - The FRLG Explosion tutor is removed.

---

## 3. Unique mechanics (neither Gen 1 nor FRLG)

| Mechanic | Where |
|---|---|
| ~100 overworld legendaries/mythicals (talk → battle → object removed; each object has its own hide flag) | `Quetzal_EventScript_840FBBD` (kanto_shared.inc) + the object list at its header; species from gfx id via `tools/quetzal_rip/pokemon_ow_gfx.json` |
| 17 Arceus type trials + Normal Arceus finale, tracked at Cinnabar Lab, sells plates | `Quetzal_EventScript_840FDF6`; `Kanto_CinnabarIsland_PokemonLab_Entrance`; unlocked by `Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B3D` (`FLAG_QZ_0x2E13`) |
| Sleep-to-heal beds; Arceus bedroom visit | `Kanto_PalletTown_PlayersHouse_2F_EventScript_8418B06`; `Kanto_SSAnne_1F_Room6` |
| Darkrai dream in the S.S. Anne cabin | `Kanto_SSAnne_1F_Room6` (`QZ_SPECIAL_760-763`) |
| Mew under the truck (Strength) | `Kanto_SSAnne_Exterior_EventScript_8410248` |
| Maxie/Archie Lv70 battles → Groudon/Kyogre + Red/Blue Orb, rematchable | `Kanto_CinnabarIsland_PrimalChamber` |
| Jessie & James roaming town event (Kanto + Johto) with Pikachu reward | `Quetzal_EventScript_83C303A` (kanto_shared.inc), `QZ_SPECIAL_773` |
| Starter picker UI | `Kanto_PalletTown_ProfessorOaksLab_EventScript_8419AFD` (`QZ_SPECIAL_691/692`) |
| Champions-only gate to Johto | `Kanto_Route22_NorthEntrance` (`QZ_SPECIAL_772`) |
| Regional-dex rules (Kanto catches only), diploma per region, DexNav | Oak's lab rating texts, `Kanto_CeladonCity_Condominiums_3F`, Fan Club |
| Daily Selphy requests | `Kanto_FiveIsland_ResortGorgeous_House` |
| Meloetta joins the Chansey Dance | `Kanto_SevenIsland_SevaultCanyon_House` |
| All six Regis in each Tanoby chamber | `Kanto_SevenIsland_TanobyRuins_*Chamber` map.json |
| Vermilion specialty shops (EV/nature/Mega/Z-move items, cooking) | `Kanto_VermilionCity` objects 9-16 |
| Ferry destination picker with reusable passes | `Kanto_VermilionCity` (Text_842FCEF/842FD8C), `Kanto_TwoIsland` |
| No TMs/HMs, field moves by badge, move management in the party menu, no move tutors | game-wide (see §1.1) |
| Leader/E4 rematch teams with legendaries/Megas/Z-crystals (8192-8204) | `data/trainers.json` only (no script references) |

---

## 4. Flag and var mapping (deliverable: `tools/quetzal_rip/kanto_frlg_flags.json`)

Because the scripts are FRLG's recompiled, every Kanto script was aligned with its pokefirered twin, and
operands at the same position of the same command were paired. The pairing starts from map script tables,
objects at the same index and x/y, and bg/coord events at the same x/y, then follows goto/call/text labels.
`_method` in the JSON has the details.

- **499 flags and 61 vars** are alignment-backed:
  - Story flags, world-map flags, FLAG_HIDE_* object flags (from `map.json` too), system flags.
  - All `VAR_MAP_SCENE_*` vars, `VAR_STARTER_MON`, and more.
  - Only one conflict: 0x2108, 17 votes for GAME_CLEAR versus 2 for CAN_LINK_WITH_RS, explained above.
  - Two spurious single votes were dropped.
- **Layout found:** Quetzal put FRLG's flags and vars in **alphabetical order of their FRLG names**:
  - 0x2100-0x2107 badges, 0x2108 `FLAG_SYS_GAME_CLEAR`.
  - 0x2109 `FLAG_DEFEATED_CHAMP`, then `FLAG_BEAT_RIVAL_IN_OAKS_LAB` (0x2110) … `FLAG_WORLD_MAP_VIRIDIAN_FOREST`
    (0x2265).
  - 0x226A `FLAG_SYS_POKEDEX_GET`, then the item/object hide flags alphabetically from 0x226B.
  - Vars: 0x5100 `VAR_CABLE_CLUB_STATE` … 0x5145, and 0x5147 `VAR_STARTER_MON`.
  - This makes gaps predictable. `inferred_by_alphabetical_order` holds 23 flags and 6 vars filled where
    the gap size matches exactly. These are **not** alignment-backed.
- Hidden items keep FRLG's hidden-item ids. Kanto trainer flags are the trainer ids (0x2000+).
- Ids outside the Kanto block seen in Kanto scripts:
  - 0xA79/0xA78: cross-region dex/starter flags (inference).
  - Emerald's `VAR_CABLE_CLUB_STATE` (0x4087).
  - Quetzal's own 0x28xx-0x2Exx flags: overworld legendaries, Arceus, orbs, Jessie & James.
- `qz_specials_frlg_counterpart`: 26 `QZ_SPECIAL_n` → FRLG special by call-site alignment, for example
  731 = `GetPokedexCount`, 755/756 = the legendary battle pair, 724 = `DoSSAnneDepartureCutscene`.

Porter's tip: to rename a Quetzal Kanto script back to FRLG names, apply `flags`/`vars`, then diff against
`pokefirered/data/maps/<Map>/scripts.inc`. What remains is Quetzal's real changes.
