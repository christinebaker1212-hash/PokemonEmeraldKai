#!/bin/sh
# Moemon VMU pet: fetch and build the two outside tools at pinned commits into vmu_pet/.tools (never committed).
#   waterbear: the VMU assembler (Rust, MIT)          https://github.com/wtetzner/waterbear
#   libevmu:   ElysianVMU's emulator core (C, MIT)     https://github.com/gyrovorbis/libevmu  (headless tests only)
# usage: vmu_pet/tools/get_tools.sh   (needs git, cargo, cmake, a C compiler)
set -e
WATERBEAR_COMMIT=9d7a2536ed7234965417adf2fe619e6301c352a2
LIBEVMU_COMMIT=2b155021472a11785a4297cc43f4d5c7fca7a5d4

here=$(cd "$(dirname "$0")/.." && pwd)
dst="$here/.tools"
mkdir -p "$dst"

fetch() { # dir url commit
    if [ ! -d "$dst/$1/.git" ]; then git clone -q "$2" "$dst/$1"; fi
    git -C "$dst/$1" fetch -q origin "$3" 2>/dev/null || git -C "$dst/$1" fetch -q origin
    git -C "$dst/$1" checkout -q "$3"
    git -C "$dst/$1" submodule update -q --init --recursive
}

fetch waterbear https://github.com/wtetzner/waterbear "$WATERBEAR_COMMIT"
(cd "$dst/waterbear" && cargo build -q --release)
ln -sf "$dst/waterbear/target/release/waterbear" "$dst/waterbear-bin"

fetch libevmu https://github.com/gyrovorbis/libevmu "$LIBEVMU_COMMIT"
cmake -S "$dst/libevmu" -B "$dst/libevmu/build" -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "$dst/libevmu/build" -j"$(nproc 2>/dev/null || echo 4)" >/dev/null
echo "tools ready in $dst"
