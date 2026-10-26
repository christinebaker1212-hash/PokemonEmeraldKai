#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT=$(mktemp -d /tmp/moemon-tcg-tests.XXXXXX)
trap 'rm -rf "$OUT"' EXIT
for dev in 0 1; do
    gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
        -DTCG_TEST -DMOEMON_DEV_BUILD="$dev" -iquote include \
        tools/tcg/test_collection.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c -o "$OUT/test-$dev"
    mkdir "$OUT/profile-$dev"
    "$OUT/test-$dev" "$OUT/profile-$dev"
done

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_duel.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c -o "$OUT/duel"
"$OUT/duel"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_trainers.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c -o "$OUT/trainers"
"$OUT/trainers"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_ai.c src/tcg/tcg_ai.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c -o "$OUT/ai"
"$OUT/ai"
