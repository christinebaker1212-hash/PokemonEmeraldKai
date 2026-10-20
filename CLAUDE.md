# Moemon Emerald Kai Co-op: notes for Claude

Native PC build of Moemon Emerald Kai v1.0 with online 4-player co-op and PvP.
Based on Kurausukun's `pc_port` branch of pret/pokeemerald (commit 1165825), SDL2.

Read `BUILDING_COOP.md` first for the architecture and build steps. The player
docs are in `dist_extras/README.txt` (shipped next to the exe).

## Key locations
- `src/mp/`: all co-op code (net, client, avatar, overworld, trainer mode, battle, menu)
- `include/mp.h`: shared co-op API. `src/mp/mp_net.h` holds the protocol (bump `MP_PROTOCOL_VERSION` on changes)
- `tools/mp_server/`: standalone relay server
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
Emote / phrase pick (mp_menu.c MP_OpenQuickPick, the only emote screen): LT/RT or Q/E held = held mode (release sends),
right-stick click / SOCIAL > EMOTES = sticky (A sends, B closes). Pages EMOTES, MINE 1-4 (16 phrases, MP_NUM_PHRASES,
ini phrase1-16; MINE n shows once page n-1 has one) + 4 stock groups (L/R); SELECT (pad west) swaps/pins
gMpConfig.quickStock (first MINE page only, ini quick_stock), START (pad north) edits MINE (INPUT_CONTEXT_QUICKPICK).
Test with `vpad xbox`, `vaxis rt 100`, `vbtn right 1`, `vbtn rstick 1`. Button icons (mp_buttons.c, hooked into text.c DrawKeypadIcon): CHAR_KEYPAD_ICON chars draw the last used
controller's buttons (gamepad.c Gamepad_IconFamily; art graphics/moemon/buttons/buttons.png -> tools/moemon_buttons/build.py
-> src/mp/mp_buttons_data.h). Co-op ASCII text writes them as {A} {B} {L} {R} {START} {SELECT} {LEFTRIGHT} {LT} {WEST}...
(MP_AsciiToGame); extra ids CHAR_MP_LT..CHAR_MP_LS. Test with `vpad xbox|ps|nintendo`.
New game LOOK step (mp_intro.c CB2_MpIntroLook, main_menu.c Task_NewGameBirchSpeech_LetMeSeeYou): after the gender
pick the LOOK page opens in intro mode (mp_menu.c MP_OpenLookForIntro) on Birch's backdrop; harness condition `introlook`.
SKIP THE INTRO (main_menu.c Task_NewGameBirchSpeech_AskSkip -> CB2_ChooseStarter -> nickname -> Birch send-off;
mp_quickstart.c MP_QuickStart_Apply at the end of NewGameInitData): the end state of truck..Route 103 rival..running shoes
for both genders, starter at the level the two intro battles give, start at Littleroot 7,17. Harness condition `naming`.
Welcome note (mp_setup.c HOST_NOTE, ini `welcome = a|b`, mp_main.c SendWelcome/ShowWelcome, MPR_WELCOME): the room
creator sends it on a new player's info; shown via MP_ShowNotice once on the map. Harness condition `notice`.
Pings (mp_ping.c, MPR_PING, INPUT_ACTION_PING: V / left stick click): bubble slot PING_SLOT in mp_emote.c, blinking inset
edge arrow via mp_compass.c MP_Compass_PointAt. Test: `vbtn lstick 1`, run2.sh WARP_B for a second start position.
Lend field moves (mp_lend.c, MPR_LEND): each client broadcasts the first party Moemon knowing CUT/SURF/STRENGTH/
ROCK SMASH/WATERFALL/DIVE; scrcmd.c checkpartymove falls back to the nearest friend within 4 tiles and returns MP_LEND_SLOT
(7), which bufferpartymonnick, FldEff_FieldMoveShowMonInit, FldEff_UseStrength and PartyHasMonWithSurf understand.
Test: Route 116 cut tree, WARP=0,31,23,7 (lender) WARP_B=0,31,21,7, `mp teach 0 15` on the lender, `mp givebadges`, `mp lend`.
TEAM PHOTO (mp_photo.c, SOCIAL > TEAM PHOTO, MPR_PHOTO): 2+ players on the same map, front pics via
mp_avatar.c MP_Avatar_PlayerFront, frame captured from Platform_GetFrame and saved 3x with mp_pack.c MP_WritePng to
photos/. Debug `mp photo`, harness condition `photo`. The co-op overlay palette (MP_TAG_UI_PAL) borrows an NPC reflection
slot when slots 12-15 are full (sprite.c MP_ClaimSpritePaletteSlot + IndexOfSpritePaletteTag) and skips weather tinting.
Joined frames (mp_ui.c MP_FrameJoin_*): co-op windows that touch get joint tiles at BG0 tiles 0x2F0-0x2FE.
Follower Pokémon (SELECT in the party menu): local trailing, 64x64 sprites, talking to it,
and remote followers seen by the other client.
Phase 1 co-op: emotes/phrases (bubbles seen remotely), FRIENDS > GO TO, Town/Fly map heads, door
labels, shared Pokédex (+ CAUGHT BY), buddy bonus, co-op Roxanne win giving both players the badge
(test: MOEMON_DEBUG_WARP=11,3,5,3 / 11,3,5,4, MOEMON_DEBUG_PARTY=283:46, `mp badges`).
Phase 2: shared PC (put/take Pokémon and items, `MOEMON_STORE_DIR`), trade with money/items and a
trade evolution (Kadabra), daily gift + thank-you bonus. Battle Flag hides name tag and follower.
Phase 3 (mp_events.c, mp_gatcha.c): goals/toasts/MoeGatcha/streak with `mp event ...`, `mp savedata`, `mp gatcharig N` (0 N .. 6 EX; 5 = legendary event),
`mp goals`, `mp pickup N` (0-18), harness `letters WORD`, MOEMON_DEBUG_DATE; `AllowSmallTweaks = 1` in the ini enables
the MoeGatcha typed codes (mp_gatcha.c CheckTypedCodes, off by default, only from the file) and typing "speed" anywhere
(5x until "slow"; text_input.c CheckTypedWords -> sdl2.c Platform_SetSpeedUp, harness `letters speed`); partner-by-partner co-op goal checked with three clients (run4-style).
Phase 4: co-op wild battles (mp_coop.c, `MP_Coop_TryWildTeamUp`): a land encounter pulls in the closest
free friend standing in grass within 8x6 tiles (no prompt), 2 players vs 1 wild Pokémon (the right
opponent slot starts absent). POKé BALLS work (only balls); the catch goes to the thrower via the
result message. Test: Route 101 grass, MOEMON_DEBUG_WARP=0,16,12,11 / 0,16,14,11, `mp wild 288:3`,
`mp item 1 2`, `waitfor action`, `mp party`. Verified: catch by the joining player, knock-out win
(EXP for both), friend out of range -> normal single battle. 2-on-2 (30%, gMpWildDoublePercent, debug `mp wilddouble N`): second mon from
wild_encounter.c MP_RollExtraLandMon on the right (enemy slot 3); a catch with the other still out runs
BattleScript_MpCoopBallCaughtGoOn (VARIOUS_MP_COOP_CAUGHT_ONE/REMOVE_CAUGHT). Only the link master runs battle scripts:
visuals for both screens must go through controller emits (sprite invisibility value 2 = caught, link partner ball throw). Also gym rematches and tournaments.

