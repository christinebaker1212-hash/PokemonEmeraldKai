#!/bin/bash
# Installs the release build's tools on the GitHub runner (release.yml, "Tools").
# usage: tools/release/ci_tools.sh DEB_DIR
# The runner's Ubuntu mirror (azure.archive.ubuntu.com) is sometimes slowed to a trickle (40 kB/s, where it
# normally gives 10 MB/s): apt never times out, and a plain apt-get crawls until the job does. So this:
# - asks only for what the build needs, without recommended packages (MinGW's win32 compiler only: the one
#   the build has always used);
# - starts from the .debs in DEB_DIR (release.yml keeps it between runs), so a run downloads only what changed;
# - stops a download (the package lists too) that brings in less than 1 MB in 15 seconds and goes on from another mirror: apt
#   keeps every package it finished, and every try's .debs go to DEB_DIR at once.
set -u
shopt -s nullglob
DEBS=${1:?usage: ci_tools.sh DEB_DIR}
PKGS="build-essential libpng-dev libsdl2-dev gcc-mingw-w64-x86-64-win32 zip unzip python3"
MIRRORS="http://azure.archive.ubuntu.com/ubuntu http://archive.ubuntu.com/ubuntu http://mirrors.edge.kernel.org/ubuntu http://us.archive.ubuntu.com/ubuntu"
. /etc/os-release
export DEBIAN_FRONTEND=noninteractive
ARCHIVES=/var/cache/apt/archives
LIST=$(mktemp)
# Only the Ubuntu archive (not the runner's other sources), no translations or AppStream / command-not-found
# indexes, no recommends.
APT=(-o Dir::Etc::sourcelist="$LIST" -o Dir::Etc::sourceparts=- -o APT::Install-Recommends=false
     -o Acquire::Languages=none -o Acquire::IndexTargets::deb::CNF::DefaultEnabled=false
     -o Acquire::IndexTargets::deb::DEP-11::DefaultEnabled=false
     -o Acquire::IndexTargets::deb::DEP-11-icons::DefaultEnabled=false
     -o Acquire::IndexTargets::deb::DEP-11-icons-small::DefaultEnabled=false
     -o Acquire::IndexTargets::deb::DEP-11-icons-hidpi::DefaultEnabled=false
     -o Acquire::Retries=1 -o Acquire::http::Timeout=20 -o DPkg::Lock::Timeout=60)

sudo rm -f /var/lib/man-db/auto-update
mkdir -p "$DEBS"
CACHED=("$DEBS"/*.deb)
echo "${#CACHED[@]} cached packages"
[ ${#CACHED[@]} -gt 0 ] && sudo cp -n "${CACHED[@]}" "$ARCHIVES"/

keep() { local f=("$ARCHIVES"/*.deb); [ ${#f[@]} -eq 0 ] || cp -n "${f[@]}" "$DEBS"/; }
got() { sudo du -sb "$1" | cut -f1; }

# usage: watched DIR ARGS: runs apt-get ARGS and stops it when less than 1 MB comes into DIR in 15 seconds.
watched() {
    local dir=$1 pid last now t=0
    shift
    sudo -E apt-get "${APT[@]}" "$@" &
    pid=$!
    last=$(got "$dir")
    while sleep 1; kill -0 $pid 2>/dev/null; do
        [ $((++t % 15)) -eq 0 ] || continue
        now=$(got "$dir")
        if [ $((now - last)) -lt 1000000 ]; then
            echo "only $(((now - last) / 15000)) kB/s"
            sudo kill $pid
            wait $pid
            return 1
        fi
        last=$now
    done
    wait $pid
}

for MIRROR in $MIRRORS; do
    echo "== $MIRROR"
    printf 'deb %s %s main universe\n' "$MIRROR" "$VERSION_CODENAME" "$MIRROR" "$VERSION_CODENAME-updates" \
        "$MIRROR" "$VERSION_CODENAME-security" > "$LIST"
    if ! watched /var/lib/apt/lists update; then
        echo "the package lists crawled or failed: next mirror"
        continue
    fi
    watched "$ARCHIVES" install -y --download-only $PKGS
    rc=$?
    keep
    if [ $rc -ne 0 ]; then
        echo "the download crawled or failed: next mirror, keeping what came"
        continue
    fi
    if timeout 300 sudo -E apt-get "${APT[@]}" install -y --no-download $PKGS; then
        # Only the versions now installed stay in DEB_DIR, so it doesn't grow run after run.
        for f in "$DEBS"/*.deb; do
            p=$(dpkg-deb -f "$f" Package) v=$(dpkg-deb -f "$f" Version)
            [ "$(dpkg-query -W -f '${Version}' "$p" 2>/dev/null)" = "$v" ] || rm -f "$f"
        done
        CACHED=("$DEBS"/*.deb)
        echo "${#CACHED[@]} packages kept for the next run"
        exit 0
    fi
    echo "the install failed: next mirror"
done
exit 1
