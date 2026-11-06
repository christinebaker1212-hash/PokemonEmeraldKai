# Moemon Maker: handover (2026-11-04)

Where Moemon Maker stands, the whole plan, and what to do next. Read this first, then `docs/MOEMON_MAKER.md` (the
design and the user's 28 decisions, which win wherever anything else differs) and COOP_NOTES "Moemon Maker, M1"
(how each M1 piece works).

**Status: M1 is done (1.80.0). M2 is built: M2a events, M2b new maps and the World view, M2c trainers and the
checker (COOP_NOTES "Moemon Maker, M2a / M2b / M2c"). M3a is built: the database for the game's own Pokémon, moves and items, their pictures from PNGs and the battle bench with a damage estimate (COOP_NOTES "Moemon Maker, M3a: the database"); M3b is built: new POKéMON, moves and items at ids that never move (COOP_NOTES "Moemon Maker, M3b"). M4a is built: the debugger (breakpoints, step, continue, watch: COOP_NOTES "Moemon Maker, M4a: the debugger"); M4b is built: every script block of every region assembles from its own macros at run time to the same bytes (`raw` / `label` event lines; tools/test/maker_decompile.sh, 20148 blocks, 0 differ), and any person, sign or trigger can start its event from the game's own script (tools/test/maker_gamescript.sh; COOP_NOTES "Moemon Maker, M4b"); and M4c's three-way merge (THE GAME CHANGED in the map inspector: keep mine / take the game's; COOP_NOTES "Moemon Maker, M4c"). M4 is done.** The user's overnight list (2026-11-06) included M2-M5, so M2 started without the questions round; the four
open questions below were answered with defaults, listed in COOP_NOTES "Moemon Maker, M2a: events" for the user to
change if they want.

## What Moemon Maker is

The user's own RPG-Maker-MZ-like editor, built into the game exe. It is not RPG Maker itself. The point is that the
user can change the game (maps, people, encounters, music, then events, trainers, new maps, data) without spending a
Claude session.

- **Way in:** `AllowSmallTweaks = 1` in moemon_coop.ini, then "maker" typed at any time after the game starts. The
  main menu then shows MOéMON MAKER. It isn't remembered; there's no shortcut, no `--maker` option, and no mention in
  the PATCH NOTES or the README (decision 27).
- **Window:** the editor opens in its own SDL window (Dear ImGui, docking, classic light theme plus a dark one). The
  game window hides while the game idles in `CB2_MakerWaiting`. F5 plays in the game window; F12 or closing it
  returns to editing.
- **Who makes content:** only the user (decision 24). There's no Maker page in the Companion app; the user rejected
  it.

## The rules every session follows

