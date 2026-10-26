# Moemon Music Studio

A desktop program (Linux / Windows, SDL2) that plays MIDI files, and music ripped from other games
(PSF, PSF2, USF, GBS, 2SF, GSF, GameCube ISH/ISD/DSP), with Moemon Emerald Kai's own instruments **and its own sound
engine**, lets you pick and tune the instruments, and exports the song in a form the game can use.

## Music from other games

| format | console | played by (`third_party/`) | how the notes are read |
|---|---|---|---|
| PSF, minipsf | PlayStation | Highly Experimental | the SPU's 24 voices: key-ons, pitch, envelope and volume; samples decoded from sound RAM |
| PSF2, minipsf2 | PlayStation 2 | Highly Experimental | the same for both SPU2 cores (48 voices) |
| USF, miniusf | Nintendo 64 | lazyusf2 | the audio microcode's command lists (libultra n_audio, EAD): codebook, decode, pitch, envelope; samples found from the driver's wavetables and decoded (VADPCM) from the ROM |
| GBS (+ .m3u) | Game Boy | Game_Music_Emu | the four channels' registers; they become the game's own Game Boy voices (the GBA has the same channels) |
| 2SF, mini2sf | Nintendo DS | vio2sf's DeSmuME | the 16 SPU channels: sample (PCM8/16, IMA-ADPCM), timer, volume, pan; PSG squares and noise |
| GSF, minigsf | Game Boy Advance | VBA-M (in_gsf) | games on MP2K (the game's own engine): the song and voicegroup read from the ROM, unchanged |
| ISH + ISD, DSP | GameCube (Pokemon Colosseum, XD) | decoded here (DSP-ADPCM) | streamed recordings: no notes to read, so the song is the recording (see below) |

GameCube music in Colosseum and XD is streamed: a recording, not notes. Its song is the recording
itself, an intro sample and a loop sample per stereo side at the GBA's rate, each played by one long
note. The loop is stretched by a hair (well under 0.1%) to last a whole number of game frames, so the
song loops forever in the game like any other. A `.dsp` that is only a 192-byte `I_SF` header has no
sound in it (its data file is missing from the rip).

PlayStation music needs a PlayStation 2 BIOS (4 MB, e.g. `scph39001.bin`, from your own console):
**Aa > BIOS folder**, or put it next to the music. The studio makes the small sound BIOS it needs from
it each time (mkhebios); no BIOS is in this repository.

Opening a rip plays it through its console's emulator once (a few seconds), writing down what the
sound chip does every game frame. That becomes a MIDI (tracks by instrument, each keeping one
instrument: a chip hands its voices to whatever instrument needs one, so a track per chip voice would
change instrument all the time; one tick per game frame, with note starts moved onto the song's own
beat grid when it has a steady one, since a console sequencing on its own clock lands notes up to a
frame off: the main grid first, then triplets and halves the song really uses, down to a 1/64 note,
so each note starts on the frame nearest its exact musical position; each note on the closest key, a note that jumps to a new pitch without a new key-on split
in two, and pitch bends for the rest with a bend range per note just wide enough, so notes stay in
tune at the game's 7 bits of bend; its volume each frame as expression, pan) and **the original instruments**: each sample, at
about the GBA's 13379 Hz as you hear it (a sample usually played low keeps more detail) and 8-bit like
the game's, with its loop. The loop is found from where the key-ons start repeating. The whole song
is then matched to the original's loudness (Game Boy channels keep the hardware's own levels).
GBA songs on MP2K need none of that: their own commands and samples come straight from the ROM.

- **Kai / Original** (next to Play): hear the song in the game's engine, or the console's own sound
  from its emulator, at the same place.
- **All original / All GM** (SONG): each original instrument, or the closest General MIDI instrument
  of the game (compared by harmonic make-up, whether notes hold or die away, and the instrument's
  usual range; drums by brightness and length). Instruments with no close GM match keep the
  original sample. Per instrument: **Orig** / **GM**, or **Change...** as usual.
- **Game Boy / Convert GB > GBA** (SONG, Game Boy songs): the channels as the GBA's own Game Boy
  channels, or each one as the GBA instrument it sounds like. Squares (by duty) and the wave channel
  (its 32 steps) are built from their harmonics and matched like samples, with how the song holds
  them and where they play (the wave channel is usually the bass). The noise channel becomes a drum
  kit: a Game Boy driver plays each drum as the same few frames of noise settings and volumes every
  hit, so hits are grouped by how they start, each group's part is read the way an arranger hears it
  (a short low rumble is a kick, a mid hiss a snare, a short bright fizz a closed hi-hat, a longer one
  an open hi-hat, a long one a cymbal), and of the game's drums for that part the one that sounds
  closest to the hit (made again from the noise generator) is picked. Hits play as drum hits: kicks,
  snares, toms and cymbals ring out their sample, hi-hats stop at the next hit, each at the
  original's level (matched over the 150 ms the ear takes a drum's loudness from).
  Levels are matched in the engine, panning included, then brought down together so none goes past
  a volume of 127 (Game Boy channels play louder than samples). Per channel: **Orig** / **GBA**.
