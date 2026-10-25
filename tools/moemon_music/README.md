# Moemon Music Studio

A desktop program (Linux / Windows, SDL2) that plays MIDI files with Moemon Emerald Kai's own
instruments **and its own sound engine**, lets you pick and tune the instruments, and exports the
song in a form the game can use.

- **Plays like the game.** `src/engine.c` is the game's MP2K engine (`src/music_player.c`,
  `src/sound_mixer.c`, `src/m4a.c`, `src/platform/cgb_audio.c`) ported line for line, with the
  game's 5 sample channels, 4 Game Boy channels, 10 tracks and reverb. On 15 game songs the
  audio comes out sample for sample identical to the game's (the game's mixer reads one byte past
  a sample's end, which is zero in the game; `gen_soundfont.py` pads the samples to match).
- **Converts like the game.** `src/convert.c` is `tools/mid2agb`'s pipeline. All 529 game MIDIs
  give the same bytes as `mid2agb -E -N` (`make test`).
- **General MIDI in, game instruments out.** Each MIDI program starts as the closest of the
  game's 775 instruments (`GM_MAP` in `gen_soundfont.py`); channel 10 gets a game drum kit. Optional
  GM fixes: expression (CC11), RPN bend range, missing program changes, `loopStart` / `loopEnd`
  markers and CC111, notes cut at the loop end, track state re-sent at the loop start.
- **Projects.** `.mmsproj` is a text file holding every setting and instrument choice, with the
  MIDI inside, so it opens anywhere.
- **Exports.** `.kaisong` (one file: song, voicegroup, sample names; layout and loader plan in
  `docs/KAISONG_FORMAT.md`) and decomp sources (`<song>.s` + voicegroup `.inc` in the game's own
  format). The game doesn't load `.kaisong` files yet.

The game's own MIDIs (`sound/songs/midi/mus_*.mid`) open with their real voicegroup, volume,
reverb and priority from `midi.cfg`.

## Building

```
cd tools/moemon_music
make                        # Linux: build/linux/moemon_music_studio (libsdl2-dev, python3)
make windows SDL2_DIR=...   # build/windows/MoemonMusicStudio.exe + SDL2.dll (x86_64-w64-mingw32-gcc,
                            # SDL2 mingw development package; default ../../SDL2)
make test                   # converter vs mid2agb on every game MIDI, exports, round trips
```

The instruments come from the game's `sound` folder at build time: `gen_soundfont.py` reads every
voicegroup, drum kit and key-split table, converts the samples with `tools/wav2agb` exactly as the
game build does, and writes `build/gen/` (the program embeds it; nothing else ships with the exe).
`src/font.h` and `src/fonts.bin` are checked in (`gen_font.py` remakes them from DejaVu Sans Mono).

## Using it

Open a MIDI (or drop one on the window). TRACKS lists each MIDI channel: **M** leaves it out of
the song, **S** plays it alone. INSTRUMENTS lists the voice slots the song uses: pick one, then
**Change...** (every game instrument, by category, with search; click to hear), **GM match**,
**Hold to hear** or keys Z-M, and its envelope. SONG sets the game song it replaces, volume,
reverb, priority, the loop (MIDI markers, none, whole song, custom bars), and can take every
instrument from a game song's voicegroup. Past the game's limits: **Go over 10 tracks** plays and
exports up to 32 tracks, and **Voices** mixes up to 12 samples at once (the game: 5; extra tracks
only sound when voices are free for them). The game plays neither yet. Then **Save** and **Export .kaisong** / **Export decomp**.
**Aa** (top right) picks the font (Pixel, or Smooth / Bold, which stay sharp at any window size)
and the window size (fit the screen, 100-200%); both are remembered in the user's settings folder.

Command line (also what the tests use):

```
moemon_music_studio [FILE]                          the window (FILE: .mid or .mmsproj)
moemon_music_studio --render IN OUT.wav [SECONDS]   to the loop end, or SECONDS
moemon_music_studio --export IN OUT.kaisong
moemon_music_studio --decomp IN DIR
moemon_music_studio --save IN OUT.mmsproj [SONG]
moemon_music_studio --dump IN                       song bytecode, a track a line
moemon_music_studio --dump-exact IN VOLUME          converted like mid2agb -E -V VOLUME
moemon_music_studio --raw IN OUT.f32 FRAMES         the engine's float output, frame by frame
```

## Files

| file | what |
|------|------|
| `gen_soundfont.py` | game sound folder -> `build/gen/sf_data.c`, `sf_counts.h`, `sf_pcm.bin`; presets; the GM map |
| `src/engine.c` | the game's music engine |
| `src/midi.c`, `src/convert.c` | MIDI reader; MIDI -> song bytecode (mid2agb) |
| `src/inst.c` | game voices -> engine instruments, names, the project file's voice text |
| `src/project.c` | projects (`.mmsproj`) and loading MIDIs |
| `src/export.c` | `.kaisong` and decomp sources |
| `src/studio.c`, `src/cli.c`, `src/ui.c`, `src/dialog.c` | the open song, command line, window, file dialogs |
| `test/` | `run_tests.sh`, `check_mid2agb.py`, `read_kaisong.py` (reference reader), `make_test_midi.py` |
