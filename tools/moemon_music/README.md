# Moemon Music Studio

A desktop program (Linux / Windows, SDL2) that plays MIDI files, and music ripped from other games
(PSF, PSF2, USF, GBS, NSF, 2SF, GSF, SPC, VGM, GameCube ISH/ISD/DSP, a DS game's own SDAT), with Moemon Emerald Kai's own instruments **and its own sound
engine**, lets you pick and tune the instruments, and exports the song in a form the game can use. **CHIPTUNE** makes
any of them NES or Game Boy music (an `.nsf` / `.gbs` that any player plays, and the studio opens again).

## Music from other games

| format | console | played by (`third_party/`) | how the notes are read |
|---|---|---|---|
| PSF, minipsf | PlayStation | Highly Experimental | the SPU's 24 voices: key-ons, pitch, envelope and volume; samples decoded from sound RAM |
| PSF2, minipsf2 | PlayStation 2 | Highly Experimental | the same for both SPU2 cores (48 voices) |
| USF, miniusf | Nintendo 64 | lazyusf2 | the audio microcode's command lists (libultra's ABI 1 "audio" as in Bomberman Hero, n_audio, EAD), sub-frame by sub-frame (libultra mixes each buffer in 184-sample parts, every voice in each): codebook, decode, pitch, envelope, each voice by its own mixer state (a voice whose sample has ended still mixes, on silence); samples found from the driver's wavetables and decoded (VADPCM) from the ROM |
| GBS (+ .m3u) | Game Boy | Game_Music_Emu | the four channels' registers; they become the game's own Game Boy voices (the GBA has the same channels) |
| NSF, NSFe (+ .m3u) | NES / Famicom | Game_Music_Emu (2A03 and the cartridges' chips) | the 2A03's registers: its squares (the Game Boy's four duties) and triangle (32 steps of 4 bits, the Game Boy's wave channel playing a triangle) become the game's Game Boy voices, the noise its noise, the DMC's 1-bit samples decoded from memory as samples; the cartridges' chips from their registers: VRC6 (16-step pulses, the sawtooth), Namco 163 (each channel's wave from its RAM), Sunsoft 5B (squares, 3 dB volume steps), the Famicom Disk System (its 64-step wave), MMC5 (two more squares), VRC7 (a pure tone per FM patch: notes, pitch and volume exact, the patch's sound not) |
| 2SF, mini2sf | Nintendo DS | vio2sf's DeSmuME | the 16 SPU channels: sample (PCM8/16, IMA-ADPCM), timer, volume, pan; PSG squares and noise |
| SDAT (`file.sdat#SEQ_NAME` or `#NUMBER`) | Nintendo DS | played here (`src/rip_sdat.c`): no emulator, no 2SF | the archive's own sequence (SSEQ) run on a sequencer written after the DS's sound library (its 192 Hz update, tempo / 240 ticks, notes, loops, calls, variables and random, ties, portamento, pitch bend, modulation), its bank's instruments (SBNK: PCM waves, PSG squares, noise, drum sets, key splits; ADSR in decibels) and waves (SWAR: PCM8/16, IMA-ADPCM), on 16 channels reported like a 2SF's (test/make_test_sdat.py). In the window, opening an `.sdat` asks for the sequence's number; its length is the intro and the loop twice |
| SPC | Super Nintendo | Game_Music_Emu (SPC700 + S-DSP) | the S-DSP's 8 voices: each key-on, the sample its SRCN names in the directory (start, loop), pitch, envelope and volume, echo; samples decoded from the sound RAM as the DSP decodes them (BRR) |
| VGM, VGZ | Sega Genesis / Mega Drive | Game_Music_Emu (Nuked-OPN2 YM2612, SN76489) | the log of register writes, read directly: the YM2612's 6 FM channels (key-ons, frequency, the 4-operator patch: each patch becomes an instrument per octave it's played in, the patch alone played by Nuked-OPN2 in that octave and looped over whole periods, so its envelope runs at the speed it does there; the carriers' total levels as the volume, a key-off fading at their release rate), the DAC's output followed write by write (from the PCM data bank or written directly) and cut into drum hits at each seek, DAC switch-on or pause, the SN76489's squares and noise (Sega's 16-bit shift register) as samples; each part at the level the emulator plays it at (test/make_test_vgm.py) |
| GSF, minigsf | Game Boy Advance | VBA-M (in_gsf) | games on MP2K (the game's own engine): the song and voicegroup read from the ROM, unchanged. Games on Konami's driver (Yu-Gi-Oh! Worldwide Edition...: found by its mixer): its six sample channels read from the driver as it plays (sample, 4.12 step at timer 0's rate, volume 0-15, placed a mixing buffer later, as heard), samples from its two tables (signed 8-bit, loop start), and the hardware's Game Boy channels (squares, the wave bank heard or both as 64 steps, noise) |
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
the game's, with its loop. The loop is found in the original sound: each game frame's spectrum, the period at which the song repeats (the notes' own repeat is tried too), the earliest frame from which it keeps repeating, and the period to the sample by matching the waveform one period apart (a loop a frame off would be heard every time it loops); the loop starts at the first note from there. A song that doesn't clearly repeat gets no loop. **custom** sets a rip's loop by time, to the game frame (-1s / -f / +f / +1s, the mouse wheel, or **here**: where the song is playing), and **Hear the loop** plays the last 3 seconds and the jump. The whole song
is then matched to the original's loudness (Game Boy channels keep the hardware's own levels).
GBA songs on MP2K need none of that: their own commands and samples come straight from the ROM.

- **GBA / Original** (next to Play, or Tab): hear the song in the game's engine, or the console's own sound
  from its emulator, at the same place.
- **All original / All GM** (SONG): each original instrument, or the closest General MIDI instrument
  of the game (compared by harmonic make-up, whether notes hold or die away, and the instrument's
  usual range; drums by brightness and length). Instruments with no close GM match keep the
  original sample. Per instrument: **Orig** / **GM**, or **Change...** as usual.
- **Game Boy / Convert GB > GBA** (SONG, Game Boy songs; **NES / Convert NES > GBA** for NES songs, whose squares,
  triangle and noise are the same kind of channels): the channels as the GBA's own Game Boy
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
  A square or wave channel played by a GBA instrument gets notes made for an instrument: the chip's
  arpeggio (a chord played one note a frame) is the chord, held; a note retriggered every frame or two
  (a buzz) is one held note; a duty switched mid-note (Gold / Silver / Crystal cycle it every frame or so) is
  the same note, on the duty the channel plays most. Vibrato and slides stay pitch bends, as on the chip.
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
- **Exports.** `.kaisong` (one file: song, voicegroup, sample names; layout and how the game loads it in
  `docs/KAISONG_FORMAT.md`) and decomp sources (`<song>.s` + voicegroup `.inc` in the game's own
  format). The game plays `.kaisong` files from its `music` folder, exactly as the studio plays them (both checked
  sample for sample by `tools/test/kaisong_test.sh`), with each file's own settings: tracks, voices, reverb, priority.

The game's own MIDIs (`sound/songs/midi/mus_*.mid`) open with their real voicegroup, volume,
reverb and priority from `midi.cfg`.

## CHIPTUNE: any song as NES or Game Boy music

**Chiptune...** (top bar, CONVERT) makes the open song, whatever it was opened from (a MIDI, a project, every kind of
rip above but GameCube recordings), into music for the NES's 2A03 or the Game Boy's sound chip, and writes it as that console's own file: an
`.nsf` or a `.gbs`, with a small driver that plays it (`gen_chip_drivers.py`), so it plays in any NSF / GBS player and on
the consoles' flash carts. **Open it afterwards** (on by default) opens the file in the studio like any NES / Game Boy
song: **Original** plays it on the chip's own emulator, **GBA** in the game's engine, and it can be exported as a
`.kaisong`, or made chiptune again for the other chip. Its length and loop are kept in the file.

How (`src/chip.c`): the song plays through the game's engine once and every voice is read each frame: which part of the
song it belongs to, its pitch with bends and vibrato, its loudness as heard, its instrument. Then it's arranged the way a
chip musician would, from how the notes behave and never from which instrument plays them, so any song
and any instrument works the same way:

- **What each part is.** Echoes (notes another part, or the same one, played a steady few frames before, as long and
  quieter); doublings (the same notes at the same moment as a stronger part, in unison or octaves: one note); the bass
  (the lowest steady part); the melody, chosen for each four seconds (weighing the stretches either side, and kept unless
  another part is clearly more of one): the part that moves from note to note over many pitches in a varied rhythm, loud,
  high and one note at a time. An ostinato repeating a note, a pad, a figure cycling a few notes or a pedal, and the lower
  of two parts moving together (a melody in 3rds, stabs) accompany.
- **One line a channel, a section at a time**, as a chip composer writes it. Square 1: the melody's line (when it rests,
  the same line on its doubling instrument, else it rests too; when it slides into a note on another instrument, the
  slide). Square 2: for each four seconds, ONE part that goes with the melody there (scored by how much it plays, how
  consonant it is with the melody: 3rds and 6ths best, the melody's own note again worst; the melody's harmony or echo
  first), kept until a section clearly calls for another; its chords play as an arpeggio, low to high (**Chords**: Auto /
  Hold / Arpeggio, which also prefers a part playing chords; frames a note); after a long rest, the section's second
  choice. Each channel rests when its line does. The bass on the triangle (NES) or the wave channel (Game Boy: the
  classic rounded-square bass wave, or a Game Boy song's own wave). Each part keeps one octave.
- **One register a line** (**Square 2 and the bass stay in one register**, on unless turned off): square 2 and the bass
  keep near where their line has been (a note further than a tritone, the bass a 5th, away plays in the octave near it:
  the same note, the same harmony), arpeggios within an octave; square 1 plays the melody's upper voice (a held note of
  its part with a higher one sounding over it is a lower voice), a flourish (a run of 8 quick notes or fewer from a held
  note and back, far from the line) an octave nearer as a whole, and the melody's doubling instrument, while the melody
  rests, in the melody's octave. Of two parts playing the melody, the one that keeps leaping octaves is the less of one.
  The melody's held notes keep their real leaps.
- **Chip instruments, not the original's levels.** Each note has the chip instrument's own envelope: a short note plucks
  (full, then down a step a frame), a long one punches and settles to a steady sustain, an instrument that dies away
  dies away; the original's level only sets each note's velocity. The melody near the top, square 2's line under it,
  arpeggios and echoes further down. Duty by role: a part's own duty (a Game Boy square keeps its; a sample is 25% when
  bright, 50% when mellow), square 2 the other of 25 / 50% when square 1 plays the same, arpeggios 12.5%, a note's first
  frame a step narrower; a delayed vibrato on the melody's long steady notes (**Vibrato**). On the Game Boy the hardware
  envelope plays these shapes, so a note isn't restarted to change its level.
