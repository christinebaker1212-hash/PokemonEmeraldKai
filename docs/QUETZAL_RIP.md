# Kanto and Johto from Moemon Quetzal: the rip and how to port it

This branch holds a full rip of the Kanto and Johto regions of **Moemon Quetzal (English Alpha 9 v1)**, a
pokeemerald-based hack with Hoenn + Johto + Kanto, plus notes for porting them into Emerald Kai. Nothing here is
built into the game yet: `quetzal_rip/` is data for a later session to wire in.

Read in this order:
1. This file: what was ripped, where it is, what is verified, and a porting plan.
2. `docs/quetzal/ENGINE.md`: what Quetzal's engine does differently (tileset split, regions, save data, flags).
3. `docs/quetzal/JOHTO.md` and `docs/quetzal/KANTO.md`: every place compared with Gen 2 (pokecrystal) and with
   Gen 1 / FireRed (pokered, pokefirered): new events, changed dialogue, dropped content, unique mechanics.
4. `docs/quetzal/SPECIALS.md`, `docs/quetzal/SCRIPT_COMMANDS.md`: Quetzal's own specials and script commands.
5. `docs/quetzal/ROM_MAP.md`: where every table is in the ROM.

## What was ripped (all of it is reproducible)

`python3 tools/quetzal_rip/rip.py QUETZAL.gba --crystal PATH/pokecrystal --firered PATH/pokefirered`
regenerates `quetzal_rip/` (the pret checkouts are only used to name maps and identify FRLG tilesets/sprites).
The ROM itself is not in the repo.

