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
mkdir -p "$OUT/ttinc" && ln -sfn "$ROOT/include/tt" "$OUT/ttinc/tt"   # (the game's include/ has its own strings.h)
mkdir -p "$OUT/tcginc" && ln -sfn "$ROOT/include/tcg" "$OUT/tcginc/tcg" && ln -sfn "$ROOT/include/mp_version.h" "$OUT/tcginc/mp_version.h"
mkdir -p "$OUT/ygoinc" && ln -sfn "$ROOT/include/ygo" "$OUT/ygoinc/ygo" && ln -sfn "$ROOT/include/mp_version.h" "$OUT/ygoinc/mp_version.h"
(cd "$ROOT" && EDO_CFLAGS=-fPIC tools/ygo/edo/build_core.sh "$OUT/ocgcore")   # (EDOPro's engine: once, then when it changes)
TT="$ROOT/src/tt"
TCG="$ROOT/src/tcg"
YGO="$ROOT/src/ygo"
cc -shared -fPIC -O2 -I"$JH/include" -I"$JH/include/linux" -I"$ROOT/src/mp" -I"$OUT/ttinc" -I"$OUT/tcginc" -I"$OUT/ygoinc" -I"$YGO" -DYGO_EDO_PAK_EXTERNAL \
    -o "$OUT/libmoemonlink.so" "$APP/cpp/link_jni.c" "$APP/cpp/tt_jni.c" "$APP/cpp/tcg_jni.c" "$APP/cpp/ygo_jni.c" "$ROOT/src/mp/phone_proto.c" "$ROOT/src/mp/monocypher/monocypher.c" \
    "$TT/tt_duel.c" "$TT/tt_ai.c" "$TT/tt_catalog.c" "$TT/tt_common.c" "$TT/tt_hands.c" "$TT/tt_pools.c" \
    "$TT/tt_rating.c" "$TT/tt_ratings.c" "$TT/tt_collection.c" \
    "$TCG/tcg_duel.c" "$TCG/tcg_effects.c" "$TCG/tcg_trainers.c" "$TCG/tcg_powers.c" "$TCG/tcg_ai.c" "$TCG/tcg_catalog.c" \
    "$TCG/tcg_collection.c" "$TCG/tcg_decks.c" "$TCG/tcg_scale.c" "$TCG/tcg_ratings.c" "$TCG/tcg_levels.c" \
    "$TCG/tcg_flow.c" "$TCG/tcg_link.c" \
    "$YGO/ygo_catalog.c" "$YGO/ygo_fusions.c" "$YGO/ygo_edo.c" "$YGO/ygo_edo_data.c" "$YGO/ygo_edo_codes.c" "$YGO/ygo_ai.c" \
    "$YGO/ygo_collection.c" "$YGO/ygo_rating.c" "$YGO/ygo_npc_decks.c" "$YGO/ygo_card_ratings.c" "$ROOT/tools/ygo/stb_zlib.c" \
    -L"$OUT/ocgcore" -locgcore -lstdc++ -lm
javac -nowarn -d "$OUT/classes" -cp "$JSON" "$APP"/java/com/moemon/companion/core/*.java \
    "$APP/java/com/moemon/companion/Native.java" "$APP/java/com/moemon/companion/NativeCrypto.java" \
    "$APP/java/com/moemon/companion/Tt.java" "$APP/java/com/moemon/companion/TtFriendGame.java" \
    "$APP/java/com/moemon/companion/Tcg.java" "$APP/java/com/moemon/companion/TcgFriendGame.java" \
    "$ROOT/android/coretest/TcgCoreTest.java" "$ROOT/android/coretest/TcgFriendsTest.java" \
    "$APP/java/com/moemon/companion/Ygo.java" "$ROOT/android/coretest/YgoCoreTest.java" \
    "$APP/java/com/moemon/companion/YgoFriendGame.java" \
    "$ROOT/android/coretest/YgoFriendsTest.java" \
    "$ROOT/android/coretest/CoreTest.java" "$ROOT/android/coretest/WalkCoreTest.java" "$ROOT/android/coretest/FriendsTest.java" "$ROOT/android/coretest/TradeTest.java" "$ROOT/android/coretest/TtCoreTest.java" "$ROOT/android/coretest/TtFriendsTest.java"
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" YgoCoreTest "$ROOT"
bash -c "source '$ROOT/tools/test/common.sh' && build_server"
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" YgoFriendsTest "$ROOT/tools/test/.bin/moemon_server" "$ROOT"
[ "$ONLY" = ygo ] && exit 0
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" TcgCoreTest
bash -c "source '$ROOT/tools/test/common.sh' && build_server"
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" TcgFriendsTest "$ROOT/tools/test/.bin/moemon_server"
[ "$ONLY" = tcg ] && exit 0
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" TtCoreTest
bash -c "source '$ROOT/tools/test/common.sh' && build_server"
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" TtFriendsTest "$ROOT/tools/test/.bin/moemon_server"
[ "$ONLY" = tt ] && exit 0
if [ "$ONLY" != trade ]; then
java -cp "$OUT/classes:$JSON" WalkCoreTest
java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" CoreTest "$ROOT"
fi
# Phase 4: friends on a real co-op server (tools/test/common.sh builds it)
bash -c "source '$ROOT/tools/test/common.sh' && build_server"
[ "$ONLY" = trade ] || java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" FriendsTest "$ROOT" "$ROOT/tools/test/.bin/moemon_server"
# trades by mail between two games at lag (docs/PHONE_LINK.md "Trades by mail"); ONLY=trade runs just this one
exec java -Djava.library.path="$OUT" -cp "$OUT/classes:$JSON" TradeTest "$ROOT" "$ROOT/tools/test/.bin/moemon_server"
