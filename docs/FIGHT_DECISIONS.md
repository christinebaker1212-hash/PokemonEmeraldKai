# VS cabinet (MUGEN fights): decisions and plan

Agreed with the project owner (October 2026). The engine is mugen-unleashed
(https://github.com/christinebaker1212-hash/mugen-unleashed, inspected at
`eea065966ab4c6557f811d707ec05221ba0f7ae9`): a C rebuild of M.U.G.E.N 1.1 Beta 1, with no game
data or Elecbyte files.

## Asked for directly
- A playable fighting game in the game, with two players fighting online with rollback.
- The game stays 240x160; the fight system alone may use double resolution (480x320).
- For now, one Game Corner slot machine becomes a VS cabinet. One player sits on one side and a
  "Fight me!" bubble appears. When another player sits on the other side, the character select
  opens, a best-of-2 fight plays out, then both players are put back in the Game Corner.
- Adding or removing characters, stages, music and screenpacks must be easy.
- Nothing may be cut off, squished or off screen, automatically.
- All music and sound effects are brought down to GBA sound.

## Answers
| # | Question | Answer |
|---|---|---|
| 1 | Which machines | The right block's bottom row: (7,9) and (8,9), back to back, nearest the door. The other 10 stay slot machines |
| 2 | "Best of 2" | First to 2 round wins (up to 3 rounds, always a winner) |
| 3 | While waiting | Seated facing the cabinet, the bubble visible to everyone on the map; B stands up and removes it |
| 4 | Stakes | None: wins and losses only counted (trainer card / achievements) |
| 5 | 3:2 screen vs 4:3 packs | Widen the view: scale by height, the camera shows more stage at the sides, the HUD anchors to the real edges; stages too narrow get their edge layers extended |
| 6 | Sharpness | 480x320, shown with bicubic interpolation (the fight only; the rest of the game keeps its own filter). Changed from nearest-neighbor in the third round. Replaced in 1.50.0: the picture goes through the game's own scaling and filters, sprites are resampled inside it ("Pixels" below) |
| 7 | HUD off screen | Nudge each element back inside an 8px safe margin, keeping its size; a log says what moved |
| 8 | Character select | The pack's own screenpack (system.def / select.def / VS screen), fitted by rule 7; both players pick at once |
| 9 | Transport | Direct UDP between the two players (introduced by the room server, with the game's port mapping); the relay if UDP fails within 2 s |
| 10 | Input delay | Automatic from the ping measured on the select screen: 1-3 frames delay, rollback covers up to 8 frames; a warning at 250 ms+ |
| 11 | Net HUD | Nothing, unless rollbacks reach 5+ frames or packets are lost: a small signal icon in a corner |
| 12 | Drop / desync | "Waiting for NAME... 10": play on if they come back, else no contest (nothing recorded). A desync silently loads the host's state |
| 13 | Content folder | A plain MUGEN folder, `mugen/` next to the exe, standard layout; edit select.def to add or remove; picked up the next time someone sits down |
| 14 | Shipping / differing packs | Releases ship the owner's pack; at select, characters and stages the other player lacks (by file hash) are greyed out |
| 15 | Music formats | OGG, MP3, WAV, FLAC (no MIDI / tracker) |
| 16 | GBA sound | Everything converted at load to 8-bit signed at 13379 Hz (sound effects mono, music stereo), played through the game's mixer |
| 17 | Settings | Per cabinet in `mugen/cabinets.ini` (life, round time, speed, stage list...); online the host's file wins; no in-game menu |
| 18 | Stage | Random from the cabinet's list, shown on the VS screen |
| 19 | Spectating | The SPECTATE panel shows the fight (as for card duels); bystanders see both players seated with a crossed-swords bubble over the cabinet |
| 20 | Sides | West seat (facing east) = P1 on the left |
| 21 | Buttons | Arcade layout. Pad: X Y RB = x y z, A B RT = a b c, Start = start. Keys: U I O / J K L, Enter. Rebindable on a new OPTIONS > CONTROLS > FIGHT row |
| 22 | Bubble | The game's own speech balloon and font, gently bobbing like a nametag; A at the empty seat asks "Fight NAME? YES / NO" |
| 23 | Transition | The CRT turns on (fade to black, a line opens into the picture) into the select screen; it turns off after the results, and both players stand beside their seats facing the cabinet |

## Second round (same day)
- Every player is on Windows. Linux / Windows agreement doesn't matter (the Linux build only runs
  the headless tests), but both players' Windows builds still have to agree.
