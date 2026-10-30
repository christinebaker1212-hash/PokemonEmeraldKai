#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT=$(mktemp -d /tmp/moemon-tcg-tests.XXXXXX)
trap 'rm -rf "$OUT"' EXIT
# The built-in Game Boy illustrations match the PNGs they come from.
python3 tools/tcg/build_original_art.py > /dev/null
git diff --quiet -- graphics/tcg/original.bin || { echo 'graphics/tcg/original.bin is out of date'; exit 1; }
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
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/duel"
"$OUT/duel"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_effects.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/effects"
"$OUT/effects"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_attacks.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/attacks"
"$OUT/attacks"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g -DCARD_QUOTES_TEST -iquote include \
    tools/tcg/test_quotes.c src/tcg/card_quotes.c -o "$OUT/quotes"
"$OUT/quotes"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_gb2.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c src/tcg/tcg_ai.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/gb2"
"$OUT/gb2"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_trainers.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/trainers"
"$OUT/trainers"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_powers.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/powers"
"$OUT/powers"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_ai.c src/tcg/tcg_ai.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/ai"
"$OUT/ai"

gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_link.c src/tcg/tcg_link.c src/tcg/tcg_ai.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/link"
"$OUT/link"

# The special trainers' decks: up to date with the generator, legal, playable
python3 tools/tcg/gen_special_decks.py > /dev/null
git diff --quiet -- src/tcg/tcg_decks.c || { echo 'src/tcg/tcg_decks.c is out of date'; exit 1; }
gcc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
    -DTCG_TEST -DMOEMON_DEV_BUILD=1 -iquote include \
    tools/tcg/test_special.c src/tcg/tcg_decks.c src/tcg/tcg_ai.c src/tcg/tcg_collection.c src/tcg/tcg_catalog.c \
    src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c -o "$OUT/special"
"$OUT/special"

# CARD SCALING's deck ratings: up to date with the rules, the AI and the decks (about a minute)
tools/tcg/rate_decks.sh > /dev/null
git diff --quiet -- src/tcg/tcg_ratings.c || { echo 'src/tcg/tcg_ratings.c is out of date (tools/tcg/rate_decks.sh)'; exit 1; }