| Folder | Contents | Format |
|---|---|---|
| `quetzal_rip/maps/` | 664 maps: 249 Johto (`Johto_*`), 415 Kanto (`Kanto_*`). `map.json` + `scripts.inc` each; `map_groups.json`, `index.json` | Kai's own formats; `map.json` has an extra `quetzal` block (group/num, source map in pokecrystal / pokefirered) |
| `quetzal_rip/scripts/` | scripts, texts and movements used by several maps (`johto_shared`, `kanto_shared`, `shared` = both regions) | Kai macro syntax |
| `quetzal_rip/layouts/` | 547 layouts (`map.bin`, `border.bin`) + `layouts.json` | Kai's |
| `quetzal_rip/tilesets/` | 131 tilesets: `tiles.png`, 16 `.pal`, `metatiles.bin`, `metatile_attributes.bin`; `tilesets.json` (tile/metatile counts, anim callbacks, source) | decomp's. 49 Kanto sets are byte-identical to pokefirered's (named after them) |
| `quetzal_rip/object_events/` | the 382 overworld sprites the maps use (`pics/gfx_<id>.png`, palettes), `object_events.json` (size, frames, which ones are Pokémon + species, which match an Emerald/FRLG sprite) | frames side by side like the decomp's sheets |
| `quetzal_rip/trainer_pics/` | front pics of every Kanto/Johto trainer (`front/pic_<id>.png/.pal`) | 64x64 indexed |
| `quetzal_rip/data/` | `trainers.json` (793 trainers, parties), `wild_encounters.json` (229 maps), `region_map_sections.json`, `flags_vars.json` (every flag/var: where set, checked, hides objects), `id_map.json` (every species/item/move used, with Kai's constant or null) | JSON |
| `quetzal_rip/include/` | `quetzal_constants.h` (every FLAG_QZ/VAR_QZ/TRAINER_QZ/OBJ_EVENT_GFX_QZ/MUS_QZ value), `quetzal_macros.inc` (custom commands, Quetzal specials), `quetzal_ids.h` (Quetzal's species/item/move numbers, for checks only) | |
| `quetzal_rip/previews/` | `johto_overview.png`, `kanto_overview.png`: all outdoor maps stitched by their connections | 1/4 scale |

### Verified

`tools/quetzal_rip/check_build.sh QUETZAL.gba` runs the rip through Kai's own tools:
- all 664 `map.json` files convert with Kai's `mapjson` (events, connections, headers);
- every `scripts.inc`, shared script file and generated `events.inc` assembles with Kai's macros
  (`asm/macros/event.inc`) through `preproc` / `cpp` / `as`;
- **all 12,344 labelled script, text, movement and mart blocks are byte-identical to the ROM** (pointers masked).
  So no command, argument or character was decoded wrongly.
Map renders (`tools/quetzal_rip/render.py`, `overview.py`), sprite sheets and trainer pics were checked by eye.

### Not ripped yet

- **Music.** Johto's songs are Quetzal's own (song ids 559-665 in the table at 0x09397008, standard m4a); the rip
  only names them `MUS_QZ_<id>`. Kanto uses FRLG songs Kai already has.
- **Region map graphics and fly / heal locations** for Johto and Kanto (ENGINE.md has the region logic; the map
  section tables are ripped in `data/region_map_sections.json`).

### Naming conventions in the rip

- Maps: Kanto = `Kanto_<pokefirered name>` (Quetzal kept FRLG's map list; it dropped 8 unused FRLG maps and added
  `Kanto_MtMoon_Route4Cave`, `Kanto_CinnabarIsland_PrimalChamber`, `Kanto_CeruleanCave_RayquazaChamber`).
  Johto = `Johto_<pokecrystal name>` where a Crystal map matches (by dialogue, then hand-checked by warps), else a
  descriptive name (`Johto_SafariZone_Desert`, `Johto_TohjoFallsGiovannisRoom`...). `_Copy` / `_Alt` maps are unused
  duplicates or alternate layouts swapped in by scripts. Constants: `MAP_JOHTO_*`, `MAP_KANTO_*`. Warps back into
  Hoenn use Kai's own map constants (Quetzal's Hoenn groups are stock Emerald's).
- Labels: `<Map>_EventScript_<ROM address>` (also `_Text_`, `_Movement_`, `_Mart_`); a `@` comment above an entry
  point says which object / trigger / sign uses it.
- Symbols that need mapping to Kai are prefixed: `FLAG_QZ_0x....`, `VAR_QZ_0x....`, `TRAINER_QZ_<id>`,
  `OBJ_EVENT_GFX_QZ_<id>`, `MUS_QZ_<song id>`, `MAPSEC_JOHTO_*`, `Qz<Name>_<n>` specials, `qz_*` commands.
  Species/items/moves use their real names (`SPECIES_TOTODILE`, `ITEM_LUM_BERRY`) in Quetzal's spelling; forms that
  share a name get `_2`, `_3`... (`SPECIES_PIKACHU_17`), see `data/id_map.json` for the Quetzal id of each.
- Stock specials Kai has are written by Kai's name. Careful: Quetzal's special *numbers* are not stock (13 were
  inserted), the rip already resolved that.

## What Kai needs before the content can run

In rough order of size. Each item says where the details are.

1. **Regions.** Quetzal decides the region from the map group (34-35 Johto, 36-39 Kanto). Per region it switches:
   the region map + map section names (Johto has its own section table), badges and field-move badge checks,
   Pokédex "caught" records, the "region cleared" bit, fly/heal points, wild/trainer tables. Kai has none of this;
   decide whether Kai keeps one dex/badge set or adds regions (ENGINE.md, "Region").
2. **640-tile primaries.** 8 of the primaries (6 Johto, 2 Kanto) are FRLG-style: 640 tiles / 640 metatiles /
   7 palettes, and their secondaries 384 / 384 / 6. Kai is 512/512/6. Either add the per-layout split Quetzal uses
   (ENGINE.md lists every function it touches) or re-split those tilesets offline. Animated tiles: the callbacks in
   `tilesets.json` (`anim_callback`) are Quetzal code; the FRLG ones match pokefirered's `tileset_anims.c`.
3. **16-bit object graphics ids.** Quetzal's object event struct has a u16 graphics id (Kai's `object_event` macro
   writes a byte; `check_build.sh` shows the truncation warnings). The 382 sprites used are in `object_events/`:
   - 141 are Pokémon overworld sprites (`"pokemon"` in the json; id = species + 256): Kai's follower graphics for that
     species are the natural replacement.
   - 186 match an Emerald or FRLG sprite already (`"matches"`): use Kai's / pokefirered's.
   - the rest are new NPCs (Johto gym leaders, Ethan/Kris/Lyra, Kimono Girls...): add them.
4. **Flags and vars.** The content uses 1,439 flags and 129 vars: Johto in Quetzal's new blocks 0x1000-0x1FFF / vars
   0x5000-0x50FF, Kanto in 0x2000-0x2FFF / 0x5100-0x51FF, plus 164 of Emerald's unused low flags (0x20-0xBE) for item
   balls. Kai needs room for them (Quetzal added a lazily-created 0x800-byte block to SaveBlock1, ENGINE.md).
   `data/flags_vars.json` lists every use. For Kanto, `tools/quetzal_rip/kanto_frlg_flags.json` maps 499 flags and
   61 vars to pokefirered's names by script alignment, so Kanto can take FRLG's flag names.
5. **Pokémon, items, moves.** The trainers, wild tables, gifts and marts use 992 species (629 not in Kai: Gen 4-9
   and forms), 530 items (343 not in Kai: Mega Stones, Z-Crystals, mints, plates...) and 403 moves (191 not in Kai).
   `data/id_map.json` lists each with its use count and Kai's constant (or null). This is the biggest content
   decision: substitute Gen 1-3 species, or extend Kai.
6. **Trainers.** `data/trainers.json`: 32-byte struct, 5 party formats (format 4 = IVs, EVs, nature, ability slot,
   item, moves). Kanto trainers are ids 0x2000+, Johto 0x1000+; gym leaders / E4 have rematch teams. Trainer battle
   type 13 (FRLG's early-rival battle) appears twice (`qz_trainerbattle_13`). Front pics in `trainer_pics/`.
7. **Wild encounters.** `data/wild_encounters.json` in Kai's format with two differences: rock smash has 3 slots
   (Emerald 5), and a new `headbutt_mons` slot (5 mons, Johto routes with trees). Rates for those two are guesses.
8. **Script commands and specials.** Only 6 custom commands are used (`qz_givestorymon`, `qz_takebackstorymon`,
   `qz_setstaticwildmon`, `qz_takepartymonofspecies`, `qz_checkpartymoninlist`, `qz_giveoddegg`) plus
   `qz_givemon_ex` (givemon with the two words stock Emerald leaves 0). 139 Quetzal specials are used; SPECIALS.md
   groups them (Bug Contest, static legendaries, FRLG ports such as Seagallop / elevators / Lorelei's dolls, region
   travel, Celebi time travel, Darkrai/Cresselia dreams, fossils, shops). Special 737 (a Quest Log leftover) is called
   132 times and does nothing: delete those calls.
9. **Music.** Johto's songs are new (`MUS_QZ_*`, not ripped yet); Kanto uses the FRLG songs Kai already has.
10. **Hidden items.** Kanto's hidden-item ids restart at 0 (they are FRLG's hidden-item ids, 183/183); the rip writes
    them as `FLAG_QZ_KANTO_HIDDEN_ITEM_<n>`. Johto has no hidden items.

## A porting plan that keeps every step testable

1. Pick the map group numbers in Kai (append after Kai's 34 groups; note Kai's group 0 has one extra map) and copy
   the wanted `quetzal_rip/maps/*` into `data/maps/`, adding the groups to `data/maps/map_groups.json`;
   append `quetzal_rip/layouts/layouts.json` entries to Kai's `data/layouts/layouts.json` and the tilesets to
   `src/data/tilesets/` (+ headers). Fix item 2 first or Kanto/Johto maps render wrong.
2. Get it building with the placeholder headers: include `quetzal_rip/include/quetzal_constants.h` and
   `quetzal_macros.inc`, add the scripts to `data/event_scripts.s`. Then replace the QZ placeholders step by step
   (flags/vars into a real Kai range, trainers into `src/data/trainers.h`, graphics ids, songs).
3. Warp in with the debug quickstart (`MOEMON_DEBUG_WARP`) and walk the maps; the overview PNGs show how the outdoor
   maps fit together.
4. Port the systems in the order the story needs them: JOHTO.md "Story progression gates" and KANTO.md list them.

Things that are easy to miss:
- New Bark's two "Pokémon Center" maps are the player's house (Mom heals you).
- `Johto_*_Copy` maps are unused duplicates; `*_Alt` are alternate layouts a script swaps in with
  `setmaplayoutindex` (Tin Tower roof, Silver Cave summit, Dept. Store roof).
- Quetzal has followers, Nuzlocke / randomizer options, a level cap, multiplayer ("MPLAYER") and a "Giovanni mode"
  (the Celebi alternate past): scripts check these through specials; Kai has its own versions of some (followers,
  co-op). SPECIALS.md says which special is which.
- No TMs/HMs in Quetzal's Kanto: TM gifts became held items; HMs are key items / badge-based (KANTO.md section 1).