Batch 3 (mp_linkx.c): record mixing over relay (record_mixing.c MP_RecordMix_*, mp_battle.c
NetLink_BeginRecordMix fakes a 2-4 player link), friend Secret Base visits (secret_base.c MP_SecretBase_*,
EventScript_MpVisitSecretBase), link Trainer Cards, Day Care co-op eggs (daycare.c MP_Daycare_TryCoopEgg,
`<save>.coopegg`), shared roamer/weather, Hall of Fame friend teams. Debug: `mp mkbase 181`, `mp daycare N`,
`mp coopegg`, `mp takeegg`, `mp hof`, `mp roamer`, `mp world`. All verified with two clients.
No UNION ROOM page any more: CO-OP > FRIENDS > a friend (mp_linkx.c MP_Linkx_OpenFriend) has GO TO, TRAINER CARD,
VISIT SECRET BASE; CO-OP > BERRY GAMES (MP_Linkx_OpenUnion) has the blender and crush. Mixing is automatic only.
Minigames (mp_minigame.c): BERRY BLENDER (berry_blender.c NPC mode with friends in the seats, leader syncs
arrow/speed/progress and final numbers) and BERRY CRUSH (a relay stand-in for the RFU calls via macros at the top
of berry_crush.c). Debug: `mp blender`, `mp crush`, `mp pokeblocks`, condition `crushask`. Link CONTESTS
(mp_grouplink.c): a 2-4 player link behind the NetLink_* calls (link.c) + matchmaking in TryContestEModeLinkup.
Test: MOEMON_DEBUG_WARP=13,4,15,4 on both, UP UP, mash A. All verified with two clients.

