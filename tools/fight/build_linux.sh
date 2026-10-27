#!/bin/bash
# Moemon Fight: builds the fight engine for Linux into build/fight/ (libmf.a) and the self test.
# The game's Makefile builds the same files; this is for testing the engine on its own.
# WIN=1: the Windows build (mingw) into build/fight/win, mf_selftest.exe (runs under Wine).
set -e
cd "$(dirname "$0")/../.."
if [ -n "$WIN" ]; then
    OUT=build/fight/win
    CC=x86_64-w64-mingw32-gcc
    AR=x86_64-w64-mingw32-ar
    OBJCOPY=x86_64-w64-mingw32-objcopy
    PLAT_INC=
    STATE_SECTIONS="--rename-section '.data=.data\$mfB' --rename-section '.bss=.data\$mfB2,alloc,load,contents,data'"
else
    OUT=build/fight/linux${SAN:+-asan}
    CC=gcc
    AR=ar
    OBJCOPY=objcopy
    PLAT_INC=-Ifight/compat/linux
    STATE_SECTIONS="--rename-section .data=mfstate --rename-section .data.rel=mfstate --rename-section .data.rel.local=mfstate --rename-section .bss=mfstate,alloc,load,contents,data"
fi
mkdir -p $OUT
CFLAGS="-std=gnu11 -O2 -g -fno-strict-aliasing -ffp-contract=off -w -DPNG_SKIP_SETJMP_CHECK${SAN:+ -fsanitize=address -fno-omit-frame-pointer -O1}"
ENGINE_INC="-include fight/compat/mf_rename.h -Ifight/engine -Ifight/compat $PLAT_INC -Ifight/compat/SDL -Ifight/third_party/minizip-0.15 -Ifight/third_party/libpng-1.2.38 -Ifight/third_party/lua-5.1.4/src -Ifight/third_party/zlib -Ifight/third_party/audio"
TP_INC="-Ifight/third_party/zlib -Ifight/third_party/lua-5.1.4/src -Ifight/third_party/libpng-1.2.38 -Ifight/compat"
objs=()
# a changed header rebuilds everything
if [ -f $OUT/libmf.a ] && [ -n "$(find fight include/mf_engine.h -name '*.h' -newer $OUT/libmf.a | head -1)" ]; then
    FORCE=1
fi
# the fight's state for rollback: the engine's globals (not the music's) and the SDL layer's, in one section
# (STATE_SECTIONS, as in the Makefile)
build() { # src flags...
    local src=$1; shift
    local obj=$OUT/$(echo "$src" | tr '/' '_' | sed 's/\.c$/.o/')
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ] || [ -n "$FORCE" ]; then
        $CC $CFLAGS "$@" -c "$src" -o "$obj"
        case $src in
        fight/engine/bgm*) ;;
        fight/engine/*|fight/compat/mf_sdl12.c) eval $OBJCOPY $STATE_SECTIONS "$obj" ;;
        esac
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
for f in fight/third_party/zlib/*.c fight/third_party/musl_math/*.c fight/third_party/dlmalloc/mf_dlmalloc.c; do build $f $TP_INC; done
for f in $(ls fight/third_party/lua-5.1.4/src/*.c | grep -v "/lua.c\|/luac.c\|/print.c"); do build $f $TP_INC; done
for f in fight/third_party/minizip-0.15/*.c; do build $f $TP_INC -Ifight/compat; done
for f in $(ls fight/third_party/libpng-1.2.38/png*.c | grep -v "pngtest\|pnggccrd\|pngvcrd"); do build $f $TP_INC; done
rm -f $OUT/libmf.a
$AR rcs $OUT/libmf.a "${objs[@]}"
if [ -n "$WIN" ]; then
    $CC $CFLAGS -iquote include tools/fight/mf_selftest.c $OUT/libmf.a -static -lwinmm -lpthread -o $OUT/mf_selftest.exe
else
    $CC $CFLAGS -iquote include tools/fight/mf_selftest.c $OUT/libmf.a -lpthread -lm -o $OUT/mf_selftest
fi
echo "built $OUT/libmf.a $OUT/mf_selftest"
