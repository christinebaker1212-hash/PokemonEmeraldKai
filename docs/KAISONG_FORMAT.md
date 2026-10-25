# .kaisong: custom songs for Moemon Emerald Kai

Moemon Music Studio (`tools/moemon_music`) turns a MIDI into a `.kaisong` file: one song, its 128-voice
voicegroup and the names of the game samples it plays. **The game doesn't read these files yet.** This
page is the format and a plan for the loader.

The studio plays songs with a C port of the game's sound engine (MP2K as the PC port runs it). Checked
against the game on 15 songs: all 15 match the game's audio sample for sample. So a `.kaisong` loaded the way described below sounds the same
in the game as in the studio.

## The file

Everything is little-endian. The file starts with a 32-byte header, then chunks.

| offset | type     | field                                                                  |
|-------:|----------|------------------------------------------------------------------------|
| 0      | char[8]  | `"KAISONG\0"`                                                          |
| 8      | u16      | version: `1`                                                           |
| 10     | u16      | header size: `32` (chunks start here)                                  |
| 12     | u8       | track count: 1-10, or up to 32 when the song was made with *Go over 10 tracks* |
| 13     | u8       | priority (`SongHeader.priority`, mid2agb `-P`)                         |
| 14     | u8       | reverb (`SongHeader.reverb`: `0x80 \| level`, or `0` for no change)      |
| 15     | u8       | master volume 0-127, for information only (already in the VOL commands) |
| 16     | u32      | chunk count (5)                                                        |
| 20     | u8[12]   | zero                                                                   |

Each chunk is a 4-letter id, a u32 size, then `size` bytes, padded with zeros to a multiple of 4. Skip
chunks you don't know. Version 1 writes these five, in this order:

### `INFO`: text

`key=value` lines (UTF-8). For information and for the loader's messages:

```
song=mus_route101          the game song it replaces (also the file's suggested name)
title=My Route 101
midi=my_route101.mid
volume=100
reverb=50                  -1 = off
priority=0
loop=yes                   yes / no
voices=5                   sample voices the song was made for (the game mixes 5; up to 12)
tool=Moemon Music Studio 1.0
made=2026-10-25 14:55
```

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

`tools/moemon_music/test/read_kaisong.py FILE` is the reference reader: it checks everything a loader
should check and prints the voices; `--bytecode` prints the tracks.

## Loading it in the game (plan)

1. **Where.** A `music` folder next to the game's other user files, `music/<song>.kaisong` replacing
   the song of that name (`mus_route101.kaisong` plays instead of `MUS_ROUTE101`). The file name is the
   switch, so players can swap songs by renaming files. `INFO song=` says what the studio meant it for.
   Online, the music is each player's own business: nothing needs sending.

2. **Samples.** Generate a table `{ "DirectSoundWaveData_x", DirectSoundWaveData_x }` for every sample in
   `sound/direct_sound_data.inc` and every wave in `sound/programmable_wave_data.inc`. They are
   global symbols (`::`), visible from C like `voicegroup_dummy` is (`include/gba/m4a_internal.h`);
   `tools/moemon_music/gen_soundfont.py` has the parsing to copy. An unknown name means the file is
   from a newer game: don't load it, play the built-in song, and say why in the log.

3. **Voices.** Build `struct MP2KInstrument groups[groupCount][128]` (`include/music_player.h`) on the
   heap: bytes 0-7 into `type, drumKey, cgbLength, panSweep, attack, decay, sustain, release`, then the
   pointer union by kind (table above). For kinds 4 / 5 set `group` and `keySplitTable` *after* the
   envelope bytes (they share memory). `struct ToneData` is the same layout.

