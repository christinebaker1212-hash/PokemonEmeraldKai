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
  * `mp_net.c`: TCP framing, client, relay server (also used by `tools/mp_server`)
  * `mp_main.c`: config file, connection, roster, local state publishing
  * `mp_avatar.c`: base sprites, recolors, custom PNG sprites (stb_image)
  * `mp_overworld.c`: remote player objects, movement replay, nametags, surf blobs
  * `mp_trainer.c`: BATTLE FLAG trainer mode and the challenge handshake
  * `mp_battle.c`: network implementation of the link block API (routed from
    `link.c` when `gNetLinkActive`), so the stock link battle code runs over
    the internet. Also PvP rewards and the whiteout.
  * `mp_menu.c`: START > CO-OP menu
  * `mp_debug.c` + harness in `src/platform/sdl2.c`: headless test mode
    (`MOEMON_TEST=script`, `MOEMON_DEBUG_QUICKSTART=1`, ...)

## Build
Linux: `sudo apt install build-essential libpng-dev libsdl2-dev` then `make linux -j$(nproc)`

Windows (cross-compiling from Linux/WSL): put the SDL2 mingw dev package in
`./SDL2` (so `./SDL2/x86_64-w64-mingw32` exists), have
`x86_64-w64-mingw32-gcc` on PATH, then `make winwsl -j$(nproc)`.
Ship `SDL2.dll` and `libwinpthread-1.dll` next to the exe.

Server: `gcc -O2 -o moemon_server tools/mp_server/main.c src/mp/mp_net.c`
(Windows: add `-lws2_32`).

Bump `MP_PROTOCOL_VERSION` in `src/mp/mp_net.h` whenever the network
protocol or anything that affects link battles changes.
