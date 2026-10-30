# .kaisong: custom songs for Moemon Emerald Kai

Moemon Music Studio (`tools/moemon_music`) turns a MIDI into a `.kaisong` file: one song, its 128-voice
voicegroup and the names of the game samples it plays. The game plays them from its `music` folder (since 1.44.0,
`src/mp/mp_kaisong.c`). This page is the format and how the game loads it.

The studio plays songs with a C port of the game's sound engine (MP2K as the PC port runs it). Checked
against the game on 15 songs: all 15 match the game's audio sample for sample. So a `.kaisong` loaded the way described below sounds the same
in the game as in the studio.

## The file

Everything is little-endian. The file starts with a 32-byte header, then chunks.

| offset | type     | field                                                                  |
|-------:|----------|------------------------------------------------------------------------|
| 0      | char[8]  | `"KAISONG\0"`                                                          |
| 8      | u16      | version: `1`, or `2` when the song brings its own samples (`SMPL` / `WAVE`) |
| 10     | u16      | header size: `32` (chunks start here)                                  |
| 12     | u8       | track count: 1-10, or up to 64 when the song was made with *GBA accurate* off |
| 13     | u8       | priority (`SongHeader.priority`, mid2agb `-P`)                         |
| 14     | u8       | reverb (`SongHeader.reverb`: `0x80 \| level`, or `0` for no change)      |
| 15     | u8       | master volume 0-127, for information only (already in the VOL commands) |
| 16     | u32      | chunk count (5, version 2: 7)                                          |
| 20     | u8[12]   | zero                                                                   |

Each chunk is a 4-letter id, a u32 size, then `size` bytes, padded with zeros to a multiple of 4. Skip
chunks you don't know. Version 1 writes these five, in this order (version 2 adds `SMPL` and `WAVE`
after them):

### `INFO`: text

`key=value` lines (UTF-8). For information and for the loader's messages:

```
song=mus_route101          the game song it replaces (also the file's suggested name)
place=route103             only on that map (a place key, docs/SONG_TABLE.md); missing = everywhere
title=My Route 101
midi=my_route101.mid
volume=100
reverb=50                  -1 = off
priority=0
loop=yes                   yes / no
voices=5                   sample voices the song was made for (5 = GBA accurate; 64 = no limit)
tool=Moemon Music Studio 1.0
made=2026-10-25 14:55
source=Final Fantasy 7 - Nobuo Uematsu - PlayStation      (songs made from music of another game)
gain=5.2                   dB, -12 to 12: the game plays the song that much louder or quieter (missing = 0)
```

`gain=` is the one key the game uses: every sample and Game Boy voice's volume (velocity × track volume, before
the clamp to 255) is multiplied by it, in 0.1 dB steps (src/music_player.c ChnVolSetAsm, mp_kaisong.c
MP_Kaisong_Gain). The studio doesn't write it; `tools/kaisong/loudness.py --write` does, so a song plays as loud as
the game song it replaces (the measure: RMS over 400 ms blocks with EBU R 128's two gates, on the game's own output).
A voice already at full volume can't go louder, so a song with big boosts may need a second `--write`.

### `NAME`: sample and wave names

`u32 count`, then `count` times: `u16 length`, `length` bytes of ASCII (no terminator). Each is a full
symbol name from the game's sound data:

- `DirectSoundWaveData_<name>` (`sound/direct_sound_data.inc`), e.g. `DirectSoundWaveData_sc88pro_flute`
- `ProgrammableWaveData_<n>` (`sound/programmable_wave_data.inc`), e.g. `ProgrammableWaveData_3`

### `VGRP`: voice groups

`u32 groupCount`, then `groupCount × 128` voice records of 16 bytes. **Group 0 is the song's voicegroup**
(what `VOICE n` selects). Groups 1 and up are the drum kits and key-split instrument groups that group 0
points at, 128 voices each. They are copies of the game's own groups (`voicegroup_rs_drumset`,
`voicegroup_piano_keysplit`...), so the file plays the same even if a later game version changes those.

Voice record:

| offset | type | field                                                                  |
|-------:|------|------------------------------------------------------------------------|
| 0      | u8   | type (`ToneData.type`: 0/8/16 sample, 1-4 / 9-12 Game Boy, 0x40 key-split, 0x80 drum kit) |
| 1      | u8   | key (`ToneData.key`, `drumKey`)                                         |
| 2      | u8   | length (`ToneData.length`, `cgbLength`)                                 |
| 3      | u8   | pan / sweep (`ToneData.pan_sweep`)                                      |
| 4      | u8   | attack                                                                 |
| 5      | u8   | decay                                                                  |
| 6      | u8   | sustain                                                                |
| 7      | u8   | release                                                                |
| 8      | u8   | ref kind (below)                                                       |
| 9      | u8   | zero                                                                   |
| 10     | u16  | ref                                                                    |
| 12     | u16  | key-split table (kind 5)                                               |
| 14     | u16  | zero                                                                   |

