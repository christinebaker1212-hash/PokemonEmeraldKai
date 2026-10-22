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
  * `mp_trainer.c`: BATTLE FLAG trainer mode and the challenge handshake
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

* **Display** (`src/platform/display.c`, `src/display_menu.c`): borderless
  fullscreen, pixel-perfect / sharp-fit / stretch scaling, filters, START > SETTINGS.

## Build
Linux: `sudo apt install build-essential libpng-dev libsdl2-dev` then `make linux -j$(nproc)`

Windows (cross-compiling from Linux/WSL): put the SDL2 mingw dev package in
`./SDL2` (so `./SDL2/x86_64-w64-mingw32` exists), have
`x86_64-w64-mingw32-gcc` on PATH, then `make winwsl -j$(nproc)`.
Ship `SDL2.dll` next to the exe. `make ... LOBBY=host[:port]` builds in an
online lobby address (players can also use `lobby.txt` or `lobby =` in the ini).
Linux and Windows builds share `build/pc64`; clean it (or move it aside) when
switching between them.

Server: `cd tools/mp_server && gcc -O2 -pthread -o moemon_server main.c ../../src/mp/mp_net.c ../../src/mp/mp_server.c ../../src/mp/mp_portmap.c ../../src/mp/mp_store.c`
(Windows: `x86_64-w64-mingw32-gcc ... -lws2_32 -liphlpapi -static`). Deploying it:
`tools/mp_server/DEPLOY.md`.

## Releases and the auto-updater
The game updates itself from this repository's GitHub releases (`src/platform/updater.c`, player side in
`dist_extras/README.txt` UPDATES). To publish a version:
1. Bump `MOEMON_VERSION` in `include/mp_version.h` (it must go up: the updater only installs a higher one).
2. Write `tools/release/notes/v<version>.md` (the release's description).
3. Commit, push, then tag and push the tag: `git tag v<version> && git push origin v<version>`.

`.github/workflows/release.yml` then builds both games and servers, packages
`MoemonEmeraldKaiCoop-windows.zip` / `-linux.zip` (`tools/release/package.sh`), writes `update.txt`
(`tools/release/manifest.sh`: version, tag, each zip's size and SHA-256), runs `tools/test/update_test.sh`
and publishes the release. Locally the same steps are `tools/release/build_windows.sh SDL2_DIR`,
`package.sh windows|linux OUT`, `manifest.sh OUT`. The updater only runs in a folder with
`moemon_release.txt` (packaged releases), never in a source checkout.

Bump `MP_PROTOCOL_VERSION` in `src/mp/mp_net.h` whenever the network
protocol or anything that affects link battles changes.
