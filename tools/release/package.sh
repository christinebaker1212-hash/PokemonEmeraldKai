#!/bin/bash
# Packages a built game into a release zip. usage: tools/release/package.sh windows|linux OUTDIR
#   windows: needs pokeemerald64.exe (make winwsl) and SDL2.dll (SDL2_DLL, default ./SDL2/x86_64-w64-mingw32/bin/SDL2.dll)
#   linux:   needs pokeemerald64 (make linux)
# Writes OUTDIR/MoemonEmeraldKaiCoop-<platform>.zip: one folder, MoemonEmeraldKaiCoop/, with the game, the
# player docs, the standalone server and moemon_release.txt (the updater only runs in a folder that has it).
set -e
cd "$(dirname "$0")/../.."
PLAT=${1:?usage: package.sh windows|linux OUTDIR}
OUT=$(mkdir -p "${2:?usage: package.sh windows|linux OUTDIR}" && cd "$2" && pwd)
VER=$(sed -n 's/^#define MOEMON_VERSION "\(.*\)"/\1/p' include/mp_version.h)
[ -n "$VER" ] || { echo "no MOEMON_VERSION in include/mp_version.h"; exit 1; }
STAGE=$(mktemp -d)
P="$STAGE/MoemonEmeraldKaiCoop"
mkdir -p "$P/server"
SRV="tools/mp_server/main.c src/mp/mp_net.c src/mp/mp_server.c src/mp/mp_portmap.c src/mp/mp_store.c"
case "$PLAT" in
windows)
    DLL=${SDL2_DLL:-SDL2/x86_64-w64-mingw32/bin/SDL2.dll}
    [ -f pokeemerald64.exe ] && [ -f "$DLL" ] || { echo "need pokeemerald64.exe and $DLL"; exit 1; }
    cp pokeemerald64.exe "$P/MoemonEmeraldKaiCoop.exe"
    cp "$DLL" "$P/SDL2.dll"
    x86_64-w64-mingw32-gcc -O2 -static -o "$P/server/moemon_server.exe" $SRV -lws2_32 -liphlpapi
    EXE=MoemonEmeraldKaiCoop.exe
    ;;
linux)
    [ -f pokeemerald64 ] || { echo "need pokeemerald64"; exit 1; }
    cp pokeemerald64 "$P/MoemonEmeraldKaiCoop"
    chmod 755 "$P/MoemonEmeraldKaiCoop"
    gcc -O2 -pthread -o "$P/server/moemon_server_linux" $SRV
    EXE=MoemonEmeraldKaiCoop
    ;;
*) echo "platform: windows or linux"; exit 1 ;;
esac
cp dist_extras/README.txt "dist_extras/Sprite Editor.html" dist_extras/custom_sprite_template.png dist_extras/lobby.txt "$P/"
cp tools/mp_server/DEPLOY.md "$P/server/"
printf 'Moemon Emerald Kai Co-op %s (%s)\nThe game checks for updates when it starts (AutoUpdate = 0 in moemon_coop.ini turns that off).\n' "$VER" "$PLAT" > "$P/moemon_release.txt"
ZIP="$OUT/MoemonEmeraldKaiCoop-$PLAT.zip"
rm -f "$ZIP"
(cd "$STAGE" && zip -qrX "$ZIP" MoemonEmeraldKaiCoop)
rm -rf "$STAGE"
echo "$ZIP ($VER, $EXE)"
