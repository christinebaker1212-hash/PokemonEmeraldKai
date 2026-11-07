# Shadow Pokémon in ORRE: the design

Settled before any of ORRE's story (the user, 2026-11-07: "we should figure out Shadow Pokemon and how they should work
before working on any part of the story"). Nothing here is built yet. The user's decisions come first; then how
Colosseum does it (from the decomp and the disc), how that maps onto Kai, and what is still open.

## The user's decisions (2026-11-07)
1. **Colosseum 1:1.** Colosseum's 48 Shadow Pokémon with their own data, Shadow Rush the only Shadow move, Hyper Mode
   (not XD's Reverse Mode), the game's own heart gauge rates, purification at AGATE VILLAGE's Relic Stone. No XD extras.
2. **They can't leave ORRE until purified.** No voyage out with one in the party, no withdrawing one from a PC outside
   ORRE. ("Shadow Pokémon in Orre only", ORRE_DECISIONS.md, made exact.)
3. **No battles or trades but ORRE's own until purified.** No PvP / link battles, no POKéMON STADIUM, BATTLE SIMULATOR,
   BATTLE FRONTIER or other facility; no trades of any kind (co-op trades, the shared PC, the phone app's trades by
   mail). Co-op story battles in ORRE are fine.
4. **Co-op snagging: the first to snag gets it.** One Shadow Pokémon in the battle; whoever's ball catches it first keeps it.
5. **More Shadow Pokémon by GENERATION.** At GENERATION 3 it is exactly Colosseum's 48. From GENERATION 4 up, Cipher's
   trainers (Peons, Admins, the Shadow-using bosses: never ordinary ORRE trainers, never other regions) also carry
   Shadow Pokémon of the newer generations, **added on top** of Colosseum's team (as TEAM PADDING adds a member): no
   team loses one of Colosseum's own Shadow Pokémon.
6. **A missed Shadow Pokémon gets a second chance later** in ORRE: none is lost for good.
7. **Purification: Colosseum's ways, and the MOéMON GARDEN too.** Battling, walking, CALL, the scents, the day care
   and AGATE's Relic Stone as Colosseum has them, plus time in Kai's Garden (in ORRE) opening the gauge.

## How Colosseum does it (the decomp: christinebaker1212-hash/pkmn-colosseum; Colosseum calls them "dark Pokémon")
- **The Shadow Pokémon table**: common_rel's .data + 0x1439D8, 97 records of 0x38 bytes (count at + 0x14A6F8; the
  decomp's BattleRangeDef in src/game/battle/battle_range_801ED640.c, ids 1-0x60). Per record: the species (Gen 3
  internal numbers: 0x14F MAKUHITA, 0x99 BAYLEEF, 0x9C QUILAVA, 0x9F CROCONAW, 0xA4 NOCTOWL...: Colosseum's own order),
  the level (+ 8), a save slot (+ 0xA) and the story flags of its states (+ 0xC .. + 0x1C: met, snagged, purified...;
  darkPokemonCheckFlag0C / 12 / 14 / 16, darkPokemonGetFlag1C). Ids past the 48 are the same Pokémon met again.
  Their per-save state: darkPokemonSaveGetPtr (0x18 bytes a slot from + 0x49C).
- **Another table** at + 0x14A45C (darkPokemonGetTableEntry: u8, u8, u32 per entry, count + 0x14A708) and an id list
  at + 0x14A5F4 (darkPokemonGetIdByIndex): to be read when they're converted.
- **The heart gauge** is "dark points" (DP), kept on the Pokémon (status 0xC5, a float times a scale), its starting value
  from the species' data (pokemonBiosGetInitDp). pokemonAddDpFormPokemonDpFilterId (src/game/pokemon_range_8011F5FC.c)
  takes it down from a source's "DP filter" value (a table, by source), scaled by the Pokémon's nature (the nature
  table's rate per source, kake / waru):
  0 battling (sent out), 1 walking (field_exact_801ED3B8.c calls it on steps), 2 CALL, 3 the day care (sodateya),
  4 a scent (nadenade: items of kind 6, times the scent's strength), 5 a share of the DP left.
  The gauge's five bars are pokemonGetDarkPokemonLevel (0-7 with the hidden states).
- **Hyper Mode**: pokemonGetHyperModeRate: a rate from the nature table by the gauge's level (pokemonSeikakuDataBios-
  GetHyperRate); CALL takes it out (pokemonSeikakuDataBiosGetCallRate). fightKindDataBiosGetDarkpokemonHypermodeFlag
  says which battles allow it.
- **Purification** ("relive", src/game/pokemon_relive.c): at the Relic Stone (hokora) when the gauge is at zero
  (scriptGetDarkPointZeroPokemonNum): its scene and particles, the EXP it stored given back (_expRecover, with moves
  learned on the way and evolution: pokemonEvolutionAll), its friendship, the NATIONAL RIBBON
  (exribbonSetNarionalRibbon), its own moves revealed.
- In battle: _fightCheckDarkPokemonAppear, fightOutPokemonDarkPokemonEffect (the aura on sending out),
  fightPokemonBiosGet/SetDarkOutFlag.
- The rest of the rules (no EXP or levels until purified, moves shown as ????, no nickname, no trade, no link battles)
  are read from the code as each part is built, not from memory.

## How it maps onto Kai
- **Data**: the 48 (97 records) converted from the disc by a tool (tools/orre/, like colo_rooms.py), never typed in:
  species through Gen 3 internal -> Kai's species, moves, levels, the gauge's DP, the DP filter and nature rate tables.
- **State in a sidecar** ("<save>.shadow": saves never change layout), per Pokémon keyed by its personality and OT ID
  (as the Garden's "<save>.life"): Shadow or not, DP, stored EXP, Hyper Mode, the record it came from. Colosseum's story
  flags for each Shadow Pokémon's states go there too (with src/orre/orre_story.c's flags, saved at last).
- **Battles**: Kai's own engine (the user, 2026-11-04). Shadow Rush as a new move; Hyper Mode as a battle state of the
  Shadow Pokémon (disobeying, the HYPER message, CALL as a battle action in ORRE); the Shadow aura drawn by an effects
  layer over the battle (as mp_ballfx.c / mp_astralfx.c draw theirs). Co-op link battles: the master's engine runs it, so
  the aura, the gauge and Hyper Mode go to the others' screens like battle weather does.
- **Snagging**: the SNAG MACHINE (a key item, Colosseum's story gives it) lets a ball be thrown in a trainer battle at a
  Shadow Pokémon only; co-op: first catch wins (decision 4).
- **The gauge on screen**: the summary's page and the party screen show the five bars (Colosseum's art from the disc
  where it has it).
- **Confined to ORRE** (decisions 2, 3): checks where a Pokémon leaves (the voyage, the PCs outside ORRE, trades, the
  shared PC, PvP and facility entry, the phone app's exports), each with the game's own refusal words where Colosseum
  has them.
- **The Garden** (decision 7): a Shadow Pokémon placed in the Garden in ORRE has its gauge taken down by time there,
  through the same DP formula (its own source, the rate open below).

## Still open (to decide when each part is built)
- Which newer-generation species become Shadow Pokémon at each GENERATION (4-9), at what levels, and on which Cipher
  trainers (decision 5): a table to propose and check with the user.
- Where a missed Shadow Pokémon is met again (decision 6): Colosseum's own way if the game has one (the 97 records'
  repeats suggest it), else a place to propose.
- The Garden's rate (decision 7): the day care's (source 3) is the natural choice.
- Kai's level scaling (FF8-style, OPTIONS > BATTLE): whether it raises Shadow Pokémon (their levels are Colosseum's).
- Shadow Pokémon met at higher GENERATIONs with abilities, natures and moves past Gen 3: their Gen 3 data kept, or
  filled from Kai's species data.
