# Custom music

`.kaisong` files here play instead of the game's songs, for every player: the folder ships with each update
(`tools/release/package.sh`). Make them with Moemon Music Studio (`tools/moemon_music`): SONG > Replaces picks the
song and where it plays, and the export is named for it:

- `<song>.kaisong`: that song everywhere (`mus_route101.kaisong`)
- `<song>@<place>.kaisong`: only on one map (`mus_vs_gym_leader@rustboro_city_gym.kaisong`) or in one scene
  (`mus_vs_trainer@sim.kaisong`: the BATTLE SIMULATOR; `@tcg`, `@ygo`, `@tt`: the card games)

Every song here plays as loud as the one it replaces, with nothing to do: each release measures them in the game
it built and writes each file's volume into the copies it ships (`tools/kaisong/loudness.py`, which can also be run
by hand to see the numbers).

`docs/SONG_TABLE.md` lists every song, the places it plays and the custom songs here. The release makes it again
(`python3 tools/kaisong/gen.py`); run that before committing new songs so the page in the repository matches.
The format: `docs/KAISONG_FORMAT.md`. The loader: `src/mp/mp_kaisong.c`.
