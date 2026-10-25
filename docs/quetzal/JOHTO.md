# Moemon Quetzal: Johto compared with Gen 2 (porting notes)

Source: `quetzal_rip/` (Johto = map groups 34 and 35, 249 maps), compared with pret/pokecrystal.
Audience: a developer porting Quetzal's Johto into Emerald Kai.

**How to read this**
- Maps are named by their current `quetzal_rip/maps/` folder without the `Johto_` prefix. When a folder name is a
  placeholder or wrong, the proposed name follows in brackets, e.g. `IcePathB2FBlackthornSide`. All the
  proposed names are in `tools/quetzal_rip/johto_map_names.json`, which also gives the map's group.num.
- Labels are the rip's `<Map>_EventScript_<ROM address>`. Only the address part is given when the map is obvious;
  `Quetzal_*` labels are in `quetzal_rip/scripts/{johto_shared,shared}.inc`.
- "NEW" means there is no counterpart in pokecrystal: no text from pokecrystal's map scripts or text files reaches 50%
  word-pair overlap with it (automatic check over every `.string`; the matching script is described at the end).
  "Inferred" marks conclusions drawn from structure (flags, warps, parties) rather than from text.
- Caveats about names in the rip:
  - In this copy of the rip, the stock special names (`special SomeName`, not `QZ_SPECIAL_n`) are shifted by one entry.
    Read `specialvar VAR_RESULT, ShowDaycareLevelMenu` after a `dowildbattle` as "get battle outcome", and so on. Nothing
    below relies on a stock special's name.
  - The rip prints `{PLAYER}` as `{MUS_RG_VS_TRAINER}`, and POKéBLOCK as `オカキクケ` (Berry Blender text).
  - Item ids 632-889 are missing from the rip's item constant table, so scripts show raw numbers. Their names come from the
    ROM's name table: 706 BICYCLE, 709-711 OLD/GOOD/SUPER ROD, 727 SS TICKET, 736 BASEMENT KEY, 750 CARD KEY, 762 PUNCHING
    GLOVE, 772/773 SCROLL OF DARKNESS/WATERS, 795 BLACK AUGURITE, 796 LINKING CORD, 832 MAX CANDY, 879 GS BALL, 880 TIDAL
    BELL, 881 CLEAR BELL, 882 SECRET POTION, 883 SQUIRTBOTTLE, 885 SILVER WING, 886 RAINBOW WING, 888 RED SCALE, 889
    MYSTERY EGG. 682-689 are HM01-HM08 as key items.

---

## 0. Global differences that show up on every map

