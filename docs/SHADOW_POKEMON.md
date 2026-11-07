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
8. **Shadow Pokémon act darkly in the A-Life** (the user, 2026-11-07: "this will supercede the expectation that Shadow
   Pokemon do not act in a negative way"): negative personality traits are amplified and new ones introduced; they can
   be clingy, selfish, hateful, annoyed, mean and so on, and it lessens as purification nears its end (below,
   "In the A-Life").

## Built so far (in the order docs/ORRE_HANDOFF.md gives)
- **S1, the data** (2026-11-07): tools/orre/shadow_data.py reads Colosseum's Shadow table, its trainers' Pokémon, the
  DP sources, the natures' rates and the rate ratios from common_rel into src/data/orre_shadow.h (48 Shadow Pokémon,
  MAKUHITA to TYRANITAR; species, moves and items are Kai's numbers already: Colosseum keeps Gen 3's). Nature, gender
  and IVs of 255 are chosen when met. Only the three Johto starters have a second trainer (Colosseum's own second
  chance). Rerun after a new rip.
- **S2, the heart gauge** (2026-11-07): src/orre/orre_shadow.c (include/orre_shadow.h): each Shadow Pokémon's record
  (personality + OT ID: Colosseum id, DP in hundredths as the game keeps it, its step count, stored EXP, room for the
  A-Life's traits) in "<save>.orre" with Colosseum's story flags (src/orre/orre_story.c: saved at last), written when
  the game saves and kept with its backups; Orre_Shadow_Drop (the six sources), the gauge's level 0-7,
  Hyper Mode's and CALL's chances by nature and level, the walking / day care step counts (field_control_avatar.c).
  `mp shadow make|drop|steps|stepcount|purify|reload|table`; test shadow_gauge (tools/orre/check_shadow.py recomputes
  every value from the table).
- **S3a, battles** (2026-11-07): MOVE_SHADOW_RUSH (912, after the Maker's slots; Colosseum's row: 90 power, 100
  accuracy, 5 PP, NORMAL, contact, recoil a quarter; Colosseum's name and description from common_rel's description
  messages at .data + 0x647B4; a violet TAKE DOWN in Kai's 2D animations; never copied by METRONOME / MIMIC / SKETCH;
  a Gen 3 move to the GENERATION). In battle a Shadow Pokémon's moves are Colosseum's (pokemon_data.c
  pokemonBiosGetPokemonWazaPtr): slot 0 SHADOW RUSH, slot 1 "????" until the gauge's level 2, slot 2 until 4, slot 3
  until 5; its own moves and PP are never written over (battle_controller_player.c Copy / SetPlayerMonData). The
  battling drop once a battle, the first time it's sent out; no EXP of its own: none at levels 0-2, stored from level
  3 (battle_script_commands.c Cmd_getexp). Test shadow_battle (tools/orre/check_shadow_battle.py). Solo battles only for
  now: in co-op the master's game doesn't know a partner's Shadow Pokémon yet (their records are in their own sidecar).
- Next, S3b: Hyper Mode (each SHADOW RUSH rolls the nature's chance by the gauge's level; not asleep or already in it;
  Colosseum's messages 0x771B / 0x771C), CALL (snaps it out by the CALL chance and takes the CALL drop; wakes a
  sleeper: messages 0x7704-0x7709, 0x771D), the aura, and the dark palettes (below); then the opponents' Shadow
  Pokémon (Cipher's) and co-op.

## Dark palettes (the user, 2026-11-07)
"Along with the aura, I think Pokemon should have special, dark palettes, similar to Shadow Lugia. Since there are a
finite amount of Shadow Pokemon, we can curate what they look like." Every Shadow Pokémon gets its own dark palette,
chosen one by one (in the spirit of XD's Shadow Lugia: darkened, violet-leaning, its eyes and accents kept bright), for
both of Kai's art sets (OPTIONS > CREATURES: MOéMON and POKéMON), its front, back and shiny pictures, and its sprite in
the field and the Garden; it returns to its own colours when purified.

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

## In the A-Life (the MOéMON GARDEN and the follower; decision 8)
The Garden's rule that Pokémon never act in a negative way (docs/POKEMON_GARDEN.md's principles: negative emotions only
from experiences, boundaries as personality never failure, repeats never negative) **does not hold for Shadow
Pokémon**. While a Pokémon is Shadow:
- **Negative traits amplified**: its own temperament's darker side pushed to the extreme (a proud one arrogant and
  dismissive, a shy one withdrawn and snapping when approached, a bold one domineering, a playful one cruel in its
  play), and its emotions skewed: anger, frustration and fear rise easily and last; joy, affection and calm are muted
  and brief.
- **Negative traits introduced**, Shadow-only, whatever its temperament: clingy (shadows its Trainer, crowds them,
  frets or sulks when they attend to others), selfish (hogs berries and toys, guards a favourite spot, takes what
  another Pokémon is holding), hateful (glares, hisses or growls at certain Pokémon or people, keeps away from the
  group), annoyed (irritated by interactions, sighs, turns away, swats a hand off), mean (knocks over another's berry
  pile, shoves past, startles the shy ones on purpose). Each Shadow Pokémon gets a few of these, chosen from its
  nature and temperament, so no two act alike.
- **Interactions land differently**: petting, praise or play may be refused, snapped at or grudgingly put up with;
  boundaries are many and crossing one brings a real outburst. A good moment can still reach it, rarely, and counts
  for a lot.
- **Hyper Mode shows**: now and then an outburst in the Garden (lashing out at the air, a burst of its type's
  particles, other Pokémon scattering), the field's echo of battle's Hyper Mode.
- **It lessens with the gauge**: how strong all of this is follows the heart gauge (its bars, as Colosseum shows
  them): at full the darkest, each bar opening softens it, near the end only flickers of it, purified none: the
  introduced traits go, the amplified ones return to the Pokémon's own temperament. Purification is a story beat in the
  Garden and the JOURNAL.
- **What still holds**: player inactivity never makes it worse (the Garden's first principle stays); Friendship is
  never lowered by the Garden; its darkness is not the player's fault and the journal never says so. The journal
  writes its behavior plainly, in character, as the Shadow's doing.
- **The others**: other Pokémon react to it (avoid it, stand up to it, the gentle ones try to calm it). They understand
  that a Shadow Pokémon's heart was closed off, and **forgive it at different rates** (the user, 2026-11-07): each by
  its own temperament and bond (a gentle or close one soon, a proud or often-wronged one slowly), the hurt fading as
  the gauge opens and after purification, never held against it for good.
- **On screen**: its emote bubbles in a Shadow look of their own (dark, purple-edged: still round, never like a
  Trainer's), and the Shadow aura over its sprite in the Garden and as a follower in ORRE.
- Where it applies: the Garden in ORRE and the follower in ORRE (a Shadow Pokémon never leaves ORRE: decision 2).

## Still open (to decide when each part is built)
- Which newer-generation species become Shadow Pokémon at each GENERATION (4-9), at what levels, and on which Cipher
  trainers (decision 5): a table to propose and check with the user.
- Where a missed Shadow Pokémon is met again (decision 6): Colosseum's own way if the game has one (the 97 records'
  repeats suggest it), else a place to propose.
- The Garden's rate (decision 7): the day care's (source 3) is the natural choice.
- The A-Life details (decision 8): the trait list with each one's behaviors and how often, how strongly each gauge
  bar softens them, whether a good moment in the Garden takes the gauge down too, and how lasting the other Pokémon's
  memories of a Shadow's meanness are.
- Kai's level scaling (FF8-style, OPTIONS > BATTLE): whether it raises Shadow Pokémon (their levels are Colosseum's).
- Shadow Pokémon met at higher GENERATIONs with abilities, natures and moves past Gen 3: their Gen 3 data kept, or
  filled from Kai's species data.
