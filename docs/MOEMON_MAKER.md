# Moemon Maker (concept)

Draft, 2026-11-03 (revised the same day: Moemon Maker is our own program that works like RPG Maker MZ, not MZ itself).
Nothing is built yet.

The user's idea is an easy but powerful program for making new areas, NPC dialogue, story scripts, wild encounters,
trainers, custom items / Pokémon / moves / animations, graphics and tilesets, GEN 3 2D and GEN 4 3D maps, with the
scripts of all four regions as editable commands. It should work like MZ: a map editor, events with pages and an
indented command list, and a database with tabs. It supports Kaisongs, including new tracks defined in the game. The
content goes in a `MoemonMaker/` folder in git and downloads to players' games automatically. It has to keep up with
Moemon's own data.

## Why first: hands-on fixes without a Claude session

Most changes the user wants day to day are small and visual: a tile, a collision, where an NPC stands or walks, a
warp, an item ball, a wild table. Today each one costs a session, tokens and a wait for a release. The Maker makes
them a few clicks: see the problem, fix it, press F5 to check, Publish. So **the first milestone is these quick edits
on the base game**, before packs, new Pokémon or anything else (see Build order).

What that needs to get right:

- **Edits to the base game go in one place.** All four regions' maps are in the repo (`data/maps`, `data/layouts`,
  `src/data/wild_encounters.json`), but some were written by generators (tools/quetzal_port, tools/sinnoh). A later
  session re-running one would quietly wipe a hand edit. So the Maker keeps base-game edits as **overrides by name** in
  `MoemonMaker/base/` (a tile at x,y of `MAP_ROUTE101`, `TRAINER_ROB` moved to 12,7, a slot of a wild table).
  **`MoemonMaker/` takes precedence over the conversion scripts** (the user, 2026-11-03): the build applies the
  overrides on top of the generated data, and each converter reads `MoemonMaker/base/` and leaves what it lists
  untouched, so a re-run can't wipe them.
- **New maps can't collide.** A map made in the Maker exists only in `MoemonMaker/`, so no converter writes it.
- **Claude sessions check before touching the user's work.** The rule is in CLAUDE.md (the user, 2026-11-03).
  Publish commits from the Maker carry a `Moemon-Maker:` trailer and no `Claude-Session:`, so the user's edits are
  easy to tell apart in `git log`.
- **SINNOH**: the walkable grid comes from the 3D pack's blocks (s3d.c S3D_LayoutBlocks), not the map's layout, so
  the same override file holds SINNOH's collision and behavior edits. The Maker draws them over the 3D picture.
- **No rebuild to see it**: the game reads `MoemonMaker/base/` at runtime too, so F5 and hot reload show the fix
  right away. A release bakes it in for everyone.
- **Publishing without a session**: Publish commits and pushes the changed override files (git, or GitHub Desktop).
  Base edits can ride the next game release, or go out as the downloadable pack between releases without a version
  bump (see "Files and git").

Until the Maker exists, Porymap (the community map editor for the pokeemerald decomp) can probably open HOENN's,
JOHTO's and KANTO's maps in this repo for tiles, collision, events and wild tables. That's untested against Kai's
changes. Seeing a result needs a local build, and SINNOH's walkable grid isn't in its layouts, so Porymap's edits
there do nothing.

## Two decisions everything else depends on

### 1. The Maker is part of the game

Moemon Maker is built from the game's own source and ships in the same exe (`Moemon --maker`, a "Moemon Maker"
shortcut beside the game, and an entry on the title screen). That is the best answer to "it has to keep up":

- **It can't fall behind.** Every release of the game is a release of the Maker. Its lists of maps, flags, items,
  species, moves, songs and script commands are the game's own tables, not a copy that someone has to update.
- **What you see is the game.** Maps are drawn by the game's renderer, SINNOH by the game's 3D engine, text by its
  font in its message box, battle animations by its battle engine, and songs played by its Kaisong player. MZ can
  only approximate its runtime. The Maker shows the real thing.
- **Play is one key away.** F5 starts the game from the cursor and F12 comes back to the editor. There's no export
  and no second program.

**"In the game" means the same program, not inside the game's screen.** The Maker never edits in the 240x160
picture. It opens its own desktop window at any size, with menus, toolbars, panels and the mouse, like RPG Maker. The
game's code just draws into it:

