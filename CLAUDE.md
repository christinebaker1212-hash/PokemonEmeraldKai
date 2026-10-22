# Moemon Emerald Kai Co-op: notes for Claude

Native PC build of Moemon Emerald Kai v1.0 with online 4-player co-op and PvP.
Based on Kurausukun's `pc_port` branch of pret/pokeemerald (commit 1165825), SDL2.

Read `BUILDING_COOP.md` first for the architecture and build steps. The player
docs are in `dist_extras/README.txt` (shipped next to the exe).

## Key locations
- `src/mp/`: all co-op code (net, client, avatar, overworld, trainer mode, battle, menu)
- `include/mp.h`: shared co-op API. `src/mp/mp_net.h` holds the protocol (bump `MP_PROTOCOL_VERSION` on changes)
- `tools/mp_server/`: standalone relay server
- `src/platform/updater.c` + `tools/release/`: the auto-updater and releases. Publishing a version = bump
  `MOEMON_VERSION` (include/mp_version.h), add `tools/release/notes/v<ver>.md`, push the tag `v<ver>`
  (.github/workflows/release.yml builds and publishes; BUILDING_COOP.md)
- `tools/moemon_import/`: scripts that imported the Kai IPS into the decomp (need the user's ROM; not in the repo)
- `graphics/moemon/`: Kai battle sprite and palette blobs (INCBIN'd)
- Hooks in stock files are marked `Moemon Co-op` / `Moemon Kai` (grep for them)

## Building
- Linux: `make linux -j$(nproc)` (needs libsdl2-dev and libpng-dev)
- Windows: `make winwsl -j$(nproc)` with x86_64-w64-mingw32-gcc and the SDL2 mingw package in ./SDL2

## Headless testing
`src/platform/sdl2.c` has a test harness: `SDL_VIDEODRIVER=dummy MOEMON_TEST=script.txt MOEMON_SHOTDIR=dir`
(commands: wait N, press KEYS N, mash KEYS COUNT INTERVAL, shot NAME, quit).
`MOEMON_DEBUG_QUICKSTART=1` (+ `MOEMON_DEBUG_NAME/GENDER/WARP/PARTY/MONEY`) skips the intro.
`MOEMON_COOP_CONFIG=x.ini` and `MOEMON_SAVE=x.sav` let several clients run on one machine.
`MOEMON_TEST_FPS=120` keeps several clients roughly in sync.
Verified this way: 4-player overworld sync, nametags, custom PNG sprite,
BATTLE FLAG -> spot -> walk up -> link battle -> rewards and whiteout, save/continue.

Two-client tests: `MOEMON_LOBBY=127.0.0.1:PORT` plus `MOEMON_COOP_AUTO=host:NAME` / `join:NAME`
and a local `tools/mp_server` build. `MOEMON_DEBUG_PARTY=280:15+` makes a Pokémon one EXP
point short of leveling. Also verified: HOST/JOIN with passwords, sprite packs seen by others,
spectating, co-op Double Battles (accept, decline, one-Pokémon trainer, win with evolutions
and move learning, loss, disconnect).

## Feature notes
Per-feature details are in `docs/COOP_NOTES.md` (read the relevant part before changing a feature): emotes and the
quick pick, controller button icons, new game LOOK step and SKIP THE INTRO, welcome note, emote icons, pings, lending
field moves, rescue, team photo, co-op sprite palettes, joined frames, piggyback, crash report, summary rename and the
POKé BALL page (styles, decorations, special balls), battle speed (intro slide, link READY, fast link sync, turbo,
message holds, low HP beep), co-op partner status and intents, battle emotes, autosave, registered items, followers,
phases 1-4 (dex, buddy bonus, shared PC, trades, dailies/achievements/MoeGatcha, co-op wild battles), difficulty,
TEAM PADDING (host setting: trainers get one extra Moemon in team-ups),
"wait for a partner", co-op healing items, the co-op doubles retrofit, batch 3 link features (record mixing, secret
bases, trainer cards, day care eggs, minigames, contests), LOOK / CREATE sprites, co-op Triple Battles (cards drawn from the user's pixel art: tools/moemon_tricards,
checked pixel for pixel by the tri_cards test; type icons layered beside every battle's boxes), and grouped
stat moves (one animation and message for Growl / Tail Whip on several Pokémon), graphics past the GBA limits (64 sprite palettes,
128 sprites, 256 OAM entries, 128 KB of sprite tiles), co-op EXP (everyone levels
up and picks moves to forget at the same time), and ball entrance / recall effects (mp_ballfx.c: an effects layer
drawn over the frame, not bound by the GBA's sprites or palettes; MY PRESET 1-3, APPLY TO PARTY).
Also the GENESIS / EPOCH BALLS (Red's and Crystal's POKé BALLS: their catch math, throw, send-out, art and sounds;
mp_legacy_*.c, tools/moemon_legacy).
Also the e-Reader Berries (items 377-388, berry numbers 47-58: use BERRY_SLOT / IsValidBerryType, never
`item - FIRST_BERRY_INDEX`), the MoeGatcha daily STANDARD lineup and the PICKUP reroll vote.

Gotchas worth knowing before touching battles:
- Network link (mp_battle.c): queued blocks move into gBlockRecvBuffer in NetLink_GetBlockReceivedStatus, never in
  ResetBlockReceivedFlag; the receive task stops after a B_COMM_TO_CONTROLLER message; link tasks never run twice a frame.
  Test link changes at lag with tools/mp_server/lagproxy.py / `WORST=1 tools/test/regress.sh`.
- Triple Battles: battlers 4/5 exist. Anything doing `battler ^ BIT_FLANK` or assuming positions 0-3 (screen rebuild,
  shadows, Cmd_monbg) breaks for them, often by touching sprite 0 = battler 0's healthbox. Use BATTLE_PARTNER.
- Bump MP_PROTOCOL_VERSION (src/mp/mp_net.h) on any protocol change; tools/test rebuilds the server from it.
- Sprite palettes 16-63 have no bit in a palette mask (`1 << (16 + n)`): fade one sprite's palette by slot with
  MP_BlendSpritePalette / MP_BeginSpritePaletteFade. Ball and co-op palettes live there (see COOP_NOTES, graphics).
- The game clock (RTC) is the PC's, and online the room host's: Berry growth must count on the player's own clock
  (clock.c, MP_ClockShift), never on the shared one, or joining a host in another time zone grows trees by hours.
- Test clients all share one Trainer ID, so anything that moves a Pokémon between games needs `mp giveforeign`-style
  checks (a different OT ID caught the BAD EGG bug). `[heap] out of memory` in a log means the game heap ran out.

## Not yet done or verified
- The Windows exe has never been run on real Windows (it was cross-compiled); it runs under Wine
  (tools/test/wine_smoke.sh: solo, and a co-op battle with a Linux player through the Windows server)
- Co-op battles: only POKé BALLS (wild) and healing items (ItemUseInBattle_Medicine); no items in PvP
- Remote players aren't shown across warps (only the same map or connected maps)
