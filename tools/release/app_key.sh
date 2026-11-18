#!/bin/bash
# The Moemon Companion app's key: what an APK is made from, so a release builds and attaches the app only when it would
# differ from the last one attached (the user, 2026-11-18: the app may lag behind while nothing it uses changed).
# Prints a hash of: the app's own files (android/, as git has them), the game's C compiled into it
# (android/app/src/main/cpp/CMakeLists.txt: the phone link's core, TRIPLE TRIAD, the POKéMON and YU-GI-OH! engines,
# EDOPro) and the headers it takes, mp_version.h without the game's version (the app's own versionName follows the
# release that attaches it), and the art and tables android/build_assets.py exported from the game (run it first: its
# output is the same bytes for the same game data).
# usage: tools/release/app_key.sh
set -e
cd "$(dirname "$0")/../.."
A=android/app/src/main/assets/game
[ -d "$A" ] || { echo "no $A: run python3 android/build_assets.py first" >&2; exit 1; }
{
    git ls-files -s android src/tt src/tcg src/ygo src/mp/phone_proto.c src/mp/phone_proto.h src/mp/monocypher \
        include/tt include/tcg include/ygo include/aw_file.h include/constants/songs.h edopro/ocgcore tools/ygo/stb_zlib.c
    grep -v 'MOEMON_VERSION\b' include/mp_version.h | sha256sum
    (cd "$A" && find . -type f | LC_ALL=C sort | xargs sha256sum)
} | sha256sum | cut -c1-40