- **The map view** is the whole map drawn by the game's own tile renderer at any zoom, not the 15x10 tiles the
  camera shows while playing.
- **The game picture** (F5) is one panel of the editor or its own window, the way display.c already opens extra
  windows for the SPECTATE pop-outs and draws the VS fights' 480x320 picture.
- **Previews** (the message box, a sprite, a battle animation) are small game pictures inside the panels.

```
   Moemon.exe --maker
   +-- Maker window (1920x1080, resizable): menus, map tree, tile palette, map view, event editor, database
   |       the map view and previews are drawn by the game's own renderers
   +-- Game window (F5): the real game at that spot, 240x160 scaled up, F12 back to editing
```

**Later, from the phone:** the Companion App could get a small Maker page over its existing link for quick fixes on
the go (move an NPC, change a wild slot), saved as the same `MoemonMaker/base/` overrides. The user liked the idea
(2026-11-03). It comes after the PC Maker exists.

Moemon Music Studio is the opposite example: a separate program with a port of the game's sound engine. It has to be
"checked against" the game whenever the engine changes. The Maker shouldn't have that problem.

The cost is that the game's code expects to own the whole program (one main loop, one current map). The editor needs
the map renderer and the 3D renderer to draw any map into any buffer, not only the one being played. That is a
refactor of the field drawing code, and M1 has to do it first (see Build order).

### 2. Content is packs loaded at runtime

What you make compiles into a **Maker pack** that the game loads when it starts. There's no rebuild of the exe, so
anyone can make content, the editor's F5 works instantly, and players get new content without a new game release.
The game already does each risky part somewhere:

| Already in the game | What the Maker reuses it for |
|---|---|
| `music/*.kaisong` loaded at runtime (mp_kaisong.c) | new tracks and song overrides |
| `sinnoh/*.pak` beside the exe (Updater_ExeFile) | the pack itself |
| the songs zip the updater downloads between releases (Updater_Songs*) | automatic pack downloads |
| JOHTO / KANTO flags and vars kept past the save block (mp_region.c) | pack flags and vars that never break a save |
| SEALED Pokémon (MP_SEALED) | Pokémon from a pack that isn't installed |
| the VS cabinet's pack check by file hashes | everyone in a co-op room has the same pack |
| Lua 5.4, already built in for EDOPro's card scripts | hooks for custom moves, abilities and items |
| one place that looks up every map header (overworld.c Overworld_GetMapHeaderByGroupAndId) | where pack maps join the world |
| SINNOH maps: ordinary field maps with a 3D skin (s3d.c) | GEN 3 and GEN 4 maps in one editor |
| the world grid of HOENN from map connections (mp_compass.c) | the world view of every region |
| the BATTLE SIMULATOR (mp_sim.c), headless battle anims (test anim_modern) | battle and animation previews |
| the headless test harness (sdl2.c MOEMON_TEST) | automated checks of every pack in CI |

```
MoemonMaker/ (JSON, one file per map / event / entry)
        |
        v
  Moemon --maker  (edit, F5 play, compile)        moemake (the same compiler, command line, for CI)
        |                                                 |
        v                                                 v
  MoemonMaker.pak (local, hot reload)             MoemonMaker.pak (release asset) --> updater --> players
```

## The editor, laid out like MZ

```
+-------------------------------------------------------------------------------------+
| File Edit Mode Draw Scale | Tools: Database  Events  Music  Text  World  Check | > F5 |
+---------------+-------------------------------------------------------+-------------+
| Tileset       |                                                       | Inspector   |
| (metatiles,   |            Map view (the game's own renderer,         | (selected   |
|  tabs per     |             2D or 3D, any zoom)                       |  tile /     |
|  tileset)     |                                                       |  event /    |
+---------------+                                                       |  warp)      |
| Map tree      |                                                       |             |
| HOENN         |                                                       |             |
|  > ROUTE 101  |                                                       |             |
| JOHTO ...     |                                                       |             |
+---------------+-------------------------------------------------------+-------------+
| Problems (3)  |  Find results  |  Output                                              |
+-------------------------------------------------------------------------------------+
```

The UI is drawn with Dear ImGui (MIT, has an SDL2 back end and docking; the build already compiles C++). It's
themed to feel like a classic Windows tool editor: familiar to anyone who's used MZ or Porymap, with modern docking,
search and undo. Panels can be moved anywhere, and every panel remembers its place.