| kind | the voice's pointer field (`wav` / `group` / ...) is                                        |
|-----:|----------------------------------------------------------------------------------------------|
| 0    | nothing (never written by version 1)                                                         |
| 1    | the sample named `NAME[ref]` (`struct WaveData *`)                                           |
| 2    | the programmable wave named `NAME[ref]` (`cgb3Sample`, 16 bytes)                             |
| 3    | the value `ref`: square duty 0-3 or noise period 0-1 (`squareNoiseConfig`)                   |
| 4    | drum kit: `group` = group `ref` (1+), `keySplitTable` = NULL                                 |
| 5    | key-split: `group` = group `ref` (1+), `keySplitTable` = `KSPL` table `ks`                   |
| 6    | version 2: the song's own sample `SMPL[ref]` (`struct WaveData *`)                           |
| 7    | version 2: the song's own programmable wave `WAVE[ref]` (16 bytes)                            |

Bytes 0-7 are the game's own `ToneData` bytes, exactly as the voice macros in `asm/macros/music_voice.inc`
lay them out (pan for a sample is in byte 3 as `0x80 | pan`; for Game Boy voices the macros put it in byte
2). For kinds 4 and 5 the four envelope bytes are 0: in memory that space holds `keySplitTable`.
Groups 1 and up only hold kinds 1-3 (the game never plays a kit inside a kit).

Unused slots hold the game's empty-slot voice, `voice_square_1 60, 0, 0, 2, 0, 0, 15, 0`.

### `KSPL`: key-split tables

`u32 count`, then `count × 128` bytes. Entry `k` is the voice of the group to use for MIDI key `k`, so the
table is indexed by key directly (the game's `keysplit_*` labels point before their data for the same
reason; here that offset is already applied).

### `TRAK`: tracks

`u32 trackCount`, then per track:

```
u32 length
u32 relocationCount
u8  code[length]
u32 relocation[relocationCount]     offsets in code of a 4-byte jump target
zero padding to a multiple of 4
```

`code` is the song's MP2K bytecode, the same commands mid2agb makes (`sound/MPlayDef.s`). Every place the
game has a pointer (after `GOTO 0xB2`, `PATT 0xB3`, and a branching `MEMACC 0xB9`) holds a **4-byte offset
into the same track** instead, and its position is in the relocation list, so a loader never has to parse
the bytecode. Version 1 writes:

| bytes            | command                                                    |
|------------------|------------------------------------------------------------|
| `80`-`B0`        | wait (`W00`...`W96`, clocks from `gClockTable`)            |
| `B1`             | FINE                                                       |
| `B2` t32         | GOTO (loops)                                               |
| `B9` op addr data [t32] | MEMACC (only from game MIDIs that use it; t32 for ops 6-17) |
| `BA` `BB` `BC` `BD` `BE` `BF` `C0` `C1` `C2` `C3` `C4` `C5` `C8` + 1 byte | PRIO TEMPO KEYSH VOICE VOL PAN BEND BENDR LFOS LFODL MOD MODT TUNE |
| `CD` 08/09 v     | XCMD xIECV / xIECL                                         |
| `CE` key         | EOT                                                        |
| `CF` key vel     | TIE                                                        |
| `D0`-`FF` key vel [gate] | note (`N01`...`N96`)                               |

No running status, no `PATT` / `PEND` / `REPT` (no pattern compression), so the code is a bit bigger
than the game's songs and plays the same.

### `SMPL` and `WAVE` (version 2): the song's own samples

