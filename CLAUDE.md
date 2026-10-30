# Moemon Emerald Kai Co-op: notes for Claude

Native PC build of Moemon Emerald Kai v1.0 with online 4-player co-op and PvP.
Based on Kurausukun's `pc_port` branch of pret/pokeemerald (commit 1165825), SDL2.

**Trigger word: "Continue Yugioh"** -> read `docs/YGO_HANDOFF.md` and carry on with its next steps (the Yu-Gi-Oh!
card duels).

Moemon Music Studio work (`tools/moemon_music/`): don't use agents (subagents / the Agent tool): do the work yourself
(the rule from the studio's own branch). The game plays its `.kaisong` exports: music/ (shipped; `<song>@<place>`),
src/mp/mp_kaisong.c, tools/kaisong/gen.py (docs/SONG_TABLE.md, the studio's place picker data; the release runs it),
scenes (`@tcg`, `@sim`...: a source file / src folder that names songs, found by gen.py; new music-playing code needs no
list edit, a scene using the battle code holds the music: MP_MUSIC_SCENE_HOLD), the BGM player at 64 tracks / 64
voices; loudness: tools/kaisong/loudness.py (INFO gain=, so each song plays as loud as the one it replaces; the release
workflows run it on the shipped copies, nothing by hand), the same music in two files plays on (MP_Kaisong_SameMusic); COOP_NOTES "Custom music"; tools/test/kaisong_test.sh, tools/kaisong/test_scenes.py.

Read `BUILDING_COOP.md` first for the architecture and build steps. The player
docs are in `dist_extras/README.txt` (shipped next to the exe).

## Key locations
- `src/mp/`: all co-op code (net, client, avatar, overworld, trainer mode, battle, menu)
- `include/mp.h`: shared co-op API. `src/mp/mp_net.h` holds the protocol (bump `MP_PROTOCOL_VERSION` on changes)
- `tools/mp_server/`: standalone relay server
- `src/platform/updater.c` + `tools/release/`: the auto-updater and releases. Publishing a version = bump
  `MOEMON_VERSION` (include/mp_version.h), add `tools/release/notes/v<ver>.md`, push to the default branch
  (.github/workflows/release.yml builds, tags and publishes; BUILDING_COOP.md). Sessions can't push tags.
  Every release's notes file starts with `Moemon Emerald Kai Co-op <ver> "<Catchy Title!>": <one-line note>`: that line and the rest
  are the game's PATCH NOTES (CO-OP setup screen only; gen_patchnotes.py at build time; v0.1-v0.4 = the days before 1.0).
  Notes are for players: say what changed in the game, never where it came from (no "the design you sent", mock-ups,
  tests, file names).
- Dev updates (planned, HANDOFF.md): unless the user asks for a feature with "As a dev update..." or something like it,
  it is a main update: don't test it as a dev update and don't push a dev update.
- `tools/moemon_import/`: scripts that imported the Kai IPS into the decomp (need the user's ROM; not in the repo)
- `tools/moemon_music/`: Moemon Music Studio, a separate SDL2 program: plays MIDIs (and PSF/PSF2/USF/GBS/2SF/GSF rips and GameCube streams,
  through vendored emulators in its third_party/ with capture hooks: README there) with the game's own instruments
  and engine (ported from src/music_player.c etc.; check engine changes against it), exports `.kaisong` songs
  (docs/KAISONG_FORMAT.md; the game plays them: src/mp/mp_kaisong.c) and decomp sources. `make test` there
- `graphics/moemon/`: Kai battle sprite and palette blobs (INCBIN'd)
- Hooks in stock files are marked `Moemon Co-op` / `Moemon Kai` (grep for them)
- `docs/user_images/`: every picture the user sent (mock-ups, bug shots), numbered, with an index. Save new ones there

## Building
- Linux: `make linux -j$(nproc)` (needs libsdl2-dev and libpng-dev)
- Windows: `make winwsl -j$(nproc)` with x86_64-w64-mingw32-gcc and the SDL2 mingw package in ./SDL2
  (or `tools/release/build_windows.sh SDL2_DIR`, which keeps the Windows objects apart). Build it before a release:
  files that include windows.h (display.c, updater.c...) can't name a static helper like a Win32 function (FillRect).

## Headless testing
`src/platform/sdl2.c` has a test harness: `SDL_VIDEODRIVER=dummy MOEMON_TEST=script.txt MOEMON_SHOTDIR=dir`
(commands: wait N, press KEYS N, mash KEYS COUNT INTERVAL, shot NAME, shotburst PREFIX FRAMES [STEP] = a shot every
frame while the script goes on, for glitches that last a few frames (rendershotburst: the window output, overlays too), fightshot NAME = a VS fight's own 480x320 picture, barrier NAME COUNT = wait until COUNT clients
sharing the result folder get there, so they act on the same frame, quit).
`MOEMON_DEBUG_QUICKSTART=1` (+ `MOEMON_DEBUG_NAME/GENDER/WARP/PARTY/MONEY`) skips the intro.
`MOEMON_COOP_CONFIG=x.ini` and `MOEMON_SAVE=x.sav` let several clients run on one machine.
`MOEMON_TEST_FPS=120` keeps several clients roughly in sync.
**Never test with moves that can miss** unless the test is about accuracy: a miss wastes the run, tokens and time.
Give the Pokémon a 100%-accurate move first (`mp teach SLOT MOVE` replaces its first move, e.g. 61 BUBBLEBEAM),
and pick foes and moves so the battle ends in a few turns.
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
checked pixel for pixel by the tri_cards test; type icons layered beside every battle's boxes; the party screen PARTY_LAYOUT_TRIPLE,
checked against the user's mock-up by tri_party), and grouped
stat moves (one animation and message for Growl / Tail Whip on several Pokémon), graphics past the GBA limits (64 sprite palettes,
128 sprites, 256 OAM entries, 128 KB of sprite tiles), co-op EXP (everyone levels
up and picks moves to forget at the same time), and ball entrance / recall effects (mp_ballfx.c: an effects layer
drawn over the frame, not bound by the GBA's sprites or palettes; MY PRESET 1-6, APPLY TO PARTY; 41 colors, the newer
pieces earned by achievements or MoeGatcha prizes (mp_ballunlock.c); art partly imported from the game's battle anims).
Also the GENESIS / EPOCH BALLS (Red's and Crystal's POKé BALLS: their catch math, throw, send-out, art and sounds;
mp_legacy_*.c, tools/moemon_legacy). Also the HGSS battle balls (16x32 frames, a spin in the air, special balls' own art:
pokeball.h BALL_FRAME_*, tools/moemon_balls/hgss_balls.py; COOP_NOTES "HGSS POKé BALLS"; test hgss_balls).
Also PC TAG mode (west button / F: tag Moemon, then MOVE / RELEASE them together; pokemon_storage_system.c end).
Also the CANDY JAR (mp_candy.c: candies from releases, PROF. BIRCH brings the jar, a physics jar menu, CONVERT to RARE
CANDY; ITEM_CANDY_JAR 398). Also the shared PC as 14 boxes in the PC box screen (mp_sharedbox.c: a mirror the stock
screen edits, diffed into server transactions; the player's own boxes are MP_PersonalStorage(), never assume
gPokemonStoragePtr is theirs while the PC is open) with MY PC, SEND TO, friends' cursors, and its ITEMS list. Also items from later games (389-397: QUICK POWDER, EVIOLITE, seven treats; item names stay 13 letters). Also the e-Reader Berries (items 377-388, berry numbers 47-58: use BERRY_SLOT / IsValidBerryType, never
`item - FIRST_BERRY_INDEX`), the MoeGatcha daily STANDARD lineup and the PICKUP reroll vote.
Also action sprites for every look (mp_hybrid.c: Brendan's / May's action frames with the look's head, data from
tools/moemon_hybrid) and HMs without restrictions plus held HMs (mp_heldhm.c: a held HM never leaves the player).
Also joining a battle in progress (mp_join.c: JOIN in front of a battling friend; mp_coop.c MP_Coop_JoinConvertMaster
rearranges the parties; battle_main.c CB2_MpJoinBattle; tests join_*), also when the trainer a friend is battling
spots you (MPR_COOP_BATTLING: JOIN BATTLE / BATTLE ALONE; tests coop_spot_*).
Link battles: only the master's engine runs, so anything the others' screens show from battle state must reach them
(battle weather: B_COMM_MP_WEATHER blocks; Pokemon emotes: MPR_BATTLE_MONEMOTE; tests coop_fieldweather*, coop_life_emote).
Whole-screen fades (28+ of the 32 palettes, 0<->16) are 30% quicker (palette.c sQuickFade).
Also two trainers at once and players grouped on one trainer (COOP_NOTES "Two trainers at once"; tests coop_two_*, coop_held_spot,
coop_wait_merge); a third player joining makes it a Triple Battle against both (test coop_two_join3). Also Pokemon Stadium hues for nicknamed Moemon (mp_hue.c: the Stadium page's math, Kai's skin slots kept;
tools/moemon_hues, tests hue_*). Also the SPECTATE panel (friends' screens beside yours, each friend below / above / left / right /
in its own pop-out window at 1x-4x, remembered by name: mp_spectate.c streaming and settings, display.c layout, pop-outs and window
fitting; pixel-for-pixel checks with MOEMON_SPECTRACE + tools/test/check_spectrace.py; tests spec_*).
Also the characters from Moemon's other branch (111 looks with their own running, bike / ridden-Pokemon and diving
sprites, 127 ID and 107 back pictures: tools/moemon_chars, mp_chars_gfx.h, graphics ids 0x9000+, ini kai_*; LOOK >
GROUP NAMED NPC / GENERICS / CUSTOM and LOOK > BIKE RIDE / BIKE; PROF. BIRCH's bikes are 64x64; test look_chars).
Also reusable TMs (party_menu.c keeps the TM, shop.c sells it once; mp_tmrefund.c: the LOTTERY CORNER clerk gives saves
from before 1.18.1 the TMs their flags prove they got and no longer have, from tools/moemon_tms; test tm_reuse).
Also catch EXP (a catch gives EXP scaled by the damage dealt: BattleScript_MpCatchExp; tests catch_exp, coop_catch_exp)
and BATTLE FLAG phrases ("Set phrases, or plant?", the save's easy chat battle phrases sent with MPR_ARRIVED /
BATTLE_GO, MPR_PHRASE_READY; the loser keeps their money; test flag_phrases).
Also OPTIONS in tabs (option_menu.c: GAME / DISPLAY / CONTROLS / CO-OP rows, L/R tabs; replaces START > SETTINGS and
CO-OP > SPECTATE) with BIKE MUSIC / SURF MUSIC (overworld.c MpRideMusic), BUTTON ICONS (gamepad.c) and MOENAV CALLS
(match_call.c SelectMatchCallTrainer); tests options_tabs, music_options, spec_menu.
Also POKéDEX entries from Ruby, Sapphire, Emerald and FireRed as one long text (pokedex.c TEXT tab: MpDexBuild / MpDexDraw,
reading mode UP/DOWN/B; data from tools/moemon_dex into src/data/pokemon/mp_dex_more.h and mp_dex_bar.h; test dex_text).
Also FF8-style level scaling (mp_levelscale.c: OPTIONS > GAME > LEVEL SCALING and its page of settings MP_SCALE_*, the host's
online, set on HOST A GAME > GAME RULES; the team's top-3 average, co-op seats average via MpState.top3; trainers, wild (halfway
by default), scripted wild; badge caps, boss margins, trainer evolutions, a raised wild one's evolved form as a rare encounter;
tests levelscale_*, coop_levelscale, host_rules).
Also the BAG's eight pockets (ITEMS, MEDICINE, POKé BALLS, TMs & HMs, BERRIES, BATTLE ITEMS, KEY ITEMS, FAVORITES: views of
the save's five pockets, MP_BAGVIEW_*; gBagPosition is per view, use GetItemPocket(item) for anything about the item; the
HGSS-style screen from docs/user_images/31.png drawn in full color by mp_bagui.c; COOP_NOTES "The BAG's eight pockets";
tests bag_pockets, bag_more). Also item descriptions from the Gen 4 games (tools/moemon_items/gen_desc.py from PokeAPI's
tables; no line breaks: mp_itemdesc.c MP_ItemDesc_Print wraps them to each window, narrow font or self-turning pages when
they don't fit; test item_desc) and REJOIN that remembers where the room is (a friend's own server too) plus the last
invite code (COOP_NOTES "REJOIN and remembered invite codes"; tools/test/rejoin_smoke.sh).
Also the Garden's BERRY farm (mp_farm.c: the game's own BERRY trees, ids 100-119, framed soil stamped as plots open; online the room host's
farm through MP_Farm_Tree in GetBerryTreeInfo, plant / pick asked of the host; GA_FARM Pokémon work by nature; COOP_NOTES "BERRY farm"; tests farm_*).
Also PMD Collab followers and Garden Pokémon (mp_pmd.c, data tools/moemon_pmd -> graphics/moemon/pmd/pmd.bin via .incbin; big sprites:
struct Sprite mpBig, mp_bigsprite.c + gba_easy_draw.c DrawBigSprite draw any size in the sprite order; 8 directions, poses 7-19 =
PMD actions, MP_Pmd_Animate; credits dist_extras/PMD_CREDITS.txt; tests pmd_*). Also battle weather on screen the whole battle (mp_battleweather.c: particles over the frame, the renderer hook gMpScenePixel
for the sandstorm and tints; no per-turn weather lines; tests battle_weather*).
Also followers on the ground (mp_followfx.c: the game's grass, sand, puddle, water, hot springs and reflection effects driven
from a follower's tiles; tests follow_ground*).
Also MOéMON GARDEN (A-Life, spec docs/POKEMON_GARDEN.md, built in milestones G1-G6: mp_life*.c, include/mp_life*.h; per-Pokémon
records in a "<save>.life" sidecar, Friendship only raised through moments and never lowered by the Garden; JOURNAL summary page;
the follower's own activities, Pokémon emote bubbles (round, pink, never like Trainer emotes), smart A + hold-A verb wheel,
story beats; battle memories and rare pink lines; friends' Pokémon and records in trades; the Garden map (mp_garden*.c:
every POKéMON CENTER's 2F, its escalator redirected in SetupWarp, START > GARDEN quick travel; tilesets via
tools/moemon_garden/gen_garden.py: original pieces joined by the original maps' rules, a place per type (zones) and
per badge (growth table), pathfinding to them; the escalator kept at its fldeff_escalator.c ids; test garden_places); G6 pursuits, rare
moments, eggs and family, evolution; every activity visible: swimming in the pond, climbing trees, digging, cut off at the
water / ground line by sprite.h mpClipOn (any sprite can use it), ground effects as followers'; test garden_doing; tests life_*, garden_*).

Also 1.30.0's WHERE IS EVERYONE arrows (mp_compass.c: a world grid of Hoenn from the map connections, doors for indoor
maps, drawn over the frame; test compass; the FRIEND FINDER: hidden until the right stick click / G, quiet arrows and a
corner radar, OPTIONS > CO-OP > FRIEND FINDER: COOP_NOTES "The FRIEND FINDER"), Town Map heads of each look (MP_Avatar_HeadIcon), RARE CANDY counts, smart
item targeting (mp_smartcursor.c), followers in their ball on bikes: COOP_NOTES "1.30.0".

Also the GAME CENTER (RUSTBORO's north-east tower, four floors drawn from the user's full-color pictures by mp_mappic.c:
pictures pinned to maps under the sprites, an "over" layer for furniture tops, past the GBA's tiles; maps and masks from
tools/moemon_gamecenter; the 2F CARD SHOP UI_GCSHOP with money singles and the 1999 theme decks; COOP_NOTES "The GAME
CENTER"; tests gc_look, gc_visit). Card packs: holos one pack in three, a new one after 20 dry packs (tcg_collection.c,
tools/tcg/pack_math.py).
Also CARD DUELS (src/tcg/, include/tcg/: the original 228-card TCG; rules engine with presentation events, the DS-style
duel board tcg_board.c drawn into a 240x160 canvas display.c shows as the game picture (Tcg_Canvas; spectate gets it via
Tcg_Frame), CARD CASE item 399 (registered: its button in front of someone challenges them, tcg_field.c), the CARD CLUB in
three POKéMON CENTERs, 24 special trainer decks (gen_special_decks.py), CARD POINTS counter, `<save>.cards` sidecar,
friends' duels / trades by request (tcg_online.c) on relay extension 255 with TCG_LINK_VERSION, card duel TOURNAMENTs
(mp_tourney.c TK_CARDS); NPCs' Prize cards (2/3/4/6) and records (collection format 6, test tcg_record); the GAME CENTER
CARD SHOP sells one of each (gcDecks / gcSingles) and runs the CARD CLUB's counter too;
all 26 Pokemon Powers (tcg_powers.c, tests tools/tcg/test_powers.c + test_ui.sh "powers"); every attack of the 228 cards (the last 20 and
their menus: COOP_NOTES "Every POKéMON card attack", test_attacks.c); CARD SCALING (tcg_scale.c:
Elo deck ratings from AI duels in the generated tcg_ratings.c, rerun tools/tcg/rate_decks.sh after any rules, AI or deck
change; per-save skill; tcg_field.c Tcg_FitOpponent); CARD ART / CARD BACKS
from the built-in graphics/tcg/printed.bin (tools/tcg/fetch_art.py + build_printed_art.py: scans, the back and each
illustration's measured rectangle; test_art.sh); everything in
docs/TCG_IMPLEMENTATION.md, decisions in docs/TCG_DECISIONS.md; tests tools/tcg/test*.sh).

Also YU-GI-OH! duels (src/ygo/, include/ygo/: the CARD CASE's second side, L/R switches; SDD's card list and
Forbidden/Limited list plus the English boosters to the end of 2003 (tools/ygo, catalog in ygo_catalog.c / card_ids.h;
the ROM is never committed): the cards that can be collected and seen. The rules are EDOPro's: edopro/ocgcore
(ygopro-core + Lua 5.4, C++ built by the Makefile's EDO_* rules, AGPL: dist_extras/EDOPRO_CREDITS.txt) with every card
and script in graphics/ygo/edo.pak (tools/ygo/edo/build_pack.py from BabelCdb, CardScripts, LFLists, strings.conf;
ygo_edo_data.c). src/ygo/ygo_edo.c bridges it to the game's API (include/ygo/duel.h: YgoQuery / YgoAnswer, events,
YgoState as objects that keep their number): never copy a YgoDuel by value (YgoDuel_Copy / Rewind rebuild from the
answers); cards outside the catalog (tokens) get ids past YGO_CARD_COUNT. AI ygo_ai.c (reads only what it may see), the
table ygo_board.c (the opponent's cards turned to face them), screens ygo_ui.c (FROM CODE: YDKE codes, DECK CODE,
.ydk import/export on Windows only via src/platform/file_dialog.c; the LIST row: every EDOPro Forbidden & Limited list,
YgoCollection.banlist), who plays which game and the special duelists ygo_field.c + tcg_field.c's which-game menu
(most duelists bring a deck from the GBA games' opponents: ygo_npc_decks.c from tools/ygo/scrape_npc_decks.py +
rate_npc_decks.sh), the CARD CLUB duelist in the POKéMON MARTS (data/scripts/ygo_club.inc), "<save>.ygo"; quiet chain
windows are passed by the board unless L is held; printed art and the real back in graphics/ygo/printed.bin;
duels / trades with friends by answers over relay types 8-13 (ygo_net.c, struct YgoNet), deck menus with RENAME / ACE
CARD / DECK RECIPES on both card games (COOP_NOTES "Deck menus"), a Starter Deck to keep or borrow for anyone asked
without a deck (ResolveNoDeck), the GAME CENTER 2F clerk; decisions docs/YGO_DECISIONS.md, COOP_NOTES "Yu-Gi-Oh!" and
"Yu-Gi-Oh! on EDOPro"; tests tools/ygo/test.sh (test_edo, test_edo_cards: scenarios with unshuffled Decks), tools/ygo/test_ui.sh, scenarios ygo_*).

The three card games share INSPECT (R2 / D: PLATFORM_KEY_INSPECT), CONCEDE on your own deck, one cursor (G_Focus)
and challenge screens with the opponent's battle picture (tcg_field.c CaptureChallengePic): COOP_NOTES "The three card
games together". They also share AI levels 0-3 that slip like people (weighed by how much worse a choice looks; dice
only from what both can see), hidden power ratings from AI duels blended with the cards' tournament history
(tools/tt/rate_cards.sh, tools/ygo/rate_cards.sh, tools/tcg/rate_cards.sh: audits in docs/audit_*.txt; rerun after
rules / AI / card changes), CARD SCALING fits with a slight player edge, and the challenge screen's live difficulty
for the deck under the cursor (TcgUi_Match): COOP_NOTES "Card AI levels, power ratings and how a match looks".

Also TRIPLE TRIAD (src/tt/, include/tt/: the CARD CASE's third side, L/R go round the three games; every Final Fantasy
XIV card from xivapi (tools/tt: fetch_cards.py, gen_catalog.py, build_art.py -> graphics/tt/cards.bin, gen_pools.py ->
tt_pools.c; ids are XIV's); FF8's capture rules from the ff8-decomp (SAME, PLUS, SAME WALL, ELEMENTAL with XIV's types,
COMBO) and XIV's rules, tt_duel.c deterministic, AI tt_ai.c; HOENN's eight regions with their own rules, the rules
carried from the last region played in spread or abolish (Tt_MixRules), the CARD QUEEN (favors, paid journeys);
only the player ever takes cards (trade rules ONE / DIFF / DIRECT / ALL); XIV decks of five; sellers in seven POKeMON
CENTERS + the GAME CENTER 2F (shelves under 15% of the cards, BRONZE / SILVER / GOLD packs, MYTHRIL / IMPERIAL never
sold); who plays which game tt_field.c Tt_FieldPick before tcg_field.c's own; "<save>.tt"; decisions
games and trades with friends (tt_net.c over relay type 14, request kinds 4-5; scenarios tt_online, tt_trade);
docs/TT_DECISIONS.md, COOP_NOTES "Triple Triad"; tests tools/tt/test.sh, tools/tt/test_ui.sh).

Also the VS cabinet (MAUVILLE GAME CORNER's two slot machines at (7,9)/(8,9)): MUGEN fights in fight/ (mugen-unleashed engine,
include/mf_engine.h), src/platform/fight_platform.c, src/mp/mp_fight.c; online with rollback (dirty pages of a fixed-address heap
region + the engine's globals section), the pack check by file hashes, desync recovery by page diff, OPTIONS > CONTROLS > FIGHT
BUTTONS; content in mugen/ (its sounds GBA sound by tools/fight/gba_audio.py; a stage's song kept under the voices by
mf_mixer.c); sprites area-resampled (fight/compat/mf_softscale.c) and the 480x320 picture shown through display.c's
own scaling and filters; docs/FIGHT_DECISIONS.md and COOP_NOTES "The VS cabinet"; tests fight_*, tools/fight/test_softscale.c. Character and
balance audits: tools/fight/check_chars.py, audit_chars.sh, balance_audit.py (docs/FIGHT_AUDIT.md, docs/FIGHT_BALANCE.md); asset
ingestion tools/fight/ingest.py (run by the build when mugen/ changed; mugen/processed_assets.json, ingestion_log.txt).

Also the DANCE machine on the GAME CENTER's 3F (a DDR EXTREME rhythm game: src/rhythm/, include/rhythm.h, src/mp/mp_rhythm.c;
StepMania's .sm / .ssc read by rh_chart.c, judged by Beware's EXTREME rules in rh_play.c; drawn into the GBA picture by
sdl2.c ScreenStep, mixed by Platform_QueueAudio; art and announcer graphics/rhythm/rhythm.bin from tools/rhythm/gen_assets.py;
the 240 songs in stepmania/Songs, shipped as their own release zip the machine downloads (updater.c Updater_Songs*,
tools/release/package_songs.sh); VERSUS online by RN_* messages, records server-wide in mp_scores.c, DANCE achievements;
docs/RHYTHM_DECISIONS.md, COOP_NOTES "The DANCE machine"; tests dance_*, tools/test/dance_songs_test.sh, tools/rhythm/test.sh).

Also BATTLE ANIMS > MODERN (mp_modernanim.c: glows, trails and hit bursts drawn over the Classic animation, which always runs:
same frame count, test anim_modern checks all 716) and the BATTLE SIMULATOR in the BATTLE TENTS (mp_sim.c: digital copies of
players from the last server, records via mp_scores.c MP_ST_SIM_*, "<save>.sim"; tests sim_*). Every trainer battle but the
Elite Four's and the Champion's can be joined (MP_Coop_JoinTrainerOk; test join_gym); COOP_NOTES "Joining a battle in progress".

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