4. **Tracks.** Pointers are `sizeof(uintptr_t)` bytes in the PC build (`MP2K_event_goto` reads them
   byte by byte, little-endian), so each relocation grows from 4 to `P = sizeof(uintptr_t)` bytes.
   Copy the code into a buffer `length + relocationCount * (P - 4)` long, mapping old offsets to new:
   `new(off) = off + (P - 4) * (number of relocations r with r < off)`. At each relocation write the
   pointer `trackBase + new(target)`, where `target` is the 4-byte value that was there.

5. **Header.** Allocate a `struct SongHeader` with room for `trackCount` parts: `trackCount`,
   `blockCount = 0`, `priority`, `reverb`, `tone = groups[0]`, `part[i]` = each track.

6. **Hook.** `m4aSongNumStart`, `m4aSongNumStartOrChange`, `m4aSongNumStop` and friends in `src/m4a.c`
   read `gSongTable[n].header`. Give them one function that returns the loaded header when there is a
   file for song `n` (load on first use, keep it), else the built-in one. Always return the *same*
   pointer for a song: `m4aSongNumStartOrChange` decides "same song, keep playing" by comparing headers,
   and `src/mp/mp_spectate.c` finds the playing song by comparing `gSongTable[i].header` with
   `gMPlayInfo_BGM.songHeader` (it needs the custom header to map back to `i`, or a spectator hears
   nothing). Song names for the file lookup: `include/constants/songs.h` / `sound/song_table.inc`.

7. **Limits.** The BGM player has 10 tracks (`gMPlayTrack_BGM[10]`; `MPlayStart` ignores parts past the
   player's track count). The studio exports at most 10 unless the song's *Go over 10 tracks* option is
   on: then up to 32, and playing those in full needs a bigger BGM player (`gMPlayTrack_BGM`, `NUM_TRACKS_BGM`
   in `sound/music_player_table.inc`, the `trackBits` / status masks in `MP2KPlayerMain`, which are 16 bits).
   Songs made with more *Voices* than 5 (INFO `voices=`) expect `m4aSoundMode` with that channel count
   (`SOUND_MODE_MAXCHN`, up to `MAX_DIRECTSOUND_CHANNELS` = 12; m4aSoundInit sets 5): extra tracks only sound
   when there are voices free for them, since earlier tracks win the channels, 5 sample channels plus the 4 Game Boy channels (the studio shows how many are busy), and
   reverb only changes when bit 7 is set, then stays for later songs, as in the game.

8. **Known engine quirk.** On 64-bit, `ply_memacc` (src/m4a.c) skips 4 bytes when a MEMACC branch is not
   taken, but the pointer after it is 8 bytes. Only game MIDIs carry MEMACC (Vs. Trainer uses `mem_set`,
   which has no pointer); if a loaded song ever needs branching MEMACC, fix that skip first.

### Checking a loader

Open one of the game's own MIDIs in the studio (`sound/songs/midi/mus_route101.mid`: it picks up the song's
instruments and settings and converts exactly like the build does), export it, load it in the game in
place of the built-in song, and record both: the audio should be identical sample for sample. That's how
the studio was checked against the game. It used a local, throwaway change to `AgbMain` that started a
song, called `m4aSoundMain` / `m4aSoundVSync` in a loop and wrote `audioBuffer` to a file.

## Building a song into the game instead

**Export decomp** writes `<song>.s` and `<song>_kai.inc` (`voice_group <song>_kai`), the same text format
as the game's sound folder, using the game's own voice macros and sample symbols. They assemble with the
game's flags (the studio's tests do this). The build only makes songs from `sound/songs/midi/*.mid`
(`MID_OBJS` in the Makefile; the `$(SONG_BUILDDIR)` rule in `audio_rules.mk` uses a `SONG_SUBDIR` nothing
defines), so adding one needs: a rule and an `OBJS` entry for the `.s` (assembled like the MIDI songs:
`$(ASM_PSEUDO_OP_CONV) $< | $(AS) $(ASFLAGS) -I sound`), the `.inc` included from `sound/voice_groups.inc`,
and the song's `.mid` and `midi.cfg` line removed so its symbol isn't defined twice.
