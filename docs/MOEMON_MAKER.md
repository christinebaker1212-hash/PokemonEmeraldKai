# Moemon Maker (concept)

Draft, 2026-11-03. Nothing is built yet. The user's idea: use RPG Maker MZ as an easy but powerful editor for new areas,
NPC dialogue, story scripts, wild encounters, trainers, custom items / Pokémon / moves / animations, graphics and
tilesets, GEN 3 2D and GEN 4 3D maps, and the scripts of all four regions as editable commands. MZ's eventing format is
the preferred way to work. Kaisongs are supported, including new tracks defined in the game. The content goes in a
`MoemonMaker/` folder in git and downloads to players' games automatically. It has to keep up with Moemon's own data.

## The decision everything else depends on

**MZ is the editor. Moemon's engine is the runtime.** Nothing made in MZ runs on MZ's JavaScript engine. A compiler
(`tools/maker/`) turns the MZ project into a **Maker pack** that the game loads when it starts, without rebuilding the
exe. Here's why:

- MZ's runtime can't run Kai's battle engine, co-op, the card games, the 3D renderer or anything else that makes Moemon
  Moemon. Rebuilding those in JavaScript would mean writing a second game.
- If the exe needed a rebuild for every map, only people who build the game could test their work. A pack loaded at
  runtime gets hot reload, playtesting from MZ and downloads that don't need a new release.

The game already does this in several places, so the risky parts have been done before:

| Already in the game | What Moemon Maker reuses it for |
|---|---|
| `music/*.kaisong` files loaded at runtime (mp_kaisong.c) | new tracks and song overrides from the pack |
| `sinnoh/*.pak` files read beside the exe (Updater_ExeFile) | the Maker pack itself |
| the songs zip the updater downloads between releases (Updater_Songs*) | automatic pack downloads |
| JOHTO / KANTO flags and vars kept past the save block (mp_region.c) | mod flags and vars that can never break a save |
| SEALED Pokémon (MP_SEALED) | Pokémon from a pack that isn't installed |
| the VS cabinet's pack check by file hashes | everyone in a co-op room has the same pack |
| Lua 5.4, already built in for EDOPro's card scripts | script hooks for custom moves, abilities and items |
| one place that looks up every map header (overworld.c Overworld_GetMapHeaderByGroupAndId) | where pack maps join the world |
| SINNOH maps: ordinary field-engine maps with a 3D skin (s3d.c) | GEN 3 and GEN 4 maps edited in the same editor |

```
MoemonMaker/ (MZ project: JSON)          schema.json (exported by each game build:
        |                                  every map, flag, var, item, species, move,
        v                                  song, script command and its arguments)
  tools/maker/moemake  <------------------------+
   (compile + check)
        |
        v
  MoemonMaker.pak  --release asset-->  updater downloads it  -->  game loads it
        ^                                                            |
        +---- MZ's Playtest button (local build, hot reload) <-------+
```

## Making it easy

### 1. MZ's own event commands, with the meaning they already suggest

Most of MZ's eventing has a direct Emerald equivalent, so most commands keep the meaning modders expect:

