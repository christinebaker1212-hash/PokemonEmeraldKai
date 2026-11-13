#!/bin/bash
# The release's compilers through ccache (release.yml "Compilers through ccache"): every gcc / g++ the Linux and
# Windows builds call (the native ones and MinGW's) is found first in a folder of links to ccache, which keeps each
# object by a hash of its preprocessed source and its flags (direct mode: the source and every header it read) and
# hands the same bytes back when they match. A file changes only when its input changes, so what is built is
# exactly what the compiler would make; only the unchanged ones are no longer compiled again. The cache (DIR) is kept
# between releases by actions/cache.
# The game's own C files are compiled through a pipe ccache can't see: tools/release/objcache.sh keeps those (OBJCACHE).
# usage: tools/release/ccache_setup.sh DIR      (on GitHub Actions: adds to $GITHUB_PATH and $GITHUB_ENV)
#        tools/release/ccache_setup.sh --prune DIR   (before the cache is kept: drops the objects this run didn't use)
set -eu
if [ "${1:-}" = --prune ]; then
    D=${2:?}
    [ -f "$D/obj/.run" ] && find "$D/obj" -type f -name '*.o' ! -newer "$D/obj/.run" -delete
    echo "object cache: $(find "$D/obj" -type f -name '*.o' | wc -l) objects, $(du -sh "$D/obj" | cut -f1)"
    exit 0
fi
DIR=${1:?usage: ccache_setup.sh CACHE_DIR}
CC=$(command -v ccache) || { echo "no ccache: the builds compile everything"; exit 0; }
mkdir -p "$DIR/bin"
for c in gcc g++ cc c++ x86_64-w64-mingw32-gcc x86_64-w64-mingw32-g++; do
    command -v "$c" > /dev/null && ln -sfn "$CC" "$DIR/bin/$c"
done
{
    echo "CCACHE_DIR=$DIR/store"
    echo "CCACHE_MAXSIZE=5G"
    echo "CCACHE_COMPRESS=1"
    echo "CCACHE_BASEDIR=${GITHUB_WORKSPACE:-$PWD}"
    # (the game's C files: tools/release/objcache.sh, keyed by these compilers' own bytes too)
    echo "OBJCACHE=$DIR/obj"
    echo "OBJCACHE_SALT=$(for c in "$(gcc --print-prog-name=cc1)" \
        "$(x86_64-w64-mingw32-gcc --print-prog-name=cc1 2>/dev/null)" "$(command -v as)" \
        "$(command -v x86_64-w64-mingw32-as)"; do [ -f "$c" ] && sha256sum < "$c"; done | sha256sum | cut -c1-16)"
} >> "${GITHUB_ENV:-/dev/null}"
mkdir -p "$DIR/obj"
touch "$DIR/obj/.run"   # (what this run doesn't use is dropped before the cache is kept: objcache_prune)
echo "$DIR/bin" >> "${GITHUB_PATH:-/dev/null}"
CCACHE_DIR="$DIR/store" ccache -z > /dev/null
echo "compilers through ccache from $DIR/bin ($(CCACHE_DIR="$DIR/store" ccache -s | grep -i 'cache size' | head -1))"
