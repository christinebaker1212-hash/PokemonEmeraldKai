# Dev workspace

This folder holds generated development builds, separate artwork packs and test
results. Source remains in `src/tcg`, `include/tcg`, and `tools/tcg` on the
`dev/pokemon-card-system` Git branch. A folder does not merge a feature into main.

- `builds/linux/`, `builds/windows/`: runnable dev binaries and their small assets.
- `artwork/tcg-art/`: the versioned one-time artwork pack and its generated images.
- `artwork/benchmark/`: compression measurements and comparison images.
- `test-results/`: functional logs and screenshots.

`./dev/build.sh linux 1` builds/stages a Linux dev executable.
`./dev/run.sh` runs it with isolated dev settings and the local artwork pack.

The normal Makefile keeps its shared asset cache under `build/`. Main release
commands and workflows remain unchanged. Generated files here are ignored by Git.