| MZ event command | Moemon |
|---|---|
| Show Text (face image, name) | message box (`msgbox`); the face is an optional speaker picture |
| Show Choices | `yesnobox` for two choices, a multichoice for more |
| Conditional Branch | `goto_if_set`, compares, `checkitem`, party checks, money, facing... |
| Control Switches / Variables | flags / vars, chosen by name |
| Control Self Switch | flags local to the event (new, kept in the pack's save sidecar) |
| Common Event | `call` |
| Label / Jump to Label / Exit Event | `goto` / `end` |
| Change Gold / Items / Party Member | `addmoney`, `giveitem` (with the "obtained" jingle), `givemon` |
| Transfer Player | `warp` / `warpsilent` / `warpdoor`, picked by the fade type |
| Set Movement Route (+ Wait) | `applymovement` + `waitmovement` |
| Show Balloon Icon | the exclamation mark and the game's emotes |
| Erase Event | `removeobject` + its hide flag |
| Fadeout / Fadein / Tint / Flash / Shake | `fadescreen`, flash level, screen shake |
| Wait | `delay` |
| Play BGM / ME / SE, Fadeout BGM | `playbgm` (Kaisongs too), `playfanfare`, `playse`, `fadeoutbgm` |
| Battle Processing | trainer or wild battle (by Troop) |
| Shop Processing | `pokemart` |
| Name Input | the naming screen |
| Recover All | heal the party |
| Change Actor Images | `setobjectgraphics` / a look |

**Event pages** (MZ's best feature) need no new runtime support. The compiler turns a page list into what Emerald
already does: an ON_TRANSITION map script that picks the active page (sprite, movement, visibility) and one interaction
script that branches by the same conditions. Triggers map the same way: **Action Button** = talk to the object,
**Player Touch / Event Touch** = a step trigger, **Autorun** = an ON_FRAME map script, **Parallel** = a per-frame hook.

### 2. Every other command as a plugin command that stays current

The game has about 270 script macros (asm/macros/event.inc), plus the specials and Kai's own commands. The MZ plugin
(`js/plugins/MoemonMaker.js`) **builds its plugin commands from schema.json**. Every command shows up with typed
arguments and dropdowns of names: "Give item: SITRUS BERRY x1", never "giveitem 0x8E 1". When a release adds a
command, item or map, the next schema has it, and nobody has to edit the plugin by hand.

### 3. MZ's database tabs where they fit

| MZ tab | Moemon |
|---|---|
| Items | items: pocket, price, description, field / battle use (an existing effect, or a Lua hook) |
| Skills | moves: type, power, accuracy, PP, category, priority, an existing effect or a Lua hook |
| Enemies | species: MHP/ATK/DEF/MAT/MDF/AGI = HP/Atk/Def/SpA/SpD/Spe, types, abilities, learnset, evolutions |
| Troops | trainer parties (levels, held items, moves, nature) and the "enemy" in wild battles |
| Map > Encounters | wild tables: Troop + weight + Region ID = grass / surf / fishing rods / rock smash / headbutt |
| Animations (MV-style) | move animations (see 8 below) |
| Tilesets | GEN 3 tilesets (see 7 below) |
| Common Events | shared scripts |
| Types | Pokémon types (stays Moemon's 18; the tab is read-only) |

MZ's editor can't get new tabs, so anything without a slot (evolutions, learnsets, trainer AI, Pokédex text) is a
**plugin-parameter form** (MZ's `@type struct` gives real form fields, not hand-typed note tags). The check (5) reports
anything missing.

### 4. Drop a picture in and get game-ready art

Front, back, icon, follower and overworld PNGs of any colors are reduced to the game's 16-color palettes by the
compiler, with a GBA-accurate preview. Sprite sheets are checked against the frame layouts the game expects. Big
sprites (mpBig) and the custom-PNG paths the game already uses mean art isn't stuck at the GBA's size limits.

### 5. Playtest launches the real game

The plugin replaces MZ's Playtest: F5 compiles the pack and starts Moemon at that map (NW.js can start programs).
**Test from here** on an event starts the game standing in front of it. With the game running, saving in MZ **reloads
the map in place** (scripts, events, tiles), much like MZ's own F5.

### 6. Quick events and a plain-language checker

MZ's Quick Event Creation (Door / Treasure / Inn) becomes Moemon's: **Trainer, Item Ball, Hidden Item, Sign, Door /
Warp, Nurse, Mart Clerk, Berry Tree, Gift Pokémon, Static Encounter, Gym Leader, Rival**, each a filled-in event to
adjust. Templates cover whole places too: New Route, New Town (Center, Mart, houses), New Gym. The **Doctor** checks a
pack before it compiles and reports problems in plain words:

- "This warp leads to a map that doesn't exist."
- "Trainer BUG CATCHER ROB has no 'already battled' flag, so he battles every time."
- "This line is too wide for the message box." / "The game's font has no Ó."
- "Nothing sets FLAG_MYMOD_GOT_KEY, so this door never opens."
- "The player can't get out of this map" (a walk over the collision and the warps).

An in-game debug window (F9, as in MZ) shows flags and vars by name and lets you change them.

## Making it powerful

### 1. All four regions' scripts as MZ events, both ways

A decompiler turns HOENN's `data/maps/*/scripts.inc`, JOHTO / KANTO's port and SINNOH's converted Platinum scripts
into MZ event lists. It loses nothing: anything without an MZ equivalent becomes a plugin command. Like
`tools/quetzal_rip/check_build.sh`, the proof is a round trip: decompile everything, compile it again, and it must
match the build's own scripts byte for byte.

The base regions are **generated on your machine, never committed** (SINNOH's words and scripts are converted from
the decomp at build time and never go in the repository; 1,693 maps of duplicated JSON don't belong in git either).

### 2. Mods are overlays, never forks

Edits to existing content are stored as patches **by name**: "replace page 2 of LITTLEROOT_TOWN / MOM", "insert after
label X", "add an encounter to ROUTE101 grass". They are never a copy of the whole map. When a release changes a map,
the base is generated again and the overlays apply on top. If the base changed under an edit, you get a three-way
merge with a clear "the original changed here" report and nothing is lost silently. This is how mods keep up with
Moemon.

### 3. Names instead of numbers, checked by every release

Packs refer to everything by name (`ITEM_SITRUS_BERRY`, `MAP_ROUTE101`, `mus_route101`, `SPECIES_...`). Every game
build exports schema.json, and the release workflow compiles everything in `MoemonMaker/` against it, the same way
`save_layout.py` protects saves. A release that would break a pack fails before anyone downloads it.

### 4. Lua hooks for anything the data can't express

The game already has Lua 5.4 inside EDOPro's core. A small sandboxed API (no files, no network, deterministic) makes
custom move effects, abilities, item uses, field specials and map logic possible. Examples: `OnDamageCalc`,
`OnMoveHit`, `OnSwitchIn`, `OnEndTurn`, `OnStep(tile)`, `OnDayChange`, `ScriptCall("name")`. In link battles only the
master's engine runs (the existing rule), so hooks stay in sync as long as everyone has the same pack, which the
lobby already checks.

### 5. New Pokémon, moves and items with IDs that never move

New entries get runtime IDs past the built-in ones (species after NUM_SPECIES, moves after Gen 9's, items after
659). The IDs are given out by name and remembered in a `<save>.maker` sidecar, so a save never changes layout and the
same Pokémon keeps its ID across pack updates. If a Pokémon's pack is missing, the Pokémon is SEALED ("from a pack
that isn't installed"). It is never deleted. Trades with a player who lacks the pack are refused cleanly.

### 6. Kaisongs, including new tracks

- **New tracks**: Moemon Music Studio gets a "Moemon Maker" export that writes `audio/bgm/<name>.kaisong` plus an
  `.ogg` preview, so MZ's own BGM picker and preview work. The pack names the track (`mus_mymod_theme`), the game gives
  it a runtime song ID, and Play BGM, a map's BGM setting, a trainer's music and battle music all accept it.
- **Overrides** keep the existing `<song>@<place>` rules (a place can be a pack map).
- Songs go into gen.py's song table, and an optional `gain=` comes from loudness.py.

### 7. GEN 3 and GEN 4 maps in one editor

**GEN 3 (2D)**: MZ's tile size is set to 16. An Emerald metatile is one MZ tile. A primary + secondary tileset (up to
1,024 metatiles) fits MZ's A5 + B-E sheets (5 x 256 = 1,280). The tile index is the metatile, so behaviors
(grass, water, ledges) come from the metatile's own attributes as they do in the game. Collision and elevation go in
MZ's Region ID layer (2 + 4 bits). New tilesets: paint the 8x8 tiles in any editor and the compiler builds metatiles,
palettes and attributes from an MZ tileset sheet plus a small behavior form.

**GEN 4 (3D)**: a SINNOH map is already a field-engine map (grid, events, warps, co-op) with Platinum's 3D picture
drawn over it. In MZ, a GEN 4 map is an ordinary map with a **3D skin**:

- the collision grid and events are edited as on any map;
- a pre-drawn top-down view of the 3D blocks is the parallax background, so you see what you edit;
- each 32x32 cell picks a block, props come from the pack's prop list (houses, trees, signs), and lights pick an area's
  lighting.

New 3D models come from Blender (`.glb`) through a converter into the same mesh and texture format. Modeling is not
MZ's job, and pretending otherwise would make it worse.

### 8. Move animations

MZ's MV-style cel animations (frames of sprite cells, positions, flashes, sound timing) compile into the effects layer
the game already draws over the frame (as mp_ballfx.c and mp_modernanim.c do), so animations aren't bound by GBA
sprites or palettes. Advanced users can also write the game's own battle-anim commands. Effekseer animations
(MZ's default) are out: shipping a particle runtime to play them is too heavy.

### 9. Co-op from the start

- The lobby compares pack hashes, and a joining player downloads the host's pack.
- Each event chooses **per player** (each friend gets the item) or **once per room** (the boss falls for everyone).
- Pack trainers can be joined like any other, and pack maps show friends, compass arrows and spectating as usual.

### 10. Text that matches the game

MZ's escapes map to the game's codes: `\V[n]` = a var, `\N[1]` = {PLAYER}, `\C[n]` = text colors, plus
`{STR_VAR_n}`. MZ's message preview uses a window skin that matches the GBA box, so line breaks look right in the
editor.

## The folder

```
MoemonMaker/
  project/            the MZ project: data/*.json, img/, audio/ (kaisong + ogg preview), js/plugins/MoemonMaker*.js
                      (MZ's core js/rmmz_*.js and its RTP art are not committed: setup.py fills them from your install)
  packs/<name>/       a pack's own content and its overlays on the base game
  lua/                hooks
tools/maker/          moemake (compiler + Doctor), decompile (base regions -> events), export_schema, setup
```

Generated, never committed: the base regions as events, schema.json and the built `.pak`.

**Distribution**: a push that changes `MoemonMaker/` makes the release workflow build `MoemonMaker.pak` and attach
it as a release asset. The updater downloads it the way it downloads songs, and in-game OPTIONS lists packs with an
on / off switch each.

## Watch out for

- **RPG Maker's licence**: MZ's RTP art and music may only be used in games made with RPG Maker. Moemon's runtime
  isn't, so the compiler refuses RTP files (it knows them by hash). MZ's core scripts stay out of git. Each modder
  needs their own MZ licence.
- **SINNOH stays 1:1 with Platinum**: the base is always regenerated from the decomp, and packs are overlays on it,
  so a pack never rewrites the converted originals in the repository.
- **Saves**: nothing a pack does changes the save's layout. Pack state lives in `<save>.maker` by pack name and
  symbol name, and removing a pack leaves everything else working.

## Build order

1. **M1, the spine**: schema.json export, the pack loader, compiling text and scripts, and overlays on existing
   NPCs. Done when an MZ edit to one of Littleroot's NPCs shows up in the game without a rebuild.
2. **M2, new GEN 3 maps**: tilesets, warps, connections, wild tables, trainers, Quick Events, Playtest launching
   the real game, hot reload, the Doctor.
3. **M3, data**: items, moves, species, art, IDs that never move, new Kaisong tracks.
4. **M4, the four regions as events**: decompiler, byte-for-byte round trip, three-way merge on releases.
5. **M5, GEN 4 3D skins, animations, Lua hooks.**
6. **M6, distribution**: release asset, updater, OPTIONS pack list, lobby pack check.

## Questions for the user

- One pack everyone gets (everything in `MoemonMaker/` together), or many packs players switch on and off?
- Should packs ship with game releases (versioned together), or update between them like the songs zip?
- Only you, or friends too (by pull request)? That decides how strict the Doctor has to be before a merge.
- Should packs be able to change the base story by default, or should each player opt in?
