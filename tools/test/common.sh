# Shared bits for the test runners (sourced). REPO = the checkout, OUT = where results go.
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TOOLS="$REPO/tools/test"
BIN="${BIN:-$REPO/pokeemerald64}"
OUT="${OUT:-${TMPDIR:-/tmp}/moemon_tests}"
SERVER="$TOOLS/.bin/moemon_server"

# The relay server, rebuilt when its sources change
build_server() {
    local srcs="$REPO/tools/mp_server/main.c $REPO/src/mp/mp_net.c $REPO/src/mp/mp_server.c $REPO/src/mp/mp_portmap.c $REPO/src/mp/mp_store.c"
    local stale=0 f
    [ -x "$SERVER" ] || stale=1
    for f in $srcs $REPO/src/mp/mp_net.h; do [ "$f" -nt "$SERVER" ] && stale=1; done
    if [ $stale = 1 ]; then
        mkdir -p "$TOOLS/.bin"
        gcc -O2 -pthread -o "$SERVER" $srcs || { echo "server build failed" >&2; return 1; }
    fi
}

# A free TCP port (asks the OS)
free_port() {
    python3 -c 'import socket; s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()'
}

# Screenshots (.bmp) to 2x PNGs
convert_shots() {
    local f
    command -v convert >/dev/null || return 0
    for f in "$1"/*.bmp; do [ -f "$f" ] && convert "$f" -scale 200% "${f%.bmp}.png" && rm -f "$f"; done
}
