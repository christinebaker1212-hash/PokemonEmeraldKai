# The DANCE machine (StepMania-style rhythm game): decisions and plan

Asked for by the project owner (October 2026, docs/user_images 43-44). Sources they named:
StepMania (https://github.com/stepmania/stepmania), the "StepMania as DDR EXTREME" guide
(https://gist.github.com/d3v1an7/3d048a4ad33ce397eb221e4bb0470a34: Beware's StepMania EXTREME build, Inorizushi's
DDR-EXTREME-JP-SM5 theme, the katzeextreme_beware announcer) and the DDR EXTREME simfiles on Zenius-I-vanisher
(category 41).

## Asked for directly
- A rhythm game, StepMania with DANCE DANCE REVOLUTION EXTREME's look and announcer, as a new machine on the GAME
  CENTER's 3F (the picture 43 replaces the floor's old one; 44 marks blue = over the player, red = walkable where it
  isn't obvious; red is only the exceptions).
- Fitted to the GBA screen the way the VS cabinet fits its packs; drawn inside the game's own picture, so the
  window's scaling, pixel grid and SPECTATE carry on as for anything else in the game.
- Arcade mode only: one credit is 4 songs, unless a song is failed early.
- Two players: wait for someone to join, or play alone.
- Scores on each song are saved server-wide.
- Achievements of its own, for DDR experts.
- Steps from both the D-pad and the face buttons, both acting as the arrows.

## Decisions (taken while building; the owner can overrule any)
| # | Question | Decision |
|---|---|---|
| 1 | StepMania itself, or a game that plays StepMania's files | A rhythm engine of our own in C (`src/rhythm/`) that reads StepMania's simfiles (.sm, .ssc, .dwi not yet) and follows Beware's EXTREME build for the rules: StepMania 5 is C++ with Lua themes, OpenGL and ffmpeg and draws at 640x480; shrunk to 240x160 its text can't be read, and it can't draw into the game's picture |
| 2 | Judgment | Beware's EXTREME build: MARVELOUS 16.7 ms, PERFECT 33.3, GREAT 83, GOOD 120, BOO 158.3; a freeze arrow is OK when it's let go of for less than 250 ms. As on the arcade, MARVELOUS is counted but shown as PERFECT unless the player turns on MARVELOUS on the options screen |
| 3 | Score, grade, life | DDR MAX2 / EXTREME scoring (feet x 10,000,000, each step p * (B / S) * n, the leftover bonus on a perfect last step); grades by dance points AAA (all PERFECT or better) / AA 93% / A 80% / B 65% / C 45% / D, E = failed; the life bar starts half full: +0.8% MARVELOUS / PERFECT / OK, +0.4% GREAT, -4% BOO, -8% MISS / NG, -10% while full, merciful drain, 5 steps to regain life after a miss (10 after an empty bar) |
| 4 | A credit | 4 stages. Failing a stage (the bar empties) ends the credit after its results, as on the arcade; the 4th is the FINAL STAGE. No extra stage |
| 5 | Screens | CAUTION, SELECT STYLE (1 PLAYER / VERSUS when a friend is on the other pad), SELECT DIFFICULTY (BEGINNER, BASIC, DIFFICULT, EXPERT, CHALLENGE as each song has them), SELECT MUSIC (a wheel by folder, then A-Z / BPM / difficulty sorts), OPTIONS (Start held on a song: speed x1-x8, BOOST, HIDDEN, SUDDEN, STEALTH, DARK, MIRROR / LEFT / RIGHT / SHUFFLE, LITTLE, REVERSE, FREEZE ARROW OFF, arrow colors VIVID / NOTE / FLAT / SOLO, JUDGMENT ARCADE / MARVELOUS, TIMING), STAGE n, READY / HERE WE GO, the song, CLEARED / FAILED, RESULTS, GAME OVER |
| 6 | Look | 240x160, laid out for the screen (not a 640x480 picture shrunk): the theme's and the note skins' art cut down at a third of its size with an area filter (the VS cabinet's scale by height), text in the game's small fonts. Both fields at once in VERSUS. The song's background picture behind the field (no movies) |
| 7 | Sound | The songs at full quality (not GBA sound: a rhythm game needs them clean), resampled to the game's 42048 Hz and mixed into its sound instead of the GBA's. The announcer from katzeextreme_beware at its cues (stage, READY, combos, the life bar's state, grades, game over) |
| 8 | Timing | The song's clock is the sound card's: the samples played so far, smoothed against the PC's clock; each step is timed by the moment its key or button event arrived (SDL's event time), not the frame. TIMING on the machine's OPTIONS screen (5 ms steps, kept with the player's options) judges the steps later or earlier for a PC's own sound delay; RESULTS shows the average offset and the FAST / SLOW counts to set it by |
| 9 | Buttons | Arrows: D-pad, left stick, the face buttons by position (west LEFT, south DOWN, north UP, east RIGHT); keys: arrows, WASD and IJKL. Menus: LEFT / RIGHT (or UP / DOWN) and A / START, B back. Hold START 1 s in a song to give up (it counts as failed) |
| 10 | The songs | Shipped (the owner: the game is only shared between friends and family). Beware's EXTREME songs, all 240 arcade songs (simfiles, music, banners; no movies), are in the repository as StepMania lays them out: `stepmania/Songs/<group>/<song>/`. So that every update doesn't carry ~400 MB, they aren't in the game's release zip: the release workflow publishes them as their own zip (`MoemonDanceSongs.zip`, with its size and SHA-256 in update.txt), and the game downloads it the first time someone steps up to the machine, and again only when it changes. Any StepMania song folder dropped into `stepmania/Songs/` works too |
| 11 | Art and announcer | The art and voice lines converted into `graphics/rhythm/` by `tools/rhythm/` (like the other screens' art) and built into the game |
| 12 | The pads | Cells (0-2, 4-6) are the machine. A player steps up from the row below it (0-2, 7) and is drawn on the left pad (1P) or the right pad (2P) while playing, facing the arrows they step on; friends see it too |
| 13 | Two players | The first player on the machine is 1P. WAIT FOR 2P shows "Let's dance!" over them; a friend stepping up joins and VERSUS starts. PLAY ALONE starts at once; a friend stepping up during a solo credit is asked to wait for the next one. 1P picks the songs, each player their own difficulty and options |
| 14 | Online play | Each game plays its own chart on its own clock (no rollback: neither player changes what the other has to step). Both start a song together: 1P sends GO with a start time some frames ahead; each sends its judgments, score, combo and life ~10 times a second, and the other field shows them (its arrows disappear as they're judged). Results are swapped at the end. A friend who drops out leaves 1P playing alone |
| 15 | Scores | The room server keeps the top 10 per chart for everyone on that server (`rhythm_scores.dat`, keyed by the chart's notes' hash, so the same song in another folder shares records), whatever the room. The machine shows the server record on SELECT MUSIC, and NEW RECORD on the results. Playing alone offline keeps them in `<save>.dance` and sends the best when next online |
| 16 | Achievements | For experts: full combos (GREAT or better), PERFECT full combos, MARVELOUS full combos, AAA on 9- and 10-footers, MAX 300, PARANOiA survivor MAX, every EXTREME boss, 4 stages all full combo, all 240 songs cleared, a 1000 combo, versus wins, and a few lighter ones to start with |
| 17 | Spectating | Inherited: the SPECTATE panel streams the game's picture, which is the machine's screen while playing |
| 18 | Coins | Free play (the GAME CENTER's COINS can come later) |

## Where things are
- `src/rhythm/`: rh_chart.c (the simfile reader, timing), rh_play.c (judgment, score, life), rh_screen.c (the
  screens), rh_draw.c (drawing into the 240x160 picture), rh_audio.c (songs and voices), rh_net.c (two players),
  rh_songs.c (the song list, the download). API `include/rhythm.h`.
- `src/mp/mp_rhythm.c`: the machine on the map (who's on which pad, the bubbles, the specials).
- `data/scripts/dance_machine.inc`, the 3F map's events.

## As built (1.39.0)
- Song keys between two games and for "songs cleared" are the title, subtitle, artist and folder: about 30 of
  EXTREME's songs share a title with another (the mix is the subtitle), and the subtitle is shown with the title.
- Online, a stage counts (achievements, VERSUS wins) when both have left its RESULTS, so the friend's final score
  is in; a tie is no win.
- New songs are offered on the machine only when dancing alone (a friend waiting at the other pad isn't kept
  waiting); with no songs at all the machine always offers them.
- The release workflow only uploads the songs zip when the songs changed (tools/release/package_songs.sh's id, a
  hash of every file); update.txt keeps pointing at the release that has it.
