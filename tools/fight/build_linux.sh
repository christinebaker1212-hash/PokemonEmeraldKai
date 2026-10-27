#!/bin/bash
# Moemon Fight: builds the fight engine for Linux into build/fight/ (libmf.a) and the self test.
# The game's Makefile builds the same files; this is for testing the engine on its own.
set -e
cd "$(dirname "$0")/../.."
OUT=build/fight/linux${SAN:+-asan}
mkdir -p $OUT
CFLAGS="-std=gnu11 -O2 -g -fno-strict-aliasing -ffp-contract=off -w -DPNG_SKIP_SETJMP_CHECK${SAN:+ -fsanitize=address -fno-omit-frame-pointer -O1}"
ENGINE_INC="-include fight/compat/mf_rename.h -Ifight/engine -Ifight/compat -Ifight/compat/linux -Ifight/compat/SDL -Ifight/third_party/minizip-0.15 -Ifight/third_party/libpng-1.2.38 -Ifight/third_party/lua-5.1.4/src -Ifight/third_party/zlib -Ifight/third_party/audio"
TP_INC="-Ifight/third_party/zlib -Ifight/third_party/lua-5.1.4/src -Ifight/third_party/libpng-1.2.38 -Ifight/compat"
objs=()
# a changed header rebuilds everything
if [ -f $OUT/libmf.a ] && [ -n "$(find fight include/mf_engine.h -name '*.h' -newer $OUT/libmf.a | head -1)" ]; then
    FORCE=1
fi
build() { # src flags...
    local src=$1; shift
    local obj=$OUT/$(echo "$src" | tr '/' '_' | sed 's/\.c$/.o/')
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ] || [ -n "$FORCE" ]; then
        gcc $CFLAGS "$@" -c "$src" -o "$obj"
    fi
    objs+=("$obj")
}
for f in fight/engine/*.c fight/compat/mf_*.c fight/host/*.c; do
    if [ $f = fight/engine/lookup3.c ] && [ -n "$SAN" ]; then
        build $f $ENGINE_INC -fno-sanitize=address   # hashlittle reads whole words past a string's end (by design)
    else
        build $f $ENGINE_INC
    fi
done
for f in fight/third_party/zlib/*.c; do build $f $TP_INC; done
for f in $(ls fight/third_party/lua-5.1.4/src/*.c | grep -v "/lua.c\|/luac.c\|/print.c"); do build $f $TP_INC; done
for f in fight/third_party/minizip-0.15/*.c; do build $f $TP_INC -Ifight/compat; done
for f in $(ls fight/third_party/libpng-1.2.38/png*.c | grep -v "pngtest\|pnggccrd\|pngvcrd"); do build $f $TP_INC; done
rm -f $OUT/libmf.a
ar rcs $OUT/libmf.a "${objs[@]}"
gcc $CFLAGS -iquote include tools/fight/mf_selftest.c $OUT/libmf.a -lpthread -lm -o $OUT/mf_selftest
echo "built $OUT/libmf.a $OUT/mf_selftest"
