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
  `MOEMON_VERSION` (include/mp_version.h), add `tools/release/notes/v<ver>.md`, push to the default branch
  (.github/workflows/release.yml builds, tags and publishes; BUILDING_COOP.md). Sessions can't push tags.
- `tools/moemon_import/`: scripts that imported the Kai IPS into the decomp (need the user's ROM; not in the repo)
- `graphics/moemon/`: Kai battle sprite and palette blobs (INCBIN'd)
- Hooks in stock files are marked `Moemon Co-op` / `Moemon Kai` (grep for them)

## Building
- Linux: `make linux -j$(nproc)` (needs libsdl2-dev and libpng-dev)
- Windows: `make winwsl -j$(nproc)` with x86_64-w64-mingw32-gcc and the SDL2 mingw package in ./SDL2
  (or `tools/release/build_windows.sh SDL2_DIR`, which keeps the Windows objects apart). Build it before a release:
  files that include windows.h (display.c, updater.c...) can't name a static helper like a Win32 function (FillRect).

## Headless testing
`src/platform/sdl2.c` has a test harness: `SDL_VIDEODRIVER=dummy MOEMON_TEST=script.txt MOEMON_SHOTDIR=dir`
(commands: wait N, press KEYS N, mash KEYS COUNT INTERVAL, shot NAME, shotburst PREFIX FRAMES [STEP] = a shot every
frame while the script goes on, for glitches that last a few frames, barrier NAME COUNT = wait until COUNT clients
sharing the result folder get there, so they act on the same frame, quit).
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
drawn over the frame, not bound by the GBA's sprites or palettes; MY PRESET 1-6, APPLY TO PARTY; 41 colors, the newer
pieces earned by achievements or MoeGatcha prizes (mp_ballunlock.c); art partly imported from the game's battle anims).
Also the GENESIS / EPOCH BALLS (Red's and Crystal's POKé BALLS: their catch math, throw, send-out, art and sounds;
mp_legacy_*.c, tools/moemon_legacy).
Also PC TAG mode (west button / F: tag Moemon, then MOVE / RELEASE them together; pokemon_storage_system.c end).
Also the CANDY JAR (mp_candy.c: candies from releases, PROF. BIRCH brings the jar, a physics jar menu, CONVERT to RARE
CANDY; ITEM_CANDY_JAR 398). Also the shared PC as 14 boxes in the PC box screen (mp_sharedbox.c: a mirror the stock
screen edits, diffed into server transactions; the player's own boxes are MP_PersonalStorage(), never assume
gPokemonStoragePtr is theirs while the PC is open) with MY PC, SEND TO, friends' cursors, and its ITEMS list. Also items from later games (389-397: QUICK POWDER, EVIOLITE, seven treats; item names stay 13 letters). Also the e-Reader Berries (items 377-388, berry numbers 47-58: use BERRY_SLOT / IsValidBerryType, never
`item - FIRST_BERRY_INDEX`), the MoeGatcha daily STANDARD lineup and the PICKUP reroll vote.
Also action sprites for every look (mp_hybrid.c: Brendan's / May's action frames with the look's head, data from
tools/moemon_hybrid) and HMs without restrictions plus held HMs (mp_heldhm.c: a held HM never leaves the player).
Also joining a battle in progress (mp_join.c: JOIN in front of a battling friend; mp_coop.c MP_Coop_JoinConvertMaster
rearranges the parties; battle_main.c CB2_MpJoinBattle; tests join_*).
Also two trainers at once and players grouped on one trainer (COOP_NOTES "Two trainers at once"; tests coop_two_*, coop_held_spot,
coop_wait_merge). Also Pokemon Stadium hues for nicknamed Moemon (mp_hue.c: the Stadium page's math, Kai's skin slots kept;
tools/moemon_hues, tests hue_*). Also the SPECTATE panel (friends' screens beside yours, each friend below / above / left / right /
in its own pop-out window at 1x-4x, remembered by name: mp_spectate.c streaming and settings, display.c layout, pop-outs and window
fitting; pixel-for-pixel checks with MOEMON_SPECTRACE + tools/test/check_spectrace.py; tests spec_*).
Also the characters from Moemon's other branch (111 looks with their own running, bike / ridden-Pokemon and diving
sprites, 127 ID and 107 back pictures: tools/moemon_chars, mp_chars_gfx.h, graphics ids 0x9000+, ini kai_*; LOOK >
GROUP NAMED NPC / GENERICS / CUSTOM and LOOK > BIKE RIDE / BIKE; PROF. BIRCH's bikes are 64x64; test look_chars).
Also catch EXP (a catch gives EXP scaled by the damage dealt: BattleScript_MpCatchExp; tests catch_exp, coop_catch_exp)
and BATTLE FLAG phrases ("Set phrases, or plant?", the save's easy chat battle phrases sent with MPR_ARRIVED /
BATTLE_GO, MPR_PHRASE_READY; the loser keeps their money; test flag_phrases).
Also OPTIONS in tabs (option_menu.c: GAME / DISPLAY / CONTROLS / CO-OP rows, L/R tabs; replaces START > SETTINGS and
CO-OP > SPECTATE) with BIKE MUSIC / SURF MUSIC (overworld.c MpRideMusic), BUTTON ICONS (gamepad.c) and MOENAV CALLS
(match_call.c SelectMatchCallTrainer); tests options_tabs, music_options, spec_menu.
Also POKéDEX entries from Ruby, Sapphire, Emerald and FireRed as one long text (pokedex.c TEXT tab: MpDexBuild / MpDexDraw,
reading mode UP/DOWN/B; data from tools/moemon_dex into src/data/pokemon/mp_dex_more.h and mp_dex_bar.h; test dex_text).
Also MOéMON GARDEN (A-Life, spec docs/POKEMON_GARDEN.md, built in milestones G1-G6: mp_life*.c, include/mp_life*.h; per-Pokémon
records in a "<save>.life" sidecar, Friendship only raised through moments and never lowered by the Garden; JOURNAL summary page;
the follower's own activities, Pokémon emote bubbles (round, pink, never like Trainer emotes), smart A + hold-A verb wheel,
story beats; tests life_*).

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
- StringCopyN / StringAppendN copy exactly N bytes and never add EOS: a full 7-letter name (PLAYER_NAME_LENGTH) runs
  on into the next bytes unless you write `*StringAppendN(buf, name, PLAYER_NAME_LENGTH) = EOS;` (test names7).
- Test clients all share one Trainer ID, so anything that moves a Pokémon between games needs `mp giveforeign`-style
  checks (a different OT ID caught the BAD EGG bug). `[heap] out of memory` in a log means the game heap ran out.

## Not yet done or verified
- The Windows exe has never been run on real Windows (it was cross-compiled); it runs under Wine
  (tools/test/wine_smoke.sh: solo, and a co-op battle with a Linux player through the Windows server)
- Co-op battles: only POKé BALLS (wild) and healing items (ItemUseInBattle_Medicine); no items in PvP
- Remote players aren't shown across warps (only the same map or connected maps)
