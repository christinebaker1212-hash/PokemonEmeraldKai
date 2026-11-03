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

Moemon Maker is built from the game's own source and ships in the same exe, hidden behind a code. That is the best
answer to "it has to keep up":

- **It can't fall behind.** Every release of the game is a release of the Maker. Its lists of maps, flags, items,
  species, moves, songs and script commands are the game's own tables, not a copy that someone has to update.
- **What you see is the game.** Maps are drawn by the game's renderer, SINNOH by the game's 3D engine, text by its
  font in its message box, battle animations by its battle engine, and songs played by its Kaisong player. MZ can
  only approximate its runtime. The Maker shows the real thing.
- **Play is one key away.** F5 starts the game from the cursor and F12 comes back to the editor. There's no export
  and no second program.

**The way in** (the user, 2026-11-03; it replaces the DOWN + L + R + START code): the main menu's MOEMON MAKER entry
appears only when both are true:

1. `AllowSmallTweaks = 1` in moemon_coop.ini (the existing small-tweaks switch, gMpConfig.smallTweaks /
   MP_SmallTweaksAllowed; the file is per save file since 1.63.5).
2. **"maker"** has been typed on the keyboard since the game started. It's one more typed word beside "speed" /
   "slow" (text_input.c CheckTypedWords: typed anywhere outside text entry, ignored when AllowSmallTweaks isn't 1).

Typing it plays a chime. On the title screen or main menu, the entry appears at once, and it stays until the game
closes. It isn't remembered, so it's typed again after each start. No shortcut ships beside the game, and there's no
`--maker` start option. Nothing about the Maker is in the player README.

**The dev channel ends with this update** (the user, 2026-11-03: the DEV branch of Moemon Kai is erased). Today
`AllowSmallTweaks = 1` also makes the updater pick the dev channel (updater.c `sWantDev = MP_ConfigAllowsTweaks()`;
BUILDING_COOP.md "Dev update channel"), and dev builds can't save. That would make the Maker unusable, since its
switch is the same setting. From the Maker's first update:

- the updater always follows main, whatever `AllowSmallTweaks` says; the setting only unlocks the small tweaks
  ("speed", "gimme", "maker"...);
- dev-release.yml, the `dev-latest` prerelease and the dev parts of BUILDING_COOP.md / HANDOFF.md go;
- **installs already on a dev build**: their updater only reads `dev-latest` and refuses a "main" manifest, so they'd
  be stranded (and unable to save) if it vanished first. One last dev release is built from the new code: those
  installs take it, and its updater moves them to main on the next start (a channel change installs even an equal
  or lower version). Only then is `dev-latest` deleted.

**"In the game" means the same program, not inside the game's screen.** The Maker never edits in the 240x160
picture. It opens its own desktop window at any size, with menus, toolbars, panels and the mouse, like RPG Maker. The
game's code just draws into it:

- **The map view** is the whole map drawn by the game's own tile renderer at any zoom, not the 15x10 tiles the
  camera shows while playing.
- **The game picture** (F5) is one panel of the editor or its own window, the way display.c already opens extra
  windows for the SPECTATE pop-outs and draws the VS fights' 480x320 picture.
- **Previews** (the message box, a sprite, a battle animation) are small game pictures inside the panels.

```
   Moemon.exe (AllowSmallTweaks = 1, "maker" typed, MOEMON MAKER picked)
   +-- Maker window (1920x1080, resizable): menus, map tree, tile palette, map view, event editor, database
   |       the map view and previews are drawn by the game's own renderers
   +-- Game window (F5): the real game at that spot, 240x160 scaled up, F12 back to editing
```

The Maker is PC only: no Maker page in the Companion App (the user, 2026-11-03).

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
  Moemon (the Maker: edit, F5 play, compile)    moemake (the same compiler, command line, for CI)
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
  maps/<MAP>/map.json      new maps: layout, events (one file per event: events/<name>.json), encounters
  db/                      one file per Pokémon / move / item / trainer
  tilesets/ graphics/ music/ lua/
tools/maker/               moemake (the command-line compiler and checks), the decompiler for the base regions
```

One pack: everything in `MoemonMaker/` is one pack, always on for every player (decision 21). One file per thing,
with stable key order, keeps git diffs small. The Maker's **Changes** panel shows what's changed since the last
publish.

Generated, never committed: the base regions as events and the built `.pak`.

**Distribution** (decisions 7, 8): Publish commits the changed files to the default branch through GitHub's API. A
new workflow (`maker-pack.yml`, on pushes that change `MoemonMaker/`) runs the checks, builds `MoemonMaker.pak` and
puts it on a rolling prerelease (`maker-pack`, never marked latest: the updater reads releases/latest for the game).
The pack records the game version it was built against. Players' updaters fetch it at start the way the DANCE songs
are fetched, and only use a pack built for their game version or older. release.yml only runs when
include/mp_version.h changes, so a publish never makes a game release by itself. Each game release also builds the
pack into its zip, so a fresh install has it without a download.

## Watch out for

- **SINNOH**: the user may edit SINNOH's Platinum scripts and words like any region (decision 20). Claude sessions
  still never write SINNOH dialogue (CLAUDE.md). SINNOH's scripts are converted at build time and never committed,
  so the user's edits live in `MoemonMaker/base/` and replace the converted ones when the game is built and loaded.
  The Maker shows no "Platinum's version" beside them.
- **Saves**: nothing a pack does changes the save's layout. Pack state lives in `<save>.maker` by pack name and
  symbol name, and removing a pack leaves everything else working.
- **"Like MZ" means its workflow**: no RPG Maker code, art, sounds or icons in the Maker.

## When a Claude session changes the base game

The user hands bug fixes to other sessions while editing in the Maker. Their fixes must reach the Maker by themselves,
and neither side may silently undo the other. The design guarantees this:

- **The Maker never keeps its own copy of the base game.** It's inside the exe and reads the build's own maps, tiles,
  wild tables and (once converted, M4) scripts each time it opens. A session's fix released in an update is in the
  Maker as soon as the update is installed. There's nothing to sync or regenerate by hand.
- **Only what the user changed is stored**, as overrides in `MoemonMaker/base/`. Everything else is the base game,
  so a fix anywhere the user hasn't touched simply appears.
- **Every override records what it replaced** ("tile 12,7 was metatile 0x1A, now 0x2B"; "NPC LOCALID_MOM was at
  5,9, now 6,9"; "grass slot 3 was WURMPLE Lv 2-3, now SEEDOT Lv 3"). When the game or the Maker applies it, it first
  checks the base still says the "was" value:
  - same: the override applies as normal;
  - different: the base changed under the user's edit. The override is **not** applied blindly. The Maker lists it
    under Problems as "The game changed here since your edit" with three views (before, the game's new version, the
    user's edit) and buttons: keep mine / take the game's / edit. The game uses the base's new version until the user
    picks, so a session's fix wins by default and nothing breaks.
- **Overrides point at stable names, not positions in a list**: maps by MAP_ name, NPCs by their LOCALID_ constant
  (or, without one, their script + look + spot, with a warning if that's ambiguous), warps by destination, wild slots
  by table and slot, scripts by label. If a session adds an NPC in front of one the user moved, nothing shifts. If a
  session removes or renames something an override points at, the checker reports "This edit points at something
  that no longer exists" and drops nothing on its own.
- **Tile overrides survive a resize**: a map that grew or shrank is detected by its size. Overrides are checked
  against their "was" tiles and flagged if the layout moved under them.
- **The pack follows game releases**: `maker-pack.yml` also runs after every game release, rebuilding and re-checking
  the pack against the new game. A conflict it finds is reported in the run, and the pack keeps the base's version at
  that spot until the user decides (as above).
- **Sessions check first** (CLAUDE.md): before changing something that `MoemonMaker/base/` overrides, a session
  reads the override and asks the user. It never edits `MoemonMaker/` to "fix" the user's work.

The one case that needs the user is a fix that lands **on the very thing the user edited**: the Maker shows both,
and nothing is lost or silently overwritten. Everywhere else, fixes flow in on their own.

## Decisions (the user, 2026-11-03)

Asked as choices, round by round. Later sections follow these, and these win where earlier text differs.

**The program**
1. The Maker lives **inside the game exe** (the main menu's MOEMON MAKER, after "maker" is typed with
   AllowSmallTweaks = 1).
2. UI: **Dear ImGui** (SDL2 back end, docking).
3. Look: **RPG Maker's classic light layout by default, with a dark theme** in the Maker's settings.

**M1's scope**
4. M1 has **tiles + collision, NPCs / signs / items, wild encounters, warps + connections, and custom music**.
5. M1's music: **pick a map's music** (every song and .kaisong, played in the editor through the game's own player),
   **add new tracks** (import a .kaisong as a named track with a runtime song number) and **replace a song somewhere**
   (the `<song>@<place>` rule, by picking). No "Open Music Studio" button.

**Storage and delivery**
6. Edits to existing maps are **overrides by name in `MoemonMaker/base/`**, read at runtime and applied by the
   build. The conversion scripts skip what they list.
7. Edits reach players **between releases**: the `maker-pack` download (Files and git).
8. Publish uses a **GitHub token** pasted into the Maker once, kept on the PC only (in a Maker settings file beside
   the game, never in the save, the repo or a log). It commits through GitHub's API with a `Moemon-Maker:` trailer;
   no git needed.

**Map editing**
9. Collision: **Walkable / Blocked painting** that keeps each tile's elevation (new tiles take their neighbours'),
   with a toggle for Porymap's full collision + elevation grid.
10. Wild tables: **a simple list by default and the exact slots in a tab**, both editable and always in step.
    Day / night and GENERATION pools are tabs too.
11. The map view shows by default: **animated tiles, NPCs as real sprites** (range box when selected), **a 15x10
    screen frame** under the mouse, **grid lines and behavior icons**. All toggle in View.
12. SINNOH in M1: **edit on a top-down view, with a 3D preview panel** through the game's camera. Full 3D editing
    comes later.

**Playing and saving**
13. F5 asks **who to play as each time, remembering the last pick**: a copy of a save file, a story-point preset or
    a quick test party. A test never writes to a real save.
14. The game opens **in its own window** beside the editor. F12 or closing it returns to editing.
15. Saving: **autosave to a recovery file within seconds, Ctrl+S writes `MoemonMaker/`**, and Publish sends what
    was saved.
16. Hot reload: **instantly**. The map you're on updates within a moment of each change, and a running script
    finishes first.

**Events and scripts (M2, M4)**
17. Events: **MZ's indented command list, with Ctrl+T for a text view**, both editable.
18. The text view is a **readable script language in Poryscript's style** (`if / else`, `msgbox(...)`), using the
    command list's names.
19. The existing scripts of **all four regions are converted together**. Each must round-trip byte for byte before
    the conversion is used.
20. SINNOH's scripts and words are **fully editable** by the user (see Watch out for).

**Content (M3 and later)**
21. **One pack, always on** for every player. No OPTIONS pack list, no missing-pack cases.
22. Art: **full colour wherever the game can draw it** (big sprites, the effects layer, pictures pinned to maps),
    reduced to GBA palettes only where it must be, with a before / after preview.
23. Lua hooks come **later, as planned (M5)**. Until then, custom moves and abilities use the game's existing effects.
24. **Only the user** makes content. Publish goes straight to the default branch.

**Safety and shipping**
25. The checker **blocks Publish on errors** (a warp to a missing map, a map you can't leave, a trainer who battles
    forever) and **lists warnings** without blocking.
26. **Publish history in the Maker**: every publish with its date and changes. "Roll back to this" publishes that
    state again as a new publish.
27. PATCH NOTES **never mention the Maker**. They only describe what players notice.
28. Progress: **screenshots of the editor for each feature as it lands**, and each milestone is **released without
    asking**.

## Build order

1. **M1, quick edits on the base game**: drawing any map into any buffer (the refactor above), the editor shell
   (ImGui, classic look + dark theme, map tree, map view, inspector), the way in (AllowSmallTweaks + "maker"), the
   dev channel's end, tiles, collision / elevation, NPCs / signs / item balls / hidden items, warps and connections,
   wild tables (list + slots), music (map music, new tracks, replacements), SINNOH top-down with a 3D preview,
   `MoemonMaker/base/` overrides read at runtime and applied at build, F5 in its own window, instant hot reload,
   autosave + Ctrl+S, Publish by token, the `maker-pack` download, the checker's M1 checks, publish history. Done
   when the user can move an NPC, fix a collision and change a wild slot on ROUTE 101 and in TWINLEAF TOWN, hear a
   new track on a map, see it all in the game at once and publish it, without a session.
2. **M2, events and new GEN 3 maps**: event pages, the command list + text view, compiling text and scripts, new
   maps, the World view, trainers, quick events, the rest of the checker.
3. **M3, the database**: Pokémon, moves, items, art, IDs that never move, the battle bench.
4. **M4, the four regions as events**: the decompiler for all four, byte-for-byte round trip, three-way merge, the
   debugger.
5. **M5, full 3D mode, the animation timeline, Lua hooks, tileset editor.**
