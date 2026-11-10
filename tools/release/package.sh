#!/bin/bash
# Packages a built game into a release zip. usage: tools/release/package.sh windows|linux OUTDIR
#   windows: needs pokeemerald64.exe (make winwsl) and SDL2.dll (SDL2_DLL, default ./SDL2/x86_64-w64-mingw32/bin/SDL2.dll)
#   linux:   needs pokeemerald64 (make linux)
# Writes OUTDIR/MoemonEmeraldKaiCoop-<platform>.zip: one folder, MoemonEmeraldKaiCoop/, with the game, the
# player docs, the VS cabinet's mugen/ folder, custom music (music/), SINNOH's 3D pack (sinnoh/), ORRE's packs (orre/), the standalone server and moemon_release.txt (the updater only runs in a folder that has it).
set -e
cd "$(dirname "$0")/../.."
PLAT=${1:?usage: package.sh windows|linux OUTDIR}
OUT=$(mkdir -p "${2:?usage: package.sh windows|linux OUTDIR}" && cd "$2" && pwd)
. tools/release/channel.sh
[ -n "$VER" ] || { echo "no MOEMON_VERSION in include/mp_version.h"; exit 1; }
STAGE=$(mktemp -d)
P="$STAGE/MoemonEmeraldKaiCoop"
mkdir -p "$P/server"
SRV="tools/mp_server/main.c src/mp/mp_net.c src/mp/mp_server.c src/mp/mp_portmap.c src/mp/mp_store.c src/mp/mp_scores.c"
case "$PLAT" in
windows)
    DLL=${SDL2_DLL:-SDL2/x86_64-w64-mingw32/bin/SDL2.dll}
    [ -f "$GAME_BIN.exe" ] && [ -f "$DLL" ] || { echo "need pokeemerald64.exe and $DLL"; exit 1; }
    cp "$GAME_BIN.exe" "$P/MoemonEmeraldKaiCoop.exe"
    cp "$DLL" "$P/SDL2.dll"
    x86_64-w64-mingw32-gcc -O2 -static -o "$P/server/moemon_server.exe" $SRV -lws2_32 -liphlpapi
    EXE=MoemonEmeraldKaiCoop.exe
    ;;
linux)
    [ -f "$GAME_BIN" ] || { echo "need pokeemerald64"; exit 1; }
    cp "$GAME_BIN" "$P/MoemonEmeraldKaiCoop"
    chmod 755 "$P/MoemonEmeraldKaiCoop"
    gcc -O2 -pthread -o "$P/server/moemon_server_linux" $SRV
    EXE=MoemonEmeraldKaiCoop
    ;;
*) echo "platform: windows or linux"; exit 1 ;;
esac
cp dist_extras/README.txt "dist_extras/Sprite Editor.html" dist_extras/custom_sprite_template.png dist_extras/lobby.txt dist_extras/PMD_CREDITS.txt dist_extras/EDOPRO_CREDITS.txt dist_extras/FORGE_CREDITS.txt dist_extras/ARCADE_CREDITS.txt "$P/"
# the GAME CENTER's CAPCOM VS cabinets: an empty folder for the player's own ROM sets (none ship)
mkdir -p "$P/arcade"
printf 'Put your ROM sets for the GAME CENTER 3F VS cabinets here (MAME zips, as they are):\r\n  SUPER STREET FIGHTER II X   ssf2t.zip and ssf2xj.zip\r\n  VAMPIRE SAVIOR              vsav.zip and vsavj.zip\r\n  CYBERBOTS                   cybots.zip\r\n  STREET FIGHTER ALPHA 3      sfa3.zip\r\nSee VS CABINETS in README.txt.\r\n' > "$P/arcade/ROM SETS GO HERE.txt"
cp tools/mp_server/DEPLOY.md "$P/server/"
# the VS cabinet's fighting game: its pack, as it goes next to the game (logs and screenshots left out)
mkdir -p "$P/mugen"
(cd mugen && tar cf - --exclude='*.log' --exclude='mugen*.png' .) | (cd "$P/mugen" && tar xf -)
# custom music (.kaisong songs that play instead of the game's, src/mp/mp_kaisong.c) and the page of every song and place
mkdir -p "$P/music"
cp music/*.kaisong "$P/music/" 2>/dev/null || true
cp music/kaisong_gains.txt "$P/music/" 2>/dev/null || true   # (every file's gain: tools/kaisong/normalize.py)
cp docs/SONG_TABLE.md "$P/music/SONG_TABLE.md"
# Moemon Maker's published pack (the repository's MoemonMaker/: the user's edits and music) as MoemonMakerPack/, so a
# fresh install has it without a download (src/platform/updater.c MakerPackWorker keeps it current between releases).
# The editor's own working copy (MoemonMaker/ beside a game) is never shipped, so an update never touches it.
if [ -d MoemonMaker ]; then
    tools/maker/build_pack.sh MoemonMaker "$P/.makerpack" > /dev/null
    mkdir -p "$P/MoemonMakerPack"
    cp -r "$P/.makerpack/pack/." "$P/MoemonMakerPack/"
    rm -rf "$P/.makerpack"
fi
# SINNOH's 3D pack and text pack (tools/sinnoh/make_pack.sh builds them from pret/pokeplatinum; the release runs it first)
if [ -f sinnoh/platinum3d.pak ]; then
    mkdir -p "$P/sinnoh"
    cp sinnoh/platinum3d.pak "$P/sinnoh/"
    if [ -f sinnoh/platinum_text.pak ]; then cp sinnoh/platinum_text.pak "$P/sinnoh/"; fi   # (its words)
    if [ -f sinnoh/platinum_daylight.bin ]; then cp sinnoh/platinum_daylight.bin "$P/sinnoh/"; fi   # (its time-of-day tint for the 2D regions)
fi
if [ -f sinnoh/platinum_gfx.pak ]; then   # (SINNOH's people as Platinum draws them, tools/sinnoh/build_gfx.py)
    mkdir -p "$P/sinnoh"
    cp sinnoh/platinum_gfx.pak "$P/sinnoh/"
fi
# ORRE's packs (tools/orre/make_pack.sh rips them from the disc; the release runs it first): in the game's own zip, so
# every update replaces them with the game (docs/ORRE_DECISIONS.md)
if [ -f orre/orre3d.pak ] && [ -f orre/orre_data.pak ] && [ -f orre/orre_text.pak ]; then
    mkdir -p "$P/orre"
    cp orre/orre3d.pak orre/orre_data.pak orre/orre_text.pak "$P/orre/"
fi
# POKeMON STADIUM's announcer voices (tools/stadium/mort/make_pack.sh, from the user's ROM; the release runs it first)
for v in announcer.s1 announcer_jp.s1; do   # (English, Japanese)
    if [ -f "stadium/$v" ]; then
        mkdir -p "$P/stadium"
        cp "stadium/$v" "$P/stadium/"
    fi
done
printf 'Moemon Emerald Kai Co-op %s (%s)\nThe game checks for updates when it starts (AutoUpdate = 0 in moemon_coop.ini turns that off).\n' "$VER ($CHANNEL)" "$PLAT" > "$P/moemon_release.txt"
ZIP="$OUT/MoemonEmeraldKaiCoop-$PLAT.zip"
rm -f "$ZIP"
(cd "$STAGE" && zip -qrX "$ZIP" MoemonEmeraldKaiCoop)
rm -rf "$STAGE"
echo "$ZIP ($VER, $EXE)"
