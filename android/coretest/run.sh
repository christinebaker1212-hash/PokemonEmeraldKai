#!/bin/sh
# The app's link core against a headless game on a desktop JVM (CoreTest.java). Run from anywhere after make linux.
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
APP=$ROOT/android/app/src/main
OUT=${OUT:-$ROOT/build/coretest}
CACHE=${XDG_CACHE_HOME:-$HOME/.cache}/moemon
JSON=$CACHE/json-20240303.jar
mkdir -p "$OUT/classes" "$CACHE"
[ -f "$JSON" ] || curl -sSfL -o "$JSON" \
    https://maven-central.storage-download.googleapis.com/maven2/org/json/json/20240303/json-20240303.jar
JH=${JAVA_HOME:-$(dirname "$(dirname "$(readlink -f "$(command -v javac)")")")}
cc -shared -fPIC -O2 -I"$JH/include" -I"$JH/include/linux" -I"$ROOT/src/mp" -o "$OUT/libmoemonlink.so" \
    "$APP/cpp/link_jni.c" "$ROOT/src/mp/phone_proto.c" "$ROOT/src/mp/monocypher/monocypher.c"
javac -nowarn -d "$OUT/classes" -cp "$JSON" "$APP"/java/com/moemon/companion/core/*.java \
    "$APP/java/com/moemon/companion/Native.java" "$APP/java/com/moemon/companion/NativeCrypto.java" \
    "$ROOT/android/coretest/CoreTest.java"
exec java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" CoreTest "$ROOT"
