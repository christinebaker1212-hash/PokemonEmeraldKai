#!/bin/bash
# Builds EDOPro's engine (edopro/ocgcore and its Lua, as C++) into a static library for the native tests:
#   tools/ygo/edo/build_core.sh OUT_DIR   ->  OUT_DIR/libocgcore.a (rebuilt only when a source is newer)
set -e
cd "$(dirname "$0")/../../.."
OUT=${1:?usage: build_core.sh OUT_DIR}
mkdir -p "$OUT/edo-obj"
LIB="$OUT/libocgcore.a"
if [ -f "$LIB" ] && [ -z "$(find edopro/ocgcore "$0" -newer "$LIB" -name '*.[ch]*' -o -newer "$LIB" -name "build_core.sh" | head -1)" ]; then
    exit 0
fi
SKIP="lbitlib lcorolib ldblib linit loadlib loslib ltests lua luac lutf8lib onelua"
objs=()
pids=()
for f in edopro/ocgcore/*.cpp edopro/ocgcore/lua/src/*.c; do
    b=$(basename "$f"); b=${b%.*}
    case " $SKIP " in *" $b "*) [[ $f == *lua/src* ]] && continue;; esac
    o="$OUT/edo-obj/$b.o"
    objs+=("$o")
    g++ -std=c++17 -O2 -fno-rtti -w -Iedopro/ocgcore/lua/src -include edopro/ocgcore/lua/luaconf-customize.h \
        -DluaP_opmodes=edo_luaP_opmodes -Dlua_ident=edo_lua_ident -DluaT_typenames_=edo_luaT_typenames_ \
        -Dluai_ctype_=edo_luai_ctype_ \
        -x c++ -c "$f" -o "$o" &
    pids+=($!)
    if [ ${#pids[@]} -ge "$(nproc)" ]; then wait "${pids[0]}"; pids=("${pids[@]:1}"); fi
done
wait
rm -f "$LIB"
ar rcs "$LIB" "${objs[@]}"
