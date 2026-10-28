# The DANCE machine's tools (docs/RHYTHM_DECISIONS.md)

- `test.sh [SONGS_DIR]`: the game's simfile reader and judging rules on every chart (default `stepmania/Songs`):
  `test_charts` (each chart loads, notes in order, time and beat agree both ways) and `test_play` (every chart
  stepped perfectly: MARVELOUS full combo, AAA, feet x 10,000,000; 50 ms late: GREAT; not stepped: the life bar
  empties).
- `gen_assets.py`: `graphics/rhythm/rhythm.bin`, DDR EXTREME's look cut down to 240x160 and its sounds. Needs
  - `THEME`: Inorizushi's DDR-EXTREME-JP-SM5 theme (`git clone https://github.com/Inorizushi/DDR-EXTREME-JP-SM5`);
  - `BEWARE`: the `stepmania` folder of Beware's StepMania EXTREME build (the Google Drive link in
    https://gist.github.com/d3v1an7/3d048a4ad33ce397eb221e4bb0470a34): its `NoteSkins/dance` and the
    `Announcers/katzeextreme_beware` announcer;
  - ffmpeg (a few of the theme's sounds are MP3).

  `THEME=... BEWARE=... python3 tools/rhythm/gen_assets.py`
- The songs in `stepmania/Songs` are that build's `Songs` folder (the 240 EXTREME arcade songs; its `.crs` courses
  and `.jbf` file left out). `tools/release/package_songs.sh` packs them for a release.