Songs made from another game's music (PSF, USF, 2SF... rips) bring their original instruments. `SMPL`:
`u32 count`, then per sample: `u16 nameLength`, the name (ASCII, `[a-z0-9_]`), zero padding to a multiple
of 4 (from the chunk's start), then exactly a `struct WaveData` as the game lays it out:

```
u16 type        0
u16 status      0x4000 = loops (the game's WaveData.status)
u32 freq        22.10 fixed point: the rate a note on key 60 plays it at
u32 loopStart   in samples
u32 size        in samples
s8  data[size]  8-bit signed PCM, then zero padding to a multiple of 4
```

so a loader can allocate `16 + size` bytes (plus a few zero bytes: the mixer reads one sample past the
end) and copy the header and data in as they are. `WAVE`: `u32 count`, then per wave the name the same way
(length, name, padding) and its 16 bytes (32 4-bit steps, like `ProgrammableWaveData_*`).

`tools/moemon_music/test/read_kaisong.py FILE` is the reference reader: it checks everything a loader
should check and prints the voices; `--bytecode` prints the tracks.

## How the game loads it (`src/mp/mp_kaisong.c`)

1. **Where.** The `music` folder next to the game (`MOEMON_MUSIC_DIR` for tests). The file name is the switch:
   `music/<song>.kaisong` plays instead of that song everywhere, `music/<song>@<place>.kaisong` only on one map.
   The place is the map's `MAP_` id without `MAP_`, in lower case (`mus_route101@route103.kaisong`), or a scene:
   a part of the game that picks its own songs (`mus_vs_trainer@sim.kaisong`: the BATTLE SIMULATOR; `@tcg`, `@ygo`,
   `@tt`: the card games; `@title_screen`...). A place's own file wins over the everywhere one. Scenes are found from
   the source (`include/mp_kaisong_scene.h`: every call that starts a song records the file that asked; a scene is a
   source file, or a folder of `src/` like `src/tcg`, that names songs; a scene borrowing the game's battle code holds
   the music with `MP_MUSIC_SCENE_HOLD(songs...)`), so a new scene is in the table and the studio's picker the next
   time `tools/kaisong/gen.py` runs. A scene's file plays only for the songs that scene names, asked for by it; a map's
   file never plays in a scene, so replacing a song in one place or scene never changes another that uses it. `docs/SONG_TABLE.md` (made by `tools/kaisong/gen.py`) lists every song and every
   place it plays. Song names are `sound/song_table.inc`'s; only songs on the music player (BGM, fanfares too)
   can be replaced. The folder ships with the game (`tools/release/package.sh`), so a song committed to the
   repository's `music` folder reaches every player in the next update. The game looks at the folder once, at the
   first song, and loads a file the first time it plays (then keeps it).

2. **Samples.** `src/data/mp_kaisong_samples.h` (generated by `tools/kaisong/gen.py`) names every
   `DirectSoundWaveData_*` and `ProgrammableWaveData_*`. Version 2 files bring some of their own (`SMPL` / `WAVE`),
   allocated as they are with zero bytes after them. An unknown name means the file is from a newer game: it isn't
   loaded, the log says why, and the game's own song plays (the same for any broken file).

3. **Voices** become `struct MP2KInstrument` groups (`include/music_player.h`, the same layout as `struct ToneData`);
   kinds 4 / 5 set `group` and `keySplitTable` after the envelope bytes.

4. **Tracks.** Each relocation grows from 4 bytes to a pointer (`MP2K_event_goto` reads `sizeof(uintptr_t)` bytes,
   little-endian), and a FINE byte goes after each track in case one runs past its end.

5. **Header.** A `struct SongHeader` with `trackCount` parts, the file's priority and reverb, `tone` = group 0.

6. **Hook.** Every `m4aSongNum*` function in `src/m4a.c` asks `MP_Kaisong_Header(n, builtin)`, which always returns
   the same pointer for the same file (`m4aSongNumStartOrChange` compares headers). The map music code
   (`src/overworld.c`) also asks `MP_Kaisong_Differs`: the same song number on the next map can still be another
   file, and then it fades over like a new song. Spectating sends the song number with the place's variant in bits
   10-15 (`MP_Kaisong_VariantOf` / `StartVariant`), so a spectator hears the friend's version.

7. **Settings.** Each file keeps its own: priority and reverb (header bytes 13-14), all of its tracks (the BGM
   player has 64: `gMPlayTrack_BGM`, `NUM_TRACKS_BGM`; the player's `status` keeps a nonzero summary of the 64 track
   bits in its low 16, and `TRACKS_ALL` reaches every track in the volume / pitch / pan controls), and its voices
   (INFO `voices=`: the sample channels mixed while it plays, `MAX_DIRECTSOUND_CHANNELS` = 64; the game's own songs
   keep 5). The loop and volume are in the bytecode.

8. **MEMACC.** `ply_memacc` skips a whole pointer when a branch isn't taken (it skipped 4 bytes on 64-bit).

### Checking it

`tools/test/kaisong_test.sh` exports songs with the studio and plays them in the game (`MOEMON_AUDIO_DUMP` records
the game's sound; `mp playsong N` starts a song from a clean sound state, Game Boy channels included, as the studio
does): the game's Route 101 exported and loaded sounds exactly like the built-in song, and a 15-track song made for
64 voices sounds exactly like the studio's own render of it (`--raw`), sample for sample. It also checks a place's
file, the switch to the everywhere file on the next map, a file naming a sample the game doesn't have, and a
version 2 file.

## Building a song into the game instead

**Export decomp** writes `<song>.s` and `<song>_kai.inc` (`voice_group <song>_kai`), the same text format
as the game's sound folder, using the game's own voice macros and sample symbols. A song with its own
samples also gets a WAV per sample (8-bit, with a `smpl` loop and an `agbp` chunk holding the exact
`freq`, so `tools/wav2agb` makes exactly the studio's sample from it), a 16-byte `.pcm` per Game Boy wave,
and `<song>_samples.inc` with their `DirectSoundWaveData_*` / `ProgrammableWaveData_*` labels to add to
`sound/direct_sound_data.inc` / `sound/programmable_wave_data.inc`. They assemble with the
game's flags (the studio's tests do this). The build only makes songs from `sound/songs/midi/*.mid`
(`MID_OBJS` in the Makefile; the `$(SONG_BUILDDIR)` rule in `audio_rules.mk` uses a `SONG_SUBDIR` nothing
defines), so adding one needs: a rule and an `OBJS` entry for the `.s` (assembled like the MIDI songs:
`$(ASM_PSEUDO_OP_CONV) $< | $(AS) $(ASFLAGS) -I sound`), the `.inc` included from `sound/voice_groups.inc`,
and the song's `.mid` and `midi.cfg` line removed so its symbol isn't defined twice.
