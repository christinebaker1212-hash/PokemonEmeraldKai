# Moemon Emerald Kai Co-op: notes for Claude

Native PC build of Moemon Emerald Kai v1.0 with online 4-player co-op and PvP.
Based on Kurausukun's `pc_port` branch of pret/pokeemerald (commit 1165825), SDL2.

SINNOH (Pokémon Platinum as a fourth region in a new 3D engine, the POKéTCH): released in 1.63.0 (a NEW GAME can start there
from 1.63.1: COOP_NOTES "A new game in SINNOH"; its starter from Platinum's briefcase screen, pre-drawn from Platinum's
models: tools/sinnoh/build_starter.py, COOP_NOTES "SINNOH's briefcase"), `docs/SINNOH_HANDOFF.md`;
everything per feature in COOP_NOTES (its SINNOH sections: story people tools/sinnoh/story_people.py, the gyms that move
src/sinnoh/s3d_gyms.c, the DISTORTION WORLD scene, the BATTLE FRONTIER's five: TOWER, FACTORY (the game's own halls,
shared maps), HALL s3d_hall.c, CASTLE / ARCADE s3d_frontier.c). **SINNOH is 1:1 with Platinum** (the user, 2026-11-02):
what Kai already has, SINNOH uses (never a second copy); anything else not 1:1 is built (the DISTORTION WORLD first);
only online-only features (GTS, Wi-Fi) and ones Kai supersedes or the DS itself needs (PAL PARK's catching show; its building is to reopen as a hall for all four
card games, original words allowed there: docs/MTG_DECISIONS.md) are closed. The list:
docs/SINNOH_HANDOFF.md "Platinum's own words and scripts". The 3D
engine (src/sinnoh/s3d.c, include/s3d.h) draws Platinum's own map models from sinnoh/platinum3d.pak, built from
pret/pokeplatinum at a pinned commit by tools/sinnoh/make_pack.sh (run it before playing SINNOH from a build; the release
runs it and ships the pack; never committed: sinnoh/ is ignored; no ROM needed); TWINLEAF TOWN and ROUTE 201 (group 40)
walk; tools/sinnoh/test_poc.sh POKEPLATINUM_DIR [ROM]; COOP_NOTES "SINNOH's 3D engine" (doors, moving lakes, buildings drawn by
Platinum's material / shape lists: COOP_NOTES "SINNOH's doors, moving lakes and the feet row"); its people's sprites and battle pictures
in sinnoh/platinum_gfx.pak (tools/sinnoh/build_gfx.py, src/sinnoh/s3d_gfx.c; test sinnoh_pics).
**SINNOH's words and scripts are Platinum's own** (the user, 2026-11-02): never write original dialogue or scripts
for SINNOH. They're converted from the decomp when the game is built, never committed: sinnoh/platinum_text.pak
(tools/sinnoh/build_text.py) and sinnoh/platinum_scripts.inc (tools/sinnoh/gen_scripts.py; maps hold only
`goto SinnohP_<file>_<n>` stubs; Platinum's map scripts, people and triggers per map behind FLAG_SINNOH_PLATINUM_STORY,
s3d_scrcmd.c Sinnoh_PlatinumMapScripts; always on in SINNOH: no old saves to keep, no migration, the user 2026-11-02);
COOP_NOTES "Platinum's own words and scripts". Progress: docs/SINNOH_CHECKLIST.md. Platinum's TV: src/sinnoh/s3d_tv.c
(COOP_NOTES "SINNOH's TV"); SUPER CONTESTS: src/sinnoh/s3d_contest.c (rules) + s3d_contest_ui.c (screens), tables by
tools/sinnoh/contest_data.py, docs/SUPER_CONTEST.md (COOP_NOTES "SUPER CONTESTS"; POKé BALL effects in the Seals'
place: s3d_seals.c; test sinnoh_contest). The Underground, for its digging and SPIRITOMB only (wireless sides are CO-OP's):
src/sinnoh/s3d_underground.c + s3d_mining.c (art tools/sinnoh/build_mining.py; COOP_NOTES "The Underground"; tests
sinnoh_underground*, sinnoh_spiritomb*). The written words in
tools/sinnoh/words*.py / story_people.py only stand in until a script's commands are converted.

**Done (2026-11-01): `docs/NEXT_TASKS.md`**, the user's list through 1.63.7 (40 PC boxes: COOP_NOTES "40 PC boxes"; the save holds
14, "<save>.boxes" the rest, src/mp/mp_boxes.c; tools/test/pc_boxes.sh).

**The Moemon Companion App (Android)**: the user's design in `docs/COMPANION_APP.md`, started 2026-11-03 ("let's build
the app"). Phase 1 (1.70.0): the game's PHONE LINK (src/mp/mp_phone.c, phone_proto.c, mp_export.c) and the app in
android/ (core/ is Android-free), all in `docs/PHONE_LINK.md`; tests tools/phone/gate.py, android/coretest/run.sh,
`./gradlew testDebugUnitTest` (pictures of every page). Assets from `python3 android/build_assets.py`. The APK is signed
by repository secrets in the release workflow; its key must never change. Phase 2 (1.72.0, the device: POKéNAV /
POKéTCH / POKéGEAR on the DEVICE page, the live map, MATCH CALL, CONDITION, RIBBONS, battle mode): src/mp/mp_phone_dev.c
(+ mp_phone_int.h), PHONE_LINK.md "The devices, the live map, battle mode". Phase 3 (1.73.0, the walk, training, the
Garden in your pocket): src/mp/mp_phone_walk.c, android core/Walk.java, PHONE_LINK.md "Phase 3"; tests
tools/phone/walk_test.py, coretest WalkCoreTest. Phase 4 (1.74.0, friends: presence, JOIN, linking away from home
through the co-op server's MAIL and PIPE): src/mp/mp_phone_friends.c ("<save>.pals"), mp_server.c MAIL / PIPE,
android core/Server.java, Friends.java, FriendsPage; PHONE_LINK.md "Phase 4"; test coretest FriendsTest.
Trades by mail (1.75.0): the server's ESCROW, pair keys (X25519), src/mp/mp_phone_trade.c ("<save>.trades"), trade.c
MP_MailTrade_Scene, android core/Trades.java; PHONE_LINK.md "Trades by mail"; tests coretest TradeTest (two games at lag;
ONLY=trade android/coretest/run.sh), tools/phone/escrow_test.py.
TRIPLE TRIAD on the phone (1.76.0): the game's src/tt through android cpp/tt_jni.c (Tt.java), art from build_assets.py,
src/mp/mp_phone_cards.c (snapshot "tt", orders ttdeck / ttwin / ttopen, "<save>.phonecards"), CardsPage / TtDuelPage,
friend duels core/TtFriends.java; PHONE_LINK.md "TRIPLE TRIAD on the phone"; tests tools/phone/cards_test.py, coretest
TtCoreTest, TtFriendsTest (ONLY=tt). The POKéMON card game on the phone (1.78.0): src/tcg through cpp/tcg_jni.c
(Tcg.java); what a move asks for is src/tcg/tcg_flow.c (include/tcg/flow.h), shared with the game's tcg_board.c (never
a second copy); snapshot "tcg", orders tcgdeck / tcgduel / tcgopen; TcgDuelPage etc.; friends by turns core/CardDuels.java
(both games; records in pages of 12: the server keeps 16 entries a key); PHONE_LINK.md "The POKéMON card game on the
phone"; tests coretest TcgCoreTest, TcgFriendsTest (ONLY=tcg), cards_test.py. YU-GI-OH! on the phone (1.79.0): EDOPro's
ocgcore + src/ygo through cpp/ygo_jni.c (Ygo.java; the engine's pack an asset: YGO_EDO_PAK_EXTERNAL), YgoDuelPage etc.,
snapshot "ygo", orders ygodeck / ygoduel / ygoopen; PHONE_LINK.md "YU-GI-OH! on the phone"; tests coretest YgoCoreTest,
YgoFriendsTest (ONLY=ygo). The app's sounds (1.81.0): the game's own, recorded from its sound engine at APK build time
(src/mp/mp_export_sound.c -> assets game/sounds.bin; Sound.java, MENU > SOUND; the card tables play their boards'
sounds and scenes' songs); PHONE_LINK.md "The app's sounds"; tests tools/phone/sounds_test.py, SoundTest. The POKéDEX's details, the type chart (TypeChartPage) and MENU > LOOK (themes, window frames, animations, start page: Look.java) (1.84.0): PHONE_LINK.md "The POKéDEX's details, the type chart, LOOK". The EGG INCUBATOR (1.172.0: twelve EGGS hatched by the phone's steps, home to a chosen box with room: mp_phone_walk.c incubate / hatchhome, "<save>.incubator", core/Incubator.java, BoxChoosePage; PHONE_LINK.md "The EGG INCUBATOR"; test tools/phone/incubator_test.py). BREEDING (1.185.0: BreedPage, core/Breeding.java: bridge kinds from a POKéMON to a target across egg groups, egg moves; PHONE_LINK.md "BREEDING"; test ONLY=breed android/coretest/run.sh). The POKé BALL page (1.85.0: BallPage.java, src/mp/mp_phone_ball.c, the effects recorded from the game's own editor by src/mp/mp_fxclip.c into fxclips.bin): PHONE_LINK.md "The POKé BALL page"; test tools/phone/ball_test.py. The ME tab (1.85.0: MePage, AchievementsPage, GatchaPage, RENAME; src/mp/mp_phone_kai.c, mp_gatcha.c MP_Gatcha_PhonePull): PHONE_LINK.md "The ME page"; test tools/phone/kai_test.py. MOéMON or POKéMON in the app (1.86.0: the game's OPTIONS > CREATURES or MENU > LOOK > CREATURES; both art sets exported, GameArt.words): PHONE_LINK.md "MOéMON or POKéMON". The POKéMON and YU-GI-OH! tables turn the
opponent's cards to face them; TRIPLE TRIAD stays as it is (never turn its cards: the user, 2026-11-03). The user asked
(2026-11-03) for all four phases in order, then a UI/UX pass: "polished Nintendo quality, mid 2000s to 2007 era
graphics, 2026 usability".

**Moemon Maker** (the user's own MZ-like editor, built into the game; **start at `docs/MAKER_HANDOFF.md`** (status, the
full plan M1-M5, next steps: M2 waits for the user's go and four open questions); design and every decision in `docs/MOEMON_MAKER.md`;
M1 built 2026-11-03: COOP_NOTES "Moemon Maker, M1"): C++ in maker/ (Dear ImGui) reaching the game only through
include/maker_api.h (src/maker/*.c); hidden until `AllowSmallTweaks = 1` and "maker" typed (never in PATCH NOTES or the
README). The user's edits are MoemonMaker/base/MAP_X.txt (each line keeps the value it replaced; format at the top of
src/maker/maker_overrides.c), applied at run time through Overworld_GetMapHeaderByGroupAndId / Maker_MapBlocks; music in
MoemonMaker/music/; published from the editor straight to the default branch (a `Moemon-Maker:` trailer), then the
maker-pack workflow ships MoemonMakerPack/. Tests tools/test/maker_*.sh.
**The user's own changes come first** (the user, 2026-11-03): anything in `MoemonMaker/` and any commit without a
`Claude-Session:` trailer (the Maker's Publish commits carry `Moemon-Maker:`) is the user's work. Before a change,
regeneration or conversion script would alter or overwrite it, check `git log` / `git blame` and ask the user; never
"fix" a user edit on assumption. `MoemonMaker/` takes precedence over the conversion scripts (tools/quetzal_port,
tools/sinnoh...): its edits sit on top of their output at run time and are never written into their files, so a
regenerated map keeps them (where the same thing changed, the editor asks the user which wins).
The dev update channel ended with the Maker's first update (the user, 2026-11-03): every build follows main,
AllowSmallTweaks only unlocks the small tweaks, and no dev updates are pushed.

**POKéMON STADIUM** on BATTLE ISLAND (the user's spec and decisions: `docs/STADIUM.md`, the cups' rules
docs/stadium/RULES.md): data from the user's ROM (tools/stadium/, never commit the ROM), src/mp/mp_stadium*.c (cups and
battles mp_stadium_cup.c, screens mp_stadium_ui.c, saved teams per server mp_stadium_teams.c, rewards / ribbons /
achievements mp_stadium_rewards.c), its music as music/stadium_*.kaisong (new tracks by name); tests stadium_*. Its lobby's
fans (1.168.0, data/maps/BattleIsland_Stadium). Its announcer (1.165.0): Stadium's own 823 voice clips from the user's ROM (stadium/announcer.s1, never committed:
tools/stadium/mort/make_voice_pack.py; the release's STADIUM_ROM_URL secret), decoded by SubDrag's MORT decoder (mort/,
public domain), lines chosen by the battle's messages and only when they fit the time the battle gives them, never cut
off: src/mp/mp_stadium_announcer.c, docs/STADIUM.md "The announcer"; OPTIONS > BATTLE > ANNOUNCER (OFF / ENGLISH /
JAPANESE: POKEMON STADIUM 2 (Japan)'s 831 clips laid under the English numbers, stadium/announcer_jp.s1, secret
STADIUM_JP_ROM_URL; STADIUM.md "The Japanese announcer"); tests stadium_announcer, stadium_announcer_jp.

**ORRE** (Pokémon Colosseum's region, started 2026-11-04): **start at `docs/ORRE_HANDOFF.md`** (status, the disc's
link and the decomp's, setting up a session, next steps, open questions); the user's decisions in
`docs/ORRE_DECISIONS.md` (3D maps under a Platinum-style camera, 2D sprites rendered from the models, Kai's battles, Colosseum's own words
and scripts, Shadow Pokémon in Orre only). Packs ripped from the disc by tools/orre/make_pack.sh into orre/ (never
committed; the release builds them from the `ORRE_DISC_URL` secret and ships them in the game's own zips, replaced with every update). **ORRE has its own
3D engine, src/orre/o3d.c, never SINNOH's** (the user, 2026-11-04): never put ORRE code in src/sinnoh/ or s3d.h, or
SINNOH's in src/orre/; where both hook into the game each has its own line. All 121 field rooms are maps
(tools/orre/maps.py ORDER), their doors / stairs / elevators and entries from the game's own tables
(tools/orre/colo_rooms.py; ORRE_HANDOFF "Rooms, doors and entries"; scenarios orre_all_maps, orre_all_doors,
orre_all_places from tools/orre/gen_tests.py); the ways out open Colosseum's own world map (src/orre/worldmap.c).
**ORRE is post-game only, reached by boat** (the user, 2026-11-07): never a NEW GAME region; the ORRE TICKET (item 810)
after any ELITE FOUR (old saves too: mp_battleisland.c's courier), on HOENN's ferry lists and every port, landing at the
OUTSKIRT STAND by a sailor who sails back (src/orre/orre_ferry.c, data/scripts/orre.inc; ORRE_DECISIONS "A post-game place").
Shadow Pokémon (designed before the story, the user 2026-11-07): `docs/SHADOW_POKEMON.md`.
ORRE's words: orre/orre_text.pak (tools/orre/build_text.py, from the disc by make_pack.sh, never committed), a line by
Colosseum's own message id through PLACEHOLDER_ID_ORRE_TEXT, the words it fills in ORRE's string slots
(src/orre/orre_text.c; ORRE_HANDOFF "Colosseum's text"; scenario orre_text). ORRE's battles: Colosseum's battle /
trainer / team tables read from the pack at run time, fought as Kai trainer battles through TRAINER_ORRE_A / _B
(src/orre/orre_battle.c; ORRE_HANDOFF "Trainers and battles"; scenario orre_battle). ORRE's scripts: Colosseum's
own GS VM bytecode run by src/orre/orre_vm.c (rooms' main loops, scenes holding the field through
Orre_EventScript_VmHold, natives mapped to Kai; on for players since 2026-11-08, MOEMON_ORRE_SCRIPTS=0 turns them off;
tools/orre/gsvm.py lists a bank; ORRE_HANDOFF "Colosseum's scripts"; scenario orre_scene_willie). ORRE's rooms' people:
Colosseum's own (src/data/orre_room_people.h from tools/orre/gen_people.py) as Kai objects shown, moved, turned and
talked to through the VM (src/orre/orre_npcs.c; A on one runs its talk script; ORRE_HANDOFF "People on the maps").
ORRE's POKéMON CENTERs, PCs and marts: Colosseum's scripts opening Kai's screens (the heal, Kai's PC, its POKéMART with
Colosseum's lists src/data/orre_shops.h from tools/orre/gen_shops.py), check spots src/data/orre_room_checks.h
(tools/orre/gen_checks.py); ORRE_HANDOFF "Shops, POKéMON CENTERs, the PC"; scenario orre_center. Colosseum's camera
shots (.cam) play in ORRE's engine (src/orre/o3d_cam.c, tools/orre/hsd_cam.py; ORRE_HANDOFF "Colosseum's cameras";
scenario orre_camera); the opening (step 7: its shots over the hideout as scene rooms, then the SNAG MACHINE: src/orre/orre_opening.c, ORRE_HANDOFF "The opening"; scenario orre_opening).
ORRE's doors (2026-11-11): Colosseum's door table as gates that start shut and open when walked up to, by their scripts or once unlocked (orre_vm.c CtrlDoor / TouchDoors, tools/orre/gen_gates.py sOrreDoors; ORRE_HANDOFF "Doors"; scenario orre_doors).
ORRE's sound (2026-11-10): Colosseum's own songs and effects from the disc's MusyX data, played by the MusyX runtime in musyx/ (AxioDL's, CC0) through a GBA filter; bridge musyx/orre_musyx.c, include/orre_sound.h, src/orre/orre_audio.c; ORRE_HANDOFF "Sound".
ORRE's TVs (1.189.9): a still of each programme drawn as the pack is built (tools/orre/tv.py, table src/data/orre_tv.h from gen_tv.py), on the screen by std171 / std211 (o3d.c O3D_TvSet). ORRE's loudness: tools/kaisong/normalize.py measures Colosseum's songs too (music/orre_gains.txt, the whole mix takes the playing song's gain: musyx/orre_musyx.c); RUI's name is the player's (orre_text.c Orre_PartnerName). **Don't run ORRE's tests: the user tests ORRE (2026-11-11).**
ORRE's people (the user, 2026-11-07): each Colosseum character model is a LOOK > CREATE recipe (field sprite) and the
closest Pokemon Showdown trainer sprite (battle picture, 80x80 as drawn): tools/orre/people.py, src/orre/orre_people.c,
ORRE_HANDOFF "Sprites"; scenario orre_people.

**Magic: The Gathering** (a fourth card game): the user's decisions in `docs/MTG_DECISIONS.md`; **start at
`docs/MTG_HANDOFF.md`**. Forge translated into C (src/mtg/, include/mtg/mtg.h; its AI too, later), reading Forge's own
card scripts for the Premodern pool (graphics/mtg/forge.pak from tools/mtg/build_pack.py at the pinned Forge commit;
pool tools/mtg/pool.json from Scryfall). A card plays only once every piece of it is translated; how many do:
docs/MTG_SCOPE.md (tools/mtg/scope.py, rerun after translating). Tests tools/mtg/test.sh. In the game since 1.95.0 at
MOSSDEEP's CARD SHOP (the GAME CORNER, data/scripts/mtg_shop.inc), the CARD CASE's MAGIC side hidden until a deck
(src/mtg/mtg_ui.c, mtg_board.c, "<save>.mtg"): COOP_NOTES "MAGIC"; scenarios mtg_shop, mtg_play. Since 1.96.0 its
screens copy the POKeMON / YU-GI-OH! sides' in every detail and its table is the POKeMON table's style (the user: keep
them so), faces src/mtg/mtg_gfx.c with Scryfall's art (graphics/mtg/printed.bin: tools/mtg/fetch_art.py,
build_printed.py). Boosters by set (1.100.0: src/mtg/mtg_sets.c, data src/mtg/mtg_sets_data.c from tools/mtg/fetch_sets.py),
triggers src/mtg/mtg_trigger.c, MAGIC achievements (mp_achieve.c C_MAGIC). The CARD CASE's L / R round: POKeMON, YU-GI-OH!, MAGIC, TRIPLE TRIAD (always last).
The CARD SHOP counter as POKeMON's (1.167.0: TRADE IN spares, CARD SHOP, PRIZES; mtg_ui.c ShopScreen) and curated
structure decks from real precons that appear once their cards play (tools/mtg/fetch_precons.py CURATED ->
src/mtg/mtg_precons_data.c): COOP_NOTES "MAGIC's CARD SHOP counter and structure decks". **MOSSDEEP's CARD CLUB maps
are the user's (2026-11-07): change only data/scripts/mtg_shop.inc and the C behind it** (and what the user asks for:
1.168.0's counter door and play room mat). The play room's players change every half hour, the club's talkers too
(src/mtg/mtg_club.c: COOP_NOTES "MOSSDEEP's CARD CLUB: the counter's door, the play room's players"; scenario mtg_club).

**Trigger word: "Continue Yugioh"** -> read `docs/YGO_HANDOFF.md` and carry on with its next steps (the Yu-Gi-Oh!
card duels).

Moemon Music Studio work (`tools/moemon_music/`): don't use agents (subagents / the Agent tool): do the work yourself
(the rule from the studio's own branch). Its own releases: raise STUDIO_VERSION (src/project.h) + release_notes/<ver>.md,
push to the default branch: .github/workflows/mms-release.yml publishes `mms-v<ver>`, never as the latest release (the
game's updater reads releases/latest; the workflow checks a `v<ver>` game release keeps the mark). The game plays its `.kaisong` exports: music/ (shipped; `<song>@<place>`),
src/mp/mp_kaisong.c, tools/kaisong/gen.py (docs/SONG_TABLE.md, the studio's place picker data; the release runs it),
scenes (`@tcg`, `@sim`...: a source file / src folder that names songs, found by gen.py; new music-playing code needs no
list edit, a scene using the battle code holds the music: MP_MUSIC_SCENE_HOLD), the BGM player at 64 tracks / 64
voices; loudness (the user, 2026-11-10: fast, at build time, dynamics kept, every added song equally loud):
tools/kaisong/normalize.py, run by the release after the Linux build and by hand after adding a song (commit
tools/kaisong/loudness_cache.json and music/kaisong_gains.txt): the game renders each new or changed song as fast as it
runs (MOEMON_MEASURE_MUSIC, mp_export_sound.c), one gain a file to the median of the songs replaced, peaks under -1 dBFS;
the game reads the gains (mp_kaisong.c ReadGainTable, over INFO gain=); the old real-time loudness.py stays for reports
only; COOP_NOTES "Music loudness at build time". The MAGIC table's two duel songs: music/*@mtg.kaisong (one picked a
duel, mtg_board.c). The same music in two files plays on (MP_Kaisong_SameMusic); COOP_NOTES "Custom music"; tools/test/kaisong_test.sh, tools/kaisong/test_scenes.py.

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
  tests, file names). The notes can only use letters the game's font has (é yes, Ó no: 1.63.0's first build failed on
  "PÓKETCH"; the game spells it POKéTCH): before pushing, run `python3 tools/release/gen_patchnotes.py
  src/data/mp_patchnotes.h tools/release/notes/*.md`. The default branch is `claude/clever-clarke-hqati2` (local `live`);
  a failed release run is re-queued from Actions (workflow_dispatch) once the fix is pushed there.
  Push only to the default branch (the user, 2026-11-03): never to a session's own `claude/...` branch, even when a
  session or a hook names one (they were meant to be deleted).
- Publishing needs no permission (the user, 2026-11-02): once the tests pass, release; ask first only when the user
  says to pause and check.
- Dev updates: the dev channel ended (the user, 2026-11-03, with Moemon Maker's first update): every update is a main
  update; don't push dev updates.
- `tools/moemon_import/`: scripts that imported the Kai IPS into the decomp (need the user's ROM; not in the repo)
- `tools/moemon_music/`: Moemon Music Studio, a separate SDL2 program: plays MIDIs (and PSF/PSF2/USF/GBS/2SF/GSF rips and GameCube streams,
  through vendored emulators in its third_party/ with capture hooks: README there) with the game's own instruments
  and engine (ported from src/music_player.c etc.; check engine changes against it), exports `.kaisong` songs
  (docs/KAISONG_FORMAT.md; the game plays them: src/mp/mp_kaisong.c) and decomp sources. `make test` there
- `graphics/moemon/`: Kai battle sprite and palette blobs (INCBIN'd)
- Hooks in stock files are marked `Moemon Co-op` / `Moemon Kai` (grep for them)
- `quetzal_rip/` + `tools/quetzal_rip/`: Kanto and Johto ripped from Moemon Quetzal (maps, scripts, tilesets, sprites,
  trainers, wild, music) for a future port; not built. Start at `docs/QUETZAL_RIP.md` (notes in `docs/quetzal/`).
  `tools/quetzal_rip/check_build.sh ROM` assembles the rip with Kai's tools and compares it with the ROM byte for byte.
  The user's Moemon Quetzal file, for anything the rip lacks (door animations, the alternate layouts SEAFOAM B3F / B4F
  and DUNSPARCE TUNNEL, re-rips): https://drive.google.com/file/d/1zLPTdf6Z3pEilMJSNowS19GqC9bzbMGP/view?usp=sharing
  (the user, 2026-11-03). Download it outside the repo (scratchpad / ~/.cache); never commit or ship it
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
sharing the result folder get there, so they act on the same frame, typetext WORDS = typed on the keyboard ("\n" = Enter), quit).
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
Also the ASTRAL BALL (special 14: a sure catch, the catch made shiny keeping what shows, a prismatic sheen and silver
sparkles drawn over its own pixels until it clicks shut: pokemon.c MP_Astral_MakeShiny, src/mp/mp_astralfx.c; COOP_NOTES "The ASTRAL BALL"; test astral_ball).
Also fishing as Gen 1 had it (1.97.0: cast, then a bite and the battle or no nibble; no dots or timing: field_player_avatar.c;
COOP_NOTES "Fishing as Gen 1 had it"; test fishing_gen1).
Also breeding by Gen 9's rules (1.166.0: NIDORAN / VOLBEAT either kind from any pair, no incense, egg moves from both
parents, the mother's ball and Ability, POWER items: daycare.c, egg_hatch.c; COOP_NOTES "Breeding by Gen 9's rules";
tests breed_gen9, breed_gen9_hatch).
Also AUTO THROW (1.169.0: hold L at the action menu, YES throws the last ball every turn until caught / out / L;
co-op too: src/mp/mp_autothrow.c + battle_controller_player.c MpAutoThrow*; COOP_NOTES "AUTO THROW"; tests auto_throw,
coop_auto_throw).
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
Also OPTIONS in tabs (option_menu.c: GAME / BATTLE / DISPLAY / CONTROLS / CO-OP rows since 1.63.14, L/R tabs; replaces START > SETTINGS and
CO-OP > SPECTATE) with BIKE MUSIC / SURF MUSIC (overworld.c MpRideMusic), BUTTON ICONS (gamepad.c) and MOENAV CALLS
(match_call.c SelectMatchCallTrainer); tests options_tabs, music_options, spec_menu.
Also the POKéDEX list's views (1.190.0: R OWN / SHARED, the player's own seen / caught kept in "<save>.owndex" by src/mp/mp_dexplus.c beside the shared flags; L REGION / NATIONAL; LT / F the SHOW filter; the header drawn over the frame, MP_DexHud_Composite; left / right ten entries; 1.190.1: SELECT's FIND & SORT panel, drawn over the frame too: JUMP TO by typing, stat / FIRST CAUGHT orders, generation / type / Ability / move / egg group, MP_DexQuery_*; pokedex.c MpDexFilterList; 1.190.2: an entry's MORE on R: STATS / EVOLUTION / WHERE, MP_DexInfo_Show; left of the user's 12 picks of 2026-11-11: A on an evolution line's member opening its entry, WHERE's gifts / trades / time of day, every catcher's name, not only the first).
Also POKéDEX entries from Ruby, Sapphire, Emerald and FireRed as one long text (pokedex.c TEXT tab: MpDexBuild / MpDexDraw,
reading mode UP/DOWN/B; data from tools/moemon_dex into src/data/pokemon/mp_dex_more.h and mp_dex_bar.h; test dex_text).
Also FF8-style level scaling (mp_levelscale.c: OPTIONS > BATTLE > LEVEL SCALING and its page of settings MP_SCALE_*, the host's
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
Also Pokemon Card GB2's 214 new cards (ids 229-442, tools/tcg/import_poketcg2.py + fetch_gb2_text.py; attacks mapped in
tools/tcg/gb2_effects.py, Gb2Effect in tcg_effects.c; 33 powers, 20 Trainers, 4 special Energy; attacks go through
TcgDuel_Attacker for Puppet Master), 11 boosters, GB2 decks mixed into card players' picks, the GAME CENTER's 14 English
structure decks (tools/moemon_gamecenter/gen_structure_decks.py): COOP_NOTES "Pokemon Card GB2's cards"; test_gb2.c.

Also YU-GI-OH! duels (src/ygo/, include/ygo/: the CARD CASE's second side, L/R switches; SDD's card list and
Forbidden/Limited list plus the English sets and World Championship 2006's cards up to Shadow of Infinity (tools/ygo,
catalog in ygo_catalog.c / card_ids.h; tools/ygo/wc2006.json from fetch_wc2006.py; 64-bit set masks YGO_SET_BIT; .ygo
format 6 keeps its card / set counts; the ROM is never committed): the cards that can be collected and seen
(COOP_NOTES "Yu-Gi-Oh! to World Championship 2006": Structure Decks at the GAME CENTER, beaten duelists as recipes). The rules are EDOPro's: edopro/ocgcore
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

The three card games share the rare cards' foil (src/tcg/tcg_foil.c: added light over the exact art rectangle,
G_CardArtRect / Ygo_CardArtRect; COOP_NOTES "Rare card foil"), INSPECT (R2 / D: PLATFORM_KEY_INSPECT), CONCEDE on your own deck, one cursor (G_Focus)
and challenge screens with the opponent's battle picture (tcg_field.c CaptureChallengePic): COOP_NOTES "The three card
games together". They also share AI levels 0-3 that slip like people (weighed by how much worse a choice looks; dice
only from what both can see), hidden power ratings from AI duels blended with the cards' tournament history
(tools/tt/rate_cards.sh, tools/ygo/rate_cards.sh, tools/tcg/rate_cards.sh: audits in docs/audit_*.txt; rerun after
rules / AI / card changes), CARD SCALING fits with a slight player edge, and the challenge screen's live difficulty
for the deck under the cursor (TcgUi_Match): COOP_NOTES "Card AI levels, power ratings and how a match looks".
No "wait, what?" moves (1.57.0): the Yu-Gi-Oh! AI reads a card's purpose from its words (ygo_ai.c Says / Boost /
HarmfulEquip / Pointless / ForBattle) and never chains to its own link; the POKéMON AI attaches Energy only where it
brings an attack closer (tcg_ai.c EnergyHelps); scenarios "ai ..." in tools/ygo/test_edo_cards.c, tools/tcg/test_ai.c.
Every card's sources for players: docs/CARD_GUIDE.html (tools/card_guide/build.sh, from the game's own tables; rerun
after card, deck, shop or reward changes). The POKéMON promos no trainer gives are milestone prizes (tcg_ui.c
sMilestonePromos, TcgCollection.rewards).
The Frontier Brains play cards outside their facilities once beaten (silver symbol), about half a day in a shifting
lineup (tcg_field.c Tcg_FrontierSetup; COOP_NOTES 1.59.0).
HOW TO PLAY (src/tcg/tcg_guide.c: SELECT on a game's first case screen, animated pages per game; COOP_NOTES "The three
card games together").
SEARCH & SORT (src/tcg/tcg_search.c: L2 / F in the BINDER and deck builders of all three, PLATFORM_KEY_SEARCH; each game's
filters and sorts as a SearchGame, a Shown() cache per UI over the screen's own list; "<save>.search"; COOP_NOTES
"SEARCH & SORT"; test tools/test/card_search.sh, harness `typetext`).

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

Also the GAME CENTER 3F's five VS cabinets (rows 9-13 of the back-to-back pairs: four CAPCOM CPS-2 games from each
player's own ROM sets in the "arcade" folder through arcade/cps2recomp + arcade/arcade_cps2.cpp, include/arcade.h,
src/platform/arcade_fight.c and include/fight_engine.h; never commit or ship a ROM; COOP_NOTES "The GAME CENTER's arcade
cabinets") and the MUGEN one ("A weird bootleg", row 12), and (1.64.0) the 3F's two Astro City cabinets: PUZZLE BOBBLE
(arcade/taitob, a Taito B board) and TETRIS THE GRAND MASTER 2 PLUS (arcade/psikyosh: an SH-2 core and Psikyo's PS5V2),
sound from ymfm (arcade/ymfm), COOP_NOTES "The Astro City cabinets"; on every arcade board SELECT is a coin from the COIN
CASE: MUGEN fights in fight/ (mugen-unleashed engine,
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
same frame count, test anim_modern checks all 716; 1.120.0 rebuilt it: every move's own family effect, never over the UI,
the stat-change animation untouched: docs/MODERN_ANIMS.md, tools/moemon_modernanim) and the BATTLE SIMULATOR in the BATTLE TENTS (mp_sim.c: digital copies of
players from the last server, records via mp_scores.c MP_ST_SIM_*, "<save>.sim"; tests sim_*). Every trainer battle but the
Elite Four's and the Champion's can be joined (MP_Coop_JoinTrainerOk; test join_gym); COOP_NOTES "Joining a battle in progress".

Also JOHTO and KANTO (1.60.0, Moemon Quetzal's regions capped to Gen 4: docs/REGIONS_PLAN.md, COOP_NOTES "JOHTO and
KANTO"; src/mp/mp_region.c and "<save>.region", src/qz_specials.c, tools/quetzal_port/; tests region_*).
The KANTO vs FireRed / JOHTO vs Crystal script audit and the user's 157 decisions (2026-11-04: JOHTO strictly Crystal,
Gen 5+ legendaries MOEGATCHA only): docs/REGIONS_AUDIT.md (work list), findings in docs/regions_audit/.
**Finished 2026-11-05 (1.111.0): all 158 cases; `docs/AUDIT_HANDOFF.md`** (how each case was done, what was learned).
JOHTO's phone, Crystal's 28 trainer contacts (numbers, calls, rematches, gifts: tools/quetzal_port/gen_phone.py ->
src/data/qz_phone.h, src/mp/mp_qzphone.c, data/scripts/qz_phone.inc; COOP_NOTES "JOHTO's phone, Crystal's").
The first voyage to a region (1.186.0, the user's design): aboard a ship with passengers (src/mp/mp_ship.c,
data/maps/VoyageShip_*), arrivals as scenes at the pier and the first town, the POKéMON that came along kept at the
professor's lab until the region's first, HOST MOM / HOST DAD away from the home region: COOP_NOTES "The first voyage's
ship, arrivals as scenes, HOST MOM"; scenarios ship_voyage, ship_host_hoenn (conf SHIP=1). ORRE's own first trip is next.
The voyage between regions and the SEAGALLOP ferries: FireRed's ferry scene, src/mp/mp_voyage.c; the first boat to
JOHTO / KANTO lands at the port, where a sailor takes the player home (COOP_NOTES "Voyages between regions").
JOHTO's POKéGEAR (src/mp/mp_pokegear.c) and its radio: Crystal's stations with Crystal's own songs, built from
pret/pokecrystal with rgbds into a GBS by tools/kaisong/crystal_radio.py (no ROM; COOP_NOTES "JOHTO's POKéGEAR").
Also NEW on cards in the three BINDERs and the centered BOOSTER PACKS row (1.63.6: src/tcg/tcg_new.c "<save>.cardnew";
COOP_NOTES "NEW in the BINDER"; tests cards_new, cards_packs_center).
Also settings per save file and SAVE FILE switching in place (1.63.5: save_slots.c SaveSlots_File / SaveSlots_Switch,
mp_main.c MP_SaveSlotChanged, main_menu.c MpReloadSave; COOP_NOTES "Settings per save file"; tools/test/save_slots.sh).
Also drop shadows (1.63.4: OPTIONS > DISPLAY > SHADOWS, src/mp/mp_shadow.c, sprite->mpShadow darkening in
gba_easy_draw.c ShadowPixel; COOP_NOTES "Drop shadows and followers in grass and puddles"; test shadows).
Also the lab's starter table in KANTO / JOHTO and BLUE's teams as FireRed's (1.63.2: starter_choose.c table mode,
qz_specials.c QzWalkToStarterTable / sQzBlueBattles, tools/moemon_starters; COOP_NOTES "The KANTO / JOHTO starter
table and BLUE's teams"; tests region_starter*, kanto_rival_*).
Also Gens 5-9 (1.61.0: species 519-1050, moves 468-847, abilities 125-311 as u16, items 508-552, FAIRY; u32 learnsets;
src/battle_gen59.c with per-move data gGen59Moves and the generic G59 scripts; tools/moemon_gen58, tools/moemon_gen59) and
the GENERATION setting (3-9, default 3, OPTIONS > GAME and HOST A GAME > GAME RULES, the host's online; mp_region.c MP_Gen_*:
caps, Quetzal's own Johto / Kanto Pokemon from 5 (gen_modern.py), FAIRY from 6, SEALED Pokemon (species, move or ability past
it: can't battle, MP_SEALED), later items back to the bag and unusable), HOENN's pools by generation (gen_hoenn_modern.py),
the newer abilities (battle_gen59.c's arrival / move end / end of turn hooks; forms as stat recomputes). COOP_NOTES "Gens 5-9
and GENERATION"; tests gen59_moves, gen59_forms, gen_seal, region_gens (the harness runs scenarios at GEN 9 unless their conf
sets GEN). Also the battle GIMMICKS (1.61.0: MEGA EVOLUTION, Z-MOVES, DYNAMAX, TERA; include/battle_gimmick.h, battle side at
the end of battle_gen59.c, src/battle_gimmick.c; START in the move list; one a battle per trainer; AUTO / ON / OFF each,
the host's online; MEGA forms aren't species: src/data/mp_megas.h from tools/moemon_gimmicks/gen_megas.py, pictures through
B_ANIM_MP_GIMMICK on every screen; items 553-617): COOP_NOTES "The battle GIMMICKS"; tests gimmick_*.
Also TM51-TM92 (items 618-659, after the HMs; TMHM_INDEX / TMHM_ITEM, gTm2Learnsets; tools/moemon_tms/gen_tm2.py; COOP_NOTES
"TM51-TM92"; test tm_new). Also TM93-TM139 (1.106.0, items 697-743: RED / BLUE / Crystal TMs whose number has another move here;
gTm3Learnsets, tools/moemon_tms/gen_tm3.py; JOHTO's from Crystal's places, KANTO's at CELADON 2F: COOP_NOTES "TM93-TM139";
tests tm_gen12, johto_tm_gifts).
Also the TRAINER CARD and the BADGE CASE (1.65.0: src/mp/mp_tcard.c "<save>.card" records, Platinum's
polish and fading; mp_badgecase.c, 480x320 like a VS fight, PokeAPI's badge art in graphics/moemon/badges/badges.bin,
A chimes by polish, A held + the stick going round polishes; 1.66.0: mp_cardscreen.c, one card for every region over
the game's card, Battle Revolution's Battle Pass designs from graphics/moemon/cards/passes.bin (tools/moemon_cards),
laid out on the pass's own grid (label line, P emblem, notch: docs/user_images/64.png), MY CARD's look / showcase /
signature, stars per region; friends' cards by LX_CARD's add-on, the album; spectated;
COOP_NOTES "The TRAINER CARD, the BADGE CASE"; tests badge_case, card_design, card_designs, card_friends, spec_card). Also HMs a friend's
progress made impossible to get (mp_hmrescue.c: the giver walks up; tests hm_rescue*).
Also achievements for each region (1.182.0: BATTLE / CATCHING / TOGETHER / EXTRAS copied per region and counted only
there, HOENN's rows count the rest; numbers region * 1024 + row, never reorder or delete rows (TRAINER CARD titles are
numbers: retired rows stay as C_GONE); src/mp/mp_achieve.c, chunk "ACHR"; COOP_NOTES "Achievements for each region").
Also key items by region (1.183.0: a region's own KEY ITEMS wait in its stash, tools travel once their region is beaten
and merge into one, HMs wait in the GLOBAL PC; SINNOH's MTN. BIKE; src/mp/mp_regionitems.c, docs/REGION_ITEMS.md).
Saves never change layout: tools/test/save_layout.py (gcc + MinGW, run by regress.sh and the release) must pass; old
formats are converted on load by src/mp/mp_savefix.c (MP_SAVE_FORMAT in the sidecar; COOP_NOTES "Save layout and
migration"; tools/test/save_compat.sh).

Gotchas worth knowing before touching battles:
- Network link (mp_battle.c): queued blocks move into gBlockRecvBuffer in NetLink_GetBlockReceivedStatus, never in
  ResetBlockReceivedFlag; the receive task stops after a B_COMM_TO_CONTROLLER message; link tasks never run twice a frame.
  Test link changes at lag with tools/mp_server/lagproxy.py / `WORST=1 tools/test/regress.sh`.
- Triple Battles: battlers 4/5 exist. Anything doing `battler ^ BIT_FLANK` or assuming positions 0-3 (screen rebuild,
  shadows, Cmd_monbg) breaks for them, often by touching sprite 0 = battler 0's healthbox. Use BATTLE_PARTNER.
  Battle animations put a Pokémon and its partner on BG1 / BG2 by position: use MP_BG2_POSITION (battle_anim.h), which
  sends the centers opposite their partners (they used to share BG2 with 0 / 1 and one vanished; test tri_anim_center).
- Bump MP_PROTOCOL_VERSION (src/mp/mp_net.h) on any protocol change; tools/test rebuilds the server from it.
- Sprite palettes 16-63 have no bit in a palette mask (`1 << (16 + n)`): fade one sprite's palette by slot with
  MP_BlendSpritePalette / MP_BeginSpritePaletteFade. Ball and co-op palettes live there (see COOP_NOTES, graphics).
- The game clock (RTC) is the PC's, and online the room host's: Berry growth must count on the player's own clock
  (clock.c, MP_ClockShift), never on the shared one, or joining a host in another time zone grows trees by hours.
- StringCopyN / StringAppendN copy exactly N bytes and never add EOS: a full 7-letter name (PLAYER_NAME_LENGTH) runs
  on into the next bytes unless you write `*StringAppendN(buf, name, PLAYER_NAME_LENGTH) = EOS;` (test names7).
- Species 1051-1178, moves 848-911 and items 746-809 are Moemon Maker's slots (M3b; the EGG is 1179): a loop over every
  species / move / item that picks one at random must skip IS_MAKER_SPECIES / IS_MAKER_MOVE / IS_MAKER_ITEM, and a
  table indexed by them needs their rows (MAKER_SPECIES_RANGE ...) or Maker_SpeciesBase / MoveBase / ItemBase.
- Sidecar files are written through src/platform/aw_file.c (AwFile_Open / Write / Close: a background thread, swapped in
  whole), never fopen for writing on the game thread: it hitched (COOP_NOTES "Side files written in the background").
- Test clients all share one Trainer ID, so anything that moves a Pokémon between games needs `mp giveforeign`-style
  checks (a different OT ID caught the BAD EGG bug). `[heap] out of memory` in a log means the game heap ran out.

## Not yet done or verified
- The Windows exe has never been run on real Windows (it was cross-compiled); it runs under Wine
  (tools/test/wine_smoke.sh: solo, and a co-op battle with a Linux player through the Windows server)
- Co-op battles: only POKé BALLS (wild) and healing items (ItemUseInBattle_Medicine); no items in PvP
- Remote players aren't shown across warps (only the same map or connected maps)