| Topic | Quetzal | Crystal |
|---|---|---|
| Map design | All maps redrawn: HGSS layouts, Emerald/FRLG-style tilesets. Sizes differ (e.g. Route 27 is 160x40, the Olivine Lighthouse is one 170x20 map holding all six floors, the four Ruins chambers and their item rooms are one 26x68 map) | GBC block maps |
| Pokémon | Gens 1-9 mixed together, plus regional forms (`SPECIES_X_2`, `_3`...). Starters of every generation appear in the wild (e.g. Treecko and Grookey in Ilex Forest, Sprigatito on Route 30, Bulbasaur and Charmander on Route 36) | Gens 1-2 only |
| Encounter slots | `wild_encounters.json`: land 12, water 5, rock smash 3, a NEW **headbutt-like** slot of 5 (Pineco, Ledyba, Exeggcute, Spinarak on most routes, as in HGSS; rocks such as Geodude, Nosepass and Lileep in some towns), fishing 10 (old 2, good 3, super 5). The rates of the new slot and of rock smash are guesses. 97 Johto encounter tables | Morning/day/night grass tables; headbutt is code-side |
| Visible legendaries | 99 overworld legendary, mythical and Ultra Beast objects across 65 Johto maps, all running `Quetzal_EventScript_840EA6A` (gate `QZ_SPECIAL_705`, battle, `QZ_SPECIAL_708`, removed when the battle is won or the Pokémon caught). List in section 14 | None |
| "Notable" overworld Pokémon | The ACADEMY says "Notable POKéMON available in an area can be seen in the overworld!" Many maps hold script-less objects (graphics set by code) | None |
| Trainers | Trainer classes are Emerald/FRLG ones. Every party is redone with mixed-generation Pokémon, often with alternate forms. Gym leaders and the Elite Four have a second, rematch team (`TRAINER_QZ_496x-498x`) | |
| Rival | SILVER always uses the Totodile line. Each of his battles has 3 identical parties in trainers.json, so his team does not change with the player's starter (inferred) | Starter-dependent |
| HMs | Key items (682-689). "HMs, once unlocked, don't need to be taught" (Earl's Academy blackboard) | TMs/HMs taught to a Pokémon |
| Evolution | Trade evolutions happen by level (MACHOKE/GRAVELER at 38, KADABRA/HAUNTER at 42; Dept Store 4F, Academy). Friendship evolutions become level evolutions (RIOLU at 28). RETURN and FRUSTRATION are always 102 power (`GoldenrodHappinessRater`) | Trade/happiness evolutions |
| Options mentioned in dialogue | EXP SHARE on/off (Academy), NUZLOCKE mode (gift Pokémon withheld: `Quetzal_EventScript_8361A4D` / `QZ_SPECIAL_779`), berry respawning ("If respawning is enabled, defeating POKéMON can replenish them", Route30BerryHouse), auto-run toggled with L (mom), RADIANT Pokémon colour filters (GoldenrodCity_House3), "New region" in the START menu (Elm, after the League) | — |
| Multiplayer | Bill: "To travel to the future, disconnect from multiplayer." Quetzal has its own online mode; parts of it live in specials | — |
| Day-Care | "The DAY-CARE service is now available from any PC" (Route 34 NPC, `Quetzal_Text_83F0C34`) | Route 34 Day-Care couple |
| Berry trees | `setberrytree` objects (`berry_trees/dirt_pile`) on most routes. New Bark Town's on-transition script sets up trees 0x5A-0x6D | Fruit trees |
| Region travel | ELM gives the S.S. TICKET after the League (`ElmsLab_EventScript_83E2256`). Ships sail from Olivine to VERMILION and LILYCOVE once "both regions' Leagues" are won (`Quetzal_EventScript_840F954`, `OlivineCityPier`). Victory Road Gate's east door leads to `KANTO_ROUTE22_NORTH_ENTRANCE` ("Only the champions of both POKéMON LEAGUES may cross") | S.S. Aqua, Magnet Train |
| Team Rocket trio | Jessie, James, Meowth and Wobbuffet stand in most cities (4 objects with `FLAG_QZ_0x1490`, script `Quetzal_EventScript_83C2D48`). They recite their motto, you battle `TRAINER_QZ_913` (party missing from trainers.json, probably built in code), and you get a Pokémon from `VAR_QZ_0x5073` at Lv50 (the text says Pikachu). Afterwards the flag hides them. In the alternate past (section 13) they have different lines | Absent |

---

## 1. New Bark Town and Route 27 east

**NewBarkTown**
- As in Crystal: Elm's aide stops you from going out without a Pokémon, and SILVER peeks at the lab.
- NEW: a lab message ("A message from PROF. ELM arrived", also used on Route 30 and in Blackthorn) pops up as an
  on-frame text event.
- NEW: an NPC blocks the Route 27 crossing until the story allows it (`EventScript_83E157F`, text "Wait! That crossing
  leads to the POKéMON LEAGUE..."). It opens once Lugia is done (see section 12).
- NEW: a postgame Lv100 rematch with SILVER (`TRAINER_QZ_4988`: Crobat, Ursaluna (Bloodmoon, `URSALUNA_2`),
  Meowscarada, Tyranitar, Weavile, Feraligatr).
- NEW: an alternate-past SILVER scene (`EventScript_84039BE`, section 13).

**PlayersHouse1F** (the ripper's "PokemonCenter" name is wrong: MOM heals you)
- MOM explains running shoes and the L auto-run toggle (NEW).
- No Crystal money-saving or Pokégear setup.
- Alternate-past event: a ROCKET (`TRAINER_QZ_4987`, Lv100 Skuntank, Crobat, Weezing) tries to take MOM's EEVEE
  (`FLAG_QZ_0x14A1/14A2`).

**PlayersHouse2F**
- Has the NES and clock texts.
- NEW: the bed (`QZ_SPECIAL_585/586`): "Your bed is neatly made. Would you like to rest?" Part of the **Darkrai
  nightmare** event: "A shadow smothers your dreams... something feels wrong" (`Text_840F22B`). The nightmare ends in
  Dark Cave (`DarkCaveBlackthornEntrance_EventScript_840F19E`, "The nightmare begins to crumble..."), which also warps
  back to this room. While the nightmare lasts, Victory Road Gate refuses: "The darkness will not let you leave JOHTO"
  (`VictoryRoadGate_EventScript_8401CE9`). Inferred: this is Quetzal's Darkrai event, with Darkrai found at the Dark
  Cave Blackthorn entrance (`FLAG_QZ_0x1238`).

**ElmsLab** (40x14, much wider than Crystal's)
- The starter is chosen through `QZ_SPECIAL_689`/`690` (a picker) instead of three fixed Poké Balls
  (`EventScript_83E1ECF`). Inferred: the choice is not limited to the three Johto starters.
- Elm hands over the **NATIONAL POKéDEX** right away (NEW text `83E2891/83E28E7`).
- Elm's story progress is kept in `VAR_QZ_0x5004`: 1 = starter, 2/3 = egg, 5+ = later; 7-10 = the Kimono Girl and
  legendary messages. NEW texts send you to the Dance Theater, then to the TIN TOWER, then to the WHIRL ISLANDS
  (`EventScript_83E21B8/83E2210/83E223E/83E2249`).
- NEW: after the League, Elm gives the **S.S. TICKET** (`83E2256`) instead of Crystal's Master Ball moment. The MASTER
  BALL gift is still there.
- The stolen-Pokémon police scene is kept. NEW: you must say "SILVER" (`Text_83E3A0D`, "Right! His name was SILVER").
  Crystal names the rival here through a naming screen.
- NEW: the Arceus egg hatches here (`EventScript_8401D20`: "This Egg belongs to no known era... That Pokémon... is
  ARCEUS!"), with an Arceus object (`FLAG_QZ_0x131F`). It follows the RuinsOfAlph_Cave5 event (section 5). Poipole is an
  overworld legendary here.

**ElmsHouse, PlayersNeighborsHouse**: Crystal text with small edits.

**Route27** (160x40) **/ Route27SandstormHouse / TohjoFalls / TohjoFalls_Cave2**
- Route27 has the Crystal trainers' dialogue with Lv42-45 mixed-generation parties, Latias as an overworld legendary
  (`FLAG_QZ_0x1205`), and the Crystal "you've crossed into KANTO" line.
- In the Sandstorm house an old woman gives a SMOOTH ROCK (Crystal: TM Sandstorm).
- NEW: Tohjo Falls has a fisherman who gives the **SUPER ROD** (Crystal: Route 12 Super Rod House). Dialga and Palkia are
  overworld legendaries here.
- **TohjoFallsGiovannisRoom** (NEW map): **GIOVANNI** (`TRAINER_QZ_4191`, Lv100 Persian, Nidoqueen,
  Nidoking, Ursaluna, Kingambit, Mewtwo) and a dead radio.
  - After the battle he regrets "the call I left unanswered" at the Radio Tower (`EventScript_8401949`).
  - With CELEBI you can "Travel to the past?" (`EventScript_8401997`, warp to RadioTower5F). This starts the alternate
    past, section 13.

---

## 2. Route 29, Route 46, Cherrygrove, Route 30-31, Mr. Pokémon

- **Route29**: Crystal NPCs. TUSCANY gives the SILK SCARF; NEW: you must guess the weekday sibling's day. The old man's catching tip
  is rewritten and sends you to the "ACADEMY in VIOLET" (Crystal: the catching demonstration). Ogerpon (inferred from the
  graphics id next to the Loyal Three, `FLAG_QZ_0x12FB`) is an overworld legendary.
- **Route29Route46Gate, Route46**: Crystal text. The Route 46 trainers are Lv38-40 (the route is reached late).
- **CherrygroveCity**:
  - The guide gent tour has NEW lines (REGION MAP).
  - The fisher gives MYSTIC WATER.
  - The SILVER battle (`TRAINER_QZ_4701`, Zubat, Houndour, Totodile).
  - NEW: after the battle, "Give it back! That's my TRAINER CARD! Oh no! You saw my name..."
  - Jessie/James and a ROCKET with a Growlithe (`FLAG_QZ_0x14A4`, alternate-past variant).
- **CherrygrovePokecenter1F** and the other Cherrygrove houses: Crystal text, small
  additions.
- **Route30**: Crystal (Joey, Mikey and Don with new parties). NEW: the Elm "disaster" message is shown as a text event.
  - **Route30BerryHouse**: rewritten. Berry explanation, CHERI BERRY, "KURT sells special balls", the respawn option.
  - **MrPokemonsHouse**:
    - MYSTERY EGG (item 889) as in Crystal.
    - OAK hands over the POKéDEX only if you don't have one ("I see ELM already gave you a POKéDEX", NEW branch).
    - MR. POKéMON trades the RED SCALE for a PRISM SCALE (Crystal: EXP. SHARE).
- **Route31**: the Kenya mail quest is kept but rewards an EVIOLITE (Crystal: TM50 Nightmare);
  `qz_cmd_ea 1` checks the delivered Pokémon. Also Wade.
- **Route31VioletGate**: NEW line about a "Pikachu lookalike" (inferred: Pawmi or Dedenne).
- **DarkCaveVioletEntrance**: Giratina is an overworld legendary. No Crystal Falkner-era NPCs.

---

## 3. Violet City, Sprout Tower, Route 32, Ruins of Alph

**VioletCity**
- Earl's tour is kept.
- NEW: the **Kimono Girl ZUKI** watches the egg ("Me oh my... That EGG must be...!", `Text_83FEF92`). This starts the
  HGSS-style Kimono Girl arc.
- On-frame events check `VAR_QZ_0x5012`/`0x5008`.

**EarlsPokemonAcademy** (10x21, NEW content): blackboards and kids explain Quetzal's rules:
- the EXP SHARE balance option;
- trade evolutions by level;
- notable Pokémon in the overworld;
- Johto berries make balls and Kanto berries lower stats; the KANTO DOJO trains individual stats;
- "opponents now use teams with better type coverage";
- HMs do not need teaching.

**VioletGym**
- FALKNER: Emolga, Skarmory (rematch team of 6).
- NEW TM-less reward: **HEAVY-DUTY BOOTS**.
- The Gym guide's tips are rewritten ("bulky owl that loves BERRIES").
- New gym sign format: "<LEADER>'S CERTIFIED TRAINERS: {PLAYER}" (all eight gyms).

**VioletPokecenter1F**
- The Elm aide gives the TOGEPI egg (`giveegg SPECIES_TOGEPI`), as in Crystal.
- NEW: if NUZLOCKE is on, he keeps the egg until the challenge is over (`EventScript_83FFDEB`).

**SproutTower1F-3F**
- Sages are Gen-mixed (Smoliv, Phantump, Zorua-Hisui...).
- Elder LI gives the FLASH HM.
- The SILVER cameo (escape rope) is kept.
- NEW: "You and your POKéMON have passed the tower's test."
- Overworld legendaries: Marshadow (1F), Spectrier (2F), Kartana (3F).

**Route32**
- **Story gate (NEW)**: an NPC blocks the south until Elder LI is beaten: "Before going on, you must defeat the elder,
  LI." (`EventScript_83EED76`). Inferred: an HGSS-like Sprout Tower requirement.
- FRIEDA (POISON BARB) and the roaring guy now give a **RED CARD** (Crystal: TM05 Roar).
- The Miracle Seed guy and the SLOWPOKETAIL seller are kept.
- NEW sign: "It's easy to see what POKéMON you can catch in an area..." (the area Pokédex).

**Route32Pokecenter1F**: the fisherman gives the OLD ROD.

**Ruins of Alph** (heavily changed)
- **RuinsOfAlphOutside**:
  - Crystal NPCs.
  - NEW **DUDUNSPARCE drilling a tunnel** (`Text_840ED22`, `QZ_SPECIAL_707/711`). Inferred: it opens the way to
    RuinsOfAlph_Cave5.
  - Koraidon and Uxie are overworld legendaries.
- **Route32RuinsOfAlphGate** and **Route36RuinsOfAlphGate**: both gates reuse
  Crystal's Route 32 gate text.
- **RuinsOfAlphChambers** (NEW layout): all four sliding-puzzle chambers plus their four item
  rooms in one 26x68 map.
  - The panel puzzles are `QZ_SPECIAL_675-678`.
  - NEW sensors: "Some kind of ancient sensor. What is it looking for?" / "The chamber reacts! The way is open."
    (`QZ_SPECIAL_679`). Inferred: Unown/HGSS-style requirements.
  - The scientist's lines ("A hole just opened up in the floor... Now a hole in the wall too?!") are NEW.
  - Item rooms: Lum, Charcoal, Max Revive, Sacred Ash, berries, stones... (16 hidden or visible items).
  - **Regirock, Regice, Registeel, Regieleki and Regidrago** are overworld legendaries here; Regigigas is in the
    InnerChamber.
- **RuinsOfAlph_Cave1-4 [Ho-Oh/Aerodactyl/Omanyte/Kabuto WordRooms]**: empty rooms leading to the InnerChamber.
- **RuinsOfAlphInnerChamber**:
  - Crystal NPCs.
  - Fossil items: HELIX, DOME, OLD AMBER (NEW).
  - Picks up the **GS BALL** (879): NEW item location. Crystal's GS Ball is a Pokémon Communication Center event.
- **RuinsOfAlphResearchCenter**:
  - A NEW **fossil revival** machine ("FOSSIL REGENERATOR").
  - A NEW research-supplies mart: Beast Ball, Genesect drives, Silvally memories, Arceus plates.
  - Genesect is an overworld legendary.
- **RuinsOfAlphArceusChamber** (NEW map): Unown Lv50 wild, an Arceus object (`FLAG_QZ_0x123E`).
  "The ORIGIN EGG answers your six Mythical Pokémon." (`EventScript_8401CF3`, `QZ_SPECIAL_700/701/709/540`). Inferred:
  bring six Mythicals to get the Arceus egg, which Elm hatches.
- Wild Pokémon: Ruins grass has Natu, Munna, Elgyem, Cleffa, Igglybuff, Togepi, Smeargle, Porygon and fossils. The
  InnerChamber has Unown, Baltoy, Golett, Lunatone, Solrock, Minior, Sigilyph, Stonjourner, Gimmighoul, Archen and
  Aerodactyl.

---

## 4. Union Cave, Route 33, Azalea, Slowpoke Well, Ilex Forest

- **UnionCave1F/B1F/B2F**:
  - Crystal trainers with Gen-mixed teams.
  - B1F gives a **LINKING CORD** (796).
  - B2F: Zygarde, plus Lapras in the water slots (the Crystal Friday Lapras is now a water encounter).
  - Terapagos (B1F) is an overworld legendary.
- **Route33**: rain via `setweather`; Zeraora is an overworld legendary.
- **AzaleaTown**:
  - The Crystal Rocket guard and SILVER battle (`TRAINER_QZ_4721`).
  - The GS BALL handed back after Celebi (`Text_83C4268`).
  - Tapu Bulu is an overworld legendary.
  - A ROCKET plus woman pair (`FLAG_QZ_0x14A4`) for the alternate past.
- **SlowpokeWellB1F**:
  - PROTON (`TRAINER_QZ_4958`) replaces Crystal's unnamed executive (NEW HGSS name).
  - New trainer class TEAM ROCKET "ETO" (`TRAINER_QZ_4859`).
- **SlowpokeWellB2F**:
  - A SLOWKING NPC.
  - KING'S ROCK, and a NEW **DAMP ROCK** gift.
  - Rain weather.
- **KurtsHouse** (HGSS-style):
  - **Kurt sells balls from a mart** (`EventScript_83C5BA8`: Sport, Level, Lure, Moon, Friend, Love, Fast, Heavy, Dream).
    There is no apricorn crafting. His Crystal-style line "bring 'em in batches of 5" survives in text only, and the Radio
    Tower quiz asks "Do you need to give KURT apricorns to buy his POKé BALLS?".
  - First visit gives a FAST BALL.
  - He checks the GS BALL (879): NEW text "What was that sound?! Let's go outside to investigate!" leads into the Ilex
    Forest Celebi event.
- **AzaleaGym**:
  - BUGSY: Dwebble, Dewpider, Scizor (rematch: Kleavor, Larvesta, Slither Wing...).
  - Gift: **BLACK AUGURITE** (Kleavor evolution).
  - Gym guide and twins text rewritten.
- **CharcoalKiln**: CHARCOAL from the apprentice if the boss could not hand it over (NEW fallback text).
- **AzaleaMart / AzaleaPokecenter1F**: Crystal text, small edits.
- **IlexForest**:
  - The Farfetch'd herding and CUT (now a key-item HM) follow Crystal.
  - The boss gives CHARCOAL (NEW: Crystal gave HM01 Cut).
  - NEW HGSS **Kimono Girl NAOKO lost in the forest** escort (`EventScript_83DC433`).
  - **CELEBI** event: put the GS BALL in the shrine, `setwildbattle SPECIES_CELEBI, 40`. NEW: "The GS BALL is still in
    the shrine. Call CELEBI again?" lets you repeat it (`EventScript_83DC2C4`).
  - Mew, Xerneas and Zarude are overworld legendaries.
  - A headbutt-tree NPC is kept.

---

## 5. Route 34, Goldenrod City, Radio Tower, National Park, Route 35-37

**Route34**
- Crystal trainers, policeman, and the triplet cooltrainers (KATE line NEW).
- **DayCare**: NEW content.
  - The Day-Care couple's son gives an **ODD EGG** (`EventScript_83F0695`).
  - An incense mart for breeding.
  - Dictionary and book gags.
  - The Day-Care itself runs from the PC.

**GoldenrodCity**
- The Rocket takeover crowd is kept (flags `0x1096/1097/1098`).
- NEW: Whitney is "not here right now. She might be at the RADIO TOWER" (`EventScript_83D256F`).
- Game Corner day/night joke.
- Postgame BILL Magearna research call (`EventScript_84070C4`).
- Tapu Koko is an overworld legendary.

**Gym gate (NEW)**: WHITNEY waits in RadioTower1F doing the QUIZ (`RadioTower1F_EventScript_83D6C76`).
- Inferred: the gym opens after you meet her and finish the quiz.
- The quiz is NEW (`83D6D95`). It asks five yes/no questions about Quetzal's features:
  1. Is Falkner a FLYING-type leader?
  2. Can you buy berries at the Flower Shop?
  3. Does catching in HOENN register Johto too?
  4. Can you rename from the party menu?
  5. Do you need apricorns for Kurt's balls?
- The prize is a METRONOME. Whitney: "I thought catches counted in every region! Oops!"

**GoldenrodGym**
- WHITNEY: Farigiraf, Miltank, Kangaskhan (rematch adds Zoroark-Hisui, Indeedee-F, Silvally...).
- Gift: CHOICE SCARF.
- The guide mentions the "newly discovered FAIRY type".

**GoldenrodDeptStore1F / 2F / 3F / 4F / 5F / 6F / Roof** (completely new stock, NEW directory
`Text_83D3AFB`)

| Floor | Contents |
|---|---|
| 2F | X items; IV UP/IV MAX, EXP Candies, Rare Candy |
| 3F | Vitamins, feathers, EV-lowering berries, Power items, Destiny Knot, Ability Capsule/Patch, all Mints |
| 4F | Every evolution and form item (stones, Galarica, sweets, pots) |
| 5F | "MEGA, Z & TERA" (`QZ_SPECIAL_569/570/571`, Giga Candy) |
| 6F | In-game trade and the Lucky Channel talk; WHITNEY visits (postgame: "come to the FIGHTING DOJO") |

- GoldenrodDeptStoreElevator.
- GoldenrodDeptStoreRoofAlt is an alternate layout of the roof (`FLAG_QZ_0x1016`).

**Underground**
- **GoldenrodUndergroundEntrances**, **GoldenrodUnderground** (16x36):
  - Crystal trainers.
  - NEW shops: HERB SHOP (Energy Powder/Root, Heal Powder), type GEMS, glass FLUTES ("My glass flutes produce
    different effects").
  - The COIN CASE (718) lies on the ground here.
  - NEW: a Kimono girl (KUNI) moment (`Text_83DB062`).
- **GoldenrodUndergroundSwitchRoomEntrances**: SILVER battle; Magearna is an overworld legendary.
- **GoldenrodUndergroundWarehouse**: Guzzlord is an overworld legendary; the director gives the CARD KEY (750).
- **GoldenrodDeptStoreB1F**: NEW "MACHOKE, help us move that boulder!" boulder puzzle.

**Other Goldenrod buildings**
- **GoldenrodMagnetTrainStation** (154x28): "The MAGNET TRAIN has broken down. Service is
  suspended indefinitely." The Magnet Train is replaced by the S.S. ticket and Route 22 gate (inferred). Miraidon is an
  overworld legendary.
- **GoldenrodBikeShop**:
  - A BICYCLE (706) loan with a MACH/ACRO toggle on L (NEW text). Crystal: just a Bicycle.
  - Meltan is an overworld legendary.
- **BillsFamilysHouse** (NEW content):
  - BILL gives **ROTOM** ("This ROTOM moved into my computer", `EventScript_83D319D`, `qz_cmd_e9 3`).
  - Postgame Magearna quest, below.
- **GoldenrodFlowerShop**:
  - SQUIRTBOTTLE (883) after the PLAINBADGE (NEW dialogue).
  - Two berry marts (type-resist berries, then all berries).
  - A Kimono sister's flower gift.
- **GoldenrodGameCorner**: slot machines (`playslotmachine`) and a Pokémon prize counter using `givemon VAR_TEMP_1, 20`.
- **GoldenrodHappinessRater**: friendship rules text (section 0).
- **GoldenrodNameRater**: talks about RADIANT Pokémon and renaming from the party menu. There is no
  Name Rater service.
- **GoldenrodPokecenter1F**: "The roof is made of gold!" jokes.
- **GoldenrodPPSpeechHouse**: Crystal text.

**Radio Tower 1F-5F**
- The Crystal takeover, with the HGSS executives by name: PROTON (4F), PETREL (5F impostor director), ARIANA and ARCHER
  (5F).
- 1F: the NEW **POKéMON LOTTERY** (Emerald-style loto, `EventScript_83D745D`) replaces Crystal's Lucky Number show.
- 3F: THROAT SPRAY reward.
- 4F: MARY gives BRIGHT POWDER (Crystal: Pink Bow).
- 5F:
  - BASEMENT KEY (736).
  - **RAINBOW WING** (886) from the Director (Crystal: Clear Bell/Silver Wing), with NEW line "That wing... the
    LEGENDARY POKéMON won't appear with just that wing".
  - NEW "FAKE OUT" gag.
  - MAX CANDY.

**National Park**
- **Route35NationalParkGate** (50x18: Crystal's Route 35 and Route 36 gates merged): runs the **Bug-Catching Contest**,
  heavily reworked (`EventScript_83D1411`, `QZ_SPECIAL_680/779`):
  - 8 minutes and 30 SAFARI BALLS.
  - Up to 5 catches; the best one is entered automatically.
  - "You keep every catch, even if you are eliminated."
  - The score uses level, IVs, rarity, catch rate, shiny and radiant.
  - EXP gained during the contest is reverted.
  - You need 5 free PC slots.
  - "You may enter again whenever you like": no Tue/Thu/Sat restriction was found in the gate scripts (inferred).
  - With Nuzlocke on it is an "exhibition run" (`NationalParkBugContest_EventScript_83C34C8`).
- **NationalParkBugContest**: the contest map (judging, prize, returning your party: `QZ_SPECIAL_681-690, 788-790`).
  Contest wild Pokémon mix Gens: Bunnelby, Blitzle, Rockruff, Pinsir, Scyther...
- **NationalPark**: Crystal NPCs and trainers (QUICK CLAW gift); Shaymin and Enamorus are overworld legendaries; NEW
  couple text.

**Route35**: Crystal trainers. Munkidori and Blacephalon are overworld legendaries. A ROCKET with a Mareep is there in
the alternate past (`FLAG_QZ_0x14A4`).

**Route35GoldenrodGate**: Randy's mail errand; reward HP UP (Crystal: HP Up as well).

**Route36**
- SUDOWOODO with the SQUIRTBOTTLE (`EventScript_83F1B1E`, Lv20).
- ROCK SMASH (key-item HM) reward.
- ARTHUR's HARD STONE.
- NEW night warning about a scary Growlithe-like Pokémon (Hisuian Growlithe).

**Route37**: SUNNY's MAGNET. Psychic GREG has a Stantler.

---

## 6. Ecruteak City, Burned Tower, Dance Theater, Tin Tower (Ho-Oh)

**EcruteakCity**
- NEW gates for the Gym (`EventScript_83CD4BF`):
  - "Gym Leader MORTY is not here right now. You may find him at the BURNED TOWER."
  - "A TEAM ROCKET member is bothering the KIMONO GIRLS. Please help them at the THEATER before entering the GYM."
- NEW: a scene where SILVER loses to the five Kimono Girls (`EventScript_83CD1E9`).
- BILL in the Pokémon Center (Crystal-like; NEW lines about Kanto PCs).
- Tapu Lele, Entei and Gouging Fire are overworld legendaries.

**BurnedTower1F/B1F**
- SILVER battle.
- EUSINE and MORTY as in Crystal (EUSINE's text partly NEW).
- NEW trainers: Firebreather NARD, Burglar RICHARDO.
- B1F: the three beasts flee (object flags `0x10AF`).
- Moltres (1F) and Chi-Yu and Yveltal (B1F) are overworld legendaries.

**DanceTheater** (HGSS Kimono Girl arc, almost all NEW)
1. A Rocket grunt harasses a Kimono Girl (`EventScript_83CF5C4`, `TRAINER_QZ_4133`). Afterwards the theater gentleman gives the
   SURF reward ("That's SURF" text kept; Crystal gives HM03 for beating the five Kimono Girls).
2. Later, Elm sends you back. ZUKI explains that the MYSTERY EGG passed "from MR. POKéMON to PROF.ELM, then to you" to find
   a worthy trainer, and the five KIMONO GIRLS (all Lv48 Eeveelutions: `TRAINER_QZ_4098/4114/4143/4312/4594`) battle you
   in turn (`EventScript_83D01FC`).
3. Reward: **CLEAR BELL** (881) and **RAINBOW WING** (886) (`EventScript_83D0224`). They leave for the TIN TOWER.
4. Each girl recalls where you met her: Violet, Ilex, the Ice Path, Goldenrod Tunnel.

Meloetta is an overworld legendary here.

**EcruteakGym**
- MORTY: Mismagius, Mimikyu, Zoroark-Hisui, Gengar.
- Gift: LIFE ORB.
- Hex Maniacs and Sages.
- Guide text NEW.

**EcruteakItemfinderHouse**: the history book only; the ITEMFINDER gift was dropped.

**EcruteakTinTowerEntrance / WiseTriosRoom / EcruteakCityNorth**
- Tin Tower stands in a NEW northern part of Ecruteak (70x69, HGSS Bell Tower grounds), reached through the entrance
  building.
- Sage text partly NEW ("The bell of tin rings clearly...").
- Postgame MORTY stands in front of it ("find me at the FIGHTING DOJO").
- Wo-Chien is an overworld legendary.

**TinTower1F, TinTower_Cave1-4 and the 4 mislabeled "RuinsOfAlph*ItemRoom" maps [TinTower2F-9F]**: Crystal's warp maze,
Gen-mixed ghost and psychic encounters, items.

**TinTowerRoof**
- The Kimono Girl dance welcomes HO-OH (`Quetzal_EventScript_83FB64A`, trigger `VAR_QZ_0x5007 == 2`).
- Battle `Quetzal_EventScript_83FB8BE`: `setwildbattle SPECIES_HO_OH, 50`.
- Afterwards the girls give the **SILVER WING** (885) and **TIDAL BELL** (880): "Bring the TIDAL BELL to the WHIRL
  ISLANDS."
- This is HGSS's order inverted: both legendaries are pre-League story events, Ho-Oh first.
- NEW postgame: **MAXIE** (`TRAINER_QZ_4960`, Lv70 with Groudon; `EventScript_840EBF7`) battles you and gives the **RED
  ORB**.
- TinTowerRoofAlt is the layout swap (`QZ_SPECIAL_704`).

---

## 7. Route 38-39, Olivine, Lighthouse, Cianwood, Route 40-41, Whirl Islands

- **Route38/39, Route38EcruteakGate**: Crystal trainers.
  - NEW sign: "NO SWIMMING OR FISHING. WATER FOR FARMING ONLY."
  - **Route39**: NEW **BAOBA** intro (former Fuchsia Safari warden, opening a Safari Zone near Cianwood;
    `EventScript_83F2DDB`). He sees Ho-Oh's shadow.
  - **Route39Barn**: feed MILTANK ORAN BERRIES (Crystal: any berries).
  - **Route39Farmhouse**: MOOMOO MILK shop, then LEFTOVERS (Crystal: TM83 Snore).
- **OlivineCity**:
  - SILVER's speech.
  - NEW Safari Zone news.
  - NEW sailors' talk about JASMINE's ranking.
  - Raikou and Raging Bolt are overworld legendaries.
- **OlivineLighthouse** (one 170x20 map holding all floors):
  - Crystal trainers with mixed parties.
  - The SECRET POTION (882) errand for AMPHY.
  - Zapdos and Victini are overworld legendaries.
- **OlivineGym**:
  - JASMINE: Skarmory, Magnezone, Scizor, Empoleon, Steelix.
  - Gift: ROCKY HELMET.
  - NEW gym trainers (cooltrainer and gentleman, `FLAG_QZ_0x10B6`); Crystal's gym has no trainers.
- **OlivineCafe**:
  - Strength sailor.
  - NEW postgame JASMINE ("visit me at the FIGHTING DOJO in SAFFRON").
- **OlivineGoodRodHouse**: GOOD ROD (710); NEW Pokédex fishing-group tip.
- **OlivineTimsHouse**: in-game trade.
- **OlivinePort, OlivineCityPier**: Crystal port text; the NEW multi-region ferry
  (`Quetzal_EventScript_840F954`).
- **CianwoodCity**:
  - The Crystal FLY HM giver and EUSINE battle (`TRAINER_QZ_4656`).
  - The Suicune sighting.
  - Tapu Fini is an overworld legendary.
  - NEW signs: "CLIFF EDGE CAVE Road to the SAFARI ZONE" and "This path is under construction".
- **CianwoodGym**: CHUCK (Pangoro, Sneasler, Lucario, Poliwrath); gift **PUNCHING GLOVE** (762).
- **CianwoodPharmacy**: SECRET POTION.
- **ManiasHouse**: Mania's Shuckle is now a gift to keep ("You can keep it for good!", `qz_cmd_e9 2`).
- **CianwoodCity_House1** (NEW, unsure name): an old woman reacts to the CLEAR BELL and the TIDAL BELL (HGSS flavour).
- **CianwoodLugiaSpeechHouse**: Crystal text.
- **Route40/41**:
  - Crystal swimmers with mixed parties.
  - MONICA's SHARP BEAK.
  - Route 41 is a separate map reached through a row of warps.
  - Thundurus is an overworld legendary on Route 41.
  - **Route40TrainerHillGate** leads north to Trainer Hill (section 11).
- **WhirlIslands_Cave1-7**: redesigned (1F with all four island entrances, B1F, an extra chain of caves 3-5, B2F, Lugia
  Chamber).
  - **WhirlIslandLugiaChamber**: the Kimono Girls' dance with the TIDAL BELL
    (`EventScript_8400ED8`), then `setwildbattle SPECIES_LUGIA, 50`. Crystal needs the Silver Wing; Lugia is
    pre-League here.
  - NEW postgame: **ARCHIE** (`TRAINER_QZ_4961`, Lv70 with Kyogre; `EventScript_840EC95`) gives the **BLUE ORB**.
  - Manaphy and Nihilego (Cave5) are overworld legendaries; Tentacruel are swimming objects (`FLAG_QZ_0x1048`).

---

## 8. Mahogany, Lake of Rage, Rocket Hideout, Route 42-44, Mt. Mortar, Ice Path, Blackthorn

- **Route42**: EUSINE's Suicune chase lines are NEW; a MODEST MINT item.
- **MountMortar1FOutside**, **MountMortar1FInside/2FInside/B1F**:
  - Crystal items and the Markus/Hugh "lost partner" pair.
  - B1F: KIYO (Urshifu both forms).
  - NEW: "Return with seven JOHTO Badges" gate (`EventScript_83E00AE`). Afterwards he gives the **SCROLL OF DARKNESS /
    SCROLL OF WATERS** for Kubfu (Kubfu is an overworld legendary here). Crystal: Tyrogue gift.
  - Cobalion, Iron Crown, Ting-Lu, Volcanion and Keldeo are overworld legendaries.
- **MahoganyTown**: RAGECANDYBAR seller (gives the item).
- **MahoganyMart1F**:
  - NEW Grandma's food mart (regional treats and nectars).
  - LANCE's DRAGONITE uses HYPER BEAM.
  - Pecharunt is an overworld legendary.
- **Route43Gate**: the Rocket toll. NEW: "You're totally broke! ... you can pass". The guard gives BLACK SLUDGE
  (Crystal: TM36 Sludge Bomb).
- **LakeOfRage**:
  - The RED GYARADOS uses a NEW command: `qz_cmd_ec SPECIES_GYARADOS, 30` (`EventScript_83DD40B`; inferred to be a
    shiny static battle) and gives the RED SCALE (888).
  - LANCE as in Crystal.
  - WESLEY's BLACK BELT.
  - NEW postgame PRYCE ("come to the FIGHTING DOJO").
  - Suicune, Walking Wake and Mesprit are overworld legendaries.
  - The weather is a thunderstorm.
  - LakeOfRage_Alt is an unused layout.
- **LakeOfRageHiddenPowerHouse**: gives a DAWN STONE (Crystal: TM10 Hidden Power).
- **LakeOfRageMagikarpHouse**: the length contest is replaced by a NEW **Magikarp buyer**: ¥2,000 each, ¥100,000 for a
  Lv100 (`EventScript_83DE00B`).
- **TeamRocketBaseB1F-B3F**:
  - Crystal traps and Electrodes (B2F Electrode `setwildbattle`, counted in `VAR_QZ_0x5025`).
  - NEW HGSS-style password via **MURKROW mimicking PETREL's voice** (`Text_83E9B2C`).
  - B2F: ARIANA plus a grunt in a 2-on-1, and LANCE joins as your partner (NEW multi battle, `Text_83E9DFE`).
  - B3F: PETREL disguised as Giovanni.
  - Mewtwo, Type: Null and Xurkitree are overworld legendaries.
  - WHIRLPOOL HM from Lance.
- **MahoganyGym**:
  - PRYCE: Ninetales-Alola, Sandslash-Alola, Iron Bundle, Froslass, Mamoswine (rematch adds Kyurem-Black, Calyrex-Ice).
  - Gift: FOCUS SASH.
  - Trainer EXPERT "ROXANNE" (a name collision with the Hoenn leader).
- **Route44**: Crystal trainers; Virizion and Iron Leaves are overworld legendaries.
- **IcePath_Cave1-5 [IcePath1F, B1F, B2FMahoganySide, B2FBlackthornSide, B3F]**:
  - 1F: NEW Kimono Girl **SAYO stuck on the ice**, push her free (`EventScript_83DB642`). Same idea as HGSS.
  - B2F Blackthorn side: NEW **SWINUB event** (`EventScript_83DBAB8`). Feed it an ASPEAR BERRY; it evolves "straight into
    MAMOSWINE"; you battle it (`setwildbattle MAMOSWINE, 45`, `QZ_SPECIAL_771`); it leaves the **ICIUM Z** (Z-crystal).
  - Chien-Pao, Glastrier, Articuno and Kyurem are overworld legendaries.
  - The Crystal HM07 Waterfall item is not in the Ice Path item list (Waterfall comes from the Rising Badge as a key
    item).
- **BlackthornCity**:
  - NEW gate: "CLAIR... only battles trainers who already have 7 badges."
  - SANTOS's SPELL TAG.
  - NEW "MOVE BROS' HOUSE" sign (`MoveDeletersHouse`, empty; inferred to be handled by code).
  - `BlackthornEmysHouse`: Sliggoo-Hisui trade.
- **BlackthornGym1F** (one floor; Crystal has 1F and 2F):
  - CLAIR: Dragalge, Archaludon, Gouging Fire, Garchomp, Kingdra (rematch: Eternatus, Dialga-Origin...).
  - NEW guide text: "a team consisting of dragons from the sea".
- **DragonsDen1F, DragonsDenB1F, DragonShrine**:
  - Crystal quiz.
  - NEW: CLAIR gives the badge plus a **DRAGON SCALE**; the Elder keeps it if your bag is full.
  - NEW: the Lv35 DRATINI gift (Crystal: Dratini with Extremespeed) is reached through `EventScript_83CD025`.
  - NEW: CLAIR "You have all eight BADGES. The way to the LEAGUE starts east of NEW BARK TOWN. Use SURF there. Once you
    become CHAMPION, you can challenge the GYM LEADERS again." (`EventScript_83CC7D5`).
  - Postgame: CLAIR and SILVER visit the shrine.
  - Azelf, Reshiram and Zekrom are overworld legendaries.
- **DarkCaveBlackthornEntrance**: BLACK GLASSES as in Crystal; the end of the Darkrai nightmare (section 1).

---

## 9. Route 26-28, Victory Road, Indigo Plateau (Johto League)

- **VictoryRoadGate**:
  - 8-badge check.
  - NEW: "closed until someone deals with that LEGENDARY POKéMON" (`EventScript_83E8F54`; gate = the Lugia event,
    `VAR_QZ_0x5007`).
  - NEW east exit to KANTO Route 22 for double champions.
  - Alternate past: "Access to KANTO is forbidden by GIOVANNI's order".
  - Darkrai nightmare block.
- **Route26South / Route26**: Route 26 is split in two maps. Crystal trainers with Lv41-50 parties.
  Latios is an overworld legendary.
- **Route26HealHouse, DayOfWeekSiblingsHouse**: Crystal.
- **VictoryRoad_Cave2/Cave1/VictoryRoad [VictoryRoad1F/2F/3F]**:
  - Three floors (Crystal: one map).
  - SILVER battle on 3F.
  - Terrakion, Iron Boulder and Stakataka are overworld legendaries.
- **Route23 / IndigoPlateau**: Johto's own copy of FRLG's Route 23 and plateau
  exterior. Zacian, Zamazenta and Eternatus are overworld legendaries.
- **IndigoPlateauPokecenter1F**:
  - NEW: the League "is not taking new challengers" until ready.
  - NEW postgame SILVER Lv70 rematch (`EventScript_83DD079`).
  - Alternate-past "GIOVANNI ordered the LEAGUE closed".
- **WillsRoom, KogasRoom, BrunosRoom, KarensRoom, LancesRoom, HallOfFame**:
  - Crystal intros, plus NEW rematch intros ("Welcome back", `QZ_SPECIAL_672`; the second teams are TRAINER_QZ_4977-4985,
    with legendaries).
  - LANCE: Dracovish, Goodra-Hisui, Gouging Fire, Hydrapple, Dragapult, Dragonite.
  - Mary and Oak's interview text NEW.

| Elite Four member | First team (Lv48-56) | Rematch |
|---|---|---|
| WILL | Indeedee, Malamar, Hatterene, Metagross, Delphox, Slowking | Tapu Lele, Starmie, Delphox, Hoopa, Calyrex, Necrozma |
| KOGA | Iron Moth, Muk-A, Weezing-G, Slowking-G, Scolipede, Naganadel | Glimmora, ..., Eternatus, Arceus form |
| BRUNO | Annihilape, Iron Hands, Okidogi, Chesnaught, Urshifu, Blaziken | Koraidon, Zamazenta, Mewtwo, Marshadow... |
| KAREN | Samurott-H, Drapion, Incineroar, Roaring Moon, Greninja, Darkrai | Absol, Greninja, Chien-Pao, Yveltal, Darkrai, Kingambit |
| LANCE | (see above) | Miraidon, Dialga, Koraidon, Rayquaza, Zygarde, Eternatus |

---

## 10. Route 28, Mt. Silver

- **Route28 / Route28SteelWingHouse**: the celebrity gives a METAL COAT (Crystal: TM47 Steel Wing).
- **SilverCaveOutside / SilverCavePokecenter1F**: NEW Pewter museum researcher (artifacts
  on the mountain).
- **SilverCaveRoom1-3**, **SilverCaveItemRoom** (Birch's bag and Necrozma),
  **Cave2** (fire room: Moltres and Heatran), **SilverCaveMountainside** (Jirachi), **SilverCaveSnowySlope** (Calyrex,
  Articuno).
  - Cave5 holds a **Deoxys** with a Birth Island-style stone (`FLAG_QZ_0x1423`, plays the RG Deoxys music).
  - Wild Pokémon Lv55-73, fully evolved Gen 1-9.
- **MtSilver_Area3/Area4 [SilverCaveSummit/Alt]**:
  - **RED** (`TRAINER_QZ_4327`, Lv100 Pikachu-cap, Snorlax, Charizard, Venusaur, Blastoise, Lapras;
    `Quetzal_EventScript_83E05E8`) stands outdoors in a blizzard. Crystal puts him inside Room 3.
  - Win: "RED offers you a Z-Crystal" (**PIKASHUNIUM Z**).
  - The two layouts are chosen with `FLAG_QZ_0x1016`.

---

## 11. Side areas not in Crystal

- **CliffEdgeCave** (from Cianwood) and **CliffCave**: HGSS Cliff Edge Gate / Cliff
  Cave, as caves. Diancie is an overworld legendary. Safari Zone teaser NPCs.
- **Route47, Route48**:
  - HGSS Routes 47-48, with HGSS trainer text (Young Couple THOM & KAE, DUFF & EDA).
  - Tornadus, Fezandipiti and Landorus are overworld legendaries.
- **EmbeddedTower**:
  - Kyogre, Groudon and Rayquaza objects (`QZ_SPECIAL_829/706/708`).
  - Inferred: the RED/BLUE ORBs from Maxie and Archie unlock Groudon and Kyogre here, with Rayquaza after (HGSS used
    Kyogre/Groudon with the Blue/Red Orbs and Rayquaza with the Jade Orb).
- **Safari Zone** (HGSS location, different rules):
  - SafariZoneGate is a small town with a Pokémon Center and four held-item marts (type boosters;
    weather/terrain; classic held items; modern held items).
  - SafariZoneEntrance:
    - BAOBA plus Berry Blender NPCs (Emerald blender code, `qz_cmd_ee 1-4`).
    - QUICK BALL starter gift.
    - "Entry is free. You can explore all six areas with no time or step limit... battled and caught normally" (NEW;
      no Safari Balls).
    - BAOBA's **exotic-Pokémon quest**: show him Pokémon from the BEACH, BRUSH, MOUNTAIN and CAVE areas of the
      *Fuchsia* (Kanto) Safari Zone to raise your "SAFARI LEVEL", ending with a DEVON-spy reward.
  - Six areas: Desert, Plains, Beach, Forest, and two unsure ones (dragon/peak and cat/savannah). There are no HGSS block
    placement mechanics.
  - Pheromosa, Cresselia and Buzzwole are overworld legendaries.
- **TrainerHillOutside** (north of Route 40, replacing Crystal's Battle Tower):
  - Signs for TRAINER HILL and BATTLE TENTS (Factory, Palace, Arena).
  - TrainerHill_Cave1 is an empty interior. Inferred: the facilities are code/menus or unfinished; nothing in the
    Johto scripts runs them.
- **GoldenrodCity_House4 [Magnet Train Station]**: kept but out of service (section 5).

---

## 12. Story progression gates (plain words)

| Where | Requirement | Mechanism (as far as visible) |
|---|---|---|
| New Bark, leaving town | Get a Pokémon | Crystal-style aide (`NewBarkTown`) |
| Route 32 south | Beat Elder LI in Sprout Tower | NPC on Route32 (`EventScript_83EED76`) |
| Goldenrod Gym | Meet WHITNEY at the Radio Tower quiz | NPC at the gym door (`GoldenrodCity_EventScript_83D256F`) |
| Route 36 west | SQUIRTBOTTLE from the Flower Shop, which needs the PLAINBADGE | Sudowoodo object `FLAG_QZ_0x102E` |
| Ecruteak Gym | MORTY found at the Burned Tower, and the Dance Theater Rocket beaten | NPCs (`EcruteakCity_EventScript_83CD4BF`) |
| Mt. Mortar B1F reward | 7 badges | `MountMortarB1F_EventScript_83E00AE` (`QZ_SPECIAL_791`) |
| Blackthorn Gym | 7 badges | NPC text in BlackthornCity |
| Kimono / legendary chain | Elm messages in order: Dance Theater > Tin Tower (Ho-Oh) > Whirl Islands (Lugia) | `VAR_QZ_0x5004` 7..10 and `VAR_QZ_0x5007` (Tin Tower trigger at 2, done at 3) |
| Route 27 crossing (New Bark) and Victory Road Gate | 8 badges plus the Lugia event | `EventScript_83E157F`, `VictoryRoadGate_EventScript_83E8F54` |
| Johto <-> Kanto (gate, ferry) | Champion of both Leagues plus the S.S. TICKET | `Quetzal_Text_83E90EF`, `Quetzal_EventScript_840F954` |
| Gym and Elite Four rematches | Johto Champion | `QZ_SPECIAL_670` in every gym, `QZ_SPECIAL_672` in the E4 rooms |
| Former leaders | Postgame: Morty, Whitney, Jasmine, Pryce and Clair point to the FIGHTING DOJO in Saffron | Kanto-side battles |
| Overworld legendaries | Inferred: post-League or other conditions | `QZ_SPECIAL_705` returns 0 = show "..." only |
| Celebi alternate past | Celebi in the party, then Giovanni at Tohjo Falls | `TohjoFalls_Cave2_EventScript_8401997` |
| Bill's future | Johto Champion, an original-colour Magearna, then Celebi as the playable character, single player | `BillsFamilysHouse_EventScript_8407185/840727F` |

---

## 13. Big postgame storylines (NEW, nothing like them in Gen 2)

1. **Celebi alternate past / Team Rocket Johto**:
   - Start: Giovanni at TohjoFalls_Cave2. CELEBI takes you to "an alternate past" where Giovanni answered his men's call
     at the Radio Tower (`Text_840203D`).
   - RadioTower5F: you fight **beside Giovanni** in a multi battle against **ETHAN**: "Choose up to three Pokémon. I'll
     bring three of my own." (`EventScript_84034C7`).
   - Johto becomes Rocket-ruled:
     - the League is closed and Kanto is sealed;
     - Rocket grunts with townsfolk's Pokémon stand in Cherrygrove, Azalea, Goldenrod and on Route 35 (flags
       `0x14A1-0x14A9`, `0x131E`, `0x1321`);
     - Jessie and James have new lines;
     - the family "contributions" text (`Quetzal_Text_8402E50`).
   - Giovanni orders you to New Bark. A grunt tries to take MOM's EEVEE, you turn on him, and MOM heals your team.
   - SILVER confronts you, then joins (`NewBarkTown_EventScript_84039BE`). Celebi takes you both back to the Radio Tower
     for a SILVER + player multi battle against Giovanni (`EventScript_8403B5E`).
   - Celebi exiles Giovanni. SILVER gives rewards and heads to the Dragon's Den. Giovanni at Tohjo later remembers "your
     betrayal".
   - Machinery: `QZ_SPECIAL_572/797/698/699/693-695/792` (camera, object and party swaps); warps to RadioTower5F and
     NewBarkTown_PokemonCenter_1F1.
2. **Bill and Magearna, the "possible future"** (BillsFamilysHouse, labels `84071xx-84082xx`): give BILL an original-colour
   Magearna for an emotion-translator experiment. He wants to visit a future where people and Pokémon can talk, using
   **CELEBI as your playable character** ("Use CELEBI as your playable character", "disconnect from multiplayer"). The
   trip ends with medical research and Bill turning the prototype off. Inferred: Quetzal can play as a Pokémon.
3. **Darkrai nightmare** (section 1).
4. **Mythical Origin Egg / Arceus** (sections 1 and 3).
5. **Maxie and Archie** at the Tin Tower roof and the Whirl Islands: Red Orb and Blue Orb, then the Embedded Tower.
6. **99 overworld legendaries**, list below.

---

## 14. Overworld legendary list (Johto, `Quetzal_EventScript_840EA6A`, flag in brackets)

| Map | Legendaries |
|---|---|
| AzaleaTown | Tapu Bulu (1283) |
| BurnedTower1F | Moltres (11E1) |
| BurnedTowerB1F | Chi-Yu (12E3), Yveltal (126B) |
| CianwoodCity | Tapu Fini (1286) |
| CliffCave_Cave1 | Diancie (1271) |
| DanceTheater | Meloetta (1262) |
| DarkCave_Cave1 | Giratina (122F) |
| DarkCaveBlackthornEntrance | Darkrai (1238, own script) |
| DragonShrine | Reshiram (1253), Zekrom (1256) |
| DragonsDenB1F | Azelf (1220) |
| EcruteakCity | Tapu Lele (1280), Entei (11ED), Gouging Fire (12FE) |
| EcruteakCity_Area1 | Wo-Chien (12DA) |
| ElmsLab | Poipole (12AA) |
| EmbeddedTower | Kyogre (120B), Groudon (120E), Rayquaza (1211) |
| GoldenrodBikeShop | Meltan (12B6) |
| GoldenrodCity | Tapu Koko (127D) |
| Magnet Train Station | Miraidon (12E9) |
| Underground switch room | Magearna (12A4) |
| Underground warehouse | Guzzlord (129E) |
| IcePath | Chien-Pao (12DD), Glastrier (12CE), Articuno (11D2), Kyurem (125C) |
| IlexForest | Mew (11E7), Xerneas (1268), Zarude (12C5) |
| Indigo Plateau | Zacian (12B9), Zamazenta (12BC), Eternatus (12BF) |
| LakeOfRage | Suicune (11F0), Walking Wake (12EC), Mesprit (121D) |
| MahoganyMart1F | Pecharunt (130D) |
| Mt. Mortar | Ting-Lu (12E0), Volcanion (1277), Keldeo (125F), Kubfu (12C2), Cobalion (1244), Iron Crown (1307) |
| Mt. Silver | Jirachi (1214), Calyrex (12D4), Articuno (11D5), Necrozma (12A1), Moltres (11DE), Heatran (1229), Deoxys (1217) |
| NationalPark | Shaymin (123B), Enamorus (12D7) |
| OlivineCity | Raikou (11EA), Raging Bolt (1301) |
| Lighthouse | Zapdos (11D8), Victini (1241) |
| Route26 south | Latios (1208) |
| Route27 | Latias (1205) |
| Route29 | Ogerpon (inferred, 12FB) |
| Route33 | Zeraora (12B3) |
| Route35 | Munkidori (12F5), Blacephalon (12B0) |
| Route41 | Thundurus (1250) |
| Route44 | Virizion (124A), Iron Leaves (12EF) |
| Route45 | Zapdos (11DB), Okidogi (12F2) |
| Route47 | Tornadus (124D), Fezandipiti (12F8) |
| Route48 | Landorus (1259) |
| Ruins of Alph | Regigigas (122C); Regirock, Regice, Registeel, Regieleki, Regidrago (11FC/11FF/1202/12C8/12CB); Koraidon (12E6), Uxie (121A), Genesect (1265); Celesteela (1298, in Tin Tower 8F) |
| Safari Zone | Pheromosa (1292), Cresselia (1232), Buzzwole (128F) |
| Sprout Tower | Marshadow (12A7), Spectrier (12D1), Kartana (129B) |
| Rocket Hideout | Xurkitree (1295), Mewtwo (11E4), Type: Null (127A) |
| TohjoFalls | Palkia (1226), Dialga (1223) |
| Union Cave | Terapagos (130A), Zygarde (126E) |
| Victory Road | Terrakion (1247), Iron Boulder (1304), Stakataka (12AD) |
| Whirl Islands | Manaphy (1235), Nihilego (128C) |

Story legendaries with their own scripts:

| Pokémon | Where | Level |
|---|---|---|
| Celebi | Ilex Forest | 40 |
| Sudowoodo | Route 36 | 20 |
| Red Gyarados | Lake of Rage, `qz_cmd_ec` | 30 |
| Ho-Oh | Tin Tower roof | 50 |
| Lugia | Whirl Islands | 50 |
| Mamoswine | Ice Path | 45 |
| Electrode ×6 | Rocket Hideout | 30 |
| Arceus | Origin Egg | (egg) |

Gifts:

| Pokémon | Where |
|---|---|
| Rotom | Bill |
| Dratini | Dragon Shrine, Lv35 |
| Shuckle | Mania |
| Togepi egg | Violet |
| Odd Egg | Route 34 |
| Pikachu | Rocket trio |
| Revived fossils | Ruins Research Center |
| Prize Pokémon | Game Corner |

---

## 15. Checklist: things not in Gen 2

- [ ] Gens 1-9 Pokémon, regional forms, Mega/Z/Tera items in shops (Dept Store 5F), Z-crystals (Icium Z, Pikashunium Z)
- [ ] New encounter slot (headbutt-like), rock smash slot, Gen 3-style fishing groups; area Pokédex
- [ ] 99 visible overworld legendaries (`Quetzal_EventScript_840EA6A`) plus "notable Pokémon" shown on the overworld
- [ ] Starter picker (`QZ_SPECIAL_689/690`); National Dex at the start
- [ ] HMs as key items; trade and friendship evolutions by level; fixed-power Return/Frustration
- [ ] Options: EXP SHARE, Nuzlocke, berry respawn, auto-run, radiant palettes; multiplayer; playable-Pokémon mode
- [ ] Kurt's ball shop (instead of apricorns); held-item gym rewards
- [ ] HGSS Kimono Girl arc: Dance Theater battles, Clear Bell / Tidal Bell, Ho-Oh and then Lugia both before the League;
      Sayo on the Ice Path ice, Naoko in Ilex, Zuki in Violet, Kuni in the Goldenrod tunnel
- [ ] HGSS executives by name (Proton, Petrel, Ariana, Archer), Murkrow password, Lance multi battle in the hideout
- [ ] Radio Tower quiz with Whitney; Pokémon Lottery; Magnet Train broken
- [ ] Bug Contest rework (5 catches, scoring with IVs/shiny/radiant, keep all catches, any day)
- [ ] Magikarp buyer; Odd Egg; fossil revival; Rotom gift; Swinub/Mamoswine Ice Path event; Kubfu scrolls (7 badges)
- [ ] Gym leader and Elite Four rematch teams; former leaders at the Saffron Fighting Dojo; Lv70/Lv100 Silver rematches
- [ ] Johto's own Indigo Plateau and Route 23; 3-floor Victory Road; Route 22 gate to Kanto for double champions;
      Olivine ferry to Vermilion/Lilycove; "New region" START option
- [ ] HGSS areas: Routes 47-48, Cliff Edge Cave, Cliff Cave, Embedded Tower, Safari Zone (Baoba, free entry, 6 areas,
      Fuchsia exotic quest, Berry Blender), Trainer Hill / Battle Tents signs, Ecruteak north (Bell Tower grounds)
- [ ] Red on the summit outdoors; Deoxys, Jirachi and others on Mt. Silver
- [ ] Maxie (Tin Tower) and Archie (Whirl Islands) postgame, with the orbs
- [ ] Celebi alternate past with a Rocket-ruled Johto, Ethan and Silver multi battles, Giovanni at Tohjo Falls
- [ ] Bill / Magearna / Celebi possible-future arc
- [ ] Darkrai nightmare (bed in Player's House 2F to Dark Cave)
- [ ] Origin Egg (six Mythicals) to Arceus
- [ ] Jessie/James/Meowth/Wobbuffet roaming battles with a Pikachu reward
- [ ] Day-Care through the PC; Berry trees (Emerald-style) with respawn

## 16. Gen 2 things Quetzal dropped or changed

- Pokégear (phone, radio channels, map card), phone rematches, the Lucky Number show, Buena's password prize show (Buena's
  line remains), Mom saving money.
- Apricorn ball crafting (Kurt is a shop now). The Crystal "batches" line survives only as unused flavour.
- Bug Contest days (Tue/Thu/Sat), Safari-ball-only scoring by species/HP.
- Odd one-floor maps: Crystal's single Victory Road, single Route 26, separate Lighthouse floors and separate Ruins
  chambers. All restructured.
- Gift TMs replaced by held items: Steel Wing to METAL COAT; Snore to LEFTOVERS; Sludge Bomb to BLACK SLUDGE; Hidden
  Power to DAWN STONE; Roar to RED CARD; Exp. Share (Mr. Pokémon) to PRISM SCALE; Kenya reward to EVIOLITE; Sandstorm to
  SMOOTH ROCK; Itemfinder dropped.
- Badge-only HM gifts: Cut comes through badges (Ilex gives Charcoal); HMs are key items.
- Rival naming screen (the name is told to the policeman); starter-dependent rival teams.
- Name Rater service (rename from the party menu instead); friendship-based Happiness Rater meaning.
- Magnet Train service.
- Battle Tower (replaced by Trainer Hill / Battle Tent signs, with no visible facility scripts).
- Lugia needs the Silver Wing; Ho-Oh needs the Rainbow Wing post-E4 → now Clear Bell / Tidal Bell with the Kimono Girls,
  both pre-League.
- The Crystal Suicune chase is reduced to sightings (Burned Tower, Cianwood, Route 42 EUSINE lines). Inferred: Suicune is
  caught as an overworld legendary at Lake of Rage, and Entei and Raikou in Ecruteak and Olivine, instead of roaming.
- Mt. Silver Room 3 with Red indoors; Blackthorn Gym 2F (single floor now).
- Wise Trio NPCs (the room exists, empty).
- Mystery Gift / Pokémon Communication Center GS Ball (the GS Ball is in the Ruins Inner Chamber).

---

### Method (for re-running)

- The text comparison grouped each map's `.string` lines into messages and compared word bigrams with every
  `text/line/cont/para` block in pokecrystal's `maps/*.asm`, `data/text`, `data/phone` and `engine/events`. A best
  coverage under 0.5 counts as NEW.
- Items were resolved with `tools/quetzal_rip/qrom.py` reading the name table up to id 889.
- Maps were identified from `warp_events` and connections. Gym and trainer parties come from `data/trainers.json`; the
  first team per leader is the story team, the `49xx` one the rematch.