### Modes (as in MZ)

- **Map**: paint metatiles on the bottom / top layers, with pencil, rectangle, fill, stamps of
  multi-tile pieces, and a picker (right-click takes a tile from the map).
- **Collision / elevation**: a paint mode with colors and numbers over the map, as in Porymap.
- **Behaviors**: icons for grass, water, ledges, doors and warps, so nothing hidden surprises you.
- **Events**: place and edit people, signs, triggers, warps, item balls and hidden items.
- **Encounters**: the map's wild tables beside it, with the grass / water / rock tiles they apply to highlighted.
- **3D** (GEN 4 maps): the same map in the 3D engine with an orbit camera. Pick a block per 32x32 cell, place props
  in 3D, and edit the walkable grid over the 3D picture. Tab switches between the flat and 3D views of the same map.
- **World**: every map of a region joined by its connections. Drag maps to connect them, and see doors to inside maps.

### Events (MZ's best feature, kept and improved)

An event has **pages**. Each page has conditions (flags, vars, a self switch, an item, time of day, badge count,
co-op: "this player has / anyone has"), a graphic (any look or Moemon), movement, options, priority, a trigger
(**Talk**, **Touch**, **Step on**, **Autorun**, **Every frame**) and an indented command list:

```
◆Text: [NURSE JOY] Welcome to the POKéMON CENTER!
◆Show Choices: HEAL, NO THANKS
: When HEAL
  ◆Play Fanfare: mus_heal
  ◆Heal Party
  ◆Text: We hope to see you again!
: When NO THANKS
  ◆Text: Please come again.
:End
```

Commands are added by double-clicking, from a picker with tabs (Message, Flow, Party & Items, Movement, Screen,
Sound, Battle, Scenes, Co-op, Advanced) or by **typing** (Ctrl+Space and a few letters: "giv" finds Give Item, Give
Pokémon, Give Egg). Everything is chosen by name from dropdowns with search and pictures (items with icons, Pokémon
with sprites, songs with a play button, maps with a thumbnail).

All of the game's script commands are there, about 270 macros plus the specials and Kai's own. The editor reads them
from the build, so a command added to the game shows up in the next release of the Maker.

Things MZ doesn't have:

- **Two views of one script**: the command list and the same script as text (Ctrl+T), both editable, always the same
  data. Beginners click and power users type.
- **Live previews**: Show Text draws the game's real message box with its font and line breaks, and warns before a
  line runs off. A movement route plays on the map. A Kaisong plays from its row.
- **Where used / rename everywhere**: right-click any flag, var, item, trainer, song or label to see every place that
  uses it, or rename it in all of them.
- **Names, not numbers**: making a flag is typing its name ("GOT_SILPH_KEY"). The pack gives it a slot, and you never
  see a number.
- **Undo and redo for everything**, including painting, database edits and moves between maps, plus copy and paste of
  events, pages and commands between maps.
- **Quick events**: Trainer, Item Ball, Hidden Item, Sign, Door / Warp, Nurse, Mart Clerk, Berry Tree, Gift Pokémon,
  Static Encounter, Gym Leader, Rival, Cutscene. Each one is filled in and ready to adjust. Templates cover whole places
  too: New Route, New Town (Center, Mart, houses), New Gym.

### Play and debug

- **F5 / Play from here**: starts the game at the map, or in front of the selected event. Choose who you play as from
  a save, a "story point" preset (start, 4 badges, Champion) or a quick party.
- **Hot reload**: edit while the game runs, and the map reloads in place (tiles, events, scripts).
- **The debugger**: breakpoints on event commands, stepping one command at a time, and a watch list of flags and vars
  by name, editable live. **"Why isn't this event here?"** shows each page's conditions and which one failed.
- **A battle bench**: run a pack trainer against your party in the BATTLE SIMULATOR to test the fight, with a damage
  calculator beside it.

## The database (tabs, as in MZ, but made for Moemon)