- Elecbyte's files are fine to use (a private build). Even so, the pack's Elecbyte characters,
  stages and screenpacks are gone. What stays is the engine's own data: `common1.cns`,
  `common.snd`, `fightfx.sff` / `.air`, and the `f-4x6` / `f-6x9` fonts the engine loads at
  start-up.
- The screenpack is Phantom Blood's STREET FIGHTER EX PLUS (640x480), in
  `mugen/STREETFIGHTER-EX-PLUS/` (where its own paths expect it). Only what the select, VS, fight and victory screens read
  is kept. Removed:
  - the intro, intro EX (122 MB), title, ending, game over and winner storyboards and their
    music;
  - SFEX-SYSTEMEX.def and the 110-slot variant.

  The removed storyboards are switched off in SFEX-SYSTEM.def, along with demo mode and the
  continue screen. The grid is 12 x 3 cells, centred (it was 16 x 5, with rows 4-5 and columns
  15-16 off screen). There are no `empty` lines: characters fill the cells in order, so adding one is
  a new line in SFEX-SELECT.def.
- Test characters (SSF2X), in the SF2 select order: Ryu, Ken, Chun-Li, Dhalsim, Cammy,
  Fei Long, Dee Jay, T.Hawk, Vega, Gouki, with RANDOM at both ends of the row.
  - Other versions in their folders are not listed: Chun-Li 2, Dhalsim 2, Gouki (normal) /
    Shin / EX Shin, and SF2'CE Vega.
  - Stages: the 18 SF2 stages, plus 6 that came inside character folders. Those 6 are now in
    `stages/`: SSF2X Chun-Li and Dhalsim (with their songs), Gouki's, and Vega's three.
- Stage music: the songs aren't fetched for the owner. `mugen/sound/README.txt` lists the 17 file
  names the stages wait for.
- No new cabinet tiles: the slot machine tiles stay as they are.
- Sitting down offers WAIT FOR 2P or FIGHT CPU. A second player who sits during a CPU fight
  cancels it, and the select screen opens for both.

## Third round
- (Until 1.50.0, see "Pixels") The fight is shown with bicubic interpolation, only the fight (#6). `Fight_CanvasBicubic`
  (src/platform/fight_platform.c) scales each new frame with Catmull-Rom on the CPU to the window's
  size, at most 960x640, and the renderer's linear filter takes it the rest of the way. A window of
  960 or wider is exactly 2x, which has its own quicker path.

## Fourth round
- The music played about 8 times too fast in the game (the mixer took a whole block from the music hook
  each time it wanted a few samples). It now pulls whole blocks; `fight_music` checks the tempo with a
  song of numbered tones.
- No black bars: the 4:3 select, VS and results screens' sides carry on their background (Screen
  fitting below).

## As built (1.33.0)
Where the plan below changed on the way:
- The engine is in `fight/` (engine/, compat/, host/, third_party/), the game's side in
  `src/platform/fight_platform.c` (picture, buttons, sound, the overlay and the CRT) and `src/mp/mp_fight.c`
  (the cabinet, the netcode). The API is `include/mf_engine.h` (`MfEngine_Begin` / `Step` / `RollBack` /
  `Hash`...). The engine runs on its own thread as a coroutine on a stack inside the fight's heap.
- Rollback keeps the pages each frame changed (Windows: GetWriteWatch; Linux: write protection) of one
  heap region at the same address on every PC, plus the engine's globals in one section; 10 frames. The
  session starts from the same start-up image every time, so a CPU fight before an online one changes
  nothing.
