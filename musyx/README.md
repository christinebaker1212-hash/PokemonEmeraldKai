# MusyX runtime (ORRE's sound)

`src/` and `include/` are the MusyX runtime from https://github.com/AxioDL/musyx at the commit in `COMMIT`
(a decompilation of Factor 5's MusyX with a PC renderer; CC0, see `LICENSE`), unchanged, without its SDL device
adapter (`hw_pc_sdl.c`): the game pulls the mix itself. `orre_musyx.c` is Moemon Emerald Kai's own bridge
(include/orre_sound.h). The songs and samples are never here: they come from ORRE's data pack, ripped from the
disc at release time (tools/orre/make_pack.sh). docs/ORRE_HANDOFF.md "Sound".
