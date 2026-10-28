# Moemon Emerald Kai Co-op: building from source

Based on Kurausukun's `pc_port` branch of pret/pokeemerald (SDL2 native PC port).

## What was changed
* **Moemon Emerald Kai v1.0** content was imported from the Kai IPS. The patch
  was diffed against a byte-matching vanilla build; see `tools/moemon_import/`.
  Imported: battle sprites and palettes (`graphics/moemon/*.bin`), sprite
  coordinates, elevation and animation tables, species info changes (gender
  ratios, held items, safari flee rates, noFlip), evolutions, the
  POKé→MOé text, fonts, title/Pokédex/PokéNav/copyright graphics, the Poké Doll
  icon, the Route 103 Altering Cave cycler and the post-Champion event tickets.
* **Co-op** (`src/mp/`, `include/mp.h`):
  * `mp_net.c`: TCP framing, client, address parsing, SHA-256 room keys, LAN discovery
  * `mp_server.c`: multi-room lobby + relay (in-game local hosting and `tools/mp_server`)
  * `mp_main.c`: config file, lobby session (connect/create/join/rejoin), roster, local state
  * `mp_setup.c`: HOST / JOIN / PLAY SOLO screen after the title screen (uses the
    naming screen, which now accepts real keyboard typing)
  * `mp_avatar.c`: any trainer/NPC sprite for every player animation, recolors,
    trainer card/battle pics, sprite packs
  * `mp_pack.c`: sprite pack PNG atlas format (see the header for the layout),
    loading, exporting; `dist_extras/Sprite Editor.html` edits them
  * `mp_spectate.c`: battle screen streaming for spectators
  * `mp_coop.c`: walk-up co-op Double Battles vs NPC trainers (runs the stock
    `BATTLE_TYPE_TOWER_LINK_MULTI` link mode over `mp_battle.c`)
  * `mp_notify.c`: field notices, sprite PNG drag and drop
  * `mp_portmap.c`: opens the host's router (UPnP IGD / NAT-PMP, background thread,
    leased + cleaned up), CGNAT check, invite codes (public IPv4 + port in 8
    base-32 characters), Radmin VPN / Hamachi fallback
  * `mp_overworld.c`: remote player objects, movement replay, nametags, surf blobs
  * `mp_trainer.c`: BATTLE FLAG trainer mode, the challenge handshake, "Set phrases, or plant?" and the battle phrases
  * `mp_battle.c`: network implementation of the link block API (routed from
    `link.c` when `gNetLinkActive`), so the stock link battle code runs over
    the internet. Also PvP rewards and the whiteout.
  * `mp_menu.c`: START > CO-OP menu
  * `mp_follower.c`: follower Pokémon (SELECT in the party menu). Plain sprites
    that trail their owner; frames are in `graphics/moemon/followers/followers.bin`,
    cut from the overworld sheets by `tools/moemon_followers/slice.py`
    (re-run it to regenerate the bin and `src/data/mp_followers.h`).
  * `mp_debug.c` + harness in `src/platform/sdl2.c`: headless test mode
    (`MOEMON_TEST=script`, `MOEMON_DEBUG_QUICKSTART=1`, ...)

* **Display** (`src/platform/display.c`): borderless fullscreen, pixel-perfect / sharp-fit / stretch scaling,
  filters; set in START > OPTIONS (`src/option_menu.c`: tabs GAME / DISPLAY / CONTROLS / CO-OP).

## Build
Linux: `sudo apt install build-essential libpng-dev libsdl2-dev` then `make linux -j$(nproc)`

Windows (cross-compiling from Linux/WSL): put the SDL2 mingw dev package in
`./SDL2` (so `./SDL2/x86_64-w64-mingw32` exists), have
`x86_64-w64-mingw32-gcc` on PATH, then `make winwsl -j$(nproc)`.
Ship `SDL2.dll` next to the exe. `make ... LOBBY=host[:port]` builds in an
online lobby address (players can also use `lobby.txt` or `lobby =` in the ini).
Linux and Windows builds share `build/pc64`; clean it (or move it aside) when
switching between them.

Server: `cd tools/mp_server && gcc -O2 -pthread -o moemon_server main.c ../../src/mp/mp_net.c ../../src/mp/mp_server.c ../../src/mp/mp_portmap.c ../../src/mp/mp_store.c ../../src/mp/mp_scores.c`
(Windows: `x86_64-w64-mingw32-gcc ... -lws2_32 -liphlpapi -static`). Deploying it:
`tools/mp_server/DEPLOY.md`.