- The hash every 30 frames covers the players, the timer and the RNGs. A desync (#12) loads the host's
  state, not a whole snapshot: both games stop after the same frame (the frame goes with the button
  packets), the other game sends a hash of each 4 KB page of its state, and the host (P1, the west seat)
  sends the pages that differ; a word holding a pointer into the other PC's own memory (code, the C
  library's heap) stays as it was there. The globals only go across between the same builds.
- Button packets carry up to 12 frames, from the first the other game hasn't got. UDP gets through by
  the room server's echo (`MFWHO` / `MFYOU`); otherwise the relay carries them.
- The pack check (#14) hashes every file each character uses (its folder) and each stage (its .def,
  sprites and music); the engine's data, the screenpack and the fonts have to match for a fight at all
  ("NAME's fight files aren't the same as yours"). The answering player's cabinet rules go with the
  answer, and both use them (#17).
- The stage's name is on the VS screen where `vs.stage.offset` / `vs.stage.font` in cabinets.ini put
  it; the stages got clean display names (`displayname` in each .def).
- Wins and losses (#4): counted per save (against friends and against the CPU), five achievements
  (INSERT COIN, ARCADE MODE, A NEW CHALLENGER, WORLD WARRIOR, GRAND MASTER) and a VS FIGHTS line on
  the back of the TRAINER CARD.
- Buttons (#21): OPTIONS > CONTROLS > FIGHT BUTTONS, a page with each fight button's key and pad button
  (A, then the key or button; Esc / Back or 8 s keeps it). Kept in moemon_controls.ini (`fight_key_x`,
  `fight_pad_x`...). The directions stay the arrows / WASD, the D-pad and the left stick.
- On screen (#11, #12): the "Waiting for NAME... 10" countdown and a signal icon, drawn over the fight's
  picture by the game (a 5x7 font).
- Tests: `fight_cpu`, `fight_online`, `fight_online_lag`, `fight_cpu_join`, `fight_drop`, `fight_music`,
  `fight_buttons`, `fight_packs`, `fight_packs_core`, `fight_resync` (tools/test/README.md), and the
  engine's own `tools/fight/build_linux.sh` self test (`WIN=1`: under Wine).

## Asset ingestion (1.52.0)
- Answers left to judgement (#54, #55):
  - where it runs: the build (Makefile `fight-ingest`, part of `make linux` / `make winwsl`, so the release
    workflow runs it too) whenever a file in mugen/ or tools/fight/ is newer than the last run, plus a check at game
    start;
  - in place or copies: in place, in mugen/, with atomic writes. Every repair is safe to repeat, and git holds the
    originals;
  - where the record and log live: `mugen/processed_assets.json` and `mugen/ingestion_log.txt`, next to the pack
    they describe, so they travel with it. `mugen/processed_assets.idx` is the game's quick index.
- `tools/fight/ingest.py`:
  - finds the characters (the select list, plus any character .def in chars/), the stages (stages/*.def and the
    ones named) and the shared files, with each one's dependencies;
  - processes an asset when it is new, when a dependency's content changed (SHA-1), when it is marked in progress
    (an interrupted run), or when the rules (RULES_VERSION) or tools and settings (a hash of the tools and
    balance_baseline.json) changed;
  - drops removed assets from the record;
  - logs everything; `--force` processes everything again, `--check` changes nothing.
- What it applies by itself (keeps behaviour): GBA sound (gba_audio.py) and debug-text controllers in states
  -3..-1 switched off. The checks it runs: check_chars.py, stage validation, and the roster's balance audit when a
  character changed. What waits for `--apply-proposals`: a ChangeState / TargetState to a missing state where one
  state of the same block of ten exists (a likely typo). Balance changes are only reported.
- Game start (fight_platform.c Fight_CheckIngestion, a detached thread, so start-up doesn't wait): compares
  file sizes with the index (times aren't compared, because unzipping an update resets them) and looks for .defs
  the index lacks. Differences go to ingestion_log.txt ("game start: N changed, M gone, K new ... run
  tools/fight/ingest.py") and to stdout. It changes nothing.
- First run on the shipped pack: 41 assets. Debug text was switched off in 7 characters. Stage findings: 17 stages
  name songs that aren't shipped. Balance: 0 likely mistakes.
- Tests: tools/fight/test_ingest.sh (first run, nothing to do, a changed file rechecks only its character, a new
  character, a removed stage, --force without repeated repairs, an interrupted run and its resumption, a proposal
  listed and then applied).

## Pixels: resampled sprites, and the game's own display (1.50.0)
- Asked: rewrite the scaling (not the same algorithms) to fix distortion, uneven pixel stretching and sprite
  shearing; show the fight inside the game's display so its scaling, pixel grid and spectating apply; keep the
  three modes alike; keep the simulation and the intended effects. Answers: keep 480x320 but through the
  game's scaling, grid and spectate path (#51); resample rather than whole scales (#52).
- The path, inspected end to end: the engine's camera and positions are floats (simulation, untouched); its
  software drawers put 8-bit sprites on the 480x320 RGB565 target (the canvas; the GL renderers are stubs);
  MfHost_Present double-buffers it; fight_platform.c adds the messages and the CRT; display.c showed it
  bicubic, on its own path.
- What was wrong: 320x240 characters and stages are drawn at 4/3 and the 640x480 screenpack at 2/3, nearest:
  every third column / row doubled (4/3) or dropped (2/3: letters and lines lost strokes), and which ones
  moved with the sprite's fraction of a pixel, so pixels crawled as the camera moved. Parallax floors put
  each row at a truncated whole pixel and picked rows nearest: the floor's rows stepped against each other
  (the "shearing"; the shear itself is MUGEN's own for "xscale" floors, the GL path does the same, so it
  stays). Rotated sprites (AngleDraw) were nearest too. 1:1 sprites truncated their position towards 0
  while scaled ones rounded.
- Now (fight/compat/mf_softscale.c, hooked into softrender.c / softblend.c / bg.c):
  - every scaled draw is area-resampled: each screen pixel the coverage-weighted average of the source
    pixels under it, from the sprite's exact float position and size, so each source pixel covers the same
    4/3 or 2/3; colour 0 lowers the coverage (outlines as even as the rest); the rectangle's outer edge
    follows the pixel-centre rule as nearest did, so tiles placed edge to edge still meet without a seam;
  - the drawer's own operation (copy, subtract, add, alpha by alpha_src / alpha_dst) is applied to the
    averaged colour and mixed in by coverage: fully covered pixels get exactly what MUGEN's operation
    gives;
  - rotated sprites: 4x4 samples per pixel through the drawers' own mapping (sine table, integer pivot);
  - parallax floors: each row at its exact fractional start, resampled across (stretched "width" rows read
    from the sprite, not from the copies stretched nearest at load), rows mixed by coverage;
  - 1:1 sprites round to the nearest pixel like the rest;
  - speed: integer weights, the palette as four 16-bit lanes in one 64-bit word (one multiply-add per
    source pixel), source rows resampled once each; big draws repeated exactly (menus, a still camera) are
    kept (12 slots, keyed by sprite, pixels, palette colours, exact place and clip). About 1-2 ms a frame
    over nearest; frames run again for rollback don't draw;
  - `MOEMON_FIGHT_NEAREST=1` brings back MUGEN's nearest drawing (for comparing; it is pixel for pixel the
    old picture).
- Display (display.c RenderToBackbuffer): the fight's 480x320 is the game picture with srcW x srcH = 480 x 320
  instead of 240 x 160, through the same code: PIXEL PERFECT / SHARP FIT / STRETCH place the rect exactly as
  for the game (PIXEL PERFECT: the GBA screen's whole multiple, so the fight at half that: exact at even
  multiples, sharp-bilinear at odd), a whole multiple of 480x320 is copied nearest, anything else
  sharp-bilinear (nearest to the next whole multiple, then linear down); the filters' overlay is built per
  picture pixel (scanlines from 2 screen rows per pixel, the LCD grid from 3x3, as for the game). The bicubic
  path (Fight_CanvasBicubic) is gone.
- Spectating: friends get Fight_Frame, the 480x320 averaged 2x2 into a 240x160 GBA frame (an exact box
  filter), and show it through their own panel / pop-out scaling, as any game frame.
- Tests: `fight_display` (each mode, 4x / 3x / 1x, SHARP FIT, STRETCH, the grid at 3x, scanlines at 2x: the
  window against a reference rendering of the fightshot of the same frame, mean error under 1/255);
  tools/fight/test_softscale.c (even widths at 4/3 and 2/3, steady as a sprite moves by twelfths of a pixel,
  seamless tiles, transparency, subtract, parallax rows, rotation; built by build_linux.sh).

## GBA sound, and music under the voices (1.50.0)
- Answers: the files shipped in mugen/ are changed (a lower sample rate), and a stage's song is kept under
  the voices (gap and method left to judgement).
- `tools/fight/gba_audio.py` rewrote the shipped sound: every clip in the .snd banks is 8-bit mono PCM at
  13379 Hz (a clip recorded lower keeps its rate), every song Ogg Vorbis at 13379 Hz stereo (quality 3),
  with the .defs pointed at the new names and sample loop points scaled. 76 MB -> 51 MB. It changes only
  what isn't GBA sound yet, so a second run does nothing (`--check` exits 1 when something is left). The
  mixer still plays everything at 8 bits and 13379 Hz, so packs added later sound the same before and
  after it runs. The two endings' BGM lines named folders that don't exist (`chars/ssf2xchunli/...`) and
  now name the real ones.
- Music under the voices (mf_mixer.c MfMix_MusicGain): the song's level is its RMS over 0.4 s blocks with
  EBU R 128's two gates (-50 dBFS, then 10 dB under the gated mean; no weighting filter), from its first
  two minutes, decoded once when the song opens (mf_bgm.c MfBgm_Measure, remembered by path and size).
  A fighter's voice level is the 75th percentile of its sounds of 0.15 s or more (shorter ones are hits
  and steps), each measured over the 50 ms windows where it sounds (above -45 dBFS); the quieter of the
  match's fighters counts, at the volume sounds play at (master x WAV volume). The music is scaled so
  that at its volume (BGM volume, the stage's bgmvolume) it stays MF_BGM_UNDER_VOICES_DB = 3 dB under
  that; it is never raised. Outside a match (menus, select, VS) nothing is measured against and the
  music plays as it is.
- Why measured at run time, not written into the files: players drop their own songs into mugen/sound/
  (17 stage songs aren't shipped), and the cap has to hold for those too. The measurement runs on the
  music thread and isn't fight state. The engine thread logs the voices when the match loads (`Fighters'
  voices ... dBFS`) and each song's level and gain once the music thread has opened it, at the next frame's end
  (`Song level ... dBFS, played at x...`, mf_host.c MfHost_FrameEnd). Nothing is logged from the music thread,
  whose log calls raced the engine's, and the stage's song often opens only as the round starts.
- Shipped: the voices measure -10 to -5 dBFS (75th percentile), the two stage songs -7.4 and -8.9 dBFS:
  both were at or over the voices before.
- Test: `fight_music` also checks the two levels in mugen.log.

## Plan (as first agreed)

### The engine in the game
- Vendor `src/mugen/` from mugen-unleashed (the commit above), keeping its file-per-module layout
  and address comments. Additions are marked `Moemon Fight` like the other hooks.
- Port it to 64-bit, SDL2, gcc and mingw:
  - `MUGEN_ASSERT_SIZE` only checks on 32-bit MSVC (the layouts hold pointers);
  - SDL 1.2 video, events, joystick and SDL_mixer are replaced by the game's own;
  - no OpenGL or GLEW: the software renderer (`softrender.c` / `softblend.c`) draws into a
    480x320 buffer;
  - libpng comes from the game; Lua and minizip come with the engine;
  - storyboards, the title screen and the options screen are never reached.
- The engine runs as one stepped function, not its own loop. `src/mugen/mf_host.c` provides:
  - `Fight_Begin(cabinet, seat, seed)`, then `Fight_Step(const u16 inputs[2])` once per frame;
  - `Fight_Save` / `Fight_Load` / `Fight_Hash` for rollback;
  - `Fight_IsActive()` and `Fight_Canvas()`.

  The whole session (select, VS, fight, results) runs in this deterministic loop, so the select
  screen is synced the same way as the fight.
- (Until 1.50.0) `display.c` asks `Fight_CanvasBicubic()` first: the picture already scaled to the game's rect
  (third round).
- `Platform_GetFrame` gives spectators a 240x160 reduction while a fight is on.

### Screen fitting
- `mugen.cfg` sizes are forced to 480x320.
- Packs keep their `localcoord`. The scale is 320 / localcoord height, and the camera width is
  480 / scale. A 320x240 stage gets about 20 more stage pixels on each side.
- Screenpacks (motifs) are fitted by height too (`MfHost_FitMotif`: the smaller of the two scales)
  and centred (`g_mfMotifOffsetX`, `MF_MOTIF_X()` in the select, VS, fight HUD and explod code). The
  select, VS and results screens keep to the pack's 4:3: what slides past its edges isn't shown. The
  sides beside them show the screen's background (asked for in the fourth round, instead of black): it is
  kept as drawn before anything else of the screen (`MfHost_KeepBackground`, gameloop.c) and put in the
  sides once the screen is drawn and before the fade (`MfHost_FillSides`). Where the background really
  reaches into a side (mostly lit) it stays as it is; else each row carries on the background's colour at
  the edge (4 columns and 5 rows averaged, so text drawn as background doesn't streak), a fifth darker at
  the picture's edge. Mirroring was tried and dropped: SFEX draws text and the grid as background layers,
  which came out backwards.
- The fight HUD is measured sprite by sprite when a session starts (`MfFit_Hud`,
  fight/host/mf_fit.c), and what moved goes to `mugen/fightfit.log`:
  - each player's part (life bar, power bar, face, name, win icons) moves out to its real edge, as far
    as the pack had it from its own screen's edge, never past the 8px safe margin;
  - the top part goes to `hud.top` in cabinets.ini (8: against the safe margin; -1: the pack's place,
    moved inside the margin when past it). The owner found SFEX's bars hanging too low otherwise;
  - the bottom part only moves when it is past the margin;
  - a sprite across the middle of the screen belongs to both sides and keeps its centred place
    (SFEX draws both players' round boxes as one strip under P2's bar; it went to the right with P2's
    side at first).
- The camera never shows past a stage's edge. When the bounds plus the wider view need more than
  the stage has, the outermost background layers repeat at the edges.
- Fonts aren't measured yet (names, counters): they move with their side and part.

### Rollback and determinism
- All state a match changes lives in one arena: players, helpers, explods, projectiles, the
  camera, background controllers, the round state, the match RNG, input and command buffers.
- What's loaded (sprites, animations, states, sounds) is read-only after load and is not saved.
- A ring of 9 snapshots holds the arena plus the engine's globals. The other player's missing
  input is predicted as their last input. When the real input differs, the game loads the
  snapshot and replays up to 8 frames.
- Sounds are keyed by (frame, sound, channel), so a replayed frame never plays them twice. A sound
  the corrected history no longer has is cut.
- Both builds use the same floating point behavior (SSE2, `-ffp-contract=off`, no fast-math) and
  the same trig / pow / exp / log code bundled in `src/mugen/` (fdlibm-style), so every player's
  build gives the same results.
- A hash of the arena is exchanged every 30 frames. On a mismatch the host sends its snapshot
  (zlib), and the other player loads it.

### Network
- New room messages:
  - a seat taken or freed: the bubble for everyone;
  - the challenge and its answer;
  - the UDP endpoint exchange (hole punching, the port mapping from `mp_portmap.c`);
  - the session seed, the pack hashes, and the result.
- `MP_PROTOCOL_VERSION` goes up. The wire state gets an "in a fight" bit for the cabinet bubble.
- Inputs travel over UDP. Each packet holds the last 8 frames' buttons (16 bits a frame) and the
  last frame received, so lost packets need no resend. That's about 30 bytes 60 times a second.
- If no UDP packet gets through within 2 s, the same packets go over a new relay extension on the
  existing TCP connection.
- The delay is set by pings on the select screen: delay = clamp(ceil(RTT / 2 / 16.7 ms), 1, 3).
- Time sync: the side that runs ahead skips a frame now and then, so both stay within the
  rollback window. More than 8 frames apart, the game waits. With 10 s of silence comes the
  "Waiting for NAME" countdown and then no contest.
- Pack check: before select, each player sends the hashes of their characters and stages (every
  file each one uses). Anything not identical on both sides is greyed out and excluded from
  random.

### Sound
- Decoders (public-domain single files) live in `src/mugen/`: stb_vorbis, dr_mp3, dr_flac, and
  MUGEN's own `wav.c`.
- At load, every sound and song becomes 8-bit signed at 13379 Hz: sound effects mono, music
  stereo. Loop points (`bgm.loopstart` / `loopend`) are kept.
- A fight mixer adds them into the game's audio callback (42048 Hz out) while the game's music
  waits.

### Content and settings
- `mugen/` next to the exe uses the standard layout: `data/` (system.def, select.def, fight.def,
  mugen.cfg), `chars/`, `stages/`, `sound/` and `font/`.
- Files are read again each time someone sits down.
- `mugen/cabinets.ini` holds one section per cabinet (`[mauville_gc]`): motif, select.def, life,
  round time, game speed, rounds to win (2) and the stage list. Online, the host's values are
  used.
- Releases ship the pack: `mugen/` in the repository is the folder as it goes next to the exe.
  `.gitattributes` keeps it byte for byte.

### Cabinet and flow
- The map:
  - the slot machine tiles at (7,9) and (8,9) stay as they are;
  - their two `bg_event`s run `MauvilleCity_GameCorner_EventScript_VsCabinetWest / East`,
    which call `MP_Fight_Sit(seat)` in `src/mp/mp_fight.c`.
- Flow:
  1. The player sits (the facing is locked) and chooses WAIT FOR 2P or FIGHT CPU. Waiting shows
     the "Fight me!" balloon; FIGHT CPU opens the select screen against the CPU, with the
     balloon still up.
  2. B stands up (while waiting).
  3. A at the free seat asks "Fight NAME? YES / NO".
  4. Connect (2 s of UDP, then the relay). A CPU fight in progress is cancelled and both players
     get the select screen.
  5. The CRT turns on, then the pack's select screen, then the VS screen with the random stage.
  6. The fight: first to 2 rounds.
  7. The results with the win quote, then the CRT turns off.
  8. Both players stand beside their seats facing the cabinet, and the W / L count is recorded.
- Controls: a FIGHT row on OPTIONS > CONTROLS (the arcade layout above), saved with the game's
  other bindings.

### Tests (headless harness)
- `fight_cabinet`: two clients sit, pick and fight with scripted inputs, and are put back.
- `fight_rollback`: the same fight at lag (`tools/mp_server/lagproxy.py`, `WORST=1`), with
  identical hashes on both sides.
- `fight_determinism`: a recorded input file replayed twice, and under Wine, gives the same hash
  every frame.
- `fight_cpu_join`: one client fights the CPU, a second sits down, and both reach the select
  screen.
- `fight_fit`: a deliberately oversized screenpack is nudged inside, checked against the log.
- `fight_sound`: decoded sounds come out at 8-bit, 13379 Hz.

### Order of work
1. Port the engine and make it a stepped function; offline fights on the canvas.
2. Screen fitting and sound.
3. Rollback state and determinism (local tests with simulated delay).
4. Networking, the cabinet and the flow.
5. Spectating, the controls row, and wins / losses.

