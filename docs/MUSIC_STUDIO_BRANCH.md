# Moemon Music Studio lives on its own branch

**Branch:** `claude/moemon-emerald-midi-player-sv25iy`. It isn't merged into this (default) branch yet.

Moemon Music Studio is a separate SDL2 program in `tools/moemon_music/`. It is not part of the game build.
It plays MIDIs and ripped game music with the game's own instruments and sound engine, and exports songs
for Kai. The engine is a port of `src/music_player.c` and friends, so check engine changes against it.

## What it does

- **Opens MIDIs and rips:** PSF / PSF2 / USF / GBS / 2SF / GSF rips and GameCube streams. The rips play
  through emulators vendored in its `third_party/`.
- **Converts them to the game's song format:** each original instrument comes in as a sample. Alternatively
  you get the closest General MIDI instrument of the game, or for Game Boy songs "Convert GB > GBA".
- **Hi-Fi mode** (studio only): plays the original samples in 16 bits at the console's own rate.
- **Exports:**
  - `.kaisong` files (format and loading plan in `docs/KAISONG_FORMAT.md` on that branch; the game has no
    loader yet);
  - decomp sources (`<song>.s` and a voicegroup `.inc`).
- **Windows build:** `make windows`. The user gets it as a zip.

## What merging it brings

- Everything under `tools/moemon_music/`: sources, tests and the vendored emulators.
- `docs/KAISONG_FORMAT.md`.
- About 6 lines added to `CLAUDE.md`: a key-locations entry and the agent rule.

Nothing in `src/`, `include/` or the game build changes. Merging only adds the studio's own folder and
those docs, so no conflicts with game work are expected.

## Before you touch it

- **Merge or check out that branch before editing anything under `tools/moemon_music/`.** That branch has
  the current studio; this branch has none of it.
- **The studio's own `CLAUDE.md` notes and `make test` live there**, along with the release procedure
  (Linux build, Windows build, Wine check, zip).
- **The user decides when to merge.** Until then the music studio session pushes only to its branch.