- **The user's own changes come first.** Anything in `MoemonMaker/`, and any commit without a `Claude-Session:`
  trailer (the Maker's Publish commits carry `Moemon-Maker:`), is the user's work. Before a change, a regeneration or a
  conversion script would alter or overwrite it, check `git log` / `git blame` and ask the user. Never "fix" a user
  edit on assumption.
- **`MoemonMaker/` sits on top of the conversion scripts** (tools/quetzal_port, tools/sinnoh...). Its edits apply at
  run time and are never written into those scripts' output, so a regenerated map keeps them. Each edit line keeps the
  value it replaced (WAS). If the base no longer has that value, the edit is left out and the editor's PROBLEMS lists
  it for the user to resolve.
- **"Makerless" fixes** (ordinary bugfixes by other sessions to maps, scripts and data) need nothing special. The
  Maker reads the game's own data, so they flow in by themselves (MOEMON_MAKER.md "When a Claude session changes the
  base game").
- **The dev update channel ended** with 1.80.0. Every build follows main, `AllowSmallTweaks` only unlocks the small
  tweaks, and no dev updates are pushed. The last dev release, dev-1.80.0.232, moves dev installs to main. The
  dev-release.yml workflow is gone. The `dev-latest` prerelease is the user's to delete; the session's tools can't.
- **Releasing:** each milestone is released without asking (decision 28), with screenshots of each feature sent to the
  user as it lands (SendUserFile). PATCH NOTES describe what players notice and never mention the Maker.

## M1, as built (released in 1.80.0)

| Piece | Where |
|---|---|
| The editor (C++, Dear ImGui 1.91.9b-docking) | `maker/`: maker_ui.cpp (window, menus, toolbar, settings, shortcuts), maker_mapview.cpp (MAP / TILES / INSPECTOR / PROBLEMS, painting, collision, events, 3D preview), maker_edits.cpp (the edit model, the text format, undo, save, recovery), maker_wild.cpp, maker_music.cpp, maker_publish.cpp, maker_icons.cpp |
| Editor ↔ game, C only | `include/maker_api.h` (extern "C"), implemented in `src/maker/*.c`. The editor never touches game internals directly |
| Game side | `include/maker.h`; src/maker: maker_gate.c (way in), maker_main.c (open / close), maker_api.c, maker_names.c, maker_tiles.c (the map renderer into any buffer), maker_objects.c, maker_overrides.c (edit files → patched headers / blocks / wild info), maker_flags.c, maker_wild.c, maker_music.c, maker_play.c (F5, hot reload), maker_publish.c (GitHub API), maker_font.c |
| Names at build time | `tools/maker/gen_names.py` → include/constants/maker_names.h, maker_tilesets.h (map_data_rules.mk) |
| Edit files | `MoemonMaker/base/MAP_X.txt`, one edit a line with WAS and NOW; the format is at the top of src/maker/maker_overrides.c. Music in `MoemonMaker/music/` (new tracks get song ids from MAKER_SONG_FIRST 0x4000, playing on MUS_LITTLEROOT's BGM table entry) |
| Hooks in stock files | overworld.c (Overworld_GetMapHeaderByGroupAndId → Maker_EditedHeader), fieldmap.c (Maker_MapBlocks), tileset_anims.c, tilesets.c, event_object_movement.c (ObjectEvent_MakerPalette, read-only), event_data.c (Maker flags 0x8000-0x9FFF in "<save>.maker", by name), save.c / new_game.c, sdl2.c, display.c, main_menu.c, text_input.c, mp_main.c, mp_kaisong.c, m4a.c, mp_region.c (wild slots applied after the GENERATION pools), field_screen_effect.c (DoMakerReloadWarp), mp_debug.c, s3d.c (S3D_RenderPreview), data/scripts/maker.inc |
| Publish and delivery | The editor commits through GitHub's git data API with a token kept only in the Maker's settings file beside the game (never in the save, the repo or a log). `.github/workflows/maker-pack.yml` runs tools/maker/check_pack.py, then build_pack.sh, then the "maker-pack" prerelease (never latest). The game downloads it at start (updater.c MakerPackWorker) into `MoemonMakerPack/`. The working copy `MoemonMaker/` beside a game wins where it exists (Maker_ReadDir) |
| Tests | `tools/test/maker_{gate,shell,map,edit,wild,music,play,publish,pack,sinnoh}.sh` (maker_github_mock.py stands in for GitHub). Harness: `maker WORDS`, `makershot NAME`, MOEMON_MAKER_UNLOCK=1, MOEMON_MAKER_DIR, MOEMON_GITHUB_API / REPO, MOEMON_MAKER_PACK_URL |

M1's known limits, carried into later milestones:
- Wild tables: slots only. Encounter rates, and tables for maps that have none, come later.
- A new person's script is one of the map's own, or nothing, until M2's events.
- SINNOH is edited top-down by what each tile does, with a 3D PREVIEW panel; a full 3D mode is M5.

## The full plan (MOEMON_MAKER.md "Build order")

1. **M1, quick edits on the base game.** Done; see above.
2. **M2, events and new GEN 3 maps.** Done when the user can make a map, put a person on it with their own
   dialogue, choices, an item and a trainer battle, play it with F5 and publish it, without a session.
   - **M2a, events on existing maps:**
     - **Event pages** as in MZ. Conditions: flags, variables, a self switch, an item, time of day, badge count, and
       co-op "this player has / anyone has". Also a graphic, movement, priority, and a trigger: Talk, Touch, Step on,
       Autorun, Every frame.
     - **The commands:** an indented command list, and a Poryscript-style text view (Ctrl+T) of the same data, both
       editable (decisions 17 and 18).
     - **Picking commands:** a command picker with tabs (Message, Flow, Party & Items, Movement, Screen, Sound, Battle,
       Scenes, Co-op, Advanced) and type-to-find (Ctrl+Space). The commands are read from the build (asm/macros/
       event.inc and the specials), so new game commands appear by themselves.
     - **Names, not numbers:** flags, variables and self switches by name.
     - **Live previews:** the message box, movement routes, songs.
     - **Where used / rename everywhere.**
     - **Quick events:** Trainer, Item Ball, Hidden Item, Sign, Door / Warp, Nurse, Mart Clerk, Berry Tree, Gift
       Pokémon, Static Encounter, Gym Leader, Rival, Cutscene.
   - **M2b, new maps and the World view:**
     - New maps: a blank map or a copy, existing tilesets, their own place in the map tree, warps, connections,
       music, wild tables, a region map name.
     - The World view: a region's maps by their connections; drag to connect, doors to inside maps.
     - Templates: New Route, New Town, New Gym.
   - **M2c, trainers and the rest of the checker:**
     - Trainers: party, AI, items, money, texts, sight. They work with level scaling, TEAM PADDING and co-op joining
       (MP_Coop_JoinTrainerOk).
     - The checker blocks Publish on errors and lists warnings (decision 25). Checks:
       - a flag never set;
       - an event no page can show;
       - a trainer who battles forever;
       - a script that never ends;
       - text that runs off the box;
       - a letter the font doesn't have.
3. **M3, the database.**
   - Pokémon, moves, items and art. Art in full colour wherever the game can draw it (decision 22).
   - IDs that never move (a pack's new entries keep their numbers across releases).
   - The battle bench: a pack trainer in the BATTLE SIMULATOR, with a damage calculator.
   - One pack, always on (decision 21).
   - Custom moves and abilities use the game's existing effects until M5 (decision 23).
4. **M4, the four regions as events.**
   - A decompiler for HOENN, KANTO, JOHTO and SINNOH, all at once (decision 19).
   - Each must round-trip byte for byte before its conversion is used.
   - A three-way merge with game updates.
   - The debugger: breakpoints, stepping, watched flags and variables, and "why isn't this event here?".
5. **M5.** Full 3D mode for SINNOH, the animation timeline, Lua hooks, and a tileset editor.

## Next steps (2026-11-06: M2, M3a and M4a built; start here)

1. **Tell the user the M2 defaults** (COOP_NOTES "Moemon Maker, M2a: events" lists the four open questions and the
   defaults taken) and change whatever they pick differently.
2. **M3b is done** (COOP_NOTES "Moemon Maker, M3b": 128 new POKéMON, 64 moves, 64 items at fixed ids, each from a base).
   Could come later: evolutions and egg moves for new POKéMON, a POKéDEX number of their own, new moves' own words,
   trades refusing a new POKéMON a friend's game doesn't have (now it arrives SEALED there).
3. **M4b is done** (COOP_NOTES "Moemon Maker, M4b": the round trip over all 20148 blocks, and "Start from the game's
   script" for every person, sign and trigger, 7671 scripts). Could come later: friendly commands in place of raw lines
   where they mean exactly the same bytes (msgbox "..." MSGBOX_DEFAULT as `text`), and map scripts (on frame /
   on transition) as events.
4. **M4c is done** (COOP_NOTES "Moemon Maker, M4c"). Could come later: an event started from the game's script
   noticing when that script changed in an update (keep the decompiled body's hash with the event).
5. **M5**: Lua hooks (field and battle), the tileset editor (metatiles and behaviors), the animation timeline and
   SINNOH's 3D models (.glb from Blender placed on SINNOH maps) are built (COOP_NOTES "Moemon Maker, M5: ..."). Could
   come later: the tileset editor's pixel / palette drawing, field animations on the timeline, models in ORRE
   (its own engine, src/orre/o3d.c: never through s3d) and a model's own collision.

## Earlier next steps (M2a done: continue with M2b, then M2c)

1. **Ask the four open questions.** The user liked choices rounds (AskUserQuestion, thorough and specific), so ask
   these and the finer M2a choices that way:
   1. **Existing people's words before M4:** can M2 replace an existing person's script with a new Maker event, so the
      user can rewrite an NPC's dialogue now? The plan otherwise leaves existing scripts to M4.
   2. **SINNOH:** CLAUDE.md says SINNOH's words and scripts are Platinum's own, never written by Claude. Decision 20
      says the user may edit them. Should M2's swap work in SINNOH too, or wait for M4?
   3. **Co-op events:** only the triggering player (like most of the game), or also "for everyone" commands (a
      cutscene everyone sees, an item everyone gets)?
   4. **New maps' tiles:** existing tilesets only (a tileset editor in M5, as planned), or bring importing a tileset
      picture forward?
2. **Build M2a.** Notes for the build:
   - **The compiler:** the game's script engine reads native pointers. ScriptReadPointer (src/script.c) reads 8 bytes
     on this 64-bit port. So the Maker can compile events into script code in its own buffers at run time, with
     pointers to its own text and other scripts. No new script engine is needed.
   - **The text:** encoded with the game's charmap at run time.
   - **Where events live:** in the edit files as new line kinds, keeping M1's one-edit-a-line text format and WAS/NOW
     conflict checks. Self switches and Maker variables go in "<save>.maker" by name, like the Maker flags.
   - **Hot reload:** recompile the edited event; a running script finishes first (decision 16).
   - **Hooks:** the person or sign's script pointer can be swapped where M1 already patches the map header's event
     tables (maker_overrides.c BuildHeader).
3. **Then M2b and M2c,** each released as it lands, with screenshots per feature.

## Things to know before working on it

- **Build:** `make linux -j$(nproc)` builds the Maker into the game (MAKER_* rules in the Makefile). Check the Windows
  side with `tools/release/build_windows.sh SDL2_DIR` before a release. Don't run the Linux tests while the Windows
  build runs: it swaps build/pc64 and makes tests fail at random (it caused a false maker_music failure).
- **SINNOH tests** need `tools/sinnoh/make_pack.sh` run first (sinnoh/ is ignored by git; no ROM needed).
- **Saves never change layout:** tools/test/save_layout.py must pass. The Maker keeps its state in "<save>.maker", not
  in the save.
- **Dev builds** (still buildable by hand with `make linux DEV_BUILD=1 DEV_REVISION=N`) must not write any save or
  side file. tools/test/dev_save_test.sh checks it, and any new "<save>.x" writer needs a MOEMON_DEV_BUILD guard.
  1.80.0's dev build found two writers without one (.region, .pals).
- **The Companion app** is another session's work (android/, src/mp/mp_phone*). Fetch the default branch before
  releasing and take the next free version.