- **Drums.** Each hit becomes a drum of a kit by its brightness and length (kick, snare, hi-hat, open hi-hat, crash),
  played as that drum is on the chip: noise frame by frame at the hit's strength; on the NES kicks and snares as DMC
  samples (**NES kicks and snares as DMC samples**: a falling sine and a tone with noise, in the file's last banks), the
  noise keeping the hi-hats.
- **The arrangement table** (the panel's PARTS): every part, what the studio reads it as, its notes and range, and what
  it plays as (Auto, or Melody / Harmony / Backing / Echo / Bass / Off), its duty and its octave, set by hand.
  **Read them again** reads the parts with the table's settings.
- Volume: NES each square's and the noise's volume register; Game Boy the hardware envelope that follows the next
  frames' volumes most closely. A note repeated on the same channel gets a
  frame's gap. On the NES a vibrato across a period's high bits stays at the edge instead of restarting the square.
- The NES plays at the game's own frame rate; the Game Boy at its vertical blank (59.73 Hz), the song read at that rate,
  so both keep the original's tempo. GameCube music is a recording, not notes, so it can't be made chiptune.

The files: an NSF has the driver in its first 4 KB bank and the song's register writes in the banks after it (up to
1 MB, NSF bank switching); a GBS has the driver at `$0400` and the writes from bank 1 on (MBC1 banks, up to 496 KB).
The stream: `00-3F rr vv` a register write, `81-FE` the frame's end and frames to wait, `80` the loop, `FF` the end
(the top of `gen_chip_drivers.py`).

## Releases

The studio has releases of its own, apart from the game's (tagged `mms-v<version>`, on the game repository's
Releases page). To publish one: raise `STUDIO_VERSION` in `src/project.h`, write `release_notes/<version>.md`
(first line `Moemon Music Studio <version> "<Title!>": <one line>`) and push to the default branch.
`.github/workflows/mms-release.yml` runs the tests, builds Linux and Windows, packages them (`package.sh`)
and publishes. A studio release is never marked the repository's latest release: the game's updater
downloads from the latest one, so the workflow publishes with `--latest=false` and then checks that a game
release (`v<version>`) still has the mark, putting it back and failing loudly if not. Changing the studio
never starts a game release (that only runs when the game's version changes).

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

Two tabs at the top left, remembered between runs:

- **LISTEN**: a player. NOW PLAYING shows the song (title, game, composer, console, its length and
  loop, **Hear the loop**) and the big **GBA / Original** switch (Tab: the same moment of the song in
  the other version; the choice carries on to the next song). The open song's folder is the PLAYLIST
  (every file the studio plays, a Game Boy file's songs one by one; **Open folder...**, **< Prev** /
  **Next >**, Page Up / Down). AFTER A SONG: **Repeat it**, **Next song** or **Stop** after 1-9 times
  through its loop (with a fade out), and **Shuffle**. **Convert this song >** goes on to CONVERT.
  The tracks' **M** / **S** work here too.
- **CONVERT**: making a song for the game. The steps under the buttons say how far it is and take you
  there when clicked: 1 the song, 2 its instruments (how many aren't the original's), 3 its loop (plays
  the jump), 4 where it plays (opens the picker), 5 whether it fits the game's player (tracks, voices),
  6 export the `.kaisong`.

Open a MIDI (or drop one on the window). TRACKS lists each MIDI channel: **M** leaves it out of
the song, **S** plays it alone, **...** shows its instrument changes ("+2" = two changes): lock the
track to **one instrument** all through, or point any single change at another instrument, or **New**
for a copy of it in a free slot to change on its own. Saved in the project and in the exports. INSTRUMENTS lists the voice slots the song uses: pick one, then
**Change...** (every game instrument, by category, with search; click to hear), **GM match**,
**Hold to hear** or keys Z-M, its envelope and its **Octave** (every note it plays moves by whole
octaves, in the exports too). SONG sets the game song it replaces and where (**Replaces**: every song the game plays as music, the places
each plays in, from `tools/kaisong/gen.py`; search finds songs by name or place; pick **Everywhere** or one place;
**None** for no game song; scenes like the card games and the BATTLE SIMULATOR are places too), volume,
reverb, priority, the loop (MIDI markers, none, whole song, custom bars), and can take every
instrument from a game song's voicegroup. **GBA accurate** (off unless turned on) plays and exports the way the game
would: 10 tracks, 5 sample voices, the rest of a busy moment cut off. Off, the song plays and exports
as it is: up to 64 tracks and 64 sample voices, so nothing is cut (the 4 Game Boy channels still play one note each); the game
plays those in full too. **Drums first** (a GM fix)
gives drum tracks priority 1 so their hits win voices in GBA accurate mode. Then **Save** and **Export .kaisong** / **Export decomp**. A `.kaisong` export is named for where it plays
(`mus_route101.kaisong` everywhere, `mus_route101@route103.kaisong` only there; no game song picked: the title) and
writes `SONG_TABLE.md` in its folder again: every song, the places it plays and the `.kaisong` files there (the same
page as the game's `docs/SONG_TABLE.md`, which the release makes for the game's `music` folder). Export into the
game's `music` folder (in the repository: committed songs reach every player in the next update).
**Aa** (top right) picks the font (Pixel, or Smooth / Bold, which stay sharp at any window size)
and the window size (fit the screen, 100-200%); both are remembered in the user's settings folder.
**Defaults** (top right): settings every MIDI and rip you open starts with: GBA accurate, volume,
reverb, even out volumes, drums first, rips' original or GM instruments, Convert GB > GBA, rips' Hi-Fi
(on at first: the 16-bit originals; off: the GBA's 8 bits), sample instruments' attack / decay /
sustain / release and an octave for every instrument but drums.
Each can stay "the song's". Saved projects keep their own; **Apply to this song** uses them now.
They live in `defaults.ini` in the settings folder (the command line reads one from `MMS_DEFAULTS`).

Command line (also what the tests use):

```
moemon_music_studio [FILE]                          the window (FILE: .mid, .mmsproj or a rip)
moemon_music_studio --original RIP OUT.wav           the rip's own sound (its emulator); MMS_BIOS = BIOS folder
moemon_music_studio --render IN OUT.wav [SECONDS]   to the loop end, or SECONDS
moemon_music_studio --export IN OUT.kaisong [SONG[@PLACE]]   OUT a folder: named for SONG@PLACE, and its SONG_TABLE.md
                                                     (prints the window's warnings; MMS_RIP_SECONDS=N records a rip at
                                                     least N s, for a song whose tags hold less than two of its loops)
moemon_music_studio --song-table DIR                 DIR/SONG_TABLE.md for the .kaisong files in DIR
moemon_music_studio --decomp IN DIR
moemon_music_studio --save IN OUT.mmsproj [SONG]
moemon_music_studio --dump IN                       song bytecode, a track a line
moemon_music_studio --dump-exact IN VOLUME          converted like mid2agb -E -V VOLUME
moemon_music_studio --raw IN OUT.f32 FRAMES         the engine's float output, frame by frame
moemon_music_studio --chip nes|gb IN OUT [SETTINGS] CHIPTUNE: OUT.nsf (NES) / OUT.gbs (Game Boy). SETTINGS, the panel's:
                                                     pN=melody|harmony|backing|echo|bass|off, pN.duty=12|25|50|75,
                                                     pN.oct=-2..2 (N: the track), chords=auto|hold|arp, arp=FRAMES,
                                                     dmc=0|1, vibrato=0|1, plan (prints the parts as read);
                                                     MMS_CHIP_DEBUG=A-B prints the parts and frames A-B
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
| `src/chip.c`, `gen_chip_drivers.py` | CHIPTUNE: any song arranged as NES / Game Boy music (parts, instruments, drum kit, DMC drums), its `.nsf` / `.gbs` and their drivers (`src/chip_drivers.h`, checked in) |
| `gen_icon.py` | `icon.png` (the GREAT BALL) -> `src/icon.h` (the window's icon) and `src/icon.ico` (the exe's), checked in |
| `src/studio.c`, `src/cli.c`, `src/ui.c`, `src/dialog.c` | the open song, command line, window, file dialogs |
| `src/rip.c`, `src/rip_*.c(pp)` | rips: loading, each console's emulator and capture, the MIDI and instruments made from it (`rip_song.c`), MP2K from the ROM (`rip_mp2k.c`), GM matching (`rip_gm.c`) |
| `src/rip_nsf.cpp` | NES rips (NSF / NSFe) |
| `third_party/` | the emulators, psflib, zlib (`third_party/README.md`: sources and licenses; the studio as a whole is GPL 2+) |
| `test/` | `run_tests.sh`, `check_mid2agb.py`, `read_kaisong.py` (reference reader), `make_test_midi.py` |