| Tab | What's in it, with its previews |
|---|---|
| **Pokémon** | stats, types, abilities, learnsets by level / TM / tutor / egg, evolutions, Pokédex text, forms; animated front / back / icon / follower sprites, the cry, stat bars against the species of its type |
| **Moves** | type, power, accuracy, PP, category, priority, flags, effect (an existing one or a Lua hook), animation; a preview of the move in a real battle scene |
| **Items** | pocket, price, description (wrapped as the game wraps it), field and battle use, icon |
| **Trainers** | class, picture, party (levels, held items, moves, nature, IVs), AI, money, intro / defeat text, music; tested in the battle bench |
| **Encounters** | each map's wild tables with percentages, time of day and GENERATION caps |
| **Tilesets** | metatile editor: 8x8 tiles, palettes, layer type, behavior, animated tiles |
| **Animations** | battle and field animations on a timeline over a live battle scene (see 8 below) |
| **Music** | every song and Kaisong, where each plays, new tracks |
| **Text** | every line of a pack in one table, for proofreading, with a check for letters the font doesn't have |
| **Types** | read-only: Moemon's 18 types and the type chart |

## The powerful parts

### 1. All four regions' scripts as events, both ways

A decompiler turns HOENN's `data/maps/*/scripts.inc`, JOHTO / KANTO's port and SINNOH's converted Platinum scripts
into events with pages and command lists. It loses nothing: anything without a friendly command shows as its raw
command, and that is still editable. Like `tools/quetzal_rip/check_build.sh`, the proof is a round trip: decompile
everything, compile it again, and it must match the build's own scripts byte for byte.

The base regions are **generated on your machine, never committed**. SINNOH's words and scripts are converted from the
decomp at build time and never go in the repository, and 1,693 maps of duplicated JSON don't belong in git either.

### 2. Changes to the base game are overlays, never forks

Edits to existing content are stored as patches **by name**: "replace page 2 of LITTLEROOT_TOWN / MOM", "insert
after label X", "add an encounter to ROUTE101 grass". They are never a copy of the whole map. When a release changes a
map, the overlays apply to the new one. If the original changed under an edit, the Maker shows a three-way merge
(base before, base now, your edit) and loses nothing silently. This is the other half of keeping up with Moemon.

### 3. Every release checks every pack

The command-line compiler (`moemake`) builds everything in `MoemonMaker/` in the release workflow. It then runs a
headless check of every pack: each map loads, each warp arrives somewhere, each trainer can be battled. This works the
same way `save_layout.py` protects saves. A release that would break a pack fails before anyone downloads it.

### 4. The checker

The **Problems** panel updates as you work and explains in plain words:

- "This warp leads to a map that doesn't exist."
- "Trainer BUG CATCHER ROB has no 'already battled' flag, so he battles every time."
- "This line is too wide for the message box." / "The game's font has no Ó."
- "Nothing sets GOT_SILPH_KEY, so this door never opens."
- "The player can't get out of this map" (a walk over the collision, warps and connections).

### 5. Lua hooks for anything the data can't express

The game already has Lua 5.4 inside EDOPro's core. A small sandboxed API (no files, no network, deterministic) makes
custom move effects, abilities, item uses, field specials and map logic possible. Examples: `OnDamageCalc`,
`OnMoveHit`, `OnSwitchIn`, `OnEndTurn`, `OnStep(tile)`, `OnDayChange`, `ScriptCall("name")`. Hooks are edited in the
Maker with highlighting and the same debugger. In link battles only the master's engine runs (the existing rule), so
hooks stay in sync as long as everyone has the same pack, which the lobby checks.

### 6. New Pokémon, moves and items with IDs that never move

New entries get runtime IDs past the built-in ones (species after NUM_SPECIES, moves after Gen 9's, items after
659). The IDs are given out by name and remembered in a `<save>.maker` sidecar, so a save never changes layout and the
same Pokémon keeps its ID across pack updates. If a Pokémon's pack is missing, the Pokémon is SEALED ("from a pack
that isn't installed"). It is never deleted. Trades with a player who lacks the pack are refused cleanly.

### 7. Kaisongs, including new tracks

- **New tracks**: drop a `.kaisong` into the Music tab, or press "Make in Music Studio" to open the studio with a new
  song and import it on export. The pack names the track (`mus_mymod_theme`), and the game gives it a runtime song
  ID. Play BGM, a map's music, a trainer's music and battle music all accept it.
- **Overrides** keep the existing `<song>@<place>` rules (a place can be a pack map).
- Songs play in the editor through the game's own Kaisong player, so the editor sounds exactly like the game. They go
  into gen.py's song table, with an optional `gain=` from loudness.py.

