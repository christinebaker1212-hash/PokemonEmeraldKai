#!/bin/bash
# The Pokemon TCG power audit: each AI level against the best (src/tcg/tcg_levels.c) and every card's worth
# (docs/audit_tcg.txt). Rerun after a change to the rules or the AI (after tools/tcg/rate_decks.sh).
#   tools/tcg/rate_cards.sh [GAMES_PER_CARD] [LEVEL_GAMES]
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT=$(mktemp -d /tmp/moemon-tcg-cards.XXXXXX)
trap 'rm -rf "$OUT"' EXIT
gcc -std=c99 -O2 -Wall -Wextra -DTCG_TEST -DMOEMON_DEV_BUILD=0 -iquote include \
    tools/tcg/rate_cards.c src/tcg/tcg_scale.c src/tcg/tcg_decks.c src/tcg/tcg_ai.c src/tcg/tcg_collection.c \
    src/tcg/tcg_catalog.c src/tcg/tcg_duel.c src/tcg/tcg_effects.c src/tcg/tcg_trainers.c src/tcg/tcg_powers.c \
    src/tcg/tcg_ratings.c -lm -o "$OUT/rate"
"$OUT/rate" "${1:-80}" "${2:-400}"
head -6 docs/audit_tcg.txt
