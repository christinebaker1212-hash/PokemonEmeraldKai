#!/bin/bash
# Source from the repository root. CHANNEL=main (default) or dev; dev needs DEV_REVISION.
CHANNEL=${CHANNEL:-main}
VER=$(sed -n 's/^#define MOEMON_VERSION "\(.*\)"/\1/p' include/mp_version.h)
case "$CHANNEL" in
main) TAG="v$VER"; GAME_BIN=pokeemerald64 ;;
dev)
    [[ ${DEV_REVISION:-} =~ ^[0-9]+$ ]] || { echo "dev needs numeric DEV_REVISION" >&2; exit 1; }
    VER="$VER.$DEV_REVISION"; TAG="dev-$VER"; GAME_BIN=pokeemerald-dev64 ;;
*) echo "CHANNEL must be main or dev" >&2; exit 1 ;;
esac