## Releases and the auto-updater
The game updates itself from this repository's GitHub releases (`src/platform/updater.c`, player side in
`dist_extras/README.txt` UPDATES). To publish a version:
1. Bump `MOEMON_VERSION` in `include/mp_version.h` (it must go up: the updater only installs a higher one).
2. Write `tools/release/notes/v<version>.md` (the release's description). Its first line,
   `Moemon Emerald Kai Co-op <version> "<Catchy Title!>": <one line>`, gives the build's title (also the GitHub
   release's name) and the one-line patch note players see in PATCH NOTES on the
   CO-OP setup screen, and the rest is that version's page there (`tools/release/gen_patchnotes.py` builds
   `src/data/mp_patchnotes.h` from every notes file at build time; `## Installing` sections are left out).
3. Commit and push to the default branch. A push that changes `include/mp_version.h` starts
   `.github/workflows/release.yml` (it can also be started by hand: Actions > Release > Run workflow; a version
   that already has a release is skipped). It creates the tag `v<version>` itself.

The workflow builds both games and servers, packages
`MoemonEmeraldKaiCoop-windows.zip` / `-linux.zip` (`tools/release/package.sh`), writes `update.txt`
(`tools/release/manifest.sh`: version, tag, each zip's size and SHA-256), runs `tools/test/update_test.sh`
and publishes the release. Locally the same steps are `tools/release/build_windows.sh SDL2_DIR`,
`package.sh windows|linux OUT`, `manifest.sh OUT`. The updater only runs in a folder with
`moemon_release.txt` (packaged releases), never in a source checkout.

Bump `MP_PROTOCOL_VERSION` in `src/mp/mp_net.h` whenever the network
protocol or anything that affects link battles changes.

## Dev update channel

The dev channel is opt-in. In `moemon_coop.ini`, set `AllowSmallTweaks = 1`
and restart a packaged game. It fetches the discovery manifest from the
`dev-latest` GitHub prerelease, then downloads the platform zip from the
immutable `dev-<main-version>.<revision>` prerelease named in that manifest.
Main still uses GitHub's latest stable release. Set `AllowSmallTweaks = 0`
and restart to reinstall main, even when main's version is lower. `AutoUpdate = 0`
and `--no-update` still disable downloads; an offline dev executable remains
read-only until it can return to main.

Dev builds have a DEV / SAVING DISABLED window title. They read existing saves,
but never create, overwrite or back up the player save or progress sidecars.
Manual SAVE shows a notice; autosave is off and cannot be enabled in OPTIONS.
The main game's configured autosave interval is preserved for the return to main.
Settings, screenshots and logs may still be written. A dev host's shared storage
is kept in memory; standalone servers retain their usual persistence.

Build locally (no tracked version edits needed for a dev revision):

```sh
make linux DEV_BUILD=1 DEV_REVISION=1 -j"$(nproc)"
DEV_BUILD=1 DEV_REVISION=1 tools/release/build_windows.sh /path/to/SDL2-2.30.9
CHANNEL=dev DEV_REVISION=1 tools/release/package.sh linux /tmp/dev-release
CHANNEL=dev DEV_REVISION=1 tools/release/manifest.sh /tmp/dev-release
NOBUILD=1 DEV_REVISION=1 tools/test/dev_save_test.sh
NOBUILD=1 DEV_REVISION=1 tools/test/dev_update_test.sh
```

The tests need both `pokeemerald64` (main) and `pokeemerald-dev64` (dev).
Dev objects are separate by revision and OS, while generated assets are shared.
The installed filename stays `MoemonEmeraldKaiCoop[.exe]` on both channels.

To publish a dev update, use Actions > Dev Release > Run workflow
and describe what is being tested in `dev_notes`. CI uses its increasing run
number as the revision, builds both platforms, tests save protection and channel
switching, publishes immutable prerelease assets, then updates `dev-latest`'s
manifest. Prereleases never become the latest stable release. To retry an already
published run, start a new workflow run so it receives a new revision.

A main release follows Claude's original version-bump workflow. A separate Dev
Release workflow then refreshes dev from the same commit, preserving parity. Main
publication does not wait for dev. Dev verification downloads the published main
binary instead of recompiling it. Both jobs have a five-minute safety limit. Keep experiments under
`#if MOEMON_DEV_BUILD` until approved for main. Unless the user explicitly asks
for a dev update, changes target main; do not publish an experimental dev release
for an ordinary main change. The automatic dev refresh after a main release is
part of keeping both channels current.