LOOK (mp_avatar.c, mp_menu.c): FRLG overworld sprites are catalog-only ids (MP_GFX_EXT_FIRST + n, mp_ext_gfx.h);
ID PIC / BK PIC choices (MpAppearance.frontSel/backSel, 0 = MATCH) load into the TRAINER_PIC_MP_* runtime slots.
Assets: tools/moemon_frlg/import.py (pokefirered checkout + Showdown download, not in the repo) writes graphics/moemon/ext.
Colors are hand-mapped per sprite in tools/moemon_frlg/colormaps.txt -> colormaps.py -> src/mp/mp_color_maps.h.
To map new sprites: `mp dumpcolors DIR`, colorsheets.py (highlight sheets), colorcheck.py (test looks).
LOOK > CREATE (mp_creator.c, mp_menu.c HandleCreateInput): CUSTOM 1-3 trainers from a parts kit. A look is a
14-byte recipe (struct MpCreated, MpAppearance.recipe, ini custom1-3 as hex, sprite = custom_N); every client
composes the 32x32 field frames (walk/run/Mach Bike), 64x64 front and 4-frame back and caches them per player
(MP_Created_Gfx). Pictures: graphics/moemon/creator/parts.bin (tools/moemon_creator/build.py KIT_DIR; the kit isn't in
the repo). The few Gen 4 walking parts build_ow.py borrows live in tools/moemon_creator/gen4_ow.bin/.json (not in the exe).
Field sprites (mp_creator.c ComposeField): Gen 3 style, baked by tools/moemon_creator/build_ow.py KIT3_DIR FRLG_DIR
(the Gen 3 Top Down Sprite Maker kit + a pokefirered checkout; neither in the repo) into graphics/moemon/creator/ow3.bin +
src/mp/mp_creator_ow.h: 27 frames per piece with color roles (skin shade / recolor group / fixed). Kit: body, head,
hair (30, recipe.fieldHair 0 = AUTO), hats (HAT_KIT), shirt/pants/shoes. Official FRLG donors (by body band, DONORS table):
skirts, shorts, coat, overalls, swimwear, bags. Small head accessories borrowed from the CREATE parts (HAT_BORROW).
Exact Gen 3 skin ramps (SKINS). `build_ow.py ... --preview out.png` renders looks with the same steps as the C code.
Test: ini `sprite = custom_1` + `custom1 = 0101020304000307010a0102060000`, `mp bike` / `mp bike 0`. Verified with two clients.
Extra field hairstyles (fieldHair 31-144): tools/moemon_creator/donor_hair.py cuts them from aveontrainer's FRLG-style
overworld sheets (2x art, `donor_hair.py --fetch DIR`; not in the repo) listed in hair_donors.txt; build_ow.py takes the
folder as a 3rd argument (Kit3.add_hair). `donor_hair.py IMG_DIR KIT3_DIR out.png [names]` renders review sheets.
Only clean cuts are listed; options per donor: long= (stop following long hair), noacc, eyes=, dx/dy.

## Not yet done or verified
- The Windows exe has never been run on real Windows (it was cross-compiled)
- Only POKé BALLS (co-op wild battles) can be used in co-op battles; no items in PvP
- Remote players aren't shown across warps (only the same map or connected maps)