### 8. Graphics and animations

- **Pictures**: drop a PNG of any colors. It's reduced to the game's 16-color palettes with a GBA-accurate preview, and
  sheets are checked against the frame layouts the game expects. Big sprites (mpBig) and custom-PNG sprites mean art
  isn't stuck at GBA sizes.
- **Animations**: a timeline (frames of sprite cells, positions, flashes, shakes, sound cues, particles) drawn by the
  effects layer the game already puts over the frame (as mp_ballfx.c and mp_modernanim.c do), so animations aren't
  bound by GBA sprites or palettes. They preview in a live battle scene. Advanced users can also write the game's own
  battle-anim commands.
- **3D**: new models come from Blender (`.glb`) through a converter into the 3D engine's mesh and texture format. The
  Maker places and previews them, and modeling stays in Blender.

### 9. Co-op from the start

- The lobby compares pack hashes, and a joining player downloads the host's pack.
- Each event chooses **per player** (each friend gets the item) or **once per room** (the boss falls for everyone).
- Page conditions can ask about the room ("anyone has the badge").
- Pack trainers can be joined like any other, and pack maps show friends, compass arrows and spectating as usual.

## Files and git

```
MoemonMaker/
  base/                    edits to the base game: overrides by name (tiles, collision, NPCs, warps, wild...)
  packs/<name>/
    pack.json              name, version, author, which game versions
    maps/<MAP>/map.json    layout, events (one file per event: events/<name>.json), encounters
    overlays/              patches on the base game, by name
    db/                    one file per Pokémon / move / item / trainer
    tilesets/ graphics/ music/ lua/
tools/maker/               moemake (the command-line compiler and checks), the decompiler for the base regions
```

One file per thing, with stable key order, keeps git diffs small and merges rare. The Maker's **Changes** panel shows
what you've edited since the last commit. A **Publish** button commits and pushes through git when it's installed.

Generated, never committed: the base regions as events and the built `.pak`.

**Distribution**: a push that changes `MoemonMaker/` makes the release workflow build `MoemonMaker.pak` and attach
it as a release asset. The updater downloads it the way it downloads songs, and in-game OPTIONS lists packs with an
on / off switch each.

## Watch out for

- **SINNOH stays 1:1 with Platinum**: the base is always regenerated from the decomp, and packs are overlays on it,
  so a pack never rewrites the converted originals in the repository.
- **Saves**: nothing a pack does changes the save's layout. Pack state lives in `<save>.maker` by pack name and
  symbol name, and removing a pack leaves everything else working.
- **"Like MZ" means its workflow**: no RPG Maker code, art, sounds or icons in the Maker.

## Build order

1. **M1, quick edits on the base game**: drawing any map into any buffer (the refactor above), the editor shell (map
   tree, map view, inspector), painting tiles, collision / elevation, moving and setting up NPCs (position, look,
   movement, range), warps, signs, item balls, wild tables, `MoemonMaker/base/` overrides read at runtime and applied
   at build, F5 play from here, hot reload, Publish. Done when the user can move an NPC and fix a collision on ROUTE
   101 and in TWINLEAF TOWN, see it in the game at once and publish it, all without a session.
2. **M2, events and new GEN 3 maps**: event pages and the command list, compiling text and scripts, the pack loader,
   new maps, connections and the World view, trainers, quick events, the checker.
3. **M3, the database**: Pokémon, moves, items, art, IDs that never move, new Kaisong tracks, the battle bench.
4. **M4, the four regions as events**: decompiler, byte-for-byte round trip, three-way merge, the debugger.
5. **M5, 3D mode, the animation timeline, Lua hooks, tileset editor.**
6. **M6, distribution**: moemake in the release workflow, the updater, the OPTIONS pack list, the lobby pack check,
   Publish.

## Questions for the user

- One pack everyone gets (everything in `MoemonMaker/` together), or many packs players switch on and off?
- Should packs ship with game releases (versioned together), or update between them like the songs zip?
- Only you, or friends too (by pull request)? That decides how strict the checks have to be before a merge.
- Should packs be able to change the base story by default, or should each player opt in?
- The Maker inside the game exe (recommended above), or its own exe like Music Studio?