- Exports carry the original samples: `.kaisong` version 2 (`SMPL` / `WAVE` chunks), decomp WAVs.

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
  markers and CC111, notes cut at the loop end, track state re-sent at the loop start, drum notes a
  kit has no sound for moved to its closest sound (same kind of drum first, then the nearest note),
  and instrument volumes evened out: each instrument's loudness is measured in the engine against
  the game's piano and the factor goes into the track's VOL commands (*Even out instrument volumes*).
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
make                        # Linux: build/linux/moemon_music_studio (libsdl2-dev, python3, g++)
make windows SDL2_DIR=...   # build/windows/MoemonMusicStudio.exe + SDL2.dll + libwinpthread-1.dll
                            # (x86_64-w64-mingw32-gcc and -g++, SDL2 mingw development package; default ../../SDL2)
make test                   # converter vs mid2agb on every game MIDI, exports, round trips
```

The instruments come from the game's `sound` folder at build time: `gen_soundfont.py` reads every
voicegroup, drum kit and key-split table, converts the samples with `tools/wav2agb` exactly as the
game build does, and writes `build/gen/` (the program embeds it; nothing else ships with the exe).
`src/font.h` and `src/fonts.bin` are checked in (`gen_font.py` remakes them from DejaVu Sans Mono).

## Using it

Open a MIDI (or drop one on the window). TRACKS lists each MIDI channel: **M** leaves it out of
the song, **S** plays it alone, **...** shows its instrument changes ("+2" = two changes): lock the
track to **one instrument** all through, or point any single change at another instrument, or **New**
for a copy of it in a free slot to change on its own. Saved in the project and in the exports. INSTRUMENTS lists the voice slots the song uses: pick one, then
**Change...** (every game instrument, by category, with search; click to hear), **GM match**,
**Hold to hear** or keys Z-M, its envelope and its **Octave** (every note it plays moves by whole
octaves, in the exports too). SONG sets the game song it replaces, volume,
reverb, priority, the loop (MIDI markers, none, whole song, custom bars), and can take every
instrument from a game song's voicegroup. **GBA accurate** (off unless turned on) plays and exports the way the game
would: 10 tracks, 5 sample voices, the rest of a busy moment cut off. Off, the song plays and exports
as it is: up to 64 tracks and 64 sample voices, so nothing is cut (the 4 Game Boy channels still play one note each); the game
can't play such songs until its music player gets more tracks and voices. **Drums first** (a GM fix)
gives drum tracks priority 1 so their hits win voices in GBA accurate mode. Then **Save** and **Export .kaisong** / **Export decomp**.
**Aa** (top right) picks the font (Pixel, or Smooth / Bold, which stay sharp at any window size)
and the window size (fit the screen, 100-200%); both are remembered in the user's settings folder.
**Defaults** (top right): settings every MIDI and rip you open starts with: GBA accurate, volume,
reverb, even out volumes, drums first, rips' original or GM instruments, Convert GB > GBA, sample
instruments' attack / decay / sustain / release and an octave for every instrument but drums.
Each can stay "the song's". Saved projects keep their own; **Apply to this song** uses them now.
They live in `defaults.ini` in the settings folder (the command line reads one from `MMS_DEFAULTS`).

Command line (also what the tests use):

```
moemon_music_studio [FILE]                          the window (FILE: .mid, .mmsproj or a rip)
moemon_music_studio --original RIP OUT.wav           the rip's own sound (its emulator); MMS_BIOS = BIOS folder
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
| `src/rip.c`, `src/rip_*.c(pp)` | rips: loading, each console's emulator and capture, the MIDI and instruments made from it (`rip_song.c`), MP2K from the ROM (`rip_mp2k.c`), GM matching (`rip_gm.c`) |
| `third_party/` | the emulators, psflib, zlib (`third_party/README.md`: sources and licenses; the studio as a whole is GPL 2+) |
| `test/` | `run_tests.sh`, `check_mid2agb.py`, `read_kaisong.py` (reference reader), `make_test_midi.py` |
